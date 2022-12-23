/***************************************************************************************
  *
  * crt.c - VPCICE Support Routines from CRT - Source File.
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

//===========
// Includes.
//===========

#include "..\Include\crt.h"

//=====================
// Definitions/Macros.
//=====================

#define FL_UNSIGNED   1       /* strtoul called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

#define HUGE_VAL _HUGE

// // //

#define SWITCH_CASE_ECX( value, label )		\
											__asm cmp		ecx, value		\
											__asm je		label

#define SWITCH_CASE_EAX( value, label )		\
											__asm cmp		eax, value		\
											__asm je		label

//==========
// Externs.
//==========

extern double _HUGE;

//=============
// Structures.
//=============

/*
 * typedef for _fltin
 */

typedef struct _flt
{
        int flags;
        int nbytes;          /* number of characters read */
        long lval;
        double dval;         /* the returned floating point number */
}
*FLT;

//=========================================
//
// DISASSEMBLED FROM C-RUNTIME SOURCE FILES...
//
//=========================================

static struct _flt		off_428A54;

static DWORD ___mb_cur_max   /*dd*/ = 1;

static BYTE ___decimal_point /*db*/ = 0x2E;

static BYTE unk_428A6A [] = {
/*.data:00428A6A*/ /*unk_428A6A*/      /*db*/  0x20 , //               ; DATA XREF: .data:00428A60.o
/*.data:00428A6A*/                                         // ; .data:00428A64.o
/*.data:00428A6B*/                 /*db*/    0x0 , //  
/*.data:00428A6C*/                 /*db*/  0x20 , //  
/*.data:00428A6D*/                 /*db*/    0x0 , //  
/*.data:00428A6E*/                 /*db*/  0x20 , //  
/*.data:00428A6F*/                 /*db*/    0x0 , //  
/*.data:00428A70*/                 /*db*/  0x20 , //  
/*.data:00428A71*/                 /*db*/    0x0 , //  
/*.data:00428A72*/                 /*db*/  0x20 , //  
/*.data:00428A73*/                 /*db*/    0x0 , //  
/*.data:00428A74*/                 /*db*/  0x20 , //  
/*.data:00428A75*/                 /*db*/    0x0 , //  
/*.data:00428A76*/                 /*db*/  0x20 , //  
/*.data:00428A77*/                 /*db*/    0x0 , //  
/*.data:00428A78*/                 /*db*/  0x20 , //  
/*.data:00428A79*/                 /*db*/    0x0 , //  
/*.data:00428A7A*/                 /*db*/  0x20 , //  
/*.data:00428A7B*/                 /*db*/    0x0 , //  
/*.data:00428A7C*/                 /*db*/  0x28 , // (
/*.data:00428A7D*/                 /*db*/    0x0 , //  
/*.data:00428A7E*/                 /*db*/  0x28 , // (
/*.data:00428A7F*/                 /*db*/    0x0 , //  
/*.data:00428A80*/                 /*db*/  0x28 , // (
/*.data:00428A81*/                 /*db*/    0x0 , //  
/*.data:00428A82*/                 /*db*/  0x28 , // (
/*.data:00428A83*/                 /*db*/    0x0 , //  
/*.data:00428A84*/                 /*db*/  0x28 , // (
/*.data:00428A85*/                 /*db*/    0x0 , //  
/*.data:00428A86*/                 /*db*/  0x20 , //  
/*.data:00428A87*/                 /*db*/    0x0 , //  
/*.data:00428A88*/                 /*db*/  0x20 , //  
/*.data:00428A89*/                 /*db*/    0x0 , //  
/*.data:00428A8A*/                 /*db*/  0x20 , //  
/*.data:00428A8B*/                 /*db*/    0x0 , //  
/*.data:00428A8C*/                 /*db*/  0x20 , //  
/*.data:00428A8D*/                 /*db*/    0x0 , //  
/*.data:00428A8E*/                 /*db*/  0x20 , //  
/*.data:00428A8F*/                 /*db*/    0x0 , //  
/*.data:00428A90*/                 /*db*/  0x20 , //  
/*.data:00428A91*/                 /*db*/    0x0 , //  
/*.data:00428A92*/                 /*db*/  0x20 , //  
/*.data:00428A93*/                 /*db*/    0x0 , //  
/*.data:00428A94*/                 /*db*/  0x20 , //  
/*.data:00428A95*/                 /*db*/    0x0 , //  
/*.data:00428A96*/                 /*db*/  0x20 , //  
/*.data:00428A97*/                 /*db*/    0x0 , //  
/*.data:00428A98*/                 /*db*/  0x20 , //  
/*.data:00428A99*/                 /*db*/    0x0 , //  
/*.data:00428A9A*/                 /*db*/  0x20 , //  
/*.data:00428A9B*/                 /*db*/    0x0 , //  
/*.data:00428A9C*/                 /*db*/  0x20 , //  
/*.data:00428A9D*/                 /*db*/    0x0 , //  
/*.data:00428A9E*/                 /*db*/  0x20 , //  
/*.data:00428A9F*/                 /*db*/    0x0 , //  
/*.data:00428AA0*/                 /*db*/  0x20 , //  
/*.data:00428AA1*/                 /*db*/    0x0 , //  
/*.data:00428AA2*/                 /*db*/  0x20 , //  
/*.data:00428AA3*/                 /*db*/    0x0 , //  
/*.data:00428AA4*/                 /*db*/  0x20 , //  
/*.data:00428AA5*/                 /*db*/    0x0 , //  
/*.data:00428AA6*/                 /*db*/  0x20 , //  
/*.data:00428AA7*/                 /*db*/    0x0 , //  
/*.data:00428AA8*/                 /*db*/  0x20 , //  
/*.data:00428AA9*/                 /*db*/    0x0 , //  
/*.data:00428AAA*/                 /*db*/  0x48 , // H
/*.data:00428AAB*/                 /*db*/    0x0 , //  
/*.data:00428AAC*/                 /*db*/  0x10 , //  
/*.data:00428AAD*/                 /*db*/    0x0 , //  
/*.data:00428AAE*/                 /*db*/  0x10 , //  
/*.data:00428AAF*/                 /*db*/    0x0 , //  
/*.data:00428AB0*/                 /*db*/  0x10 , //  
/*.data:00428AB1*/                 /*db*/    0x0 , //  
/*.data:00428AB2*/                 /*db*/  0x10 , //  
/*.data:00428AB3*/                 /*db*/    0x0 , //  
/*.data:00428AB4*/                 /*db*/  0x10 , //  
/*.data:00428AB5*/                 /*db*/    0x0 , //  
/*.data:00428AB6*/                 /*db*/  0x10 , //  
/*.data:00428AB7*/                 /*db*/    0x0 , //  
/*.data:00428AB8*/                 /*db*/  0x10 , //  
/*.data:00428AB9*/                 /*db*/    0x0 , //  
/*.data:00428ABA*/                 /*db*/  0x10 , //  
/*.data:00428ABB*/                 /*db*/    0x0 , //  
/*.data:00428ABC*/                 /*db*/  0x10 , //  
/*.data:00428ABD*/                 /*db*/    0x0 , //  
/*.data:00428ABE*/                 /*db*/  0x10 , //  
/*.data:00428ABF*/                 /*db*/    0x0 , //  
/*.data:00428AC0*/                 /*db*/  0x10 , //  
/*.data:00428AC1*/                 /*db*/    0x0 , //  
/*.data:00428AC2*/                 /*db*/  0x10 , //  
/*.data:00428AC3*/                 /*db*/    0x0 , //  
/*.data:00428AC4*/                 /*db*/  0x10 , //  
/*.data:00428AC5*/                 /*db*/    0x0 , //  
/*.data:00428AC6*/                 /*db*/  0x10 , //  
/*.data:00428AC7*/                 /*db*/    0x0 , //  
/*.data:00428AC8*/                 /*db*/  0x10 , //  
/*.data:00428AC9*/                 /*db*/    0x0 , //  
/*.data:00428ACA*/                 /*db*/  0x84 , // ä
/*.data:00428ACB*/                 /*db*/    0x0 , //  
/*.data:00428ACC*/                 /*db*/  0x84 , // ä
/*.data:00428ACD*/                 /*db*/    0x0 , //  
/*.data:00428ACE*/                 /*db*/  0x84 , // ä
/*.data:00428ACF*/                 /*db*/    0x0 , //  
/*.data:00428AD0*/                 /*db*/  0x84 , // ä
/*.data:00428AD1*/                 /*db*/    0x0 , //  
/*.data:00428AD2*/                 /*db*/  0x84 , // ä
/*.data:00428AD3*/                 /*db*/    0x0 , //  
/*.data:00428AD4*/                 /*db*/  0x84 , // ä
/*.data:00428AD5*/                 /*db*/    0x0 , //  
/*.data:00428AD6*/                 /*db*/  0x84 , // ä
/*.data:00428AD7*/                 /*db*/    0x0 , //  
/*.data:00428AD8*/                 /*db*/  0x84 , // ä
/*.data:00428AD9*/                 /*db*/    0x0 , //  
/*.data:00428ADA*/                 /*db*/  0x84 , // ä
/*.data:00428ADB*/                 /*db*/    0x0 , //  
/*.data:00428ADC*/                 /*db*/  0x84 , // ä
/*.data:00428ADD*/                 /*db*/    0x0 , //  
/*.data:00428ADE*/                 /*db*/  0x10 , //  
/*.data:00428ADF*/                 /*db*/    0x0 , //  
/*.data:00428AE0*/                 /*db*/  0x10 , //  
/*.data:00428AE1*/                 /*db*/    0x0 , //  
/*.data:00428AE2*/                 /*db*/  0x10 , //  
/*.data:00428AE3*/                 /*db*/    0x0 , //  
/*.data:00428AE4*/                 /*db*/  0x10 , //  
/*.data:00428AE5*/                 /*db*/    0x0 , //  
/*.data:00428AE6*/                 /*db*/  0x10 , //  
/*.data:00428AE7*/                 /*db*/    0x0 , //  
/*.data:00428AE8*/                 /*db*/  0x10 , //  
/*.data:00428AE9*/                 /*db*/    0x0 , //  
/*.data:00428AEA*/                 /*db*/  0x10 , //  
/*.data:00428AEB*/                 /*db*/    0x0 , //  
/*.data:00428AEC*/                 /*db*/  0x81 , // ü
/*.data:00428AED*/                 /*db*/    0x0 , //  
/*.data:00428AEE*/                 /*db*/  0x81 , // ü
/*.data:00428AEF*/                 /*db*/    0x0 , //  
/*.data:00428AF0*/                 /*db*/  0x81 , // ü
/*.data:00428AF1*/                 /*db*/    0x0 , //  
/*.data:00428AF2*/                 /*db*/  0x81 , // ü
/*.data:00428AF3*/                 /*db*/    0x0 , //  
/*.data:00428AF4*/                 /*db*/  0x81 , // ü
/*.data:00428AF5*/                 /*db*/    0x0 , //  
/*.data:00428AF6*/                 /*db*/  0x81 , // ü
/*.data:00428AF7*/                 /*db*/    0x0 , //  
/*.data:00428AF8*/                 /*db*/    0x1 , //  
/*.data:00428AF9*/                 /*db*/    0x0 , //  
/*.data:00428AFA*/                 /*db*/    0x1 , //  
/*.data:00428AFB*/                 /*db*/    0x0 , //  
/*.data:00428AFC*/                 /*db*/    0x1 , //  
/*.data:00428AFD*/                 /*db*/    0x0 , //  
/*.data:00428AFE*/                 /*db*/    0x1 , //  
/*.data:00428AFF*/                 /*db*/    0x0 , //  
/*.data:00428B00*/                 /*db*/    0x1 , //  
/*.data:00428B01*/                 /*db*/    0x0 , //  
/*.data:00428B02*/                 /*db*/    0x1 , //  
/*.data:00428B03*/                 /*db*/    0x0 , //  
/*.data:00428B04*/                 /*db*/    0x1 , //  
/*.data:00428B05*/                 /*db*/    0x0 , //  
/*.data:00428B06*/                 /*db*/    0x1 , //  
/*.data:00428B07*/                 /*db*/    0x0 , //  
/*.data:00428B08*/                 /*db*/    0x1 , //  
/*.data:00428B09*/                 /*db*/    0x0 , //  
/*.data:00428B0A*/                 /*db*/    0x1 , //  
/*.data:00428B0B*/                 /*db*/    0x0 , //  
/*.data:00428B0C*/                 /*db*/    0x1 , //  
/*.data:00428B0D*/                 /*db*/    0x0 , //  
/*.data:00428B0E*/                 /*db*/    0x1 , //  
/*.data:00428B0F*/                 /*db*/    0x0 , //  
/*.data:00428B10*/                 /*db*/    0x1 , //  
/*.data:00428B11*/                 /*db*/    0x0 , //  
/*.data:00428B12*/                 /*db*/    0x1 , //  
/*.data:00428B13*/                 /*db*/    0x0 , //  
/*.data:00428B14*/                 /*db*/    0x1 , //  
/*.data:00428B15*/                 /*db*/    0x0 , //  
/*.data:00428B16*/                 /*db*/    0x1 , //  
/*.data:00428B17*/                 /*db*/    0x0 , //  
/*.data:00428B18*/                 /*db*/    0x1 , //  
/*.data:00428B19*/                 /*db*/    0x0 , //  
/*.data:00428B1A*/                 /*db*/    0x1 , //  
/*.data:00428B1B*/                 /*db*/    0x0 , //  
/*.data:00428B1C*/                 /*db*/    0x1 , //  
/*.data:00428B1D*/                 /*db*/    0x0 , //  
/*.data:00428B1E*/                 /*db*/    0x1 , //  
/*.data:00428B1F*/                 /*db*/    0x0 , //  
/*.data:00428B20*/                 /*db*/  0x10 , //  
/*.data:00428B21*/                 /*db*/    0x0 , //  
/*.data:00428B22*/                 /*db*/  0x10 , //  
/*.data:00428B23*/                 /*db*/    0x0 , //  
/*.data:00428B24*/                 /*db*/  0x10 , //  
/*.data:00428B25*/                 /*db*/    0x0 , //  
/*.data:00428B26*/                 /*db*/  0x10 , //  
/*.data:00428B27*/                 /*db*/    0x0 , //  
/*.data:00428B28*/                 /*db*/  0x10 , //  
/*.data:00428B29*/                 /*db*/    0x0 , //  
/*.data:00428B2A*/                 /*db*/  0x10 , //  
/*.data:00428B2B*/                 /*db*/    0x0 , //  
/*.data:00428B2C*/                 /*db*/  0x82 , // é
/*.data:00428B2D*/                 /*db*/    0x0 , //  
/*.data:00428B2E*/                 /*db*/  0x82 , // é
/*.data:00428B2F*/                 /*db*/    0x0 , //  
/*.data:00428B30*/                 /*db*/  0x82 , // é
/*.data:00428B31*/                 /*db*/    0x0 , //  
/*.data:00428B32*/                 /*db*/  0x82 , // é
/*.data:00428B33*/                 /*db*/    0x0 , //  
/*.data:00428B34*/                 /*db*/  0x82 , // é
/*.data:00428B35*/                 /*db*/    0x0 , //  
/*.data:00428B36*/                 /*db*/  0x82 , // é
/*.data:00428B37*/                 /*db*/    0x0 , //  
/*.data:00428B38*/                 /*db*/    0x2 , //  
/*.data:00428B39*/                 /*db*/    0x0 , //  
/*.data:00428B3A*/                 /*db*/    0x2 , //  
/*.data:00428B3B*/                 /*db*/    0x0 , //  
/*.data:00428B3C*/                 /*db*/    0x2 , //  
/*.data:00428B3D*/                 /*db*/    0x0 , //  
/*.data:00428B3E*/                 /*db*/    0x2 , //  
/*.data:00428B3F*/                 /*db*/    0x0 , //  
/*.data:00428B40*/                 /*db*/    0x2 , //  
/*.data:00428B41*/                 /*db*/    0x0 , //  
/*.data:00428B42*/                 /*db*/    0x2 , //  
/*.data:00428B43*/                 /*db*/    0x0 , //  
/*.data:00428B44*/                 /*db*/    0x2 , //  
/*.data:00428B45*/                 /*db*/    0x0 , //  
/*.data:00428B46*/                 /*db*/    0x2 , //  
/*.data:00428B47*/                 /*db*/    0x0 , //  
/*.data:00428B48*/                 /*db*/    0x2 , //  
/*.data:00428B49*/                 /*db*/    0x0 , //  
/*.data:00428B4A*/                 /*db*/    0x2 , //  
/*.data:00428B4B*/                 /*db*/    0x0 , //  
/*.data:00428B4C*/                 /*db*/    0x2 , //  
/*.data:00428B4D*/                 /*db*/    0x0 , //  
/*.data:00428B4E*/                 /*db*/    0x2 , //  
/*.data:00428B4F*/                 /*db*/    0x0 , //  
/*.data:00428B50*/                 /*db*/    0x2 , //  
/*.data:00428B51*/                 /*db*/    0x0 , //  
/*.data:00428B52*/                 /*db*/    0x2 , //  
/*.data:00428B53*/                 /*db*/    0x0 , //  
/*.data:00428B54*/                 /*db*/    0x2 , //  
/*.data:00428B55*/                 /*db*/    0x0 , //  
/*.data:00428B56*/                 /*db*/    0x2 , //  
/*.data:00428B57*/                 /*db*/    0x0 , //  
/*.data:00428B58*/                 /*db*/    0x2 , //  
/*.data:00428B59*/                 /*db*/    0x0 , //  
/*.data:00428B5A*/                 /*db*/    0x2 , //  
/*.data:00428B5B*/                 /*db*/    0x0 , //  
/*.data:00428B5C*/                 /*db*/    0x2 , //  
/*.data:00428B5D*/                 /*db*/    0x0 , //  
/*.data:00428B5E*/                 /*db*/    0x2 , //  
/*.data:00428B5F*/                 /*db*/    0x0 , //  
/*.data:00428B60*/                 /*db*/  0x10 , //  
/*.data:00428B61*/                 /*db*/    0x0 , //  
/*.data:00428B62*/                 /*db*/  0x10 , //  
/*.data:00428B63*/                 /*db*/    0x0 , //  
/*.data:00428B64*/                 /*db*/  0x10 , //  
/*.data:00428B65*/                 /*db*/    0x0 , //  
/*.data:00428B66*/                 /*db*/  0x10 , //  
/*.data:00428B67*/                 /*db*/    0x0 , //  
/*.data:00428B68*/                 /*db*/  0x20 , //  
/*.data:00428B69*/                 /*db*/    0x0 , //  
/*.data:00428B6A*/                 /*db*/    0x0 , //  
/*.data:00428B6B*/                 /*db*/    0x0 , //  
/*.data:00428B6C*/                 /*db*/    0x0 , //  
/*.data:00428B6D*/                 /*db*/    0x0 , //  
/*.data:00428B6E*/                 /*db*/    0x0 , //  
/*.data:00428B6F*/                 /*db*/    0x0 , //  
/*.data:00428B70*/                 /*db*/    0x0 , //  
/*.data:00428B71*/                 /*db*/    0x0 , //  
/*.data:00428B72*/                 /*db*/    0x0 , //  
/*.data:00428B73*/                 /*db*/    0x0 , //  
/*.data:00428B74*/                 /*db*/    0x0 , //  
/*.data:00428B75*/                 /*db*/    0x0 , //  
/*.data:00428B76*/                 /*db*/    0x0 , //  
/*.data:00428B77*/                 /*db*/    0x0 , //  
/*.data:00428B78*/                 /*db*/    0x0 , //  
/*.data:00428B79*/                 /*db*/    0x0 , //  
/*.data:00428B7A*/                 /*db*/    0x0 , //  
/*.data:00428B7B*/                 /*db*/    0x0 , //  
/*.data:00428B7C*/                 /*db*/    0x0 , //  
/*.data:00428B7D*/                 /*db*/    0x0 , //  
/*.data:00428B7E*/                 /*db*/    0x0 , //  
/*.data:00428B7F*/                 /*db*/    0x0 , //  
/*.data:00428B80*/                 /*db*/    0x0 , //  
/*.data:00428B81*/                 /*db*/    0x0 , //  
/*.data:00428B82*/                 /*db*/    0x0 , //  
/*.data:00428B83*/                 /*db*/    0x0 , //  
/*.data:00428B84*/                 /*db*/    0x0 , //  
/*.data:00428B85*/                 /*db*/    0x0 , //  
/*.data:00428B86*/                 /*db*/    0x0 , //  
/*.data:00428B87*/                 /*db*/    0x0 , //  
/*.data:00428B88*/                 /*db*/    0x0 , //  
/*.data:00428B89*/                 /*db*/    0x0 , //  
/*.data:00428B8A*/                 /*db*/    0x0 , //  
/*.data:00428B8B*/                 /*db*/    0x0 , //  
/*.data:00428B8C*/                 /*db*/    0x0 , //  
/*.data:00428B8D*/                 /*db*/    0x0 , //  
/*.data:00428B8E*/                 /*db*/    0x0 , //  
/*.data:00428B8F*/                 /*db*/    0x0 , //  
/*.data:00428B90*/                 /*db*/    0x0 , //  
/*.data:00428B91*/                 /*db*/    0x0 , //  
/*.data:00428B92*/                 /*db*/    0x0 , //  
/*.data:00428B93*/                 /*db*/    0x0 , //  
/*.data:00428B94*/                 /*db*/    0x0 , //  
/*.data:00428B95*/                 /*db*/    0x0 , //  
/*.data:00428B96*/                 /*db*/    0x0 , //  
/*.data:00428B97*/                 /*db*/    0x0 , //  
/*.data:00428B98*/                 /*db*/    0x0 , //  
/*.data:00428B99*/                 /*db*/    0x0 , //  
/*.data:00428B9A*/                 /*db*/    0x0 , //  
/*.data:00428B9B*/                 /*db*/    0x0 , //  
/*.data:00428B9C*/                 /*db*/    0x0 , //  
/*.data:00428B9D*/                 /*db*/    0x0 , //  
/*.data:00428B9E*/                 /*db*/    0x0 , //  
/*.data:00428B9F*/                 /*db*/    0x0 , //  
/*.data:00428BA0*/                 /*db*/    0x0 , //  
/*.data:00428BA1*/                 /*db*/    0x0 , //  
/*.data:00428BA2*/                 /*db*/    0x0 , //  
/*.data:00428BA3*/                 /*db*/    0x0 , //  
/*.data:00428BA4*/                 /*db*/    0x0 , //  
/*.data:00428BA5*/                 /*db*/    0x0 , //  
/*.data:00428BA6*/                 /*db*/    0x0 , //  
/*.data:00428BA7*/                 /*db*/    0x0 , //  
/*.data:00428BA8*/                 /*db*/    0x0 , //  
/*.data:00428BA9*/                 /*db*/    0x0 , //  
/*.data:00428BAA*/                 /*db*/    0x0 , //  
/*.data:00428BAB*/                 /*db*/    0x0 , //  
/*.data:00428BAC*/                 /*db*/    0x0 , //  
/*.data:00428BAD*/                 /*db*/    0x0 , //  
/*.data:00428BAE*/                 /*db*/    0x0 , //  
/*.data:00428BAF*/                 /*db*/    0x0 , //  
/*.data:00428BB0*/                 /*db*/    0x0 , //  
/*.data:00428BB1*/                 /*db*/    0x0 , //  
/*.data:00428BB2*/                 /*db*/    0x0 , //  
/*.data:00428BB3*/                 /*db*/    0x0 , //  
/*.data:00428BB4*/                 /*db*/    0x0 , //  
/*.data:00428BB5*/                 /*db*/    0x0 , //  
/*.data:00428BB6*/                 /*db*/    0x0 , //  
/*.data:00428BB7*/                 /*db*/    0x0 , //  
/*.data:00428BB8*/                 /*db*/    0x0 , //  
/*.data:00428BB9*/                 /*db*/    0x0 , //  
/*.data:00428BBA*/                 /*db*/    0x0 , //  
/*.data:00428BBB*/                 /*db*/    0x0 , //  
/*.data:00428BBC*/                 /*db*/    0x0 , //  
/*.data:00428BBD*/                 /*db*/    0x0 , //  
/*.data:00428BBE*/                 /*db*/    0x0 , //  
/*.data:00428BBF*/                 /*db*/    0x0 , //  
/*.data:00428BC0*/                 /*db*/    0x0 , //  
/*.data:00428BC1*/                 /*db*/    0x0 , //  
/*.data:00428BC2*/                 /*db*/    0x0 , //  
/*.data:00428BC3*/                 /*db*/    0x0 , //  
/*.data:00428BC4*/                 /*db*/    0x0 , //  
/*.data:00428BC5*/                 /*db*/    0x0 , //  
/*.data:00428BC6*/                 /*db*/    0x0 , //  
/*.data:00428BC7*/                 /*db*/    0x0 , //  
/*.data:00428BC8*/                 /*db*/    0x0 , //  
/*.data:00428BC9*/                 /*db*/    0x0 , //  
/*.data:00428BCA*/                 /*db*/    0x0 , //  
/*.data:00428BCB*/                 /*db*/    0x0 , //  
/*.data:00428BCC*/                 /*db*/    0x0 , //  
/*.data:00428BCD*/                 /*db*/    0x0 , //  
/*.data:00428BCE*/                 /*db*/    0x0 , //  
/*.data:00428BCF*/                 /*db*/    0x0 , //  
/*.data:00428BD0*/                 /*db*/    0x0 , //  
/*.data:00428BD1*/                 /*db*/    0x0 , //  
/*.data:00428BD2*/                 /*db*/    0x0 , //  
/*.data:00428BD3*/                 /*db*/    0x0 , //  
/*.data:00428BD4*/                 /*db*/    0x0 , //  
/*.data:00428BD5*/                 /*db*/    0x0 , //  
/*.data:00428BD6*/                 /*db*/    0x0 , //  
/*.data:00428BD7*/                 /*db*/    0x0 , //  
/*.data:00428BD8*/                 /*db*/    0x0 , //  
/*.data:00428BD9*/                 /*db*/    0x0 , //  
/*.data:00428BDA*/                 /*db*/    0x0 , //  
/*.data:00428BDB*/                 /*db*/    0x0 , //  
/*.data:00428BDC*/                 /*db*/    0x0 , //  
/*.data:00428BDD*/                 /*db*/    0x0 , //  
/*.data:00428BDE*/                 /*db*/    0x0 , //  
/*.data:00428BDF*/                 /*db*/    0x0 , //  
/*.data:00428BE0*/                 /*db*/    0x0 , //  
/*.data:00428BE1*/                 /*db*/    0x0 , //  
/*.data:00428BE2*/                 /*db*/    0x0 , //  
/*.data:00428BE3*/                 /*db*/    0x0 , //  
/*.data:00428BE4*/                 /*db*/    0x0 , //  
/*.data:00428BE5*/                 /*db*/    0x0 , //  
/*.data:00428BE6*/                 /*db*/    0x0 , //  
/*.data:00428BE7*/                 /*db*/    0x0 , //  
/*.data:00428BE8*/                 /*db*/    0x0 , //  
/*.data:00428BE9*/                 /*db*/    0x0 , //  
/*.data:00428BEA*/                 /*db*/    0x0 , //  
/*.data:00428BEB*/                 /*db*/    0x0 , //  
/*.data:00428BEC*/                 /*db*/    0x0 , //  
/*.data:00428BED*/                 /*db*/    0x0 , //  
/*.data:00428BEE*/                 /*db*/    0x0 , //  
/*.data:00428BEF*/                 /*db*/    0x0 , //  
/*.data:00428BF0*/                 /*db*/    0x0 , //  
/*.data:00428BF1*/                 /*db*/    0x0 , //  
/*.data:00428BF2*/                 /*db*/    0x0 , //  
/*.data:00428BF3*/                 /*db*/    0x0 , //  
/*.data:00428BF4*/                 /*db*/    0x0 , //  
/*.data:00428BF5*/                 /*db*/    0x0 , //  
/*.data:00428BF6*/                 /*db*/    0x0 , //  
/*.data:00428BF7*/                 /*db*/    0x0 , //  
/*.data:00428BF8*/                 /*db*/    0x0 , //  
/*.data:00428BF9*/                 /*db*/    0x0 , //  
/*.data:00428BFA*/                 /*db*/    0x0 , //  
/*.data:00428BFB*/                 /*db*/    0x0 , //  
/*.data:00428BFC*/                 /*db*/    0x0 , //  
/*.data:00428BFD*/                 /*db*/    0x0 , //  
/*.data:00428BFE*/                 /*db*/    0x0 , //  
/*.data:00428BFF*/                 /*db*/    0x0 , //  
/*.data:00428C00*/                 /*db*/    0x0 , //  
/*.data:00428C01*/                 /*db*/    0x0 , //  
/*.data:00428C02*/                 /*db*/    0x0 , //  
/*.data:00428C03*/                 /*db*/    0x0 , //  
/*.data:00428C04*/                 /*db*/    0x0 , //  
/*.data:00428C05*/                 /*db*/    0x0 , //  
/*.data:00428C06*/                 /*db*/    0x0 , //  
/*.data:00428C07*/                 /*db*/    0x0 , //  
/*.data:00428C08*/                 /*db*/    0x0 , //  
/*.data:00428C09*/                 /*db*/    0x0 , //  
/*.data:00428C0A*/                 /*db*/    0x0 , //  
/*.data:00428C0B*/                 /*db*/    0x0 , //  
/*.data:00428C0C*/                 /*db*/    0x0 , //  
/*.data:00428C0D*/                 /*db*/    0x0 , //  
/*.data:00428C0E*/                 /*db*/    0x0 , //  
/*.data:00428C0F*/                 /*db*/    0x0 , //  
/*.data:00428C10*/                 /*db*/    0x0 , //  
/*.data:00428C11*/                 /*db*/    0x0 , //  
/*.data:00428C12*/                 /*db*/    0x0 , //  
/*.data:00428C13*/                 /*db*/    0x0 , //  
/*.data:00428C14*/                 /*db*/    0x0 , //  
/*.data:00428C15*/                 /*db*/    0x0 , //  
/*.data:00428C16*/                 /*db*/    0x0 , //  
/*.data:00428C17*/                 /*db*/    0x0 , //  
/*.data:00428C18*/                 /*db*/    0x0 , //  
/*.data:00428C19*/                 /*db*/    0x0 , //  
/*.data:00428C1A*/                 /*db*/    0x0 , //  
/*.data:00428C1B*/                 /*db*/    0x0 , //  
/*.data:00428C1C*/                 /*db*/    0x0 , //  
/*.data:00428C1D*/                 /*db*/    0x0 , //  
/*.data:00428C1E*/                 /*db*/    0x0 , //  
/*.data:00428C1F*/                 /*db*/    0x0 , //  
/*.data:00428C20*/                 /*db*/    0x0 , //  
/*.data:00428C21*/                 /*db*/    0x0 , //  
/*.data:00428C22*/                 /*db*/    0x0 , //  
/*.data:00428C23*/                 /*db*/    0x0 , //  
/*.data:00428C24*/                 /*db*/    0x0 , //  
/*.data:00428C25*/                 /*db*/    0x0 , //  
/*.data:00428C26*/                 /*db*/    0x0 , //  
/*.data:00428C27*/                 /*db*/    0x0 , //  
/*.data:00428C28*/                 /*db*/    0x0 , //  
/*.data:00428C29*/                 /*db*/    0x0 , //  
/*.data:00428C2A*/                 /*db*/    0x0 , //  
/*.data:00428C2B*/                 /*db*/    0x0 , //  
/*.data:00428C2C*/                 /*db*/    0x0 , //  
/*.data:00428C2D*/                 /*db*/    0x0 , //  
/*.data:00428C2E*/                 /*db*/    0x0 , //  
/*.data:00428C2F*/                 /*db*/    0x0 , //  
/*.data:00428C30*/                 /*db*/    0x0 , //  
/*.data:00428C31*/                 /*db*/    0x0 , //  
/*.data:00428C32*/                 /*db*/    0x0 , //  
/*.data:00428C33*/                 /*db*/    0x0 , //  
/*.data:00428C34*/                 /*db*/    0x0 , //  
/*.data:00428C35*/                 /*db*/    0x0 , //  
/*.data:00428C36*/                 /*db*/    0x0 , //  
/*.data:00428C37*/                 /*db*/    0x0 , //  
/*.data:00428C38*/                 /*db*/    0x0 , //  
/*.data:00428C39*/                 /*db*/    0x0 , //  
/*.data:00428C3A*/                 /*db*/    0x0 , //  
/*.data:00428C3B*/                 /*db*/    0x0 , //  
/*.data:00428C3C*/                 /*db*/    0x0 , //  
/*.data:00428C3D*/                 /*db*/    0x0 , //  
/*.data:00428C3E*/                 /*db*/    0x0 , //  
/*.data:00428C3F*/                 /*db*/    0x0 , //  
/*.data:00428C40*/                 /*db*/    0x0 , //  
/*.data:00428C41*/                 /*db*/    0x0 , //  
/*.data:00428C42*/                 /*db*/    0x0 , //  
/*.data:00428C43*/                 /*db*/    0x0 , //  
/*.data:00428C44*/                 /*db*/    0x0 , //  
/*.data:00428C45*/                 /*db*/    0x0 , //  
/*.data:00428C46*/                 /*db*/    0x0 , //  
/*.data:00428C47*/                 /*db*/    0x0 , //  
/*.data:00428C48*/                 /*db*/    0x0 , //  
/*.data:00428C49*/                 /*db*/    0x0 , //  
/*.data:00428C4A*/                 /*db*/    0x0 , //  
/*.data:00428C4B*/                 /*db*/    0x0 , //  
/*.data:00428C4C*/                 /*db*/    0x0 , //  
/*.data:00428C4D*/                 /*db*/    0x0 , //  
/*.data:00428C4E*/                 /*db*/    0x0 , //  
/*.data:00428C4F*/                 /*db*/    0x0 , //  
/*.data:00428C50*/                 /*db*/    0x0 , //  
/*.data:00428C51*/                 /*db*/    0x0 , //  
/*.data:00428C52*/                 /*db*/    0x0 , //  
/*.data:00428C53*/                 /*db*/    0x0 , //  
/*.data:00428C54*/                 /*db*/    0x0 , //  
/*.data:00428C55*/                 /*db*/    0x0 , //  
/*.data:00428C56*/                 /*db*/    0x0 , //  
/*.data:00428C57*/                 /*db*/    0x0 , //  
/*.data:00428C58*/                 /*db*/    0x0 , //  
/*.data:00428C59*/                 /*db*/    0x0 , //  
/*.data:00428C5A*/                 /*db*/    0x0 , //  
/*.data:00428C5B*/                 /*db*/    0x0 , //  
/*.data:00428C5C*/                 /*db*/    0x0 , //  
/*.data:00428C5D*/                 /*db*/    0x0 , //  
/*.data:00428C5E*/                 /*db*/    0x0 , //  
/*.data:00428C5F*/                 /*db*/    0x0 , //  
/*.data:00428C60*/                 /*db*/    0x0 , //  
/*.data:00428C61*/                 /*db*/    0x0 , //  
/*.data:00428C62*/                 /*db*/    0x0 , //  
/*.data:00428C63*/                 /*db*/    0x0 , //  
/*.data:00428C64*/                 /*db*/    0x0 , //  
/*.data:00428C65*/                 /*db*/    0x0 , //  
/*.data:00428C66*/                 /*db*/    0x0 , //  
/*.data:00428C67*/                 /*db*/    0x0 , //  
/*.data:00428C68*/                 /*db*/    0x0 , //  
/*.data:00428C69*/                 /*db*/    0x0 , //  
/*.data:00428C6A*/                 /*db*/    0x0 , //  
/*.data:00428C6B*/                 /*db*/    0x0  //  
};

static DWORD __pctype = (DWORD)        /*dd offset*/ & unk_428A6A;

