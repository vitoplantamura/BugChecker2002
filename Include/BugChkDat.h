//
// BugChkDat.h, Copyright(c) 2003, Vito Plantamura. All rights reserved.
//

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

//
// Header Files.
//

#include <windows.h>
#include "..\Include\StructuredFileUtils.h"
#include "..\Include\BugChkDatDefs.h"

//
// Types.
//

typedef struct // memset to 0 !
{
	int MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP		; // = 0 ; // default value = ( - 0x8 ) for Windows 2000 SP4
	DWORD MACRO_MAPVIEWOFIMAGESECTION_ADDR			; // = 0;

	int  MACRO_KTEBPTR_FIELDOFFSET_IN_PCR			; // = 0 ; // default value = ( 0x124 ) for Windows 2000 SP4
	int  MACRO_CURRIRQL_FIELDOFFSET_IN_PCR			; // = 0 ; // default value = ( 0x24 ) for Windows 2000 SP4

	int  MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB			; // = 0 ; // default value = ( 0x44 ) for Windows 2000 SP4
	int  MACRO_TID_FIELDOFFSET_IN_KTEB				; // = 0 ; // default value = ( 0x1E4 ) for Windows 2000 SP4

	int  MACRO_PID_FIELDOFFSET_IN_KPEB				; // = 0 ; // default value = ( 0x9C ) for Windows 2000 SP4
	int  MACRO_VADROOT_FIELDOFFSET_IN_KPEB			; // = 0 ; // default value = ( 0x194 ) for Windows 2000 SP4
	int  MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB	; // = 0 ; // default value = ( 0xA0 ) for Windows 2000 SP4
	int  MACRO_CR3_FIELDOFFSET_IN_KPEB				; // = 0 ; // default value = ( 0x18 ) for Windows 2000 SP4
	int  MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB		; // = 0 ; // default value = ( 0x1FC ) for Windows 2000 SP4

	int  MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC		; // = 0 ; // default value = ( 0x18 ) for Windows 2000 SP4
	int  MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC		; // = 0 ; // default value = ( 0x24 ) for Windows 2000 SP4

} SOsSpecificSettings;

//
// Prototypes.
//

widestring CharStringToWideString( charstring& csString );

VOID MakeDefaultBugChkDat( IN SOsSpecificSettings* psOsSpec, IN charstring& csPath, IN OUT SStrFileNode** ppStr, IN BOOLEAN bAllocate = TRUE, IN BOOLEAN bWriteHeader = TRUE, IN CONST CHAR* pszSection = NULL );
BOOL NormalizeBugChkDat( SOsSpecificSettings* psOsSpec, SStrFileNode* pStr );
BOOL SaveBugChkDat ( charstring& csBugChkDatFilePath, SStrFileNode* pBugChkDat );
