/****************************************************************************
** $Id: //depot/qt/main/src/emoc/moc.y#8 $
**
** Parser and code generator for meta object compiler
**
** Created : 930417
**
** Copyright (C) 1993-1999 by Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
** --------------------------------------------------------------------------
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
void yyerror( char *msg );

#include "qlist.h"
#include "qdict.h"
#include "qstringlist.h"
#include "qstring.h"
#include "qdatetime.h"
#include "qfile.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


static QCString rmWS( const char * );

enum AccessPerm { _PRIVATE, _PROTECTED, _PUBLIC };


struct Argument					// single arg meta data
{
    Argument( char *left, char *right )
	{ leftType  = rmWS( left );
	  rightType = rmWS( right );
	  if ( leftType == "void" && rightType.isEmpty() )
	      leftType = "";
	}
    QCString leftType;
    QCString rightType;
};

class ArgList : public QList<Argument> {	// member function arg list
public:
    ArgList() { setAutoDelete(TRUE); }
   ~ArgList() { clear(); }
};


struct Function					// member function meta data
{						//   used for signals and slots
    AccessPerm accessPerm;
    QCString    qualifier;			// const or volatile
    QCString    name;
    QCString    type;
    int	       lineNo;
    ArgList   *args;
    Function() { args=0; }
   ~Function() { delete args; }
};

class FuncList : public QList<Function> {	// list of member functions
public:
    FuncList() { setAutoDelete(TRUE); }
};

class Enum : public QStringList
{
public:
    QCString name;
};

class EnumList : public QList<Enum> {		// list of property enums
public:
    EnumList() { setAutoDelete(TRUE); }
};

ArgList *addArg( Argument * );			// add arg to tmpArgList
void	 addMember( char );			// add tmpFunc to current class
void     addEnum();				// add tmpEnum to current class

char	*strnew( const char * );		// returns a new string (copy)
char	*stradd( const char *, const char * );	// add two strings
char	*stradd( const char *, const char *,	// add three strings
			       const char * );
char	*straddSpc( const char *, const char * );
char	*straddSpc( const char *, const char *,
			       const char * );
char	*straddSpc( const char *, const char *,
		    const char *, const char * );

extern int yydebug;
bool	   lexDebug	   = FALSE;
bool	   grammarDebug	   = FALSE;
int	   lineNo;				// current line number
bool	   errorControl	   = FALSE;		// controlled errors
bool	   displayWarnings = TRUE;
bool	   skipClass;				// don't generate for class
bool	   skipFunc;				// don't generate for func
bool	   templateClass;			// class is a template

ArgList	  *tmpArgList;				// current argument list
Function  *tmpFunc;				// current member function
Enum      *tmpEnum;				// current enum
AccessPerm tmpAccessPerm;			// current access permission
AccessPerm subClassPerm;			// current access permission
bool	   Q_OBJECTdetected;			// TRUE if current class
						// contains the Q_OBJECT macro
bool	   Q_BUILDERdetected;			// TRUE if current class
						// contains the Q_BUILDER macro
bool	   Q_CUSTOM_FACTORYdetected;			// TRUE if current class
						// contains the Q_CUSTOM_FACTORY macro
QCString   Q_BUILDERcomment;			// Comment to show in the builder ( maybe empty )
QCString   Q_BUILDERpixmap;			// Pixmap to show in the builder ( maybe empty )
bool	   Q_INSPECTORdetected;			// TRUE if current class
						// contains the Q_INSPECTOR macro
QCString   Q_INSPECTORclass;			// Class for which this inspector should be used

QCString   tmpExpression;

const int  formatRevision = 4;			// moc output format revision

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

%start declaration_seq

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
%token			NAMESPACE
%token			USING
%token			MUTABLE

%token			QPROP
%token			QPROPERTIES
%token			SIGNALS
%token			SLOTS
%token			Q_OBJECT
%token			Q_BUILDER
%token			Q_INSPECTOR
%token			Q_CUSTOM_FACTORY

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
declaration_seq:	  /* empty */
                        | declaration_seq declaration
			;

declaration:		  class_def
/* | template_declaration */
                        | namespace_def
                        | namespace_alias_def
                        | using_declaration
                        | using_directive
                        ;

namespace_def:            named_namespace_def
                        | unnamed_namespace_def
                        ;

named_namespace_def:      NAMESPACE
                          IDENTIFIER         { enterNameSpace($2); }
                          '{'                { BEGIN IN_NAMESPACE; }
                          namespace_body
                          '}'                { leaveNameSpace();
			                       selectOutsideClassState();
                                             }
                        ;

unnamed_namespace_def:    NAMESPACE          { enterNameSpace(); }
                          '{'                { BEGIN IN_NAMESPACE; }
                          namespace_body
                          '}'                { leaveNameSpace();
  			                       selectOutsideClassState();
			                     }
                        ;

namespace_body:           declaration_seq
                        ;

namespace_alias_def:      NAMESPACE IDENTIFIER '=' complete_class_name ';'
                                            { selectOutsideClassState(); }
                        ;


using_directive:          USING NAMESPACE   { selectOutsideClassState(); } /* Skip namespace */
                        ;

using_declaration:        USING IDENTIFIER  { selectOutsideClassState(); }
                        | USING DBL_COLON   { selectOutsideClassState(); }
                        ;

class_def:				      { initClass(); }
			  class_specifier ';' { generateClass();
			                        registerClassInNamespace();
						selectOutsideClassState(); }
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

template_args:		  /* empty */		  { initExpression();
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

enumerator_expression:	   /* empty */		  { initExpression();
						    BEGIN IN_ENUM; }
			;

/***** r.17.3 (ARM p.391): Declarations *****/

decl_specifier:		  storage_class_specifier { $$ = ""; }
			| type_specifier	  { $$ = $1; }
			| fct_specifier		  { $$ = ""; }
			| FRIEND		  { skipFunc = TRUE; $$ = ""; }
			| TYPEDEF		  { skipFunc = TRUE; $$ = ""; }
			;

decl_specifiers:	  decl_specs_opt type_name decl_specs_opt
						  { $$ = straddSpc($1,$2,$3); }
			;

decl_specs_opt:			/* empty */	  { $$ = ""; }
			| decl_specs		  { $$ = $1; }
			;

decl_specs:		  decl_specifier	    { $$ = $1; }
			| decl_specs decl_specifier { $$ = straddSpc($1,$2); }
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
						    { $$ = straddSpc($1,$2); }
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

template_spec:		  TEMPLATE '<' template_args '>'
				   { $$ = stradd( "template<",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), ">" ); }
			;

opt_template_spec:	  /* empty */
			| template_spec		{ templateClass = TRUE; }
			;