static BYTE			__pow10pos [] = {
/*.data:0042AF88*/ /*__pow10pos*/      /*db*/    0x0 , // ;               ; DATA XREF: ___multtenpow12+6.o
/*.data:0042AF89*/                 /*db*/    0x0 , // ;  
/*.data:0042AF8A*/                 /*db*/    0x0 , // ;  
/*.data:0042AF8B*/                 /*db*/    0x0 , // ;  
/*.data:0042AF8C*/                 /*db*/    0x0 , // ;  
/*.data:0042AF8D*/                 /*db*/    0x0 , // ;  
/*.data:0042AF8E*/                 /*db*/    0x0 , // ;  
/*.data:0042AF8F*/                 /*db*/    0x0 , // ;  
/*.data:0042AF90*/                 /*db*/    0x0 , // ;  
/*.data:0042AF91*/                 /*db*/ 0x0A0 , // ; á
/*.data:0042AF92*/                 /*db*/    0x2 , // ;  
/*.data:0042AF93*/                 /*db*/  0x40 , // ; @
/*.data:0042AF94*/                 /*db*/    0x0 , // ;  
/*.data:0042AF95*/                 /*db*/    0x0 , // ;  
/*.data:0042AF96*/                 /*db*/    0x0 , // ;  
/*.data:0042AF97*/                 /*db*/    0x0 , // ;  
/*.data:0042AF98*/                 /*db*/    0x0 , // ;  
/*.data:0042AF99*/                 /*db*/    0x0 , // ;  
/*.data:0042AF9A*/                 /*db*/    0x0 , // ;  
/*.data:0042AF9B*/                 /*db*/    0x0 , // ;  
/*.data:0042AF9C*/                 /*db*/    0x0 , // ;  
/*.data:0042AF9D*/                 /*db*/ 0x0C8 , // ; +
/*.data:0042AF9E*/                 /*db*/    0x5 , // ;  
/*.data:0042AF9F*/                 /*db*/  0x40 , // ; @
/*.data:0042AFA0*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA1*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA2*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA3*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA4*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA5*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA6*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA7*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA8*/                 /*db*/    0x0 , // ;  
/*.data:0042AFA9*/                 /*db*/ 0x0FA , // ; ·
/*.data:0042AFAA*/                 /*db*/    0x8 , // ;  
/*.data:0042AFAB*/                 /*db*/  0x40 , // ; @
/*.data:0042AFAC*/                 /*db*/    0x0 , // ;  
/*.data:0042AFAD*/                 /*db*/    0x0 , // ;  
/*.data:0042AFAE*/                 /*db*/    0x0 , // ;  
/*.data:0042AFAF*/                 /*db*/    0x0 , // ;  
/*.data:0042AFB0*/                 /*db*/    0x0 , // ;  
/*.data:0042AFB1*/                 /*db*/    0x0 , // ;  
/*.data:0042AFB2*/                 /*db*/    0x0 , // ;  
/*.data:0042AFB3*/                 /*db*/    0x0 , // ;  
/*.data:0042AFB4*/                 /*db*/  0x40 , // ; @
/*.data:0042AFB5*/                 /*db*/  0x9C , // ; £
/*.data:0042AFB6*/                 /*db*/  0x0C , // ;  
/*.data:0042AFB7*/                 /*db*/  0x40 , // ; @
/*.data:0042AFB8*/                 /*db*/    0x0 , // ;  
/*.data:0042AFB9*/                 /*db*/    0x0 , // ;  
/*.data:0042AFBA*/                 /*db*/    0x0 , // ;  
/*.data:0042AFBB*/                 /*db*/    0x0 , // ;  
/*.data:0042AFBC*/                 /*db*/    0x0 , // ;  
/*.data:0042AFBD*/                 /*db*/    0x0 , // ;  
/*.data:0042AFBE*/                 /*db*/    0x0 , // ;  
/*.data:0042AFBF*/                 /*db*/    0x0 , // ;  
/*.data:0042AFC0*/                 /*db*/  0x50 , // ; P
/*.data:0042AFC1*/                 /*db*/ 0x0C3 , // ; +
/*.data:0042AFC2*/                 /*db*/  0x0F , // ;  
/*.data:0042AFC3*/                 /*db*/  0x40 , // ; @
/*.data:0042AFC4*/                 /*db*/    0x0 , // ;  
/*.data:0042AFC5*/                 /*db*/    0x0 , // ;  
/*.data:0042AFC6*/                 /*db*/    0x0 , // ;  
/*.data:0042AFC7*/                 /*db*/    0x0 , // ;  
/*.data:0042AFC8*/                 /*db*/    0x0 , // ;  
/*.data:0042AFC9*/                 /*db*/    0x0 , // ;  
/*.data:0042AFCA*/                 /*db*/    0x0 , // ;  
/*.data:0042AFCB*/                 /*db*/    0x0 , // ;  
/*.data:0042AFCC*/                 /*db*/  0x24 , // ; $
/*.data:0042AFCD*/                 /*db*/ 0x0F4 , // ; (
/*.data:0042AFCE*/                 /*db*/  0x12 , // ;  
/*.data:0042AFCF*/                 /*db*/  0x40 , // ; @
/*.data:0042AFD0*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD1*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD2*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD3*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD4*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD5*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD6*/                 /*db*/    0x0 , // ;  
/*.data:0042AFD7*/                 /*db*/  0x80 , // ; Ç
/*.data:0042AFD8*/                 /*db*/  0x96 , // ; û
/*.data:0042AFD9*/                 /*db*/  0x98 , // ; ÿ
/*.data:0042AFDA*/                 /*db*/  0x16 , // ;  
/*.data:0042AFDB*/                 /*db*/  0x40 , // ; @
/*.data:0042AFDC*/                 /*db*/    0x0 , // ;  
/*.data:0042AFDD*/                 /*db*/    0x0 , // ;  
/*.data:0042AFDE*/                 /*db*/    0x0 , // ;  
/*.data:0042AFDF*/                 /*db*/    0x0 , // ;  
/*.data:0042AFE0*/                 /*db*/    0x0 , // ;  
/*.data:0042AFE1*/                 /*db*/    0x0 , // ;  
/*.data:0042AFE2*/                 /*db*/    0x0 , // ;  
/*.data:0042AFE3*/                 /*db*/  0x20 , // ;  
/*.data:0042AFE4*/                 /*db*/ 0x0BC , // ; +
/*.data:0042AFE5*/                 /*db*/ 0x0BE , // ; +
/*.data:0042AFE6*/                 /*db*/  0x19 , // ;  
/*.data:0042AFE7*/                 /*db*/  0x40 , // ; @
/*.data:0042AFE8*/                 /*db*/    0x0 , // ;  
/*.data:0042AFE9*/                 /*db*/    0x0 , // ;  
/*.data:0042AFEA*/                 /*db*/    0x0 , // ;  
/*.data:0042AFEB*/                 /*db*/    0x0 , // ;  
/*.data:0042AFEC*/                 /*db*/    0x0 , // ;  
/*.data:0042AFED*/                 /*db*/    0x4 , // ;  
/*.data:0042AFEE*/                 /*db*/ 0x0BF , // ; +
/*.data:0042AFEF*/                 /*db*/ 0x0C9 , // ; +
/*.data:0042AFF0*/                 /*db*/  0x1B , // ;  
/*.data:0042AFF1*/                 /*db*/  0x8E , // ; Ä
/*.data:0042AFF2*/                 /*db*/  0x34 , // ; 4
/*.data:0042AFF3*/                 /*db*/  0x40 , // ; @
/*.data:0042AFF4*/                 /*db*/    0x0 , // ;  
/*.data:0042AFF5*/                 /*db*/    0x0 , // ;  
/*.data:0042AFF6*/                 /*db*/    0x0 , // ;  
/*.data:0042AFF7*/                 /*db*/ 0x0A1 , // ; í
/*.data:0042AFF8*/                 /*db*/ 0x0ED , // ; f
/*.data:0042AFF9*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042AFFA*/                 /*db*/ 0x0CE , // ; +
/*.data:0042AFFB*/                 /*db*/  0x1B , // ;  
/*.data:0042AFFC*/                 /*db*/ 0x0C2 , // ; -
/*.data:0042AFFD*/                 /*db*/ 0x0D3 , // ; +
/*.data:0042AFFE*/                 /*db*/  0x4E , // ; N
/*.data:0042AFFF*/                 /*db*/  0x40 , // ; @
/*.data:0042B000*/                 /*db*/  0x20 , // ;  
/*.data:0042B001*/                 /*db*/ 0x0F0 , // ; =
/*.data:0042B002*/                 /*db*/  0x9E , // ; P
/*.data:0042B003*/                 /*db*/ 0x0B5 , // ; ¦
/*.data:0042B004*/                 /*db*/  0x70 , // ; p
/*.data:0042B005*/                 /*db*/  0x2B , // ; +
/*.data:0042B006*/                 /*db*/ 0x0A8 , // ; ¿
/*.data:0042B007*/                 /*db*/ 0x0AD , // ; ¡
/*.data:0042B008*/                 /*db*/ 0x0C5 , // ; +
/*.data:0042B009*/                 /*db*/  0x9D , // ; ¥
/*.data:0042B00A*/                 /*db*/  0x69 , // ; i
/*.data:0042B00B*/                 /*db*/  0x40 , // ; @
/*.data:0042B00C*/                 /*db*/ 0x0D0 , // ; -
/*.data:0042B00D*/                 /*db*/  0x5D , // ; ]
/*.data:0042B00E*/                 /*db*/ 0x0FD , // ; ²
/*.data:0042B00F*/                 /*db*/  0x25 , // ; %
/*.data:0042B010*/                 /*db*/ 0x0E5 , // ; s
/*.data:0042B011*/                 /*db*/  0x1A , // ;  
/*.data:0042B012*/                 /*db*/  0x8E , // ; Ä
/*.data:0042B013*/                 /*db*/  0x4F , // ; O
/*.data:0042B014*/                 /*db*/  0x19 , // ;  
/*.data:0042B015*/                 /*db*/ 0x0EB , // ; d
/*.data:0042B016*/                 /*db*/  0x83 , // ; â
/*.data:0042B017*/                 /*db*/  0x40 , // ; @
/*.data:0042B018*/                 /*db*/  0x71 , // ; q
/*.data:0042B019*/                 /*db*/  0x96 , // ; û
/*.data:0042B01A*/                 /*db*/ 0x0D7 , // ; +
/*.data:0042B01B*/                 /*db*/  0x95 , // ; ò
/*.data:0042B01C*/                 /*db*/  0x43 , // ; C
/*.data:0042B01D*/                 /*db*/  0x0E , // ;  
/*.data:0042B01E*/                 /*db*/    0x5 , // ;  
/*.data:0042B01F*/                 /*db*/  0x8D , // ; ì
/*.data:0042B020*/                 /*db*/  0x29 , // ; )
/*.data:0042B021*/                 /*db*/ 0x0AF , // ; »
/*.data:0042B022*/                 /*db*/  0x9E , // ; P
/*.data:0042B023*/                 /*db*/  0x40 , // ; @
/*.data:0042B024*/                 /*db*/ 0x0F9 , // ; ·
/*.data:0042B025*/                 /*db*/ 0x0BF , // ; +
/*.data:0042B026*/                 /*db*/ 0x0A0 , // ; á
/*.data:0042B027*/                 /*db*/  0x44 , // ; D
/*.data:0042B028*/                 /*db*/ 0x0ED , // ; f
/*.data:0042B029*/                 /*db*/  0x81 , // ; ü
/*.data:0042B02A*/                 /*db*/  0x12 , // ;  
/*.data:0042B02B*/                 /*db*/  0x8F , // ; Å
/*.data:0042B02C*/                 /*db*/  0x81 , // ; ü
/*.data:0042B02D*/                 /*db*/  0x82 , // ; é
/*.data:0042B02E*/                 /*db*/ 0x0B9 , // ; ¦
/*.data:0042B02F*/                 /*db*/  0x40 , // ; @
/*.data:0042B030*/                 /*db*/ 0x0BF , // ; +
/*.data:0042B031*/                 /*db*/  0x3C , // ; <
/*.data:0042B032*/                 /*db*/ 0x0D5 , // ; +
/*.data:0042B033*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B034*/                 /*db*/ 0x0CF , // ; -
/*.data:0042B035*/                 /*db*/ 0x0FF , // ;  
/*.data:0042B036*/                 /*db*/  0x49 , // ; I
/*.data:0042B037*/                 /*db*/  0x1F , // ;  
/*.data:0042B038*/                 /*db*/  0x78 , // ; x
/*.data:0042B039*/                 /*db*/ 0x0C2 , // ; -
/*.data:0042B03A*/                 /*db*/ 0x0D3 , // ; +
/*.data:0042B03B*/                 /*db*/  0x40 , // ; @
/*.data:0042B03C*/                 /*db*/  0x6F , // ; o
/*.data:0042B03D*/                 /*db*/ 0x0C6 , // ; ¦
/*.data:0042B03E*/                 /*db*/ 0x0E0 , // ; a
/*.data:0042B03F*/                 /*db*/  0x8C , // ; î
/*.data:0042B040*/                 /*db*/ 0x0E9 , // ; T
/*.data:0042B041*/                 /*db*/  0x80 , // ; Ç
/*.data:0042B042*/                 /*db*/ 0x0C9 , // ; +
/*.data:0042B043*/                 /*db*/  0x47 , // ; G
/*.data:0042B044*/                 /*db*/ 0x0BA , // ; ¦
/*.data:0042B045*/                 /*db*/  0x93 , // ; ô
/*.data:0042B046*/                 /*db*/ 0x0A8 , // ; ¿
/*.data:0042B047*/                 /*db*/  0x41 , // ; A
/*.data:0042B048*/                 /*db*/ 0x0BC , // ; +
/*.data:0042B049*/                 /*db*/  0x85 , // ; à
/*.data:0042B04A*/                 /*db*/  0x6B , // ; k
/*.data:0042B04B*/                 /*db*/  0x55 , // ; U
/*.data:0042B04C*/                 /*db*/  0x27 , // ; '
/*.data:0042B04D*/                 /*db*/  0x39 , // ; 9
/*.data:0042B04E*/                 /*db*/  0x8D , // ; ì
/*.data:0042B04F*/                 /*db*/ 0x0F7 , // ; ˜
/*.data:0042B050*/                 /*db*/  0x70 , // ; p
/*.data:0042B051*/                 /*db*/ 0x0E0 , // ; a
/*.data:0042B052*/                 /*db*/  0x7C , // ; |
/*.data:0042B053*/                 /*db*/  0x42 , // ; B
/*.data:0042B054*/                 /*db*/ 0x0BC , // ; +
/*.data:0042B055*/                 /*db*/ 0x0DD , // ; ¦
/*.data:0042B056*/                 /*db*/  0x8E , // ; Ä
/*.data:0042B057*/                 /*db*/ 0x0DE , // ; ¦
/*.data:0042B058*/                 /*db*/ 0x0F9 , // ; ·
/*.data:0042B059*/                 /*db*/  0x9D , // ; ¥
/*.data:0042B05A*/                 /*db*/ 0x0FB , // ; v
/*.data:0042B05B*/                 /*db*/ 0x0EB , // ; d
/*.data:0042B05C*/                 /*db*/  0x7E , // ; ~
/*.data:0042B05D*/                 /*db*/ 0x0AA , // ; ¬
/*.data:0042B05E*/                 /*db*/  0x51 , // ; Q
/*.data:0042B05F*/                 /*db*/  0x43 , // ; C
/*.data:0042B060*/                 /*db*/ 0x0A1 , // ; í
/*.data:0042B061*/                 /*db*/ 0x0E6 , // ; µ
/*.data:0042B062*/                 /*db*/  0x76 , // ; v
/*.data:0042B063*/                 /*db*/ 0x0E3 , // ; p
/*.data:0042B064*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B065*/                 /*db*/ 0x0F2 , // ; =
/*.data:0042B066*/                 /*db*/  0x29 , // ; )
/*.data:0042B067*/                 /*db*/  0x2F , // ; /
/*.data:0042B068*/                 /*db*/  0x84 , // ; ä
/*.data:0042B069*/                 /*db*/  0x81 , // ; ü
/*.data:0042B06A*/                 /*db*/  0x26 , // ; &
/*.data:0042B06B*/                 /*db*/  0x44 , // ; D
/*.data:0042B06C*/                 /*db*/  0x28 , // ; (
/*.data:0042B06D*/                 /*db*/  0x10 , // ;  
/*.data:0042B06E*/                 /*db*/  0x17 , // ;  
/*.data:0042B06F*/                 /*db*/ 0x0AA , // ; ¬
/*.data:0042B070*/                 /*db*/ 0x0F8 , // ; °
/*.data:0042B071*/                 /*db*/ 0x0AE , // ; «
/*.data:0042B072*/                 /*db*/  0x10 , // ;  
/*.data:0042B073*/                 /*db*/ 0x0E3 , // ; p
/*.data:0042B074*/                 /*db*/ 0x0C5 , // ; +
/*.data:0042B075*/                 /*db*/ 0x0C4 , // ; -
/*.data:0042B076*/                 /*db*/ 0x0FA , // ; ·
/*.data:0042B077*/                 /*db*/  0x44 , // ; D
/*.data:0042B078*/                 /*db*/ 0x0EB , // ; d
/*.data:0042B079*/                 /*db*/ 0x0A7 , // ; º
/*.data:0042B07A*/                 /*db*/ 0x0D4 , // ; +
/*.data:0042B07B*/                 /*db*/ 0x0F3 , // ; =
/*.data:0042B07C*/                 /*db*/ 0x0F7 , // ; ˜
/*.data:0042B07D*/                 /*db*/ 0x0EB , // ; d
/*.data:0042B07E*/                 /*db*/ 0x0E1 , // ; ß
/*.data:0042B07F*/                 /*db*/  0x4A , // ; J
/*.data:0042B080*/                 /*db*/  0x7A , // ; z
/*.data:0042B081*/                 /*db*/  0x95 , // ; ò
/*.data:0042B082*/                 /*db*/ 0x0CF , // ; -
/*.data:0042B083*/                 /*db*/  0x45 , // ; E
/*.data:0042B084*/                 /*db*/  0x65 , // ; e
/*.data:0042B085*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B086*/                 /*db*/ 0x0C7 , // ; ¦
/*.data:0042B087*/                 /*db*/  0x91 , // ; æ
/*.data:0042B088*/                 /*db*/  0x0E , // ;  
/*.data:0042B089*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B08A*/                 /*db*/ 0x0AE , // ; «
/*.data:0042B08B*/                 /*db*/ 0x0A0 , // ; á
/*.data:0042B08C*/                 /*db*/  0x19 , // ;  
/*.data:0042B08D*/                 /*db*/ 0x0E3 , // ; p
/*.data:0042B08E*/                 /*db*/ 0x0A3 , // ; ú
/*.data:0042B08F*/                 /*db*/  0x46 , // ; F
/*.data:0042B090*/                 /*db*/  0x0D , // ;  
/*.data:0042B091*/                 /*db*/  0x65 , // ; e
/*.data:0042B092*/                 /*db*/  0x17 , // ;  
/*.data:0042B093*/                 /*db*/  0x0C , // ;  
/*.data:0042B094*/                 /*db*/  0x75 , // ; u
/*.data:0042B095*/                 /*db*/  0x81 , // ; ü
/*.data:0042B096*/                 /*db*/  0x86 , // ; å
/*.data:0042B097*/                 /*db*/  0x75 , // ; u
/*.data:0042B098*/                 /*db*/  0x76 , // ; v
/*.data:0042B099*/                 /*db*/ 0x0C9 , // ; +
/*.data:0042B09A*/                 /*db*/  0x48 , // ; 
/*.data:0042B09B*/                 /*db*/  0x4D , // ; M
/*.data:0042B09C*/                 /*db*/  0x58 , // ; X
/*.data:0042B09D*/                 /*db*/  0x42 , // ; B
/*.data:0042B09E*/                 /*db*/ 0x0E4 , // ; S
/*.data:0042B09F*/                 /*db*/ 0x0A7 , // ; º
/*.data:0042B0A0*/                 /*db*/  0x93 , // ; ô
/*.data:0042B0A1*/                 /*db*/  0x39 , // ; 9
/*.data:0042B0A2*/                 /*db*/  0x3B , // ; ;
/*.data:0042B0A3*/                 /*db*/  0x35 , // ; 5
/*.data:0042B0A4*/                 /*db*/ 0x0B8 , // ; +
/*.data:0042B0A5*/                 /*db*/ 0x0B2 , // ; ¦
/*.data:0042B0A6*/                 /*db*/ 0x0ED , // ; f
/*.data:0042B0A7*/                 /*db*/  0x53 , // ; S
/*.data:0042B0A8*/                 /*db*/  0x4D , // ; M
/*.data:0042B0A9*/                 /*db*/ 0x0A7 , // ; º
/*.data:0042B0AA*/                 /*db*/ 0x0E5 , // ; s
/*.data:0042B0AB*/                 /*db*/  0x5D , // ; ]
/*.data:0042B0AC*/                 /*db*/  0x3D , // ; =
/*.data:0042B0AD*/                 /*db*/ 0x0C5 , // ; +
/*.data:0042B0AE*/                 /*db*/  0x5D , // ; ]
/*.data:0042B0AF*/                 /*db*/  0x3B , // ; ;
/*.data:0042B0B0*/                 /*db*/  0x8B , // ; ï
/*.data:0042B0B1*/                 /*db*/  0x9E , // ; P
/*.data:0042B0B2*/                 /*db*/  0x92 , // ; Æ
/*.data:0042B0B3*/                 /*db*/  0x5A , // ; Z
/*.data:0042B0B4*/                 /*db*/ 0x0FF , // ;  
/*.data:0042B0B5*/                 /*db*/  0x5D , // ; ]
/*.data:0042B0B6*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B0B7*/                 /*db*/ 0x0F0 , // ; =
/*.data:0042B0B8*/                 /*db*/ 0x0A1 , // ; í
/*.data:0042B0B9*/                 /*db*/  0x20 , // ;  
/*.data:0042B0BA*/                 /*db*/ 0x0C0 , // ; +
/*.data:0042B0BB*/                 /*db*/  0x54 , // ; T
/*.data:0042B0BC*/                 /*db*/ 0x0A5 , // ; Ñ
/*.data:0042B0BD*/                 /*db*/  0x8C , // ; î
/*.data:0042B0BE*/                 /*db*/  0x37 , // ; 7
/*.data:0042B0BF*/                 /*db*/  0x61 , // ; a
/*.data:0042B0C0*/                 /*db*/ 0x0D1 , // ; -
/*.data:0042B0C1*/                 /*db*/ 0x0FD , // ; ²
/*.data:0042B0C2*/                 /*db*/  0x8B , // ; ï
/*.data:0042B0C3*/                 /*db*/  0x5A , // ; Z
/*.data:0042B0C4*/                 /*db*/  0x8B , // ; ï
/*.data:0042B0C5*/                 /*db*/ 0x0D8 , // ; +
/*.data:0042B0C6*/                 /*db*/  0x25 , // ; %
/*.data:0042B0C7*/                 /*db*/  0x5D , // ; ]
/*.data:0042B0C8*/                 /*db*/  0x89 , // ; ë
/*.data:0042B0C9*/                 /*db*/ 0x0F9 , // ; ·
/*.data:0042B0CA*/                 /*db*/ 0x0DB , // ; ¦
/*.data:0042B0CB*/                 /*db*/  0x67 , // ; g
/*.data:0042B0CC*/                 /*db*/ 0x0AA , // ; ¬
/*.data:0042B0CD*/                 /*db*/  0x95 , // ; ò
/*.data:0042B0CE*/                 /*db*/ 0x0F8 , // ; °
/*.data:0042B0CF*/                 /*db*/ 0x0F3 , // ; =
/*.data:0042B0D0*/                 /*db*/  0x27 , // ; '
/*.data:0042B0D1*/                 /*db*/ 0x0BF , // ; +
/*.data:0042B0D2*/                 /*db*/ 0x0A2 , // ; ó
/*.data:0042B0D3*/                 /*db*/ 0x0C8 , // ; +
/*.data:0042B0D4*/                 /*db*/  0x5D , // ; ]
/*.data:0042B0D5*/                 /*db*/ 0x0DD , // ; ¦
/*.data:0042B0D6*/                 /*db*/  0x80 , // ; Ç
/*.data:0042B0D7*/                 /*db*/  0x6E , // ; n
/*.data:0042B0D8*/                 /*db*/  0x4C , // ; L
/*.data:0042B0D9*/                 /*db*/ 0x0C9 , // ; +
/*.data:0042B0DA*/                 /*db*/  0x9B , // ; ¢
/*.data:0042B0DB*/                 /*db*/  0x97 , // ; ù
/*.data:0042B0DC*/                 /*db*/  0x20 , // ;  
/*.data:0042B0DD*/                 /*db*/  0x8A , // ; è
/*.data:0042B0DE*/                 /*db*/    0x2 , // ;  
/*.data:0042B0DF*/                 /*db*/  0x52 , // ; R
/*.data:0042B0E0*/                 /*db*/  0x60 , // ; `
/*.data:0042B0E1*/                 /*db*/ 0x0C4 , // ; -
/*.data:0042B0E2*/                 /*db*/  0x25 , // ; %
/*.data:0042B0E3*/                 /*db*/  0x75 , // ; u
/*.data:0042B0E4*/                 /*db*/    0x0 , // ;  
/*.data:0042B0E5*/                 /*db*/    0x0 , // ;  
/*.data:0042B0E6*/                 /*db*/    0x0 , // ;  
/*.data:0042B0E7*/                 /*db*/    0x0 , // ;  
/*.data:0042B0E8*/ /*__pow10neg*/      /*db*/ 0x0CD , // ; -             ; DATA XREF: ___multtenpow12+2A.o
/*.data:0042B0E9*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0EA*/                 /*db*/ 0x0CD , // ; -
/*.data:0042B0EB*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0EC*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0ED*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0EE*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0EF*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0F0*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0F1*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B0F2*/                 /*db*/ 0x0FB , // ; v
/*.data:0042B0F3*/                 /*db*/  0x3F , // ; ?
/*.data:0042B0F4*/                 /*db*/  0x71 , // ; q
/*.data:0042B0F5*/                 /*db*/  0x3D , // ; =
/*.data:0042B0F6*/                 /*db*/  0x0A , // ;  
/*.data:0042B0F7*/                 /*db*/ 0x0D7 , // ; +
/*.data:0042B0F8*/                 /*db*/ 0x0A3 , // ; ú
/*.data:0042B0F9*/                 /*db*/  0x70 , // ; p
/*.data:0042B0FA*/                 /*db*/  0x3D , // ; =
/*.data:0042B0FB*/                 /*db*/  0x0A , // ;  
/*.data:0042B0FC*/                 /*db*/ 0x0D7 , // ; +
/*.data:0042B0FD*/                 /*db*/ 0x0A3 , // ; ú
/*.data:0042B0FE*/                 /*db*/ 0x0F8 , // ; °
/*.data:0042B0FF*/                 /*db*/  0x3F , // ; ?
/*.data:0042B100*/                 /*db*/  0x5A , // ; Z
/*.data:0042B101*/                 /*db*/  0x64 , // ; d
/*.data:0042B102*/                 /*db*/  0x3B , // ; ;
/*.data:0042B103*/                 /*db*/ 0x0DF , // ; ¯
/*.data:0042B104*/                 /*db*/  0x4F , // ; O
/*.data:0042B105*/                 /*db*/  0x8D , // ; ì
/*.data:0042B106*/                 /*db*/  0x97 , // ; ù
/*.data:0042B107*/                 /*db*/  0x6E , // ; n
/*.data:0042B108*/                 /*db*/  0x12 , // ;  
/*.data:0042B109*/                 /*db*/  0x83 , // ; â
/*.data:0042B10A*/                 /*db*/ 0x0F5 , // ; )
/*.data:0042B10B*/                 /*db*/  0x3F , // ; ?
/*.data:0042B10C*/                 /*db*/ 0x0C3 , // ; +
/*.data:0042B10D*/                 /*db*/ 0x0D3 , // ; +
/*.data:0042B10E*/                 /*db*/  0x2C , // ; ,
/*.data:0042B10F*/                 /*db*/  0x65 , // ; e
/*.data:0042B110*/                 /*db*/  0x19 , // ;  
/*.data:0042B111*/                 /*db*/ 0x0E2 , // ; G
/*.data:0042B112*/                 /*db*/  0x58 , // ; X
/*.data:0042B113*/                 /*db*/  0x17 , // ;  
/*.data:0042B114*/                 /*db*/ 0x0B7 , // ; +
/*.data:0042B115*/                 /*db*/ 0x0D1 , // ; -
/*.data:0042B116*/                 /*db*/ 0x0F1 , // ; ±
/*.data:0042B117*/                 /*db*/  0x3F , // ; ?
/*.data:0042B118*/                 /*db*/ 0x0D0 , // ; -
/*.data:0042B119*/                 /*db*/  0x0F , // ;  
/*.data:0042B11A*/                 /*db*/  0x23 , // ; #
/*.data:0042B11B*/                 /*db*/  0x84 , // ; ä
/*.data:0042B11C*/                 /*db*/  0x47 , // ; G
/*.data:0042B11D*/                 /*db*/  0x1B , // ;  
/*.data:0042B11E*/                 /*db*/  0x47 , // ; G
/*.data:0042B11F*/                 /*db*/ 0x0AC , // ; ¼
/*.data:0042B120*/                 /*db*/ 0x0C5 , // ; +
/*.data:0042B121*/                 /*db*/ 0x0A7 , // ; º
/*.data:0042B122*/                 /*db*/ 0x0EE , // ; e
/*.data:0042B123*/                 /*db*/  0x3F , // ; ?
/*.data:0042B124*/                 /*db*/  0x40 , // ; @
/*.data:0042B125*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B126*/                 /*db*/ 0x0B6 , // ; ¦
/*.data:0042B127*/                 /*db*/  0x69 , // ; i
/*.data:0042B128*/                 /*db*/  0x6C , // ; l
/*.data:0042B129*/                 /*db*/ 0x0AF , // ; »
/*.data:0042B12A*/                 /*db*/    0x5 , // ;  
/*.data:0042B12B*/                 /*db*/ 0x0BD , // ; +
/*.data:0042B12C*/                 /*db*/  0x37 , // ; 7
/*.data:0042B12D*/                 /*db*/  0x86 , // ; å
/*.data:0042B12E*/                 /*db*/ 0x0EB , // ; d
/*.data:0042B12F*/                 /*db*/  0x3F , // ; ?
/*.data:0042B130*/                 /*db*/  0x33 , // ; 3
/*.data:0042B131*/                 /*db*/  0x3D , // ; =
/*.data:0042B132*/                 /*db*/ 0x0BC , // ; +
/*.data:0042B133*/                 /*db*/  0x42 , // ; B
/*.data:0042B134*/                 /*db*/  0x7A , // ; z
/*.data:0042B135*/                 /*db*/ 0x0E5 , // ; s
/*.data:0042B136*/                 /*db*/ 0x0D5 , // ; +
/*.data:0042B137*/                 /*db*/  0x94 , // ; ö
/*.data:0042B138*/                 /*db*/ 0x0BF , // ; +
/*.data:0042B139*/                 /*db*/ 0x0D6 , // ; +
/*.data:0042B13A*/                 /*db*/ 0x0E7 , // ; t
/*.data:0042B13B*/                 /*db*/  0x3F , // ; ?
/*.data:0042B13C*/                 /*db*/ 0x0C2 , // ; -
/*.data:0042B13D*/                 /*db*/ 0x0FD , // ; ²
/*.data:0042B13E*/                 /*db*/ 0x0FD , // ; ²
/*.data:0042B13F*/                 /*db*/ 0x0CE , // ; +
/*.data:0042B140*/                 /*db*/  0x61 , // ; a
/*.data:0042B141*/                 /*db*/  0x84 , // ; ä
/*.data:0042B142*/                 /*db*/  0x11 , // ;  
/*.data:0042B143*/                 /*db*/  0x77 , // ; w
/*.data:0042B144*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B145*/                 /*db*/ 0x0AB , // ; ½
/*.data:0042B146*/                 /*db*/ 0x0E4 , // ; S
/*.data:0042B147*/                 /*db*/  0x3F , // ; ?
/*.data:0042B148*/                 /*db*/  0x2F , // ; /
/*.data:0042B149*/                 /*db*/  0x4C , // ; L
/*.data:0042B14A*/                 /*db*/  0x5B , // ; [
/*.data:0042B14B*/                 /*db*/ 0x0E1 , // ; ß
/*.data:0042B14C*/                 /*db*/  0x4D , // ; M
/*.data:0042B14D*/                 /*db*/ 0x0C4 , // ; -
/*.data:0042B14E*/                 /*db*/ 0x0BE , // ; +
/*.data:0042B14F*/                 /*db*/  0x94 , // ; ö
/*.data:0042B150*/                 /*db*/  0x95 , // ; ò
/*.data:0042B151*/                 /*db*/ 0x0E6 , // ; µ
/*.data:0042B152*/                 /*db*/ 0x0C9 , // ; +
/*.data:0042B153*/                 /*db*/  0x3F , // ; ?
/*.data:0042B154*/                 /*db*/  0x92 , // ; Æ
/*.data:0042B155*/                 /*db*/ 0x0C4 , // ; -
/*.data:0042B156*/                 /*db*/  0x53 , // ; S
/*.data:0042B157*/                 /*db*/  0x3B , // ; ;
/*.data:0042B158*/                 /*db*/  0x75 , // ; u
/*.data:0042B159*/                 /*db*/  0x44 , // ; D
/*.data:0042B15A*/                 /*db*/ 0x0CD , // ; -
/*.data:0042B15B*/                 /*db*/  0x14 , // ;  
/*.data:0042B15C*/                 /*db*/ 0x0BE , // ; +
/*.data:0042B15D*/                 /*db*/  0x9A , // ; Ü
/*.data:0042B15E*/                 /*db*/ 0x0AF , // ; »
/*.data:0042B15F*/                 /*db*/  0x3F , // ; ?
/*.data:0042B160*/                 /*db*/ 0x0DE , // ; ¦
/*.data:0042B161*/                 /*db*/  0x67 , // ; g
/*.data:0042B162*/                 /*db*/ 0x0BA , // ; ¦
/*.data:0042B163*/                 /*db*/  0x94 , // ; ö
/*.data:0042B164*/                 /*db*/  0x39 , // ; 9
/*.data:0042B165*/                 /*db*/  0x45 , // ; E
/*.data:0042B166*/                 /*db*/ 0x0AD , // ; ¡
/*.data:0042B167*/                 /*db*/  0x1E , // ;  
/*.data:0042B168*/                 /*db*/ 0x0B1 , // ; ¦
/*.data:0042B169*/                 /*db*/ 0x0CF , // ; -
/*.data:0042B16A*/                 /*db*/  0x94 , // ; ö
/*.data:0042B16B*/                 /*db*/  0x3F , // ; ?
/*.data:0042B16C*/                 /*db*/  0x24 , // ; $
/*.data:0042B16D*/                 /*db*/  0x23 , // ; #
/*.data:0042B16E*/                 /*db*/ 0x0C6 , // ; ¦
/*.data:0042B16F*/                 /*db*/ 0x0E2 , // ; G
/*.data:0042B170*/                 /*db*/ 0x0BC , // ; +
/*.data:0042B171*/                 /*db*/ 0x0BA , // ; ¦
/*.data:0042B172*/                 /*db*/  0x3B , // ; ;
/*.data:0042B173*/                 /*db*/  0x31 , // ; 1
/*.data:0042B174*/                 /*db*/  0x61 , // ; a
/*.data:0042B175*/                 /*db*/  0x8B , // ; ï
/*.data:0042B176*/                 /*db*/  0x7A , // ; z
/*.data:0042B177*/                 /*db*/  0x3F , // ; ?
/*.data:0042B178*/                 /*db*/  0x61 , // ; a
/*.data:0042B179*/                 /*db*/  0x55 , // ; U
/*.data:0042B17A*/                 /*db*/  0x59 , // ; Y
/*.data:0042B17B*/                 /*db*/ 0x0C1 , // ; -
/*.data:0042B17C*/                 /*db*/  0x7E , // ; ~
/*.data:0042B17D*/                 /*db*/ 0x0B1 , // ; ¦
/*.data:0042B17E*/                 /*db*/  0x53 , // ; S
/*.data:0042B17F*/                 /*db*/  0x7C , // ; |
/*.data:0042B180*/                 /*db*/  0x12 , // ;  
/*.data:0042B181*/                 /*db*/ 0x0BB , // ; +
/*.data:0042B182*/                 /*db*/  0x5F , // ; _
/*.data:0042B183*/                 /*db*/  0x3F , // ; ?
/*.data:0042B184*/                 /*db*/ 0x0D7 , // ; +
/*.data:0042B185*/                 /*db*/ 0x0EE , // ; e
/*.data:0042B186*/                 /*db*/  0x2F , // ; /
/*.data:0042B187*/                 /*db*/  0x8D , // ; ì
/*.data:0042B188*/                 /*db*/    0x6 , // ;  
/*.data:0042B189*/                 /*db*/ 0x0BE , // ; +
/*.data:0042B18A*/                 /*db*/  0x92 , // ; Æ
/*.data:0042B18B*/                 /*db*/  0x85 , // ; à
/*.data:0042B18C*/                 /*db*/  0x15 , // ;  
/*.data:0042B18D*/                 /*db*/ 0x0FB , // ; v
/*.data:0042B18E*/                 /*db*/  0x44 , // ; D
/*.data:0042B18F*/                 /*db*/  0x3F , // ; ?
/*.data:0042B190*/                 /*db*/  0x24 , // ; $
/*.data:0042B191*/                 /*db*/  0x3F , // ; ?
/*.data:0042B192*/                 /*db*/ 0x0A5 , // ; Ñ
/*.data:0042B193*/                 /*db*/ 0x0E9 , // ; T
/*.data:0042B194*/                 /*db*/  0x39 , // ; 9
/*.data:0042B195*/                 /*db*/ 0x0A5 , // ; Ñ
/*.data:0042B196*/                 /*db*/  0x27 , // ; '
/*.data:0042B197*/                 /*db*/ 0x0EA , // ; O
/*.data:0042B198*/                 /*db*/  0x7F , // ; 
/*.data:0042B199*/                 /*db*/ 0x0A8 , // ; ¿
/*.data:0042B19A*/                 /*db*/  0x2A , // ; *
/*.data:0042B19B*/                 /*db*/  0x3F , // ; ?
/*.data:0042B19C*/                 /*db*/  0x7D , // ; }
/*.data:0042B19D*/                 /*db*/ 0x0AC , // ; ¼
/*.data:0042B19E*/                 /*db*/ 0x0A1 , // ; í
/*.data:0042B19F*/                 /*db*/ 0x0E4 , // ; S
/*.data:0042B1A0*/                 /*db*/ 0x0BC , // ; +
/*.data:0042B1A1*/                 /*db*/  0x64 , // ; d
/*.data:0042B1A2*/                 /*db*/  0x7C , // ; |
/*.data:0042B1A3*/                 /*db*/  0x46 , // ; F
/*.data:0042B1A4*/                 /*db*/ 0x0D0 , // ; -
/*.data:0042B1A5*/                 /*db*/ 0x0DD , // ; ¦
/*.data:0042B1A6*/                 /*db*/  0x55 , // ; U
/*.data:0042B1A7*/                 /*db*/  0x3E , // ; >
/*.data:0042B1A8*/                 /*db*/  0x63 , // ; c
/*.data:0042B1A9*/                 /*db*/  0x7B , // ; {
/*.data:0042B1AA*/                 /*db*/    0x6 , // ;  
/*.data:0042B1AB*/                 /*db*/ 0x0CC , // ; ¦
/*.data:0042B1AC*/                 /*db*/  0x23 , // ; #
/*.data:0042B1AD*/                 /*db*/  0x54 , // ; T
/*.data:0042B1AE*/                 /*db*/  0x77 , // ; w
/*.data:0042B1AF*/                 /*db*/  0x83 , // ; â
/*.data:0042B1B0*/                 /*db*/ 0x0FF , // ;  
/*.data:0042B1B1*/                 /*db*/  0x91 , // ; æ
/*.data:0042B1B2*/                 /*db*/  0x81 , // ; ü
/*.data:0042B1B3*/                 /*db*/  0x3D , // ; =
/*.data:0042B1B4*/                 /*db*/  0x91 , // ; æ
/*.data:0042B1B5*/                 /*db*/ 0x0FA , // ; ·
/*.data:0042B1B6*/                 /*db*/  0x3A , // ; :
/*.data:0042B1B7*/                 /*db*/  0x19 , // ;  
/*.data:0042B1B8*/                 /*db*/  0x7A , // ; z
/*.data:0042B1B9*/                 /*db*/  0x63 , // ; c
/*.data:0042B1BA*/                 /*db*/  0x25 , // ; %
/*.data:0042B1BB*/                 /*db*/  0x43 , // ; C
/*.data:0042B1BC*/                 /*db*/  0x31 , // ; 1
/*.data:0042B1BD*/                 /*db*/ 0x0C0 , // ; +
/*.data:0042B1BE*/                 /*db*/ 0x0AC , // ; ¼
/*.data:0042B1BF*/                 /*db*/  0x3C , // ; <
/*.data:0042B1C0*/                 /*db*/  0x21 , // ; !
/*.data:0042B1C1*/                 /*db*/  0x89 , // ; ë
/*.data:0042B1C2*/                 /*db*/ 0x0D1 , // ; -
/*.data:0042B1C3*/                 /*db*/  0x38 , // ; 8
/*.data:0042B1C4*/                 /*db*/  0x82 , // ; é
/*.data:0042B1C5*/                 /*db*/  0x47 , // ; G
/*.data:0042B1C6*/                 /*db*/  0x97 , // ; ù
/*.data:0042B1C7*/                 /*db*/ 0x0B8 , // ; +
/*.data:0042B1C8*/                 /*db*/    0x0 , // ;  
/*.data:0042B1C9*/                 /*db*/ 0x0FD , // ; ²
/*.data:0042B1CA*/                 /*db*/ 0x0D7 , // ; +
/*.data:0042B1CB*/                 /*db*/  0x3B , // ; ;
/*.data:0042B1CC*/                 /*db*/ 0x0DC , // ; _
/*.data:0042B1CD*/                 /*db*/  0x88 , // ; ê
/*.data:0042B1CE*/                 /*db*/  0x58 , // ; X
/*.data:0042B1CF*/                 /*db*/    0x8 , // ;  
/*.data:0042B1D0*/                 /*db*/  0x1B , // ;  
/*.data:0042B1D1*/                 /*db*/ 0x0B1 , // ; ¦
/*.data:0042B1D2*/                 /*db*/ 0x0E8 , // ; F
/*.data:0042B1D3*/                 /*db*/ 0x0E3 , // ; p
/*.data:0042B1D4*/                 /*db*/  0x86 , // ; å
/*.data:0042B1D5*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B1D6*/                 /*db*/    0x3 , // ;  
/*.data:0042B1D7*/                 /*db*/  0x3B , // ; ;
/*.data:0042B1D8*/                 /*db*/ 0x0C6 , // ; ¦
/*.data:0042B1D9*/                 /*db*/  0x84 , // ; ä
/*.data:0042B1DA*/                 /*db*/  0x45 , // ; E
/*.data:0042B1DB*/                 /*db*/  0x42 , // ; B
/*.data:0042B1DC*/                 /*db*/    0x7 , // ;  
/*.data:0042B1DD*/                 /*db*/ 0x0B6 , // ; ¦
/*.data:0042B1DE*/                 /*db*/  0x99 , // ; Ö
/*.data:0042B1DF*/                 /*db*/  0x75 , // ; u
/*.data:0042B1E0*/                 /*db*/  0x37 , // ; 7
/*.data:0042B1E1*/                 /*db*/ 0x0DB , // ; ¦
/*.data:0042B1E2*/                 /*db*/  0x2E , // ; .
/*.data:0042B1E3*/                 /*db*/  0x3A , // ; :
/*.data:0042B1E4*/                 /*db*/  0x33 , // ; 3
/*.data:0042B1E5*/                 /*db*/  0x71 , // ; q
/*.data:0042B1E6*/                 /*db*/  0x1C , // ;  
/*.data:0042B1E7*/                 /*db*/ 0x0D2 , // ; -
/*.data:0042B1E8*/                 /*db*/  0x23 , // ; #
/*.data:0042B1E9*/                 /*db*/ 0x0DB , // ; ¦
/*.data:0042B1EA*/                 /*db*/  0x32 , // ; 2
/*.data:0042B1EB*/                 /*db*/ 0x0EE , // ; e
/*.data:0042B1EC*/                 /*db*/  0x49 , // ; I
/*.data:0042B1ED*/                 /*db*/  0x90 , // ; É
/*.data:0042B1EE*/                 /*db*/  0x5A , // ; Z
/*.data:0042B1EF*/                 /*db*/  0x39 , // ; 9
/*.data:0042B1F0*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B1F1*/                 /*db*/  0x87 , // ; ç
/*.data:0042B1F2*/                 /*db*/ 0x0BE , // ; +
/*.data:0042B1F3*/                 /*db*/ 0x0C0 , // ; +
/*.data:0042B1F4*/                 /*db*/  0x57 , // ; W
/*.data:0042B1F5*/                 /*db*/ 0x0DA , // ; +
/*.data:0042B1F6*/                 /*db*/ 0x0A5 , // ; Ñ
/*.data:0042B1F7*/                 /*db*/  0x82 , // ; é
/*.data:0042B1F8*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B1F9*/                 /*db*/ 0x0A2 , // ; ó
/*.data:0042B1FA*/                 /*db*/ 0x0B5 , // ; ¦
/*.data:0042B1FB*/                 /*db*/  0x32 , // ; 2
/*.data:0042B1FC*/                 /*db*/ 0x0E2 , // ; G
/*.data:0042B1FD*/                 /*db*/  0x68 , // ; h
/*.data:0042B1FE*/                 /*db*/ 0x0B2 , // ; ¦
/*.data:0042B1FF*/                 /*db*/  0x11 , // ;  
/*.data:0042B200*/                 /*db*/ 0x0A7 , // ; º
/*.data:0042B201*/                 /*db*/  0x52 , // ; R
/*.data:0042B202*/                 /*db*/  0x9F , // ; ƒ
/*.data:0042B203*/                 /*db*/  0x44 , // ; D
/*.data:0042B204*/                 /*db*/  0x59 , // ; Y
/*.data:0042B205*/                 /*db*/ 0x0B7 , // ; +
/*.data:0042B206*/                 /*db*/  0x10 , // ;  
/*.data:0042B207*/                 /*db*/  0x2C , // ; ,
/*.data:0042B208*/                 /*db*/  0x25 , // ; %
/*.data:0042B209*/                 /*db*/  0x49 , // ; I
/*.data:0042B20A*/                 /*db*/ 0x0E4 , // ; S
/*.data:0042B20B*/                 /*db*/  0x2D , // ; -
/*.data:0042B20C*/                 /*db*/  0x36 , // ; 6
/*.data:0042B20D*/                 /*db*/  0x34 , // ; 4
/*.data:0042B20E*/                 /*db*/  0x4F , // ; O
/*.data:0042B20F*/                 /*db*/  0x53 , // ; S
/*.data:0042B210*/                 /*db*/ 0x0AE , // ; «
/*.data:0042B211*/                 /*db*/ 0x0CE , // ; +
/*.data:0042B212*/                 /*db*/  0x6B , // ; k
/*.data:0042B213*/                 /*db*/  0x25 , // ; %
/*.data:0042B214*/                 /*db*/  0x8F , // ; Å
/*.data:0042B215*/                 /*db*/  0x59 , // ; Y
/*.data:0042B216*/                 /*db*/    0x4 , // ;  
/*.data:0042B217*/                 /*db*/ 0x0A4 , // ; ñ
/*.data:0042B218*/                 /*db*/ 0x0C0 , // ; +
/*.data:0042B219*/                 /*db*/ 0x0DE , // ; ¦
/*.data:0042B21A*/                 /*db*/ 0x0C2 , // ; -
/*.data:0042B21B*/                 /*db*/  0x7D , // ; }
/*.data:0042B21C*/                 /*db*/ 0x0FB , // ; v
/*.data:0042B21D*/                 /*db*/ 0x0E8 , // ; F
/*.data:0042B21E*/                 /*db*/ 0x0C6 , // ; ¦
/*.data:0042B21F*/                 /*db*/  0x1E , // ;  
/*.data:0042B220*/                 /*db*/  0x9E , // ; P
/*.data:0042B221*/                 /*db*/ 0x0E7 , // ; t
/*.data:0042B222*/                 /*db*/  0x88 , // ; ê
/*.data:0042B223*/                 /*db*/  0x5A , // ; Z
/*.data:0042B224*/                 /*db*/  0x57 , // ; W
/*.data:0042B225*/                 /*db*/  0x91 , // ; æ
/*.data:0042B226*/                 /*db*/  0x3C , // ; <
/*.data:0042B227*/                 /*db*/ 0x0BF , // ; +
/*.data:0042B228*/                 /*db*/  0x50 , // ; P
/*.data:0042B229*/                 /*db*/  0x83 , // ; â
/*.data:0042B22A*/                 /*db*/  0x22 , // ; "
/*.data:0042B22B*/                 /*db*/  0x18 , // ;  
/*.data:0042B22C*/                 /*db*/  0x4E , // ; N
/*.data:0042B22D*/                 /*db*/  0x4B , // ; K
/*.data:0042B22E*/                 /*db*/  0x65 , // ; e
/*.data:0042B22F*/                 /*db*/  0x62 , // ; b
/*.data:0042B230*/                 /*db*/ 0x0FD , // ; ²
/*.data:0042B231*/                 /*db*/  0x83 , // ; â
/*.data:0042B232*/                 /*db*/  0x8F , // ; Å
/*.data:0042B233*/                 /*db*/ 0x0AF , // ; »
/*.data:0042B234*/                 /*db*/    0x6 , // ;  
/*.data:0042B235*/                 /*db*/  0x94 , // ; ö
/*.data:0042B236*/                 /*db*/  0x7D , // ; }
/*.data:0042B237*/                 /*db*/  0x11 , // ;  
/*.data:0042B238*/                 /*db*/ 0x0E4 , // ; S
/*.data:0042B239*/                 /*db*/  0x2D , // ; -
/*.data:0042B23A*/                 /*db*/ 0x0DE , // ; ¦
/*.data:0042B23B*/                 /*db*/  0x9F , // ; ƒ
/*.data:0042B23C*/                 /*db*/ 0x0CE , // ; +
/*.data:0042B23D*/                 /*db*/ 0x0D2 , // ; -
/*.data:0042B23E*/                 /*db*/ 0x0C8 , // ; +
/*.data:0042B23F*/                 /*db*/    0x4 , // ;  
/*.data:0042B240*/                 /*db*/ 0x0DD , // ; ¦
/*.data:0042B241*/                 /*db*/ 0x0A6 , // ; ª
/*.data:0042B242*/                 /*db*/ 0x0D8 , // ; +
/*.data:0042B243*/                 /*db*/  0x0A , // ;  
/*.data:0042B244*/                 /*db*/    0x0 , // ;  
/*.data:0042B245*/                 /*db*/    0x0 , // ;  
/*.data:0042B246*/                 /*db*/    0x0 , // ;  
/*.data:0042B247*/                 /*db*/    0x0  // ;  
};

