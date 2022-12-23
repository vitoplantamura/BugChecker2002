/***************************************************************************************
  *
  * disasm.h - VPCICE Disassembler Routines Header File.
  *
  * INFO: Extracted and Derived from Source Code of NASM Project (http://sourceforge.net/projects/nasm).
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
#include "WinDefs.h"

//==============
// Definitions.
//==============

/*
 * Special values for expr->type. ASSUMPTION MADE HERE: the number
 * of distinct register names (i.e. possible "type" fields for an
 * expr structure) does not exceed 124 (EXPR_REG_START through
 * EXPR_REG_END).
 */
#define EXPR_REG_START 1
#define EXPR_REG_END 124
#define EXPR_UNKNOWN 125L	       /* for forward references */
#define EXPR_SIMPLE 126L
#define EXPR_WRT 127L
#define EXPR_SEGBASE 128L

/* automatically generated from ./regs.dat - do not edit */
enum reg_enum {
    R_AH = EXPR_REG_START,
    R_AL,
    R_AX,
    R_BH,
    R_BL,
    R_BP,
    R_BX,
    R_CH,
    R_CL,
    R_CR0,
    R_CR1,
    R_CR2,
    R_CR3,
    R_CR4,
    R_CR5,
    R_CR6,
    R_CR7,
    R_CS,
    R_CX,
    R_DH,
    R_DI,
    R_DL,
    R_DR0,
    R_DR1,
    R_DR2,
    R_DR3,
    R_DR4,
    R_DR5,
    R_DR6,
    R_DR7,
    R_DS,
    R_DX,
    R_EAX,
    R_EBP,
    R_EBX,
    R_ECX,
    R_EDI,
    R_EDX,
    R_ES,
    R_ESI,
    R_ESP,
    R_FS,
    R_GS,
    R_MM0,
    R_MM1,
    R_MM2,
    R_MM3,
    R_MM4,
    R_MM5,
    R_MM6,
    R_MM7,
    R_SEGR6,
    R_SEGR7,
    R_SI,
    R_SP,
    R_SS,
    R_ST0,
    R_ST1,
    R_ST2,
    R_ST3,
    R_ST4,
    R_ST5,
    R_ST6,
    R_ST7,
    R_TR0,
    R_TR1,
    R_TR2,
    R_TR3,
    R_TR4,
    R_TR5,
    R_TR6,
    R_TR7,
    R_XMM0,
    R_XMM1,
    R_XMM2,
    R_XMM3,
    R_XMM4,
    R_XMM5,
    R_XMM6,
    R_XMM7,
    REG_ENUM_LIMIT
};

enum {				       /* condition code names */
    C_A, C_AE, C_B, C_BE, C_C, C_E, C_G, C_GE, C_L, C_LE, C_NA, C_NAE,
    C_NB, C_NBE, C_NC, C_NE, C_NG, C_NGE, C_NL, C_NLE, C_NO, C_NP,
    C_NS, C_NZ, C_O, C_P, C_PE, C_PO, C_S, C_Z
};

/*
 * Instruction template flags. These specify which processor
 * targets the instruction is eligible for, whether it is
 * privileged or undocumented, and also specify extra error
 * checking on the matching of the instruction.
 *
 * IF_SM stands for Size Match: any operand whose size is not
 * explicitly specified by the template is `really' intended to be
 * the same size as the first size-specified operand.
 * Non-specification is tolerated in the input instruction, but
 * _wrong_ specification is not.
 *
 * IF_SM2 invokes Size Match on only the first _two_ operands, for
 * three-operand instructions such as SHLD: it implies that the
 * first two operands must match in size, but that the third is
 * required to be _unspecified_.
 *
 * IF_SB invokes Size Byte: operands with unspecified size in the
 * template are really bytes, and so no non-byte specification in
 * the input instruction will be tolerated. IF_SW similarly invokes
 * Size Word, and IF_SD invokes Size Doubleword.
 *
 * (The default state if neither IF_SM nor IF_SM2 is specified is
 * that any operand with unspecified size in the template is
 * required to have unspecified size in the instruction too...)
 */

