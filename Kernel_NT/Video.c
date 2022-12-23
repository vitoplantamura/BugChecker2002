/***************************************************************************************
  *
  * Video.c - VPCICE Video Routines Source File.
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

#include <ntddk.h>
#include "..\Include\vpcice.h"
#include "..\Include\Video.h"
#include "..\Include\IOCTLs_Video.h"

//==========
// Pragmas.
//==========

#pragma warning( disable : 4090 )

//==========
// Globals.
//==========

BOOLEAN		g_bDrawScreenIsEnabled = TRUE;

//==================================
// g_vdgciDrawGlyphCache Structure.
//==================================

typedef struct _DRAWGLYPH_CACHEITEM
{
	ULONG		ulCacheItemStatus;
	DWORD*		pdwDwordStart;
	ULONG		ulDwordBitStart;

	BYTE		bDrawCacheCharColor;
	BYTE		vbDrawCache[ 1024 ];

} DRAWGLYPH_CACHEITEM, *PDRAWGLYPH_CACHEITEM;

static DRAWGLYPH_CACHEITEM		g_vdgciDrawGlyphCache[ 256 ];

//===========================================================
// InitializeVpcICEVideoMemoryStructure Function Definition.
//===========================================================

static BOOLEAN				g_bVpcICEVideoMemoryStructureInitialized = FALSE;
static VPCICE_VIDEOMEMORY	g_vivmVpcICEVideoMemoryStructureBackup;

NTSTATUS InitializeVpcICEVideoMemoryStructure( OUT VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo, IN VIDEOMEMORYINFO* pvmiVideoMemoryInfo, IN PVOID pvVideoMemory, IN ULONG ulVideoMemorySizeInBytes, IN ULONG ulTextBuffersWidthInChars, IN ULONG ulTextBuffersHeightInChars )
{
	ULONG			ulI;

	// === Make a Backup Copy of the Structure. ===

	g_vivmVpcICEVideoMemoryStructureBackup = * pvivmVideoMemoryInfo;

	// === Reset the Structure Memory Contents. ===

	memset( pvivmVideoMemoryInfo, 0, sizeof( VPCICE_VIDEOMEMORY ) );

	// === Initialize the Structure. ===

	// Initialize the Standard Fields.

	pvivmVideoMemoryInfo->pvVideoMemory = pvVideoMemory;
	pvivmVideoMemoryInfo->ulVideoMemorySizeInBytes = ulVideoMemorySizeInBytes;

	pvivmVideoMemoryInfo->pdwFontTable = g_vdw9x16FontTable;
	pvivmVideoMemoryInfo->ulFontTableSizeInBytes = g_ul9x16FontTableSizeInBytes;
	pvivmVideoMemoryInfo->ulFontTableWidthInBits =
		pvivmVideoMemoryInfo->ulFontTableStrideInBits =
			9 * 32;
	pvivmVideoMemoryInfo->ulGlyphWidthInBits = 9;
	pvivmVideoMemoryInfo->ulGlyphHeightInBits = 16;

	pvivmVideoMemoryInfo->pwTextFrontBuffer = (WORD*) pvVideoMemory;
	pvivmVideoMemoryInfo->pwTextBackBuffer = (WORD*) pvVideoMemory +
		ulTextBuffersWidthInChars * ulTextBuffersHeightInChars;
	pvivmVideoMemoryInfo->ulTextBuffersSizeInBytes = ulTextBuffersWidthInChars * ulTextBuffersHeightInChars * sizeof( WORD );
	pvivmVideoMemoryInfo->ulTextBuffersWidthInChars = ulTextBuffersWidthInChars;
	pvivmVideoMemoryInfo->ulTextBuffersHeightInChars = ulTextBuffersHeightInChars;

	pvivmVideoMemoryInfo->pbApplicationScreen = (BYTE*) ( (WORD*) pvVideoMemory +
		ulTextBuffersWidthInChars * ulTextBuffersHeightInChars * 2 );
	pvivmVideoMemoryInfo->ulApplicationScreenSizeInBytes =
		(ULONG) pvVideoMemory + ulVideoMemorySizeInBytes - (ULONG) pvivmVideoMemoryInfo->pbApplicationScreen;

	pvivmVideoMemoryInfo->ulConsoleWidthInPx = pvivmVideoMemoryInfo->ulGlyphWidthInBits * pvivmVideoMemoryInfo->ulTextBuffersWidthInChars;
	pvivmVideoMemoryInfo->ulConsoleHeightInPx = pvivmVideoMemoryInfo->ulGlyphHeightInBits * pvivmVideoMemoryInfo->ulTextBuffersHeightInChars;

	if ( pvmiVideoMemoryInfo->pvPrimary ) // ### DirectDraw Mode ###
	{
		pvivmVideoMemoryInfo->ulTargetStartX = ( pvmiVideoMemoryInfo->dwDisplayWidth - pvivmVideoMemoryInfo->ulConsoleWidthInPx ) / 2;
		pvivmVideoMemoryInfo->ulTargetStartY = ( pvmiVideoMemoryInfo->dwDisplayHeight - pvivmVideoMemoryInfo->ulConsoleHeightInPx ) / 2;
	}
	else // ### 80x25 Text Mode ###
	{
		pvivmVideoMemoryInfo->ulTargetStartX = ( MACRO_DEFAULT_TEXTMODE_VIDEO_WIDTH - pvivmVideoMemoryInfo->ulTextBuffersWidthInChars ) / 2;
		pvivmVideoMemoryInfo->ulTargetStartY = ( MACRO_DEFAULT_TEXTMODE_VIDEO_HEIGHT - pvivmVideoMemoryInfo->ulTextBuffersHeightInChars ) / 2;
	}

	pvivmVideoMemoryInfo->ulCursorX = MACRO_CURSORPOS_UNDEFINED_VALUE;
	pvivmVideoMemoryInfo->ulCursorY = MACRO_CURSORPOS_UNDEFINED_VALUE;

	// Take care of the Mouse Pointer informations.
	// By default, the Mouse Pointer is displayed and its initial position is at the Console Center.

	pvivmVideoMemoryInfo->bMousePointerDisplayed = TRUE;
	pvivmVideoMemoryInfo->ulMousePointerX = ulTextBuffersWidthInChars / 2;
	pvivmVideoMemoryInfo->ulMousePointerY = ulTextBuffersHeightInChars / 2;
	pvivmVideoMemoryInfo->ulMousePointerOldX = MACRO_MOUSEPOINTERPOS_UNDEFINED_VALUE;
	pvivmVideoMemoryInfo->ulMousePointerOldY = MACRO_MOUSEPOINTERPOS_UNDEFINED_VALUE;

	pvivmVideoMemoryInfo->fMousePointerX = (float) pvivmVideoMemoryInfo->ulMousePointerX;
	pvivmVideoMemoryInfo->fMousePointerY = (float) pvivmVideoMemoryInfo->ulMousePointerY;

	if ( pvmiVideoMemoryInfo->pvPrimary ) // ### DirectDraw Mode ###
	{
		pvivmVideoMemoryInfo->fMovementXFactor =
			( (float) pvivmVideoMemoryInfo->ulConsoleWidthInPx / (float) pvmiVideoMemoryInfo->dwDisplayWidth ) *
			MACRO_MOUSEMOVEMENTFACTOR_ADJUSTX;
		pvivmVideoMemoryInfo->fMovementYFactor =
			( (float) pvivmVideoMemoryInfo->ulConsoleHeightInPx / (float) pvmiVideoMemoryInfo->dwDisplayHeight ) *
			MACRO_MOUSEMOVEMENTFACTOR_ADJUSTY;
	}
	else // ### 80x25 Text Mode ###
	{
		pvivmVideoMemoryInfo->fMovementXFactor =
			pvivmVideoMemoryInfo->fMovementYFactor =
				MACRO_MOUSEMOVEMENTFACTOR_DEFAULT;
	}

	// Take care of the Window Status Initialization.

	if ( g_bVpcICEVideoMemoryStructureInitialized == FALSE )
	{
		// Specify the various Windows.

		AddNewVpcICEWindow( MACRO_VPCICE_WINDOW_NAME_REGISTERS, MACRO_VPCICE_WINDOW_ORDER_REGISTERS,
			MACRO_VPCICE_WINDOW_HEIGHT_REGISTERS, MACRO_VPCICE_WINDOW_MINHEIGHT_REGISTERS,
			/* bHasTopLine = */ FALSE, /* bDisplayed = */ TRUE,
			pvivmVideoMemoryInfo );

		AddNewVpcICEWindow( MACRO_VPCICE_WINDOW_NAME_CODE, MACRO_VPCICE_WINDOW_ORDER_CODE,
			MACRO_VPCICE_WINDOW_HEIGHT_CODE, MACRO_VPCICE_WINDOW_MINHEIGHT_CODE,
			/* bHasTopLine = */ TRUE, /* bDisplayed = */ TRUE,
			pvivmVideoMemoryInfo );

		AddNewVpcICEWindow( MACRO_VPCICE_WINDOW_NAME_SCRIPT, MACRO_VPCICE_WINDOW_ORDER_SCRIPT,
			MACRO_VPCICE_WINDOW_HEIGHT_SCRIPT, MACRO_VPCICE_WINDOW_MINHEIGHT_SCRIPT,
			/* bHasTopLine = */ TRUE, /* bDisplayed = */ TRUE,
			pvivmVideoMemoryInfo );

		AddNewVpcICEWindow( MACRO_VPCICE_WINDOW_NAME_OUTPUT, MACRO_VPCICE_WINDOW_ORDER_OUTPUT,
			MACRO_VPCICE_WINDOW_HEIGHT_OUTPUT, MACRO_VPCICE_WINDOW_MINHEIGHT_OUTPUT,
			/* bHasTopLine = */ TRUE, /* bDisplayed = */ TRUE,
			pvivmVideoMemoryInfo );
	}
	else
	{
		// Restore the Previous State.

		pvivmVideoMemoryInfo->ulWindowsNum = g_vivmVpcICEVideoMemoryStructureBackup.ulWindowsNum;

		for ( ulI = 0; ulI < g_vivmVpcICEVideoMemoryStructureBackup.ulWindowsNum; ulI ++ )
			pvivmVideoMemoryInfo->vvwWindows[ ulI ] = g_vivmVpcICEVideoMemoryStructureBackup.vvwWindows[ ulI ];
	}

	// Remember that we have initialized the Structure.

	g_bVpcICEVideoMemoryStructureInitialized = TRUE;

	// === Check whether the Video Mode is large enough to contain the VpcICE Console. ===

	if ( pvmiVideoMemoryInfo->pvPrimary ) // ### DirectDraw Mode ###
	{
		if ( pvivmVideoMemoryInfo->ulConsoleWidthInPx >
			pvmiVideoMemoryInfo->dwDisplayWidth )
				return STATUS_UNSUCCESSFUL;
		if ( pvivmVideoMemoryInfo->ulConsoleHeightInPx >
			pvmiVideoMemoryInfo->dwDisplayHeight )
				return STATUS_UNSUCCESSFUL;
	}
	else // ### 80x25 Text Mode ###
	{
		if ( pvivmVideoMemoryInfo->ulTextBuffersWidthInChars > MACRO_DEFAULT_TEXTMODE_VIDEO_WIDTH )
			return STATUS_UNSUCCESSFUL;
		if ( pvivmVideoMemoryInfo->ulTextBuffersHeightInChars > MACRO_DEFAULT_TEXTMODE_VIDEO_HEIGHT )
			return STATUS_UNSUCCESSFUL;
	}

	// === Check whether the RGB Bit Count value is compatible with our Requirements. ===

	if ( pvmiVideoMemoryInfo->ddpfDisplay.dwRGBBitCount % 8 )
		return STATUS_UNSUCCESSFUL;

	// === Check whether the Application Screen Restore Memory is large enough. ===

	if ( pvmiVideoMemoryInfo->pvPrimary ) // ### DirectDraw Mode ###
	{
		if ( pvivmVideoMemoryInfo->ulApplicationScreenSizeInBytes <
			pvivmVideoMemoryInfo->ulTextBuffersWidthInChars * pvivmVideoMemoryInfo->ulTextBuffersHeightInChars *
			pvivmVideoMemoryInfo->ulGlyphWidthInBits * pvivmVideoMemoryInfo->ulGlyphHeightInBits *
			( pvmiVideoMemoryInfo->ddpfDisplay.dwRGBBitCount / 8 ) )
				return STATUS_UNSUCCESSFUL;
	}
	else // ### 80x25 Text Mode ###
	{
		if ( pvivmVideoMemoryInfo->ulApplicationScreenSizeInBytes <
			pvivmVideoMemoryInfo->ulTextBuffersWidthInChars * pvivmVideoMemoryInfo->ulTextBuffersHeightInChars * sizeof( WORD ) )
				return STATUS_UNSUCCESSFUL;
	}

	// === Initialize the "Draw Glyph Cache". ===

	memset( g_vdgciDrawGlyphCache, 0, sizeof( g_vdgciDrawGlyphCache ) );

	// === Return to the Caller. ===

	return STATUS_SUCCESS;
}

