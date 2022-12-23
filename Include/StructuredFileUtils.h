//
// StructuredFileUtils.h, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

#include "StructuredFile.h"

// === Prototypes. ===

SStrFileNode* SubNode( IN SStrFileNode& parent, IN CONST CHAR* name );
SStrFileNode* SubSubNode( IN SStrFileNode& root, IN CONST CHAR* l0, IN CONST CHAR* l1 );
VOID SetNodeAndSubNode( SStrFileNode& l0, CONST CHAR* l1, CONST CHAR* l2 );
