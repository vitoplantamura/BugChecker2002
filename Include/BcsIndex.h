//
// BcsIndex.h, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

#include "bcsfiles.h"

#define MACRO_BCS_MAJORVER		1
#define MACRO_BCS_MINORVER		0

// === Types. ===

typedef unsigned long TI;

#define MAX_PACKEDFILES_INDEX				64

typedef struct _SBcsIndex
{
	FPO_DATA* pFpoData;								size_t stFpoData;
	IMAGE_SECTION_HEADER* pImageSectionHeader;		size_t stImageSectionHeader;
	IMAGE_SECTION_HEADER* pOriginalSectionData;		size_t stOriginalSectionData;
	CHAR* pSymNames;								size_t stSymNames;
	BCSSYMBOL* pBcsSymbols;							size_t stBcsSymbols;
	TI* pTypeIds;									size_t stTypeIds;
	BYTE* pTypes; /* USHORT + TYPTYPE */			size_t stTypes;
	BYTE* pOmfSymbols; /* USHORT + SYM */			size_t stOmfSymbols;
	BCSSRCFILE* pSrcFiles;							size_t stSrcFiles;
	BCSSRCSEGMENT* pSrcSegments;					size_t stSrcSegments;
	BCSSRCLINE* pSrcLines;							size_t stSrcLines;
	BCSMODULE* pModuleList;							size_t stModuleList;
	CHAR* pSrcNames;								size_t stSrcNames;

	BYTE* pPackedFiles[ MAX_PACKEDFILES_INDEX ];	size_t stPackedFiles;

} SBcsIndex;

// === Prototypes. ===

#define bcs_ptr_params			BYTE** ptrRef, BYTE* pbBase, size_t stDim

#ifdef __cplusplus
	extern "C" {
#endif

		BCSHEADER* BcsValidate (bcs_ptr_params);
		BOOL BcsCreateIndex (bcs_ptr_params, SBcsIndex* pBcsIndex /* memset to 0 */);

#ifdef __cplusplus
	}
#endif
