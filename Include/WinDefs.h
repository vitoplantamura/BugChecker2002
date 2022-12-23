/***************************************************************************************
  *
  * WinDefs.h - VPCICE Windows Definitions Header File.
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

//=======================
// Required Definitions.
//=======================

typedef ULONG		DWORD;
typedef UCHAR		BYTE, *PBYTE;

//===========
// Includes.
//===========

#include <ntddk.h>
#include "VIDEOMEMORYINFO.h"

//====================================================
// Required Structure Definitions taken from WINNT.H.
//====================================================

typedef unsigned short		WORD;

typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    WORD   e_magic;                     // Magic number
    WORD   e_cblp;                      // Bytes on last page of file
    WORD   e_cp;                        // Pages in file
    WORD   e_crlc;                      // Relocations
    WORD   e_cparhdr;                   // Size of header in paragraphs
    WORD   e_minalloc;                  // Minimum extra paragraphs needed
    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    WORD   e_ss;                        // Initial (relative) SS value
    WORD   e_sp;                        // Initial SP value
    WORD   e_csum;                      // Checksum
    WORD   e_ip;                        // Initial IP value
    WORD   e_cs;                        // Initial (relative) CS value
    WORD   e_lfarlc;                    // File address of relocation table
    WORD   e_ovno;                      // Overlay number
    WORD   e_res[4];                    // Reserved words
    WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    WORD   e_oeminfo;                   // OEM information; e_oemid specific
    WORD   e_res2[10];                  // Reserved words
    LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_OPTIONAL_HEADER {
    //
    // Standard fields.
    //

    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;

    //
    // NT additional fields.
    //

    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_NT_HEADERS {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef IMAGE_NT_HEADERS32		IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS32		PIMAGE_NT_HEADERS;

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00

#define WINAPI      __stdcall
#define APIENTRY    WINAPI

typedef LONG_PTR (APIENTRY *PFN)();

typedef struct  _DRVFN  /* drvfn */
{
    ULONG   iFunc;
    PFN     pfn;
} DRVFN, *PDRVFN;

typedef struct  tagDRVENABLEDATA
{
    ULONG   iDriverVersion;
    ULONG   c;
    DRVFN  *pdrvfn;
} DRVENABLEDATA, *PDRVENABLEDATA;

typedef BOOLEAN BOOL;

DECLARE_HANDLE(DHPDEV);

#define DD_ROP_SPACE            (256/32)        // space required to store ROP array

typedef struct _DDSCAPS
{
    DWORD       dwCaps;         // capabilities of surface wanted
} DDSCAPS;

typedef struct _DDNTCORECAPS
{
    DWORD       dwSize;                 // size of the DDDRIVERCAPS structure
    DWORD       dwCaps;                 // driver specific capabilities
    DWORD       dwCaps2;                // more driver specific capabilites
    DWORD       dwCKeyCaps;             // color key capabilities of the surface
    DWORD       dwFXCaps;               // driver specific stretching and effects capabilites
    DWORD       dwFXAlphaCaps;          // alpha driver specific capabilities
    DWORD       dwPalCaps;              // palette capabilities
    DWORD       dwSVCaps;               // stereo vision capabilities
    DWORD       dwAlphaBltConstBitDepths;       // DDBD_2,4,8
    DWORD       dwAlphaBltPixelBitDepths;       // DDBD_1,2,4,8
    DWORD       dwAlphaBltSurfaceBitDepths;     // DDBD_1,2,4,8
    DWORD       dwAlphaOverlayConstBitDepths;   // DDBD_2,4,8
    DWORD       dwAlphaOverlayPixelBitDepths;   // DDBD_1,2,4,8
    DWORD       dwAlphaOverlaySurfaceBitDepths; // DDBD_1,2,4,8
    DWORD       dwZBufferBitDepths;             // DDBD_8,16,24,32
    DWORD       dwVidMemTotal;          // total amount of video memory
    DWORD       dwVidMemFree;           // amount of free video memory
    DWORD       dwMaxVisibleOverlays;   // maximum number of visible overlays
    DWORD       dwCurrVisibleOverlays;  // current number of visible overlays
    DWORD       dwNumFourCCCodes;       // number of four cc codes
    DWORD       dwAlignBoundarySrc;     // source rectangle alignment
    DWORD       dwAlignSizeSrc;         // source rectangle byte size
    DWORD       dwAlignBoundaryDest;    // dest rectangle alignment
    DWORD       dwAlignSizeDest;        // dest rectangle byte size
    DWORD       dwAlignStrideAlign;     // stride alignment
    DWORD       dwRops[DD_ROP_SPACE];   // ROPS supported
    DDSCAPS     ddsCaps;                // DDSCAPS structure has all the general capabilities
    DWORD       dwMinOverlayStretch;    // minimum overlay stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    DWORD       dwMaxOverlayStretch;    // maximum overlay stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    DWORD       dwMinLiveVideoStretch;  // minimum live video stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    DWORD       dwMaxLiveVideoStretch;  // maximum live video stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    DWORD       dwMinHwCodecStretch;    // minimum hardware codec stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    DWORD       dwMaxHwCodecStretch;    // maximum hardware codec stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    DWORD       dwReserved1;            // reserved
    DWORD       dwReserved2;            // reserved
    DWORD       dwReserved3;            // reserved
    DWORD       dwSVBCaps;              // driver specific capabilities for System->Vmem blts
    DWORD       dwSVBCKeyCaps;          // driver color key capabilities for System->Vmem blts
    DWORD       dwSVBFXCaps;            // driver FX capabilities for System->Vmem blts
    DWORD       dwSVBRops[DD_ROP_SPACE];// ROPS supported for System->Vmem blts
    DWORD       dwVSBCaps;              // driver specific capabilities for Vmem->System blts
    DWORD       dwVSBCKeyCaps;          // driver color key capabilities for Vmem->System blts
    DWORD       dwVSBFXCaps;            // driver FX capabilities for Vmem->System blts
    DWORD       dwVSBRops[DD_ROP_SPACE];// ROPS supported for Vmem->System blts
    DWORD       dwSSBCaps;              // driver specific capabilities for System->System blts
    DWORD       dwSSBCKeyCaps;          // driver color key capabilities for System->System blts
    DWORD       dwSSBFXCaps;            // driver FX capabilities for System->System blts
    DWORD       dwSSBRops[DD_ROP_SPACE];// ROPS supported for System->System blts
    DWORD       dwMaxVideoPorts;        // maximum number of usable video ports
    DWORD       dwCurrVideoPorts;       // current number of video ports used
    DWORD       dwSVBCaps2;             // more driver specific capabilities for System->Vmem blts
} DDNTCORECAPS, *PDDNTCORECAPS;

