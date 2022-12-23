/***************************************************************************************
  *
  * Video.h - VPCICE Video Routines Header File.
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
#include "VIDEOMEMORYINFO.h"

//===================
// External Globals.
//===================

extern BOOLEAN		g_bDrawScreenIsEnabled;

//======================
// General Definitions.
//======================

#define MACRO_DEFAULT_TEXTMODE_VIDEO_WIDTH		80
#define MACRO_DEFAULT_TEXTMODE_VIDEO_HEIGHT		25

//=====================
// Cursor Definitions.
//=====================

#define MACRO_CURSORPOS_UNDEFINED_VALUE		0xFFFF

//============================
// Mouse Pointer Definitions.
//============================

#define MACRO_MOUSEMOVEMENTFACTOR_DEFAULT	( 0.2f )
#define MACRO_MOUSEMOVEMENTFACTOR_ADJUSTX	( 0.28f )
#define MACRO_MOUSEMOVEMENTFACTOR_ADJUSTY	( 0.28f )

//============================
// Mouse Pointer Definitions.
//============================

#define MACRO_MOUSEPOINTERPOS_UNDEFINED_VALUE	0xFFFF

//=====================
// Window Definitions.
//=====================

#define MACRO_MAXNUM_OF_VPCICE_WINDOWS		0x20

#define MACRO_RESIZINGSPECIFICWINDOWNAME_UNDEF		0xFFFFFFFF

//=====================================
// Specific VpcICE Window Definitions.
//=====================================

#define MACRO_VPCICE_WINDOW_NAME_REGISTERS			0
#define MACRO_VPCICE_WINDOW_ORDER_REGISTERS			0
#define MACRO_VPCICE_WINDOW_HEIGHT_REGISTERS		3
#define MACRO_VPCICE_WINDOW_MINHEIGHT_REGISTERS		3

#define MACRO_VPCICE_WINDOW_NAME_CODE				1
#define MACRO_VPCICE_WINDOW_ORDER_CODE				0x1000
#define MACRO_VPCICE_WINDOW_HEIGHT_CODE				10
#define MACRO_VPCICE_WINDOW_MINHEIGHT_CODE			1

#define MACRO_VPCICE_WINDOW_NAME_SCRIPT				2
#define MACRO_VPCICE_WINDOW_ORDER_SCRIPT			0x2000
#define MACRO_VPCICE_WINDOW_HEIGHT_SCRIPT			4
#define MACRO_VPCICE_WINDOW_MINHEIGHT_SCRIPT		1

#define MACRO_VPCICE_WINDOW_NAME_OUTPUT				3
#define MACRO_VPCICE_WINDOW_ORDER_OUTPUT			0xFFFFFFFF
#define MACRO_VPCICE_WINDOW_HEIGHT_OUTPUT			0
#define MACRO_VPCICE_WINDOW_MINHEIGHT_OUTPUT		2

//=============
// Structures.
//=============

// --> VPCICE_WINDOW Structure. <--

typedef struct _VPCICE_WINDOW
{
	// == Attributes. ==

	ULONG			ulName;
	ULONG			ulOrder;
	ULONG			ulHeight;
	ULONG			ulMinHeight;
	BOOLEAN			bHasTopLine;
	BOOLEAN			bDisplayed;

	// == Working Data. ==

	ULONG			ulX0, ulY0, ulX1, ulY1;

} VPCICE_WINDOW, *PVPCICE_WINDOW;

// --> VPCICE_VIDEOMEMORY Structure. <--

	// Note: the Video Memory is organized in this way:
	//		First, there is the memory reserved to the Text Front Buffer.
	//		Then, the memory for the Text Back Buffer.
	//		At the end, the memory for the Application Screen used when restoring the Application View.

typedef struct _VPCICE_VIDEOMEMORY
{
	// === General Informations. ===

	PVOID			pvVideoMemory;
	ULONG			ulVideoMemorySizeInBytes;

	// === Font Table Informations. ===

	DWORD*			pdwFontTable;
	ULONG			ulFontTableSizeInBytes;
	ULONG			ulFontTableWidthInBits, ulFontTableStrideInBits;
	ULONG			ulGlyphWidthInBits, ulGlyphHeightInBits;

	// === Text Front Buffer / Text Back Buffer Informations. ===

	WORD*			pwTextFrontBuffer;
	WORD*			pwTextBackBuffer;
	ULONG			ulTextBuffersSizeInBytes;
	ULONG			ulTextBuffersWidthInChars, ulTextBuffersHeightInChars;

	// === Application Screen Restore Memory Informations. ===

	BYTE*			pbApplicationScreen;
	ULONG			ulApplicationScreenSizeInBytes;

	// === Informations about the Target Display Output. ===

	ULONG			ulConsoleWidthInPx, ulConsoleHeightInPx;
	ULONG			ulTargetStartX, ulTargetStartY;

	// === Informations and State about the Console Cursor. ===

	// Low Level Restore Informations.

	BOOLEAN			bCRTCAddress_InfoFull;
	BYTE			bCRTCAddress_Original;

	BOOLEAN			bIndex0Ah_InfoFull;
	BYTE			bIndex0Ah_Original;

	BOOLEAN			bIndex0Bh_InfoFull;
	BYTE			bIndex0Bh_Original;

	BOOLEAN			bIndex0Eh_InfoFull;
	BYTE			bIndex0Eh_Original;

	BOOLEAN			bIndex0Fh_InfoFull;
	BYTE			bIndex0Fh_Original;

	// Low Level Text Mode.

	WORD			wCRTC_AddressRegPort;
	WORD			wCRTC_DataRegPort;

	// Console Cursor Informations.

	BOOLEAN			bCursorDisplayed;
	ULONG			ulCursorX, ulCursorY;

	BOOLEAN			bCursorBlinkStatus;

	// Mouse Pointer Informations.

	BOOLEAN			bMousePointerDisplayed;
	ULONG			ulMousePointerX, ulMousePointerY;
	ULONG			ulMousePointerOldX, ulMousePointerOldY;

	float			fMousePointerX, fMousePointerY;
	float			fMovementXFactor, fMovementYFactor;

	// === Window Status. ===

	VPCICE_WINDOW	vvwWindows[ MACRO_MAXNUM_OF_VPCICE_WINDOWS ];
	ULONG			ulWindowsNum;

} VPCICE_VIDEOMEMORY, *PVPCICE_VIDEOMEMORY;

// --> Externals. <--

extern const DWORD			g_vdw9x16FontTable[];
extern const ULONG			g_ul9x16FontTableSizeInBytes;

// --> DRAWGLYPH_LAYOUTS Structure. <--

typedef struct _DRAWGLYPH_LAYOUTS
{
	// Informations about the Surfaces.

	PVOID					pvTextVideoBuffer;
	VIDEOMEMORYINFO			vmiVideoMemInfo;
	VPCICE_VIDEOMEMORY		vivmVpcICEVideo;

} DRAWGLYPH_LAYOUTS, *PDRAWGLYPH_LAYOUTS;

//======================
// Function Prototypes.
//======================

NTSTATUS InitializeVpcICEVideoMemoryStructure( OUT VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo, IN VIDEOMEMORYINFO* pvmiVideoMemoryInfo, IN PVOID pvVideoMemory, IN ULONG ulVideoMemorySizeInBytes, IN ULONG ulTextBuffersWidthInChars, IN ULONG ulTextBuffersHeightInChars );
NTSTATUS SubscribeVideoMemoryInfoBuffer( OUT VIDEOMEMORYINFO** ppvmiRetPointer );
NTSTATUS DrawGlyph( IN ULONG ulX, IN ULONG ulY, IN BYTE bTextColor, IN BYTE bTextChar, DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS SaveApplicationScreen( IN DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS RestoreApplicationScreen( IN DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS DrawScreen( IN DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS DrawCursor( IN ULONG ulX, IN ULONG ulY, IN DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS EnableCursor( IN BOOLEAN bEnable, IN DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS RestoreVGALowLevelState( IN DRAWGLYPH_LAYOUTS* pdglLayouts );
NTSTATUS AddNewVpcICEWindow( IN ULONG ulName, IN ULONG ulOrder, IN ULONG ulHeight, IN ULONG ulMinHeight, IN BOOLEAN bHasTopLine, IN BOOLEAN bDisplayed, IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo );
NTSTATUS RearrangeVpcICEWindows( IN ULONG ulResizingSpecificWindowName, IN ULONG ulResizingSpecificWindowPrevH, IN ULONG ulX0, IN ULONG ulY0, IN ULONG ulX1, IN ULONG ulY1, IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo );
VPCICE_WINDOW* GetVpcICEWindowByName( ULONG ulName, IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo );
