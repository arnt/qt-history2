/****************************************************************************
** $Id: //depot/qt/main/src/moc/moc.y#50 $
**
** Parser and code generator for meta object compiler
**
** Authors : Eirik Eng, Haavard Nord
** Created : 930417
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file contains the parser and code generator for the meta object
** compiler (moc) of the Qt development framework.
**
** This compiler reads a C++ header file with class definitions and ouputs
** C++ code to build a meta class. The meta data includes public methods
** (not constructors, destructors or operator functions), signals and slot
** definitions. The output file should be compiled and linked into the
** target application.
**
** C++ header files are assumed to have correct syntax, and we are therefore
** doing less strict checking than C++ compilers.
**
** The C++ grammar has been adopted from the "The Annotated C++ Reference
** Manual" (ARM), by Ellis and Stroustrup (Addison Wesley, 1992).
**
** Notice that this code is not possible to compile with GNU bison, instead
** use standard AT&T yacc or Berkeley yacc.
*****************************************************************************/

%{
#include "qlist.h"
#include "qstring.h"
#include "qdatetm.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/moc/moc.y#50 $";
#endif


QString rmWS( const char * );

enum AccessPerm { _PRIVATE, _PROTECTED, _PUBLIC };


struct Argument					// single arg meta data
{
    Argument( char *left, char *right )
	{ leftType  = rmWS( left );
	  rightType = rmWS( right ); }
    QString leftType;
    QString rightType;
};

declare(QListM,Argument);

class ArgList : public QListM(Argument) {	// member function arg list
public:
    ArgList() { setAutoDelete(TRUE); }
};


struct Function					// member function meta data
{						//   used for signals and slots
    AccessPerm accessPerm;
    QString    qualifier;			// const or volatile
    QString    name;
    QString    type;
    int	       lineNo;
    ArgList   *args;
    Function() { args=0; }
   ~Function() { delete args; }
};

declare(QListM,Function);

class FuncList : public QListM(Function) {	// list of member functions
public:
    FuncList() { setAutoDelete(TRUE); }
};


ArgList *addArg( Argument * );			// add arg to tmpArgList
void	 addMember( char );			// add tmpFunc to current class

char	*strnew( const char * );		// returns a new string (copy)
char	*stradd( const char *, const char * );	// add two strings
char	*stradd( const char *, const char *,	// add three strings
			       const char * );

extern int yydebug;
bool	   lexDebug	   = FALSE;
bool	   grammarDebug	   = FALSE;
int	   lineNo;				// current line number
bool	   errorControl	   = FALSE;		// controlled errors
bool	   displayWarnings = TRUE;
bool	   skipClass;				// don't generate for class
bool	   skipFunc;				// don't generate for func
bool	   templateClass;		       	// class is a template

ArgList	  *tmpArgList;				// current argument list
Function  *tmpFunc;				// current member function
AccessPerm tmpAccessPerm;			// current access permission
AccessPerm subClassPerm;			// current access permission
bool	   Q_OBJECTdetected;			// TRUE if current class
						// contains the Q_OBJECT macro
QString	   tmpExpression;

%}


%union {
    char	char_val;
    int		int_val;
    double	double_val;
    char       *string;
    AccessPerm	access;
    Function   *function;
    ArgList    *arg_list;
    Argument   *arg;
}

%start class_defs

%token <char_val>	CHAR_VAL		/* value types */
%token <int_val>	INT_VAL
%token <double_val>	DOUBLE_VAL
%token <string>		STRING
%token <string>		IDENTIFIER		/* identifier string */

%token			FRIEND			/* declaration keywords */
%token			TYPEDEF
%token			AUTO
%token			REGISTER
%token			STATIC
%token			EXTERN
%token			INLINE
%token			VIRTUAL
%token			CONST
%token			VOLATILE
%token			CHAR
%token			SHORT
%token			INT
%token			LONG
%token			SIGNED
%token			UNSIGNED
%token			FLOAT
%token			DOUBLE
%token			VOID
%token			ENUM
%token			CLASS
%token			STRUCT
%token			UNION
%token			ASM
%token			PRIVATE
%token			PROTECTED
%token			PUBLIC
%token			OPERATOR
%token			DBL_COLON
%token			TRIPLE_DOT
%token			TEMPLATE

%token			SIGNALS
%token			SLOTS
%token			Q_OBJECT


%type  <string>		class_name
%type  <string>		template_class_name
%type  <string>		template_spec
%type  <string>		opt_base_spec
%type  <string>		base_spec
%type  <string>		base_list
%type  <string>		qt_macro_name
%type  <string>		base_specifier
%type  <access>		access_specifier
%type  <string>		fct_name
%type  <string>		type_name
%type  <string>		simple_type_names
%type  <string>		simple_type_name
%type  <string>		class_key
%type  <string>		complete_class_name
%type  <string>		qualified_class_name
%type  <string>		elaborated_type_specifier

