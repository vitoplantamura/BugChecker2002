/***************************************************************************************
  *
  * apic.c - VpcICE "i8259A PIC / i82489 APIC" Routines Source File.
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

#pragma warning(disable : 4700)

//========================
// Required Header Files.
//========================

#define INITU2KSTUB		// -> Make the "g_u2kstub" Structure to Initialize ...

#include <ntddk.h>
#include <stdio.h>
#include <stdarg.h>
#include "..\Include\apic.h"
#include "..\Include\8042.h"
#include "..\Include\vpcice.h"
#include "..\Include\crt.h"

//==========
// Pragmas.
//==========

#pragma warning( disable : 4102 )

//==========
// Externs.
//==========

extern DWORD VmWareVersion;
extern BOOLEAN _8042MinimalProgramming;

//================================
// Debug Registers Global Status.
//================================

DWORD			g_vdwDR[ 4 ] = { 0, 0, 0, 0 };
DWORD			g_dwDR7 = MACRO_DEFAULT_DR7_MASK;

//=================================
// Debugger Enter/Exit TSC Values.
//=================================

DWORD			g_dwEnterTscHigh = 0,
				g_dwEnterTscLow = 0,
				g_dwExitTscHigh = 0,
				g_dwExitTscLow = 0;

//===========================================
// Global Data Related to the Keyboard Hook.
//===========================================

static PVOID		g_pfnMouseISR = NULL;
DETOUR_TRAMPOLINE_GLOBVAR( void Trampoline_MouseISR( void ), g_pfnMouseISR )

static PVOID		g_pfnKeyboardISR = NULL;
DETOUR_TRAMPOLINE_GLOBVAR( void Trampoline_KeyboardISR( void ), g_pfnKeyboardISR )

SYSINTERRUPTS_STATUS*		g_psisSysInterrStatus;

WORD				g_wVpcICEDataSegment = 0;
static WORD			g_wVpcICEExtraSegment = 0;

static WORD			g_wFSSEG_in_Kernel = 0;

//=================================
// Interrupt Hooks Related Macros.
//=================================

#define MACRO_INTHOOK_PUSH_REGISTERS /* ###WARNING###: Also used in normal Detoured Functions... */ \
		__asm push		eax		\
		__asm push		ebx		\
		__asm push		ecx		\
		__asm push		edx		\
		__asm push		esi		\
		__asm push		edi		\
		__asm push		ebp		\
		__asm pushf				\
		__asm push		ds		\
		__asm push		es

#define MACRO_INTHOOK_POP_REGISTERS_EPILOGUE(n) \
		__asm mov		edx, g_psisSysInterrStatus										\
		__asm cmp		[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE			\
		__asm je		_NoSingleStep##n												\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EFLAGS ]		\
		__asm or		eax, (1<<8) /* Trap Flag. */									\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EFLAGS ], eax		\
		__asm jmp		_SingleStepBitWasSet##n											\
__asm _NoSingleStep##n:																	\
		__asm cmp		[ edx ]SYSINTERRUPTS_STATUS.bKeyStrokeSimulationPending, FALSE	\
		__asm je		_SingleStepBitWasSet##n											\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.bKeyStrokeSimulationPending, FALSE	\
		__asm push		edx																\
		__asm mov		dh, 0x80 | 0x1D													\
		__asm call		SimulateKeyStroke												\
		__asm pop		edx																\
__asm _SingleStepBitWasSet##n:															\
		__asm pop		es																\
		__asm pop		ds																\
		__asm popf																		\
		__asm pop		ebp																\
		__asm pop		edi																\
		__asm pop		esi																\
		__asm pop		edx																\
		__asm pop		ecx																\
		__asm pop		ebx																\
		__asm pop		eax

#define MACRO_INTHOOK_POP_REGISTERS_PLAIN /* ###WARNING###: Also used in normal Detoured Functions... */ \
		__asm pop		es																\
		__asm pop		ds																\
		__asm popf																		\
		__asm pop		ebp																\
		__asm pop		edi																\
		__asm pop		esi																\
		__asm pop		edx																\
		__asm pop		ecx																\
		__asm pop		ebx																\
		__asm pop		eax

#define MACRO_INTHOOK_SETUP_ENVIRONMENT /* ###WARNING###: Also used in normal Detoured Functions... */ \
		__asm mov		ax, cs:g_wVpcICEDataSegment		\
		__asm mov		ds, ax							\
		__asm mov		ax, g_wVpcICEExtraSegment		\
		__asm mov		es, ax							\
														\
		__asm cld										\
														\
		__asm mov		edx, g_psisSysInterrStatus

#define MACRO_INTHOOK_GET_REGS_STATE \
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EAX ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EAX, eax						\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EBX ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EBX, eax						\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ECX ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ECX, eax						\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EDX ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EDX, eax						\
																							\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ESI ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESI, eax						\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EDI ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EDI, eax						\
																							\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EBP ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EBP, eax						\
																							\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EFLAGS ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EFLAGS, eax						\
																							\
		/* --- */																			\
																							\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EIP ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EIP, eax						\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_CS ]				\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.CS, ax							\
		__asm test		eax, 0x3															\
		__asm jnz		_FromUserSpace														\
																							\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.bWasStackSwitching, FALSE				\
																							\
		__asm lea		eax, [ esp + MACRO_PROCCONTEXT_STACK_DISP_EFLAGS + 4 ]				\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESP, eax						\
																							\
		__asm mov		ax, ss																\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.SS, ax							\
																							\
		__asm jmp		_SsEspWereSaved														\
																							\
__asm _FromUserSpace:																		\
																							\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.bWasStackSwitching, TRUE				\
																							\
		__asm mov		eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ESP_LESS ]		\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESP, eax						\
																							\
		__asm mov		ax, word ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_SS_LESS ]			\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.SS, ax							\
																							\
__asm _SsEspWereSaved:																		\
																							\
		/* --- */																			\
																							\
		__asm mov		ax, word ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_DS ]				\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.DS, ax							\
		__asm mov		ax, word ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ES ]				\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ES, ax							\
																							\
		__asm mov		ax, fs																\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.FS, ax							\
		__asm mov		ax, gs																\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.GS, ax

#define MACRO_INTHOOK_SET_REGS_STATE(n) \
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EAX						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EAX ], eax			\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EBX						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EBX ], eax			\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ECX						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ECX ], eax			\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EDX						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EDX ], eax			\
																							\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESI						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ESI ], eax			\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EDI						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EDI ], eax			\
																							\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EBP						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EBP ], eax			\
																							\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EFLAGS						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EFLAGS ], eax			\
																							\
		/* --- */																			\
																							\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EIP						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EIP ], eax			\
																							\
		__asm cmp		[ edx ]SYSINTERRUPTS_STATUS.bWasStackSwitching, FALSE				\
		__asm jne		_FromUserSpace##n													\
																							\
		__asm mov		ax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.SS							\
		__asm mov		ss, ax																\
																							\
		__asm jmp		_SsEspWereRestored##n												\
																							\
__asm _FromUserSpace##n:																	\
																							\
		__asm mov		eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ESP						\
		__asm mov		dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ESP_LESS ], eax		\
																							\
		__asm mov		ax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.SS							\
		__asm mov		word ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_SS_LESS ], ax			\
																							\
__asm _SsEspWereRestored##n:																\
																							\
		/* --- */																			\
																							\
		__asm mov		ax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.DS							\
		__asm mov		word ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_DS ], ax				\
		__asm mov		ax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.ES							\
		__asm mov		word ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_ES ], ax				\
																							\
		__asm mov		ax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.FS							\
		__asm mov		fs, ax																\
		__asm mov		ax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.GS							\
		__asm mov		gs, ax

//=========================================
// Hooked_KeyboardISR Function Definition.
//=========================================

BYTE g_bLastScanCode = 0;

static void VmWareKeyboardIntDeReentrancyThreadProc(PVOID pv /* not used */) // PASSIVE_LEVEL
{
	LARGE_INTEGER		delayTime = RtlConvertLongToLargeInteger( - 8000 ); // <----- if this wait is not long enough, the vm will dead lock on the keyb IRQL !!!
	BYTE				bPrevScanCode;

	while( 1 )
	{
		__asm
		{
			push			eax
			push			ebx
			push			ecx
			push			edx

			// check
			mov				dh, g_bLastScanCode
			cmp				dh, 0
			je				_Skip

			// simulate
			cli
			mov				al, 0xD2
			out				0x64, al
			mov				al, dh
			out				0x60, al
			sti

_Skip:
			pop				edx
			pop				ecx
			pop				ebx
			pop				eax
		}

		bPrevScanCode = g_bLastScanCode;
		if ( g_bLastScanCode ) KeDelayExecutionThread( KernelMode, FALSE, & delayTime );

		__asm
		{
			mov				g_bLastScanCode, 0
		}

		if ( ! bPrevScanCode ) KeDelayExecutionThread( KernelMode, FALSE, & delayTime );
	}
}

