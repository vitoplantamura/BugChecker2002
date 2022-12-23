//---------------------------------------------------------
// u2kint.h - User To Kernel Mode Interfaces Header File.
// Copyright (c)2003 Vito Plantamura, VPC Technologies SRL. All Rights Reserved.
//---------------------------------------------------------

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

//############
// # Types. #
//############

#define MACRO_U2KGUID_LEN			0x10

#pragma pack( push, 1 )

	typedef struct _U2KSTUB			//
	{								// This structure HAS to be ALIGNED to Virtual Page Boundaries ( 0x1000 bytes )...
		BYTE		bINT3;			//
		BYTE		vGUID[ MACRO_U2KGUID_LEN ];
		BYTE		bRET;

	} U2KSTUB, *PU2KSTUB;

#pragma pack( pop )

//#############
// # Macros. #
//#############

#define MACRO_SYSTEM_PAGE_SIZE				( 0x1000 )

//

#ifdef __cplusplus
    #define U2KINT_EXT_C    extern "C"
#else
    #define U2KINT_EXT_C    extern
#endif

#ifndef INITU2KSTUB
	U2KINT_EXT_C const U2KSTUB g_u2kstub;
#else
	U2KINT_EXT_C const U2KSTUB g_u2kstub =
		{
			/* int3 */ 0xCC,
			/* guid */ { 0x99, 0xA8, 0xAC, 0x34, 0x94, 0xA4, 0x44, 0x2D, 0x8C, 0x93, 0xEB, 0x90, 0x83, 0xCC, 0xEE, 0x97 },
			/* ret  */ 0xC3
		};
#endif

//#################
// # Interfaces. #
//#################

//================
// === TYPES. ===
//================

#pragma pack( push, 1 )

	typedef enum _ENUM_U2KSYMTYP
	{
		U2KSYMTYP_BCS = 0

	} ENUM_U2KSYMTYP, *PENUM_U2KSYMTYP;

	typedef struct _U2KINT_SYMTABLEINFO
	{
		ULONG			ulOrdinal;	// = unique value in the current session. A kind of session guid...
		CHAR			szName[ MAX_PATH ];
		ULONG			ulSize;
		ENUM_U2KSYMTYP	eType;

	} U2KINT_SYMTABLEINFO, *PU2KINT_SYMTABLEINFO;

#pragma pack( pop )

//===================
// === COMMANDS. ===
//===================

//
// EAX = 0x00 -> IsDebuggerPresent (check out whether the Debugger is present or not).
//
// Return Value:
//  none.
//

#define U2KCMD_EAX_ISDEBUGGERPRESENT		0x00

//
// EAX = 0x01 -> GetSymTableInfo (get informations about a symbol table loaded in the debugger).
// EBX = index of the sym table.
// ECX = address of the receiving U2KINT_SYMTABLEINFO. TOUCH THE MEMORY FIRST !
//
// Return Value:
//  EAX = 1 if index is valid ; 0 otherwise.
//

#define U2KCMD_EAX_GETSYMTABLEINFO			0x01

//
// EAX = 0x02 -> GetSymMemInfo ( get informations about the debugger symbols memory).
//
// Return Value:
//  EAX = total memory size.
//  EBX = free memory size.
//

#define U2KCMD_EAX_GETSYMMEMINFO			0x02

//
// EAX = 0x03 -> AddSymTable (add the specified symbol table in the debugger).
// EBX = address of the NULL TERMINATED buffer that contains the file path. TOUCH THE MEMORY FIRST !
// ECX = size of path buffer including the NULL terminator.
//
// Return Value:
//  EAX = token ; 0 if failure.
//
// Notes:
//  After the call the load operation is still pending. To complete the load, send the "IOCTL_VPCICE_COMPLETE_SYMLOAD"
//  code to the Debugger specifying as input a DWORD containing the token returned by this service.
//

#define U2KCMD_EAX_ADDSYMTABLE				0x03

//
// EAX = 0x04 -> RemoveSymTable (remove the specified symbol table from the debugger memory).
// EBX = index of the sym table: i.e. the "U2KINT_SYMTABLEINFO.ulOrdinal" field value.
//
// Return Value:
//  EAX = 1 if index is valid ; 0 otherwise.
//

