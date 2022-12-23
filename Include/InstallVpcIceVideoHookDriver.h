/***************************************************************************************
  *
  * InstallVpcIceVideoHookDriver.h - Header File for Video Hook Kernel Mode Driver installation module.
  *
  * Copyright(c) 2003, Vito Plantamura. All rights reserved.
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

//
// ### Required Header Files. ###
//

#include <windows.h>

//
// ### Function Prototypes. ###
//

BOOL InstallVpcIceVideoHookDriver ( HINSTANCE hResMod, LPCTSTR pszResName, LPCTSTR pszResType, LPCTSTR pszDriverName, LPCTSTR pszSysFileName );
BOOL RemoveVpcIceVideoHookDriver ( LPCTSTR pszDriverName, LPCTSTR pszSysFileName );