static VOID __declspec( naked ) ___shr_12( VOID )
{
	__asm
	{
		/*.text:0040D350*/ // ___shr_12       proc near               ; CODE XREF: ___ld12mul+316.p
		/*.text:0040D350*/                                         ; _$I10_OUTPUT+382.p
		/*.text:0040D350*/ 
		/*.text:0040D350*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D350*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D350*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D350*/ 
		/*.text:0040D350*/                 push    ebp
		/*.text:0040D351*/                 mov     ebp, esp
		/*.text:0040D353*/                 sub     esp, 8
		/*.text:0040D356*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D359*/                 mov     ecx, [eax+8]
		/*.text:0040D35C*/                 and     ecx, 1
		/*.text:0040D35F*/                 neg     ecx
		/*.text:0040D361*/                 sbb     ecx, ecx
		/*.text:0040D363*/                 and     ecx, 80000000h
		/*.text:0040D369*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:0040D36C*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D36F*/                 mov     eax, [edx+4]
		/*.text:0040D372*/                 and     eax, 1
		/*.text:0040D375*/                 neg     eax
		/*.text:0040D377*/                 sbb     eax, eax
		/*.text:0040D379*/                 and     eax, 80000000h
		/*.text:0040D37E*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0040D381*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D384*/                 mov     edx, [ecx+8]
		/*.text:0040D387*/                 shr     edx, 1
		/*.text:0040D389*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D38C*/                 mov     [eax+8], edx
		/*.text:0040D38F*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D392*/                 mov     edx, [ecx+4]
		/*.text:0040D395*/                 shr     edx, 1
		/*.text:0040D397*/                 or      edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D39A*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D39D*/                 mov     [eax+4], edx
		/*.text:0040D3A0*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D3A3*/                 mov     edx, [ecx]
		/*.text:0040D3A5*/                 shr     edx, 1
		/*.text:0040D3A7*/                 or      edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D3AA*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D3AD*/                 mov     [eax], edx
		/*.text:0040D3AF*/                 mov     esp, ebp
		/*.text:0040D3B1*/                 pop     ebp
		/*.text:0040D3B2*/                 retn
		/*.text:0040D3B2*/ // ___shr_12       endp
	}
}
#undef var_8
#undef var_4
#undef arg_0

static VOID __declspec( naked ) ___shl_12( VOID )
{
	__asm
	{
		/*.text:0040D2F0*/ // ___shl_12       proc near               ; CODE XREF: ___mtold12+5B.p
		/*.text:0040D2F0*/                                         ; ___mtold12+67.p ...
		/*.text:0040D2F0*/ 
		/*.text:0040D2F0*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D2F0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D2F0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D2F0*/ 
		/*.text:0040D2F0*/                 push    ebp
		/*.text:0040D2F1*/                 mov     ebp, esp
		/*.text:0040D2F3*/                 sub     esp, 8
		/*.text:0040D2F6*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D2F9*/                 mov     ecx, [eax]
		/*.text:0040D2FB*/                 and     ecx, 80000000h
		/*.text:0040D301*/                 neg     ecx
		/*.text:0040D303*/                 sbb     ecx, ecx
		/*.text:0040D305*/                 neg     ecx
		/*.text:0040D307*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0040D30A*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D30D*/                 mov     eax, [edx+4]
		/*.text:0040D310*/                 and     eax, 80000000h
		/*.text:0040D315*/                 neg     eax
		/*.text:0040D317*/                 sbb     eax, eax
		/*.text:0040D319*/                 neg     eax
		/*.text:0040D31B*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0040D31E*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D321*/                 mov     edx, [ecx]
		/*.text:0040D323*/                 shl     edx, 1
		/*.text:0040D325*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D328*/                 mov     [eax], edx
		/*.text:0040D32A*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D32D*/                 mov     edx, [ecx+4]
		/*.text:0040D330*/                 shl     edx, 1
		/*.text:0040D332*/                 or      edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D335*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D338*/                 mov     [eax+4], edx
		/*.text:0040D33B*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D33E*/                 mov     edx, [ecx+8]
		/*.text:0040D341*/                 shl     edx, 1
		/*.text:0040D343*/                 or      edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D346*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D349*/                 mov     [eax+8], edx
		/*.text:0040D34C*/                 mov     esp, ebp
		/*.text:0040D34E*/                 pop     ebp
		/*.text:0040D34F*/                 retn
		/*.text:0040D34F*/ // ___shl_12       endp
	}
}
#undef var_8
#undef var_4
#undef arg_0

static VOID __declspec( naked ) ___addl( VOID )
{
	__asm
	{
		/*.text:0040D200*/ // ___addl         proc near               ; CODE XREF: __IncMan+53.p
		/*.text:0040D200*/                                         ; __IncMan+94.p ...
		/*.text:0040D200*/ 
		/*.text:0040D200*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D200*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D200*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D200*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:0040D200*/ #define arg_8            dword ptr [ebp+0x10]
		/*.text:0040D200*/ 
		/*.text:0040D200*/                 push    ebp
		/*.text:0040D201*/                 mov     ebp, esp
		/*.text:0040D203*/                 sub     esp, 8
		/*.text:0040D206*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:0040D20D*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D210*/                 add     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D213*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0040D216*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D219*/                 cmp     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D21C*/                 jb      short loc_40D226
		/*.text:0040D21E*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D221*/                 cmp     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D224*/                 jnb     short loc_40D22F
		/*.text:0040D226*/ 
		/*.text:0040D226*/ loc_40D226:                             ; CODE XREF: ___addl+1C.j
		/*.text:0040D226*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D229*/                 add     eax, 1
		/*.text:0040D22C*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0040D22F*/ 
		/*.text:0040D22F*/ loc_40D22F:                             ; CODE XREF: ___addl+24.j
		/*.text:0040D22F*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D232*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D235*/                 mov     [ecx], edx
		/*.text:0040D237*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D23A*/                 mov     esp, ebp
		/*.text:0040D23C*/                 pop     ebp
		/*.text:0040D23D*/                 retn
		/*.text:0040D23D*/ // ___addl         endp
	}
}
#undef var_8
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8

static VOID __declspec( naked ) ___ld12mul( VOID )
{
	__asm
	{
		/*.text:0040D500*/ // ___ld12mul      proc near               ; CODE XREF: ___multtenpow12+B5.p
		/*.text:0040D500*/                                         ; _$I10_OUTPUT+2B5.p
		/*.text:0040D500*/ 
		/*.text:0040D500*/ #define var_48           dword ptr [ebp-0x48]
		/*.text:0040D500*/ #define var_44           dword ptr [ebp-0x44]
		/*.text:0040D500*/ #define var_40           dword ptr [ebp-0x40]
		/*.text:0040D500*/ #define var_3C           dword ptr [ebp-0x3C]
		/*.text:0040D500*/ #define var_38           dword ptr [ebp-0x38]
		/*.text:0040D500*/ #define var_34           dword ptr [ebp-0x34]
		/*.text:0040D500*/ #define var_30           /*dword ptr*/ [ebp-0x30]
		/*.text:0040D500*/ #define var_2C           /*dword ptr*/ [ebp-0x2C]
		/*.text:0040D500*/ #define var_28           dword ptr [ebp-0x28]
		/*.text:0040D500*/ #define var_24           dword ptr [ebp-0x24]
		/*.text:0040D500*/ #define var_20           /*dword ptr*/ [ebp-0x20]
		/*.text:0040D500*/ #define var_1C           dword ptr [ebp-0x1C]
		/*.text:0040D500*/ #define var_18           /*dword ptr*/ [ebp-0x18]
		/*.text:0040D500*/ #define var_14           /*dword ptr [*/ ebp-0x14 /*]*/
		/*.text:0040D500*/ #define var_10           /*dword ptr [*/ ebp-0x10 /*]*/
		/*.text:0040D500*/ #define var_C            /*dword ptr [*/ ebp-0x0C /*]*/
		/*.text:0040D500*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D500*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D500*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D500*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:0040D500*/ 
		/*.text:0040D500*/                 push    ebp
		/*.text:0040D501*/                 mov     ebp, esp
		/*.text:0040D503*/                 sub     esp, 48h
		/*.text:0040D506*/                 mov     word ptr /*[ebp+*/ var_2C /*]*/, 0
		/*.text:0040D50C*/                 mov     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0040D513*/                 mov     [ var_14 ], 0
		/*.text:0040D51A*/                 mov     [ var_10 ], 0
		/*.text:0040D521*/                 mov     [ var_C ], 0
		/*.text:0040D528*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D52B*/                 mov     cx, [eax+0Ah]
		/*.text:0040D52F*/                 mov     word ptr /*[ebp+*/ var_18 /*]*/, cx
		/*.text:0040D533*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D536*/                 mov     ax, [edx+0Ah]
		/*.text:0040D53A*/                 mov     word ptr /*[ebp+*/ var_20 /*]*/, ax
		/*.text:0040D53E*/                 mov     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040D541*/                 and     ecx, 0FFFFh
		/*.text:0040D547*/                 mov     edx, /*[ebp+*/ var_20 /*]*/
		/*.text:0040D54A*/                 and     edx, 0FFFFh
		/*.text:0040D550*/                 xor     ecx, edx
		/*.text:0040D552*/                 and     ecx, 8000h
		/*.text:0040D558*/                 mov     word ptr /*[ebp+*/ var_2C /*]*/, cx
		/*.text:0040D55C*/                 mov     ax, word ptr /*[ebp+*/ var_18 /*]*/
		/*.text:0040D560*/                 and     ax, 7FFFh
		/*.text:0040D564*/                 mov     word ptr /*[ebp+*/ var_18 /*]*/, ax
		/*.text:0040D568*/                 mov     cx, word ptr /*[ebp+*/ var_20 /*]*/
		/*.text:0040D56C*/                 and     cx, 7FFFh
		/*.text:0040D571*/                 mov     word ptr /*[ebp+*/ var_20 /*]*/, cx
		/*.text:0040D575*/                 mov     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040D578*/                 and     edx, 0FFFFh
		/*.text:0040D57E*/                 mov     eax, /*[ebp+*/ var_20 /*]*/
		/*.text:0040D581*/                 and     eax, 0FFFFh
		/*.text:0040D586*/                 add     edx, eax
		/*.text:0040D588*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, dx
		/*.text:0040D58C*/                 mov     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040D58F*/                 and     ecx, 0FFFFh
		/*.text:0040D595*/                 cmp     ecx, 7FFFh
		/*.text:0040D59B*/                 jge     short loc_40D5BD
		/*.text:0040D59D*/                 mov     edx, /*[ebp+*/ var_20 /*]*/
		/*.text:0040D5A0*/                 and     edx, 0FFFFh
		/*.text:0040D5A6*/                 cmp     edx, 7FFFh
		/*.text:0040D5AC*/                 jge     short loc_40D5BD
		/*.text:0040D5AE*/                 mov     eax, /*[ebp+*/ var_30 /*]*/
		/*.text:0040D5B1*/                 and     eax, 0FFFFh
		/*.text:0040D5B6*/                 cmp     eax, 0BFFDh
		/*.text:0040D5BB*/                 jle     short loc_40D5F4
		/*.text:0040D5BD*/ 
		/*.text:0040D5BD*/ loc_40D5BD:                             ; CODE XREF: ___ld12mul+9B.j
		/*.text:0040D5BD*/                                         ; ___ld12mul+AC.j
		/*.text:0040D5BD*/                 mov     ecx, /*[ebp+*/ var_2C /*]*/
		/*.text:0040D5C0*/                 and     ecx, 0FFFFh
		/*.text:0040D5C6*/                 neg     ecx
		/*.text:0040D5C8*/                 sbb     ecx, ecx
		/*.text:0040D5CA*/                 and     ecx, 80000000h
		/*.text:0040D5D0*/                 add     ecx, 7FFF8000h
		/*.text:0040D5D6*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D5D9*/                 mov     [edx+8], ecx
		/*.text:0040D5DC*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D5DF*/                 mov     dword ptr [eax+4], 0
		/*.text:0040D5E6*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D5E9*/                 mov     dword ptr [ecx], 0
		/*.text:0040D5EF*/                 jmp     loc_40D939
		/*.text:0040D5F4*/ ; ---------------------------------------------------------------------------
		/*.text:0040D5F4*/ 
		/*.text:0040D5F4*/ loc_40D5F4:                             ; CODE XREF: ___ld12mul+BB.j
		/*.text:0040D5F4*/                 mov     edx, /*[ebp+*/ var_30 /*]*/
		/*.text:0040D5F7*/                 and     edx, 0FFFFh
		/*.text:0040D5FD*/                 cmp     edx, 3FBFh
		/*.text:0040D603*/                 jg      short loc_40D627
		/*.text:0040D605*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D608*/                 mov     dword ptr [eax+8], 0
		/*.text:0040D60F*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D612*/                 mov     dword ptr [ecx+4], 0
		/*.text:0040D619*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D61C*/                 mov     dword ptr [edx], 0
		/*.text:0040D622*/                 jmp     loc_40D939
		/*.text:0040D627*/ ; ---------------------------------------------------------------------------
		/*.text:0040D627*/ 
		/*.text:0040D627*/ loc_40D627:                             ; CODE XREF: ___ld12mul+103.j
		/*.text:0040D627*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:0040D62A*/                 and     eax, 0FFFFh
		/*.text:0040D62F*/                 test    eax, eax
		/*.text:0040D631*/                 jnz     short loc_40D66D
		/*.text:0040D633*/                 mov     cx, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D637*/                 add     cx, 1
		/*.text:0040D63B*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, cx
		/*.text:0040D63F*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D642*/                 mov     eax, [edx+8]
		/*.text:0040D645*/                 and     eax, 7FFFFFFFh
		/*.text:0040D64A*/                 test    eax, eax
		/*.text:0040D64C*/                 jnz     short loc_40D66D
		/*.text:0040D64E*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D651*/                 cmp     dword ptr [ecx+4], 0
		/*.text:0040D655*/                 jnz     short loc_40D66D
		/*.text:0040D657*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D65A*/                 cmp     dword ptr [edx], 0
		/*.text:0040D65D*/                 jnz     short loc_40D66D
		/*.text:0040D65F*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D662*/                 mov     word ptr [eax+0Ah], 0
		/*.text:0040D668*/                 jmp     loc_40D939
		/*.text:0040D66D*/ ; ---------------------------------------------------------------------------
		/*.text:0040D66D*/ 
		/*.text:0040D66D*/ loc_40D66D:                             ; CODE XREF: ___ld12mul+131.j
		/*.text:0040D66D*/                                         ; ___ld12mul+14C.j ...
		/*.text:0040D66D*/                 mov     ecx, /*[ebp+*/ var_20 /*]*/
		/*.text:0040D670*/                 and     ecx, 0FFFFh
		/*.text:0040D676*/                 test    ecx, ecx
		/*.text:0040D678*/                 jnz     short loc_40D6C9
		/*.text:0040D67A*/                 mov     dx, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D67E*/                 add     dx, 1
		/*.text:0040D682*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, dx
		/*.text:0040D686*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D689*/                 mov     ecx, [eax+8]
		/*.text:0040D68C*/                 and     ecx, 7FFFFFFFh
		/*.text:0040D692*/                 test    ecx, ecx
		/*.text:0040D694*/                 jnz     short loc_40D6C9
		/*.text:0040D696*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D699*/                 cmp     dword ptr [edx+4], 0
		/*.text:0040D69D*/                 jnz     short loc_40D6C9
		/*.text:0040D69F*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D6A2*/                 cmp     dword ptr [eax], 0
		/*.text:0040D6A5*/                 jnz     short loc_40D6C9
		/*.text:0040D6A7*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D6AA*/                 mov     dword ptr [ecx+8], 0
		/*.text:0040D6B1*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D6B4*/                 mov     dword ptr [edx+4], 0
		/*.text:0040D6BB*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D6BE*/                 mov     dword ptr [eax], 0
		/*.text:0040D6C4*/                 jmp     loc_40D939
		/*.text:0040D6C9*/ ; ---------------------------------------------------------------------------
		/*.text:0040D6C9*/ 
		/*.text:0040D6C9*/ loc_40D6C9:                             ; CODE XREF: ___ld12mul+178.j
		/*.text:0040D6C9*/                                         ; ___ld12mul+194.j ...
		/*.text:0040D6C9*/                 mov     /*[ebp+*/ var_28 /*]*/, 0
		/*.text:0040D6D0*/                 mov     /*[ebp+*/ var_1C /*]*/, 0
		/*.text:0040D6D7*/                 jmp     short loc_40D6E2
		/*.text:0040D6D9*/ ; ---------------------------------------------------------------------------
		/*.text:0040D6D9*/ 
		/*.text:0040D6D9*/ loc_40D6D9:                             ; CODE XREF: ___ld12mul+29C.j
		/*.text:0040D6D9*/                 mov     ecx, /*[ebp+*/ var_1C /*]*/
		/*.text:0040D6DC*/                 add     ecx, 1
		/*.text:0040D6DF*/                 mov     /*[ebp+*/ var_1C /*]*/, ecx
		/*.text:0040D6E2*/ 
		/*.text:0040D6E2*/ loc_40D6E2:                             ; CODE XREF: ___ld12mul+1D7.j
		/*.text:0040D6E2*/                 cmp     /*[ebp+*/ var_1C /*]*/, 5
		/*.text:0040D6E6*/                 jge     loc_40D7A1
		/*.text:0040D6EC*/                 mov     edx, /*[ebp+*/ var_1C /*]*/
		/*.text:0040D6EF*/                 shl     edx, 1
		/*.text:0040D6F1*/                 mov     /*[ebp+*/ var_24 /*]*/, edx
		/*.text:0040D6F4*/                 mov     /*[ebp+*/ var_8 /*]*/, 8
		/*.text:0040D6FB*/                 mov     eax, 5
		/*.text:0040D700*/                 sub     eax, /*[ebp+*/ var_1C /*]*/
		/*.text:0040D703*/                 mov     /*[ebp+*/ var_34 /*]*/, eax
		/*.text:0040D706*/                 jmp     short loc_40D711
		/*.text:0040D708*/ ; ---------------------------------------------------------------------------
		/*.text:0040D708*/ 
		/*.text:0040D708*/ loc_40D708:                             ; CODE XREF: ___ld12mul+28E.j
		/*.text:0040D708*/                 mov     ecx, /*[ebp+*/ var_34 /*]*/
		/*.text:0040D70B*/                 sub     ecx, 1
		/*.text:0040D70E*/                 mov     /*[ebp+*/ var_34 /*]*/, ecx
		/*.text:0040D711*/ 
		/*.text:0040D711*/ loc_40D711:                             ; CODE XREF: ___ld12mul+206.j
		/*.text:0040D711*/                 cmp     /*[ebp+*/ var_34 /*]*/, 0
		/*.text:0040D715*/                 jle     short loc_40D793
		/*.text:0040D717*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D71A*/                 add     edx, /*[ebp+*/ var_24 /*]*/
		/*.text:0040D71D*/                 mov     /*[ebp+*/ var_38 /*]*/, edx
		/*.text:0040D720*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D723*/                 add     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D726*/                 mov     /*[ebp+*/ var_3C /*]*/, eax
		/*.text:0040D729*/                 mov     ecx, /*[ebp+*/ var_28 /*]*/
		/*.text:0040D72C*/                 lea     edx, [/*ebp+*/ecx + var_14 ]
		/*.text:0040D730*/                 mov     /*[ebp+*/ var_40 /*]*/, edx
		/*.text:0040D733*/                 mov     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:0040D736*/                 xor     ecx, ecx
		/*.text:0040D738*/                 mov     cx, [eax]
		/*.text:0040D73B*/                 mov     edx, /*[ebp+*/ var_3C /*]*/
		/*.text:0040D73E*/                 xor     eax, eax
		/*.text:0040D740*/                 mov     ax, [edx]
		/*.text:0040D743*/                 imul    ecx, eax
		/*.text:0040D746*/                 mov     /*[ebp+*/ var_44 /*]*/, ecx
		/*.text:0040D749*/                 mov     ecx, /*[ebp+*/ var_40 /*]*/
		/*.text:0040D74C*/                 push    ecx
		/*.text:0040D74D*/                 mov     edx, /*[ebp+*/ var_44 /*]*/
		/*.text:0040D750*/                 push    edx
		/*.text:0040D751*/                 mov     eax, /*[ebp+*/ var_40 /*]*/
		/*.text:0040D754*/                 mov     ecx, [eax]
		/*.text:0040D756*/                 push    ecx
		/*.text:0040D757*/                 call    ___addl
		/*.text:0040D75C*/                 add     esp, 0Ch
		/*.text:0040D75F*/                 mov     /*[ebp+*/ var_48 /*]*/, eax
		/*.text:0040D762*/                 cmp     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:0040D766*/                 jz      short loc_40D77C
		/*.text:0040D768*/                 mov     edx, /*[ebp+*/ var_28 /*]*/
		/*.text:0040D76B*/                 mov     ax, word ptr [edx+ var_10 ]
		/*.text:0040D770*/                 add     ax, 1
		/*.text:0040D774*/                 mov     ecx, /*[ebp+*/ var_28 /*]*/
		/*.text:0040D777*/                 mov     word ptr [ecx+ var_10 ], ax
		/*.text:0040D77C*/ 
		/*.text:0040D77C*/ loc_40D77C:                             ; CODE XREF: ___ld12mul+266.j
		/*.text:0040D77C*/                 mov     edx, /*[ebp+*/ var_24 /*]*/
		/*.text:0040D77F*/                 add     edx, 2
		/*.text:0040D782*/                 mov     /*[ebp+*/ var_24 /*]*/, edx
		/*.text:0040D785*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D788*/                 sub     eax, 2
		/*.text:0040D78B*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0040D78E*/                 jmp     loc_40D708
		/*.text:0040D793*/ ; ---------------------------------------------------------------------------
		/*.text:0040D793*/ 
		/*.text:0040D793*/ loc_40D793:                             ; CODE XREF: ___ld12mul+215.j
		/*.text:0040D793*/                 mov     ecx, /*[ebp+*/ var_28 /*]*/
		/*.text:0040D796*/                 add     ecx, 2
		/*.text:0040D799*/                 mov     /*[ebp+*/ var_28 /*]*/, ecx
		/*.text:0040D79C*/                 jmp     loc_40D6D9
		/*.text:0040D7A1*/ ; ---------------------------------------------------------------------------
		/*.text:0040D7A1*/ 
		/*.text:0040D7A1*/ loc_40D7A1:                             ; CODE XREF: ___ld12mul+1E6.j
		/*.text:0040D7A1*/                 mov     dx, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D7A5*/                 sub     dx, 3FFEh
		/*.text:0040D7AA*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, dx
		/*.text:0040D7AE*/ 
		/*.text:0040D7AE*/ loc_40D7AE:                             ; CODE XREF: ___ld12mul+2DB.j
		/*.text:0040D7AE*/                 movsx   eax, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D7B2*/                 test    eax, eax
		/*.text:0040D7B4*/                 jle     short loc_40D7DD
		/*.text:0040D7B6*/                 mov     ecx, [ var_C ]
		/*.text:0040D7B9*/                 and     ecx, 80000000h
		/*.text:0040D7BF*/                 test    ecx, ecx
		/*.text:0040D7C1*/                 jnz     short loc_40D7DD
		/*.text:0040D7C3*/                 lea     edx, [ var_14 ]
		/*.text:0040D7C6*/                 push    edx
		/*.text:0040D7C7*/                 call    ___shl_12
		/*.text:0040D7CC*/                 add     esp, 4
		/*.text:0040D7CF*/                 mov     ax, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D7D3*/                 sub     ax, 1
		/*.text:0040D7D7*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, ax
		/*.text:0040D7DB*/                 jmp     short loc_40D7AE
		/*.text:0040D7DD*/ ; ---------------------------------------------------------------------------
		/*.text:0040D7DD*/ 
		/*.text:0040D7DD*/ loc_40D7DD:                             ; CODE XREF: ___ld12mul+2B4.j
		/*.text:0040D7DD*/                                         ; ___ld12mul+2C1.j
		/*.text:0040D7DD*/                 movsx   ecx, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D7E1*/                 test    ecx, ecx
		/*.text:0040D7E3*/                 jg      short loc_40D83D
		/*.text:0040D7E5*/                 mov     dx, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D7E9*/                 sub     dx, 1
		/*.text:0040D7ED*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, dx
		/*.text:0040D7F1*/ 
		/*.text:0040D7F1*/ loc_40D7F1:                             ; CODE XREF: ___ld12mul+32A.j
		/*.text:0040D7F1*/                 movsx   eax, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D7F5*/                 test    eax, eax
		/*.text:0040D7F7*/                 jge     short loc_40D82C
		/*.text:0040D7F9*/                 mov     ecx, [ var_14 ]
		/*.text:0040D7FC*/                 and     ecx, 0FFFFh
		/*.text:0040D802*/                 and     ecx, 1
		/*.text:0040D805*/                 test    ecx, ecx
		/*.text:0040D807*/                 jz      short loc_40D812
		/*.text:0040D809*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D80C*/                 add     edx, 1
		/*.text:0040D80F*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0040D812*/ 
		/*.text:0040D812*/ loc_40D812:                             ; CODE XREF: ___ld12mul+307.j
		/*.text:0040D812*/                 lea     eax, [ var_14 ]
		/*.text:0040D815*/                 push    eax
		/*.text:0040D816*/                 call    ___shr_12
		/*.text:0040D81B*/                 add     esp, 4
		/*.text:0040D81E*/                 mov     cx, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D822*/                 add     cx, 1
		/*.text:0040D826*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, cx
		/*.text:0040D82A*/                 jmp     short loc_40D7F1
		/*.text:0040D82C*/ ; ---------------------------------------------------------------------------
		/*.text:0040D82C*/ 
		/*.text:0040D82C*/ loc_40D82C:                             ; CODE XREF: ___ld12mul+2F7.j
		/*.text:0040D82C*/                 cmp     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0040D830*/                 jz      short loc_40D83D
		/*.text:0040D832*/                 mov     dx, word ptr [ var_14 ]
		/*.text:0040D836*/                 or      dl, 1
		/*.text:0040D839*/                 mov     word ptr [ var_14 ], dx
		/*.text:0040D83D*/ 
		/*.text:0040D83D*/ loc_40D83D:                             ; CODE XREF: ___ld12mul+2E3.j
		/*.text:0040D83D*/                                         ; ___ld12mul+330.j
		/*.text:0040D83D*/                 mov     eax, [ var_14 ]
		/*.text:0040D840*/                 and     eax, 0FFFFh
		/*.text:0040D845*/                 cmp     eax, 8000h
		/*.text:0040D84A*/                 jg      short loc_40D85D
		/*.text:0040D84C*/                 mov     ecx, [ var_14 ]
		/*.text:0040D84F*/                 and     ecx, 1FFFFh
		/*.text:0040D855*/                 cmp     ecx, 18000h
		/*.text:0040D85B*/                 jnz     short loc_40D8BE
		/*.text:0040D85D*/ 
		/*.text:0040D85D*/ loc_40D85D:                             ; CODE XREF: ___ld12mul+34A.j
		/*.text:0040D85D*/                 cmp     [ var_14 +2], 0FFFFFFFFh
		/*.text:0040D861*/                 jnz     short loc_40D8B5
		/*.text:0040D863*/                 mov     [ var_14 +2], 0
		/*.text:0040D86A*/                 cmp     [ var_10 +2], 0FFFFFFFFh
		/*.text:0040D86E*/                 jnz     short loc_40D8AA
		/*.text:0040D870*/                 mov     [ var_10 +2], 0
		/*.text:0040D877*/                 mov     edx, [ var_C +2]
		/*.text:0040D87A*/                 and     edx, 0FFFFh
		/*.text:0040D880*/                 cmp     edx, 0FFFFh
		/*.text:0040D886*/                 jnz     short loc_40D89C
		/*.text:0040D888*/                 mov     word ptr [ var_C +2], 8000h
		/*.text:0040D88E*/                 mov     ax, word ptr /*[ebp+*/ var_30 /*]*/
		/*.text:0040D892*/                 add     ax, 1
		/*.text:0040D896*/                 mov     word ptr /*[ebp+*/ var_30 /*]*/, ax
		/*.text:0040D89A*/                 jmp     short loc_40D8A8
		/*.text:0040D89C*/ ; ---------------------------------------------------------------------------
		/*.text:0040D89C*/ 
		/*.text:0040D89C*/ loc_40D89C:                             ; CODE XREF: ___ld12mul+386.j
		/*.text:0040D89C*/                 mov     cx, word ptr [ var_C +2]
		/*.text:0040D8A0*/                 add     cx, 1
		/*.text:0040D8A4*/                 mov     word ptr [ var_C +2], cx
		/*.text:0040D8A8*/ 
		/*.text:0040D8A8*/ loc_40D8A8:                             ; CODE XREF: ___ld12mul+39A.j
		/*.text:0040D8A8*/                 jmp     short loc_40D8B3
		/*.text:0040D8AA*/ ; ---------------------------------------------------------------------------
		/*.text:0040D8AA*/ 
		/*.text:0040D8AA*/ loc_40D8AA:                             ; CODE XREF: ___ld12mul+36E.j
		/*.text:0040D8AA*/                 mov     edx, [ var_10 +2]
		/*.text:0040D8AD*/                 add     edx, 1
		/*.text:0040D8B0*/                 mov     [ var_10 +2], edx
		/*.text:0040D8B3*/ 
		/*.text:0040D8B3*/ loc_40D8B3:                             ; CODE XREF: ___ld12mul+3A8.j
		/*.text:0040D8B3*/                 jmp     short loc_40D8BE
		/*.text:0040D8B5*/ ; ---------------------------------------------------------------------------
		/*.text:0040D8B5*/ 
		/*.text:0040D8B5*/ loc_40D8B5:                             ; CODE XREF: ___ld12mul+361.j
		/*.text:0040D8B5*/                 mov     eax, [ var_14 +2]
		/*.text:0040D8B8*/                 add     eax, 1
		/*.text:0040D8BB*/                 mov     [ var_14 +2], eax
		/*.text:0040D8BE*/ 
		/*.text:0040D8BE*/ loc_40D8BE:                             ; CODE XREF: ___ld12mul+35B.j
		/*.text:0040D8BE*/                                         ; ___ld12mul+3B3.j
		/*.text:0040D8BE*/                 mov     ecx, /*[ebp+*/ var_30 /*]*/
		/*.text:0040D8C1*/                 and     ecx, 0FFFFh
		/*.text:0040D8C7*/                 cmp     ecx, 7FFFh
		/*.text:0040D8CD*/                 jl      short loc_40D903
		/*.text:0040D8CF*/                 mov     edx, /*[ebp+*/ var_2C /*]*/
		/*.text:0040D8D2*/                 and     edx, 0FFFFh
		/*.text:0040D8D8*/                 neg     edx
		/*.text:0040D8DA*/                 sbb     edx, edx
		/*.text:0040D8DC*/                 and     edx, 80000000h
		/*.text:0040D8E2*/                 add     edx, 7FFF8000h
		/*.text:0040D8E8*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D8EB*/                 mov     [eax+8], edx
		/*.text:0040D8EE*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D8F1*/                 mov     dword ptr [ecx+4], 0
		/*.text:0040D8F8*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D8FB*/                 mov     dword ptr [edx], 0
		/*.text:0040D901*/                 jmp     short loc_40D939
		/*.text:0040D903*/ ; ---------------------------------------------------------------------------
		/*.text:0040D903*/ 
		/*.text:0040D903*/ loc_40D903:                             ; CODE XREF: ___ld12mul+3CD.j
		/*.text:0040D903*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D906*/                 mov     cx, word ptr [ var_14 +2]
		/*.text:0040D90A*/                 mov     [eax], cx
		/*.text:0040D90D*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D910*/                 mov     eax, [ var_10 ]
		/*.text:0040D913*/                 mov     [edx+2], eax
		/*.text:0040D916*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D919*/                 mov     edx, [ var_C ]
		/*.text:0040D91C*/                 mov     [ecx+6], edx
		/*.text:0040D91F*/                 mov     eax, /*[ebp+*/ var_30 /*]*/
		/*.text:0040D922*/                 and     eax, 0FFFFh
		/*.text:0040D927*/                 mov     ecx, /*[ebp+*/ var_2C /*]*/
		/*.text:0040D92A*/                 and     ecx, 0FFFFh
		/*.text:0040D930*/                 or      eax, ecx
		/*.text:0040D932*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D935*/                 mov     [edx+0Ah], ax
		/*.text:0040D939*/ 
		/*.text:0040D939*/ loc_40D939:                             ; CODE XREF: ___ld12mul+EF.j
		/*.text:0040D939*/                                         ; ___ld12mul+122.j ...
		/*.text:0040D939*/                 mov     esp, ebp
		/*.text:0040D93B*/                 pop     ebp
		/*.text:0040D93C*/                 retn
		/*.text:0040D93C*/ // ___ld12mul      endp
	}
}
#undef var_48
#undef var_44
#undef var_40
#undef var_3C
#undef var_38
#undef var_34
#undef var_30
#undef var_2C
#undef var_28
#undef var_24
#undef var_20
#undef var_1C
#undef var_18
#undef var_14
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 

