/****************************************************************************
** $Id$
**
** Parser and code generator for meta object compiler
**
** Created : 930417
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
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
#define MOC_YACC_CODE
void yyerror(const char *msg);

#include "qplatformdefs.h"
#include "qdatetime.h"
#include "qhash.h"
#include "qlist.h"
#include "qfile.h"
#include "qdir.h"
#include "qstringlist.h"
#ifdef MOC_MWERKS_PLUGIN
# ifdef Q_OS_DARWIN
#  undef OLD_DEBUG
#  ifdef DEBUG
#   define OLD_DEBUG DEBUG
#   undef DEBUG
#  endif
#  define DEBUG 0
#  ifndef __IMAGECAPTURE__
#   define __IMAGECAPTURE__
#  endif
#  include <Carbon/Carbon.h>
# endif
# include "mwerks_mac.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined CONST
#undef CONST
#endif
#if defined VOID
#undef VOID
#endif

 typedef QList<QByteArray> QStrList;

static inline bool isIdentChar(char x)
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace(char x)
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.  isspace() usually
    works, but not here.  This implementation is sufficient for our
    internal use.
  */
    return (uchar) x <= 32;
#else
    return isspace((uchar) x);
#endif
}

static QByteArray normalizeTypeInternal(const char *t, const char *e, bool adjustConst = true){
    int len = e - t;
    if (strncmp("void", t, len) == 0)
	return QByteArray();
    /*
      Convert 'char const *' into 'const char *'. Start at index 1,
      not 0, because 'const char *' is already OK.
    */
    QByteArray constbuf;
    for (int i = 1; i < len; i++) {
	if (t[i] == 'c' &&
	     strncmp(t + i + 1, "onst", 4) == 0) {
	    constbuf = QByteArray(t, len);
	    if (isSpace(t[i-1]))
		constbuf.remove(i-1, 6);
	    else
		constbuf.remove(i, 5);
	    constbuf.prepend("const");
	    t = constbuf.data();
	    e = constbuf.data() + constbuf.length();
	    break;
	}
	/*
	  We musn't convert 'char * const *' into 'const char **'
	  and we must beware of 'Bar<const Bla>'.
	*/
	if (t[i] == '&' || t[i] == '*' ||t[i] == '<')
	    break;
    }
    if (adjustConst && e > t + 6 && strncmp("const ", t, 6) == 0) {
	if (*(e-1) == '&') { // treat const reference as value
	    t += 6;
	    --e;
	} else if (isIdentChar(*(e-1))) { // treat const value as value
	    t += 6;
	}
    }
    QByteArray result;
    result.reserve(len);

    // some type substitutions for 'unsigned x'
    if (strncmp("unsigned ", t, 9) == 0) {
	if (strncmp("int", t+9, 3) == 0) {
	    t += 9+3;
	    result += "uint";
	} else if (strncmp("long", t+9, 4) == 0) {
	    t += 9+4;
	    result += "ulong";
	}
    }
    while (t != e) {
	result += *t;
	if (*t == '<') {
	    //template recursion
	    const char* tt = ++t;
	    int templdepth = 1;
	    while (*++t) {
		if (*t == '<')
		    ++templdepth;
		if (*t == '>')
		    --templdepth;
		if (templdepth == 0) {
		    result += normalizeTypeInternal(tt, t, false);
		    result += *t;
		    if (t[1] == '>')
			result += ' '; // avoid >>
		    break;
		}
	    }
	}
	++t;
    }
    return result;
}

// only moc needs this function
static QByteArray normalizeType(const char *s)
{
    int len = strlen(s);
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len] : stackbuf);
    char *d = buf;
    char last = 0;
    while(*s && isSpace(*s))
	s++;
    while (*s) {
	while (*s && !isSpace(*s))
	    last = *d++ = *s++;
	while (*s && isSpace(*s))
	    s++;
	if (*s && isIdentChar(*s) && isIdentChar(last))
	    last = *d++ = ' ';
    }
    *d = '\0';
    QByteArray result = normalizeTypeInternal(buf, d);
    if (buf != stackbuf)
	delete [] buf;
    return result;
}



bool isEnumType(const char* type);
bool isFlagType(const char* type);
int enumIndex(const char* type);
bool isVariantType(const char* type);
int qvariant_nameToType(const char* name);

/*
  Attention!  This table is copied from qcorevariant.cpp. If you
  change one, change both.
*/
static const int ntypes = 35;
static const char* const type_map[ntypes] =
{
    0,
    "QMap<QString,QCoreVariant>",
    "QList<QCoreVariant>",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
#ifndef QT_NO_COMPAT
    "QColorGroup",
#else
    "",
#endif
    "QIconSet",
    "QPoint",
    "QImage",
    "int",
    "uint",
    "bool",
    "double",
    "",
    "QPointArray",
    "QRegion",
    "QBitmap",
    "QCursor",
    "QSizePolicy",
    "QDate",
    "QTime",
    "QDateTime",
    "QByteArray",
    "QBitArray",
    "QKeySequence",
    "QPen",
    "Q_LLONG",
    "Q_ULLONG"
};

int qvariant_nameToType(const char* name)
{
    if (name) {
	if ( strcmp(name, "QCString") == 0 )
	    name = "QByteArray";
	for (int i = 1; i < ntypes; i++) {
	    if (!strcmp(type_map[i], name))
		return i;
	}
    }
    return 0;
}

/*
  Returns TRUE if the type is a QVariant types.
*/
bool isVariantType(const char* type)
{
    return qvariant_nameToType(type) != 0;
}

static QByteArray rmWS(const char *);

static QByteArray rmRef(const QByteArray& s)
{
    if (!s.isEmpty() && s.at(s.length()-1) == '&')
	return s.left(s.length()-1);
    return s;
}

static bool isPropFunction(const QByteArray &s)
{
    return !s.isEmpty() && s != "true" && s != "false";
}

enum Access { Private, Protected, Public };


class Argument					// single arg meta data
{
public:
    Argument(const char *left, const char *right,
	      const char* name = 0, bool isDefault = FALSE)
	: leftType(rmWS(left)), rightType(rmWS(right)), name(name), isDefault(isDefault)
    {
	type = normalizeType(leftType + ' ' + rightType);
    }

    QByteArray leftType;
    QByteArray rightType;
    QByteArray name;
    bool isDefault;

    QByteArray type;
};

class ArgList : public QList<Argument *> {	// member function arg list
    bool cloned;
    int defargs;
public:
    ArgList() : cloned(FALSE), defargs(0) { setAutoDelete(TRUE); }
    ~ArgList() { clear(); }


    /* the clone has one default argument less, the orignal has all default arguments removed */
    ArgList* magicClone() {
	Argument *firstDefault = 0;
	for (int i = 0; i < size(); ++i) {
	    if (at(i)->isDefault) {
		firstDefault = at(i);
		break;
	    }
	}
	if (!firstDefault)
	    return 0;
	if (!defargs) {
	    for (int i = 0; i < size(); ++i) {
		if (at(i)->isDefault)
		    ++defargs;
	    }
	}
	cloned = TRUE;
	ArgList* l = new ArgList;
	l->defargs = defargs;
	for (int i = 0; i < size(); ++i) {
	    Argument *current = at(i);
	    l->append(new Argument(current->leftType,
				   current->rightType,
				   current->name,
				   (current != firstDefault && current->isDefault)));
	}
	for (int i = size()-1; i >= 0; --i) {
	    if (at(i)->isDefault)
		removeAt(i);
	}
	return l;
    }


    inline int defaultArguments() const { return defargs; }
    inline bool wasCloned() const { return cloned; }
};


struct Function					// member function meta data
{
    Access access;
    QByteArray qualifier; // const or volatile
    QByteArray name;
    QByteArray type;
    QByteArray rawType;
    QByteArray tag;
    int lineNo;
    ArgList *args;
    Function() { args=0;}
   ~Function() { delete args; }
};

class FuncList : public QList<Function *> {	// list of member functions
public:
    FuncList(bool autoDelete = FALSE) { setAutoDelete(autoDelete); }

    FuncList find(const char* name)  {
	FuncList result;
	for (int i = 0; i < size(); ++i) {
	    Function *f = at(i);
	    if (f->name == name)
		result.append(f);
	}
	return result;
    }

    void insertSorted(Function *f) {
	int i = 0;
	// ### could be done with a binary search
	for (; i < size(); ++i)
	    if (at(i)->name >= f->name)
		break;
	insert(i, f);
    }
};

class Enum : public QStrList
{
public:
    QByteArray name;
    bool set;
};

class EnumList : public QList<Enum *> {		// list of property enums
public:
    EnumList() { setAutoDelete(TRUE); }
};


struct Property
{
    Property(int lineNo_,
	     const QByteArray &typearg,
	     const QByteArray &name,
	     const QByteArray &read,
	     const QByteArray &write,
	     const QByteArray &reset,
	     const QByteArray &designable,
	     const QByteArray &scriptable,
	     const QByteArray &stored,
	     const QByteArray &editable,
	     bool override)
	: lineNo(lineNo_), type(typearg), name(name),
	  read(read), write(write), reset(reset),
	  designable(designable),
	  scriptable(scriptable),
	  stored(stored),
	  editable(editable),
	  override(override),
	  gspec(ValueSpec)
    {
	/*
	  The Q_PROPERTY construct cannot contain any commas, since
	  commas separate macro arguments. We therefore expect users
	  to type "QMap" instead of "QMap<QString, QVariant>". For
	  coherence, we also expect the same for
	  QValueList<QVariant>, the other template class supported by
	  QVariant.
	*/
	type = normalizeType(type);
	if (type == "QMap") {
	    type = "QMap<QString,QVariant>";
	} else if (type == "QValueList") {
	    type = "QValueList<QVariant>";
	} else if (type == "LongLong") {
	    type = "Q_LLONG";
	} else if (type == "ULongLong") {
	    type = "Q_ULLONG";
	}
    }

