/***************************************************************************************
  *
  * Utils.c - VPCICE Support Routines Source File.
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
#include <stdio.h>
#include "..\Include\Utils.h"

//==========================
// OS-Specific Definitions.
//==========================

int MACRO_KPEB_IMGFLNAME_FIELD_SIZE				= ( 0x10 ); // default value for Windows 2000 SP4 // <-- ## WARNING ## do not modify this: it is hardcoded in various parts of the debugger...

DWORD MACRO_PCR_ADDRESS_OF_1ST_PROCESSOR		= ( 0xFFDFF000 ); // default value for Windows 2000 SP4
int MACRO_KTEBPTR_FIELDOFFSET_IN_PCR			= ( 0x124 ); // default value for Windows 2000 SP4
int MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB			= ( 0x44 ); // default value for Windows 2000 SP4
int MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB			= ( 0x1FC ); // default value for Windows 2000 SP4
int MACRO_CURRIRQL_FIELDOFFSET_IN_PCR			= ( 0x24 ); // default value for Windows 2000 SP4
int MACRO_TID_FIELDOFFSET_IN_KTEB				= ( 0x1E4 ); // default value for Windows 2000 SP4
int MACRO_PID_FIELDOFFSET_IN_KPEB				= ( 0x9C ); // default value for Windows 2000 SP4
int MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC		= ( 0x18 ); // default value for Windows 2000 SP4
int MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC		= ( 0x24 ); // default value for Windows 2000 SP4
char* MACRO_NTOSKRNL_MODULENAME_UPPERCASE		= "NTOSKRNL.EXE"; // default value for Windows 2000 SP4
int MACRO_VADROOT_FIELDOFFSET_IN_KPEB			= ( 0x194 ); // default value for Windows 2000 SP4
int MACRO_VADTREE_UNDOCUMENTED_DISP_0			= ( 0x24 ); // default value for Windows 2000 SP4
int MACRO_VADTREE_UNDOCUMENTED_DISP_1			= ( 0x30 ); // default value for Windows 2000 SP4
int MACRO_MAPVIEWOFIMAGESECTION_STARTPARAM		= ( 0x8 ); // default value for Windows 2000 SP4
int MACRO_MAPVIEWOFIMAGESECTION_SIZEPARAM		= ( 0x10 ); // default value for Windows 2000 SP4
int MACRO_UNMAPVIEWOFIMAGESECTION_STARTPARAM	= ( 0x4 ); // default value for Windows 2000 SP4
int MACRO_MAPVIEWOFIMAGESECTION_KPEBPARAM		= ( 0x4 ); // default value for Windows 2000 SP4
int MACRO_UNMAPVIEWOFIMAGESECTION_KPEBPARAM		= ( 0x0 ); // default value for Windows 2000 SP4
int MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP		= ( - 0x8 ); // default value for Windows 2000 SP4
char* MACRO_IDLEPROCESS_IMAGEFILENAME			= "Idle"; // default value for Windows 2000 SP4
int MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB		= ( 0xA0 ); // default value for Windows 2000 SP4
int MACRO_CR3_FIELDOFFSET_IN_KPEB				= ( 0x18 ); // default value for Windows 2000 SP4
int MACRO_MAXVALUE_FOR_WINNT_SEGSELECTOR		= ( 0x60 ); // default value for Windows 2000 SP4
DWORD MACRO_MAPVIEWOFIMAGESECTION_ADDR			= 0; // default value for Windows 2000 SP4
int MACRO_NTTERMINATEPROCESS_FNINDEX			= 0; // default value for Windows 2000 SP4

//=====================================================
// This one will Allow the Use of Floating Point code.
//=====================================================

char		_fltused;

//===================================================
// PhysAddressToLinearAddresses Function Definition.
//===================================================

NTSTATUS PhysAddressToLinearAddresses( OUT PVOID* ppvOutputVector, IN ULONG ulOutputVectorSize, OUT ULONG* pulOutputVectorRetItemsNum, IN DWORD dwPhysAddress )
{
	NTSTATUS			nsRetVal = STATUS_SUCCESS;
	DWORD*				pdwPageDir = (DWORD*) 0xC0300000;
	ULONG				i, j;
	DWORD				dwPageDirEntry, dwPageTblEntry;
	DWORD*				pdwPageTable;
	DWORD				dwPageTblEntryPhysAddress[ 2 ];
	ULONG				ulOutputVectorPos = 0;

	if ( pulOutputVectorRetItemsNum )
		* pulOutputVectorRetItemsNum = 0;

	// Search in the Page Directory for the Specified Address.

	for ( i=0; i<1024; i++ )
	{
		dwPageDirEntry = pdwPageDir[ i ];

		// Check if this Page Table has an Address and if its Present bit is set to 1.

		if ( ( dwPageDirEntry >> 12 ) &&
			( dwPageDirEntry & 0x1 ) )
		{
			pdwPageTable = (DWORD*) ( (BYTE*) 0xC0000000 + i * 0x1000 );

			for ( j=0; j<1024; j++ )
			{
				dwPageTblEntry = pdwPageTable[ j ];

				// Check if this Page Table Entry has an associated Physical Address.

				if ( dwPageTblEntry >> 12 )
				{
					// Calculate the MIN and MAX Phys Address of the Page Table Entry.

					dwPageTblEntryPhysAddress[ 0 ] = dwPageTblEntry & 0xFFFFF000;
					dwPageTblEntryPhysAddress[ 1 ] = dwPageTblEntryPhysAddress[ 0 ] + 0x1000 - 1;

					// Check if our Address is between the Interval.

					if ( dwPhysAddress >= dwPageTblEntryPhysAddress[ 0 ] &&
						dwPhysAddress <= dwPageTblEntryPhysAddress[ 1 ] )
					{
						// Add this Linear Address.

						if ( ulOutputVectorPos < ulOutputVectorSize )
						{
							ppvOutputVector[ ulOutputVectorPos ++ ] = (PVOID)
								( i * 0x400000 + j * 0x1000 +
								( dwPhysAddress - dwPageTblEntryPhysAddress[ 0 ] ) );
						}
						else
						{
							if ( pulOutputVectorRetItemsNum )
								* pulOutputVectorRetItemsNum = ulOutputVectorPos;

							return STATUS_SUCCESS;
						}
					}
				}
			}
		}
	}

	// Return to the Caller.

	if ( pulOutputVectorRetItemsNum )
		* pulOutputVectorRetItemsNum = ulOutputVectorPos;

	return nsRetVal;
}

//=======================================
// OutputTextString Function Definition.
//=======================================

VOID OutputTextString( IN PVOID pvTextVideoBufferPtr, IN ULONG ulTextVideoBufferWidth, IN ULONG ulTextVideoBufferHeight, IN ULONG ulX, IN ULONG ulY, IN BYTE bTextColor, CHAR* pszTextString )
{
	WORD*			pwVideoBufferPos;
	WORD			wColorMask;
	size_t			sTextStringLen;
	ULONG			i;
	CHAR*			pszTextStringPtr;

	// Write the String.

	wColorMask = bTextColor << 8;
	pwVideoBufferPos = ( WORD* ) pvTextVideoBufferPtr + ulY * ulTextVideoBufferWidth + ulX;
	sTextStringLen = strlen( pszTextString );
	pszTextStringPtr = pszTextString;

	for ( i=0; i<sTextStringLen; i++ )
		* pwVideoBufferPos ++ = wColorMask | (BYTE) * pszTextStringPtr ++;

	// Return to the Caller.

	return;
}

//=============================================
// WaitForKeyBeingPressed Function Definition.
//=============================================

NTSTATUS WaitForKeyBeingPressed( IN BYTE bKeyScanCode, IN ULONG ulElapseInHundredthsOfSec )
{
	LARGE_INTEGER			liWaitElapse;
	ULONG					i;
	BYTE					bKeybPortByte;

	// Wait and Read the Keyboard Port.

	liWaitElapse = RtlConvertLongToLargeInteger( - 100000 ); // Ten Milliseconds.

	for ( i=0; i<ulElapseInHundredthsOfSec; i++ )
	{
		// Read from the Keyboard Port.

		__asm
		{
			in		al, 0x64
			test	al, 1
			mov		al, 0
			jz		_SkipKeybInputPortRead

			in		al, 0x60

_SkipKeybInputPortRead:

			mov		bKeybPortByte, al
		}

		// Check if we have to Exit.

		if ( bKeybPortByte == bKeyScanCode )
			return STATUS_SUCCESS;

		// Wait.

		KeDelayExecutionThread( UserMode, FALSE, & liWaitElapse );
	}

	// Return to the Caller.

	return STATUS_UNSUCCESSFUL;
}

//==============================
// CompMem Function Definition.
//==============================

BOOLEAN CompMem( IN PVOID pvMem1, IN PVOID pvMem2, IN ULONG ulSize )
{
	BYTE*		pbMem1 = (BYTE*) pvMem1;
	BYTE*		pbMem2 = (BYTE*) pvMem2;
	ULONG		ulI;

	// Do the Memory Comparison and Return.

	for ( ulI = 0; ulI < ulSize; ulI ++, pbMem1 ++, pbMem2 ++ )
		if ( * pbMem1 != * pbMem2 )
			return TRUE;

	return FALSE;
}

//=========================================
// GetPtrInTextBuffer Function Definition.
//=========================================

WORD* GetPtrInTextBuffer( IN PVOID pvTextVideoBufferPtr, IN ULONG ulTextVideoBufferWidth, IN ULONG ulTextVideoBufferHeight, IN ULONG ulX, IN ULONG ulY )
{
	// Return the Required Information.

	return ( WORD* ) pvTextVideoBufferPtr + ulY * ulTextVideoBufferWidth + ulX;
}

//=========================================================
// GetImageFileNameFieldPtrOfCurrProc Function Definition.
//=========================================================

CHAR* GetImageFileNameFieldPtrOfCurrProc( IN VOID* pvPCRAddress )
{
	BYTE*			pbPcr = (BYTE*) pvPCRAddress;
	BYTE*			pbKteb;
	BYTE*			pbKpeb;

	// Return the Required Information.

	pbKteb = (BYTE*) ( * (DWORD*) ( pbPcr + MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ) );

	if ( pbKteb == NULL )
		return NULL;

	pbKpeb = (BYTE*) ( * (DWORD*) ( pbKteb + MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB ) );

	if ( pbKpeb == NULL )
		return NULL;

	return (CHAR*) ( pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );
}

//=====================================
// GetCurrentIrql Function Definition.
//=====================================

KIRQL GetCurrentIrql( IN VOID* pvPCRAddress )
{
	BYTE*			pbPcr = (BYTE*) pvPCRAddress;

	// Return the Required Information.

	return * (KIRQL*) ( pbPcr + MACRO_CURRIRQL_FIELDOFFSET_IN_PCR );
}

//================================
// IsHexChar Function Definition.
//================================

static BOOLEAN IsHexChar( CHAR cChar )
{
	// Test the Character.

	if ( ( cChar >= '0' && cChar <= '9' ) || ( cChar >= 'A' && cChar <= 'F' ) || ( cChar >= 'a' && cChar <= 'f' ) )
		return TRUE;
	else
		return FALSE;
}

//===================================
// ShiftHexChar Function Definition.
//===================================

static ULONG ShiftHexChar( CHAR cChar )
{
	// Test the Character.

	if ( cChar >= '0' && cChar <= '9' )
		return cChar - '0';
	else if ( cChar >= 'A' && cChar <= 'F' )
		return cChar - 'A' + 0xA;
	else if ( cChar >= 'a' && cChar <= 'f' )
		return cChar - 'a' + 0xA;
	else
		return 0;
}

//==============================================
// OutputTextStringSpecial Function Definition.
//==============================================

VOID OutputTextStringSpecial( IN PVOID pvTextVideoBufferPtr, IN ULONG ulTextVideoBufferWidth, IN ULONG ulTextVideoBufferHeight, IN ULONG ulX, IN ULONG ulY, IN ULONG ulLenOnScreen, IN BYTE bTextColor, CHAR* pszTextString )
{
	WORD*			pwVideoBufferPos;
	WORD			wColorMask;
	size_t			sTextStringLen;
	ULONG			i;
	CHAR*			pszTextStringPtr;
	CHAR			cNextChar;

	if ( ulLenOnScreen == 0 )
		return;

	// Write the String.

	wColorMask = bTextColor << 8;
	pwVideoBufferPos = ( WORD* ) pvTextVideoBufferPtr + ulY * ulTextVideoBufferWidth + ulX;
	sTextStringLen = strlen( pszTextString );
	pszTextStringPtr = pszTextString;

	for ( i=0; i<sTextStringLen; i++ )
	{
		cNextChar = * pszTextStringPtr ++;

		if ( cNextChar == '!' )
		{
			if ( * pszTextStringPtr == '!' )
			{
				i ++;
				pszTextStringPtr ++;
			}
			else if ( IsHexChar( *pszTextStringPtr ) &&
				IsHexChar( *(pszTextStringPtr+1) ) )
			{
				wColorMask = (WORD) ( ( ( ( ShiftHexChar(*pszTextStringPtr) ) << 4 ) | ( ShiftHexChar(*(pszTextStringPtr+1)) ) ) << 8 );

				i += 2;
				pszTextStringPtr += 2;

				continue;
			}
		}

		* pwVideoBufferPos ++ = wColorMask | (BYTE) cNextChar;

		if ( ( -- ulLenOnScreen ) == 0 )
			return;
	}

	// Clear the Remaining Portion of the Line.

	if ( ulLenOnScreen < 0x80000000 )
	{
		while( TRUE )
		{
			* pwVideoBufferPos ++ = wColorMask | ' ';

			if ( ( -- ulLenOnScreen ) == 0 )
				return;
		}
	}

	// Return to the Caller.

	return;
}

//=============================================
// PrepareUnEscapedString Function Definition.
//=============================================

VOID PrepareUnEscapedString( OUT CHAR* pszOutputString, IN CHAR* pszInputString )
{
	// Do the Requested Operation.

	do
	{
		* pszOutputString = * pszInputString;

		if ( * pszInputString == 0 )
			return;
		else if ( * pszInputString == '!' )
			* ++ pszOutputString = '!';

		pszInputString ++;
		pszOutputString ++;
	}
	while( TRUE );
}

//=============================================
// EliminateStringEscapes Function Definition.
//=============================================

VOID EliminateStringEscapes( OUT CHAR* pszOutputString, IN CHAR* pszInputString )
{
	// Do the Requested Operation.

	do
	{
_Loop_Start:

		if ( * pszInputString == '!' &&
			IsHexChar( *(pszInputString+1) ) &&
			IsHexChar( *(pszInputString+2) ) )
		{
			pszInputString += 3;

			goto _Loop_Start;
		}
		else if ( * pszInputString == '!' &&
			* ( pszInputString + 1 ) == '!' )
		{
			pszInputString += 2;

			* pszOutputString ++ = '!';

			goto _Loop_Start;
		}

		* pszOutputString = * pszInputString;

		if ( * pszInputString == 0 )
			return;

		pszInputString ++;
		pszOutputString ++;
	}
	while( TRUE );
}

//=====================================
// GetCurrentKTEB Function Definition.
//=====================================

PVOID GetCurrentKTEB( IN VOID* pvPCRAddress )
{
	BYTE*			pbPcr = (BYTE*) pvPCRAddress;
	BYTE*			pbKteb;

	// Return the Information.

	pbKteb = (BYTE*) ( * (DWORD*) ( pbPcr + MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ) );

	return (PVOID) pbKteb;
}

//====================================
// GetCurrentTID Function Definition.
//====================================

WORD GetCurrentTID( IN VOID* pvPCRAddress )
{
	BYTE*			pbPcr = (BYTE*) pvPCRAddress;
	BYTE*			pbKteb;

	// Return the Information.

	pbKteb = (BYTE*) ( * (DWORD*) ( pbPcr + MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ) );

	if ( pbKteb == NULL )
		return 0;

	return * (WORD*) ( pbKteb + MACRO_TID_FIELDOFFSET_IN_KTEB );
}

//====================================
// IsPagePresent Function Definition.
//====================================

BOOLEAN IsPagePresent( IN PVOID pvVirtAddress )
{
	DWORD*				pdwPageDir = (DWORD*) 0xC0300000;
	DWORD				dwPageDirEntry;
	DWORD*				pdwPageTables = (DWORD*) 0xC0000000;
	DWORD				dwPageTableEntry;

	// Check the Page Tables.

	dwPageDirEntry = pdwPageDir[ ( (DWORD) pvVirtAddress ) / 0x400000 ];

	if ( ( dwPageDirEntry >> 12 ) &&
		( dwPageDirEntry & 0x1 ) )
	{
		if ( dwPageDirEntry & (1<<7) )
			return TRUE;

		dwPageTableEntry = pdwPageTables[ ( (DWORD) pvVirtAddress ) / 0x1000 ];

		if ( ( dwPageTableEntry >> 12 ) &&
			( dwPageTableEntry & 0x1 ) )
				return TRUE;
	}

	// Return to the Caller.

	return FALSE;
}

//=======================================================
// InitializeMultiProcessorSpinLock Function Definition.
//=======================================================

VOID InitializeMultiProcessorSpinLock( OUT MP_SPINLOCK* pmpslSpinLock )
{
	// Initialize.

	pmpslSpinLock->state = 0;
}

//==================================================
// EnterMultiProcessorSpinLock Function Definition.
//==================================================

VOID __declspec( naked ) EnterMultiProcessorSpinLock( VOID /* "IN MP_SPINLOCK* pmpslSpinLock" in EBX Register. */ )
{
	__asm
	{
		// Enter.

		push		eax

		lea			eax, [ ebx ]MP_SPINLOCK.state

_SpinLockLoop:
		lock bts	dword ptr[ eax ], 0
		jc			_SpinLockLoop

		pop			eax

		// Return.

		ret
	}
}