static VOID __declspec( naked ) ___multtenpow12( VOID )
{
	__asm
	{
		/*.text:0040D940*/ // ___multtenpow12 proc near               ; CODE XREF: ___strgtold12+914.p
		/*.text:0040D940*/                                         ; _$I10_OUTPUT+288.p
		/*.text:0040D940*/ 
		/*.text:0040D940*/ #define var_18           dword ptr [ebp-0x18]
		/*.text:0040D940*/ #define var_14           /*dword ptr [*/ ebp-0x14 /*]*/
		/*.text:0040D940*/ #define var_10           dword ptr [ebp-0x10]
		/*.text:0040D940*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:0040D940*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D940*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D940*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D940*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:0040D940*/ #define arg_8            dword ptr [ebp+0x10]
		/*.text:0040D940*/ 
		/*.text:0040D940*/                 push    ebp
		/*.text:0040D941*/                 mov     ebp, esp
		/*.text:0040D943*/                 sub     esp, 18h
		/*.text:0040D946*/                 mov     eax, offset __pow10pos
		/*.text:0040D94B*/                 sub     eax, 60h
		/*.text:0040D94E*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0040D951*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0040D955*/                 jnz     short loc_40D95C
		/*.text:0040D957*/                 jmp     loc_40D9FF
		/*.text:0040D95C*/ ; ---------------------------------------------------------------------------
		/*.text:0040D95C*/ 
		/*.text:0040D95C*/ loc_40D95C:                             ; CODE XREF: ___multtenpow12+15.j
		/*.text:0040D95C*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0040D960*/                 jge     short loc_40D975
		/*.text:0040D962*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D965*/                 neg     ecx
		/*.text:0040D967*/                 mov     /*[ebp+*/ arg_4 /*]*/, ecx
//-----------------------------------------------------------------------------------------------------------------------
		/*.text:0040D96A*/                 // mov     edx, offset __pow10neg

											mov     edx, offset __pow10pos
											add		edx, 352
//-----------------------------------------------------------------------------------------------------------------------
		/*.text:0040D96F*/                 sub     edx, 60h
		/*.text:0040D972*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0040D975*/ 
		/*.text:0040D975*/ loc_40D975:                             ; CODE XREF: ___multtenpow12+20.j
		/*.text:0040D975*/                 cmp     /*[ebp+*/ arg_8 /*]*/, 0
		/*.text:0040D979*/                 jnz     short loc_40D983
		/*.text:0040D97B*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D97E*/                 mov     word ptr [eax], 0
		/*.text:0040D983*/ 
		/*.text:0040D983*/ loc_40D983:                             ; CODE XREF: ___multtenpow12+39.j
		/*.text:0040D983*/                                         ; ___multtenpow12+6A.j ...
		/*.text:0040D983*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0040D987*/                 jz      short loc_40D9FF
		/*.text:0040D989*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D98C*/                 add     ecx, 54h
		/*.text:0040D98F*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0040D992*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D995*/                 and     edx, 7
		/*.text:0040D998*/                 mov     /*[ebp+*/ var_18 /*]*/, edx
		/*.text:0040D99B*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D99E*/                 sar     eax, 3
		/*.text:0040D9A1*/                 mov     /*[ebp+*/ arg_4 /*]*/, eax
		/*.text:0040D9A4*/                 cmp     /*[ebp+*/ var_18 /*]*/, 0
		/*.text:0040D9A8*/                 jnz     short loc_40D9AC
		/*.text:0040D9AA*/                 jmp     short loc_40D983
		/*.text:0040D9AC*/ ; ---------------------------------------------------------------------------
		/*.text:0040D9AC*/ 
		/*.text:0040D9AC*/ loc_40D9AC:                             ; CODE XREF: ___multtenpow12+68.j
		/*.text:0040D9AC*/                 mov     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040D9AF*/                 imul    ecx, 0Ch
		/*.text:0040D9B2*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040D9B5*/                 add     edx, ecx
		/*.text:0040D9B7*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:0040D9BA*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D9BD*/                 xor     ecx, ecx
		/*.text:0040D9BF*/                 mov     cx, [eax]
		/*.text:0040D9C2*/                 cmp     ecx, 8000h
		/*.text:0040D9C8*/                 jl      short loc_40D9ED
		/*.text:0040D9CA*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D9CD*/                 mov     eax, [edx]
		/*.text:0040D9CF*/                 mov     [ var_14 ], eax
		/*.text:0040D9D2*/                 mov     ecx, [edx+4]
		/*.text:0040D9D5*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:0040D9D8*/                 mov     edx, [edx+8]
		/*.text:0040D9DB*/                 mov     /*[ebp+*/ var_C /*]*/, edx
		/*.text:0040D9DE*/                 mov     eax, [ var_14 +2]
		/*.text:0040D9E1*/                 sub     eax, 1
		/*.text:0040D9E4*/                 mov     [ var_14 +2], eax
		/*.text:0040D9E7*/                 lea     ecx, [ var_14 ]
		/*.text:0040D9EA*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:0040D9ED*/ 
		/*.text:0040D9ED*/ loc_40D9ED:                             ; CODE XREF: ___multtenpow12+88.j
		/*.text:0040D9ED*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0040D9F0*/                 push    edx
		/*.text:0040D9F1*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D9F4*/                 push    eax
		/*.text:0040D9F5*/                 call    ___ld12mul
		/*.text:0040D9FA*/                 add     esp, 8
		/*.text:0040D9FD*/                 jmp     short loc_40D983
		/*.text:0040D9FF*/ ; ---------------------------------------------------------------------------
		/*.text:0040D9FF*/ 
		/*.text:0040D9FF*/ loc_40D9FF:                             ; CODE XREF: ___multtenpow12+17.j
		/*.text:0040D9FF*/                                         ; ___multtenpow12+47.j
		/*.text:0040D9FF*/                 mov     esp, ebp
		/*.text:0040DA01*/                 pop     ebp
		/*.text:0040DA02*/                 retn
		/*.text:0040DA02*/ // ___multtenpow12 endp
	}
}
#undef var_18
#undef var_14
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 
#undef arg_8 

static VOID __declspec( naked ) ___add_12( VOID )
{
	__asm
	{
		/*.text:0040D240*/ // ___add_12       proc near               ; CODE XREF: ___mtold12+77.p
		/*.text:0040D240*/                                         ; ___mtold12+AA.p ...
		/*.text:0040D240*/ 
		/*.text:0040D240*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:0040D240*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D240*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D240*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D240*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:0040D240*/ 
		/*.text:0040D240*/                 push    ebp
		/*.text:0040D241*/                 mov     ebp, esp
		/*.text:0040D243*/                 sub     esp, 0Ch
		/*.text:0040D246*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D249*/                 push    eax
		/*.text:0040D24A*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D24D*/                 mov     edx, [ecx]
		/*.text:0040D24F*/                 push    edx
		/*.text:0040D250*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D253*/                 mov     ecx, [eax]
		/*.text:0040D255*/                 push    ecx
		/*.text:0040D256*/                 call    ___addl
		/*.text:0040D25B*/                 add     esp, 0Ch
		/*.text:0040D25E*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0040D261*/                 cmp     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0040D265*/                 jz      short loc_40D297
		/*.text:0040D267*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D26A*/                 add     edx, 4
		/*.text:0040D26D*/                 push    edx
		/*.text:0040D26E*/                 push    1
		/*.text:0040D270*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D273*/                 mov     ecx, [eax+4]
		/*.text:0040D276*/                 push    ecx
		/*.text:0040D277*/                 call    ___addl
		/*.text:0040D27C*/                 add     esp, 0Ch
		/*.text:0040D27F*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0040D282*/                 cmp     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:0040D286*/                 jz      short loc_40D297
		/*.text:0040D288*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D28B*/                 mov     eax, [edx+8]
		/*.text:0040D28E*/                 add     eax, 1
		/*.text:0040D291*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D294*/                 mov     [ecx+8], eax
		/*.text:0040D297*/ 
		/*.text:0040D297*/ loc_40D297:                             ; CODE XREF: ___add_12+25.j
		/*.text:0040D297*/                                         ; ___add_12+46.j
		/*.text:0040D297*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D29A*/                 add     edx, 4
		/*.text:0040D29D*/                 push    edx
		/*.text:0040D29E*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D2A1*/                 mov     ecx, [eax+4]
		/*.text:0040D2A4*/                 push    ecx
		/*.text:0040D2A5*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D2A8*/                 mov     eax, [edx+4]
		/*.text:0040D2AB*/                 push    eax
		/*.text:0040D2AC*/                 call    ___addl
		/*.text:0040D2B1*/                 add     esp, 0Ch
		/*.text:0040D2B4*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:0040D2B7*/                 cmp     /*[ebp+*/ var_C /*]*/, 0
		/*.text:0040D2BB*/                 jz      short loc_40D2CC
		/*.text:0040D2BD*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D2C0*/                 mov     edx, [ecx+8]
		/*.text:0040D2C3*/                 add     edx, 1
		/*.text:0040D2C6*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D2C9*/                 mov     [eax+8], edx
		/*.text:0040D2CC*/ 
		/*.text:0040D2CC*/ loc_40D2CC:                             ; CODE XREF: ___add_12+7B.j
		/*.text:0040D2CC*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D2CF*/                 add     ecx, 8
		/*.text:0040D2D2*/                 push    ecx
		/*.text:0040D2D3*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D2D6*/                 mov     eax, [edx+8]
		/*.text:0040D2D9*/                 push    eax
		/*.text:0040D2DA*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D2DD*/                 mov     edx, [ecx+8]
		/*.text:0040D2E0*/                 push    edx
		/*.text:0040D2E1*/                 call    ___addl
		/*.text:0040D2E6*/                 add     esp, 0Ch
		/*.text:0040D2E9*/                 mov     esp, ebp
		/*.text:0040D2EB*/                 pop     ebp
		/*.text:0040D2EC*/                 retn
		/*.text:0040D2EC*/ // ___add_12       endp
	}
}

static VOID __declspec( naked ) ___mtold12( VOID )
{
	__asm
	{
		/*.text:0040D3C0*/ // ___mtold12      proc near               ; CODE XREF: ___strgtold12+8A7.p
		/*.text:0040D3C0*/ 
		/*.text:0040D3C0*/ #define var_10           word ptr [ebp-0x10]
		/*.text:0040D3C0*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:0040D3C0*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:0040D3C0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:0040D3C0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:0040D3C0*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:0040D3C0*/ #define arg_8            dword ptr [ebp+0x10]
		/*.text:0040D3C0*/ 
		/*.text:0040D3C0*/                 push    ebp
		/*.text:0040D3C1*/                 mov     ebp, esp
		/*.text:0040D3C3*/                 sub     esp, 10h
		/*.text:0040D3C6*/                 mov     /*[ebp+*/ var_10 /*]*/, 404Eh
		/*.text:0040D3CC*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D3CF*/                 mov     dword ptr [eax], 0
		/*.text:0040D3D5*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D3D8*/                 mov     dword ptr [ecx+4], 0
		/*.text:0040D3DF*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D3E2*/                 mov     dword ptr [edx+8], 0
		/*.text:0040D3E9*/                 jmp     short loc_40D3FD
		/*.text:0040D3EB*/ ; ---------------------------------------------------------------------------
		/*.text:0040D3EB*/ 
		/*.text:0040D3EB*/ loc_40D3EB:                             ; CODE XREF: ___mtold12+B2.j
		/*.text:0040D3EB*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040D3EE*/                 sub     eax, 1
		/*.text:0040D3F1*/                 mov     /*[ebp+*/ arg_4 /*]*/, eax
		/*.text:0040D3F4*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D3F7*/                 add     ecx, 1
		/*.text:0040D3FA*/                 mov     /*[ebp+*/ arg_0 /*]*/, ecx
		/*.text:0040D3FD*/ 
		/*.text:0040D3FD*/ loc_40D3FD:                             ; CODE XREF: ___mtold12+29.j
		/*.text:0040D3FD*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0040D401*/                 jbe     short loc_40D477
		/*.text:0040D403*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D406*/                 mov     eax, [edx]
		/*.text:0040D408*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:0040D40B*/                 mov     ecx, [edx+4]
		/*.text:0040D40E*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:0040D411*/                 mov     edx, [edx+8]
		/*.text:0040D414*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0040D417*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D41A*/                 push    eax
		/*.text:0040D41B*/                 call    ___shl_12
		/*.text:0040D420*/                 add     esp, 4
		/*.text:0040D423*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D426*/                 push    ecx
		/*.text:0040D427*/                 call    ___shl_12
		/*.text:0040D42C*/                 add     esp, 4
		/*.text:0040D42F*/                 lea     edx, /*[ebp+*/ var_C /*]*/
		/*.text:0040D432*/                 push    edx
		/*.text:0040D433*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D436*/                 push    eax
		/*.text:0040D437*/                 call    ___add_12
		/*.text:0040D43C*/                 add     esp, 8
		/*.text:0040D43F*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D442*/                 push    ecx
		/*.text:0040D443*/                 call    ___shl_12
		/*.text:0040D448*/                 add     esp, 4
		/*.text:0040D44B*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040D44E*/                 movsx   eax, byte ptr [edx]
		/*.text:0040D451*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:0040D454*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:0040D45B*/                 mov     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0040D462*/                 lea     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:0040D465*/                 push    ecx
		/*.text:0040D466*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D469*/                 push    edx
		/*.text:0040D46A*/                 call    ___add_12
		/*.text:0040D46F*/                 add     esp, 8
		/*.text:0040D472*/                 jmp     loc_40D3EB
		/*.text:0040D477*/ ; ---------------------------------------------------------------------------
		/*.text:0040D477*/ 
		/*.text:0040D477*/ loc_40D477:                             ; CODE XREF: ___mtold12+41.j
		/*.text:0040D477*/                                         ; ___mtold12+101.j
		/*.text:0040D477*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D47A*/                 cmp     dword ptr [eax+8], 0
		/*.text:0040D47E*/                 jnz     short loc_40D4C3
		/*.text:0040D480*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D483*/                 mov     edx, [ecx+4]
		/*.text:0040D486*/                 shr     edx, 10h
		/*.text:0040D489*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D48C*/                 mov     [eax+8], edx
		/*.text:0040D48F*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D492*/                 mov     edx, [ecx+4]
		/*.text:0040D495*/                 shl     edx, 10h
		/*.text:0040D498*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D49B*/                 mov     ecx, [eax]
		/*.text:0040D49D*/                 shr     ecx, 10h
		/*.text:0040D4A0*/                 or      edx, ecx
		/*.text:0040D4A2*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D4A5*/                 mov     [eax+4], edx
		/*.text:0040D4A8*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D4AB*/                 mov     edx, [ecx]
		/*.text:0040D4AD*/                 shl     edx, 10h
		/*.text:0040D4B0*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D4B3*/                 mov     [eax], edx
		/*.text:0040D4B5*/                 mov     cx, /*[ebp+*/ var_10 /*]*/
		/*.text:0040D4B9*/                 sub     cx, 10h
		/*.text:0040D4BD*/                 mov     /*[ebp+*/ var_10 /*]*/, cx
		/*.text:0040D4C1*/                 jmp     short loc_40D477
		/*.text:0040D4C3*/ ; ---------------------------------------------------------------------------
		/*.text:0040D4C3*/ 
		/*.text:0040D4C3*/ loc_40D4C3:                             ; CODE XREF: ___mtold12+BE.j
		/*.text:0040D4C3*/                                         ; ___mtold12+12A.j
		/*.text:0040D4C3*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D4C6*/                 mov     eax, [edx+8]
		/*.text:0040D4C9*/                 and     eax, 8000h
		/*.text:0040D4CE*/                 test    eax, eax
		/*.text:0040D4D0*/                 jnz     short loc_40D4EC
		/*.text:0040D4D2*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D4D5*/                 push    ecx
		/*.text:0040D4D6*/                 call    ___shl_12
		/*.text:0040D4DB*/                 add     esp, 4
		/*.text:0040D4DE*/                 mov     dx, /*[ebp+*/ var_10 /*]*/
		/*.text:0040D4E2*/                 sub     dx, 1
		/*.text:0040D4E6*/                 mov     /*[ebp+*/ var_10 /*]*/, dx
		/*.text:0040D4EA*/                 jmp     short loc_40D4C3
		/*.text:0040D4EC*/ ; ---------------------------------------------------------------------------
		/*.text:0040D4EC*/ 
		/*.text:0040D4EC*/ loc_40D4EC:                             ; CODE XREF: ___mtold12+110.j
		/*.text:0040D4EC*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040D4EF*/                 mov     cx, /*[ebp+*/ var_10 /*]*/
		/*.text:0040D4F3*/                 mov     [eax+0Ah], cx
		/*.text:0040D4F7*/                 mov     esp, ebp
		/*.text:0040D4F9*/                 pop     ebp
		/*.text:0040D4FA*/                 retn
		/*.text:0040D4FA*/ // ___mtold12      endp
	}
}
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 
#undef arg_8 

static VOID __declspec( naked ) __isctype( VOID )
{
	__asm
	{
		/*.text:00401740*/ // __isctype       proc near               ; CODE XREF: _strtod+1F.p
		/*.text:00401740*/                                         ; __forcdecpt+35.p ...
		/*.text:00401740*/ 
		/*.text:00401740*/ #define MultiByteStr     byte ptr [ebp-0x0C]
		/*.text:00401740*/ #define var_B            byte ptr [ebp-0x0B]
		/*.text:00401740*/ #define var_A            byte ptr [ebp-0x0A]
		/*.text:00401740*/ #define cchMultiByte     dword ptr [ebp-0x8]
		/*.text:00401740*/ #define CharType         /*word ptr*/ [ebp-0x4]
		/*.text:00401740*/ #define arg_0            /*dword ptr*/ [ebp+0x8]
		/*.text:00401740*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00401740*/ 
		/*.text:00401740*/                 push    ebp
		/*.text:00401741*/                 mov     ebp, esp
		/*.text:00401743*/                 sub     esp, 0Ch
		/*.text:00401746*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00401749*/                 add     eax, 1
		/*.text:0040174C*/                 cmp     eax, 100h
		/*.text:00401751*/                 ja      short loc_40176A
		/*.text:00401753*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00401756*/                 mov     edx, __pctype
		/*.text:0040175C*/                 xor     eax, eax
		/*.text:0040175E*/                 mov     ax, [edx+ecx*2]
		/*.text:00401762*/                 and     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00401765*/                 jmp     loc_4017F3
		/*.text:0040176A*/ ; ---------------------------------------------------------------------------
		/*.text:0040176A*/ 
		/*.text:0040176A*/ loc_40176A:                             ; CODE XREF: __isctype+11.j
		/*.text:0040176A*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040176D*/                 sar     ecx, 8
		/*.text:00401770*/                 and     ecx, 0FFh
		/*.text:00401776*/                 and     ecx, 0FFh
		/*.text:0040177C*/                 mov     edx, __pctype
		/*.text:00401782*/                 xor     eax, eax
		/*.text:00401784*/                 mov     ax, [edx+ecx*2]
		/*.text:00401788*/                 and     eax, 8000h
		/*.text:0040178D*/                 test    eax, eax
		/*.text:0040178F*/                 jz      short loc_4017B3
		/*.text:00401791*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00401794*/                 sar     ecx, 8
		/*.text:00401797*/                 and     ecx, 0FFh
		/*.text:0040179D*/                 mov     /*[ebp+*/ MultiByteStr /*]*/, cl
		/*.text:004017A0*/                 mov     dl, byte ptr /*[ebp+*/ arg_0 /*]*/
		/*.text:004017A3*/                 mov     /*[ebp+*/ var_B /*]*/, dl
		/*.text:004017A6*/                 mov     /*[ebp+*/ var_A /*]*/, 0
		/*.text:004017AA*/                 mov     /*[ebp+*/ cchMultiByte /*]*/, 2
		/*.text:004017B1*/                 jmp     short loc_4017C4
		/*.text:004017B3*/ ; ---------------------------------------------------------------------------
		/*.text:004017B3*/ 
		/*.text:004017B3*/ loc_4017B3:                             ; CODE XREF: __isctype+4F.j
		/*.text:004017B3*/                 mov     al, byte ptr /*[ebp+*/ arg_0 /*]*/
		/*.text:004017B6*/                 mov     /*[ebp+*/ MultiByteStr /*]*/, al
		/*.text:004017B9*/                 mov     /*[ebp+*/ var_B /*]*/, 0
		/*.text:004017BD*/                 mov     /*[ebp+*/ cchMultiByte /*]*/, 1
		/*.text:004017C4*/ 
		/*.text:004017C4*/ loc_4017C4:                             ; CODE XREF: __isctype+71.j
		/*.text:004017C4*/                 push    1               ; int
		/*.text:004017C6*/                 push    0               ; Locale
		/*.text:004017C8*/                 push    0               ; CodePage
		/*.text:004017CA*/                 lea     ecx, /*[ebp+*/ CharType /*]*/
		/*.text:004017CD*/                 push    ecx             ; lpCharType
		/*.text:004017CE*/                 mov     edx, /*[ebp+*/ cchMultiByte /*]*/
		/*.text:004017D1*/                 push    edx             ; cchMultiByte
		/*.text:004017D2*/                 lea     eax, /*[ebp+*/ MultiByteStr /*]*/
		/*.text:004017D5*/                 push    eax             ; lpMultiByteStr
		/*.text:004017D6*/                 push    1               ; dwInfoType
//-----------------------------------------------------------------------------------------------------------------------
		/*.text:004017D8*/                 // call    ___crtGetStringTypeA
											sub		eax, eax
//-----------------------------------------------------------------------------------------------------------------------
		/*.text:004017DD*/                 add     esp, 1Ch
		/*.text:004017E0*/                 test    eax, eax
		/*.text:004017E2*/                 jnz     short loc_4017E8
		/*.text:004017E4*/                 xor     eax, eax
		/*.text:004017E6*/                 jmp     short loc_4017F3
		/*.text:004017E8*/ ; ---------------------------------------------------------------------------
		/*.text:004017E8*/ 
		/*.text:004017E8*/ loc_4017E8:                             ; CODE XREF: __isctype+A2.j
		/*.text:004017E8*/                 mov     eax, dword ptr /*[ebp+*/ CharType /*]*/
		/*.text:004017EB*/                 and     eax, 0FFFFh
		/*.text:004017F0*/                 and     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:004017F3*/ 
		/*.text:004017F3*/ loc_4017F3:                             ; CODE XREF: __isctype+25.j
		/*.text:004017F3*/                                         ; __isctype+A6.j
		/*.text:004017F3*/                 mov     esp, ebp
		/*.text:004017F5*/                 pop     ebp
		/*.text:004017F6*/                 retn
		/*.text:004017F6*/ // __isctype       endp
	}
}
#undef MultiByteStr
#undef var_B 
#undef var_A 
#undef cchMultiByte
#undef CharType
#undef arg_0 
#undef arg_4 