%type  <arg_list>	argument_declaration_list
%type  <arg_list>	arg_declaration_list
%type  <arg_list>	arg_declaration_list_opt
%type  <string>		abstract_decl_opt
%type  <string>		abstract_decl
%type  <arg>		argument_declaration
%type  <string>		cv_qualifier_list_opt
%type  <string>		cv_qualifier_list
%type  <string>		cv_qualifier
%type  <string>		decl_specifiers
%type  <string>		decl_specifier
%type  <string>		decl_specs_opt
%type  <string>		decl_specs
%type  <string>		type_specifier
%type  <string>		declarator
%type  <string>		ptr_operator
%type  <string>		ptr_operators
%type  <string>		ptr_operators_opt

%%
class_defs:		  /* empty */
			| class_defs class_def
			;

class_def:				      { initClass(); }
     			  class_specifier ';' { generateClass();
						BEGIN OUTSIDE; }
                        ;


/***** r.17.1 (ARM p.387 ): Keywords	*****/

class_name:		  IDENTIFIER	      { $$ = $1; }
                        | template_class_name { $$ = $1; }
			;

template_class_name:	  IDENTIFIER '<' template_args '>'  
				   { $$ = stradd( $1, "<",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), ">" ); }
                        ;
	
/*
   template_args skips all characters until it encounters a ">" (it
   handles and discards sublevels of parentheses).  Since the rule is
   empty it must be used with care!
*/

template_args:	          /* empty */		  { initExpression();
						    templLevel = 1;
 						    BEGIN IN_TEMPL_ARGS; }
			;

/***** r.17.2 (ARM p.388): Expressions	*****/


/* const_expression skips all characters until it encounters either one
   of "]", ")" or "," (it handles and discards sublevels of parentheses).
   Since the rule is empty it must be used with care!
*/

const_expression:	   /* empty */		  { initExpression();
						    BEGIN IN_EXPR; }
			;

/***** r.17.3 (ARM p.391): Declarations *****/

decl_specifier:		  storage_class_specifier { $$ = ""; }
			| type_specifier	  { $$ = $1; }
			| fct_specifier		  { $$ = ""; }
			| FRIEND		  { skipFunc = TRUE; $$ = ""; }
			| TYPEDEF		  { skipFunc = TRUE; $$ = ""; }
			;

decl_specifiers:	  decl_specs_opt type_name decl_specs_opt
						  { char *tmp = new char[
						    strlen($1) +
						    strlen($2) +
						    strlen($3) + 3 ];
						    strcpy(tmp,$1);
						    strcat(tmp," ");
						    strcat(tmp,$2);
						    strcat(tmp," ");
						    strcat(tmp,$3);
						    $$ = tmp; }
			;

decl_specs_opt:			/* empty */	  { $$ = ""; }
			| decl_specs		  { $$ = $1; }
			;

decl_specs:		  decl_specifier	    { $$ = $1; }
			| decl_specs decl_specifier { $$ = stradd($1," ",$2); }
			;

storage_class_specifier:  AUTO
			| REGISTER
			| STATIC		{ skipFunc = TRUE; }
			| EXTERN
			;

fct_specifier:		  INLINE
			| VIRTUAL
			;

type_specifier:		  CONST			{ $$ = "const"; }
			| VOLATILE		{ $$ = "volatile"; }
			;

type_name:		  elaborated_type_specifier { $$ = $1; }
			| complete_class_name	    { $$ = $1; }
			| simple_type_names	    { $$ = $1; }
			;

simple_type_names:	  simple_type_names simple_type_name
						    { $$ = stradd($1," ",$2); }
			| simple_type_name	    { $$ = $1; }

simple_type_name:	  CHAR			    { $$ = "char"; }
			| SHORT			    { $$ = "short"; }
			| INT			    { $$ = "int"; }
			| LONG			    { $$ = "long"; }
			| SIGNED		    { $$ = "signed"; }
			| UNSIGNED		    { $$ = "unsigned"; }
			| FLOAT			    { $$ = "float"; }
			| DOUBLE		    { $$ = "double"; }
			| VOID			    { $$ = "void"; }
			;

template_spec:            TEMPLATE '<' template_args '>' 
				   { $$ = stradd( "template<",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), ">" ); }
                        ;

opt_template_spec:	  /* empty */
			| template_spec		{ templateClass = TRUE; }
			;


class_key:		  opt_template_spec CLASS	{ $$ = "class"; }
			| opt_template_spec STRUCT	{ $$ = "struct"; }
			;

complete_class_name:	  qualified_class_name	{ $$ = $1; }
			| DBL_COLON  qualified_class_name
						{ $$ = stradd( "::", $2 ); }
			;

qualified_class_name:	  qualified_class_name DBL_COLON class_name
						{ $$ = stradd( $1, "::", $3 );}
			| class_name		{ $$ = $1; }
			;