#define IF_SM     0x00000001UL	       /* size match */
#define IF_SM2    0x00000002UL	       /* size match first two operands */
#define IF_SB     0x00000004UL	       /* unsized operands can't be non-byte */
#define IF_SW     0x00000008UL	       /* unsized operands can't be non-word */
#define IF_SD     0x00000010UL	       /* unsized operands can't be nondword */
#define IF_AR0	  0x00000020UL	       /* SB, SW, SD applies to argument 0 */
#define IF_AR1	  0x00000040UL	       /* SB, SW, SD applies to argument 1 */
#define IF_AR2	  0x00000060UL	       /* SB, SW, SD applies to argument 2 */
#define IF_ARMASK 0x00000060UL         /* mask for unsized argument spec */
#define IF_PRIV   0x00000100UL	       /* it's a privileged instruction */
#define IF_SMM    0x00000200UL	       /* it's only valid in SMM */
#define IF_PROT   0x00000400UL         /* it's protected mode only */
#define IF_UNDOC  0x00001000UL	       /* it's an undocumented instruction */
#define IF_FPU    0x00002000UL	       /* it's an FPU instruction */
#define IF_MMX    0x00004000UL	       /* it's an MMX instruction */
#define IF_3DNOW  0x00008000UL	       /* it's a 3DNow! instruction */
#define IF_SSE    0x00010000UL	       /* it's a SSE (KNI, MMX2) instruction */
#define IF_SSE2   0x00020000UL	       /* it's a SSE2 instruction */
#define IF_SSE3   0x00040000UL	       /* it's a SSE3 (PNI) instruction */
#define IF_PMASK  0xFF000000UL	       /* the mask for processor types */
#define IF_PLEVEL 0x0F000000UL         /* the mask for processor instr. level */
					/* also the highest possible processor */
#define IF_PFMASK 0xF001FF00UL	       /* the mask for disassembly "prefer" */
#define IF_8086   0x00000000UL	       /* 8086 instruction */
#define IF_186    0x01000000UL	       /* 186+ instruction */
#define IF_286    0x02000000UL	       /* 286+ instruction */
#define IF_386    0x03000000UL	       /* 386+ instruction */
#define IF_486    0x04000000UL	       /* 486+ instruction */
#define IF_PENT   0x05000000UL	       /* Pentium instruction */
#define IF_P6     0x06000000UL	       /* P6 instruction */
#define IF_KATMAI 0x07000000UL         /* Katmai instructions */
#define IF_WILLAMETTE 0x08000000UL         /* Willamette instructions */
#define IF_PRESCOTT   0x09000000UL     /* Prescott instructions */
#define IF_IA64   0x0F000000UL	       /* IA64 instructions */
#define IF_CYRIX  0x10000000UL	       /* Cyrix-specific instruction */
#define IF_AMD    0x20000000UL	       /* AMD-specific instruction */

/*
 * Note that because segment registers may be used as instruction
 * prefixes, we must ensure the enumerations for prefixes and
 * register names do not overlap.
 */
enum {				       /* instruction prefixes */
    PREFIX_ENUM_START = REG_ENUM_LIMIT,
    P_A16 = PREFIX_ENUM_START, P_A32, P_LOCK, P_O16, P_O32, P_REP, P_REPE,
    P_REPNE, P_REPNZ, P_REPZ, P_TIMES
};

/*
 * Here we define the operand types. These are implemented as bit
 * masks, since some are subsets of others; e.g. AX in a MOV
 * instruction is a special operand type, whereas AX in other
 * contexts is just another 16-bit register. (Also, consider CL in
 * shift instructions, DX in OUT, etc.)
 */

/* size, and other attributes, of the operand */
#define BITS8     0x00000001L
#define BITS16    0x00000002L
#define BITS32    0x00000004L
#define BITS64    0x00000008L	       /* FPU only */
#define BITS80    0x00000010L	       /* FPU only */
#define __FAR       0x00000020L	       /* grotty: this means 16:16 or */
				       /* 16:32, like in CALL/JMP */
#define NEAR      0x00000040L
#define SHORT     0x00000080L	       /* and this means what it says :) */

#define SIZE_MASK 0x000000FFL	       /* all the size attributes */
#define NON_SIZE  (~SIZE_MASK)