#define U2KCMD_EAX_REMOVESYMTABLE			0x04

//
// EAX = 0x05 -> GetHistoryLastLineIdAndDebuggerVerb (get the last printed line id in the history pane and the debugger verb, if specified).
// EBX, ECX, EDX = eventually used for restitution of complete verb info. (NOT IMPLEMENTED)
//
// Return Value:
//  EAX = last line id.
//  EBX = first valid line id.
//  ECX = verb id.
//   0 (DEBVERB_ECX_NONE)			-> no verb.
//   1 (DEBVERB_ECX_OPENDASHBOARD)	-> open user mode dashboard (no other info required or returned).
//  EDX = eventually used for returning informations related to the current verb. (NOT IMPLEMENTED)
//

#define U2KCMD_EAX_GETHISTORYLASTLINEIDANDDEBUGGERVERB	0x05

#define DEBVERB_ECX_NONE			0
#define DEBVERB_ECX_OPENDASHBOARD	1

//
// EAX = 0x06 -> GetHistoryLineText (get the text of the specified history line).
// EBX = pointer to a vector of 2 ulong32: the first is the start index, the second is
//  the end index. TOUCH THE MEMORY FIRST !
// ECX = address of the chars output buffer ( this will include the TERMINATOR ). Each line
//  is separated by the "\x0D\x0A" characters. TOUCH THE MEMORY FIRST !
// EDX = size in bytes/chars of the output buffer (including the NULL TERMINATOR).
//
// Return Value:
//  EAX = 1 if successful ; 0 otherwise.
//  EBX = validated start index.
//  ECX = validated end index.
//

#define U2KCMD_EAX_GETHISTORYLINETEXT		0x06

//
// EAX = 0x07 -> DownloadText (get the script or clipboard text from the debugger)
// EBX = 0 if script ; 1 if clipboard.
// ECX = address of the chars output buffer: this will be terminated by a NULL chr.
//  Each line will be separated by the "\x0D\x0A" characters. TOUCH THE MEMORY FIRST !
// EDX = size in bytes/chars of the output buffer, including the NULL TERMINATOR.
//
// Return Value:
//  EAX = 1 if successful ; 0 otherwise.
//

#define U2KCMD_EAX_DOWNLOADTEXT				0x07

//
// EAX = 0x08 -> UploadText (upload and set the script or clipboard text in the debugger)
// EBX = 0 if script ; 1 if clipboard.
// ECX = address of the NULL TERMINATED input buffer. Each line has to be separated by
//  the "\x0D\x0A" characters. TOUCH THE MEMORY FIRST !
// EDX = size in bytes/chars of the input buffer, including the NULL TERMINATOR.
//
// Return Value:
//  EAX = 1 if successful ; 0 otherwise.
//

#define U2KCMD_EAX_UPLOADTEXT				0x08

//
// EAX = 0x09 -> ExecuteCommand (execute a command)
// EBX = address of the NULL TERMINATED command chars buffer. TOUCH THE MEMORY FIRST !
// ECX = size in bytes/chars of command buffer, including the NULL Terminator.
//
// Return Value:
//  EAX = 1 if successful ; 0 otherwise.
//

#define U2KCMD_EAX_EXECUTECOMMAND			0x09

//
// EAX = 0x0A -> SetOsKeybLEDsStatusVar (set the variable that describes the status of the keyboard leds)
// EBX = "OsKeybLEDsStatus" Variable Value (bit0: scroll lock ; bit1: num lock ; bit2: caps lock)
//
// Return Value:
//  EAX = 1 if replaced var was different ; 0 otherwise.
//   If 1 the caller should issue a "U2KCMD_EAX_UPDATEKEYBLEDS" call to update the hardware.
//

#define U2KCMD_EAX_SETOSKEYBLEDSSTATUSVAR	0x0A

//
// EAX = 0x0B -> UpdateKeybLEDs (update the keyboard leds status according to the "OsKeybLEDsStatus" var value)
//
// Return Value:
//  none.
//

#define U2KCMD_EAX_UPDATEKEYBLEDS			0x0B
