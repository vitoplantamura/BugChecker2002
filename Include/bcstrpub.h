/***************************************************************************************
  *
  * bcstrpub.h - VPCICE NT BCSTRANS Module Public Header File
  *
  * Copyright (c)2003 Vito Plantamura. All Rights Reserved.
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

//===========
// Includes.
//===========

#include <windows.h>

//==========
// Defines.
//==========

#define MACRO_MSPDBMODULESVEC_MAXITEMS			32
#define MACRO_SOURCEDIRSVEC_MAXITEMS			32

//======================
// "Special" Defines...
//======================

#define MACRO_NEWMSPDB_GUIDLEN					0x10

//========
// Enums.
//========

enum ENUM_TRANSGEN
{
	TRANSGEN_FIRST
};

enum ENUM_TRANSERR
{
	TRANSERR_SUCCESS,
	TRANSERR_GENERR,			// Problems with DLL version: the generation of the caller is not compatible.
	TRANSERR_MSPDBDLL,			// Problems with the MSPDB DLL.
	TRANSERR_PARAMS,			// Problems with the Trans Parameters.
	TRANSERR_OPENPDB,			// Problems when Opening the PDB File.
	TRANSERR_OPENBCS,			// Problems when Opening the BCS File for writing.
	TRANSERR_OPENDBI,			// Problems when Opening DBI.
	TRANSERR_OPENTPI,			// Problems when Opening TPI.
	TRANSERR_OPENPUB,			// Problems when Opening Publics.
	TRANSERR_WRITEBCS,			// Problems when Writing BCS File.
	TRANSERR_READOMAPF,			// Problems when Reading Omap From data from PDB.
	TRANSERR_READOMAPT,			// Problems when Reading Omap To data from PDB.
	TRANSERR_READFPO,			// Problems when Reading Fpo data from PDB.
	TRANSERR_READSECS,			// Problems when Reading PE Sections.
	TRANSERR_READOSECS,			// Problems when Reading PE Original Sections.
	TRANSERR_PDBEXT,			// Extension != .PDB and != .DBG.
	TRANSERR_READPDB,			// Problems when reading PDB Header.
	TRANSERR_PDBFORMAT,			// Invalid PDB Format.
	TRANSERR_OPENDBG,			// Problems when Opening the DBG File.
	TRANSERR_READDBG,			// Problems when reading DBG File.
	TRANSERR_DBGFORMAT,			// Invalid DBG Format.
	TRANSERR_MISMDBG,			// Mismatched DBG File. (datetimestamp or imagesize mismatch.)
	TRANSERR_MISSPDBNAME,		// Missing PDB File Name in DBG Symbol File.
	TRANSERR_READDBGOMAPF,		// Problems when Reading Omap From data from DBG.
	TRANSERR_READDBGOMAPT,		// Problems when Reading Omap To data from DBG.
	TRANSERR_READDBGFPO,		// Problems when Reading Fpo data from DBG.
	TRANSERR_OPENMOD,			// ## INFOFromMOD ## Unable to open MOD.
	TRANSERR_READMOD,			// ## INFOFromMOD ## Unable to read from MOD or format of MOD is Invalid.
	TRANSERR_NOMODMEM,			// ## INFOFromMOD ## Unable to allocate Memory.
	TRANSERR_NOMODDBG,			// ## INFOFromMOD ## Unable to find Debug Info in MOD.
	TRANSERR_OUTBUFDIM,			// ## INFOFromMOD ## Dimension of Output Buffer is too Small.
	TRANSERR_PDBPUBSYM,			// Problems when processing public symbols (from Pdb).
	TRANSERR_PDBTYPS,			// Problems when processing Types (reading/processing or writing to disk...).
	TRANSERR_PDBMODS,			// Problems when processing Modules.
	TRANSERR_PCKSRC				// Problems when copying Source Files into .BCS File.
};

enum ENUM_TRANSLEVEL
{
	TRANSLEVEL_PUBLICSONLY,		// Public symbol names. No types, no source.
	TRANSLEVEL_TYPESONLY,		// Only type information.
	TRANSLEVEL_SYMBOLSONLY,		// Public+local symbol names. No source.
	TRANSLEVEL_SYMANDSOURCE,	// Full debug information (line infos). <-- DEFAULT.
	TRANSLEVEL_SYMANDSRCWTFILES,// Full debug information + source code packaged in BCS.
	// // //
	TRANSLEVEL_GETDBGINFOONLY	// ## SPECIAL ## : Get only Informations about the .DBG Module.
};

enum ENUM_MISSINGSRCRES
{
	MISSINGSRCRES_SKIPONLYTHIS,	// Skip only this file.
	MISSINGSRCRES_SKIPALL,		// Skip all the missing files from this point on.
	MISSINGSRCRES_OTHERPROVIDED	// Alternative path provided. Look in the Output Buffer.
};

//=========
// Macros.
//=========

#define BCSTRFNP( rettyp, funcname )				typedef rettyp ( __cdecl * PFN##funcname )

//========
// Types.
//========

typedef ULONG SIG;
typedef ULONG AGE;

typedef VOID ( __cdecl * PFNBCSMESSAGEBOX )( IN CHAR* pszText, IN ULONG ulType /* same as Win32 MessageBox API... */ );
typedef VOID ( __cdecl * PFNBCSEVENT )( IN CHAR* pszTextEventLine );
typedef VOID ( __cdecl * PFNBCSMISSINGSRC )( IN CONST CHAR* pszSrcFilePath, OUT ENUM_MISSINGSRCRES* peRes, OUT CHAR* pszOutputBuffer, IN ULONG ulOutputBufferSize );
typedef PVOID ( __cdecl * PFNALLOCATOR )( IN size_t sSize );