elaborated_type_specifier:
			  class_key IDENTIFIER	{ $$ = stradd($1," ",$2); }
			| ENUM IDENTIFIER	{ $$ = stradd("enum ",$2); }
			| UNION IDENTIFIER	{ $$ = stradd("union ",$2); }
			;

/***** r.17.4 (ARM p.393): Declarators	*****/

argument_declaration_list:  arg_declaration_list_opt triple_dot_opt { $$ = $1;}
			|   arg_declaration_list ',' TRIPLE_DOT	    { $$ = $1;
				       moc_warn("Ellipsis not supported"
					       " in signals and slots.\n"
					       "Ellipsis argument ignored."); }
			;

arg_declaration_list_opt:	/* empty */	{ $$ = tmpArgList; }
			| arg_declaration_list	{ $$ = $1; }
			;

triple_dot_opt:			/* empty */
			| TRIPLE_DOT { moc_warn("Ellipsis not supported"
					       " in signals and slots.\n"
					       "Ellipsis argument ignored."); }

			;

arg_declaration_list:	  arg_declaration_list
			  ','
			  argument_declaration	{ $$ = addArg($3); }
			| argument_declaration	{ $$ = addArg($1); }

argument_declaration:	  decl_specifiers abstract_decl_opt
				{ $$ = new Argument(stradd($1,$2),"");
				  CHECK_PTR($$); }
			| decl_specifiers abstract_decl_opt
			  '=' { expLevel = 1; }
			  const_expression
				{ $$ = new Argument(stradd($1,$2),"");
				  CHECK_PTR($$); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
				{ $$ = new Argument(stradd($1,$2),$4);
				  CHECK_PTR($$); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
			  '='	{ expLevel = 1; }
			  const_expression
				{ $$ = new Argument(stradd($1,$2),$4);
				  CHECK_PTR($$); }
			;


abstract_decl_opt:	  /* empty */		{ $$ = ""; }
			| abstract_decl		{ $$ = $1; }
			;

abstract_decl:		  abstract_decl ptr_operator
						{ $$ = stradd($1,$2); }
			| '['			{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd( "[",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), "]" ); }
			| abstract_decl '['	{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd( $1,"[",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(),"]" ); }
			| ptr_operator		{ $$ = $1; }
			| '(' abstract_decl ')' { $$ = $2; }
			;

declarator:		  dname			{ $$ = ""; }
			| declarator ptr_operator
						{ $$ = stradd($1,$2);}
			| declarator '['	{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd( $1,"[",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(),"]" ); }
			| '(' declarator ')'	{ $$ = $2; }
			;

dname:			  IDENTIFIER
			;

fct_decl:		  '('
			  argument_declaration_list
			  ')'
			  cv_qualifier_list_opt
			  fct_body_or_semicolon
						{ tmpFunc->args	     = $2;
						  tmpFunc->qualifier = $4; }
			;

fct_name:		  IDENTIFIER		/* NOTE: simplified! */
			| IDENTIFIER array_decls
				{ func_warn("Variable as signal or slot."); }
			| IDENTIFIER '=' { expLevel=0; }
			  const_expression     /* probably const member */
						{ skipFunc = TRUE; }
			| IDENTIFIER array_decls '=' { expLevel=0; }
			  const_expression     /* probably const member */
						{ skipFunc = TRUE; }
			;


array_decls:		 '['			{ expLevel = 1; }
			  const_expression ']'
			| array_decls '['	 { expLevel = 1; }
			  const_expression ']'

			;

ptr_operators_opt:	   /* empty */		{ $$ = ""; }
			| ptr_operators		 { $$ = $1; }
			;

ptr_operators:		  ptr_operator		{ $$ = $1; }
			| ptr_operators ptr_operator { $$ = stradd($1," ",$2);}
			;

ptr_operator:		  '*' cv_qualifier_list_opt { $$ = stradd("*"," ",$2);}
			| '&' cv_qualifier_list_opt { $$ = stradd("&"," ",$2);}
/*!			| complete_class_name
			  DBL_COLON
			  '*'
			  cv_qualifier_list_opt { $$ = stradd($1,"::*",$4); }*/
			;

cv_qualifier_list_opt:		/* empty */	{ $$ = ""; }
			| cv_qualifier_list	{ $$ = $1; }
			;

cv_qualifier_list:	  cv_qualifier		{ $$ = $1; }
			| cv_qualifier_list cv_qualifier
						{ $$ = stradd($1,$2); }
			;

cv_qualifier:		  CONST			{ $$ = "const"; }
			| VOLATILE		{ $$ = "volatile"; }
			;

fct_body_or_semicolon:	  ';'
			| fct_body
			| '=' INT_VAL ';'   /* abstract func, INT_VAL = 0 */
			;

fct_body:		  '{' {BEGIN IN_FCT; fctLevel = 1;}
			  '}' {BEGIN QT_DEF; }
			;


/***** r.17.5 (ARM p.395): Class Declarations *****/

class_specifier:	  class_head
			  '{'			{ BEGIN IN_CLASS; level = 1; }
			  opt_obj_member_list
			  '}'			{ BEGIN QT_DEF; } /*catch ';'*/
			| class_key class_name	{ BEGIN QT_DEF;	  /* -- " -- */
						  skipClass = TRUE; }
			| class_key class_name
			  '(' IDENTIFIER ')' /* Qt macro name */
						{ BEGIN QT_DEF; /* catch ';' */
						  skipClass = TRUE; }
                        | template_spec whatever { skipClass = TRUE;
                                                  BEGIN GIMME_SEMICOLON; }
			;

whatever:                 IDENTIFIER
                        | simple_type_name
                        | type_specifier
                        | storage_class_specifier
                        | fct_specifier
                        ;

class_head:               class_key
			  class_name		{ className = $2; }
			  opt_base_spec		{ superclassName = $4; }
			;

opt_base_spec:			/* empty */	{ $$ = NULL; }
			| base_spec		{ $$ = $1; }
			;

opt_obj_member_list:		/* empty */
			| obj_member_list
			;

obj_member_list:	  obj_member_list obj_member_area
			| obj_member_area
			;


qt_access_specifier:	  access_specifier	{ tmpAccessPerm = $1; }
			| SLOTS	      { moc_err( "Missing access specifier"
						   " before \"slots:\"." ); }
			;

obj_member_area:	  qt_access_specifier	{ BEGIN QT_DEF; }
			  slot_area
			| SIGNALS		{ BEGIN QT_DEF; }
			  ':'  opt_signal_declarations
			| Q_OBJECT		{ if ( tmpAccessPerm )
				moc_warn("Q_OBJECT is not in the private"
					" section of the class.\n"
					"Q_OBJECT is a macro that resets"
					" access permission to \"private\".");
						  Q_OBJECTdetected = TRUE; }
			;

slot_area:		  SIGNALS ':'	     { moc_err( "Signals cannot "
						 "have access specifiers" ); }
			| SLOTS	  ':' opt_slot_declarations
			| ':'		 { if ( grammarDebug )
						  BEGIN QT_DEF;
					      else
						  BEGIN IN_CLASS;
					   if ( level != 1 )
					       moc_warn( "unexpected ':'" );
					 }
				      opt_slot_declarations

			;

opt_signal_declarations:	/* empty */
			| signal_declarations
			;

signal_declarations:	  signal_declarations signal_declaration
			| signal_declaration
			;


signal_declaration:	  signal_or_slot	{ addMember('s'); }
			;

opt_slot_declarations:		/* empty */
			| slot_declarations
			;

slot_declarations:	  slot_declarations slot_declaration
			| slot_declaration
			;

slot_declaration:	  signal_or_slot	{ addMember('t'); }
			;

opt_semicolons:		  /* empty */
			| opt_semicolons ';'
			;

base_spec:		  ':' base_list		{ $$=$2; }
			;

base_list		: base_list ',' base_specifier
			| base_specifier
			;

qt_macro_name:		  IDENTIFIER '(' IDENTIFIER ')'
					   { $$ = stradd( $1, "(", $3, ")" ); }
			| IDENTIFIER '(' simple_type_name ')'
					   { $$ = stradd( $1, "(", $3, ")" ); }
			;

base_specifier:		  complete_class_name			       {$$=$1;}
			| VIRTUAL access_specifier complete_class_name {$$=$3;}
			| VIRTUAL complete_class_name		       {$$=$2;}
			| access_specifier VIRTUAL complete_class_name {$$=$3;}
			| access_specifier complete_class_name	       {$$=$2;}
			| qt_macro_name				       {$$=$1;}
			| VIRTUAL access_specifier qt_macro_name       {$$=$3;}
			| VIRTUAL qt_macro_name			       {$$=$2;}
			| access_specifier VIRTUAL qt_macro_name       {$$=$3;}
			| access_specifier qt_macro_name	       {$$=$2;}
			;

access_specifier:	  PRIVATE		{ $$=_PRIVATE; }
			| PROTECTED		{ $$=_PROTECTED; }
			| PUBLIC		{ $$=_PUBLIC; }
			;

operator_name:		  decl_specs_opt IDENTIFIER ptr_operators_opt
			| decl_specs_opt simple_type_name ptr_operators_opt
			| '+'
			| '-'
			| '*'
			| '/'
			| '%'
			| '^'
			| '&'
			| '|'
			| '~'
			| '!'
			| '='
			| '<'
			| '>'
			| '+' '='
			| '-' '='
			| '*' '='
			| '/' '='
			| '%' '='
			| '^' '='
			| '&' '='
			| '|' '='
			| '~' '='
			| '!' '='
			| '=' '='
			| '<' '='
			| '>' '='
			| '<' '<'
			| '>' '>'
			| '<' '<' '='
			| '>' '>' '='
			| '&' '&'
			| '|' '|'
			| '+' '+'
			| '-' '-'
			| ','
			| '-' '>' '*'
			| '-' '>'
			| '(' ')'
			| '[' ']'
			;


opt_virtual:		  /* empty */
			| VIRTUAL
			;

type_and_name:		  type_name fct_name
						{ tmpFunc->type = $1;
						  tmpFunc->name = $2; }
			| fct_name
						{ tmpFunc->type = "int";
						  tmpFunc->name = $1;
				  if ( tmpFunc->name == className )
				      func_warn( "Constructors cannot be"
						 " signals or slots.");
						}
			| opt_virtual '~' fct_name
						{
				       func_warn( "Destructors cannot be"
						  " signals or slots.");
						}
			| decl_specs type_name decl_specs_opt
			  ptr_operators_opt fct_name
						{ tmpFunc->type =
						      stradd($1,$2,$3,$4);
						  tmpFunc->name = $5; }
			| decl_specs type_name /* probably friend decl */
						{ skipFunc = TRUE; }
			| type_name ptr_operators fct_name
						{ tmpFunc->type =
						      stradd($1,$2);
						  tmpFunc->name = $3; }
			| type_name decl_specs ptr_operators_opt
			  fct_name
						{ tmpFunc->type =
						      stradd($1,$2,$3);
						  tmpFunc->name = $4; }
			| type_name OPERATOR operator_name
						{ operatorError();    }
			| OPERATOR operator_name
						{ operatorError();    }
			| decl_specs type_name decl_specs_opt
			  ptr_operators_opt OPERATOR operator_name
						{ operatorError();    }
			| type_name ptr_operators OPERATOR  operator_name
						{ operatorError();    }
			| type_name decl_specs ptr_operators_opt
			  OPERATOR  operator_name
						{ operatorError();    }
			;

signal_or_slot:		  type_and_name fct_decl opt_semicolons
			| type_and_name opt_bitfield ';' opt_semicolons
				{ func_warn("Variable as signal or slot."); }
			| type_and_name opt_bitfield ','member_declarator_list
			  ';' opt_semicolons
				{ func_warn("Variable as signal or slot."); }
			| enum_specifier  ';' opt_semicolons
				{ func_warn("Enum declaration as signal or"
					  " slot."); }
			;


member_declarator_list:	  member_declarator
			| member_declarator_list ',' member_declarator
			;

member_declarator:	  declarator
			| IDENTIFIER ':'	 { expLevel = 0; }
			  const_expression
			| ':'			 { expLevel = 0; }
			  const_expression
			;

opt_bitfield:		  /* empty */
			| ':'			 { expLevel = 0; }
			  const_expression
			;


enum_specifier:		  ENUM opt_identifier '{'   {BEGIN IN_FCT; fctLevel=1;}
					      '}'   {BEGIN QT_DEF; }
			;

opt_identifier:		  /* empty */
			| IDENTIFIER
			;

%%

#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
extern "C" int read( int, void *, uint );
#if defined(_CC_MSC_)
extern "C" int fileno( FILE *stream );
#endif
int yydebug;
#include "lexyy.c"
#else
#include "lex.yy.c"
#endif

void init();					// initialize
void initClass();				// prepare for new class
void generateClass();				// generate C++ code for class
void initExpression();				// prepare for new expression

QString	  fileName;				// file name
QString	  ofileName;				// output file name
bool	  noInclude = FALSE;			// no #include <filename>
QString	  className;				// name of parsed class
QString	  superclassName;			// name of super class
FuncList  signals;				// signal interface
FuncList  slots;				// slots interface

FILE  *out;					// output file

int yyparse();


int main( int argc, char **argv )		// program starts here
{
    bool autoInclude = TRUE;
    char *error	     = 0;
    for ( int n=1; n<argc; n++ ) {
	QString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    char *opt = &arg[1];
	    if ( *opt == 'o' )	{		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    ofileName = argv[++n];
		}
		else
		    ofileName = &arg[2];
	    }
	    else
	    if ( strcmp(opt,"i") == 0 ) {	// no #include statement
		noInclude   = TRUE;
		autoInclude = FALSE;
	    } else
	    if ( strcmp(opt,"f") == 0 ) {	// produce #include statement
		noInclude   = FALSE;
		autoInclude = FALSE;
	    } else
	    if ( strcmp(opt,"k") == 0 ) {	// don't stop on errors
		errorControl = TRUE;
	    } else
	    if ( strcmp(opt,"nw") == 0 ) {	// don't display warnings
		displayWarnings = FALSE;
	    } else
	    if ( strcmp(opt,"ldbg") == 0 ) {	// lex debug output
		lexDebug = TRUE;
	    } else
	    if ( strcmp(opt,"ydbg") == 0 ) {	// yacc debug output
		yydebug = TRUE;
	    } else
	    if ( strcmp(opt,"dbg") == 0 ) {	// non-signal members are slots
		grammarDebug = TRUE;
	    } else {
		error = "Invalid argument";
	    }
	} else {
	    if ( !fileName.isNull() )		// can handle only one file
		error	 = "Too many input files specified";
	    else
		fileName = arg.copy();
	}
    }
    if ( autoInclude ) {
	int ppos = fileName.find('.');
	if ( ppos != -1 && tolower( fileName[ppos + 1] ) == 'h' )
	    noInclude = FALSE;
	else
	    noInclude = TRUE;
    }
    if ( fileName.isNull() ) {
	fileName = "standard input";
	yyin	 = stdin;
    }
    else if ( argc < 2 || error ) {		// incomplete/wrong args
	fprintf( stderr, "Qt meta object compiler\n" );
	if ( error )
	    fprintf( stderr, "moc: %s\n", error );
	fprintf( stderr, "Usage:  moc [-i] [-o output] <header-file>\n" );
	return 1;
    }
    else {
	yyin = fopen( fileName, "r" );
	if ( !yyin ) {
	    fprintf( stderr, "moc: %s: No such file\n", (char*)fileName );
	    return 1;
	}
    }
    if ( !ofileName.isNull() ) {		// output file specified
	out = fopen( ofileName, "w" );		// create output file
	if ( !out ) {
	    fprintf( stderr, "moc: Cannot create %s\n", (char*)ofileName );
	    return 1;
	}
    }
    else					// use stdout
	out = stdout;
    init();
    yyparse();
    fclose( yyin );
    if ( !ofileName.isNull() )
	fclose( out );

    slots.clear();
    signals.clear();
    return 0;
}