/*.text:00407068*/ static BYTE byte_407068 [] = {    /*db*/ 0                    , // DATA XREF: ___strgtold12+4A8.r
/*.text:00407069*/                 /*db*/    2 ,  
/*.text:0040706A*/                 /*db*/    0 ,  
/*.text:0040706B*/                 /*db*/    2 ,  
/*.text:0040706C*/                 /*db*/    2 ,  
/*.text:0040706D*/                 /*db*/    2 ,  
/*.text:0040706E*/                 /*db*/    2 ,  
/*.text:0040706F*/                 /*db*/    2 ,  
/*.text:00407070*/                 /*db*/    2 ,  
/*.text:00407071*/                 /*db*/    2 ,  
/*.text:00407072*/                 /*db*/    2 ,  
/*.text:00407073*/                 /*db*/    2 ,  
/*.text:00407074*/                 /*db*/    2 ,  
/*.text:00407075*/                 /*db*/    2 ,  
/*.text:00407076*/                 /*db*/    2 ,  
/*.text:00407077*/                 /*db*/    2 ,  
/*.text:00407078*/                 /*db*/    2 ,  
/*.text:00407079*/                 /*db*/    2 ,  
/*.text:0040707A*/                 /*db*/    2 ,  
/*.text:0040707B*/                 /*db*/    2 ,  
/*.text:0040707C*/                 /*db*/    2 ,  
/*.text:0040707D*/                 /*db*/    2 ,  
/*.text:0040707E*/                 /*db*/    2 ,  
/*.text:0040707F*/                 /*db*/    2 ,  
/*.text:00407080*/                 /*db*/    2 ,  
/*.text:00407081*/                 /*db*/    1 ,  
/*.text:00407082*/                 /*db*/    1 ,  
/*.text:00407083*/                 /*db*/    2 ,  
/*.text:00407084*/                 /*db*/    2 ,  
/*.text:00407085*/                 /*db*/    2 ,  
/*.text:00407086*/                 /*db*/    2 ,  
/*.text:00407087*/                 /*db*/    2 ,  
/*.text:00407088*/                 /*db*/    2 ,  
/*.text:00407089*/                 /*db*/    2 ,  
/*.text:0040708A*/                 /*db*/    2 ,  
/*.text:0040708B*/                 /*db*/    2 ,  
/*.text:0040708C*/                 /*db*/    2 ,  
/*.text:0040708D*/                 /*db*/    2 ,  
/*.text:0040708E*/                 /*db*/    2 ,  
/*.text:0040708F*/                 /*db*/    2 ,  
/*.text:00407090*/                 /*db*/    2 ,  
/*.text:00407091*/                 /*db*/    2 ,  
/*.text:00407092*/                 /*db*/    2 ,  
/*.text:00407093*/                 /*db*/    2 ,  
/*.text:00407094*/                 /*db*/    2 ,  
/*.text:00407095*/                 /*db*/    2 ,  
/*.text:00407096*/                 /*db*/    2 ,  
/*.text:00407097*/                 /*db*/    2 ,  
/*.text:00407098*/                 /*db*/    2 ,  
/*.text:00407099*/                 /*db*/    2 ,  
/*.text:0040709A*/                 /*db*/    2 ,  
/*.text:0040709B*/                 /*db*/    2 ,  
/*.text:0040709C*/                 /*db*/    2 ,  
/*.text:0040709D*/                 /*db*/    2 ,  
/*.text:0040709E*/                 /*db*/    2 ,  
/*.text:0040709F*/                 /*db*/    2 ,  
/*.text:004070A0*/                 /*db*/    2 ,  
/*.text:004070A1*/                 /*db*/    1 ,  
/*.text:004070A2*/                 /*db*/    1   
};

/*.text:00407021*/ static BYTE byte_407021 [] = {    /*db*/ 0                    , // DATA XREF: ___strgtold12+376.r
/*.text:00407022*/                 /*db*/    2 ,  
/*.text:00407023*/                 /*db*/    0 ,  
/*.text:00407024*/                 /*db*/    2 ,  
/*.text:00407025*/                 /*db*/    2 ,  
/*.text:00407026*/                 /*db*/    2 ,  
/*.text:00407027*/                 /*db*/    2 ,  
/*.text:00407028*/                 /*db*/    2 ,  
/*.text:00407029*/                 /*db*/    2 ,  
/*.text:0040702A*/                 /*db*/    2 ,  
/*.text:0040702B*/                 /*db*/    2 ,  
/*.text:0040702C*/                 /*db*/    2 ,  
/*.text:0040702D*/                 /*db*/    2 ,  
/*.text:0040702E*/                 /*db*/    2 ,  
/*.text:0040702F*/                 /*db*/    2 ,  
/*.text:00407030*/                 /*db*/    2 ,  
/*.text:00407031*/                 /*db*/    2 ,  
/*.text:00407032*/                 /*db*/    2 ,  
/*.text:00407033*/                 /*db*/    2 ,  
/*.text:00407034*/                 /*db*/    2 ,  
/*.text:00407035*/                 /*db*/    2 ,  
/*.text:00407036*/                 /*db*/    2 ,  
/*.text:00407037*/                 /*db*/    2 ,  
/*.text:00407038*/                 /*db*/    2 ,  
/*.text:00407039*/                 /*db*/    2 ,  
/*.text:0040703A*/                 /*db*/    1 ,  
/*.text:0040703B*/                 /*db*/    1 ,  
/*.text:0040703C*/                 /*db*/    2 ,  
/*.text:0040703D*/                 /*db*/    2 ,  
/*.text:0040703E*/                 /*db*/    2 ,  
/*.text:0040703F*/                 /*db*/    2 ,  
/*.text:00407040*/                 /*db*/    2 ,  
/*.text:00407041*/                 /*db*/    2 ,  
/*.text:00407042*/                 /*db*/    2 ,  
/*.text:00407043*/                 /*db*/    2 ,  
/*.text:00407044*/                 /*db*/    2 ,  
/*.text:00407045*/                 /*db*/    2 ,  
/*.text:00407046*/                 /*db*/    2 ,  
/*.text:00407047*/                 /*db*/    2 ,  
/*.text:00407048*/                 /*db*/    2 ,  
/*.text:00407049*/                 /*db*/    2 ,  
/*.text:0040704A*/                 /*db*/    2 ,  
/*.text:0040704B*/                 /*db*/    2 ,  
/*.text:0040704C*/                 /*db*/    2 ,  
/*.text:0040704D*/                 /*db*/    2 ,  
/*.text:0040704E*/                 /*db*/    2 ,  
/*.text:0040704F*/                 /*db*/    2 ,  
/*.text:00407050*/                 /*db*/    2 ,  
/*.text:00407051*/                 /*db*/    2 ,  
/*.text:00407052*/                 /*db*/    2 ,  
/*.text:00407053*/                 /*db*/    2 ,  
/*.text:00407054*/                 /*db*/    2 ,  
/*.text:00407055*/                 /*db*/    2 ,  
/*.text:00407056*/                 /*db*/    2 ,  
/*.text:00407057*/                 /*db*/    2 ,  
/*.text:00407058*/                 /*db*/    2 ,  
/*.text:00407059*/                 /*db*/    2 ,  
/*.text:0040705A*/                 /*db*/    1 ,  
/*.text:0040705B*/                 /*db*/    1   
};

/*.text:00406FDA*/ static BYTE byte_406FDA [] = {     /*db*/ 0                    , // DATA XREF: ___strgtold12+1EA.r
/*.text:00406FDB*/                 /*db*/    3 ,  
/*.text:00406FDC*/                 /*db*/    0 ,  
/*.text:00406FDD*/                 /*db*/    3 ,  
/*.text:00406FDE*/                 /*db*/    3 ,  
/*.text:00406FDF*/                 /*db*/    1 ,  
/*.text:00406FE0*/                 /*db*/    3 ,  
/*.text:00406FE1*/                 /*db*/    3 ,  
/*.text:00406FE2*/                 /*db*/    3 ,  
/*.text:00406FE3*/                 /*db*/    3 ,  
/*.text:00406FE4*/                 /*db*/    3 ,  
/*.text:00406FE5*/                 /*db*/    3 ,  
/*.text:00406FE6*/                 /*db*/    3 ,  
/*.text:00406FE7*/                 /*db*/    3 ,  
/*.text:00406FE8*/                 /*db*/    3 ,  
/*.text:00406FE9*/                 /*db*/    3 ,  
/*.text:00406FEA*/                 /*db*/    3 ,  
/*.text:00406FEB*/                 /*db*/    3 ,  
/*.text:00406FEC*/                 /*db*/    3 ,  
/*.text:00406FED*/                 /*db*/    3 ,  
/*.text:00406FEE*/                 /*db*/    3 ,  
/*.text:00406FEF*/                 /*db*/    3 ,  
/*.text:00406FF0*/                 /*db*/    3 ,  
/*.text:00406FF1*/                 /*db*/    3 ,  
/*.text:00406FF2*/                 /*db*/    3 ,  
/*.text:00406FF3*/                 /*db*/    2 ,  
/*.text:00406FF4*/                 /*db*/    2 ,  
/*.text:00406FF5*/                 /*db*/    3 ,  
/*.text:00406FF6*/                 /*db*/    3 ,  
/*.text:00406FF7*/                 /*db*/    3 ,  
/*.text:00406FF8*/                 /*db*/    3 ,  
/*.text:00406FF9*/                 /*db*/    3 ,  
/*.text:00406FFA*/                 /*db*/    3 ,  
/*.text:00406FFB*/                 /*db*/    3 ,  
/*.text:00406FFC*/                 /*db*/    3 ,  
/*.text:00406FFD*/                 /*db*/    3 ,  
/*.text:00406FFE*/                 /*db*/    3 ,  
/*.text:00406FFF*/                 /*db*/    3 ,  
/*.text:00407000*/                 /*db*/    3 ,  
/*.text:00407001*/                 /*db*/    3 ,  
/*.text:00407002*/                 /*db*/    3 ,  
/*.text:00407003*/                 /*db*/    3 ,  
/*.text:00407004*/                 /*db*/    3 ,  
/*.text:00407005*/                 /*db*/    3 ,  
/*.text:00407006*/                 /*db*/    3 ,  
/*.text:00407007*/                 /*db*/    3 ,  
/*.text:00407008*/                 /*db*/    3 ,  
/*.text:00407009*/                 /*db*/    3 ,  
/*.text:0040700A*/                 /*db*/    3 ,  
/*.text:0040700B*/                 /*db*/    3 ,  
/*.text:0040700C*/                 /*db*/    3 ,  
/*.text:0040700D*/                 /*db*/    3 ,  
/*.text:0040700E*/                 /*db*/    3 ,  
/*.text:0040700F*/                 /*db*/    3 ,  
/*.text:00407010*/                 /*db*/    3 ,  
/*.text:00407011*/                 /*db*/    3 ,  
/*.text:00407012*/                 /*db*/    3 ,  
/*.text:00407013*/                 /*db*/    2 ,  
/*.text:00407014*/                 /*db*/    2   
};

