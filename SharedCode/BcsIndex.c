//
// BcsIndex.c, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

//===============
// Header files.
//===============

#ifdef WIN32 // =VP= 2010 - This file should be linked against the BugChecker driver, in the future.
	#include <windows.h>
#endif

#include "..\Include\BcsIndex.h"

//==============
// Definitions.
//==============

#define ptr					(* ptrRef)
#define ptr_param			bcs_ptr_params
#define ptr_check			if ( ptr - pbBase > (int) stDim ) return FALSE;

BCSHEADER* BcsValidate (ptr_param)
{
	BCSHEADER*	pBCSHEADER;
	size_t		stHeaderStrSize = strlen( MACRO_BCS_HEADERSTRING );

	if ( stDim <= stHeaderStrSize + sizeof( BCSRECORD ) + sizeof( BCSHEADER ) )
		return NULL;

	if ( memcmp( ptr, MACRO_BCS_HEADERSTRING, stHeaderStrSize ) )
		return NULL;

	ptr += stHeaderStrSize + sizeof( BCSRECORD );

	pBCSHEADER = (BCSHEADER*) ptr;
	if ( pBCSHEADER->wMajorVer != MACRO_BCS_MAJORVER || pBCSHEADER->wMinorVer != MACRO_BCS_MINORVER )
		return NULL;

	ptr += sizeof( BCSHEADER );

	return pBCSHEADER;
}

BOOL BcsCreateIndex (ptr_param, SBcsIndex* pBcsIndex /* memset to 0 */)
{
	while( 1 )
	{
		BCSRECORD*			pBCSRECORD;
		BYTE*				pbPayload;
		size_t				stPayload;

		// get a ptr to the next record...

		ptr_check

		pBCSRECORD = (BCSRECORD*) ptr;
		pbPayload = (BYTE*) ptr + sizeof (BCSRECORD);
		stPayload = pBCSRECORD->dwLEN - sizeof( BCSRECORD ) + sizeof( DWORD );

		ptr += pBCSRECORD->dwLEN + sizeof( DWORD );

		ptr_check

		// which record ?

		switch ( pBCSRECORD->dwTYP )
		{
		case MACRO_R_FILEEND:

			return TRUE;

		case MACRO_R_FILESTART:

			return FALSE;

		case MACRO_R_FPO:

			pBcsIndex->pFpoData = (FPO_DATA*) pbPayload;
			pBcsIndex->stFpoData = stPayload / sizeof (FPO_DATA);

			break;

		case MACRO_R_SECTIONS:

			pBcsIndex->pImageSectionHeader = (IMAGE_SECTION_HEADER*) pbPayload;
			pBcsIndex->stImageSectionHeader = stPayload / sizeof (IMAGE_SECTION_HEADER);

			break;

		case MACRO_R_ORIGINALSECTIONS:

			pBcsIndex->pOriginalSectionData = (IMAGE_SECTION_HEADER*) pbPayload;
			pBcsIndex->stOriginalSectionData = stPayload / sizeof (IMAGE_SECTION_HEADER);

			break;

		case MACRO_R_SYMBOLS:

			pBcsIndex->pBcsSymbols = (BCSSYMBOL*) pbPayload;
			pBcsIndex->stBcsSymbols = stPayload / sizeof (BCSSYMBOL);

			break;

		case MACRO_R_SYMNAMES:

			pBcsIndex->pSymNames = (CHAR*) pbPayload;
			pBcsIndex->stSymNames = stPayload;

			break;

		case MACRO_R_TYPES:

			pBcsIndex->pTypes = (BYTE*) pbPayload;
			pBcsIndex->stTypes = stPayload;

			break;

		case MACRO_R_TYPIDS:

			pBcsIndex->pTypeIds = (TI*) pbPayload;
			pBcsIndex->stTypeIds = stPayload / sizeof (TI);

			break;

		case MACRO_R_MODULELIST:

			pBcsIndex->pModuleList = (BCSMODULE*) pbPayload;
			pBcsIndex->stModuleList = stPayload / sizeof (BCSMODULE);

			break;

		case MACRO_R_SRCNAMES:

			pBcsIndex->pSrcNames = (CHAR*) pbPayload;
			pBcsIndex->stSrcNames = stPayload;

			break;

		case MACRO_R_OMFSYMBOLS:

			pBcsIndex->pOmfSymbols = (BYTE*) pbPayload;
			pBcsIndex->stOmfSymbols = stPayload;

			break;

		case MACRO_R_SRCLINES:

			pBcsIndex->pSrcLines = (BCSSRCLINE*) pbPayload;
			pBcsIndex->stSrcLines = stPayload / sizeof (BCSSRCLINE);

			break;

		case MACRO_R_SRCSEGMENTS:

			pBcsIndex->pSrcSegments = (BCSSRCSEGMENT*) pbPayload;
			pBcsIndex->stSrcSegments = stPayload / sizeof (BCSSRCSEGMENT);

			break;

		case MACRO_R_SRCFILES:

			pBcsIndex->pSrcFiles = (BCSSRCFILE*) pbPayload;
			pBcsIndex->stSrcFiles = stPayload / sizeof (BCSSRCFILE);

			break;

		case MACRO_R_PACKEDFILE:

			if ( pBcsIndex->stPackedFiles < MAX_PACKEDFILES_INDEX )
				pBcsIndex->pPackedFiles[ pBcsIndex->stPackedFiles ++ ] = pbPayload;

			break;

		default:
			return FALSE;
		}
	}

	return FALSE; // unreachable code.
}

#undef ptr
