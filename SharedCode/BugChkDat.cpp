//
// BugChkDat.cpp, Copyright(c) 2003, Vito Plantamura. All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////////

#include "..\Include\BugChkDat.h"

///////////////////////////////////////////////////////////////////////////////////

static charstring IntToString ( int val )
{
	CHAR			buff[ 64 ];
	::sprintf( buff, "%i", val );
	return charstring( buff );
}

widestring CharStringToWideString( charstring& csString )
{
	size_t		nStringDim = ( csString.size() + 1 ) * sizeof( WCHAR );
	WCHAR*		pszString = (WCHAR*) ::malloc( nStringDim );

	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
		csString.c_str(), -1,
		pszString, nStringDim / sizeof( WCHAR ) );

	widestring	wsStringToRet = pszString;
	::free( pszString );

	return wsStringToRet;
}

///////////////////////////////////////////////////////////////////////////////////

VOID MakeDefaultBugChkDat( IN SOsSpecificSettings* psOsSpec, IN charstring& csPath, IN OUT SStrFileNode** ppStr, IN BOOLEAN bAllocate /*= TRUE*/, IN BOOLEAN bWriteHeader /*= TRUE*/, IN CONST CHAR* pszSection /*= NULL*/ )
{
	int					i;

	SStrFileNode*			main =
								*ppStr =
									bAllocate ? new SStrFileNode() : *ppStr;

	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//
	//	WARNING ! !
	//
	//	If you add something here, don't forget to update the "NormalizeBugChkDat" function as well ! !
	//
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	//============================
	// Write a Default .dat File.
	//============================

	// # # # Populate Each Section Locally. # # #

	//

	SStrFileNode			startup;
	startup.m_csName = MACRO_BUGCHKDAT_STA_N;
	::SetNodeAndSubNode( startup, MACRO_BUGCHKDAT_STA_MODE_N, MACRO_BUGCHKDAT_STA_MODE_DEFAULT );

	//

	SStrFileNode			memory;
	memory.m_csName = MACRO_BUGCHKDAT_MEM_N;
	::SetNodeAndSubNode( memory, MACRO_BUGCHKDAT_MEM_HISTORY_N, MACRO_BUGCHKDAT_MEM_HISTORY_DEFAULT );
	::SetNodeAndSubNode( memory, MACRO_BUGCHKDAT_MEM_VIDEO_N, MACRO_BUGCHKDAT_MEM_VIDEO_DEFAULT );
	::SetNodeAndSubNode( memory, MACRO_BUGCHKDAT_MEM_SYMBOLS_N, MACRO_BUGCHKDAT_MEM_SYMBOLS_DEFAULT );

	//

	SStrFileNode			symbols;
	symbols.m_csName = MACRO_BUGCHKDAT_SYM_N;
	::SetNodeAndSubNode( symbols, MACRO_BUGCHKDAT_SYM_STARTUP_N, NULL );

	//

	SStrFileNode			commands;
	commands.m_csName = MACRO_BUGCHKDAT_CMD_N;
	::SetNodeAndSubNode( commands, MACRO_BUGCHKDAT_CMD_STARTUP_N, NULL );

	//

	SStrFileNode			troubleshooting;
	troubleshooting.m_csName = MACRO_BUGCHKDAT_TRB_N;
	::SetNodeAndSubNode( troubleshooting, MACRO_BUGCHKDAT_TRB_DISMOUSESUP_N, MACRO_BUGCHKDAT_TRB_DISMOUSESUP_DEFAULT );
	::SetNodeAndSubNode( troubleshooting, MACRO_BUGCHKDAT_TRB_DISNUMACAPS_N, MACRO_BUGCHKDAT_TRB_DISNUMACAPS_DEFAULT );

	//

	SStrFileNode			osspec;
	osspec.m_csName = MACRO_BUGCHKDAT_OSS_N;
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_IDLEPROCESS_OFFSET_REL_INITSYSP_N, ::IntToString( psOsSpec->MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_ADDR_N, ::IntToString( psOsSpec->MACRO_MAPVIEWOFIMAGESECTION_ADDR ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_KTEBPTR_FIELDOFFSET_IN_PCR_N, ::IntToString( psOsSpec->MACRO_KTEBPTR_FIELDOFFSET_IN_PCR ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_CURRIRQL_FIELDOFFSET_IN_PCR_N, ::IntToString( psOsSpec->MACRO_CURRIRQL_FIELDOFFSET_IN_PCR ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_KPEBPTR_FIELDOFFSET_IN_KTEB_N, ::IntToString( psOsSpec->MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_TID_FIELDOFFSET_IN_KTEB_N, ::IntToString( psOsSpec->MACRO_TID_FIELDOFFSET_IN_KTEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_PID_FIELDOFFSET_IN_KPEB_N, ::IntToString( psOsSpec->MACRO_PID_FIELDOFFSET_IN_KPEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_VADROOT_FIELDOFFSET_IN_KPEB_N, ::IntToString( psOsSpec->MACRO_VADROOT_FIELDOFFSET_IN_KPEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB_N, ::IntToString( psOsSpec->MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_CR3_FIELDOFFSET_IN_KPEB_N, ::IntToString( psOsSpec->MACRO_CR3_FIELDOFFSET_IN_KPEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_IMGFLNAME_FIELDOFFSET_IN_KPEB_N, ::IntToString( psOsSpec->MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_IMAGEBASE_FIELDOFFSET_IN_DRVSEC_N, ::IntToString( psOsSpec->MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC ).c_str () );
	::SetNodeAndSubNode( osspec, MACRO_BUGCHKDAT_OSS_IMAGENAME_FIELDOFFSET_IN_DRVSEC_N, ::IntToString( psOsSpec->MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC ).c_str () );

	// # # # Write in the Structured Tree. # # #

	// Write the Header.

	if ( bWriteHeader )
	{
		SStrFileNode			header[ 5 ];
		header[ 0 ].m_csName = MACRO_BUGCHKDAT_HDR_1;
		header[ 1 ].m_csName = MACRO_BUGCHKDAT_HDR_2;
		header[ 2 ].m_csName = MACRO_BUGCHKDAT_HDR_3;
		header[ 3 ].m_csName = MACRO_BUGCHKDAT_HDR_4;
		header[ 4 ].m_csName = MACRO_BUGCHKDAT_HDR_5;

		main->m_csName = csPath;

		for ( i=0; i<5; i ++ )
			main->m_vssfnSubs.push_back( header[ i ] );
	}

	// Calculate the Position of the Header.

	for ( i=0; i<main->m_vssfnSubs.size(); i ++ )
	{
		charstring*			pcs = & main->m_vssfnSubs[ i ].m_csName;
		if ( (*pcs).size() &&
			(*pcs)[ 0 ] != '+' && (*pcs)[ 0 ] != '|' )
		{
			break;
		}
	}

	ULONG			ulPos = i;

	// Insert the Various Sections.

	if ( pszSection == NULL || ::stricmp( pszSection, MACRO_BUGCHKDAT_STA_N ) == 0 )
		main->m_vssfnSubs.insert( main->m_vssfnSubs.begin() + ulPos, startup );

	if ( pszSection == NULL || ::stricmp( pszSection, MACRO_BUGCHKDAT_MEM_N ) == 0 )
		main->m_vssfnSubs.insert( main->m_vssfnSubs.begin() + ulPos, memory );

	if ( pszSection == NULL || ::stricmp( pszSection, MACRO_BUGCHKDAT_SYM_N ) == 0 )
		main->m_vssfnSubs.insert( main->m_vssfnSubs.begin() + ulPos, symbols );

	if ( pszSection == NULL || ::stricmp( pszSection, MACRO_BUGCHKDAT_CMD_N ) == 0 )
		main->m_vssfnSubs.insert( main->m_vssfnSubs.begin() + ulPos, commands );

	if ( pszSection == NULL || ::stricmp( pszSection, MACRO_BUGCHKDAT_TRB_N ) == 0 )
		main->m_vssfnSubs.insert( main->m_vssfnSubs.begin() + ulPos, troubleshooting );

	if ( pszSection == NULL || ::stricmp( pszSection, MACRO_BUGCHKDAT_OSS_N ) == 0 )
		main->m_vssfnSubs.insert( main->m_vssfnSubs.begin() + ulPos, osspec );

	//=========
	// Return.
	//=========

	return;
}

BOOL NormalizeBugChkDat( SOsSpecificSettings* psOsSpec, SStrFileNode* pStr )
{
	BOOL			retval = FALSE;

	//
	// Validate and Correct the Contents of the Structured File
	//  for each File Section.
	//

	//

	if ( SubSubNode( *pStr, MACRO_BUGCHKDAT_STA_N, MACRO_BUGCHKDAT_STA_MODE_N ) == NULL )
	{
		MakeDefaultBugChkDat( psOsSpec, pStr->m_csName, & pStr, FALSE, FALSE, MACRO_BUGCHKDAT_STA_N );
		retval = TRUE;
	}

	//

	if ( SubSubNode( *pStr, MACRO_BUGCHKDAT_MEM_N, MACRO_BUGCHKDAT_MEM_HISTORY_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_MEM_N, MACRO_BUGCHKDAT_MEM_VIDEO_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_MEM_N, MACRO_BUGCHKDAT_MEM_SYMBOLS_N ) == NULL )
	{
		MakeDefaultBugChkDat( psOsSpec, pStr->m_csName, & pStr, FALSE, FALSE, MACRO_BUGCHKDAT_MEM_N );
		retval = TRUE;
	}

	//

	if ( SubSubNode( *pStr, MACRO_BUGCHKDAT_SYM_N, MACRO_BUGCHKDAT_SYM_STARTUP_N ) == NULL )
	{
		MakeDefaultBugChkDat( psOsSpec, pStr->m_csName, & pStr, FALSE, FALSE, MACRO_BUGCHKDAT_SYM_N );
		retval = TRUE;
	}

	//

	if ( SubSubNode( *pStr, MACRO_BUGCHKDAT_CMD_N, MACRO_BUGCHKDAT_CMD_STARTUP_N ) == NULL )
	{
		MakeDefaultBugChkDat( psOsSpec, pStr->m_csName, & pStr, FALSE, FALSE, MACRO_BUGCHKDAT_CMD_N );
		retval = TRUE;
	}

	//

	if ( SubSubNode( *pStr, MACRO_BUGCHKDAT_TRB_N, MACRO_BUGCHKDAT_TRB_DISMOUSESUP_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_TRB_N, MACRO_BUGCHKDAT_TRB_DISNUMACAPS_N ) == NULL )
	{
		MakeDefaultBugChkDat( psOsSpec, pStr->m_csName, & pStr, FALSE, FALSE, MACRO_BUGCHKDAT_TRB_N );
		retval = TRUE;
	}

	//

	if ( SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IDLEPROCESS_OFFSET_REL_INITSYSP_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_MAPVIEWOFIMAGESECTION_ADDR_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_KTEBPTR_FIELDOFFSET_IN_PCR_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_CURRIRQL_FIELDOFFSET_IN_PCR_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_KPEBPTR_FIELDOFFSET_IN_KTEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_TID_FIELDOFFSET_IN_KTEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_PID_FIELDOFFSET_IN_KPEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_VADROOT_FIELDOFFSET_IN_KPEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_CR3_FIELDOFFSET_IN_KPEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IMGFLNAME_FIELDOFFSET_IN_KPEB_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IMAGEBASE_FIELDOFFSET_IN_DRVSEC_N ) == NULL ||
		SubSubNode( *pStr, MACRO_BUGCHKDAT_OSS_N, MACRO_BUGCHKDAT_OSS_IMAGENAME_FIELDOFFSET_IN_DRVSEC_N ) == NULL )
	{
		MakeDefaultBugChkDat( psOsSpec, pStr->m_csName, & pStr, FALSE, FALSE, MACRO_BUGCHKDAT_OSS_N );
		retval = TRUE;
	}

	//

	//
	// Return.
	//

	return retval;
}

BOOL SaveBugChkDat ( charstring& csBugChkDatFilePath, SStrFileNode* pBugChkDat )
{
	return ::SaveStructuredFile(
		::CharStringToWideString( csBugChkDatFilePath ).c_str(),
		pBugChkDat, 1000 );
}
