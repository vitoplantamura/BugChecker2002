/***************************************************************************************
  *
  * vpcicevd.h - Kernel Module Primary Header File.
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

//==============
// Definitions.
//==============

#define MACRO_PROGRAM_NAME					"BugChecker"

//=============
// Structures.
//=============

// --> Device Extension Structure Definition. <--

typedef struct _DEVICE_EXTENSION 
{
	// Informations about the Display Driver dll name.

	NTSTATUS			nsDisplayDriverDllNameGetOpRes;
	UNICODE_STRING		usDisplayDriverDllName;
	FAST_MUTEX			fmDisplayDriverDllNameFastMutex;

	// Informations/Structures about the Display Driver Hook Operation.

	ULONG				ulDisplayDriverHookedCounter;
	FAST_MUTEX			fmDisplayDriverHookedCounterFastMutex;

	// Informations/Structures about the "DrvEnableDriver" Hook.

	ULONG				ulHookedDrvEnableDriverCallsCounter;
	FAST_MUTEX			fmHookedDrvEnableDriverCallsCounterFastMutex;

	// Informations about the system Current Video Mode and VIDEOMEMORYINFO Subscriptions.

	VIDEOMEMORYINFO		vmiVideoMemoryInfo;
	FAST_MUTEX			fmVideoMemoryInfoFastMutex;

	ULONG				ulVmiSubscriptionsNum;
	#define		MACRO_VMI_SUBSCRIPTIONS_MAXNUM		16
	VIDEOMEMORYINFO*	vpvmiVmiSubscriptions[ MACRO_VMI_SUBSCRIPTIONS_MAXNUM ];
	FAST_MUTEX			fmVmiSubscriptionsDataFastMutex;

	// Informations about the Text Video Buffer;

	PVOID				pvTextVideoBuffer;

	// Informations about the Cancel Message displayed at the Start-Up.

	BOOLEAN				bStartUpCancelMessageAccepted;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// --> Externs <--

extern PDEVICE_OBJECT		g_pDeviceObject;

//======================
// Function Prototypes.
//======================

NTSTATUS UnicodeStringToUnicode( IN PUNICODE_STRING pusInput, OUT WCHAR* pszOutput, IN ULONG ulOutputSizeInBytes );
NTSTATUS IsEnforceWriteProtectionSetTo0( OUT BOOLEAN* pbResult );
NTSTATUS InitializeTextVideoBufferPtr( VOID );