class_key:		  opt_template_spec CLASS { $$ = "class"; }
			| opt_template_spec STRUCT { $$ = "struct"; }
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
			  class_key IDENTIFIER	{ $$ = straddSpc($1,$2); }
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
				{ $$ = new Argument(straddSpc($1,$2),"");
				  CHECK_PTR($$); }
			| decl_specifiers abstract_decl_opt
			  '=' { expLevel = 1; }
			  const_expression
				{ $$ = new Argument(straddSpc($1,$2),"");
				  CHECK_PTR($$); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
				{ $$ = new Argument(straddSpc($1,$2),$4);
				  CHECK_PTR($$); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
			  '='	{ expLevel = 1; }
			  const_expression
				{ $$ = new Argument(straddSpc($1,$2),$4);
				  CHECK_PTR($$); }
			;


abstract_decl_opt:	  /* empty */		{ $$ = ""; }
			| abstract_decl		{ $$ = $1; }
			;

abstract_decl:		  abstract_decl ptr_operator
						{ $$ = straddSpc($1,$2); }
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
						{ $$ = straddSpc($1,$2);}
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
			| ptr_operators ptr_operator { $$ = straddSpc($1,$2);}
			;

ptr_operator:		  '*' cv_qualifier_list_opt { $$ = straddSpc("*",$2);}
			| '&' cv_qualifier_list_opt { $$ = stradd("&",$2);}
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
						{ $$ = straddSpc($1,$2); }
			;

cv_qualifier:		  CONST			{ $$ = "const"; }
			| VOLATILE		{ $$ = "volatile"; }
			| QPROP			{ $$ = "q_prop"; }
			;

fct_body_or_semicolon:	  ';'
			| fct_body
			| '=' INT_VAL ';'   /* abstract func, INT_VAL = 0 */
			;

fct_body:		  '{' {BEGIN IN_FCT; fctLevel = 1;}
			  '}' {BEGIN QT_DEF; }
			;


/***** r.17.5 (ARM p.395): Class Declarations *****/

class_specifier:	  full_class_head
			  '{'			{ BEGIN IN_CLASS;
                                                  classPLevel = 1; }
			  opt_obj_member_list
			  '}'			{ BEGIN QT_DEF; } /*catch ';'*/
			| class_head		{ BEGIN QT_DEF;	  /* -- " -- */
						  skipClass = TRUE; }
			| class_head
			  '(' IDENTIFIER ')' /* Qt macro name */
						{ BEGIN QT_DEF; /* catch ';' */
						  skipClass = TRUE; }
			| template_spec whatever { skipClass = TRUE;
						  BEGIN GIMME_SEMICOLON; }
			;

whatever:		  IDENTIFIER
			| simple_type_name
			| type_specifier
			| storage_class_specifier
			| fct_specifier
			;


class_head:		  class_key
			  qualified_class_name	{ className = $2; }
			| class_key
			  IDENTIFIER		/* possible DLL EXPORT macro */
			  class_name		{ className = $3; }
			;

full_class_head:	  class_head
			  opt_base_spec		{ superclassName = $2; }
			;

opt_base_spec:			/* empty */	{ $$ = 0; }
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
			| QPROPERTIES		{ BEGIN QT_DEF; }
			  ':'  opt_qprop_declarations
			| Q_OBJECT		{ if ( tmpAccessPerm )
				moc_warn("Q_OBJECT is not in the private"
					" section of the class.\n"
					"Q_OBJECT is a macro that resets"
					" access permission to \"private\".");
						  Q_OBJECTdetected = TRUE; }
			| Q_CUSTOM_FACTORY		{ if ( tmpAccessPerm )
				moc_warn("Q_CUSTOM_FACTORY is not in the private"
					" section of the class.\n"
					"Q_CUSTOM_FACTORY is a macro that resets"
					" access permission to \"private\".");
						  Q_CUSTOM_FACTORYdetected = TRUE; }
			| Q_BUILDER { BEGIN IN_BUILDER; }
			  '(' STRING ',' STRING ')'
				{ if ( tmpAccessPerm )
				  	moc_warn("Q_BUILDER is not in the private"
						 " section of the class.\n"
						 "Q_BUILDER is a macro that resets"
						 " access permission to \"private\".");
				  Q_BUILDERdetected = TRUE; Q_BUILDERcomment = $4;
				  Q_BUILDERpixmap = $6; }
			| Q_INSPECTOR { BEGIN IN_BUILDER; }
			  '(' STRING ')'
				{ if ( tmpAccessPerm )
				  	moc_warn("Q_INSPECTOR is not in the private"
						 " section of the class.\n"
						 "Q_INSPECTOR is a macro that resets"
						 " access permission to \"private\".");
				  Q_INSPECTORdetected = TRUE; Q_INSPECTORclass = $4;
				  if ( Q_INSPECTORclass.isEmpty() )
				      moc_warn("No parameter to Q_INSPECTOR macro.\n" ); }
			;

slot_area:		  SIGNALS ':'	     { moc_err( "Signals cannot "
						 "have access specifiers" ); }
			| SLOTS	  ':' opt_slot_declarations
			| ':'		 { if ( grammarDebug )
						  BEGIN QT_DEF;
					      else
						  BEGIN IN_CLASS;
					 }
				      opt_slot_declarations
			| IDENTIFIER	 { BEGIN IN_CLASS;
					   if ( classPLevel != 1 )
					       moc_warn( "unexpected access"
							 "specifier" );
					 }
			;

opt_qprop_declarations:	/* empty */
			| qprop_declarations
			;

qprop_declarations:	  qprop_declarations qprop_declaration
			| qprop_declaration
			;


qprop_declaration:	  qprop	{ /* It could have been a template */
                                  if ( !tmpFunc->name.isEmpty() ) addMember('p');
                                  else if ( !tmpEnum->name.isEmpty() ) addEnum(); }

opt_signal_declarations:	/* empty */
			| signal_declarations
			;

signal_declarations:	  signal_declarations signal_declaration
			| signal_declaration
			;


signal_declaration:	  qsignal	{ addMember('s'); }
			;

opt_slot_declarations:		/* empty */
			| slot_declarations
			;

slot_declarations:	  slot_declarations slot_declaration
			| slot_declaration
			;

slot_declaration:	  qslot	{ addMember('t'); }
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
						{
						    char *tmp =
							straddSpc($1,$2,$3,$4);
						    tmpFunc->type = rmWS(tmp);
						    delete tmp;
						    tmpFunc->name = $5; }
			| decl_specs type_name /* probably friend decl */
						{ skipFunc = TRUE; }
			| type_name ptr_operators fct_name
						{ tmpFunc->type =
						      straddSpc($1,$2);
						  tmpFunc->name = $3; }
			| type_name decl_specs ptr_operators_opt
			  fct_name
						{ tmpFunc->type =
						      straddSpc($1,$2,$3);
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

qprop:			  type_and_name fct_decl opt_semicolons
			| type_and_name opt_bitfield ';' opt_semicolons
				{ func_warn("Variable as property access function."); }
			| type_and_name opt_bitfield ','member_declarator_list
			  ';' opt_semicolons
				{ func_warn("Variable as property access function."); }
			| prop_enum_specifier  ';' opt_semicolons
                        | USING complete_class_name ';' opt_semicolons
                                { func_warn("Using declaration as signal or"
					    " slot."); }
			| USING NAMESPACE complete_class_name ';' opt_semicolons
                                { func_warn("Using declaration as signal or"
					    " slot."); }
			| NAMESPACE IDENTIFIER '{'
                                { classPLevel++;
				  moc_err("Namespace declaration as signal or"
					  " slot."); }
			;

qslot:			  type_and_name fct_decl opt_semicolons
			| type_and_name opt_bitfield ';' opt_semicolons
				{ func_warn("Variable as signal or slot."); }
			| type_and_name opt_bitfield ','member_declarator_list
			  ';' opt_semicolons
				{ func_warn("Variable as signal or slot."); }
			| enum_specifier  ';' opt_semicolons
				{ func_warn("Enum declaration as signal or"
					  " slot."); }
                        | USING complete_class_name ';' opt_semicolons
                                { func_warn("Using declaration as signal or"
					    " slot."); }
			| USING NAMESPACE complete_class_name ';' opt_semicolons
                                { func_warn("Using declaration as signal or"
					    " slot."); }
			| NAMESPACE IDENTIFIER '{'
                                { classPLevel++;
				  moc_err("Namespace declaration as signal or"
					  " slot."); }
			;

qsignal:		  type_and_name fct_decl opt_semicolons
				{ if ( tmpFunc->qualifier.find("q_prop") != -1 )
				  func_warn("Signal as property function."); }
			| type_and_name opt_bitfield ';' opt_semicolons
				{ func_warn("Variable as signal or slot."); }
			| type_and_name opt_bitfield ','member_declarator_list
			  ';' opt_semicolons
				{ func_warn("Variable as signal or slot."); }
			| enum_specifier  ';' opt_semicolons
				{ func_warn("Enum declaration as signal or"
					  " slot."); }
                        | USING complete_class_name ';' opt_semicolons
                                { func_warn("Using declaration as signal or"
					    " slot."); }
			| USING NAMESPACE complete_class_name ';' opt_semicolons
                                { func_warn("Using declaration as signal or"
					    " slot."); }
			| NAMESPACE IDENTIFIER '{'
                                { classPLevel++;
				  moc_err("Namespace declaration as signal or"
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

prop_enum_specifier:	  ENUM IDENTIFIER '{'   prop_enum_list
				          '}'   {BEGIN QT_DEF; tmpEnum->name = $2; }
			| ENUM  '{'   {BEGIN IN_FCT; fctLevel=1;}
                                '}'   {BEGIN QT_DEF; func_warn("No anonymous enums allowed"
							       "in properties section"); }
			;

prop_enum_list:		  /* empty */
			| prop_enumerator
			| prop_enum_list ',' prop_enumerator
			;

prop_enumerator:	  IDENTIFIER { tmpEnum->append( $1 ); }
			| IDENTIFIER '=' { expLevel=0; }
			  enumerator_expression { tmpEnum->append( $1 );  }

opt_identifier:		  /* empty */
			| IDENTIFIER
			;

%%

#if defined(_OS_WIN32_)
#include <io.h>
#undef isatty
extern "C" int hack_isatty( int )
{
    return 0;
}
#define isatty hack_isatty
#else
#include <unistd.h>
#endif

#include "lex.yy.c"

// HACK
#include "properties.cpp"

struct Property
{
  Property() { setfunc = 0; getfunc = 0; getSpec = '?'; setSpec = '?'; }
  Function* setfunc;
  Function* getfunc;
  QCString type;
  char getSpec;					// 'p' = pointer, 'r' = reference, 'c' = class
  char setSpec;					// 'p' = pointer, 'r' = reference, 'c' = class
};

void 	  init();				// initialize
void 	  initClass();				// prepare for new class
void 	  generateClass();			// generate C++ code for class
void 	  initExpression();			// prepare for new expression
QCString  combinePath( const char *, const char * );

QCString  fileName;				// file name
QCString  outputFile;				// output file name
QCString  includeFile;				// name of #include file
QCString  includePath;				// #include file path
QCString  qtPath;				// #include qt file path
bool	  noInclude     = FALSE;		// no #include <filename>
bool	  generatedCode = FALSE;		// no code generated
bool	  mocError = FALSE;			// moc parsing error occurred
QCString  className;				// name of parsed class
QCString  superclassName;			// name of super class
FuncList  signals;				// signal interface
FuncList  slots;				// slots interface
FuncList  props;				// properties interface
EnumList  enums;				// enums used in properties
QStringList propTypes;				// list of legal property types ( except enums )
QDict<Property> pdict;				// dict of all properties

FILE  *out;					// output file

int yyparse();

void replace( char *s, char c1, char c2 );

int main( int argc, char **argv )
{
    bool autoInclude = TRUE;
    char *error	     = 0;
    qtPath = "";
    for ( int n=1; n<argc && error==0; n++ ) {
	QCString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    QCString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    outputFile = argv[++n];
		} else
		    outputFile = &opt[1];
	    } else if ( opt == "i" ) {		// no #include statement
		noInclude   = TRUE;
		autoInclude = FALSE;
	    } else if ( opt[0] == 'f' ) {	// produce #include statement
		noInclude   = FALSE;
		autoInclude = FALSE;
		if ( opt[1] ) {			// -fsomething.h
		    includeFile = &opt[1];
		}
	    } else if ( opt[0] == 'p' ) {	// include file path
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing path name for the -p option.";
			break;
		    }
		    includePath = argv[++n];
		} else {
		    includePath = &opt[1];
		}
	    } else if ( opt[0] == 'q' ) {	// qt include file path
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing path name for the -q option.";
			break;
		    }
		    qtPath = argv[++n];
		} else {
		    qtPath = &opt[1];
		}
		replace(qtPath.data(),'\\','/');
		if ( qtPath.right(1) != "/" )
		    qtPath += '/';
	    } else if ( opt == "k" ) {		// don't stop on errors
		errorControl = TRUE;
	    } else if ( opt == "nw" ) {		// don't display warnings
		displayWarnings = FALSE;
	    } else if ( opt == "ldbg" ) {	// lex debug output
		lexDebug = TRUE;
	    } else if ( opt == "ydbg" ) {	// yacc debug output
		yydebug = TRUE;
	    } else if ( opt == "dbg" ) {	// non-signal members are slots
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
	int ppos = fileName.findRev('.');
	if ( ppos != -1 && tolower( fileName[ppos + 1] ) == 'h' )
	    noInclude = FALSE;
	else
	    noInclude = TRUE;
    }
    if ( !fileName.isEmpty() && !outputFile.isEmpty() &&
	 includeFile.isEmpty() && includePath.isEmpty() ) {
	includeFile = combinePath(fileName,outputFile);
    }
    if ( includeFile.isEmpty() )
	includeFile = fileName.copy();
    if ( !includePath.isEmpty() ) {
	if ( includePath.right(1) != "/" )
	    includePath += '/';
	includeFile = includePath + includeFile;
    }
    if ( fileName.isNull() && !error ) {
	fileName = "standard input";
	yyin	 = stdin;
    } else if ( argc < 2 || error ) {		// incomplete/wrong args
	fprintf( stderr, "Qt meta object compiler\n" );
	if ( error )
	    fprintf( stderr, "moc: %s\n", error );
	fprintf( stderr, "Usage:  moc [options] <header-file>\n"
		 "\t-o file  Write output to file rather than stdout\n"
		 "\t-i       Do not generate an #include statement\n"
		 "\t-f[file] Force #include, optional file name\n"
		 "\t-p path  Path prefix for included file\n"
		 "\t-k       Do not stop on errors\n"
		 "\t-nw      Do not display warnings\n"
#if 0
		 "\t-ldbg    lex debug output\n"
		 "\t-ydbg    yacc debug output\n"
		 "\t-dbg     test parser, all non-signal members are slots\n"
#endif
		 );
	return 1;
    } else {
	yyin = fopen( (const char *)fileName, "r" );
	if ( !yyin ) {
	    fprintf( stderr, "moc: %s: No such file\n", (const char*)fileName);
	    return 1;
	}
    }
    if ( !outputFile.isEmpty() ) {		// output file specified
	out = fopen( (const char *)outputFile, "w" );	// create output file
	if ( !out ) {
	    fprintf( stderr, "moc: Cannot create %s\n",
		     (const char*)outputFile );
	    return 1;
	}
    } else {					// use stdout
	out = stdout;
    }
    init();
    yyparse();
    fclose( yyin );
    if ( !outputFile.isNull() )
	fclose( out );

    if ( !generatedCode && displayWarnings && !mocError ) {
        fprintf( stderr, "%s:%d: Warning: %s\n", fileName.data(), 0,
		 "No relevant classes found. No output generated." );
    }

    slots.clear();
    signals.clear();
    props.clear();

    return mocError ? 1 : 0;
}