    int lineNo;
    QByteArray type;
    QByteArray name;
    QByteArray read;
    QByteArray write;
    QByteArray reset;
    QByteArray designable;
    QByteArray scriptable;
    QByteArray stored;
    QByteArray editable;
    bool override;

    enum Specification  { ValueSpec, ReferenceSpec, PointerSpec };
    Specification gspec;

    bool stdCppSet() {
	QByteArray s("set");
	s += toupper(name[0]);
	s += name.mid(1);
	return s == write;
    }
};

class PropList : public QList<Property *> {	// list of properties
public:
    PropList() { setAutoDelete(TRUE); }
};


struct ClassInfo
{
    ClassInfo(const char* n, const char* v)
	: name(n), value(v)
    {}
    QByteArray name;
    QByteArray value;
};

class ClassInfoList : public QList<ClassInfo *> {	// list of class infos
public:
    ClassInfoList() { setAutoDelete(TRUE); }
};

ArgList *addArg(Argument *);			// add arg to tmpArgList

enum Member { SignalMember,
	      SlotMember,
	      PropertyCandidateMember
	    };

void	 addMember(Member);			// add tmpFunc to current class
void     addEnum();				// add tmpEnum to current class

int      strreg(const char *);		// registers a string and returns its id
char	*strnew(const char *);		// returns a new string (copy)
char	*stradd(const char *, const char *);	// add two strings
char	*stradd(const char *, const char *,	// add three strings
			       const char *);
char	*straddSpc(const char *, const char *);
char	*straddSpc(const char *, const char *,
			       const char *);
char	*straddSpc(const char *, const char *,
		    const char *, const char *);

extern int yydebug;
bool	   lexDebug	   = FALSE;
int	   lineNo;			// current line number
bool	   errorControl	   = FALSE;	// controled errors
bool	   displayWarnings = TRUE;
bool	   skipClass;			// don't generate for class
bool	   skipFunc;			// don't generate for func
bool	   templateClass;		// class is a template
bool	   templateClassOld;		// previous class is a template

ArgList	  *tmpArgList;			// current argument list
Function  *tmpFunc;			// current member function
Enum      *tmpEnum;			// current enum
Access tmpAccess;			// current access permission
Access subClassPerm;			// current access permission

bool	   Q_OBJECTdetected;		// TRUE if current class
					//  contains the Q_OBJECT macro
bool	   Q_PROPERTYdetected;		// TRUE if current class
					//  contains at least one Q_PROPERTY,
					//  Q_OVERRIDE, Q_FLAGS or Q_ENUMS macro
bool	   tmpPropOverride;		// current property override setting

int	   tmpYYStart;			// Used to store the lexers current mode
int	   tmpYYStart2;			// Used to store the lexers current mode
					//  (if tmpYYStart is already used)

// if the format revision changes, you MUST change it in qmetaobject.h too
const int formatRevision = 45;		// moc output format revision

// if the flags change, you HAVE to change it in qmetaobject.h too
enum ProperyFlags  {
    Invalid		= 0x00000000,
    Readable		= 0x00000001,
    Writable		= 0x00000002,
    Resetable		= 0x00000004,
    EnumOrFlag		= 0x00000008,
    StdCppSet		= 0x00000100,
    Override		= 0x00000200,
    Designable		= 0x00001000,
    ResolveDesignable	= 0x00002000,
    Scriptable		= 0x00004000,
    ResolveScriptable	= 0x00008000,
    Stored		= 0x00010000,
    ResolveStored	= 0x00020000,
    Editable		= 0x00040000,
    ResolveEditable	= 0x00080000
};

%}




%union {
    char	char_val;
    int		int_val;
    double	double_val;
    char       *string;
    Access	access;
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
%token			THROW

%token			SIGNALS
%token			SLOTS
%token			Q_OBJECT
%token			Q_PROPERTY
%token			Q_OVERRIDE
%token			Q_CLASSINFO
%token			Q_ENUMS
%token			Q_FLAGS
%token			ATTRIBUTE

%token			READ
%token			WRITE
%token			RESET
%token			DESIGNABLE
%token			SCRIPTABLE
%token			STORED
%token			EDITABLE

%type  <string>		class_modifs
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
%type  <string>		whatever

%type  <arg_list>	argument_declaration_list
%type  <arg_list>	arg_declaration_list
%type  <arg_list>	arg_declaration_list_opt
%type  <string>		abstract_decl_opt
%type  <string>		abstract_decl
%type  <arg>		argument_declaration
%type  <arg>		opt_exception_argument
%type  <string>		cv_qualifier_list_opt
%type  <string>		cv_qualifier_list
%type  <string>		cv_qualifier
%type  <string>		decl_specifiers
%type  <string>		decl_specifier
%type  <string>		decl_specs_opt
%type  <string>		decl_specs
%type  <string>		type_specifier
%type  <string>		fct_specifier
%type  <string>		declarator
%type  <string>		ptr_operator
%type  <string>		ptr_operators
%type  <string>		ptr_operators_opt

%type  <string>		dname

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


/***** r.17.1 (ARM p.387): Keywords	*****/

class_name:		  IDENTIFIER	      { $$ = $1; }
			| template_class_name { $$ = $1; }
			;

template_class_name:	  IDENTIFIER '<' template_args '>'
				   { g->tmpExpression = rmWS(g->tmpExpression);
				     $$ = stradd($1, "<",
						  g->tmpExpression, ">"); }
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

/* def_argument is just like const expression but handles the ","
   slightly differently. It was added for 3.0 as quick solution. TODO:
   merge with const_expression.
 */

def_argument:	   /* empty */		  { BEGIN IN_DEF_ARG; }
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
                        | ATTRIBUTE               { $$ = ""; }
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

fct_specifier:		  INLINE                { }
			| VIRTUAL               { }
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
                        ;

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
				   { g->tmpExpression = rmWS(g->tmpExpression);
				     $$ = stradd("template<",
						  g->tmpExpression, ">"); }
			;

opt_template_spec:	  /* empty */
			| template_spec		{ templateClassOld = templateClass;
						  templateClass = TRUE;
						}
			;


class_key:		  opt_template_spec CLASS	{ $$ = "class"; }
			| opt_template_spec STRUCT	{ $$ = "struct"; }
			;

complete_class_name:	  qualified_class_name	{ $$ = $1; }
			| DBL_COLON  qualified_class_name
						{ $$ = stradd("::", $2); }
			;

qualified_class_name:	  qualified_class_name DBL_COLON class_name
						{ $$ = stradd($1, "::", $3);}
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
				       func_warn("Ellipsis not supported"
						 " in signals and slots.\n"
						 "Ellipsis argument ignored."); }
			;

arg_declaration_list_opt:	/* empty */	{ $$ = tmpArgList; }
			| arg_declaration_list	{ $$ = $1; }
			;

opt_exception_argument:		/* empty */     { $$ = 0; }
			| argument_declaration
			;

triple_dot_opt:			/* empty */
			| TRIPLE_DOT { func_warn("Ellipsis not supported"
						 " in signals and slots.\n"
						 "Ellipsis argument ignored."); }

			;

arg_declaration_list:	  arg_declaration_list
			  ','
			  argument_declaration	{ $$ = addArg($3); }
			| argument_declaration	{ $$ = addArg($1); }
			;

