/***************************************************************************************
  *
  * InstallVpcIce.cpp - Source File for VpcICE Kernel Mode Driver installation.
  *
  * Copyright(c) 2003, Vito Plantamura. All rights reserved.
  *
  * Note: Based on the source found on the www.sysinternals.com site for the filemon application.
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

//
// ### Required Header Files. ###
//

#include <windows.h>
#include <stdio.h>

#include "..\Include\InstallVpcIce.h"

//
// ### Function Definitions. ###
//

/**************************************************
*
*      HELPER FUNCTION
*
**************************************************/
static BOOL GetDriverOutFileFullPath( LPCTSTR pszSysFile, char* pszOutFullPath ) // --> Note: the Buffer (pszOutFullPath) must be MAX_PATH*2 characters in length.
{
	// Calculate the final file name.

	if ( ::GetSystemDirectory( pszOutFullPath, MAX_PATH ) == 0 )
		return FALSE;

	char		cLastChr = pszOutFullPath[ ::strlen( pszOutFullPath ) - 1 ];
	if ( cLastChr != '\\' && cLastChr != '/' )
		::strcat( pszOutFullPath, "\\" );

	::strcat( pszOutFullPath, "drivers\\" );
	::strcat( pszOutFullPath, pszSysFile );

	// Return to the Caller.

	return TRUE;
}

/**************************************************
*
*	FUNCTION:  CreateDriverFileFromAppResources
*
**************************************************/
//static BOOL CreateDriverFileFromAppResources( LPTSTR pszOutFileFullPathBufferPtr, HINSTANCE hResMod, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszSysFile )
//{
//	// Get the full path of the output file.
//
//	char		szOutFullPath[ MAX_PATH * 2 ];
//	if ( ::GetDriverOutFileFullPath( pszSysFile, szOutFullPath ) == FALSE )
//		return FALSE;
//
//	// Check whether we have to return the output file path.
//
//	if ( pszOutFileFullPathBufferPtr )
//		::strcpy( pszOutFileFullPathBufferPtr, szOutFullPath );
//
//	// Get a memory copy of the Resource.
//
//	HRSRC		hRes = ::FindResource( hResMod, pszName, pszType );
//	if ( hRes == NULL )
//		return FALSE;
//
//	HGLOBAL		hgResMem = ::LoadResource( hResMod, hRes );
//	if ( hgResMem == NULL )
//		return FALSE;
//
//	LPVOID		pRes = ::LockResource( hgResMem );
//	if ( pRes == NULL )
//		return FALSE;
//
//	DWORD		dwResSize = ::SizeofResource( hResMod, hRes );
//	if ( dwResSize == 0 )
//		return FALSE;
//
//	// Write the Driver File on disk.
//
//	FILE*			pfDriverFile = ::fopen( szOutFullPath, "wb" );
//	if ( pfDriverFile == NULL )
//		return FALSE;
//
//	// Write the file.
//
//	BOOL		bExitWithErr = FALSE;
//
//	if ( ::fwrite( pRes, 1, dwResSize, pfDriverFile ) != dwResSize )
//		bExitWithErr = TRUE;
//
//	::fclose( pfDriverFile );
//
//	// Exit, eventually, returning error.
//
//	if ( bExitWithErr )
//	{
//		::DeleteFile( szOutFullPath );
//		return FALSE;
//	}
//
//	// Return to the Caller.
//
//	return TRUE;
//}

static BOOL CopyFileToDriversDir ( LPTSTR pszOutFileFullPathBufferPtr, LPCTSTR pszSysFile )
{
	// Get the full path of the output file.

	char		szOutFullPath[ MAX_PATH * 2 ];
	if ( ::GetDriverOutFileFullPath( pszSysFile, szOutFullPath ) == FALSE )
		return FALSE;

	// Check whether we have to return the output file path.

	if ( pszOutFileFullPathBufferPtr )
		::strcpy( pszOutFileFullPathBufferPtr, szOutFullPath );

	// Copy the file.

	char		szModName[ MAX_PATH ] = "";
	::GetModuleFileName( NULL, szModName, MAX_PATH );

	char		drive[_MAX_DRIVE];
	char		dir[_MAX_DIR];
	char		fname[_MAX_FNAME];
	char		ext[_MAX_EXT];
	::_splitpath( szModName, drive, dir, fname, ext );

	char		source[ MAX_PATH * 2 ];
	::strcpy( source, drive );
	::strcat( source, dir );
	::strcat( source, pszSysFile );

	return ::CopyFile( source, szOutFullPath, FALSE );
}