#define TO        0x00000100L          /* reverse effect in FADD, FSUB &c */
#define COLON     0x00000200L	       /* operand is followed by a colon */
//#define STRICT    0x00000400L	       /* do not optimize this operand */

/* type of operand: memory reference, register, etc. */
#define MEMORY    0x00204000L
#define REGISTER  0x00001000L	       /* register number in 'basereg' */
#define IMMEDIATE 0x00002000L

#define REGMEM    0x00200000L	       /* for r/m, ie EA, operands */
#define REGNORM   0x00201000L	       /* 'normal' reg, qualifies as EA */
#define REG8      0x00201001L
#define REG16     0x00201002L
#define REG32     0x00201004L
#define MMXREG    0x00201008L	       /* MMX registers */
#define XMMREG    0x00201010L          /* XMM Katmai reg */
#define FPUREG    0x01000000L	       /* floating point stack registers */
#define FPU0      0x01000800L	       /* FPU stack register zero */

/* special register operands: these may be treated differently */
#define REG_SMASK 0x00070000L	       /* a mask for the following */
#define REG_ACCUM 0x00211000L	       /* accumulator: AL, AX or EAX */
#define REG_AL    0x00211001L	       /* REG_ACCUM | BITSxx */
#define REG_AX    0x00211002L	       /* ditto */
#define REG_EAX   0x00211004L	       /* and again */
#define REG_COUNT 0x00221000L	       /* counter: CL, CX or ECX */
#define REG_CL    0x00221001L	       /* REG_COUNT | BITSxx */
#define REG_CX    0x00221002L	       /* ditto */
#define REG_ECX   0x00221004L	       /* another one */
#define REG_DL    0x00241001L
#define REG_DX    0x00241002L
#define REG_EDX   0x00241004L
#define REG_SREG  0x00081002L	       /* any segment register */
#define REG_CS    0x01081002L	       /* CS */
#define REG_DESS  0x02081002L	       /* DS, ES, SS (non-CS 86 registers) */
#define REG_FSGS  0x04081002L	       /* FS, GS (386 extended registers) */
#define REG_SEG67 0x08081002L          /* Non-implemented segment registers */
#define REG_CDT   0x00101004L	       /* CRn, DRn and TRn */
#define REG_CREG  0x08101004L	       /* CRn */
#define REG_DREG  0x10101004L	       /* DRn */
#define REG_TREG  0x20101004L	       /* TRn */

/* special type of EA */
#define MEM_OFFS  0x00604000L	       /* simple [address] offset */

/* special type of immediate operand */
#define ONENESS   0x00800000L          /* so UNITY == IMMEDIATE | ONENESS */
#define UNITY     0x00802000L	       /* for shift/rotate instructions */
#define BYTENESS  0x40000000L          /* so SBYTE == IMMEDIATE | BYTENESS */
#define SBYTE 	  0x40002000L	       /* for op r16/32,immediate instrs. */

/*
 * Flags that go into the `segment' field of `insn' structures
 * during disassembly.
 */
#define SEG_RELATIVE 1
#define SEG_32BIT 2
#define SEG_RMREG 4
#define SEG_DISP8 8
#define SEG_DISP16 16
#define SEG_DISP32 32
#define SEG_NODISP 64
#define SEG_SIGNED 128

