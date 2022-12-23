/***************************************************************************************
  *
  * IOCTLs_Video.h - Definition File for Kernel Driver IOCTLs.
  *   Note: This is the ONLY Header File that can be included both by Kernel and Win32 modules.
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

#ifndef VPCICEVID_SYSMODULE
	#include <winioctl.h>
#endif

#include "VIDEOMEMORYINFO.h"

//=====================
// IOCTLs Definitions.
//=====================

#define IOCTL_VPCICEVID_GET_VERSION							CTL_CODE(IOCTL_UNKNOWN_BASE, 0x0800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_VPCICEVID_GET_VIDEOMEMORYINFO					CTL_CODE(IOCTL_UNKNOWN_BASE, 0x0801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_VPCICEVID_SUBSCRIBE_VIDEOMEMORYINFO_BUFFER	CTL_CODE(IOCTL_UNKNOWN_BASE, 0x0802, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//=================================
// IOCTLs Related Data Structures.
//=================================

#include "VIDEOMEMORYINFO.h"

//======================
// General Definitions.
//======================

#define FILE_DEVICE_UNKNOWN             0x00000022
#define IOCTL_UNKNOWN_BASE              FILE_DEVICE_UNKNOWN

//================================================
// Definitions about the Driver Version and Name.
//================================================

#define SYSDRIVER_NAME_ANSI			"VPCIceVd"	// Name of the Driver and Device.
#define SYSDRIVER_NAME_WIDE			L"VPCIceVd"	//   "      "      "      "

#define _VPCICEVID_DRIVER_VERSION_		( 1 )		// Driver Version actually in use.
