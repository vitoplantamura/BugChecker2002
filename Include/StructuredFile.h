//
// StructuredFile.h, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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
// STL inclusions and types
//

#include <string>
#include <vector>
typedef std::basic_string< CHAR > charstring;
typedef std::basic_string< WCHAR > widestring;

//
// structures and types
//

typedef struct SStrFileNode
{
	charstring					m_csName;
	std::vector< SStrFileNode > m_vssfnSubs;
} SStrFileNode;

//
// prototypes
//

BOOL LoadStructuredFile( const WCHAR* pszStrFileFullName, SStrFileNode** ppssfnFileNode, DWORD dwRetriesMilliseconds = 0, charstring* pcsLoadFromString = NULL );
BOOL SaveStructuredFile( const WCHAR* pszStrFileFullName, SStrFileNode* pssfnFileNode, DWORD dwRetriesMilliseconds = 0, charstring* pcsSaveToString = NULL );
void FreeStructuredFile( SStrFileNode* pssfnFileNode );