void replace( char *s, char c1, char c2 )
{
    if ( !s )
	return;
    while ( *s ) {
	if ( *s == c1 )
	    *s = c2;
	s++;
    }
}

/*
  This function looks at two file names and returns the name of the
  infile, with a path relative to outfile. Examples:
    /tmp/abc	/tmp/bcd	->	abc
    xyz/a/bc	xyz/b/ac	->	../a/bc
    /tmp/abc	xyz/klm		-)	/tmp/abc
 */

QCString combinePath( const char *infile, const char *outfile )
{
    QCString a = infile;  replace(a.data(),'\\','/');
    QCString b = outfile; replace(b.data(),'\\','/');
    a = a.stripWhiteSpace();
    b = b.stripWhiteSpace();
    QCString r;
    int i = 0;
    int ncommondirs = 0;
    while ( a[i] && a[i] == b[i] ) {
	if ( a[i] == '/' && i > 0 )
	    ncommondirs++;
	i++;
    }
    if ( ncommondirs > 0 ) {			// common base directory
	while ( i>=0 ) {
	    if ( a[i] == '/' && b[i] == '/' )
		break;
	    --i;
	}
	++i;
	a = &a[i];
	b = &b[i];
    } else {
	if ( (a[0] == '/') || (isalpha(a[0]) && a[1] == ':') )
	    return a;
	b = &b[i];
    }
    i = b.contains('/');
    while ( i-- > 0 )
	r += "../";
    r += a;
    return r;
}