//=====================================================
// SubscribeVideoMemoryInfoBuffer Function Definition.
//=====================================================

NTSTATUS SubscribeVideoMemoryInfoBuffer( OUT VIDEOMEMORYINFO** ppvmiRetPointer )
{
	NTSTATUS		nsRetVal = STATUS_UNSUCCESSFUL;
	UNICODE_STRING	usVpcICEvdDeviceName;
	NTSTATUS		nsGetDevPtrRes;
	PFILE_OBJECT	pfoVpcICEvdFileObject;
	PDEVICE_OBJECT	pdoVpcICEvdDeviceObject;
	PIRP			piIRP;
	KEVENT			keIrpComplEvent;
	IO_STATUS_BLOCK	iosbIrpIOStatusBlock;
	NTSTATUS		nsCallDrvRes;

	* ppvmiRetPointer = NULL;

	// Allocate the NonPageable memory for the Structure.

	* ppvmiRetPointer = (VIDEOMEMORYINFO*) ExAllocatePool( NonPagedPool, sizeof( VIDEOMEMORYINFO ) );

	if ( * ppvmiRetPointer == NULL )
		return STATUS_UNSUCCESSFUL;

	// Send the corresponding IOCTL to the VpcICEvd Driver.

	RtlInitUnicodeString( & usVpcICEvdDeviceName, L"\\Device\\" SYSDRIVER_NAME_WIDE );

	nsGetDevPtrRes = IoGetDeviceObjectPointer( & usVpcICEvdDeviceName,
		FILE_READ_ATTRIBUTES,
		& pfoVpcICEvdFileObject, & pdoVpcICEvdDeviceObject );

	if ( NT_SUCCESS( nsGetDevPtrRes ) )
	{
		KeInitializeEvent( & keIrpComplEvent, NotificationEvent, FALSE );

		piIRP = IoBuildDeviceIoControlRequest( IOCTL_VPCICEVID_SUBSCRIBE_VIDEOMEMORYINFO_BUFFER,
			pdoVpcICEvdDeviceObject,
			(PVOID) ppvmiRetPointer, sizeof( VIDEOMEMORYINFO* ),
			NULL, 0,
			FALSE,
			& keIrpComplEvent, & iosbIrpIOStatusBlock );

		if ( piIRP != NULL )
		{
			nsCallDrvRes = IoCallDriver( pdoVpcICEvdDeviceObject, piIRP );

			if ( nsCallDrvRes == STATUS_PENDING )
			{
				KeWaitForSingleObject( & keIrpComplEvent, Executive, KernelMode, FALSE, NULL );
				nsCallDrvRes = iosbIrpIOStatusBlock.Status;
			}

			if ( NT_SUCCESS( nsCallDrvRes ) )
				nsRetVal = STATUS_SUCCESS;
		}

		ObDereferenceObject( pfoVpcICEvdFileObject );
	}

	// Return to the Caller.

	return nsRetVal;
}

//==========================
// Color Tables Definition.
//==========================

static BYTE		g_vbColorTablePaletted[] =
{
	0,
	4,
	2,
	6,
	1,
	5,
	3,
	7,
	248,
	252,
	250,
	254,
	249,
	253,
	251,
	255
};

static WORD		g_vwColorTable565[] =
{
	0x0000,
	0x0010,
	0x0400,
	0x0410,
	0x8000,
	0x8010,
	0x8400,
	0xc618,
	0x8410,
	0x001f,
	0x07e0,
	0x07ff,
	0xf800,
	0xf81f,
	0xffe0,
	0xffff
};

static WORD		g_vwColorTable555[] =
{
	0x0000,
	0x0010,
	0x0200,
	0x0210,
	0x4000,
	0x4010,
	0x4200,
	0x6318,
	0x4210,
	0x001f,
	0x03e0,
	0x03ff,
	0x7c00,
	0x7c1f,
	0x7fe0,
	0x7fff
};