argument_declaration:	  decl_specifiers abstract_decl_opt
				{ $$ = new Argument(straddSpc($1,$2),""); }
			| decl_specifiers abstract_decl_opt
			  '=' { expLevel = 1; }
			  def_argument
				{ $$ = new Argument(straddSpc($1,$2),"", 0, TRUE); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
				{ $$ = new Argument(straddSpc($1,$2),$4, $3); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
			  '='	{ expLevel = 1; }
			  def_argument
				{ $$ = new Argument(straddSpc($1,$2),$4, $3, TRUE); }
			;


abstract_decl_opt:	  /* empty */		{ $$ = ""; }
			| abstract_decl		{ $$ = $1; }
			;

abstract_decl:		  abstract_decl ptr_operator
						{ $$ = straddSpc($1,$2); }
			| '['			{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd("[",
				     g->tmpExpression =
				     g->tmpExpression.trimmed(), "]"); }
			| abstract_decl '['	{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd($1,"[",
				     g->tmpExpression =
				     g->tmpExpression.trimmed(),"]"); }
			| ptr_operator		{ $$ = $1; }
			| '(' abstract_decl ')' { $$ = $2; }
			;

declarator:		  dname			{ $$ = ""; }
			| declarator ptr_operator
						{ $$ = straddSpc($1,$2);}
			| declarator '['	{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd($1,"[",
				     g->tmpExpression =
				     g->tmpExpression.trimmed(),"]"); }
			| '(' declarator ')'	{ $$ = $2; }
			;

dname:			  IDENTIFIER
			;

fct_decl:		  '('
			  argument_declaration_list
			  ')'
			  cv_qualifier_list_opt
			  ctor_initializer_opt
			  exception_spec_opt
                          opt_identifier
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
			;

fct_body_or_semicolon:	  ';'
			| fct_body
			| '=' INT_VAL ';'   /* abstract func, INT_VAL = 0 */
			;

fct_body:		  '{' { BEGIN IN_FCT; fctLevel = 1;}
			  '}' { BEGIN QT_DEF; }
			;


/***** r.17.5 (ARM p.395): Class Declarations *****/

class_specifier:	  full_class_head
			  '{'			{ BEGIN IN_CLASS;
						  classPLevel = 1;
						}
			  opt_obj_member_list
			  '}'			{ BEGIN QT_DEF; } /*catch ';'*/
			| class_head		{ BEGIN QT_DEF;	  /* -- " -- */
						  skipClass = TRUE; }
 			| class_head '*' IDENTIFIER	{ BEGIN QT_DEF;	  /* -- " -- */
 						  skipClass = TRUE; }
 			| class_head '&' IDENTIFIER	{ BEGIN QT_DEF;	  /* -- " -- */
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
			| storage_class_specifier { $$ = ""; }
			| fct_specifier
			;


class_modifs:             IDENTIFIER { $$ = ""; } /* possible DLL EXPORT macro and other attributes */
                        | ATTRIBUTE { $$ = ""; }
                        | IDENTIFIER ATTRIBUTE { $$ = ""; }
                        ;

class_head:		  class_key
			  qualified_class_name	{ g->className = $2;
						  if (g->className == "QObject")
						     Q_OBJECTdetected = TRUE;
						}
			| class_key
			  class_modifs
			  class_name		{ g->className = $3;
						  if (g->className == "QObject")
						     Q_OBJECTdetected = TRUE;
						}
			;

full_class_head:	  class_head
			  opt_base_spec		{ g->superClassName = $2; }
			;

nested_class_head:	  class_key
			  qualified_class_name
			  opt_base_spec		{ templateClass = templateClassOld; }
                        ;

exception_spec_opt:		/* empty */
			| exception_spec
			;

/* looser than the real thing */
exception_spec:		THROW '(' opt_exception_argument ')'
			;

ctor_initializer_opt:		/* empty */
			| ctor_initializer
			;

ctor_initializer:	':' mem_initializer_list
			;

mem_initializer_list:	mem_initializer
			| mem_initializer ',' mem_initializer_list
			;

/* complete_class_name also represents IDENTIFIER */
mem_initializer:	complete_class_name '('	{ expLevel = 1; }
			const_expression ')'
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


qt_access_specifier:	  access_specifier	{ tmpAccess = $1; }
			| SLOTS	      { moc_err("Missing access specifier"
						   " before \"slots:\"."); }
			;

obj_member_area:	  qt_access_specifier	{ BEGIN QT_DEF; }
			  slot_area
			| SIGNALS		{ BEGIN QT_DEF; }
			  ':'  opt_signal_declarations
			| Q_OBJECT		{
			      if (tmpAccess)
				  moc_warn("Q_OBJECT is not in the private"
					   " section of the class.\n"
					   "Q_OBJECT is a macro that resets"
					   " access permission to \"private\".");
			      Q_OBJECTdetected = TRUE;
			  }
			| Q_PROPERTY { tmpYYStart = YY_START;
				       tmpPropOverride = FALSE;
				       BEGIN IN_PROPERTY; }
			  '(' property ')' {
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			| Q_OVERRIDE { tmpYYStart = YY_START;
				       tmpPropOverride = TRUE;
				       BEGIN IN_PROPERTY; }
			  '(' property ')' {
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			| Q_CLASSINFO { tmpYYStart = YY_START; BEGIN IN_CLASSINFO; }
			  '(' STRING ',' STRING ')'
				  {
				      g->infos.append(new ClassInfo($4, $6));
				      BEGIN tmpYYStart;
				  }
			  opt_property_candidates
			| Q_ENUMS { tmpYYStart = YY_START; BEGIN IN_PROPERTY; }
			  '(' qt_enums ')' {
						Q_PROPERTYdetected = TRUE;
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			| Q_FLAGS { tmpYYStart = YY_START; BEGIN IN_PROPERTY; }
			  '(' qt_sets ')' {
						Q_PROPERTYdetected = TRUE;
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			;

slot_area:		  SIGNALS ':'	{ moc_err("Signals cannot "
						 "have access specifiers"); }
			| SLOTS	  ':' opt_slot_declarations
			| ':'		{ if (tmpAccess == Public && Q_PROPERTYdetected)
						  BEGIN QT_DEF;
					      else
						  BEGIN IN_CLASS;
					  suppress_func_warn = TRUE;
					}
			  opt_property_candidates
					{
					  suppress_func_warn = FALSE;
					}
			| IDENTIFIER	{ BEGIN IN_CLASS;
					   if (classPLevel != 1)
					       moc_warn("unexpected access"
							 "specifier");
					}
			;

opt_property_candidates:	  /*empty*/
				| property_candidate_declarations
			;

property_candidate_declarations:	  property_candidate_declarations property_candidate_declaration
					| property_candidate_declaration
				;

property_candidate_declaration:	signal_or_slot { addMember(PropertyCandidateMember); }
				;

opt_signal_declarations:	/* empty */
			| signal_declarations
			;

signal_declarations:	  signal_declarations signal_declaration
			| signal_declaration
			;


signal_declaration:	  signal_or_slot	{ addMember(SignalMember); }
			;

opt_slot_declarations:		/* empty */
			| slot_declarations
			;

slot_declarations:	  slot_declarations slot_declaration
			| slot_declaration
			;

slot_declaration:	  signal_or_slot	{ addMember(SlotMember); }
			;

opt_semicolons:			/* empty */
			| opt_semicolons ';'
			;

base_spec:		  ':' base_list		{ $$=$2; }
			;

base_list		: base_list ',' base_specifier  { g->multipleSuperClasses.append($3); }
			| base_specifier
			;

qt_macro_name:		  IDENTIFIER '(' IDENTIFIER ')'
					   { $$ = stradd($1, "(", $3, ")"); }
			| IDENTIFIER '(' simple_type_name ')'
					   { $$ = stradd($1, "(", $3, ")"); }
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

access_specifier:	  PRIVATE		{ $$=Private; }
			| PROTECTED		{ $$=Protected; }
			| PUBLIC		{ $$=Public; }
			;

operator_name:		  decl_specs_opt IDENTIFIER ptr_operators_opt { }
			| decl_specs_opt simple_type_name ptr_operators_opt { }
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
			| type_name IDENTIFIER fct_name
						{ tmpFunc->type = $1;
						  tmpFunc->tag = $2;
						  tmpFunc->name = $3;
						}
			| fct_name
						{ tmpFunc->type = "int";
						  tmpFunc->name = $1;
				  if (tmpFunc->name == g->className)
				      func_warn("Constructors cannot be"
						 " signals or slots.");
						}
			| opt_virtual '~' fct_name
						{ tmpFunc->type = "void";
						  tmpFunc->name = "~";
						  tmpFunc->name += $3;
				       func_warn("Destructors cannot be"
						  " signals or slots.");
						}
			| decl_specs type_name decl_specs_opt
			  ptr_operators_opt fct_name
						{
						    char *tmp =
							straddSpc($1,$2,$3,$4);
						    tmpFunc->type = rmWS(tmp);
						    delete [] tmp;
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


signal_or_slot:		type_and_name fct_decl opt_semicolons
			| type_and_name opt_bitfield ';' opt_semicolons
				{ func_warn("Unexpected variable declaration."); }
			| type_and_name opt_bitfield ','member_declarator_list
			  ';' opt_semicolons
				{ func_warn("Unexpected variable declaration."); }
			| enum_specifier opt_identifier ';' opt_semicolons
				{ func_warn("Unexpected enum declaration."); }
			| USING complete_class_name ';' opt_semicolons
				{ func_warn("Unexpected using declaration."); }
			| USING NAMESPACE complete_class_name ';' opt_semicolons
				{ func_warn("Unexpected using declaration."); }
			| NAMESPACE IDENTIFIER '{'
				{ classPLevel++;
				  moc_err("Unexpected namespace declaration."); }
			| nested_class_head ';' opt_semicolons
				{ func_warn("Unexpected class declaration.");}
			| nested_class_head
			  '{'   { func_warn("Unexpected class declaration.");
				  BEGIN IN_FCT; fctLevel=1;
				}
			  '}'  { BEGIN QT_DEF; }
			  ';' opt_semicolons
			;


member_declarator_list:	  member_declarator
			| member_declarator_list ',' member_declarator
			;

member_declarator:	  declarator             { }
			| IDENTIFIER ':'	 { expLevel = 0; }
			  const_expression
			| ':'			 { expLevel = 0; }
			  const_expression
			;

opt_bitfield:		  /* empty */
			| ':'			 { expLevel = 0; }
			  const_expression
			;


enum_specifier:		  ENUM enum_tail
			;

/* optional final comma at the end of an enum list. Not really C++ but
some people still use it */
opt_komma:		  /*empty*/
			| ','
			;

enum_tail:		  IDENTIFIER '{'   enum_list opt_komma
			  '}'   { BEGIN QT_DEF;
				  if (tmpAccess == Public) {
				      tmpEnum->name = $1;
				      addEnum();
				  }
				}
			| '{'   enum_list opt_komma
			  '}'   { tmpEnum->clear();}
			;

opt_identifier:		  /* empty */
			| IDENTIFIER 		 { }
			;

enum_list:		  /* empty */
			| enumerator
			| enum_list ',' enumerator
			;

enumerator:		  IDENTIFIER { if (tmpAccess == Public) tmpEnum->append($1); }
			| IDENTIFIER '=' { enumLevel=0; }
			  enumerator_expression {  if (tmpAccess == Public) tmpEnum->append($1);  }
			;

property:		IDENTIFIER IDENTIFIER
				{
				     g->propWrite = "";
				     g->propRead = "";
				     g->propOverride = tmpPropOverride;
				     g->propReset = "";
				     if (g->propOverride) {
					 g->propDesignable = "";
					 g->propScriptable = "";
					 g->propStored = "";
					 g->propEditable = "";
				     } else {
					 g->propDesignable = "true";
					 g->propScriptable = "true";
					 g->propStored = "true";
					 g->propEditable = "true";
				     }
				}
			prop_statements
				{
				    if (g->propRead.isEmpty() && !g->propOverride)
					moc_err("A property must at least feature a read method.");
				    checkPropertyName($2);
				    Q_PROPERTYdetected = TRUE;
				    // Avoid duplicates
				    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
					Property *p = g->props.at(propindex);
					if (p->name == $2) {
					    if (displayWarnings)
						moc_err("Property '%s' defined twice.",
							 (const char*)p->name);
					}
				    }
				    g->props.append(new Property(lineNo, $1, $2,
								 g->propRead,
								 g->propWrite,
								 g->propReset,
								 g->propDesignable,
								 g->propScriptable,
								 g->propStored,
								 g->propEditable,
								 g->propOverride));
				}
			;

prop_statements:	  /* empty */
			| READ IDENTIFIER prop_statements { g->propRead = $2; }
			| WRITE IDENTIFIER prop_statements { g->propWrite = $2; }
			| RESET IDENTIFIER prop_statements { g->propReset = $2; }
			| DESIGNABLE IDENTIFIER prop_statements { g->propDesignable = $2; }
			| SCRIPTABLE IDENTIFIER prop_statements { g->propScriptable = $2; }
			| STORED IDENTIFIER prop_statements { g->propStored = $2; }
			| EDITABLE IDENTIFIER prop_statements { g->propEditable = $2; }
			;

qt_enums:		  /* empty */ { }
			| IDENTIFIER qt_enums { g->qtEnums.append($1); }
			;

qt_sets:		  /* empty */ { }
			| IDENTIFIER qt_sets { g->qtFlags.append($1); }
			;

%%

#if defined(Q_OS_WIN32)
#include <io.h>
#undef isatty
extern "C" int hack_isatty(int)
{
    return 0;
}
#define isatty hack_isatty
#else
#include <unistd.h>
#endif

#include "moc_lex.cpp"

void	  init();				// initialize
void      cleanup();
void	  initClass();				// prepare for new class
void	  generateClass();			// generate C++ code for class
void	  initExpression();			// prepare for new expression
QByteArray  combinePath(const char *, const char *);

class parser_reg {
 public:
    parser_reg();
    ~parser_reg();

    // some temporary values
    QByteArray   tmpExpression;			// Used to store the characters the lexer
						// is currently skipping (see addExpressionChar and friends)
    QByteArray  fileName;				// file name
    QByteArray  outputFile;				// output file name
    QStrList  includeFiles;			// name of #include files
    QByteArray  pchFile;				// name of PCH file (used on Windows)
    QByteArray  includePath;				// #include file path
    QByteArray  qtPath;				// #include qt file path
    int           gen_count; //number of classes generated
    bool	  noInclude;		// no #include <filename>
    bool	  generatedCode;		// no code generated
    bool	  mocError;			// moc parsing error occurred
    QByteArray  stringDataArray;		// name of the shared string data array
    QByteArray  className;				// name of parsed class
    QByteArray  superClassName;			// name of first super class
    QStrList  multipleSuperClasses;			// other superclasses
    FuncList  signals;				// signal interface
    FuncList  slots;				// slots interface
    FuncList  propfuncs;				// all possible property access functions
    FuncList  funcs;			// all parsed functions, including signals
    EnumList  enums;				// enums used in properties
    PropList  props;				// list of all properties
    ClassInfoList	infos;				// list of all class infos
    QStrList  strings;

// Used to store the values in the Q_PROPERTY macro
    QByteArray propRead;				// get function
    QByteArray propWrite;				// set function
    QByteArray propReset;				// reset function
    QByteArray propDesignable;				// "true", "false" or function or empty if not specified
    QByteArray propScriptable;				// "true", "false" or function or empty if not specified
    QByteArray propStored;				// "true", "false" or function or empty if not specified
    QByteArray propEditable;				// "true", "false" or function or empty if not specified
    bool propOverride;				// Wether OVERRIDE was detected

    QStrList qtEnums;				// Used to store the contents of Q_ENUMS
    QStrList qtFlags;				// Used to store the contents of Q_FLAGS

};
FILE  *out;					// output file
static parser_reg *g = NULL;

parser_reg::parser_reg() : funcs(TRUE)
{
    gen_count = 0;
    noInclude     = FALSE;		// no #include <filename>
    generatedCode = FALSE;		// no code generated
    mocError = FALSE;			// moc parsing error occurred
}


parser_reg::~parser_reg()
{}

int yyparse();

void replace(char *s, char c1, char c2);

void setDefaultIncludeFile()
{
    if (g->includeFiles.isEmpty()) {
	if (g->includePath.isEmpty()) {
	    if (!g->fileName.isEmpty() && !g->outputFile.isEmpty()) {
		g->includeFiles.append(combinePath(g->fileName, g->outputFile));
	    } else {
		g->includeFiles.append(g->fileName);
	    }
	} else {
	    g->includeFiles.append(combinePath(g->fileName, g->fileName));
	}
    }
}

#ifdef Q_CC_MSVC
#define ErrorFormatString "%s(%d):"
#else
#define ErrorFormatString "%s:%d:"
#endif

#ifndef MOC_MWERKS_PLUGIN
int main(int argc, char **argv)
{
    init();

    bool autoInclude = TRUE;
    const char *error	     = 0;
    g->qtPath = "";
    for (int n=1; n<argc && error==0; n++) {
	QByteArray arg(argv[n]);
	if (arg[0] == '-') {			// option
	    QByteArray opt = arg.mid(1);
	    if (opt[0] == 'o') {		// output redirection
		if (opt[1] == '\0') {
		    if (!(n < argc-1)) {
			error = "Missing output file name";
			break;
		    }
		    g->outputFile = argv[++n];
		} else
		    g->outputFile = opt.mid(1);
	    } else if (opt == "i") {		// no #include statement
		g->noInclude   = TRUE;
		autoInclude = FALSE;
	    } else if (opt[0] == 'f') {	// produce #include statement
		g->noInclude   = FALSE;
		autoInclude = FALSE;
		if (opt[1])			// -fsomething.h
		    g->includeFiles.append(opt.mid(1));
	    } else if (opt == "pch") {	// produce #include statement for PCH
		if (!(n < argc-1)) {
		    error = "Missing name of PCH file";
		    break;
		}
		g->pchFile = argv[++n];
	    } else if (opt[0] == 'p') {	// include file path
		if (opt[1] == '\0') {
		    if (!(n < argc-1)) {
			error = "Missing path name for the -p option.";
			break;
		    }
		    g->includePath = argv[++n];
		} else {
		    g->includePath = opt.mid(1);
		}
	    } else if (opt[0] == 'q') {	// qt include file path
		if (opt[1] == '\0') {
		    if (!(n < argc-1)) {
			error = "Missing path name for the -q option.";
			break;
		    }
		    g->qtPath = argv[++n];
		} else {
		    g->qtPath = opt.mid(1);
		}
		replace(g->qtPath.data(),'\\','/');
		if (g->qtPath.right(1) != "/")
		    g->qtPath += '/';
	    } else if (opt == "v") {		// version number
		fprintf(stderr, "Qt Meta Object Compiler version %d"
				 " (Qt %s)\n", formatRevision,
				 QT_VERSION_STR);
		cleanup();
		return 1;
	    } else if (opt == "k") {		// stop on errors
		errorControl = TRUE;
	    } else if (opt == "nw") {		// don't display warnings
		displayWarnings = FALSE;
	    } else if (opt == "ldbg") {	// lex debug output
		lexDebug = TRUE;
	    } else if (opt == "ydbg") {	// yacc debug output
		yydebug = TRUE;
	    } else {
		error = "Invalid argument";
	    }
	} else {
	    if (!g->fileName.isNull())		// can handle only one file
		error = "Too many input files specified";
	    else
		g->fileName = arg;
	}
    }

    if (autoInclude) {
	int ppos = g->fileName.lastIndexOf('.');
	if (ppos != -1 && tolower(g->fileName[ppos + 1]) == 'h')
	    g->noInclude = FALSE;
	else
	    g->noInclude = TRUE;
    }
    setDefaultIncludeFile();

    if (g->fileName.isNull() && !error) {
	g->fileName = "standard input";
	yyin	 = stdin;
    } else if (argc < 2 || error) {		// incomplete/wrong args
	fprintf(stderr, "Qt meta object compiler\n");
	if (error)
	    fprintf(stderr, "moc: %s\n", error);
	fprintf(stderr, "Usage:  moc [options] <header-file>\n"
		 "\t-o file    Write output to file rather than stdout\n"
		 "\t-pch file  Add #include \"file\" to beginning of output\n"
		 "\t-f[file]   Force #include, optional file name\n"
		 "\t-p path    Path prefix for included file\n"
		 "\t-i         Do not generate an #include statement\n"
		 "\t-k         Do not stop on errors\n"
		 "\t-nw        Do not display warnings\n"
		 "\t-v         Display version of moc\n" );
	cleanup();
	return 1;
    } else {
	yyin = fopen((const char *)g->fileName, "r");
	if (!yyin) {
	    fprintf(stderr, "moc: %s: No such file\n", (const char*)g->fileName);
	    cleanup();
	    return 1;
	}
    }
    if (!g->outputFile.isEmpty()) {		// output file specified
	out = fopen((const char *)g->outputFile, "w");	// create output file
	if (!out) {
	    fprintf(stderr, "moc: Cannot create %s\n",
		     (const char*)g->outputFile);
	    cleanup();
	    return 1;
	}
    } else {					// use stdout
	out = stdout;
    }
    yyparse();
    fclose(yyin);
    if (!g->outputFile.isNull())
	fclose(out);

    if (!g->generatedCode && displayWarnings && !g->mocError) {
	fprintf(stderr, ErrorFormatString" Warning: %s\n", g->fileName.data(), 0,
		 "No relevant classes found. No output generated.");
    }

    int ret = g->mocError ? 1 : 0;
    cleanup();
    return ret;
}
#else
bool qt_is_gui_used = FALSE;
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "CWPluginErrors.h"
#include <CWPlugins.h>
#include "DropInCompilerLinker.h"
#include <stat.h>

const unsigned char *p_str(const char *, int =-1);

CWPluginContext g_ctx;

moc_status do_moc(CWPluginContext ctx, const QByteArray &fin, const QByteArray &fout, CWFileSpec *dspec, bool i)
{
    init();

    g_ctx = ctx;
    g->noInclude = i;
    g->fileName = fin;
    g->outputFile = fout;

    setDefaultIncludeFile();

    CWFileInfo fi;
    memset(&fi, 0, sizeof(fi));
	fi.fullsearch = TRUE;
	fi.dependencyType = cwNormalDependency;
	fi.isdependentoffile = kCurrentCompiledFile;
    if(CWFindAndLoadFile(ctx, fin.data(), &fi) != cwNoErr) {
	cleanup();
	return moc_no_source;
    }

    if(dspec) {
	memcpy(dspec, &fi.filespec, sizeof(fi.filespec));
	const unsigned char *f = p_str(fout.data());
	memcpy(dspec->name, f, f[0]+1);
	free(f);
    }
    buf_size_total = fi.filedatalength;
    buf_buffer = fi.filedata;

    QByteArray path("");
    AliasHandle alias;
    Str63 str;
    AliasInfoType x = 1;
    char tmp[sizeof(Str63)+2];
    if(NewAlias(NULL, &fi.filespec, &alias) != noErr) {
	cleanup();
	return moc_general_error;
    }
    for(;;) {
	 GetAliasInfo(alias, x++, str);
	 if(!str[0])
	    break;
	 strncpy((char *)tmp, (const char *)str+1, str[0]);
	 tmp[str[0]] = '\0';
	 path.prepend(":");
	 path.prepend((char *)tmp);
    }
    path.prepend("MacOS 9:"); //FIXME

    QString inpath = path + fin, outpath = path + fout;
    struct stat istat, ostat;
    if(stat(inpath, &istat) == -1) {
	cleanup();
	return moc_no_source;
    }
    if(stat(outpath, &ostat) == 0 && istat.st_mtime < ostat.st_mtime) {
	cleanup();
	return moc_not_time;
    }

    unlink(outpath.data());
    out = fopen(outpath.data(), "w+");
    if(!out) {
	cleanup();
	return moc_general_error;
    }

    yyparse();
    if(out != stdout)
      fclose(out);

   if(g->mocError || !g->generatedCode) {
	unlink(outpath.data());
	moc_status ret = !g->generatedCode ? moc_no_qobject : moc_parse_error;
	cleanup();
	return ret;
    }

    cleanup();
    return moc_success;
}
#endif
void replace(char *s, char c1, char c2)
{
    if (!s)
	return;
    while (*s) {
	if (*s == c1)
	    *s = c2;
	s++;
    }
}

/*
    This function looks at two file names and returns the name of the
    infile with a path relative to outfile.

    Examples:

	/tmp/abc, /tmp/bcd -> abc
	xyz/a/bc, xyz/b/ac -> ../a/bc
	/tmp/abc, xyz/klm -> /tmp/abc
 */

QByteArray combinePath(const char *infile, const char *outfile)
{
    QFileInfo inFileInfo(QDir::current(), QFile::decodeName(infile));
    QFileInfo outFileInfo(QDir::current(), QFile::decodeName(outfile));
    int numCommonComponents = 0;

    QStringList inSplitted =
	QStringList::split('/', inFileInfo.dir().canonicalPath(), TRUE);
    QStringList outSplitted =
	QStringList::split('/', outFileInfo.dir().canonicalPath(), TRUE);

    while (!inSplitted.isEmpty() && !outSplitted.isEmpty() &&
	    inSplitted.first() == outSplitted.first()) {
	inSplitted.removeFirst();
	outSplitted.removeFirst();
	numCommonComponents++;
    }

    if (numCommonComponents < 2) {
	/*
	  The paths don't have the same drive, or they don't have the
	  same root directory. Use an absolute path.
	*/
	return QFile::encodeName(inFileInfo.absFilePath());
    } else {
	/*
	  The paths have something in common. Use a path relative to
	  the output file.
	*/
	while (!outSplitted.isEmpty()) {
	    outSplitted.removeFirst();
	    inSplitted.prepend("..");
	}
	inSplitted.append(inFileInfo.fileName());
	return QFile::encodeName(inSplitted.join("/"));
    }
}


#define getenv hack_getenv			// workaround for byacc
char *getenv()		     { return 0; }
char *getenv(const char *) { return 0; }

void init()					// initialize
{
    BEGIN OUTSIDE;
    if(g)
	delete g;
    g = new parser_reg;
    lineNo	 = 1;
    skipClass	 = FALSE;
    skipFunc	 = FALSE;
    tmpArgList	 = new ArgList;
    tmpFunc	 = new Function;
    tmpEnum	 = new Enum;

#ifdef MOC_MWERKS_PLUGIN
    buf_buffer = NULL;
    buf_index = 0;
    buf_size_total = 0;
#endif
}

void cleanup()
{
    delete g;
    g = NULL;

#ifdef MOC_MWERKS_PLUGIN
    if(buf_buffer && g_ctx)
	CWReleaseFileText(g_ctx, buf_buffer);
#endif
}

void initClass()				 // prepare for new class
{
    tmpAccess      = Private;
    subClassPerm       = Private;
    Q_OBJECTdetected   = FALSE;
    Q_PROPERTYdetected = FALSE;
    skipClass	       = FALSE;
    templateClass      = FALSE;
    g->slots.clear();
    g->signals.clear();
    g->propfuncs.clear();
    g->enums.clear();
    g->funcs.clear();
    g->props.clear();
    g->infos.clear();
    g->qtFlags.clear();
    g->qtEnums.clear();
    g->strings.clear();
    g->multipleSuperClasses.clear();
}

struct NamespaceInfo
{
    QByteArray name;
    int pLevelOnEntering; // Parenthesis level on entering the namespace
    QHash<QByteArray, bool> definedClasses; // Classes defined in the namespace
};

QList<NamespaceInfo *> namespaces;

void enterNameSpace(const char *name = 0)	 // prepare for new class
{
    static bool first = TRUE;
    if (first) {
	namespaces.setAutoDelete(TRUE);
	first = FALSE;
    }

    NamespaceInfo *tmp = new NamespaceInfo;
    if (name)
	tmp->name = name;
    tmp->pLevelOnEntering = namespacePLevel;
    namespaces.append(tmp);
}

void leaveNameSpace()				 // prepare for new class
{
    NamespaceInfo *tmp = namespaces.last();
    namespacePLevel = tmp->pLevelOnEntering;
    namespaces.removeLast();
}

QByteArray nameQualifier()
{
    QByteArray qualifier;
    for (int i = 0; i < namespaces.size(); ++i) {
	NamespaceInfo *n = namespaces.at(i);
	if (!n->name.isNull()) {  // If not unnamed namespace
	    qualifier += n->name;
	    qualifier += "::";
	}
    }
    return qualifier;
}

int openNameSpaceForMetaObject(FILE *out)
{
    int levels = 0;
    QByteArray indent;
    for (int i = 0; i < namespaces.size(); ++i) {
	NamespaceInfo *n = namespaces.at(i);
	if (!n->name.isNull()) {  // If not unnamed namespace
	    fprintf(out, "%snamespace %s {\n", (const char *)indent,
		     (const char *) n->name);
	    indent += "    ";
	    levels++;
	}
    }
    QByteArray nm = g->className;
    int pos;
    while((pos = nm.indexOf("::")) != -1) {
	QByteArray spaceName = nm.left(pos);
	nm = nm.right(nm.length() - pos - 2);
	if (!spaceName.isEmpty()) {
	    fprintf(out, "%snamespace %s {\n", (const char *)indent,
		     (const char *) spaceName);
	    indent += "    ";
	    levels++;
	}
    }
    return levels;
}

void closeNameSpaceForMetaObject(FILE *out, int levels)
{
    int i;
    for(i = 0 ; i < levels ; i++)
	    fprintf(out, "}");
    if (levels)
	fprintf(out, "\n");

}

void selectOutsideClassState()
{
    if (namespaces.count() == 0)
	BEGIN OUTSIDE;
    else
	BEGIN IN_NAMESPACE;
}

void registerClassInNamespace()
{
    if (namespaces.count() == 0)
	return;
    namespaces.last()->definedClasses.insert(g->className,true);
}


static QByteArray rmWS(const char *src)
{
    QByteArray result(qstrlen(src)+1);
    char *d = result.data();
    char *s = (char *)src;
    char last = 0;
    while(*s && isSpace(*s))			// skip leading space
	s++;
    while (*s) {
	while (*s && !isSpace(*s))
	    last = *d++ = *s++;
	while (*s && isSpace(*s))
	    s++;
	if (*s && isIdentChar(*s) && isIdentChar(last))
	    last = *d++ = ' ';
    }
    result.truncate((int)(d - result.data()));
    return result;
}


void initExpression()
{
    g->tmpExpression = "";
}

void addExpressionString(const char *s)
{
    g->tmpExpression += s;
}

void addExpressionChar(const char c)
{
    g->tmpExpression += c;
}

void yyerror(const char *msg)			// print yacc error message
{
    g->mocError = TRUE;
#ifndef MOC_MWERKS_PLUGIN
    fprintf(stderr, ErrorFormatString" Error: %s\n", g->fileName.data(), lineNo, msg);
#else
    char	msg2[200];
    sprintf(msg2, ErrorFormatString" Error: %s", g->fileName.data(), lineNo, msg);
    CWReportMessage(g_ctx, NULL, msg2, NULL, messagetypeError, 0);
#endif
    if (errorControl) {
	if (!g->outputFile.isEmpty() && yyin && fclose(yyin) == 0)
	    remove(g->outputFile);
	exit(-1);
    }
}

void moc_err(const char *s)
{
    yyerror(s);
}

void moc_err(const char *s1, const char *s2)
{
    static char tmp[1024];
    sprintf(tmp, s1, s2);
    yyerror(tmp);
}

void moc_warn(const char *msg)
{
    if (displayWarnings)
	fprintf(stderr, ErrorFormatString" Warning: %s\n", g->fileName.data(), lineNo, msg);
}

void moc_warn(char *s1, char *s2)
{
    static char tmp[1024];
    sprintf(tmp, s1, s2);
    if (displayWarnings)
	fprintf(stderr, ErrorFormatString" Warning: %s\n", g->fileName.data(), lineNo, tmp);
}

static bool suppress_func_warn = FALSE;
void func_warn(const char *msg)
{
    if (!suppress_func_warn)
	moc_warn(msg);
    skipFunc = TRUE;
}

void operatorError()
{
    if (!suppress_func_warn)
	moc_warn("Operator functions cannot be signals or slots.");
    skipFunc = TRUE;
}

#ifndef yywrap
int yywrap()					// more files?
{
    return 1;					// end of file
}
#endif

int strreg(const char *s)
{
    int idx = 0;
    if (!s)
	s = "";
    for (int i = 0; i < g->strings.size(); ++i) {
	QByteArray str = g->strings.at(i);
	if (strcmp(s, str) == 0)
	    return idx;
	idx += str.length() + 1;
    }
    g->strings.append(s);
    return idx;
}

char *stradd(const char *s1, const char *s2)	// adds two strings
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+1];
    qstrcpy(n, s1);
    strcat(n, s2);
    return n;
}

char *stradd(const char *s1, const char *s2, const char *s3)// adds 3 strings
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+1];
    qstrcpy(n, s1);
    strcat(n, s2);
    strcat(n, s3);
    return n;
}

char *stradd(const char *s1, const char *s2,
	      const char *s3, const char *s4)// adds 4 strings
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+qstrlen(s4)+1];
    qstrcpy(n, s1);
    strcat(n, s2);
    strcat(n, s3);
    strcat(n, s4);
    return n;
}


char *straddSpc(const char *s1, const char *s2)
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+2];
    qstrcpy(n, s1);
    strcat(n, " ");
    strcat(n, s2);
    return n;
}

