/***************************************************************************************
  *
  * vpcice.h - Kernel Module Primary Header File.
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
#include "..\Include\Video.h"
#include "..\Include\apic.h"
#include "..\Include\disasm.h"
#include "..\Include\BugChkDatDefs.h"

//==============
// Definitions.
//==============

// General.

#define MACRO_PROGRAM_NAME					"BugChecker"

// System.

#ifndef MACRO_SYSTEM_PAGE_SIZE
#define MACRO_SYSTEM_PAGE_SIZE				( 0x1000 )
#endif

// Console.

#define MACRO_CONSOLE_DEFAULT_WIDTH			80
#define MACRO_CONSOLE_DEFAULT_HEIGHT		25
#define MACRO_CONSOLE_DEFAULT_WIDTH_MIN		71
#define MACRO_CONSOLE_DEFAULT_HEIGHT_MIN	20

// Code Window.

#define MACRO_CODEBYTES_MAXNUM				10

// Mem Files.

#define MACRO_MEMFILE_RESERVDMEM_SIZE		( 72 * 1024 )
#define MACRO_MEMFILE_RESERVDMEM_STEP		( MACRO_MEMFILE_RESERVDMEM_SIZE / 3 )	// Note: This value should be the Reserved Memory Amount /
																					//	the Number of "Write" Files ("wt", "wb" and "w+x") that
																					//	the compiler creates.

// MiniC Compilation.

#define MACRO_MINICSOURCE_FILENAME			"___source___.c"
#define MACRO_MINICINCLUDE_FILENAME			"bugchk_priv.h"
#define MACRO_MINICOBJECT_FILENAME			"___object___.obj"
#define MACRO_MINICPSI_FILENAME				"___object___.psi"

#define MACRO_MINIC_DEFINITIONS				""

#define MACRO_MINICCOMP_MEMFACTOR			0.2f

// Debugger System Stack.

#define MACRO_VPCICE_SYSSTACK_SIZE			( 128 * 1024 )

// I3HERE Command.

#define MACRO_I3HERE_ON						0
#define MACRO_I3HERE_OFF					1
#define MACRO_I3HERE_DRV					2

// Command Option.

#define MACRO_CMDOPT( opt )		{ opt, NULL, 0 }
#define MACRO_CMDOPT_END		{ NULL, NULL, 0 }

#define MACRO_CMDOPT_POSERR		0xFFFFFFFF

// SnapShot Mode.

#define MACRO_SNAPSHOTMODE_ON_STRING		"SNAPSHOTMODEON"
#define MACRO_SNAPSHOTMODE_OFF_STRING		"SNAPSHOTMODEOFF"

// Report Mode.

#define MACRO_REPORTMODE_STATUSMSG			"     Press any key to continue; Esc to cancel; Enter to step."

// ParseHexNumSeriesString function.

#define MACRO_PARSEHEXNUMSERIESSTRING_ERR	0xFFFFFFFF

// Timer Thread.

#define MACRO_TIMERTHREAD_DEFAULT_INTERVAL	( - ( 30L * 1000L * 10000L ) )	//
																			// The Default wait Time is 30 Seconds (minus (-)
																			//  means "Relative Time").
																			//

// Script Window.

#define MACRO_SCRIPTWIN_LINESIZE_IN_CHARS	256

#define MACRO_SCRIPTWIN_SYNTCOLOR_KEYWORD	0x3
#define MACRO_SCRIPTWIN_SYNTCOLOR_COMMENT	0x2
#define MACRO_SCRIPTWIN_SYNTCOLOR_PREPROC	0x4
#define MACRO_SCRIPTWIN_SYNTCOLOR_STRING	0x6

// BugChk.Dat File.

#define MACRO_BUGCHKDAT_COMPLETE_PATH		"\\SystemRoot\\system32\\drivers\\" MACRO_BUGCHKDAT_FILENAME
#define MACRO_BUGCHKDAT_COMPLETE_PATH_U		L"\\SystemRoot\\system32\\drivers\\" MACRO_BUGCHKDAT_FILENAME_U

//========
// Types.
//========

typedef VOID ( __cdecl * PFN_INITIALIZATION )( VOID );
typedef VOID ( __cdecl * PFN_SINGLEFUNCTION )( VOID );

// //

typedef VOID ( * PFNCOMMANDHANDLER )( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );

//=============
// Structures.
//=============

// --> Symbol Table Structure Definition. <--

typedef struct _SYMTABLEINFO
{
	U2KINT_SYMTABLEINFO			general;
	ULONG						offset;

} SYMTABLEINFO, *PSYMTABLEINFO;

#define MACRO_SYMTABLES_MAXNUM		0x50

// --> Device Extension Structure Definition. <--

typedef struct _DEVICE_EXTENSION 
{
	// === Informations about the Operating System. ===

	// General.

	VOID*					pvNtoskrnlDriverSection;

	// APIs.

	VOID*					pvMapViewOfImageSectionFnPtr;
	VOID*					pvUnMapViewOfImageSectionFnPtr;

	// Processes.

	PROCESSLIST_INFO		pliProcessListInfo;

	// === Informations about the Current Screen Resolution, VPCICE Video Memory and Text Video Buffer. ===

	PVOID					pvVpcICEVideoBuffer;
	ULONG					ulVpcICEVideoBufferSizeInBytes;

	VIDEOMEMORYINFO*		pvmiVideoMemoryInfo;
	VIDEOMEMORYINFO			vmiPrevVideoMemoryInfo;

	DRAWGLYPH_LAYOUTS		dglLayouts;

	// === Informations about the Code Window. ===

	// General.

	DWORD					dwCodeWindowPos;

	BOOLEAN					bCursorInCodeWinMode;

	DWORD					dwCodeWinLastCursorPosY;

	// First Hidden Instruction.

	BOOLEAN					b1stHiddenInstrInfoIsValid;
	DWORD					dw1stHiddenInstrAddress;

	// Highlighted Instruction Disasm Informations.

	DISASM_INFO				daiHighlightedInstr;

	DWORD					dwTargetAddress;

	// === Informations about the Script Window. ===

	// Storage Memory.

	WORD*					pwScriptWinBuffer;	// # The buffer is mantained in Words, because of Syntax Coloring. #
	ULONG					ulScriptWinBufferSizeInBytes;

	WORD**					ppwScriptWinStrPtrsBuffer;
	ULONG					ulScriptWinStrPtrsBufferSizeInBytes;

	// Pointers in Storage Mem.

	ULONG					ulScriptWinBufferPosInBytes;
	ULONG					ulScriptWinStrPtrsBufferPosInBytes;

	// Clipboard Storage Memory.

	WORD*					pwScriptWinClipBuffer;
	ULONG					ulScriptWinClipBufferSizeInBytes;

	WORD**					ppwScriptWinClipStrPtrsBuffer;
	ULONG					ulScriptWinClipStrPtrsBufferSizeInBytes;

	// Pointers in Clipboard Storage Mem.

	ULONG					ulScriptWinClipBufferPosInBytes;
	ULONG					ulScriptWinClipStrPtrsBufferPosInBytes;

	// Compilation Memory.

	CHAR*					pszScriptWinCompilationBuffer;	// This will hold the Parsable version of the Script File
															//  when Compiling and, normally, at any given time, the
															//  list of Macro Prototypes. To be initialized to "".

	ULONG					ulScriptWinCompilationBufferSizeInBytes;	// This should be "ulScriptWinBufferSizeInBytes" / sizeof( WORD ).
																		//  This however will NOT ensure that the Script Text will fit in the Allocated memory
																		//  because of differences in the type of the NULL Terminating Character, for example.

	// Script Object File Memory.

	BYTE*					pbScriptWinObjectFileBuffer;
	ULONG					ulScriptWinObjectFileBufferSizeInBytes;

	ULONG					ulScriptWinObjectFileBufferUsedBytes; // If this is == 0, THEN no Successful Compilation occurred.

	PFN_INITIALIZATION		pfnScriptWinObjectFileInitFnPtr;

	// Dirty Bit.

	BOOLEAN					bScriptWinDirtyBit;

	// Selection Area.

	BOOLEAN					bScriptWinShowSelArea;

	ULONG					vulScriptWinSelAreaX[ 2 ];
	ULONG					vulScriptWinSelAreaY[ 2 ];

	// Cursor Position.

	//

	// - - - - > (These Variables are Validated in the "ValidateScriptWinLnColInfos" function.)
	ULONG					ulScriptWinCol /* = X */, ulScriptWinLn /* = Y */;	// # Please note that These start from 1 and NOT from 0. #
	ULONG					ulScriptWinOffsetX, ulScriptWinOffsetY;		// # Offset in the Script Window. Base = 0. #
	// < - - - -

	//

	ULONG					ulScriptWinPrevCurX, ulScriptWinPrevCurY;	// # Cursor Position backup. #

	//

	// Working Memory.

	ULONG					ulScriptWinLineIndex;	// # If 0xFFFFFFFF the Buffer is not Consistent.
													//  If 0xEEEEEEEE the Buffer has no Storage associated to It. In this
													//  case (and only in this case) the ulScriptWinNoStorageLine is set. #

	ULONG					ulScriptWinNoStorageLine;	// # Valid if ulScriptWinLineIndex is == 0xEEEEEEEE. #

	WORD					vwScriptWinLine[ MACRO_SCRIPTWIN_LINESIZE_IN_CHARS ];	// ## WARNING: this buffer is ->
																					//  -> NOT Null Terminated! ##

	WORD					vwScriptWinBackupBuffer[ MACRO_SCRIPTWIN_LINESIZE_IN_CHARS ];	// ## The lenght of this Buffer ->
																							//  -> has to be the same of "vwScriptWinLine".

	// === Informations about the U2K Debugger Verbs. ===

	BOOLEAN					bOpenUserModeDashboard;

	LARGE_INTEGER			liLastDebuggerVerbReadTime;

	DWORD					dwLastEnterTscHigh, dwLastEnterTscLow;

	// === Informations about the Output Window. ===

	// Reserved Memory.

	CHAR*					pszOutputWinBuffer;
	ULONG					ulOutputWinBufferSizeInBytes;

	CHAR**					ppszOutputWinStrPtrsBuffer;
	ULONG					ulOutputWinStrPtrsBufferSizeInBytes;

	// Position in Reserved Memory.

	ULONG					ulOutputWinBufferPosInBytes;
	ULONG					ulOutputWinStrPtrsBufferPosInBytes;

	// Synchronization.

	MP_SPINLOCK				mpslOutputPrintFunctionMutex;

	// User Interface.

	ULONG					ulLineOffsetInOutputWin;
	ULONG					ulMruCommandOffsetFromEnd;

	ULONG					ulSysStatusBarModuleInfoStartX;

	// Report Mode.

	BOOLEAN					bReportMode;
	BOOLEAN					bReportModeAborted;
	BOOLEAN					bReportModeMsgPrinted;
	BOOLEAN					bReportMode1StScrollDone;

	ULONG					ulReportModeLinesTotal;
	ULONG					ulReportModeLinesLeft;

	ULONG					ulReportModeOldCursorX, ulReportModeOldCursorY;

	// History Line ID.

	ULONG					ulHistoryLineID;

	// === Informations about the Settings File. ===

	BYTE*					pbBugChkDat;
	ULONG					ulBugChkDatSize;

	// === Informations about the Symbols Memory. ===

	BYTE*					pbSymMemory;
	ULONG					ulSymMemorySize;

	SYMTABLEINFO			stiSymTables[ MACRO_SYMTABLES_MAXNUM ];
	ULONG					ulSymTablesNum;

	DWORD					dwSymLoaderCounter; // COUNTER FOR TOKEN and SYM TABLE INFO ORDINAL GENERATION. INITIALIZED TO 1.
	DWORD					dwSymLoaderToken; // DEFAULT: 0 = NO PENDING LOAD.
	CHAR					szSymLoaderPendingTblPath[ MAX_PATH ];

	// === Informations about the System Interrupts. ===

	// General.

	LARGE_INTEGER			liCpuCyclesPerSecond;
	LARGE_INTEGER			liCursorCpuCycles;

	SYSINTERRUPTS_STATUS	sisSysInterrStatus;

	// Debugger System Stack Memory.

	BYTE*					pbStackBase;
	BYTE*					pbStackPointerStart;

	DWORD					dwPrevStackPointer;

	// === Informations about the Memory Files. ===

	VOID*					pvMemFileMemory;
	ULONG					ulMemFileMemoryLength;
	ULONG					ulMemFileMemoryStep;

	// === Informations about the Initialization. ===

	BOOLEAN					bInitializationDone;

	// === Various Commands State. ===

	ULONG					ulI3HereState;

	BOOLEAN					bSnapShotMode;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// --> Command Option <--