static DWORD	g_vdwColorTable00RRGGBB[] =
{
	0x00000000,
	0x00000080,
	0x00008000,
	0x00008080,
	0x00800000,
	0x00800080,
	0x00808000,
	0x00c0c0c0,
	0x00808080,
	0x000000ff,
	0x0000ff00,
	0x0000ffff,
	0x00ff0000,
	0x00ff00ff,
	0x00ffff00,
	0x00ffffff
};

//================================
// DrawGlyph Function Definition.
//================================

NTSTATUS DrawGlyph( IN ULONG ulX, IN ULONG ulY, IN BYTE bTextColor, IN BYTE bTextChar, DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Check if Text Mode or DirectDraw.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary == NULL ) // ### 80x25 Text Mode ###
	{
		// Check if we can continue.

		if ( pdglLayouts->pvTextVideoBuffer == NULL )
		{
			// Return to the Caller.

			return STATUS_UNSUCCESSFUL;
		}
		else
		{
			WORD*			pwVideoBufferPos;

			// Write in the VGA Text Buffer.

			pwVideoBufferPos = ( WORD* ) pdglLayouts->pvTextVideoBuffer +
				( ulY + pdglLayouts->vivmVpcICEVideo.ulTargetStartY ) * pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars +
				( ulX + pdglLayouts->vivmVpcICEVideo.ulTargetStartX );

			* pwVideoBufferPos = ( bTextColor << 8 ) | bTextChar;
		}
	}
	else // ### DirectDraw ###
	{
		ULONG		ulFontTableChrWidth, ulChrX, ulChrY, ulBitX, ulBitY, ulBitStart;
		DWORD*		pdwDwordStart;
		ULONG		ulDwordBitStart;
		BYTE*		pbDisplayStart;
		ULONG		ulVideoX, ulVideoY;
		BYTE		bForeColor, bBackColor;
		DWORD		dwDword;
		ULONG		ulI, ulW;
		DWORD*		pdwDwordPtr;
		DWORD*		pdwDwordPtrLine;
		ULONG		ulFinalFrameBufStrideInBytes, ulFinalFontTblStrideInDwords;
		ULONG		ulDwordBit;
		ULONG		ulFinalFrameBufStrideInWords;
		ULONG		ulFinalFrameBufStrideInDwords;
		BYTE*		pbDrawCache;
		ULONG		ulRenderWithDrawCache;

		// Calculate the position of the Character in the Font Table.

		if ( g_vdgciDrawGlyphCache[ bTextChar ].ulCacheItemStatus == 0 )
		{
			// Calculate the Information.

			ulFontTableChrWidth = pdglLayouts->vivmVpcICEVideo.ulFontTableWidthInBits / pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;

			ulChrY = ( (ULONG) bTextChar ) / ulFontTableChrWidth;
			ulChrX = ( (ULONG) bTextChar ) - ( ulChrY * ulFontTableChrWidth );

			ulBitX = ulChrX * pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
			ulBitY = ulChrY * pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits;

			ulBitStart = ulBitY * pdglLayouts->vivmVpcICEVideo.ulFontTableStrideInBits + ulBitX;

			pdwDwordStart = pdglLayouts->vivmVpcICEVideo.pdwFontTable + ulBitStart / 32;
			ulDwordBitStart = 31 - ( ulBitStart % 32 );

			// Cache the Information.

			g_vdgciDrawGlyphCache[ bTextChar ].ulCacheItemStatus = 1;
			g_vdgciDrawGlyphCache[ bTextChar ].pdwDwordStart = pdwDwordStart;
			g_vdgciDrawGlyphCache[ bTextChar ].ulDwordBitStart = ulDwordBitStart;
		}
		else
		{
			// Return the Information.

			pdwDwordStart = g_vdgciDrawGlyphCache[ bTextChar ].pdwDwordStart;
			ulDwordBitStart = g_vdgciDrawGlyphCache[ bTextChar ].ulDwordBitStart;
		}

		// Check if we have to use the Draw Cache.

		pbDrawCache = g_vdgciDrawGlyphCache[ bTextChar ].vbDrawCache;

		if ( bTextColor == g_vdgciDrawGlyphCache[ bTextChar ].bDrawCacheCharColor )
		{
			// Use the Cached Data.

			ulRenderWithDrawCache = 1;
		}
		else
		{
			// Fill the Cache.

			ulRenderWithDrawCache = 0;
			g_vdgciDrawGlyphCache[ bTextChar ].bDrawCacheCharColor = bTextColor;
		}

		// Calculate the start position in the Video Framebuffer.

		ulVideoX = pdglLayouts->vivmVpcICEVideo.ulTargetStartX + ulX * pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
		ulVideoY = pdglLayouts->vivmVpcICEVideo.ulTargetStartY + ulY * pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits;

		pbDisplayStart = (BYTE*) pdglLayouts->vmiVideoMemInfo.pvPrimary + (ULONG) pdglLayouts->vmiVideoMemInfo.fpPrimary +
			ulVideoY * pdglLayouts->vmiVideoMemInfo.lDisplayPitch +
			ulVideoX * ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 );

		// Calculate the Foreground and Background colors.

		bForeColor = bTextColor & 0xF;
		bBackColor = bTextColor >> 4;

		// Calculate the Stride in bytes relative to the Font Table.

		ulFinalFontTblStrideInDwords =
			pdglLayouts->vivmVpcICEVideo.ulFontTableStrideInBits / 32;

		// Draw the Glyph according to the Current Video Mode.

		switch ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 )
		{
		case 1: // ### 1 byte per pixel ###
			{
				BYTE*			pbBytePtrFB;
				BYTE			bFore, bBack;
				BYTE*			pbCachePtr = (BYTE*) pbDrawCache;

				// ### Paletted ###

				// Initialize the variables.

				ulFinalFrameBufStrideInBytes =
					pdglLayouts->vmiVideoMemInfo.lDisplayPitch - pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;

				bFore = g_vbColorTablePaletted[ bForeColor ];
				bBack = g_vbColorTablePaletted[ bBackColor ];

				pbBytePtrFB = pbDisplayStart;
				pdwDwordPtr = pdwDwordStart;

				// Draw in the Framebuffer.

				if ( ulRenderWithDrawCache )
				{
					ULONG		ulLineLength = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
					ULONG		ulVideoStride = pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
					ULONG		ulCacheStride = ulLineLength;

					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						memcpy( pbBytePtrFB, pbCachePtr, ulLineLength );
						pbBytePtrFB += ulVideoStride;
						pbCachePtr += ulCacheStride;
					}
				}
				else
				{
					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						ulW = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
						pdwDwordPtrLine = pdwDwordPtr;

						dwDword = * pdwDwordPtr;
						ulDwordBit = ulDwordBitStart;

						do
						{
							if ( ulDwordBit == 0xFFFFFFFF )
							{
								dwDword = * ++ pdwDwordPtrLine;
								ulDwordBit = 31;
							}

							if ( ( dwDword >> ulDwordBit ) & 1 )
								* pbCachePtr ++ = * pbBytePtrFB ++ = bFore;
							else
								* pbCachePtr ++ = * pbBytePtrFB ++ = bBack;

							ulDwordBit --;

						} while( -- ulW );

						pbBytePtrFB += ulFinalFrameBufStrideInBytes;
						pdwDwordPtr += ulFinalFontTblStrideInDwords;
					}
				}
			}
			break;

		case 2: // ### 2 byte per pixel ###
			{
				WORD*			pwWordPtrFB;
				WORD			wFore, wBack;
				WORD*			pwCachePtr = (WORD*) pbDrawCache;

				// Initialize the variables.

				ulFinalFrameBufStrideInWords =
					pdglLayouts->vmiVideoMemInfo.lDisplayPitch / 2 - pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;

				if ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRBitMask == 0xF800 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwGBitMask == 0x7E0 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwBBitMask == 0x1F )
				{
					// ### 565 ###

					wFore = g_vwColorTable565[ bForeColor ];
					wBack = g_vwColorTable565[ bBackColor ];
				}
				else if ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRBitMask == 0x7C00 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwGBitMask == 0x3E0 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwBBitMask == 0x1F )
				{
					// ### 555 ###

					wFore = g_vwColorTable555[ bForeColor ];
					wBack = g_vwColorTable555[ bBackColor ];
				}
				else
					return STATUS_UNSUCCESSFUL;

				pwWordPtrFB = (WORD*) pbDisplayStart;
				pdwDwordPtr = pdwDwordStart;

				// Draw in the Framebuffer.

				if ( ulRenderWithDrawCache )
				{
					ULONG		ulLineLength = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits * sizeof( WORD );
					ULONG		ulVideoStride = pdglLayouts->vmiVideoMemInfo.lDisplayPitch / sizeof( WORD );
					ULONG		ulCacheStride = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;

					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						memcpy( pwWordPtrFB, pwCachePtr, ulLineLength );
						pwWordPtrFB += ulVideoStride;
						pwCachePtr += ulCacheStride;
					}
				}
				else
				{
					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						ulW = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
						pdwDwordPtrLine = pdwDwordPtr;

						dwDword = * pdwDwordPtr;
						ulDwordBit = ulDwordBitStart;

						do
						{
							if ( ulDwordBit == 0xFFFFFFFF )
							{
								dwDword = * ++ pdwDwordPtrLine;
								ulDwordBit = 31;
							}

							if ( ( dwDword >> ulDwordBit ) & 1 )
								* pwCachePtr ++ = * pwWordPtrFB ++ = wFore;
							else
								* pwCachePtr ++ = * pwWordPtrFB ++ = wBack;

							ulDwordBit --;

						} while( -- ulW );

						pwWordPtrFB += ulFinalFrameBufStrideInWords;
						pdwDwordPtr += ulFinalFontTblStrideInDwords;
					}
				}
			}
			break;

		case 3: // ### 3 byte per pixel ###
			{
				BYTE*			pbBytePtrFB;
				DWORD			dwFore, dwBack;
				BYTE			bForeRed, bForeGreen, bForeBlue;
				BYTE			bBackRed, bBackGreen, bBackBlue;
				BYTE*			pbCachePtr = (BYTE*) pbDrawCache;

				// Initialize the variables.

				ulFinalFrameBufStrideInBytes =
					pdglLayouts->vmiVideoMemInfo.lDisplayPitch - pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits * 3;

				if ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRBitMask == 0xFF0000 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwGBitMask == 0xFF00 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwBBitMask == 0xFF )
				{
					// ### RRGGBB ##

					dwFore = g_vdwColorTable00RRGGBB[ bForeColor ];
					dwBack = g_vdwColorTable00RRGGBB[ bBackColor ];

					bForeBlue = (BYTE) ( dwFore & 0xFF );
					bForeGreen = (BYTE) ( ( dwFore >> 8 ) & 0xFF );
					bForeRed = (BYTE) ( ( dwFore >> 16 ) & 0xFF );

					bBackBlue = (BYTE) ( dwBack & 0xFF );
					bBackGreen = (BYTE) ( ( dwBack >> 8 ) & 0xFF );
					bBackRed = (BYTE) ( ( dwBack >> 16 ) & 0xFF );
				}
				else
					return STATUS_UNSUCCESSFUL;

				pbBytePtrFB = (BYTE*) pbDisplayStart;
				pdwDwordPtr = pdwDwordStart;

				// Draw in the Framebuffer.

				if ( ulRenderWithDrawCache )
				{
					ULONG		ulLineLength = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits * 3;
					ULONG		ulVideoStride = pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
					ULONG		ulCacheStride = ulLineLength;

					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						memcpy( pbBytePtrFB, pbCachePtr, ulLineLength );
						pbBytePtrFB += ulVideoStride;
						pbCachePtr += ulCacheStride;
					}
				}
				else
				{
					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						ulW = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
						pdwDwordPtrLine = pdwDwordPtr;

						dwDword = * pdwDwordPtr;
						ulDwordBit = ulDwordBitStart;

						do
						{
							if ( ulDwordBit == 0xFFFFFFFF )
							{
								dwDword = * ++ pdwDwordPtrLine;
								ulDwordBit = 31;
							}

							if ( ( dwDword >> ulDwordBit ) & 1 )
							{
								* pbCachePtr ++ = * pbBytePtrFB ++ = bForeBlue;
								* pbCachePtr ++ = * pbBytePtrFB ++ = bForeGreen;
								* pbCachePtr ++ = * pbBytePtrFB ++ = bForeRed;
							}
							else
							{
								* pbCachePtr ++ = * pbBytePtrFB ++ = bBackBlue;
								* pbCachePtr ++ = * pbBytePtrFB ++ = bBackGreen;
								* pbCachePtr ++ = * pbBytePtrFB ++ = bBackRed;
							}

							ulDwordBit --;

						} while( -- ulW );

						pbBytePtrFB += ulFinalFrameBufStrideInBytes;
						pdwDwordPtr += ulFinalFontTblStrideInDwords;
					}
				}
			}
			break;

		case 4: // ### 4 byte per pixel ###
			{
				DWORD*			pdwDwordPtrFB;
				DWORD			dwFore, dwBack;
				DWORD*			pdwCachePtr = (DWORD*) pbDrawCache;

				// Initialize the variables.

				ulFinalFrameBufStrideInDwords =
					pdglLayouts->vmiVideoMemInfo.lDisplayPitch / 4 - pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;

				if ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRBitMask == 0xFF0000 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwGBitMask == 0xFF00 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwBBitMask == 0xFF )
				{
					// ### 00RRGGBB ##

					dwFore = g_vdwColorTable00RRGGBB[ bForeColor ];
					dwBack = g_vdwColorTable00RRGGBB[ bBackColor ];
				}
				else
					return STATUS_UNSUCCESSFUL;

				pdwDwordPtrFB = (DWORD*) pbDisplayStart;
				pdwDwordPtr = pdwDwordStart;

				// Draw in the Framebuffer.

				if ( ulRenderWithDrawCache )
				{
					ULONG		ulLineLength = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits * sizeof( DWORD );
					ULONG		ulVideoStride = pdglLayouts->vmiVideoMemInfo.lDisplayPitch / sizeof( DWORD );
					ULONG		ulCacheStride = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;

					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						memcpy( pdwDwordPtrFB, pdwCachePtr, ulLineLength );
						pdwDwordPtrFB += ulVideoStride;
						pdwCachePtr += ulCacheStride;
					}
				}
				else
				{
					for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
					{
						ulW = pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits;
						pdwDwordPtrLine = pdwDwordPtr;

						dwDword = * pdwDwordPtr;
						ulDwordBit = ulDwordBitStart;

						do
						{
							if ( ulDwordBit == 0xFFFFFFFF )
							{
								dwDword = * ++ pdwDwordPtrLine;
								ulDwordBit = 31;
							}

							if ( ( dwDword >> ulDwordBit ) & 1 )
								* pdwCachePtr ++ = * pdwDwordPtrFB ++ = dwFore;
							else
								* pdwCachePtr ++ = * pdwDwordPtrFB ++ = dwBack;

							ulDwordBit --;

						} while( -- ulW );

						pdwDwordPtrFB += ulFinalFrameBufStrideInDwords;
						pdwDwordPtr += ulFinalFontTblStrideInDwords;
					}
				}
			}
			break;

		default:
			return STATUS_UNSUCCESSFUL;
		}
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//============================================
// SaveApplicationScreen Function Definition.
//============================================

