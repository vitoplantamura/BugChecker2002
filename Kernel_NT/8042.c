/***************************************************************************************
  *
  * 8042.c - VpcICE "8042 (Keyboard)" Routines Source File.
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

//========================
// Required Header Files.
//========================

#include "..\Include\8042.h"
#include "..\Include\apic.h"

//==========
// Externs.
//==========

extern DWORD VmWareVersion;
extern BOOLEAN _8042MinimalProgramming;

//=================================================
// WaitForOutputRegisterReady Function Definition.
//=================================================

static VOID __declspec( naked ) WaitForOutputRegisterReady( VOID )
{
	__asm
	{
		push		eax
		push		ecx

		mov			ecx, 10000

_PollLoop:
		in			al, 0x64

		push		ecx
		mov			ecx, 30
_SillyLoop:
		loop		_SillyLoop
		pop			ecx

		test		al, 0x1
		loope		_PollLoop

		pop			ecx
		pop			eax

		ret
	}
}

//================================================
// WaitForInputRegisterReady Function Definition.
//================================================

static VOID WaitForInputRegisterReady( VOID )
{
	__asm
	{
		push		eax
		push		ecx

		mov			ecx, 10000

_PollLoop:
		in			al, 0x64

		push		ecx
		mov			ecx, 30
_SillyLoop:
		loop		_SillyLoop
		pop			ecx

		test		al, 0x2
		loopne		_PollLoop

		pop			ecx
		pop			eax

		ret
	}
}

//================================================
// WaitForKeyboardAcknoledge Function Definition.
//================================================

static VOID __declspec( naked ) WaitForKeyboardAcknoledge( VOID )
{
	__asm
	{
		// === Wait for the ACK code. ===

		// Push.

		push		eax
		push		ecx

		// Wait on bit 1.

		mov			ecx, 0x3000

_AckCodeLoop:

		in			al, 0x64

		push		ecx
		mov			ecx, 30
_SillyLoop:
		loop		_SillyLoop
		pop			ecx

		test		al, 0x1
		loope		_AckCodeLoop

		// Wait for ACK code.

		in			al, 0x60

		push		ecx
		mov			ecx, 30
_SillyLoop2:
		loop		_SillyLoop2
		pop			ecx

		cmp			al, 0xFA
		jne			_AckCodeLoop

		// Pop.

		pop			ecx
		pop			eax

		// === Return to the Caller. ===

		ret
	}
}

//========================================
// SimulateKeyStroke Function Definition.
//========================================

VOID __declspec( naked ) SimulateKeyStrokeImpl( VOID /* "IN BYTE bKeyScanCode" in DH Register. */ )
{
	__asm
	{
		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Write Keyboard Output Register.

		mov			al, 0xD2
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop5:
		loop		_SillyLoop5

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Write the Scan Code.

		mov			al, dh
		out			0x60, al

		// return.

		ret
	}
}

VOID __declspec( naked ) SimulateKeyStroke( VOID /* "IN BYTE bKeyScanCode" in DH Register. */ )
{
	if ( VmWareVersion || _8042MinimalProgramming ) __asm ret;

	__asm
	{

#ifdef SET_OFFON_KEYBINTERRUPT_WHEN_SIMULKEYSTROKE
		// Read 8042 Command Byte.

		mov			al, 0x20
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop1:
		loop		_SillyLoop1

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Read the Command Byte.

		in			al, 0x60
		mov			dl, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop2:
		loop		_SillyLoop2

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Write 8042 Command Byte.

		mov			al, 0x60
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop3:
		loop		_SillyLoop3

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Reset bit 0 of the Command Byte ("Output  Buffer Full").

		mov			al, dl
		and			al, 0xFE
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop4:
		loop		_SillyLoop4
#endif

		call		SimulateKeyStrokeImpl

		// Silly Loop.

		mov			ecx, 30
_SillyLoop6:
		loop		_SillyLoop6

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

#ifdef SET_OFFON_KEYBINTERRUPT_WHEN_SIMULKEYSTROKE
		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Write 8042 Command Byte.

		mov			al, 0x60
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop7:
		loop		_SillyLoop7

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Set bit 0 of the Command Byte ("Output  Buffer Full").

		mov			al, dl
		or			al, 0x1
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop8:
		loop		_SillyLoop8

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady
#endif

		// Return to the Caller.

		ret
	}
}