typedef DWORD   (APIENTRY *PDD_GETDRIVERINFO)(PDD_GETDRIVERINFODATA);

typedef VOID* LPVOID;

typedef DWORD   (APIENTRY *PDD_CANCREATESURFACE)(PDD_CANCREATESURFACEDATA );
typedef DWORD   (APIENTRY *PDD_CREATESURFACE)(PDD_CREATESURFACEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_DESTROYSURFACE)(PDD_DESTROYSURFACEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_LOCK)(PDD_LOCKDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_UNLOCK)(PDD_UNLOCKDATA);

typedef struct _DD_D3DBUFCALLBACKS
{
    DWORD dwSize;
    DWORD dwFlags;
    PDD_CANCREATESURFACE        CanCreateD3DBuffer;
    PDD_CREATESURFACE           CreateD3DBuffer;
    PDD_SURFCB_DESTROYSURFACE   DestroyD3DBuffer;
    PDD_SURFCB_LOCK             LockD3DBuffer;
    PDD_SURFCB_UNLOCK           UnlockD3DBuffer;
} DD_D3DBUFCALLBACKS, *PDD_D3DBUFCALLBACKS;

typedef struct _DD_HALINFO
{
    DWORD                       dwSize;
    VIDEOMEMORYINFO             vmiData;                // video memory info
    DDNTCORECAPS                ddCaps;                 // hw specific caps
    PDD_GETDRIVERINFO           GetDriverInfo;          // callback for querying driver data
    DWORD                       dwFlags;                // create flags
    LPVOID                      lpD3DGlobalDriverData;  // D3D global Data
    LPVOID                      lpD3DHALCallbacks;      // D3D callbacks
    PDD_D3DBUFCALLBACKS         lpD3DBufCallbacks;      // Buffer callbacks
} DD_HALINFO, *PDD_HALINFO;

typedef struct _VIDEOMEMORY
{
    DWORD               dwFlags;        // flags
    FLATPTR             fpStart;        // start of memory chunk
    union
    {
        FLATPTR         fpEnd;          // end of memory chunk
        DWORD           dwWidth;        // width of chunk (rectanglar memory)
    };
    DDSCAPS             ddsCaps;        // what this memory CANNOT be used for
    DDSCAPS             ddsCapsAlt;     // what this memory CANNOT be used for if it must
    union
    {
        struct _VMEMHEAP *lpHeap;       // heap pointer, used by DDRAW
        DWORD           dwHeight;       // height of chunk (rectanguler memory)
    };
} VIDEOMEMORY;

#define INDEX_DrvGetDirectDrawInfo              59L

//
// Section header format.
//

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            DWORD   PhysicalAddress;
            DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#ifndef MAX_PATH
#define MAX_PATH                    260
#endif