extern char *getenv();
extern char *getenv( const char * );

char *getenv() { return 0; }
char *getenv( const char * ) { return 0; }


void init()					// initialize
{
    BEGIN OUTSIDE;
    lineNo	 = 1;
    skipClass	 = FALSE;
    skipFunc	 = FALSE;
    signals.setAutoDelete( TRUE );
    slots.setAutoDelete( TRUE );

    tmpArgList	 = new ArgList;
    CHECK_PTR( tmpArgList );
    tmpFunc	 = new Function;
    CHECK_PTR( tmpFunc );
}

void initClass()				 // prepare for new class
{
    tmpAccessPerm    = _PRIVATE;
    subClassPerm     = _PRIVATE;
    Q_OBJECTdetected = FALSE;
    skipClass	     = FALSE;
    templateClass    = FALSE;
    slots.clear();
    signals.clear();
}

/* Remove white space from SIGNAL and SLOT names */

QString rmWS( const char *src )
{
    QString tmp( strlen( src ) + 1 );
    register char *d = tmp.data();
    register char *s = (char *)src;
    while( *s && isspace(*s) )
	s++;
    while ( *s ) {
	while( *s && !isspace(*s) )
	    *d++ = *s++;
	while( *s && isspace(*s) )
	    s++;
	if ( *s && ( isalpha(*s) || *s == '_' ) )
	    *d++ = ' ';
    }
    tmp.truncate( d - tmp.data() );
    return tmp;
}