#define getenv hack_getenv			// workaround for byacc
char *getenv()		     { return 0; }
char *getenv( const char * ) { return 0; }


void init()					// initialize
{
    BEGIN OUTSIDE;
    lineNo	 = 1;
    skipClass	 = FALSE;
    skipFunc	 = FALSE;
    signals.setAutoDelete( TRUE );
    slots.setAutoDelete( TRUE );
    // The props list may contain elements which are in other lists
    props.setAutoDelete( FALSE );
    propTypes.append( "QFont" );
    propTypes.append( "QBrush" );
    propTypes.append( "QImage" );
    propTypes.append( "QPixmap" );
    propTypes.append( "QMovie" );
    propTypes.append( "QString" );
    propTypes.append( "QStringList" );
    propTypes.append( "QRect" );
    propTypes.append( "QSize" );
    propTypes.append( "QPoint" );
    propTypes.append( "QColor" );
    propTypes.append( "QPalette" );
    propTypes.append( "QColorGroup" );
    propTypes.append( "int" );
    propTypes.append( "bool" );
    propTypes.append( "double" );

    tmpArgList	 = new ArgList;
    CHECK_PTR( tmpArgList );
    tmpFunc	 = new Function;
    CHECK_PTR( tmpFunc );
    tmpEnum	 = new Enum;
    CHECK_PTR( tmpEnum );
}

void initClass()				 // prepare for new class
{
    tmpAccessPerm    = _PRIVATE;
    subClassPerm     = _PRIVATE;
    Q_OBJECTdetected = FALSE;
    skipClass	     = FALSE;
    templateClass    = FALSE;
    Q_CUSTOM_FACTORYdetected  = FALSE;
    Q_BUILDERdetected  = FALSE;
    Q_BUILDERcomment   = "";
    Q_BUILDERpixmap    = "";
    Q_INSPECTORdetected = FALSE;
    Q_INSPECTORclass   = "";
    slots.clear();
    signals.clear();
    props.clear();
    enums.clear();
}

struct NamespaceInfo
{
    QCString name;
    int pLevelOnEntering; // Parenthesis level on entering the namespace
    QDict<char> definedClasses; // Classes defined in the namespace
};

QList<NamespaceInfo> namespaces;

void enterNameSpace( const char *name = 0 )   	 // prepare for new class
{
    static bool first = TRUE;
    if ( first ) {
	namespaces.setAutoDelete( TRUE );
	first = FALSE;
    }
	
    NamespaceInfo *tmp = new NamespaceInfo;
    if ( name )
	tmp->name = name;
    tmp->pLevelOnEntering = namespacePLevel;
    namespaces.append( tmp );
}

void leaveNameSpace()				 // prepare for new class
{
    NamespaceInfo *tmp = namespaces.last();
    namespacePLevel = tmp->pLevelOnEntering;
    namespaces.remove();
}

QCString nameQualifier()
{
    QListIterator<NamespaceInfo> iter( namespaces );
    NamespaceInfo *tmp;
    QCString qualifier = "";
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->name.isNull() ) {  // If not unnamed namespace
	    qualifier += tmp->name;
	    qualifier += "::";
	}
    }
    return qualifier;
}

int openNameSpaceForMetaObject( FILE *out )
{
    int levels = 0;
    QListIterator<NamespaceInfo> iter( namespaces );
    NamespaceInfo *tmp;
    QCString indent = "";
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->name.isNull() ) {  // If not unnamed namespace
	    fprintf( out, "%snamespace %s {\n", (const char *)indent,
		     (const char *) tmp->name );
	    indent += "    ";
	    levels++;
	}
    }
    QCString nm = className;
    int pos;
    while( (pos = nm.find( "::" )) != -1 ) {
	QCString spaceName = nm.left( pos );
	nm = nm.right( nm.length() - pos - 2 );
	if ( !spaceName.isEmpty() ) {
	    fprintf( out, "%snamespace %s {\n", (const char *)indent,
		     (const char *) spaceName );
	    indent += "    ";
	    levels++;
	}
    }
    return levels;
}

void closeNameSpaceForMetaObject( FILE *out, int levels )
{
    int i;
    for( i = 0 ; i < levels ; i++ )
	    fprintf( out, "}" );
    if ( levels )
	fprintf( out, "\n" );
	
}

void selectOutsideClassState()
{
    if ( namespaces.count() == 0 )
	BEGIN OUTSIDE;
    else
	BEGIN IN_NAMESPACE;
}

void registerClassInNamespace()
{
    if ( namespaces.count() == 0 )
	return;
    namespaces.last()->definedClasses.insert((const char *)className,(char*)1);
}

//
// Remove white space from SIGNAL and SLOT names.
// This function has been copied from qobject.cpp.
//

inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

inline bool isSpace( char x )
{
#if defined(_CC_BOR_)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar)x <= 32;
#else
    return isspace( x );
#endif
}

static QCString rmWS( const char *src )
{
    QCString result( strlen(src)+1 );
    char *d = result.data();
    char *s = (char *)src;
    char last = 0;
    while( *s && isSpace(*s) )			// skip leading space
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    result.truncate( (int)(d - result.data()) );
    return result;
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
    mocError = TRUE;
    fprintf( stderr, "%s:%d: Error: %s\n", fileName.data(), lineNo, msg );
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
	fprintf( stderr, "%s:%d: Warning: %s\n", fileName.data(), lineNo, msg);
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


char *straddSpc( const char *s1, const char *s2 )
{
    char *n = new char[strlen(s1)+strlen(s2)+2];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    return n;
}

char *straddSpc( const char *s1, const char *s2, const char *s3 )
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+3];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    strcat( n, " " );
    strcat( n, s3 );
    return n;
}

char *straddSpc( const char *s1, const char *s2,
	      const char *s3, const char *s4 )
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+4];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    strcat( n, " " );
    strcat( n, s3 );
    strcat( n, " " );
    strcat( n, s4 );
    return n;
}

// Generate C++ code for building member function table

QCString pureClassName()
{
    QCString result;
    int pos = className.findRev( "::");
    if ( pos != -1 )
        result = className.right( className.length() - pos - 2 );
    else
	result = className;
    return result;
}

QCString qualifiedClassName()
{
    QCString tmp = nameQualifier();
    tmp += className;
    return tmp;
}

QCString qualifiedSuperclassName()
{
    if ( namespaces.count() == 0 )
	return superclassName;
    if ( namespaces.last()->definedClasses.find((const char *)superclassName)){
	QCString tmp = nameQualifier();
	tmp += superclassName;
	return tmp;
    } else {
	return superclassName;
    }
}

const int Slot_Num   = 1;
const int Signal_Num = 2;
const int Prop_Num = 3;