NTSTATUS SaveApplicationScreen( IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Check if Text Mode or DirectDraw.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary == NULL ) // ### 80x25 Text Mode ###
	{
		// Check if we can continue.

		if ( pdglLayouts->pvTextVideoBuffer == NULL )
		{
			// Return to the Caller.

			return STATUS_UNSUCCESSFUL;
		}
		else
		{
			WORD*			pwVideoBufferPos;
			ULONG			ulI;
			WORD*			pwSaveBufferPos;

			// Save the Application Screen.

			pwVideoBufferPos = ( WORD* ) pdglLayouts->pvTextVideoBuffer +
				pdglLayouts->vivmVpcICEVideo.ulTargetStartY * pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars +
				pdglLayouts->vivmVpcICEVideo.ulTargetStartX;

			pwSaveBufferPos = ( WORD* ) pdglLayouts->vivmVpcICEVideo.pbApplicationScreen;

			for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars; ulI ++ )
			{
				memcpy( pwSaveBufferPos, pwVideoBufferPos, pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars * sizeof( WORD ) );

				pwVideoBufferPos += MACRO_DEFAULT_TEXTMODE_VIDEO_WIDTH;
				pwSaveBufferPos += pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
			}
		}
	}
	else // ### DirectDraw ###
	{
		BYTE*			pbVideoBufferPos;
		BYTE*			pbSaveBufferPos;

		ULONG			ulLineLengthInBytes;
		ULONG			ulVideoBufferStrideInBytes;
		ULONG			ulSaveBufferStrideInBytes;

		ULONG			ulVideoBufferLinesToMove;

		ULONG			ulI;

		// Set the Metrics Variables of the Screen.

		pbVideoBufferPos = (BYTE*) pdglLayouts->vmiVideoMemInfo.pvPrimary + (ULONG) pdglLayouts->vmiVideoMemInfo.fpPrimary +
			pdglLayouts->vivmVpcICEVideo.ulTargetStartY * pdglLayouts->vmiVideoMemInfo.lDisplayPitch +
			pdglLayouts->vivmVpcICEVideo.ulTargetStartX * ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 );

		pbSaveBufferPos = pdglLayouts->vivmVpcICEVideo.pbApplicationScreen;

		ulLineLengthInBytes = pdglLayouts->vivmVpcICEVideo.ulConsoleWidthInPx *
			( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 );

		ulVideoBufferStrideInBytes = pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
		ulSaveBufferStrideInBytes = ulLineLengthInBytes;

		ulVideoBufferLinesToMove = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars *
			pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits;

		// Save the Application Screen.

		for ( ulI = 0; ulI < ulVideoBufferLinesToMove; ulI ++ )
		{
			memcpy( pbSaveBufferPos, pbVideoBufferPos, ulLineLengthInBytes );

			pbVideoBufferPos += ulVideoBufferStrideInBytes;
			pbSaveBufferPos += ulSaveBufferStrideInBytes;
		}
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//===============================================
// RestoreApplicationScreen Function Definition.
//===============================================