//==================================================
// LeaveMultiProcessorSpinLock Function Definition.
//==================================================

VOID __declspec( naked ) LeaveMultiProcessorSpinLock( VOID /* "IN MP_SPINLOCK* pmpslSpinLock" in EBX Register. */ )
{
	__asm
	{
		// Leave.

		push		eax

		lea			eax, [ ebx ]MP_SPINLOCK.state

		mov			dword ptr[ eax ], 0

		pop			eax

		// Return.

		ret
	}
}

//====================================
// GetCurrentPID Function Definition.
//====================================

WORD GetCurrentPID( IN VOID* pvPCRAddress )
{
	BYTE*			pbPcr = (BYTE*) pvPCRAddress;
	BYTE*			pbKteb;
	BYTE*			pbKpeb;

	// Return the Required Information.

	pbKteb = (BYTE*) ( * (DWORD*) ( pbPcr + MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ) );

	if ( pbKteb == NULL )
		return 0;

	pbKpeb = (BYTE*) ( * (DWORD*) ( pbKteb + MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB ) );

	if ( pbKpeb == NULL )
		return 0;

	return (WORD) ( * (DWORD*) ( pbKpeb + MACRO_PID_FIELDOFFSET_IN_KPEB ) );
}

//====================================================
// DiscoverNtoskrnlDriverSection Function Definition.
//====================================================

static CHAR			g_szDiscoverNtoskrnlDriverSectionTempBuffer[ 2 * 1024 ]; // NOTE: sizeof < sizeof( System Page Size )

VOID* DiscoverNtoskrnlDriverSection( IN VOID* pvDriverSection )
{
	LIST_ENTRY*			pleListNodePtr;
	WORD*				pwImageNameLengthPtr;
	WORD				wImageNameLength;
	WORD*				pwImageNameUnicodePtr;
	DWORD*				pdwImageNameUnicodePtrPtr;
	WORD*				pwWordPtr;
	CHAR*				pcCharPtr;
	ULONG				ulI;

	// Do the Requested Operation.

	pleListNodePtr = (LIST_ENTRY*) pvDriverSection;

	while( TRUE )
	{
		// Get the Pointer to the Previous Node.

		if ( pleListNodePtr == NULL ||
			IsPagePresent_DWORD( (DWORD*) ( ( (BYTE*) pleListNodePtr ) + FIELD_OFFSET( LIST_ENTRY, Blink ) ) ) == FALSE )
				return NULL;

		pleListNodePtr = pleListNodePtr->Blink;

		if ( pleListNodePtr == NULL ||
			pleListNodePtr == (LIST_ENTRY*) pvDriverSection ||
			IsPagePresent( pleListNodePtr ) == FALSE )
				return NULL;

		// Get the Name of the Module.

		pwImageNameLengthPtr = (WORD*) ( ( (BYTE*) pleListNodePtr ) + MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC );

		if ( IsPagePresent_WORD( pwImageNameLengthPtr ) == FALSE )
				return NULL;

		wImageNameLength = * pwImageNameLengthPtr / sizeof( WORD );

		if ( wImageNameLength == 0 ||
			wImageNameLength > sizeof( g_szDiscoverNtoskrnlDriverSectionTempBuffer ) - 1 )
				return NULL;

		pdwImageNameUnicodePtrPtr = (DWORD*) ( ( (BYTE*) pleListNodePtr ) +
			MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC + FIELD_OFFSET( UNICODE_STRING, Buffer ) );

		if ( IsPagePresent_DWORD( pdwImageNameUnicodePtrPtr ) == FALSE )
				return NULL;

		pwImageNameUnicodePtr = (WORD*) * pdwImageNameUnicodePtrPtr;

		if ( pwImageNameUnicodePtr == NULL ||
			IsPagePresent( pwImageNameUnicodePtr ) == FALSE )
			return NULL;

		if ( IsPagePresent_WORD( pwImageNameUnicodePtr + wImageNameLength - 1 ) == FALSE )
			return NULL;

		pwWordPtr = pwImageNameUnicodePtr;
		pcCharPtr = g_szDiscoverNtoskrnlDriverSectionTempBuffer;

		for ( ulI = 0; ulI < wImageNameLength; ulI ++ )
			* pcCharPtr ++ = (CHAR) ( ( * pwWordPtr ++ ) & 0xFF );

		* pcCharPtr = '\0';

		_strupr( g_szDiscoverNtoskrnlDriverSectionTempBuffer );

		// Check for the Presence of the NTOSKRNL Module Name.

		if ( strstr( g_szDiscoverNtoskrnlDriverSectionTempBuffer, MACRO_NTOSKRNL_MODULENAME_UPPERCASE ) )
			return pleListNodePtr;
	}
}

//==========================================
// IsPagePresent_DWORD Function Definition.
//==========================================

BOOLEAN IsPagePresent_DWORD( IN DWORD* pdwDwordPtr )
{
	// Check the Pointer.

	if ( IsPagePresent( pdwDwordPtr ) == FALSE )
		return FALSE;

	if ( IsPagePresent( ( (BYTE*) pdwDwordPtr ) + sizeof( DWORD ) - 1 ) == FALSE )
		return FALSE;

	return TRUE;
}

//=========================================
// IsPagePresent_WORD Function Definition.
//=========================================

BOOLEAN IsPagePresent_WORD( IN WORD* pwWordPtr )
{
	// Check the Pointer.

	if ( IsPagePresent( pwWordPtr ) == FALSE )
		return FALSE;

	if ( IsPagePresent( ( (BYTE*) pwWordPtr ) + sizeof( WORD ) - 1 ) == FALSE )
		return FALSE;

	return TRUE;
}

//================================================
// Static "Touch" + "Present Bit Check" Routines.
//================================================

static BOOLEAN IsPagePresentAndTouch( IN PVOID pvVirtAddress, IN BOOLEAN bTouchPages )
{
	if ( bTouchPages )
		TouchPage_BYTE( (BYTE*) pvVirtAddress );
	return IsPagePresent( pvVirtAddress );
}
static BOOLEAN IsPagePresentAndTouch_WORD( IN WORD* pwWordPtr, IN BOOLEAN bTouchPages )
{
	if ( bTouchPages )
		TouchPage_BYTERANGE( (BYTE*) pwWordPtr, sizeof( WORD ) );
	return IsPagePresent_WORD( pwWordPtr );
}
static BOOLEAN IsPagePresentAndTouch_DWORD( IN DWORD* pdwDwordPtr, IN BOOLEAN bTouchPages )
{
	if ( bTouchPages )
		TouchPage_BYTERANGE( (BYTE*) pdwDwordPtr, sizeof( DWORD ) );
	return IsPagePresent_DWORD( pdwDwordPtr );
}
static BOOLEAN IsPagePresentAndTouch_BYTERANGE( IN BYTE* pbStart, IN ULONG ulLength, IN BOOLEAN bTouchPages )
{
	if ( bTouchPages )
		TouchPage_BYTERANGE( pbStart, ulLength );
	return IsPagePresent_BYTERANGE( pbStart, ulLength );
}

//======================================================
// DiscoverBytePointerPosInModules Function Definition.
//======================================================

