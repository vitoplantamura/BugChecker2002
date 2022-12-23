/***************************************************************************************
  *
  * memfile.h - VpcICE Memory Files Routines Header File.
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

//========================
// Required Header Files.
//========================

#include <ntddk.h>
#include <stdio.h>
#include <stdarg.h>

#include "WinDefs.h"

//=======================
// Macros - Definitions.
//=======================

#define MEMFILE_NAME(x)			_memfile_##x

#define MACRO_MAXNUM_OF_SESSIONFILES		16
#define MACRO_MAXNUM_OF_SESSIONHANDLES		256

//=============
// Structures.
//=============

// // //

typedef struct _MEMFILE
{
	//----------------------------------
	// Public Informations.
	//----------------------------------

	CHAR			szName[ MAX_PATH ];

	BYTE*			pbMemory;
	ULONG			ulMemoryLength;

	ULONG			ulFileSize;

	//----------------------------------
	// ### ### ### INTERNAL ### ### ###
	//----------------------------------

	// Pool.

	BOOLEAN			bFromPool;

} MEMFILE, *PMEMFILE;

// // //

typedef struct
{
	//----------------------------------
	// ### USER OPAQUE DATA ###
	//----------------------------------

	// If Stream was Closed...

	BOOLEAN			bClosed;

	// Body Pointer.

	MEMFILE*		pmfBody;

	// Ptr.

	ULONG			ulPointer;

	// Access.

	BOOLEAN			bRead;
	BOOLEAN			bWrite;
	BOOLEAN			bText;
	BOOLEAN			bBinary;

} MEMFILE_NAME(FILE);

// // //

typedef struct _MEMFILE_SESSION
{
	//----------------------------------
	// Public Informations.
	//----------------------------------

	// Memory.

	VOID*				pvMemFileMemory;
	ULONG				ulMemFileMemoryLength;

	// Mem Assignment.

	ULONG				ulFileMemoryStep;
	ULONG				ulFileMemoryAssigned;

	// Directory.

	MEMFILE				vmfFiles[ MACRO_MAXNUM_OF_SESSIONFILES ];
	ULONG				ulFilesNum;

	//----------------------------------
	// ### ### ### INTERNAL ### ### ###
	//----------------------------------

	// Handles.

	MEMFILE_NAME(FILE)	vhHandles[ MACRO_MAXNUM_OF_SESSIONHANDLES ];
	ULONG				ulHandlesNum;

} MEMFILE_SESSION, *PMEMFILE_SESSION;

// // //

//=============
// Prototypes.
//=============

// Initialization.

VOID InitializeMemFileSession( IN OUT MEMFILE_SESSION* pmfsSession, IN VOID* pvMemFileMemory, IN ULONG ulMemFileMemoryLength, IN ULONG ulFileMemoryStep, IN MEMFILE* pmfInputFiles, IN ULONG ulInputFilesNum );

// CRT Bridge Functions.

MEMFILE_NAME(FILE) *MEMFILE_NAME(fopen)( const char *filename, const char *mode );
int MEMFILE_NAME(fclose)( MEMFILE_NAME(FILE) *stream );
char *MEMFILE_NAME(fgets)( char *string, int n, MEMFILE_NAME(FILE) *stream );
int MEMFILE_NAME(remove)( const char *path );
void MEMFILE_NAME(rewind)( MEMFILE_NAME(FILE) *stream );
size_t MEMFILE_NAME(fwrite)( const void *buffer, size_t size, size_t count, MEMFILE_NAME(FILE) *stream );
int MEMFILE_NAME(fseek)( MEMFILE_NAME(FILE) *stream, long offset, int origin );
int MEMFILE_NAME(fprintf)( MEMFILE_NAME(FILE) *stream, const char *format, ... );
int MEMFILE_NAME(feof)( MEMFILE_NAME(FILE) *stream );
int MEMFILE_NAME(ferror)( MEMFILE_NAME(FILE) *stream );