void initExpression()
{
    tmpExpression = "";
}

void addExpressionString( char *s )
{
    tmpExpression += s;
}

void addExpressionChar( char c )
{
    tmpExpression += c;
}

void yyerror( char *msg )			// print yacc error message
{

    fprintf( stderr, "%s:%d: Error: %s\n", fileName.data(), lineNo, msg);
/*
    if ( errorControl )
	fprintf( stderr, "%s:%d: Warning: %s\n", fileName.data(), lineNo, msg);
    else
	fprintf( stderr, "%s:%d: Error: %s\n", fileName.data(), lineNo, msg );
*/
}

void moc_err( char *s )
{
    yyerror( s );
    if ( errorControl )
	exit( -1 );
}

void moc_err( char *s1, char *s2 )
{
    static char tmp[1024];
    sprintf( tmp, s1, s2 );
    yyerror( tmp );
    if ( errorControl )
	exit( -1 );
}

void moc_warn( char *msg )
{
    if ( displayWarnings )
    fprintf( stderr, "%s:%d: Warning: %s\n", fileName.data(), lineNo, msg );
}

void func_warn( char *msg )
{
    moc_warn( msg );
    skipFunc = TRUE;
}

void operatorError()
{
    moc_warn("Operator functions cannot be signals or slots.");
    skipFunc = TRUE;
}