static VOID ExamineUserModule( IN VAD* pvadMatchingNode, IN BYTE* pbBytePointer, IN BOOLEAN bTouchPages, IN ULONG ulSizeOfSzImageName, OUT VOID** P_pvMatchingModuleStart, OUT ULONG* P_ulMatchingModuleLength, OUT IMAGE_NT_HEADERS** P_pinhMatchingModuleNtHeaders, OUT CHAR* P_szImageName )
{
	DWORD				dwImageBase;
	IMAGE_DOS_HEADER*	pidhDosHdr;
	IMAGE_NT_HEADERS*	pinhNtHdrs;
	DWORD*				pdwDwordPtr;
	WORD*				pwImageNameLengthPtr;
	WORD				wImageNameLength;
	DWORD*				pdwImageNameUnicodePtrPtr;
	WORD*				pwImageNameUnicodePtr;
	WORD*				pwWordPtr;
	CHAR*				pcCharPtr;
	ULONG				ulI;

	// Check the Flags of the Matching Node.

	if ( ( pvadMatchingNode->dwFlags & 0x80000000 ) == 0 &&
		( pvadMatchingNode->dwFlags & 0x00100000 ) )
	{
		dwImageBase = ( (DWORD) pvadMatchingNode->pvStartingAddress ) << 12;

		// Check the DOS Header.

		pidhDosHdr = (IMAGE_DOS_HEADER*) dwImageBase;

		if ( IsPagePresentAndTouch_BYTERANGE( (BYTE*) pidhDosHdr, sizeof( IMAGE_DOS_HEADER ), bTouchPages ) )
		{
			if ( pidhDosHdr->e_magic == IMAGE_DOS_SIGNATURE &&
				pidhDosHdr->e_lfarlc >= 0x40 )
			{
				// Check the NT Headers.

				pinhNtHdrs = (IMAGE_NT_HEADERS*) ( dwImageBase + pidhDosHdr->e_lfanew );

				if ( IsPagePresentAndTouch_BYTERANGE( (BYTE*) pinhNtHdrs, sizeof( IMAGE_NT_HEADERS ), bTouchPages ) )
				{
					if ( pinhNtHdrs->Signature == IMAGE_NT_SIGNATURE )
					{
						if ( pbBytePointer == NULL ||
							((DWORD)pbBytePointer) <= dwImageBase + pinhNtHdrs->OptionalHeader.SizeOfImage - 1 )
						{
							// Setup the Variables required later.

							* P_pvMatchingModuleStart = (VOID*) dwImageBase;
							* P_ulMatchingModuleLength = pinhNtHdrs->OptionalHeader.SizeOfImage;
							* P_pinhMatchingModuleNtHeaders = pinhNtHdrs;

							// Get the Full Name of the Specified Module.

							if ( pvadMatchingNode->dwUndocumented_DWORD )
							{
								pdwDwordPtr = (DWORD*) ( pvadMatchingNode->dwUndocumented_DWORD + MACRO_VADTREE_UNDOCUMENTED_DISP_0 );

								if ( IsPagePresentAndTouch_DWORD( pdwDwordPtr, bTouchPages ) )
								{
									pwImageNameLengthPtr = (WORD*) ( *pdwDwordPtr + MACRO_VADTREE_UNDOCUMENTED_DISP_1 );

									// Get the Full Name of the Module.

									if ( IsPagePresentAndTouch_WORD( pwImageNameLengthPtr, bTouchPages ) )
									{
										wImageNameLength = * pwImageNameLengthPtr / sizeof( WORD );

										if ( wImageNameLength != 0 &&
											wImageNameLength <= ulSizeOfSzImageName - 1 )
										{
											pdwImageNameUnicodePtrPtr = (DWORD*) ( *pdwDwordPtr +
												MACRO_VADTREE_UNDOCUMENTED_DISP_1 + FIELD_OFFSET( UNICODE_STRING, Buffer ) );

											if ( IsPagePresentAndTouch_DWORD( pdwImageNameUnicodePtrPtr, bTouchPages ) )
											{
												pwImageNameUnicodePtr = (WORD*) * pdwImageNameUnicodePtrPtr;

												if ( pwImageNameUnicodePtr != NULL &&
													IsPagePresentAndTouch( pwImageNameUnicodePtr, bTouchPages ) &&
													IsPagePresentAndTouch_WORD( pwImageNameUnicodePtr + wImageNameLength - 1, bTouchPages ) )
												{
													pwWordPtr = pwImageNameUnicodePtr;
													pcCharPtr = P_szImageName;

													for ( ulI = 0; ulI < wImageNameLength; ulI ++ )
														* pcCharPtr ++ = (CHAR) ( ( * pwWordPtr ++ ) & 0xFF );

													* pcCharPtr = '\0';
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Return to the Caller.

	return;
}

typedef struct _VADTREEWALKFN_PARAMS
{
	/*IN*/ BYTE*						pbBytePointer;
	/*IN*/ BOOLEAN						bTouchPages;
	/*IN*/ PFNUSERMODULESNAMELISTENUM	pfnUserModListCallBack;
	/*IN*/ BOOLEAN*						pbExitNOW;
	/*IN*/ DWORD						dwUserModListCallBackPARAM;

} VADTREEWALKFN_PARAMS, *PVADTREEWALKFN_PARAMS;

static BOOLEAN InvokeUserModListCallBack( OUT VAD** ppvRetVal, IN VAD* pvadNode, IN VADTREEWALKFN_PARAMS* ppParams )
{
	CHAR				szImageName[ 256 ] = "";
	VOID*				pvMatchingModuleStart = NULL;
	ULONG				ulMatchingModuleLength = 0;
	IMAGE_NT_HEADERS*	pinhMatchingModuleNtHeaders = NULL;

	// Invoke the Callback Function.

	ExamineUserModule( pvadNode, NULL, ppParams->bTouchPages, sizeof( szImageName ),
		& pvMatchingModuleStart, & ulMatchingModuleLength, & pinhMatchingModuleNtHeaders, szImageName );

	if ( strlen( szImageName ) && pvMatchingModuleStart && ulMatchingModuleLength && pinhMatchingModuleNtHeaders )
		if ( ppParams->pfnUserModListCallBack( szImageName, pvMatchingModuleStart, ulMatchingModuleLength, pinhMatchingModuleNtHeaders, ppParams->dwUserModListCallBackPARAM ) == FALSE )
		{
			* ppParams->pbExitNOW = TRUE;
			* ppvRetVal = NULL;
			return TRUE;
		}

	return FALSE;
}

static VAD* VadTreeWalk( IN VAD* pvadNode, IN VADTREEWALKFN_PARAMS* ppParams )
{
	VAD*		pvadTemp;
	DWORD		dwStart, dwEnd;

	// Check this Node.

	if ( pvadNode == NULL ||
		IsPagePresentAndTouch_BYTERANGE( (BYTE*) pvadNode, sizeof( VAD ), ppParams->bTouchPages ) == FALSE )
			return NULL;

	pvadTemp = VadTreeWalk( pvadNode->pvadLeftLink, ppParams );
	if ( pvadTemp )
		return pvadTemp;

	dwStart = ( (DWORD) pvadNode->pvStartingAddress ) << 12;
	dwEnd = ( ( ( (DWORD) pvadNode->pvEndingAddress ) + 1 ) << 12 ) - 1;

	if ( ppParams->pbBytePointer &&
		( (DWORD) ppParams->pbBytePointer ) >= dwStart &&
		( (DWORD) ppParams->pbBytePointer ) <= dwEnd )
	{
		return pvadNode;
	}
	else if( ppParams->pfnUserModListCallBack )
	{
		if ( * ppParams->pbExitNOW )
			return NULL;

		if ( InvokeUserModListCallBack( & pvadTemp, pvadNode, ppParams ) )
			return pvadTemp;
	}

	pvadTemp = VadTreeWalk( pvadNode->pvadRightLink, ppParams );
	if ( pvadTemp )
		return pvadTemp;

	return NULL;
}

VOID DiscoverBytePointerPosInModules( OUT CHAR* pszOutputBuffer, IN BYTE* pbBytePointer, IN VOID* pvNtoskrnlDriverSection, IN VOID* pvMmUserProbeAddress, IN VOID* pvPCRAddress, IN OUT DISCVBPTR_ADDPARAMS* pdbpAddParams )
{
	VOID*				pvMatchingModuleStart;
	ULONG				ulMatchingModuleLength;
	IMAGE_NT_HEADERS*	pinhMatchingModuleNtHeaders;

	WORD*				pwImageNameLengthPtr;
	WORD				wImageNameLength;
	WORD*				pwWordPtr;
	CHAR*				pcCharPtr;
	ULONG				ulI;
	WORD*				pwImageNameUnicodePtr;
	DWORD*				pdwImageNameUnicodePtrPtr;
	DWORD				dwImageBase;
	IMAGE_DOS_HEADER*	pidhDosHdr;
	IMAGE_NT_HEADERS*	pinhNtHdrs;
	BOOLEAN				bTouchPages = FALSE;
	CHAR				szSectionName[ 64 ];
	CHAR				szImageName[ 256 ];

	BOOLEAN				bVadWalkExit;

	VADTREEWALKFN_PARAMS	vtwfpWalkParams;

	if ( pdbpAddParams && pdbpAddParams->bTouchPages )
		bTouchPages = TRUE;

	if ( pszOutputBuffer )
		strcpy( pszOutputBuffer, "" );

	if ( pdbpAddParams )
	{
		pdbpAddParams->bIsBytePointerAtTheBeginning = FALSE;
	}

	// Check whether the Pointer refers to User Memory or Kernel Memory.

	pvMatchingModuleStart = NULL;
	ulMatchingModuleLength = 0;
	pinhMatchingModuleNtHeaders = NULL;

	strcpy( szImageName, "" );

	if ( (DWORD) pbBytePointer < (DWORD) pvMmUserProbeAddress )
	{
		BYTE*			pbPcr = (BYTE*) pvPCRAddress;
		BYTE*			pbKteb;
		BYTE*			pbKpeb;
		VAD*			pvadVadRoot;
		VAD*			pvadMatchingNode;
		DWORD			dwDword;

		//
		// USER SPACE.
		//

		// Get the Root Address of the VAT.

		pbKteb = (BYTE*) ( * (DWORD*) ( pbPcr + MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ) );

		if ( pbKteb )
		{
			if ( pdbpAddParams && pdbpAddParams->pbKpeb )
				pbKpeb = pdbpAddParams->pbKpeb;
			else
				pbKpeb = (BYTE*) ( * (DWORD*) ( pbKteb + MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB ) );

			if ( pbKpeb )
			{
				pvadVadRoot = (VAD*) ( * (DWORD*) ( pbKpeb + MACRO_VADROOT_FIELDOFFSET_IN_KPEB ) );

				if ( pvadVadRoot )
				{
					if ( IsPagePresentAndTouch_BYTERANGE( (BYTE*) pvadVadRoot, sizeof( VAD ), bTouchPages ) )
					{
						bVadWalkExit = FALSE;

						vtwfpWalkParams.pbBytePointer = pbBytePointer;
						vtwfpWalkParams.bTouchPages = bTouchPages;
						vtwfpWalkParams.pfnUserModListCallBack = pdbpAddParams && pdbpAddParams->pfnUserModListCallBack ? pdbpAddParams->pfnUserModListCallBack : NULL;
						vtwfpWalkParams.pbExitNOW = & bVadWalkExit;
						vtwfpWalkParams.dwUserModListCallBackPARAM = pdbpAddParams ? pdbpAddParams->dwUserModListCallBackPARAM : 0;

						pvadMatchingNode = VadTreeWalk( pvadVadRoot, & vtwfpWalkParams );

						if ( pvadMatchingNode )
							ExamineUserModule( pvadMatchingNode, pbBytePointer, bTouchPages, sizeof( szImageName ),
								& pvMatchingModuleStart, & ulMatchingModuleLength, & pinhMatchingModuleNtHeaders, szImageName );
					}
				}
			}
		}
	}
	else
	{
		LIST_ENTRY*			pleListNodePtr;
		DWORD*				pdwImageBasePtr;

		//
		// KERNEL SPACE.
		//

		pleListNodePtr = (LIST_ENTRY*) pvNtoskrnlDriverSection;

		while( TRUE )
		{
			if ( pleListNodePtr == NULL )
				break;

			// Check the Module Start Address.

			pdwImageBasePtr = (DWORD*) ( ( (BYTE*) pleListNodePtr ) + MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC );

			if ( IsPagePresentAndTouch_DWORD( pdwImageBasePtr, bTouchPages ) )
			{
				dwImageBase = * pdwImageBasePtr;

				// Compare the Image Base and the Specified Pointer.

				if ( ( (DWORD) pbBytePointer ) >= dwImageBase )
				{
					// Check the DOS Header.

					pidhDosHdr = (IMAGE_DOS_HEADER*) dwImageBase;

					if ( IsPagePresentAndTouch_BYTERANGE( (BYTE*) pidhDosHdr, sizeof( IMAGE_DOS_HEADER ), bTouchPages ) )
					{
						if ( pidhDosHdr->e_magic == IMAGE_DOS_SIGNATURE &&
							pidhDosHdr->e_lfarlc >= 0x40 )
						{
							// Check the NT Headers.

							pinhNtHdrs = (IMAGE_NT_HEADERS*) ( dwImageBase + pidhDosHdr->e_lfanew );

							if ( IsPagePresentAndTouch_BYTERANGE( (BYTE*) pinhNtHdrs, sizeof( IMAGE_NT_HEADERS ), bTouchPages ) )
							{
								if ( pinhNtHdrs->Signature == IMAGE_NT_SIGNATURE )
								{
									if ( ( (DWORD) pbBytePointer ) <= dwImageBase + pinhNtHdrs->OptionalHeader.SizeOfImage - 1 )
									{
										// Setup the Variables required later.

										pvMatchingModuleStart = (VOID*) dwImageBase;
										ulMatchingModuleLength = pinhNtHdrs->OptionalHeader.SizeOfImage;
										pinhMatchingModuleNtHeaders = pinhNtHdrs;

										// Get the Name of the Module.

										pwImageNameLengthPtr = (WORD*) ( ( (BYTE*) pleListNodePtr ) + MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC );

										if ( IsPagePresentAndTouch_WORD( pwImageNameLengthPtr, bTouchPages ) )
										{
											wImageNameLength = * pwImageNameLengthPtr / sizeof( WORD );

											if ( wImageNameLength != 0 &&
												wImageNameLength <= sizeof( szImageName ) - 1 )
											{
												pdwImageNameUnicodePtrPtr = (DWORD*) ( ( (BYTE*) pleListNodePtr ) +
													MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC + FIELD_OFFSET( UNICODE_STRING, Buffer ) );

												if ( IsPagePresentAndTouch_DWORD( pdwImageNameUnicodePtrPtr, bTouchPages ) )
												{
													pwImageNameUnicodePtr = (WORD*) * pdwImageNameUnicodePtrPtr;

													if ( pwImageNameUnicodePtr != NULL &&
														IsPagePresentAndTouch( pwImageNameUnicodePtr, bTouchPages ) &&
														IsPagePresentAndTouch_WORD( pwImageNameUnicodePtr + wImageNameLength - 1, bTouchPages ) )
													{
														pwWordPtr = pwImageNameUnicodePtr;
														pcCharPtr = szImageName;

														for ( ulI = 0; ulI < wImageNameLength; ulI ++ )
															* pcCharPtr ++ = (CHAR) ( ( * pwWordPtr ++ ) & 0xFF );

														* pcCharPtr = '\0';
													}
												}
											}
										}

										// Exit from the Loop.

										break;
									}
								}
							}
						}
					}
				}
			}

			// Get the Pointer to the Next Node.

			if ( IsPagePresentAndTouch_DWORD( (DWORD*) ( ( (BYTE*) pleListNodePtr ) + FIELD_OFFSET( LIST_ENTRY, Flink ) ), bTouchPages ) == FALSE )
				break;

			pleListNodePtr = pleListNodePtr->Flink;

			if ( pleListNodePtr == NULL ||
				pleListNodePtr == (LIST_ENTRY*) pvNtoskrnlDriverSection ||
				IsPagePresentAndTouch( pleListNodePtr, bTouchPages ) == FALSE )
					break;
		}
	}

	// Check whether there is a Match.

	if ( pszOutputBuffer &&
		pvMatchingModuleStart && ulMatchingModuleLength && pinhMatchingModuleNtHeaders &&
		strlen( szImageName ) )
	{
		IMAGE_SECTION_HEADER*		pishSectionList;
		ULONG						ulI;
		IMAGE_SECTION_HEADER*		pishThis;
		DWORD						dwStart, dwEnd, dwOffset;
		CHAR*						pszImageNameToBePrinted;
		BOOLEAN						bPreserveModuleFileExtension;

		if ( pdbpAddParams == NULL || pdbpAddParams->bPreserveModuleFileExtension == FALSE )
			bPreserveModuleFileExtension = FALSE;
		else
			bPreserveModuleFileExtension = TRUE;

		// Set the Pointer to the Image Name String.

		pszImageNameToBePrinted = & szImageName[ strlen( szImageName ) - 1 ];

		for( ; pszImageNameToBePrinted >= szImageName; pszImageNameToBePrinted -- )
			if ( * pszImageNameToBePrinted == '.' && bPreserveModuleFileExtension == FALSE )
				* pszImageNameToBePrinted = '\0';
			else if ( * pszImageNameToBePrinted == '\\' ||
				* pszImageNameToBePrinted == '/' )
					break;

		pszImageNameToBePrinted ++;

		if ( strlen( pszImageNameToBePrinted ) )
		{
			// Check the Sections of the Image.

			pishSectionList = (IMAGE_SECTION_HEADER*) ( ( (BYTE*) pinhMatchingModuleNtHeaders ) + sizeof( IMAGE_NT_HEADERS ) );

			if ( IsPagePresentAndTouch_BYTERANGE( (BYTE*) pishSectionList, pinhMatchingModuleNtHeaders->FileHeader.NumberOfSections * sizeof( IMAGE_SECTION_HEADER ), bTouchPages ) )
			{
				// Iterate through the Section List Items.

				if ( !( pdbpAddParams && pdbpAddParams->ulSplitInfoNoSectionStringMaxLen ) )
				{
					for ( ulI = 0; ulI < pinhMatchingModuleNtHeaders->FileHeader.NumberOfSections; ulI ++ )
					{
						pishThis = & pishSectionList[ ulI ];

						if ( pishThis->SizeOfRawData == 0 )
							continue;

						dwStart = ( (DWORD) pvMatchingModuleStart ) + pishThis->VirtualAddress;
						dwEnd = dwStart + pishThis->SizeOfRawData - 1;

						if ( ( (DWORD) pbBytePointer ) >= dwStart &&
							( (DWORD) pbBytePointer ) <= dwEnd )
						{
							dwOffset = ( (DWORD) pbBytePointer ) - dwStart;

							// Compose the Required String.

							memset( szSectionName, 0, sizeof( szSectionName ) );
							memcpy( szSectionName, pishThis->Name, IMAGE_SIZEOF_SHORT_NAME );

							if ( dwOffset )
							{
								sprintf( pszOutputBuffer, "%s!%s+%.8X",
									pszImageNameToBePrinted, szSectionName, dwOffset );
							}
							else
							{
								sprintf( pszOutputBuffer, "%s!%s",
									pszImageNameToBePrinted, szSectionName );
							}

							// Return to the Caller.

							return;
						}
					}
				}

				// Compose the Required String without Section Information.

				dwOffset = ( (DWORD) pbBytePointer ) - ( (DWORD) pvMatchingModuleStart );

				if ( dwOffset &&
					!( pdbpAddParams && pdbpAddParams->ulSplitInfoNoSectionStringMaxLen ) )
				{
					sprintf( pszOutputBuffer, "%s+%.8X",
						pszImageNameToBePrinted, dwOffset );
				}
				else
				{
					sprintf( pszOutputBuffer, "%s",
						pszImageNameToBePrinted );

					if ( pdbpAddParams )
						pdbpAddParams->bIsBytePointerAtTheBeginning = TRUE;
				}

				if ( pdbpAddParams && pdbpAddParams->ulSplitInfoNoSectionStringMaxLen )
				{
					pdbpAddParams->dwSplittedOffsetNoSection = dwOffset;
					pszOutputBuffer[ pdbpAddParams->ulSplitInfoNoSectionStringMaxLen ] = '\0';
				}
			}
		}
	}

	// Return to the Caller.

	return;
}

//==============================================
// IsPagePresent_BYTERANGE Function Definition.
//==============================================

BOOLEAN IsPagePresent_BYTERANGE( IN BYTE* pbStart, IN ULONG ulLength )
{
	BYTE*		pbPageStart;
	BYTE*		pbPageEnd;

	if ( ulLength == 0 )
		return TRUE;

	// Check the Pages.

	pbPageStart = (BYTE*) ( ( (DWORD) pbStart ) & MACRO_SYSTEM_PAGE_MASK );
	pbPageEnd = (BYTE*) ( ( ( (DWORD) pbStart ) + ulLength - 1 ) & MACRO_SYSTEM_PAGE_MASK );

	for ( ; pbPageStart <= pbPageEnd; pbPageStart += MACRO_SYSTEM_PAGE_SIZE )
		if ( IsPagePresent( pbPageStart ) == FALSE )
			return FALSE;

	return TRUE;
}

//==========================================================
// Guess_MiMapViewOfImageSection_FnPtr Function Definition.
//==========================================================

PVOID Guess_MiMapViewOfImageSection_FnPtr( VOID* pvNtoskrnlDriverSection )
{
	BYTE*		pbPtr = (BYTE*) & MmIsThisAnNtAsSystem;
	VOID*		pvRetVal = NULL;
	ULONG		ulI;

	// We have already the ptr to the fn ??

	if ( MACRO_MAPVIEWOFIMAGESECTION_ADDR && pvNtoskrnlDriverSection )
		return (VOID*) ( MACRO_MAPVIEWOFIMAGESECTION_ADDR + * (DWORD*) ((BYTE*)pvNtoskrnlDriverSection+MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC) );

	// Search for the Beginning of the Function.

	__try
	{
		for ( ulI = 0; ulI < MACRO_MAPVIEWOFIMAGESECTION_FNSRCH_RANGE; ulI ++, pbPtr ++ )
		{
			if ( * pbPtr == MACRO_MAPVIEWOFIMAGESECTION_FNSRCH_1STBYTE )
				if ( * (DWORD*) ( pbPtr + 1 ) == MACRO_MAPVIEWOFIMAGESECTION_FNSRCH_2NDDWORD )
				{
					pvRetVal = (VOID*) pbPtr;
					break;
				}
		}
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{ }

	return pvRetVal;
}

//============================================================
// Guess_MiUnMapViewOfImageSection_FnPtr Function Definition.
//============================================================

PVOID Guess_MiUnMapViewOfImageSection_FnPtr( VOID )
{
	return (VOID*) & MmUnmapViewOfSection;
}

//=====================================
// GetCurrentKPEB Function Definition.
//=====================================

PVOID GetCurrentKPEB( IN VOID* pvPCRAddress )
{
	BYTE*			pbPcr = (BYTE*) pvPCRAddress;
	BYTE*			pbKteb;
	BYTE*			pbKpeb;

	// Return the Required Information.

	pbKteb = (BYTE*) ( * (DWORD*) ( pbPcr + MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ) );

	if ( pbKteb == NULL )
		return NULL;

	pbKpeb = (BYTE*) ( * (DWORD*) ( pbKteb + MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB ) );

	if ( pbKpeb == NULL )
		return NULL;

	return (PVOID) pbKpeb;
}

//============================================
// TouchPage_BYTE Function Definition.
//============================================

VOID TouchPage_BYTE( IN BYTE* pbPtr )
{
	// Touch the Page.

	__asm
	{
		mov			eax, pbPtr
		mov			al, byte ptr[ eax ]
	}

	// Return to the Caller.

	return;
}

//=================================================
// TouchPage_BYTERANGE Function Definition.
//=================================================

VOID TouchPage_BYTERANGE( IN BYTE* pbStart, IN ULONG ulLength )
{
	BYTE*		pbPageStart;
	BYTE*		pbPageEnd;

	if ( ulLength == 0 )
		return;

	// Touch the Pages.

	pbPageStart = (BYTE*) ( ( (DWORD) pbStart ) & MACRO_SYSTEM_PAGE_MASK );
	pbPageEnd = (BYTE*) ( ( ( (DWORD) pbStart ) + ulLength - 1 ) & MACRO_SYSTEM_PAGE_MASK );

	for ( ; pbPageStart <= pbPageEnd; pbPageStart += MACRO_SYSTEM_PAGE_SIZE )
		TouchPage_BYTE( pbPageStart );

	// Return to the Caller.

	return;
}

//=====================================================
// Get_NtTerminateProcess_FnIndex Function Definition.
//=====================================================

ULONG Get_NtTerminateProcess_FnIndex( VOID )
{
	BYTE*		pbPtr = (BYTE*) & ZwTerminateProcess;
	ULONG		ulRetVal = MACRO_GETNTTERMINATEPROCESSFNINDEX_ERR;

	// We have already the ptr to the fn ??

	if ( MACRO_NTTERMINATEPROCESS_FNINDEX )
		return MACRO_NTTERMINATEPROCESS_FNINDEX;

	// Try to Acquire the Function Index in the Service Table.

	__try
	{
		if ( * pbPtr != 0xB8 /* MOV EAX, IMM32 */ )
		{
			if ( * (WORD*) pbPtr == 0x25FF /* JMP DWORD PTR */ )
			{
				pbPtr = (BYTE*) * (DWORD*) * (DWORD*) ( pbPtr + 2 );

				if ( * pbPtr != 0xB8 /* MOV EAX, IMM32 */ )
					return ulRetVal;
				else
					pbPtr ++;
			}
			else
				return ulRetVal;
		}
		else
			pbPtr ++;

		ulRetVal = * (ULONG*) pbPtr;

		if ( ulRetVal > 0xFFFF )
			ulRetVal = MACRO_GETNTTERMINATEPROCESSFNINDEX_ERR;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{ }

	// Return to the Caller.

	return ulRetVal;
}

//====================================================
// IterateThroughListOfProcesses Function Definition.
//====================================================

VOID IterateThroughListOfProcesses( IN PROCESSLIST_INFO* pliProcessListInfo, IN PFNPROCESSESLISTENUM pfnProcessListEnumFn, IN DWORD dwParam )
{
	LIST_ENTRY*		plePtr;
	BYTE*			pbKpeb;

	// Iterate through the Active Process List.

	if ( IsPagePresent_BYTERANGE( (BYTE*) pliProcessListInfo->pleProcessListHead, sizeof( LIST_ENTRY ) ) == FALSE )
		return;

	plePtr = pliProcessListInfo->pleProcessListHead->Flink;

	while( plePtr != pliProcessListInfo->pleProcessListHead )
	{
		if ( IsPagePresent_BYTERANGE( (BYTE*) plePtr, sizeof( LIST_ENTRY ) ) == FALSE )
			return;

		pbKpeb = (BYTE*) plePtr - MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB;

		if ( IsPagePresent( (VOID*) pbKpeb ) )
			if ( pfnProcessListEnumFn( pbKpeb, dwParam ) == FALSE )
				return;

		plePtr = plePtr->Flink;
	}

	// Call the Function also for the Idle Process.

	pbKpeb = pliProcessListInfo->pbIdleProcess;

	if ( IsPagePresent( (VOID*) pbKpeb ) )
		pfnProcessListEnumFn( pbKpeb, dwParam );

	// Return.

	return;
}

//=========================================
// GetProcessListInfo Function Definition.
//=========================================

BOOLEAN GetProcessListInfo( OUT PROCESSLIST_INFO* pliProcessListInfo )
{
	BOOLEAN			bRetVal = TRUE;
	DWORD*			pdwIdleProcess;
	CHAR*			pszIdleProcessName;
	DWORD*			pInitSysProcPtr;

	memset( pliProcessListInfo, 0, sizeof( PROCESSLIST_INFO ) );

	// Try to determine the Address of the Idle Process Block.

	pInitSysProcPtr = (DWORD*) & PsInitialSystemProcess;

	__try
	{
		// Idle Process Test.

		pdwIdleProcess = (DWORD*) ( (DWORD) pInitSysProcPtr + MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP );
		pszIdleProcessName = (CHAR*) ( * pdwIdleProcess + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );

		if ( memcmp( pszIdleProcessName, MACRO_IDLEPROCESS_IMAGEFILENAME, strlen( MACRO_IDLEPROCESS_IMAGEFILENAME ) ) )
			return FALSE;

		// Save the Informations.

		pliProcessListInfo->pbIdleProcess = (BYTE*) * pdwIdleProcess;
		pliProcessListInfo->pleProcessListHead = (LIST_ENTRY*) * (DWORD*)
			( (BYTE*) *pInitSysProcPtr + MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB + FIELD_OFFSET( LIST_ENTRY, Blink ) );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		memset( pliProcessListInfo, 0, sizeof( PROCESSLIST_INFO ) );
		bRetVal = FALSE;
	}

	// Return.

	return bRetVal;
}

//==========================================
// VpcICEAttachProcess Function Definition.
//==========================================

DWORD VpcICEAttachProcess( IN BYTE* pbKpeb )
{
	DWORD			dwCurrCR3, dwNextCR3;
	DWORD			dwCr3FieldOffsetInKPEB;

	dwCr3FieldOffsetInKPEB = MACRO_CR3_FIELDOFFSET_IN_KPEB;

	// Read the Values of the CR3 Register.

	__asm
	{
		// ... current value ...
		mov			eax, cr3
		mov			dwCurrCR3, eax

		// ... next value ...
		mov			eax, pbKpeb
		add			eax, dwCr3FieldOffsetInKPEB
		mov			eax, dword ptr[ eax ]
		mov			dwNextCR3, eax
	}

	// Check whether we have to Exit.

	if ( dwCurrCR3 == dwNextCR3 )
		return 0;

	// Attach to the Process.

	__asm
	{
		// ... change the value ...
		mov			eax, dwNextCR3
		mov			cr3, eax
	}

	// Return the Old Value.

	return dwCurrCR3;
}

//==========================================
// VpcICEDetachProcess Function Definition.
//==========================================

VOID VpcICEDetachProcess( IN DWORD dwCookie )
{
	// Check whether the Detach is necessary.

	if ( dwCookie == 0 )
		return;

	// Detach from the Process.

	__asm
	{
		mov			eax, dwCookie
		mov			cr3, eax
	}

	// Return.

	return;
}

//======================================
// RebootSystemNOW Function Definition.
//======================================

VOID __declspec( naked ) RebootSystemNOW( VOID )
{
	// Use various tricks to cause a System Reboot...

	__asm
	{
		cli

		in			al, 0x61

		mov			ecx, 0x100
rsnLabel1:
		loop		rsnLabel1

		or			al, 0xC

		out			0x61, al

		mov			ecx, 0x100
rsnLabel2:
		loop		rsnLabel2

		mov			al, 0xFE
		out			0x64, al

		mov			ecx, 0x1000000
rsnLabel3:
		loop		rsnLabel3

		mov			ax, 8
		mov			ebx, offset RebootSystemNOW

		lgdt		[ ebx ]
		mov			ds, ax

		hlt
	}
}

//=============================================================
// EnterMultiProcessorSpinLockAndTestByte Function Definition.
//=============================================================

VOID __declspec( naked ) EnterMultiProcessorSpinLockAndTestByte( VOID /* "IN MP_SPINLOCK* pmpslSpinLock" in EBX Register, "IN BYTE* pbVarToWatchPtr" in ECX Register. Returns TRUE if *pbVarToWatchPtr != 0. */ )
{
	__asm
	{
		// Enter.

		lea			eax, [ ebx ]MP_SPINLOCK.state

_SpinLockLoop:

		cmp			byte ptr[ ecx ], 0
		je			_TestedByteIsZero

		// Return TRUE.

		mov			eax, TRUE
		ret

_TestedByteIsZero:

		lock bts	dword ptr[ eax ], 0
		jc			_SpinLockLoop

		// Return FALSE.

		mov			eax, FALSE
		ret
	}
}

//=================================================
// LinearAddressToPhysAddress Function Definition.
//=================================================

NTSTATUS LinearAddressToPhysAddress( OUT DWORD* pdwPhysAddress, IN DWORD dwLinearAddress ) /* WARNING: The page has to be PRESENT. */
{
	DWORD*				pdwPageDir = (DWORD*) 0xC0300000;
	DWORD				dwPageDirEntry;
	DWORD*				pdwPageTables = (DWORD*) 0xC0000000;
	DWORD				dwPageTableEntry;

	* pdwPhysAddress = 0;

	// Check in the Page Directory.

	dwPageDirEntry = pdwPageDir[ dwLinearAddress / 0x400000 ];

	if ( ( dwPageDirEntry >> 12 ) &&
		( dwPageDirEntry & 0x1 ) )
	{
		if ( dwPageDirEntry & (1<<7) ) // 4Mb Page.
		{
			* pdwPhysAddress = ( dwPageDirEntry & 0xFFC00000 ) + ( dwLinearAddress & 0x3FFFFF );
			return STATUS_SUCCESS;
		}

		dwPageTableEntry = pdwPageTables[ dwLinearAddress / 0x1000 ];

		if ( ( dwPageTableEntry >> 12 ) &&
			( dwPageTableEntry & 0x1 ) )
		{
			* pdwPhysAddress = ( dwPageTableEntry & 0xFFFFF000 ) + ( dwLinearAddress & 0xFFF ); // 4Kb Page.
			return STATUS_SUCCESS;
		}
	}

	return STATUS_UNSUCCESSFUL;
}

//===========================================
// BugChecker_ReadTimeStampCounter Function Definition.
//===========================================

VOID BugChecker_ReadTimeStampCounter( OUT LARGE_INTEGER* pliOutput )
{
	// Read the CPU Counter.

	__asm
	{
		rdtsc

		mov			ecx, pliOutput
		mov			[ ecx ], eax
		mov			[ ecx + 4 ], edx
	}

	// Return.

	return;
}

//=====================================================
// MapPhysPageInDebuggerHyperSlot Function Definition.
//=====================================================

static BYTE		g_vbDebuggerHyperSlot[ MACRO_SYSTEM_PAGE_SIZE * 3 ];

VOID* MapPhysPageInDebuggerHyperSlot( IN DWORD dwPhysAddress, OUT DWORD* pdwOldPTE )
{
	DWORD			dwHyperSlotLinearAddress;
	DWORD*			pdwPageDir = (DWORD*) 0xC0300000;
	DWORD			dwPageDirEntry;
	DWORD*			pdwPageTables = (DWORD*) 0xC0000000;
	DWORD			dwPageTableEntry;

	if ( pdwOldPTE )
		* pdwOldPTE = 0;

	//
	// Map the Physical Page in the Debugger Hyper Slot.
	//

	dwHyperSlotLinearAddress =
		( ((DWORD)&g_vbDebuggerHyperSlot[0]) + MACRO_SYSTEM_PAGE_SIZE ) & MACRO_SYSTEM_PAGE_MASK;

	dwPageDirEntry = pdwPageDir[ dwHyperSlotLinearAddress / 0x400000 ];

	if ( ( dwPageDirEntry >> 12 ) &&
		( dwPageDirEntry & 0x1 ) )
	{
		if ( dwPageDirEntry & (1<<7) ) // 4Mb Page.
			return NULL; // This should Never happen!

		dwPageTableEntry = pdwPageTables[ dwHyperSlotLinearAddress / 0x1000 ];
		if ( ( dwPageTableEntry >> 12 ) &&
			( dwPageTableEntry & 0x1 ) )
		{
			//
			// Modify the PTE, Invalidate the TLB and Return.
			//

			if ( pdwOldPTE )
				* pdwOldPTE = dwPageTableEntry;

			dwPageTableEntry = ( dwPhysAddress & 0xFFFFF000 ) | ( dwPageTableEntry & 0xFFF );

			pdwPageTables[ dwHyperSlotLinearAddress / 0x1000 ] = dwPageTableEntry;

			__asm
			{
				mov				eax, dwHyperSlotLinearAddress
				invlpg			[ eax ]
			}

			return (PVOID) dwHyperSlotLinearAddress;
		}
	}

	return NULL;
}

//====================================================
// RestoreDebuggerHyperSlotState Function Definition.
//====================================================

VOID RestoreDebuggerHyperSlotState( IN DWORD dwPTE )
{
	DWORD			dwHyperSlotLinearAddress;
	DWORD*			pdwPageDir = (DWORD*) 0xC0300000;
	DWORD			dwPageDirEntry;
	DWORD*			pdwPageTables = (DWORD*) 0xC0000000;
	DWORD			dwPageTableEntry;

	if ( dwPTE == 0 )
		return;

	//
	// Restore the PTE.
	//

	dwHyperSlotLinearAddress =
		( ((DWORD)&g_vbDebuggerHyperSlot[0]) + MACRO_SYSTEM_PAGE_SIZE ) & MACRO_SYSTEM_PAGE_MASK;

	dwPageDirEntry = pdwPageDir[ dwHyperSlotLinearAddress / 0x400000 ];

	if ( ( dwPageDirEntry >> 12 ) &&
		( dwPageDirEntry & 0x1 ) )
	{
		if ( dwPageDirEntry & (1<<7) ) // 4Mb Page.
			return; // This should Never happen!

		dwPageTableEntry = pdwPageTables[ dwHyperSlotLinearAddress / 0x1000 ];
		if ( ( dwPageTableEntry >> 12 ) &&
			( dwPageTableEntry & 0x1 ) )
		{
			//
			// Modify the PTE and Invalidate the TLB.
			//

			pdwPageTables[ dwHyperSlotLinearAddress / 0x1000 ] = dwPTE;

			__asm
			{
				mov				eax, dwHyperSlotLinearAddress
				invlpg			[ eax ]
			}
		}
	}

	return;
}

//====================================
// WordStringLen Function Definition.
//====================================

ULONG WordStringLen( IN WORD* pwStr )
{
	WORD*		pwPtr = pwStr;
	ULONG		ulC = 0;

	while( * pwPtr ++ )
		++ ulC;

	return ulC;
}

//=================================
// ultobinstr Function Definition.
//=================================

CHAR* ultobinstr( OUT CHAR* pszOutput, IN ULONG ulInput )
{
	ULONG			ulI, ulJ;
	CHAR*			pszPtr = pszOutput;

	//
	// Convert the ULong to a Binary Rep.
	//

	for ( ulI = 31; ulI < 0x80000000; ulI -- )
		if ( ( ulInput >> ulI ) & 1 )
			break;

	if ( ulI > 0x80000000 )
		ulI = 0;

	for ( ulJ = ulI; ulJ < 0x80000000; ulJ -- )
		if ( ( ulInput >> ulJ ) & 1 )
			* pszPtr ++ = '1';
		else
			* pszPtr ++ = '0';

	* pszPtr = 0;

	return pszOutput;
}

//========================================
// GetLastBranchMSRs Function Definition.
//========================================

BOOLEAN GetLastBranchMSRs( OUT DWORD* pdwFromIP, OUT DWORD* pdwToIP )
{
	DWORD			__EAX__;

	// Get the Information and Reset the System Bit.

	if ( AreMSRsAvailable() == FALSE )
		return FALSE;

	__asm
	{
		mov			ecx, 0x1DB		// LASTBRANCHFROMIP MSR
		rdmsr
		mov			__EAX__, eax
	}

	* pdwFromIP = __EAX__;

	__asm
	{
		mov			ecx, 0x1DC		// LASTBRANCHTOIP MSR
		rdmsr
		mov			__EAX__, eax
	}

	* pdwToIP = __EAX__;

	return TRUE;
}

//=======================================
// AreMSRsAvailable Function Definition.
//=======================================

BOOLEAN AreMSRsAvailable( VOID )
{
	DWORD			__EDX__;

	// Check the Features.

	__asm
	{
		mov			eax, 0x1
		cpuid
		mov			__EDX__, edx
	}

	if ( __EDX__ & (1<<5) )
		return TRUE;
	else
		return FALSE;
}

//===========================================
// EnableLastBranchMSRs Function Definition.
//===========================================

VOID EnableLastBranchMSRs( VOID )
{
	//
	// Enable the MSRs.
	//

	if ( AreMSRsAvailable() == FALSE )
		return;

	__asm
	{
		mov			ecx, 0x1D9		// DEBUGCTL MSR
		rdmsr

		or			eax, 0x1		// LBR

		mov			ecx, 0x1D9		// DEBUGCTL MSR
		wrmsr
	}

	return;
}

//======================================================
// InitializeProcessorForDebugging Function Definition.
//======================================================

VOID InitializeProcessorForDebugging( VOID )
{
	//
	// Reset the DR6 Register, Enable the Debug Extensions, Enable the Branch MSRs.
	//

	__asm
	{
		__asm _emit 0x0F __asm _emit 0x20 __asm _emit 0xE0 /* mov eax, CR4 */
		or			eax, (1<<3) // DE bit: Debug Extensions.
		__asm _emit 0x0F __asm _emit 0x22 __asm _emit 0xE0 /* mov CR4, eax */

		mov			eax, dr6
		and			eax, ~( (1<<14) | 0xF ) // Single Step Bit + B0-B3 bits of DR6.
		mov			dr6, eax
	}

	EnableLastBranchMSRs();

	return;
}

//=====================================
// IsInsideVmware Function Definition.
//=====================================

#define VMWARE_HYPERVISOR_MAGIC			0x564D5868
#define VMWARE_HYPERVISOR_PORT			0x5658
#define VMWARE_PORT_CMD_GETVERSION		10

DWORD IsInsideVmware ( VOID )
{
	DWORD			dwRetVal = 0;
	DWORD			eaxVal = 0, ebxVal = 0, ecxVal = 0, edxVal = 0;

	__asm
	{
		push	eax
		push	ebx
		push	ecx
		push	edx

		mov		eax, VMWARE_HYPERVISOR_MAGIC
		xor		ebx, ebx
		mov		ecx, VMWARE_PORT_CMD_GETVERSION
		mov		edx, VMWARE_HYPERVISOR_PORT

		in		eax, dx

		mov		eaxVal, eax
		mov		ebxVal, ebx
		mov		ecxVal, ecx
		mov		edxVal, edx

		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
	}

	if ( ebxVal == VMWARE_HYPERVISOR_MAGIC )
		dwRetVal = eaxVal; // VMWare Version.

	return dwRetVal;
}
