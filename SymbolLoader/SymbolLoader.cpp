//
// SymbolLoader.cpp, Copyright (c)2010 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
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

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>
#include <dia2.h>

#include "..\Bin\BuildNumTracker\PROBUILD.H"

#include "..\Include\InstallVpcIce.h"
#include "..\Include\InstallVpcIceVideoHookDriver.h"
#include "..\Include\bcstrpub.h"
#include "..\Include\LoadFile.h"
#include "..\Include\BcsIndex.h"
#include "..\Include\StructuredFileUtils.h"
#include "..\Include\BugChkDat.h"

#include "CodeView.h"

//=========================================
// BugChk.Dat creation code.
//=========================================

//////////////////////////////////////////////////////////////////////////////////////////////////////

typedef HRESULT (STDAPICALLTYPE* PFNDllRegisterServer)();

static BOOLEAN IsNewMsPdbGuidSpecified( IN BYTE* pbGuid )
{
	// Check out Every Byte.

	for ( int i=0; i<MACRO_NEWMSPDB_GUIDLEN; i++ )
		if ( pbGuid[ i ] )
			return TRUE;

	return FALSE;
}

typedef BOOL ( __stdcall * PFNSymbolServerSetOptions ) (
                       UINT_PTR options,
                       ULONG64 data
);

typedef BOOL ( __stdcall * PFNSymbolServer ) (
                       LPCSTR params,
                       LPCSTR filename,
                       PVOID id,
                       DWORD two,
                       DWORD three,
                       LPSTR path
);

#define SSRVOPT_OVERWRITE           0x00004000L

static BCSSYMBOL* SymbolByName ( SBcsIndex* pBcsIndex, char* pSymbolName )
{
	for( int i=0; i<pBcsIndex->stBcsSymbols; i ++ )
	{
		BCSSYMBOL* pSym = & pBcsIndex->pBcsSymbols[ i ];
		CHAR* pSymName = & pBcsIndex->pSymNames[ pSym->ulNameOffsetInSymNames ];

		if ( ! ::strcmp( pSymName, pSymbolName ) )
			return pSym;
	}

	return NULL;
}

static VOID SkipLeaf( lfEasy* pl, UINT* puiSkip )
{
	if ( pl->leaf < LF_NUMERIC ) { * puiSkip += 2; return; }

	switch( pl->leaf )
	{
		case LF_CHAR:
		* puiSkip += 3; return;

		case LF_USHORT:
		* puiSkip += 4; return;

		case LF_SHORT:
		* puiSkip += 4; return;

		case LF_LONG:
		case LF_ULONG:
		* puiSkip += 6; return;

		case LF_QUADWORD:
		case LF_UQUADWORD:
		* puiSkip += 10; return;

		default: return;
	}
}