static VOID __declspec( naked ) ___strgtold12( VOID )
{
	__asm
	{
		/*.text:00406580*/ // ___strgtold12   proc near               ; CODE XREF: __fltin+21.p
		/*.text:00406580*/                                         ; __atodbl+1A.p ...
		/*.text:00406580*/ 
		/*.text:00406580*/ #define var_B4           byte ptr [ebp-0x0B4]
		/*.text:00406580*/ #define var_B0           dword ptr [ebp-0x0B0]
		/*.text:00406580*/ #define var_AC           dword ptr [ebp-0x0AC]
		/*.text:00406580*/ #define var_A8           byte ptr [ebp-0x0A8]
		/*.text:00406580*/ #define var_A4           byte ptr [ebp-0x0A4]
		/*.text:00406580*/ #define var_A0           dword ptr [ebp-0x0A0]
		/*.text:00406580*/ #define var_9C           dword ptr [ebp-0x9C]
		/*.text:00406580*/ #define var_98           dword ptr [ebp-0x98]
		/*.text:00406580*/ #define var_94           dword ptr [ebp-0x94]
		/*.text:00406580*/ #define var_90           dword ptr [ebp-0x90]
		/*.text:00406580*/ #define var_8C           byte ptr [ebp-0x8C]
		/*.text:00406580*/ #define var_88           dword ptr [ebp-0x88]
		/*.text:00406580*/ #define var_84           byte ptr [ebp-0x84]
		/*.text:00406580*/ #define var_80           dword ptr [ebp-0x80]
		/*.text:00406580*/ #define var_7C           dword ptr [ebp-0x7C]
		/*.text:00406580*/ #define var_78           dword ptr [ebp-0x78]
		/*.text:00406580*/ #define var_74           dword ptr [ebp-0x74]
		/*.text:00406580*/ #define var_70           dword ptr [ebp-0x70]
		/*.text:00406580*/ #define var_6C           dword ptr [ebp-0x6C]
		/*.text:00406580*/ #define var_68           dword ptr [ebp-0x68]
		/*.text:00406580*/ #define var_64           dword ptr [ebp-0x64]
		/*.text:00406580*/ #define var_60           word ptr [ebp-0x60]
		/*.text:00406580*/ #define var_5E           dword ptr [ebp-0x5E]
		/*.text:00406580*/ #define var_5A           dword ptr [ebp-0x5A]
		/*.text:00406580*/ #define var_56           word ptr [ebp-0x56]
		/*.text:00406580*/ #define var_54           dword ptr [ebp-0x54]
		/*.text:00406580*/ #define var_50           /*dword ptr*/ [ebp-0x50]
		/*.text:00406580*/ #define var_4C           dword ptr [ebp-0x4C]
		/*.text:00406580*/ #define var_48           word ptr [ebp-0x48]
		/*.text:00406580*/ #define var_44           dword ptr [ebp-0x44]
		/*.text:00406580*/ #define var_40           dword ptr [ebp-0x40]
		/*.text:00406580*/ #define var_3C           /*dword ptr*/ [ebp-0x3C]
		/*.text:00406580*/ #define var_38           byte ptr [ebp-0x38]
		/*.text:00406580*/ #define var_21           byte ptr [ebp-0x21]
		/*.text:00406580*/ #define var_1C           /*dword ptr*/ [ebp-0x1C]
		/*.text:00406580*/ #define var_18           dword ptr [ebp-0x18]
		/*.text:00406580*/ #define var_14           dword ptr [ebp-0x14]
		/*.text:00406580*/ #define var_10           dword ptr [ebp-0x10]
		/*.text:00406580*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:00406580*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:00406580*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00406580*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00406580*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00406580*/ #define arg_8            dword ptr [ebp+0x10]
		/*.text:00406580*/ #define arg_C            dword ptr [ebp+0x14]
		/*.text:00406580*/ #define arg_10           dword ptr [ebp+0x18]
		/*.text:00406580*/ #define arg_14           dword ptr [ebp+0x1C]
		/*.text:00406580*/ #define arg_18           dword ptr [ebp+0x20]
		/*.text:00406580*/ 
		/*.text:00406580*/                 push    ebp
		/*.text:00406581*/                 mov     ebp, esp
		/*.text:00406583*/                 sub     esp, 0B4h
		/*.text:00406589*/                 lea     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:0040658C*/                 mov     /*[ebp+*/ var_68 /*]*/, eax
		/*.text:0040658F*/                 mov     word ptr /*[ebp+*/ var_1C /*]*/, 0
		/*.text:00406595*/                 mov     /*[ebp+*/ var_74 /*]*/, 1
		/*.text:0040659C*/                 mov     /*[ebp+*/ var_70 /*]*/, 0
		/*.text:004065A3*/                 mov     /*[ebp+*/ var_54 /*]*/, 0
		/*.text:004065AA*/                 mov     /*[ebp+*/ var_C /*]*/, 0
		/*.text:004065B1*/                 mov     /*[ebp+*/ var_18 /*]*/, 0
		/*.text:004065B8*/                 mov     /*[ebp+*/ var_40 /*]*/, 0
		/*.text:004065BF*/                 mov     /*[ebp+*/ var_78 /*]*/, 0
		/*.text:004065C6*/                 mov     /*[ebp+*/ var_14 /*]*/, 0
		/*.text:004065CD*/                 mov     /*[ebp+*/ var_6C /*]*/, 0
		/*.text:004065D4*/                 mov     /*[ebp+*/ var_44 /*]*/, 0
		/*.text:004065DB*/                 mov     /*[ebp+*/ var_4C /*]*/, 0
		/*.text:004065E2*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:004065E5*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:004065E8*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004065EB*/                 mov     /*[ebp+*/ var_64 /*]*/, edx
		/*.text:004065EE*/                 jmp     short loc_4065F9
		/*.text:004065F0*/ ; ---------------------------------------------------------------------------
		/*.text:004065F0*/ 
		/*.text:004065F0*/ loc_4065F0:                             ; CODE XREF: ___strgtold12+A5.j
		/*.text:004065F0*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004065F3*/                 add     eax, 1
		/*.text:004065F6*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004065F9*/ 
		/*.text:004065F9*/ loc_4065F9:                             ; CODE XREF: ___strgtold12+6E.j
		/*.text:004065F9*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004065FC*/                 movsx   edx, byte ptr [ecx]
		/*.text:004065FF*/                 cmp     edx, 20h
		/*.text:00406602*/                 jz      short loc_406625
		/*.text:00406604*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406607*/                 movsx   ecx, byte ptr [eax]
		/*.text:0040660A*/                 cmp     ecx, 9
		/*.text:0040660D*/                 jz      short loc_406625
		/*.text:0040660F*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406612*/                 movsx   eax, byte ptr [edx]
		/*.text:00406615*/                 cmp     eax, 0Ah
		/*.text:00406618*/                 jz      short loc_406625
		/*.text:0040661A*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040661D*/                 movsx   edx, byte ptr [ecx]
		/*.text:00406620*/                 cmp     edx, 0Dh
		/*.text:00406623*/                 jnz     short loc_406627
		/*.text:00406625*/ 
		/*.text:00406625*/ loc_406625:                             ; CODE XREF: ___strgtold12+82.j
		/*.text:00406625*/                                         ; ___strgtold12+8D.j ...
		/*.text:00406625*/                 jmp     short loc_4065F0
		/*.text:00406627*/ ; ---------------------------------------------------------------------------
		/*.text:00406627*/ 
		/*.text:00406627*/ loc_406627:                             ; CODE XREF: ___strgtold12+A3.j
		/*.text:00406627*/                                         ; ___strgtold12+803.j
		/*.text:00406627*/                 cmp     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:0040662B*/                 jz      loc_406D88
		/*.text:00406631*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406634*/                 mov     cl, [eax]
		/*.text:00406636*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, cl
		/*.text:00406639*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040663C*/                 add     edx, 1
		/*.text:0040663F*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406642*/                 mov     eax, /*[ebp+*/ var_4C /*]*/
		/*.text:00406645*/                 mov     /*[ebp+*/ var_80 /*]*/, eax
		/*.text:00406648*/                 cmp     /*[ebp+*/ var_80 /*]*/, 0Bh
		/*.text:0040664C*/                 ja      loc_406D83
		/*.text:00406652*/                 mov     ecx, /*[ebp+*/ var_80 /*]*/
//---------------------------------------------------------------------------------------------------------------------
		/*.text:00406655*/                 // jmp     ds:off_406F9A[ecx*4]
						// .text:00406F9A off_406F9A      dd offset loc_40665C    ; DATA XREF: ___strgtold12+D5.r
						// .text:00406F9E                 dd offset loc_4066FA
						// .text:00406FA2                 dd offset loc_4067B0
						// .text:00406FA6                 dd offset loc_40681B
						// .text:00406FAA                 dd offset loc_406933
						// .text:00406FAE                 dd offset loc_406A65
						// .text:00406FB2                 dd offset loc_406ADB
						// .text:00406FB6                 dd offset loc_406BC0
						// .text:00406FBA                 dd offset loc_406B62
						// .text:00406FBE                 dd offset loc_406C13
						// .text:00406FC2                 dd offset loc_406D83
						// .text:00406FC6                 dd offset loc_406D1F
						// .text:00406FCA off_406FCA      dd offset loc_406789    ; DATA XREF: ___strgtold12+1F0.r
						// .text:00406FCE                 dd offset loc_406777
						// .text:00406FD2                 dd offset loc_406780
						// .text:00406FD6                 dd offset loc_40679B
											SWITCH_CASE_ECX( 0, loc_40665C )
											SWITCH_CASE_ECX( 1, loc_4066FA )
											SWITCH_CASE_ECX( 2, loc_4067B0 )
											SWITCH_CASE_ECX( 3, loc_40681B )
											SWITCH_CASE_ECX( 4, loc_406933 )
											SWITCH_CASE_ECX( 5, loc_406A65 )
											SWITCH_CASE_ECX( 6, loc_406ADB )
											SWITCH_CASE_ECX( 7, loc_406BC0 )
											SWITCH_CASE_ECX( 8, loc_406B62 )
											SWITCH_CASE_ECX( 9, loc_406C13 )
											SWITCH_CASE_ECX( 10, loc_406D83 )
											SWITCH_CASE_ECX( 11, loc_406D1F )
											SWITCH_CASE_ECX( 12, loc_406789 )
											SWITCH_CASE_ECX( 13, loc_406777 )
											SWITCH_CASE_ECX( 14, loc_406780 )
											SWITCH_CASE_ECX( 15, loc_40679B )
//---------------------------------------------------------------------------------------------------------------------
		/*.text:0040665C*/ 
		/*.text:0040665C*/ loc_40665C:                             ; DATA XREF: .text:00406F9A.o
		/*.text:0040665C*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406660*/                 cmp     edx, 31h
		/*.text:00406663*/                 jl      short loc_406680
		/*.text:00406665*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406669*/                 cmp     eax, 39h
		/*.text:0040666C*/                 jg      short loc_406680
		/*.text:0040666E*/                 mov     /*[ebp+*/ var_4C /*]*/, 3
		/*.text:00406675*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406678*/                 sub     ecx, 1
		/*.text:0040667B*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0040667E*/                 jmp     short loc_4066F5
		/*.text:00406680*/ ; ---------------------------------------------------------------------------
		/*.text:00406680*/ 
		/*.text:00406680*/ loc_406680:                             ; CODE XREF: ___strgtold12+E3.j
		/*.text:00406680*/                                         ; ___strgtold12+EC.j
		/*.text:00406680*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406684*/                 movsx   eax, ___decimal_point
		/*.text:0040668B*/                 cmp     edx, eax
		/*.text:0040668D*/                 jnz     short loc_406698
		/*.text:0040668F*/                 mov     /*[ebp+*/ var_4C /*]*/, 5
		/*.text:00406696*/                 jmp     short loc_4066F5
		/*.text:00406698*/ ; ---------------------------------------------------------------------------
		/*.text:00406698*/ 
		/*.text:00406698*/ loc_406698:                             ; CODE XREF: ___strgtold12+10D.j
		/*.text:00406698*/                 mov     cl, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:0040669B*/                 mov     /*[ebp+*/ var_84 /*]*/, cl
		/*.text:004066A1*/                 cmp     /*[ebp+*/ var_84 /*]*/, 2Bh
		/*.text:004066A8*/                 jz      short loc_4066C7
		/*.text:004066AA*/                 cmp     /*[ebp+*/ var_84 /*]*/, 2Dh
		/*.text:004066B1*/                 jz      short loc_4066D6
		/*.text:004066B3*/                 cmp     /*[ebp+*/ var_84 /*]*/, 30h
		/*.text:004066BA*/                 jz      short loc_4066BE
		/*.text:004066BC*/                 jmp     short loc_4066E5
		/*.text:004066BE*/ ; ---------------------------------------------------------------------------
		/*.text:004066BE*/ 
		/*.text:004066BE*/ loc_4066BE:                             ; CODE XREF: ___strgtold12+13A.j
		/*.text:004066BE*/                 mov     /*[ebp+*/ var_4C /*]*/, 1
		/*.text:004066C5*/                 jmp     short loc_4066F5
		/*.text:004066C7*/ ; ---------------------------------------------------------------------------
		/*.text:004066C7*/ 
		/*.text:004066C7*/ loc_4066C7:                             ; CODE XREF: ___strgtold12+128.j
		/*.text:004066C7*/                 mov     /*[ebp+*/ var_4C /*]*/, 2
		/*.text:004066CE*/                 mov     word ptr /*[ebp+*/ var_1C /*]*/, 0
		/*.text:004066D4*/                 jmp     short loc_4066F5
		/*.text:004066D6*/ ; ---------------------------------------------------------------------------
		/*.text:004066D6*/ 
		/*.text:004066D6*/ loc_4066D6:                             ; CODE XREF: ___strgtold12+131.j
		/*.text:004066D6*/                 mov     /*[ebp+*/ var_4C /*]*/, 2
		/*.text:004066DD*/                 mov     word ptr /*[ebp+*/ var_1C /*]*/, 8000h
		/*.text:004066E3*/                 jmp     short loc_4066F5
		/*.text:004066E5*/ ; ---------------------------------------------------------------------------
		/*.text:004066E5*/ 
		/*.text:004066E5*/ loc_4066E5:                             ; CODE XREF: ___strgtold12+13C.j
		/*.text:004066E5*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:004066EC*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004066EF*/                 sub     edx, 1
		/*.text:004066F2*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:004066F5*/ 
		/*.text:004066F5*/ loc_4066F5:                             ; CODE XREF: ___strgtold12+FE.j
		/*.text:004066F5*/                                         ; ___strgtold12+116.j ...
		/*.text:004066F5*/                 jmp     loc_406D83
		/*.text:004066FA*/ ; ---------------------------------------------------------------------------
		/*.text:004066FA*/ 
		/*.text:004066FA*/ loc_4066FA:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:004066FA*/                                         ; DATA XREF: .text:00406F9E.o
		/*.text:004066FA*/                 mov     /*[ebp+*/ var_54 /*]*/, 1
		/*.text:00406701*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406705*/                 cmp     eax, 31h
		/*.text:00406708*/                 jl      short loc_406728
		/*.text:0040670A*/                 movsx   ecx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:0040670E*/                 cmp     ecx, 39h
		/*.text:00406711*/                 jg      short loc_406728
		/*.text:00406713*/                 mov     /*[ebp+*/ var_4C /*]*/, 3
		/*.text:0040671A*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040671D*/                 sub     edx, 1
		/*.text:00406720*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406723*/                 jmp     loc_4067AB
		/*.text:00406728*/ ; ---------------------------------------------------------------------------
		/*.text:00406728*/ 
		/*.text:00406728*/ loc_406728:                             ; CODE XREF: ___strgtold12+188.j
		/*.text:00406728*/                                         ; ___strgtold12+191.j
		/*.text:00406728*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:0040672C*/                 movsx   ecx, ___decimal_point
		/*.text:00406733*/                 cmp     eax, ecx
		/*.text:00406735*/                 jnz     short loc_406740
		/*.text:00406737*/                 mov     /*[ebp+*/ var_4C /*]*/, 4
		/*.text:0040673E*/                 jmp     short loc_4067AB
		/*.text:00406740*/ ; ---------------------------------------------------------------------------
		/*.text:00406740*/ 
		/*.text:00406740*/ loc_406740:                             ; CODE XREF: ___strgtold12+1B5.j
		/*.text:00406740*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406744*/                 mov     /*[ebp+*/ var_88 /*]*/, edx
		/*.text:0040674A*/                 mov     eax, /*[ebp+*/ var_88 /*]*/
		/*.text:00406750*/                 sub     eax, 2Bh
		/*.text:00406753*/                 mov     /*[ebp+*/ var_88 /*]*/, eax
		/*.text:00406759*/                 cmp     /*[ebp+*/ var_88 /*]*/, 3Ah
		/*.text:00406760*/                 ja      short loc_40679B
		/*.text:00406762*/                 mov     edx, /*[ebp+*/ var_88 /*]*/
		/*.text:00406768*/                 xor     ecx, ecx
		/*.text:0040676A*/                 mov     cl, ds:byte_406FDA[edx]
//---------------------------------------------------------------------------------------------------------------------
		/*.text:00406770*/                 // jmp     ds:off_406FCA[ecx*4]
						// .text:00406FCA off_406FCA      dd offset loc_406789    ; DATA XREF: ___strgtold12+1F0.r
						// .text:00406FCE                 dd offset loc_406777
						// .text:00406FD2                 dd offset loc_406780
						// .text:00406FD6                 dd offset loc_40679B
											SWITCH_CASE_ECX( 0, loc_406789 )
											SWITCH_CASE_ECX( 1, loc_406777 )
											SWITCH_CASE_ECX( 2, loc_406780 )
											SWITCH_CASE_ECX( 3, loc_40679B )
//---------------------------------------------------------------------------------------------------------------------
		/*.text:00406777*/ ; ---------------------------------------------------------------------------
		/*.text:00406777*/ 
		/*.text:00406777*/ loc_406777:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406777*/                                         ; DATA XREF: .text:00406FCE.o
		/*.text:00406777*/                 mov     /*[ebp+*/ var_4C /*]*/, 1
		/*.text:0040677E*/                 jmp     short loc_4067AB
		/*.text:00406780*/ ; ---------------------------------------------------------------------------
		/*.text:00406780*/ 
		/*.text:00406780*/ loc_406780:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406780*/                                         ; DATA XREF: .text:00406FD2.o
		/*.text:00406780*/                 mov     /*[ebp+*/ var_4C /*]*/, 6
		/*.text:00406787*/                 jmp     short loc_4067AB
		/*.text:00406789*/ ; ---------------------------------------------------------------------------
		/*.text:00406789*/ 
		/*.text:00406789*/ loc_406789:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406789*/                                         ; ___strgtold12+1F0.j
		/*.text:00406789*/                                         ; DATA XREF: ...
		/*.text:00406789*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0040678C*/                 sub     eax, 1
		/*.text:0040678F*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406792*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Bh
		/*.text:00406799*/                 jmp     short loc_4067AB
		/*.text:0040679B*/ ; ---------------------------------------------------------------------------
		/*.text:0040679B*/ 
		/*.text:0040679B*/ loc_40679B:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:0040679B*/                                         ; ___strgtold12+1E0.j
		/*.text:0040679B*/                                         ; DATA XREF: ...
		/*.text:0040679B*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:004067A2*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004067A5*/                 sub     ecx, 1
		/*.text:004067A8*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:004067AB*/ 
		/*.text:004067AB*/ loc_4067AB:                             ; CODE XREF: ___strgtold12+1A3.j
		/*.text:004067AB*/                                         ; ___strgtold12+1BE.j ...
		/*.text:004067AB*/                 jmp     loc_406D83
		/*.text:004067B0*/ ; ---------------------------------------------------------------------------
		/*.text:004067B0*/ 
		/*.text:004067B0*/ loc_4067B0:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:004067B0*/                                         ; DATA XREF: .text:00406FA2.o
		/*.text:004067B0*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004067B4*/                 cmp     edx, 31h
		/*.text:004067B7*/                 jl      short loc_4067D4
		/*.text:004067B9*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004067BD*/                 cmp     eax, 39h
		/*.text:004067C0*/                 jg      short loc_4067D4
		/*.text:004067C2*/                 mov     /*[ebp+*/ var_4C /*]*/, 3
		/*.text:004067C9*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004067CC*/                 sub     ecx, 1
		/*.text:004067CF*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:004067D2*/                 jmp     short loc_406816
		/*.text:004067D4*/ ; ---------------------------------------------------------------------------
		/*.text:004067D4*/ 
		/*.text:004067D4*/ loc_4067D4:                             ; CODE XREF: ___strgtold12+237.j
		/*.text:004067D4*/                                         ; ___strgtold12+240.j
		/*.text:004067D4*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004067D8*/                 movsx   eax, ___decimal_point
		/*.text:004067DF*/                 cmp     edx, eax
		/*.text:004067E1*/                 jnz     short loc_4067EC
		/*.text:004067E3*/                 mov     /*[ebp+*/ var_4C /*]*/, 5
		/*.text:004067EA*/                 jmp     short loc_406816
		/*.text:004067EC*/ ; ---------------------------------------------------------------------------
		/*.text:004067EC*/ 
		/*.text:004067EC*/ loc_4067EC:                             ; CODE XREF: ___strgtold12+261.j
		/*.text:004067EC*/                 mov     cl, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004067EF*/                 mov     /*[ebp+*/ var_8C /*]*/, cl
		/*.text:004067F5*/                 cmp     /*[ebp+*/ var_8C /*]*/, 30h
		/*.text:004067FC*/                 jz      short loc_406800
		/*.text:004067FE*/                 jmp     short loc_406809
		/*.text:00406800*/ ; ---------------------------------------------------------------------------
		/*.text:00406800*/ 
		/*.text:00406800*/ loc_406800:                             ; CODE XREF: ___strgtold12+27C.j
		/*.text:00406800*/                 mov     /*[ebp+*/ var_4C /*]*/, 1
		/*.text:00406807*/                 jmp     short loc_406816
		/*.text:00406809*/ ; ---------------------------------------------------------------------------
		/*.text:00406809*/ 
		/*.text:00406809*/ loc_406809:                             ; CODE XREF: ___strgtold12+27E.j
		/*.text:00406809*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406810*/                 mov     edx, /*[ebp+*/ var_64 /*]*/
		/*.text:00406813*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406816*/ 
		/*.text:00406816*/ loc_406816:                             ; CODE XREF: ___strgtold12+252.j
		/*.text:00406816*/                                         ; ___strgtold12+26A.j ...
		/*.text:00406816*/                 jmp     loc_406D83
		/*.text:0040681B*/ ; ---------------------------------------------------------------------------
		/*.text:0040681B*/ 
		/*.text:0040681B*/ loc_40681B:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:0040681B*/                                         ; DATA XREF: .text:00406FA6.o
		/*.text:0040681B*/                 mov     /*[ebp+*/ var_54 /*]*/, 1
		/*.text:00406822*/                 jmp     short loc_406835
		/*.text:00406824*/ ; ---------------------------------------------------------------------------
		/*.text:00406824*/ 
		/*.text:00406824*/ loc_406824:                             ; CODE XREF: ___strgtold12+32F.j
		/*.text:00406824*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406827*/                 mov     cl, [eax]
		/*.text:00406829*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, cl
		/*.text:0040682C*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040682F*/                 add     edx, 1
		/*.text:00406832*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406835*/ 
		/*.text:00406835*/ loc_406835:                             ; CODE XREF: ___strgtold12+2A2.j
		/*.text:00406835*/                 cmp     ___mb_cur_max, 1
		/*.text:0040683C*/                 jle     short loc_406859
		/*.text:0040683E*/                 push    4               ; int
		/*.text:00406840*/                 mov     eax, /*[ebp+*/ var_3C /*]*/
		/*.text:00406843*/                 and     eax, 0FFh
		/*.text:00406848*/                 push    eax             ; int
		/*.text:00406849*/                 call    __isctype
		/*.text:0040684E*/                 add     esp, 8
		/*.text:00406851*/                 mov     /*[ebp+*/ var_90 /*]*/, eax
		/*.text:00406857*/                 jmp     short loc_406877
		/*.text:00406859*/ ; ---------------------------------------------------------------------------
		/*.text:00406859*/ 
		/*.text:00406859*/ loc_406859:                             ; CODE XREF: ___strgtold12+2BC.j
		/*.text:00406859*/                 mov     ecx, /*[ebp+*/ var_3C /*]*/
		/*.text:0040685C*/                 and     ecx, 0FFh
		/*.text:00406862*/                 mov     edx, __pctype
		/*.text:00406868*/                 xor     eax, eax
		/*.text:0040686A*/                 mov     ax, [edx+ecx*2]
		/*.text:0040686E*/                 and     eax, 4
		/*.text:00406871*/                 mov     /*[ebp+*/ var_90 /*]*/, eax
		/*.text:00406877*/ 
		/*.text:00406877*/ loc_406877:                             ; CODE XREF: ___strgtold12+2D7.j
		/*.text:00406877*/                 cmp     /*[ebp+*/ var_90 /*]*/, 0
		/*.text:0040687E*/                 jz      short loc_4068B4
		/*.text:00406880*/                 cmp     /*[ebp+*/ var_70 /*]*/, 19h
		/*.text:00406884*/                 jnb     short loc_4068A6
		/*.text:00406886*/                 mov     ecx, /*[ebp+*/ var_70 /*]*/
		/*.text:00406889*/                 add     ecx, 1
		/*.text:0040688C*/                 mov     /*[ebp+*/ var_70 /*]*/, ecx
		/*.text:0040688F*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406893*/                 sub     edx, 30h
		/*.text:00406896*/                 mov     eax, /*[ebp+*/ var_68 /*]*/
		/*.text:00406899*/                 mov     [eax], dl
		/*.text:0040689B*/                 mov     ecx, /*[ebp+*/ var_68 /*]*/
		/*.text:0040689E*/                 add     ecx, 1
		/*.text:004068A1*/                 mov     /*[ebp+*/ var_68 /*]*/, ecx
		/*.text:004068A4*/                 jmp     short loc_4068AF
		/*.text:004068A6*/ ; ---------------------------------------------------------------------------
		/*.text:004068A6*/ 
		/*.text:004068A6*/ loc_4068A6:                             ; CODE XREF: ___strgtold12+304.j
		/*.text:004068A6*/                 mov     edx, /*[ebp+*/ var_6C /*]*/
		/*.text:004068A9*/                 add     edx, 1
		/*.text:004068AC*/                 mov     /*[ebp+*/ var_6C /*]*/, edx
		/*.text:004068AF*/ 
		/*.text:004068AF*/ loc_4068AF:                             ; CODE XREF: ___strgtold12+324.j
		/*.text:004068AF*/                 jmp     loc_406824
		/*.text:004068B4*/ ; ---------------------------------------------------------------------------
		/*.text:004068B4*/ 
		/*.text:004068B4*/ loc_4068B4:                             ; CODE XREF: ___strgtold12+2FE.j
		/*.text:004068B4*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004068B8*/                 movsx   ecx, ___decimal_point
		/*.text:004068BF*/                 cmp     eax, ecx
		/*.text:004068C1*/                 jnz     short loc_4068CC
		/*.text:004068C3*/                 mov     /*[ebp+*/ var_4C /*]*/, 4
		/*.text:004068CA*/                 jmp     short loc_40692E
		/*.text:004068CC*/ ; ---------------------------------------------------------------------------
		/*.text:004068CC*/ 
		/*.text:004068CC*/ loc_4068CC:                             ; CODE XREF: ___strgtold12+341.j
		/*.text:004068CC*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004068D0*/                 mov     /*[ebp+*/ var_94 /*]*/, edx
		/*.text:004068D6*/                 mov     eax, /*[ebp+*/ var_94 /*]*/
		/*.text:004068DC*/                 sub     eax, 2Bh
		/*.text:004068DF*/                 mov     /*[ebp+*/ var_94 /*]*/, eax
		/*.text:004068E5*/                 cmp     /*[ebp+*/ var_94 /*]*/, 3Ah
		/*.text:004068EC*/                 ja      short loc_40691E
		/*.text:004068EE*/                 mov     edx, /*[ebp+*/ var_94 /*]*/
		/*.text:004068F4*/                 xor     ecx, ecx
		/*.text:004068F6*/                 mov     cl, ds:byte_407021[edx]
//---------------------------------------------------------------------------------------------------------------------
		/*.text:004068FC*/                 // jmp     ds:off_407015[ecx*4]
						// .text:00407015 off_407015      dd offset loc_40690C    ; DATA XREF: ___strgtold12+37C.r
						// .text:00407019                 dd offset loc_406903
						// .text:0040701D                 dd offset loc_40691E
											SWITCH_CASE_ECX( 0, loc_40690C )
											SWITCH_CASE_ECX( 1, loc_406903 )
											SWITCH_CASE_ECX( 2, loc_40691E )
//---------------------------------------------------------------------------------------------------------------------
		/*.text:00406903*/ 
		/*.text:00406903*/ loc_406903:                             ; DATA XREF: .text:00407019.o
		/*.text:00406903*/                 mov     /*[ebp+*/ var_4C /*]*/, 6
		/*.text:0040690A*/                 jmp     short loc_40692E
		/*.text:0040690C*/ ; ---------------------------------------------------------------------------
		/*.text:0040690C*/ 
		/*.text:0040690C*/ loc_40690C:                             ; CODE XREF: ___strgtold12+37C.j
		/*.text:0040690C*/                                         ; DATA XREF: .text:00407015.o
		/*.text:0040690C*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0040690F*/                 sub     eax, 1
		/*.text:00406912*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406915*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Bh
		/*.text:0040691C*/                 jmp     short loc_40692E
		/*.text:0040691E*/ ; ---------------------------------------------------------------------------
		/*.text:0040691E*/ 
		/*.text:0040691E*/ loc_40691E:                             ; CODE XREF: ___strgtold12+36C.j
		/*.text:0040691E*/                                         ; ___strgtold12+37C.j
		/*.text:0040691E*/                                         ; DATA XREF: ...
		/*.text:0040691E*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406925*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406928*/                 sub     ecx, 1
		/*.text:0040692B*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0040692E*/ 
		/*.text:0040692E*/ loc_40692E:                             ; CODE XREF: ___strgtold12+34A.j
		/*.text:0040692E*/                                         ; ___strgtold12+38A.j ...
		/*.text:0040692E*/                 jmp     loc_406D83
		/*.text:00406933*/ ; ---------------------------------------------------------------------------
		/*.text:00406933*/ 
		/*.text:00406933*/ loc_406933:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406933*/                                         ; DATA XREF: .text:00406FAA.o
		/*.text:00406933*/                 mov     /*[ebp+*/ var_54 /*]*/, 1
		/*.text:0040693A*/                 mov     /*[ebp+*/ var_C /*]*/, 1
		/*.text:00406941*/                 cmp     /*[ebp+*/ var_70 /*]*/, 0
		/*.text:00406945*/                 jnz     short loc_40696E
		/*.text:00406947*/                 jmp     short loc_40695A
		/*.text:00406949*/ ; ---------------------------------------------------------------------------
		/*.text:00406949*/ 
		/*.text:00406949*/ loc_406949:                             ; CODE XREF: ___strgtold12+3EC.j
		/*.text:00406949*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040694C*/                 mov     al, [edx]
		/*.text:0040694E*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, al
		/*.text:00406951*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406954*/                 add     ecx, 1
		/*.text:00406957*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0040695A*/ 
		/*.text:0040695A*/ loc_40695A:                             ; CODE XREF: ___strgtold12+3C7.j
		/*.text:0040695A*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:0040695E*/                 cmp     edx, 30h
		/*.text:00406961*/                 jnz     short loc_40696E
		/*.text:00406963*/                 mov     eax, /*[ebp+*/ var_6C /*]*/
		/*.text:00406966*/                 sub     eax, 1
		/*.text:00406969*/                 mov     /*[ebp+*/ var_6C /*]*/, eax
		/*.text:0040696C*/                 jmp     short loc_406949
		/*.text:0040696E*/ ; ---------------------------------------------------------------------------
		/*.text:0040696E*/ 
		/*.text:0040696E*/ loc_40696E:                             ; CODE XREF: ___strgtold12+3C5.j
		/*.text:0040696E*/                                         ; ___strgtold12+3E1.j
		/*.text:0040696E*/                 jmp     short loc_406981
		/*.text:00406970*/ ; ---------------------------------------------------------------------------
		/*.text:00406970*/ 
		/*.text:00406970*/ loc_406970:                             ; CODE XREF: ___strgtold12+479.j
		/*.text:00406970*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406973*/                 mov     dl, [ecx]
		/*.text:00406975*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, dl
		/*.text:00406978*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0040697B*/                 add     eax, 1
		/*.text:0040697E*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406981*/ 
		/*.text:00406981*/ loc_406981:                             ; CODE XREF: ___strgtold12+3EE.j
		/*.text:00406981*/                 cmp     ___mb_cur_max, 1
		/*.text:00406988*/                 jle     short loc_4069A6
		/*.text:0040698A*/                 push    4               ; int
		/*.text:0040698C*/                 mov     ecx, /*[ebp+*/ var_3C /*]*/
		/*.text:0040698F*/                 and     ecx, 0FFh
		/*.text:00406995*/                 push    ecx             ; int
		/*.text:00406996*/                 call    __isctype
		/*.text:0040699B*/                 add     esp, 8
		/*.text:0040699E*/                 mov     /*[ebp+*/ var_98 /*]*/, eax
		/*.text:004069A4*/                 jmp     short loc_4069C3
		/*.text:004069A6*/ ; ---------------------------------------------------------------------------
		/*.text:004069A6*/ 
		/*.text:004069A6*/ loc_4069A6:                             ; CODE XREF: ___strgtold12+408.j
		/*.text:004069A6*/                 mov     edx, /*[ebp+*/ var_3C /*]*/
		/*.text:004069A9*/                 and     edx, 0FFh
		/*.text:004069AF*/                 mov     eax, __pctype
		/*.text:004069B4*/                 xor     ecx, ecx
		/*.text:004069B6*/                 mov     cx, [eax+edx*2]
		/*.text:004069BA*/                 and     ecx, 4
		/*.text:004069BD*/                 mov     /*[ebp+*/ var_98 /*]*/, ecx
		/*.text:004069C3*/ 
		/*.text:004069C3*/ loc_4069C3:                             ; CODE XREF: ___strgtold12+424.j
		/*.text:004069C3*/                 cmp     /*[ebp+*/ var_98 /*]*/, 0
		/*.text:004069CA*/                 jz      short loc_4069FE
		/*.text:004069CC*/                 cmp     /*[ebp+*/ var_70 /*]*/, 19h
		/*.text:004069D0*/                 jnb     short loc_4069F9
		/*.text:004069D2*/                 mov     edx, /*[ebp+*/ var_70 /*]*/
		/*.text:004069D5*/                 add     edx, 1
		/*.text:004069D8*/                 mov     /*[ebp+*/ var_70 /*]*/, edx
		/*.text:004069DB*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:004069DF*/                 sub     eax, 30h
		/*.text:004069E2*/                 mov     ecx, /*[ebp+*/ var_68 /*]*/
		/*.text:004069E5*/                 mov     [ecx], al
		/*.text:004069E7*/                 mov     edx, /*[ebp+*/ var_68 /*]*/
		/*.text:004069EA*/                 add     edx, 1
		/*.text:004069ED*/                 mov     /*[ebp+*/ var_68 /*]*/, edx
		/*.text:004069F0*/                 mov     eax, /*[ebp+*/ var_6C /*]*/
		/*.text:004069F3*/                 sub     eax, 1
		/*.text:004069F6*/                 mov     /*[ebp+*/ var_6C /*]*/, eax
		/*.text:004069F9*/ 
		/*.text:004069F9*/ loc_4069F9:                             ; CODE XREF: ___strgtold12+450.j
		/*.text:004069F9*/                 jmp     loc_406970
		/*.text:004069FE*/ ; ---------------------------------------------------------------------------
		/*.text:004069FE*/ 
		/*.text:004069FE*/ loc_4069FE:                             ; CODE XREF: ___strgtold12+44A.j
		/*.text:004069FE*/                 movsx   ecx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406A02*/                 mov     /*[ebp+*/ var_9C /*]*/, ecx
		/*.text:00406A08*/                 mov     edx, /*[ebp+*/ var_9C /*]*/
		/*.text:00406A0E*/                 sub     edx, 2Bh
		/*.text:00406A11*/                 mov     /*[ebp+*/ var_9C /*]*/, edx
		/*.text:00406A17*/                 cmp     /*[ebp+*/ var_9C /*]*/, 3Ah
		/*.text:00406A1E*/                 ja      short loc_406A50
		/*.text:00406A20*/                 mov     ecx, /*[ebp+*/ var_9C /*]*/
		/*.text:00406A26*/                 xor     eax, eax
		/*.text:00406A28*/                 mov     al, ds:byte_407068[ecx]
//---------------------------------------------------------------------------------------------------------------------
		/*.text:00406A2E*/                 // jmp     ds:off_40705C[eax*4]
						// .text:0040705C off_40705C      dd offset loc_406A3E    ; DATA XREF: ___strgtold12+4AE.r
						// .text:00407060                 dd offset loc_406A35
						// .text:00407064                 dd offset loc_406A50
											SWITCH_CASE_EAX( 0, loc_406A3E )
											SWITCH_CASE_EAX( 1, loc_406A35 )
											SWITCH_CASE_EAX( 2, loc_406A50 )
//---------------------------------------------------------------------------------------------------------------------
		/*.text:00406A35*/ 
		/*.text:00406A35*/ loc_406A35:                             ; DATA XREF: .text:00407060.o
		/*.text:00406A35*/                 mov     /*[ebp+*/ var_4C /*]*/, 6
		/*.text:00406A3C*/                 jmp     short loc_406A60
		/*.text:00406A3E*/ ; ---------------------------------------------------------------------------
		/*.text:00406A3E*/ 
		/*.text:00406A3E*/ loc_406A3E:                             ; CODE XREF: ___strgtold12+4AE.j
		/*.text:00406A3E*/                                         ; DATA XREF: .text:0040705C.o
		/*.text:00406A3E*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406A41*/                 sub     edx, 1
		/*.text:00406A44*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406A47*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Bh
		/*.text:00406A4E*/                 jmp     short loc_406A60
		/*.text:00406A50*/ ; ---------------------------------------------------------------------------
		/*.text:00406A50*/ 
		/*.text:00406A50*/ loc_406A50:                             ; CODE XREF: ___strgtold12+49E.j
		/*.text:00406A50*/                                         ; ___strgtold12+4AE.j
		/*.text:00406A50*/                                         ; DATA XREF: ...
		/*.text:00406A50*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406A57*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406A5A*/                 sub     eax, 1
		/*.text:00406A5D*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406A60*/ 
		/*.text:00406A60*/ loc_406A60:                             ; CODE XREF: ___strgtold12+4BC.j
		/*.text:00406A60*/                                         ; ___strgtold12+4CE.j
		/*.text:00406A60*/                 jmp     loc_406D83
		/*.text:00406A65*/ ; ---------------------------------------------------------------------------
		/*.text:00406A65*/ 
		/*.text:00406A65*/ loc_406A65:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406A65*/                                         ; DATA XREF: .text:00406FAE.o
		/*.text:00406A65*/                 mov     /*[ebp+*/ var_C /*]*/, 1
		/*.text:00406A6C*/                 cmp     ___mb_cur_max, 1
		/*.text:00406A73*/                 jle     short loc_406A91
		/*.text:00406A75*/                 push    4               ; int
		/*.text:00406A77*/                 mov     ecx, /*[ebp+*/ var_3C /*]*/
		/*.text:00406A7A*/                 and     ecx, 0FFh
		/*.text:00406A80*/                 push    ecx             ; int
		/*.text:00406A81*/                 call    __isctype
		/*.text:00406A86*/                 add     esp, 8
		/*.text:00406A89*/                 mov     /*[ebp+*/ var_A0 /*]*/, eax
		/*.text:00406A8F*/                 jmp     short loc_406AAE
		/*.text:00406A91*/ ; ---------------------------------------------------------------------------
		/*.text:00406A91*/ 
		/*.text:00406A91*/ loc_406A91:                             ; CODE XREF: ___strgtold12+4F3.j
		/*.text:00406A91*/                 mov     edx, /*[ebp+*/ var_3C /*]*/
		/*.text:00406A94*/                 and     edx, 0FFh
		/*.text:00406A9A*/                 mov     eax, __pctype
		/*.text:00406A9F*/                 xor     ecx, ecx
		/*.text:00406AA1*/                 mov     cx, [eax+edx*2]
		/*.text:00406AA5*/                 and     ecx, 4
		/*.text:00406AA8*/                 mov     /*[ebp+*/ var_A0 /*]*/, ecx
		/*.text:00406AAE*/ 
		/*.text:00406AAE*/ loc_406AAE:                             ; CODE XREF: ___strgtold12+50F.j
		/*.text:00406AAE*/                 cmp     /*[ebp+*/ var_A0 /*]*/, 0
		/*.text:00406AB5*/                 jz      short loc_406AC9
		/*.text:00406AB7*/                 mov     /*[ebp+*/ var_4C /*]*/, 4
		/*.text:00406ABE*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406AC1*/                 sub     edx, 1
		/*.text:00406AC4*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406AC7*/                 jmp     short loc_406AD6
		/*.text:00406AC9*/ ; ---------------------------------------------------------------------------
		/*.text:00406AC9*/ 
		/*.text:00406AC9*/ loc_406AC9:                             ; CODE XREF: ___strgtold12+535.j
		/*.text:00406AC9*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406AD0*/                 mov     eax, /*[ebp+*/ var_64 /*]*/
		/*.text:00406AD3*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406AD6*/ 
		/*.text:00406AD6*/ loc_406AD6:                             ; CODE XREF: ___strgtold12+547.j
		/*.text:00406AD6*/                 jmp     loc_406D83
		/*.text:00406ADB*/ ; ---------------------------------------------------------------------------
		/*.text:00406ADB*/ 
		/*.text:00406ADB*/ loc_406ADB:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406ADB*/                                         ; DATA XREF: .text:00406FB2.o
		/*.text:00406ADB*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406ADE*/                 sub     ecx, 2
		/*.text:00406AE1*/                 mov     /*[ebp+*/ var_64 /*]*/, ecx
		/*.text:00406AE4*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406AE8*/                 cmp     edx, 31h
		/*.text:00406AEB*/                 jl      short loc_406B08
		/*.text:00406AED*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406AF1*/                 cmp     eax, 39h
		/*.text:00406AF4*/                 jg      short loc_406B08
		/*.text:00406AF6*/                 mov     /*[ebp+*/ var_4C /*]*/, 9
		/*.text:00406AFD*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406B00*/                 sub     ecx, 1
		/*.text:00406B03*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00406B06*/                 jmp     short loc_406B5D
		/*.text:00406B08*/ ; ---------------------------------------------------------------------------
		/*.text:00406B08*/ 
		/*.text:00406B08*/ loc_406B08:                             ; CODE XREF: ___strgtold12+56B.j
		/*.text:00406B08*/                                         ; ___strgtold12+574.j
		/*.text:00406B08*/                 mov     dl, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406B0B*/                 mov     /*[ebp+*/ var_A4 /*]*/, dl
		/*.text:00406B11*/                 cmp     /*[ebp+*/ var_A4 /*]*/, 2Bh
		/*.text:00406B18*/                 jz      short loc_406B47
		/*.text:00406B1A*/                 cmp     /*[ebp+*/ var_A4 /*]*/, 2Dh
		/*.text:00406B21*/                 jz      short loc_406B37
		/*.text:00406B23*/                 cmp     /*[ebp+*/ var_A4 /*]*/, 30h
		/*.text:00406B2A*/                 jz      short loc_406B2E
		/*.text:00406B2C*/                 jmp     short loc_406B50
		/*.text:00406B2E*/ ; ---------------------------------------------------------------------------
		/*.text:00406B2E*/ 
		/*.text:00406B2E*/ loc_406B2E:                             ; CODE XREF: ___strgtold12+5AA.j
		/*.text:00406B2E*/                 mov     /*[ebp+*/ var_4C /*]*/, 8
		/*.text:00406B35*/                 jmp     short loc_406B5D
		/*.text:00406B37*/ ; ---------------------------------------------------------------------------
		/*.text:00406B37*/ 
		/*.text:00406B37*/ loc_406B37:                             ; CODE XREF: ___strgtold12+5A1.j
		/*.text:00406B37*/                 mov     /*[ebp+*/ var_4C /*]*/, 7
		/*.text:00406B3E*/                 mov     /*[ebp+*/ var_74 /*]*/, 0FFFFFFFFh
		/*.text:00406B45*/                 jmp     short loc_406B5D
		/*.text:00406B47*/ ; ---------------------------------------------------------------------------
		/*.text:00406B47*/ 
		/*.text:00406B47*/ loc_406B47:                             ; CODE XREF: ___strgtold12+598.j
		/*.text:00406B47*/                 mov     /*[ebp+*/ var_4C /*]*/, 7
		/*.text:00406B4E*/                 jmp     short loc_406B5D
		/*.text:00406B50*/ ; ---------------------------------------------------------------------------
		/*.text:00406B50*/ 
		/*.text:00406B50*/ loc_406B50:                             ; CODE XREF: ___strgtold12+5AC.j
		/*.text:00406B50*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406B57*/                 mov     eax, /*[ebp+*/ var_64 /*]*/
		/*.text:00406B5A*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406B5D*/ 
		/*.text:00406B5D*/ loc_406B5D:                             ; CODE XREF: ___strgtold12+586.j
		/*.text:00406B5D*/                                         ; ___strgtold12+5B5.j ...
		/*.text:00406B5D*/                 jmp     loc_406D83
		/*.text:00406B62*/ ; ---------------------------------------------------------------------------
		/*.text:00406B62*/ 
		/*.text:00406B62*/ loc_406B62:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406B62*/                                         ; DATA XREF: .text:00406FBA.o
		/*.text:00406B62*/                 mov     /*[ebp+*/ var_18 /*]*/, 1
		/*.text:00406B69*/                 jmp     short loc_406B7C
		/*.text:00406B6B*/ ; ---------------------------------------------------------------------------
		/*.text:00406B6B*/ 
		/*.text:00406B6B*/ loc_406B6B:                             ; CODE XREF: ___strgtold12+605.j
		/*.text:00406B6B*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406B6E*/                 mov     dl, [ecx]
		/*.text:00406B70*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, dl
		/*.text:00406B73*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406B76*/                 add     eax, 1
		/*.text:00406B79*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406B7C*/ 
		/*.text:00406B7C*/ loc_406B7C:                             ; CODE XREF: ___strgtold12+5E9.j
		/*.text:00406B7C*/                 movsx   ecx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406B80*/                 cmp     ecx, 30h
		/*.text:00406B83*/                 jnz     short loc_406B87
		/*.text:00406B85*/                 jmp     short loc_406B6B
		/*.text:00406B87*/ ; ---------------------------------------------------------------------------
		/*.text:00406B87*/ 
		/*.text:00406B87*/ loc_406B87:                             ; CODE XREF: ___strgtold12+603.j
		/*.text:00406B87*/                 movsx   edx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406B8B*/                 cmp     edx, 31h
		/*.text:00406B8E*/                 jl      short loc_406BAB
		/*.text:00406B90*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406B94*/                 cmp     eax, 39h
		/*.text:00406B97*/                 jg      short loc_406BAB
		/*.text:00406B99*/                 mov     /*[ebp+*/ var_4C /*]*/, 9
		/*.text:00406BA0*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406BA3*/                 sub     ecx, 1
		/*.text:00406BA6*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00406BA9*/                 jmp     short loc_406BBB
		/*.text:00406BAB*/ ; ---------------------------------------------------------------------------
		/*.text:00406BAB*/ 
		/*.text:00406BAB*/ loc_406BAB:                             ; CODE XREF: ___strgtold12+60E.j
		/*.text:00406BAB*/                                         ; ___strgtold12+617.j
		/*.text:00406BAB*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406BB2*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406BB5*/                 sub     edx, 1
		/*.text:00406BB8*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406BBB*/ 
		/*.text:00406BBB*/ loc_406BBB:                             ; CODE XREF: ___strgtold12+629.j
		/*.text:00406BBB*/                 jmp     loc_406D83
		/*.text:00406BC0*/ ; ---------------------------------------------------------------------------
		/*.text:00406BC0*/ 
		/*.text:00406BC0*/ loc_406BC0:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406BC0*/                                         ; DATA XREF: .text:00406FB6.o
		/*.text:00406BC0*/                 movsx   eax, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406BC4*/                 cmp     eax, 31h
		/*.text:00406BC7*/                 jl      short loc_406BE4
		/*.text:00406BC9*/                 movsx   ecx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406BCD*/                 cmp     ecx, 39h
		/*.text:00406BD0*/                 jg      short loc_406BE4
		/*.text:00406BD2*/                 mov     /*[ebp+*/ var_4C /*]*/, 9
		/*.text:00406BD9*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406BDC*/                 sub     edx, 1
		/*.text:00406BDF*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406BE2*/                 jmp     short loc_406C0E
		/*.text:00406BE4*/ ; ---------------------------------------------------------------------------
		/*.text:00406BE4*/ 
		/*.text:00406BE4*/ loc_406BE4:                             ; CODE XREF: ___strgtold12+647.j
		/*.text:00406BE4*/                                         ; ___strgtold12+650.j
		/*.text:00406BE4*/                 mov     al, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406BE7*/                 mov     /*[ebp+*/ var_A8 /*]*/, al
		/*.text:00406BED*/                 cmp     /*[ebp+*/ var_A8 /*]*/, 30h
		/*.text:00406BF4*/                 jz      short loc_406BF8
		/*.text:00406BF6*/                 jmp     short loc_406C01
		/*.text:00406BF8*/ ; ---------------------------------------------------------------------------
		/*.text:00406BF8*/ 
		/*.text:00406BF8*/ loc_406BF8:                             ; CODE XREF: ___strgtold12+674.j
		/*.text:00406BF8*/                 mov     /*[ebp+*/ var_4C /*]*/, 8
		/*.text:00406BFF*/                 jmp     short loc_406C0E
		/*.text:00406C01*/ ; ---------------------------------------------------------------------------
		/*.text:00406C01*/ 
		/*.text:00406C01*/ loc_406C01:                             ; CODE XREF: ___strgtold12+676.j
		/*.text:00406C01*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406C08*/                 mov     ecx, /*[ebp+*/ var_64 /*]*/
		/*.text:00406C0B*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00406C0E*/ 
		/*.text:00406C0E*/ loc_406C0E:                             ; CODE XREF: ___strgtold12+662.j
		/*.text:00406C0E*/                                         ; ___strgtold12+67F.j
		/*.text:00406C0E*/                 jmp     loc_406D83
		/*.text:00406C13*/ ; ---------------------------------------------------------------------------
		/*.text:00406C13*/ 
		/*.text:00406C13*/ loc_406C13:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406C13*/                                         ; DATA XREF: .text:00406FBE.o
		/*.text:00406C13*/                 mov     /*[ebp+*/ var_18 /*]*/, 1
		/*.text:00406C1A*/                 mov     /*[ebp+*/ var_7C /*]*/, 0
		/*.text:00406C21*/                 jmp     short loc_406C34
		/*.text:00406C23*/ ; ---------------------------------------------------------------------------
		/*.text:00406C23*/ 
		/*.text:00406C23*/ loc_406C23:                             ; CODE XREF: ___strgtold12+722.j
		/*.text:00406C23*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406C26*/                 mov     al, [edx]
		/*.text:00406C28*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, al
		/*.text:00406C2B*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406C2E*/                 add     ecx, 1
		/*.text:00406C31*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00406C34*/ 
		/*.text:00406C34*/ loc_406C34:                             ; CODE XREF: ___strgtold12+6A1.j
		/*.text:00406C34*/                 cmp     ___mb_cur_max, 1
		/*.text:00406C3B*/                 jle     short loc_406C59
		/*.text:00406C3D*/                 push    4               ; int
		/*.text:00406C3F*/                 mov     edx, /*[ebp+*/ var_3C /*]*/
		/*.text:00406C42*/                 and     edx, 0FFh
		/*.text:00406C48*/                 push    edx             ; int
		/*.text:00406C49*/                 call    __isctype
		/*.text:00406C4E*/                 add     esp, 8
		/*.text:00406C51*/                 mov     /*[ebp+*/ var_AC /*]*/, eax
		/*.text:00406C57*/                 jmp     short loc_406C76
		/*.text:00406C59*/ ; ---------------------------------------------------------------------------
		/*.text:00406C59*/ 
		/*.text:00406C59*/ loc_406C59:                             ; CODE XREF: ___strgtold12+6BB.j
		/*.text:00406C59*/                 mov     eax, /*[ebp+*/ var_3C /*]*/
		/*.text:00406C5C*/                 and     eax, 0FFh
		/*.text:00406C61*/                 mov     ecx, __pctype
		/*.text:00406C67*/                 xor     edx, edx
		/*.text:00406C69*/                 mov     dx, [ecx+eax*2]
		/*.text:00406C6D*/                 and     edx, 4
		/*.text:00406C70*/                 mov     /*[ebp+*/ var_AC /*]*/, edx
		/*.text:00406C76*/ 
		/*.text:00406C76*/ loc_406C76:                             ; CODE XREF: ___strgtold12+6D7.j
		/*.text:00406C76*/                 cmp     /*[ebp+*/ var_AC /*]*/, 0
		/*.text:00406C7D*/                 jz      short loc_406CA7
		/*.text:00406C7F*/                 mov     eax, /*[ebp+*/ var_7C /*]*/
		/*.text:00406C82*/                 imul    eax, 0Ah
		/*.text:00406C85*/                 movsx   ecx, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406C89*/                 lea     edx, [eax+ecx-30h]
		/*.text:00406C8D*/                 mov     /*[ebp+*/ var_7C /*]*/, edx
		/*.text:00406C90*/                 cmp     /*[ebp+*/ var_7C /*]*/, 1450h
		/*.text:00406C97*/                 jle     short loc_406CA2
		/*.text:00406C99*/                 mov     /*[ebp+*/ var_7C /*]*/, 1451h
		/*.text:00406CA0*/                 jmp     short loc_406CA7
		/*.text:00406CA2*/ ; ---------------------------------------------------------------------------
		/*.text:00406CA2*/ 
		/*.text:00406CA2*/ loc_406CA2:                             ; CODE XREF: ___strgtold12+717.j
		/*.text:00406CA2*/                 jmp     loc_406C23
		/*.text:00406CA7*/ ; ---------------------------------------------------------------------------
		/*.text:00406CA7*/ 
		/*.text:00406CA7*/ loc_406CA7:                             ; CODE XREF: ___strgtold12+6FD.j
		/*.text:00406CA7*/                                         ; ___strgtold12+720.j
		/*.text:00406CA7*/                 mov     eax, /*[ebp+*/ var_7C /*]*/
		/*.text:00406CAA*/                 mov     /*[ebp+*/ var_14 /*]*/, eax
		/*.text:00406CAD*/                 jmp     short loc_406CC0
		/*.text:00406CAF*/ ; ---------------------------------------------------------------------------
		/*.text:00406CAF*/ 
		/*.text:00406CAF*/ loc_406CAF:                             ; CODE XREF: ___strgtold12+78B.j
		/*.text:00406CAF*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406CB2*/                 mov     dl, [ecx]
		/*.text:00406CB4*/                 mov     byte ptr /*[ebp+*/ var_3C /*]*/, dl
		/*.text:00406CB7*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406CBA*/                 add     eax, 1
		/*.text:00406CBD*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406CC0*/ 
		/*.text:00406CC0*/ loc_406CC0:                             ; CODE XREF: ___strgtold12+72D.j
		/*.text:00406CC0*/                 cmp     ___mb_cur_max, 1
		/*.text:00406CC7*/                 jle     short loc_406CE5
		/*.text:00406CC9*/                 push    4               ; int
		/*.text:00406CCB*/                 mov     ecx, /*[ebp+*/ var_3C /*]*/
		/*.text:00406CCE*/                 and     ecx, 0FFh
		/*.text:00406CD4*/                 push    ecx             ; int
		/*.text:00406CD5*/                 call    __isctype
		/*.text:00406CDA*/                 add     esp, 8
		/*.text:00406CDD*/                 mov     /*[ebp+*/ var_B0 /*]*/, eax
		/*.text:00406CE3*/                 jmp     short loc_406D02
		/*.text:00406CE5*/ ; ---------------------------------------------------------------------------
		/*.text:00406CE5*/ 
		/*.text:00406CE5*/ loc_406CE5:                             ; CODE XREF: ___strgtold12+747.j
		/*.text:00406CE5*/                 mov     edx, /*[ebp+*/ var_3C /*]*/
		/*.text:00406CE8*/                 and     edx, 0FFh
		/*.text:00406CEE*/                 mov     eax, __pctype
		/*.text:00406CF3*/                 xor     ecx, ecx
		/*.text:00406CF5*/                 mov     cx, [eax+edx*2]
		/*.text:00406CF9*/                 and     ecx, 4
		/*.text:00406CFC*/                 mov     /*[ebp+*/ var_B0 /*]*/, ecx
		/*.text:00406D02*/ 
		/*.text:00406D02*/ loc_406D02:                             ; CODE XREF: ___strgtold12+763.j
		/*.text:00406D02*/                 cmp     /*[ebp+*/ var_B0 /*]*/, 0
		/*.text:00406D09*/                 jz      short loc_406D0D
		/*.text:00406D0B*/                 jmp     short loc_406CAF
		/*.text:00406D0D*/ ; ---------------------------------------------------------------------------
		/*.text:00406D0D*/ 
		/*.text:00406D0D*/ loc_406D0D:                             ; CODE XREF: ___strgtold12+789.j
		/*.text:00406D0D*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406D14*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406D17*/                 sub     edx, 1
		/*.text:00406D1A*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406D1D*/                 jmp     short loc_406D83
		/*.text:00406D1F*/ ; ---------------------------------------------------------------------------
		/*.text:00406D1F*/ 
		/*.text:00406D1F*/ loc_406D1F:                             ; CODE XREF: ___strgtold12+D5.j
		/*.text:00406D1F*/                                         ; DATA XREF: .text:00406FC6.o
		/*.text:00406D1F*/                 cmp     /*[ebp+*/ arg_18 /*]*/, 0
		/*.text:00406D23*/                 jz      short loc_406D73
		/*.text:00406D25*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406D28*/                 sub     eax, 1
		/*.text:00406D2B*/                 mov     /*[ebp+*/ var_64 /*]*/, eax
		/*.text:00406D2E*/                 mov     cl, byte ptr /*[ebp+*/ var_3C /*]*/
		/*.text:00406D31*/                 mov     /*[ebp+*/ var_B4 /*]*/, cl
		/*.text:00406D37*/                 cmp     /*[ebp+*/ var_B4 /*]*/, 2Bh
		/*.text:00406D3E*/                 jz      short loc_406D5B
		/*.text:00406D40*/                 cmp     /*[ebp+*/ var_B4 /*]*/, 2Dh
		/*.text:00406D47*/                 jz      short loc_406D4B
		/*.text:00406D49*/                 jmp     short loc_406D64
		/*.text:00406D4B*/ ; ---------------------------------------------------------------------------
		/*.text:00406D4B*/ 
		/*.text:00406D4B*/ loc_406D4B:                             ; CODE XREF: ___strgtold12+7C7.j
		/*.text:00406D4B*/                 mov     /*[ebp+*/ var_4C /*]*/, 7
		/*.text:00406D52*/                 mov     /*[ebp+*/ var_74 /*]*/, 0FFFFFFFFh
		/*.text:00406D59*/                 jmp     short loc_406D71
		/*.text:00406D5B*/ ; ---------------------------------------------------------------------------
		/*.text:00406D5B*/ 
		/*.text:00406D5B*/ loc_406D5B:                             ; CODE XREF: ___strgtold12+7BE.j
		/*.text:00406D5B*/                 mov     /*[ebp+*/ var_4C /*]*/, 7
		/*.text:00406D62*/                 jmp     short loc_406D71
		/*.text:00406D64*/ ; ---------------------------------------------------------------------------
		/*.text:00406D64*/ 
		/*.text:00406D64*/ loc_406D64:                             ; CODE XREF: ___strgtold12+7C9.j
		/*.text:00406D64*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406D6B*/                 mov     edx, /*[ebp+*/ var_64 /*]*/
		/*.text:00406D6E*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406D71*/ 
		/*.text:00406D71*/ loc_406D71:                             ; CODE XREF: ___strgtold12+7D9.j
		/*.text:00406D71*/                                         ; ___strgtold12+7E2.j
		/*.text:00406D71*/                 jmp     short loc_406D83
		/*.text:00406D73*/ ; ---------------------------------------------------------------------------
		/*.text:00406D73*/ 
		/*.text:00406D73*/ loc_406D73:                             ; CODE XREF: ___strgtold12+7A3.j
		/*.text:00406D73*/                 mov     /*[ebp+*/ var_4C /*]*/, 0Ah
		/*.text:00406D7A*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406D7D*/                 sub     eax, 1
		/*.text:00406D80*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406D83*/ 
		/*.text:00406D83*/ loc_406D83:                             ; CODE XREF: ___strgtold12+CC.j
		/*.text:00406D83*/                                         ; ___strgtold12+D5.j ...
		/*.text:00406D83*/                 jmp     loc_406627
		/*.text:00406D88*/ ; ---------------------------------------------------------------------------
		/*.text:00406D88*/ 
		/*.text:00406D88*/ loc_406D88:                             ; CODE XREF: ___strgtold12+AB.j
		/*.text:00406D88*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00406D8B*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406D8E*/                 mov     [ecx], edx
		/*.text:00406D90*/                 cmp     /*[ebp+*/ var_54 /*]*/, 0
		/*.text:00406D94*/                 jz      loc_406ED7
		/*.text:00406D9A*/                 cmp     /*[ebp+*/ var_40 /*]*/, 0
		/*.text:00406D9E*/                 jnz     loc_406ED7
		/*.text:00406DA4*/                 cmp     /*[ebp+*/ var_78 /*]*/, 0
		/*.text:00406DA8*/                 jnz     loc_406ED7
		/*.text:00406DAE*/                 cmp     /*[ebp+*/ var_70 /*]*/, 18h
		/*.text:00406DB2*/                 jbe     short loc_406DDF
		/*.text:00406DB4*/                 movsx   eax, /*[ebp+*/ var_21 /*]*/
		/*.text:00406DB8*/                 cmp     eax, 5
		/*.text:00406DBB*/                 jl      short loc_406DC6
		/*.text:00406DBD*/                 mov     cl, /*[ebp+*/ var_21 /*]*/
		/*.text:00406DC0*/                 add     cl, 1
		/*.text:00406DC3*/                 mov     /*[ebp+*/ var_21 /*]*/, cl
		/*.text:00406DC6*/ 
		/*.text:00406DC6*/ loc_406DC6:                             ; CODE XREF: ___strgtold12+83B.j
		/*.text:00406DC6*/                 mov     /*[ebp+*/ var_70 /*]*/, 18h
		/*.text:00406DCD*/                 mov     edx, /*[ebp+*/ var_68 /*]*/
		/*.text:00406DD0*/                 sub     edx, 1
		/*.text:00406DD3*/                 mov     /*[ebp+*/ var_68 /*]*/, edx
		/*.text:00406DD6*/                 mov     eax, /*[ebp+*/ var_6C /*]*/
		/*.text:00406DD9*/                 add     eax, 1
		/*.text:00406DDC*/                 mov     /*[ebp+*/ var_6C /*]*/, eax
		/*.text:00406DDF*/ 
		/*.text:00406DDF*/ loc_406DDF:                             ; CODE XREF: ___strgtold12+832.j
		/*.text:00406DDF*/                 cmp     /*[ebp+*/ var_70 /*]*/, 0
		/*.text:00406DE3*/                 jbe     loc_406EBA
		/*.text:00406DE9*/                 mov     ecx, /*[ebp+*/ var_68 /*]*/
		/*.text:00406DEC*/                 sub     ecx, 1
		/*.text:00406DEF*/                 mov     /*[ebp+*/ var_68 /*]*/, ecx
		/*.text:00406DF2*/                 jmp     short loc_406DFD
		/*.text:00406DF4*/ ; ---------------------------------------------------------------------------
		/*.text:00406DF4*/ 
		/*.text:00406DF4*/ loc_406DF4:                             ; CODE XREF: ___strgtold12+899.j
		/*.text:00406DF4*/                 mov     edx, /*[ebp+*/ var_68 /*]*/
		/*.text:00406DF7*/                 sub     edx, 1
		/*.text:00406DFA*/                 mov     /*[ebp+*/ var_68 /*]*/, edx
		/*.text:00406DFD*/ 
		/*.text:00406DFD*/ loc_406DFD:                             ; CODE XREF: ___strgtold12+872.j
		/*.text:00406DFD*/                 mov     eax, /*[ebp+*/ var_68 /*]*/
		/*.text:00406E00*/                 movsx   ecx, byte ptr [eax]
		/*.text:00406E03*/                 test    ecx, ecx
		/*.text:00406E05*/                 jnz     short loc_406E1B
		/*.text:00406E07*/                 mov     edx, /*[ebp+*/ var_70 /*]*/
		/*.text:00406E0A*/                 sub     edx, 1
		/*.text:00406E0D*/                 mov     /*[ebp+*/ var_70 /*]*/, edx
		/*.text:00406E10*/                 mov     eax, /*[ebp+*/ var_6C /*]*/
		/*.text:00406E13*/                 add     eax, 1
		/*.text:00406E16*/                 mov     /*[ebp+*/ var_6C /*]*/, eax
		/*.text:00406E19*/                 jmp     short loc_406DF4
		/*.text:00406E1B*/ ; ---------------------------------------------------------------------------
		/*.text:00406E1B*/ 
		/*.text:00406E1B*/ loc_406E1B:                             ; CODE XREF: ___strgtold12+885.j
		/*.text:00406E1B*/                 lea     ecx, /*[ebp+*/ var_60 /*]*/
		/*.text:00406E1E*/                 push    ecx
		/*.text:00406E1F*/                 mov     edx, /*[ebp+*/ var_70 /*]*/
		/*.text:00406E22*/                 push    edx
		/*.text:00406E23*/                 lea     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:00406E26*/                 push    eax
		/*.text:00406E27*/                 call    ___mtold12
		/*.text:00406E2C*/                 add     esp, 0Ch
		/*.text:00406E2F*/                 cmp     /*[ebp+*/ var_74 /*]*/, 0
		/*.text:00406E33*/                 jge     short loc_406E3D
		/*.text:00406E35*/                 mov     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:00406E38*/                 neg     ecx
		/*.text:00406E3A*/                 mov     /*[ebp+*/ var_14 /*]*/, ecx
		/*.text:00406E3D*/ 
		/*.text:00406E3D*/ loc_406E3D:                             ; CODE XREF: ___strgtold12+8B3.j
		/*.text:00406E3D*/                 mov     edx, /*[ebp+*/ var_14 /*]*/
		/*.text:00406E40*/                 add     edx, /*[ebp+*/ var_6C /*]*/
		/*.text:00406E43*/                 mov     /*[ebp+*/ var_14 /*]*/, edx
		/*.text:00406E46*/                 cmp     /*[ebp+*/ var_18 /*]*/, 0
		/*.text:00406E4A*/                 jnz     short loc_406E55
		/*.text:00406E4C*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00406E4F*/                 add     eax, /*[ebp+*/ arg_10 /*]*/
		/*.text:00406E52*/                 mov     /*[ebp+*/ var_14 /*]*/, eax
		/*.text:00406E55*/ 
		/*.text:00406E55*/ loc_406E55:                             ; CODE XREF: ___strgtold12+8CA.j
		/*.text:00406E55*/                 cmp     /*[ebp+*/ var_C /*]*/, 0
		/*.text:00406E59*/                 jnz     short loc_406E64
		/*.text:00406E5B*/                 mov     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:00406E5E*/                 sub     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00406E61*/                 mov     /*[ebp+*/ var_14 /*]*/, ecx
		/*.text:00406E64*/ 
		/*.text:00406E64*/ loc_406E64:                             ; CODE XREF: ___strgtold12+8D9.j
		/*.text:00406E64*/                 cmp     /*[ebp+*/ var_14 /*]*/, 1450h
		/*.text:00406E6B*/                 jle     short loc_406E76
		/*.text:00406E6D*/                 mov     /*[ebp+*/ var_40 /*]*/, 1
		/*.text:00406E74*/                 jmp     short loc_406EB8
		/*.text:00406E76*/ ; ---------------------------------------------------------------------------
		/*.text:00406E76*/ 
		/*.text:00406E76*/ loc_406E76:                             ; CODE XREF: ___strgtold12+8EB.j
		/*.text:00406E76*/                 cmp     /*[ebp+*/ var_14 /*]*/, 0FFFFEBB0h
		/*.text:00406E7D*/                 jge     short loc_406E88
		/*.text:00406E7F*/                 mov     /*[ebp+*/ var_78 /*]*/, 1
		/*.text:00406E86*/                 jmp     short loc_406EB8
		/*.text:00406E88*/ ; ---------------------------------------------------------------------------
		/*.text:00406E88*/ 
		/*.text:00406E88*/ loc_406E88:                             ; CODE XREF: ___strgtold12+8FD.j
		/*.text:00406E88*/                 mov     edx, /*[ebp+*/ arg_C /*]*/
		/*.text:00406E8B*/                 push    edx
		/*.text:00406E8C*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00406E8F*/                 push    eax
		/*.text:00406E90*/                 lea     ecx, /*[ebp+*/ var_60 /*]*/
		/*.text:00406E93*/                 push    ecx
		/*.text:00406E94*/                 call    ___multtenpow12
		/*.text:00406E99*/                 add     esp, 0Ch
		/*.text:00406E9C*/                 mov     dx, /*[ebp+*/ var_60 /*]*/
		/*.text:00406EA0*/                 mov     /*[ebp+*/ var_48 /*]*/, dx
		/*.text:00406EA4*/                 mov     eax, /*[ebp+*/ var_5E /*]*/
		/*.text:00406EA7*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:00406EAA*/                 mov     ecx, /*[ebp+*/ var_5A /*]*/
		/*.text:00406EAD*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:00406EB0*/                 mov     dx, /*[ebp+*/ var_56 /*]*/
		/*.text:00406EB4*/                 mov     word ptr /*[ebp+*/ var_50 /*]*/, dx
		/*.text:00406EB8*/ 
		/*.text:00406EB8*/ loc_406EB8:                             ; CODE XREF: ___strgtold12+8F4.j
		/*.text:00406EB8*/                                         ; ___strgtold12+906.j
		/*.text:00406EB8*/                 jmp     short loc_406ED7
		/*.text:00406EBA*/ ; ---------------------------------------------------------------------------
		/*.text:00406EBA*/ 
		/*.text:00406EBA*/ loc_406EBA:                             ; CODE XREF: ___strgtold12+863.j
		/*.text:00406EBA*/                 mov     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:00406EC0*/                 mov     word ptr /*[ebp+*/ var_50 /*]*/, 0
		/*.text:00406EC6*/                 mov     eax, /*[ebp+*/ var_50 /*]*/
		/*.text:00406EC9*/                 and     eax, 0FFFFh
		/*.text:00406ECE*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:00406ED1*/                 mov     ecx, /*[ebp+*/ var_10 /*]*/
		/*.text:00406ED4*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:00406ED7*/ 
		/*.text:00406ED7*/ loc_406ED7:                             ; CODE XREF: ___strgtold12+814.j
		/*.text:00406ED7*/                                         ; ___strgtold12+81E.j ...
		/*.text:00406ED7*/                 cmp     /*[ebp+*/ var_54 /*]*/, 0
		/*.text:00406EDB*/                 jnz     short loc_406F06
		/*.text:00406EDD*/                 mov     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:00406EE3*/                 mov     word ptr /*[ebp+*/ var_50 /*]*/, 0
		/*.text:00406EE9*/                 mov     edx, /*[ebp+*/ var_50 /*]*/
		/*.text:00406EEC*/                 and     edx, 0FFFFh
		/*.text:00406EF2*/                 mov     /*[ebp+*/ var_10 /*]*/, edx
		/*.text:00406EF5*/                 mov     eax, /*[ebp+*/ var_10 /*]*/
		/*.text:00406EF8*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:00406EFB*/                 mov     ecx, /*[ebp+*/ var_44 /*]*/
		/*.text:00406EFE*/                 or      ecx, 4
		/*.text:00406F01*/                 mov     /*[ebp+*/ var_44 /*]*/, ecx
		/*.text:00406F04*/                 jmp     short loc_406F5D
		/*.text:00406F06*/ ; ---------------------------------------------------------------------------
		/*.text:00406F06*/ 
		/*.text:00406F06*/ loc_406F06:                             ; CODE XREF: ___strgtold12+95B.j
		/*.text:00406F06*/                 cmp     /*[ebp+*/ var_40 /*]*/, 0
		/*.text:00406F0A*/                 jz      short loc_406F31
		/*.text:00406F0C*/                 mov     word ptr /*[ebp+*/ var_50 /*]*/, 7FFFh
		/*.text:00406F12*/                 mov     /*[ebp+*/ var_10 /*]*/, 80000000h
		/*.text:00406F19*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:00406F20*/                 mov     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:00406F26*/                 mov     edx, /*[ebp+*/ var_44 /*]*/
		/*.text:00406F29*/                 or      edx, 2
		/*.text:00406F2C*/                 mov     /*[ebp+*/ var_44 /*]*/, edx
		/*.text:00406F2F*/                 jmp     short loc_406F5D
		/*.text:00406F31*/ ; ---------------------------------------------------------------------------
		/*.text:00406F31*/ 
		/*.text:00406F31*/ loc_406F31:                             ; CODE XREF: ___strgtold12+98A.j
		/*.text:00406F31*/                 cmp     /*[ebp+*/ var_78 /*]*/, 0
		/*.text:00406F35*/                 jz      short loc_406F5D
		/*.text:00406F37*/                 mov     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:00406F3D*/                 mov     word ptr /*[ebp+*/ var_50 /*]*/, 0
		/*.text:00406F43*/                 mov     eax, /*[ebp+*/ var_50 /*]*/
		/*.text:00406F46*/                 and     eax, 0FFFFh
		/*.text:00406F4B*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:00406F4E*/                 mov     ecx, /*[ebp+*/ var_10 /*]*/
		/*.text:00406F51*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:00406F54*/                 mov     edx, /*[ebp+*/ var_44 /*]*/
		/*.text:00406F57*/                 or      edx, 1
		/*.text:00406F5A*/                 mov     /*[ebp+*/ var_44 /*]*/, edx
		/*.text:00406F5D*/ 
		/*.text:00406F5D*/ loc_406F5D:                             ; CODE XREF: ___strgtold12+984.j
		/*.text:00406F5D*/                                         ; ___strgtold12+9AF.j ...
		/*.text:00406F5D*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406F60*/                 mov     cx, /*[ebp+*/ var_48 /*]*/
		/*.text:00406F64*/                 mov     [eax], cx
		/*.text:00406F67*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406F6A*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00406F6D*/                 mov     [edx+2], eax
		/*.text:00406F70*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406F73*/                 mov     edx, /*[ebp+*/ var_10 /*]*/
		/*.text:00406F76*/                 mov     [ecx+6], edx
		/*.text:00406F79*/                 mov     eax, /*[ebp+*/ var_50 /*]*/
		/*.text:00406F7C*/                 and     eax, 0FFFFh
		/*.text:00406F81*/                 mov     ecx, /*[ebp+*/ var_1C /*]*/
		/*.text:00406F84*/                 and     ecx, 0FFFFh
		/*.text:00406F8A*/                 or      eax, ecx
		/*.text:00406F8C*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406F8F*/                 mov     [edx+0Ah], ax
		/*.text:00406F93*/                 mov     eax, /*[ebp+*/ var_44 /*]*/
		/*.text:00406F96*/                 mov     esp, ebp
		/*.text:00406F98*/                 pop     ebp
		/*.text:00406F99*/                 retn
		/*.text:00406F99*/ // ___strgtold12   endp
	}
}
#undef var_B4
#undef var_B0
#undef var_AC
#undef var_A8
#undef var_A4
#undef var_A0
#undef var_9C
#undef var_98
#undef var_94
#undef var_90
#undef var_8C
#undef var_88
#undef var_84
#undef var_80
#undef var_7C
#undef var_78
#undef var_74
#undef var_70
#undef var_6C
#undef var_68
#undef var_64
#undef var_60
#undef var_5E
#undef var_5A
#undef var_56
#undef var_54
#undef var_50
#undef var_4C
#undef var_48
#undef var_44
#undef var_40
#undef var_3C
#undef var_38
#undef var_21
#undef var_1C
#undef var_18
#undef var_14
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 
#undef arg_8 
#undef arg_C 
#undef arg_10
#undef arg_14
#undef arg_18