enum {
	I_AAA,
	I_AAD,
	I_AAM,
	I_AAS,
	I_ADC,
	I_ADD,
	I_ADDPD,
	I_ADDPS,
	I_ADDSD,
	I_ADDSS,
	I_ADDSUBPD,
	I_ADDSUBPS,
	I_AND,
	I_ANDNPD,
	I_ANDNPS,
	I_ANDPD,
	I_ANDPS,
	I_ARPL,
	I_BOUND,
	I_BSF,
	I_BSR,
	I_BSWAP,
	I_BT,
	I_BTC,
	I_BTR,
	I_BTS,
	I_CALL,
	I_CBW,
	I_CDQ,
	I_CLC,
	I_CLD,
	I_CLFLUSH,
	I_CLI,
	I_CLTS,
	I_CMC,
	I_CMP,
	I_CMPEQPD,
	I_CMPEQPS,
	I_CMPEQSD,
	I_CMPEQSS,
	I_CMPLEPD,
	I_CMPLEPS,
	I_CMPLESD,
	I_CMPLESS,
	I_CMPLTPD,
	I_CMPLTPS,
	I_CMPLTSD,
	I_CMPLTSS,
	I_CMPNEQPD,
	I_CMPNEQPS,
	I_CMPNEQSD,
	I_CMPNEQSS,
	I_CMPNLEPD,
	I_CMPNLEPS,
	I_CMPNLESD,
	I_CMPNLESS,
	I_CMPNLTPD,
	I_CMPNLTPS,
	I_CMPNLTSD,
	I_CMPNLTSS,
	I_CMPORDPD,
	I_CMPORDPS,
	I_CMPORDSD,
	I_CMPORDSS,
	I_CMPPD,
	I_CMPPS,
	I_CMPSB,
	I_CMPSD,
	I_CMPSS,
	I_CMPSW,
	I_CMPUNORDPD,
	I_CMPUNORDPS,
	I_CMPUNORDSD,
	I_CMPUNORDSS,
	I_CMPXCHG,
	I_CMPXCHG486,
	I_CMPXCHG8B,
	I_COMISD,
	I_COMISS,
	I_CPUID,
	I_CVTDQ2PD,
	I_CVTDQ2PS,
	I_CVTPD2DQ,
	I_CVTPD2PI,
	I_CVTPD2PS,
	I_CVTPI2PD,
	I_CVTPI2PS,
	I_CVTPS2DQ,
	I_CVTPS2PD,
	I_CVTPS2PI,
	I_CVTSD2SI,
	I_CVTSD2SS,
	I_CVTSI2SD,
	I_CVTSI2SS,
	I_CVTSS2SD,
	I_CVTSS2SI,
	I_CVTTPD2DQ,
	I_CVTTPD2PI,
	I_CVTTPS2DQ,
	I_CVTTPS2PI,
	I_CVTTSD2SI,
	I_CVTTSS2SI,
	I_CWD,
	I_CWDE,
	I_DAA,
	I_DAS,
	I_DB,
	I_DD,
	I_DEC,
	I_DIV,
	I_DIVPD,
	I_DIVPS,
	I_DIVSD,
	I_DIVSS,
	I_DQ,
	I_DT,
	I_DW,
	I_EMMS,
	I_ENTER,
	I_EQU,
	I_F2XM1,
	I_FABS,
	I_FADD,
	I_FADDP,
	I_FBLD,
	I_FBSTP,
	I_FCHS,
	I_FCLEX,
	I_FCMOVB,
	I_FCMOVBE,
	I_FCMOVE,
	I_FCMOVNB,
	I_FCMOVNBE,
	I_FCMOVNE,
	I_FCMOVNU,
	I_FCMOVU,
	I_FCOM,
	I_FCOMI,
	I_FCOMIP,
	I_FCOMP,
	I_FCOMPP,
	I_FCOS,
	I_FDECSTP,
	I_FDISI,
	I_FDIV,
	I_FDIVP,
	I_FDIVR,
	I_FDIVRP,
	I_FEMMS,
	I_FENI,
	I_FFREE,
	I_FFREEP,
	I_FIADD,
	I_FICOM,
	I_FICOMP,
	I_FIDIV,
	I_FIDIVR,
	I_FILD,
	I_FIMUL,
	I_FINCSTP,
	I_FINIT,
	I_FIST,
	I_FISTP,
	I_FISTTP,
	I_FISUB,
	I_FISUBR,
	I_FLD,
	I_FLD1,
	I_FLDCW,
	I_FLDENV,
	I_FLDL2E,
	I_FLDL2T,
	I_FLDLG2,
	I_FLDLN2,
	I_FLDPI,
	I_FLDZ,
	I_FMUL,
	I_FMULP,
	I_FNCLEX,
	I_FNDISI,
	I_FNENI,
	I_FNINIT,
	I_FNOP,
	I_FNSAVE,
	I_FNSTCW,
	I_FNSTENV,
	I_FNSTSW,
	I_FPATAN,
	I_FPREM,
	I_FPREM1,
	I_FPTAN,
	I_FRNDINT,
	I_FRSTOR,
	I_FSAVE,
	I_FSCALE,
	I_FSETPM,
	I_FSIN,
	I_FSINCOS,
	I_FSQRT,
	I_FST,
	I_FSTCW,
	I_FSTENV,
	I_FSTP,
	I_FSTSW,
	I_FSUB,
	I_FSUBP,
	I_FSUBR,
	I_FSUBRP,
	I_FTST,
	I_FUCOM,
	I_FUCOMI,
	I_FUCOMIP,
	I_FUCOMP,
	I_FUCOMPP,
	I_FWAIT,
	I_FXAM,
	I_FXCH,
	I_FXRSTOR,
	I_FXSAVE,
	I_FXTRACT,
	I_FYL2X,
	I_FYL2XP1,
	I_HADDPD,
	I_HADDPS,
	I_HLT,
	I_HSUBPD,
	I_HSUBPS,
	I_IBTS,
	I_ICEBP,
	I_IDIV,
	I_IMUL,
	I_IN,
	I_INC,
	I_INCBIN,
	I_INSB,
	I_INSD,
	I_INSW,
	I_INT,
	I_INT01,
	I_INT03,
	I_INT1,
	I_INT3,
	I_INTO,
	I_INVD,
	I_INVLPG,
	I_IRET,
	I_IRETD,
	I_IRETW,
	I_JCXZ,
	I_JECXZ,
	I_JMP,
	I_JMPE,
	I_LAHF,
	I_LAR,
	I_LDDQU,
	I_LDMXCSR,
	I_LDS,
	I_LEA,
	I_LEAVE,
	I_LES,
	I_LFENCE,
	I_LFS,
	I_LGDT,
	I_LGS,
	I_LIDT,
	I_LLDT,
	I_LMSW,
	I_LOADALL,
	I_LOADALL286,
	I_LODSB,
	I_LODSD,
	I_LODSW,
	I_LOOP,
	I_LOOPE,
	I_LOOPNE,
	I_LOOPNZ,
	I_LOOPZ,
	I_LSL,
	I_LSS,
	I_LTR,
	I_MASKMOVDQU,
	I_MASKMOVQ,
	I_MAXPD,
	I_MAXPS,
	I_MAXSD,
	I_MAXSS,
	I_MFENCE,
	I_MINPD,
	I_MINPS,
	I_MINSD,
	I_MINSS,
	I_MONITOR,
	I_MOV,
	I_MOVAPD,
	I_MOVAPS,
	I_MOVD,
	I_MOVDDUP,
	I_MOVDQ2Q,
	I_MOVDQA,
	I_MOVDQU,
	I_MOVHLPS,
	I_MOVHPD,
	I_MOVHPS,
	I_MOVLHPS,
	I_MOVLPD,
	I_MOVLPS,
	I_MOVMSKPD,
	I_MOVMSKPS,
	I_MOVNTDQ,
	I_MOVNTI,
	I_MOVNTPD,
	I_MOVNTPS,
	I_MOVNTQ,
	I_MOVQ,
	I_MOVQ2DQ,
	I_MOVSB,
	I_MOVSD,
	I_MOVSHDUP,
	I_MOVSLDUP,
	I_MOVSS,
	I_MOVSW,
	I_MOVSX,
	I_MOVUPD,
	I_MOVUPS,
	I_MOVZX,
	I_MUL,
	I_MULPD,
	I_MULPS,
	I_MULSD,
	I_MULSS,
	I_MWAIT,
	I_NEG,
	I_NOP,
	I_NOT,
	I_OR,
	I_ORPD,
	I_ORPS,
	I_OUT,
	I_OUTSB,
	I_OUTSD,
	I_OUTSW,
	I_PACKSSDW,
	I_PACKSSWB,
	I_PACKUSWB,
	I_PADDB,
	I_PADDD,
	I_PADDQ,
	I_PADDSB,
	I_PADDSIW,
	I_PADDSW,
	I_PADDUSB,
	I_PADDUSW,
	I_PADDW,
	I_PAND,
	I_PANDN,
	I_PAUSE,
	I_PAVEB,
	I_PAVGB,
	I_PAVGUSB,
	I_PAVGW,
	I_PCMPEQB,
	I_PCMPEQD,
	I_PCMPEQW,
	I_PCMPGTB,
	I_PCMPGTD,
	I_PCMPGTW,
	I_PDISTIB,
	I_PEXTRW,
	I_PF2ID,
	I_PF2IW,
	I_PFACC,
	I_PFADD,
	I_PFCMPEQ,
	I_PFCMPGE,
	I_PFCMPGT,
	I_PFMAX,
	I_PFMIN,
	I_PFMUL,
	I_PFNACC,
	I_PFPNACC,
	I_PFRCP,
	I_PFRCPIT1,
	I_PFRCPIT2,
	I_PFRSQIT1,
	I_PFRSQRT,
	I_PFSUB,
	I_PFSUBR,
	I_PI2FD,
	I_PI2FW,
	I_PINSRW,
	I_PMACHRIW,
	I_PMADDWD,
	I_PMAGW,
	I_PMAXSW,
	I_PMAXUB,
	I_PMINSW,
	I_PMINUB,
	I_PMOVMSKB,
	I_PMULHRIW,
	I_PMULHRWA,
	I_PMULHRWC,
	I_PMULHUW,
	I_PMULHW,
	I_PMULLW,
	I_PMULUDQ,
	I_PMVGEZB,
	I_PMVLZB,
	I_PMVNZB,
	I_PMVZB,
	I_POP,
	I_POPA,
	I_POPAD,
	I_POPAW,
	I_POPF,
	I_POPFD,
	I_POPFW,
	I_POR,
	I_PREFETCH,
	I_PREFETCHNTA,
	I_PREFETCHT0,
	I_PREFETCHT1,
	I_PREFETCHT2,
	I_PREFETCHW,
	I_PSADBW,
	I_PSHUFD,
	I_PSHUFHW,
	I_PSHUFLW,
	I_PSHUFW,
	I_PSLLD,
	I_PSLLDQ,
	I_PSLLQ,
	I_PSLLW,
	I_PSRAD,
	I_PSRAW,
	I_PSRLD,
	I_PSRLDQ,
	I_PSRLQ,
	I_PSRLW,
	I_PSUBB,
	I_PSUBD,
	I_PSUBQ,
	I_PSUBSB,
	I_PSUBSIW,
	I_PSUBSW,
	I_PSUBUSB,
	I_PSUBUSW,
	I_PSUBW,
	I_PSWAPD,
	I_PUNPCKHBW,
	I_PUNPCKHDQ,
	I_PUNPCKHQDQ,
	I_PUNPCKHWD,
	I_PUNPCKLBW,
	I_PUNPCKLDQ,
	I_PUNPCKLQDQ,
	I_PUNPCKLWD,
	I_PUSH,
	I_PUSHA,
	I_PUSHAD,
	I_PUSHAW,
	I_PUSHF,
	I_PUSHFD,
	I_PUSHFW,
	I_PXOR,
	I_RCL,
	I_RCPPS,
	I_RCPSS,
	I_RCR,
	I_RDMSR,
	I_RDPMC,
	I_RDSHR,
	I_RDTSC,
	I_RESB,
	I_RESD,
	I_RESQ,
	I_REST,
	I_RESW,
	I_RET,
	I_RETF,
	I_RETN,
	I_ROL,
	I_ROR,
	I_RSDC,
	I_RSLDT,
	I_RSM,
	I_RSQRTPS,
	I_RSQRTSS,
	I_RSTS,
	I_SAHF,
	I_SAL,
	I_SALC,
	I_SAR,
	I_SBB,
	I_SCASB,
	I_SCASD,
	I_SCASW,
	I_SFENCE,
	I_SGDT,
	I_SHL,
	I_SHLD,
	I_SHR,
	I_SHRD,
	I_SHUFPD,
	I_SHUFPS,
	I_SIDT,
	I_SLDT,
	I_SMI,
	I_SMINT,
	I_SMINTOLD,
	I_SMSW,
	I_SQRTPD,
	I_SQRTPS,
	I_SQRTSD,
	I_SQRTSS,
	I_STC,
	I_STD,
	I_STI,
	I_STMXCSR,
	I_STOSB,
	I_STOSD,
	I_STOSW,
	I_STR,
	I_SUB,
	I_SUBPD,
	I_SUBPS,
	I_SUBSD,
	I_SUBSS,
	I_SVDC,
	I_SVLDT,
	I_SVTS,
	I_SYSCALL,
	I_SYSENTER,
	I_SYSEXIT,
	I_SYSRET,
	I_TEST,
	I_UCOMISD,
	I_UCOMISS,
	I_UD0,
	I_UD1,
	I_UD2,
	I_UMOV,
	I_UNPCKHPD,
	I_UNPCKHPS,
	I_UNPCKLPD,
	I_UNPCKLPS,
	I_VERR,
	I_VERW,
	I_WAIT,
	I_WBINVD,
	I_WRMSR,
	I_WRSHR,
	I_XADD,
	I_XBTS,
	I_XCHG,
	I_XLAT,
	I_XLATB,
	I_XOR,
	I_XORPD,
	I_XORPS,
	I_XSTORE,
	I_CMOVcc,
	I_Jcc,
	I_SETcc
};

