//
// StructuredFile.cpp, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

#include <windows.h>

#include "..\Include\StructuredFile.h"

//
// Windows 98 / ME Compatibility Layer
//

static charstring WideStringToCharString( widestring& wsString )
{
	size_t		nStringDim = ( wsString.size() + 1 ) * sizeof( char );
	char*		pszString = (char*) ::malloc( nStringDim );

	::WideCharToMultiByte( CP_ACP, 0,
		wsString.c_str(), -1,
		pszString, nStringDim / sizeof( char ),
		NULL, NULL );

	charstring	csStringToRet = pszString;
	::free( pszString );

	return csStringToRet;
}

HANDLE CreateFileW_Override(
  const WCHAR* lpFileName,                    // file name
  DWORD dwDesiredAccess,                      // access mode
  DWORD dwShareMode,                          // share mode
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
  DWORD dwCreationDisposition,                // how to create
  DWORD dwFlagsAndAttributes,                 // file attributes
  HANDLE hTemplateFile                        // handle to template file
)
{
	OSVERSIONINFO		osviVersionInfo;
	::memset( & osviVersionInfo, 0, sizeof( osviVersionInfo ) );
	osviVersionInfo.dwOSVersionInfoSize = sizeof( osviVersionInfo );
	::GetVersionEx( & osviVersionInfo );

	if ( osviVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		return ::CreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	}
	else
	{
		charstring		csFileName = ::WideStringToCharString( widestring( lpFileName ) );
		return ::CreateFileA( csFileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	}
}

//
// LoadStructuredFile
//

BOOL LoadStructuredFile( const WCHAR* pszStrFileFullName, SStrFileNode** ppssfnFileNode, DWORD dwRetriesMilliseconds /*= 0*/, charstring* pcsLoadFromString /*= NULL*/ )
{
	BOOL		bReturnValue = FALSE;

	// check if we have to open a file or load the content from a string

	CHAR*		b = NULL;

	if ( pszStrFileFullName != NULL )
	{
		DWORD		dwStartMilliseconds = ::GetTickCount();

		// try to open the file

		HANDLE		h;
		do
		{
			h = ::CreateFileW_Override( pszStrFileFullName,
				GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL );
			if ( h == INVALID_HANDLE_VALUE )
			{
				// retry only in the case of a sharing violation
				if ( ::GetLastError() != ERROR_SHARING_VIOLATION )
					break;
				else
					::Sleep( 0 );
			}
			else
			{
				// allocate the memory and read the file
				size_t		s;
				b = (CHAR*)::malloc( ( s = ::GetFileSize( h, NULL ) ) + sizeof( CHAR ) );
				if ( b != NULL )
				{
					// read the file contents
					DWORD		br;
					if ( ::ReadFile( h, b, s, &br, NULL ) != FALSE && br == s )
					{
						// append the terminating null character

						*(CHAR*)( (BYTE*)b + s ) = '\0';
					}
				}

				// release the file handle
				::CloseHandle( h );
			}
		}
		while ( dwRetriesMilliseconds &&
			h == INVALID_HANDLE_VALUE &&
			::GetTickCount() - dwStartMilliseconds < dwRetriesMilliseconds );
	}
	else
	{
		// get the content from a string

		b = (CHAR*) pcsLoadFromString->c_str();
	}

	// parse the contents

	if ( b != NULL )
	{
		// allocate the file node

		*ppssfnFileNode = new SStrFileNode();
		char		szStrFileFullNameA[ MAX_PATH ];
		::WideCharToMultiByte( CP_ACP, 0,
			pszStrFileFullName, -1,
			szStrFileFullNameA, sizeof( szStrFileFullNameA ),
			NULL, NULL );
		(**ppssfnFileNode).m_csName = szStrFileFullNameA;

		// we can consider the operation successful...

		bReturnValue = TRUE;

		// read and parse each line of the file

		CHAR*		p = b;
		BOOL		bEndOfFile = FALSE;
		do
		{
			CHAR		szLine[ 4096 ];
			BOOL		bEndOfLine = FALSE;

			for ( int i=0; i<sizeof(szLine)/sizeof(CHAR); i++ )
			{
				if ( *p == '\0' )
				{
					bEndOfLine = TRUE;
					bEndOfFile = TRUE;

					szLine[ i ] = '\0';
					break;
				}
				else if ( *p == '\r' )
				{
					if ( *(p+1) == '\n' )
						p++;
					bEndOfLine = TRUE;

					szLine[ i ] = '\0';
					break;
				}
				else
				{
					szLine[ i ] = *p++;
				}
			}

			p++;

			if ( bEndOfLine == FALSE ) // LINE TOO LONG
				break;
			else
			{
				// calculate the number of tabulations at the left of the line

				size_t		nNumOfTabs = 0;
				for ( int x=0, y=::strlen( szLine ); x<y; x++ )
					if ( szLine[ x ] == '\t' )
						nNumOfTabs++;
					else
						break;

				// the resulting string must be non-null

				char*		pszLine = &szLine[ nNumOfTabs ];
				if ( ::strlen( pszLine ) != 0 )
				{
					// pick up the node to which we have to add the new leaf

					SStrFileNode*		pssfnThis = *ppssfnFileNode;

					for ( int a=0; a<nNumOfTabs; a++ )
					{
						if ( pssfnThis->m_vssfnSubs.size() == 0 )
						{
							SStrFileNode		ssfnThis;
							pssfnThis->m_vssfnSubs.push_back( ssfnThis );
						}

						pssfnThis = &( pssfnThis->m_vssfnSubs[ pssfnThis->m_vssfnSubs.size() - 1 ] );
					}

					// add the new leaf

					SStrFileNode		ssfnNew;
					ssfnNew.m_csName = pszLine;
					pssfnThis->m_vssfnSubs.push_back( ssfnNew );
				}
			}
		}
		while( bEndOfFile == FALSE );
	}

	// return to the caller

	if ( b != NULL && pszStrFileFullName != NULL )
	{
		// free the memory
		::free( b );
	}

	return bReturnValue;
}

//
// SaveStructuredFile
//

static void SaveStructuredFileRecur( charstring* pcsOutputContent, size_t nTabNum, SStrFileNode* pssfnThis )
{
	// write in the output stream and call ourselves

	if ( nTabNum != -1 && (*pssfnThis).m_csName.size() != 0 )
	{
		for ( int i=0; i<nTabNum; i++ )
			(*pcsOutputContent) += "\t";

		(*pcsOutputContent) += (*pssfnThis).m_csName + "\r\n";
	}

	// call ourselves for all our children

	size_t		nTabNumChildren = nTabNum + 1;

	for ( int i=0; i<(*pssfnThis).m_vssfnSubs.size(); i++ )
		SaveStructuredFileRecur( pcsOutputContent, nTabNumChildren, &( (*pssfnThis).m_vssfnSubs[ i ] ) );
}

BOOL SaveStructuredFile( const WCHAR* pszStrFileFullName, SStrFileNode* pssfnFileNode, DWORD dwRetriesMilliseconds /*= 0*/, charstring* pcsSaveToString /*= NULL*/ )
{
	BOOL		bReturnValue = FALSE;

	// iterate through all the file nodes

	charstring		csOutputContent;
	csOutputContent.reserve( 6 * 1024 * 1024 );

	size_t			nTabNum = -1;

	SaveStructuredFileRecur( &csOutputContent, nTabNum, pssfnFileNode );

	// remove the last lf

	if ( csOutputContent.size() >= 2 &&
		csOutputContent[ csOutputContent.size() - 1 ] == '\n' )
			csOutputContent.erase( csOutputContent.size() - 2, 2 );

	// write the file

	if ( pcsSaveToString )
	{
		*pcsSaveToString = csOutputContent;
		bReturnValue = TRUE;
	}
	else
	{
		DWORD		dwStartMilliseconds = ::GetTickCount();

		HANDLE		h;
		do
		{
			h = ::CreateFileW_Override( pszStrFileFullName,
				GENERIC_WRITE, 0,
				NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL );
			if ( h == INVALID_HANDLE_VALUE )
			{
				// retry only in the case of a sharing violation
				if ( ::GetLastError() != ERROR_SHARING_VIOLATION )
					break;
				else
					::Sleep( 0 );
			}
			else
			{
				// write the file contents
				DWORD		br;
				if ( ::WriteFile( h, csOutputContent.c_str(), csOutputContent.size(), &br, NULL ) != FALSE && br == csOutputContent.size() )
					bReturnValue = TRUE;

				// release the file handle
				::CloseHandle( h );
			}
		}
		while ( dwRetriesMilliseconds &&
			h == INVALID_HANDLE_VALUE &&
			::GetTickCount() - dwStartMilliseconds < dwRetriesMilliseconds );
	}

	// return to the caller

	return bReturnValue;
}

//
// FreeStructuredFile
//

void FreeStructuredFile( SStrFileNode* pssfnFileNode )
{
	// free the memory

	delete pssfnFileNode;

	// return to the caller

	return;
}