typedef struct _PDBVALIDATEPARAMS // <-- ### WARNING: Memset this to 0 before Using !!! #
{
	//
	// Validation data used when opening Pdb Files with the API: PDBOpenValidate.
	//

	SIG			sSig;
	AGE			aAge;

	//
	// Special Signature used when Opening new VC++ 7/.NET PDBs...
	// ## WARNING ## Initialize this field to 0s... !
	//

	BYTE		vbGuid[ MACRO_NEWMSPDB_GUIDLEN ];

	//
	// Validation data used when Opening .DBG Files. This information is MANDATORY, because it is
	//  included in the .BCS File Header. If you Omit it, the Corresponding Fields in the .BCS header
	//  will be Left Blank (and no Validation is Performed against the Loaded .DBG File...).
	//

	DWORD		dwTimeDateStamp;
	DWORD		dwSizeOfImage;
	DWORD		dwCheckSum;

	//
	// Additional Informations that can be Omitted. If Available it is highly advised to Specify Them.
	// ## WARNING ## If not Available set Them to 0/NULL. IMPORTANT !! !!
	//

	DWORD		dwImageAlign;

	//
	// Allocator pointer.
	//

	PFNALLOCATOR		pfnMemAlloc;

	//
	// Image Sections Information.
	//  This information can be obtained calling "INFOFromMOD" API. The API will use the Specified Allocator
	//  to allocate the Memory. The sections information is Mandatory (as input) when calling the "BCSFromPDB"
	//  in the case that the ".PDB" file doesn't contain the Same Information.
	//

	IMAGE_SECTION_HEADER*		pishSecs;
	ULONG						ulSecsNum;

	//
	// PDB Filename. Returned only when extracted from a DBG file.
	//

	CHAR						szPdbFilename[ MAX_PATH ];

} PDBVALIDATEPARAMS, *PPDBVALIDATEPARAMS;

typedef struct _BCSTRANSPARAMS
{
	//
	// PDB validation.
	//
	PDBVALIDATEPARAMS	pdbvpValidate;

	//
	// MSPDB file names.
	//  (order matters, they should be arranged with the most recent version as the first item).
	//
	CHAR*				vpszMsPdbModules[ MACRO_MSPDBMODULESVEC_MAXITEMS ];

	//
	// User notification.
	//
	PFNBCSMESSAGEBOX	pfnBcsMessageBox;
	PFNBCSEVENT			pfnBcsEvent;

	//
	// Translation level.
	//
	ENUM_TRANSLEVEL		eTransLevel;

	//
	// Source code loading.
	//
	CHAR*				vpszSourceDirs[ MACRO_SOURCEDIRSVEC_MAXITEMS ];
	PFNBCSMISSINGSRC	pfnBcsMissingSourceFile;

} BCSTRANSPARAMS, *PBCSTRANSPARAMS;

//===========
// Typedefs.
//===========

BCSTRFNP(BOOLEAN, BCSFromPDB)(
		IN ENUM_TRANSGEN eEngineGeneration,
		IN CHAR* pszPDBFileName,
		IN CHAR* pszBCSFileName,
		IN CHAR* pszDMPFileName,
		IN BCSTRANSPARAMS* pbcstpBcsParams,
		OUT ENUM_TRANSERR* peErr
	);

BCSTRFNP(BOOLEAN, INFOFromMOD)(
		IN ENUM_TRANSGEN eEngineGeneration,
		IN CHAR* pszMODFileName,
		OUT PDBVALIDATEPARAMS* ppdbvpOut,
		OUT CHAR* pszDebugFile,
		IN ULONG ulDebugFileSize,
		OUT ENUM_TRANSERR* peErr
	);
