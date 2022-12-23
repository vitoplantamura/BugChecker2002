/***************************************************************************************
  *
  * ccomp.c - VpcICE MiniC Compiler Source File.
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

//==============
// Definitions.
//==============

#define MALLOC(x)			ExAllocatePool( NonPagedPool, x )
#define FREE(x)				ExFreePool( x )

//=====================
//header files + prototypes + variables + structs
//
//include gli header files che servono al compilatore e dichiara le funzioni e le strutture interne
//=====================

#include "..\Include\ccomp.h"
#include "..\Include\crt.h"
#include "..\Include\memfile.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#pragma warning( disable : 4554 )
#pragma warning( disable : 4242 )

//=================
//structs
//
//definizioni delle strutture utilizzate internamente dal compilatore
//=================
typedef struct {
	int				id;
	int				tok_id;

	int				left_tok;
	int				right_tok;
	int				third_tok;

	void			*pointer; // usato solo da alcuni operatori
} operator_t;

#define identifier_max_len		204
typedef struct identifier_s {
	int		type;			// all
	char	id[identifier_max_len];		// all
	int		address;		// VARIABLE,LABEL,FUNCTION(not used),STRUCT(sizeof),STRUCT_FIELD(offset),FUNCTION_PARAMETER(sizeof)
	int		fields;			// STRUCT,FUNCTION
	int		indirection;	// VARIABLE,STRUCT_FIELD,FUNCTION,FUNCTION_PARAMETER
	int		dimensions;		// VARIABLE,STRUCT_FIELD,FUNCTION_PARAMETER
	int		const_flag;		// VARIABLE,STRUCT_FIELD,FUNCTION,FUNCTION_PARAMETER
	int		static_flag;	// VARIABLE,FUNCTION
	void	*pointer;		// (char*)MACRO, (int*)VARIABLE,STRUCT_FIELD,FUNCTION_PARAMETER(dimension list)
	char	*initializer;	// VARIABLE
	int		initializer_line;	// VARIABLE
	struct	identifier_s	*struct_type;		// VARIABLE,STRUCT_FIELD,FUNCTION,FUNCTION_PARAMETER
	int		array_stptr;	// VARIABLE
	int		ellipses_arguments; // FUNCTION
} identifier_t;

typedef struct {
	int				id;
	void			*ptr0;
	int				tok_taken;
	char			*ptr1;
	int				sizeof_result;
	int				typecast_type;
	int				typecast_indir;
	identifier_t	*typecast_stype;
	int				func_call_has_one_param;
	int				func_call_commas;
	int				comma_number;
	int				comma_func_tok;
	int				ellipses_arguments_dim;
} token_t;

typedef struct {
	int				is_keyword;
	int				is_operator;
	identifier_t	*is_identifier;
	char			*is_string;

	int		len;
} token2_t;

typedef struct {
	int		type;
	char	number[8];
} number_t;

typedef struct {
	int				lenght;
	unsigned char	*bytes;
	int				constant_size;
	int				addresses[10];
	int				temp_addresses[10];
	int				constants[10];
} psi_t;

#define		LEFT_VALUE_0	0x000000	// allocato nell'area dati
#define		LEFT_VALUE_1	0x010000	// allocato nello stack
#define		RIGHT_VALUE		0x020000	// allocato nell'area temporanea delle espressioni
typedef struct {
	int				type;
	int				address;
	int				indirection;
	int				dimensions;
	int				*dimensions_list;
	int				const_flag;
	identifier_t	*struct_type;
	int				is_pointer;		// uguale a 1 se l'oggetto è un RIGHT_VALUE ma non si trova
									// nella SECTION_TEMPDATA1 poichè address si riferisce ad
									// un suo puntatore. Oggetti di questo tipo sono lvalue,
									// poichè risultati di operatori quali, ad esempio, l'array
									// subscript.
	int				array_stptr;	// l'oggetto deve essere un LEFT_VALUE_1 e deve essere un
									// array (dimensions!=0). Uguale a 1 se l'oggetto è un
									// pointer alla matrice.
} nonnumber_t;

#define		NUMBER		0x0000
#define		NON_NUMBER	0x0001
#define		NOTHING_	0x0002	// void (valore fittizio restituito da una funzione void x (...))
typedef struct {
	int			type;
	number_t	number;
	nonnumber_t	non_number;
} exprvalue_t;

//===============
//prototypes
//
//prototipi delle funzioni interne del compilatore
//===============
__inline int CompareStrings(char *string0, char *string1, int len);
int	ParseExpressionTokens(char *expr, int buffer);
int	IsString(char *pointer);
void ConsolePrintf(char *string, ...);
int	MakeExprOrderedOpList(int buffer);
void CompilerErrorOrWarning(int error);
int	ConvertStringToNumber(number_t *number, char *string);
static int IsIdentfString(char *pointer);
int	ResolveConstantExpression(number_t *result, char *string);
int	InitCompiler(void);
static int EliminateEscapeSequences (char *string0, char *string1, int s0_len, int warn);
void CastTypeOfConstantNumber (number_t *number0, number_t *number1, int type);
int ParseVariableDeclaration (char *pointer, int is_struct_member, int is_parameter, int is_function);
int ParseStructDeclaration (char *pointer);
int ParseFunctionDeclaration (char *pointer);
int ResolveConstantOperation (operator_t *op_ptr, number_t *buffer, token_t *tok_ptr, int op_id);
int ResolveNonConstantOperation (operator_t *op_ptr, exprvalue_t *buffer, token_t *tok_ptr, int op_id);
int PSI_PushNonNumber (nonnumber_t *nn, identifier_t *par);
int CastTypeOfNonConstantNumber (nonnumber_t *nn0, nonnumber_t *nn1, int standard_conversion);
int ResolveNonConstantExpression (exprvalue_t *result, char *string);
int GeneratePSIOutput (MEMFILE_NAME(FILE) *stream, int PSI0index, int PSI1index);
int ReducePSICode (int PSI0index, int PSI1index);
int IsTypeCastToken (token_t *tok);
int InitializePreprocessorStack (char *file);
int ReadLineFromSource (char *buffer);
void PreprocStackFcloseAll (void);
int ReadStatementFromSource (void);
int LinkOperations (operator_t *stack, int *list, int index);
void ReorderStackForLogicalAndConditionalOpAndComma (operator_t *stack, token_t *tokens);
int ProcessNextStatement (void);
void StatementProcessed (void);
void WriteOutputFile (void);
int ProcessExternalDefinitions (void);
void FreeCompilerMemory (void);

//================
//variables
//
//variabili globali e #define del compilatore
//================

// dimensione dello stack delle parentesi
#define		parentheses_stack_dim	4096

// memoria riservata ai token di una espressione
#define		expr_tokens_mem				100*1024
#define		single_expr_tokens_buffer	25*1024
token_t		*expr_tokens=NULL;

// memoria riservata agli identificatori di una espressione
#define		identifiers_mem				100*1024
#define		single_identifiers_buffer	25*1024
char		*identifiers=NULL;

// memoria riservata agli operatori di una espressione
#define		operators_mem				100*1024
#define		single_operators_buffer		25*1024
operator_t	*operators=NULL;

// linea corrente di compilazione
int			current_line;

// memoria e variabili riservate alla console
#define		console_dim				128*1024
char		*console=NULL;
char		*console_messages[max_console_messages];
int			current_console_message=0;
char		*console_ptr; // pointer

// variabili e memoria riservata agli identificatori
#define			cident_mem				5*1024*1024
identifier_t	*cident=NULL;
int				cident_id;
int				cident_id_local;
#define			cident_aux_mem			512*1024
void			*cident_aux=NULL;
void			*cident_aux_ptr;
#define			cident_aux2_mem			512*1024
void			*cident_aux2=NULL;
void			*cident_aux2_ptr;

// memoria a disposizione di varie operazioni
#define			auxmem_dim				512*1024
void			*auxmem=NULL;

// token dell'espressione di minore priorità (quindi risultato dell'espressione)
int				result_id=-1;

// memoria a disposizione di ResolveConstantExpression
#define			rcemem_dim				10*1024
void			*rcemem=NULL;

// numero massimo di dichiarazione di variabili nella stessa linea di codice
#define			max_num_of_multiple_declarations	1024

// massimo numero di dimensioni per una stessa matrice
#define			max_num_of_dimensions				256

// indirizzo della memoria destinata alla prossima variabile dichiarata e al prossimo membro di struttura e alla prossima variabile temporanea
int				cur_global_var_address;
int				cur_local_var_address;
int				cur_fld_address;
int				cur_tmp_address;

// numero di struttura senza tipo dichiarata
int				struct_num;

// fattore utilizzato quando viene allocata la memoria del compilatore
float			compiler_memory_factor=1.0f;
float			compiler_memory_old_factor=0.0f;

// dimensione della memoria utilizzata per il controllo degli out of memory del compiler
#define			check_point_offset		1024

// fatal_error: impostato a 1 quando si verifica un errore fatale
int				fatal_error;

// directory con slash finale
char			source_directory[256]="";
char			headers_directory[256]="";

// nome del file nel quale si è verificato l'ultimo errore
char			error_file[256];

// numero massimo di operatori collegati
#define			max_num_of_linked_operators		1024

// impostato a n quando è legale: char a[5]; a[0]="12345"; // con n=5 (len stringa)
int				allow_special_char_assignment=0;

// specifica l'allineamento dei membri delle strutture dichiarate
#define			struct_alignment_default		4
int				struct_alignment;

// impostato a 1 quando si tratta di un prototipo
int				is_prototype;

// rappresenta il puntatore al prototipo della funzione
identifier_t	*func_prototype;

// impostato a 1 quando le routine di riduzione vengono richiamate all'esterno di ResolveNonConstantExpression
int				DeactivateAutomaticCodeReduction=0;

// impostato a 1 quando l'espressione è costituita da un solo operatore e DeactivateAutomaticCodeReduction == 0
int				single_operator;

// file sorgente e file compilato (senza directory e con estensione)
char			source_file[256]="";
char			compiled_file[256]="";
char			PSI_file[256]="";

// stream del file compilato
MEMFILE_NAME(FILE)	*output_stream;

// numero di errori e di warning generati dal compilatore
int				compiler_errors;
int				compiler_warnings;

// numero massimo di errori e warnings visualizzabili
#define			max_num_of_errors_and_warnings		35

// array di char contenente tutte le definizioni passate al compilatore (separate da una virgola)
char	definitions[1024];

// lunghezza massima del file di errore stampato nella console
#define		max_console_error_file_len			52

//====================
//.o4 file specification / compilation memory
//
//Definizioni e strutture per i file .o4 e direttive della memoria per la compilazione
//====================
#define		SECTION_HEADER			0x0000
#define		SECTION_CODE			0x0001
#define		SECTION_RELOCATION		0x0002
#define		SECTION_NAMES			0x0003
#define		SECTION_FUNCTIONS		0x0004
#define		SECTION_EXTFUNCTIONS	0x0005
#define		SECTION_STRINGS			0x0006
//allocated run-time
#define		SECTION_DATA			0x0007
#define		SECTION_TEMPDATA0		0x0008
#define		SECTION_TEMPDATA1		0x0009
//special
#define		SECTION_STACK			0x000a
#define		SECTION_PSI				0x000b
#define		SECTION_LABELS			0x000c

// code
typedef struct {
	int		section;
	int		address;
} address_t;
typedef struct {
	int			psi;
	address_t	address;
	int			constant;
	// my_address: utilizzato alla fine del processo di compilazione
	char		*my_address;
} instruction_t;

//-------------------------------- pcode_instruction_t

#define PCODE_SECTION_NONE		0
#define PCODE_SECTION_NAMES		1
#define PCODE_SECTION_STRINGS	2
#define PCODE_SECTION_DATA		3
#define PCODE_SECTION_TEMPDATA	4
#define PCODE_SECTION_STACK		5
#define PCODE_SECTION_PCODE		6

#pragma pack(1)
typedef struct {
	DWORD			psi;
	DWORD			section;
	DWORD			address;
	DWORD			constant;
} pcode_instruction_t;
#pragma pack()

//----------------------------------------------------

#define				psi_mem_dim				5*1024*1024
instruction_t		*psi_mem=NULL;
instruction_t		*psi_ptr=NULL;

#define				code_mem_dim			3*1024*1024
char				*code=NULL;
char				*code_ptr=NULL;

// relocation
#define				reloc_mem_dim			512*1024
relocation_t		*relocation=NULL;
relocation_t		*relocation_ptr=NULL;

// names
#define				names_mem_dim			512*1024
char				*names=NULL;
char				*names_ptr=NULL;

// functions
#define				func_mem_dim			128*1024
function_t			*functions=NULL;
function_t			*functions_ptr=NULL;

// functions2 (usato per il PCODE)
function_t			*functions2=NULL;
function_t			*functions2_ptr=NULL;

// extfunctions
#define				ext_func_mem_dim		128*1024
extfunction_t		*extfunctions=NULL;
extfunction_t		*extfunctions_ptr=NULL;

// strings
#define				strings_mem_dim			1*1024*1024
char				*strings=NULL;
char				*strings_ptr=NULL;

// SECTION_TEMPDATA0/1 (memoria temporanea riservata rispettivamente alle PSI e alle espressioni)
#define				section_tempdata0_dim		1024
#define				section_tempdata1_dim		16*1024

// labels (utilizzato dal compilatore, distrutto alla fine della compilazione)
#define				labels_mem_dim			128*1024
char				*labels=NULL;
char				*labels_ptr=NULL;
#define				labels_ptrs_mem_dim		64*1024
instruction_t		**labels_ptrs=NULL;
instruction_t		**labels_ptrs_ptr=NULL;

// lista dei files nei quali si è verificato un errore (associato alle righe della console)
#define				error_files_mem_dim			256*1024
char				*error_files=NULL;
char				*error_files_ptr=NULL;

// lista dei files dai quali il source file dipende
#define				dependencies_mem_dim		64*1024
char				*dependencies_mem=NULL;

//===================
//preprocessor
//
//Variabili e #define del preprocessore
//===================
#define PREPROC_INCLUDE		0x0000
#define PREPROC_IF			0x0001
#define PREPROC_IFDEF		0x0002
#define PREPROC_IFNDEF		0x0003
#define PREPROC_ELSE		0x0004

// item dello stack del preprocessore
typedef struct {
	int			id;
	MEMFILE_NAME(FILE)	*stream;	// include
	char		filename[256];		// include
	int			line;				// include
	int			condition;			// if / ifdef / ifndef / else
} preproc_item_t;

// stack del preprocessore
#define				preproc_stack_dim			1*1024*1024
preproc_item_t		*preproc_stack;
preproc_item_t		*preproc_stack_ptr;

// nome del file attualmente in compilazione
char				current_file[256];
MEMFILE_NAME(FILE)	*current_file_stream;

// #define dei codici restituiti da ReadLineFromSource
#define				READLINE_OK			0
#define				READLINE_ERROR		1
#define				READLINE_FATAL		2
#define				READLINE_REPEAT		3
#define				READLINE_END		4

// controllo del livello dei condizionali nelle direttive del preprocessore
int					preprocessor_conditional_level;

// lunghezza massima di una linea di codice
#define				max_code_line_lenght		4096

// numero di file aperti contemporaneamente
int					used_streams;

//=====================
//main_stack
//
//Variabili e #define del main_stack
//=====================
#define MSTACK_BRACE					0x0000
#define MSTACK_DO						0x0001
#define MSTACK_FOR						0x0002
#define MSTACK_WHILE					0x0003
#define MSTACK_IF						0x0004
#define MSTACK_ELSE						0x0005

// item del main_stack
typedef struct {
	int					id; // all
	int					aux_mem_required; // all
	char				*loop_expression; // for
	int					loop_line; // for
	int					identifier; // brace
	void				*identifier_aux_mem; // brace
	int					accepts_declarations; // brace
	instruction_t		*enter_address; // brace
	int					if_do_have_statement; // if, do
	instruction_t		*condition_address; // for, while
	instruction_t		*jump_address; // for, while, if, else
	instruction_t		*do_address; // do
	instruction_t		**continue_stack; // do
	instruction_t		**continue_stack_pointer; // do
	instruction_t		**break_stack; // do, for, while
	instruction_t		**break_stack_pointer; // do, for, while
} main_item_t;

// dimensione della memoria riservata agli stack dei continue o dei break
#define				continue_break_stack_dim	16*1024

// caratteristiche e puntatori della memoria riservata al main_stack
#define				main_stack_dim				128*1024
main_item_t			*main_stack;
main_item_t			*main_stack_ptr;
#define				main_stack_aux_mem_dim		1*1024*1024
void				*main_stack_aux_mem;
void				*main_stack_aux_mem_ptr;

// lunghezza massima di un singolo statement e array per memorizzarlo
#define				max_statement_lenght		16*1024
char				statement[max_statement_lenght];
char				auxstatement[max_statement_lenght];

// #define dei codici restituiti da ReadStatementFromSource (statement non restituito)
#define				READSTATEMENT_FATAL				0
#define				READSTATEMENT_END				1
#define				READSTATEMENT_REPEAT			2
#define				READSTATEMENT_ERROR				3
// ai seguenti codici corrisponde uno statement restituito nell'array char statement
#define				READSTATEMENT_OK				4
#define				READSTATEMENT_LABEL				5
#define				READSTATEMENT_OBRACE			6
#define				READSTATEMENT_CBRACE			7
#define				READSTATEMENT_NULL				8
#define				READSTATEMENT_KEYWORD_DO		9
#define				READSTATEMENT_KEYWORD_FOR		10
#define				READSTATEMENT_KEYWORD_WHILE		11
#define				READSTATEMENT_KEYWORD_IF		12
#define				READSTATEMENT_KEYWORD_ELSE		13
#define				READSTATEMENT_FUNCTIONDECL		14
#define				READSTATEMENT_STRUCTUREDECL		15
#define				READSTATEMENT_VARIABLEDECL		16

// se lo statement C è incompleto e prosegue alla linea successiva
char				*statement_incomplete;

// se lo statement seguente inizia in una linea già letta
char				*statement_start;

// array che contiene i puntatori ai tre argomenti di un for
char				*for_arguments[3][2];
int					for_arguments_p;

// #define dei codici restituiti da ProcessNextStatement
#define				PROCESSNSTATEMENT_FATAL			0
#define				PROCESSNSTATEMENT_END			1
#define				PROCESSNSTATEMENT_OK			2

// lunghezza massima di un'espressione del for o del while
#define				max_for_while_expression_len	1024

// indirizzo del prototipo della funzione nella quale si trova correntemente il parser
identifier_t		*cur_prototype;

//=====================
//Generate_PSI_Output
//
//#define e variabili per la generazione del codice psi su file di testo
//=====================

// stream del file del sorgente PSI
MEMFILE_NAME(FILE)	*psi_output_stream;
int					psi_output_stream_index;

// definizione della struttura per memorizzare i psi_bounds
typedef struct {
	int				instruction_id;
	int				text_id;
} psi_bound_t;

// limiti nel codice PSI di ciascuno statement processato
#define			psi_bounds_mem_dim		1024*1024
psi_bound_t		*psi_bounds=NULL;
psi_bound_t		*psi_bounds_ptr;

// nome del file temporaneo
#define			psi_output_temporary_file_name		"$$$$$$$$.$$$"

//=====================
//c operators
//
//definizioni di tutti gli operatori c con precedenza e associatività
//=====================

// group -1 (non-operators)
#define O_PARENTHESES					0xffff		// (
#define C_PARENTHESES					0xfffe		// )
#define IDENTIFIER						0xfffd		// abc
#define EXPRESSION_END					0xfffc		// 0
#define CA_PARENTHESES					0xfffb		// ]
#define TWO_POINTS						0xfffa		// :
#define O_BRACE							0xfff9		// {
#define C_BRACE							0xfff8		// }
#define STRING							0xfff7		// "abc"
#define NEW_DECLARATION_COMMA			0xfff6		// ,
#define	LOGICAL_AND_STEP0				0xfff5		// &&
#define	LOGICAL_AND_STEP1				0xfff4		// &&
#define	LOGICAL_OR_STEP0				0xfff3		// ||
#define	LOGICAL_OR_STEP1				0xfff2		// ||
#define CONDITIONAL_STEP0				0xfff1		// ?:
#define CONDITIONAL_STEP1				0xfff0		// ?:
#define CONDITIONAL_STEP2				0xffef		// ?:

// group 0 (left to right)
#define	ARRAY_SUBSCRIPT					0x0000		// [ ]
#define	FUNCTION_CALL					0x0001		// ( )
#define	MEMBER_SELECTION_OBJECT			0x0002		// .
#define	MEMBER_SELECTION_POINTER		0x0003		// ->
#define	POSTFIX_INCREMENT				0x0004		// ++
#define	POSTFIX_DECREMENT				0x0005		// --

// group 1 (right to left)
#define	PREFIX_INCREMENT				0x0100		// ++
#define	PREFIX_DECREMENT				0x0101		// --
#define	SIZE_OF_TYPE					0x0102		// SIZEOF ( )
#define	ADDRESS_OF						0x0103		// &
#define	DEREFERENCE						0x0104		// *
#define	ARITHMETIC_NEGATION_UNARY		0x0105		// -
#define UNARY_PLUS						0x0106		// +
#define	BITWISE_COMPLEMENT				0x0107		// ~
#define	LOGICAL_NOT						0x0108		// !
#define	TYPE_CAST						0x0109		// (TYPE)

// group 2 (left to right)
#define	MULTIPLICATION					0x0200		// *
#define	DIVISION						0x0201		// /
#define	REMAINDER						0x0202		// %

// group 3 (left to right)
#define	ADDITION						0x0300		// +
#define	SUBTRACTION						0x0301		// -

// group 4 (left to right)
#define	LEFT_SHIFT						0x0400		// <<
#define	RIGHT_SHIFT						0x0401		// >>

// group 5 (left to right)
#define	LESS_THAN						0x0500		// <
#define	GREATER_THAN					0x0501		// >
#define	LESS_THAN_OR_EQUAL_TO			0x0502		// <=
#define	GREATER_THAN_OR_EQUAL_TO		0x0503		// >=

// group 6 (left to right)
#define	EQUALITY						0x0600		// ==
#define	INEQUALITY						0x0601		// !=

// group 7 (left to right)
#define	BITWISE_AND						0x0700		// &

// group 8 (left to right)
#define	BITWISE_EXCLUSIVE_OR			0x0800		// ^

// group 9 (left to right)
#define	BITWISE_OR						0x0900		// |

// group 10 (left to right)
#define	LOGICAL_AND						0x0a00		// &&

// group 11 (left to right)
#define	LOGICAL_OR						0x0b00		// ||

// group 12 (right to left)
#define	CONDITIONAL						0x0c00		// E1?E2:E3

// group 13 (right to left)
#define	ASSIGNMENT						0x0d00		// =
#define	MULTIPLICATION_ASSIGNMENT		0x0d01		// *=
#define	DIVISION_ASSIGNMENT				0x0d02		// /=
#define	MODULUS_ASSIGNMENT				0x0d03		// %=
#define	ADDITION_ASSIGNMENT				0x0d04		// +=
#define	SUBTRACTION_ASSIGNMENT			0x0d05		// -=
#define	LEFT_SHIFT_ASSIGNMENT			0x0d06		// <<=
#define	RIGHT_SHIFT_ASSIGNMENT			0x0d07		// >>=
#define	BITWISE_AND_ASSIGNMENT			0x0d08		// &=
#define	BITWISE_INCLUSIVE_OR_ASSIGNMENT	0x0d09		// |=
#define	BITWISE_EXCLUSIVE_OR_ASSIGNMENT	0x0d0a		// ^=

// group 14 (left to right)
#define	COMMA							0x0e00		// ,

//=====================
//c basic types
//
//definizioni di tutti i tipi di dati fondamentali
//=====================
//basic (i numeri hanno significato per i type-cast)
#define VOID_TYPE			0x0000
#define CHAR				0x0100
#define UNSIGNED_CHAR		0x0101
#define SHORT				0x0200
#define UNSIGNED_SHORT		0x0201
#define INT					0x0300
#define UNSIGNED_INT		0x0301
#define FLOAT				0x0400
#define DOUBLE				0x0500
#define LONG_DOUBLE			0x0501
//non-basic
#define STRUCTURE_TYPE		0x0600

//=====================
//c identifiers
//
//definizioni di tutti i tipi di identificatori
//=====================
#define VARIABLE			0x010000
#define MACRO				0x020000
#define	STRUCT				0x030000
#define STRUCT_FIELD		0x040000
#define FUNCTION			0x050000
#define FUNCTION_PARAMETER	0x060000
#define LABEL				0x070000
#define UNUSED				0x080000

//=======================
//c keywords
//
//definizioni di tutte le parole chiave
//=======================
#define KW_CHAR				0x00
#define KW_INT				0x01
#define KW_SHORT			0x02
#define KW_LONG				0x03
#define KW_UNSIGNED			0x04
#define KW_SIGNED			0x05
#define KW_FLOAT			0x06
#define KW_DOUBLE			0x07
#define KW_VOID				0x08
#define KW_STATIC			0x09
#define	KW_CONST			0x0a

//======================
//reswords
//
//Lista di parole riservate che non possono essere utilizzate come identificatori
//======================
struct {
	char	*ptr;
	int		len;
} reswords[]={		
{	{"sizeof"},			6},
{	{"NULL"},			4},
{	{"TRUE"},			4},
{	{"FALSE"},			5},
{	{"null"},			4},
{	{"true"},			4},
{	{"false"},			5},
{	{"break"},			5},
{	{"goto"},			4},
{	{"continue"},		8},
{	{"return"},			6},
{	{"struct"},			6},
{	{"do"},				2},
{	{"for"},			3},
{	{"while"},			5},
{	{"if"},				2},
{	{"else"},			4},
{NULL,0}
};

//=====================
//pseudo-instructions (psi)
//
//definizioni di tutte le pseudo istruzioni
//=====================
typedef unsigned char	byte;
/*------------------------------------------------------------------------*/
#define LOAD_SIGNED_CHAR_IN_EAX										0x0000
	// movsx	eax,byte ptr [signed char]
byte psi_0000[]={0x0F,0xBE,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi0000={7,psi_0000,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_SIGNED_CHAR_IN_ECX										0x0001
	// movsx	ecx,byte ptr [signed char]
byte psi_0001[]={0x0F,0xBE,0x0D,0x00,0x00,0x00,0x00}; // address
psi_t psi0001={7,psi_0001,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_UNSIGNED_CHAR_IN_EAX									0x0002
	// mov		eax,dword ptr [unsigned char]
	// and		eax,0FFh
byte psi_0002[]={0xA1,0x00,0x00,0x00,0x00, // address
	0x25,0xFF,0x00,0x00,0x00};
psi_t psi0002={10,psi_0002,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_UNSIGNED_CHAR_IN_ECX									0x0003
	// mov		ecx,dword ptr [unsigned char]
	// and		ecx,0FFh
byte psi_0003[]={0x8B,0x0D,0x00,0x00,0x00,0x00, // address
	0x81,0xE1,0xFF,0x00,0x00,0x00};
psi_t psi0003={12,psi_0003,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_SIGNED_SHORT_IN_EAX									0x0004
	// movsx	eax,word ptr [signed short]
byte psi_0004[]={0x0F,0xBF,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi0004={7,psi_0004,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_SIGNED_SHORT_IN_ECX									0x0005
	// movsx	ecx,word ptr [signed short]
byte psi_0005[]={0x0F,0xBF,0x0D,0x00,0x00,0x00,0x00}; // address
psi_t psi0005={7,psi_0005,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_UNSIGNED_SHORT_IN_EAX									0x0006
	// mov		eax,dword ptr [unsigned short]
	// and		eax,0FFFFh
byte psi_0006[]={0xA1,0x00,0x00,0x00,0x00, // address
	0x25,0xFF,0xFF,0x00,0x00};
psi_t psi0006={10,psi_0006,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_UNSIGNED_SHORT_IN_ECX									0x0007
	// mov		ecx,dword ptr [unsigned short]
	// and		ecx,0FFFFh
byte psi_0007[]={0x8B,0x0D,0x00,0x00,0x00,0x00, // address
	0x81,0xE1,0xFF,0xFF,0x00,0x00};
psi_t psi0007={12,psi_0007,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_INT_IN_EAX												0x0008
	// mov		eax,dword ptr [int]
byte psi_0008[]={0xA1,0x00,0x00,0x00,0x00}; // address
psi_t psi0008={5,psi_0008,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_INT_IN_ECX												0x0009
	// mov		ecx,dword ptr [int]
byte psi_0009[]={0x8B,0x0D,0x00,0x00,0x00,0x00}; // address
psi_t psi0009={6,psi_0009,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_FLOAT_IN_ST0											0x000a
	// fld		dword ptr [float]
byte psi_000a[]={0xD9,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi000a={6,psi_000a,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_DOUBLE_IN_ST0											0x000b
	// fld      qword ptr [double]
byte psi_000b[]={0xDD,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi000b={6,psi_000b,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_SIGNED_EAX_IN_ST0										0x000c
	// mov		dword ptr [tmp_word_0],eax
	// fild		dword ptr [tmp_word_0]
byte psi_000c[]={0xA3,0x00,0x00,0x00,0x00, // address
	0xDB,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi000c={11,psi_000c,0,{-1},{1,7,-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_UNSIGNED_EAX_IN_ST0									0x000d
// in eax deve esserci un intero a 32 bit senza segno
	// mov		dword ptr [tmp_qword_0],eax
	// mov		dword ptr [tmp_qword_0+4],0
	// fild		qword ptr [tmp_qword_0]
byte psi_000d[]={0xA3,0x00,0x00,0x00,0x00, // address
	0xC7,0x05,0x04,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xDF,0x2D,0x00,0x00,0x00,0x00}; // address
psi_t psi000d={21,psi_000d,0,{-1},{1,7,17,-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ST0_IN_EAX												0x000e
	// wait
	// fnstcw	word ptr [tmp_word_0]
	// wait
	// mov		ax,word ptr [tmp_word_0]
	// or		ah,0Ch
	// mov		word ptr [tmp_word_1],ax
	// fldcw	word ptr [tmp_word_1]
	// fistp	dword ptr [tmp_dword_2]
	// fldcw	word ptr [tmp_word_0]
	// mov		eax,dword ptr [tmp_dword_2]
byte psi_000e[]={0x9B,0xD9,0x3D,0x00,0x00,0x00,0x00, // address
	0x9B,0x66,0xA1,0x00,0x00,0x00,0x00, // address
	0x80,0xCC,0x0C,0x66,0xA3,0x02,0x00,0x00,0x00, // address
	0xD9,0x2D,0x02,0x00,0x00,0x00, // address
	0xDB,0x1D,0x04,0x00,0x00,0x00, // address
	0xD9,0x2D,0x00,0x00,0x00,0x00, // address
	0xA1,0x04,0x00,0x00,0x00}; // address
psi_t psi000e={46,psi_000e,0,{-1},{3,10,19,25,31,37,42,-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_CHAR											0x000f
	// mov		byte ptr [char],al
byte psi_000f[]={0xA2,0x00,0x00,0x00,0x00}; // address
psi_t psi000f={5,psi_000f,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_SHORT											0x0010
	// mov		word ptr [short],ax
byte psi_0010[]={0x66,0xA3,0x00,0x00,0x00,0x00}; // address
psi_t psi0010={6,psi_0010,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_INT											0x0011
	// mov		dword ptr [int],eax
byte psi_0011[]={0xA3,0x00,0x00,0x00,0x00}; // address
psi_t psi0011={5,psi_0011,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_FLOAT											0x0012
	// fstp		dword ptr [float]
byte psi_0012[]={0xD9,0x1D,0x00,0x00,0x00,0x00}; // address
psi_t psi0012={6,psi_0012,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_DOUBLE											0x0013
	// fstp		qword ptr [double]
byte psi_0013[]={0xDD,0x1D,0x00,0x00,0x00,0x00}; // address
psi_t psi0013={6,psi_0013,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_TO_UNSIGNED_8_BIT_VALUE									0x0014
	// and		eax,0FFh
byte psi_0014[]={0x25,0xFF,0x00,0x00,0x00};
psi_t psi0014={5,psi_0014,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_TO_SIGNED_8_BIT_VALUE									0x0015
	// movsx	eax,al
byte psi_0015[]={0x0F,0xBE,0xC0};
psi_t psi0015={3,psi_0015,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_TO_UNSIGNED_16_BIT_VALUE								0x0016
	// and		eax,0FFFFh
byte psi_0016[]={0x25,0xFF,0xFF,0x00,0x00};
psi_t psi0016={5,psi_0016,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_TO_SIGNED_16_BIT_VALUE									0x0017
	// movsx	eax,ax
byte psi_0017[]={0x0F,0xBF,0xC0};
psi_t psi0017={3,psi_0017,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ARRAY_FIRST_DIMENSION										0x0018
	// imul		eax,eax,(max2*max3*...*maxn*sizeof_type)
	// lea		ecx,dword ptr (address)[eax]
byte psi_0018[]={0x69,0xC0,0x00,0x00,0x00,0x00, // (max2*max3*...*maxn*sizeof_type)
	0x8D,0x88,0x00,0x00,0x00,0x00}; // address
psi_t psi0018={12,psi_0018,4,{8,-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define ARRAY_X_DIMENSION											0x0019
	// imul		eax,eax,(max(x+1)*max(x+2)*...*maxn*sizeof_type)
	// add		ecx,eax
byte psi_0019[]={0x69,0xC0,0x00,0x00,0x00,0x00, // (max(x+1)*max(x+2)*...*maxn*sizeof_type)
	0x03,0xC8};
psi_t psi0019={8,psi_0019,4,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define CALL_FUNCTION												0x001a
	// call		(function)
	// add		esp,constant
byte psi_001a[]={0xE8,0x00,0x00,0x00,0x00, // (dest-instr-5)
	0x81,0xC4,0x00,0x00,0x00,0x00}; // constant
psi_t psi001a={11,psi_001a,4,{1,-1},{-1},{7,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ARITHMETIC_NEGATION										0x001b
	// neg		eax
byte psi_001b[]={0xF7,0xD8};
psi_t psi001b={2,psi_001b,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_ARITHMETIC_NEGATION										0x001c
	// fchs
byte psi_001c[]={0xD9,0xE0};
psi_t psi001c={2,psi_001c,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_BITWISE_COMPLEMENT										0x001d
	// not		eax
byte psi_001d[]={0xF7,0xD0};
psi_t psi001d={2,psi_001d,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_LOGICAL_NOT												0x001e
	// neg		eax
	// sbb		eax,eax
	// inc		eax
byte psi_001e[]={0xF7,0xD8,0x1B,0xC0,0x40};
psi_t psi001e={5,psi_001e,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_LOGICAL_NOT												0x001f
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_001f[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi001f={32,psi_001f,0,{-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_MULTIPLICATION										0x0020
	// imul		eax,ecx
byte psi_0020[]={0x0F,0xAF,0xC1};
psi_t psi0020={3,psi_0020,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_MULTIPLICATION									0x0021
	// fmul		dword ptr [float]
byte psi_0021[]={0xD8,0x0D,0x00,0x00,0x00,0x00}; // address
psi_t psi0021={6,psi_0021,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_MULTIPLICATION									0x0022
	// fmul		qword ptr [double]
byte psi_0022[]={0xDC,0x0D,0x00,0x00,0x00,0x00}; // address
psi_t psi0022={6,psi_0022,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_DIVISION											0x0023
	// cwd
	// idiv		ecx
byte psi_0023[]={0x66,0x99,0xF7,0xF9};
psi_t psi0023={4,psi_0023,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define UNSIGNED_EAX_ECX_DIVISION									0x0024
// in eax deve esserci un intero a 32 bit senza segno
	// xor		edx,edx
	// div		ecx
byte psi_0024[]={0x33,0xD2,0xF7,0xF1};
psi_t psi0024={4,psi_0024,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_DIVISION											0x0025
	// fdiv		dword ptr [float]
byte psi_0025[]={0xD8,0x35,0x00,0x00,0x00,0x00}; // address
psi_t psi0025={6,psi_0025,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_DIVISION											0x0026
	// fdiv		qword ptr [double]
byte psi_0026[]={0xDC,0x35,0x00,0x00,0x00,0x00}; // address
psi_t psi0026={6,psi_0026,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_DIVISION									0x0027
	// fdiv		dword ptr [edx=float]
byte psi_0027[]={0xD8,0x32};
psi_t psi0027={2,psi_0027,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_DIVISION									0x0028
	// fdiv		qword ptr [edx=double]
byte psi_0028[]={0xDC,0x32};
psi_t psi0028={2,psi_0028,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_REMAINDER											0x0029
	// cwd
	// idiv		ecx
	// mov		eax,edx
byte psi_0029[]={0x66,0x99,0xF7,0xF9,0xF7,0xF9};
psi_t psi0029={6,psi_0029,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define UNSIGNED_EAX_ECX_REMAINDER									0x002a
// in eax deve esserci un intero a 32 bit senza segno
	// xor		edx,edx
	// div		ecx
	// mov		eax,edx
byte psi_002a[]={0x33,0xD2,0xF7,0xF1,0x8B,0xC2};
psi_t psi002a={6,psi_002a,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_ADDITION											0x002b
	// add		eax,ecx
byte psi_002b[]={0x03,0xC1};
psi_t psi002b={2,psi_002b,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_ADDITION											0x002c
	// fadd		dword ptr [float]
byte psi_002c[]={0xD8,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi002c={6,psi_002c,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_ADDITION											0x002d
	// fadd		qword ptr [double]
byte psi_002d[]={0xDC,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi002d={6,psi_002d,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_SUBTRACTION											0x002e
	// sub		eax,ecx
byte psi_002e[]={0x2B,0xC1};
psi_t psi002e={2,psi_002e,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_SUBTRACTION										0x002f
	// fsub		dword ptr [float]
byte psi_002f[]={0xD8,0x25,0x00,0x00,0x00,0x00}; // address
psi_t psi002f={6,psi_002f,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_SUBTRACTION										0x0030
	// fsub		qword ptr [double]
byte psi_0030[]={0xDC,0x25,0x00,0x00,0x00,0x00}; // address
psi_t psi0030={6,psi_0030,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_SUBTRACTION								0x0031
	// fsub		dword ptr [edx=float]
byte psi_0031[]={0xD8,0x22};
psi_t psi0031={2,psi_0031,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_SUBTRACTION								0x0032
	// fsub		qword ptr [edx=double]
byte psi_0032[]={0xDC,0x22};
psi_t psi0032={2,psi_0032,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_LEFT_SHIFT											0x0033
	// shl		eax,cl
byte psi_0033[]={0xD3,0xE0};
psi_t psi0033={2,psi_0033,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_RIGHT_SHIFT											0x0034
	// sar		eax,cl
byte psi_0034[]={0xD3,0xF8};
psi_t psi0034={2,psi_0034,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define UNSIGNED_EAX_ECX_RIGHT_SHIFT								0x0035
// in eax deve esserci un intero a 32 bit senza segno
	// shr		eax,cl
byte psi_0035[]={0xD3,0xE8};
psi_t psi0035={2,psi_0035,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_LEFT_SHIFT										0x0036
	// shl		eax,constant
byte psi_0036[]={0xC1,0xE0,0x00}; // constant (1 byte)
psi_t psi0036={3,psi_0036,1,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_RIGHT_SHIFT									0x0037
	// sar		eax,constant
byte psi_0037[]={0xC1,0xF8,0x00}; // constant (1 byte)
psi_t psi0037={3,psi_0037,1,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define UNSIGNED_EAX_CONSTANT_RIGHT_SHIFT							0x0038
// in eax deve esserci un intero a 32 bit senza segno
	// shr		eax,constant
byte psi_0038[]={0xC1,0xE8,0x00}; // constant (1 byte)
psi_t psi0038={3,psi_0038,1,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_LESS_THAN											0x0039
	// xor		edx,edx
	// cmp		eax,ecx
	// setl		dl
	// mov		eax,edx
byte psi_0039[]={0x33,0xD2,0x3B,0xC1,0x0F,0x9C,0xC2,0x8B,0xC2};
psi_t psi0039={9,psi_0039,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_UNSIGNED_LESS_THAN									0x003a
// è sufficiente che o eax o ecx siano interi a 32 bit senza segno
	// cmp		eax,ecx
	// sbb		edx,edx
	// neg		edx
	// mov		eax,edx
byte psi_003a[]={0x3B,0xC1,0x1B,0xD2,0xF7,0xDA,0x8B,0xC2};
psi_t psi003a={8,psi_003a,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_GREATER_THAN										0x003b
	// xor		edx,edx
	// cmp		eax,ecx
	// setg		dl
	// mov		eax,edx
byte psi_003b[]={0x33,0xD2,0x3B,0xC1,0x0F,0x9F,0xC2,0x8B,0xC2};
psi_t psi003b={9,psi_003b,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_UNSIGNED_GREATER_THAN								0x003c
// è sufficiente che o eax o ecx siano interi a 32 bit senza segno
	// cmp		ecx,eax
	// sbb		edx,edx
	// neg		edx
	// mov		eax,edx
byte psi_003c[]={0x3B,0xC8,0x1B,0xD2,0xF7,0xDA,0x8B,0xC2};
psi_t psi003c={8,psi_003c,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_LESS_THAN_OR_EQUAL_TO								0x003d
	// xor		edx,edx
	// cmp		eax,ecx
	// setle	dl
	// mov		eax,edx
byte psi_003d[]={0x33,0xD2,0x3B,0xC1,0x0F,0x9E,0xC2,0x8B,0xC2};
psi_t psi003d={9,psi_003d,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_UNSIGNED_LESS_THAN_OR_EQUAL_TO						0x003e
// è sufficiente che o eax o ecx siano interi a 32 bit senza segno
	// cmp		ecx,eax
	// sbb		edx,edx
	// inc		edx
	// mov		eax,edx
byte psi_003e[]={0x3B,0xC8,0x1B,0xD2,0x42,0x8B,0xC2};
psi_t psi003e={7,psi_003e,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_GREATER_THAN_OR_EQUAL_TO							0x003f
	// xor		edx,edx
	// cmp		eax,ecx
	// setge	dl
	// mov		eax,edx
byte psi_003f[]={0x33,0xD2,0x3B,0xC1,0x0F,0x9D,0xC2,0x8B,0xC2};
psi_t psi003f={9,psi_003f,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_UNSIGNED_GREATER_THAN_OR_EQUAL_TO					0x0040
// è sufficiente che o eax o ecx siano interi a 32 bit senza segno
	// cmp		eax,ecx
	// sbb		edx,edx
	// inc		edx
	// mov		eax,edx
byte psi_0040[]={0x3B,0xC1,0x1B,0xD2,0x42,0x8B,0xC2};
psi_t psi0040={7,psi_0040,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_EQUALITY											0x0041
	// xor		edx,edx
	// cmp		eax,ecx
	// sete		dl
	// mov		eax,edx
byte psi_0041[]={0x33,0xD2,0x3B,0xC1,0x0F,0x94,0xC2,0x8B,0xC2};
psi_t psi0041={9,psi_0041,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_INEQUALITY											0x0042
	// xor		edx,edx
	// cmp		eax,ecx
	// setne	dl
	// mov		eax,edx
byte psi_0042[]={0x33,0xD2,0x3B,0xC1,0x0F,0x95,0xC2,0x8B,0xC2};
psi_t psi0042={9,psi_0042,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_LESS_THAN											0x0043
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0043[]={0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0043={22,psi_0043,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_LESS_THAN										0x0044
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0044[]={0xDC,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0044={22,psi_0044,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_LESS_THAN									0x0045
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0045[]={0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0045={18,psi_0045,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_LESS_THAN									0x0046
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0046[]={0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0046={18,psi_0046,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_GREATER_THAN										0x0047
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0047[]={0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0047={22,psi_0047,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_GREATER_THAN										0x0048
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0048[]={0xDC,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0048={22,psi_0048,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_GREATER_THAN								0x0049
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0049[]={0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0049={18,psi_0049,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_GREATER_THAN								0x004a
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_004a[]={0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi004a={18,psi_004a,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_EQUALITY											0x004b
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_004b[]={0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi004b={22,psi_004b,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_EQUALITY											0x004c
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_004c[]={0xDC,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi004c={22,psi_004c,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_INEQUALITY										0x004d
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_004d[]={0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi004d={22,psi_004d,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_INEQUALITY										0x004e
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_004e[]={0xDC,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi004e={22,psi_004e,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_LESS_THAN_OR_EQUAL_TO								0x004f
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_004f[]={0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi004f={22,psi_004f,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_LESS_THAN_OR_EQUAL_TO							0x0050
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0050[]={0xDC,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0050={22,psi_0050,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_LESS_THAN_OR_EQUAL_TO						0x0051
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0051[]={0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0051={18,psi_0051,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_LESS_THAN_OR_EQUAL_TO						0x0052
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0052[]={0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0052={18,psi_0052,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_GREATER_THAN_OR_EQUAL_TO							0x0053
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0053[]={0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0053={22,psi_0053,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_GREATER_THAN_OR_EQUAL_TO							0x0054
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0054[]={0xDC,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0054={22,psi_0054,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_GREATER_THAN_OR_EQUAL_TO					0x0055
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0055[]={0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0055={18,psi_0055,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_GREATER_THAN_OR_EQUAL_TO					0x0056
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0056[]={0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0056={18,psi_0056,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_BITWISE_AND											0x0057
	// and		eax,ecx
byte psi_0057[]={0x23,0xC1};
psi_t psi0057={2,psi_0057,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_BITWISE_EXCLUSIVE_OR								0x0058
	// xor		eax,ecx
byte psi_0058[]={0x33,0xC1};
psi_t psi0058={2,psi_0058,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_BITWISE_OR											0x0059
	// or		eax,ecx
byte psi_0059[]={0x0B,0xC1};
psi_t psi0059={2,psi_0059,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_LOGICAL_AND											0x005a
	// test		eax,eax
	// je		jump_0
	// test		ecx,ecx
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_005a[]={0x85,0xC0,0x74,0x0B,0x85,0xC9,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi005a={17,psi_005a,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_LOGICAL_OR											0x005b
	// test		eax,eax
	// jne		jump_0
	// test		ecx,ecx
	// jne		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_005b[]={0x85,0xC0,0x75,0x08,0x85,0xC9,0x75,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi005b={17,psi_005b,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_LOGICAL_AND										0x005c
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// fldz
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_005c[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x16,0xD9,0xEE,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi005c={47,psi_005c,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_LOGICAL_AND										0x005d
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// fldz
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_005d[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x16,0xD9,0xEE,0xDC,0x1D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi005d={47,psi_005d,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_FLOAT_LOGICAL_OR										0x005e
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// fldz
	// fcomp	dword ptr [float]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_005e[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x13,0xD9,0xEE,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi005e={47,psi_005e,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_DOUBLE_LOGICAL_OR										0x005f
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// fldz
	// fcomp	qword ptr [double]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_005f[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x13,0xD9,0xEE,0xDC,0x1D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi005f={47,psi_005f,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define JUMP_IF_EAX_NOT_ZERO										0x0060
	// test		eax,eax
	// jne		(address)
byte psi_0060[]={0x85,0xC0,0x0F,0x85,0x00,0x00,0x00,0x00}; // address
psi_t psi0060={8,psi_0060,0,{4,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define JUMP_IF_ST0_NOT_ZERO										0x0061
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		(address)
byte psi_0061[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x0F,0x84,0x00,0x00,0x00,0x00}; // address 1
psi_t psi0061={27,psi_0061,0,{23,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define JUMP_IF_EAX_ZERO											0x0062
	// test		eax,eax
	// je		(address)
byte psi_0062[]={0x85,0xC0,0x0F,0x84,0x00,0x00,0x00,0x00}; // address
psi_t psi0062={8,psi_0062,0,{4,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define JUMP_IF_ST0_ZERO											0x0063
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		(address)
byte psi_0063[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x0F,0x85,0x00,0x00,0x00,0x00}; // address 1
psi_t psi0063={27,psi_0063,0,{23,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define JUMP														0x0064
	// jmp		(address)
byte psi_0064[]={0xE9,0x00,0x00,0x00,0x00}; // (dest-instr-5)
psi_t psi0064={5,psi_0064,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_SIGNED_CHAR_IN_EAX								0x0065
	// movsx	eax,byte ptr [ebp-x=signed char]
byte psi_0065[]={0x0F,0xBE,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi0065={7,psi_0065,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_SIGNED_CHAR_IN_ECX								0x0066
	// movsx	ecx,byte ptr [ebp-x=signed char]
byte psi_0066[]={0x0F,0xBE,0x8D,0x00,0x00,0x00,0x00}; // offset
psi_t psi0066={7,psi_0066,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_UNSIGNED_CHAR_IN_EAX								0x0067
	// mov		eax,dword ptr [ebp-x=unsigned char]
	// and		eax,0FFh
byte psi_0067[]={0x8B,0x85,0x00,0x00,0x00,0x00, // offset
	0x25,0xFF,0x00,0x00,0x00};
psi_t psi0067={11,psi_0067,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_UNSIGNED_CHAR_IN_ECX								0x0068
	// mov		ecx,dword ptr [ebp-x=unsigned char]
	// and		ecx,0FFh
byte psi_0068[]={0x8B,0x8D,0x00,0x00,0x00,0x00, // offset
	0x81,0xE1,0xFF,0x00,0x00,0x00};
psi_t psi0068={12,psi_0068,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_SIGNED_SHORT_IN_EAX								0x0069
	// movsx	eax,word ptr [ebp-x=signed short]
byte psi_0069[]={0x0F,0xBF,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi0069={7,psi_0069,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_SIGNED_SHORT_IN_ECX								0x006a
	// movsx	ecx,word ptr [ebp-x=signed short]
byte psi_006a[]={0x0F,0xBF,0x8D,0x00,0x00,0x00,0x00}; // offset
psi_t psi006a={7,psi_006a,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_UNSIGNED_SHORT_IN_EAX							0x006b
	// mov		eax,dword ptr [ebp-x=unsigned short]
	// and		eax,0FFFFh
byte psi_006b[]={0x8B,0x85,0x00,0x00,0x00,0x00, // offset
	0x25,0xFF,0xFF,0x00,0x00};
psi_t psi006b={11,psi_006b,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_UNSIGNED_SHORT_IN_ECX							0x006c
	// mov		ecx,dword ptr [ebp-x=unsigned short]
	// and		ecx,0FFFFh
byte psi_006c[]={0x8B,0x8D,0x00,0x00,0x00,0x00, // offset
	0x81,0xE1,0xFF,0xFF,0x00,0x00};
psi_t psi006c={12,psi_006c,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_INT_IN_EAX										0x006d
	// mov		eax,dword ptr [ebp-x=int]
byte psi_006d[]={0x8B,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi006d={6,psi_006d,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_INT_IN_ECX										0x006e
	// mov		ecx,dword ptr [ebp-x=int]
byte psi_006e[]={0x8B,0x8D,0x00,0x00,0x00,0x00}; // offset
psi_t psi006e={6,psi_006e,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_FLOAT_IN_ST0										0x006f
	// fld		dword ptr [ebp-x=float]
byte psi_006f[]={0xD9,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi006f={6,psi_006f,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_DOUBLE_IN_ST0									0x0070
	// fld      qword ptr [ebp-x=double]
byte psi_0070[]={0xDD,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi0070={6,psi_0070,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_STACK_CHAR										0x0071
	// mov		byte ptr [ebp-x=char],al
byte psi_0071[]={0x88,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi0071={6,psi_0071,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_STACK_SHORT									0x0072
	// mov		word ptr [ebp-x=short],ax
byte psi_0072[]={0x66,0x89,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi0072={7,psi_0072,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_STACK_INT										0x0073
	// mov		dword ptr [ebp-x=int],eax
byte psi_0073[]={0x89,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi0073={6,psi_0073,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_STACK_FLOAT									0x0074
	// fstp		dword ptr [ebp-x=float]
byte psi_0074[]={0xD9,0x9D,0x00,0x00,0x00,0x00}; // offset
psi_t psi0074={6,psi_0074,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_STACK_DOUBLE									0x0075
	// fstp		qword ptr [ebp-x=double]
byte psi_0075[]={0xDD,0x9D,0x00,0x00,0x00,0x00}; // offset
psi_t psi0075={6,psi_0075,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_ARRAY_FIRST_DIMENSION									0x0076
	// imul		eax,eax,(max2*max3*...*maxn*sizeof_type)
	// lea		ecx,dword ptr (ebp-x=address)[eax]
byte psi_0076[]={0x69,0xC0,0x00,0x00,0x00,0x00, // (max2*max3*...*maxn*sizeof_type) (4 bytes)
	0x8D,0x8C,0x05,0x00,0x00,0x00,0x00}; // offset
psi_t psi0076={13,psi_0076,4,{9,-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_MULTIPLICATION								0x0077
	// fmul		dword ptr [ebp-x=float]
byte psi_0077[]={0xD8,0x8D,0x00,0x00,0x00,0x00}; // offset
psi_t psi0077={6,psi_0077,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_MULTIPLICATION								0x0078
	// fmul		qword ptr [ebp-x=double]
byte psi_0078[]={0xDC,0x8D,0x00,0x00,0x00,0x00}; // offset
psi_t psi0078={6,psi_0078,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_DIVISION									0x0079
	// fdiv		dword ptr [ebp-x=float]
byte psi_0079[]={0xD8,0xB5,0x00,0x00,0x00,0x00}; // offset
psi_t psi0079={6,psi_0079,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_DIVISION									0x007a
	// fdiv		qword ptr [ebp-x=double]
byte psi_007a[]={0xDC,0xB5,0x00,0x00,0x00,0x00}; // offset
psi_t psi007a={6,psi_007a,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_FLOAT_ST0_DIVISION									0x007b
	// fdivr	dword ptr [ebp-x=float]
byte psi_007b[]={0xD8,0xBD,0x00,0x00,0x00,0x00}; // offset
psi_t psi007b={6,psi_007b,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_DOUBLE_ST0_DIVISION									0x007c
	// fdivr	qword ptr [ebp-x=double]
byte psi_007c[]={0xDC,0xBD,0x00,0x00,0x00,0x00}; // offset
psi_t psi007c={6,psi_007c,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_ADDITION									0x007d
	// fadd		dword ptr [ebp-x=float]
byte psi_007d[]={0xD8,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi007d={6,psi_007d,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_ADDITION									0x007e
	// fadd		qword ptr [ebp-x=double]
byte psi_007e[]={0xDC,0x85,0x00,0x00,0x00,0x00}; // offset
psi_t psi007e={6,psi_007e,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_SUBTRACTION									0x007f
	// fsub		dword ptr [ebp-x=float]
byte psi_007f[]={0xD8,0xA5,0x00,0x00,0x00,0x00}; // offset
psi_t psi007f={6,psi_007f,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_SUBTRACTION								0x0080
	// fsub		qword ptr [ebp-x=double]
byte psi_0080[]={0xDC,0xA5,0x00,0x00,0x00,0x00}; // offset
psi_t psi0080={6,psi_0080,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_FLOAT_ST0_SUBTRACTION									0x0081
	// fsubr	dword ptr [ebp-x=float]
byte psi_0081[]={0xD8,0xAD,0x00,0x00,0x00,0x00}; // offset
psi_t psi0081={6,psi_0081,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_DOUBLE_ST0_SUBTRACTION								0x0082
	// fsubr	qword ptr [ebp-x=double]
byte psi_0082[]={0xDC,0xAD,0x00,0x00,0x00,0x00}; // offset
psi_t psi0082={6,psi_0082,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_LESS_THAN									0x0083
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0083[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0083={22,psi_0083,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_LESS_THAN									0x0084
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0084[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0084={22,psi_0084,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_FLOAT_ST0_LESS_THAN									0x0085
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0085[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0085={22,psi_0085,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_DOUBLE_ST0_LESS_THAN									0x0086
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0086[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0086={22,psi_0086,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_GREATER_THAN								0x0087
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0087[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0087={22,psi_0087,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_GREATER_THAN								0x0088
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,41h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0088[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0088={22,psi_0088,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_FLOAT_ST0_GREATER_THAN								0x0089
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0089[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0089={22,psi_0089,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_DOUBLE_ST0_GREATER_THAN								0x008a
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,1
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_008a[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi008a={22,psi_008a,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_EQUALITY									0x008b
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_008b[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi008b={22,psi_008b,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_EQUALITY									0x008c
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_008c[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi008c={22,psi_008c,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_INEQUALITY									0x008d
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_008d[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi008d={22,psi_008d,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_INEQUALITY									0x008e
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_008e[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi008e={22,psi_008e,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_LESS_THAN_OR_EQUAL_TO						0x008f
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_008f[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi008f={22,psi_008f,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_LESS_THAN_OR_EQUAL_TO						0x0090
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0090[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0090={22,psi_0090,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_FLOAT_ST0_LESS_THAN_OR_EQUAL_TO						0x0091
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0091[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0091={22,psi_0091,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_DOUBLE_ST0_LESS_THAN_OR_EQUAL_TO						0x0092
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0092[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0092={22,psi_0092,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_GREATER_THAN_OR_EQUAL_TO					0x0093
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0093[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0093={22,psi_0093,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_GREATER_THAN_OR_EQUAL_TO					0x0094
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,1
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0094[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x01,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0094={22,psi_0094,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_FLOAT_ST0_GREATER_THAN_OR_EQUAL_TO					0x0095
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0095[]={0xD8,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0095={22,psi_0095,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_DOUBLE_ST0_GREATER_THAN_OR_EQUAL_TO					0x0096
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,41h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0096[]={0xDC,0x9D,0x00,0x00,0x00,0x00, // offset
	0xDF,0xE0,0xF6,0xC4,0x41,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0096={22,psi_0096,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_LOGICAL_AND									0x0097
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// fldz
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0097[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x16,0xD9,0xEE,0xD8,0x9D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0097={47,psi_0097,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_LOGICAL_AND								0x0098
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// fldz
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_0098[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x16,0xD9,0xEE,0xDC,0x9D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi0098={47,psi_0098,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_FLOAT_LOGICAL_OR									0x0099
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// fldz
	// fcomp	dword ptr [ebp-x=float]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_0099[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x13,0xD9,0xEE,0xD8,0x9D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi0099={47,psi_0099,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_STACK_DOUBLE_LOGICAL_OR									0x009a
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// fldz
	// fcomp	qword ptr [ebp-x=double]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_009a[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address 0
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address 0
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x13,0xD9,0xEE,0xDC,0x9D,0x00,0x00,0x00,0x00, // address 1
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi009a={47,psi_009a,0,{27,-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CONSTANT_IN_EAX										0x009b
	// mov		eax,constant
byte psi_009b[]={0xB8,0x00,0x00,0x00,0x00}; // constant
psi_t psi009b={5,psi_009b,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define STORE_ECX_IN_INT											0x009c
	// mov		dword ptr [int],ecx
byte psi_009c[]={0x89,0x0D,0x00,0x00,0x00,0x00}; // address
psi_t psi009c={6,psi_009c,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define PUSH_CONSTANT												0x009d
	// push		constant
byte psi_009d[]={0x68,0x00,0x00,0x00,0x00}; // constant
psi_t psi009d={5,psi_009d,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define PUSH_INT													0x009e
	// push		dword ptr [int]
byte psi_009e[]={0xFF,0x35,0x00,0x00,0x00,0x00}; // address
psi_t psi009e={6,psi_009e,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define PUSH_EAX													0x009f
	// push		eax
byte psi_009f[]={0x50};
psi_t psi009f={1,psi_009f,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define PUSH_STACK_INT												0x00a0
	// push		dword ptr [ebp-x=int]
byte psi_00a0[]={0xFF,0xB5,0x00,0x00,0x00,0x00}; // address
psi_t psi00a0={6,psi_00a0,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_PUSH_MEMORY_BLOCK									0x00a1
// input: eax=address, ecx=count (multiplo di 4)
	// mov		esi,eax
	// sub		esp,ecx
	// mov		edi,esp
	// shr		ecx,2
	// rep		movsd
byte psi_00a1[]={0x8B,0xF0,0x2B,0xE1,0x8B,0xFC,0xC1,0xE9,0x02,0xF3,0xA5};
psi_t psi00a1={11,psi_00a1,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define PUSH_CONSTANT_ADDRESS										0x00a2
	// push		(address)
byte psi_00a2[]={0x68,0x00,0x00,0x00,0x00}; // address
psi_t psi00a2={5,psi_00a2,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_INT_POINTER_IN_EAX								0x00a3
	// lea		eax,dword ptr [ebp-x=int]
byte psi_00a3[]={0x8D,0x85,0x00,0x00,0x00,0x00}; // address
psi_t psi00a3={6,psi_00a3,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CONSTANT_ADDRESS_IN_EAX								0x00a4
	// mov		eax,(address)
byte psi_00a4[]={0xB8,0x00,0x00,0x00,0x00}; // address
psi_t psi00a4={5,psi_00a4,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_INT_POINTER_IN_ECX								0x00a5
	// lea		ecx,dword ptr [ebp-x=int]
byte psi_00a5[]={0x8D,0x8D,0x00,0x00,0x00,0x00}; // address
psi_t psi00a5={6,psi_00a5,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CONSTANT_ADDRESS_IN_ECX								0x00a6
	// mov		ecx,(address)
byte psi_00a6[]={0xB9,0x00,0x00,0x00,0x00}; // address
psi_t psi00a6={5,psi_00a6,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_CHAR_IN_EAX										0x00a7
	// mov		al,byte ptr [ebp-x=char]
byte psi_00a7[]={0x8A,0x85,0x00,0x00,0x00,0x00}; // address
psi_t psi00a7={6,psi_00a7,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CHAR_IN_EAX											0x00a8
	// mov		al,byte ptr [char]
byte psi_00a8[]={0xA0,0x00,0x00,0x00,0x00}; // address
psi_t psi00a8={5,psi_00a8,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_SHORT_IN_EAX										0x00a9
	// mov		ax,word ptr [ebp-x=short]
byte psi_00a9[]={0x66,0x8B,0x85,0x00,0x00,0x00,0x00}; // address
psi_t psi00a9={7,psi_00a9,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_SHORT_IN_EAX											0x00aa
	// mov		ax,word ptr [short]
byte psi_00aa[]={0x66,0xA1,0x00,0x00,0x00,0x00}; // address
psi_t psi00aa={6,psi_00aa,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CONSTANT_IN_ECX										0x00ab
	// mov		ecx,constant
byte psi_00ab[]={0xB9,0x00,0x00,0x00,0x00}; // constant
psi_t psi00ab={5,psi_00ab,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_SIGNED_CHAR_IN_EAX								0x00ac
	// movsx	eax,byte ptr [ecx=signed char]
byte psi_00ac[]={0x0F,0xBE,0x01};
psi_t psi00ac={3,psi_00ac,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX							0x00ad
	// mov		eax,dword ptr [ecx=unsigned char]
	// and		eax,0FFh
byte psi_00ad[]={0x8B,0x01,0x25,0xFF,0x00,0x00,0x00};
psi_t psi00ad={7,psi_00ad,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_SIGNED_SHORT_IN_EAX								0x00ae
	// movsx	eax,word ptr [ecx=signed short]
byte psi_00ae[]={0x0F,0xBF,0x01};
psi_t psi00ae={3,psi_00ae,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX							0x00af
	// mov		eax,dword ptr [ecx=unsigned short]
	// and		eax,0FFFFh
byte psi_00af[]={0x8B,0x01,0x25,0xFF,0xFF,0x00,0x00};
psi_t psi00af={7,psi_00af,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_INT_IN_EAX										0x00b0
	// mov		eax,dword ptr [ecx=int]
byte psi_00b0[]={0x8B,0x01};
psi_t psi00b0={2,psi_00b0,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_FLOAT_IN_ST0									0x00b1
	// fld		dword ptr [ecx=float]
byte psi_00b1[]={0xD9,0x01};
psi_t psi00b1={2,psi_00b1,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_DOUBLE_IN_ST0									0x00b2
	// fld      qword ptr [ecx=double]
byte psi_00b2[]={0xDD,0x01};
psi_t psi00b2={2,psi_00b2,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_CHAR_IN_EAX										0x00b3
	// mov		al,byte ptr [ecx=char]
byte psi_00b3[]={0x8A,0x01};
psi_t psi00b3={2,psi_00b3,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_SHORT_IN_EAX									0x00b4
	// mov		ax,word ptr [ecx=short]
byte psi_00b4[]={0x66,0x8B,0x01};
psi_t psi00b4={3,psi_00b4,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ECX_CONSTANT_ADDITION										0x00b5
	// add		ecx,constant
byte psi_00b5[]={0x81,0xC1,0x00,0x00,0x00,0x00}; // constant
psi_t psi00b5={6,psi_00b5,4,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define LOAD_INT_IN_EDX												0x00b6
	// mov		edx,dword ptr [int]
byte psi_00b6[]={0x8B,0x15,0x00,0x00,0x00,0x00}; // address
psi_t psi00b6={6,psi_00b6,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_INT_IN_ECX										0x00b7
	// mov		ecx,dword ptr [edx=int]
byte psi_00b7[]={0x8B,0x0A};
psi_t psi00b7={2,psi_00b7,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_ADDITION										0x00b8
	// add		eax,constant
byte psi_00b8[]={0x05,0x00,0x00,0x00,0x00}; // constant
psi_t psi00b8={5,psi_00b8,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define STORE_CONSTANT_IN_MEM										0x00b9
	// mov		dword ptr [int],constant
byte psi_00b9[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00}; // constant
psi_t psi00b9={10,psi_00b9,4,{2,-1},{-1},{6,-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_ECXPTR_CHAR									0x00ba
	// mov		byte ptr [ecx=char],al
byte psi_00ba[]={0x88,0x01};
psi_t psi00ba={2,psi_00ba,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_ECXPTR_SHORT									0x00bb
	// mov		word ptr [ecx=short],ax
byte psi_00bb[]={0x66,0x89,0x01};
psi_t psi00bb={3,psi_00bb,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EAX_IN_ECXPTR_INT										0x00bc
	// mov		dword ptr [ecx=int],eax
byte psi_00bc[]={0x89,0x01};
psi_t psi00bc={2,psi_00bc,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_ECXPTR_FLOAT									0x00bd
	// fstp		dword ptr [ecx=float]
byte psi_00bd[]={0xD9,0x19};
psi_t psi00bd={2,psi_00bd,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_ECXPTR_DOUBLE									0x00be
	// fstp		qword ptr [ecx=double]
byte psi_00be[]={0xDD,0x19};
psi_t psi00be={2,psi_00be,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_SUBTRACTION									0x00bf
	// sub		eax,constant
byte psi_00bf[]={0x2D,0x00,0x00,0x00,0x00}; // constant
psi_t psi00bf={5,psi_00bf,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_SIGNED_CHAR_IN_ECX								0x00c0
	// movsx	ecx,byte ptr [edx=signed char]
byte psi_00c0[]={0x0F,0xBE,0x0A};
psi_t psi00c0={3,psi_00c0,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_UNSIGNED_CHAR_IN_ECX							0x00c1
	// mov		ecx,dword ptr [edx=unsigned char]
	// and		ecx,0FFh
byte psi_00c1[]={0x8B,0x0A,0x81,0xE1,0xFF,0x00,0x00,0x00};
psi_t psi00c1={8,psi_00c1,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_SIGNED_SHORT_IN_ECX								0x00c2
	// movsx	ecx,word ptr [edx=signed short]
byte psi_00c2[]={0x0F,0xBF,0x0A};
psi_t psi00c2={3,psi_00c2,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_UNSIGNED_SHORT_IN_ECX							0x00c3
	// mov		ecx,dword ptr [edx=unsigned short]
	// and		ecx,0FFFFh
byte psi_00c3[]={0x8B,0x0A,0x81,0xE1,0xFF,0xFF,0x00,0x00};
psi_t psi00c3={8,psi_00c3,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_MULTIPLICATION								0x00c4
	// fmul		dword ptr [edx=float]
byte psi_00c4[]={0xD8,0x0A};
psi_t psi00c4={2,psi_00c4,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_MULTIPLICATION							0x00c5
	// fmul		qword ptr [edx=double]
byte psi_00c5[]={0xDC,0x0A};
psi_t psi00c5={2,psi_00c5,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_UNSIGNED_ECX_IN_ST0									0x00c6
// in ecx deve esserci un intero a 32 bit senza segno
	// mov		dword ptr [tmp_qword_0],ecx
	// mov		dword ptr [tmp_qword_0+4],0
	// fild		qword ptr [tmp_qword_0]
byte psi_00c6[]={0x89,0x0D,0x00,0x00,0x00,0x00, // address
	0xC7,0x05,0x04,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xDF,0x2D,0x00,0x00,0x00,0x00}; // address
psi_t psi00c6={22,psi_00c6,0,{-1},{2,8,18,-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_SIGNED_ECX_IN_ST0										0x00c7
	// mov		dword ptr [tmp_word_0],ecx
	// fild		dword ptr [tmp_word_0]
byte psi_00c7[]={0x89,0x0D,0x00,0x00,0x00,0x00, // address
	0xDB,0x05,0x00,0x00,0x00,0x00}; // address
psi_t psi00c7={12,psi_00c7,0,{-1},{2,8,-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_PERCONSTANT_ADDITION								0x00c8
	// imul		ecx,ecx,constant
	// add		eax,ecx
byte psi_00c8[]={0x69,0xC9,0x00,0x00,0x00,0x00, // constant
	0x03,0xC1};
psi_t psi00c8={8,psi_00c8,4,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_PERCONSTANT_SUBTRACTION								0x00c9
	// imul		ecx,ecx,constant
	// sub		eax,ecx
byte psi_00c9[]={0x69,0xC9,0x00,0x00,0x00,0x00, // constant
	0x2B,0xC1};
psi_t psi00c9={8,psi_00c9,4,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_OBJECT_SUBTRACTION									0x00ca
	// mov		ebx,constant
	// sub		eax,ecx
	// cwd
	// idiv		ebx
byte psi_00ca[]={0xBB,0x00,0x00,0x00,0x00, // constant
	0x2B,0xC1,0x66,0x99,0xF7,0xFB};
psi_t psi00ca={11,psi_00ca,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_ADDITION									0x00cb
	// fadd		dword ptr [edx=float]
byte psi_00cb[]={0xD8,0x02};
psi_t psi00cb={2,psi_00cb,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_ADDITION									0x00cc
	// fadd		qword ptr [edx=double]
byte psi_00cc[]={0xDC,0x02};
psi_t psi00cc={2,psi_00cc,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_EQUALITY									0x00cd
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00cd[]={0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00cd={18,psi_00cd,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_EQUALITY									0x00ce
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00ce[]={0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00ce={18,psi_00ce,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_INEQUALITY									0x00cf
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00cf[]={0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00cf={18,psi_00cf,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_INEQUALITY								0x00d0
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00d0[]={0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00d0={18,psi_00d0,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_LOGICAL_AND								0x00d1
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// fldz
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00d1[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x12,0xD9,0xEE,0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00d1={43,psi_00d1,0,{-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_LOGICAL_AND								0x00d2
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// fldz
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00d2[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x12,0xD9,0xEE,0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00d2={43,psi_00d2,0,{-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_FLOAT_LOGICAL_OR									0x00d3
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// fldz
	// fcomp	dword ptr [edx=float]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_00d3[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x0F,0xD9,0xEE,0xD8,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi00d3={43,psi_00d3,0,{-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_EDXPTR_DOUBLE_LOGICAL_OR								0x00d4
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// fldz
	// fcomp	qword ptr [edx=double]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_00d4[]={0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x0F,0xD9,0xEE,0xDC,0x1A,0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi00d4={43,psi_00d4,0,{-1},{2,12,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_ECX_LOGICAL_AND											0x00d5
	// test		ecx,ecx
	// je		jump_0
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// jne		jump_0
	// mov		eax,1
	// jmp		jump_1
	// jump_0:
	// sub		eax,eax
	// jump_1:
byte psi_00d5[]={0x85,0xC9,0x74,0x1E,0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x75,0x07,0xB8,0x01,0x00,0x00,0x00,0xEB,0x02,0x2B,0xC0};
psi_t psi00d5={36,psi_00d5,0,{-1},{6,16,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ST0_ECX_LOGICAL_OR											0x00d6
	// test		ecx,ecx
	// je		jump_0
	// mov		dword ptr [tmp_dword_0],0
	// fcomp	dword ptr [tmp_dword_0]
	// fnstsw	ax
	// test		ah,40h
	// je		jump_0
	// sub		eax,eax
	// jmp		jump_1
	// jump_0:
	// mov		eax,1
	// jump_1:
byte psi_00d6[]={0x85,0xC9,0x74,0x1B,0xC7,0x05,0x00,0x00,0x00,0x00, // address
	0x00,0x00,0x00,0x00,0xD8,0x1D,0x00,0x00,0x00,0x00, // address
	0xDF,0xE0,0xF6,0xC4,0x40,0x74,0x04,0x2B,0xC0,0xEB,0x05,0xB8,0x01,0x00,0x00,0x00};
psi_t psi00d6={36,psi_00d6,0,{-1},{6,16,-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_INITIALIZE_MOVEDATA									0x00d7
	// mov		esi,eax
	// mov		edi,ecx
byte psi_00d7[]={0x8B,0xF0,0x8B,0xF9};
psi_t psi00d7={4,psi_00d7,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define MOVEDATA_DWORD												0x00d8
	// mov		ecx,constant
	// rep		movsd
byte psi_00d8[]={0xB9,0x00,0x00,0x00,0x00, // constant
	0xF3,0xA5};
psi_t psi00d8={7,psi_00d8,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define MOVEDATA_BYTE												0x00d9
	// mov		ecx,constant
	// rep		movsb
byte psi_00d9[]={0xB9,0x00,0x00,0x00,0x00, // constant
	0xF3,0xA4};
psi_t psi00d9={7,psi_00d9,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define NOP															0x00da
	// ---
psi_t psi00da={0,NULL,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECX_IN_EDX												0x00db
	// mov		edx,ecx
byte psi_00db[]={0x8B,0xD1};
psi_t psi00db={2,psi_00db,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EAX_IN_ECX												0x00dc
	// mov		ecx,eax
byte psi_00dc[]={0x8B,0xC8};
psi_t psi00dc={2,psi_00dc,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EAX_IN_EDX												0x00dd
	// mov		edx,eax
byte psi_00dd[]={0x8B,0xD0};
psi_t psi00dd={2,psi_00dd,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECX_IN_EAX												0x00de
	// mov		eax,ecx
byte psi_00de[]={0x8B,0xC1};
psi_t psi00de={2,psi_00de,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_INT_POINTER_IN_EDX								0x00df
	// lea		edx,dword ptr [ebp-x=int]
byte psi_00df[]={0x8D,0x95,0x00,0x00,0x00,0x00}; // address
psi_t psi00df={6,psi_00df,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_INT_IN_EDX										0x00e0
	// mov		edx,dword ptr [edx=int]
byte psi_00e0[]={0x8B,0x12};
psi_t psi00e0={2,psi_00e0,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define CALL_FUNCTION_WITH_NOPARAMS									0x00e1
	// call		(function)
byte psi_00e1[]={0xE8,0x00,0x00,0x00,0x00}; // (dest-instr-5)
psi_t psi00e1={5,psi_00e1,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ECX_CONSTANT_SUBTRACTION									0x00e2
	// sub		ecx,constant
byte psi_00e2[]={0x81,0xE9,0x00,0x00,0x00,0x00}; // constant
psi_t psi00e2={6,psi_00e2,4,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CONSTANT_ADDRESS_IN_EDX								0x00e3
	// mov		edx,(address)
byte psi_00e3[]={0xBA,0x00,0x00,0x00,0x00}; // address
psi_t psi00e3={5,psi_00e3,0,{1,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_FLOAT_NOPOP									0x00e4
	// fst		dword ptr [float]
byte psi_00e4[]={0xD9,0x15,0x00,0x00,0x00,0x00}; // address
psi_t psi00e4={6,psi_00e4,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_DOUBLE_NOPOP									0x00e5
	// fst		qword ptr [double]
byte psi_00e5[]={0xDD,0x15,0x00,0x00,0x00,0x00}; // address
psi_t psi00e5={6,psi_00e5,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_STACK_FLOAT_NOPOP								0x00e6
	// fst		dword ptr [ebp-x=float]
byte psi_00e6[]={0xD9,0x95,0x00,0x00,0x00,0x00}; // offset
psi_t psi00e6={6,psi_00e6,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ST0_IN_STACK_DOUBLE_NOPOP								0x00e7
	// fst		qword ptr [ebp-x=double]
byte psi_00e7[]={0xDD,0x95,0x00,0x00,0x00,0x00}; // offset
psi_t psi00e7={6,psi_00e7,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_EDXPTR_INT_IN_EAX										0x00e8
	// mov		eax,dword ptr [edx=int]
byte psi_00e8[]={0x8B,0x02};
psi_t psi00e8={2,psi_00e8,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_ECX_IN_STACK_INT										0x00e9
	// mov		dword ptr [ebp-x=int],ecx
byte psi_00e9[]={0x89,0x8D,0x00,0x00,0x00,0x00}; // offset
psi_t psi00e9={6,psi_00e9,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_INT_IN_ECX										0x00ea
	// mov		ecx,dword ptr [ecx=int]
byte psi_00ea[]={0x8B,0x09};
psi_t psi00ea={2,psi_00ea,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ST0_IN_ECX												0x00eb
	// wait
	// fnstcw	word ptr [tmp_word_0]
	// wait
	// mov		ax,word ptr [tmp_word_0]
	// or		ah,0Ch
	// mov		word ptr [tmp_word_1],ax
	// fldcw	word ptr [tmp_word_1]
	// fistp	dword ptr [tmp_dword_2]
	// fldcw	word ptr [tmp_word_0]
	// mov		ecx,dword ptr [tmp_dword_2]
byte psi_00eb[]={0x9B,0xD9,0x3D,0x00,0x00,0x00,0x00, // address
	0x9B,0x66,0xA1,0x00,0x00,0x00,0x00, // address
	0x80,0xCC,0x0C,0x66,0xA3,0x02,0x00,0x00,0x00, // address
	0xD9,0x2D,0x02,0x00,0x00,0x00, // address
	0xDB,0x1D,0x04,0x00,0x00,0x00, // address
	0xD9,0x2D,0x00,0x00,0x00,0x00, // address
	0x8B,0x0D,0x04,0x00,0x00,0x00}; // address
psi_t psi00eb={47,psi_00eb,0,{-1},{3,10,19,25,31,37,43,-1},{-1}};
/*------------------------------------------------------------------------*/
#define ARRAY_X_DIMENSION_WITH_LSHIFT								0x00ec
	// shl		eax,constant	// 2^constant=(max(x+1)*max(x+2)*...*maxn*sizeof_type)
	// add		ecx,eax
byte psi_00ec[]={0xC1,0xE0,0x00, // constant
	0x03,0xC8};
psi_t psi00ec={5,psi_00ec,1,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define ARRAY_FIRST_DIMENSION_WITH_LSHIFT							0x00ed
	// shl		eax,constant	// 2^constant=(max2*max3*...*maxn*sizeof_type)
	// lea		ecx,dword ptr (address)[eax]
byte psi_00ed[]={0xC1,0xE0,0x00, // constant
	0x8D,0x88,0x00,0x00,0x00,0x00}; // address
psi_t psi00ed={9,psi_00ed,1,{5,-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define STACK_ARRAY_FIRST_DIMENSION_WITH_LSHIFT						0x00ee
	// shl		eax,constant	// 2^constant=(max2*max3*...*maxn*sizeof_type)
	// lea		ecx,dword ptr (ebp-x=address)[eax]
byte psi_00ee[]={0xC1,0xE0,0x00, // constant
	0x8D,0x8C,0x05,0x00,0x00,0x00,0x00}; // offset
psi_t psi00ee={10,psi_00ee,1,{6,-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define LOAD_STACK_INT_IN_EDX										0x00ef
	// mov		edx,dword ptr [ebp-x=int]
byte psi_00ef[]={0x8B,0x95,0x00,0x00,0x00,0x00}; // offset
psi_t psi00ef={6,psi_00ef,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPTR_INT_IN_EDX										0x00f0
	// mov		edx,dword ptr [ecx=int]
byte psi_00f0[]={0x8B,0x11};
psi_t psi00f0={2,psi_00f0,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_MULTIPLICATION									0x00f1
	// imul		eax,eax,constant
byte psi_00f1[]={0x69,0xC0,0x00,0x00,0x00,0x00}; // constant
psi_t psi00f1={6,psi_00f1,4,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_BITWISE_AND									0x00f2
	// and		eax,constant
byte psi_00f2[]={0x25,0x00,0x00,0x00,0x00}; // constant
psi_t psi00f2={5,psi_00f2,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_BITWISE_EXCLUSIVE_OR							0x00f3
	// xor		eax,constant
byte psi_00f3[]={0x35,0x00,0x00,0x00,0x00}; // constant
psi_t psi00f3={5,psi_00f3,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_BITWISE_OR										0x00f4
	// or		eax,constant
byte psi_00f4[]={0x0D,0x00,0x00,0x00,0x00}; // constant
psi_t psi00f4={5,psi_00f4,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_PERCONSTANT_ADDITION_WITH_LSHIFT					0x00f5
	// shl		ecx,constant
	// add		eax,ecx
byte psi_00f5[]={0xC1,0xE1,0x00, // constant
	0x03,0xC1};
psi_t psi00f5={5,psi_00f5,1,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_PERCONSTANT_SUBTRACTION_WITH_LSHIFT					0x00f6
	// shl		ecx,constant
	// sub		eax,ecx
byte psi_00f6[]={0xC1,0xE1,0x00, // constant
	0x2B,0xC1};
psi_t psi00f6={5,psi_00f6,1,{-1},{-1},{2,-1}};
/*------------------------------------------------------------------------*/
#define EAX_ECX_OBJECT_SUBTRACTION_WITH_LSHIFT						0x00f7
	// sub		eax,ecx
	// shr		eax,constant
byte psi_00f7[]={0x2B,0xC1,0xC1,0xE8,0x00}; // constant
psi_t psi00f7={5,psi_00f7,1,{-1},{-1},{4,-1}};
/*------------------------------------------------------------------------*/
#define ECX_EAX_ADDITION											0x00f8
	// add		ecx,eax
byte psi_00f8[]={0x03,0xC8};
psi_t psi00f8={2,psi_00f8,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ADDRESS_EAX_ADDITION_IN_ECX									0x00f9
	// lea		ecx,dword ptr (address)[eax]
byte psi_00f9[]={0x8D,0x88,0x00,0x00,0x00,0x00}; // address
psi_t psi00f9={6,psi_00f9,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STACK_ADDRESS_EAX_ADDITION_IN_ECX							0x00fa
	// lea		ecx,dword ptr (ebp-x=address)[eax]
byte psi_00fa[]={0x8D,0x8C,0x05,0x00,0x00,0x00,0x00}; // offset
psi_t psi00fa={7,psi_00fa,0,{3,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define RETURN														0x00fb
	// pop		edi
	// pop		esi
	// pop		ebx
	// mov		esp,ebp
	// pop		ebp
	// ret
byte psi_00fb[]={0x5F,0x5E,0x5B,0x8B,0xE5,0x5D,0xC3};
psi_t psi00fb={7,psi_00fb,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ENTER														0x00fc
	// push		ebp
	// mov		ebp,esp
	// sub		esp,constant
	// push		ebx
	// push		esi
	// push		edi
byte psi_00fc[]={0x55,0x8B,0xEC,0x81,0xEC,0x00,0x00,0x00,0x00,0x53,0x56,0x57}; // constant
psi_t psi00fc={12,psi_00fc,4,{-1},{-1},{5,-1}};
/*------------------------------------------------------------------------*/
#define ENTER_WITH_NO_LOCAL_VARS									0x00fd
	// push		ebp
	// mov		ebp,esp
	// push		ebx
	// push		esi
	// push		edi
byte psi_00fd[]={0x55,0x8B,0xEC,0x53,0x56,0x57};
psi_t psi00fd={6,psi_00fd,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define ENTER_AND_CHECK_STACK										0x00fe
	// push		ebp
	// mov		ebp,esp
	// mov		eax,constant
	// cmp		eax,1000h
	// mov		ecx,esp
	// jb		lastpage
	// probepages:
	// sub		ecx,1000h
	// sub		eax,1000h
	// test		dword ptr [ecx],eax
	// cmp		eax,1000h
	// jae		probepages
	// lastpage:
	// sub		ecx,eax
	// mov		eax,esp
	// test		dword ptr [ecx],eax
	// mov		esp,ecx
	// push		ebx
	// push		esi
	// push		edi
byte psi_00fe[]={0x55,0x8B,0xEC,0xB8,0x00,0x00,0x00,0x00, // constant
	0x3D,0x00,0x10,0x00,0x00,0x8B,0xCC,0x72,0x14,0x81,
	0xE9,0x00,0x10,0x00,0x00,0x2D,0x00,0x10,0x00,0x00,
	0x85,0x01,0x3D,0x00,0x10,0x00,0x00,0x73,0xEC,0x2B,
	0xC8,0x8B,0xC4,0x85,0x01,0x8B,0xE1,0x53,0x56,0x57};
psi_t psi00fe={48,psi_00fe,4,{-1},{-1},{4,-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPLUS4PTR_INT_IN_EAX									0x00ff
	// mov		eax,dword ptr [ecx+4=int]
byte psi_00ff[]={0x8B,0x41,0x04};
psi_t psi00ff={3,psi_00ff,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_ECXPLUS4PTR_INT_IN_EDX									0x0100
	// mov		edx,dword ptr [ecx+4=int]
byte psi_0100[]={0x8B,0x51,0x04};
psi_t psi0100={3,psi_0100,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define LOAD_CONSTANT_IN_EDX										0x0101
	// mov		edx,constant
byte psi_0101[]={0xBA,0x00,0x00,0x00,0x00}; // constant
psi_t psi0101={5,psi_0101,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define STORE_EDX_IN_ECXPLUS4PTR_INT								0x0102
	// mov		dword ptr [ecx+4=int],edx
byte psi_0102[]={0x89,0x51,0x04};
psi_t psi0102={3,psi_0102,0,{-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EDX_IN_STACK_INT										0x0103
	// mov		dword ptr [ebp-x=int],edx
byte psi_0103[]={0x89,0x95,0x00,0x00,0x00,0x00}; // address
psi_t psi0103={6,psi_0103,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define STORE_EDX_IN_INT											0x0104
	// mov		dword ptr [int],edx
byte psi_0104[]={0x89,0x15,0x00,0x00,0x00,0x00}; // address
psi_t psi0104={6,psi_0104,0,{2,-1},{-1},{-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_DIVISION										0x0105
	// mov		ecx,constant
	// cwd
	// idiv		ecx
byte psi_0105[]={0xB9,0x00,0x00,0x00,0x00, // constant
	0x66,0x99,0xF7,0xF9};
psi_t psi0105={9,psi_0105,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/
#define EAX_CONSTANT_REMAINDER										0x0106
	// mov		ecx,constant
	// cwd
	// idiv		ecx
	// mov		eax,edx
byte psi_0106[]={0xB9,0x00,0x00,0x00,0x00, // constant
	0x66,0x99,0xF7,0xF9,0xF7,0xF9};
psi_t psi0106={11,psi_0106,4,{-1},{-1},{1,-1}};
/*------------------------------------------------------------------------*/

//==================
//pseudo-instructions list
//
//array di puntatori alle singole pseudo istruzioni
//==================
psi_t *psi[]={	&psi0000,&psi0001,&psi0002,&psi0003,&psi0004,&psi0005,&psi0006,&psi0007,
				&psi0008,&psi0009,&psi000a,&psi000b,&psi000c,&psi000d,&psi000e,&psi000f,
				&psi0010,&psi0011,&psi0012,&psi0013,&psi0014,&psi0015,&psi0016,&psi0017,
				&psi0018,&psi0019,&psi001a,&psi001b,&psi001c,&psi001d,&psi001e,&psi001f,
				&psi0020,&psi0021,&psi0022,&psi0023,&psi0024,&psi0025,&psi0026,&psi0027,
				&psi0028,&psi0029,&psi002a,&psi002b,&psi002c,&psi002d,&psi002e,&psi002f,
				&psi0030,&psi0031,&psi0032,&psi0033,&psi0034,&psi0035,&psi0036,&psi0037,
				&psi0038,&psi0039,&psi003a,&psi003b,&psi003c,&psi003d,&psi003e,&psi003f,
				&psi0040,&psi0041,&psi0042,&psi0043,&psi0044,&psi0045,&psi0046,&psi0047,
				&psi0048,&psi0049,&psi004a,&psi004b,&psi004c,&psi004d,&psi004e,&psi004f,
				&psi0050,&psi0051,&psi0052,&psi0053,&psi0054,&psi0055,&psi0056,&psi0057,
				&psi0058,&psi0059,&psi005a,&psi005b,&psi005c,&psi005d,&psi005e,&psi005f,
				&psi0060,&psi0061,&psi0062,&psi0063,&psi0064,&psi0065,&psi0066,&psi0067,
				&psi0068,&psi0069,&psi006a,&psi006b,&psi006c,&psi006d,&psi006e,&psi006f,
				&psi0070,&psi0071,&psi0072,&psi0073,&psi0074,&psi0075,&psi0076,&psi0077,
				&psi0078,&psi0079,&psi007a,&psi007b,&psi007c,&psi007d,&psi007e,&psi007f,
				&psi0080,&psi0081,&psi0082,&psi0083,&psi0084,&psi0085,&psi0086,&psi0087,
				&psi0088,&psi0089,&psi008a,&psi008b,&psi008c,&psi008d,&psi008e,&psi008f,
				&psi0090,&psi0091,&psi0092,&psi0093,&psi0094,&psi0095,&psi0096,&psi0097,
				&psi0098,&psi0099,&psi009a,&psi009b,&psi009c,&psi009d,&psi009e,&psi009f,
				&psi00a0,&psi00a1,&psi00a2,&psi00a3,&psi00a4,&psi00a5,&psi00a6,&psi00a7,
				&psi00a8,&psi00a9,&psi00aa,&psi00ab,&psi00ac,&psi00ad,&psi00ae,&psi00af,
				&psi00b0,&psi00b1,&psi00b2,&psi00b3,&psi00b4,&psi00b5,&psi00b6,&psi00b7,
				&psi00b8,&psi00b9,&psi00ba,&psi00bb,&psi00bc,&psi00bd,&psi00be,&psi00bf,
				&psi00c0,&psi00c1,&psi00c2,&psi00c3,&psi00c4,&psi00c5,&psi00c6,&psi00c7,
				&psi00c8,&psi00c9,&psi00ca,&psi00cb,&psi00cc,&psi00cd,&psi00ce,&psi00cf,
				&psi00d0,&psi00d1,&psi00d2,&psi00d3,&psi00d4,&psi00d5,&psi00d6,&psi00d7,
				&psi00d8,&psi00d9,&psi00da,&psi00db,&psi00dc,&psi00dd,&psi00de,&psi00df,
				&psi00e0,&psi00e1,&psi00e2,&psi00e3,&psi00e4,&psi00e5,&psi00e6,&psi00e7,
				&psi00e8,&psi00e9,&psi00ea,&psi00eb,&psi00ec,&psi00ed,&psi00ee,&psi00ef,
				&psi00f0,&psi00f1,&psi00f2,&psi00f3,&psi00f4,&psi00f5,&psi00f6,&psi00f7,
				&psi00f8,&psi00f9,&psi00fa,&psi00fb,&psi00fc,&psi00fd,&psi00fe,&psi00ff,
				&psi0100,&psi0101,&psi0102,&psi0103,&psi0104,&psi0105,&psi0106
};

//==================
//pseudo-instructions names list
//
//array di puntatori ai nomi delle pseudo istruzioni
//==================
char *psi_names[]={
	"LOAD_SIGNED_CHAR_IN_EAX",
	"LOAD_SIGNED_CHAR_IN_ECX",
	"LOAD_UNSIGNED_CHAR_IN_EAX",
	"LOAD_UNSIGNED_CHAR_IN_ECX",
	"LOAD_SIGNED_SHORT_IN_EAX",
	"LOAD_SIGNED_SHORT_IN_ECX",
	"LOAD_UNSIGNED_SHORT_IN_EAX",
	"LOAD_UNSIGNED_SHORT_IN_ECX",
	"LOAD_INT_IN_EAX",
	"LOAD_INT_IN_ECX",
	"LOAD_FLOAT_IN_ST0",
	"LOAD_DOUBLE_IN_ST0",
	"LOAD_SIGNED_EAX_IN_ST0",
	"LOAD_UNSIGNED_EAX_IN_ST0",
	"LOAD_ST0_IN_EAX",
	"STORE_EAX_IN_CHAR",
	"STORE_EAX_IN_SHORT",
	"STORE_EAX_IN_INT",
	"STORE_ST0_IN_FLOAT",
	"STORE_ST0_IN_DOUBLE",
	"EAX_TO_UNSIGNED_8_BIT_VALUE",
	"EAX_TO_SIGNED_8_BIT_VALUE",
	"EAX_TO_UNSIGNED_16_BIT_VALUE",
	"EAX_TO_SIGNED_16_BIT_VALUE",
	"ARRAY_FIRST_DIMENSION",
	"ARRAY_X_DIMENSION",
	"CALL_FUNCTION",
	"EAX_ARITHMETIC_NEGATION",
	"ST0_ARITHMETIC_NEGATION",
	"EAX_BITWISE_COMPLEMENT",
	"EAX_LOGICAL_NOT",
	"ST0_LOGICAL_NOT",
	"EAX_ECX_MULTIPLICATION",
	"ST0_FLOAT_MULTIPLICATION",
	"ST0_DOUBLE_MULTIPLICATION",
	"EAX_ECX_DIVISION",
	"UNSIGNED_EAX_ECX_DIVISION",
	"ST0_FLOAT_DIVISION",
	"ST0_DOUBLE_DIVISION",
	"ST0_EDXPTR_FLOAT_DIVISION",
	"ST0_EDXPTR_DOUBLE_DIVISION",
	"EAX_ECX_REMAINDER",
	"UNSIGNED_EAX_ECX_REMAINDER",
	"EAX_ECX_ADDITION",
	"ST0_FLOAT_ADDITION",
	"ST0_DOUBLE_ADDITION",
	"EAX_ECX_SUBTRACTION",
	"ST0_FLOAT_SUBTRACTION",
	"ST0_DOUBLE_SUBTRACTION",
	"ST0_EDXPTR_FLOAT_SUBTRACTION",
	"ST0_EDXPTR_DOUBLE_SUBTRACTION",
	"EAX_ECX_LEFT_SHIFT",
	"EAX_ECX_RIGHT_SHIFT",
	"UNSIGNED_EAX_ECX_RIGHT_SHIFT",
	"EAX_CONSTANT_LEFT_SHIFT",
	"EAX_CONSTANT_RIGHT_SHIFT",
	"UNSIGNED_EAX_CONSTANT_RIGHT_SHIFT",
	"EAX_ECX_LESS_THAN",
	"EAX_ECX_UNSIGNED_LESS_THAN",
	"EAX_ECX_GREATER_THAN",
	"EAX_ECX_UNSIGNED_GREATER_THAN",
	"EAX_ECX_LESS_THAN_OR_EQUAL_TO",
	"EAX_ECX_UNSIGNED_LESS_THAN_OR_EQUAL_TO",
	"EAX_ECX_GREATER_THAN_OR_EQUAL_TO",
	"EAX_ECX_UNSIGNED_GREATER_THAN_OR_EQUAL_TO",
	"EAX_ECX_EQUALITY",
	"EAX_ECX_INEQUALITY",
	"ST0_FLOAT_LESS_THAN",
	"ST0_DOUBLE_LESS_THAN",
	"ST0_EDXPTR_FLOAT_LESS_THAN",
	"ST0_EDXPTR_DOUBLE_LESS_THAN",
	"ST0_FLOAT_GREATER_THAN",
	"ST0_DOUBLE_GREATER_THAN",
	"ST0_EDXPTR_FLOAT_GREATER_THAN",
	"ST0_EDXPTR_DOUBLE_GREATER_THAN",
	"ST0_FLOAT_EQUALITY",
	"ST0_DOUBLE_EQUALITY",
	"ST0_FLOAT_INEQUALITY",
	"ST0_DOUBLE_INEQUALITY",
	"ST0_FLOAT_LESS_THAN_OR_EQUAL_TO",
	"ST0_DOUBLE_LESS_THAN_OR_EQUAL_TO",
	"ST0_EDXPTR_FLOAT_LESS_THAN_OR_EQUAL_TO",
	"ST0_EDXPTR_DOUBLE_LESS_THAN_OR_EQUAL_TO",
	"ST0_FLOAT_GREATER_THAN_OR_EQUAL_TO",
	"ST0_DOUBLE_GREATER_THAN_OR_EQUAL_TO",
	"ST0_EDXPTR_FLOAT_GREATER_THAN_OR_EQUAL_TO",
	"ST0_EDXPTR_DOUBLE_GREATER_THAN_OR_EQUAL_TO",
	"EAX_ECX_BITWISE_AND",
	"EAX_ECX_BITWISE_EXCLUSIVE_OR",
	"EAX_ECX_BITWISE_OR",
	"EAX_ECX_LOGICAL_AND",
	"EAX_ECX_LOGICAL_OR",
	"ST0_FLOAT_LOGICAL_AND",
	"ST0_DOUBLE_LOGICAL_AND",
	"ST0_FLOAT_LOGICAL_OR",
	"ST0_DOUBLE_LOGICAL_OR",
	"JUMP_IF_EAX_NOT_ZERO",
	"JUMP_IF_ST0_NOT_ZERO",
	"JUMP_IF_EAX_ZERO",
	"JUMP_IF_ST0_ZERO",
	"JUMP",
	"LOAD_STACK_SIGNED_CHAR_IN_EAX",
	"LOAD_STACK_SIGNED_CHAR_IN_ECX",
	"LOAD_STACK_UNSIGNED_CHAR_IN_EAX",
	"LOAD_STACK_UNSIGNED_CHAR_IN_ECX",
	"LOAD_STACK_SIGNED_SHORT_IN_EAX",
	"LOAD_STACK_SIGNED_SHORT_IN_ECX",
	"LOAD_STACK_UNSIGNED_SHORT_IN_EAX",
	"LOAD_STACK_UNSIGNED_SHORT_IN_ECX",
	"LOAD_STACK_INT_IN_EAX",
	"LOAD_STACK_INT_IN_ECX",
	"LOAD_STACK_FLOAT_IN_ST0",
	"LOAD_STACK_DOUBLE_IN_ST0",
	"STORE_EAX_IN_STACK_CHAR",
	"STORE_EAX_IN_STACK_SHORT",
	"STORE_EAX_IN_STACK_INT",
	"STORE_ST0_IN_STACK_FLOAT",
	"STORE_ST0_IN_STACK_DOUBLE",
	"STACK_ARRAY_FIRST_DIMENSION",
	"ST0_STACK_FLOAT_MULTIPLICATION",
	"ST0_STACK_DOUBLE_MULTIPLICATION",
	"ST0_STACK_FLOAT_DIVISION",
	"ST0_STACK_DOUBLE_DIVISION",
	"STACK_FLOAT_ST0_DIVISION",
	"STACK_DOUBLE_ST0_DIVISION",
	"ST0_STACK_FLOAT_ADDITION",
	"ST0_STACK_DOUBLE_ADDITION",
	"ST0_STACK_FLOAT_SUBTRACTION",
	"ST0_STACK_DOUBLE_SUBTRACTION",
	"STACK_FLOAT_ST0_SUBTRACTION",
	"STACK_DOUBLE_ST0_SUBTRACTION",
	"ST0_STACK_FLOAT_LESS_THAN",
	"ST0_STACK_DOUBLE_LESS_THAN",
	"STACK_FLOAT_ST0_LESS_THAN",
	"STACK_DOUBLE_ST0_LESS_THAN",
	"ST0_STACK_FLOAT_GREATER_THAN",
	"ST0_STACK_DOUBLE_GREATER_THAN",
	"STACK_FLOAT_ST0_GREATER_THAN",
	"STACK_DOUBLE_ST0_GREATER_THAN",
	"ST0_STACK_FLOAT_EQUALITY",
	"ST0_STACK_DOUBLE_EQUALITY",
	"ST0_STACK_FLOAT_INEQUALITY",
	"ST0_STACK_DOUBLE_INEQUALITY",
	"ST0_STACK_FLOAT_LESS_THAN_OR_EQUAL_TO",
	"ST0_STACK_DOUBLE_LESS_THAN_OR_EQUAL_TO",
	"STACK_FLOAT_ST0_LESS_THAN_OR_EQUAL_TO",
	"STACK_DOUBLE_ST0_LESS_THAN_OR_EQUAL_TO",
	"ST0_STACK_FLOAT_GREATER_THAN_OR_EQUAL_TO",
	"ST0_STACK_DOUBLE_GREATER_THAN_OR_EQUAL_TO",
	"STACK_FLOAT_ST0_GREATER_THAN_OR_EQUAL_TO",
	"STACK_DOUBLE_ST0_GREATER_THAN_OR_EQUAL_TO",
	"ST0_STACK_FLOAT_LOGICAL_AND",
	"ST0_STACK_DOUBLE_LOGICAL_AND",
	"ST0_STACK_FLOAT_LOGICAL_OR",
	"ST0_STACK_DOUBLE_LOGICAL_OR",
	"LOAD_CONSTANT_IN_EAX",
	"STORE_ECX_IN_INT",
	"PUSH_CONSTANT",
	"PUSH_INT",
	"PUSH_EAX",
	"PUSH_STACK_INT",
	"EAX_ECX_PUSH_MEMORY_BLOCK",
	"PUSH_CONSTANT_ADDRESS",
	"LOAD_STACK_INT_POINTER_IN_EAX",
	"LOAD_CONSTANT_ADDRESS_IN_EAX",
	"LOAD_STACK_INT_POINTER_IN_ECX",
	"LOAD_CONSTANT_ADDRESS_IN_ECX",
	"LOAD_STACK_CHAR_IN_EAX",
	"LOAD_CHAR_IN_EAX",
	"LOAD_STACK_SHORT_IN_EAX",
	"LOAD_SHORT_IN_EAX",
	"LOAD_CONSTANT_IN_ECX",
	"LOAD_ECXPTR_SIGNED_CHAR_IN_EAX",
	"LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX",
	"LOAD_ECXPTR_SIGNED_SHORT_IN_EAX",
	"LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX",
	"LOAD_ECXPTR_INT_IN_EAX",
	"LOAD_ECXPTR_FLOAT_IN_ST0",
	"LOAD_ECXPTR_DOUBLE_IN_ST0",
	"LOAD_ECXPTR_CHAR_IN_EAX",
	"LOAD_ECXPTR_SHORT_IN_EAX",
	"ECX_CONSTANT_ADDITION",
	"LOAD_INT_IN_EDX",
	"LOAD_EDXPTR_INT_IN_ECX",
	"EAX_CONSTANT_ADDITION",
	"STORE_CONSTANT_IN_MEM",
	"STORE_EAX_IN_ECXPTR_CHAR",
	"STORE_EAX_IN_ECXPTR_SHORT",
	"STORE_EAX_IN_ECXPTR_INT",
	"STORE_ST0_IN_ECXPTR_FLOAT",
	"STORE_ST0_IN_ECXPTR_DOUBLE",
	"EAX_CONSTANT_SUBTRACTION",
	"LOAD_EDXPTR_SIGNED_CHAR_IN_ECX",
	"LOAD_EDXPTR_UNSIGNED_CHAR_IN_ECX",
	"LOAD_EDXPTR_SIGNED_SHORT_IN_ECX",
	"LOAD_EDXPTR_UNSIGNED_SHORT_IN_ECX",
	"ST0_EDXPTR_FLOAT_MULTIPLICATION",
	"ST0_EDXPTR_DOUBLE_MULTIPLICATION",
	"LOAD_UNSIGNED_ECX_IN_ST0",
	"LOAD_SIGNED_ECX_IN_ST0",
	"EAX_ECX_PERCONSTANT_ADDITION",
	"EAX_ECX_PERCONSTANT_SUBTRACTION",
	"EAX_ECX_OBJECT_SUBTRACTION",
	"ST0_EDXPTR_FLOAT_ADDITION",
	"ST0_EDXPTR_DOUBLE_ADDITION",
	"ST0_EDXPTR_FLOAT_EQUALITY",
	"ST0_EDXPTR_DOUBLE_EQUALITY",
	"ST0_EDXPTR_FLOAT_INEQUALITY",
	"ST0_EDXPTR_DOUBLE_INEQUALITY",
	"ST0_EDXPTR_FLOAT_LOGICAL_AND",
	"ST0_EDXPTR_DOUBLE_LOGICAL_AND",
	"ST0_EDXPTR_FLOAT_LOGICAL_OR",
	"ST0_EDXPTR_DOUBLE_LOGICAL_OR",
	"ST0_ECX_LOGICAL_AND",
	"ST0_ECX_LOGICAL_OR",
	"EAX_ECX_INITIALIZE_MOVEDATA",
	"MOVEDATA_DWORD",
	"MOVEDATA_BYTE",
	"NOP",
	"LOAD_ECX_IN_EDX",
	"LOAD_EAX_IN_ECX",
	"LOAD_EAX_IN_EDX",
	"LOAD_ECX_IN_EAX",
	"LOAD_STACK_INT_POINTER_IN_EDX",
	"LOAD_EDXPTR_INT_IN_EDX",
	"CALL_FUNCTION_WITH_NOPARAMS",
	"ECX_CONSTANT_SUBTRACTION",
	"LOAD_CONSTANT_ADDRESS_IN_EDX",
	"STORE_ST0_IN_FLOAT_NOPOP",
	"STORE_ST0_IN_DOUBLE_NOPOP",
	"STORE_ST0_IN_STACK_FLOAT_NOPOP",
	"STORE_ST0_IN_STACK_DOUBLE_NOPOP",
	"LOAD_EDXPTR_INT_IN_EAX",
	"STORE_ECX_IN_STACK_INT",
	"LOAD_ECXPTR_INT_IN_ECX",
	"LOAD_ST0_IN_ECX",
	"ARRAY_X_DIMENSION_WITH_LSHIFT",
	"ARRAY_FIRST_DIMENSION_WITH_LSHIFT",
	"STACK_ARRAY_FIRST_DIMENSION_WITH_LSHIFT",
	"LOAD_STACK_INT_IN_EDX",
	"LOAD_ECXPTR_INT_IN_EDX",
	"EAX_CONSTANT_MULTIPLICATION",
	"EAX_CONSTANT_BITWISE_AND",
	"EAX_CONSTANT_BITWISE_EXCLUSIVE_OR",
	"EAX_CONSTANT_BITWISE_OR",
	"EAX_ECX_PERCONSTANT_ADDITION_WITH_LSHIFT",
	"EAX_ECX_PERCONSTANT_SUBTRACTION_WITH_LSHIFT",
	"EAX_ECX_OBJECT_SUBTRACTION_WITH_LSHIFT",
	"ECX_EAX_ADDITION",
	"ADDRESS_EAX_ADDITION_IN_ECX",
	"STACK_ADDRESS_EAX_ADDITION_IN_ECX",
	"RETURN",
	"ENTER",
	"ENTER_WITH_NO_LOCAL_VARS",
	"ENTER_AND_CHECK_STACK",
	"LOAD_ECXPLUS4PTR_INT_IN_EAX",
	"LOAD_ECXPLUS4PTR_INT_IN_EDX",
	"LOAD_CONSTANT_IN_EDX",
	"STORE_EDX_IN_ECXPLUS4PTR_INT",
	"STORE_EDX_IN_STACK_INT",
	"STORE_EDX_IN_INT",
	"EAX_CONSTANT_DIVISION",
	"EAX_CONSTANT_REMAINDER"
};

//==================
//sections names list
//
//array di puntatori ai nomi delle section
//==================
char *sections_names[]={
	"HEADER",
	"CODE",
	"RELOCATION",
	"NAMES",
	"FUNCTIONS",
	"EXTFUNCTIONS",
	"STRINGS",
	"DATA",
	"TEMPDATA0",
	"TEMPDATA1",
	"STACK",
	"PSI",
	"LABELS"
};

//===============
//CompareStrings
//
//Confronta due stringhe e restituisce 1 se sono identiche
//===============
__inline int CompareStrings (char *string0, char *string1, int len)
{
	// confronta le due stringhe
	if (len==1) {
		if (*string0==*string1)
			return(1);
	}
	else if (len==2) {
		if (*(unsigned short *)string0==*(unsigned short *)string1)
			return(1);
	}
	else if (len==4) {
		if (*(unsigned int *)string0==*(unsigned int *)string1)
			return(1);
	}
	else {
		if (!memcmp(string0,string1,len))
			return(1);
	}

	// ritorna
	return(0);
}

//=================
//InitCompiler
//
//inizializza il compilatore
//=================
float compiler_memory=0.0f;
int InitCompiler (void)
{
	// alloca la memoria
	if ( compiler_memory_factor != compiler_memory_old_factor ) {

		// libera la memoria eventualmente allocata
		FreeCompilerMemory();

		// alloca la memoria
		compiler_memory=0.0f;
#pragma warning ( disable : 4244 )
		console=			MALLOC(compiler_memory+=compiler_memory_factor*console_dim);
		error_files=		MALLOC(compiler_memory+=compiler_memory_factor*error_files_mem_dim);
		dependencies_mem=	MALLOC(compiler_memory+=compiler_memory_factor*dependencies_mem_dim);
		expr_tokens=		MALLOC(compiler_memory+=compiler_memory_factor*expr_tokens_mem);
		identifiers=		MALLOC(compiler_memory+=compiler_memory_factor*identifiers_mem);
		operators=			MALLOC(compiler_memory+=compiler_memory_factor*operators_mem);
		cident=				MALLOC(compiler_memory+=compiler_memory_factor*cident_mem);
		cident_aux=			MALLOC(compiler_memory+=compiler_memory_factor*cident_aux_mem);
		cident_aux2=		MALLOC(compiler_memory+=compiler_memory_factor*cident_aux2_mem);
		auxmem=				MALLOC(compiler_memory+=compiler_memory_factor*auxmem_dim);
		rcemem=				MALLOC(compiler_memory+=compiler_memory_factor*rcemem_dim);
		psi_mem=			MALLOC(compiler_memory+=compiler_memory_factor*psi_mem_dim);
		code=				MALLOC(compiler_memory+=compiler_memory_factor*code_mem_dim);
		relocation=			MALLOC(compiler_memory+=compiler_memory_factor*reloc_mem_dim);
		names=				MALLOC(compiler_memory+=compiler_memory_factor*names_mem_dim);
		functions=			MALLOC(compiler_memory+=compiler_memory_factor*func_mem_dim);
		functions2=			MALLOC(compiler_memory+=compiler_memory_factor*func_mem_dim);
		extfunctions=		MALLOC(compiler_memory+=compiler_memory_factor*ext_func_mem_dim);
		strings=			MALLOC(compiler_memory+=compiler_memory_factor*strings_mem_dim);
		preproc_stack=		MALLOC(compiler_memory+=compiler_memory_factor*preproc_stack_dim);
		main_stack=			MALLOC(compiler_memory+=compiler_memory_factor*main_stack_dim);
		main_stack_aux_mem=	MALLOC(compiler_memory+=compiler_memory_factor*main_stack_aux_mem_dim);
		labels=				MALLOC(compiler_memory+=compiler_memory_factor*labels_mem_dim);
		labels_ptrs=		MALLOC(compiler_memory+=compiler_memory_factor*labels_ptrs_mem_dim);
		psi_bounds=			MALLOC(compiler_memory+=compiler_memory_factor*psi_bounds_mem_dim);
#pragma warning ( default : 4244 )

	}
	compiler_memory_old_factor=compiler_memory_factor;

	// resetta i vari puntatori
	console_ptr=console;
	cident_aux_ptr=cident_aux;
	cident_aux2_ptr=cident_aux2;
	psi_ptr=psi_mem;
	code_ptr=code;
	relocation_ptr=relocation;
	names_ptr=names;
	functions_ptr=functions;
	functions2_ptr=functions2;
	extfunctions_ptr=extfunctions;
	strings_ptr=strings;
	preproc_stack_ptr=preproc_stack;
	main_stack_ptr=main_stack;
	main_stack_aux_mem_ptr=main_stack_aux_mem;
	labels_ptr=labels;
	labels_ptrs_ptr=labels_ptrs;
	psi_bounds_ptr=psi_bounds;
	error_files_ptr=error_files;

	// inizializza dependencies_mem
	if ( dependencies_mem )
		strcpy(dependencies_mem,"");

	// resetta current_console_message
	current_console_message=0;

	// stampa il nome del compilatore nella console
	if (console) {
		/*
		ConsolePrintf("Built-in miniC4 Compiler vers1.0, (c)1999, PLANTAMURA SOFTWARE. %ikb",(int)compiler_memory/1024);
		ConsolePrintf("---------------------------------------------------------------------------");
		*/
	}

	// controlla che sia tutto ok
	if (!expr_tokens ||
		!identifiers ||
		!console ||
		!operators ||
		!cident ||
		!cident_aux ||
		!cident_aux2 ||
		!auxmem ||
		!rcemem ||
		!psi_mem ||
		!code ||
		!relocation ||
		!names ||
		!functions ||
		!functions2 ||
		!extfunctions ||
		!strings ||
		!preproc_stack ||
		!main_stack ||
		!main_stack_aux_mem ||
		!labels ||
		!labels_ptrs ||
		!psi_bounds ||
		!error_files ||
		!dependencies_mem ) {
		CompilerErrorOrWarning(175);
		return(1);
	}

	// resetta fatal_error
	fatal_error=0;

	// resetta cident_id
	cident_id=0;
	cident_id_local=99999999;

	// resetta cur_global_var_address
	cur_global_var_address=0;

	// resetta struct_num
	struct_num=0;

	// resetta error_file
	*error_file=0;

	// resetta current_line
	current_line=0;

	// resetta statement_incomplete e statement_start
	statement_incomplete=NULL;
	statement_start=NULL;

	// resetta cur_prototype
	cur_prototype=NULL;

	// imposta al valore di default struct_alignment
	struct_alignment=struct_alignment_default;

	// resetta psi_output_stream e psi_output_stream_index
	psi_output_stream=NULL;
	psi_output_stream_index=0;

	// resetta compiler_errors e compiler_warnings
	compiler_errors=0;
	compiler_warnings=0;

	// resetta current_file
	*current_file=0;

	// ritorna
	return(0);
}

//====================
//CompilerMemoryCheckPoint
//
//Controlla che non vengano superati i limiti della memoria allocata
//====================
int CompilerMemoryCheckPoint (void)
{
	// controlla la memoria allocata
	if ( cident_id*sizeof(identifier_t)							> compiler_memory_factor*cident_mem-check_point_offset ) {
		CompilerErrorOrWarning(176);
		return(1);
	}
	if ( (int)cident_aux_ptr-(int)cident_aux					> compiler_memory_factor*cident_aux_mem-check_point_offset ) {
		CompilerErrorOrWarning(177);
		return(1);
	}
	if ( (int)cident_aux2_ptr-(int)cident_aux2					> compiler_memory_factor*cident_aux2_mem-check_point_offset ) {
		CompilerErrorOrWarning(235);
		return(1);
	}
	if ( (int)psi_ptr-(int)psi_mem								> compiler_memory_factor*psi_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(178);
		return(1);
	}
	if ( (int)code_ptr-(int)code								> compiler_memory_factor*code_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(179);
		return(1);
	}
	if ( (int)relocation_ptr-(int)relocation					> compiler_memory_factor*reloc_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(180);
		return(1);
	}
	if ( (int)names_ptr-(int)names								> compiler_memory_factor*names_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(181);
		return(1);
	}
	if ( (int)functions_ptr-(int)functions						> compiler_memory_factor*func_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(182);
		return(1);
	}
	if ( (int)functions2_ptr-(int)functions2					> compiler_memory_factor*func_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(182);
		return(1);
	}
	if ( (int)extfunctions_ptr-(int)extfunctions				> compiler_memory_factor*ext_func_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(183);
		return(1);
	}
	if ( (int)strings_ptr-(int)strings							> compiler_memory_factor*strings_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(184);
		return(1);
	}
	if ( (int)preproc_stack_ptr-(int)preproc_stack				> compiler_memory_factor*preproc_stack_dim-check_point_offset ) {
		CompilerErrorOrWarning(185);
		return(1);
	}
	if ( (int)main_stack_ptr-(int)main_stack					> compiler_memory_factor*main_stack_dim-check_point_offset ) {
		CompilerErrorOrWarning(210);
		return(1);
	}
	if ( (int)main_stack_aux_mem_ptr-(int)main_stack_aux_mem	> compiler_memory_factor*main_stack_aux_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(211);
		return(1);
	}
	if ( (int)labels_ptr-(int)labels							> compiler_memory_factor*labels_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(247);
		return(1);
	}
	if ( (int)labels_ptrs_ptr-(int)labels_ptrs					> compiler_memory_factor*labels_ptrs_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(250);
		return(1);
	}
	if ( (int)psi_bounds_ptr-(int)psi_bounds					> compiler_memory_factor*psi_bounds_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(259);
		return(1);
	}
	if ( (int)error_files_ptr-(int)error_files					> compiler_memory_factor*error_files_mem_dim-check_point_offset ) {
		CompilerErrorOrWarning(266);
		return(1);
	}

	// ritorna
	return(0);
}

//================
//ParseExpressionTokens
//
//Crea la lista di tokens
//================
int ParseExpressionTokens (char *expr, int buffer)
{
	token_t		*expr_tokens_b=(token_t*)((char *)expr_tokens+buffer*(int)(single_expr_tokens_buffer*compiler_memory_factor));
	char		*identifiers_b=identifiers+buffer*(int)(single_identifiers_buffer*compiler_memory_factor);

	token_t		*tok_ptr=expr_tokens_b;
	char		*idf_ptr=identifiers_b;

	char		*exp_ptr=expr;

	int			ret;

	char		parentheses_stack[parentheses_stack_dim];
	int			parentheses_sp;

	int			i,j;

	token_t		*token;

	char		*nested_macro_stack[1024];
	int			nested_macro_stack_pointer=0;

	// analizza i diversi token
	while (1) {

		// salta gli spazi vuoti
		for (;;exp_ptr++)
			if ( *exp_ptr!=' ' &&
				*exp_ptr!='\t' )
				break;
		tok_ptr->ptr0=NULL;
		tok_ptr->tok_taken=0;
		tok_ptr->ptr1=exp_ptr;
		tok_ptr->sizeof_result=0;
		tok_ptr->typecast_type=0;
		tok_ptr->typecast_indir=0;
		tok_ptr->typecast_stype=NULL;
		tok_ptr->func_call_has_one_param=0;
		tok_ptr->func_call_commas=0;
		tok_ptr->comma_number=-1;
		tok_ptr->comma_func_tok=-1;
		tok_ptr->ellipses_arguments_dim=0;

		// controlla se la stringa ASCIIZ è finita
		if (!*exp_ptr) {
			do {
				if (nested_macro_stack_pointer)
					exp_ptr=nested_macro_stack[--nested_macro_stack_pointer];
				for (;;exp_ptr++)
					if ( *exp_ptr!=' ' &&
						*exp_ptr!='\t' )
						break;
			} while ( !*exp_ptr && nested_macro_stack_pointer );
			if ( !*exp_ptr && !nested_macro_stack_pointer ) {
				tok_ptr->id=EXPRESSION_END;
				break;
			}
		}

		// calcola in ret il numero di caratteri di identificatore consecutivi
		ret=IsIdentfString(exp_ptr);

		// sizeof
		if ( ret == 6 && CompareStrings(exp_ptr,"sizeof",6) ) {
			tok_ptr->id=SIZE_OF_TYPE;
			exp_ptr+=6;
		}

		// identificatore
		else if ( ret ) {

			// controlla se l'identificatore è una macro del compilatore
			if ( ret==4 &&
				( CompareStrings(exp_ptr,"NULL",4) ||
				CompareStrings(exp_ptr,"null",4) ) ) {
				tok_ptr->id=IDENTIFIER;
				tok_ptr->ptr0="0";
				exp_ptr+=ret;
			}
			else if ( ret==4 &&
				( CompareStrings(exp_ptr,"TRUE",4) ||
				CompareStrings(exp_ptr,"true",4) ) ) {
				tok_ptr->id=IDENTIFIER;
				tok_ptr->ptr0="1";
				exp_ptr+=ret;
			}
			else if ( ret==5 &&
				( CompareStrings(exp_ptr,"FALSE",5) ||
				CompareStrings(exp_ptr,"false",5) ) ) {
				tok_ptr->id=IDENTIFIER;
				tok_ptr->ptr0="0";
				exp_ptr+=ret;
			}
			// l'identificatore non è un macro del compilatore
			else {

				// isola l'identificatore
				memcpy(idf_ptr,exp_ptr,ret);
				idf_ptr[ret]=0;

				// controlla se è stata definita una macro col suo nome
				for (i=0;i<cident_id;i++)
					if ( cident[i].type == MACRO &&
						!strcmp(idf_ptr,cident[i].id) )
						break;

				// l'identificatore non indica una macro
				if (i==cident_id) {
					tok_ptr->id=IDENTIFIER;
					tok_ptr->ptr0=idf_ptr;
					idf_ptr+=ret+1;
					exp_ptr+=ret;
				}
				// l'identificatore è una definizione del tipo "#define id"
				else if ( !*(char*)cident[i].pointer ) {
					exp_ptr+=ret;
					continue;
				}
				// l'identificatore è una macro
				else {
					nested_macro_stack[nested_macro_stack_pointer++]=exp_ptr+ret;
					exp_ptr=cident[i].pointer;
					continue;
				}

			}

		}

		// stringa
		else if (ret=IsString(exp_ptr)) {

			if (ret==-1) {
				CompilerErrorOrWarning(3);
				return(1);
			}

			tok_ptr->id=STRING;
			tok_ptr->ptr0=idf_ptr;

			memcpy(idf_ptr,exp_ptr,ret);
			idf_ptr[ret]=0;
			idf_ptr+=ret+1;

			exp_ptr+=ret;
		}

		// parentesi tonde
		else if (CompareStrings(exp_ptr,"(",1)) {
			if (tok_ptr!=expr_tokens_b) {
				if ((tok_ptr-1)->id==IDENTIFIER) {
					tok_ptr->id=FUNCTION_CALL;
					exp_ptr++;
					goto skip;
				}
			}
			tok_ptr->id=O_PARENTHESES;
			exp_ptr++;
		}
		else if (CompareStrings(exp_ptr,")",1)) {
			tok_ptr->id=C_PARENTHESES;
			exp_ptr++;
		}

		// parentesi quadre
		else if (CompareStrings(exp_ptr,"[",1)) {
			tok_ptr->id=ARRAY_SUBSCRIPT;
			exp_ptr++;
		}
		else if (CompareStrings(exp_ptr,"]",1)) {
			tok_ptr->id=CA_PARENTHESES;
			exp_ptr++;
		}

		// membro di struttura
		else if (CompareStrings(exp_ptr,".",1)) {
			tok_ptr->id=MEMBER_SELECTION_OBJECT;
			exp_ptr++;
		}

		// puntatore a membro di struttura
		else if (CompareStrings(exp_ptr,"->",2)) {
			tok_ptr->id=MEMBER_SELECTION_POINTER;
			exp_ptr+=2;
		}

		// incremento
		else if (CompareStrings(exp_ptr,"++",2)) {
			if (tok_ptr!=expr_tokens_b) {

				if ( tok_ptr > expr_tokens_b+1 &&
					(tok_ptr-1)->id == C_PARENTHESES ) {
					if (IsTypeCastToken(tok_ptr-2))
						tok_ptr->id=PREFIX_INCREMENT;
					else
						tok_ptr->id=POSTFIX_INCREMENT;
					exp_ptr+=2;
					goto skip;
				}
				else if ((tok_ptr-1)->id==IDENTIFIER ||
					(tok_ptr-1)->id==STRING ||
					(tok_ptr-1)->id==CA_PARENTHESES) {
					tok_ptr->id=POSTFIX_INCREMENT;
					exp_ptr+=2;
					goto skip;
				}

			}
			tok_ptr->id=PREFIX_INCREMENT;
			exp_ptr+=2;
		}

		// decremento
		else if (CompareStrings(exp_ptr,"--",2)) {
			if (tok_ptr!=expr_tokens_b) {

				if ( tok_ptr > expr_tokens_b+1 &&
					(tok_ptr-1)->id == C_PARENTHESES ) {
					if (IsTypeCastToken(tok_ptr-2))
						tok_ptr->id=PREFIX_DECREMENT;
					else
						tok_ptr->id=POSTFIX_DECREMENT;
					exp_ptr+=2;
					goto skip;
				}
				else if ((tok_ptr-1)->id==IDENTIFIER ||
					(tok_ptr-1)->id==STRING ||
					(tok_ptr-1)->id==CA_PARENTHESES) {
					tok_ptr->id=POSTFIX_DECREMENT;
					exp_ptr+=2;
					goto skip;
				}

			}
			tok_ptr->id=PREFIX_DECREMENT;
			exp_ptr+=2;
		}

		// assegnazione composta
		else if (CompareStrings(exp_ptr,"*=",2)) {
			tok_ptr->id=MULTIPLICATION_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"/=",2)) {
			tok_ptr->id=DIVISION_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"%=",2)) {
			tok_ptr->id=MODULUS_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"+=",2)) {
			tok_ptr->id=ADDITION_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"-=",2)) {
			tok_ptr->id=SUBTRACTION_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"<<=",3)) {
			tok_ptr->id=LEFT_SHIFT_ASSIGNMENT;
			exp_ptr+=3;
		}
		else if (CompareStrings(exp_ptr,">>=",3)) {
			tok_ptr->id=RIGHT_SHIFT_ASSIGNMENT;
			exp_ptr+=3;
		}
		else if (CompareStrings(exp_ptr,"&=",2)) {
			tok_ptr->id=BITWISE_AND_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"|=",2)) {
			tok_ptr->id=BITWISE_INCLUSIVE_OR_ASSIGNMENT;
			exp_ptr+=2;
		}
		else if (CompareStrings(exp_ptr,"^=",2)) {
			tok_ptr->id=BITWISE_EXCLUSIVE_OR_ASSIGNMENT;
			exp_ptr+=2;
		}

		// AND logico
		else if (CompareStrings(exp_ptr,"&&",2)) {
			tok_ptr->id=LOGICAL_AND;
			exp_ptr+=2;
		}

		// indirizzo di
		else if (CompareStrings(exp_ptr,"&",1)) {
			if (tok_ptr!=expr_tokens_b) {

				if ( tok_ptr > expr_tokens_b+1 &&
					(tok_ptr-1)->id == C_PARENTHESES ) {
					if (IsTypeCastToken(tok_ptr-2))
						tok_ptr->id=ADDRESS_OF;
					else
						tok_ptr->id=BITWISE_AND;
					exp_ptr+=1;
					goto skip;
				}
				else if ((tok_ptr-1)->id==IDENTIFIER ||
					(tok_ptr-1)->id==STRING ||
					(tok_ptr-1)->id==CA_PARENTHESES ||
					(tok_ptr-1)->id==POSTFIX_INCREMENT ||
					(tok_ptr-1)->id==POSTFIX_DECREMENT) {
					tok_ptr->id=BITWISE_AND;
					exp_ptr+=1;
					goto skip;
				}

			}
			tok_ptr->id=ADDRESS_OF;
			exp_ptr+=1;
		}

		// dereference / moltiplicazione
		else if (CompareStrings(exp_ptr,"*",1)) {
			if (tok_ptr!=expr_tokens_b) {

				if ( tok_ptr > expr_tokens_b+1 &&
					(tok_ptr-1)->id == C_PARENTHESES ) {
					if (IsTypeCastToken(tok_ptr-2))
						tok_ptr->id=DEREFERENCE;
					else
						tok_ptr->id=MULTIPLICATION;
					exp_ptr+=1;
					goto skip;
				}
				else if ((tok_ptr-1)->id==IDENTIFIER ||
					(tok_ptr-1)->id==STRING ||
					(tok_ptr-1)->id==CA_PARENTHESES ||
					(tok_ptr-1)->id==POSTFIX_INCREMENT ||
					(tok_ptr-1)->id==POSTFIX_DECREMENT) {
					tok_ptr->id=MULTIPLICATION;
					exp_ptr+=1;
					goto skip;
				}

			}
			tok_ptr->id=DEREFERENCE;
			exp_ptr+=1;
		}

		// negazione / sottrazione
		else if (CompareStrings(exp_ptr,"-",1)) {
			if (tok_ptr!=expr_tokens_b) {

				if ( tok_ptr > expr_tokens_b+1 &&
					(tok_ptr-1)->id == C_PARENTHESES ) {
					if (IsTypeCastToken(tok_ptr-2))
						tok_ptr->id=ARITHMETIC_NEGATION_UNARY;
					else
						tok_ptr->id=SUBTRACTION;
					exp_ptr+=1;
					goto skip;
				}
				else if ((tok_ptr-1)->id==IDENTIFIER ||
					(tok_ptr-1)->id==STRING ||
					(tok_ptr-1)->id==CA_PARENTHESES ||
					(tok_ptr-1)->id==POSTFIX_INCREMENT ||
					(tok_ptr-1)->id==POSTFIX_DECREMENT) {
					tok_ptr->id=SUBTRACTION;
					exp_ptr+=1;
					goto skip;
				}

			}
			tok_ptr->id=ARITHMETIC_NEGATION_UNARY;
			exp_ptr+=1;
		}

		// plus / addizione
		else if (CompareStrings(exp_ptr,"+",1)) {
			if (tok_ptr!=expr_tokens_b) {

				if ( tok_ptr > expr_tokens_b+1 &&
					(tok_ptr-1)->id == C_PARENTHESES ) {
					if (IsTypeCastToken(tok_ptr-2))
						tok_ptr->id=UNARY_PLUS;
					else
						tok_ptr->id=ADDITION;
					exp_ptr+=1;
					goto skip;
				}
				else if ((tok_ptr-1)->id==IDENTIFIER ||
					(tok_ptr-1)->id==STRING ||
					(tok_ptr-1)->id==CA_PARENTHESES ||
					(tok_ptr-1)->id==POSTFIX_INCREMENT ||
					(tok_ptr-1)->id==POSTFIX_DECREMENT) {
					tok_ptr->id=ADDITION;
					exp_ptr+=1;
					goto skip;
				}

			}
			tok_ptr->id=UNARY_PLUS;
			exp_ptr+=1;
		}

		// complemento di uno
		else if (CompareStrings(exp_ptr,"~",1)) {
			tok_ptr->id=BITWISE_COMPLEMENT;
			exp_ptr++;
		}

		// disuguaglianza
		else if (CompareStrings(exp_ptr,"!=",2)) {
			tok_ptr->id=INEQUALITY;
			exp_ptr+=2;
		}

		// not logico
		else if (CompareStrings(exp_ptr,"!",1)) {
			tok_ptr->id=LOGICAL_NOT;
			exp_ptr++;
		}

		// divisione
		else if (CompareStrings(exp_ptr,"/",1)) {
			tok_ptr->id=DIVISION;
			exp_ptr++;
		}

		// modulo
		else if (CompareStrings(exp_ptr,"%",1)) {
			tok_ptr->id=REMAINDER;
			exp_ptr++;
		}

		// slittamento a sinistra
		else if (CompareStrings(exp_ptr,"<<",2)) {
			tok_ptr->id=LEFT_SHIFT;
			exp_ptr+=2;
		}

		// slittamento a destra
		else if (CompareStrings(exp_ptr,">>",2)) {
			tok_ptr->id=RIGHT_SHIFT;
			exp_ptr+=2;
		}

		// minore o uguale a
		else if (CompareStrings(exp_ptr,"<=",2)) {
			tok_ptr->id=LESS_THAN_OR_EQUAL_TO;
			exp_ptr+=2;
		}

		// maggiore o uguale a
		else if (CompareStrings(exp_ptr,">=",2)) {
			tok_ptr->id=GREATER_THAN_OR_EQUAL_TO;
			exp_ptr+=2;
		}

		// minore di
		else if (CompareStrings(exp_ptr,"<",1)) {
			tok_ptr->id=LESS_THAN;
			exp_ptr++;
		}

		// maggiore di
		else if (CompareStrings(exp_ptr,">",1)) {
			tok_ptr->id=GREATER_THAN;
			exp_ptr++;
		}

		// uguaglianza
		else if (CompareStrings(exp_ptr,"==",2)) {
			tok_ptr->id=EQUALITY;
			exp_ptr+=2;
		}

		// OR logico
		else if (CompareStrings(exp_ptr,"||",2)) {
			tok_ptr->id=LOGICAL_OR;
			exp_ptr+=2;
		}

		// XOR su bit
		else if (CompareStrings(exp_ptr,"^",1)) {
			tok_ptr->id=BITWISE_EXCLUSIVE_OR;
			exp_ptr++;
		}

		// OR su bit
		else if (CompareStrings(exp_ptr,"|",1)) {
			tok_ptr->id=BITWISE_OR;
			exp_ptr++;
		}

		// condizionale
		else if (CompareStrings(exp_ptr,"?",1)) {
			tok_ptr->id=CONDITIONAL;
			exp_ptr++;
		}
		else if (CompareStrings(exp_ptr,":",1)) {
			tok_ptr->id=TWO_POINTS;
			exp_ptr++;
		}

		// assegnazione
		else if (CompareStrings(exp_ptr,"=",1)) {
			tok_ptr->id=ASSIGNMENT;
			exp_ptr++;
		}

		// virgola
		else if (CompareStrings(exp_ptr,",",1)) {
			tok_ptr->id=COMMA;
			exp_ptr++;
		}

		// carattere/stringa sconosciuto
		else {
			CompilerErrorOrWarning(4);
			return(1);
		}

skip:	// incrementa tok_ptr
		tok_ptr++;
	}

	// associa le parentesi aperte a quelle chiuse e associa i condizionali ai due punti
	for (tok_ptr=expr_tokens_b;
		tok_ptr->id!=EXPRESSION_END;
		tok_ptr++) {

			// associa le varie parentesi aperte a quelle chiuse
			if (tok_ptr->id==O_PARENTHESES ||
				tok_ptr->id==FUNCTION_CALL ||
				tok_ptr->id==ARRAY_SUBSCRIPT ||
				tok_ptr->id==CONDITIONAL) {

				parentheses_sp=0;
				for (i=1;;i++) {
					if (tok_ptr[i].id==EXPRESSION_END) {
						if (parentheses_sp>=0) {
							CompilerErrorOrWarning(3);
							return(1);
						}
						break;
					}
					else if (tok_ptr[i].id==O_PARENTHESES || tok_ptr[i].id==FUNCTION_CALL) {
						parentheses_stack[parentheses_sp]=0;
						parentheses_sp++;
					}
					else if (tok_ptr[i].id==ARRAY_SUBSCRIPT) {
						parentheses_stack[parentheses_sp]=1;
						parentheses_sp++;
					}
					else if (tok_ptr[i].id==CONDITIONAL) {
						parentheses_stack[parentheses_sp]=2;
						parentheses_sp++;
					}
					else if (tok_ptr[i].id==C_PARENTHESES) {
						parentheses_sp--;
						if (parentheses_sp<0)
							break;
						if (parentheses_stack[parentheses_sp]!=0) {
							CompilerErrorOrWarning(5);
							return(1);
						}
					}
					else if (tok_ptr[i].id==CA_PARENTHESES) {
						parentheses_sp--;
						if (parentheses_sp<0)
							break;
						if (parentheses_stack[parentheses_sp]!=1) {
							CompilerErrorOrWarning(6);
							return(1);
						}
					}
					else if (tok_ptr[i].id==TWO_POINTS) {
						parentheses_sp--;
						if (parentheses_sp<0)
							break;
						if (parentheses_stack[parentheses_sp]!=2) {
							CompilerErrorOrWarning(7);
							return(1);
						}
					}
				}
				if (parentheses_sp<0) {
					if (tok_ptr->id==O_PARENTHESES || tok_ptr->id==FUNCTION_CALL) {
						if (tok_ptr[i].id==C_PARENTHESES) {
							tok_ptr->ptr0=&tok_ptr[i];
							tok_ptr[i].ptr0=tok_ptr;
						}
						else {
							CompilerErrorOrWarning(5);
							return(1);
						}
					}
					else if (tok_ptr->id==ARRAY_SUBSCRIPT) {
						if (tok_ptr[i].id==CA_PARENTHESES) {
							tok_ptr->ptr0=&tok_ptr[i];
							tok_ptr[i].ptr0=tok_ptr;
						}
						else {
							CompilerErrorOrWarning(6);
							return(1);
						}
					}
					else if (tok_ptr->id==CONDITIONAL) {
						if (tok_ptr[i].id==TWO_POINTS) {
							tok_ptr->ptr0=&tok_ptr[i];
							tok_ptr[i].ptr0=tok_ptr;
						}
						else {
							CompilerErrorOrWarning(7);
							return(1);
						}
					}
				}
				else
					return(1);
			}

			// controlla che le parentesi chiuse siano state associate
			else if (tok_ptr->id==C_PARENTHESES ||
				tok_ptr->id==CA_PARENTHESES ||
				tok_ptr->id==TWO_POINTS) {
				if (!tok_ptr->ptr0) {
					if (tok_ptr->id==C_PARENTHESES)
						CompilerErrorOrWarning(5);
					else if (tok_ptr->id==CA_PARENTHESES)
						CompilerErrorOrWarning(6);
					else
						CompilerErrorOrWarning(7);
					return(1);
				}
			}

		}

	// typecast e sizeof
	for (tok_ptr=expr_tokens_b;
		tok_ptr->id!=EXPRESSION_END;
		tok_ptr++) {

			if (tok_ptr->id==O_PARENTHESES) {
				token=(token_t*)tok_ptr->ptr0+1;
				if (token->id==O_PARENTHESES ||
					token->id==IDENTIFIER ||
					token->id==STRING ||
					( (token->id & 0xff00) == 0x0100)) {
					tok_ptr->id=TYPE_CAST;
					for (i=1;&tok_ptr[i]<token-1;i++) {
						if (tok_ptr[i].id==IDENTIFIER ||
							tok_ptr[i].id==DEREFERENCE) {
							tok_ptr[i].tok_taken=1;
						}
						else if (tok_ptr[i].id==MULTIPLICATION) {
							tok_ptr[i].id=DEREFERENCE;
							tok_ptr[i].tok_taken=1;
						}
						else {
							CompilerErrorOrWarning(8);
							return(1);
						}
					}
				}
			}

			if (tok_ptr->id==SIZE_OF_TYPE) {
				if ( (tok_ptr+1)->id!=O_PARENTHESES ) {
					CompilerErrorOrWarning(9);
					return(1);
				}
				else {
					token=(token_t*)(tok_ptr+1)->ptr0;
					for (i=2;&tok_ptr[i]<token;i++) {
						if (tok_ptr[i].id==IDENTIFIER ||
							tok_ptr[i].id==DEREFERENCE) {
							tok_ptr[i].tok_taken=1;
						}
						else if (tok_ptr[i].id==MULTIPLICATION) {
							tok_ptr[i].id=DEREFERENCE;
							tok_ptr[i].tok_taken=1;
						}
						else if (tok_ptr[i].id==ARRAY_SUBSCRIPT) {
							j=(token_t*)tok_ptr[i].ptr0-expr_tokens_b;
							for (;i<=j;i++)
								tok_ptr[i].tok_taken=1;
						}
						else {
							CompilerErrorOrWarning(10);
							return(1);
						}
					}
				}
			}

		}

	// ritorna
	return(0);
}

//=================
//IsIdentfString
//
//Restituisce il numero di caratteri di identificatore consecutivi
//=================
int IsIdentfString (char *pointer)
{
	int		i,t;

	// controlla se l'identificatore è un numero
	if (*pointer>='0' && *pointer<='9')
		t=1;
	else
		t=0;

	// controlla la stringa
	for (i=0;;i++) {
		if ( !(pointer[i]>='a' && pointer[i]<='z') &&
			!(pointer[i]>='A' && pointer[i]<='Z') &&
			!(pointer[i]>='0' && pointer[i]<='9') &&
			pointer[i]!='_' ) {

			if (pointer[i]=='.' && t)
				continue;

			return(i);
		}
	}
}

//=================
//IsString
//
//Restituisce il numero di caratteri di stringa consecutivi
//=================
int IsString (char *pointer)
{
	int		i,j,k;

	// controlla che pointer punti effettivamente ad una stringa
	if (*pointer=='\'')
		j=0;
	else if (*pointer=='"')
		j=1;
	else
		return(0);

	// controlla la stringa
	k=-1;
	for (i=1;;i++) {
		if (pointer[i]=='\\' && pointer[i+1]=='\\') {
			k=i++ +1;
			continue;
		}
		if (pointer[i]=='\'' && !j) {
			if (pointer[i-1]!='\\' || k==i-1)
				break;
		}
		else if (pointer[i]=='"' && j) {
			if (pointer[i-1]!='\\' || k==i-1)
				break;
		}
		else if (!pointer[i])
			return(-1);
	}

	// ritorna
	return(i+1);
}

//=================
//ConsolePrintf
//
//Stampa un messaggio nella console
//=================
void ConsolePrintf (char *string, ...)
{
	va_list			argptr;
	char			text[1024];
	int				len;

	// controlla se è possibile continuare
	if (console_ptr-console>=(int)(console_dim*compiler_memory_factor))
		return;
	if (current_console_message>=max_console_messages)
		return;

	// acquisisce i parametri
	va_start (argptr,string);
	vsprintf (text,string,argptr);
	va_end (argptr);

	// stampa il messaggio
	len=strlen(text);
	if (console_ptr-console+len<=(int)(console_dim*compiler_memory_factor)) {
		strcpy(console_ptr,text);
		console_messages[current_console_message]=console_ptr;
	}

	// incrementa i puntatori
	console_ptr+=len+1;
	current_console_message++;

	// imposta un carattere null in error_files
	*error_files_ptr=0;
	error_files_ptr++;

	// ritorna
	return;
}

//=================
//MakeExprOrderedOpList
//
//Ordina i diversi operatori dal meno prioritario al più prioritario
//=================
int MakeExprOrderedOpList (int buffer)
{
	operator_t		*operators_b=(operator_t*)((char*)operators+buffer*(int)(single_operators_buffer*compiler_memory_factor));
	token_t			*expr_tokens_b=(token_t*)((char *)expr_tokens+buffer*(int)(single_expr_tokens_buffer*compiler_memory_factor));

	operator_t		*op_ptr=operators_b;
	token_t			*tok_ptr=expr_tokens_b;

	int				end_id,i,j,k,l,m;
	int				associativity;
	int				parentheses_stack[2][parentheses_stack_dim];
	int				parentheses_sp;
	int				first_id,last_id;

	char			*pointer;
	int				type;
	char			ident[256];
	int				ret;
	identifier_t	*stype;
	int				typesize;

	// cerca l'id dell'ultimo token
	for (end_id=0;;end_id++)
		if (tok_ptr[end_id].id==EXPRESSION_END)
			break;

	// imposta lo stack delle parentesi
	parentheses_stack[0][0]=0;
	parentheses_stack[1][0]=end_id;
	parentheses_sp=1;

	// ordina i diversi operatori dal meno prioritario al più prioritario
	while (1) {

		parentheses_sp--;
		if (parentheses_sp<0)
			break;
		first_id=parentheses_stack[0][parentheses_sp];
		last_id=parentheses_stack[1][parentheses_sp];

		for (i=0x0f00;i>=0;i-=0x0100) {
			if (i==0x0100 || i==0x0c00 || i==0x0d00 ||
				i==0x0e00)	// 0e00=comma (associata da destra a sinistra per motivi di stack)
				associativity=1;
			else
				associativity=0;

			if (associativity)
				j=first_id;
			else
				j=last_id;
			while (1) {

				if ( (tok_ptr[j].id & 0xff00) == i &&
					!tok_ptr[j].tok_taken ) {
					op_ptr->id=tok_ptr[j].id;
					op_ptr->tok_id=j;
					op_ptr++;
					if (tok_ptr[j].id == COMMA)
						tok_ptr[j].tok_taken=1;
				}

				k=0;
				if (associativity && ( tok_ptr[j].id == O_PARENTHESES ||
					tok_ptr[j].id == FUNCTION_CALL ||
					tok_ptr[j].id == ARRAY_SUBSCRIPT ||
					tok_ptr[j].id == CONDITIONAL ) ) {

					j=(token_t*)tok_ptr[j].ptr0-tok_ptr;

				}
				else if (!associativity && ( tok_ptr[j].id == C_PARENTHESES ||
					tok_ptr[j].id == CA_PARENTHESES ||
					tok_ptr[j].id == TWO_POINTS ) ) {

					if (i==0x0f00)
						parentheses_stack[1][parentheses_sp]=j-1;

					j=(token_t*)tok_ptr[j].ptr0-tok_ptr;

					if (i==0x0f00) {
						parentheses_stack[0][parentheses_sp]=j+1;
						parentheses_sp++;
					}

					if (tok_ptr[j].id == ARRAY_SUBSCRIPT ||
						tok_ptr[j].id == FUNCTION_CALL)
						k=1;

					if (i==0x0f00 && tok_ptr[j].id==FUNCTION_CALL) {
						if (parentheses_stack[0][parentheses_sp-1] >
							parentheses_stack[1][parentheses_sp-1]) {
							parentheses_sp--;
							tok_ptr[j].func_call_has_one_param=0;
						}
						else {
							tok_ptr[j].func_call_has_one_param=1;
							m=0;
							for (l=parentheses_stack[0][parentheses_sp-1];
								l<=parentheses_stack[1][parentheses_sp-1];
								l++) {
									if (tok_ptr[l].id == O_PARENTHESES ||
										tok_ptr[l].id == FUNCTION_CALL ||
										tok_ptr[l].id == ARRAY_SUBSCRIPT ||
										tok_ptr[l].id == CONDITIONAL)
											l=(token_t*)tok_ptr[l].ptr0-tok_ptr;
									else if (tok_ptr[l].id == COMMA) {
										tok_ptr[j].func_call_has_one_param=0;
										tok_ptr[l].comma_number=m++;
										tok_ptr[l].comma_func_tok=j;
									}
								}
						}
					}

				}

				if (associativity) {
					j++;
					if (j>last_id)
						break;
				}
				else if (!k) {
					j--;
					if (j<first_id)
						break;
				}

			}

		}

	}
	op_ptr->id=EXPRESSION_END;

	// associa ciascun operatore
	for (op_ptr--;op_ptr>=operators_b;op_ptr--) {

		// left
		if ( (op_ptr->id & 0xff00) != 0x0100 ) {
			i=op_ptr->tok_id;
			if (op_ptr->id == COMMA && tok_ptr[i].comma_number>0)
				op_ptr->left_tok=-1;
			else {
				for (i--;i>=0;i--) {
					if (!tok_ptr[i].tok_taken) {
						if (tok_ptr[i].id == IDENTIFIER ||
							tok_ptr[i].id == STRING ||
							(tok_ptr[i].id & 0xff00) != 0xff00)
							break;
					}
				}
				if (i>=0) {
					tok_ptr[i].tok_taken=1;
					op_ptr->left_tok=i;
				}
				else {
					CompilerErrorOrWarning(11);
					return(1);
				}
			}
		}
		else
			op_ptr->left_tok=-1;

		// right
		if (op_ptr->id!=POSTFIX_INCREMENT &&
			op_ptr->id!=POSTFIX_DECREMENT &&
			op_ptr->id!=TYPE_CAST &&
			op_ptr->id!=SIZE_OF_TYPE) {
			i=op_ptr->tok_id;
			if (op_ptr->id==FUNCTION_CALL && !tok_ptr[i].func_call_has_one_param)
				op_ptr->right_tok=-1;
			else if (op_ptr->id==COMMA && tok_ptr[i].comma_number==-1)
				op_ptr->right_tok=-1;
			else {
				for (i++;i<end_id;i++) {
					if (!tok_ptr[i].tok_taken) {
						if (tok_ptr[i].id == IDENTIFIER ||
							tok_ptr[i].id == STRING ||
							(tok_ptr[i].id & 0xff00) != 0xff00)
							break;
					}
				}
				if (i<end_id) {
					tok_ptr[i].tok_taken=1;
					op_ptr->right_tok=i;
				}
				else {
					CompilerErrorOrWarning(11);
					return(1);
				}
			}
		}
		else
			op_ptr->right_tok=-1;

		// third
		if (op_ptr->id==CONDITIONAL || op_ptr->id==TYPE_CAST) {
			i=(token_t*)tok_ptr[op_ptr->tok_id].ptr0-tok_ptr;
			for (i++;i<end_id;i++) {
				if (!tok_ptr[i].tok_taken) {
					if (tok_ptr[i].id == IDENTIFIER ||
						tok_ptr[i].id == STRING ||
						(tok_ptr[i].id & 0xff00) != 0xff00)
						break;
				}
			}
			if (i<end_id) {
				tok_ptr[i].tok_taken=1;
				op_ptr->third_tok=i;
			}
			else {
				CompilerErrorOrWarning(11);
				return(1);
			}
		}
		else
			op_ptr->third_tok=-1;

	}

	// controlla che le associazioni siano corrette
	for (op_ptr=operators_b;op_ptr->id!=EXPRESSION_END;op_ptr++) {
		for (i=0;&operators_b[i]!=op_ptr;i++) {
			j=operators_b[i].tok_id;
			if (op_ptr->left_tok==j) {
				CompilerErrorOrWarning(12);
				return(1);
			}
			if (op_ptr->right_tok==j) {
				CompilerErrorOrWarning(12);
				return(1);
			}
			if (op_ptr->third_tok==j) {
				CompilerErrorOrWarning(12);
				return(1);
			}
		}
	}

	// controlla se qualche operatore, stringa o identificatore non è stato associato
	result_id=-1;
	for (i=0;i<end_id;i++) {
		if (!tok_ptr[i].tok_taken) {
			if (tok_ptr[i].id == IDENTIFIER ||
				tok_ptr[i].id == STRING ||
				(tok_ptr[i].id & 0xff00) != 0xff00) {
				if (result_id!=-1) {
					CompilerErrorOrWarning(12);
					return(1);
				}
				else
					result_id=i;
			}
		}
	}
	if (result_id==-1) {
		CompilerErrorOrWarning(12);
		return(1);
	}

	// risolve i sizeof
	for (i=0;i<end_id;i++) {
		if (tok_ptr[i].id == SIZE_OF_TYPE) {

			if (tok_ptr[i+2].id != IDENTIFIER) {
				CompilerErrorOrWarning(13);
				return(1);
			}

			j=i+2;
			k=0; // per evitare contraddizioni nei tipi

			type=-1;
			stype=NULL;
			while (1) {
				pointer=tok_ptr[j].ptr0;

				// pone in ret il numero di caratteri dell'identificatore
				ret=strlen(pointer);

				// interpreta l'identificatore
				if (ret == 4 && CompareStrings(pointer,"void",4)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=VOID_TYPE;
				}
				else if (ret == 4 && CompareStrings(pointer,"char",4)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=CHAR;
				}
				else if (ret == 3 && CompareStrings(pointer,"int",3)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					if (k & 1)
						type=SHORT;
					else
						type=INT;
				}
				else if (ret == 5 && CompareStrings(pointer,"float",5)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					if (k & 2)
						type=DOUBLE;
					else
						type=FLOAT;
				}
				else if (ret == 6 && CompareStrings(pointer,"double",6)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=DOUBLE;
				}
				else if (ret == 5 && CompareStrings(pointer,"short",5)) {
					if (k & 2) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=1;
					if (type==INT)
						type=SHORT;
				}
				else if (ret == 4 && CompareStrings(pointer,"long",4)) {
					if (k & 1) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=2;
					if (type==FLOAT)
						type=DOUBLE;
				}
				else if (ret == 6 && CompareStrings(pointer,"signed",6)) {
					if (k & 8) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=4;
				}
				else if (ret == 8 && CompareStrings(pointer,"unsigned",8)) {
					if (k & 4) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=8;
				}
				else if (ret != 5 || !CompareStrings(pointer,"const",5)) {
					if (k || type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=STRUCTURE_TYPE;

					// interpreta l'identificatore
					ret=strlen(pointer);
					if (ret>=identifier_max_len) {
						m=identifier_max_len-1;
						CompilerErrorOrWarning(1);
					}
					else
						m=ret;

					memcpy(ident,pointer,m);
					ident[m]=0;

					for (l=0;l<cident_id;l++) {
						if ( ( (cident[l].type & 0xff0000) == STRUCT ||
							(cident[l].type & 0xff0000) == VARIABLE ||
							(cident[l].type & 0xff0000) == FUNCTION ) &&
							!strcmp(ident,cident[l].id) )
							break;
					}
					if (l==cident_id) {
						CompilerErrorOrWarning(2);
						return(1);
					}
					else
						stype=&cident[l];

				}

				if (tok_ptr[j+1].id == IDENTIFIER)
					j++;
				else if (type==-1) {
					if (k & 1)
						type=SHORT;
					else
						type=INT;
					break;
				}
				else
					break;
			}

			// parentesi tonda chiusa
			if (tok_ptr[j+1].id == C_PARENTHESES) {
				if (type==VOID_TYPE) {
					CompilerErrorOrWarning(17);
					return(1);
				}
				else if (type==CHAR)
					tok_ptr[i].sizeof_result=1;
				else if (type==SHORT)
					tok_ptr[i].sizeof_result=2;
				else if (type==INT)
					tok_ptr[i].sizeof_result=4;
				else if (type==FLOAT)
					tok_ptr[i].sizeof_result=4;
				else if (type==DOUBLE)
					tok_ptr[i].sizeof_result=8;
				else {
					if ( (stype->type & 0xff0000) == FUNCTION ) {
						CompilerErrorOrWarning(18);
						return(1);
					}
					else if ( (stype->type & 0xff0000) == STRUCT )
						tok_ptr[i].sizeof_result=stype->address;
					else if ( (stype->type & 0xff0000) == VARIABLE ) {
						if (stype->indirection)
							typesize=4;
						else {
							if ( (stype->type & 0xff00) == CHAR )
								typesize=1;
							else if ( (stype->type & 0xff00) == SHORT )
								typesize=2;
							else if ( (stype->type & 0xff00) == INT )
								typesize=4;
							else if ( (stype->type & 0xff00) == FLOAT )
								typesize=4;
							else if ( (stype->type & 0xff00) == DOUBLE )
								typesize=8;
							else if ( (stype->type & 0xffff) == STRUCTURE_TYPE)
								typesize=stype->struct_type->address;
						}
						if (stype->dimensions) {
							for (k=0;k<stype->dimensions;k++)
								typesize*=((int*)stype->pointer)[k];
						}
						tok_ptr[i].sizeof_result=typesize;
					}
				}
			}

			// dereference
			else if (tok_ptr[j+1].id == DEREFERENCE) {
				if (stype) {
					if ( (stype->type & 0xff0000) == FUNCTION ||
						(stype->type & 0xff0000) == VARIABLE) {
						CompilerErrorOrWarning(19);
						return(1);
					}
				}
				for (k=j+1;;k++) {
					if (tok_ptr[k].id != DEREFERENCE &&
						tok_ptr[k].id != C_PARENTHESES) {
						CompilerErrorOrWarning(13);
						return(1);
					}
					else if (tok_ptr[k].id == C_PARENTHESES)
						break;
				}
				tok_ptr[i].sizeof_result=4;
			}

			// array subscript
			else if (tok_ptr[j+1].id == ARRAY_SUBSCRIPT) {
				if (!stype) {
					CompilerErrorOrWarning(20);
					return(1);
				}
				else if ( (stype->type & 0xff0000) != VARIABLE) {
					CompilerErrorOrWarning(20);
					return(1);
				}
				l=0;
				k=j+1;
				while(1) {
					l++;
					k=(token_t*)tok_ptr[k].ptr0-expr_tokens_b+1;
					if (tok_ptr[k].id != ARRAY_SUBSCRIPT)
						break;
				}
				if (tok_ptr[k].id != C_PARENTHESES) {
					CompilerErrorOrWarning(13);
					return(1);
				}
				if (stype->dimensions<l) {
					CompilerErrorOrWarning(21);
					return(1);
				}
				if (stype->indirection)
					typesize=4;
				else {
					if ( (stype->type & 0xff00) == CHAR )
						typesize=1;
					else if ( (stype->type & 0xff00) == SHORT )
						typesize=2;
					else if ( (stype->type & 0xff00) == INT )
						typesize=4;
					else if ( (stype->type & 0xff00) == FLOAT )
						typesize=4;
					else if ( (stype->type & 0xff00) == DOUBLE )
						typesize=8;
					else if ( (stype->type & 0xffff) == STRUCTURE_TYPE)
						typesize=stype->struct_type->address;
				}
				for (k=l;k<stype->dimensions;k++)
					typesize*=((int*)stype->pointer)[k];
				tok_ptr[i].sizeof_result=typesize;
			}

		}
	}

	// interpreta i type cast
	for (i=0;i<end_id;i++) {
		if (tok_ptr[i].id == TYPE_CAST) {

			if (tok_ptr[i+1].id != IDENTIFIER) {
				CompilerErrorOrWarning(27);
				return(1);
			}

			j=i+1;
			k=0; // per evitare contraddizioni nei tipi

			type=-1;
			stype=NULL;
			while (1) {
				pointer=tok_ptr[j].ptr0;

				// pone in ret il numero di caratteri dell'identificatore
				ret=strlen(pointer);

				// interpreta l'identificatore
				if (ret == 4 && CompareStrings(pointer,"void",4)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=VOID_TYPE;
				}
				else if (ret == 4 && CompareStrings(pointer,"char",4)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=CHAR;
				}
				else if (ret == 3 && CompareStrings(pointer,"int",3)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					if (k & 1)
						type=SHORT;
					else
						type=INT;
				}
				else if (ret == 5 && CompareStrings(pointer,"float",5)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					if (k & 2)
						type=DOUBLE;
					else
						type=FLOAT;
				}
				else if (ret == 6 && CompareStrings(pointer,"double",6)) {
					if (type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					if (k & 2)
						type=LONG_DOUBLE;
					else
						type=DOUBLE;
				}
				else if (ret == 5 && CompareStrings(pointer,"short",5)) {
					if (k & 2) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=1;
					if (type==INT)
						type=SHORT;
				}
				else if (ret == 4 && CompareStrings(pointer,"long",4)) {
					if (k & 1) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					if (type==FLOAT)
						type=DOUBLE;
					else if (type==DOUBLE && !(k & 2))
						type=LONG_DOUBLE;
					k|=2;
				}
				else if (ret == 6 && CompareStrings(pointer,"signed",6)) {
					if (k & 8) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=4;
				}
				else if (ret == 8 && CompareStrings(pointer,"unsigned",8)) {
					if (k & 4) {
						CompilerErrorOrWarning(14);
						return(1);
					}
					if (type==STRUCTURE_TYPE) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					k|=8;
				}
				else if (ret != 5 || !CompareStrings(pointer,"const",5)) {
					if (k || type!=-1) {
						CompilerErrorOrWarning(16);
						return(1);
					}
					type=STRUCTURE_TYPE;

					// interpreta l'identificatore
					ret=strlen(pointer);
					if (ret>=identifier_max_len) {
						m=identifier_max_len-1;
						CompilerErrorOrWarning(1);
					}
					else
						m=ret;

					memcpy(ident,pointer,m);
					ident[m]=0;

					for (l=0;l<cident_id;l++) {
						if ( ( (cident[l].type & 0xff0000) == STRUCT ||
							(cident[l].type & 0xff0000) == VARIABLE ||
							(cident[l].type & 0xff0000) == FUNCTION ) &&
							!strcmp(ident,cident[l].id) )
							break;
					}
					if (l==cident_id) {
						CompilerErrorOrWarning(2);
						return(1);
					}
					else
						stype=&cident[l];

				}

				if (tok_ptr[j+1].id == IDENTIFIER)
					j++;
				else if (type==-1) {
					if (k & 1)
						type=SHORT;
					else
						type=INT;
					break;
				}
				else
					break;
			}

			// acquisisce il level of indirection
			l=0;
			j++;
			if (tok_ptr[j].id != C_PARENTHESES) {
				while (1) {
					if (tok_ptr[j].id == DEREFERENCE)
						l++;
					else if (tok_ptr[j].id == C_PARENTHESES)
						break;
					else {
						CompilerErrorOrWarning(27);
						return(1);
					}
					j++;
				}
			}

			// controlla che il type-cast non sia (void)
			if (type==VOID_TYPE && !l) {
				CompilerErrorOrWarning(28);
				return(1);
			}

			// imposta le informazioni definitive
			tok_ptr[i].typecast_stype=stype;
			tok_ptr[i].typecast_type=type;
			if ( (k & 8) && (type==CHAR || type==SHORT || type==INT) )
				tok_ptr[i].typecast_type|=1;
			tok_ptr[i].typecast_indir=l;

		}
	}

	// ritorna
	return(0);
}

//================
//CompilerErrorOrWarning
//
//Stampa un errore o un warning nella console
//================
char *errors[]={	//123456789[10]456789[20]456789[30]456789[40]456789[50]456789[60]
					{"eUnable to interpretate or order MiniC expression"},				// 0
					{"wIdentifier too long, it must be truncated when evaluated"},		// 1
					{"eUndeclared identifier in sizeof or type-cast operator"},			// 2
					{"eUnterminated line (string or parentheses error)"},				// 3
					{"eMiniC expression contains illegal character(s)"},				// 4
					{"eUnable to associate parentheses in MiniC expression"},			// 5
					{"eUnable to associate brackets in MiniC expression"},				// 6
					{"eUnable to associate conditional operator"},						// 7
					{"eType-cast operator contains an illegal token"},					// 8
					{"eSizeof operator requires parentheses"},							// 9
					{"eSizeof operator contains an illegal token"},						// 10
					{"eUnable to associate an operator in MiniC expression"},			// 11
					{"eUnable to interpretate association in MiniC expression"},		// 12
					{"eSizeof operator syntax error"},									// 13
					{"eTypes conflict in MiniC expression"},							// 14
					{"UNUSED"},															// 15
					{"eRedundancy of type specifiers in MiniC expression"},				// 16
					{"eSize of void is zero (not an allowed type)"},					// 17
					{"eSizeof operator not applicable to functions"},					// 18
					{"eSizeof operator requires a type between parentheses"},			// 19
					{"eSizeof operator requires a variable between parentheses"},		// 20
					{"eArray dimension error in sizeof operator argument"},				// 21
					{"eBraces association error in variable initializator"},			// 22
					{"eConstant expression is required (no variables or strings)"},		// 23
					{"eIllegal operator in constant expression, unable to resolve"},	// 24
					{"eToo many characters in constant"},								// 25
					{"eUnable to interpretate format of number"},						// 26
					{"eType-cast operator syntax error"},								// 27
					{"eType-cast allows pointer to void, not void as type"},			// 28
					{"wMinus applied to unsigned type, result still unsigned"},			// 29
					{"eOperand must be integral, floating-point not allowed"},			// 30
					{"wType conversion may cause a loss of data"},						// 31
					{"eIllegal type-cast of constant number"},							// 32
					{"wSuspicious escape sequences, unrecognized character(s)"},		// 33
					{"eUnterminated line in function declaration"},						// 34
					{"eIdentifier already used in variable declaration"},				// 35
					{"eArray must have at least one element"},							// 36
					{"eUnable to establish dimension in variable declaration"},			// 37
					{"eToo many initializers"},											// 38
					{"eSyntax error in variable declaration"},							// 39
					{"eBraces required when initializing this type of variable"},		// 40
					{"eMiniC does not allow initialization of data structures"},		// 41
					{"eIllegal storage class (structure member as static)"},			// 42
					{"eData structure has an illegal zero-sized array"},				// 43
					{"eUnable to initialize a data structure member"},					// 44
					{"eMultiple declaration for data structure member"},				// 45
					{"eIllegal storage class (function parameter as static)"},			// 46
					{"eIdentifier already used for function parameter"},				// 47
					{"eFirst dimension required for function parameter"},				// 48
					{"eUnable to initialize a function parameter"},						// 49
					{"eType identifier for data structure is already used"},			// 50
					{"eStructure x as type in structure x, illogical nesting"},			// 51
					{"eDeclaration in data structure missing ;"},						// 52
					{"eUnterminated line in data structure declaration"},				// 53
					{"eDeclaration in data structure missing {"},						// 54
					{"eData structure without type declares no symbols"},				// 55
					{"eData structure missing member declaration"},						// 56
					{"eData structure missing symbol declaration"},						// 57
					{"eVariable declaration missing initializator"},					// 58
					{"eError in variable initialization, incomprehensible syntax"},		// 59
					{"eNo dimension, illegal initialization with braces"},				// 60
					{"eIdentifier already used in function declaration"},				// 61
					{"eSyntax error in data structure member declaration"},				// 62
					{"eSyntax error in function declaration"},							// 63
					{"eSyntax error in function parameter declaration"},				// 64
					{"eUnable to establish dimension in member declaration"},			// 65
					{"eUnable to establish dimension in parameter declaration"},		// 66
					{"eBrackets in function declaration, syntax error"},				// 67
					{"eEqual sign in function declaration, syntax error"},				// 68
					{"eFunction declaration missing parameter"},						// 69
					{"eCharacters follow function declaration, syntax error"},			// 70
					{"eUnable to interpretate an array of voids"},						// 71
					{"eIdentifier already used in structure member declaration"},		// 72
					{"eIdentifier already used in function parameter declaration"},		// 73
					{"eIllegal use of void as function parameter"},						// 74
					{"eSubscript op: left argument is a number"},						// 75
					{"eSubscript op: left argument is not a pointer/array"},			// 76
					{"eSubscript op: illegal type of right argument"},					// 77
					{"wSubscript op: suspicious right argument type: float"},			// 78
					{"wSubscript op: suspicious right argument type: double"},			// 79
					{"wSubscript op: suspicious right argument type: ldouble"},			// 80
					{"eSubscript op: illegal right argument"},							// 81
					{"eSubscript op: size of void is zero"},							// 82
					{"eImpossible type conversion in function parameter"},				// 83
					{"eIllegal type conversion"},										// 84
					{"eExtra parameter in function call"},								// 85
					{"eToo few parameters in function call"},							// 86
					{"eUndeclared identifier: function should have a prototype"},		// 87
					{"eIllegal left argument of function call"},						// 88
					{"eCannot convert a number to a pointer in function call"},			// 89
					{"eValue of type void is not allowed"},								// 90
					{"eStructure required on left side of ."},							// 91
					{"eStructure member identifier required on right side of ."},		// 92
					{"ePointer to structure required on left side of ->"},				// 93
					{"eStructure member identifier required on right side of ->"},		// 94
					{"eLvalue required on left side of postfix increment"},				// 95
					{"eLvalue required on left side of postfix decrement"},				// 96
					{"eCannot modify a const object"},									// 97
					{"eLvalue required on right side of prefix increment"},				// 98
					{"eLvalue required on right side of prefix decrement"},				// 99
					{"eIllegal type conversion without type-cast"},						// 100
					{"eMust take address of a memory location"},						// 101
					{"ePointer required on right side of indirection operator"},		// 102
					{"eIllegal variable on right side of negation operator"},			// 103
					{"eIllegal variable on right side of plus operator"},				// 104
					{"eVariable of illegal type on right side of ~ operator"},			// 105
					{"eIllegal variable on right side of ~ operator"},					// 106
					{"eValue of type void on right side of ! operator"},				// 107
					{"eStructure on right side of ! operator"},							// 108
					{"eValue of type void is not convertible with a type cast"},		// 109
					{"ePointers cannot be operands of multiplications"},				// 110
					{"eValue of type void as operand in multiplication"},				// 111
					{"eData structures cannot be operands of multiplications"},			// 112
					{"ePointers cannot be operands of divisions"},						// 113
					{"eValue of type void as operand in division"},						// 114
					{"eData structures cannot be operands of divisions"},				// 115
					{"ePointers cannot be operands of remainder operators"},			// 116
					{"eValue of type void as operand of remainder operator"},			// 117
					{"eData structures cannot be operands of remainder operators"},		// 118
					{"eOperands of remainder operator must be integral, no fp"},		// 119
					{"eSize of void is zero, unable to add or subtract pointers"},		// 120
					{"eData structures cannot be operands of additions"},				// 121
					{"eData structures cannot be operands of subtractions"},			// 122
					{"eValue of type void as operand in addition"},						// 123
					{"eValue of type void as operand in subtraction"},					// 124
					{"eCannot add two pointers"},										// 125
					{"eCannot subtract a pointer to a number"},							// 126
					{"ePointers of subtraction are not compatible"},					// 127
					{"eUnable to add a floating point to a pointer"},					// 128
					{"eUnable to subtract a floating point to a pointer"},				// 129
					{"ePointers cannot be operands of left shifts"},					// 130
					{"ePointers cannot be operands of right shifts"},					// 131
					{"eData structures cannot be operands of left shifts"},				// 132
					{"eData structures cannot be operands of rights shifts"},			// 133
					{"eOperands of left shift operator must be integral, no fp"},		// 134
					{"eOperands of right shift operator must be integral, no fp"},		// 135
					{"eValue of type void as operand in left shift"},					// 136
					{"eValue of type void as operand in right shift"},					// 137
					{"eValue of type void as operand in relational operation"},			// 138
					{"eValue of type void as operand in equality operation"},			// 139
					{"ePointers cannot be operands of bitwise operations"},				// 140
					{"eData structures cannot be operands of bitwise operations"},		// 141
					{"eOperands of bitwise operators must be integral, no fp"},			// 142
					{"eValue of type void as operand in bitwise operation"},			// 143
					{"eData structures cannot be operands of logical operations"},		// 144
					{"eValue of type void as operand in logical operation"},			// 145
					{"eIllegal pointer comparison"},									// 146
					{"eData structures cannot be operands of relational operations"},	// 147
					{"eData structures cannot be operands of equality operations"},		// 148
					{"eCompound assignment: r-value on left side of operator"},			// 149
					{"eCompound assignment: constant on left side of operator"},		// 150
					{"eCompound assignment: pointer as operand"},						// 151
					{"eCompound assignment: data structure as operand"},				// 152
					{"eCompound assignment: value of type void as operand"},			// 153
					{"eCompound assignment: operands must be integral, no fp"},			// 154
					{"eCompound assignment: size of void is zero"},						// 155
					{"eCompound assignment: cannot add two pointers"},					// 156
					{"eCompound assignment: cannot add/subtract a fp to a pointer"},	// 157
					{"eCompound assignment: pointers of subtraction incompatible"},		// 158
					{"eCompound assignment: cannot subtract a pointer to a number"},	// 159
					{"eAssignment: constant of pointer assignment may be only zero"},	// 160
					{"eAssignment: r-value on left side of equal operator"},			// 161
					{"eAssignment: constant on left side of equal operator"},			// 162
					{"eAssignment: pointers are not compatible"},						// 163
					{"eAssignment: value of type void as operand of equal operator"},	// 164
					{"eAssignment: structures on left and right sides incompatible"},	// 165
					{"eComma internal error *** compiler bug detected ***"},			// 166
					{"eCannot convert a number to a structure in function call"},		// 167
					{"eConditional: data structure on left side of ?"},					// 168
					{"eConditional: value of type void on left side of ?"},				// 169
					{"eConditional: second and third operands are incompatible"},		// 170
					{"eUndeclared identifier"},											// 171
					{"eWrong number of parameters in function call"},					// 172
					{"eCompound assignment: bad right operand"},						// 173
					{"eCompiler reserved word used as identifier"},						// 174
					{"fUnable to allocate compiler working memory"},					// 175
					{"fIdentifiers memory *** OUT OF MEMORY ***"},						// 176
					{"fIdentifiers auxiliary memory *** OUT OF MEMORY ***"},			// 177
					{"fPSI instructions memory *** OUT OF MEMORY ***"},					// 178
					{"fFinal code memory *** OUT OF MEMORY ***"},						// 179
					{"fRelocation directives memory *** OUT OF MEMORY ***"},			// 180
					{"fNames memory *** OUT OF MEMORY ***"},							// 181
					{"fFunctions memory *** OUT OF MEMORY ***"},						// 182
					{"fExternal functions memory *** OUT OF MEMORY ***"},				// 183
					{"fStrings memory *** OUT OF MEMORY ***"},							// 184
					{"fPreprocessor stack memory *** OUT OF MEMORY ***"},				// 185
					{"fSource file not found"},											// 186
					{"fUnable to read a source file line"},								// 187
					{"fUnexpected end of file in #if conditional"},						// 188
					{"fUnexpected end of file in #ifdef conditional"},					// 189
					{"fUnexpected end of file in #ifndef conditional"},					// 190
					{"fUnexpected end of file in #else conditional"},					// 191
					{"e#define preprocessor directive needs an identifier"},			// 192
					{"wRedefinition is not identical"},									// 193
					{"e#undef preprocessor directive needs an identifier"},				// 194
					{"wUnable to remove a definition not declared before"},				// 195
					{"fBad file name format in #include preprocessor directive"},		// 196
					{"wExtra characters in #include preprocessor directive"},			// 197
					{"fHeader file not found"},											// 198
					{"fBad #ifdef preprocessor directive syntax"},						// 199
					{"wExtra characters in #ifdef preprocessor directive"},				// 200
					{"fBad #ifndef preprocessor directive syntax"},						// 201
					{"wExtra characters in #ifndef preprocessor directive"},			// 202
					{"fMisplaced #else preprocessor directive"},						// 203
					{"wExtra characters in #else preprocessor directive"},				// 204
					{"fMisplaced #endif preprocessor directive"},						// 205
					{"wExtra characters in #endif preprocessor directive"},				// 206
					{"fBad #if preprocessor directive syntax"},							// 207
					{"fUnknown preprocessor directive"},								// 208
					{"f#include nesting level is 99 deep; compiler limit reached"},		// 209
					{"fMain stack memory *** OUT OF MEMORY ***"},						// 210
					{"fMain stack auxiliary memory *** OUT OF MEMORY ***"},				// 211
					{"eUnterminated string or character constant"},						// 212
					{"eFor statement missing )"},										// 213
					{"eFor statement missing ("},										// 214
					{"eWhile statement missing )"},										// 215
					{"eWhile statement missing ("},										// 216
					{"eStatement too long"},											// 217
					{"eIf statement missing ("},										// 218
					{"eIf statement missing )"},										// 219
					{"eFor statement missing ;"},										// 220
					{"eUnexpected end of file, statement missing ;"},					// 221
					{"eFunction declaration missing )"},								// 222
					{"eFor cond-expression: data structure not allowed"},				// 223
					{"eFor cond-expression: value of type void not allowed"},			// 224
					{"eWhile expression: data structure not allowed"},					// 225
					{"eWhile expression: value of type void not allowed"},				// 226
					{"eIf expression: data structure not allowed"},						// 227
					{"eIf expression: value of type void not allowed"},					// 228
					{"eIllegal declaration"},											// 229
					{"eMisplaced else"},												// 230
					{"eDo statement must have while"},									// 231
					{"eIllegal label declaration"},										// 232
					{"wLabel too long, it must be truncated"},							// 233
					{"eUnexpected }"},													// 234
					{"fIdentifiers auxiliary memory 2 *** OUT OF MEMORY ***"},			// 235
					{"wStatic keyword not implemented; it has no effect"},				// 236
					{"eInitializer too long"},											// 237
					{"e#align preprocessor directive needs a parameter"},				// 238
					{"e#align parameter may be: 1 2 4 8 16 default"},					// 239
					{"eFunction declaration terminated incorrectly"},					// 240
					{"eType mismatch in function declaration"},							// 241
					{"eMultiple function declaration"},									// 242
					{"eBreak statement missing ;"},										// 243
					{"eMisplaced break"},												// 244
					{"eContinue statement missing ;"},									// 245
					{"eMisplaced continue"},											// 246
					{"fLabels memory *** OUT OF MEMORY ***"},							// 247
					{"eGoto statement missing label"},									// 248
					{"eGoto statement missing ;"},										// 249
					{"fLabels_ptrs memory *** OUT OF MEMORY ***"},						// 250
					{"eUndefined label"},												// 251
					{"eMultiple label declaration"},									// 252
					{"eReturn statement missing ;"},									// 253
					{"eFunction must return a value"},									// 254
					{"eEllipses arguments not allowed in function definition"},			// 255
					{"eReturn statement: cannot convert a number to a pointer"},		// 256
					{"eReturn statement: cannot convert a number to a structure"},		// 257
					{"eReturn statement: return value incompatible with prototype"},	// 258
					{"fPsi_bounds memory *** OUT OF MEMORY ***"},						// 259
					{"fUnable to create PSI output text file"},							// 260
					{"fUnable to create PSI output text temporary file"},				// 261
					{"fUnable to create output file"},									// 262
					{"eDeclaration is not allowed here"},								// 263
					{"fToo many error or warning messages"},							// 264
					{"fIllegal definition"},											// 265
					{"fError files memory *** OUT OF MEMORY ***"},						// 266
					{"eWhile statement missing ;"}										// 267
};
void CompilerErrorOrWarning (int error)
{
	char	file[256];

	// se c'è già stato un errore fatale, esce direttamente
	if (fatal_error)
		return;

	// valuta se stampare il nome del file
	if (strcmp(error_file,current_file)) {
		strcpy(error_file,current_file);
		if ( strlen(error_file) > max_console_error_file_len ) {
			strcpy(file,"...");
			strcat(file,error_file+strlen(error_file)-max_console_error_file_len+3);
		}
		else
			strcpy(file,error_file);
		ConsolePrintf("*** some errors in %s ***",file);
	}

	// imposta error_files
	strcpy(error_files_ptr,current_file);
	error_files_ptr+=strlen(current_file);

	// stampa l'errore o il warning
	if (errors[error][0]=='e') {
		ConsolePrintf("(%i)err%i: %s",current_line,error,&errors[error][1]);
		compiler_errors++;
	}
	else if (errors[error][0]=='w') {
		ConsolePrintf("(%i)war%i: %s",current_line,error,&errors[error][1]);
		compiler_warnings++;
	}
	else if (errors[error][0]=='f') {
		ConsolePrintf("(%i)fat%i: %s",current_line,error,&errors[error][1]);
		fatal_error=1;
	}

	// valuta se il numero di errori è troppo elevato
	if ( compiler_errors + compiler_warnings > max_num_of_errors_and_warnings )
		CompilerErrorOrWarning(264);

	// ritorna
	return;
}

//=====================
//ParseVariableDeclaration
//
//Processa una dichiarazione di variabile
//=====================
int ParseVariableDeclaration (char *pointer, int is_struct_member, int is_parameter, int is_function)
{
	token2_t		*tok_ptr=auxmem;
	char			*vd_ptr=pointer;
	int				ret;
	char			ident[256];
	int				i,j,k,l;
	int				left=1;
	int				braces=0;
	int				first_dim[max_num_of_multiple_declarations];
	int				first_dim_s[max_num_of_multiple_declarations];
	int				decl=0;
	int				deref=0;

	int				type;
	identifier_t	*stype;
	int				is_const;
	int				is_static;

	number_t		result;
	char			string[4096];

	int				num_of_dimensions;
	int				*dim,*vardim;

	int				size=0;

	int				dimensions[max_num_of_dimensions];

	int				skip_increment;

	int				char_fd;

	char			*init;

	token2_t		*ptr;

	int				first_dim_undef;

	int				struct_init;

	int				spec_inc;

	int				block;

	char*			pszMacroRestorePtr = NULL; // // // VITO PLANTAMURA PATCH 21GEN2004

	int				identifier_already_spec = 0; // VPCICE PATCH // 19MAG2004

	// resetta la matrice delle prime dimensioni
	for (i=0;i<max_num_of_multiple_declarations;i++) {
		first_dim[i]=-1;
		first_dim_s[i]=0;
	}

	// analizza i diversi token
	while (1) {

		// salta gli spazi vuoti
		for (;;vd_ptr++)
			if ( *vd_ptr!=' ' &&
				*vd_ptr!='\t' )
				break;
		tok_ptr->is_keyword=-1;
		tok_ptr->is_operator=-1;
		tok_ptr->is_identifier=NULL;
		tok_ptr->is_string=NULL;

		// // // VITO PLANTAMURA PATCH 21GEN2004

		if ( ! * vd_ptr && pszMacroRestorePtr )
		{
			vd_ptr = pszMacroRestorePtr;
			pszMacroRestorePtr = NULL;

			for (;;vd_ptr++)
				if ( *vd_ptr!=' ' &&
					*vd_ptr!='\t' )
					break;
		}

		// // //

		// controlla se la stringa ASCIIZ è finita
		if (!*vd_ptr) {
			decl++;
			break;
		}

		// controlla gli operandi a sinistra dell'uguale
		if (left) {

			// calcola in ret il numero di caratteri di identificatore consecutivi
			ret=IsIdentfString(vd_ptr);

			// // // VITO PLANTAMURA PATCH 21GEN2004

			if ( pszMacroRestorePtr == NULL )
				for ( i=0; i<cident_id; i++ )
					if ( cident[i].type == MACRO &&
						strlen( cident[i].id ) == (size_t) ret &&
						CompareStrings( vd_ptr, cident[i].id, ret ) )
					{
						pszMacroRestorePtr = vd_ptr + ret;
						vd_ptr = cident[i].pointer;
						ret = IsIdentfString( vd_ptr );
						break;
					}

			// // //

			// char
			if (ret == 4 && CompareStrings(vd_ptr,"char",4)) {
				tok_ptr->is_keyword=KW_CHAR;
				vd_ptr+=4;
			}

			// int
			else if (ret == 3 && CompareStrings(vd_ptr,"int",3)) {
				tok_ptr->is_keyword=KW_INT;
				vd_ptr+=3;
			}

			// short
			else if (ret == 5 && CompareStrings(vd_ptr,"short",5)) {
				tok_ptr->is_keyword=KW_SHORT;
				vd_ptr+=5;
			}

			// long
			else if (ret == 4 && CompareStrings(vd_ptr,"long",4)) {
				tok_ptr->is_keyword=KW_LONG;
				vd_ptr+=4;
			}

			// unsigned
			else if (ret == 8 && CompareStrings(vd_ptr,"unsigned",8)) {
				tok_ptr->is_keyword=KW_UNSIGNED;
				vd_ptr+=8;
			}

			// signed
			else if (ret == 6 && CompareStrings(vd_ptr,"signed",6)) {
				tok_ptr->is_keyword=KW_SIGNED;
				vd_ptr+=6;
			}

			// float
			else if (ret == 5 && CompareStrings(vd_ptr,"float",5)) {
				tok_ptr->is_keyword=KW_FLOAT;
				vd_ptr+=5;
			}

			// double
			else if (ret == 6 && CompareStrings(vd_ptr,"double",6)) {
				tok_ptr->is_keyword=KW_DOUBLE;
				vd_ptr+=6;
			}

			// void
			else if (ret == 4 && CompareStrings(vd_ptr,"void",4)) {
				tok_ptr->is_keyword=KW_VOID;
				vd_ptr+=4;
			}

			// static
			else if (ret == 6 && CompareStrings(vd_ptr,"static",6)) {
				tok_ptr->is_keyword=KW_STATIC;
				vd_ptr+=6;
			}

			// const
			else if (ret == 5 && CompareStrings(vd_ptr,"const",5)) {
				tok_ptr->is_keyword=KW_CONST;
				vd_ptr+=5;
			}

			// // // // // VPCICE PATCH // 19MAG2004

			// name
			else if (ret == 4 && CompareStrings(vd_ptr,"name",4)) {
				tok_ptr --;
				vd_ptr+=4;
			}

			// macro
			else if (ret == 5 && CompareStrings(vd_ptr,"macro",5)) {
				tok_ptr --;
				vd_ptr+=5;
			}

			// // // // // // // // // // // // // //

			// assegnazione
			else if (CompareStrings(vd_ptr,"=",1)) {
				tok_ptr->is_operator=ASSIGNMENT;
				vd_ptr++;
				left=0;
				first_dim[decl]=1;
				braces=0;
				char_fd=0;
			}

			// dereference
			else if (CompareStrings(vd_ptr,"*",1)) {
				tok_ptr->is_operator=DEREFERENCE;
				vd_ptr++;
				deref++;
			}

			// parentesi quadre
			else if (CompareStrings(vd_ptr,"[",1)) {
				tok_ptr->is_operator=ARRAY_SUBSCRIPT;
				vd_ptr++;
				tok_ptr++;
				for (i=0;;i++) {
					if (vd_ptr[i]==']')
						break;
					else if (!vd_ptr[i]) {
						CompilerErrorOrWarning(3);
						return(1);
					}
				}
				if (i) {
					tok_ptr->is_keyword=-1;
					tok_ptr->is_operator=-1;
					tok_ptr->is_identifier=NULL;
					tok_ptr->is_string=vd_ptr;
					tok_ptr->len=i;
					vd_ptr+=i;
					tok_ptr++;
				}
				tok_ptr->is_keyword=-1;
				tok_ptr->is_identifier=NULL;
				tok_ptr->is_string=NULL;
				tok_ptr->is_operator=CA_PARENTHESES;
				vd_ptr++;
			}
			else if (CompareStrings(vd_ptr,"]",1)) {
				CompilerErrorOrWarning(6);
				return(1);
			}

			// virgola
			else if (CompareStrings(vd_ptr,",",1)) {
				tok_ptr->is_operator=NEW_DECLARATION_COMMA;
				vd_ptr++;
				decl++;
				deref=0;
			}

			// identificatore
			else if (ret=IsIdentfString(vd_ptr)) {
				if (ret>=identifier_max_len) {
					j=identifier_max_len-1;
					CompilerErrorOrWarning(1);
				}
				else
					j=ret;

				memcpy(ident,vd_ptr,j);
				ident[j]=0;

				for (i=0;i<cident_id;i++) {
					if (!strcmp(ident,cident[i].id)) {
						if ((cident[i].type & 0xff0000) == STRUCT) {
							tok_ptr->is_identifier=&cident[i];
							break;
						}
						else if ((cident[i].type & 0xff0000) == VARIABLE ||
							(cident[i].type & 0xff0000) == FUNCTION) {
							if (is_struct_member) {
								tok_ptr->is_string=vd_ptr;
								tok_ptr->len=j;
								break;
							}
							else if (is_parameter) {
								if (is_prototype) {
									tok_ptr->is_string=vd_ptr;
									tok_ptr->len=j;
									break;
								}
								else {
									CompilerErrorOrWarning(47);
									return(1);
								}
							}
							else if (is_function) {
								if ( (cident[i].type & 0xff0000) == VARIABLE ) {
									CompilerErrorOrWarning(61);
									return(1);
								}
								else
									func_prototype=&cident[i];
							}
							else {
								CompilerErrorOrWarning(35);
								return(1);
							}
						}
					}
				}
				if (i==cident_id) {
					if (*vd_ptr>='0' && *vd_ptr<='9') {
						if (is_struct_member) {
							CompilerErrorOrWarning(62);
							return(1);
						}
						else if (is_parameter) {
							CompilerErrorOrWarning(64);
							return(1);
						}
						else if (is_function) {
							CompilerErrorOrWarning(63);
							return(1);
						}
						else {
							CompilerErrorOrWarning(39);
							return(1);
						}
					}
					for (ptr=auxmem;ptr<tok_ptr;ptr++)
						if (ptr->is_string)
							if (ptr->len==j)
								if (!memcmp(ptr->is_string,vd_ptr,j)) {
									if (is_struct_member) {
										CompilerErrorOrWarning(72);
										return(1);
									}
									else if (is_parameter) {
										CompilerErrorOrWarning(73);
										return(1);
									}
									else if (is_function) { // inutile...
										CompilerErrorOrWarning(61);
										return(1);
									}
									else {
										CompilerErrorOrWarning(35);
										return(1);
									}
								}
					// // // // // VPCICE PATCH // 19MAG2004

					if ( identifier_already_spec )
					{
						if (is_struct_member) {
							CompilerErrorOrWarning(62);
							return(1);
						}
						else if (is_parameter) {
							CompilerErrorOrWarning(64);
							return(1);
						}
						else if (is_function) {
							CompilerErrorOrWarning(63);
							return(1);
						}
						else {
							CompilerErrorOrWarning(39);
							return(1);
						}
					}
					else
					{
						identifier_already_spec = 1;
					}

					// // // // // // // // // // // // // //
					tok_ptr->is_string=vd_ptr;
					tok_ptr->len=j;
				}

				vd_ptr+=ret;
			}

			// carattere sconosciuto
			else {
				CompilerErrorOrWarning(4);
				return(1);
			}

		}

		// controlla gli operandi a destra dell'uguale
		else {

			// virgola
			if (CompareStrings(vd_ptr,",",1)) {
				if (braces) {
					tok_ptr->is_operator=COMMA;
					if (char_fd==1) {
						first_dim[decl]=1;
						first_dim_s[decl]=0;
					}
					char_fd=2;
					if (braces==1)
						first_dim[decl]++;
				}
				else {
					tok_ptr->is_operator=NEW_DECLARATION_COMMA;
					left=1;
					decl++;
					deref=0;
				}
				vd_ptr++;
			}

			// parentesi graffe
			else if (CompareStrings(vd_ptr,"{",1)) {
				tok_ptr->is_operator=O_BRACE;
				braces++;
				vd_ptr++;
			}
			else if (CompareStrings(vd_ptr,"}",1)) {
				tok_ptr->is_operator=C_BRACE;
				braces--;
				vd_ptr++;
			}

			// inizializzatore
			else {
				if (!(i=IsString(vd_ptr)))
					for (i=0;;i++) {
						if (vd_ptr[i]=='}' ||
							( braces && vd_ptr[i]==',' ) || // VPCICE PATCH // // 20MAG2004 //
							vd_ptr[i]==0 )
							break;
						else if (vd_ptr[i]=='{') {
							CompilerErrorOrWarning(22);
							return(1);
						}
					}
				else if (i==-1) {
					CompilerErrorOrWarning(3);
					return(1);
				}
				else if (*vd_ptr=='"' && !char_fd) {
					if (!braces) {
						if (!deref) {
							first_dim[decl]=i-1;
							first_dim_s[decl]=1;
						}
					}
					else if (braces==1) {
						if (!deref) {
							first_dim[decl]=i-1;
							first_dim_s[decl]=1;
							char_fd=1;
						}
					}
				}
				tok_ptr->is_string=vd_ptr;
				tok_ptr->len=i;
				vd_ptr+=i;
			}

		}

		// incrementa tok_ptr
		tok_ptr++;
	}

	// effettua un primo controllo sulle parentesi graffe
	if (braces) {
		CompilerErrorOrWarning(22);
		return(1);
	}

	// reimposta tok_ptr
	tok_ptr=auxmem;

	// stabilisce il tipo di variabile che si desidera dichiarare
	type=-1;
	stype=NULL;
	is_const=0;
	is_static=0;
	j=0;
	for (i=0;;i++) {
		if (type==-1) {
			if (tok_ptr[i].is_keyword==KW_CHAR)
				type=CHAR;
			else if (tok_ptr[i].is_keyword==KW_INT) {
				if (j & 1)
					type=SHORT;
				else
					type=INT;
			}
			else if (tok_ptr[i].is_keyword==KW_FLOAT) {
				if (j & 2)
					type=DOUBLE;
				else
					type=FLOAT;
			}
			else if (tok_ptr[i].is_keyword==KW_DOUBLE) {
				if (j & 2)
					type=LONG_DOUBLE;
				else
					type=DOUBLE;
			}
			else if (tok_ptr[i].is_keyword==KW_VOID)
				type=VOID_TYPE;
			else if (tok_ptr[i].is_identifier) {
				stype=tok_ptr[i].is_identifier;
				type=STRUCTURE_TYPE;
			}
		}
		else {
			if (tok_ptr[i].is_keyword==KW_CHAR ||
				tok_ptr[i].is_keyword==KW_INT ||
				tok_ptr[i].is_keyword==KW_FLOAT ||
				tok_ptr[i].is_keyword==KW_DOUBLE ||
				tok_ptr[i].is_keyword==KW_VOID ||
				tok_ptr[i].is_identifier) {
				CompilerErrorOrWarning(16);
				return(1);
			}
		}
		if (tok_ptr[i].is_keyword==KW_SHORT) {
			if (j & 2) {
				CompilerErrorOrWarning(14);
				return(1);
			}
			if (type==STRUCTURE_TYPE) {
				CompilerErrorOrWarning(16);
				return(1);
			}
			j|=1;
			if (type==INT)
				type=SHORT;
		}
		else if (tok_ptr[i].is_keyword==KW_LONG) {
			if (j & 1) {
				CompilerErrorOrWarning(14);
				return(1);
			}
			if (type==STRUCTURE_TYPE) {
				CompilerErrorOrWarning(16);
				return(1);
			}
			if (type==FLOAT)
				type=DOUBLE;
			else if (type==DOUBLE && !(j & 2))
				type=LONG_DOUBLE;
			j|=2;
		}
		else if (tok_ptr[i].is_keyword==KW_SIGNED) {
			if (j & 8) {
				CompilerErrorOrWarning(14);
				return(1);
			}
			if (type==STRUCTURE_TYPE) {
				CompilerErrorOrWarning(16);
				return(1);
			}
			j|=4;
		}
		else if (tok_ptr[i].is_keyword==KW_UNSIGNED) {
			if (j & 4) {
				CompilerErrorOrWarning(14);
				return(1);
			}
			if (type==STRUCTURE_TYPE) {
				CompilerErrorOrWarning(16);
				return(1);
			}
			j|=8;
		}
		else if (tok_ptr[i].is_keyword==KW_STATIC)
			is_static=1;
		else if (tok_ptr[i].is_keyword==KW_CONST)
			is_const=1;

		if (tok_ptr[i].is_keyword==-1 &&
			!tok_ptr[i].is_identifier) {
			if (type==-1 && !j && !is_static) {
				if (is_struct_member) {
					CompilerErrorOrWarning(62);
					return(1);
				}
				else if (is_parameter) {
					CompilerErrorOrWarning(64);
					return(1);
				}
				else if (is_function) {
					CompilerErrorOrWarning(63);
					return(1);
				}
				else {
					CompilerErrorOrWarning(39);
					return(1);
				}
			}
			if (type==-1) {
				if (j & 1)
					type=SHORT;
				else
					type=INT;
			}
			if ( (j & 8) &&
				(type==CHAR ||
				type==SHORT ||
				type==INT) )
				type|=1;
			break;
		}
	}

	// controlla che la variabile o le variabili non siano statiche se membre di struttura o parametri di funzione
	if (is_static) {
		if (is_struct_member) {
			CompilerErrorOrWarning(42);
			return(1);
		}
		else if (is_parameter) {
			CompilerErrorOrWarning(46);
			return(1);
		}
		else
			CompilerErrorOrWarning(236);
	}

	// analizza ciascuna variabile dichiarata
	for (j=0;j<decl;j++,cident_id++) {

		// inizializza
		if (is_struct_member)
			cident[cident_id].type=type | STRUCT_FIELD;
		else if (is_parameter)
			cident[cident_id].type=type | FUNCTION_PARAMETER;
		else if (is_function)
			cident[cident_id].type=type | FUNCTION;
		else
			cident[cident_id].type=type | VARIABLE;
		cident[cident_id].static_flag=is_static;
		cident[cident_id].const_flag=is_const;
		cident[cident_id].struct_type=stype;
		cident[cident_id].fields=0;
		cident[cident_id].array_stptr=0;
		cident[cident_id].ellipses_arguments=0;
		cident[cident_id].initializer_line=0;
		first_dim_undef=0;
		struct_init=0;

		// level of indirection
		for (k=0;;i++) {
			if (tok_ptr[i].is_operator==DEREFERENCE)
				k++;
			else
				break;
		}
		cident[cident_id].indirection=k;

		// controlla che il tipo non sia vuoto
		if (type==VOID_TYPE && !k && !is_parameter && !is_function) {
			CompilerErrorOrWarning(17);
			return(1);
		}

		// controlla se un'eventuale inizializzazione è su una struttura
		if (type==STRUCTURE_TYPE && !k)
			struct_init=1;

		// stabilisce size
		if (k)
			size=4;
		else if (type==CHAR)
			size=1;
		else if (type==UNSIGNED_CHAR)
			size=1;
		else if (type==SHORT)
			size=2;
		else if (type==UNSIGNED_SHORT)
			size=2;
		else if (type==INT)
			size=4;
		else if (type==UNSIGNED_INT)
			size=4;
		else if (type==FLOAT)
			size=4;
		else if (type==DOUBLE)
			size=8;
		else if (type==LONG_DOUBLE)
			size=8;
		else if (type==STRUCTURE_TYPE) {
			size=stype->address;
			if (size==-1) {
				CompilerErrorOrWarning(51);
				return(1);
			}
		}

		// identifier
		if (tok_ptr[i].is_string) {
			memcpy(cident[cident_id].id,tok_ptr[i].is_string,tok_ptr[i].len);
			cident[cident_id].id[tok_ptr[i].len]=0;
			i++;
		}
		else if (tok_ptr[i].is_identifier) {
			if (is_struct_member) {
				//===== patch 16/5/99 VP
				// strcpy(cident[cident_id].id,tok_ptr[i++].is_identifier->id);
				//=====
				CompilerErrorOrWarning(72);
				return(1);
			}
			else if (is_parameter) {
				//===== patch 16/5/99 VP
				// strcpy(cident[cident_id].id,tok_ptr[i++].is_identifier->id);
				//=====
				CompilerErrorOrWarning(73);
				return(1);
			}
			else if (is_function) {
				CompilerErrorOrWarning(61);
				return(1);
			}
			else {
				CompilerErrorOrWarning(35);
				return(1);
			}
		}
		else {
			if (is_parameter)
				*cident[cident_id].id=0;	// abstract declarator
			else {
				if (is_struct_member) {
					CompilerErrorOrWarning(62);
					return(1);
				}
				else if (is_function) {
					CompilerErrorOrWarning(63);
					return(1);
				}
				else {
					CompilerErrorOrWarning(39);
					return(1);
				}
			}
		}

		// controlla che l'identificatore non sia una reserved word
		for (k=0;reswords[k].ptr!=NULL;k++)
			if (reswords[k].len == (signed)strlen(cident[cident_id].id) &&
				CompareStrings(cident[cident_id].id,reswords[k].ptr,reswords[k].len)) {
				CompilerErrorOrWarning(174);
				return(1);
			}

		// controlla che gli identificatori non siano ripetuti
		if (is_struct_member) {
			for (k=cident_id-1;
				(cident[k].type & 0xff0000)==STRUCT_FIELD;
				k--)
					if (!strcmp(cident[cident_id].id,cident[k].id)) {
						CompilerErrorOrWarning(45);
						return(1);
					}
		}
		else if (is_parameter && *cident[cident_id].id) {
			for (k=cident_id-1;
				(cident[k].type & 0xff0000)==FUNCTION_PARAMETER;
				k--)
					if (!strcmp(cident[cident_id].id,cident[k].id)) {
						CompilerErrorOrWarning(47);
						return(1);
					}
		}

		// controlla che void come parametro non sia seguito da identificatore
		if (is_parameter &&
			type==VOID_TYPE &&
			!cident[cident_id].indirection &&
			*cident[cident_id].id) {
			CompilerErrorOrWarning(74);
			return(1);
		}

		// dimensions
		if (tok_ptr[i].is_operator==ARRAY_SUBSCRIPT) {

			// controlla che le parentesi quadre non siano in una dichiarazione di funzione
			if (is_function) {
				CompilerErrorOrWarning(67);
				return(1);
			}

			// controlla che void sia utilizzato correttamente come parametro
			if (is_parameter && type==VOID_TYPE && !cident[cident_id].indirection) {
				CompilerErrorOrWarning(71);
				return(1);
			}

			i++;
			if (tok_ptr[i].is_string) {
				memcpy(string,tok_ptr[i].is_string,tok_ptr[i].len);
				string[tok_ptr[i].len]=0;
				if (ResolveConstantExpression(&result,string))
					return(1);

				if (result.type==CHAR)
					k=*(char*)result.number;
				else if (result.type==UNSIGNED_CHAR)
					k=*(unsigned char*)result.number;
				else if (result.type==SHORT)
					k=*(short*)result.number;
				else if (result.type==UNSIGNED_SHORT)
					k=*(unsigned short*)result.number;
				else if (result.type==INT)
					k=*(int*)result.number;
				else if (result.type==UNSIGNED_INT)
					k=*(unsigned int*)result.number;
				else if (result.type==FLOAT)
#pragma warning( disable : 4244 ) // "conversion from 'xxx ' to 'xxx ', possible loss of data"
					k=*(float*)result.number;
				else if (result.type==DOUBLE)
					k=*(double*)result.number;
				else if (result.type==LONG_DOUBLE)
					k=*(long double*)result.number;
#pragma warning( default : 4244 ) // "conversion from 'xxx ' to 'xxx ', possible loss of data"

				i++;
			}
			else {
				if (is_struct_member) {
					CompilerErrorOrWarning(43);
					return(1);
				}
				else if (is_parameter) {
					CompilerErrorOrWarning(48);
					return(1);
				}
				else {
					k=first_dim[j];
					if (!first_dim_s[j])
						first_dim_undef=1;
				}
			}
			i++;

			if (k<=0) {
				CompilerErrorOrWarning(36);
				return(1);
			}

			vardim=dim=cident_aux_ptr;

			*dim=k;
			if (!first_dim_undef)
				size*=k;
			dim++;

			for (;;i++) {
				if (tok_ptr[i].is_operator==ARRAY_SUBSCRIPT) {
					i++;
					if (tok_ptr[i].is_string) {
						memcpy(string,tok_ptr[i].is_string,tok_ptr[i].len);
						string[tok_ptr[i].len]=0;
						if (ResolveConstantExpression(&result,string))
							return(1);

						if (result.type==CHAR)
							k=*(char*)result.number;
						else if (result.type==UNSIGNED_CHAR)
							k=*(unsigned char*)result.number;
						else if (result.type==SHORT)
							k=*(short*)result.number;
						else if (result.type==UNSIGNED_SHORT)
							k=*(unsigned short*)result.number;
						else if (result.type==INT)
							k=*(int*)result.number;
						else if (result.type==UNSIGNED_INT)
							k=*(unsigned int*)result.number;
						else if (result.type==FLOAT)
#pragma warning( disable : 4244 ) // "conversion from 'xxx ' to 'xxx ', possible loss of data"
							k=*(float*)result.number;
						else if (result.type==DOUBLE)
							k=*(double*)result.number;
						else if (result.type==LONG_DOUBLE)
							k=*(long double*)result.number;
#pragma warning( default : 4244 ) // "conversion from 'xxx ' to 'xxx ', possible loss of data"

						if (k<=0) {
							CompilerErrorOrWarning(36);
							return(1);
						}

						*dim=k;
						dim++;
						size*=k;

						i++;
					}
					else {
						if (is_struct_member) {
							CompilerErrorOrWarning(65);
							return(1);
						}
						else if (is_parameter) {
							CompilerErrorOrWarning(66);
							return(1);
						}
						else {
							CompilerErrorOrWarning(37);
							return(1);
						}
					}

				}
				else
					break;
			}

			num_of_dimensions=
			cident[cident_id].dimensions=
				dim-(int*)cident_aux_ptr;

			cident[cident_id].pointer=cident_aux_ptr;

			(char*)cident_aux_ptr+=(char*)dim-(char*)cident_aux_ptr;

		}
		else {
			num_of_dimensions=
			cident[cident_id].dimensions=
				0;

			cident[cident_id].pointer=NULL;
		}

		// controlla se ci sono degli inizializzatori
		if (tok_ptr[i].is_operator==ASSIGNMENT) {
			i++;

			// controlla se l'inizializzazione è lecita
			if (is_struct_member) {
				CompilerErrorOrWarning(44);
				return(1);
			}
			else if (is_parameter) {
				CompilerErrorOrWarning(49);
				return(1);
			}
			else if (is_function) {
				CompilerErrorOrWarning(68);
				return(1);
			}

			// controlla se è possibile inizializzare
			if (struct_init) {
				CompilerErrorOrWarning(41);
				return(1);
			}

			// controlla le parentesi graffe
			if (num_of_dimensions>0)
				if (!(type==CHAR && num_of_dimensions==1 && !cident[cident_id].indirection))
					if (tok_ptr[i].is_operator!=O_BRACE) {
						CompilerErrorOrWarning(40);
						return(1);
					}

			// resetta
			for (k=0;k<max_num_of_dimensions;k++)
				dimensions[k]=0;
			skip_increment=0;
			braces=0;
			init=cident_aux_ptr;
			init[0]=0;

			// ciclo
			block=0;
			for (;;i++) {

				spec_inc=0;

				// parentesi graffe
				if (tok_ptr[i].is_operator==O_BRACE) {
					if (braces &&
						tok_ptr[i-1].is_operator!=COMMA &&
						tok_ptr[i-1].is_operator!=O_BRACE) {
						CompilerErrorOrWarning(59);
						return(1);
					}
					if ( !(tok_ptr[i+1].is_string && tok_ptr[i+2].is_operator==C_BRACE) ) {
						if (!num_of_dimensions) {
							CompilerErrorOrWarning(60);
							return(1);
						}
						braces++;
						if (tok_ptr[i-1].is_operator!=O_BRACE &&
							tok_ptr[i-2].is_string) {
							if (!(*tok_ptr[i-2].is_string=='"' &&
								!cident[cident_id].indirection)) {
								if (braces>1) {
									if (braces-2>=num_of_dimensions) {
										CompilerErrorOrWarning(38);
										return(1);
									}
									if (++dimensions[braces-2]>=vardim[braces-2]) {
										CompilerErrorOrWarning(38);
										return(1);
									}
									for (k=braces-1;k<num_of_dimensions;k++)
										dimensions[k]=0;
								}
							}
						}
					}
					skip_increment=1;
				}
				else if (tok_ptr[i].is_operator==C_BRACE) {
					if (!tok_ptr[i-1].is_string &&
						tok_ptr[i-1].is_operator!=C_BRACE) {
						CompilerErrorOrWarning(59);
						return(1);
					}
					if ( !(tok_ptr[i-1].is_string && tok_ptr[i-2].is_operator==O_BRACE) ) {
						braces--;
						if (tok_ptr[i+1].is_operator!=C_BRACE) {
							if (braces>0) {
								if (++dimensions[braces-1]>=vardim[braces-1])
									block=1;
								for (k=braces;k<num_of_dimensions;k++)
									dimensions[k]=0;
							}
						}
					}
					skip_increment=1;
				}
				// virgola
				else if (tok_ptr[i].is_operator==COMMA) {
					if (!tok_ptr[i-1].is_string &&
						tok_ptr[i-1].is_operator!=C_BRACE) {
						CompilerErrorOrWarning(58);
						return(1);
					}
					if (!num_of_dimensions) {
						CompilerErrorOrWarning(38);
						return(1);
					}
					skip_increment=1;
				}
				// inizializzatore
				else if (tok_ptr[i].is_string) {
					if (block) {
						CompilerErrorOrWarning(38);
						return(1);
					}

					strcat(init,cident[cident_id].id);

					if (num_of_dimensions) {
						for (k=0;k<num_of_dimensions;k++) {
							sprintf(string,"[%i]",dimensions[k]);
							strcat(init,string);
						}
					}
					strcat(init,"=");

					memcpy(string,tok_ptr[i].is_string,tok_ptr[i].len);
					string[tok_ptr[i].len]=0;

					strcat(init,string);
					strcat(init,"\n");

					if (tok_ptr[i+1].is_operator==C_BRACE) {
						if (tok_ptr[i-1].is_operator!=O_BRACE)
							skip_increment=1;
					}

					if (tok_ptr[i].is_string[0]=='"' &&
						!cident[cident_id].indirection) {
						if (tok_ptr[i].len-2>vardim[num_of_dimensions-1]) {
							CompilerErrorOrWarning(38);
							return(1);
						}
						spec_inc=1;
					}

				}
				else
					break;

				// incremento
				if (!skip_increment && num_of_dimensions) {
					for (k=num_of_dimensions- (spec_inc ? 2 : 1);k>=0;k--)
						if (dimensions[k]<vardim[k]-1)
							break;
					if (k<0) {
						if (tok_ptr[i+1].is_operator!=C_BRACE) {
							if ( spec_inc && num_of_dimensions == 1 ) {
								i+=1;
								break;
							}
							CompilerErrorOrWarning(38);
							return(1);
						}
						else {
							i+=2;
							break;
						}
					}
					dimensions[k]++;
					for (k++;k<num_of_dimensions;k++)
						dimensions[k]=0;
				}
				else
					skip_increment=0;
			}

			// controlla che sia tutto ok con la sintassi dopo l'ultima parentesi graffa
			if (tok_ptr[i].is_operator!=NEW_DECLARATION_COMMA &&
				!(tok_ptr[i].is_keyword==-1 &&
				tok_ptr[i].is_operator==-1 &&
				tok_ptr[i].is_identifier==NULL &&
				tok_ptr[i].is_string==NULL) ) {
				CompilerErrorOrWarning(59);
				return(1);
			}
			else if (tok_ptr[i].is_operator==NEW_DECLARATION_COMMA)
				i++;

			// aggiorna cident_aux_ptr
			(char*)cident_aux_ptr+=strlen(init)+1;

			// imposta il puntatore agli inizializzatori
			cident[cident_id].initializer=init;
			cident[cident_id].initializer_line=current_line;

			// controlla se bisogna ridefinire la prima dimensione
			if (first_dim_undef) {
				*(int*)cident[cident_id].pointer=dimensions[0]+1;
				size*=dimensions[0]+1;
			}

		}
		else if ( tok_ptr[i].is_operator==NEW_DECLARATION_COMMA ||
			(tok_ptr[i].is_keyword==-1 &&
			tok_ptr[i].is_operator==-1 &&
			tok_ptr[i].is_identifier==NULL &&
			tok_ptr[i].is_string==NULL) ) {
			cident[cident_id].initializer=NULL;
			i++;
		}

		// address
		if (is_struct_member) {

			// allinea i membri della struttura
			if (size % struct_alignment) {
				if (struct_alignment==1)
					l=0;
				else if (struct_alignment==2)
					l=1;
				else if (struct_alignment==4)
					l=2;
				else if (struct_alignment==8)
					l=3;
				else if (struct_alignment==16)
					l=4;
				size = ( size + struct_alignment >> l ) << l;
			}

			// imposta address e cur_fld_address
			cident[cident_id].address=cur_fld_address;
			cur_fld_address+=size;

		}
		else if (is_parameter) {
			if (cident[cident_id].dimensions)
				cident[cident_id].address=4;
			else
				cident[cident_id].address=size;
		}
		else if (is_function) {
			cident[cident_id].address=0;
		}
		else {

			// per prima cosa allinea i dati arrotondando size per 4
			if (size % 4)
				size=(size+4>>2)<<2;

			// a seconda del tipo di variabile aggiorna address
			if (cur_prototype) { // locale
				cur_local_var_address-=size;
				cident[cident_id].address=cur_local_var_address;
			}
			else { // globale
				cident[cident_id].address=cur_global_var_address;
				cur_global_var_address+=size;
			}

		}
	}

	// controlla che la memoria sia ok
	if (CompilerMemoryCheckPoint())
		return(1);

	// ritorna
	return(0);
}

//=====================
//ConvertStringToNumber
//
//Converte una stringa in un numero
//=====================
	//
	// NOTE: Various PATCHES applied for VPCICE Debugger Compatibility. (06/07APR2004)
	//
int ConvertStringToNumber (number_t *number, char *string)
{
	int		i,len;
	char	*pointer;
	int		c;

	int		n;
	double	d;

	// controlla che sia effettivamente un numero
	if (!(*string>='0' && *string<='9') && *string!='\'')
		return(1);

	// calcola la lunghezza della stringa da convertire
	len=strlen(string);

	// valuta che tipo di conversione è richiesta
	if (*string=='\'') {

		// converte in int una successione di caratteri
		if (string[len-1]!='\'')
			return(1);
		if (len>6) {
			CompilerErrorOrWarning(25);
			return(1);
		}

		number->type=INT;
		n=0;
		for (i=1;i<len-1;i++)
			n+=string[i]<<(8*(i-1));
		*(int*)number->number=n;

	}
	else {

		// controlla se ci sono caratteri tipici di un numero double
		for (i=0;i<len;i++) {
			/*if (string[i]=='e') // // // // VPCICE PATCH // // // 07APR2004 // // //
				break;
			else*/ if (string[i]=='.')
				break;
		}

		// esegue la conversione
		if (i==len) {
			// // // // // // // // // // // // // // // // // // // // // VPCICE PATCH // // // 06APR2004 // // //

			for (i=0;i<len;i++)
				if ( string[i] == 't' || string[i] == 'T' )
					break;

			if ( i != len )
			{
				number->type=INT;
				n = MACRO_CRTFN_NAME(strtol)( string, & pointer, 0 );
				c = len - ( pointer - string );
				if ( c > 1 )
				{
					return(1);
				}
				else if (c)
				{
					if ( string[len-1]!='t' && string[len-1]!='T' )
						return(1);
				}
				* (int *) number -> number = n;
			}
			else
			{
				unsigned int un;
				number->type=UNSIGNED_INT;
				un = MACRO_CRTFN_NAME(strtoul)( string, & pointer, 16 );
				c = len - ( pointer - string );
				if ( c > 1 )
				{
					return(1);
				}
				else if ( c )
				{
					if ( string[len-1]!='u' && string[len-1]!='U' )
						return(1);
				}
				* (unsigned int *) number -> number = un;
			}

			// -- OLD CODE --
			//number->type=INT;
			//n=strtol(string,&pointer,0);
			//c=len-(pointer-string);
			//if (c>1)
			//	return(1);
			//else if (c) {
			//	if (string[len-1]!='u' && string[len-1]!='l')
			//		return(1);
			//	else if (string[len-1]=='u')
			//		number->type=UNSIGNED_INT;
			//}
			//*(int*)number->number=n;

			// // // // // // // // // // // // // // // // // // // // //
		}
		else {
			d=MACRO_CRTFN_NAME(strtod)(string,&pointer);
			c=len-(pointer-string);
			if (c>1)
				return(1);
			else if (c) {
				if (string[len-1]!='f' && string[len-1]!='l' &&
					string[len-1]!='F' && string[len-1]!='L')
					return(1);
			}
			if ( (d<3.4e-38 || d>3.4e+38) &&
				string[len-1]!='f' && string[len-1]!='F') {
				number->type=DOUBLE;
				*(double*)number->number=d;
			}
			else {
				number->type=FLOAT;
				*(float*)number->number=(float)d;
			}
		}

	}

	// ritorna
	return(0);
}

//===================
//EliminateEscapeSequences
//
//sostituisce alle escape sequence i rispettivi caratteri ASCII
//===================
int EliminateEscapeSequences (char *string0, char *string1, int s0_len, int warn)
{
	int		i,j;
	char	*pointer;
	int		n;
	char	buffer[256];
	char	aux[16*1024];

	// controlla se string1 è uguale a NULL
	if ( string1==NULL )
		string1=aux;

	// esegue la sostituzione
	for (i=0,j=0;i<s0_len;i++) {
		if (string0[i]!='\\')
			string1[j++]=string0[i];
		else {
			i++;
			if (string0[i]=='a')		// Bell (alert)
				string1[j++]='\a';
			else if (string0[i]=='b')	// Backspace
				string1[j++]='\b';
			else if (string0[i]=='f')	// Formfeed
				string1[j++]='\f';
			else if (string0[i]=='n')	// New line
				string1[j++]='\n';
			else if (string0[i]=='r')	// Carriage return
				string1[j++]='\r';
			else if (string0[i]=='t')	// Horizontal tab
				string1[j++]='\t';
			else if (string0[i]=='v')	// Vertical tab
				string1[j++]='\v';
			else if (string0[i]=='x' &&
				string0[i+1]>='0' && 
				string0[i+1]<='9') {	// ASCII character in hexadecimal notation

				memcpy(buffer,&string0[i-1],5);
				buffer[0]='0';
				buffer[5]=0;
				n=MACRO_CRTFN_NAME(strtol)(buffer,&pointer,0);

				i+=pointer-buffer-1;
				string1[j++]=n;

			}
			else if (string0[i]>='0' &&
				string0[i]<='9') {		// ASCII character in octal notation

				memcpy(buffer,&string0[i-1],4);
				buffer[0]='0';
				buffer[4]=0;
				n=MACRO_CRTFN_NAME(strtol)(buffer,&pointer,0);

				i+=pointer-buffer-1;
				string1[j++]=n;

			}
			else {						// other
				string1[j++]=string0[i];
				if (string0[i]!='"' &&
					string0[i]!='\'' &&
					string0[i]!='?' &&
					string0[i]!='\\')
					if ( warn )
						CompilerErrorOrWarning(33);
			}
		}
	}

	// ritorna
	return(j);
}

//=====================
//ResolveConstantExpression
//
//Restituisce il risultato di una espressione costante
//=====================
#define		ebuffer		1
int ResolveConstantExpression (number_t *result, char *string)
{
	token_t		*tok_ptr=(token_t*)((char *)expr_tokens+ebuffer*(int)(single_expr_tokens_buffer*compiler_memory_factor));
	operator_t	*op_ptr=(operator_t*)((char*)operators+ebuffer*(int)(single_operators_buffer*compiler_memory_factor));
	number_t	*buffer=rcemem;

	token_t		*pointer;
	char		tmpstring[4096];
	int			ret;
	int			end_id;

	// analizza i diversi token dell'espressione
	if (ParseExpressionTokens(string,ebuffer))
		return(1);

	// controlla se effettivamente l'espressione è costante
	for (pointer=tok_ptr;pointer->id!=EXPRESSION_END;pointer++) {
		if (!pointer->tok_taken) {
			if (pointer->id==IDENTIFIER) {
				if (!(*(char*)pointer->ptr0>='0' && *(char*)pointer->ptr0<='9')) {
					CompilerErrorOrWarning(23);
					return(1);
				}
			}
			else if (pointer->id==STRING) {
				if (*(char*)pointer->ptr0!='\'') {
					CompilerErrorOrWarning(23);
					return(1);
				}
			}
			else if ( (pointer->id & 0xff00) == 0x0000 ) {
				CompilerErrorOrWarning(24);
				return(1);
			}
			else if ( (pointer->id & 0xff00) == 0x0d00 ) {
				CompilerErrorOrWarning(24);
				return(1);
			}
			else if (pointer->id==PREFIX_INCREMENT ||
				pointer->id==PREFIX_DECREMENT ||
				pointer->id==ADDRESS_OF ||
				pointer->id==DEREFERENCE ||
				pointer->id==COMMA) {
				CompilerErrorOrWarning(24);
				return(1);
			}
		}
	}

	// inizializza il buffer dei risultati temporanei
	for (pointer=tok_ptr;pointer->id!=EXPRESSION_END;pointer++) {
		if (!pointer->tok_taken) {
			if (pointer->id==IDENTIFIER || pointer->id==STRING) {
				if (*(char*)pointer->ptr0=='\'') {
					tmpstring[0]='\'';
					ret=EliminateEscapeSequences((char*)pointer->ptr0+1,&tmpstring[1],strlen(pointer->ptr0)-2,1);
					tmpstring[ret+1]='\'';
					tmpstring[ret+2]=0;
					ret=ConvertStringToNumber(&buffer[pointer-tok_ptr],tmpstring);
					if (ret) {
						CompilerErrorOrWarning(26);
						return(1);
					}
				}
				else {
					ret=ConvertStringToNumber(&buffer[pointer-tok_ptr],pointer->ptr0);
					if (ret) {
						CompilerErrorOrWarning(26);
						return(1);
					}
				}
			}
		}
	}

	// ordina gli operatori e crea lo stack delle operazioni
	if (MakeExprOrderedOpList(ebuffer))
		return(1);

	// trova l'ultimo elemento dello stack
	for (end_id=0;op_ptr[end_id].id!=EXPRESSION_END;end_id++) {}

	// esegue le varie operazioni
	for (end_id--;end_id>=0;end_id--)
		if (ResolveConstantOperation(op_ptr,buffer,tok_ptr,end_id))
			return(1);

	// restituisce il risultato
	*result=buffer[result_id];

	// ritorna
	return(0);
}
#undef		ebuffer

//======================
//CastTypeOfConstantNumber
//
//Modifica il tipo di un numero costante
//======================
void CastTypeOfConstantNumber (number_t *number0, number_t *number1, int type)
{
	// modifica il tipo
	number1->type=type;
#pragma warning( disable : 4244 ) // "conversion from 'xxx ' to 'xxx ', possible loss of data"
	if (type==CHAR) {
		if (number0->type==CHAR)
			*(char*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(char*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(char*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(char*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(char*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(char*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT) {
			*(char*)number1->number=*(float*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==DOUBLE) {
			*(char*)number1->number=*(double*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==LONG_DOUBLE) {
			*(char*)number1->number=*(long double*)number0->number;
			CompilerErrorOrWarning(31);
		}
	}
	else if (type==UNSIGNED_CHAR) {
		if (number0->type==CHAR)
			*(unsigned char*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(unsigned char*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(unsigned char*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(unsigned char*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(unsigned char*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(unsigned char*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT) {
			*(unsigned char*)number1->number=*(float*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==DOUBLE) {
			*(unsigned char*)number1->number=*(double*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==LONG_DOUBLE) {
			*(unsigned char*)number1->number=*(long double*)number0->number;
			CompilerErrorOrWarning(31);
		}
	}
	else if (type==SHORT) {
		if (number0->type==CHAR)
			*(short*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(short*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(short*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(short*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(short*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(short*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT) {
			*(short*)number1->number=*(float*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==DOUBLE) {
			*(short*)number1->number=*(double*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==LONG_DOUBLE) {
			*(short*)number1->number=*(long double*)number0->number;
			CompilerErrorOrWarning(31);
		}
	}
	else if (type==UNSIGNED_SHORT) {
		if (number0->type==CHAR)
			*(unsigned short*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(unsigned short*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(unsigned short*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(unsigned short*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(unsigned short*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(unsigned short*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT) {
			*(unsigned short*)number1->number=*(float*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==DOUBLE) {
			*(unsigned short*)number1->number=*(double*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==LONG_DOUBLE) {
			*(unsigned short*)number1->number=*(long double*)number0->number;
			CompilerErrorOrWarning(31);
		}
	}
	else if (type==INT) {
		if (number0->type==CHAR)
			*(int*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(int*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(int*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(int*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(int*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(int*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT)
			*(int*)number1->number=*(float*)number0->number;
		else if (number0->type==DOUBLE) {
			*(int*)number1->number=*(double*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==LONG_DOUBLE) {
			*(int*)number1->number=*(long double*)number0->number;
			CompilerErrorOrWarning(31);
		}
	}
	else if (type==UNSIGNED_INT) {
		if (number0->type==CHAR)
			*(unsigned int*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(unsigned int*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(unsigned int*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(unsigned int*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(unsigned int*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(unsigned int*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT)
			*(unsigned int*)number1->number=*(float*)number0->number;
		else if (number0->type==DOUBLE) {
			*(unsigned int*)number1->number=*(double*)number0->number;
			CompilerErrorOrWarning(31);
		}
		else if (number0->type==LONG_DOUBLE) {
			*(unsigned int*)number1->number=*(long double*)number0->number;
			CompilerErrorOrWarning(31);
		}
	}
	else if (type==FLOAT) {
		if (number0->type==CHAR)
			*(float*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(float*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(float*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(float*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(float*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(float*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT)
			*(float*)number1->number=*(float*)number0->number;
		else if (number0->type==DOUBLE)
			*(float*)number1->number=*(double*)number0->number;
		else if (number0->type==LONG_DOUBLE)
			*(float*)number1->number=*(long double*)number0->number;
	}
	else if (type==DOUBLE) {
		if (number0->type==CHAR)
			*(double*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(double*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(double*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(double*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(double*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(double*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT)
			*(double*)number1->number=*(float*)number0->number;
		else if (number0->type==DOUBLE)
			*(double*)number1->number=*(double*)number0->number;
		else if (number0->type==LONG_DOUBLE)
			*(double*)number1->number=*(long double*)number0->number;
	}
	else if (type==LONG_DOUBLE) {
		if (number0->type==CHAR)
			*(long double*)number1->number=*(char*)number0->number;
		else if (number0->type==UNSIGNED_CHAR)
			*(long double*)number1->number=*(unsigned char*)number0->number;
		else if (number0->type==SHORT)
			*(long double*)number1->number=*(short*)number0->number;
		else if (number0->type==UNSIGNED_SHORT)
			*(long double*)number1->number=*(unsigned short*)number0->number;
		else if (number0->type==INT)
			*(long double*)number1->number=*(int*)number0->number;
		else if (number0->type==UNSIGNED_INT)
			*(long double*)number1->number=*(unsigned int*)number0->number;
		else if (number0->type==FLOAT)
			*(long double*)number1->number=*(float*)number0->number;
		else if (number0->type==DOUBLE)
			*(long double*)number1->number=*(double*)number0->number;
		else if (number0->type==LONG_DOUBLE)
			*(long double*)number1->number=*(long double*)number0->number;
	}
#pragma warning( default : 4244 ) // "conversion from 'xxx ' to 'xxx ', possible loss of data"

	// ritorna
	return;
}

//======================
//ParseStructDeclaration
//
//Processa una dichiarazione di struttura
//======================
int ParseStructDeclaration (char *pointer)
{
	int				i=0,j,k;
	int				ret;
	char			ident[256];
	int				struct_id;
	char			buffer[4096];
	int				num_members=0;
	char			buffer2[4096];
	int				no_type=0;

	// inizializza l'offset dei membri
	cur_fld_address=0;

	// elimina gli spazi iniziali
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// controlla che sia tutto ok con la keyword
	ret=IsIdentfString(&pointer[i]);
	if (ret != 6 || !CompareStrings(&pointer[i],"struct",6))
		return(1);
	else
		i+=6;

	// elimina gli spazi dopo la keyword
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// controlla se è stato dichiarato il nome del tipo
	if (ret=IsIdentfString(&pointer[i])) {
		if (ret>=identifier_max_len) {
			j=identifier_max_len-1;
			CompilerErrorOrWarning(1);
		}
		else
			j=ret;

		memcpy(ident,&pointer[i],j);
		ident[j]=0;

		// controlla che l'identificatore non sia una reserved word
		for (k=0;reswords[k].ptr!=NULL;k++)
			if (reswords[k].len == (signed)strlen(ident) &&
				CompareStrings(ident,reswords[k].ptr,reswords[k].len)) {
				CompilerErrorOrWarning(174);
				return(1);
			}

		// controlla se l'identificatore è già utilizzato
		for (k=0;k<cident_id;k++) {
			if (!strcmp(ident,cident[k].id)) {
				if ((cident[k].type & 0xff0000) == STRUCT ||
					(cident[k].type & 0xff0000) == VARIABLE ||
					(cident[k].type & 0xff0000) == FUNCTION) {
					CompilerErrorOrWarning(50);
					return(1);
				}
			}
		}

		i+=ret;
	}
	else {
		sprintf(ident,"_______%i",struct_num++);
		no_type=1;
	}

	// inizializza l'identificatore
	struct_id=cident_id++;

	cident[struct_id].type=STRUCT;
	strcpy(cident[struct_id].id,ident);
	cident[struct_id].address=-1;			// provvisorio
	cident[struct_id].fields=-1;			// provvisorio

	cident[struct_id].indirection=0;		// not used
	cident[struct_id].dimensions=0;			// not used
	cident[struct_id].const_flag=0;			// not used
	cident[struct_id].static_flag=0;		// not used
	cident[struct_id].pointer=NULL;			// not used
	cident[struct_id].initializer=NULL;		// not used
	cident[struct_id].struct_type=NULL;		// not used
	cident[struct_id].array_stptr=0;		// not used
	cident[struct_id].ellipses_arguments=0;	// not used
	cident[struct_id].initializer_line=0;	// not used

	// elimina gli spazi prima della parentesi graffa
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// controlla che ci sia la parentesi graffa
	if (!CompareStrings(&pointer[i],"{",1)) {
		cident_id=struct_id;
		CompilerErrorOrWarning(54);
		return(1);
	}
	else
		i++;

	// elimina gli spazi dopo la parentesi graffa
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// interpreta i vari membri della struttura
	while (1) {

		// isola la prossima dichiarazione di membro
		j=0;
		for	(;pointer[i]!=';';i++) {
			if (pointer[i]=='}') {
				cident_id=struct_id;
				CompilerErrorOrWarning(52);
				return(1);
			}
			else if (!pointer[i]) {
				cident_id=struct_id;
				CompilerErrorOrWarning(53);
				return(1);
			}
			buffer[j++]=pointer[i];
		}
		i++;
		buffer[j]=0;
		num_members++;

		// controlla che ci sia qualcosa in buffer
		if (!strlen(buffer)) {
			cident_id=struct_id;
			CompilerErrorOrWarning(56);
			return(1);
		}

		// processa la dichiarazione del membro di struttura
		if (ParseVariableDeclaration(buffer,1,0,0)) {
			cident_id=struct_id;
			return(1);
		}

		// elimina gli spazi dopo il punto e virgola
		for (;;i++)
			if ( pointer[i]!=' ' &&
				pointer[i]!='\t' )
				break;

		// controlla se la dichiarazione della struttura è terminata
		if (CompareStrings(&pointer[i],"}",1)) {
			i++;
			break;
		}

	}

	// memorizza il numero di membri della struttura ed il sizeof della struttura
	cident[struct_id].fields=num_members;
	cident[struct_id].address=cur_fld_address;

	// elimina gli spazi dopo la parentesi graffa chiusa
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// controlla se la dichiarazione è finita
	if (!pointer[i]) {
		if (no_type) {
			cident_id=struct_id;
			CompilerErrorOrWarning(55);
			return(1);
		}
		else
			return(0);
	}

	// inizializza le diverse variabili
	while (1) {

		// legge il prossimo identificatore
		j=0;
		for	(;pointer[i]!=',' && pointer[i];i++)
			buffer[j++]=pointer[i];
		buffer[j]=0;

		// controlla che ci sia qualcosa in buffer
		if (!strlen(buffer)) {
			cident_id=struct_id;
			CompilerErrorOrWarning(57);
			return(1);
		}

		// processa la dichiarazione di variabile
		sprintf(buffer2,"%s %s",ident,buffer);
		if (ParseVariableDeclaration(buffer2,0,0,0)) {
			cident_id=struct_id;
			return(1);
		}

		// controlla se la dichiarazione di variabili è finita
		if (!pointer[i])
			break;

		// elimina gli spazi dopo la virgola
		for (i++;;i++)
			if ( pointer[i]!=' ' &&
				pointer[i]!='\t' )
				break;
	}

	// ritorna
	return(0);
}

//======================
//ParseFunctionDeclaration
//
//Processa una dichiarazione di funzione
//======================
int ParseFunctionDeclaration (char *pointer)
{
	int				i=0,j;
	int				func_id;
	char			buffer[4096];
	int				num_parameters=0;
	int				to_exit=0;
	int				void_warning=0;
	int				ellipses_arguments=0;

	// resetta func_prototype
	func_prototype=NULL;

	// salva il corrente cident_id in caso di errori
	func_id=cident_id;

	// elimina gli spazi iniziali
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// isola l'id della funzione ed il tipo dei dati restituiti
	j=0;
	for	(;pointer[i]!='(';i++) {
		if (!pointer[i]) {
			CompilerErrorOrWarning(34);
			return(1);
		}
		buffer[j++]=pointer[i];
	}
	i++;
	buffer[j]=0;

	// processa l'id della funzione ed il tipo dei dati restituiti
	if (ParseVariableDeclaration(buffer,0,0,1)) {
		cident_id=func_id;
		return(1);
	}

	// elimina gli spazi dopo la parentesi tonda aperta
	for (;;i++)
		if ( pointer[i]!=' ' &&
			pointer[i]!='\t' )
			break;

	// interpreta i vari parametri della dichiarazione
	while (1) {

		// isola la prossima dichiarazione di parametro
		j=0;
		for	(;;i++) {
			if (pointer[i]==')') {
				to_exit=1;
				break;
			}
			else if (pointer[i]==',')
				break;
			else if (!pointer[i]) {
				cident_id=func_id;
				CompilerErrorOrWarning(34);
				return(1);
			}
			buffer[j++]=pointer[i];
		}
		i++;
		buffer[j]=0;
		num_parameters++;

		// controlla che ci sia qualcosa in buffer
		if (!strlen(buffer)) {
			if (num_parameters==1)
				strcpy(buffer,"void");
			else {
				cident_id=func_id;
				CompilerErrorOrWarning(69);
				return(1);
			}
		}

		// controlla per i ...
		if ( CompareStrings(buffer,"...",3) ) {

			// controlla che non sia il primo parametro e che sia l'ultimo
			if ( num_parameters == 1 || !to_exit || void_warning) {
				CompilerErrorOrWarning(63);
				return(1);
			}

			// controlla che non ci siano altri caratteri dopo i ...
			for (j=3;buffer[j];j++) {
				if ( buffer[j] != ' ' &&
					buffer[j] != '\t' ) {
					CompilerErrorOrWarning(63);
					return(1);
				}
			}

			// decrementa num_parameters e imposta a 1 ellipses_arguments
			num_parameters--;
			ellipses_arguments=1;

		}
		else {

			// processa la dichiarazione del parametro di funzione
			if (ParseVariableDeclaration(buffer,0,1,0)) {
				cident_id=func_id;
				return(1);
			}
			else if ( (cident[cident_id-1].type & 0xffff) == VOID_TYPE &&
				!cident[cident_id-1].indirection)
				void_warning=1;

		}

		// elimina gli spazi dopo la virgola o dopo la parentesi tonda chiusa
		for (;;i++)
			if ( pointer[i]!=' ' &&
				pointer[i]!='\t' )
				break;

		// controlla se la dichiarazione della funzione è terminata
		if (to_exit)
			break;

	}

	// memorizza il numero di parametri della funzione dichiarata
	cident[func_id].fields=num_parameters;

	// valuta se impostare ellipses_arguments
	if (ellipses_arguments)
		cident[func_id].ellipses_arguments=1;
	else
		cident[func_id].ellipses_arguments=0;

	// controlla che void come parametro sia utilizzato correttamente
	if (void_warning && num_parameters!=1) {
		cident_id=func_id;
		CompilerErrorOrWarning(74);
		return(1);
	}

	// controlla che la dichiarazione sia finita
	if (pointer[i]) {
		cident_id=func_id;
		CompilerErrorOrWarning(70);
		return(1);
	}

	// ritorna
	return(0);
}

//==================
//ResolveConstantOperation
//
//Risolve un'operazione di un'espressione costante
//==================
int ResolveConstantOperation (operator_t *op_ptr, number_t *buffer, token_t *tok_ptr, int op_id)
{
	int			i,j,k,l;
	number_t	n0,n1;

	// sizeof
	if (op_ptr[op_id].id==SIZE_OF_TYPE) {
		i=op_ptr[op_id].tok_id;
		buffer[i].type=INT;
		*(int*)buffer[i].number=tok_ptr[i].sizeof_result;
	}
	// meno
	else if (op_ptr[op_id].id==ARITHMETIC_NEGATION_UNARY) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;
		if (buffer[j].type==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=-*(char*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=-*(unsigned char*)buffer[j].number;
		}
		else if (buffer[j].type==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=-*(short*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=-*(unsigned short*)buffer[j].number;
		}
		else if (buffer[j].type==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=-*(int*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
#pragma warning( disable : 4146 ) // "unary minus operator applied to unsigned type, result still unsigned"
			*(unsigned int*)buffer[i].number=-*(unsigned int*)buffer[j].number;
			CompilerErrorOrWarning(29);
#pragma warning( default : 4146 ) // "unary minus operator applied to unsigned type, result still unsigned"
		}
		else if (buffer[j].type==FLOAT) {
			buffer[i].type=FLOAT;
			*(float*)buffer[i].number=-*(float*)buffer[j].number;
		}
		else if (buffer[j].type==DOUBLE) {
			buffer[i].type=DOUBLE;
			*(double*)buffer[i].number=-*(double*)buffer[j].number;
		}
		else if (buffer[j].type==LONG_DOUBLE) {
			buffer[i].type=LONG_DOUBLE;
			*(long double*)buffer[i].number=-*(long double*)buffer[j].number;
		}
	}
	// più
	else if (op_ptr[op_id].id==UNARY_PLUS) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;
			buffer[i]=buffer[j];
	}
	// complemento di uno
	else if (op_ptr[op_id].id==BITWISE_COMPLEMENT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;
		if (buffer[j].type==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=~*(char*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=~*(unsigned char*)buffer[j].number;
		}
		else if (buffer[j].type==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=~*(short*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=~*(unsigned short*)buffer[j].number;
		}
		else if (buffer[j].type==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=~*(int*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=~*(unsigned int*)buffer[j].number;
		}
		else if (buffer[j].type==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (buffer[j].type==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (buffer[j].type==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// logical not
	else if (op_ptr[op_id].id==LOGICAL_NOT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;
		if (buffer[j].type==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=!*(char*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=!*(unsigned char*)buffer[j].number;
		}
		else if (buffer[j].type==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=!*(short*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=!*(unsigned short*)buffer[j].number;
		}
		else if (buffer[j].type==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=!*(int*)buffer[j].number;
		}
		else if (buffer[j].type==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=!*(unsigned int*)buffer[j].number;
		}
		else if (buffer[j].type==FLOAT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=!*(float*)buffer[j].number;
			CompilerErrorOrWarning(31);
		}
		else if (buffer[j].type==DOUBLE) {
			buffer[i].type=DOUBLE;
			*(double*)buffer[i].number=!*(double*)buffer[j].number;
		}
		else if (buffer[j].type==LONG_DOUBLE) {
			buffer[i].type=LONG_DOUBLE;
			*(long double*)buffer[i].number=!*(long double*)buffer[j].number;
		}
	}
	// type cast
	else if (op_ptr[op_id].id==TYPE_CAST) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].third_tok;
		if (tok_ptr[i].typecast_indir ||
			(tok_ptr[i].typecast_type != CHAR &&
			tok_ptr[i].typecast_type != UNSIGNED_CHAR &&
			tok_ptr[i].typecast_type != SHORT &&
			tok_ptr[i].typecast_type != UNSIGNED_SHORT &&
			tok_ptr[i].typecast_type != INT &&
			tok_ptr[i].typecast_type != UNSIGNED_INT &&
			tok_ptr[i].typecast_type != FLOAT &&
			tok_ptr[i].typecast_type != DOUBLE &&
			tok_ptr[i].typecast_type != LONG_DOUBLE) ) {
			CompilerErrorOrWarning(32);
			return(1);
		}
		CastTypeOfConstantNumber(&buffer[j],&buffer[i],tok_ptr[i].typecast_type);
	}
	// moltiplicazione
	else if (op_ptr[op_id].id==MULTIPLICATION) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number * *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number * *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number * *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number * *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number * *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number * *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			buffer[i].type=FLOAT;
			*(float*)buffer[i].number=*(float*)n0.number * *(float*)n1.number;
		}
		else if (l==DOUBLE) {
			buffer[i].type=DOUBLE;
			*(double*)buffer[i].number=*(double*)n0.number * *(double*)n1.number;
		}
		else if (l==LONG_DOUBLE) {
			buffer[i].type=LONG_DOUBLE;
			*(long double*)buffer[i].number=*(long double*)n0.number * *(long double*)n1.number;
		}
	}
	// divisione
	else if (op_ptr[op_id].id==DIVISION) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number / *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number / *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number / *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number / *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number / *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number / *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			buffer[i].type=FLOAT;
			*(float*)buffer[i].number=*(float*)n0.number / *(float*)n1.number;
		}
		else if (l==DOUBLE) {
			buffer[i].type=DOUBLE;
			*(double*)buffer[i].number=*(double*)n0.number / *(double*)n1.number;
		}
		else if (l==LONG_DOUBLE) {
			buffer[i].type=LONG_DOUBLE;
			*(long double*)buffer[i].number=*(long double*)n0.number / *(long double*)n1.number;
		}
	}
	// modulo
	else if (op_ptr[op_id].id==REMAINDER) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number % *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number % *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number % *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number % *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number % *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number % *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// addizione
	else if (op_ptr[op_id].id==ADDITION) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number + *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number + *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number + *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number + *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number + *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number + *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			buffer[i].type=FLOAT;
			*(float*)buffer[i].number=*(float*)n0.number + *(float*)n1.number;
		}
		else if (l==DOUBLE) {
			buffer[i].type=DOUBLE;
			*(double*)buffer[i].number=*(double*)n0.number + *(double*)n1.number;
		}
		else if (l==LONG_DOUBLE) {
			buffer[i].type=LONG_DOUBLE;
			*(long double*)buffer[i].number=*(long double*)n0.number + *(long double*)n1.number;
		}
	}
	// sottrazione
	else if (op_ptr[op_id].id==SUBTRACTION) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number - *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number - *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number - *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number - *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number - *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number - *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			buffer[i].type=FLOAT;
			*(float*)buffer[i].number=*(float*)n0.number - *(float*)n1.number;
		}
		else if (l==DOUBLE) {
			buffer[i].type=DOUBLE;
			*(double*)buffer[i].number=*(double*)n0.number - *(double*)n1.number;
		}
		else if (l==LONG_DOUBLE) {
			buffer[i].type=LONG_DOUBLE;
			*(long double*)buffer[i].number=*(long double*)n0.number - *(long double*)n1.number;
		}
	}
	// slittamento a sinistra
	else if (op_ptr[op_id].id==LEFT_SHIFT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number << *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number << *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number << *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number << *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number << *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number << *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// slittamento a destra
	else if (op_ptr[op_id].id==RIGHT_SHIFT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number >> *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number >> *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number >> *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number >> *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number >> *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number >> *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// minore di
	else if (op_ptr[op_id].id==LESS_THAN) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number < *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number < *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number < *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number < *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number < *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number < *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number < *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number < *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number < *(long double*)n1.number;
	}
	// maggiore di
	else if (op_ptr[op_id].id==GREATER_THAN) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number > *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number > *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number > *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number > *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number > *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number > *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number > *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number > *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number > *(long double*)n1.number;
	}
	// minore o uguale a
	else if (op_ptr[op_id].id==LESS_THAN_OR_EQUAL_TO) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number <= *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number <= *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number <= *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number <= *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number <= *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number <= *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number <= *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number <= *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number <= *(long double*)n1.number;
	}
	// maggiore o uguale a
	else if (op_ptr[op_id].id==GREATER_THAN_OR_EQUAL_TO) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number >= *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number >= *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number >= *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number >= *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number >= *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number >= *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number >= *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number >= *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number >= *(long double*)n1.number;
	}
	// uguaglianza
	else if (op_ptr[op_id].id==EQUALITY) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number == *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number == *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number == *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number == *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number == *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number == *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number == *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number == *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number == *(long double*)n1.number;
	}
	// disuguaglianza
	else if (op_ptr[op_id].id==INEQUALITY) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number != *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number != *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number != *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number != *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number != *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number != *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number != *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number != *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number != *(long double*)n1.number;
	}
	// AND su singoli bit
	else if (op_ptr[op_id].id==BITWISE_AND) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number & *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number & *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number & *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number & *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number & *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number & *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// XOR su singoli bit
	else if (op_ptr[op_id].id==BITWISE_EXCLUSIVE_OR) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number ^ *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number ^ *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number ^ *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number ^ *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number ^ *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number ^ *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// OR su singoli bit
	else if (op_ptr[op_id].id==BITWISE_OR) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		if (l==CHAR) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(char*)n0.number | *(char*)n1.number;
		}
		else if (l==UNSIGNED_CHAR) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned char*)n0.number | *(unsigned char*)n1.number;
		}
		else if (l==SHORT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(short*)n0.number | *(short*)n1.number;
		}
		else if (l==UNSIGNED_SHORT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned short*)n0.number | *(unsigned short*)n1.number;
		}
		else if (l==INT) {
			buffer[i].type=INT;
			*(int*)buffer[i].number=*(int*)n0.number | *(int*)n1.number;
		}
		else if (l==UNSIGNED_INT) {
			buffer[i].type=UNSIGNED_INT;
			*(unsigned int*)buffer[i].number=*(unsigned int*)n0.number | *(unsigned int*)n1.number;
		}
		else if (l==FLOAT) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
		else if (l==LONG_DOUBLE) {
			CompilerErrorOrWarning(30);
			return(1);
		}
	}
	// AND logico
	else if (op_ptr[op_id].id==LOGICAL_AND) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number && *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number && *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number && *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number && *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number && *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number && *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number && *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number && *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number && *(long double*)n1.number;
	}
	// OR logico
	else if (op_ptr[op_id].id==LOGICAL_OR) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		if (buffer[j].type==buffer[k].type) {
			n0=buffer[j];
			n1=buffer[k];
			l=buffer[j].type;
		}
		else if (buffer[j].type>buffer[k].type) {
			n0=buffer[j];
			CastTypeOfConstantNumber(&buffer[k],&n1,buffer[j].type);
			l=buffer[j].type;
		}
		else {
			CastTypeOfConstantNumber(&buffer[j],&n0,buffer[k].type);
			n1=buffer[k];
			l=buffer[k].type;
		}

		buffer[i].type=INT;
		if (l==CHAR)
			*(int*)buffer[i].number=*(char*)n0.number || *(char*)n1.number;
		else if (l==UNSIGNED_CHAR)
			*(int*)buffer[i].number=*(unsigned char*)n0.number || *(unsigned char*)n1.number;
		else if (l==SHORT)
			*(int*)buffer[i].number=*(short*)n0.number || *(short*)n1.number;
		else if (l==UNSIGNED_SHORT)
			*(int*)buffer[i].number=*(unsigned short*)n0.number || *(unsigned short*)n1.number;
		else if (l==INT)
			*(int*)buffer[i].number=*(int*)n0.number || *(int*)n1.number;
		else if (l==UNSIGNED_INT)
			*(int*)buffer[i].number=*(unsigned int*)n0.number || *(unsigned int*)n1.number;
		else if (l==FLOAT)
			*(int*)buffer[i].number=*(float*)n0.number || *(float*)n1.number;
		else if (l==DOUBLE)
			*(int*)buffer[i].number=*(double*)n0.number || *(double*)n1.number;
		else if (l==LONG_DOUBLE)
			*(int*)buffer[i].number=*(long double*)n0.number || *(long double*)n1.number;
	}
	// condizionale
	else if (op_ptr[op_id].id==CONDITIONAL) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;
		l=op_ptr[op_id].third_tok;

		CastTypeOfConstantNumber(&buffer[j],&n0,LONG_DOUBLE);

		if (*(long double*)n0.number)
			buffer[i]=buffer[k];
		else
			buffer[i]=buffer[l];
	}

	// ritorna
	return(0);
}

//==================
//ResolveNonConstantOperation
//
//Genera il codice per la risoluzione di un'operazione di una espressione
//==================
int ResolveNonConstantOperation (operator_t *op_ptr, exprvalue_t *buffer, token_t *tok_ptr, int op_id)
{
	int				i,j,k,l,m,n,o,p,q;
	nonnumber_t		*nn0,*nn1;
	number_t		number;

	int				len;
	char			*pointer;
	char			ident[256];
	identifier_t	*prototype;

	identifier_t	*field;

	int				operation;
	int				with_pointers;
	int				eax_in_st0;
	int				control_if_zero;
	int				struct_assignment;

	int				objsub;

	instruction_t	*jump;

	instruction_t	*psi_pointer;

	int				special_char_assignment;

	int				ellipsis,e;
	identifier_t	ellipses_parameter;

	int				func_id;

	int				do_not_use_st0;

	// controlla se l'operatore appartiene sicuramente ad un'espressione non costante
	if ( (op_ptr[op_id].id & 0xff00) == 0x0000 )
		goto resolve;
	else if ( (op_ptr[op_id].id & 0xff00) == 0x0d00 )
		goto resolve;
	else if (op_ptr[op_id].id==PREFIX_INCREMENT ||
		op_ptr[op_id].id==PREFIX_DECREMENT ||
		op_ptr[op_id].id==ADDRESS_OF ||
		op_ptr[op_id].id==DEREFERENCE ||
		op_ptr[op_id].id==COMMA)
		goto resolve;
	else if ( (op_ptr[op_id].id & 0xff00) == 0xff00 )
		goto resolve;

	// controlla se gli operandi sono tutti numeri
	if ( (i=op_ptr[op_id].left_tok) != -1 )
		if (buffer[i].type != NUMBER)
			goto resolve;
	if ( (i=op_ptr[op_id].right_tok) != -1 )
		if (buffer[i].type != NUMBER)
			goto resolve;
	if ( (i=op_ptr[op_id].third_tok) != -1 )
		if (buffer[i].type != NUMBER)
			goto resolve;

	// risolve l'operazione tra numeri senza generare codice
	if ( (i=op_ptr[op_id].left_tok) != -1 )
		((number_t*)auxmem)[i]=buffer[i].number;
	if ( (i=op_ptr[op_id].right_tok) != -1 )
		((number_t*)auxmem)[i]=buffer[i].number;
	if ( (i=op_ptr[op_id].third_tok) != -1 )
		((number_t*)auxmem)[i]=buffer[i].number;

	// risolve l'operazione costante
	if (ResolveConstantOperation (op_ptr,auxmem,tok_ptr,op_id))
		return(1);

	// imposta correttamente il risultato in buffer
	i=op_ptr[op_id].tok_id;
	buffer[i].type=NUMBER;
	buffer[i].number=((number_t*)auxmem)[i];

	// ritorna
	return(0);

resolve:
	// elemento di matrice
	if (op_ptr[op_id].id==ARRAY_SUBSCRIPT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;

		// genera il codice (eax=argomento di destra)
		if (buffer[k].type==NOTHING_) {
			CompilerErrorOrWarning(90);
			return(1);
		}
		else if (buffer[k].type==NUMBER) {
			psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
			CastTypeOfConstantNumber(&buffer[k].number,&number,UNSIGNED_INT);
			psi_ptr->constant=*(int*)number.number;
		}
		else {
			nn1=&buffer[k].non_number;

			// controlla che a destra non ci sia una matrice o un puntatore
			if (nn1->indirection || nn1->dimensions) {
				CompilerErrorOrWarning(81);
				return(1);
			}

			// controlla se è un RIGHT_VALUE pointer
			if (nn1->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn1->address;
				psi_ptr++;

				// carica in eax o st0 l'argomento di destra
				if ( (nn1->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn1->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn1->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn1->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn1->type & 0xffff) == INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn1->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn1->type & 0xffff) == FLOAT ) {
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
					CompilerErrorOrWarning(78);
				}
				else if ( (nn1->type & 0xffff) == DOUBLE ) {
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					CompilerErrorOrWarning(79);
				}
				else if ( (nn1->type & 0xffff) == LONG_DOUBLE ) {
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					CompilerErrorOrWarning(80);
				}
				else {
					CompilerErrorOrWarning(77);
					return(1);
				}

			}
			// non è un RIGHT_VALUE pointer...
			else {

				// individua l'origine dell'argomento di destra
				if ( (nn1->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn1->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn1->address;

				// carica in eax o st0 l'argomento di destra
				if ( (nn1->type & 0xffff) == CHAR ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn1->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn1->type & 0xffff) == SHORT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn1->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn1->type & 0xffff) == INT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn1->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn1->type & 0xffff) == FLOAT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
					CompilerErrorOrWarning(78);
				}
				else if ( (nn1->type & 0xffff) == DOUBLE ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					CompilerErrorOrWarning(79);
				}
				else if ( (nn1->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					CompilerErrorOrWarning(80);
				}
				else {
					CompilerErrorOrWarning(77);
					return(1);
				}

			}

			// carica in eax il contenuto del primo registro del coprocessore
			if ( (nn1->type & 0xffff) >= FLOAT ) {
				psi_ptr++;
				psi_ptr->psi=LOAD_ST0_IN_EAX;
			}

		}
		psi_ptr++;

		// controlla che a sinistra ci sia effettivamente una matrice o un puntatore
		if (buffer[j].type==NOTHING_) {
			CompilerErrorOrWarning(90);
			return(1);
		}
		else if (buffer[j].type==NUMBER) {
			CompilerErrorOrWarning(75);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if (!nn0->indirection && !nn0->dimensions) {
			CompilerErrorOrWarning(76);
			return(1);
		}

		// calcola il fattore di moltiplicazione dell'indice
		if (nn0->dimensions) {
			if (nn0->indirection)
				m=4;
			else if ( (nn0->type & 0xff00) == CHAR )
				m=1;
			else if ( (nn0->type & 0xff00) == SHORT )
				m=2;
			else if ( (nn0->type & 0xff00) == INT )
				m=4;
			else if ( (nn0->type & 0xff00) == FLOAT )
				m=4;
			else if ( (nn0->type & 0xff00) == DOUBLE )
				m=8;
			else
				m=nn0->struct_type->address;

			for (l=1;l<nn0->dimensions;l++)
				m*=nn0->dimensions_list[l];
		}
		else {
			if (nn0->indirection==1) {
				if ( (nn0->type & 0xffff) == VOID_TYPE ) {
					CompilerErrorOrWarning(82);
					return(1);
				}
				else if ( (nn0->type & 0xff00) == CHAR )
					m=1;
				else if ( (nn0->type & 0xff00) == SHORT )
					m=2;
				else if ( (nn0->type & 0xff00) == INT )
					m=4;
				else if ( (nn0->type & 0xff00) == FLOAT )
					m=4;
				else if ( (nn0->type & 0xff00) == DOUBLE )
					m=8;
				else
					m=nn0->struct_type->address;
			}
			else
				m=4;
		}

		// valuta a questo punto che tipo di istruzione assemblare
		if (nn0->array_stptr) {

			// assembla le varie istruzioni
			psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
			psi_ptr->address.section=SECTION_STACK;
			psi_ptr->address.address=nn0->address;
			psi_ptr++;
			psi_ptr->psi=ARRAY_X_DIMENSION;
			psi_ptr->constant=m;

		}
		else if (nn0->dimensions &&
			( nn0->type & 0xff0000 ) != RIGHT_VALUE) {

			// evita di tradurre una semplice operazione tra numeri
			if ( (psi_ptr-1)->psi != LOAD_CONSTANT_IN_EAX ) {

				if ( ( nn0->type & 0xff0000 ) == LEFT_VALUE_0 ) {
					psi_ptr->psi=ARRAY_FIRST_DIMENSION;
					psi_ptr->address.section=SECTION_DATA;
				}
				else {
					psi_ptr->psi=STACK_ARRAY_FIRST_DIMENSION;
					psi_ptr->address.section=SECTION_STACK;
				}
				psi_ptr->address.address=nn0->address;
				psi_ptr->constant=m;

			}
			else {

				psi_ptr--;
				n=psi_ptr->constant;

				if ( ( nn0->type & 0xff0000 ) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_ECX;
					psi_ptr->address.section=SECTION_DATA;
				}
				else {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_ECX;
					psi_ptr->address.section=SECTION_STACK;
				}
				psi_ptr->address.address=nn0->address+n*m;

			}

		}
		else {

			// il puntatore è identificato da un RIGHT_VALUE pointer
			if (nn0->is_pointer) {
				psi_ptr->psi=LOAD_INT_IN_EDX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;
				psi_ptr->psi=LOAD_EDXPTR_INT_IN_ECX;
				psi_ptr++;
			}
			// il puntatore richiesto è riferito direttamente
			else {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
				}
				psi_ptr->address.address=nn0->address;
				psi_ptr++;
			}

			psi_ptr->psi=ARRAY_X_DIMENSION;
			psi_ptr->constant=m;
		}
		psi_ptr++;

		// in buffer per adesso memorizza le informazioni della vecchia matrice
		buffer[i]=buffer[j];

		// resetta array_stptr
		buffer[i].non_number.array_stptr=0;

		// memorizza il risultato nella zona di memoria temporanea riservata alle espressioni
		psi_ptr->psi=STORE_ECX_IN_INT;
		psi_ptr->address.address=cur_tmp_address;
		cur_tmp_address+=4;
		psi_ptr->address.section=SECTION_TEMPDATA1;

		// aggiusta le informazioni nel buffer
		buffer[i].non_number.type&=0xffff;
		buffer[i].non_number.type|=RIGHT_VALUE;
		buffer[i].non_number.address=psi_ptr->address.address;
		if (buffer[i].non_number.dimensions) {
			buffer[i].non_number.dimensions--;
			buffer[i].non_number.dimensions_list++;
		}
		else
			buffer[i].non_number.indirection--;

		// valuta se impostare is_pointer
		if (!buffer[i].non_number.dimensions &&
			!( (buffer[i].non_number.type & 0xffff) == STRUCTURE_TYPE &&
			!buffer[i].non_number.indirection) )
			buffer[i].non_number.is_pointer=1;

		// fa avanzare psi_ptr
		psi_ptr++;
	}
	// chiamata di funzione
	else if (op_ptr[op_id].id==FUNCTION_CALL) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;

		// individua il nome della funzione
		if (tok_ptr[j].id != IDENTIFIER) {
			CompilerErrorOrWarning(88);
			return(1);
		}
		pointer=tok_ptr[j].ptr0;
		len=strlen(pointer);
		if (len>=identifier_max_len) {
			len=identifier_max_len-1;
			CompilerErrorOrWarning(1);
		}
		memcpy(ident,pointer,len);
		ident[len]=0;

		for (l=0;l<cident_id;l++)
			if ( (cident[l].type & 0xff0000) == FUNCTION &&
				!strcmp(ident,cident[l].id) )
				break;
		if (l==cident_id) {
			CompilerErrorOrWarning(87);
			return(1);
		}
		else
			prototype=&cident[l];

		// controlla se in names c'è già l'identifier di questa funzione
		for (pointer=names;pointer<names_ptr;) {
			if (!strcmp(pointer,prototype->id))
				break;
			pointer+=strlen(pointer)+1;
		}
		if (pointer>=names_ptr) {
			pointer=names_ptr;
			strcpy(pointer,prototype->id);
			names_ptr+=strlen(pointer)+1;
		}

		// controlla se c'è l'argomento di destra e se c'è lo immette nello stack
		if (k!=-1) {
			if (prototype->fields!=1) {
				CompilerErrorOrWarning(86);
				return(1);
			}
			if (buffer[k].type==NOTHING_) {
				CompilerErrorOrWarning(90);
				return(1);
			}
			else if (buffer[k].type==NUMBER) {

				// controlla il prototipo della funzione
				if ( (prototype+1)->indirection ||
					(prototype+1)->dimensions ) {

					// se il numero è un floating point esce direttamente
					if (buffer[k].number.type >= FLOAT) {
						CompilerErrorOrWarning(89);
						return(1);
					}

					// converte in intero il numero
					if (buffer[k].number.type==CHAR)
						l=*(char*)buffer[k].number.number;
					else if (buffer[k].number.type==UNSIGNED_CHAR)
						l=*(unsigned char*)buffer[k].number.number;
					else if (buffer[k].number.type==SHORT)
						l=*(short*)buffer[k].number.number;
					else if (buffer[k].number.type==UNSIGNED_SHORT)
						l=*(unsigned short*)buffer[k].number.number;
					else if (buffer[k].number.type==INT)
						l=*(int*)buffer[k].number.number;
					else if (buffer[k].number.type==UNSIGNED_INT)
						l=*(unsigned int*)buffer[k].number.number;

					// continua solo se è uguale a zero
					if (l) {
						CompilerErrorOrWarning(89);
						return(1);
					}
					else
						m=UNSIGNED_INT;

				}
				else if ( ((prototype+1)->type & 0xffff) == STRUCTURE_TYPE ) {
					CompilerErrorOrWarning(167);
					return(1);
				}
				else
					m=(prototype+1)->type & 0xffff;

				// controlla se è possibile convertire
				if (m==VOID_TYPE) {
					CompilerErrorOrWarning(85);
					return(1);
				}
				CastTypeOfConstantNumber(&buffer[k].number,&number,m);

				if (number.type==CHAR) {
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*(char*)number.number;
				}
				else if (number.type==UNSIGNED_CHAR) {
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*(unsigned char*)number.number;
				}
				else if (number.type==SHORT) {
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*(short*)number.number;
				}
				else if (number.type==UNSIGNED_SHORT) {
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*(unsigned short*)number.number;
				}
				else if ( (number.type & 0xff00) == INT ||
					number.type == FLOAT ) {
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*(int*)number.number;
				}
				else if ( (number.type & 0xff00) == DOUBLE) {
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*((int*)number.number+1);
					psi_ptr++;
					psi_ptr->psi=PUSH_CONSTANT;
					psi_ptr->constant=*(int*)number.number;
				}
				psi_ptr++;
			}
			else if (PSI_PushNonNumber(&buffer[k].non_number,prototype+1))
				return(1);
		}
		else {

			// controlla se il numero di parametri passati è ok
			if (tok_ptr[i].func_call_commas+1 < prototype->fields) {
				CompilerErrorOrWarning(172);
				return(1);
			}
			else if ( !prototype->ellipses_arguments &&
				tok_ptr[i].func_call_commas+1 > prototype->fields) {
				CompilerErrorOrWarning(172);
				return(1);
			}
			else if (!tok_ptr[i].func_call_commas &&
				( ((prototype+1)->type & 0xffff) != VOID_TYPE ||
				(prototype+1)->dimensions ||
				(prototype+1)->indirection ) ) {
				CompilerErrorOrWarning(86);
				return(1);
			}

		}

		// calcola la dimensione del blocco dei parametri passati
		n=0;
		for (l=0;l<prototype->fields;l++) {
			m=(prototype+l+1)->address;
			if (m % 4)
				m=(m+4>>2)<<2;
			n+=m;
		}

		// aggiunge ad n la dimensione del blocco dei parametri omessi
		if ( prototype->ellipses_arguments )
			n+=tok_ptr[i].ellipses_arguments_dim;

		// imposta il codice per la chiamata di funzione
		psi_ptr->psi=CALL_FUNCTION;
		psi_ptr->constant=n;
		psi_ptr->address.section=SECTION_NAMES;
		psi_ptr->address.address=(int)pointer;
		psi_ptr++;

		// memorizza il risultato della funzione
		if (!prototype->indirection &&
			(prototype->type & 0xffff) == VOID_TYPE)
			buffer[i].type=NOTHING_;
		else {
			buffer[i].type=NON_NUMBER;
			nn0=&buffer[i].non_number;

			nn0->type=prototype->type & 0xffff;
			nn0->type|=RIGHT_VALUE;
			nn0->address=cur_tmp_address;
			nn0->indirection=prototype->indirection;
			nn0->dimensions=0;
			nn0->dimensions_list=NULL;
			nn0->const_flag=0;
			nn0->struct_type=prototype->struct_type;
			nn0->is_pointer=0;
			nn0->array_stptr=0;

			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=cur_tmp_address;
			if (nn0->indirection || (nn0->type & 0xffff) == STRUCTURE_TYPE) {
				psi_ptr->psi=STORE_EAX_IN_INT;
				cur_tmp_address+=4;
			}
			else {
				if ( (nn0->type & 0xff00) == CHAR ) {
					psi_ptr->psi=STORE_EAX_IN_CHAR;
					cur_tmp_address+=1;
				}
				else if ( (nn0->type & 0xff00) == SHORT ) {
					psi_ptr->psi=STORE_EAX_IN_SHORT;
					cur_tmp_address+=2;
				}
				else if ( (nn0->type & 0xff00) == INT ) {
					psi_ptr->psi=STORE_EAX_IN_INT;
					cur_tmp_address+=4;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					psi_ptr->psi=STORE_ST0_IN_FLOAT;
					cur_tmp_address+=4;
				}
				else if ( (nn0->type & 0xff00) == DOUBLE ) {
					psi_ptr->psi=STORE_ST0_IN_DOUBLE;
					cur_tmp_address+=8;
				}
			}
			psi_ptr++;
		}
	}
	// .
	else if (op_ptr[op_id].id==MEMBER_SELECTION_OBJECT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;

		// controlla che a sinistra ci sia una struttura
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(91);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if ( (nn0->type & 0xffff) != STRUCTURE_TYPE ||
			nn0->indirection ||
			nn0->dimensions ) {
			CompilerErrorOrWarning(91);
			return(1);
		}

		// controlla che a destra ci sia effettivamente un campo della struttura
		if (tok_ptr[k].id != IDENTIFIER) {
			CompilerErrorOrWarning(92);
			return(1);
		}
		pointer=tok_ptr[k].ptr0;
		m=nn0->struct_type->fields;
		for (l=0;l<m;l++)
			if (!strcmp(nn0->struct_type[l+1].id,pointer))
				break;
		if (l>=m) {
			CompilerErrorOrWarning(92);
			return(1);
		}
		field=&nn0->struct_type[l+1];

		// pone in ecx l'indirizzo del membro
		if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
			psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_ECX;
			psi_ptr->address.section=SECTION_DATA;
			psi_ptr->address.address=nn0->address+field->address;
		}
		else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
			psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_ECX;
			psi_ptr->address.section=SECTION_STACK;
			psi_ptr->address.address=nn0->address+field->address;
		}
		else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
			psi_ptr->psi=LOAD_INT_IN_ECX;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nn0->address;
			psi_ptr++;
			psi_ptr->psi=ECX_CONSTANT_ADDITION;
			psi_ptr->constant=field->address;
		}
		psi_ptr++;

		// salva nella SECTION_TEMPDATA1 il risultato dell'operazione
		buffer[i].type=NON_NUMBER;
		nn0=&buffer[i].non_number;

		nn0->type=field->type & 0xffff;
		nn0->type|=RIGHT_VALUE;
		nn0->address=cur_tmp_address;
		nn0->indirection=field->indirection;
		nn0->dimensions=field->dimensions;
		nn0->dimensions_list=field->pointer;
		nn0->const_flag=field->const_flag;
		nn0->struct_type=field->struct_type;
		nn0->array_stptr=0;

		if (!nn0->dimensions &&
			!( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
			!nn0->indirection) )
			nn0->is_pointer=1;
		else
			nn0->is_pointer=0;

		psi_ptr->address.section=SECTION_TEMPDATA1;
		psi_ptr->address.address=cur_tmp_address;
		psi_ptr->psi=STORE_ECX_IN_INT;
		cur_tmp_address+=4;
		psi_ptr++;
	}
	// ->
	else if (op_ptr[op_id].id==MEMBER_SELECTION_POINTER) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;

		// controlla che a sinistra ci sia un puntatore ad una struttura
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(93);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if ( (nn0->type & 0xffff) != STRUCTURE_TYPE ||
			nn0->indirection + nn0->dimensions != 1 ) {
			CompilerErrorOrWarning(93);
			return(1);
		}

		// controlla che a destra ci sia effettivamente un campo della struttura
		if (tok_ptr[k].id != IDENTIFIER) {
			CompilerErrorOrWarning(94);
			return(1);
		}
		pointer=tok_ptr[k].ptr0;
		m=nn0->struct_type->fields;
		for (l=0;l<m;l++)
			if (!strcmp(nn0->struct_type[l+1].id,pointer))
				break;
		if (l>=m) {
			CompilerErrorOrWarning(94);
			return(1);
		}
		field=&nn0->struct_type[l+1];

		// pone in ecx l'indirizzo del membro
		if (nn0->array_stptr) {

			// assembla le varie istruzioni
			psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
			psi_ptr->address.section=SECTION_STACK;
			psi_ptr->address.address=nn0->address;
			psi_ptr++;
			psi_ptr->psi=ECX_CONSTANT_ADDITION;
			psi_ptr->constant=field->address;
			psi_ptr++;

		}
		else if (nn0->dimensions) {
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
				psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_ECX;
				psi_ptr->address.section=SECTION_DATA;
				psi_ptr->address.address=nn0->address+field->address;
			}
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
				psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_ECX;
				psi_ptr->address.section=SECTION_STACK;
				psi_ptr->address.address=nn0->address+field->address;
			}
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;
				psi_ptr->psi=ECX_CONSTANT_ADDITION;
				psi_ptr->constant=field->address;
			}
			psi_ptr++;
		}
		else {
			if (nn0->is_pointer) {
				psi_ptr->psi=LOAD_INT_IN_EDX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;
				psi_ptr->psi=LOAD_EDXPTR_INT_IN_ECX;
			}
			else {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
				}
				psi_ptr->address.address=nn0->address;
			}
			psi_ptr++;
			psi_ptr->psi=ECX_CONSTANT_ADDITION;
			psi_ptr->constant=field->address;
			psi_ptr++;
		}

		// salva nella SECTION_TEMPDATA1 il risultato dell'operazione
		buffer[i].type=NON_NUMBER;
		nn0=&buffer[i].non_number;

		nn0->type=field->type & 0xffff;
		nn0->type|=RIGHT_VALUE;
		nn0->address=cur_tmp_address;
		nn0->indirection=field->indirection;
		nn0->dimensions=field->dimensions;
		nn0->dimensions_list=field->pointer;
		nn0->const_flag=field->const_flag;
		nn0->struct_type=field->struct_type;
		nn0->array_stptr=0;

		if (!nn0->dimensions &&
			!( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
			!nn0->indirection) )
			nn0->is_pointer=1;
		else
			nn0->is_pointer=0;

		psi_ptr->address.section=SECTION_TEMPDATA1;
		psi_ptr->address.address=cur_tmp_address;
		psi_ptr->psi=STORE_ECX_IN_INT;
		cur_tmp_address+=4;
		psi_ptr++;
	}
	// a++ / a-- / ++a / --a
	else if (op_ptr[op_id].id==POSTFIX_INCREMENT ||
		op_ptr[op_id].id==POSTFIX_DECREMENT ||
		op_ptr[op_id].id==PREFIX_INCREMENT ||
		op_ptr[op_id].id==PREFIX_DECREMENT) {
		i=op_ptr[op_id].tok_id;

		// considera l'argomento o a sinistra o a destra a seconda del gruppo dell'operatore
		if (op_ptr[op_id].id & 0xff00)
			j=op_ptr[op_id].right_tok;
		else
			j=op_ptr[op_id].left_tok;

		// controlla se l'argomento alla sinistra o alla destra dell'operatore è un lvalue
		if (buffer[j].type!=NON_NUMBER) {
			if (op_ptr[op_id].id==POSTFIX_INCREMENT)
				CompilerErrorOrWarning(95);
			else if (op_ptr[op_id].id==POSTFIX_DECREMENT)
				CompilerErrorOrWarning(96);
			else if (op_ptr[op_id].id==PREFIX_INCREMENT)
				CompilerErrorOrWarning(98);
			else if (op_ptr[op_id].id==PREFIX_DECREMENT)
				CompilerErrorOrWarning(99);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if ( nn0->dimensions ||
			( (nn0->type & 0xffff) == STRUCTURE_TYPE && !nn0->indirection ) ||
			( (nn0->type & 0xff0000) == RIGHT_VALUE && !nn0->is_pointer) ) {
			if (op_ptr[op_id].id==POSTFIX_INCREMENT)
				CompilerErrorOrWarning(95);
			else if (op_ptr[op_id].id==POSTFIX_DECREMENT)
				CompilerErrorOrWarning(96);
			else if (op_ptr[op_id].id==PREFIX_INCREMENT)
				CompilerErrorOrWarning(98);
			else if (op_ptr[op_id].id==PREFIX_DECREMENT)
				CompilerErrorOrWarning(99);
			return(1);
		}
		if (nn0->const_flag) {
			CompilerErrorOrWarning(97);
			return(1);
		}

		// considera se conservare il valore originario della variabile
		if ( (op_ptr[op_id].id & 0xff00) ||
			single_operator )
			buffer[i]=buffer[j];	// no...
		else {
			// preserva il valore dell'argomento prima di incrementarlo o decrementarlo
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
				if (nn0->indirection || 
					(nn0->type & 0xff00) == INT ||
					(nn0->type & 0xffff) == FLOAT) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=4;
				}
				else if ( (nn0->type & 0xff00) == CHAR ) {
					psi_ptr->psi=LOAD_CHAR_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_CHAR;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=1;
				}
				else if ( (nn0->type & 0xff00) == SHORT ) {
					psi_ptr->psi=LOAD_SHORT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_SHORT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=2;
				}
				else if ( (nn0->type & 0xff00) == DOUBLE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr++;
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.address=nn0->address+4;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr++;
					psi_ptr->psi=STORE_ECX_IN_INT;
					psi_ptr->address.address=cur_tmp_address+4;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=8;
				}
				psi_ptr++;
				buffer[i]=buffer[j];
				nn1=&buffer[i].non_number;
				nn1->type&=0xffff;
				nn1->type|=RIGHT_VALUE;
				nn1->address=k;
			}
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
				if (nn0->indirection || 
					(nn0->type & 0xff00) == INT ||
					(nn0->type & 0xffff) == FLOAT) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=4;
				}
				else if ( (nn0->type & 0xff00) == CHAR ) {
					psi_ptr->psi=LOAD_STACK_CHAR_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_CHAR;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=1;
				}
				else if ( (nn0->type & 0xff00) == SHORT ) {
					psi_ptr->psi=LOAD_STACK_SHORT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_SHORT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=2;
				}
				else if ( (nn0->type & 0xff00) == DOUBLE ) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr++;
					psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
					psi_ptr->address.address=nn0->address+4;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr++;
					psi_ptr->psi=STORE_ECX_IN_INT;
					psi_ptr->address.address=cur_tmp_address+4;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=8;
				}
				psi_ptr++;
				buffer[i]=buffer[j];
				nn1=&buffer[i].non_number;
				nn1->type&=0xffff;
				nn1->type|=RIGHT_VALUE;
				nn1->address=k;
			}
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// duplica la variabile
				if (nn0->indirection || 
					(nn0->type & 0xff00) == INT ||
					(nn0->type & 0xffff) == FLOAT) {
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=4;
				}
				else if ( (nn0->type & 0xff00) == CHAR ) {
					psi_ptr->psi=LOAD_ECXPTR_CHAR_IN_EAX;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_CHAR;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=1;
				}
				else if ( (nn0->type & 0xff00) == SHORT ) {
					psi_ptr->psi=LOAD_ECXPTR_SHORT_IN_EAX;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_SHORT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=2;
				}
				else if ( (nn0->type & 0xff00) == DOUBLE ) {
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr++;
					psi_ptr->psi=LOAD_ECXPLUS4PTR_INT_IN_EAX;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=cur_tmp_address+4;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					k=cur_tmp_address;
					cur_tmp_address+=8;
				}
				psi_ptr++;
				buffer[i]=buffer[j];
				nn1=&buffer[i].non_number;
				nn1->address=k;
				nn1->is_pointer=0;
			}
		}

		// carica in eax o st0 il valore della variabile
		if (nn0->indirection) {
			if ( (op_ptr[op_id].id & 0xff00) ||
				single_operator ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				}
				psi_ptr++;
			}
			if (nn0->indirection == 1) {
				if ( (nn0->type & 0xff00) == CHAR )
					k=1;
				else if ( (nn0->type & 0xff00) == SHORT )
					k=2;
				else if ( (nn0->type & 0xff00) == INT )
					k=4;
				else if ( (nn0->type & 0xffff) == FLOAT )
					k=4;
				else if ( (nn0->type & 0xff00) == DOUBLE )
					k=8;
				else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE )
					k=nn0->struct_type->address;
			}
			else
				k=4;
		}
		else {
			if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
				// carica in ecx il puntatore
				if ( (op_ptr[op_id].id & 0xff00) ||
					single_operator ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;
				}

				// carica in eax il contenuto del puntatore ecx
				if ( (op_ptr[op_id].id & 0xff00) ||
					single_operator ) {
					if ( (nn0->type & 0xffff) == CHAR ) {
						psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
						psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == SHORT ) {
						psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
						psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == INT ) {
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
						psi_ptr++;
					}
				}
				if ( (nn0->type & 0xffff) == FLOAT ) {
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
					psi_ptr++;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					psi_ptr++;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					psi_ptr++;
				}
			}
			else {
				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (op_ptr[op_id].id & 0xff00) ||
					single_operator ) {
					if ( (nn0->type & 0xffff) == CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
						else
							psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
						else
							psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
						else
							psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
						else
							psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == INT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
						psi_ptr++;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
						psi_ptr++;
					}
				}
				if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
					psi_ptr++;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					psi_ptr++;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					psi_ptr++;
				}
			}

			// calcola k
			if ( (nn0->type & 0xffff) >= FLOAT )
				k=-1;
			else
				k=1;
		}

		// incrementa o decrementa eax o st0
		if (k==-1) {
			*(int*)strings_ptr=0x3f800000;
			if (op_ptr[op_id].id==POSTFIX_INCREMENT || op_ptr[op_id].id==PREFIX_INCREMENT)
				psi_ptr->psi=ST0_FLOAT_ADDITION;
			else
				psi_ptr->psi=ST0_FLOAT_SUBTRACTION;
			psi_ptr->address.address=(int)strings_ptr;
			strings_ptr+=4;
			psi_ptr->address.section=SECTION_STRINGS;
		}
		else {
			if (op_ptr[op_id].id==POSTFIX_INCREMENT || op_ptr[op_id].id==PREFIX_INCREMENT)
				psi_ptr->psi=EAX_CONSTANT_ADDITION;
			else
				psi_ptr->psi=EAX_CONSTANT_SUBTRACTION;
			psi_ptr->constant=k;
		}
		psi_ptr++;

		// aggiorna il valore della variabile
		if (nn0->indirection) {
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
				psi_ptr->psi=STORE_EAX_IN_INT;
				psi_ptr->address.address=nn0->address;
				psi_ptr->address.section=SECTION_DATA;
			}
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
				psi_ptr->psi=STORE_EAX_IN_STACK_INT;
				psi_ptr->address.address=nn0->address;
				psi_ptr->address.section=SECTION_STACK;
			}
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
				psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
			psi_ptr++;
		}
		else {
			if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {

				// memorizza eax o st0 nella variabile puntata da ecx
				if ( (nn0->type & 0xff00) == CHAR )
					psi_ptr->psi=STORE_EAX_IN_ECXPTR_CHAR;
				else if ( (nn0->type & 0xff00) == SHORT )
					psi_ptr->psi=STORE_EAX_IN_ECXPTR_SHORT;
				else if ( (nn0->type & 0xff00) == INT )
					psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
				else if ( (nn0->type & 0xffff) == FLOAT )
					psi_ptr->psi=STORE_ST0_IN_ECXPTR_FLOAT;
				else if ( (nn0->type & 0xff00) == DOUBLE )
					psi_ptr->psi=STORE_ST0_IN_ECXPTR_DOUBLE;
				psi_ptr++;
			}
			else {
				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				psi_ptr->address.address=nn0->address;

				// memorizza eax o st0
				if ( (nn0->type & 0xff00) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=STORE_EAX_IN_STACK_CHAR;
					else
						psi_ptr->psi=STORE_EAX_IN_CHAR;
				}
				else if ( (nn0->type & 0xff00) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=STORE_EAX_IN_STACK_SHORT;
					else
						psi_ptr->psi=STORE_EAX_IN_SHORT;
				}
				else if ( (nn0->type & 0xff00) == INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=STORE_EAX_IN_STACK_INT;
					else
						psi_ptr->psi=STORE_EAX_IN_INT;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=STORE_ST0_IN_STACK_FLOAT;
					else
						psi_ptr->psi=STORE_ST0_IN_FLOAT;
				}
				else if ( (nn0->type & 0xff00) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=STORE_ST0_IN_STACK_DOUBLE;
					else
						psi_ptr->psi=STORE_ST0_IN_DOUBLE;
				}
				psi_ptr++;
			}
		}
	}
	// &
	else if (op_ptr[op_id].id==ADDRESS_OF) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;

		// controlla l'argomento alla destra dell'operatore
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(101);
			return(1);
		}
		nn0=&buffer[j].non_number;

		// ottiene l'indirizzo
		buffer[i]=buffer[j];
		nn1=&buffer[i].non_number;
		nn1->type&=0xffff;
		nn1->type|=RIGHT_VALUE;
		nn1->indirection++;
		nn1->dimensions=0;
		nn1->dimensions_list=NULL;
		nn1->is_pointer=0;
		nn1->array_stptr=0;
		if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
			psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
			psi_ptr->address.section=SECTION_DATA;
			psi_ptr->address.address=nn0->address;
		}
		else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
			psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
			psi_ptr->address.section=SECTION_STACK;
			psi_ptr->address.address=nn0->address;
		}
		else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
			if (!nn0->is_pointer) {
				if ( ( (nn0->type & 0xffff) != STRUCTURE_TYPE &&
					!nn0->dimensions ) ||
					( nn0->indirection &&
					!nn0->dimensions ) ) {
					CompilerErrorOrWarning(101);
					return(1);
				}
			}
			nn1->address=nn0->address;
			return(0);
		}
		psi_ptr++;
		psi_ptr->psi=STORE_EAX_IN_INT;
		psi_ptr->address.section=SECTION_TEMPDATA1;
		psi_ptr->address.address=cur_tmp_address;
		nn1->address=cur_tmp_address;
		cur_tmp_address+=4;
		psi_ptr++;
	}
	// *
	else if (op_ptr[op_id].id==DEREFERENCE) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;

		// controlla l'argomento alla destra dell'operatore
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(102);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if (!nn0->indirection && !nn0->dimensions) {
			CompilerErrorOrWarning(102);
			return(1);
		}

		// considera il caso di un array
		if (nn0->dimensions) {
			buffer[i]=buffer[j];
			nn1=&buffer[i].non_number;
			nn1->dimensions--;
			nn1->dimensions_list++;
			if ( (nn1->type & 0xff0000) == RIGHT_VALUE &&
				!nn1->dimensions &&
				!( (nn1->type & 0xffff) == STRUCTURE_TYPE &&
				!nn1->indirection) )
				nn1->is_pointer=1;
		}
		// considera il caso di un puntatore
		else {
			buffer[i]=buffer[j];
			nn1=&buffer[i].non_number;
			nn1->type&=0xffff;
			nn1->type|=RIGHT_VALUE;
			nn1->indirection--;
			if (!( (nn1->type & 0xffff) == STRUCTURE_TYPE &&
				!nn1->indirection) )
				nn1->is_pointer=1;
			else
				nn1->is_pointer=0;
			if (nn0->is_pointer) {
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			}
			else {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					return(0);
				psi_ptr->address.address=nn0->address;
			}
			psi_ptr++;
			psi_ptr->psi=STORE_EAX_IN_INT;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=cur_tmp_address;
			nn1->address=cur_tmp_address;
			cur_tmp_address+=4;
			psi_ptr++;
		}
	}
	// -a / ~a
	else if (op_ptr[op_id].id==ARITHMETIC_NEGATION_UNARY ||
		op_ptr[op_id].id==BITWISE_COMPLEMENT ) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;

		// controlla l'argomento alla destra dell'operatore
		if (buffer[j].type!=NON_NUMBER) {
			if ( op_ptr[op_id].id == BITWISE_COMPLEMENT )
				CompilerErrorOrWarning(106);
			else
				CompilerErrorOrWarning(103);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if (nn0->indirection || nn0->dimensions) {
			if ( op_ptr[op_id].id == BITWISE_COMPLEMENT )
				CompilerErrorOrWarning(106);
			else
				CompilerErrorOrWarning(103);
			return(1);
		}

		// controlla il tipo
		if ( op_ptr[op_id].id==BITWISE_COMPLEMENT &&
			(nn0->type & 0xffff) >= FLOAT ) {
			CompilerErrorOrWarning(105);
			return(1);
		}
		if ( op_ptr[op_id].id==ARITHMETIC_NEGATION_UNARY &&
			(nn0->type & 0xffff) > LONG_DOUBLE ) {
			CompilerErrorOrWarning(103);
			return(1);
		}

		// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
		if (nn0->is_pointer) {

			// carica in ecx il puntatore
			psi_ptr->psi=LOAD_INT_IN_ECX;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nn0->address;
			psi_ptr++;

			// carica in eax il contenuto del puntatore ecx
			if ( (nn0->type & 0xffff) == CHAR )
				psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
			else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
				psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
			else if ( (nn0->type & 0xffff) == SHORT )
				psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
			else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
				psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
			else if ( (nn0->type & 0xffff) == INT )
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			else if ( (nn0->type & 0xffff) == FLOAT )
				psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
			else if ( (nn0->type & 0xffff) == DOUBLE )
				psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
			else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
				psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

		}
		// carica in eax o st0 il valore
		else {

			// individua l'origine del valore
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
				psi_ptr->address.section=SECTION_DATA;
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
				psi_ptr->address.section=SECTION_STACK;
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
				psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nn0->address;

			// carica in eax o st0 il valore
			if ( (nn0->type & 0xffff) == CHAR ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
				else
					psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
				else
					psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == SHORT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
				else
					psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
				else
					psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == INT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
				else
					psi_ptr->psi=LOAD_INT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
				else
					psi_ptr->psi=LOAD_INT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == FLOAT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
				else
					psi_ptr->psi=LOAD_FLOAT_IN_ST0;
			}
			else if ( (nn0->type & 0xffff) == DOUBLE ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
				else
					psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
			}
			else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
				else
					psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
			}

		}
		psi_ptr++;

		// esegue l'operazione
		if ( op_ptr[op_id].id == ARITHMETIC_NEGATION_UNARY ) {
			if ( (nn0->type & 0xffff) < FLOAT )
				psi_ptr->psi=EAX_ARITHMETIC_NEGATION;
			else
				psi_ptr->psi=ST0_ARITHMETIC_NEGATION;
		}
		else
			psi_ptr->psi=EAX_BITWISE_COMPLEMENT;
		psi_ptr++;

		// memorizza il risultato
		buffer[i]=buffer[j];
		nn1=&buffer[i].non_number;

		nn1->type&=0xffff;
		if (nn1->type < INT) {
			if (nn1->type == CHAR)
				nn1->type=INT;
			else if (nn1->type == UNSIGNED_CHAR)
				nn1->type=UNSIGNED_INT;
			else if (nn1->type == SHORT)
				nn1->type=INT;
			else if (nn1->type == UNSIGNED_SHORT)
				nn1->type=UNSIGNED_INT;
		}
		nn1->type|=RIGHT_VALUE;
		nn1->address=cur_tmp_address;
		nn1->is_pointer=0;
		psi_ptr->address.section=SECTION_TEMPDATA1;
		psi_ptr->address.address=cur_tmp_address;

		if ( (nn1->type & 0xff00) == INT) {
			cur_tmp_address+=4;
			psi_ptr->psi=STORE_EAX_IN_INT;
		}
		else if ( (nn1->type & 0xffff) == FLOAT ) {
			cur_tmp_address+=4;
			psi_ptr->psi=STORE_ST0_IN_FLOAT;
		}
		else if ( (nn1->type & 0xff00) == DOUBLE ) {
			cur_tmp_address+=8;
			psi_ptr->psi=STORE_ST0_IN_DOUBLE;
		}
		psi_ptr++;
	}
	// +a
	else if (op_ptr[op_id].id==UNARY_PLUS) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;

		// controlla l'argomento alla destra dell'operatore
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(104);
			return(1);
		}
		nn0=&buffer[j].non_number;
		if (nn0->indirection || nn0->dimensions) {
			CompilerErrorOrWarning(104);
			return(1);
		}
		if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
			CompilerErrorOrWarning(104);
			return(1);
		}

		// l'operatore non modifica il valore della variabile...
		buffer[i]=buffer[j];
	}
	// !a
	else if (op_ptr[op_id].id==LOGICAL_NOT) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].right_tok;

		// controlla l'argomento alla destra dell'operatore
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(107);
			return(1);
		}
		nn0=&buffer[j].non_number;

		// controlla che ! non sia applicato ad una struttura
		if (!nn0->dimensions &&
			!nn0->indirection &&
			(nn0->type & 0xffff) == STRUCTURE_TYPE) {
			CompilerErrorOrWarning(108);
			return(1);
		}

		// applicato su un array (questo è un caso banale; il risultato è sempre 0)
		if (nn0->dimensions) {
			psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
			psi_ptr->constant=0;
			psi_ptr++;
		}
		// applicato su un puntatore o su una variabile
		else {

			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			if (nn0->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// carica in eax il contenuto del puntatore ecx
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == FLOAT )
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
				else if ( (nn0->type & 0xffff) == DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}
			}
			psi_ptr++;
		}

		// esegue l'operazione ponendo il risultato in eax
		if ( (nn0->type & 0xffff) >= FLOAT )
			psi_ptr->psi=ST0_LOGICAL_NOT;
		else
			psi_ptr->psi=EAX_LOGICAL_NOT;
		psi_ptr++;

		// memorizza il risultato
		buffer[i].type=NON_NUMBER;
		nn1=&buffer[i].non_number;
		nn1->type=INT;
		nn1->type|=RIGHT_VALUE;
		nn1->address=cur_tmp_address;
		nn1->indirection=0;
		nn1->dimensions=0;
		nn1->dimensions_list=NULL;
		nn1->const_flag=0;
		nn1->struct_type=NULL;
		nn1->is_pointer=0;
		nn1->array_stptr=0;

		psi_ptr->psi=STORE_EAX_IN_INT;
		psi_ptr->address.section=SECTION_TEMPDATA1;
		psi_ptr->address.address=cur_tmp_address;
		cur_tmp_address+=4;
		psi_ptr++;
	}
	// (...)a
	else if (op_ptr[op_id].id==TYPE_CAST) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].third_tok;

		// esegue la conversione del tipo
		if (buffer[j].type!=NON_NUMBER) {
			CompilerErrorOrWarning(109);
			return(1);
		}
		nn0=&buffer[j].non_number;
		buffer[i].type=NON_NUMBER;
		nn1=&buffer[i].non_number;
		nn1->type=tok_ptr[i].typecast_type;
		nn1->indirection=tok_ptr[i].typecast_indir;
		nn1->dimensions=0;
		nn1->dimensions_list=NULL;
		nn1->const_flag=nn0->const_flag;
		nn1->struct_type=tok_ptr[i].typecast_stype;
		nn1->is_pointer=0;
		nn1->array_stptr=0;
		if (CastTypeOfNonConstantNumber(nn0,nn1,0))
			return(1);
	}
	// a,b
	else if (op_ptr[op_id].id==COMMA) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;

		// virgola come operatore
		if (tok_ptr[i].comma_number==-1)
			// puramente formale...
			buffer[i]=buffer[j];
		// virgola come separatore
		else {

			// incrementa func_call_commas
			tok_ptr[tok_ptr[i].comma_func_tok].func_call_commas++;

			// trova l'argomento di sinistra del function call
			for (m=op_id-1;m>=0;m--)
				if (op_ptr[m].tok_id == tok_ptr[i].comma_func_tok)
					break;
			if (m<0) {
				CompilerErrorOrWarning(166);
				return(1);
			}

			// imposta func_id
			func_id=op_ptr[m].tok_id;

			// in m l'id del token dell'argomento di sinistra del function call
			m=op_ptr[m].left_tok;

			// individua il nome della funzione
			if (tok_ptr[m].id != IDENTIFIER) {
				CompilerErrorOrWarning(88);
				return(1);
			}
			pointer=tok_ptr[m].ptr0;
			len=strlen(pointer);
			if (len>=identifier_max_len) {
				len=identifier_max_len-1;
				CompilerErrorOrWarning(1);
			}
			memcpy(ident,pointer,len);
			ident[len]=0;

			for (l=0;l<cident_id;l++)
				if ( (cident[l].type & 0xff0000) == FUNCTION &&
					!strcmp(ident,cident[l].id) )
					break;
			if (l==cident_id) {
				CompilerErrorOrWarning(87);
				return(1);
			}
			else
				prototype=&cident[l];

			// controlla se ci sono troppi parametri nella chiamata di funzione
			ellipsis=0;
			if (prototype->fields < tok_ptr[i].comma_number+2) {
				if ( prototype->ellipses_arguments )
					ellipsis=1;
				else {
					CompilerErrorOrWarning(85);
					return(1);
				}
			}

			// controlla se in names c'è già l'identifier di questa funzione
			for (pointer=names;pointer<names_ptr;) {
				if (!strcmp(pointer,prototype->id))
					break;
				pointer+=strlen(pointer)+1;
			}
			if (pointer>=names_ptr) {
				pointer=names_ptr;
				strcpy(pointer,prototype->id);
				names_ptr+=strlen(pointer)+1;
			}

			// considera la posizione della virgola ed il numero di parametri da passare
			if (tok_ptr[i].comma_number)
				l=1;
			else
				l=2;

			// immette nello stack uno o due parametri a seconda della posizione della virgola
			for (m=0;m<l;m++) {

				// stabilisce l'id del parametro e la sua posizione nella chiamata di funzione
				if (m) {

					// imposta n ed o
					n=j;
					o=tok_ptr[i].comma_number;

					// imposta e ( = 1 : extra parameter ) a 0
					e=0;

				}
				else {

					// imposta n ed o
					n=k;
					o=tok_ptr[i].comma_number+1;

					// imposta e ( = 1 : extra parameter )
					if (ellipsis)
						e=1;
					else
						e=0;

				}

				// controlla che il tipo del parametro non sia void
				if (buffer[n].type==NOTHING_) {
					CompilerErrorOrWarning(90);
					return(1);
				}
				// immette nello stack un numero
				else if (buffer[n].type==NUMBER) {

					// considera se dobbiamo inserire nello stack un ellipses_argument
					if (e) {

						// imposta p ( il tipo del numero da inserire )
						p=buffer[n].number.type;
						if (p >= FLOAT) {
							p=DOUBLE;
							tok_ptr[func_id].ellipses_arguments_dim+=8;
						}
						else
							tok_ptr[func_id].ellipses_arguments_dim+=4;

					}
					else {

						// controlla il prototipo della funzione
						if ( (prototype+o+1)->indirection ||
							(prototype+o+1)->dimensions ) {

							// se il numero è un floating point esce direttamente
							if (buffer[n].number.type >= FLOAT) {
								CompilerErrorOrWarning(89);
								return(1);
							}

							// converte in intero il numero
							if (buffer[n].number.type==CHAR)
								p=*(char*)buffer[n].number.number;
							else if (buffer[n].number.type==UNSIGNED_CHAR)
								p=*(unsigned char*)buffer[n].number.number;
							else if (buffer[n].number.type==SHORT)
								p=*(short*)buffer[n].number.number;
							else if (buffer[n].number.type==UNSIGNED_SHORT)
								p=*(unsigned short*)buffer[n].number.number;
							else if (buffer[n].number.type==INT)
								p=*(int*)buffer[n].number.number;
							else if (buffer[n].number.type==UNSIGNED_INT)
								p=*(unsigned int*)buffer[n].number.number;

							// continua solo se è uguale a zero
							if (p) {
								CompilerErrorOrWarning(89);
								return(1);
							}
							else
								p=UNSIGNED_INT;

						}
						else if ( ((prototype+o+1)->type & 0xffff) == STRUCTURE_TYPE ) {
							CompilerErrorOrWarning(167);
							return(1);
						}
						else
							p=(prototype+o+1)->type & 0xffff;

					}

					CastTypeOfConstantNumber(&buffer[n].number,&number,p);
					if (number.type==CHAR) {
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*(char*)number.number;
					}
					else if (number.type==UNSIGNED_CHAR) {
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*(unsigned char*)number.number;
					}
					else if (number.type==SHORT) {
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*(short*)number.number;
					}
					else if (number.type==UNSIGNED_SHORT) {
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*(unsigned short*)number.number;
					}
					else if ( (number.type & 0xff00) == INT ||
						number.type == FLOAT ) {
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*(int*)number.number;
					}
					else if ( (number.type & 0xff00) == DOUBLE) {
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*((int*)number.number+1);
						psi_ptr++;
						psi_ptr->psi=PUSH_CONSTANT;
						psi_ptr->constant=*(int*)number.number;
					}
					psi_ptr++;
				}
				// immette nello stack un nonnumber_t
				else if (!e) {
					if  ( PSI_PushNonNumber(&buffer[n].non_number,prototype+o+1) )
						return(1);
				}
				// deve immettere nello stack un ellipses_argument
				else {

					// imposta ellipses_parameter
					ellipses_parameter.type = buffer[n].non_number.type & 0xffff;
					if ( ellipses_parameter.type == FLOAT &&
						!buffer[n].non_number.indirection &&
						!buffer[n].non_number.dimensions )
						ellipses_parameter.type = DOUBLE;
					ellipses_parameter.type |= FUNCTION_PARAMETER;
					*ellipses_parameter.id=0;
					if ( buffer[n].non_number.indirection || buffer[n].non_number.dimensions)
						q=4;
					else if ( (buffer[n].non_number.type & 0xffff) >= FLOAT ) {
						if ( (buffer[n].non_number.type & 0xffff) != STRUCTURE_TYPE )
							q=8;
						else {
							q=buffer[n].non_number.struct_type->address;
							if (q % 4)
								q=(q+4>>2)<<2;
						}
					}
					else
						q=4;
					ellipses_parameter.address=q;
					tok_ptr[func_id].ellipses_arguments_dim+=q;
					ellipses_parameter.fields=0;
					ellipses_parameter.indirection=buffer[n].non_number.indirection;
					ellipses_parameter.dimensions=buffer[n].non_number.dimensions;
					ellipses_parameter.const_flag=buffer[n].non_number.const_flag;
					ellipses_parameter.static_flag=0;
					ellipses_parameter.pointer=buffer[n].non_number.dimensions_list;
					ellipses_parameter.initializer=NULL;
					ellipses_parameter.struct_type=buffer[n].non_number.struct_type;
					ellipses_parameter.array_stptr=0;
					ellipses_parameter.ellipses_arguments=0;
					ellipses_parameter.initializer_line=0;

					// immette nello stack il parametro omesso nella dichiarazione
					if  ( PSI_PushNonNumber(&buffer[n].non_number,&ellipses_parameter) )
						return(1);

				}

			}
		}
	}
	// a&&b / a||b
	else if ( op_ptr[op_id].id==LOGICAL_AND_STEP0 ||
		op_ptr[op_id].id==LOGICAL_OR_STEP0 ||
		op_ptr[op_id].id==LOGICAL_AND_STEP1 ||
		op_ptr[op_id].id==LOGICAL_OR_STEP1 ) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].third_tok;

		// carica in eax o st0 l'operando
		if (buffer[j].type==NON_NUMBER) {

			// imposta nn0 e verifica che non sia una struttura
			nn0=&buffer[j].non_number;
			if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
				!nn0->dimensions &&
				!nn0->indirection ) {
				CompilerErrorOrWarning(144);
				return(1);
			}

			// carica in eax l'indirizzo della matrice
			if (nn0->dimensions) {
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}
			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// carica in eax o st0 il contenuto del puntatore ecx
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection)
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == FLOAT )
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
				else if ( (nn0->type & 0xffff) == DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}

			}
			psi_ptr++;

		}
		else if (buffer[j].type==NUMBER) {

			// valuta se caricare il numero in eax oppure in st0
			if ( buffer[j].number.type > FLOAT )
				m=DOUBLE;
			else if ( buffer[j].number.type == FLOAT )
				m=FLOAT;
			else if (buffer[j].number.type == UNSIGNED_CHAR ||
				buffer[j].number.type == UNSIGNED_SHORT ||
				buffer[j].number.type == UNSIGNED_INT)
				m=UNSIGNED_INT;
			else
				m=INT;
			CastTypeOfConstantNumber(&buffer[j].number,&number,m);

			// carica il numero in eax o st0
			if (m==DOUBLE) {
				*(double*)strings_ptr=*(double*)number.number;
				psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=8;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else if (m==FLOAT) {
				*(float*)strings_ptr=*(float*)number.number;
				psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=4;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else {
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=*(int*)number.number;
			}
			psi_ptr++;

		}
		else {
			CompilerErrorOrWarning(145);
			return(1);
		}

		// stabilisce quale istruzione assemblare per il salto
		m=0;
		if (buffer[j].type==NON_NUMBER) {
			if ( !nn0->dimensions &&
				!nn0->indirection &&
				(nn0->type & 0xffff) >= FLOAT )
				m=1;
		}
		else if ( buffer[j].number.type >= FLOAT )
			m=1;

		// assembla le istruzioni a seconda dell'operatore
		if ( op_ptr[op_id].id==LOGICAL_AND_STEP0 ||
			op_ptr[op_id].id==LOGICAL_OR_STEP0 ) {

			// assembla l'istruzione per il salto
			if ( op_ptr[op_id].id==LOGICAL_AND_STEP0 ) {
				if (m)
					psi_ptr->psi=JUMP_IF_ST0_ZERO;
				else
					psi_ptr->psi=JUMP_IF_EAX_ZERO;
			}
			else if ( op_ptr[op_id].id==LOGICAL_OR_STEP0 ) {
				if (m)
					psi_ptr->psi=JUMP_IF_ST0_NOT_ZERO;
				else
					psi_ptr->psi=JUMP_IF_EAX_NOT_ZERO;
			}

			// memorizza l'indirizzo di questa PSI per impostare l'indirizzo del salto successivamente
			op_ptr[op_id].pointer=psi_ptr++;

		}
		else {

			// assembla le istruzioni a seconda dell'operatore
			if ( op_ptr[op_id].id==LOGICAL_AND_STEP1 ) {

				// cerca il relativo LOGICAL_AND_STEP_0
				for (n=op_id+1;;n++)
					if ( op_ptr[n].id==LOGICAL_AND_STEP0 &&
						op_ptr[op_id].tok_id==op_ptr[n].tok_id )
						break;

				// imposta l'istruzione del salto per il secondo operando
				if (m)
					psi_ptr->psi=JUMP_IF_ST0_NOT_ZERO;
				else
					psi_ptr->psi=JUMP_IF_EAX_NOT_ZERO;
				jump=psi_ptr++;

				// imposta l'indirizzo del salto del primo operando
				((instruction_t*)op_ptr[n].pointer)->address.address=(int)psi_ptr;
				((instruction_t*)op_ptr[n].pointer)->address.section=SECTION_PSI;

				// imposta le seguenti istruzioni
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=0;
				psi_ptr++;
				jump->address.address=(int)(psi_ptr+1);
				jump->address.section=SECTION_PSI;
				psi_ptr->psi=JUMP;
				jump=psi_ptr++;
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=1;
				psi_ptr++;
				jump->address.address=(int)psi_ptr;
				jump->address.section=SECTION_PSI;

			}
			else if ( op_ptr[op_id].id==LOGICAL_OR_STEP1 ) {

				// cerca il relativo LOGICAL_OR_STEP_0
				for (n=op_id+1;;n++)
					if ( op_ptr[n].id==LOGICAL_OR_STEP0 &&
						op_ptr[op_id].tok_id==op_ptr[n].tok_id )
						break;

				// imposta l'istruzione del salto per il secondo operando
				if (m)
					psi_ptr->psi=JUMP_IF_ST0_ZERO;
				else
					psi_ptr->psi=JUMP_IF_EAX_ZERO;
				jump=psi_ptr++;

				// imposta l'indirizzo del salto del primo operando
				((instruction_t*)op_ptr[n].pointer)->address.address=(int)psi_ptr;
				((instruction_t*)op_ptr[n].pointer)->address.section=SECTION_PSI;

				// imposta le seguenti istruzioni
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=1;
				psi_ptr++;
				jump->address.address=(int)(psi_ptr+1);
				jump->address.section=SECTION_PSI;
				psi_ptr->psi=JUMP;
				jump=psi_ptr++;
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=0;
				psi_ptr++;
				jump->address.address=(int)psi_ptr;
				jump->address.section=SECTION_PSI;

			}

			// assembla l'istruzione per la memorizzazione del risultato
			psi_ptr->psi=STORE_EAX_IN_INT;
			psi_ptr->address.address=cur_tmp_address;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr++;

			// imposta le informazioni sul tipo del risultato dell'operazione
			buffer[i].type=NON_NUMBER;
			nn0=&buffer[i].non_number;
			nn0->type=INT;
			nn0->type|=RIGHT_VALUE;
			nn0->address=cur_tmp_address;
			nn0->indirection=0;
			nn0->dimensions=0;
			nn0->dimensions_list=NULL;
			nn0->const_flag=0;
			nn0->struct_type=NULL;
			nn0->is_pointer=0;
			nn0->array_stptr=0;
			cur_tmp_address+=4;

		}

	}
	// E1?E2:E3
	else if ( op_ptr[op_id].id==CONDITIONAL_STEP0 ) {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].third_tok;

		// carica in eax o st0 l'operando
		if (buffer[j].type==NON_NUMBER) {

			// imposta nn0 e verifica che non sia una struttura
			nn0=&buffer[j].non_number;
			if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
				!nn0->dimensions &&
				!nn0->indirection ) {
				CompilerErrorOrWarning(168);
				return(1);
			}

			// carica in eax l'indirizzo della matrice
			if (nn0->dimensions) {
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}
			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// carica in eax o st0 il contenuto del puntatore ecx
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection)
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == FLOAT )
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
				else if ( (nn0->type & 0xffff) == DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}

			}
			psi_ptr++;

		}
		else if (buffer[j].type==NUMBER) {

			// valuta se caricare il numero in eax oppure in st0
			if ( buffer[j].number.type > FLOAT )
				m=DOUBLE;
			else if ( buffer[j].number.type == FLOAT )
				m=FLOAT;
			else if (buffer[j].number.type == UNSIGNED_CHAR ||
				buffer[j].number.type == UNSIGNED_SHORT ||
				buffer[j].number.type == UNSIGNED_INT)
				m=UNSIGNED_INT;
			else
				m=INT;
			CastTypeOfConstantNumber(&buffer[j].number,&number,m);

			// carica il numero in eax o st0
			if (m==DOUBLE) {
				*(double*)strings_ptr=*(double*)number.number;
				psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=8;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else if (m==FLOAT) {
				*(float*)strings_ptr=*(float*)number.number;
				psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=4;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else {
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=*(int*)number.number;
			}
			psi_ptr++;

		}
		else {
			CompilerErrorOrWarning(169);
			return(1);
		}

		// stabilisce quale istruzione assemblare per il salto
		m=0;
		if (buffer[j].type==NON_NUMBER) {
			if ( !nn0->dimensions &&
				!nn0->indirection &&
				(nn0->type & 0xffff) >= FLOAT )
				m=1;
		}
		else if ( buffer[j].number.type >= FLOAT )
			m=1;

		// assembla l'istruzione per il salto
		if (m)
			psi_ptr->psi=JUMP_IF_ST0_ZERO;
		else
			psi_ptr->psi=JUMP_IF_EAX_ZERO;

		// memorizza l'indirizzo di questa PSI per impostare l'indirizzo del salto successivamente
		op_ptr[op_id].pointer=psi_ptr++;

	}
	// E1?E2:E3
	else if ( op_ptr[op_id].id==CONDITIONAL_STEP1 ) {

		// salva l'indirizzo delle istruzioni relative al secondo operando
		op_ptr[op_id].pointer=psi_ptr;

		// riserva uno spazio di istruzioni nel codice PSI
		for (m=0;m<10;m++,psi_ptr++)
			psi_ptr->psi=NOP;

		// cerca il relativo CONDITIONAL_STEP0
		for (n=op_id+1;;n++)
			if ( op_ptr[n].id==CONDITIONAL_STEP0 &&
				op_ptr[op_id].tok_id==op_ptr[n].tok_id )
				break;

		// imposta l'indirizzo del salto del primo operando
		((instruction_t*)op_ptr[n].pointer)->address.address=(int)psi_ptr;
		((instruction_t*)op_ptr[n].pointer)->address.section=SECTION_PSI;

	}
	// E1?E2:E3
	else if ( op_ptr[op_id].id==CONDITIONAL_STEP2 ) {
		i=op_ptr[op_id].tok_id;

		// cerca il relativo CONDITIONAL_STEP1
		for (n=op_id+1;;n++)
			if ( op_ptr[n].id==CONDITIONAL_STEP1 &&
				op_ptr[op_id].tok_id==op_ptr[n].tok_id )
				break;
		j=op_ptr[n].third_tok;
		k=op_ptr[op_id].third_tok;

		// imposta psi_pointer
		psi_pointer=(instruction_t*)op_ptr[n].pointer;

		// carica in eax o st0 il secondo argomento
		if (buffer[j].type==NON_NUMBER) {

			// imposta nn0
			nn0=&buffer[j].non_number;

			// carica in eax l'indirizzo della matrice
			if (nn0->dimensions) {

				// controlla il terzo operando
				if (buffer[k].type!=NON_NUMBER) {
					if (buffer[k].type!=NUMBER) {
						CompilerErrorOrWarning(170);
						return(1);
					}
					else {

						// se il numero è un floating point esce direttamente
						if (buffer[k].number.type >= FLOAT) {
							CompilerErrorOrWarning(170);
							return(1);
						}

						// converte in intero il numero
						if (buffer[k].number.type==CHAR)
							m=*(char*)buffer[k].number.number;
						else if (buffer[k].number.type==UNSIGNED_CHAR)
							m=*(unsigned char*)buffer[k].number.number;
						else if (buffer[k].number.type==SHORT)
							m=*(short*)buffer[k].number.number;
						else if (buffer[k].number.type==UNSIGNED_SHORT)
							m=*(unsigned short*)buffer[k].number.number;
						else if (buffer[k].number.type==INT)
							m=*(int*)buffer[k].number.number;
						else if (buffer[k].number.type==UNSIGNED_INT)
							m=*(unsigned int*)buffer[k].number.number;

						// continua solo se è uguale a zero
						if (m) {
							CompilerErrorOrWarning(170);
							return(1);
						}

					}
				}
				else {
					nn1=&buffer[k].non_number;

					// verifica che il terzo operando sia un puntatore compatibile
					if (!nn1->dimensions && !nn1->indirection) {
						CompilerErrorOrWarning(170);
						return(1);
					}
					else {

						// controlla il tipo dei puntatori
						if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) ) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
							nn0->struct_type != nn1->struct_type ) {
							CompilerErrorOrWarning(170);
							return(1);
						}

						// controlla il level of indirection dei puntatori
						if (nn0->dimensions != nn1->dimensions ||
							nn0->indirection != nn1->indirection) {

							// controlla i vari casi
							if (nn0->dimensions==1) {
								if (nn1->dimensions ||
									nn0->indirection + 1 != nn1->indirection) {
									CompilerErrorOrWarning(170);
									return(1);
								}
							}
							else {
								CompilerErrorOrWarning(170);
								return(1);
							}

						}
						else {

							// controlla che le dimensioni corrispondano
							for (m=1;m<nn0->dimensions;m++)
								if (nn0->dimensions_list[m] !=
									nn1->dimensions_list[m]) {
									CompilerErrorOrWarning(170);
									return(1);
								}

						}
					}
				}

				// imposta le istruzioni
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_pointer->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_pointer->address.section=SECTION_DATA;
					psi_pointer->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_pointer->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_pointer->address.section=SECTION_STACK;
					psi_pointer->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_pointer->psi=LOAD_INT_IN_EAX;
					psi_pointer->address.section=SECTION_TEMPDATA1;
					psi_pointer->address.address=nn0->address;
				}

				// imposta buffer
				if (buffer[k].type == NUMBER)
					buffer[i]=buffer[j];
				else if (nn1->indirection > nn0->indirection)
					buffer[i]=buffer[k];
				else
					buffer[i]=buffer[j];

				// completa le informazioni di buffer
				buffer[i].non_number.type &= 0xffff;
				buffer[i].non_number.type |= RIGHT_VALUE;
				buffer[i].non_number.address=cur_tmp_address;
				buffer[i].non_number.is_pointer=0;

				// valuta se impostare a 1 il const_flag
				if (buffer[k].type == NON_NUMBER &&
					(nn0->const_flag || nn1->const_flag) )
					buffer[i].non_number.const_flag=1;

			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// controlla il terzo operando per vedere se è compatibile
				if (nn0->indirection) {

					// controlla il terzo operando
					if (buffer[k].type!=NON_NUMBER) {
						if (buffer[k].type!=NUMBER) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else {

							// se il numero è un floating point esce direttamente
							if (buffer[k].number.type >= FLOAT) {
								CompilerErrorOrWarning(170);
								return(1);
							}

							// converte in intero il numero
							if (buffer[k].number.type==CHAR)
								m=*(char*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_CHAR)
								m=*(unsigned char*)buffer[k].number.number;
							else if (buffer[k].number.type==SHORT)
								m=*(short*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_SHORT)
								m=*(unsigned short*)buffer[k].number.number;
							else if (buffer[k].number.type==INT)
								m=*(int*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_INT)
								m=*(unsigned int*)buffer[k].number.number;

							// continua solo se è uguale a zero
							if (m) {
								CompilerErrorOrWarning(170);
								return(1);
							}

						}
					}
					else {
						nn1=&buffer[k].non_number;

						// verifica che il terzo operando sia un puntatore compatibile
						if (!nn1->dimensions && !nn1->indirection) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else {

							// controlla il tipo dei puntatori
							if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) ) {
								CompilerErrorOrWarning(170);
								return(1);
							}
							else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
								nn0->struct_type != nn1->struct_type ) {
								CompilerErrorOrWarning(170);
								return(1);
							}

							// controlla il level of indirection dei puntatori
							if (nn1->dimensions ||
								nn0->indirection != nn1->indirection) {

								// controlla i vari casi
								if (nn1->dimensions==1) {
									if (nn1->indirection + 1 != nn0->indirection) {
										CompilerErrorOrWarning(170);
										return(1);
									}
								}
								else {
									CompilerErrorOrWarning(170);
									return(1);
								}

							}
						}
					}

				}
				else {

					// controlla che il terzo operando non sia un puntatore o una struttura
					if (buffer[k].type!=NON_NUMBER) {
						if (buffer[k].type!=NUMBER) {
							CompilerErrorOrWarning(170);
							return(1);
						}
					}
					else {
						nn1=&buffer[k].non_number;
						if (nn1->dimensions ||
							nn1->indirection ||
							(nn1->type & 0xffff) == STRUCTURE_TYPE) {
							CompilerErrorOrWarning(170);
							return(1);
						}
					}

				}

				// valuta se caricare in eax il contenuto del puntatore o il puntatore stesso
				if ( buffer[k].type==NUMBER ||
					buffer[k].non_number.dimensions ||
					(buffer[k].non_number.type & 0xffff) != (nn0->type & 0xffff) ||
					( (buffer[k].non_number.type & 0xff0000) == RIGHT_VALUE &&
					!buffer[k].non_number.is_pointer ) ) {

					// carica in ecx il puntatore
					psi_pointer->psi=LOAD_INT_IN_ECX;
					psi_pointer->address.section=SECTION_TEMPDATA1;
					psi_pointer->address.address=nn0->address;
					psi_pointer++;

					// carica in eax o st0 il contenuto del puntatore ecx
					if ( (nn0->type & 0xffff) == INT ||
						nn0->indirection)
						psi_pointer->psi=LOAD_ECXPTR_INT_IN_EAX;
					else if ( (nn0->type & 0xffff) == CHAR )
						psi_pointer->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
						psi_pointer->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
					else if ( (nn0->type & 0xffff) == SHORT )
						psi_pointer->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
						psi_pointer->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
						psi_pointer->psi=LOAD_ECXPTR_INT_IN_EAX;
					else if ( (nn0->type & 0xffff) == FLOAT )
						psi_pointer->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
					else if ( (nn0->type & 0xffff) == DOUBLE )
						psi_pointer->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
						psi_pointer->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

				}
				else {

					// carica in eax il puntatore
					psi_pointer->psi=LOAD_INT_IN_EAX;
					psi_pointer->address.section=SECTION_TEMPDATA1;
					psi_pointer->address.address=nn0->address;

				}

				// imposta buffer
				if (buffer[k].type == NUMBER && !nn0->indirection) {
					if ( (nn0->type & 0xffff) < FLOAT &&
						buffer[k].number.type >= FLOAT ) {

						// imposta le istruzioni per la conversione e imposta buffer
						psi_pointer++;
						if ( (nn0->type & 0xffff) == UNSIGNED_INT )
							psi_pointer->psi=LOAD_UNSIGNED_EAX_IN_ST0;
						else
							psi_pointer->psi=LOAD_SIGNED_EAX_IN_ST0;
						buffer[i]=buffer[j];
						buffer[i].non_number.type &= 0xff0000;
						if (buffer[k].number.type == FLOAT)
							buffer[i].non_number.type |= FLOAT;
						else
							buffer[i].non_number.type |= DOUBLE;

					}
					else {
						buffer[i]=buffer[j];
						if ( (nn0->type & 0xffff) < buffer[k].number.type ) {
							buffer[i].non_number.type &= 0xff0000;
							if (buffer[k].number.type == FLOAT)
								buffer[i].non_number.type |= FLOAT;
							else
								buffer[i].non_number.type |= DOUBLE;
						}
					}
				}
				else if (nn0->indirection)
					buffer[i]=buffer[j];
				else {

					// imposta buffer e controlla se è necessario caricare eax in st0
					if ( (nn0->type & 0xffff) < FLOAT &&
						(nn1->type & 0xffff) >= FLOAT ) {

						// carica eax in st0 e imposta buffer
						psi_pointer++;
						if ( (nn0->type & 0xffff) == UNSIGNED_INT )
							psi_pointer->psi=LOAD_UNSIGNED_EAX_IN_ST0;
						else
							psi_pointer->psi=LOAD_SIGNED_EAX_IN_ST0;
						buffer[i]=buffer[k];

					}
					else if ( (nn0->type & 0xffff) > (nn1->type & 0xffff) )
						buffer[i]=buffer[j];
					else
						buffer[i]=buffer[k];

				}

				// aggiusta il tipo di buffer
				if (!nn0->indirection &&
					!nn0->dimensions) {
					nn1=&buffer[i].non_number;

					if ( (nn1->type & 0xffff) == UNSIGNED_CHAR ||
						(nn1->type & 0xffff) == UNSIGNED_SHORT) {
						nn1->type &= 0xff0000;
						nn1->type |= UNSIGNED_INT;
					}
					else if ( (nn1->type & 0xffff) == CHAR ||
						(nn1->type & 0xffff) == SHORT) {
						nn1->type &= 0xff0000;
						nn1->type |= INT;
					}

				}

				// completa le informazioni di buffer
				buffer[i].non_number.type &= 0xffff;
				buffer[i].non_number.type |= RIGHT_VALUE;
				buffer[i].non_number.address=cur_tmp_address;

				// valuta se impostare is_pointer
				if ( buffer[k].type==NUMBER ||
					buffer[k].non_number.dimensions ||
					(buffer[k].non_number.type & 0xffff) != (nn0->type & 0xffff) ||
					( (buffer[k].non_number.type & 0xff0000) == RIGHT_VALUE &&
					!buffer[k].non_number.is_pointer ) )
					buffer[i].non_number.is_pointer=0;
				else
					buffer[i].non_number.is_pointer=1;

				// valuta se impostare a 1 il const_flag
				if (buffer[k].type == NON_NUMBER) {
					nn1=&buffer[k].non_number;
					if ( nn0->const_flag || nn1->const_flag )
						buffer[i].non_number.const_flag=1;
				}

			}
			// carica in eax o st0 il valore
			else {

				// controlla il terzo operando per vedere se è compatibile
				if (nn0->indirection) {

					// controlla il terzo operando
					if (buffer[k].type!=NON_NUMBER) {
						if (buffer[k].type!=NUMBER) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else {

							// se il numero è un floating point esce direttamente
							if (buffer[k].number.type >= FLOAT) {
								CompilerErrorOrWarning(170);
								return(1);
							}

							// converte in intero il numero
							if (buffer[k].number.type==CHAR)
								m=*(char*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_CHAR)
								m=*(unsigned char*)buffer[k].number.number;
							else if (buffer[k].number.type==SHORT)
								m=*(short*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_SHORT)
								m=*(unsigned short*)buffer[k].number.number;
							else if (buffer[k].number.type==INT)
								m=*(int*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_INT)
								m=*(unsigned int*)buffer[k].number.number;

							// continua solo se è uguale a zero
							if (m) {
								CompilerErrorOrWarning(170);
								return(1);
							}

						}
					}
					else {
						nn1=&buffer[k].non_number;

						// verifica che il terzo operando sia un puntatore compatibile
						if (!nn1->dimensions && !nn1->indirection) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else {

							// controlla il tipo dei puntatori
							if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) ) {
								CompilerErrorOrWarning(170);
								return(1);
							}
							else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
								nn0->struct_type != nn1->struct_type ) {
								CompilerErrorOrWarning(170);
								return(1);
							}

							// controlla il level of indirection dei puntatori
							if (nn1->dimensions ||
								nn0->indirection != nn1->indirection) {

								// controlla i vari casi
								if (nn1->dimensions==1) {
									if (nn1->indirection + 1 != nn0->indirection) {
										CompilerErrorOrWarning(170);
										return(1);
									}
								}
								else {
									CompilerErrorOrWarning(170);
									return(1);
								}

							}
						}
					}

				}
				else {

					// controlla che il terzo operando non sia un puntatore
					if (buffer[k].type!=NON_NUMBER) {
						if (buffer[k].type!=NUMBER) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
							CompilerErrorOrWarning(170);
							return(1);
						}
					}
					else {
						nn1=&buffer[k].non_number;
						if (nn1->dimensions ||
							nn1->indirection) {
							CompilerErrorOrWarning(170);
							return(1);
						}
						else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
							if ( (nn1->type & 0xffff) != STRUCTURE_TYPE ||
								nn0->struct_type != nn1->struct_type ) {
								CompilerErrorOrWarning(170);
								return(1);
							}
						}
					}

				}

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_pointer->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_pointer->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_pointer->address.section=SECTION_TEMPDATA1;
				psi_pointer->address.address=nn0->address;

				// carica in eax o st0 il valore o il puntatore
				if ( (nn0->type & 0xff0000) == RIGHT_VALUE ||
					buffer[k].type==NUMBER ||
					( (nn0->type & 0xffff) == STRUCTURE_TYPE && !nn0->indirection ) ||
					buffer[k].non_number.dimensions ||
					(buffer[k].non_number.type & 0xffff) != (nn0->type & 0xffff) ||
					( (buffer[k].non_number.type & 0xff0000) == RIGHT_VALUE &&
					!buffer[k].non_number.is_pointer ) ) {

					// carica il valore in eax o st0
					if ( (nn0->type & 0xffff) == INT ||
						nn0->indirection) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_pointer->psi=LOAD_INT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
						else
							psi_pointer->psi=LOAD_SIGNED_CHAR_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
						else
							psi_pointer->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
						else
							psi_pointer->psi=LOAD_SIGNED_SHORT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
						else
							psi_pointer->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_pointer->psi=LOAD_INT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == FLOAT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_FLOAT_IN_ST0;
						else
							psi_pointer->psi=LOAD_FLOAT_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == DOUBLE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_pointer->psi=LOAD_DOUBLE_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_pointer->psi=LOAD_DOUBLE_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
							psi_pointer->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
						else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_pointer->psi=LOAD_STACK_INT_POINTER_IN_EAX;
						else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
							psi_pointer->psi=LOAD_INT_IN_EAX;
					}

				}
				else {

					// carica il puntatore
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
						psi_pointer->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_pointer->psi=LOAD_STACK_INT_POINTER_IN_EAX;

				}

				// imposta buffer
				if (buffer[k].type == NUMBER && !nn0->indirection) {
					if ( (nn0->type & 0xffff) < FLOAT &&
						buffer[k].number.type >= FLOAT ) {

						// imposta le istruzioni per la conversione e imposta buffer
						psi_pointer++;
						if ( (nn0->type & 0xffff) == UNSIGNED_INT )
							psi_pointer->psi=LOAD_UNSIGNED_EAX_IN_ST0;
						else
							psi_pointer->psi=LOAD_SIGNED_EAX_IN_ST0;
						buffer[i]=buffer[j];
						buffer[i].non_number.type &= 0xff0000;
						if (buffer[k].number.type == FLOAT)
							buffer[i].non_number.type |= FLOAT;
						else
							buffer[i].non_number.type |= DOUBLE;

					}
					else {
						buffer[i]=buffer[j];
						if ( (nn0->type & 0xffff) < buffer[k].number.type ) {
							buffer[i].non_number.type &= 0xff0000;
							if (buffer[k].number.type == FLOAT)
								buffer[i].non_number.type |= FLOAT;
							else
								buffer[i].non_number.type |= DOUBLE;
						}
					}
				}
				else if (nn0->indirection)
					buffer[i]=buffer[j];
				else {

					// imposta buffer e controlla se è necessario caricare eax in st0
					if ( (nn0->type & 0xffff) < FLOAT &&
						(nn1->type & 0xffff) >= FLOAT ) {

						// carica eax in st0 e imposta buffer
						psi_pointer++;
						if ( (nn0->type & 0xffff) == UNSIGNED_INT )
							psi_pointer->psi=LOAD_UNSIGNED_EAX_IN_ST0;
						else
							psi_pointer->psi=LOAD_SIGNED_EAX_IN_ST0;
						buffer[i]=buffer[k];

					}
					else if ( (nn0->type & 0xffff) > (nn1->type & 0xffff) )
						buffer[i]=buffer[j];
					else
						buffer[i]=buffer[k];

				}

				// aggiusta il tipo di buffer
				if (!nn0->indirection &&
					!nn0->dimensions) {
					nn1=&buffer[i].non_number;

					if ( (nn1->type & 0xffff) == UNSIGNED_CHAR ||
						(nn1->type & 0xffff) == UNSIGNED_SHORT) {
						nn1->type &= 0xff0000;
						nn1->type |= UNSIGNED_INT;
					}
					else if ( (nn1->type & 0xffff) == CHAR ||
						(nn1->type & 0xffff) == SHORT) {
						nn1->type &= 0xff0000;
						nn1->type |= INT;
					}

				}

				// completa le informazioni di buffer
				buffer[i].non_number.type &= 0xffff;
				buffer[i].non_number.type |= RIGHT_VALUE;
				buffer[i].non_number.address=cur_tmp_address;

				// valuta se impostare is_pointer
				if ( (nn0->type & 0xff0000) == RIGHT_VALUE ||
					buffer[k].type==NUMBER ||
					( (nn0->type & 0xffff) == STRUCTURE_TYPE && !nn0->indirection ) ||
					buffer[k].non_number.dimensions ||
					(buffer[k].non_number.type & 0xffff) != (nn0->type & 0xffff) ||
					( (buffer[k].non_number.type & 0xff0000) == RIGHT_VALUE &&
					!buffer[k].non_number.is_pointer ) )
					buffer[i].non_number.is_pointer=0;
				else
					buffer[i].non_number.is_pointer=1;

				// valuta se impostare a 1 il const_flag
				if (buffer[k].type == NON_NUMBER) {
					nn1=&buffer[k].non_number;
					if ( nn0->const_flag || nn1->const_flag )
						buffer[i].non_number.const_flag=1;
				}

			}
			psi_pointer++;

		}
		// carica in eax o st0 un numero
		else if (buffer[j].type==NUMBER) {

			// controlla il terzo operando
			control_if_zero=0;
			if (buffer[k].type!=NON_NUMBER) {
				if (buffer[k].type!=NUMBER) {
					CompilerErrorOrWarning(170);
					return(1);
				}
				else {
					if (buffer[j].number.type >= buffer[k].number.type)
						m=buffer[j].number.type;
					else
						m=buffer[k].number.type;

					// imposta buffer
					buffer[i].type=NON_NUMBER;
					nn0=&buffer[i].non_number;
					nn0->type=RIGHT_VALUE; // VPCICE PATCH // // 20MAG2004 //
					nn0->address=cur_tmp_address; // VPCICE PATCH // // 20MAG2004 //
					nn0->indirection=0;
					nn0->dimensions=0;
					nn0->dimensions_list=NULL;
					nn0->const_flag=0;
					nn0->struct_type=NULL;
					nn0->is_pointer=0;
					nn0->array_stptr=0;
				}
			}
			else {

				// imposta nn1
				nn1=&buffer[k].non_number;

				// controlla il terzo operando
				if (!nn1->indirection &&
					!nn1->dimensions) {

					// controlla che non sia una struttura e stabilisce il tipo
					if ( (nn1->type & 0xffff) == STRUCTURE_TYPE ) {
						CompilerErrorOrWarning(170);
						return(1);
					}
					else if ( (nn1->type & 0xffff) >= buffer[k].number.type )
						m=(nn1->type & 0xffff);
					else
						m=buffer[j].number.type;
					buffer[i]=buffer[k];

				}
				else {
					if (buffer[j].number.type >= FLOAT) {
						CompilerErrorOrWarning(170);
						return(1);
					}
					else {
						m=UNSIGNED_INT;
						buffer[i]=buffer[k];
						control_if_zero=1;
					}
				}

			}

			// converte il numero
			if ( m == LONG_DOUBLE )
				m=DOUBLE;
			else if ( m == UNSIGNED_CHAR ||
				m == UNSIGNED_SHORT)
				m=UNSIGNED_INT;
			else if ( m == CHAR ||
				m == SHORT)
				m=INT;
			if (!buffer[i].non_number.dimensions &&
				!buffer[i].non_number.indirection) {
				buffer[i].non_number.type&=0xff0000;
				buffer[i].non_number.type|=m;
			}
			CastTypeOfConstantNumber(&buffer[j].number,&number,m);

			// carica il numero in eax
			if (m==DOUBLE) {
				*(double*)strings_ptr=*(double*)number.number;
				psi_pointer->psi=LOAD_DOUBLE_IN_ST0;
				psi_pointer->address.address=(int)strings_ptr;
				strings_ptr+=8;
				psi_pointer->address.section=SECTION_STRINGS;
			}
			else if (m==FLOAT) {
				*(float*)strings_ptr=*(float*)number.number;
				psi_pointer->psi=LOAD_FLOAT_IN_ST0;
				psi_pointer->address.address=(int)strings_ptr;
				strings_ptr+=4;
				psi_pointer->address.section=SECTION_STRINGS;
			}
			else {
				if (control_if_zero && *(int*)number.number) {
					CompilerErrorOrWarning(170);
					return(1);
				}
				psi_pointer->psi=LOAD_CONSTANT_IN_EAX;
				psi_pointer->constant=*(int*)number.number;
			}
			psi_pointer++;

		}
		else {
			CompilerErrorOrWarning(170);
			return(1);
		}

		// imposta l'indice del terzo argomento
		j=op_ptr[op_id].third_tok;

		// carica in eax o st0 il terzo argomento
		if (buffer[j].type==NON_NUMBER) {

			// imposta nn0
			nn0=&buffer[j].non_number;

			// carica in eax l'indirizzo della matrice
			if (nn0->dimensions) {

				// imposta le istruzioni
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}

			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// carica il puntatore all'oggetto oppure l'oggetto stesso
				if ( buffer[i].non_number.is_pointer ) {

					// carica in eax il puntatore
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;

				}
				else {

					// carica in ecx il puntatore
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;

					// carica in eax o st0 il contenuto del puntatore ecx
					if ( (nn0->type & 0xffff) == INT ||
						nn0->indirection)
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					else if ( (nn0->type & 0xffff) == CHAR )
						psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
						psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
					else if ( (nn0->type & 0xffff) == SHORT )
						psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
						psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					else if ( (nn0->type & 0xffff) == FLOAT )
						psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
					else if ( (nn0->type & 0xffff) == DOUBLE )
						psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
						psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

					// controlla se bisogna caricare eax in st0
					if ( (nn0->type & 0xffff) < FLOAT && 
						!nn0->indirection &&
						!nn0->dimensions &&
						(buffer[i].non_number.type & 0xffff) >= FLOAT ) {

						// carica eax in st0 e imposta buffer
						psi_ptr++;
						if ( (nn0->type & 0xffff) == UNSIGNED_INT )
							psi_ptr->psi=LOAD_UNSIGNED_EAX_IN_ST0;
						else
							psi_ptr->psi=LOAD_SIGNED_EAX_IN_ST0;

					}

				}

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica il puntatore all'oggetto oppure l'oggetto stesso
				if ( buffer[i].non_number.is_pointer ) {

					// carica il puntatore
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
						psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;

				}
				else {

					// carica in eax o st0 il valore
					if ( (nn0->type & 0xffff) == INT ||
						nn0->indirection) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
						else
							psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
						else
							psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
						else
							psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
						else
							psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == FLOAT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
						else
							psi_ptr->psi=LOAD_FLOAT_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == DOUBLE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
							psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
						else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
						else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
							psi_ptr->psi=LOAD_INT_IN_EAX;
					}

					// controlla se bisogna caricare eax in st0
					if ( (nn0->type & 0xffff) < FLOAT && 
						!nn0->indirection &&
						!nn0->dimensions &&
						(buffer[i].non_number.type & 0xffff) >= FLOAT ) {

						// carica eax in st0 e imposta buffer
						psi_ptr++;
						if ( (nn0->type & 0xffff) == UNSIGNED_INT )
							psi_ptr->psi=LOAD_UNSIGNED_EAX_IN_ST0;
						else
							psi_ptr->psi=LOAD_SIGNED_EAX_IN_ST0;

					}

				}

			}
			psi_ptr++;

		}
		// carica in eax o st0 un numero
		else if (buffer[j].type==NUMBER) {

			// converte il numero
			if (!buffer[i].non_number.dimensions &&
				!buffer[i].non_number.indirection)
				m=buffer[i].non_number.type & 0xffff;
			else
				m=UNSIGNED_INT;
			CastTypeOfConstantNumber(&buffer[j].number,&number,m);

			// carica il numero in eax
			if (m==DOUBLE) {
				*(double*)strings_ptr=*(double*)number.number;
				psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=8;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else if (m==FLOAT) {
				*(float*)strings_ptr=*(float*)number.number;
				psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=4;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else {
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=*(int*)number.number;
			}
			psi_ptr++;

		}
		else {
			CompilerErrorOrWarning(170);
			return(1);
		}

		// assembla l'istruzione per il primo salto
		psi_pointer->psi=JUMP;
		psi_pointer->address.section=SECTION_PSI;
		psi_pointer->address.address=(int)psi_ptr;

		// memorizza il risultato
		nn0=&buffer[i].non_number;
		if (nn0->is_pointer ||
			nn0->dimensions ||
			nn0->indirection ||
			(nn0->type & 0xffff) == STRUCTURE_TYPE ||
			(nn0->type & 0xffff) < FLOAT) {
			psi_ptr->psi=STORE_EAX_IN_INT;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=cur_tmp_address;
			cur_tmp_address+=4;
		}
		else if ( (nn0->type & 0xffff) == FLOAT ) {
			psi_ptr->psi=STORE_ST0_IN_FLOAT;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=cur_tmp_address;
			cur_tmp_address+=4;
		}
		else {
			psi_ptr->psi=STORE_ST0_IN_DOUBLE;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=cur_tmp_address;
			cur_tmp_address+=8;
		}
		psi_ptr++;

	}
	// a*b / a/b / a%b / a+b / a-b / a<<b / a>>b / a<b / a>b / a<=b / a>=b / a==b / a!=b
	// a&b / a^b / a|b / a&&b / a||b / a=b / a*=b / a/=b / a%=b / a+=b / a-=b / a<<=b
	// a>>=b / a&=b / a|=b / a^=b
	else {
		i=op_ptr[op_id].tok_id;
		j=op_ptr[op_id].left_tok;
		k=op_ptr[op_id].right_tok;

		// imposta operation
		operation=op_ptr[op_id].id;

		// nel caso di una somma ordina i due operandi
		if (operation == ADDITION) {
			if (buffer[k].type == NON_NUMBER &&
				(buffer[k].non_number.indirection ||
				buffer[k].non_number.dimensions)) {
				// inverte i due operandi per semplificare il lavoro più tardi
				l=j;
				j=k;
				k=l;
			}
		}

		// nel caso di un and logico o or logico ordina i due operandi
		if (operation==LOGICAL_AND || operation==LOGICAL_OR) {

			// ottiene il tipo dei due operandi per valutare se invertirli
			if (buffer[j].type == NUMBER)
				l=buffer[j].number.type;
			else if (buffer[j].type == NON_NUMBER) {
				if (buffer[j].non_number.indirection ||
					buffer[j].non_number.dimensions)
					l=INT;
				else
					l=buffer[j].non_number.type & 0xffff;
			}
			if (buffer[k].type == NUMBER)
				m=buffer[k].number.type;
			else if (buffer[k].type == NON_NUMBER) {
				if (buffer[k].non_number.indirection ||
					buffer[k].non_number.dimensions)
					m=INT;
				else
					m=buffer[k].non_number.type & 0xffff;
			}

			// controlla se è necessaria l'inversione
			if (l<=UNSIGNED_INT && m>=FLOAT) {
				// inverte i due operandi per semplificare il lavoro più tardi
				l=j;
				j=k;
				k=l;
			}
		}

		// nel caso di una assegnazione composta controlla che a sinistra ci sia un l-value
		if ( (operation & 0xff00) == 0x0d00 ) {
			if (buffer[j].type!=NON_NUMBER) {
				if (operation == ASSIGNMENT)
					CompilerErrorOrWarning(161);
				else
					CompilerErrorOrWarning(149);
				return(1);
			}
			nn0=&buffer[j].non_number;
			if ( nn0->dimensions ||
				( (nn0->type & 0xffff) == STRUCTURE_TYPE && !nn0->indirection ) ||
				( (nn0->type & 0xff0000) == RIGHT_VALUE && !nn0->is_pointer) ) {

				// controlla prima se siamo nel caso (struttura=struttura)
				if (operation!=ASSIGNMENT ||
					nn0->dimensions ||
					nn0->indirection ||
					(nn0->type & 0xffff) != STRUCTURE_TYPE) {
					if (operation == ASSIGNMENT)
						CompilerErrorOrWarning(161);
					else
						CompilerErrorOrWarning(149);
					return(1);
				}

			}
			if (nn0->const_flag) {
				if (operation == ASSIGNMENT)
					CompilerErrorOrWarning(162);
				else
					CompilerErrorOrWarning(150);
				return(1);
			}
		}

		// nel caso di una assegnazione semplice inverte temporaneamente i due operandi
		if (operation==ASSIGNMENT) {
			l=j;
			j=k;
			k=l;
		}

		// nel caso di una operazione commutativa e di una costante come primo operando inverte
		// i due operandi (per semplificare l'operazione finale di semplificazione delle istruzioni)
		if ( operation == ADDITION || operation == MULTIPLICATION ) {
			if ( buffer[j].type==NUMBER &&
				buffer[k].type==NON_NUMBER &&
				!buffer[k].non_number.dimensions &&
				!buffer[k].non_number.indirection ) {
				l=j;
				j=k;
				k=l;
			}
		}

		// controlla se impostare do_not_use_st0
		do_not_use_st0=0;
		if ( operation == ASSIGNMENT ) {

			// effettua un controllo sul tipo dei due operandi ( buffer[j] = l-value o number )
			if (buffer[j].type == NUMBER)
				l=-2;
			else if (buffer[j].type == NON_NUMBER) {
				if ( (buffer[j].non_number.type & 0xff0000) == RIGHT_VALUE &&
					!buffer[j].non_number.is_pointer )
					l=-1;
				else if (buffer[j].non_number.indirection ||
					buffer[j].non_number.dimensions)
					l=-1;
				else if ( (buffer[j].non_number.type & 0xffff) != STRUCTURE_TYPE )
					l=buffer[j].non_number.type & 0xffff;
				else
					l=-1;
			}
			if (buffer[k].type == NUMBER)
				m=buffer[k].number.type;
			else if (buffer[k].type == NON_NUMBER) {
				if (buffer[k].non_number.indirection ||
					buffer[k].non_number.dimensions)
					m=-1;
				else if ( (buffer[k].non_number.type & 0xffff) != STRUCTURE_TYPE )
					m=buffer[k].non_number.type & 0xffff;
				else
					m=-1;
			}

			// controlla di non avere a che fare con puntatori, array o matrici
			if ( l != -1 && m != -1 ) {

				// valuta se impostare do_not_use_st0
				if ( ( l == FLOAT && m == FLOAT ) ||
					( l > FLOAT && m > FLOAT ) ||
					( l == -2 && m >= FLOAT ) )
					do_not_use_st0=1;

			}

		}

		// resetta with_pointers (serve solo per le addizioni e per le sottrazioni)
		with_pointers=0;

		// resetta eax_in_st0 (serve solo per le assegnazioni composte)
		eax_in_st0=0;

		// resetta struct_assignment (impostato a 1 nel caso di struttura=struttura)
		struct_assignment=0;

		// resetta objsub (impostato a 1 nel caso di una operazione del tipo pointer-pointer)
		objsub=0;

		// resetta special_char_assignment (utilizzato per le inizializzazioni di stringhe)
		special_char_assignment=0;

		// carica in eax o st0 il primo operando
		if (buffer[j].type==NON_NUMBER) {

			// imposta nn0 e controlla che non si tratti di un puntatore o di una struttura
			nn0=&buffer[j].non_number;
			if (nn0->dimensions || nn0->indirection) {
				if (operation == ADDITION ||
					operation == SUBTRACTION ||
					operation == ADDITION_ASSIGNMENT ||
					operation == SUBTRACTION_ASSIGNMENT) {

					// stabilisce la dimensione dell'oggetto puntato
					if (nn0->dimensions) {
						if (nn0->indirection)
							with_pointers=4;
						else if ( (nn0->type & 0xff00) == CHAR )
							with_pointers=1;
						else if ( (nn0->type & 0xff00) == SHORT )
							with_pointers=2;
						else if ( (nn0->type & 0xff00) == INT )
							with_pointers=4;
						else if ( (nn0->type & 0xff00) == FLOAT )
							with_pointers=4;
						else if ( (nn0->type & 0xff00) == DOUBLE )
							with_pointers=8;
						else
							with_pointers=nn0->struct_type->address;

						for (l=1;l<nn0->dimensions;l++)
							with_pointers*=nn0->dimensions_list[l];
					}
					else {
						if (nn0->indirection==1) {
							if ( (nn0->type & 0xffff) == VOID_TYPE ) {
								if ( (operation & 0xff00) == 0x0d00 )
									CompilerErrorOrWarning(155);
								else
									CompilerErrorOrWarning(120);
								return(1);
							}
							else if ( (nn0->type & 0xff00) == CHAR )
								with_pointers=1;
							else if ( (nn0->type & 0xff00) == SHORT )
								with_pointers=2;
							else if ( (nn0->type & 0xff00) == INT )
								with_pointers=4;
							else if ( (nn0->type & 0xff00) == FLOAT )
								with_pointers=4;
							else if ( (nn0->type & 0xff00) == DOUBLE )
								with_pointers=8;
							else
								with_pointers=nn0->struct_type->address;
						}
						else
							with_pointers=4;
					}

				}
				else if ( (operation & 0xff00) == 0x0500 ||
					(operation & 0xff00) == 0x0600) {
					if (buffer[k].type != NON_NUMBER) {
						if (buffer[k].type != NUMBER) {
							CompilerErrorOrWarning(146);
							return(1);
						}
						else {
							if (buffer[k].number.type==CHAR)
								l=*(char*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_CHAR)
								l=*(unsigned char*)buffer[k].number.number;
							else if (buffer[k].number.type==SHORT)
								l=*(short*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_SHORT)
								l=*(unsigned short*)buffer[k].number.number;
							else if (buffer[k].number.type==INT)
								l=*(int*)buffer[k].number.number;
							else if (buffer[k].number.type==UNSIGNED_INT)
								l=*(unsigned int*)buffer[k].number.number;
							else {
								CompilerErrorOrWarning(146);
								return(1);
							}
							if (l) {
								CompilerErrorOrWarning(146);
								return(1);
							}
						}
					}
					else if (!buffer[k].non_number.indirection &&
						!buffer[k].non_number.dimensions) {
						CompilerErrorOrWarning(146);
						return(1);
					}
				}
				else if (operation == ASSIGNMENT) {

					// controlla se è una special_char_assignment
					nn1=&buffer[k].non_number;
					if (allow_special_char_assignment) {
						if ( (nn0->type & 0xffff) == CHAR &&
							(nn1->type & 0xff00) == CHAR &&
							(nn0->type & 0xff0000) == LEFT_VALUE_0 &&
							(nn1->type & 0xff0000) == RIGHT_VALUE &&
							(nn0->address & 0x40000000) &&
							nn0->dimensions == 1 &&
							!nn0->indirection &&
							!nn1->dimensions &&
							!nn1->indirection &&
							nn1->is_pointer )
							special_char_assignment=allow_special_char_assignment;
					}

					// continua solo se non è una special_char_assignment
					if (!special_char_assignment) {

						// confronta i tipi dei due puntatori; devono essere compatibili
						nn1=&buffer[k].non_number;
						if (!nn1->indirection) {
							CompilerErrorOrWarning(163);
							return(1);
						}
						if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) ) {
							if ( (nn1->type & 0xffff) != VOID_TYPE ) {
								CompilerErrorOrWarning(163);
								return(1);
							}
						}
						else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
							nn0->struct_type != nn1->struct_type ) {
							CompilerErrorOrWarning(163);
							return(1);
						}
						if (nn0->indirection != nn1->indirection) {
							if (nn0->dimensions==1) {
								if (nn0->indirection + 1 != nn1->indirection) {
									CompilerErrorOrWarning(163);
									return(1);
								}
							}
							else {
								CompilerErrorOrWarning(163);
								return(1);
							}
						}
						else if (nn0->dimensions) {
							CompilerErrorOrWarning(163);
							return(1);
						}

					}

				}
				else {
					if (operation==MULTIPLICATION)
						CompilerErrorOrWarning(110);
					else if (operation==DIVISION)
						CompilerErrorOrWarning(113);
					else if (operation==REMAINDER)
						CompilerErrorOrWarning(116);
					else if (operation==LEFT_SHIFT)
						CompilerErrorOrWarning(130);
					else if (operation==RIGHT_SHIFT)
						CompilerErrorOrWarning(131);
					else if (operation==BITWISE_AND ||
						operation==BITWISE_EXCLUSIVE_OR ||
						operation==BITWISE_OR)
						CompilerErrorOrWarning(140);
					else if ( (operation & 0xff00) == 0x0d00 )
						CompilerErrorOrWarning(151);

					if ( operation != LOGICAL_AND &&
						operation != LOGICAL_OR )
						return(1);

				}
			}
			else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE) {
				if (operation==MULTIPLICATION)
					CompilerErrorOrWarning(112);
				else if (operation==DIVISION)
					CompilerErrorOrWarning(115);
				else if (operation==REMAINDER)
					CompilerErrorOrWarning(118);
				else if (operation==ADDITION)
					CompilerErrorOrWarning(121);
				else if (operation==SUBTRACTION)
					CompilerErrorOrWarning(122);
				else if (operation==LEFT_SHIFT)
					CompilerErrorOrWarning(132);
				else if (operation==RIGHT_SHIFT)
					CompilerErrorOrWarning(133);
				else if ( (operation & 0xff00) == 0x0500 )
					CompilerErrorOrWarning(147);
				else if ( (operation & 0xff00) == 0x0600 )
					CompilerErrorOrWarning(148);
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR)
					CompilerErrorOrWarning(141);
				else if (operation==LOGICAL_AND ||
					operation==LOGICAL_OR)
					CompilerErrorOrWarning(144);
				else if ( operation == ASSIGNMENT ) {
					nn1=&buffer[k].non_number;
					if (nn1->dimensions ||
						nn1->indirection ||
						(nn1->type & 0xffff) != STRUCTURE_TYPE)
						CompilerErrorOrWarning(165);
					else if (nn0->struct_type != nn1->struct_type)
						CompilerErrorOrWarning(165);
					else
						struct_assignment=1;
				}
				else if ( (operation & 0xff00) == 0x0d00 )
					CompilerErrorOrWarning(152);

				if (!struct_assignment)
					return(1);
			}
			else if ( (operation==REMAINDER ||
				operation==LEFT_SHIFT ||
				operation==RIGHT_SHIFT ||
				operation==BITWISE_AND ||
				operation==BITWISE_EXCLUSIVE_OR ||
				operation==BITWISE_OR ||
				operation==MODULUS_ASSIGNMENT ||
				operation==LEFT_SHIFT_ASSIGNMENT ||
				operation==RIGHT_SHIFT_ASSIGNMENT ||
				operation==BITWISE_AND_ASSIGNMENT ||
				operation==BITWISE_INCLUSIVE_OR_ASSIGNMENT ||
				operation==BITWISE_EXCLUSIVE_OR_ASSIGNMENT) &&
				(nn0->type & 0xffff) >= FLOAT) {
				if (operation==REMAINDER)
					CompilerErrorOrWarning(119);
				else if (operation==LEFT_SHIFT)
					CompilerErrorOrWarning(134);
				else if (operation==RIGHT_SHIFT)
					CompilerErrorOrWarning(135);
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR)
					CompilerErrorOrWarning(142);
				else if ( (operation & 0xff00) == 0x0d00 )
					CompilerErrorOrWarning(154);
				return(1);
			}

			// carica in eax l'indirizzo della struttura
			if (struct_assignment) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}
			}
			// carica in eax l'indirizzo della matrice
			else if (nn0->dimensions) {
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}
			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// carica in eax o st0 il contenuto del puntatore ecx
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection)
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( do_not_use_st0 )
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( do_not_use_st0 ) {
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
						psi_ptr++;
						psi_ptr->psi=LOAD_ECXPLUS4PTR_INT_IN_EDX;
					}
					else
						psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( do_not_use_st0 ) {
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
						psi_ptr++;
						psi_ptr->psi=LOAD_ECXPLUS4PTR_INT_IN_EDX;
					}
					else
						psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				}

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( do_not_use_st0 ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
					}
					else {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
						else
							psi_ptr->psi=LOAD_FLOAT_IN_ST0;
					}
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( do_not_use_st0 ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
							*(psi_ptr+1)=*psi_ptr;
							psi_ptr++;
							psi_ptr->address.address+=4;
							psi_ptr->psi=LOAD_STACK_INT_IN_EDX;
						}
						else {
							psi_ptr->psi=LOAD_INT_IN_EAX;
							*(psi_ptr+1)=*psi_ptr;
							psi_ptr++;
							psi_ptr->address.address+=4;
							psi_ptr->psi=LOAD_INT_IN_EDX;
						}
					}
					else {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					}
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( do_not_use_st0 ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
							*(psi_ptr+1)=*psi_ptr;
							psi_ptr++;
							psi_ptr->address.address+=4;
							psi_ptr->psi=LOAD_STACK_INT_IN_EDX;
						}
						else {
							psi_ptr->psi=LOAD_INT_IN_EAX;
							*(psi_ptr+1)=*psi_ptr;
							psi_ptr++;
							psi_ptr->address.address+=4;
							psi_ptr->psi=LOAD_INT_IN_EDX;
						}
					}
					else {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					}
				}

			}
			psi_ptr++;

			// controlla se bisogna caricare eax in st0
			if ( !nn0->dimensions && !nn0->indirection &&
				(nn0->type & 0xffff) < FLOAT ) {
				if (buffer[k].type == NUMBER)
					l=buffer[k].number.type;
				else if (buffer[k].type == NON_NUMBER) {
					if (buffer[k].non_number.indirection ||
						buffer[k].non_number.dimensions)
						l=UNSIGNED_INT;
					else
						l=buffer[k].non_number.type & 0xffff;
				}
				else {
					if (operation==MULTIPLICATION)
						CompilerErrorOrWarning(111);
					else if (operation==DIVISION)
						CompilerErrorOrWarning(114);
					else if (operation==REMAINDER)
						CompilerErrorOrWarning(117);
					else if (operation==ADDITION)
						CompilerErrorOrWarning(123);
					else if (operation==SUBTRACTION)
						CompilerErrorOrWarning(124);
					else if (operation==LEFT_SHIFT)
						CompilerErrorOrWarning(136);
					else if (operation==RIGHT_SHIFT)
						CompilerErrorOrWarning(137);
					else if ( (operation & 0xff00) == 0x0500 )
						CompilerErrorOrWarning(138);
					else if ( (operation & 0xff00) == 0x0600 )
						CompilerErrorOrWarning(139);
					else if (operation==BITWISE_AND ||
						operation==BITWISE_EXCLUSIVE_OR ||
						operation==BITWISE_OR)
						CompilerErrorOrWarning(143);
					else if (operation==LOGICAL_AND ||
						operation==LOGICAL_OR)
						CompilerErrorOrWarning(145);
					else if (operation == ASSIGNMENT)
						CompilerErrorOrWarning(164);
					else if ( (operation & 0xff00) == 0x0d00 )
						CompilerErrorOrWarning(153);
					return(1);
				}
				if (l>=FLOAT) {
					if ( (nn0->type & 0xffff) == UNSIGNED_INT )
						psi_ptr->psi=LOAD_UNSIGNED_EAX_IN_ST0;
					else
						psi_ptr->psi=LOAD_SIGNED_EAX_IN_ST0;
					psi_ptr++;
					eax_in_st0=1;
				}
			}

		}
		else if (buffer[j].type==NUMBER) {

			// ottiene il tipo del secondo operando
			control_if_zero=0;
			if (buffer[k].type == NUMBER)
				l=buffer[k].number.type;
			else if (buffer[k].type == NON_NUMBER) {
				if (buffer[k].non_number.dimensions ||
					buffer[k].non_number.indirection) {
					if (operation == ASSIGNMENT)
						control_if_zero=1;
					l=INT;
				}
				else
					l=buffer[k].non_number.type & 0xffff;
			}
			else {
				if (operation==MULTIPLICATION)
					CompilerErrorOrWarning(111);
				else if (operation==DIVISION)
					CompilerErrorOrWarning(114);
				else if (operation==REMAINDER)
					CompilerErrorOrWarning(117);
				else if (operation==ADDITION)
					CompilerErrorOrWarning(123);
				else if (operation==SUBTRACTION)
					CompilerErrorOrWarning(124);
				else if (operation==LEFT_SHIFT)
					CompilerErrorOrWarning(136);
				else if (operation==RIGHT_SHIFT)
					CompilerErrorOrWarning(137);
				else if ( (operation & 0xff00) == 0x0500 )
					CompilerErrorOrWarning(138);
				else if ( (operation & 0xff00) == 0x0600 )
					CompilerErrorOrWarning(139);
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR)
					CompilerErrorOrWarning(143);
				else if (operation==LOGICAL_AND ||
					operation==LOGICAL_OR)
					CompilerErrorOrWarning(145);
				else if (operation == ASSIGNMENT)
					CompilerErrorOrWarning(164);
				else if ( (operation & 0xff00) == 0x0d00 )
					CompilerErrorOrWarning(153);
				return(1);
			}

			// valuta se caricare il numero in eax oppure in st0
			if ( buffer[j].number.type > FLOAT || l > FLOAT )
				m=DOUBLE;
			else if ( buffer[j].number.type == FLOAT || l == FLOAT )
				m=FLOAT;
			else if (buffer[j].number.type == UNSIGNED_CHAR ||
				buffer[j].number.type == UNSIGNED_SHORT ||
				buffer[j].number.type == UNSIGNED_INT)
				m=UNSIGNED_INT;
			else
				m=INT;
			CastTypeOfConstantNumber(&buffer[j].number,&number,m);

			// carica il numero in eax o st0
			if (m==FLOAT || m==DOUBLE) {
				if (operation==REMAINDER) {
					CompilerErrorOrWarning(119);
					return(1);
				}
				else if (operation==LEFT_SHIFT) {
					CompilerErrorOrWarning(134);
					return(1);
				}
				else if (operation==RIGHT_SHIFT) {
					CompilerErrorOrWarning(135);
					return(1);
				}
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR) {
					CompilerErrorOrWarning(142);
					return(1);
				}
				else if (operation==MODULUS_ASSIGNMENT ||
					operation==LEFT_SHIFT_ASSIGNMENT ||
					operation==RIGHT_SHIFT_ASSIGNMENT ||
					operation==BITWISE_AND_ASSIGNMENT ||
					operation==BITWISE_INCLUSIVE_OR_ASSIGNMENT ||
					operation==BITWISE_EXCLUSIVE_OR_ASSIGNMENT) {
					CompilerErrorOrWarning(154);
					return(1);
				}
				else if (control_if_zero) {
					CompilerErrorOrWarning(160);
					return(1);
				}
				if (m==FLOAT) {
					if ( do_not_use_st0 ) {
						psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
						psi_ptr->constant=*(int*)number.number;
					}
					else {
						*(float*)strings_ptr=*(float*)number.number;
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
						psi_ptr->address.address=(int)strings_ptr;
						strings_ptr+=4;
						psi_ptr->address.section=SECTION_STRINGS;
					}
				}
				else {
					if ( do_not_use_st0 ) {
						psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
						psi_ptr->constant=*(int*)number.number;
						psi_ptr++;
						psi_ptr->psi=LOAD_CONSTANT_IN_EDX;
						psi_ptr->constant=*((int*)number.number+1);
					}
					else {
						*(double*)strings_ptr=*(double*)number.number;
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
						psi_ptr->address.address=(int)strings_ptr;
						strings_ptr+=8;
						psi_ptr->address.section=SECTION_STRINGS;
					}
				}
				psi_ptr++;

			}
			else {
				if (control_if_zero && *(int*)number.number) {
					CompilerErrorOrWarning(160);
					return(1);
				}
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=*(int*)number.number;
				psi_ptr++;
			}

		}
		else {
			if (operation==MULTIPLICATION)
				CompilerErrorOrWarning(111);
			else if (operation==DIVISION)
				CompilerErrorOrWarning(114);
			else if (operation==REMAINDER)
				CompilerErrorOrWarning(117);
			else if (operation==ADDITION)
				CompilerErrorOrWarning(123);
			else if (operation==SUBTRACTION)
				CompilerErrorOrWarning(124);
			else if (operation==LEFT_SHIFT)
				CompilerErrorOrWarning(136);
			else if (operation==RIGHT_SHIFT)
				CompilerErrorOrWarning(137);
			else if ( (operation & 0xff00) == 0x0500 )
				CompilerErrorOrWarning(138);
			else if ( (operation & 0xff00) == 0x0600 )
				CompilerErrorOrWarning(139);
			else if (operation==BITWISE_AND ||
				operation==BITWISE_EXCLUSIVE_OR ||
				operation==BITWISE_OR)
				CompilerErrorOrWarning(143);
			else if (operation==LOGICAL_AND ||
				operation==LOGICAL_OR)
				CompilerErrorOrWarning(145);
			else if (operation == ASSIGNMENT)
				CompilerErrorOrWarning(164);
			else if ( (operation & 0xff00) == 0x0d00 )
				CompilerErrorOrWarning(153);
			return(1);
		}

		// considera il secondo operando ed esegue l'operazione
		if (operation==ASSIGNMENT) {

			// confronta i tipi dei due operandi
			if (buffer[j].type == NON_NUMBER) {
				nn1=&buffer[k].non_number;
				if (nn1->indirection) {
					if (!nn0->indirection && !nn0->dimensions) {
						CompilerErrorOrWarning(163);
						return(1);
					}
					if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) ) {
						if ( (nn1->type & 0xffff) != VOID_TYPE ) {
							CompilerErrorOrWarning(163);
							return(1);
						}
					}
					else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
						nn0->struct_type != nn1->struct_type ) {
						CompilerErrorOrWarning(163);
						return(1);
					}
					if (nn0->indirection != nn1->indirection) {
						if (nn0->dimensions==1) {
							if (nn0->indirection + 1 != nn1->indirection) {
								CompilerErrorOrWarning(163);
								return(1);
							}
						}
						else {
							CompilerErrorOrWarning(163);
							return(1);
						}
					}
					else if (nn0->dimensions) {
						CompilerErrorOrWarning(163);
						return(1);
					}
				}
				else if ( (nn1->type & 0xffff) == STRUCTURE_TYPE) {
					if (!struct_assignment) {
						CompilerErrorOrWarning(165);
						return(1);
					}
				}
			}

			// ripristina l'ordine degli operandi nel caso di una assegnazione semplice
			l=j;
			j=k;
			k=l;

		}
		else if (buffer[k].type==NON_NUMBER) {

			// imposta nn1 e controlla che non si tratti di un puntatore o di una struttura
			nn1=&buffer[k].non_number;
			if (nn1->dimensions || nn1->indirection) {
				if (operation==ADDITION) {
					CompilerErrorOrWarning(125);
					return(1);
				}
				else if (operation==ADDITION_ASSIGNMENT) {
					CompilerErrorOrWarning(156);
					return(1);
				}
				else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT) {

					// verifica se anche il primo argomento è un puntatore
					if (!with_pointers) {
						if ( (operation & 0xff00) == 0x0d00 )
							CompilerErrorOrWarning(159);
						else
							CompilerErrorOrWarning(126);
						return(1);
					}

					// confronta i tipi dei due puntatori; devono essere compatibili
					if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) ) {
						if ( (operation & 0xff00) == 0x0d00 )
							CompilerErrorOrWarning(158);
						else
							CompilerErrorOrWarning(127);
						return(1);
					}
					else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
						nn0->struct_type != nn1->struct_type ) {
						if ( (operation & 0xff00) == 0x0d00 )
							CompilerErrorOrWarning(158);
						else
							CompilerErrorOrWarning(127);
						return(1);
					}
					if (nn0->indirection != nn1->indirection ||
						nn0->dimensions != nn1->dimensions) {
						if (!nn0->dimensions) {
							if (nn1->dimensions != 1 ||
								nn1->indirection + 1 != nn0->indirection) {
								if ( (operation & 0xff00) == 0x0d00 )
									CompilerErrorOrWarning(158);
								else
									CompilerErrorOrWarning(127);
								return(1);
							}
						}
						else if (nn0->dimensions==1) {
							if (nn1->dimensions ||
								nn0->indirection + 1 != nn1->indirection) {
								if ( (operation & 0xff00) == 0x0d00 )
									CompilerErrorOrWarning(158);
								else
									CompilerErrorOrWarning(127);
								return(1);
							}
						}
						else {
							if ( (operation & 0xff00) == 0x0d00 )
								CompilerErrorOrWarning(158);
							else
								CompilerErrorOrWarning(127);
							return(1);
						}
					}
					else {
						for (l=1;l<nn0->dimensions;l++)
							if (nn0->dimensions_list[l]!=nn1->dimensions_list[l]) {
								if ( (operation & 0xff00) == 0x0d00 )
									CompilerErrorOrWarning(158);
								else
									CompilerErrorOrWarning(127);
								return(1);
							}
					}

				}
				else if ( (operation & 0xff00) == 0x0500 ||
					(operation & 0xff00) == 0x0600) {
					if (buffer[j].type != NON_NUMBER) {
						if (buffer[j].type != NUMBER) {
							CompilerErrorOrWarning(146);
							return(1);
						}
						else {
							if (buffer[j].number.type==CHAR)
								l=*(char*)buffer[j].number.number;
							else if (buffer[j].number.type==UNSIGNED_CHAR)
								l=*(unsigned char*)buffer[j].number.number;
							else if (buffer[j].number.type==SHORT)
								l=*(short*)buffer[j].number.number;
							else if (buffer[j].number.type==UNSIGNED_SHORT)
								l=*(unsigned short*)buffer[j].number.number;
							else if (buffer[j].number.type==INT)
								l=*(int*)buffer[j].number.number;
							else if (buffer[j].number.type==UNSIGNED_INT)
								l=*(unsigned int*)buffer[j].number.number;
							else {
								CompilerErrorOrWarning(146);
								return(1);
							}
							if (l) {
								CompilerErrorOrWarning(146);
								return(1);
							}
						}
					}
					else if (!buffer[j].non_number.indirection &&
						!buffer[j].non_number.dimensions) {
						CompilerErrorOrWarning(146);
						return(1);
					}
				}
				else {
					if (operation==MULTIPLICATION)
						CompilerErrorOrWarning(110);
					else if (operation==DIVISION)
						CompilerErrorOrWarning(113);
					else if (operation==REMAINDER)
						CompilerErrorOrWarning(116);
					else if (operation==LEFT_SHIFT)
						CompilerErrorOrWarning(130);
					else if (operation==RIGHT_SHIFT)
						CompilerErrorOrWarning(131);
					else if (operation==BITWISE_AND ||
						operation==BITWISE_EXCLUSIVE_OR ||
						operation==BITWISE_OR)
						CompilerErrorOrWarning(140);
					else if ( (operation & 0xff00) == 0x0d00 )
						CompilerErrorOrWarning(151);

					if ( operation != LOGICAL_AND &&
						operation != LOGICAL_OR )
						return(1);

				}
			}
			else if ( (nn1->type & 0xffff) == STRUCTURE_TYPE ) {
				if (operation==MULTIPLICATION)
					CompilerErrorOrWarning(112);
				else if (operation==DIVISION)
					CompilerErrorOrWarning(115);
				else if (operation==REMAINDER)
					CompilerErrorOrWarning(118);
				else if (operation==ADDITION)
					CompilerErrorOrWarning(121);
				else if (operation==SUBTRACTION)
					CompilerErrorOrWarning(122);
				else if (operation==LEFT_SHIFT)
					CompilerErrorOrWarning(132);
				else if (operation==RIGHT_SHIFT)
					CompilerErrorOrWarning(133);
				else if ( (operation & 0xff00) == 0x0500 )
					CompilerErrorOrWarning(147);
				else if ( (operation & 0xff00) == 0x0600 )
					CompilerErrorOrWarning(148);
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR)
					CompilerErrorOrWarning(141);
				else if (operation==LOGICAL_AND ||
					operation==LOGICAL_OR)
					CompilerErrorOrWarning(144);
				else if ( (operation & 0xff00) == 0x0d00 )
					CompilerErrorOrWarning(152);
				return(1);
			}
			else if ( (operation==REMAINDER ||
				operation==LEFT_SHIFT ||
				operation==RIGHT_SHIFT ||
				operation==BITWISE_AND ||
				operation==BITWISE_EXCLUSIVE_OR ||
				operation==BITWISE_OR ||
				operation==MODULUS_ASSIGNMENT ||
				operation==LEFT_SHIFT_ASSIGNMENT ||
				operation==RIGHT_SHIFT_ASSIGNMENT ||
				operation==BITWISE_AND_ASSIGNMENT ||
				operation==BITWISE_INCLUSIVE_OR_ASSIGNMENT ||
				operation==BITWISE_EXCLUSIVE_OR_ASSIGNMENT) &&
				(nn1->type & 0xffff) >= FLOAT) {
				if (operation==REMAINDER)
					CompilerErrorOrWarning(119);
				else if (operation==LEFT_SHIFT)
					CompilerErrorOrWarning(134);
				else if (operation==RIGHT_SHIFT)
					CompilerErrorOrWarning(135);
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR)
					CompilerErrorOrWarning(142);
				else if ( (operation & 0xff00) == 0x0d00 )
					CompilerErrorOrWarning(154);
				return(1);
			}
			else if (with_pointers &&
				(nn1->type & 0xffff) >= FLOAT) {
				if (operation==ADDITION)
					CompilerErrorOrWarning(128);
				else if (operation==SUBTRACTION)
					CompilerErrorOrWarning(129);
				else if ( (operation & 0xff00) == 0x0d00 )
					CompilerErrorOrWarning(157);
				return(1);
			}

			// carica in ecx l'indirizzo della matrice
			if (nn1->dimensions) {
				if (nn1->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn1->address;
				}
				else if ( (nn1->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_ECX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn1->address;
				}
				else if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_ECX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn1->address;
				}
				else if ( (nn1->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn1->address;
				}
			}
			// il secondo operando è un RIGHT_VALUE pointer
			else if (nn1->is_pointer) {

				// carica in edx il puntatore
				psi_ptr->psi=LOAD_INT_IN_EDX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn1->address;
				psi_ptr++;

				// carica in ecx il contenuto del puntatore edx oppure esegue l'operazione
				if ( (nn1->type & 0xffff) == INT ||
					nn1->indirection)
					psi_ptr->psi=LOAD_EDXPTR_INT_IN_ECX;
				else if ( (nn1->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_EDXPTR_SIGNED_CHAR_IN_ECX;
				else if ( (nn1->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_EDXPTR_UNSIGNED_CHAR_IN_ECX;
				else if ( (nn1->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_EDXPTR_SIGNED_SHORT_IN_ECX;
				else if ( (nn1->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_EDXPTR_UNSIGNED_SHORT_IN_ECX;
				else if ( (nn1->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_EDXPTR_INT_IN_ECX;
				else if ( (nn1->type & 0xffff) == FLOAT ) {
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_DIVISION;
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_ADDITION;
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_SUBTRACTION;
					else if (operation==LESS_THAN)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_LESS_THAN;
					else if (operation==GREATER_THAN)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_GREATER_THAN;
					else if (operation==LESS_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_LESS_THAN_OR_EQUAL_TO;
					else if (operation==GREATER_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_GREATER_THAN_OR_EQUAL_TO;
					else if (operation==EQUALITY)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_INEQUALITY;
					else if (operation==LOGICAL_AND)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_LOGICAL_AND;
					else if (operation==LOGICAL_OR)
						psi_ptr->psi=ST0_EDXPTR_FLOAT_LOGICAL_OR;
				}
				else if ( (nn1->type & 0xffff) == DOUBLE ) {
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_DIVISION;
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_ADDITION;
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_SUBTRACTION;
					else if (operation==LESS_THAN)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LESS_THAN;
					else if (operation==GREATER_THAN)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_GREATER_THAN;
					else if (operation==LESS_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LESS_THAN_OR_EQUAL_TO;
					else if (operation==GREATER_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
					else if (operation==EQUALITY)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_INEQUALITY;
					else if (operation==LOGICAL_AND)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LOGICAL_AND;
					else if (operation==LOGICAL_OR)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LOGICAL_OR;
				}
				else if ( (nn1->type & 0xffff) == LONG_DOUBLE ) {
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_DIVISION;
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_ADDITION;
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_SUBTRACTION;
					else if (operation==LESS_THAN)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LESS_THAN;
					else if (operation==GREATER_THAN)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_GREATER_THAN;
					else if (operation==LESS_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LESS_THAN_OR_EQUAL_TO;
					else if (operation==GREATER_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
					else if (operation==EQUALITY)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_INEQUALITY;
					else if (operation==LOGICAL_AND)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LOGICAL_AND;
					else if (operation==LOGICAL_OR)
						psi_ptr->psi=ST0_EDXPTR_DOUBLE_LOGICAL_OR;
				}

			}
			// carica in ecx il valore oppure esegue l'operazione
			else {

				// individua l'origine del valore
				if ( (nn1->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn1->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn1->address;

				// carica in ecx il valore oppure esegue l'operazione
				if ( (nn1->type & 0xffff) == INT ||
					nn1->indirection) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
					else
						psi_ptr->psi=LOAD_INT_IN_ECX;
				}
				else if ( (nn1->type & 0xffff) == CHAR ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_ECX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_ECX;
				}
				else if ( (nn1->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_ECX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_ECX;
				}
				else if ( (nn1->type & 0xffff) == SHORT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_ECX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_ECX;
				}
				else if ( (nn1->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_ECX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_ECX;
				}
				else if ( (nn1->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_ECX;
					else
						psi_ptr->psi=LOAD_INT_IN_ECX;
				}
				else if ( (nn1->type & 0xffff) == FLOAT ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 ) {
						if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_FLOAT_MULTIPLICATION;
						else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_FLOAT_DIVISION;
						else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_FLOAT_ADDITION;
						else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_FLOAT_SUBTRACTION;
						else if (operation==LESS_THAN)
							psi_ptr->psi=ST0_STACK_FLOAT_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=ST0_STACK_FLOAT_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_STACK_FLOAT_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_STACK_FLOAT_GREATER_THAN_OR_EQUAL_TO;
						else if (operation==EQUALITY)
							psi_ptr->psi=ST0_STACK_FLOAT_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=ST0_STACK_FLOAT_INEQUALITY;
						else if (operation==LOGICAL_AND)
							psi_ptr->psi=ST0_STACK_FLOAT_LOGICAL_AND;
						else if (operation==LOGICAL_OR)
							psi_ptr->psi=ST0_STACK_FLOAT_LOGICAL_OR;
					}
					else {
						if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
							psi_ptr->psi=ST0_FLOAT_MULTIPLICATION;
						else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
							psi_ptr->psi=ST0_FLOAT_DIVISION;
						else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
							psi_ptr->psi=ST0_FLOAT_ADDITION;
						else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
							psi_ptr->psi=ST0_FLOAT_SUBTRACTION;
						else if (operation==LESS_THAN)
							psi_ptr->psi=ST0_FLOAT_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=ST0_FLOAT_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_FLOAT_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_FLOAT_GREATER_THAN_OR_EQUAL_TO;
						else if (operation==EQUALITY)
							psi_ptr->psi=ST0_FLOAT_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=ST0_FLOAT_INEQUALITY;
						else if (operation==LOGICAL_AND)
							psi_ptr->psi=ST0_FLOAT_LOGICAL_AND;
						else if (operation==LOGICAL_OR)
							psi_ptr->psi=ST0_FLOAT_LOGICAL_OR;
					}
				}
				else if ( (nn1->type & 0xffff) == DOUBLE ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 ) {
						if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_MULTIPLICATION;
						else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_DIVISION;
						else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_ADDITION;
						else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_SUBTRACTION;
						else if (operation==LESS_THAN)
							psi_ptr->psi=ST0_STACK_DOUBLE_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=ST0_STACK_DOUBLE_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_STACK_DOUBLE_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_STACK_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
						else if (operation==EQUALITY)
							psi_ptr->psi=ST0_STACK_DOUBLE_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=ST0_STACK_DOUBLE_INEQUALITY;
						else if (operation==LOGICAL_AND)
							psi_ptr->psi=ST0_STACK_DOUBLE_LOGICAL_AND;
						else if (operation==LOGICAL_OR)
							psi_ptr->psi=ST0_STACK_DOUBLE_LOGICAL_OR;
					}
					else {
						if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_MULTIPLICATION;
						else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_DIVISION;
						else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_ADDITION;
						else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_SUBTRACTION;
						else if (operation==LESS_THAN)
							psi_ptr->psi=ST0_DOUBLE_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=ST0_DOUBLE_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_DOUBLE_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
						else if (operation==EQUALITY)
							psi_ptr->psi=ST0_DOUBLE_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=ST0_DOUBLE_INEQUALITY;
						else if (operation==LOGICAL_AND)
							psi_ptr->psi=ST0_DOUBLE_LOGICAL_AND;
						else if (operation==LOGICAL_OR)
							psi_ptr->psi=ST0_DOUBLE_LOGICAL_OR;
					}
				}
				else if ( (nn1->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn1->type & 0xff0000) == LEFT_VALUE_1 ) {
						if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_MULTIPLICATION;
						else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_DIVISION;
						else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_ADDITION;
						else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
							psi_ptr->psi=ST0_STACK_DOUBLE_SUBTRACTION;
						else if (operation==LESS_THAN)
							psi_ptr->psi=ST0_STACK_DOUBLE_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=ST0_STACK_DOUBLE_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_STACK_DOUBLE_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_STACK_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
						else if (operation==EQUALITY)
							psi_ptr->psi=ST0_STACK_DOUBLE_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=ST0_STACK_DOUBLE_INEQUALITY;
						else if (operation==LOGICAL_AND)
							psi_ptr->psi=ST0_STACK_DOUBLE_LOGICAL_AND;
						else if (operation==LOGICAL_OR)
							psi_ptr->psi=ST0_STACK_DOUBLE_LOGICAL_OR;
					}
					else {
						if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_MULTIPLICATION;
						else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_DIVISION;
						else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_ADDITION;
						else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
							psi_ptr->psi=ST0_DOUBLE_SUBTRACTION;
						else if (operation==LESS_THAN)
							psi_ptr->psi=ST0_DOUBLE_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=ST0_DOUBLE_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_DOUBLE_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=ST0_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
						else if (operation==EQUALITY)
							psi_ptr->psi=ST0_DOUBLE_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=ST0_DOUBLE_INEQUALITY;
						else if (operation==LOGICAL_AND)
							psi_ptr->psi=ST0_DOUBLE_LOGICAL_AND;
						else if (operation==LOGICAL_OR)
							psi_ptr->psi=ST0_DOUBLE_LOGICAL_OR;
					}
				}

			}
			psi_ptr++;

			// controlla se l'operazione è tra un operando integrale ed uno non integrale e poi la esegue
			if ( (nn1->type & 0xffff) < FLOAT ||
				nn1->dimensions ||
				nn1->indirection) {
				if (buffer[j].type == NUMBER)
					l=buffer[j].number.type;
				else if (buffer[j].type == NON_NUMBER) {
					if (buffer[j].non_number.dimensions ||
						buffer[j].non_number.indirection)
						l=INT;
					else
						l=buffer[j].non_number.type & 0xffff;
				}
				if (l>=FLOAT &&
					operation != LOGICAL_AND &&
					operation != LOGICAL_OR) {
					if ( (nn1->type & 0xffff) == UNSIGNED_INT )
						psi_ptr->psi=LOAD_UNSIGNED_ECX_IN_ST0;
					else
						psi_ptr->psi=LOAD_SIGNED_ECX_IN_ST0;
					psi_ptr++;
					psi_ptr->psi=STORE_ST0_IN_FLOAT;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=cur_tmp_address;
					psi_ptr++;
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_DIVISION;
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_ADDITION;
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_SUBTRACTION;
					else if (operation==LESS_THAN)
						psi_ptr->psi=ST0_FLOAT_LESS_THAN;
					else if (operation==GREATER_THAN)
						psi_ptr->psi=ST0_FLOAT_GREATER_THAN;
					else if (operation==LESS_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_FLOAT_LESS_THAN_OR_EQUAL_TO;
					else if (operation==GREATER_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_FLOAT_GREATER_THAN_OR_EQUAL_TO;
					else if (operation==EQUALITY)
						psi_ptr->psi=ST0_FLOAT_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=ST0_FLOAT_INEQUALITY;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=cur_tmp_address;
				}
				else {
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=EAX_ECX_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT) {

						// ottiene il tipo del primo operando
						if (buffer[j].type == NUMBER)
							n=buffer[j].number.type;
						else if (buffer[j].type == NON_NUMBER)
							n=buffer[j].non_number.type & 0xffff;

						// secondo il tipo del primo operando stabilisce quale istruzione assemblare
						if (n==UNSIGNED_INT)
							psi_ptr->psi=UNSIGNED_EAX_ECX_DIVISION;
						else
							psi_ptr->psi=EAX_ECX_DIVISION;

					}
					else if (operation==REMAINDER || operation==MODULUS_ASSIGNMENT) {

						// ottiene il tipo del primo operando
						if (buffer[j].type == NUMBER)
							n=buffer[j].number.type;
						else if (buffer[j].type == NON_NUMBER)
							n=buffer[j].non_number.type & 0xffff;

						// secondo il tipo del primo operando stabilisce quale istruzione assemblare
						if (n==UNSIGNED_INT)
							psi_ptr->psi=UNSIGNED_EAX_ECX_REMAINDER;
						else
							psi_ptr->psi=EAX_ECX_REMAINDER;

					}
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT) {
						if (with_pointers) {
							psi_ptr->psi=EAX_ECX_PERCONSTANT_ADDITION;
							psi_ptr->constant=with_pointers;
						}
						else
							psi_ptr->psi=EAX_ECX_ADDITION;
					}
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT) {
						if (with_pointers) {
							if (nn1->indirection || nn1->dimensions) {
								psi_ptr->psi=EAX_ECX_OBJECT_SUBTRACTION;
								objsub=1;
							}
							else
								psi_ptr->psi=EAX_ECX_PERCONSTANT_SUBTRACTION;
							psi_ptr->constant=with_pointers;
						}
						else
							psi_ptr->psi=EAX_ECX_SUBTRACTION;
					}
					else if (operation==LEFT_SHIFT || operation==LEFT_SHIFT_ASSIGNMENT)
						psi_ptr->psi=EAX_ECX_LEFT_SHIFT;
					else if (operation==RIGHT_SHIFT || operation==RIGHT_SHIFT_ASSIGNMENT) {

						// ottiene il tipo del primo operando
						if (buffer[j].type == NUMBER)
							n=buffer[j].number.type;
						else if (buffer[j].type == NON_NUMBER)
							n=buffer[j].non_number.type & 0xffff;

						// secondo il tipo del primo operando stabilisce quale istruzione assemblare
						if (n==UNSIGNED_INT)
							psi_ptr->psi=UNSIGNED_EAX_ECX_RIGHT_SHIFT;
						else
							psi_ptr->psi=EAX_ECX_RIGHT_SHIFT;

					}
					else if ( (operation & 0xff00) == 0x0500 ) {

						// ottiene il tipo del primo operando
						if (buffer[j].type == NUMBER)
							n=buffer[j].number.type;
						else if (buffer[j].type == NON_NUMBER) {
							if (buffer[j].non_number.dimensions ||
								buffer[j].non_number.indirection)
								n=UNSIGNED_INT;
							else	
								n=buffer[j].non_number.type & 0xffff;
						}

						// ottiene il tipo del secondo operando
						if (nn1->dimensions || nn1->indirection)
							m=UNSIGNED_INT;
						else
							m=nn1->type & 0xffff;

						// stabilisce quale istruzione assemblare
						if (n==UNSIGNED_INT || m==UNSIGNED_INT) {
							if (operation==LESS_THAN)
								psi_ptr->psi=EAX_ECX_UNSIGNED_LESS_THAN;
							else if (operation==GREATER_THAN)
								psi_ptr->psi=EAX_ECX_UNSIGNED_GREATER_THAN;
							else if (operation==LESS_THAN_OR_EQUAL_TO)
								psi_ptr->psi=EAX_ECX_UNSIGNED_LESS_THAN_OR_EQUAL_TO;
							else if (operation==GREATER_THAN_OR_EQUAL_TO)
								psi_ptr->psi=EAX_ECX_UNSIGNED_GREATER_THAN_OR_EQUAL_TO;
						}
						else {
							if (operation==LESS_THAN)
								psi_ptr->psi=EAX_ECX_LESS_THAN;
							else if (operation==GREATER_THAN)
								psi_ptr->psi=EAX_ECX_GREATER_THAN;
							else if (operation==LESS_THAN_OR_EQUAL_TO)
								psi_ptr->psi=EAX_ECX_LESS_THAN_OR_EQUAL_TO;
							else if (operation==GREATER_THAN_OR_EQUAL_TO)
								psi_ptr->psi=EAX_ECX_GREATER_THAN_OR_EQUAL_TO;
						}

					}
					else if ( (operation & 0xff00) == 0x0600 ) {
						if (operation==EQUALITY)
							psi_ptr->psi=EAX_ECX_EQUALITY;
						else if (operation==INEQUALITY)
							psi_ptr->psi=EAX_ECX_INEQUALITY;
					}
					else if (operation==BITWISE_AND || operation==BITWISE_AND_ASSIGNMENT)
						psi_ptr->psi=EAX_ECX_BITWISE_AND;
					else if (operation==BITWISE_EXCLUSIVE_OR || operation==BITWISE_EXCLUSIVE_OR_ASSIGNMENT)
						psi_ptr->psi=EAX_ECX_BITWISE_EXCLUSIVE_OR;
					else if (operation==BITWISE_OR || operation==BITWISE_INCLUSIVE_OR_ASSIGNMENT)
						psi_ptr->psi=EAX_ECX_BITWISE_OR;
					else if (operation==LOGICAL_AND) {

						// ottiene il tipo del primo operando
						if (buffer[j].type == NUMBER)
							n=buffer[j].number.type;
						else if (buffer[j].type == NON_NUMBER) {
							if (buffer[j].non_number.dimensions ||
								buffer[j].non_number.indirection)
								n=UNSIGNED_INT;
							else	
								n=buffer[j].non_number.type & 0xffff;
						}

						// in base al tipo del primo operando stabilisce quale istruzione assemblare
						if (n>=FLOAT)
							psi_ptr->psi=ST0_ECX_LOGICAL_AND;
						else
							psi_ptr->psi=EAX_ECX_LOGICAL_AND;

					}
					else if (operation==LOGICAL_OR) {

						// ottiene il tipo del primo operando
						if (buffer[j].type == NUMBER)
							n=buffer[j].number.type;
						else if (buffer[j].type == NON_NUMBER) {
							if (buffer[j].non_number.dimensions ||
								buffer[j].non_number.indirection)
								n=UNSIGNED_INT;
							else	
								n=buffer[j].non_number.type & 0xffff;
						}

						// in base al tipo del primo operando stabilisce quale istruzione assemblare
						if (n>=FLOAT)
							psi_ptr->psi=ST0_ECX_LOGICAL_OR;
						else
							psi_ptr->psi=EAX_ECX_LOGICAL_OR;

					}

				}
				psi_ptr++;
			}

		}
		else if (buffer[k].type==NUMBER) {

			// ottiene il tipo del primo operando
			if (buffer[j].type == NUMBER)
				l=buffer[j].number.type;
			else if (buffer[j].type == NON_NUMBER) {
				if (buffer[j].non_number.dimensions ||
					buffer[j].non_number.indirection)
					l=UNSIGNED_INT;
				else
					l=buffer[j].non_number.type & 0xffff;
			}

			// converte il numero secondo il tipo del primo operando
			if ( buffer[k].number.type >= FLOAT || l >= FLOAT ) {
				if ( (operation == LOGICAL_AND ||
					operation == LOGICAL_OR) &&
					l >= FLOAT &&
					buffer[k].number.type < FLOAT ) {
						// stabilisce se il numero è con segno oppure senza segno
						if (buffer[k].number.type == UNSIGNED_CHAR ||
							buffer[k].number.type == UNSIGNED_SHORT ||
							buffer[k].number.type == UNSIGNED_INT)
							m=UNSIGNED_INT;
						else
							m=INT;
					}
				else if ( buffer[k].number.type > FLOAT || l > FLOAT )
					m=DOUBLE;
				else
					m=FLOAT;
			}
			else if (buffer[k].number.type == UNSIGNED_CHAR ||
				buffer[k].number.type == UNSIGNED_SHORT ||
				buffer[k].number.type == UNSIGNED_INT)
				m=UNSIGNED_INT;
			else
				m=INT;
			CastTypeOfConstantNumber(&buffer[k].number,&number,m);

			// carica il numero in eax o st0
			if (m==FLOAT || m==DOUBLE) {
				if (operation==REMAINDER) {
					CompilerErrorOrWarning(119);
					return(1);
				}
				else if (operation==LEFT_SHIFT) {
					CompilerErrorOrWarning(134);
					return(1);
				}
				else if (operation==RIGHT_SHIFT) {
					CompilerErrorOrWarning(135);
					return(1);
				}
				else if (operation==BITWISE_AND ||
					operation==BITWISE_EXCLUSIVE_OR ||
					operation==BITWISE_OR) {
					CompilerErrorOrWarning(142);
					return(1);
				}
				else if (operation==MODULUS_ASSIGNMENT ||
					operation==LEFT_SHIFT_ASSIGNMENT ||
					operation==RIGHT_SHIFT_ASSIGNMENT ||
					operation==BITWISE_AND_ASSIGNMENT ||
					operation==BITWISE_INCLUSIVE_OR_ASSIGNMENT ||
					operation==BITWISE_EXCLUSIVE_OR_ASSIGNMENT) {
					CompilerErrorOrWarning(154);
					return(1);
				}
				if (with_pointers) {
					if (operation==ADDITION)
						CompilerErrorOrWarning(128);
					else if (operation==SUBTRACTION)
						CompilerErrorOrWarning(129);
					else if ( (operation & 0xff00) == 0x0d00 )
						CompilerErrorOrWarning(157);
					return(1);
				}
				if (m==FLOAT) {
					*(float*)strings_ptr=*(float*)number.number;
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_DIVISION;
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_ADDITION;
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
						psi_ptr->psi=ST0_FLOAT_SUBTRACTION;
					else if (operation==LESS_THAN)
						psi_ptr->psi=ST0_FLOAT_LESS_THAN;
					else if (operation==GREATER_THAN)
						psi_ptr->psi=ST0_FLOAT_GREATER_THAN;
					else if (operation==LESS_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_FLOAT_LESS_THAN_OR_EQUAL_TO;
					else if (operation==GREATER_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_FLOAT_GREATER_THAN_OR_EQUAL_TO;
					else if (operation==EQUALITY)
						psi_ptr->psi=ST0_FLOAT_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=ST0_FLOAT_INEQUALITY;
					else if (operation==LOGICAL_AND)
						psi_ptr->psi=ST0_FLOAT_LOGICAL_AND;
					else if (operation==LOGICAL_OR)
						psi_ptr->psi=ST0_FLOAT_LOGICAL_OR;
					psi_ptr->address.address=(int)strings_ptr;
					strings_ptr+=4;
					psi_ptr->address.section=SECTION_STRINGS;
				}
				else {
					*(double*)strings_ptr=*(double*)number.number;
					if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
						psi_ptr->psi=ST0_DOUBLE_MULTIPLICATION;
					else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT)
						psi_ptr->psi=ST0_DOUBLE_DIVISION;
					else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT)
						psi_ptr->psi=ST0_DOUBLE_ADDITION;
					else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT)
						psi_ptr->psi=ST0_DOUBLE_SUBTRACTION;
					else if (operation==LESS_THAN)
						psi_ptr->psi=ST0_DOUBLE_LESS_THAN;
					else if (operation==GREATER_THAN)
						psi_ptr->psi=ST0_DOUBLE_GREATER_THAN;
					else if (operation==LESS_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_DOUBLE_LESS_THAN_OR_EQUAL_TO;
					else if (operation==GREATER_THAN_OR_EQUAL_TO)
						psi_ptr->psi=ST0_DOUBLE_GREATER_THAN_OR_EQUAL_TO;
					else if (operation==EQUALITY)
						psi_ptr->psi=ST0_DOUBLE_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=ST0_DOUBLE_INEQUALITY;
					else if (operation==LOGICAL_AND)
						psi_ptr->psi=ST0_DOUBLE_LOGICAL_AND;
					else if (operation==LOGICAL_OR)
						psi_ptr->psi=ST0_DOUBLE_LOGICAL_OR;
					psi_ptr->address.address=(int)strings_ptr;
					strings_ptr+=8;
					psi_ptr->address.section=SECTION_STRINGS;
				}
			}
			else {

				// considera se caricare la costante in ecx oppure se risolvere direttamente
				if (operation==LEFT_SHIFT || operation==LEFT_SHIFT_ASSIGNMENT) {
					psi_ptr->psi=EAX_CONSTANT_LEFT_SHIFT;
					psi_ptr->constant=*(int*)number.number;
				}
				else if (operation==RIGHT_SHIFT || operation==RIGHT_SHIFT_ASSIGNMENT) {

					// ottiene il tipo del primo operando
					if (buffer[j].type == NUMBER)
						n=buffer[j].number.type;
					else if (buffer[j].type == NON_NUMBER)
						n=buffer[j].non_number.type & 0xffff;

					// secondo il tipo del primo operando stabilisce quale istruzione assemblare
					if (n==UNSIGNED_INT)
						psi_ptr->psi=UNSIGNED_EAX_CONSTANT_RIGHT_SHIFT;
					else
						psi_ptr->psi=EAX_CONSTANT_RIGHT_SHIFT;
					psi_ptr->constant=*(int*)number.number;

				}
				else {
					psi_ptr->psi=LOAD_CONSTANT_IN_ECX;
					psi_ptr->constant=*(int*)number.number;
					psi_ptr++;
				}

				// in ecx c'è la costante; adesso può eseguire l'operazione
				if (operation==MULTIPLICATION || operation==MULTIPLICATION_ASSIGNMENT)
					psi_ptr->psi=EAX_ECX_MULTIPLICATION;
				else if (operation==DIVISION || operation==DIVISION_ASSIGNMENT) {

					// ottiene il tipo del primo operando
					if (buffer[j].type == NUMBER)
						n=buffer[j].number.type;
					else if (buffer[j].type == NON_NUMBER)
						n=buffer[j].non_number.type & 0xffff;

					// secondo il tipo del primo operando stabilisce quale istruzione assemblare
					if (n==UNSIGNED_INT)
						psi_ptr->psi=UNSIGNED_EAX_ECX_DIVISION;
					else
						psi_ptr->psi=EAX_ECX_DIVISION;

				}
				else if (operation==REMAINDER || operation==MODULUS_ASSIGNMENT) {

					// ottiene il tipo del primo operando
					if (buffer[j].type == NUMBER)
						n=buffer[j].number.type;
					else if (buffer[j].type == NON_NUMBER)
						n=buffer[j].non_number.type & 0xffff;

					// secondo il tipo del primo operando stabilisce quale istruzione assemblare
					if (n==UNSIGNED_INT)
						psi_ptr->psi=UNSIGNED_EAX_ECX_REMAINDER;
					else
						psi_ptr->psi=EAX_ECX_REMAINDER;

				}
				else if (operation==ADDITION || operation==ADDITION_ASSIGNMENT) {
					if (with_pointers) {
						psi_ptr->psi=EAX_ECX_PERCONSTANT_ADDITION;
						psi_ptr->constant=with_pointers;
					}
					else
						psi_ptr->psi=EAX_ECX_ADDITION;
				}
				else if (operation==SUBTRACTION || operation==SUBTRACTION_ASSIGNMENT) {
					if (with_pointers) {
						psi_ptr->psi=EAX_ECX_PERCONSTANT_SUBTRACTION;
						psi_ptr->constant=with_pointers;
					}
					else
						psi_ptr->psi=EAX_ECX_SUBTRACTION;
				}
				else if ( (operation & 0xff00) == 0x0500 ) {

					// ottiene il tipo del primo operando
					if (buffer[j].type == NUMBER)
						n=buffer[j].number.type;
					else if (buffer[j].type == NON_NUMBER) {
						if (buffer[j].non_number.dimensions ||
							buffer[j].non_number.indirection)
							n=UNSIGNED_INT;
						else	
							n=buffer[j].non_number.type & 0xffff;
					}

					// ottiene il tipo del secondo operando
					m=buffer[k].number.type;

					// stabilisce quale istruzione assemblare
					if (n==UNSIGNED_INT || m==UNSIGNED_INT) {
						if (operation==LESS_THAN)
							psi_ptr->psi=EAX_ECX_UNSIGNED_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=EAX_ECX_UNSIGNED_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=EAX_ECX_UNSIGNED_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=EAX_ECX_UNSIGNED_GREATER_THAN_OR_EQUAL_TO;
					}
					else {
						if (operation==LESS_THAN)
							psi_ptr->psi=EAX_ECX_LESS_THAN;
						else if (operation==GREATER_THAN)
							psi_ptr->psi=EAX_ECX_GREATER_THAN;
						else if (operation==LESS_THAN_OR_EQUAL_TO)
							psi_ptr->psi=EAX_ECX_LESS_THAN_OR_EQUAL_TO;
						else if (operation==GREATER_THAN_OR_EQUAL_TO)
							psi_ptr->psi=EAX_ECX_GREATER_THAN_OR_EQUAL_TO;
					}

				}
				else if ( (operation & 0xff00) == 0x0600 ) {
					if (operation==EQUALITY)
						psi_ptr->psi=EAX_ECX_EQUALITY;
					else if (operation==INEQUALITY)
						psi_ptr->psi=EAX_ECX_INEQUALITY;
				}
				else if (operation==BITWISE_AND || operation==BITWISE_AND_ASSIGNMENT)
					psi_ptr->psi=EAX_ECX_BITWISE_AND;
				else if (operation==BITWISE_EXCLUSIVE_OR || operation==BITWISE_EXCLUSIVE_OR_ASSIGNMENT)
					psi_ptr->psi=EAX_ECX_BITWISE_EXCLUSIVE_OR;
				else if (operation==BITWISE_OR || operation==BITWISE_INCLUSIVE_OR_ASSIGNMENT)
					psi_ptr->psi=EAX_ECX_BITWISE_OR;
				else if (operation==LOGICAL_AND) {

					// ottiene il tipo del primo operando
					if (buffer[j].type == NUMBER)
						n=buffer[j].number.type;
					else if (buffer[j].type == NON_NUMBER) {
						if (buffer[j].non_number.dimensions ||
							buffer[j].non_number.indirection)
							n=UNSIGNED_INT;
						else	
							n=buffer[j].non_number.type & 0xffff;
					}

					// in base al tipo del primo operando stabilisce quale istruzione assemblare
					if (n>=FLOAT)
						psi_ptr->psi=ST0_ECX_LOGICAL_AND;
					else
						psi_ptr->psi=EAX_ECX_LOGICAL_AND;

				}
				else if (operation==LOGICAL_OR) {

					// ottiene il tipo del primo operando
					if (buffer[j].type == NUMBER)
						n=buffer[j].number.type;
					else if (buffer[j].type == NON_NUMBER) {
						if (buffer[j].non_number.dimensions ||
							buffer[j].non_number.indirection)
							n=UNSIGNED_INT;
						else	
							n=buffer[j].non_number.type & 0xffff;
					}

					// in base al tipo del primo operando stabilisce quale istruzione assemblare
					if (n>=FLOAT)
						psi_ptr->psi=ST0_ECX_LOGICAL_OR;
					else
						psi_ptr->psi=EAX_ECX_LOGICAL_OR;

				}

			}
			psi_ptr++;

		}
		else {
			if (operation==MULTIPLICATION)
				CompilerErrorOrWarning(111);
			else if (operation==DIVISION)
				CompilerErrorOrWarning(114);
			else if (operation==REMAINDER)
				CompilerErrorOrWarning(117);
			else if (operation==ADDITION)
				CompilerErrorOrWarning(123);
			else if (operation==SUBTRACTION)
				CompilerErrorOrWarning(124);
			else if (operation==LEFT_SHIFT)
				CompilerErrorOrWarning(136);
			else if (operation==RIGHT_SHIFT)
				CompilerErrorOrWarning(137);
			else if ( (operation & 0xff00) == 0x0500 )
				CompilerErrorOrWarning(138);
			else if ( (operation & 0xff00) == 0x0600 )
				CompilerErrorOrWarning(139);
			else if (operation==BITWISE_AND ||
				operation==BITWISE_EXCLUSIVE_OR ||
				operation==BITWISE_OR)
				CompilerErrorOrWarning(143);
			else if (operation==LOGICAL_AND ||
				operation==LOGICAL_OR)
				CompilerErrorOrWarning(145);
			else if ( (operation & 0xff00) == 0x0d00 )
				CompilerErrorOrWarning(153);
			return(1);
		}

		// memorizza il risultato di un assignment struttura=struttura
		if (struct_assignment) {
			buffer[i]=buffer[j];
			nn0=&buffer[j].non_number;
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
				psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_ECX;
				psi_ptr->address.section=SECTION_DATA;
				psi_ptr->address.address=nn0->address;
			}
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
				psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_ECX;
				psi_ptr->address.section=SECTION_STACK;
				psi_ptr->address.address=nn0->address;
			}
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
			}
			psi_ptr++;
			psi_ptr->psi=EAX_ECX_INITIALIZE_MOVEDATA;
			psi_ptr++;
			if (nn0->struct_type->address >= 4) {
				psi_ptr->psi=MOVEDATA_DWORD;
				psi_ptr->constant=nn0->struct_type->address / 4;
				psi_ptr++;
			}
			if (nn0->struct_type->address % 4) {
				psi_ptr->psi=MOVEDATA_BYTE;
				psi_ptr->constant=nn0->struct_type->address % 4;
				psi_ptr++;
			}
		}
		// memorizza il risultato di uno special_char_assignment
		else if (special_char_assignment) {
			buffer[i]=buffer[j];
			nn0=&buffer[j].non_number;
			psi_ptr->psi=LOAD_INT_IN_ECX;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nn0->address;
			psi_ptr++;
			psi_ptr->psi=EAX_ECX_INITIALIZE_MOVEDATA;
			psi_ptr++;
			if (special_char_assignment >= 4) {
				psi_ptr->psi=MOVEDATA_DWORD;
				psi_ptr->constant=special_char_assignment / 4;
				psi_ptr++;
			}
			if (special_char_assignment % 4) {
				psi_ptr->psi=MOVEDATA_BYTE;
				psi_ptr->constant=special_char_assignment % 4;
				psi_ptr++;
			}
		}
		// memorizza il risultato in un intero
		else if ( (operation & 0xff00) == 0x0500 ||
			(operation & 0xff00) == 0x0600 ||
			operation==LOGICAL_AND ||
			operation==LOGICAL_OR ||
			objsub) {

			// prima di tutto controlla che non siamo nel caso di un pointer-=pointer
			if (operation == SUBTRACTION_ASSIGNMENT) {
				CompilerErrorOrWarning(173);
				return(1);
			}

			// memorizza il risultato
			buffer[i].type=NON_NUMBER;
			nn0=&buffer[i].non_number;
			nn0->type=INT;
			nn0->type|=RIGHT_VALUE;
			nn0->address=cur_tmp_address;
			psi_ptr->psi=STORE_EAX_IN_INT;
			psi_ptr->address.address=cur_tmp_address;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			cur_tmp_address+=4;
			psi_ptr++;
			nn0->indirection=0;
			nn0->dimensions=0;
			nn0->dimensions_list=NULL;
			nn0->const_flag=0;
			nn0->struct_type=NULL;
			nn0->is_pointer=0;
			nn0->array_stptr=0;

		}
		// memorizza nella SECTION_TEMPDATA1 il risultato dell'operazione tra pointers
		else if (with_pointers) {
			buffer[i]=buffer[j];
			nn0=&buffer[i].non_number;

			// è un'assegnazione composta
			if ( operation == ADDITION_ASSIGNMENT ||
				operation == SUBTRACTION_ASSIGNMENT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=STORE_EAX_IN_STACK_INT;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
				}
				psi_ptr++;
			}
			// non è un'assegnazione composta
			else {
				nn0->type&=0xffff;
				nn0->type|=RIGHT_VALUE;
				nn0->address=cur_tmp_address;
				nn0->is_pointer=0;
				psi_ptr->psi=STORE_EAX_IN_INT;
				psi_ptr->address.address=cur_tmp_address;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				cur_tmp_address+=4;
				psi_ptr++;
			}
		}
		// memorizza il risultato di un assignment
		else if ( (operation & 0xff00) == 0x0d00 ) {
			buffer[i]=buffer[j];
			if (operation == ASSIGNMENT) {
				if (buffer[j].type == NUMBER)
					l=buffer[j].number.type;
				else if (buffer[j].type == NON_NUMBER) {
					if (buffer[j].non_number.indirection ||
						buffer[j].non_number.dimensions)
						l=UNSIGNED_INT;
					else
						l=buffer[j].non_number.type & 0xffff;
				}
				if (buffer[k].type == NUMBER)
					m=buffer[k].number.type;
				else if (buffer[k].type == NON_NUMBER) {
					if (buffer[k].non_number.indirection ||
						buffer[k].non_number.dimensions)
						m=UNSIGNED_INT;
					else
						m=buffer[k].non_number.type & 0xffff;
				}
				if (l<FLOAT && m>=FLOAT) {
					psi_ptr->psi=LOAD_ST0_IN_EAX;
					psi_ptr++;
				}
			}
			else {
				if (eax_in_st0) {
					psi_ptr->psi=LOAD_ST0_IN_EAX;
					psi_ptr++;
				}
			}
			nn0=&buffer[j].non_number;
			if (nn0->indirection) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=STORE_EAX_IN_INT;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=STORE_EAX_IN_STACK_INT;
					psi_ptr->address.address=nn0->address;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;
					psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
				}
				psi_ptr++;
			}
			else {
				if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					// carica in ecx il puntatore
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;

					// memorizza eax o st0 nella variabile puntata da ecx
					if ( (nn0->type & 0xff00) == CHAR )
						psi_ptr->psi=STORE_EAX_IN_ECXPTR_CHAR;
					else if ( (nn0->type & 0xff00) == SHORT )
						psi_ptr->psi=STORE_EAX_IN_ECXPTR_SHORT;
					else if ( (nn0->type & 0xff00) == INT )
						psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
					else if ( (nn0->type & 0xffff) == FLOAT ) {
						if ( do_not_use_st0 )
							psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
						else
							psi_ptr->psi=STORE_ST0_IN_ECXPTR_FLOAT;
					}
					else if ( (nn0->type & 0xff00) == DOUBLE ) {
						if ( do_not_use_st0 ) {
							psi_ptr->psi=STORE_EAX_IN_ECXPTR_INT;
							psi_ptr++;
							psi_ptr->psi=STORE_EDX_IN_ECXPLUS4PTR_INT;
						}
						else
							psi_ptr->psi=STORE_ST0_IN_ECXPTR_DOUBLE;
					}
					psi_ptr++;
				}
				else {
					// individua l'origine del valore
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
						psi_ptr->address.section=SECTION_DATA;
					else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;

					// memorizza eax o st0
					if ( (nn0->type & 0xff00) == CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=STORE_EAX_IN_STACK_CHAR;
						else
							psi_ptr->psi=STORE_EAX_IN_CHAR;
					}
					else if ( (nn0->type & 0xff00) == SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=STORE_EAX_IN_STACK_SHORT;
						else
							psi_ptr->psi=STORE_EAX_IN_SHORT;
					}
					else if ( (nn0->type & 0xff00) == INT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=STORE_EAX_IN_STACK_INT;
						else
							psi_ptr->psi=STORE_EAX_IN_INT;
					}
					else if ( (nn0->type & 0xffff) == FLOAT ) {
						if ( do_not_use_st0 ) {
							if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
								psi_ptr->psi=STORE_EAX_IN_STACK_INT;
							else
								psi_ptr->psi=STORE_EAX_IN_INT;
						}
						else {
							if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
								psi_ptr->psi=STORE_ST0_IN_STACK_FLOAT;
							else
								psi_ptr->psi=STORE_ST0_IN_FLOAT;
						}
					}
					else if ( (nn0->type & 0xff00) == DOUBLE ) {
						if ( do_not_use_st0 ) {
							if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
								psi_ptr->psi=STORE_EAX_IN_STACK_INT;
								*(psi_ptr+1)=*psi_ptr;
								psi_ptr++;
								psi_ptr->address.address+=4;
								psi_ptr->psi=STORE_EDX_IN_STACK_INT;
							}
							else {
								psi_ptr->psi=STORE_EAX_IN_INT;
								*(psi_ptr+1)=*psi_ptr;
								psi_ptr++;
								psi_ptr->address.address+=4;
								psi_ptr->psi=STORE_EDX_IN_INT;
							}
						}
						else {
							if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
								psi_ptr->psi=STORE_ST0_IN_STACK_DOUBLE;
							else
								psi_ptr->psi=STORE_ST0_IN_DOUBLE;
						}
					}
					psi_ptr++;
				}
			}
		}
		// memorizza il risultato delle restanti operazioni
		else {
			// memorizza il risultato dell'operazione
			buffer[i].type=NON_NUMBER;
			nn0=&buffer[i].non_number;

			// stabilisce il tipo del risultato restituito
			if (buffer[j].type == NUMBER)
				l=buffer[j].number.type;
			else if (buffer[j].type == NON_NUMBER)
				l=buffer[j].non_number.type & 0xffff;
			if (buffer[k].type == NUMBER)
				m=buffer[k].number.type;
			else if (buffer[k].type == NON_NUMBER)
				m=buffer[k].non_number.type & 0xffff;
			if (l<=UNSIGNED_INT && m<=UNSIGNED_INT) {
				if (l==UNSIGNED_CHAR || m==UNSIGNED_CHAR ||
					l==UNSIGNED_SHORT || m==UNSIGNED_SHORT ||
					l==UNSIGNED_INT || m==UNSIGNED_INT)
					nn0->type=UNSIGNED_INT;
				else
					nn0->type=INT;
				nn0->type|=RIGHT_VALUE;
				nn0->address=cur_tmp_address;
				psi_ptr->psi=STORE_EAX_IN_INT;
				psi_ptr->address.address=cur_tmp_address;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				cur_tmp_address+=4;
				psi_ptr++;
			}
			else if (l>FLOAT || m>FLOAT) {
				nn0->type=DOUBLE;
				nn0->type|=RIGHT_VALUE;
				nn0->address=cur_tmp_address;
				psi_ptr->psi=STORE_ST0_IN_DOUBLE;
				psi_ptr->address.address=cur_tmp_address;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				cur_tmp_address+=8;
				psi_ptr++;
			}
			else {
				nn0->type=FLOAT;
				nn0->type|=RIGHT_VALUE;
				nn0->address=cur_tmp_address;
				psi_ptr->psi=STORE_ST0_IN_FLOAT;
				psi_ptr->address.address=cur_tmp_address;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				cur_tmp_address+=4;
				psi_ptr++;
			}
			nn0->indirection=0;
			nn0->dimensions=0;
			nn0->dimensions_list=NULL;
			nn0->const_flag=0;
			nn0->struct_type=NULL;
			nn0->is_pointer=0;
			nn0->array_stptr=0;
		}
	}

	// ritorna
	return(0);
}

//===================
//PSI_PushNonNumber
//
//Traduce in istruzioni PSI un push nello stack di un nonnumber_t
//===================
int PSI_PushNonNumber (nonnumber_t *nn, identifier_t *par)
{
	int				i;
	nonnumber_t		nnx;

	// controlla se il tipo del parametro è void
	if (!par->dimensions &&
		!par->indirection &&
		(par->type & 0xffff) == VOID_TYPE) {
		if ( nn->dimensions ||
			nn->indirection ||
			(nn->type & 0xffff) != VOID_TYPE ) {
			CompilerErrorOrWarning(85);
			return(1);
		}
		else
			return(0);
	}

	// controlla se il parametro che si intende passare è compatibile col parametro del prototipo
	i=0;
	if ( (par->type & 0xffff) == STRUCTURE_TYPE )
		i++;
	if ( (nn->type & 0xffff) == STRUCTURE_TYPE )
		i++;
	if (i==1) {
		CompilerErrorOrWarning(83);
		return(1);
	}
	if (!par->dimensions || par->dimensions==1) {
		i=nn->indirection;
		if (nn->dimensions>1) {
			CompilerErrorOrWarning(83);
			return(1);
		}
		else if (nn->dimensions)
			i++;
		if (par->indirection+par->dimensions!=i) {
			CompilerErrorOrWarning(83);
			return(1);
		}
		if (i) {
			if ( (par->type & 0xffff) != (nn->type & 0xffff) ) {
				if ( (par->type & 0xffff) != VOID_TYPE ) {
					CompilerErrorOrWarning(83);
					return(1);
				}
			}
		}
	}
	else {
		if ( par->dimensions!=nn->dimensions ||
			par->indirection!=nn->indirection ||
			( (par->type & 0xffff) != (nn->type & 0xffff) &&
			(par->type & 0xffff) != VOID_TYPE ) ) {
			CompilerErrorOrWarning(83);
			return(1);
		}
	}

	// prima di passare il nonnumber lo converte
	nnx.type=par->type & 0xffff;
	nnx.indirection=par->indirection;
	nnx.dimensions=par->dimensions;
	nnx.dimensions_list=par->pointer;
	nnx.const_flag=par->const_flag;
	nnx.struct_type=par->struct_type;
	nnx.is_pointer=0;
	nnx.array_stptr=0;
	if (CastTypeOfNonConstantNumber(nn,&nnx,1))
		return(1);

	// deve passare un puntatore alla matrice
	if (nnx.dimensions) {

		if (nnx.array_stptr) {
			psi_ptr->psi=PUSH_STACK_INT;
			psi_ptr->address.section=SECTION_STACK;
			psi_ptr->address.address=nnx.address;
		}
		else if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 ) {
			psi_ptr->psi=PUSH_CONSTANT_ADDRESS;
			psi_ptr->address.section=SECTION_DATA;
			psi_ptr->address.address=nnx.address;
		}
		else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 ) {
			psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
			psi_ptr->address.section=SECTION_STACK;
			psi_ptr->address.address=nnx.address;
			psi_ptr++;
			psi_ptr->psi=PUSH_EAX;
		}
		else if ( (nnx.type & 0xff0000) == RIGHT_VALUE ) {
			psi_ptr->psi=PUSH_INT;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nnx.address;
		}
		psi_ptr++;

	}
	// deve passare un puntatore
	else if (nnx.indirection) {

		if (nnx.is_pointer) {
			psi_ptr->psi=LOAD_INT_IN_ECX;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nnx.address;
			psi_ptr++;
			psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			psi_ptr++;
			psi_ptr->psi=PUSH_EAX;
		}
		else {
			if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 ) {
				psi_ptr->psi=PUSH_INT;
				psi_ptr->address.section=SECTION_DATA;
			}
			else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 ) {
				psi_ptr->psi=PUSH_STACK_INT;
				psi_ptr->address.section=SECTION_STACK;
			}
			else if ( (nnx.type & 0xff0000) == RIGHT_VALUE ) {
				psi_ptr->psi=PUSH_INT;
				psi_ptr->address.section=SECTION_TEMPDATA1;
			}
			psi_ptr->address.address=nnx.address;
		}
		psi_ptr++;

	}
	// deve passare il valore
	else {

		// passa un RIGHT_VALUE pointer
		if (nnx.is_pointer) {

			// carica in ecx il puntatore
			psi_ptr->psi=LOAD_INT_IN_ECX;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nnx.address;
			psi_ptr++;

			// carica nello stack il valore
			if ( (nnx.type & 0xff00) == CHAR ) {
				psi_ptr->psi=LOAD_ECXPTR_CHAR_IN_EAX;
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
			}
			else if ( (nnx.type & 0xff00) == SHORT ) {
				psi_ptr->psi=LOAD_ECXPTR_SHORT_IN_EAX;
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
			}
			else if ( (nnx.type & 0xff00) == INT ||
				(nnx.type & 0xffff) == FLOAT ) {
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
			}
			else if ( (nnx.type & 0xff00) == DOUBLE ) {
				psi_ptr->psi=LOAD_ECXPLUS4PTR_INT_IN_EAX;
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
				psi_ptr++;
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
			}
			psi_ptr++;

		}
		// non è un RIGHT_VALUE pointer...
		else {

			// carica nello stack il valore
			if ( (nnx.type & 0xff00) == CHAR ) {

				// individua l'origine del valore
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nnx.type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nnx.address;

				// carica in eax il valore char
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_CHAR_IN_EAX;
				else
					psi_ptr->psi=LOAD_CHAR_IN_EAX;
				psi_ptr++;

				// carica eax nello stack
				psi_ptr->psi=PUSH_EAX;
				psi_ptr++;

			}
			else if ( (nnx.type & 0xff00) == SHORT ) {

				// individua l'origine del valore
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nnx.type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nnx.address;

				// carica in eax il valore short
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_SHORT_IN_EAX;
				else
					psi_ptr->psi=LOAD_SHORT_IN_EAX;
				psi_ptr++;

				// carica eax nello stack
				psi_ptr->psi=PUSH_EAX;
				psi_ptr++;

			}
			else if ( (nnx.type & 0xff00) == INT ||
				(nnx.type & 0xffff) == FLOAT ) {

				// individua l'origine del valore
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nnx.type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nnx.address;

				// carica in eax il valore int/float
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
				else
					psi_ptr->psi=LOAD_INT_IN_EAX;
				psi_ptr++;

				// carica eax nello stack
				psi_ptr->psi=PUSH_EAX;
				psi_ptr++;

			}
			else if ( (nnx.type & 0xff00) == DOUBLE ) {

				// individua l'origine del valore
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nnx.type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;

				// carica nello stack il double
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->address.address=nnx.address+4;
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
				}
				else {
					psi_ptr->address.address=nnx.address+4;
					psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
				psi_ptr++;
				*psi_ptr=*(psi_ptr-2);
				psi_ptr->address.address-=4;
				psi_ptr++;
				psi_ptr->psi=PUSH_EAX;
				psi_ptr++;

			}
			else {	// struttura

				// carica in eax l'indirizzo della struttura
				if ( (nnx.type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
				}
				else if ( (nnx.type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
				}
				else if ( (nnx.type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
				}
				psi_ptr->address.address=nnx.address;
				psi_ptr++;

				// carica in ecx la dimensione della struttura arrotondata per accesso di 4
				psi_ptr->psi=LOAD_CONSTANT_IN_ECX;
				psi_ptr->constant=nnx.struct_type->address;
				if (psi_ptr->constant % 4)
					psi_ptr->constant=(psi_ptr->constant+4>>2)<<2;
				psi_ptr++;

				// imposta il codice per il caricamento nello stack della struttura
				psi_ptr->psi=EAX_ECX_PUSH_MEMORY_BLOCK;
				psi_ptr++;

			}
		}
	}

	// ritorna
	return(0);
}

//=====================
//CastTypeOfNonConstantNumber
//
//Genera il codice per la modificazione di un tipo di variabile in un altro
//NOTA: di nn1 devono essere impostati tutti i campi eccetto address e i bit 0xff0000 di type
//		e sia is_pointer sia array_stptr devono essere uguali a 0
//=====================
int CastTypeOfNonConstantNumber (nonnumber_t *nn0, nonnumber_t *nn1, int standard_conversion)
{
	int		i;

	// controlla se il type cast è legale
	if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
		!nn0->dimensions &&
		!nn0->indirection ) {
		if ( (nn1->type & 0xffff) != STRUCTURE_TYPE ||
			nn1->dimensions ||
			nn1->indirection ||
			nn0->struct_type != nn1->struct_type) {
			CompilerErrorOrWarning(84);
			return(1);
		}
		else {
			nn1->type=nn0->type;
			nn1->address=nn0->address;
			return(0);
		}
	}
	if ( (nn1->type & 0xffff) == STRUCTURE_TYPE &&
		!nn1->dimensions &&
		!nn1->indirection ) {
		if ( (nn0->type & 0xffff) != STRUCTURE_TYPE ||
			nn0->dimensions ||
			nn0->indirection ||
			nn0->struct_type != nn1->struct_type) {
			CompilerErrorOrWarning(84);
			return(1);
		}
		else {
			nn1->type=nn0->type;
			nn1->address=nn0->address;
			return(0);
		}
	}

	// carica un puntatore ad una matrice in eax
	if (nn0->dimensions) {
		if (!nn1->dimensions &&
			!nn1->indirection) {
			if ( (nn1->type & 0xffff) >= FLOAT) {
				CompilerErrorOrWarning(84);
				return(1);
			}
		}
		// controlla se la conversione è legale anche se è standard
		if (standard_conversion) {
			if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) &&
				(nn1->type & 0xffff) != VOID_TYPE ) {
				CompilerErrorOrWarning(100);
				return(1);
			}
			if (nn0->indirection != nn1->indirection ||
				nn0->dimensions != nn1->dimensions) {
				if (nn0->dimensions!=1 || nn1->dimensions ||
					nn0->indirection + 1 != nn1->indirection) {
					CompilerErrorOrWarning(100);
					return(1);
				}
			}
			else {
				for (i=1;i<nn0->dimensions;i++)
					if (nn0->dimensions_list[i] != nn1->dimensions_list[i]) {
						CompilerErrorOrWarning(100);
						return(1);
					}
			}
			if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
				nn0->struct_type != nn1->struct_type &&
				(nn1->type & 0xffff) != VOID_TYPE ) {
				CompilerErrorOrWarning(100);
				return(1);
			}
		}
		if (nn0->array_stptr) {
			psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
			psi_ptr->address.section=SECTION_STACK;
		}
		else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
			psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
			psi_ptr->address.section=SECTION_DATA;
		}
		else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
			psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
			psi_ptr->address.section=SECTION_STACK;
		}
		else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
			if (!nn1->dimensions &&
				!nn1->indirection &&
				(nn1->type & 0xff00) != INT ) {
				psi_ptr->psi=LOAD_INT_IN_EAX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
			}
			else {
				nn1->type|=RIGHT_VALUE;
				nn1->address=nn0->address;
				return(0);
			}
		}
		psi_ptr->address.address=nn0->address;
	}
	// carica un puntatore in eax
	else if (nn0->indirection) {
		if (!nn1->dimensions &&
			!nn1->indirection) {
			if ( (nn1->type & 0xffff) >= FLOAT) {
				CompilerErrorOrWarning(84);
				return(1);
			}
		}
		// controlla se la conversione è legale anche se è standard
		if (standard_conversion) {
			if ( (nn0->type & 0xffff) != (nn1->type & 0xffff) &&
				(nn1->type & 0xffff) != VOID_TYPE ) {
				CompilerErrorOrWarning(100);
				return(1);
			}
			if (nn0->indirection != nn1->indirection ||
				nn1->dimensions) {
				if (nn1->dimensions!=1 ||
					nn1->indirection + 1 != nn0->indirection) {
					CompilerErrorOrWarning(100);
					return(1);
				}
			}
			if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
				nn0->struct_type != nn1->struct_type &&
				(nn1->type & 0xffff) != VOID_TYPE ) {
				CompilerErrorOrWarning(100);
				return(1);
			}
		}
		if (nn0->is_pointer) {
			if (!nn1->indirection &&
				!nn1->dimensions &&
				(nn1->type & 0xff00) != INT) {
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			}
			else {
				nn1->type|=RIGHT_VALUE;
				nn1->address=nn0->address;
				nn1->is_pointer=1;
				return(0);
			}
		}
		else {
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
				if ( (nn1->indirection &&
					!nn1->dimensions) ||
					(!nn1->indirection &&
					!nn1->dimensions &&
					(nn1->type & 0xff00) == INT) ) {
					nn1->type|=LEFT_VALUE_0;
					nn1->address=nn0->address;
					return(0);
				}
				else {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
				}
			}
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
				if ( (nn1->indirection &&
					!nn1->dimensions) ||
					(!nn1->indirection &&
					!nn1->dimensions &&
					(nn1->type & 0xff00) == INT) ) {
					nn1->type|=LEFT_VALUE_1;
					nn1->address=nn0->address;
					return(0);
				}
				else {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
				}
			}
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
				if (!nn1->indirection &&
					!nn1->dimensions &&
					(nn1->type & 0xff00) != INT) {
						psi_ptr->psi=LOAD_INT_IN_EAX;
						psi_ptr->address.section=SECTION_TEMPDATA1;
				}
				else {
					nn1->type|=RIGHT_VALUE;
					nn1->address=nn0->address;
					return(0);
				}
			}
			psi_ptr->address.address=nn0->address;
		}
	}
	// carica in eax o st0 il valore
	else {
		// verifica la legalità del type cast
		if ( (nn1->indirection ||
			nn1->dimensions) &&
			(nn0->type & 0xffff) >= FLOAT ) {
			CompilerErrorOrWarning(84);
			return(1);
		}

		// controlla se la conversione è legale anche se è standard
		if (standard_conversion) {
			if (nn1->indirection ||
				nn1->dimensions) {
				CompilerErrorOrWarning(100);
				return(1);
			}
		}

		// controlla se è necessario continuare
		if ( !nn1->dimensions &&
			!nn1->indirection &&
			(nn0->type & 0xffff) == (nn1->type & 0xffff) ) {
			nn1->type=nn0->type;
			nn1->address=nn0->address;
			nn1->is_pointer=nn0->is_pointer;
			return(0);
		}
		else if ( nn1->indirection &&
			( (nn0->type & 0xffff) == INT ||
			(nn0->type & 0xffff) == UNSIGNED_INT ) ) {

			nn1->type|=(nn0->type & 0xff0000);
			nn1->address=nn0->address;
			nn1->is_pointer=nn0->is_pointer;
			return(0);

		}

		// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
		if (nn0->is_pointer) {

			// carica in ecx il puntatore
			psi_ptr->psi=LOAD_INT_IN_ECX;
			psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nn0->address;
			psi_ptr++;

			// carica in eax il contenuto del puntatore ecx
			if ( (nn0->type & 0xffff) == CHAR )
				psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
			else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
				psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
			else if ( (nn0->type & 0xffff) == SHORT )
				psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
			else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
				psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
			else if ( (nn0->type & 0xffff) == INT )
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
				psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
			else if ( (nn0->type & 0xffff) == FLOAT )
				psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
			else if ( (nn0->type & 0xffff) == DOUBLE )
				psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
			else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
				psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

		}
		// carica in eax o st0 il valore
		else {

			// individua l'origine del valore
			if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
				psi_ptr->address.section=SECTION_DATA;
			else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
				psi_ptr->address.section=SECTION_STACK;
			else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
				psi_ptr->address.section=SECTION_TEMPDATA1;
			psi_ptr->address.address=nn0->address;

			// carica in eax o st0 il valore
			if ( (nn0->type & 0xffff) == CHAR ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
				else
					psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
				else
					psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == SHORT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
				else
					psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
				else
					psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == INT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
				else
					psi_ptr->psi=LOAD_INT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
				else
					psi_ptr->psi=LOAD_INT_IN_EAX;
			}
			else if ( (nn0->type & 0xffff) == FLOAT ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
				else
					psi_ptr->psi=LOAD_FLOAT_IN_ST0;
			}
			else if ( (nn0->type & 0xffff) == DOUBLE ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
				else
					psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
			}
			else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
				else
					psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
			}

		}

		// carica in eax il contenuto del primo registro del coprocessore se necessario
		if ( (nn0->type & 0xffff) >= FLOAT &&
			(nn1->type & 0xffff) < FLOAT) {
			psi_ptr++;
			psi_ptr->psi=LOAD_ST0_IN_EAX;
		}
		// carica eax in st0
		else if ( (nn0->type & 0xffff) == UNSIGNED_INT &&
			(nn1->type & 0xffff) >= FLOAT) {
			psi_ptr++;
			psi_ptr->psi=LOAD_UNSIGNED_EAX_IN_ST0;
		}
		else if ( (nn0->type & 0xffff) < FLOAT &&
			(nn1->type & 0xffff) >= FLOAT) {
			psi_ptr++;
			psi_ptr->psi=LOAD_SIGNED_EAX_IN_ST0;
		}

	}
	psi_ptr++;

	// memorizza in tempdata1 il risultato del type cast
	nn1->type|=RIGHT_VALUE;
	nn1->address=cur_tmp_address;
	psi_ptr->address.section=SECTION_TEMPDATA1;
	psi_ptr->address.address=cur_tmp_address;

	if ( (nn1->type & 0xff00) == INT ||
		(nn1->dimensions || nn1->indirection) ) {
		cur_tmp_address+=4;
		psi_ptr->psi=STORE_EAX_IN_INT;
	}
	else if ( (nn1->type & 0xff00) == CHAR ) {
		cur_tmp_address+=1;
		psi_ptr->psi=STORE_EAX_IN_CHAR;
	}
	else if ( (nn1->type & 0xff00) == SHORT ) {
		cur_tmp_address+=2;
		psi_ptr->psi=STORE_EAX_IN_SHORT;
	}
	else if ( (nn1->type & 0xffff) == FLOAT ) {
		cur_tmp_address+=4;
		psi_ptr->psi=STORE_ST0_IN_FLOAT;
	}
	else if ( (nn1->type & 0xff00) == DOUBLE ) {
		cur_tmp_address+=8;
		psi_ptr->psi=STORE_ST0_IN_DOUBLE;
	}
	psi_ptr++;

	// ritorna
	return(0);
}

//=====================
//ResolveNonConstantExpression
//
//Restituisce il risultato di una espressione non costante
//=====================
#define		ebuffer		1
int ResolveNonConstantExpression (exprvalue_t *result, char *string)
{
	token_t			*tok_ptr=(token_t*)((char *)expr_tokens+ebuffer*(int)(single_expr_tokens_buffer*compiler_memory_factor));
	operator_t		*op_ptr=(operator_t*)((char*)operators+ebuffer*(int)(single_operators_buffer*compiler_memory_factor));
	exprvalue_t		*buffer=rcemem;

	token_t			*pointer,*last_op=NULL,*next_op=NULL;
	char			tmpstring[4096];
	int				ret;
	int				end_id;

	int				len;
	int				i;

	identifier_t	*id;

	nonnumber_t		*nn;

	instruction_t	*old_psi_ptr;

	// imposta old_psi_ptr
	old_psi_ptr=psi_ptr;

	// analizza i diversi token dell'espressione
	if (ParseExpressionTokens(string,ebuffer))
		return(1);

	// processa le stringhe e gli identificatori dell'espressione
	for (pointer=tok_ptr;pointer->id!=EXPRESSION_END;pointer++) {
		if (!pointer->tok_taken) {

			// imposta last_op oppure next_op
			if ( (pointer->id & 0xff00) != 0xff00 )
				last_op=pointer;
			else {
				for (next_op=pointer+1;next_op->id!=EXPRESSION_END;next_op++)
					if ( (next_op->id & 0xff00) != 0xff00 )
						break;
			}

			// processa gli identificatori
			if (pointer->id==IDENTIFIER) {

				// prima verifica se saltare questo identificatore
				if ( ( !last_op ||
					( last_op->id != MEMBER_SELECTION_OBJECT &&
					last_op->id != MEMBER_SELECTION_POINTER ) ) &&
					!( next_op == pointer + 1 && next_op->id == FUNCTION_CALL ) ) { // VPCICE PATCH // // 20MAG2004 //

					// interpreta un numero
					if ( *(char*)pointer->ptr0 >= '0' && *(char*)pointer->ptr0 <= '9' ) {
						buffer[pointer-tok_ptr].type=NUMBER;
						ret=ConvertStringToNumber(&buffer[pointer-tok_ptr].number,pointer->ptr0);
						if (ret) {
							CompilerErrorOrWarning(26);
							return(1);
						}
					}

					// interpreta un identificatore
					else {

						// cerca l'identificatore
						len=strlen(pointer->ptr0);
						if (len>=identifier_max_len) {
							len=identifier_max_len-1;
							CompilerErrorOrWarning(1);
						}
						memcpy(tmpstring,pointer->ptr0,len);
						tmpstring[len]=0;

						for (i=0;i<cident_id;i++)
							if ( (cident[i].type & 0xff0000) == VARIABLE &&
								!strcmp(tmpstring,cident[i].id) )
								break;
						if (i==cident_id) {
							CompilerErrorOrWarning(171);
							return(1);
						}
						else
							id=&cident[i];

						// imposta i dati relativi all'identificatore nel buffer
						buffer[pointer-tok_ptr].type=NON_NUMBER;
						nn=&buffer[pointer-tok_ptr].non_number;
						if (i < cident_id_local)
							nn->type=LEFT_VALUE_0;
						else
							nn->type=LEFT_VALUE_1;
						nn->type|=id->type & 0xffff;
						nn->address=id->address;
						nn->indirection=id->indirection;
						nn->dimensions=id->dimensions;
						nn->dimensions_list=id->pointer;
						nn->const_flag=id->const_flag;
						nn->struct_type=id->struct_type;
						nn->is_pointer=0;
						nn->array_stptr=id->array_stptr;

					}

				}
				else
					buffer[pointer-tok_ptr].type=NOTHING_;

			}
			// processa le stringhe
			else if (pointer->id==STRING) {

				// processa una costante
				if (*(char*)pointer->ptr0 == '\'') {
					tmpstring[0]='\'';
					ret=EliminateEscapeSequences((char*)pointer->ptr0+1,&tmpstring[1],strlen(pointer->ptr0)-2,1);
					tmpstring[ret+1]='\'';
					tmpstring[ret+2]=0;
					buffer[pointer-tok_ptr].type=NUMBER;
					ret=ConvertStringToNumber(&buffer[pointer-tok_ptr].number,tmpstring);
					if (ret) {
						CompilerErrorOrWarning(26);
						return(1);
					}
				}

				// processa una stringa
				else {

					// interpreta le escape sequences
					ret=EliminateEscapeSequences((char*)pointer->ptr0+1,strings_ptr,strlen(pointer->ptr0)-2,1);
					strings_ptr[ret]=0;

					// imposta i dati relativi all'identificatore nel buffer
					buffer[pointer-tok_ptr].type=NON_NUMBER;
					nn=&buffer[pointer-tok_ptr].non_number;
					nn->type=CHAR | LEFT_VALUE_0;
					nn->address=(strings_ptr-strings) | 0x40000000;
					nn->indirection=0;
					nn->dimensions=1;
					*(int*)cident_aux2_ptr=ret+1;
					nn->dimensions_list=cident_aux2_ptr;
					(int*)cident_aux2_ptr+=1;
					nn->const_flag=0;
					nn->struct_type=NULL;
					nn->is_pointer=0;
					nn->array_stptr=0;

					// incrementa strings_ptr
					strings_ptr+=ret+1;

				}

			}

		}
	}

	// ordina gli operatori e crea lo stack delle operazioni
	if (MakeExprOrderedOpList(ebuffer))
		return(1);

	// riordina lo stack per le operazioni && || ?: ,
	ReorderStackForLogicalAndConditionalOpAndComma(op_ptr,tok_ptr);

	// trova l'ultimo elemento dello stack
	for (end_id=0;op_ptr[end_id].id!=EXPRESSION_END;end_id++) {}

	// resetta cur_tmp_address
	cur_tmp_address=0;

	// esegue le varie operazioni
	for (end_id--;end_id>=0;end_id--) {

		// valuta se impostare single_operator (per i ++ e -- postfissi)
		if ( !end_id && !DeactivateAutomaticCodeReduction )
			single_operator=1;
		else
			single_operator=0;

		// risolve questa operazione
		if (ResolveNonConstantOperation(op_ptr,buffer,tok_ptr,end_id))
			return(1);

		// controlla che la memoria sia ok
		if (CompilerMemoryCheckPoint())
			return(1);

	}

	// restituisce il risultato
	*result=buffer[result_id];

	// tenta di ridurre il codice appena assemblato
	if (!DeactivateAutomaticCodeReduction)
		ReducePSICode(old_psi_ptr-psi_mem,
			psi_ptr-psi_mem-1);
	else
		DeactivateAutomaticCodeReduction=0;

	// ritorna
	return(0);
}
#undef		ebuffer

//===================
//GeneratePSIOutput
//
//Memorizza su file una porzione del codice PSI
//===================
int GeneratePSIOutput (MEMFILE_NAME(FILE) *stream, int PSI0index, int PSI1index)
{
	int		i,j,k;

	// ciclo per tutte le istruzioni indicate
	for (i=PSI0index;i<=PSI1index;i++) {

		// memorizza su file la stringa
		if (psi_mem[i].psi != NOP) {
			j=*psi[psi_mem[i].psi]->addresses;
			k=*psi[psi_mem[i].psi]->constants;
			if (j!=-1 && k!=-1) {
				MEMFILE_NAME(fprintf)(stream,"%X.%x: %s (%s:%x,%x)\n",
					i,(int)&psi_mem[i],
					psi_names[psi_mem[i].psi],
					sections_names[psi_mem[i].address.section],
					psi_mem[i].address.address,
					psi_mem[i].constant);
			}
			else if (j==-1 && k!=-1) {
				MEMFILE_NAME(fprintf)(stream,"%X.%x: %s (%x)\n",
					i,(int)&psi_mem[i],
					psi_names[psi_mem[i].psi],
					psi_mem[i].constant);
			}
			else if (j!=-1 && k==-1) {
				MEMFILE_NAME(fprintf)(stream,"%X.%x: %s (%s:%x)\n",
					i,(int)&psi_mem[i],
					psi_names[psi_mem[i].psi],
					sections_names[psi_mem[i].address.section],
					psi_mem[i].address.address);
			}
			else {
				MEMFILE_NAME(fprintf)(stream,"%X.%x: %s\n",
					i,(int)&psi_mem[i],
					psi_names[psi_mem[i].psi]);
			}
		}

	}

	// ritorna
	return(0);
}

//===================
//IsTypeCastToken
//
//Restituisce 1 se il token appartiene all'argomento di un type cast
//===================
int IsTypeCastToken (token_t *tok)
{
	char		*pointer;
	int			len;
	char		ident[256];
	int			i;

	// controlla i casi più banali
	if (tok->id==DEREFERENCE ||
		tok->id==MULTIPLICATION)
		return(1);
	else if (tok->id != IDENTIFIER)
		return(0);
	pointer=tok->ptr0;

	// calcola in len il numero di caratteri dell'identificatore
	len=strlen(pointer);

	// verifica se è una keyword
	if (len == 4 && CompareStrings(pointer,"char",4))
		return(1);
	else if (len == 3 && CompareStrings(pointer,"int",3))
		return(1);
	else if (len == 5 && CompareStrings(pointer,"short",5))
		return(1);
	else if (len == 4 && CompareStrings(pointer,"long",4))
		return(1);
	else if (len == 8 && CompareStrings(pointer,"unsigned",8))
		return(1);
	else if (len == 6 && CompareStrings(pointer,"signed",6))
		return(1);
	else if (len == 5 && CompareStrings(pointer,"float",5))
		return(1);
	else if (len == 6 && CompareStrings(pointer,"double",6))
		return(1);
	else if (len == 4 && CompareStrings(pointer,"void",4))
		return(1);
	else if (len == 6 && CompareStrings(pointer,"static",6))
		return(1);
	else if (len == 5 && CompareStrings(pointer,"const",5))
		return(1);

	// cerca l'identificatore
	if (len>=identifier_max_len)
		len=identifier_max_len-1;
	memcpy(ident,pointer,len);
	ident[len]=0;

	for (i=0;i<cident_id;i++)
		if ( (cident[i].type & 0xff0000) == STRUCT &&
			!strcmp(ident,cident[i].id) )
			break;
	if (i==cident_id)
		return(0);
	else
		return(1);
}

//===================
//ReducePSICode
//
//Semplifica una porzione di codice PSI ed elimina le istruzioni che annichiliscono
//===================
int ReducePSICode (int PSI0index, int PSI1index)
{
	int		i,j,k;

	// ciclo per tutte le istruzioni indicate
	for (i=PSI0index;i<=PSI1index;i++) {

		// considera i casi in cui la costante è uguale a zero
		if (!psi_mem[i].constant) {

			// l'istruzione va semplicemente rimossa
			if (psi_mem[i].psi==EAX_CONSTANT_LEFT_SHIFT ||
				psi_mem[i].psi==EAX_CONSTANT_RIGHT_SHIFT ||
				psi_mem[i].psi==UNSIGNED_EAX_CONSTANT_RIGHT_SHIFT ||
				psi_mem[i].psi==ECX_CONSTANT_ADDITION ||
				psi_mem[i].psi==EAX_CONSTANT_ADDITION ||
				psi_mem[i].psi==EAX_CONSTANT_SUBTRACTION)
				psi_mem[i].psi=NOP;
			// sostituisce un function call senza parametri con un'istruzione più veloce
			else if (psi_mem[i].psi==CALL_FUNCTION)
				psi_mem[i].psi=CALL_FUNCTION_WITH_NOPARAMS;

		}

		// valuta se è possibile semplificare due istruzioni
		if (psi_mem[i].address.section==SECTION_TEMPDATA1) {

			// trova l'istruzione semplificabile con quella corrente
			if (i < PSI1index &&
				psi_mem[i+1].address.section==SECTION_TEMPDATA1 &&
				psi_mem[i].address.address==psi_mem[i+1].address.address)
				j=1;
			else if (i < PSI1index-1 &&
				psi_mem[i+2].address.section==SECTION_TEMPDATA1 &&
				psi_mem[i].address.address==psi_mem[i+2].address.address)
				j=2;
			else
				j=-1;

			// semplifica le istruzioni, se possibile
			if (j!=-1) {

				// considera i vari casi
				if (psi_mem[i].psi==STORE_EAX_IN_INT) {

					// controlla se è necessario proseguire
					if ( psi_mem[i+j].psi==LOAD_INT_IN_EAX ||
						psi_mem[i+j].psi==LOAD_INT_IN_ECX ||
						psi_mem[i+j].psi==LOAD_INT_IN_EDX ) {

						// controlla se il valore temporaneo è richiesto successivamente
						for (k=i+j+1;k<=PSI1index;k++) {
							if ( ( psi_mem[k].psi==LOAD_INT_IN_EAX ||
								psi_mem[k].psi==LOAD_INT_IN_ECX ||
								psi_mem[k].psi==LOAD_INT_IN_EDX ) &&
								psi_mem[k].address.section==SECTION_TEMPDATA1 &&
								psi_mem[k].address.address==psi_mem[i].address.address)
								break;
						}

						// semplifica le due istruzioni
						if (k>PSI1index) {
							if (psi_mem[i+j].psi==LOAD_INT_IN_EAX) {
								psi_mem[i].psi=NOP;
								psi_mem[i+j].psi=NOP;
							}
							else if (psi_mem[i+j].psi==LOAD_INT_IN_ECX) {
								psi_mem[i].psi=LOAD_EAX_IN_ECX;
								psi_mem[i+j].psi=NOP;
							}
							else if (psi_mem[i+j].psi==LOAD_INT_IN_EDX) {
								psi_mem[i].psi=LOAD_EAX_IN_EDX;
								psi_mem[i+j].psi=NOP;
							}
						}
						else if (j==1) {
							if (psi_mem[i+1].psi==LOAD_INT_IN_EAX)
								psi_mem[i+1].psi=NOP;
							else if (psi_mem[i+1].psi==LOAD_INT_IN_ECX)
								psi_mem[i+1].psi=LOAD_EAX_IN_ECX;
							else if (psi_mem[i+1].psi==LOAD_INT_IN_EDX)
								psi_mem[i+1].psi=LOAD_EAX_IN_EDX;
						}
					}

				}
				else if (psi_mem[i].psi==STORE_ECX_IN_INT) {

					// controlla se è necessario proseguire
					if ( psi_mem[i+j].psi==LOAD_INT_IN_EAX ||
						psi_mem[i+j].psi==LOAD_INT_IN_ECX ||
						psi_mem[i+j].psi==LOAD_INT_IN_EDX ) {

						// controlla se il valore temporaneo è richiesto successivamente
						for (k=i+j+1;k<=PSI1index;k++) {
							if ( ( psi_mem[k].psi==LOAD_INT_IN_EAX ||
								psi_mem[k].psi==LOAD_INT_IN_ECX ||
								psi_mem[k].psi==LOAD_INT_IN_EDX ) &&
								psi_mem[k].address.section==SECTION_TEMPDATA1 &&
								psi_mem[k].address.address==psi_mem[i].address.address)
								break;
						}

						// semplifica le due istruzioni
						if (k>PSI1index) {
							if (psi_mem[i+j].psi==LOAD_INT_IN_EAX) {
								psi_mem[i].psi=LOAD_ECX_IN_EAX;
								psi_mem[i+j].psi=NOP;
							}
							else if (psi_mem[i+j].psi==LOAD_INT_IN_ECX) {
								psi_mem[i].psi=NOP;
								psi_mem[i+j].psi=NOP;
							}
							else if (psi_mem[i+j].psi==LOAD_INT_IN_EDX) {
								psi_mem[i].psi=LOAD_ECX_IN_EDX;
								psi_mem[i+j].psi=NOP;
							}
						}
						else if (j==1) {
							if (psi_mem[i+1].psi==LOAD_INT_IN_EAX)
								psi_mem[i+1].psi=LOAD_ECX_IN_EAX;
							else if (psi_mem[i+1].psi==LOAD_INT_IN_ECX)
								psi_mem[i+1].psi=NOP;
							else if (psi_mem[i+1].psi==LOAD_INT_IN_EDX)
								psi_mem[i+1].psi=LOAD_ECX_IN_EDX;
						}
					}

				}
				else if (psi_mem[i].psi==STORE_ST0_IN_FLOAT) {

					// controlla se è necessario proseguire
					if ( psi_mem[i+j].psi==LOAD_FLOAT_IN_ST0 ) {

						// controlla se il valore temporaneo è richiesto successivamente
						for (k=i+j+1;k<=PSI1index;k++) {
							if ( psi_mem[k].psi==LOAD_FLOAT_IN_ST0 &&
								psi_mem[k].address.section==SECTION_TEMPDATA1 &&
								psi_mem[k].address.address==psi_mem[i].address.address)
								break;
						}

						// semplifica le due istruzioni
						if (k>PSI1index) {
							if (psi_mem[i+j].psi==LOAD_FLOAT_IN_ST0) {
								psi_mem[i].psi=NOP;
								psi_mem[i+j].psi=NOP;
							}
						}
						else if (j==1 && psi_mem[i+1].psi==LOAD_FLOAT_IN_ST0) {
							psi_mem[i].psi=STORE_ST0_IN_FLOAT_NOPOP;
							psi_mem[i+1].psi=NOP;
						}
					}

				}
				else if (psi_mem[i].psi==STORE_ST0_IN_DOUBLE) {

					// controlla se è necessario proseguire
					if ( psi_mem[i+j].psi==LOAD_DOUBLE_IN_ST0 ) {

						// controlla se il valore temporaneo è richiesto successivamente
						for (k=i+j+1;k<=PSI1index;k++) {
							if ( psi_mem[k].psi==LOAD_DOUBLE_IN_ST0 &&
								psi_mem[k].address.section==SECTION_TEMPDATA1 &&
								psi_mem[k].address.address==psi_mem[i].address.address)
								break;
						}

						// semplifica le due istruzioni
						if (k>PSI1index) {
							if (psi_mem[i+j].psi==LOAD_DOUBLE_IN_ST0) {
								psi_mem[i].psi=NOP;
								psi_mem[i+j].psi=NOP;
							}
						}
						else if (j==1 && psi_mem[i+1].psi==LOAD_DOUBLE_IN_ST0) {
							psi_mem[i].psi=STORE_ST0_IN_DOUBLE_NOPOP;
							psi_mem[i+1].psi=NOP;
						}
					}

				}

			}

		}

		// semplifica una istruzione del tipo mov reg32,reg32
		if (psi_mem[i].psi==LOAD_ECX_IN_EDX) {

			if (i>PSI0index) {
				if (psi_mem[i-1].psi==LOAD_STACK_INT_POINTER_IN_ECX) {
					psi_mem[i-1].psi=LOAD_STACK_INT_POINTER_IN_EDX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_EDXPTR_INT_IN_ECX) {
					psi_mem[i-1].psi=LOAD_EDXPTR_INT_IN_EDX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_CONSTANT_ADDRESS_IN_ECX) {
					psi_mem[i-1].psi=LOAD_CONSTANT_ADDRESS_IN_EDX;
					psi_mem[i].psi=NOP;
				}
			}
			if (i>PSI0index+1 && psi_mem[i-1].psi==NOP) {
				if (psi_mem[i-2].psi==LOAD_STACK_INT_POINTER_IN_ECX) {
					psi_mem[i-2].psi=LOAD_STACK_INT_POINTER_IN_EDX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-2].psi==LOAD_EDXPTR_INT_IN_ECX) {
					psi_mem[i-2].psi=LOAD_EDXPTR_INT_IN_EDX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-2].psi==LOAD_CONSTANT_ADDRESS_IN_ECX) {
					psi_mem[i-2].psi=LOAD_CONSTANT_ADDRESS_IN_EDX;
					psi_mem[i].psi=NOP;
				}
			}

		}
		else if (psi_mem[i].psi==LOAD_ECX_IN_EAX) {

			if (i>PSI0index) {
				if (psi_mem[i-1].psi==LOAD_STACK_INT_POINTER_IN_ECX) {
					psi_mem[i-1].psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_EDXPTR_INT_IN_ECX) {
					psi_mem[i-1].psi=LOAD_EDXPTR_INT_IN_EAX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_CONSTANT_ADDRESS_IN_ECX) {
					psi_mem[i-1].psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_mem[i].psi=NOP;
				}
			}
			if (i>PSI0index+1 && psi_mem[i-1].psi==NOP) {
				if (psi_mem[i-2].psi==LOAD_STACK_INT_POINTER_IN_ECX) {
					psi_mem[i-2].psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-2].psi==LOAD_EDXPTR_INT_IN_ECX) {
					psi_mem[i-2].psi=LOAD_EDXPTR_INT_IN_EAX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-2].psi==LOAD_CONSTANT_ADDRESS_IN_ECX) {
					psi_mem[i-2].psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_mem[i].psi=NOP;
				}
			}
			if (i < PSI1index-1 && psi_mem[i+1].psi==NOP) {
				if (psi_mem[i+2].psi==STORE_EAX_IN_STACK_INT) {
					psi_mem[i+2].psi=STORE_ECX_IN_STACK_INT;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i+2].psi==STORE_EAX_IN_INT) {
					psi_mem[i+2].psi=STORE_ECX_IN_INT;
					psi_mem[i].psi=NOP;
				}
			}

		}
		else if (psi_mem[i].psi==LOAD_EAX_IN_ECX) {

			if (i>PSI0index) {
				if (psi_mem[i-1].psi==LOAD_STACK_INT_IN_EAX) {
					psi_mem[i-1].psi=LOAD_STACK_INT_IN_ECX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_INT_IN_EAX) {
					psi_mem[i-1].psi=LOAD_INT_IN_ECX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_ECXPTR_INT_IN_EAX) {
					psi_mem[i-1].psi=LOAD_ECXPTR_INT_IN_ECX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_ST0_IN_EAX) {
					psi_mem[i-1].psi=LOAD_ST0_IN_ECX;
					psi_mem[i].psi=NOP;
				}
			}

		}
		else if (psi_mem[i].psi==LOAD_EAX_IN_EDX) {

			if (i>PSI0index) {
				if (psi_mem[i-1].psi==LOAD_STACK_INT_IN_EAX) {
					psi_mem[i-1].psi=LOAD_STACK_INT_IN_EDX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_INT_IN_EAX) {
					psi_mem[i-1].psi=LOAD_INT_IN_EDX;
					psi_mem[i].psi=NOP;
				}
				else if (psi_mem[i-1].psi==LOAD_ECXPTR_INT_IN_EAX) {
					psi_mem[i-1].psi=LOAD_ECXPTR_INT_IN_EDX;
					psi_mem[i].psi=NOP;
				}
			}

		}

		// semplifica un ARRAY_X_DIMENSION
		if (psi_mem[i].psi==ARRAY_X_DIMENSION) {

			// trova l'istruzione semplificabile con quella corrente
			if (i>PSI0index &&
				psi_mem[i-1].psi==LOAD_CONSTANT_IN_EAX)
				j=1;
			else if (i>PSI0index+1 &&
				psi_mem[i-1].psi==NOP &&
				psi_mem[i-2].psi==LOAD_CONSTANT_IN_EAX)
				j=2;
			else
				j=-1;

			// semplifica le istruzioni
			if (j!=-1) {
				psi_mem[i-j].psi=ECX_CONSTANT_ADDITION;
				psi_mem[i-j].constant*=psi_mem[i].constant;
				psi_mem[i].psi=NOP;
				i-=j;
			}

		}

		// semplifica LOAD_CONSTANT_ADDRESS_IN_ECX/LOAD_STACK_INT_POINTER_IN_ECX e ECX_CONSTANT_ADDITION
		if ( i>PSI0index &&
			psi_mem[i].psi==ECX_CONSTANT_ADDITION ) {

			// trova l'istruzione semplificabile con quella corrente
			for (j=i-1;j>=PSI0index;j--)
				if (psi_mem[j].psi!=NOP)
					break;

			// controlla se l'istruzione trovata è quella giusta
			if ( psi_mem[j].psi==LOAD_CONSTANT_ADDRESS_IN_ECX ||
				psi_mem[j].psi==LOAD_STACK_INT_POINTER_IN_ECX ) {
					psi_mem[j].address.address+=psi_mem[i].constant;
					psi_mem[i].psi=NOP;
			}

		}

		// semplifica uno store ed un load consecutivi eliminando la seconda istruzione
		if ( i != PSI1index &&
			(psi_mem[i].address.section==SECTION_DATA ||
			psi_mem[i].address.section==SECTION_STACK) &&
			psi_mem[i].address.section==psi_mem[i+1].address.section &&
			psi_mem[i].address.address==psi_mem[i+1].address.address ) {

			// stack
			if (psi_mem[i].psi==STORE_EAX_IN_STACK_CHAR &&
				( psi_mem[i+1].psi==LOAD_STACK_SIGNED_CHAR_IN_EAX ||
				psi_mem[i+1].psi==LOAD_STACK_UNSIGNED_CHAR_IN_EAX ) )
				psi_mem[i+1].psi=NOP;
			else if (psi_mem[i].psi==STORE_EAX_IN_STACK_SHORT &&
				( psi_mem[i+1].psi==LOAD_STACK_SIGNED_SHORT_IN_EAX ||
				psi_mem[i+1].psi==LOAD_STACK_UNSIGNED_SHORT_IN_EAX ) )
				psi_mem[i+1].psi=NOP;
			else if ( psi_mem[i].psi==STORE_EAX_IN_STACK_INT &&
				psi_mem[i+1].psi==LOAD_STACK_INT_IN_EAX )
				psi_mem[i+1].psi=NOP;
			else if ( psi_mem[i].psi==STORE_ST0_IN_STACK_FLOAT &&
				psi_mem[i+1].psi==LOAD_STACK_FLOAT_IN_ST0 ) {
				psi_mem[i].psi=STORE_ST0_IN_STACK_FLOAT_NOPOP;
				psi_mem[i+1].psi=NOP;
			}
			else if ( psi_mem[i].psi==STORE_ST0_IN_STACK_DOUBLE &&
				psi_mem[i+1].psi==LOAD_STACK_DOUBLE_IN_ST0 ) {
				psi_mem[i].psi=STORE_ST0_IN_STACK_DOUBLE_NOPOP;
				psi_mem[i+1].psi=NOP;
			}

			// data
			else if (psi_mem[i].psi==STORE_EAX_IN_CHAR &&
				( psi_mem[i+1].psi==LOAD_SIGNED_CHAR_IN_EAX ||
				psi_mem[i+1].psi==LOAD_UNSIGNED_CHAR_IN_EAX ) )
				psi_mem[i+1].psi=NOP;
			else if (psi_mem[i].psi==STORE_EAX_IN_SHORT &&
				( psi_mem[i+1].psi==LOAD_SIGNED_SHORT_IN_EAX ||
				psi_mem[i+1].psi==LOAD_UNSIGNED_SHORT_IN_EAX ) )
				psi_mem[i+1].psi=NOP;
			else if ( psi_mem[i].psi==STORE_EAX_IN_INT &&
				psi_mem[i+1].psi==LOAD_INT_IN_EAX )
				psi_mem[i+1].psi=NOP;
			else if ( psi_mem[i].psi==STORE_ST0_IN_FLOAT &&
				psi_mem[i+1].psi==LOAD_FLOAT_IN_ST0 ) {
				psi_mem[i].psi=STORE_ST0_IN_FLOAT_NOPOP;
				psi_mem[i+1].psi=NOP;
			}
			else if ( psi_mem[i].psi==STORE_ST0_IN_DOUBLE &&
				psi_mem[i+1].psi==LOAD_DOUBLE_IN_ST0 ) {
				psi_mem[i].psi=STORE_ST0_IN_DOUBLE_NOPOP;
				psi_mem[i+1].psi=NOP;
			}

		}

		// semplifica gli ECX_CONSTANT_ADDITION
		if (i > PSI0index &&
			psi_mem[i].psi==ECX_CONSTANT_ADDITION) {

			// controlla gli ECX_CONSTANT_ADDITION precedenti
			for (j=i-1;j>=PSI0index;j--) {
				if (psi_mem[j].psi==ECX_CONSTANT_ADDITION) {
					psi_mem[j].psi=NOP;
					psi_mem[i].constant+=psi_mem[j].constant;
				}
				else if (psi_mem[j].psi!=NOP)
					break;
			}

		}

		// semplifica un EAX_ECX_PERCONSTANT_ADDITION oppure un EAX_ECX_PERCONSTANT_SUBTRACTION
		if ( psi_mem[i].psi==EAX_ECX_PERCONSTANT_ADDITION ||
			psi_mem[i].psi==EAX_ECX_PERCONSTANT_SUBTRACTION ) {

			// controlla se è possibile semplificare
			if (i > PSI0index &&
				psi_mem[i-1].psi == LOAD_CONSTANT_IN_ECX ) {

				if ( psi_mem[i].psi==EAX_ECX_PERCONSTANT_ADDITION )
					psi_mem[i].psi=EAX_CONSTANT_ADDITION;
				else
					psi_mem[i].psi=EAX_CONSTANT_SUBTRACTION;
				psi_mem[i].constant*=psi_mem[i-1].constant;
				psi_mem[i-1].psi=NOP;

			}

		}

		// semplifica un ARRAY_X_DIMENSION o un ARRAY_FIRST_DIMENSION o un STACK_ARRAY_FIRST_DIMENSION
		if ( psi_mem[i].psi==ARRAY_X_DIMENSION ||
			psi_mem[i].psi==ARRAY_FIRST_DIMENSION ||
			psi_mem[i].psi==STACK_ARRAY_FIRST_DIMENSION ||
			psi_mem[i].psi==EAX_ECX_PERCONSTANT_ADDITION ||
			psi_mem[i].psi==EAX_ECX_PERCONSTANT_SUBTRACTION ||
			psi_mem[i].psi==EAX_ECX_OBJECT_SUBTRACTION ) {

			// valuta che tipo di semplificazione apportare al codice
			if ( psi_mem[i].constant == 1 ) {

				// sostituisce la PSI con un'istruzione più veloce
				if ( psi_mem[i].psi==ARRAY_X_DIMENSION )
					psi_mem[i].psi=ECX_EAX_ADDITION;
				else if ( psi_mem[i].psi==ARRAY_FIRST_DIMENSION )
					psi_mem[i].psi=ADDRESS_EAX_ADDITION_IN_ECX;
				else if ( psi_mem[i].psi==STACK_ARRAY_FIRST_DIMENSION )
					psi_mem[i].psi=STACK_ADDRESS_EAX_ADDITION_IN_ECX;
				else if ( psi_mem[i].psi==EAX_ECX_PERCONSTANT_ADDITION )
					psi_mem[i].psi=EAX_ECX_ADDITION;
				else if ( psi_mem[i].psi==EAX_ECX_PERCONSTANT_SUBTRACTION )
					psi_mem[i].psi=EAX_ECX_SUBTRACTION;
				else if ( psi_mem[i].psi==EAX_ECX_OBJECT_SUBTRACTION )
					psi_mem[i].psi=EAX_ECX_SUBTRACTION;

			}
			else {

				// controlla se la costante è un potenza di 2
				for (j=1;j<32;j++)
					if (psi_mem[i].constant == 1<<j) {
						if ( psi_mem[i].psi==ARRAY_X_DIMENSION )
							psi_mem[i].psi=ARRAY_X_DIMENSION_WITH_LSHIFT;
						else if ( psi_mem[i].psi==ARRAY_FIRST_DIMENSION )
							psi_mem[i].psi=ARRAY_FIRST_DIMENSION_WITH_LSHIFT;
						else if ( psi_mem[i].psi==STACK_ARRAY_FIRST_DIMENSION )
							psi_mem[i].psi=STACK_ARRAY_FIRST_DIMENSION_WITH_LSHIFT;
						else if ( psi_mem[i].psi==EAX_ECX_PERCONSTANT_ADDITION )
							psi_mem[i].psi=EAX_ECX_PERCONSTANT_ADDITION_WITH_LSHIFT;
						else if ( psi_mem[i].psi==EAX_ECX_PERCONSTANT_SUBTRACTION )
							psi_mem[i].psi=EAX_ECX_PERCONSTANT_SUBTRACTION_WITH_LSHIFT;
						else if ( psi_mem[i].psi==EAX_ECX_OBJECT_SUBTRACTION )
							psi_mem[i].psi=EAX_ECX_OBJECT_SUBTRACTION_WITH_LSHIFT;
						psi_mem[i].constant=j;
						break;
					}

			}

		}

		// cerca di semplificare passaggi inutili attraverso la memoria temporanea
		if ( i > PSI0index && 
			psi_mem[i].psi==STORE_ECX_IN_INT &&
			psi_mem[i].address.section==SECTION_TEMPDATA1 ) {

			// cerca un LOAD_STACK_INT_POINTER_IN_ECX o un LOAD_CONSTANT_ADDRESS_IN_ECX
			for (j=i-1;j>=PSI0index;j--)
				if (psi_mem[j].psi!=NOP)
					break;

			// controlla se è possibile semplificare
			if ( psi_mem[j].psi==LOAD_STACK_INT_POINTER_IN_ECX ||
				psi_mem[j].psi==LOAD_CONSTANT_ADDRESS_IN_ECX ) {

				// cerca l'istruzione da sostituire
				for (k=i+1;k<=PSI1index;k++)
					if (psi_mem[k].psi==LOAD_INT_IN_ECX &&
						psi_mem[k].address.section==SECTION_TEMPDATA1 &&
						psi_mem[k].address.address==psi_mem[i].address.address) {

						psi_mem[k]=psi_mem[j];
						psi_mem[i].psi=NOP;
						psi_mem[j].psi=NOP;

						break;

					}
					else if (psi_mem[k].psi==LOAD_INT_IN_EDX &&
						psi_mem[k].address.section==SECTION_TEMPDATA1 &&
						psi_mem[k].address.address==psi_mem[i].address.address) {

						psi_mem[k]=psi_mem[j];
						if ( psi_mem[k].psi==LOAD_STACK_INT_POINTER_IN_ECX )
							psi_mem[k].psi=LOAD_STACK_INT_POINTER_IN_EDX;
						if ( psi_mem[k].psi==LOAD_CONSTANT_ADDRESS_IN_ECX )
							psi_mem[k].psi=LOAD_CONSTANT_ADDRESS_IN_EDX;
						psi_mem[i].psi=NOP;
						psi_mem[j].psi=NOP;

						break;

					}

			}

		}

		// cerca di semplificare passaggi inutili attraverso la memoria temporanea
		if ( i > PSI0index &&
			psi_mem[i].psi==STORE_EAX_IN_INT &&
			psi_mem[i].address.section==SECTION_TEMPDATA1 ) {

			// cerca un LOAD_STACK_INT_POINTER_IN_EAX o un LOAD_CONSTANT_ADDRESS_IN_EAX
			for (j=i-1;j>=PSI0index;j--)
				if (psi_mem[j].psi!=NOP)
					break;

			// controlla se è possibile semplificare
			if ( psi_mem[j].psi==LOAD_STACK_INT_POINTER_IN_EAX ||
				psi_mem[j].psi==LOAD_CONSTANT_ADDRESS_IN_EAX ) {

				// cerca l'istruzione da sostituire
				for (k=i+1;k<=PSI1index;k++)
					if (psi_mem[k].psi==LOAD_INT_IN_ECX &&
						psi_mem[k].address.section==SECTION_TEMPDATA1 &&
						psi_mem[k].address.address==psi_mem[i].address.address) {

						psi_mem[k]=psi_mem[j];
						if ( psi_mem[k].psi==LOAD_STACK_INT_POINTER_IN_EAX )
							psi_mem[k].psi=LOAD_STACK_INT_POINTER_IN_ECX;
						if ( psi_mem[k].psi==LOAD_CONSTANT_ADDRESS_IN_EAX )
							psi_mem[k].psi=LOAD_CONSTANT_ADDRESS_IN_ECX;
						psi_mem[i].psi=NOP;
						psi_mem[j].psi=NOP;

						break;

					}

			}

		}

		// riduce le istruzioni di operazioni con costanti intere
		if (i > PSI0index && (
			psi_mem[i].psi == EAX_ECX_MULTIPLICATION ||
			psi_mem[i].psi == EAX_ECX_DIVISION ||
			psi_mem[i].psi == EAX_ECX_REMAINDER ||
			psi_mem[i].psi == EAX_ECX_ADDITION ||
			psi_mem[i].psi == EAX_ECX_SUBTRACTION ||
			psi_mem[i].psi == EAX_ECX_BITWISE_AND ||
			psi_mem[i].psi == EAX_ECX_BITWISE_EXCLUSIVE_OR ||
			psi_mem[i].psi == EAX_ECX_BITWISE_OR ) ) {

			// controlla se il secondo operando è una costante
			if (psi_mem[i-1].psi == LOAD_CONSTANT_IN_ECX) {
				if ( psi_mem[i].psi == EAX_ECX_MULTIPLICATION )
					psi_mem[i].psi = EAX_CONSTANT_MULTIPLICATION;
				else if ( psi_mem[i].psi == EAX_ECX_DIVISION )
					psi_mem[i].psi = EAX_CONSTANT_DIVISION;
				else if ( psi_mem[i].psi == EAX_ECX_REMAINDER )
					psi_mem[i].psi = EAX_CONSTANT_REMAINDER;
				else if ( psi_mem[i].psi == EAX_ECX_ADDITION )
					psi_mem[i].psi = EAX_CONSTANT_ADDITION;
				else if ( psi_mem[i].psi == EAX_ECX_SUBTRACTION )
					psi_mem[i].psi = EAX_CONSTANT_SUBTRACTION;
				else if ( psi_mem[i].psi == EAX_ECX_BITWISE_AND )
					psi_mem[i].psi = EAX_CONSTANT_BITWISE_AND;
				else if ( psi_mem[i].psi == EAX_ECX_BITWISE_EXCLUSIVE_OR )
					psi_mem[i].psi = EAX_CONSTANT_BITWISE_EXCLUSIVE_OR;
				else if ( psi_mem[i].psi == EAX_ECX_BITWISE_OR )
					psi_mem[i].psi = EAX_CONSTANT_BITWISE_OR;
				psi_mem[i].constant=psi_mem[i-1].constant;
				psi_mem[i-1].psi=NOP;
			}

		}

// ******************************************************************************************
// ** Questo codice provoca in alcune situzioni la rimozione di LOAD_STACK_INT_POINTER_IN_ECX
// ** quando seguono istruzioni che richiedono ecx. Considerato che l'impatto di riduzione
// ** per questa routine è limitato anche in situazioni di corretto funzionamento, ho deciso
// ** di rimuoverla.
// **																Vito Plantamura 26/5/1999
// ******************************************************************************************
/*
		// elimina coppie di istruzioni del tipo LOAD_STACK_INT_POINTER_IN_ECX + LOAD_ECXPTR_INT_IN_EAX
		if ( psi_mem[i].psi == LOAD_ECXPTR_INT_IN_EAX ||
			psi_mem[i].psi == LOAD_ECXPTR_INT_IN_ECX ) {

			// cerca l'istruzione immediatamente precedente
			for (j=i-1;j>=PSI0index;j--)
				if (psi_mem[j].psi!=NOP)
					break;

			if (psi_mem[j].psi == LOAD_STACK_INT_POINTER_IN_ECX) {
				if (psi_mem[i].psi == LOAD_ECXPTR_INT_IN_EAX)
					psi_mem[i].psi = LOAD_STACK_INT_IN_EAX;
				else if (psi_mem[i].psi == LOAD_ECXPTR_INT_IN_ECX)
					psi_mem[i].psi = LOAD_STACK_INT_IN_ECX;
				psi_mem[i].address=psi_mem[j].address;
				psi_mem[j].psi=NOP;
			}
			else if (psi_mem[j].psi == LOAD_CONSTANT_ADDRESS_IN_ECX) {
				if (psi_mem[i].psi == LOAD_ECXPTR_INT_IN_EAX)
					psi_mem[i].psi = LOAD_INT_IN_EAX;
				else if (psi_mem[i].psi == LOAD_ECXPTR_INT_IN_ECX)
					psi_mem[i].psi = LOAD_INT_IN_ECX;
				psi_mem[i].address=psi_mem[j].address;
				psi_mem[j].psi=NOP;
			}

		}
*/

		// elimina coppie di istruzioni del tipo STORE_EAX_IN_INT e PUSH_INT
		if ( i > PSI0index &&
			psi_mem[i].psi == PUSH_INT &&
			psi_mem[i-1].psi == STORE_EAX_IN_INT &&
			psi_mem[i].address.section == SECTION_TEMPDATA1 &&
			psi_mem[i-1].address.section == SECTION_TEMPDATA1 &&
			psi_mem[i].address.address == psi_mem[i-1].address.address ) {
			psi_mem[i-1].psi=NOP;
			psi_mem[i].psi=PUSH_EAX;
		}

		// valuta se è possibile risolvere direttamente due operazioni con costanti
		if ( i > PSI0index && (
			psi_mem[i].psi==EAX_CONSTANT_ADDITION ||
			psi_mem[i].psi==EAX_CONSTANT_SUBTRACTION ) ) {

			// cerca la prima istruzione che precede
			for (j=i-1;j>=PSI0index;j--)
				if (psi_mem[j].psi!=NOP)
					break;

			// controlla se è possibile semplificare
			if ( psi_mem[j].psi==EAX_CONSTANT_ADDITION ||
				psi_mem[j].psi==EAX_CONSTANT_SUBTRACTION ) {

				// imposta la nuova costante della prima istruzione
				if ( ( psi_mem[j].psi==EAX_CONSTANT_ADDITION && psi_mem[i].psi==EAX_CONSTANT_ADDITION ) ||
					( psi_mem[j].psi==EAX_CONSTANT_SUBTRACTION && psi_mem[i].psi==EAX_CONSTANT_SUBTRACTION ) )
					psi_mem[j].constant+=psi_mem[i].constant;
				else
					psi_mem[j].constant-=psi_mem[i].constant;

				// controlla che la prima istruzione sia ok
				if ( psi_mem[j].constant < 0 ) {
					if ( psi_mem[j].psi == EAX_CONSTANT_ADDITION )
						psi_mem[j].psi=EAX_CONSTANT_SUBTRACTION;
					else if ( psi_mem[j].psi == EAX_CONSTANT_SUBTRACTION )
						psi_mem[j].psi=EAX_CONSTANT_ADDITION;
					psi_mem[j].constant=-psi_mem[j].constant;
				}

				// elimina dal codice la seconda istruzione
				psi_mem[i].psi=NOP;

			}

		}

	}

	// controlla l'ultima istruzione del blocco
	if ( ( psi_mem[PSI1index].psi == STORE_EAX_IN_CHAR ||
		psi_mem[PSI1index].psi == STORE_EAX_IN_SHORT ||
		psi_mem[PSI1index].psi == STORE_EAX_IN_INT ||
		psi_mem[PSI1index].psi == STORE_ST0_IN_FLOAT ||
		psi_mem[PSI1index].psi == STORE_ST0_IN_DOUBLE ) &&
		psi_mem[PSI1index].address.section == SECTION_TEMPDATA1 )
		psi_mem[PSI1index].psi=NOP;

	// ritorna
	return(0);
}

//==================
//InitializePreprocessorStack
//
//Carica il file da compilare ed inizializza lo stack del preprocessore
//==================
int InitializePreprocessorStack (char *file)
{
	char	buffer[1024];

	// carica il file
	strcpy(buffer,source_directory);
	strcat(buffer,file);
	preproc_stack_ptr->id=PREPROC_INCLUDE;
	preproc_stack_ptr->stream =
		current_file_stream =
		MEMFILE_NAME(fopen) ( buffer , "rt" );
	strcpy(preproc_stack_ptr->filename,buffer);
	preproc_stack_ptr->line=current_line=0;
	preproc_stack_ptr->condition=1;
	preproc_stack_ptr++;
	strcpy(current_file,buffer);

	// controlla che non ci siano errori
	if ( !current_file_stream ) {
		CompilerErrorOrWarning(186);
		return(1);
	}

	// resetta preprocessor_conditional_level
	preprocessor_conditional_level=0;

	// imposta used_streams
	used_streams=1;

	// ritorna
	return(0);
}

//====================
//ReadLineFromSource
//
//Legge una linea di codice dal file attualmente in compilazione
//====================
int ReadLineFromSource (char *buffer)
{
	int					ret;
	preproc_item_t		*pointer;
	char				*ptr,*char_ptr;
	char				ident[1024];
	int					i,j,k;
	number_t			result;
	int					len;
	int					ident_len;
	char				*auxptr;
	char				*depend_ptr;

	// legge la linea di codice
	ptr=MEMFILE_NAME(fgets)(buffer,max_code_line_lenght,current_file_stream);

	// controlla che sia tutto ok
	if ( !ptr && MEMFILE_NAME(feof)(current_file_stream) ) {

		// decrementa lo stack pointer del preprocessore
		preproc_stack_ptr--;

		// controlla preprocessor_conditional_level
		if (preprocessor_conditional_level) {
			PreprocStackFcloseAll();
			if (preproc_stack_ptr->id == PREPROC_IF)
				CompilerErrorOrWarning(188);
			else if (preproc_stack_ptr->id == PREPROC_IFDEF)
				CompilerErrorOrWarning(189);
			else if (preproc_stack_ptr->id == PREPROC_IFNDEF)
				CompilerErrorOrWarning(190);
			else if (preproc_stack_ptr->id == PREPROC_ELSE)
				CompilerErrorOrWarning(191);
			return(READLINE_FATAL);
		}

		// controlla se questo file prima di essere chiuso non abbia statement incompleti
		if (statement_incomplete) {
			CompilerErrorOrWarning(221);
			statement_incomplete=NULL;
		}

		// è stata raggiunta la fine del file
		if ( preproc_stack_ptr - preproc_stack ) {

			// chiude il file e controlla che lo stack sia ok
			MEMFILE_NAME(fclose)(current_file_stream);
			used_streams--;
			if (preproc_stack_ptr->id == PREPROC_INCLUDE) {

				for (pointer=preproc_stack_ptr-1;pointer>=preproc_stack;pointer--)
					if (pointer->id == PREPROC_INCLUDE)
						break;

				current_file_stream=pointer->stream;
				strcpy(current_file,pointer->filename);
				current_line=preproc_stack_ptr->line;

				return(READLINE_REPEAT);
			}
			else {

				PreprocStackFcloseAll();
				if (preproc_stack_ptr->id == PREPROC_IF)
					CompilerErrorOrWarning(188);
				else if (preproc_stack_ptr->id == PREPROC_IFDEF)
					CompilerErrorOrWarning(189);
				else if (preproc_stack_ptr->id == PREPROC_IFNDEF)
					CompilerErrorOrWarning(190);
				else if (preproc_stack_ptr->id == PREPROC_ELSE)
					CompilerErrorOrWarning(191);
				return(READLINE_FATAL);

			}

		}
		else {
			// chiude il file sorgente e comunica che la lettura è terminata
			MEMFILE_NAME(fclose)(current_file_stream);
			used_streams--;
			return(READLINE_END);
		}

	}
	else if ( !ptr && MEMFILE_NAME(ferror)(current_file_stream) ) {
		PreprocStackFcloseAll();
		CompilerErrorOrWarning(187);
		return(READLINE_FATAL);
	}
	else {
		current_line++;

		// elimina il carattere 0xa alla fine della stringa rendendola ASCIIZ
		for (i=0;i<max_code_line_lenght;i++) {
			if (buffer[i]==0xa) {
				buffer[i]=0;
				break;
			}
			else if (!buffer[i])
				break;
		}

		// controlla se c'è un commento alla destra della linea e se c'è lo esclude
		k=-1;
		for (i=0,j=strlen(buffer);i<j;i++) {
			if (buffer[i]=='\\' && buffer[i+1]=='\\') {
				k=i++ +1;
				continue;
			}
			if (buffer[i]=='"') {
				if (!i || buffer[i-1]!='\\' || k==i-1) {
					for (i++;i<j;i++) {
						if (buffer[i]=='\\' && buffer[i+1]=='\\') {
							k=i++ +1;
							continue;
						}
						if (buffer[i]=='"' &&
							(buffer[i-1]!='\\' || k==i-1))
							break;
					}
					if (i==j) {
						CompilerErrorOrWarning(212);
						return(READLINE_ERROR);
					}
				}
			}
			if (buffer[i]=='\'') {
				if (!i || buffer[i-1]!='\\' || k==i-1) {
					for (i++;i<j;i++) {
						if (buffer[i]=='\\' && buffer[i+1]=='\\') {
							k=i++ +1;
							continue;
						}
						if (buffer[i]=='\'' &&
							(buffer[i-1]!='\\' || k==i-1))
							break;
					}
					if (i==j) {
						CompilerErrorOrWarning(212);
						return(READLINE_ERROR);
					}
				}
			}
			if (buffer[i]=='/' && buffer[i+1]=='/') {
				buffer[i]=0;
				break;
			}
		}

		// elimina gli spazi alla destra della linea
		j=strlen(buffer);
		for (i=j-1;i>=0;i--)
			if ( buffer[i] != ' ' &&
				buffer[i] != '\t' )
				break;
		buffer[i+1]=0;

		// controlla che la linea non sia nulla
		if (!strlen(buffer))
			return(READLINE_REPEAT);

		// controlla se la linea è una direttiva per il preprocessore
		for (ptr=buffer;*ptr;ptr++)
			if ( *ptr!=' ' &&
				*ptr!='\t' )
				break;
		if (*ptr == '#') {

			ptr++;
			for (;*ptr;ptr++)
				if ( *ptr!=' ' &&
					*ptr!='\t' )
					break;

			// controlla che la memoria sia ok
			if (CompilerMemoryCheckPoint()) {
				PreprocStackFcloseAll();
				return(READLINE_FATAL);
			}

			// imposta ident_len
			ident_len=IsIdentfString(ptr);

			// la direttiva è una #define
			if ( CompareStrings(ptr,"define",6) && ident_len==6 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition )
					return(READLINE_REPEAT);

				ptr+=6;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				if (!*ptr) {
					CompilerErrorOrWarning(192);
					return(READLINE_ERROR);
				}
				else if (ret=IsIdentfString(ptr)) {

					// controlla che non sia un numero
					if (*ptr>='0' && *ptr<='9') {
						CompilerErrorOrWarning(192);
						return(READLINE_ERROR);
					}

					// controlla la lunghezza dell'identificatore
					if (ret>=identifier_max_len) {
						i=identifier_max_len-1;
						CompilerErrorOrWarning(1);
					}
					else
						i=ret;
					memcpy(ident,ptr,i);
					ident[i]=0;

					// controlla se esiste già una macro con lo stesso nome
					for (i=0;i<cident_id;i++)
						if ( cident[i].type == MACRO &&
							!strcmp(ident,cident[i].id) )
							break;

					// aggiorna il puntatore ed elimina i white spaces
					ptr+=ret;
					for (;*ptr;ptr++)
						if ( *ptr!=' ' &&
							*ptr!='\t' )
							break;

					// considera i due casi: se è una ridefinizione e se non lo è
					if (i==cident_id) {

						// controlla che l'identificatore non sia una reserved word
						len=strlen(ident);
						for (i=0;reswords[i].ptr!=NULL;i++)
							if (reswords[i].len == len &&
								CompareStrings(ident,reswords[i].ptr,reswords[i].len)) {
								CompilerErrorOrWarning(174);
								return(READLINE_ERROR);
							}

						// imposta i dati sulla macro
						cident[cident_id].type=MACRO;
						strcpy(cident[cident_id].id,ident);
						cident[cident_id].pointer=cident_aux2_ptr;
						strcpy(cident_aux2_ptr,ptr);
						(char*)cident_aux2_ptr+=strlen(ptr)+1;

						// resetta gli altri campi
						cident[cident_id].address=0;
						cident[cident_id].fields=0;
						cident[cident_id].indirection=0;
						cident[cident_id].dimensions=0;
						cident[cident_id].const_flag=0;
						cident[cident_id].static_flag=0;
						cident[cident_id].initializer=NULL;
						cident[cident_id].struct_type=NULL;
						cident[cident_id].array_stptr=0;
						cident[cident_id].ellipses_arguments=0;
						cident[cident_id].initializer_line=0;

						// incrementa cident_id
						cident_id++;

					}
					else {

						// controlla se c'è bisogno di ridefinire
						if (strcmp(ptr,cident[i].pointer)) {

							CompilerErrorOrWarning(193);

							cident[i].pointer=cident_aux2_ptr;
							strcpy(cident_aux2_ptr,ptr);
							(char*)cident_aux2_ptr+=strlen(ptr)+1;

						}

					}

				}
				else {
					CompilerErrorOrWarning(192);
					return(READLINE_ERROR);
				}

			}
			// la direttiva è un #undef
			else if ( CompareStrings(ptr,"undef",5) && ident_len==5 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition )
					return(READLINE_REPEAT);

				ptr+=5;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				if (!*ptr) {
					CompilerErrorOrWarning(194);
					return(READLINE_ERROR);
				}
				else if (ret=IsIdentfString(ptr)) {

					// controlla che non sia un numero
					if (*ptr>='0' && *ptr<='9') {
						CompilerErrorOrWarning(194);
						return(READLINE_ERROR);
					}

					// controlla la lunghezza dell'identificatore
					if (ret>=identifier_max_len) {
						i=identifier_max_len-1;
						CompilerErrorOrWarning(1);
					}
					else
						i=ret;
					memcpy(ident,ptr,i);
					ident[i]=0;

					// controlla se esiste già una macro con lo stesso nome
					for (i=0;i<cident_id;i++)
						if ( cident[i].type == MACRO &&
							!strcmp(ident,cident[i].id) )
							break;

					// se la macro non esiste, visualizza un warning
					if (i==cident_id)
						CompilerErrorOrWarning(195);
					else {
						cident[i].type=UNUSED;
						cident[i].pointer=NULL;
					}

				}
				else {
					CompilerErrorOrWarning(194);
					return(READLINE_ERROR);
				}
			}
			// la direttiva è un #include
			else if ( CompareStrings(ptr,"include",7) && ident_len==7 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition )
					return(READLINE_REPEAT);

				ptr+=7;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				if (!*ptr) {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(196);
					return(READLINE_FATAL);
				}
				else {

					// associa i " " o i < >
					if (*ptr=='"') {
						for (char_ptr=ptr+1;*char_ptr;char_ptr++)
							if (*char_ptr=='"')
								break;
					}
					else if (*ptr=='<') {
						for (char_ptr=ptr+1;*char_ptr;char_ptr++)
							if (*char_ptr=='>')
								break;
					}
					else {
						PreprocStackFcloseAll();
						CompilerErrorOrWarning(196);
						return(READLINE_FATAL);
					}

					// controlla che la sintassi sia ok
					if (!*char_ptr ||
						char_ptr-ptr == 1) {
						PreprocStackFcloseAll();
						CompilerErrorOrWarning(196);
						return(READLINE_FATAL);
					}

					// compone il nome del file da includere
					if (*ptr=='"') {
						for ( auxptr=current_file+strlen(current_file)-1;
							auxptr >= current_file;
							auxptr-- )
								if ( *auxptr == '\\' ||
									*auxptr == ':' )
									break;
						if ( *auxptr == '\\' || *auxptr == ':' ) {
							memcpy(ident,current_file,auxptr-current_file+1);
							ident[auxptr-current_file+1]=0;
						}
						else
							*ident=0;
					}
					else
						strcpy(ident,headers_directory);
					i=strlen(ident);
					memcpy(ident+i,ptr+1,char_ptr-ptr-1);
					ident[i+char_ptr-ptr-1]=0;

					// carica il file
					preproc_stack_ptr->id=PREPROC_INCLUDE;
					preproc_stack_ptr->stream =
						current_file_stream =
						MEMFILE_NAME(fopen) ( ident , "rt" );
					strcpy(preproc_stack_ptr->filename,ident);
					preproc_stack_ptr->line=current_line;
					preproc_stack_ptr->condition=1;
					preproc_stack_ptr++;

					// controlla che non ci siano errori
					if ( !current_file_stream ) {
						PreprocStackFcloseAll();
						CompilerErrorOrWarning(198);
						return(READLINE_FATAL);
					}

					// *** aggiorna dependencies_mem ***

					depend_ptr=dependencies_mem;
					while ( strlen(depend_ptr) != 0 ) {

						char		depend_filename[1024];
						char		*depend_next=MACRO_CRTFN_NAME(strchr)(depend_ptr,'|');
						int			len=depend_next-depend_ptr;

						memcpy(depend_filename,depend_ptr,len);
						depend_filename[len]=0;

						if ( !MACRO_CRTFN_NAME(stricmp)(depend_filename,ident) )
							break;

						depend_ptr=depend_next+1;
					}

					if ( strlen(depend_ptr) == 0 ) {
						strcat(dependencies_mem,ident);
						strcat(dependencies_mem,"|");
					}

					// controlla e incrementa used_streams
					if (++used_streams==99) {
						PreprocStackFcloseAll();
						CompilerErrorOrWarning(209);
						return(READLINE_FATAL);
					}

					// imposta le informazioni sul nuovo file che si sta leggendo
					strcpy(current_file,ident);
					current_line=0;

					// controlla che non ci siano caratteri dopo il > o il "
					ptr=char_ptr+1;
					for (;*ptr;ptr++)
						if ( *ptr!=' ' &&
							*ptr!='\t' )
							break;
					if (*ptr)
						CompilerErrorOrWarning(197);

				}

			}
			// la direttiva è un #ifdef
			else if ( CompareStrings(ptr,"ifdef",5) && ident_len==5 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition ) {
					preprocessor_conditional_level++;
					return(READLINE_REPEAT);
				}

				ptr+=5;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				if (!*ptr) {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(199);
					return(READLINE_FATAL);
				}
				else if (ret=IsIdentfString(ptr)) {

					// controlla che non sia un numero
					if (*ptr>='0' && *ptr<='9') {
						CompilerErrorOrWarning(199);
						return(READLINE_ERROR);
					}

					// controlla la lunghezza dell'identificatore
					if (ret>=identifier_max_len) {
						i=identifier_max_len-1;
						CompilerErrorOrWarning(1);
					}
					else
						i=ret;
					memcpy(ident,ptr,i);
					ident[i]=0;

					// controlla se esiste già una macro con lo stesso nome
					for (i=0;i<cident_id;i++)
						if ( cident[i].type == MACRO &&
							!strcmp(ident,cident[i].id) )
							break;

					// aggiorna il puntatore ed elimina i white spaces
					ptr+=ret;
					for (;*ptr;ptr++)
						if ( *ptr!=' ' &&
							*ptr!='\t' )
							break;

					// controlla se la macro esiste
					preproc_stack_ptr->id=PREPROC_IFDEF;
					preproc_stack_ptr->stream=NULL;
					*preproc_stack_ptr->filename=0;
					preproc_stack_ptr->line=0;
					if (i==cident_id)
						preproc_stack_ptr->condition=0;
					else
						preproc_stack_ptr->condition=1;
					preproc_stack_ptr++;

					// controlla che non ci siano caratteri dopo l'identificatore
					if (*ptr)
						CompilerErrorOrWarning(200);

				}
				else {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(199);
					return(READLINE_FATAL);
				}

			}
			// la direttiva è un #ifndef
			else if ( CompareStrings(ptr,"ifndef",6) && ident_len==6 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition ) {
					preprocessor_conditional_level++;
					return(READLINE_REPEAT);
				}

				ptr+=6;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				if (!*ptr) {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(201);
					return(READLINE_FATAL);
				}
				else if (ret=IsIdentfString(ptr)) {

					// controlla che non sia un numero
					if (*ptr>='0' && *ptr<='9') {
						CompilerErrorOrWarning(201);
						return(READLINE_ERROR);
					}

					// controlla la lunghezza dell'identificatore
					if (ret>=identifier_max_len) {
						i=identifier_max_len-1;
						CompilerErrorOrWarning(1);
					}
					else
						i=ret;
					memcpy(ident,ptr,i);
					ident[i]=0;

					// controlla se esiste già una macro con lo stesso nome
					for (i=0;i<cident_id;i++)
						if ( cident[i].type == MACRO &&
							!strcmp(ident,cident[i].id) )
							break;

					// aggiorna il puntatore ed elimina i white spaces
					ptr+=ret;
					for (;*ptr;ptr++)
						if ( *ptr!=' ' &&
							*ptr!='\t' )
							break;

					// controlla se la macro esiste
					preproc_stack_ptr->id=PREPROC_IFNDEF;
					preproc_stack_ptr->stream=NULL;
					*preproc_stack_ptr->filename=0;
					preproc_stack_ptr->line=0;
					if (i==cident_id)
						preproc_stack_ptr->condition=1;
					else
						preproc_stack_ptr->condition=0;
					preproc_stack_ptr++;

					// controlla che non ci siano caratteri dopo l'identificatore
					if (*ptr)
						CompilerErrorOrWarning(202);

				}
				else {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(201);
					return(READLINE_FATAL);
				}

			}
			// la direttiva è un #else
			else if ( CompareStrings(ptr,"else",4) && ident_len==4 ) {

				// controlla se accettare la direttiva
				if ( preprocessor_conditional_level )
					return(READLINE_REPEAT);

				ptr+=4;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				// controlla se l'#else è regolare
				if ( (preproc_stack_ptr-1)->id != PREPROC_IF &&
					(preproc_stack_ptr-1)->id != PREPROC_IFDEF &&
					(preproc_stack_ptr-1)->id != PREPROC_IFNDEF ) {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(203);
					return(READLINE_FATAL);
				}

				// imposta lo stack del preprocessore
				(preproc_stack_ptr-1)->id = PREPROC_ELSE;
				(preproc_stack_ptr-1)->condition = !(preproc_stack_ptr-1)->condition;

				// controlla che non ci siano caratteri dopo l'#else
				if (*ptr)
					CompilerErrorOrWarning(204);

			}
			// la direttiva è un #endif
			else if ( CompareStrings(ptr,"endif",5) && ident_len==5 ) {

				// controlla se accettare la direttiva
				if ( preprocessor_conditional_level ) {
					preprocessor_conditional_level--;
					return(READLINE_REPEAT);
				}

				ptr+=5;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				// controlla se l'#endif è regolare
				if ( (preproc_stack_ptr-1)->id != PREPROC_IF &&
					(preproc_stack_ptr-1)->id != PREPROC_IFDEF &&
					(preproc_stack_ptr-1)->id != PREPROC_IFNDEF &&
					(preproc_stack_ptr-1)->id != PREPROC_ELSE) {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(205);
					return(READLINE_FATAL);
				}

				// decrementa lo stack pointer
				preproc_stack_ptr--;

				// controlla che non ci siano caratteri dopo l'#else
				if (*ptr)
					CompilerErrorOrWarning(206);

			}
			// la direttiva è un #if
			else if ( CompareStrings(ptr,"if",2) && ident_len==2 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition ) {
					preprocessor_conditional_level++;
					return(READLINE_REPEAT);
				}

				ptr+=2;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				// controlla che ci sia un argomento
				if (!*ptr) {
					PreprocStackFcloseAll();
					CompilerErrorOrWarning(207);
					return(READLINE_FATAL);
				}

				// interpreta l'argomento dell'#if
				if (ResolveConstantExpression(&result,ptr)) {
					PreprocStackFcloseAll();
					fatal_error=1;
					return(READLINE_FATAL);
				}
				if (result.type==CHAR)
					i=!*(char*)result.number;
				else if (result.type==UNSIGNED_CHAR)
					i=!*(unsigned char*)result.number;
				else if (result.type==SHORT)
					i=!*(short*)result.number;
				else if (result.type==UNSIGNED_SHORT)
					i=!*(unsigned short*)result.number;
				else if (result.type==INT)
					i=!*(int*)result.number;
				else if (result.type==UNSIGNED_INT)
					i=!*(unsigned int*)result.number;
				else if (result.type==FLOAT)
					i=!*(float*)result.number;
				else if (result.type==DOUBLE)
					i=!*(double*)result.number;
				else if (result.type==LONG_DOUBLE)
					i=!*(long double*)result.number;

				// imposta lo stack
				preproc_stack_ptr->id=PREPROC_IF;
				preproc_stack_ptr->stream=NULL;
				*preproc_stack_ptr->filename=0;
				preproc_stack_ptr->line=0;
				if (i)
					preproc_stack_ptr->condition=0;
				else
					preproc_stack_ptr->condition=1;
				preproc_stack_ptr++;

			}
			// la direttiva è un #align
			else if ( CompareStrings(ptr,"align",5) && ident_len==5 ) {

				// controlla se accettare la direttiva
				if ( !(preproc_stack_ptr-1)->condition )
					return(READLINE_REPEAT);

				ptr+=5;
				for (;*ptr;ptr++)
					if ( *ptr!=' ' &&
						*ptr!='\t' )
						break;

				if (!*ptr) {
					CompilerErrorOrWarning(238);
					return(READLINE_ERROR);
				}
				else {

					// valuta la grandezza dell'allineamento
					if ( CompareStrings(ptr,"1",1) && (ptr[1]==' ' || ptr[1]=='\t' || !ptr[1]) )
						struct_alignment=1;
					else if ( CompareStrings(ptr,"2",1) && (ptr[1]==' ' || ptr[1]=='\t' || !ptr[1]) )
						struct_alignment=2;
					else if ( CompareStrings(ptr,"4",1) && (ptr[1]==' ' || ptr[1]=='\t' || !ptr[1]) )
						struct_alignment=4;
					else if ( CompareStrings(ptr,"8",1) && (ptr[1]==' ' || ptr[1]=='\t' || !ptr[1]) )
						struct_alignment=8;
					else if ( CompareStrings(ptr,"16",2) && (ptr[2]==' ' || ptr[2]=='\t' || !ptr[2]) )
						struct_alignment=16;
					else if ( CompareStrings(ptr,"default",7) && (ptr[7]==' ' || ptr[7]=='\t' || !ptr[7]) )
						struct_alignment=struct_alignment_default;
					else {
						CompilerErrorOrWarning(239);
						return(READLINE_ERROR);
					}

				}

			}
			// la direttiva non è stata riconosciuta dal compilatore
			else {
				PreprocStackFcloseAll();
				CompilerErrorOrWarning(208);
				return(READLINE_FATAL);
			}

			// ritorna
			return(READLINE_REPEAT);

		}
		else if ( !(preproc_stack_ptr-1)->condition )
			return(READLINE_REPEAT);

	}

	// ritorna
	return(READLINE_OK);
}

//====================
//PreprocStackFcloseAll
//
//Chiude tutti i file aperti per la compilazione (solo in caso di errore fatale)
//====================
void PreprocStackFcloseAll (void)
{
	preproc_item_t		*ptr;

	// chiude tutti i file aperti
	for (ptr=preproc_stack_ptr-1;ptr>=preproc_stack;ptr--)
		if ( ptr->id == PREPROC_INCLUDE &&
			ptr->stream )
			MEMFILE_NAME(fclose)(ptr->stream);

	// resetta used_streams
	used_streams=0;

	// ritorna
	return;
}

//===================
//LinkOperations
//
//Nello stack degli operatori associa le operazioni collegate
//===================
int LinkOperations (operator_t *stack, int *list, int index)
{
	int		*l,*m;

	// immette nella lista l'indice passato alla funzione
	l=list;
	*l=index;
	l++;

	// controlla tutti gli altri operatori con priorità più alta
	for (index++;stack[index].id != EXPRESSION_END;index++) {

		// controlla se sono collegate ricercando negli operatori già selezionati
		for (m=list;m<l;m++) {
			if ( stack[index].tok_id == stack[*m].left_tok ||
				stack[index].tok_id == stack[*m].right_tok ||
				stack[index].tok_id == stack[*m].third_tok ) {
				*l=index;
				l++;
				break;
			}
		}

	}

	// l'ultimo item della lista è -1 per indicarne la fine
	*l=-1;

	// ritorna
	return(l-list);
}

//=====================
//ReorderStackForLogicalAndConditionalOpAndComma
//
//Riordina lo stack degli operatori per le istruzioni: && || ?: ,
//=====================
void ReorderStackForLogicalAndConditionalOpAndComma (operator_t *stack, token_t *tokens)
{
	int				oplist[max_num_of_linked_operators];
	operator_t		*stack0=auxmem;
	int				i,j,k,l;
	int				num_of_linked_ops_0;
	int				num_of_linked_ops_1;

	// attraversa lo stack alla ricerca degli operatori && || ?:
	for (i=0;stack[i].id != EXPRESSION_END;i++) {

		// AND logico e OR logico
		if ( stack[i].id == LOGICAL_AND ||
			stack[i].id == LOGICAL_OR ) {

			oplist[ 0 ] = -1; // VPCICE PATCH // // 20MAG2004 //

			// controlla se è davvero necessario riordinare lo stack
			if ( ( tokens[stack[i].right_tok].id & 0xff00 ) != 0xff00 ) {

				// collega gli operatori alla destra dell'operatore
				for (j=i+1;stack[j].id!=EXPRESSION_END;j++)
					if (stack[j].tok_id == stack[i].right_tok)
						break;
				num_of_linked_ops_0=LinkOperations(stack,oplist,j);

				// copia la prima parte dello stack (incluso l'operatore && o ||)
				memcpy(stack0,stack,(i+1)*sizeof(operator_t));

				// imposta un nuovo operatore fittizio
				if (stack[i].id == LOGICAL_AND)
					stack0[i].id=LOGICAL_AND_STEP1;
				else
					stack0[i].id=LOGICAL_OR_STEP1;
				stack0[i].third_tok=stack[i].right_tok;
				stack0[i].left_tok=-1;
				stack0[i].right_tok=-1;

				// copia gli operatori collegati
				for (j=0;j<num_of_linked_ops_0;j++)
					stack0[i+j+1]=stack[oplist[j]];

				// imposta un secondo operatore fittizio
				j+=i+1;
				stack0[j]=stack[i];
				if (stack[i].id == LOGICAL_AND)
					stack0[j].id=LOGICAL_AND_STEP0;
				else
					stack0[j].id=LOGICAL_OR_STEP0;
				stack0[j].third_tok=stack[i].left_tok;
				stack0[j].left_tok=-1;
				stack0[j].right_tok=-1;
				j++;

				// copia la seconda parte dello stack escludendo gli operatori collegati
				for (k=i+1;stack[k].id != EXPRESSION_END;k++) {
					for (l=0;l<num_of_linked_ops_0;l++)
						if (k == oplist[l])
							break;
					if (l == num_of_linked_ops_0)
						stack0[j++]=stack[k];
				}
				stack0[j].id=EXPRESSION_END;

				// copia stack0 nello stack
				memcpy(stack,stack0,(j+1)*sizeof(operator_t));

			}

		}
		// condizionale
		else if ( stack[i].id == CONDITIONAL ) {

			oplist[ 0 ] = -1; // VPCICE PATCH // // 20MAG2004 //

			// copia la prima parte dello stack (incluso l'operatore ?:)
			memcpy(stack0,stack,(i+1)*sizeof(operator_t));

			// imposta un primo operatore fittizio
			stack0[i].id=CONDITIONAL_STEP2;
			stack0[i].left_tok=-1;
			stack0[i].right_tok=-1;

			// collega gli operatori alla destra del :
			for (j=i+1;stack[j].id!=EXPRESSION_END;j++)
				if (stack[j].tok_id == stack[i].third_tok)
					break;
			if (stack[j].id!=EXPRESSION_END)
				num_of_linked_ops_0=LinkOperations(stack,oplist,j);
			else
				num_of_linked_ops_0=0;

			// copia gli operatori collegati
			for (j=0;j<num_of_linked_ops_0;j++)
				stack0[i+j+1]=stack[oplist[j]];

			// imposta un secondo operatore fittizio
			j+=i+1;
			stack0[j]=stack[i];
			stack0[j].id=CONDITIONAL_STEP1;
			stack0[j].third_tok=stack[i].right_tok;
			stack0[j].left_tok=-1;
			stack0[j].right_tok=-1;
			j++;

			// collega gli operatori alla destra del ?
			for (k=i+1;stack[k].id!=EXPRESSION_END;k++)
				if (stack[k].tok_id == stack[i].right_tok)
					break;
			if (stack[k].id!=EXPRESSION_END)
				num_of_linked_ops_1=LinkOperations(stack,oplist+num_of_linked_ops_0,k);
			else
				num_of_linked_ops_1=0;

			// copia gli operatori collegati
			for (k=0;k<num_of_linked_ops_1;k++,j++)
				stack0[j]=stack[oplist[k+num_of_linked_ops_0]];

			// imposta un terzo operatore fittizio
			stack0[j]=stack[i];
			stack0[j].id=CONDITIONAL_STEP0;
			stack0[j].third_tok=stack[i].left_tok;
			stack0[j].left_tok=-1;
			stack0[j].right_tok=-1;
			j++;

			// copia la seconda parte dello stack escludendo gli operatori collegati
			for (k=i+1;stack[k].id != EXPRESSION_END;k++) {
				for (l=0;oplist[l]!=-1;l++)
					if (k == oplist[l])
						break;
				if (oplist[l]==-1)
					stack0[j++]=stack[k];
			}
			stack0[j].id=EXPRESSION_END;

			// copia stack0 nello stack
			memcpy(stack,stack0,(j+1)*sizeof(operator_t));

		}
		// virgola
		else if ( 0 ) { // VPCICE PATCH // // 20MAG2004 //

			// copia la prima parte dello stack (incluso l'operatore comma)
			memcpy(stack0,stack,(i+1)*sizeof(operator_t));

			// continua solo se la virgola ha degli operatori collegati a sinistra
			if ( stack[i].left_tok != -1 ) {

				// collega gli operatori alla sinistra della virgola
				for (j=i+1;stack[j].id!=EXPRESSION_END;j++)
					if (stack[j].tok_id == stack[i].left_tok)
						break;
				if (stack[j].id!=EXPRESSION_END)
					num_of_linked_ops_0=LinkOperations(stack,oplist,j);
				else
					num_of_linked_ops_0=0;

				// copia gli operatori collegati
				for (j=0;j<num_of_linked_ops_0;j++)
					stack0[i+j+1]=stack[oplist[j]];

				// imposta j
				j+=i+1;

			}
			else {
				num_of_linked_ops_0=0;
				j=i+1;
			}

			// continua solo se la virgola ha degli operatori collegati a destra
			if ( stack[i].right_tok != -1 ) {

				// collega gli operatori alla destra della virgola
				for (k=i+1;stack[k].id!=EXPRESSION_END;k++)
					if (stack[k].tok_id == stack[i].right_tok)
						break;
				if (stack[k].id!=EXPRESSION_END)
					num_of_linked_ops_1=LinkOperations(stack,oplist+num_of_linked_ops_0,k);
				else
					num_of_linked_ops_1=0;

				// copia gli operatori collegati
				for (k=0;k<num_of_linked_ops_1;k++,j++)
					stack0[j]=stack[oplist[k+num_of_linked_ops_0]];

			}

			// copia la seconda parte dello stack escludendo gli operatori collegati
			for (k=i+1;stack[k].id != EXPRESSION_END;k++) {
				for (l=0;oplist[l]!=-1;l++)
					if (k == oplist[l])
						break;
				if (oplist[l]==-1)
					stack0[j++]=stack[k];
			}
			stack0[j].id=EXPRESSION_END;

			// copia stack0 nello stack
			memcpy(stack,stack0,(j+1)*sizeof(operator_t));

		}

	}

	// ritorna
	return;
}

//====================
//ReadStatementFromSource
//
//Legge una istruzione del file sorgente
//====================
int ReadStatementFromSource (void)
{
	static char		buffer[max_code_line_lenght];
	static int		return_code;
	static int		stop_to_parenthesis;
	static int		waiting_for_keyword;
	static int		parentheses_counter;
	static char		*first_parenthesis_pointer;
	static int		waiting_for_parenthesis;
	static int		equal_sign;
	static int		braces_counter;
	static int		is_struct_declaration;
	static int		is_variable_declaration;
	static int		is_function_declaration;
	int				skip_statement_update=0;
	int				skip_pointer0_setting=0;
	int				i,j,k,l;
	char			*pointer;
	int				ret;
	char			*stptr;
	char			*pointer0,*pointer1,*pointer2;
	char			ident[256];

	// legge una linea di codice dal file sorgente se necessario
	if (statement_start) {
		pointer=statement_start;
		statement_start=NULL;
	}
	else {

		// legge la linea di codice ed imposta pointer
		do {
			i=ReadLineFromSource(buffer);
			if (fatal_error || i==READLINE_FATAL)
				return(READSTATEMENT_FATAL);
		} while (i==READLINE_REPEAT || i==READLINE_ERROR);
		pointer=buffer;

		// controlla se il file sorgente è terminato
		if (i==READLINE_END)
			return(READSTATEMENT_END);

		// stampa nel file di output questa linea
		if ( psi_output_stream && used_streams==1 ) {
			MEMFILE_NAME(fprintf)(psi_output_stream,"==========(%i) %s\n",current_line,pointer);
			psi_output_stream_index++;
		}

	}

	// imposta stptr
	if (statement_incomplete) {
		stptr=statement_incomplete;
		statement_incomplete=NULL;
	}
	else {
		stptr=statement;
		stop_to_parenthesis=0;
		waiting_for_keyword=1;
		parentheses_counter=0;
		first_parenthesis_pointer=NULL;
		waiting_for_parenthesis=0;
		equal_sign=0;
		braces_counter=0;
		is_struct_declaration=0;
		is_variable_declaration=0;
		is_function_declaration=0;
	}

	// elimina gli spazi all'inizio della linea
	for (;*pointer;pointer++)
		if ( *pointer!=' ' &&
			*pointer!='\t' )
			break;

	// controlla se per caso non c'è niente da processare
	if (!*pointer)
		return(READSTATEMENT_REPEAT);

	// valuta se lo statement termina in questa linea
	k=-1;
	for (i=0,j=strlen(pointer);i<j;i++) {

		// valuta se impostare pointer0
		if (!skip_pointer0_setting)
			pointer0=&pointer[i];
		else
			skip_pointer0_setting=0;

		// esclude dal parsing il contenuto delle costanti e delle stringhe
		if (pointer[i]=='\\' && pointer[i+1]=='\\') {
			k=i++ +1;
			skip_pointer0_setting=1;
			continue;
		}
		if (pointer[i]=='"') {
			if (!i || pointer[i-1]!='\\' || k==i-1) {
				for (i++;i<j;i++) {
					if (pointer[i]=='\\' && pointer[i+1]=='\\') {
						k=i++ +1;
						continue;
					}
					if (pointer[i]=='"' &&
						(pointer[i-1]!='\\' || k==i-1))
						break;
				}
				if (i==j) {
					CompilerErrorOrWarning(212);
					return(READSTATEMENT_ERROR);
				}
			}
		}
		if (pointer[i]=='\'') {
			if (!i || pointer[i-1]!='\\' || k==i-1) {
				for (i++;i<j;i++) {
					if (pointer[i]=='\\' && pointer[i+1]=='\\') {
						k=i++ +1;
						continue;
					}
					if (pointer[i]=='\'' &&
						(pointer[i-1]!='\\' || k==i-1))
						break;
				}
				if (i==j) {
					CompilerErrorOrWarning(212);
					return(READSTATEMENT_ERROR);
				}
			}
		}

		// controlla se sono state escluse dal parsing stringhe o costanti
		pointer1=&pointer[i];
		if (pointer0 != pointer1) {

			// calcola la lunghezza della stringa o della costante
			k=pointer1-pointer0+1;

			// controlla che stptr non sia troppo grande
			if ( stptr-statement > max_statement_lenght-k-16 ) {
				CompilerErrorOrWarning(217);
				return(READSTATEMENT_ERROR);
			}

			// salva la stringa o la costante
			i++;
			memcpy(stptr,pointer0,k);
			stptr+=k;

			// resetta waiting_for_keyword
			waiting_for_keyword=0;

			// valuta se uscire adesso
			if (!pointer[i])
				break;

		}

		// valuta le diverse possibilità
		if ( pointer[i] == ' ' ||
			pointer[i] == '\t' ) {
			*stptr=' ';
			stptr++;
			continue;
		}
		else if ( waiting_for_keyword ) {

			// controlla che il carattere precedente non sia un carattere di un identificatore
			if (!i || (
				!(pointer[i-1]>='a' && pointer[i-1]<='z') &&
				!(pointer[i-1]>='A' && pointer[i-1]<='Z') &&
				!(pointer[i-1]>='0' && pointer[i-1]<='9') &&
				pointer[i-1]!='_' ) ) {

				// resetta for_arguments_p
				for_arguments_p=0;

				// controlla se si tratta di una keyword
				ret=IsIdentfString(&pointer[i]);
				if ( (ret==3 && CompareStrings(&pointer[i],"for",3)) ) {
					return_code=READSTATEMENT_KEYWORD_FOR;
					stop_to_parenthesis=1;
					waiting_for_parenthesis=1;
					i+=2;
					skip_statement_update=1;
				}
				else if ( (ret==5 && CompareStrings(&pointer[i],"while",5)) ) {
					return_code=READSTATEMENT_KEYWORD_WHILE;
					stop_to_parenthesis=1;
					waiting_for_parenthesis=1;
					i+=4;
					skip_statement_update=1;
				}
				else if ( (ret==2 && CompareStrings(&pointer[i],"if",2)) ) {
					return_code=READSTATEMENT_KEYWORD_IF;
					stop_to_parenthesis=1;
					waiting_for_parenthesis=1;
					i++;
					skip_statement_update=1;
				}
				else if ( ret==2 && CompareStrings(&pointer[i],"do",2) ) {
					statement_start=&pointer[i+2];
					*stptr=0;
					return(READSTATEMENT_KEYWORD_DO);
				}
				else if ( ret==4 && CompareStrings(&pointer[i],"else",4) ) {
					statement_start=&pointer[i+4];
					*stptr=0;
					return(READSTATEMENT_KEYWORD_ELSE);
				}
				else if ( ret==6 && CompareStrings(&pointer[i],"struct",6) )
					is_struct_declaration=1;
				else if ( ret==4 && CompareStrings(&pointer[i],"char",4) )
					is_variable_declaration=1;
				else if ( ret==3 && CompareStrings(&pointer[i],"int",3) )
					is_variable_declaration=1;
				else if ( ret==5 && CompareStrings(&pointer[i],"short",5) )
					is_variable_declaration=1;
				else if ( ret==4 && CompareStrings(&pointer[i],"long",4) )
					is_variable_declaration=1;
				else if ( ret==8 && CompareStrings(&pointer[i],"unsigned",8) )
					is_variable_declaration=1;
				else if ( ret==6 && CompareStrings(&pointer[i],"signed",6) )
					is_variable_declaration=1;
				else if ( ret==5 && CompareStrings(&pointer[i],"float",5) )
					is_variable_declaration=1;
				else if ( ret==6 && CompareStrings(&pointer[i],"double",6) )
					is_variable_declaration=1;
				else if ( ret==4 && CompareStrings(&pointer[i],"void",4) )
					is_variable_declaration=1;
				else if ( ret==6 && CompareStrings(&pointer[i],"static",6) )
					is_variable_declaration=1;
				else if ( ret==5 && CompareStrings(&pointer[i],"const",5) )
					is_variable_declaration=1;
				else if ( ret ) {

					// controlla se si tratta di un identificatore
					if (ret>=identifier_max_len)
						ret=identifier_max_len-1;
					memcpy(ident,&pointer[i],ret);
					ident[ret]=0;
					for (k=0;k<cident_id;k++)
						if ( (cident[k].type & 0xff0000) == STRUCT &&
							!strcmp(ident,cident[k].id) ) {
							is_variable_declaration=1;
							break;
						}

					// // // VITO PLANTAMURA PATCH 21GEN2004

					if ( is_variable_declaration == 0 )
						for ( k=0; k<cident_id; k++ )
							if ( cident[k].type == MACRO &&
								strlen( cident[k].id ) == (size_t) ret &&
								CompareStrings( & pointer[i], cident[k].id, ret ) )
							{
								char*		pszMacro = cident[k].pointer;

								if ( ret==4 && CompareStrings(pszMacro,"char",4) )
									is_variable_declaration=1;
								else if ( ret==3 && CompareStrings(pszMacro,"int",3) )
									is_variable_declaration=1;
								else if ( ret==5 && CompareStrings(pszMacro,"short",5) )
									is_variable_declaration=1;
								else if ( ret==4 && CompareStrings(pszMacro,"long",4) )
									is_variable_declaration=1;
								else if ( ret==8 && CompareStrings(pszMacro,"unsigned",8) )
									is_variable_declaration=1;
								else if ( ret==6 && CompareStrings(pszMacro,"signed",6) )
									is_variable_declaration=1;
								else if ( ret==5 && CompareStrings(pszMacro,"float",5) )
									is_variable_declaration=1;
								else if ( ret==6 && CompareStrings(pszMacro,"double",6) )
									is_variable_declaration=1;
								else if ( ret==4 && CompareStrings(pszMacro,"void",4) )
									is_variable_declaration=1;
								else if ( ret==6 && CompareStrings(pszMacro,"static",6) )
									is_variable_declaration=1;
								else if ( ret==5 && CompareStrings(pszMacro,"const",5) )
									is_variable_declaration=1;

								break;
							}

					// // //

				}
				else if ( CompareStrings(&pointer[i],"{",1) ) {
					statement_start=&pointer[i+1];
					*stptr=0;
					return(READSTATEMENT_OBRACE);
				}
				else if ( CompareStrings(&pointer[i],"}",1) ) {
					statement_start=&pointer[i+1];
					*stptr=0;
					return(READSTATEMENT_CBRACE);
				}
				else if ( CompareStrings(&pointer[i],";",1) ) {
					statement_start=&pointer[i+1];
					*stptr=0;
					return(READSTATEMENT_NULL);
				}

			}

			// resetta waiting_for_keyword
			waiting_for_keyword=0;

		}
		else {

			// ha trovato una parentesi tonda aperta
			if (CompareStrings(&pointer[i],"(",1)) {

				// incrementa il contatore delle parentesi tonde
				if (!parentheses_counter)
					first_parenthesis_pointer=stptr;
				parentheses_counter++;
				waiting_for_parenthesis=0;

				// controlla se si tratta di una dichiarazione di funzione
				if ( !equal_sign &&
					!is_struct_declaration &&
					is_variable_declaration ) {
					is_variable_declaration=0;
					is_function_declaration=1;
				}

			}
			// controlla se dopo un for o while o if non c'è la parentesi tonda aperta
			else if (waiting_for_parenthesis) {
				if (return_code == READSTATEMENT_KEYWORD_FOR)
					CompilerErrorOrWarning(214);
				else if (return_code == READSTATEMENT_KEYWORD_WHILE)
					CompilerErrorOrWarning(216);
				else if (return_code == READSTATEMENT_KEYWORD_IF)
					CompilerErrorOrWarning(218);
				return(READSTATEMENT_ERROR);
			}
			// decrementa il contatore delle parentesi tonde e controlla
			else if (CompareStrings(&pointer[i],")",1)) {
				if ( !--parentheses_counter &&
					( stop_to_parenthesis || is_function_declaration ) ) {

						// imposta le informazioni sui punti e virgola nel caso di un for
						if (!is_function_declaration &&
							return_code == READSTATEMENT_KEYWORD_FOR) {
							if (for_arguments_p!=2) {
								CompilerErrorOrWarning(220);
								return(READSTATEMENT_ERROR);
							}
							for_arguments[2][0]=for_arguments[1][1]+2;
							for_arguments[2][1]=stptr-1;
						}

						// imposta le informazioni sulla corrente posizione del parser
						statement_start=&pointer[i+1];
						*(stptr++)=')';
						*stptr=0;

						// ritorna
						if (is_function_declaration)
							return(READSTATEMENT_FUNCTIONDECL);
						else
							return(return_code);

				}
			}
			// gestisce i punti e virgola
			else if (CompareStrings(&pointer[i],";",1)) {

				// esce se non stiamo aspettando una parentesi tonda
				if (!stop_to_parenthesis) {

					// prima di procedere si assicura di non trovarsi nel corpo di una struct declaration
					if ( !(is_struct_declaration && braces_counter) ) {
						statement_start=&pointer[i+1];
						*stptr=0;
						if (is_struct_declaration)
							return(READSTATEMENT_STRUCTUREDECL);
						else if (is_variable_declaration)
							return(READSTATEMENT_VARIABLEDECL);
						else if (is_function_declaration) {
							CompilerErrorOrWarning(222);
							return(READSTATEMENT_ERROR);
						}
						else
							return(READSTATEMENT_OK);
					}

				}
				// se siamo nel caso di un for, aggiorna l'array dei puntatori
				else if (return_code == READSTATEMENT_KEYWORD_FOR) {

					// controlla che non ci siano troppi punti e virgola
					if (for_arguments_p == 2) {
						CompilerErrorOrWarning(213);
						return(READSTATEMENT_ERROR);
					}

					// aggiorna la lista dei punti e virgola
					if (!for_arguments_p) {
						if (first_parenthesis_pointer)
							for_arguments[0][0]=first_parenthesis_pointer+1;
						else {
							CompilerErrorOrWarning(214);
							return(READSTATEMENT_ERROR);
						}
					}
					else
						for_arguments[for_arguments_p][0]=for_arguments[for_arguments_p-1][1]+2;
					for_arguments[for_arguments_p][1]=stptr-1;

					// incrementa for_arguments_p
					for_arguments_p++;

				}
				// se siamo nel caso di un while o di un if, c'è un errore
				else if (return_code == READSTATEMENT_KEYWORD_WHILE) {
					CompilerErrorOrWarning(215);
					return(READSTATEMENT_ERROR);
				}
				else if (return_code == READSTATEMENT_KEYWORD_IF) {
					CompilerErrorOrWarning(219);
					return(READSTATEMENT_ERROR);
				}

			}
			// gestisce i due punti
			else if (CompareStrings(&pointer[i],":",1)) {

				// controlla che effettivamente sia una label (patch 14/7/1999 VP)
				l=0;
				for (pointer2=statement;pointer2<stptr;pointer2++) {
					if ( (*pointer2>='0' && *pointer2<='9') ||
						(*pointer2>='A' && *pointer2<='Z') ||
						(*pointer2>='a' && *pointer2<='z') ||
						*pointer2=='_' ) {
							if (l==2)
								break;
							else if ( !l )
								l=1;
					}
					else {
						if ( *pointer2 == ' ' ) {
							if ( l )
								l=2;
						}
						else
							break;
					}
				}

				// procede solo se abbiamo a che fare con una label (patch 14/7/1999 VP)
				if ( pointer2 == stptr && l ) {
					statement_start=&pointer[i+1];
					*stptr=0;
					return(READSTATEMENT_LABEL);
				}

			}
			// deve ricordarsi che ha trovato un segno di uguale
			else if (CompareStrings(&pointer[i],"=",1))
				equal_sign=1;
			else if ( CompareStrings(&pointer[i],"{",1) )
				braces_counter++;
			else if ( CompareStrings(&pointer[i],"}",1) )
				braces_counter--;

		}

		// incrementa stptr e aggiorna statement
		if (!skip_statement_update) {

			// aggiorna statement
			*stptr=pointer[i];
			stptr++;

			// controlla che stptr non sia troppo grande
			if ( stptr-statement > max_statement_lenght-16 ) {
				CompilerErrorOrWarning(217);
				return(READSTATEMENT_ERROR);
			}

		}
		skip_statement_update=0;

	}

	// imposta statement_incomplete
	*stptr=' ';
	statement_incomplete=++stptr;

	// ritorna
	return(READSTATEMENT_REPEAT);
}

//======================
//ProcessNextStatement
//
//Processa lo statement successivo
//======================
int ProcessNextStatement (void)
{
	int				ret,ret2;
	int				len;
	void			*old_pointer;
	instruction_t	**for_while_condition;
	instruction_t	**for_while_if_else_jump;
	char			expression[max_for_while_expression_len];
	exprvalue_t		result;
	int				i,j,k;
	nonnumber_t		*nn0;
	number_t		number;
	main_item_t		*main_stack_pointer;
	instruction_t	**instruction_ptrptr;
	identifier_t	identifier;
	char			*pointer;
	char			*aux_pointer;
	identifier_t	*func_declaration;
	instruction_t	*old_psi_ptr;

	// legge e compone il successivo statement
	do {
		ret=ReadStatementFromSource();
		if (ret == READSTATEMENT_FATAL)
			return(PROCESSNSTATEMENT_FATAL);
		else if (ret == READSTATEMENT_END)
			return(PROCESSNSTATEMENT_END);
	} while ( ret == READSTATEMENT_REPEAT || ret == READSTATEMENT_ERROR );

	// interpreta i diversi statement; ha trovato un do
	if (ret == READSTATEMENT_KEYWORD_DO) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// imposta le informazioni nel main_stack
		old_pointer=main_stack_aux_mem_ptr;
		main_stack_ptr->id=MSTACK_DO;
		main_stack_ptr->if_do_have_statement=0;
		main_stack_ptr->do_address=psi_ptr;
		main_stack_ptr->continue_stack_pointer =
			main_stack_ptr->continue_stack =
			main_stack_aux_mem_ptr;
		(char*)main_stack_aux_mem_ptr+=continue_break_stack_dim;
		main_stack_ptr->break_stack_pointer =
			main_stack_ptr->break_stack =
			main_stack_aux_mem_ptr;
		(char*)main_stack_aux_mem_ptr+=continue_break_stack_dim;
		main_stack_ptr->aux_mem_required=(int)main_stack_aux_mem_ptr-(int)old_pointer;
		main_stack_ptr++;

	}
	// ha trovato un for
	else if (ret == READSTATEMENT_KEYWORD_FOR) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// aggiusta eliminando gli spazi finali le tre stringhe di for_arguments
		for (i=0;i<3;i++)
			for (;for_arguments[i][1]>=for_arguments[i][0];for_arguments[i][1]--)
				if ( *for_arguments[i][1] != ' ' &&
					*for_arguments[i][1] != '\t' )
					break;

		// imposta le informazioni nel main_stack
		old_pointer=main_stack_aux_mem_ptr;
		main_stack_ptr->id=MSTACK_FOR;
		len=for_arguments[2][1]-for_arguments[2][0]+1;
		if (len) {
			main_stack_ptr->loop_expression=main_stack_aux_mem_ptr;
			memcpy(main_stack_aux_mem_ptr,for_arguments[2][0],len);
			((char*)main_stack_aux_mem_ptr)[len]=0;
			(char*)main_stack_aux_mem_ptr+=len+1;
		}
		else
			main_stack_ptr->loop_expression=NULL;
		main_stack_ptr->loop_line=current_line;
		for_while_condition=&main_stack_ptr->condition_address;
		for_while_if_else_jump=&main_stack_ptr->jump_address;
		main_stack_ptr->break_stack_pointer =
			main_stack_ptr->break_stack =
			main_stack_aux_mem_ptr;
		(char*)main_stack_aux_mem_ptr+=continue_break_stack_dim;
		main_stack_ptr->aux_mem_required=(int)main_stack_aux_mem_ptr-(int)old_pointer;
		main_stack_ptr++;

		// compone e assembla l'espressione di inizializzazione del for
		len=for_arguments[0][1]-for_arguments[0][0]+1;
		if (len) {
			memcpy(expression,for_arguments[0][0],len);
			expression[len]=0;
			if (ResolveNonConstantExpression(&result,expression))
				return(PROCESSNSTATEMENT_OK);
		}

		// compone e assembla l'espressione di condizione del for
		*for_while_condition=psi_ptr;
		len=for_arguments[1][1]-for_arguments[1][0]+1;
		if (len) {

			// imposta old_psi_ptr
			old_psi_ptr=psi_ptr;
			DeactivateAutomaticCodeReduction=1;

			// assembla l'espressione di condizione
			memcpy(expression,for_arguments[1][0],len);
			expression[len]=0;
			if (ResolveNonConstantExpression(&result,expression))
				return(PROCESSNSTATEMENT_OK);

			// carica in eax o st0 l'operando
			if (result.type==NON_NUMBER) {

				// imposta nn0 e verifica che non sia una struttura
				nn0=&result.non_number;
				if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
					!nn0->dimensions &&
					!nn0->indirection ) {
					CompilerErrorOrWarning(223);
					return(PROCESSNSTATEMENT_OK);
				}

				// carica in eax l'indirizzo della matrice
				if (nn0->dimensions) {
					if (nn0->array_stptr) {
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						psi_ptr->address.section=SECTION_STACK;
						psi_ptr->address.address=nn0->address;
					}
					else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
						psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
						psi_ptr->address.section=SECTION_DATA;
						psi_ptr->address.address=nn0->address;
					}
					else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
						psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
						psi_ptr->address.section=SECTION_STACK;
						psi_ptr->address.address=nn0->address;
					}
					else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
						psi_ptr->psi=LOAD_INT_IN_EAX;
						psi_ptr->address.section=SECTION_TEMPDATA1;
						psi_ptr->address.address=nn0->address;
					}
				}
				// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
				else if (nn0->is_pointer) {

					// carica in ecx il puntatore
					psi_ptr->psi=LOAD_INT_IN_ECX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
					psi_ptr++;

					// carica in eax o st0 il contenuto del puntatore ecx
					if ( (nn0->type & 0xffff) == INT ||
						nn0->indirection)
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					else if ( (nn0->type & 0xffff) == CHAR )
						psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
						psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
					else if ( (nn0->type & 0xffff) == SHORT )
						psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
						psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
						psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
					else if ( (nn0->type & 0xffff) == FLOAT )
						psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
					else if ( (nn0->type & 0xffff) == DOUBLE )
						psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
					else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
						psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

				}
				// carica in eax o st0 il valore
				else {

					// individua l'origine del valore
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
						psi_ptr->address.section=SECTION_DATA;
					else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->address.section=SECTION_STACK;
					else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
						psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;

					// carica in eax o st0 il valore
					if ( (nn0->type & 0xffff) == INT ||
						nn0->indirection) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
						else
							psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
						else
							psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
						else
							psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
						else
							psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
						else
							psi_ptr->psi=LOAD_INT_IN_EAX;
					}
					else if ( (nn0->type & 0xffff) == FLOAT ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
						else
							psi_ptr->psi=LOAD_FLOAT_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == DOUBLE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					}
					else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
						if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
							psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
						else
							psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					}

				}
				psi_ptr++;

			}
			else if (result.type==NUMBER) {

				// valuta se caricare il numero in eax oppure in st0
				if ( result.number.type > FLOAT )
					i=DOUBLE;
				else if ( result.number.type == FLOAT )
					i=FLOAT;
				else if (result.number.type == UNSIGNED_CHAR ||
					result.number.type == UNSIGNED_SHORT ||
					result.number.type == UNSIGNED_INT)
					i=UNSIGNED_INT;
				else
					i=INT;
				CastTypeOfConstantNumber(&result.number,&number,i);

				// carica il numero in eax o st0
				if (i==DOUBLE) {
					*(double*)strings_ptr=*(double*)number.number;
					psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
					psi_ptr->address.address=(int)strings_ptr;
					strings_ptr+=8;
					psi_ptr->address.section=SECTION_STRINGS;
				}
				else if (i==FLOAT) {
					*(float*)strings_ptr=*(float*)number.number;
					psi_ptr->psi=LOAD_FLOAT_IN_ST0;
					psi_ptr->address.address=(int)strings_ptr;
					strings_ptr+=4;
					psi_ptr->address.section=SECTION_STRINGS;
				}
				else {
					psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
					psi_ptr->constant=*(int*)number.number;
				}
				psi_ptr++;

			}
			else {
				CompilerErrorOrWarning(224);
				return(PROCESSNSTATEMENT_OK);
			}

			// stabilisce quale istruzione assemblare per il salto
			i=0;
			if (result.type==NON_NUMBER) {
				if ( !nn0->dimensions &&
					!nn0->indirection &&
					(nn0->type & 0xffff) >= FLOAT )
					i=1;
			}
			else if ( result.number.type >= FLOAT )
				i=1;

			// assembla l'istruzione per il salto
			if (i)
				psi_ptr->psi=JUMP_IF_ST0_ZERO;
			else
				psi_ptr->psi=JUMP_IF_EAX_ZERO;

			// memorizza nel main_stack l'indirizzo dell'istruzione per il salto
			*for_while_if_else_jump=psi_ptr++;

			// tenta di ridurre il codice appena assemblato
			ReducePSICode(old_psi_ptr-psi_mem,
				psi_ptr-psi_mem-1);

		}
		else
			*for_while_if_else_jump=NULL;

	}
	// ha trovato un while
	else if (ret == READSTATEMENT_KEYWORD_WHILE) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// verifica di che tipo di costrutto si tratta: do...while oppure while...
		if ( main_stack_ptr == main_stack ||
			(main_stack_ptr-1)->id != MSTACK_DO ||
			!(main_stack_ptr-1)->if_do_have_statement )
			main_stack_pointer = NULL; // while...
		else {

			// imposta main_stack_pointer
			main_stack_pointer = main_stack_ptr - 1; // do...while

		}

		// imposta le informazioni nel main_stack
		if (main_stack_pointer) { // do...while

			// processa gli item nello stack dei continue
			for ( instruction_ptrptr = main_stack_pointer->continue_stack;
				instruction_ptrptr < main_stack_pointer->continue_stack_pointer;
				instruction_ptrptr ++ ) {
					(*instruction_ptrptr)->address.section=SECTION_PSI;
					(*instruction_ptrptr)->address.address=(int)psi_ptr;
				}

		}
		else { // while...
			old_pointer=main_stack_aux_mem_ptr;
			main_stack_ptr->id=MSTACK_WHILE;
			for_while_condition=&main_stack_ptr->condition_address;
			for_while_if_else_jump=&main_stack_ptr->jump_address;
			main_stack_ptr->break_stack_pointer =
				main_stack_ptr->break_stack =
				main_stack_aux_mem_ptr;
			(char*)main_stack_aux_mem_ptr+=continue_break_stack_dim;
			main_stack_ptr->aux_mem_required=(int)main_stack_aux_mem_ptr-(int)old_pointer;
			main_stack_ptr++;
		}

		// solo nel caso di un while... memorizza la posizione della corrente istruzione
		if (!main_stack_pointer)
			*for_while_condition=psi_ptr;

		// imposta old_psi_ptr
		old_psi_ptr=psi_ptr;
		DeactivateAutomaticCodeReduction=1;

		// assembla l'espressione di condizione del while... o del do...while
		if (ResolveNonConstantExpression(&result,statement))
			return(PROCESSNSTATEMENT_OK);

		// carica in eax o st0 l'operando
		if (result.type==NON_NUMBER) {

			// imposta nn0 e verifica che non sia una struttura
			nn0=&result.non_number;
			if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
				!nn0->dimensions &&
				!nn0->indirection ) {
				CompilerErrorOrWarning(225);
				return(PROCESSNSTATEMENT_OK);
			}

			// carica in eax l'indirizzo della matrice
			if (nn0->dimensions) {
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}
			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// carica in eax o st0 il contenuto del puntatore ecx
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection)
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == FLOAT )
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
				else if ( (nn0->type & 0xffff) == DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}

			}
			psi_ptr++;

		}
		else if (result.type==NUMBER) {

			// valuta se caricare il numero in eax oppure in st0
			if ( result.number.type > FLOAT )
				i=DOUBLE;
			else if ( result.number.type == FLOAT )
				i=FLOAT;
			else if (result.number.type == UNSIGNED_CHAR ||
				result.number.type == UNSIGNED_SHORT ||
				result.number.type == UNSIGNED_INT)
				i=UNSIGNED_INT;
			else
				i=INT;
			CastTypeOfConstantNumber(&result.number,&number,i);

			// carica il numero in eax o st0
			if (i==DOUBLE) {
				*(double*)strings_ptr=*(double*)number.number;
				psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=8;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else if (i==FLOAT) {
				*(float*)strings_ptr=*(float*)number.number;
				psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=4;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else {
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=*(int*)number.number;
			}
			psi_ptr++;

		}
		else {
			CompilerErrorOrWarning(226);
			return(PROCESSNSTATEMENT_OK);
		}

		// stabilisce quale istruzione assemblare per il salto
		i=0;
		if (result.type==NON_NUMBER) {
			if ( !nn0->dimensions &&
				!nn0->indirection &&
				(nn0->type & 0xffff) >= FLOAT )
				i=1;
		}
		else if ( result.number.type >= FLOAT )
			i=1;

		// assembla l'istruzione per il salto
		if (main_stack_pointer) { // do...while

			// assembla l'istruzione
			if (i)
				psi_ptr->psi=JUMP_IF_ST0_NOT_ZERO;
			else
				psi_ptr->psi=JUMP_IF_EAX_NOT_ZERO;

			// imposta l'indirizzo del salto al rispettivo do
			psi_ptr->address.section=SECTION_PSI;
			psi_ptr->address.address=(int)main_stack_pointer->do_address;
			psi_ptr++;

			// processa lo stack dei break
			for ( instruction_ptrptr = main_stack_pointer->break_stack;
				instruction_ptrptr < main_stack_pointer->break_stack_pointer;
				instruction_ptrptr ++ ) {
					(*instruction_ptrptr)->address.section=SECTION_PSI;
					(*instruction_ptrptr)->address.address=(int)psi_ptr;
				}

			// libera lo stack dal do del blocco do...while
			(char*)main_stack_aux_mem_ptr-=main_stack_pointer->aux_mem_required;
			main_stack_ptr--;

			// il blocco do...while è uno statement: aggiusta lo stack
			StatementProcessed();

		}
		else { // while...

			// assembla l'istruzione
			if (i)
				psi_ptr->psi=JUMP_IF_ST0_ZERO;
			else
				psi_ptr->psi=JUMP_IF_EAX_ZERO;

			// memorizza nel main_stack l'indirizzo dell'istruzione per il salto
			*for_while_if_else_jump=psi_ptr++;

		}

		// tenta di ridurre il codice appena assemblato
		ReducePSICode(old_psi_ptr-psi_mem,
			psi_ptr-psi_mem-1);


		// controlla il punto e virgola finale
		if (main_stack_pointer) { // do...while

			// si assicura che il while sia seguito da un punto e virgola (patch 14/7/1999 VP)
			do
				ret2=ReadStatementFromSource();
			while ( ret2 == READSTATEMENT_REPEAT || ret2 == READSTATEMENT_ERROR );
			if ( ret2 != READSTATEMENT_NULL ) {
				CompilerErrorOrWarning(267);
				if (ret2 == READSTATEMENT_FATAL)
					return(PROCESSNSTATEMENT_FATAL);
				else if (ret2 == READSTATEMENT_END)
					return(PROCESSNSTATEMENT_END);
				else
					return(PROCESSNSTATEMENT_OK);
			}

		}

	}
	// ha trovato un if
	else if (ret == READSTATEMENT_KEYWORD_IF) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// imposta le informazioni nel main_stack
		main_stack_ptr->id=MSTACK_IF;
		main_stack_ptr->aux_mem_required=0;
		main_stack_ptr->if_do_have_statement=0;
		for_while_if_else_jump=&main_stack_ptr->jump_address;
		main_stack_ptr++;

		// imposta old_psi_ptr
		old_psi_ptr=psi_ptr;
		DeactivateAutomaticCodeReduction=1;

		// assembla l'espressione di condizione dell'if
		if (ResolveNonConstantExpression(&result,statement))
			return(PROCESSNSTATEMENT_OK);

		// carica in eax o st0 l'operando
		if (result.type==NON_NUMBER) {

			// imposta nn0 e verifica che non sia una struttura
			nn0=&result.non_number;
			if ( (nn0->type & 0xffff) == STRUCTURE_TYPE &&
				!nn0->dimensions &&
				!nn0->indirection ) {
				CompilerErrorOrWarning(227);
				return(PROCESSNSTATEMENT_OK);
			}

			// carica in eax l'indirizzo della matrice
			if (nn0->dimensions) {
				if (nn0->array_stptr) {
					psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
					psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
					psi_ptr->address.section=SECTION_DATA;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
					psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
					psi_ptr->address.section=SECTION_STACK;
					psi_ptr->address.address=nn0->address;
				}
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
					psi_ptr->psi=LOAD_INT_IN_EAX;
					psi_ptr->address.section=SECTION_TEMPDATA1;
					psi_ptr->address.address=nn0->address;
				}
			}
			// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
			else if (nn0->is_pointer) {

				// carica in ecx il puntatore
				psi_ptr->psi=LOAD_INT_IN_ECX;
				psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;
				psi_ptr++;

				// carica in eax o st0 il contenuto del puntatore ecx
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection)
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == CHAR )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
				else if ( (nn0->type & 0xffff) == SHORT )
					psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
					psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
					psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
				else if ( (nn0->type & 0xffff) == FLOAT )
					psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
				else if ( (nn0->type & 0xffff) == DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
					psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

			}
			// carica in eax o st0 il valore
			else {

				// individua l'origine del valore
				if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
					psi_ptr->address.section=SECTION_DATA;
				else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
					psi_ptr->address.section=SECTION_STACK;
				else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
					psi_ptr->address.section=SECTION_TEMPDATA1;
				psi_ptr->address.address=nn0->address;

				// carica in eax o st0 il valore
				if ( (nn0->type & 0xffff) == INT ||
					nn0->indirection) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
					else
						psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
					else
						psi_ptr->psi=LOAD_INT_IN_EAX;
				}
				else if ( (nn0->type & 0xffff) == FLOAT ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
					else
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}
				else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
					if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
						psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
					else
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				}

			}
			psi_ptr++;

		}
		else if (result.type==NUMBER) {

			// valuta se caricare il numero in eax oppure in st0
			if ( result.number.type > FLOAT )
				i=DOUBLE;
			else if ( result.number.type == FLOAT )
				i=FLOAT;
			else if (result.number.type == UNSIGNED_CHAR ||
				result.number.type == UNSIGNED_SHORT ||
				result.number.type == UNSIGNED_INT)
				i=UNSIGNED_INT;
			else
				i=INT;
			CastTypeOfConstantNumber(&result.number,&number,i);

			// carica il numero in eax o st0
			if (i==DOUBLE) {
				*(double*)strings_ptr=*(double*)number.number;
				psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=8;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else if (i==FLOAT) {
				*(float*)strings_ptr=*(float*)number.number;
				psi_ptr->psi=LOAD_FLOAT_IN_ST0;
				psi_ptr->address.address=(int)strings_ptr;
				strings_ptr+=4;
				psi_ptr->address.section=SECTION_STRINGS;
			}
			else {
				psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
				psi_ptr->constant=*(int*)number.number;
			}
			psi_ptr++;

		}
		else {
			CompilerErrorOrWarning(228);
			return(PROCESSNSTATEMENT_OK);
		}

		// stabilisce quale istruzione assemblare per il salto
		i=0;
		if (result.type==NON_NUMBER) {
			if ( !nn0->dimensions &&
				!nn0->indirection &&
				(nn0->type & 0xffff) >= FLOAT )
				i=1;
		}
		else if ( result.number.type >= FLOAT )
			i=1;

		// assembla l'istruzione per il salto
		if (i)
			psi_ptr->psi=JUMP_IF_ST0_ZERO;
		else
			psi_ptr->psi=JUMP_IF_EAX_ZERO;

		// memorizza nel main_stack l'indirizzo dell'istruzione per il salto
		*for_while_if_else_jump=psi_ptr++;

		// tenta di ridurre il codice appena assemblato
		ReducePSICode(old_psi_ptr-psi_mem,
			psi_ptr-psi_mem-1);

	}
	// ha trovato un else
	else if (ret == READSTATEMENT_KEYWORD_ELSE) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// controlla che all'else corrisponda un if
		if ( main_stack_ptr == main_stack ||
			(main_stack_ptr-1)->id != MSTACK_IF ||
			!(main_stack_ptr-1)->if_do_have_statement ) {
			CompilerErrorOrWarning(230);
			return(PROCESSNSTATEMENT_OK);
		}
		else
			main_stack_pointer = main_stack_ptr-1;

		// assembla e completa le istruzioni dei salti
		psi_ptr->psi=JUMP;
		psi_ptr++;
		if ( !compiler_errors ) { // patch 14/7/99 VP
			main_stack_pointer->jump_address->address.section=SECTION_PSI;
			main_stack_pointer->jump_address->address.address=(int)psi_ptr;
		}

		// imposta le informazioni nello stack
		main_stack_pointer->id=MSTACK_ELSE;
		main_stack_pointer->aux_mem_required=0;
		main_stack_pointer->jump_address=(psi_ptr-1);

	}
	// ha trovato un'etichetta
	else if (ret == READSTATEMENT_LABEL) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// calcola la lunghezza dell'etichetta e controlla se è tutto ok
		len=IsIdentfString(statement);
		if (len) {
			for (i=len;statement[i]!=0;i++) {
				if ( statement[i] != ' ' &&
					statement[i] != '\t' ) {
					CompilerErrorOrWarning(232);
					return(PROCESSNSTATEMENT_OK);
				}
			}
		}
		else {
			CompilerErrorOrWarning(232);
			return(PROCESSNSTATEMENT_OK);
		}

		// controlla se per caso la label è troppo lunga
		if (len>=identifier_max_len) {
			len=identifier_max_len-1;
			CompilerErrorOrWarning(233);
		}

		// registra questa label
		cident[cident_id].type=LABEL;
		memcpy(cident[cident_id].id,statement,len);
		cident[cident_id].id[len]=0;
		cident[cident_id].address=(int)psi_ptr;

		cident[cident_id].fields=0;				// not used
		cident[cident_id].indirection=0;		// not used
		cident[cident_id].dimensions=0;			// not used
		cident[cident_id].const_flag=0;			// not used
		cident[cident_id].static_flag=0;		// not used
		cident[cident_id].pointer=NULL;			// not used
		cident[cident_id].initializer=NULL;		// not used
		cident[cident_id].struct_type=NULL;		// not used
		cident[cident_id].array_stptr=0;		// not used
		cident[cident_id].ellipses_arguments=0;	// not used
		cident[cident_id].initializer_line=0;	// not used

		// controlla che non esista già una label con lo stesso identificatore
		for ( i = 0;
			i < cident_id;
			i ++ ) {
				if ( cident[i].type == LABEL &&
					!strcmp(cident[cident_id].id,cident[i].id) ) {
					CompilerErrorOrWarning(252);
					return(PROCESSNSTATEMENT_OK);
				}
			}

		// può adesso incrementare cident_id
		cident_id++;

		// una label è come se fosse un null statement
		StatementProcessed();

	}
	// ha trovato un null statement
	else if (ret == READSTATEMENT_NULL) {

		// continua solo se è all'interno di una funzione
		if (cur_prototype) {

			// rimuove dallo stack gli if morti, controlla che non ci siano do senza while
			if ( main_stack_ptr != main_stack ) {
				if ( (main_stack_ptr-1)->id == MSTACK_IF &&
					(main_stack_ptr-1)->if_do_have_statement )
					StatementProcessed();
				else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
					(main_stack_ptr-1)->if_do_have_statement ) {
					StatementProcessed();
					return(PROCESSNSTATEMENT_OK);
				}
			}

			// controlla lo stack
			StatementProcessed();

		}

	}
	// ha trovato una parentesi graffa aperta
	else if (ret == READSTATEMENT_OBRACE) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// imposta le informazioni nello stack
		main_stack_ptr->id=MSTACK_BRACE;
		main_stack_ptr->aux_mem_required=0;
		main_stack_ptr->identifier=cident_id;
		main_stack_ptr->identifier_aux_mem=cident_aux_ptr;
		main_stack_ptr->accepts_declarations=1;
		main_stack_ptr->enter_address=NULL;
		main_stack_ptr++;

	}
	// ha trovato una parentesi graffa chiusa
	else if (ret == READSTATEMENT_CBRACE) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
		}

		// controlla se effettivamente c'è una corrispondente parentesi graffa aperta
		if ( main_stack_ptr == main_stack ||
			(main_stack_ptr-1)->id != MSTACK_BRACE ) {
			CompilerErrorOrWarning(234);
			return(PROCESSNSTATEMENT_OK);
		}
		else
			main_stack_pointer = main_stack_ptr - 1;

		// risolve i salti dei goto se la parentesi chiude il corpo della funzione
		if ( main_stack_pointer == main_stack ) {

			// per ogni goto, cerca l'etichetta e risolve il salto
			for ( labels_ptrs_ptr--;
				labels_ptrs_ptr >= labels_ptrs;
				labels_ptrs_ptr-- ) {

					// imposta pointer
					pointer=(char*)((*labels_ptrs_ptr)->address.address);

					// cerca l'etichetta
					for ( i = main_stack_pointer->identifier;
						i < cident_id;
						i ++ ) {
							if ( cident[i].type == LABEL &&
								!strcmp(pointer,cident[i].id) ) {

								// imposta l'indirizzo
								(*labels_ptrs_ptr)->address.section=SECTION_PSI;
								(*labels_ptrs_ptr)->address.address=cident[i].address;
								break;

							}
						}

					// controlla se l'etichetta è stata trovata
					if ( i == cident_id )
						CompilerErrorOrWarning(251);

				}

		}

		// non distrugge le macro e le etichette definite nel corpo delle { }
		j = main_stack_pointer->identifier;
		for ( i = main_stack_pointer->identifier;
			i < cident_id;
			i ++ ) {

				// si regola sul da farsi a seconda che la parentesi chiuda il corpo della funzione o meno
				if ( main_stack_pointer != main_stack ) {

					// cerca macro oppure etichette
					if ( cident[i].type == MACRO ||
						cident[i].type == LABEL ) {
						if (i != j) {
							identifier=cident[j];
							cident[j]=cident[i];
							cident[i]=identifier;
						}
						j++;
					}

				}
				// la parentesi chiude il corpo della funzione: ci interessa salvare solo le macro
				else {

					// cerca e recupera solo le macro
					if ( cident[i].type == MACRO ) {
						if (i != j) {
							identifier=cident[j];
							cident[j]=cident[i];
							cident[i]=identifier;
						}
						j++;
					}

				}

			}

		// imposta cident_id e libera la memoria ausiliaria utilizzata per gli identificatori
		cident_id=j;
		cident_aux_ptr=main_stack_pointer->identifier_aux_mem;

		// imposta lo stack
		main_stack_ptr--;
		StatementProcessed();

		// se la parentesi chiude il corpo della funzione...
		if ( main_stack_ptr == main_stack ) {

			// controlla che sia già stato assemblato un RETURN alla fine della funzione
			if ( (psi_ptr-1)->psi != RETURN ) {
				psi_ptr->psi=RETURN;
				psi_ptr++;
			}

			// imposta a NULL cur_prototype
			cur_prototype=NULL;

			// imposta cident_id_local
			cident_id_local=99999999;

			// imposta la costante dell'ENTER di questa funzione
			if ( !compiler_errors ) { // patch 14/7/99 VP
				if ( !cur_local_var_address )
					main_stack_ptr->enter_address->psi=ENTER_WITH_NO_LOCAL_VARS;
				else if ( (-cur_local_var_address) >= 0x1000 ) {
					main_stack_ptr->enter_address->psi=ENTER_AND_CHECK_STACK;
					main_stack_ptr->enter_address->constant=-cur_local_var_address;
				}
				else
					main_stack_ptr->enter_address->constant=-cur_local_var_address;
			}

		}

	}
	// ha trovato una dichiarazione di struttura
	else if (ret == READSTATEMENT_STRUCTUREDECL) {

		// valuta se accettare la dichiarazione in questa posizione del codice
		if ( cur_prototype &&
			main_stack_ptr != main_stack )
			if ( (main_stack_ptr-1)->id != MSTACK_BRACE ||
				!(main_stack_ptr-1)->accepts_declarations ) {
				CompilerErrorOrWarning(263);
				return(PROCESSNSTATEMENT_OK);
			}

		// processa la dichiarazione di struttura
		if (ParseStructDeclaration(statement))
			return(PROCESSNSTATEMENT_OK);

	}
	// ha trovato una dichiarazione di variabile
	else if (ret == READSTATEMENT_VARIABLEDECL) {

		// valuta se accettare la dichiarazione in questa posizione del codice
		if ( cur_prototype &&
			main_stack_ptr != main_stack )
			if ( (main_stack_ptr-1)->id != MSTACK_BRACE ||
				!(main_stack_ptr-1)->accepts_declarations ) {
				CompilerErrorOrWarning(263);
				return(PROCESSNSTATEMENT_OK);
			}

		// processa la dichiarazione di variabile
		i=cident_id;
		if (ParseVariableDeclaration(statement,0,0,0))
			return(PROCESSNSTATEMENT_OK);

		// se è una o più variabili locali, deve procedere all'inizializzazione
		if (cur_prototype) {

			// per ognuna delle variabili dichiarate, inizializza
			for (;i<cident_id;i++) {

				// controlla se questa variabile ha bisogno di essere inizializzata
				if (cident[i].initializer) {

					// analizza la stringa di inizializzazione
					aux_pointer = cident[i].initializer;
					for ( pointer = cident[i].initializer ;
						*pointer ;
						pointer ++ ) {

							// cerca il carattere \n
							if ( *pointer == '\n' ) {

								// compone la stringa di inizializzazione
								j=pointer-aux_pointer;
								if ( j >= max_statement_lenght-1 ) {
									CompilerErrorOrWarning(237);
									return(PROCESSNSTATEMENT_OK);
								}
								memcpy(statement,aux_pointer,j);
								statement[j]=0;

								// controlla se si tratta di un'inizializzazione di stringa
								if ( (cident[i].type & 0xff00) == CHAR &&
									( cident[i].dimensions + cident[i].indirection ) &&
									cident[i].indirection <= 1) {

									// cerca l'uguale
									for (;aux_pointer<pointer;aux_pointer++) {

										if ( (*aux_pointer >= 'a' && *aux_pointer <= 'z') ||
											(*aux_pointer >= 'A' && *aux_pointer <= 'Z') ||
											(*aux_pointer >= '0' && *aux_pointer <= '9') ||
											*aux_pointer == '_' ||
											*aux_pointer == '[' ||
											*aux_pointer == ']' )
											continue;
										else if ( *aux_pointer == '=' ) {

											// ha trovato l'uguale; calcola la len della stringa
											k=EliminateEscapeSequences(aux_pointer+2,NULL,pointer-aux_pointer-3,0);
											if ( cident[i].pointer && ((int*)cident[i].pointer)[cident[i].dimensions-1] > k ) // VPCICE PATCH // // 20MAG2004 //
												allow_special_char_assignment=k+1;
											else
												allow_special_char_assignment=k;
											break;

										}
										else
											break;

									}

								}

								// assembla l'inizializzatore
								k=ResolveNonConstantExpression(&result,statement);
								allow_special_char_assignment=0;
								if (k)
									return(PROCESSNSTATEMENT_OK);

								// imposta aux_pointer
								aux_pointer=pointer+1;

							}

						}

				}

			}

		}

	}
	// ha trovato una dichiarazione di funzione
	else if (ret == READSTATEMENT_FUNCTIONDECL) {

		// valuta se accettare la dichiarazione in questa posizione del codice
		if ( cur_prototype &&
			main_stack_ptr != main_stack )
			if ( (main_stack_ptr-1)->id != MSTACK_BRACE ||
				!(main_stack_ptr-1)->accepts_declarations ) {
				CompilerErrorOrWarning(263);
				return(PROCESSNSTATEMENT_OK);
			}

		// fa una copia della dichiarazione di funzione
		memcpy(auxstatement,statement,max_statement_lenght);

		// legge lo statement successivo; si aspetta un ; oppure una {
		do {
			ret=ReadStatementFromSource();
			if (ret == READSTATEMENT_FATAL)
				return(PROCESSNSTATEMENT_FATAL);
			else if (ret == READSTATEMENT_END) {
				CompilerErrorOrWarning(240);
				return(PROCESSNSTATEMENT_END);
			}
			else if (ret == READSTATEMENT_ERROR)
				return(PROCESSNSTATEMENT_OK);
		} while ( ret == READSTATEMENT_REPEAT );

		// stabilisce se è un prototipo o meno oppure se c'è un errore sintattico
		if ( ret == READSTATEMENT_OBRACE ) {
			if (cur_prototype) {
				CompilerErrorOrWarning(240);
				return(PROCESSNSTATEMENT_OK);
			}
			else
				is_prototype=0;
		}
		else if ( ret == READSTATEMENT_NULL )
			is_prototype=1;
		else {
			CompilerErrorOrWarning(240);
			return(PROCESSNSTATEMENT_OK);
		}

		// processa la dichiarazione di funzione
		i=cident_id;
		if (ParseFunctionDeclaration(auxstatement))
			return(PROCESSNSTATEMENT_OK);

		// imposta func_declaration
		func_declaration=&cident[i];

		// verifica che in caso di dichiarazione non ci siano ellipses_arguments
		if ( func_declaration->ellipses_arguments &&
			!is_prototype ) {
			cident_id=i;
			CompilerErrorOrWarning(255);
			return(PROCESSNSTATEMENT_OK);
		}

		// se esiste un prototipo controlla che sia compatibile
		if ( func_prototype ) {

			// per prima cosa, controlla se il numero di parametri corrisponde
			if ( func_declaration -> fields !=
				func_prototype -> fields ) {
				CompilerErrorOrWarning(241);
				return(PROCESSNSTATEMENT_OK);
			}

			// controlla il formato dei dati restituiti e ciascun parametro
			for ( j = 0 ;
				j <= func_prototype -> fields ;
				j ++ ) {

					// confronta il tipo dei parametri
					if ( (func_prototype[j].type & 0xffff) !=
						(func_declaration[j].type & 0xffff) ) {

						// tipi incompatibili
						CompilerErrorOrWarning(241);
						return(PROCESSNSTATEMENT_OK);

					}

					// controlla il level of indirection
					if ( func_prototype[j].indirection != func_declaration[j].indirection ||
						func_prototype[j].dimensions != func_declaration[j].dimensions ) {

						// effettua un ultimo controllo
						if ( !func_prototype[j].dimensions &&
							func_declaration[j].dimensions == 1 ) {

							if ( func_prototype[j].indirection - func_declaration[j].indirection != 1 ) {

								// tipi incompatibili
								CompilerErrorOrWarning(241);
								return(PROCESSNSTATEMENT_OK);

							}

						}
						else if ( !func_declaration[j].dimensions &&
							func_prototype[j].dimensions == 1 ) {

							if ( func_declaration[j].indirection - func_prototype[j].indirection != 1 ) {

								// tipi incompatibili
								CompilerErrorOrWarning(241);
								return(PROCESSNSTATEMENT_OK);

							}

						}
						else {

							// tipi incompatibili
							CompilerErrorOrWarning(241);
							return(PROCESSNSTATEMENT_OK);

						}

					}
					else if ( func_prototype[j].dimensions ) {

						// controlla che le dimensioni corrispondano
						for ( k = 1 ;
							k < func_prototype[j].dimensions ;
							k ++ )
								if ( ((int*)func_prototype[j].pointer)[k] !=
									((int*)func_declaration[j].pointer)[k] ) {

									// almeno una dimensione è diversa... esce
									CompilerErrorOrWarning(241);
									return(PROCESSNSTATEMENT_OK);

								}

					}

				}

		}

		// controlla se si tratta di una dichiarazione multipla
		if ( !is_prototype &&
			( func_prototype &&
			func_prototype -> address ) ) {
			CompilerErrorOrWarning(242);
			return(PROCESSNSTATEMENT_OK);
		}

		// controlla se esiste un prototipo e se non esiste duplica la dichiarazione
		if ( !func_prototype &&
			!is_prototype) {

			// duplica la dichiarazione
			for ( i=0;
				i<=func_declaration->fields;
				i++ )
					cident[cident_id+i]=func_declaration[i];

			// aggiorna cident_id e imposta func_prototype
			func_prototype=func_declaration;
			func_declaration=&cident[cident_id];
			cident_id+=func_declaration->fields+1;

		}

		// se è un prototipo e ne esiste già uno elimina il secondo
		if ( is_prototype &&
			func_prototype )
			cident_id=func_declaration-cident;

		// se si tratta di una dichiarazione, predispone le variabili passate e assembla il codice
		if ( !is_prototype ) {

			// assembla il codice all'inizio della funzione e imposta lo stack
			psi_ptr->psi=ENTER;
			main_stack_ptr->id=MSTACK_BRACE;
			main_stack_ptr->aux_mem_required=0;
			main_stack_ptr->identifier=cident_id-func_prototype->fields-1;
			main_stack_ptr->identifier_aux_mem=cident_aux_ptr;
			main_stack_ptr->accepts_declarations=1;
			main_stack_ptr->enter_address=psi_ptr;
			func_prototype->address=(int)psi_ptr;

			// incrementa il puntatore del codice e quello del main_stack
			psi_ptr++;
			main_stack_ptr++;

			// imposta le informazioni sui parametri passati
			func_declaration->type=UNUSED;
			for ( i=1 , j=8 ;
				i<=func_prototype->fields;
				i++ ) {

					// calcola l'indirizzo del parametro nello stack del sistema
					k=func_prototype[i].address;
					if (k % 4)
						k=(k+4>>2)<<2;
					func_declaration[i].address=j;
					j+=k;

					// imposta le altre informazioni...
					func_declaration[i].type &= 0xffff;
					func_declaration[i].type |= VARIABLE;
					func_declaration[i].static_flag=0;
					func_declaration[i].initializer=NULL;
					func_declaration[i].initializer_line=0;
					if ( func_declaration[i].dimensions )
						func_declaration[i].array_stptr=1;
					else
						func_declaration[i].array_stptr=0;
					func_declaration[i].ellipses_arguments=0;

				}

			// resetta cur_local_var_address
			cur_local_var_address=0;

			// imposta cident_id_local
			cident_id_local=cident_id-func_prototype->fields-1;

			// imposta cur_prototype
			cur_prototype=func_prototype;

			// imposta labels_ptr e labels_ptrs_ptr
			labels_ptr=labels;
			labels_ptrs_ptr=labels_ptrs;

		}

	}
	// ha trovato uno statement
	else if (ret == READSTATEMENT_OK) {

		// si assicura che sia tutto ok
		if (!cur_prototype) {
			CompilerErrorOrWarning(229);
			return(PROCESSNSTATEMENT_OK);
		}

		// rimuove dallo stack gli if morti, controlla che non ci siano do senza while, controlla le parentesi graffe
		if ( main_stack_ptr != main_stack ) {
			if ( (main_stack_ptr-1)->id == MSTACK_IF &&
				(main_stack_ptr-1)->if_do_have_statement )
				StatementProcessed();
			else if ( (main_stack_ptr-1)->id == MSTACK_DO &&
				(main_stack_ptr-1)->if_do_have_statement ) {
				StatementProcessed();
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( (main_stack_ptr-1)->id == MSTACK_BRACE &&
				(main_stack_ptr-1)->accepts_declarations )
				(main_stack_ptr-1)->accepts_declarations=0;
		}

		// controlla  se si tratta di una keyword
		pointer=statement;
		ret=IsIdentfString(pointer);
		// si tratta di un break
		if ( (ret==5 && CompareStrings(pointer,"break",5)) ) {
			pointer+=5;

			// elimina gli spazi dopo la keyword
			for (;*pointer;pointer++)
				if ( *pointer!=' ' &&
					*pointer!='\t' )
					break;

			// controlla che sia tutto ok
			if ( *pointer ) {
				CompilerErrorOrWarning(243);
				return(PROCESSNSTATEMENT_OK);
			}

			// cerca un corrispondente do, for oppure while
			for ( main_stack_pointer = main_stack_ptr-1 ;
				main_stack_pointer >= main_stack ;
				main_stack_pointer-- ) {

					// controlla se si tratta di un do, un for oppure un while
					if ( main_stack_pointer->id == MSTACK_DO ||
						main_stack_pointer->id == MSTACK_FOR ||
						main_stack_pointer->id == MSTACK_WHILE )
						break;

				}

			// controlla se ha effettivamente trovato qualcosa
			if ( main_stack_pointer < main_stack ) {
				CompilerErrorOrWarning(244);
				return(PROCESSNSTATEMENT_OK);
			}

			// pone nello stack dei break l'indirizzo dello jump
			psi_ptr->psi=JUMP;
			*main_stack_pointer->break_stack_pointer=psi_ptr++;
			main_stack_pointer->break_stack_pointer++;

		}
		// si tratta di un continue
		else if ( (ret==8 && CompareStrings(pointer,"continue",8)) ) {
			pointer+=8;

			// elimina gli spazi dopo la keyword
			for (;*pointer;pointer++)
				if ( *pointer!=' ' &&
					*pointer!='\t' )
					break;

			// controlla che sia tutto ok
			if ( *pointer ) {
				CompilerErrorOrWarning(245);
				return(PROCESSNSTATEMENT_OK);
			}

			// cerca un corrispondente do, for oppure while
			for ( main_stack_pointer = main_stack_ptr-1 ;
				main_stack_pointer >= main_stack ;
				main_stack_pointer-- ) {

					// controlla se si tratta di un do, un for oppure un while
					if ( main_stack_pointer->id == MSTACK_DO ||
						main_stack_pointer->id == MSTACK_FOR ||
						main_stack_pointer->id == MSTACK_WHILE )
						break;

				}

			// controlla se ha effettivamente trovato qualcosa
			if ( main_stack_pointer < main_stack ) {
				CompilerErrorOrWarning(246);
				return(PROCESSNSTATEMENT_OK);
			}

			// pone nello stack dei continue l'indirizzo dello jump o risolve direttamente
			psi_ptr->psi=JUMP;
			if ( main_stack_pointer->id == MSTACK_DO ) {
				*main_stack_pointer->continue_stack_pointer=psi_ptr++;
				main_stack_pointer->continue_stack_pointer++;
			}
			else {
				psi_ptr->address.section=SECTION_PSI;
				psi_ptr->address.address=(int)main_stack_pointer->condition_address;
				psi_ptr++;
			}

		}
		// si tratta di un goto
		else if ( (ret==4 && CompareStrings(pointer,"goto",4)) ) {
			pointer+=4;

			// elimina gli spazi dopo la keyword
			for (;*pointer;pointer++)
				if ( *pointer!=' ' &&
					*pointer!='\t' )
					break;

			// controlla che sia tutto ok
			if ( !*pointer ) {
				CompilerErrorOrWarning(248);
				return(PROCESSNSTATEMENT_OK);
			}

			// copia la label nella sezione appropriata
			ret=IsIdentfString(pointer);
			if (!ret) {
				CompilerErrorOrWarning(248);
				return(PROCESSNSTATEMENT_OK);
			}
			else if ( *pointer >= '0' && *pointer <= '9' ) {
				CompilerErrorOrWarning(248);
				return(PROCESSNSTATEMENT_OK);
			}
			else {

				// imposta il codice per il salto
				psi_ptr->psi=JUMP;
				psi_ptr->address.section=SECTION_LABELS;
				psi_ptr->address.address=(int)labels_ptr;
				*labels_ptrs_ptr=psi_ptr++;
				labels_ptrs_ptr++;
				if (ret>=identifier_max_len) {
					i=identifier_max_len-1;
					CompilerErrorOrWarning(233);
				}
				else
					i=ret;
				memcpy(labels_ptr,pointer,i);
				labels_ptr[i]=0;
				labels_ptr+=i+1;

				// controlla che non ci sia dell'altro dopo la label
				pointer+=ret;
				for (;*pointer;pointer++)
					if ( *pointer!=' ' &&
						*pointer!='\t' )
						break;
				if ( *pointer ) {
					CompilerErrorOrWarning(249);
					return(PROCESSNSTATEMENT_OK);
				}

			}


		}
		// si tratta di un return
		else if ( (ret==6 && CompareStrings(pointer,"return",6)) ) {
			pointer+=6;

			// elimina gli spazi dopo la keyword
			for (;*pointer;pointer++)
				if ( *pointer!=' ' &&
					*pointer!='\t' )
					break;

			// controlla se è una void function
			if ( !cur_prototype->indirection &&
				( cur_prototype->type & 0xffff ) == VOID_TYPE ) {

				// controlla se è tutto ok
				if ( *pointer ) {
					CompilerErrorOrWarning(253);
					return(PROCESSNSTATEMENT_OK);
				}

				// assembla l'istruzione del ritorno
				psi_ptr->psi=RETURN;
				psi_ptr++;

			}
			// non è una void function
			else {

				// controlla se è tutto ok
				if ( !*pointer ) {
					CompilerErrorOrWarning(254);
					return(PROCESSNSTATEMENT_OK);
				}

				// imposta old_psi_ptr
				old_psi_ptr=psi_ptr;
				DeactivateAutomaticCodeReduction=1;

				// assembla l'espressione che segue il return
				if (ResolveNonConstantExpression(&result,pointer))
					return(PROCESSNSTATEMENT_OK);

				// controlla che il tipo non sia void
				if ( result.type == NOTHING_ ) {
					CompilerErrorOrWarning(90);
					return(PROCESSNSTATEMENT_OK);
				}
				// è un numero
				else if ( result.type == NUMBER ) {

					// controlla il prototipo della funzione
					if ( cur_prototype->indirection ) {

						// se il numero è un floating point esce direttamente
						if (result.number.type >= FLOAT) {
							CompilerErrorOrWarning(256);
							return(PROCESSNSTATEMENT_OK);
						}

						// converte in intero il numero
						if (result.number.type==CHAR)
							i=*(char*)result.number.number;
						else if (result.number.type==UNSIGNED_CHAR)
							i=*(unsigned char*)result.number.number;
						else if (result.number.type==SHORT)
							i=*(short*)result.number.number;
						else if (result.number.type==UNSIGNED_SHORT)
							i=*(unsigned short*)result.number.number;
						else if (result.number.type==INT)
							i=*(int*)result.number.number;
						else if (result.number.type==UNSIGNED_INT)
							i=*(unsigned int*)result.number.number;

						// continua solo se è uguale a zero
						if (i) {
							CompilerErrorOrWarning(256);
							return(PROCESSNSTATEMENT_OK);
						}
						else
							i=UNSIGNED_INT;

					}
					else if ( (cur_prototype->type & 0xffff) == STRUCTURE_TYPE ) {
						CompilerErrorOrWarning(257);
						return(PROCESSNSTATEMENT_OK);
					}
					else {
						i=cur_prototype->type & 0xffff;
						if ( i > FLOAT )
							i=DOUBLE;
						else if ( i == UNSIGNED_CHAR ||
							i == UNSIGNED_SHORT ||
							i == UNSIGNED_INT )
							i=UNSIGNED_INT;
						else if ( i != FLOAT )
							i=INT;
					}

					// converte il numero nel formato richiesto dal prototipo
					CastTypeOfConstantNumber(&result.number,&number,i);

					// passa attraverso i registri il numero
					if ( (number.type & 0xffff) == FLOAT ) {
						*(float*)strings_ptr=*(float*)number.number;
						psi_ptr->psi=LOAD_FLOAT_IN_ST0;
						psi_ptr->address.address=(int)strings_ptr;
						strings_ptr+=4;
						psi_ptr->address.section=SECTION_STRINGS;
					}
					else if ( (number.type & 0xff00) == DOUBLE) {
						*(double*)strings_ptr=*(double*)number.number;
						psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
						psi_ptr->address.address=(int)strings_ptr;
						strings_ptr+=8;
						psi_ptr->address.section=SECTION_STRINGS;
					}
					else {
						psi_ptr->psi=LOAD_CONSTANT_IN_EAX;
						psi_ptr->constant=*(int*)number.number;
					}
					psi_ptr++;

				}
				// è un nonnumber_t
				else {

					// controlla che i tipi siano compatibili e passa il valore
					nn0=&result.non_number;
					if ( nn0->dimensions ) {

						// controlla le dimensioni
						if ( nn0->dimensions > 1 ||
							nn0->indirection+1 != cur_prototype->indirection ||
							(nn0->type & 0xffff) != (cur_prototype->type & 0xffff) ) {
							CompilerErrorOrWarning(258);
							return(PROCESSNSTATEMENT_OK);
						}

						// passa in eax l'indirizzo della matrice
						if (nn0->array_stptr) {
							psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
							psi_ptr->address.section=SECTION_STACK;
							psi_ptr->address.address=nn0->address;
						}
						else if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
							psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
							psi_ptr->address.section=SECTION_DATA;
							psi_ptr->address.address=nn0->address;
						}
						else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
							psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
							psi_ptr->address.section=SECTION_STACK;
							psi_ptr->address.address=nn0->address;
						}
						else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
							psi_ptr->psi=LOAD_INT_IN_EAX;
							psi_ptr->address.section=SECTION_TEMPDATA1;
							psi_ptr->address.address=nn0->address;
						}
						psi_ptr++;

					}
					else if ( nn0->indirection ) {

						// controlla i level of indirection
						if ( nn0->indirection != cur_prototype->indirection ||
							(nn0->type & 0xffff) != (cur_prototype->type & 0xffff) ) {
							CompilerErrorOrWarning(258);
							return(PROCESSNSTATEMENT_OK);
						}

						// carica in eax il contenuto di un RIGHT_VALUE pointer
						if (nn0->is_pointer) {

							// carica in ecx il puntatore
							psi_ptr->psi=LOAD_INT_IN_ECX;
							psi_ptr->address.section=SECTION_TEMPDATA1;
							psi_ptr->address.address=nn0->address;
							psi_ptr++;

							// carica in eax il contenuto del puntatore ecx
							psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;

						}
						// carica in eax il valore
						else {

							// carica in eax il puntatore
							if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 ) {
								psi_ptr->address.section=SECTION_DATA;
								psi_ptr->psi=LOAD_INT_IN_EAX;
							}
							else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 ) {
								psi_ptr->address.section=SECTION_STACK;
								psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
							}
							else if ( (nn0->type & 0xff0000) == RIGHT_VALUE ) {
								psi_ptr->address.section=SECTION_TEMPDATA1;
								psi_ptr->psi=LOAD_INT_IN_EAX;
							}
							psi_ptr->address.address=nn0->address;

						}
						psi_ptr++;

					}
					// non deve passare nè una matrice nè un puntatore
					else {

						// controlla il tipo
						if ( cur_prototype->indirection ) {
							CompilerErrorOrWarning(258);
							return(PROCESSNSTATEMENT_OK);
						}
						else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
							if ( (cur_prototype->type & 0xffff) != STRUCTURE_TYPE ||
								nn0->struct_type != cur_prototype->struct_type ) {
								CompilerErrorOrWarning(258);
								return(PROCESSNSTATEMENT_OK);
							}
						}

						// carica in eax o st0 il contenuto di un RIGHT_VALUE pointer
						if (nn0->is_pointer) {

							// carica in ecx il puntatore
							psi_ptr->psi=LOAD_INT_IN_ECX;
							psi_ptr->address.section=SECTION_TEMPDATA1;
							psi_ptr->address.address=nn0->address;
							psi_ptr++;

							// carica in eax o st0 il contenuto del puntatore ecx
							if ( (nn0->type & 0xffff) == CHAR )
								psi_ptr->psi=LOAD_ECXPTR_SIGNED_CHAR_IN_EAX;
							else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR )
								psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_CHAR_IN_EAX;
							else if ( (nn0->type & 0xffff) == SHORT )
								psi_ptr->psi=LOAD_ECXPTR_SIGNED_SHORT_IN_EAX;
							else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT )
								psi_ptr->psi=LOAD_ECXPTR_UNSIGNED_SHORT_IN_EAX;
							else if ( (nn0->type & 0xffff) == INT )
								psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
							else if ( (nn0->type & 0xffff) == UNSIGNED_INT )
								psi_ptr->psi=LOAD_ECXPTR_INT_IN_EAX;
							else if ( (nn0->type & 0xffff) == FLOAT )
								psi_ptr->psi=LOAD_ECXPTR_FLOAT_IN_ST0;
							else if ( (nn0->type & 0xffff) == DOUBLE )
								psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;
							else if ( (nn0->type & 0xffff) == LONG_DOUBLE )
								psi_ptr->psi=LOAD_ECXPTR_DOUBLE_IN_ST0;

						}
						// carica in eax o st0 il valore
						else {

							// individua l'origine del valore
							if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
								psi_ptr->address.section=SECTION_DATA;
							else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
								psi_ptr->address.section=SECTION_STACK;
							else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
								psi_ptr->address.section=SECTION_TEMPDATA1;
							psi_ptr->address.address=nn0->address;

							// carica in eax o st0 il valore
							if ( (nn0->type & 0xffff) == CHAR ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_SIGNED_CHAR_IN_EAX;
								else
									psi_ptr->psi=LOAD_SIGNED_CHAR_IN_EAX;
							}
							else if ( (nn0->type & 0xffff) == UNSIGNED_CHAR ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_UNSIGNED_CHAR_IN_EAX;
								else
									psi_ptr->psi=LOAD_UNSIGNED_CHAR_IN_EAX;
							}
							else if ( (nn0->type & 0xffff) == SHORT ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_SIGNED_SHORT_IN_EAX;
								else
									psi_ptr->psi=LOAD_SIGNED_SHORT_IN_EAX;
							}
							else if ( (nn0->type & 0xffff) == UNSIGNED_SHORT ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_UNSIGNED_SHORT_IN_EAX;
								else
									psi_ptr->psi=LOAD_UNSIGNED_SHORT_IN_EAX;
							}
							else if ( (nn0->type & 0xffff) == INT ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
								else
									psi_ptr->psi=LOAD_INT_IN_EAX;
							}
							else if ( (nn0->type & 0xffff) == UNSIGNED_INT ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_INT_IN_EAX;
								else
									psi_ptr->psi=LOAD_INT_IN_EAX;
							}
							else if ( (nn0->type & 0xffff) == FLOAT ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_FLOAT_IN_ST0;
								else
									psi_ptr->psi=LOAD_FLOAT_IN_ST0;
							}
							else if ( (nn0->type & 0xffff) == DOUBLE ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
								else
									psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
							}
							else if ( (nn0->type & 0xffff) == LONG_DOUBLE ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_DOUBLE_IN_ST0;
								else
									psi_ptr->psi=LOAD_DOUBLE_IN_ST0;
							}
							else if ( (nn0->type & 0xffff) == STRUCTURE_TYPE ) {
								if ( (nn0->type & 0xff0000) == LEFT_VALUE_0 )
									psi_ptr->psi=LOAD_CONSTANT_ADDRESS_IN_EAX;
								else if ( (nn0->type & 0xff0000) == LEFT_VALUE_1 )
									psi_ptr->psi=LOAD_STACK_INT_POINTER_IN_EAX;
								else if ( (nn0->type & 0xff0000) == RIGHT_VALUE )
									psi_ptr->psi=LOAD_INT_IN_EAX;
							}

						}
						psi_ptr++;

						// valuta se effettuare una qualche conversione
						if ( (nn0->type & 0xffff) != STRUCTURE_TYPE ) {

							// controlla i due casi
							if ( (nn0->type & 0xffff) < FLOAT &&
								(cur_prototype->type & 0xffff) >= FLOAT ) {

								// imposta le istruzioni per la conversione
								if ( (nn0->type & 0xffff) == UNSIGNED_INT )
									psi_ptr->psi=LOAD_UNSIGNED_EAX_IN_ST0;
								else
									psi_ptr->psi=LOAD_SIGNED_EAX_IN_ST0;
								psi_ptr++;

							}
							else if ( (cur_prototype->type & 0xffff) < FLOAT &&
								(nn0->type & 0xffff) >= FLOAT ) {
								
								// imposta le istruzioni per la conversione
								psi_ptr->psi=LOAD_ST0_IN_EAX;
								psi_ptr++;

							}

						}

					}

				}

				// tenta di ridurre il codice appena assemblato
				ReducePSICode(old_psi_ptr-psi_mem,
					psi_ptr-psi_mem-1);

				// assembla l'istruzione del ritorno
				psi_ptr->psi=RETURN;
				psi_ptr++;

			}

		}
		// si tratta di uno statement da assemblare
		else {

			// assembla lo statement
			if (ResolveNonConstantExpression(&result,statement))
				return(PROCESSNSTATEMENT_OK);

		}

		// controlla gli item nello stack
		StatementProcessed();

	}

	// ritorna
	return(PROCESSNSTATEMENT_OK);
}

//====================
//StatementProcessed
//
//Controlla lo stack e completa eventuali costrutti incompleti
//====================
void StatementProcessed (void)
{
	main_item_t		*main_stack_pointer;
	int				old_line;
	exprvalue_t		result;
	instruction_t	**instruction_ptrptr;

	// processa gli item dello stack dall'alto verso il basso
	while ( main_stack_ptr != main_stack ) {

		// imposta main_stack_pointer
		main_stack_pointer = main_stack_ptr - 1;

		// ha trovato un do
		if ( main_stack_pointer->id == MSTACK_DO ) {

			// considera il flag if_do_have_statement
			if ( main_stack_pointer->if_do_have_statement ) {

				// rimuove dallo stack il do, visualizza un messaggio di errore, esce
				(char*)main_stack_aux_mem_ptr-=main_stack_pointer->aux_mem_required;
				main_stack_ptr--;
				CompilerErrorOrWarning(231);
				break;

			}
			else {

				// imposta if_do_have_statement a 1 ed esce
				main_stack_pointer->if_do_have_statement=1;
				break;

			}

		}
		// ha trovato un for
		else if ( main_stack_pointer->id == MSTACK_FOR ) {

			// assembla la loop expression, se c'è
			if (main_stack_pointer->loop_expression) {
				old_line=current_line;
				current_line=main_stack_pointer->loop_line;
				if (ResolveNonConstantExpression(&result,main_stack_pointer->loop_expression)) {
					current_line=old_line;
					break;
				}
				current_line=old_line;
			}

			// imposta le istruzioni per il salto al codice della condizione del for
			psi_ptr->psi=JUMP;
			psi_ptr->address.section=SECTION_PSI;
			psi_ptr->address.address=(int)main_stack_pointer->condition_address;
			psi_ptr++;

			// imposta l'indirizzo dell'istruzione di salto nel caso di condizione negativa
			if ( main_stack_pointer->jump_address && !compiler_errors ) { // patch 14/7/99 VP
				main_stack_pointer->jump_address->address.section=SECTION_PSI;
				main_stack_pointer->jump_address->address.address=(int)psi_ptr;
			}

			// processa lo stack dei break
			for ( instruction_ptrptr = main_stack_pointer->break_stack;
				instruction_ptrptr < main_stack_pointer->break_stack_pointer;
				instruction_ptrptr ++ ) {
					(*instruction_ptrptr)->address.section=SECTION_PSI;
					(*instruction_ptrptr)->address.address=(int)psi_ptr;
				}

			// libera lo stack
			(char*)main_stack_aux_mem_ptr-=main_stack_pointer->aux_mem_required;
			main_stack_ptr--;

		}
		// ha trovato un while
		else if ( main_stack_pointer->id == MSTACK_WHILE ) {

			// imposta le istruzioni per il salto al codice della condizione del while
			psi_ptr->psi=JUMP;
			psi_ptr->address.section=SECTION_PSI;
			psi_ptr->address.address=(int)main_stack_pointer->condition_address;
			psi_ptr++;

			// imposta l'indirizzo dell'istruzione di salto nel caso di condizione negativa
			if ( !compiler_errors ) { // patch 14/7/99 VP
				main_stack_pointer->jump_address->address.section=SECTION_PSI;
				main_stack_pointer->jump_address->address.address=(int)psi_ptr;
			}

			// processa lo stack dei break
			for ( instruction_ptrptr = main_stack_pointer->break_stack;
				instruction_ptrptr < main_stack_pointer->break_stack_pointer;
				instruction_ptrptr ++ ) {
					(*instruction_ptrptr)->address.section=SECTION_PSI;
					(*instruction_ptrptr)->address.address=(int)psi_ptr;
				}

			// libera lo stack
			(char*)main_stack_aux_mem_ptr-=main_stack_pointer->aux_mem_required;
			main_stack_ptr--;

		}
		// ha trovato un if
		else if ( main_stack_pointer->id == MSTACK_IF ) {

			// considera il flag if_do_have_statement
			if ( main_stack_pointer->if_do_have_statement ) {

				// imposta l'indirizzo del salto dell'if in caso di condizione negativa
				if ( !compiler_errors ) { // patch 14/7/99 VP
					main_stack_pointer->jump_address->address.section=SECTION_PSI;
					main_stack_pointer->jump_address->address.address=(int)psi_ptr;
				}

				// rimuove dallo stack l'if
				main_stack_ptr--;

			}
			else {

				// imposta if_do_have_statement a 1 ed esce
				main_stack_pointer->if_do_have_statement=1;
				break;

			}

		}
		// ha trovato un else
		else if ( main_stack_pointer->id == MSTACK_ELSE ) {

			// imposta l'indirizzo del salto dell'else in caso di condizione negativa
			if ( !compiler_errors ) { // patch 14/7/99 VP
				main_stack_pointer->jump_address->address.section=SECTION_PSI;
				main_stack_pointer->jump_address->address.address=(int)psi_ptr;
			}

			// rimuove dallo stack l'else
			main_stack_ptr--;

		}
		// non ha trovato niente di interessante, quindi esce
		else
			break;

	}

	// ritorna
	return;
}

//==================
//CompileSource
//
//Esegue la compilazione
//==================
void CompileSource (CompileSourceOptions_t *CompileSourceOptions)
{
	MEMFILE_NAME(FILE)	*stream;
	int					i,j,k,l;
	char				buffer[max_code_line_lenght];
	instruction_t		*old_psi_ptr;
	psi_bound_t			*psi_bounds_pointer;
	char				*ptr;
	instruction_t		*jump_pointer;
	instruction_t		*psi_pointer;

	// copia le informazioni passate
	strcpy(source_file,CompileSourceOptions->source_file);
	strcpy(compiled_file,CompileSourceOptions->compiled_file);
	strcpy(PSI_file,CompileSourceOptions->PSI_file);
	strcpy(source_directory,CompileSourceOptions->source_directory);
	strcpy(headers_directory,CompileSourceOptions->headers_directory);
	strcpy(definitions,CompileSourceOptions->definitions);

	// inizializza il compilatore
	InitCompiler();

	// registra le definizioni passate dall'esterno
	if (ProcessExternalDefinitions())
		return;

	// inizializza lo stack del preprocessore
	InitializePreprocessorStack(source_file);
	if (fatal_error)
		return;

	// imposta lo stream del file compilato
	output_stream=MEMFILE_NAME(fopen)(compiled_file,"wb");
	if (!output_stream) {
		PreprocStackFcloseAll();
		CompilerErrorOrWarning(262);
		return;
	}

	// imposta gli stream dell'output PSI per la lettura e la scrittura
	if (*PSI_file) {

		// apre gli stream
		stream=MEMFILE_NAME(fopen)(PSI_file,"wt");
		psi_output_stream=MEMFILE_NAME(fopen)(psi_output_temporary_file_name,"w+t");

		// controlla che sia tutto ok
		if (!stream) {
			PreprocStackFcloseAll();
			MEMFILE_NAME(fclose)(output_stream);
			CompilerErrorOrWarning(260);
			return;
		}
		else if (!psi_output_stream) {
			PreprocStackFcloseAll();
			MEMFILE_NAME(fclose)(output_stream);
			MEMFILE_NAME(fclose)(stream);
			CompilerErrorOrWarning(261);
			return;
		}

	}
	else {
		stream=NULL;
		psi_output_stream=NULL;
	}

	// esegue la compilazione tenendo traccia dei blocchi di istruzioni PSI
	do {
		old_psi_ptr=psi_ptr;
		i=ProcessNextStatement();
		if (i==PROCESSNSTATEMENT_FATAL || fatal_error) {
			PreprocStackFcloseAll();
			MEMFILE_NAME(fclose)(output_stream);
			if (stream) {
				MEMFILE_NAME(fclose)(psi_output_stream);
				MEMFILE_NAME(fclose)(stream);
				MEMFILE_NAME(remove)(psi_output_temporary_file_name);
			}
			return;
		}
		if (stream &&
			psi_ptr != old_psi_ptr) {
			psi_bounds_ptr->instruction_id=psi_ptr-psi_mem;
			psi_bounds_ptr->text_id=psi_output_stream_index;
			psi_bounds_ptr++;
		}
	}
	while (i==PROCESSNSTATEMENT_OK);

	// azzera il puntatore del file temporaneo
	if (stream && !compiler_errors && !fatal_error )
		MEMFILE_NAME(rewind)(psi_output_stream);

	// continua solo se non ci sono errori
	if ( !compiler_errors && !fatal_error ) {

		// ciclo per tutte le istruzioni PSI; risolve i JUMP multipli
		for ( psi_pointer=psi_mem;
			psi_pointer<psi_ptr;
			psi_pointer++ ) {

				// se è un JUMP, procede
				if (psi_pointer->psi == JUMP) {

					// ciclo di risoluzione dei jump multipli
					for ( jump_pointer=(instruction_t*)psi_pointer->address.address;
						jump_pointer->psi==JUMP;
						jump_pointer=(instruction_t*)jump_pointer->address.address );

					// imposta il nuovo indirizzo del salto
					psi_pointer->address.address=(int)jump_pointer;

				}

			}

	}

	// continua solo se bisogna creare il PSI output e non ci sono errori di nessun tipo
	if (stream && !compiler_errors && !fatal_error ) {

		// ciclo per la creazione del file di output delle istruzioni PSI
		for ( i=0, psi_bounds_pointer=psi_bounds;
			psi_bounds_pointer<psi_bounds_ptr;
			psi_bounds_pointer++,i=1 ) {

				// riporta nel file di output delle istruzioni PSI le linee del file sorgente
				k=psi_bounds_pointer->text_id;
				if (i)
					k-=(psi_bounds_pointer-1)->text_id;
				for (j=0;j<k;j++) {

					// copia dal file temporaneo al file di testo di output 
					ptr=MEMFILE_NAME(fgets)(buffer,max_code_line_lenght,psi_output_stream);
					for (l=0;l<max_code_line_lenght;l++) {
						if (buffer[l]==0xa) {
							buffer[l]=0;
							break;
						}
						else if (!buffer[l])
							break;
					}
					MEMFILE_NAME(fprintf)(stream,"%s\n",ptr);

				}

				// dopo aver copiato le linee del file sorgente, stampa le istruzioni PSI
				if (i)
					GeneratePSIOutput(stream,
						(psi_bounds_pointer-1)->instruction_id,
						psi_bounds_pointer->instruction_id-1);
				else
					GeneratePSIOutput(stream,
						0,
						psi_bounds_pointer->instruction_id-1);

			}

		// controlla se le linee finali nel file temporaneo siano state copiate
		if (psi_bounds_pointer != psi_bounds) {
			k=psi_output_stream_index-(psi_bounds_pointer-1)->text_id;
			for	(j=0;j<k;j++) {
				ptr=MEMFILE_NAME(fgets)(buffer,max_code_line_lenght,psi_output_stream);
				for (l=0;l<max_code_line_lenght;l++) {
					if (buffer[l]==0xa) {
						buffer[l]=0;
						break;
					}
					else if (!buffer[l])
						break;
				}
				MEMFILE_NAME(fprintf)(stream,"%s\n",ptr);
			}
		}

	}

	// genera il file di output compilato
	if ( !compiler_errors && !fatal_error )
		WriteOutputFile();

	// chiude gli stream ed elimina il file temporaneo
	MEMFILE_NAME(fclose)(output_stream);
	if (stream) {
		MEMFILE_NAME(fclose)(psi_output_stream);
		MEMFILE_NAME(fclose)(stream);
		MEMFILE_NAME(remove)(psi_output_temporary_file_name);
	}

	// stampa il numero di errori generati dal compilatore
	// if ( compiler_errors || compiler_warnings || fatal_error )
	// 	ConsolePrintf("---------------------------------------------------------------------------");
	// ConsolePrintf("*** %i error(s), %i warning(s) ***",compiler_errors,compiler_warnings);

	// ritorna
	return;
}

//================
//WriteOutputFile
//
//Genera il file di output compilato
//================

static int Helper_InstructionPtr2PCODEAddress (instruction_t* piDest)
{
	instruction_t*		piPointer = psi_mem;
	int					iAddress = 0;

	for ( ; piPointer != piDest; piPointer++ )
	{
		if ( psi[piPointer->psi]->lenght )
		{
			iAddress += sizeof(pcode_instruction_t);
		}
	}

	return iAddress;
}

void WriteOutputFile (void)
{
	instruction_t		*psi_pointer;
	char				*code_pointer;
	int					*int_pointer;
	int					*int_auxpointer;
	char				*pointer;
	char				*aux_pointer;
	instruction_t		*initfunc_pointer;
	exprvalue_t			result;
	int					i,j,k;

	int					strings_len;
	int					data_len;
	int					tdata0_len;
	int					tdata1_len;

	pcode_instruction_t	piInstruction;

	// imposta initfunc_pointer
	initfunc_pointer=psi_ptr;

	// assembla l'ENTER della funzione di inizializzazione
	psi_ptr->psi=ENTER_WITH_NO_LOCAL_VARS;
	psi_ptr++;

	// per ognuna delle variabili dichiarate, controlla se bisogna inizializzare
	for (i=0;i<cident_id;i++) {

		// controlla se questa variabile ha bisogno di essere inizializzata
		if ( (cident[i].type & 0xff0000) == VARIABLE &&
			cident[i].initializer) {

			// analizza la stringa di inizializzazione
			aux_pointer = cident[i].initializer;
			for ( pointer = cident[i].initializer ;
				*pointer ;
				pointer ++ ) {

					// cerca il carattere \n
					if ( *pointer == '\n' ) {

						// compone la stringa di inizializzazione
						j=pointer-aux_pointer;
						if ( j >= max_statement_lenght-1 ) {
							CompilerErrorOrWarning(237);
							goto skip;
						}
						memcpy(statement,aux_pointer,j);
						statement[j]=0;

						// controlla se si tratta di un'inizializzazione di stringa
						if ( (cident[i].type & 0xff00) == CHAR &&
							( cident[i].dimensions + cident[i].indirection ) &&
							cident[i].indirection <= 1) {

							// cerca l'uguale
							for (;aux_pointer<pointer;aux_pointer++) {

								if ( (*aux_pointer >= 'a' && *aux_pointer <= 'z') ||
									(*aux_pointer >= 'A' && *aux_pointer <= 'Z') ||
									(*aux_pointer >= '0' && *aux_pointer <= '9') ||
									*aux_pointer == '_' ||
									*aux_pointer == '[' ||
									*aux_pointer == ']' )
									continue;
								else if ( *aux_pointer == '=' ) {

									// ha trovato l'uguale; calcola la len della stringa
									k=EliminateEscapeSequences(aux_pointer+2,NULL,pointer-aux_pointer-3,0);
									if ( cident[i].pointer && ((int*)cident[i].pointer)[cident[i].dimensions-1] > k ) // VPCICE PATCH // // 20MAG2004 //
										allow_special_char_assignment=k+1;
									else
										allow_special_char_assignment=k;
									break;

								}
								else
									break;

							}

						}

						// assembla l'inizializzatore
						current_line=cident[i].initializer_line;
						ResolveNonConstantExpression(&result,statement);
						allow_special_char_assignment=0;

skip:					// imposta aux_pointer
						aux_pointer=pointer+1;

					}

				}

		}

	}

	// controlla se si può continuare lo stesso
	if ( compiler_errors )
		return;

	// assembla il return della funzione di inizializzazione
	psi_ptr->psi=RETURN;
	psi_ptr++;

	// ciclo per tutte le istruzioni PSI; calcola l'indirizzo di ciascuna istruzione
	for ( psi_pointer=psi_mem,code_pointer=code;
		psi_pointer<psi_ptr;
		psi_pointer++ ) {

			// memorizza l'indirizzo di questa istruzione
			psi_pointer->my_address=code_pointer;
			code_pointer+=psi[psi_pointer->psi]->lenght;

		}

	// registra la funzione di inizializzazione; di norma la prima nella lista
	strcpy(names_ptr,o4_initialization_function_name);
	functions_ptr->name_address=names_ptr-names;
	functions_ptr->address=initfunc_pointer->my_address-code;
	functions_ptr++;
	functions2_ptr->name_address = names_ptr-names;
	functions2_ptr->address = Helper_InstructionPtr2PCODEAddress(initfunc_pointer);
	functions2_ptr++;
	names_ptr+=strlen(o4_initialization_function_name)+1;

	// ciclo per ogni identificatore; crea la lista di funzioni definite nel file sorgente
	for (i=0;i<cident_id;i++)
		if ( (cident[i].type & 0xff0000) == FUNCTION ) { // PATCH VPCICE // // 20MAG2004

			// controlla che la funzione sia stata definita, oppure se è solo un prototipo
			if (!cident[i].address)
				continue;

			// controlla se in names c'è già l'identifier di questa funzione
			for (pointer=names;pointer<names_ptr;) {
				if (!strcmp(pointer,cident[i].id))
					break;
				pointer+=strlen(pointer)+1;
			}
			if (pointer>=names_ptr) {
				pointer=names_ptr;
				strcpy(pointer,cident[i].id);
				names_ptr+=strlen(pointer)+1;
			}

			// registra l'identificatore e l'indirizzo di questa funzione
			functions_ptr->name_address=pointer-names;
			functions_ptr->address=((instruction_t*)cident[i].address)->my_address-code;
			functions_ptr++;
			functions2_ptr->name_address = pointer-names;
			functions2_ptr->address = Helper_InstructionPtr2PCODEAddress((instruction_t*)cident[i].address);
			functions2_ptr++;

			// controlla che la memoria sia ok
			if (CompilerMemoryCheckPoint())
				return;

		}

	// imposta strings_len, data_len, tdata0_len e tdata1_len
	strings_len=strings_ptr-strings;
	data_len=cur_global_var_address;
	tdata0_len=section_tempdata0_dim;
	tdata1_len=section_tempdata1_dim;

	// ciclo per tutte le istruzioni PSI; assembla il codice
	for ( psi_pointer=psi_mem;
		psi_pointer<psi_ptr;
		psi_pointer++ ) {

			// non continua se l'istruzione è fittizia (come un NOP)
			if (!psi[psi_pointer->psi]->lenght)
				continue;

			// copia il codice assembler
			memcpy( code_ptr,
				psi[psi_pointer->psi]->bytes,
				psi[psi_pointer->psi]->lenght );

			// incrementa code_ptr
			code_pointer=code_ptr;
			code_ptr+=psi[psi_pointer->psi]->lenght;

			// controlla se c'è un indirizzo
			int_pointer=psi[psi_pointer->psi]->addresses;
			if ( *int_pointer != -1 ) {

				// ciclo per ogni indirizzo dell'istruzione
				for (;*int_pointer != -1;int_pointer++) {

					// int_auxpointer è il puntatore all'indirizzo
					int_auxpointer=(int*)(code_pointer+*int_pointer);

					// l'indirizzo è un puntatore al nome della funzione esterna chiamata
					if ( psi_pointer->address.section == SECTION_NAMES ) {
						extfunctions_ptr->name_address=(char*)psi_pointer->address.address-names;
						extfunctions_ptr->address=(char*)int_auxpointer-code;
						extfunctions_ptr++;
						*int_auxpointer=0;
					}

					// l'indirizzo è un puntatore alla stringa
					else if ( psi_pointer->address.section == SECTION_STRINGS ) {
						relocation_ptr->address=(char*)int_auxpointer-code;
						relocation_ptr++;
						*int_auxpointer+=(char*)psi_pointer->address.address-strings;
					}

					// l'indirizzo è un puntatore alla stringa
					else if ( psi_pointer->address.section == SECTION_DATA &&
						(psi_pointer->address.address & 0x40000000) ) {
						relocation_ptr->address=(char*)int_auxpointer-code;
						relocation_ptr++;
						*int_auxpointer+=psi_pointer->address.address & 0xfffffff; // PATCH 4/FEB/2000 VP
					}

					// l'indirizzo è un puntatore all'area dati
					else if ( psi_pointer->address.section == SECTION_DATA ) {
						relocation_ptr->address=(char*)int_auxpointer-code;
						relocation_ptr++;
						*int_auxpointer+=psi_pointer->address.address+strings_len;
					}

					// l'indirizzo è un puntatore all'area temporanea
					else if ( psi_pointer->address.section == SECTION_TEMPDATA1 ) {
						relocation_ptr->address=(char*)int_auxpointer-code;
						relocation_ptr++;
						*int_auxpointer+=psi_pointer->address.address+strings_len+data_len+tdata0_len;
					}

					// l'indirizzo si riferisce allo stack di sistema
					else if ( psi_pointer->address.section == SECTION_STACK )
						*int_auxpointer+=psi_pointer->address.address;

					// l'indirizzo è un puntatore all'interno del codice
					else if ( psi_pointer->address.section == SECTION_PSI ) {
						*int_auxpointer=( ((instruction_t*)psi_pointer->address.address)->my_address-code ) -
							( psi_pointer->my_address - code ) - psi[psi_pointer->psi]->lenght;
					}

				}

			}

			// controlla se ci sono indirizzi temporanei
			int_pointer=psi[psi_pointer->psi]->temp_addresses;
			if ( *int_pointer != -1 ) {

				// ciclo per ogni indirizzo temporaneo dell'istruzione
				for (;*int_pointer != -1;int_pointer++) {

					// int_auxpointer è il puntatore all'indirizzo
					int_auxpointer=(int*)(code_pointer+*int_pointer);

					// imposta l'indirizzo dell'istruzione
					relocation_ptr->address=(char*)int_auxpointer-code;
					relocation_ptr++;
					*int_auxpointer+=strings_len+data_len;

				}

			}

			// controlla se c'è una costante
			int_pointer=psi[psi_pointer->psi]->constants;
			if ( *int_pointer != -1 ) {

				// ciclo per ogni costante dell'istruzione
				for (;*int_pointer != -1;int_pointer++) {

					// int_auxpointer è il puntatore alla costante
					int_auxpointer=(int*)(code_pointer+*int_pointer);

					// memorizza la costante a seconda della sua lunghezza
					if (psi[psi_pointer->psi]->constant_size==1)
						*(char*)int_auxpointer=psi_pointer->constant;
					else if (psi[psi_pointer->psi]->constant_size==2)
						*(short*)int_auxpointer=psi_pointer->constant;
					else if (psi[psi_pointer->psi]->constant_size==4)
						*int_auxpointer=psi_pointer->constant;

				}

			}

			// controlla che la memoria sia ok
			if (CompilerMemoryCheckPoint())
				return;

		}

	// imposta il magic number nell'header
	header.magic_number=o4_magic_number;
	MEMFILE_NAME(fseek)(output_stream,sizeof(header),SEEK_SET);

	// code
	header.code.offset=sizeof(header);
	header.code.size=code_ptr-code;
	MEMFILE_NAME(fwrite)(code,1,header.code.size,output_stream);

	// relocation
	header.relocation.offset=header.code.offset+header.code.size;
	header.relocation.size=(char*)relocation_ptr-(char*)relocation;
	MEMFILE_NAME(fwrite)(relocation,1,header.relocation.size,output_stream);

	// names
	header.names.offset=header.relocation.offset+header.relocation.size;
	header.names.size=names_ptr-names;
	MEMFILE_NAME(fwrite)(names,1,header.names.size,output_stream);

	// functions
	header.functions.offset=header.names.offset+header.names.size;
	header.functions.size=(char*)functions_ptr-(char*)functions;
	MEMFILE_NAME(fwrite)(functions,1,header.functions.size,output_stream);

	// extfunctions
	header.extfunctions.offset=header.functions.offset+header.functions.size;
	header.extfunctions.size=(char*)extfunctions_ptr-(char*)extfunctions;
	MEMFILE_NAME(fwrite)(extfunctions,1,header.extfunctions.size,output_stream);

	// strings
	header.strings.offset=header.extfunctions.offset+header.extfunctions.size;
	header.strings.size=strings_ptr-strings;
	MEMFILE_NAME(fwrite)(strings,1,header.strings.size,output_stream);

	// imposta mem_required
	header.mem_required=strings_len+
		data_len+
		tdata0_len+
		tdata1_len;

	// *** PCODE stuff *** INIZIO *** 23 Marzo 2001

	header.PCODE_code.offset = header.strings.offset + header.strings.size;
	header.PCODE_code.size = 0;

	// ciclo per tutte le istruzioni PSI
	for ( psi_pointer=psi_mem;
		psi_pointer<psi_ptr;
		psi_pointer++ ) {

			// non continua se l'istruzione è fittizia (come un NOP)
			if (!psi[psi_pointer->psi]->lenght)
				continue;

			// *** imposta i campi di piInstruction ***

			memset(&piInstruction, 0, sizeof(pcode_instruction_t));
			piInstruction.psi = psi_pointer->psi;

			// controlla se c'è un indirizzo
			if ( *psi[psi_pointer->psi]->addresses != -1 )
			{
				// l'indirizzo è un puntatore al nome della funzione esterna chiamata
				if ( psi_pointer->address.section == SECTION_NAMES )
				{
					piInstruction.section = PCODE_SECTION_NAMES;
					piInstruction.address = (char*)psi_pointer->address.address - names;
				}
				// l'indirizzo è un puntatore alla stringa
				else if ( psi_pointer->address.section == SECTION_STRINGS )
				{
					piInstruction.section = PCODE_SECTION_STRINGS;
					piInstruction.address = (char*)psi_pointer->address.address - strings;
				}
				// l'indirizzo è un puntatore alla stringa
				else if ( psi_pointer->address.section == SECTION_DATA &&
					(psi_pointer->address.address & 0x40000000) )
				{
					piInstruction.section = PCODE_SECTION_STRINGS;
					piInstruction.address = psi_pointer->address.address & 0xfffffff;
				}
				// l'indirizzo è un puntatore all'area dati
				else if ( psi_pointer->address.section == SECTION_DATA )
				{
					piInstruction.section = PCODE_SECTION_DATA;
					piInstruction.address = psi_pointer->address.address;
				}
				// l'indirizzo è un puntatore all'area temporanea
				else if ( psi_pointer->address.section == SECTION_TEMPDATA1 )
				{
					piInstruction.section = PCODE_SECTION_TEMPDATA;
					piInstruction.address = psi_pointer->address.address;
				}
				// l'indirizzo si riferisce allo stack di sistema
				else if ( psi_pointer->address.section == SECTION_STACK )
				{
					piInstruction.section = PCODE_SECTION_STACK;
					piInstruction.address = psi_pointer->address.address;
				}
				// l'indirizzo è un puntatore all'interno del codice
				else if ( psi_pointer->address.section == SECTION_PSI )
				{
					piInstruction.section = PCODE_SECTION_PCODE;
					piInstruction.address = Helper_InstructionPtr2PCODEAddress((instruction_t*)psi_pointer->address.address);
				}
			}

			// controlla se c'è una costante
			if ( *psi[psi_pointer->psi]->constants != -1 )
			{
				piInstruction.constant = psi_pointer->constant;
			}

			// controlla che la memoria sia ok
			if ( CompilerMemoryCheckPoint() )
			{
				return;
			}

			// scrive su disco piInstruction
			MEMFILE_NAME(fwrite)(&piInstruction, 1, sizeof(pcode_instruction_t), output_stream);
			header.PCODE_code.size += sizeof(pcode_instruction_t);

		}

	// *** si occupa di PCODE_functions... ***

	header.PCODE_functions.offset = header.PCODE_code.offset + header.PCODE_code.size;
	header.PCODE_functions.size = (char*)functions2_ptr - (char*)functions2;
	MEMFILE_NAME(fwrite)(functions2, 1, header.PCODE_functions.size, output_stream);

	// *** PCODE stuff *** FINE ***

	// scrive l'header nel file
	MEMFILE_NAME(fseek)(output_stream,0,SEEK_SET);
	MEMFILE_NAME(fwrite)(&header,1,sizeof(header),output_stream);

	// ritorna
	return;
}

//==================
//ProcessExternalDefinitions
//
//Processa le definizioni passate dall'esterno
//==================
int ProcessExternalDefinitions (void)
{
	char				*ptr=definitions;
	int					ret;
	int					i;
	char				ident[identifier_max_len];
	int					len;

	// ciclo per ogni definizione
	while (1) {

		// elimina gli eventuali spazi vuoti
		for (;*ptr;ptr++)
			if ( *ptr!=' ' &&
				*ptr!='\t' )
				break;

		// esce se la stringa delle definizioni esterne è finita
		if (!*ptr)
			break;

		// calcola il numero di caratteri di identificatore
		else if (ret=IsIdentfString(ptr)) {

			// controlla che non sia un numero
			if (*ptr>='0' && *ptr<='9') {
				CompilerErrorOrWarning(265);
				return(1);
			}

			// controlla la lunghezza dell'identificatore
			if (ret>=identifier_max_len) {
				i=identifier_max_len-1;
				CompilerErrorOrWarning(1);
			}
			else
				i=ret;
			memcpy(ident,ptr,i);
			ident[i]=0;

			// controlla se esiste già una macro con lo stesso nome
			for (i=0;i<cident_id;i++)
				if ( cident[i].type == MACRO &&
					!strcmp(ident,cident[i].id) )
					break;

			// aggiorna il puntatore ed elimina i white spaces
			ptr+=ret;
			for (;*ptr;ptr++)
				if ( *ptr!=' ' &&
					*ptr!='\t' )
						break;

			// controlla che ci sia una virgola oppure un carattere null
			if ( *ptr != ',' &&
				*ptr ) {
				CompilerErrorOrWarning(265);
				return(1);
			}
			else if ( *ptr == ',' )
				ptr++;

			// continua solo se non esiste già una definizione con lo stesso identifier
			if (i==cident_id) {

				// controlla che l'identificatore non sia una reserved word
				len=strlen(ident);
				for (i=0;reswords[i].ptr!=NULL;i++)
					if (reswords[i].len == len &&
						CompareStrings(ident,reswords[i].ptr,reswords[i].len)) {
						CompilerErrorOrWarning(265);
						return(1);
					}

				// imposta i dati sulla macro
				cident[cident_id].type=MACRO;
				strcpy(cident[cident_id].id,ident);
				cident[cident_id].pointer=cident_aux2_ptr;
				*(char*)cident_aux2_ptr=0;
				(char*)cident_aux2_ptr+=1;

				// resetta gli altri campi
				cident[cident_id].address=0;
				cident[cident_id].fields=0;
				cident[cident_id].indirection=0;
				cident[cident_id].dimensions=0;
				cident[cident_id].const_flag=0;
				cident[cident_id].static_flag=0;
				cident[cident_id].initializer=NULL;
				cident[cident_id].struct_type=NULL;
				cident[cident_id].array_stptr=0;
				cident[cident_id].ellipses_arguments=0;
				cident[cident_id].initializer_line=0;

				// incrementa cident_id
				cident_id++;

			}

		}

		// la definizione contiene caratteri non validi
		else {
			CompilerErrorOrWarning(265);
			return(1);
		}

	}

	// ritorna
	return(0);
}

//======================
//FreeCompilerMemory
//======================
void FreeCompilerMemory (void)
{
	if (console)			FREE(console);
	if (expr_tokens)		FREE(expr_tokens);
	if (identifiers)		FREE(identifiers);
	if (operators)			FREE(operators);
	if (cident)				FREE(cident);
	if (cident_aux)			FREE(cident_aux);
	if (cident_aux2)		FREE(cident_aux2);
	if (auxmem)				FREE(auxmem);
	if (rcemem)				FREE(rcemem);
	if (psi_mem)			FREE(psi_mem);
	if (code)				FREE(code);
	if (relocation)			FREE(relocation);
	if (names)				FREE(names);
	if (functions)			FREE(functions);
	if (functions2)			FREE(functions2);
	if (extfunctions)		FREE(extfunctions);
	if (strings)			FREE(strings);
	if (preproc_stack)		FREE(preproc_stack);
	if (main_stack)			FREE(main_stack);
	if (main_stack_aux_mem)	FREE(main_stack_aux_mem);
	if (labels)				FREE(labels);
	if (labels_ptrs)		FREE(labels_ptrs);
	if (psi_bounds)			FREE(psi_bounds);
	if (error_files)		FREE(error_files);
	if (dependencies_mem)	FREE(dependencies_mem);
}