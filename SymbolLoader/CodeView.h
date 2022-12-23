//
// CodeView.h
//

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

#pragma warning (disable: 4200)

//
// Definitions.
//

#pragma pack ( push, 1 )

	typedef struct lfEasy {
		unsigned short  leaf;
	} lfEasy;

	typedef enum LEAF_ENUM_e {
		LF_NUMERIC          = 0x8000,
		LF_CHAR             = 0x8000,
		LF_USHORT           = 0x8002,
		LF_SHORT            = 0x8001,
		LF_LONG             = 0x8003,
		LF_ULONG            = 0x8004,
		LF_QUADWORD         = 0x8009,
		LF_STRUCTURE        = 0x1005,
		LF_FIELDLIST        = 0x1203,
		LF_MEMBER           = 0x1405,
		LF_UQUADWORD        = 0x800a
	} LEAF_ENUM_e;

	typedef struct TYPTYPE {
		unsigned short  len;
		unsigned short  leaf;
		unsigned char   data[0];
	} TYPTYPE;

	typedef unsigned short CV_prop_t;
	typedef unsigned long CV_typ_t;
	typedef unsigned short CV_fldattr_t;

	typedef struct lfClass {
		unsigned short  leaf;
		unsigned short  count;
		CV_prop_t       property;
		CV_typ_t        field;
		CV_typ_t        derived;
		CV_typ_t        vshape;
		unsigned char   data[0];
	} lfClass;

	typedef struct lfFieldList {
		unsigned short  leaf;
		char            data[0];
	} lfFieldList;

	typedef struct lfMember {
		unsigned short  leaf;
		CV_fldattr_t    attr;
		CV_typ_t        index;
		unsigned char   offset[0];
	} lfMember;

#pragma pack ( pop )
