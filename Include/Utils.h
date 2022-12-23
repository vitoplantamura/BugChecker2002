/***************************************************************************************
  *
  * Utils.h - VPCICE Support Routines Header File.
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

#pragma once

#pragma warning(disable : 4273) // WDK 7600.16385.0 compatibility.

//===========
// Includes.
//===========

#include <ntddk.h>
#include "WinDefs.h"

//=============================
// General Definitions/Macros.
//=============================

// Video.

#define CLRTXT2VIDBUFWORD( fore, back, chr )	( ( back << 12 ) | ( fore << 8 ) | ( chr ) )

// Virtual Memory.

#ifndef MACRO_SYSTEM_PAGE_SIZE
#define MACRO_SYSTEM_PAGE_SIZE				( 0x1000 )
#endif

#define MACRO_SYSTEM_PAGE_MASK				( 0xFFFFF000 )

// System (OS-Specific) - converted from MACROs to INTs.

extern int  MACRO_KPEB_IMGFLNAME_FIELD_SIZE			; // default value = ( 0x10 ) for Windows 2000 SP4 // <-- ## WARNING ## do not modify this: it is hardcoded in various parts of the debugger...

extern DWORD MACRO_PCR_ADDRESS_OF_1ST_PROCESSOR		; // default value = ( 0xFFDFF000 ) for Windows 2000 SP4
extern int  MACRO_KTEBPTR_FIELDOFFSET_IN_PCR		; // default value = ( 0x124 ) for Windows 2000 SP4
extern int  MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB		; // default value = ( 0x44 ) for Windows 2000 SP4
extern int  MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB		; // default value = ( 0x1FC ) for Windows 2000 SP4
extern int  MACRO_CURRIRQL_FIELDOFFSET_IN_PCR		; // default value = ( 0x24 ) for Windows 2000 SP4
extern int  MACRO_TID_FIELDOFFSET_IN_KTEB			; // default value = ( 0x1E4 ) for Windows 2000 SP4
extern int  MACRO_PID_FIELDOFFSET_IN_KPEB			; // default value = ( 0x9C ) for Windows 2000 SP4
extern int  MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC	; // default value = ( 0x18 ) for Windows 2000 SP4
extern int  MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC	; // default value = ( 0x24 ) for Windows 2000 SP4
extern char* MACRO_NTOSKRNL_MODULENAME_UPPERCASE	; // default value = "NTOSKRNL.EXE" for Windows 2000 SP4
extern int  MACRO_VADROOT_FIELDOFFSET_IN_KPEB		; // default value = ( 0x194 ) for Windows 2000 SP4
extern int  MACRO_VADTREE_UNDOCUMENTED_DISP_0		; // default value = ( 0x24 ) for Windows 2000 SP4
extern int  MACRO_VADTREE_UNDOCUMENTED_DISP_1		; // default value = ( 0x30 ) for Windows 2000 SP4
extern int  MACRO_MAPVIEWOFIMAGESECTION_STARTPARAM	; // default value = ( 0x8 ) for Windows 2000 SP4
extern int  MACRO_MAPVIEWOFIMAGESECTION_SIZEPARAM	; // default value = ( 0x10 ) for Windows 2000 SP4
extern int  MACRO_UNMAPVIEWOFIMAGESECTION_STARTPARAM; // default value = ( 0x4 ) for Windows 2000 SP4
extern int  MACRO_MAPVIEWOFIMAGESECTION_KPEBPARAM	; // default value = ( 0x4 ) for Windows 2000 SP4
extern int  MACRO_UNMAPVIEWOFIMAGESECTION_KPEBPARAM	; // default value = ( 0x0 ) for Windows 2000 SP4
extern int  MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP	; // default value = ( - 0x8 ) for Windows 2000 SP4
extern char* MACRO_IDLEPROCESS_IMAGEFILENAME		; // default value = "Idle" for Windows 2000 SP4
extern int  MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB	; // default value = ( 0xA0 ) for Windows 2000 SP4
extern int  MACRO_CR3_FIELDOFFSET_IN_KPEB			; // default value = ( 0x18 ) for Windows 2000 SP4
extern int  MACRO_MAXVALUE_FOR_WINNT_SEGSELECTOR	; // default value = ( 0x60 ) for Windows 2000 SP4
extern DWORD MACRO_MAPVIEWOFIMAGESECTION_ADDR		; // default value = 0 for Windows 2000 SP4 <-- do a byte pattern search for the function entry point.
extern int MACRO_NTTERMINATEPROCESS_FNINDEX			; // default value = 0 for Windows 2000 SP4 <-- do a byte pattern search for the function entry point.

// APIs.

#define MACRO_MAPVIEWOFIMAGESECTION_FNSRCH_RANGE	( 0x150 )
#define MACRO_MAPVIEWOFIMAGESECTION_FNSRCH_1STBYTE	( 0x55 )
#define MACRO_MAPVIEWOFIMAGESECTION_FNSRCH_2NDDWORD	( 0xFF6AEC8B )

// Various.

#define MACRO_GETNTTERMINATEPROCESSFNINDEX_ERR	( 0xFFFFFFFF )

//===================
// Structures/Types.
//===================

// // //

typedef struct _MP_SPINLOCK
{
	DWORD		state;

} MP_SPINLOCK, *PMP_SPINLOCK;

// // //

#pragma pack(push, 1)

	typedef struct _VAD
	{
		VOID*			pvStartingAddress;
		VOID*			pvEndingAddress;
		struct _VAD*	pvadParentLink;
		struct _VAD*	pvadLeftLink;
		struct _VAD*	pvadRightLink;
		DWORD			dwFlags;
		DWORD			dwUndocumented_DWORD;

	} VAD, *PVAD;

#pragma pack(pop)

// // //

typedef BOOLEAN ( *PFNUSERMODULESNAMELISTENUM )( IN CHAR* pszModName, IN VOID* pvStart, IN ULONG ulLength, IN IMAGE_NT_HEADERS* pinhNtHeaders, IN DWORD dwParam );

typedef struct _DISCVBPTR_ADDPARAMS
{
	BOOLEAN						bIsBytePointerAtTheBeginning;
	BOOLEAN						bTouchPages;
	BYTE*						pbKpeb;
	PFNUSERMODULESNAMELISTENUM	pfnUserModListCallBack;
	DWORD						dwUserModListCallBackPARAM;
	ULONG						ulSplitInfoNoSectionStringMaxLen;
	DWORD						dwSplittedOffsetNoSection;
	BOOLEAN						bPreserveModuleFileExtension;

} DISCVBPTR_ADDPARAMS, *PDISCVBPTR_ADDPARAMS;

// // //

typedef BOOLEAN ( *PFNPROCESSESLISTENUM )( BYTE* pbKpeb, DWORD dwParam );

typedef struct _PROCESSLIST_INFO
{
	BYTE*			pbIdleProcess;
	LIST_ENTRY*		pleProcessListHead;

} PROCESSLIST_INFO, *PPROCESSLIST_INFO;

// // //

typedef struct _IEEE_FLOAT
{
  ULONG				ulMantissa:23;
  ULONG				ulExponent:8;
  ULONG				ulNegative:1;
} IEEE_FLOAT, *PIEEE_FLOAT;

typedef struct _IEEE_DOUBLE
{
  __int64			i64Mantissa:52;
  ULONG				ulExponent:11;
  ULONG				ulNegative:1;
} IEEE_DOUBLE, *PIEEE_DOUBLE;

#define IEEE754_SINGLE_BIAS			127
#define IEEE754_DOUBLE_BIAS			1023

// // //

//============================
// System Prototypes/Externs.
//============================

NTKERNELAPI
NTSTATUS
NTAPI
MmMapViewOfSection (
    IN PVOID                SectionObject,
    IN PEPROCESS            Process,
    IN OUT PVOID            *BaseAddress,
    IN ULONG                ZeroBits,
    IN ULONG                CommitSize,
    IN OUT PLARGE_INTEGER   SectionOffset OPTIONAL,
    IN OUT PULONG           ViewSize,
    IN SECTION_INHERIT      InheritDisposition,
    IN ULONG                AllocationType,
    IN ULONG                Protect
);

NTKERNELAPI
NTSTATUS
NTAPI
MmUnmapViewOfSection (
    IN PEPROCESS            Process,
	IN PVOID				BaseAddress
);

NTSTATUS
NTAPI
ZwTerminateProcess /* = NtTerminateProcess */ (
	IN HANDLE				ProcessHandle,
	IN LONG					ExitStatus
);