char *straddSpc(const char *s1, const char *s2, const char *s3)
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+3];
    qstrcpy(n, s1);
    strcat(n, " ");
    strcat(n, s2);
    strcat(n, " ");
    strcat(n, s3);
    return n;
}

char *straddSpc(const char *s1, const char *s2,
	      const char *s3, const char *s4)
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+qstrlen(s4)+4];
    qstrcpy(n, s1);
    strcat(n, " ");
    strcat(n, s2);
    strcat(n, " ");
    strcat(n, s3);
    strcat(n, " ");
    strcat(n, s4);
    return n;
}

// Generate C++ code for building member function table


/*
  We call B::qt_invoke() rather than A::B::qt_invoke() to
  work around a bug in MSVC 6. The bug occurs if the
  super-class is in a namespace and the sub-class isn't.

  Exception: If the superclass has the same name as the subclass, we
  want non-MSVC users to have a working generated files.
*/
QByteArray purestSuperClassName()
{
    QByteArray sc = g->superClassName;
    QByteArray c = g->className;
    /*
      Make sure qualified template arguments (e.g., foo<bar::baz>)
      don't interfere.
    */
    int pos = sc.lastIndexOf("::", sc.indexOf('<'));
    if (pos != -1) {
	sc = sc.right(sc.length() - pos - 2);
	pos = c.lastIndexOf("::");
	if (pos != -1)
	    c = c.right(c.length() - pos - 2);
	if (sc == c)
	    sc = g->superClassName;
    }
    return sc;
}