void generateFuncs( FuncList *list, char *functype, int num )
{
    Function *f;
    for ( f=list->first(); f; f=list->next() ) {
	QCString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( !a->leftType.isEmpty() || ! a->rightType.isEmpty() ) {
		if ( count++ )
		    typstr += ",";
		typstr += a->leftType;
		typstr += a->rightType;
	    }
	    a = f->args->next();
	}
	fprintf( out, "    typedef %s(%s::*m%d_t%d)(%s)%s;\n",
		 (const char*)f->type, (const char*)qualifiedClassName(),
		 num, list->at(),
		 (const char*)typstr,  (const char*)f->qualifier );
	f->type = f->name.copy();
	f->type += "(";
	f->type += typstr;
	f->type += ")";
    }
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    m%d_t%d v%d_%d = Q_AMPERSAND %s::%s;\n",
		 num, list->at(), num, list->at(),
		 (const char*)qualifiedClassName(), (const char*)f->name);
    if ( list->count() )
	fprintf(out,"    QMetaData *%s_tbl = QMetaObject::new_metadata(%d);\n",
		functype, list->count() );
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    %s_tbl[%d].name = \"%s\";\n",
		 functype, list->at(), (const char*)f->type );
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    %s_tbl[%d].ptr = *((QMember*)&v%d_%d);\n",
		 functype, list->at(), num, list->at() );
}


void generateTypedef( Function* f, int num )
{
  QCString typstr = "";
  int count = 0;
  Argument *a = f->args->first();
  while ( a ) {
    if ( !a->leftType.isEmpty() || ! a->rightType.isEmpty() ) {
      if ( count++ )
	typstr += ",";
      typstr += a->leftType;
      typstr += a->rightType;
    }
    a = f->args->next();
  }

  // Remove q_prop from the qualifier
  int i = f->qualifier.find( "q_prop" );
  if ( i != -1 )
    f->qualifier.remove( i, 6 );

  fprintf( out, "    typedef %s(%s::*m%d_t%d)(%s)%s;\n",
	   (const char*)f->type, (const char*)className, Prop_Num, num,
	   (const char*)typstr,  (const char*)f->qualifier );
}

int generateProps()
{
    Function *f;
    //
    // Make a list of all properties
    //
    pdict.clear();
    for ( f=props.first(); f; f=props.next() ) {
      QCString name = f->name.copy();
      if ( name.left(5) == "setIs" && isupper( name[5] ) )
	name = name.mid( 5, name.length() - 5 );
      else if ( name.left(3) == "set" )
	name = name.mid( 3, name.length() - 3 );
      else if ( name.left(2) == "is" )
	name = name.mid( 2, name.length() - 2 );
      else if ( name.left(3) == "has" )
	name = name.mid( 3, name.length() - 3 );
      name = name.lower();
      Property* p = pdict.find( name.data() );
      if ( !p )
      {
	p = new Property;
	pdict.insert( name.data(), p );
      }

      // get function
      if ( f->type != "void" )
      {
	if ( p->getfunc )
	  moc_err("Property '%s' has get function defined twice.", name.data() );
	p->getfunc = f;
	QCString tmp = f->type.copy();
	tmp = tmp.simplifyWhiteSpace();
	if ( tmp.left(6) == "const " )
	  tmp = tmp.mid( 6, tmp.length() - 6 );
	if ( tmp.right(1) == "&" )
	{
	  tmp = tmp.left( tmp.length() - 1 );
	  p->getSpec = 'r';
	}
	else if ( tmp.right(1) == "*" )
	{
	  tmp = tmp.left( tmp.length() - 1 );
	  p->getSpec = 'p';
	}
	else
	  p->getSpec = 'c';
	tmp = tmp.simplifyWhiteSpace();
	if ( !p->type.isEmpty() && p->type != tmp )
	  moc_err("Property %s has different types in get and set functions.", name.data() );

	p->type = tmp;
      }
      // set function
      else
      {
	if ( p->setfunc )
	  moc_err("Property '%s' has set function defined twice.", name.data() );

	QCString tmp = f->args->first()->leftType.copy();
	tmp = tmp.simplifyWhiteSpace();
	if ( tmp.left(6) == "const " )
	  tmp = tmp.mid( 6, tmp.length() - 6 );
	if ( tmp.right(1) == "&" )
	{
	  tmp = tmp.left( tmp.length() - 1 );
	  p->setSpec = 'r';
	}
	/* else if ( tmp.right(1) == "*" )
	{
	  tmp = tmp.left( tmp.length() - 1 );
	  p->setSpec = 'p';
	  } */
	else
	  p->setSpec = 'c';
	tmp = tmp.simplifyWhiteSpace();
	if ( !p->type.isEmpty() && p->type != tmp )
	  moc_err("Property %s has different types in set and get functions.", name.data() );

	p->type = tmp;
	p->setfunc = f;
      }	
    }

    //
    // Generate all typedefs
    //
    int count = 0;
    QDictIterator<Property> dit( pdict );
    for( ; dit.current(); ++dit )
    {
      if ( dit.current()->getfunc )
	generateTypedef( dit.current()->getfunc, count );
      if ( dit.current()->setfunc )
	generateTypedef( dit.current()->setfunc, count + 1 );
      count += 2;
    }

    //
    // Crazy stuff for crazy compilers
    //
    count = 0;
    dit.toFirst();
    for( ; dit.current(); ++dit )
    {
	if ( dit.current()->getfunc )
	  fprintf( out, "    m%d_t%d v%d_%d = &%s::%s;\n", Prop_Num, count,
		   Prop_Num, count, (const char*)className,(const char*)dit.current()->getfunc->name);
	if ( dit.current()->setfunc )
	  fprintf( out, "    m%d_t%d v%d_%d = &%s::%s;\n", Prop_Num, count + 1,
		   Prop_Num, count + 1, (const char*)className,(const char*)dit.current()->setfunc->name);
	count += 2;
    }

    //
    // Create meta data
    //
    if ( pdict.count() )
	fprintf( out, "    QMetaProperty *props_tbl = new QMetaProperty[%d];\n", pdict.count() );
    count = 0;
    int entry = 0;
    dit.toFirst();
    for( ; dit.current(); ++dit )
    {
      fprintf( out, "    props_tbl[%d].name = \"%s\";\n",
	       entry, (const char*)dit.currentKey() );
	if ( dit.current()->getfunc )
	  fprintf( out, "    props_tbl[%d].get = *((QMember*)&v%d_%d);\n",
		   entry, Prop_Num, count );
	else
	  fprintf( out, "    props_tbl[%d].get = 0;\n", entry );
	if ( dit.current()->setfunc )
	{
	  fprintf( out, "    props_tbl[%d].set = *((QMember*)&v%d_%d);\n",
		   entry, Prop_Num, count + 1 );
	  fprintf( out, "    props_tbl[%d].readonly = FALSE;\n", entry );
	}
	else
	{
	  fprintf( out, "    props_tbl[%d].set = 0;\n", entry );
	  fprintf( out, "    props_tbl[%d].readonly = TRUE;\n", entry );
	}
	fprintf( out, "    props_tbl[%d].type = \"%s\";\n", entry, (const char*)dit.current()->type );
	fprintf( out, "    props_tbl[%d].getSpec = '%c';\n", entry, dit.current()->getSpec );
	fprintf( out, "    props_tbl[%d].setSpec = '%c';\n", entry, dit.current()->setSpec );

	int enumpos = -1;
	int k = 0;
	QListIterator<Enum> it( enums );
	for( ; it.current(); ++it, ++k )
	{
	  if ( it.current()->name == dit.current()->type )
	    enumpos = k;
	}

	if ( enumpos == -1 )
	  fprintf( out, "    props_tbl[%d].enumType = 0;\n", entry );
	else
	  fprintf( out, "    props_tbl[%d].enumType = &enums[%i];\n", entry, enumpos );

	++entry;
	count += 2;
    }

    return pdict.count();
}

bool isPropertyType( const char* type, bool test_enums = TRUE )
{
    QStringList::Iterator it = propTypes.begin();
    for( ; it != propTypes.end(); ++it )
    {
      if ( *it == type )
	return TRUE;
    }

    if ( !test_enums )
      return FALSE;

    QListIterator<Enum> lit( enums );
    for( ; lit.current(); ++lit )
    {
      if ( lit.current()->name == type )
	return TRUE;
    }
    
    return FALSE;
}