/* 
 * this define is used to signify the end of an itemplate 
 */
#define ITEMPLATE_END {-1,-1,{-1,-1,-1},NULL,0}

/*
 * This is a useful #define which I keep meaning to use more often:
 * the number of elements of a statically defined array.
 */

#define elements(x)     ( sizeof(x) / sizeof(*(x)) )

//=============
// Structures.
//=============

typedef struct {		       /* operand to an instruction */
    long type;			       /* type of operand */
    int addr_size;		       /* 0 means default; 16; 32 */
    int basereg, indexreg, scale;      /* registers and scale involved */
    int hintbase, hinttype;	       /* hint as to real base register */
    long segment;		       /* immediate segment, if needed */
    long offset;		       /* any immediate number */
    long wrt;			       /* segment base it's relative to */
    int eaflags;		       /* special EA flags */
    int opflags;		       /* see OPFLAG_* defines below */
} operand;

#define OPFLAG_FORWARD		1      /* operand is a forward reference */
#define OPFLAG_EXTERN		2      /* operand is an external reference */

typedef struct extop {		       /* extended operand */
    struct extop *next;		       /* linked list */
    long type;			       /* defined above */
    char *stringval;		       /* if it's a string, then here it is */
    int stringlen;		       /* ... and here's how long it is */
    long segment;		       /* if it's a number/address, then... */
    long offset;		       /* ... it's given here ... */
    long wrt;			       /* ... and here */
} extop;