#ifndef yywrap
int yywrap()					// more files?
{
    return 1;					// end of file
}
#endif

char *stradd( const char *s1, const char *s2 )	// adds two strings
{
    char *n = new char[strlen(s1)+strlen(s2)+1];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, s2 );
    return n;
}

char *stradd( const char *s1, const char *s2, const char *s3 )// adds 3 strings
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+1];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, s2 );
    strcat( n, s3 );
    return n;
}

char *stradd( const char *s1, const char *s2,
	      const char *s3, const char *s4 )// adds 4 strings
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+1];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, s2 );
    strcat( n, s3 );
    strcat( n, s4 );
    return n;
}


// Generate C++ code for building member function table

const int Slot_Num   = 1;
const int Signal_Num = 2;

void generateFuncs( FuncList *list, char *functype, int num )
{
    Function *f;
    for ( f=list->first(); f; f=list->next() ) {
	QString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( count++ )
		typstr += ",";
	    typstr += a->leftType;
	    typstr += a->rightType;
	    a = f->args->next();
	}
	fprintf( out, "    typedef %s(%s::*m%d_t%d)(%s);\n",
		 (char*)f->type,
		 (char*)className, num, list->at(),(char*)typstr );
	f->type = f->name.copy();
	f->type += "(";
	f->type += typstr;
	f->type += ")";
    }
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    m%d_t%d v%d_%d = &%s::%s;\n", num, list->at(),
		 num, list->at(), (char*)className, (char*)f->name);
    if ( list->count() )
	fprintf( out, "    QMetaData *%s_tbl = new QMetaData[%d];\n",
		 functype, list->count() );
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    %s_tbl[%d].name = \"%s\";\n",
		 functype, list->at(), (char*)f->type );
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    %s_tbl[%d].ptr = *((QMember*)&v%d_%d);\n",
		 functype, list->at(), num, list->at() );
}