static VOID __declspec( naked ) __CopyMan( VOID )
{
	__asm
	{
		/*.text:00405FC0*/ // __CopyMan       proc near               ; CODE XREF: __ld12cvt+98.p
		/*.text:00405FC0*/                                         ; __ld12cvt+10F.p
		/*.text:00405FC0*/ 
		/*.text:00405FC0*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:00405FC0*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:00405FC0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00405FC0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00405FC0*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00405FC0*/ 
		/*.text:00405FC0*/                 push    ebp
		/*.text:00405FC1*/                 mov     ebp, esp
		/*.text:00405FC3*/                 sub     esp, 0Ch
		/*.text:00405FC6*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00405FC9*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00405FCC*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405FCF*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:00405FD2*/                 mov     /*[ebp+*/ var_C /*]*/, 0
		/*.text:00405FD9*/                 jmp     short loc_405FE4
		/*.text:00405FDB*/ ; ---------------------------------------------------------------------------
		/*.text:00405FDB*/ 
		/*.text:00405FDB*/ loc_405FDB:                             ; CODE XREF: __CopyMan+46.j
		/*.text:00405FDB*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:00405FDE*/                 add     edx, 1
		/*.text:00405FE1*/                 mov     /*[ebp+*/ var_C /*]*/, edx
		/*.text:00405FE4*/ 
		/*.text:00405FE4*/ loc_405FE4:                             ; CODE XREF: __CopyMan+19.j
		/*.text:00405FE4*/                 cmp     /*[ebp+*/ var_C /*]*/, 3
		/*.text:00405FE8*/                 jge     short loc_406008
		/*.text:00405FEA*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00405FED*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00405FF0*/                 mov     edx, [ecx]
		/*.text:00405FF2*/                 mov     [eax], edx
		/*.text:00405FF4*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00405FF7*/                 add     eax, 4
		/*.text:00405FFA*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:00405FFD*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406000*/                 add     ecx, 4
		/*.text:00406003*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00406006*/                 jmp     short loc_405FDB
		/*.text:00406008*/ ; ---------------------------------------------------------------------------
		/*.text:00406008*/ 
		/*.text:00406008*/ loc_406008:                             ; CODE XREF: __CopyMan+28.j
		/*.text:00406008*/                 mov     esp, ebp
		/*.text:0040600A*/                 pop     ebp
		/*.text:0040600B*/                 retn
		/*.text:0040600B*/ // __CopyMan       endp
	}
}
#undef var_C
#undef var_8
#undef var_4
#undef arg_0
#undef arg_4

static VOID __declspec( naked ) __FillZeroMan( VOID )
{
	__asm
	{
		/*.text:00406010*/ // __FillZeroMan   proc near               ; CODE XREF: __ld12cvt+7C.p
		/*.text:00406010*/                                         ; __ld12cvt+D5.p ...
		/*.text:00406010*/ 
		/*.text:00406010*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00406010*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00406010*/ 
		/*.text:00406010*/                 push    ebp
		/*.text:00406011*/                 mov     ebp, esp
		/*.text:00406013*/                 push    ecx
		/*.text:00406014*/                 mov     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0040601B*/                 jmp     short loc_406026
		/*.text:0040601D*/ ; ---------------------------------------------------------------------------
		/*.text:0040601D*/ 
		/*.text:0040601D*/ loc_40601D:                             ; CODE XREF: __FillZeroMan+29.j
		/*.text:0040601D*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406020*/                 add     eax, 1
		/*.text:00406023*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406026*/ 
		/*.text:00406026*/ loc_406026:                             ; CODE XREF: __FillZeroMan+B.j
		/*.text:00406026*/                 cmp     /*[ebp+*/ var_4 /*]*/, 3
		/*.text:0040602A*/                 jge     short loc_40603B
		/*.text:0040602C*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040602F*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406032*/                 mov     dword ptr [edx+ecx*4], 0
		/*.text:00406039*/                 jmp     short loc_40601D
		/*.text:0040603B*/ ; ---------------------------------------------------------------------------
		/*.text:0040603B*/ 
		/*.text:0040603B*/ loc_40603B:                             ; CODE XREF: __FillZeroMan+1A.j
		/*.text:0040603B*/                 mov     esp, ebp
		/*.text:0040603D*/                 pop     ebp
		/*.text:0040603E*/                 retn
		/*.text:0040603E*/ // __FillZeroMan   endp
	}
}
#undef var_4
#undef arg_0

static VOID __declspec( naked ) __IsZeroMan( VOID )
{
	__asm
	{
		/*.text:00406040*/ // __IsZeroMan     proc near               ; CODE XREF: __ld12cvt+63.p
		/*.text:00406040*/ 
		/*.text:00406040*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00406040*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00406040*/ 
		/*.text:00406040*/                 push    ebp
		/*.text:00406041*/                 mov     ebp, esp
		/*.text:00406043*/                 push    ecx
		/*.text:00406044*/                 mov     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0040604B*/                 jmp     short loc_406056
		/*.text:0040604D*/ ; ---------------------------------------------------------------------------
		/*.text:0040604D*/ 
		/*.text:0040604D*/ loc_40604D:                             ; CODE XREF: __IsZeroMan+2C.j
		/*.text:0040604D*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00406050*/                 add     eax, 1
		/*.text:00406053*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00406056*/ 
		/*.text:00406056*/ loc_406056:                             ; CODE XREF: __IsZeroMan+B.j
		/*.text:00406056*/                 cmp     /*[ebp+*/ var_4 /*]*/, 3
		/*.text:0040605A*/                 jge     short loc_40606E
		/*.text:0040605C*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040605F*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406062*/                 cmp     dword ptr [edx+ecx*4], 0
		/*.text:00406066*/                 jz      short loc_40606C
		/*.text:00406068*/                 xor     eax, eax
		/*.text:0040606A*/                 jmp     short loc_406073
		/*.text:0040606C*/ ; ---------------------------------------------------------------------------
		/*.text:0040606C*/ 
		/*.text:0040606C*/ loc_40606C:                             ; CODE XREF: __IsZeroMan+26.j
		/*.text:0040606C*/                 jmp     short loc_40604D
		/*.text:0040606E*/ ; ---------------------------------------------------------------------------
		/*.text:0040606E*/ 
		/*.text:0040606E*/ loc_40606E:                             ; CODE XREF: __IsZeroMan+1A.j
		/*.text:0040606E*/                 mov     eax, 1
		/*.text:00406073*/ 
		/*.text:00406073*/ loc_406073:                             ; CODE XREF: __IsZeroMan+2A.j
		/*.text:00406073*/                 mov     esp, ebp
		/*.text:00406075*/                 pop     ebp
		/*.text:00406076*/                 retn
		/*.text:00406076*/ // __IsZeroMan     endp
	}
}
#undef var_4
#undef arg_0

static VOID __declspec( naked ) __ZeroTail( VOID )
{
	__asm
	{
		/*.text:00405DA0*/ // __ZeroTail      proc near               ; CODE XREF: __RoundMan+70.p
		/*.text:00405DA0*/ 
		/*.text:00405DA0*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:00405DA0*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:00405DA0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00405DA0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00405DA0*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00405DA0*/ 
		/*.text:00405DA0*/                 push    ebp
		/*.text:00405DA1*/                 mov     ebp, esp
		/*.text:00405DA3*/                 sub     esp, 0Ch
		/*.text:00405DA6*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00405DA9*/                 cdq
		/*.text:00405DAA*/                 and     edx, 1Fh
		/*.text:00405DAD*/                 add     eax, edx
		/*.text:00405DAF*/                 sar     eax, 5
		/*.text:00405DB2*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:00405DB5*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00405DB8*/                 and     eax, 8000001Fh
		/*.text:00405DBD*/                 jns     short loc_405DC4
		/*.text:00405DBF*/                 dec     eax
		/*.text:00405DC0*/                 or      eax, 0FFFFFFE0h
		/*.text:00405DC3*/                 inc     eax
		/*.text:00405DC4*/ 
		/*.text:00405DC4*/ loc_405DC4:                             ; CODE XREF: __ZeroTail+1D.j
		/*.text:00405DC4*/                 mov     ecx, 1Fh
		/*.text:00405DC9*/                 sub     ecx, eax
		/*.text:00405DCB*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00405DCE*/                 or      edx, 0FFFFFFFFh
		/*.text:00405DD1*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00405DD4*/                 shl     edx, cl
		/*.text:00405DD6*/                 not     edx
		/*.text:00405DD8*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00405DDB*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405DDE*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405DE1*/                 mov     edx, [ecx+eax*4]
		/*.text:00405DE4*/                 and     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:00405DE7*/                 test    edx, edx
		/*.text:00405DE9*/                 jz      short loc_405DEF
		/*.text:00405DEB*/                 xor     eax, eax
		/*.text:00405DED*/                 jmp     short loc_405E20
		/*.text:00405DEF*/ ; ---------------------------------------------------------------------------
		/*.text:00405DEF*/ 
		/*.text:00405DEF*/ loc_405DEF:                             ; CODE XREF: __ZeroTail+49.j
		/*.text:00405DEF*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405DF2*/                 add     eax, 1
		/*.text:00405DF5*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:00405DF8*/                 jmp     short loc_405E03
		/*.text:00405DFA*/ ; ---------------------------------------------------------------------------
		/*.text:00405DFA*/ 
		/*.text:00405DFA*/ loc_405DFA:                             ; CODE XREF: __ZeroTail+79.j
		/*.text:00405DFA*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00405DFD*/                 add     ecx, 1
		/*.text:00405E00*/                 mov     /*[ebp+*/ var_C /*]*/, ecx
		/*.text:00405E03*/ 
		/*.text:00405E03*/ loc_405E03:                             ; CODE XREF: __ZeroTail+58.j
		/*.text:00405E03*/                 cmp     /*[ebp+*/ var_C /*]*/, 3
		/*.text:00405E07*/                 jge     short loc_405E1B
		/*.text:00405E09*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:00405E0C*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405E0F*/                 cmp     dword ptr [eax+edx*4], 0
		/*.text:00405E13*/                 jz      short loc_405E19
		/*.text:00405E15*/                 xor     eax, eax
		/*.text:00405E17*/                 jmp     short loc_405E20
		/*.text:00405E19*/ ; ---------------------------------------------------------------------------
		/*.text:00405E19*/ 
		/*.text:00405E19*/ loc_405E19:                             ; CODE XREF: __ZeroTail+73.j
		/*.text:00405E19*/                 jmp     short loc_405DFA
		/*.text:00405E1B*/ ; ---------------------------------------------------------------------------
		/*.text:00405E1B*/ 
		/*.text:00405E1B*/ loc_405E1B:                             ; CODE XREF: __ZeroTail+67.j
		/*.text:00405E1B*/                 mov     eax, 1
		/*.text:00405E20*/ 
		/*.text:00405E20*/ loc_405E20:                             ; CODE XREF: __ZeroTail+4D.j
		/*.text:00405E20*/                                         ; __ZeroTail+77.j
		/*.text:00405E20*/                 mov     esp, ebp
		/*.text:00405E22*/                 pop     ebp
		/*.text:00405E23*/                 retn
		/*.text:00405E23*/ // __ZeroTail      endp
	}
}
#undef var_C
#undef var_8
#undef var_4
#undef arg_0
#undef arg_4

static VOID __declspec( naked ) __IncMan( VOID )
{
	__asm
	{
		/*.text:00405E30*/ // __IncMan        proc near               ; CODE XREF: __RoundMan+84.p
		/*.text:00405E30*/ 
		/*.text:00405E30*/ #define var_10           dword ptr [ebp-0x10]
		/*.text:00405E30*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:00405E30*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:00405E30*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00405E30*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00405E30*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00405E30*/ 
		/*.text:00405E30*/                 push    ebp
		/*.text:00405E31*/                 mov     ebp, esp
		/*.text:00405E33*/                 sub     esp, 10h
		/*.text:00405E36*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00405E39*/                 cdq
		/*.text:00405E3A*/                 and     edx, 1Fh
		/*.text:00405E3D*/                 add     eax, edx
		/*.text:00405E3F*/                 sar     eax, 5
		/*.text:00405E42*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:00405E45*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00405E48*/                 and     eax, 8000001Fh
		/*.text:00405E4D*/                 jns     short loc_405E54
		/*.text:00405E4F*/                 dec     eax
		/*.text:00405E50*/                 or      eax, 0FFFFFFE0h
		/*.text:00405E53*/                 inc     eax
		/*.text:00405E54*/ 
		/*.text:00405E54*/ loc_405E54:                             ; CODE XREF: __IncMan+1D.j
		/*.text:00405E54*/                 mov     ecx, 1Fh
		/*.text:00405E59*/                 sub     ecx, eax
		/*.text:00405E5B*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00405E5E*/                 mov     edx, 1
		/*.text:00405E63*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00405E66*/                 shl     edx, cl
		/*.text:00405E68*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00405E6B*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405E6E*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405E71*/                 lea     edx, [ecx+eax*4]
		/*.text:00405E74*/                 push    edx
		/*.text:00405E75*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00405E78*/                 push    eax
		/*.text:00405E79*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00405E7C*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405E7F*/                 mov     eax, [edx+ecx*4]
		/*.text:00405E82*/                 push    eax
		/*.text:00405E83*/                 call    ___addl
		/*.text:00405E88*/                 add     esp, 0Ch
		/*.text:00405E8B*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:00405E8E*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00405E91*/                 sub     ecx, 1
		/*.text:00405E94*/                 mov     /*[ebp+*/ var_C /*]*/, ecx
		/*.text:00405E97*/                 jmp     short loc_405EA2
		/*.text:00405E99*/ ; ---------------------------------------------------------------------------
		/*.text:00405E99*/ 
		/*.text:00405E99*/ loc_405E99:                             ; CODE XREF: __IncMan+9F.j
		/*.text:00405E99*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:00405E9C*/                 sub     edx, 1
		/*.text:00405E9F*/                 mov     /*[ebp+*/ var_C /*]*/, edx
		/*.text:00405EA2*/ 
		/*.text:00405EA2*/ loc_405EA2:                             ; CODE XREF: __IncMan+67.j
		/*.text:00405EA2*/                 cmp     /*[ebp+*/ var_C /*]*/, 0
		/*.text:00405EA6*/                 jl      short loc_405ED1
		/*.text:00405EA8*/                 cmp     /*[ebp+*/ var_10 /*]*/, 0
		/*.text:00405EAC*/                 jz      short loc_405ED1
		/*.text:00405EAE*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405EB1*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405EB4*/                 lea     edx, [ecx+eax*4]
		/*.text:00405EB7*/                 push    edx
		/*.text:00405EB8*/                 push    1
		/*.text:00405EBA*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405EBD*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405EC0*/                 mov     edx, [ecx+eax*4]
		/*.text:00405EC3*/                 push    edx
		/*.text:00405EC4*/                 call    ___addl
		/*.text:00405EC9*/                 add     esp, 0Ch
		/*.text:00405ECC*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:00405ECF*/                 jmp     short loc_405E99
		/*.text:00405ED1*/ ; ---------------------------------------------------------------------------
		/*.text:00405ED1*/ 
		/*.text:00405ED1*/ loc_405ED1:                             ; CODE XREF: __IncMan+76.j
		/*.text:00405ED1*/                                         ; __IncMan+7C.j
		/*.text:00405ED1*/                 mov     eax, /*[ebp+*/ var_10 /*]*/
		/*.text:00405ED4*/                 mov     esp, ebp
		/*.text:00405ED6*/                 pop     ebp
		/*.text:00405ED7*/                 retn
		/*.text:00405ED7*/ // __IncMan        endp
	}
}
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 

static VOID __declspec( naked ) __RoundMan( VOID )
{
	__asm
	{
		/*.text:00405EE0*/ // __RoundMan      proc near               ; CODE XREF: __ld12cvt+AB.p
		/*.text:00405EE0*/                                         ; __ld12cvt+132.p ...
		/*.text:00405EE0*/ 
		/*.text:00405EE0*/ #define var_1C           dword ptr [ebp-0x1C]
		/*.text:00405EE0*/ #define var_18           dword ptr [ebp-0x18]
		/*.text:00405EE0*/ #define var_14           dword ptr [ebp-0x14]
		/*.text:00405EE0*/ #define var_10           dword ptr [ebp-0x10]
		/*.text:00405EE0*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:00405EE0*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:00405EE0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00405EE0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00405EE0*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00405EE0*/ 
		/*.text:00405EE0*/                 push    ebp
		/*.text:00405EE1*/                 mov     ebp, esp
		/*.text:00405EE3*/                 sub     esp, 1Ch
		/*.text:00405EE6*/                 mov     /*[ebp+*/ var_18 /*]*/, 0
		/*.text:00405EED*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00405EF0*/                 sub     eax, 1
		/*.text:00405EF3*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00405EF6*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00405EF9*/                 add     ecx, 1
		/*.text:00405EFC*/                 mov     /*[ebp+*/ var_C /*]*/, ecx
		/*.text:00405EFF*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405F02*/                 cdq
		/*.text:00405F03*/                 and     edx, 1Fh
		/*.text:00405F06*/                 add     eax, edx
		/*.text:00405F08*/                 sar     eax, 5
		/*.text:00405F0B*/                 mov     /*[ebp+*/ var_14 /*]*/, eax
		/*.text:00405F0E*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:00405F11*/                 and     edx, 8000001Fh
		/*.text:00405F17*/                 jns     short loc_405F1E
		/*.text:00405F19*/                 dec     edx
		/*.text:00405F1A*/                 or      edx, 0FFFFFFE0h
		/*.text:00405F1D*/                 inc     edx
		/*.text:00405F1E*/ 
		/*.text:00405F1E*/ loc_405F1E:                             ; CODE XREF: __RoundMan+37.j
		/*.text:00405F1E*/                 mov     eax, 1Fh
		/*.text:00405F23*/                 sub     eax, edx
		/*.text:00405F25*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:00405F28*/                 mov     edx, 1
		/*.text:00405F2D*/                 mov     ecx, /*[ebp+*/ var_8 /*]*/
		/*.text:00405F30*/                 shl     edx, cl
		/*.text:00405F32*/                 mov     /*[ebp+*/ var_1C /*]*/, edx
		/*.text:00405F35*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00405F38*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405F3B*/                 mov     edx, [ecx+eax*4]
		/*.text:00405F3E*/                 and     edx, /*[ebp+*/ var_1C /*]*/
		/*.text:00405F41*/                 test    edx, edx
		/*.text:00405F43*/                 jz      short loc_405F6F
		/*.text:00405F45*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00405F48*/                 add     eax, 1
		/*.text:00405F4B*/                 push    eax
		/*.text:00405F4C*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405F4F*/                 push    ecx
		/*.text:00405F50*/                 call    __ZeroTail
		/*.text:00405F55*/                 add     esp, 8
		/*.text:00405F58*/                 test    eax, eax
		/*.text:00405F5A*/                 jnz     short loc_405F6F
		/*.text:00405F5C*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00405F5F*/                 push    edx
		/*.text:00405F60*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405F63*/                 push    eax
		/*.text:00405F64*/                 call    __IncMan
		/*.text:00405F69*/                 add     esp, 8
		/*.text:00405F6C*/                 mov     /*[ebp+*/ var_18 /*]*/, eax
		/*.text:00405F6F*/ 
		/*.text:00405F6F*/ loc_405F6F:                             ; CODE XREF: __RoundMan+63.j
		/*.text:00405F6F*/                                         ; __RoundMan+7A.j
		/*.text:00405F6F*/                 or      edx, 0FFFFFFFFh
		/*.text:00405F72*/                 mov     ecx, /*[ebp+*/ var_8 /*]*/
		/*.text:00405F75*/                 shl     edx, cl
		/*.text:00405F77*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00405F7A*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405F7D*/                 mov     eax, [ecx+eax*4]
		/*.text:00405F80*/                 and     eax, edx
		/*.text:00405F82*/                 mov     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:00405F85*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405F88*/                 mov     [edx+ecx*4], eax
		/*.text:00405F8B*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00405F8E*/                 add     eax, 1
		/*.text:00405F91*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:00405F94*/                 jmp     short loc_405F9F
		/*.text:00405F96*/ ; ---------------------------------------------------------------------------
		/*.text:00405F96*/ 
		/*.text:00405F96*/ loc_405F96:                             ; CODE XREF: __RoundMan+D2.j
		/*.text:00405F96*/                 mov     ecx, /*[ebp+*/ var_10 /*]*/
		/*.text:00405F99*/                 add     ecx, 1
		/*.text:00405F9C*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:00405F9F*/ 
		/*.text:00405F9F*/ loc_405F9F:                             ; CODE XREF: __RoundMan+B4.j
		/*.text:00405F9F*/                 cmp     /*[ebp+*/ var_10 /*]*/, 3
		/*.text:00405FA3*/                 jge     short loc_405FB4
		/*.text:00405FA5*/                 mov     edx, /*[ebp+*/ var_10 /*]*/
		/*.text:00405FA8*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00405FAB*/                 mov     dword ptr [eax+edx*4], 0
		/*.text:00405FB2*/                 jmp     short loc_405F96
		/*.text:00405FB4*/ ; ---------------------------------------------------------------------------
		/*.text:00405FB4*/ 
		/*.text:00405FB4*/ loc_405FB4:                             ; CODE XREF: __RoundMan+C3.j
		/*.text:00405FB4*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:00405FB7*/                 mov     esp, ebp
		/*.text:00405FB9*/                 pop     ebp
		/*.text:00405FBA*/                 retn
		/*.text:00405FBA*/ // __RoundMan      endp
	}
}
#undef var_1C
#undef var_18
#undef var_14
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 