QByteArray qualifiedClassName()
{
    return nameQualifier() + g->className;
}

void generateFuncs(FuncList *list, const char *functype)
{
    if (list->isEmpty())
	return;
    fprintf(out, "\n // %ss: signature, parameters, type, tag, flags\n", functype);

    for (int i = 0; i < list->size(); ++i) {
	Function *f = list->at(i);

	QByteArray sig = f->name + '(';
	QByteArray args;

	for (int j = 0; j < f->args->size(); ++j) {
	    Argument *a = f->args->at(j);
	    if (j) {
		sig += ",";
		args += ",";
	    }
	    sig += a->type;
	    args += a->name;
	}
	sig += ')';

	fprintf(out, "    %4d, %4d, %4d, %4d, 0x%.1x,\n", strreg(sig),
		 strreg(args), strreg(f->type), strreg(f->tag), f->access);
    }
}

void generateMetacall()
{
    bool isQObject =  g->className == "QObject" ;

    fprintf(out, "\nint %s::qt_metacall(QMetaObject::Call _c, int _id, void **_a)\n{\n",
	     (const char *)qualifiedClassName());

    if (!g->superClassName.isEmpty() && !isQObject)
	fprintf(out, "    _id = %s::qt_metacall(_c, _id, _a);\n",
		 (const char *) purestSuperClassName());

    fprintf(out, "    if (_id < 0)\n        return _id;\n");
    fprintf(out, "    ");

    bool needElse = false;
    if(!g->slots.isEmpty()) {
	needElse = true;
	fprintf(out, "if (_c == QMetaObject::InvokeSlot) {\n");
        fprintf(out, "        switch (_id) {\n");
	for (int slotindex = 0; slotindex < g->slots.size(); ++slotindex) {
	    Function *f = g->slots.at(slotindex);
	    fprintf(out, "        case %d: ", slotindex);
	    if (!f->type.isEmpty())
		fprintf(out, "{ %s _r = ", (const char *)rmRef(f->type));
	    fprintf(out, "%s(", (const char *)f->name);
	    int offset = 1;
	    for (int j = 0; j < f->args->size(); ++j) {
		Argument *a = f->args->at(j);
		if (j)
		    fprintf(out, ",");
		fprintf(out, "*(%s*)_a[%d]", (const char *)rmRef(a->type), offset++);
	    }
	    fprintf(out, ");");
	    if (!f->type.isEmpty()) {
		fprintf(out, "\n            if (_a[0]) *(%s*)_a[0] = _r; } ",
			 (const char *)rmRef(f->type));
	    }
	    fprintf(out, " break;\n");
	}
	fprintf(out,
		 "        }\n"
		 "        _id -= %d;\n"
		 "    }", g->slots.count());
    }
    if(!g->signals.isEmpty()) {
	if (needElse)
	    fprintf(out, " else ");
	needElse = true;
	fprintf(out, "if (_c == QMetaObject::EmitSignal) {\n");
        fprintf(out, "        switch (_id) {\n");
	for (int signalindex = 0; signalindex < g->signals.size(); ++signalindex) {
	    Function *f = g->signals.at(signalindex);
	    fprintf(out, "        case %d: ", signalindex);
	    if (!f->type.isEmpty())
		fprintf(out, "{ %s _r = ", (const char *)rmRef(f->type));
	    fprintf(out, "%s(", (const char *)f->name);
	    int offset = 1;
	    for (int j = 0; j < f->args->size(); ++j) {
		Argument *a = f->args->at(j);
		if (j)
		    fprintf(out, ",");
		fprintf(out, "*(%s*)_a[%d]", (const char *)rmRef(a->type), offset++);
	    }
	    fprintf(out, ");");
	    if (!f->type.isEmpty()) {
		fprintf(out, "\n            if (_a[0]) *(%s*)_a[0] = _r; } ",
			 (const char *)rmRef(f->type));
	    }
	    fprintf(out, " break;\n");
	}
	fprintf(out,
		 "        }\n"
		 "        _id -= %d;\n"
		 "    }", g->signals.count());
    }

    if (!g->props.isEmpty()) {
	bool needGet = false;
	bool needSet = false;
	bool needReset = false;
	bool needDesignable = false;
	bool needScriptable = false;
	bool needStored = false;
	bool needEditable = false;
	for (int i = 0; i < g->props.size(); ++i) {
	    Property *p = g->props.at(i);
	    needGet |= !p->read.isEmpty();
	    needSet |= !p->write.isEmpty();
	    needReset |= !p->reset.isEmpty();
	    needDesignable |= isPropFunction(p->designable);
	    needScriptable |= isPropFunction(p->scriptable);
	    needStored |= isPropFunction(p->stored);
	    needEditable |= isPropFunction(p->editable);
	}
	bool needAnything = needGet
			    | needSet
			    | needReset
			    | needDesignable
			    | needScriptable
			    | needStored
			    | needEditable;
	if (!needAnything)
	    goto skip_properties;
	fprintf(out, "\n#ifndef QT_NO_PROPERTIES\n     ");

	if (needGet){
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::ReadProperty) {\n");
	    fprintf(out, "        void *_v = _a[0];\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (p->read.isEmpty())
		    continue;
		if (p->gspec == Property::PointerSpec)
		    fprintf(out, "        case %d: _a[0] = (void*)%s(); break;\n",
			    propindex,
			    (const char *)p->read);
		else if (p->gspec == Property::ReferenceSpec)
		    fprintf(out, "        case %d: _a[0] = (void*)&%s(); break;\n",
			    propindex,
			    (const char *)p->read);
		else if (isVariantType(p->type))
		    fprintf(out, "        case %d: *(%s*)_v = %s(); break;\n",
			    propindex,
			    (const char *)p->type,
			    (const char *)p->read);
		else
		    fprintf(out, "        case %d: *(int*)_v = QFlag(%s()); break;\n",
			    propindex,
			    (const char *)p->read);
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}
	if (needSet) {
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::WriteProperty) {\n");
	    fprintf(out, "        void *_v = _a[0];\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (p->write.isEmpty())
		    continue;
		if (isFlagType(p->type)) {
		    fprintf(out, "        case %d: %s(QFlag(*(int*)_v)); break;\n",
			    propindex,
			    (const char *)p->write );
		} else {
		    fprintf(out, "        case %d: %s(*(%s*)_v); break;\n",
			    propindex,
			    (const char *)p->write,
			    (const char *)p->type);
		}
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}

	if (needReset) {
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::ResetProperty) {\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (p->reset.isEmpty())
		    continue;
		fprintf(out, "        case %d: %s(); break;\n",
			propindex, (const char *)p->reset);
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}

	if (needDesignable) {
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::QueryPropertyDesignable) {\n");
	    fprintf(out, "        bool *_b = (bool*)_a[0];\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (!isPropFunction(p->designable))
		    continue;
		fprintf(out, "        case %d: *_b = %s(); break;\n",
			 propindex, (const char *)p->designable);
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}
	if (needScriptable) {
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::QueryPropertyScriptable) {\n");
	    fprintf(out, "        bool *_b = (bool*)_a[0];\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (!isPropFunction(p->scriptable))
		    continue;
		fprintf(out, "        case %d: *_b = %s(); break;\n",
			 propindex, (const char *)p->scriptable);
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}
	if (needStored) {
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::QueryPropertyStored) {\n");
	    fprintf(out, "        bool *_b = (bool*)_a[0];\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (!isPropFunction(p->stored))
		    continue;
		fprintf(out, "        case %d: *_b = %s(); break;\n",
			 propindex, (const char *)p->stored);
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}
	if (needEditable) {
	    if (needElse)
		fprintf(out, " else ");
	    needElse = true;
	    fprintf(out, "if (_c == QMetaObject::QueryPropertyEditable) {\n");
	    fprintf(out, "        bool *_b = (bool*)_a[0];\n");
	    fprintf(out, "        switch (_id) {\n");
	    for (int propindex = 0; propindex < g->props.size(); ++propindex) {
		Property *p = g->props.at(propindex);
		if (!isPropFunction(p->editable))
		    continue;
		fprintf(out, "        case %d: *_b = %s(); break;\n",
			 propindex, (const char *)p->editable);
	    }
	    fprintf(out,
		    "        }\n"
		    "        _id -= %d;\n"
		    "    }", g->props.count());
	}


	fprintf(out, "\n#endif // QT_NO_PROPERTIES");
    }
 skip_properties:
    if (!g->slots.isEmpty() || !g->signals.isEmpty() || !g->props.isEmpty())
	fprintf(out, "\n    ");
    fprintf(out,"return _id;\n};\n");
}

void generateSignal(Function *f, int index)
{
    if (f->args && f->args->wasCloned())
	return;
    fprintf(out, "\n// SIGNAL %d\n%s %s::%s(",
	     index,
	     (const char*) f->rawType,
	     (const char*)qualifiedClassName(),
	     (const char*)f->name);

    if ((!f->args || f->args->isEmpty()) && f->type.isEmpty()) {
	fprintf(out, ")\n{\n"
		 "    QMetaObject::activate(this, &staticMetaObject, %d, 0);\n"
		 "};\n", index);
	return;
    }

    int offset = 1;
    for (int j = 0; j < f->args->size(); ++j) {
	Argument *a = f->args->at(j);
	if (j)
	    fprintf(out, ", ");
	fprintf(out, "%s _t%d%s", (const char *)a->leftType, offset++, (const char *)a->rightType);
    }
    fprintf(out, ")\n{\n");
    if (!f->type.isEmpty())
	fprintf(out, "    %s _t0;\n", (const char *)rmRef(f->type));
    fprintf(out, "    void *_a[] = { %s",
	     f->type.isEmpty() ? "0" : "(void*)&_t0");
    int i;
    for (i = 1; i < offset; ++i)
	fprintf(out, ", (void*)&_t%d", i);
    fprintf(out, " };\n");
    int n = 1;
    if (f->args)
	n += f->args->defaultArguments();
    for (i = 0; i < n; ++i)
	fprintf(out, "    QMetaObject::activate(this, &staticMetaObject, %d, _a);\n",
		index + i);
    if (!f->type.isEmpty())
	fprintf(out, "    return _t0;\n");
    fprintf(out, "}\n");
}


int enumIndex(const char* type)
{
    for (int i = 0; i < g->enums.size(); ++i) {
	Enum *e = g->enums.at(i);
	if (e->name == type)
	    return i;
    }
    return -1;
}

bool isEnumType(const char* type)
{
    return g->qtEnums.contains(type) || g->qtFlags.contains(type);
}

bool isFlagType(const char* type)
{
    return g->qtFlags.contains(type);
}

bool isPropertyType(const char* type)
{
    if (isVariantType(type))
	return TRUE;

    return isEnumType(type);
}

void generateEnums(int &index)
{
    if (g->enums.isEmpty())
	return;

    if (displayWarnings && !Q_OBJECTdetected)
	moc_err("The declaration of the class \"%s\" contains enums or sets"
		" but no Q_OBJECT macro.", g->className.data());

    fprintf(out, "\n // enums: name, flags, count, data\n");

    index += 4 * g->enums.count();
    for (int i = 0; i < g->enums.size(); ++i) {
	Enum *e = g->enums.at(i);
	fprintf(out, "    %4d, 0x%.1x, %4d, %4d,\n",
		 strreg(e->name),
		 e->set ? 1 : 0,
		 e->count(),
		 index);
	index += e->count() * 2;
    }

    fprintf(out, "\n // enum data: key, value\n");

    for (int i = 0; i < g->enums.size(); ++i) {
	Enum *e = g->enums.at(i);
	for (int j = 0; j < e->size(); ++j) {
	    const QByteArray &val = e->at(j);
	    fprintf(out, "    %4d, %s::%s,\n",
		     strreg(val),
		     (const char*) g->className,
		     val.constData());
	}
    }
}

void generateProps()
{
    if (g->props.isEmpty())
	return;

    if (displayWarnings && !Q_OBJECTdetected)
	moc_err("The declaration of the class \"%s\" contains properties"
		" but no Q_OBJECT macro.", g->className.data());

    //
    // specify get function, for compatibiliy we accept functions
    // returning pointers, or const char * for QByteArray.
    //
    for (int i = 0; i < g->props.size(); ++i) {
	Property *p = g->props.at(i);
	if (!p->read.isEmpty()) {
	    FuncList candidates = g->propfuncs.find(p->read);
	    for (int j = 0; j < candidates.size(); ++j) {
		Function *f = candidates.at(j);
		if (f->qualifier != "const") // get functions must be const
		    continue;
		if (f->args && !f->args->isEmpty()) // and must not take any arguments
		    continue;
		Property::Specification spec = Property::ValueSpec;
		QByteArray tmp = f->type;
		if (tmp.left(6) == "const ")
		    tmp = tmp.mid(6);
		if (p->type == "QByteArray" && (f->type == "const char*")) {
		    tmp = "QByteArray";
		} else if (tmp.right(1) == "*") {
		    tmp = tmp.left(tmp.length() - 1);
		    spec = Property::PointerSpec;
		} else if (f->rawType.right(1) == "&") {
		    spec = Property::ReferenceSpec;
		}
		if (p->type == tmp ||
		    ( isEnumType(p->type) && (tmp == "int" || tmp == "uint"))) {
		    p->gspec = spec;
		    break;
		}
	    }
	}
    }

    //
    // Create meta data
    //

    fprintf(out, "\n // properties: name, type, flags\n");
    for (int i = 0; i < g->props.size(); ++i) {
	Property *p = g->props.at(i);
	int flags = Invalid;
	if (!isVariantType(p->type)) {
	    flags |= EnumOrFlag;
	} else {
	    flags |= qvariant_nameToType(p->type) << 24;
	}
	if (!p->read.isEmpty())
	    flags |= Readable;
	if (!p->write.isEmpty()) {
	    flags |= Writable;
	    if (p->stdCppSet())
		flags |= StdCppSet;
	}
	if (!p->reset.isEmpty())
	    flags |= Resetable;

	if (p->override)
	    flags |= Override;

	if (p->designable.isEmpty())
	    flags |= ResolveDesignable;
	else if (p->designable != "false")
	    flags |= Designable;

	if (p->scriptable.isEmpty())
	    flags |= ResolveScriptable;
	else if (p->scriptable != "false")
	    flags |= Scriptable;

	if (p->stored.isEmpty())
	    flags |= ResolveStored;
	else if (p->stored != "false")
	    flags |= Stored;

	if (p->editable.isEmpty())
	    flags |= ResolveEditable;
	else if (p->editable != "false")
	    flags |= Editable;

	fprintf(out, "    %4d, %4d, 0x%.8x,\n",
		 strreg(p->name),
		 strreg(p->type),
		 flags);
    }
}



void generateClassInfos()
{
    if (g->infos.isEmpty())
	return;

    if (displayWarnings && !Q_OBJECTdetected)
	moc_err("The declaration of the class \"%s\" contains class infos"
		" but no Q_OBJECT macro.", g->className.data());

    fprintf(out, "\n // classinfo: key, value\n");

    for (int i = 0; i < g->infos.size(); ++i) {
	ClassInfo *c = g->infos.at(i);
	fprintf(out, "    %4d, %4d,\n",
		 strreg(c->name),
		 strreg(c->value));
    }
}


void generateClass()		      // generate C++ source code for a class
{
    const char *hdr1 = "/****************************************************************************\n"
		 "** %s meta object code from reading C++ file '%s'\n**\n";
    const char *hdr2 = "** Created: %s\n"
		 "**      by: The Qt MOC ($Id: $)\n**\n";
    const char *hdr3 = "** WARNING! All changes made in this file will be lost!\n";
    const char *hdr4 = "*****************************************************************************/\n\n";
    int   i;

    if (skipClass)				// don't generate for class
	return;

    bool isQt =  g->className == "Qt" ;
    if (isQt)
	Q_OBJECTdetected = true; // supress warnings

    if (!Q_OBJECTdetected) {
	if (g->signals.count() == 0 && g->slots.count() == 0 && g->props.count() == 0 && g->infos.count() == 0 && g->enums.count() == 0)
	    return;
	if (displayWarnings && (g->signals.count() + g->slots.count()) != 0)
	    moc_err("The declaration of the class \"%s\" contains signals "
		    "or slots\n\t but no Q_OBJECT macro.", g->className.data());
    } else if (!isQt) {
	if (g->superClassName.isEmpty())
	    moc_err("The declaration of the class \"%s\" contains the\n"
		    "\tQ_OBJECT macro but does not inherit from any class!\n"
		    "\tInherit from QObject or one of its descendants"
		    " or remove Q_OBJECT.", g->className.data());
    }
    if (templateClass) {			// don't generate for class
	moc_err("Sorry, Qt does not support templates that contain\n"
		"\tsignals, slots or Q_OBJECT.");
	return;
    }
    g->generatedCode = TRUE;
    g->gen_count++;

    if (g->gen_count == 1) {			// first class to be generated
	QDateTime dt = QDateTime::currentDateTime();
	QByteArray dstr = dt.toString().ascii();
	QByteArray fn = g->fileName;
	i = g->fileName.length()-1;
	while (i>0 && g->fileName[i-1] != '/' && g->fileName[i-1] != '\\')
	    --i;				// skip path
	if (i >= 0)
	    fn = g->fileName.mid(i);
	fprintf(out, hdr1, (const char*)qualifiedClassName(),(const char*)fn);
	fprintf(out, hdr2, (const char*)dstr);
	fprintf(out, hdr3);
	fprintf(out, hdr4);

	if (!g->noInclude) {
	    /*
	      The header file might be a Qt header file with
	      QT_NO_COMPAT macros around signals, slots or
	      properties. Without the #undef, we cannot compile the
	      Qt library with QT_NO_COMPAT defined.

	      Header files of libraries build around Qt can also use
	      QT_NO_COMPAT, so this #undef might be beneficial to
	      users of Qt, and not only to developers of Qt.
	    */
	    fprintf(out, "#undef QT_NO_COMPAT\n");

	    if (!g->pchFile.isEmpty())
	    	fprintf(out, "#include \"%s\" // PCH include\n", (const char*)g->pchFile);
	    if (!g->includePath.isEmpty() && g->includePath.right(1) != "/")
		g->includePath += "/";

	    for (int i = 0; i < g->includeFiles.size(); ++i) {
		QByteArray inc = g->includeFiles.at(i);
		if (inc[0] != '<' && inc[0] != '"') {
		    if (!g->includePath.isEmpty() && g->includePath != "./")
			inc.prepend(g->includePath);
		    inc = "\"" + inc + "\"";
		}
		fprintf(out, "#include %s\n", (const char *)inc);
	    }
	}
	fprintf(out, "#include <%sqmetaobject.h>\n", (const char*)g->qtPath);
	if (isQt)
	    fprintf(out, "#include <%sqobject.h>\n", (const char*)g->qtPath);
	fprintf(out, "#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != %d)\n", formatRevision);
	fprintf(out, "#error \"This file was generated using the moc from %s."
		 " It\"\n#error \"cannot be used with the include files from"
		 " this version of Qt.\"\n#error \"(The moc has changed too"
		 " much.)\"\n", QT_VERSION_STR);
	fprintf(out, "#endif\n\n");
    } else {
	fprintf(out, "\n\n");
    }

    bool isQObject =  g->className == "QObject" ;

//
// build the data array
//
    QByteArray qualifiedClassNameIdentifier = qualifiedClassName();
    for (i = 0; i < qualifiedClassNameIdentifier.length(); ++i)
	if (qualifiedClassNameIdentifier[i] == ':')
	    qualifiedClassNameIdentifier[i] = '_';

    int index = 12;
    fprintf(out, "static const uint qt_meta_data_%s[] = {\n", (const char*)qualifiedClassNameIdentifier);
    fprintf(out, "\n // content:\n");
    fprintf(out, "    %4d,       // revision\n", 1);
    fprintf(out, "    %4d,       // classname\n", strreg(g->className));
    fprintf(out, "    %4d, %4d, // classinfo\n", g->infos.count(), index);
    index += g->infos.count() * 2;
    fprintf(out, "    %4d, %4d, // signals\n", g->signals.count(), index);
    index += g->signals.count() * 5;
    fprintf(out, "    %4d, %4d, // slots\n", g->slots.count(), index);
    index += g->slots.count() * 5;
    fprintf(out, "    %4d, %4d, // properties\n", g->props.count(), index);
    index += g->props.count() * 3;
    fprintf(out, "    %4d, %4d, // enums/sets\n", g->enums.count(), index);

//
// Build classinfo array
//
    generateClassInfos();

//
// Build signals array
//
    generateFuncs(&g->signals, "signal");

//
// Build slots array
//
    generateFuncs(&g->slots, "slot");

//
// Build property array
//
    generateProps();

//
// Build enums array
//
    generateEnums(index);

//
// Terminate data array
//
    fprintf(out, "\n       0        // eod\n};\n\n");

//
// Build stringdata array
//
    fprintf(out, "static const char qt_meta_stringdata_%s[] = {\n", (const char*)qualifiedClassNameIdentifier);
    fprintf(out, "    \"");
    int col = 0;
    for (int i = 0; i < g->strings.size(); ++i) {
	QByteArray s = g->strings.at(i);
	if (col && col + strlen(s) >= 72) {
	    fprintf(out, "\"\n    \"");
	    col = 0;
	}
	fprintf(out, "%s\\0", s.constData());
	col += strlen(s)+2;
    }
    fprintf(out, "\"\n};\n\n");


//
// Finally create and initialize the static meta object
//

    if (isQt)
	fprintf(out, "const QMetaObject QObject::staticQtMetaObject = {\n");
    else
	fprintf(out, "const QMetaObject %s::staticMetaObject = {\n",
		(const char*)qualifiedClassName());

    if (isQObject)
	fprintf(out, "    { &staticQtMetaObject,\n      ");
    else if (!g->superClassName.isEmpty())
	fprintf(out, "    { &%s::staticMetaObject,\n      ", (const char*)g->superClassName);
    else
	fprintf(out, "    { 0, ");
    fprintf(out, "qt_meta_stringdata_%s,\n      qt_meta_data_%s }\n",
	     (const char*)qualifiedClassNameIdentifier, (const char*)qualifiedClassNameIdentifier);
    fprintf(out, "};\n");

    if (isQt)
	return;

//
// Generate virtual metaObject()  function
//
    fprintf(out,
	     "\nconst QMetaObject *%s::metaObject() const\n{\n"
	     "    return &staticMetaObject;\n"
	     "}\n", (const char*)qualifiedClassName());

//
// Generate smart cast function
//
    fprintf(out, "\nvoid *%s::qt_metacast(const char *clname) const\n{\n",
	     (const char*)qualifiedClassName());
    fprintf(out, "    if (!clname) return 0;\n");
    fprintf(out, "    if (!strcmp(clname, qt_meta_stringdata_%s))\n"
		  "\treturn (void*)this;\n",
	     (const char*)qualifiedClassNameIdentifier);
    for (int i = 0; i < g->multipleSuperClasses.size(); ++i) {
	const char *cname = g->multipleSuperClasses.at(i);
	fprintf(out, "    if (!strcmp(clname, \"%s\"))\n\treturn (%s*)this;\n", cname, cname);
    }
    if (!g->superClassName.isEmpty() && !isQObject)
	fprintf(out, "    return %s::qt_metacast(clname);\n",
		 (const char*)purestSuperClassName());
    else
	fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");

//
// Generate internal qt_metacall()  function
//
    generateMetacall();

//
// Generate internal signal functions
//
    for (int signalindex = 0; signalindex < g->signals.size(); ++signalindex)
	generateSignal(g->signals.at(signalindex), signalindex);
}

ArgList *addArg(Argument *a)			// add argument to list
{
    if (!a->type.isEmpty()) //filter out truely void arguments
	tmpArgList->append(a);
    return tmpArgList;
}

void addEnum()
{
    // Avoid duplicates
    for (int i = 0; i < g->enums.size(); ++i) {
	Enum *e = g->enums.at(i);
	if (e->name == tmpEnum->name)
	{
	    if (displayWarnings)
		moc_err("Enum %s defined twice.", (const char*)tmpEnum->name);
	}
    }

    // Only look at types mentioned  in Q_ENUMS and Q_FLAGS
    if (g->qtEnums.contains(tmpEnum->name) || g->qtFlags.contains(tmpEnum->name))
    {
	g->enums.append(tmpEnum);
	if (g->qtFlags.contains(tmpEnum->name))
	    tmpEnum->set = TRUE;
	else
	    tmpEnum->set = FALSE;
    }
    else
	delete tmpEnum;
    tmpEnum = new Enum;
}

void addMember(Member m)
{
    if (skipFunc) {
	tmpFunc->args = tmpArgList; // just to be sure
	delete tmpFunc;
	tmpArgList  = new ArgList;   // ugly but works
	tmpFunc	    = new Function;
	skipFunc    = FALSE;
	return;
    }

    tmpFunc->rawType = rmWS(tmpFunc->type);
    tmpFunc->type = normalizeType(tmpFunc->type);
    tmpFunc->access = tmpAccess;
    tmpFunc->args = tmpArgList;
    tmpFunc->lineNo = lineNo;

    for (;;) {
	g->funcs.append(tmpFunc);
	switch (m) {
	case SignalMember:
	    // Important for the cloning: the inSort is done by name
	    // only, not by signatures.
	    g->signals.insertSorted(tmpFunc);
	    break;
	case SlotMember:
	    g->slots.insertSorted(tmpFunc);
	    // fall through ...
	case PropertyCandidateMember:
	    if (!tmpFunc->name.isEmpty() && tmpFunc->access == Public)
		g->propfuncs.append(tmpFunc);
	}
	ArgList *args = tmpFunc->args;
	if (!args || !(args = args->magicClone()))
	    break;
	tmpFunc = new Function(*tmpFunc);
	tmpFunc->args = args;
    }

    skipFunc = FALSE;
    tmpFunc = new Function;
    tmpArgList = new ArgList;
}

void checkPropertyName(const char* ident)
{
    if (ident[0] == '_') {
	moc_err("Invalid property name '%s'.", ident);
	return;
    }
}