NTSTATUS RestoreApplicationScreen( IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Check if Text Mode or DirectDraw.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary == NULL ) // ### 80x25 Text Mode ###
	{
		// Check if we can continue.

		if ( pdglLayouts->pvTextVideoBuffer == NULL )
		{
			// Return to the Caller.

			return STATUS_UNSUCCESSFUL;
		}
		else
		{
			WORD*			pwVideoBufferPos;
			ULONG			ulI;
			WORD*			pwSaveBufferPos;

			// Restore the Application Screen.

			pwVideoBufferPos = ( WORD* ) pdglLayouts->pvTextVideoBuffer +
				pdglLayouts->vivmVpcICEVideo.ulTargetStartY * pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars +
				pdglLayouts->vivmVpcICEVideo.ulTargetStartX;

			pwSaveBufferPos = ( WORD* ) pdglLayouts->vivmVpcICEVideo.pbApplicationScreen;

			for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars; ulI ++ )
			{
				memcpy( pwVideoBufferPos, pwSaveBufferPos, pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars * sizeof( WORD ) );

				pwVideoBufferPos += MACRO_DEFAULT_TEXTMODE_VIDEO_WIDTH;
				pwSaveBufferPos += pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars;
			}

			// Restore the state of the VGA Registers.

			RestoreVGALowLevelState( pdglLayouts );
		}
	}
	else // ### DirectDraw ###
	{
		BYTE*			pbVideoBufferPos;
		BYTE*			pbSaveBufferPos;

		ULONG			ulLineLengthInBytes;
		ULONG			ulVideoBufferStrideInBytes;
		ULONG			ulSaveBufferStrideInBytes;

		ULONG			ulVideoBufferLinesToMove;

		ULONG			ulI;

		// Set the Metrics Variables of the Screen.

		pbVideoBufferPos = (BYTE*) pdglLayouts->vmiVideoMemInfo.pvPrimary + (ULONG) pdglLayouts->vmiVideoMemInfo.fpPrimary +
			pdglLayouts->vivmVpcICEVideo.ulTargetStartY * pdglLayouts->vmiVideoMemInfo.lDisplayPitch +
			pdglLayouts->vivmVpcICEVideo.ulTargetStartX * ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 );

		pbSaveBufferPos = pdglLayouts->vivmVpcICEVideo.pbApplicationScreen;

		ulLineLengthInBytes = pdglLayouts->vivmVpcICEVideo.ulConsoleWidthInPx *
			( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 );

		ulVideoBufferStrideInBytes = pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
		ulSaveBufferStrideInBytes = ulLineLengthInBytes;

		ulVideoBufferLinesToMove = pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars *
			pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits;

		// Restore the Application Screen.

		for ( ulI = 0; ulI < ulVideoBufferLinesToMove; ulI ++ )
		{
			memcpy( pbVideoBufferPos, pbSaveBufferPos, ulLineLengthInBytes );

			pbVideoBufferPos += ulVideoBufferStrideInBytes;
			pbSaveBufferPos += ulSaveBufferStrideInBytes;
		}
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//=================================
// DrawScreen Function Definition.
//=================================

