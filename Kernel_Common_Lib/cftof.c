/***************************************************************************************
  *
  * cftof.c - VPCICE Support Routines from CRT (Double2String Conversion) - Source File.
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

//======================================
// Wrappers for Compiler Compatibility.
//  ( A compiler's BUG ??? )
//======================================

static char* __cdecl __strcpy( char* p1, const char* p2 )
{
	return strcpy( p1, p2 );
}

//=======
// Data.
//=======

static BYTE ___decimal_point  = 0x2E;

static CHAR a1Qnan[] =          /*db*/ "1#QNAN"; // ,0
static CHAR a1Inf[] =           /*db*/ "1#INF"; // ,0
static CHAR a1Ind[] =           /*db*/ "1#IND"; // ,0
static CHAR a1Snan[] =          /*db*/ "1#SNAN"; // ,0

/*.data:005E4D70*/ static BYTE __pow10pos[] = {      /*db*/    0x0 , // ;               ; DATA XREF: ___multtenpow12+6.o
/*.data:005E4D71*/                 /*db*/    0x0 , // ;  
/*.data:005E4D72*/                 /*db*/    0x0 , // ;  
/*.data:005E4D73*/                 /*db*/    0x0 , // ;  
/*.data:005E4D74*/                 /*db*/    0x0 , // ;  
/*.data:005E4D75*/                 /*db*/    0x0 , // ;  
/*.data:005E4D76*/                 /*db*/    0x0 , // ;  
/*.data:005E4D77*/                 /*db*/    0x0 , // ;  
/*.data:005E4D78*/                 /*db*/    0x0 , // ;  
/*.data:005E4D79*/                 /*db*/ 0x0A0  , // ; á
/*.data:005E4D7A*/                 /*db*/    0x2 , // ;  
/*.data:005E4D7B*/                 /*db*/  0x40  , // ; @
/*.data:005E4D7C*/                 /*db*/    0x0 , // ;  
/*.data:005E4D7D*/                 /*db*/    0x0 , // ;  
/*.data:005E4D7E*/                 /*db*/    0x0 , // ;  
/*.data:005E4D7F*/                 /*db*/    0x0 , // ;  
/*.data:005E4D80*/                 /*db*/    0x0 , // ;  
/*.data:005E4D81*/                 /*db*/    0x0 , // ;  
/*.data:005E4D82*/                 /*db*/    0x0 , // ;  
/*.data:005E4D83*/                 /*db*/    0x0 , // ;  
/*.data:005E4D84*/                 /*db*/    0x0 , // ;  
/*.data:005E4D85*/                 /*db*/ 0x0C8  , // ; +
/*.data:005E4D86*/                 /*db*/    0x5 , // ;  
/*.data:005E4D87*/                 /*db*/  0x40  , // ; @
/*.data:005E4D88*/                 /*db*/    0x0 , // ;  
/*.data:005E4D89*/                 /*db*/    0x0 , // ;  
/*.data:005E4D8A*/                 /*db*/    0x0 , // ;  
/*.data:005E4D8B*/                 /*db*/    0x0 , // ;  
/*.data:005E4D8C*/                 /*db*/    0x0 , // ;  
/*.data:005E4D8D*/                 /*db*/    0x0 , // ;  
/*.data:005E4D8E*/                 /*db*/    0x0 , // ;  
/*.data:005E4D8F*/                 /*db*/    0x0 , // ;  
/*.data:005E4D90*/                 /*db*/    0x0 , // ;  
/*.data:005E4D91*/                 /*db*/ 0x0FA  , // ; ·
/*.data:005E4D92*/                 /*db*/    0x8 , // ;  
/*.data:005E4D93*/                 /*db*/  0x40  , // ; @
/*.data:005E4D94*/                 /*db*/    0x0 , // ;  
/*.data:005E4D95*/                 /*db*/    0x0 , // ;  
/*.data:005E4D96*/                 /*db*/    0x0 , // ;  
/*.data:005E4D97*/                 /*db*/    0x0 , // ;  
/*.data:005E4D98*/                 /*db*/    0x0 , // ;  
/*.data:005E4D99*/                 /*db*/    0x0 , // ;  
/*.data:005E4D9A*/                 /*db*/    0x0 , // ;  
/*.data:005E4D9B*/                 /*db*/    0x0 , // ;  
/*.data:005E4D9C*/                 /*db*/  0x40  , // ; @
/*.data:005E4D9D*/                 /*db*/  0x9C  , // ; £
/*.data:005E4D9E*/                 /*db*/  0x0C  , // ;  
/*.data:005E4D9F*/                 /*db*/  0x40  , // ; @
/*.data:005E4DA0*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA1*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA2*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA3*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA4*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA5*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA6*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA7*/                 /*db*/    0x0 , // ;  
/*.data:005E4DA8*/                 /*db*/  0x50  , // ; P
/*.data:005E4DA9*/                 /*db*/ 0x0C3  , // ; +
/*.data:005E4DAA*/                 /*db*/  0x0F  , // ;  
/*.data:005E4DAB*/                 /*db*/  0x40  , // ; @
/*.data:005E4DAC*/                 /*db*/    0x0 , // ;  
/*.data:005E4DAD*/                 /*db*/    0x0 , // ;  
/*.data:005E4DAE*/                 /*db*/    0x0 , // ;  
/*.data:005E4DAF*/                 /*db*/    0x0 , // ;  
/*.data:005E4DB0*/                 /*db*/    0x0 , // ;  
/*.data:005E4DB1*/                 /*db*/    0x0 , // ;  
/*.data:005E4DB2*/                 /*db*/    0x0 , // ;  
/*.data:005E4DB3*/                 /*db*/    0x0 , // ;  
/*.data:005E4DB4*/                 /*db*/  0x24  , // ; $
/*.data:005E4DB5*/                 /*db*/ 0x0F4  , // ; ¶
/*.data:005E4DB6*/                 /*db*/  0x12  , // ;  
/*.data:005E4DB7*/                 /*db*/  0x40  , // ; @
/*.data:005E4DB8*/                 /*db*/    0x0 , // ;  
/*.data:005E4DB9*/                 /*db*/    0x0 , // ;  
/*.data:005E4DBA*/                 /*db*/    0x0 , // ;  
/*.data:005E4DBB*/                 /*db*/    0x0 , // ;  
/*.data:005E4DBC*/                 /*db*/    0x0 , // ;  
/*.data:005E4DBD*/                 /*db*/    0x0 , // ;  
/*.data:005E4DBE*/                 /*db*/    0x0 , // ;  
/*.data:005E4DBF*/                 /*db*/  0x80  , // ; Ç
/*.data:005E4DC0*/                 /*db*/  0x96  , // ; û
/*.data:005E4DC1*/                 /*db*/  0x98  , // ; ÿ
/*.data:005E4DC2*/                 /*db*/  0x16  , // ;  
/*.data:005E4DC3*/                 /*db*/  0x40  , // ; @
/*.data:005E4DC4*/                 /*db*/    0x0 , // ;  
/*.data:005E4DC5*/                 /*db*/    0x0 , // ;  
/*.data:005E4DC6*/                 /*db*/    0x0 , // ;  
/*.data:005E4DC7*/                 /*db*/    0x0 , // ;  
/*.data:005E4DC8*/                 /*db*/    0x0 , // ;  
/*.data:005E4DC9*/                 /*db*/    0x0 , // ;  
/*.data:005E4DCA*/                 /*db*/    0x0 , // ;  
/*.data:005E4DCB*/                 /*db*/  0x20  , // ;  
/*.data:005E4DCC*/                 /*db*/ 0x0BC  , // ; +
/*.data:005E4DCD*/                 /*db*/ 0x0BE  , // ; ¥
/*.data:005E4DCE*/                 /*db*/  0x19  , // ;  
/*.data:005E4DCF*/                 /*db*/  0x40  , // ; @
/*.data:005E4DD0*/                 /*db*/    0x0 , // ;  
/*.data:005E4DD1*/                 /*db*/    0x0 , // ;  
/*.data:005E4DD2*/                 /*db*/    0x0 , // ;  
/*.data:005E4DD3*/                 /*db*/    0x0 , // ;  
/*.data:005E4DD4*/                 /*db*/    0x0 , // ;  
/*.data:005E4DD5*/                 /*db*/    0x4 , // ;  
/*.data:005E4DD6*/                 /*db*/ 0x0BF  , // ; +
/*.data:005E4DD7*/                 /*db*/ 0x0C9  , // ; +
/*.data:005E4DD8*/                 /*db*/  0x1B  , // ;  
/*.data:005E4DD9*/                 /*db*/  0x8E  , // ; Ä
/*.data:005E4DDA*/                 /*db*/  0x34  , // ; 4
/*.data:005E4DDB*/                 /*db*/  0x40  , // ; @
/*.data:005E4DDC*/                 /*db*/    0x0 , // ;  
/*.data:005E4DDD*/                 /*db*/    0x0 , // ;  
/*.data:005E4DDE*/                 /*db*/    0x0 , // ;  
/*.data:005E4DDF*/                 /*db*/ 0x0A1  , // ; í
/*.data:005E4DE0*/                 /*db*/ 0x0ED  , // ; Ý
/*.data:005E4DE1*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4DE2*/                 /*db*/ 0x0CE  , // ; +
/*.data:005E4DE3*/                 /*db*/  0x1B  , // ;  
/*.data:005E4DE4*/                 /*db*/ 0x0C2  , // ; -
/*.data:005E4DE5*/                 /*db*/ 0x0D3  , // ; Ë
/*.data:005E4DE6*/                 /*db*/  0x4E  , // ; N
/*.data:005E4DE7*/                 /*db*/  0x40  , // ; @
/*.data:005E4DE8*/                 /*db*/  0x20  , // ;  
/*.data:005E4DE9*/                 /*db*/ 0x0F0  , // ; ­
/*.data:005E4DEA*/                 /*db*/  0x9E  , // ; ×
/*.data:005E4DEB*/                 /*db*/ 0x0B5  , // ; Á
/*.data:005E4DEC*/                 /*db*/  0x70  , // ; p
/*.data:005E4DED*/                 /*db*/  0x2B  , // ; +
/*.data:005E4DEE*/                 /*db*/ 0x0A8  , // ; ¿
/*.data:005E4DEF*/                 /*db*/ 0x0AD  , // ; ¡
/*.data:005E4DF0*/                 /*db*/ 0x0C5  , // ; +
/*.data:005E4DF1*/                 /*db*/  0x9D  , // ; Ø
/*.data:005E4DF2*/                 /*db*/  0x69  , // ; i
/*.data:005E4DF3*/                 /*db*/  0x40  , // ; @
/*.data:005E4DF4*/                 /*db*/ 0x0D0  , // ; ð
/*.data:005E4DF5*/                 /*db*/  0x5D  , // ; ]
/*.data:005E4DF6*/                 /*db*/ 0x0FD  , // ; ²
/*.data:005E4DF7*/                 /*db*/  0x25  , // ; %
/*.data:005E4DF8*/                 /*db*/ 0x0E5  , // ; Õ
/*.data:005E4DF9*/                 /*db*/  0x1A  , // ;  
/*.data:005E4DFA*/                 /*db*/  0x8E  , // ; Ä
/*.data:005E4DFB*/                 /*db*/  0x4F  , // ; O
/*.data:005E4DFC*/                 /*db*/  0x19  , // ;  
/*.data:005E4DFD*/                 /*db*/ 0x0EB  , // ; Ù
/*.data:005E4DFE*/                 /*db*/  0x83  , // ; â
/*.data:005E4DFF*/                 /*db*/  0x40  , // ; @
/*.data:005E4E00*/                 /*db*/  0x71  , // ; q
/*.data:005E4E01*/                 /*db*/  0x96  , // ; û
/*.data:005E4E02*/                 /*db*/ 0x0D7  , // ; Î
/*.data:005E4E03*/                 /*db*/  0x95  , // ; ò
/*.data:005E4E04*/                 /*db*/  0x43  , // ; C
/*.data:005E4E05*/                 /*db*/  0x0E  , // ;  
/*.data:005E4E06*/                 /*db*/    0x5 , // ;  
/*.data:005E4E07*/                 /*db*/  0x8D  , // ; ì
/*.data:005E4E08*/                 /*db*/  0x29  , // ; )
/*.data:005E4E09*/                 /*db*/ 0x0AF  , // ; »
/*.data:005E4E0A*/                 /*db*/  0x9E  , // ; ×
/*.data:005E4E0B*/                 /*db*/  0x40  , // ; @
/*.data:005E4E0C*/                 /*db*/ 0x0F9  , // ; ¨
/*.data:005E4E0D*/                 /*db*/ 0x0BF  , // ; +
/*.data:005E4E0E*/                 /*db*/ 0x0A0  , // ; á
/*.data:005E4E0F*/                 /*db*/  0x44  , // ; D
/*.data:005E4E10*/                 /*db*/ 0x0ED  , // ; Ý
/*.data:005E4E11*/                 /*db*/  0x81  , // ; ü
/*.data:005E4E12*/                 /*db*/  0x12  , // ;  
/*.data:005E4E13*/                 /*db*/  0x8F  , // ; Å
/*.data:005E4E14*/                 /*db*/  0x81  , // ; ü
/*.data:005E4E15*/                 /*db*/  0x82  , // ; é
/*.data:005E4E16*/                 /*db*/ 0x0B9  , // ; ¦
/*.data:005E4E17*/                 /*db*/  0x40  , // ; @
/*.data:005E4E18*/                 /*db*/ 0x0BF  , // ; +
/*.data:005E4E19*/                 /*db*/  0x3C  , // ; <
/*.data:005E4E1A*/                 /*db*/ 0x0D5  , // ; i
/*.data:005E4E1B*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4E1C*/                 /*db*/ 0x0CF  , // ; ¤
/*.data:005E4E1D*/                 /*db*/ 0x0FF  , // ;  
/*.data:005E4E1E*/                 /*db*/  0x49  , // ; I
/*.data:005E4E1F*/                 /*db*/  0x1F  , // ;  
/*.data:005E4E20*/                 /*db*/  0x78  , // ; x
/*.data:005E4E21*/                 /*db*/ 0x0C2  , // ; -
/*.data:005E4E22*/                 /*db*/ 0x0D3  , // ; Ë
/*.data:005E4E23*/                 /*db*/  0x40  , // ; @
/*.data:005E4E24*/                 /*db*/  0x6F  , // ; o
/*.data:005E4E25*/                 /*db*/ 0x0C6  , // ; ã
/*.data:005E4E26*/                 /*db*/ 0x0E0  , // ; Ó
/*.data:005E4E27*/                 /*db*/  0x8C  , // ; î
/*.data:005E4E28*/                 /*db*/ 0x0E9  , // ; Ú
/*.data:005E4E29*/                 /*db*/  0x80  , // ; Ç
/*.data:005E4E2A*/                 /*db*/ 0x0C9  , // ; +
/*.data:005E4E2B*/                 /*db*/  0x47  , // ; G
/*.data:005E4E2C*/                 /*db*/ 0x0BA  , // ; ¦
/*.data:005E4E2D*/                 /*db*/  0x93  , // ; ô
/*.data:005E4E2E*/                 /*db*/ 0x0A8  , // ; ¿
/*.data:005E4E2F*/                 /*db*/  0x41  , // ; A
/*.data:005E4E30*/                 /*db*/ 0x0BC  , // ; +
/*.data:005E4E31*/                 /*db*/  0x85  , // ; à
/*.data:005E4E32*/                 /*db*/  0x6B  , // ; k
/*.data:005E4E33*/                 /*db*/  0x55  , // ; U
/*.data:005E4E34*/                 /*db*/  0x27  , // ; '
/*.data:005E4E35*/                 /*db*/  0x39  , // ; 9
/*.data:005E4E36*/                 /*db*/  0x8D  , // ; ì
/*.data:005E4E37*/                 /*db*/ 0x0F7  , // ; ¸
/*.data:005E4E38*/                 /*db*/  0x70  , // ; p
/*.data:005E4E39*/                 /*db*/ 0x0E0  , // ; Ó
/*.data:005E4E3A*/                 /*db*/  0x7C  , // ; |
/*.data:005E4E3B*/                 /*db*/  0x42  , // ; B
/*.data:005E4E3C*/                 /*db*/ 0x0BC  , // ; +
/*.data:005E4E3D*/                 /*db*/ 0x0DD  , // ; ¦
/*.data:005E4E3E*/                 /*db*/  0x8E  , // ; Ä
/*.data:005E4E3F*/                 /*db*/ 0x0DE  , // ; Ì
/*.data:005E4E40*/                 /*db*/ 0x0F9  , // ; ¨
/*.data:005E4E41*/                 /*db*/  0x9D  , // ; Ø
/*.data:005E4E42*/                 /*db*/ 0x0FB  , // ; ¹
/*.data:005E4E43*/                 /*db*/ 0x0EB  , // ; Ù
/*.data:005E4E44*/                 /*db*/  0x7E  , // ; ~
/*.data:005E4E45*/                 /*db*/ 0x0AA  , // ; ¬
/*.data:005E4E46*/                 /*db*/  0x51  , // ; Q
/*.data:005E4E47*/                 /*db*/  0x43  , // ; C
/*.data:005E4E48*/                 /*db*/ 0x0A1  , // ; í
/*.data:005E4E49*/                 /*db*/ 0x0E6  , // ; µ
/*.data:005E4E4A*/                 /*db*/  0x76  , // ; v
/*.data:005E4E4B*/                 /*db*/ 0x0E3  , // ; Ò
/*.data:005E4E4C*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4E4D*/                 /*db*/ 0x0F2  , // ; =
/*.data:005E4E4E*/                 /*db*/  0x29  , // ; )
/*.data:005E4E4F*/                 /*db*/  0x2F  , // ; /
/*.data:005E4E50*/                 /*db*/  0x84  , // ; ä
/*.data:005E4E51*/                 /*db*/  0x81  , // ; ü
/*.data:005E4E52*/                 /*db*/  0x26  , // ; &
/*.data:005E4E53*/                 /*db*/  0x44  , // ; D
/*.data:005E4E54*/                 /*db*/  0x28  , // ; (
/*.data:005E4E55*/                 /*db*/  0x10  , // ;  
/*.data:005E4E56*/                 /*db*/  0x17  , // ;  
/*.data:005E4E57*/                 /*db*/ 0x0AA  , // ; ¬
/*.data:005E4E58*/                 /*db*/ 0x0F8  , // ; °
/*.data:005E4E59*/                 /*db*/ 0x0AE  , // ; «
/*.data:005E4E5A*/                 /*db*/  0x10  , // ;  
/*.data:005E4E5B*/                 /*db*/ 0x0E3  , // ; Ò
/*.data:005E4E5C*/                 /*db*/ 0x0C5  , // ; +
/*.data:005E4E5D*/                 /*db*/ 0x0C4  , // ; -
/*.data:005E4E5E*/                 /*db*/ 0x0FA  , // ; ·
/*.data:005E4E5F*/                 /*db*/  0x44  , // ; D
/*.data:005E4E60*/                 /*db*/ 0x0EB  , // ; Ù
/*.data:005E4E61*/                 /*db*/ 0x0A7  , // ; º
/*.data:005E4E62*/                 /*db*/ 0x0D4  , // ; È
/*.data:005E4E63*/                 /*db*/ 0x0F3  , // ; ¾
/*.data:005E4E64*/                 /*db*/ 0x0F7  , // ; ¸
/*.data:005E4E65*/                 /*db*/ 0x0EB  , // ; Ù
/*.data:005E4E66*/                 /*db*/ 0x0E1  , // ; ß
/*.data:005E4E67*/                 /*db*/  0x4A  , // ; J
/*.data:005E4E68*/                 /*db*/  0x7A  , // ; z
/*.data:005E4E69*/                 /*db*/  0x95  , // ; ò
/*.data:005E4E6A*/                 /*db*/ 0x0CF  , // ; ¤
/*.data:005E4E6B*/                 /*db*/  0x45  , // ; E
/*.data:005E4E6C*/                 /*db*/  0x65  , // ; e
/*.data:005E4E6D*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4E6E*/                 /*db*/ 0x0C7  , // ; Ã
/*.data:005E4E6F*/                 /*db*/  0x91  , // ; æ
/*.data:005E4E70*/                 /*db*/  0x0E  , // ;  
/*.data:005E4E71*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4E72*/                 /*db*/ 0x0AE  , // ; «
/*.data:005E4E73*/                 /*db*/ 0x0A0  , // ; á
/*.data:005E4E74*/                 /*db*/  0x19  , // ;  
/*.data:005E4E75*/                 /*db*/ 0x0E3  , // ; Ò
/*.data:005E4E76*/                 /*db*/ 0x0A3  , // ; ú
/*.data:005E4E77*/                 /*db*/  0x46  , // ; F
/*.data:005E4E78*/                 /*db*/  0x0D  , // ;  
/*.data:005E4E79*/                 /*db*/  0x65  , // ; e
/*.data:005E4E7A*/                 /*db*/  0x17  , // ;  
/*.data:005E4E7B*/                 /*db*/  0x0C  , // ;  
/*.data:005E4E7C*/                 /*db*/  0x75  , // ; u
/*.data:005E4E7D*/                 /*db*/  0x81  , // ; ü
/*.data:005E4E7E*/                 /*db*/  0x86  , // ; å
/*.data:005E4E7F*/                 /*db*/  0x75  , // ; u
/*.data:005E4E80*/                 /*db*/  0x76  , // ; v
/*.data:005E4E81*/                 /*db*/ 0x0C9  , // ; +
/*.data:005E4E82*/                 /*db*/  0x48  , // ; H
/*.data:005E4E83*/                 /*db*/  0x4D  , // ; M
/*.data:005E4E84*/                 /*db*/  0x58  , // ; X
/*.data:005E4E85*/                 /*db*/  0x42  , // ; B
/*.data:005E4E86*/                 /*db*/ 0x0E4  , // ; õ
/*.data:005E4E87*/                 /*db*/ 0x0A7  , // ; º
/*.data:005E4E88*/                 /*db*/  0x93  , // ; ô
/*.data:005E4E89*/                 /*db*/  0x39  , // ; 9
/*.data:005E4E8A*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4E8B*/                 /*db*/  0x35  , // ; 5
/*.data:005E4E8C*/                 /*db*/ 0x0B8  , // ; ©
/*.data:005E4E8D*/                 /*db*/ 0x0B2  , // ; ¦
/*.data:005E4E8E*/                 /*db*/ 0x0ED  , // ; Ý
/*.data:005E4E8F*/                 /*db*/  0x53  , // ; S
/*.data:005E4E90*/                 /*db*/  0x4D  , // ; M
/*.data:005E4E91*/                 /*db*/ 0x0A7  , // ; º
/*.data:005E4E92*/                 /*db*/ 0x0E5  , // ; Õ
/*.data:005E4E93*/                 /*db*/  0x5D  , // ; ]
/*.data:005E4E94*/                 /*db*/  0x3D  , // ; =
/*.data:005E4E95*/                 /*db*/ 0x0C5  , // ; +
/*.data:005E4E96*/                 /*db*/  0x5D  , // ; ]
/*.data:005E4E97*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4E98*/                 /*db*/  0x8B  , // ; ï
/*.data:005E4E99*/                 /*db*/  0x9E  , // ; ×
/*.data:005E4E9A*/                 /*db*/  0x92  , // ; Æ
/*.data:005E4E9B*/                 /*db*/  0x5A  , // ; Z
/*.data:005E4E9C*/                 /*db*/ 0x0FF  , // ;  
/*.data:005E4E9D*/                 /*db*/  0x5D  , // ; ]
/*.data:005E4E9E*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4E9F*/                 /*db*/ 0x0F0  , // ; ­
/*.data:005E4EA0*/                 /*db*/ 0x0A1  , // ; í
/*.data:005E4EA1*/                 /*db*/  0x20  , // ;  
/*.data:005E4EA2*/                 /*db*/ 0x0C0  , // ; +
/*.data:005E4EA3*/                 /*db*/  0x54  , // ; T
/*.data:005E4EA4*/                 /*db*/ 0x0A5  , // ; Ñ
/*.data:005E4EA5*/                 /*db*/  0x8C  , // ; î
/*.data:005E4EA6*/                 /*db*/  0x37  , // ; 7
/*.data:005E4EA7*/                 /*db*/  0x61  , // ; a
/*.data:005E4EA8*/                 /*db*/ 0x0D1  , // ; Ð
/*.data:005E4EA9*/                 /*db*/ 0x0FD  , // ; ²
/*.data:005E4EAA*/                 /*db*/  0x8B  , // ; ï
/*.data:005E4EAB*/                 /*db*/  0x5A  , // ; Z
/*.data:005E4EAC*/                 /*db*/  0x8B  , // ; ï
/*.data:005E4EAD*/                 /*db*/ 0x0D8  , // ; Ï
/*.data:005E4EAE*/                 /*db*/  0x25  , // ; %
/*.data:005E4EAF*/                 /*db*/  0x5D  , // ; ]
/*.data:005E4EB0*/                 /*db*/  0x89  , // ; ë
/*.data:005E4EB1*/                 /*db*/ 0x0F9  , // ; ¨
/*.data:005E4EB2*/                 /*db*/ 0x0DB  , // ; ¦
/*.data:005E4EB3*/                 /*db*/  0x67  , // ; g
/*.data:005E4EB4*/                 /*db*/ 0x0AA  , // ; ¬
/*.data:005E4EB5*/                 /*db*/  0x95  , // ; ò
/*.data:005E4EB6*/                 /*db*/ 0x0F8  , // ; °
/*.data:005E4EB7*/                 /*db*/ 0x0F3  , // ; ¾
/*.data:005E4EB8*/                 /*db*/  0x27  , // ; '
/*.data:005E4EB9*/                 /*db*/ 0x0BF  , // ; +
/*.data:005E4EBA*/                 /*db*/ 0x0A2  , // ; ó
/*.data:005E4EBB*/                 /*db*/ 0x0C8  , // ; +
/*.data:005E4EBC*/                 /*db*/  0x5D  , // ; ]
/*.data:005E4EBD*/                 /*db*/ 0x0DD  , // ; ¦
/*.data:005E4EBE*/                 /*db*/  0x80  , // ; Ç
/*.data:005E4EBF*/                 /*db*/  0x6E  , // ; n
/*.data:005E4EC0*/                 /*db*/  0x4C  , // ; L
/*.data:005E4EC1*/                 /*db*/ 0x0C9  , // ; +
/*.data:005E4EC2*/                 /*db*/  0x9B  , // ; ø
/*.data:005E4EC3*/                 /*db*/  0x97  , // ; ù
/*.data:005E4EC4*/                 /*db*/  0x20  , // ;  
/*.data:005E4EC5*/                 /*db*/  0x8A  , // ; è
/*.data:005E4EC6*/                 /*db*/    0x2 , // ;  
/*.data:005E4EC7*/                 /*db*/  0x52  , // ; R
/*.data:005E4EC8*/                 /*db*/  0x60  , // ; `
/*.data:005E4EC9*/                 /*db*/ 0x0C4  , // ; -
/*.data:005E4ECA*/                 /*db*/  0x25  , // ; %
/*.data:005E4ECB*/                 /*db*/  0x75  , // ; u
/*.data:005E4ECC*/                 /*db*/    0x0 , // ;  
/*.data:005E4ECD*/                 /*db*/    0x0 , // ;  
/*.data:005E4ECE*/                 /*db*/    0x0 , // ;  
/*.data:005E4ECF*/                 /*db*/    0x0 }; // ;  
/*.data:005E4ED0*/ static BYTE __pow10neg[] = {      /*db*/ 0x0CD  , // ; -             ; DATA XREF: ___multtenpow12+2A.o
/*.data:005E4ED1*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED2*/                 /*db*/ 0x0CD  , // ; -
/*.data:005E4ED3*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED4*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED5*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED6*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED7*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED8*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4ED9*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4EDA*/                 /*db*/ 0x0FB  , // ; ¹
/*.data:005E4EDB*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4EDC*/                 /*db*/  0x71  , // ; q
/*.data:005E4EDD*/                 /*db*/  0x3D  , // ; =
/*.data:005E4EDE*/                 /*db*/  0x0A  , // ;  
/*.data:005E4EDF*/                 /*db*/ 0x0D7  , // ; Î
/*.data:005E4EE0*/                 /*db*/ 0x0A3  , // ; ú
/*.data:005E4EE1*/                 /*db*/  0x70  , // ; p
/*.data:005E4EE2*/                 /*db*/  0x3D  , // ; =
/*.data:005E4EE3*/                 /*db*/  0x0A  , // ;  
/*.data:005E4EE4*/                 /*db*/ 0x0D7  , // ; Î
/*.data:005E4EE5*/                 /*db*/ 0x0A3  , // ; ú
/*.data:005E4EE6*/                 /*db*/ 0x0F8  , // ; °
/*.data:005E4EE7*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4EE8*/                 /*db*/  0x5A  , // ; Z
/*.data:005E4EE9*/                 /*db*/  0x64  , // ; d
/*.data:005E4EEA*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4EEB*/                 /*db*/ 0x0DF  , // ; ¯
/*.data:005E4EEC*/                 /*db*/  0x4F  , // ; O
/*.data:005E4EED*/                 /*db*/  0x8D  , // ; ì
/*.data:005E4EEE*/                 /*db*/  0x97  , // ; ù
/*.data:005E4EEF*/                 /*db*/  0x6E  , // ; n
/*.data:005E4EF0*/                 /*db*/  0x12  , // ;  
/*.data:005E4EF1*/                 /*db*/  0x83  , // ; â
/*.data:005E4EF2*/                 /*db*/ 0x0F5  , // ; §
/*.data:005E4EF3*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4EF4*/                 /*db*/ 0x0C3  , // ; +
/*.data:005E4EF5*/                 /*db*/ 0x0D3  , // ; Ë
/*.data:005E4EF6*/                 /*db*/  0x2C  , // ; ,
/*.data:005E4EF7*/                 /*db*/  0x65  , // ; e
/*.data:005E4EF8*/                 /*db*/  0x19  , // ;  
/*.data:005E4EF9*/                 /*db*/ 0x0E2  , // ; Ô
/*.data:005E4EFA*/                 /*db*/  0x58  , // ; X
/*.data:005E4EFB*/                 /*db*/  0x17  , // ;  
/*.data:005E4EFC*/                 /*db*/ 0x0B7  , // ; À
/*.data:005E4EFD*/                 /*db*/ 0x0D1  , // ; Ð
/*.data:005E4EFE*/                 /*db*/ 0x0F1  , // ; ±
/*.data:005E4EFF*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F00*/                 /*db*/ 0x0D0  , // ; ð
/*.data:005E4F01*/                 /*db*/  0x0F  , // ;  
/*.data:005E4F02*/                 /*db*/  0x23  , // ; #
/*.data:005E4F03*/                 /*db*/  0x84  , // ; ä
/*.data:005E4F04*/                 /*db*/  0x47  , // ; G
/*.data:005E4F05*/                 /*db*/  0x1B  , // ;  
/*.data:005E4F06*/                 /*db*/  0x47  , // ; G
/*.data:005E4F07*/                 /*db*/ 0x0AC  , // ; ¼
/*.data:005E4F08*/                 /*db*/ 0x0C5  , // ; +
/*.data:005E4F09*/                 /*db*/ 0x0A7  , // ; º
/*.data:005E4F0A*/                 /*db*/ 0x0EE  , // ; ¯
/*.data:005E4F0B*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F0C*/                 /*db*/  0x40  , // ; @
/*.data:005E4F0D*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4F0E*/                 /*db*/ 0x0B6  , // ; Â
/*.data:005E4F0F*/                 /*db*/  0x69  , // ; i
/*.data:005E4F10*/                 /*db*/  0x6C  , // ; l
/*.data:005E4F11*/                 /*db*/ 0x0AF  , // ; »
/*.data:005E4F12*/                 /*db*/    0x5 , // ;  
/*.data:005E4F13*/                 /*db*/ 0x0BD  , // ; ¢
/*.data:005E4F14*/                 /*db*/  0x37  , // ; 7
/*.data:005E4F15*/                 /*db*/  0x86  , // ; å
/*.data:005E4F16*/                 /*db*/ 0x0EB  , // ; Ù
/*.data:005E4F17*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F18*/                 /*db*/  0x33  , // ; 3
/*.data:005E4F19*/                 /*db*/  0x3D  , // ; =
/*.data:005E4F1A*/                 /*db*/ 0x0BC  , // ; +
/*.data:005E4F1B*/                 /*db*/  0x42  , // ; B
/*.data:005E4F1C*/                 /*db*/  0x7A  , // ; z
/*.data:005E4F1D*/                 /*db*/ 0x0E5  , // ; Õ
/*.data:005E4F1E*/                 /*db*/ 0x0D5  , // ; i
/*.data:005E4F1F*/                 /*db*/  0x94  , // ; ö
/*.data:005E4F20*/                 /*db*/ 0x0BF  , // ; +
/*.data:005E4F21*/                 /*db*/ 0x0D6  , // ; Í
/*.data:005E4F22*/                 /*db*/ 0x0E7  , // ; þ
/*.data:005E4F23*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F24*/                 /*db*/ 0x0C2  , // ; -
/*.data:005E4F25*/                 /*db*/ 0x0FD  , // ; ²
/*.data:005E4F26*/                 /*db*/ 0x0FD  , // ; ²
/*.data:005E4F27*/                 /*db*/ 0x0CE  , // ; +
/*.data:005E4F28*/                 /*db*/  0x61  , // ; a
/*.data:005E4F29*/                 /*db*/  0x84  , // ; ä
/*.data:005E4F2A*/                 /*db*/  0x11  , // ;  
/*.data:005E4F2B*/                 /*db*/  0x77  , // ; w
/*.data:005E4F2C*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4F2D*/                 /*db*/ 0x0AB  , // ; ½
/*.data:005E4F2E*/                 /*db*/ 0x0E4  , // ; õ
/*.data:005E4F2F*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F30*/                 /*db*/  0x2F  , // ; /
/*.data:005E4F31*/                 /*db*/  0x4C  , // ; L
/*.data:005E4F32*/                 /*db*/  0x5B  , // ; [
/*.data:005E4F33*/                 /*db*/ 0x0E1  , // ; ß
/*.data:005E4F34*/                 /*db*/  0x4D  , // ; M
/*.data:005E4F35*/                 /*db*/ 0x0C4  , // ; -
/*.data:005E4F36*/                 /*db*/ 0x0BE  , // ; ¥
/*.data:005E4F37*/                 /*db*/  0x94  , // ; ö
/*.data:005E4F38*/                 /*db*/  0x95  , // ; ò
/*.data:005E4F39*/                 /*db*/ 0x0E6  , // ; µ
/*.data:005E4F3A*/                 /*db*/ 0x0C9  , // ; +
/*.data:005E4F3B*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F3C*/                 /*db*/  0x92  , // ; Æ
/*.data:005E4F3D*/                 /*db*/ 0x0C4  , // ; -
/*.data:005E4F3E*/                 /*db*/  0x53  , // ; S
/*.data:005E4F3F*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4F40*/                 /*db*/  0x75  , // ; u
/*.data:005E4F41*/                 /*db*/  0x44  , // ; D
/*.data:005E4F42*/                 /*db*/ 0x0CD  , // ; -
/*.data:005E4F43*/                 /*db*/  0x14  , // ;  
/*.data:005E4F44*/                 /*db*/ 0x0BE  , // ; ¥
/*.data:005E4F45*/                 /*db*/  0x9A  , // ; Ü
/*.data:005E4F46*/                 /*db*/ 0x0AF  , // ; »
/*.data:005E4F47*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F48*/                 /*db*/ 0x0DE  , // ; Ì
/*.data:005E4F49*/                 /*db*/  0x67  , // ; g
/*.data:005E4F4A*/                 /*db*/ 0x0BA  , // ; ¦
/*.data:005E4F4B*/                 /*db*/  0x94  , // ; ö
/*.data:005E4F4C*/                 /*db*/  0x39  , // ; 9
/*.data:005E4F4D*/                 /*db*/  0x45  , // ; E
/*.data:005E4F4E*/                 /*db*/ 0x0AD  , // ; ¡
/*.data:005E4F4F*/                 /*db*/  0x1E  , // ;  
/*.data:005E4F50*/                 /*db*/ 0x0B1  , // ; ¦
/*.data:005E4F51*/                 /*db*/ 0x0CF  , // ; ¤
/*.data:005E4F52*/                 /*db*/  0x94  , // ; ö
/*.data:005E4F53*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F54*/                 /*db*/  0x24  , // ; $
/*.data:005E4F55*/                 /*db*/  0x23  , // ; #
/*.data:005E4F56*/                 /*db*/ 0x0C6  , // ; ã
/*.data:005E4F57*/                 /*db*/ 0x0E2  , // ; Ô
/*.data:005E4F58*/                 /*db*/ 0x0BC  , // ; +
/*.data:005E4F59*/                 /*db*/ 0x0BA  , // ; ¦
/*.data:005E4F5A*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4F5B*/                 /*db*/  0x31  , // ; 1
/*.data:005E4F5C*/                 /*db*/  0x61  , // ; a
/*.data:005E4F5D*/                 /*db*/  0x8B  , // ; ï
/*.data:005E4F5E*/                 /*db*/  0x7A  , // ; z
/*.data:005E4F5F*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F60*/                 /*db*/  0x61  , // ; a
/*.data:005E4F61*/                 /*db*/  0x55  , // ; U
/*.data:005E4F62*/                 /*db*/  0x59  , // ; Y
/*.data:005E4F63*/                 /*db*/ 0x0C1  , // ; -
/*.data:005E4F64*/                 /*db*/  0x7E  , // ; ~
/*.data:005E4F65*/                 /*db*/ 0x0B1  , // ; ¦
/*.data:005E4F66*/                 /*db*/  0x53  , // ; S
/*.data:005E4F67*/                 /*db*/  0x7C  , // ; |
/*.data:005E4F68*/                 /*db*/  0x12  , // ;  
/*.data:005E4F69*/                 /*db*/ 0x0BB  , // ; +
/*.data:005E4F6A*/                 /*db*/  0x5F  , // ; _
/*.data:005E4F6B*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F6C*/                 /*db*/ 0x0D7  , // ; Î
/*.data:005E4F6D*/                 /*db*/ 0x0EE  , // ; ¯
/*.data:005E4F6E*/                 /*db*/  0x2F  , // ; /
/*.data:005E4F6F*/                 /*db*/  0x8D  , // ; ì
/*.data:005E4F70*/                 /*db*/    0x6 , // ;  
/*.data:005E4F71*/                 /*db*/ 0x0BE  , // ; ¥
/*.data:005E4F72*/                 /*db*/  0x92  , // ; Æ
/*.data:005E4F73*/                 /*db*/  0x85  , // ; à
/*.data:005E4F74*/                 /*db*/  0x15  , // ;  
/*.data:005E4F75*/                 /*db*/ 0x0FB  , // ; ¹
/*.data:005E4F76*/                 /*db*/  0x44  , // ; D
/*.data:005E4F77*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F78*/                 /*db*/  0x24  , // ; $
/*.data:005E4F79*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F7A*/                 /*db*/ 0x0A5  , // ; Ñ
/*.data:005E4F7B*/                 /*db*/ 0x0E9  , // ; Ú
/*.data:005E4F7C*/                 /*db*/  0x39  , // ; 9
/*.data:005E4F7D*/                 /*db*/ 0x0A5  , // ; Ñ
/*.data:005E4F7E*/                 /*db*/  0x27  , // ; '
/*.data:005E4F7F*/                 /*db*/ 0x0EA  , // ; Û
/*.data:005E4F80*/                 /*db*/  0x7F  , // ; 
/*.data:005E4F81*/                 /*db*/ 0x0A8  , // ; ¿
/*.data:005E4F82*/                 /*db*/  0x2A  , // ; *
/*.data:005E4F83*/                 /*db*/  0x3F  , // ; ?
/*.data:005E4F84*/                 /*db*/  0x7D  , // ; }
/*.data:005E4F85*/                 /*db*/ 0x0AC  , // ; ¼
/*.data:005E4F86*/                 /*db*/ 0x0A1  , // ; í
/*.data:005E4F87*/                 /*db*/ 0x0E4  , // ; õ
/*.data:005E4F88*/                 /*db*/ 0x0BC  , // ; +
/*.data:005E4F89*/                 /*db*/  0x64  , // ; d
/*.data:005E4F8A*/                 /*db*/  0x7C  , // ; |
/*.data:005E4F8B*/                 /*db*/  0x46  , // ; F
/*.data:005E4F8C*/                 /*db*/ 0x0D0  , // ; ð
/*.data:005E4F8D*/                 /*db*/ 0x0DD  , // ; ¦
/*.data:005E4F8E*/                 /*db*/  0x55  , // ; U
/*.data:005E4F8F*/                 /*db*/  0x3E  , // ; >
/*.data:005E4F90*/                 /*db*/  0x63  , // ; c
/*.data:005E4F91*/                 /*db*/  0x7B  , // ; {
/*.data:005E4F92*/                 /*db*/    0x6 , // ;  
/*.data:005E4F93*/                 /*db*/ 0x0CC  , // ; ¦
/*.data:005E4F94*/                 /*db*/  0x23  , // ; #
/*.data:005E4F95*/                 /*db*/  0x54  , // ; T
/*.data:005E4F96*/                 /*db*/  0x77  , // ; w
/*.data:005E4F97*/                 /*db*/  0x83  , // ; â
/*.data:005E4F98*/                 /*db*/ 0x0FF  , // ;  
/*.data:005E4F99*/                 /*db*/  0x91  , // ; æ
/*.data:005E4F9A*/                 /*db*/  0x81  , // ; ü
/*.data:005E4F9B*/                 /*db*/  0x3D  , // ; =
/*.data:005E4F9C*/                 /*db*/  0x91  , // ; æ
/*.data:005E4F9D*/                 /*db*/ 0x0FA  , // ; ·
/*.data:005E4F9E*/                 /*db*/  0x3A  , // ; :
/*.data:005E4F9F*/                 /*db*/  0x19  , // ;  
/*.data:005E4FA0*/                 /*db*/  0x7A  , // ; z
/*.data:005E4FA1*/                 /*db*/  0x63  , // ; c
/*.data:005E4FA2*/                 /*db*/  0x25  , // ; %
/*.data:005E4FA3*/                 /*db*/  0x43  , // ; C
/*.data:005E4FA4*/                 /*db*/  0x31  , // ; 1
/*.data:005E4FA5*/                 /*db*/ 0x0C0  , // ; +
/*.data:005E4FA6*/                 /*db*/ 0x0AC  , // ; ¼
/*.data:005E4FA7*/                 /*db*/  0x3C  , // ; <
/*.data:005E4FA8*/                 /*db*/  0x21  , // ; !
/*.data:005E4FA9*/                 /*db*/  0x89  , // ; ë
/*.data:005E4FAA*/                 /*db*/ 0x0D1  , // ; Ð
/*.data:005E4FAB*/                 /*db*/  0x38  , // ; 8
/*.data:005E4FAC*/                 /*db*/  0x82  , // ; é
/*.data:005E4FAD*/                 /*db*/  0x47  , // ; G
/*.data:005E4FAE*/                 /*db*/  0x97  , // ; ù
/*.data:005E4FAF*/                 /*db*/ 0x0B8  , // ; ©
/*.data:005E4FB0*/                 /*db*/    0x0 , // ;  
/*.data:005E4FB1*/                 /*db*/ 0x0FD  , // ; ²
/*.data:005E4FB2*/                 /*db*/ 0x0D7  , // ; Î
/*.data:005E4FB3*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4FB4*/                 /*db*/ 0x0DC  , // ; _
/*.data:005E4FB5*/                 /*db*/  0x88  , // ; ê
/*.data:005E4FB6*/                 /*db*/  0x58  , // ; X
/*.data:005E4FB7*/                 /*db*/    0x8 , // ;  
/*.data:005E4FB8*/                 /*db*/  0x1B  , // ;  
/*.data:005E4FB9*/                 /*db*/ 0x0B1  , // ; ¦
/*.data:005E4FBA*/                 /*db*/ 0x0E8  , // ; Þ
/*.data:005E4FBB*/                 /*db*/ 0x0E3  , // ; Ò
/*.data:005E4FBC*/                 /*db*/  0x86  , // ; å
/*.data:005E4FBD*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4FBE*/                 /*db*/    0x3 , // ;  
/*.data:005E4FBF*/                 /*db*/  0x3B  , // ; ;
/*.data:005E4FC0*/                 /*db*/ 0x0C6  , // ; ã
/*.data:005E4FC1*/                 /*db*/  0x84  , // ; ä
/*.data:005E4FC2*/                 /*db*/  0x45  , // ; E
/*.data:005E4FC3*/                 /*db*/  0x42  , // ; B
/*.data:005E4FC4*/                 /*db*/    0x7 , // ;  
/*.data:005E4FC5*/                 /*db*/ 0x0B6  , // ; Â
/*.data:005E4FC6*/                 /*db*/  0x99  , // ; Ö
/*.data:005E4FC7*/                 /*db*/  0x75  , // ; u
/*.data:005E4FC8*/                 /*db*/  0x37  , // ; 7
/*.data:005E4FC9*/                 /*db*/ 0x0DB  , // ; ¦
/*.data:005E4FCA*/                 /*db*/  0x2E  , // ; .
/*.data:005E4FCB*/                 /*db*/  0x3A  , // ; :
/*.data:005E4FCC*/                 /*db*/  0x33  , // ; 3
/*.data:005E4FCD*/                 /*db*/  0x71  , // ; q
/*.data:005E4FCE*/                 /*db*/  0x1C  , // ;  
/*.data:005E4FCF*/                 /*db*/ 0x0D2  , // ; Ê
/*.data:005E4FD0*/                 /*db*/  0x23  , // ; #
/*.data:005E4FD1*/                 /*db*/ 0x0DB  , // ; ¦
/*.data:005E4FD2*/                 /*db*/  0x32  , // ; 2
/*.data:005E4FD3*/                 /*db*/ 0x0EE  , // ; ¯
/*.data:005E4FD4*/                 /*db*/  0x49  , // ; I
/*.data:005E4FD5*/                 /*db*/  0x90  , // ; É
/*.data:005E4FD6*/                 /*db*/  0x5A  , // ; Z
/*.data:005E4FD7*/                 /*db*/  0x39  , // ; 9
/*.data:005E4FD8*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4FD9*/                 /*db*/  0x87  , // ; ç
/*.data:005E4FDA*/                 /*db*/ 0x0BE  , // ; ¥
/*.data:005E4FDB*/                 /*db*/ 0x0C0  , // ; +
/*.data:005E4FDC*/                 /*db*/  0x57  , // ; W
/*.data:005E4FDD*/                 /*db*/ 0x0DA  , // ; +
/*.data:005E4FDE*/                 /*db*/ 0x0A5  , // ; Ñ
/*.data:005E4FDF*/                 /*db*/  0x82  , // ; é
/*.data:005E4FE0*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E4FE1*/                 /*db*/ 0x0A2  , // ; ó
/*.data:005E4FE2*/                 /*db*/ 0x0B5  , // ; Á
/*.data:005E4FE3*/                 /*db*/  0x32  , // ; 2
/*.data:005E4FE4*/                 /*db*/ 0x0E2  , // ; Ô
/*.data:005E4FE5*/                 /*db*/  0x68  , // ; h
/*.data:005E4FE6*/                 /*db*/ 0x0B2  , // ; ¦
/*.data:005E4FE7*/                 /*db*/  0x11  , // ;  
/*.data:005E4FE8*/                 /*db*/ 0x0A7  , // ; º
/*.data:005E4FE9*/                 /*db*/  0x52  , // ; R
/*.data:005E4FEA*/                 /*db*/  0x9F  , // ; ƒ
/*.data:005E4FEB*/                 /*db*/  0x44  , // ; D
/*.data:005E4FEC*/                 /*db*/  0x59  , // ; Y
/*.data:005E4FED*/                 /*db*/ 0x0B7  , // ; À
/*.data:005E4FEE*/                 /*db*/  0x10  , // ;  
/*.data:005E4FEF*/                 /*db*/  0x2C  , // ; ,
/*.data:005E4FF0*/                 /*db*/  0x25  , // ; %
/*.data:005E4FF1*/                 /*db*/  0x49  , // ; I
/*.data:005E4FF2*/                 /*db*/ 0x0E4  , // ; õ
/*.data:005E4FF3*/                 /*db*/  0x2D  , // ; -
/*.data:005E4FF4*/                 /*db*/  0x36  , // ; 6
/*.data:005E4FF5*/                 /*db*/  0x34  , // ; 4
/*.data:005E4FF6*/                 /*db*/  0x4F  , // ; O
/*.data:005E4FF7*/                 /*db*/  0x53  , // ; S
/*.data:005E4FF8*/                 /*db*/ 0x0AE  , // ; «
/*.data:005E4FF9*/                 /*db*/ 0x0CE  , // ; +
/*.data:005E4FFA*/                 /*db*/  0x6B  , // ; k
/*.data:005E4FFB*/                 /*db*/  0x25  , // ; %
/*.data:005E4FFC*/                 /*db*/  0x8F  , // ; Å
/*.data:005E4FFD*/                 /*db*/  0x59  , // ; Y
/*.data:005E4FFE*/                 /*db*/    0x4 , // ;  
/*.data:005E4FFF*/                 /*db*/ 0x0A4  , // ; ñ
/*.data:005E5000*/                 /*db*/ 0x0C0  , // ; +
/*.data:005E5001*/                 /*db*/ 0x0DE  , // ; Ì
/*.data:005E5002*/                 /*db*/ 0x0C2  , // ; -
/*.data:005E5003*/                 /*db*/  0x7D  , // ; }
/*.data:005E5004*/                 /*db*/ 0x0FB  , // ; ¹
/*.data:005E5005*/                 /*db*/ 0x0E8  , // ; Þ
/*.data:005E5006*/                 /*db*/ 0x0C6  , // ; ã
/*.data:005E5007*/                 /*db*/  0x1E  , // ;  
/*.data:005E5008*/                 /*db*/  0x9E  , // ; ×
/*.data:005E5009*/                 /*db*/ 0x0E7  , // ; þ
/*.data:005E500A*/                 /*db*/  0x88  , // ; ê
/*.data:005E500B*/                 /*db*/  0x5A  , // ; Z
/*.data:005E500C*/                 /*db*/  0x57  , // ; W
/*.data:005E500D*/                 /*db*/  0x91  , // ; æ
/*.data:005E500E*/                 /*db*/  0x3C  , // ; <
/*.data:005E500F*/                 /*db*/ 0x0BF  , // ; +
/*.data:005E5010*/                 /*db*/  0x50  , // ; P
/*.data:005E5011*/                 /*db*/  0x83  , // ; â
/*.data:005E5012*/                 /*db*/  0x22  , // ; "
/*.data:005E5013*/                 /*db*/  0x18  , // ;  
/*.data:005E5014*/                 /*db*/  0x4E  , // ; N
/*.data:005E5015*/                 /*db*/  0x4B  , // ; K
/*.data:005E5016*/                 /*db*/  0x65  , // ; e
/*.data:005E5017*/                 /*db*/  0x62  , // ; b
/*.data:005E5018*/                 /*db*/ 0x0FD  , // ; ²
/*.data:005E5019*/                 /*db*/  0x83  , // ; â
/*.data:005E501A*/                 /*db*/  0x8F  , // ; Å
/*.data:005E501B*/                 /*db*/ 0x0AF  , // ; »
/*.data:005E501C*/                 /*db*/    0x6 , // ;  
/*.data:005E501D*/                 /*db*/  0x94  , // ; ö
/*.data:005E501E*/                 /*db*/  0x7D  , // ; }
/*.data:005E501F*/                 /*db*/  0x11  , // ;  
/*.data:005E5020*/                 /*db*/ 0x0E4  , // ; õ
/*.data:005E5021*/                 /*db*/  0x2D  , // ; -
/*.data:005E5022*/                 /*db*/ 0x0DE  , // ; Ì
/*.data:005E5023*/                 /*db*/  0x9F  , // ; ƒ
/*.data:005E5024*/                 /*db*/ 0x0CE  , // ; +
/*.data:005E5025*/                 /*db*/ 0x0D2  , // ; Ê
/*.data:005E5026*/                 /*db*/ 0x0C8  , // ; +
/*.data:005E5027*/                 /*db*/    0x4 , // ;  
/*.data:005E5028*/                 /*db*/ 0x0DD  , // ; ¦
/*.data:005E5029*/                 /*db*/ 0x0A6  , // ; ª
/*.data:005E502A*/                 /*db*/ 0x0D8  , // ; Ï
/*.data:005E502B*/                 /*db*/  0x0A };  // ;  

