//
// BugChkDatDefs.h
// (c) Vito Plantamura Consulting, VPC Technologies SRL, 2003-2004.
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

//==============
// Definitions.
//==============

#define MACRO_BUGCHKDAT_FILENAME			"bugchk.dat"
#define MACRO_BUGCHKDAT_FILENAME_U			L"bugchk.dat"

#define MACRO_BUGCHKDAT_YES					"Yes"
#define MACRO_BUGCHKDAT_NO					"No"

//---------
// Header.
//---------

	// ## WARNING ##: Make the Header Strings begin with the '+' or '|' characters... In this way,
	//  they can be recognized and distinguished easily from the Structured Content.

#define MACRO_BUGCHKDAT_HDR_1				"+---------------------------------------------------------------------------------+"
#define MACRO_BUGCHKDAT_HDR_2				"| bugchk.dat file -> VPC Technologies's BugChecker Structured Configuration File. |"
#define MACRO_BUGCHKDAT_HDR_3				"| !! WARNING !! DO NOT EDIT directly: use Symbol Loader application instead...    |"
#define MACRO_BUGCHKDAT_HDR_4				"| Format Note: the number of TABULATIONS determines the hierarchy of the data.    |"
#define MACRO_BUGCHKDAT_HDR_5				"+---------------------------------------------------------------------------------+"

//----------
// Startup.
//----------

#define MACRO_BUGCHKDAT_STA_N				"Startup Settings (version 1.0)"

	#define MACRO_BUGCHKDAT_STA_MODE_N			"Mode"
	#define MACRO_BUGCHKDAT_STA_MODE_BOOT		"Boot"
	#define MACRO_BUGCHKDAT_STA_MODE_SYSTEM		"System"
	#define MACRO_BUGCHKDAT_STA_MODE_AUTOMATIC	"Automatic"
	#define MACRO_BUGCHKDAT_STA_MODE_MANUAL		"Manual"
	#define MACRO_BUGCHKDAT_STA_MODE_DISABLED	"Disabled"
	#define MACRO_BUGCHKDAT_STA_MODE_DEFAULT	MACRO_BUGCHKDAT_STA_MODE_MANUAL

//---------
// Memory.
//---------

#define MACRO_BUGCHKDAT_MEM_N				"Memory Settings (version 1.0)"

	#define MACRO_BUGCHKDAT_MEM_HISTORY_N		"History buffer size"
	#define MACRO_BUGCHKDAT_MEM_HISTORY_DEFAULT	"256"

	#define MACRO_BUGCHKDAT_MEM_VIDEO_N			"Video memory size"
	#define MACRO_BUGCHKDAT_MEM_VIDEO_DEFAULT	"2048"

	#define MACRO_BUGCHKDAT_MEM_SYMBOLS_N		"Symbols memory size"
	#define MACRO_BUGCHKDAT_MEM_SYMBOLS_DEFAULT	"512"

//----------
// Symbols.
//----------

#define MACRO_BUGCHKDAT_SYM_N				"Symbols Settings (version 1.0)"

	#define MACRO_BUGCHKDAT_SYM_STARTUP_N		"Startup Symbols"

//-----------
// Commands.
//-----------

#define MACRO_BUGCHKDAT_CMD_N				"Commands Settings (version 1.0)"

	#define MACRO_BUGCHKDAT_CMD_STARTUP_N		"Startup Commands"

//------------------
// TroubleShooting.
//------------------

#define MACRO_BUGCHKDAT_TRB_N				"Trouble Shooting Settings (version 1.0)"

	#define MACRO_BUGCHKDAT_TRB_DISMOUSESUP_N			"Disable mouse support"
	#define MACRO_BUGCHKDAT_TRB_DISMOUSESUP_DEFAULT		MACRO_BUGCHKDAT_NO

	#define MACRO_BUGCHKDAT_TRB_DISNUMACAPS_N			"Disable Num Lock and Caps Lock programming"
	#define MACRO_BUGCHKDAT_TRB_DISNUMACAPS_DEFAULT		MACRO_BUGCHKDAT_NO