#define MAXPREFIX 4

typedef struct {		       /* an instruction itself */
    char *label;		       /* the label defined, or NULL */
    int prefixes[MAXPREFIX];	       /* instruction prefixes, if any */
    int nprefix;		       /* number of entries in above */
    int opcode;			       /* the opcode - not just the string */
    int condition;		       /* the condition code, if Jcc/SETcc */
    int operands;		       /* how many operands? 0-3 
                                        * (more if db et al) */
    operand oprs[3];	   	       /* the operands, defined as above */
    extop *eops;		       /* extended operands */
    int eops_float;                    /* true if DD and floating */
    long times;			       /* repeat count (TIMES prefix) */
    int forw_ref;		       /* is there a forward reference? */
} insn;

struct itemplate {
    int opcode;			       /* the token, passed from "parser.c" */
    int operands;		       /* number of operands */
    long opd[3];		       /* bit flags for operand types */
    const char *code;		   /* the code it assembles to */
    unsigned long flags;	       /* some flags */
};

//======================
// Function Prototypes.
//======================

typedef struct _DISASM_INFO
{
	struct itemplate*	pitTemplate;
	CHAR*				pszSegOver;
	insn				iInstruction;

} DISASM_INFO, *PDISASM_INFO;

long disasm (unsigned char *data, char *output, int segsize, long offset,
	     int autosync, unsigned long prefer, DISASM_INFO* pdaiRetInfo );