NTSTATUS DrawScreen( IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	ULONG			ulX, ulY;
	WORD*			pwFrontPtr;
	WORD*			pwBackPtr;
	WORD			wChrToDraw;
	NTSTATUS		nsDrawRes;
	BOOLEAN			bCursorOverwritten = FALSE;
	BOOLEAN			bRememberOldMousePointerPos = FALSE;

	if ( g_bDrawScreenIsEnabled == FALSE )
		return STATUS_SUCCESS;

	// Make Sure that the Cursor is Erased.

	if ( 
			( pdglLayouts->vivmVpcICEVideo.bCursorDisplayed == FALSE &&
			pdglLayouts->vivmVpcICEVideo.ulCursorX != MACRO_CURSORPOS_UNDEFINED_VALUE &&
			pdglLayouts->vivmVpcICEVideo.ulCursorY != MACRO_CURSORPOS_UNDEFINED_VALUE ) ||
			( pdglLayouts->vivmVpcICEVideo.bCursorDisplayed &&
			pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus == FALSE &&
			pdglLayouts->vmiVideoMemInfo.pvPrimary )
		)
	{
		WORD*		pwBackBufferPtr;

		// Reset the Character in the Back Buffer.

		pwBackBufferPtr = pdglLayouts->vivmVpcICEVideo.pwTextBackBuffer +
			pdglLayouts->vivmVpcICEVideo.ulCursorY * pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars +
			pdglLayouts->vivmVpcICEVideo.ulCursorX;

		* pwBackBufferPtr = 0;

		// Reset the Variables.

		if ( pdglLayouts->vivmVpcICEVideo.bCursorDisplayed == FALSE )
		{
			pdglLayouts->vivmVpcICEVideo.ulCursorX = MACRO_CURSORPOS_UNDEFINED_VALUE;
			pdglLayouts->vivmVpcICEVideo.ulCursorY = MACRO_CURSORPOS_UNDEFINED_VALUE;
		}
	}

	// Compare the Front and Back Buffer and Draw in the Framebuffer.

	pwFrontPtr = ( WORD* ) pdglLayouts->vivmVpcICEVideo.pwTextFrontBuffer;
	pwBackPtr = ( WORD* ) pdglLayouts->vivmVpcICEVideo.pwTextBackBuffer;

	for ( ulY = 0; ulY < pdglLayouts->vivmVpcICEVideo.ulTextBuffersHeightInChars; ulY ++ )
	{
		for ( ulX = 0; ulX < pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars; ulX ++ )
		{
			// === Compare the Contents of the Two Buffers. ===

			wChrToDraw = 0;

			if ( * pwFrontPtr == 0 )
			{
				* pwFrontPtr = 0x0720;
				* pwBackPtr = 0x0720;
				wChrToDraw = 0x0720;
			}
			else if ( * pwFrontPtr != * pwBackPtr )
			{
				* pwBackPtr = * pwFrontPtr;
				wChrToDraw = * pwFrontPtr;
			}

			// === Draw the Mouse Pointer, if required. ===

			if ( pdglLayouts->vivmVpcICEVideo.ulMousePointerOldX == ulX &&
				pdglLayouts->vivmVpcICEVideo.ulMousePointerOldY == ulY )
			{
				// The Glyph must be written...

				if ( wChrToDraw == 0 )
					wChrToDraw = * pwFrontPtr;

				// The Variables are Resetted.

				pdglLayouts->vivmVpcICEVideo.ulMousePointerOldX = MACRO_MOUSEPOINTERPOS_UNDEFINED_VALUE;
				pdglLayouts->vivmVpcICEVideo.ulMousePointerOldY = MACRO_MOUSEPOINTERPOS_UNDEFINED_VALUE;
			}

			if ( pdglLayouts->vivmVpcICEVideo.bMousePointerDisplayed )
			{
				if ( pdglLayouts->vivmVpcICEVideo.ulMousePointerX == ulX &&
					pdglLayouts->vivmVpcICEVideo.ulMousePointerY == ulY )
				{
					// Apply the Mouse Pointer Color to the Glyph.

					wChrToDraw = ( * pwFrontPtr ) ^ 0x7700;

					// Remember the Old Position.

					bRememberOldMousePointerPos = TRUE;
				}
			}

			// === Check if we have to Draw Something. ===

			if ( wChrToDraw )
			{
				// Draw the Glyph.

				nsDrawRes = DrawGlyph( ulX, ulY, wChrToDraw >> 8, wChrToDraw & 0xFF, pdglLayouts );

				if ( nsDrawRes != STATUS_SUCCESS )
					return nsDrawRes;

				// Check if the Cursor was Overwritten.

				if ( ulX == pdglLayouts->vivmVpcICEVideo.ulCursorX &&
					ulY == pdglLayouts->vivmVpcICEVideo.ulCursorY )
				{
					bCursorOverwritten = TRUE;
				}
			}

			// === Update the Pointers. ===

			pwFrontPtr ++;
			pwBackPtr ++;
		}
	}

	// Take Care of the Cursor.

	if ( bCursorOverwritten &&
		pdglLayouts->vivmVpcICEVideo.bCursorDisplayed &&
		pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus )
	{
		// Draw the Cursor.

		DrawCursor( pdglLayouts->vivmVpcICEVideo.ulCursorX, pdglLayouts->vivmVpcICEVideo.ulCursorY, pdglLayouts );
	}

	// Check if we have to Save the Mouse Pos.

	if ( bRememberOldMousePointerPos )
	{
		pdglLayouts->vivmVpcICEVideo.ulMousePointerOldX = pdglLayouts->vivmVpcICEVideo.ulMousePointerX;
		pdglLayouts->vivmVpcICEVideo.ulMousePointerOldY = pdglLayouts->vivmVpcICEVideo.ulMousePointerY;
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//======================================
// ReadIOAS Static Function Definition.
//======================================

static VOID ReadIOAS( IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Do the Requested Operation.

	__asm
	{
		push		eax
		push		edx
		push		edi

		// Check if the "Input/Output Address Select" was read.

		mov			edi, pdglLayouts

		mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
		or			dx, dx
		jnz			_IOASWasRead

		mov			dx, 0x3CC
		in			al, dx

		test		al, 1
		jz			_IOASIsNotSet

		// IOAS is set.

		mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort, 0x3D4
		mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort, 0x3D5

		jmp			_IOASWasRead

_IOASIsNotSet:

		// IOAS is not set.

		mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort, 0x3B4
		mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort, 0x3B5

_IOASWasRead:

		pop			edi
		pop			edx
		pop			eax
	}

	// Return to the Caller.

	return;
}

//==========================================
// VGA_REGACCESS_SYNCWAIT Macro Definition.
//==========================================

#define VGA_REGACCESS_SYNCWAIT(n)		__asm push		ecx					\
										__asm mov			ecx, 0x100		\
										__asm _SyncWaitLabel##n:			\
										__asm loop		_SyncWaitLabel##n	\
										__asm pop			ecx

//=================================
// Cursor Color Table Definitions.
//=================================

static BYTE		g_vbCursorColorTable_Paletted[ 256 ] =
{
	/*255*/7, 254, 253, 252, 251, 250, 249, /*248*/0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0
};

//=================================
// DrawCursor Function Definition.
//=================================

NTSTATUS DrawCursor( IN ULONG ulX, IN ULONG ulY, IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Check if Text Mode or DirectDraw.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary == NULL ) // ### 80x25 Text Mode ###
	{
		WORD		wCursorAddress;

		// Check if the "Input/Output Address Select" was read.

		ReadIOAS( pdglLayouts );

		// Set the Position of the Cursor in Text-Mode.

		wCursorAddress = (WORD) ( ulY * pdglLayouts->vivmVpcICEVideo.ulTextBuffersWidthInChars + ulX );

		__asm
		{
			// Push.

			push		eax
			push		ebx
			push		ecx
			push		edx
			push		edi

			// Load EDI.

			mov			edi, pdglLayouts

			// === Read the Previous State of the Registers. ===

			// CRTC Address.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_InfoFull, 0
			jne			_CRTCAddress_OriginalStateAlreadyRead

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_InfoFull, 1

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			in			al, dx
			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_Original, al

_CRTCAddress_OriginalStateAlreadyRead:

			// Index E.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Eh_InfoFull, 0
			jne			_Index0Eh_OriginalStateAlreadyRead

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Eh_InfoFull, 1

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xE
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 1 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx
			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Eh_Original, al

_Index0Eh_OriginalStateAlreadyRead:

			// Index F.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Fh_InfoFull, 0
			jne			_Index0Fh_OriginalStateAlreadyRead

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Fh_InfoFull, 1

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xF
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 2 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx
			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Fh_Original, al

_Index0Fh_OriginalStateAlreadyRead:

			// === Modify the Position of the Cursor. ===

			// Write the New Position.

			mov			bx, wCursorAddress

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xF
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 4 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			mov			al, bl
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 10 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xE
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 5 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			mov			al, bh
			out			dx, al

			// Pop.

			pop			edi
			pop			edx
			pop			ecx
			pop			ebx
			pop			eax
		}
	}
	else // ### DirectDraw ###
	{
		BYTE*			pbVideoBufferPos;
		ULONG			ulI, ulJ;

		// Set the Metrics Variables of the Screen.

		pbVideoBufferPos = (BYTE*) pdglLayouts->vmiVideoMemInfo.pvPrimary + (ULONG) pdglLayouts->vmiVideoMemInfo.fpPrimary +
			( pdglLayouts->vivmVpcICEVideo.ulTargetStartY + ulY * pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits ) * pdglLayouts->vmiVideoMemInfo.lDisplayPitch +
			( pdglLayouts->vivmVpcICEVideo.ulTargetStartX + ulX * pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits ) * ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 );

		// Draw the Cursor according to the Current Video Mode.

		switch ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRGBBitCount / 8 )
		{
		case 1: // ### 1 byte per pixel ###
			{
				for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
				{
					BYTE*		pbPtr = pbVideoBufferPos;

					for ( ulJ = 0; ulJ < pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits; ulJ ++, pbPtr ++ )
						* pbPtr = g_vbCursorColorTable_Paletted[ * pbPtr ];

					pbVideoBufferPos += pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
				}
			}
			break;

		case 2: // ### 2 byte per pixel ###
			{
				WORD		wMask;

				if ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRBitMask == 0xF800 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwGBitMask == 0x7E0 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwBBitMask == 0x1F )
				{
					// ### 565 ###

					wMask = g_vwColorTable565[ 7 ];
				}
				else if ( pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwRBitMask == 0x7C00 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwGBitMask == 0x3E0 &&
					pdglLayouts->vmiVideoMemInfo.ddpfDisplay.dwBBitMask == 0x1F )
				{
					// ### 555 ###

					wMask = g_vwColorTable555[ 7 ];
				}
				else
					return STATUS_UNSUCCESSFUL;

				for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
				{
					WORD*		pwPtr = (WORD*) pbVideoBufferPos;

					for ( ulJ = 0; ulJ < pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits; ulJ ++, pwPtr ++ )
						* pwPtr = ( * pwPtr ) ^ wMask;

					pbVideoBufferPos += pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
				}
			}
			break;

		case 3: // ### 3 byte per pixel ###
			{
				for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
				{
					BYTE*		pbPtr = pbVideoBufferPos;

					for ( ulJ = 0; ulJ < pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits; ulJ ++, pbPtr += 3 )
					{
						pbPtr[ 0 ] = pbPtr[ 0 ] ^ 0xC0;
						pbPtr[ 1 ] = pbPtr[ 1 ] ^ 0xC0;
						pbPtr[ 2 ] = pbPtr[ 2 ] ^ 0xC0;
					}

					pbVideoBufferPos += pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
				}
			}
			break;

		case 4: // ### 4 byte per pixel ###
			{
				DWORD		dwMask = g_vdwColorTable00RRGGBB[ 7 ];

				for ( ulI = 0; ulI < pdglLayouts->vivmVpcICEVideo.ulGlyphHeightInBits; ulI ++ )
				{
					DWORD*		pdwPtr = (DWORD*) pbVideoBufferPos;

					for ( ulJ = 0; ulJ < pdglLayouts->vivmVpcICEVideo.ulGlyphWidthInBits; ulJ ++, pdwPtr ++ )
						* pdwPtr = ( * pdwPtr ) ^ dwMask;

					pbVideoBufferPos += pdglLayouts->vmiVideoMemInfo.lDisplayPitch;
				}
			}
			break;

		default:
			return STATUS_UNSUCCESSFUL;
		}
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//===================================
// EnableCursor Function Definition.
//===================================