//===============================
// Grabbed Functions (from CRT).
//===============================

static VOID __declspec( naked ) ___dtold( VOID )
{
	__asm
	{
		/*.text:00433CC0*/ // ___dtold        proc near               ; CODE XREF: __fltout2+E.p
		/*.text:00433CC0*/ 
		/*.text:00433CC0*/ #define var_1C           dword ptr [ebp-1Ch]
		/*.text:00433CC0*/ #define var_18           dword ptr [ebp-18h]
		/*.text:00433CC0*/ #define var_14           dword ptr [ebp-14h]
		/*.text:00433CC0*/ #define var_10           dword ptr [ebp-10h]
		/*.text:00433CC0*/ #define var_C            dword ptr [ebp-0Ch]
		/*.text:00433CC0*/ #define var_8            dword ptr [ebp-8]
		/*.text:00433CC0*/ #define var_4            dword ptr [ebp-4]
		/*.text:00433CC0*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:00433CC0*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:00433CC0*/ 
		/*.text:00433CC0*/                 push    ebp
		/*.text:00433CC1*/                 mov     ebp, esp
		/*.text:00433CC3*/                 sub     esp, 1Ch
		/*.text:00433CC6*/                 mov     /*[ebp+*/ var_C /*]*/, 80000000h
		/*.text:00433CCD*/                 mov     word ptr [ebp-4], 0
		/*.text:00433CD3*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00433CD6*/                 xor     ecx, ecx
		/*.text:00433CD8*/                 mov     cx, [eax+6]
		/*.text:00433CDC*/                 and     ecx, 7FF0h
		/*.text:00433CE2*/                 sar     ecx, 4
		/*.text:00433CE5*/                 mov     [ebp-14h], cx
		/*.text:00433CE9*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00433CEC*/                 xor     eax, eax
		/*.text:00433CEE*/                 mov     ax, [edx+6]
		/*.text:00433CF2*/                 and     eax, 8000h
		/*.text:00433CF7*/                 mov     [ebp-18h], ax
		/*.text:00433CFB*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00433CFE*/                 mov     edx, [ecx+4]
		/*.text:00433D01*/                 and     edx, 0FFFFFh
		/*.text:00433D07*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00433D0A*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00433D0D*/                 mov     ecx, [eax]
		/*.text:00433D0F*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:00433D12*/                 mov     edx, /*[ebp+*/ var_14 /*]*/
		/*.text:00433D15*/                 and     edx, 0FFFFh
		/*.text:00433D1B*/                 mov     /*[ebp+*/ var_1C /*]*/, edx
		/*.text:00433D1E*/                 cmp     /*[ebp+*/ var_1C /*]*/, 0
		/*.text:00433D22*/                 jz      short loc_433D37
		/*.text:00433D24*/                 cmp     /*[ebp+*/ var_1C /*]*/, 7FFh
		/*.text:00433D2B*/                 jz      short loc_433D2F
		/*.text:00433D2D*/                 jmp     short loc_433D7A
		/*.text:00433D2F*/ ; ---------------------------------------------------------------------------
		/*.text:00433D2F*/ 
		/*.text:00433D2F*/ loc_433D2F:                             ; CODE XREF: ___dtold+6B.j
		/*.text:00433D2F*/                 mov     word ptr [ebp-4], 7FFFh
		/*.text:00433D35*/                 jmp     short loc_433D95
		/*.text:00433D37*/ ; ---------------------------------------------------------------------------
		/*.text:00433D37*/ 
		/*.text:00433D37*/ loc_433D37:                             ; CODE XREF: ___dtold+62.j
		/*.text:00433D37*/                 cmp     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:00433D3B*/                 jnz     short loc_433D64
		/*.text:00433D3D*/                 cmp     /*[ebp+*/ var_10 /*]*/, 0
		/*.text:00433D41*/                 jnz     short loc_433D64
		/*.text:00433D43*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433D46*/                 mov     dword ptr [eax+4], 0
		/*.text:00433D4D*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433D50*/                 mov     dword ptr [ecx], 0
		/*.text:00433D56*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433D59*/                 mov     word ptr [edx+8], 0
		/*.text:00433D5F*/                 jmp     loc_433E1E
		/*.text:00433D64*/ ; ---------------------------------------------------------------------------
		/*.text:00433D64*/ 
		/*.text:00433D64*/ loc_433D64:                             ; CODE XREF: ___dtold+7B.j
		/*.text:00433D64*/                                         ; ___dtold+81.j
		/*.text:00433D64*/                 movsx   eax, word ptr [ebp-14h]
		/*.text:00433D68*/                 add     eax, 3C01h
		/*.text:00433D6D*/                 mov     [ebp-4], ax
		/*.text:00433D71*/                 mov     /*[ebp+*/ var_C /*]*/, 0
		/*.text:00433D78*/                 jmp     short loc_433D95
		/*.text:00433D7A*/ ; ---------------------------------------------------------------------------
		/*.text:00433D7A*/ 
		/*.text:00433D7A*/ loc_433D7A:                             ; CODE XREF: ___dtold+6D.j
		/*.text:00433D7A*/                 mov     cx, [ebp-14h]
		/*.text:00433D7E*/                 sub     cx, 3FFh
		/*.text:00433D83*/                 mov     [ebp-14h], cx
		/*.text:00433D87*/                 movsx   edx, word ptr [ebp-14h]
		/*.text:00433D8B*/                 add     edx, 3FFFh
		/*.text:00433D91*/                 mov     [ebp-4], dx
		/*.text:00433D95*/ 
		/*.text:00433D95*/ loc_433D95:                             ; CODE XREF: ___dtold+75.j
		/*.text:00433D95*/                                         ; ___dtold+B8.j
		/*.text:00433D95*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00433D98*/                 shl     eax, 0Bh
		/*.text:00433D9B*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00433D9E*/                 or      ecx, eax
		/*.text:00433DA0*/                 mov     edx, /*[ebp+*/ var_10 /*]*/
		/*.text:00433DA3*/                 shr     edx, 15h
		/*.text:00433DA6*/                 or      ecx, edx
		/*.text:00433DA8*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DAB*/                 mov     [eax+4], ecx
		/*.text:00433DAE*/                 mov     ecx, /*[ebp+*/ var_10 /*]*/
		/*.text:00433DB1*/                 shl     ecx, 0Bh
		/*.text:00433DB4*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DB7*/                 mov     [edx], ecx
		/*.text:00433DB9*/ 
		/*.text:00433DB9*/ loc_433DB9:                             ; CODE XREF: ___dtold+142.j
		/*.text:00433DB9*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DBC*/                 mov     ecx, [eax+4]
		/*.text:00433DBF*/                 and     ecx, 80000000h
		/*.text:00433DC5*/                 test    ecx, ecx
		/*.text:00433DC7*/                 jnz     short loc_433E04
		/*.text:00433DC9*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DCC*/                 mov     eax, [edx+4]
		/*.text:00433DCF*/                 shl     eax, 1
		/*.text:00433DD1*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DD4*/                 mov     edx, [ecx]
		/*.text:00433DD6*/                 and     edx, 80000000h
		/*.text:00433DDC*/                 neg     edx
		/*.text:00433DDE*/                 sbb     edx, edx
		/*.text:00433DE0*/                 neg     edx
		/*.text:00433DE2*/                 or      eax, edx
		/*.text:00433DE4*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DE7*/                 mov     [ecx+4], eax
		/*.text:00433DEA*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DED*/                 mov     eax, [edx]
		/*.text:00433DEF*/                 shl     eax, 1
		/*.text:00433DF1*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433DF4*/                 mov     [ecx], eax
		/*.text:00433DF6*/                 mov     dx, [ebp-4]
		/*.text:00433DFA*/                 sub     dx, 1
		/*.text:00433DFE*/                 mov     [ebp-4], dx
		/*.text:00433E02*/                 jmp     short loc_433DB9
		/*.text:00433E04*/ ; ---------------------------------------------------------------------------
		/*.text:00433E04*/ 
		/*.text:00433E04*/ loc_433E04:                             ; CODE XREF: ___dtold+107.j
		/*.text:00433E04*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:00433E07*/                 and     eax, 0FFFFh
		/*.text:00433E0C*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00433E0F*/                 and     ecx, 0FFFFh
		/*.text:00433E15*/                 or      eax, ecx
		/*.text:00433E17*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433E1A*/                 mov     [edx+8], ax
		/*.text:00433E1E*/ 
		/*.text:00433E1E*/ loc_433E1E:                             ; CODE XREF: ___dtold+9F.j
		/*.text:00433E1E*/                 mov     esp, ebp
		/*.text:00433E20*/                 pop     ebp
		/*.text:00433E21*/                 retn
		/*.text:00433E21*/ // ___dtold        endp
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

static VOID __declspec( naked ) ___addl( VOID )
{
	__asm
	{
		/*.text:00439260*/ // ___addl         proc near               ; CODE XREF: __IncMan+53.p
		/*.text:00439260*/                                         ; __IncMan+94.p ...
		/*.text:00439260*/ 
		/*.text:00439260*/ #define var_8            dword ptr [ebp-8]
		/*.text:00439260*/ #define var_4            dword ptr [ebp-4]
		/*.text:00439260*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:00439260*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:00439260*/ #define arg_8            dword ptr  [ebp+10h]
		/*.text:00439260*/ 
		/*.text:00439260*/                 push    ebp
		/*.text:00439261*/                 mov     ebp, esp
		/*.text:00439263*/                 sub     esp, 8
		/*.text:00439266*/                 mov     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:0043926D*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439270*/                 add     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00439273*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00439276*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00439279*/                 cmp     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043927C*/                 jb      short loc_439286
		/*.text:0043927E*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00439281*/                 cmp     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00439284*/                 jnb     short loc_43928F
		/*.text:00439286*/ 
		/*.text:00439286*/ loc_439286:                             ; CODE XREF: ___addl+1C.j
		/*.text:00439286*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00439289*/                 add     eax, 1
		/*.text:0043928C*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0043928F*/ 
		/*.text:0043928F*/ loc_43928F:                             ; CODE XREF: ___addl+24.j
		/*.text:0043928F*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00439292*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00439295*/                 mov     [ecx], edx
		/*.text:00439297*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0043929A*/                 mov     esp, ebp
		/*.text:0043929C*/                 pop     ebp
		/*.text:0043929D*/                 retn
		/*.text:0043929D*/ // ___addl         endp
	}
}

#undef var_8
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8

static VOID __declspec( naked ) ___shl_12( VOID )
{
	__asm
	{
		/*.text:00439350*/ // ___shl_12       proc near               ; CODE XREF: _$I10_OUTPUT+350.p
		/*.text:00439350*/                                         ; _$I10_OUTPUT+3C5.p ...
		/*.text:00439350*/ 
		/*.text:00439350*/ #define var_8            dword ptr [ebp-8]
		/*.text:00439350*/ #define var_4            dword ptr [ebp-4]
		/*.text:00439350*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:00439350*/ 
		/*.text:00439350*/                 push    ebp
		/*.text:00439351*/                 mov     ebp, esp
		/*.text:00439353*/                 sub     esp, 8
		/*.text:00439356*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439359*/                 mov     ecx, [eax]
		/*.text:0043935B*/                 and     ecx, 80000000h
		/*.text:00439361*/                 neg     ecx
		/*.text:00439363*/                 sbb     ecx, ecx
		/*.text:00439365*/                 neg     ecx
		/*.text:00439367*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0043936A*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043936D*/                 mov     eax, [edx+4]
		/*.text:00439370*/                 and     eax, 80000000h
		/*.text:00439375*/                 neg     eax
		/*.text:00439377*/                 sbb     eax, eax
		/*.text:00439379*/                 neg     eax
		/*.text:0043937B*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0043937E*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439381*/                 mov     edx, [ecx]
		/*.text:00439383*/                 shl     edx, 1
		/*.text:00439385*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439388*/                 mov     [eax], edx
		/*.text:0043938A*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043938D*/                 mov     edx, [ecx+4]
		/*.text:00439390*/                 shl     edx, 1
		/*.text:00439392*/                 or      edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00439395*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439398*/                 mov     [eax+4], edx
		/*.text:0043939B*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043939E*/                 mov     edx, [ecx+8]
		/*.text:004393A1*/                 shl     edx, 1
		/*.text:004393A3*/                 or      edx, /*[ebp+*/ var_8 /*]*/
		/*.text:004393A6*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393A9*/                 mov     [eax+8], edx
		/*.text:004393AC*/                 mov     esp, ebp
		/*.text:004393AE*/                 pop     ebp
		/*.text:004393AF*/                 retn
		/*.text:004393AF*/ // ___shl_12       endp
	}
}

#undef var_8
#undef var_4
#undef arg_0

static VOID __declspec( naked ) ___shr_12( VOID )
{
	__asm
	{
		/*.text:004393B0*/ // ___shr_12       proc near               ; CODE XREF: _$I10_OUTPUT+382.p
		/*.text:004393B0*/                                         ; ___ld12mul+316.p
		/*.text:004393B0*/ 
		/*.text:004393B0*/ #define var_8            dword ptr [ebp-8]
		/*.text:004393B0*/ #define var_4            dword ptr [ebp-4]
		/*.text:004393B0*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:004393B0*/ 
		/*.text:004393B0*/                 push    ebp
		/*.text:004393B1*/                 mov     ebp, esp
		/*.text:004393B3*/                 sub     esp, 8
		/*.text:004393B6*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393B9*/                 mov     ecx, [eax+8]
		/*.text:004393BC*/                 and     ecx, 1
		/*.text:004393BF*/                 neg     ecx
		/*.text:004393C1*/                 sbb     ecx, ecx
		/*.text:004393C3*/                 and     ecx, 80000000h
		/*.text:004393C9*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:004393CC*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393CF*/                 mov     eax, [edx+4]
		/*.text:004393D2*/                 and     eax, 1
		/*.text:004393D5*/                 neg     eax
		/*.text:004393D7*/                 sbb     eax, eax
		/*.text:004393D9*/                 and     eax, 80000000h
		/*.text:004393DE*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004393E1*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393E4*/                 mov     edx, [ecx+8]
		/*.text:004393E7*/                 shr     edx, 1
		/*.text:004393E9*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393EC*/                 mov     [eax+8], edx
		/*.text:004393EF*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393F2*/                 mov     edx, [ecx+4]
		/*.text:004393F5*/                 shr     edx, 1
		/*.text:004393F7*/                 or      edx, /*[ebp+*/ var_8 /*]*/
		/*.text:004393FA*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004393FD*/                 mov     [eax+4], edx
		/*.text:00439400*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439403*/                 mov     edx, [ecx]
		/*.text:00439405*/                 shr     edx, 1
		/*.text:00439407*/                 or      edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043940A*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043940D*/                 mov     [eax], edx
		/*.text:0043940F*/                 mov     esp, ebp
		/*.text:00439411*/                 pop     ebp
		/*.text:00439412*/                 retn
		/*.text:00439412*/ // ___shr_12       endp
	}
}

#undef var_8
#undef var_4
#undef arg_0

static VOID __declspec( naked ) ___ld12mul( VOID )
{
	__asm
	{
		/*.text:0043AA90*/ // ___ld12mul      proc near               ; CODE XREF: _$I10_OUTPUT+2B5.p
		/*.text:0043AA90*/                                         ; ___multtenpow12+B5.p
		/*.text:0043AA90*/ 
		/*.text:0043AA90*/ #define var_48           dword ptr [ebp-48h]
		/*.text:0043AA90*/ #define var_44           dword ptr [ebp-44h]
		/*.text:0043AA90*/ #define var_40           dword ptr [ebp-40h]
		/*.text:0043AA90*/ #define var_3C           dword ptr [ebp-3Ch]
		/*.text:0043AA90*/ #define var_38           dword ptr [ebp-38h]
		/*.text:0043AA90*/ #define var_34           dword ptr [ebp-34h]
		/*.text:0043AA90*/ #define var_30           dword ptr [ebp-30h]
		/*.text:0043AA90*/ #define var_2C           dword ptr [ebp-2Ch]
		/*.text:0043AA90*/ #define var_28           dword ptr [ebp-28h]
		/*.text:0043AA90*/ #define var_24           dword ptr [ebp-24h]
		/*.text:0043AA90*/ #define var_20           dword ptr [ebp-20h]
		/*.text:0043AA90*/ #define var_1C           dword ptr [ebp-1Ch]
		/*.text:0043AA90*/ #define var_18           dword ptr [ebp-18h]
		/*.text:0043AA90*/ #define var_14           dword ptr [ebp-14h]
		/*.text:0043AA90*/ #define var_10           dword ptr [ebp-10h]
		/*.text:0043AA90*/ #define var_C            dword ptr [ebp-0Ch]
		/*.text:0043AA90*/ #define var_8            dword ptr [ebp-8]
		/*.text:0043AA90*/ #define var_4            dword ptr [ebp-4]
		/*.text:0043AA90*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:0043AA90*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:0043AA90*/ 
		/*.text:0043AA90*/                 push    ebp
		/*.text:0043AA91*/                 mov     ebp, esp
		/*.text:0043AA93*/                 sub     esp, 48h
		/*.text:0043AA96*/                 mov     word ptr /*[ebp+var_2C]*/ [ebp-2ch], 0
		/*.text:0043AA9C*/                 mov     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0043AAA3*/                 mov     /*[ebp+*/ var_14 /*]*/, 0
		/*.text:0043AAAA*/                 mov     /*[ebp+*/ var_10 /*]*/, 0
		/*.text:0043AAB1*/                 mov     /*[ebp+*/ var_C /*]*/, 0
		/*.text:0043AAB8*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AABB*/                 mov     cx, [eax+0Ah]
		/*.text:0043AABF*/                 mov     word ptr /*[ebp+var_18]*/ [ebp-18h], cx
		/*.text:0043AAC3*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AAC6*/                 mov     ax, [edx+0Ah]
		/*.text:0043AACA*/                 mov     word ptr /*[ebp+var_20]*/ [ebp-20h], ax
		/*.text:0043AACE*/                 mov     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0043AAD1*/                 and     ecx, 0FFFFh
		/*.text:0043AAD7*/                 mov     edx, /*[ebp+*/ var_20 /*]*/
		/*.text:0043AADA*/                 and     edx, 0FFFFh
		/*.text:0043AAE0*/                 xor     ecx, edx
		/*.text:0043AAE2*/                 and     ecx, 8000h
		/*.text:0043AAE8*/                 mov     word ptr /*[ebp+var_2C]*/ [ebp-2ch], cx
		/*.text:0043AAEC*/                 mov     ax, word ptr /*[ebp+var_18]*/ [ebp-18h]
		/*.text:0043AAF0*/                 and     ax, 7FFFh
		/*.text:0043AAF4*/                 mov     word ptr /*[ebp+var_18]*/ [ebp-18h], ax
		/*.text:0043AAF8*/                 mov     cx, word ptr /*[ebp+var_20]*/ [ebp-20h]
		/*.text:0043AAFC*/                 and     cx, 7FFFh
		/*.text:0043AB01*/                 mov     word ptr /*[ebp+var_20]*/ [ebp-20h], cx
		/*.text:0043AB05*/                 mov     edx, /*[ebp+*/ var_18 /*]*/
		/*.text:0043AB08*/                 and     edx, 0FFFFh
		/*.text:0043AB0E*/                 mov     eax, /*[ebp+*/ var_20 /*]*/
		/*.text:0043AB11*/                 and     eax, 0FFFFh
		/*.text:0043AB16*/                 add     edx, eax
		/*.text:0043AB18*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], dx
		/*.text:0043AB1C*/                 mov     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0043AB1F*/                 and     ecx, 0FFFFh
		/*.text:0043AB25*/                 cmp     ecx, 7FFFh
		/*.text:0043AB2B*/                 jge     short loc_43AB4D
		/*.text:0043AB2D*/                 mov     edx, /*[ebp+*/ var_20 /*]*/
		/*.text:0043AB30*/                 and     edx, 0FFFFh
		/*.text:0043AB36*/                 cmp     edx, 7FFFh
		/*.text:0043AB3C*/                 jge     short loc_43AB4D
		/*.text:0043AB3E*/                 mov     eax, /*[ebp+*/ var_30 /*]*/
		/*.text:0043AB41*/                 and     eax, 0FFFFh
		/*.text:0043AB46*/                 cmp     eax, 0BFFDh
		/*.text:0043AB4B*/                 jle     short loc_43AB84
		/*.text:0043AB4D*/ 
		/*.text:0043AB4D*/ loc_43AB4D:                             ; CODE XREF: ___ld12mul+9B.j
		/*.text:0043AB4D*/                                         ; ___ld12mul+AC.j
		/*.text:0043AB4D*/                 mov     ecx, /*[ebp+*/ var_2C /*]*/
		/*.text:0043AB50*/                 and     ecx, 0FFFFh
		/*.text:0043AB56*/                 neg     ecx
		/*.text:0043AB58*/                 sbb     ecx, ecx
		/*.text:0043AB5A*/                 and     ecx, 80000000h
		/*.text:0043AB60*/                 add     ecx, 7FFF8000h
		/*.text:0043AB66*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AB69*/                 mov     [edx+8], ecx
		/*.text:0043AB6C*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AB6F*/                 mov     dword ptr [eax+4], 0
		/*.text:0043AB76*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AB79*/                 mov     dword ptr [ecx], 0
		/*.text:0043AB7F*/                 jmp     loc_43AEC9
		/*.text:0043AB84*/ ; ---------------------------------------------------------------------------
		/*.text:0043AB84*/ 
		/*.text:0043AB84*/ loc_43AB84:                             ; CODE XREF: ___ld12mul+BB.j
		/*.text:0043AB84*/                 mov     edx, /*[ebp+*/ var_30 /*]*/
		/*.text:0043AB87*/                 and     edx, 0FFFFh
		/*.text:0043AB8D*/                 cmp     edx, 3FBFh
		/*.text:0043AB93*/                 jg      short loc_43ABB7
		/*.text:0043AB95*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AB98*/                 mov     dword ptr [eax+8], 0
		/*.text:0043AB9F*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ABA2*/                 mov     dword ptr [ecx+4], 0
		/*.text:0043ABA9*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ABAC*/                 mov     dword ptr [edx], 0
		/*.text:0043ABB2*/                 jmp     loc_43AEC9
		/*.text:0043ABB7*/ ; ---------------------------------------------------------------------------
		/*.text:0043ABB7*/ 
		/*.text:0043ABB7*/ loc_43ABB7:                             ; CODE XREF: ___ld12mul+103.j
		/*.text:0043ABB7*/                 mov     eax, /*[ebp+*/ var_18 /*]*/
		/*.text:0043ABBA*/                 and     eax, 0FFFFh
		/*.text:0043ABBF*/                 test    eax, eax
		/*.text:0043ABC1*/                 jnz     short loc_43ABFD
		/*.text:0043ABC3*/                 mov     cx, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043ABC7*/                 add     cx, 1
		/*.text:0043ABCB*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], cx
		/*.text:0043ABCF*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ABD2*/                 mov     eax, [edx+8]
		/*.text:0043ABD5*/                 and     eax, 7FFFFFFFh
		/*.text:0043ABDA*/                 test    eax, eax
		/*.text:0043ABDC*/                 jnz     short loc_43ABFD
		/*.text:0043ABDE*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ABE1*/                 cmp     dword ptr [ecx+4], 0
		/*.text:0043ABE5*/                 jnz     short loc_43ABFD
		/*.text:0043ABE7*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ABEA*/                 cmp     dword ptr [edx], 0
		/*.text:0043ABED*/                 jnz     short loc_43ABFD
		/*.text:0043ABEF*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ABF2*/                 mov     word ptr [eax+0Ah], 0
		/*.text:0043ABF8*/                 jmp     loc_43AEC9
		/*.text:0043ABFD*/ ; ---------------------------------------------------------------------------
		/*.text:0043ABFD*/ 
		/*.text:0043ABFD*/ loc_43ABFD:                             ; CODE XREF: ___ld12mul+131.j
		/*.text:0043ABFD*/                                         ; ___ld12mul+14C.j ...
		/*.text:0043ABFD*/                 mov     ecx, /*[ebp+*/ var_20 /*]*/
		/*.text:0043AC00*/                 and     ecx, 0FFFFh
		/*.text:0043AC06*/                 test    ecx, ecx
		/*.text:0043AC08*/                 jnz     short loc_43AC59
		/*.text:0043AC0A*/                 mov     dx, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AC0E*/                 add     dx, 1
		/*.text:0043AC12*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], dx
		/*.text:0043AC16*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AC19*/                 mov     ecx, [eax+8]
		/*.text:0043AC1C*/                 and     ecx, 7FFFFFFFh
		/*.text:0043AC22*/                 test    ecx, ecx
		/*.text:0043AC24*/                 jnz     short loc_43AC59
		/*.text:0043AC26*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AC29*/                 cmp     dword ptr [edx+4], 0
		/*.text:0043AC2D*/                 jnz     short loc_43AC59
		/*.text:0043AC2F*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AC32*/                 cmp     dword ptr [eax], 0
		/*.text:0043AC35*/                 jnz     short loc_43AC59
		/*.text:0043AC37*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AC3A*/                 mov     dword ptr [ecx+8], 0
		/*.text:0043AC41*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AC44*/                 mov     dword ptr [edx+4], 0
		/*.text:0043AC4B*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AC4E*/                 mov     dword ptr [eax], 0
		/*.text:0043AC54*/                 jmp     loc_43AEC9
		/*.text:0043AC59*/ ; ---------------------------------------------------------------------------
		/*.text:0043AC59*/ 
		/*.text:0043AC59*/ loc_43AC59:                             ; CODE XREF: ___ld12mul+178.j
		/*.text:0043AC59*/                                         ; ___ld12mul+194.j ...
		/*.text:0043AC59*/                 mov     /*[ebp+*/ var_28 /*]*/, 0
		/*.text:0043AC60*/                 mov     /*[ebp+*/ var_1C /*]*/, 0
		/*.text:0043AC67*/                 jmp     short loc_43AC72
		/*.text:0043AC69*/ ; ---------------------------------------------------------------------------
		/*.text:0043AC69*/ 
		/*.text:0043AC69*/ loc_43AC69:                             ; CODE XREF: ___ld12mul+29C.j
		/*.text:0043AC69*/                 mov     ecx, /*[ebp+*/ var_1C /*]*/
		/*.text:0043AC6C*/                 add     ecx, 1
		/*.text:0043AC6F*/                 mov     /*[ebp+*/ var_1C /*]*/, ecx
		/*.text:0043AC72*/ 
		/*.text:0043AC72*/ loc_43AC72:                             ; CODE XREF: ___ld12mul+1D7.j
		/*.text:0043AC72*/                 cmp     /*[ebp+*/ var_1C /*]*/, 5
		/*.text:0043AC76*/                 jge     loc_43AD31
		/*.text:0043AC7C*/                 mov     edx, /*[ebp+*/ var_1C /*]*/
		/*.text:0043AC7F*/                 shl     edx, 1
		/*.text:0043AC81*/                 mov     /*[ebp+*/ var_24 /*]*/, edx
		/*.text:0043AC84*/                 mov     /*[ebp+*/ var_8 /*]*/, 8
		/*.text:0043AC8B*/                 mov     eax, 5
		/*.text:0043AC90*/                 sub     eax, /*[ebp+*/ var_1C /*]*/
		/*.text:0043AC93*/                 mov     /*[ebp+*/ var_34 /*]*/, eax
		/*.text:0043AC96*/                 jmp     short loc_43ACA1
		/*.text:0043AC98*/ ; ---------------------------------------------------------------------------
		/*.text:0043AC98*/ 
		/*.text:0043AC98*/ loc_43AC98:                             ; CODE XREF: ___ld12mul+28E.j
		/*.text:0043AC98*/                 mov     ecx, /*[ebp+*/ var_34 /*]*/
		/*.text:0043AC9B*/                 sub     ecx, 1
		/*.text:0043AC9E*/                 mov     /*[ebp+*/ var_34 /*]*/, ecx
		/*.text:0043ACA1*/ 
		/*.text:0043ACA1*/ loc_43ACA1:                             ; CODE XREF: ___ld12mul+206.j
		/*.text:0043ACA1*/                 cmp     /*[ebp+*/ var_34 /*]*/, 0
		/*.text:0043ACA5*/                 jle     short loc_43AD23
		/*.text:0043ACA7*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043ACAA*/                 add     edx, /*[ebp+*/ var_24 /*]*/
		/*.text:0043ACAD*/                 mov     /*[ebp+*/ var_38 /*]*/, edx
		/*.text:0043ACB0*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043ACB3*/                 add     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0043ACB6*/                 mov     /*[ebp+*/ var_3C /*]*/, eax
		/*.text:0043ACB9*/                 mov     ecx, /*[ebp+*/ var_28 /*]*/
		/*.text:0043ACBC*/                 lea     edx, /*[ebp+ecx+var_14]*/ [ ebp+ecx-14h]
		/*.text:0043ACC0*/                 mov     /*[ebp+*/ var_40 /*]*/, edx
		/*.text:0043ACC3*/                 mov     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:0043ACC6*/                 xor     ecx, ecx
		/*.text:0043ACC8*/                 mov     cx, [eax]
		/*.text:0043ACCB*/                 mov     edx, /*[ebp+*/ var_3C /*]*/
		/*.text:0043ACCE*/                 xor     eax, eax
		/*.text:0043ACD0*/                 mov     ax, [edx]
		/*.text:0043ACD3*/                 imul    ecx, eax
		/*.text:0043ACD6*/                 mov     /*[ebp+*/ var_44 /*]*/, ecx
		/*.text:0043ACD9*/                 mov     ecx, /*[ebp+*/ var_40 /*]*/
		/*.text:0043ACDC*/                 push    ecx
		/*.text:0043ACDD*/                 mov     edx, /*[ebp+*/ var_44 /*]*/
		/*.text:0043ACE0*/                 push    edx
		/*.text:0043ACE1*/                 mov     eax, /*[ebp+*/ var_40 /*]*/
		/*.text:0043ACE4*/                 mov     ecx, [eax]
		/*.text:0043ACE6*/                 push    ecx
		/*.text:0043ACE7*/                 call    ___addl
		/*.text:0043ACEC*/                 add     esp, 0Ch
		/*.text:0043ACEF*/                 mov     /*[ebp+*/ var_48 /*]*/, eax
		/*.text:0043ACF2*/                 cmp     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:0043ACF6*/                 jz      short loc_43AD0C
		/*.text:0043ACF8*/                 mov     edx, /*[ebp+*/ var_28 /*]*/
		/*.text:0043ACFB*/                 mov     ax, word ptr /*[ebp+edx+var_10]*/ [ebp+edx-10h]
		/*.text:0043AD00*/                 add     ax, 1
		/*.text:0043AD04*/                 mov     ecx, /*[ebp+*/ var_28 /*]*/
		/*.text:0043AD07*/                 mov     word ptr /*[ebp+ecx+var_10]*/ [ebp+ecx-10h], ax
		/*.text:0043AD0C*/ 
		/*.text:0043AD0C*/ loc_43AD0C:                             ; CODE XREF: ___ld12mul+266.j
		/*.text:0043AD0C*/                 mov     edx, /*[ebp+*/ var_24 /*]*/
		/*.text:0043AD0F*/                 add     edx, 2
		/*.text:0043AD12*/                 mov     /*[ebp+*/ var_24 /*]*/, edx
		/*.text:0043AD15*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0043AD18*/                 sub     eax, 2
		/*.text:0043AD1B*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:0043AD1E*/                 jmp     loc_43AC98
		/*.text:0043AD23*/ ; ---------------------------------------------------------------------------
		/*.text:0043AD23*/ 
		/*.text:0043AD23*/ loc_43AD23:                             ; CODE XREF: ___ld12mul+215.j
		/*.text:0043AD23*/                 mov     ecx, /*[ebp+*/ var_28 /*]*/
		/*.text:0043AD26*/                 add     ecx, 2
		/*.text:0043AD29*/                 mov     /*[ebp+*/ var_28 /*]*/, ecx
		/*.text:0043AD2C*/                 jmp     loc_43AC69
		/*.text:0043AD31*/ ; ---------------------------------------------------------------------------
		/*.text:0043AD31*/ 
		/*.text:0043AD31*/ loc_43AD31:                             ; CODE XREF: ___ld12mul+1E6.j
		/*.text:0043AD31*/                 mov     dx, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AD35*/                 sub     dx, 3FFEh
		/*.text:0043AD3A*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], dx
		/*.text:0043AD3E*/ 
		/*.text:0043AD3E*/ loc_43AD3E:                             ; CODE XREF: ___ld12mul+2DB.j
		/*.text:0043AD3E*/                 movsx   eax, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AD42*/                 test    eax, eax
		/*.text:0043AD44*/                 jle     short loc_43AD6D
		/*.text:0043AD46*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:0043AD49*/                 and     ecx, 80000000h
		/*.text:0043AD4F*/                 test    ecx, ecx
		/*.text:0043AD51*/                 jnz     short loc_43AD6D
		/*.text:0043AD53*/                 lea     edx, /*[ebp+*/ var_14 /*]*/
		/*.text:0043AD56*/                 push    edx
		/*.text:0043AD57*/                 call    ___shl_12
		/*.text:0043AD5C*/                 add     esp, 4
		/*.text:0043AD5F*/                 mov     ax, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AD63*/                 sub     ax, 1
		/*.text:0043AD67*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], ax
		/*.text:0043AD6B*/                 jmp     short loc_43AD3E
		/*.text:0043AD6D*/ ; ---------------------------------------------------------------------------
		/*.text:0043AD6D*/ 
		/*.text:0043AD6D*/ loc_43AD6D:                             ; CODE XREF: ___ld12mul+2B4.j
		/*.text:0043AD6D*/                                         ; ___ld12mul+2C1.j
		/*.text:0043AD6D*/                 movsx   ecx, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AD71*/                 test    ecx, ecx
		/*.text:0043AD73*/                 jg      short loc_43ADCD
		/*.text:0043AD75*/                 mov     dx, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AD79*/                 sub     dx, 1
		/*.text:0043AD7D*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], dx
		/*.text:0043AD81*/ 
		/*.text:0043AD81*/ loc_43AD81:                             ; CODE XREF: ___ld12mul+32A.j
		/*.text:0043AD81*/                 movsx   eax, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AD85*/                 test    eax, eax
		/*.text:0043AD87*/                 jge     short loc_43ADBC
		/*.text:0043AD89*/                 mov     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:0043AD8C*/                 and     ecx, 0FFFFh
		/*.text:0043AD92*/                 and     ecx, 1
		/*.text:0043AD95*/                 test    ecx, ecx
		/*.text:0043AD97*/                 jz      short loc_43ADA2
		/*.text:0043AD99*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043AD9C*/                 add     edx, 1
		/*.text:0043AD9F*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0043ADA2*/ 
		/*.text:0043ADA2*/ loc_43ADA2:                             ; CODE XREF: ___ld12mul+307.j
		/*.text:0043ADA2*/                 lea     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:0043ADA5*/                 push    eax
		/*.text:0043ADA6*/                 call    ___shr_12
		/*.text:0043ADAB*/                 add     esp, 4
		/*.text:0043ADAE*/                 mov     cx, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043ADB2*/                 add     cx, 1
		/*.text:0043ADB6*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], cx
		/*.text:0043ADBA*/                 jmp     short loc_43AD81
		/*.text:0043ADBC*/ ; ---------------------------------------------------------------------------
		/*.text:0043ADBC*/ 
		/*.text:0043ADBC*/ loc_43ADBC:                             ; CODE XREF: ___ld12mul+2F7.j
		/*.text:0043ADBC*/                 cmp     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:0043ADC0*/                 jz      short loc_43ADCD
		/*.text:0043ADC2*/                 mov     dx, word ptr /*[ebp+var_14]*/ [ebp-14h]
		/*.text:0043ADC6*/                 or      dl, 1
		/*.text:0043ADC9*/                 mov     word ptr /*[ebp+var_14]*/ [ebp-14h], dx
		/*.text:0043ADCD*/ 
		/*.text:0043ADCD*/ loc_43ADCD:                             ; CODE XREF: ___ld12mul+2E3.j
		/*.text:0043ADCD*/                                         ; ___ld12mul+330.j
		/*.text:0043ADCD*/                 mov     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:0043ADD0*/                 and     eax, 0FFFFh
		/*.text:0043ADD5*/                 cmp     eax, 8000h
		/*.text:0043ADDA*/                 jg      short loc_43ADED
		/*.text:0043ADDC*/                 mov     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:0043ADDF*/                 and     ecx, 1FFFFh
		/*.text:0043ADE5*/                 cmp     ecx, 18000h
		/*.text:0043ADEB*/                 jnz     short loc_43AE4E
		/*.text:0043ADED*/ 
		/*.text:0043ADED*/ loc_43ADED:                             ; CODE XREF: ___ld12mul+34A.j
		/*.text:0043ADED*/                 cmp     /*[ebp+var_14+2]*/ [ebp-14h+2], 0FFFFFFFFh
		/*.text:0043ADF1*/                 jnz     short loc_43AE45
		/*.text:0043ADF3*/                 mov     /*[ebp+var_14+2]*/ [ebp-14h+2], 0
		/*.text:0043ADFA*/                 cmp     /*[ebp+var_10+2]*/ [ebp-10h+2], 0FFFFFFFFh
		/*.text:0043ADFE*/                 jnz     short loc_43AE3A
		/*.text:0043AE00*/                 mov     /*[ebp+var_10+2]*/ [ebp-10h+2], 0
		/*.text:0043AE07*/                 mov     edx, /*[ebp+var_C+2]*/ [ebp-0ch+2]
		/*.text:0043AE0A*/                 and     edx, 0FFFFh
		/*.text:0043AE10*/                 cmp     edx, 0FFFFh
		/*.text:0043AE16*/                 jnz     short loc_43AE2C
		/*.text:0043AE18*/                 mov     word ptr /*[ebp+var_C+2]*/ [ebp-0ch+2], 8000h
		/*.text:0043AE1E*/                 mov     ax, word ptr /*[ebp+var_30]*/ [ebp-30h]
		/*.text:0043AE22*/                 add     ax, 1
		/*.text:0043AE26*/                 mov     word ptr /*[ebp+var_30]*/ [ebp-30h], ax
		/*.text:0043AE2A*/                 jmp     short loc_43AE38
		/*.text:0043AE2C*/ ; ---------------------------------------------------------------------------
		/*.text:0043AE2C*/ 
		/*.text:0043AE2C*/ loc_43AE2C:                             ; CODE XREF: ___ld12mul+386.j
		/*.text:0043AE2C*/                 mov     cx, word ptr /*[ebp+var_C+2]*/ [ebp-0ch+2]
		/*.text:0043AE30*/                 add     cx, 1
		/*.text:0043AE34*/                 mov     word ptr /*[ebp+var_C+2]*/ [ebp-0ch+2], cx
		/*.text:0043AE38*/ 
		/*.text:0043AE38*/ loc_43AE38:                             ; CODE XREF: ___ld12mul+39A.j
		/*.text:0043AE38*/                 jmp     short loc_43AE43
		/*.text:0043AE3A*/ ; ---------------------------------------------------------------------------
		/*.text:0043AE3A*/ 
		/*.text:0043AE3A*/ loc_43AE3A:                             ; CODE XREF: ___ld12mul+36E.j
		/*.text:0043AE3A*/                 mov     edx, /*[ebp+var_10+2]*/ [ebp-10h+2]
		/*.text:0043AE3D*/                 add     edx, 1
		/*.text:0043AE40*/                 mov     /*[ebp+var_10+2]*/ [ebp-10h+2], edx
		/*.text:0043AE43*/ 
		/*.text:0043AE43*/ loc_43AE43:                             ; CODE XREF: ___ld12mul+3A8.j
		/*.text:0043AE43*/                 jmp     short loc_43AE4E
		/*.text:0043AE45*/ ; ---------------------------------------------------------------------------
		/*.text:0043AE45*/ 
		/*.text:0043AE45*/ loc_43AE45:                             ; CODE XREF: ___ld12mul+361.j
		/*.text:0043AE45*/                 mov     eax, /*[ebp+var_14+2]*/ [ebp-14h+2]
		/*.text:0043AE48*/                 add     eax, 1
		/*.text:0043AE4B*/                 mov     /*[ebp+var_14+2]*/ [ebp-14h+2], eax
		/*.text:0043AE4E*/ 
		/*.text:0043AE4E*/ loc_43AE4E:                             ; CODE XREF: ___ld12mul+35B.j
		/*.text:0043AE4E*/                                         ; ___ld12mul+3B3.j
		/*.text:0043AE4E*/                 mov     ecx, /*[ebp+*/ var_30 /*]*/
		/*.text:0043AE51*/                 and     ecx, 0FFFFh
		/*.text:0043AE57*/                 cmp     ecx, 7FFFh
		/*.text:0043AE5D*/                 jl      short loc_43AE93
		/*.text:0043AE5F*/                 mov     edx, /*[ebp+*/ var_2C /*]*/
		/*.text:0043AE62*/                 and     edx, 0FFFFh
		/*.text:0043AE68*/                 neg     edx
		/*.text:0043AE6A*/                 sbb     edx, edx
		/*.text:0043AE6C*/                 and     edx, 80000000h
		/*.text:0043AE72*/                 add     edx, 7FFF8000h
		/*.text:0043AE78*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AE7B*/                 mov     [eax+8], edx
		/*.text:0043AE7E*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AE81*/                 mov     dword ptr [ecx+4], 0
		/*.text:0043AE88*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AE8B*/                 mov     dword ptr [edx], 0
		/*.text:0043AE91*/                 jmp     short loc_43AEC9
		/*.text:0043AE93*/ ; ---------------------------------------------------------------------------
		/*.text:0043AE93*/ 
		/*.text:0043AE93*/ loc_43AE93:                             ; CODE XREF: ___ld12mul+3CD.j
		/*.text:0043AE93*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AE96*/                 mov     cx, word ptr /*[ebp+var_14+2]*/ [ebp-14h+2]
		/*.text:0043AE9A*/                 mov     [eax], cx
		/*.text:0043AE9D*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AEA0*/                 mov     eax, /*[ebp+*/ var_10 /*]*/
		/*.text:0043AEA3*/                 mov     [edx+2], eax
		/*.text:0043AEA6*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AEA9*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:0043AEAC*/                 mov     [ecx+6], edx
		/*.text:0043AEAF*/                 mov     eax, /*[ebp+*/ var_30 /*]*/
		/*.text:0043AEB2*/                 and     eax, 0FFFFh
		/*.text:0043AEB7*/                 mov     ecx, /*[ebp+*/ var_2C /*]*/
		/*.text:0043AEBA*/                 and     ecx, 0FFFFh
		/*.text:0043AEC0*/                 or      eax, ecx
		/*.text:0043AEC2*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AEC5*/                 mov     [edx+0Ah], ax
		/*.text:0043AEC9*/ 
		/*.text:0043AEC9*/ loc_43AEC9:                             ; CODE XREF: ___ld12mul+EF.j
		/*.text:0043AEC9*/                                         ; ___ld12mul+122.j ...
		/*.text:0043AEC9*/                 mov     esp, ebp
		/*.text:0043AECB*/                 pop     ebp
		/*.text:0043AECC*/                 retn
		/*.text:0043AECC*/ // ___ld12mul      endp
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
		/*.text:0043AED0*/ // ___multtenpow12 proc near               ; CODE XREF: _$I10_OUTPUT+288.p
		/*.text:0043AED0*/                                         ; ___strgtold12+914.p
		/*.text:0043AED0*/ 
		/*.text:0043AED0*/ #define var_18           dword ptr [ebp-18h]
		/*.text:0043AED0*/ #define var_14           dword ptr [ebp-14h]
		/*.text:0043AED0*/ #define var_10           dword ptr [ebp-10h]
		/*.text:0043AED0*/ #define var_C            dword ptr [ebp-0Ch]
		/*.text:0043AED0*/ #define var_8            dword ptr [ebp-8]
		/*.text:0043AED0*/ #define var_4            dword ptr [ebp-4]
		/*.text:0043AED0*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:0043AED0*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:0043AED0*/ #define arg_8            dword ptr  [ebp+10h]
		/*.text:0043AED0*/ 
		/*.text:0043AED0*/                 push    ebp
		/*.text:0043AED1*/                 mov     ebp, esp
		/*.text:0043AED3*/                 sub     esp, 18h
		/*.text:0043AED6*/                 mov     eax, offset __pow10pos
		/*.text:0043AEDB*/                 sub     eax, 60h
		/*.text:0043AEDE*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0043AEE1*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0043AEE5*/                 jnz     short loc_43AEEC
		/*.text:0043AEE7*/                 jmp     loc_43AF8F
		/*.text:0043AEEC*/ ; ---------------------------------------------------------------------------
		/*.text:0043AEEC*/ 
		/*.text:0043AEEC*/ loc_43AEEC:                             ; CODE XREF: ___multtenpow12+15.j
		/*.text:0043AEEC*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0043AEF0*/                 jge     short loc_43AF05
		/*.text:0043AEF2*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AEF5*/                 neg     ecx
		/*.text:0043AEF7*/                 mov     /*[ebp+*/ arg_4 /*]*/, ecx
		/*.text:0043AEFA*/                 mov     edx, offset __pow10neg
		/*.text:0043AEFF*/                 sub     edx, 60h
		/*.text:0043AF02*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0043AF05*/ 
		/*.text:0043AF05*/ loc_43AF05:                             ; CODE XREF: ___multtenpow12+20.j
		/*.text:0043AF05*/                 cmp     /*[ebp+*/ arg_8 /*]*/, 0
		/*.text:0043AF09*/                 jnz     short loc_43AF13
		/*.text:0043AF0B*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AF0E*/                 mov     word ptr [eax], 0
		/*.text:0043AF13*/ 
		/*.text:0043AF13*/ loc_43AF13:                             ; CODE XREF: ___multtenpow12+39.j
		/*.text:0043AF13*/                                         ; ___multtenpow12+6A.j ...
		/*.text:0043AF13*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:0043AF17*/                 jz      short loc_43AF8F
		/*.text:0043AF19*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043AF1C*/                 add     ecx, 54h
		/*.text:0043AF1F*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0043AF22*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AF25*/                 and     edx, 7
		/*.text:0043AF28*/                 mov     /*[ebp+*/ var_18 /*]*/, edx
		/*.text:0043AF2B*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:0043AF2E*/                 sar     eax, 3
		/*.text:0043AF31*/                 mov     /*[ebp+*/ arg_4 /*]*/, eax
		/*.text:0043AF34*/                 cmp     /*[ebp+*/ var_18 /*]*/, 0
		/*.text:0043AF38*/                 jnz     short loc_43AF3C
		/*.text:0043AF3A*/                 jmp     short loc_43AF13
		/*.text:0043AF3C*/ ; ---------------------------------------------------------------------------
		/*.text:0043AF3C*/ 
		/*.text:0043AF3C*/ loc_43AF3C:                             ; CODE XREF: ___multtenpow12+68.j
		/*.text:0043AF3C*/                 mov     ecx, /*[ebp+*/ var_18 /*]*/
		/*.text:0043AF3F*/                 imul    ecx, 0Ch
		/*.text:0043AF42*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043AF45*/                 add     edx, ecx
		/*.text:0043AF47*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:0043AF4A*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0043AF4D*/                 xor     ecx, ecx
		/*.text:0043AF4F*/                 mov     cx, [eax]
		/*.text:0043AF52*/                 cmp     ecx, 8000h
		/*.text:0043AF58*/                 jl      short loc_43AF7D
		/*.text:0043AF5A*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0043AF5D*/                 mov     eax, [edx]
		/*.text:0043AF5F*/                 mov     /*[ebp+*/ var_14 /*]*/, eax
		/*.text:0043AF62*/                 mov     ecx, [edx+4]
		/*.text:0043AF65*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:0043AF68*/                 mov     edx, [edx+8]
		/*.text:0043AF6B*/                 mov     /*[ebp+*/ var_C /*]*/, edx
		/*.text:0043AF6E*/                 mov     eax, /*[ebp+var_14+2]*/ [ebp-14h+2]
		/*.text:0043AF71*/                 sub     eax, 1
		/*.text:0043AF74*/                 mov     /*[ebp+var_14+2]*/ [ebp-14h+2], eax
		/*.text:0043AF77*/                 lea     ecx, /*[ebp+*/ var_14 /*]*/
		/*.text:0043AF7A*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:0043AF7D*/ 
		/*.text:0043AF7D*/ loc_43AF7D:                             ; CODE XREF: ___multtenpow12+88.j
		/*.text:0043AF7D*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0043AF80*/                 push    edx
		/*.text:0043AF81*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043AF84*/                 push    eax
		/*.text:0043AF85*/                 call    ___ld12mul
		/*.text:0043AF8A*/                 add     esp, 8
		/*.text:0043AF8D*/                 jmp     short loc_43AF13
		/*.text:0043AF8F*/ ; ---------------------------------------------------------------------------
		/*.text:0043AF8F*/ 
		/*.text:0043AF8F*/ loc_43AF8F:                             ; CODE XREF: ___multtenpow12+17.j
		/*.text:0043AF8F*/                                         ; ___multtenpow12+47.j
		/*.text:0043AF8F*/                 mov     esp, ebp
		/*.text:0043AF91*/                 pop     ebp
		/*.text:0043AF92*/                 retn
		/*.text:0043AF92*/ // ___multtenpow12 endp
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
		/*.text:004392A0*/ // ___add_12       proc near               ; CODE XREF: _$I10_OUTPUT+3E1.p
		/*.text:004392A0*/                                         ; ___mtold12+77.p ...
		/*.text:004392A0*/ 
		/*.text:004392A0*/ #define var_C            dword ptr [ebp-0Ch]
		/*.text:004392A0*/ #define var_8            dword ptr [ebp-8]
		/*.text:004392A0*/ #define var_4            dword ptr [ebp-4]
		/*.text:004392A0*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:004392A0*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:004392A0*/ 
		/*.text:004392A0*/                 push    ebp
		/*.text:004392A1*/                 mov     ebp, esp
		/*.text:004392A3*/                 sub     esp, 0Ch
		/*.text:004392A6*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392A9*/                 push    eax
		/*.text:004392AA*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004392AD*/                 mov     edx, [ecx]
		/*.text:004392AF*/                 push    edx
		/*.text:004392B0*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392B3*/                 mov     ecx, [eax]
		/*.text:004392B5*/                 push    ecx
		/*.text:004392B6*/                 call    ___addl
		/*.text:004392BB*/                 add     esp, 0Ch
		/*.text:004392BE*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004392C1*/                 cmp     /*[ebp+*/ var_4 /*]*/, 0
		/*.text:004392C5*/                 jz      short loc_4392F7
		/*.text:004392C7*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392CA*/                 add     edx, 4
		/*.text:004392CD*/                 push    edx
		/*.text:004392CE*/                 push    1
		/*.text:004392D0*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392D3*/                 mov     ecx, [eax+4]
		/*.text:004392D6*/                 push    ecx
		/*.text:004392D7*/                 call    ___addl
		/*.text:004392DC*/                 add     esp, 0Ch
		/*.text:004392DF*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:004392E2*/                 cmp     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:004392E6*/                 jz      short loc_4392F7
		/*.text:004392E8*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392EB*/                 mov     eax, [edx+8]
		/*.text:004392EE*/                 add     eax, 1
		/*.text:004392F1*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392F4*/                 mov     [ecx+8], eax
		/*.text:004392F7*/ 
		/*.text:004392F7*/ loc_4392F7:                             ; CODE XREF: ___add_12+25.j
		/*.text:004392F7*/                                         ; ___add_12+46.j
		/*.text:004392F7*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004392FA*/                 add     edx, 4
		/*.text:004392FD*/                 push    edx
		/*.text:004392FE*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:00439301*/                 mov     ecx, [eax+4]
		/*.text:00439304*/                 push    ecx
		/*.text:00439305*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439308*/                 mov     eax, [edx+4]
		/*.text:0043930B*/                 push    eax
		/*.text:0043930C*/                 call    ___addl
		/*.text:00439311*/                 add     esp, 0Ch
		/*.text:00439314*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:00439317*/                 cmp     /*[ebp+*/ var_C /*]*/, 0
		/*.text:0043931B*/                 jz      short loc_43932C
		/*.text:0043931D*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439320*/                 mov     edx, [ecx+8]
		/*.text:00439323*/                 add     edx, 1
		/*.text:00439326*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00439329*/                 mov     [eax+8], edx
		/*.text:0043932C*/ 
		/*.text:0043932C*/ loc_43932C:                             ; CODE XREF: ___add_12+7B.j
		/*.text:0043932C*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043932F*/                 add     ecx, 8
		/*.text:00439332*/                 push    ecx
		/*.text:00439333*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00439336*/                 mov     eax, [edx+8]
		/*.text:00439339*/                 push    eax
		/*.text:0043933A*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0043933D*/                 mov     edx, [ecx+8]
		/*.text:00439340*/                 push    edx
		/*.text:00439341*/                 call    ___addl
		/*.text:00439346*/                 add     esp, 0Ch
		/*.text:00439349*/                 mov     esp, ebp
		/*.text:0043934B*/                 pop     ebp
		/*.text:0043934C*/                 retn
		/*.text:0043934C*/ // ___add_12       endp
	}
}

#undef var_C
#undef var_8
#undef var_4
#undef arg_0
#undef arg_4

static VOID __declspec( naked ) __I10_OUTPUT( VOID )
{
	__asm
	{
		/*.text:004383E0*/ // __I10_OUTPUT    proc near               ; CODE XREF: __fltout2+36.p
		/*.text:004383E0*/ 
		/*.text:004383E0*/ #define var_74           dword ptr [ebp-74h]
		/*.text:004383E0*/ #define var_70           dword ptr [ebp-70h]
		/*.text:004383E0*/ #define var_6C           dword ptr [ebp-6Ch]
		/*.text:004383E0*/ #define var_68           dword ptr [ebp-68h]
		/*.text:004383E0*/ #define var_64           dword ptr [ebp-64h]
		/*.text:004383E0*/ #define var_60           dword ptr [ebp-60h]
		/*.text:004383E0*/ #define var_5C           word ptr [ebp-5Ch]
		/*.text:004383E0*/ #define var_58           dword ptr [ebp-58h]
		/*.text:004383E0*/ #define var_54           byte ptr [ebp-54h]
		/*.text:004383E0*/ #define var_53           byte ptr [ebp-53h]
		/*.text:004383E0*/ #define var_52           byte ptr [ebp-52h]
		/*.text:004383E0*/ #define var_51           byte ptr [ebp-51h]
		/*.text:004383E0*/ #define var_50           byte ptr [ebp-50h]
		/*.text:004383E0*/ #define var_4F           byte ptr [ebp-4Fh]
		/*.text:004383E0*/ #define var_4E           byte ptr [ebp-4Eh]
		/*.text:004383E0*/ #define var_4D           byte ptr [ebp-4Dh]
		/*.text:004383E0*/ #define var_4C           byte ptr [ebp-4Ch]
		/*.text:004383E0*/ #define var_4B           byte ptr [ebp-4Bh]
		/*.text:004383E0*/ #define var_4A           byte ptr [ebp-4Ah]
		/*.text:004383E0*/ #define var_49           byte ptr [ebp-49h]
		/*.text:004383E0*/ #define var_48           dword ptr [ebp-48h]
		/*.text:004383E0*/ #define var_44           dword ptr [ebp-44h]
		/*.text:004383E0*/ #define var_40           dword ptr [ebp-40h]
		/*.text:004383E0*/ #define var_3C           dword ptr [ebp-3Ch]
		/*.text:004383E0*/ #define var_38           dword ptr [ebp-38h]
		/*.text:004383E0*/ #define var_32           dword ptr [ebp-32h]
		/*.text:004383E0*/ #define var_2E           dword ptr [ebp-2Eh]
		/*.text:004383E0*/ #define var_28           byte ptr [ebp-28h]
		/*.text:004383E0*/ #define var_24           dword ptr [ebp-24h]
		/*.text:004383E0*/ #define var_20           dword ptr [ebp-20h]
		/*.text:004383E0*/ #define var_1C           dword ptr [ebp-1Ch]
		/*.text:004383E0*/ #define var_18           dword ptr [ebp-18h]
		/*.text:004383E0*/ #define var_14           dword ptr [ebp-14h]
		/*.text:004383E0*/ #define var_10           dword ptr [ebp-10h]
		/*.text:004383E0*/ #define var_C            word ptr [ebp-0Ch]
		/*.text:004383E0*/ #define var_8            dword ptr [ebp-8]
		/*.text:004383E0*/ #define var_4            dword ptr [ebp-4]
		/*.text:004383E0*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:004383E0*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:004383E0*/ #define arg_8            word ptr  [ebp+10h]
		/*.text:004383E0*/ #define arg_C            dword ptr  [ebp+14h]
		/*.text:004383E0*/ #define arg_10           dword ptr  [ebp+18h]
		/*.text:004383E0*/ #define arg_14           dword ptr  [ebp+1Ch]
		/*.text:004383E0*/ 
		/*.text:004383E0*/                 push    ebp
		/*.text:004383E1*/                 mov     ebp, esp
		/*.text:004383E3*/                 sub     esp, 74h
		/*.text:004383E6*/                 mov     /*word ptr [ebp+var_64]*/ word ptr [ebp-64h], 4D10h
		/*.text:004383EC*/                 mov     /*word ptr [ebp+var_3C]*/ word ptr [ebp-3Ch], 4Dh
		/*.text:004383F2*/                 mov     /*word ptr [ebp+var_60]*/ word ptr [ebp-60h], 9Ah
		/*.text:004383F8*/                 mov     /*[ebp+*/ var_20 /*]*/, 134312F4h
		/*.text:004383FF*/                 mov     /*[ebp+*/ var_54 /*]*/, 0CCh
		/*.text:00438403*/                 mov     /*[ebp+*/ var_53 /*]*/, 0CCh
		/*.text:00438407*/                 mov     /*[ebp+*/ var_52 /*]*/, 0CCh
		/*.text:0043840B*/                 mov     /*[ebp+*/ var_51 /*]*/, 0CCh
		/*.text:0043840F*/                 mov     /*[ebp+*/ var_50 /*]*/, 0CCh
		/*.text:00438413*/                 mov     /*[ebp+*/ var_4F /*]*/, 0CCh
		/*.text:00438417*/                 mov     /*[ebp+*/ var_4E /*]*/, 0CCh
		/*.text:0043841B*/                 mov     /*[ebp+*/ var_4D /*]*/, 0CCh
		/*.text:0043841F*/                 mov     /*[ebp+*/ var_4C /*]*/, 0CCh
		/*.text:00438423*/                 mov     /*[ebp+*/ var_4B /*]*/, 0CCh
		/*.text:00438427*/                 mov     /*[ebp+*/ var_4A /*]*/, 0FBh
		/*.text:0043842B*/                 mov     /*[ebp+*/ var_49 /*]*/, 3Fh
		/*.text:0043842F*/                 mov     /*[ebp+*/ var_58 /*]*/, 1
		/*.text:00438436*/                 mov     ax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0043843A*/                 mov     word ptr /*[ebp+var_70] */ [ebp-70h], ax
		/*.text:0043843E*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00438441*/                 mov     /*[ebp+*/ var_24 /*]*/, ecx
		/*.text:00438444*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00438447*/                 mov     /*[ebp+*/ var_48 /*]*/, edx
		/*.text:0043844A*/                 mov     eax, /*[ebp+*/ var_70 /*]*/
		/*.text:0043844D*/                 and     eax, 0FFFFh
		/*.text:00438452*/                 and     eax, 8000h
		/*.text:00438457*/                 mov     word ptr /*[ebp+var_68]*/ [ebp-68h], ax
		/*.text:0043845B*/                 mov     cx, word ptr /*[ebp+var_70]*/ [ebp-70h]
		/*.text:0043845F*/                 and     cx, 7FFFh
		/*.text:00438464*/                 mov     word ptr /*[ebp+var_70]*/ [ebp-0x70], cx
		/*.text:00438468*/                 mov     edx, /*[ebp+*/ var_68 /*]*/
		/*.text:0043846B*/                 and     edx, 0FFFFh
		/*.text:00438471*/                 test    edx, edx
		/*.text:00438473*/                 jz      short loc_43847E
		/*.text:00438475*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438478*/                 mov     byte ptr [eax+2], 2Dh
		/*.text:0043847C*/                 jmp     short loc_438485
		/*.text:0043847E*/ ; ---------------------------------------------------------------------------
		/*.text:0043847E*/ 
		/*.text:0043847E*/ loc_43847E:                             ; CODE XREF: __I10_OUTPUT+93.j
		/*.text:0043847E*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438481*/                 mov     byte ptr [ecx+2], 20h
		/*.text:00438485*/ 
		/*.text:00438485*/ loc_438485:                             ; CODE XREF: __I10_OUTPUT+9C.j
		/*.text:00438485*/                 mov     edx, /*[ebp+*/ var_70 /*]*/
		/*.text:00438488*/                 and     edx, 0FFFFh
		/*.text:0043848E*/                 test    edx, edx
		/*.text:00438490*/                 jnz     short loc_4384CC
		/*.text:00438492*/                 cmp     /*[ebp+*/ var_24 /*]*/, 0
		/*.text:00438496*/                 jnz     short loc_4384CC
		/*.text:00438498*/                 cmp     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:0043849C*/                 jnz     short loc_4384CC
		/*.text:0043849E*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004384A1*/                 mov     word ptr [eax], 0
		/*.text:004384A6*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004384A9*/                 mov     byte ptr [ecx+2], 20h
		/*.text:004384AD*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004384B0*/                 mov     byte ptr [edx+3], 1
		/*.text:004384B4*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004384B7*/                 mov     byte ptr [eax+4], 30h
		/*.text:004384BB*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004384BE*/                 mov     byte ptr [ecx+5], 0
		/*.text:004384C2*/                 mov     eax, 1
		/*.text:004384C7*/                 jmp     loc_4388F2
		/*.text:004384CC*/ ; ---------------------------------------------------------------------------
		/*.text:004384CC*/ 
		/*.text:004384CC*/ loc_4384CC:                             ; CODE XREF: __I10_OUTPUT+B0.j
		/*.text:004384CC*/                                         ; __I10_OUTPUT+B6.j ...
		/*.text:004384CC*/                 mov     edx, /*[ebp+*/ var_70 /*]*/
		/*.text:004384CF*/                 and     edx, 0FFFFh
		/*.text:004384D5*/                 cmp     edx, 7FFFh
		/*.text:004384DB*/                 jnz     loc_4385C6
		/*.text:004384E1*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004384E4*/                 mov     word ptr [eax], 1
		/*.text:004384E9*/                 cmp     /*[ebp+*/ var_24 /*]*/, 80000000h
		/*.text:004384F0*/                 jnz     short loc_4384F8
		/*.text:004384F2*/                 cmp     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:004384F6*/                 jz      short loc_43852C
		/*.text:004384F8*/ 
		/*.text:004384F8*/ loc_4384F8:                             ; CODE XREF: __I10_OUTPUT+110.j
		/*.text:004384F8*/                 mov     ecx, /*[ebp+*/ var_24 /*]*/
		/*.text:004384FB*/                 and     ecx, 40000000h
		/*.text:00438501*/                 test    ecx, ecx
		/*.text:00438503*/                 jnz     short loc_43852C
		/*.text:00438505*/                 push    offset a1Snan   ; "1#SNAN"
		/*.text:0043850A*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:0043850D*/                 add     edx, 4
		/*.text:00438510*/                 push    edx
		/*.text:00438511*/                 call    __strcpy
		/*.text:00438516*/                 add     esp, 8
		/*.text:00438519*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:0043851C*/                 mov     byte ptr [eax+3], 6
		/*.text:00438520*/                 mov     /*[ebp+*/ var_58 /*]*/, 0
		/*.text:00438527*/                 jmp     loc_4385C1
		/*.text:0043852C*/ ; ---------------------------------------------------------------------------
		/*.text:0043852C*/ 
		/*.text:0043852C*/ loc_43852C:                             ; CODE XREF: __I10_OUTPUT+116.j
		/*.text:0043852C*/                                         ; __I10_OUTPUT+123.j
		/*.text:0043852C*/                 mov     ecx, /*[ebp+*/ var_68 /*]*/
		/*.text:0043852F*/                 and     ecx, 0FFFFh
		/*.text:00438535*/                 test    ecx, ecx
		/*.text:00438537*/                 jz      short loc_43856C
		/*.text:00438539*/                 cmp     /*[ebp+*/ var_24 /*]*/, 0C0000000h
		/*.text:00438540*/                 jnz     short loc_43856C
		/*.text:00438542*/                 cmp     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:00438546*/                 jnz     short loc_43856C
		/*.text:00438548*/                 push    offset a1Ind    ; "1#IND"
		/*.text:0043854D*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438550*/                 add     edx, 4
		/*.text:00438553*/                 push    edx
		/*.text:00438554*/                 call    __strcpy
		/*.text:00438559*/                 add     esp, 8
		/*.text:0043855C*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:0043855F*/                 mov     byte ptr [eax+3], 5
		/*.text:00438563*/                 mov     /*[ebp+*/ var_58 /*]*/, 0
		/*.text:0043856A*/                 jmp     short loc_4385C1
		/*.text:0043856C*/ ; ---------------------------------------------------------------------------
		/*.text:0043856C*/ 
		/*.text:0043856C*/ loc_43856C:                             ; CODE XREF: __I10_OUTPUT+157.j
		/*.text:0043856C*/                                         ; __I10_OUTPUT+160.j ...
		/*.text:0043856C*/                 cmp     /*[ebp+*/ var_24 /*]*/, 80000000h
		/*.text:00438573*/                 jnz     short loc_43859F
		/*.text:00438575*/                 cmp     /*[ebp+*/ var_48 /*]*/, 0
		/*.text:00438579*/                 jnz     short loc_43859F
		/*.text:0043857B*/                 push    offset a1Inf    ; "1#INF"
		/*.text:00438580*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438583*/                 add     ecx, 4
		/*.text:00438586*/                 push    ecx
		/*.text:00438587*/                 call    __strcpy
		/*.text:0043858C*/                 add     esp, 8
		/*.text:0043858F*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438592*/                 mov     byte ptr [edx+3], 5
		/*.text:00438596*/                 mov     /*[ebp+*/ var_58 /*]*/, 0
		/*.text:0043859D*/                 jmp     short loc_4385C1
		/*.text:0043859F*/ ; ---------------------------------------------------------------------------
		/*.text:0043859F*/ 
		/*.text:0043859F*/ loc_43859F:                             ; CODE XREF: __I10_OUTPUT+193.j
		/*.text:0043859F*/                                         ; __I10_OUTPUT+199.j
		/*.text:0043859F*/                 push    offset a1Qnan   ; "1#QNAN"
		/*.text:004385A4*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004385A7*/                 add     eax, 4
		/*.text:004385AA*/                 push    eax
		/*.text:004385AB*/                 call    __strcpy
		/*.text:004385B0*/                 add     esp, 8
		/*.text:004385B3*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004385B6*/                 mov     byte ptr [ecx+3], 6
		/*.text:004385BA*/                 mov     /*[ebp+*/ var_58 /*]*/, 0
		/*.text:004385C1*/ 
		/*.text:004385C1*/ loc_4385C1:                             ; CODE XREF: __I10_OUTPUT+147.j
		/*.text:004385C1*/                                         ; __I10_OUTPUT+18A.j ...
		/*.text:004385C1*/                 jmp     loc_4388EF
		/*.text:004385C6*/ ; ---------------------------------------------------------------------------
		/*.text:004385C6*/ 
		/*.text:004385C6*/ loc_4385C6:                             ; CODE XREF: __I10_OUTPUT+FB.j
		/*.text:004385C6*/                 mov     edx, /*[ebp+*/ var_70 /*]*/
		/*.text:004385C9*/                 and     edx, 0FFFFh
		/*.text:004385CF*/                 sar     edx, 8
		/*.text:004385D2*/                 mov     word ptr /*[ebp+var_6C]*/ [ebp-6ch], dx
		/*.text:004385D6*/                 mov     eax, /*[ebp+*/ var_70 /*]*/
		/*.text:004385D9*/                 and     eax, 0FFFFh
		/*.text:004385DE*/                 and     eax, 0FFh
		/*.text:004385E3*/                 mov     /*[ebp+*/ var_C /*]*/, ax
		/*.text:004385E7*/                 mov     ecx, /*[ebp+*/ var_24 /*]*/
		/*.text:004385EA*/                 shr     ecx, 18h
		/*.text:004385ED*/                 mov     word ptr /*[ebp+var_40]*/ [ebp-40h], cx
		/*.text:004385F1*/                 mov     edx, /*[ebp+*/ var_64 /*]*/
		/*.text:004385F4*/                 and     edx, 0FFFFh
		/*.text:004385FA*/                 mov     eax, /*[ebp+*/ var_70 /*]*/
		/*.text:004385FD*/                 and     eax, 0FFFFh
		/*.text:00438602*/                 imul    edx, eax
		/*.text:00438605*/                 mov     ecx, /*[ebp+*/ var_3C /*]*/
		/*.text:00438608*/                 and     ecx, 0FFFFh
		/*.text:0043860E*/                 mov     eax, /*[ebp+*/ var_6C /*]*/
		/*.text:00438611*/                 and     eax, 0FFFFh
		/*.text:00438616*/                 imul    ecx, eax
		/*.text:00438619*/                 add     edx, ecx
		/*.text:0043861B*/                 mov     ecx, /*[ebp+*/ var_60 /*]*/
		/*.text:0043861E*/                 and     ecx, 0FFFFh
		/*.text:00438624*/                 mov     eax, /*[ebp+*/ var_40 /*]*/
		/*.text:00438627*/                 and     eax, 0FFFFh
		/*.text:0043862C*/                 imul    ecx, eax
		/*.text:0043862F*/                 add     edx, ecx
		/*.text:00438631*/                 sub     edx, /*[ebp+*/ var_20 /*]*/
		/*.text:00438634*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00438637*/                 mov     ecx, /*[ebp+*/ var_8 /*]*/
		/*.text:0043863A*/                 sar     ecx, 10h
		/*.text:0043863D*/                 mov     /*[ebp+*/ var_5C /*]*/, cx
		/*.text:00438641*/                 mov     dx, word ptr /*[ebp+var_70]*/ [ebp-70h]
		/*.text:00438645*/                 mov     word ptr /*[ebp+var_2E]*/ [ebp-2eh], dx
		/*.text:00438649*/                 mov     eax, /*[ebp+*/ var_24 /*]*/
		/*.text:0043864C*/                 mov     /*[ebp+*/ var_32 /*]*/, eax
		/*.text:0043864F*/                 mov     ecx, /*[ebp+*/ var_48 /*]*/
		/*.text:00438652*/                 mov     /*[ebp+var_38+2]*/ [ebp-38h+2], ecx
		/*.text:00438655*/                 mov     word ptr /*[ebp+var_38]*/ [ebp-38h], 0
		/*.text:0043865B*/                 push    1
		/*.text:0043865D*/                 movsx   edx, /*[ebp+*/ var_5C /*]*/
		/*.text:00438661*/                 neg     edx
		/*.text:00438663*/                 push    edx
		/*.text:00438664*/                 lea     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:00438667*/                 push    eax
		/*.text:00438668*/                 call    ___multtenpow12
		/*.text:0043866D*/                 add     esp, 0Ch
		/*.text:00438670*/                 mov     ecx, /*[ebp+*/ var_2E /*]*/
		/*.text:00438673*/                 and     ecx, 0FFFFh
		/*.text:00438679*/                 cmp     ecx, 3FFFh
		/*.text:0043867F*/                 jl      short loc_43869D
		/*.text:00438681*/                 mov     dx, /*[ebp+*/ var_5C /*]*/
		/*.text:00438685*/                 add     dx, 1
		/*.text:00438689*/                 mov     /*[ebp+*/ var_5C /*]*/, dx
		/*.text:0043868D*/                 lea     eax, /*[ebp+*/ var_54 /*]*/
		/*.text:00438690*/                 push    eax
		/*.text:00438691*/                 lea     ecx, /*[ebp+*/ var_38 /*]*/
		/*.text:00438694*/                 push    ecx
		/*.text:00438695*/                 call    ___ld12mul
		/*.text:0043869A*/                 add     esp, 8
		/*.text:0043869D*/ 
		/*.text:0043869D*/ loc_43869D:                             ; CODE XREF: __I10_OUTPUT+29F.j
		/*.text:0043869D*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004386A0*/                 mov     ax, /*[ebp+*/ var_5C /*]*/
		/*.text:004386A4*/                 mov     [edx], ax
		/*.text:004386A7*/                 mov     ecx, /*[ebp+*/ arg_10 /*]*/
		/*.text:004386AA*/                 and     ecx, 1
		/*.text:004386AD*/                 test    ecx, ecx
		/*.text:004386AF*/                 jz      short loc_4386F1
		/*.text:004386B1*/                 movsx   edx, /*[ebp+*/ var_5C /*]*/
		/*.text:004386B5*/                 mov     eax, /*[ebp+*/ arg_C /*]*/
		/*.text:004386B8*/                 add     eax, edx
		/*.text:004386BA*/                 mov     /*[ebp+*/ arg_C /*]*/, eax
		/*.text:004386BD*/                 cmp     /*[ebp+*/ arg_C /*]*/, 0
		/*.text:004386C1*/                 jg      short loc_4386F1
		/*.text:004386C3*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004386C6*/                 mov     word ptr [ecx], 0
		/*.text:004386CB*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004386CE*/                 mov     byte ptr [edx+2], 20h
		/*.text:004386D2*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004386D5*/                 mov     byte ptr [eax+3], 1
		/*.text:004386D9*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004386DC*/                 mov     byte ptr [ecx+4], 30h
		/*.text:004386E0*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004386E3*/                 mov     byte ptr [edx+5], 0
		/*.text:004386E7*/                 mov     eax, 1
		/*.text:004386EC*/                 jmp     loc_4388F2
		/*.text:004386F1*/ ; ---------------------------------------------------------------------------
		/*.text:004386F1*/ 
		/*.text:004386F1*/ loc_4386F1:                             ; CODE XREF: __I10_OUTPUT+2CF.j
		/*.text:004386F1*/                                         ; __I10_OUTPUT+2E1.j
		/*.text:004386F1*/                 cmp     /*[ebp+*/ arg_C /*]*/, 15h
		/*.text:004386F5*/                 jle     short loc_4386FE
		/*.text:004386F7*/                 mov     /*[ebp+*/ arg_C /*]*/, 15h
		/*.text:004386FE*/ 
		/*.text:004386FE*/ loc_4386FE:                             ; CODE XREF: __I10_OUTPUT+315.j
		/*.text:004386FE*/                 mov     eax, /*[ebp+*/ var_2E /*]*/
		/*.text:00438701*/                 and     eax, 0FFFFh
		/*.text:00438706*/                 sub     eax, 3FFEh
		/*.text:0043870B*/                 mov     /*[ebp+var_2E+2]*/ [ebp-2eh+2], eax
		/*.text:0043870E*/                 mov     word ptr /*[ebp+var_2E]*/ [ebp-2eh], 0
		/*.text:00438714*/                 mov     /*[ebp+*/ var_44 /*]*/, 0
		/*.text:0043871B*/                 jmp     short loc_438726
		/*.text:0043871D*/ ; ---------------------------------------------------------------------------
		/*.text:0043871D*/ 
		/*.text:0043871D*/ loc_43871D:                             ; CODE XREF: __I10_OUTPUT+358.j
		/*.text:0043871D*/                 mov     ecx, /*[ebp+*/ var_44 /*]*/
		/*.text:00438720*/                 add     ecx, 1
		/*.text:00438723*/                 mov     /*[ebp+*/ var_44 /*]*/, ecx
		/*.text:00438726*/ 
		/*.text:00438726*/ loc_438726:                             ; CODE XREF: __I10_OUTPUT+33B.j
		/*.text:00438726*/                 cmp     /*[ebp+*/ var_44 /*]*/, 8
		/*.text:0043872A*/                 jge     short loc_43873A
		/*.text:0043872C*/                 lea     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:0043872F*/                 push    edx
		/*.text:00438730*/                 call    ___shl_12
		/*.text:00438735*/                 add     esp, 4
		/*.text:00438738*/                 jmp     short loc_43871D
		/*.text:0043873A*/ ; ---------------------------------------------------------------------------
		/*.text:0043873A*/ 
		/*.text:0043873A*/ loc_43873A:                             ; CODE XREF: __I10_OUTPUT+34A.j
		/*.text:0043873A*/                 cmp     dword ptr /*[ebp+var_2E+2]*/ [ebp-2eh+2], 0
		/*.text:0043873E*/                 jge     short loc_43876C
		/*.text:00438740*/                 mov     eax, /*[ebp+var_2E+2]*/ [ebp-2eh+2]
		/*.text:00438743*/                 neg     eax
		/*.text:00438745*/                 and     eax, 0FFh
		/*.text:0043874A*/                 mov     /*[ebp+*/ var_74 /*]*/, eax
		/*.text:0043874D*/                 jmp     short loc_438758
		/*.text:0043874F*/ ; ---------------------------------------------------------------------------
		/*.text:0043874F*/ 
		/*.text:0043874F*/ loc_43874F:                             ; CODE XREF: __I10_OUTPUT+38A.j
		/*.text:0043874F*/                 mov     ecx, /*[ebp+*/ var_74 /*]*/
		/*.text:00438752*/                 sub     ecx, 1
		/*.text:00438755*/                 mov     /*[ebp+*/ var_74 /*]*/, ecx
		/*.text:00438758*/ 
		/*.text:00438758*/ loc_438758:                             ; CODE XREF: __I10_OUTPUT+36D.j
		/*.text:00438758*/                 cmp     /*[ebp+*/ var_74 /*]*/, 0
		/*.text:0043875C*/                 jle     short loc_43876C
		/*.text:0043875E*/                 lea     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:00438761*/                 push    edx
		/*.text:00438762*/                 call    ___shr_12
		/*.text:00438767*/                 add     esp, 4
		/*.text:0043876A*/                 jmp     short loc_43874F
		/*.text:0043876C*/ ; ---------------------------------------------------------------------------
		/*.text:0043876C*/ 
		/*.text:0043876C*/ loc_43876C:                             ; CODE XREF: __I10_OUTPUT+35E.j
		/*.text:0043876C*/                                         ; __I10_OUTPUT+37C.j
		/*.text:0043876C*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:0043876F*/                 add     eax, 4
		/*.text:00438772*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00438775*/                 mov     ecx, /*[ebp+*/ arg_C /*]*/
		/*.text:00438778*/                 add     ecx, 1
		/*.text:0043877B*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:0043877E*/                 jmp     short loc_438789
		/*.text:00438780*/ ; ---------------------------------------------------------------------------
		/*.text:00438780*/ 
		/*.text:00438780*/ loc_438780:                             ; CODE XREF: __I10_OUTPUT+413.j
		/*.text:00438780*/                 mov     edx, /*[ebp+*/ var_10 /*]*/
		/*.text:00438783*/                 sub     edx, 1
		/*.text:00438786*/                 mov     /*[ebp+*/ var_10 /*]*/, edx
		/*.text:00438789*/ 
		/*.text:00438789*/ loc_438789:                             ; CODE XREF: __I10_OUTPUT+39E.j
		/*.text:00438789*/                 cmp     /*[ebp+*/ var_10 /*]*/, 0
		/*.text:0043878D*/                 jle     short loc_4387F5
		/*.text:0043878F*/                 mov     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:00438792*/                 mov     /*[ebp+*/ var_1C /*]*/, eax
		/*.text:00438795*/                 mov     ecx, [ebp-34h]
		/*.text:00438798*/                 mov     /*[ebp+*/ var_18 /*]*/, ecx
		/*.text:0043879B*/                 mov     edx, /*[ebp+var_32+2]*/ [ebp-32h+2]
		/*.text:0043879E*/                 mov     /*[ebp+*/ var_14 /*]*/, edx
		/*.text:004387A1*/                 lea     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:004387A4*/                 push    eax
		/*.text:004387A5*/                 call    ___shl_12
		/*.text:004387AA*/                 add     esp, 4
		/*.text:004387AD*/                 lea     ecx, /*[ebp+*/ var_38 /*]*/
		/*.text:004387B0*/                 push    ecx
		/*.text:004387B1*/                 call    ___shl_12
		/*.text:004387B6*/                 add     esp, 4
		/*.text:004387B9*/                 lea     edx, /*[ebp+*/ var_1C /*]*/
		/*.text:004387BC*/                 push    edx
		/*.text:004387BD*/                 lea     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:004387C0*/                 push    eax
		/*.text:004387C1*/                 call    ___add_12
		/*.text:004387C6*/                 add     esp, 8
		/*.text:004387C9*/                 lea     ecx, /*[ebp+*/ var_38 /*]*/
		/*.text:004387CC*/                 push    ecx
		/*.text:004387CD*/                 call    ___shl_12
		/*.text:004387D2*/                 add     esp, 4
		/*.text:004387D5*/                 mov     edx, /*[ebp+var_2E+1]*/ [ebp-2eh+1]
		/*.text:004387D8*/                 and     edx, 0FFh
		/*.text:004387DE*/                 add     edx, 30h
		/*.text:004387E1*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004387E4*/                 mov     [eax], dl
		/*.text:004387E6*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004387E9*/                 add     ecx, 1
		/*.text:004387EC*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:004387EF*/                 mov     byte ptr /*[ebp+var_2E+1]*/ [ebp-2eh+1], 0
		/*.text:004387F3*/                 jmp     short loc_438780
		/*.text:004387F5*/ ; ---------------------------------------------------------------------------
		/*.text:004387F5*/ 
		/*.text:004387F5*/ loc_4387F5:                             ; CODE XREF: __I10_OUTPUT+3AD.j
		/*.text:004387F5*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004387F8*/                 sub     edx, 1
		/*.text:004387FB*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:004387FE*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00438801*/                 mov     cl, [eax]
		/*.text:00438803*/                 mov     /*[ebp+*/ var_28 /*]*/, cl
		/*.text:00438806*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00438809*/                 sub     edx, 1
		/*.text:0043880C*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0043880F*/                 movsx   eax, /*[ebp+*/ var_28 /*]*/
		/*.text:00438813*/                 cmp     eax, 35h
		/*.text:00438816*/                 jl      short loc_438873
		/*.text:00438818*/                 jmp     short loc_438823
		/*.text:0043881A*/ ; ---------------------------------------------------------------------------
		/*.text:0043881A*/ 
		/*.text:0043881A*/ loc_43881A:                             ; CODE XREF: __I10_OUTPUT+45F.j
		/*.text:0043881A*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043881D*/                 sub     ecx, 1
		/*.text:00438820*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00438823*/ 
		/*.text:00438823*/ loc_438823:                             ; CODE XREF: __I10_OUTPUT+438.j
		/*.text:00438823*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438826*/                 add     edx, 4
		/*.text:00438829*/                 cmp     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0043882C*/                 jb      short loc_438841
		/*.text:0043882E*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00438831*/                 movsx   ecx, byte ptr [eax]
		/*.text:00438834*/                 cmp     ecx, 39h
		/*.text:00438837*/                 jnz     short loc_438841
		/*.text:00438839*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043883C*/                 mov     byte ptr [edx], 30h
		/*.text:0043883F*/                 jmp     short loc_43881A
		/*.text:00438841*/ ; ---------------------------------------------------------------------------
		/*.text:00438841*/ 
		/*.text:00438841*/ loc_438841:                             ; CODE XREF: __I10_OUTPUT+44C.j
		/*.text:00438841*/                                         ; __I10_OUTPUT+457.j
		/*.text:00438841*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438844*/                 add     eax, 4
		/*.text:00438847*/                 cmp     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0043884A*/                 jnb     short loc_438865
		/*.text:0043884C*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043884F*/                 add     ecx, 1
		/*.text:00438852*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00438855*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438858*/                 mov     ax, [edx]
		/*.text:0043885B*/                 add     ax, 1
		/*.text:0043885F*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438862*/                 mov     [ecx], ax
		/*.text:00438865*/ 
		/*.text:00438865*/ loc_438865:                             ; CODE XREF: __I10_OUTPUT+46A.j
		/*.text:00438865*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00438868*/                 mov     al, [edx]
		/*.text:0043886A*/                 add     al, 1
		/*.text:0043886C*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043886F*/                 mov     [ecx], al
		/*.text:00438871*/                 jmp     short loc_4388CC
		/*.text:00438873*/ ; ---------------------------------------------------------------------------
		/*.text:00438873*/ 
		/*.text:00438873*/ loc_438873:                             ; CODE XREF: __I10_OUTPUT+436.j
		/*.text:00438873*/                 jmp     short loc_43887E
		/*.text:00438875*/ ; ---------------------------------------------------------------------------
		/*.text:00438875*/ 
		/*.text:00438875*/ loc_438875:                             ; CODE XREF: __I10_OUTPUT+4B4.j
		/*.text:00438875*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00438878*/                 sub     edx, 1
		/*.text:0043887B*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0043887E*/ 
		/*.text:0043887E*/ loc_43887E:                             ; CODE XREF: __I10_OUTPUT+493.j
		/*.text:0043887E*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438881*/                 add     eax, 4
		/*.text:00438884*/                 cmp     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00438887*/                 jb      short loc_438896
		/*.text:00438889*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043888C*/                 movsx   edx, byte ptr [ecx]
		/*.text:0043888F*/                 cmp     edx, 30h
		/*.text:00438892*/                 jnz     short loc_438896
		/*.text:00438894*/                 jmp     short loc_438875
		/*.text:00438896*/ ; ---------------------------------------------------------------------------
		/*.text:00438896*/ 
		/*.text:00438896*/ loc_438896:                             ; CODE XREF: __I10_OUTPUT+4A7.j
		/*.text:00438896*/                                         ; __I10_OUTPUT+4B2.j
		/*.text:00438896*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:00438899*/                 add     eax, 4
		/*.text:0043889C*/                 cmp     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0043889F*/                 jnb     short loc_4388CC
		/*.text:004388A1*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388A4*/                 mov     word ptr [ecx], 0
		/*.text:004388A9*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388AC*/                 mov     byte ptr [edx+2], 20h
		/*.text:004388B0*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388B3*/                 mov     byte ptr [eax+3], 1
		/*.text:004388B7*/                 mov     ecx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388BA*/                 mov     byte ptr [ecx+4], 30h
		/*.text:004388BE*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388C1*/                 mov     byte ptr [edx+5], 0
		/*.text:004388C5*/                 mov     eax, 1
		/*.text:004388CA*/                 jmp     short loc_4388F2
		/*.text:004388CC*/ ; ---------------------------------------------------------------------------
		/*.text:004388CC*/ 
		/*.text:004388CC*/ loc_4388CC:                             ; CODE XREF: __I10_OUTPUT+491.j
		/*.text:004388CC*/                                         ; __I10_OUTPUT+4BF.j
		/*.text:004388CC*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388CF*/                 add     eax, 4
		/*.text:004388D2*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004388D5*/                 sub     ecx, eax
		/*.text:004388D7*/                 add     ecx, 1
		/*.text:004388DA*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388DD*/                 mov     [edx+3], cl
		/*.text:004388E0*/                 mov     eax, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388E3*/                 movsx   ecx, byte ptr [eax+3]
		/*.text:004388E7*/                 mov     edx, /*[ebp+*/ arg_14 /*]*/
		/*.text:004388EA*/                 mov     byte ptr [edx+ecx+4], 0
		/*.text:004388EF*/ 
		/*.text:004388EF*/ loc_4388EF:                             ; CODE XREF: __I10_OUTPUT+1E1.j
		/*.text:004388EF*/                 mov     eax, /*[ebp+*/ var_58 /*]*/
		/*.text:004388F2*/ 
		/*.text:004388F2*/ loc_4388F2:                             ; CODE XREF: __I10_OUTPUT+E7.j
		/*.text:004388F2*/                                         ; __I10_OUTPUT+30C.j ...
		/*.text:004388F2*/                 mov     esp, ebp
		/*.text:004388F4*/                 pop     ebp
		/*.text:004388F5*/                 retn
		/*.text:004388F5*/ // __I10_OUTPUT    endp
	}
}

#undef var_74
#undef var_70
#undef var_6C
#undef var_68
#undef var_64
#undef var_60
#undef var_5C
#undef var_58
#undef var_54
#undef var_53
#undef var_52
#undef var_51
#undef var_50
#undef var_4F
#undef var_4E
#undef var_4D
#undef var_4C
#undef var_4B
#undef var_4A
#undef var_49
#undef var_48
#undef var_44
#undef var_40
#undef var_3C
#undef var_38
#undef var_32
#undef var_2E
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
#undef arg_8
#undef arg_C
#undef arg_10
#undef arg_14

static VOID __declspec( naked ) __fltout2( VOID )
{
	__asm
	{
		/*.text:00433C40*/ // __fltout2       proc near               ; CODE XREF: __gcvt+16.p
		/*.text:00433C40*/                                         ; __cftoe+1E.p ...
		/*.text:00433C40*/ 
		/*.text:00433C40*/ #define var_28           word ptr [ebp-28h]
		/*.text:00433C40*/ #define var_26           byte ptr [ebp-26h]
		/*.text:00433C40*/ #define var_24           byte ptr [ebp-24h]
		/*.text:00433C40*/ #define var_C            dword ptr [ebp-0Ch]
		/*.text:00433C40*/ #define var_8            dword ptr [ebp-8]
		/*.text:00433C40*/ #define var_4            word ptr [ebp-4]
		/*.text:00433C40*/ #define arg_0            byte ptr  [ebp+8]
		/*.text:00433C40*/ #define arg_8            dword ptr  [ebp+10h]
		/*.text:00433C40*/ #define arg_C            dword ptr  [ebp+14h]
		/*.text:00433C40*/ 
		/*.text:00433C40*/                 push    ebp
		/*.text:00433C41*/                 mov     ebp, esp
		/*.text:00433C43*/                 sub     esp, 28h
		/*.text:00433C46*/                 lea     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00433C49*/                 push    eax
		/*.text:00433C4A*/                 lea     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00433C4D*/                 push    ecx
		/*.text:00433C4E*/                 call    ___dtold
		/*.text:00433C53*/                 add     esp, 8
		/*.text:00433C56*/                 lea     edx, /*[ebp+*/ var_28 /*]*/
		/*.text:00433C59*/                 push    edx
		/*.text:00433C5A*/                 push    0
		/*.text:00433C5C*/                 push    11h
		/*.text:00433C5E*/                 sub     esp, 0Ch
		/*.text:00433C61*/                 mov     eax, esp
		/*.text:00433C63*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00433C66*/                 mov     [eax], ecx
		/*.text:00433C68*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:00433C6B*/                 mov     [eax+4], edx
		/*.text:00433C6E*/                 mov     cx, /*[ebp+*/ var_4 /*]*/
		/*.text:00433C72*/                 mov     [eax+8], cx
		/*.text:00433C76*/                 call    __I10_OUTPUT
		/*.text:00433C7B*/                 add     esp, 18h
		/*.text:00433C7E*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00433C81*/                 mov     [edx+8], eax
		/*.text:00433C84*/                 movsx   eax, /*[ebp+*/ var_26 /*]*/
		/*.text:00433C88*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00433C8B*/                 mov     [ecx], eax
		/*.text:00433C8D*/                 movsx   edx, /*[ebp+*/ var_28 /*]*/
		/*.text:00433C91*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:00433C94*/                 mov     [eax+4], edx
		/*.text:00433C97*/                 lea     ecx, /*[ebp+*/ var_24 /*]*/
		/*.text:00433C9A*/                 push    ecx
		/*.text:00433C9B*/                 mov     edx, /*[ebp+*/ arg_C /*]*/
		/*.text:00433C9E*/                 push    edx
		/*.text:00433C9F*/                 call    __strcpy
		/*.text:00433CA4*/                 add     esp, 8
		/*.text:00433CA7*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:00433CAA*/                 mov     ecx, /*[ebp+*/ arg_C /*]*/
		/*.text:00433CAD*/                 mov     [eax+0Ch], ecx
		/*.text:00433CB0*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:00433CB3*/                 mov     esp, ebp
		/*.text:00433CB5*/                 pop     ebp
		/*.text:00433CB6*/                 retn
		/*.text:00433CB6*/ // __fltout2       endp
	}
}

#undef var_28
#undef var_26
#undef var_24
#undef var_C
#undef var_8
#undef var_4
#undef arg_0
#undef arg_8
#undef arg_C

static VOID __declspec( naked ) _strlen( VOID )
{
	__asm
	{
		/*.text:00426250*/ // _strlen         proc near               ; CODE XREF: __strdup+12.p
		/*.text:00426250*/                                         ; __CrtDbgReport+2BA.p ...
		/*.text:00426250*/ 
		/*.text:00426250*/ #define arg_0            dword ptr  [esp+0x4]
		/*.text:00426250*/ 
		/*.text:00426250*/                 mov     ecx, /*[esp+*/ arg_0 /*]*/
		/*.text:00426254*/                 test    ecx, 3
		/*.text:0042625A*/                 jz      short loc_426270
		/*.text:0042625C*/ 
		/*.text:0042625C*/ loc_42625C:                             ; CODE XREF: _strlen+19.j
		/*.text:0042625C*/                 mov     al, [ecx]
		/*.text:0042625E*/                 inc     ecx
		/*.text:0042625F*/                 test    al, al
		/*.text:00426261*/                 jz      short loc_4262A3
		/*.text:00426263*/                 test    ecx, 3
		/*.text:00426269*/                 jnz     short loc_42625C
		/*.text:0042626B*/                 add     eax, 0
		/*.text:00426270*/ 
		/*.text:00426270*/ loc_426270:                             ; CODE XREF: _strlen+A.j
		/*.text:00426270*/                                         ; _strlen+36.j ...
		/*.text:00426270*/                 mov     eax, [ecx]
		/*.text:00426272*/                 mov     edx, 7EFEFEFFh
		/*.text:00426277*/                 add     edx, eax
		/*.text:00426279*/                 xor     eax, 0FFFFFFFFh
		/*.text:0042627C*/                 xor     eax, edx
		/*.text:0042627E*/                 add     ecx, 4
		/*.text:00426281*/                 test    eax, 81010100h
		/*.text:00426286*/                 jz      short loc_426270
		/*.text:00426288*/                 mov     eax, [ecx-4]
		/*.text:0042628B*/                 test    al, al
		/*.text:0042628D*/                 jz      short loc_4262C1
		/*.text:0042628F*/                 test    ah, ah
		/*.text:00426291*/                 jz      short loc_4262B7
		/*.text:00426293*/                 test    eax, 0FF0000h
		/*.text:00426298*/                 jz      short loc_4262AD
		/*.text:0042629A*/                 test    eax, 0FF000000h
		/*.text:0042629F*/                 jz      short loc_4262A3
		/*.text:004262A1*/                 jmp     short loc_426270
		/*.text:004262A3*/ ; ---------------------------------------------------------------------------
		/*.text:004262A3*/ 
		/*.text:004262A3*/ loc_4262A3:                             ; CODE XREF: _strlen+11.j
		/*.text:004262A3*/                                         ; _strlen+4F.j
		/*.text:004262A3*/                 lea     eax, [ecx-1]
		/*.text:004262A6*/                 mov     ecx, /*[esp+*/ arg_0 /*]*/
		/*.text:004262AA*/                 sub     eax, ecx
		/*.text:004262AC*/                 retn
		/*.text:004262AD*/ ; ---------------------------------------------------------------------------
		/*.text:004262AD*/ 
		/*.text:004262AD*/ loc_4262AD:                             ; CODE XREF: _strlen+48.j
		/*.text:004262AD*/                 lea     eax, [ecx-2]
		/*.text:004262B0*/                 mov     ecx, /*[esp+*/ arg_0 /*]*/
		/*.text:004262B4*/                 sub     eax, ecx
		/*.text:004262B6*/                 retn
		/*.text:004262B7*/ ; ---------------------------------------------------------------------------
		/*.text:004262B7*/ 
		/*.text:004262B7*/ loc_4262B7:                             ; CODE XREF: _strlen+41.j
		/*.text:004262B7*/                 lea     eax, [ecx-3]
		/*.text:004262BA*/                 mov     ecx, /*[esp+*/ arg_0 /*]*/
		/*.text:004262BE*/                 sub     eax, ecx
		/*.text:004262C0*/                 retn
		/*.text:004262C1*/ ; ---------------------------------------------------------------------------
		/*.text:004262C1*/ 
		/*.text:004262C1*/ loc_4262C1:                             ; CODE XREF: _strlen+3D.j
		/*.text:004262C1*/                 lea     eax, [ecx-4]
		/*.text:004262C4*/                 mov     ecx, /*[esp+*/ arg_0 /*]*/
		/*.text:004262C8*/                 sub     eax, ecx
		/*.text:004262CA*/                 retn
		/*.text:004262CA*/ // _strlen         endp
	}
}

#undef arg_0

static VOID __declspec( naked ) __fptostr( VOID )
{
	__asm
	{
		/*.text:00435600*/ // __fptostr       proc near               ; CODE XREF: __cftoe+4D.p
		/*.text:00435600*/                                         ; __cftof+45.p ...
		/*.text:00435600*/ 
		/*.text:00435600*/ #define var_C            dword ptr [ebp-0Ch]
		/*.text:00435600*/ #define var_8            dword ptr [ebp-8]
		/*.text:00435600*/ #define var_4            dword ptr [ebp-4]
		/*.text:00435600*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:00435600*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:00435600*/ #define arg_8            dword ptr  [ebp+10h]
		/*.text:00435600*/ 
		/*.text:00435600*/                 push    ebp
		/*.text:00435601*/                 mov     ebp, esp
		/*.text:00435603*/                 sub     esp, 0Ch
		/*.text:00435606*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00435609*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0043560C*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0043560F*/                 mov     edx, [ecx+0Ch]
		/*.text:00435612*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00435615*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00435618*/                 mov     byte ptr [eax], 30h
		/*.text:0043561B*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043561E*/                 add     ecx, 1
		/*.text:00435621*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00435624*/ 
		/*.text:00435624*/ loc_435624:                             ; CODE XREF: __fptostr+69.j
		/*.text:00435624*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:00435628*/                 jle     short loc_43566B
		/*.text:0043562A*/                 mov     edx, /*[ebp+*/ var_8 /*]*/
		/*.text:0043562D*/                 movsx   eax, byte ptr [edx]
		/*.text:00435630*/                 test    eax, eax
		/*.text:00435632*/                 jz      short loc_435648
		/*.text:00435634*/                 mov     ecx, /*[ebp+*/ var_8 /*]*/
		/*.text:00435637*/                 movsx   edx, byte ptr [ecx]
		/*.text:0043563A*/                 mov     /*[ebp+*/ var_C /*]*/, edx
		/*.text:0043563D*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00435640*/                 add     eax, 1
		/*.text:00435643*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:00435646*/                 jmp     short loc_43564F
		/*.text:00435648*/ ; ---------------------------------------------------------------------------
		/*.text:00435648*/ 
		/*.text:00435648*/ loc_435648:                             ; CODE XREF: __fptostr+32.j
		/*.text:00435648*/                 mov     /*[ebp+*/ var_C /*]*/, 30h
		/*.text:0043564F*/ 
		/*.text:0043564F*/ loc_43564F:                             ; CODE XREF: __fptostr+46.j
		/*.text:0043564F*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00435652*/                 mov     dl, /*byte ptr [ebp+var_C]*/ [ebp-0Ch]
		/*.text:00435655*/                 mov     [ecx], dl
		/*.text:00435657*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0043565A*/                 add     eax, 1
		/*.text:0043565D*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00435660*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00435663*/                 sub     ecx, 1
		/*.text:00435666*/                 mov     /*[ebp+*/ arg_4 /*]*/, ecx
		/*.text:00435669*/                 jmp     short loc_435624
		/*.text:0043566B*/ ; ---------------------------------------------------------------------------
		/*.text:0043566B*/ 
		/*.text:0043566B*/ loc_43566B:                             ; CODE XREF: __fptostr+28.j
		/*.text:0043566B*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0043566E*/                 mov     byte ptr [edx], 0
		/*.text:00435671*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:00435675*/                 jl      short loc_4356B4
		/*.text:00435677*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0043567A*/                 movsx   ecx, byte ptr [eax]
		/*.text:0043567D*/                 cmp     ecx, 35h
		/*.text:00435680*/                 jl      short loc_4356B4
		/*.text:00435682*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00435685*/                 sub     edx, 1
		/*.text:00435688*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:0043568B*/ 
		/*.text:0043568B*/ loc_43568B:                             ; CODE XREF: __fptostr+A5.j
		/*.text:0043568B*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0043568E*/                 movsx   ecx, byte ptr [eax]
		/*.text:00435691*/                 cmp     ecx, 39h
		/*.text:00435694*/                 jnz     short loc_4356A7
		/*.text:00435696*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00435699*/                 mov     byte ptr [edx], 30h
		/*.text:0043569C*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0043569F*/                 sub     eax, 1
		/*.text:004356A2*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004356A5*/                 jmp     short loc_43568B
		/*.text:004356A7*/ ; ---------------------------------------------------------------------------
		/*.text:004356A7*/ 
		/*.text:004356A7*/ loc_4356A7:                             ; CODE XREF: __fptostr+94.j
		/*.text:004356A7*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004356AA*/                 mov     dl, [ecx]
		/*.text:004356AC*/                 add     dl, 1
		/*.text:004356AF*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004356B2*/                 mov     [eax], dl
		/*.text:004356B4*/ 
		/*.text:004356B4*/ loc_4356B4:                             ; CODE XREF: __fptostr+75.j
		/*.text:004356B4*/                                         ; __fptostr+80.j
		/*.text:004356B4*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004356B7*/                 movsx   edx, byte ptr [ecx]
		/*.text:004356BA*/                 cmp     edx, 31h
		/*.text:004356BD*/                 jnz     short loc_4356D0
		/*.text:004356BF*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004356C2*/                 mov     ecx, [eax+4]
		/*.text:004356C5*/                 add     ecx, 1
		/*.text:004356C8*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:004356CB*/                 mov     [edx+4], ecx
		/*.text:004356CE*/                 jmp     short loc_4356F6
		/*.text:004356D0*/ ; ---------------------------------------------------------------------------
		/*.text:004356D0*/ 
		/*.text:004356D0*/ loc_4356D0:                             ; CODE XREF: __fptostr+BD.j
		/*.text:004356D0*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004356D3*/                 add     eax, 1
		/*.text:004356D6*/                 push    eax
		/*.text:004356D7*/                 call    _strlen
		/*.text:004356DC*/                 add     esp, 4
		/*.text:004356DF*/                 add     eax, 1
		/*.text:004356E2*/                 push    eax
		/*.text:004356E3*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004356E6*/                 add     ecx, 1
		/*.text:004356E9*/                 push    ecx
		/*.text:004356EA*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004356ED*/                 push    edx
		/*.text:004356EE*/                 call    memmove
		/*.text:004356F3*/                 add     esp, 0Ch
		/*.text:004356F6*/ 
		/*.text:004356F6*/ loc_4356F6:                             ; CODE XREF: __fptostr+CE.j
		/*.text:004356F6*/                 mov     esp, ebp
		/*.text:004356F8*/                 pop     ebp
		/*.text:004356F9*/                 retn
		/*.text:004356F9*/ // __fptostr       endp
	}
}

#undef var_C
#undef var_8
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8

static VOID __declspec( naked ) sub_429650( VOID )
{
	__asm
	{
		/*.text:00429650*/ // sub_429650      proc near               ; CODE XREF: sub_4291E0+2F.p
		/*.text:00429650*/                                         ; sub_4293C0+7F.p ...
		/*.text:00429650*/ 
		/*.text:00429650*/ #define arg_0            dword ptr  [ebp+0x8]
		/*.text:00429650*/ #define arg_4            dword ptr  [ebp+0x0C]
		/*.text:00429650*/ 
		/*.text:00429650*/                 push    ebp
		/*.text:00429651*/                 mov     ebp, esp
		/*.text:00429653*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:00429657*/                 jz      short loc_42967C
		/*.text:00429659*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0042965C*/                 push    eax
		/*.text:0042965D*/                 call    _strlen
		/*.text:00429662*/                 add     esp, 4
		/*.text:00429665*/                 add     eax, 1
		/*.text:00429668*/                 push    eax
		/*.text:00429669*/                 mov     ecx, /*[ebp+*/ arg_0 /*]*/
		/*.text:0042966C*/                 push    ecx
		/*.text:0042966D*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00429670*/                 add     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00429673*/                 push    edx
		/*.text:00429674*/                 call    memmove
		/*.text:00429679*/                 add     esp, 0Ch
		/*.text:0042967C*/ 
		/*.text:0042967C*/ loc_42967C:                             ; CODE XREF: sub_429650+7.j
		/*.text:0042967C*/                 pop     ebp
		/*.text:0042967D*/                 retn
		/*.text:0042967D*/ // sub_429650      endp
	}
}

#undef arg_0
#undef arg_4

static VOID __declspec( naked ) _memset( VOID )
{
	__asm
	{
		/*.text:0041DF90*/ // _memset         proc near               ; CODE XREF: test(int,int)+20.p
		/*.text:0041DF90*/                                         ; COleControlSite::COleControlSite(COleControlContainer *)+1AD.p ...
		/*.text:0041DF90*/ 
		/*.text:0041DF90*/ #define arg_0            dword ptr  [esp+0x4]
		/*.text:0041DF90*/ #define arg_4            byte ptr  [esp+0x8]
		/*.text:0041DF90*/ #define arg_8            dword ptr  [esp+0x0C]
		/*.text:0041DF90*/ 
		/*.text:0041DF90*/                 mov     edx, /*[esp+*/ arg_8 /*]*/
		/*.text:0041DF94*/                 mov     ecx, /*[esp+*/ arg_0 /*]*/
		/*.text:0041DF98*/                 test    edx, edx
		/*.text:0041DF9A*/                 jz      short loc_41DFE3
		/*.text:0041DF9C*/                 xor     eax, eax
		/*.text:0041DF9E*/                 mov     al, /*[esp+*/ arg_4 /*]*/
		/*.text:0041DFA2*/                 push    edi
		/*.text:0041DFA3*/                 mov     edi, ecx
		/*.text:0041DFA5*/                 cmp     edx, 4
		/*.text:0041DFA8*/                 jb      short loc_41DFD7
		/*.text:0041DFAA*/                 neg     ecx
		/*.text:0041DFAC*/                 and     ecx, 3
		/*.text:0041DFAF*/                 jz      short loc_41DFB9
		/*.text:0041DFB1*/                 sub     edx, ecx
		/*.text:0041DFB3*/ 
		/*.text:0041DFB3*/ loc_41DFB3:                             ; CODE XREF: _memset+27.j
		/*.text:0041DFB3*/                 mov     [edi], al
		/*.text:0041DFB5*/                 inc     edi
		/*.text:0041DFB6*/                 dec     ecx
		/*.text:0041DFB7*/                 jnz     short loc_41DFB3
		/*.text:0041DFB9*/ 
		/*.text:0041DFB9*/ loc_41DFB9:                             ; CODE XREF: _memset+1F.j
		/*.text:0041DFB9*/                 mov     ecx, eax
		/*.text:0041DFBB*/                 shl     eax, 8
		/*.text:0041DFBE*/                 add     eax, ecx
		/*.text:0041DFC0*/                 mov     ecx, eax
		/*.text:0041DFC2*/                 shl     eax, 10h
		/*.text:0041DFC5*/                 add     eax, ecx
		/*.text:0041DFC7*/                 mov     ecx, edx
		/*.text:0041DFC9*/                 and     edx, 3
		/*.text:0041DFCC*/                 shr     ecx, 2
		/*.text:0041DFCF*/                 jz      short loc_41DFD7
		/*.text:0041DFD1*/                 repe stosd
		/*.text:0041DFD3*/                 test    edx, edx
		/*.text:0041DFD5*/                 jz      short loc_41DFDD
		/*.text:0041DFD7*/ 
		/*.text:0041DFD7*/ loc_41DFD7:                             ; CODE XREF: _memset+18.j
		/*.text:0041DFD7*/                                         ; _memset+3F.j ...
		/*.text:0041DFD7*/                 mov     [edi], al
		/*.text:0041DFD9*/                 inc     edi
		/*.text:0041DFDA*/                 dec     edx
		/*.text:0041DFDB*/                 jnz     short loc_41DFD7
		/*.text:0041DFDD*/ 
		/*.text:0041DFDD*/ loc_41DFDD:                             ; CODE XREF: _memset+45.j
		/*.text:0041DFDD*/                 mov     eax, [esp+ /*4+arg_0*/ 8 ]
		/*.text:0041DFE1*/                 pop     edi
		/*.text:0041DFE2*/                 retn
		/*.text:0041DFE3*/ ; ---------------------------------------------------------------------------
		/*.text:0041DFE3*/ 
		/*.text:0041DFE3*/ loc_41DFE3:                             ; CODE XREF: _memset+A.j
		/*.text:0041DFE3*/                 mov     eax, /*[esp+*/ arg_0 /*]*/
		/*.text:0041DFE7*/                 retn
		/*.text:0041DFE7*/ // _memset         endp
	}
}

#undef arg_0
#undef arg_4
#undef arg_8

static VOID __declspec( naked ) sub_4293C0( VOID )
{
	__asm
	{
		/*.text:004293C0*/ // sub_4293C0      proc near               ; CODE XREF: __cftof+5B.p
		/*.text:004293C0*/                                         ; __cftog+D9.p
		/*.text:004293C0*/ 
		/*.text:004293C0*/ #define var_10           dword ptr [ebp-0x10]
		/*.text:004293C0*/ #define var_C            dword ptr [ebp-0x0C]
		/*.text:004293C0*/ #define var_8            dword ptr [ebp-0x8]
		/*.text:004293C0*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:004293C0*/ #define arg_0            dword ptr  [ebp+0x8]
		/*.text:004293C0*/ #define arg_4            dword ptr  [ebp+0x0C]
		/*.text:004293C0*/ #define arg_8            dword ptr  [ebp+0x10]
		/*.text:004293C0*/ #define arg_C            byte ptr  [ebp+0x14]
		/*.text:004293C0*/ 
		/*.text:004293C0*/                 push    ebp
		/*.text:004293C1*/                 mov     ebp, esp
		/*.text:004293C3*/                 sub     esp, 10h
		/*.text:004293C6*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004293C9*/                 mov     ecx, [eax+4]
		/*.text:004293CC*/                 sub     ecx, 1
		/*.text:004293CF*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:004293D2*/                 movsx   edx, /*[ebp+*/ arg_C /*]*/
		/*.text:004293D6*/                 test    edx, edx
		/*.text:004293D8*/                 jz      short loc_429413
		/*.text:004293DA*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004293DD*/                 xor     ecx, ecx
		/*.text:004293DF*/                 cmp     dword ptr [eax], 2Dh
		/*.text:004293E2*/                 setz    cl
		/*.text:004293E5*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:004293E8*/                 add     edx, ecx
		/*.text:004293EA*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:004293ED*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:004293F0*/                 cmp     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:004293F3*/                 jnz     short loc_429413
		/*.text:004293F5*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004293F8*/                 add     ecx, /*[ebp+*/ var_8 /*]*/
		/*.text:004293FB*/                 mov     /*[ebp+*/ var_C /*]*/, ecx
		/*.text:004293FE*/                 mov     edx, /*[ebp+*/ var_C /*]*/
		/*.text:00429401*/                 mov     byte ptr [edx], 30h
		/*.text:00429404*/                 mov     eax, /*[ebp+*/ var_C /*]*/
		/*.text:00429407*/                 add     eax, 1
		/*.text:0042940A*/                 mov     /*[ebp+*/ var_C /*]*/, eax
		/*.text:0042940D*/                 mov     ecx, /*[ebp+*/ var_C /*]*/
		/*.text:00429410*/                 mov     byte ptr [ecx], 0
		/*.text:00429413*/ 
		/*.text:00429413*/ loc_429413:                             ; CODE XREF: sub_4293C0+18.j
		/*.text:00429413*/                                         ; sub_4293C0+33.j
		/*.text:00429413*/                 mov     edx, /*[ebp+*/ arg_0 /*]*/
		/*.text:00429416*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00429419*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:0042941C*/                 cmp     dword ptr [eax], 2Dh
		/*.text:0042941F*/                 jnz     short loc_429430
		/*.text:00429421*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429424*/                 mov     byte ptr [ecx], 2Dh
		/*.text:00429427*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042942A*/                 add     edx, 1
		/*.text:0042942D*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00429430*/ 
		/*.text:00429430*/ loc_429430:                             ; CODE XREF: sub_4293C0+5F.j
		/*.text:00429430*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:00429433*/                 cmp     dword ptr [eax+4], 0
		/*.text:00429437*/                 jg      short loc_429458
		/*.text:00429439*/                 push    1
		/*.text:0042943B*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042943E*/                 push    ecx
		/*.text:0042943F*/                 call    sub_429650      ; __shift
		/*.text:00429444*/                 add     esp, 8
		/*.text:00429447*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042944A*/                 mov     byte ptr [edx], 30h
		/*.text:0042944D*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00429450*/                 add     eax, 1
		/*.text:00429453*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00429456*/                 jmp     short loc_429464
		/*.text:00429458*/ ; ---------------------------------------------------------------------------
		/*.text:00429458*/ 
		/*.text:00429458*/ loc_429458:                             ; CODE XREF: sub_4293C0+77.j
		/*.text:00429458*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:0042945B*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042945E*/                 add     edx, [ecx+4]
		/*.text:00429461*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00429464*/ 
		/*.text:00429464*/ loc_429464:                             ; CODE XREF: sub_4293C0+96.j
		/*.text:00429464*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:00429468*/                 jle     loc_4294F6
		/*.text:0042946E*/                 push    1
		/*.text:00429470*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00429473*/                 push    eax
		/*.text:00429474*/                 call    sub_429650      ; __shift
		/*.text:00429479*/                 add     esp, 8
		/*.text:0042947C*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042947F*/                 mov     dl, ___decimal_point
		/*.text:00429485*/                 mov     [ecx], dl
		/*.text:00429487*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0042948A*/                 add     eax, 1
		/*.text:0042948D*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00429490*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00429493*/                 cmp     dword ptr [ecx+4], 0
		/*.text:00429497*/                 jge     short loc_4294F6
		/*.text:00429499*/                 movsx   edx, /*[ebp+*/ arg_C /*]*/
		/*.text:0042949D*/                 test    edx, edx
		/*.text:0042949F*/                 jz      short loc_4294AE
		/*.text:004294A1*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004294A4*/                 mov     ecx, [eax+4]
		/*.text:004294A7*/                 neg     ecx
		/*.text:004294A9*/                 mov     /*[ebp+*/ arg_4 /*]*/, ecx
		/*.text:004294AC*/                 jmp     short loc_4294D4
		/*.text:004294AE*/ ; ---------------------------------------------------------------------------
		/*.text:004294AE*/ 
		/*.text:004294AE*/ loc_4294AE:                             ; CODE XREF: sub_4293C0+DF.j
		/*.text:004294AE*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:004294B1*/                 mov     eax, [edx+4]
		/*.text:004294B4*/                 neg     eax
		/*.text:004294B6*/                 cmp     /*[ebp+*/ arg_4 /*]*/, eax
		/*.text:004294B9*/                 jge     short loc_4294C3
		/*.text:004294BB*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004294BE*/                 mov     /*[ebp+*/ var_10 /*]*/, ecx
		/*.text:004294C1*/                 jmp     short loc_4294CE
		/*.text:004294C3*/ ; ---------------------------------------------------------------------------
		/*.text:004294C3*/ 
		/*.text:004294C3*/ loc_4294C3:                             ; CODE XREF: sub_4293C0+F9.j
		/*.text:004294C3*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:004294C6*/                 mov     eax, [edx+4]
		/*.text:004294C9*/                 neg     eax
		/*.text:004294CB*/                 mov     /*[ebp+*/ var_10 /*]*/, eax
		/*.text:004294CE*/ 
		/*.text:004294CE*/ loc_4294CE:                             ; CODE XREF: sub_4293C0+101.j
		/*.text:004294CE*/                 mov     ecx, /*[ebp+*/ var_10 /*]*/
		/*.text:004294D1*/                 mov     /*[ebp+*/ arg_4 /*]*/, ecx
		/*.text:004294D4*/ 
		/*.text:004294D4*/ loc_4294D4:                             ; CODE XREF: sub_4293C0+EC.j
		/*.text:004294D4*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004294D7*/                 push    edx
		/*.text:004294D8*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004294DB*/                 push    eax
		/*.text:004294DC*/                 call    sub_429650      ; __shift
		/*.text:004294E1*/                 add     esp, 8
		/*.text:004294E4*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004294E7*/                 push    ecx
		/*.text:004294E8*/                 push    30h
		/*.text:004294EA*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004294ED*/                 push    edx
		/*.text:004294EE*/                 call    _memset
		/*.text:004294F3*/                 add     esp, 0Ch
		/*.text:004294F6*/ 
		/*.text:004294F6*/ loc_4294F6:                             ; CODE XREF: sub_4293C0+A8.j
		/*.text:004294F6*/                                         ; sub_4293C0+D7.j
		/*.text:004294F6*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004294F9*/                 mov     esp, ebp
		/*.text:004294FB*/                 pop     ebp
		/*.text:004294FC*/                 retn
		/*.text:004294FC*/ // sub_4293C0      endp
	}
}

#undef var_10
#undef var_C
#undef var_8
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8
#undef arg_C

static VOID __declspec( naked ) __cftof( VOID )
{
	__asm
	{
		/*.text:00429350*/ // __cftof         proc near               ; CODE XREF: __gcvt+74.p
		/*.text:00429350*/                                         ; __cfltcvt+3B.p
		/*.text:00429350*/ 
		/*.text:00429350*/ #define var_2C           byte ptr [ebp-0x2C]
		/*.text:00429350*/ #define var_14           byte ptr [ebp-0x14]
		/*.text:00429350*/ #define var_4            dword ptr [ebp-0x4]
		/*.text:00429350*/ #define arg_0            dword ptr  [ebp+0x8]
		/*.text:00429350*/ #define arg_4            dword ptr  [ebp+0x0C]
		/*.text:00429350*/ #define arg_8            dword ptr  [ebp+0x10]
		/*.text:00429350*/ 
		/*.text:00429350*/                 push    ebp
		/*.text:00429351*/                 mov     ebp, esp
		/*.text:00429353*/                 sub     esp, 2Ch
		/*.text:00429356*/                 lea     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:00429359*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0042935C*/                 lea     ecx, /*[ebp+*/ var_2C /*]*/
		/*.text:0042935F*/                 push    ecx
		/*.text:00429360*/                 lea     edx, /*[ebp+*/ var_14 /*]*/
		/*.text:00429363*/                 push    edx
		/*.text:00429364*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00429367*/                 mov     ecx, [eax+4]
		/*.text:0042936A*/                 push    ecx
		/*.text:0042936B*/                 mov     edx, [eax]
		/*.text:0042936D*/                 push    edx
		/*.text:0042936E*/                 call    __fltout2
		/*.text:00429373*/                 add     esp, 10h
		/*.text:00429376*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00429379*/                 push    eax
		/*.text:0042937A*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042937D*/                 mov     edx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00429380*/                 add     edx, [ecx+4]
		/*.text:00429383*/                 push    edx
		/*.text:00429384*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00429387*/                 xor     ecx, ecx
		/*.text:00429389*/                 cmp     dword ptr [eax], 2Dh
		/*.text:0042938C*/                 setz    cl
		/*.text:0042938F*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00429392*/                 add     edx, ecx
		/*.text:00429394*/                 push    edx
		/*.text:00429395*/                 call    __fptostr
		/*.text:0042939A*/                 add     esp, 0Ch
		/*.text:0042939D*/                 push    0
		/*.text:0042939F*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004293A2*/                 push    eax
		/*.text:004293A3*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:004293A6*/                 push    ecx
		/*.text:004293A7*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004293AA*/                 push    edx
		/*.text:004293AB*/                 call    sub_4293C0      ; __cftof2
		/*.text:004293B0*/                 add     esp, 10h
		/*.text:004293B3*/                 mov     eax, /*[ebp+*/ arg_4 /*]*/
		/*.text:004293B6*/                 mov     esp, ebp
		/*.text:004293B8*/                 pop     ebp
		/*.text:004293B9*/                 retn
		/*.text:004293B9*/ // __cftof         endp
	}
}

#undef var_2C
#undef var_14
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8

//============================
// cftof Function Definition.
//============================

CHAR* __cdecl cftof( IN DOUBLE dValue, OUT CHAR* pszBuffer, IN ULONG ulPrecision )
{
	CHAR*			pszRetVal;
	CHAR*			pszPtr;
	BOOLEAN			bPtrDec;

	//
	// Call the CRT Function.
	//

	__asm
	{
		mov			eax, ulPrecision
		push		eax
		mov			eax, pszBuffer
		push		eax
		lea			eax, dValue
		push		eax
		call		__cftof
		add			esp, 0Ch
		mov			pszRetVal, eax
	}

	//
	// Remove the Last Zeros.
	//

	if ( pszRetVal )
	{
		pszPtr = pszRetVal + strlen( pszRetVal ) - 1;

		bPtrDec = FALSE;

		while( pszPtr >= pszRetVal &&
			* pszPtr == '0' )
		{
			pszPtr --;
			bPtrDec = TRUE;
		}

		if ( pszPtr >= pszRetVal &&
			bPtrDec )
		{
			if ( * pszPtr == '.' )
				* ( pszPtr + 2 ) = 0;
			else
				* ( pszPtr + 1 ) = 0;
		}
	}

	//
	// Return.
	//

	return pszRetVal;
}

//==========================
// cftog Grabbed Functions.
//==========================

static CHAR aE000[]           /*db*/ = "e+000"; // ,0

static VOID __declspec( naked ) sub_4291E0( VOID )
{
	__asm
	{
		/*.text:004291E0*/ // sub_4291E0      proc near               ; CODE XREF: __cftoe+67.p
		/*.text:004291E0*/                                         ; __cftog+9D.p
		/*.text:004291E0*/ 
		/*.text:004291E0*/ #define var_8            dword ptr [ebp-8]
		/*.text:004291E0*/ #define var_4            dword ptr [ebp-4]
		/*.text:004291E0*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:004291E0*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:004291E0*/ #define arg_8            dword ptr  [ebp+10h]
		/*.text:004291E0*/ #define arg_C            dword ptr  [ebp+14h]
		/*.text:004291E0*/ #define arg_10           byte ptr  [ebp+18h]
		/*.text:004291E0*/ 
		/*.text:004291E0*/                 push    ebp
		/*.text:004291E1*/                 mov     ebp, esp
		/*.text:004291E3*/                 sub     esp, 8
		/*.text:004291E6*/                 movsx   eax, /*[ebp+*/ arg_10 /*]*/
		/*.text:004291EA*/                 test    eax, eax
		/*.text:004291EC*/                 jz      short loc_429217
		/*.text:004291EE*/                 mov     ecx, /*[ebp+*/ arg_C /*]*/
		/*.text:004291F1*/                 xor     edx, edx
		/*.text:004291F3*/                 cmp     dword ptr [ecx], 2Dh
		/*.text:004291F6*/                 setz    dl
		/*.text:004291F9*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:004291FC*/                 add     eax, edx
		/*.text:004291FE*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00429201*/                 xor     ecx, ecx
		/*.text:00429203*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:00429207*/                 setnle  cl
		/*.text:0042920A*/                 push    ecx
		/*.text:0042920B*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042920E*/                 push    edx
		/*.text:0042920F*/                 call    sub_429650      ; __shift
		/*.text:00429214*/                 add     esp, 8
		/*.text:00429217*/ 
		/*.text:00429217*/ loc_429217:                             ; CODE XREF: sub_4291E0+C.j
		/*.text:00429217*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0042921A*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0042921D*/                 mov     ecx, /*[ebp+*/ arg_C /*]*/
		/*.text:00429220*/                 cmp     dword ptr [ecx], 2Dh
		/*.text:00429223*/                 jnz     short loc_429234
		/*.text:00429225*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429228*/                 mov     byte ptr [edx], 2Dh
		/*.text:0042922B*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:0042922E*/                 add     eax, 1
		/*.text:00429231*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:00429234*/ 
		/*.text:00429234*/ loc_429234:                             ; CODE XREF: sub_4291E0+43.j
		/*.text:00429234*/                 cmp     /*[ebp+*/ arg_4 /*]*/, 0
		/*.text:00429238*/                 jle     short loc_429258
		/*.text:0042923A*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042923D*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429240*/                 mov     al, [edx+1]
		/*.text:00429243*/                 mov     [ecx], al
		/*.text:00429245*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429248*/                 add     ecx, 1
		/*.text:0042924B*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0042924E*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429251*/                 mov     al, ___decimal_point
		/*.text:00429256*/                 mov     [edx], al
		/*.text:00429258*/ 
		/*.text:00429258*/ loc_429258:                             ; CODE XREF: sub_4291E0+58.j
		/*.text:00429258*/                 push    offset aE000    ; "e+000"
		/*.text:0042925D*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429260*/                 add     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00429263*/                 movsx   edx, /*[ebp+*/ arg_10 /*]*/
		/*.text:00429267*/                 neg     edx
		/*.text:00429269*/                 sbb     edx, edx
		/*.text:0042926B*/                 inc     edx
		/*.text:0042926C*/                 add     ecx, edx
		/*.text:0042926E*/                 push    ecx
		/*.text:0042926F*/                 call    __strcpy
		/*.text:00429274*/                 add     esp, 8
		/*.text:00429277*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:0042927A*/                 cmp     /*[ebp+*/ arg_8 /*]*/, 0
		/*.text:0042927E*/                 jz      short loc_429286
		/*.text:00429280*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00429283*/                 mov     byte ptr [eax], 45h
		/*.text:00429286*/ 
		/*.text:00429286*/ loc_429286:                             ; CODE XREF: sub_4291E0+9E.j
		/*.text:00429286*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429289*/                 add     ecx, 1
		/*.text:0042928C*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:0042928F*/                 mov     edx, /*[ebp+*/ arg_C /*]*/
		/*.text:00429292*/                 mov     eax, [edx+0Ch]
		/*.text:00429295*/                 movsx   ecx, byte ptr [eax]
		/*.text:00429298*/                 cmp     ecx, 30h
		/*.text:0042929B*/                 jz      loc_42933F
		/*.text:004292A1*/                 mov     edx, /*[ebp+*/ arg_C /*]*/
		/*.text:004292A4*/                 mov     eax, [edx+4]
		/*.text:004292A7*/                 sub     eax, 1
		/*.text:004292AA*/                 mov     /*[ebp+*/ var_8 /*]*/, eax
		/*.text:004292AD*/                 cmp     /*[ebp+*/ var_8 /*]*/, 0
		/*.text:004292B1*/                 jge     short loc_4292C1
		/*.text:004292B3*/                 mov     ecx, /*[ebp+*/ var_8 /*]*/
		/*.text:004292B6*/                 neg     ecx
		/*.text:004292B8*/                 mov     /*[ebp+*/ var_8 /*]*/, ecx
		/*.text:004292BB*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004292BE*/                 mov     byte ptr [edx], 2Dh
		/*.text:004292C1*/ 
		/*.text:004292C1*/ loc_4292C1:                             ; CODE XREF: sub_4291E0+D1.j
		/*.text:004292C1*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004292C4*/                 add     eax, 1
		/*.text:004292C7*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004292CA*/                 cmp     /*[ebp+*/ var_8 /*]*/, 64h
		/*.text:004292CE*/                 jl      short loc_4292F5
		/*.text:004292D0*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:004292D3*/                 cdq
		/*.text:004292D4*/                 mov     ecx, 64h
		/*.text:004292D9*/                 idiv    ecx
		/*.text:004292DB*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004292DE*/                 mov     cl, [edx]
		/*.text:004292E0*/                 add     cl, al
		/*.text:004292E2*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004292E5*/                 mov     [edx], cl
		/*.text:004292E7*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:004292EA*/                 cdq
		/*.text:004292EB*/                 mov     ecx, 64h
		/*.text:004292F0*/                 idiv    ecx
		/*.text:004292F2*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:004292F5*/ 
		/*.text:004292F5*/ loc_4292F5:                             ; CODE XREF: sub_4291E0+EE.j
		/*.text:004292F5*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:004292F8*/                 add     edx, 1
		/*.text:004292FB*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:004292FE*/                 cmp     /*[ebp+*/ var_8 /*]*/, 0Ah
		/*.text:00429302*/                 jl      short loc_429329
		/*.text:00429304*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:00429307*/                 cdq
		/*.text:00429308*/                 mov     ecx, 0Ah
		/*.text:0042930D*/                 idiv    ecx
		/*.text:0042930F*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429312*/                 mov     cl, [edx]
		/*.text:00429314*/                 add     cl, al
		/*.text:00429316*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429319*/                 mov     [edx], cl
		/*.text:0042931B*/                 mov     eax, /*[ebp+*/ var_8 /*]*/
		/*.text:0042931E*/                 cdq
		/*.text:0042931F*/                 mov     ecx, 0Ah
		/*.text:00429324*/                 idiv    ecx
		/*.text:00429326*/                 mov     /*[ebp+*/ var_8 /*]*/, edx
		/*.text:00429329*/ 
		/*.text:00429329*/ loc_429329:                             ; CODE XREF: sub_4291E0+122.j
		/*.text:00429329*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042932C*/                 add     edx, 1
		/*.text:0042932F*/                 mov     /*[ebp+*/ var_4 /*]*/, edx
		/*.text:00429332*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:00429335*/                 mov     cl, [eax]
		/*.text:00429337*/                 add     cl, [ebp-8]
		/*.text:0042933A*/                 mov     edx, /*[ebp+*/ var_4 /*]*/
		/*.text:0042933D*/                 mov     [edx], cl
		/*.text:0042933F*/ 
		/*.text:0042933F*/ loc_42933F:                             ; CODE XREF: sub_4291E0+BB.j
		/*.text:0042933F*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:00429342*/                 mov     esp, ebp
		/*.text:00429344*/                 pop     ebp
		/*.text:00429345*/                 retn
		/*.text:00429345*/ // sub_4291E0      endp
	}
}

#undef var_8
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8
#undef arg_C
#undef arg_10

static VOID __declspec( naked ) __cftog( VOID )
{
	__asm
	{
		/*.text:00429500*/ // __cftog         proc near               ; CODE XREF: __cfltcvt+55.p
		/*.text:00429500*/ 
		/*.text:00429500*/ #define var_38           dword ptr [ebp-38h]
		/*.text:00429500*/ #define var_34           dword ptr [ebp-34h]
		/*.text:00429500*/ #define var_30           byte ptr [ebp-30h]
		/*.text:00429500*/ #define var_2C           byte ptr [ebp-2Ch]
		/*.text:00429500*/ #define var_14           byte ptr [ebp-14h]
		/*.text:00429500*/ #define var_4            dword ptr [ebp-4]
		/*.text:00429500*/ #define arg_0            dword ptr  [ebp+8]
		/*.text:00429500*/ #define arg_4            dword ptr  [ebp+0Ch]
		/*.text:00429500*/ #define arg_8            dword ptr  [ebp+10h]
		/*.text:00429500*/ #define arg_C            dword ptr  [ebp+14h]
		/*.text:00429500*/ 
		/*.text:00429500*/                 push    ebp
		/*.text:00429501*/                 mov     ebp, esp
		/*.text:00429503*/                 sub     esp, 38h
		/*.text:00429506*/                 mov     /*[ebp+*/ var_30 /*]*/, 0
		/*.text:0042950A*/                 lea     eax, /*[ebp+*/ var_14 /*]*/
		/*.text:0042950D*/                 mov     /*[ebp+*/ var_38 /*]*/, eax
		/*.text:00429510*/                 lea     ecx, /*[ebp+*/ var_2C /*]*/
		/*.text:00429513*/                 push    ecx
		/*.text:00429514*/                 lea     edx, /*[ebp+*/ var_14 /*]*/
		/*.text:00429517*/                 push    edx
		/*.text:00429518*/                 mov     eax, /*[ebp+*/ arg_0 /*]*/
		/*.text:0042951B*/                 mov     ecx, [eax+4]
		/*.text:0042951E*/                 push    ecx
		/*.text:0042951F*/                 mov     edx, [eax]
		/*.text:00429521*/                 push    edx
		/*.text:00429522*/                 call    __fltout2
		/*.text:00429527*/                 add     esp, 10h
		/*.text:0042952A*/                 mov     eax, /*[ebp+*/ var_38 /*]*/
		/*.text:0042952D*/                 mov     ecx, [eax+4]
		/*.text:00429530*/                 sub     ecx, 1
		/*.text:00429533*/                 mov     /*[ebp+*/ var_34 /*]*/, ecx
		/*.text:00429536*/                 mov     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:00429539*/                 xor     eax, eax
		/*.text:0042953B*/                 cmp     dword ptr [edx], 2Dh
		/*.text:0042953E*/                 setz    al
		/*.text:00429541*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:00429544*/                 add     ecx, eax
		/*.text:00429546*/                 mov     /*[ebp+*/ var_4 /*]*/, ecx
		/*.text:00429549*/                 mov     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:0042954C*/                 push    edx
		/*.text:0042954D*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:00429550*/                 push    eax
		/*.text:00429551*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:00429554*/                 push    ecx
		/*.text:00429555*/                 call    __fptostr
		/*.text:0042955A*/                 add     esp, 0Ch
		/*.text:0042955D*/                 mov     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:00429560*/                 mov     eax, [edx+4]
		/*.text:00429563*/                 sub     eax, 1
		/*.text:00429566*/                 xor     ecx, ecx
		/*.text:00429568*/                 cmp     /*[ebp+*/ var_34 /*]*/, eax
		/*.text:0042956B*/                 setl    cl
		/*.text:0042956E*/                 mov     /*[ebp+*/ var_30 /*]*/, cl
		/*.text:00429571*/                 mov     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:00429574*/                 mov     eax, [edx+4]
		/*.text:00429577*/                 sub     eax, 1
		/*.text:0042957A*/                 mov     /*[ebp+*/ var_34 /*]*/, eax
		/*.text:0042957D*/                 cmp     /*[ebp+*/ var_34 /*]*/, 0FFFFFFFCh
		/*.text:00429581*/                 jl      short loc_42958B
		/*.text:00429583*/                 mov     ecx, /*[ebp+*/ var_34 /*]*/
		/*.text:00429586*/                 cmp     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00429589*/                 jl      short loc_4295A7
		/*.text:0042958B*/ 
		/*.text:0042958B*/ loc_42958B:                             ; CODE XREF: __cftog+81.j
		/*.text:0042958B*/                 push    1
		/*.text:0042958D*/                 mov     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:00429590*/                 push    edx
		/*.text:00429591*/                 mov     eax, /*[ebp+*/ arg_C /*]*/
		/*.text:00429594*/                 push    eax
		/*.text:00429595*/                 mov     ecx, /*[ebp+*/ arg_8 /*]*/
		/*.text:00429598*/                 push    ecx
		/*.text:00429599*/                 mov     edx, /*[ebp+*/ arg_4 /*]*/
		/*.text:0042959C*/                 push    edx
		/*.text:0042959D*/                 call    sub_4291E0      ; __cftoe2
		/*.text:004295A2*/                 add     esp, 14h
		/*.text:004295A5*/                 jmp     short loc_4295E1
		/*.text:004295A7*/ ; ---------------------------------------------------------------------------
		/*.text:004295A7*/ 
		/*.text:004295A7*/ loc_4295A7:                             ; CODE XREF: __cftog+89.j
		/*.text:004295A7*/                 movsx   eax, /*[ebp+*/ var_30 /*]*/
		/*.text:004295AB*/                 test    eax, eax
		/*.text:004295AD*/                 jz      short loc_4295CB
		/*.text:004295AF*/ 
		/*.text:004295AF*/ loc_4295AF:                             ; CODE XREF: __cftog+C2.j
		/*.text:004295AF*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004295B2*/                 movsx   edx, byte ptr [ecx]
		/*.text:004295B5*/                 mov     eax, /*[ebp+*/ var_4 /*]*/
		/*.text:004295B8*/                 add     eax, 1
		/*.text:004295BB*/                 mov     /*[ebp+*/ var_4 /*]*/, eax
		/*.text:004295BE*/                 test    edx, edx
		/*.text:004295C0*/                 jz      short loc_4295C4
		/*.text:004295C2*/                 jmp     short loc_4295AF
		/*.text:004295C4*/ ; ---------------------------------------------------------------------------
		/*.text:004295C4*/ 
		/*.text:004295C4*/ loc_4295C4:                             ; CODE XREF: __cftog+C0.j
		/*.text:004295C4*/                 mov     ecx, /*[ebp+*/ var_4 /*]*/
		/*.text:004295C7*/                 mov     byte ptr [ecx-2], 0
		/*.text:004295CB*/ 
		/*.text:004295CB*/ loc_4295CB:                             ; CODE XREF: __cftog+AD.j
		/*.text:004295CB*/                 push    1
		/*.text:004295CD*/                 mov     edx, /*[ebp+*/ var_38 /*]*/
		/*.text:004295D0*/                 push    edx
		/*.text:004295D1*/                 mov     eax, /*[ebp+*/ arg_8 /*]*/
		/*.text:004295D4*/                 push    eax
		/*.text:004295D5*/                 mov     ecx, /*[ebp+*/ arg_4 /*]*/
		/*.text:004295D8*/                 push    ecx
		/*.text:004295D9*/                 call    sub_4293C0      ; __cftof2
		/*.text:004295DE*/                 add     esp, 10h
		/*.text:004295E1*/ 
		/*.text:004295E1*/ loc_4295E1:                             ; CODE XREF: __cftog+A5.j
		/*.text:004295E1*/                 mov     esp, ebp
		/*.text:004295E3*/                 pop     ebp
		/*.text:004295E4*/                 retn
		/*.text:004295E4*/ // __cftog         endp
	}
}

#undef var_38
#undef var_34
#undef var_30
#undef var_2C
#undef var_14
#undef var_4
#undef arg_0
#undef arg_4
#undef arg_8
#undef arg_C

//============================
// cftog Function Definition.
//============================

CHAR* __cdecl cftog( IN DOUBLE dValue, OUT CHAR* pszBuffer, IN ULONG ulPrecision, IN BOOLEAN bCapilalizeExp )
{
	CHAR*			pszRetVal;

	//
	// Call the CRT Function.
	//

	__asm
	{
		xor			eax, eax
		mov			al, bCapilalizeExp
		push		eax
		mov			eax, ulPrecision
		push		eax
		mov			eax, pszBuffer
		push		eax
		lea			eax, dValue
		push		eax
		call		__cftog
		add			esp, 10h
		mov			pszRetVal, eax
	}

	//
	// Return.
	//

	return pszRetVal;
}