void finishProps()
{
    int entry = 0;
    QDictIterator<Property> dit( pdict );
    for( ; dit.current(); ++dit )
    {
      if ( !isPropertyType( dit.current()->type ) )
	fprintf( out, "    metaObj->fixProperty( &props_tbl[%d], TRUE );\n", entry );
      else if ( dit.current()->getfunc == 0 || dit.current()->setfunc == 0 )
	fprintf( out, "    metaObj->fixProperty( &props_tbl[%d], FALSE );\n", entry );
      ++entry;
    }
}

int generateEnums()
{
  if ( enums.count() == 0 )
    return 0;

  fprintf( out, "    QMetaEnum* enums = new QMetaEnum[ %i ];\n", enums.count() );

  int i = 0;
  QListIterator<Enum> it( enums );
  for( ; it.current(); ++it, ++i )
  {
    fprintf( out, "    enums[%i].name = \"%s\";\n", i, (const char*)it.current()->name );
    fprintf( out, "    enums[%i].nEnumerators = %i;\n", i, (const char*)it.current()->count() );
    fprintf( out, "    enums[%i].enumerators = new QMetaEnumerator[%i];\n", i, (const char*)it.current()->count() );
    Enum::Iterator eit = it.current()->begin();
    int k = 0;
    for( ; eit != it.current()->end(); ++eit, ++k )
    {
      fprintf( out, "    enums[%i].enumerators[%i].name = \"%s\";\n", i, k, (*eit).ascii() );
      fprintf( out, "    enums[%i].enumerators[%i].value = (int)%s::%s;\n", i, k, (const char*)className, (*eit).ascii() );
    }
  }

  return enums.count();
}