static charstring GuidToCharstring( GUID* pGuid )
{
	charstring		rv = "";

	for( int i=0; i<sizeof(GUID); i ++ )
	{
		CHAR			szBuff[ MAX_PATH ];
		::sprintf( szBuff, "%02x", ( (BYTE*) pGuid )[ i ] );
		rv += szBuff;
	}

	return rv;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

BYTE* LoadSymbols ( charstring csModuleFullPath, size_t& stBcsFullPath, charstring& csBcsFullPath, charstring& csMsPdbFullPath ) // http://support.microsoft.com/kb/148652
{
	::printf( "Getting symbols for \"%s\"...\r\n", csModuleFullPath.c_str () );

	BYTE*			pbBcsFullPath = NULL;

	stBcsFullPath = 0;
	csBcsFullPath = "";

	// paths/urls to required resources.

	charstring		csLocalSymsPath = "C:\\symbols";
	charstring		csMsdlSymUrl = "http://msdl.microsoft.com/download/symbols";

	charstring		csSymSrvFullPath = "symsrv.dll";

	// get info about the module.

	BCSTRANSPARAMS				params;
	::memset( & params, 0, sizeof( params ) );

	PDBVALIDATEPARAMS			out;
	::memset( & out, 0, sizeof( out ) );

	CHAR				szDbgFile[ MAX_PATH ] = "";
	charstring			csPrevPdbFullPath = "";

	for( int i=0; i < 2 ; i ++ )
	{
		PFNINFOFromMOD				pfnINFOFromMOD = NULL;
		PFNBCSFromPDB				pfnBCSFromPDB = NULL;

		HMODULE						bcstrans = ::LoadLibrary( "bcstrans.dll" );
		pfnINFOFromMOD = (PFNINFOFromMOD) ::GetProcAddress( bcstrans, "INFOFromMOD" );
		pfnBCSFromPDB = (PFNBCSFromPDB) ::GetProcAddress( bcstrans, "BCSFromPDB" );

		ENUM_TRANSERR	err;
		BOOLEAN			res;

		if ( i == 0 )
		{
			res = pfnINFOFromMOD(
				TRANSGEN_FIRST,
				(char*) csModuleFullPath.c_str (),
				& out,
				szDbgFile, sizeof( szDbgFile ),
				& err );
		}
		else
		{
			res = TRUE;
		}

		if ( res == FALSE )
		{
			::printf( "FATAL: Unable to read PE info for \"%s\".\r\n\r\n", csModuleFullPath.c_str () );
		}
		else
		{
			// download the PDB file.

			charstring		csDbgFile = szDbgFile;

			charstring::size_type	n = csDbgFile.find_last_of( '\\' );
			if ( n != charstring::npos )
				csDbgFile = csDbgFile.substr( n + 1 );

			ENUM_TRANSLEVEL		eTransLevel = TRANSLEVEL_SYMANDSOURCE;

			HMODULE				symsrv = ::LoadLibrary( csSymSrvFullPath.c_str () );

			PFNSymbolServerSetOptions pfnSymbolServerSetOptions = (PFNSymbolServerSetOptions) ::GetProcAddress( symsrv, "SymbolServerSetOptions" );
			PFNSymbolServer pfnSymbolServer = (PFNSymbolServer) ::GetProcAddress( symsrv, "SymbolServer" );

			VOID*				pvSsParam1 = NULL;
			DWORD				dwSsParam2 = 0;

			if ( ::IsNewMsPdbGuidSpecified( out.vbGuid ) )
			{
				pfnSymbolServerSetOptions( SSRVOPT_PARAMTYPE, (ULONG64) SSRVOPT_GUIDPTR );

				pvSsParam1 = (VOID*) out.vbGuid;
				dwSsParam2 = (DWORD) out.aAge;

				::printf( "Downloading \"%s\", Signature=\"%s\", Age=\"%x\"...\r\n", csDbgFile.c_str (), GuidToCharstring( (GUID*) pvSsParam1 ).c_str (), dwSsParam2 );
			}
			else
			{
				pfnSymbolServerSetOptions( SSRVOPT_PARAMTYPE, (ULONG64) SSRVOPT_DWORD );

				if ( out.sSig )
				{
					pvSsParam1 = (VOID*) out.sSig;
					dwSsParam2 = out.aAge;

					::printf( "Downloading \"%s\", Signature=\"%x\", Age=\"%x\"...\r\n", csDbgFile.c_str (), (DWORD) pvSsParam1, dwSsParam2 );
				}
				else
				{
					pvSsParam1 = (VOID*) out.dwTimeDateStamp;
					dwSsParam2 = out.dwSizeOfImage;

					eTransLevel = TRANSLEVEL_GETDBGINFOONLY;

					::printf( "Downloading \"%s\", TimeStamp=\"%x\", Size=\"%x\"...\r\n", csDbgFile.c_str (), (DWORD) pvSsParam1, dwSsParam2 );
				}
			}

			pfnSymbolServerSetOptions( SSRVOPT_PARENTWIN, (ULONG64) NULL );
			pfnSymbolServerSetOptions( SSRVOPT_UNATTENDED, (ULONG64) TRUE );
			pfnSymbolServerSetOptions( SSRVOPT_SECURE, (ULONG64) TRUE );
			pfnSymbolServerSetOptions( SSRVOPT_OVERWRITE, (ULONG64) TRUE );

			CHAR		szSymSrvPath[ MAX_PATH ] = "";
			BOOL		bSSRes = pfnSymbolServer( ( csLocalSymsPath + "*" + csMsdlSymUrl ).c_str (),
				csDbgFile.c_str (),
				pvSsParam1, dwSsParam2, 0,
				szSymSrvPath );

			if ( bSSRes == FALSE )
			{
				::printf( "FATAL: Download error. Make sure you are connected to the internet.\r\n\r\n" );
			}
			else
			{
				charstring		csPdbFullPath = szSymSrvPath;

				// translate the PDB into a BCS file.

				if ( csPdbFullPath.size () )
				{
					csBcsFullPath = csPdbFullPath.substr( 0, csPdbFullPath.size () - 4 ) + ".bcs";

					FILE*			pfBcsFullPath = ::fopen( csBcsFullPath.c_str (), "rb" );
					if (pfBcsFullPath) ::fclose( pfBcsFullPath );

					if ( pfBcsFullPath == NULL )
					{
						::memset( & params, 0, sizeof( params ) );

						params.eTransLevel = eTransLevel;

						params.vpszMsPdbModules[ 0 ] = (char*) csMsPdbFullPath.c_str ();

						if ( ::IsNewMsPdbGuidSpecified( out.vbGuid ) )
						{
							::memcpy( params.pdbvpValidate.vbGuid, out.vbGuid, sizeof (out.vbGuid) );
							params.pdbvpValidate.aAge = out.aAge;
						}
						else if ( csPrevPdbFullPath.size () )
						{
							char		drive[_MAX_DRIVE];
							char		dir[_MAX_DIR];
							char		fname[_MAX_FNAME];
							char		ext[_MAX_EXT];

							::_splitpath( csPrevPdbFullPath.c_str () , drive, dir, fname, ext );

							charstring	csFp = charstring( drive ) + dir + csDbgFile;
							::CopyFile( csPdbFullPath.c_str (), csFp.c_str (), FALSE );

							csPdbFullPath = csPrevPdbFullPath;
						}

						ENUM_TRANSERR			err;

						BOOLEAN			r = pfnBCSFromPDB(
							TRANSGEN_FIRST,
							(char*) csPdbFullPath.c_str (),
							(char*) csBcsFullPath.c_str (),
							NULL, & params, & err );

						if ( r == FALSE )
						{
							::DeleteFile( csBcsFullPath.c_str () );
							csBcsFullPath = "";

							CHAR			szErr[ MAX_PATH ] = "Undefined";
							switch( err )
							{
							case TRANSERR_SUCCESS:
								::strcpy( szErr, "ERR_FAILURE (may be caused by a wrong or missing MsPdbXX.dll)" ); break;
							case TRANSERR_GENERR:
								::strcpy( szErr, "ERR_GENERR" ); break;
							case TRANSERR_MSPDBDLL:
								::strcpy( szErr, "ERR_MSPDBDLL" ); break;
							case TRANSERR_PARAMS:
								::strcpy( szErr, "ERR_PARAMS" ); break;
							case TRANSERR_OPENPDB:
								::strcpy( szErr, "ERR_OPENPDB (may be caused by a wrong or missing MsPdbXX.dll)" ); break;
							case TRANSERR_OPENBCS:
								::strcpy( szErr, "ERR_OPENBCS" ); break;
							case TRANSERR_OPENDBI:
								::strcpy( szErr, "ERR_OPENDBI" ); break;
							case TRANSERR_OPENTPI:
								::strcpy( szErr, "ERR_OPENTPI" ); break;
							case TRANSERR_OPENPUB:
								::strcpy( szErr, "ERR_OPENPUB" ); break;
							case TRANSERR_WRITEBCS:
								::strcpy( szErr, "ERR_WRITEBCS" ); break;
							case TRANSERR_READOMAPF:
								::strcpy( szErr, "ERR_READOMAPF" ); break;
							case TRANSERR_READOMAPT:
								::strcpy( szErr, "ERR_READOMAPT" ); break;
							case TRANSERR_READFPO:
								::strcpy( szErr, "ERR_READFPO" ); break;
							case TRANSERR_READSECS:
								::strcpy( szErr, "ERR_READSECS" ); break;
							case TRANSERR_READOSECS:
								::strcpy( szErr, "ERR_READOSECS" ); break;
							case TRANSERR_PDBEXT:
								::strcpy( szErr, "ERR_PDBEXT" ); break;
							case TRANSERR_READPDB:
								::strcpy( szErr, "ERR_READPDB" ); break;
							case TRANSERR_PDBFORMAT:
								::strcpy( szErr, "ERR_PDBFORMAT" ); break;
							case TRANSERR_OPENDBG:
								::strcpy( szErr, "ERR_OPENDBG" ); break;
							case TRANSERR_READDBG:
								::strcpy( szErr, "ERR_READDBG" ); break;
							case TRANSERR_DBGFORMAT:
								::strcpy( szErr, "ERR_DBGFORMAT" ); break;
							case TRANSERR_MISMDBG:
								::strcpy( szErr, "ERR_MISMDBG" ); break;
							case TRANSERR_MISSPDBNAME:
								::strcpy( szErr, "ERR_MISSPDBNAME" ); break;
							case TRANSERR_READDBGOMAPF:
								::strcpy( szErr, "ERR_READDBGOMAPF" ); break;
							case TRANSERR_READDBGOMAPT:
								::strcpy( szErr, "ERR_READDBGOMAPT" ); break;
							case TRANSERR_READDBGFPO:
								::strcpy( szErr, "ERR_READDBGFPO" ); break;
							case TRANSERR_OPENMOD:
								::strcpy( szErr, "ERR_OPENMOD" ); break;
							case TRANSERR_READMOD:
								::strcpy( szErr, "ERR_READMOD" ); break;
							case TRANSERR_NOMODMEM:
								::strcpy( szErr, "ERR_NOMODMEM" ); break;
							case TRANSERR_NOMODDBG:
								::strcpy( szErr, "ERR_NOMODDBG" ); break;
							case TRANSERR_OUTBUFDIM:
								::strcpy( szErr, "ERR_OUTBUFDIM" ); break;
							case TRANSERR_PDBPUBSYM:
								::strcpy( szErr, "ERR_PDBPUBSYM" ); break;
							case TRANSERR_PDBTYPS:
								::strcpy( szErr, "ERR_PDBTYPS" ); break;
							case TRANSERR_PDBMODS:
								::strcpy( szErr, "ERR_PDBMODS" ); break;
							case TRANSERR_PCKSRC:
								::strcpy( szErr, "ERR_PCKSRC" ); break;
							}

							::printf( "FATAL: Symbol translation error: %s.\r\n\r\n", szErr );
						}
						else
						{
							csPrevPdbFullPath = csPdbFullPath;
						}
					}
				}
			}

			::FreeLibrary( symsrv );
		}

		::FreeLibrary( bcstrans );

		// read the BCS file.

		if ( csBcsFullPath.size () )
		{
			pbBcsFullPath = ::LoadFile( (char*) csBcsFullPath.c_str (), & stBcsFullPath, NULL );
			if ( pbBcsFullPath == NULL )
			{
				if ( i==0 && ::strlen( params.pdbvpValidate.szPdbFilename ) )
				{
					::strcpy( szDbgFile, params.pdbvpValidate.szPdbFilename );

					out.sSig = params.pdbvpValidate.sSig;
					out.aAge = params.pdbvpValidate.aAge;

					continue;
				}
				else
				{
					::printf( "FATAL: Symbol translation error. Unable to continue.\r\n\r\n" );
				}
			}
		}

		break;
	}

	return pbBcsFullPath;
}

BOOL GetOsSpecificSettings ( SOsSpecificSettings* pOsSpec, charstring& csNtOsKrnlFullPath, charstring& csMsPdbFullPath )
{
	BOOL			bRetVal = FALSE;

	::memset( pOsSpec, 0, sizeof( SOsSpecificSettings ) );

	size_t			stNtKernelBcs;
	charstring		csNtKernelBcs;
	BYTE*			pbNtKernelBcs = ::LoadSymbols( csNtOsKrnlFullPath, stNtKernelBcs, csNtKernelBcs, csMsPdbFullPath );

	if ( pbNtKernelBcs )
	{
		// parse the BCS files.

		BYTE*		ptr = pbNtKernelBcs;

		SBcsIndex	NtKernelBcsIndex;
		::memset( & NtKernelBcsIndex, 0, sizeof (NtKernelBcsIndex) );

		if ( ::BcsValidate( & ptr, pbNtKernelBcs, stNtKernelBcs ) == NULL ||
			::BcsCreateIndex( & ptr, pbNtKernelBcs, stNtKernelBcs, & NtKernelBcsIndex ) == FALSE )
		{
			::printf( "FATAL: Symbol translation error. Unable to continue.\r\n\r\n" );
		}
		else
		{
			// symbols.

			BCSSYMBOL*		pPsIdleProcess = ::SymbolByName( & NtKernelBcsIndex, "PsIdleProcess" );
			BCSSYMBOL*		pPsInitialSystemProcess = ::SymbolByName( & NtKernelBcsIndex, "PsInitialSystemProcess" );
			BCSSYMBOL*		pMiMapViewOfImageSection = ::SymbolByName( & NtKernelBcsIndex, "MiMapViewOfImageSection" );

			if ( pPsIdleProcess && pPsInitialSystemProcess )
				pOsSpec->MACRO_IDLEPROCESS_OFFSET_REL_INITSYSP = (int) pPsIdleProcess->ulAddressFromModBase - (int) pPsInitialSystemProcess->ulAddressFromModBase;

			if ( pMiMapViewOfImageSection )
				pOsSpec->MACRO_MAPVIEWOFIMAGESECTION_ADDR = pMiMapViewOfImageSection->ulAddressFromModBase;

			// types.

			TI				tiKProcess = -1;
			TI				tiEProcess = -1;
			TI				tiKThread = -1;
			TI				tiEThread = -1;
			TI				tiKPcr = -1;
			TI				tiDriverSec = -1;
			TI				tiClientId = -1;
			TI				tiKApcState = -1;

			BYTE*			pTypes = NtKernelBcsIndex.pTypes;

			int				i = 0;
			while( pTypes - NtKernelBcsIndex.pTypes < NtKernelBcsIndex.stTypes )
			{
				TYPTYPE*		pTypType = (TYPTYPE*) pTypes;

				switch ( pTypType->leaf )
				{
				case LF_STRUCTURE:

					lfClass*		l = (lfClass*) & pTypType->leaf;
					if ( l->field )
					{
						UINT			s = 0;
						::SkipLeaf( (lfEasy*) l->data, & s );

						CHAR*			pszName = (CHAR*) ( l->data + s );

						if ( ! ::strcmp( pszName, "_KPROCESS" ) )
							tiKProcess = l->field;
						else if ( ! ::strcmp( pszName, "_KTHREAD" ) )
							tiKThread = l->field;
						else if ( ! ::strcmp( pszName, "_KPCR" ) )
							tiKPcr = l->field;
						else if ( ! ::strcmp( pszName, "_LDR_DATA_TABLE_ENTRY" ) )
							tiDriverSec = l->field;
						else if ( ! ::strcmp( pszName, "_EPROCESS" ) )
							tiEProcess = l->field;
						else if ( ! ::strcmp( pszName, "_ETHREAD" ) )
							tiEThread = l->field;
						else if ( ! ::strcmp( pszName, "_CLIENT_ID" ) )
							tiClientId = l->field;
						else if ( ! ::strcmp( pszName, "_KAPC_STATE" ) )
							tiKApcState = l->field;
					}

					break;
				}

				pTypes += pTypType->len + sizeof ( unsigned short );
				i ++;
			}

			// succeeded ?? (yes, if we got publics and types, in this case)

			if ( tiKProcess != -1 && pMiMapViewOfImageSection )
			{
				bRetVal = TRUE;
				::printf( "Symbol translation succeeded.\r\n" );
			}
			else
			{
				::printf( "FATAL: Symbol translation error. Unable to continue.\r\n\r\n" );
			}

			// fields.

			WORD		woffCid = 0xFFFF;
			WORD		woffUniqueThread = 0xFFFF;
			WORD		woffApcState = 0xFFFF;
			WORD		woffProcess = 0xFFFF;

			pTypes = NtKernelBcsIndex.pTypes;

			i = 0;
			while( pTypes - NtKernelBcsIndex.pTypes < NtKernelBcsIndex.stTypes )
			{
				TYPTYPE*		pTypType = (TYPTYPE*) pTypes;
				INT				len = pTypType->len + sizeof ( unsigned short );

				TI				ti = NtKernelBcsIndex.pTypeIds[ i ];

				if ( pTypType->leaf == LF_FIELDLIST && ( ti==tiKProcess || ti==tiKThread || ti==tiKPcr || ti==tiDriverSec || ti==tiEProcess || ti==tiEThread || ti==tiClientId || ti==tiKApcState ) )
				{
					lfFieldList*	f = (lfFieldList*) & pTypType->leaf;
					lfEasy*			lfe = (lfEasy*) f->data;

					while( (BYTE*) lfe - (BYTE*) pTypType < len )
					{
						switch ( lfe->leaf )
						{
						case LF_MEMBER:
							{
								lfMember*		m = (lfMember*) & lfe->leaf;
								lfe = (lfEasy*) ( (BYTE*) lfe + sizeof( lfMember ) );
								WORD*			offset = (WORD*) lfe;
								UINT			s = 0;
								::SkipLeaf( lfe, & s );
								lfe = (lfEasy*) ( (BYTE*) lfe + s );
								CHAR*			name = (CHAR*) lfe;
								lfe = (lfEasy*) ( (BYTE*) lfe + ::strlen( name ) + 1 );

								while( (BYTE*) lfe - (BYTE*) pTypType < len ) if ( (*(BYTE*)lfe) & 0xF0 ) lfe = (lfEasy*) ( (BYTE*) lfe + 1 ); else break;

								if ( ti==tiKProcess )
								{
									if ( ! ::strcmp( name, "DirectoryTableBase" ) ) pOsSpec->MACRO_CR3_FIELDOFFSET_IN_KPEB = * offset; // default value = ( 0x18 ) for Windows 2000 SP4 (DirectoryTableBase)
								}
								else if ( ti==tiKThread )
								{
									if ( ! ::strcmp( name, "ApcState" ) ) woffApcState = * offset;

									if ( woffApcState != 0xFFFF && woffProcess != 0xFFFF )
										pOsSpec->MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB = woffApcState + woffProcess; // default value = ( 0x44 ) for Windows 2000 SP4 (ApcState->Process)
								}
								else if ( ti==tiKApcState )
								{
									if ( ! ::strcmp( name, "Process" ) ) woffProcess = * offset;

									if ( woffApcState != 0xFFFF && woffProcess != 0xFFFF )
										pOsSpec->MACRO_KPEBPTR_FIELDOFFSET_IN_KTEB = woffApcState + woffProcess; // default value = ( 0x44 ) for Windows 2000 SP4 (ApcState->Process)
								}
								else if ( ti==tiKPcr )
								{
									if ( ! ::strcmp( name, "PrcbData" ) ) pOsSpec->MACRO_KTEBPTR_FIELDOFFSET_IN_PCR = * offset + 4; // default value = ( 0x124 ) for Windows 2000 SP4 (PrcbData+4)
									if ( ! ::strcmp( name, "Irql" ) ) pOsSpec->MACRO_CURRIRQL_FIELDOFFSET_IN_PCR = * offset; // default value = ( 0x24 ) for Windows 2000 SP4 (Irql)
								}
								else if ( ti==tiDriverSec )
								{
									if ( ! ::strcmp( name, "DllBase" ) ) pOsSpec->MACRO_IMAGEBASE_FIELDOFFSET_IN_DRVSEC = * offset; // default value = ( 0x18 ) for Windows 2000 SP4 (DllBase)
									if ( ! ::strcmp( name, "FullDllName" ) ) pOsSpec->MACRO_IMAGENAME_FIELDOFFSET_IN_DRVSEC = * offset; // default value = ( 0x24 ) for Windows 2000 SP4 (FullDllName)
								}
								else if ( ti==tiEProcess )
								{
									if ( ! ::strcmp( name, "UniqueProcessId" ) ) pOsSpec->MACRO_PID_FIELDOFFSET_IN_KPEB = * offset; // default value = ( 0x9C ) for Windows 2000 SP4 (UniqueProcessId)
									if ( ! ::strcmp( name, "VadRoot" ) ) pOsSpec->MACRO_VADROOT_FIELDOFFSET_IN_KPEB = * offset; // default value = ( 0x194 ) for Windows 2000 SP4 (VadRoot)
									if ( ! ::strcmp( name, "ActiveProcessLinks" ) ) pOsSpec->MACRO_ACTVPROCLINKS_FIELDOFFSET_IN_KPEB = * offset; // default value = ( 0xA0 ) for Windows 2000 SP4 (ActiveProcessLinks)
									if ( ! ::strcmp( name, "ImageFileName" ) ) pOsSpec->MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB = * offset; // default value = ( 0x1FC ) for Windows 2000 SP4 (ImageFileName)
								}
								else if ( ti==tiEThread )
								{
									if ( ! ::strcmp( name, "Cid" ) ) woffCid = * offset;

									if ( woffCid != 0xFFFF && woffUniqueThread != 0xFFFF )
										pOsSpec->MACRO_TID_FIELDOFFSET_IN_KTEB = woffCid + woffUniqueThread; // default value = ( 0x1E4 ) for Windows 2000 SP4 (Cid->UniqueThread)
								}
								else if ( ti==tiClientId )
								{
									if ( ! ::strcmp( name, "UniqueThread" ) ) woffUniqueThread = * offset;

									if ( woffCid != 0xFFFF && woffUniqueThread != 0xFFFF )
										pOsSpec->MACRO_TID_FIELDOFFSET_IN_KTEB = woffCid + woffUniqueThread; // default value = ( 0x1E4 ) for Windows 2000 SP4 (Cid->UniqueThread)
								}
							}
							break;

						default:
							lfe = (lfEasy*) 0xFFFFFFFF; break;
						}
					}
				}

				pTypes += len;
				i ++;
			}
		}
	}

	if ( pbNtKernelBcs )
		::free( pbNtKernelBcs );

	return bRetVal;
}

static VOID CreateBugChkDat ( charstring& csNtOsKrnlFp, charstring& csMsPdbFp, BOOL _local = FALSE )
{
	printf( "--- --- --- Generating OS-Specific BugChk.Dat --- --- ---\r\n");

	// DIA requires COM...

	::CoInitialize( NULL );

	IDiaDataSource*			pDiaDataSource = NULL;

	HRESULT hr = CoCreateInstance( __uuidof(DiaSource),
		NULL, 
		CLSCTX_INPROC_SERVER, 
		__uuidof(IDiaDataSource),
		(void **) & pDiaDataSource );

	if( hr == S_OK && pDiaDataSource )
	{
		pDiaDataSource->Release ();
	}
	else
	{
		printf( "Registering DIA...\r\n" );

		hr = E_FAIL;

		HMODULE		hinst = ::LoadLibrary ( "msdia80.dll" );
		if ( hinst )
		{
			PFNDllRegisterServer	pfn = (PFNDllRegisterServer) ::GetProcAddress ( hinst, "DllRegisterServer" );
			if ( pfn )
				hr = pfn();

			::FreeLibrary( hinst );
		}

		if ( hr != S_OK )
		{
			printf( "FATAL: DIA registration failed (err=%X). Make sure \"msdia80.dll\" is available and run BcUtil as an Administrator.\r\n\r\n", (DWORD) hr );
			return;
		}
		else
		{
			printf( "DIA registration succeeded.\r\n" );
		}
	}

	// get os specific info.

	SOsSpecificSettings			sOsSpec;
	if ( ! ::GetOsSpecificSettings( & sOsSpec,
		csNtOsKrnlFp,
		csMsPdbFp ) )
	{
		return;
	}

	// update/create the BugChk.Dat file.

	charstring			csBugChkDatFilePath = "";
	SStrFileNode*		pBugChkDat = NULL;

	CHAR			szSysDir[ MAX_PATH ] = "";
	if ( _local )
	{
		char		drive[_MAX_DRIVE];
		char		dir[_MAX_DIR];
		char		fname[_MAX_FNAME];
		char		ext[_MAX_EXT];

		char		szModuleName[ MAX_PATH ] = "";
		::GetModuleFileName( NULL, szModuleName, MAX_PATH );

		::_splitpath( szModuleName, drive, dir, fname, ext );
		::strcpy( szSysDir, drive );
		::strcat( szSysDir, dir );
	}
	else
	{
		::GetSystemDirectory( szSysDir, sizeof( szSysDir ) );
	}

	charstring		csDrvDir = szSysDir;

	if ( csDrvDir.size() )
	{
		CHAR		cLastChr = csDrvDir[ csDrvDir.size() - 1 ];
		if ( cLastChr != '\\' && cLastChr != '/' )
			csDrvDir += "\\";

		if ( ! _local )
			csDrvDir += "drivers\\" MACRO_BUGCHKDAT_FILENAME;
		else
			csDrvDir += MACRO_BUGCHKDAT_FILENAME;

		// Store the Path.

		csBugChkDatFilePath = csDrvDir;

		// Read from the File.

		BOOL		res = ::LoadStructuredFile(
			::CharStringToWideString( csBugChkDatFilePath ).c_str(),
			& pBugChkDat, 1000 );

		BOOL		bSave = FALSE;

		if ( TRUE /*res == FALSE*/ ) // always save.
		{
			// Create a Default version of the "bugchk.dat" File.
			MakeDefaultBugChkDat( & sOsSpec, csBugChkDatFilePath, & pBugChkDat );

			// Save the New File.
			bSave = TRUE;
		}
		//else
		//{
		//	//
		//	// Normalize the Contents of the File.
		//	// If required, save the New Version.
		//	//

		//	if ( NormalizeBugChkDat( & sOsSpec, pBugChkDat ) )
		//		bSave = TRUE;
		//}

		// save the file.

		if ( bSave )
		{
			::printf( "Saving file to: \"%s\"...\r\n", csBugChkDatFilePath.c_str () );

			if ( ::SaveBugChkDat ( csBugChkDatFilePath, pBugChkDat ) )
				printf( "Operation succeeded.\r\n\r\n");
			else
				printf( "Operation failed (run BcUtil as an Administrator).\r\n\r\n");
		}
	}
}

//=========================================
// command line starter.
//=========================================

#define MACRO_REGISTRY_APP_PATH			"SOFTWARE\\VPC Technologies SRL\\BugChecker"
#define MACRO_REGISTRY_MSPDB_VALUE		"MsPdbPath"

static void PrintUsage ( BOOL printVmWare = FALSE )
{
	printf( "Steps to installing BugChecker on a system:\r\n\r\n" );
	printf( " (/1) Generate a BugChk.Dat file. This file must be placed in the C:\\Windows\\System32\\Drivers directory on the target system.\r\n\r\n" );
	printf( " (/2) Set \"EnforceWriteProtection\" in the registry to 0.\r\n\r\n" );
	printf( " (/3) Install BugCheckerVideo (reboot required) and (/4) BugChecker. The command \"NET START VPCICE\" will start BugChecker.\r\n\r\n" );
	printf( " (/vmware) Display further information about installing BugChecker on a VmWare Virtual Machine.\r\n\r\n" );
	printf( " (/local) Implies /1 or /install: pick the NtOsKrnl.exe file from the BcUtil directory and also generate the BugChk.Dat file in that directory. If not specified, BcUtil picks the current system NtOsKrnl.exe file, and creates BugChk.Dat in the System32\\Drivers folder.\r\n\r\n");
	printf( " (/mspdb) Ask for the MSPDBxx.DLL directory, and store the path in the registry for later use. MSPDB50/60/70/71/80.dll can be specified (VC 5.0 to Visual Studio .NET 2005 supported).\r\n\r\n");

	if ( printVmWare )
	{
		printf( "==============================================================\r\n\r\n" );

		printf( "If you want to install BugChecker in a virtual machine inside VmWare, open the configuration file (.vmx) for the virtual machine in a text editor and add the following two lines:\r\n\r\n");
		printf( "vmmouse.present = FALSE\r\n");
		printf( "svga.maxFullscreenRefreshTick = 5\r\n");

		printf( "\r\n==============================================================\r\n\r\n" );
	}

	printf( "usage: bcutil [/1][/2][/3][/4][/local] or [/install (same as /1/2/3/4)] or [/uninstall (same as /-2/-3/-4)] or [/vmware] or [/mspdb]\r\n" );
}

int main(int argc, char* argv[])
{
	printf( "\r\n==============================================================\r\n" );
	printf( "   BugChecker Utility Program (BcUtil.exe) Version 0.1.%s.\r\n", __PROGRAMBUILD__ );
	printf( "    Architecture/Platform: 32-bit 80x86 Win2000/XP/S2003.\r\n");
	printf( "Copyright (C) 2002-2010 Vito Plantamura, VPC Technologies SRL.\r\n" );
	printf( "                     All rights reserved.\r\n" );
	printf( "==============================================================\r\n\r\n" );

	if ( argc <= 1 )
	{
		PrintUsage ();
		return 0;
	}

	int		_1 = 0;
	int		_2 = 0;
	int		_3 = 0;
	int		_4 = 0;
	int		_vmware = 0;
	int		_local = 0;
	int		_mspdb = 0;

	for( int i = 1; i < argc; i ++ )
	{
		char			p[ MAX_PATH ];
		strcpy( p, argv[ i ] );
		_strlwr( p );

		char*			ptr = p;

		while( ptr )
		{
			char*			ptr2;

			if ( ptr2 = strstr( ptr, "/1" ) )
				_1 ++;
			if ( ptr2 = strstr( ptr, "/2" ) )
				_2 ++;
			if ( ptr2 = strstr( ptr, "/3" ) )
				_3 ++;
			if ( ptr2 = strstr( ptr, "/4" ) )
				_4 ++;
			if ( ptr2 = strstr( ptr, "/-1" ) )
				_1 --;
			if ( ptr2 = strstr( ptr, "/-2" ) )
				_2 --;
			if ( ptr2 = strstr( ptr, "/-3" ) )
				_3 --;
			if ( ptr2 = strstr( ptr, "/-4" ) )
				_4 --;
			if ( ptr2 = strstr( ptr, "/install" ) )
			{
				_1 ++;
				_2 ++;
				_3 ++;
				_4 ++;
			}
			if ( ptr2 = strstr( ptr, "/uninstall" ) )
			{
				_1 --;
				_2 --;
				_3 --;
				_4 --;
			}
			if ( ptr2 = strstr( ptr, "/vmware" ) )
				_vmware ++;
			if ( ptr2 = strstr( ptr, "/local" ) )
				_local ++;
			if ( ptr2 = strstr( ptr, "/mspdb" ) )
				_mspdb ++;

			ptr = ptr2;
			if ( ptr ) ptr ++;
		}
	}

	if ( ( _1 == 0 && _2 == 0 && _3 == 0 && _4 == 0 ) || 
		  ( _1 < 0 && _2 == 0 && _3 == 0 && _4 == 0 ) ||
		  _vmware )
	{
		if ( ! _mspdb )
		{
			PrintUsage ( _vmware != 0 );
			return 0;
		}
	}

	//============
	// execution.
	//============

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _mspdb )
	{
		OPENFILENAME			ofn;
		char					szFileName[ MAX_PATH ] = "";

		ZeroMemory( & ofn, sizeof( ofn ) );

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "MsPdb 50/60/70/71 -or- 80.dll\0mspdb50.dll;mspdb60.dll;mspdb70.dll;mspdb71.dll;mspdb80.dll\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		if( ::GetOpenFileName( & ofn ) && ::strlen( szFileName ) )
		{
			BOOL		ok = FALSE;

			HKEY		hKey;
			LONG		lRet = ::RegCreateKeyEx( HKEY_LOCAL_MACHINE,
				MACRO_REGISTRY_APP_PATH, NULL, NULL, 0, KEY_ALL_ACCESS, NULL, & hKey, NULL );
			if ( lRet == ERROR_SUCCESS )
			{
				if ( ::RegSetValueEx( hKey, MACRO_REGISTRY_MSPDB_VALUE, NULL, REG_SZ, (BYTE*) szFileName, ::strlen( szFileName ) + 1 ) == ERROR_SUCCESS )
					ok = TRUE;

				::RegCloseKey( hKey );
			}

			if ( ok )
				printf( "Operation succeeded.\r\n\r\n");
			else
				printf( "Operation failed (run BcUtil as an Administrator).\r\n\r\n");
		}
		else
		{
			printf( "Operation failed. This version of BcUtil requires a file named MSPDBxx.dll (xx stands for 50, 60, 70, 71 or 80) from an installation of Visual Studio (VC 5.0 to Visual Studio .NET 2005 supported). This file is usually located in the X:\\Program Files\\Microsoft Visual Studio X\\CommonX\\IDE folder.\r\n\r\n");
		}

		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _1 > 0 )
	{
		charstring			csNtOsKrnlFp = "";
		charstring			csMsPdbFp = "";

		// get the MsPdb dll path from the registry...

		BOOL		ok = FALSE;

		HKEY		hKey;
		LONG		lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			MACRO_REGISTRY_APP_PATH, 0,
			KEY_ALL_ACCESS, &hKey);
		if ( lRet == ERROR_SUCCESS )
		{
			CHAR			szData[ MAX_PATH ];
			DWORD			dwData = sizeof( szData );
			if ( ::RegQueryValueEx( hKey, MACRO_REGISTRY_MSPDB_VALUE, NULL, NULL, (BYTE*) szData, & dwData ) == ERROR_SUCCESS && dwData )
			{
				szData[ dwData ] = '\0';
				csMsPdbFp = szData;
				ok = TRUE;
			}

			::RegCloseKey( hKey );
		}

		if ( ! ok )
		{
			::printf( "FATAL: this version of BcUtil requires a file named MSPDBxx.dll (xx stands for 50, 60, 70, 71 or 80) from an installation of Visual Studio (VC 5.0 to Visual Studio .NET 2005 supported). This file is usually located in the X:\\Program Files\\Microsoft Visual Studio X\\CommonX\\IDE folder. RUN \"BCUTIL /MSPDB\" TO SPECIFY THIS PATH (ITS VALUE WILL BE SAVED IN THE REGISTRY FOR LATER USE).\r\n\r\n" );
			return 0;
		}

		// set the path to the ntoskrnl file.

		CHAR			szSysDir[ MAX_PATH ] = "";
		if ( _local )
		{
			char		drive[_MAX_DRIVE];
			char		dir[_MAX_DIR];
			char		fname[_MAX_FNAME];
			char		ext[_MAX_EXT];

			char		szModuleName[ MAX_PATH ] = "";
			::GetModuleFileName( NULL, szModuleName, MAX_PATH );

			::_splitpath( szModuleName, drive, dir, fname, ext );
			::strcpy( szSysDir, drive );
			::strcat( szSysDir, dir );
		}
		else
		{
			::GetSystemDirectory( szSysDir, sizeof( szSysDir ) );
		}

		charstring		csDrvDir = szSysDir;

		if ( csDrvDir.size() )
		{
			CHAR		cLastChr = csDrvDir[ csDrvDir.size() - 1 ];
			if ( cLastChr != '\\' && cLastChr != '/' )
				csDrvDir += "\\";

			csNtOsKrnlFp = csDrvDir + "ntoskrnl.exe";
		}

		// create the bugchk.dat file.

		::CreateBugChkDat (
				csNtOsKrnlFp,
				csMsPdbFp,
				_local != 0
			);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _2 > 0 )
	{
		printf( "Setting \"EnforceWriteProtection\" in the registry...\r\n");
		BOOL			bRetVal = FALSE;

		HKEY		hKey;
		LONG		lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management", 0,
			KEY_ALL_ACCESS, &hKey);
		if ( lRet == ERROR_SUCCESS )
		{
			DWORD			dwData = 0;
			lRet = ::RegSetValueEx( hKey, "EnforceWriteProtection", NULL, REG_DWORD, (BYTE*) & dwData, sizeof( dwData ) );
			if ( lRet == ERROR_SUCCESS )
				bRetVal = TRUE;

			::RegCloseKey( hKey );
		}

		if ( bRetVal )
			printf( "Operation succeeded.\r\n\r\n");
		else
			printf( "Operation failed (run BcUtil as an Administrator).\r\n\r\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _3 > 0 )
	{
		printf( "Installing BugCheckerVideo (reboot required)...\r\n");

		if ( ::InstallVpcIceVideoHookDriver( NULL, NULL, NULL, "vpcicevd", "vpcicevd.sys" ) )
			printf( "Operation succeeded.\r\n\r\n");
		else
			printf( "Operation failed (run BcUtil as an Administrator or try to reboot the system).\r\n\r\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _4 > 0 )
	{
		printf( "Installing BugChecker...\r\n");

		if ( ::InstallVpcIce( NULL, NULL, NULL, "vpcice", "vpcice.sys" ) )
			printf( "Operation succeeded.\r\n\r\n");
		else
			printf( "Operation failed (run BcUtil as an Administrator or try to reboot the system).\r\n\r\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _2 < 0 )
	{
		printf( "Resetting \"EnforceWriteProtection\" in the registry...\r\n");

		BOOL			bRetVal = FALSE;

		HKEY		hKey;
		LONG		lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management", 0,
			KEY_ALL_ACCESS, &hKey);
		if ( lRet == ERROR_SUCCESS )
		{
			lRet = ::RegDeleteValue( hKey, "EnforceWriteProtection" );
			if ( lRet == ERROR_SUCCESS || lRet == 2 )
				bRetVal = TRUE;

			::RegCloseKey( hKey );
		}

		if ( bRetVal )
			printf( "Operation succeeded.\r\n\r\n");
		else
			printf( "Operation failed (run BcUtil as an Administrator).\r\n\r\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _3 < 0 )
	{
		printf( "Uninstalling BugCheckerVideo...\r\n");

		if ( ::RemoveVpcIceVideoHookDriver( "vpcicevd", "vpcicevd.sys" ) )
			printf( "Operation succeeded.\r\n\r\n");
		else
			printf( "Operation failed (run BcUtil as an Administrator or try to reboot the system).\r\n\r\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( _4 < 0 )
	{
		printf( "Uninstalling BugChecker...\r\n");

		if ( ::RemoveVpcIce( "vpcice", "vpcice.sys" ) )
			printf( "Operation succeeded.\r\n\r\n");
		else
			printf( "Operation failed (run BcUtil as an Administrator or try to reboot the system).\r\n\r\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	return 0;
}