NTSTATUS EnableCursor( IN BOOLEAN bEnable, IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Check if Text Mode or DirectDraw.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary == NULL ) // ### 80x25 Text Mode ###
	{
		// Check if the "Input/Output Address Select" was read.

		ReadIOAS( pdglLayouts );

		// Enable the Text Mode Cursor.

		__asm
		{
			// Push.

			push		eax
			push		ebx
			push		ecx
			push		edx
			push		edi

			// Load EDI.

			mov			edi, pdglLayouts

			// === Read the Previous State of the Registers. ===

			// CRTC Address.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_InfoFull, 0
			jne			_CRTCAddress_OriginalStateAlreadyRead

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_InfoFull, 1

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			in			al, dx
			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_Original, al

_CRTCAddress_OriginalStateAlreadyRead:

			// Index A.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Ah_InfoFull, 0
			jne			_Index0Ah_OriginalStateAlreadyRead

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Ah_InfoFull, 1

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xA
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 6 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx
			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Ah_Original, al

_Index0Ah_OriginalStateAlreadyRead:

			// Index B.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Bh_InfoFull, 0
			jne			_Index0Bh_OriginalStateAlreadyRead

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Bh_InfoFull, 1

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xB
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 0 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx
			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Bh_Original, al

_Index0Bh_OriginalStateAlreadyRead:

			// === Enable the Cursor ===

			// Read the "Maximum Scan Line Register".

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0x9
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 7 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx

			mov			cl, al
			and			cl, 0x1F

			// Reset the Skew and Set the Shape of the Cursor.

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xB
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 3 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx

			and			al, 0x80
			or			al, cl

			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 11 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xA
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 8 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			in			al, dx

			and			al, 0xC0
			cmp			bEnable, 0
			jne			_EnableDisableMaskOK
			or			al, 0x20
_EnableDisableMaskOK:

			out			dx, al

			// Pop.

			pop			edi
			pop			edx
			pop			ecx
			pop			ebx
			pop			eax
		}
	}
	else // ### DirectDraw ###
	{
		// === Enable or Disable the DD Cursor ===

		// ---> DO NOTHING <---
	}

	// Keep track of the New State.

	pdglLayouts->vivmVpcICEVideo.bCursorDisplayed = bEnable;

	if ( bEnable )
	{
		// Take Care of the Other Variables.

		if ( pdglLayouts->vivmVpcICEVideo.ulCursorX == MACRO_CURSORPOS_UNDEFINED_VALUE )
			pdglLayouts->vivmVpcICEVideo.ulCursorX = 0;
		if ( pdglLayouts->vivmVpcICEVideo.ulCursorY == MACRO_CURSORPOS_UNDEFINED_VALUE )
			pdglLayouts->vivmVpcICEVideo.ulCursorY = 0;

		pdglLayouts->vivmVpcICEVideo.bCursorBlinkStatus = TRUE;
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//==============================================
// RestoreVGALowLevelState Function Definition.
//==============================================

NTSTATUS RestoreVGALowLevelState( IN DRAWGLYPH_LAYOUTS* pdglLayouts )
{
	// Check if Text Mode or DirectDraw.

	if ( pdglLayouts->vmiVideoMemInfo.pvPrimary == NULL ) // ### 80x25 Text Mode ###
	{
		// Check if the "Input/Output Address Select" was read.

		ReadIOAS( pdglLayouts );

		// Restore the State.

		__asm
		{
			// Push.

			push		eax
			push		ebx
			push		ecx
			push		edx
			push		edi

			// Load EDI.

			mov			edi, pdglLayouts

			// Index A.

			VGA_REGACCESS_SYNCWAIT( 12 )

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Ah_InfoFull, 0
			je			_Index0Ah_WasNeverAccessed

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Ah_InfoFull, 0

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xA
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 0 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			mov			al, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Ah_Original
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 13 )

_Index0Ah_WasNeverAccessed:

			// Index B.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Bh_InfoFull, 0
			je			_Index0Bh_WasNeverAccessed

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Bh_InfoFull, 0

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xB
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 1 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			mov			al, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Bh_Original
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 14 )

_Index0Bh_WasNeverAccessed:

			// Index E.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Eh_InfoFull, 0
			je			_Index0Eh_WasNeverAccessed

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Eh_InfoFull, 0

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xE
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 2 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			mov			al, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Eh_Original
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 15 )

_Index0Eh_WasNeverAccessed:

			// Index F.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Fh_InfoFull, 0
			je			_Index0Fh_WasNeverAccessed

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Fh_InfoFull, 0

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, 0xF
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 3 )

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_DataRegPort
			mov			al, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bIndex0Fh_Original
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 16 )

_Index0Fh_WasNeverAccessed:

			// Address Register.

			cmp			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_InfoFull, 0
			je			_CRTCAddress_WasNeverAccessed

			mov			[ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_InfoFull, 0

			mov			dx, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.wCRTC_AddressRegPort
			mov			al, [ edi ]DRAWGLYPH_LAYOUTS.vivmVpcICEVideo.bCRTCAddress_Original
			out			dx, al

			VGA_REGACCESS_SYNCWAIT( 17 )

_CRTCAddress_WasNeverAccessed:

			// Pop.

			pop			edi
			pop			edx
			pop			ecx
			pop			ebx
			pop			eax
		}
	}
	else // ### DirectDraw ###
	{
		// ---> DO NOTHING <---
	}

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//=========================================
// AddNewVpcICEWindow Function Definition.
//=========================================

NTSTATUS AddNewVpcICEWindow( IN ULONG ulName, IN ULONG ulOrder, IN ULONG ulHeight, IN ULONG ulMinHeight, IN BOOLEAN bHasTopLine, IN BOOLEAN bDisplayed, IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo )
{
	ULONG			ulI;

	// Validation Phase.

	if ( pvivmVideoMemoryInfo->ulWindowsNum >= MACRO_MAXNUM_OF_VPCICE_WINDOWS )
		return STATUS_UNSUCCESSFUL;

	for ( ulI = 0; ulI < pvivmVideoMemoryInfo->ulWindowsNum; ulI ++ )
		if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulName == ulName ||
			pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulOrder == ulOrder )
				return STATUS_UNSUCCESSFUL;

	// Add the Window.

	pvivmVideoMemoryInfo->vvwWindows[ pvivmVideoMemoryInfo->ulWindowsNum ].ulName = ulName;
	pvivmVideoMemoryInfo->vvwWindows[ pvivmVideoMemoryInfo->ulWindowsNum ].ulOrder = ulOrder;
	pvivmVideoMemoryInfo->vvwWindows[ pvivmVideoMemoryInfo->ulWindowsNum ].ulHeight = ulHeight;
	pvivmVideoMemoryInfo->vvwWindows[ pvivmVideoMemoryInfo->ulWindowsNum ].ulMinHeight = ulMinHeight;
	pvivmVideoMemoryInfo->vvwWindows[ pvivmVideoMemoryInfo->ulWindowsNum ].bHasTopLine = bHasTopLine;
	pvivmVideoMemoryInfo->vvwWindows[ pvivmVideoMemoryInfo->ulWindowsNum ].bDisplayed = bDisplayed;

	pvivmVideoMemoryInfo->ulWindowsNum ++;

	// Return to the Caller.

	return STATUS_SUCCESS;
}

//=============================================
// RearrangeVpcICEWindows Function Definition.
//=============================================

static ULONG GetVpcICEWindowsTotalHeight( IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo, ULONG ulConsoleHeight )
{
	ULONG			ulRequestedConsoleHeight;
	ULONG			ulI;

	// Return to Requested Information.

	ulRequestedConsoleHeight = 0;

	for ( ulI = 0; ulI < pvivmVideoMemoryInfo->ulWindowsNum; ulI ++ )
		if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].bDisplayed )
		{
			// Validate.

			if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulHeight < pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulMinHeight )
				pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulHeight = pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulMinHeight;
			else if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulHeight > ulConsoleHeight )
				pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulHeight = ulConsoleHeight;

			// Add the Height.

			ulRequestedConsoleHeight += pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulHeight;

			if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].bHasTopLine )
				ulRequestedConsoleHeight ++;
		}

	return ulRequestedConsoleHeight;
}

