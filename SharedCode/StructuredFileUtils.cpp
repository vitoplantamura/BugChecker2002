//
// StructuredFileUtils.cpp, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

#include <windows.h>
#include "..\Include\StructuredFileUtils.h"

//==============
// Definitions.
//==============

SStrFileNode* SubNode( IN SStrFileNode& parent, IN CONST CHAR* name )
{
	// Iterate.

	for( int i=0; i<parent.m_vssfnSubs.size(); i ++ )
		if ( ::stricmp( parent.m_vssfnSubs[ i ].m_csName.c_str(), name ) == 0 )
			return & parent.m_vssfnSubs[ i ];

	// Return Failure.

	return NULL;
}

VOID SetNodeAndSubNode( SStrFileNode& l0, CONST CHAR* l1, CONST CHAR* l2 )
{
	// Set the Nodes.

	SStrFileNode			node;
	node.m_csName = l1;

	SStrFileNode			sub;
	if ( l2 )
		sub.m_csName = l2;

	if ( l2 )
		node.m_vssfnSubs.push_back( sub );

	l0.m_vssfnSubs.push_back( node );

	// Return.

	return;
}

static VOID AddSubNodeToNode( SStrFileNode& l0, CONST CHAR* l1, CONST CHAR* l2 )
{
	// Set the Nodes.

	SStrFileNode*			_l1 = SubNode( l0, l1 );
	if ( _l1 == NULL )
		return;

	SStrFileNode			sub;
	sub.m_csName = l2;

	_l1->m_vssfnSubs.push_back( sub );

	// Return.

	return;
}

SStrFileNode* SubSubNode( IN SStrFileNode& root, IN CONST CHAR* l0, IN CONST CHAR* l1 )
{
	// Check the Existence of the Nodes.

	SStrFileNode*		_l0 = SubNode( root, l0 );
	if ( _l0 == NULL )
		return NULL;

	SStrFileNode*		_l1 = SubNode( * _l0, l1 );
	if ( _l1 == NULL )
		return NULL;

	// Return Success.

	return _l1;
}
