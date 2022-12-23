/***************************************************************************************
  *
  * vpcicevd.c - VPCICE "Video Hooking" Kernel Module for Windows NT - Version 0.1.
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
#include "..\Include\IOCTLs_Video.h"
#include "..\Include\VidHook.h"
#include "..\Include\Utils.h"

//==================================
// Boilerplate KM device functions.
//==================================

NTSTATUS VPCIceVideoDispatchCreate( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS VPCIceVideoDispatchClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS VPCIceVideoDispatchIoctl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );

//==============================================
// VPCIceVideoImageCallback Function Prototype.
//==============================================

VOID VPCIceVideoImageCallback( IN PUNICODE_STRING FullImageName, IN HANDLE ProcessId, IN PIMAGE_INFO ImageInfo );

//=================================================
// Private storage for us to squirrel things away.
//=================================================

// *** Buffer used in the Device Extension. ***

static WCHAR		g_szDisplayDriverDllNameBuffer[ 256 ] = L""; // Buffer used by _DEVICE_EXTENSION.usDisplayDriverDllName.

// *** Device Object Global Pointer. ***

PDEVICE_OBJECT		g_pDeviceObject;

//==================================
// DriverEntry Function Definition.
//==================================

NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath )
{
	NTSTATUS        ntStatus;
	UNICODE_STRING  uszDriverString;
	UNICODE_STRING  uszDeviceString;

	PDEVICE_OBJECT    pDeviceObject;
	PDEVICE_EXTENSION extension;

	// Point uszDriverString at the driver name.

	RtlInitUnicodeString( & uszDriverString, L"\\Device\\" SYSDRIVER_NAME_WIDE );

	// Create and initialize device object.

	ntStatus = IoCreateDevice( DriverObject,
		sizeof( DEVICE_EXTENSION ),
		& uszDriverString,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		& pDeviceObject );

	if ( ntStatus != STATUS_SUCCESS )
	{
		// Return to the Caller.

		return ntStatus;
	}

	// Assign extension variable.

	extension = pDeviceObject->DeviceExtension;

	// Point uszDeviceString at the device name.

	RtlInitUnicodeString( & uszDeviceString, L"\\DosDevices\\" SYSDRIVER_NAME_WIDE );

	// Create symbolic link to the user-visible name.

	ntStatus = IoCreateSymbolicLink( & uszDeviceString, & uszDriverString );

	if ( ntStatus != STATUS_SUCCESS )
	{
		// Delete device object if not successful.

		IoDeleteDevice( pDeviceObject );

		return ntStatus;
	}

	// Assign global pointer to the device object.

	g_pDeviceObject = pDeviceObject;

	// Load structure to point to IRP handlers.

	DriverObject->DriverUnload                           = NULL;
	DriverObject->MajorFunction[ IRP_MJ_CREATE ]         = VPCIceVideoDispatchCreate;
	DriverObject->MajorFunction[ IRP_MJ_CLOSE ]          = VPCIceVideoDispatchClose;
	DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = VPCIceVideoDispatchIoctl;

	// Initialize the variables in the Device Extension.

	extension->ulDisplayDriverHookedCounter = 0;
	ExInitializeFastMutex( & extension->fmDisplayDriverHookedCounterFastMutex );

	extension->ulHookedDrvEnableDriverCallsCounter = 0;
	ExInitializeFastMutex( & extension->fmHookedDrvEnableDriverCallsCounterFastMutex );

	// Initialize the variables related to the acquisition of the Display Driver Dll name.

	RtlInitUnicodeString( & extension->usDisplayDriverDllName, g_szDisplayDriverDllNameBuffer );
	extension->usDisplayDriverDllName.MaximumLength = sizeof( g_szDisplayDriverDllNameBuffer );

	extension->nsDisplayDriverDllNameGetOpRes = STATUS_UNSUCCESSFUL;

	ExInitializeFastMutex( & extension->fmDisplayDriverDllNameFastMutex );

	// Initialize the variables related to the system Current Video Mode.

	memset( & extension->vmiVideoMemoryInfo, 0, sizeof( VIDEOMEMORYINFO ) );
	ExInitializeFastMutex( & extension->fmVideoMemoryInfoFastMutex );

	// Initialize the Structures for the "VIDEOMEMORYINFO Subscriptions" mechanism.

	extension->ulVmiSubscriptionsNum = 0;
	memset( extension->vpvmiVmiSubscriptions, 0, sizeof( VIDEOMEMORYINFO* ) * MACRO_VMI_SUBSCRIPTIONS_MAXNUM );
	ExInitializeFastMutex( & extension->fmVmiSubscriptionsDataFastMutex );

	// Initialize the Pointer to the Text Video Buffer.

	extension->pvTextVideoBuffer = NULL;

	// Register our Image Loading notification handler. However, first show a Confirmation Message.

	extension->bStartUpCancelMessageAccepted = FALSE;

	if ( InitializeTextVideoBufferPtr() == STATUS_SUCCESS )
		OutputTextString( extension->pvTextVideoBuffer, 80, 25, 15, 24, 0x74, "  Press \"X\" to cancel loading " MACRO_PROGRAM_NAME "Video...  " );

	if ( WaitForKeyBeingPressed( 0x2D, 500 ) != STATUS_SUCCESS )
		PsSetLoadImageNotifyRoutine( & VPCIceVideoImageCallback );
	else
		extension->bStartUpCancelMessageAccepted = TRUE;

	if ( InitializeTextVideoBufferPtr() == STATUS_SUCCESS )
		OutputTextString( extension->pvTextVideoBuffer, 80, 25, 15, 24, 0x7, "                                                  " );

	// Return Success.

	return STATUS_SUCCESS;
}

//================================================
// VPCIceVideoDispatchCreate Function Definition.
//================================================

NTSTATUS VPCIceVideoDispatchCreate( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
	NTSTATUS          ntStatus = STATUS_SUCCESS;

	// Return to the caller

	Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;
}

//===============================================
// VPCIceVideoDispatchClose Function Definition.
//===============================================

NTSTATUS VPCIceVideoDispatchClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

	// Return to the caller

	Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}

//===============================================
// VPCIceVideoDispatchIoctl Function Definition.
//===============================================

NTSTATUS VPCIceVideoDispatchIoctl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    NTSTATUS              ntStatus = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION    irpStack  = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION     extension = DeviceObject->DeviceExtension;
	VIDEOMEMORYINFO*      pvmiVmiPtr;

	DWORD*                pl;

	// IOCTL Processor Switch.

	if ( extension->bStartUpCancelMessageAccepted )
	{
		// The user pressed "ESC" at the Start-Up.

		ntStatus = STATUS_UNSUCCESSFUL;
	}
	else
	{
		switch( irpStack->Parameters.DeviceIoControl.IoControlCode )
		{
			//-----------------------------------------------------------------------------------

			case IOCTL_VPCICEVID_GET_VERSION:
				{
					// Return the information required.

					if ( irpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof( DWORD ) )
					{
						// Copy the information in the system buffer.

						pl = Irp->AssociatedIrp.SystemBuffer;
						*pl = ( DWORD ) _VPCICEVID_DRIVER_VERSION_;

						// Everything went ok.

						ntStatus = STATUS_SUCCESS;
					}
				}
				break;

			//-----------------------------------------------------------------------------------

			case IOCTL_VPCICEVID_GET_VIDEOMEMORYINFO:
				{
					// Return the information required.

					if ( irpStack->Parameters.DeviceIoControl.OutputBufferLength == sizeof( VIDEOMEMORYINFO ) )
					{
						// Copy the information in the system buffer.

						ExAcquireFastMutex( & extension->fmVideoMemoryInfoFastMutex );

							* ( VIDEOMEMORYINFO * ) Irp->AssociatedIrp.SystemBuffer = extension->vmiVideoMemoryInfo;

						ExReleaseFastMutex( & extension->fmVideoMemoryInfoFastMutex );

						// Everything went ok.

						ntStatus = STATUS_SUCCESS;
					}
				}
				break;

			//-----------------------------------------------------------------------------------

			case IOCTL_VPCICEVID_SUBSCRIBE_VIDEOMEMORYINFO_BUFFER:
				{
					// Subscribe the passed pointer.

					if ( irpStack->Parameters.DeviceIoControl.InputBufferLength == sizeof( VIDEOMEMORYINFO* ) )
					{
						pvmiVmiPtr = (VIDEOMEMORYINFO*) * (DWORD*) Irp->AssociatedIrp.SystemBuffer;

						// Do the Subscription.

						ExAcquireFastMutex( & extension->fmVmiSubscriptionsDataFastMutex );

						if ( extension->ulVmiSubscriptionsNum < MACRO_VMI_SUBSCRIPTIONS_MAXNUM )
						{
							// Keep the passed pointer.
							extension->vpvmiVmiSubscriptions[ extension->ulVmiSubscriptionsNum ++ ] = pvmiVmiPtr;

							// Initialize the passed pointed structure.
							ExAcquireFastMutex( & extension->fmVideoMemoryInfoFastMutex );
								* pvmiVmiPtr = extension->vmiVideoMemoryInfo;
							ExReleaseFastMutex( & extension->fmVideoMemoryInfoFastMutex );

							// Everything went ok.
							ntStatus = STATUS_SUCCESS;
						}

						ExReleaseFastMutex( & extension->fmVmiSubscriptionsDataFastMutex );
					}
				}
				break;

			//-----------------------------------------------------------------------------------

			default:
				break;

			//-----------------------------------------------------------------------------------
		}
	}

	// Set the Return Value.

    Irp->IoStatus.Status = ntStatus;
    
    // Set # of bytes to copy back to user-mode.

    if ( ntStatus == STATUS_SUCCESS )
        Irp->IoStatus.Information = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    else
        Irp->IoStatus.Information = 0;

	// Return to the Caller.

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return ntStatus;
}

//===============================================
// VPCIceVideoImageCallback Function Definition.
//===============================================

VOID VPCIceVideoImageCallback( IN PUNICODE_STRING FullImageName, IN HANDLE ProcessId, IN PIMAGE_INFO ImageInfo )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	WCHAR					szFullImageNameUpper[ 256 ], szDispDrvDllNameUpper[ 256 ];
	ULONG					ulDisplayDriverHookedCounterPrev;
	ULONG					ulProceedWithHooking;
	WCHAR*					pszWcsStrRes;
	NTSTATUS				nsEntryPtHookRes;
	PVOID					pvDrvEnableDriverFnPtr;
	BOOLEAN					bIsEnforceWriteProtectionSetTo0;

	// Intercept only Image Loading operations relative to the kernel space.

	if ( ProcessId != 0 || ImageInfo->SystemModeImage == 0 )
		return;

	// Check if the Display Driver name was acquired.

	ExAcquireFastMutex( & extension->fmDisplayDriverDllNameFastMutex );

		if ( extension->nsDisplayDriverDllNameGetOpRes != STATUS_SUCCESS )
			extension->nsDisplayDriverDllNameGetOpRes = GetDisplayDriverDllName( & extension->usDisplayDriverDllName );

		if ( extension->nsDisplayDriverDllNameGetOpRes == STATUS_SUCCESS )
			ulProceedWithHooking = 1;
		else
			ulProceedWithHooking = 0;

	ExReleaseFastMutex( & extension->fmDisplayDriverDllNameFastMutex );

	if ( ulProceedWithHooking == 0 )
		return;

	// Check to see if the Loading Module is the Display Driver we are interested in.

	if ( UnicodeStringToUnicode( FullImageName, szFullImageNameUpper, sizeof( szFullImageNameUpper ) ) != STATUS_SUCCESS ||
		UnicodeStringToUnicode( & extension->usDisplayDriverDllName, szDispDrvDllNameUpper, sizeof( szDispDrvDllNameUpper ) ) != STATUS_SUCCESS )
			return;

	_wcsupr( szFullImageNameUpper );
	_wcsupr( szDispDrvDllNameUpper );

	pszWcsStrRes = wcsstr( szFullImageNameUpper, szDispDrvDllNameUpper );

	if ( pszWcsStrRes == NULL )
		return;

	pszWcsStrRes --;

	if ( pszWcsStrRes >= szFullImageNameUpper &&
		* pszWcsStrRes != L'\\' &&
		* pszWcsStrRes != L'/' )
			return;

	// Check whether we have already hooked the module.

	ExAcquireFastMutex( & extension->fmDisplayDriverHookedCounterFastMutex );
		ulDisplayDriverHookedCounterPrev = extension->ulDisplayDriverHookedCounter;
		extension->ulDisplayDriverHookedCounter ++;
	ExReleaseFastMutex( & extension->fmDisplayDriverHookedCounterFastMutex );

	// !! WARNING !! We will hook only the module with the specified name that the system will try to load the SECOND time.

	if ( ulDisplayDriverHookedCounterPrev != 1 )
		return;

	// Proceed further only if "EnforceWriteProtection == 0".

	if ( IsEnforceWriteProtectionSetTo0( & bIsEnforceWriteProtectionSetTo0 ) != STATUS_SUCCESS ||
		bIsEnforceWriteProtectionSetTo0 == FALSE )
			return;

	// We can hook the Display Driver Module.

	pvDrvEnableDriverFnPtr = GePeImageEntryPoint( ImageInfo->ImageBase );

	if ( pvDrvEnableDriverFnPtr == NULL )
		return;

	nsEntryPtHookRes = HookDisplayDriverEntryPoint( pvDrvEnableDriverFnPtr );

	if ( nsEntryPtHookRes != STATUS_SUCCESS )
		return;

	// Return to the Caller.

	return;
}

//=============================================
// UnicodeStringToUnicode Function Definition.
//=============================================

NTSTATUS UnicodeStringToUnicode( IN PUNICODE_STRING pusInput, OUT WCHAR* pszOutput, IN ULONG ulOutputSizeInBytes )
{
	// Do the Memory Copy.

	if ( pusInput->Length + sizeof( WCHAR ) > ulOutputSizeInBytes )
		return STATUS_UNSUCCESSFUL;

	memcpy( pszOutput, pusInput->Buffer, pusInput->Length );
	memset( (BYTE*) pszOutput + pusInput->Length, 0, sizeof( WCHAR ) );

	return STATUS_SUCCESS;
}

//=====================================================
// IsEnforceWriteProtectionSetTo0 Function Definition.
//=====================================================

NTSTATUS IsEnforceWriteProtectionSetTo0( OUT BOOLEAN* pbResult )
{
	NTSTATUS						nsRetVal = STATUS_UNSUCCESSFUL;
	UNICODE_STRING					usMemoryManagementKeyName;
	OBJECT_ATTRIBUTES				oaMemoryManagementKeyAttr;
	NTSTATUS						nsOpenMemoryManagementKeyRes;
	HANDLE							hMemoryManagementKeyHandle;
	PKEY_VALUE_FULL_INFORMATION		pkvfiEnforceWriteProtectionValueBuffer;
	ULONG							ulEnforceWriteProtectionValueBufferLen = 8 * 1024;
	UNICODE_STRING					usEnforceWriteProtectionValueName;
	NTSTATUS						nsReadEnforceWriteProtectionValueRes;

	* pbResult = FALSE;

	// Read from the Registry.

	RtlInitUnicodeString( & usMemoryManagementKeyName, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management" );

	InitializeObjectAttributes( & oaMemoryManagementKeyAttr,
		& usMemoryManagementKeyName, 0, NULL, NULL );

	nsOpenMemoryManagementKeyRes = ZwOpenKey( & hMemoryManagementKeyHandle,
		KEY_READ, & oaMemoryManagementKeyAttr );

	if ( nsOpenMemoryManagementKeyRes == STATUS_SUCCESS && hMemoryManagementKeyHandle )
	{
		pkvfiEnforceWriteProtectionValueBuffer = (PKEY_VALUE_FULL_INFORMATION) ExAllocatePool( PagedPool, ulEnforceWriteProtectionValueBufferLen );

		if ( pkvfiEnforceWriteProtectionValueBuffer )
		{
			RtlInitUnicodeString( & usEnforceWriteProtectionValueName, L"EnforceWriteProtection" );

			nsReadEnforceWriteProtectionValueRes = ZwQueryValueKey( hMemoryManagementKeyHandle,
				& usEnforceWriteProtectionValueName, KeyValueFullInformation,
				pkvfiEnforceWriteProtectionValueBuffer, ulEnforceWriteProtectionValueBufferLen, & ulEnforceWriteProtectionValueBufferLen );

			if ( nsReadEnforceWriteProtectionValueRes == STATUS_SUCCESS &&
				ulEnforceWriteProtectionValueBufferLen >= sizeof( KEY_VALUE_FULL_INFORMATION ) )
			{
				if ( pkvfiEnforceWriteProtectionValueBuffer->Type == REG_DWORD &&
					pkvfiEnforceWriteProtectionValueBuffer->DataLength == sizeof( DWORD ) &&
					* (DWORD*) ( ( BYTE* ) pkvfiEnforceWriteProtectionValueBuffer + pkvfiEnforceWriteProtectionValueBuffer->DataOffset ) == 0 )
				{
					* pbResult = TRUE;
				}

				nsRetVal = STATUS_SUCCESS;
			}

			ExFreePool( pkvfiEnforceWriteProtectionValueBuffer );
		}

		ZwClose( hMemoryManagementKeyHandle );
	}

	// Return to the Caller.

	return nsRetVal;
}

//===================================================
// InitializeTextVideoBufferPtr Function Definition.
//===================================================

NTSTATUS InitializeTextVideoBufferPtr( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	PVOID					pvTextVideoBuffer;
	ULONG					ulOutBufferSize;

	// Initialize the Pointer.

	if ( extension->pvTextVideoBuffer == NULL )
	{
#ifdef USE_PAGETABLE_TO_OBTAIN_TEXTMODE_VIDEOADDRESS
		if ( PhysAddressToLinearAddresses( & pvTextVideoBuffer, 1, & ulOutBufferSize, 0xB8000 ) == STATUS_SUCCESS &&
			ulOutBufferSize == 1 )
		{
			extension->pvTextVideoBuffer = pvTextVideoBuffer;

			return STATUS_SUCCESS;
		}
		else
		{
			return STATUS_UNSUCCESSFUL;
		}
#else
		PHYSICAL_ADDRESS		paPhysAddr;
		paPhysAddr.QuadPart = 0xB8000;
		extension->pvTextVideoBuffer = MmMapIoSpace( paPhysAddr, 1, MmNonCached );
#endif
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}
