/***************************************************************************************
  *
  * bcsfiles.h - VPCICE NT BCS File Format Structures and Types
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

//==========
// Defines.
//==========

#define MACRO_BCS_HEADERSTRING		"VPC BugChecker Symbol File 1.00\x0D\x0A\x1A"

#define MACRO_R_FILEEND				0  // ### Used to Indicate the End of the BCS File. #
#define MACRO_R_FILESTART			1  // ### File Header informations: type = BCSHEADER. #
#define MACRO_R_FPO					2  // ### Frame Pointer Omission Data: a collection of FPO_DATA Structures. #
#define MACRO_R_SECTIONS			3  // ### Pe Sections: a collection of IMAGE_SECTION_HEADER Structures. #
#define MACRO_R_ORIGINALSECTIONS	4  // ### Pe Original Sections: a collection of IMAGE_SECTION_HEADER Structures. #
#define MACRO_R_SYMBOLS				5  // ### Symbols: a collection of BCSSYMBOL Structures. #
#define MACRO_R_SYMNAMES			6  // ### Symbols Names: sequence of NULL-Terminated ANSI Strings. #
#define MACRO_R_TYPES				7  // ### OMF Types. #
#define MACRO_R_TYPIDS				8  // ### Type Indexes. #
#define MACRO_R_MODULELIST			9  // ### List of Modules: a collection of BCSMODULE Structures. #
#define MACRO_R_SRCNAMES			10 // ### Source Files Names: sequence of NULL-Terminated ANSI Strings. #
#define MACRO_R_OMFSYMBOLS			11 // ### OMF Source Symbols. #
#define MACRO_R_SRCLINES			12 // ### Source Lines: a collection of BCSSRCLINE Structures. #
#define MACRO_R_SRCSEGMENTS			13 // ### Source Segments: a collection of BCSSRCSEGMENT Structures. #
#define MACRO_R_SRCFILES			14 // ### Source Files: a collection of BCSSRCFILE Structures. #
#define MACRO_R_PACKEDFILE			15 // ### Packed Source File: a structure of BCSPACKEDFILE Type followed by the File Contents. #

//==================
// Special Defines.
//==================

#define S_MODSTART					0xEEEE // ### Special Symbol Typ, indicating the Start of a New Module. #

//==========
// Structs.
//==========

#pragma pack ( push, 1 )

	typedef struct _BCSRECORD
	{
		DWORD		dwLEN; // ### The LEN Field itself is Excluded from the Count. #
		DWORD		dwTYP;

	} BCSRECORD, *PBCSRECORD;

	typedef struct _BCSHEADER
	{
		// Symbol File Version.
		WORD		wMajorVer;
		WORD		wMinorVer;

		// Image Identity.
		DWORD		dwTimeDateStamp; // CAN BE 0 !
		DWORD		dwSizeOfImage; // CAN BE 0 !
		DWORD		dwCheckSum; // CAN BE 0 !

		// Further Image Information.
		DWORD		dwImageAlign; // CAN BE 0 !

	} BCSHEADER, *PBCSHEADER;

	typedef struct _BCSSYMBOL
	{
		// Informations about the Symbol.
		ULONG		ulNameOffsetInSymNames;
		USHORT		usModSection;
		ULONG		ulAddressFromModBase;
		ULONG		ulSymSize;

	} BCSSYMBOL, *PBCSSYMBOL;

	typedef struct _BCSMODULE
	{
		// Informations about a Module.
		USHORT		usId;
		ULONG		ulNameOffsetInSrcNames;
	} BCSMODULE, *PBCSMODULE;

	typedef struct _MODSTARTSYM32
	{
		// Informations about this Special OMF Record.
		unsigned short	reclen;		// Record length
		unsigned short	rectyp;		// S_MODSTART
		unsigned short	modid;		// Id of the Module.
	} MODSTARTSYM32, *PMODSTARTSYM32;

	typedef struct _BCSSRCLINE
	{
		// Informations about the Line.
		ULONG		ulLineId; // # Line number: first line = index 1. #
		ULONG		ulAddr; // # This address is Section Relative. #
	} BCSSRCLINE, *PBCSSRCLINE;

	typedef struct _BCSSRCSEGMENT
	{
		// Informations about the Segment.
		ULONG		ulLineStart, ulLineNum;
		USHORT		usSection;
		ULONG		ulMinAddrFromSecStart, ulMaxAddrFromSecStart;
	} BCSSRCSEGMENT, *PBCSSRCSEGMENT;

	typedef struct _BCSSRCFILE
	{
		// Informations about the Source File.
		ULONG		ulSegmentStart, ulSegmentNum;
		USHORT		usModuleId;
		ULONG		ulNameOffsetInSrcNames;
	} BCSSRCFILE, *PBCSSRCFILE;

	typedef struct _BCSPACKEDFILE
	{
		// Informations about a Packed File.
		ULONG		ulNameOffsetInSrcNames;
		ULONG		ulFileSize;
	} BCSPACKEDFILE, *PBCSPACKEDFILE;

#pragma pack ( pop )