//--------------
// OS-Specific.
//--------------

#define MACRO_BUGCHKDAT_OSS_N				"OS Specific Settings (version 1.0)"

	#define MACRO_BUGCHKDAT_OSS_PCR_ADDRESS_OF_1ST_PROCESSOR_N				"pcr_address_of_1st_processor"
	#define MACRO_BUGCHKDAT_OSS_KTEBPTR_FIELDOFFSET_IN_PCR_N				"ktebptr_fieldoffset_in_pcr"
	#define MACRO_BUGCHKDAT_OSS_KPEBPTR_FIELDOFFSET_IN_KTEB_N				"kpebptr_fieldoffset_in_kteb"
	#define MACRO_BUGCHKDAT_OSS_IMGFLNAME_FIELDOFFSET_IN_KPEB_N				"imgflname_fieldoffset_in_kpeb"
	#define MACRO_BUGCHKDAT_OSS_CURRIRQL_FIELDOFFSET_IN_PCR_N				"currirql_fieldoffset_in_pcr"
	#define MACRO_BUGCHKDAT_OSS_TID_FIELDOFFSET_IN_KTEB_N					"tid_fieldoffset_in_kteb"
	#define MACRO_BUGCHKDAT_OSS_PID_FIELDOFFSET_IN_KPEB_N					"pid_fieldoffset_in_kpeb"
	#define MACRO_BUGCHKDAT_OSS_IMAGEBASE_FIELDOFFSET_IN_DRVSEC_N			"imagebase_fieldoffset_in_drvsec"
	#define MACRO_BUGCHKDAT_OSS_IMAGENAME_FIELDOFFSET_IN_DRVSEC_N			"imagename_fieldoffset_in_drvsec"
	#define MACRO_BUGCHKDAT_OSS_NTOSKRNL_MODULENAME_UPPERCASE_N				"ntoskrnl_modulename_uppercase"
	#define MACRO_BUGCHKDAT_OSS_VADROOT_FIELDOFFSET_IN_KPEB_N				"vadroot_fieldoffset_in_kpeb"
	#define MACRO_BUGCHKDAT_OSS_VADTREE_UNDOCUMENTED_DISP_0_N				"vadtree_undocumented_disp_0"
	#define MACRO_BUGCHKDAT_OSS_VADTREE_UNDOCUMENTED_DISP_1_N				"vadtree_undocumented_disp_1"
	#define MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_STARTPARAM_N			"mapviewofimagesection_startparam"
	#define MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_SIZEPARAM_N			"mapviewofimagesection_sizeparam"
	#define MACRO_BUGCHKDAT_OSS_UNMAPVIEWOFIMAGESECTION_STARTPARAM_N		"unmapviewofimagesection_startparam"
	#define MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_KPEBPARAM_N			"mapviewofimagesection_kpebparam"
	#define MACRO_BUGCHKDAT_OSS_UNMAPVIEWOFIMAGESECTION_KPEBPARAM_N			"unmapviewofimagesection_kpebparam"
	#define MACRO_BUGCHKDAT_OSS_IDLEPROCESS_OFFSET_REL_INITSYSP_N			"idleprocess_offset_rel_initsysp"
	#define MACRO_BUGCHKDAT_OSS_IDLEPROCESS_IMAGEFILENAME_N					"idleprocess_imagefilename"
	#define MACRO_BUGCHKDAT_OSS_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB_N			"actvproclinks_fieldoffset_in_kpeb"
	#define MACRO_BUGCHKDAT_OSS_CR3_FIELDOFFSET_IN_KPEB_N					"cr3_fieldoffset_in_kpeb"
	#define MACRO_BUGCHKDAT_OSS_MAXVALUE_FOR_WINNT_SEGSELECTOR_N			"maxvalue_for_winnt_segselector"
	#define MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_ADDR_N				"mapviewofimagesection_addr"
	#define MACRO_BUGCHKDAT_OSS_NTTERMINATEPROCESS_FNINDEX_N				"ntterminateprocess_fnindex"
