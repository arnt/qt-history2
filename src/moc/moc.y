/****************************************************************************
** $Id: //depot/qt/main/src/moc/moc.y#5 $
**
** Parser and code generator for meta object compiler
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993 by Haavard Nord.	All rights reserved.
**
** --------------------------------------------------------------------------
** This file contains the parser and code generator for the meta object
** compiler (moc) of the Quasar application framework.
**
** This compiler reads a C++ header file with class definitions and dumps
** meta data to an output C++ file. The meta data includes public methods
** (not constructors, destructors or operator functions), signals and slot
** definitions. The output file should be compiled and linked into the
** target Quasar application.
**
** C++ header files are assumed to have correct syntax, and we are therefore
** doing less checking that C++ compilers.
**
** The C++ grammar has been adopted from the "The Annotated C++ Reference
** Manual" (ARM), by Ellis and Stroustrup (Addison Wesley, 1992).
**
** Notice that this code is not possible to compile with GNU bison, instead
** use standard the AT&T yacc or Berkeley yacc.
** Don't panic if you get 7 shift/reduce conflicts. That is perfectly normal.
**
** TODO:
**    Better grammer. int as def. return value.
**    Specify output file. Default name on UNIX: file.m.C ?
**    Clean up memory.
*****************************************************************************/

%{
#include <qlist.h>
#include <qstring.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


enum AccessPerm { _PRIVATE, _PROTECTED, _PUBLIC };


struct Argument					// single arg meta data
{
    Argument( char *name, char *ptr )
    	{ typeName=name; ptrType=ptr; }
    QString typeName;
    QString ptrType;
};

declare(QList,Argument);

class ArgList : public QList(Argument) {	// member function arg list
public:
    ArgList() { autoDelete(TRUE); }
};


struct Function					// member function meta data
{						//   used for public methods,
    QString  name;				//   signals and slots
    QString  type;
    QString  ptrType;
    int	     lineNo;
    ArgList *args;
   ~Function() { delete args; }
};

declare(QList,Function);

class FuncList : public QList(Function) {	// list of member functions
public:
    FuncList() { autoDelete(TRUE); }
};


ArgList *addArg( Argument * );			// add arg to tmpArgList
void	 addMember( QString, char );		// add tmpFunc to current class

char    *strnew( char * );			// returns a new string (copy)
char	*stradd( char *, char * );		// add two strings

extern int yydebug;
int	 lexdebug;
int	 lineNo;				// current line number
bool	 errorControl;				// controlled errors

ArgList	 *tmpArgList;				// current argument list
Function *tmpFunc;				// current member function

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

%token			METHODS
%token			SIGNALS
%token			SLOTS


%type  <string>		class_name
%type  <string>		opt_base_spec
%type  <string>		base_spec
%type  <string>		base_list
%type  <string>		base_specifier
%type  <access>		access_specifier
%type  <function>	fct_decl
%type  <string>		fct_name
%type  <function>	fct_name_decl
%type  <string>		simple_type_name
%type  <string>		complete_class_name

%type  <arg_list>	argument_declaration_list
%type  <arg_list>	arg_declaration_list
%type  <arg_list>	arg_declaration_list_opt
%type  <string>		abstract_decl_opt
%type  <string>		abstract_decl
%type  <arg>		argument_declaration
%type  <string>		decl_specifiers
%type  <string>		declarator
%type  <string>		ptr_operator

%%
class_defs:
			| class_defs class_def
			;

class_def:		  class_specifier ';' { generate(); BEGIN OUTSIDE; }
			;


/***** r.17.1 (ARM p.387 ): Keywords	*****/

class_name:		  IDENTIFIER
			;


/***** r.17.2 (ARM p.388): Expressions	*****/

expression:		  primary_expression
			;

primary_expression:	  literal
			;

literal:		  integer_constant
			| character_constant
			| floating_constant
			| string_literal
			| IDENTIFIER
			;

integer_constant:	  INT_VAL
			;

character_constant:	  CHAR_VAL
			;

floating_constant:	  DOUBLE_VAL
			;

string_literal:		  STRING
			;


/***** r.17.3 (ARM p.391): Declarations	*****/

decl_specifier:		  storage_class_specifier
			| type_specifier
			| fct_specifier
			| FRIEND
			| TYPEDEF
			;

decl_specifiers:	  decl_specs_opt simple_type_name decl_specs_opt
						{ $$ = $2; }
			;

decl_specs_opt:		  	/* empty */
			| decl_specs
			;

decl_specs:		  decl_specs decl_specifier
			| decl_specifier
			;

storage_class_specifier:  AUTO
			| REGISTER
			| STATIC
			| EXTERN
			;

fct_specifier:		  INLINE
			| VIRTUAL
			;

type_specifier:		  CONST
			| VOLATILE
			;

simple_type_name:	  complete_class_name	{ $$ = $1; }
			| CHAR			{ $$ = "char"; }
			| SHORT			{ $$ = "short"; }
			| INT			{ $$ = "int"; }
			| LONG			{ $$ = "long"; }
			| SIGNED		{ $$ = "signed"; }
			| UNSIGNED		{ $$ = "unsigned"; }
			| FLOAT			{ $$ = "float"; }
			| DOUBLE		{ $$ = "double"; }
			| VOID			{ $$ = "void"; }
			;

class_key		: CLASS
			;

complete_class_name:	  class_name
			;


/***** r.17.4 (ARM p.393): Declarators	*****/

argument_declaration_list:  arg_declaration_list_opt triple_dot_opt
			|   arg_declaration_list ',' TRIPLE_DOT
			;

arg_declaration_list_opt:	/* empty */	{ $$ = tmpArgList; }
			| arg_declaration_list	{ $$ = $1; }
			;

triple_dot_opt:			/* empty */
			| TRIPLE_DOT
			;

arg_declaration_list:	  arg_declaration_list
			  ','
			  argument_declaration	{ $$ = addArg($3); }
			| argument_declaration	{ $$ = addArg($1); }

argument_declaration:	  decl_specifiers declarator
						{ $$ = new Argument($1,$2); }
			| decl_specifiers declarator '=' expression
						{ $$ = new Argument($1,$2); }
			| decl_specifiers abstract_decl_opt
						{ $$ = new Argument($1,$2); }
			| decl_specifiers abstract_decl_opt '=' expression
						{ $$ = new Argument($1,$2); }
			;

abstract_decl_opt:	  /* empty */		{ $$ = ""; }
			| abstract_decl		{ $$ = $1; }
			;

abstract_decl:		 abstract_decl ptr_operator  /* NOTE: Simplified! */
						{ $$ = stradd($1,$2); }
			| ptr_operator		{ $$ = $1; }
			;

declarator:		  dname			{ $$ = "";}
			| ptr_operator declarator { $$ = stradd($1,$2);}
			| '(' declarator ')'	{ $$ = $2; }
			;

dname:			  IDENTIFIER
			;

fct_decl:		  fct_name_decl
			  '('
			  argument_declaration_list
			  ')'
			  cv_qualifier_list_opt
			  fct_body_opt
						{ tmpFunc->args = $3;
						  $$ = tmpFunc; }
			;

fct_name:		  IDENTIFIER		/* NOTE: simplified! */
			;

fct_name_decl:		  fct_name		{ tmpFunc->name = $1;
						  tmpFunc->ptrType  = "";
					      	  $$ = tmpFunc; }
			| ptr_operator fct_name_decl
						{ tmpFunc->name = $2->name;
						  tmpFunc->ptrType =
						    stradd($1,$2->ptrType);
						  $$ = tmpFunc; }

ptr_operator:		  '*' cv_qualifier_list_opt { $$="*"; }
			| '&' cv_qualifier_list_opt { $$="&"; }
			| complete_class_name
			  DBL_COLON
			  '*'
			  cv_qualifier_list_opt	{ $$=stradd($1,"*"); }
			;

cv_qualifier_list_opt:	   	/* empty */
			| cv_qualifier_list
			;

cv_qualifier_list:	  cv_qualifier
			| cv_qualifier_list cv_qualifier
			;

cv_qualifier:		  CONST
			| VOLATILE
			;

fct_body_opt:		  	/* empty */
			| fct_body
			;

fct_body:		  '{' {BEGIN INSIDE; level = 1;}
			  '}' {BEGIN CLASS_DEF;}
			;


/***** r.17.5 (ARM p.395): Class Declarations *****/

class_specifier:	  class_head
			  '{'			{ BEGIN INSIDE; level = 1; }
			  opt_obj_member_list
			  '}'			{ BEGIN CLASS_DEF; }
			;

class_head:		  class_key
			  class_name		{ className = $2; }
			  opt_base_spec		{ superclassName = $4; }
			;

opt_base_spec:		  	/* empty */	{ $$ = NULL; }
			| base_spec		{ $$ = $1; }
			;

opt_obj_member_list:	  	/* empty */
			| obj_member_list
			;

obj_member_list:	  obj_member_list obj_member_area
			| obj_member_area
			;

obj_member_area:	  METHODS		{ BEGIN CLASS_DEF; }
			  opt_method_declarations
			| SIGNALS		{ BEGIN CLASS_DEF; }
			  opt_signal_declarations
			| SLOTS			{ BEGIN CLASS_DEF; }
			  opt_slot_declarations
			| PRIVATE		{ BEGIN INSIDE; level = 1; }
			| PROTECTED		{ BEGIN INSIDE; level = 1; }
			| PUBLIC		{ BEGIN INSIDE; level = 1; }
			;

opt_method_declarations:	/* empty */
			| { errorControl=TRUE; }  method_declarations
			  { errorControl=FALSE; }
			;

method_declarations:	  method_declarations method_declaration
			| method_declaration
			;

method_declaration:	  decl_specifiers fct_decl optional_semicolon
						{ addMember($1,'m'); }
		 	| error ';'		{ yyerrok; }
			;

opt_signal_declarations:  	/* empty */
			| signal_declarations
			;

signal_declarations:	  signal_declarations signal_declaration
			| signal_declaration
			;

signal_declaration:	  decl_specifiers fct_decl optional_semicolon
						{ addMember($1,'s'); }
			;

opt_slot_declarations:		/* empty */
			| slot_declarations
			;

slot_declarations:	  slot_declarations slot_declaration
			| slot_declaration
			;

slot_declaration:	  decl_specifiers fct_decl optional_semicolon
						{ addMember($1,'t'); }
			;

optional_semicolon:	  	/* empty */
			| ';'
			;

base_spec:		  ':' base_list		{ $$=$2; }
			;

base_list		: base_list ',' base_specifier
			| base_specifier
			;

base_specifier:		  complete_class_name			       {$$=$1;}
			| VIRTUAL access_specifier complete_class_name {$$=$3;}
			| VIRTUAL complete_class_name		       {$$=$2;}
			| access_specifier VIRTUAL complete_class_name {$$=$3;}
			| access_specifier complete_class_name	       {$$=$2;}
			;

access_specifier:	  PRIVATE		{ $$=_PRIVATE; }
			| PROTECTED		{ $$=_PROTECTED; }
			| PUBLIC		{ $$=_PUBLIC; }
			;

%%

#include "lex.yy.c"


void init();					// initialize
void generate();				// generate C++ source code

QString	  fileName;				// file name
QString	  className;				// name of parsed class
QString	  superclassName;			// name of super class
FuncList  methods;				// method interface (public)
FuncList  signals;				// signal interface
FuncList  slots;				// slots interface

FILE  *out = stdout;				// output file

int yyparse();


int main( int argc, char **argv )		// program starts here
{
    if ( argc != 2 ) {
	fprintf( stderr, "Quasar meta object compiler\n" );
	fprintf( stderr, "Usage:  moc <header-file>\n" );
	return 1;
    }

    fileName = argv[1];
    yyin = fopen( fileName, "r" );
    if ( !yyin ) {
	fprintf( stderr, "moc: %s: No such file\n", (char*)fileName );
	return 1;
    }
    init();
    yyparse();
//    methods.clear();
//    signals.clear();
//    slots.clear();

    return 0;
}


void init()					// initialize
{
    BEGIN OUTSIDE;
    lineNo = 1;
    lexdebug = 0;
    yydebug = 0;
    errorControl = FALSE;
    methods.autoDelete( TRUE );
    signals.autoDelete( TRUE );
    slots.autoDelete( TRUE );

    tmpArgList = new ArgList;
    tmpFunc = new Function;
}

void yyerror( char *msg )			// print yacc error message
{
    if ( errorControl )
	fprintf( stderr, "moc: Ignoring definition on line %d\n",
lineNo );
    else
	fprintf( stderr, "moc: %s, line %d\n", msg, lineNo );
}


char *stradd( char *s1, char *s2 )		// adds two strings
{
    char *n = new char[strlen(s1)+strlen(s2)+1];
    strcpy( n, s1 );
    strcat( n, s2 );
    return n;
}


void generate()					// generate C++ source code
{
    char *hdr1 = "/****************************************************************************\n** Quasar object meta data from C++ file %s\n**\n** ";
    char *hdr2 = "Generated by Quasar meta object compiler (moc)\n**\n** ";
    char *hdr3 = "Warning: All changes will be lost!\n";
    char *hdr4 = "*****************************************************************************/\n\n";
    static int gen_count = 0;
    if ( gen_count++ == 0 ) {			// first class to be generated
	fprintf( out, hdr1, (char*)fileName );
	fprintf( out, hdr2 );
	fprintf( out, hdr3 );
	fprintf( out, hdr4 );
	fprintf( out, "#include <qmetaobj.h>\n" );
	fprintf( out, "#include \"%s\"\n\n\n", (char*)fileName );
    }
    else
        fprintf( out, "\n" );

    fprintf( out, "class QObject__%s : public QObject \n{\npublic:",
	     (char*)className );
    fprintf( out, "\n    void setSender( QObject *s ) { sender=s; }\n};\n\n" );

    fprintf( out, "char *%s::className() const\n{\n    ", (char*)className );
    fprintf( out, "return \"%s\";\n}\n\n", (char*)className );

    Function *f;

//
// Generate initMetaObject member function
//
    fprintf( out, "void %s::initMetaObject()\n{\n    ", (char*)className );
//
// Call to initialize parent meta object
//
    fprintf( out, "if ( getMetaObject(\"%s\") == 0 )\n",(char*)superclassName);
    fprintf( out, "\t%s::initMetaObject();\n", (char *)superclassName );
//
// Build methods array in initMetaObject()
//
    for ( f=methods.first(); f; f=methods.next() ) {
	QString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( count++ )
		typstr += ",";
	    typstr += a->typeName;
	    typstr += a->ptrType;
	    a = f->args->next();
	}
	fprintf( out, "    typedef %s (%s::*m1_t%d)(%s);\n", (char *)f->type,
		 (char*)className, methods.at(),(char*)typstr );
	f->type = f->name.copy();
	f->type += "(";
	f->type += typstr;
	f->type += ")";
    }
    if ( methods.count() )
	fprintf( out, "    static QMetaData method_tbl[] = {\n" );
    for ( f=methods.first(); f; f=methods.next() ) {
	fprintf( out, "\t{ \"%s\", (QMember)((m1_t%d)&%s::%s) }%s\n",
	         (char*)f->type, methods.at(), (char*)className,
		 (char*)f->name,
		 (methods.at() == methods.count()-1 ? "};" : ",") );
    }
//
// Build slots array in initMetaObject()
//
    for ( f=slots.first(); f; f=slots.next() ) {
	QString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( count++ )
		typstr += ",";
	    typstr += a->typeName;
	    typstr += a->ptrType;
	    a = f->args->next();
	}
	fprintf( out, "    typedef %s (%s::*m2_t%d)(%s);\n", (char *)f->type,
		 (char*)className, slots.at(),(char*)typstr );
	f->type = f->name.copy();
	f->type += "(";
	f->type += typstr;
	f->type += ")";
    }
    if ( slots.count() )
	fprintf( out, "    static QMetaData slot_tbl[] = {\n" );
    for ( f=slots.first(); f; f=slots.next() ) {
	fprintf( out, "\t{ \"%s\", (QMember)((m2_t%d)&%s::%s) }%s\n",
	         (char*)f->type, slots.at(), (char*)className,
		 (char*)f->name,
		 (slots.at() == slots.count()-1 ? "};" : ",") );
    }
