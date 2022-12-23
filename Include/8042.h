/***************************************************************************************
  *
  * 8042.h - VpcICE "8042 (Keyboard)" Routines Header File.
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
#include "WinDefs.h"

//========================
// Scancodes Definitions.
//========================

#define MACRO_SCANCODE_ESC    0x01		// ESC    
#define MACRO_SCANCODE_1      0x02		// 1      
#define MACRO_SCANCODE_2      0x03		// 2      
#define MACRO_SCANCODE_3      0x04		// 3      
#define MACRO_SCANCODE_4      0x05		// 4      
#define MACRO_SCANCODE_5      0x06		// 5      
#define MACRO_SCANCODE_6      0x07		// 6      
#define MACRO_SCANCODE_7      0x08		// 7      
#define MACRO_SCANCODE_8      0x09		// 8      
#define MACRO_SCANCODE_9      0x0A		// 9      
#define MACRO_SCANCODE_0      0x0B		// 0      
#define MACRO_SCANCODE_Minus  0x0C		// -
#define MACRO_SCANCODE_Equal  0x0D		// =
#define MACRO_SCANCODE_BackSp 0x0E		// Back Space
#define MACRO_SCANCODE_Tab    0x0F		// Tab    
#define MACRO_SCANCODE_Q      0x10		// Q      
#define MACRO_SCANCODE_W      0x11		// W      
#define MACRO_SCANCODE_E      0x12		// E      
#define MACRO_SCANCODE_R      0x13		// R      
#define MACRO_SCANCODE_T      0x14		// T      
#define MACRO_SCANCODE_Y      0x15		// Y      
#define MACRO_SCANCODE_U      0x16		// U      
#define MACRO_SCANCODE_I      0x17		// I      
#define MACRO_SCANCODE_O      0x18		// O      
#define MACRO_SCANCODE_P      0x19		// P      
#define MACRO_SCANCODE_OpBrac 0x1A		// [
#define MACRO_SCANCODE_ClBrac 0x1B		// ]
#define MACRO_SCANCODE_ENTER  0x1C		// ENTER
#define MACRO_SCANCODE_CTRL   0x1D		// CTRL   
#define MACRO_SCANCODE_A      0x1E		// A      
#define MACRO_SCANCODE_S      0x1F		// S      
#define MACRO_SCANCODE_D      0x20		// D      
#define MACRO_SCANCODE_F      0x21		// F      
#define MACRO_SCANCODE_G      0x22		// G      
#define MACRO_SCANCODE_H      0x23		// H      
#define MACRO_SCANCODE_J      0x24		// J      
#define MACRO_SCANCODE_K      0x25		// K      
#define MACRO_SCANCODE_L      0x26		// L      
#define MACRO_SCANCODE_SemiCl 0x27		// ;
#define MACRO_SCANCODE_Acc0   0x28		// '
#define MACRO_SCANCODE_Acc1   0x29		// `
#define MACRO_SCANCODE_LShift 0x2A		// Left Shift
#define MACRO_SCANCODE_BSlash 0x2B		// \ 
#define MACRO_SCANCODE_Z      0x2C		// Z      
#define MACRO_SCANCODE_X      0x2D		// X      
#define MACRO_SCANCODE_C      0x2E		// C      
#define MACRO_SCANCODE_V      0x2F		// V      
#define MACRO_SCANCODE_B      0x30		// B      
#define MACRO_SCANCODE_N      0x31		// N      
#define MACRO_SCANCODE_M      0x32		// M      
#define MACRO_SCANCODE_Comma  0x33		// ,
#define MACRO_SCANCODE_Dot    0x34		// .
#define MACRO_SCANCODE_Slash  0x35		// /
#define MACRO_SCANCODE_RShift 0x36		// Right Shift
#define MACRO_SCANCODE_AstrP  0x37		// * Pad
#define MACRO_SCANCODE_Alt    0x38		// Alt    
#define MACRO_SCANCODE_Space  0x39		// Space  
#define MACRO_SCANCODE_Caps   0x3A		// Caps   
#define MACRO_SCANCODE_F1     0x3B		// F1     
#define MACRO_SCANCODE_F2     0x3C		// F2     
#define MACRO_SCANCODE_F3     0x3D		// F3     
#define MACRO_SCANCODE_F4     0x3E		// F4     
#define MACRO_SCANCODE_F5     0x3F		// F5     
#define MACRO_SCANCODE_F6     0x40		// F6
#define MACRO_SCANCODE_F7     0x41		// F7
#define MACRO_SCANCODE_F8     0x42		// F8
#define MACRO_SCANCODE_F9     0x43		// F9
#define MACRO_SCANCODE_F10    0x44		// F10
#define MACRO_SCANCODE_Num    0x45		// Num
#define MACRO_SCANCODE_Scroll 0x46		// Scroll
#define MACRO_SCANCODE_Home   0x47		// Home
#define MACRO_SCANCODE_Up     0x48		// Up
#define MACRO_SCANCODE_PgUp   0x49		// PgUp
#define MACRO_SCANCODE_MinusP 0x4A		// - Pad
#define MACRO_SCANCODE_Left   0x4B		// Left
#define MACRO_SCANCODE_Center 0x4C		// Center
#define MACRO_SCANCODE_Right  0x4D		// Right
#define MACRO_SCANCODE_PlusP  0x4E		// + Pad
#define MACRO_SCANCODE_End    0x4F		// End
#define MACRO_SCANCODE_Down   0x50		// Down
#define MACRO_SCANCODE_PgDn   0x51		// PgDn
#define MACRO_SCANCODE_Ins    0x52		// Ins
#define MACRO_SCANCODE_Del    0x53		// Del

#define MACRO_SCANCODE_EX_ALTGR    0x38		// Alt GR
#define MACRO_SCANCODE_EX_RCtrl    0x1D		// Right Ctrl
#define MACRO_SCANCODE_EX_INS      0x52		// Insert
#define MACRO_SCANCODE_EX_DELETE   0x53		// Delete
#define MACRO_SCANCODE_EX_LEFT     0x4B		// Left
#define MACRO_SCANCODE_EX_HOME     0x47		// Home
#define MACRO_SCANCODE_EX_END      0x4F		// End
#define MACRO_SCANCODE_EX_UP       0x48		// Up
#define MACRO_SCANCODE_EX_DOWN     0x50		// Down
#define MACRO_SCANCODE_EX_PGUP     0x49		// Page Up
#define MACRO_SCANCODE_EX_PGDN     0x51		// Page Down
#define MACRO_SCANCODE_EX_RIGHT    0x4D		// Right
#define MACRO_SCANCODE_EX_SlashP   0x35		// Slash Pad
#define MACRO_SCANCODE_EX_ENTERP   0x1C		// Enter Pad

#define MACRO_ASCIICONVERSION_RETVAL_NOT_INTERESTING		0x00
#define MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE			0xFF
#define MACRO_ASCIICONVERSION_RETVAL_CHECK_SCANCODE_EX		0xFE

//======================
// Function Prototypes.
//======================

VOID SimulateKeyStroke( VOID /* "IN BYTE bKeyScanCode" in DH Register. */ );
VOID SetKeyboardLEDsStatus( VOID /* "IN BYTE bLEDsStatus" in DH Register. */ );
VOID ConvertScancodeToAscii( VOID /* "IN BYTE bScancode" in DH Register. "IN BOOLEAN bShiftPressed" in DL Register. "IN BOOLEAN bNumLockPressed" in CH Register. "bCapsLockPressed" in CL Register. "Return Value" in AL Register. */ );
VOID EnablePs2MouseInterrupt( VOID /* "IN BYTE bEnable" in DH Register. "Return Value (Need Disabling)" in AL Register. */ );
VOID CheckPs2MouseState( VOID /* "IN SYSINTERRUPTS_STATUS* psisIntStatus" in EDX Register. */ );

VOID SimulateKeyStrokeImpl( VOID /* "IN BYTE bKeyScanCode" in DH Register. */ );