static VOID __declspec( naked ) __ShrMan( VOID )
{
	__asm
	{
		/*.text:00406080*/ // __ShrMan        proc near               ; CODE XREF: __ld12cvt+11F.p
		/*.text:00406080*/                                         ; __ld12cvt+148.p ...
		/*.text:00406080*/ 
		/*.text:00406080*/ #define var_18           dword ptr [ebp-0x18]
		/*.text:00406080*/ #define var_14           dword ptr [ebp-0x14]
		/*.text:00406080*/ #define var_10           dword ptr [ebp-0x10]
		/*.text:00406080*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:00406080*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:00406080*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00406080*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:00406080*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00406080*/ 
		/*.text:00406080*/                 push    ebp
		/*.text:00406081*/                 mov     ebp, esp
		/*.text:00406083*/                 sub     esp, 18h
		/*.text:00406086*/                 push    esi
		/*.text:00406087*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040608A*/                 cdq
		/*.text:0040608B*/                 and     edx, 1Fh
		/*.text:0040608E*/                 add     eax, edx
		/*.text:00406090*/                 sar     eax, 5
		/*.text:00406093*/                 mov     /*[ebp+*/ var_18 /*]*/, eax
		/*.text:00406096*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00406099*/                 and     eax, 8000001Fh
		/*.text:0040609E*/                 jns     short loc_4060A5
		/*.text:004060A0*/                 dec     eax
		/*.text:004060A1*/                 or      eax, 0FFFFFFE0h
		/*.text:004060A4*/                 inc     eax
		/*.text:004060A5*/ 
		/*.text:004060A5*/ loc_4060A5:                             ; CODE XREF: __ShrMan+1E.j
		/*.text:004060A5*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004060A8*/                 or      edx, 0FFFFFFFFh
		/*.text:004060AB*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004060AE*/                 shl     edx, cl
		/*.text:004060B0*/                 not     edx
		/*.text:004060B2*/                 mov     /*[ebp+*/ var_14 /*]*/, edx
		/*.text:004060B5*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:004060BC*/                 mov     /*[ebp+*/ var_C /*]*/, 0
		/*.text:004060C3*/                 jmp     short loc_4060CE
		/*.text:004060C5*/ ; ---------------------------------------------------------------------------
		/*.text:004060C5*/ 
		/*.text:004060C5*/ loc_4060C5:                             ; CODE XREF: __ShrMan+9F.j
		/*.text:004060C5*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:004060C8*/                 add     eax, 1
		/*.text:004060CB*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:004060CE*/ 
		/*.text:004060CE*/ loc_4060CE:                             ; CODE XREF: __ShrMan+43.j
		/*.text:004060CE*/                 cmp     /*[ebp+*/ var_C /*]*/, 3
		/*.text:004060D2*/                 jge     short loc_406121
		/*.text:004060D4*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:004060D7*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004060DA*/                 mov     eax, [edx+ecx*4]
		/*.text:004060DD*/                 and     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:004060E0*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:004060E3*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:004060E6*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004060E9*/                 mov     eax, [edx+ecx*4]
		/*.text:004060EC*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004060EF*/                 shr     eax, cl
		/*.text:004060F1*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:004060F4*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004060F7*/                 mov     [edx+ecx*4], eax
		/*.text:004060FA*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:004060FD*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406100*/                 mov     edx, [ecx+eax*4]
		/*.text:00406103*/                 or      edx, /*[ebp+*/ var_8 /*]*/
		/*.text:00406106*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00406109*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040610C*/                 mov     [ecx+eax*4], edx
		/*.text:0040610F*/                 mov     ecx, 20h
		/*.text:00406114*/                 sub     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406117*/                 mov     edx, /*[ebp+*/ var_10 /*]*/
		/*.text:0040611A*/                 shl     edx, cl
		/*.text:0040611C*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:0040611F*/                 jmp     short loc_4060C5
		/*.text:00406121*/ ; ---------------------------------------------------------------------------
		/*.text:00406121*/ 
		/*.text:00406121*/ loc_406121:                             ; CODE XREF: __ShrMan+52.j
		/*.text:00406121*/                 mov     /*[ebp+*/ var_C /*]*/, 2
		/*.text:00406128*/                 jmp     short loc_406133
		/*.text:0040612A*/ ; ---------------------------------------------------------------------------
		/*.text:0040612A*/ 
		/*.text:0040612A*/ loc_40612A:                             ; CODE XREF: __ShrMan+E5.j
		/*.text:0040612A*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:0040612D*/                 sub     eax, 1
		/*.text:00406130*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:00406133*/ 
		/*.text:00406133*/ loc_406133:                             ; CODE XREF: __ShrMan+A8.j
		/*.text:00406133*/                 cmp     /*[ebp+*/ var_C /*]*/, 0
		/*.text:00406137*/                 jl      short loc_406167
		/*.text:00406139*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:0040613C*/                 cmp     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040613F*/                 jl      short loc_406158
		/*.text:00406141*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:00406144*/                 sub     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:00406147*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:0040614A*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040614D*/                 mov     esi, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406150*/                 mov     edx, [esi+edx*4]
		/*.text:00406153*/                 mov     [ecx+eax*4], edx
		/*.text:00406156*/                 jmp     short loc_406165
		/*.text:00406158*/ ; ---------------------------------------------------------------------------
		/*.text:00406158*/ 
		/*.text:00406158*/ loc_406158:                             ; CODE XREF: __ShrMan+BF.j
		/*.text:00406158*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:0040615B*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0040615E*/                 mov     dword ptr [ecx+eax*4], 0
		/*.text:00406165*/ 
		/*.text:00406165*/ loc_406165:                             ; CODE XREF: __ShrMan+D6.j
		/*.text:00406165*/                 jmp     short loc_40612A
		/*.text:00406167*/ ; ---------------------------------------------------------------------------
		/*.text:00406167*/ 
		/*.text:00406167*/ loc_406167:                             ; CODE XREF: __ShrMan+B7.j
		/*.text:00406167*/                 pop     esi
		/*.text:00406168*/                 mov     esp, ebp
		/*.text:0040616A*/                 pop     ebp
		/*.text:0040616B*/                 retn
		/*.text:0040616B*/ // __ShrMan        endp
	}
}
#undef var_18
#undef var_14
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 

static VOID __declspec( naked ) __ld12cvt( VOID )
{
	__asm
	{
		/*.text:00406170*/ // __ld12cvt       proc near               ; CODE XREF: __ld12tod+10.p
		/*.text:00406170*/                                         ; __ld12tof+10.p
		/*.text:00406170*/ 
		/*.text:00406170*/  #define var_34           dword ptr [ebp-0x34]
		/*.text:00406170*/  #define var_30           byte ptr [ebp-0x30]
		/*.text:00406170*/  #define var_24           dword ptr [ebp-0x24]
		/*.text:00406170*/  #define var_20           dword ptr [ebp-0x20]
		/*.text:00406170*/  #define var_1C           dword ptr [ebp-0x1C]
		/*.text:00406170*/  #define var_18           dword ptr [ebp-0x18]
		/*.text:00406170*/  #define var_14           dword ptr [ebp-0x14]
		/*.text:00406170*/  #define var_10           dword ptr [ebp-0x10]
		/*.text:00406170*/  #define var_C            dword ptr [ebp-0x0C]
		/*.text:00406170*/  #define var_8            dword ptr [ebp-0x8]
		/*.text:00406170*/  #define var_4            dword ptr [ebp-0x4]
		/*.text:00406170*/  #define arg_0            dword ptr [ebp+0x8]
		/*.text:00406170*/  #define arg_4            dword ptr [ebp+0x0C]
		/*.text:00406170*/  #define arg_8            dword ptr [ebp+0x10]
		/*.text:00406170*/ 
		/*.text:00406170*/                 push    ebp
		/*.text:00406171*/                 mov     ebp, esp
		/*.text:00406173*/                 sub     esp, 34h
		/*.text:00406176*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406179*/                 xor     ecx, ecx
		/*.text:0040617B*/                 mov     cx, [eax+0Ah]
		/*.text:0040617F*/                 and     ecx, 7FFFh
		/*.text:00406185*/                 sub     ecx, 3FFFh
		/*.text:0040618B*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0040618E*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00406191*/                 xor     eax, eax
		/*.text:00406193*/                 mov     ax, [edx+0Ah]
		/*.text:00406197*/                 and     eax, 8000h
		/*.text:0040619C*/                 mov     /*[ebp+*/ var_20 /*]*/, eax
		/*.text:0040619F*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004061A2*/                 mov     edx, [ecx+6]
		/*.text:004061A5*/                 mov     /*[ebp+*/ var_18 /*]*/, edx
		/*.text:004061A8*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004061AB*/                 mov     ecx, [eax+2]
		/*.text:004061AE*/                 mov     /*[ebp+*/ var_14 /*]*/, ecx
		/*.text:004061B1*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004061B4*/                 xor     eax, eax
		/*.text:004061B6*/                 mov     ax, [edx]
		/*.text:004061B9*/                 shl     eax, 10h
		/*.text:004061BC*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:004061BF*/                 cmp     /*[ebp+*/ var_4 /*]*/, 0FFFFC001h
		/*.text:004061C6*/                 jnz     short loc_406200
		/*.text:004061C8*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:004061CF*/                 lea     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:004061D2*/                 push    ecx
		/*.text:004061D3*/                 call    __IsZeroMan
		/*.text:004061D8*/                 add     esp, 4
		/*.text:004061DB*/                 test    eax, eax
		/*.text:004061DD*/                 jz      short loc_4061E8
		/*.text:004061DF*/                 mov     /*[ebp+*/ var_1C /*]*/, 0
		/*.text:004061E6*/                 jmp     short loc_4061FB
		/*.text:004061E8*/ ; ---------------------------------------------------------------------------
		/*.text:004061E8*/ 
		/*.text:004061E8*/ loc_4061E8:                             ; CODE XREF: __ld12cvt+6D.j
		/*.text:004061E8*/                 lea     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:004061EB*/                 push    edx
		/*.text:004061EC*/                 call    __FillZeroMan
		/*.text:004061F1*/                 add     esp, 4
		/*.text:004061F4*/                 mov     /*[ebp+*/ var_1C /*]*/, 2
		/*.text:004061FB*/ 
		/*.text:004061FB*/ loc_4061FB:                             ; CODE XREF: __ld12cvt+76.j
		/*.text:004061FB*/                 jmp     loc_40634C
		/*.text:00406200*/ ; ---------------------------------------------------------------------------
		/*.text:00406200*/ 
		/*.text:00406200*/ loc_406200:                             ; CODE XREF: __ld12cvt+56.j
		/*.text:00406200*/                 lea     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:00406203*/                 push    eax
		/*.text:00406204*/                 lea     ecx, /*[ebp+*/ var_30 /*]*/
		/*.text:00406207*/                 push    ecx
		/*.text:00406208*/                 call    __CopyMan
		/*.text:0040620D*/                 add     esp, 8
		/*.text:00406210*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406213*/                 mov     eax, [edx+8]
		/*.text:00406216*/                 push    eax
		/*.text:00406217*/                 lea     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040621A*/                 push    ecx
		/*.text:0040621B*/                 call    __RoundMan
		/*.text:00406220*/                 add     esp, 8
		/*.text:00406223*/                 test    eax, eax
		/*.text:00406225*/                 jz      short loc_406230
		/*.text:00406227*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0040622A*/                 add     edx, 1
		/*.text:0040622D*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00406230*/ 
		/*.text:00406230*/ loc_406230:                             ; CODE XREF: __ld12cvt+B5.j
		/*.text:00406230*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406233*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406236*/                 mov     edx, [eax+4]
		/*.text:00406239*/                 sub     edx, [ecx+8]
		/*.text:0040623C*/                 cmp     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0040623F*/                 jge     short loc_406260
		/*.text:00406241*/                 lea     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:00406244*/                 push    eax
		/*.text:00406245*/                 call    __FillZeroMan
		/*.text:0040624A*/                 add     esp, 4
		/*.text:0040624D*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:00406254*/                 mov     /*[ebp+*/ var_1C /*]*/, 2
		/*.text:0040625B*/                 jmp     loc_40634C
		/*.text:00406260*/ ; ---------------------------------------------------------------------------
		/*.text:00406260*/ 
		/*.text:00406260*/ loc_406260:                             ; CODE XREF: __ld12cvt+CF.j
		/*.text:00406260*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406263*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406266*/                 cmp     edx, [ecx+4]
		/*.text:00406269*/                 jg      short loc_4062D0
		/*.text:0040626B*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040626E*/                 mov     ecx, [eax+4]
		/*.text:00406271*/                 sub     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406274*/                 mov     /*[ebp+*/ var_34 /*]*/, ecx
		/*.text:00406277*/                 lea     edx, /*[ebp+*/ var_30 /*]*/
		/*.text:0040627A*/                 push    edx
		/*.text:0040627B*/                 lea     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:0040627E*/                 push    eax
		/*.text:0040627F*/                 call    __CopyMan
		/*.text:00406284*/                 add     esp, 8
		/*.text:00406287*/                 mov     ecx, /*[ebp+*/ var_34 /*]*/
		/*.text:0040628A*/                 push    ecx
		/*.text:0040628B*/                 lea     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:0040628E*/                 push    edx
		/*.text:0040628F*/                 call    __ShrMan
		/*.text:00406294*/                 add     esp, 8
		/*.text:00406297*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040629A*/                 mov     ecx, [eax+8]
		/*.text:0040629D*/                 push    ecx
		/*.text:0040629E*/                 lea     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:004062A1*/                 push    edx
		/*.text:004062A2*/                 call    __RoundMan
		/*.text:004062A7*/                 add     esp, 8
		/*.text:004062AA*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004062AD*/                 mov     ecx, [eax+0Ch]
		/*.text:004062B0*/                 add     ecx, 1
		/*.text:004062B3*/                 push    ecx
		/*.text:004062B4*/                 lea     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:004062B7*/                 push    edx
		/*.text:004062B8*/                 call    __ShrMan
		/*.text:004062BD*/                 add     esp, 8
		/*.text:004062C0*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:004062C7*/                 mov     /*[ebp+*/ var_1C /*]*/, 2
		/*.text:004062CE*/                 jmp     short loc_40634C
		/*.text:004062D0*/ ; ---------------------------------------------------------------------------
		/*.text:004062D0*/ 
		/*.text:004062D0*/ loc_4062D0:                             ; CODE XREF: __ld12cvt+F9.j
		/*.text:004062D0*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004062D3*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004062D6*/                 cmp     ecx, [eax]
		/*.text:004062D8*/                 jl      short loc_40631B
		/*.text:004062DA*/                 lea     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:004062DD*/                 push    edx
		/*.text:004062DE*/                 call    __FillZeroMan
		/*.text:004062E3*/                 add     esp, 4
		/*.text:004062E6*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:004062E9*/                 or      eax, 80000000h
		/*.text:004062EE*/                 mov     /*[ebp+*/ var_18 /*]*/, eax
		/*.text:004062F1*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:004062F4*/                 mov     edx, [ecx+0Ch]
		/*.text:004062F7*/                 push    edx
		/*.text:004062F8*/                 lea     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:004062FB*/                 push    eax
		/*.text:004062FC*/                 call    __ShrMan
		/*.text:00406301*/                 add     esp, 8
		/*.text:00406304*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406307*/                 mov     edx, [ecx]
		/*.text:00406309*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040630C*/                 add     edx, [eax+14h]
		/*.text:0040630F*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00406312*/                 mov     /*[ebp+*/ var_1C /*]*/, 1
		/*.text:00406319*/                 jmp     short loc_40634C
		/*.text:0040631B*/ ; ---------------------------------------------------------------------------
		/*.text:0040631B*/ 
		/*.text:0040631B*/ loc_40631B:                             ; CODE XREF: __ld12cvt+168.j
		/*.text:0040631B*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040631E*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00406321*/                 add     edx, [ecx+14h]
		/*.text:00406324*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00406327*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:0040632A*/                 and     eax, 7FFFFFFFh
		/*.text:0040632F*/                 mov     /*[ebp+*/ var_18 /*]*/, eax
		/*.text:00406332*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406335*/                 mov     edx, [ecx+0Ch]
		/*.text:00406338*/                 push    edx
		/*.text:00406339*/                 lea     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:0040633C*/                 push    eax
		/*.text:0040633D*/                 call    __ShrMan
		/*.text:00406342*/                 add     esp, 8
		/*.text:00406345*/                 mov     /*[ebp+*/ var_1C /*]*/, 0
		/*.text:0040634C*/ 
		/*.text:0040634C*/ loc_40634C:                             ; CODE XREF: __ld12cvt+8B.j
		/*.text:0040634C*/                                         ; __ld12cvt+EB.j ...
		/*.text:0040634C*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040634F*/                 mov     edx, [ecx+0Ch]
		/*.text:00406352*/                 add     edx, 1
		/*.text:00406355*/                 mov     eax, 20h
		/*.text:0040635A*/                 sub     eax, edx
		/*.text:0040635C*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:0040635F*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:00406362*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00406365*/                 shl     edx, cl
		/*.text:00406367*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:0040636A*/                 or      eax, edx
		/*.text:0040636C*/                 mov     ecx, /*[ebp+*/ var_20 /*]*/
		/*.text:0040636F*/                 neg     ecx
		/*.text:00406371*/                 sbb     ecx, ecx
		/*.text:00406373*/                 and     ecx, 80000000h
		/*.text:00406379*/                 or      eax, ecx
		/*.text:0040637B*/                 mov     /*[ebp+*/ var_24 /*]*/, eax
		/*.text:0040637E*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00406381*/                 cmp     dword ptr [edx+10h], 40h
		/*.text:00406385*/                 jnz     short loc_40639A
		/*.text:00406387*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0040638A*/                 mov     ecx, /*[ebp+*/ var_24 /*]*/
		/*.text:0040638D*/                 mov     [eax+4], ecx
		/*.text:00406390*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00406393*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00406396*/                 mov     [edx], eax
		/*.text:00406398*/                 jmp     short loc_4063AB
		/*.text:0040639A*/ ; ---------------------------------------------------------------------------
		/*.text:0040639A*/ 
		/*.text:0040639A*/ loc_40639A:                             ; CODE XREF: __ld12cvt+215.j
		/*.text:0040639A*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0040639D*/                 cmp     dword ptr [ecx+10h], 20h
		/*.text:004063A1*/                 jnz     short loc_4063AB
		/*.text:004063A3*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004063A6*/                 mov     eax, /*[ebp+*/ var_24 /*]*/
		/*.text:004063A9*/                 mov     [edx], eax
		/*.text:004063AB*/ 
		/*.text:004063AB*/ loc_4063AB:                             ; CODE XREF: __ld12cvt+228.j
		/*.text:004063AB*/                                         ; __ld12cvt+231.j
		/*.text:004063AB*/                 mov     eax, /*[ebp+*/ var_1C /*]*/
		/*.text:004063AE*/                 mov     esp, ebp
		/*.text:004063B0*/                 pop     ebp
		/*.text:004063B1*/                 retn
		/*.text:004063B1*/ // __ld12cvt       endp
	}
}
#undef var_34
#undef var_30
#undef var_24
#undef var_20
#undef var_1C
#undef var_18
#undef var_14
#undef var_10
#undef var_C 
#undef var_8 
#undef var_4 
#undef arg_0 
#undef arg_4 
#undef arg_8 

/*.data:00428E18*/ static BYTE unk_428E18 [] = {      /*db*/    0x0 , // ;               ; DATA XREF: __ld12tod+3.o
/*.data:00428E19*/                 /*db*/    0x4 , // ;  
/*.data:00428E1A*/                 /*db*/    0x0 , // ;  
/*.data:00428E1B*/                 /*db*/    0x0 , // ;  
/*.data:00428E1C*/                 /*db*/    0x1 , // ;  
/*.data:00428E1D*/                 /*db*/ 0x0FC , // ; n
/*.data:00428E1E*/                 /*db*/ 0x0FF , // ;  
/*.data:00428E1F*/                 /*db*/ 0x0FF , // ;  
/*.data:00428E20*/                 /*db*/  0x35 , // ; 5
/*.data:00428E21*/                 /*db*/    0x0 , // ;  
/*.data:00428E22*/                 /*db*/    0x0 , // ;  
/*.data:00428E23*/                 /*db*/    0x0 , // ;  
/*.data:00428E24*/                 /*db*/  0x0B , // ;  
/*.data:00428E25*/                 /*db*/    0x0 , // ;  
/*.data:00428E26*/                 /*db*/    0x0 , // ;  
/*.data:00428E27*/                 /*db*/    0x0 , // ;  
/*.data:00428E28*/                 /*db*/  0x40 , // ; @
/*.data:00428E29*/                 /*db*/    0x0 , // ;  
/*.data:00428E2A*/                 /*db*/    0x0 , // ;  
/*.data:00428E2B*/                 /*db*/    0x0 , // ;  
/*.data:00428E2C*/                 /*db*/ 0x0FF , // ;  
/*.data:00428E2D*/                 /*db*/    0x3 , // ;  
/*.data:00428E2E*/                 /*db*/    0x0 , // ;  
/*.data:00428E2F*/                 /*db*/    0x0  // ;  
};

static VOID __declspec( naked ) __ld12tod( VOID )
{
	__asm
	{
		/*.text:004063C0*/ // __ld12tod       proc near               ; CODE XREF: __fltin+57.p
		/*.text:004063C0*/                                         ; __atodbl+2A.p
		/*.text:004063C0*/ 
		/*.text:004063C0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:004063C0*/ #define arg_4            dword ptr [ebp+0x0C]
		/*.text:004063C0*/ 
		/*.text:004063C0*/                 push    ebp
		/*.text:004063C1*/                 mov     ebp, esp
		/*.text:004063C3*/                 push    offset unk_428E18
		/*.text:004063C8*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:004063CB*/                 push    eax
		/*.text:004063CC*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004063CF*/                 push    ecx
		/*.text:004063D0*/                 call    __ld12cvt
		/*.text:004063D5*/                 add     esp, 0Ch
		/*.text:004063D8*/                 pop     ebp
		/*.text:004063D9*/                 retn
		/*.text:004063D9*/ // __ld12tod       endp
	}
}
#undef arg_0
#undef arg_4

static VOID /*FLT     __cdecl */ __declspec( naked ) _fltin( VOID /*const char *, int, int, int */)
{
	__asm
	{
		/*.text:004015F0*/ // __fltin         proc near               ; CODE XREF: _strtod+6B.p
		/*.text:004015F0*/ 
		/*.text:004015F0*/ #define var_24           dword ptr [ebp-0x24]
		/*.text:004015F0*/ #define var_20           dword ptr [ebp-0x20]
		/*.text:004015F0*/ #define var_1C           dword ptr [ebp-0x1C]
		/*.text:004015F0*/ #define var_18           dword ptr [ebp-0x18]
		/*.text:004015F0*/ #define var_14           dword ptr [ebp-0x14]
		/*.text:004015F0*/ #define var_10           byte ptr [ebp-0x10]
		/*.text:004015F0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:004015F0*/ #define arg_0            dword ptr [ebp+0x8]
		/*.text:004015F0*/ 
		/*.text:004015F0*/                 push    ebp
		/*.text:004015F1*/                 mov     ebp, esp
		/*.text:004015F3*/                 sub     esp, 24h
		/*.text:004015F6*/                 mov     /*[ebp+*/ var_20 /*]*/, 0
		/*.text:004015FD*/                 push    0
		/*.text:004015FF*/                 push    0
		/*.text:00401601*/                 push    0
		/*.text:00401603*/                 push    0
		/*.text:00401605*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00401608*/                 push    eax
		/*.text:00401609*/                 lea     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:0040160C*/                 push    ecx
		/*.text:0040160D*/                 lea     edx, /*[ebp+*/ var_10 /*]*/
		/*.text:00401610*/                 push    edx
		/*.text:00401611*/                 call    ___strgtold12
		/*.text:00401616*/                 add     esp, 1Ch
		/*.text:00401619*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0040161C*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0040161F*/                 and     eax, 4
		/*.text:00401622*/                 test    eax, eax
		/*.text:00401624*/                 jz      short loc_40163F
		/*.text:00401626*/                 mov     ecx, /*[ebp+*/ var_20 /*]*/
		/*.text:00401629*/                 or      ch, 2
		/*.text:0040162C*/                 mov     /*[ebp+*/ var_20 /*]*/, ecx
		/*.text:0040162F*/                 mov     /*[ebp+*/ var_1C /*]*/, 0
		/*.text:00401636*/                 mov     /*[ebp+*/ var_18 /*]*/, 0
		/*.text:0040163D*/                 jmp     short loc_401684
		/*.text:0040163F*/ ; ---------------------------------------------------------------------------
		/*.text:0040163F*/ 
		/*.text:0040163F*/ loc_40163F:                             ; CODE XREF: __fltin+34.j
		/*.text:0040163F*/                 lea     edx, /*[ebp+*/ var_1C /*]*/
		/*.text:00401642*/                 push    edx
		/*.text:00401643*/                 lea     eax, /*[ebp+*/ var_10 /*]*/
		/*.text:00401646*/                 push    eax
		/*.text:00401647*/                 call    __ld12tod
		/*.text:0040164C*/                 add     esp, 8
		/*.text:0040164F*/                 mov     /*[ebp+*/ var_24 /*]*/, eax
		/*.text:00401652*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00401655*/                 and     ecx, 2
		/*.text:00401658*/                 test    ecx, ecx
		/*.text:0040165A*/                 jnz     short loc_401662
		/*.text:0040165C*/                 cmp     /*[ebp+*/ var_24 /*]*/, 1
		/*.text:00401660*/                 jnz     short loc_40166B
		/*.text:00401662*/ 
		/*.text:00401662*/ loc_401662:                             ; CODE XREF: __fltin+6A.j
		/*.text:00401662*/                 mov     edx, /*[ebp+*/ var_20 /*]*/
		/*.text:00401665*/                 or      dl, 80h
		/*.text:00401668*/                 mov     /*[ebp+*/ var_20 /*]*/, edx
		/*.text:0040166B*/ 
		/*.text:0040166B*/ loc_40166B:                             ; CODE XREF: __fltin+70.j
		/*.text:0040166B*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0040166E*/                 and     eax, 1
		/*.text:00401671*/                 test    eax, eax
		/*.text:00401673*/                 jnz     short loc_40167B
		/*.text:00401675*/                 cmp     /*[ebp+*/ var_24 /*]*/, 2
		/*.text:00401679*/                 jnz     short loc_401684
		/*.text:0040167B*/ 
		/*.text:0040167B*/ loc_40167B:                             ; CODE XREF: __fltin+83.j
		/*.text:0040167B*/                 mov     ecx, /*[ebp+*/ var_20 /*]*/
		/*.text:0040167E*/                 or      ch, 1
		/*.text:00401681*/                 mov     /*[ebp+*/ var_20 /*]*/, ecx
		/*.text:00401684*/ 
		/*.text:00401684*/ loc_401684:                             ; CODE XREF: __fltin+4D.j
		/*.text:00401684*/                                         ; __fltin+89.j
		/*.text:00401684*/                 mov     edx, offset off_428A54
		/*.text:0040168A*/                 mov     eax, /*[ebp+*/ var_20 /*]*/
		/*.text:0040168D*/                 mov     [edx], eax
		/*.text:0040168F*/                 mov     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:00401692*/                 sub     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00401695*/                 mov     edx, offset off_428A54
		/*.text:0040169B*/                 mov     [edx+4], ecx
		/*.text:0040169E*/                 mov     eax, offset off_428A54
		/*.text:004016A3*/                 mov     ecx, /*[ebp+*/ var_1C /*]*/
		/*.text:004016A6*/                 mov     [eax+10h], ecx
		/*.text:004016A9*/                 mov     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:004016AC*/                 mov     [eax+14h], edx
		/*.text:004016AF*/                 mov     eax, offset off_428A54
		/*.text:004016B4*/                 mov     esp, ebp
		/*.text:004016B6*/                 pop     ebp
		/*.text:004016B7*/                 retn
		/*.text:004016B7*/ // __fltin         endp
	}
}
#undef var_24
#undef var_20
#undef var_1C
#undef var_18
#undef var_14
#undef var_10
#undef var_4
#undef arg_0

//=========================================
//
// FROM C-RUNTIME SOURCE FILES...
//
//=========================================

static unsigned long __cdecl MACRO_CRTFN_NAME(strtoxl) (
        const char *nptr,
        const char **endptr,
        int ibase,
        int flags
        )
{
        const char *p;
        char c;
        unsigned long number;
        unsigned digval;
        unsigned long maxval;

        p = nptr;                       /* p is our scanning pointer */
        number = 0;                     /* start with zero */

        c = *p++;                       /* read char */
        while ( isspace((int)(unsigned char)c) )
                c = *p++;               /* skip whitespace */

        if (c == '-') {
                flags |= FL_NEG;        /* remember minus sign */
                c = *p++;
        }
        else if (c == '+')
                c = *p++;               /* skip sign */

        if (ibase < 0 || ibase == 1 || ibase > 36) {
                /* bad base! */
                if (endptr)
                        /* store beginning of string in endptr */
                        *endptr = nptr;
                return 0L;              /* return 0 */
        }
        else if (ibase == 0) {
                /* determine base free-lance, based on first two chars of
                   string */
                if (c != '0')
                        ibase = 10;
                else if (*p == 'x' || *p == 'X')
                        ibase = 16;
                else
                        ibase = 8;
        }

        if (ibase == 16) {
                /* we might have 0x in front of number; remove if there */
                if (c == '0' && (*p == 'x' || *p == 'X')) {
                        ++p;
                        c = *p++;       /* advance past prefix */
                }
        }

        /* if our number exceeds this, we will overflow on multiply */
        maxval = ULONG_MAX / ibase;


        for (;;) {      /* exit in middle of loop */
                /* convert c to value */
                if ( isdigit((int)(unsigned char)c) )
                        digval = c - '0';
                else if ( isalpha((int)(unsigned char)c) )
                        digval = toupper(c) - 'A' + 10;
                else
                        break;
                if (digval >= (unsigned)ibase)
                        break;          /* exit loop if bad digit found */

                /* record the fact we have read one digit */
                flags |= FL_READDIGIT;

                /* we now need to compute number = number * base + digval,
                   but we need to know if overflow occured.  This requires
                   a tricky pre-check. */

                if (number < maxval || (number == maxval &&
                (unsigned long)digval <= ULONG_MAX % ibase)) {
                        /* we won't overflow, go ahead and multiply */
                        number = number * ibase + digval;
                }
                else {
                        /* we would have overflowed -- set the overflow flag */
                        flags |= FL_OVERFLOW;
                }

                c = *p++;               /* read next digit */
        }

        --p;                            /* point to place that stopped scan */

        if (!(flags & FL_READDIGIT)) {
                /* no number there; return 0 and point to beginning of
                   string */
                if (endptr)
                        /* store beginning of string in endptr later on */
                        p = nptr;
                number = 0L;            /* return 0 */
        }
        else if ( (flags & FL_OVERFLOW) ||
                  ( !(flags & FL_UNSIGNED) &&
                    ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
                      ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
        {
                /* overflow or signed overflow occurred */
                //errno = ERANGE;
                if ( flags & FL_UNSIGNED )
                        number = ULONG_MAX;
                else if ( flags & FL_NEG )
                        number = (unsigned long)(-LONG_MIN);
                else
                        number = LONG_MAX;
        }

        if (endptr != NULL)
                /* store pointer to char that stopped the scan */
                *endptr = p;

        if (flags & FL_NEG)
                /* negate result if there was a neg sign */
                number = (unsigned long)(-(long)number);

        return number;                  /* done. */
}

long __cdecl MACRO_CRTFN_NAME(strtol) (
        const char *nptr,
        char **endptr,
        int ibase
        )
{
        return (long) MACRO_CRTFN_NAME(strtoxl)(nptr, endptr, ibase, 0);
}

unsigned long __cdecl MACRO_CRTFN_NAME(strtoul) (
        const char *nptr,
        char **endptr,
        int ibase
        )
{
        return MACRO_CRTFN_NAME(strtoxl)(nptr, endptr, ibase, FL_UNSIGNED);
}

char * __cdecl MACRO_CRTFN_NAME(strchr) (
        const char * string,
        int ch
        )
{
        while (*string && *string != (char)ch)
                string++;

        if (*string == (char)ch)
                return((char *)string);
        return(NULL);
}

double __cdecl MACRO_CRTFN_NAME(strtod) (
        const char *nptr,
        /*REG2*/ char **endptr
        )
{

#ifdef _MT
        struct _flt answerstruct;
#endif  /* _MT */

        FLT      answer;
        double       tmp;
        unsigned int flags;
        /*REG1*/ char *ptr = (char *) nptr;

        /* scan past leading space/tab characters */

        while ( isspace((int)(unsigned char)*ptr) )
                ptr++;

        /* let _fltin routine do the rest of the work */

#ifdef _MT
        /* ok to take address of stack variable here; fltin2 knows to use ss */
        answer = _fltin2( &answerstruct, ptr, strlen(ptr), 0, 0);
#else  /* _MT */
		{
			int		temp = strlen(ptr);
			DWORD	temp2 = (DWORD) ptr;
			// answer = _fltin(ptr, strlen(ptr), 0, 0);
			__asm
			{
				push		0
				push		0
				push		temp
				push		temp2
				call		_fltin
				add			esp, 4*4
				mov			answer, eax
			}
		}
#endif  /* _MT */

        if ( endptr != NULL )
                *endptr = (char *) ptr + answer->nbytes;

        flags = answer->flags;
        if ( flags & (512 | 64)) {
                /* no digits found or invalid format:
                   ANSI says return 0.0, and *endptr = nptr */
                tmp = 0.0;
                if ( endptr != NULL )
                        *endptr = (char *) nptr;
        }
        else if ( flags & (128 | 1) ) {
                if ( *ptr == '-' )
                        tmp = -HUGE_VAL;        /* negative overflow */
                else
                        tmp = HUGE_VAL;         /* positive overflow */
                //errno = ERANGE;
        }
        else if ( flags & 256 ) {
                tmp = 0.0;                      /* underflow */
                //errno = ERANGE;
        }
        else
                tmp = answer->dval;

        return(tmp);
}

static int __cdecl _strcmpi(const char * dst, const char * src)
{
        int f,l;
#ifdef _MT
        int local_lock_flag;
#endif  /* _MT */

#if 0 // defined (_WIN32)
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif  /* defined (_WIN32) */
            do {
                if ( ((f = (unsigned char)(*(dst++))) >= 'A') && (f <= 'Z') )
                    f -= ('A' - 'a');

                if ( ((l = (unsigned char)(*(src++))) >= 'A') && (l <= 'Z') )
                    l -= ('A' - 'a');
            } while ( f && (f == l) );
#if 0 // defined (_WIN32)
        }
        else {
            _lock_locale( local_lock_flag )
            do {
                f = _tolower_lk( (unsigned char)(*(dst++)) );
                l = _tolower_lk( (unsigned char)(*(src++)) );
            } while ( f && (f == l) );
            _unlock_locale( local_lock_flag )
        }
#endif  /* defined (_WIN32) */

        return(f - l);
}

int __cdecl MACRO_CRTFN_NAME(stricmp) (
        const char * dst,
        const char * src
        )
{
        return( _strcmpi(dst,src) );
}

#define _TOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') :\
              (c) )

int __cdecl MACRO_CRTFN_NAME(memicmp) (
        const void * first,
        const void * last,
        unsigned int count
        )
{
        int f = 0;
        int l = 0;
#ifdef _MT
        int local_lock_flag;
#endif  /* _MT */

#if 0 // defined (_WIN32)
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif  /* defined (_WIN32) */
            while ( count-- )
            {
                if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                     ((f = _TOLOWER( *(unsigned char *)first )) ==
                      (l = _TOLOWER( *(unsigned char *)last ))) )
                {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
                }
                else
                    break;
            }
#if 0 // defined (_WIN32)
        }
        else {
            _lock_locale( local_lock_flag )
            while ( count-- )
                if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                     ((f = _tolower_lk( *(unsigned char *)first )) ==
                      (l = _tolower_lk( *(unsigned char *)last ))) )
                {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
                }
                else
                    break;
            _unlock_locale( local_lock_flag )
        }
#endif  /* defined (_WIN32) */

        return ( f - l );
}