// // //

VOID KeAttachProcess(
  IN PEPROCESS Process
);

VOID KeDetachProcess(
  VOID
);

NTSTATUS PsLookupProcessByProcessId(
  IN ULONG Process_ID,
  OUT PVOID *EProcess
);

NTSTATUS ZwAllocateVirtualMemory(
  IN HANDLE ProcessHandle,
  IN OUT PVOID *BaseAddress,
  IN ULONG ZeroBits,
  IN OUT PULONG RegionSize,
  IN ULONG AllocationType,
  IN ULONG Protect
);

NTSTATUS NTAPI ZwFreeVirtualMemory(
  IN HANDLE ProcessHandle,
  IN OUT PVOID *BaseAddress,
  IN OUT PULONG RegionSize,
  IN ULONG FreeType
);

//======================
// Function Prototypes.
//======================

NTSTATUS PhysAddressToLinearAddresses( OUT PVOID* ppvOutputVector, IN ULONG ulOutputVectorSize, OUT ULONG* pulOutputVectorRetItemsNum, IN DWORD dwPhysAddress );
VOID OutputTextString( IN PVOID pvTextVideoBufferPtr, IN ULONG ulTextVideoBufferWidth, IN ULONG ulTextVideoBufferHeight, IN ULONG ulX, IN ULONG ulY, IN BYTE bTextColor, CHAR* pszTextString );
NTSTATUS WaitForKeyBeingPressed( IN BYTE bKeyScanCode, IN ULONG ulElapseInHundredthsOfSec );
BOOLEAN CompMem( IN PVOID pvMem1, IN PVOID pvMem2, IN ULONG ulSize );
WORD* GetPtrInTextBuffer( IN PVOID pvTextVideoBufferPtr, IN ULONG ulTextVideoBufferWidth, IN ULONG ulTextVideoBufferHeight, IN ULONG ulX, IN ULONG ulY );
CHAR* GetImageFileNameFieldPtrOfCurrProc( IN VOID* pvPCRAddress );
KIRQL GetCurrentIrql( IN VOID* pvPCRAddress );
VOID OutputTextStringSpecial( IN PVOID pvTextVideoBufferPtr, IN ULONG ulTextVideoBufferWidth, IN ULONG ulTextVideoBufferHeight, IN ULONG ulX, IN ULONG ulY, IN ULONG ulLenOnScreen, IN BYTE bTextColor, CHAR* pszTextString );
VOID PrepareUnEscapedString( OUT CHAR* pszOutputString, IN CHAR* pszInputString );
VOID EliminateStringEscapes( OUT CHAR* pszOutputString, IN CHAR* pszInputString );
PVOID GetCurrentKTEB( IN VOID* pvPCRAddress );
WORD GetCurrentTID( IN VOID* pvPCRAddress );
BOOLEAN IsPagePresent( IN PVOID pvVirtAddress );
VOID InitializeMultiProcessorSpinLock( OUT MP_SPINLOCK* pmpslSpinLock );
VOID EnterMultiProcessorSpinLock( VOID /* "IN MP_SPINLOCK* pmpslSpinLock" in EBX Register. */ );
VOID LeaveMultiProcessorSpinLock( VOID /* "IN MP_SPINLOCK* pmpslSpinLock" in EBX Register. */ );
WORD GetCurrentPID( IN VOID* pvPCRAddress );
VOID* DiscoverNtoskrnlDriverSection( IN VOID* pvDriverSection );
BOOLEAN IsPagePresent_DWORD( IN DWORD* pdwDwordPtr );
BOOLEAN IsPagePresent_WORD( IN WORD* pwWordPtr );
VOID DiscoverBytePointerPosInModules( OUT CHAR* pszOutputBuffer, IN BYTE* pbBytePointer, IN VOID* pvNtoskrnlDriverSection, IN VOID* pvMmUserProbeAddress, IN VOID* pvPCRAddress, IN OUT DISCVBPTR_ADDPARAMS* pdbpAddParams );
BOOLEAN IsPagePresent_BYTERANGE( IN BYTE* pbStart, IN ULONG ulLength );
PVOID Guess_MiMapViewOfImageSection_FnPtr( VOID* pvNtoskrnlDriverSection );
PVOID Guess_MiUnMapViewOfImageSection_FnPtr( VOID );
PVOID GetCurrentKPEB( IN VOID* pvPCRAddress );
VOID TouchPage_BYTE( IN BYTE* pbPtr );
VOID TouchPage_BYTERANGE( IN BYTE* pbStart, IN ULONG ulLength );
ULONG Get_NtTerminateProcess_FnIndex( VOID );
VOID IterateThroughListOfProcesses( IN PROCESSLIST_INFO* pliProcessListInfo, IN PFNPROCESSESLISTENUM pfnProcessListEnumFn, IN DWORD dwParam );
BOOLEAN GetProcessListInfo( OUT PROCESSLIST_INFO* pliProcessListInfo );
DWORD VpcICEAttachProcess( IN BYTE* pbKpeb );
VOID VpcICEDetachProcess( IN DWORD dwCookie );
VOID RebootSystemNOW( VOID );
VOID EnterMultiProcessorSpinLockAndTestByte( VOID /* "IN MP_SPINLOCK* pmpslSpinLock" in EBX Register, "IN BYTE* pbVarToWatchPtr" in ECX Register. Returns TRUE if *pbVarToWatchPtr != 0. */ );
NTSTATUS LinearAddressToPhysAddress( OUT DWORD* pdwPhysAddress, IN DWORD dwLinearAddress ); /* WARNING: The page has to be PRESENT. */
VOID BugChecker_ReadTimeStampCounter( OUT LARGE_INTEGER* pliOutput );
VOID* MapPhysPageInDebuggerHyperSlot( IN DWORD dwPhysAddress, OUT DWORD* pdwOldPTE );
VOID RestoreDebuggerHyperSlotState( IN DWORD dwPTE );
ULONG WordStringLen( IN WORD* pwStr );
CHAR* ultobinstr( OUT CHAR* pszOutput, IN ULONG ulInput );
BOOLEAN GetLastBranchMSRs( OUT DWORD* pdwFromIP, OUT DWORD* pdwToIP );
BOOLEAN AreMSRsAvailable( VOID );
VOID EnableLastBranchMSRs( VOID );
VOID InitializeProcessorForDebugging( VOID );
DWORD IsInsideVmware ( VOID );
