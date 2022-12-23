/***************************************************************************************
  *
  * crt.h - VPCICE Support Routines from CRT - Header File.
  *
  * Copyright (c)2003 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

#include <ntddk.h>
#include <limits.h>
#include <errno.h>
#include "WinDefs.h"

//==============
// Definitions.
//==============

#define MACRO_CRTFN_NAME( orig_name )		vpcice_##orig_name

//=============
// Prototypes.
//=============

long __cdecl MACRO_CRTFN_NAME(strtol) (
        const char *nptr,
        char **endptr,
        int ibase
        );

unsigned long __cdecl MACRO_CRTFN_NAME(strtoul) (
        const char *nptr,
        char **endptr,
        int ibase
        );

char * __cdecl MACRO_CRTFN_NAME(strchr) (
        const char * string,
        int ch
        );

double __cdecl MACRO_CRTFN_NAME(strtod) (
        const char *nptr,
        /*REG2*/ char **endptr
        );

int __cdecl MACRO_CRTFN_NAME(stricmp) (
        const char * dst,
        const char * src
        );

int __cdecl MACRO_CRTFN_NAME(memicmp) (
        const void * first,
        const void * last,
        unsigned int count
        );

CHAR* __cdecl cftof(
		IN DOUBLE dValue,
		OUT CHAR* pszBuffer,
		IN ULONG ulPrecision
		);

CHAR* __cdecl cftog(
		IN DOUBLE dValue,
		OUT CHAR* pszBuffer,
		IN ULONG ulPrecision,
		IN BOOLEAN bCapilalizeExp
		);