void generateClass()		      // generate C++ source code for a class
{
    char *hdr1 = "/****************************************************************************\n"
		 "** %s meta object code from reading C++ file '%s'\n**\n";
    char *hdr2 = "** Created: %s\n"
		 "**      by: The Qt Meta Object Compiler (moc)\n**\n";
    char *hdr3 = "** WARNING! All changes made in this file will be lost!\n";
    char *hdr4 = "*****************************************************************************/\n\n";
    static int gen_count = 0;
    if ( skipClass )				// don't generate for class
	return;
    if ( !Q_OBJECTdetected ) {
	if ( signals.count() == 0 && slots.count() == 0 )
	    return;
	if ( displayWarnings )
	    moc_err("The declaration of the class \"%s\" contains slots "
		    "and/or signals\n\t but no Q_OBJECT macro!", className.data());
    } else {
	if ( !superclassName )
	    moc_err("The declaration of the class \"%s\" contains the\n"
		    "\tQ_OBJECT macro but does not inherit from any class!\n"
		    "\tInherit from QObject or one of its descendants"
		    " or remove Q_OBJECT. ", className.data() );
    }
    if ( templateClass ) {			// don't generate for class
	moc_err("Sorry, Qt does not support templates that contain\n
                 signals, slots or Q_OBJECT. This will be supported soon.");
	return;
    }
    if ( gen_count++ == 0 ) {			// first class to be generated
	QDateTime dt = QDateTime::currentDateTime();
	QString dstr = dt.toString();
	QString fn = fileName;
	int i = fileName.length()-1;
	while ( i>0 && fileName[i-1] != '/' && fileName[i-1] != '\\' )
	    i--;				// skip path
	if ( i >= 0 )
	    fn = &fileName[i];
	fprintf( out, hdr1, (char*)className, (char*)fn );
	fprintf( out, hdr2, (char*)dstr );
	fprintf( out, hdr3 );
	fprintf( out, hdr4 );
	fprintf( out, "#include \"qmetaobj.h\"\n" );
	if ( !noInclude )
	    fprintf( out, "#include \"%s\"\n", (char*)fileName );
	fprintf( out, "\n\n" );
    }
    else
	fprintf( out, "\n\n" );

//
// Generate virtual function className()
//
    fprintf( out, "const char *%s::className() const\n{\n    ",
	     (char*)className );
    fprintf( out, "return \"%s\";\n}\n\n", (char*)className );

//
// Generate static metaObj variable
//
    fprintf( out, "QMetaObject *%s::metaObj = 0;\n\n", (char*)className );

//
// Generate initMetaObject member function
//
    fprintf( out, "void %s::initMetaObject()\n{\n", (char*)className );
    fprintf( out, "    if ( metaObj )\n\treturn;\n" );
    fprintf( out, "    if ( !%s::metaObject() )\n\t%s::initMetaObject();\n",
	     (char*)superclassName, (char*)superclassName );
//
// Build slots array in initMetaObject()
//
    generateFuncs( &slots, "slot", Slot_Num );

//
// Build signals array in initMetaObject()
//
    generateFuncs( &signals, "signal", Signal_Num );

//
// Finally code to create meta object
//
    fprintf( out, "    metaObj = new QMetaObject( \"%s\", \"%s\",\n",
	     (char*)className, (char*)superclassName );
    if ( slots.count() )
	fprintf( out, "\tslot_tbl, %d,\n", slots.count() );
    else
	fprintf( out, "\t0, 0,\n" );
    if ( signals.count() )
	fprintf( out, "\tsignal_tbl, %d );\n", signals.count());
    else
	fprintf( out, "\t0, 0 );\n" );
    fprintf( out, "}\n" );

//
// End of function initMetaObject()
//

//
// Generate internal signal functions
//'
    Function *f;
    f = signals.first();			// make internal signal methods
    static bool included_list_stuff = FALSE;
    while ( f ) {
	QString typstr = "";			// type string
	QString valstr = "";			// value string
	QString argstr = "";			// argument string (type+value)
	int  count     = 0;
	char buf[12];
						// method header
	Argument *a = f->args->first();
	while ( a ) {				// argument list
	    if ( count++ ) {
		typstr += ",";
		valstr += ", ";
		argstr += ", ";
	    }
	    typstr += a->leftType;
	    typstr += a->rightType;
	    argstr += a->leftType;
	    argstr += " ";
	    sprintf( buf, "t%d", count );
	    valstr += buf;
	    argstr += buf;
	    argstr += a->rightType;
	    a = f->args->next();
	}

	bool predef_call = FALSE;
	if ( typstr.isEmpty() || typstr == "short" || typstr == "int" ||
	     typstr == "long" || typstr == "char*" || typstr == "const char*"){
	    predef_call = TRUE;
	}
	if ( !predef_call && !included_list_stuff ) {
	    fprintf( out, "\n#include <qlist.h>\n" );
	    fprintf( out, "declare(QListM,QConnection);\n" );
	    fprintf( out, "declare(QListIteratorM,QConnection);\n" );
	    included_list_stuff = TRUE;
	}

	fprintf( out, "\n// SIGNAL %s\n", (char*)f->name );
	fprintf( out, "void %s::%s(", (char*)className, (char*)f->name );

	if ( argstr.isEmpty() )
	    fprintf( out, ")\n{\n" );
	else
	    fprintf( out, " %s )\n{\n", (char*)argstr );

	if ( predef_call ) {
	    fprintf( out, "    activate_signal( \"%s(%s)\"",
		     (char*)f->name, (char*)typstr );
	    if ( !valstr.isEmpty() )
		fprintf( out, ", %s", (char *)valstr );
	    fprintf( out, " );\n}\n" );
	    f = signals.next();
	    continue;
	}

	fprintf( out, "    QConnectionList *clist = receivers(\"%s(%s)\");\n",
		 (char*)f->name, (char*)typstr );
	fprintf( out, "    if ( !clist || signalsBlocked() )\n\treturn;\n" );
	fprintf( out, "    typedef void (QObject::*RT)(%s);\n",(char*)typstr);
	fprintf( out, "    typedef RT *PRT;\n" );
	fprintf( out, "    QConnectionListIt it(*clist);\n" );
	fprintf( out, "    register QConnection *c;\n" );
	fprintf( out, "    RT r;\n" );
	fprintf( out, "    QSenderObject *object;\n" );
	fprintf( out, "    while ( (c=it.current()) ) {\n" );
	fprintf( out, "\t++it;\n" );
	fprintf( out, "\tr = *((PRT)(c->member()));\n" );
	fprintf( out, "\tobject = (QSenderObject*)c->object();\n" );
	fprintf( out, "\tobject->setSender( this );\n" );
	fprintf( out, "\t(object->*r)(%s);\n", (char*)valstr );
	fprintf( out, "\tobject->setSender( 0 );\n" );
	fprintf( out, "    }\n}\n" );
	f = signals.next();
    }

}


ArgList *addArg( Argument *a )			// add argument to list
{
    tmpArgList->append( a );
    return tmpArgList;
}

void addMember( char m )
{
    if ( skipFunc ) {
	tmpArgList  = new ArgList;   // ugly but works!
	CHECK_PTR( tmpArgList );
	tmpFunc	    = new Function;
	CHECK_PTR( tmpFunc );
	skipFunc    = FALSE;
	return;
    }
    if ( m == 's' && tmpFunc->type != "void" ) {
	moc_err( "Signals must have \"void\" as their return type" );
	return;
    }
    tmpFunc->accessPerm = tmpAccessPerm;
    tmpFunc->args	= tmpArgList;
    tmpFunc->lineNo	= lineNo;
    tmpArgList		= new ArgList;
    CHECK_PTR( tmpArgList );
    switch( m ) {
	case 's': signals.append( tmpFunc ); break;
	case 't': slots.  append( tmpFunc ); break;
    }
    skipFunc = FALSE;
    tmpFunc  = new Function;
    CHECK_PTR( tmpFunc );
}
