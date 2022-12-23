/***************************************************************************************
  *
  * ccomp.h - VpcICE MiniC Compiler Header File.
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

#define		max_console_messages	1024

//============
// Externals.
//============

// Variabili releative alla console.
extern char				*console_messages[max_console_messages];
extern int				current_console_message;
extern char				*error_files;

// Per evitare che il compilatore venga inizializzato due volte.
extern float compiler_memory_factor;

// Numero di errori verificatisi e quantità di memoria allocata.
extern int compiler_errors,compiler_warnings;
extern float compiler_memory;

// Fatal_error: impostato a 1 quando si verifica un errore fatale.
extern int fatal_error;

// Lista dei files dai quali il source file dipende.
extern char *dependencies_mem;

//=============
// Structures.
//=============

// Bisogna passare questa struttura al compilatore.
typedef struct {
	char	source_file[256]; // senza directory e con estensione
	char	compiled_file[256]; // con directory e con estensione
	char	PSI_file[256]; // null string = non produce PSI output; (con directory e con estensione)
	char	source_directory[256]; // con slash finale
	char	headers_directory[256]; // con slash finale
	char	definitions[1024]; // definizioni separate da una virgola
	float	compiler_memory_factor; // 1.0 = memoria allocata normalmente
} CompileSourceOptions_t;

//=============================
// OBJ File Format Structures.
//=============================

#pragma pack(1)
typedef struct {
	int		offset;
	int		size;
} entry_t;
#pragma pack()

#pragma pack(1)
struct header_s {
	int			magic_number;
	entry_t		code;
	entry_t		relocation;
	entry_t		names;
	entry_t		functions;
	entry_t		extfunctions;
	entry_t		strings;
	int			mem_required;
	entry_t		PCODE_code;
	entry_t		PCODE_functions;
} header;
#pragma pack()

#pragma pack(1)
typedef struct {
	int		address;
} relocation_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	int		address;
	int		name_address;
} function_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	int		address;
	int		name_address;
} extfunction_t;
#pragma pack()

// // //

typedef struct header_s header_t;

//==============================
// OBJ File Format Definitions.
//==============================

// Magic number ( primi due byte = versione; ultimi due byte = 'o4' ).
#define		o4_magic_number		0x0002346f

// Nome della funzione di inizializzazione.
#define				o4_initialization_function_name			"_______initialization"

//======================
// Function Prototypes.
//======================

// Per inizializzare il compilatore.
int	InitCompiler(void);

// Libera la memoria allocata dal compilatore MiniC.
void FreeCompilerMemory (void);

// Bisogna chiamare questa funzione per avviare la compilazione.
void CompileSource (CompileSourceOptions_t *CompileSourceOptions);
