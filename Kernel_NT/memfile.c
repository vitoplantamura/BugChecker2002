/***************************************************************************************
  *
  * memfile.c - VpcICE Memory Files Routines Source File.
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

//===============
// Header Files.
//===============

#include "..\Include\memfile.h"
#include "..\Include\crt.h"

//===============================================
// InitializeMemFileSession Function Definition.
//===============================================

static MEMFILE_SESSION*			g_pmfsGlobalSess = NULL;

VOID InitializeMemFileSession( IN OUT MEMFILE_SESSION* pmfsSession, IN VOID* pvMemFileMemory, IN ULONG ulMemFileMemoryLength, IN ULONG ulFileMemoryStep, IN MEMFILE* pmfInputFiles, IN ULONG ulInputFilesNum )
{
	ULONG			ulI;
	MEMFILE*		pmfInPtr;
	MEMFILE*		pmfOutPtr;

	// Save the Pointer to the Session.

	g_pmfsGlobalSess = pmfsSession;

	// Initialize the Structure Fields.

	memset( pmfsSession, 0, sizeof( MEMFILE_SESSION ) );

	pmfsSession->pvMemFileMemory = pvMemFileMemory;
	pmfsSession->ulMemFileMemoryLength = ulMemFileMemoryLength;
	pmfsSession->ulFileMemoryStep = ulFileMemoryStep;

	for ( ulI = 0; ulI < ulInputFilesNum; ulI ++ )
	{
		pmfInPtr = & pmfInputFiles[ ulI ];
		pmfOutPtr = & pmfsSession->vmfFiles[ pmfsSession->ulFilesNum ++ ];

		strcpy( pmfOutPtr->szName, pmfInPtr->szName );
		pmfOutPtr->pbMemory = pmfInPtr->pbMemory;
		pmfOutPtr->ulMemoryLength = pmfInPtr->ulMemoryLength;
		pmfOutPtr->ulFileSize = pmfInPtr->ulFileSize;
	}

	// Return to the Caller.

	return;
}

//============================
// fopen Function Definition.
//============================

MEMFILE_NAME(FILE) *MEMFILE_NAME(fopen)( const char *filename, const char *mode )
{
	ULONG				ulI;
	MEMFILE*			pmfThis;
	MEMFILE*			pmfMatch;

	BOOLEAN				bRead = FALSE;
	BOOLEAN				bWrite = FALSE;
	BOOLEAN				bText = FALSE;
	BOOLEAN				bBinary = FALSE;

	MEMFILE_NAME(FILE)*	phFile;

	// Check whether there is a Session State.

	if ( g_pmfsGlobalSess == NULL )
		return NULL;

	// Check whether we can Release an Handle.

	if ( g_pmfsGlobalSess->ulHandlesNum == MACRO_MAXNUM_OF_SESSIONHANDLES )
		return NULL;

	// Parse the Mode String.

	if ( MACRO_CRTFN_NAME(strchr)( mode, '+' ) )
	{
		bRead = TRUE;
		bWrite = TRUE;
	}
	else if ( MACRO_CRTFN_NAME(strchr)( mode, 'w' ) || MACRO_CRTFN_NAME(strchr)( mode, 'W' ) )
	{
		bWrite = TRUE;
	}
	else if ( MACRO_CRTFN_NAME(strchr)( mode, 'r' ) || MACRO_CRTFN_NAME(strchr)( mode, 'R' ) )
	{
		bRead = TRUE;
	}

	if ( bRead == FALSE && bWrite == FALSE )
		return NULL;

	if ( MACRO_CRTFN_NAME(strchr)( mode, 't' ) || MACRO_CRTFN_NAME(strchr)( mode, 'T' ) )
		bText = TRUE;
	else if ( MACRO_CRTFN_NAME(strchr)( mode, 'b' ) || MACRO_CRTFN_NAME(strchr)( mode, 'B' ) )
		bBinary = TRUE;

	if ( bText == FALSE && bBinary == FALSE )
		return NULL;

	// Scan the Directory for the File.

	pmfMatch = NULL;

	for ( ulI = 0; ulI < g_pmfsGlobalSess->ulFilesNum; ulI ++ )
	{
		pmfThis = & g_pmfsGlobalSess->vmfFiles[ ulI ];

		if ( MACRO_CRTFN_NAME(stricmp)( pmfThis->szName, filename ) == 0 )
		{
			pmfMatch = pmfThis;
			break;
		}
	}

	// See whether the File has to be Created.

	if ( pmfMatch == NULL )
	{
		if ( bWrite == FALSE )
		{
			return NULL;
		}
		else
		{
			if ( g_pmfsGlobalSess->ulFilesNum == MACRO_MAXNUM_OF_SESSIONFILES )
			{
				return NULL;
			}
			else
			{
				if ( g_pmfsGlobalSess->ulFileMemoryAssigned + g_pmfsGlobalSess->ulFileMemoryStep > g_pmfsGlobalSess->ulMemFileMemoryLength )
					return NULL;

				pmfMatch = & g_pmfsGlobalSess->vmfFiles[ g_pmfsGlobalSess->ulFilesNum ++ ];

				strcpy( pmfMatch->szName, filename );
				pmfMatch->pbMemory = ( BYTE* ) g_pmfsGlobalSess->pvMemFileMemory + g_pmfsGlobalSess->ulFileMemoryAssigned;
				pmfMatch->ulMemoryLength = g_pmfsGlobalSess->ulFileMemoryStep;
				pmfMatch->ulFileSize = 0;
				pmfMatch->bFromPool = TRUE;

				g_pmfsGlobalSess->ulFileMemoryAssigned += g_pmfsGlobalSess->ulFileMemoryStep;
			}
		}
	}
	else
	{
		// See whether the File has to be Emptied.

		if ( bWrite )
		{
			pmfMatch->ulFileSize = 0;

			for ( ulI = 0; ulI < g_pmfsGlobalSess->ulHandlesNum; ulI ++ )
				if ( g_pmfsGlobalSess->vhHandles[ ulI ].bClosed == FALSE &&
					g_pmfsGlobalSess->vhHandles[ ulI ].pmfBody == pmfMatch )
				{
					g_pmfsGlobalSess->vhHandles[ ulI ].ulPointer = 0;
				}
		}
	}

	// Return the Handle to the Caller.

	phFile = & g_pmfsGlobalSess->vhHandles[ g_pmfsGlobalSess->ulHandlesNum ++ ];

	phFile->bClosed = FALSE;
	phFile->pmfBody = pmfMatch;
	phFile->ulPointer = 0;
	phFile->bRead = bRead;
	phFile->bWrite = bWrite;
	phFile->bText = bText;
	phFile->bBinary = bBinary;

	return phFile;
}

//=============================
// fclose Function Definition.
//=============================

int MEMFILE_NAME(fclose)( MEMFILE_NAME(FILE) *stream )
{
	// Check if Closed.

	if ( stream->bClosed )
		return EOF;

	// Close the Stream and Return.

	stream->bClosed = TRUE;

	return 0;
}

//============================
// fgets Function Definition.
//============================

char *MEMFILE_NAME(fgets)( char *string, int n, MEMFILE_NAME(FILE) *stream )
{
	MEMFILE*		pmfBody = stream->pmfBody;
	BYTE*			pbPtr;
	CHAR*			pszOut;

	// Check if Closed.

	if ( stream->bClosed )
		return NULL;

	// Check the Access.

	if ( stream->bRead == FALSE )
		return NULL;

	// Check if EOF.

	if ( MEMFILE_NAME(feof)( stream ) )
		return NULL;

	// Find the End of the String.

	if ( n == 0 )
		return string;
	n --; // For NULL char.
	if ( n == 0 )
	{
		* string = '\0';
		return string;
	}

	pbPtr = pmfBody->pbMemory + stream->ulPointer;
	pszOut = string;

	while( TRUE )
	{
		// Read from the Stream.

		if ( stream->ulPointer == pmfBody->ulFileSize || n == 0 )
			break;
		else if ( * pbPtr == 0xD )
		{
			pbPtr ++;
			stream->ulPointer ++;

			if ( stream->ulPointer < pmfBody->ulFileSize &&
				* pbPtr == 0xA )
			{
				stream->ulPointer ++;
			}

			break;
		}

		* pszOut = * pbPtr;

		// Increment.

		n --;
		stream->ulPointer ++;

		pbPtr ++;
		pszOut ++;
	}

	* pszOut = '\0';

	// Return to the Caller.

	return string;
}

//=============================
// remove Function Definition.
//=============================

int MEMFILE_NAME(remove)( const char *path )
{
	// ### DO NOTHING HERE ###

	// Return.

	return 0;
}

//=============================
// rewind Function Definition.
//=============================

void MEMFILE_NAME(rewind)( MEMFILE_NAME(FILE) *stream )
{
	// Check if Closed.

	if ( stream->bClosed )
		return;

	// Rewind and Return.

	stream->ulPointer = 0;

	return;
}

//=============================
// fwrite Function Definition.
//=============================

size_t MEMFILE_NAME(fwrite)( const void *buffer, size_t size, size_t count, MEMFILE_NAME(FILE) *stream )
{
	MEMFILE*		pmfBody = stream->pmfBody;
	ULONG			ulLength = size * count;

	// Check if Closed.

	if ( stream->bClosed )
		return 0;

	// Check the Access to the File.

	if ( stream->bWrite == FALSE )
		return 0;

	// Check if There is Enough space in the Stream.

	if ( stream->ulPointer + ulLength > pmfBody->ulMemoryLength )
		return 0;

	// Write in the Stream.

	memcpy( pmfBody->pbMemory + stream->ulPointer, buffer, ulLength );

	// Increment the Stream Pointer.

	stream->ulPointer += ulLength;

	if ( stream->ulPointer > pmfBody->ulFileSize )
		pmfBody->ulFileSize = stream->ulPointer;

	// Return.

	return count;
}

//============================
// fseek Function Definition.
//============================

int MEMFILE_NAME(fseek)( MEMFILE_NAME(FILE) *stream, long offset, int origin )
{
	MEMFILE*		pmfBody = stream->pmfBody;

	// Check if Closed.

	if ( stream->bClosed )
		return 1;

	// Set the File Pointer.

	switch( origin )
	{
	case SEEK_CUR:
		{
			offset += stream->ulPointer;
		}
		break;

	case SEEK_END:
		{
			offset += pmfBody->ulFileSize;
		}
		break;

	case SEEK_SET:
		{
			// ### DO NOTHING HERE ###
		}
		break;

	default:
		return 1;
	}

	if ( (ULONG) offset > pmfBody->ulMemoryLength )
		offset = pmfBody->ulMemoryLength;

	stream->ulPointer = offset;

	return 0;
}

//==============================
// fprintf Function Definition.
//==============================

static CHAR			g_szFprintfBuffer[ 1024 ];

int MEMFILE_NAME(fprintf)( MEMFILE_NAME(FILE) *stream, const char *format, ... )
{
	va_list			valList;
	MEMFILE*		pmfBody = stream->pmfBody;
	ULONG			ulStrSize;
	ULONG			ulDiff;
	BYTE*			pbPtr;

	// Check if Closed.

	if ( stream->bClosed )
		return -1;

	// Check the Access to the File.

	if ( stream->bWrite == FALSE )
		return 0;

	// Format the String.

	va_start( valList, format );
	vsprintf( g_szFprintfBuffer, format, valList );
	va_end( valList );

	ulStrSize = strlen( g_szFprintfBuffer );

	// Check if There is Enough space in the Stream.

	if ( stream->ulPointer + ulStrSize + 2 > pmfBody->ulMemoryLength )
	{
		ulDiff = ( stream->ulPointer + ulStrSize + 2 ) - pmfBody->ulMemoryLength;

		if ( ulDiff > ulStrSize )
		{
			return 0;
		}
		else
		{
			ulStrSize -= ulDiff;
			g_szFprintfBuffer[ ulStrSize ] = '\0';
		}
	}

	// Write the String into the Stream.

	pbPtr = pmfBody->pbMemory + stream->ulPointer;

	strcpy( pbPtr, g_szFprintfBuffer );

	pbPtr += ulStrSize;

	* pbPtr ++ = 0xD;
	* pbPtr ++ = 0xA;

	// Increment the Stream Pointer.

	stream->ulPointer += ulStrSize + 2;

	if ( stream->ulPointer > pmfBody->ulFileSize )
		pmfBody->ulFileSize = stream->ulPointer;

	// Return to the Caller.

	return ulStrSize + 2;
}

//===========================
// feof Function Definition.
//===========================

int MEMFILE_NAME(feof)( MEMFILE_NAME(FILE) *stream )
{
	MEMFILE*		pmfBody = stream->pmfBody;

	// Check if Closed.

	if ( stream->bClosed )
		return 1;

	// Check if the Stream is at Its End.

	if ( stream->ulPointer >= pmfBody->ulFileSize )
		return 1;
	else
		return 0;
}

//=============================
// ferror Function Definition.
//=============================

int MEMFILE_NAME(ferror)( MEMFILE_NAME(FILE) *stream )
{
	// Check if Closed.

	if ( stream->bClosed )
		return 1;

	// No Error in all cases.

	return 0;
}
