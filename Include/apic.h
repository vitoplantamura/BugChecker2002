/***************************************************************************************
  *
  * apic.h - VpcICE "i8259A PIC / i82489 APIC" Routines Header File.
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
#include "Utils.h"
#include "detours.h"
#include "u2kint.h"

//==============
// Definitions.
//==============

// -- I/O Apic --

#define MACRO_VPCICE_MAXIMUM_INTINPPINS_SUPPORTED		0x20

#define MACRO_IOAPICID				0x00
#define MACRO_IOAPICVER				0x01
#define MACRO_IOAPICARB				0x02

#define MACRO_IOREDTBL0_0			0x10
#define MACRO_IOREDTBL0_1			0x11
#define MACRO_IOREDTBL1_0			0x12
#define MACRO_IOREDTBL1_1			0x13
#define MACRO_IOREDTBL2_0			0x14
#define MACRO_IOREDTBL2_1			0x15
#define MACRO_IOREDTBL3_0			0x16
#define MACRO_IOREDTBL3_1			0x17
#define MACRO_IOREDTBL4_0			0x18
#define MACRO_IOREDTBL4_1			0x19
#define MACRO_IOREDTBL5_0			0x1A
#define MACRO_IOREDTBL5_1			0x1B
#define MACRO_IOREDTBL6_0			0x1C
#define MACRO_IOREDTBL6_1			0x1D
#define MACRO_IOREDTBL7_0			0x1E
#define MACRO_IOREDTBL7_1			0x1F
#define MACRO_IOREDTBL8_0			0x20
#define MACRO_IOREDTBL8_1			0x21
#define MACRO_IOREDTBL9_0			0x22
#define MACRO_IOREDTBL9_1			0x23
#define MACRO_IOREDTBL10_0			0x24
#define MACRO_IOREDTBL10_1			0x25
#define MACRO_IOREDTBL11_0			0x26
#define MACRO_IOREDTBL11_1			0x27
#define MACRO_IOREDTBL12_0			0x28
#define MACRO_IOREDTBL12_1			0x29
#define MACRO_IOREDTBL13_0			0x2A
#define MACRO_IOREDTBL13_1			0x2B
#define MACRO_IOREDTBL14_0			0x2C
#define MACRO_IOREDTBL14_1			0x2D
#define MACRO_IOREDTBL15_0			0x2E
#define MACRO_IOREDTBL15_1			0x2F
#define MACRO_IOREDTBL16_0			0x30
#define MACRO_IOREDTBL16_1			0x31
#define MACRO_IOREDTBL17_0			0x32
#define MACRO_IOREDTBL17_1			0x33
#define MACRO_IOREDTBL18_0			0x34
#define MACRO_IOREDTBL18_1			0x35
#define MACRO_IOREDTBL19_0			0x36
#define MACRO_IOREDTBL19_1			0x37
#define MACRO_IOREDTBL20_0			0x38
#define MACRO_IOREDTBL20_1			0x39
#define MACRO_IOREDTBL21_0			0x3A
#define MACRO_IOREDTBL21_1			0x3B
#define MACRO_IOREDTBL22_0			0x3C
#define MACRO_IOREDTBL22_1			0x3D
#define MACRO_IOREDTBL23_0			0x3E
#define MACRO_IOREDTBL23_1			0x3F

// -- General --

#define MACRO_WINNT_DEFAULT_INTERRUPT_VECTOR_BASE		0x30
#define MACRO_x86_KEYBOARD_IRQ							0x1
#define MACRO_x86_MOUSE_IRQ								0xC
#define MACRO_WINNT_DEFAULT_IOAPIC_KEYBOARD_IOREDTBL_0	MACRO_IOREDTBL1_0
#define MACRO_WINNT_DEFAULT_IOAPIC_KEYBOARD_IOREDTBL_1	MACRO_IOREDTBL1_1
#define MACRO_WINNT_DEFAULT_IOAPIC_MOUSE_IOREDTBL_0		MACRO_IOREDTBL12_0
#define MACRO_WINNT_DEFAULT_IOAPIC_MOUSE_IOREDTBL_1		MACRO_IOREDTBL12_1

#define MACRO_VPCICE_IPI_INTVECTOR						0xF5
#define MACRO_IOAPIC_KEYBOARD_PROXY_INTVECTOR			0xF6
#define MACRO_IOAPIC_MOUSE_PROXY_INTVECTOR				0xF7

// -- Cursor --

#define MACRO_CURSOR_BLINKING_INTERVAL_INSECS		( 0.3f )

// -- x86 --

#define MACRO_OF_MASK				( 1 << 11 )
#define MACRO_DF_MASK				( 1 << 10 )
#define MACRO_IF_MASK				( 1 << 9 )
#define MACRO_SF_MASK				( 1 << 7 )
#define MACRO_ZF_MASK				( 1 << 6 )
#define MACRO_AF_MASK				( 1 << 4 )
#define MACRO_PF_MASK				( 1 << 2 )
#define MACRO_CF_MASK				( 1 << 0 )

#define MACRO_CR0_WP_MASK			( 0x00010000 )

// -- CPUs --

#define MACRO_MAX_NUM_OF_PROCESSORS	32

// -- System --

#define MACRO_PCR_SELECTOR_IN_SYSTEMCODE		cs:g_wFSSEG_in_Kernel // 0x30

// -- FxSave Instruction --

#define MACRO_FXSAVE_MEMSIZE				512
#define MACRO_FXSAVE_MEMALIGN				16
#define MACRO_FXSAVE_MEMALIGN_MASK			0xFFFFFFF0

// -- FnSave Instruction --

#define MACRO_FNSAVE_MEMSIZE				108

// -- Context Info --

#define MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE	( 0x10 + 1 ) // MACRO_KPEB_IMGFLNAME_FIELD_SIZE
#define MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE		80
#define MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE	5
#define MACRO_CONTEXTINFO_PROCESSNAME_HEXFORM_SIZE	10

// -- Breakpoints --

#define MACRO_BREAKPOINT_SIZE				1
#define MACRO_BREAKPOINT_OPCODE				0xCC

#define MACRO_MAXNUM_OF_BREAKPOINTS			256

#define MACRO_MAXNUM_OF_DISCOVERED_BPS		64

// -- Detours --

#define MACRO_DETOUR_MAXSIZE				0x20

#define MACRO_MAXNUM_OF_DETOURS				256		// ### WARNING ### This value depends on the Number of Detours
													//   that are defined in the file apic.c. If you are changing this
													//   value, make sure that the Structures in apic.c are updated as well.

#define MACRO_TRAMPOLINE_SIZE				32

#define MACRO_MAXNUM_OF_DISCOVERED_DTS		64

//

#define MACRO_USRSPACE_TRAMPS_MAGIC_STR		"xBUGCHECKERTRAMPOLINESTRUCTURESx"
#define MACRO_USRSPACE_TRAMPS_MAGIC_STR_LEN	32

#define MACRO_USRSPACE_MEMORY_MAGIC_STR		"x_BUGCHECKER_USER_SPACE_MEMORY_x" // DETOURLESS
#define MACRO_USRSPACE_MEMORY_MAGIC_STR_LEN	32 // DETOURLESS

#define MACRO_USRSPACE_TRAMPS_VIRTADDR		0x6F800000
#define MACRO_USRSPACE_MEMORY_SIZE			0x8000 // DETOURLESS

// -- System Restore Actions --

#define MACRO_SYSREST_NONE					0 // None.
#define MACRO_SYSREST_CCOPREST				1 // We have to replace a CC Opcode with the previous byte.
#define MACRO_SYSREST_CCOPREST_SINGSTP		2 // Same as above but the user requested a Single Step.

// -- Mouse Packets --

#define MACRO_MOUSEPACKET_MAXSIZE			4

// -- CPU Speed - Time Stamp Counter --

#define MACRO_CALCULATECPUSPEED_FACTOR		6
#define MACRO_CURSORTIME_FACTOR				3

// -- Page Frame Modifications Database --

#define MACRO_PFMDB_ADDFRAMETOPFMDB_MAXBYTESNUM		32	// This should be less than or equal to the max size of
														//  a debugger Breakpoint and Detour.

// -- Debug Registers --

#define MACRO_DEFAULT_DR7_MASK				( (1<<10) | (1<<8) | (1<<9) )	// bit 10 + LE (bit 8) + GE (bit 9)

// -- Timer Thread Services --

#define MACRO_EAX_TT_SERVICE_NONE				0x00
#define MACRO_EAX_TT_SERVICE_COMPLETE_SYMLOAD	0x01	// EBX=address, ECX=size, EDX=null-term name ptr. Returns: EAX=success.

// -- Auto Typed Command --

#define MACRO_AUTOTYPEDCOMMAND_MAXSIZE			0x50

//==========
// Externs.
//==========

extern DWORD			g_vdwDR[ 4 ];
extern DWORD			g_dwDR7;

extern DWORD			g_dwEnterTscHigh, g_dwEnterTscLow, g_dwExitTscHigh, g_dwExitTscLow;

extern WORD				g_wVpcICEDataSegment;

extern BYTE				g_bLastScanCode;

//=========
// Macros.
//=========

#define ENTERING_VPCICE_AREA \
		__asm push		eax																	\
		__asm mov		eax, MACRO_DEFAULT_DR7_MASK											\
		__asm mov		dr7, eax															\
		__asm pop		eax

#define EXITING_VPCICE_AREA \
		__asm push		eax																	\
		__asm mov		eax, cs:g_vdwDR[ 0*4 ]												\
		__asm mov		dr0, eax															\
		__asm mov		eax, cs:g_vdwDR[ 1*4 ]												\
		__asm mov		dr1, eax															\
		__asm mov		eax, cs:g_vdwDR[ 2*4 ]												\
		__asm mov		dr2, eax															\
		__asm mov		eax, cs:g_vdwDR[ 3*4 ]												\
		__asm mov		dr3, eax															\
		__asm mov		eax, cs:g_dwDR7														\
		__asm mov		dr7, eax															\
		__asm pop		eax

#define ENTERING_DEBUGGER \
		__asm push		edx																	\
		__asm pushf																			\
		__asm mov		edx, g_psisSysInterrStatus											\
		__asm cmp		[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE				\
		__asm jne		_DebuggerIsStepping_E												\
		__asm ENTERING_VPCICE_AREA															\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.bInDebugger, TRUE						\
		__asm mov		g_bLastScanCode, 0													\
		__asm push		edx																	\
		__asm push		eax																	\
		__asm push		ds																	\
		__asm mov		ax, cs:g_wVpcICEDataSegment											\
		__asm mov		ds, ax																\
		__asm rdtsc																			\
		__asm mov		g_dwEnterTscHigh, edx												\
		__asm mov		g_dwEnterTscLow, eax												\
		__asm pop		ds																	\
		__asm pop		eax																	\
		__asm pop		edx																	\
		__asm _DebuggerIsStepping_E:														\
		__asm popf																			\
		__asm pop		edx

#define EXITING_DEBUGGER(n) \
		__asm push		edx																	\
		__asm pushf																			\
		__asm mov		edx, g_psisSysInterrStatus											\
		__asm cmp		[ edx ]SYSINTERRUPTS_STATUS.bIsSingleStepping, FALSE				\
		__asm jne		_DebuggerIsStepping_L##n											\
		__asm push		edx																	\
		__asm push		eax																	\
		__asm push		ds																	\
		__asm mov		ax, cs:g_wVpcICEDataSegment											\
		__asm mov		ds, ax																\
		__asm rdtsc																			\
		__asm mov		g_dwExitTscHigh, edx												\
		__asm mov		g_dwExitTscLow, eax													\
		__asm pop		ds																	\
		__asm pop		eax																	\
		__asm pop		edx																	\
		__asm mov		[ edx ]SYSINTERRUPTS_STATUS.bInDebugger, FALSE						\
		__asm EXITING_VPCICE_AREA															\
		__asm _DebuggerIsStepping_L##n:														\
		__asm popf																			\
		__asm pop		edx

//=============
// Structures.
//=============

#pragma pack(push, 1)

	typedef struct _x86_IDTReg
	{
		WORD		wLimit;
		DWORD		dwBase;

	} x86_IDTReg, *Px86_IDTReg;

#pragma pack(pop)

// // //

#pragma pack(push, 1)

	typedef struct _x86_GDTReg
	{
		WORD		wLimit;
		DWORD		dwBase;

	} x86_GDTReg, *Px86_GDTReg;

#pragma pack(pop)

// // //

#pragma pack(push, 1)

	typedef struct _IDT_HOOKEDENTRY_INFO
	{
		//==========
		// INFO.
		//==========

		ULONG			ulVectorNum;
		ULONG			ulProcessorNum;

		//==========
		// STUB.
		//==========

		BYTE			bPushOpcode; // = 0x68
		DWORD			dwOriginalISRAddress; // = Address of the Original Windows ISR.
		BYTE			bJumpOpcode; // = 0xE9
		DWORD			dwNewISRAddress_Sub; // = ( Address of New ISR ) - ( offset IDT_HOOKEDENTRY_INFO + FIELD_OFFSET( IDT_HOOKEDENTRY_INFO, bJumpOpcode ) ) - 5

	} IDT_HOOKEDENTRY_INFO, *PIDT_HOOKEDENTRY_INFO;

#pragma pack(pop)

#define MACRO_MAXNUM_OF_IDTHOOKEDENTRYINFOS		( 0x8 * MACRO_MAX_NUM_OF_PROCESSORS )

// // //

typedef struct _x86_REGISTERS_CONTEXT
{
	// General Purpose Registers.

	DWORD		EAX, EBX, ECX, EDX;
	DWORD		ESI, EDI;
	DWORD		EBP, ESP;
	DWORD		EFLAGS;
	DWORD		EIP;
	WORD		CS, DS, SS, ES, FS, GS;

} x86_REGISTERS_CONTEXT, *Px86_REGISTERS_CONTEXT;

// // //

typedef struct _x86_CPU_CONTEXT
{
	//======================================================================================
	//
	// CPU STATE FOLLOWS ...
	//
	//======================================================================================

	// ### General. ###

	VOID*					pvGDTBase;
	VOID*					pvIDTBase;
	VOID*					pvPCRBase;

	// ### CPUID(1) Info. ###

	DWORD					dwCpuIdInfo[ 4 ];

	// ### System Registers. ###

	DWORD					dwCR0;
	DWORD					dwCR2;
	DWORD					dwCR3;
	DWORD					dwCR4;
	DWORD					dwDR0;
	DWORD					dwDR1;
	DWORD					dwDR2;
	DWORD					dwDR3;
	DWORD					dwDR6;
	DWORD					dwDR7;

	// ### Local APIC. ###

	DWORD					dwTPRValue;

	DWORD					dwIDRValue;
	DWORD					dwVerRValue;
	DWORD					dwAPRValue;
	DWORD					dwPPRValue;
	DWORD					dwDFRValue;
	DWORD					dwLDRValue;
	DWORD					dwSVRValue;
	DWORD					dwICRValue[ 2 ];
	DWORD					dwLVTTRValue;
	DWORD					dwLVTPMCRValue;
	DWORD					dwLVTLINT0RValue;
	DWORD					dwLVTLINT1RValue;
	DWORD					dwLVTErrRValue;
	DWORD					dwICountRValue;
	DWORD					dwCCountRValue;
	DWORD					dwDivCRValue;

	// ### Registers Context. ###

	x86_REGISTERS_CONTEXT	x86vicContext;

	// ### x87 / MMX / SSE / SSE2 State. ###

	BOOLEAN					b_x87_MMX_StateIsValid;
	BOOLEAN					b_SSE_SSE2_StateIsValid;

	BYTE					vb_x87_MMX_State[ MACRO_FNSAVE_MEMSIZE ];
	BYTE					vb_SSE_SSE2_State[ MACRO_FXSAVE_MEMALIGN + MACRO_FXSAVE_MEMSIZE ];

	DWORD					dw_SSE_SSE2_State_AlignDisp;

	//======================================================================================
	//
	// TEMPORARY AREA USED FOR STORING WORKING VARIABLES. (INSTEAD OF USING THE STACK...)
	//
	//======================================================================================

	// ### Temporary place where the GDTR is placed. ###

	x86_GDTReg						xgrGDTR;

	// ### Temporary place where the IDTR is placed. ###

	x86_IDTReg						xirIDTR;

	// ### Temporary place for the MXCSR Register. ###

	BOOLEAN							bWriteMXCSR;
	DWORD							dwMXCSR;
	DWORD							dwMXCSR_2;

	// ### Temporary place for "px86friFpuRestoreInfs" Parameter of SaveCpuContext Function. ###

	DWORD							dwFpuRestoreInfsPtr;

} x86_CPU_CONTEXT, *Px86_CPU_CONTEXT;

// // //

typedef struct _x86_FPU_RESTOREINFS
{
	DWORD					dwCR0;

	BOOLEAN					b_x87_MMX_StateIsValid;
	BYTE					vb_x87_MMX_State[ MACRO_FNSAVE_MEMSIZE ];

} x86_FPU_RESTOREINFS, *Px86_FPU_RESTOREINFS;

// // //

	//
	// NOTE#1: If the Target Address is above User Probe Address the CONTEXT information must be NOT present.
	//  If, otherwise, the Target Address is below, the CONTEXT information has to be specified. In this case,
	//  the Process Name information is Optional. If present the Target is tied to every process with that name.
	//  If NOT present the Target is tied to every MODULE with the name specified by the Module Name info. At
	//  least either the Process Name or the Module Name info has to be present OR (in the case of a Breakpoint)
	//  a Debug Register has to be Used (no CC breakpoints). If the Module Name info is NOT present, the Module
	//  Offset info represents the Virtual Address of the Target, starting from the beginning of the Virtual
	//  Memory.
	//
	// NOTE#2: The Process Name can be either the Name of the Process as it appears in the KPEB Structure WITHOUT
	//	the File Extension or an Hex String representing the KPEB address of the Process. The Module Name, if
	//	Present, is the name of the Module WITH the File Extension. If the Process Name is in the Hex Form, it will
	//	begin with the prefix "0x".
	//

typedef struct _CONTEXT_INFO
{
	// General.

	CHAR		szProcessName[ MACRO_CONTEXTINFO_PROCESSNAME_FIELD_SIZE ];

	CHAR		szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE ];
	DWORD		dwModuleOffset;

	// Verification.

	BYTE		vbVerification[ MACRO_CONTEXTINFO_VERIFICATION_FIELD_SIZE ]; // ## Size must be less than MACRO_DETOUR_MAXSIZE... ##

} CONTEXT_INFO, *PCONTEXT_INFO;

// // //

#define MACRO_BREAKPOINTTYPE_UNUSED				0
#define MACRO_BREAKPOINTTYPE_EXEC				1
#define MACRO_BREAKPOINTTYPE_INTERRUPT			2
#define MACRO_BREAKPOINTTYPE_IO					3
#define MACRO_BREAKPOINTTYPE_MEMORY				4

typedef struct _BREAKPOINT
{
	// General.

	ULONG			ulType;

	// State.

	BOOLEAN			bDisabled;

	BOOLEAN			bInstalled;

	//
	// Context.
	//  NOTE: in the case of an INTERRUPT or IO breakpoint, this Structure remains Unused.
	//   In the case of a MEMORY breakpoint, only the ProcessName field of this Structure CAN be Used.
	//   If it is Unused, the MEMORY breakpoint is Global.
	//

	CONTEXT_INFO	ciContext;

	//
	// Characteristics.
	//  NOTE: in the case of an INTERRUPT breakpoint, the Address field Contains the Interrupt Vector Number.
	//  In the case of an EXEC breakpoint, only the first Item of the PrevCodeBytes vector is Used.
	//  In the case of an INTERRUPT breakpoint, the PrevCodeBytes vector is filled Respecting the Order of the
	//   Processors vector in SysInterruptStatus. Consider that a 0xCC byte Indicates that that Item was Undefined.
	//  In the case of an IO or MEMORY breakpoint, the Address field Contains respectively the Io port or the Memory Address.
	//

	DWORD			dwAddress;

	BYTE			vbPrevCodeBytes[ MACRO_MAX_NUM_OF_PROCESSORS ];

	// Advanced.

	BOOLEAN			bIsUsingDebugRegisters;
	ULONG			ulDebugRegisterNum;

	ULONG			ulDebugRegisterCondLen; // valid values: 1, 2, 4. (IO and MEMORY breakp.)
	ULONG			ulDebugRegisterCondRW; // bit 0: READ; bit 1: WRITE.

	// Speed Up.

	BOOLEAN			bIsContextCompatible;

} BREAKPOINT, *PBREAKPOINT;

// // //

typedef struct _STATIC_TRAMPOLINE
{
	PVOID*		ppvOrigFnPtrPtr;
	VOID*		pvTrampoline;
	VOID*		pvGetVaFuncAddress;

} STATIC_TRAMPOLINE, *PSTATIC_TRAMPOLINE;

//

#define MACRO_DETOURTYPE_UNUSED							0
#define MACRO_DETOURTYPE_USER							1

#define MACRO_DETOURTYPE_SYS_MAPVIEWOFIMAGESECTION		0xFFFFFFFF
#define MACRO_DETOURTYPE_SYS_UNMAPVIEWOFIMAGESECTION	0xFFFFFFFE

typedef struct _DETOUR
{
	// General.

	ULONG			ulType;

	// State.

	BOOLEAN			bInstalled;

	// Context.

	CONTEXT_INFO	ciContext;

	// Characteristics.

	DWORD			dwAddress;

	ULONG			ulDetourSize;

	BYTE			vbPrevCodeBytes[ MACRO_DETOUR_MAXSIZE ];
	BYTE			vbJumpCodeBytes[ MACRO_DETOUR_MAXSIZE ];

	// Trampoline.

	STATIC_TRAMPOLINE*	pstTrampolineInf;

	// Speed Up.

	BOOLEAN			bIsContextCompatible;

} DETOUR, *PDETOUR;

// // //

typedef struct _SYSINTERRUPTS_STATUS
{
	//
	// === STATUS ===
	//

	// Debugger Status Informations.

	BOOLEAN							bInDebugger;

	BYTE							bLastScanCodeAcquired;
	BYTE							bLastAsciiCodeAcquired;

	BYTE							bKeyboardLEDsStatus;
	BYTE							bKeyboardLEDsPressed;

	BOOLEAN							bE0Recv, bE1Recv;
	BOOLEAN							bPREV_E0Recv, bPREV_E1Recv;

	BOOLEAN							bShiftPressed, bControlPressed, bAltPressed;

	//

	BOOLEAN							bEnforceOsKeybLEDsStatus;

	BYTE							bOsKeybLEDsStatus;
	BYTE							bOsKeybLEDsPressed;

	//

	BOOLEAN							bTextInsert;

	//

	x86_REGISTERS_CONTEXT			x86vicPrevContext;

	// The I/O Apic Linear Address. NULL if no I/O Apic found in the System.

	PVOID							pvIoApicMemoryPtr;
	DWORD							dwIORedirTableBackupItemsNum;

	PVOID							pvLocalApicMemoryPtr;

	DWORD							dwLocalApicTPR;

	PVOID							pvIpiIsrAddr;

	// Linear address of the ISR of the Keyboard and Informations about the Hook.

	BYTE							bMouseIntVec;
	PVOID							pvMouseIsrAddr;
	BYTE							bREAL_MouseIntVec;		// Used when an I/O Apic is detected.

	BYTE							bKeybIntVec;
	PVOID							pvKeybIsrAddr;
	BYTE							bREAL_KeybIntVec;		// Used when an I/O Apic is detected.

	BYTE							vbKeybStatus[ 0x80 ];
	BOOLEAN							bKeyb_E0_Received, bKeyb_E1_Received;
	BOOLEAN							bPREV_Keyb_E0_Received, bPREV_Keyb_E1_Received;

	BOOLEAN							bPressingRightCtrlKey;

	BOOLEAN							bMouseInterruptHasToBeEnabled;

	// Interrupts.

	BOOLEAN							bEOIRequired;

	// CPU Informations.

	MP_SPINLOCK						mpslIpiSpinLock;

	x86_CPU_CONTEXT					vx86ccProcessors[ MACRO_MAX_NUM_OF_PROCESSORS ];

	DWORD							dwNumberOfProcessors;
	DWORD							dwNumProcsWithContextFilled;
	DWORD							dwCurrentProcessor;

	// IDT Hooks Informations.

	BOOLEAN							bIdtHooksInstalled;
	BOOLEAN							bIdtHooksError;

	// IDT Hooks.

	IDT_HOOKEDENTRY_INFO			viheiIdtHooks[ MACRO_MAXNUM_OF_IDTHOOKEDENTRYINFOS ];
	ULONG							ulIdtHooksNum;

	// Synchronization at Exception Handler and Debugger Entry Point level.

	MP_SPINLOCK						mpslReentrancySpinLock;

	// Ipi Interrupt.

	BOOLEAN							bIsSendingIPIs;

	BOOLEAN							bSTIinIPIs;

	// PS/2 Mouse/Keyboard Consistence System.

	ULONG							ulMouseKeybControllerIntrCount;	// # Incremented by Mouse/Keyb ISRs... #

	BOOLEAN							bPs2MouseKeybConsistenceTestPassed;
	ULONG							ulPrevControllerIntrCount;
	BYTE							bPrevControllerDataByte;

	// Mouse Status.

	BOOLEAN							bMouseEnabled;
	BOOLEAN							bMouseDetected;

	BYTE							bMouseStatus1Byte, bMouseStatus2Byte, bMouseStatus3Byte;
	BYTE							bMouseDeviceID;

	//

	MP_SPINLOCK						mpslMousePacketSpinLock;

	BOOLEAN							bIsMousePacketReady;						//
	ULONG							ulReadyPacketSize;							//
	BYTE							bMousePacket[ MACRO_MOUSEPACKET_MAXSIZE ];	// ... before accessing any of these,
	BOOLEAN							bIsMousePacketForming;						//		acquire the Spin Lock above ...
	BYTE							bMousePacketCurrByte;						//
	BOOLEAN							bMousePacketUnknownDevice;					//

	// Page Frame Modifications Database.

	BOOLEAN							bResumeExecBecauseOfStaleBpInPf;

	MP_SPINLOCK						mpslPfmdbSpinLock;

	// Timer Thread.

	BOOLEAN							bTimerThreadEventOccurred;

	BOOLEAN							bTimerThreadCreationAllowed;
	HANDLE							hTimerThreadHandle;
	LARGE_INTEGER					liTimerThreadWaitInterval;

	// User2Kernel Gates.

	BOOLEAN							bUser2KernelGateBkpOccurred;

	// System Restore Action (after breakpoint hits etc.)

	ULONG							ulSysRestoreAction;

	// Single Step.

	BOOLEAN							bIsSingleStepping;

	BOOLEAN							bKeyStrokeSimulationPending;

	// Interrupt 3.

	BOOLEAN							bFromInt3;
	DWORD							dwInt3OpcodeInstructionPtr;
	BOOLEAN							bTransferControlToOriginalInt3;
	BOOLEAN							bINT3RequiresCodeWinEipDec;

	BOOLEAN							bDiscoverWhichObjectWasHit;
	DWORD							dwBreakpointOpcodePhysAddr;

	ULONG							ulDiscoveredBreakpointsNum;
	BREAKPOINT*						vpbpDiscoveredBreakpoints[ MACRO_MAXNUM_OF_DISCOVERED_BPS ];
	BREAKPOINT*						pbpUniqueHitBreakpoint;

	ULONG							ulDiscoveredDetoursNum;
	DETOUR*							vpdDiscoveredDetours[ MACRO_MAXNUM_OF_DISCOVERED_DTS ];
	DETOUR*							pdUniqueHitDetour;

	// Hardware Breakpoints.

	BOOLEAN							bFromInt1;
	ULONG							ulDebugRegHit;

	BOOLEAN							bINT1RequiresCodeWinPosDec;

	// Breakpoints.

	BREAKPOINT						vbpBreakpoints[ MACRO_MAXNUM_OF_BREAKPOINTS ];

	// Detours.

	DETOUR							vdDetours[ MACRO_MAXNUM_OF_DETOURS ];

	BOOLEAN							bDetoursInitFatalError;

	// MapViewOfImageSection.

	MP_SPINLOCK						mpslMapViewOfImageSectionSync;

	// NtTerminateProcess

	ULONG							ulNtTerminateProcessFnIndex;

	// "User Space Trampoline Structures":

	FAST_MUTEX						fmVpcICEProcessCallbackAccess;

	DWORD							dwUserSpaceTrampolineStructuresSize;
	DWORD							dwUserSpaceTrampolineStructuresAddress;

	//
	// === InvokeDebugger PARAMETERS ===
	//

	BOOLEAN							bCopyToPrevContext;

	x86_REGISTERS_CONTEXT			CONTEXT;

	BOOLEAN							bWasStackSwitching;

	//
	// === WORK AREA ===
	//

	// Temporary place where the Interrupt Mask Registers (IMR) of both PIC are placed.

	BYTE							bPic1IMR, bPic2IMR;

	// Temporary place where the I/O Apic Registers are placed.

	unsigned __int64				viiIORedirTableBackup[ MACRO_VPCICE_MAXIMUM_INTINPPINS_SUPPORTED ];

	// Temporary place where the IDTR is placed.

	x86_IDTReg						xirIDTR;

	// Temporary place where the GDTR is placed.

	x86_GDTReg						xgrGDTR;

	// Temporary place where the Informations about the Jump Insertion are placed.

	DELAY_TARGET_JUMP_INSERTION		dtjiJmpInsertInfo_Mouse;
	DELAY_TARGET_JUMP_INSERTION		dtjiJmpInsertInfo;

	// Temporary place for the Keyboard I/O Redirection Table Register (low 32 bits);

	DWORD							dwPrevMouseIoRedTRLow32;
	DWORD							dwPrevKeybIoRedTRLow32;

	// Temporary place for storing the received Keyboard Scancode.

	BYTE							bKeyboardScanCode;

	// Temporary place for the CR0 Register.

	DWORD							dwCR0;

	// Temporary place of FPU Restore Informations.

	x86_FPU_RESTOREINFS				x86friFpuRestoreInfs;

	// Temporary place for the Context Structure Copy.

	x86_REGISTERS_CONTEXT			x86vicContextCopy;

} SYSINTERRUPTS_STATUS, *PSYSINTERRUPTS_STATUS;

// // //

#define MACRO_ISCONTEXTCOMPATIBLE_CACHEINFOS_PROCESSNAME_FIELD_SIZE		( 0x10 + 1 ) // MACRO_KPEB_IMGFLNAME_FIELD_SIZE

//

typedef struct _ISCONTEXTCOMPATIBLE_CACHEINFOS // ### WARNING: you must initialize (zeroing its memory) this structure before use !!! ###
{
	BOOLEAN					bProcessNameInit;
	CHAR					szProcessName[ MACRO_ISCONTEXTCOMPATIBLE_CACHEINFOS_PROCESSNAME_FIELD_SIZE ];

	BOOLEAN					bModuleNameInit;
	CHAR					szModuleName[ MACRO_CONTEXTINFO_MODULENAME_FIELD_SIZE ];
	DWORD					dwModuleStart;

} ISCONTEXTCOMPATIBLE_CACHEINFOS, *PISCONTEXTCOMPATIBLE_CACHEINFOS;

// // //

#define MACRO_IDOB_OBJECTTYPE_DETOUR				0
#define MACRO_IDOB_OBJECTTYPE_BREAKPOINT			1

typedef struct _INSTALLDETOURORBREAKPOINT_PARAMS
{
	// # Type. #

	ULONG					ulObjectType;

	// # Common. #

	SYSINTERRUPTS_STATUS*	psisSysInterrStatus;
	ULONG					ulType;
	DWORD					dwAddress;
	BOOLEAN					bUseSEH;
	CHAR*					pszProcessName;
	CHAR*					pszModuleName;

	// # ONLY DETOURS. #

	VOID*					pvNewFunc;

	// # ONLY BREAKPOINTS. #

	ULONG					ulDebugReg;

} INSTALLDETOURORBREAKPOINT_PARAMS, *PINSTALLDETOURORBREAKPOINT_PARAMS;

//==========
// Externs.
//==========

extern SYSINTERRUPTS_STATUS*		g_psisSysInterrStatus;
extern CHAR							g_szAutoTypedCommand[ MACRO_AUTOTYPEDCOMMAND_MAXSIZE ];

//======================
// Function Prototypes.
//======================

NTSTATUS InstallVpcICEInterruptHooks ( OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID GetKeyboardISRLinearAddress( IN OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID HookKeyboardISR( IN OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID SendEOIToPIC1( VOID );
VOID SendEOIToPIC2( VOID );
VOID MaskInterrupts( IN BYTE bPIC1Mask, IN BYTE bPIC2Mask, OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID RestoreInterrupts( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID EnableInterrupts( VOID );
VOID DisableInterrupts( VOID );
VOID SendEOIToLocalAPIC( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID DebuggerLoopQuantum( IN LARGE_INTEGER liCpuCycles, IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID CalculateCPUSpeed( OUT LARGE_INTEGER* pliCpuClockCyclesPerSecond );
KIRQL GetCurrentIrqlFromLocalAPIC( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID SetLocalAPIC_TPR( IN DWORD dwTPRNewValue, IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID RestoreLocalAPIC_TPR( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID GetVpcICEIPIISRLinearAddress( IN OUT SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID SendVpcICEIPI( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
VOID SaveCpuContext( VOID /* "IN SYSINTERRUPTS_STATUS* psisSysInterrStatus" in EBX Register. "OUT x86_FPU_RESTOREINFS* px86friFpuRestoreInfs" in EAX Register. Returns "x86vicContext" Field Offset in EAX. */ );
VOID IncrementNumProcsWithContextFilled( VOID /* "IN SYSINTERRUPTS_STATUS* psisSysInterrStatus" in EBX Register. */ );
VOID ReorderListOfProcessors( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus );
KIRQL GetIrqlFromTPRValue( IN DWORD dwTPRValue );
VOID RestoreFPUState( VOID /* "IN x86_FPU_RESTOREINFS* px86friFpuRestoreInfs" in EBX Register. */ );
BOOLEAN InstallIDTHook( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulVectorNum, IN DWORD dwNewISRAddress );
VOID InvokeDebugger( VOID /* parameters: InvokeDebugger section of SYSINTERRUPTS_STATUS. */ );
VOID VpcICEDebugExceptionHandler( VOID );
VOID VpcICEBreakpointExceptionHandler( VOID );
BOOLEAN InstallDetour( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulType, IN DWORD dwAddress, IN VOID* pvNewFunc, IN BOOLEAN bUseSEH, IN CHAR* pszProcessName, IN CHAR* pszModuleName );
DETOUR*	GetDetourByType( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulType );
VOID EnableDetoursAndBreakpoints( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN BOOLEAN bEnable );
VOID NEW_MapViewOfImageSection( VOID );
VOID NEW_UnMapViewOfImageSection( VOID );
VOID __cdecl SetMapViewOfImageSectionFnPtr( DWORD dwRetAddressOnStackPtr );
VOID __cdecl VpcICEUnMapViewOfImageSection( DWORD dwRetAddressOnStackPtr );
VOID MapViewOfImageSectionCallBack( VOID );
VOID VpcICESystemServiceInterruptHandler( VOID );
BOOLEAN InstallBreakpoint( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN ULONG ulType, IN DWORD dwAddress, IN ULONG ulDebugReg, IN CHAR* pszProcessName, IN CHAR* pszModuleName );
BOOLEAN AreMemoryRangesConflicting( IN DWORD dwAddress0, IN ULONG ulSize0, IN DWORD dwAddress1, IN ULONG ulSize1 );
BOOLEAN AreContextesConflicting( IN CONTEXT_INFO* pciContext0, IN ULONG ulSize0, IN CONTEXT_INFO* pciContext1, IN ULONG ulSize1 );
VOID VpcICEProcessCallback( IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create );
BOOLEAN IsContextCompatible( IN CONTEXT_INFO* pciContext, IN OUT DWORD* pdwCurrCntxtAddress, IN BYTE* pbKpeb, IN VOID* pvPCRAddress, IN OUT ISCONTEXTCOMPATIBLE_CACHEINFOS* pCacheInfos, IN CHAR* pszDirectModuleNameCompare, IN DWORD dwDirectCompareModuleStart );
BOOLEAN InstallDetourOrBreakpoint( IN INSTALLDETOURORBREAKPOINT_PARAMS* pidobParams );
VOID InitEnableXXXCache ();
DWORD EnableCopyOnWrite( IN BOOLEAN bEnable, IN DWORD dwEnableCookie );
VOID RegenerateTrampoline( IN PVOID pvTrampAddress, IN PVOID pvGetVaFuncAddress );
ULONG GetUserSpaceTrampolineStructuresSize( ULONG ulEntriesNum /* ### ...THIS IS FIXED TO MACRO_MAXNUM_OF_DETOURS... ### */ );
VOID InitializeUserSpaceTrampolineStructures( IN VOID* pvMemPtr );
BOOLEAN AllocateUserSpaceTrampolineStructures( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN BYTE* pbKpeb, IN DWORD dwProcessId );
VOID AllocateUserSpaceTrampolineStructuresInAllProcesses( VOID );
VOID DeleteBreakpoint( IN BREAKPOINT* pbBP );
VOID DeleteDetour( IN DETOUR* pdDT );
BOOLEAN ContextProcName2KpebLikeProcName( OUT CHAR* pszKpebLikeProcName, IN CHAR* pszContextProcName );
VOID InitializePFMDB( VOID );
BOOLEAN AddFrameToPFMDB( IN VOID* pvVirtualAddress, IN ULONG ulBytesNum );
BOOLEAN RemoveFrameFromPFMDB( IN VOID* pvVirtualAddress, IN ULONG ulBytesNum );
BOOLEAN CheckOutFramesInPFMDB( IN DWORD dwTestPhysAddr );
VOID TimerThreadEntryPoint( IN PVOID pvParam );
VOID TimerThreadDebuggerEntrance( VOID );
BOOLEAN AllocateUserSpaceMemory( IN SYSINTERRUPTS_STATUS* psisSysInterrStatus, IN BYTE* pbKpeb, IN DWORD dwProcessId );
VOID InitializeUserSpaceMemory( IN VOID* pvMemPtr );
ULONG GetUserSpaceMemorySize( VOID );
VOID* GetInterruptVectorHandlerAddress( IN ULONG ulVector, IN ULONG ulProcessor );
VOID EnableInterruptBreakpoint( IN BREAKPOINT* pbBP, IN BOOLEAN bEnable );
VOID InitializePs2MouseKeybConsistenceSys( VOID );
VOID CheckPs2MouseKeybConsistence( VOID );
VOID GetUpdatedDebugRegisterValues( VOID );
VOID ResetDebugRegisters( VOID );
BOOLEAN CheckIfMemoryBreakpointIsCompatible( IN BREAKPOINT* pbThis );
BOOLEAN CheckIfIoBreakpointIsCompatible( IN BREAKPOINT* pbThis );
BOOLEAN GetBreakpointDescription( OUT CHAR* pszOutputBuffer, OUT CHAR* pszAddInfoBuffer, IN BREAKPOINT* pbThis );
VOID EnableBreakpoint( IN BREAKPOINT* pbBP );
VOID DisableBreakpoint( IN BREAKPOINT* pbBP );
VOID PrintBreakpointHitMessage( IN BREAKPOINT* pbBP, IN BOOLEAN bPrintBranchInfo );
BOOLEAN AddressIsUser2KernelGate( IN DWORD dwAddress );
BOOLEAN HandleUser2KernelBkp( IN OUT x86_REGISTERS_CONTEXT* prcParams, IN BOOLEAN bFirstChance );
VOID HandleTimerThreadEvent( IN OUT x86_REGISTERS_CONTEXT* prcParams );
VOID RestoreKeybLEDs ( VOID );
BOOLEAN IsClientPollingForDebuggerVerb ( VOID );
