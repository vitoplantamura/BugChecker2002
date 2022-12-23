//
// LoadFile.cpp, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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
#include <stdio.h>

static void GetDateTimeStamp( char* pszBuffer, SYSTEMTIME* stSystemTime )
{
	// fill the buffer with the date/time stamp

	char*		pszDayOfWeek = "???";
	switch( stSystemTime->wDayOfWeek )
	{
	case 0:
		pszDayOfWeek = "Sun";
		break;
	case 1:
		pszDayOfWeek = "Mon";
		break;
	case 2:
		pszDayOfWeek = "Tue";
		break;
	case 3:
		pszDayOfWeek = "Wed";
		break;
	case 4:
		pszDayOfWeek = "Thu";
		break;
	case 5:
		pszDayOfWeek = "Fri";
		break;
	case 6:
		pszDayOfWeek = "Sat";
		break;
	}

	char*		pszMonth = "???";
	switch( stSystemTime->wMonth )
	{
	case 1:
		pszMonth = "Jan";
		break;
	case 2:
		pszMonth = "Feb";
		break;
	case 3:
		pszMonth = "Mar";
		break;
	case 4:
		pszMonth = "Apr";
		break;
	case 5:
		pszMonth = "May";
		break;
	case 6:
		pszMonth = "Jun";
		break;
	case 7:
		pszMonth = "Jul";
		break;
	case 8:
		pszMonth = "Aug";
		break;
	case 9:
		pszMonth = "Sep";
		break;
	case 10:
		pszMonth = "Oct";
		break;
	case 11:
		pszMonth = "Nov";
		break;
	case 12:
		pszMonth = "Dec";
		break;
	}

	::sprintf( pszBuffer, "%s, %02d %s %i %02d:%02d:%02d GMT", pszDayOfWeek,
		stSystemTime->wDay, pszMonth, stSystemTime->wYear,
		stSystemTime->wHour, stSystemTime->wMinute, stSystemTime->wSecond );

	// return to the caller
	return;
}

BYTE* LoadFile( char* szFilename, size_t* psDim, char* pszLastModified )
{
	if ( psDim )
		*psDim = 0;
	if ( pszLastModified )
		*pszLastModified = 0;

	HANDLE		h = ::CreateFile( szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL );
	if ( h != INVALID_HANDLE_VALUE )
	{
		size_t		s;
		BYTE*		b = (BYTE*)::malloc( ( s = ::GetFileSize( h, NULL ) ) + 1 );

		DWORD		br;
		if ( ::ReadFile( h, b, s, &br, NULL ) == FALSE || br != s )
		{
			::free( b );
			b = NULL;
		}
		else if ( psDim )
			*psDim = s;

		if ( b != NULL )
		{
			b[ s ] = '\0';

			if ( pszLastModified )
			{
				FILETIME		ft;
				::memset( &ft, 0, sizeof( ft ) );
				::GetFileTime( h, NULL, NULL, &ft );
				SYSTEMTIME		st;
				::FileTimeToSystemTime( &ft, &st );
				GetDateTimeStamp( pszLastModified, &st );
			}
		}

		::CloseHandle( h );

		return b;
	}
	else
	{
		return NULL;
	}
}
