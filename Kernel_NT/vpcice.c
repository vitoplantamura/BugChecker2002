/***************************************************************************************
  *
  * vpcice.c - VPCICE for Windows NT - Version 0.1
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

#pragma warning(disable : 4296) // WDK 7600.16385.0 compatibility.

void _chkstk() {} // WDK 7600.16385.0 compatibility.

//===========
// Includes.
//===========

#include <ntddk.h>
#include <stdio.h>
#include <stdarg.h>
#include "..\Include\vpcice.h"
#include "..\Include\IOCTLs.h"
#include "..\Include\Utils.h"
#include "..\Include\8042.h"
#include "..\Include\disasm.h"
#include "..\Include\crt.h"
#include "..\Include\memfile.h"
#include "..\Include\ccomp.h"

//==================================
// Boilerplate KM device functions.
//==================================

NTSTATUS VPCIceDispatchCreate( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS VPCIceDispatchClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS VPCIceDispatchIoctl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );

//=================================================
// Private storage for us to squirrel things away.
//=================================================

// *** Device Object Global Pointer. ***

PDEVICE_OBJECT			g_pDeviceObject;

//===================
// Global Variables.
//===================

PVOID					g_pvMmUserProbeAddress = NULL;

DWORD					g_dwNumberProcessors = 0;

//==================================
// DriverEntry Function Definition.
//==================================

ULONG MajorVersion = 0;
ULONG MinorVersion = 0;
ULONG BuildNumber = 0;
BOOLEAN CheckedBuild = FALSE;
DWORD VmWareVersion = 0;
BOOLEAN _8042MinimalProgramming = FALSE;

NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath )
{
	NTSTATUS        ntStatus;
	UNICODE_STRING  uszDriverString;
	UNICODE_STRING  uszDeviceString;

	PDEVICE_OBJECT    pDeviceObject;
	PDEVICE_EXTENSION extension;

	NTSTATUS			nsVpcICEvdSubscrRes;
	NTSTATUS			nsVpcICEVidMemInitRes;
	NTSTATUS			nsVpcICEIntHooksInstallRes;

	ULONG				ulInitialWidth, ulInitialHeight;

	BOOLEAN				bCanContinue;
	BOOLEAN				bCanOpenDebuggerForInitialization = FALSE;

	ULONG				ulNtTerminateProcessFnIndex;

	ULONG				ulI;

	CHAR*				psz;

	CHAR				szVmware[ 64 ];

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
	DriverObject->MajorFunction[ IRP_MJ_CREATE ]         = VPCIceDispatchCreate;
	DriverObject->MajorFunction[ IRP_MJ_CLOSE ]          = VPCIceDispatchClose;
	DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = VPCIceDispatchIoctl;

	// Initialize the Debugger Verb Vars.

	extension->bOpenUserModeDashboard = FALSE;

	memset( & extension->liLastDebuggerVerbReadTime, 0, sizeof( LARGE_INTEGER ) );

	extension->dwLastEnterTscHigh = 0;
	extension->dwLastEnterTscLow = 0;

	// Initialize the "Page Frame Modifications Database".

	InitializePFMDB();

	// Initialize several Variables.

	extension->bSnapShotMode = FALSE;
	extension->ulI3HereState = MACRO_I3HERE_DRV;

	extension->b1stHiddenInstrInfoIsValid = FALSE;
	extension->dw1stHiddenInstrAddress = 0;

	extension->bReportMode = FALSE;

	// Initialize the Initialization Flag.

	extension->bInitializationDone = FALSE;

	// Initialize the pointer to the Text Video Buffer.

	extension->dglLayouts.pvTextVideoBuffer = NULL;

#ifdef USE_PAGETABLE_TO_OBTAIN_TEXTMODE_VIDEOADDRESS
	PhysAddressToLinearAddresses( & extension->pvTextVideoBuffer, 1, NULL, 0xB8000 );
#else
	{
		PHYSICAL_ADDRESS		paPhysAddr;
		paPhysAddr.QuadPart = 0xB8000;
		extension->dglLayouts.pvTextVideoBuffer = MmMapIoSpace( paPhysAddr, 1, MmNonCached );
	}
#endif

	// Initialize the memory associated to the Script Window.

	extension->ulScriptWinBufferSizeInBytes = 48 * 1024;
	extension->pwScriptWinBuffer = (WORD*) ExAllocatePool( NonPagedPool, extension->ulScriptWinBufferSizeInBytes );

	extension->ulScriptWinStrPtrsBufferSizeInBytes = 2 * 1024;
	extension->ppwScriptWinStrPtrsBuffer = (WORD**) ExAllocatePool( NonPagedPool, extension->ulScriptWinStrPtrsBufferSizeInBytes );

	extension->bScriptWinDirtyBit = FALSE;

	extension->ulScriptWinLineIndex = 0xFFFFFFFF;
	for ( ulI = 0; ulI < MACRO_SCRIPTWIN_LINESIZE_IN_CHARS; ulI ++ )
		extension->vwScriptWinLine[ ulI ] = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	extension->ulScriptWinBufferPosInBytes = 0;
	extension->ulScriptWinStrPtrsBufferPosInBytes = 0;

	extension->ulScriptWinLn = 1;
	extension->ulScriptWinCol = 1;

	extension->ulScriptWinOffsetX = 0;
	extension->ulScriptWinOffsetY = 0;

	extension->bScriptWinShowSelArea = FALSE;

	// Initialize the memory associated to the Script Window Clipboard.

	extension->ulScriptWinClipBufferSizeInBytes = 24 * 1024;
	extension->pwScriptWinClipBuffer = (WORD*) ExAllocatePool( NonPagedPool, extension->ulScriptWinClipBufferSizeInBytes );

	extension->ulScriptWinClipStrPtrsBufferSizeInBytes = 2 * 1024;
	extension->ppwScriptWinClipStrPtrsBuffer = (WORD**) ExAllocatePool( NonPagedPool, extension->ulScriptWinClipStrPtrsBufferSizeInBytes );

	extension->ulScriptWinClipBufferPosInBytes = 0;
	extension->ulScriptWinClipStrPtrsBufferPosInBytes = 0;

	// Initialize the memory associated to the Compilation Buffer.

	extension->ulScriptWinCompilationBufferSizeInBytes = extension->ulScriptWinBufferSizeInBytes / sizeof( WORD );
	extension->pszScriptWinCompilationBuffer = (CHAR*) ExAllocatePool( NonPagedPool, extension->ulScriptWinCompilationBufferSizeInBytes );

	if ( extension->pszScriptWinCompilationBuffer )
		* extension->pszScriptWinCompilationBuffer = 0;

	extension->ulScriptWinObjectFileBufferSizeInBytes = 64 * 1024;
	extension->pbScriptWinObjectFileBuffer = (BYTE*) ExAllocatePool( NonPagedPool, extension->ulScriptWinObjectFileBufferSizeInBytes );

	extension->ulScriptWinObjectFileBufferUsedBytes = 0;

	// Initialize the memory associated to the Output Window.

	extension->ulHistoryLineID = 0;

	extension->ulLineOffsetInOutputWin = 0;
	extension->ulMruCommandOffsetFromEnd = 0;

	InitializeMultiProcessorSpinLock( & extension->mpslOutputPrintFunctionMutex );

	extension->ulOutputWinBufferSizeInBytes = 48 * 1024;
	extension->pszOutputWinBuffer = (CHAR*) ExAllocatePool( NonPagedPool, extension->ulOutputWinBufferSizeInBytes );

	extension->ulOutputWinStrPtrsBufferSizeInBytes = 2 * 1024;
	extension->ppszOutputWinStrPtrsBuffer = (CHAR**) ExAllocatePool( NonPagedPool, extension->ulOutputWinStrPtrsBufferSizeInBytes );

	extension->ulOutputWinBufferPosInBytes = 0;
	extension->ulOutputWinStrPtrsBufferPosInBytes = 0;

	// Check whether the Memory was Allocated.

	if ( extension->pwScriptWinBuffer && extension->ppwScriptWinStrPtrsBuffer &&
		extension->pwScriptWinClipBuffer && extension->ppwScriptWinClipStrPtrsBuffer &&
		extension->pszOutputWinBuffer && extension->ppszOutputWinStrPtrsBuffer &&
		extension->pszScriptWinCompilationBuffer && extension->pbScriptWinObjectFileBuffer )
	{
		// Set the Variables.

		g_bCanOutputPrint = TRUE;
		bCanContinue = TRUE;

		// Print the Version Message.

		OutputVersionMessage();

		CheckedBuild = PsGetVersion(
			& MajorVersion,
			& MinorVersion,
			& BuildNumber,
			NULL );

		VmWareVersion = IsInsideVmware ();
		sprintf( szVmware, "VmWare_Version_%i", VmWareVersion );

		OutputPrint( FALSE, FALSE, "Microsoft(r) Windows(r) NT Version %i.%i.%i.%s.%s.", MajorVersion, MinorVersion, BuildNumber, CheckedBuild ? "Checked_Build" : "Free_Build", VmWareVersion ? szVmware : "Native_No_VmWare" );

		OutputPrint( FALSE, TRUE, "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD" );
	}
	else
	{
		bCanContinue = FALSE;
	}

	// Try Opening the "BugChk.Dat" settings file...

	extension->ulBugChkDatSize = 0;

	extension->pbBugChkDat = LoadFile(
		MACRO_BUGCHKDAT_COMPLETE_PATH_U, NonPagedPool, & extension->ulBugChkDatSize ); // <-- NonPagedPool important here !!! !!!

	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Settings File: \"%s\": %s.",
		MACRO_BUGCHKDAT_COMPLETE_PATH, extension->pbBugChkDat ? "Loaded" : "Failed to load" );

	// Read the OS-Specific settings.

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_PCR_ADDRESS_OF_1ST_PROCESSOR_N, "" );
	if ( strlen( psz ) ) { MACRO_PCR_ADDRESS_OF_1ST_PROCESSOR = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 16 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: pcr_address_of_1st_processor set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_KTEBPTR_FIELDOFFSET_IN_PCR_N, "" );
	if ( strlen( psz ) ) { MACRO_KTEBPTR_FIELDOFFSET_IN_PCR = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: ktebptr_fieldoffset_in_pcr set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_KPEBPTR_FIELDOFFSET_IN_KTEB_N, "" );
	if ( strlen( psz ) ) { MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: kpebptr_fieldoffset_in_kteb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IMGFLNAME_FIELDOFFSET_IN_KPEB_N, "" );
	if ( strlen( psz ) ) { MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: imgflname_fieldoffset_in_kpeb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_CURRIRQL_FIELDOFFSET_IN_PCR_N, "" );
	if ( strlen( psz ) ) { MACRO_CURRIRQL_FIELDOFFSET_IN_PCR = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: currirql_fieldoffset_in_pcr set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_TID_FIELDOFFSET_IN_KTEB_N, "" );
	if ( strlen( psz ) ) { MACRO_TID_FIELDOFFSET_IN_KTEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: tid_fieldoffset_in_kteb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_PID_FIELDOFFSET_IN_KPEB_N, "" );
	if ( strlen( psz ) ) { MACRO_PID_FIELDOFFSET_IN_KPEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: pid_fieldoffset_in_kpeb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IMAGEBASE_FIELDOFFSET_IN_DRVSEC_N, "" );
	if ( strlen( psz ) ) { MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: imagebase_fieldoffset_in_drvsec set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IMAGENAME_FIELDOFFSET_IN_DRVSEC_N, "" );
	if ( strlen( psz ) ) { MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: imagename_fieldoffset_in_drvsec set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_NTOSKRNL_MODULENAME_UPPERCASE_N, "" );
	if ( strlen( psz ) ) { MACRO_NTOSKRNL_MODULENAME_UPPERCASE = psz; OutputPrint( FALSE, FALSE, "BugChk.Dat: ntoskrnl_modulename_uppercase set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_VADROOT_FIELDOFFSET_IN_KPEB_N, "" );
	if ( strlen( psz ) ) { MACRO_VADROOT_FIELDOFFSET_IN_KPEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: vadroot_fieldoffset_in_kpeb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_VADTREE_UNDOCUMENTED_DISP_0_N, "" );
	if ( strlen( psz ) ) { MACRO_VADTREE_UNDOCUMENTED_DISP_0 = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: vadtree_undocumented_disp_0 set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_VADTREE_UNDOCUMENTED_DISP_1_N, "" );
	if ( strlen( psz ) ) { MACRO_VADTREE_UNDOCUMENTED_DISP_1 = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: vadtree_undocumented_disp_1 set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_STARTPARAM_N, "" );
	if ( strlen( psz ) ) { MACRO_MAPVIEWOFIMAGESECTION_STARTPARAM = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: mapviewofimagesection_startparam set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_SIZEPARAM_N, "" );
	if ( strlen( psz ) ) { MACRO_MAPVIEWOFIMAGESECTION_SIZEPARAM = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: mapviewofimagesection_sizeparam set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_UNMAPVIEWOFIMAGESECTION_STARTPARAM_N, "" );
	if ( strlen( psz ) ) { MACRO_UNMAPVIEWOFIMAGESECTION_STARTPARAM = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: unmapviewofimagesection_startparam set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_KPEBPARAM_N, "" );
	if ( strlen( psz ) ) { MACRO_MAPVIEWOFIMAGESECTION_KPEBPARAM = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: mapviewofimagesection_kpebparam set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_UNMAPVIEWOFIMAGESECTION_KPEBPARAM_N, "" );
	if ( strlen( psz ) ) { MACRO_UNMAPVIEWOFIMAGESECTION_KPEBPARAM = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: unmapviewofimagesection_kpebparam set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IDLEPROCESS_OFFSET_REL_INITSYSP_N, "" );
	if ( strlen( psz ) ) { MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: idleprocess_offset_rel_initsysp set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IDLEPROCESS_IMAGEFILENAME_N, "" );
	if ( strlen( psz ) ) { MACRO_IDLEPROCESS_IMAGEFILENAME = psz; OutputPrint( FALSE, FALSE, "BugChk.Dat: idleprocess_imagefilename set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB_N, "" );
	if ( strlen( psz ) ) { MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: actvproclinks_fieldoffset_in_kpeb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_CR3_FIELDOFFSET_IN_KPEB_N, "" );
	if ( strlen( psz ) ) { MACRO_CR3_FIELDOFFSET_IN_KPEB = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: cr3_fieldoffset_in_kpeb set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_MAXVALUE_FOR_WINNT_SEGSELECTOR_N, "" );
	if ( strlen( psz ) ) { MACRO_MAXVALUE_FOR_WINNT_SEGSELECTOR = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: maxvalue_for_winnt_segselector set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_ADDR_N, "" );
	if ( strlen( psz ) ) { MACRO_MAPVIEWOFIMAGESECTION_ADDR = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: mapviewofimagesection_addr set to %s.", psz ); }

	psz = GetSfSetting( extension->pbBugChkDat, extension->ulBugChkDatSize, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_NTTERMINATEPROCESS_FNINDEX_N, "" );
	if ( strlen( psz ) ) { MACRO_NTTERMINATEPROCESS_FNINDEX = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ); OutputPrint( FALSE, FALSE, "BugChk.Dat: ntterminateprocess_fnindex set to %s.", psz ); }

	// Allocate the Symbols Memory.

	extension->dwSymLoaderCounter = 1;
	extension->dwSymLoaderToken = 0;
	strcpy( extension->szSymLoaderPendingTblPath, "" );

	extension->ulSymTablesNum = 0;

	psz = GetSfSetting(
		extension->pbBugChkDat, extension->ulBugChkDatSize,
		MACRO_BUGCHKDAT_MEM_N, MACRO_BUGCHKDAT_MEM_SYMBOLS_N, MACRO_BUGCHKDAT_MEM_SYMBOLS_DEFAULT );

	extension->ulSymMemorySize = MACRO_CRTFN_NAME(strtoul)( psz, NULL, 10 ) * 1024;
	extension->pbSymMemory = (BYTE*) ExAllocatePool( NonPagedPool, extension->ulSymMemorySize );

	if ( extension->pbSymMemory == NULL )
		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": ## FAILED ## to Allocate %iKb for Symbols.", extension->ulSymMemorySize / 1024 );
	else
		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Allocated %iKb for Symbols.", extension->ulSymMemorySize / 1024 );

	// Read the g_pvMmUserProbeAddress Value.

	g_pvMmUserProbeAddress = (PVOID) MM_USER_PROBE_ADDRESS;

	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": User Probe Address is \"0x%.8X\".", g_pvMmUserProbeAddress );

	// Read the Number of Processors.

	g_dwNumberProcessors = (DWORD) ( KeNumberProcessors );

	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Number of Processors in the System: %i.", g_dwNumberProcessors );

	// Try to determine the Driver Section of the NTOSKNRL Module.

	if ( bCanContinue )
	{
		extension->pvNtoskrnlDriverSection = DiscoverNtoskrnlDriverSection( DriverObject->DriverSection );

		if ( extension->pvNtoskrnlDriverSection == NULL )
			bCanContinue = FALSE;
	}

	// Try to Acquire the Informations about the Process List.

	if ( bCanContinue )
	{
		if ( GetProcessListInfo( & extension->pliProcessListInfo ) == FALSE )
		{
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Unable to Determine the Informations about the Active Process List. Aborting..." );
			bCanContinue = FALSE;
		}
		else
		{
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Active Process List: ProcessListHead = %.8X, IdleProcess = %.8X.",
				(DWORD) extension->pliProcessListInfo.pleProcessListHead, (DWORD) extension->pliProcessListInfo.pbIdleProcess );
		}
	}

	// Allocate the Memory for the MiniC Compiler.

	if ( bCanContinue )
	{
		extension->ulMemFileMemoryStep = MACRO_MEMFILE_RESERVDMEM_STEP;
		extension->ulMemFileMemoryLength = MACRO_MEMFILE_RESERVDMEM_SIZE;
		extension->pvMemFileMemory = ExAllocatePool( NonPagedPool, MACRO_MEMFILE_RESERVDMEM_SIZE );

		if ( extension->pvMemFileMemory == NULL )
			bCanContinue = FALSE;
	}

	// Try to Initialize the MiniC Compiler.

	if ( bCanContinue )
	{
		if ( InitializeMiniCCompiler() == FALSE )
			bCanContinue = FALSE;
		else
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Allocated %i KBytes for MiniC Compiler.",
				( (ULONG) compiler_memory ) / 1024 );
	}

	// Allocate the Memory for the VPCICE Stack.

	if ( bCanContinue )
	{
		extension->pbStackBase = (BYTE*) ExAllocatePool( NonPagedPool, MACRO_VPCICE_SYSSTACK_SIZE );

		if ( extension->pbStackBase == NULL )
		{
			bCanContinue = FALSE;
		}
		else
		{
			extension->pbStackPointerStart = extension->pbStackBase + MACRO_VPCICE_SYSSTACK_SIZE;
			extension->dwPrevStackPointer = 0;

			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Allocated %i Bytes for " MACRO_PROGRAM_NAME " System Stack.", MACRO_VPCICE_SYSSTACK_SIZE );
		}
	}

	// Try to Determine the Address of Several Critical System Functions.

	if ( bCanContinue )
	{
		// Try to determine the Address of the Functions.

		extension->pvMapViewOfImageSectionFnPtr = Guess_MiMapViewOfImageSection_FnPtr( extension->pvNtoskrnlDriverSection );
		extension->pvUnMapViewOfImageSectionFnPtr = Guess_MiUnMapViewOfImageSection_FnPtr();
		ulNtTerminateProcessFnIndex = Get_NtTerminateProcess_FnIndex();

		// Print an Information Massage.

		if ( extension->pvMapViewOfImageSectionFnPtr == NULL )
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": ### WARNING! ### Unable to Determine \"MapViewOfImageSection\" Entry Point." );
		else
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": \"MapViewOfImageSection\" Entry Point located at %X.", extension->pvMapViewOfImageSectionFnPtr );

		if ( extension->pvUnMapViewOfImageSectionFnPtr == NULL )
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": ### WARNING! ### Unable to Determine \"UnMapViewOfImageSection\" Entry Point." );
		else
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": \"UnMapViewOfImageSection\" Entry Point located at %X.", extension->pvUnMapViewOfImageSectionFnPtr );

		if ( ulNtTerminateProcessFnIndex == MACRO_GETNTTERMINATEPROCESSFNINDEX_ERR )
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": ### WARNING! ### Unable to Determine \"NtTerminateProcess\" System Table Index." );
		else
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": \"NtTerminateProcess\" System Table Index located at %X.", ulNtTerminateProcessFnIndex );
	}

	// Subscribe ourselves to the VPC Video Hook Driver...

	if ( bCanContinue )
	{
		nsVpcICEvdSubscrRes = SubscribeVideoMemoryInfoBuffer( & extension->pvmiVideoMemoryInfo );

		if ( nsVpcICEvdSubscrRes == STATUS_SUCCESS )
		{
			// Initialize the structure used for keeping track of Video Memory Info modifications.

			extension->vmiPrevVideoMemoryInfo = * extension->pvmiVideoMemoryInfo;

			// Set the initial dimensions of the VpcICE Console.

			ulInitialWidth = MACRO_CONSOLE_DEFAULT_WIDTH;
			ulInitialHeight = MACRO_CONSOLE_DEFAULT_HEIGHT;

			if ( extension->pvmiVideoMemoryInfo->pvPrimary )
			{
				ulInitialWidth += 2;
				ulInitialHeight += 2;
			}

			// Initialize the memory reserved to the VpcICE Video.

			extension->ulVpcICEVideoBufferSizeInBytes = 2 * 1024 * 1024;
			extension->pvVpcICEVideoBuffer = ExAllocatePool( NonPagedPool, extension->ulVpcICEVideoBufferSizeInBytes );

			if ( extension->pvVpcICEVideoBuffer )
			{
				// Initialize the structure describing the Video Memory Layout of VpcICE.

				nsVpcICEVidMemInitRes = InitializeVpcICEVideoMemoryStructure( & extension->dglLayouts.vivmVpcICEVideo,
					extension->pvmiVideoMemoryInfo,
					extension->pvVpcICEVideoBuffer, extension->ulVpcICEVideoBufferSizeInBytes,
					ulInitialWidth, ulInitialHeight );

				if ( nsVpcICEVidMemInitRes != STATUS_SUCCESS )
				{
					nsVpcICEVidMemInitRes = InitializeVpcICEVideoMemoryStructure( & extension->dglLayouts.vivmVpcICEVideo,
						extension->pvmiVideoMemoryInfo,
						extension->pvVpcICEVideoBuffer, extension->ulVpcICEVideoBufferSizeInBytes,
						MACRO_CONSOLE_DEFAULT_WIDTH_MIN, MACRO_CONSOLE_DEFAULT_HEIGHT_MIN );
				}

				if ( nsVpcICEVidMemInitRes == STATUS_SUCCESS )
				{
					// Initialize the memory of the Text Front and Back Buffer.

					memset( extension->dglLayouts.vivmVpcICEVideo.pwTextFrontBuffer, 0, extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersSizeInBytes );
					memset( extension->dglLayouts.vivmVpcICEVideo.pwTextBackBuffer, 0, extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersSizeInBytes );

					// Calculate the Processor Speed.

					CalculateCPUSpeed( & extension->liCpuCyclesPerSecond );

					extension->liCursorCpuCycles.QuadPart =
						extension->liCpuCyclesPerSecond.QuadPart / MACRO_CURSORTIME_FACTOR;

					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Pentium TSC Calibration result: %i MHz.",
						(ULONG) ( extension->liCpuCyclesPerSecond.QuadPart / 1000000 ) );

					// Install the VpcICE Interrupt Hooks.

					nsVpcICEIntHooksInstallRes = InstallVpcICEInterruptHooks( & extension->sisSysInterrStatus );

					if ( nsVpcICEIntHooksInstallRes != STATUS_SUCCESS )
					{
						// Print a Debug Message.

						OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Install" MACRO_PROGRAM_NAME "InterruptHooks failed." );
					}
					else
					{
						// Can open the Debugger.

						bCanOpenDebuggerForInitialization = TRUE;
					}
				}
				else
				{
					// Print a Debug Message.

					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Initialization of " MACRO_PROGRAM_NAME " Video Structure failed." );
				}
			}
			else
			{
				// Print a Debug Message.

				OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Allocation of " MACRO_PROGRAM_NAME " Video Buffer failed." );
			}
		}
		else
		{
			// Print a Debug Message.

			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": SubscribeVideoMemoryInfoBuffer failed." );
		}
	}

	// Open the Debugger.

	if ( bCanOpenDebuggerForInitialization )
	{
		bCanContinue = TRUE;

		// Init some Variables.

		ExInitializeFastMutex( & extension->sisSysInterrStatus.fmVpcICEProcessCallbackAccess );

		// Set some Variables.

		extension->sisSysInterrStatus.ulNtTerminateProcessFnIndex = ulNtTerminateProcessFnIndex;

		extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresSize =
			// GetUserSpaceTrampolineStructuresSize( MACRO_MAXNUM_OF_DETOURS ); // DETOURLESS
			GetUserSpaceMemorySize();
		extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresAddress =
			MACRO_USRSPACE_TRAMPS_VIRTADDR;

		extension->sisSysInterrStatus.liTimerThreadWaitInterval.QuadPart = MACRO_TIMERTHREAD_DEFAULT_INTERVAL;

		// Print some log informations.

		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Debugger User Space Memory: Address: %X, Size: %X.",
			extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresAddress,
			extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresSize );

		// Allocate the User Space Trampoline Structures in all the system processes.

		AllocateUserSpaceTrampolineStructuresInAllProcesses();

		// Set the "MouseEnabled" Variable.

		extension->sisSysInterrStatus.bMouseEnabled = ( VmWareVersion || _8042MinimalProgramming ) ? FALSE : TRUE;

		// Install the Process Creation Notification callback.

		if ( bCanContinue )
		{
			ntStatus = PsSetCreateProcessNotifyRoutine( & VpcICEProcessCallback, 0 );

			if ( ntStatus != STATUS_SUCCESS )
				bCanContinue = FALSE;
		}

		// Hook the System Functions of Interest for the Debugger.

		if ( bCanContinue )
		{
			if ( extension->pvMapViewOfImageSectionFnPtr )
			{
				if ( MajorVersion != 5 || MinorVersion != 0 )
				{
					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": =FIXME= MapViewOfImageSection signature changed in WinXP. Unable to hook." );
				}
				else if ( InstallDetour( & extension->sisSysInterrStatus, MACRO_DETOURTYPE_SYS_MAPVIEWOFIMAGESECTION,
					(DWORD) extension->pvMapViewOfImageSectionFnPtr, (VOID*) & NEW_MapViewOfImageSection, TRUE, NULL, NULL ) == FALSE )
				{
					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Failed to Hook \"MapViewOfImageSection\". Aborting." );
					bCanContinue = FALSE;
				}
			}

			if ( extension->pvUnMapViewOfImageSectionFnPtr )
			{
				if ( InstallDetour( & extension->sisSysInterrStatus, MACRO_DETOURTYPE_SYS_UNMAPVIEWOFIMAGESECTION,
					(DWORD) extension->pvUnMapViewOfImageSectionFnPtr, (VOID*) & NEW_UnMapViewOfImageSection, TRUE, NULL, NULL ) == FALSE )
				{
					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Failed to Hook \"UnMapViewOfImageSection\". Aborting." );
					bCanContinue = FALSE;
				}
			}
		}

		// Print a Delimiter.

		OutputPrint( FALSE, TRUE, "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD" );

		// Open the Debugger.

		if ( bCanContinue == FALSE )
		{
			extension->sisSysInterrStatus.bDetoursInitFatalError = TRUE;
		}
		else
		{
			//
			// Initialize the Text of the Script Window.
			//

			InitializeScriptFileText();

			//
			// Enter into the Debugger for a limited amount of Time.
			//  This will allow IDT Hook installations etc. in a Controlled Environment.
			//

			__asm
			{
				// ENTERING into the DEBUGGER.

				ENTERING_DEBUGGER

				// Push.

				pushad

				// Set the Parameters of the Function.

				mov			edx, extension
				lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus

				mov			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, TRUE

				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EAX, eax
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EBX, ebx
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ECX, ecx
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EDX, edx

				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESI, esi
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EDI, edi

				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EBP, ebp
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESP, esp

				pushfd
				pop			eax

				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EFLAGS, eax

				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.CS, cs
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.DS, ds
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.SS, ss
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ES, es
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.FS, fs
				mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.GS, gs

				// Open the Debugger.

				lea			eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EIP

				cli
				call		InvokeDebuggerSettingEIP
				sti

				// Pop.

				popad

				// EXITING from the DEBUGGER.

				EXITING_DEBUGGER(0)
			}

			//
			// Check whether we can Create the Timer Thread.
			//

			if ( extension->sisSysInterrStatus.bTimerThreadCreationAllowed )
			{
				ntStatus = PsCreateSystemThread( & extension->sisSysInterrStatus.hTimerThreadHandle,
					(ACCESS_MASK) 0L,
					NULL,
					NULL,
					NULL,
					& TimerThreadEntryPoint,
					NULL );

				if ( ntStatus != STATUS_SUCCESS )
					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": ### WARNING! ### Timer Thread creation failed! (NTSTATUS=%.8X)", (DWORD) ntStatus );
			}
		}
	}

	// Return Success.

	return STATUS_SUCCESS;
}

//================================================
// VPCIceDispatchCreate Function Definition.
//================================================

NTSTATUS VPCIceDispatchCreate( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
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
// VPCIceDispatchClose Function Definition.
//===============================================

NTSTATUS VPCIceDispatchClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

	// Return to the caller

	Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}

//===============================================
// VPCIceDispatchIoctl Function Definition.
//===============================================

NTSTATUS VPCIceDispatchIoctl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    NTSTATUS              ntStatus = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION    irpStack  = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION     extension = DeviceObject->DeviceExtension;

	DWORD*                pl;

	// IOCTL Processor Switch.

	switch( irpStack->Parameters.DeviceIoControl.IoControlCode )
	{
		//-----------------------------------------------------------------------------------

		case IOCTL_VPCICE_GET_VERSION:
			{
				// Return the information required.

				if ( irpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof( DWORD ) )
				{
					// Copy the information in the system buffer.

					pl = Irp->AssociatedIrp.SystemBuffer;
					*pl = ( DWORD ) _VPCICE_DRIVER_VERSION_;

					// Everything went ok.

					ntStatus = STATUS_SUCCESS;
				}
			}
			break;

		//-----------------------------------------------------------------------------------

		case IOCTL_VPCICE_COMPLETE_SYMLOAD:
			{
				DWORD			dwToken = 0;
				BOOLEAN			res;

				if ( irpStack->Parameters.DeviceIoControl.InputBufferLength == sizeof( dwToken ) )
				{
					dwToken = * (DWORD*) Irp->AssociatedIrp.SystemBuffer;
					if ( dwToken )
					{
						// Call the Implementation.

						res = CompleteSymTableLoad( dwToken );

						// Set the Return Value.

						ntStatus =
							( res ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL );
					}
				}
			}
			break;

		//-----------------------------------------------------------------------------------

        default:
            break;

		//-----------------------------------------------------------------------------------
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

//================================================
// DebuggerInvokedCore Function Definition.
//================================================

VOID DebuggerInvokedCore( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	NTSTATUS				nsConStrupRes;
	DWORD					dwEIP;
	BOOLEAN					bDetAndBreakpEnableReq = FALSE;
	BOOLEAN					bIsUserReturning = extension->sisSysInterrStatus.bIsSingleStepping;
	BOOLEAN					bAvoidSettingKeybLEDsAndMouse = extension->sisSysInterrStatus.bIsSingleStepping;
	BOOLEAN					bIsConsoleRestoreRequired = FALSE;
	BOOLEAN					bRestoreKeybLEDsAndMouse = FALSE;
	ULONG					ulI;
	BREAKPOINT*				pbThis;
	DETOUR*					pdThis;
	BOOLEAN					bEnterDebugger = TRUE;
	VOID*					pvAddress;

	//
	// Manage the Debugger Verb counter accuracy.
	//

	if ( extension->dwLastEnterTscHigh || extension->dwLastEnterTscLow )
	{
		__int64			enter, exit, delta;

		* (DWORD*) & enter = extension->dwLastEnterTscLow;
		* ( (DWORD*) & enter + 1 ) = extension->dwLastEnterTscHigh;

		* (DWORD*) & exit = g_dwExitTscLow;
		* ( (DWORD*) & exit + 1 ) = g_dwExitTscHigh;

		delta = exit - enter;

		* (__int64*) & extension->liLastDebuggerVerbReadTime += delta;
	}

	extension->dwLastEnterTscHigh = g_dwEnterTscHigh;
	extension->dwLastEnterTscLow = g_dwEnterTscLow;

	// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
	//		HANDLE FIRST-CHANCE USER MODE REQUESTS...
	//			THE CONTEXT IN WHICH FC REQUESTS ARE PROCESSED IS NOT THE DEBUGGER ONE:
	//			IN FACT THE INTERRUPTS ARE NOT MASKED AND NO IPI INTERRUPT IS GENERATED.
	//			THIS ALLOWS A MORE QUICK AND SAFE DELIVERING OF THE REQUEST RESPONSE.
	// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

	if ( extension->sisSysInterrStatus.bFromInt3 &&
		AddressIsUser2KernelGate( extension->sisSysInterrStatus.dwInt3OpcodeInstructionPtr ) )
	{
		BOOLEAN		bContinue = HandleUser2KernelBkp(
			& extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext,
			TRUE );

		if ( bContinue == FALSE )
		{
			extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext.EIP +=
				MACRO_U2KGUID_LEN;

			return;
		}
	}

	// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
	// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
	// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

	// ### DISABLE INTERRUPTS ###

	if ( extension->sisSysInterrStatus.bIsSingleStepping == FALSE )
	{
		if ( extension->sisSysInterrStatus.bEOIRequired )
		{
			if ( extension->sisSysInterrStatus.pvLocalApicMemoryPtr == NULL )
				SendEOIToPIC1();
			else
				SendEOIToLocalAPIC( & extension->sisSysInterrStatus );

			extension->sisSysInterrStatus.bEOIRequired = FALSE;
		}

		MaskInterrupts( 0xF9, 0xEF, & extension->sisSysInterrStatus ); // ### NOTE: Keyboard Interrupt is LEFT ENABLED. ###

		if ( extension->sisSysInterrStatus.pvLocalApicMemoryPtr )
		{
			// ### TASK PRIORITY SET + IPI SEND CODE AND SYNCHRONIZATION ###

			// We are sending IPIs...

			extension->sisSysInterrStatus.bSTIinIPIs = FALSE;
			extension->sisSysInterrStatus.bIsSendingIPIs = TRUE;

			// Set the Processor Task Priority Register.

			SetLocalAPIC_TPR( 0xEF, & extension->sisSysInterrStatus );

			// Send the VpcICE IPI and Wait.

			SendVpcICEIPI( & extension->sisSysInterrStatus );

			__asm
			{
				// Push.

				push		eax
				push		ecx

				// Pointers.

				mov			eax, extension
				lea			eax, [ eax ]DEVICE_EXTENSION.sisSysInterrStatus

	_IpiLoop:
				// Enter.

				push		ebx
				lea			ebx, [ eax ]SYSINTERRUPTS_STATUS.mpslIpiSpinLock
				call		EnterMultiProcessorSpinLock
				pop			ebx

				// Get.

				mov			ecx, [ eax ]SYSINTERRUPTS_STATUS.dwNumProcsWithContextFilled

				// Leave.

				push		ebx
				lea			ebx, [ eax ]SYSINTERRUPTS_STATUS.mpslIpiSpinLock
				call		LeaveMultiProcessorSpinLock
				pop			ebx

				// Compare.

				cmp			ecx, g_dwNumberProcessors
				jne			_IpiLoop

				// Pop.

				pop			ecx
				pop			eax
			}

			// IPIs Sending is Over...

			extension->sisSysInterrStatus.bIsSendingIPIs = FALSE;
		}

		ReorderListOfProcessors( & extension->sisSysInterrStatus );
	}

	// ### CPU INITIALIZATION ###

	if ( extension->bInitializationDone == FALSE )
		InitializeProcessorForDebugging();

	// ### IDT HOOKS INSTALLATION ###

	if ( extension->sisSysInterrStatus.bIdtHooksInstalled == FALSE )
	{
		if ( InstallIDTHook( & extension->sisSysInterrStatus, 0x1, (DWORD) & VpcICEDebugExceptionHandler ) == FALSE ||
			InstallIDTHook( & extension->sisSysInterrStatus, 0x3, (DWORD) & VpcICEBreakpointExceptionHandler ) == FALSE ||
			InstallIDTHook( & extension->sisSysInterrStatus, 0x2E, (DWORD) & VpcICESystemServiceInterruptHandler ) == FALSE )
		{
			extension->sisSysInterrStatus.bIdtHooksError = TRUE;
		}

		extension->sisSysInterrStatus.bIdtHooksInstalled = TRUE;
	}

	if ( extension->sisSysInterrStatus.bIdtHooksError == FALSE &&
		extension->sisSysInterrStatus.bDetoursInitFatalError == FALSE )
	{
		// ### GLOBAL INITIALIZATION ###

		strcpy( g_szAutoTypedCommand, "" );

		extension->sisSysInterrStatus.bINT1RequiresCodeWinPosDec = FALSE;

		InitEnableXXXCache();

		if ( extension->bInitializationDone == FALSE )
		{
			// Install the Detours.

			EnableDetoursAndBreakpoints( & extension->sisSysInterrStatus, TRUE );
		}
		else
		{
			// Disable the Detours and the Breakpoints in the Environment.

			if ( extension->sisSysInterrStatus.ulSysRestoreAction != MACRO_SYSREST_CCOPREST &&
				extension->sisSysInterrStatus.ulSysRestoreAction != MACRO_SYSREST_CCOPREST_SINGSTP &&
				extension->sisSysInterrStatus.bIsSingleStepping == FALSE )
			{
				//
				// Disable the Detours/Breakpoints.
				//

				EnableDetoursAndBreakpoints( & extension->sisSysInterrStatus, FALSE );

				//
				// SPECIAL CONDITION: we are here because of a "Stale" Hardcoded Breakpoint in a Cached Page Frame.
				//  Resume the execution without Entering in the Debugger.
				//

				if ( extension->sisSysInterrStatus.bResumeExecBecauseOfStaleBpInPf )
					bEnterDebugger = FALSE;

				//
				// SPECIAL CONDITION: the Timer Thread is entering in the Debugger. This allows to keep the
				//  Page Frame Modifications DB as accurate as possible.
				//

				if ( extension->sisSysInterrStatus.bTimerThreadEventOccurred )
				{
					// Don't Enter in the Debugger.
					bEnterDebugger = FALSE;

					// Handle the Request.
					HandleTimerThreadEvent(
						& extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext );
				}

				//
				// SPECIAL CONDITION: a "User2Kernel" Gate Breakpoint occurred.
				//

				if ( extension->sisSysInterrStatus.bUser2KernelGateBkpOccurred )
				{
					// Handle the Request.

					bEnterDebugger = HandleUser2KernelBkp(
						& extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext,
						FALSE );
				}
			}

			bDetAndBreakpEnableReq = TRUE;
		}

		// ### SET STEPPING VAR TO FALSE ###

		extension->sisSysInterrStatus.bIsSingleStepping = FALSE;

		// ### CHECK WHETHER ENTERING IN THE DEBUGGER IS DESIRED ###

		extension->sisSysInterrStatus.pbpUniqueHitBreakpoint = NULL;
		extension->sisSysInterrStatus.pdUniqueHitDetour = NULL;

		if (
			( extension->sisSysInterrStatus.bFromInt1 || extension->sisSysInterrStatus.bFromInt3 ) &&
			( extension->sisSysInterrStatus.ulDiscoveredBreakpointsNum || extension->sisSysInterrStatus.ulDiscoveredDetoursNum )
			)
		{
			// Check whether there is an Hit Detour.

			for ( ulI = 0; ulI < extension->sisSysInterrStatus.ulDiscoveredDetoursNum; ulI ++ )
			{
				pdThis = extension->sisSysInterrStatus.vpdDiscoveredDetours[ ulI ];
				if ( pdThis->bIsContextCompatible )
				{
					extension->sisSysInterrStatus.pdUniqueHitDetour = pdThis;
					break;
				}
			}

			// Check whether there is an Hit Breakpoint.

			for ( ulI = 0; ulI < extension->sisSysInterrStatus.ulDiscoveredBreakpointsNum; ulI ++ )
			{
				pbThis = extension->sisSysInterrStatus.vpbpDiscoveredBreakpoints[ ulI ];
				if ( pbThis->bIsContextCompatible )
				{
					extension->sisSysInterrStatus.pbpUniqueHitBreakpoint = pbThis;
					break;
				}
			}

			// Check whether we have to enter in the Debugger.

			if ( extension->sisSysInterrStatus.pbpUniqueHitBreakpoint == NULL &&
				extension->sisSysInterrStatus.pdUniqueHitDetour == NULL )
			{
				bEnterDebugger = FALSE;
			}

			//
			// Check whether We have to Check Out Special Conditions related to the Hardware Breakpoints.
			//

			if ( extension->sisSysInterrStatus.bFromInt1 &&
				extension->sisSysInterrStatus.pbpUniqueHitBreakpoint &&
				extension->sisSysInterrStatus.pbpUniqueHitBreakpoint->bIsUsingDebugRegisters )
			{
				switch( extension->sisSysInterrStatus.pbpUniqueHitBreakpoint->ulType )
				{
				case MACRO_BREAKPOINTTYPE_MEMORY:

					bEnterDebugger = CheckIfMemoryBreakpointIsCompatible( extension->sisSysInterrStatus.pbpUniqueHitBreakpoint );

					break;

				case MACRO_BREAKPOINTTYPE_IO:

					bEnterDebugger = CheckIfIoBreakpointIsCompatible( extension->sisSysInterrStatus.pbpUniqueHitBreakpoint );

					break;
				}
			}
		}

		// ### DRAW THE VPCICE INTERFACE ###

		if ( bEnterDebugger &&
			extension->sisSysInterrStatus.bTransferControlToOriginalInt3 == FALSE &&
			extension->sisSysInterrStatus.ulSysRestoreAction != MACRO_SYSREST_CCOPREST )
		{
			// ### ENVIRONMENT INITIALIZATION ###

			// Code Windows Variables.

			dwEIP = extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext.EIP;

			if ( extension->b1stHiddenInstrInfoIsValid == FALSE ||
				( dwEIP < extension->dwCodeWindowPos || dwEIP >= extension->dw1stHiddenInstrAddress ) )
			{
				extension->dwCodeWindowPos = dwEIP;

				if ( extension->sisSysInterrStatus.bINT3RequiresCodeWinEipDec )
					extension->dwCodeWindowPos --;
			}
			else if ( extension->sisSysInterrStatus.bINT1RequiresCodeWinPosDec &&
				dwEIP != extension->dwCodeWindowPos )
			{
				extension->sisSysInterrStatus.bINT1RequiresCodeWinPosDec = FALSE;
			}

			extension->bCursorInCodeWinMode = FALSE;
			extension->dwCodeWinLastCursorPosY = 0;

			if ( extension->sisSysInterrStatus.bINT1RequiresCodeWinPosDec )
			{
				pvAddress = AddressPrevInstruction( (VOID*) extension->dwCodeWindowPos, 0 );
				if ( pvAddress )
					extension->dwCodeWindowPos = (DWORD) pvAddress;
			}

			// ### MOUSE INTERRUPT INITIALIZATION ###

			if ( bAvoidSettingKeybLEDsAndMouse == FALSE )
			{
				if ( extension->sisSysInterrStatus.bMouseEnabled == FALSE )
				{
					__asm
					{
						push		eax
						push		ebx
						push		ecx
						push		edx

						// Load in EDX the Pointer to the Structure.

						mov			edx, extension
						lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus

						// Disable Mouse Interrupt.

						push		edx
						xor			dh, dh
						call		EnablePs2MouseInterrupt
						pop			edx
						mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseInterruptHasToBeEnabled, al

						// Pop the Registers.

						pop			edx
						pop			ecx
						pop			ebx
						pop			eax
					}
				}
				else
				{
					__asm
					{
						push		eax
						push		ebx
						push		ecx
						push		edx

						// Load in EDX the Pointer to the Structure.

						mov			edx, extension
						lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus

						// Initialize the Mouse Variables.

						call		CheckPs2MouseState

						// Pop the Registers.

						pop			edx
						pop			ecx
						pop			ebx
						pop			eax
					}
				}
			}

			// ### ENABLE THE VPCICE KEYBOARD + MOUSE INTERRUPT ###

			EnableInterrupts();
			extension->sisSysInterrStatus.bSTIinIPIs = TRUE;

			// ### KEYBOARD LED INITIALIZATION ###

			if ( bAvoidSettingKeybLEDsAndMouse == FALSE )
			{
				__asm
				{
					push		eax
					push		ebx
					push		ecx
					push		edx

					// Load in EDX the Pointer to the Structure.

					mov			edx, extension
					lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus

					// Set the LEDs Status.

					push		edx
					mov			dh, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus
					call		SetKeyboardLEDsStatus
					pop			edx

					// Pop the Registers.

					pop			edx
					pop			ecx
					pop			ebx
					pop			eax
				}
			}

			bRestoreKeybLEDsAndMouse = TRUE;

			// ### PS/2 MOUSE/KEYB CONSISTENCE SYSTEM ###

			if ( extension->sisSysInterrStatus.bIsSingleStepping == FALSE )
				InitializePs2MouseKeybConsistenceSys();

			// ### SCREEN STARTUP ###

			// Startup and Main Loop.

			nsConStrupRes = ConsoleStartup( bIsUserReturning );

			if ( nsConStrupRes == STATUS_SUCCESS )
			{
				// The Console Restore function has to be Called.

				bIsConsoleRestoreRequired = TRUE;

				// Print only one time a Mouse Information Message.

				if ( extension->bInitializationDone == FALSE &&
					extension->sisSysInterrStatus.bMouseEnabled &&
					extension->sisSysInterrStatus.bMouseDetected )
				{
					OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": PS/2 Mouse detected. Device ID = 0x%.2X.",
						extension->sisSysInterrStatus.bMouseDeviceID );
				}

				// Print a Message in the case of a Breakpoint Hit.

				if ( extension->sisSysInterrStatus.pbpUniqueHitBreakpoint )
				{
					PrintBreakpointHitMessage(
							extension->sisSysInterrStatus.pbpUniqueHitBreakpoint,
							extension->sisSysInterrStatus.pbpUniqueHitBreakpoint->ulType == MACRO_BREAKPOINTTYPE_IO ||
								extension->sisSysInterrStatus.pbpUniqueHitBreakpoint->ulType == MACRO_BREAKPOINTTYPE_MEMORY
						);
				}

				// ### DEBUGGER LOOP ###

				if ( strlen( g_szAutoTypedCommand ) )
				{
					// Execute the Command.

					ExecuteAutoTypedCommand();
				}
				else
				{
					extension->bOpenUserModeDashboard = FALSE;
					
					do
					{
						// Loop Quantum.

						DebuggerLoopQuantum( extension->liCursorCpuCycles, & extension->sisSysInterrStatus );

						// Process the Key Stroke.

						if ( extension->sisSysInterrStatus.bLastAsciiCodeAcquired )
						{
							// Process the Codes.

							if ( extension->bInitializationDone )
								ProcessKeyStroke( extension->sisSysInterrStatus.bLastAsciiCodeAcquired,
									extension->sisSysInterrStatus.bLastScanCodeAcquired );

							// Reset the ASCII Code.

							extension->sisSysInterrStatus.bLastAsciiCodeAcquired = 0;
						}

						// Make the Cursor Blink.

						if ( extension->sisSysInterrStatus.bIsSingleStepping == FALSE )
							MakeCursorBlink();
					}
					while( extension->bInitializationDone &&
						extension->sisSysInterrStatus.bLastScanCodeAcquired != MACRO_SCANCODE_F5 &&
						extension->sisSysInterrStatus.bIsSingleStepping == FALSE &&
						extension->bOpenUserModeDashboard == FALSE );
				}

				extension->bInitializationDone = TRUE;
				extension->sisSysInterrStatus.bTimerThreadCreationAllowed = TRUE;
			}
		}

		// System Restore.

		if ( extension->sisSysInterrStatus.bFromInt3 &&
			extension->sisSysInterrStatus.ulDiscoveredBreakpointsNum )
		{
			// We have to Single Step in order to Replace the CC Opcode.

			if ( extension->sisSysInterrStatus.bIsSingleStepping )
				extension->sisSysInterrStatus.ulSysRestoreAction = MACRO_SYSREST_CCOPREST_SINGSTP;
			else
				extension->sisSysInterrStatus.ulSysRestoreAction = MACRO_SYSREST_CCOPREST;

			extension->sisSysInterrStatus.bIsSingleStepping = TRUE;
			bDetAndBreakpEnableReq = FALSE;
		}
		else
		{
			// Reset the Variable(s).

			extension->sisSysInterrStatus.ulSysRestoreAction = MACRO_SYSREST_NONE;
		}

		// Screen Restore.

		if (
			( bIsConsoleRestoreRequired &&
			extension->sisSysInterrStatus.ulSysRestoreAction == MACRO_SYSREST_CCOPREST ) ||
			( bIsConsoleRestoreRequired &&
			extension->sisSysInterrStatus.bIsSingleStepping == FALSE ) )
		{
			if ( extension->bSnapShotMode == FALSE )
				ConsoleRestore();
		}
	}

	// ### KEYBOARD RESTORE ###

	// Reset the State of the Keyboard LEDs.

	if ( 
		( extension->sisSysInterrStatus.ulSysRestoreAction == MACRO_SYSREST_CCOPREST &&
		bRestoreKeybLEDsAndMouse ) ||
		( extension->sisSysInterrStatus.bIsSingleStepping == FALSE &&
		bRestoreKeybLEDsAndMouse ) )
	{
		RestoreKeybLEDs();
	}

	// ### ENABLE INTERRUPTS ###

	DisableInterrupts();

	if ( extension->sisSysInterrStatus.bIsSingleStepping == FALSE )
	{
		if ( extension->sisSysInterrStatus.pvLocalApicMemoryPtr )
			RestoreLocalAPIC_TPR( & extension->sisSysInterrStatus );

		RestoreInterrupts( & extension->sisSysInterrStatus );
	}

	// ### BREAKPOINTS AND DETOURS ENABLE CODE. ###

	ResetDebugRegisters();

	if ( bDetAndBreakpEnableReq &&
		extension->sisSysInterrStatus.bIsSingleStepping == FALSE )
	{
		// Opcode Breakpoints.

		EnableDetoursAndBreakpoints( & extension->sisSysInterrStatus, TRUE );

		// Hardware Breakpoints.

		GetUpdatedDebugRegisterValues();
	}

	EnableLastBranchMSRs();

	// ### MOUSE RESTORE ###

	// Restore the State of the Mouse.

	if ( 
		( extension->sisSysInterrStatus.ulSysRestoreAction == MACRO_SYSREST_CCOPREST &&
		bRestoreKeybLEDsAndMouse ) ||
		( extension->sisSysInterrStatus.bIsSingleStepping == FALSE &&
		bRestoreKeybLEDsAndMouse ) )
	{
		if ( extension->sisSysInterrStatus.bMouseEnabled == FALSE )
		{
			__asm
			{
				push		eax
				push		ebx
				push		ecx
				push		edx

				// Load in EDX the Pointer to the Structure.

				mov			edx, extension
				lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus

				// Restore the Mouse Status.

				cmp			[ edx ]SYSINTERRUPTS_STATUS.bMouseInterruptHasToBeEnabled, 0
				je			_SkipMouseEnableCall

				push		edx
				mov			dh, 1
				call		EnablePs2MouseInterrupt
				pop			edx

		_SkipMouseEnableCall:

				pop			edx
				pop			ecx
				pop			ebx
				pop			eax
			}
		}
	}

	// Return to the Caller.

	return;
}

//==================================================
// GetDrawGlyphLayoutsInstance Function Definition.
//==================================================

static DRAWGLYPH_LAYOUTS* GetDrawGlyphLayoutsInstance( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	// Set and Return the Instance.

	extension->dglLayouts.vmiVideoMemInfo = * extension->pvmiVideoMemoryInfo;

	return & extension->dglLayouts;
}

//=====================================
// ConsoleStartup Function Definition.
//=====================================

NTSTATUS ConsoleStartup( IN BOOLEAN bIsUserReturning )
{
	NTSTATUS				nsRetVal = STATUS_UNSUCCESSFUL;
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	NTSTATUS				nsVpcICEVidMemInitRes;
	ULONG					ulInitialWidth, ulInitialHeight;
	ULONG					ulProposedWidth, ulProposedHeight;
	ULONG					ulX0, ulY0, ulX1, ulY1;

	// Check to see whether the Video Mode changed.

	if ( CompMem( extension->pvmiVideoMemoryInfo, & extension->vmiPrevVideoMemoryInfo, sizeof( VIDEOMEMORYINFO ) ) )
	{
		// Calculate the dimensions of the Console.

		ulInitialWidth = MACRO_CONSOLE_DEFAULT_WIDTH;
		ulInitialHeight = MACRO_CONSOLE_DEFAULT_HEIGHT;

		if ( extension->pvmiVideoMemoryInfo->pvPrimary )
		{
			ulInitialWidth += 2;
			ulInitialHeight += 2;
		}

		ulProposedWidth = extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ulProposedHeight = extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersHeightInChars;

		if ( ulProposedWidth < ulInitialWidth )
			ulProposedWidth = ulInitialWidth;
		if ( ulProposedHeight < ulInitialHeight )
			ulProposedHeight = ulInitialHeight;

		// Initialize the structure describing the Video Memory Layout of VpcICE.

		nsVpcICEVidMemInitRes = InitializeVpcICEVideoMemoryStructure( & extension->dglLayouts.vivmVpcICEVideo,
			extension->pvmiVideoMemoryInfo,
			extension->pvVpcICEVideoBuffer, extension->ulVpcICEVideoBufferSizeInBytes,
			ulProposedWidth, ulProposedHeight );

		if ( nsVpcICEVidMemInitRes != STATUS_SUCCESS )
		{
			nsVpcICEVidMemInitRes = InitializeVpcICEVideoMemoryStructure( & extension->dglLayouts.vivmVpcICEVideo,
				extension->pvmiVideoMemoryInfo,
				extension->pvVpcICEVideoBuffer, extension->ulVpcICEVideoBufferSizeInBytes,
				ulInitialWidth, ulInitialHeight );

			if ( nsVpcICEVidMemInitRes != STATUS_SUCCESS )
			{
				nsVpcICEVidMemInitRes = InitializeVpcICEVideoMemoryStructure( & extension->dglLayouts.vivmVpcICEVideo,
					extension->pvmiVideoMemoryInfo,
					extension->pvVpcICEVideoBuffer, extension->ulVpcICEVideoBufferSizeInBytes,
					MACRO_CONSOLE_DEFAULT_WIDTH_MIN, MACRO_CONSOLE_DEFAULT_HEIGHT_MIN );
			}
		}

		if ( nsVpcICEVidMemInitRes == STATUS_SUCCESS )
		{
			// Initialize the memory of the Text Front and Back Buffer.

			memset( extension->dglLayouts.vivmVpcICEVideo.pwTextFrontBuffer, 0, extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersSizeInBytes );
			memset( extension->dglLayouts.vivmVpcICEVideo.pwTextBackBuffer, 0, extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersSizeInBytes );

			// Initialization OK.

			nsRetVal = STATUS_SUCCESS;
		}

		// Record the Display Mode Change.

		extension->vmiPrevVideoMemoryInfo = * extension->pvmiVideoMemoryInfo;
	}
	else
	{
		// Reset the Back Buffer, forcing a Redraw.

		if ( bIsUserReturning == FALSE )
			memset( extension->dglLayouts.vivmVpcICEVideo.pwTextBackBuffer, 0, extension->dglLayouts.vivmVpcICEVideo.ulTextBuffersSizeInBytes );

		// Initialization OK.

		nsRetVal = STATUS_SUCCESS;
	}

	// Manage the Mouse Pointer Variables.

	if ( extension->sisSysInterrStatus.bMouseEnabled &&
		extension->sisSysInterrStatus.bMouseDetected )
	{
		extension->dglLayouts.vivmVpcICEVideo.bMousePointerDisplayed = TRUE;
	}
	else
	{
		extension->dglLayouts.vivmVpcICEVideo.bMousePointerDisplayed = FALSE;
	}

	// Rearrange the VpcICE Windows.

	if ( nsRetVal == STATUS_SUCCESS )
	{
		// Rearrange.

		GetConsolePrintCoords( & ulX0, & ulY0, & ulX1, & ulY1 );

		nsRetVal = RearrangeVpcICEWindows( MACRO_RESIZINGSPECIFICWINDOWNAME_UNDEF, 0, ulX0, ulY0, ulX1, ulY1, & extension->dglLayouts.vivmVpcICEVideo );
	}

	// Draw the Initial Screen of VpcICE.

	if ( nsRetVal == STATUS_SUCCESS )
	{
		// Take a snapshot of the Windows Desktop.

		if ( bIsUserReturning == FALSE )
			SaveApplicationScreen( GetDrawGlyphLayoutsInstance() );

		// Draw the Console.

		DrawConsole( bIsUserReturning == FALSE );
	}

	// Return to the Caller.

	return nsRetVal;
}

//=====================================
// ConsoleRestore Function Definition.
//=====================================

VOID ConsoleRestore( VOID )
{
	// Restore the Application Screen.

	RestoreApplicationScreen( GetDrawGlyphLayoutsInstance() );

	// Return to the Caller.

	return;
}

//==================================
// DrawConsole Function Definition.
//==================================

static CHAR			g_szDrawConsoleFnTempBuffer[ 2 * 1024 ];

VOID DrawConsole( IN BOOLEAN bIsFirstDraw )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	ULONG					ulI, ulJ, ulK;
	WORD*					pwPtr;
	ULONG					ulX0, ulY0, ulX1, ulY1;
	VPCICE_WINDOW*			pviwWin;
	VPCICE_WINDOW*			pviwOutputWindow;
	VPCICE_WINDOW*			pviwRegistersWindow;
	VPCICE_WINDOW*			pviwCodeWindow;

	// === Reset the Front Buffer. ===

	pwPtr = pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer;

	for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulTextBuffersSizeInBytes / sizeof( WORD ); ulI ++ )
		* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	// === Draw the VpcICE Console. ===

	// Get the Print Coords of the Console.

	GetConsolePrintCoords( & ulX0, & ulY0, & ulX1, & ulY1 );

	// Check whether we have to draw a border around the Console.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary )
	{
		// Draw the Console Border.

		* GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			0, 0 ) = CLRTXT2VIDBUFWORD( 3, 0, 218 );
		* GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulConsoleW - 1, 0 ) = CLRTXT2VIDBUFWORD( 3, 0, 191 );
		* GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			0, ulConsoleH - 1 ) = CLRTXT2VIDBUFWORD( 3, 0, 192 );
		* GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulConsoleW - 1, ulConsoleH - 1 ) = CLRTXT2VIDBUFWORD( 3, 0, 217 );

		pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			1, 0 );
		for ( ulI = 1; ulI < ulConsoleW - 1; ulI ++ )
		{
			* pwPtr ++ = CLRTXT2VIDBUFWORD( 3, 0, 196 );
		}

		pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			0, 1 );
		for ( ulI = 1; ulI < ulConsoleH - 1; ulI ++ )
		{
			* pwPtr = CLRTXT2VIDBUFWORD( 3, 0, 179 );
			pwPtr += ulConsoleW;
		}

		pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulConsoleW - 1, 1 );
		for ( ulI = 1; ulI < ulConsoleH - 1; ulI ++ )
		{
			* pwPtr = CLRTXT2VIDBUFWORD( 3, 0, 179 );
			pwPtr += ulConsoleW;
		}

		pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			1, ulConsoleH - 1 );
		for ( ulI = 1; ulI < ulConsoleW - 1; ulI ++ )
		{
			* pwPtr ++ = CLRTXT2VIDBUFWORD( 3, 0, 196 );
		}
	}

	// Draw the VpcICE Window Lines.

	for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulWindowsNum; ulI ++ )
	{
		pviwWin = & pdglLayouts->vivmVpcICEVideo.vvwWindows[ ulI ];

		// Draw the Line, if Required.

		if ( pviwWin->bDisplayed && pviwWin->bHasTopLine )
		{
			pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwWin->ulX0, pviwWin->ulY0 - 1 );

			for ( ulK = pviwWin->ulX1 - pviwWin->ulX0 + 1, ulJ = 0; ulJ < ulK; ulJ ++ )
			{
				* pwPtr ++ = CLRTXT2VIDBUFWORD( 2, 0, 196 );
			}
		}
	}

	// Draw the Contents of the Script Window.

	DrawScriptWindowContents( FALSE, FALSE );

	// Draw the Contents of the Output Window.

	pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwOutputWindow )
	{
		KIRQL		kilCurrentIRQL;
		CHAR*		pszIrqlRepr;
		CHAR		szIrqlReprBuffer[ 0x10 ];
		ULONG		ulSysStatusBarCurrX;
		PVOID		pvKTEB;
		WORD		wTID;

		ulSysStatusBarCurrX = pviwOutputWindow->ulX0;

		// === Draw the System Status Informations. ===

		// Current IRQ Level.

		if ( extension->sisSysInterrStatus.pvLocalApicMemoryPtr == NULL )
			kilCurrentIRQL = GetCurrentIrql( extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase );
		else
			kilCurrentIRQL = GetCurrentIrqlFromLocalAPIC( & extension->sisSysInterrStatus );

		switch( kilCurrentIRQL )
		{
		case PASSIVE_LEVEL:
			pszIrqlRepr = "(PASSIVE)";
			break;

		case APC_LEVEL:
			pszIrqlRepr = "(APC)";
			break;

		case DISPATCH_LEVEL:
			pszIrqlRepr = "(DISPATCH)";
			break;

		case PROFILE_LEVEL:
			pszIrqlRepr = "(PROFILE)";
			break;

		case CLOCK2_LEVEL:
			pszIrqlRepr = "(CLOCK2)";
			break;

		case IPI_LEVEL:
			pszIrqlRepr = "(IPI)";
			break;

		case POWER_LEVEL:
			pszIrqlRepr = "(POWER)";
			break;

		case HIGH_LEVEL:
			pszIrqlRepr = "(HIGH)";
			break;

		default:
			sprintf( szIrqlReprBuffer, "%.2X", kilCurrentIRQL );
			pszIrqlRepr = szIrqlReprBuffer;
			break;
		}

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulSysStatusBarCurrX, pviwOutputWindow->ulY0 - 1, 0x02, "IRQL=" );

		ulSysStatusBarCurrX += 5;

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulSysStatusBarCurrX, pviwOutputWindow->ulY0 - 1, 0x02, pszIrqlRepr );

		ulSysStatusBarCurrX += strlen( pszIrqlRepr );

		// === KTEB and TID Informations. ===

		pvKTEB = GetCurrentKTEB( extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase );
		wTID = GetCurrentTID( extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase );

		sprintf( g_szDrawConsoleFnTempBuffer, "KTEB(%.8X)", (DWORD) pvKTEB );

		ulSysStatusBarCurrX ++;

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulSysStatusBarCurrX, pviwOutputWindow->ulY0 - 1, 0x02, g_szDrawConsoleFnTempBuffer );

		ulSysStatusBarCurrX += strlen( g_szDrawConsoleFnTempBuffer ) + 1;

		sprintf( g_szDrawConsoleFnTempBuffer, "TID(%.4X)", wTID );

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulSysStatusBarCurrX, pviwOutputWindow->ulY0 - 1, 0x02, g_szDrawConsoleFnTempBuffer );

		ulSysStatusBarCurrX += strlen( g_szDrawConsoleFnTempBuffer );

		// === Module Info + CPU Number. ===

		extension->ulSysStatusBarModuleInfoStartX = ulSysStatusBarCurrX;

		PrintCodeWindowPosInModules( pviwOutputWindow, ulConsoleW, ulConsoleH, & ulSysStatusBarCurrX );

		// === Draw the Contents of the Output Window. ===

		DrawOutputWindowContents( pviwOutputWindow, ulConsoleW, ulConsoleH );
	}

	// Draw the Contents of the Registers Window.

	pviwRegistersWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_REGISTERS, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwRegistersWindow &&
		pviwRegistersWindow->bDisplayed )
	{
		// === Draw the Contents of the Registers Window. ===

		DrawRegistersWindowContents( pviwRegistersWindow, ulConsoleW, ulConsoleH, TRUE );
	}

	// Draw the Contents of the Code Window.

	pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwCodeWindow &&
		pviwCodeWindow->bDisplayed )
	{
		// === Draw the Contents of the Code Window. ===

		DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
	}

	// Prepare the Console Cursor.

	pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		ulX0, ulY1 - 1 );

	* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ':' );

	for ( ulI = ulX0 + 1; ulI <= ulX1; ulI ++ )
		* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	pdglLayouts->vivmVpcICEVideo.ulCursorX = ulX0 + 1;
	pdglLayouts->vivmVpcICEVideo.ulCursorY = ulY1 - 1;

	// Draw the Status Bar.

	DrawStatusBar( ulConsoleW, ulConsoleH );

	// === Enable the Cursor and Draw the First Screen. ===

	if ( bIsFirstDraw )
		EnableCursor( TRUE, pdglLayouts );

	DrawScreen( pdglLayouts );

	// Return to the Caller.

	return;
}

//======================================
// MakeCursorBlink Function Definition.
//======================================

VOID MakeCursorBlink( VOID )
{
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();

	// Make the Cursor Blink.

	if ( pdglLayouts->vivmVpcICEVideo.bCursorDisplayed &&
		pdglLayouts->vmiVideoMemInfo.pvPrimary )
	{
		DrawCursor( pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY, pdglLayouts );

		if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus == FALSE )
			pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus = TRUE;
		else
			pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus = FALSE;
	}

	// Return to the Caller.

	return;
}

//============================================
// GetConsolePrintCoords Function Definition.
//============================================

VOID GetConsolePrintCoords( OUT ULONG* pulX0, OUT ULONG* pulY0, OUT ULONG* pulX1, OUT ULONG* pulY1 )
{
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();

	// Return the Information.

	* pulX0 = 0;
	* pulY0 = 0;
	* pulX1 = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars - 1;
	* pulY1 = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars - 1;

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary )
	{
		++ * pulX0;
		++ * pulY0;
		-- * pulX1;
		-- * pulY1;
	}

	// Return to the Caller.

	return;
}

//======================================================
// GetEscapedStringUnescapedLength Function Definition.
//======================================================

static CHAR			g_szGetEscapedStringUnescapedLengthFnBuffer[ 2 * 1024 ];

static ULONG GetEscapedStringUnescapedLength( IN CHAR* pszString )
{
	EliminateStringEscapes( g_szGetEscapedStringUnescapedLengthFnBuffer, pszString );
	return strlen( g_szGetEscapedStringUnescapedLengthFnBuffer );
}

static CHAR* GetEscapedSafeString( IN CHAR* pszString )
{
	EliminateStringEscapes( g_szGetEscapedStringUnescapedLengthFnBuffer, pszString );
	return g_szGetEscapedStringUnescapedLengthFnBuffer;
}

//=======================================
// ProcessKeyStroke Function Definition.
//=======================================

#define MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE	\
	if ( extension->sisSysInterrStatus.bShiftPressed )							\
	{																			\
		extension->vulScriptWinSelAreaX[ 1 ] = extension->ulScriptWinCol - 1;	\
		extension->vulScriptWinSelAreaY[ 1 ] = extension->ulScriptWinLn - 1;	\
		vulSelAreaX[ 0 ] = extension->vulScriptWinSelAreaX[ 0 ];				\
		vulSelAreaX[ 1 ] = extension->vulScriptWinSelAreaX[ 1 ];				\
		vulSelAreaY[ 0 ] = extension->vulScriptWinSelAreaY[ 0 ];				\
		vulSelAreaY[ 1 ] = extension->vulScriptWinSelAreaY[ 1 ];				\
		ApplySelAreaToText( TRUE, vulSelAreaX, vulSelAreaY, NULL, NULL );		\
		DrawScriptWindowContents( FALSE, FALSE );								\
		extension->bScriptWinShowSelArea = TRUE;								\
	}

static CHAR			g_szCurrentFunctionName[ 80 ] = "";

VOID ProcessKeyStroke( IN BYTE bAsciiCode, IN BYTE bScanCode )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	WORD*					pwPtr;
	ULONG					ulX0, ulY0, ulX1, ulY1;
	ULONG					ulI, ulChrsNum;
	BOOLEAN					bCanContinue;
	BOOLEAN					bScriptWin = FALSE;
	VPCICE_WINDOW*			pviwScriptWindow;
	ULONG					ulA, ulB, ulC, ulD;
	BOOLEAN					bKeepSelectingInScriptWin = FALSE;
	ULONG					vulSelAreaX[ 2 ];
	ULONG					vulSelAreaY[ 2 ];
	BOOLEAN					bSelAreaValid = FALSE;
	BOOLEAN					bSelAreaYInc = FALSE;
	ULONG					ulYIncCorrectX = 0;
	BOOLEAN					bCopyRes;
	BOOLEAN					bOldScriptWinDirtyBit = extension->bScriptWinDirtyBit;
	CHAR					szFunctionName[ sizeof( g_szCurrentFunctionName ) ];
	BOOLEAN					bUpdateScriptWinLnColInfos = FALSE;

	// Get the Print Coords of the Console.

	GetConsolePrintCoords( & ulX0, & ulY0, & ulX1, & ulY1 );

	//
	// If a Selection is in progress in the Script Window, cancel the Selection Area.
	//

	if ( extension->bScriptWinShowSelArea )
	{
		vulSelAreaX[ 0 ] = extension->vulScriptWinSelAreaX[ 0 ];
		vulSelAreaX[ 1 ] = extension->vulScriptWinSelAreaX[ 1 ];
		vulSelAreaY[ 0 ] = extension->vulScriptWinSelAreaY[ 0 ];
		vulSelAreaY[ 1 ] = extension->vulScriptWinSelAreaY[ 1 ];
		bSelAreaValid = ApplySelAreaToText( FALSE, vulSelAreaX, vulSelAreaY, & bSelAreaYInc, & ulYIncCorrectX );
	}
	else
	{
		extension->vulScriptWinSelAreaX[ 0 ] = extension->ulScriptWinCol - 1;
		extension->vulScriptWinSelAreaY[ 0 ] = extension->ulScriptWinLn - 1;
	}

	//
	// Check whether we are Editing in the Script Window.
	// If so, Begin the Editing of the Line.
	//

	pviwScriptWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_SCRIPT, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed &&
		pdglLayouts->vivmVpcICEVideo.ulCursorX != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwScriptWindow->ulY0 &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwScriptWindow->ulY1 )
	{
		bScriptWin = TRUE;
	}

	if ( bScriptWin )
		BeginScriptWindowLineEdit();

	// Process the Information.

	if ( bAsciiCode != MACRO_ASCIICONVERSION_RETVAL_NOT_INTERESTING &&
		bAsciiCode != MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
		bAsciiCode != MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX )
	{
		//
		// If the CTRL and ALT key are Not Pressed, Display the Character.
		//

		if ( extension->sisSysInterrStatus.bControlPressed == FALSE &&
			extension->sisSysInterrStatus.bAltPressed == FALSE )
		{
			VPCICE_WINDOW*	pviwCodeWindow;

			pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

			// If the "Cursor in Code Window Mode" is on, take care of the Cursor Position.

			if ( pviwCodeWindow && pviwCodeWindow->bDisplayed &&
				pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwCodeWindow->ulY0 &&
				pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwCodeWindow->ulY1 )
			{
				// Modify the Position of the Cursor.

				if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
					MakeCursorBlink();

				pdglLayouts->vivmVpcICEVideo.ulCursorX = ulX0 + 1;
				pdglLayouts->vivmVpcICEVideo.ulCursorY = ulY1 - 1;

				// Check whether we have to disable the "Cursor in Code Window Mode".

				if ( bAsciiCode == ' ' )
					extension->bCursorInCodeWinMode = FALSE;
			}

			// Draw the Character.

			if ( bAsciiCode == ' ' && DoAutoCompletion( ulConsoleW, ulConsoleH ) )
			{
				// Draw the Updated Screen.

				DrawScreen( pdglLayouts );
			}
			else if ( bScriptWin || pdglLayouts->vivmVpcICEVideo.ulCursorX < ulX1 )
			{
				// Draw the Character at the Current Cursor Position.

				if ( bScriptWin && extension->ulScriptWinCol >= MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
					bCanContinue = FALSE;
				else
					bCanContinue = TRUE;

				if ( bCanContinue && extension->sisSysInterrStatus.bTextInsert )
				{
					// Get the Final Position of the Cursor.

					if ( bScriptWin )
					{
						pwPtr = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1;

						ulB = ulI = MACRO_SCRIPTWIN_LINESIZE_IN_CHARS;
						ulA = 0;
					}
					else
					{
						pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							ulX1, pdglLayouts->vivmVpcICEVideo.ulCursorY );

						ulB = ulI = ulX1;
						ulA = ulX0;
					}

					for ( ; ulI > ulA; ulI --, pwPtr -- )
						if ( * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
							break;

					ulI ++;

					// Check whether we can continue.

					if ( ulI >= ulB )
					{
						bCanContinue = FALSE;
					}
					else
					{
						if ( bScriptWin )
						{
							ulChrsNum = ( MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1 ) - ( extension->ulScriptWinCol - 1 ) - 1;

							pwPtr = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 2;
						}
						else
						{
							ulChrsNum = ulX1 - pdglLayouts->vivmVpcICEVideo.ulCursorX - 1;

							pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
								ulX1 - 1, pdglLayouts->vivmVpcICEVideo.ulCursorY );
						}

						for ( ulI = 0; ulI < ulChrsNum; ulI ++, pwPtr -- )
							* pwPtr = * ( pwPtr - 1 );

						if ( bScriptWin )
							extension->bScriptWinDirtyBit = TRUE;
					}
				}

				if ( bCanContinue )
				{
					if ( bScriptWin )
					{
						pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );
					}
					else
					{
						pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY );
					}

					* pwPtr = CLRTXT2VIDBUFWORD( 7, 0, bAsciiCode );

					if ( bScriptWin )
					{
						extension->ulScriptWinCol ++;
						DrawScriptWindowContents( TRUE, TRUE );
					}
					else
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();
						pdglLayouts->vivmVpcICEVideo.ulCursorX ++;
					}

					DrawScreen( pdglLayouts );

					if ( bScriptWin )
						extension->bScriptWinDirtyBit = TRUE;
				}
			}
		}
		else	// # CTRL and/or ATL key PRESSED ! #
		{
			if ( bScriptWin )
			{
				//
				// By default, Keep the Text Selection in the Script Window.
				//

				bKeepSelectingInScriptWin = TRUE;

				//
				// Script Window Clipboard Functions.
				//

				if ( extension->sisSysInterrStatus.bControlPressed &&
					extension->sisSysInterrStatus.bAltPressed == FALSE )
				{
					if ( bAsciiCode == 'c' || bAsciiCode == 'C' ) // # COPY #
					{
						// Copy in the Clipboard.

						if ( bSelAreaValid )
							CopyScriptTextToClip( vulSelAreaX, vulSelAreaY );
					}
					else if ( bAsciiCode == 'v' || bAsciiCode == 'V' ) // # PASTE #
					{
						// Paste in the Script Win.

						ulA = extension->ulScriptWinCol - 1;
						ulB = extension->ulScriptWinLn - 1;
						PasteScriptText( & ulA, & ulB );
						extension->ulScriptWinCol = ulA + 1;
						extension->ulScriptWinLn = ulB + 1;

						// Update.

						DrawScriptWindowContents( FALSE, TRUE );
						DrawScreen( pdglLayouts );
						bKeepSelectingInScriptWin = FALSE;

						extension->bScriptWinDirtyBit = TRUE;
					}
					else if ( bAsciiCode == 'x' || bAsciiCode == 'X' ) // # CUT #
					{
						if ( bSelAreaValid )
						{
							// Cut the Selected Text.

							bCopyRes = CopyScriptTextToClip( vulSelAreaX, vulSelAreaY );
							if ( bCopyRes )
							{
								DeleteScriptText( vulSelAreaX, vulSelAreaY );
								extension->ulScriptWinCol = ( bSelAreaYInc ? ulYIncCorrectX : vulSelAreaX[ 0 ] ) + 1;
								extension->ulScriptWinLn = vulSelAreaY[ 0 ] + ( bSelAreaYInc ? 0 : 1 );
							}

							// Update.

							DrawScriptWindowContents( FALSE, TRUE );
							DrawScreen( pdglLayouts );
							bKeepSelectingInScriptWin = FALSE;

							extension->bScriptWinDirtyBit = TRUE;
						}
					}
				}
			}
		}
	}
	else
	{
		//
		// Tab.
		//
		if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_Tab )
		{
			if ( bScriptWin )
			{
				// Get the Number of Tabs to Draw.

				ulA = 4 - ( ( extension->ulScriptWinCol - 1 ) % 4 );

				g_bDrawScreenIsEnabled = FALSE;

					for ( ulI = 0; ulI < ulA; ulI ++ )
						ProcessKeyStroke( ' ', MACRO_SCANCODE_Space );

				g_bDrawScreenIsEnabled = TRUE;

				DrawScreen( pdglLayouts );

				extension->bScriptWinDirtyBit = TRUE;
			}
			else if ( pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				if ( DoAutoCompletion( ulConsoleW, ulConsoleH ) )
				{
					// Draw the Updated Screen.

					DrawScreen( pdglLayouts );
				}
			}
		}
		//
		// F6 Key.
		//
		else if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_F6 )
		{
			// Toggle the Code Window Mode.

			if ( bScriptWin )
			{
				ProcessKeyStroke(
					MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE, MACRO_SCANCODE_F7 );
			}
			else if ( extension->bCursorInCodeWinMode )
			{
				extension->bCursorInCodeWinMode = FALSE;
				extension->dwCodeWinLastCursorPosY = 0;
			}
			else
			{
				VPCICE_WINDOW*	pviwCodeWindow;

				pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
				{
					extension->bCursorInCodeWinMode = TRUE;
					extension->dwCodeWinLastCursorPosY = pviwCodeWindow->ulY0;
				}
			}
		}
		//
		// F7 Key.
		//
		else if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_F7 )
		{
			VPCICE_WINDOW*	pviwOutputWindow;

			// Toggle the Script Window Focus.

			pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );

			if ( pviwOutputWindow /* always != NULL ... */ &&
				pviwScriptWindow && pviwScriptWindow->bDisplayed )
			{
				//
				// Check out in which Window the Cursor is currently placed.
				//

				if ( pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwScriptWindow->ulY0 &&
					pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwScriptWindow->ulY1 ) // ## We are in the Script Window... ##
				{
					// Change the Cursor Position back to the Output Window...

					if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
						MakeCursorBlink();

					pdglLayouts->vivmVpcICEVideo.ulCursorX = extension->ulScriptWinPrevCurX;
					pdglLayouts->vivmVpcICEVideo.ulCursorY = extension->ulScriptWinPrevCurY;
				}
				else // ## We are Somewhere Else... ###
				{
					// Reset the Code Window Mode Flag.

					extension->bCursorInCodeWinMode = FALSE;
					extension->dwCodeWinLastCursorPosY = 0;

					//
					// Check whether we are in the Output Window.
					// We need here to Save the Current Cursor Position.
					//

					if ( pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwOutputWindow->ulY0 &&
						pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwOutputWindow->ulY1 ) // ## We are in the Output Window... ##
					{
						// Save the Cursor Position, KEEP the current Output Window Line.

						extension->ulScriptWinPrevCurX = pdglLayouts->vivmVpcICEVideo.ulCursorX;
						extension->ulScriptWinPrevCurY = pdglLayouts->vivmVpcICEVideo.ulCursorY;
					}
					else // ## We are in an Other Window... ##
					{
						// Reset the Cursor Position to the Output Window, before Giving Control to the Script One...

						pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							ulX0, ulY1 - 1 );

						* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ':' );

						for ( ulI = ulX0 + 1; ulI <= ulX1; ulI ++ )
							* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

						extension->ulScriptWinPrevCurX = ulX0 + 1;
						extension->ulScriptWinPrevCurY = ulY1 - 1;
					}

					// Change the Cursor Position into the Script Window.

					ValidateScriptWinLnColInfos();

					if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
						MakeCursorBlink();

					pdglLayouts->vivmVpcICEVideo.ulCursorX =
						pviwScriptWindow->ulX0 + extension->ulScriptWinCol - 1 - extension->ulScriptWinOffsetX;
					pdglLayouts->vivmVpcICEVideo.ulCursorY =
						pviwScriptWindow->ulY0 + extension->ulScriptWinLn - 1 - extension->ulScriptWinOffsetY;
				}

				//
				// Redraw the Debugger Status Bar.
				//

				DrawStatusBar( ulConsoleW, ulConsoleH );
				DrawScreen( pdglLayouts );
			}
		}
		//
		// Backspace.
		//
		else if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_BackSp )
		{
			if ( bScriptWin || pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				// Check whether we can Continue.

				bCanContinue = FALSE;

				if ( bScriptWin )
				{
					if ( extension->ulScriptWinCol > 1 )
					{
						bCanContinue = TRUE;
					}
					else
					{
						//
						// Check whether we are at the Beginning of the Script File.
						//

						if ( extension->ulScriptWinLn > 1 )
						{
							// Calculate the Length of the Script File Lines of Interest.

							ulC = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

							ulA = 0xFFFFFFFF;
							ulB = 0xFFFFFFFF;

							if ( extension->ulScriptWinLn - 1 < ulC )
								ulA = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLn - 1 ] );

							if ( extension->ulScriptWinLn - 2 < ulC )
								ulB = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLn - 2 ] );

							// Check out what action is Required.

							if ( ulB == 0xFFFFFFFF )
							{
								//
								// The PREVIOUS LINE has no Storage allocated to It.
								//  Simply, decrement the Current Line Counter.
								//

								extension->ulScriptWinLn --;

								DrawScriptWindowContents( TRUE, TRUE );
								DrawScreen( pdglLayouts );
							}
							else if ( ulA == 0xFFFFFFFF )
							{
								//
								// The CURRENT LINE has no Storage allocated to It.
								//  Set the Cursor Position to the END of the Previous Line.
								//

								extension->ulScriptWinLn --;
								extension->ulScriptWinCol = ulB + 1;

								DrawScriptWindowContents( TRUE, TRUE );
								DrawScreen( pdglLayouts );
							}
							else
							{
								//
								// Both the LINES are Allocated.
								//  Sadly we can support only Line Joins where the Total Final Line Length is within the Limits.
								//

								if ( ulA + ulB < MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
								{
									//
									// Save the Current Line and make the Join with the Previous.
									//

									ulD = extension->ulScriptWinLn - 1;

									wcscpy(
										extension->vwScriptWinBackupBuffer,
										extension->ppwScriptWinStrPtrsBuffer[ ulD ] );

									extension->ulScriptWinLn --;
									extension->ulScriptWinCol = ulB + 1;
									DrawScriptWindowContents( TRUE, TRUE );

									DeleteScriptLine( ulD );

									BeginScriptWindowLineEdit();

									memcpy(
										extension->vwScriptWinLine + ulB,
										extension->vwScriptWinBackupBuffer,
										ulA * sizeof( WORD ) );

									DrawScriptWindowContents( TRUE, TRUE );
									DrawScreen( pdglLayouts );

									extension->bScriptWinDirtyBit = TRUE;
								}
							}
						}
					}
				}
				else if ( pdglLayouts->vivmVpcICEVideo.ulCursorX > ulX0 + 1 )
				{
					bCanContinue = TRUE;
				}

				if ( bCanContinue )
				{
					// Take care of the Cursor.

					if ( bScriptWin )
					{
						extension->ulScriptWinCol --;
					}
					else
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();
						pdglLayouts->vivmVpcICEVideo.ulCursorX --;
					}

					// Shift the Contents of the Line.

					if ( bScriptWin )
					{
						pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );

						ulChrsNum = ( MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1 ) - ( extension->ulScriptWinCol - 1 );
					}
					else
					{
						pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY );

						ulChrsNum = ulX1 - pdglLayouts->vivmVpcICEVideo.ulCursorX;
					}

					for ( ulI = 0; ulI < ulChrsNum; ulI ++, pwPtr ++ )
						* pwPtr = * ( pwPtr + 1 );
					* pwPtr = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

					// Draw the Updated Screen.

					if ( bScriptWin )
						DrawScriptWindowContents( TRUE, TRUE );

					DrawScreen( pdglLayouts );

					if ( bScriptWin )
						extension->bScriptWinDirtyBit = TRUE;
				}
			}
		}
		//
		// Delete.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Del ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_DELETE )
			)
		{
			if ( bScriptWin || pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				bCanContinue = TRUE;

				//
				// SPECIAL CONDITION: in Script Window, the User is trying to Cancel a Text Selection.
				//

				if ( bScriptWin && bSelAreaValid )
				{
					bCanContinue = FALSE;

					// Delete the Selected Text.

					DeleteScriptText( vulSelAreaX, vulSelAreaY );
					extension->ulScriptWinCol = ( bSelAreaYInc ? ulYIncCorrectX : vulSelAreaX[ 0 ] ) + 1;
					extension->ulScriptWinLn = vulSelAreaY[ 0 ] + ( bSelAreaYInc ? 0 : 1 );

					// Update.

					DrawScriptWindowContents( FALSE, TRUE );
					DrawScreen( pdglLayouts );

					extension->bScriptWinDirtyBit = TRUE;
				}

				//
				// SPECIAL CONDITION: Script File mode: Check whether we have to Join this Line with the Next.
				//

				if ( bCanContinue && bScriptWin )
				{
					WORD*			pwPtrEnd;

					pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );
					pwPtrEnd = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1;

					while( pwPtr <= pwPtrEnd )
						if ( * pwPtr ++ != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
							break;

					if ( pwPtr > pwPtrEnd )
					{
						bCanContinue = FALSE;

						//
						// Join this Line with the Next.
						//  Sadly this Operation is allowed only if the Memory Constraints are Satisfied.
						//

						ulC = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

						ulA = 0xFFFFFFFF;
						if ( extension->ulScriptWinLn < ulC )
							ulA = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLn ] );

						if ( ulA != 0xFFFFFFFF &&
							( extension->ulScriptWinCol - 1 ) + ulA < MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
						{
							memcpy(
								extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 ),
								extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLn ],
								ulA * sizeof( WORD ) );

							EndScriptWindowLineEdit();

							DeleteScriptLine( extension->ulScriptWinLn );

							DrawScriptWindowContents( FALSE, TRUE );
							DrawScreen( pdglLayouts );

							extension->bScriptWinDirtyBit = TRUE;
						}
					}
				}

				// Check whether we can Continue.

				if ( bCanContinue )
				{
					// Take care of the Cursor.

					if ( bScriptWin == FALSE )
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();
					}

					// Shift the Contents of the Line.

					if ( bScriptWin )
					{
						pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );

						ulChrsNum = ( MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1 ) - ( extension->ulScriptWinCol - 1 );
					}
					else
					{
						pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY );

						ulChrsNum = ulX1 - pdglLayouts->vivmVpcICEVideo.ulCursorX;
					}

					for ( ulI = 0; ulI < ulChrsNum; ulI ++, pwPtr ++ )
						* pwPtr = * ( pwPtr + 1 );
					* pwPtr = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

					// Draw the Updated Screen.

					if ( bScriptWin )
						DrawScriptWindowContents( TRUE, TRUE );

					DrawScreen( pdglLayouts );

					if ( bScriptWin )
						extension->bScriptWinDirtyBit = TRUE;
				}
			}
		}
		//
		// Home.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Home ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_HOME )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if ( bScriptWin || pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				// Take care of the Cursor.

				if ( bScriptWin )
				{
					extension->ulScriptWinCol = 1;
					DrawScriptWindowContents( TRUE, TRUE );

					MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

					DrawScreen( pdglLayouts );
				}
				else
				{
					if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
						MakeCursorBlink();

					pdglLayouts->vivmVpcICEVideo.ulCursorX = ulX0 + 1;
				}
			}
		}
		//
		// End.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_End ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_END )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if ( bScriptWin || pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				// Get the Final Position of the Cursor.

				if ( bScriptWin )
				{
					pwPtr = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1;

					ulI = MACRO_SCRIPTWIN_LINESIZE_IN_CHARS;
					ulA = 0;
				}
				else
				{
					pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
						ulX1, pdglLayouts->vivmVpcICEVideo.ulCursorY );

					ulI = ulX1;
					ulA = ulX0;
				}

				for ( ; ulI > ulA; ulI --, pwPtr -- )
					if ( * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						break;

				// Take care of the Cursor.

				if ( bScriptWin )
				{
					extension->ulScriptWinCol = ulI + 1;
					DrawScriptWindowContents( TRUE, TRUE );

					MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

					DrawScreen( pdglLayouts );
				}
				else
				{
					if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
						MakeCursorBlink();

					pdglLayouts->vivmVpcICEVideo.ulCursorX = ulI + 1;
				}
			}
		}
		//
		// Left Arrow.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Left ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_LEFT )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if ( bScriptWin || pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				if ( extension->sisSysInterrStatus.bControlPressed )
				{
					// Check whether we can Continue.

					bCanContinue = FALSE;

					if ( bScriptWin )
					{
						if ( extension->ulScriptWinCol > 1 )
						{
							bCanContinue = TRUE;
						}
						else
						{
							//
							// Modify the Position of the Cursor.
							//

							if ( extension->ulScriptWinLn > 1 )
							{
								ulC = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

								ulA = 0xFFFFFFFF;
								if ( extension->ulScriptWinLn - 2 < ulC )
									ulA = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLn - 2 ] );

								if ( ulA == 0xFFFFFFFF )
									ulB = 1;
								else
									ulB = ulA + 1;

								extension->ulScriptWinLn --;
								extension->ulScriptWinCol = ulB;
								DrawScriptWindowContents( TRUE, TRUE );

								MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

								DrawScreen( pdglLayouts );
							}
						}
					}
					else if ( pdglLayouts->vivmVpcICEVideo.ulCursorX > ulX0 + 1 )
					{
						bCanContinue = TRUE;
					}

					if ( bCanContinue )
					{
						WORD*			pwPtrStart;

						// Take care of the Cursor.

						if ( bScriptWin == FALSE )
						{
							if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
								MakeCursorBlink();
						}

						// Get the New Position of the Cursor.

						if ( bScriptWin )
						{
							pwPtrStart = extension->vwScriptWinLine;

							pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );
						}
						else
						{
							pwPtrStart = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
								ulX0 + 1, pdglLayouts->vivmVpcICEVideo.ulCursorY );

							pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
								pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY );
						}

						while( TRUE )
						{
							pwPtr --;

							if ( bScriptWin )
								extension->ulScriptWinCol --;
							else
								pdglLayouts->vivmVpcICEVideo.ulCursorX --;

							if ( pwPtr == pwPtrStart ||
								* pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
							{
								if ( pwPtr - 1 < pwPtrStart ||
									* ( pwPtr - 1 ) == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
										break;
							}
						}

						if ( bScriptWin )
						{
							DrawScriptWindowContents( TRUE, TRUE );

							MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

							DrawScreen( pdglLayouts );
						}
					}
				}
				else
				{
					// Check whether we can Continue.

					bCanContinue = FALSE;

					if ( bScriptWin )
					{
						if ( extension->ulScriptWinCol > 1 )
						{
							bCanContinue = TRUE;
						}
						else
						{
							//
							// Modify the Position of the Cursor.
							//

							if ( extension->ulScriptWinLn > 1 )
							{
								ulC = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

								ulA = 0xFFFFFFFF;
								if ( extension->ulScriptWinLn - 2 < ulC )
									ulA = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLn - 2 ] );

								if ( ulA == 0xFFFFFFFF )
									ulB = 1;
								else
									ulB = ulA + 1;

								extension->ulScriptWinLn --;
								extension->ulScriptWinCol = ulB;
								DrawScriptWindowContents( TRUE, TRUE );

								MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

								DrawScreen( pdglLayouts );
							}
						}
					}
					else if ( pdglLayouts->vivmVpcICEVideo.ulCursorX > ulX0 + 1 )
					{
						bCanContinue = TRUE;
					}

					if ( bCanContinue )
					{
						// Take care of the Cursor.

						if ( bScriptWin )
						{
							extension->ulScriptWinCol --;
							DrawScriptWindowContents( TRUE, TRUE );

							MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

							DrawScreen( pdglLayouts );
						}
						else
						{
							if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
								MakeCursorBlink();

							pdglLayouts->vivmVpcICEVideo.ulCursorX --;
						}
					}
				}
			}
		}
		//
		// Right Arrow.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Right ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_RIGHT )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			// If the "Cursor in Code Window Mode" is on, take care of the Cursor Position.

			if ( bScriptWin == FALSE && pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 )
			{
				// Modify the Position of the Cursor.

				if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
					MakeCursorBlink();

				pdglLayouts->vivmVpcICEVideo.ulCursorX = ulX0 + 1;
				pdglLayouts->vivmVpcICEVideo.ulCursorY = ulY1 - 1;

				// Disable the "Cursor in Code Window Mode".

				extension->bCursorInCodeWinMode = FALSE;
			}

			// Manage the Key Stroke.

			if ( extension->sisSysInterrStatus.bControlPressed )
			{
				ULONG			ulCursorEndX;

				// Get the Right Most Position in which the Cursor can stay.

				if ( bScriptWin )
				{
					pwPtr = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1;

					ulI = MACRO_SCRIPTWIN_LINESIZE_IN_CHARS;
					ulA = 0;
				}
				else
				{
					pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
						ulX1, pdglLayouts->vivmVpcICEVideo.ulCursorY );

					ulI = ulX1;
					ulA = ulX0;
				}

				for ( ; ulI > ulA; ulI --, pwPtr -- )
					if ( * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						break;

				ulCursorEndX = ulI + 1;

				// Get the New Position of the Cursor.

				bCanContinue = FALSE;

				if ( bScriptWin )
				{
					if ( extension->ulScriptWinCol < ulCursorEndX )
					{
						bCanContinue = TRUE;
					}
					else
					{
						//
						// Modify the Position of the Cursor.
						//

						extension->ulScriptWinLn ++;
						extension->ulScriptWinCol = 1;
						DrawScriptWindowContents( TRUE, TRUE );

						MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

						DrawScreen( pdglLayouts );
					}
				}
				else if ( pdglLayouts->vivmVpcICEVideo.ulCursorX < ulCursorEndX )
				{
					bCanContinue = TRUE;
				}

				if ( bCanContinue )
				{
					WORD*			pwPtrEnd;

					// Take care of the Cursor.

					if ( bScriptWin == FALSE )
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();
					}

					// Get the New Position of the Cursor.

					if ( bScriptWin )
					{
						pwPtrEnd = extension->vwScriptWinLine + ( ulCursorEndX - 1 );

						pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );
					}
					else
					{
						pwPtrEnd = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							ulCursorEndX, pdglLayouts->vivmVpcICEVideo.ulCursorY );

						pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY );
					}

					while( TRUE )
					{
						pwPtr ++;

						if ( bScriptWin )
							extension->ulScriptWinCol ++;
						else
							pdglLayouts->vivmVpcICEVideo.ulCursorX ++;

						if ( pwPtr == pwPtrEnd ||
							* pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						{
							if ( pwPtr + 1 >= pwPtrEnd ||
								* ( pwPtr + 1 ) == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
									break;
						}
					}

					if ( bScriptWin )
					{
						DrawScriptWindowContents( TRUE, TRUE );

						MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

						DrawScreen( pdglLayouts );
					}
				}
			}
			else
			{
				// Check whether we can Continue.

				bCanContinue = FALSE;

				if ( bScriptWin )
				{
					if ( extension->ulScriptWinCol < MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
					{
						bCanContinue = TRUE;
					}
					else
					{
						//
						// Modify the Position of the Cursor.
						//

						extension->ulScriptWinLn ++;
						extension->ulScriptWinCol = 1;
						DrawScriptWindowContents( TRUE, TRUE );

						MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

						DrawScreen( pdglLayouts );
					}
				}
				else if ( pdglLayouts->vivmVpcICEVideo.ulCursorX < ulX1 )
				{
					bCanContinue = TRUE;
				}

				if ( bCanContinue )
				{
					// Take care of the Cursor.

					if ( bScriptWin )
					{
						extension->ulScriptWinCol ++;
						DrawScriptWindowContents( TRUE, TRUE );

						MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

						DrawScreen( pdglLayouts );
					}
					else
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();

						pdglLayouts->vivmVpcICEVideo.ulCursorX ++;
					}
				}
			}
		}
		//
		// Insert.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Ins ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_INS )
			)
		{
			// Invert the Status of the Flag.

			if ( extension->sisSysInterrStatus.bTextInsert )
				extension->sisSysInterrStatus.bTextInsert = FALSE;
			else
				extension->sisSysInterrStatus.bTextInsert = TRUE;
		}
		//
		// Up Arrow.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Up ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_UP )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if (	bScriptWin == FALSE &&
					( extension->sisSysInterrStatus.bControlPressed ||
					pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 )
				)
			{
				VOID*			pvAddress;
				VPCICE_WINDOW*	pviwCodeWindow;
				VPCICE_WINDOW*	pviwOutputWindow;
				ULONG			ulSysStatusBarCurrX;
				BOOLEAN			bScrollCodePane;

				pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

				if ( pviwCodeWindow )
				{
					// Check if we are Browsing in the Code Window.

					if ( pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 )
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();

						if ( extension->sisSysInterrStatus.bControlPressed ||
							pdglLayouts->vivmVpcICEVideo.ulCursorY == pviwCodeWindow->ulY0 )
						{
							bScrollCodePane = TRUE;
						}
						else
						{
							bScrollCodePane = FALSE;
							pdglLayouts->vivmVpcICEVideo.ulCursorY --;
						}
					}
					else
					{
						bScrollCodePane = TRUE;
					}

					// Get the New Address in the Code Window.

					if ( bScrollCodePane )
					{
						pvAddress = AddressPrevInstruction( (VOID*) extension->dwCodeWindowPos, 0 );

						// Update the Code Window.

						if ( pvAddress )
						{
							extension->dwCodeWindowPos = (DWORD) pvAddress;

							pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
							if ( pviwOutputWindow )
							{
								ulSysStatusBarCurrX = extension->ulSysStatusBarModuleInfoStartX;
								PrintCodeWindowPosInModules( pviwOutputWindow, ulConsoleW, ulConsoleH, & ulSysStatusBarCurrX );
							}

							DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
							DrawScreen( pdglLayouts );
						}
					}
				}
			}
			else if (
					( bScriptWin == FALSE && extension->sisSysInterrStatus.bShiftPressed ) ||
					( bScriptWin && extension->sisSysInterrStatus.bAltPressed )
				)
			{
				VPCICE_WINDOW*			pviwOutputWindow;

				// Scroll the Output Window.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					extension->ulLineOffsetInOutputWin ++;
					DrawOutputWindowContents( pviwOutputWindow, ulConsoleW, ulConsoleH );
					DrawScreen( pdglLayouts );
				}
			}
			else if ( bScriptWin )
			{
				if ( extension->sisSysInterrStatus.bControlPressed )
				{
					if ( extension->ulScriptWinOffsetY )
					{
						extension->ulScriptWinOffsetY --;
						if ( extension->ulScriptWinLn > 1 )
							extension->ulScriptWinLn --;

						DrawScriptWindowContents( TRUE, TRUE );

						MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

						DrawScreen( pdglLayouts );
					}
				}
				else
				{
					if ( extension->ulScriptWinLn > 1 )
					{
						extension->ulScriptWinLn --;

						DrawScriptWindowContents( TRUE, TRUE );

						MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

						DrawScreen( pdglLayouts );
					}
				}
			}
			else
			{
				VPCICE_WINDOW*			pviwOutputWindow;
				CHAR*					pszNextMru;
				ULONG					ulMaxLineSize;
				ULONG					ulCursorDisp;

				// Get the Next MRU.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					pszNextMru = GetNextMruCommand( TRUE, pviwOutputWindow, ulConsoleW, ulConsoleH );
					if ( pszNextMru && strlen( pszNextMru ) )
					{
						// Print the Line.

						ulMaxLineSize = pviwOutputWindow->ulX1 - pviwOutputWindow->ulX0;

						OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							pviwOutputWindow->ulX0, pviwOutputWindow->ulY1,
							ulMaxLineSize,
							0x07, pszNextMru );

						// Take Care of the Cursor.

						ulCursorDisp = GetEscapedStringUnescapedLength( pszNextMru );
						if ( ulCursorDisp > ulMaxLineSize )
							ulCursorDisp = ulMaxLineSize;

						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();

						pdglLayouts->vivmVpcICEVideo.ulCursorX = pviwOutputWindow->ulX0 + ulCursorDisp;
						pdglLayouts->vivmVpcICEVideo.ulCursorY = pviwOutputWindow->ulY1;

						// Update the Screen.

						DrawScreen( pdglLayouts );
					}
				}
			}
		}
		//
		// Down Arrow.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_Down ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_DOWN )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if (	bScriptWin == FALSE &&
					( extension->sisSysInterrStatus.bControlPressed ||
					pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 )
				)
			{
				VOID*			pvAddress;
				VPCICE_WINDOW*	pviwCodeWindow;
				VPCICE_WINDOW*	pviwOutputWindow;
				ULONG			ulSysStatusBarCurrX;
				BOOLEAN			bScrollCodePane;

				pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

				if ( pviwCodeWindow )
				{
					// Check if we are Browsing in the Code Window.

					if ( pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 )
					{
						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();

						if ( extension->sisSysInterrStatus.bControlPressed ||
							pdglLayouts->vivmVpcICEVideo.ulCursorY == pviwCodeWindow->ulY1 )
						{
							bScrollCodePane = TRUE;
						}
						else
						{
							bScrollCodePane = FALSE;
							pdglLayouts->vivmVpcICEVideo.ulCursorY ++;
						}
					}
					else
					{
						bScrollCodePane = TRUE;
					}

					// Get the New Address in the Code Window.

					if ( bScrollCodePane )
					{
						pvAddress = AddressNextInstruction( (VOID*) extension->dwCodeWindowPos, 0 );

						// Update the Code Window.

						if ( pvAddress )
						{
							extension->dwCodeWindowPos = (DWORD) pvAddress;

							pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
							if ( pviwOutputWindow )
							{
								ulSysStatusBarCurrX = extension->ulSysStatusBarModuleInfoStartX;
								PrintCodeWindowPosInModules( pviwOutputWindow, ulConsoleW, ulConsoleH, & ulSysStatusBarCurrX );
							}

							DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
							DrawScreen( pdglLayouts );
						}
					}
				}
			}
			else if (
					( bScriptWin == FALSE && extension->sisSysInterrStatus.bShiftPressed ) ||
					( bScriptWin && extension->sisSysInterrStatus.bAltPressed )
				)
			{
				VPCICE_WINDOW*			pviwOutputWindow;

				// Scroll the Output Window.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					extension->ulLineOffsetInOutputWin --;
					DrawOutputWindowContents( pviwOutputWindow, ulConsoleW, ulConsoleH );
					DrawScreen( pdglLayouts );
				}
			}
			else if ( bScriptWin )
			{
				if ( extension->sisSysInterrStatus.bControlPressed )
				{
					extension->ulScriptWinOffsetY ++;
					extension->ulScriptWinLn ++;
				}
				else
				{
					extension->ulScriptWinLn ++;
				}

				DrawScriptWindowContents( TRUE, TRUE );

				MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

				DrawScreen( pdglLayouts );
			}
			else
			{
				VPCICE_WINDOW*			pviwOutputWindow;
				CHAR*					pszNextMru;
				ULONG					ulMaxLineSize;
				ULONG					ulCursorDisp;

				// Get the Next MRU.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					pszNextMru = GetNextMruCommand( FALSE, pviwOutputWindow, ulConsoleW, ulConsoleH );
					if ( pszNextMru && strlen( pszNextMru ) )
					{
						// Print the Line.

						ulMaxLineSize = pviwOutputWindow->ulX1 - pviwOutputWindow->ulX0;

						OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
							pviwOutputWindow->ulX0, pviwOutputWindow->ulY1,
							ulMaxLineSize,
							0x07, pszNextMru );

						// Take Care of the Cursor.

						ulCursorDisp = GetEscapedStringUnescapedLength( pszNextMru );
						if ( ulCursorDisp > ulMaxLineSize )
							ulCursorDisp = ulMaxLineSize;

						if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
							MakeCursorBlink();

						pdglLayouts->vivmVpcICEVideo.ulCursorX = pviwOutputWindow->ulX0 + ulCursorDisp;
						pdglLayouts->vivmVpcICEVideo.ulCursorY = pviwOutputWindow->ulY1;

						// Update the Screen.

						DrawScreen( pdglLayouts );
					}
				}
			}
		}
		//
		// Page Up.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_PgUp ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_PGUP )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if (	( bScriptWin && extension->sisSysInterrStatus.bControlPressed ) ||
					( bScriptWin == FALSE && ( extension->sisSysInterrStatus.bControlPressed ||
					pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 ) )
				)
			{
				VOID*			pvAddress;
				VPCICE_WINDOW*	pviwCodeWindow;
				VPCICE_WINDOW*	pviwOutputWindow;
				ULONG			ulSysStatusBarCurrX;

				pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

				if ( pviwCodeWindow )
				{
					// Get the New Address in the Code Window.

					pvAddress = AddressPrevInstruction( (VOID*) extension->dwCodeWindowPos,
						pviwCodeWindow->ulY1 - pviwCodeWindow->ulY0 );

					// Update the Code Window.

					if ( pvAddress )
					{
						extension->dwCodeWindowPos = (DWORD) pvAddress;

						pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
						if ( pviwOutputWindow )
						{
							ulSysStatusBarCurrX = extension->ulSysStatusBarModuleInfoStartX;
							PrintCodeWindowPosInModules( pviwOutputWindow, ulConsoleW, ulConsoleH, & ulSysStatusBarCurrX );
						}

						DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
						DrawScreen( pdglLayouts );
					}
				}
			}
			else if ( bScriptWin &&
				extension->sisSysInterrStatus.bAltPressed == FALSE )
			{
				ulA = pviwScriptWindow->ulY1 - pviwScriptWindow->ulY0 + 1;

				if ( extension->ulScriptWinOffsetY < ulA )
					extension->ulScriptWinOffsetY = 0;
				else
					extension->ulScriptWinOffsetY -= ulA;

				if ( extension->ulScriptWinLn <= ulA )
					extension->ulScriptWinLn = 1;
				else
					extension->ulScriptWinLn -= ulA;

				DrawScriptWindowContents( TRUE, TRUE );

				MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

				DrawScreen( pdglLayouts );
			}
			else
			{
				VPCICE_WINDOW*			pviwOutputWindow;

				// Scroll the Output Window.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					extension->ulLineOffsetInOutputWin += ( pviwOutputWindow->ulY1 - pviwOutputWindow->ulY0 );
					DrawOutputWindowContents( pviwOutputWindow, ulConsoleW, ulConsoleH );
					DrawScreen( pdglLayouts );
				}
			}
		}
		//
		// Page Down.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_PgDn ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_PGDN )
			)
		{
			if ( extension->sisSysInterrStatus.bShiftPressed )
				bKeepSelectingInScriptWin = TRUE;

			if (	( bScriptWin && extension->sisSysInterrStatus.bControlPressed ) ||
					( bScriptWin == FALSE && ( extension->sisSysInterrStatus.bControlPressed ||
					pdglLayouts->vivmVpcICEVideo.ulCursorY != ulY1 - 1 ) )
				)
			{
				VOID*			pvAddress;
				VPCICE_WINDOW*	pviwCodeWindow;
				VPCICE_WINDOW*	pviwOutputWindow;
				ULONG			ulSysStatusBarCurrX;

				pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

				if ( pviwCodeWindow )
				{
					// Get the New Address in the Code Window.

					pvAddress = AddressNextInstruction( (VOID*) extension->dwCodeWindowPos,
						pviwCodeWindow->ulY1 - pviwCodeWindow->ulY0 );

					// Update the Code Window.

					if ( pvAddress )
					{
						extension->dwCodeWindowPos = (DWORD) pvAddress;

						pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
						if ( pviwOutputWindow )
						{
							ulSysStatusBarCurrX = extension->ulSysStatusBarModuleInfoStartX;
							PrintCodeWindowPosInModules( pviwOutputWindow, ulConsoleW, ulConsoleH, & ulSysStatusBarCurrX );
						}

						DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
						DrawScreen( pdglLayouts );
					}
				}
			}
			else if ( bScriptWin &&
				extension->sisSysInterrStatus.bAltPressed == FALSE )
			{
				ulA = pviwScriptWindow->ulY1 - pviwScriptWindow->ulY0 + 1;

				extension->ulScriptWinOffsetY += ulA;
				extension->ulScriptWinLn += ulA;
				DrawScriptWindowContents( TRUE, TRUE );

				MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

				DrawScreen( pdglLayouts );
			}
			else
			{
				VPCICE_WINDOW*			pviwOutputWindow;

				// Scroll the Output Window.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					extension->ulLineOffsetInOutputWin -= ( pviwOutputWindow->ulY1 - pviwOutputWindow->ulY0 );
					DrawOutputWindowContents( pviwOutputWindow, ulConsoleW, ulConsoleH );
					DrawScreen( pdglLayouts );
				}
			}
		}
		//
		// Enter.
		//
		else if (
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
				bScanCode == MACRO_SCANCODE_ENTER ) ||
				( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
				bScanCode == MACRO_SCANCODE_EX_ENTERP )
			)
		{
			// Check whether in the Output Window.

			if ( bScriptWin )
			{
				WORD*			pwPtrBuff;
				WORD*			pwPtrEnd;

				//
				// Check whether we must include Spaces for the Line Paragraph.
				//

				pwPtr = extension->vwScriptWinLine;
				pwPtrEnd = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );

				ulA = 0;

				while( pwPtr < pwPtrEnd )
					if ( * pwPtr ++ == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						ulA ++;
					else
						break;

				//
				// Copy the Second Part of the Line in the Backup Buffer,
				//  update the Text and Add the Backup Buffer to the Script File...
				//

				pwPtrBuff = extension->vwScriptWinBackupBuffer;

				for ( ulI = 0; ulI < ulA; ulI ++ )
					* pwPtrBuff ++ = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

				pwPtr = extension->vwScriptWinLine + ( extension->ulScriptWinCol - 1 );
				pwPtrEnd = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1;

				while( pwPtr <= pwPtrEnd )
				{
					* pwPtrBuff = * pwPtr;
					* pwPtr = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

					++ pwPtrBuff;
					++ pwPtr;
				}

				while( -- pwPtrBuff >= extension->vwScriptWinBackupBuffer )	// # Consider that the Last Chr of the Line Buffer is always ' '... We don't need to
				{															//  write a Preventive NULL Chr in the Buffer because the String will be NULL Terminated in the 1st Interation. #
					if ( * pwPtrBuff == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						* pwPtrBuff = 0;
					else
						break;
				}

				extension->ulScriptWinCol = 1 + ulA;
				extension->ulScriptWinLn ++;

				EndScriptWindowLineEdit();
				AddScriptLine( extension->ulScriptWinLn - 1, extension->vwScriptWinBackupBuffer );
				DrawScriptWindowContents( FALSE, TRUE );

				DrawScreen( pdglLayouts );

				extension->bScriptWinDirtyBit = TRUE;
			}
			else
			{
				VPCICE_WINDOW*			pviwOutputWindow;

				// Scroll the Output Window.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					// Scroll.

					ScrollOutputWindow( pviwOutputWindow, ulConsoleW, ulConsoleH );
					DrawScreen( pdglLayouts );

					// Call the Handler.

					DispatchCommandToHandler();
				}
			}
		}
		//
		// Esc.
		//
		else if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_ESC )
		{
			// Check whether in the Output Window.

			if ( bScriptWin )
			{
				ProcessKeyStroke(
					MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE, MACRO_SCANCODE_F7 );
			}
			else
			{
				VPCICE_WINDOW*			pviwOutputWindow;

				// Scroll the Output Window.

				pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
				if ( pviwOutputWindow )
				{
					ScrollOutputWindow( pviwOutputWindow, ulConsoleW, ulConsoleH );
					DrawScreen( pdglLayouts );
				}
			}
		}
		//
		// F8 Key.
		//
		else if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_F8 )
		{
			// Trace Command.

			extension->sisSysInterrStatus.bIsSingleStepping = TRUE;
		}
		//
		// F4 Key.
		//
		else if ( bAsciiCode == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
			bScanCode == MACRO_SCANCODE_F4 )
		{
			BOOLEAN			bLinkRes = FALSE;

			//
			// Compile the Script File.
			//

			if ( FillCompilationBuffer() == FALSE )
			{
				//
				// Print an Error Message.
				//

				OutputPrint( FALSE, FALSE, "There is not enough memory." );
			}
			else
			{
				//
				// Compile the Script File.
				//

				OutputPrint( FALSE, FALSE, "\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4 compilation begins \xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4" );

				bLinkRes = LinkMiniCSource(
					extension->pbScriptWinObjectFileBuffer,
					extension->ulScriptWinObjectFileBufferSizeInBytes,
					& extension->ulScriptWinObjectFileBufferUsedBytes,
					(BYTE**) & extension->pfnScriptWinObjectFileInitFnPtr,
					NULL, NULL,
					(BYTE*) extension->pszScriptWinCompilationBuffer,
					strlen( extension->pszScriptWinCompilationBuffer ),
					TRUE, NULL );

				if ( bLinkRes == FALSE )
				{
					//
					// Compilation Error: reset the "Used Bytes" counter.
					//

					extension->ulScriptWinObjectFileBufferUsedBytes = 0;
				}
				else
				{
					//
					// Compilation OK: execute the Init function.
					//

					if ( extension->pfnScriptWinObjectFileInitFnPtr )
						extension->pfnScriptWinObjectFileInitFnPtr();

					//
					// The Script File is no more Dirty.
					//

					extension->bScriptWinDirtyBit = FALSE;
				}
			}

			//
			// Take care of the Collection of Macro Prototypes.
			//

			* extension->pszScriptWinCompilationBuffer = 0;

			if ( bLinkRes )
			{
				if ( CollectMacroPrototypes() == FALSE )
					* extension->pszScriptWinCompilationBuffer = 0;
			}
		}
		//
		// # # NOT RECOGNIZED ! # #
		//
		else
		{
			if ( bScriptWin )
			{
				//
				// By default, Keep the Text Selection in the Script Window.
				//

				bKeepSelectingInScriptWin = TRUE;
			}
		}
	}

	//
	// Manage the State of the Selection Area Flag of the Script Win.
	//

	if ( bKeepSelectingInScriptWin == FALSE &&
		extension->bScriptWinShowSelArea )
	{
		extension->bScriptWinShowSelArea = FALSE;
	}

	//
	// Update the Status Bar and the Script Window Function Name.
	//

	if ( bScriptWin )
	{
		if ( extension->bScriptWinShowSelArea ||
			SearchForFunctionName( extension->ulScriptWinLn - 1, sizeof( szFunctionName ), szFunctionName ) == FALSE )
		{
			strcpy( szFunctionName, "" );
		}

		if ( MACRO_CRTFN_NAME(stricmp)( szFunctionName, g_szCurrentFunctionName ) )
		{
			strcpy( g_szCurrentFunctionName, szFunctionName );
			bUpdateScriptWinLnColInfos = TRUE;
		}
	}

	if ( pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
	{
		DrawStatusBar( ulConsoleW, ulConsoleH );
		DrawScreen( pdglLayouts );
	}
	else if ( bScriptWin &&
		( bUpdateScriptWinLnColInfos || bOldScriptWinDirtyBit != extension->bScriptWinDirtyBit ) )
	{
		DrawStatusBar( ulConsoleW, ulConsoleH );
		DrawScriptWinLnColInfos();
		DrawScreen( pdglLayouts );

		bOldScriptWinDirtyBit = extension->bScriptWinDirtyBit;
	}
	
	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed &&
		bOldScriptWinDirtyBit != extension->bScriptWinDirtyBit )
	{
		DrawScriptWinLnColInfos();
		DrawScreen( pdglLayouts );
	}

	// If we are in the "Cursor in Code Window Mode", Manage the Position of the Cursor.

	if ( extension->bCursorInCodeWinMode )
	{
		VPCICE_WINDOW*			pviwCodeWindow;

		pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

		if ( pviwCodeWindow )
		{
			// Check the Cursor Line and see whether we have to Move the Cursor.

			if ( pdglLayouts->vivmVpcICEVideo.ulCursorY == ulY1 - 1 )
			{
				pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
					ulX1, pdglLayouts->vivmVpcICEVideo.ulCursorY );

				ulI = ulX1;

				for ( ; ulI > ulX0; ulI --, pwPtr -- )
					if ( * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						break;

				if ( ulI == ulX0 )
				{
					// Modify the Position of the Cursor.

					if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
						MakeCursorBlink();

					pdglLayouts->vivmVpcICEVideo.ulCursorX = pviwCodeWindow->ulX0;

					if ( extension->dwCodeWinLastCursorPosY == 0 )
						pdglLayouts->vivmVpcICEVideo.ulCursorY = pviwCodeWindow->ulY0;
					else
						pdglLayouts->vivmVpcICEVideo.ulCursorY = extension->dwCodeWinLastCursorPosY;
				}
			}
			else
			{
				// Remember the Position of the Cursor.

				extension->dwCodeWinLastCursorPosY = pdglLayouts->vivmVpcICEVideo.ulCursorY;
			}
		}
	}
	else
	{
		VPCICE_WINDOW*	pviwCodeWindow;

		pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

		// Make sure that the Cursor is over the Correct Line.

		if ( pviwCodeWindow && pviwCodeWindow->bDisplayed &&
			pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwCodeWindow->ulY0 &&
			pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwCodeWindow->ulY1 )
		{
			if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
				MakeCursorBlink();

			pdglLayouts->vivmVpcICEVideo.ulCursorX = ulX0 + 1;
			pdglLayouts->vivmVpcICEVideo.ulCursorY = ulY1 - 1;
		}
	}

	// Manage the State of the Cursor.

	if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
		MakeCursorBlink();

	// Return to the Caller.

	return;
}

#undef MACRO_SCRIPTWIN_UPDATE_SELAREA_STATE

//==================================
// OutputPrint Function Definition.
//==================================

BOOLEAN				g_bCanOutputPrint = FALSE;
static CHAR			g_szOutputPrintBuffer[ 2 * 1024 ];
static CHAR			g_szOutputPrintTemp[ 2 * 1024 ];

VOID __cdecl OutputPrint( IN BOOLEAN bContainsColorEscapes, IN BOOLEAN bAvoidDbgPrintRelay, IN CONST CHAR* pszFormat, ... )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	va_list					valList;
	ULONG					ulFinalStrSize;
	WORD					wFLAGS;
	BOOLEAN					bAvoidFnSync = FALSE;

	// === Check whether we can continue. ===

	if ( g_bCanOutputPrint == FALSE )
		return;

	if ( extension->sisSysInterrStatus.bInDebugger &&
		extension->bReportMode &&
		extension->bReportModeAborted )
	{
		return;
	}

	// === Synchronize the Execution of the Function. ===

	if ( extension->bInitializationDone )
		bAvoidDbgPrintRelay = TRUE;

	if ( extension->sisSysInterrStatus.bInDebugger &&
		extension->bReportMode )
	{
		bAvoidFnSync = TRUE;
	}

	if ( bAvoidDbgPrintRelay && bAvoidFnSync == FALSE )
	{
		__asm
		{
			pushf
			pop			ax
			mov			wFLAGS, ax
			cli
			mov			ebx, extension
			lea			ebx, [ ebx ]DEVICE_EXTENSION.mpslOutputPrintFunctionMutex
			call		EnterMultiProcessorSpinLock
		}
	}

	// === Format the Passed Arguments. ===

	va_start( valList, pszFormat );
	vsprintf( g_szOutputPrintBuffer, pszFormat, valList );
	va_end( valList );

	// === Debug Print the New String ===

	if ( bAvoidDbgPrintRelay == FALSE &&
		extension->sisSysInterrStatus.bInDebugger == FALSE )
	{
		if ( bContainsColorEscapes == FALSE )
		{
			strcpy( g_szOutputPrintTemp, g_szOutputPrintBuffer );
			strcat( g_szOutputPrintTemp, "\r\n" );
			DbgPrint( g_szOutputPrintTemp );
		}
		else
		{
			EliminateStringEscapes( g_szOutputPrintTemp, g_szOutputPrintBuffer );
			strcat( g_szOutputPrintTemp, "\r\n" );
			DbgPrint( g_szOutputPrintTemp );
		}
	}

	// === Save the Output Message. ===

	// Make sure that the Output String is correctly Escaped.

	if ( bContainsColorEscapes == FALSE )
	{
		PrepareUnEscapedString( g_szOutputPrintTemp, g_szOutputPrintBuffer );
		strcpy( g_szOutputPrintBuffer, g_szOutputPrintTemp );
	}

	// Check if there is space for the New String.

	ulFinalStrSize = strlen( g_szOutputPrintBuffer ) + sizeof( CHAR );

	while ( extension->ulOutputWinBufferPosInBytes + ulFinalStrSize > extension->ulOutputWinBufferSizeInBytes ||
		extension->ulOutputWinStrPtrsBufferPosInBytes + sizeof( CHAR* ) > extension->ulOutputWinStrPtrsBufferSizeInBytes )
	{
		ULONG			ulShiftAmount = extension->ppszOutputWinStrPtrsBuffer[ 1 ] - extension->ppszOutputWinStrPtrsBuffer[ 0 ];
		ULONG			ulI;
		CHAR*			pszFirstPtr;
		CHAR*			pszSecondPtr;
		ULONG			ulPtrsTblPos;
		CHAR**			ppszFirstPtr;
		CHAR**			ppszSecondPtr;

		// Shift the Contents of the Output Window Buffer.

		pszFirstPtr = & extension->pszOutputWinBuffer[ 0 ];
		pszSecondPtr = & extension->pszOutputWinBuffer[ ulShiftAmount ];

		for ( ulI = 0; ulI < extension->ulOutputWinBufferPosInBytes - ulShiftAmount; ulI ++ )
			* pszFirstPtr ++ = * pszSecondPtr ++;

		extension->ulOutputWinBufferPosInBytes -= ulShiftAmount;

		// Shift and Adjust the Contents of the Pointers Table.

		ulPtrsTblPos = extension->ulOutputWinStrPtrsBufferPosInBytes / sizeof( CHAR* );

		ppszFirstPtr = & extension->ppszOutputWinStrPtrsBuffer[ 0 ];
		ppszSecondPtr = & extension->ppszOutputWinStrPtrsBuffer[ 1 ];

		for ( ulI = 0; ulI < ulPtrsTblPos - 1; ulI ++ )
		{
			* ppszSecondPtr -= ulShiftAmount;
			* ppszFirstPtr ++ = * ppszSecondPtr ++;
		}

		extension->ulOutputWinStrPtrsBufferPosInBytes -= sizeof( CHAR* );
	}

	// Save the New String.

	extension->ppszOutputWinStrPtrsBuffer[ extension->ulOutputWinStrPtrsBufferPosInBytes / sizeof( CHAR* ) ] =
		& extension->pszOutputWinBuffer[ extension->ulOutputWinBufferPosInBytes ];
	extension->ulOutputWinStrPtrsBufferPosInBytes += sizeof( CHAR* );

	memcpy( & extension->pszOutputWinBuffer[ extension->ulOutputWinBufferPosInBytes ], g_szOutputPrintBuffer, ulFinalStrSize );
	extension->ulOutputWinBufferPosInBytes += ulFinalStrSize;

	// === Check if we have to Display the String in the Debugger. ===

	if ( extension->sisSysInterrStatus.bInDebugger )
	{
		DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
		ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
		VPCICE_WINDOW*			pviwOutputWindow;
		ULONG					ulLinesToMove, ulBytesToMove;
		ULONG					ulI;
		WORD*					pwPtrSource;
		WORD*					pwPtrDest;
		ULONG					ulConsoleX0, ulConsoleY0, ulConsoleX1, ulConsoleY1;
		BYTE					bLastAsciiCodeAcquired, bLastScanCodeAcquired;
		WORD*					pwPtr;
		BOOLEAN					bScrollWindow = TRUE;

		GetConsolePrintCoords( & ulConsoleX0, & ulConsoleY0, & ulConsoleX1, & ulConsoleY1 );

		// Scroll the Output Window and Draw the Last Line in the Window.

		pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );

		if ( pviwOutputWindow )
		{
			// Scroll the Window.

			if ( extension->bReportMode &&
				extension->bReportMode1StScrollDone == FALSE )
			{
				extension->bReportMode1StScrollDone = TRUE;
				bScrollWindow = FALSE;
			}

			if ( bScrollWindow )
			{
				ulLinesToMove = pviwOutputWindow->ulY1 - pviwOutputWindow->ulY0 - 1;
				ulBytesToMove = ( pviwOutputWindow->ulX1 - pviwOutputWindow->ulX0 + 1 ) * sizeof( WORD );

				if ( extension->bReportMode )
					ulLinesToMove ++;

				for ( ulI = 0; ulI < ulLinesToMove; ulI ++ )
				{
					// Scroll the Line.

					pwPtrSource = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
						pviwOutputWindow->ulX0, pviwOutputWindow->ulY0 + 1 + ulI );
					pwPtrDest = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
						pviwOutputWindow->ulX0, pviwOutputWindow->ulY0 + ulI );

					memcpy( pwPtrDest, pwPtrSource, ulBytesToMove );
				}
			}

			// Draw the Last Line.

			OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwOutputWindow->ulX0,
				extension->bReportMode == FALSE ? ( pviwOutputWindow->ulY1 - 1 ) : pviwOutputWindow->ulY1,
				pviwOutputWindow->ulX1 - pviwOutputWindow->ulX0 + 1,
				0x07, g_szOutputPrintBuffer );

			// Check whether in Report Mode: print the Status Bar message and modify the Cursor Position.

			if ( extension->bReportMode )
			{
				extension->ulReportModeLinesLeft --;

				if ( extension->ulReportModeLinesLeft == 0 &&
					extension->bReportModeMsgPrinted == FALSE )
				{
					// Erase the Status Bar contents.

					pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
						ulConsoleX0, ulConsoleY1 );
					for ( ulI = ulConsoleX0; ulI <= ulConsoleX1; ulI ++ )
						* pwPtr ++ = CLRTXT2VIDBUFWORD( 0, 3, 32 );

					// Print the Report Mode message.

					OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
						ulConsoleX0, ulConsoleY1, 0x30, MACRO_REPORTMODE_STATUSMSG );

					// Modify the Position of the Cursor.

					extension->ulReportModeOldCursorX = pdglLayouts->vivmVpcICEVideo.ulCursorX;
					extension->ulReportModeOldCursorY = pdglLayouts->vivmVpcICEVideo.ulCursorY;

					pdglLayouts->vivmVpcICEVideo.ulCursorX = pviwOutputWindow->ulX0;
					pdglLayouts->vivmVpcICEVideo.ulCursorY = pviwOutputWindow->ulY1;

					// Avoid re-Printing the Message.

					extension->bReportModeMsgPrinted = TRUE;
				}
			}

			// Draw the Updated Screen.

			DrawScreen( pdglLayouts );

			// Block the Execution if in Report Mode.

			if ( extension->bReportMode &&
				extension->ulReportModeLinesLeft == 0 )
			{
				// Wait for a Key Stroke.

				WaitForUserInput( & bLastAsciiCodeAcquired, & bLastScanCodeAcquired );

				// Update the "Lines Left" counter.

				if ( 
						( bLastAsciiCodeAcquired == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
						bLastScanCodeAcquired == MACRO_SCANCODE_ENTER ) ||
						( bLastAsciiCodeAcquired == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX &&
						bLastScanCodeAcquired == MACRO_SCANCODE_EX_ENTERP )
					)
				{
					extension->ulReportModeLinesLeft = 1;
				}
				else if (
							( bLastAsciiCodeAcquired == MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE &&
							bLastScanCodeAcquired == MACRO_SCANCODE_ESC )
						)
				{
					extension->bReportModeAborted = TRUE;
				}
				else
				{
					extension->ulReportModeLinesLeft = extension->ulReportModeLinesTotal;
				}
			}
		}
	}

	// === Increment the Line ID. ===

	extension->ulHistoryLineID ++;

	// === Synchronize the Execution of the Function. ===

	if ( bAvoidDbgPrintRelay && bAvoidFnSync == FALSE )
	{
		__asm
		{
			mov			ebx, extension
			lea			ebx, [ ebx ]DEVICE_EXTENSION.mpslOutputPrintFunctionMutex
			call		LeaveMultiProcessorSpinLock
			mov			ax, wFLAGS
			push		ax
			popf
		}
	}

	// === Return to the Caller. ===

	return;
}

//=========================================
// GetHistoryLineText Function Definition.
//=========================================

BOOLEAN GetHistoryLineText( IN OUT ULONG* pulIndexS, IN OUT ULONG* pulIndexE, OUT CHAR* pszBuffer, IN ULONG ulBuffSize )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					retval = FALSE;
	WORD					wFLAGS;
	ULONG					ulTotalLinesNum, ulFirstValidID, ulLastLineID;
	ULONG					Num, Index, I;
	CHAR*					out = pszBuffer;
	CHAR*					end = out + ulBuffSize - 1; // Last Valid Char...
	CHAR*					in;
	CHAR*					newout;

	if ( ulBuffSize == 0 )
		return FALSE;

	// Enter.

	__asm
	{
		pushf
		pop			ax
		mov			wFLAGS, ax
		cli
		mov			ebx, extension
		lea			ebx, [ ebx ]DEVICE_EXTENSION.mpslOutputPrintFunctionMutex
		call		EnterMultiProcessorSpinLock
	}

	//
	// Return the Text.
	//
	// NOTE: The First Line drawn in the History Window has ID = 1...
	//

	if ( extension->ulHistoryLineID )
	{
		ulTotalLinesNum = extension->ulOutputWinStrPtrsBufferPosInBytes / sizeof( CHAR* );

		ulLastLineID = extension->ulHistoryLineID;
		ulFirstValidID = ulLastLineID - ulTotalLinesNum + 1;

		if (
				( *pulIndexS >= ulFirstValidID && *pulIndexS <= ulLastLineID ) ||
				( *pulIndexE >= ulFirstValidID && *pulIndexE <= ulLastLineID ) ||
				( *pulIndexS < ulFirstValidID && *pulIndexE > ulLastLineID )
			)
		{
			// Correct the Variables.

			if ( *pulIndexS < ulFirstValidID )
				*pulIndexS = ulFirstValidID;

			if ( *pulIndexE > ulLastLineID )
				*pulIndexE = ulLastLineID;

			// Copy the Text.

			Num = *pulIndexE - *pulIndexS + 1;
			Index = *pulIndexS - ulFirstValidID;

			retval = TRUE;

			for ( I = 0; I < Num; I ++ )
			{
				in = extension->ppszOutputWinStrPtrsBuffer[ Index + I ];
				newout = out + strlen( in ) + 2;

				if ( newout > end )
				{
					retval = FALSE;
					break;
				}

				strcpy( out, in );
				strcat( out, "\x0D\x0A" );

				out = newout;
			}
		}
	}

	// Exit.

	__asm
	{
		mov			ebx, extension
		lea			ebx, [ ebx ]DEVICE_EXTENSION.mpslOutputPrintFunctionMutex
		call		LeaveMultiProcessorSpinLock
		mov			ax, wFLAGS
		push		ax
		popf
	}

	// Return.

	return retval;
}

//===========================================
// OutputVersionMessage Function Definition.
//===========================================

#include "..\Bin\BuildNumTracker\PROBUILD.H"

VOID OutputVersionMessage( VOID )
{
	// Output the Version Message of VpcICE.

	OutputPrint( TRUE, FALSE, MACRO_PROGRAM_NAME " 32bit Debugger for Win2000/XP/S2003 (UP/SMP) - !0B(Ver0.9 - Bld%s)!07.", __PROGRAMBUILD__ );
	OutputPrint( FALSE, FALSE, "Copyright (c) 2003-2004/2010 Vito Plantamura Consulting. All Rights Reserved." );

	// Return to the Caller.

	return;
}

#undef __PROGRAMBUILD__

//===============================================
// DrawOutputWindowContents Function Definition.
//===============================================

VOID DrawOutputWindowContents( IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulOutWinLinesNum, ulOutWinTotalLinesNum, ulMaxLineOffset, ulStrPtrsIndex;
	ULONG					ulI;

	// Calculate several Variables.

	ulOutWinLinesNum = pviwOutputWindow->ulY1 - pviwOutputWindow->ulY0;
	ulOutWinTotalLinesNum = extension->ulOutputWinStrPtrsBufferPosInBytes / sizeof( CHAR* );

	if ( ulOutWinTotalLinesNum < ulOutWinLinesNum )
		ulMaxLineOffset = 0;
	else
		ulMaxLineOffset = ulOutWinTotalLinesNum - ulOutWinLinesNum;

	// Validation.

	if ( extension->ulLineOffsetInOutputWin > 0x80000000 )
		extension->ulLineOffsetInOutputWin = 0;

	if ( extension->ulLineOffsetInOutputWin > ulMaxLineOffset )
		extension->ulLineOffsetInOutputWin = ulMaxLineOffset;

	// Print in the Output Window.

	ulStrPtrsIndex = ulOutWinTotalLinesNum - 1 - extension->ulLineOffsetInOutputWin;

	for ( ulI = 0; ulI < ulOutWinLinesNum; ulI ++ )
	{
		if ( ulStrPtrsIndex == 0xFFFFFFFF )
			break;

		OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwOutputWindow->ulX0, pviwOutputWindow->ulY1 - 1 - ulI,
			pviwOutputWindow->ulX1 - pviwOutputWindow->ulX0 + 1,
			0x07, extension->ppszOutputWinStrPtrsBuffer[ ulStrPtrsIndex -- ] );
	}

	// Return to the Caller.

	return;
}

//=========================================
// ScrollOutputWindow Function Definition.
//=========================================

static CHAR			g_szGrabbedLastLine[ 2 * 1024 ];

VOID ScrollOutputWindow( IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulI;
	WORD*					pwPtr;
	WORD*					pwLastLinePtr;
	WORD*					pwLastLinePtrEnd;
	ULONG					ulLastLineLengthInWords;
	CHAR*					pszPtr;

	// Update the Displacement of the Window.

	extension->ulLineOffsetInOutputWin = 0;
	extension->ulMruCommandOffsetFromEnd = 0;

	// Calculate the Address of the Last Line.

	pwLastLinePtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwOutputWindow->ulX0, pviwOutputWindow->ulY1 );
	pwLastLinePtrEnd = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwOutputWindow->ulX1, pviwOutputWindow->ulY1 );

	// Grab the Line of Text entered and Add it to the Output Log.

	pwPtr = pwLastLinePtrEnd;

	while( * pwPtr == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
		pwPtr --;

	ulLastLineLengthInWords = pwPtr - pwLastLinePtr + 1;

	pwPtr = pwLastLinePtr;
	pszPtr = g_szGrabbedLastLine;

	for ( ulI = 0; ulI < ulLastLineLengthInWords; ulI ++ )
		* pszPtr ++ = (BYTE) ( ( * pwPtr ++ ) & 0xFF );

	* pszPtr = '\0';

	OutputPrint( FALSE, TRUE, "%s", g_szGrabbedLastLine );

	// Reset the State of the Last Line.

	pwPtr = pwLastLinePtr;

	* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ':' );

	for ( ulI = pviwOutputWindow->ulX0 + 1; ulI <= pviwOutputWindow->ulX1; ulI ++ )
		* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	// Take care of the Cursor.

	if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
		MakeCursorBlink();

	pdglLayouts->vivmVpcICEVideo.ulCursorX = pviwOutputWindow->ulX0 + 1;
	pdglLayouts->vivmVpcICEVideo.ulCursorY = pviwOutputWindow->ulY1;

	// Return to the Caller.

	return;
}

//========================================
// GetNextMruCommand Function Definition.
//========================================

static CHAR			g_szGetNextMruCommandFnRetBuffer[ 2 * 1024 ];
static CHAR			g_szGetNextMruCommandTempBuffer[ 2 * 1024 ];

static VOID ValidateMruCommandOffsetFromEnd( IN ULONG ulPtrsTblPos )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	if ( extension->ulMruCommandOffsetFromEnd > 0x80000000 )
		extension->ulMruCommandOffsetFromEnd = 0;

	if ( extension->ulMruCommandOffsetFromEnd > ulPtrsTblPos - 1 )
		extension->ulMruCommandOffsetFromEnd = ulPtrsTblPos - 1;

	return;
}

CHAR* GetNextMruCommand( IN BOOLEAN bDirection, IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulPtrsTblPos;
	WORD*					pwPtr;
	WORD*					pwLastLinePtr;
	ULONG					ulLastLineLengthInWords;
	ULONG					ulI;
	CHAR*					pszPtr;
	ULONG					ulIterations;
	ULONG					ulIndex;

	// Check the Number of Entries in the Pointers Table.

	ulPtrsTblPos = extension->ulOutputWinStrPtrsBufferPosInBytes / sizeof( CHAR* );

	if ( ulPtrsTblPos == 0 )
	{
		strcpy( g_szGetNextMruCommandFnRetBuffer, "" );
		return g_szGetNextMruCommandFnRetBuffer;
	}

	// Validation.

	ValidateMruCommandOffsetFromEnd( ulPtrsTblPos );

	// Get the Line already Displayed in the Cursor Line.

	pwLastLinePtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwOutputWindow->ulX0, pviwOutputWindow->ulY1 );
	pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwOutputWindow->ulX1, pviwOutputWindow->ulY1 );

	while( * pwPtr == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
		pwPtr --;

	ulLastLineLengthInWords = pwPtr - pwLastLinePtr + 1;

	pwPtr = pwLastLinePtr;
	pszPtr = g_szGetNextMruCommandTempBuffer;

	for ( ulI = 0; ulI < ulLastLineLengthInWords; ulI ++ )
		* pszPtr ++ = (BYTE) ( ( * pwPtr ++ ) & 0xFF );

	* pszPtr = '\0';

	// Prepare the Cursor String.

	PrepareUnEscapedString( g_szGetNextMruCommandFnRetBuffer, g_szGetNextMruCommandTempBuffer );

	// Calculate the Number of Iterations.

	if ( bDirection )
		ulIterations = ulPtrsTblPos - extension->ulMruCommandOffsetFromEnd;
	else
		ulIterations = extension->ulMruCommandOffsetFromEnd + 1;

	// According to the Direction, Scan the List of String Pointers.

	for ( ulI = 0; ulI < ulIterations; ulI ++ )
	{
		// Check the Corresponding Entry.

		ulIndex = ulPtrsTblPos - 1 - extension->ulMruCommandOffsetFromEnd;

		if ( strlen( extension->ppszOutputWinStrPtrsBuffer[ ulIndex ] ) >= 2 &&
			extension->ppszOutputWinStrPtrsBuffer[ ulIndex ][ 0 ] == ':' &&
			strcmp( extension->ppszOutputWinStrPtrsBuffer[ ulIndex ], g_szGetNextMruCommandFnRetBuffer ) )
		{
			strcpy( g_szGetNextMruCommandFnRetBuffer, extension->ppszOutputWinStrPtrsBuffer[ ulIndex ] );
			return g_szGetNextMruCommandFnRetBuffer;
		}

		// Update the Offset.

		if ( bDirection )
			extension->ulMruCommandOffsetFromEnd ++;
		else
			extension->ulMruCommandOffsetFromEnd --;

		ValidateMruCommandOffsetFromEnd( ulPtrsTblPos );
	}

	// Return to the Caller.

	strcpy( g_szGetNextMruCommandFnRetBuffer, "" );
	return g_szGetNextMruCommandFnRetBuffer;
}

//==================================================
// DrawRegistersWindowContents Function Definition.
//==================================================

static CHAR			g_szDrawRegistersWindowContentsFnBuffer[ 1024 ];

VOID DrawRegistersWindowContents( IN VPCICE_WINDOW* pviwRegistersWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH, IN BOOLEAN bEraseMemHintSpace )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	x86_REGISTERS_CONTEXT*	px86vicContext = & extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext;
	x86_REGISTERS_CONTEXT*	px86vicPrevContext = & extension->sisSysInterrStatus.x86vicPrevContext;

	// === Draw the Window Lines. ===

	sprintf( g_szDrawRegistersWindowContentsFnBuffer, "%sEAX=%.8X!07   %sEBX=%.8X!07   %sECX=%.8X!07   %sEDX=%.8X!07   %sESI=%.8X!07",
		px86vicContext->EAX != px86vicPrevContext->EAX ? "!0B" : "", px86vicContext->EAX,
		px86vicContext->EBX != px86vicPrevContext->EBX ? "!0B" : "", px86vicContext->EBX,
		px86vicContext->ECX != px86vicPrevContext->ECX ? "!0B" : "", px86vicContext->ECX,
		px86vicContext->EDX != px86vicPrevContext->EDX ? "!0B" : "", px86vicContext->EDX,
		px86vicContext->ESI != px86vicPrevContext->ESI ? "!0B" : "", px86vicContext->ESI );

	OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwRegistersWindow->ulX0, pviwRegistersWindow->ulY0 + 0,
		pviwRegistersWindow->ulX1 - pviwRegistersWindow->ulX0 + 1,
		0x07, g_szDrawRegistersWindowContentsFnBuffer );

	sprintf( g_szDrawRegistersWindowContentsFnBuffer, "%sEDI=%.8X!07   %sEBP=%.8X!07   %sESP=%.8X!07   %sEIP=%.8X!07   %s%s!07 %s%s!07 %s%s!07 %s%s!07 %s%s!07 %s%s!07 %s%s!07 %s%s!07",
		px86vicContext->EDI != px86vicPrevContext->EDI ? "!0B" : "", px86vicContext->EDI,
		px86vicContext->EBP != px86vicPrevContext->EBP ? "!0B" : "", px86vicContext->EBP,
		px86vicContext->ESP != px86vicPrevContext->ESP ? "!0B" : "", px86vicContext->ESP,
		px86vicContext->EIP != px86vicPrevContext->EIP ? "!0B" : "", px86vicContext->EIP,
		( px86vicContext->EFLAGS & MACRO_OF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_OF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_OF_MASK ) ? "O" : "o",
		( px86vicContext->EFLAGS & MACRO_DF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_DF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_DF_MASK ) ? "D" : "d",
		( px86vicContext->EFLAGS & MACRO_IF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_IF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_IF_MASK ) ? "I" : "i",
		( px86vicContext->EFLAGS & MACRO_SF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_SF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_SF_MASK ) ? "S" : "s",
		( px86vicContext->EFLAGS & MACRO_ZF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_ZF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_ZF_MASK ) ? "Z" : "z",
		( px86vicContext->EFLAGS & MACRO_AF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_AF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_AF_MASK ) ? "A" : "a",
		( px86vicContext->EFLAGS & MACRO_PF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_PF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_PF_MASK ) ? "P" : "p",
		( px86vicContext->EFLAGS & MACRO_CF_MASK ) != ( px86vicPrevContext->EFLAGS & MACRO_CF_MASK ) ? "!0B" : "", ( px86vicContext->EFLAGS & MACRO_CF_MASK ) ? "C" : "c" );

	OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwRegistersWindow->ulX0, pviwRegistersWindow->ulY0 + 1,
		pviwRegistersWindow->ulX1 - pviwRegistersWindow->ulX0 + 1,
		0x07, g_szDrawRegistersWindowContentsFnBuffer );

	sprintf( g_szDrawRegistersWindowContentsFnBuffer, "%sCS=%.4X!07   %sDS=%.4X!07   %sSS=%.4X!07   %sES=%.4X!07   %sFS=%.4X!07   %sGS=%.4X!07%s",
		px86vicContext->CS != px86vicPrevContext->CS ? "!0B" : "", px86vicContext->CS,
		px86vicContext->DS != px86vicPrevContext->DS ? "!0B" : "", px86vicContext->DS,
		px86vicContext->SS != px86vicPrevContext->SS ? "!0B" : "", px86vicContext->SS,
		px86vicContext->ES != px86vicPrevContext->ES ? "!0B" : "", px86vicContext->ES,
		px86vicContext->FS != px86vicPrevContext->FS ? "!0B" : "", px86vicContext->FS,
		px86vicContext->GS != px86vicPrevContext->GS ? "!0B" : "", px86vicContext->GS,
		bEraseMemHintSpace ? "                       " /* 23 chrs. */ : "" );

	OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		pviwRegistersWindow->ulX0, pviwRegistersWindow->ulY0 + 2,
		pviwRegistersWindow->ulX1 - pviwRegistersWindow->ulX0 + 1,
		0x07, g_szDrawRegistersWindowContentsFnBuffer );

	// === Return to the Caller. ===

	return;
}

//=============================================
// DrawCodeWindowContents Function Definition.
//=============================================

static BYTE			g_vbCodePagesSnapshot[ MACRO_SYSTEM_PAGE_SIZE * 2 ];
static CHAR			g_szDrawCodeWindowContentsDisAsmBuffer[ 1024 ];
static CHAR			g_szDrawCodeWindowContentsTxtBuffer[ 1024 ];
static CHAR			g_szDrawCodeWindowContentsFinalLine[ 1024 ];
static CHAR			g_szDrawCodeWindowContentsMemHint[ 40 ];
static CHAR			g_szDrawCodeWindowContentsJumpHint[ 40 ];

static DWORD		g_vdwDrawCodeWindowContentsAddresses[ 256 ];
static ULONG		g_ulDrawCodeWindowContentsAddressesPos = 0;

static BYTE			g_vbMiniSnapshot[ 0x10 ];

VOID DrawCodeWindowContents( IN VPCICE_WINDOW* pviwCodeWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	x86_REGISTERS_CONTEXT*	px86vicContext = & extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].x86vicContext;

	DWORD					dwEipPageBase;

	ULONG					ulLinesToDraw;
	ULONG					ulI;
	DWORD					dwEipDisAsm;

	VPCICE_WINDOW*			pviwRegistersWindow;

	BOOLEAN					bJumpHintFnExecuted = FALSE;

	LONG					lLenDisAsm;

	// === Initialize. ===

	memset( & extension->daiHighlightedInstr, 0, sizeof( DISASM_INFO ) );

	extension->dwTargetAddress = 0;

	g_ulDrawCodeWindowContentsAddressesPos = 0;

	// === Read the Code Pages. ===

	memset( g_vbCodePagesSnapshot, 0xFF, sizeof( g_vbCodePagesSnapshot ) );

	dwEipPageBase = extension->dwCodeWindowPos & 0xFFFFF000;

	if ( IsPagePresent( (PVOID) dwEipPageBase ) )
		memcpy( g_vbCodePagesSnapshot, (PVOID) dwEipPageBase, MACRO_SYSTEM_PAGE_SIZE );

	if ( IsPagePresent( (PVOID) ( dwEipPageBase + MACRO_SYSTEM_PAGE_SIZE ) ) )
		memcpy( & g_vbCodePagesSnapshot[ MACRO_SYSTEM_PAGE_SIZE ],
			(PVOID) ( dwEipPageBase + MACRO_SYSTEM_PAGE_SIZE ),
			MACRO_SYSTEM_PAGE_SIZE );

	// === Draw the Window Lines. ===

	dwEipDisAsm = extension->dwCodeWindowPos;

	ulLinesToDraw = pviwCodeWindow->ulY1 - pviwCodeWindow->ulY0 + 1;

	for ( ulI = 0; ulI < ulLinesToDraw; ulI ++ )
	{
		ULONG			ulJ;
		CHAR			szTempBuffer[ 10 ];
		BOOLEAN			bBreakpoint = FALSE;
		BOOLEAN			bDetour = FALSE;
		BOOLEAN			bDetourExactAddress = FALSE;
		BYTE			bDefColor;

		// Check if we are on a Breakpoint.

		for ( ulJ = 0; ulJ < MACRO_MAXNUM_OF_BREAKPOINTS; ulJ ++ )
			if ( psisInts->vbpBreakpoints[ ulJ ].ulType == MACRO_BREAKPOINTTYPE_EXEC &&
				psisInts->vbpBreakpoints[ ulJ ].bDisabled == FALSE &&
				psisInts->vbpBreakpoints[ ulJ ].bIsContextCompatible &&
				psisInts->vbpBreakpoints[ ulJ ].dwAddress == dwEipDisAsm )
			{
				bBreakpoint = TRUE;
				break;
			}

		// Check if we are on a Detour.

		for ( ulJ = 0; ulJ < MACRO_MAXNUM_OF_DETOURS; ulJ ++ )
			if ( psisInts->vdDetours[ ulJ ].ulType == MACRO_DETOURTYPE_USER &&
				psisInts->vdDetours[ ulJ ].bIsContextCompatible &&
				dwEipDisAsm >= psisInts->vdDetours[ ulJ ].dwAddress &&
				dwEipDisAsm < psisInts->vdDetours[ ulJ ].dwAddress + psisInts->vdDetours[ ulJ ].ulDetourSize )
			{
				bDetour = TRUE;

				if ( psisInts->vdDetours[ ulJ ].dwAddress == dwEipDisAsm )
					bDetourExactAddress = TRUE;

				break;
			}

		// Try Disassembling.

		lLenDisAsm = disasm( & g_vbCodePagesSnapshot[ dwEipDisAsm - dwEipPageBase ], g_szDrawCodeWindowContentsDisAsmBuffer,
			32 /* BITS */, dwEipDisAsm, FALSE, 0 /* INTEL INSTR. SET */,
			dwEipDisAsm != px86vicContext->EIP ? NULL : & extension->daiHighlightedInstr );

		if ( lLenDisAsm == 0 )
		{
			lLenDisAsm = 1;
			strcpy( g_szDrawCodeWindowContentsDisAsmBuffer, "INVALID" );
		}

		// Compose the Final Line.

		sprintf( g_szDrawCodeWindowContentsTxtBuffer, "%.4X:%.8X\xB3 ", px86vicContext->CS, dwEipDisAsm );

		for ( ulJ = 0; ulJ < (ULONG) lLenDisAsm; ulJ ++ )
		{
			if ( ulJ == MACRO_CODEBYTES_MAXNUM )
				break;

			sprintf( szTempBuffer, "%.2X", g_vbCodePagesSnapshot[ dwEipDisAsm - dwEipPageBase + ulJ ] );
			strcat( g_szDrawCodeWindowContentsTxtBuffer, szTempBuffer );
		}

		while( ulJ < MACRO_CODEBYTES_MAXNUM )
		{
			strcat( g_szDrawCodeWindowContentsTxtBuffer, "  " );
			ulJ ++;
		}

		strcat( g_szDrawCodeWindowContentsTxtBuffer, "\xB3 " );

		_strupr( g_szDrawCodeWindowContentsDisAsmBuffer );
		strcat( g_szDrawCodeWindowContentsTxtBuffer, g_szDrawCodeWindowContentsDisAsmBuffer );

		// Apply Overlays, eventually.

		bDefColor = 0x07;

		if ( bBreakpoint )
		{
			bDefColor = 0x0B;
			memcpy( g_szDrawCodeWindowContentsTxtBuffer, "brkp", 4 );
		}
		else if ( bDetour )
		{
			bDefColor = 0x0C;

			if ( bDetourExactAddress )
				memcpy( g_szDrawCodeWindowContentsTxtBuffer, "dtur", 4 );
		}

		// Print the Final Line.

		PrepareUnEscapedString( g_szDrawCodeWindowContentsFinalLine, g_szDrawCodeWindowContentsTxtBuffer );

		OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwCodeWindow->ulX0, pviwCodeWindow->ulY0 + ulI,
			pviwCodeWindow->ulX1 - pviwCodeWindow->ulX0 + 1,
			( dwEipDisAsm != px86vicContext->EIP ) ? bDefColor : 0x71, g_szDrawCodeWindowContentsFinalLine );

		// Take care of the Jump Hint.

		if ( dwEipDisAsm == px86vicContext->EIP )
		{
			GetHighlightedInstructionJumpHint( g_szDrawCodeWindowContentsJumpHint, g_szDrawCodeWindowContentsDisAsmBuffer );

			if ( strlen( g_szDrawCodeWindowContentsJumpHint ) )
			{
				OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
					pviwCodeWindow->ulX0 + 66, pviwCodeWindow->ulY0 + ulI,
					pviwCodeWindow->ulX1 - (pviwCodeWindow->ulX0 + 66) + 1,
					0x70, g_szDrawCodeWindowContentsJumpHint );
			}

			bJumpHintFnExecuted = TRUE;
		}

		// Keep track of the Address of this Line.

		g_vdwDrawCodeWindowContentsAddresses[ g_ulDrawCodeWindowContentsAddressesPos ++ ] = dwEipDisAsm;

		// Increment.

		dwEipDisAsm += lLenDisAsm;
	}

	// Save the Eip Address of the First Hidden Instruction.

	extension->b1stHiddenInstrInfoIsValid = TRUE;
	extension->dw1stHiddenInstrAddress = dwEipDisAsm;

	// === Draw the Memory Hint in the Registers Window. ===

	pviwRegistersWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_REGISTERS, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwRegistersWindow )
	{
		OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwRegistersWindow->ulX0 + 60, pviwRegistersWindow->ulY0 + 2,
			pviwRegistersWindow->ulX1 - (pviwRegistersWindow->ulX0 + 60) + 1,
			0x07, "                       " /* 23 chrs. */ );

		GetHighlightedInstructionMemoryHint( g_szDrawCodeWindowContentsMemHint );

		if ( strlen( g_szDrawCodeWindowContentsMemHint ) )
		{
			OutputTextStringSpecial( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwRegistersWindow->ulX0 + 60, pviwRegistersWindow->ulY0 + 2,
				pviwRegistersWindow->ulX1 - (pviwRegistersWindow->ulX0 + 60) + 1,
				0x0B, g_szDrawCodeWindowContentsMemHint );
		}
	}

	// === Draw the Target Address Hint in the Code Window. ===

	if ( bJumpHintFnExecuted == FALSE )
	{
		if ( IsPagePresent_BYTERANGE( (BYTE*) px86vicContext->EIP, sizeof( g_vbMiniSnapshot ) ) )
		{
			memcpy( g_vbMiniSnapshot, (PVOID) px86vicContext->EIP, sizeof( g_vbMiniSnapshot ) );

			lLenDisAsm = disasm( g_vbMiniSnapshot, g_szDrawCodeWindowContentsDisAsmBuffer,
				32 /* BITS */, px86vicContext->EIP, FALSE, 0 /* INTEL INSTR. SET */,
				NULL );

			if ( lLenDisAsm )
				GetHighlightedInstructionJumpHint( g_szDrawCodeWindowContentsJumpHint, g_szDrawCodeWindowContentsDisAsmBuffer );
		}
	}

	if ( extension->dwTargetAddress )
	{
		for ( ulI = 0; ulI < g_ulDrawCodeWindowContentsAddressesPos; ulI ++ )
			if ( g_vdwDrawCodeWindowContentsAddresses[ ulI ] == extension->dwTargetAddress )
			{
				// Print the Hint.

				OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
					pviwCodeWindow->ulX0, pviwCodeWindow->ulY0 + ulI,
					extension->dwTargetAddress == px86vicContext->EIP ? 0x7B : 0x0B, " ==> " );

				break;
			}
	}

	// === Return to the Caller. ===

	return;
}

//===============================
// VpcICECommandHints Structure.
//===============================

typedef struct _VPCICE_COMMANDHINT
{
	CHAR*				pszCommand;
	CHAR*				pszDescHint;
	CHAR*				pszParamsHint;

	PFNCOMMANDHANDLER	pfnHandler;

} VPCICE_COMMANDHINT, *PVPCICE_COMMANDHINT;

static VPCICE_COMMANDHINT			g_vvichVpcICECommandHints[ 1024 ] =
	{
		{ "?", "Evaluate expression as dword (integer). Use EVALF for fp.", "? expression", & EvalExprCommandHandler },
		{ "BC", "Clear breakpoint.", "BC list|*", & BcCommandHandler },
		{ "BD", "Disable breakpoint.", "BD list|*", & BdCommandHandler },
		{ "BE", "Enable breakpoint.", "BE list|*", & BeCommandHandler },
		{ "BL", "List current breakpoints.", "BL (enter)", & BlCommandHandler },
		{ "BPE", "Edit breakpoint.", "BPE breakpoint-number", & BpeCommandHandler },
		{ "BPINT", "Breakpoint on interrupt.", "BPINT interrupt-number", & BpIntCommandHandler },
		{ "BPIO", "Breakpoint on I/O port access.", "BPIO port [/R|/W|/RW][/Lsize][/DRreg]", & BpIoCommandHandler },
		{ "BPMB", "Breakpoint on memory access (byte size).", "BPMB address [/W|/RW][/G][/DRreg]", & BpMbCommandHandler },
		{ "BPMD", "Breakpoint on memory access (dword size).", "BPMD address [/W|/RW][/G][/DRreg]", & BpMdCommandHandler },
		{ "BPMW", "Breakpoint on memory access (word size).", "BPMW address [/W|/RW][/G][/DRreg]", & BpMwCommandHandler },
		{ "BPX", "Breakpoint on execution.", "BPX address [/DRreg|none][/Pname|none][/Mname|none]", & BpxCommandHandler },
		{ "CPU", "Display informations about all the CPUs or a CPU in particular.", "CPU [(enter)|#cpu]", & CpuCommandHandler },
		{ "DASH", "Go to dashboard. The execution will return to Windows.", "DASH (enter)", & DashCommandHandler },
		// { "DTC", "Clear detour.", "DTC list|*", & DtcCommandHandler }, // DETOURLESS
		// { "DTX", "Detour on execution.", "DTX address [/Pname|none][/Mname|none]", & DtxCommandHandler }, // DETOURLESS
		{ "EC", "Enable/disable code window.", "EC (enter)", & EcCommandHandler },
		{ "EVALF", "Evaluate expression as floating point.", "EVALF expression", & EvalExprFloatCommandHandler },
		{ "HBOOT", "Reboot the system (total reset).", "HBOOT (enter)", & HbootCommandHandler },
		{ "I3HERE", "Direct INT3 to the debugger.", "I3HERE [ON|OFF|DRV]", & I3HereCommandHandler },
		{ "VERSION", "Display VpcICE version informations.", "VERSION (enter)", & VersionCommandHandler },

		{ NULL, NULL, NULL, & GenericCommandHandler }
	};

//=========================================
// DispatchCommandToHandler Function Data.
//=========================================

static PFNCOMMANDHANDLER		g_pfnDispatchCommandToHandlerFunction = NULL;
static CHAR						g_szDispatchCommandToHandlerLineParam[ 1024 ];
static CHAR						g_szDispatchCommandToHandlerCommandParam[ 1024 ];
static CHAR						g_szDispatchCommandToHandlerParamsParam[ 1024 ];

//===============================================
// DispatchCommandToHandler Function Definition.
//===============================================

VOID DispatchCommandToHandler( VOID )
{
	// Do the Requested Operation.

	if ( g_pfnDispatchCommandToHandlerFunction )
		g_pfnDispatchCommandToHandlerFunction(
			g_szDispatchCommandToHandlerLineParam,
			g_szDispatchCommandToHandlerCommandParam,
			g_szDispatchCommandToHandlerParamsParam );
}

//=================================
// DoAutoCompletion Function Data.
//=================================

static BOOLEAN					g_bAutoCompletionRequired = FALSE;
static VPCICE_COMMANDHINT*		g_pvichAutoCompletionHint;
static ULONG					g_ulAutoCompletionCurX;
static ULONG					g_ulAutoCompletionDestX, g_ulAutoCompletionDestY;

//=======================================
// DoAutoCompletion Function Definition.
//=======================================

BOOLEAN DoAutoCompletion( IN ULONG ulConsoleW, IN ULONG ulConsoleH )
{
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();

	// === Do Auto Completion, if Required. ===

	if ( g_bAutoCompletionRequired )
	{
		// Write the Command.

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			g_ulAutoCompletionDestX, g_ulAutoCompletionDestY, 0x07, g_pvichAutoCompletionHint->pszCommand );

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			g_ulAutoCompletionCurX, g_ulAutoCompletionDestY, 0x07, " " );

		// Take care of the Cursor.

		if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
			MakeCursorBlink();

		pdglLayouts->vivmVpcICEVideo.ulCursorX = g_ulAutoCompletionCurX;

		// Reset the Auto Completion Flag.

		g_bAutoCompletionRequired = FALSE;

		// Return to the Caller.

		return TRUE;
	}

	// === Return to the Caller. ===

	return FALSE;
}

//====================================
// DrawStatusBar Function Definition.
//====================================

static BOOLEAN		g_bMacroSyntaxCheckOutRes = FALSE;

static CHAR			g_szDrawStatusBarHintTxt[ 1024 ];
static CHAR			g_szDrawStatusBarUserTxt[ 1024 ];
static CHAR			g_szDrawStatusBarCmdTxt[ 1024 ];

#define MACRO_DRAWSTATUSBARHINTS_MAXITEMSNUM			0x20
static VPCICE_COMMANDHINT*			g_vpvichDrawStatusBarHints[ MACRO_DRAWSTATUSBARHINTS_MAXITEMSNUM ];

#define MACRO_DRAWSTATUSBAR_INVALIDCMD_STRING		"Unrecognized command. Type \"HELP\" for informations."
#define MACRO_DRAWSTATUSBAR_SCRIPTWND_STRING		"Press F7 to exit."
#define MACRO_DRAWSTATUSBAR_SCRIPTWND_STRING_COMP	"Press F4 to compile."

VOID DrawStatusBar( IN ULONG ulConsoleW, IN ULONG ulConsoleH )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulX0, ulY0, ulX1, ulY1;
	WORD*					pwPtr;
	CHAR					szProcessName[ 1 + 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
	ULONG					ulProcessNameLen;
	ULONG					ulI;
	CHAR*					pszCurrentProcessName;
	WORD*					pwLastLineStartPtr;
	WORD*					pwLastLineEndPtr;
	CHAR*					pszPtr;
	ULONG					ulProcessNameX;
	CHAR*					pszDot;
	BOOLEAN					bScriptWin = FALSE;
	VPCICE_WINDOW*			pviwScriptWindow;
	BOOLEAN					bUseColorStr = FALSE;

	// === Initialize the Auto-Completion Boolean. ===

	g_bAutoCompletionRequired = FALSE;

	// === Initialize the Dispatcher Pointer. ===

	g_pfnDispatchCommandToHandlerFunction = NULL;

	strcpy( g_szDispatchCommandToHandlerLineParam, "" );
	strcpy( g_szDispatchCommandToHandlerCommandParam, "" );
	strcpy( g_szDispatchCommandToHandlerParamsParam, "" );

	// === Check whether we are in the Script Window. ===

	pviwScriptWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_SCRIPT, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed &&
		pdglLayouts->vivmVpcICEVideo.ulCursorX != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwScriptWindow->ulY0 &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwScriptWindow->ulY1 )
	{
		bScriptWin = TRUE;
	}

	// === Get the Print Coords of the Console. ===

	GetConsolePrintCoords( & ulX0, & ulY0, & ulX1, & ulY1 );

	// === Get the Text in the User Bar. ===

	if ( bScriptWin == FALSE )
	{
		// Start and End Pointers.

		pwLastLineStartPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulX0 + 1, ulY1 - 1 );

		pwLastLineEndPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulX1, ulY1 - 1 );

		while( pwLastLineStartPtr < pwLastLineEndPtr &&
			* pwLastLineStartPtr == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
			pwLastLineStartPtr ++;

		while( * pwLastLineEndPtr == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
			pwLastLineEndPtr --;

		pwLastLineEndPtr ++;

		// Copy the Trimmed String.

		if ( pwLastLineStartPtr < pwLastLineEndPtr )
		{
			ULONG					ulLastLineLengthInWords;

			ulLastLineLengthInWords = pwLastLineEndPtr - pwLastLineStartPtr;

			pwPtr = pwLastLineStartPtr;
			pszPtr = g_szDrawStatusBarUserTxt;

			for ( ulI = 0; ulI < ulLastLineLengthInWords; ulI ++ )
				* pszPtr ++ = (BYTE) ( ( * pwPtr ++ ) & 0xFF );

			* pszPtr = '\0';
		}
		else
		{
			strcpy( g_szDrawStatusBarUserTxt, "" );
		}

		// Resolve the Macro Names.

		g_bMacroSyntaxCheckOutRes = CheckIfMacrosAreSyntaxedCorrectly( g_szDrawStatusBarUserTxt );

		// Keep a Copy for the Dispatcher.

		strcpy( g_szDispatchCommandToHandlerLineParam, g_szDrawStatusBarUserTxt );
	}

	// === Get the Hint Text. ===

	if ( bScriptWin )
	{
		if ( extension->bScriptWinDirtyBit )
		{
			strcpy( g_szDrawStatusBarHintTxt, "    " MACRO_DRAWSTATUSBAR_SCRIPTWND_STRING " !37" MACRO_DRAWSTATUSBAR_SCRIPTWND_STRING_COMP );
			bUseColorStr = TRUE;
		}
		else
		{
			strcpy( g_szDrawStatusBarHintTxt, "    " MACRO_DRAWSTATUSBAR_SCRIPTWND_STRING );
		}
	}
	else if ( strlen( g_szDrawStatusBarUserTxt ) == 0 )
	{
		if ( IsClientPollingForDebuggerVerb () )
			strcpy( g_szDrawStatusBarHintTxt, "     Enter a command. Type \"DASH\" for user-mode app." );
		else
			strcpy( g_szDrawStatusBarHintTxt, "     Enter a command for " MACRO_PROGRAM_NAME "." );
	}
	else
	{
		CHAR*					pszFirst;
		CHAR*					pszSecond;
		VPCICE_COMMANDHINT*		pvichCommandHintPtr;
		ULONG					ulCmdTxtLength;
		ULONG					ulHintsPos;
		VPCICE_COMMANDHINT*		pvichExactMatch;
		ULONG					ulCmdHintLength;

		// Get the Command Entered.

		pszFirst = g_szDrawStatusBarUserTxt;
		pszSecond = g_szDrawStatusBarCmdTxt;

		while( * pszFirst != '\0' &&
			* pszFirst != ' ' )
		{
			* pszSecond = * pszFirst;

			pszFirst ++;
			pszSecond ++;
		}

		* pszSecond = '\0';

		strcpy( g_szDispatchCommandToHandlerCommandParam, g_szDrawStatusBarCmdTxt );
		_strupr( g_szDrawStatusBarCmdTxt );

		// Keep a String indicating the Parameters of the Command for the Dispatcher.

		while( * pszFirst == ' ' )
			pszFirst ++;

		strcpy( g_szDispatchCommandToHandlerParamsParam, pszFirst );

		// Scan the List of Available Hints.

		ulHintsPos = 0;
		ulCmdTxtLength = strlen( g_szDrawStatusBarCmdTxt );
		pvichCommandHintPtr = g_vvichVpcICECommandHints;
		pvichExactMatch = NULL;

		while( pvichCommandHintPtr->pszCommand )
		{
			ulCmdHintLength = strlen( pvichCommandHintPtr->pszCommand );

			if ( ulCmdHintLength >= ulCmdTxtLength &&
				memcmp( pvichCommandHintPtr->pszCommand, g_szDrawStatusBarCmdTxt, ulCmdTxtLength ) == 0 )
			{
				if ( ulCmdHintLength == ulCmdTxtLength )
					pvichExactMatch = pvichCommandHintPtr;

				g_vpvichDrawStatusBarHints[ ulHintsPos ++ ] = pvichCommandHintPtr;
				if ( ulHintsPos == MACRO_DRAWSTATUSBARHINTS_MAXITEMSNUM )
					break;
			}

			pvichCommandHintPtr ++;
		}

		// Set the Pointer used by the Dispatcher.

		if ( pvichExactMatch )
			g_pfnDispatchCommandToHandlerFunction = pvichExactMatch->pfnHandler;
		else
			g_pfnDispatchCommandToHandlerFunction = pvichCommandHintPtr->pfnHandler;

		// Set the Hint Message according to the Number of Hints that match.

		if ( ulHintsPos == 0 )
		{
			strcpy( g_szDrawStatusBarHintTxt, "   " MACRO_DRAWSTATUSBAR_INVALIDCMD_STRING );
		}
		else
		{
			ULONG			ulCursorSpecialParamPosX;
			BOOLEAN			bSpecifyCommandList;

			bSpecifyCommandList = FALSE;

			// Check the Position of the Cursor.

			pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				ulX0, ulY1 - 1 );

			ulCursorSpecialParamPosX = ulX0 + ( pwLastLineStartPtr - pwPtr ) + ulCmdTxtLength;

			if ( pdglLayouts->vivmVpcICEVideo.ulCursorX <= ulCursorSpecialParamPosX )
			{
				if ( ulHintsPos == 1 && pvichExactMatch )
					strcpy( g_szDrawStatusBarHintTxt, pvichExactMatch->pszDescHint );
				else
					bSpecifyCommandList = TRUE;
			}
			else
			{
				if ( pvichExactMatch )
					strcpy( g_szDrawStatusBarHintTxt, pvichExactMatch->pszParamsHint );
				else if ( pdglLayouts->vivmVpcICEVideo.ulCursorX == ulCursorSpecialParamPosX + 1 )
					strcpy( g_szDrawStatusBarHintTxt, "   " MACRO_DRAWSTATUSBAR_INVALIDCMD_STRING );
				else
					bSpecifyCommandList = TRUE;
			}

			// Include the List of Command Hints, if required.

			if ( bSpecifyCommandList )
			{
				pszPtr = g_szDrawStatusBarHintTxt;

				for ( ulI = 0; ulI < ulHintsPos; ulI ++ )
				{
					ulCmdHintLength = strlen( g_vpvichDrawStatusBarHints[ ulI ]->pszCommand );

					if ( pszPtr - g_szDrawStatusBarHintTxt + ulCmdHintLength + 8 >= sizeof( g_szDrawStatusBarHintTxt ) )
						break;

					if ( ulI )
					{
						* pszPtr ++ = ',';
						* pszPtr ++ = ' ';
					}

					strcpy( pszPtr, g_vpvichDrawStatusBarHints[ ulI ]->pszCommand );
					pszPtr += ulCmdHintLength;
				}

				* pszPtr = '\0';
			}

			// Take care of the Auto-Completion Case.

			if ( pdglLayouts->vivmVpcICEVideo.ulCursorX == ulCursorSpecialParamPosX &&
				pvichExactMatch == NULL &&
				ulHintsPos == 1 )
			{
				ULONG			ulLineReqsForAutoCompletionX;

				// Check whether there is Enough Space.

				pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
					ulX0, ulY1 - 1 );

				ulLineReqsForAutoCompletionX = ulX0 + ( pwLastLineStartPtr - pwPtr ) +
					strlen( g_vpvichDrawStatusBarHints[ 0 ]->pszCommand );

				if ( ulLineReqsForAutoCompletionX < ulX1 )
				{
					g_bAutoCompletionRequired = TRUE;
					g_pvichAutoCompletionHint = g_vpvichDrawStatusBarHints[ 0 ];
					g_ulAutoCompletionCurX = ulLineReqsForAutoCompletionX + 1;
					g_ulAutoCompletionDestX = ulX0 + ( pwLastLineStartPtr - pwPtr );
					g_ulAutoCompletionDestY = ulY1 - 1;
				}
			}
		}
	}

	// === Draw the Status Bar. ===

	// Get the Name of the Current Process.

	pszCurrentProcessName = GetImageFileNameFieldPtrOfCurrProc( extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase );

	memset( szProcessName, 0, sizeof( szProcessName ) );

	szProcessName[ 0 ] = ' ';
	if ( pszCurrentProcessName )
		memcpy( szProcessName + 1, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );
	else
		strcpy( szProcessName + 1, "???" );

	pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
	if ( pszDot )
		* pszDot = 0;

	ulProcessNameLen = strlen( szProcessName );

	// Draw the Bar.

	//

	pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		ulX0, ulY1 );
	for ( ulI = ulX0; ulI <= ulX1; ulI ++ )
		* pwPtr ++ = CLRTXT2VIDBUFWORD( 0, 3, 32 );

	//

	ulProcessNameX = ulX1 - ulProcessNameLen;

	if ( strlen( g_szDrawStatusBarHintTxt ) > ulProcessNameX - ulX0 )
		strcpy( g_szDrawStatusBarHintTxt + ( ulProcessNameX - ulX0 ) - 3, "..." );

	if ( bUseColorStr )
	{
		OutputTextStringSpecial(
			pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulX0, ulY1, 0xFFFFFFFF /* very high line size */,
			0x30, g_szDrawStatusBarHintTxt );
	}
	else
	{
		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulX0, ulY1, 0x30, g_szDrawStatusBarHintTxt );
	}

	OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		ulProcessNameX, ulY1, 0x30, szProcessName );

	//

	// === Return to the Caller. ===

	return;
}

//============================================
// VersionCommandHandler Function Definition.
//============================================

VOID VersionCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	// Handle the Command.

	OutputVersionMessage();

	// Return to the Caller.

	return;
}

//============================================
// GenericCommandHandler Function Definition.
//============================================

VOID GenericCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	VPCICE_COMMANDHINT*		pvichCommandHintPtr;
	ULONG					ulCmdHintLength, ulLineLength;
	VPCICE_COMMANDHINT*		pvichMatch;
	ULONG					ulMatchLength;
	DWORD					dwRes;
	BOOLEAN					bEvalRes;
	CHAR					szBin[ 128 ];

	// Take care of Special Cases.

	if ( MACRO_CRTFN_NAME(stricmp)( pszLine, MACRO_SNAPSHOTMODE_ON_STRING ) == 0 )
	{
		extension->bSnapShotMode = TRUE;
		OutputPrint( FALSE, FALSE, "Snapshot mode is on." );
		return;
	}
	else if ( MACRO_CRTFN_NAME(stricmp)( pszLine, MACRO_SNAPSHOTMODE_OFF_STRING ) == 0 )
	{
		extension->bSnapShotMode = FALSE;
		OutputPrint( FALSE, FALSE, "Snapshot mode is off." );
		return;
	}

	// Check whether the User typed the line without Spaces.

	pvichCommandHintPtr = g_vvichVpcICECommandHints;
	ulLineLength = strlen( pszLine );

	pvichMatch = NULL;
	ulMatchLength = 0;

	while( pvichCommandHintPtr->pszCommand )
	{
		ulCmdHintLength = strlen( pvichCommandHintPtr->pszCommand );

		if ( ulLineLength > ulCmdHintLength )
		{
			if ( ! MACRO_CRTFN_NAME(memicmp)( pszLine, pvichCommandHintPtr->pszCommand, ulCmdHintLength ) &&
				ulCmdHintLength > ulMatchLength )
			{
				// Track this Match.

				pvichMatch = pvichCommandHintPtr;
				ulMatchLength = ulCmdHintLength;
			}
		}

		pvichCommandHintPtr ++;
	}

	if ( pvichMatch )
	{
		// Fill the Params and Call the Handler.

		g_pfnDispatchCommandToHandlerFunction = pvichMatch->pfnHandler;
		memcpy( g_szDispatchCommandToHandlerCommandParam, pszLine, ulMatchLength );
		strcpy( g_szDispatchCommandToHandlerParamsParam, pszLine + ulMatchLength );

		DispatchCommandToHandler();

		return;
	}

	// Check whether the Expression needs to be Resolved.

	if ( pszLine[ 0 ] == '?' ||
		g_bMacroSyntaxCheckOutRes )
	{
		//
		// Resolve the Expression and Print the Result.
		//

		bEvalRes = FALSE;

		dwRes = EvaluateExpression_DWORD(
			pszLine[ 0 ] == '?' ? pszLine + 1 : pszLine,
			& bEvalRes );

		if ( bEvalRes )
			OutputPrint( FALSE, FALSE, "result: 0x%X. (decimal: %i, binary: %s)",
				dwRes, dwRes, ultobinstr( szBin, dwRes ) );

		return;
	}

	// Output an Error Message.

	OutputPrint( FALSE, FALSE, "%s", MACRO_DRAWSTATUSBAR_INVALIDCMD_STRING );

	// Return to the Caller.

	return;
}

//==============================================
// System Flags "Bit to String" Mapping Tables.
//==============================================

typedef struct _BITtoSTRING
{
	DWORD			dwMask;
	CHAR*			pszString;

} BITtoSTRING, *PBITtoSTRING;

#define BITtoSTRING_entry( bit, str )			{ ( 1<<bit ), str }

static BITtoSTRING			g_vbtsCR0Bits[] =
{
	BITtoSTRING_entry( 0, "PE" ),
	BITtoSTRING_entry( 1, "MP" ),
	BITtoSTRING_entry( 2, "EM" ),
	BITtoSTRING_entry( 3, "TS" ),
	BITtoSTRING_entry( 4, "ET" ),
	BITtoSTRING_entry( 5, "NE" ),
	BITtoSTRING_entry( 16, "WP" ),
	BITtoSTRING_entry( 18, "AM" ),
	BITtoSTRING_entry( 29, "NW" ),
	BITtoSTRING_entry( 30, "CD" ),
	BITtoSTRING_entry( 31, "PG" ),

	{ 0, NULL }
};

static BITtoSTRING			g_vbtsCR3Bits[] =
{
	BITtoSTRING_entry( 3, "PWT" ),
	BITtoSTRING_entry( 4, "PCD" ),

	{ 0, NULL }
};

static BITtoSTRING			g_vbtsCR4Bits[] =
{
	BITtoSTRING_entry( 0, "VME" ),
	BITtoSTRING_entry( 1, "PVI" ),
	BITtoSTRING_entry( 2, "TSD" ),
	BITtoSTRING_entry( 3, "DE" ),
	BITtoSTRING_entry( 4, "PSE" ),
	BITtoSTRING_entry( 5, "PAE" ),
	BITtoSTRING_entry( 6, "MCE" ),
	BITtoSTRING_entry( 7, "PGE" ),
	BITtoSTRING_entry( 8, "PCE" ),
	BITtoSTRING_entry( 9, "OSFXSR" ),
	BITtoSTRING_entry( 10, "OSXMMEXCPT" ),

	{ 0, NULL }
};

static BITtoSTRING			g_vbtsEFLBits[] =
{
	BITtoSTRING_entry( 0, "CF" ),
	BITtoSTRING_entry( 2, "PF" ),
	BITtoSTRING_entry( 4, "AF" ),
	BITtoSTRING_entry( 6, "ZF" ),
	BITtoSTRING_entry( 7, "SF" ),
	BITtoSTRING_entry( 8, "TF" ),
	BITtoSTRING_entry( 9, "IF" ),
	BITtoSTRING_entry( 10, "DF" ),
	BITtoSTRING_entry( 11, "OF" ),
	BITtoSTRING_entry( 14, "NT" ),
	BITtoSTRING_entry( 16, "RF" ),
	BITtoSTRING_entry( 17, "VM" ),
	BITtoSTRING_entry( 18, "AC" ),
	BITtoSTRING_entry( 19, "VIF" ),
	BITtoSTRING_entry( 20, "VIP" ),
	BITtoSTRING_entry( 21, "ID" ),

	{ 0, NULL }
};

static BITtoSTRING			g_vbtsCPUID1Bits[] =
{
	BITtoSTRING_entry( 0, "FPU" ),
	BITtoSTRING_entry( 1, "VME" ),
	BITtoSTRING_entry( 2, "DE" ),
	BITtoSTRING_entry( 3, "PSE" ),
	BITtoSTRING_entry( 4, "TSC" ),
	BITtoSTRING_entry( 5, "MSR" ),
	BITtoSTRING_entry( 6, "PAE" ),
	BITtoSTRING_entry( 7, "MCE" ),
	BITtoSTRING_entry( 8, "CX8" ),
	BITtoSTRING_entry( 9, "APIC" ),
	BITtoSTRING_entry( 11, "SEP" ),
	BITtoSTRING_entry( 12, "MTRR" ),
	BITtoSTRING_entry( 13, "PGE" ),
	BITtoSTRING_entry( 14, "MCA" ),
	BITtoSTRING_entry( 15, "CMOV" ),
	BITtoSTRING_entry( 16, "PAT" ),
	BITtoSTRING_entry( 17, "PSE-36" ),
	BITtoSTRING_entry( 18, "PSN" ),
	BITtoSTRING_entry( 19, "CLFSH" ),
	BITtoSTRING_entry( 21, "DS" ),
	BITtoSTRING_entry( 22, "ACPI" ),
	BITtoSTRING_entry( 23, "MMX" ),
	BITtoSTRING_entry( 24, "FXSR" ),
	BITtoSTRING_entry( 25, "SSE" ),
	BITtoSTRING_entry( 26, "SSE2" ),
	BITtoSTRING_entry( 27, "SS" ),
	BITtoSTRING_entry( 28, "HTT" ),
	BITtoSTRING_entry( 29, "TM" ),
	BITtoSTRING_entry( 31, "PBE" ),

	{ 0, NULL }
};

static BITtoSTRING* GenerateBitMaskStringRep( OUT CHAR* pszString, IN ULONG ulMaxToBePrinted, IN DWORD dwDword, IN BITtoSTRING* pbtsMap )
{
	ULONG			ulI;
	ULONG			ulLen;

	strcpy( pszString, "" );

	// Generate the String Representation.

	ulI = 0;

	for ( ; pbtsMap->pszString != NULL; pbtsMap ++ )
	{
		if ( dwDword & pbtsMap->dwMask )
		{
			if ( ulI )
				strcat( pszString, " " );

			strcat( pszString, pbtsMap->pszString );

			ulI ++;
			if ( ulI == ulMaxToBePrinted )
			{
				pbtsMap ++;
				break;
			}
		}
	}

	// Eliminate the Ending Space.

	ulLen = strlen( pszString );

	if ( ulLen )
		pszString[ ulLen - 1 ] = '\0';

	// Return to the Caller.

	return pbtsMap;
}

//========================================
// CpuCommandHandler Function Definition.
//========================================

static CHAR			g_szCpuCommandHandlerTempBuffer[ 1024 ];

VOID CpuCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	CHAR*					pszDot;

	// Check whether there are Parameters.

	if ( strlen( pszParams ) == 0 )
	{
		ULONG				ulI;
		x86_CPU_CONTEXT*	px86ccThis;
		CHAR				szIrql[ 20 ];
		KIRQL				kilIRQL;
		CHAR				szIrqlReprBuffer[ 20 ];
		CHAR*				pszIrqlRepr;
		CHAR*				pszCurrentProcessName;
		CHAR				szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE

		//
		// ### ENTERING REPORT MODE ###
		//

		EnteringReportMode();

		// Display informations about All the CPUs.

		OutputPrint( FALSE, FALSE, "CPU\xB3PCR-Base\xB3IDT-Base\xB3GDT-Base\xB3IRQL(Priority)\xB3 TID\xB3       CS:EIP\xB3Proc(ID)" );

		for ( ulI = 0; ulI < psisInts->dwNumberOfProcessors; ulI ++ )
		{
			px86ccThis = & psisInts->vx86ccProcessors[ ulI ];

			kilIRQL = GetIrqlFromTPRValue( ulI == psisInts->dwCurrentProcessor ? psisInts->dwLocalApicTPR : px86ccThis->dwTPRValue );

			switch( kilIRQL )
			{
			case PASSIVE_LEVEL:
				pszIrqlRepr = " PASSIVE";
				break;

			case APC_LEVEL:
				pszIrqlRepr = "     APC";
				break;

			case DISPATCH_LEVEL:
				pszIrqlRepr = "DISPATCH";
				break;

			case PROFILE_LEVEL:
				pszIrqlRepr = " PROFILE";
				break;

			case CLOCK2_LEVEL:
				pszIrqlRepr = "  CLOCK2";
				break;

			case IPI_LEVEL:
				pszIrqlRepr = "     IPI";
				break;

			case POWER_LEVEL:
				pszIrqlRepr = "   POWER";
				break;

			case HIGH_LEVEL:
				pszIrqlRepr = "    HIGH";
				break;

			default:
				sprintf( szIrqlReprBuffer, "      %.2X", kilIRQL );
				pszIrqlRepr = szIrqlReprBuffer;
				break;
			}

			sprintf( szIrql, "  %s(%.2X)", pszIrqlRepr,
				ulI == psisInts->dwCurrentProcessor ? psisInts->dwLocalApicTPR : px86ccThis->dwTPRValue );

			//

			pszCurrentProcessName = GetImageFileNameFieldPtrOfCurrProc( px86ccThis->pvPCRBase );

			memset( szProcessName, 0, sizeof( szProcessName ) );

			if ( pszCurrentProcessName )
				memcpy( szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );
			else
				strcpy( szProcessName, "???" );

			pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
			if ( pszDot )
				* pszDot = 0;

			//

			OutputPrint( TRUE, FALSE, "%s%.2X \xB3%.8X\xB3%.8X\xB3%.8X\xB3%s\xB3%.4X\xB3%s%.4X:%.8X%s\xB3%s(%.4X)",
				( ulI == psisInts->dwCurrentProcessor ) ? "!27" : "!07", ulI,
				px86ccThis->pvPCRBase, px86ccThis->pvIDTBase, px86ccThis->pvGDTBase,
				szIrql, GetCurrentTID( px86ccThis->pvPCRBase ),
				( ulI == psisInts->dwCurrentProcessor ) ? "!2B" : "!0B",
				px86ccThis->x86vicContext.CS, px86ccThis->x86vicContext.EIP,
				( ulI == psisInts->dwCurrentProcessor ) ? "!27" : "!07",
				szProcessName, GetCurrentPID( px86ccThis->pvPCRBase ) );
		}

		//
		// ### LEAVING REPORT MODE ###
		//

		ExitingReportMode();
	}
	else
	{
		DWORD				dwCPUNum;
		x86_CPU_CONTEXT*	px86ccCPU;
		BITtoSTRING*		pbtsMapPtr;
		BOOLEAN				bEvalRes;

		// Display informations about the Specified CPU.

		dwCPUNum = EvaluateExpression_DWORD( pszParams, & bEvalRes );

		if ( bEvalRes )
		{
			if ( dwCPUNum >= psisInts->dwNumberOfProcessors )
			{
				// Print an Error Message.

				OutputPrint( FALSE, FALSE, "Invalid CPU Id." );
			}
			else
			{
				//
				// ### ENTERING REPORT MODE ###
				//

				EnteringReportMode();

				// Print the Requested Informations.

				px86ccCPU = & psisInts->vx86ccProcessors[ dwCPUNum ];

				OutputPrint( FALSE, FALSE, "" );
				OutputPrint( TRUE, FALSE, "         !71 Processor %.2X Registers !07", dwCPUNum );
				OutputPrint( FALSE, FALSE, "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD" );
				OutputPrint( FALSE, FALSE, "CS:EIP=%.4X:%.8X  SS:ESP=%.4X:%.8X",
					px86ccCPU->x86vicContext.CS, px86ccCPU->x86vicContext.EIP, px86ccCPU->x86vicContext.SS, px86ccCPU->x86vicContext.ESP );
				OutputPrint( FALSE, FALSE, "EAX=%.8X  EBX=%.8X  ECX=%.8X  EDX=%.8X",
					px86ccCPU->x86vicContext.EAX, px86ccCPU->x86vicContext.EBX, px86ccCPU->x86vicContext.ECX, px86ccCPU->x86vicContext.EDX );
				OutputPrint( FALSE, FALSE, "ESI=%.8X  EDI=%.8X  EBP=%.8X  EFL=%.8X",
					px86ccCPU->x86vicContext.ESI, px86ccCPU->x86vicContext.EDI, px86ccCPU->x86vicContext.EBP, px86ccCPU->x86vicContext.EFLAGS );
				OutputPrint( FALSE, FALSE, "DS=%.4X  ES=%.4X  FS=%.4X  GS=%.4X",
					px86ccCPU->x86vicContext.DS, px86ccCPU->x86vicContext.ES, px86ccCPU->x86vicContext.FS, px86ccCPU->x86vicContext.GS );
				OutputPrint( FALSE, FALSE, "" );

				GenerateBitMaskStringRep( g_szCpuCommandHandlerTempBuffer, 0, px86ccCPU->dwCR0, g_vbtsCR0Bits );
				OutputPrint( FALSE, FALSE, "CR0=%.8X %s", px86ccCPU->dwCR0, g_szCpuCommandHandlerTempBuffer );

				OutputPrint( FALSE, FALSE, "CR2=%.8X", px86ccCPU->dwCR2 );

				GenerateBitMaskStringRep( g_szCpuCommandHandlerTempBuffer, 0, px86ccCPU->dwCR3, g_vbtsCR3Bits );
				OutputPrint( FALSE, FALSE, "CR3=%.8X %s", px86ccCPU->dwCR3, g_szCpuCommandHandlerTempBuffer );

				GenerateBitMaskStringRep( g_szCpuCommandHandlerTempBuffer, 0, px86ccCPU->dwCR4, g_vbtsCR4Bits );
				OutputPrint( FALSE, FALSE, "CR4=%.8X %s", px86ccCPU->dwCR4, g_szCpuCommandHandlerTempBuffer );

				OutputPrint( FALSE, FALSE, "DR0=%.8X", px86ccCPU->dwDR0 );
				OutputPrint( FALSE, FALSE, "DR1=%.8X", px86ccCPU->dwDR1 );
				OutputPrint( FALSE, FALSE, "DR2=%.8X", px86ccCPU->dwDR2 );
				OutputPrint( FALSE, FALSE, "DR3=%.8X", px86ccCPU->dwDR3 );

				OutputPrint( FALSE, FALSE, "DR6=%.8X", px86ccCPU->dwDR6 );
				OutputPrint( FALSE, FALSE, "DR7=%.8X", px86ccCPU->dwDR7 );

				GenerateBitMaskStringRep( g_szCpuCommandHandlerTempBuffer, 0, px86ccCPU->x86vicContext.EFLAGS, g_vbtsEFLBits );

				if ( strlen( g_szCpuCommandHandlerTempBuffer ) )
					strcat( g_szCpuCommandHandlerTempBuffer, " " );

				strcat( g_szCpuCommandHandlerTempBuffer, "IOPL=" );
				sprintf( & g_szCpuCommandHandlerTempBuffer[ strlen( g_szCpuCommandHandlerTempBuffer ) ], "%i", ( px86ccCPU->x86vicContext.EFLAGS >> 12 ) & 0x3 );

				OutputPrint( FALSE, FALSE, "EFL=%.8X %s", px86ccCPU->x86vicContext.EFLAGS, g_szCpuCommandHandlerTempBuffer );

				OutputPrint( FALSE, FALSE, "" );

				OutputPrint( FALSE, FALSE, "Family:%X  Model:%X  Stepping:%X  Type:%X  Features:%.8X",
					( px86ccCPU->dwCpuIdInfo[ 0 ] >> 8 ) & 0xF,
					( px86ccCPU->dwCpuIdInfo[ 0 ] >> 4 ) & 0xF,
					px86ccCPU->dwCpuIdInfo[ 0 ] & 0xF,
					( px86ccCPU->dwCpuIdInfo[ 0 ] >> 12 ) & 0x3,
					px86ccCPU->dwCpuIdInfo[ 3 ] );

				pbtsMapPtr = g_vbtsCPUID1Bits;
				while( TRUE )
				{
					pbtsMapPtr = GenerateBitMaskStringRep( g_szCpuCommandHandlerTempBuffer, 8, px86ccCPU->dwCpuIdInfo[ 3 ], pbtsMapPtr );
					if ( strlen( g_szCpuCommandHandlerTempBuffer ) == 0 )
						break;

					OutputPrint( FALSE, FALSE, " > %s", g_szCpuCommandHandlerTempBuffer );
				}

				if ( psisInts->pvLocalApicMemoryPtr )
				{
					OutputPrint( FALSE, FALSE, "" );

					OutputPrint( TRUE, FALSE, "         !71 Local APIC (at 0x%.8X) !07", psisInts->pvLocalApicMemoryPtr );
					OutputPrint( FALSE, FALSE, "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD" );

					OutputPrint( FALSE, FALSE, "                   ID: %.8X", px86ccCPU->dwIDRValue );
					OutputPrint( FALSE, FALSE, "              Version: %.8X", px86ccCPU->dwVerRValue );
					OutputPrint( FALSE, FALSE, "        Task Priority: %.8X", dwCPUNum == psisInts->dwCurrentProcessor ? psisInts->dwLocalApicTPR : px86ccCPU->dwTPRValue );
					OutputPrint( FALSE, FALSE, " Arbitration Priority: %.8X", px86ccCPU->dwAPRValue );
					OutputPrint( FALSE, FALSE, "   Processor Priority: %.8X", px86ccCPU->dwPPRValue );
					OutputPrint( FALSE, FALSE, "   Destination Format: %.8X", px86ccCPU->dwDFRValue );
					OutputPrint( FALSE, FALSE, "  Logical Destination: %.8X", px86ccCPU->dwLDRValue );
					OutputPrint( FALSE, FALSE, "      Spurious Vector: %.8X", px86ccCPU->dwSVRValue );
					OutputPrint( FALSE, FALSE, "    Interrupt Command: %.8X%.8X", px86ccCPU->dwICRValue[ 0 ], px86ccCPU->dwICRValue[ 1 ] );
					OutputPrint( FALSE, FALSE, "          LVT (Timer): %.8X", px86ccCPU->dwLVTTRValue );
					OutputPrint( FALSE, FALSE, "  LVT (Perf. Counter): %.8X", px86ccCPU->dwLVTPMCRValue );
					OutputPrint( FALSE, FALSE, "          LVT (LINT0): %.8X", px86ccCPU->dwLVTLINT0RValue );
					OutputPrint( FALSE, FALSE, "          LVT (LINT1): %.8X", px86ccCPU->dwLVTLINT1RValue );
					OutputPrint( FALSE, FALSE, "          LVT (Error): %.8X", px86ccCPU->dwLVTErrRValue );
					OutputPrint( FALSE, FALSE, "          Timer Count: %.8X", px86ccCPU->dwICountRValue );
					OutputPrint( FALSE, FALSE, "        Timer Current: %.8X", px86ccCPU->dwCCountRValue );
					OutputPrint( FALSE, FALSE, "         Timer Divide: %.8X", px86ccCPU->dwDivCRValue );
				}

				//
				// ### LEAVING REPORT MODE ###
				//

				ExitingReportMode();
			}
		}
	}

	// Return to the Caller.

	return;
}

//=================================
// MACRO_MINICPROLOGUE Definition.
//=================================
	//
	// ## WARNING ## Don't USE here, in This Text, the Word "macro",
	//  because the Macro Prototypes List (that Includes MACRO_MINICPROLOGUE) is Searched for It !!
	//
	// ## note ##: IF YOU ADD SOMETHING HERE, you should consider adding it also in "MACRO_MINICINCLUDE" !!
	//
#define MACRO_MINICPROLOGUE \
"\
   #define VOID void\x0D\x0A\
   #define DWORD unsigned long\x0D\x0A\
   VOID ______eval_dword______( DWORD dwValue );\x0D\x0A\
   VOID ______eval_dword_2______( DWORD dwValue0, DWORD dwValue1 );\x0D\x0A\
   VOID ______eval_double______( double dValue );\x0D\x0A\
\x0D\x0A\
"

//=================================================
// g_szSingleFnPrototypePattern String Definition.
//=================================================

#define MACRO_MINIC_SINGLE_FNNAME		"______single_function______"

#define MACRO_SINGLEFNPROTOTYPEPATTERN(pfform) \
"\
   VOID ______single_function______( VOID )\x0D\x0A\
   {\x0D\x0A\
      " pfform " \x0D\x0A\
   }\x0D\x0A\
"

// // //

#define MACRO_SINGLEFNPROTOTYPESTRING_ADDITIONALSPACE		256

static CHAR		g_szSingleFnPrototypePattern[] = /*MACRO_MINICPROLOGUE*/ MACRO_SINGLEFNPROTOTYPEPATTERN("%s%s%s");
static CHAR		g_szSingleFnPrototypePattern_5[] = /*MACRO_MINICPROLOGUE*/ MACRO_SINGLEFNPROTOTYPEPATTERN("%s%s%s%s%s");
static CHAR		g_szSingleFnPrototypeString[ sizeof( /*MACRO_MINICPROLOGUE*/ MACRO_SINGLEFNPROTOTYPEPATTERN("xxxxxxxxxxxx") ) + MACRO_SINGLEFNPROTOTYPESTRING_ADDITIONALSPACE ];

//===============================================
// EvaluateExpression_DWORD Function Definition.
//===============================================

// // //

static BOOLEAN			g_bEvalDwordFnCalled;
static DWORD			g_dwEvalDwordValue;

static VOID __cdecl ______eval_dword______( DWORD dwValue )
{
	g_bEvalDwordFnCalled = TRUE;
	g_dwEvalDwordValue = dwValue;
	return;
}

// // //

DWORD EvaluateExpression_DWORD( IN CHAR* pszString, OUT BOOLEAN* pbRes )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bLinkRes;
	PFN_INITIALIZATION		pfnInitFnPtr;
	PFN_SINGLEFUNCTION		pfnSingleFnPtr;
	ULONG					ulBuffLen;

	* pbRes = FALSE;

	//
	// Compile and Link the MiniC Source.
	//  Manage also the State of the Buffer used for storing the Macro Prototypes List.
	//

	sprintf( g_szSingleFnPrototypeString, g_szSingleFnPrototypePattern, "______eval_dword______( (DWORD) ( ", pszString, " ) );" );

	ulBuffLen = strlen( extension->pszScriptWinCompilationBuffer );
	if ( ulBuffLen + strlen( g_szSingleFnPrototypeString ) >= extension->ulScriptWinCompilationBufferSizeInBytes )
	{
		OutputPrint( FALSE, FALSE, "Memory not enough." );
		return 0;
	}

	if ( ulBuffLen == 0 )
		strcpy( extension->pszScriptWinCompilationBuffer, MACRO_MINICPROLOGUE );
	strcat( extension->pszScriptWinCompilationBuffer, g_szSingleFnPrototypeString );

		//

		bLinkRes = LinkMiniCSource( NULL, 0, NULL,
			(BYTE**) & pfnInitFnPtr,
			MACRO_MINIC_SINGLE_FNNAME, (BYTE**) & pfnSingleFnPtr,
			(BYTE*) extension->pszScriptWinCompilationBuffer, strlen( extension->pszScriptWinCompilationBuffer ),
			FALSE, extension->ulScriptWinObjectFileBufferUsedBytes ? extension->pbScriptWinObjectFileBuffer : NULL );

		//

	extension->pszScriptWinCompilationBuffer[ ulBuffLen ] = 0;

	//
	// Check whether the Compilation Succeeded.
	//

	if ( bLinkRes == FALSE )
		return 0;

	if ( pfnInitFnPtr )
		pfnInitFnPtr();

	g_bEvalDwordFnCalled = FALSE;

	if ( pfnSingleFnPtr )
		pfnSingleFnPtr();

	if ( g_bEvalDwordFnCalled )
	{
		* pbRes = TRUE;
		return g_dwEvalDwordValue;
	}
	else
	{
		return 0;
	}
}

//==========================================================
// EvaluateMultipleExpressions_DWORD_2 Function Definition.
//==========================================================

// // //

static BOOLEAN			g_bEvalDword_2_FnCalled;
static DWORD			g_vdwEvalDword_2_Values[ 2 ];

static VOID __cdecl ______eval_dword_2______( DWORD dwValue0, DWORD dwValue1 )
{
	g_bEvalDword_2_FnCalled = TRUE;
	g_vdwEvalDword_2_Values[ 0 ] = dwValue0;
	g_vdwEvalDword_2_Values[ 1 ] = dwValue1;
	return;
}

// // //

BOOLEAN EvaluateMultipleExpressions_DWORD_2( IN CHAR* pszString0, IN CHAR* pszString1, OUT DWORD* pdwDword0, OUT DWORD* pdwDword1 )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bLinkRes;
	PFN_INITIALIZATION		pfnInitFnPtr;
	PFN_SINGLEFUNCTION		pfnSingleFnPtr;
	ULONG					ulBuffLen;

	//
	// Compile and Link the MiniC Source.
	//  Manage also the State of the Buffer used for storing the Macro Prototypes List.
	//

	sprintf( g_szSingleFnPrototypeString, g_szSingleFnPrototypePattern_5, "______eval_dword_2______( (DWORD) ( ", pszString0, " ), (DWORD) ( ", pszString1, " ) );" );

	ulBuffLen = strlen( extension->pszScriptWinCompilationBuffer );
	if ( ulBuffLen + strlen( g_szSingleFnPrototypeString ) >= extension->ulScriptWinCompilationBufferSizeInBytes )
	{
		OutputPrint( FALSE, FALSE, "Memory not enough." );
		return FALSE;
	}

	if ( ulBuffLen == 0 )
		strcpy( extension->pszScriptWinCompilationBuffer, MACRO_MINICPROLOGUE );
	strcat( extension->pszScriptWinCompilationBuffer, g_szSingleFnPrototypeString );

		//

		bLinkRes = LinkMiniCSource( NULL, 0, NULL,
			(BYTE**) & pfnInitFnPtr,
			MACRO_MINIC_SINGLE_FNNAME, (BYTE**) & pfnSingleFnPtr,
			(BYTE*) extension->pszScriptWinCompilationBuffer, strlen( extension->pszScriptWinCompilationBuffer ),
			FALSE, extension->ulScriptWinObjectFileBufferUsedBytes ? extension->pbScriptWinObjectFileBuffer : NULL );

		//

	extension->pszScriptWinCompilationBuffer[ ulBuffLen ] = 0;

	//
	// Check whether the Compilation Succeeded.
	//

	if ( bLinkRes == FALSE )
		return FALSE;

	if ( pfnInitFnPtr )
		pfnInitFnPtr();

	g_bEvalDword_2_FnCalled = FALSE;

	if ( pfnSingleFnPtr )
		pfnSingleFnPtr();

	if ( g_bEvalDword_2_FnCalled )
	{
		* pdwDword0 = g_vdwEvalDword_2_Values[ 0 ];
		* pdwDword1 = g_vdwEvalDword_2_Values[ 1 ];

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//================================================
// EvaluateExpression_DOUBLE Function Definition.
//================================================

// // //

static BOOLEAN			g_bEvalDoubleFnCalled;
static double			g_dEvalDoubleValue;

static VOID __cdecl ______eval_double______( double dValue )
{
	g_bEvalDoubleFnCalled = TRUE;
	g_dEvalDoubleValue = dValue;
	return;
}

// // //

double EvaluateExpression_DOUBLE( IN CHAR* pszString, OUT BOOLEAN* pbRes )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bLinkRes;
	PFN_INITIALIZATION		pfnInitFnPtr;
	PFN_SINGLEFUNCTION		pfnSingleFnPtr;
	ULONG					ulBuffLen;

	* pbRes = FALSE;

	//
	// Compile and Link the MiniC Source.
	//  Manage also the State of the Buffer used for storing the Macro Prototypes List.
	//

	sprintf( g_szSingleFnPrototypeString, g_szSingleFnPrototypePattern, "______eval_double______( (double) ( ", pszString, " ) );" );

	ulBuffLen = strlen( extension->pszScriptWinCompilationBuffer );
	if ( ulBuffLen + strlen( g_szSingleFnPrototypeString ) >= extension->ulScriptWinCompilationBufferSizeInBytes )
	{
		OutputPrint( FALSE, FALSE, "Memory not enough." );
		return 0;
	}

	if ( ulBuffLen == 0 )
		strcpy( extension->pszScriptWinCompilationBuffer, MACRO_MINICPROLOGUE );
	strcat( extension->pszScriptWinCompilationBuffer, g_szSingleFnPrototypeString );

		//

		bLinkRes = LinkMiniCSource( NULL, 0, NULL,
			(BYTE**) & pfnInitFnPtr,
			MACRO_MINIC_SINGLE_FNNAME, (BYTE**) & pfnSingleFnPtr,
			(BYTE*) extension->pszScriptWinCompilationBuffer, strlen( extension->pszScriptWinCompilationBuffer ),
			FALSE, extension->ulScriptWinObjectFileBufferUsedBytes ? extension->pbScriptWinObjectFileBuffer : NULL );

		//

	extension->pszScriptWinCompilationBuffer[ ulBuffLen ] = 0;

	//
	// Check whether the Compilation Succeeded.
	//

	if ( bLinkRes == FALSE )
		return 0;

	if ( pfnInitFnPtr )
		pfnInitFnPtr();

	g_bEvalDoubleFnCalled = FALSE;

	if ( pfnSingleFnPtr )
		pfnSingleFnPtr();

	if ( g_bEvalDoubleFnCalled )
	{
		* pbRes = TRUE;
		return g_dEvalDoubleValue;
	}
	else
	{
		return 0;
	}
}

//====================================
// AddressPrevInstruction Structures.
//====================================

typedef struct _SYNCPOINT
{
	ULONG				ulDisp, ulHits;
	struct _SYNCPOINT*	pspNext;

} SYNCPOINT, *PSYNCPOINT;

#define MACRO_ASSIGNEDNUM_OF_SYNCPOINTS		128
#define MACRO_MAXNUM_OF_SYNCPOINTS			0x2000

static SYNCPOINT		g_vspSyncPoints[ MACRO_MAXNUM_OF_SYNCPOINTS ];
static ULONG			g_ulSyncPointsPos = 0;

static DWORD			g_vdwInstrsLen[ MACRO_ASSIGNEDNUM_OF_SYNCPOINTS ];

#define MACRO_NUM_OF_PREVINSTR_TOPARSE		26

//============================================
// AddressPrevInstruction Function Definition.
//============================================

VOID* AddressPrevInstruction( IN VOID* pvRefPointer, IN ULONG ulInstrOrdinal )
{
	ULONG			ulI, ulJ, ulK;
	BYTE*			pbPos;
	BYTE*			pbPosFromRefPtr;
	DWORD			dwPageBase, dwPageBaseNew;
	SYNCPOINT*		pspPtr;
	ULONG			ulHitsMax;
	SYNCPOINT*		pspHitsMaxSyncP;
	BYTE*			pbRef;
	LONG			lLenDisAsm;
	ULONG			ulInstrsLenPos;
	BOOLEAN			bExitFromLoop;
	ULONG			ulInstrLimit;
	ULONG			ulDisp;

	// Calculate the Instruction Limit.

	ulInstrLimit = ulInstrOrdinal + MACRO_NUM_OF_PREVINSTR_TOPARSE;

	if ( ulInstrLimit >= MACRO_ASSIGNEDNUM_OF_SYNCPOINTS )
		return NULL;

	// Initialize the Structure.

	g_ulSyncPointsPos = MACRO_ASSIGNEDNUM_OF_SYNCPOINTS;

	for ( ulI = 0; ulI < MACRO_ASSIGNEDNUM_OF_SYNCPOINTS; ulI ++ )
	{
		pspPtr = & g_vspSyncPoints[ ulI ];

		pspPtr->ulDisp = 0;
		pspPtr->ulHits = 0;
		pspPtr->pspNext = NULL;
	}

	// Check for all the Bytes that are before the Reference Pointer.

	bExitFromLoop = FALSE;

	dwPageBase = 0xFFFFFFFF;

	for ( ulI = 1; ulI < MACRO_SYSTEM_PAGE_SIZE; ulI ++ )
	{
		pbPosFromRefPtr = ( (BYTE*) pvRefPointer ) - ulI;

		// === Read the Code Pages. ===

		// Calculate the new Page Base.

		dwPageBaseNew = ( (DWORD) pbPosFromRefPtr ) & 0xFFFFF000;

		// If the Page Base changed, Read again the Code Pages.

		if ( dwPageBaseNew != dwPageBase )
		{
			dwPageBase = dwPageBaseNew;

			memset( g_vbCodePagesSnapshot, 0xFF, sizeof( g_vbCodePagesSnapshot ) );

			if ( IsPagePresent( (PVOID) dwPageBase ) )
				memcpy( g_vbCodePagesSnapshot, (PVOID) dwPageBase, MACRO_SYSTEM_PAGE_SIZE );

			if ( IsPagePresent( (PVOID) ( dwPageBase + MACRO_SYSTEM_PAGE_SIZE ) ) )
				memcpy( & g_vbCodePagesSnapshot[ MACRO_SYSTEM_PAGE_SIZE ],
					(PVOID) ( dwPageBase + MACRO_SYSTEM_PAGE_SIZE ),
					MACRO_SYSTEM_PAGE_SIZE );
		}

		// Set the Pointer.

		pbPos = & g_vbCodePagesSnapshot[ ( (DWORD) pbPosFromRefPtr ) - dwPageBase ];
		pbRef = & g_vbCodePagesSnapshot[ ( (DWORD) pvRefPointer ) - dwPageBase ];

		// === Disassemble. ===

		ulInstrsLenPos = 0;

		while( TRUE )
		{
			// Disassemble at the Current Position.

			lLenDisAsm = disasm( pbPos, g_szDrawCodeWindowContentsDisAsmBuffer,
				32 /* BITS */, (DWORD) pbPosFromRefPtr, FALSE, 0 /* INTEL INSTR. SET */, NULL );

			if ( lLenDisAsm == 0 )
			{
				ulInstrsLenPos = 0;
				break;
			}

			// Record the Instruction Length.

			g_vdwInstrsLen[ ulInstrsLenPos ++ ] = lLenDisAsm;

			if ( ulInstrsLenPos == ulInstrLimit )
			{
				bExitFromLoop = TRUE;
				break;
			}

			// Check whether we have to Exit from the Loop.

			pbPos += lLenDisAsm;
			pbPosFromRefPtr += lLenDisAsm;

			if ( pbPos == pbRef )
			{
				break;
			}
			else if ( pbPos > pbRef )
			{
				ulInstrsLenPos = 0;
				break;
			}
		}

		if ( bExitFromLoop )
			break;

		// === Set up the Structures. ===

		if ( ulInstrsLenPos )
		{
			for ( ulJ = 0; ulJ < ulInstrsLenPos; ulJ ++ )
			{
				pspPtr = & g_vspSyncPoints[ ulInstrsLenPos - ulJ - 1 ];

				// Calculate the Displacement Value.

				ulDisp = 0;

				for ( ulK = ulJ; ulK < ulInstrsLenPos; ulK ++ )
					ulDisp += g_vdwInstrsLen[ ulK ];

				// Iterate through the Nodes.

				while( TRUE )
				{
					if ( pspPtr->ulDisp == ulDisp )
					{
						pspPtr->ulHits ++;
						break;
					}
					else if ( pspPtr->ulDisp == 0 )
					{
						pspPtr->ulDisp = ulDisp;
						pspPtr->ulHits = 1;
						break;
					}
					else if ( pspPtr->pspNext == NULL )
					{
						if ( g_ulSyncPointsPos < MACRO_MAXNUM_OF_SYNCPOINTS )
						{
							g_vspSyncPoints[ g_ulSyncPointsPos ].ulDisp = ulDisp;
							g_vspSyncPoints[ g_ulSyncPointsPos ].ulHits = 1;
							g_vspSyncPoints[ g_ulSyncPointsPos ].pspNext = NULL;

							pspPtr->pspNext = & g_vspSyncPoints[ g_ulSyncPointsPos ];

							g_ulSyncPointsPos ++;
						}

						break;
					}
					else
					{
						pspPtr = pspPtr->pspNext;
					}
				}
			}
		}
	}

	// Check the Structures for the Results.

	ulHitsMax = 0;
	pspHitsMaxSyncP = NULL;

	pspPtr = & g_vspSyncPoints[ ulInstrOrdinal ];

	do
	{
		if ( pspPtr->ulHits >= ulHitsMax )
		{
			ulHitsMax = pspPtr->ulHits;
			pspHitsMaxSyncP = pspPtr;
		}

		pspPtr = pspPtr->pspNext;

	} while( pspPtr );

	if ( ulHitsMax == 0 || pspHitsMaxSyncP == NULL )
		return NULL;
	else
		return (VOID*) ( ( (DWORD) pvRefPointer ) - pspHitsMaxSyncP->ulDisp );
}

//=============================================
// AddressNextInstruction Function Definition.
//=============================================

VOID* AddressNextInstruction( IN VOID* pvRefPointer, IN ULONG ulInstrOrdinal )
{
	ULONG			ulI;
	BYTE*			pbPtr;
	LONG			lLenDisAsm;
	DWORD			dwEipPageBase;
	BYTE*			pbPtrFromRefPtr;

	// === Read the Code Pages. ===

	memset( g_vbCodePagesSnapshot, 0xFF, sizeof( g_vbCodePagesSnapshot ) );

	dwEipPageBase = ( (DWORD) pvRefPointer ) & 0xFFFFF000;

	if ( IsPagePresent( (PVOID) dwEipPageBase ) )
		memcpy( g_vbCodePagesSnapshot, (PVOID) dwEipPageBase, MACRO_SYSTEM_PAGE_SIZE );

	if ( IsPagePresent( (PVOID) ( dwEipPageBase + MACRO_SYSTEM_PAGE_SIZE ) ) )
		memcpy( & g_vbCodePagesSnapshot[ MACRO_SYSTEM_PAGE_SIZE ],
			(PVOID) ( dwEipPageBase + MACRO_SYSTEM_PAGE_SIZE ),
			MACRO_SYSTEM_PAGE_SIZE );

	// === Iterate through all the Next Instructions. ===

	pbPtr = & g_vbCodePagesSnapshot[ ( (DWORD) pvRefPointer ) - dwEipPageBase ];
	pbPtrFromRefPtr = (BYTE*) pvRefPointer;

	for ( ulI = 0; ulI <= ulInstrOrdinal; ulI ++ )
	{
		// === Disassemble. ===

		lLenDisAsm = disasm( pbPtr, g_szDrawCodeWindowContentsDisAsmBuffer,
			32 /* BITS */, (DWORD) pbPtrFromRefPtr, FALSE, 0 /* INTEL INSTR. SET */, NULL );

		if ( lLenDisAsm == 0 )
			lLenDisAsm = 1;

		// === Increment. ===

		pbPtr += lLenDisAsm;
		pbPtrFromRefPtr += lLenDisAsm;
	}

	// Return to the Caller.

	return pbPtrFromRefPtr;
}

//==================================================
// PrintCodeWindowPosInModules Function Definition.
//==================================================

VOID PrintCodeWindowPosInModules( IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH, IN OUT ULONG* pulSysStatusBarCurrX )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();

	WORD*					pwPtr;
	ULONG					ulI, ulCount;

	// === Draw the Background. ===

	pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		* pulSysStatusBarCurrX, pviwOutputWindow->ulY0 - 1 );

	ulCount = pviwOutputWindow->ulX1 - (*pulSysStatusBarCurrX) + 1;

	for ( ulI = 0; ulI < ulCount; ulI ++ )
		* pwPtr ++ = CLRTXT2VIDBUFWORD( 2, 0, 196 );

	// === Module + Section + Displacement Information. ===

	DiscoverBytePointerPosInModules( g_szDrawConsoleFnTempBuffer,
		(BYTE*) extension->dwCodeWindowPos,
		extension->pvNtoskrnlDriverSection, g_pvMmUserProbeAddress,
		extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase,
		NULL );

	if ( strlen( g_szDrawConsoleFnTempBuffer ) )
	{
		( * pulSysStatusBarCurrX ) += 2;

		g_szDrawConsoleFnTempBuffer[ pviwOutputWindow->ulX1 - (*pulSysStatusBarCurrX) + 1 ] = '\0';

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			* pulSysStatusBarCurrX, pviwOutputWindow->ulY0 - 1, 0x02, g_szDrawConsoleFnTempBuffer );
	}

	// === CPU Number. ===

	if ( g_dwNumberProcessors > 1 )
	{
		sprintf( g_szDrawConsoleFnTempBuffer, "CPU(#%.2X)", extension->sisSysInterrStatus.dwCurrentProcessor );

		OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwOutputWindow->ulX1 - 7, pviwOutputWindow->ulY0 - 1, 0x0B, g_szDrawConsoleFnTempBuffer );
	}

	// === Return to the Caller. ===

	return;
}

//=======================================
// EcCommandHandler Function Definition.
//=======================================

VOID EcCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	// === Simulate the F6 Key Stroke. ===

	ProcessKeyStroke( MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE, MACRO_SCANCODE_F6 );

	// === Return to the Caller. ===

	return;
}

//============================================================
//
// BugChecker (VpcICE) Default Include File for Script Files.
//
//============================================================

#define MACRO_MINICINCLUDE \
"\
  // MiniC Include File for VpcICE Script Files\x0D\x0A\
\x0D\x0A\
  #define VOID void\x0D\x0A\
  #define DWORD unsigned long\x0D\x0A\
\x0D\x0A\
  void print( int iClrEscapes, int iReserved, char* pszFormat, ... );\x0D\x0A\
\x0D\x0A\
"

static CHAR g_szMiniCInclude[] = MACRO_MINICINCLUDE;

//=========================================
// CompileMiniCSource Function Definition.
//=========================================

static MEMFILE_SESSION			g_mfsMemFileSession;
static MEMFILE					g_vmfSourceInputFiles[ 2 ];

static CompileSourceOptions_t	g_csoCompileSourceOptions;

BOOLEAN CompileMiniCSource( OUT BYTE** ppbOutputMem, OUT ULONG* pulOutputMemSize, IN BYTE* pbInputMem, IN ULONG ulInputMemSize, OUT BYTE** ppbPsiMem, OUT ULONG* pulPsiMemSize )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bGeneratePSI = FALSE;
	MEMFILE_NAME(FILE)*		phOutputFile;
	MEMFILE_NAME(FILE)*		phPsiFile;
	ULONG					ulSize;

	* ppbOutputMem = NULL;
	* pulOutputMemSize = 0;

	if ( ppbPsiMem )
		* ppbPsiMem = NULL;

	if ( pulPsiMemSize )
		* pulPsiMemSize = 0;

	if ( ppbPsiMem && pulPsiMemSize )
		bGeneratePSI = TRUE;

	// ### Compile the Input File. ###

	// Set up the MiniC Compilation Directives.

	memset( & g_csoCompileSourceOptions, 0, sizeof( g_csoCompileSourceOptions ) );

	strcpy( g_csoCompileSourceOptions.source_file, MACRO_MINICSOURCE_FILENAME );
	strcpy( g_csoCompileSourceOptions.compiled_file, MACRO_MINICOBJECT_FILENAME );

	if ( bGeneratePSI )
		strcpy( g_csoCompileSourceOptions.PSI_file, MACRO_MINICPSI_FILENAME );

	strcpy( g_csoCompileSourceOptions.source_directory, "" );
	strcpy( g_csoCompileSourceOptions.headers_directory, "" );

	strcpy( g_csoCompileSourceOptions.definitions, MACRO_MINIC_DEFINITIONS );

	g_csoCompileSourceOptions.compiler_memory_factor = MACRO_MINICCOMP_MEMFACTOR;

	// Set up the Memory Files Structures.

	memset( & g_vmfSourceInputFiles, 0, sizeof( g_vmfSourceInputFiles ) );

	//

	strcpy( g_vmfSourceInputFiles[ 0 ].szName, MACRO_MINICSOURCE_FILENAME );
	g_vmfSourceInputFiles[ 0 ].pbMemory = pbInputMem;
	g_vmfSourceInputFiles[ 0 ].ulMemoryLength = ulInputMemSize;
	g_vmfSourceInputFiles[ 0 ].ulFileSize = ulInputMemSize;

	//

	strcpy( g_vmfSourceInputFiles[ 1 ].szName, MACRO_MINICINCLUDE_FILENAME );
	g_vmfSourceInputFiles[ 1 ].pbMemory = g_szMiniCInclude;
	ulSize = strlen( g_szMiniCInclude );
	g_vmfSourceInputFiles[ 1 ].ulMemoryLength = ulSize;
	g_vmfSourceInputFiles[ 1 ].ulFileSize = ulSize;

	//

	InitializeMemFileSession( & g_mfsMemFileSession,
		extension->pvMemFileMemory, extension->ulMemFileMemoryLength, extension->ulMemFileMemoryStep,
		g_vmfSourceInputFiles, sizeof( g_vmfSourceInputFiles ) / sizeof( MEMFILE ) );

	// Initialize the Compiler.

	if ( InitializeMiniCCompiler() == FALSE )
		return FALSE;

	// Compile the Source File.

	CompileSource( & g_csoCompileSourceOptions );

	// Return the Informations to the Caller.

	phOutputFile = MEMFILE_NAME(fopen)( MACRO_MINICOBJECT_FILENAME, "rb" );

	if ( phOutputFile )
	{
		* ppbOutputMem = phOutputFile->pmfBody->pbMemory;
		* pulOutputMemSize = phOutputFile->pmfBody->ulFileSize;
	}

	if ( bGeneratePSI )
	{
		phPsiFile = MEMFILE_NAME(fopen)( MACRO_MINICPSI_FILENAME, "rb" );

		if ( phPsiFile )
		{
			* ppbPsiMem = phPsiFile->pmfBody->pbMemory;
			* pulPsiMemSize = phPsiFile->pmfBody->ulFileSize;
		}
	}

	// Return to the Caller.

	return TRUE;
}

//==============================================
// InitializeMiniCCompiler Function Definition.
//==============================================

BOOLEAN InitializeMiniCCompiler( VOID )
{
	// Initialize the Compiler and Return.

	compiler_memory_factor = MACRO_MINICCOMP_MEMFACTOR;

	if ( InitCompiler() == 0 )
		return TRUE;
	else
		return FALSE;
}

//======================================
// LinkMiniCSource Function Definition.
//======================================

// // //

typedef struct _NAMESPACE_ENTRY
{
	CHAR*		pszName;
	BYTE*		pbAddress;

} NAMESPACE_ENTRY, *PNAMESPACE_ENTRY;

static NAMESPACE_ENTRY		g_vneVpcICENameSpace[ 1024 ] = {
	{ "______eval_dword______", (BYTE*) & ______eval_dword______ },
	{ "______eval_dword_2______", (BYTE*) & ______eval_dword_2______ },
	{ "______eval_double______", (BYTE*) & ______eval_double______ },
	{ "print", (BYTE*) & OutputPrint },
	{ NULL, NULL }
};

// // //

static BYTE					g_vbAdditionalLinkerMem[ 64 * 1024 ];

// // //

BOOLEAN LinkMiniCSource( OUT BYTE* pbOutputMem, IN ULONG ulOutputMemSize, OUT ULONG* pulOutputMemUsedDim, OUT BYTE** ppbInitFuncPtr, IN CHAR* pszFunctionName, OUT BYTE** ppbFunctionPtr, IN BYTE* pbInputMem, IN ULONG ulInputMemSize, IN BOOLEAN bPrintErrorLines, IN BYTE* pbObjectFileToLink )
{
	BYTE*				pbCompileOutputMem = NULL;
	ULONG				ulCompileOutputMemSize = 0;
	BOOLEAN				bCompileRes;
	BYTE*				pbBase;
	ULONG				ulI, ulJ, ulK;
	header_t*			phdrHeader;
	relocation_t*		prRelocTable;
	BYTE*				pbNames;
	function_t*			pfFunctions;
	extfunction_t*		pefExternals;
	BYTE*				pbAdditionalMem;
	BYTE*				pbStrings;
	ULONG				ulRelocTableDim;
	ULONG				ulFunctionsDim;
	ULONG				ulExternalsDim;
	BYTE*				pbCode;
	ULONG*				pulRelocAddress;
	function_t*			pfThis;
	CHAR*				pszIdentifier;
	extfunction_t*		pefThis;
	NAMESPACE_ENTRY*	pnePtr;
	BOOLEAN				bUnresExterns;
	ULONG*				pulRef;
	CHAR*				pszMessage;
	CHAR*				pszFuncName;
	BOOLEAN				bContinue;

	if ( pulOutputMemUsedDim )
		* pulOutputMemUsedDim = 0;

	if ( ppbInitFuncPtr )
		* ppbInitFuncPtr = NULL;

	if ( ppbFunctionPtr )
		* ppbFunctionPtr = NULL;

	// Try to Compile the Source.

	bCompileRes = CompileMiniCSource( & pbCompileOutputMem, & ulCompileOutputMemSize, pbInputMem, ulInputMemSize, NULL, NULL );

	if ( bCompileRes == FALSE )
	{
		OutputPrint( FALSE, FALSE, "Unrecoverable fatal error while compiling." );
		return FALSE;
	}

	// Print the Compiler Messages.

	for ( ulI = 0; ulI < ( ULONG ) current_console_message; ulI ++ )
	{
		pszMessage = console_messages[ ulI ];

		if ( strlen( pszMessage ) > 3 &&
			memcmp( pszMessage, "***", 3 ) == 0 )
		{
			continue;
		}
		else if ( bPrintErrorLines == FALSE &&
			MACRO_CRTFN_NAME(strchr)( pszMessage, '(' ) == pszMessage )
		{
			pszMessage = MACRO_CRTFN_NAME(strchr)( pszMessage, ')' );
			if ( pszMessage == NULL )
				continue;
			pszMessage ++;
		}

		OutputPrint( TRUE, FALSE, "!0C%s", GetEscapedSafeString( pszMessage ) );
	}

	// Check whether we can continue.

	if ( pbCompileOutputMem == NULL ||
		ulCompileOutputMemSize == 0 ||
		fatal_error || compiler_errors )
	{
		return FALSE;
	}

	// Copy the Object File Memory, if required.

	if ( pbOutputMem && ulOutputMemSize )
	{
		if ( ulCompileOutputMemSize + ((header_t*)pbCompileOutputMem)->mem_required > ulOutputMemSize )
		{
			OutputPrint( FALSE, FALSE, "Insufficient memory while linking." );
			return FALSE;
		}
		else
		{
			memcpy( pbOutputMem, pbCompileOutputMem, ulCompileOutputMemSize );
			pbBase = pbOutputMem;

			pbAdditionalMem = pbOutputMem + ulCompileOutputMemSize;
		}
	}
	else
	{
		pbBase = pbCompileOutputMem;

		if ( ((header_t*)pbCompileOutputMem)->mem_required > sizeof( g_vbAdditionalLinkerMem ) )
		{
			OutputPrint( FALSE, FALSE, "Insufficient memory for data/strings." );
			return FALSE;
		}
		else
		{
			pbAdditionalMem = g_vbAdditionalLinkerMem;
		}
	}

	if ( pulOutputMemUsedDim )
		* pulOutputMemUsedDim = ulCompileOutputMemSize + ((header_t*)pbCompileOutputMem)->mem_required;

	// Check the Magic Number.

	phdrHeader = (header_t*) pbBase;

	if ( phdrHeader->magic_number != o4_magic_number )
	{
		OutputPrint( FALSE, FALSE, "Erroneous object file format." );
		return FALSE;
	}

	// Set the Pointers and the Dimensions.

	prRelocTable = (relocation_t*) ( pbBase + phdrHeader->relocation.offset );
	pbNames = pbBase + phdrHeader->names.offset;
	pfFunctions = (function_t*) ( pbBase + phdrHeader->functions.offset );
	pefExternals = (extfunction_t*) ( pbBase + phdrHeader->extfunctions.offset );

	pbStrings = pbBase + phdrHeader->strings.offset;

	ulRelocTableDim = phdrHeader->relocation.size / sizeof( relocation_t );
	ulFunctionsDim = phdrHeader->functions.size / sizeof( function_t );
	ulExternalsDim = phdrHeader->extfunctions.size / sizeof( extfunction_t );

	pbCode = pbBase + phdrHeader->code.offset;

	// Reset the Data Memory and Copy the Strings.

	if ( phdrHeader->strings.size > phdrHeader->mem_required )
	{
		OutputPrint( FALSE, FALSE, "Erroneous object file format." );
		return FALSE;
	}

	memcpy( pbAdditionalMem, pbStrings, phdrHeader->strings.size );
	memset( pbAdditionalMem + phdrHeader->strings.size, 0, phdrHeader->mem_required - phdrHeader->strings.size );

	// Reallocate the Code in the Object File.

	for ( ulI = 0; ulI < ulRelocTableDim; ulI ++ )
	{
		pulRelocAddress = (ULONG*) ( pbCode + prRelocTable[ ulI ].address );
		* pulRelocAddress += (ULONG) pbAdditionalMem;
	}

	// Take care of the Exports.

	for ( ulI = 0; ulI < ulFunctionsDim; ulI ++ )
	{
		pfThis = & pfFunctions[ ulI ];
		pszIdentifier = (CHAR*) ( pbNames + pfThis->name_address );

		// Check the Various Cases.

		if ( ppbInitFuncPtr && strcmp( pszIdentifier, o4_initialization_function_name ) == 0 )
			* ppbInitFuncPtr = pbCode + pfThis->address;

		if ( ppbFunctionPtr && pszFunctionName && strcmp( pszIdentifier, pszFunctionName ) == 0 )
			* ppbFunctionPtr = pbCode + pfThis->address;
	}

	// Take care of the Imports.

	bUnresExterns = FALSE;

	for ( ulI = 0; ulI < ulExternalsDim; ulI ++ )
	{
		pefThis = & pefExternals[ ulI ];
		pszIdentifier = (CHAR*) ( pbNames + pefThis->name_address );

		// Check the Object File NameSpace itself.

		for ( ulJ = 0; ulJ < ulFunctionsDim; ulJ ++ )
		{
			pfThis = & pfFunctions[ ulJ ];
			pszFuncName = (CHAR*) ( pbNames + pfThis->name_address );

			if ( strcmp( pszFuncName, pszIdentifier ) == 0 )
				break;
		}

		if ( ulJ != ulFunctionsDim )
		{
			pulRef = (ULONG*) ( pbCode + pefThis->address );
			* pulRef = ((DWORD)( pbCode + pfThis->address )) - ((DWORD)pulRef) - 4;
		}
		else
		{
			// Check the Second Object File, if specified.

			bContinue = TRUE;

			if ( pbObjectFileToLink )
			{
				header_t*			phdrHeader2 = (header_t*) pbObjectFileToLink;
				ULONG				ulFunctionsDim2 = phdrHeader2->functions.size / sizeof( function_t );
				function_t*			pfFunctions2 = (function_t*) ( pbObjectFileToLink + phdrHeader2->functions.offset );
				BYTE*				pbNames2 = pbObjectFileToLink + phdrHeader2->names.offset;
				BYTE*				pbCode2 = pbObjectFileToLink + phdrHeader2->code.offset;

				for ( ulK = 0; ulK < ulFunctionsDim2; ulK ++ )
				{
					pfThis = & pfFunctions2[ ulK ];
					pszFuncName = (CHAR*) ( pbNames2 + pfThis->name_address );

					if ( strcmp( pszFuncName, pszIdentifier ) == 0 )
					{
						pulRef = (ULONG*) ( pbCode + pefThis->address );
						* pulRef = ((DWORD)( pbCode2 + pfThis->address )) - ((DWORD)pulRef) - 4;

						bContinue = FALSE;

						break;
					}
				}
			}

			// If necessary, we will Search in the VpcICE Namespace.

			if ( bContinue )
			{
				// Check the VpcICE NameSpace.

				pnePtr = g_vneVpcICENameSpace;

				while( pnePtr->pszName )
				{
					if ( strcmp( pnePtr->pszName, pszIdentifier ) == 0 )
						break;
					pnePtr ++;
				}

				if ( pnePtr->pszName == NULL || pnePtr->pbAddress == NULL )
				{
					OutputPrint( FALSE, FALSE, "Unresolved external: \"%s\".", pszIdentifier );
					bUnresExterns = TRUE;
				}
				else
				{
					pulRef = (ULONG*) ( pbCode + pefThis->address );
					* pulRef = ((DWORD)pnePtr->pbAddress) - ((DWORD)pulRef) - 4;
				}
			}
		}
	}

	if ( bUnresExterns )
		return FALSE;

	// Return to the Caller.

	return TRUE;
}

//===============================================
// InvokeDebuggerSettingEIP Function Definition.
//===============================================

VOID __declspec( naked ) InvokeDebuggerSettingEIP( VOID /* Input: EAX = address of EIP Dword. */ )
{
	__asm
	{
		mov			ebx, dword ptr[ esp ]
		mov			dword ptr[ eax ], ebx

		call		InvokeDebugger

		ret
	}
}

//===========================================
// I3HereCommandHandler Function Definition.
//===========================================

VOID I3HereCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	// Check the Parameters of the Command.

	if ( strlen( pszParams ) == 0 )
	{
		// Print a Information String.

		switch( extension->ulI3HereState )
		{
		case MACRO_I3HERE_ON:
			OutputPrint( FALSE, FALSE, "I3Here is on." );
			break;

		case MACRO_I3HERE_OFF:
			OutputPrint( FALSE, FALSE, "I3Here is off." );
			break;

		case MACRO_I3HERE_DRV:
			OutputPrint( FALSE, FALSE, "I3Here is on for device drivers. (address > %s)",
				((DWORD)g_pvMmUserProbeAddress) <= 0x80000000 ? "2GB" : "3GB" );
			break;
		}
	}
	else if ( MACRO_CRTFN_NAME(stricmp)( pszParams, "on" ) == 0 )
	{
		extension->ulI3HereState = MACRO_I3HERE_ON;
	}
	else if ( MACRO_CRTFN_NAME(stricmp)( pszParams, "off" ) == 0 )
	{
		extension->ulI3HereState = MACRO_I3HERE_OFF;
	}
	else if ( MACRO_CRTFN_NAME(stricmp)( pszParams, "drv" ) == 0 )
	{
		extension->ulI3HereState = MACRO_I3HERE_DRV;
	}
	else
	{
		OutputPrint( FALSE, FALSE, "Invalid parameter." );
	}

	// Return to the Caller.

	return;
}

//==========================================================
// GetHighlightedInstructionMemoryHint Function Definition.
//==========================================================

static WORD GetRegisterValue16( ULONG ulRegOrd )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	x86_CPU_CONTEXT*		px86ccState = & extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ];

	// Return the Information.

	switch( ulRegOrd )
	{
	case 2: // ax
		return (WORD) ( px86ccState->x86vicContext.EAX & 0xFFFF );
	case 5: // bp
		return (WORD) ( px86ccState->x86vicContext.EBP & 0xFFFF );
	case 6: // bx
		return (WORD) ( px86ccState->x86vicContext.EBX & 0xFFFF );
	case 17: // cs
		return px86ccState->x86vicContext.CS;
	case 18: // cx
		return (WORD) ( px86ccState->x86vicContext.ECX & 0xFFFF );
	case 20: // di
		return (WORD) ( px86ccState->x86vicContext.EDI & 0xFFFF );
	case 30: // ds
		return px86ccState->x86vicContext.DS;
	case 31: // dx
		return (WORD) ( px86ccState->x86vicContext.EDX & 0xFFFF );
	case 38: // es
		return px86ccState->x86vicContext.ES;
	case 41: // fs
		return px86ccState->x86vicContext.FS;
	case 42: // gs
		return px86ccState->x86vicContext.GS;
	case 53: // si
		return (WORD) ( px86ccState->x86vicContext.ESI & 0xFFFF );
	case 54: // sp
		return (WORD) ( px86ccState->x86vicContext.ESP & 0xFFFF );
	case 55: // ss
		return px86ccState->x86vicContext.SS;
	}

	return 0;
}

static DWORD GetRegisterValue32( ULONG ulRegOrd )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	x86_CPU_CONTEXT*		px86ccState = & extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ];

	// Return the Information.

	switch( ulRegOrd )
	{
	case 9: // cr0
		return px86ccState->dwCR0;
	case 10: // cr1
		return 0;
	case 11: // cr2
		return px86ccState->dwCR2;
	case 12: // cr3
		return px86ccState->dwCR3;
	case 13: // cr4
		return px86ccState->dwCR4;
	case 14: // cr5
		return 0;
	case 15: // cr6
		return 0;
	case 16: // cr7
		return 0;
	case 22: // dr0
		return px86ccState->dwDR0;
	case 23: // dr1
		return px86ccState->dwDR1;
	case 24: // dr2
		return px86ccState->dwDR2;
	case 25: // dr3
		return px86ccState->dwDR3;
	case 26: // dr4
		return 0;
	case 27: // dr5
		return 0;
	case 28: // dr6
		return px86ccState->dwDR6;
	case 29: // dr7
		return px86ccState->dwDR7;
	case 32: // eax
		return px86ccState->x86vicContext.EAX;
	case 33: // ebp
		return px86ccState->x86vicContext.EBP;
	case 34: // ebx
		return px86ccState->x86vicContext.EBX;
	case 35: // ecx
		return px86ccState->x86vicContext.ECX;
	case 36: // edi
		return px86ccState->x86vicContext.EDI;
	case 37: // edx
		return px86ccState->x86vicContext.EDX;
	case 39: // esi
		return px86ccState->x86vicContext.ESI;
	case 40: // esp
		return px86ccState->x86vicContext.ESP;
	case 64: // tr0
		return 0;
	case 65: // tr1
		return 0;
	case 66: // tr2
		return 0;
	case 67: // tr3
		return 0;
	case 68: // tr4
		return 0;
	case 69: // tr5
		return 0;
	case 70: // tr6
		return 0;
	case 71: // tr7
		return 0;
	}

	return 0;
}

static ULONG GetRegisterSize( ULONG ulRegOrd )
{
	ULONG			ulRegSize = 0;

	// Return the Information.

	switch( ulRegOrd )
	{
	case 0: // ah
	case 1: // al
	case 3: // bh
	case 4: // bl
	case 7: // ch
	case 8: // cl
	case 19: // dh
	case 21: // dl
		ulRegSize = 1;
		break;

	case 2: // ax
	case 5: // bp
	case 6: // bx
	case 17: // cs
	case 18: // cx
	case 20: // di
	case 30: // ds
	case 31: // dx
	case 38: // es
	case 41: // fs
	case 42: // gs
	case 53: // si
	case 54: // sp
	case 55: // ss
		ulRegSize = 2;
		break;

	case 9: // cr0
	case 10: // cr1
	case 11: // cr2
	case 12: // cr3
	case 13: // cr4
	case 14: // cr5
	case 15: // cr6
	case 16: // cr7
	case 22: // dr0
	case 23: // dr1
	case 24: // dr2
	case 25: // dr3
	case 26: // dr4
	case 27: // dr5
	case 28: // dr6
	case 29: // dr7
	case 32: // eax
	case 33: // ebp
	case 34: // ebx
	case 35: // ecx
	case 36: // edi
	case 37: // edx
	case 39: // esi
	case 40: // esp
	case 64: // tr0
	case 65: // tr1
	case 66: // tr2
	case 67: // tr3
	case 68: // tr4
	case 69: // tr5
	case 70: // tr6
	case 71: // tr7
		ulRegSize = 4;
		break;
	}

	return ulRegSize;
}

VOID GetHighlightedInstructionMemoryHint( OUT CHAR* pszHint )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	x86_CPU_CONTEXT*		px86ccState = & extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ];
	DISASM_INFO*			pdaiInfo = & extension->daiHighlightedInstr;

	DWORD					dwAddress = 0;
	ULONG					ulSize = 0;
	CHAR*					pszSegment = "ds";

	ULONG					ulRegSize = 0;

	ULONG					ulI;

	ULONG					ulAddressSize = 0;
	WORD					wAddressW;
	DWORD					dwAddressDW;

	ULONG					ulRegOrd;

	DWORD					dwEffectiveAddress = 0;
	WORD					wSegment;
	DWORD*					pdwGdtEntry0;
	DWORD*					pdwGdtEntry4;

	BYTE					bValueB;
	WORD					wValueW;
	DWORD					dwValueDW;

	* pszHint = '\0';

	// === Try to get the Address and Size Informations from the Instruction. ===

	if ( pdaiInfo->pitTemplate == NULL )
		return;

	for ( ulI = 0; ulI < (ULONG) pdaiInfo->pitTemplate->operands; ulI ++ )
	{
		if ( ( pdaiInfo->pitTemplate->opd[ ulI ] & ( REGISTER | FPUREG ) ) ||
			( pdaiInfo->iInstruction.oprs[ ulI ].segment & SEG_RMREG ) )
		{
			ulRegSize = GetRegisterSize( pdaiInfo->iInstruction.oprs[ ulI ].basereg - EXPR_REG_START );
		}
		else if ( !( UNITY & ~ pdaiInfo->pitTemplate->opd[ ulI ] ) )
		{
			continue;
		}
		else if ( pdaiInfo->pitTemplate->opd[ ulI ] & IMMEDIATE )
		{
			if ( pdaiInfo->pitTemplate->opd[ ulI ] & BITS8 )
				ulRegSize = 1;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & BITS16 )
				ulRegSize = 2;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & BITS32 )
				ulRegSize = 4;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & NEAR )
				ulRegSize = 4;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & SHORT )
				ulRegSize = 4;
		}
	}

	for ( ulI = 0; ulI < (ULONG) pdaiInfo->pitTemplate->operands; ulI ++ )
	{
		if ( ( pdaiInfo->pitTemplate->opd[ ulI ] & ( REGISTER | FPUREG ) ) ||
			( pdaiInfo->iInstruction.oprs[ ulI ].segment & SEG_RMREG ) )
		{
			continue;
		}
		else if ( !( UNITY & ~ pdaiInfo->pitTemplate->opd[ ulI ] ) )
		{
			continue;
		}
		else if ( pdaiInfo->pitTemplate->opd[ ulI ] & IMMEDIATE )
		{
			continue;
		}
		else if ( !( MEM_OFFS & ~pdaiInfo->pitTemplate->opd[ ulI ] ) )
		{
			// Return the Informations.

			switch( pdaiInfo->iInstruction.oprs[ ulI ].addr_size )
			{
			case 32:
				ulSize = 4;
				break;
			case 16:
				ulSize = 2;
				break;
			default:
				ulSize = ulRegSize;
				break;
			}

			dwAddress = pdaiInfo->iInstruction.oprs[ ulI ].offset;
			break;
		}
		else if ( !( REGMEM & ~ pdaiInfo->pitTemplate->opd[ ulI ] ) )
		{
			// Return the Informations.

			if ( pdaiInfo->pitTemplate->opd[ ulI ] & BITS8 )
				ulSize = 1;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & BITS16 )
				ulSize = 2;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & BITS32 )
				ulSize = 4;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & __FAR )
				ulSize = 4;
			else if ( pdaiInfo->pitTemplate->opd[ ulI ] & NEAR )
				ulSize = 4;
			else if ( !( pdaiInfo->pitTemplate->opd[ ulI ] & BITS64 ) &&
				!( pdaiInfo->pitTemplate->opd[ ulI ] & BITS80 ) )
			{
				if ( pdaiInfo->iInstruction.oprs[ ulI ].addr_size == 16 )
					ulSize = 2;
				else if ( pdaiInfo->iInstruction.oprs[ ulI ].addr_size == 32 )
					ulSize = 4;
				else
					ulSize = ulRegSize;
			}

			if ( pdaiInfo->iInstruction.oprs[ ulI ].basereg != -1 )
			{
				ulRegOrd = pdaiInfo->iInstruction.oprs[ ulI ].basereg - EXPR_REG_START;
				ulAddressSize = GetRegisterSize( ulRegOrd );

				if ( ulAddressSize == 2 )
					wAddressW = GetRegisterValue16( ulRegOrd );
				else if ( ulAddressSize == 4 )
					dwAddressDW = GetRegisterValue32( ulRegOrd );
				else
					ulSize = 0;

				if ( ulRegOrd == 5 /* BP */ || ulRegOrd == 33 /* EBP */ ||
					ulRegOrd == 54 /* SP */ || ulRegOrd == 40 /* ESP */ )
						pszSegment = "ss";
			}

			if ( pdaiInfo->iInstruction.oprs[ ulI ].indexreg != -1 )
			{
				ulRegOrd = pdaiInfo->iInstruction.oprs[ ulI ].indexreg - EXPR_REG_START;
				if ( ulAddressSize == 0 )
				{
					ulAddressSize = GetRegisterSize( ulRegOrd );
					wAddressW = 0;
					dwAddressDW = 0;
				}

				if ( ulAddressSize == 2 )
					wAddressW += GetRegisterValue16( ulRegOrd );
				else if ( ulAddressSize == 4 )
					dwAddressDW += GetRegisterValue32( ulRegOrd );
				else
					ulSize = 0;

				if ( pdaiInfo->iInstruction.oprs[ ulI ].scale > 1 )
				{
					if ( ulAddressSize == 2 )
						wAddressW *= (WORD) pdaiInfo->iInstruction.oprs[ ulI ].scale;
					else if ( ulAddressSize == 4 )
						dwAddressDW *= pdaiInfo->iInstruction.oprs[ ulI ].scale;
				}

				if ( ulRegOrd == 5 /* BP */ || ulRegOrd == 33 /* EBP */ ||
					ulRegOrd == 54 /* SP */ || ulRegOrd == 40 /* ESP */ )
						pszSegment = "ss";
			}

			if ( (pdaiInfo->iInstruction.oprs[ ulI ].segment & SEG_DISP8) ||
				(pdaiInfo->iInstruction.oprs[ ulI ].segment & SEG_DISP16) ||
				(pdaiInfo->iInstruction.oprs[ ulI ].segment & SEG_DISP32) )
			{
				if ( ulAddressSize == 0 )
				{
					ulAddressSize = 4;
					dwAddressDW = 0;
				}

				if ( ulAddressSize == 2 )
					wAddressW += (WORD) pdaiInfo->iInstruction.oprs[ ulI ].offset;
				else if ( ulAddressSize == 4 )
					dwAddressDW += (DWORD) pdaiInfo->iInstruction.oprs[ ulI ].offset;
				else
					ulSize = 0;
			}

			if ( ulAddressSize == 2 )
				dwAddress = (DWORD) wAddressW;
			else if ( ulAddressSize == 4 )
				dwAddress = (DWORD) dwAddressDW;

			break;
		}
	}

	if ( pdaiInfo->pszSegOver )
		pszSegment = pdaiInfo->pszSegOver;

	if ( ulSize != 1 && ulSize != 2 && ulSize != 4 )
		return;

	// === Get the Effective Address. ===

	if ( MACRO_CRTFN_NAME(stricmp)( pszSegment, "cs" ) == 0 )
		wSegment = px86ccState->x86vicContext.CS;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszSegment, "ss" ) == 0 )
		wSegment = px86ccState->x86vicContext.SS;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszSegment, "ds" ) == 0 )
		wSegment = px86ccState->x86vicContext.DS;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszSegment, "es" ) == 0 )
		wSegment = px86ccState->x86vicContext.ES;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszSegment, "fs" ) == 0 )
		wSegment = px86ccState->x86vicContext.FS;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszSegment, "gs" ) == 0 )
		wSegment = px86ccState->x86vicContext.GS;
	else
		wSegment = 0;

	if ( wSegment > 0 && wSegment <= MACRO_MAXVALUE_FOR_WINNT_SEGSELECTOR )
	{
		pdwGdtEntry0 = (DWORD*) ( ((BYTE*)px86ccState->pvGDTBase) + ( ((DWORD)wSegment) & 0xFFFFFFF8 ) );
		pdwGdtEntry4 = pdwGdtEntry0 + 1;

		dwEffectiveAddress =
			( (*pdwGdtEntry4) & 0xFF000000 ) | ( ((*pdwGdtEntry4)&0x000000FF) << 16 ) | ( (*pdwGdtEntry0) >> 16 );
	}

	dwEffectiveAddress += dwAddress;

	// === Read the Value from Memory. ===

	switch( ulSize )
	{
	case 1:
		if ( IsPagePresent( (PVOID) dwEffectiveAddress ) == FALSE )
			bValueB = 0xFF;
		else
			bValueB = * (BYTE*) dwEffectiveAddress;

		break;

	case 2:
		if ( IsPagePresent_WORD( (WORD*) dwEffectiveAddress ) == FALSE )
			wValueW = 0xFFFF;
		else
			wValueW = * (WORD*) dwEffectiveAddress;

		break;

	case 4:
		if ( IsPagePresent_DWORD( (DWORD*) dwEffectiveAddress ) == FALSE )
			dwValueDW = 0xFFFFFFFF;
		else
			dwValueDW = * (DWORD*) dwEffectiveAddress;

		break;
	}

	// === Compose the Hint String. ===

	if ( ulAddressSize == 2 )
	{
		switch( ulSize )
		{
		case 1:
			sprintf( pszHint, "%s:%.4X=%.2X", pszSegment, dwAddress, bValueB );
			break;
		case 2:
			sprintf( pszHint, "%s:%.4X=%.4X", pszSegment, dwAddress, wValueW );
			break;
		case 4:
			sprintf( pszHint, "%s:%.4X=%.8X", pszSegment, dwAddress, dwValueDW );
			break;
		}
	}
	else
	{
		switch( ulSize )
		{
		case 1:
			sprintf( pszHint, "%s:%.8X=%.2X", pszSegment, dwAddress, bValueB );
			break;
		case 2:
			sprintf( pszHint, "%s:%.8X=%.4X", pszSegment, dwAddress, wValueW );
			break;
		case 4:
			sprintf( pszHint, "%s:%.8X=%.8X", pszSegment, dwAddress, dwValueDW );
			break;
		}
	}

	// === Return to the Caller. ===

	_strupr( pszHint );

	return;
}

//========================================================
// GetHighlightedInstructionJumpHint Function Definition.
//========================================================

VOID GetHighlightedInstructionJumpHint( OUT CHAR* pszHint, IN CHAR* pszDisasmStr )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	x86_CPU_CONTEXT*		px86ccState = & extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ];

	CHAR*					pszSpaceChr;

	ULONG					ulJumpState = 0;
	DWORD					dwJumpAddress;

	CHAR*					pszAddress;

	* pszHint = '\0';

	// === Initialize. ===

	extension->dwTargetAddress = 0;

	// === Check whether the Current Instruction is a Jump Instruction. ===

	pszSpaceChr = MACRO_CRTFN_NAME(strchr)( pszDisasmStr, ' ' );
	if ( pszSpaceChr == NULL )
		return;

	* pszSpaceChr = '\0';
	pszSpaceChr ++;

#define MACRO_TESTFLAG(flag)		( px86ccState->x86vicContext.EFLAGS & MACRO_##flag##_MASK )

	if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JA" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) == 0 && MACRO_TESTFLAG( ZF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JAE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JB" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JBE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) || MACRO_TESTFLAG( ZF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JC" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JCXZ" ) == 0 )
		ulJumpState = ( px86ccState->x86vicContext.ECX & 0xFFFF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JECXZ" ) == 0 )
		ulJumpState = px86ccState->x86vicContext.ECX == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JG" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) == 0 && (MACRO_TESTFLAG(SF)==0) == (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JGE" ) == 0 )
		ulJumpState = (MACRO_TESTFLAG(SF)==0) == (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JL" ) == 0 )
		ulJumpState = (MACRO_TESTFLAG(SF)==0) != (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JLE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) || (MACRO_TESTFLAG(SF)==0) != (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNA" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) || MACRO_TESTFLAG( ZF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNAE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNB" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNBE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) == 0 && MACRO_TESTFLAG( ZF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNC" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( CF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNG" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) || (MACRO_TESTFLAG(SF)==0) != (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNGE" ) == 0 )
		ulJumpState = (MACRO_TESTFLAG(SF)==0) != (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNL" ) == 0 )
		ulJumpState = (MACRO_TESTFLAG(SF)==0) == (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNLE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) == 0 && (MACRO_TESTFLAG(SF)==0) == (MACRO_TESTFLAG(OF)==0) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNO" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( OF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNP" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( PF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNS" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( SF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JNZ" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JO" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( OF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JP" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( PF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JPE" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( PF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JPO" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( PF ) == 0 ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JS" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( SF ) ? 2 : 1;
	else if ( MACRO_CRTFN_NAME(stricmp)( pszDisasmStr, "JZ" ) == 0 )
		ulJumpState = MACRO_TESTFLAG( ZF ) ? 2 : 1;

#undef MACRO_TESTFLAG

	if ( ulJumpState == 0 )
		return;

	// === Compose the Final Hint String. ===

	// Get the Pointer to the Address.

	if ( strlen( pszSpaceChr ) == 0 )
		return;

	pszAddress = & pszSpaceChr[ strlen( pszSpaceChr ) - 1 ];

	while( pszAddress > pszSpaceChr && * pszAddress != ' ' )
		pszAddress --;

	// Compose the String.

	dwJumpAddress = MACRO_CRTFN_NAME(strtoul)( pszAddress, NULL, 16 );

	if ( ulJumpState == 1 )
	{
		strcpy( pszHint, "(NO JUMP)" );
	}
	else if ( dwJumpAddress < px86ccState->x86vicContext.EIP )
	{
		strcpy( pszHint, "(JUMP \x18)" );
		extension->dwTargetAddress = dwJumpAddress;
	}
	else
	{
		strcpy( pszHint, "(JUMP \x19)" );
		extension->dwTargetAddress = dwJumpAddress;
	}

	// === Return to the Caller. ===

	return;
}

//========================================
// BpxCommandHandler Function Definition.
//========================================

#define MACRO_BPXCMDOPT_INDEX_DR			0
#define MACRO_BPXCMDOPT_INDEX_P				1
#define MACRO_BPXCMDOPT_INDEX_M				2
#define MACRO_BPXCMDOPT_INDEX_ADDRESS		3

static COMMAND_OPTION			g_vcoBpxCommandOptions[] =
{
	MACRO_CMDOPT( "/DR" ), MACRO_CMDOPT( "/P" ), MACRO_CMDOPT( "/M" ), MACRO_CMDOPT_END
};

VOID BpxCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	ULONG					ulDebugReg = 0xFFFFFFFF;
	ULONG					ulI;
	BOOLEAN					vbDebugRegsTaken[ 4 ] = { FALSE, FALSE, FALSE, FALSE };
	DWORD					dwAddress;
	VPCICE_WINDOW*			pviwCodeWindow;
	BOOLEAN					bDRSpecified, bADDRSpecified;
	BOOLEAN					bEvalRes;
	BOOLEAN					bCodeWindowMode = FALSE;
	BOOLEAN					bTakeAvailDRReg = FALSE;

	pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

	// Parse the Command Options.

	if ( ParseCommandOptions( pszParams, g_vcoBpxCommandOptions ) == FALSE )
		return;

	// Fill the Debug Regs Taken Vector.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		if ( psisInts->vbpBreakpoints[ ulI ].ulType != MACRO_BREAKPOINTTYPE_UNUSED &&
			psisInts->vbpBreakpoints[ ulI ].bIsUsingDebugRegisters )
		{
			vbDebugRegsTaken[ psisInts->vbpBreakpoints[ ulI ].ulDebugRegisterNum ] = TRUE;
		}

	// Detect a Special Condition where a Debug Register is Necessary.

	if ( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_P ].pszParam &&
		strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_P ].pszParam ) == 0 &&
		g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_M ].pszParam &&
		strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_M ].pszParam ) == 0 )
	{
		bTakeAvailDRReg = TRUE;
	}

	// Debug Register Param.

	if ( bTakeAvailDRReg ||
			( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_DR ].pszParam &&
			strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_DR ].pszParam ) == 0 )
		)
	{
		// Get the Register Num.

		for ( ulI = 3; ulI < 0x80000000; ulI -- )
			if ( vbDebugRegsTaken[ ulI ] == FALSE )
			{
				ulDebugReg = ulI;
				break;
			}

		if ( ulDebugReg == 0xFFFFFFFF )
		{
			OutputPrint( FALSE, FALSE, "No debug register available." );
			return;
		}
	}

	// Get/Calculate the various Parameters.

	if ( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_ADDRESS ].pszParam == NULL ||
		strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_ADDRESS ].pszParam ) == 0 )
	{
		// We must be in the Code Win Mode.

		if ( extension->bCursorInCodeWinMode == FALSE ||
			pviwCodeWindow == NULL ||
			pviwCodeWindow->bDisplayed == FALSE ||
			extension->dwCodeWinLastCursorPosY < pviwCodeWindow->ulY0 )
		{
			OutputPrint( FALSE, FALSE, "Address must be specified." );
			return;
		}
		else
		{
			dwAddress = g_vdwDrawCodeWindowContentsAddresses[ extension->dwCodeWinLastCursorPosY - pviwCodeWindow->ulY0 ];
			bCodeWindowMode = TRUE;
		}
	}

	// Evaluate the Expressions.

	if ( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_DR ].pszParam &&
		strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_DR ].pszParam ) )
			bDRSpecified = TRUE;
	else
			bDRSpecified = FALSE;

	if ( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_ADDRESS ].pszParam &&
		strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_ADDRESS ].pszParam ) )
			bADDRSpecified = TRUE;
	else
			bADDRSpecified = FALSE;

	if ( bDRSpecified && bADDRSpecified )
	{
		if ( EvaluateMultipleExpressions_DWORD_2(
			g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_DR ].pszParam,
			g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_ADDRESS ].pszParam,
			& ulDebugReg, & dwAddress ) == FALSE )
				return;
	}
	else if ( bDRSpecified )
	{
		ulDebugReg = EvaluateExpression_DWORD(
			g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_DR ].pszParam, & bEvalRes );

		if ( bEvalRes == FALSE )
			return;
	}
	else if ( bADDRSpecified )
	{
		dwAddress = EvaluateExpression_DWORD(
			g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_ADDRESS ].pszParam, & bEvalRes );

		if ( bEvalRes == FALSE )
			return;
	}

	if ( bDRSpecified && ulDebugReg > 3 )
	{
		OutputPrint( FALSE, FALSE, "Debug register expr must be between 0-3." );
		return;
	}

	if ( bADDRSpecified )
		bCodeWindowMode = FALSE;

	// Check whether the Debug Register is Available.

	if ( ulDebugReg != 0xFFFFFFFF && vbDebugRegsTaken[ ulDebugReg ] )
	{
		OutputPrint( FALSE, FALSE, "Debug register already in use." );
		return;
	}

	// Check whether using /M option and Code Window Mode.

	if ( bCodeWindowMode &&
		g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_M ].pszParam &&
		strlen( g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_M ].pszParam ) )
	{
		OutputPrint( FALSE, FALSE, "Cannot use /M option in code window." );
		return;
	}

	// Install the Breakpoint.

	if ( InstallBreakpoint( psisInts, MACRO_BREAKPOINTTYPE_EXEC, dwAddress, ulDebugReg,
		g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_P ].pszParam,
		g_vcoBpxCommandOptions[ MACRO_BPXCMDOPT_INDEX_M ].pszParam ) == FALSE )
	{
		return;
	}

	// Redraw the Code Window.

	if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
	{
		DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
		DrawScreen( pdglLayouts );
	}

	// Return to the Caller.

	return;
}

//==========================================
// ParseCommandOptions Function Definition.
//==========================================

static CHAR		g_szParseCommandOptionsBuffer[ 1024 ];

BOOLEAN ParseCommandOptions( IN CHAR* pszParams, IN COMMAND_OPTION* pcoOptions )
{
	CHAR*				pszBuffPtr = g_szParseCommandOptionsBuffer;
	COMMAND_OPTION*		pcoThis;
	CHAR*				pszSearch;
	COMMAND_OPTION*		pcoPtr;
	ULONG				ulPos, ulMaxPos;
	ULONG				ulI;
	COMMAND_OPTION*		pcoLast;

	// Iterate through the Options.

	pcoThis = pcoOptions;

	while( pcoThis->pszOption )
	{
		// Search for the Pattern.

		pszSearch = stristr( pszParams, pcoThis->pszOption );

		if ( pszSearch == NULL )
		{
			pcoThis->pszParam = NULL;
			pcoThis->ulPos = MACRO_CMDOPT_POSERR;
		}
		else
		{
			if ( stristr( pszSearch + 1, pcoThis->pszOption ) )
			{
				OutputPrint( FALSE, FALSE, "Syntax error." );
				return FALSE;
			}

			pcoThis->pszParam = NULL;
			pcoThis->ulPos = pszSearch - pszParams;
		}

		// Increment.

		pcoThis ++;
	}

	// Iterate AGAIN through the Options.

	pcoThis = pcoOptions;

	while( pcoThis->pszOption )
	{
		// Check whether there was an Occorrence.

		if ( pcoThis->ulPos != MACRO_CMDOPT_POSERR )
		{
			// Iterate through the Other Options for determining the "Max Pos".

			ulPos = pcoThis->ulPos;
			ulMaxPos = strlen( pszParams ) - 1;

			pcoPtr = pcoOptions;

			while( pcoPtr->pszOption )
			{
				if ( pcoPtr != pcoThis && pcoPtr->ulPos != MACRO_CMDOPT_POSERR )
				{
					if ( pcoPtr->ulPos > ulPos &&
						pcoPtr->ulPos <= ulMaxPos )
					{
						ulMaxPos = pcoPtr->ulPos - 1;
					}
				}
				pcoPtr++;
			}

			// Trim at the Left.

			ulPos += strlen( pcoThis->pszOption );

			while( ulPos <= ulMaxPos && pszParams[ ulPos ] == ' ' )
				ulPos ++;

			// Trim at the Right.

			while( ulMaxPos >= ulPos && pszParams[ ulMaxPos ] == ' ' )
				ulMaxPos --;

			// Save the Resulting Parameter.

			pcoThis->pszParam = pszBuffPtr;

			for ( ulI = ulPos; ulI <= ulMaxPos; ulI ++ )
				* pszBuffPtr ++ = pszParams[ ulI ];

			* pszBuffPtr ++ = '\0';
		}

		// Increment.

		pcoThis ++;
	}

	// Take care of the First Parameter.

	pcoLast = pcoOptions;
	while( pcoLast->pszOption )
		pcoLast ++;

	pcoLast->pszParam = NULL;
	pcoLast->ulPos = MACRO_CMDOPT_POSERR;

	//

	ulPos = 0;
	ulMaxPos = strlen( pszParams ) - 1;

	pcoThis = pcoOptions;
	while( pcoThis->pszOption )
	{
		// Update the Max Pos value.

		if ( pcoThis->ulPos != MACRO_CMDOPT_POSERR &&
			pcoThis->ulPos <= ulMaxPos )
		{
			ulMaxPos = pcoThis->ulPos - 1;
		}

		// Increment.

		pcoThis ++;
	}

	//

	if ( ulMaxPos < 0x80000000 )
	{
		// Trim at the Left.

		while( ulPos <= ulMaxPos && pszParams[ ulPos ] == ' ' )
			ulPos ++;

		// Trim at the Right.

		while( ulMaxPos >= ulPos && ulMaxPos < 0x80000000 && pszParams[ ulMaxPos ] == ' ' )
			ulMaxPos --;

		// Save the Resulting Parameter.

		if ( ulMaxPos < 0x80000000 )
		{
			pcoLast->pszParam = pszBuffPtr;

			for ( ulI = ulPos; ulI <= ulMaxPos; ulI ++ )
				* pszBuffPtr ++ = pszParams[ ulI ];

			* pszBuffPtr ++ = '\0';
		}
	}

	// Return to the Caller.

	return TRUE;
}

//==============================
// stristr Function Definition.
//==============================

CHAR* stristr( IN CONST CHAR* string, IN CONST CHAR* strCharSet )
{
	if ( strlen( strCharSet ) <= strlen( string ) )
	{
		size_t		size = strlen( string );
		size_t		sizeCharSet = strlen( strCharSet );
		ULONG		i;

		for ( i=0; i<=size-sizeCharSet; i++, string++ )
			if ( ! MACRO_CRTFN_NAME(memicmp)( string, strCharSet, sizeCharSet ) )
				return (CHAR*) string;
	}

	return NULL;
}

//========================================
// DtxCommandHandler Function Definition.
//========================================

#define MACRO_DTXCMDOPT_INDEX_P				0
#define MACRO_DTXCMDOPT_INDEX_M				1
#define MACRO_DTXCMDOPT_INDEX_ADDRESS		2

static COMMAND_OPTION			g_vcoDtxCommandOptions[] =
{
	MACRO_CMDOPT( "/P" ), MACRO_CMDOPT( "/M" ), MACRO_CMDOPT_END
};

VOID DtxCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	VPCICE_WINDOW*			pviwCodeWindow;
	BOOLEAN					bCodeWindowMode = FALSE;
	DWORD					dwAddress;
	BOOLEAN					bADDRSpecified;
	BOOLEAN					bEvalRes;

	pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

	// Parse the Command Options.

	if ( ParseCommandOptions( pszParams, g_vcoDtxCommandOptions ) == FALSE )
		return;

	// Get/Calculate the various Parameters.

	if ( g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_ADDRESS ].pszParam == NULL ||
		strlen( g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_ADDRESS ].pszParam ) == 0 )
	{
		// We must be in the Code Win Mode.

		if ( extension->bCursorInCodeWinMode == FALSE ||
			pviwCodeWindow == NULL ||
			pviwCodeWindow->bDisplayed == FALSE ||
			extension->dwCodeWinLastCursorPosY < pviwCodeWindow->ulY0 )
		{
			OutputPrint( FALSE, FALSE, "Address must be specified." );
			return;
		}
		else
		{
			dwAddress = g_vdwDrawCodeWindowContentsAddresses[ extension->dwCodeWinLastCursorPosY - pviwCodeWindow->ulY0 ];
			bCodeWindowMode = TRUE;
		}
	}

	// Evaluate the Expressions.

	if ( g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_ADDRESS ].pszParam &&
		strlen( g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_ADDRESS ].pszParam ) )
			bADDRSpecified = TRUE;
	else
			bADDRSpecified = FALSE;

	if ( bADDRSpecified )
	{
		dwAddress = EvaluateExpression_DWORD(
			g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_ADDRESS ].pszParam, & bEvalRes );

		if ( bEvalRes == FALSE )
			return;
	}

	if ( bADDRSpecified )
		bCodeWindowMode = FALSE;

	// Check whether using /M option and Code Window Mode.

	if ( bCodeWindowMode &&
		g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_M ].pszParam &&
		strlen( g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_M ].pszParam ) )
	{
		OutputPrint( FALSE, FALSE, "Cannot use /M option in code window." );
		return;
	}

	// Install the Detour.

	if ( InstallDetour( psisInts, MACRO_DETOURTYPE_USER, dwAddress, NULL, FALSE,
		g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_P ].pszParam,
		g_vcoDtxCommandOptions[ MACRO_DTXCMDOPT_INDEX_M ].pszParam ) == FALSE )
	{
		return;
	}

	// Redraw the Code Window.

	if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
	{
		DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
		DrawScreen( pdglLayouts );
	}

	// Return to the Caller.

	return;
}

//==========================================
// HbootCommandHandler Function Definition.
//==========================================

VOID HbootCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	// Reboot the System.

	__asm
	{
		jmp			RebootSystemNOW
	}
}

//=======================================
// HandleMouseEvent Function Definition.
//=======================================

VOID HandleMouseEvent( VOID )
{
	PDEVICE_EXTENSION	extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*	pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG				ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG				ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	BYTE				bMousePacket[ MACRO_MOUSEPACKET_MAXSIZE ];
	ULONG				ulReadyPacketSize;
	BOOLEAN				bIsMousePacketReady = FALSE;
	CHAR				cWheelComponent;
	LONG				lXComponent, lYComponent;
	BOOLEAN				bLeftButton, bRightButton, bMiddleButton;
	BOOLEAN				bDrawScreenRequired = FALSE;
	ULONG				ulBorderOffset = 0;

	// === Get the Packet Data. ===

	//
	// ... entering synchronized area ...
	//

	__asm
	{
		cli // <-- important to avoid dead locks across processors... !!!
		mov			edx, extension
		lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus
		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslMousePacketSpinLock
		call		EnterMultiProcessorSpinLock
	}

		if ( extension->sisSysInterrStatus.bIsMousePacketReady )
		{
			// Copy the Informations.

			bIsMousePacketReady = TRUE;
			memcpy( bMousePacket, extension->sisSysInterrStatus.bMousePacket,
				extension->sisSysInterrStatus.ulReadyPacketSize );
			ulReadyPacketSize = extension->sisSysInterrStatus.ulReadyPacketSize;

			// The Packet was Received.

			extension->sisSysInterrStatus.bIsMousePacketReady = FALSE;
		}

	__asm
	{
		mov			edx, extension
		lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus
		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslMousePacketSpinLock
		call		LeaveMultiProcessorSpinLock
		sti
	}

	//
	// ... leaving synchronized area ...
	//

	// === Check whether we can Continue... ===

	if ( bIsMousePacketReady == FALSE )
		return;

	// === Process the Mouse Packet. ===

	// No Overflow is allowed.

	if ( ( bMousePacket[ 0 ] & (1<<6) ) ||
		( bMousePacket[ 0 ] & (1<<7) ) )
	{
		return;
	}

	// Read the Buttons information.

	bLeftButton = ( bMousePacket[ 0 ] & (1<<0) ) != 0;
	bRightButton = ( bMousePacket[ 0 ] & (1<<1) ) != 0;
	bMiddleButton = ( bMousePacket[ 0 ] & (1<<2) ) != 0;

	// Extract the Informations from the Packet.

	if ( bMousePacket[ 0 ] & (1<<4) )
		lXComponent = 0xFFFFFF00;
	else
		lXComponent = 0;

	if ( bMousePacket[ 0 ] & (1<<5) )
		lYComponent = 0xFFFFFF00;
	else
		lYComponent = 0;

	lXComponent |= bMousePacket[ 1 ];
	lYComponent |= bMousePacket[ 2 ];

	switch( extension->sisSysInterrStatus.bMouseDeviceID )
	{
	case 0: // ### Standard Ps/2 Mouse ###

		cWheelComponent = 0;

		break;

	case 3: // ### Standard Intellimouse ###
	case 4: // ### Intellimouse With 5 Buttons ###

		if ( bMousePacket[ 3 ] & 0x8 )
			cWheelComponent = (BYTE) 0xF8;
		else
			cWheelComponent = 0;

		cWheelComponent |= ( bMousePacket[ 3 ] & 0x7 );

		break;

	default:
		return;
	}

	// === Update the Mouse Pointer position. ===

	if ( lXComponent || lYComponent )
	{
		// Update the Float Variables.

		pdglLayouts->vivmVpcICEVideo.fMousePointerX += ( (float) lXComponent ) * pdglLayouts->vivmVpcICEVideo.fMovementXFactor;
		pdglLayouts->vivmVpcICEVideo.fMousePointerY -= ( (float) lYComponent ) * pdglLayouts->vivmVpcICEVideo.fMovementYFactor;

		// Validation.

		if ( pdglLayouts->vmiVideoMemInfo.pvPrimary )
			ulBorderOffset = 1;

		if ( pdglLayouts->vivmVpcICEVideo.fMousePointerX < ulBorderOffset )
			pdglLayouts->vivmVpcICEVideo.fMousePointerX = (float) ulBorderOffset;
		if ( pdglLayouts->vivmVpcICEVideo.fMousePointerY < ulBorderOffset )
			pdglLayouts->vivmVpcICEVideo.fMousePointerY = (float) ulBorderOffset;

		if ( pdglLayouts->vivmVpcICEVideo.fMousePointerX >= ulConsoleW - ulBorderOffset )
			pdglLayouts->vivmVpcICEVideo.fMousePointerX = (float) ( ulConsoleW - ulBorderOffset - 1 );
		if ( pdglLayouts->vivmVpcICEVideo.fMousePointerY >= ulConsoleH - ulBorderOffset )
			pdglLayouts->vivmVpcICEVideo.fMousePointerY = (float) ( ulConsoleH - ulBorderOffset - 1 );

		// Update: Float to Int.

		pdglLayouts->vivmVpcICEVideo.ulMousePointerX = (ULONG) pdglLayouts->vivmVpcICEVideo.fMousePointerX;
		pdglLayouts->vivmVpcICEVideo.ulMousePointerY = (ULONG) pdglLayouts->vivmVpcICEVideo.fMousePointerY;

		// We must Redraw.

		bDrawScreenRequired = TRUE;
	}

	// === Draw the Updated Screen. ===

	if ( bDrawScreenRequired )
		DrawScreen( pdglLayouts );

	// === Return. ===

	return;
}

//=========================================
// EnteringReportMode Function Definition.
//=========================================

VOID EnteringReportMode( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	VPCICE_WINDOW*			pviwOutputWindow;

	// Report Mode Enter.

	pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
	if ( pviwOutputWindow )
	{
		// Setup the Variables.

		extension->ulReportModeLinesTotal =
			extension->ulReportModeLinesLeft =
				pviwOutputWindow->ulY1 - pviwOutputWindow->ulY0 + 1;

		extension->bReportMode = TRUE;
		extension->bReportModeAborted = FALSE;
		extension->bReportModeMsgPrinted = FALSE;
		extension->bReportMode1StScrollDone = FALSE;
	}

	// Return.

	return;
}

//========================================
// ExitingReportMode Function Definition.
//========================================

VOID ExitingReportMode( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	VPCICE_WINDOW*			pviwOutputWindow;

	// Scroll the Output Window and Restore the GUI.

	pviwOutputWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_OUTPUT, & pdglLayouts->vivmVpcICEVideo );
	if ( pviwOutputWindow )
	{
		ULONG			ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ULONG			ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
		ULONG			ulLinesToMove, ulBytesToMove;
		ULONG			ulI;
		WORD*			pwPtrSource;
		WORD*			pwPtrDest;
		WORD*			pwPtr;

		// Scroll the Window.

		ulLinesToMove = pviwOutputWindow->ulY1 - pviwOutputWindow->ulY0;
		ulBytesToMove = ( pviwOutputWindow->ulX1 - pviwOutputWindow->ulX0 + 1 ) * sizeof( WORD );

		for ( ulI = 0; ulI < ulLinesToMove; ulI ++ )
		{
			// Scroll the Line.

			pwPtrSource = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwOutputWindow->ulX0, pviwOutputWindow->ulY0 + 1 + ulI );
			pwPtrDest = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwOutputWindow->ulX0, pviwOutputWindow->ulY0 + ulI );

			memcpy( pwPtrDest, pwPtrSource, ulBytesToMove );
		}

		// Draw the Last Line.

		pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwOutputWindow->ulX0, pviwOutputWindow->ulY1 );

		* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ':' );

		for ( ulI = pviwOutputWindow->ulX0 + 1; ulI <= pviwOutputWindow->ulX1; ulI ++ )
			* pwPtr ++ = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

		// Take care of the Cursor Blink State.

		if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
			MakeCursorBlink();

		// Check whether restoring the Cursor Position.

		if ( extension->bReportModeMsgPrinted )
		{
			pdglLayouts->vivmVpcICEVideo.ulCursorX = extension->ulReportModeOldCursorX;
			pdglLayouts->vivmVpcICEVideo.ulCursorY = extension->ulReportModeOldCursorY;
		}

		// Draw the Updated Screen.

		DrawScreen( pdglLayouts );
	}

	// Report Mode Exit.

	extension->bReportMode = FALSE;

	// Return.

	return;
}

//=======================================
// WaitForUserInput Function Definition.
//=======================================

VOID WaitForUserInput( OUT BYTE* pbLastAsciiCodeAcquired, OUT BYTE* pbLastScanCodeAcquired )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	// Draw the Blinking Cursor.

	do
	{
		DebuggerLoopQuantum( extension->liCursorCpuCycles, & extension->sisSysInterrStatus );
		MakeCursorBlink();
	}
	while( extension->sisSysInterrStatus.bLastAsciiCodeAcquired == 0 );

	// Return the Key Stroke informations.

	if ( pbLastAsciiCodeAcquired )
		* pbLastAsciiCodeAcquired = extension->sisSysInterrStatus.bLastAsciiCodeAcquired;
	if ( pbLastScanCodeAcquired )
		* pbLastScanCodeAcquired = extension->sisSysInterrStatus.bLastScanCodeAcquired;

	// Reset the ASCII Code.

	extension->sisSysInterrStatus.bLastAsciiCodeAcquired = 0;

	// Return to the Caller.

	return;
}

//=======================================
// BcCommandHandler Function Definition.
//=======================================

VOID BcCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	BOOLEAN					bUpdateCodeWindow = FALSE;
	ULONG					ulI;
	BREAKPOINT*				pbThis;
	DWORD					vdwIndexes[ 64 ];
	ULONG					ulIndexesNum;

	// Check the Command Parameters.

	if ( strcmp( pszParams, "*" ) == 0 )
	{
		// Clear all the Breakpoints.

		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & psisInts->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
				DeleteBreakpoint( pbThis );
		}

		bUpdateCodeWindow = TRUE;
	}
	else if ( strlen( pszParams ) )
	{
		// Clear the Specified Breakpoints.

		ulIndexesNum = ParseHexNumSeriesString(
			vdwIndexes, sizeof( vdwIndexes ) / sizeof( DWORD ), pszParams );

		if ( ulIndexesNum == MACRO_PARSEHEXNUMSERIESSTRING_ERR )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
		}
		else
		{
			for ( ulI = 0; ulI < ulIndexesNum; ulI ++ )
				if ( vdwIndexes[ ulI ] >= 0 && vdwIndexes[ ulI ] < MACRO_MAXNUM_OF_BREAKPOINTS )
				{
					pbThis = & psisInts->vbpBreakpoints[ vdwIndexes[ ulI ] ];
					if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
						DeleteBreakpoint( pbThis );
				}

			bUpdateCodeWindow = TRUE;
		}
	}
	else
	{
		// Print an Error Message.

		OutputPrint( FALSE, FALSE, "Specify the breakpoints to clear." );
	}

	// Update the Code Window.

	if ( bUpdateCodeWindow )
	{
		ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
		VPCICE_WINDOW*			pviwCodeWindow;

		pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

		if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
		{
			DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
			DrawScreen( pdglLayouts );
		}
	}

	// Return to the Caller.

	return;
}

//========================================
// DtcCommandHandler Function Definition.
//========================================

VOID DtcCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	BOOLEAN					bUpdateCodeWindow = FALSE;
	ULONG					ulI;
	DETOUR*					pdThis;
	DWORD					vdwIndexes[ 64 ];
	ULONG					ulIndexesNum;

	// Check the Command Parameters.

	if ( strcmp( pszParams, "*" ) == 0 )
	{
		// Clear all the Detours.

		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
		{
			pdThis = & psisInts->vdDetours[ ulI ];
			if ( pdThis->ulType != MACRO_DETOURTYPE_UNUSED )
				DeleteDetour( pdThis );
		}

		bUpdateCodeWindow = TRUE;
	}
	else if ( strlen( pszParams ) )
	{
		// Clear the Specified Detours.

		ulIndexesNum = ParseHexNumSeriesString(
			vdwIndexes, sizeof( vdwIndexes ) / sizeof( DWORD ), pszParams );

		if ( ulIndexesNum == MACRO_PARSEHEXNUMSERIESSTRING_ERR )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
		}
		else
		{
			for ( ulI = 0; ulI < ulIndexesNum; ulI ++ )
				if ( vdwIndexes[ ulI ] >= 0 && vdwIndexes[ ulI ] < MACRO_MAXNUM_OF_DETOURS )
				{
					pdThis = & psisInts->vdDetours[ vdwIndexes[ ulI ] ];
					if ( pdThis->ulType != MACRO_DETOURTYPE_UNUSED )
						DeleteDetour( pdThis );
				}

			bUpdateCodeWindow = TRUE;
		}
	}
	else
	{
		// Print an Error Message.

		OutputPrint( FALSE, FALSE, "Specify the detours to clear." );
	}

	// Update the Code Window.

	if ( bUpdateCodeWindow )
	{
		ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
		VPCICE_WINDOW*			pviwCodeWindow;

		pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

		if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
		{
			DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
			DrawScreen( pdglLayouts );
		}
	}

	// Return to the Caller.

	return;
}

//==============================================
// ParseHexNumSeriesString Function Definition.
//==============================================

ULONG ParseHexNumSeriesString( OUT DWORD* pdwOutputSeries, IN ULONG ulOutputSeriesMaxItemsNum, IN CHAR* pszInputString )
{
	ULONG		ulCounter = 0;
	CHAR*		pszPtr = pszInputString;
	CHAR*		pszStart;
	DWORD		dwNumber;
	CHAR*		pszEndPtr;

	while( TRUE )
	{
		// Search for the Next Token.

		if ( * pszPtr == 0 )
		{
			break;
		}
		else if ( * pszPtr == ' ' )
		{
			pszPtr ++;
			continue;
		}

		// Isolate the Found Token.

		pszStart = pszPtr;

		while( TRUE )
		{
			if ( * pszPtr == 0 )
			{
				break;
			}
			else if ( * pszPtr == ' ' )
			{
				* pszPtr ++ = 0;
				break;
			}
			else
			{
				pszPtr ++;
			}
		}

		// Parse the Token.

		dwNumber = MACRO_CRTFN_NAME(strtoul)( pszStart, & pszEndPtr, 16 );
		if ( pszEndPtr && * pszEndPtr != 0 )
		{
			return MACRO_PARSEHEXNUMSERIESSTRING_ERR;
		}
		else
		{
			if ( ulCounter < ulOutputSeriesMaxItemsNum )
				pdwOutputSeries[ ulCounter ++ ] = dwNumber;
			else
				break;
		}
	}

	// Return to the Caller.

	return ulCounter;
}

//==============================================
// DrawScriptWinLnColInfos Function Definition.
//==============================================

static BOOLEAN IsCharAnHexNum( IN CHAR cC )
{
	if (
			( cC >= '0' && cC <= '9' ) ||
			( cC >= 'a' && cC <= 'f' ) ||
			( cC >= 'A' && cC <= 'F' )
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID DrawScriptWinLnColInfos( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	VPCICE_WINDOW*			pviwScriptWindow;
	CHAR					szStrBuffer[ 100 ];
	ULONG					ulX, ulW, ulSize, ulNum, ulI;
	CHAR					szFunctionName[ sizeof( g_szCurrentFunctionName ) ];
	WORD*					pwPtr;

	// Validate Cursor/Position Variables.

	ValidateScriptWinLnColInfos();

	// Draw the "Ln, Col" Infos only if the Window is Displayed.

	pviwScriptWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_SCRIPT, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed )
	{
		// Clear the Line before Writing.

		pwPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwScriptWindow->ulX0,
			pviwScriptWindow->ulY0 - 1 );

		ulW = pviwScriptWindow->ulX1 - pviwScriptWindow->ulX0 + 1;
		for ( ulI = 0; ulI < ulW; ulI ++ )
			* pwPtr ++ = CLRTXT2VIDBUFWORD( 2, 0, 196 );

		// Draw the String concerning the LnCol Information.

		sprintf( szStrBuffer,
			"%s\xC4" "Ln(%i)" "\xC4" "Col(%i)" "\xC4" "Dim(%i)" /* ### Consider the sizeof(szStrBuffer) when Changing this string. ###  */,
			extension->bScriptWinDirtyBit ? "!0B(D)!02" : "",
			extension->ulScriptWinLn, extension->ulScriptWinCol,
			extension->ulScriptWinBufferPosInBytes / sizeof( WORD ) );

		ulX = pviwScriptWindow->ulX1 - strlen( szStrBuffer );
		if ( extension->bScriptWinDirtyBit )
			ulX += 6; // ## WARNING ## : This is necessary to Compensate the Presence of the Color Escapes !!

		OutputTextStringSpecial(
			pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			ulX,
			pviwScriptWindow->ulY0 - 1,
			0xFFFFFFFF /* very high line size */,
			0x02, szStrBuffer
		);

		// Draw the Function Name.

		strcpy( szFunctionName, g_szCurrentFunctionName );

		ulW = ulX - pviwScriptWindow->ulX0 - 2 - 2;
		if ( ulW < sizeof( szFunctionName ) )
			szFunctionName[ ulW ] = 0;

		ulSize = strlen( szFunctionName );

		if ( ulSize == 20 &&
			memcmp( szFunctionName, "BreakpointHandler", 17 ) == 0 &&
			IsCharAnHexNum( szFunctionName[ ulSize - 1 ] ) &&
			IsCharAnHexNum( szFunctionName[ ulSize - 2 ] ) &&
			IsCharAnHexNum( szFunctionName[ ulSize - 3 ] ) )
		{
			ulNum = MACRO_CRTFN_NAME(strtoul)( & szFunctionName[ ulSize - 3 ], NULL, 16 );
			sprintf( szFunctionName, "(handler of breakpoint #%x)", ulNum );
		}
		else if ( ulSize == 16 &&
			memcmp( szFunctionName, "DetourHandler", 13 ) == 0 &&
			IsCharAnHexNum( szFunctionName[ ulSize - 1 ] ) &&
			IsCharAnHexNum( szFunctionName[ ulSize - 2 ] ) &&
			IsCharAnHexNum( szFunctionName[ ulSize - 3 ] ) )
		{
			ulNum = MACRO_CRTFN_NAME(strtoul)( & szFunctionName[ ulSize - 3 ], NULL, 16 );
			sprintf( szFunctionName, "(handler of detour #%x)", ulNum );
		}

		OutputTextString(
			pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
			pviwScriptWindow->ulX0 + 2,
			pviwScriptWindow->ulY0 - 1,
			0x02, szFunctionName
		);
	}

	// Return.

	return;
}

//==================================================
// ValidateScriptWinLnColInfos Function Definition.
//==================================================

VOID ValidateScriptWinLnColInfos( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	VPCICE_WINDOW*			pviwScriptWindow;
	LONG					lCurX, lCurY;
	ULONG					ulW, ulH;

	//
	// ## ## Validate the "Ln, Col" Infos. ## ##
	// The Purpose is that the System Cursor must be always Visible in the Script Window.
	//

	// Check whether we can Continue.

	pviwScriptWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_SCRIPT, & pdglLayouts->vivmVpcICEVideo );

	if ( pviwScriptWindow == NULL || pviwScriptWindow->bDisplayed == FALSE )
		return;

	// Check the Position in the Script File.

	if ( extension->ulScriptWinCol > MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
		extension->ulScriptWinCol = MACRO_SCRIPTWIN_LINESIZE_IN_CHARS;

	// Check the Position of the Cursor.

	ulW = pviwScriptWindow->ulX1 - pviwScriptWindow->ulX0 + 1;
	ulH = pviwScriptWindow->ulY1 - pviwScriptWindow->ulY0 + 1;

	lCurX = (LONG) extension->ulScriptWinCol - 1 - extension->ulScriptWinOffsetX;
	lCurY = (LONG) extension->ulScriptWinLn - 1 - extension->ulScriptWinOffsetY;

	if ( lCurX < 0 )
		extension->ulScriptWinOffsetX -= ( -lCurX );
	else if ( lCurX >= (LONG) ulW )
		extension->ulScriptWinOffsetX += ( lCurX - ulW + 1 );

	if ( lCurY < 0 )
		extension->ulScriptWinOffsetY -= ( -lCurY );
	else if ( lCurY >= (LONG) ulH )
		extension->ulScriptWinOffsetY += ( lCurY - ulH + 1 );

	// Return.

	return;
}

//===============================================
// DrawScriptWindowContents Function Definition.
//===============================================

VOID DrawScriptWindowContents( IN BOOLEAN bEndLine, IN BOOLEAN bSetCursorPos )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	VPCICE_WINDOW*			pviwScriptWindow;
	WORD*					pwVideoPtr;
	WORD*					pwVideoLineEnd;
	WORD*					pwStringPtr;
	ULONG					ulI, ulJ;
	ULONG					ulColumnsNum, ulLinesNum;
	ULONG					ulLine, ulOffsetCounter;
	ULONG					ulCurX, ulCurY;
	BOOLEAN					bEditing = FALSE;

	pviwScriptWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_SCRIPT, & pdglLayouts->vivmVpcICEVideo );

	// Check whether we are Editing.

	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed &&
		pdglLayouts->vivmVpcICEVideo.ulCursorX != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwScriptWindow->ulY0 &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwScriptWindow->ulY1 )
	{
		bEditing = TRUE;
	}

	// Validate Cursor/Position Variables.

	ValidateScriptWinLnColInfos();

	//
	// Take the Contents of the Working Line and Update the Script File Contents.
	//

	if ( bEndLine )
		EndScriptWindowLineEdit();

	// Draw the Line/Col Information.

	DrawScriptWinLnColInfos();

	// Draw the Contents of the Script Window.

	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed )
	{
		//
		// Draw each Line in Turn.
		//

		ulLinesNum = pviwScriptWindow->ulY1 - pviwScriptWindow->ulY0 + 1;
		ulColumnsNum = pviwScriptWindow->ulX1 - pviwScriptWindow->ulX0 + 1;

		for ( ulI = 0; ulI < ulLinesNum; ulI ++ )
		{
			pwVideoPtr = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwScriptWindow->ulX0, pviwScriptWindow->ulY0 + ulI );
			pwVideoLineEnd = GetPtrInTextBuffer( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
				pviwScriptWindow->ulX1, pviwScriptWindow->ulY0 + ulI );

			// Reset the Underlaying background.

			for ( ulJ = 0; ulJ < ulColumnsNum; ulJ ++ )
				* ( pwVideoPtr + ulJ ) = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

			// Draw the Line.

			ulLine = extension->ulScriptWinOffsetY + ulI; // = 0-based absolute line index.
			if ( ulLine >= extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) )
				continue;

			pwStringPtr = extension->ppwScriptWinStrPtrsBuffer[ ulLine ];
			ulOffsetCounter = extension->ulScriptWinOffsetX;

			while( * pwStringPtr )
			{
				// Draw the Character.

				if ( ulOffsetCounter )
				{
					-- ulOffsetCounter;
				}
				else
				{
					* pwVideoPtr = * pwStringPtr;

					if ( pwVideoPtr == pwVideoLineEnd )
						break;
					else
						++ pwVideoPtr;
				}

				// Increment.

				++ pwStringPtr;
			}
		}

		//
		// Check whether the Cursor requires being Positioned.
		//

		if ( bSetCursorPos && bEditing )
		{
			//
			// Propose new Coordinates for the Cursor.
			//  The Validation Code called at the Beginning of this Function should ensure that the Vertical/Horizontal
			//  position of the Cursor can be computed safely.
			//

			ulCurX = pviwScriptWindow->ulX0 - extension->ulScriptWinOffsetX + extension->ulScriptWinCol - 1;
			ulCurY = pviwScriptWindow->ulY0 - extension->ulScriptWinOffsetY + extension->ulScriptWinLn - 1;

			// Check whether the Cursor Position changed.

			if ( ulCurX != pdglLayouts->vivmVpcICEVideo.ulCursorX ||
				ulCurY != pdglLayouts->vivmVpcICEVideo.ulCursorY )
			{
				// Change the Cursor Position.

				if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
					MakeCursorBlink();

				pdglLayouts->vivmVpcICEVideo.ulCursorX = ulCurX;
				pdglLayouts->vivmVpcICEVideo.ulCursorY = ulCurY;
			}
		}
	}

	// Return.

	return;
}

//================================================
// BeginScriptWindowLineEdit Function Definition.
//================================================

VOID BeginScriptWindowLineEdit( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	VPCICE_WINDOW*			pviwScriptWindow;
	BOOLEAN					bEditing = FALSE;
	ULONG					ulLine;
	ULONG					ulI;
	WORD*					pwStringPtr;
	WORD*					pwLinePtr;
	WORD					wWord;

	pviwScriptWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_SCRIPT, & pdglLayouts->vivmVpcICEVideo );

	// Check whether we are Editing.

	if ( pviwScriptWindow && pviwScriptWindow->bDisplayed &&
		pdglLayouts->vivmVpcICEVideo.ulCursorX != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY != MACRO_CURSORPOS_UNDEFINED_VALUE &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY >= pviwScriptWindow->ulY0 &&
		pdglLayouts->vivmVpcICEVideo.ulCursorY <= pviwScriptWindow->ulY1 )
	{
		bEditing = TRUE;
	}

	// Validate Cursor/Position Variables.

	ValidateScriptWinLnColInfos();

	//
	// ### Popolate the Working Line. ###
	//

	// Check whether Editing.

	if ( bEditing == FALSE ||
		pviwScriptWindow == NULL || pviwScriptWindow->bDisplayed == FALSE )
	{
		extension->ulScriptWinLineIndex = 0xFFFFFFFF;
		return;
	}

	// Calculate the Current Line.

	ulLine = extension->ulScriptWinOffsetY +
		( pdglLayouts->vivmVpcICEVideo.ulCursorY - pviwScriptWindow->ulY0 );

	// Reset the Working Line Buffer.

	for ( ulI = 0; ulI < MACRO_SCRIPTWIN_LINESIZE_IN_CHARS; ulI ++ )
		extension->vwScriptWinLine[ ulI ] = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	// Check whether the Line has Storage associated to It.

	if ( ulLine >= extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) )
	{
		extension->ulScriptWinLineIndex = 0xEEEEEEEE;
		extension->ulScriptWinNoStorageLine = ulLine;
		return;
	}

	// Copy the Line Contents.

	pwStringPtr = extension->ppwScriptWinStrPtrsBuffer[ ulLine ];
	pwLinePtr = extension->vwScriptWinLine;

	for ( ulI = 0; ulI < MACRO_SCRIPTWIN_LINESIZE_IN_CHARS; ulI ++ )
	{
		wWord = * pwStringPtr ++;
		if ( ! wWord )
			break;

		* pwLinePtr ++ = wWord;
	}

	// Return.

	extension->ulScriptWinLineIndex = ulLine;
	return;
}

//==============================================
// EndScriptWindowLineEdit Function Definition.
//==============================================

VOID EndScriptWindowLineEdit( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	WORD*					pwLineEnd;
	ULONG					ulLineLen, ulPrevLen;
	ULONG					ulNullLinesNum;
	ULONG					ulBytesToAddToBuff = 0, ulBytesToAddToStrPtrs = 0;
	ULONG					ulBytesToSubToBuff = 0;
	ULONG					ulI;
	WORD*					pwPtr;
	WORD*					pwPtr2;
	WORD*					pwPtr3;
	WORD**					ppwPtrPtr;
	ULONG					ulLineId;
	ULONG					ulSize, ulSize2, ulDim;

	// Check the Line Index.

	if ( extension->ulScriptWinLineIndex == 0xFFFFFFFF )
		return;

	//
	// ### Update the Script File Contents. ###
	//

	// Get the Size of the Working Line - IN CHARACTERS.

	pwLineEnd = extension->vwScriptWinLine + MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1;
	while( pwLineEnd >= & extension->vwScriptWinLine[ 0 ] && * pwLineEnd == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
		pwLineEnd --;

	if ( pwLineEnd < & extension->vwScriptWinLine[ 0 ] )
		ulLineLen = 1;	// # # There is Nothing on this Line: Include only the NULL CHR. # #
	else
		ulLineLen = pwLineEnd - & extension->vwScriptWinLine[ 0 ] + 1 + 1 /* NULL CHR INCLUDED */;

	// Get the Size of the Previous Line - IN CHARACTERS.

	if ( extension->ulScriptWinLineIndex == 0xEEEEEEEE )
		ulPrevLen = 0;
	else
		ulPrevLen =
			WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinLineIndex ] ) + 1 /* NULL CHR INCLUDED */;

	//
	// Calculate the Number of NULL Lines between the Edited Line and the Rest of the Script File.
	// One or more NULL Lines may happen when the User is editing a Script Line that has no Storage associated to it.
	//

	if ( extension->ulScriptWinLineIndex == 0xEEEEEEEE )
	{
		//
		// Calculate the Number of Null Lines.
		// ## ## WARNING ## ##: This count includes ALSO the Edited Line !!!
		//

		ulNullLinesNum =
			extension->ulScriptWinNoStorageLine - extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) + 1;

		//
		// Set the Line Index.
		//

		ulLineId = extension->ulScriptWinNoStorageLine;
	}
	else
	{
		//
		// No Null Lines.
		//

		ulNullLinesNum = 0;

		//
		// Set the Line Index.
		//

		ulLineId = extension->ulScriptWinLineIndex;
	}

	// Check whether there is Enough Memory for this Operation.

	if ( ulLineLen > ulPrevLen )
	{
		// Calculate the Variables.

		ulBytesToAddToBuff =
			( ulLineLen - ulPrevLen ) * sizeof( WORD ) +
			( ulNullLinesNum ? ulNullLinesNum - 1 : 0 ) * sizeof( WORD );
		ulBytesToAddToStrPtrs =
			ulNullLinesNum * sizeof( WORD* );

		// Check the Limits.

		if ( extension->ulScriptWinBufferPosInBytes + ulBytesToAddToBuff > extension->ulScriptWinBufferSizeInBytes ||
			extension->ulScriptWinStrPtrsBufferPosInBytes + ulBytesToAddToStrPtrs > extension->ulScriptWinStrPtrsBufferSizeInBytes )
		{
			//
			// Return to the Caller: there is NO SPACE in one of the Buffers for copying the Working Line.
			//

			return;
		}
	}
	else
	{
		// Set the Number of Bytes to Subtract from the Pos Counter.

		ulBytesToSubToBuff =
			( ulPrevLen - ulLineLen ) * sizeof( WORD );
	}

	// Add the Null Lines.

	if ( ulNullLinesNum )
	{
		// Add Them.

		pwPtr = extension->pwScriptWinBuffer + ( extension->ulScriptWinBufferPosInBytes / sizeof( WORD ) );
		ppwPtrPtr = extension->ppwScriptWinStrPtrsBuffer + ( extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) );

		for ( ulI = 0; ulI < ulNullLinesNum; ulI ++ )
		{
			pwPtr[ ulI ] = 0;
			ppwPtrPtr[ ulI ] = & pwPtr[ ulI ];
		}

		// Update the Pos Variables.

		ulBytesToAddToBuff -= ulNullLinesNum * sizeof( WORD );
		extension->ulScriptWinBufferPosInBytes += ulNullLinesNum * sizeof( WORD );
		extension->ulScriptWinStrPtrsBufferPosInBytes += ulBytesToAddToStrPtrs;
	}

	// Shrink or Grow the Buffer Memory.

	if ( ulLineLen > ulPrevLen ) // # Growing... #
	{
		//
		// ### Move the Memory of the Script Buffer. ###
		//

		// The Number of WORDS the Buffer should be Increased.

		ulSize =
			ulLineLen - ulPrevLen;

		// Pointer to the current Edited Line.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ ulLineId ];

		// Pointers to: first Word after current Edited Line, Word where new Block should be Moved.

		pwPtr3 =
			pwPtr + WordStringLen( pwPtr ) + 1;
		pwPtr2 =
			pwPtr3 + ulSize;

		// Pointer to the Last Line, Dimension of the Block to Move.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ];
		ulDim =
			( pwPtr + WordStringLen( pwPtr ) + 1 ) - pwPtr3;

		// Adjust the Pointers and Move the Memory.

		if ( ulDim )
		{
			pwPtr3 += ulDim - 1;
			pwPtr2 += ulDim - 1;

			__asm
			{
				mov			ecx, ulDim
				mov			esi, pwPtr3
				mov			edi, pwPtr2

				std
				rep			movsw
				cld
			}
		}

		//
		// ### Adjust the Pointers Buffer. ###
		//

		if ( ulDim )
		{
			ulSize2 = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

			for ( ulI = ulLineId + 1; ulI < ulSize2; ulI ++ )
				extension->ppwScriptWinStrPtrsBuffer[ ulI ] += ulSize;
		}
	}
	else if ( ulLineLen < ulPrevLen ) // # Shrinking... #
	{
		//
		// ### Move the Memory of the Script Buffer. ###
		//

		// The Number of WORDS the Buffer should be Decreased.

		ulSize =
			ulPrevLen - ulLineLen;

		// Pointer to the current Edited Line.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ ulLineId ];

		// Pointers to: first Word after current Edited Line, Word where new Block should be Moved.

		pwPtr3 =
			pwPtr + WordStringLen( pwPtr ) + 1;
		pwPtr2 =
			pwPtr3 - ulSize;

		// Pointer to the Last Line, Dimension of the Block to Move.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ];
		ulDim =
			( pwPtr + WordStringLen( pwPtr ) + 1 ) - pwPtr3;

		// Move the Memory.

		if ( ulDim )
		{
			__asm
			{
				mov			ecx, ulDim
				mov			esi, pwPtr3
				mov			edi, pwPtr2

				rep			movsw
			}
		}

		//
		// ### Adjust the Pointers Buffer. ###
		//

		if ( ulDim )
		{
			ulSize2 = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

			for ( ulI = ulLineId + 1; ulI < ulSize2; ulI ++ )
				extension->ppwScriptWinStrPtrsBuffer[ ulI ] -= ulSize;
		}
	}

	// Copy the Updated Working Line.

	memcpy( extension->ppwScriptWinStrPtrsBuffer[ ulLineId ], extension->vwScriptWinLine, ( ulLineLen - 1 ) * sizeof( WORD ) );
	extension->ppwScriptWinStrPtrsBuffer[ ulLineId ][ ulLineLen - 1 ] = 0;

	// Apply the Syntax Colors to the New String.

	ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ ulLineId ] );

	// Update the Pos Var.

	if ( ulBytesToSubToBuff )
		extension->ulScriptWinBufferPosInBytes -= ulBytesToSubToBuff;
	else
		extension->ulScriptWinBufferPosInBytes += ulBytesToAddToBuff;

	// Check whether There are NULL Lines to Remove.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	for ( ulI = ulSize - 1; ulI < 0x80000000; ulI -- )
		if ( * extension->ppwScriptWinStrPtrsBuffer[ ulI ] == 0 )
		{
			extension->ulScriptWinBufferPosInBytes -= sizeof( WORD );
			extension->ulScriptWinStrPtrsBufferPosInBytes -= sizeof( WORD* );
		}
		else
			break;

	// Return.

	return;
}

//====================================
// AddScriptLine Function Definition.
//====================================

VOID AddScriptLine( IN ULONG ulLineId, IN WORD* pwLineText )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulNullLinesNum;
	ULONG					ulSize, ulSize2;
	ULONG					ulBytesToAddToBuff, ulBytesToAddToStrPtrs;
	ULONG					ulLineTextLen;
	WORD*					pwPtr;
	WORD*					pwPtr2;
	WORD*					pwPtr3;
	WORD**					ppwPtrPtr;
	ULONG					ulI;
	ULONG					ulDim;

	ulLineTextLen = WordStringLen( pwLineText );

	// Calculate the Number of NULL Lines.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	if ( ulLineId >= ulSize )
		ulNullLinesNum = ulLineId - ulSize + 1; // # Note that this Count INCLUDES the Added Line Itself !! #
	else
		ulNullLinesNum = 0;

	//
	// Check whether there is Enough Memory for this Operation.
	//

	if ( ulNullLinesNum == 0 ) // # Adding the Line in the Middle of the Script File. #
	{
		ulBytesToAddToBuff = ( ulLineTextLen + 1 ) * sizeof( WORD );
		ulBytesToAddToStrPtrs = sizeof( WORD* );
	}
	else // # Adding the Line at the END of the Script File.
	{
		ulBytesToAddToBuff = ( ulLineTextLen + 1 ) * sizeof( WORD ) + ( ulNullLinesNum - 1 ) * sizeof( WORD );
		ulBytesToAddToStrPtrs = ulNullLinesNum * sizeof( WORD* );
	}

	if ( extension->ulScriptWinBufferPosInBytes + ulBytesToAddToBuff > extension->ulScriptWinBufferSizeInBytes ||
		extension->ulScriptWinStrPtrsBufferPosInBytes + ulBytesToAddToStrPtrs > extension->ulScriptWinStrPtrsBufferSizeInBytes )
	{
		//
		// ### WARNING ### Not sufficient Memory.
		//

		return;
	}

	//
	// ## Add the Line: at the End of the Script File or in the Middle of It. ##
	//

	// Make room for the String and Copy the Text.

	if ( ulNullLinesNum == 0 ) // # Adding the Line in the Middle of the Script File. #
	{
		//
		// # Make Room. #
		//

		// The Number of WORDS the Buffer should be Increased.

		ulSize =
			ulLineTextLen + 1;

		// Pointer to the Line that will be the Next to the Inserted One.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ ulLineId ];

		// Pointers to: first Word to Move, Word where new Block should be Moved.

		pwPtr3 =
			pwPtr;
		pwPtr2 =
			pwPtr3 + ulSize;

		// Pointer to the Last Line, Dimension of the Block to Move.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ];
		ulDim =
			( pwPtr + WordStringLen( pwPtr ) + 1 ) - pwPtr3;

		// Adjust the Pointers and Move the Memory.

		if ( ulDim )
		{
			pwPtr3 += ulDim - 1;
			pwPtr2 += ulDim - 1;

			__asm
			{
				mov			ecx, ulDim
				mov			esi, pwPtr3
				mov			edi, pwPtr2

				std
				rep			movsw
				cld
			}
		}

		//
		// ### Adjust the Pointers Buffer. ###
		//

		if ( ulDim )
		{
			ulSize2 = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

			for ( ulI = ulSize2 - 1; ulI >= ulLineId && ulI < 0x80000000; ulI -- )
				extension->ppwScriptWinStrPtrsBuffer[ ulI + 1 ] = extension->ppwScriptWinStrPtrsBuffer[ ulI ] + ulSize;
		}
	}
	else // # Adding the Line at the END of the Script File.
	{
		// Add the Lines at the End of the File.

		pwPtr = extension->pwScriptWinBuffer + ( extension->ulScriptWinBufferPosInBytes / sizeof( WORD ) );
		ppwPtrPtr = extension->ppwScriptWinStrPtrsBuffer + ( extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) );

		for ( ulI = 0; ulI < ulNullLinesNum; ulI ++ )
		{
			pwPtr[ ulI ] = 0;
			ppwPtrPtr[ ulI ] = & pwPtr[ ulI ];
		}
	}

	//
	// Copy the Specified String.
	//

	wcscpy( extension->ppwScriptWinStrPtrsBuffer[ ulLineId ], pwLineText );

	// Apply Syntax Coloring.

	ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ ulLineId ] );

	//
	// Update the Pos Variables.
	//

	extension->ulScriptWinBufferPosInBytes += ulBytesToAddToBuff;
	extension->ulScriptWinStrPtrsBufferPosInBytes += ulBytesToAddToStrPtrs;

	// Check whether There are NULL Lines to Remove.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	for ( ulI = ulSize - 1; ulI < 0x80000000; ulI -- )
		if ( * extension->ppwScriptWinStrPtrsBuffer[ ulI ] == 0 )
		{
			extension->ulScriptWinBufferPosInBytes -= sizeof( WORD );
			extension->ulScriptWinStrPtrsBufferPosInBytes -= sizeof( WORD* );
		}
		else
			break;

	// Return.

	return;
}

//=======================================
// DeleteScriptLine Function Definition.
//=======================================

VOID DeleteScriptLine( IN ULONG ulLineId )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulSize, ulSize2, ulDim;
	WORD*					pwPtr;
	WORD*					pwPtr2;
	WORD*					pwPtr3;
	ULONG					ulI;

	// Validate the Parameter.

	ulSize2 = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	if ( ulLineId >= ulSize2 )
		return;

	// Pointer to the Specified Line.

	pwPtr =
		extension->ppwScriptWinStrPtrsBuffer[ ulLineId ];

	// The Number of WORDS the Buffer should be Decreased.

	ulSize =
		WordStringLen( pwPtr ) + 1;

	// Pointers to: first Word after Specified Line, Word where new Block should be Moved.

	pwPtr3 =
		pwPtr + ulSize;
	pwPtr2 =
		pwPtr;

	// Pointer to the Last Line, Dimension of the Block to Move.

	pwPtr =
		extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ];
	ulDim =
		( pwPtr + WordStringLen( pwPtr ) + 1 ) - pwPtr3;

	// Move the Memory.

	if ( ulDim )
	{
		__asm
		{
			mov			ecx, ulDim
			mov			esi, pwPtr3
			mov			edi, pwPtr2

			rep			movsw
		}
	}

	//
	// ### Adjust the Pointers Buffer. ###
	//

	if ( ulDim )
	{
		for ( ulI = ulLineId + 1; ulI < ulSize2; ulI ++ )
			extension->ppwScriptWinStrPtrsBuffer[ ulI - 1 ] = extension->ppwScriptWinStrPtrsBuffer[ ulI ] - ulSize;
	}

	// Update the Pos Vars.

	extension->ulScriptWinBufferPosInBytes -= ulSize * sizeof( WORD );
	extension->ulScriptWinStrPtrsBufferPosInBytes -= sizeof( WORD* );

	// Check whether There are NULL Lines to Remove.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	for ( ulI = ulSize - 1; ulI < 0x80000000; ulI -- )
		if ( * extension->ppwScriptWinStrPtrsBuffer[ ulI ] == 0 )
		{
			extension->ulScriptWinBufferPosInBytes -= sizeof( WORD );
			extension->ulScriptWinStrPtrsBufferPosInBytes -= sizeof( WORD* );
		}
		else
			break;

	// Return.

	return;
}

//====================================================================
//
// ApplySyntaxColoring Function Definition + Related Data Structures.
//
//====================================================================

typedef struct _LANGKEYW
{
	CHAR*				pszKeyword;
	struct _LANGKEYW*	plkNext;

} LANGKEYW, *PLANGKEYW;

#define KEYW( n )					__priv__##n
#define DEFLANGKEYW( name, next )	static LANGKEYW KEYW( name ) = { #name, next };

// // // // begin data //

DEFLANGKEYW( break, NULL )

DEFLANGKEYW( char, NULL )
DEFLANGKEYW( const, & KEYW( char ) )
DEFLANGKEYW( continue, & KEYW( const ) )

DEFLANGKEYW( do, NULL )
DEFLANGKEYW( double, & KEYW( do ) )

DEFLANGKEYW( else, NULL )

DEFLANGKEYW( float, NULL )
DEFLANGKEYW( for, & KEYW( float ) )

DEFLANGKEYW( goto, NULL )

DEFLANGKEYW( if, NULL )
DEFLANGKEYW( int, & KEYW( if ) )

DEFLANGKEYW( long, NULL )

DEFLANGKEYW( macro, NULL )

DEFLANGKEYW( name, NULL )

DEFLANGKEYW( return, NULL )

DEFLANGKEYW( short, NULL )
DEFLANGKEYW( signed, & KEYW( short ) )
DEFLANGKEYW( sizeof, & KEYW( signed ) )
DEFLANGKEYW( static, & KEYW( sizeof ) )
DEFLANGKEYW( struct, & KEYW( static ) )

DEFLANGKEYW( unsigned, NULL )

DEFLANGKEYW( void, NULL )

DEFLANGKEYW( while, NULL )

// // // // end data //

static LANGKEYW*			g_vplkLangKeywordsLOWER[] =
{
/* a */ NULL,
/* b */ & KEYW( break ),
/* c */ & KEYW( continue ),
/* d */ & KEYW( double ),
/* e */ & KEYW( else ),
/* f */ & KEYW( for ),
/* g */ & KEYW( goto ),
/* h */ NULL,
/* i */ & KEYW( int ),
/* j */ NULL,
/* k */ NULL,
/* l */ & KEYW( long ),
/* m */ & KEYW( macro ),
/* n */ & KEYW( name ),
/* o */ NULL,
/* p */ NULL,
/* q */ NULL,
/* r */ & KEYW( return ),
/* s */ & KEYW( struct ),
/* t */ NULL,
/* u */ & KEYW( unsigned ),
/* v */ & KEYW( void ),
/* w */ & KEYW( while ),
/* x */ NULL,
/* y */ NULL,
/* z */ NULL
};

static LANGKEYW*			g_vplkLangKeywordsUPPER[] =
{
/* A */ NULL,
/* B */ NULL,
/* C */ NULL,
/* D */ NULL,
/* E */ NULL,
/* F */ NULL,
/* G */ NULL,
/* H */ NULL,
/* I */ NULL,
/* J */ NULL,
/* K */ NULL,
/* L */ NULL,
/* M */ NULL,
/* N */ NULL,
/* O */ NULL,
/* P */ NULL,
/* Q */ NULL,
/* R */ NULL,
/* S */ NULL,
/* T */ NULL,
/* U */ NULL,
/* V */ NULL,
/* W */ NULL,
/* X */ NULL,
/* Y */ NULL,
/* Z */ NULL
};

#undef KEYW
#undef DEFLANGKEYW

static BOOLEAN CompareCharWordIdentfStrings( IN CHAR* pszCharStr, IN WORD* pszWordStr )
{
	ULONG			ulL = strlen( pszCharStr ), ulI;
	CHAR*			pszCharPTR = pszCharStr;
	WORD*			pszWordPTR = pszWordStr;
	BYTE			bC;

	for ( ulI = 0; ulI < ulL; ulI ++, pszCharPTR ++, pszWordPTR ++ )
		if ( * pszWordPTR == 0 ||
			* pszCharPTR != ( * pszWordPTR & 0xFF ) )
				return FALSE;

	bC = (BYTE) ( * pszWordPTR & 0xFF );
	if (
			( bC >= 'a' && bC <= 'z' ) ||
			( bC >= 'A' && bC <= 'Z' ) ||
			( bC >= '0' && bC <= '9' ) ||
			( bC == '_' )
		)
	{
		return FALSE;
	}

	return TRUE;
}

static LANGKEYW* CheckWhetherKeywordOrNot( IN WORD* pszString )
{
	BYTE		bC = (BYTE) ( * pszString & 0xFF );
	LANGKEYW*	plkPtr = NULL;

	if ( bC >= 'a' && bC <= 'z' )
		plkPtr = g_vplkLangKeywordsLOWER[ bC - 'a' ];
	else if ( bC >= 'A' && bC <= 'Z' )
		plkPtr = g_vplkLangKeywordsUPPER[ bC - 'A' ];

	while( plkPtr )
		if ( CompareCharWordIdentfStrings( plkPtr->pszKeyword, pszString ) )
			return plkPtr;
		else
			plkPtr = plkPtr->plkNext;

	return NULL;
}

static VOID ApplyForeColorToWordString( IN WORD* pszString, IN ULONG ulLength, IN BYTE bForeColor )
{
	ULONG		ulI;
	WORD*		pwPtr = pszString;
	WORD		wForeColorWORD = bForeColor << 8;

	for ( ulI = 0; ulI < ulLength; ulI ++, pwPtr ++ )
		if ( ( * pwPtr & 0xFF ) != ' ' )
			* pwPtr = ( * pwPtr & 0xF0FF ) | wForeColorWORD;
		else
			* pwPtr = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	return;
}

static ULONG CountIdentifierChrsNumInWordString( IN WORD* pwStr )
{
	WORD*		pwPtr = pwStr;
	ULONG		ulL = 0;
	BYTE		bC;

	while( * pwPtr )
	{
		bC = (BYTE) ( * pwPtr & 0xFF );

		if (
				( bC >= 'a' && bC <= 'z' ) ||
				( bC >= 'A' && bC <= 'Z' ) ||
				( bC >= '0' && bC <= '9' ) ||
				( bC == '_' )
			)
		{
			++ ulL;
		}
		else
		{
			break;
		}

		++ pwPtr;
	}

	return ulL;
}

VOID ApplySyntaxColoring( IN OUT WORD* pwStr )
{
	WORD*			pwPtr;
	WORD*			pwPtrEnd;
	BYTE			bC;
	LANGKEYW*		plkKeyW;
	ULONG			ulL;
	ULONG			ulInString = 0;
	BOOLEAN			bChrsFound = FALSE;

	ApplyForeColorToWordString( pwStr, WordStringLen( pwStr ), 0x7 );

	//
	// Apply Syntax Coloring to the Passed String.
	//

	pwPtr = pwStr;
	pwPtrEnd = pwStr + WordStringLen( pwStr );

	while( pwPtr < pwPtrEnd )
	{
		if ( * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
		{
			* pwPtr =
				( * pwPtr & 0xF0FF ) |
					(WORD) ( ulInString == 0 ? 0x0700 : ( MACRO_SCRIPTWIN_SYNTCOLOR_STRING << 8 ) );
		}
		bC = (BYTE) ( * pwPtr & 0xFF );

		if ( bC == '\\' )
		{
			if ( * ( pwPtr + 1 ) )
				ApplyForeColorToWordString( ++ pwPtr, 1, ulInString == 0 ? 0x7 : MACRO_SCRIPTWIN_SYNTCOLOR_STRING );
		}
		else if ( bC == '\'' )
		{
			if ( ulInString == 0 )
			{
				ApplyForeColorToWordString( pwPtr, 1, MACRO_SCRIPTWIN_SYNTCOLOR_STRING );
				ulInString = 1;
			}
			else if ( ulInString == 1 )
			{
				ulInString = 0;
			}
		}
		else if ( bC == '"' )
		{
			if ( ulInString == 0 )
			{
				ApplyForeColorToWordString( pwPtr, 1, MACRO_SCRIPTWIN_SYNTCOLOR_STRING );
				ulInString = 2;
			}
			else if ( ulInString == 2 )
			{
				ulInString = 0;
			}
		}
		else if ( bC == '/' )
		{
			if ( ulInString == 0 &&
				( (*(pwPtr+1)) & 0xFF ) == '/' )
			{
				ApplyForeColorToWordString( pwPtr, WordStringLen( pwPtr ), MACRO_SCRIPTWIN_SYNTCOLOR_COMMENT );
				break;
			}
		}
		else if ( bC == '#' )
		{
			if ( bChrsFound == FALSE )
			{
				ApplyForeColorToWordString( pwPtr, WordStringLen( pwPtr ), MACRO_SCRIPTWIN_SYNTCOLOR_PREPROC );
				break;
			}
		}
		else if ( ulInString == 0 &&
					( ( bC >= 'a' && bC <= 'z' ) ||
					( bC >= 'A' && bC <= 'Z' ) ||
					( bC >= '0' && bC <= '9' ) ||
					( bC == '_' ) )
			)
		{
			if ( plkKeyW = CheckWhetherKeywordOrNot( pwPtr ) )
			{
				ulL = strlen( plkKeyW->pszKeyword );
				ApplyForeColorToWordString( pwPtr, ulL, MACRO_SCRIPTWIN_SYNTCOLOR_KEYWORD );
			}
			else
			{
				ulL = CountIdentifierChrsNumInWordString( pwPtr );
				ApplyForeColorToWordString( pwPtr, ulL, 0x7 );
			}

			pwPtr += ulL - 1;
		}

		if ( bC != ' ' )
			bChrsFound = TRUE;

		pwPtr ++;
	}

	//
	// Return.
	//

	return;
}

//=========================================
// ApplySelAreaToText Function Definition.
//=========================================

static VOID ApplyForeBackColorToWordString( IN WORD* pszString, IN ULONG ulLength, IN BYTE bForeColor, IN BYTE bBackColor )
{
	ULONG		ulI;
	WORD*		pwPtr = pszString;
	WORD		wColorWORD = ( bBackColor << 12 ) | ( bForeColor << 8 );

	for ( ulI = 0; ulI < ulLength; ulI ++, pwPtr ++ )
		if ( * pwPtr )
			* pwPtr = ( * pwPtr & 0x00FF ) | wColorWORD;
		else
			break;

	return;
}

BOOLEAN ApplySelAreaToText( IN BOOLEAN bOn, IN ULONG* pulSelAreaX, IN ULONG* pulSelAreaY, OUT BOOLEAN* pbBlockYInc, OUT ULONG* pulIfYIncCorrectX )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulPos;
	ULONG					vulLen[ 2 ];
	ULONG					ulI;
	WORD*					pwPtr;
	ULONG					ulA, ulB;

	if ( pbBlockYInc )
		* pbBlockYInc = FALSE;
	if ( pulIfYIncCorrectX )
		* pulIfYIncCorrectX = 0;

	//
	// ## Apply the Selection Area to the Script File Text. ##
	//

	// Validation.

	if ( pulSelAreaY[ 1 ] < pulSelAreaY[ 0 ] )
	{
		ulA = pulSelAreaX[ 0 ];
		ulB = pulSelAreaY[ 0 ];
		pulSelAreaX[ 0 ] = pulSelAreaX[ 1 ];
		pulSelAreaY[ 0 ] = pulSelAreaY[ 1 ];
		pulSelAreaX[ 1 ] = ulA;
		pulSelAreaY[ 1 ] = ulB;
	}

	ulPos = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	if ( pulSelAreaY[ 0 ] >= ulPos )
		return FALSE;

	if ( pulSelAreaY[ 1 ] >= ulPos )
	{
		pulSelAreaX[ 1 ] = MACRO_SCRIPTWIN_LINESIZE_IN_CHARS;
		pulSelAreaY[ 1 ] = ulPos - 1;
	}

	if ( pulSelAreaY[ 0 ] == pulSelAreaY[ 1 ] &&
		pulSelAreaX[ 1 ] < pulSelAreaX[ 0 ] )
	{
		ulA = pulSelAreaX[ 0 ];
		pulSelAreaX[ 0 ] = pulSelAreaX[ 1 ];
		pulSelAreaX[ 1 ] = ulA;
	}

	vulLen[ 0 ] = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 0 ] ] );
	vulLen[ 1 ] = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 1 ] ] );

	if ( pulSelAreaX[ 0 ] >= vulLen[ 0 ] )
	{
		if ( pulSelAreaY[ 0 ] == pulSelAreaY[ 1 ] )
			return FALSE;
		else
		{
			if ( vulLen[ 0 ] == 0 )
			{
				pulSelAreaX[ 0 ] = 0;
			}
			else
			{
				pulSelAreaX[ 0 ] = 0;
				pulSelAreaY[ 0 ] ++;

				if ( pbBlockYInc )
					* pbBlockYInc = TRUE;
				if ( pulIfYIncCorrectX )
					* pulIfYIncCorrectX = vulLen[ 0 ];
			}
		}
	}

	if ( pulSelAreaX[ 1 ] >= vulLen[ 1 ] )
		pulSelAreaX[ 1 ] = vulLen[ 1 ] ? vulLen[ 1 ] - 1 : 0;

	// Text Selection.

	if ( pulSelAreaY[ 0 ] == pulSelAreaY[ 1 ] )
	{
		pwPtr = extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 0 ] ] + pulSelAreaX[ 0 ];
		ApplyForeBackColorToWordString( pwPtr, pulSelAreaX[ 1 ] - pulSelAreaX[ 0 ] + 1, bOn ? 0x0 : 0x7, bOn ? 0x7 : 0x0 );
		if ( bOn == FALSE )
			ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 0 ] ] );
	}
	else
	{
		for ( ulI = pulSelAreaY[ 0 ]; ulI <= pulSelAreaY[ 1 ]; ulI ++ )
		{
			if ( ulI == pulSelAreaY[ 0 ] )
			{
				pwPtr = extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 0 ] ] + pulSelAreaX[ 0 ];
				ApplyForeBackColorToWordString( pwPtr, WordStringLen( pwPtr ), bOn ? 0x0 : 0x7, bOn ? 0x7 : 0x0 );
				if ( bOn == FALSE )
					ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 0 ] ] );
			}
			else if ( ulI == pulSelAreaY[ 1 ] )
			{
				ApplyForeBackColorToWordString(
					extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 1 ] ], pulSelAreaX[ 1 ] + 1,
					bOn ? 0x0 : 0x7, bOn ? 0x7 : 0x0 );
				if ( bOn == FALSE )
					ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ pulSelAreaY[ 1 ] ] );
			}
			else
			{
				pwPtr = extension->ppwScriptWinStrPtrsBuffer[ ulI ];
				ApplyForeBackColorToWordString( pwPtr, WordStringLen( pwPtr ), bOn ? 0x0 : 0x7, bOn ? 0x7 : 0x0 );
				if ( bOn == FALSE )
					ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ ulI ] );
			}
		}
	}

	//
	// ## Return. ##
	//

	return TRUE;
}

//===========================================
// CopyScriptTextToClip Function Definition.
//===========================================

BOOLEAN CopyScriptTextToClip( IN ULONG* pulValidatedSelAreaX, IN ULONG* pulValidatedSelAreaY )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulBytesInBuff, ulBytesInStrPtrsBuff;
	ULONG					ulI, ulA;
	WORD*					pwPtr;

	// Calculate the Dimension of the Selected Text.

	ulBytesInBuff = 0;
	ulBytesInStrPtrsBuff = 0;

	if ( pulValidatedSelAreaY[ 0 ] == pulValidatedSelAreaY[ 1 ] )
	{
		ulA =
			WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] ) ? sizeof( WORD ) : 0;
		ulBytesInBuff +=
			( pulValidatedSelAreaX[ 1 ] - pulValidatedSelAreaX[ 0 ] + 1 ) * sizeof( WORD ) + ulA;
		ulBytesInStrPtrsBuff +=
			sizeof( WORD* );
	}
	else
	{
		for ( ulI = pulValidatedSelAreaY[ 0 ]; ulI <= pulValidatedSelAreaY[ 1 ]; ulI ++ )
		{
			if ( ulI == pulValidatedSelAreaY[ 0 ] )
			{
				pwPtr = extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] + pulValidatedSelAreaX[ 0 ];
				ulBytesInBuff += WordStringLen( pwPtr ) * sizeof( WORD ) + sizeof( WORD );
				ulBytesInStrPtrsBuff += sizeof( WORD* );
			}
			else if ( ulI == pulValidatedSelAreaY[ 1 ] )
			{
				ulA =
					WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 1 ] ] ) ? pulValidatedSelAreaX[ 1 ] + 1 : 0;
				ulBytesInBuff += ulA * sizeof( WORD ) + sizeof( WORD );
				ulBytesInStrPtrsBuff += sizeof( WORD* );
			}
			else
			{
				pwPtr = extension->ppwScriptWinStrPtrsBuffer[ ulI ];
				ulBytesInBuff += WordStringLen( pwPtr ) * sizeof( WORD ) + sizeof( WORD );
				ulBytesInStrPtrsBuff += sizeof( WORD* );
			}
		}
	}

	// Check whether There is Enough space in the Clipboard.

	if ( ulBytesInBuff > extension->ulScriptWinClipBufferSizeInBytes ||
		ulBytesInStrPtrsBuff > extension->ulScriptWinClipStrPtrsBufferSizeInBytes )
	{
		//
		// The Clipboard is not Large Enough...
		//

		OutputPrint( FALSE, FALSE, "Clipboard size is insufficient." );
		return FALSE;
	}

	// Copy the Memory in the Clipboard.

	pwPtr = extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] + pulValidatedSelAreaX[ 0 ];
	ulA = ulBytesInBuff - sizeof( WORD );
	memcpy( extension->pwScriptWinClipBuffer, pwPtr, ulA );
	extension->pwScriptWinClipBuffer[ ulA / sizeof( WORD ) ] = 0;

	pwPtr = extension->pwScriptWinClipBuffer;
	ulA = ulBytesInStrPtrsBuff / sizeof( WORD* );

	for ( ulI = 0; ulI < ulA; ulI ++ )
	{
		extension->ppwScriptWinClipStrPtrsBuffer[ ulI ] = pwPtr;
		pwPtr += WordStringLen( pwPtr ) + 1;
	}

	// Save the Buffer Pos Vars.

	extension->ulScriptWinClipBufferPosInBytes = ulBytesInBuff;
	extension->ulScriptWinClipStrPtrsBufferPosInBytes = ulBytesInStrPtrsBuff;

	// Return.

	return TRUE;
}

//======================================
// PasteScriptText Function Definition.
//======================================

VOID PasteScriptText( IN OUT ULONG* pulPasteX, IN OUT ULONG* pulPasteY )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulSize, ulDim, ulSize2, ulSize3, ulCollSize;
	ULONG					ulNullLinesNum, ulNullColsNum;
	BOOLEAN					bEatLastClipNullTerm;
	ULONG					ulBytesToAddToBuff, ulBytesToAddToStrPtrs;
	WORD*					pwPtr;
	WORD*					pwPtr2;
	WORD*					pwPtr3;
	WORD*					pwPtr4;
	WORD**					ppwPtrPtr;
	ULONG					ulX, ulI;
	ULONG					ulEndX, ulEndY;

	// Check whether the Clipboard contains Something...

	if ( extension->ulScriptWinClipBufferPosInBytes == 0 ||
		extension->ulScriptWinClipStrPtrsBufferPosInBytes == 0 )
	{
		// Clipboard Empty !!

		return;
	}

	// Calculate the Number of NULL Lines and Columns.

	ulCollSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	if ( * pulPasteY >= ulCollSize )
	{
		ulNullLinesNum = * pulPasteY - ulCollSize;
		ulNullColsNum = * pulPasteX;
		bEatLastClipNullTerm = FALSE;
	}
	else
	{
		ulNullLinesNum = 0;

		ulDim = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] );
		if ( * pulPasteX > ulDim )
			ulNullColsNum = * pulPasteX - ulDim;
		else
			ulNullColsNum = 0;

		bEatLastClipNullTerm = TRUE;
	}

	//
	// Check whether there is Enough Memory for this Operation.
	//

	ulBytesToAddToBuff =
		( ulNullLinesNum + ulNullColsNum ) * sizeof( WORD ) + extension->ulScriptWinClipBufferPosInBytes
		- ( bEatLastClipNullTerm ? sizeof( WORD ) : 0 );
	ulBytesToAddToStrPtrs =
		ulNullLinesNum * sizeof( WORD* ) + extension->ulScriptWinClipStrPtrsBufferPosInBytes
		- ( bEatLastClipNullTerm ? sizeof( WORD* ) : 0 );

	if ( extension->ulScriptWinBufferPosInBytes + ulBytesToAddToBuff > extension->ulScriptWinBufferSizeInBytes ||
		extension->ulScriptWinStrPtrsBufferPosInBytes + ulBytesToAddToStrPtrs > extension->ulScriptWinStrPtrsBufferSizeInBytes )
	{
		//
		// ### WARNING ### Not sufficient Memory.
		//

		return;
	}

	//
	// Check the Constraints at the Row Length level.
	//

	if ( * pulPasteY >= ulCollSize )
	{
		if ( ulNullColsNum + WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ 0 ] ) >= MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
		{
			//
			// First Pasted Line would be TOO LONG !
			//

			return;
		}
	}
	else
	{
		if ( ulNullColsNum )
		{
			ulSize = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] ) +
				ulNullColsNum + WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ 0 ] );
		}
		else
		{
			if ( bEatLastClipNullTerm && extension->ulScriptWinClipStrPtrsBufferPosInBytes == sizeof( WORD* ) )
			{
				ulSize = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] ) +
					WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ 0 ] );
			}
			else
			{
				ulSize = * pulPasteX +
					WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ 0 ] );
			}
		}

		if ( ulSize >= MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
		{
			//
			// First Pasted Line would be TOO LONG !
			//

			return;
		}

		if ( bEatLastClipNullTerm && ulNullColsNum == 0 && extension->ulScriptWinClipStrPtrsBufferPosInBytes > sizeof( WORD* ) )
		{
			ulSize =
				WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ extension->ulScriptWinClipStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ] )
				+ ( WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] ) - * pulPasteX );

			if ( ulSize >= MACRO_SCRIPTWIN_LINESIZE_IN_CHARS )
			{
				//
				// JOIN: Last Clipboard Pasted Line + First Script Line Fragment TOTAL LENGHT would be TOO LONG !
				//

				return;
			}
		}
	}

	// Calculate the END Paste Coordinates.

	if ( extension->ulScriptWinClipStrPtrsBufferPosInBytes == sizeof( WORD* ) )
	{
		ulEndX =
			* pulPasteX + WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ 0 ] );
	}
	else
	{
		ulEndX =
			WordStringLen( extension->ppwScriptWinClipStrPtrsBuffer[ extension->ulScriptWinClipStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ] );
	}

	ulEndY =
		* pulPasteY + extension->ulScriptWinClipStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1;

	//
	// ## Paste the Clipboard Text: at the End of the Script File or in the Middle of It. ##
	//

	// Make room for the Text and Copy it.

	if ( * pulPasteY < ulCollSize ) // # Adding the Text in the Middle of the Script File. #
	{
		//
		// # Make Room. #
		//

		// The Number of WORDS the Buffer should be Increased.

		ulSize =
			ulNullColsNum + extension->ulScriptWinClipBufferPosInBytes / sizeof( WORD ) - 1 /* Last Null Term is EATEN. */;

		// Pointer to the Line Chr that will be the Next to the First Inserted Line.

		if ( ulNullColsNum ) // the cursor is over the end of the line.
			ulX = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] );
		else
			ulX = * pulPasteX;

		pwPtr4 =
			extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] + ulX;

		// Pointers to: first Word to Move, Word where new Block should be Moved.

		pwPtr3 =
			pwPtr4;
		pwPtr2 =
			pwPtr3 + ulSize;

		// Pointer to the Last Line, Dimension of the Block to Move.

		pwPtr =
			extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ];
		ulDim =
			( pwPtr + WordStringLen( pwPtr ) + 1 ) - pwPtr3;

		// Adjust the Pointers and Move the Memory.

		if ( ulDim )
		{
			pwPtr3 += ulDim - 1;
			pwPtr2 += ulDim - 1;

			__asm
			{
				mov			ecx, ulDim
				mov			esi, pwPtr3
				mov			edi, pwPtr2

				std
				rep			movsw
				cld
			}
		}

		//
		// ### Adjust the Pointers Buffer. ###
		//

		if ( ulDim )
		{
			ulSize2 = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );
			ulSize3 = extension->ulScriptWinClipStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 /* Last NULL Term EATEN! */;

			for ( ulI = ulSize2 - 1; ulI > * pulPasteY && ulI < 0x80000000; ulI -- )
				extension->ppwScriptWinStrPtrsBuffer[ ulI + ulSize3 ] = extension->ppwScriptWinStrPtrsBuffer[ ulI ] + ulSize;

			for ( ulI = 1; ulI <= ulSize3; ulI ++ ) // WARNING: The first line RETAINS its Original Pointer !!
				extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY + ulI ] =
					pwPtr4 + ulNullColsNum + ( extension->ppwScriptWinClipStrPtrsBuffer[ ulI ] - extension->pwScriptWinClipBuffer );
		}
	}
	else // # Adding the Text at the END of the Script File.
	{
		// Add the Lines at the End of the File.

		pwPtr = extension->pwScriptWinBuffer + ( extension->ulScriptWinBufferPosInBytes / sizeof( WORD ) );
		ppwPtrPtr = extension->ppwScriptWinStrPtrsBuffer + ( extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) );

		for ( ulI = 0; ulI < ulNullLinesNum; ulI ++, pwPtr ++, ppwPtrPtr ++ )
		{
			* pwPtr = 0;
			* ppwPtrPtr = pwPtr;
		}

		ulSize = extension->ulScriptWinClipStrPtrsBufferPosInBytes / sizeof( WORD* );
		pwPtr2 = pwPtr;

		for ( ulI = 0; ulI < ulSize; ulI ++, ppwPtrPtr ++ )
		{
			if ( ulI )
				pwPtr = pwPtr2 + ulNullColsNum + ( extension->ppwScriptWinClipStrPtrsBuffer[ ulI ] - extension->pwScriptWinClipBuffer );

			* pwPtr = 0;
			* ppwPtrPtr = pwPtr;
		}
	}

	//
	// Copy the Clipboard Buffer to the Script Buffer.
	//

	if ( ulNullColsNum ) // the cursor is over the end of the line.
		ulX = WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] );
	else
		ulX = * pulPasteX;

	pwPtr =
		extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] + ulX;

	for ( ulI = 0; ulI < ulNullColsNum; ulI ++, pwPtr ++ )
		* pwPtr = CLRTXT2VIDBUFWORD( 7, 0, ' ' );

	memcpy( pwPtr, extension->pwScriptWinClipBuffer,
		extension->ulScriptWinClipBufferPosInBytes - ( bEatLastClipNullTerm ? sizeof( WORD ) : 0 ) );

	// Apply Syntax Coloring to the First and Last Pasted Strings.

	ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ * pulPasteY ] );

	if ( * pulPasteY != ulEndY )
		ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ ulEndY ] );

	//
	// Update the Pos Variables.
	//

	extension->ulScriptWinBufferPosInBytes += ulBytesToAddToBuff;
	extension->ulScriptWinStrPtrsBufferPosInBytes += ulBytesToAddToStrPtrs;

	// Check whether There are NULL Lines to Remove.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	for ( ulI = ulSize - 1; ulI < 0x80000000; ulI -- )
		if ( * extension->ppwScriptWinStrPtrsBuffer[ ulI ] == 0 )
		{
			extension->ulScriptWinBufferPosInBytes -= sizeof( WORD );
			extension->ulScriptWinStrPtrsBufferPosInBytes -= sizeof( WORD* );
		}
		else
			break;

	// Return.

	* pulPasteX = ulEndX;
	* pulPasteY = ulEndY;

	return;
}

//=======================================
// DeleteScriptText Function Definition.
//=======================================

VOID DeleteScriptText( IN ULONG* pulValidatedSelAreaX, IN ULONG* pulValidatedSelAreaY )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulBytesInBuff, ulBytesInStrPtrsBuff;
	WORD*					pwPtr;
	WORD*					pwPtr2;
	WORD*					pwPtr3;
	ULONG					ulI, ulDim, ulSize, ulSize2, ulA;
	ULONG					ulSelAreaX0_Backup = pulValidatedSelAreaX[ 0 ];
	BOOLEAN					bXWasDecd = FALSE;

	//
	// Check whether we have to Trim the Text at the Left of the Block.
	//

	while( TRUE )
	{
		pwPtr = extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] + pulValidatedSelAreaX[ 0 ];
		if ( * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) ||
			pulValidatedSelAreaX[ 0 ] == 0 )
		{
			if ( bXWasDecd && * pwPtr != CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
				pulValidatedSelAreaX[ 0 ] ++;
			break;
		}
		else
		{
			pulValidatedSelAreaX[ 0 ] --;
			bXWasDecd = TRUE;
		}
	}

	// Calculate the Dimensions of the Text to delete.

	ulBytesInBuff = 0;
	ulBytesInStrPtrsBuff = 0;

	if ( pulValidatedSelAreaY[ 0 ] == pulValidatedSelAreaY[ 1 ] )
	{
		ulBytesInBuff +=
			( pulValidatedSelAreaX[ 1 ] - pulValidatedSelAreaX[ 0 ] + 1 ) * sizeof( WORD );
		ulA =
			WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] ) ? 0 : sizeof( WORD* );
		ulBytesInStrPtrsBuff +=
			ulA;
	}
	else
	{
		for ( ulI = pulValidatedSelAreaY[ 0 ]; ulI <= pulValidatedSelAreaY[ 1 ]; ulI ++ )
		{
			if ( ulI == pulValidatedSelAreaY[ 0 ] )
			{
				pwPtr = extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] + pulValidatedSelAreaX[ 0 ];
				ulBytesInBuff += WordStringLen( pwPtr ) * sizeof( WORD ) + sizeof( WORD );
				ulBytesInStrPtrsBuff += sizeof( WORD* );
			}
			else if ( ulI == pulValidatedSelAreaY[ 1 ] )
			{
				ulA =
					WordStringLen( extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 1 ] ] ) ? pulValidatedSelAreaX[ 1 ] + 1 : 0;
				ulBytesInBuff += ulA * sizeof( WORD );
			}
			else
			{
				pwPtr = extension->ppwScriptWinStrPtrsBuffer[ ulI ];
				ulBytesInBuff += WordStringLen( pwPtr ) * sizeof( WORD ) + sizeof( WORD );
				ulBytesInStrPtrsBuff += sizeof( WORD* );
			}
		}
	}

	// Pointers to: Word where new Block should be Moved, first Word after Specified Text Block.

	pwPtr2 =
		extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] + pulValidatedSelAreaX[ 0 ];
	pwPtr3 =
		pwPtr2 + ulBytesInBuff / sizeof( WORD );

	// Pointer to the Last Line, Dimension of the Block to Move.

	pwPtr =
		extension->ppwScriptWinStrPtrsBuffer[ extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) - 1 ];
	ulDim =
		( pwPtr + WordStringLen( pwPtr ) + 1 ) - pwPtr3;

	// Move the Memory.

	if ( ulDim )
	{
		__asm
		{
			mov			ecx, ulDim
			mov			esi, pwPtr3
			mov			edi, pwPtr2

			rep			movsw
		}
	}

	//
	// ### Adjust the Pointers Buffer. ###
	//

	if ( ulDim )
	{
		ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );
		ulSize2 = ulBytesInStrPtrsBuff / sizeof( WORD* );

		for ( ulI = pulValidatedSelAreaY[ 0 ] + ulSize2 + 1; ulI < ulSize; ulI ++ )
			extension->ppwScriptWinStrPtrsBuffer[ ulI - ulSize2 ] = extension->ppwScriptWinStrPtrsBuffer[ ulI ] - ulBytesInBuff / sizeof( WORD );
	}

	// Update the Pos Vars.

	extension->ulScriptWinBufferPosInBytes -= ulBytesInBuff;
	extension->ulScriptWinStrPtrsBufferPosInBytes -= ulBytesInStrPtrsBuff;

	// Syntax Coloring Update.

	ApplySyntaxColoring( extension->ppwScriptWinStrPtrsBuffer[ pulValidatedSelAreaY[ 0 ] ] );

	// Check whether There are NULL Lines to Remove.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	for ( ulI = ulSize - 1; ulI < 0x80000000; ulI -- )
		if ( * extension->ppwScriptWinStrPtrsBuffer[ ulI ] == 0 )
		{
			extension->ulScriptWinBufferPosInBytes -= sizeof( WORD );
			extension->ulScriptWinStrPtrsBufferPosInBytes -= sizeof( WORD* );
		}
		else
			break;

	// Return.

	pulValidatedSelAreaX[ 0 ] = ulSelAreaX0_Backup;
	return;
}

//============================================
// SearchForFunctionName Function Definition.
//============================================

BOOLEAN SearchForFunctionName( IN ULONG ulStartY, IN ULONG ulOutputBufferSize, OUT CHAR* pszOutputBuffer )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	ULONG					ulI, ulSize;
	WORD*					pwPtr;
	WORD					wW;
	BYTE					bC;
	CHAR*					pszPtr = pszOutputBuffer;
	CHAR*					pszPtrEnd = pszOutputBuffer + ulOutputBufferSize - 1;

	* pszOutputBuffer = 0;

	// Check whether the Start Y param is OK.

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );

	if ( ulSize == 0 )
	{
		return FALSE;
	}
	else if ( ulStartY >= ulSize )
	{
		ulStartY = ulSize - 1;
	}

	//
	// Search for the "name" Keyword.
	//

	for ( ulI = ulStartY; ulI < 0x80000000; ulI -- )
	{
		pwPtr = extension->ppwScriptWinStrPtrsBuffer[ ulI ];

		// Iterate through the Line String Characters.

		while( TRUE )
		{
			wW = * pwPtr ++;
			if ( wW == 0 )
			{
				break;
			}
			else if ( wW == CLRTXT2VIDBUFWORD( 3, 0, 'n' ) )
			{
				if ( * pwPtr == CLRTXT2VIDBUFWORD( 3, 0, 'a' ) &&
					* ( pwPtr + 1 ) == CLRTXT2VIDBUFWORD( 3, 0, 'm' ) &&
					* ( pwPtr + 2 ) == CLRTXT2VIDBUFWORD( 3, 0, 'e' ) &&
					* ( pwPtr + 3 ) == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
				{
					pwPtr += 4;

					// Get rid of the Spaces.

					while( * pwPtr == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
						pwPtr ++;

					if ( * pwPtr == 0 )
						continue;

					// Isolate the Name of the Function.

					while( TRUE )
					{
						bC = (BYTE) ( *pwPtr++ & 0xFF );
						if (
								( bC >= 'a' && bC <= 'z' ) ||
								( bC >= 'A' && bC <= 'Z' ) ||
								( bC >= '0' && bC <= '9' ) ||
								( bC == '_' )
							)
						{
							* pszPtr ++ = (CHAR) bC;
							if ( pszPtr == pszPtrEnd )
							{
								break;
							}
						}
						else
						{
							break;
						}
					}

					* pszPtr = 0;

					// Return Success.

					return TRUE;
				}
			}
		}
	}

	// Return.

	return FALSE;
}

//============================================
// FillCompilationBuffer Function Definition.
//============================================

BOOLEAN FillCompilationBuffer( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	CHAR*					pszPtr = extension->pszScriptWinCompilationBuffer;
	CHAR*					pszPtrEnd =
		extension->pszScriptWinCompilationBuffer + extension->ulScriptWinCompilationBufferSizeInBytes - 1;
	ULONG					ulI, ulSize, ulLen;
	WORD*					pwPtr;

	//
	// Fill the Compilation Memory.
	//

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );
	for ( ulI = 0; ulI < ulSize; ulI ++ )
	{
		pwPtr = extension->ppwScriptWinStrPtrsBuffer[ ulI ];

		// Check the Memory Constraints.

		ulLen = WordStringLen( pwPtr );
		if ( pszPtr + ulLen + 2 /* "\x0D\x0A" */ + 1 /* NULL CHR */ > pszPtrEnd )
			return FALSE;

		// Copy this Line.

		while( * pwPtr )
			* pszPtr ++ = ( CHAR ) ( ( * pwPtr ++ ) & 0xFF );

		* pszPtr ++ = 0xD;
		* pszPtr ++ = 0xA;
	}

	* pszPtr ++ = 0x0;

	//
	// Return.
	//

	return TRUE;
}

//===============================================
// InitializeScriptFileText Function Definition.
//===============================================

VOID InitializeScriptFileText( VOID )
{
	//
	// Add the Lines to the Script File.
	//

	AddScriptLine( 0, L"//" );
	AddScriptLine( 1, L"// BugChecker Script File." );
	AddScriptLine( 2, L"//  This Script File controls the BugChecker behaviour in such events as Breakpoint hits etc." );
	AddScriptLine( 3, L"//  Also, all the commands entered in the BugChecker Output Window are linked against it, so you" );
	AddScriptLine( 4, L"//   can specify and execute complex actions that you can define here as macros (\"macro\" keyword)." );
	AddScriptLine( 5, L"//  Type \"HELP\" in the Output Window for further details about the provided API." );
	AddScriptLine( 6, L"//" );

	AddScriptLine( 8, L"// !! DON'T REMOVE THIS !!" );
	AddScriptLine( 9, L"#include <bugchk_priv.h>" );
	AddScriptLine( 10, L"// !! !! !! !! !! !! !! !!" );

	//
	// Return.
	//

	return;
}

//=============================================
// CollectMacroPrototypes Function Definition.
//=============================================

BOOLEAN CollectMacroPrototypes( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	CHAR*					pszPtr = extension->pszScriptWinCompilationBuffer;
	CHAR*					pszPtrEnd =
		extension->pszScriptWinCompilationBuffer + extension->ulScriptWinCompilationBufferSizeInBytes - 1;
	ULONG					ulSize, ulI, ulLen;
	WORD					wW;
	WORD*					pwPtr;
	WORD*					pwPtrLine;

	//
	// Add the MiniC Prologue.
	//

	ulLen = strlen( MACRO_MINICPROLOGUE );

	strcpy( pszPtr, MACRO_MINICPROLOGUE );
	pszPtr += ulLen;

	//
	// Fill the Compilation Memory with the Macro Prototypes in the Script File.
	//

	ulSize = extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* );
	for ( ulI = 0; ulI < ulSize; ulI ++ )
	{
		pwPtrLine =
			pwPtr =
				extension->ppwScriptWinStrPtrsBuffer[ ulI ];

		// Check whether this line contains the Macro keyword.

		while( TRUE )
		{
			wW = * pwPtr ++;
			if ( wW == 0 )
			{
				break;
			}
			else if ( wW == CLRTXT2VIDBUFWORD( 3, 0, 'm' ) )
			{
				if ( * pwPtr == CLRTXT2VIDBUFWORD( 3, 0, 'a' ) &&
					* ( pwPtr + 1 ) == CLRTXT2VIDBUFWORD( 3, 0, 'c' ) &&
					* ( pwPtr + 2 ) == CLRTXT2VIDBUFWORD( 3, 0, 'r' ) &&
					* ( pwPtr + 3 ) == CLRTXT2VIDBUFWORD( 3, 0, 'o' ) &&
					* ( pwPtr + 4 ) == CLRTXT2VIDBUFWORD( 7, 0, ' ' ) )
				{
					// Check the Memory Constraints.

					ulLen = WordStringLen( pwPtrLine );
					if ( pszPtr + ulLen + 1 /* ';' */ + 2 /* "\x0D\x0A" */ + 1 /* NULL CHR */ > pszPtrEnd )
						return FALSE;

					// Copy this Line.

					while( * pwPtrLine )
						if ( * pwPtrLine != CLRTXT2VIDBUFWORD( 2, 0, '/' ) )
							* pszPtr ++ = ( CHAR ) ( ( * pwPtrLine ++ ) & 0xFF );
						else
							break;

					* pszPtr ++ = ';';

					* pszPtr ++ = 0xD;
					* pszPtr ++ = 0xA;

					// Exit.

					break;
				}
			}
		}
	}

	* pszPtr ++ = 0x0;

	//
	// Return.
	//

	return TRUE;
}

//========================================================
// CheckIfMacrosAreSyntaxedCorrectly Function Definition.
//========================================================

BOOLEAN CheckIfMacrosAreSyntaxedCorrectly( IN CHAR* pszInputText )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	CHAR*					pszPtr = extension->pszScriptWinCompilationBuffer;
	CHAR*					pszPtr2;
	CHAR*					pszPtr2End;
	CHAR*					pszMatch;
	CHAR					szIdentifier[ 128 ];
	ULONG					ulLen, ulSize, ulSize2;
	BOOLEAN					bContinue;
	CHAR					szString[ 16 ];
	CHAR*					pszMovePtr;
	ULONG					ulMoveSize, ulI;
	BOOLEAN					bReturnValue = FALSE;

	//
	// Search for all the Occurrences of the Word "macro".
	//

	while( TRUE )
	{
		pszPtr = strstr( pszPtr, " macro " );
		if ( pszPtr == NULL )
		{
			break;
		}
		else
		{
			//
			// Isolate the Identifier of the Macro Function.
			//

			pszPtr += 7;

			while( * pszPtr == ' ' )
				pszPtr ++;

			if ( * pszPtr == 'n' &&
				* ( pszPtr + 1 ) == 'a' &&
				* ( pszPtr + 2 ) == 'm' &&
				* ( pszPtr + 3 ) == 'e' &&
				* ( pszPtr + 4 ) == ' ' )
			{
				pszPtr += 5;

				while( * pszPtr == ' ' )
					pszPtr ++;
			}

			pszPtr2 = szIdentifier;
			pszPtr2End = szIdentifier + sizeof( szIdentifier ) - 1;

			while(	pszPtr2 < pszPtr2End &&
					( ( * pszPtr >= 'a' && * pszPtr <= 'z' ) ||
					( * pszPtr >= 'A' && * pszPtr <= 'Z' ) ||
					( * pszPtr >= '0' && * pszPtr <= '9' ) ||
					( * pszPtr == '_' ) )
				)
			{
				* pszPtr2 ++ = * pszPtr ++;
			}

			* pszPtr2 ++ = 0;

			//
			// Search for the Identifier in the Passed Text.
			//

			ulLen = strlen( szIdentifier );
			if ( ulLen )
			{
				pszMatch = pszInputText;

				while( TRUE )
				{
					pszMatch = strstr( pszMatch, szIdentifier );
					if ( pszMatch == NULL )
					{
						break;
					}
					else
					{
						//
						// Make sure that the Identifier is NOT part of a Bigger One.
						//

						bContinue = TRUE;

						pszPtr2 = pszMatch - 1; // first before.
						pszMatch += ulLen; // first next.

						if ( pszPtr2 >= pszInputText )
						{
							if ( ( * pszPtr2 >= 'a' && * pszPtr2 <= 'z' ) ||
								( * pszPtr2 >= 'A' && * pszPtr2 <= 'Z' ) ||
								( * pszPtr2 >= '0' && * pszPtr2 <= '9' ) ||
								( * pszPtr2 == '_' ) )
							{
								bContinue = FALSE;
							}
						}

						if ( bContinue )
						{
							if ( ( * pszMatch >= 'a' && * pszMatch <= 'z' ) ||
								( * pszMatch >= 'A' && * pszMatch <= 'Z' ) ||
								( * pszMatch >= '0' && * pszMatch <= '9' ) ||
								( * pszMatch == '_' ) )
							{
								bContinue = FALSE;
							}
						}

						if ( bContinue )
						{
							//
							// Return Success when Exiting.
							//

							bReturnValue = TRUE;

							//
							// Check whether the Parentheses are Present.
							//

							strcpy( szString, "" );

							while( * pszMatch == ' ' )
								pszMatch ++;

							if ( * pszMatch != '(' )
							{
								strcpy( szString, " (0)" );
							}
							else
							{
								pszMatch ++;

								while( * pszMatch == ' ' )
									pszMatch ++;

								if ( * pszMatch == ')' )
								{
									strcpy( szString, "0" );
								}
							}

							//
							// Insert the Specified Text, if Required.
							//

							if ( ulSize = strlen( szString ) )
							{
								// Make the Buffer GROW.

								ulSize2 = strlen( pszInputText );
								pszMovePtr = pszInputText + ulSize2;
								ulMoveSize = pszMovePtr - pszMatch + 1 /* NULL Chr. */;

								for ( ulI = 0; ulI < ulMoveSize; ulI ++, pszMovePtr -- )
									* ( pszMovePtr + ulSize ) = * pszMovePtr;

								// Copy the New Text.

								memcpy( pszMatch, szString, ulSize );
							}
						}
					}
				}
			}
		}
	}

	//
	// Return.
	//

	return bReturnValue;
}

//=============================================
// EvalExprCommandHandler Function Definition.
//=============================================

VOID EvalExprCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	BOOLEAN		bEvalRes;
	DWORD		dwRes;
	CHAR		szBin[ 128 ];

	//
	// Resolve the Expression and Print the Result.
	//

	bEvalRes = FALSE;

	dwRes = EvaluateExpression_DWORD(
		pszParams,
		& bEvalRes );

	if ( bEvalRes )
		OutputPrint( FALSE, FALSE, "result: 0x%X. (decimal: %i, binary: %s)",
			dwRes, dwRes, ultobinstr( szBin, dwRes ) );

	//
	// Return.
	//

	return;
}

//==================================================
// EvalExprFloatCommandHandler Function Definition.
//==================================================

VOID EvalExprFloatCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	BOOLEAN		bEvalRes;
	double		dRes;
	CHAR		szBuffer[ 128 ];

	//
	// Resolve the Expression and Print the Result.
	//

	bEvalRes = FALSE;

	dRes = EvaluateExpression_DOUBLE(
		pszParams,
		& bEvalRes );

	if ( bEvalRes )
	{
		* szBuffer = 0;
		cftog( dRes, szBuffer, 5, FALSE );

		OutputPrint( FALSE, FALSE, "result: %s.",
			szBuffer );
	}

	//
	// Return.
	//

	return;
}

//==========================================
// BpIntCommandHandler Function Definition.
//==========================================

VOID BpIntCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	ULONG					ulVectorNum;
	BOOLEAN					bEvalRes;
	ULONG					ulI;
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisSysInterrStatus = & extension->sisSysInterrStatus;
	BREAKPOINT*				pbSlot = NULL;

	//
	// Evaluate the Interrupt Vector expression and Validate the Parameters.
	//

	ulVectorNum = EvaluateExpression_DWORD(
		pszParams, & bEvalRes );

	if ( bEvalRes == FALSE )
	{
		return;
	}
	else if ( ulVectorNum >= 0x100 )
	{
		OutputPrint( FALSE, FALSE, "Interrupt number expr must be between 0-ff." );
		return;
	}

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		if ( psisSysInterrStatus->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_UNUSED )
		{
			if ( pbSlot == NULL )
				pbSlot = & psisSysInterrStatus->vbpBreakpoints[ ulI ];
		}
		else if ( psisSysInterrStatus->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_INTERRUPT &&
			psisSysInterrStatus->vbpBreakpoints[ ulI ].dwAddress == ulVectorNum )
		{
			OutputPrint( FALSE, FALSE, "Duplicate breakpoint." );
			return;
		}

	if ( pbSlot == NULL )
	{
		OutputPrint( FALSE, FALSE, "Too many breakpoints." );
		return;
	}

	//
	// Install the Breakpoint.
	//

	memset( pbSlot, 0, sizeof( BREAKPOINT ) );

	pbSlot->ulType = MACRO_BREAKPOINTTYPE_INTERRUPT;
	pbSlot->bInstalled = FALSE;
	pbSlot->dwAddress = ulVectorNum;

	memset( & pbSlot->vbPrevCodeBytes, 0xCC, MACRO_MAX_NUM_OF_PROCESSORS );

	//
	// Return.
	//

	return;
}

//=========================================
// BpIoCommandHandler Function Definition.
//=========================================

#define MACRO_BPIOCMDOPT_INDEX_R			0
#define MACRO_BPIOCMDOPT_INDEX_W			1
#define MACRO_BPIOCMDOPT_INDEX_L			2
#define MACRO_BPIOCMDOPT_INDEX_DR			3
#define MACRO_BPIOCMDOPT_INDEX_PORT			4

static COMMAND_OPTION			g_vcoBpIoCommandOptions[] =
{
	MACRO_CMDOPT( "/R" ),
	MACRO_CMDOPT( "/W" ),
	MACRO_CMDOPT( "/L" ),
	MACRO_CMDOPT( "/DR" ),
	MACRO_CMDOPT_END
};

VOID BpIoCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	ULONG					ulI;
	BOOLEAN					vbDebugRegsTaken[ 4 ] = { FALSE, FALSE, FALSE, FALSE };
	ULONG					ulDebugReg = 0xFFFFFFFF;
	DWORD					dwPort;
	BOOLEAN					bEvalRes;
	CHAR*					pszPtr;
	ULONG					ulLength;
	BREAKPOINT*				pbSlot = NULL;
	ULONG					ulRW;

	//
	// ### Parse the Parameters. ###
	//

	// Parse.

	if ( ParseCommandOptions( pszParams, g_vcoBpIoCommandOptions ) == FALSE )
		return;

	// Fill the Debug Regs Taken Vector.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		if ( psisInts->vbpBreakpoints[ ulI ].ulType != MACRO_BREAKPOINTTYPE_UNUSED &&
			psisInts->vbpBreakpoints[ ulI ].bIsUsingDebugRegisters )
		{
			vbDebugRegsTaken[ psisInts->vbpBreakpoints[ ulI ].ulDebugRegisterNum ] = TRUE;
		}

	// Get the Next Free Debug Register, if Required, or Evaluate the Provided Number.

	if (
		( g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_DR ].pszParam == NULL ||
		strlen( g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_DR ].pszParam ) == 0 )
		)
	{
		// Get the Register Num.

		for ( ulI = 3; ulI < 0x80000000; ulI -- )
			if ( vbDebugRegsTaken[ ulI ] == FALSE )
			{
				ulDebugReg = ulI;
				break;
			}

		if ( ulDebugReg == 0xFFFFFFFF )
		{
			OutputPrint( FALSE, FALSE, "No debug register available." );
			return;
		}
	}
	else
	{
		pszPtr = g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_DR ].pszParam;

		if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "0" ) == 0 )
		{
			ulDebugReg = 0;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "1" ) == 0 )
		{
			ulDebugReg = 1;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "2" ) == 0 )
		{
			ulDebugReg = 2;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "3" ) == 0 )
		{
			ulDebugReg = 3;
		}
		else
		{
			OutputPrint( FALSE, FALSE, "Debug register can be 0, 1, 2 or 3." );
			return;
		}

		if ( vbDebugRegsTaken[ ulDebugReg ] )
		{
			OutputPrint( FALSE, FALSE, "Debug register already in use." );
			return;
		}
	}

	// Get the Size Parameter.

	pszPtr = g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_L ].pszParam;

	if ( pszPtr == NULL )
	{
		ulLength = 1;
	}
	else
	{
		if ( strlen( pszPtr ) == 0 )
		{
			OutputPrint( FALSE, FALSE, "Specify the length of the breakpoint." );
			return;
		}
		else
		{
			if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "1" ) == 0 )
			{
				ulLength = 1;
			}
			else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "2" ) == 0 )
			{
				ulLength = 2;
			}
			else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "4" ) == 0 )
			{
				ulLength = 4;
			}
			else
			{
				OutputPrint( FALSE, FALSE, "Length of breakpoint can be 1, 2 or 4." );
				return;
			}
		}
	}

	// Get the RW Parameter.

	ulRW = 0;

	pszPtr = g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_R ].pszParam;
	if ( pszPtr )
	{
		if ( strlen( pszPtr ) )
		{
			if ( * pszPtr == 'W' || * pszPtr == 'w' )
			{
				ulRW |= ( 1 | 2 );
			}
			else
			{
				OutputPrint( FALSE, FALSE, "Syntax error." );
				return;
			}
		}
		else
		{
			ulRW |= 1;
		}
	}

	pszPtr = g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_W ].pszParam;
	if ( pszPtr )
	{
		if ( strlen( pszPtr ) )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
			return;
		}
		else
		{
			ulRW |= 2;
		}
	}

	if ( ulRW == 0 )
		ulRW |= ( 1 | 2 );

	// The Port Param has to be Specified.

	if ( g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_PORT ].pszParam == NULL ||
		strlen( g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_PORT ].pszParam ) == 0 )
	{
		OutputPrint( FALSE, FALSE, "Port must be specified." );
		return;
	}

	// Evaluate the Port Parameter.

	dwPort = EvaluateExpression_DWORD(
		g_vcoBpIoCommandOptions[ MACRO_BPIOCMDOPT_INDEX_PORT ].pszParam, & bEvalRes );

	if ( bEvalRes == FALSE )
		return;

	// Check the Port Parameter alignment and limits.

	if ( dwPort > 0xFFFF )
	{
		OutputPrint( FALSE, FALSE, "Port value must be between 0-FFFF." );
		return;
	}
	else if ( dwPort % ulLength )
	{
		OutputPrint( FALSE, FALSE, "Port value must be aligned to %i (length).", ulLength );
		return;
	}

	// Search for a Free Slot.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		if ( psisInts->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_UNUSED )
		{
			if ( pbSlot == NULL )
				pbSlot = & psisInts->vbpBreakpoints[ ulI ];
		}
		else if (
			psisInts->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_IO &&
			AreMemoryRangesConflicting(
				psisInts->vbpBreakpoints[ ulI ].dwAddress, psisInts->vbpBreakpoints[ ulI ].ulDebugRegisterCondLen,
				dwPort, ulLength )
			)
		{
			OutputPrint( FALSE, FALSE, "Duplicate breakpoint." );
			return;
		}

	if ( pbSlot == NULL )
	{
		OutputPrint( FALSE, FALSE, "Too many breakpoints." );
		return;
	}

	//
	// Install the Breakpoint.
	//

	memset( pbSlot, 0, sizeof( BREAKPOINT ) );

	pbSlot->ulType = MACRO_BREAKPOINTTYPE_IO;
	pbSlot->bInstalled = FALSE;
	pbSlot->bIsContextCompatible = TRUE;

	pbSlot->bIsUsingDebugRegisters = TRUE;
	pbSlot->ulDebugRegisterNum = ulDebugReg;

	pbSlot->dwAddress = dwPort;
	pbSlot->ulDebugRegisterCondLen = ulLength;
	pbSlot->ulDebugRegisterCondRW = ulRW;

	memset( & pbSlot->vbPrevCodeBytes, 0xCC, MACRO_MAX_NUM_OF_PROCESSORS );

	//
	// Return.
	//

	return;
}

//===========================================
// BpMxxxCommandHandler Function Definition.
//===========================================

#define MACRO_BPMXXXCMDOPT_INDEX_W			0
#define MACRO_BPMXXXCMDOPT_INDEX_RW			1
#define MACRO_BPMXXXCMDOPT_INDEX_G			2
#define MACRO_BPMXXXCMDOPT_INDEX_DR			3
#define MACRO_BPMXXXCMDOPT_INDEX_ADDRESS	4

static COMMAND_OPTION			g_vcoBpMxxxCommandOptions[] =
{
	MACRO_CMDOPT( "/W" ),
	MACRO_CMDOPT( "/RW" ),
	MACRO_CMDOPT( "/G" ),
	MACRO_CMDOPT( "/DR" ),
	MACRO_CMDOPT_END
};

static VOID BpMxxxCommandHandler( IN ULONG ulLength, IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	ULONG					ulI;
	BOOLEAN					vbDebugRegsTaken[ 4 ] = { FALSE, FALSE, FALSE, FALSE };
	ULONG					ulDebugReg = 0xFFFFFFFF;
	DWORD					dwAddress;
	BOOLEAN					bGlobal = FALSE;
	CHAR					szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
	CHAR*					pszCurrentProcessName;
	CHAR*					pszDot;
	BOOLEAN					bEvalRes;
	CHAR*					pszPtr;
	BREAKPOINT*				pbSlot = NULL;
	ULONG					ulRW;

	//
	// ### Parse the Parameters. ###
	//

	// Parse.

	if ( ParseCommandOptions( pszParams, g_vcoBpMxxxCommandOptions ) == FALSE )
		return;

	// Fill the Debug Regs Taken Vector.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		if ( psisInts->vbpBreakpoints[ ulI ].ulType != MACRO_BREAKPOINTTYPE_UNUSED &&
			psisInts->vbpBreakpoints[ ulI ].bIsUsingDebugRegisters )
		{
			vbDebugRegsTaken[ psisInts->vbpBreakpoints[ ulI ].ulDebugRegisterNum ] = TRUE;
		}

	// Get the Next Free Debug Register, if Required, or Evaluate the Provided Number.

	if (
		( g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_DR ].pszParam == NULL ||
		strlen( g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_DR ].pszParam ) == 0 )
		)
	{
		// Get the Register Num.

		for ( ulI = 3; ulI < 0x80000000; ulI -- )
			if ( vbDebugRegsTaken[ ulI ] == FALSE )
			{
				ulDebugReg = ulI;
				break;
			}

		if ( ulDebugReg == 0xFFFFFFFF )
		{
			OutputPrint( FALSE, FALSE, "No debug register available." );
			return;
		}
	}
	else
	{
		pszPtr = g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_DR ].pszParam;

		if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "0" ) == 0 )
		{
			ulDebugReg = 0;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "1" ) == 0 )
		{
			ulDebugReg = 1;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "2" ) == 0 )
		{
			ulDebugReg = 2;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pszPtr, "3" ) == 0 )
		{
			ulDebugReg = 3;
		}
		else
		{
			OutputPrint( FALSE, FALSE, "Debug register can be 0, 1, 2 or 3." );
			return;
		}

		if ( vbDebugRegsTaken[ ulDebugReg ] )
		{
			OutputPrint( FALSE, FALSE, "Debug register already in use." );
			return;
		}
	}

	// Get the RW Parameter.

	ulRW = 0;

	pszPtr = g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_W ].pszParam;
	if ( pszPtr )
	{
		if ( strlen( pszPtr ) )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
			return;
		}
		else
		{
			ulRW |= 2;
		}
	}

	pszPtr = g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_RW ].pszParam;
	if ( pszPtr )
	{
		if ( strlen( pszPtr ) )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
			return;
		}
		else
		{
			ulRW |= ( 1 | 2 );
		}
	}

	if ( ulRW == 0 )
		ulRW |= ( 1 | 2 );

	// Check whether the "Global" flag was Specified.

	pszPtr = g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_G ].pszParam;
	if ( pszPtr )
	{
		if ( strlen( pszPtr ) )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
			return;
		}
		else
		{
			bGlobal = TRUE;
		}
	}

	// Get the Current Process Name.

	pszCurrentProcessName = GetImageFileNameFieldPtrOfCurrProc(
			extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase
		);

	memset( szProcessName, 0, sizeof( szProcessName ) );
	if ( pszCurrentProcessName )
	{
		memcpy( szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );
	}
	else
	{
		OutputPrint( FALSE, FALSE, "Unable to determine the current process name." );
		return;
	}

	pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
	if ( pszDot )
		* pszDot = 0;

	// The Address Param has to be Specified.

	if ( g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_ADDRESS ].pszParam == NULL ||
		strlen( g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_ADDRESS ].pszParam ) == 0 )
	{
		OutputPrint( FALSE, FALSE, "Address must be specified." );
		return;
	}

	// Evaluate the Address Parameter.

	dwAddress = EvaluateExpression_DWORD(
		g_vcoBpMxxxCommandOptions[ MACRO_BPMXXXCMDOPT_INDEX_ADDRESS ].pszParam, & bEvalRes );

	if ( bEvalRes == FALSE )
		return;

	// Check the Address Parameter alignment.

	if ( dwAddress > (DWORD) g_pvMmUserProbeAddress )
	{
		bGlobal = ! bGlobal;
	}

	if ( dwAddress % ulLength )
	{
		OutputPrint( FALSE, FALSE, "Address value must be aligned to %i (length).", ulLength );
		return;
	}

	// Search for a Free Slot.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		if ( psisInts->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_UNUSED )
		{
			if ( pbSlot == NULL )
				pbSlot = & psisInts->vbpBreakpoints[ ulI ];
		}
		else if (
			psisInts->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_MEMORY &&
			AreMemoryRangesConflicting(
				psisInts->vbpBreakpoints[ ulI ].dwAddress, psisInts->vbpBreakpoints[ ulI ].ulDebugRegisterCondLen,
				dwAddress, ulLength )
			)
		{
			OutputPrint( FALSE, FALSE, "Duplicate breakpoint." );
			return;
		}

	if ( pbSlot == NULL )
	{
		OutputPrint( FALSE, FALSE, "Too many breakpoints." );
		return;
	}

	//
	// Install the Breakpoint.
	//

	memset( pbSlot, 0, sizeof( BREAKPOINT ) );

	pbSlot->ulType = MACRO_BREAKPOINTTYPE_MEMORY;
	pbSlot->bInstalled = FALSE;
	pbSlot->bIsContextCompatible = TRUE;

	pbSlot->bIsUsingDebugRegisters = TRUE;
	pbSlot->ulDebugRegisterNum = ulDebugReg;

	pbSlot->dwAddress = dwAddress;
	pbSlot->ulDebugRegisterCondLen = ulLength;
	pbSlot->ulDebugRegisterCondRW = ulRW;

	memset( & pbSlot->vbPrevCodeBytes, 0xCC, MACRO_MAX_NUM_OF_PROCESSORS );

	if ( bGlobal == FALSE )
		strcpy( pbSlot->ciContext.szProcessName, szProcessName );

	//
	// Return.
	//

	return;
}

//=========================================
// BpMbCommandHandler Function Definition.
//=========================================

VOID BpMbCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	//
	// Call the Implementation Function.
	//

	BpMxxxCommandHandler( 1, pszLine, pszCommand, pszParams );

	//
	// Return.
	//

	return;
}

//=========================================
// BpMdCommandHandler Function Definition.
//=========================================

VOID BpMdCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	//
	// Call the Implementation Function.
	//

	BpMxxxCommandHandler( 4, pszLine, pszCommand, pszParams );

	//
	// Return.
	//

	return;
}

//=========================================
// BpMwCommandHandler Function Definition.
//=========================================

VOID BpMwCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	//
	// Call the Implementation Function.
	//

	BpMxxxCommandHandler( 2, pszLine, pszCommand, pszParams );

	//
	// Return.
	//

	return;
}

//=======================================
// BdCommandHandler Function Definition.
//=======================================

VOID BdCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	BOOLEAN					bUpdateCodeWindow = FALSE;
	ULONG					ulI;
	BREAKPOINT*				pbThis;
	DWORD					vdwIndexes[ 64 ];
	ULONG					ulIndexesNum;

	// Check the Command Parameters.

	if ( strcmp( pszParams, "*" ) == 0 )
	{
		// Disable all the Breakpoints.

		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & psisInts->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
				DisableBreakpoint( pbThis );
		}

		bUpdateCodeWindow = TRUE;
	}
	else if ( strlen( pszParams ) )
	{
		// Disable the Specified Breakpoints.

		ulIndexesNum = ParseHexNumSeriesString(
			vdwIndexes, sizeof( vdwIndexes ) / sizeof( DWORD ), pszParams );

		if ( ulIndexesNum == MACRO_PARSEHEXNUMSERIESSTRING_ERR )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
		}
		else
		{
			for ( ulI = 0; ulI < ulIndexesNum; ulI ++ )
				if ( vdwIndexes[ ulI ] >= 0 && vdwIndexes[ ulI ] < MACRO_MAXNUM_OF_BREAKPOINTS )
				{
					pbThis = & psisInts->vbpBreakpoints[ vdwIndexes[ ulI ] ];
					if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
						DisableBreakpoint( pbThis );
				}

			bUpdateCodeWindow = TRUE;
		}
	}
	else
	{
		// Print an Error Message.

		OutputPrint( FALSE, FALSE, "Specify the breakpoints to disable." );
	}

	// Update the Code Window.

	if ( bUpdateCodeWindow )
	{
		ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
		VPCICE_WINDOW*			pviwCodeWindow;

		pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

		if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
		{
			DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
			DrawScreen( pdglLayouts );
		}
	}

	// Return to the Caller.

	return;
}

//=======================================
// BeCommandHandler Function Definition.
//=======================================

VOID BeCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	BOOLEAN					bUpdateCodeWindow = FALSE;
	ULONG					ulI;
	BREAKPOINT*				pbThis;
	DWORD					vdwIndexes[ 64 ];
	ULONG					ulIndexesNum;

	// Check the Command Parameters.

	if ( strcmp( pszParams, "*" ) == 0 )
	{
		// Enable all the Breakpoints.

		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & psisInts->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
				EnableBreakpoint( pbThis );
		}

		bUpdateCodeWindow = TRUE;
	}
	else if ( strlen( pszParams ) )
	{
		// Enable the Specified Breakpoints.

		ulIndexesNum = ParseHexNumSeriesString(
			vdwIndexes, sizeof( vdwIndexes ) / sizeof( DWORD ), pszParams );

		if ( ulIndexesNum == MACRO_PARSEHEXNUMSERIESSTRING_ERR )
		{
			OutputPrint( FALSE, FALSE, "Syntax error." );
		}
		else
		{
			for ( ulI = 0; ulI < ulIndexesNum; ulI ++ )
				if ( vdwIndexes[ ulI ] >= 0 && vdwIndexes[ ulI ] < MACRO_MAXNUM_OF_BREAKPOINTS )
				{
					pbThis = & psisInts->vbpBreakpoints[ vdwIndexes[ ulI ] ];
					if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
						EnableBreakpoint( pbThis );
				}

			bUpdateCodeWindow = TRUE;
		}
	}
	else
	{
		// Print an Error Message.

		OutputPrint( FALSE, FALSE, "Specify the breakpoints to enable." );
	}

	// Update the Code Window.

	if ( bUpdateCodeWindow )
	{
		ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
		ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
		VPCICE_WINDOW*			pviwCodeWindow;

		pviwCodeWindow = GetVpcICEWindowByName( MACRO_VPCICE_WINDOW_NAME_CODE, & pdglLayouts->vivmVpcICEVideo );

		if ( pviwCodeWindow && pviwCodeWindow->bDisplayed )
		{
			DrawCodeWindowContents( pviwCodeWindow, ulConsoleW, ulConsoleH );
			DrawScreen( pdglLayouts );
		}
	}

	// Return to the Caller.

	return;
}

//=======================================
// BlCommandHandler Function Definition.
//=======================================

VOID BlCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisInts = & extension->sisSysInterrStatus;
	ULONG					ulI;
	BREAKPOINT*				pbThis;
	CHAR					szBuffer1[ 0x180 ];
	CHAR					szBuffer2[ 0x180 ];
	CHAR					szBuffer3[ 0x180 ];
	CHAR					szBuffer4[ 0x180 ];
	BOOLEAN					bRes;
	BOOLEAN					bPrint = FALSE;

	//
	// Display the List.
	//

	if ( strlen( pszParams ) )
	{
		OutputPrint( FALSE, FALSE, "Syntax error." );
		return;
	}

	//
	// ### ENTERING REPORT MODE ###
	//

	EnteringReportMode();

	//
	// List Display Loop.
	//

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
	{
		pbThis = & psisInts->vbpBreakpoints[ ulI ];

		if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED )
		{
			bRes = GetBreakpointDescription(
				szBuffer1, szBuffer2, pbThis );

			if ( bRes == FALSE )
			{
				sprintf( szBuffer3, "%.2X)   Error.",
						ulI
					);
			}
			else
			{
				sprintf( szBuffer3, "%.2X) %s %s%s%s",
						ulI,
						pbThis->bDisabled ? "*" : " ",
						szBuffer1,
						strlen( szBuffer2 ) ? " " : "",
						szBuffer2
					);
			}

			EliminateStringEscapes( szBuffer4, szBuffer3 );

			OutputPrint( TRUE, FALSE, "%s%s",
				pbThis == psisInts->pbpUniqueHitBreakpoint ? "!0B" : "!07",
				szBuffer4 );

			bPrint = TRUE;
		}
	}

	if ( bPrint == FALSE )
		OutputPrint( FALSE, FALSE, "No breakpoint found." );

	//
	// ### LEAVING REPORT MODE ###
	//

	ExitingReportMode();

	//
	// Return.
	//

	return;
}

//===============================
// LoadFile Function Definition.
//===============================

PVOID LoadFile( IN PCWSTR pszFileName, IN POOL_TYPE ptPoolType, OUT ULONG* pulSize )
{
	PVOID						retval = NULL;
	NTSTATUS					ntStatus;
	HANDLE						handle = NULL;
	OBJECT_ATTRIBUTES			attrs;
	UNICODE_STRING				unicode_fn;
	IO_STATUS_BLOCK				iosb;
	FILE_STANDARD_INFORMATION	info;
	ULONG						size = 0;
	PVOID						mem;
	LARGE_INTEGER				zeropos;

	memset( & zeropos, 0, sizeof( zeropos ) );

	// Load the File.

	RtlInitUnicodeString( & unicode_fn, pszFileName );

	InitializeObjectAttributes( & attrs,
		& unicode_fn,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL );

	ntStatus = ZwCreateFile( & handle,
		FILE_READ_DATA | GENERIC_READ | SYNCHRONIZE,
		& attrs,
		& iosb,
		0,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0 );

	if ( ntStatus == STATUS_SUCCESS && handle )
	{
		ntStatus = ZwQueryInformationFile(
			handle,
			& iosb,
			& info,
			sizeof( info ),
			FileStandardInformation );

		if ( ntStatus == STATUS_SUCCESS )
		{
			size = info.EndOfFile.LowPart;

			mem = ExAllocatePool( ptPoolType, size );

			if ( mem )
			{
				ntStatus = ZwReadFile(
					handle,
					NULL,
					NULL,
					NULL,
					& iosb,
					mem,
					size,
					& zeropos,
					NULL );

				if ( ntStatus != STATUS_SUCCESS || iosb.Information != size )
				{
					ExFreePool( mem );
				}
				else
				{
					retval = mem;
				}
			}
		}

		ZwClose( handle );
	}

	// Return.

	if ( pulSize && retval )
		* pulSize = size;

	return retval;
}

//==========================================
// ParseStructuredFile Function Definition.
//==========================================

CHAR* ParseStructuredFile( IN BYTE* pbFile, IN ULONG ulSize, IN ULONG ulTabsNum, IN CHAR* pszString, IN CHAR* pszStart )
{
	CHAR*			pszEnd;
	CHAR*			p;
	BOOLEAN			bEndOfLine = FALSE, bEndOfFile = FALSE;
	CHAR			szLine[ 4096 ];
	int				i, x, y;
	CHAR*			retval = NULL;
	size_t			nNumOfTabs;
	char*			pszLine;
	CHAR*			pszLineStart;
	BOOLEAN			bTNReached = FALSE;
	BOOLEAN			bFirstLine = TRUE;

	if ( pbFile == NULL || ulSize == 0 )
		return NULL;
	if ( ulTabsNum == 0 && pszString == NULL )
		return NULL;

	pszEnd = (CHAR*) pbFile + ulSize - 1; // -> LAST VALID CHAR <-

	if ( pszStart == NULL )
		pszStart = (CHAR*) pbFile;

	// Parse Each Line.

	p = pszStart;

	do
	{
		bEndOfLine = FALSE;

		pszLineStart = p;

		for ( i=0; i<sizeof(szLine)/sizeof(CHAR); i++ )
		{
			if ( p > pszEnd )
			{
				bEndOfLine = TRUE;
				bEndOfFile = TRUE;

				szLine[ i ] = '\0';
				break;
			}
			else if ( *p == '\r' || *p == '\0' )
			{
				*p = '\0';

				if ( (p+1) <= pszEnd &&
					*(p+1) == '\n' )
						p++;
				bEndOfLine = TRUE;

				szLine[ i ] = '\0';
				break;
			}
			else
			{
				szLine[ i ] = *p++;
			}
		}

		p++;

		if ( bEndOfLine == FALSE ) // LINE TOO LONG
			break;
		else
		{
			// Calculate the number of tabulations at the left of the line.

			nNumOfTabs = 0;
			for ( x=0, y=strlen( szLine ); x<y; x++ )
				if ( szLine[ x ] == '\t' )
					nNumOfTabs++;
				else
					break;

			// Make controls on the Tabs Num.

			if (
					( (pszString == NULL && bFirstLine == FALSE) || bTNReached ) &&
					nNumOfTabs < ulTabsNum
				)
					break;

			if ( nNumOfTabs >= ulTabsNum )
				bTNReached = TRUE;

			// The resulting string must be non-null.

			pszLine = &szLine[ nNumOfTabs ];
			if ( strlen( pszLine ) != 0 )
			{
				// Check if there is a Match.

				if ( ulTabsNum == nNumOfTabs &&
					( pszString == NULL || MACRO_CRTFN_NAME(stricmp)( pszString, pszLine ) == 0 ) )
				{
					retval = pszLineStart + nNumOfTabs;
					break;
				}
			}
		}

		bFirstLine = FALSE;
	}
	while( bEndOfFile == FALSE );

	// Return.

	return retval;
}

//===================================
// GetSfSetting Function Definition.
//===================================

CHAR* GetSfSetting( IN BYTE* pbFile, IN ULONG ulSize, IN CHAR* l0, IN CHAR* l1, IN CHAR* l1def )
{
	CHAR*			p;

	// Get the Setting and Return.

	p = ParseStructuredFile( pbFile, ulSize, 0, l0, NULL );
	if ( p == NULL )
		return l1def;

	p = ParseStructuredFile( pbFile, ulSize, 1, l1, p );
	if ( p == NULL )
		return l1def;

	p = ParseStructuredFile( pbFile, ulSize, 2, NULL, p );
	if ( p == NULL )
		return l1def;

	return p;
}

//=======================================
// AddSymTableToken Function Definition.
//=======================================

DWORD AddSymTableToken( IN CHAR* pszPath )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	// Add.

	strcpy( extension->szSymLoaderPendingTblPath, pszPath );
	extension->dwSymLoaderToken = extension->dwSymLoaderCounter ++;

	return extension->dwSymLoaderToken;
}

//===========================================
// CompleteSymTableLoad Function Definition.
//===========================================

BOOLEAN CompleteSymTableLoad( IN DWORD dwToken )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	PVOID					pv;

	DWORD					dwSymLoaderToken;
	CHAR					szSymLoaderPendingTblPath[ MAX_PATH ];
	WCHAR					szUcPath[ MAX_PATH + 0x10 ] = L"\\??\\";

	ANSI_STRING				asPath;
	UNICODE_STRING			usPath;
	NTSTATUS				ntStatus;

	ULONG					ulSize = 0;

	BOOLEAN					retval = TRUE;

	DWORD					ret_EAX;

	// Read the Informations from the Extension in a Safe Manner.

	_asm			cli

	dwSymLoaderToken = extension->dwSymLoaderToken;
	strcpy( szSymLoaderPendingTblPath, extension->szSymLoaderPendingTblPath );

	_asm			sti

	if ( dwToken &&
		dwToken != dwSymLoaderToken )
			return FALSE;

	// Load the File.

	RtlInitAnsiString( & asPath, szSymLoaderPendingTblPath );
	ntStatus = RtlAnsiStringToUnicodeString( & usPath, & asPath, TRUE );
	if ( ntStatus != STATUS_SUCCESS )
		return FALSE;

	memcpy( (szUcPath+4), usPath.Buffer, usPath.Length );
	(szUcPath+4)[ usPath.Length / sizeof( WCHAR ) ] = L'\0';

	RtlFreeUnicodeString( & usPath );

	pv = LoadFile( szUcPath, PagedPool, & ulSize );
	if ( pv == NULL || ulSize == 0 )
		return FALSE;

	// Complete.

	TouchPage_BYTERANGE( (BYTE*) pv, ulSize );

	if ( dwToken )
	{
		// Generate a Sync Interrupt.

		__asm
		{
			mov				eax, MACRO_EAX_TT_SERVICE_COMPLETE_SYMLOAD
			mov				ebx, pv
			mov				ecx, ulSize
			lea				edx, szSymLoaderPendingTblPath

			call			TimerThreadDebuggerEntrance

			mov				ret_EAX, eax
		}

		if ( ret_EAX )
			retval = TRUE;
		else
			retval = FALSE;
	}
	else
	{
		// Call the Implementation Directly.

		retval = AddSymTable(
			(BYTE*) pv, ulSize, szSymLoaderPendingTblPath );
	}

	// Free the Memory.

	ExFreePool( pv );

	// Return.

	return retval;
}

//==================================
// AddSymTable Function Definition.
//==================================

BOOLEAN AddSymTable( IN BYTE* pbTable, IN ULONG ulSize, IN CHAR* pszName )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	ULONG					offset = 0;
	ULONG					space = extension->ulSymMemorySize;

	SYMTABLEINFO*			last;
	SYMTABLEINFO*			_this;

	// Check out Conditions.

	if ( IsPagePresent_BYTERANGE( pbTable, ulSize ) == FALSE ||
		IsPagePresent_BYTERANGE( pszName, MAX_PATH ) == FALSE )
			return FALSE;

	if ( extension->ulSymTablesNum >= MACRO_SYMTABLES_MAXNUM )
		return FALSE;

	if ( extension->ulSymTablesNum )
	{
		last = & extension->stiSymTables[ extension->ulSymTablesNum - 1 ];

		offset = last->offset + last->general.ulSize;
		space -= offset;
	}

	if ( ulSize > space )
		return FALSE;

	// Add.

	_this = & extension->stiSymTables[ extension->ulSymTablesNum ++ ];

	_this->offset = offset;
	_this->general.ulOrdinal = extension->dwSymLoaderCounter ++;
	strcpy( _this->general.szName, pszName );
	_this->general.ulSize = ulSize;
	_this->general.eType = U2KSYMTYP_BCS;

	memcpy( extension->pbSymMemory + offset, pbTable, ulSize );

	// Return.

	return TRUE;
}

//=====================================
// RemoveSymTable Function Definition.
//=====================================

BOOLEAN RemoveSymTable( IN ULONG ulOrdinal )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	int						i, j;

	// Remove.

	for( i=0; i<(int)extension->ulSymTablesNum; i ++ )
	{
		SYMTABLEINFO*			_this = & extension->stiSymTables[ i ];

		if ( _this->general.ulOrdinal == ulOrdinal )
		{
			SYMTABLEINFO*	_last = & extension->stiSymTables[ extension->ulSymTablesNum - 1 ];

			ULONG			size = _this->general.ulSize;
			ULONG			blocksize =
				( _last->offset + _last->general.ulSize ) -
				( _this->offset + _this->general.ulSize );

			BYTE*			out = extension->pbSymMemory + _this->offset;
			BYTE*			in = ( _this == _last ) ? NULL :
				( extension->pbSymMemory + (_this+1)->offset );

			// Shift the Symbol Memory.

			for ( j = 0; j < (int)blocksize; j ++ )
				* out ++ = * in ++;

			// Shift the Symbol Structures.

			for ( j = i + 1; j < (int)extension->ulSymTablesNum; j ++ )
			{
				extension->stiSymTables[ j - 1 ] = extension->stiSymTables[ j ];
				extension->stiSymTables[ j - 1 ].offset -= size;
			}
			extension->ulSymTablesNum --;

			// Return Success.

			return TRUE;
		}
	}

	// Return Failure.

	return FALSE;
}

//=======================================
// GetEnvScriptText Function Definition.
//=======================================

static VOID strcpyWS2CS( OUT CHAR* pszCS, IN WORD* pwWS )
{
	CHAR*		out = pszCS;
	WORD*		in = pwWS;

	// Copy.

	while( * in )
		* out ++ = (BYTE) ( ( * in ++ ) & 0xFF );
	* out = '\0';

	// Return.

	return;
}

BOOLEAN GetEnvScriptText( IN BOOLEAN bFromClipboard, OUT CHAR* pszBuff, IN ULONG ulBuffSize )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					retval = FALSE;
	ULONG					ulTotalLinesNum, i;
	WORD*					in;
	WORD**					src;
	CHAR*					out = pszBuff;
	CHAR*					newout;
	CHAR*					end = out + ulBuffSize - 1; // Last Valid Char...

	if ( ulBuffSize == 0 )
		return FALSE;

	//
	// Return the Text.
	//

	src = bFromClipboard == FALSE ?
		( extension->ppwScriptWinStrPtrsBuffer ) :
		( extension->ppwScriptWinClipStrPtrsBuffer );

	ulTotalLinesNum = bFromClipboard == FALSE ?
		( extension->ulScriptWinStrPtrsBufferPosInBytes / sizeof( WORD* ) ) :
		( extension->ulScriptWinClipStrPtrsBufferPosInBytes / sizeof( WORD* ) );

	retval = TRUE;

	for ( i=0; i<ulTotalLinesNum; i ++ )
	{
		in = src[ i ];
		newout = out + WordStringLen( in ) + 2;

		if ( newout > end )
		{
			retval = FALSE;
			break;
		}

		strcpyWS2CS( out, in );
		strcat( out, "\x0D\x0A" );

		out = newout;
	}

	//
	// Return.
	//

	return retval;
}

//=======================================
// SetEnvScriptText Function Definition.
//=======================================

BOOLEAN SetEnvScriptText( IN BOOLEAN bFromClipboard, IN CHAR* pszBuff )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					retval = FALSE;
	CHAR*					in;
	CHAR*					end = pszBuff + strlen( pszBuff ) - 1; // Last Valid Char.
	ULONG					ulLinesNum, ulCharsNum, ulLineStartCharsNum;
	WORD*					out;
	WORD**					lout;
	BOOLEAN					bLineStart;
	ULONG					i;

	WORD*					pwBuffer = bFromClipboard == FALSE ?
		extension->pwScriptWinBuffer :
		extension->pwScriptWinClipBuffer;
	ULONG					ulBufferSizeInBytes = bFromClipboard == FALSE ?
		extension->ulScriptWinBufferSizeInBytes :
		extension->ulScriptWinClipBufferSizeInBytes;
	WORD**					ppwStrPtrsBuffer = bFromClipboard == FALSE ?
		extension->ppwScriptWinStrPtrsBuffer :
		extension->ppwScriptWinClipStrPtrsBuffer;
	ULONG					ulStrPtrsBufferSizeInBytes = bFromClipboard == FALSE ?
		extension->ulScriptWinStrPtrsBufferSizeInBytes :
		extension->ulScriptWinClipStrPtrsBufferSizeInBytes;
	ULONG*					pulBufferPosInBytes = bFromClipboard == FALSE ?
		& extension->ulScriptWinBufferPosInBytes :
		& extension->ulScriptWinClipBufferPosInBytes;
	ULONG*					pulStrPtrsBufferPosInBytes = bFromClipboard == FALSE ?
		& extension->ulScriptWinStrPtrsBufferPosInBytes :
		& extension->ulScriptWinClipStrPtrsBufferPosInBytes;

	//
	// Check the Memory Constraints.
	//

	ulLinesNum = 0;
	ulCharsNum = 0;
	ulLineStartCharsNum = 0;

	in = pszBuff;

	bLineStart = TRUE;

	while( in <= end )
	{
		if ( bLineStart )
		{
			if ( ulCharsNum - ulLineStartCharsNum >= MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1 )
				return FALSE;
			ulLineStartCharsNum = ulCharsNum;

			ulLinesNum ++;
			bLineStart = FALSE;
		}

		if ( * in == 0x0D &&
			(in+1) <= end &&
			* (in+1) == 0x0A )
		{
			in ++;

			ulCharsNum ++; // NULL TERMINATOR IN THE WORD BUFFER...
			bLineStart = TRUE;
		}
		else
		{
			ulCharsNum ++; // A CHARACTER IN THE WORD BUFFER...
		}

		in ++;
	}

	if ( bLineStart )
	{
		ulLinesNum ++;
		ulCharsNum ++;

		if ( ulCharsNum - ulLineStartCharsNum >= MACRO_SCRIPTWIN_LINESIZE_IN_CHARS - 1 )
			return FALSE;
	}

	if ( end < pszBuff /* there is nothing in the buffer */ )
	{
		ulLinesNum = 0;
		ulCharsNum = 0;
	}

	if ( ulCharsNum * sizeof( WORD ) > ulBufferSizeInBytes ||
		ulLinesNum * sizeof( WORD* ) > ulStrPtrsBufferSizeInBytes )
	{
		return FALSE;
	}

	//
	// Set the Text.
	//

	retval = TRUE;

	* pulBufferPosInBytes = ulCharsNum * sizeof( WORD );
	* pulStrPtrsBufferPosInBytes = ulLinesNum * sizeof( WORD* );

	//

	in = pszBuff;
	out = pwBuffer;
	lout = ppwStrPtrsBuffer;

	bLineStart = TRUE;

	while( in <= end )
	{
		if ( bLineStart )
		{
			* lout ++ = out;
			bLineStart = FALSE;
		}

		if ( * in == 0x0D &&
			(in+1) <= end &&
			* (in+1) == 0x0A )
		{
			in ++;

			* out ++ = 0;
			bLineStart = TRUE;
		}
		else
		{
			* out ++ = * in;
		}

		in ++;
	}

	if ( bLineStart )
	{
		* lout ++ = out;
		* out = 0;
	}

	//
	// Syntax Color the File...
	//

	for ( i=0; i<ulLinesNum; i ++ )
		ApplySyntaxColoring( ppwStrPtrsBuffer[ i ] );

	//
	// In the case of the Script File, do some other Operations.
	//

	if ( bFromClipboard == FALSE )
	{
		//
		// Set the Dirty Flag...
		//

		extension->bScriptWinDirtyBit = TRUE;
	}

	//
	// Return.
	//

	return retval;
}

//==============================================
// ExecuteAutoTypedCommand Function Definition.
//==============================================

VOID ExecuteAutoTypedCommand( VOID )
{
	DRAWGLYPH_LAYOUTS*		pdglLayouts = GetDrawGlyphLayoutsInstance();
	ULONG					ulConsoleW = pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
	ULONG					ulConsoleH = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars;
	ULONG					ulX0, ulY0, ulX1, ulY1;

	// Execute the Command.

	GetConsolePrintCoords( & ulX0, & ulY0, & ulX1, & ulY1 );

	OutputTextString( pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer, ulConsoleW, ulConsoleH,
		ulX0 + 1, ulY1 - 1, 0x07, g_szAutoTypedCommand );

	if ( pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
		MakeCursorBlink();

	ProcessKeyStroke( MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE, MACRO_SCANCODE_ENTER );

	// Return.

	return;
}

//=========================================
// DashCommandHandler Function Definition.
//=========================================

VOID DashCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	// Check whether a Launcher is active.

	if ( IsClientPollingForDebuggerVerb () == FALSE )
	{
		OutputPrint( FALSE, FALSE, "No launcher found. The support service was not started." );
		return;
	}

	// Show the Dashboard.

	extension->bOpenUserModeDashboard = TRUE;

	// Return.

	return;
}

//========================================
// BpeCommandHandler Function Definition.
//========================================

VOID BpeCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams )
{








}
