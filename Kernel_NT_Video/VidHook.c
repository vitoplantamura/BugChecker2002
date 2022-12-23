/***************************************************************************************
  *
  * VidHook.c - VPCICE "Video Hooking" Routines.
  *
  * Copyright (c)2003 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
  *
***************************************************************************************/

/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

//===========
// Includes.
//===========

#include <ntddk.h>
#include "..\Include\vpcicevd.h"
#include "..\Include\VidHook.h"
#include "..\Include\WinDefs.h"
#include "..\Include\detours.h"

//==============================================
// GetDisplayDriverDllName Function Definition.
//==============================================

NTSTATUS GetDisplayDriverDllName( IN OUT PUNICODE_STRING pusDllName )
{
	NTSTATUS						nsRetVal = STATUS_UNSUCCESSFUL;
	NTSTATUS						nsOpenDevMapKeyRes;
	HANDLE							hDevMapKeyHandle;
	OBJECT_ATTRIBUTES				oaDevMapKeyAttr;
	UNICODE_STRING					usDevMapKeyName;
	NTSTATUS						nsReadVideo0ValueRes;
	UNICODE_STRING					usVideo0ValueName;
	PKEY_VALUE_FULL_INFORMATION		pkvfiVideo0ValueBuffer;
	ULONG							ulVideo0ValueBufferLen = 8 * 1024;
	WCHAR*							pszDevice0KeyName;
	UNICODE_STRING					usDevice0KeyName;
	OBJECT_ATTRIBUTES				oaDevice0KeyAttr;
	NTSTATUS						nsOpenDevice0KeyRes;
	HANDLE							hDevice0KeyHandle;
	PKEY_VALUE_FULL_INFORMATION		pkvfiInstDispDrvsValueBuffer;
	ULONG							ulInstDispDrvsValueBufferLen = 8 * 1024;
	UNICODE_STRING					usInstDispDrvsValueName;
	NTSTATUS						nsReadInstDispDrvsValueRes;

	// Try to acquire the name of the Display Driver.

	RtlInitUnicodeString( & usDevMapKeyName, L"\\REGISTRY\\MACHINE\\HARDWARE\\DEVICEMAP\\VIDEO" );

	InitializeObjectAttributes( & oaDevMapKeyAttr,
		& usDevMapKeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	nsOpenDevMapKeyRes = ZwOpenKey( & hDevMapKeyHandle,
		KEY_READ, & oaDevMapKeyAttr );

	if ( nsOpenDevMapKeyRes == STATUS_SUCCESS && hDevMapKeyHandle )
	{
		pkvfiVideo0ValueBuffer = (PKEY_VALUE_FULL_INFORMATION) ExAllocatePool( PagedPool, ulVideo0ValueBufferLen );

		if ( pkvfiVideo0ValueBuffer )
		{
			RtlInitUnicodeString( & usVideo0ValueName, L"\\Device\\Video0" );

			nsReadVideo0ValueRes = ZwQueryValueKey( hDevMapKeyHandle,
				& usVideo0ValueName, KeyValueFullInformation,
				pkvfiVideo0ValueBuffer, ulVideo0ValueBufferLen, & ulVideo0ValueBufferLen );

			if ( nsReadVideo0ValueRes == STATUS_SUCCESS &&
				ulVideo0ValueBufferLen >= sizeof( KEY_VALUE_FULL_INFORMATION ) )
			{
				pszDevice0KeyName = (WCHAR*) ExAllocatePool( PagedPool, pkvfiVideo0ValueBuffer->DataLength + sizeof( WCHAR ) );

				if ( pszDevice0KeyName )
				{
					memcpy( pszDevice0KeyName, (BYTE*) pkvfiVideo0ValueBuffer + pkvfiVideo0ValueBuffer->DataOffset, pkvfiVideo0ValueBuffer->DataLength );
					memset( (BYTE*) pszDevice0KeyName + pkvfiVideo0ValueBuffer->DataLength, 0, sizeof( WCHAR ) );

					RtlInitUnicodeString( & usDevice0KeyName, pszDevice0KeyName );

					InitializeObjectAttributes( & oaDevice0KeyAttr,
						& usDevice0KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

					nsOpenDevice0KeyRes = ZwOpenKey( & hDevice0KeyHandle,
						KEY_READ, & oaDevice0KeyAttr );

					if ( nsOpenDevice0KeyRes == STATUS_SUCCESS && hDevice0KeyHandle )
					{
						pkvfiInstDispDrvsValueBuffer = (PKEY_VALUE_FULL_INFORMATION) ExAllocatePool( PagedPool, ulInstDispDrvsValueBufferLen );

						if ( pkvfiInstDispDrvsValueBuffer )
						{
							RtlInitUnicodeString( & usInstDispDrvsValueName, L"InstalledDisplayDrivers" );

							nsReadInstDispDrvsValueRes = ZwQueryValueKey( hDevice0KeyHandle,
								& usInstDispDrvsValueName, KeyValueFullInformation,
								pkvfiInstDispDrvsValueBuffer, ulInstDispDrvsValueBufferLen, & ulInstDispDrvsValueBufferLen );

							if ( nsReadInstDispDrvsValueRes == STATUS_SUCCESS &&
								ulInstDispDrvsValueBufferLen >= sizeof( KEY_VALUE_FULL_INFORMATION ) )
							{
								RtlAppendUnicodeToString( pusDllName,
									(PCWSTR) ( (BYTE*) pkvfiInstDispDrvsValueBuffer + pkvfiInstDispDrvsValueBuffer->DataOffset ) );
								RtlAppendUnicodeToString( pusDllName,
									L".dll" );

								nsRetVal = STATUS_SUCCESS;
							}

							ExFreePool( pkvfiInstDispDrvsValueBuffer );
						}

						ZwClose( hDevice0KeyHandle );
					}

					ExFreePool( pszDevice0KeyName );
				}
			}

			ExFreePool( pkvfiVideo0ValueBuffer );
		}

		ZwClose( hDevMapKeyHandle );
	}

	// Return to the Caller.

	return nsRetVal;
}

//==========================================
// GePeImageEntryPoint Function Definition.
//==========================================

PVOID GePeImageEntryPoint( PVOID pvPeImageStart )
{
	IMAGE_DOS_HEADER*			pidhImageDosHeader;
	IMAGE_NT_HEADERS*			pinhPeNtHdrs;

	__try
	{
		// Do the Requested Operation.

		pidhImageDosHeader = (IMAGE_DOS_HEADER*) pvPeImageStart;

		if ( pidhImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE )
			return NULL;
		if ( pidhImageDosHeader->e_lfarlc < 0x40 )
			return NULL;

		pinhPeNtHdrs = (IMAGE_NT_HEADERS*) ( (BYTE*) pvPeImageStart + pidhImageDosHeader->e_lfanew );

		if ( pinhPeNtHdrs->Signature != IMAGE_NT_SIGNATURE )
			return NULL;

		// Return the Information to the Caller.

		return (PVOID) ( (BYTE*) pvPeImageStart + pinhPeNtHdrs->OptionalHeader.AddressOfEntryPoint );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		return NULL;
	}
}

//============================================================================
// HookDrvGetDirectDrawInfoFunction Function Definition / Related Structures.
//============================================================================

//
// ### Macros / Global Data ###
//

typedef BOOL ( APIENTRY * PFNDRVGETDIRECTDRAWINFO ) (
    DHPDEV        dhpdev,
    DD_HALINFO   *pHalInfo,
    DWORD        *pdwNumHeaps,
    VIDEOMEMORY  *pvmList,
    DWORD        *pdwNumFourCCCodes,
    DWORD        *pdwFourCC
);

static PFNDRVGETDIRECTDRAWINFO		g_pfnDrvGetDirectDrawInfo = NULL;

//
// ### Hooked_DrvGetDirectDrawInfo ###
//

BOOL APIENTRY Hooked_DrvGetDirectDrawInfo(
    DHPDEV        dhpdev,
    DD_HALINFO   *pHalInfo,
    DWORD        *pdwNumHeaps,
    VIDEOMEMORY  *pvmList,
    DWORD        *pdwNumFourCCCodes,
    DWORD        *pdwFourCC
)
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOL					bOriginalFnRetVal;
	ULONG					i;

	// Call the Original Function.

	bOriginalFnRetVal = g_pfnDrvGetDirectDrawInfo( dhpdev, pHalInfo, pdwNumHeaps, pvmList, pdwNumFourCCCodes, pdwFourCC );

	// Check to see whether we have the Video Memory Informations that we need.

	if ( bOriginalFnRetVal &&
		pHalInfo && pHalInfo->dwSize >= sizeof( DD_HALINFO ) )
	{
		// Save the Video Memory Informations.

		ExAcquireFastMutex( & extension->fmVideoMemoryInfoFastMutex );

			extension->vmiVideoMemoryInfo = pHalInfo->vmiData;

		ExReleaseFastMutex( & extension->fmVideoMemoryInfoFastMutex );

		// Inform about the change all the Subscribers.

		ExAcquireFastMutex( & extension->fmVmiSubscriptionsDataFastMutex );

			for ( i=0; i<extension->ulVmiSubscriptionsNum; i++ )
				* extension->vpvmiVmiSubscriptions[ i ] = pHalInfo->vmiData;

		ExReleaseFastMutex( & extension->fmVmiSubscriptionsDataFastMutex );
	}

	// Return to the Caller.

	return bOriginalFnRetVal;
}

//
// ### HookDrvGetDirectDrawInfoFunction ###
//

NTSTATUS HookDrvGetDirectDrawInfoFunction( DRVFN* pdrvfnDrvGetDirectDrawInfo )
{
	NTSTATUS		nsRetVal = STATUS_UNSUCCESSFUL;

	// Do the Requested Operation.

	__try
	{
		g_pfnDrvGetDirectDrawInfo = (PFNDRVGETDIRECTDRAWINFO) pdrvfnDrvGetDirectDrawInfo->pfn;
		pdrvfnDrvGetDirectDrawInfo->pfn = (PFN) & Hooked_DrvGetDirectDrawInfo;

		nsRetVal = STATUS_SUCCESS;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		nsRetVal = STATUS_UNSUCCESSFUL;
	}

	// Return to the Caller.

	return nsRetVal;
}

//=======================================================================
// HookDisplayDriverEntryPoint Function Definition / Related Structures.
//=======================================================================

//
// ### Macros / Global Data ###
//

static PVOID		g_pfnDrvEnableDriver = NULL;
DETOUR_TRAMPOLINE_GLOBVAR( BOOL APIENTRY Trampoline_DrvEnableDriver( ULONG iEngineVersion, ULONG cj, DRVENABLEDATA *pded ), g_pfnDrvEnableDriver )

static DRVFN		g_vpfnReplacedDrvFunctions[ 1024 ] = {
	{	0,		NULL }
};

#define		MACRO_ReplacedDrvFunctions_C		( sizeof( g_vpfnReplacedDrvFunctions ) / sizeof( DRVFN ) )

//
// ### Hooked_DrvEnableDriver ###
//

BOOL APIENTRY Hooked_DrvEnableDriver(
    ULONG          iEngineVersion,
    ULONG          cj,
    DRVENABLEDATA *pded )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOL					bOriginalFnRetVal;
	ULONG					i;
	DRVFN*					pdrvfnDrvGetDirectDrawInfo;
	ULONG					ulHookedDrvEnableDriverCallsCounterPrev;

	// Call the Original Function.

	bOriginalFnRetVal = Trampoline_DrvEnableDriver( iEngineVersion, cj, pded );

	// Check the Counter associated to this Operation.

	ExAcquireFastMutex( & extension->fmHookedDrvEnableDriverCallsCounterFastMutex );

		ulHookedDrvEnableDriverCallsCounterPrev = extension->ulHookedDrvEnableDriverCallsCounter;
		extension->ulHookedDrvEnableDriverCallsCounter ++;

	ExReleaseFastMutex( & extension->fmHookedDrvEnableDriverCallsCounterFastMutex );

	// We have to hook the "DrvGetDirectDrawInfo" Function.

	if ( ulHookedDrvEnableDriverCallsCounterPrev == 0 &&
		bOriginalFnRetVal &&
		cj >= sizeof( DRVENABLEDATA ) && pded && pded->pdrvfn && pded->c &&
		pded->c <= MACRO_ReplacedDrvFunctions_C )
	{
		// Replace the list of Exported Functions.

		memcpy( g_vpfnReplacedDrvFunctions, pded->pdrvfn, pded->c * sizeof( DRVFN ) );
		pded->pdrvfn = g_vpfnReplacedDrvFunctions;

		// Iterate through the list of the Exported Functions by the driver.

		pdrvfnDrvGetDirectDrawInfo = NULL;

		for ( i=0; i<pded->c; i++ )
			if ( pded->pdrvfn[ i ].iFunc == INDEX_DrvGetDirectDrawInfo )
			{
				pdrvfnDrvGetDirectDrawInfo = & pded->pdrvfn[ i ];
				break;
			}

		// Check if we have found the function and/or if it is supported by the driver.

		if ( pdrvfnDrvGetDirectDrawInfo )
			HookDrvGetDirectDrawInfoFunction( pdrvfnDrvGetDirectDrawInfo );
	}

	// Return to the Caller.

	return bOriginalFnRetVal;
}

//
// ### HookDisplayDriverEntryPoint ###
//

NTSTATUS HookDisplayDriverEntryPoint( PVOID pvDrvEnableDriverFnPtr )
{
	NTSTATUS		nsRetVal = STATUS_UNSUCCESSFUL;
	BOOLEAN			bHookRes;

	// Do the Requested Operation.

	__try
	{
		g_pfnDrvEnableDriver = pvDrvEnableDriverFnPtr;
		bHookRes = DetourFunctionWithTrampoline( (PBYTE) Trampoline_DrvEnableDriver, (PBYTE) Hooked_DrvEnableDriver, NULL );

		if ( bHookRes )
			nsRetVal = STATUS_SUCCESS;
		else
			nsRetVal = STATUS_UNSUCCESSFUL;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		nsRetVal = STATUS_UNSUCCESSFUL;
	}

	// Return to the Caller.

	return nsRetVal;
}