//============================================
// SetKeyboardLEDsStatus Function Definition.
//============================================

VOID __declspec( naked ) SetKeyboardLEDsStatus( VOID /* "IN BYTE bLEDsStatus" in DH Register. */ )
{
	if ( VmWareVersion || _8042MinimalProgramming ) __asm ret;

	__asm
	{
		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Disable Keyboard.

		mov			al, 0xAD
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop3:
		loop		_SillyLoop3

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Inform that we want to set the LEDs Status.

		mov			al, 0xED
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop1:
		loop		_SillyLoop1

		// Wait for Acknoledge.

		call		WaitForKeyboardAcknoledge

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Set the LEDs Status.

		mov			al, dh
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop2:
		loop		_SillyLoop2

		// Wait for Acknoledge.

		call		WaitForKeyboardAcknoledge

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Enable Keyboard.

		mov			al, 0xAE
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop4:
		loop		_SillyLoop4

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Return to the Caller.

		ret
	}
}

//=============================================
// ConvertScancodeToAscii Function Definition.
//=============================================

static WORD			g_vwScancodeToAsciiMap[] = // === NumLock NOT pressed. ===
{
	( MACRO_SCANCODE_ESC     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_1       << 8 ) | '1',
	( MACRO_SCANCODE_2       << 8 ) | '2',
	( MACRO_SCANCODE_3       << 8 ) | '3',
	( MACRO_SCANCODE_4       << 8 ) | '4',
	( MACRO_SCANCODE_5       << 8 ) | '5',
	( MACRO_SCANCODE_6       << 8 ) | '6',
	( MACRO_SCANCODE_7       << 8 ) | '7',
	( MACRO_SCANCODE_8       << 8 ) | '8',
	( MACRO_SCANCODE_9       << 8 ) | '9',
	( MACRO_SCANCODE_0       << 8 ) | '0',
	( MACRO_SCANCODE_Minus   << 8 ) | '-',
	( MACRO_SCANCODE_Equal   << 8 ) | '=',
	( MACRO_SCANCODE_BackSp  << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Tab     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Q       << 8 ) | 'q',
	( MACRO_SCANCODE_W       << 8 ) | 'w',
	( MACRO_SCANCODE_E       << 8 ) | 'e',
	( MACRO_SCANCODE_R       << 8 ) | 'r',
	( MACRO_SCANCODE_T       << 8 ) | 't',
	( MACRO_SCANCODE_Y       << 8 ) | 'y',
	( MACRO_SCANCODE_U       << 8 ) | 'u',
	( MACRO_SCANCODE_I       << 8 ) | 'i',
	( MACRO_SCANCODE_O       << 8 ) | 'o',
	( MACRO_SCANCODE_P       << 8 ) | 'p',
	( MACRO_SCANCODE_OpBrac  << 8 ) | '[',
	( MACRO_SCANCODE_ClBrac  << 8 ) | ']',
	( MACRO_SCANCODE_ENTER   << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_CTRL    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_A       << 8 ) | 'a',
	( MACRO_SCANCODE_S       << 8 ) | 's',
	( MACRO_SCANCODE_D       << 8 ) | 'd',
	( MACRO_SCANCODE_F       << 8 ) | 'f',
	( MACRO_SCANCODE_G       << 8 ) | 'g',
	( MACRO_SCANCODE_H       << 8 ) | 'h',
	( MACRO_SCANCODE_J       << 8 ) | 'j',
	( MACRO_SCANCODE_K       << 8 ) | 'k',
	( MACRO_SCANCODE_L       << 8 ) | 'l',
	( MACRO_SCANCODE_SemiCl  << 8 ) | ';',
	( MACRO_SCANCODE_Acc0    << 8 ) | '\'',
	( MACRO_SCANCODE_Acc1    << 8 ) | '`',
	( MACRO_SCANCODE_LShift  << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_BSlash  << 8 ) | '\\',
	( MACRO_SCANCODE_Z       << 8 ) | 'z',
	( MACRO_SCANCODE_X       << 8 ) | 'x',
	( MACRO_SCANCODE_C       << 8 ) | 'c',
	( MACRO_SCANCODE_V       << 8 ) | 'v',
	( MACRO_SCANCODE_B       << 8 ) | 'b',
	( MACRO_SCANCODE_N       << 8 ) | 'n',
	( MACRO_SCANCODE_M       << 8 ) | 'm',
	( MACRO_SCANCODE_Comma   << 8 ) | ',',
	( MACRO_SCANCODE_Dot     << 8 ) | '.',
	( MACRO_SCANCODE_Slash   << 8 ) | '/',
	( MACRO_SCANCODE_RShift  << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_AstrP   << 8 ) | '*',
	( MACRO_SCANCODE_Alt     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Space   << 8 ) | ' ',
	( MACRO_SCANCODE_Caps    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F1      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F2      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F3      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F4      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F5      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F6      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F7      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F8      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F9      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_F10     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Num     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Scroll  << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Home    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Up      << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_PgUp    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_MinusP  << 8 ) | '-',
	( MACRO_SCANCODE_Left    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Center  << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Right   << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_PlusP   << 8 ) | '+',
	( MACRO_SCANCODE_End     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Down    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_PgDn    << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Ins     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,
	( MACRO_SCANCODE_Del     << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE,

	( ( MACRO_SCANCODE_EX_ALTGR     | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_RCtrl     | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_INS       | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_DELETE    | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_LEFT      | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_HOME      | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_END       | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_UP        | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_DOWN      | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_PGUP      | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_PGDN      | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_RIGHT     | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,
	( ( MACRO_SCANCODE_EX_SlashP    | 0x80 ) << 8 ) | '/',
	( ( MACRO_SCANCODE_EX_ENTERP    | 0x80 ) << 8 ) | MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX,

	0x0000 // END
};

static WORD			g_vwScancodeToAsciiMap_NumLockPressed[] =
{
	( MACRO_SCANCODE_Home    << 8 ) | '7',
	( MACRO_SCANCODE_Up      << 8 ) | '8',
	( MACRO_SCANCODE_PgUp    << 8 ) | '9',
	( MACRO_SCANCODE_Left    << 8 ) | '4',
	( MACRO_SCANCODE_Center  << 8 ) | '5',
	( MACRO_SCANCODE_Right   << 8 ) | '6',
	( MACRO_SCANCODE_End     << 8 ) | '1',
	( MACRO_SCANCODE_Down    << 8 ) | '2',
	( MACRO_SCANCODE_PgDn    << 8 ) | '3',
	( MACRO_SCANCODE_Ins     << 8 ) | '0',
	( MACRO_SCANCODE_Del     << 8 ) | '.',

	0x0000 // END
};

static WORD			g_vwScancodeToAsciiMap_ShiftPressed[] =
{
	( MACRO_SCANCODE_1       << 8 ) | '!',
	( MACRO_SCANCODE_2       << 8 ) | '@',
	( MACRO_SCANCODE_3       << 8 ) | '#',
	( MACRO_SCANCODE_4       << 8 ) | '$',
	( MACRO_SCANCODE_5       << 8 ) | '%',
	( MACRO_SCANCODE_6       << 8 ) | '^',
	( MACRO_SCANCODE_7       << 8 ) | '&',
	( MACRO_SCANCODE_8       << 8 ) | '*',
	( MACRO_SCANCODE_9       << 8 ) | '(',
	( MACRO_SCANCODE_0       << 8 ) | ')',
	( MACRO_SCANCODE_Minus   << 8 ) | '_',
	( MACRO_SCANCODE_Equal   << 8 ) | '+',
	( MACRO_SCANCODE_Q       << 8 ) | 'Q',
	( MACRO_SCANCODE_W       << 8 ) | 'W',
	( MACRO_SCANCODE_E       << 8 ) | 'E',
	( MACRO_SCANCODE_R       << 8 ) | 'R',
	( MACRO_SCANCODE_T       << 8 ) | 'T',
	( MACRO_SCANCODE_Y       << 8 ) | 'Y',
	( MACRO_SCANCODE_U       << 8 ) | 'U',
	( MACRO_SCANCODE_I       << 8 ) | 'I',
	( MACRO_SCANCODE_O       << 8 ) | 'O',
	( MACRO_SCANCODE_P       << 8 ) | 'P',
	( MACRO_SCANCODE_OpBrac  << 8 ) | '{',
	( MACRO_SCANCODE_ClBrac  << 8 ) | '}',
	( MACRO_SCANCODE_A       << 8 ) | 'A',
	( MACRO_SCANCODE_S       << 8 ) | 'S',
	( MACRO_SCANCODE_D       << 8 ) | 'D',
	( MACRO_SCANCODE_F       << 8 ) | 'F',
	( MACRO_SCANCODE_G       << 8 ) | 'G',
	( MACRO_SCANCODE_H       << 8 ) | 'H',
	( MACRO_SCANCODE_J       << 8 ) | 'J',
	( MACRO_SCANCODE_K       << 8 ) | 'K',
	( MACRO_SCANCODE_L       << 8 ) | 'L',
	( MACRO_SCANCODE_SemiCl  << 8 ) | ':',
	( MACRO_SCANCODE_Acc0    << 8 ) | '"',
	( MACRO_SCANCODE_BSlash  << 8 ) | '|',
	( MACRO_SCANCODE_Z       << 8 ) | 'Z',
	( MACRO_SCANCODE_X       << 8 ) | 'X',
	( MACRO_SCANCODE_C       << 8 ) | 'C',
	( MACRO_SCANCODE_V       << 8 ) | 'V',
	( MACRO_SCANCODE_B       << 8 ) | 'B',
	( MACRO_SCANCODE_N       << 8 ) | 'N',
	( MACRO_SCANCODE_M       << 8 ) | 'M',
	( MACRO_SCANCODE_Comma   << 8 ) | '<',
	( MACRO_SCANCODE_Dot     << 8 ) | '>',
	( MACRO_SCANCODE_Slash   << 8 ) | '?',

	0x0000 // END
};

static VOID __declspec( naked ) ScancodeToAsciiTableLookup( VOID )
{
	__asm
	{
		// Do the Lookup in the Specified Table.

_ConversionLoop:
		mov			ax, [ ebx ]
		or			ax, ax
		jz			_LoopExit

		cmp			ah, dh
		je			_LoopExit

		add			ebx, 2
		jmp			_ConversionLoop

_LoopExit:

		ret
	}
}

VOID __declspec( naked ) ConvertScancodeToAscii( VOID /* "IN BYTE bScancode" in DH Register. "IN BOOLEAN bShiftPressed" in DL Register. "IN BOOLEAN bNumLockPressed" in CH Register. "bCapsLockPressed" in CL Register. "Return Value" in AL Register. */ )
{
	__asm
	{
		// === Do the Conversion. ===

		// Check whether the Num Lock key is pressed or not.

		or			ch, ch
		jz			_NumLockIsNotPressed

		or			dl, dl
		jnz			_NumLockIsNotPressed

		mov			ebx, OFFSET g_vwScancodeToAsciiMap_NumLockPressed
		call		ScancodeToAsciiTableLookup

		or			al, al
		jz			_NumLockIsNotPressed

		jmp			_CheckIfCapsLockIsPressed

_NumLockIsNotPressed:

		// Check whether the Shift key is pressed or not.

		or			dl, dl
		jz			_ShiftIsNotPressed

		mov			ebx, OFFSET g_vwScancodeToAsciiMap_ShiftPressed
		call		ScancodeToAsciiTableLookup

		or			al, al
		jz			_ShiftIsNotPressed

		jmp			_CheckIfCapsLockIsPressed

_ShiftIsNotPressed:

		// Refer the Standard Table.

		mov			ebx, OFFSET g_vwScancodeToAsciiMap
		call		ScancodeToAsciiTableLookup

		// Check whether the Caps Lock is pressed or not:

_CheckIfCapsLockIsPressed:

		or			cl, cl
		jz			_CapsLockIsNotPressed

		cmp			al, 'a'
		jb			_IsNotLowercaseLetter
		cmp			al, 'z'
		ja			_IsNotLowercaseLetter

		sub			al, 'a' - 'A'

		jmp			_CapsLockIsNotPressed

_IsNotLowercaseLetter:

		cmp			al, 'A'
		jb			_CapsLockIsNotPressed
		cmp			al, 'Z'
		ja			_CapsLockIsNotPressed

		add			al, 'a' - 'A'

_CapsLockIsNotPressed:

		// === Return to the Caller. ===

		ret
	}
}

//==============================================
// EnablePs2MouseInterrupt Function Definition.
//==============================================

VOID __declspec( naked ) EnablePs2MouseInterrupt( VOID /* "IN BYTE bEnable" in DH Register. "Return Value (Need Disabling)" in AL Register. */ )
{
	__asm
	{
		// === Enable or Disable the PS/2 Mouse Interrupt. ===

		// Read 8042 Command Byte.

		mov			al, 0x20
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop1:
		loop		_SillyLoop1

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Read the Command Byte.

		in			al, 0x60
		mov			dl, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop2:
		loop		_SillyLoop2

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// If we are Enabling, skip the Command Byte check.

		or			dh, dh
		jnz			_SetCommandByte

		// Check whether we have to exit without Disabling Anything.

		test		dl, 0x2
		jnz			_MouseInterruptIsActive
		test		dl, 0x20
		jz			_MouseStatusBitsIncoherence

		xor			al, al
		jmp			_Exit

_MouseStatusBitsIncoherence:

		xor			al, al
		jmp			_Exit

_MouseInterruptIsActive:

		test		dl, 0x20
		jnz			_MouseStatusBitsIncoherence

_SetCommandByte:

		// Write 8042 Command Byte.

		mov			al, 0x60
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop3:
		loop		_SillyLoop3

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Reset/Set bit 1 and bit 5 of the Command Byte ("Mouse Interrupt Active / Mouse Disabled").

		mov			al, dl

		or			dh, dh
		jnz			_Enabling
		//and			al, 0xFD
		or			al, 0x20
		jmp			_BitMaskOk
_Enabling:
		//or			al, 0x2
		and			al, 0xDF
_BitMaskOk:

		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop4:
		loop		_SillyLoop4

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// === Return to the Caller. ===

		// Return Value = TRUE.

		mov			al, 1

		// Exit from the Function.
_Exit:

		ret
	}
}

//=========================================
// CheckPs2MouseState Function Definition.
//=========================================

VOID __declspec( naked ) CheckPs2MouseState( VOID /* "IN SYSINTERRUPTS_STATUS* psisIntStatus" in EDX Register. */ )
{
	__asm
	{
		// === Check if the Mouse is Present. ===

		// Reset the Variable.

		mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseDetected, FALSE

		// Read 8042 Command Byte.

		mov			al, 0x20
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop1:
		loop		_SillyLoop1

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Read the Command Byte.

		in			al, 0x60

		// Silly Loop.

		mov			ecx, 30
_SillyLoop2:
		loop		_SillyLoop2

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Check if the Mouse is enabled.

		test		al, 0x2
		jz			_ExitFromFunc
		test		al, 0x20
		jnz			_ExitFromFunc

		mov			bl, al // <--- WARNING WARNING WARNING: BL Register, from now on, will hold this value... !

		// === Disable Mouse Interrupt. ===

		// Write 8042 Command Byte.

		mov			al, 0x60
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop15:
		loop		_SillyLoop15

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Disable the Mouse Interrupt.

		mov			al, bl
		and			al, 0xFD
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop16:
		loop		_SillyLoop16

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// === Disable Data Reporting of the Mouse. ===

		// Write to AUX Device.

		mov			al, 0xD4
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop3:
		loop		_SillyLoop3

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Disable Data Rep.

		mov			al, 0xF5
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop4:
		loop		_SillyLoop4

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// === Get the State of the Mouse. ===

		// Write to AUX Device.

		mov			al, 0xD4
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop5:
		loop		_SillyLoop5

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Get Status.

		mov			al, 0xE9
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop6:
		loop		_SillyLoop6

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Wait for Acknoledge.

		call		WaitForKeyboardAcknoledge

		// Silly Loop.

		mov			ecx, 30
_SillyLoop7:
		loop		_SillyLoop7

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Read the First Byte.

_AckCodeLoop1:

		mov			ecx, 30
_SillyLoop19:
		loop		_SillyLoop19

		in			al, 0x60

		cmp			al, 0xFA
		je			_AckCodeLoop1

		mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseStatus1Byte, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop8:
		loop		_SillyLoop8

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Read the Second Byte.

		in			al, 0x60
		mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseStatus2Byte, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop9:
		loop		_SillyLoop9

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Read the Third Byte.

		in			al, 0x60
		mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseStatus3Byte, al

		// === Check if Stream Mode is Enabled. ===

		test		[ edx ]SYSINTERRUPTS_STATUS.bMouseStatus1Byte, 0x40
		jnz			_EnableDataReporting

		// === Get the Mouse Device ID. ===

		// Write to AUX Device.

		mov			al, 0xD4
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop10:
		loop		_SillyLoop10

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Get Device ID.

		mov			al, 0xF2
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop11:
		loop		_SillyLoop11

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Wait for Acknoledge.

		call		WaitForKeyboardAcknoledge

		// Silly Loop.

		mov			ecx, 30
_SillyLoop12:
		loop		_SillyLoop12

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Read the Device ID.

_AckCodeLoop2:

		mov			ecx, 30
_SillyLoop20:
		loop		_SillyLoop20

		in			al, 0x60

		cmp			al, 0xFA
		je			_AckCodeLoop2

		mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseDeviceID, al

		// === Set the "MouseDetected" Variable. ===

		mov			[ edx ]SYSINTERRUPTS_STATUS.bMouseDetected, TRUE

		// === Enable Data Reporting of the Mouse. ===
_EnableDataReporting:

		// Write to AUX Device.

		mov			al, 0xD4
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop13:
		loop		_SillyLoop13

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Enable Data Rep.

		mov			al, 0xF4
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop14:
		loop		_SillyLoop14

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Wait until the 0x60 port is Ready.

		call		WaitForOutputRegisterReady

		// === Enable Mouse Interrupts. ===

		// Write 8042 Command Byte.

		mov			al, 0x60
		out			0x64, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop17:
		loop		_SillyLoop17

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// Enable the Mouse Interrupt.

		mov			al, bl
		or			al, 0x2
		out			0x60, al

		// Silly Loop.

		mov			ecx, 30
_SillyLoop18:
		loop		_SillyLoop18

		// Wait on bit 2 of 0x64.

		call		WaitForInputRegisterReady

		// === Return ===
_ExitFromFunc:

		ret
	}
}
