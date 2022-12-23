/***************************************************************************************
  *
  * detours.h - VPCICE "Microsoft Research Detours" Kernel Mode Version by Vito Plantamura. Header File.
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

//===========
// Includes.
//===========

#include <ntddk.h>
#include "..\Include\WinDefs.h"

//=====================
// Structures / Types.
//=====================

typedef int INT32;

struct CDetourDis__COPYENTRY;
typedef const struct CDetourDis__COPYENTRY * CDetourDis__REFCOPYENTRY;

typedef PBYTE (* CDetourDis__COPYFUNC)(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,    PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);

enum {
	CDetourDis__DYNAMIC 	= 0x1u,
	CDetourDis__ADDRESS 	= 0x2u,
	CDetourDis__NOENLARGE	= 0x4u,

	CDetourDis__SIB			= 0x10u,
	CDetourDis__NOTSIB		= 0x0fu,
};

struct CDetourDis__COPYENTRY 
{
	ULONG 					nOpcode 		: 8;				// Opcode
	ULONG					nFixedSize 		: 3;				// Fixed size of opcode
	ULONG					nFixedSize16 	: 3;				// Fixed size when 16 bit operand
	ULONG					nModOffset 		: 3;				// Offset to mod/rm byte (0=none)
	LONG					nRelOffset 		: 3;				// Offset to relative target.
	ULONG					nFlagBits		: 4;				// Flags for DYNAMIC, etc.
	CDetourDis__COPYFUNC	pfCopy;								// Function pointer.
};

//=========
// Macros.
//=========

#define DETOUR_INSTRUCTION_TARGET_NONE          ((PBYTE)0)
#define DETOUR_INSTRUCTION_TARGET_DYNAMIC       ((PBYTE)~0ul)

#define DETOUR_TRAMPOLINE_SIZE          32

#define DETOUR_TRAMPOLINE(trampoline,target) \
static PVOID __fastcall _Detours_GetVA_##target(VOID) \
{ \
    return &target; \
} \
\
__declspec(naked) trampoline \
{ \
    __asm { nop };\
    __asm { nop };\
    __asm { call _Detours_GetVA_##target };\
    __asm { jmp eax };\
    __asm { ret };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
}

#define DETOUR_TRAMPOLINE_GLOBVAR(trampoline,target) \
static PVOID __fastcall _Detours_GetVA_##target(VOID) \
{ \
    return (PVOID) target; \
} \
\
__declspec(naked) trampoline \
{ \
    __asm { nop };\
    __asm { nop };\
    __asm { call _Detours_GetVA_##target };\
    __asm { jmp eax };\
    __asm { ret };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
}

//===================
// Inline Functions.
//===================

/*inline*/ PBYTE DetourGenJmp(PBYTE pbCode, PBYTE pbJmpDst, PBYTE pbJmpSrc /*= 0*/);
/*inline*/ PBYTE DetourGenBreak(PBYTE pbCode);

//===================================================
// DELAY_TARGET_JUMP_INSERTION Structure Definition.
//===================================================

typedef struct _DELAY_TARGET_JUMP_INSERTION
{
	PBYTE		pbCode;
	PBYTE		pbDest;
	LONG		cbCode;

} DELAY_TARGET_JUMP_INSERTION, *PDELAY_TARGET_JUMP_INSERTION;

//======================
// Function Prototypes.
//======================

BOOLEAN /*WINAPI*/ DetourFunctionWithTrampoline(PBYTE pbTrampoline,
                                          PBYTE pbDetour,
										  DELAY_TARGET_JUMP_INSERTION* pdtjiDelayJmpInsert );

BOOLEAN /*WINAPI*/ DetourFunctionWithTrampolineEx(PBYTE pbTrampoline,
                                            PBYTE pbDetour,
                                            PBYTE *ppbRealTrampoline,
                                            PBYTE *ppbRealTarget,
											DELAY_TARGET_JUMP_INSERTION* pdtjiDelayJmpInsert );

PBYTE /*WINAPI*/ DetourGetFinalCode(PBYTE pbCode, BOOLEAN fSkipJmp);

PBYTE /*WINAPI*/ DetourCopyInstruction(PBYTE pbDst, PBYTE pbSrc, PBYTE *ppbTarget);

PBYTE CDetourDis__CopyBytes(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__CopyBytesPrefix(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__Copy0F(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__Copy66(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__Copy67(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__CopyF6(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__CopyF7(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__CopyFF(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);
PBYTE CDetourDis__Invalid(CDetourDis__REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc,   PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);

PBYTE CDetourDis__CopyInstruction(PBYTE pbDst, PBYTE pbSrc,     PBYTE * m_ppbTarget, LONG * m_plExtra, BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress);

PBYTE CDetourDis__AdjustTarget(PBYTE pbDst, PBYTE pbSrc, LONG cbOp, LONG cbTargetOffset,        LONG * m_plExtra);

VOID CDetourDis__Set16BitOperand( BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress );
VOID CDetourDis__Set16BitAddress( BOOLEAN * m_b16BitOperand, BOOLEAN * m_b16BitAddress );