//
// Build signals array in initMetaObject()
//
    for ( f=signals.first(); f; f=signals.next() ) {
	QString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( count++ )
		typstr += ",";
	    typstr += a->typeName;
	    typstr += a->ptrType;
	    a = f->args->next();
	}
	fprintf( out, "    typedef %s (%s::*m3_t%d)(%s);\n", (char *)f->type,
		 (char*)className, signals.at(),(char*)typstr );
	f->type = f->name.copy();
	f->type += "(";
	f->type += typstr;
	f->type += ")";
    }
    if ( signals.count() )
	fprintf( out, "    static QMetaData signal_tbl[] = {\n" );
    for ( f=signals.first(); f; f=signals.next() ) {
	fprintf( out, "\t{ \"%s\", (QMember)((m3_t%d)&%s::%s) }%s\n",
	         (char*)f->type, signals.at(), (char*)className,
		 (char*)f->name,
		 (signals.at() == signals.count()-1 ? "};" : ",") );
    }
//
// Finally create meta object
//
    fprintf( out, "    metaObject = new QMetaObject( \"%s\", \"%s\",\n",
	     (char*)className, (char*)superclassName );
    if ( methods.count() )
	fprintf( out, "\tmethod_tbl, %d,\n", methods.count() );
    else
        fprintf( out, "\t0, 0,\n" );
    if ( slots.count() )
	fprintf( out, "\tslot_tbl, %d,\n", slots.count() );
    else
        fprintf( out, "\t0, 0,\n" );
    if ( signals.count() )
	fprintf( out, "\tsignal_tbl, %d );\n", signals.count());
    else
        fprintf( out, "\t0, 0 );\n\n" );
    fprintf( out, "}\n\n" );