void generateClass()		      // generate C++ source code for a class
{
    static int gen_count = 0;
    char *hdr1 = "/****************************************************************************\n"
		 "** %s meta object code from reading C++ file '%s'\n**\n";
    char *hdr2 = "** Created: %s\n"
		 "**      by: The Qt Meta Object Compiler ($Revision: 1.8 $)\n**\n";
    char *hdr3 = "** WARNING! All changes made in this file will be lost!\n";
    char *hdr4 = "*****************************************************************************/\n\n";
    int   i;

    /**
      * Torbens incredible hack until Qt 3.0 is released. I am doing
      * this with protest only!
      */
    int ti = 0;
    while( ti >= 0 && TorbensHack[ti] != 0 )
    {
      const char* tp = TorbensHack[ti];
      if ( *tp == '+' && strcmp( TorbensHack[ti+1], className.data() ) == 0 )
      {
	ti += 2;
	Q_BUILDERdetected = TRUE;
	Q_BUILDERcomment = TorbensHack[ti++];
	Q_BUILDERpixmap = TorbensHack[ti++];
	Q_INSPECTORclass = TorbensHack[ti++];
	printf("comm=%s pix=%s class=%s\n",Q_BUILDERcomment.data(), Q_BUILDERpixmap.data(),
	       Q_INSPECTORclass.data() );
	while( TorbensHack[ti] != 0 && TorbensHack[ti][0] != '+' )
	{
	  tmpFunc->type = TorbensHack[ti++];
	  tmpFunc->name = TorbensHack[ti++];
	  tmpFunc->qualifier = TorbensHack[ti++];
	  printf("ret=%s name=%s qual=%s\n",tmpFunc->type.data(),tmpFunc->name.data(),tmpFunc->qualifier.data());
	  while( TorbensHack[ti][0] != ')' )
	  {
	    printf("Argument %s\n",(char*)TorbensHack[ti]);
	    tmpFunc->args = addArg( new Argument( (char*)TorbensHack[ti++], "" ) );
	  }
	  ++ti;

	  addMember( 'p' );
	}
	// we are done
	ti = -1;
      }
      else
	++ti;
    }
    // End of hack

    if ( skipClass )				// don't generate for class
	return;
    if ( !Q_OBJECTdetected ) {
	if ( signals.count() == 0 && slots.count() == 0 )
	    return;
	generatedCode = TRUE;
	if ( displayWarnings )
	    moc_err("The declaration of the class \"%s\" contains slots "
		    "and/or signals\n\t but no Q_OBJECT macro!", className.data());
    } else {
	if ( superclassName.isEmpty() )
	    moc_err("The declaration of the class \"%s\" contains the\n"
		    "\tQ_OBJECT macro but does not inherit from any class!\n"
		    "\tInherit from QObject or one of its descendants"
		    " or remove Q_OBJECT. ", className.data() );
    }
    if ( templateClass ) {			// don't generate for class
	moc_err( "Sorry, Qt does not support templates that contain\n"
		 "signals, slots or Q_OBJECT. This will be supported soon." );
	return;
    }
    generatedCode = TRUE;
    if ( gen_count++ == 0 ) {			// first class to be generated
	QDateTime dt = QDateTime::currentDateTime();
	QCString dstr = dt.toString().ascii();
	QCString fn = fileName;
	i = fileName.length()-1;
	while ( i>0 && fileName[i-1] != '/' && fileName[i-1] != '\\' )
	    i--;				// skip path
	if ( i >= 0 )
	    fn = &fileName[i];
	fprintf( out, hdr1, (const char*)qualifiedClassName(),(const char*)fn);
	fprintf( out, hdr2, (const char*)dstr );
	fprintf( out, hdr3 );
	fprintf( out, hdr4 );
	fprintf( out, "#define Q_MOC_%s\n", className.data() );
	fprintf( out, "#if !defined(Q_MOC_OUTPUT_REVISION)\n" );
	fprintf( out, "#define Q_MOC_OUTPUT_REVISION %d\n", formatRevision );
	fprintf( out, "#elif Q_MOC_OUTPUT_REVISION != %d\n", formatRevision );
	fprintf( out, "#error \"Moc format conflict - "
		 "please regenerate all moc files\"\n" );
	fprintf( out, "#endif\n\n" );
	if ( !noInclude )
	    fprintf( out, "#include \"%s\"\n", (const char*)includeFile );
	fprintf( out, "#include <%sqmetaobject.h>\n", (const char*)qtPath );
	fprintf( out, "#include <%sqapplication.h>\n", (const char*)qtPath );
	fprintf( out, "\n" );
	fprintf( out, "#if defined(Q_SPARCWORKS_FUNCP_BUG)\n" );
	fprintf( out, "#define Q_AMPERSAND\n" );
	fprintf( out, "#else\n" );
	fprintf( out, "#define Q_AMPERSAND &\n" );
	fprintf( out, "#endif\n" );
	fprintf( out, "\n\n" );
    } else {
	fprintf( out, "\n\n" );
    }

//
// Generate the builder pixmap if needed
//
    if ( Q_BUILDERdetected && !Q_BUILDERpixmap.isEmpty() )
    {
        QCString fn;
	if ( includePath.isEmpty() )
	{
	  QCString a = fileName;  replace(a.data(),'\\','/');
	  if ( a.isEmpty() )
	    fn = Q_BUILDERpixmap;
	  else
	  {
	    int i = a.findRev( '/' );
	    if ( i == -1 )
	      fn = Q_BUILDERpixmap;
	    else
	    {
	      fn = a.left( i + 1 );
	      fn += Q_BUILDERpixmap;
	    }
	  }
	}
	else
	    fn = includePath + fn + Q_BUILDERpixmap;
        QFile file( fn );
	if ( file.open( IO_ReadOnly ) )
	{
	    fprintf( out, "static const char* pixmap_%s[] = {\n", (const char*)className );
	    while( !file.atEnd() )
	    {
	      QString tmp;
	      int n = file.readLine( tmp, 1024 );
	      if ( n < 0 || n == 1024 )
		moc_err("Pixmap in file %s does not seem to be in XPM format", fn.data() );
	      if ( !tmp.isEmpty() && tmp[0] == '"' )
		fprintf( out, "%s", tmp.ascii() );
	    }
	    fprintf( out, "\n" );
	}
	else
	  moc_err("Pixmap file %s can not be read", fn.data() );
    }

//
// Generate static member function factory() if needed
//

    // This variable is part of Torbens hack
    if ( className == "QToolBar" )
    {
      // Has its own custom factory
      Q_CUSTOM_FACTORYdetected = TRUE;
    }
    bool hasFactory = TRUE;

    if ( ( Q_BUILDERdetected && !Q_CUSTOM_FACTORYdetected ) || Q_INSPECTORdetected )
    {
        // Torbens incrdible hack start here
        bool layout = FALSE;
	bool abstr = FALSE;
        int tl = 0;
	while( !layout && TorbensLayout[tl] )
	{
	  if ( className == TorbensLayout[tl++] )
	    layout = TRUE;
	}
        tl = 0;
	while( !abstr && TorbensAbstract[tl] )
	{
	  if ( className == TorbensAbstract[tl++] )
	    abstr = TRUE;
	}

	if ( abstr )
	{
	  hasFactory = FALSE;
	}
	else if ( layout )
	{
	    fprintf( out, "static QObject *%s_factory( QObject* _parent )\n{\n", (const char*)className );
	    fprintf( out, "    if ( _parent == 0 ) return new %s;\n",(const char*)className);
	    fprintf( out, "    if ( _parent->inherits( \"QLayout\" ) ) return new %s( (QLayout*)_parent );\n",
		     (const char*)className);
	    fprintf( out, "    return new %s( (QWidget*)_parent );\n}\n\n", (const char*)className );
	}
	else
	{
	    // The hack ends here
	    fprintf( out, "static QObject *%s_factory( QObject* _parent )\n{\n", (const char*)className );
	    fprintf( out, "    return new %s( (QWidget*)_parent );\n}\n\n", (const char*)className );
	}
    }
    
//
// Generate virtual function className()
//
    fprintf( out, "const char *%s::className() const\n{\n    ",
	     (const char*)qualifiedClassName() );
    fprintf( out, "return \"%s\";\n}\n\n", (const char*)qualifiedClassName() );

//
// Generate static metaObj variable
//
    fprintf( out, "QMetaObject *%s::metaObj = 0;\n\n", (const char*)qualifiedClassName());

//
// Generate static meta-object constructor-object (we don't rely on
// it, except for QBuilder).
//
    fprintf( out, "\n#if QT_VERSION >= 199\n" );
    int levels = openNameSpaceForMetaObject( out );
    fprintf( out, "static QMetaObjectInit init_%s(&%s::staticMetaObject);\n\n",
	(const char*)pureClassName(), (const char*)qualifiedClassName() );
    closeNameSpaceForMetaObject( out, levels );
    fprintf( out, "#endif\n\n" );

//
// Generate initMetaObject member function
//
    fprintf( out, "void %s::initMetaObject()\n{\n", (const char*)qualifiedClassName() );
    fprintf( out, "    if ( metaObj )\n\treturn;\n" );
    fprintf( out, "    if ( strcmp(%s::className(), \"%s\") != 0 )\n"
	          "\tbadSuperclassWarning(\"%s\",\"%s\");\n",
             (const char*)qualifiedSuperclassName(), (const char*)qualifiedSuperclassName(),
             (const char*)qualifiedClassName(), (const char*)qualifiedSuperclassName() );
    fprintf( out, "\n#if QT_VERSION >= 199\n" );
    fprintf( out, "    staticMetaObject();\n");
    fprintf( out, "}\n\n");

//
// Generate tr member function
//
    fprintf( out, "QString %s::tr(const char* s)\n{\n", (const char*)qualifiedClassName() );
    fprintf( out, "    return ((QNonBaseApplication*)qApp)->translate(\"%s\",s);\n}\n\n", (const char*)qualifiedClassName() );

//
// Generate staticMetaObject member function
//
    fprintf( out, "QMetaObject* %s::staticMetaObject()\n{\n", (const char*)qualifiedClassName() );
    fprintf( out, "    if ( metaObj )\n\treturn metaObj;\n" );
    fprintf( out, "    %s::staticMetaObject();\n", (const char*)qualifiedSuperclassName() );
    fprintf( out, "#else\n\n" );
    fprintf( out, "    %s::initMetaObject();\n", (const char*)qualifiedSuperclassName() );
    fprintf( out, "#endif\n\n" );

//
// Initialize the meta object of the class for which
// this class acts as an inspector
//
    if ( Q_INSPECTORdetected  )
    {
        fprintf( out, "    QMetaObject* iobj = %s::initMetaObject();\n", (const char*)Q_INSPECTORclass.data() );
	fprintf( out, "    iobj->setInspector( (QInspectorFactory)%s::factory );\n", (const char*)className );
    }

//
// Build the enums array in staticMetaObject()
// Enums HAVE to be generated BEFORE the properties
//
   int n_enums = generateEnums();

//
// Build property array in staticMetaObject()
//
   int n_props = generateProps();

//
// Build slots array in staticMetaObject()
//
    generateFuncs( &slots, "slot", Slot_Num );

//
// Build signals array in staticMetaObject()
//
    generateFuncs( &signals, "signal", Signal_Num );

//
// Finally code to create and return meta object
//
    fprintf( out, "    metaObj = QMetaObject::new_metaobject(\n"
		  "\t\"%s\", \"%s\",\n",
	     (const char*)qualifiedClassName(), (const char*)qualifiedSuperclassName() );
    if ( slots.count() )
	fprintf( out, "\tslot_tbl, %d,\n", slots.count() );
    else
	fprintf( out, "\t0, 0,\n" );
    if ( signals.count() )
	fprintf( out, "\tsignal_tbl, %d,\n", signals.count());
    else
	fprintf( out, "\t0, 0,\n" );
    if ( n_props )
	fprintf( out, "\tprops_tbl, %d,\n", n_props );
    else
	fprintf( out, "\t0, 0,\n" );
    if ( n_enums )
	fprintf( out, "\tenums, %d );\n", n_enums );
    else
	fprintf( out, "\t0, 0 );\n" );

//
// Finish property array in staticMetaObject()
//
    finishProps();

//
// create Builder information in staticMetaObject()
//
    if ( Q_BUILDERdetected )
    {
        if ( !Q_BUILDERcomment.isEmpty() )
           fprintf( out, "    metaObj->setComment( \"%s\" );\n", (const char*)Q_BUILDERcomment );
        if ( !Q_BUILDERpixmap.isEmpty() )
           fprintf( out, "    metaObj->setPixmap( pixmap_%s );\n", (const char*)className );
        // This is one of Torbens hacks
        if ( Q_CUSTOM_FACTORYdetected )
           fprintf( out, "    metaObj->setFactory( %s::factory );\n", (const char*)className );
        else if ( hasFactory )
           fprintf( out, "    metaObj->setFactory( %s_factory );\n", (const char*)className );
    }
    fprintf( out, "    return metaObj;\n}\n" );

//
// End of function initMetaObject()
//

//
// Generate internal signal functions
//
    Function *f;
    f = signals.first();			// make internal signal methods
    static bool included_list_stuff = FALSE;
    while ( f ) {
	QCString typstr = "";			// type string
	QCString valstr = "";			// value string
	QCString argstr = "";			// argument string (type+value)
	char	buf[12];
	Argument *a = f->args->first();
	QCString typvec[32], valvec[32], argvec[32];
	typvec[0] = "";
	valvec[0] = "";
	argvec[0] = "";

	i = 0;
	while ( a ) {				// argument list
	    if ( !a->leftType.isEmpty() || !a->rightType.isEmpty() ) {
		if ( i ) {
		    typstr += ",";
		    valstr += ", ";
		    argstr += ", ";
		}
		typstr += a->leftType;
		typstr += a->rightType;
		argstr += a->leftType;
		argstr += " ";
		sprintf( buf, "t%d", i );
		valstr += buf;
		argstr += buf;
		argstr += a->rightType;
		++i;
		typvec[i] = typstr.copy();
		valvec[i] = valstr.copy();
		argvec[i] = argstr.copy();
	    }
	    a = f->args->next();
	}

	const char *predef_call_func;
	if ( typstr == "bool" ) {
	    predef_call_func = "activate_signal_bool";
	} else if ( typstr == "QString" ) {
	    predef_call_func = "activate_signal_string";
	} else if ( typstr == "const QString&" ) {
	    predef_call_func = "activate_signal_strref";
	} else if ( typstr.isEmpty() || typstr == "short" ||
		    typstr == "int" ||  typstr == "long" ||
		    typstr == "char*" || typstr == "const char*" ) {
	    predef_call_func = "activate_signal";
	} else {
	    predef_call_func = 0;
	}
	if ( !predef_call_func && !included_list_stuff ) {
	    // yes we need it, because otherwise QT_VERSION may not be defined
	    fprintf( out, "\n#include <%sqobjectdefs.h>\n", (const char*)qtPath );
	    fprintf( out, "#include <%sqsignalslotimp.h>\n", (const char*)qtPath );
	    included_list_stuff = TRUE;
	}

	fprintf( out, "\n/" /* c++ */ "/ SIGNAL %s\n", (const char*)f->name );
	fprintf( out, "void %s::%s(", (const char*)qualifiedClassName(),
		 (const char*)f->name );

	if ( argstr.isEmpty() )
	    fprintf( out, ")\n{\n" );
	else
	    fprintf( out, " %s )\n{\n", (const char*)argstr );

	if ( predef_call_func ) {
	    fprintf( out, "    %s( \"%s(%s)\"", predef_call_func,
		     (const char*)f->name, (const char*)typstr );
	    if ( !valstr.isEmpty() )
		fprintf( out, ", %s", (const char*)valstr );
	    fprintf( out, " );\n}\n" );
	    f = signals.next();
	    continue;
	}


	fprintf( out,"    // No builtin function for signal parameter type %s\n",
		 (const char*)typstr );
	int nargs = f->args->count();
	fprintf( out, "    QConnectionList *clist = receivers(\"%s(%s)\");\n",
		 (const char*)f->name, (const char*)typstr );
	fprintf( out, "    if ( !clist || signalsBlocked() )\n\treturn;\n" );
	if ( nargs ) {
	    for ( i=0; i<=nargs; i++ ) {
		fprintf( out, "    typedef void (QObject::*RT%d)(%s);\n",
			 i, (const char*)typvec[i] );
		fprintf( out, "    typedef RT%d *PRT%d;\n", i, i );
	    }
	} else {
	    fprintf( out, "    typedef void (QObject::*RT)(%s);\n",
		     (const char*)typstr);
	    fprintf( out, "    typedef RT *PRT;\n" );	
	}
	if ( nargs ) {
	    for ( i=0; i<=nargs; i++ )
		fprintf( out, "    RT%d r%d;\n", i, i );
	} else {
	    fprintf( out, "    RT r;\n" );
	}
	fprintf( out, "    QConnectionListIt it(*clist);\n" );
	fprintf( out, "    QConnection   *c;\n" );
	fprintf( out, "    QSenderObject *object;\n" );
	fprintf( out, "    while ( (c=it.current()) ) {\n" );
	fprintf( out, "\t++it;\n" );
	fprintf( out, "\tobject = (QSenderObject*)c->object();\n" );
	fprintf( out, "\tobject->setSender( this );\n" );
	if ( nargs ) {
	    fprintf( out, "\tswitch ( c->numArgs() ) {\n" );
	    for ( i=0; i<=nargs; i++ ) {
		fprintf( out, "\t    case %d:\n", i );
		fprintf( out, "\t\tr%d = *((PRT%d)(c->member()));\n", i, i );
		fprintf( out, "\t\t(object->*r%d)(%s);\n",
			 i, (const char*)valvec[i] );
		fprintf( out, "\t\tbreak;\n" );
	    }
	    fprintf( out, "\t}\n" );
	} else {
	    fprintf( out, "\tr = *((PRT)(c->member()));\n" );
	    fprintf( out, "\t(object->*r)(%s);\n", (const char*)valstr );
	}
	fprintf( out, "    }\n}\n" );
	f = signals.next();
    }
}