typedef struct _COMMAND_OPTION
{
	// General.

	CHAR*			pszOption;
	CHAR*			pszParam;

	// Extended.

	ULONG			ulPos;

} COMMAND_OPTION, *PCOMMAND_OPTION;

// --> Externs <--

extern PDEVICE_OBJECT		g_pDeviceObject;
extern BOOLEAN				g_bCanOutputPrint;
extern PVOID				g_pvMmUserProbeAddress;
extern DWORD				g_dwNumberProcessors;

//======================
// Function Prototypes.
//======================

VOID DebuggerInvokedCore( VOID );
NTSTATUS ConsoleStartup( IN BOOLEAN bIsUserReturning );
VOID ConsoleRestore( VOID );
VOID DrawConsole( IN BOOLEAN bIsFirstDraw );
VOID MakeCursorBlink( VOID );
VOID GetConsolePrintCoords( OUT ULONG* pulX0, OUT ULONG* pulY0, OUT ULONG* pulX1, OUT ULONG* pulY1 );
VOID ProcessKeyStroke( IN BYTE bAsciiCode, IN BYTE bScanCode );
VOID __cdecl OutputPrint( IN BOOLEAN bContainsColorEscapes, IN BOOLEAN bAvoidDbgPrintRelay, IN CONST CHAR* pszFormat, ... );
VOID OutputVersionMessage( VOID );
VOID DrawOutputWindowContents( IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH );
VOID ScrollOutputWindow( IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH );
CHAR* GetNextMruCommand( IN BOOLEAN bDirection, IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH );
VOID DrawRegistersWindowContents( IN VPCICE_WINDOW* pviwRegistersWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH, IN BOOLEAN bEraseMemHintSpace );
VOID DrawCodeWindowContents( IN VPCICE_WINDOW* pviwCodeWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH );
VOID DrawStatusBar( IN ULONG ulConsoleW, IN ULONG ulConsoleH );
BOOLEAN DoAutoCompletion( IN ULONG ulConsoleW, IN ULONG ulConsoleH );
VOID VersionCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID GenericCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID DispatchCommandToHandler( VOID );
VOID CpuCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
DWORD EvaluateExpression_DWORD( IN CHAR* pszString, OUT BOOLEAN* pbRes );
VOID* AddressPrevInstruction( IN VOID* pvRefPointer, IN ULONG ulInstrOrdinal );
VOID* AddressNextInstruction( IN VOID* pvRefPointer, IN ULONG ulInstrOrdinal );
VOID PrintCodeWindowPosInModules( IN VPCICE_WINDOW* pviwOutputWindow, IN ULONG ulConsoleW, IN ULONG ulConsoleH, IN OUT ULONG* pulSysStatusBarCurrX );
VOID EcCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
BOOLEAN CompileMiniCSource( OUT BYTE** ppbOutputMem, OUT ULONG* pulOutputMemSize, IN BYTE* pbInputMem, IN ULONG ulInputMemSize, OUT BYTE** ppbPsiMem, OUT ULONG* pulPsiMemSize );
BOOLEAN InitializeMiniCCompiler( VOID );
BOOLEAN LinkMiniCSource( OUT BYTE* pbOutputMem, IN ULONG ulOutputMemSize, OUT ULONG* pulOutputMemUsedDim, OUT BYTE** ppbInitFuncPtr, IN CHAR* pszFunctionName, OUT BYTE** ppbFunctionPtr, IN BYTE* pbInputMem, IN ULONG ulInputMemSize, IN BOOLEAN bPrintErrorLines, IN BYTE* pbObjectFileToLink );
VOID InvokeDebuggerSettingEIP( VOID /* Input: EAX = address of EIP Dword. */ );
VOID I3HereCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID GetHighlightedInstructionMemoryHint( OUT CHAR* pszHint );
VOID GetHighlightedInstructionJumpHint( OUT CHAR* pszHint, IN CHAR* pszDisasmStr );
VOID BpxCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
BOOLEAN ParseCommandOptions( IN CHAR* pszParams, IN COMMAND_OPTION* pcoOptions );
CHAR* stristr( IN CONST CHAR* string, IN CONST CHAR* strCharSet );
BOOLEAN EvaluateMultipleExpressions_DWORD_2( IN CHAR* pszString0, IN CHAR* pszString1, OUT DWORD* pdwDword0, OUT DWORD* pdwDword1 );
VOID DtxCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID HbootCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID HandleMouseEvent( VOID );
VOID EnteringReportMode( VOID );
VOID ExitingReportMode( VOID );
VOID WaitForUserInput( OUT BYTE* pbLastAsciiCodeAcquired, OUT BYTE* pbLastScanCodeAcquired );
VOID BcCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID DtcCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
ULONG ParseHexNumSeriesString( OUT DWORD* pdwOutputSeries, IN ULONG ulOutputSeriesMaxItemsNum, IN CHAR* pszInputString );
VOID DrawScriptWinLnColInfos( VOID );
VOID ValidateScriptWinLnColInfos( VOID );
VOID DrawScriptWindowContents( IN BOOLEAN bEndLine, IN BOOLEAN bSetCursorPos );
VOID BeginScriptWindowLineEdit( VOID );
VOID EndScriptWindowLineEdit( VOID );
VOID AddScriptLine( IN ULONG ulLineId, IN WORD* pwLineText );
VOID DeleteScriptLine( IN ULONG ulLineId );
VOID ApplySyntaxColoring( IN OUT WORD* pwStr );
BOOLEAN ApplySelAreaToText( IN BOOLEAN bOn, IN ULONG* pulSelAreaX, IN ULONG* pulSelAreaY, OUT BOOLEAN* pbBlockYInc, OUT ULONG* pulIfYIncCorrectX );
BOOLEAN CopyScriptTextToClip( IN ULONG* pulValidatedSelAreaX, IN ULONG* pulValidatedSelAreaY );
VOID PasteScriptText( IN OUT ULONG* pulPasteX, IN OUT ULONG* pulPasteY );
VOID DeleteScriptText( IN ULONG* pulValidatedSelAreaX, IN ULONG* pulValidatedSelAreaY );
BOOLEAN SearchForFunctionName( IN ULONG ulStartY, IN ULONG ulOutputBufferSize, OUT CHAR* pszOutputBuffer );
BOOLEAN FillCompilationBuffer( VOID );
VOID InitializeScriptFileText( VOID );
BOOLEAN CollectMacroPrototypes( VOID );
BOOLEAN CheckIfMacrosAreSyntaxedCorrectly( IN CHAR* pszInputText );
VOID EvalExprCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID EvalExprFloatCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
double EvaluateExpression_DOUBLE( IN CHAR* pszString, OUT BOOLEAN* pbRes );
VOID BpIntCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BpIoCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BpMbCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BpMdCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BpMwCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BdCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BeCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BlCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
VOID BpeCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
PVOID LoadFile( IN PCWSTR pszFileName, IN POOL_TYPE ptPoolType, OUT ULONG* pulSize );
CHAR* ParseStructuredFile( IN BYTE* pbFile, IN ULONG ulSize, IN ULONG ulTabsNum, IN CHAR* pszString, IN CHAR* pszStart );
CHAR* GetSfSetting( IN BYTE* pbFile, IN ULONG ulSize, IN CHAR* l0, IN CHAR* l1, IN CHAR* l1def );
DWORD AddSymTableToken( IN CHAR* pszPath );
BOOLEAN CompleteSymTableLoad( IN DWORD dwToken );
BOOLEAN AddSymTable( IN BYTE* pbTable, IN ULONG ulSize, IN CHAR* pszName );
BOOLEAN RemoveSymTable( IN ULONG ulOrdinal );
BOOLEAN GetHistoryLineText( IN OUT ULONG* pulIndexS, IN OUT ULONG* pulIndexE, OUT CHAR* pszBuffer, IN ULONG ulBuffSize );
BOOLEAN GetEnvScriptText( IN BOOLEAN bFromClipboard, OUT CHAR* pszBuff, IN ULONG ulBuffSize );
BOOLEAN SetEnvScriptText( IN BOOLEAN bFromClipboard, IN CHAR* pszBuff );
VOID ExecuteAutoTypedCommand( VOID );
VOID DashCommandHandler( IN CHAR* pszLine, IN CHAR* pszCommand, IN CHAR* pszParams );