//
// End of function initMetaObject()
//

//
// Generate internal signal functions
//
    f = signals.first();			// make internal signal methods
    while ( f ) {
	QString typstr = "";			// type string
	QString valstr = "";			// value string
	QString argstr = "";			// argument string (type+value)
	int  count = 0;
	char buf[12];
						// method header
	fprintf( out, "// SIGNAL %s\n", (char*)f->name );
	fprintf( out, "void %s::%s( ", (char*)className, (char*)f->name );
	Argument *a = f->args->first();
	while ( a ) {				// argument list
	    if ( count++ ) {
		typstr += ",";
		valstr += ", ";
		argstr += ", ";
	    }
	    typstr += a->typeName;
	    typstr += a->ptrType;
	    argstr += a->typeName;
	    argstr += a->ptrType;
	    argstr += " ";
	    sprintf( buf, "t%d", count );
	    argstr += buf;
	    valstr += buf;
	    a = f->args->next();
	}

	fprintf( out, "%s )\n{\n", (char*)argstr );
	fprintf( out, "    typedef void (QObject::*RT)(%s);\n", (char*)typstr);
	fprintf( out, "    QConnection *c = receiver(\"%s(%s)\");\n", 
		 (char*)f->name, (char*)typstr );
	fprintf( out, "    if ( !c )\n\treturn;\n" );
	fprintf( out, "    RT r = (RT)(*(c->member()));\n" );
	fprintf( out, "    QObject__%s *object = (QObject__%s*)c->object();\n",
		 (char*)className, (char*)className );
	fprintf( out, "    object->setSender( this );\n" );
	fprintf( out, "    (object->*r)(%s);\n}\n\n", (char*)valstr );
/*
	fprintf( out, "	   Part_%s *owner = (Part_%s*)getOwner();\n",
		 (char*)className, (char*)className );
//	fprintf( out, "	   Part *owner = getOwner();\n" );
	fprintf( out, "	   owner->setSender(this);\n" );
	fprintf( out, "	   SPListIter it(*list);\n" );
	fprintf( out, "	   while ( it ) {\n" );
	fprintf( out, "\tPPS p = (PPS)(*it.get());\n" );
	fprintf( out, "\t(owner->*p)(%s);\n", (char*)valstr );
	fprintf( out, "\t++it;\n    }\n" );
	fprintf( out, "}\n\n" );
*/
	f = signals.next();
    }

    methods.clear();
    slots.clear();
    signals.clear();
}


ArgList *addArg( Argument *a )			// add argument to list
{
    tmpArgList->append( a );
    return tmpArgList;
}

void addMember( QString type, char m )
{
/*
    Function *n = new Function;
    n->name = f->name;
    n->type = type;
    n->ptrType = f->ptrType;
    n->lineNo = f->lineNo;
    n->args = tmpArgList;
*/
    tmpFunc->type = type;
    tmpFunc->args = tmpArgList;
    tmpArgList = new ArgList;
    switch( m ) {
	case 'm': methods.append( tmpFunc ); break;
	case 's': signals.append( tmpFunc ); break;
	case 't': slots.  append( tmpFunc ); break;
    }
    tmpFunc = new Function;
}