ArgList *addArg( Argument *a )			// add argument to list
{
    tmpArgList->append( a );
    return tmpArgList;
}

void addEnum()
{
    enums.append( tmpEnum );
    tmpEnum = new Enum;
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
    bool isprop = ( m == 'p' );
    if ( m == 't' && tmpFunc->qualifier.find( "q_prop" ) != -1 )
      isprop = TRUE;

    if ( m == 's' && tmpFunc->type != "void" ) {
	moc_err( "Signals must have \"void\" as their return type" );
	goto Failed;
    }
    else if ( isprop && tmpFunc->type != "void" && tmpFunc->args && tmpFunc->args->count() != 0 )
    {
      moc_err( "getProperty function may not have parameters." );
      goto Failed;
    }
    else if ( isprop && tmpFunc->type == "void" && ( tmpFunc->args == 0 || tmpFunc->args->count() != 1 ) )
    {
      moc_err( "setProperty function must accept exactly one parameter." );
      goto Failed;
    }

    // Check wether the parameter is of legal type
    if ( isprop && tmpFunc->type == "void" )
    {
      bool special = FALSE;
      QCString tmp = tmpFunc->args->first()->leftType.copy();
      tmp = tmp.simplifyWhiteSpace();
      if ( tmp.left(6) == "const " )
      {
	tmp = tmp.mid( 6, tmp.length() - 6 );
	special = TRUE;
      }
      if ( tmp.right(1) == "&" )
      {
	special = TRUE;
	tmp = tmp.left( tmp.length() - 1 );
      }
      tmp = tmp.simplifyWhiteSpace();
      /* else if ( tmp.right(1) == "*" )
      {
	special = TRUE;
	tmp = tmp.left( tmp.length() - 1 );
	} */
      // No []* etc. allowed here
      for( int i = 0; i < tmp.length(); ++i )
	if ( !isIdentChar( tmp[i] ) && tmp[i] != '_' )
	{
	  moc_err( "setProperty function %s(...) has illegal parameter type.", tmpFunc->name.data() );
	  goto Failed;
	}
      // pointers and refs are not allowed on enums
      if ( special && !isPropertyType( tmp, FALSE ) )
      {
	moc_err( "setProperty function %s(...) has illegal parameter type.", tmpFunc->name.data() );
	goto Failed;
      }
    }
    // Check wether the return type is of legal type
    else if ( isprop && tmpFunc->type != "void" )
    {
      QCString tmp = tmpFunc->type;
      tmp = tmp.simplifyWhiteSpace();
      bool special = FALSE;
      if ( tmp.left(6) == "const " )
      {
	tmp = tmp.mid( 6, tmp.length() - 6 );
	special = TRUE;
      }
      if ( tmp.right(1) == "&" )
      {
	special = TRUE;
	tmp = tmp.left( tmp.length() - 1 );
      }
      else if ( tmp.right(1) == "*" )
      {
	special = TRUE;
	tmp = tmp.left( tmp.length() - 1 );
      }
      tmp = tmp.simplifyWhiteSpace();
      // No []* etc. allowed here
      for( int i = 0; i < tmp.length(); ++i )
	if ( !isIdentChar( tmp[i] ) && tmp[i] != '_' )
	{
	  moc_err( "getProperty function %s() has illegal parameter type.", tmpFunc->name.data() );
	  goto Failed;
	}
      // pointers and refs are not allowed on enums
      if ( special && !isPropertyType( tmp, FALSE ) )
      {
	moc_err( "getProperty function %s() has illegal parameter type.", tmpFunc->name.data() );
	goto Failed;
      }
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
    if ( isprop )
      props.append( tmpFunc );

 Failed:
    skipFunc = FALSE;
    tmpFunc  = new Function;
    CHECK_PTR( tmpFunc );
}