static void __declspec( naked ) Hooked_KeyboardISR( void )
{
	__asm
	{
		// ### Push the State. ###

		push		0xFFFFFFFF	// Used for ISR Exit Method with "RET" Instruction.

		//-------------------------------------------------------- ISR's Pushs ---
#define MACRO_SIZEOF_PUSHED_VARS		0x26

		MACRO_INTHOOK_PUSH_REGISTERS
		//------------------------------------------------------------------------
#define MACRO_PROCCONTEXT_STACK_DISP_EAX			0x22
#define MACRO_PROCCONTEXT_STACK_DISP_EBX			0x1E
#define MACRO_PROCCONTEXT_STACK_DISP_ECX			0x1A
#define MACRO_PROCCONTEXT_STACK_DISP_EDX			0x16
#define MACRO_PROCCONTEXT_STACK_DISP_ESI			0x12
#define MACRO_PROCCONTEXT_STACK_DISP_EDI			0x0E
#define MACRO_PROCCONTEXT_STACK_DISP_EBP			0x0A
#define MACRO_PROCCONTEXT_STACK_DISP_DS				0x04
#define MACRO_PROCCONTEXT_STACK_DISP_ES				0x00

#define MACRO_PROCCONTEXT_STACK_DISP_SS_LESS		( MACRO_SIZEOF_PUSHED_VARS + 4 + 0x10 )
#define MACRO_PROCCONTEXT_STACK_DISP_ESP_LESS		( MACRO_SIZEOF_PUSHED_VARS + 4 + 0x0C )
#define MACRO_PROCCONTEXT_STACK_DISP_EFLAGS			( MACRO_SIZEOF_PUSHED_VARS + 4 + 0x08 )
#define MACRO_PROCCONTEXT_STACK_DISP_CS				( MACRO_SIZEOF_PUSHED_VARS + 4 + 0x04 )
#define MACRO_PROCCONTEXT_STACK_DISP_EIP			( MACRO_SIZEOF_PUSHED_VARS + 4 + 0x00 )
		//------------------------------------------------------------------------

		MACRO_INTHOOK_SETUP_ENVIRONMENT

		inc			[ edx ]SYSINTERRUPTS_STATUS.ulMouseKeybControllerIntrCount

		// ### Check whether we are in the Debugger. ###

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bInDebugger, 0
		je			_NotInDebugger

		// Read the Scancode.

		mov			g_bLastScanCode, 0

		in			al, 0x60

		// Interpret the Scancode.

		cmp			al, 0xE0
		jne			_Debugger_E0_NotReceived
		mov			[ edx ]SYSINTERRUPTS_STATUS.bE0Recv, 1
		jmp			_SignalEoi
_Debugger_E0_NotReceived:

		cmp			al, 0xE1
		jne			_Debugger_E1_NotReceived
		mov			[ edx ]SYSINTERRUPTS_STATUS.bE1Recv, 1
		jmp			_SignalEoi
_Debugger_E1_NotReceived:

		mov			bl, [ edx ]SYSINTERRUPTS_STATUS.bE0Recv
		mov			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, bl
		mov			bl, [ edx ]SYSINTERRUPTS_STATUS.bE1Recv
		mov			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E1Recv, bl
		mov			[ edx ]SYSINTERRUPTS_STATUS.bE0Recv, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bE1Recv, 0

		cmp			al, 0x2A | 0x80
		jne			_LeftShiftNotReleased
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		mov			[ edx ]SYSINTERRUPTS_STATUS.bShiftPressed, 0
		jmp			_SignalEoi
_LeftShiftNotReleased:

		cmp			al, 0x36 | 0x80
		jne			_RightShiftNotReleased
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		mov			[ edx ]SYSINTERRUPTS_STATUS.bShiftPressed, 0
		jmp			_SignalEoi
_RightShiftNotReleased:

		cmp			al, 0x1D | 0x80
		jne			_ControlNotReleased
		mov			[ edx ]SYSINTERRUPTS_STATUS.bControlPressed, 0
		jmp			_SignalEoi
_ControlNotReleased:

		cmp			al, 0x38 | 0x80
		jne			_AltNotReleased
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		mov			[ edx ]SYSINTERRUPTS_STATUS.bAltPressed, 0
		jmp			_SignalEoi
_AltNotReleased:

		cmp			al, 0x3A | 0x80
		jne			_CapsLockNotDepressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		and			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0xFF ^ 0x4
		jmp			_SignalEoi
_CapsLockNotDepressed:

		cmp			al, 0x45 | 0x80
		jne			_NumLockNotDepressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		and			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0xFF ^ 0x2
		jmp			_SignalEoi
_NumLockNotDepressed:

		cmp			al, 0x46 | 0x80
		jne			_ScrollLockNotDepressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		and			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0xFF ^ 0x1
		jmp			_SignalEoi
_ScrollLockNotDepressed:

		cmp			al, 0x80
		jae			_SignalEoi

		cmp			al, 0x2A
		jne			_LeftShiftNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		mov			[ edx ]SYSINTERRUPTS_STATUS.bShiftPressed, 1
		jmp			_SignalEoi
_LeftShiftNotPressed:

		cmp			al, 0x36
		jne			_RightShiftNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		mov			[ edx ]SYSINTERRUPTS_STATUS.bShiftPressed, 1
		jmp			_SignalEoi
_RightShiftNotPressed:

		cmp			al, 0x1D
		jne			_ControlNotPressed
		mov			[ edx ]SYSINTERRUPTS_STATUS.bControlPressed, 1
		jmp			_SignalEoi
_ControlNotPressed:

		cmp			al, 0x38
		jne			_AltNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		mov			[ edx ]SYSINTERRUPTS_STATUS.bAltPressed, 1
		jmp			_SignalEoi
_AltNotPressed:

		cmp			al, 0x3A
		jne			_CapsLockNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		test		[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0x4
		jnz			_SignalEoi
		or			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0x4
		xor			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus, 0x4
		push		edx
		mov			dh, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus
		call		SetKeyboardLEDsStatus
		pop			edx
		jmp			_SignalEoi
_CapsLockNotPressed:

		cmp			al, 0x45
		jne			_NumLockNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		test		[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0x2
		jnz			_SignalEoi
		or			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0x2
		xor			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus, 0x2
		push		edx
		mov			dh, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus
		call		SetKeyboardLEDsStatus
		pop			edx
		jmp			_SignalEoi
_NumLockNotPressed:

		cmp			al, 0x46
		jne			_ScrollLockNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		jne			_SignalEoi
		test		[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0x1
		jnz			_SignalEoi
		or			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0x1
		xor			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus, 0x1
		push		edx
		mov			dh, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus
		call		SetKeyboardLEDsStatus
		pop			edx
		jmp			_SignalEoi
_ScrollLockNotPressed:

		// Convert the Scancode to Ascii and put it in the Queue.

		mov			[ edx ]SYSINTERRUPTS_STATUS.bLastScanCodeAcquired, al

		mov			ah, [ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv

		push		edx

		mov			cl, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus
		shr			cl, 2
		and			cl, 1

		mov			ch, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsStatus
		shr			ch, 1
		and			ch, 1

		mov			dl, [ edx ]SYSINTERRUPTS_STATUS.bShiftPressed
		mov			dh, al

		or			ah, ah
		jz			_E0WasNOTReceived
		or			dh, 0x80
_E0WasNOTReceived:

		call		ConvertScancodeToAscii

		pop			edx

		mov			[ edx ]SYSINTERRUPTS_STATUS.bLastAsciiCodeAcquired, al

		// Signal the EOI.

_SignalEoi:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr, 0
		jne			_LocalApicIsPresent

		call		SendEOIToPIC1

		jmp			_EoiSignaled

_LocalApicIsPresent:

		push		edx
		call		SendEOIToLocalAPIC

_EoiSignaled:

		// Resume the Execution.

		jmp			_ExitFromISRWithIRETD_Plain

_NotInDebugger:

		// ### Updates our Status Vector. ###

		cmp			g_bLastScanCode, 0
		jne			_VmwareSkipOurVector

		sub			eax, eax
		in			al, 0x60
		mov			bl, 1

		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, al

		cmp			al, 0x80
		jb			_KeyBeingPressed

		sub			al, 0x80
		mov			bl, 0

_KeyBeingPressed:

		mov			[ edx + eax ]SYSINTERRUPTS_STATUS.vbKeybStatus, bl

		// ### Check if the Scancode was a Special Code. ###

		mov			al, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode

		cmp			al, 0xE0
		jne			_Scancode_E0_NotReceived

		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyb_E0_Received, 1

		jmp			_HotKeyNotPressed

_Scancode_E0_NotReceived:

		cmp			al, 0xE1
		jne			_Scancode_E1_NotReceived

		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyb_E1_Received, 1

		jmp			_HotKeyNotPressed

_Scancode_E1_NotReceived:

		mov			al, [ edx ]SYSINTERRUPTS_STATUS.bKeyb_E0_Received
		mov			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, al

		mov			al, [ edx ]SYSINTERRUPTS_STATUS.bKeyb_E1_Received
		mov			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E1_Received, al

		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyb_E0_Received, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyb_E1_Received, 0

		// ### Check if the Right Ctrl was just pressed. ###

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x1D
		jne			_CtrlWasNotPressed

		mov			[ edx ]SYSINTERRUPTS_STATUS.bPressingRightCtrlKey, 0

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		je			_CtrlWasNotPressed

		mov			[ edx ]SYSINTERRUPTS_STATUS.bPressingRightCtrlKey, 1

_CtrlWasNotPressed:

		// ### Track the State of the Keys associated to the Keyboard LEDs. ###

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x3A | 0x80
		jne			_SysCapsLockNotDepressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		jne			_SysCapsLockNotDepressed
		and			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0xFF ^ 0x4
_SysCapsLockNotDepressed:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x45 | 0x80
		jne			_SysNumLockNotDepressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		jne			_SysNumLockNotDepressed
		and			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0xFF ^ 0x2
_SysNumLockNotDepressed:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x46 | 0x80
		jne			_SysScrollLockNotDepressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		jne			_SysScrollLockNotDepressed
		and			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0xFF ^ 0x1
_SysScrollLockNotDepressed:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x3A
		jne			_SysCapsLockNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		jne			_SysCapsLockNotPressed
		test		[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0x4
		jnz			_SysCapsLockNotPressed
		or			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0x4
		xor			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsStatus, 0x4
_SysCapsLockNotPressed:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x45
		jne			_SysNumLockNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		jne			_SysNumLockNotPressed
		test		[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0x2
		jnz			_SysNumLockNotPressed
		or			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0x2
		xor			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsStatus, 0x2
_SysNumLockNotPressed:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode, 0x46
		jne			_SysScrollLockNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPREV_Keyb_E0_Received, 0
		jne			_SysScrollLockNotPressed
		test		[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0x1
		jnz			_SysScrollLockNotPressed
		or			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsPressed, 0x1
		xor			[ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsStatus, 0x1
_SysScrollLockNotPressed:

		// ### Check if the Hotkey is being Pressed. ###

		// Check for "LEFT_CTRL+D".

		cmp			[ edx + 0x1D ]SYSINTERRUPTS_STATUS.vbKeybStatus, 0 // ### CTRL ###
		je			_HotKeyNotPressed
		cmp			[ edx + 0x20 ]SYSINTERRUPTS_STATUS.vbKeybStatus, 0 // ### D ###
		je			_HotKeyNotPressed
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bPressingRightCtrlKey, 0 // ### ONLY LEFT CTRL ###
		jne			_HotKeyNotPressed

		// Clear the Keys Table.

		lea			edi, [ edx ]SYSINTERRUPTS_STATUS.vbKeybStatus
		mov			ecx, 0x80
		xor			eax, eax
		rep			stosb

		// Entering Debugger...

		ENTERING_DEBUGGER

		// Get the State of the Registers.

		MACRO_INTHOOK_GET_REGS_STATE

		// Invoke the Debugger.

		mov			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, TRUE

		mov			[ edx ]SYSINTERRUPTS_STATUS.bEOIRequired, TRUE

		call		InvokeDebugger

		// Simulate the Release of the Hot Key.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		jne			_DoNotSimulateKeyStroke

		push		edx
		mov			dh, 0x80 | 0x1D

		//////////////////////////////////////////////////////// VmWare virtual 8042 trigger /////// start ///////
		cmp			VmWareVersion, 0
		jne			_VmCase2
		cmp			_8042MinimalProgramming, 0
		jne			_VmCase2
		jmp			_SkipVmCase2
_VmCase2:
		mov			g_bLastScanCode, dh
_SkipVmCase2:
		//////////////////////////////////////////////////////// VmWare virtual 8042 trigger /////// end ///////

		call		SimulateKeyStroke
		pop			edx

		jmp			_KeyStrokeWasSimulated

_DoNotSimulateKeyStroke:

		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyStrokeSimulationPending, TRUE

_KeyStrokeWasSimulated:

		// Reflect the Changes made in the Debugger to the CPU State.

		MACRO_INTHOOK_SET_REGS_STATE(1)

		// Exit.

		jmp			_ExitFromISRWithIRETD_EXITINGDEB

_HotKeyNotPressed:

		// ### Simulate the Keystroke before calling the Previous ISR ###

		push		edx
		mov			dh, [ edx ]SYSINTERRUPTS_STATUS.bKeyboardScanCode

		//////////////////////////////////////////////////////// VmWare virtual 8042 trigger /////// start ///////
		cmp			VmWareVersion, 0
		jne			_VmCase
		cmp			_8042MinimalProgramming, 0
		jne			_VmCase
		jmp			_SkipVmCase
_VmCase:
		mov			g_bLastScanCode, dh
_SkipVmCase:
		//////////////////////////////////////////////////////// VmWare virtual 8042 trigger /////// end ///////

		call		SimulateKeyStroke
		pop			edx

_VmwareSkipOurVector:

		// ### Check whether we have to exit redirecting execution to the REAL Windows ISR ###

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bREAL_KeybIntVec, 0
		je			_JustExit

		// Get the Address of the ISR for this Processor.

		lea		eax, [ edx ]SYSINTERRUPTS_STATUS.xirIDTR
		SIDT	[ eax ]

		movzx	ebx, [ edx ]SYSINTERRUPTS_STATUS.bREAL_KeybIntVec
		mov		eax, [ edx ]SYSINTERRUPTS_STATUS.xirIDTR.dwBase

		mov		ecx, [ eax + ebx * 8 ]
		mov		eax, [ eax + ebx * 8 + 4 ]

		and		eax, 0xFFFF0000
		and		ecx, 0x0000FFFF

		or		ecx, eax

		// Set the Stack for the "RET" Instruction.

		mov		dword ptr[ esp + MACRO_SIZEOF_PUSHED_VARS ], ecx

		// Exit.

		jmp			_ExitFromISRWithRET

_JustExit:

//-----------------------------------------------------------------------------------------------------------------------
//   ===> ISR EXIT <===
//-----------------------------------------------------------------------------------------------------------------------

		// ### Pop the State. ###

		MACRO_INTHOOK_POP_REGISTERS_PLAIN

		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".

		// ### Call the Original Handler. ###

		jmp			Trampoline_KeyboardISR

//-----------------------------------------------------------------------------------------------------------------------

		// ### Pop the State. ###

_ExitFromISRWithIRETD_EXITINGDEB:

		MACRO_INTHOOK_POP_REGISTERS_EPILOGUE(1)

		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".

		EXITING_DEBUGGER(1)

		// ### Return. ###

		iretd

//-----------------------------------------------------------------------------------------------------------------------

_ExitFromISRWithRET:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN

		// ### Return. ###

		ret

//-----------------------------------------------------------------------------------------------------------------------

_ExitFromISRWithIRETD_Plain:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN

		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".
		iretd

//-----------------------------------------------------------------------------------------------------------------------
	}
}

//======================================
// Hooked_MouseISR Function Definition.
//======================================

static void __declspec( naked ) Hooked_MouseISR( void )
{
	__asm
	{
		// ### Push the State. ###

		push		0xFFFFFFFF	// Used for ISR Exit Method with "RET" Instruction.

		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		inc			[ edx ]SYSINTERRUPTS_STATUS.ulMouseKeybControllerIntrCount

		// ### Check whether we are in the Debugger. ###

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bInDebugger, 0
		je			_NotInDebugger

		// Read the Scancode.

		in			al, 0x60

		// Check whether the Mouse is Enabled.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bMouseEnabled, FALSE
		je			_SignalEoi
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bMouseDetected, FALSE
		je			_SignalEoi

		// Store the New Packet Byte.

		//
		// ... entering synchronized area ...
		//

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslMousePacketSpinLock
		call		EnterMultiProcessorSpinLock

			cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketForming, FALSE
			jne			_PacketIsForming

			// Initialize the Variables, checking the First Byte consistency bit.

			test		al, (1<<3) // Always 1 for the First Byte.
			jnz			_FirstByteIsCorrect

			mov			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketReady, FALSE
			mov			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketForming, FALSE

			jmp			_ExitFromSynchronizedArea

_FirstByteIsCorrect:

			mov			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketReady, FALSE
			mov			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketForming, TRUE
			mov			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketCurrByte, 0

			mov			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketUnknownDevice, FALSE

_PacketIsForming:

			// Determine in BH the Packet Size.

			mov			bl, [ edx ]SYSINTERRUPTS_STATUS.bMouseDeviceID

			cmp			bl, 0
			jne			_NotStandardPs2Mouse

			mov			bh, 3
			jmp			_MousePacketSizeDetermined

_NotStandardPs2Mouse:
			cmp			bl, 3
			jne			_NotStandardIntellimouse

			mov			bh, 4
			jmp			_MousePacketSizeDetermined

_NotStandardIntellimouse:
			cmp			bl, 4
			jne			_NotIntellimouseWith5Buttons

			mov			bh, 4
			jmp			_MousePacketSizeDetermined

_NotIntellimouseWith5Buttons:
			mov			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketUnknownDevice, TRUE
			jmp			_ExitFromSynchronizedArea

_MousePacketSizeDetermined:

			// Check the Byte Index in the Vector.

			cmp			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketCurrByte, bh
			jae			_ExitFromSynchronizedArea

			// Store the Packet Byte.

			push		eax
			movzx		eax, byte ptr [ edx ]SYSINTERRUPTS_STATUS.bMousePacketCurrByte
			lea			ecx, [ edx + eax ]SYSINTERRUPTS_STATUS.bMousePacket
			pop			eax

			mov			byte ptr[ ecx ], al

			// Index Increment + Check whether the Packet is Complete.

			inc			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketCurrByte

			cmp			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketCurrByte, bh
			jb			_ExitFromSynchronizedArea

			// The Packet is Complete.

			mov			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketReady, TRUE
			mov			[ edx ]SYSINTERRUPTS_STATUS.bIsMousePacketForming, FALSE
			mov			[ edx ]SYSINTERRUPTS_STATUS.bMousePacketCurrByte, 0
			movzx		ebx, bh
			mov			[ edx ]SYSINTERRUPTS_STATUS.ulReadyPacketSize, ebx

_ExitFromSynchronizedArea:

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslMousePacketSpinLock
		call		LeaveMultiProcessorSpinLock

		//
		// ... leaving synchronized area ...
		//

		// Signal the EOI.

_SignalEoi:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr, 0
		jne			_LocalApicIsPresent

		call		SendEOIToPIC2
		call		SendEOIToPIC1

		jmp			_EoiSignaled

_LocalApicIsPresent:

		push		edx
		call		SendEOIToLocalAPIC

_EoiSignaled:

		// Resume the Execution.

		jmp			_ExitFromISRWithIRETD_Plain

_NotInDebugger:

		// ### Check whether we have to exit redirecting execution to the REAL Windows ISR ###

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bREAL_MouseIntVec, 0
		je			_JustExit

		// Get the Address of the ISR for this Processor.

		lea		eax, [ edx ]SYSINTERRUPTS_STATUS.xirIDTR
		SIDT	[ eax ]

		movzx	ebx, [ edx ]SYSINTERRUPTS_STATUS.bREAL_MouseIntVec
		mov		eax, [ edx ]SYSINTERRUPTS_STATUS.xirIDTR.dwBase

		mov		ecx, [ eax + ebx * 8 ]
		mov		eax, [ eax + ebx * 8 + 4 ]

		and		eax, 0xFFFF0000
		and		ecx, 0x0000FFFF

		or		ecx, eax

		// Set the Stack for the "RET" Instruction.

		mov		dword ptr[ esp + MACRO_SIZEOF_PUSHED_VARS ], ecx

		// Exit.

		jmp			_ExitFromISRWithRET

_JustExit:

//-----------------------------------------------------------------------------------------------------------------------
//   ===> ISR EXIT <===
//-----------------------------------------------------------------------------------------------------------------------

		// ### Pop the State. ###

		MACRO_INTHOOK_POP_REGISTERS_PLAIN

		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".

		// ### Call the Original Handler. ###

		jmp			Trampoline_MouseISR

//-----------------------------------------------------------------------------------------------------------------------

_ExitFromISRWithRET:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN

		// ### Return. ###

		ret

//-----------------------------------------------------------------------------------------------------------------------

_ExitFromISRWithIRETD_Plain:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN

		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".
		iretd

//-----------------------------------------------------------------------------------------------------------------------
	}
}

//=====================================
// Global Data Related to the IPI ISR.
//=====================================

static PVOID		g_pfnIpiISR = NULL;
DETOUR_TRAMPOLINE_GLOBVAR( void Trampoline_IpiISR( void ), g_pfnIpiISR )

//====================================
// Hooked_IpiISR Function Definition.
//====================================

static void __declspec( naked ) Hooked_IpiISR( void )
{
	__asm
	{
		// ### Push the State. ###

		//-------------------------------------------------------- ISR's Pushs ---
		#define MACRO_IPI_SIZEOF_PUSHED_VARS		0x1A

		push		eax
		push		ebx
		push		ecx
		push		edx
		pushf
		push		ds
		push		es
		//------------------------------------------------------------------------
		#define MACRO_IPI_STACKVARS_SIZE			( SIZE x86_FPU_RESTOREINFS )

		sub			esp, MACRO_IPI_STACKVARS_SIZE
		//------------------------------------------------------------------------
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_EAX			( 0x16 + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_EBX			( 0x12 + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_ECX			( 0x0E + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_EDX			( 0x0A + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_DS				( 0x04 + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_ES				( 0x00 + MACRO_IPI_STACKVARS_SIZE )

#define MACRO_IPI_PROCCONTEXT_STACK_DISP_SS_LESS		( MACRO_IPI_SIZEOF_PUSHED_VARS + 0x10 + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_ESP_LESS		( MACRO_IPI_SIZEOF_PUSHED_VARS + 0x0C + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_EFLAGS			( MACRO_IPI_SIZEOF_PUSHED_VARS + 0x08 + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_CS				( MACRO_IPI_SIZEOF_PUSHED_VARS + 0x04 + MACRO_IPI_STACKVARS_SIZE )
#define MACRO_IPI_PROCCONTEXT_STACK_DISP_EIP			( MACRO_IPI_SIZEOF_PUSHED_VARS + 0x00 + MACRO_IPI_STACKVARS_SIZE )
		//------------------------------------------------------------------------

		// ### Setup the Environment. ###

		mov			ax, cs:g_wVpcICEDataSegment
		mov			ds, ax
		mov			ax, g_wVpcICEExtraSegment
		mov			es, ax

		cld

		mov			edx, g_psisSysInterrStatus

		// ### Check whether this IPI is wanted and make sure that it is not Nested. ###

		mov			eax, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EIP ]
		cmp			eax, OFFSET Hooked_IpiISR
		jb			_IpiIsNotNested
		cmp			eax, OFFSET _IPIHandlerExit
		ja			_IpiIsNotNested
		jmp			_DontProcessIPI
_IpiIsNotNested:

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsSendingIPIs, FALSE
		jne			_ProcessIPI

_DontProcessIPI:
		// Set ECX to the Linear Address of the Local Apic.

		mov			ecx, [ edx ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr

		// Send the EOI Signal.

		mov			dword ptr[ ecx + 0xB0 ], 0

		// Exit from the Handler.

		jmp			_IPIHandlerExit

_ProcessIPI:

		ENTERING_VPCICE_AREA

		// ### CPU INITIALIZATION ###

		mov			eax, g_pDeviceObject
		mov			eax, [ eax ]DEVICE_OBJECT.DeviceExtension
		cmp			[ eax ]DEVICE_EXTENSION.bInitializationDone, FALSE
		jne			_Initialization_was_done

		push		eax
		push		ebx
		push		ecx
		push		edx
		call		InitializeProcessorForDebugging
		pop			edx
		pop			ecx
		pop			ebx
		pop			eax

_Initialization_was_done:

		// ### Get the Context of this CPU. ###

		// Save the System Context.

		mov			eax, esp	// <-- ## Do NOT Wrap in PUSH/POP Frames... ##
		mov			ebx, edx
		call		SaveCpuContext

		// Save the General Registers.

		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EAX ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.EAX, ecx
		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EBX ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.EBX, ecx
		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_ECX ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.ECX, ecx
		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EDX ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.EDX, ecx

		mov			[ eax ]x86_REGISTERS_CONTEXT.ESI, esi
		mov			[ eax ]x86_REGISTERS_CONTEXT.EDI, edi
		mov			[ eax ]x86_REGISTERS_CONTEXT.EBP, ebp

		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EFLAGS ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.EFLAGS, ecx

		//

		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EIP ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.EIP, ecx
		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_CS ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.CS, cx
		test		ecx, 0x3
		jnz			_FromUserSpace

		lea			ecx, [ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_EFLAGS + 4 ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.ESP, ecx

		mov			cx, ss
		mov			[ eax ]x86_REGISTERS_CONTEXT.SS, cx

		jmp			_SsEspWereSaved

_FromUserSpace:

		mov			ecx, dword ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_ESP_LESS ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.ESP, ecx

		mov			cx, word ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_SS_LESS ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.SS, cx

_SsEspWereSaved:

		//

		mov			cx, word ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_DS ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.DS, cx
		mov			cx, word ptr[ esp + MACRO_IPI_PROCCONTEXT_STACK_DISP_ES ]
		mov			[ eax ]x86_REGISTERS_CONTEXT.ES, cx

		mov			cx, FS
		mov			[ eax ]x86_REGISTERS_CONTEXT.FS, cx
		mov			cx, GS
		mov			[ eax ]x86_REGISTERS_CONTEXT.GS, cx

		// Inform that we have Finished.

		mov			ebx, edx
		call		IncrementNumProcsWithContextFilled

		// ### Block this Processor. ###

		// Set ECX to the Linear Address of the Local Apic.

		mov			ecx, [ edx ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr

		// Send the EOI Signal.

		mov			dword ptr[ ecx + 0xB0 ], 0

		// Set EBX to the Value of the TPR and Mask all the Interrupts.

		mov			ebx, dword ptr [ ecx + 0x80 ]
		mov			dword ptr [ ecx + 0x80 ], 0xEF

		// Spinlock for Debugger being Closed.

_DoInDebuggerTest:
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bInDebugger, 0
		je			_NotInDebugger

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bSTIinIPIs, FALSE
		je			_NoStiInIpi
		sti
_NoStiInIpi:

		jmp			_DoInDebuggerTest
_NotInDebugger:

		// Disable Interrupts.

		cli

		EXITING_VPCICE_AREA

		// Restore the Fpu State.

		push		ebx
		lea			ebx, [ esp + 4 ]	// <-- ## Note: "+4" is caused by the Previous Push... ##
		call		RestoreFPUState
		pop			ebx

		// Restore the TPR.

		mov			dword ptr [ ecx + 0x80 ], ebx

		// ### IPI Handler EXIT. ###

_IPIHandlerExit:

		// Pop the State.

		add			esp, MACRO_IPI_STACKVARS_SIZE

		pop			es
		pop			ds
		popf
		pop			edx
		pop			ecx
		pop			ebx
		pop			eax

		// Return.

		iretd
	}
}

//==================================================
// InstallVpcICEInterruptHooks Function Definition.
//
// =VP= 2010	=FIXME= maybe should read the local apic base address from MSR and then map the resulting IO address using MmMapIoSpace, thus obtaining an exclusive copy.
//				=FIXME= Verify whether CR8 register is used in Vista and above.
//
//==================================================

NTSTATUS InstallVpcICEInterruptHooks ( OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	PVOID		pvIoApicMemoryPtr;
	NTSTATUS	nsIoApicMemFinderRes;
	ULONG		ulIoApicMemFinderRetItems;
	BOOLEAN		bKeybHookRes, bMouseHookRes;
	NTSTATUS	nsLocalApicMemFinderRes;
	PVOID		vpvLocalApicMemFinds[ 16 ];
	ULONG		ulLocalApicMemFindsItemsNum = sizeof( vpvLocalApicMemFinds ) / sizeof( PVOID );
	PVOID		pvLocalApicMemoryPtr;
	ULONG		ulI;
	BOOLEAN		bIpiHookRes;

	// Initialize the Structure.

	memset( psisSysInterrStatus, 0, sizeof( SYSINTERRUPTS_STATUS ) );

	psisSysInterrStatus->bTextInsert = TRUE;

	InitializeMultiProcessorSpinLock( & psisSysInterrStatus->mpslIpiSpinLock );
	InitializeMultiProcessorSpinLock( & psisSysInterrStatus->mpslMapViewOfImageSectionSync );
	InitializeMultiProcessorSpinLock( & psisSysInterrStatus->mpslReentrancySpinLock );
	InitializeMultiProcessorSpinLock( & psisSysInterrStatus->mpslMousePacketSpinLock );
	InitializeMultiProcessorSpinLock( & psisSysInterrStatus->mpslPfmdbSpinLock );

	// Initialize the Global Variables.

	g_psisSysInterrStatus = psisSysInterrStatus;

	__asm
	{
		mov		ax, ds
		mov		g_wVpcICEDataSegment, ax

		mov		ax, es
		mov		g_wVpcICEExtraSegment, ax

		mov		ax, fs
		mov		g_wFSSEG_in_Kernel, ax
	}

	// Check to see if we have an I/O Apic installed in the System.

	nsIoApicMemFinderRes = PhysAddressToLinearAddresses( & pvIoApicMemoryPtr, 1, & ulIoApicMemFinderRetItems, 0xFEC00000 );

	if ( nsIoApicMemFinderRes != STATUS_SUCCESS || ulIoApicMemFinderRetItems == 0 )
	{
		nsIoApicMemFinderRes = PhysAddressToLinearAddresses( & pvIoApicMemoryPtr, 1, & ulIoApicMemFinderRetItems, 0xFEC10000 );

		if ( nsIoApicMemFinderRes != STATUS_SUCCESS || ulIoApicMemFinderRetItems == 0 )
			pvIoApicMemoryPtr = NULL;
	}

	// Do more controls upon the found Address.

	if ( pvIoApicMemoryPtr )
	{
		if ( MmIsAddressValid( pvIoApicMemoryPtr ) )
		{
			__asm
			{
				push		eax
				push		ebx
				push		ecx
				push		edx

				cli

				mov			eax, pvIoApicMemoryPtr

				mov			ebx, [ eax ]
				mov			[ eax ], MACRO_IOREDTBL0_0
				mov			ecx, [ eax + 0x10 ]

				mov			[ eax ], MACRO_IOAPICVER
				mov			edx, [ eax + 0x10 ]

				cmp			ecx, edx
				jne			_IoApicIsEffectivelyPresent

				mov			[ eax ], ebx
				mov			[ eax + 0x10 ], ecx

				mov			pvIoApicMemoryPtr, 0

_IoApicIsEffectivelyPresent:

				sti

				pop			edx
				pop			ecx
				pop			ebx
				pop			eax
			}
		}
		else
		{
			pvIoApicMemoryPtr = NULL;
		}
	}

	// Save the Address of the I/O Apic.

	psisSysInterrStatus->pvIoApicMemoryPtr = pvIoApicMemoryPtr;

	if ( pvIoApicMemoryPtr )
		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": I/O Apic found at linear address \"0x%.8X\".", (DWORD) pvIoApicMemoryPtr );

	// If we have an I/O Apic, calculate the Items Num of the "IORedirTableBackup" Vector.

	__asm
	{
		cli

		push		eax
		push		ebx

		mov			ebx, psisSysInterrStatus

		mov			eax, pvIoApicMemoryPtr
		or			eax, eax
		jz			_IoApicIsNOTPresent

		mov			dword ptr [ eax ], MACRO_IOAPICVER
		mov			eax, dword ptr [ eax + 0x10 ]

		shr			eax, 0x10
		movzx		eax, al
		inc			eax

		cmp			eax, MACRO_VPCICE_MAXIMUM_INTINPPINS_SUPPORTED
		jb			_IoApicIntInpPinsBelowLimit

		mov			eax, MACRO_VPCICE_MAXIMUM_INTINPPINS_SUPPORTED

_IoApicIntInpPinsBelowLimit:

		mov			[ ebx ]SYSINTERRUPTS_STATUS.dwIORedirTableBackupItemsNum, eax

_IoApicIsNOTPresent:

		pop			ebx
		pop			eax

		sti
	}

	if ( psisSysInterrStatus->pvIoApicMemoryPtr &&
		( psisSysInterrStatus->dwIORedirTableBackupItemsNum == 0 || psisSysInterrStatus->dwIORedirTableBackupItemsNum > MACRO_VPCICE_MAXIMUM_INTINPPINS_SUPPORTED ) )
			return STATUS_UNSUCCESSFUL;

	// Search for the Linear Address of the Local APIC.

	pvLocalApicMemoryPtr = NULL;

	if ( psisSysInterrStatus->pvIoApicMemoryPtr )
	{
		// Do the search in the Page Directory.

		nsLocalApicMemFinderRes = PhysAddressToLinearAddresses( vpvLocalApicMemFinds,
			ulLocalApicMemFindsItemsNum, & ulLocalApicMemFindsItemsNum,
			0xFEE00000 );

		// Check out the Results.

		if ( nsLocalApicMemFinderRes == STATUS_SUCCESS )
		{
			for ( ulI = 0; ulI < ulLocalApicMemFindsItemsNum; ulI ++ )
				if ( vpvLocalApicMemFinds[ ulI ] == (PVOID) 0xFFFE0000 )
				{
					pvLocalApicMemoryPtr = (PVOID) 0xFFFE0000;
					break;
				}

			if ( pvLocalApicMemoryPtr == NULL && ulLocalApicMemFindsItemsNum )
				pvLocalApicMemoryPtr = vpvLocalApicMemFinds[ 0 ];
		}

		// We MUST find at least one address.

		if ( pvLocalApicMemoryPtr == NULL )
		{
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Unable to find Linear Address of Local APIC for Current Processor. Aborting." );
			return STATUS_UNSUCCESSFUL;
		}
		else
		{
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Using Linear Address for accessing Per-Processor Local Apic at \"0x%.8X\".", (DWORD) pvLocalApicMemoryPtr );
		}
	}

	psisSysInterrStatus->pvLocalApicMemoryPtr = pvLocalApicMemoryPtr;

	// Hook the VpcICE IPI ISR, if required.

	if ( psisSysInterrStatus->pvLocalApicMemoryPtr )
	{
		// Get the Address of the ISR.

		GetVpcICEIPIISRLinearAddress( psisSysInterrStatus );

		if ( psisSysInterrStatus->pvIpiIsrAddr == NULL )
			return STATUS_UNSUCCESSFUL;

		// Patch the ISR Entry Point.

		__try
		{
			g_pfnIpiISR = psisSysInterrStatus->pvIpiIsrAddr;
			bIpiHookRes = DetourFunctionWithTrampoline( (PBYTE) Trampoline_IpiISR, (PBYTE) Hooked_IpiISR, NULL );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			bIpiHookRes = FALSE;
		}

		if ( bIpiHookRes == FALSE )
			return STATUS_UNSUCCESSFUL;

		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": " MACRO_PROGRAM_NAME " Inter-Processor Interrupt installed at Vector \"0x%.2X\". ISR at Linear Address \"0x%.8X\".", MACRO_VPCICE_IPI_INTVECTOR, psisSysInterrStatus->pvIpiIsrAddr );
	}

	// Get the Linear Address of the ISR of the Keyboard.

	GetKeyboardISRLinearAddress( psisSysInterrStatus );

	if ( psisSysInterrStatus->bREAL_MouseIntVec )
		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Real Mouse Interrupt Vector detected at 0x%.2X.", psisSysInterrStatus->bREAL_MouseIntVec );
	if ( psisSysInterrStatus->bREAL_KeybIntVec )
		OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Real Keyboard Interrupt Vector detected at 0x%.2X.", psisSysInterrStatus->bREAL_KeybIntVec );

	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Installing Mouse Trampoline for Interrupt Vector 0x%.2X.", psisSysInterrStatus->bMouseIntVec );
	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Installing Keyboard Trampoline for Interrupt Vector 0x%.2X.", psisSysInterrStatus->bKeybIntVec );

	// Check if we were unable to get the ISR Address.

	if ( psisSysInterrStatus->pvMouseIsrAddr == NULL ||
		psisSysInterrStatus->pvKeybIsrAddr == NULL )
	{
		return STATUS_UNSUCCESSFUL;
	}

	// Disassemble the Entry Point of the ISR.

	__try
	{
		g_pfnMouseISR = psisSysInterrStatus->pvMouseIsrAddr;
		bMouseHookRes = DetourFunctionWithTrampoline( (PBYTE) Trampoline_MouseISR, (PBYTE) Hooked_MouseISR, & psisSysInterrStatus->dtjiJmpInsertInfo_Mouse );

		g_pfnKeyboardISR = psisSysInterrStatus->pvKeybIsrAddr;
		bKeybHookRes = DetourFunctionWithTrampoline( (PBYTE) Trampoline_KeyboardISR, (PBYTE) Hooked_KeyboardISR, & psisSysInterrStatus->dtjiJmpInsertInfo );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		bMouseHookRes = FALSE;
		bKeybHookRes = FALSE;
	}

	// Check if the Disassemble Operation failed.

	if ( bMouseHookRes == FALSE ||
		bKeybHookRes == FALSE )
	{
		return STATUS_UNSUCCESSFUL;
	}

	if ( psisSysInterrStatus->dtjiJmpInsertInfo_Mouse.cbCode < 5 ||
		psisSysInterrStatus->dtjiJmpInsertInfo.cbCode < 5 )
	{
		return STATUS_UNSUCCESSFUL;
	}

	// Start the Vmware special thread, that addresses the virtual 8042 compatibility problems. (=VP= 2010)

	if ( VmWareVersion || _8042MinimalProgramming )
	{
		HANDLE				hThread;
		OBJECT_ATTRIBUTES	oa;

		InitializeObjectAttributes( & oa, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );

		if ( ! NT_SUCCESS( PsCreateSystemThread( & hThread, THREAD_ALL_ACCESS, & oa, NULL, NULL, (PKSTART_ROUTINE) VmWareKeyboardIntDeReentrancyThreadProc, NULL ) ) )
		{
			OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Unable to start VmWare Keyboard Int De-Reentrancy Thread." );
			return STATUS_UNSUCCESSFUL;
		}

		ZwClose( hThread );
	}

	// Hook the ISR of the Keyboard.

	HookKeyboardISR( psisSysInterrStatus );

	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Installed trampoline for IRQ12 (PrevISR: \"0x%.8X\", NewISR: \"0x%.8X\").",
		(DWORD) psisSysInterrStatus->dtjiJmpInsertInfo_Mouse.pbCode,
		(DWORD) psisSysInterrStatus->dtjiJmpInsertInfo_Mouse.pbDest );
	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Installed trampoline for IRQ1 (PrevISR: \"0x%.8X\", NewISR: \"0x%.8X\").",
		(DWORD) psisSysInterrStatus->dtjiJmpInsertInfo.pbCode,
		(DWORD) psisSysInterrStatus->dtjiJmpInsertInfo.pbDest );

	// Return to the Caller.

	OutputPrint( FALSE, FALSE, MACRO_PROGRAM_NAME ": Exiting \"Install" MACRO_PROGRAM_NAME "InterruptHooks\" with Success." );

	return STATUS_SUCCESS;
}

//==================================================
// GetKeyboardISRLinearAddress Function Definition.
//==================================================

VOID GetKeyboardISRLinearAddress( IN OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	// Reset the Ptr.

	psisSysInterrStatus->pvKeybIsrAddr = NULL;
	psisSysInterrStatus->bREAL_KeybIntVec = 0;

	// Do the Requested Operation.

	__asm
	{
		// Save the Registers in the Stack.

		push	edi
		push	eax
		push	ebx
		push	ecx
		push	edx

		// Load the Address of the Structure.

		mov		edi, psisSysInterrStatus

		// Mask all the System Interrupts.

		cli

		in		al, 0x21
		mov		[ edi ]SYSINTERRUPTS_STATUS.bPic1IMR, al
		in		al, 0xA1
		mov		[ edi ]SYSINTERRUPTS_STATUS.bPic2IMR, al

		mov		al, 0xFF
		out		0x21, al
		out		0xA1, al

		// Get the Interrupt Vector of the Keyboard.

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr

		or		eax, eax
		jz		_IoApicIsNOTPresent

		// for keyboard.

		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_KEYBOARD_IOREDTBL_0
		mov		eax, [ eax + 0x10 ]

		mov		[ edi ]SYSINTERRUPTS_STATUS.bKeybIntVec, MACRO_IOAPIC_KEYBOARD_PROXY_INTVECTOR
		mov		[ edi ]SYSINTERRUPTS_STATUS.bREAL_KeybIntVec, al

		// for mouse.

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr

		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_MOUSE_IOREDTBL_0
		mov		eax, [ eax + 0x10 ]

		mov		[ edi ]SYSINTERRUPTS_STATUS.bMouseIntVec, MACRO_IOAPIC_MOUSE_PROXY_INTVECTOR
		mov		[ edi ]SYSINTERRUPTS_STATUS.bREAL_MouseIntVec, al

		jmp		_GetKeybIsrAddress

_IoApicIsNOTPresent:

		mov		[ edi ]SYSINTERRUPTS_STATUS.bKeybIntVec, MACRO_WINNT_DEFAULT_INTERRUPT_VECTOR_BASE + MACRO_x86_KEYBOARD_IRQ
		mov		[ edi ]SYSINTERRUPTS_STATUS.bMouseIntVec, MACRO_WINNT_DEFAULT_INTERRUPT_VECTOR_BASE + MACRO_x86_MOUSE_IRQ

_GetKeybIsrAddress:

		lea		eax, [ edi ]SYSINTERRUPTS_STATUS.xirIDTR
		SIDT	[ eax ]

		// Get the Address of the Mouse ISR.

		movzx	ebx, [ edi ]SYSINTERRUPTS_STATUS.bMouseIntVec
		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.xirIDTR.dwBase

		mov		ecx, [ eax + ebx * 8 ]
		mov		edx, [ eax + ebx * 8 + 4 ]

		and		edx, 0xFFFF0000
		and		ecx, 0x0000FFFF

		or		ecx, edx
		mov		[ edi ]SYSINTERRUPTS_STATUS.pvMouseIsrAddr, ecx

		// Get the Address of the Keyboard ISR.

		movzx	ebx, [ edi ]SYSINTERRUPTS_STATUS.bKeybIntVec
		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.xirIDTR.dwBase

		mov		ecx, [ eax + ebx * 8 ]
		mov		edx, [ eax + ebx * 8 + 4 ]

		and		edx, 0xFFFF0000
		and		ecx, 0x0000FFFF

		or		ecx, edx
		mov		[ edi ]SYSINTERRUPTS_STATUS.pvKeybIsrAddr, ecx

		// Restore the previous Interrupt Mask.

		mov		al, [ edi ]SYSINTERRUPTS_STATUS.bPic2IMR
		out		0xA1, al
		mov		al, [ edi ]SYSINTERRUPTS_STATUS.bPic1IMR
		out		0x21, al

		sti

		// Restore the Stack.

		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
		pop		edi
	}

	// Return to the Caller.

	return;
}

//======================================
// HookKeyboardISR Function Definition.
//======================================

VOID HookKeyboardISR( IN OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	// Do the Requested Operation.

	__asm
	{
		// Save the Registers in the Stack.

		push	edi
		push	eax
		push	ebx
		push	ecx

		// Load the Address of the Structure.

		mov		edi, psisSysInterrStatus

		// Mask all the System Interrupts.

		cli

		in		al, 0x21
		mov		[ edi ]SYSINTERRUPTS_STATUS.bPic1IMR, al
		in		al, 0xA1
		mov		[ edi ]SYSINTERRUPTS_STATUS.bPic2IMR, al

		mov		al, 0xFF
		out		0x21, al
		out		0xA1, al

		// Mask the Keyboard Interrupt at I/O Apic level.

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr

		or		eax, eax
		jz		_IoApicIsNOTPresent

		// for mouse.

		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_MOUSE_IOREDTBL_0
		mov		ebx, [ eax + 0x10 ]

		mov		[ edi ]SYSINTERRUPTS_STATUS.dwPrevMouseIoRedTRLow32, ebx

		or		ebx, 0x00010000
		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_MOUSE_IOREDTBL_0
		mov		[ eax + 0x10 ], ebx

		// for keyboard.

		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_KEYBOARD_IOREDTBL_0
		mov		ebx, [ eax + 0x10 ]

		mov		[ edi ]SYSINTERRUPTS_STATUS.dwPrevKeybIoRedTRLow32, ebx

		or		ebx, 0x00010000
		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_KEYBOARD_IOREDTBL_0
		mov		[ eax + 0x10 ], ebx

_IoApicIsNOTPresent:

		// Disable WinNT "Copy-on-Write".

		mov		eax, cr0
		mov		[ edi ]SYSINTERRUPTS_STATUS.dwCR0, eax

		and		eax, 0xFFFEFFFF
		mov		cr0, eax

		// Hook the Entry Point of the ISR, according to the results of the Disassembler.

		// for mouse.

		mov		ebx, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo_Mouse.pbDest
		sub		ebx, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo_Mouse.pbCode
		sub		ebx, 5

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo_Mouse.pbCode

		mov		byte ptr[ eax ], 0xE9
		mov		dword ptr[ eax + 1 ], ebx

		add		eax, 5

		mov		ecx, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo_Mouse.cbCode
		sub		ecx, 5

		push	edi
		mov		edi, eax
		mov		al, 0xCC
		rep		stosb
		pop		edi

		// for keyboard.

		mov		ebx, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo.pbDest
		sub		ebx, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo.pbCode
		sub		ebx, 5

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo.pbCode

		mov		byte ptr[ eax ], 0xE9
		mov		dword ptr[ eax + 1 ], ebx

		add		eax, 5

		mov		ecx, [ edi ]SYSINTERRUPTS_STATUS.dtjiJmpInsertInfo.cbCode
		sub		ecx, 5

		push	edi
		mov		edi, eax
		mov		al, 0xCC
		rep		stosb
		pop		edi

		// Enable WinNT "Copy-on-Write".

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.dwCR0
		mov		cr0, eax

		// Restore the I/O Apic Register and Set the New Interrupt Vector for the Keyboard.

		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr

		or		eax, eax
		jz		_IoApicIsNOTPresent2

		// for mouse.

		mov		ebx, [ edi ]SYSINTERRUPTS_STATUS.dwPrevMouseIoRedTRLow32

		mov		bl, [ edi ]SYSINTERRUPTS_STATUS.bMouseIntVec		// Impose the New Interrupt Vector.

		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_MOUSE_IOREDTBL_0
		mov		[ eax + 0x10 ], ebx

		// for keyboard.

		mov		ebx, [ edi ]SYSINTERRUPTS_STATUS.dwPrevKeybIoRedTRLow32

		mov		bl, [ edi ]SYSINTERRUPTS_STATUS.bKeybIntVec		// Impose the New Interrupt Vector.

		mov		[ eax ], MACRO_WINNT_DEFAULT_IOAPIC_KEYBOARD_IOREDTBL_0
		mov		[ eax + 0x10 ], ebx

_IoApicIsNOTPresent2:

		// Restore the previous Interrupt Mask.

		mov		al, [ edi ]SYSINTERRUPTS_STATUS.bPic2IMR
		out		0xA1, al
		mov		al, [ edi ]SYSINTERRUPTS_STATUS.bPic1IMR
		out		0x21, al

		sti

		// Restore the Stack.

		pop		ecx
		pop		ebx
		pop		eax
		pop		edi
	}

	// Return to the Caller.

	return;
}

//====================================
// SendEOIToPIC1 Function Definition.
//====================================

VOID __declspec( naked ) SendEOIToPIC1( VOID )
{
	__asm
	{
		push		eax

		mov			al, 0x20
		out			0x20, al

		pop			eax

		ret
	}
}

//====================================
// SendEOIToPIC2 Function Definition.
//====================================

VOID __declspec( naked ) SendEOIToPIC2( VOID )
{
	__asm
	{
		push		eax

		mov			al, 0x20
		out			0xA0, al

		pop			eax

		ret
	}
}

//=====================================
// MaskInterrupts Function Definition.
//=====================================

VOID MaskInterrupts( IN BYTE bPIC1Mask, IN BYTE bPIC2Mask, OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	DWORD		dwIrqMask = 0xFFFF0000 | ( bPIC2Mask << 8 ) | bPIC1Mask;

	// Do the Requested Operation.

	__asm
	{
		push		eax
		push		ebx
		push		ecx
		push		edx

		mov			ebx, psisSysInterrStatus

		// Skip the 8259 Programming if a I/O APIC is present.

		cmp			[ ebx ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr, 0
		jne			_Skip8259Programming

		// Set IMR of PIC1

		in			al, 0x21
		mov			[ ebx ]SYSINTERRUPTS_STATUS.bPic1IMR, al

		mov			al, bPIC1Mask
		out			0x21, al

		// Set IMR of PIC2

		in			al, 0xA1
		mov			[ ebx ]SYSINTERRUPTS_STATUS.bPic2IMR, al

		mov			al, bPIC2Mask
		out			0xA1, al

_Skip8259Programming:

		// Check to see whether we have an I/O Apic.

		mov			edx, [ ebx ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr
		or			edx, edx
		jz			_IoApicIsNOTPresent

		push		ebx

		mov			eax, 0x10
		mov			ecx, [ ebx ]SYSINTERRUPTS_STATUS.dwIORedirTableBackupItemsNum
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.viiIORedirTableBackup

_IORedirTableBackUpLoop:

		push		ecx

		// Save this Entry.

		mov			[ edx ], eax
		mov			ecx, [ edx + 0x10 ]
		mov			[ ebx ], ecx

		// Modify this Entry.

		test		dwIrqMask, 1
		jz			_DoNotConsiderFirst32Bits

		test		dword ptr [ esp ], 1 // <--- ### LOOP ECX !!! ###
		jz			_DoNotConsiderFirst32Bits

		or			ecx, 0x00010000
		mov			[ edx ], eax
		mov			[ edx + 0x10 ], ecx

_DoNotConsiderFirst32Bits:

		// Increment.

		inc			eax
		add			ebx, 4

		pop			ecx

		test		ecx, 1
		jz			_DoNotShiftIrqMask
		shr			dwIrqMask, 1
_DoNotShiftIrqMask:

		loop		_IORedirTableBackUpLoop

		pop			ebx

_IoApicIsNOTPresent:

		pop			edx
		pop			ecx
		pop			ebx
		pop			eax
	}

	// Return to the Caller.

	return;
}

//========================================
// RestoreInterrupts Function Definition.
//========================================

VOID RestoreInterrupts( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	// Do the Requested Operation.

	__asm
	{
		push		eax
		push		ebx
		push		ecx
		push		edx

		mov			ebx, psisSysInterrStatus

		// Check to see whether we have an I/O Apic.

		mov			edx, [ ebx ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr
		or			edx, edx
		jz			_IoApicIsNOTPresent

		// Restore the I/O Apic Registers.

		push		ebx

		mov			eax, 0x10
		mov			ecx, [ ebx ]SYSINTERRUPTS_STATUS.dwIORedirTableBackupItemsNum
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.viiIORedirTableBackup

_IORedirTableRestoreLoop:

		push		ecx

		mov			ecx, [ ebx ]
		mov			[ edx ], eax
		mov			[ edx + 0x10 ], ecx

		inc			eax
		add			ebx, 4

		pop			ecx

		loop		_IORedirTableRestoreLoop

		pop			ebx

_IoApicIsNOTPresent:

		// Skip the 8259 Programming if a I/O APIC is present.

		cmp			[ ebx ]SYSINTERRUPTS_STATUS.pvIoApicMemoryPtr, 0
		jne			_Skip8259Programming

		// Restore PIC1.

		mov			al, [ ebx ]SYSINTERRUPTS_STATUS.bPic1IMR
		out			0x21, al

		// Restore PIC2.

		mov			al, [ ebx ]SYSINTERRUPTS_STATUS.bPic2IMR
		out			0xA1, al

_Skip8259Programming:

		pop			edx
		pop			ecx
		pop			ebx
		pop			eax
	}

	// Return to the Caller.

	return;
}

//=======================================
// EnableInterrupts Function Definition.
//=======================================

VOID __declspec( naked ) EnableInterrupts( VOID )
{
	__asm
	{
		sti
		ret
	}
}

//========================================
// DisableInterrupts Function Definition.
//========================================

VOID __declspec( naked ) DisableInterrupts( VOID )
{
	__asm
	{
		cli
		ret
	}
}

//=========================================
// SendEOIToLocalAPIC Function Definition.
//=========================================

VOID SendEOIToLocalAPIC( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	// Do the Requested Operation.

	__asm
	{
		push		eax
		push		ebx

		mov			eax, psisSysInterrStatus
		mov			ebx, [ eax ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr

		or			ebx, ebx
		jz			_LocalAPICIsNotPresent

		mov			dword ptr[ ebx + 0xB0 ], 0

_LocalAPICIsNotPresent:

		pop			ebx
		pop			eax
	}

	// Return to the Caller.

	return;
}

//==========================================
// DebuggerLoopQuantum Function Definition.
//==========================================

VOID DebuggerLoopQuantum( IN LARGE_INTEGER liCpuCycles, IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	LARGE_INTEGER			liStampStart, liStampEnd;

	// Main Loop of the Debugger.

	psisSysInterrStatus->bLastAsciiCodeAcquired = 0;

	BugChecker_ReadTimeStampCounter( & liStampStart );

	do
	{
		if ( psisSysInterrStatus->bLastAsciiCodeAcquired )
			break;
		else if ( psisSysInterrStatus->bIsMousePacketReady )
			HandleMouseEvent();

		BugChecker_ReadTimeStampCounter( & liStampEnd );
	}
	while( liStampEnd.QuadPart - liStampStart.QuadPart < liCpuCycles.QuadPart );

	// Check whether the PS/2 Mouse/Keyboard is OK.

	CheckPs2MouseKeybConsistence();

	// Return to the Caller.

	return;
}

//=======================================================================
// CalculateCPUSpeed Function Definition.
//=======================================================================

VOID CalculateCPUSpeed( OUT LARGE_INTEGER* pliCpuClockCyclesPerSecond )
{
	LARGE_INTEGER	liCounterStart, liCounterFreq, liCounterEnd;
	LARGE_INTEGER	liStampStart, liStampEnd;

	// Calculate how many CPU Cycles happen in a second.

	liCounterStart = KeQueryPerformanceCounter( & liCounterFreq );
	liCounterFreq.QuadPart /= MACRO_CALCULATECPUSPEED_FACTOR;

	BugChecker_ReadTimeStampCounter( & liStampStart );

	while( TRUE )
	{
		BugChecker_ReadTimeStampCounter( & liStampEnd );

		liCounterEnd = KeQueryPerformanceCounter( NULL );
		if ( liCounterEnd.QuadPart - liCounterStart.QuadPart > liCounterFreq.QuadPart )
			break;
	}

	pliCpuClockCyclesPerSecond->QuadPart = ( liStampEnd.QuadPart - liStampStart.QuadPart ) *
		MACRO_CALCULATECPUSPEED_FACTOR;

	// Return.

	return;
}

//==================================================
// GetCurrentIrqlFromLocalAPIC Function Definition.
//==================================================

static BYTE			g_vbTaskPriority2IrqlMapping[ 0x10 ] =
	{
		0x00, 0xFF, 0xFF, 0x01, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x1B, 0x1C, 0x1D, 0x1E
	};

KIRQL GetCurrentIrqlFromLocalAPIC( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	if ( psisSysInterrStatus->pvLocalApicMemoryPtr == NULL )
		return 0xFF;

	// Return the Required Information.

	return (KIRQL) g_vbTaskPriority2IrqlMapping[ ( psisSysInterrStatus->dwLocalApicTPR >> 4 ) & 0xF ];
}

//==========================================
// GetIrqlFromTPRValue Function Definition.
//==========================================

KIRQL GetIrqlFromTPRValue( IN DWORD dwTPRValue )
{
	// Return the Required Information.

	return (KIRQL) g_vbTaskPriority2IrqlMapping[ ( dwTPRValue >> 4 ) & 0xF ];
}

//=======================================
// SetLocalAPIC_TPR Function Definition.
//=======================================

VOID SetLocalAPIC_TPR( IN DWORD dwTPRNewValue, IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	if ( psisSysInterrStatus->pvLocalApicMemoryPtr == NULL )
		return;

	// Set the Task Priority Register Value of the Local APIC.

	__asm
	{
		push		eax
		push		ebx
		push		ecx
		push		edx

		mov			eax, psisSysInterrStatus
		mov			ebx, [ eax ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr

		mov			ecx, dword ptr [ ebx + 0x80 ]
		mov			edx, dwTPRNewValue
		mov			dword ptr [ ebx + 0x80 ], edx

		mov			[ eax ]SYSINTERRUPTS_STATUS.dwLocalApicTPR, ecx

		pop			edx
		pop			ecx
		pop			ebx
		pop			eax
	}

	return;
}

//===========================================
// RestoreLocalAPIC_TPR Function Definition.
//===========================================

VOID RestoreLocalAPIC_TPR( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	if ( psisSysInterrStatus->pvLocalApicMemoryPtr == NULL )
		return;

	// Restore the Task Priority Register Value of the Local APIC.

	__asm
	{
		push		eax
		push		ebx
		push		ecx

		mov			eax, psisSysInterrStatus
		mov			ebx, [ eax ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr

		mov			ecx, [ eax ]SYSINTERRUPTS_STATUS.dwLocalApicTPR
		mov			dword ptr [ ebx + 0x80 ], ecx

		pop			ecx
		pop			ebx
		pop			eax
	}

	return;
}

//===================================================
// GetVpcICEIPIISRLinearAddress Function Definition.
//===================================================

VOID GetVpcICEIPIISRLinearAddress( IN OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	// Reset the Ptr.

	psisSysInterrStatus->pvIpiIsrAddr = NULL;

	// Do the Requested Operation.

	__asm
	{
		// Save the Registers in the Stack.

		push	edi
		push	eax
		push	ebx
		push	ecx
		push	edx

		// Load the Address of the Structure.

		mov		edi, psisSysInterrStatus

		// Mask all the System Interrupts.

		cli

		in		al, 0x21
		mov		[ edi ]SYSINTERRUPTS_STATUS.bPic1IMR, al
		in		al, 0xA1
		mov		[ edi ]SYSINTERRUPTS_STATUS.bPic2IMR, al

		mov		al, 0xFF
		out		0x21, al
		out		0xA1, al

		// Get the Address of the VpcICE IPI ISR.

		lea		eax, [ edi ]SYSINTERRUPTS_STATUS.xirIDTR
		SIDT	[ eax ]

		mov		ebx, MACRO_VPCICE_IPI_INTVECTOR
		mov		eax, [ edi ]SYSINTERRUPTS_STATUS.xirIDTR.dwBase

		mov		ecx, [ eax + ebx * 8 ]
		mov		edx, [ eax + ebx * 8 + 4 ]

		and		edx, 0xFFFF0000
		and		ecx, 0x0000FFFF

		or		ecx, edx
		mov		[ edi ]SYSINTERRUPTS_STATUS.pvIpiIsrAddr, ecx

		// Restore the previous Interrupt Mask.

		mov		al, [ edi ]SYSINTERRUPTS_STATUS.bPic2IMR
		out		0xA1, al
		mov		al, [ edi ]SYSINTERRUPTS_STATUS.bPic1IMR
		out		0x21, al

		sti

		// Restore the Stack.

		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
		pop		edi
	}

	// Return to the Caller.

	return;
}

//====================================
// SendVpcICEIPI Function Definition.
//====================================

VOID SendVpcICEIPI( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	if ( psisSysInterrStatus->pvLocalApicMemoryPtr == NULL )
		return;

	// Send the IPI.

	__asm
	{
		push		eax
		push		ebx

		mov			eax, psisSysInterrStatus
		mov			ebx, [ eax ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr

		mov			dword ptr [ ebx + 0x300 ], 0x000C0800 | MACRO_VPCICE_IPI_INTVECTOR

		pop			ebx
		pop			eax
	}

	return;
}

//=====================================
// SaveCpuContext Function Definition.
//=====================================

VOID __declspec( naked ) SaveCpuContext( VOID /* "IN SYSINTERRUPTS_STATUS* psisSysInterrStatus" in EBX Register. "OUT x86_FPU_RESTOREINFS* px86friFpuRestoreInfs" in EAX Register. Returns "x86vicContext" Field Offset in EAX. */ )
{
	__asm
	{
		// Push State.

		push		ecx
		push		edx

		// Push EAX.

		push		eax

		// Enter.

		push		ebx
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslIpiSpinLock
		call		EnterMultiProcessorSpinLock
		pop			ebx

		// Get the Pointers to the Structures.

		cmp			[ ebx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		je			_NoSingleStep

		mov			eax, [ ebx ]SYSINTERRUPTS_STATUS.dwCurrentProcessor
		mov			edx, SIZE x86_CPU_CONTEXT
		mul			edx

		jmp			_EaxHasProcOrdinal

_NoSingleStep:

		mov			eax, [ ebx ]SYSINTERRUPTS_STATUS.dwNumberOfProcessors
		mov			edx, SIZE x86_CPU_CONTEXT
		mul			edx

_EaxHasProcOrdinal:

		lea			ecx, [ ebx + eax ]SYSINTERRUPTS_STATUS.vx86ccProcessors

		// Increment.

		cmp			[ ebx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		jne			_DoNotIncrementProcNum

		inc			[ ebx ]SYSINTERRUPTS_STATUS.dwNumberOfProcessors

_DoNotIncrementProcNum:

		// Leave.

		push		ebx
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslIpiSpinLock
		call		LeaveMultiProcessorSpinLock
		pop			ebx

		// Pop EAX. --- Save EAX Parameter.

		pop			eax
		mov			[ ecx ]x86_CPU_CONTEXT.dwFpuRestoreInfsPtr, eax

		// GDTr.

		lea			eax, [ ecx ]x86_CPU_CONTEXT.xgrGDTR
		SGDT		[ eax ]

		mov			eax, [ ecx ]x86_CPU_CONTEXT.xgrGDTR.dwBase
		mov			[ ecx ]x86_CPU_CONTEXT.pvGDTBase, eax

		// IDTr.

		lea			eax, [ ecx ]x86_CPU_CONTEXT.xirIDTR
		SIDT		[ eax ]

		mov			eax, [ ecx ]x86_CPU_CONTEXT.xirIDTR.dwBase
		mov			[ ecx ]x86_CPU_CONTEXT.pvIDTBase, eax

		// PCR.

		xor			eax, eax
		mov			ax, MACRO_PCR_SELECTOR_IN_SYSTEMCODE
		and			eax, 0xFFFFFFF8
		add			eax, [ ecx ]x86_CPU_CONTEXT.pvGDTBase

		mov			[ ecx ]x86_CPU_CONTEXT.pvPCRBase, 0

		mov			edx, dword ptr[ eax + 4 ]
		and			edx, 0xFF000000
		or			[ ecx ]x86_CPU_CONTEXT.pvPCRBase, edx

		mov			edx, dword ptr[ eax + 4 ]
		and			edx, 0x000000FF
		shl			edx, 16
		or			[ ecx ]x86_CPU_CONTEXT.pvPCRBase, edx

		mov			edx, dword ptr[ eax ]
		shr			edx, 16
		or			[ ecx ]x86_CPU_CONTEXT.pvPCRBase, edx

		// System Registers.

		mov			eax, CR0
		mov			[ ecx ]x86_CPU_CONTEXT.dwCR0, eax

		mov			eax, CR2
		mov			[ ecx ]x86_CPU_CONTEXT.dwCR2, eax

		mov			eax, CR3
		mov			[ ecx ]x86_CPU_CONTEXT.dwCR3, eax

		__asm _emit 0x0F __asm _emit 0x20 __asm _emit 0xE0 /* mov eax, CR4 */
		mov			[ ecx ]x86_CPU_CONTEXT.dwCR4, eax

		mov			eax, DR0
		mov			[ ecx ]x86_CPU_CONTEXT.dwDR0, eax

		mov			eax, DR1
		mov			[ ecx ]x86_CPU_CONTEXT.dwDR1, eax

		mov			eax, DR2
		mov			[ ecx ]x86_CPU_CONTEXT.dwDR2, eax

		mov			eax, DR3
		mov			[ ecx ]x86_CPU_CONTEXT.dwDR3, eax

		mov			eax, DR6
		mov			[ ecx ]x86_CPU_CONTEXT.dwDR6, eax

		mov			eax, DR7
		mov			[ ecx ]x86_CPU_CONTEXT.dwDR7, eax

		// CPUID(1) Informations.

		push		esi
		lea			esi, [ ecx ]x86_CPU_CONTEXT.dwCpuIdInfo

		push		eax
		push		ebx
		push		ecx
		push		edx

		mov			eax, 1
		CPUID
		mov			dword ptr[ esi + 0 ], eax
		mov			dword ptr[ esi + 4 ], ebx
		mov			dword ptr[ esi + 8 ], ecx
		mov			dword ptr[ esi + 12 ], edx

		pop			edx
		pop			ecx
		pop			ebx
		pop			eax

		pop			esi

		// TPr + Local APIC Informations.

		mov			eax, [ ebx ]SYSINTERRUPTS_STATUS.pvLocalApicMemoryPtr
		or			eax, eax
		jz			_LocalApicIsNotPresent

		mov			edx, dword ptr[ eax + 0x80 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwTPRValue, edx

		mov			edx, dword ptr[ eax + 0x20 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwIDRValue, edx

		mov			edx, dword ptr[ eax + 0x30 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwVerRValue, edx

		mov			edx, dword ptr[ eax + 0x90 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwAPRValue, edx

		mov			edx, dword ptr[ eax + 0xA0 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwPPRValue, edx

		mov			edx, dword ptr[ eax + 0xE0 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwDFRValue, edx

		mov			edx, dword ptr[ eax + 0xD0 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwLDRValue, edx

		mov			edx, dword ptr[ eax + 0xF0 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwSVRValue, edx

		mov			edx, dword ptr[ eax + 0x0310 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwICRValue, edx
		mov			edx, dword ptr[ eax + 0x0300 ]
		mov			[ ecx + 4 ]x86_CPU_CONTEXT.dwICRValue, edx

		mov			edx, dword ptr[ eax + 0x0320 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwLVTTRValue, edx

		mov			edx, dword ptr[ eax + 0x0340 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwLVTPMCRValue, edx

		mov			edx, dword ptr[ eax + 0x0350 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwLVTLINT0RValue, edx

		mov			edx, dword ptr[ eax + 0x0360 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwLVTLINT1RValue, edx

		mov			edx, dword ptr[ eax + 0x0370 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwLVTErrRValue, edx

		mov			edx, dword ptr[ eax + 0x0380 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwICountRValue, edx

		mov			edx, dword ptr[ eax + 0x0390 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwCCountRValue, edx

		mov			edx, dword ptr[ eax + 0x03E0 ]
		mov			[ ecx ]x86_CPU_CONTEXT.dwDivCRValue, edx

_LocalApicIsNotPresent:

		// SSE / SSE2 State.

		mov			[ ecx ]x86_CPU_CONTEXT.b_SSE_SSE2_StateIsValid, FALSE

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwCR0

		test		al, (1<<2)			// <-- Emulation (Bit 2).
		jnz			_UnableToAcquireSseSse2State

		mov			eax, [ ecx + ( 4 /* SIZE DWORD */ * 3 /* INDEX */ ) ]x86_CPU_CONTEXT.dwCpuIdInfo

		test		eax, (1<<25)		// <-- SSE Present (Bit 25).
		jz			_UnableToAcquireSseSse2State
		test		eax, (1<<24)		// <-- FXSAVE Present (Bit 24).
		jz			_UnableToAcquireSseSse2State

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwCR4

		test		eax, (1<<9)			// <-- OSFXSR Support (Bit 9).
		jz			_UnableToAcquireSseSse2State

		// // // // (no more jumps to _UnableToAcquireSseSse2State...)
		clts

		//

		mov			[ ecx ]x86_CPU_CONTEXT.bWriteMXCSR, FALSE
		stmxcsr		[ ecx ]x86_CPU_CONTEXT.dwMXCSR

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwMXCSR
		and			eax, 0x3F			// <-- SIMD Exception Flags (Bits 5-0).
		mov			edx, [ ecx ]x86_CPU_CONTEXT.dwMXCSR
		shr			edx, 7
		and			edx, 0x3F			// <-- SIMD Exception Mask (Bits 12-7).

		not			edx
		and			eax, edx
		jz			_NoSseSse2ExceptionPending

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwMXCSR
		and			eax, 0xFFFFFFC0		// <-- SIMD Exception Flags ZEROED (Bits 5-0).
		mov			[ ecx ]x86_CPU_CONTEXT.dwMXCSR_2, eax

		ldmxcsr		[ ecx ]x86_CPU_CONTEXT.dwMXCSR_2

		mov			[ ecx ]x86_CPU_CONTEXT.bWriteMXCSR, TRUE

_NoSseSse2ExceptionPending:

		//

		lea			eax, [ ecx + MACRO_FXSAVE_MEMALIGN ]x86_CPU_CONTEXT.vb_SSE_SSE2_State
		and			eax, MACRO_FXSAVE_MEMALIGN_MASK

		push		eax
		lea			edx, [ ecx ]x86_CPU_CONTEXT.vb_SSE_SSE2_State
		sub			eax, edx
		mov			[ ecx ]x86_CPU_CONTEXT.dw_SSE_SSE2_State_AlignDisp, eax
		pop			eax

		fxsave		[ eax ]

		mov			[ ecx ]x86_CPU_CONTEXT.b_SSE_SSE2_StateIsValid, TRUE

		//

		cmp			[ ecx ]x86_CPU_CONTEXT.bWriteMXCSR, FALSE
		je			_NoSseSse2ExceptionFlagsRestore

		ldmxcsr		[ ecx ]x86_CPU_CONTEXT.dwMXCSR

_NoSseSse2ExceptionFlagsRestore:

		//

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwCR0
		mov			CR0, eax
		// // // //

_UnableToAcquireSseSse2State:

		// x87 / MMX State.

		mov			[ ecx ]x86_CPU_CONTEXT.b_x87_MMX_StateIsValid, FALSE

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwCR0

		test		al, (1<<2)			// <-- Emulation (Bit 2).
		jnz			_UnableToAcquirex87MmxState

		// // // // (no more jumps to _UnableToAcquirex87MmxState...)
		clts

		//

		fnsave		[ ecx ]x86_CPU_CONTEXT.vb_x87_MMX_State		// ### ### ### WARNING ### ### ###
																//  -> From this point on, FPU State freshened and INTs masked.

		mov			[ ecx ]x86_CPU_CONTEXT.b_x87_MMX_StateIsValid, TRUE

		//

		// mov		eax, [ ecx ]x86_CPU_CONTEXT.dwCR0	// ### ### ### WARNING ### ### ###
		// mov		CR0, eax							//	-> TS Flag in CR0 remains CLEARED !!!
		// // // //

_UnableToAcquirex87MmxState:

		// Populate the x86_FPU_RESTOREINFS Structure.

		mov			eax, [ ecx ]x86_CPU_CONTEXT.dwFpuRestoreInfsPtr

		mov			edx, [ ecx ]x86_CPU_CONTEXT.dwCR0
		mov			[ eax ]x86_FPU_RESTOREINFS.dwCR0, edx

		mov			dl, [ ecx ]x86_CPU_CONTEXT.b_x87_MMX_StateIsValid
		mov			[ eax ]x86_FPU_RESTOREINFS.b_x87_MMX_StateIsValid, dl

		push		esi
		push		edi
		push		ecx

		lea			esi, [ ecx ]x86_CPU_CONTEXT.vb_x87_MMX_State
		lea			edi, [ eax ]x86_FPU_RESTOREINFS.vb_x87_MMX_State
		mov			ecx, MACRO_FNSAVE_MEMSIZE
		rep			movsb

		pop			ecx
		pop			edi
		pop			esi

		// Return Value.

		lea			eax, [ ecx ]x86_CPU_CONTEXT.x86vicContext

		// Pop State.

		pop			edx
		pop			ecx

		// Return.

		ret
	}
}

//=========================================================
// IncrementNumProcsWithContextFilled Function Definition.
//=========================================================

VOID __declspec( naked ) IncrementNumProcsWithContextFilled( VOID /* "IN SYSINTERRUPTS_STATUS* psisSysInterrStatus" in EBX Register. */ )
{
	__asm
	{
		// Enter.

		push		ebx
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslIpiSpinLock
		call		EnterMultiProcessorSpinLock
		pop			ebx

		// Increment.

		inc			[ ebx ]SYSINTERRUPTS_STATUS.dwNumProcsWithContextFilled

		// Leave.

		push		ebx
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslIpiSpinLock
		call		LeaveMultiProcessorSpinLock
		pop			ebx

		// Return.

		ret
	}
}

//==============================================
// ReorderListOfProcessors Function Definition.
//==============================================

static x86_CPU_CONTEXT		g_x86ccSwapObj;

VOID ReorderListOfProcessors( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus )
{
	ULONG				ulI, ulJ;
	x86_CPU_CONTEXT*	px86ccThis;
	VOID*				pvCurrentCpuPcr;
	x86_CPU_CONTEXT*	px86ccSecond;

	pvCurrentCpuPcr = psisSysInterrStatus->vx86ccProcessors[ 0 ].pvPCRBase;

	// Reorder the List.

	for ( ulI = 0; ulI < psisSysInterrStatus->dwNumberOfProcessors; ulI ++ )
	{
		px86ccThis = & psisSysInterrStatus->vx86ccProcessors[ ulI ];

		for ( ulJ = ulI + 1; ulJ < psisSysInterrStatus->dwNumberOfProcessors; ulJ ++ )
		{
			px86ccSecond = & psisSysInterrStatus->vx86ccProcessors[ ulJ ];

			if ( px86ccSecond->pvPCRBase == (VOID*) MACRO_PCR_ADDRESS_OF_1ST_PROCESSOR )
			{
				px86ccThis = px86ccSecond;
				break;
			}
			else if ( px86ccSecond->pvPCRBase < px86ccThis->pvPCRBase &&
				px86ccThis->pvPCRBase != (VOID*) MACRO_PCR_ADDRESS_OF_1ST_PROCESSOR )
			{
				px86ccThis = px86ccSecond;
			}
		}

		if ( px86ccThis != & psisSysInterrStatus->vx86ccProcessors[ ulI ] )
		{
			g_x86ccSwapObj = * px86ccThis;
			* px86ccThis = psisSysInterrStatus->vx86ccProcessors[ ulI ];
			psisSysInterrStatus->vx86ccProcessors[ ulI ] = g_x86ccSwapObj;
		}
	}

	// Set the Current Processor Index.

	for ( ulI = 0; ulI < psisSysInterrStatus->dwNumberOfProcessors; ulI ++ )
		if ( psisSysInterrStatus->vx86ccProcessors[ ulI ].pvPCRBase == pvCurrentCpuPcr )
		{
			psisSysInterrStatus->dwCurrentProcessor = ulI;
			break;
		}

	// Return to the Caller.

	return;
}

//======================================
// RestoreFPUState Function Definition.
//======================================

VOID __declspec( naked ) RestoreFPUState( VOID /* "IN x86_FPU_RESTOREINFS* px86friFpuRestoreInfs" in EBX Register. */ )
{
	__asm
	{
		// Push.

		push		eax

		// Restore.

		cmp			[ ebx ]x86_FPU_RESTOREINFS.b_x87_MMX_StateIsValid, FALSE
		je			_NoFpuStateAvailable

		frstor		[ ebx ]x86_FPU_RESTOREINFS.vb_x87_MMX_State

_NoFpuStateAvailable:

		mov			eax, [ ebx ]x86_FPU_RESTOREINFS.dwCR0
		mov			CR0, eax

		// Pop.

		pop			eax

		// Return.

		ret
	}
}

//=====================================
// InstallIDTHook Function Definition.
//=====================================

BOOLEAN InstallIDTHook( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulVectorNum, IN DWORD dwNewISRAddress )
{
	ULONG					ulI;
	BOOLEAN					bRetVal = TRUE;
	x86_CPU_CONTEXT*		px86ccCPU;
	IDT_HOOKEDENTRY_INFO*	piheiIDTHook;
	DWORD*					pdwIdtDesc4;
	DWORD*					pdwIdtDesc0;
	DWORD					dwStubAddress;

	// Iterate through all the Processors in the System.

	for ( ulI = 0; ulI < psisSysInterrStatus->dwNumberOfProcessors; ulI ++ )
	{
		px86ccCPU = & psisSysInterrStatus->vx86ccProcessors[ ulI ];

		// Check if we can Continue.

		if ( psisSysInterrStatus->ulIdtHooksNum >= MACRO_MAXNUM_OF_IDTHOOKEDENTRYINFOS )
		{
			bRetVal = FALSE;
			break;
		}

		// Check the Nature of the IDT Descriptor.

		pdwIdtDesc0 = ( DWORD* ) ( ( (BYTE*) px86ccCPU->pvIDTBase ) + ( ulVectorNum * 8 ) );
		pdwIdtDesc4 = pdwIdtDesc0 + 1;

		if ( ( ( (*pdwIdtDesc4) >> 15 ) & 1 ) == 0 /* Segment Present flag must be 1 */ ||
			( ( (*pdwIdtDesc4) >> 5 ) & 0xFF ) != 0x70 /* Must be an Interrupt Gate */ )
		{
			bRetVal = FALSE;
			continue;
		}

		// Install the Hook.

		piheiIDTHook = & psisSysInterrStatus->viheiIdtHooks[ psisSysInterrStatus->ulIdtHooksNum ++ ];

		piheiIDTHook->ulVectorNum = ulVectorNum;
		piheiIDTHook->ulProcessorNum = ulI;
		piheiIDTHook->bPushOpcode = 0x68;
		piheiIDTHook->dwOriginalISRAddress = ( (*pdwIdtDesc4) & 0xFFFF0000 ) | ( (*pdwIdtDesc0) & 0x0000FFFF );
		piheiIDTHook->bJumpOpcode = 0xE9;
		piheiIDTHook->dwNewISRAddress_Sub =
			dwNewISRAddress - ( ((DWORD)piheiIDTHook) + FIELD_OFFSET( IDT_HOOKEDENTRY_INFO, bJumpOpcode ) ) - 5;

		dwStubAddress = ( (DWORD) piheiIDTHook ) + FIELD_OFFSET( IDT_HOOKEDENTRY_INFO, bPushOpcode );

		*pdwIdtDesc4 = ( (*pdwIdtDesc4) & 0x0000FFFF ) | ( dwStubAddress & 0xFFFF0000 );
		*pdwIdtDesc0 = ( (*pdwIdtDesc0) & 0xFFFF0000 ) | ( dwStubAddress & 0x0000FFFF );
	}

	// Return to the Caller.

	return bRetVal;
}

//=====================================
// InvokeDebugger Function Definition.
//=====================================

VOID __declspec( naked ) InvokeDebugger( VOID /* parameters: InvokeDebugger section of SYSINTERRUPTS_STATUS. */ )
{
	__asm
	{
		mov			edx, g_psisSysInterrStatus

		// Reset the State of the Keyboard.

		mov			[ edx ]SYSINTERRUPTS_STATUS.bLastScanCodeAcquired, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bLastAsciiCodeAcquired, 0

		mov			[ edx ]SYSINTERRUPTS_STATUS.bE0Recv, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E0Recv, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bE1Recv, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bPREV_E1Recv, 0

		mov			[ edx ]SYSINTERRUPTS_STATUS.bShiftPressed, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bControlPressed, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.bAltPressed, 0

		mov			[ edx ]SYSINTERRUPTS_STATUS.bKeyboardLEDsPressed, 0

		// Load the State of the Processor in our Context Structure.

		//

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		jne			_DoNotResetProcRelatedVars

		mov			[ edx ]SYSINTERRUPTS_STATUS.dwNumberOfProcessors, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.dwNumProcsWithContextFilled, 0
		mov			[ edx ]SYSINTERRUPTS_STATUS.dwCurrentProcessor, 0

_DoNotResetProcRelatedVars:

		//

		lea			eax, [ edx ]SYSINTERRUPTS_STATUS.x86friFpuRestoreInfs
		mov			ebx, edx
		call		SaveCpuContext
		mov			edi, eax

		//

		push		edi

		lea			esi, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT
		mov			ecx, SIZE x86_REGISTERS_CONTEXT
		rep			movsb

		pop			edi

		//

		// The context for This Processor is Set.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		jne			_DoNotIncrementContextFillCount

		mov			ebx, edx
		call		IncrementNumProcsWithContextFilled

_DoNotIncrementContextFillCount:

		// Set the Contents of the Previous Context Status Structure.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, FALSE
		je			_DoNotCopyPrevContext

		push		edi

		mov			esi, edi
		lea			edi, [ edx ]SYSINTERRUPTS_STATUS.x86vicPrevContext
		mov			ecx, SIZE x86_REGISTERS_CONTEXT
		rep			movsb

		pop			edi

_DoNotCopyPrevContext:

		// Save the Context Status Structure for later use.

		mov			esi, edi
		lea			edi, [ edx ]SYSINTERRUPTS_STATUS.x86vicContextCopy
		mov			ecx, SIZE x86_REGISTERS_CONTEXT
		rep			movsb

		// Call the Handler.

		push		edx

		// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

		mov			edx, g_pDeviceObject
		mov			edx, [ edx ]DEVICE_OBJECT.DeviceExtension

		mov			[ edx ]DEVICE_EXTENSION.dwPrevStackPointer, esp
		mov			esp, [ edx ]DEVICE_EXTENSION.pbStackPointerStart

		call		DebuggerInvokedCore

		mov			edx, g_pDeviceObject
		mov			edx, [ edx ]DEVICE_OBJECT.DeviceExtension

		mov			esp, [ edx ]DEVICE_EXTENSION.dwPrevStackPointer

		// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

		pop			edx

		// Check whether copying the Context Status Structure is Required.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, FALSE
		jne			_DoNotCopyPrevContext2

		lea			esi, [ edx ]SYSINTERRUPTS_STATUS.x86vicContextCopy
		lea			edi, [ edx ]SYSINTERRUPTS_STATUS.x86vicPrevContext
		mov			ecx, SIZE x86_REGISTERS_CONTEXT
		rep			movsb

_DoNotCopyPrevContext2:

		// Copy the Processor Context to the CONTEXT structure that will update the system.

		push		edx

		mov			eax, [ edx ]SYSINTERRUPTS_STATUS.dwCurrentProcessor
		mov			edx, SIZE x86_CPU_CONTEXT
		mul			edx

		pop			edx

		lea			eax, [ edx + eax ]SYSINTERRUPTS_STATUS.vx86ccProcessors

		lea			esi, [ eax ]x86_CPU_CONTEXT.x86vicContext
		lea			edi, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT
		mov			ecx, SIZE x86_REGISTERS_CONTEXT
		rep			movsb

		// Restore the Fpu State.

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.x86friFpuRestoreInfs
		call		RestoreFPUState

		// Return to the Caller.

		ret
	}
}

//==================================================
// VpcICEDebugExceptionHandler Function Definition.
//==================================================

VOID __declspec( naked ) VpcICEDebugExceptionHandler( VOID )
{
	__asm
	{
		// Push the State and Setup the Environment.

		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		//- - - - >
		// ... ENTERING SYNCHRONIZED SECTION ...
		//- - - - >

		// Manage the Reentrancy of the code.

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslReentrancySpinLock
		lea			ecx, [ edx ]SYSINTERRUPTS_STATUS.bIsSendingIPIs
		call		EnterMultiProcessorSpinLockAndTestByte

		or			eax, eax
		jz			_CanEnterSafelyInTheDebugger

		//
		// Trasfer the control to the IPI Handler.
		//
		//  ## WARNING ##: This means that the Debug Exception (if it is a TRAP: i.e. Data or IO breakpoint)
		//					is actually EATEN, because the Execution resumes without the Debugger having
		//					Handled the Exception.
		//

		mov			dword ptr[ esp + MACRO_SIZEOF_PUSHED_VARS ], OFFSET Hooked_IpiISR
		jmp			_ExitFromISRWithRET_PLAIN_NOSPINLOCKLEAVE

_CanEnterSafelyInTheDebugger:

		// Single-Step Check.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		jne			_IsSingleStepping

		// B0-B3 bits Check.

		mov			eax, dr6
		and			eax, 0xF
		jz			_ExitFromISRWithRET_PLAIN

		test		eax, 0x8
		jz			_Test1
		test		g_dwDR7, (1<<7)
		jz			_ExitFromISRWithRET_PLAIN
		mov			[ edx ]SYSINTERRUPTS_STATUS.ulDebugRegHit, 3
		jmp			_HardwareBreakpointHit
_Test1:
		test		eax, 0x4
		jz			_Test2
		test		g_dwDR7, (1<<5)
		jz			_ExitFromISRWithRET_PLAIN
		mov			[ edx ]SYSINTERRUPTS_STATUS.ulDebugRegHit, 2
		jmp			_HardwareBreakpointHit
_Test2:
		test		eax, 0x2
		jz			_Test3
		test		g_dwDR7, (1<<3)
		jz			_ExitFromISRWithRET_PLAIN
		mov			[ edx ]SYSINTERRUPTS_STATUS.ulDebugRegHit, 1
		jmp			_HardwareBreakpointHit
_Test3:
		test		eax, 0x1
		jz			_Test4
		test		g_dwDR7, (1<<1)
		jz			_ExitFromISRWithRET_PLAIN
		mov			[ edx ]SYSINTERRUPTS_STATUS.ulDebugRegHit, 0
		jmp			_HardwareBreakpointHit
_Test4:
		jmp			_ExitFromISRWithRET_PLAIN

_HardwareBreakpointHit:

		mov			[ edx ]SYSINTERRUPTS_STATUS.bFromInt1, TRUE
		jmp			_EnterDebugger

_IsSingleStepping:

		// Check the Bit in the DR6 Register.

		mov			eax, dr6
		test		eax, (1<<14) // Single Step Bit of DR6.
		jz			_ExitFromISRWithRET_PLAIN

_EnterDebugger:

		// Entering Debugger...

		ENTERING_DEBUGGER

		// Save the State of the Registers.

		MACRO_INTHOOK_GET_REGS_STATE

		// Reset the "Single-Step" bit and the B0-B3 bits in the DR6 Register.

		mov			eax, dr6
		and			eax, ~( (1<<14) | 0xF ) // Single Step Bit + B0-B3 bits of DR6.
		mov			dr6, eax

		// Clear the TF Flag.

		mov			eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EFLAGS
		and			eax, ~(1<<8) /* Trap Flag. */
		mov			[ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EFLAGS, eax

		// Pop up the Debugger.

		cmp			[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE
		je			_Set_CopyToPrevContext_To_TRUE
		mov			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, FALSE
		jmp			_CopyToPrevContext_Is_OK
_Set_CopyToPrevContext_To_TRUE:
		mov			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, TRUE
_CopyToPrevContext_Is_OK:
		call		InvokeDebugger

		// Reflect the Changes made in the Debugger to the CPU State.

		MACRO_INTHOOK_SET_REGS_STATE(1)

		// Reset the "FromInt1" Status Byte.

		mov			[ edx ]SYSINTERRUPTS_STATUS.bFromInt1, FALSE

		// ISR Exit.

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslReentrancySpinLock
		call		LeaveMultiProcessorSpinLock

		//

		MACRO_INTHOOK_POP_REGISTERS_EPILOGUE(1)
		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".
		EXITING_DEBUGGER(1)
		iretd

_ExitFromISRWithRET_PLAIN:

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslReentrancySpinLock
		call		LeaveMultiProcessorSpinLock

_ExitFromISRWithRET_PLAIN_NOSPINLOCKLEAVE:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN
		ret
	}
}

//=======================================================
// VpcICEBreakpointExceptionHandler Function Definition.
//=======================================================

VOID __declspec( naked ) VpcICEBreakpointExceptionHandler( VOID )
{
	__asm
	{
		// Push the State and Setup the Environment.

		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		// Check out what form of the Instruction we have here.

		mov			eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EIP ]
		dec			eax
		cmp			byte ptr[ eax ], MACRO_BREAKPOINT_OPCODE
		jne			_ExitFromISRWithRET_PLAIN_NOSPINLOCKLEAVE

		//- - - - >
		// ... ENTERING SYNCHRONIZED SECTION ...
		//- - - - >

		// Manage the Reentrancy of the code.

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslReentrancySpinLock
		lea			ecx, [ edx ]SYSINTERRUPTS_STATUS.bIsSendingIPIs
		call		EnterMultiProcessorSpinLockAndTestByte

		or			eax, eax
		jz			_CanEnterSafelyInTheDebugger

		// Decrement the EIP pointer and Trasfer control to the IPI Handler.

		dec			dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EIP ]
		mov			dword ptr[ esp + MACRO_SIZEOF_PUSHED_VARS ], OFFSET Hooked_IpiISR

		jmp			_ExitFromISRWithRET_PLAIN_NOSPINLOCKLEAVE

_CanEnterSafelyInTheDebugger:

		// Entering Debugger...

		ENTERING_DEBUGGER

		// Save the State of the Registers.

		MACRO_INTHOOK_GET_REGS_STATE

		// Transfer the control to the Debugger.

		//

		mov			[ edx ]SYSINTERRUPTS_STATUS.bFromInt3, TRUE
		mov			eax, [ edx ]SYSINTERRUPTS_STATUS.CONTEXT.EIP
		dec			eax
		mov			[ edx ]SYSINTERRUPTS_STATUS.dwInt3OpcodeInstructionPtr, eax
		mov			[ edx ]SYSINTERRUPTS_STATUS.bTransferControlToOriginalInt3, FALSE

		//

		mov			[ edx ]SYSINTERRUPTS_STATUS.bINT3RequiresCodeWinEipDec, TRUE

		//

		mov			[ edx ]SYSINTERRUPTS_STATUS.bCopyToPrevContext, TRUE
		call		InvokeDebugger

		//

		mov			[ edx ]SYSINTERRUPTS_STATUS.bFromInt3, FALSE
		mov			[ edx ]SYSINTERRUPTS_STATUS.bINT3RequiresCodeWinEipDec, FALSE

		//

		// Reflect the Changes made in the Debugger to the CPU State.

		MACRO_INTHOOK_SET_REGS_STATE(1)

		// ISR Exit.

		mov			al, [ edx ]SYSINTERRUPTS_STATUS.bTransferControlToOriginalInt3
		mov			[ edx ]SYSINTERRUPTS_STATUS.bTransferControlToOriginalInt3, FALSE
		cmp			al, FALSE
		jne			_ExitFromISRWithRET_EXITINGDEB

		//

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslReentrancySpinLock
		call		LeaveMultiProcessorSpinLock

		MACRO_INTHOOK_POP_REGISTERS_EPILOGUE(1)
		lea			esp, [ esp + 4 ]	// For "push 0xFFFFFFFF".
		EXITING_DEBUGGER(1)

		iretd

		//

_ExitFromISRWithRET_EXITINGDEB:

		lea			ebx, [ edx ]SYSINTERRUPTS_STATUS.mpslReentrancySpinLock
		call		LeaveMultiProcessorSpinLock

		MACRO_INTHOOK_POP_REGISTERS_EPILOGUE(2)
		EXITING_DEBUGGER(2)

		ret

		//

_ExitFromISRWithRET_PLAIN_NOSPINLOCKLEAVE:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN
		ret

		//
	}
}

//================
// Detours Stuff.
//================

#define DT( n ) \
	static PVOID		g_pfnDetourOriginalFunction_ORD##n = NULL;														\
	DETOUR_TRAMPOLINE_GLOBVAR( VOID Trampoline_OriginalFunction_ORD##n( VOID ), g_pfnDetourOriginalFunction_ORD##n )

#define IT( n ) \
	{ & g_pfnDetourOriginalFunction_ORD##n, (VOID*) & Trampoline_OriginalFunction_ORD##n, (VOID*) & _Detours_GetVA_g_pfnDetourOriginalFunction_ORD##n  }

#include "masstram.inc"

static STATIC_TRAMPOLINE		g_vstStaticTrampolines[ MACRO_MAXNUM_OF_DETOURS ] =
	{
		IT( 00 ), IT( 01 ), IT( 02 ), IT( 03 ), IT( 04 ), IT( 05 ), IT( 06 ), IT( 07 ), IT( 08 ), IT( 09 ), IT( 0A ), IT( 0B ), IT( 0C ), IT( 0D ), IT( 0E ), IT( 0F ),
		IT( 10 ), IT( 11 ), IT( 12 ), IT( 13 ), IT( 14 ), IT( 15 ), IT( 16 ), IT( 17 ), IT( 18 ), IT( 19 ), IT( 1A ), IT( 1B ), IT( 1C ), IT( 1D ), IT( 1E ), IT( 1F ),
		IT( 20 ), IT( 21 ), IT( 22 ), IT( 23 ), IT( 24 ), IT( 25 ), IT( 26 ), IT( 27 ), IT( 28 ), IT( 29 ), IT( 2A ), IT( 2B ), IT( 2C ), IT( 2D ), IT( 2E ), IT( 2F ),
		IT( 30 ), IT( 31 ), IT( 32 ), IT( 33 ), IT( 34 ), IT( 35 ), IT( 36 ), IT( 37 ), IT( 38 ), IT( 39 ), IT( 3A ), IT( 3B ), IT( 3C ), IT( 3D ), IT( 3E ), IT( 3F ),
		IT( 40 ), IT( 41 ), IT( 42 ), IT( 43 ), IT( 44 ), IT( 45 ), IT( 46 ), IT( 47 ), IT( 48 ), IT( 49 ), IT( 4A ), IT( 4B ), IT( 4C ), IT( 4D ), IT( 4E ), IT( 4F ),
		IT( 50 ), IT( 51 ), IT( 52 ), IT( 53 ), IT( 54 ), IT( 55 ), IT( 56 ), IT( 57 ), IT( 58 ), IT( 59 ), IT( 5A ), IT( 5B ), IT( 5C ), IT( 5D ), IT( 5E ), IT( 5F ),
		IT( 60 ), IT( 61 ), IT( 62 ), IT( 63 ), IT( 64 ), IT( 65 ), IT( 66 ), IT( 67 ), IT( 68 ), IT( 69 ), IT( 6A ), IT( 6B ), IT( 6C ), IT( 6D ), IT( 6E ), IT( 6F ),
		IT( 70 ), IT( 71 ), IT( 72 ), IT( 73 ), IT( 74 ), IT( 75 ), IT( 76 ), IT( 77 ), IT( 78 ), IT( 79 ), IT( 7A ), IT( 7B ), IT( 7C ), IT( 7D ), IT( 7E ), IT( 7F ),
		IT( 80 ), IT( 81 ), IT( 82 ), IT( 83 ), IT( 84 ), IT( 85 ), IT( 86 ), IT( 87 ), IT( 88 ), IT( 89 ), IT( 8A ), IT( 8B ), IT( 8C ), IT( 8D ), IT( 8E ), IT( 8F ),
		IT( 90 ), IT( 91 ), IT( 92 ), IT( 93 ), IT( 94 ), IT( 95 ), IT( 96 ), IT( 97 ), IT( 98 ), IT( 99 ), IT( 9A ), IT( 9B ), IT( 9C ), IT( 9D ), IT( 9E ), IT( 9F ),
		IT( A0 ), IT( A1 ), IT( A2 ), IT( A3 ), IT( A4 ), IT( A5 ), IT( A6 ), IT( A7 ), IT( A8 ), IT( A9 ), IT( AA ), IT( AB ), IT( AC ), IT( AD ), IT( AE ), IT( AF ),
		IT( B0 ), IT( B1 ), IT( B2 ), IT( B3 ), IT( B4 ), IT( B5 ), IT( B6 ), IT( B7 ), IT( B8 ), IT( B9 ), IT( BA ), IT( BB ), IT( BC ), IT( BD ), IT( BE ), IT( BF ),
		IT( C0 ), IT( C1 ), IT( C2 ), IT( C3 ), IT( C4 ), IT( C5 ), IT( C6 ), IT( C7 ), IT( C8 ), IT( C9 ), IT( CA ), IT( CB ), IT( CC ), IT( CD ), IT( CE ), IT( CF ),
		IT( D0 ), IT( D1 ), IT( D2 ), IT( D3 ), IT( D4 ), IT( D5 ), IT( D6 ), IT( D7 ), IT( D8 ), IT( D9 ), IT( DA ), IT( DB ), IT( DC ), IT( DD ), IT( DE ), IT( DF ),
		IT( E0 ), IT( E1 ), IT( E2 ), IT( E3 ), IT( E4 ), IT( E5 ), IT( E6 ), IT( E7 ), IT( E8 ), IT( E9 ), IT( EA ), IT( EB ), IT( EC ), IT( ED ), IT( EE ), IT( EF ),
		IT( F0 ), IT( F1 ), IT( F2 ), IT( F3 ), IT( F4 ), IT( F5 ), IT( F6 ), IT( F7 ), IT( F8 ), IT( F9 ), IT( FA ), IT( FB ), IT( FC ), IT( FD ), IT( FE ), IT( FF )
	};

#undef DT
#undef IT

static STATIC_TRAMPOLINE		g_vstStaticTrampolines_USER[ MACRO_MAXNUM_OF_DETOURS ];

//======================================================
// GetKpebPtrFromProcNameCB Static Function Definition.
//======================================================

typedef struct _GETKPEBPTRFROMPROCNAMECB_INFOS // # ### NOTE - IMPORTANT ### #: This structure needs to be Zeroed before use !!!
{
	BYTE*			pbLastKpeb; // --> For enumerations with multiple results... <--

	CHAR*			pszProcName;
	BYTE*			pbKpeb;

	BOOLEAN			bIsHexNumCheckDone;
	BOOLEAN			bIsHexNum;
	DWORD			dwHexNum;

	BOOLEAN			bReturnNormalizedProcName;
	CHAR			szNormalizedProcName[ 0x10 + 1 ]; // ### This field size must be > 8 + 1 bytes (size of KPEB Hex String). ### // MACRO_KPEB_IMGFLNAME_FIELD_SIZE

} GETKPEBPTRFROMPROCNAMECB_INFOS, *PGETKPEBPTRFROMPROCNAMECB_INFOS;

static BOOLEAN GetKpebPtrFromProcNameCB( BYTE* pbKpeb, DWORD dwParam )
{
	GETKPEBPTRFROMPROCNAMECB_INFOS*		pInfos = (GETKPEBPTRFROMPROCNAMECB_INFOS*) dwParam;
	CHAR								szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
	BYTE*								pbProcName;
	CHAR*								pszDot;

	if ( pInfos->pbLastKpeb )
	{
		if ( pInfos->pbLastKpeb == pbKpeb )
			pInfos->pbLastKpeb = NULL;

		return TRUE;
	}

	// Check whether the Supplied Name is an Hex Number.

	if ( pInfos->bIsHexNumCheckDone == FALSE )
	{
		CHAR*		pszEndPtr;

		pInfos->dwHexNum = MACRO_CRTFN_NAME(strtoul)( pInfos->pszProcName, & pszEndPtr, 16 );
		if ( pInfos->dwHexNum != 0 && pInfos->dwHexNum != LONG_MAX && pInfos->dwHexNum != LONG_MIN &&
			pInfos->dwHexNum > (DWORD) g_pvMmUserProbeAddress &&
			( pszEndPtr == NULL || * pszEndPtr == 0 ) )
		{
			pInfos->bIsHexNum = TRUE;
		}
		else
		{
			pInfos->bIsHexNum = FALSE;
		}

		pInfos->bIsHexNumCheckDone = TRUE;
	}

	if ( pInfos->bIsHexNum )
	{
		if ( (DWORD) pbKpeb == pInfos->dwHexNum )
		{
			pInfos->pbKpeb = pbKpeb;

			if ( pInfos->bReturnNormalizedProcName )
				sprintf( pInfos->szNormalizedProcName, "0x%.8X", (DWORD) pbKpeb );

			return FALSE;
		}
	}

	// Get the Name of this Process.

	memset( szProcessName, 0, sizeof( szProcessName ) );
	pbProcName = pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB;
	memcpy( szProcessName, pbProcName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

	// Check whether there is a Match (with or without the Process File Name Extension).

	if ( MACRO_CRTFN_NAME(stricmp)( szProcessName, pInfos->pszProcName ) == 0 )
	{
		pInfos->pbKpeb = pbKpeb;

		if ( pInfos->bReturnNormalizedProcName )
		{
			strcpy( pInfos->szNormalizedProcName, szProcessName );
			pszDot = MACRO_CRTFN_NAME(strchr)( pInfos->szNormalizedProcName, '.' );
			if ( pszDot )
				* pszDot = 0;
		}

		return FALSE;
	}
	else
	{
		ULONG		ulNameFirstPartLength;

		pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
		if ( pszDot )
		{
			ulNameFirstPartLength = pszDot - szProcessName;
			if ( strlen( pInfos->pszProcName ) <= ulNameFirstPartLength &&
				MACRO_CRTFN_NAME(memicmp)( szProcessName, pInfos->pszProcName, ulNameFirstPartLength ) == 0 )
			{
				pInfos->pbKpeb = pbKpeb;

				if ( pInfos->bReturnNormalizedProcName )
				{
					strcpy( pInfos->szNormalizedProcName, szProcessName );
					pszDot = MACRO_CRTFN_NAME(strchr)( pInfos->szNormalizedProcName, '.' );
					if ( pszDot )
						* pszDot = 0;
				}

				return FALSE;
			}
		}
	}

	// Return.

	return TRUE;
}

//=================================================
// SearchForUserModule Static Function Definition.
//=================================================

typedef struct _SEARCHFORUSERMODULE_INFOS		SEARCHFORUSERMODULE_INFOS; // forward...
typedef BOOLEAN ( *PFNSEARCHFORUSERMODULECB )( IN SEARCHFORUSERMODULE_INFOS* psfumiInfos );

#define MACRO_NORMALIZEDMODULENAME_SIZE			128

typedef struct _SEARCHFORUSERMODULE_INFOS
{
	// Input.

	CHAR*						pszModuleName;
	BOOLEAN						bUseSEH;
	PVOID						pvPCRAddress;
	DWORD						dwRegionRelPtr;
	ULONG						ulRegionSize;

	PFNSEARCHFORUSERMODULECB	pfnModCallBack;

	BOOLEAN						bNormalizeModuleName;

	// Intermediate Input.

	BYTE*				pbCurrentCbKpeb;

	// Output.

	BYTE*				pbKpeb;
	BYTE*				pbModuleStart;
	ULONG				ulModuleLength;
	IMAGE_NT_HEADERS*	pinhModuleNtHdrs;
	BOOLEAN				bRegionIsMappedIn;
	BOOLEAN				bRegionIsInModule;

	CHAR				szNormalizedModuleName[ MACRO_NORMALIZEDMODULENAME_SIZE ];

} SEARCHFORUSERMODULE_INFOS, *PSEARCHFORUSERMODULE_INFOS;

static BOOLEAN SearchForUserModule_ModCB( IN CHAR* pszModName, IN VOID* pvStart, IN ULONG ulLength, IN IMAGE_NT_HEADERS* pinhNtHeaders, IN DWORD dwParam )
{
	SEARCHFORUSERMODULE_INFOS*		pInfos = (SEARCHFORUSERMODULE_INFOS*) dwParam;
	BOOLEAN							bMatch = FALSE;

	// Check if there is a Match.

	if ( MACRO_CRTFN_NAME(stricmp)( pszModName, pInfos->pszModuleName ) == 0 )
	{
		bMatch = TRUE;
	}
	else
	{
		CHAR*		pszImageNameToBeCompared;

		// Try with the Module Name, with File Extension...

		pszImageNameToBeCompared = & pszModName[ strlen( pszModName ) - 1 ];

		for( ; pszImageNameToBeCompared >= pszModName; pszImageNameToBeCompared -- )
			if ( * pszImageNameToBeCompared == '\\' ||
				* pszImageNameToBeCompared == '/' )
					break;

		pszImageNameToBeCompared ++;

		if ( MACRO_CRTFN_NAME(stricmp)( pszImageNameToBeCompared, pInfos->pszModuleName ) == 0 )
		{
			bMatch = TRUE;
		}
		else
		{
			// Try with the Module Name, without File Extension...

			pszImageNameToBeCompared = & pszModName[ strlen( pszModName ) - 1 ];

			for( ; pszImageNameToBeCompared >= pszModName; pszImageNameToBeCompared -- )
				if ( * pszImageNameToBeCompared == '.' )
					* pszImageNameToBeCompared = '\0';
				else if ( * pszImageNameToBeCompared == '\\' ||
					* pszImageNameToBeCompared == '/' )
						break;

			pszImageNameToBeCompared ++;

			if ( MACRO_CRTFN_NAME(stricmp)( pszImageNameToBeCompared, pInfos->pszModuleName ) == 0 )
			{
				bMatch = TRUE;
			}
		}
	}

	// If there is a Match, setup the Structure.

	if ( bMatch )
	{
		// Set up the Return Infos.

		pInfos->pbKpeb = pInfos->pbCurrentCbKpeb;
		pInfos->pbModuleStart = (BYTE*) pvStart;
		pInfos->ulModuleLength = ulLength;
		pInfos->pinhModuleNtHdrs = pinhNtHeaders;

		if ( pInfos->ulRegionSize == 0 ||
			pInfos->dwRegionRelPtr + pInfos->ulRegionSize > ulLength )
		{
			pInfos->bRegionIsMappedIn = FALSE;
			pInfos->bRegionIsInModule = FALSE;
		}
		else
		{
			pInfos->bRegionIsInModule = TRUE;
			pInfos->bRegionIsMappedIn =
				IsPagePresent_BYTERANGE( (BYTE*) pvStart + pInfos->dwRegionRelPtr, pInfos->ulRegionSize );
		}

		if ( pInfos->bNormalizeModuleName )
		{
			CHAR*		pszImageNameToBeCopied;

			pszImageNameToBeCopied = & pszModName[ strlen( pszModName ) - 1 ];
			for( ; pszImageNameToBeCopied >= pszModName; pszImageNameToBeCopied -- )
				if ( * pszImageNameToBeCopied == '\\' ||
					* pszImageNameToBeCopied == '/' )
						break;
			pszImageNameToBeCopied ++;

			if ( strlen( pszImageNameToBeCopied ) >= MACRO_NORMALIZEDMODULENAME_SIZE )
			{
				memcpy( pInfos->szNormalizedModuleName, pszImageNameToBeCopied, MACRO_NORMALIZEDMODULENAME_SIZE - 1 );
				pInfos->szNormalizedModuleName[ MACRO_NORMALIZEDMODULENAME_SIZE - 1 ] = 0;
			}
			else
			{
				strcpy( pInfos->szNormalizedModuleName, pszImageNameToBeCopied );
			}
		}

		// Return.

		return FALSE;
	}

	// Return.

	return TRUE;
}

static BOOLEAN SearchForUserModule( BYTE* pbKpeb, DWORD dwParam )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SEARCHFORUSERMODULE_INFOS*		pInfos = (SEARCHFORUSERMODULE_INFOS*) dwParam;
	DISCVBPTR_ADDPARAMS				dvbpapAddParams;
	DWORD							dwAttachPrCookie;

	memset( & dvbpapAddParams, 0, sizeof( dvbpapAddParams ) );

	// Initialize the Output Parameters.

	pInfos->pbKpeb = NULL;
	pInfos->pbModuleStart = NULL;
	pInfos->ulModuleLength = 0;
	pInfos->pinhModuleNtHdrs = NULL;

	// Do the Requested Search in the Module List.

	pInfos->pbCurrentCbKpeb = pbKpeb;

	dvbpapAddParams.bTouchPages = pInfos->bUseSEH;
	dvbpapAddParams.pbKpeb = pbKpeb;
	dvbpapAddParams.pfnUserModListCallBack = & SearchForUserModule_ModCB;
	dvbpapAddParams.dwUserModListCallBackPARAM = (DWORD) pInfos;
	dvbpapAddParams.bPreserveModuleFileExtension = TRUE;

	if ( pInfos->bUseSEH )
		KeAttachProcess( (PEPROCESS) pbKpeb );
	else
		dwAttachPrCookie = VpcICEAttachProcess( pbKpeb );

	DiscoverBytePointerPosInModules( NULL, NULL,
		extension->pvNtoskrnlDriverSection, g_pvMmUserProbeAddress,
		pInfos->pvPCRAddress,
		& dvbpapAddParams );

	if ( pInfos->bUseSEH )
		KeDetachProcess();
	else
		VpcICEDetachProcess( dwAttachPrCookie );

	if ( pInfos->pbModuleStart )
	{
		if ( pInfos->pfnModCallBack )
			return pInfos->pfnModCallBack( pInfos );
		else if ( pInfos->bRegionIsMappedIn )
			return FALSE;
	}

	// Return.

	return TRUE;
}

//====================================
// InstallDetour Function Definition.
//====================================

BOOLEAN InstallDetour( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulType, IN DWORD dwAddress, IN VOID* pvNewFunc, IN BOOLEAN bUseSEH, IN CHAR* pszProcessName, IN CHAR* pszModuleName )
{
	INSTALLDETOURORBREAKPOINT_PARAMS		idobParams;

	// Call the Implementation Function and Return.

	memset( & idobParams, 0, sizeof( idobParams ) );

	idobParams.ulObjectType = MACRO_IDOB_OBJECTTYPE_DETOUR;

	idobParams.psisSysInterrStatus = psisSysInterrStatus;
	idobParams.ulType = ulType;
	idobParams.dwAddress = dwAddress;
	idobParams.bUseSEH = bUseSEH;
	idobParams.pszProcessName = pszProcessName;
	idobParams.pszModuleName = pszModuleName;

	idobParams.pvNewFunc = pvNewFunc;

	return InstallDetourOrBreakpoint( & idobParams );
}

//======================================
// GetDetourByType Function Definition.
//======================================

DETOUR*	GetDetourByType( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulType )
{
	ULONG		ulI;

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
		if ( psisSysInterrStatus->vdDetours[ ulI ].ulType == ulType )
			return & psisSysInterrStatus->vdDetours[ ulI ];

	return NULL;
}

//======================================================
// InitEnableXXXCache Function Definition + Structures.
//======================================================

//

typedef struct _EXXXC_PROCESS
{
	CHAR		szProcessName[ MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE ];
	BYTE*		pbKpeb;

	BOOLEAN		bAllProcessesWithThisNameInTheCache;

} EXXXC_PROCESS, *PEXXXC_PROCESS;

typedef struct _EXXXC_MODULE
{
	CHAR			szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE ];
	BYTE*			pbModuleStart;
	EXXXC_PROCESS*	pexcpProcess;

} EXXXC_MODULE, *PEXXXC_MODULE;

//

#define MACRO_EXXXC_PROCESS_LIST_MAXNUM			0x40
#define MACRO_EXXXC_MODULE_LIST_MAXNUM			0x500

//

static EXXXC_PROCESS			g_vexcpProcessList[ MACRO_EXXXC_PROCESS_LIST_MAXNUM ];
static ULONG					g_ulProcessListItemsNum;

static EXXXC_MODULE				g_vexcmModuleList[ MACRO_EXXXC_MODULE_LIST_MAXNUM ];
static ULONG					g_ulModuleListItemsNum;

//

VOID InitEnableXXXCache ()
{
	// Init the Structures.

	g_ulProcessListItemsNum = 0;
	g_ulModuleListItemsNum = 0;

	// Return.

	return;
}

//==================================================
// EnableDetoursAndBreakpoints Function Definition.
//==================================================

static VOID EnableSingleDetourIMPL( IN DETOUR* pdThis, IN BOOLEAN bEnable, IN DWORD dwAddress )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	BYTE*							pbPrev;
	BYTE*							pbAfter;

	// Enable eventually the Detour, if the Target Memory is successfully verified.

	if ( IsPagePresent_BYTERANGE( (BYTE*) dwAddress, pdThis->ulDetourSize ) )
	{
		if ( bEnable )
		{
			pbPrev = pdThis->vbPrevCodeBytes;
			pbAfter = pdThis->vbJumpCodeBytes;
		}
		else
		{
			pbPrev = pdThis->vbJumpCodeBytes;
			pbAfter = pdThis->vbPrevCodeBytes;
		}

		if ( memcmp( (VOID*) dwAddress, pbPrev, pdThis->ulDetourSize ) == 0 )
		{
			DWORD		dwCookie;

			if ( pdThis->ulType == MACRO_DETOURTYPE_USER )
			{
				if ( bEnable )
					AddFrameToPFMDB( (VOID*) dwAddress, pdThis->ulDetourSize );
				else
					RemoveFrameFromPFMDB( (VOID*) dwAddress, pdThis->ulDetourSize );
			}

			dwCookie = EnableCopyOnWrite( FALSE, 0 );
			memcpy( (VOID*) dwAddress, pbAfter, pdThis->ulDetourSize );
			EnableCopyOnWrite( TRUE, dwCookie );
		}
	}

	// Check whether we are required to determine if this is the Hit Detour.

	if ( bEnable == FALSE &&
		psisSysInterrStatus->bDiscoverWhichObjectWasHit )
	{
		NTSTATUS		ntStatus;
		DWORD			dwPhysAddress;

		ntStatus = LinearAddressToPhysAddress( & dwPhysAddress, dwAddress );

		if ( ntStatus == STATUS_SUCCESS &&
			dwPhysAddress == psisSysInterrStatus->dwBreakpointOpcodePhysAddr ) // <--- Consider that User Mode Detours are Breakpoints !!!
		{
			// Add the Detour to the Collection.

			if ( psisSysInterrStatus->ulDiscoveredDetoursNum < MACRO_MAXNUM_OF_DISCOVERED_DTS )
				psisSysInterrStatus->vpdDiscoveredDetours[ psisSysInterrStatus->ulDiscoveredDetoursNum ++ ] = pdThis;
		}
	}

	// Return to the Caller.

	return;
}

	//
	// NOTE: The Processor Parameter is meaningful only in the case of a INTERRUPT Breakpoint. Otherwise, it MUST be passed as 0.
	//
static BOOLEAN EnableSingleBreakpointIMPL( IN BREAKPOINT* pbThis, IN BOOLEAN bEnable, IN DWORD dwAddress, IN DWORD dwProcessor )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	ULONG							ulI;

	if ( pbThis->bIsUsingDebugRegisters == FALSE )
	{
		// Enable the Breakpoint, if the Memory is verified.

		if ( IsPagePresent_BYTERANGE( (BYTE*) dwAddress, MACRO_BREAKPOINT_SIZE ) )
		{
			DWORD		dwCookie;

			if ( bEnable )
			{
				if ( * (BYTE*) dwAddress != pbThis->vbPrevCodeBytes[ dwProcessor ] )
					return FALSE;

				if (
					pbThis->ulType == MACRO_BREAKPOINTTYPE_EXEC ||
					pbThis->ulType == MACRO_BREAKPOINTTYPE_INTERRUPT )
				{
					AddFrameToPFMDB( (VOID*) dwAddress, MACRO_BREAKPOINT_SIZE );
				}
			}
			else
			{
				if ( * (BYTE*) dwAddress != MACRO_BREAKPOINT_OPCODE )
					return FALSE;

				if (
					pbThis->ulType == MACRO_BREAKPOINTTYPE_EXEC ||
					pbThis->ulType == MACRO_BREAKPOINTTYPE_INTERRUPT )
				{
					RemoveFrameFromPFMDB( (VOID*) dwAddress, MACRO_BREAKPOINT_SIZE );
				}
			}

			dwCookie = EnableCopyOnWrite( FALSE, 0 );

			if ( bEnable )
			{
				if ( * (BYTE*) dwAddress == pbThis->vbPrevCodeBytes[ dwProcessor ] )
					* (BYTE*) dwAddress = MACRO_BREAKPOINT_OPCODE;
			}
			else
			{
				if ( * (BYTE*) dwAddress == MACRO_BREAKPOINT_OPCODE )
					* (BYTE*) dwAddress = pbThis->vbPrevCodeBytes[ dwProcessor ];
			}

			EnableCopyOnWrite( TRUE, dwCookie );
		}
	}

	// Check whether we are required to determine if this is the Hit Breakpoint.

	if ( bEnable == FALSE &&
		psisSysInterrStatus->bDiscoverWhichObjectWasHit )
	{
		NTSTATUS		ntStatus;
		DWORD			dwPhysAddress;
		BOOLEAN			bAdd = FALSE;

		if ( pbThis->bIsUsingDebugRegisters == FALSE )
		{
			ntStatus = LinearAddressToPhysAddress( & dwPhysAddress, dwAddress );

			if ( ntStatus == STATUS_SUCCESS &&
				dwPhysAddress == psisSysInterrStatus->dwBreakpointOpcodePhysAddr )
			{
				// Add the Breakpoint to the Collection.

				bAdd = TRUE;
			}
		}
		else
		{
			if ( psisSysInterrStatus->bFromInt1 &&
				pbThis->ulDebugRegisterNum == psisSysInterrStatus->ulDebugRegHit )
			{
				// Add the Breakpoint to the Collection.

				bAdd = TRUE;
			}
		}

		// Check if Adding the Breakpoint is Required.

		if ( bAdd )
		{
			if ( psisSysInterrStatus->ulDiscoveredBreakpointsNum < MACRO_MAXNUM_OF_DISCOVERED_BPS )
			{
				for ( ulI = 0; ulI < psisSysInterrStatus->ulDiscoveredBreakpointsNum; ulI ++ )
					if ( psisSysInterrStatus->vpbpDiscoveredBreakpoints[ ulI ] == pbThis )
						break;

				if ( ulI == psisSysInterrStatus->ulDiscoveredBreakpointsNum )
					psisSysInterrStatus->vpbpDiscoveredBreakpoints[ psisSysInterrStatus->ulDiscoveredBreakpointsNum ++ ] = pbThis;
			}
		}
	}

	// Return to the Caller.

	return TRUE;
}

static BOOLEAN SearchForUserModulePrivCb( IN SEARCHFORUSERMODULE_INFOS* psfumiInfos )
{
	EXXXC_PROCESS*			pexcpProcess = NULL;
	ULONG					ulI;
	CHAR*					pszProcessName;
	EXXXC_MODULE*			pexcmModule;
	CHAR*					pszDot;

	// === Add the Item to the Cache, if not already present. ===

	// Search for the Process.

	for ( ulI = 0; ulI < g_ulProcessListItemsNum; ulI ++ )
	{
		if ( g_vexcpProcessList[ ulI ].pbKpeb == psfumiInfos->pbKpeb )
		{
			pexcpProcess = & g_vexcpProcessList[ ulI ];
			break;
		}
	}

	// Add the Process, if not already present.

	if ( pexcpProcess == NULL )
	{
		if ( psfumiInfos->pbKpeb &&
			g_ulProcessListItemsNum < MACRO_EXXXC_PROCESS_LIST_MAXNUM )
		{
			pszProcessName = (CHAR*) ( psfumiInfos->pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );

			pexcpProcess = & g_vexcpProcessList[ g_ulProcessListItemsNum ++ ];
			memset( pexcpProcess->szProcessName, 0, MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE );
			memcpy( pexcpProcess->szProcessName, pszProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

			pszDot = MACRO_CRTFN_NAME(strchr)( pexcpProcess->szProcessName, '.' );
			if ( pszDot )
				* pszDot = 0;

			pexcpProcess->pbKpeb = psfumiInfos->pbKpeb;
			pexcpProcess->bAllProcessesWithThisNameInTheCache = FALSE;
		}
		else
		{
			return FALSE;
		}
	}

	// Search for the Module.

	if ( g_ulModuleListItemsNum >= MACRO_EXXXC_MODULE_LIST_MAXNUM )
		return FALSE;

	for ( ulI = 0; ulI < g_ulModuleListItemsNum; ulI ++ )
	{
		if ( g_vexcmModuleList[ ulI ].pexcpProcess == pexcpProcess &&
			g_vexcmModuleList[ ulI ].pbModuleStart == psfumiInfos->pbModuleStart )
		{
			return TRUE;
		}
	}

	// Add the Module.

	pexcmModule = & g_vexcmModuleList[ g_ulModuleListItemsNum ++ ];

	pexcmModule->pbModuleStart = psfumiInfos->pbModuleStart;
	pexcmModule->pexcpProcess = pexcpProcess;

	if ( strlen( psfumiInfos->szNormalizedModuleName ) >= MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE )
	{
		memcpy( pexcmModule->szModuleName, psfumiInfos->szNormalizedModuleName, MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 );
		pexcmModule->szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 ] = 0;
	}
	else
	{
		strcpy( pexcmModule->szModuleName, psfumiInfos->szNormalizedModuleName );
	}

	// === Return to the Caller. ===

	// ...make the search to Continue.

	return TRUE;
}

typedef struct _ENABLEDETOURSANDBREAKPOINTSIMPL_PARAMS // ### RESET TO ZERO BEFORE PASSING IT... !!! ###
{
	// Input

	SYSINTERRUPTS_STATUS*			psisSysInterrStatus;
	BOOLEAN							bEnable;
	ISCONTEXTCOMPATIBLE_CACHEINFOS*	piccciCacheInfos;
	BYTE**							ppbCurrentKPEB;

	// Common Pointers.

	BOOLEAN*						pbIsContextCompatible;
	CONTEXT_INFO*					pciContext;
	DWORD*							pdwAddress;
	BOOLEAN*						pbInstalled;

	// Detour Specific.

	DETOUR*							pdThis;

	// Breakpoint Specific.

	BREAKPOINT*						pbThis;

} ENABLEDETOURSANDBREAKPOINTSIMPL_PARAMS, *PENABLEDETOURSANDBREAKPOINTSIMPL_PARAMS;

#define MACRO_EDABI_KPEBSLIST_MAXSIZE			0x30

static VOID EnableDetoursAndBreakpointsIMPL( IN ENABLEDETOURSANDBREAKPOINTSIMPL_PARAMS* pedabipParams )
{
	PDEVICE_EXTENSION						extension = g_pDeviceObject->DeviceExtension;
	BYTE*									pbProcHexForm = NULL;

	// Check whether the Process Name is in the Hex Form.

	if ( * pedabipParams->pdwAddress < (DWORD) g_pvMmUserProbeAddress &&
		strlen( pedabipParams->pciContext->szProcessName ) )
	{
		if ( strlen( pedabipParams->pciContext->szProcessName ) == MACRO_CONTEXTINFO_PROCESSNAME_HEXFORM_SIZE &&
			pedabipParams->pciContext->szProcessName[ 0 ] == '0' &&
			pedabipParams->pciContext->szProcessName[ 1 ] == 'x' )
		{
			CHAR*		pszEndPtr;
			DWORD		dwKpebHexForm = MACRO_CRTFN_NAME(strtoul)(
				pedabipParams->pciContext->szProcessName, & pszEndPtr, 16 );
			if ( dwKpebHexForm > (DWORD) g_pvMmUserProbeAddress &&
				( pszEndPtr == NULL || * pszEndPtr == 0 ) )
			{
				pbProcHexForm = (BYTE*) dwKpebHexForm;
			}
		}
	}

	// Take care of the "IsContextCompatible" flag.

	if ( pedabipParams->bEnable == FALSE )
	{
		// Set the state of the "IsContextCompatible" flag.

		* pedabipParams->pbIsContextCompatible = IsContextCompatible(
			pedabipParams->pciContext, pedabipParams->pdwAddress,
			NULL, pedabipParams->psisSysInterrStatus->vx86ccProcessors[ pedabipParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase,
			pedabipParams->piccciCacheInfos, NULL, 0 );
	}

	// Check whether the State of the Detour/Breakpoint has to be updated.

	if ( * pedabipParams->pbInstalled != pedabipParams->bEnable )
	{
		// Check whether the Detour/Breakpoint is in User Mode or in Kernel Mode.

		if ( * pedabipParams->pdwAddress < (DWORD) g_pvMmUserProbeAddress ) // User mode.
		{
			BYTE*							vpbKpebs[ MACRO_EDABI_KPEBSLIST_MAXSIZE ];
			ULONG							ulKpebsPos = 0;
			ULONG							ulJ, ulK;
			BOOLEAN							bCanContinue = TRUE;
			GETKPEBPTRFROMPROCNAMECB_INFOS	ckpiCbInfos;
			BOOLEAN							bAllProcessesWithThisNameInTheCache = FALSE;
			BOOLEAN							bSearchInSysProcList = FALSE;
			BYTE*							pbLastKpeb = NULL;

			if ( strlen( pedabipParams->pciContext->szProcessName ) ) // A process in particular.
			{
				// Search for the KPEB in the Cache.

				for ( ulJ = 0; ulJ < g_ulProcessListItemsNum; ulJ ++ )
				{
					if ( ( pbProcHexForm && pbProcHexForm == g_vexcpProcessList[ ulJ ].pbKpeb ) ||
						MACRO_CRTFN_NAME(stricmp)(
						pedabipParams->pciContext->szProcessName, g_vexcpProcessList[ ulJ ].szProcessName ) == 0 )
					{
						if ( ulKpebsPos < MACRO_EDABI_KPEBSLIST_MAXSIZE )
							vpbKpebs[ ulKpebsPos ++ ] = g_vexcpProcessList[ ulJ ].pbKpeb;

						if ( g_vexcpProcessList[ ulJ ].bAllProcessesWithThisNameInTheCache )
							bAllProcessesWithThisNameInTheCache = TRUE;
					}
				}

				// Check whether searching in the System List is necessary...

				if ( bAllProcessesWithThisNameInTheCache == FALSE )
				{
					if ( ( ulKpebsPos && pbProcHexForm ) == FALSE )
						bSearchInSysProcList = TRUE;
				}

				// Search for the KPEB in the System Structures, if necessary.

				if ( bSearchInSysProcList )
				{
					ulKpebsPos = 0;

					while( TRUE )
					{
						memset( & ckpiCbInfos, 0, sizeof( ckpiCbInfos ) );
						ckpiCbInfos.pszProcName = pedabipParams->pciContext->szProcessName;
						ckpiCbInfos.pbKpeb = NULL;
						ckpiCbInfos.pbLastKpeb = pbLastKpeb;

						IterateThroughListOfProcesses( & extension->pliProcessListInfo,
							& GetKpebPtrFromProcNameCB, (DWORD) & ckpiCbInfos );

						if ( ckpiCbInfos.pbKpeb )
						{
							// Add to the Collection.

							if ( ulKpebsPos < MACRO_EDABI_KPEBSLIST_MAXSIZE )
								vpbKpebs[ ulKpebsPos ++ ] = ckpiCbInfos.pbKpeb;

							// Cache Management.

							for ( ulJ = 0; ulJ < g_ulProcessListItemsNum; ulJ ++ )
								if ( g_vexcpProcessList[ ulJ ].pbKpeb == ckpiCbInfos.pbKpeb )
								{
									g_vexcpProcessList[ ulJ ].bAllProcessesWithThisNameInTheCache = TRUE;
									break;
								}

							if ( ulJ == g_ulProcessListItemsNum )
							{
								CHAR*					pszProcessName;
								EXXXC_PROCESS*			pexcpProcess;
								CHAR*					pszDot;

								// Add the Item to the Cache.

								if ( g_ulProcessListItemsNum < MACRO_EXXXC_PROCESS_LIST_MAXNUM )
								{
									pszProcessName = (CHAR*) ( ckpiCbInfos.pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );

									pexcpProcess = & g_vexcpProcessList[ g_ulProcessListItemsNum ++ ];
									memset( pexcpProcess->szProcessName, 0, MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE );
									memcpy( pexcpProcess->szProcessName, pszProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

									pszDot = MACRO_CRTFN_NAME(strchr)( pexcpProcess->szProcessName, '.' );
									if ( pszDot )
										* pszDot = 0;

									pexcpProcess->pbKpeb = ckpiCbInfos.pbKpeb;
									pexcpProcess->bAllProcessesWithThisNameInTheCache = TRUE;
								}
							}

							// Last KPEB for the Next Iteration.

							pbLastKpeb = ckpiCbInfos.pbKpeb;
						}
						else
						{
							break;
						}
					}
				}

				// Check whether Continuing is possible.

				if ( ulKpebsPos == 0 )
					bCanContinue = FALSE;
			}

			if ( bCanContinue )
			{
				// Is it in a Module in particular ?

				if ( strlen( pedabipParams->pciContext->szModuleName ) ) // A module in particular.
				{
					BOOLEAN							bIsInCache = FALSE;
					SEARCHFORUSERMODULE_INFOS		sfumiCbInfos;
					BYTE*							pbRequiredKPEB;

					// Search for the Module in the Cache, checking whether it is present.

					for ( ulJ = 0; ulJ < g_ulModuleListItemsNum; ulJ ++ )
					{
						if ( MACRO_CRTFN_NAME(stricmp)(
							pedabipParams->pciContext->szModuleName, g_vexcmModuleList[ ulJ ].szModuleName ) == 0 )
						{
							bIsInCache = TRUE;
							break;
						}
					}

					// If not in the Cache, search for it in the System Structures.

					if ( bIsInCache == FALSE )
					{
						sfumiCbInfos.pszModuleName = pedabipParams->pciContext->szModuleName;
						sfumiCbInfos.pbKpeb = NULL;
						sfumiCbInfos.pbModuleStart = NULL;
						sfumiCbInfos.bUseSEH = FALSE;
						sfumiCbInfos.pvPCRAddress =
							pedabipParams->psisSysInterrStatus->vx86ccProcessors[ pedabipParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase;
						sfumiCbInfos.ulModuleLength = 0;
						sfumiCbInfos.pinhModuleNtHdrs = NULL;
						sfumiCbInfos.bRegionIsMappedIn = FALSE;
						sfumiCbInfos.bRegionIsInModule = FALSE;
						sfumiCbInfos.dwRegionRelPtr = 0;
						sfumiCbInfos.ulRegionSize = 0;
						sfumiCbInfos.pbCurrentCbKpeb = NULL;
						sfumiCbInfos.pfnModCallBack = & SearchForUserModulePrivCb;
						sfumiCbInfos.bNormalizeModuleName = TRUE;

						IterateThroughListOfProcesses( & extension->pliProcessListInfo,
							& SearchForUserModule, (DWORD) & sfumiCbInfos );
					}

					// Search for the Module in the Cache.

					for ( ulJ = 0; ulJ < g_ulModuleListItemsNum; ulJ ++ )
					{
						if ( MACRO_CRTFN_NAME(stricmp)(
							pedabipParams->pciContext->szModuleName, g_vexcmModuleList[ ulJ ].szModuleName ) == 0 )
						{
							// Consider only the Modules in the Selected Process.

							if ( ulKpebsPos )
							{
								BOOLEAN		bKpebRefFound = FALSE;
								BYTE*		pbModKpeb = g_vexcmModuleList[ ulJ ].pexcpProcess->pbKpeb;

								for ( ulK = 0; ulK < ulKpebsPos; ulK ++ )
									if ( vpbKpebs[ ulK ] == pbModKpeb )
									{
										bKpebRefFound = TRUE;
										break;
									}

								if ( bKpebRefFound == FALSE )
									continue;
							}

							// === Enable the Detour/Breakpoint. ===

							// What KPEB ?

							pbRequiredKPEB = g_vexcmModuleList[ ulJ ].pexcpProcess->pbKpeb;

							// Modify the Address Context if necessary.

							if ( pbRequiredKPEB != * pedabipParams->ppbCurrentKPEB )
							{
								// Modify the Address Context.

								* pedabipParams->ppbCurrentKPEB = pbRequiredKPEB;

								VpcICEAttachProcess(
									* pedabipParams->ppbCurrentKPEB );
							}

							// We can finally enable the Detour/Breakpoint.

							* pedabipParams->pdwAddress =
								(DWORD) g_vexcmModuleList[ ulJ ].pbModuleStart + pedabipParams->pciContext->dwModuleOffset;

							if ( pedabipParams->pdThis )
								EnableSingleDetourIMPL( pedabipParams->pdThis, pedabipParams->bEnable, pedabipParams->pdThis->dwAddress );
							else
								EnableSingleBreakpointIMPL( pedabipParams->pbThis, pedabipParams->bEnable, pedabipParams->pbThis->dwAddress, 0 );
						}
					}
				}
				else // In all modules.
				{
					if ( ulKpebsPos == 0 )
					{
						//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
						// Any process, Any module: This case is not realistic for a Detour/Breakpoint.
						// If execution comes here, there is a BUG in the code that sets the Detours/Breakpoints...
						//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
					}
					else
					{
						// Iterate through all the Kpebs.

						for ( ulK = 0; ulK < ulKpebsPos; ulK ++ )
						{
							// Modify the Address Context if necessary.

							if ( vpbKpebs[ ulK ] != * pedabipParams->ppbCurrentKPEB )
							{
								// Modify the Address Context.

								* pedabipParams->ppbCurrentKPEB = vpbKpebs[ ulK ];

								VpcICEAttachProcess(
									* pedabipParams->ppbCurrentKPEB );
							}

							// Enable the Detour/Breakpoint.

							if ( pedabipParams->pdThis )
								EnableSingleDetourIMPL( pedabipParams->pdThis, pedabipParams->bEnable, pedabipParams->pdThis->dwAddress );
							else
								EnableSingleBreakpointIMPL( pedabipParams->pbThis, pedabipParams->bEnable, pedabipParams->pbThis->dwAddress, 0 );
						}
					}
				}
			}
		}
		else // Kernel mode.
		{
			// Enable the Detour/Breakpoint.

			if ( pedabipParams->pdThis )
				EnableSingleDetourIMPL( pedabipParams->pdThis, pedabipParams->bEnable, pedabipParams->pdThis->dwAddress );
			else
				EnableSingleBreakpointIMPL( pedabipParams->pbThis, pedabipParams->bEnable, pedabipParams->pbThis->dwAddress, 0 );
		}

		// Remember whether the Detour/Breakpoint was applied.

		* pedabipParams->pbInstalled = pedabipParams->bEnable;
	}

	// Return.

	return;
}

VOID EnableDetoursAndBreakpoints( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN BOOLEAN bEnable )
{
	PDEVICE_EXTENSION						extension = g_pDeviceObject->DeviceExtension;
	ULONG									ulI;
	DETOUR*									pdThis;
	BREAKPOINT*								pbThis;
	ISCONTEXTCOMPATIBLE_CACHEINFOS			iccciCacheInfos;
	BYTE*									pbStartingKPEB;
	BYTE*									pbCurrentKPEB;
	ENABLEDETOURSANDBREAKPOINTSIMPL_PARAMS	edabipParams;
	NTSTATUS								ntStatus;
	BOOLEAN									bPFMDBCheckOutRes = FALSE;

	psisSysInterrStatus->bTimerThreadEventOccurred = FALSE;
	psisSysInterrStatus->bUser2KernelGateBkpOccurred = FALSE;
	psisSysInterrStatus->bResumeExecBecauseOfStaleBpInPf = FALSE;

	memset( & iccciCacheInfos, 0, sizeof( iccciCacheInfos ) );

	// We have to determine which Detour/Breakpoint was hit.

	psisSysInterrStatus->dwBreakpointOpcodePhysAddr = 0;
	psisSysInterrStatus->ulDiscoveredBreakpointsNum = 0;
	psisSysInterrStatus->ulDiscoveredDetoursNum = 0;

	if ( bEnable == FALSE )
	{
		if ( psisSysInterrStatus->bFromInt3 )
		{
			ntStatus = LinearAddressToPhysAddress(
				& psisSysInterrStatus->dwBreakpointOpcodePhysAddr,
				psisSysInterrStatus->dwInt3OpcodeInstructionPtr );

			if ( ntStatus == STATUS_SUCCESS )
				psisSysInterrStatus->bDiscoverWhichObjectWasHit = TRUE;
		}
		else if ( psisSysInterrStatus->bFromInt1 )
		{
			for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
			{
				pbThis = & psisSysInterrStatus->vbpBreakpoints[ ulI ];

				if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_UNUSED &&
					pbThis->bDisabled == FALSE &&
					pbThis->bIsUsingDebugRegisters && pbThis->ulDebugRegisterNum == psisSysInterrStatus->ulDebugRegHit )
				{
					//
					// # WARNING: This code HAS to be Called if we are Arriving from an INT 1. #
					//

					if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_EXEC )
					{
						psisSysInterrStatus->bDiscoverWhichObjectWasHit = TRUE;
					}

					psisSysInterrStatus->vpbpDiscoveredBreakpoints[ psisSysInterrStatus->ulDiscoveredBreakpointsNum ++ ] =
						pbThis;

					break;
				}
			}
		}
	}

	// Take care of the KPEB Variables.

	pbStartingKPEB =
		pbCurrentKPEB =
			(BYTE*) GetCurrentKPEB(
				psisSysInterrStatus->vx86ccProcessors[ psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase );

	//
	// If Entering in the Debugger, Disable NOW the INTERRUPT Breakpoints.
	//  Iterate through all the Breakpoints of type INTERRUPT.
	//

	if ( bEnable == FALSE )
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & psisSysInterrStatus->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_INTERRUPT &&
				pbThis->bDisabled == FALSE )
			{
				EnableInterruptBreakpoint( pbThis, FALSE );
			}
		}
	}

	// Iterate through all the Breakpoints of type EXEC.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
	{
		pbThis = & psisSysInterrStatus->vbpBreakpoints[ ulI ];

		// Check whether the Slot is in use.

		if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_EXEC &&
			pbThis->bDisabled == FALSE )
		{
			// Call the Implementation Function.

			memset( & edabipParams, 0, sizeof( edabipParams ) );

			edabipParams.psisSysInterrStatus = psisSysInterrStatus;
			edabipParams.bEnable = bEnable;
			edabipParams.piccciCacheInfos = & iccciCacheInfos;
			edabipParams.ppbCurrentKPEB = & pbCurrentKPEB;
			edabipParams.pbIsContextCompatible = & pbThis->bIsContextCompatible;
			edabipParams.pciContext = & pbThis->ciContext;
			edabipParams.pdwAddress = & pbThis->dwAddress;
			edabipParams.pbInstalled = & pbThis->bInstalled;
			edabipParams.pbThis = pbThis;

			EnableDetoursAndBreakpointsIMPL( & edabipParams );
		}
	}

	// Iterate through all the Detours.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
	{
		pdThis = & psisSysInterrStatus->vdDetours[ ulI ];

		// Check whether the Slot is in use.

		if ( pdThis->ulType != MACRO_DETOURTYPE_UNUSED )
		{
			// Call the Implementation Function.

			memset( & edabipParams, 0, sizeof( edabipParams ) );

			edabipParams.psisSysInterrStatus = psisSysInterrStatus;
			edabipParams.bEnable = bEnable;
			edabipParams.piccciCacheInfos = & iccciCacheInfos;
			edabipParams.ppbCurrentKPEB = & pbCurrentKPEB;
			edabipParams.pbIsContextCompatible = & pdThis->bIsContextCompatible;
			edabipParams.pciContext = & pdThis->ciContext;
			edabipParams.pdwAddress = & pdThis->dwAddress;
			edabipParams.pbInstalled = & pdThis->bInstalled;
			edabipParams.pdThis = pdThis;

			EnableDetoursAndBreakpointsIMPL( & edabipParams );
		}
	}

	//
	// If Exiting from the Debugger, Enable NOW the INTERRUPT Breakpoints.
	//  Iterate through all the Breakpoints of type INTERRUPT.
	//

	if ( bEnable )
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & psisSysInterrStatus->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_INTERRUPT &&
				pbThis->bDisabled == FALSE )
			{
				EnableInterruptBreakpoint( pbThis, TRUE );
			}
		}
	}

	// Make sure that the KPEB state is Consistent.

	if ( pbStartingKPEB != pbCurrentKPEB )
	{
		// Original KPEB restore.

		VpcICEAttachProcess(
			pbStartingKPEB );
	}

	// Take care of the Page Frame Mod DB, if we are Entering in the Debugger.

	if ( bEnable == FALSE )
	{
		bPFMDBCheckOutRes = CheckOutFramesInPFMDB(
			psisSysInterrStatus->bDiscoverWhichObjectWasHit ? psisSysInterrStatus->dwBreakpointOpcodePhysAddr : 0 );
	}

	// Reset the Variable(s).

	psisSysInterrStatus->bDiscoverWhichObjectWasHit = FALSE;

	// Check if we need to Decrement the CPU EIP.

	if ( bEnable == FALSE )
	{
		if ( psisSysInterrStatus->bFromInt1 )
		{
			if ( psisSysInterrStatus->ulDiscoveredBreakpointsNum &&
				psisSysInterrStatus->vpbpDiscoveredBreakpoints[ 0 ]->bIsUsingDebugRegisters )
			{
				//
				// We have to Check whether a Trap or a Fault occurred.
				//

				if ( psisSysInterrStatus->vpbpDiscoveredBreakpoints[ 0 ]->ulType == MACRO_BREAKPOINTTYPE_IO ||
					psisSysInterrStatus->vpbpDiscoveredBreakpoints[ 0 ]->ulType == MACRO_BREAKPOINTTYPE_MEMORY )
				{
					// Decrement the Position in the Code Window.

					psisSysInterrStatus->bINT1RequiresCodeWinPosDec = TRUE;
				}
			}
		}
		else if ( psisSysInterrStatus->bFromInt3 )
		{
			if ( psisSysInterrStatus->ulDiscoveredBreakpointsNum ||
				psisSysInterrStatus->ulDiscoveredDetoursNum )
			{
				// Decrement the CPU EIP.

				psisSysInterrStatus->vx86ccProcessors[ psisSysInterrStatus->dwCurrentProcessor ].x86vicContext.EIP --;

				// Don't make the Code Window to scroll...

				psisSysInterrStatus->bINT3RequiresCodeWinEipDec = FALSE;
			}
			else if ( bPFMDBCheckOutRes )
			{
				//
				// The Breakpoint originated from a "Stale" Hardcoded BP. The Page Frame was restored and the execution
				//  should resume to the Previous Restored Instruction.
				//

				// Set the Flag.

				psisSysInterrStatus->bResumeExecBecauseOfStaleBpInPf = TRUE;

				// Decrement the CPU EIP.

				psisSysInterrStatus->vx86ccProcessors[ psisSysInterrStatus->dwCurrentProcessor ].x86vicContext.EIP --;
			}
			else if ( psisSysInterrStatus->dwInt3OpcodeInstructionPtr >= (DWORD) & TimerThreadDebuggerEntrance &&
				psisSysInterrStatus->dwInt3OpcodeInstructionPtr < ( (DWORD) & TimerThreadDebuggerEntrance ) + 8 )
			{
				//
				// The Breakpoint originated from the Timer Thread Debugger Entrance function. This function is called
				//  at specified intervals so the Page Frame Modifications Database is kept accurate.
				//

				psisSysInterrStatus->bTimerThreadEventOccurred = TRUE;
			}
			else if ( AddressIsUser2KernelGate( psisSysInterrStatus->dwInt3OpcodeInstructionPtr ) )
			{
				//
				// The Breakpoint is a "User2Kernel" Gate.
				// Handle it appropriately.
				//

				// Set the Flag.

				psisSysInterrStatus->bUser2KernelGateBkpOccurred = TRUE;

				// Increment the EIP.

				psisSysInterrStatus->vx86ccProcessors[ psisSysInterrStatus->dwCurrentProcessor ].x86vicContext.EIP +=
					MACRO_U2KGUID_LEN;
			}
			else
			{
				// Check whether the Execution should go to the Original Int3 Handler...

				if ( extension->ulI3HereState == MACRO_I3HERE_OFF ||
					( extension->ulI3HereState == MACRO_I3HERE_DRV && psisSysInterrStatus->dwInt3OpcodeInstructionPtr < (DWORD) g_pvMmUserProbeAddress ) )
				{
					psisSysInterrStatus->bTransferControlToOriginalInt3 = TRUE;
				}
			}
		}
	}

	// === Return to the Caller. ===

	return;
}

//================================================
// NEW_MapViewOfImageSection Function Definition.
//================================================

static DWORD		g_dwPrevMapViewOfImageSection;

VOID __declspec( naked ) NEW_MapViewOfImageSection( VOID )
{
	__asm
	{
		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		lea				eax, [ esp + MACRO_SIZEOF_PUSHED_VARS ]
		push			eax

		call			SetMapViewOfImageSectionFnPtr
		add				esp, 4

		MACRO_INTHOOK_POP_REGISTERS_PLAIN
		jmp				cs:g_dwPrevMapViewOfImageSection;
	}
}

//==================================================
// NEW_UnMapViewOfImageSection Function Definition.
//==================================================

static DWORD		g_dwPrevUnMapViewOfImageSection;

VOID __declspec( naked ) NEW_UnMapViewOfImageSection( VOID )
{
	__asm
	{
		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		lea				eax, [ esp + MACRO_SIZEOF_PUSHED_VARS ]
		push			eax

		call			VpcICEUnMapViewOfImageSection
		add				esp, 4

		MACRO_INTHOOK_POP_REGISTERS_PLAIN
		jmp				cs:g_dwPrevUnMapViewOfImageSection;
	}
}

//=================================================================================
// MapViewOfImageSection System Return Addresses Structures.
//
//   ###WARNING### The access to These Structures is regulated by the
//                   SYSINTERRUPTS_STATUS.mpslMapViewOfImageSectionSync spin lock.
//=================================================================================

typedef struct _MAPVIEWOFIMAGESECTION_RETINFO
{
	DWORD			dwStackPointer;
	DWORD			dwReturnAddress;

	DWORD*			pdwStartValuePtr;
	DWORD*			pdwSizeValuePtr;
	BYTE*			pbKpeb;

} MAPVIEWOFIMAGESECTION_RETINFO, *PMAPVIEWOFIMAGESECTION_RETINFO;

#define MACRO_MVOISRETINFO_MAXPOS		0x20
static MAPVIEWOFIMAGESECTION_RETINFO	g_vmvoisriMvoisRetInfo[ MACRO_MVOISRETINFO_MAXPOS ];
static ULONG							g_ulMvoisRetInfoPos = 0;

//====================================================
// SetMapViewOfImageSectionFnPtr Function Definition.
//====================================================

VOID __cdecl SetMapViewOfImageSectionFnPtr( DWORD dwRetAddressOnStackPtr )
{
	DETOUR*								pdDetour;
	MAPVIEWOFIMAGESECTION_RETINFO*		pmvoisriRetInfo;

	// Set up the Return Function.

	pdDetour = GetDetourByType( g_psisSysInterrStatus, MACRO_DETOURTYPE_SYS_MAPVIEWOFIMAGESECTION );
	g_dwPrevMapViewOfImageSection = (DWORD) pdDetour->pstTrampolineInf->pvTrampoline;

	// Enter.

	__asm
	{
		mov			ebx, g_psisSysInterrStatus
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslMapViewOfImageSectionSync
		call		EnterMultiProcessorSpinLock
	}

	// Set up an Entry in the List.

	if ( g_ulMvoisRetInfoPos < MACRO_MVOISRETINFO_MAXPOS )
	{
		pmvoisriRetInfo = & g_vmvoisriMvoisRetInfo[ g_ulMvoisRetInfoPos ++ ];

		pmvoisriRetInfo->dwStackPointer = dwRetAddressOnStackPtr;
		pmvoisriRetInfo->dwReturnAddress = * (DWORD*) dwRetAddressOnStackPtr;
		pmvoisriRetInfo->pdwStartValuePtr = (DWORD*) * (DWORD*) ( dwRetAddressOnStackPtr + 4 + MACRO_MAPVIEWOFIMAGESECTION_STARTPARAM );
		pmvoisriRetInfo->pdwSizeValuePtr = (DWORD*) * (DWORD*) ( dwRetAddressOnStackPtr + 4 + MACRO_MAPVIEWOFIMAGESECTION_SIZEPARAM );
		pmvoisriRetInfo->pbKpeb = (BYTE*) * (DWORD*) ( dwRetAddressOnStackPtr + 4 + MACRO_MAPVIEWOFIMAGESECTION_KPEBPARAM );

		* (DWORD*) dwRetAddressOnStackPtr = (DWORD) & MapViewOfImageSectionCallBack;
	}

	// Leave.

	__asm
	{
		mov			ebx, g_psisSysInterrStatus
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslMapViewOfImageSectionSync
		call		LeaveMultiProcessorSpinLock
	}

	// Return to the Caller.

	return;
}

//====================================================
// MapViewOfImageSectionCallBack Function Definition.
//====================================================

static VOID __cdecl MapViewOfImageSectionCallBackIMPL( DWORD dwEaxValue, DWORD dwRetAddressOnStackPtr )
{
	PDEVICE_EXTENSION					extension = g_pDeviceObject->DeviceExtension;
	ULONG								ulI, ulJ;
	MAPVIEWOFIMAGESECTION_RETINFO*		pmvoisriRetInfo;
	DWORD*								pdwStartPtr = NULL;
	DWORD*								pdwSizePtr = NULL;
	DWORD								dwStart, dwSize, dwKPEB;
	VOID*								pvPCRAddress;
	DISCVBPTR_ADDPARAMS					dvbpapAddParams;
	CHAR								szBuffer[ 512 ];
	BYTE*								pbKpeb;
	x86_GDTReg							xgrGDTR;
	ISCONTEXTCOMPATIBLE_CACHEINFOS		iccciCacheInfos;

	memset( & dvbpapAddParams, 0, sizeof( dvbpapAddParams ) );

	memset( & iccciCacheInfos, 0, sizeof( iccciCacheInfos ) );

	// Enter.

	__asm
	{
		mov			ebx, g_psisSysInterrStatus
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslMapViewOfImageSectionSync
		call		EnterMultiProcessorSpinLock
	}

	// Search for the Entry.

	for ( ulI = 0; ulI < g_ulMvoisRetInfoPos; ulI ++ )
	{
		pmvoisriRetInfo = & g_vmvoisriMvoisRetInfo[ ulI ];

		if ( dwRetAddressOnStackPtr >= pmvoisriRetInfo->dwStackPointer &&
			dwRetAddressOnStackPtr <= pmvoisriRetInfo->dwStackPointer + 0x100 )
		{
			// Restore the Return Address.

			* (DWORD*) dwRetAddressOnStackPtr = pmvoisriRetInfo->dwReturnAddress;

			// Remember the Stack Pointers.

			pdwStartPtr = pmvoisriRetInfo->pdwStartValuePtr;
			pdwSizePtr = pmvoisriRetInfo->pdwSizeValuePtr;
			pbKpeb = pmvoisriRetInfo->pbKpeb;

			// Shift the List.

			for ( ulJ = ulI; ulJ < g_ulMvoisRetInfoPos; ulJ ++ )
				g_vmvoisriMvoisRetInfo[ ulJ ] = g_vmvoisriMvoisRetInfo[ ulJ + 1 ];

			g_ulMvoisRetInfoPos --;

			// Exit.

			break;
		}
	}

	// Leave.

	__asm
	{
		mov			ebx, g_psisSysInterrStatus
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslMapViewOfImageSectionSync
		call		LeaveMultiProcessorSpinLock
	}

	// Log the Load of this Module.

	if ( dwEaxValue == 0 &&
		pdwStartPtr && pdwSizePtr &&
		IsPagePresent_DWORD( pdwStartPtr ) && IsPagePresent_DWORD( pdwSizePtr ) )
	{
		dwStart = * pdwStartPtr;
		dwSize = * pdwSizePtr;

		if ( dwStart && dwSize )
		{
			// Get the Address of the PCR.

			__asm
			{
				lea			eax, xgrGDTR
				SGDT		[ eax ]
				mov			ecx, [ eax ]x86_GDTReg.dwBase

				xor			eax, eax
				mov			ax, MACRO_PCR_SELECTOR_IN_SYSTEMCODE
				and			eax, 0xFFFFFFF8
				add			eax, ecx

				mov			pvPCRAddress, 0

				mov			edx, dword ptr[ eax + 4 ]
				and			edx, 0xFF000000
				or			pvPCRAddress, edx

				mov			edx, dword ptr[ eax + 4 ]
				and			edx, 0x000000FF
				shl			edx, 16
				or			pvPCRAddress, edx

				mov			edx, dword ptr[ eax ]
				shr			edx, 16
				or			pvPCRAddress, edx
			}

			// Get the Address of the KPEB.

			if ( pbKpeb == NULL || (DWORD) pbKpeb == 0xFFFFFFFF || IsPagePresent( (PVOID) pbKpeb ) == FALSE )
				dwKPEB = (DWORD) GetCurrentKPEB( pvPCRAddress );
			else
				dwKPEB = (DWORD) pbKpeb;

			// Get the Name of the Module from the VADs.

			dvbpapAddParams.bTouchPages = TRUE;
			dvbpapAddParams.pbKpeb = (BYTE*) dwKPEB;
			dvbpapAddParams.bPreserveModuleFileExtension = TRUE;

			KeAttachProcess( (PEPROCESS) dwKPEB );

			DiscoverBytePointerPosInModules( szBuffer,
				(BYTE*) dwStart,
				extension->pvNtoskrnlDriverSection, g_pvMmUserProbeAddress,
				pvPCRAddress,
				& dvbpapAddParams );

			KeDetachProcess();

			if ( dvbpapAddParams.bIsBytePointerAtTheBeginning &&
				strlen( szBuffer ) )
			{
				BOOLEAN				bProcessAttached = FALSE;
				DWORD				dwAddress;
				BOOLEAN				bIsContextCompatible;

				// Log the Load.

				OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": Load32 -> START=%.8X  SIZE=%X  KPEB=%.8X  MOD=%s",
					dwStart, dwSize, dwKPEB, szBuffer );

				// Check whether there is a Breakpoint/Detour to activate.

				for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
				{
					BREAKPOINT*			pbThis = & extension->sisSysInterrStatus.vbpBreakpoints[ ulI ];

					if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_EXEC ||
						pbThis->bDisabled ||
						pbThis->dwAddress > (DWORD) g_pvMmUserProbeAddress )
					{
						continue;
					}

					// Check the Context Compatibility.

					dwAddress = pbThis->dwAddress;
					bIsContextCompatible = IsContextCompatible( & pbThis->ciContext,
						& dwAddress, (BYTE*) dwKPEB, pvPCRAddress,
						& iccciCacheInfos, szBuffer, dwStart );

					if ( bIsContextCompatible &&
						dwAddress >= dwStart && dwAddress < dwStart + dwSize )
					{
						if ( bProcessAttached == FALSE )
						{
							KeAttachProcess( (PEPROCESS) dwKPEB );
							bProcessAttached = TRUE;
						}

						TouchPage_BYTE( (BYTE*) dwAddress );
						EnableSingleBreakpointIMPL( pbThis, TRUE, dwAddress, 0 );
					}
				}

				for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
				{
					DETOUR*				pdThis = & extension->sisSysInterrStatus.vdDetours[ ulI ];

					if ( pdThis->ulType == MACRO_DETOURTYPE_UNUSED ||
						pdThis->dwAddress > (DWORD) g_pvMmUserProbeAddress )
					{
						continue;
					}

					// Check the Context Compatibility.

					dwAddress = pdThis->dwAddress;
					bIsContextCompatible = IsContextCompatible( & pdThis->ciContext,
						& dwAddress, (BYTE*) dwKPEB, pvPCRAddress,
						& iccciCacheInfos, szBuffer, dwStart );

					if ( bIsContextCompatible &&
						dwAddress >= dwStart && dwAddress < dwStart + dwSize )
					{
						if ( bProcessAttached == FALSE )
						{
							KeAttachProcess( (PEPROCESS) dwKPEB );
							bProcessAttached = TRUE;
						}

						TouchPage_BYTERANGE( (BYTE*) dwAddress, pdThis->ulDetourSize );
						EnableSingleDetourIMPL( pdThis, TRUE, dwAddress );
					}
				}

				// Detach from the Process Address Space.

				if ( bProcessAttached )
					KeDetachProcess();
			}
		}
	}

	// Return to the Caller.

	return;
}

VOID __declspec( naked ) MapViewOfImageSectionCallBack( VOID )
{
	__asm
	{
		push		0xFFFFFFFF	// Used by the "RET" Instruction.

		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		lea				eax, [ esp + MACRO_SIZEOF_PUSHED_VARS ]
		mov				ebx, [ esp + MACRO_PROCCONTEXT_STACK_DISP_EAX ]

		push			eax
		push			ebx

		call			MapViewOfImageSectionCallBackIMPL
		add				esp, 8

		MACRO_INTHOOK_POP_REGISTERS_PLAIN
		ret
	}
}

//====================================================
// VpcICEUnMapViewOfImageSection Function Definition.
//====================================================

VOID __cdecl VpcICEUnMapViewOfImageSection( DWORD dwRetAddressOnStackPtr )
{
	PDEVICE_EXTENSION					extension = g_pDeviceObject->DeviceExtension;
	DWORD								dwStart;
	VOID*								pvPCRAddress;
	CHAR								szBuffer[ 512 ];
	DISCVBPTR_ADDPARAMS					dvbpapAddParams;
	DETOUR*								pdDetour;
	BYTE*								pbKpeb;
	x86_GDTReg							xgrGDTR;

	memset( & dvbpapAddParams, 0, sizeof( dvbpapAddParams ) );

	// Set up the Return Function.

	pdDetour = GetDetourByType( g_psisSysInterrStatus, MACRO_DETOURTYPE_SYS_UNMAPVIEWOFIMAGESECTION );
	g_dwPrevUnMapViewOfImageSection = (DWORD) pdDetour->pstTrampolineInf->pvTrampoline;

	// Log the Unload.

	dwStart = * (DWORD*) ( dwRetAddressOnStackPtr + 4 + MACRO_UNMAPVIEWOFIMAGESECTION_STARTPARAM );
	pbKpeb = (BYTE*) * (DWORD*) ( dwRetAddressOnStackPtr + 4 + MACRO_UNMAPVIEWOFIMAGESECTION_KPEBPARAM );

	if ( dwStart )
	{
		// Get the Address of the PCR.

		__asm
		{
			lea			eax, xgrGDTR
			SGDT		[ eax ]
			mov			ecx, [ eax ]x86_GDTReg.dwBase

			xor			eax, eax
			mov			ax, MACRO_PCR_SELECTOR_IN_SYSTEMCODE
			and			eax, 0xFFFFFFF8
			add			eax, ecx

			mov			pvPCRAddress, 0

			mov			edx, dword ptr[ eax + 4 ]
			and			edx, 0xFF000000
			or			pvPCRAddress, edx

			mov			edx, dword ptr[ eax + 4 ]
			and			edx, 0x000000FF
			shl			edx, 16
			or			pvPCRAddress, edx

			mov			edx, dword ptr[ eax ]
			shr			edx, 16
			or			pvPCRAddress, edx
		}

		// Check the KPEB Pointer.

		if ( pbKpeb == NULL || (DWORD) pbKpeb == 0xFFFFFFFF || IsPagePresent( (PVOID) pbKpeb ) == FALSE )
			pbKpeb = (BYTE*) GetCurrentKPEB( pvPCRAddress );

		// Get the Name of the Module from the VADs.

		dvbpapAddParams.bTouchPages = TRUE;
		dvbpapAddParams.pbKpeb = pbKpeb;
		dvbpapAddParams.bPreserveModuleFileExtension = TRUE;

		KeAttachProcess( (PEPROCESS) pbKpeb );

		DiscoverBytePointerPosInModules( szBuffer,
			(BYTE*) dwStart,
			extension->pvNtoskrnlDriverSection, g_pvMmUserProbeAddress,
			pvPCRAddress,
			& dvbpapAddParams );

		KeDetachProcess();

		if ( dvbpapAddParams.bIsBytePointerAtTheBeginning &&
			strlen( szBuffer ) )
		{
			// Log the Load.

			OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": Unload32 -> MOD=%s",
				szBuffer );
		}
	}

	// Return to the Caller.

	return;
}

//==========================================================
// VpcICESystemServiceInterruptHandler Function Definition.
//==========================================================

static BOOLEAN UserModListCallBack( IN CHAR* pszModName, IN VOID* pvStart, IN ULONG ulLength, IN IMAGE_NT_HEADERS* pinhNtHeaders, IN DWORD dwParam )
{
	CHAR*		pszImageNameToBePrinted;

	// === Log the Load. ===

	if ( pszModName && strlen( pszModName ) )
	{
		// Set the Pointer to the Image Name String.

		pszImageNameToBePrinted = & pszModName[ strlen( pszModName ) - 1 ];

		for( ; pszImageNameToBePrinted >= pszModName; pszImageNameToBePrinted -- )
			/*if ( * pszImageNameToBePrinted == '.' )	// Extension must be Printed...
				* pszImageNameToBePrinted = '\0';
			else */if ( * pszImageNameToBePrinted == '\\' ||
				* pszImageNameToBePrinted == '/' )
					break;

		pszImageNameToBePrinted ++;

		// Print the Image Name in the LOG.

		if ( strlen( pszImageNameToBePrinted ) )
		{
			OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": Unload32* -> MOD=%s",
				pszImageNameToBePrinted );
		}
	}

	// === Return. ===

	return TRUE;
}

static VOID __cdecl NtTerminateProcessHandlerIMPL( IN HANDLE* pProcessHandle, IN LONG* pExitStatus )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	HANDLE					ProcessHandle;
	LONG					ExitStatus;

	BYTE*					pbKpeb = NULL;

	DWORD					dwPID;
	CHAR*					pszImgNamePtr;
	CHAR					szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE

	DISCVBPTR_ADDPARAMS		dvbpapAddParams;

	VOID*					pvPCRAddress;

	DWORD					dwAttachPrCookie;

	x86_GDTReg				xgrGDTR;

	CHAR*					pszDot;

	memset( & dvbpapAddParams, 0, sizeof( dvbpapAddParams ) );

	// Get the Parameters.

	if ( IsPagePresent_DWORD( (DWORD*) pProcessHandle ) == FALSE ||
		IsPagePresent_DWORD( (DWORD*) pExitStatus ) == FALSE )
			return;

	ProcessHandle = * pProcessHandle;
	ExitStatus = * pExitStatus;

	// Get the Address of the PCR.

	__asm
	{
		lea			eax, xgrGDTR
		SGDT		[ eax ]
		mov			ecx, [ eax ]x86_GDTReg.dwBase

		xor			eax, eax
		mov			ax, MACRO_PCR_SELECTOR_IN_SYSTEMCODE
		and			eax, 0xFFFFFFF8
		add			eax, ecx

		mov			pvPCRAddress, 0

		mov			edx, dword ptr[ eax + 4 ]
		and			edx, 0xFF000000
		or			pvPCRAddress, edx

		mov			edx, dword ptr[ eax + 4 ]
		and			edx, 0x000000FF
		shl			edx, 16
		or			pvPCRAddress, edx

		mov			edx, dword ptr[ eax ]
		shr			edx, 16
		or			pvPCRAddress, edx
	}

	// Get the KPEB of the Handle.

	if ( (DWORD) ProcessHandle == 0xFFFFFFFF )
	{
		pbKpeb = (BYTE*) GetCurrentKPEB( pvPCRAddress );
	}
	else
	{
#if 0 // --> START of OMITTED CODE...

				//=================================================================================================
				//
				// ### WARNING !!! ###
				//
				// The following API Calls are from a Context in VpcICE that is UNSTABLE !!
				// The Interrupt Flag is OFF and the Transition in Kernel Mode is from an Interrupt Call that was not
				//  processed by the System (so the System-Wide State may be INCONSISTENT at this point in Code.).
				//
				//=================================================================================================

				NTSTATUS				nsRes;

				nsRes = ObReferenceObjectByHandle( ProcessHandle,
					(ACCESS_MASK) 0, (POBJECT_TYPE) NULL, KernelMode,
					(PVOID*) & pbKpeb,
					(POBJECT_HANDLE_INFORMATION) NULL );

				if ( nsRes != STATUS_SUCCESS )
					return;

				ObDereferenceObject( (PVOID) pbKpeb );

				//=================================================================================================
				//
				// ### END of WARNING... ###
				//
				//=================================================================================================

#endif // --> END of OMITTED CODE...
	}

	if ( pbKpeb == NULL || IsPagePresent( (PVOID) pbKpeb ) == FALSE )
		return;

	// Log the Exit.

	dwPID = * (DWORD*) ( pbKpeb + MACRO_PID_FIELDOFFSET_IN_KPEB );
	pszImgNamePtr = (CHAR*) ( pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );

	memset( szProcessName, 0, sizeof( szProcessName ) );
	if ( pszImgNamePtr )
		memcpy( szProcessName, pszImgNamePtr, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );
	else
		strcpy( szProcessName, "???" );

	pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
	if ( pszDot )
		* pszDot = 0;

	OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": Exit32 -> PID=%X  MOD=%s",
		dwPID, szProcessName );

	// Log the Unload of all the Modules.

	dvbpapAddParams.bTouchPages = FALSE;
	dvbpapAddParams.pbKpeb = pbKpeb;
	dvbpapAddParams.pfnUserModListCallBack = & UserModListCallBack;
	dvbpapAddParams.dwUserModListCallBackPARAM = 0;
	dvbpapAddParams.bPreserveModuleFileExtension = TRUE;

	dwAttachPrCookie = VpcICEAttachProcess( pbKpeb );

	DiscoverBytePointerPosInModules( NULL, NULL,
		extension->pvNtoskrnlDriverSection, g_pvMmUserProbeAddress,
		pvPCRAddress,
		& dvbpapAddParams );

	VpcICEDetachProcess( dwAttachPrCookie );

	// Return to the Caller.

	return;
}

VOID __declspec( naked ) VpcICESystemServiceInterruptHandler( VOID )
{
	__asm
	{
		// Push the State and Setup the Environment.

		MACRO_INTHOOK_PUSH_REGISTERS
		MACRO_INTHOOK_SETUP_ENVIRONMENT

		// Check if the system required a "NtTerminateProcess" Call.

		mov			eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EAX ]

		cmp			[ edx ]SYSINTERRUPTS_STATUS.ulNtTerminateProcessFnIndex, MACRO_GETNTTERMINATEPROCESSFNINDEX_ERR
		je			ExitFromISRWithRET
		cmp			[ edx ]SYSINTERRUPTS_STATUS.ulNtTerminateProcessFnIndex, eax
		jne			ExitFromISRWithRET

		// Save + Set FS.

		mov			cx, fs			// <--- <--- <--- ### WARNING! ### CX contains the Previous FS !!!
		mov			ax, g_wFSSEG_in_Kernel
		mov			fs, ax

		// Process the Request.

		mov			eax, dword ptr[ esp + MACRO_PROCCONTEXT_STACK_DISP_EDX ]
		lea			ebx, [ eax + 4 ]

		push		ecx

			push		ebx
			push		eax

			call		NtTerminateProcessHandlerIMPL
			add			esp, 8

		pop			ecx

		// Restore FS.

		mov			fs, cx			// <--- <--- <--- ### WARNING! ### FS is restored from CX !!!

		// ISR Exit.

ExitFromISRWithRET:

		MACRO_INTHOOK_POP_REGISTERS_PLAIN
		ret
	}
}

//========================================
// InstallBreakpoint Function Definition.
//========================================

BOOLEAN InstallBreakpoint( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulType, IN DWORD dwAddress, IN ULONG ulDebugReg, IN CHAR* pszProcessName, IN CHAR* pszModuleName )
{
	INSTALLDETOURORBREAKPOINT_PARAMS		idobParams;

	// Call the Implementation Function and Return.

	memset( & idobParams, 0, sizeof( idobParams ) );

	idobParams.ulObjectType = MACRO_IDOB_OBJECTTYPE_BREAKPOINT;

	idobParams.psisSysInterrStatus = psisSysInterrStatus;
	idobParams.ulType = ulType;
	idobParams.dwAddress = dwAddress;
	idobParams.bUseSEH = FALSE; // ... FIXED ...
	idobParams.pszProcessName = pszProcessName;
	idobParams.pszModuleName = pszModuleName;

	idobParams.ulDebugReg = ulDebugReg;

	return InstallDetourOrBreakpoint( & idobParams );
}

//=================================================
// AreMemoryRangesConflicting Function Definition.
//=================================================

BOOLEAN AreMemoryRangesConflicting( IN DWORD dwAddress0, IN ULONG ulSize0, IN DWORD dwAddress1, IN ULONG ulSize1 )
{
	// Do the Test.

	if ( ulSize0 == 0 || ulSize1 == 0 )
	{
		return FALSE;
	}
	else if ( dwAddress0 < dwAddress1 &&
		dwAddress0 + ulSize0 - 1 < dwAddress1 )
	{
		return FALSE;
	}
	else if ( dwAddress1 < dwAddress0 &&
		dwAddress1 + ulSize1 - 1 < dwAddress0 )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

//==============================================
// AreContextesConflicting Function Definition.
//==============================================

BOOLEAN AreContextesConflicting( IN CONTEXT_INFO* pciContext0, IN ULONG ulSize0, IN CONTEXT_INFO* pciContext1, IN ULONG ulSize1 )
{
	CHAR			szProcessName0[ MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE ];
	CHAR			szProcessName1[ MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE ];

	// Do the Test.

	if ( strlen( pciContext0->szModuleName ) || strlen( pciContext1->szModuleName ) )
	{
		if ( strlen( pciContext0->szModuleName ) == 0 || strlen( pciContext1->szModuleName ) == 0 ) // Addresses not comparable.
		{
			return FALSE;
		}
		else if ( MACRO_CRTFN_NAME(stricmp)( pciContext0->szModuleName, pciContext1->szModuleName ) )
		{
			return FALSE;
		}
		else
		{
			return AreMemoryRangesConflicting(
				pciContext0->dwModuleOffset, ulSize0,
				pciContext1->dwModuleOffset, ulSize1 );
		}
	}

	if ( ContextProcName2KpebLikeProcName( szProcessName0, pciContext0->szProcessName ) == FALSE ||
		ContextProcName2KpebLikeProcName( szProcessName1, pciContext1->szProcessName ) == FALSE )
	{
		return FALSE;
	}
	else if ( strlen( szProcessName0 ) == 0 || strlen( szProcessName1 ) == 0 ||
		MACRO_CRTFN_NAME(stricmp)( szProcessName0, szProcessName1 ) == 0 )
	{
		return AreMemoryRangesConflicting(
			pciContext0->dwModuleOffset, ulSize0,
			pciContext1->dwModuleOffset, ulSize1 );
	}
	else
	{
		return FALSE;
	}
}

//============================================
// VpcICEProcessCallback Function Definition.
//============================================

VOID VpcICEProcessCallback( IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bAllocRes;
	NTSTATUS				ntStatus;
	BYTE*					pbKpeb = NULL;
	CHAR*					pszCurrentProcessName;
	CHAR					szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
	CHAR*					pszDot;

	// Only Process Creations.

	if ( Create == FALSE || ProcessId == 0 )
		return;

	// Allocate the Detours Memory in this Process.

	ExAcquireFastMutex( & extension->sisSysInterrStatus.fmVpcICEProcessCallbackAccess );

		// Allocate the Memory.

		bAllocRes = /* AllocateUserSpaceTrampolineStructures( & extension->sisSysInterrStatus,
			NULL, (DWORD) ProcessId ); */ // DETOURLESS
			AllocateUserSpaceMemory( & extension->sisSysInterrStatus,
			NULL, (DWORD) ProcessId );

		// Get the Name of the Process and Display a Message.

		ntStatus = PsLookupProcessByProcessId(
			(DWORD) ProcessId, & pbKpeb );

		if ( ntStatus == STATUS_SUCCESS && pbKpeb )
		{
			pszCurrentProcessName = (CHAR*) ( pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );
			memset( & szProcessName, 0, sizeof( szProcessName ) );
			memcpy( & szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

			pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
			if ( pszDot )
				* pszDot = 0;

			if ( bAllocRes )
				OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": UsrSpaceOK -> PROCID=%X  NAME=%s  ADDR(FXD)=%X",
					(DWORD) ProcessId, szProcessName, extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresAddress );
			else
				OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": UsrSpaceAllocERR -> PROCID=%X  NAME=%s",
					(DWORD) ProcessId, szProcessName );
		}

	ExReleaseFastMutex( & extension->sisSysInterrStatus.fmVpcICEProcessCallbackAccess );

	// Return to the Caller.

	return;
}

//==========================================
// IsContextCompatible Function Definition.
//==========================================

BOOLEAN IsContextCompatible( IN CONTEXT_INFO* pciContext, IN OUT DWORD* pdwCurrCntxtAddress, IN BYTE* pbKpeb, IN VOID* pvPCRAddress, IN OUT ISCONTEXTCOMPATIBLE_CACHEINFOS* pCacheInfos, IN CHAR* pszDirectModuleNameCompare, IN DWORD dwDirectCompareModuleStart )
{
	CHAR*						pszCurrentProcessName;
	SEARCHFORUSERMODULE_INFOS	sfumiCbInfos;
	CHAR*						pszDot;
	BOOLEAN						bCompareNames = TRUE;

	// Check whether User mode or Kernel mode.

	if ( * pdwCurrCntxtAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel area.
	{
		return TRUE;
	}
	else // User area.
	{
		// Process Name.

		if ( strlen( pciContext->szProcessName ) ) // A Process in particular.
		{
			// Compare the Process Names.

			if ( pCacheInfos->bProcessNameInit == FALSE )
			{
				if ( pbKpeb == NULL )
					pszCurrentProcessName = GetImageFileNameFieldPtrOfCurrProc( pvPCRAddress );
				else
					pszCurrentProcessName = (CHAR*) ( pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );

				memset( pCacheInfos->szProcessName, 0, MACRO_ISCONTEXTCOMPATIBLE_CACHEINFOS_PROCESSNAME_FIELD_SIZE );
				if ( pszCurrentProcessName )
					memcpy( pCacheInfos->szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );
				else
					return FALSE;

				pszDot = MACRO_CRTFN_NAME(strchr)( pCacheInfos->szProcessName, '.' );
				if ( pszDot )
					* pszDot = 0;

				pCacheInfos->bProcessNameInit = TRUE;
			}

			//

			if ( strlen( pciContext->szProcessName ) == MACRO_CONTEXTINFO_PROCESSNAME_HEXFORM_SIZE &&
				pciContext->szProcessName[ 0 ] == '0' &&
				pciContext->szProcessName[ 1 ] == 'x' )
			{
				CHAR*		pszEndPtr;
				DWORD		dwKpebHexForm = MACRO_CRTFN_NAME(strtoul)(
					pciContext->szProcessName, & pszEndPtr, 16 );
				if ( dwKpebHexForm > (DWORD) g_pvMmUserProbeAddress &&
					( pszEndPtr == NULL || * pszEndPtr == 0 ) )
				{
					BYTE*		pbCurrentKPEB;

					if ( pbKpeb == NULL )
						pbCurrentKPEB = GetCurrentKPEB( pvPCRAddress );
					else
						pbCurrentKPEB = pbKpeb;

					// Compare the Hex KPEB with the Current KPEB.

					if ( (DWORD) pbCurrentKPEB != dwKpebHexForm )
						return FALSE;
					else
						bCompareNames = FALSE;
				}
			}

			//

			if ( bCompareNames &&
				MACRO_CRTFN_NAME(stricmp)( pCacheInfos->szProcessName, pciContext->szProcessName ) )
			{
				return FALSE;
			}
		}

		// Module Name.

		if ( strlen( pciContext->szModuleName ) == 0 ) // Any Module.
		{
			* pdwCurrCntxtAddress = pciContext->dwModuleOffset;
		}
		else // A Module in particular.
		{
			if ( pszDirectModuleNameCompare )
			{
				// Simply compare the Names of the Modules.

				if ( MACRO_CRTFN_NAME(stricmp)( pciContext->szModuleName, pszDirectModuleNameCompare ) )
					return FALSE;

				// Set the Absolute Address.

				* pdwCurrCntxtAddress = dwDirectCompareModuleStart + pciContext->dwModuleOffset;
			}
			else
			{
				// Check the Cached Item first.

				if ( pCacheInfos->bModuleNameInit == FALSE ||
					MACRO_CRTFN_NAME(stricmp)( pciContext->szModuleName, pCacheInfos->szModuleName ) )
				{
					// Get the Informations about the Module.

					sfumiCbInfos.pszModuleName = pciContext->szModuleName;
					sfumiCbInfos.pbKpeb = NULL;
					sfumiCbInfos.pbModuleStart = NULL;
					sfumiCbInfos.bUseSEH = FALSE;
					sfumiCbInfos.pvPCRAddress = pvPCRAddress;
					sfumiCbInfos.ulModuleLength = 0;
					sfumiCbInfos.pinhModuleNtHdrs = NULL;
					sfumiCbInfos.bRegionIsMappedIn = FALSE;
					sfumiCbInfos.bRegionIsInModule = FALSE;
					sfumiCbInfos.dwRegionRelPtr = 0;
					sfumiCbInfos.ulRegionSize = 0;
					sfumiCbInfos.pbCurrentCbKpeb = NULL;
					sfumiCbInfos.pfnModCallBack = NULL;
					sfumiCbInfos.bNormalizeModuleName = TRUE;

					SearchForUserModule(
						pbKpeb == NULL ? (BYTE*) GetCurrentKPEB( pvPCRAddress ) : pbKpeb, (DWORD) & sfumiCbInfos );

					if ( sfumiCbInfos.pbKpeb == NULL || sfumiCbInfos.pbModuleStart == NULL )
						return FALSE;

					// Save in the Cache.

					pCacheInfos->bModuleNameInit = TRUE;
					pCacheInfos->dwModuleStart = (DWORD) sfumiCbInfos.pbModuleStart;

					if ( strlen( sfumiCbInfos.szNormalizedModuleName ) >= MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE )
					{
						memcpy( pCacheInfos->szModuleName, sfumiCbInfos.szNormalizedModuleName, MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 );
						pCacheInfos->szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 ] = 0;
					}
					else
					{
						strcpy( pCacheInfos->szModuleName, sfumiCbInfos.szNormalizedModuleName );
					}
				}

				// Set the Absolute Address.

				* pdwCurrCntxtAddress = pCacheInfos->dwModuleStart + pciContext->dwModuleOffset;
			}
		}
	}

	// Return.

	return TRUE;
}

//================================================
// InstallDetourOrBreakpoint Function Definition.
//================================================

static BOOLEAN InstallDetourImplSEH( VOID* pvTrampoline, VOID* pvNewFunc, DELAY_TARGET_JUMP_INSERTION* pdtjiJumpInfo )
{
	BOOLEAN		bRetVal;

	__try
	{
		bRetVal = DetourFunctionWithTrampoline( (PBYTE) pvTrampoline, (PBYTE) pvNewFunc, pdtjiJumpInfo );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		bRetVal = FALSE;
	}

	return bRetVal;
}

//

static BYTE			g_vbDetourMemSnapshot[ MACRO_DETOUR_MAXSIZE ];

//

BOOLEAN InstallDetourOrBreakpoint( IN INSTALLDETOURORBREAKPOINT_PARAMS* pidobParams )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN							bRetVal = TRUE;
	ULONG							ulI;
	DETOUR*							pdSlot = NULL;
	BREAKPOINT*						pbSlot = NULL;
	DETOUR*							pdThis;
	STATIC_TRAMPOLINE*				pstTramp = NULL;
	ULONG							ulIndex;
	DELAY_TARGET_JUMP_INSERTION		dtjiJumpInfo;
	CONTEXT_INFO					ciContextInfo;
	BOOLEAN							bRelAddress;
	CHAR*							pszCurrentProcessName;
	BYTE*							pbKpeb;
	GETKPEBPTRFROMPROCNAMECB_INFOS	ckpiCbInfos;
	DISCVBPTR_ADDPARAMS				dvbpapAddParams;
	DWORD							dwAttchProcCookie;
	BOOLEAN							bMemCheckFailed;
	SEARCHFORUSERMODULE_INFOS		sfumiCbInfos;
	BREAKPOINT*						pbThis;
	BOOLEAN							bDetouredDetour = FALSE;
	x86_CPU_CONTEXT*				px86ccThis;
	BYTE							bPrevCodeByte;
	ISCONTEXTCOMPATIBLE_CACHEINFOS	iccciCacheInfos;
	VOID*							pvPCRBase;
	CHAR*							pszDot;
	ULONG							ulStartByteIndex;

	memset( & dvbpapAddParams, 0, sizeof( dvbpapAddParams ) );

	memset( & iccciCacheInfos, 0, sizeof( iccciCacheInfos ) );

	// === Set the Context Info informations, checking the Field Sizes. ===

	pvPCRBase =
		pidobParams->psisSysInterrStatus->vx86ccProcessors[ pidobParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase;

	if ( pvPCRBase )
		pbKpeb = (BYTE*) GetCurrentKPEB( pvPCRBase );
	else
		pbKpeb = NULL;

	if ( pidobParams->pszModuleName && strlen( pidobParams->pszModuleName ) )
		bRelAddress = TRUE;
	else
		bRelAddress = FALSE;

	memset( & ciContextInfo, 0, sizeof( ciContextInfo ) );

	if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // User Space.
	{
		// Get the Correct Kpeb.

		if ( pidobParams->pszProcessName && strlen( pidobParams->pszProcessName ) )
		{
			memset( & ckpiCbInfos, 0, sizeof( ckpiCbInfos ) );
			ckpiCbInfos.pszProcName = pidobParams->pszProcessName;
			ckpiCbInfos.pbKpeb = NULL;
			ckpiCbInfos.bReturnNormalizedProcName = TRUE;

			IterateThroughListOfProcesses( & extension->pliProcessListInfo,
				& GetKpebPtrFromProcNameCB, (DWORD) & ckpiCbInfos );

			if ( ckpiCbInfos.pbKpeb == NULL )
			{
				OutputPrint( FALSE, FALSE, "Process \"%s\" not found.", pidobParams->pszProcessName );
				return FALSE;
			}
			else
			{
				pbKpeb = ckpiCbInfos.pbKpeb;
				pidobParams->pszProcessName = ckpiCbInfos.szNormalizedProcName;
			}
		}

		// Process Name.

		if ( pidobParams->pszProcessName )
		{
			if ( strlen( pidobParams->pszProcessName ) < MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE )
				strcpy( ciContextInfo.szProcessName, pidobParams->pszProcessName );
			else
				memcpy( ciContextInfo.szProcessName, pidobParams->pszProcessName, MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE - 1 );
		}
		else
		{
			pszCurrentProcessName = GetImageFileNameFieldPtrOfCurrProc(
				pidobParams->psisSysInterrStatus->vx86ccProcessors[ pidobParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase );

			if ( pszCurrentProcessName )
			{
				// Store the Normalized Process Name.

				memcpy( ciContextInfo.szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

				pszDot = MACRO_CRTFN_NAME(strchr)( ciContextInfo.szProcessName, '.' );
				if ( pszDot )
					* pszDot = 0;
			}
			else
			{
				OutputPrint( FALSE, FALSE, "Unable to determine the current proc name." );
				return FALSE;
			}
		}
	}

	if ( bRelAddress == FALSE )
	{
		if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // User Space.
		{
			// === Get the Missing Informations. ===

			// Module Name.

			if ( pidobParams->pszModuleName )
			{
				if ( strlen( pidobParams->pszModuleName ) < MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE )
					strcpy( ciContextInfo.szModuleName, pidobParams->pszModuleName );
				else
					memcpy( ciContextInfo.szModuleName, pidobParams->pszModuleName, MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 );

				ciContextInfo.dwModuleOffset = pidobParams->dwAddress;
			}
			else
			{
				// Get the Module Infos.

				dvbpapAddParams.bTouchPages = pidobParams->bUseSEH;
				dvbpapAddParams.pbKpeb = pbKpeb;
				dvbpapAddParams.ulSplitInfoNoSectionStringMaxLen = MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1;
				dvbpapAddParams.dwSplittedOffsetNoSection = 0;
				dvbpapAddParams.bPreserveModuleFileExtension = TRUE;

				if ( pidobParams->bUseSEH )
					KeAttachProcess( (PEPROCESS) pbKpeb );
				else
					dwAttchProcCookie = VpcICEAttachProcess(
						pbKpeb );

				DiscoverBytePointerPosInModules( ciContextInfo.szModuleName,
					(BYTE*) pidobParams->dwAddress,
					extension->pvNtoskrnlDriverSection, g_pvMmUserProbeAddress,
					pidobParams->psisSysInterrStatus->vx86ccProcessors[ pidobParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase,
					& dvbpapAddParams );

				if ( pidobParams->bUseSEH )
					KeDetachProcess();
				else
					VpcICEDetachProcess(
						dwAttchProcCookie );

				if ( strlen( ciContextInfo.szModuleName ) )
					ciContextInfo.dwModuleOffset = dvbpapAddParams.dwSplittedOffsetNoSection;
				else
					ciContextInfo.dwModuleOffset = pidobParams->dwAddress;
			}
		}
		else // Kernel Space.
		{
			// Parameters Validation.

			if ( pidobParams->pszProcessName )
			{
				OutputPrint( FALSE, FALSE, "The /P option is not valid in kernel area." );
				return FALSE;
			}
			else if ( pidobParams->pszModuleName )
			{
				OutputPrint( FALSE, FALSE, "The /M option is not valid in kernel area." );
				return FALSE;
			}
		}
	}
	else
	{
		if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // Silly mem check.
		{
			// === Get the Missing Informations. ===

			// Set up the Context Module Name Info.

			if ( strlen( pidobParams->pszModuleName ) < MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE )
				strcpy( ciContextInfo.szModuleName, pidobParams->pszModuleName );
			else
				memcpy( ciContextInfo.szModuleName, pidobParams->pszModuleName, MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 );

			// Set up the Information Structure.

			sfumiCbInfos.pszModuleName = ciContextInfo.szModuleName;
			sfumiCbInfos.pbKpeb = NULL;
			sfumiCbInfos.pbModuleStart = NULL;
			sfumiCbInfos.bUseSEH = pidobParams->bUseSEH;
			sfumiCbInfos.pvPCRAddress =
				pidobParams->psisSysInterrStatus->vx86ccProcessors[ pidobParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase;
			sfumiCbInfos.ulModuleLength = 0;
			sfumiCbInfos.pinhModuleNtHdrs = NULL;
			sfumiCbInfos.bRegionIsMappedIn = FALSE;
			sfumiCbInfos.bRegionIsInModule = FALSE;
			sfumiCbInfos.dwRegionRelPtr = pidobParams->dwAddress;
			sfumiCbInfos.ulRegionSize =
				( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ? MACRO_DETOUR_MAXSIZE : /*MACRO_BREAKPOINT_SIZE*/ MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE );
			sfumiCbInfos.pbCurrentCbKpeb = NULL;
			sfumiCbInfos.pfnModCallBack = NULL;
			sfumiCbInfos.bNormalizeModuleName = TRUE;

			// Module Name.

			if ( pidobParams->pszProcessName && strlen( pidobParams->pszProcessName ) == 0 )
			{
				// Search for the Module in all the Processes.

				IterateThroughListOfProcesses( & extension->pliProcessListInfo,
					& SearchForUserModule, (DWORD) & sfumiCbInfos );

				if ( sfumiCbInfos.pbKpeb == NULL || sfumiCbInfos.pbModuleStart == NULL )
				{
					OutputPrint( FALSE, FALSE, "Module \"%s\" not found in any process.", ciContextInfo.szModuleName );
					return FALSE;
				}
				else
				{
					// Set the Normalized Version of the Module Name.

					if ( strlen( sfumiCbInfos.szNormalizedModuleName ) >= MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE )
					{
						memcpy( ciContextInfo.szModuleName, sfumiCbInfos.szNormalizedModuleName, MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 );
						ciContextInfo.szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 ] = 0;
					}
					else
					{
						strcpy( ciContextInfo.szModuleName, sfumiCbInfos.szNormalizedModuleName );
					}

					// Print an Information Message to the User.

					OutputPrint( FALSE, FALSE, "Module \"%s\" found (syswide search) -> START=%.8X  SIZE=%X  KPEB=%.8X.",
						ciContextInfo.szModuleName, (DWORD) sfumiCbInfos.pbModuleStart, sfumiCbInfos.ulModuleLength, (DWORD) sfumiCbInfos.pbKpeb );
				}
			}
			else
			{
				// Search for the Module in the Process.

				SearchForUserModule( pbKpeb, (DWORD) & sfumiCbInfos );

				if ( sfumiCbInfos.pbKpeb == NULL || sfumiCbInfos.pbModuleStart == NULL )
				{
					OutputPrint( FALSE, FALSE, "Module \"%s\" not found in current process.", ciContextInfo.szModuleName );
					return FALSE;
				}
				else
				{
					// Set the Normalized Version of the Module Name.

					if ( strlen( sfumiCbInfos.szNormalizedModuleName ) >= MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE )
					{
						memcpy( ciContextInfo.szModuleName, sfumiCbInfos.szNormalizedModuleName, MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 );
						ciContextInfo.szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE - 1 ] = 0;
					}
					else
					{
						strcpy( ciContextInfo.szModuleName, sfumiCbInfos.szNormalizedModuleName );
					}

					// Print an Information Message to the User.

					OutputPrint( FALSE, FALSE, "Module \"%s\" found -> START=%.8X  SIZE=%X.",
						ciContextInfo.szModuleName, (DWORD) sfumiCbInfos.pbModuleStart, sfumiCbInfos.ulModuleLength );
				}
			}

			// Check the Correctness of the Address.

			if ( sfumiCbInfos.bRegionIsInModule == FALSE )
			{
				OutputPrint( FALSE, FALSE, "The specified address is outside the module mapped mem." );
				return FALSE;
			}

			// Check if the Region of interest is Mapped In.

			if ( sfumiCbInfos.bRegionIsMappedIn == FALSE )
			{
				if ( pidobParams->bUseSEH )
				{
					// Touch the Region of Pages.

					KeAttachProcess( (PEPROCESS) sfumiCbInfos.pbKpeb );
					TouchPage_BYTERANGE( sfumiCbInfos.pbModuleStart + pidobParams->dwAddress,
						( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ? MACRO_DETOUR_MAXSIZE : /*MACRO_BREAKPOINT_SIZE*/ MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE ) );
					KeDetachProcess();
				}
				else
				{
					// Print an Error Message and Exit.

					OutputPrint( FALSE, FALSE, "Region of memory in module not paged in." );
					return FALSE;
				}
			}

			// Set up the Remaining Context Informations.

			ciContextInfo.dwModuleOffset = pidobParams->dwAddress;

			// Set the New KPEB Pointer.

			pbKpeb = sfumiCbInfos.pbKpeb;

			// Transform the Relative Address to an Absolute One.

			pidobParams->dwAddress += (DWORD) sfumiCbInfos.pbModuleStart;
		}
		else
		{
			OutputPrint( FALSE, FALSE, "The address expr is invalid." );
			return FALSE;
		}
	}

	// === Search for a Free Slot (Detour Only). ===

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
		{
			ulIndex = pidobParams->ulType < 0x80000000 ? ulI : MACRO_MAXNUM_OF_DETOURS - ulI - 1;
			pdThis = & pidobParams->psisSysInterrStatus->vdDetours[ ulIndex ];

			if ( pdThis->ulType == MACRO_DETOURTYPE_UNUSED )
			{
				pdSlot = pdThis;
				pstTramp = & g_vstStaticTrampolines[ ulIndex ];
				break;
			}
		}

		if ( pdSlot == NULL || pstTramp == NULL ||
			pstTramp->ppvOrigFnPtrPtr == NULL || pstTramp->pvTrampoline == NULL )
		{
			OutputPrint( FALSE, FALSE, "Too many detours." );
			return FALSE;
		}
	}

	// === Install the Detour / Breakpoint. ===

	// Structures Init (Detour Only).

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		memset( & dtjiJumpInfo, 0, sizeof( dtjiJumpInfo ) );
		memset( pdSlot, 0, sizeof( DETOUR ) );

		* pstTramp->ppvOrigFnPtrPtr = (PVOID) pidobParams->dwAddress;
	}

	// Install.

	if ( pbKpeb )
	{
		if ( pidobParams->bUseSEH )
			KeAttachProcess( (PEPROCESS) pbKpeb );
		else
			dwAttchProcCookie = VpcICEAttachProcess(
				pbKpeb );
	}

	//

	if ( pidobParams->bUseSEH == FALSE &&
		IsPagePresent_BYTERANGE( (BYTE*) pidobParams->dwAddress,
		( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ? MACRO_DETOUR_MAXSIZE : /*MACRO_BREAKPOINT_SIZE*/ MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE ) ) == FALSE )
			bMemCheckFailed = TRUE;
	else
			bMemCheckFailed = FALSE;

	//

	if ( bMemCheckFailed == FALSE )
	{
		if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
		{
			// Check if the Detour is Detoured...

			if ( (DWORD) DetourGetFinalCode( (BYTE*) pidobParams->dwAddress, FALSE ) != pidobParams->dwAddress )
				bDetouredDetour = TRUE;

			if ( bDetouredDetour == FALSE )
			{
				// Fill the vbVerification Array.

				memcpy( ciContextInfo.vbVerification, (PVOID) pidobParams->dwAddress, MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE );

				// Take a Snapshot of the Memory.

				memcpy( (PVOID) & g_vbDetourMemSnapshot, (PVOID) pidobParams->dwAddress, MACRO_DETOUR_MAXSIZE );

				// Install the Detour.

				if ( pidobParams->bUseSEH )
					bRetVal = InstallDetourImplSEH( pstTramp->pvTrampoline, pidobParams->pvNewFunc, & dtjiJumpInfo );
				else
					bRetVal = DetourFunctionWithTrampoline( (PBYTE) pstTramp->pvTrampoline, (PBYTE) pidobParams->pvNewFunc, & dtjiJumpInfo );
			}
		}
		else // ### ...Breakpoint case... ###
		{
			// Fill the vbVerification Array.

			memcpy( ciContextInfo.vbVerification, (PVOID) pidobParams->dwAddress, MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE );

			// Install the Breakpoint.

			bPrevCodeByte = * (BYTE*) pidobParams->dwAddress;
		}
	}

	//

	if ( pbKpeb )
	{
		if ( pidobParams->bUseSEH )
			KeDetachProcess();
		else
			VpcICEDetachProcess(
				dwAttchProcCookie );
	}

	// Ret Val Check.

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		if ( bDetouredDetour )
		{
			OutputPrint( FALSE, FALSE, "Detoured detour... Put it on jump dest." );
			return FALSE;
		}
	}

	if ( bMemCheckFailed )
	{
		OutputPrint( FALSE, FALSE, "Memory not committed or not paged-in." );
		return FALSE;
	}

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		if ( bRetVal == FALSE )
		{
			OutputPrint( FALSE, FALSE, "Error disassembling." );
			return FALSE;
		}

		if ( dtjiJumpInfo.cbCode < 5 || dtjiJumpInfo.cbCode > MACRO_DETOUR_MAXSIZE )
		{
			OutputPrint( FALSE, FALSE, "Error disassembling." );
			return FALSE;
		}
	}

	// Processor EIPs Check (...Detour Only...).

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		if ( pidobParams->bUseSEH == FALSE )
		{
			for ( ulI = 0; ulI < pidobParams->psisSysInterrStatus->dwNumberOfProcessors; ulI ++ )
			{
				px86ccThis = & pidobParams->psisSysInterrStatus->vx86ccProcessors[ ulI ];

				// Process Check.

				if ( strlen( ciContextInfo.szProcessName ) )
				{
					if ( (BYTE*) GetCurrentKPEB( px86ccThis->pvPCRBase ) != pbKpeb )
						continue;
				}

				// Address Check.

				if ( px86ccThis->x86vicContext.EIP > pidobParams->dwAddress &&
					px86ccThis->x86vicContext.EIP < pidobParams->dwAddress + dtjiJumpInfo.cbCode )
				{
					OutputPrint( FALSE, FALSE, "Ip of processor %x is on detour body.", ulI );
					return FALSE;
				}
			}
		}
	}

	// === Check whether the Detour/Breakpoint is a Duplicate. ===

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
		{
			pdThis = & pidobParams->psisSysInterrStatus->vdDetours[ ulI ];
			if ( pdThis->ulType == MACRO_DETOURTYPE_UNUSED )
				continue;

			if ( pdThis->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
			{
				if ( pidobParams->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
				{
					if ( AreMemoryRangesConflicting(
						pidobParams->dwAddress, dtjiJumpInfo.cbCode,
						pdThis->dwAddress, pdThis->ulDetourSize ) )
					{
						OutputPrint( FALSE, FALSE, "Duplicate detour or address conflict." );
						return FALSE;
					}
				}
			}
			else // User Space.
			{
				if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // User Space.
				{
					if ( AreContextesConflicting(
						& ciContextInfo, dtjiJumpInfo.cbCode,
						& pdThis->ciContext, pdThis->ulDetourSize ) )
					{
						OutputPrint( FALSE, FALSE, "Duplicate detour or address conflict." );
						return FALSE;
					}
				}
			}
		}
	}
	else // ### ...Breakpoint case... ###
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & pidobParams->psisSysInterrStatus->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_EXEC )
				continue;

			if ( pbThis->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
			{
				if ( pidobParams->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
				{
					if ( AreMemoryRangesConflicting(
						pidobParams->dwAddress, 1,
						pbThis->dwAddress, 1 ) )
					{
						// Delete the Breakpoint and Return.

						DeleteBreakpoint( pbThis );
						OutputPrint( FALSE, FALSE, "Breakpoint %i deleted.", ulI );

						return TRUE;
					}
				}
			}
			else // User Space.
			{
				if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // User Space.
				{
					if ( AreContextesConflicting(
						& ciContextInfo, 1,
						& pbThis->ciContext, 1 ) )
					{
						// Delete the Breakpoint and Return.

						DeleteBreakpoint( pbThis );
						OutputPrint( FALSE, FALSE, "Breakpoint %i deleted.", ulI );

						return TRUE;
					}
				}
			}
		}
	}

	// === Check whether there is a conflict between a Detour and a Breakpoint. ===

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
		{
			pbThis = & pidobParams->psisSysInterrStatus->vbpBreakpoints[ ulI ];
			if ( pbThis->ulType != MACRO_BREAKPOINTTYPE_EXEC )
				continue;

			if ( pbThis->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
			{
				if ( pidobParams->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
				{
					if ( AreMemoryRangesConflicting(
						pidobParams->dwAddress, dtjiJumpInfo.cbCode,
						pbThis->dwAddress, 1 ) )
					{
						OutputPrint( FALSE, FALSE, "Detour/breakpoint conflict." );
						return FALSE;
					}
				}
			}
			else // User Space.
			{
				if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // User Space.
				{
					if ( AreContextesConflicting(
						& ciContextInfo, dtjiJumpInfo.cbCode,
						& pbThis->ciContext, 1 ) )
					{
						OutputPrint( FALSE, FALSE, "Detour/breakpoint conflict." );
						return FALSE;
					}
				}
			}
		}
	}
	else // ### ...Breakpoint case... ###
	{
		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
		{
			pdThis = & pidobParams->psisSysInterrStatus->vdDetours[ ulI ];
			if ( pdThis->ulType == MACRO_DETOURTYPE_UNUSED )
				continue;

			if ( pdThis->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
			{
				if ( pidobParams->dwAddress > (DWORD) g_pvMmUserProbeAddress ) // Kernel Space.
				{
					if ( AreMemoryRangesConflicting(
						pidobParams->dwAddress, MACRO_BREAKPOINT_SIZE,
						pdThis->dwAddress, pdThis->ulDetourSize ) )
					{
						OutputPrint( FALSE, FALSE, "Detour/breakpoint conflict." );
						return FALSE;
					}
				}
			}
			else // User Space.
			{
				if ( pidobParams->dwAddress < (DWORD) g_pvMmUserProbeAddress ) // User Space.
				{
					if ( AreContextesConflicting(
						& ciContextInfo, MACRO_BREAKPOINT_SIZE,
						& pdThis->ciContext, pdThis->ulDetourSize ) )
					{
						OutputPrint( FALSE, FALSE, "Detour/breakpoint conflict." );
						return FALSE;
					}
				}
			}
		}
	}

	// === Set up the Infos in the Structure. ===

	if ( pidobParams->ulObjectType == MACRO_IDOB_OBJECTTYPE_DETOUR ) // ### ...Detour case... ###
	{
		// Setup the Structure.

		pdSlot->ulType = pidobParams->ulType;
		pdSlot->bInstalled = FALSE;
		pdSlot->ciContext = ciContextInfo;
		pdSlot->dwAddress = pidobParams->dwAddress;
		pdSlot->ulDetourSize = dtjiJumpInfo.cbCode;
		memcpy( pdSlot->vbPrevCodeBytes, (VOID*) & g_vbDetourMemSnapshot, pdSlot->ulDetourSize );
		pdSlot->pstTrampolineInf = pstTramp;

		if ( pidobParams->ulType != MACRO_DETOURTYPE_USER )
		{
			// The Detour is a JUMP instruction.

			pdSlot->vbJumpCodeBytes[ 0 ] = 0xE9;
			* (DWORD*) & pdSlot->vbJumpCodeBytes[ 1 ] = (DWORD) dtjiJumpInfo.pbDest - (DWORD) dtjiJumpInfo.pbCode - 5;
			ulStartByteIndex = 5;
		}
		else
		{
			// The Detour is effectively a Breakpoint...

			ulStartByteIndex = 0;
		}

		for ( ulI = ulStartByteIndex; ulI < pdSlot->ulDetourSize; ulI ++ )
			pdSlot->vbJumpCodeBytes[ ulI ] = 0xCC;

		if ( pidobParams->bUseSEH == FALSE )
		{
			pdSlot->bIsContextCompatible = IsContextCompatible(
				& pdSlot->ciContext, & pdSlot->dwAddress,
				NULL, pidobParams->psisSysInterrStatus->vx86ccProcessors[ pidobParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase,
				& iccciCacheInfos, NULL, 0 );
		}
	}
	else // ### ...Breakpoint case... ###
	{
		// Search for a Free Slot.

		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
			if ( pidobParams->psisSysInterrStatus->vbpBreakpoints[ ulI ].ulType == MACRO_BREAKPOINTTYPE_UNUSED )
			{
				pbSlot = & pidobParams->psisSysInterrStatus->vbpBreakpoints[ ulI ];
				break;
			}

		if ( pbSlot == NULL )
		{
			OutputPrint( FALSE, FALSE, "Too many breakpoints." );
			return FALSE;
		}

		// Zero the Memory.

		memset( pbSlot, 0, sizeof( BREAKPOINT ) );

		// Setup the Structure.

		pbSlot->ulType = pidobParams->ulType;
		pbSlot->bInstalled = FALSE;
		pbSlot->ciContext = ciContextInfo;
		pbSlot->dwAddress = pidobParams->dwAddress;
		pbSlot->vbPrevCodeBytes[ 0 ] = bPrevCodeByte;
		pbSlot->bIsUsingDebugRegisters = ( pidobParams->ulDebugReg == 0xFFFFFFFF ? FALSE : TRUE );
		pbSlot->ulDebugRegisterNum = pidobParams->ulDebugReg;

		if ( pidobParams->bUseSEH == FALSE )
		{
			pbSlot->bIsContextCompatible = IsContextCompatible(
				& pbSlot->ciContext, & pbSlot->dwAddress,
				NULL, pidobParams->psisSysInterrStatus->vx86ccProcessors[ pidobParams->psisSysInterrStatus->dwCurrentProcessor ].pvPCRBase,
				& iccciCacheInfos, NULL, 0 );
		}
	}

	// === Return to the Caller. ===

	return bRetVal;
}

//========================================
// EnableCopyOnWrite Function Definition.
//========================================

DWORD EnableCopyOnWrite( IN BOOLEAN bEnable, IN DWORD dwEnableCookie )
{
	// Disable or Enable the Copy on Write feature of the Processor.

	if ( bEnable ) // ### ENABLING ###
	{
		// Set the WP Flag.

		if ( dwEnableCookie & 2 )
		{
			__asm
			{
				mov			eax, cr0
				or			eax, MACRO_CR0_WP_MASK
				mov			cr0, eax
			}
		}

		// Enable the Interrupts.

		if ( dwEnableCookie & 1 )
		{
			__asm		sti;
		}

		// Return.

		return 0;
	}
	else // ### DISABLING ###
	{
		DWORD			dwCookie = 0;
		DWORD			dwEFlags;
		DWORD			dwCR0;

		// Read the EFLAGS.

		__asm
		{
			pushfd
			pop			eax
			mov			dwEFlags, eax
		}

		// Disable the Interrupts.

		if ( dwEFlags & MACRO_IF_MASK )
		{
			dwCookie |= 1;
			__asm		cli;
		}

		// Clear the WP flag.

		__asm
		{
			mov			eax, cr0
			mov			dwCR0, eax
		}

		if ( dwCR0 & MACRO_CR0_WP_MASK )
		{
			dwCookie |= 2;
			dwCR0 &= ~ MACRO_CR0_WP_MASK;

			__asm
			{
				mov			eax, dwCR0
				mov			cr0, eax
			}
		}

		// Return.

		return dwCookie;
	}
}

//===========================================
// RegenerateTrampoline Function Definition.
//===========================================

VOID RegenerateTrampoline( IN PVOID pvTrampAddress, IN PVOID pvGetVaFuncAddress )
{
	BYTE*		pbPtr = (BYTE*) pvTrampAddress;
	ULONG		ulI;

	// Recreate the Trampoline Data.

	for ( ulI = 0; ulI < 2; ulI ++ )
		* pbPtr ++ = 0x90;

	* pbPtr ++ = 0xE8;

	* (DWORD*) pbPtr = (DWORD) pvGetVaFuncAddress - (DWORD) pbPtr - 4;
	pbPtr += sizeof( DWORD );

	* pbPtr ++ = 0xFF;
	* pbPtr ++ = 0xE0;

	* pbPtr ++ = 0xC3;

	for ( ulI = 0; ulI < 22; ulI ++ )
		* pbPtr ++ = 0x90;

	// Return to the Caller.

	return;
}

//===========================================================
// GetUserSpaceTrampolineStructuresSize Function Definition.
//===========================================================

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Layout of "User Space Trampoline Structures" data:
//
//  string "xBUGCHECKERTRAMPOLINESTRUCTURESx"	= 32 Byte Magic String. (MACRO_USRSPACE_TRAMPS_MAGIC_STR)
//  CC CC CC ... x EntriesNum					= ReturnDetour gates to Kernel Mode.
//  PVOID PVOID PVOID ... x EntriesNum			= Detour Original Function address (varx).
//  ( A1 varx_Address C3 ) x EntriesNum			= "_Detours_GetVA_##target" Function. LEN = 6 Bytes.
//  trampoline x EntriesNum						= Detour Trampolines. LEN = MACRO_TRAMPOLINE_SIZE.
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ULONG GetUserSpaceTrampolineStructuresSize( ULONG ulEntriesNum /* ### ...THIS IS FIXED TO MACRO_MAXNUM_OF_DETOURS... ### */ )
{
	// Return the Information.

	return MACRO_USRSPACE_TRAMPS_MAGIC_STR_LEN +
		ulEntriesNum * sizeof( BYTE ) +
		ulEntriesNum * sizeof( PVOID ) +
		ulEntriesNum * 6 +
		ulEntriesNum * MACRO_TRAMPOLINE_SIZE;
}

//==============================================================
// InitializeUserSpaceTrampolineStructures Function Definition.
//==============================================================

static BYTE*		g_pbUsrSpcTramps_Start = NULL;
static BYTE*		g_pbUsrSpcTramps_ReturnDetourGates = NULL;
static BYTE*		g_pbUsrSpcTramps_OrigFuncAddresses = NULL;
static BYTE*		g_pbUsrSpcTramps_DetGetVAFuncs = NULL;
static BYTE*		g_pbUsrSpcTramps_Trampolines = NULL;

VOID InitializeUserSpaceTrampolineStructures( IN VOID* pvMemPtr )
{
	BYTE*		pbPtr = (BYTE*) pvMemPtr;
	ULONG		ulI;
	DWORD		dwAddress;
	BOOLEAN		bFillStaticTramps = ( g_pbUsrSpcTramps_Start == NULL );

	// Initialize the Memory.

	g_pbUsrSpcTramps_Start = pbPtr;
	memcpy( pbPtr, MACRO_USRSPACE_TRAMPS_MAGIC_STR, MACRO_USRSPACE_TRAMPS_MAGIC_STR_LEN );
	pbPtr += MACRO_USRSPACE_TRAMPS_MAGIC_STR_LEN;

	g_pbUsrSpcTramps_ReturnDetourGates = pbPtr;
	memset( pbPtr, MACRO_BREAKPOINT_OPCODE, MACRO_MAXNUM_OF_DETOURS );
	pbPtr += MACRO_MAXNUM_OF_DETOURS;

	g_pbUsrSpcTramps_OrigFuncAddresses = pbPtr;
	memset( pbPtr, 0x0, MACRO_MAXNUM_OF_DETOURS * sizeof( PVOID ) );
	pbPtr += MACRO_MAXNUM_OF_DETOURS * sizeof( PVOID );

	g_pbUsrSpcTramps_DetGetVAFuncs = pbPtr;
	dwAddress = (DWORD) g_pbUsrSpcTramps_OrigFuncAddresses;
	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
	{
		* pbPtr ++ = 0xA1;

		* (DWORD*) pbPtr = dwAddress;
		pbPtr += sizeof( DWORD );
		dwAddress += sizeof( PVOID );

		* pbPtr ++ = 0xC3;
	}

	g_pbUsrSpcTramps_Trampolines = pbPtr;
	dwAddress = (DWORD) g_pbUsrSpcTramps_DetGetVAFuncs;
	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
	{
		RegenerateTrampoline( pbPtr, (PVOID) dwAddress );
		pbPtr += MACRO_TRAMPOLINE_SIZE;
		dwAddress += 6;
	}

	// Fill the Static Trampolines Structure, if this is the first time here.

	if ( bFillStaticTramps )
	{
		STATIC_TRAMPOLINE*		pstThis;
		DWORD					dwAddress01 = (DWORD) g_pbUsrSpcTramps_OrigFuncAddresses;
		DWORD					dwAddress02 = (DWORD) g_pbUsrSpcTramps_Trampolines;
		DWORD					dwAddress03 = (DWORD) g_pbUsrSpcTramps_DetGetVAFuncs;

		for ( ulI = 0; ulI < MACRO_MAXNUM_OF_DETOURS; ulI ++ )
		{
			pstThis = & g_vstStaticTrampolines_USER[ ulI ];

			pstThis->ppvOrigFnPtrPtr = (PVOID*) dwAddress01;
			pstThis->pvTrampoline = (VOID*) dwAddress02;
			pstThis->pvGetVaFuncAddress = (VOID*) dwAddress03;

			dwAddress01 += sizeof( PVOID );
			dwAddress02 += MACRO_TRAMPOLINE_SIZE;
			dwAddress03 += 6;
		}
	}

	// Return to the Caller.

	return;
}

//================================================
// InitializeUserSpaceMemory Function Definition.
//================================================

VOID InitializeUserSpaceMemory( IN VOID* pvMemPtr )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	BYTE*							pbPtr = (BYTE*) pvMemPtr;

	// Initialize the Memory.

	memset( pbPtr, 0, extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresSize );
	memcpy( pbPtr, MACRO_USRSPACE_MEMORY_MAGIC_STR, MACRO_USRSPACE_MEMORY_MAGIC_STR_LEN );

	// Return to the Caller.

	return;
}

//=============================================
// GetUserSpaceMemorySize Function Definition.
//=============================================

ULONG GetUserSpaceMemorySize( VOID )
{
	// Return the Memory Size.

	return MACRO_USRSPACE_MEMORY_SIZE;
}

//============================================================
// AllocateUserSpaceTrampolineStructures Function Definition.
//============================================================

BOOLEAN AllocateUserSpaceTrampolineStructures( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN BYTE* pbKpeb, IN DWORD dwProcessId )
{
	BOOLEAN				bRetVal = TRUE;
	NTSTATUS			ntStatus;
	PVOID				pvBaseAddress;
	ULONG				ulSize;
	DWORD				dwCookie;

	// Get the KPEB of the Specified Process.

	if ( pbKpeb == NULL )
	{
		if ( dwProcessId == 0 )
		{
			return FALSE;
		}
		else
		{
			pbKpeb = NULL;

			ntStatus = PsLookupProcessByProcessId(
				dwProcessId, & pbKpeb );

			if ( ntStatus != STATUS_SUCCESS || pbKpeb == NULL )
			{
				return FALSE;
			}
		}
	}

	// Allocate the Memory in the Specified Process.

	KeAttachProcess( (PEPROCESS) pbKpeb );

		pvBaseAddress = (PVOID) psisSysInterrStatus->dwUserSpaceTrampolineStructuresAddress;
		ulSize = (ULONG) psisSysInterrStatus->dwUserSpaceTrampolineStructuresSize;

		ntStatus = ZwAllocateVirtualMemory( NtCurrentProcess(),
			& pvBaseAddress,
			0,
			& ulSize,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_EXECUTE_READ | PAGE_NOCACHE );

		if ( ntStatus != STATUS_SUCCESS )
		{
			bRetVal = FALSE;
		}
		else if ( pvBaseAddress && pvBaseAddress != (PVOID) psisSysInterrStatus->dwUserSpaceTrampolineStructuresAddress )
		{
			ZwFreeVirtualMemory( NtCurrentProcess(),
				& pvBaseAddress,
				& ulSize,
				MEM_RELEASE );

			bRetVal = FALSE;
		}
		else
		{
			// Let the memory to be Allocated.

			TouchPage_BYTERANGE( (BYTE*) pvBaseAddress, ulSize );

			// Write the data related to the User Trampolines.

			dwCookie = EnableCopyOnWrite( FALSE, 0 );
			InitializeUserSpaceTrampolineStructures( pvBaseAddress );
			EnableCopyOnWrite( TRUE, dwCookie );
		}

	KeDetachProcess();

	// Return to the Caller.

	return bRetVal;
}

//==============================================
// AllocateUserSpaceMemory Function Definition.
//==============================================

BOOLEAN AllocateUserSpaceMemory( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN BYTE* pbKpeb, IN DWORD dwProcessId )
{
	BOOLEAN				bRetVal = TRUE;
	NTSTATUS			ntStatus;
	PVOID				pvBaseAddress;
	ULONG				ulSize;
	DWORD				dwCookie;

	// Get the KPEB of the Specified Process.

	if ( pbKpeb == NULL )
	{
		if ( dwProcessId == 0 )
		{
			return FALSE;
		}
		else
		{
			pbKpeb = NULL;

			ntStatus = PsLookupProcessByProcessId(
				dwProcessId, & pbKpeb );

			if ( ntStatus != STATUS_SUCCESS || pbKpeb == NULL )
			{
				return FALSE;
			}
		}
	}

	// Allocate the Memory in the Specified Process.

	KeAttachProcess( (PEPROCESS) pbKpeb );

		pvBaseAddress = (PVOID) psisSysInterrStatus->dwUserSpaceTrampolineStructuresAddress;
		ulSize = (ULONG) psisSysInterrStatus->dwUserSpaceTrampolineStructuresSize;

		ntStatus = ZwAllocateVirtualMemory( NtCurrentProcess(),
			& pvBaseAddress,
			0,
			& ulSize,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_EXECUTE_READ | PAGE_NOCACHE );

		if ( ntStatus != STATUS_SUCCESS )
		{
			bRetVal = FALSE;
		}
		else if ( pvBaseAddress && pvBaseAddress != (PVOID) psisSysInterrStatus->dwUserSpaceTrampolineStructuresAddress )
		{
			ZwFreeVirtualMemory( NtCurrentProcess(),
				& pvBaseAddress,
				& ulSize,
				MEM_RELEASE );

			bRetVal = FALSE;
		}
		else
		{
			// Let the memory to be Allocated.

			TouchPage_BYTERANGE( (BYTE*) pvBaseAddress, ulSize );

			// Write the data related to the User Memory.

			dwCookie = EnableCopyOnWrite( FALSE, 0 );
			InitializeUserSpaceMemory( pvBaseAddress );
			EnableCopyOnWrite( TRUE, dwCookie );
		}

	KeDetachProcess();

	// Return to the Caller.

	return bRetVal;
}

//==========================================================================
// AllocateUserSpaceTrampolineStructuresInAllProcesses Function Definition.
//==========================================================================

static BOOLEAN AllocateUserSpaceTrampolineStructuresInAllProcesses_CBfn( BYTE* pbKpeb, DWORD dwParam )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bAllocRes;
	CHAR*					pszCurrentProcessName;
	CHAR					szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
	CHAR*					pszDot;

	// Allocate the Memory in this Process.

	bAllocRes = /* AllocateUserSpaceTrampolineStructures( & extension->sisSysInterrStatus,
		pbKpeb, 0 ); */ // DETOURLESS
		AllocateUserSpaceMemory( & extension->sisSysInterrStatus,
		pbKpeb, 0 );

	// Get the Name of the Process and Display a Message.

	pszCurrentProcessName = (CHAR*) ( pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );
	memset( & szProcessName, 0, sizeof( szProcessName ) );
	memcpy( & szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

	pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
	if ( pszDot )
		* pszDot = 0;

	if ( bAllocRes )
		OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": UsrSpaceOK -> KPEB=%X  NAME=%s  ADDR(FXD)=%X",
			(DWORD) pbKpeb, szProcessName, extension->sisSysInterrStatus.dwUserSpaceTrampolineStructuresAddress );
	else
		OutputPrint( FALSE, TRUE, MACRO_PROGRAM_NAME ": UsrSpaceAllocERR -> KPEB=%X  NAME=%s",
			(DWORD) pbKpeb, szProcessName );

	// Return to the Caller.

	return TRUE;
}

VOID AllocateUserSpaceTrampolineStructuresInAllProcesses( VOID )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;

	// Iterate through the list of System Processes.

	IterateThroughListOfProcesses( & extension->pliProcessListInfo,
		& AllocateUserSpaceTrampolineStructuresInAllProcesses_CBfn, 0 );

	// Return to the Caller.

	return;
}

//=======================================
// DeleteBreakpoint Function Definition.
//=======================================

VOID DeleteBreakpoint( IN BREAKPOINT* pbBP )
{
	// Delete the Breakpoint.

	pbBP->ulType = MACRO_BREAKPOINTTYPE_UNUSED;

	// Return.

	return;
}

//===================================
// DeleteDetour Function Definition.
//===================================

VOID DeleteDetour( IN DETOUR* pdDT )
{
	// Delete the Detour.

	pdDT->ulType = MACRO_DETOURTYPE_UNUSED;

	// Return.

	return;
}

//=======================================================
// ContextProcName2KpebLikeProcName Function Definition.
//=======================================================

BOOLEAN ContextProcName2KpebLikeProcName( OUT CHAR* pszKpebLikeProcName, IN CHAR* pszContextProcName )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN					bCopyStrings = TRUE;
	BOOLEAN					bReturnValue = TRUE;

	// Check whether we have here an Hex Number.

	if ( strlen( pszContextProcName ) == MACRO_CONTEXTINFO_PROCESSNAME_HEXFORM_SIZE &&
		pszContextProcName[ 0 ] == '0' &&
		pszContextProcName[ 1 ] == 'x' )
	{
		CHAR*		pszEndPtr;
		DWORD		dwKpebHexForm = MACRO_CRTFN_NAME(strtoul)(
			pszContextProcName, & pszEndPtr, 16 );
		if ( dwKpebHexForm > (DWORD) g_pvMmUserProbeAddress &&
			( pszEndPtr == NULL || * pszEndPtr == 0 ) )
		{
			GETKPEBPTRFROMPROCNAMECB_INFOS			ckpiCbInfos;

			bCopyStrings = FALSE;
			strcpy( pszKpebLikeProcName, "" );

			// Check whether the KPEB belongs to an Active Process.

			memset( & ckpiCbInfos, 0, sizeof( ckpiCbInfos ) );
			ckpiCbInfos.pszProcName = pszContextProcName;
			ckpiCbInfos.pbKpeb = NULL;
			ckpiCbInfos.pbLastKpeb = NULL;

			IterateThroughListOfProcesses( & extension->pliProcessListInfo,
				& GetKpebPtrFromProcNameCB, (DWORD) & ckpiCbInfos );

			if ( ckpiCbInfos.pbKpeb )
			{
				CHAR			szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
				CHAR*			pszCurrentProcessName;
				CHAR*			pszDot;

				// Return the Name of the Process.

				pszCurrentProcessName = (CHAR*) ( ckpiCbInfos.pbKpeb + MACRO_IMGFLNAME_FIELDOFFSET_IN_KPEB );
				memset( & szProcessName, 0, sizeof( szProcessName ) );
				memcpy( & szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );

				pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
				if ( pszDot )
					* pszDot = 0;

				strcpy( pszKpebLikeProcName, szProcessName );
			}
			else
			{
				bReturnValue = FALSE;
			}
		}
	}

	if ( bCopyStrings )
		strcpy( pszKpebLikeProcName, pszContextProcName );

	// Return.

	return bReturnValue;
}

//=========================================================================
//
// ## ## Page Frame Modifications Database structures. ## ##
//
//=========================================================================

// definitions.

#define MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM		0x4

// types.

typedef struct _PAGFRMMOD_CHANGE
{
	struct _PAGFRMMOD_CHANGE*		ppfmcNext;			// next: NULL if the list is Over.
	ULONG							ulOffset;			// offset: offset from the Frame Beginning.
	BYTE							vbChanges[			//
		MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM ];	// changes: all unused bytes initialized to 0xCC.
														//  If the first byte is 0xCC, the slot is Unused.
														//  The 0xCC Bytes are always the Ending Ones.
} PAGFRMMOD_CHANGE, *PPAGFRMMOD_CHANGE;

typedef struct _PAGFRMMOD_PHYSPAGE
{
	DWORD				dwPageFrameAddr;	// pageframeaddr: physical address of the Frame (aligned to 4Kb).
	PAGFRMMOD_CHANGE*	ppfmcChanges;		// changes: NULL if this slot if Available...

} PAGFRMMOD_PHYSPAGE, *PPAGFRMMOD_PHYSPAGE;

// sizes.

#define MACRO_PAGFRMMOD_CHANGE_DATA_SIZEINBYTES			0x10000
#define MACRO_PAGFRMMOD_PHYSPAGE_DATA_SIZEINBYTES		0x8000

#define MACRO_PAGFRMMOD_CHANGE_DATA_MAXITEMS			( MACRO_PAGFRMMOD_CHANGE_DATA_SIZEINBYTES / sizeof( PAGFRMMOD_CHANGE ) )
#define MACRO_PAGFRMMOD_PHYSPAGE_DATA_MAXITEMS			( MACRO_PAGFRMMOD_PHYSPAGE_DATA_SIZEINBYTES / sizeof( PAGFRMMOD_PHYSPAGE ) )

// data.

static PAGFRMMOD_CHANGE			g_vpfmcChanges[ MACRO_PAGFRMMOD_CHANGE_DATA_MAXITEMS ];
static PAGFRMMOD_PHYSPAGE		g_vpfmppPhysPages[ MACRO_PAGFRMMOD_PHYSPAGE_DATA_MAXITEMS ];

//==============================================
// Page Frame Modifications Database functions.
//==============================================

VOID InitializePFMDB( VOID )
{
	ULONG				ulI;
	PAGFRMMOD_CHANGE*	ppfmcThis;

	// Initialize the Structures.

	for ( ulI = 0; ulI < MACRO_PAGFRMMOD_CHANGE_DATA_MAXITEMS; ulI ++ )
	{
		ppfmcThis = & g_vpfmcChanges[ ulI ];

		memset( & ppfmcThis->vbChanges, 0xCC, MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM );
	}

	// Return.

	return;
}

static PAGFRMMOD_CHANGE* GetChangeFreeSlot( IN OUT PAGFRMMOD_CHANGE** pppfmcLookupHelper )
{
	ULONG				ulI, ulStart;
	PAGFRMMOD_CHANGE*	ppfmcRetVal;

	// Search for a Free Slot.

	if ( pppfmcLookupHelper == NULL || * pppfmcLookupHelper == NULL )
		ulStart = 0;
	else
		ulStart = * pppfmcLookupHelper - & g_vpfmcChanges[ 0 ];

	for ( ulI = ulStart; ulI < MACRO_PAGFRMMOD_CHANGE_DATA_MAXITEMS; ulI ++ )
		if ( g_vpfmcChanges[ ulI ].vbChanges[ 0 ] == 0xCC )
		{
			ppfmcRetVal = & g_vpfmcChanges[ ulI ];

			// Return Success.

			if ( pppfmcLookupHelper )
				* pppfmcLookupHelper = ppfmcRetVal;

			return ppfmcRetVal;
		}

	// Return Failure.

	if ( pppfmcLookupHelper )
		* pppfmcLookupHelper = NULL;

	return NULL;
}

static BOOLEAN AddFrameToPFMDB_impl( IN VOID* pvVirtualAddress, IN ULONG ulBytesNum )
{
	NTSTATUS			ntStat;
	DWORD				dwPhysAddress;
	DWORD				dwPageFrameAddr, dwPageFrameOffset;
	PAGFRMMOD_PHYSPAGE*	ppfmppPageFrame = NULL;
	PAGFRMMOD_PHYSPAGE*	ppfmppFirstFree = NULL;
	ULONG				ulI;
	BYTE				vbChangeMemMap[ MACRO_PFMDB_ADDFRAMETOPFMDB_MAXBYTESNUM ];
	PAGFRMMOD_CHANGE*	ppfmcItem;
	ULONG				ulChangeBytesNum;
	DWORD				dwAddress;
	PAGFRMMOD_CHANGE*	ppfmcLookupHelper = NULL;
	PAGFRMMOD_CHANGE*	ppfmcListTail = NULL;
	ULONG				ulStart;
	DWORD				dwOffPage0, dwOffPage1;
	DWORD				dwSize;
	BOOLEAN				bRetVal0, bRetVal1;

	// Parameters Validation.

	if ( pvVirtualAddress == NULL )
		return FALSE;
	else if ( ulBytesNum == 0 )
		return TRUE;
	else if ( ulBytesNum > MACRO_PFMDB_ADDFRAMETOPFMDB_MAXBYTESNUM )
		return FALSE;

	//
	// - - - - - - - - - - >
	// SPECIAL CONDITION: the Memory Range spans TWO Pages.
	// - - - - - - - - - - >
	//

	dwOffPage0 = ((DWORD)pvVirtualAddress) & 0x00000FFF;
	dwOffPage1 = ((DWORD)pvVirtualAddress+ulBytesNum-1) & 0x00000FFF;

	if ( dwOffPage0 > dwOffPage1 )
	{
		dwSize = 0x1000 - dwOffPage0;

		bRetVal0 = AddFrameToPFMDB_impl( pvVirtualAddress, dwSize );
		bRetVal1 = AddFrameToPFMDB_impl( (PVOID) ( (BYTE*)pvVirtualAddress+dwSize ), ulBytesNum - dwSize );

		return bRetVal0 || bRetVal1;
	}

	// Get the Page Frame Address.

	if ( IsPagePresent_BYTERANGE( (BYTE*) pvVirtualAddress, ulBytesNum ) == FALSE )
		return FALSE;

	ntStat = LinearAddressToPhysAddress( & dwPhysAddress, (DWORD) pvVirtualAddress );
	if ( ntStat != STATUS_SUCCESS )
		return FALSE;

	dwPageFrameAddr = dwPhysAddress & 0xFFFFF000;
	dwPageFrameOffset = dwPhysAddress & 0x00000FFF;

	// Take care of the Memory Map.

	memset( vbChangeMemMap, 0x00, ulBytesNum );
	memset( vbChangeMemMap + ulBytesNum, 0xFF, MACRO_PFMDB_ADDFRAMETOPFMDB_MAXBYTESNUM - ulBytesNum );

	for ( ulI = 0; ulI < ulBytesNum; ulI ++ )
		if ( * ((BYTE*) pvVirtualAddress + ulI) == 0xCC )
		{
			vbChangeMemMap[ ulI ] = 0xFF;
		}

	//
	// Check whether the Page Frame is already in the Data Base.
	// If not, allocate a new Entry for the Physical Page.
	//

	for ( ulI = 0; ulI < MACRO_PAGFRMMOD_PHYSPAGE_DATA_MAXITEMS; ulI ++ )
	{
		if ( g_vpfmppPhysPages[ ulI ].ppfmcChanges ) // ### slot allocated ###
		{
			if ( g_vpfmppPhysPages[ ulI ].dwPageFrameAddr == dwPageFrameAddr )
			{
				ppfmppPageFrame = & g_vpfmppPhysPages[ ulI ];
				break;
			}
		}
		else // ### slot free ###
		{
			if ( ppfmppFirstFree == NULL )
			{
				ppfmppFirstFree = & g_vpfmppPhysPages[ ulI ];
			}
		}
	}

	if ( ppfmppPageFrame == NULL )
	{
		// There is a Free Slot for us to Use ??

		if ( ppfmppFirstFree == NULL )
			return FALSE;

		// This will be our Phys Page slot...

		ppfmppPageFrame = ppfmppFirstFree;
	}

	//
	// If the Page Frame is already in the Data Base, check whether this Memory Change was already
	//  tracked (fully or partially).
	//

	if ( ppfmppPageFrame->ppfmcChanges )
	{
		//
		// Iterate through the Linked List of Changes,
		//  filling the Memory Map.
		//

		ppfmcItem = ppfmppPageFrame->ppfmcChanges;

		do
		{
			// Is this Item referring to one or more bytes in the Passed Interval ?

			ulChangeBytesNum = 0;
			for ( ulI = 0; ulI < MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM; ulI ++ )
				if ( ppfmcItem->vbChanges[ ulI ] != 0xCC )
					ulChangeBytesNum ++;
				else
					break;

			if ( AreMemoryRangesConflicting( dwPageFrameOffset, ulBytesNum,
				ppfmcItem->ulOffset, ulChangeBytesNum ) )
			{
				// Mark the Taken Bytes in the Memory Map.

				for ( ulI = 0; ulI < ulChangeBytesNum; ulI ++ )
				{
					dwAddress = ppfmcItem->ulOffset + ulI;

					if ( dwAddress >= dwPageFrameOffset &&
						dwAddress < dwPageFrameOffset + ulBytesNum )
					{
						vbChangeMemMap[ dwAddress - dwPageFrameOffset ] = 0xFF;
					}
				}
			}

			// Set the List Tail pointer.

			if ( ppfmcItem->ppfmcNext == NULL )
				ppfmcListTail = ppfmcItem;

			// Next item of the List.

			ppfmcItem = ppfmcItem->ppfmcNext;
		}
		while( ppfmcItem );
	}
	else
	{
		//
		// Set the Physical Address of the Frame.
		//

		ppfmppPageFrame->dwPageFrameAddr = dwPageFrameAddr;
	}

	//
	// According to the Change Memory Map, track the Changes in the Linked List.
	//

	for ( ulI = 0; ulI < sizeof( vbChangeMemMap ); ulI ++ )
		if ( ! vbChangeMemMap[ ulI ] )
		{
			ulStart = ulI;

			//
			// Get the Number of Bytes assigned to this Change Slot.
			//

			ulChangeBytesNum = 0;
			for ( ; ulI < sizeof( vbChangeMemMap ); ulI ++ )
				if ( ! vbChangeMemMap[ ulI ] )
				{
					if ( ++ ulChangeBytesNum == MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM )
						break;
				}
				else
				{
					break;
				}

			//
			// Set the Change Slot.
			//

			ppfmcItem = GetChangeFreeSlot( & ppfmcLookupHelper );
			if ( ppfmcItem == NULL )
				break;

			//

			if ( ppfmcListTail == NULL )
				ppfmppPageFrame->ppfmcChanges = ppfmcItem;
			else
				ppfmcListTail->ppfmcNext = ppfmcItem;

			ppfmcListTail = ppfmcItem;

			//

			ppfmcItem->ppfmcNext = NULL;
			ppfmcItem->ulOffset = (ULONG) dwPageFrameOffset + ulStart;

			memcpy( ppfmcItem->vbChanges, (BYTE*) pvVirtualAddress + ulStart, ulChangeBytesNum );
			memset( ppfmcItem->vbChanges + ulChangeBytesNum, 0xCC, MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM - ulChangeBytesNum );
		}

	// Return.

	return TRUE;
}

static BOOLEAN RemoveFrameFromPFMDB_impl( IN VOID* pvVirtualAddress, IN ULONG ulBytesNum )
{
	DWORD				dwOffPage0, dwOffPage1, dwSize;
	BOOLEAN				bRetVal0, bRetVal1;
	NTSTATUS			ntStat;
	DWORD				dwPhysAddress, dwPageFrameAddr, dwPageFrameOffset;
	ULONG				ulI;
	PAGFRMMOD_PHYSPAGE*	ppfmppThis;
	PAGFRMMOD_CHANGE*	ppfmcThis;

	// Parameters Validation.

	if ( pvVirtualAddress == NULL )
		return FALSE;
	else if ( ulBytesNum == 0 )
		return FALSE;
	else if ( ulBytesNum > MACRO_PFMDB_ADDFRAMETOPFMDB_MAXBYTESNUM )
		return FALSE;

	//
	// - - - - - - - - - - >
	// SPECIAL CONDITION: the Memory Range spans TWO Pages.
	// - - - - - - - - - - >
	//

	dwOffPage0 = ((DWORD)pvVirtualAddress) & 0x00000FFF;
	dwOffPage1 = ((DWORD)pvVirtualAddress+ulBytesNum-1) & 0x00000FFF;

	if ( dwOffPage0 > dwOffPage1 )
	{
		dwSize = 0x1000 - dwOffPage0;

		bRetVal0 = RemoveFrameFromPFMDB_impl( pvVirtualAddress, dwSize );
		bRetVal1 = RemoveFrameFromPFMDB_impl( (PVOID) ( (BYTE*)pvVirtualAddress+dwSize ), ulBytesNum - dwSize );

		return bRetVal0 || bRetVal1;
	}

	// Get the Page Frame Address.

	ntStat = LinearAddressToPhysAddress( & dwPhysAddress, (DWORD) pvVirtualAddress );
	if ( ntStat != STATUS_SUCCESS )
		return FALSE;

	dwPageFrameAddr = dwPhysAddress & 0xFFFFF000;
	dwPageFrameOffset = dwPhysAddress & 0x00000FFF;

	// Iterate through the List of Frames.

	for ( ulI = 0; ulI < MACRO_PAGFRMMOD_PHYSPAGE_DATA_MAXITEMS; ulI ++ )
		if ( g_vpfmppPhysPages[ ulI ].ppfmcChanges &&
			g_vpfmppPhysPages[ ulI ].dwPageFrameAddr == dwPageFrameAddr )
		{
			ppfmppThis = & g_vpfmppPhysPages[ ulI ];
			ppfmcThis = ppfmppThis->ppfmcChanges;

			//
			// Free the Slot.
			//

			ppfmppThis->ppfmcChanges = NULL;

			do
			{
				ppfmcThis->vbChanges[ 0 ] = 0xCC;
				ppfmcThis = ppfmcThis->ppfmcNext;
			}
			while( ppfmcThis );

			// Return SUCCESS.

			return TRUE;
		}

	// Return FAILURE.

	return FALSE;
}

static BOOLEAN CheckOutFramesInPFMDB_impl( IN DWORD dwTestPhysAddr )
{
	ULONG				ulI, ulJ;
	PAGFRMMOD_PHYSPAGE*	ppfmppThis;
	VOID*				pvMappedPage;
	DWORD				dwOldPTE;
	PAGFRMMOD_CHANGE*	ppfmcThis;
	BOOLEAN				bValidated;
	ULONG				ulChangeBytesNum;
	BOOLEAN				bReturnValue = FALSE;

	//
	// Iterate through the Frame List.
	//

	for ( ulI = 0; ulI < MACRO_PAGFRMMOD_PHYSPAGE_DATA_MAXITEMS; ulI ++ )
		if ( g_vpfmppPhysPages[ ulI ].ppfmcChanges )
		{
			ppfmppThis = & g_vpfmppPhysPages[ ulI ];

			//
			// Map the page in the Debugger Hyperspace.
			//

			pvMappedPage = MapPhysPageInDebuggerHyperSlot( ppfmppThis->dwPageFrameAddr, & dwOldPTE );

			if ( pvMappedPage )
			{
				// Check whether the Page is Validated.

				bValidated = TRUE;

				ppfmcThis = ppfmppThis->ppfmcChanges;
				do
				{
					// Get the Number of Bytes.

					ulChangeBytesNum = 0;
					for ( ulJ = 0; ulJ < MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM; ulJ ++ )
						if ( ppfmcThis->vbChanges[ ulJ ] != 0xCC )
							ulChangeBytesNum ++;
						else
							break;

					// Check the Page Frame Bytes.

					for ( ulJ = 0; ulJ < ulChangeBytesNum; ulJ ++ )
						if ( ((BYTE*)pvMappedPage)[ ppfmcThis->ulOffset + ulJ ] != 0xCC )
						{
							bValidated = FALSE;
							break;
						}

					// Next item.

					ppfmcThis = ppfmcThis->ppfmcNext;
				}
				while( ppfmcThis );

				if ( bValidated )
				{
					//
					// Restore the Page Frame Bytes.
					//

					ppfmcThis = ppfmppThis->ppfmcChanges;
					do
					{
						// Restore.

						for ( ulJ = 0; ulJ < MACRO_PAGFRMMOD_CHANGE_MAXBYTESNUM_PERITEM; ulJ ++ )
							if ( ppfmcThis->vbChanges[ ulJ ] != 0xCC )
							{
								// Restore the Byte.

								((BYTE*)pvMappedPage)[ ppfmcThis->ulOffset + ulJ ] = ppfmcThis->vbChanges[ ulJ ];

								// Check out the Test Address.

								if ( dwTestPhysAddr &&
									dwTestPhysAddr == ppfmppThis->dwPageFrameAddr + ppfmcThis->ulOffset + ulJ )
										bReturnValue = TRUE;
							}
							else
								break;

						// Next item.

						ppfmcThis = ppfmcThis->ppfmcNext;
					}
					while( ppfmcThis );
				}

				// Restore the HyperSlot.

				RestoreDebuggerHyperSlotState( dwOldPTE );
			}

			//
			// Free the Slot.
			//

			ppfmcThis = ppfmppThis->ppfmcChanges;
			ppfmppThis->ppfmcChanges = NULL;

			do
			{
				ppfmcThis->vbChanges[ 0 ] = 0xCC;
				ppfmcThis = ppfmcThis->ppfmcNext;
			}
			while( ppfmcThis );
		}

	// Return.

	return bReturnValue;
}

//====================================================================
// Syncronized "Page Frame Modifications Database" function wrappers.
//====================================================================

static DWORD AcquirePfmdbLock( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisSysInterrStatus = & extension->sisSysInterrStatus;
	DWORD					dwCookie = 0;
	DWORD					dwEFlags;

	// Read the EFLAGS.

	__asm
	{
		pushfd
		pop			eax
		mov			dwEFlags, eax
	}

	// Disable the Interrupts.

	if ( dwEFlags & MACRO_IF_MASK )
	{
		dwCookie |= 1;
		__asm		cli;
	}

	// Acquire the Spinlock.

	__asm
	{
		mov			ebx, psisSysInterrStatus
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslPfmdbSpinLock
		call		EnterMultiProcessorSpinLock
	}

	// Return.

	return dwCookie;
}

static VOID ReleasePfmdbLock( IN DWORD dwAcquireCookie )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisSysInterrStatus = & extension->sisSysInterrStatus;

	// Release the Spinlock.

	__asm
	{
		mov			ebx, psisSysInterrStatus
		lea			ebx, [ ebx ]SYSINTERRUPTS_STATUS.mpslPfmdbSpinLock
		call		LeaveMultiProcessorSpinLock
	}

	// Enable the Interrupts.

	if ( dwAcquireCookie & 1 )
	{
		__asm		sti;
	}

	// Return.

	return;
}

BOOLEAN AddFrameToPFMDB( IN VOID* pvVirtualAddress, IN ULONG ulBytesNum )
{
	BOOLEAN		bRetVal;
	DWORD		dwCookie;

	dwCookie = AcquirePfmdbLock();
	bRetVal = AddFrameToPFMDB_impl( pvVirtualAddress, ulBytesNum );
	ReleasePfmdbLock( dwCookie );

	return bRetVal;
}

BOOLEAN RemoveFrameFromPFMDB( IN VOID* pvVirtualAddress, IN ULONG ulBytesNum )
{
	BOOLEAN		bRetVal;
	DWORD		dwCookie;

	dwCookie = AcquirePfmdbLock();
	bRetVal = RemoveFrameFromPFMDB_impl( pvVirtualAddress, ulBytesNum );
	ReleasePfmdbLock( dwCookie );

	return bRetVal;
}

BOOLEAN CheckOutFramesInPFMDB( IN DWORD dwTestPhysAddr )
{
	BOOLEAN		bRetVal;
	DWORD		dwCookie;

	dwCookie = AcquirePfmdbLock();
	bRetVal = CheckOutFramesInPFMDB_impl( dwTestPhysAddr );
	ReleasePfmdbLock( dwCookie );

	return bRetVal;
}

//============================================
// TimerThreadEntryPoint Function Definition.
//============================================

VOID TimerThreadEntryPoint( IN PVOID pvParam )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*	psisSysInterrStatus = & extension->sisSysInterrStatus;

	// Infinite Loop.

	while( TRUE )
	{
		// Wait the Specified Amount of Time.

		KeDelayExecutionThread( KernelMode,
			FALSE,
			& psisSysInterrStatus->liTimerThreadWaitInterval );

		// Enter in the Debugger.

		__asm
		{
			mov			eax, MACRO_EAX_TT_SERVICE_NONE
			call		TimerThreadDebuggerEntrance
		}
	}
}

//==================================================
// TimerThreadDebuggerEntrance Function Definition.
//==================================================

VOID __declspec( naked ) TimerThreadDebuggerEntrance( VOID )
{
	__asm
	{
		int			3
		ret
	}
}

//=======================================================
// GetInterruptVectorHandlerAddress Function Definition.
//=======================================================

VOID* GetInterruptVectorHandlerAddress( IN ULONG ulVector, IN ULONG ulProcessor )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	x86_CPU_CONTEXT*				px86ccCPU;
	VOID*							pvIDTBase;
	DWORD							dw00, dw01;
	ULONG							ulI;
	IDT_HOOKEDENTRY_INFO*			piheiIDTHook;

	//
	// Validation.
	//

	if ( ulProcessor >= psisSysInterrStatus->dwNumberOfProcessors )
		return NULL;
	if ( ulVector >= 0x100 )
		return NULL;

	//
	// Check whether this is a Special Case.
	//

	if ( psisSysInterrStatus->pvLocalApicMemoryPtr &&
		ulVector == MACRO_VPCICE_IPI_INTVECTOR )
	{
		//
		// We cannot return the Address of the VpcICE IPI Handler.
		//

		return NULL;
	}
	else if ( psisSysInterrStatus->pvIoApicMemoryPtr &&
		ulVector == MACRO_IOAPIC_MOUSE_PROXY_INTVECTOR )
	{
		//
		// We cannot return the Address of our Mouse Interrupt Handler.
		//

		return NULL;
	}
	else if ( psisSysInterrStatus->pvIoApicMemoryPtr &&
		ulVector == MACRO_IOAPIC_KEYBOARD_PROXY_INTVECTOR )
	{
		//
		// We cannot return the Address of our Keyboard Interrupt Handler.
		//

		return NULL;
	}

	//
	// Check whether VpcICE hooked the Vector at IDT Level.
	//

	for ( ulI = 0; ulI < psisSysInterrStatus->ulIdtHooksNum; ulI ++ )
	{
		piheiIDTHook = & psisSysInterrStatus->viheiIdtHooks[ ulI ];

		if ( piheiIDTHook->ulVectorNum == ulVector &&
			piheiIDTHook->ulProcessorNum == ulProcessor )
		{
			return (PVOID) piheiIDTHook->dwOriginalISRAddress;
		}
	}

	//
	// Return the Information.
	//

	px86ccCPU = & psisSysInterrStatus->vx86ccProcessors[ ulProcessor ];
	pvIDTBase = px86ccCPU->pvIDTBase;

	dw00 = * (DWORD*) ( (BYTE*) pvIDTBase + ulVector * 8 );
	dw01 = * (DWORD*) ( (BYTE*) pvIDTBase + ulVector * 8 + 4 );

	return (VOID*) ( ( dw00 & 0x0000FFFF ) | ( dw01 & 0xFFFF0000 ) );
}

//================================================
// EnableInterruptBreakpoint Function Definition.
//================================================

VOID EnableInterruptBreakpoint( IN BREAKPOINT* pbBP, IN BOOLEAN bEnable )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	ULONG							ulI;
	PVOID							pvAddress;
	BOOLEAN							bEnRes;

	pbBP->bIsContextCompatible = TRUE; // ### Always set to True... ###

	//
	// Enable or Disable the Breakpoint.
	//

	if ( bEnable )
		memset( & pbBP->vbPrevCodeBytes, 0xCC, MACRO_MAX_NUM_OF_PROCESSORS );

	for ( ulI = 0; ulI < psisSysInterrStatus->dwNumberOfProcessors; ulI ++ )
	{
		pvAddress = GetInterruptVectorHandlerAddress( pbBP->dwAddress, ulI );

		if ( pvAddress && IsPagePresent_BYTERANGE( pvAddress, MACRO_BREAKPOINT_SIZE ) &&
			! ( bEnable          && * (BYTE*) pvAddress == 0xCC ) &&
			! ( bEnable == FALSE && pbBP->vbPrevCodeBytes[ ulI ] == 0xCC ) )
		{
			//
			// Enable or Disable (this Processor) Interrupt Handler Breakpoint.
			//

			if ( bEnable )
				pbBP->vbPrevCodeBytes[ ulI ] = * (BYTE*) pvAddress;

			bEnRes = EnableSingleBreakpointIMPL( pbBP, bEnable, (DWORD) pvAddress, ulI );

			if ( bEnable &&
				bEnRes == FALSE )
			{
				pbBP->vbPrevCodeBytes[ ulI ] = 0xCC;
			}
		}
	}

	pbBP->bInstalled = bEnable;

	//
	// Return.
	//

	return;
}

//===========================================================
// InitializePs2MouseKeybConsistenceSys Function Definition.
//===========================================================

VOID InitializePs2MouseKeybConsistenceSys( VOID )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	BYTE							bPrevControllerDataByte;

	// Initialize.

	__asm
	{
		nop
		nop
		in			al, 0x60
		mov			bPrevControllerDataByte, al
	}

	psisSysInterrStatus->bPs2MouseKeybConsistenceTestPassed = FALSE;
	psisSysInterrStatus->ulPrevControllerIntrCount = psisSysInterrStatus->ulMouseKeybControllerIntrCount;
	psisSysInterrStatus->bPrevControllerDataByte = bPrevControllerDataByte;

	// Return.

	return;
}

//===================================================
// CheckPs2MouseKeybConsistence Function Definition.
//===================================================

VOID CheckPs2MouseKeybConsistence( VOID )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	BYTE							bPrevControllerDataByte;

	// Check.

	if ( psisSysInterrStatus->bPs2MouseKeybConsistenceTestPassed == FALSE )
	{
		__asm
		{
			nop
			nop
			in			al, 0x60
			mov			bPrevControllerDataByte, al
		}

		if ( psisSysInterrStatus->ulPrevControllerIntrCount != psisSysInterrStatus->ulMouseKeybControllerIntrCount )
		{
			psisSysInterrStatus->bPs2MouseKeybConsistenceTestPassed = TRUE;
		}
		else if ( psisSysInterrStatus->bPrevControllerDataByte != bPrevControllerDataByte )
		{
			if ( psisSysInterrStatus->pvLocalApicMemoryPtr == NULL )
				SendEOIToPIC1();
			else
				SendEOIToLocalAPIC( psisSysInterrStatus );

			psisSysInterrStatus->bPs2MouseKeybConsistenceTestPassed = TRUE;
		}
	}

	// Return.

	return;
}

//====================================================
// GetUpdatedDebugRegisterValues Function Definition.
//====================================================

VOID GetUpdatedDebugRegisterValues( VOID )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	SYSINTERRUPTS_STATUS*			psisSysInterrStatus = & extension->sisSysInterrStatus;
	ULONG							ulI;
	BREAKPOINT*						pbThis;
	ULONG							ulRWLenShift, ulGShift;
	ULONG							ulRW, ulLen, ulAddr;

	// Initialize the Debug Registers.

	g_dwDR7 = MACRO_DEFAULT_DR7_MASK;
	memset( & g_vdwDR, 0, sizeof( g_vdwDR ) );

	// Iterate through the List of Breakpoints of IO, MEMORY and EXEC type.

	for ( ulI = 0; ulI < MACRO_MAXNUM_OF_BREAKPOINTS; ulI ++ )
	{
		pbThis = & psisSysInterrStatus->vbpBreakpoints[ ulI ];

		if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_UNUSED ||
			pbThis->bDisabled ||
			pbThis->bIsUsingDebugRegisters == FALSE )
				continue;

		// Calculate RW, Len and Addr.

		switch( pbThis->ulType )
		{
		case MACRO_BREAKPOINTTYPE_EXEC:
			{
				ulLen = 0;
				ulRW = 0;
			}
			break;

		case MACRO_BREAKPOINTTYPE_IO:
			{
				ulLen = pbThis->ulDebugRegisterCondLen - 1;
				ulRW = 0x2;
			}
			break;

		case MACRO_BREAKPOINTTYPE_MEMORY:
			{
				ulLen = pbThis->ulDebugRegisterCondLen - 1;
				ulRW = pbThis->ulDebugRegisterCondRW == 0x2 ? 0x1 : 0x3;
			}
			break;

		default:
			continue;
		}

		ulAddr = pbThis->dwAddress;

		// Calculate the Shift Amounts.

		ulRWLenShift = 16 + ( pbThis->ulDebugRegisterNum * 4 );
		ulGShift = 1 + ( pbThis->ulDebugRegisterNum * 2 );

		// Set the Registers.

		g_dwDR7 |= ( ulRW << ulRWLenShift ) | ( ulLen << (ulRWLenShift+2) ) | ( 1 << ulGShift );
		g_vdwDR[ pbThis->ulDebugRegisterNum ] = ulAddr;

		// Set the "Installed" bit.

		pbThis->bInstalled = TRUE;
	}

	// Return.

	return;
}

//==========================================
// ResetDebugRegisters Function Definition.
//==========================================

VOID ResetDebugRegisters( VOID )
{
	// Reset the Values.

	g_dwDR7 = MACRO_DEFAULT_DR7_MASK;
	memset( & g_vdwDR, 0, sizeof( g_vdwDR ) );

	// Return.

	return;
}

//==========================================================
// CheckIfMemoryBreakpointIsCompatible Function Definition.
//==========================================================

BOOLEAN CheckIfMemoryBreakpointIsCompatible( IN BREAKPOINT* pbThis )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	CHAR							szProcessName[ 0x10 + 1 ]; // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
	CHAR*							pszCurrentProcessName;
	CHAR*							pszDot;

	//
	// Compare the Process Name.
	//

	if ( strlen( pbThis->ciContext.szProcessName ) )
	{
		// Get the Name of the Current Process.

		pszCurrentProcessName = GetImageFileNameFieldPtrOfCurrProc(
				extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].pvPCRBase
			);
		memset( szProcessName, 0, sizeof( szProcessName ) );
		if ( pszCurrentProcessName )
			memcpy( szProcessName, pszCurrentProcessName, MACRO_KPEB_IMGFLNAME_FIELD_SIZE );
		pszDot = MACRO_CRTFN_NAME(strchr)( szProcessName, '.' );
		if ( pszDot )
			* pszDot = 0;

		// Compare the Names.

		if ( MACRO_CRTFN_NAME( stricmp )( szProcessName, pbThis->ciContext.szProcessName ) )
			return FALSE;
	}

	//
	// Return.
	//

	return TRUE;
}

//======================================================
// CheckIfIoBreakpointIsCompatible Function Definition.
//======================================================

BOOLEAN CheckIfIoBreakpointIsCompatible( IN BREAKPOINT* pbThis )
{
	PDEVICE_EXTENSION				extension = g_pDeviceObject->DeviceExtension;
	BYTE*							pbInstr;

	//
	// Check the Instruction that Caused the Exception.
	//

	if ( pbThis->ulDebugRegisterCondRW == 0x3 /* read + write */ )
		return TRUE;

	pbInstr = (BYTE*)
		extension->sisSysInterrStatus.vx86ccProcessors[ extension->sisSysInterrStatus.dwCurrentProcessor ].
			x86vicContext.EIP - 1;

	if ( IsPagePresent_BYTERANGE( pbInstr - 1, 2 ) == FALSE )
		return TRUE;

	switch( * pbInstr )
	{
	case 0xEC: // IN.
	case 0xED:

		if ( pbThis->ulDebugRegisterCondRW == 0x1 /* read */ )
			return TRUE;
		else
			return FALSE;

	case 0xEE: // OUT.
	case 0xEF:

		if ( pbThis->ulDebugRegisterCondRW == 0x1 /* read */ )
			return FALSE;
		else
			return TRUE;

	default: // TWO BYTE IN or OUT.

		pbInstr --;

		switch( * pbInstr )
		{
		case 0xE4: // IN
		case 0xE5:

			if ( pbThis->ulDebugRegisterCondRW == 0x1 /* read */ )
				return TRUE;
			else
				return FALSE;

		case 0xE6: // OUT
		case 0xE7:

			if ( pbThis->ulDebugRegisterCondRW == 0x1 /* read */ )
				return FALSE;
			else
				return TRUE;

		default: // ???

			return FALSE;

		}
	}
}

//===============================================
// GetBreakpointDescription Function Definition.
//===============================================

BOOLEAN GetBreakpointDescription( OUT CHAR* pszOutputBuffer, OUT CHAR* pszAddInfoBuffer, IN BREAKPOINT* pbThis )
{
	CHAR		szBuffer[ 0x40 ] = "";
	ULONG		ulLen;

	strcpy( pszOutputBuffer, "" );
	if ( pszAddInfoBuffer )
		strcpy( pszAddInfoBuffer, "" );

	//
	// Return a Description of the Breakpoint.
	//

	switch( pbThis->ulType )
	{
	case MACRO_BREAKPOINTTYPE_EXEC:

		strcat( pszOutputBuffer, "BPX" );

		if ( pbThis->dwAddress > (DWORD) g_pvMmUserProbeAddress )
		{
			sprintf( szBuffer, " 0x%.8X ", pbThis->dwAddress );
		}
		else
		{
			if ( strlen( pbThis->ciContext.szModuleName ) )
			{
				sprintf( szBuffer, " 0x%.8X /M %s ",
					pbThis->ciContext.dwModuleOffset, pbThis->ciContext.szModuleName );

				if ( pszAddInfoBuffer )
					strcpy( pszAddInfoBuffer, "(address is module start based)" );
			}
			else
			{
				sprintf( szBuffer, " 0x%.8X ",
					pbThis->ciContext.dwModuleOffset );
			}

			strcat( szBuffer, "/P " );
			if ( strlen( pbThis->ciContext.szProcessName ) )
			{
				strcat( szBuffer, pbThis->ciContext.szProcessName );
				strcat( szBuffer, " " );
			}
		}

		break;

	case MACRO_BREAKPOINTTYPE_INTERRUPT:

		strcat( pszOutputBuffer, "BPINT" );

		sprintf( szBuffer, " 0x%.2X ", pbThis->dwAddress );

		break;

	case MACRO_BREAKPOINTTYPE_IO:

		strcat( pszOutputBuffer, "BPIO" );

		sprintf( szBuffer, " 0x%.4X ", pbThis->dwAddress );

		break;

	case MACRO_BREAKPOINTTYPE_MEMORY:

		strcat( pszOutputBuffer, "BPM" );

		switch ( pbThis->ulDebugRegisterCondLen )
		{
		case 1:
			strcat( pszOutputBuffer, "B" );
			break;
		case 2:
			strcat( pszOutputBuffer, "W" );
			break;
		case 4:
			strcat( pszOutputBuffer, "D" );
			break;
		default:
			return FALSE;
		}

		sprintf( szBuffer, " 0x%.8X ", pbThis->dwAddress );

		break;

	default:
		return FALSE;
	}

	strcat( pszOutputBuffer, szBuffer );

	if ( pbThis->bIsUsingDebugRegisters )
	{
		sprintf( szBuffer, "/DR %.1X ", pbThis->ulDebugRegisterNum );
		strcat( pszOutputBuffer, szBuffer );
	}

	if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_IO ||
		pbThis->ulType == MACRO_BREAKPOINTTYPE_MEMORY )
	{
		switch( pbThis->ulDebugRegisterCondRW )
		{
		case 1:
			strcat( pszOutputBuffer, "/R " );
			break;
		case 2:
			strcat( pszOutputBuffer, "/W " );
			break;
		case 3:
			strcat( pszOutputBuffer, "/RW " );
			break;
		}
	}

	if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_IO )
	{
		sprintf( szBuffer, "/L %.1X ", pbThis->ulDebugRegisterCondLen );
		strcat( pszOutputBuffer, szBuffer );
	}

	ulLen = strlen( pszOutputBuffer );
	if ( ulLen && pszOutputBuffer[ ulLen - 1 ] == ' ' )
		pszOutputBuffer[ ulLen - 1 ] = 0;

	//
	// Additional Informations.
	//

	if ( pbThis->ulType == MACRO_BREAKPOINTTYPE_MEMORY &&
		pszAddInfoBuffer &&
		strlen( pbThis->ciContext.szProcessName ) )
	{
		sprintf( pszAddInfoBuffer, "(tied to \"%s\")", pbThis->ciContext.szProcessName );
	}

	//
	// Return Success.
	//

	return TRUE;
}

//=======================================
// EnableBreakpoint Function Definition.
//=======================================

VOID EnableBreakpoint( IN BREAKPOINT* pbBP )
{
	// Enable and return.

	pbBP->bDisabled = FALSE;
}

//========================================
// DisableBreakpoint Function Definition.
//========================================

VOID DisableBreakpoint( IN BREAKPOINT* pbBP )
{
	// Enable and return.

	pbBP->bDisabled = TRUE;
}

//================================================
// PrintBreakpointHitMessage Function Definition.
//================================================

VOID PrintBreakpointHitMessage( IN BREAKPOINT* pbBP, IN BOOLEAN bPrintBranchInfo )
{
	PDEVICE_EXTENSION	extension = g_pDeviceObject->DeviceExtension;
	CHAR				szBuffer1[ 0x200 ];
	CHAR				szBuffer2[ 0x200 ];
	__int64				liStart, liEnd;
	double				dET;
	BOOLEAN				bRes;
	DWORD				dwFromIP, dwToIP;

	//
	// Print the Message.
	//

	if ( GetBreakpointDescription( szBuffer1, NULL, pbBP ) == FALSE )
		return;

	EliminateStringEscapes( szBuffer2, szBuffer1 );

	OutputPrint( TRUE, FALSE, "Break due to !71 %s !07.", szBuffer2 );

	if ( g_dwEnterTscHigh && g_dwEnterTscLow &&
		g_dwExitTscHigh && g_dwExitTscLow )
	{
		* (DWORD*) & liStart = g_dwExitTscLow;
		* ( (DWORD*) & liStart + 1 ) = g_dwExitTscHigh;

		* (DWORD*) & liEnd = g_dwEnterTscLow;
		* ( (DWORD*) & liEnd + 1 ) = g_dwEnterTscHigh;

		dET = ( (double) ( ( liEnd - liStart ) * 1000000 ) ) / extension->liCpuCyclesPerSecond.QuadPart;

		* szBuffer1 = 0;
		cftog( dET, szBuffer1, 5, FALSE );

		if ( strlen( szBuffer1 ) )
			OutputPrint( FALSE, FALSE, "      elapsed time = %s microseconds", szBuffer1 );
	}

	if ( bPrintBranchInfo )
	{
		bRes = GetLastBranchMSRs( & dwFromIP, & dwToIP );
		if ( bRes )
		{
			OutputPrint( FALSE, FALSE, "  lastbranchfromip = %.8X", dwFromIP );
			OutputPrint( FALSE, FALSE, "    lastbranchtoip = %.8X", dwToIP );
		}
	}

	return;
}

//===============================================
// AddressIsUser2KernelGate Function Definition.
//===============================================

BOOLEAN AddressIsUser2KernelGate( IN DWORD dwAddress )
{
	BOOLEAN				retval = FALSE;

	// Check the Conditions.

	if ( ( dwAddress & 0xFFF ) == 0 &&
		IsPagePresent_BYTERANGE( (BYTE*) dwAddress, sizeof( U2KSTUB ) ) &&
		memcmp( (PVOID) dwAddress, & g_u2kstub, sizeof( U2KSTUB ) ) == 0 )
	{
		retval = TRUE;
	}

	// Return.

	return retval;
}

//===========================================
// HandleUser2KernelBkp Function Definition.
//===========================================

CHAR		g_szAutoTypedCommand[ MACRO_AUTOTYPEDCOMMAND_MAXSIZE ];

BOOLEAN HandleUser2KernelBkp( IN OUT x86_REGISTERS_CONTEXT* prcParams, IN BOOLEAN bFirstChance )
{
	PDEVICE_EXTENSION	extension = g_pDeviceObject->DeviceExtension;
	BOOLEAN				retval = FALSE;	// <- Meaning: "Enter Debugger" if FirstChance == FALSE ; "U2K Breakpoint Handled" if FirstChance == TRUE .

	//
	// Handle the Request.
	//

	if ( bFirstChance )
	{
		//
		// The Current Context is the "FirstChance" one.
		//

		retval = TRUE;

		switch( prcParams->EAX )
		{
		case U2KCMD_EAX_ISDEBUGGERPRESENT:
			{
				// ### DO NOTHING ... ###
			}

			retval = FALSE;
			break;

		case U2KCMD_EAX_GETHISTORYLASTLINEIDANDDEBUGGERVERB:
			{
				ULONG		ulTotalLinesNum = 0, ulFirstValidID = 0, ulLastLineID = 0;

				// Return the History Pane Information.

				if ( extension->ulHistoryLineID )
				{
					ulTotalLinesNum = extension->ulOutputWinStrPtrsBufferPosInBytes / sizeof( CHAR* );
					ulLastLineID = extension->ulHistoryLineID;
					ulFirstValidID = ulLastLineID - ulTotalLinesNum + 1;
				}

				prcParams->EAX = ulLastLineID;
				prcParams->EBX = ulFirstValidID;

				// Return the Debugger Verb Information.

				prcParams->ECX = DEBVERB_ECX_NONE;

				if ( extension->bOpenUserModeDashboard )
				{
					extension->bOpenUserModeDashboard = FALSE;
					prcParams->ECX = DEBVERB_ECX_OPENDASHBOARD;
				}

				// Record the Times.

				BugChecker_ReadTimeStampCounter( & extension->liLastDebuggerVerbReadTime );

				extension->dwLastEnterTscHigh = 0;
				extension->dwLastEnterTscLow = 0;
			}

			retval = FALSE;
			break;

		case U2KCMD_EAX_GETHISTORYLINETEXT:
			{
				ULONG*			pulIDs = (ULONG*) prcParams->EBX;
				CHAR*			pszBuff = (CHAR*) prcParams->ECX;
				ULONG			ulBuffSize = prcParams->EDX;

				ULONG			ulIndexS = 0, ulIndexE = 0;

				BOOLEAN			res;

				// Call the Implementation.

				prcParams->EAX = 0;

				if ( ulBuffSize &&
					IsPagePresent_BYTERANGE( (BYTE*) pszBuff, ulBuffSize ) &&
					IsPagePresent_BYTERANGE( (BYTE*) pulIDs, sizeof( ULONG ) * 2 ) )
				{
					ulIndexS = pulIDs[ 0 ];
					ulIndexE = pulIDs[ 1 ];

					res = GetHistoryLineText( & ulIndexS, & ulIndexE, pszBuff, ulBuffSize );

					if ( res )
					{
						prcParams->EAX = 1;
						prcParams->EBX = ulIndexS;
						prcParams->ECX = ulIndexE;
					}
				}
			}

			retval = FALSE;
			break;

		case U2KCMD_EAX_SETOSKEYBLEDSSTATUSVAR:
			{
				// Store the Variable.

				BYTE			bOldStatus = extension->sisSysInterrStatus.bOsKeybLEDsStatus;
				BOOLEAN			bOldEnforce = extension->sisSysInterrStatus.bEnforceOsKeybLEDsStatus;

				extension->sisSysInterrStatus.bOsKeybLEDsStatus = (BYTE) ( prcParams->EBX & 0xFF );
				extension->sisSysInterrStatus.bEnforceOsKeybLEDsStatus = TRUE;

				prcParams->EAX = 0;

				if ( bOldStatus != extension->sisSysInterrStatus.bOsKeybLEDsStatus ||
					bOldEnforce != extension->sisSysInterrStatus.bEnforceOsKeybLEDsStatus )
				{
					prcParams->EAX = 1;
				}
			}

			retval = FALSE;
			break;
		}
	}
	else
	{
		//
		// The Current Context is the Debugger one: all the Structures and the State are Valid.
		//

		switch( prcParams->EAX )
		{
		case U2KCMD_EAX_GETSYMTABLEINFO:
			{
				DWORD					index = prcParams->EBX;
				U2KINT_SYMTABLEINFO*	out = (U2KINT_SYMTABLEINFO*) prcParams->ECX;

				prcParams->EAX = 0;

				// Return the Information.

				if ( IsPagePresent_BYTERANGE( (BYTE*) out, sizeof( U2KINT_SYMTABLEINFO ) ) &&
					index < extension->ulSymTablesNum )
				{
					memcpy( out, & extension->stiSymTables[ index ].general, sizeof( U2KINT_SYMTABLEINFO ) );
					prcParams->EAX = 1;
				}
			}
			break;

		case U2KCMD_EAX_GETSYMMEMINFO:
			{
				ULONG			space = extension->ulSymMemorySize;
				SYMTABLEINFO*	last;

				if ( extension->ulSymTablesNum )
				{
					last = & extension->stiSymTables[ extension->ulSymTablesNum - 1 ];
					space -= last->offset + last->general.ulSize;
				}

				// Return the Information.

				prcParams->EAX = extension->ulSymMemorySize;
				prcParams->EBX = space;
			}
			break;

		case U2KCMD_EAX_ADDSYMTABLE:
			{
				// Call the Implementation.

				prcParams->EAX = 0;

				if ( prcParams->ECX &&
					IsPagePresent_BYTERANGE( (BYTE*) prcParams->EBX, prcParams->ECX ) &&
					*( (BYTE*) prcParams->EBX + prcParams->ECX - 1 ) == 0 )
				{
					prcParams->EAX = AddSymTableToken(
						(CHAR*) prcParams->EBX );
				}
			}
			break;

		case U2KCMD_EAX_REMOVESYMTABLE:
			{
				BOOLEAN			res;

				// Call the Implementation.

				res = RemoveSymTable( prcParams->EBX );

				prcParams->EAX = res ? 1 : 0;
			}
			break;

		case U2KCMD_EAX_DOWNLOADTEXT:
			{
				BOOLEAN			bFromClipboard = prcParams->EBX ? TRUE : FALSE;
				CHAR*			pszBuff = (CHAR*) prcParams->ECX;
				ULONG			ulBuffSize = prcParams->EDX;

				BOOLEAN			res;

				// Call the Implementation.

				prcParams->EAX = 0;

				if ( ulBuffSize &&
					IsPagePresent_BYTERANGE( (BYTE*) pszBuff, ulBuffSize ) )
				{
					res = GetEnvScriptText( bFromClipboard, pszBuff, ulBuffSize );

					prcParams->EAX = res ? 1 : 0;
				}
			}
			break;

		case U2KCMD_EAX_UPLOADTEXT:
			{
				BOOLEAN			bFromClipboard = prcParams->EBX ? TRUE : FALSE;
				CHAR*			pszBuff = (CHAR*) prcParams->ECX;
				ULONG			ulBuffSize = prcParams->EDX;

				BOOLEAN			res;

				// Call the Implementation.

				prcParams->EAX = 0;

				if ( ulBuffSize &&
					IsPagePresent_BYTERANGE( (BYTE*) pszBuff, ulBuffSize ) &&
					*( pszBuff + ulBuffSize - 1 ) == '\0' )
				{
					res = SetEnvScriptText( bFromClipboard, pszBuff );

					prcParams->EAX = res ? 1 : 0;
				}
			}
			break;

		case U2KCMD_EAX_EXECUTECOMMAND:
			{
				CHAR*			pszBuff = (CHAR*) prcParams->EBX;
				ULONG			ulBuffSize = prcParams->ECX;

				ULONG			size, maxsize;
				ULONG			ulX0, ulY0, ulX1, ulY1;

				// Copy the Command Text.

				prcParams->EAX = 0;

				if ( ulBuffSize &&
					IsPagePresent_BYTERANGE( (BYTE*) pszBuff, ulBuffSize ) &&
					*( pszBuff + ulBuffSize - 1 ) == '\0' )
				{
					GetConsolePrintCoords( & ulX0, & ulY0, & ulX1, & ulY1 );

					size = strlen( pszBuff );
					maxsize = ulX1 - ulX0 - 1;

					if ( size < maxsize && size < sizeof( g_szAutoTypedCommand ) - 1 )
					{
						strcpy( g_szAutoTypedCommand, pszBuff );
						prcParams->EAX = 1;
						retval = TRUE;
					}
				}
			}
			break;

		case U2KCMD_EAX_UPDATEKEYBLEDS:
			{
				// Update the LEDs.

				RestoreKeybLEDs();
			}
			break;
		}
	}

	// Return.

	return retval;
}

//=============================================
// HandleTimerThreadEvent Function Definition.
//=============================================

VOID HandleTimerThreadEvent( IN OUT x86_REGISTERS_CONTEXT* prcParams )
{
	// Handle the Event.

	switch( prcParams->EAX )
	{
	case MACRO_EAX_TT_SERVICE_NONE:
		{
			// ### DO NOTHING ###
		}
		break;

	case MACRO_EAX_TT_SERVICE_COMPLETE_SYMLOAD:
		{
			BOOLEAN			res;

			// Call the Implementation.

			res = AddSymTable(
				(BYTE*) prcParams->EBX, prcParams->ECX, (CHAR*) prcParams->EDX );

			prcParams->EAX = res ? 1 : 0;
		}
		break;
	}

	// Return.

	return;
}

//======================================
// RestoreKeybLEDs Function Definition.
//======================================

VOID RestoreKeybLEDs ( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;

	// Set the LEDs.

	__asm
	{
		// Push.

		push		eax
		push		ebx
		push		ecx
		push		edx

		// Load in EDX the Pointer to the Structure.

		mov			edx, extension
		lea			edx, [ edx ]DEVICE_EXTENSION.sisSysInterrStatus

		// Restore the LEDs Status.

		push		edx
		cmp			[ edx ]SYSINTERRUPTS_STATUS.bEnforceOsKeybLEDsStatus, FALSE
		je			_DoNotEnforceLedsStat
		mov			dh, [ edx ]SYSINTERRUPTS_STATUS.bOsKeybLEDsStatus
		jmp			_CanSetKeybLeds
_DoNotEnforceLedsStat:
		mov			dh, 0
_CanSetKeybLeds:
		call		SetKeyboardLEDsStatus
		pop			edx

		// Pop.

		pop			edx
		pop			ecx
		pop			ebx
		pop			eax
	}

	// Return.

	return;
}

//=====================================================
// IsClientPollingForDebuggerVerb Function Definition.
//=====================================================

BOOLEAN IsClientPollingForDebuggerVerb ( VOID )
{
	PDEVICE_EXTENSION		extension = g_pDeviceObject->DeviceExtension;
	__int64					delta, start, end, res;

	if ( extension->sisSysInterrStatus.bInDebugger == FALSE )
		return FALSE;

	// Return the Information.

	start = * (__int64*) & extension->liLastDebuggerVerbReadTime;

	* (DWORD*) & end = g_dwEnterTscLow;
	* ( (DWORD*) & end + 1 ) = g_dwEnterTscHigh;

	delta = end - start;

	res = * (__int64*) & extension->liCpuCyclesPerSecond;

	return delta < ( (res*2) / 3 );
}