/****************************************************************************
*
*    FUNCTION: InstallDriver( IN SC_HANDLE, IN LPCTSTR, IN LPCTSTR)
*
*    PURPOSE: Creates a driver service.
*
****************************************************************************/
static BOOL InstallDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName, IN LPCTSTR ServiceExe )
{
    SC_HANDLE  schService;

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //

    schService = CreateService( SchSCManager,          // SCManager database
                                DriverName,            // name of service
                                DriverName,            // name to display
                                SERVICE_ALL_ACCESS,    // desired access
                                SERVICE_KERNEL_DRIVER, // service type
                                SERVICE_DEMAND_START,  // start type
                                SERVICE_ERROR_NORMAL,  // error control type
                                ServiceExe,            // service's binary
                                NULL,                  // no load ordering group
                                NULL,                  // no tag identifier
                                NULL,                  // no dependencies
                                NULL,                  // LocalSystem account
                                NULL                   // no password
                                );
    if ( schService == NULL )
        return FALSE;

    CloseServiceHandle( schService );

    return TRUE;
}

/****************************************************************************
*
*    FUNCTION: RemoveDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Deletes the driver service.
*
****************************************************************************/
static BOOL RemoveDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName )
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );

    if ( schService == NULL )
        return FALSE;

    ret = DeleteService( schService );

    CloseServiceHandle( schService );

    return ret;
}

/****************************************************************************
*
*    FUNCTION: InstallVpcIce( void )
*
*    PURPOSE: Install the VPCIce Driver on the machine.
*
****************************************************************************/
BOOL InstallVpcIce ( HINSTANCE hResMod, LPCTSTR pszResName, LPCTSTR pszResType, LPCTSTR pszDriverName, LPCTSTR pszSysFileName )
{
	BOOL		bRetVal = FALSE;

	// Open the Service Control Manager on this computer.

	SC_HANDLE	schSCManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if ( schSCManager != NULL )
	{
		// Copy the Driver File in the "System32\Drivers" folder.

		char		szOutFullPath[ MAX_PATH * 2 ];
		BOOL		bDriversFldCopyRes = CopyFileToDriversDir ( szOutFullPath, pszSysFileName ); // ::CreateDriverFileFromAppResources( szOutFullPath, hResMod, pszResName, pszResType, pszSysFileName );
		if ( bDriversFldCopyRes )
		{
			// Install the driver.

			bRetVal = InstallDriver( schSCManager, pszDriverName, szOutFullPath );

			if ( bRetVal == FALSE && ::GetLastError() == ERROR_SERVICE_EXISTS )
				bRetVal = TRUE;
		}

		// Close the Service Control Manager.

		::CloseServiceHandle( schSCManager );
	}

	// Return to the Caller.

	return bRetVal;
}

/****************************************************************************
*
*    FUNCTION: RemoveVpcIce( void )
*
*    PURPOSE: Remove the VPCIce Driver from the machine.
*
****************************************************************************/
BOOL RemoveVpcIce ( LPCTSTR pszDriverName, LPCTSTR pszSysFileName )
{
	// Get the full path of the output file.

	char		szOutFullPath[ MAX_PATH * 2 ];
	if ( ::GetDriverOutFileFullPath( pszSysFileName, szOutFullPath ) == FALSE )
		return FALSE;

	// Try to delete the file on disk.

	::DeleteFile( szOutFullPath );

	// Open the Service Control Manager on this computer.

	SC_HANDLE	schSCManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if ( schSCManager != NULL )
	{
		// Remove the driver.

		RemoveDriver( schSCManager, pszDriverName );

		// Close the Service Control Manager.

		::CloseServiceHandle( schSCManager );
	}
	else
		return FALSE;

	// Return to the Caller.

	return TRUE;
}