NTSTATUS RearrangeVpcICEWindows( IN ULONG ulResizingSpecificWindowName, IN ULONG ulResizingSpecificWindowPrevH, IN ULONG ulX0, IN ULONG ulY0, IN ULONG ulX1, IN ULONG ulY1, IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo )
{
	ULONG			ulRequestedConsoleHeight;
	ULONG			ulI, ulJ, ulK;
	VPCICE_WINDOW*	pviwBelowMostWindow;
	ULONG			ulPrevOrder;
	ULONG			ulConsoleHeight;
	VPCICE_WINDOW*	vpviwOrderedByHeightWindows[ MACRO_MAXNUM_OF_VPCICE_WINDOWS ];
	ULONG			ulOrderedByHeightWindowsPos;
	VPCICE_WINDOW*	pviwResizingSpecificWindow;
	VPCICE_WINDOW*	vpviwOrderedByOrderWindows[ MACRO_MAXNUM_OF_VPCICE_WINDOWS ];
	ULONG			ulOrderedByOrderWindowsPos;
	ULONG			ulCurrPosY;

	ulConsoleHeight = ulY1 - ulY0 + 1;

	// === Rearrange the VpcICE Windows. ===

	// Reset the Coordinates of the Windows.

	for ( ulI = 0; ulI < pvivmVideoMemoryInfo->ulWindowsNum; ulI ++ )
	{
		pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulX0 = 0;
		pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulY0 = 0;
		pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulX1 = 0;
		pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulY1 = 0;
	}

	// Set the pointer to the Specified Window.

	if ( ulResizingSpecificWindowName != MACRO_RESIZINGSPECIFICWINDOWNAME_UNDEF )
	{
		pviwResizingSpecificWindow = GetVpcICEWindowByName( ulResizingSpecificWindowName, pvivmVideoMemoryInfo );
		if ( pviwResizingSpecificWindow == NULL )
			return STATUS_UNSUCCESSFUL;
	}
	else
	{
		pviwResizingSpecificWindow = NULL;
	}

	// Discover the Below Most Window.

	pviwBelowMostWindow = NULL;
	ulPrevOrder = 0;

	for ( ulI = 0; ulI < pvivmVideoMemoryInfo->ulWindowsNum; ulI ++ )
		if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].bDisplayed &&
			pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulOrder >= ulPrevOrder )
		{
			ulPrevOrder = pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulOrder;
			pviwBelowMostWindow = & pvivmVideoMemoryInfo->vvwWindows[ ulI ];
		}

	if ( pviwBelowMostWindow == NULL )
		return STATUS_UNSUCCESSFUL;

	pviwBelowMostWindow->ulHeight = 0;

	// Calculate the Total Height.

	ulRequestedConsoleHeight = GetVpcICEWindowsTotalHeight( pvivmVideoMemoryInfo, ulConsoleHeight );

	// Calculate the Ordered Collection of Windows (by Height and Order).

	ulOrderedByHeightWindowsPos = 0;
	ulOrderedByOrderWindowsPos = 0;

	for ( ulI = 0; ulI < pvivmVideoMemoryInfo->ulWindowsNum; ulI ++ )
		if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].bDisplayed )
		{
			// Take care of the Collection of Windows ordered by Height.

			for ( ulJ = 0; ulJ < ulOrderedByHeightWindowsPos; ulJ ++ )
				if ( vpviwOrderedByHeightWindows[ ulJ ]->ulHeight > pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulHeight )
					break;

			for ( ulK = ulOrderedByHeightWindowsPos - 1; ulK != ulJ - 1; ulK -- )
				vpviwOrderedByHeightWindows[ ulK + 1 ] = vpviwOrderedByHeightWindows[ ulK ];

			vpviwOrderedByHeightWindows[ ulJ ] = & pvivmVideoMemoryInfo->vvwWindows[ ulI ];
			ulOrderedByHeightWindowsPos ++;

			// Take care of the Collection of Windows ordered by Order.

			for ( ulJ = 0; ulJ < ulOrderedByOrderWindowsPos; ulJ ++ )
				if ( vpviwOrderedByOrderWindows[ ulJ ]->ulOrder > pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulOrder )
					break;

			for ( ulK = ulOrderedByOrderWindowsPos - 1; ulK != ulJ - 1; ulK -- )
				vpviwOrderedByOrderWindows[ ulK + 1 ] = vpviwOrderedByOrderWindows[ ulK ];

			vpviwOrderedByOrderWindows[ ulJ ] = & pvivmVideoMemoryInfo->vvwWindows[ ulI ];
			ulOrderedByOrderWindowsPos ++;
		}

	// Check if the Height is too much for the Console.

	if ( ulRequestedConsoleHeight > ulConsoleHeight )
	{
		// Check if we are resizing a Window in particular.

		if ( pviwResizingSpecificWindow )
		{
			pviwResizingSpecificWindow->ulHeight = 0;

			ulRequestedConsoleHeight = GetVpcICEWindowsTotalHeight( pvivmVideoMemoryInfo, ulConsoleHeight );

			if ( ulRequestedConsoleHeight <= ulConsoleHeight )
				pviwResizingSpecificWindow->ulHeight += ulConsoleHeight - ulRequestedConsoleHeight;
			else
				pviwResizingSpecificWindow->ulHeight = ulResizingSpecificWindowPrevH;

			ulRequestedConsoleHeight = GetVpcICEWindowsTotalHeight( pvivmVideoMemoryInfo, ulConsoleHeight );
		}

		// Check whether we have to resize all the Windows.

		if ( ulRequestedConsoleHeight > ulConsoleHeight )
		{
			// Iterate through all the Displayed Windows.

			for ( ulJ = vpviwOrderedByHeightWindows[ ulOrderedByHeightWindowsPos - 1 ]->ulHeight - 1; ulJ != 0xFFFFFFFF; ulJ -- )
			{
				// Iterate through all the Windows.

				for ( ulI = ulOrderedByHeightWindowsPos - 1; ulI != 0xFFFFFFFF; ulI -- )
				{
					// Decrement the Height of the Window.

					if ( vpviwOrderedByHeightWindows[ ulI ]->ulHeight != 0 )
						vpviwOrderedByHeightWindows[ ulI ]->ulHeight --;
					else
						continue;

					// Check the Results.

					ulRequestedConsoleHeight = GetVpcICEWindowsTotalHeight( pvivmVideoMemoryInfo, ulConsoleHeight );

					if ( ulRequestedConsoleHeight <= ulConsoleHeight )
						break;
				}

				if ( ulI != 0xFFFFFFFF )
					break;
			}

			if ( ulJ == 0xFFFFFFFF )
				return STATUS_UNSUCCESSFUL;
		}
	}

	// Calculate the Coordinates of the Windows.

	ulCurrPosY = ulY0;

	for ( ulI = 0; ulI < ulOrderedByOrderWindowsPos; ulI ++ )
	{
		vpviwOrderedByOrderWindows[ ulI ]->ulX0 = ulX0;
		vpviwOrderedByOrderWindows[ ulI ]->ulX1 = ulX1;

		if ( vpviwOrderedByOrderWindows[ ulI ]->bHasTopLine )
			ulCurrPosY ++;

		vpviwOrderedByOrderWindows[ ulI ]->ulY0 = ulCurrPosY;

		ulCurrPosY += vpviwOrderedByOrderWindows[ ulI ]->ulHeight;

		vpviwOrderedByOrderWindows[ ulI ]->ulY1 = ulCurrPosY - 1;
	}

	// Adjust the Informations about the Below Most Window.

	pviwBelowMostWindow->ulHeight += ulY1 - ulCurrPosY;
	pviwBelowMostWindow->ulY1 += ulY1 - ulCurrPosY;

	// === Return to the Caller. ===

	return STATUS_SUCCESS;
}

//============================================
// GetVpcICEWindowByName Function Definition.
//============================================

VPCICE_WINDOW* GetVpcICEWindowByName( ULONG ulName, IN VPCICE_VIDEOMEMORY* pvivmVideoMemoryInfo )
{
	ULONG			ulI;

	// Return the Specified Window.

	for ( ulI = 0; ulI < pvivmVideoMemoryInfo->ulWindowsNum; ulI ++ )
		if ( pvivmVideoMemoryInfo->vvwWindows[ ulI ].ulName == ulName )
			return & pvivmVideoMemoryInfo->vvwWindows[ ulI ];

	return NULL;
}
