/*
  LocalSQL

  Copyright (C) 2001 Trolltech AS

  Contact:
	 Dave Berton (db@trolltech.com)
	 Jasmin Blanchette (jasmin@trolltech.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <qpoint.h>
#include <qstring.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "op.h"
#include "sqlinterpreter.h"

/*
  Here are the tokens recognized by the tokenizer.
*/
enum { Tok_Eoi, Tok_Equal, Tok_NotEq, Tok_LessThan, Tok_GreaterThan,
       Tok_LessEq, Tok_GreaterEq, Tok_Plus, Tok_Minus, Tok_Aster,
       Tok_Div, Tok_LeftParen, Tok_RightParen, Tok_Comma, Tok_Dot,
       Tok_Colon, Tok_Semicolon, Tok_Name, Tok_IntNum, Tok_ApproxNum,
       Tok_String,

       Tok_abs, Tok_all, Tok_and, Tok_any, Tok_as, Tok_asc, Tok_avg,
       Tok_between, Tok_by, Tok_ceil, Tok_character, Tok_check,
       Tok_close, Tok_commit, Tok_count, Tok_create, Tok_current,
       Tok_date, Tok_decimal, Tok_declare, Tok_default, Tok_delete,
       Tok_desc, Tok_distinct, Tok_double, Tok_drop, Tok_escape,
       Tok_float, Tok_floor, Tok_for, Tok_foreign, Tok_from,
       Tok_group, Tok_having, Tok_in, Tok_index, Tok_insert,
       Tok_integer, Tok_into, Tok_is, Tok_key, Tok_length, Tok_like,
       Tok_lower, Tok_max, Tok_min, Tok_mod, Tok_not, Tok_null,
       Tok_numeric, Tok_of, Tok_on, Tok_or, Tok_order, Tok_power,
       Tok_precision, Tok_primary, Tok_real, Tok_references,
       Tok_replace, Tok_rollback, Tok_select, Tok_set, Tok_sign,
       Tok_smallint, Tok_some, Tok_soundex, Tok_substring, Tok_sum,
       Tok_table, Tok_to, Tok_translate, Tok_union, Tok_unique,
       Tok_update, Tok_upper, Tok_user, Tok_values, Tok_varchar,
       Tok_view, Tok_where, Tok_with, Tok_work };

/*
  The syntax tree of an expression is either a simple node or a
  complex node. Simple nodes are QVariant objects of type other than
  List. Complex nodes are QValueList<QVariant>. The first item in the
  list is the kind of the complex node (one of the Node_Xxx constants
  below). Here is the format and meaning of the nodes:

      (Node_UnresolvedField t s) - field s of table called t
      (Node_UnresolvedStar t) - all fields of table called t
      (Node_ResolvedField n s) - field s of table n
      (Node_ResolvedStar n1 n2 ... nN) - all fields of tables n1, n2, ..., nN
      (Node_ResultColumnNo n) - field number n of result set

      (Node_Abs x) - abs(x)
      (Node_Add x y) - x + y
      (Node_Ceil x) - ceil(x)
      (Node_Divide x y) - x / y
      (Node_Floor x) - floor(x)
      (Node_Mod x y) - mod(x, y)
      (Node_Multiply x y) - x * y
      (Node_Power x y) - power(x, y)
      (Node_Sign x) - sign(x)
      (Node_Subtract x y) - x - y

      (Node_Length s) - length(s)
      (Node_Lower s) - lower(s)
      (Node_Replace s t u) - replace(s, t, u)
      (Node_Soundex s) - soundex(s)
      (Node_Substring s, k, n) - substring(s, k, n)
      (Node_Translate s t u) - translate(s, t, u)
      (Node_Upper s) - upper(s)

      (Node_And x y) - x and y
      (Node_Eq x y) - x = y
      (Node_GreaterEq x y) - x >= y
      (Node_LessThan x y) - x < y
      (Node_In x y1 y2 ... yN) - x in (y1, y2, ..., yN)
      (Node_Like s t) - s like t
      (Node_Not x) - not x
      (Node_NotEq x y) - x <> y
      (Node_Or x y) - x or y

      (Node_Avg X) - avg(X)
      (Node_Count X) - count(X)
      (Node_Max X) - max(X)
      (Node_Min X) - min(X)
      (Node_Sum X) - sum(X)

  In some cases, an expression is really a scalar expression, and at
  the end of its evaluation, its value is on the top of the stack. In
  other cases, an "expression" contains conditionals, and the
  evaluation of such an "expression" leaves nothing on the stack but
  rather jumps to a certain instruction if true and to another
  instruction if false.
*/
enum { Node_Abs, Node_Add, Node_And, Node_Avg, Node_Ceil, Node_Count,
       Node_Divide, Node_Eq, Node_Floor, Node_GreaterEq, Node_In,
       Node_Length, Node_LessThan, Node_Like, Node_Lower, Node_Max,
       Node_Min, Node_Mod, Node_Multiply, Node_Not, Node_NotEq,
       Node_Or, Node_Power, Node_Replace, Node_ResolvedField,
       Node_ResolvedStar, Node_ResultColumnNo, Node_Sign,
       Node_Soundex, Node_Substring, Node_Subtract, Node_Sum,
       Node_Translate, Node_UnresolvedField, Node_UnresolvedStar,
       Node_Upper };

/*
  Yes, that's the representation of 'null' in the virtual machine.
*/
static QPoint NullRep( 0, 0 );

static int ResultId = 0;
static int AuxResultId = 1;

/*
  Returns (Node_ResolvedField tableId fieldName).
*/
static QVariant resolvedField( int tableId, const QString& fieldName )
{
    QValueList<QVariant> f;
    f.append( (int) Node_ResolvedField );
    f.append( tableId );
    f.append( fieldName );
    return f;
}

static bool isIdent( int ch )
{
    return isalnum( ch ) || ch == '_';
}

/*
  Returns a canonical column name. For example, 'SUM(id)' and ' sum (id )' are
  both 'sum(id)' canonically, whereas 'sum(ID)' stays 'sum(ID)'. Remember:
  Keywords are case-insensitive in SQL, but other identifiers are
  case-sensitive.
*/
static QString fixedColumnName( const QString& name )
{
    QString out;
    bool keepSpace = FALSE;
    bool pendingSpace = FALSE;

    for ( int i = 0; i < (int) name.length(); i++ ) {
	QChar ch = name[i];
	if ( ch.isSpace() ) {
	    if ( keepSpace ) {
		pendingSpace = TRUE;
		keepSpace = FALSE;
	    }
	} else {
	    keepSpace = isIdent( ch.unicode() );
	    if ( pendingSpace && keepSpace ) {
		out += QChar( ' ' );
		pendingSpace = FALSE;
	    }

	    // normalize function names to lowercase
	    if ( ch == QChar('(') ) {
		int j = (int) out.length() - 1;
		while ( j >= 0 && isIdent(out[j].unicode()) ) {
		    out[j] = out[j].lower();
		    j--;
		}
	    }
	    out += ch;
	}
    }
    return out;
}

Parser::Parser()
{
}

Parser::~Parser()
{
}

inline void Parser::readChar()
{
    if ( yyCh == EOF )
	return;

    if ( yyLexLen < (int) sizeof(yyLex) + 1 ) {
	yyLex[yyLexLen++] = (char) yyCh;
	yyLex[yyLexLen] = '\0';
    }

    if ( yyCurPos == (int) yyIn.length() ) {
	yyCh = EOF;
	yyCurPos++;
    } else {
	yyCh = yyIn[yyCurPos++].unicode();
	if ( yyCh == '\n' ) {
	    yyCurLineNo++;
	    yyCurColumnNo = 0;
	} else if ( yyCh == '\t' ) {
	    yyCurColumnNo = ( yyCurColumnNo + 8 ) & ~0x7;
	} else {
	    yyCurColumnNo++;
	}
    }
}

bool Parser::parse( const QString& commands, LocalSQLEnvironment *env )
{
    startTokenizer( commands );
    yyTok = getToken();
    yyEnv = env;
    yyProg = env->program();
    yyNextLabel = -1;
    yyOK = TRUE;
    for ( int i = 0; i < 4; i++ )
	yyNeedIndex[i >> 1][i & 0x1].clear();
    yyOpenedTableMap.clear();
    yyAliasTableMap.clear();
    yyActiveTableMap.clear();
    yyActiveTableIds.clear();
    yyLookedUpColumnMap.clear();
    yyNumAggregateOccs = 0;

    matchSql();
    return yyOK;
}

void Parser::startTokenizer( const QString& in )
{
    static QRegExp comment( QString("--[^\n]*") );
    yyIn = in;
    yyIn.replace( comment, QString::null );
    yyPos = 0;
    yyCurPos = 0;
    yyLexLen = 0;
    yyLineNo = 1;
    yyCurLineNo = 1;
    yyColumnNo = 0;
    yyCurColumnNo = 0;
    yyCh = '\0';
    yyStr = QString::null;
    yyNum = 0.0;
    readChar();
}

void Parser::warning( const char *format, ... )
{
    va_list ap;

    va_start( ap, format );
    sprintf( yyPrintfBuf, "Warning: line %d:%d: ", yyLineNo, yyColumnNo );
    vsprintf( yyPrintfBuf + strlen(yyPrintfBuf), format, ap );
    yyEnv->output() << yyPrintfBuf << endl;;
    va_end( ap );
}

void Parser::error( const char *format, ... )
{
    if ( !yyOK )
	return;

    va_list ap;

    va_start( ap, format );
    sprintf( yyPrintfBuf, "line %d:%d: ", yyLineNo, yyColumnNo );
    vsprintf( yyPrintfBuf + strlen(yyPrintfBuf), format, ap );
    yyEnv->output() << yyPrintfBuf << endl;
    va_end( ap );

    yyOK = FALSE;
    yyPos = yyIn.length(); // skip the rest of the file
}

void Parser::readTrailingGarbage()
{
    if ( isalpha(yyCh) || yyCh == '_' ) {
	error( "Trailing garbage after '%s'", yyLex );
	do {
	    readChar();
	} while ( isalpha(yyCh) || yyCh == '_' );
    }
}

int Parser::readExponent()
{
    if ( yyCh == 'e' || yyCh == 'E' ) {
	readChar();
	if ( yyCh == '+' || yyCh == '-' )
	    readChar();
	if ( !isdigit(yyCh) )
	    error( "Expected digit after '%s'", yyLex );
	while ( isdigit(yyCh) )
	    readChar();
	readTrailingGarbage();
	yyNum = QString( yyLex ).toDouble();
	return Tok_ApproxNum;
    } else {
	readTrailingGarbage();
	yyNum = QString( yyLex ).toDouble();
	return Tok_IntNum;
    }
}

/*
  We use a neat trick in the tokenizer to recognize keywords. The
  reader is challenged to make sense out of the trick by herself.
*/
#define HASH( first, omitted, last ) \
    ( ((((first) << 5) | (omitted)) << 7) | (last) )
#define CHECK( target ) \
    if ( qstricmp(target, yyLex) != 0 ) \
	break;

int Parser::getToken()
{
    while ( TRUE ) {
	yyPos = yyCurPos - 1; // yyCurPos is one character ahead
	yyLex[0] = '\0';
	yyLexLen = 0;
	yyLineNo = yyCurLineNo;
	yyColumnNo = yyCurColumnNo;
	yyStr = QString::null;
	yyNum = 0.0;

	if ( yyCh == EOF ) {
	    break;
	} else if ( isspace(yyCh) ) {
	    do {
		readChar();
	    } while ( isspace(yyCh) );
	} else if ( isalpha(yyCh) || yyCh == '_' ) {
	    do {
		readChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );

	    /*
	      The author of the following code is paid per line of code.
	    */
	    int h = HASH( tolower(yyLex[0]), yyLexLen - 2,
			  tolower(yyLex[yyLexLen - 1]) );
	    switch ( h ) {
	    case HASH( 'a', 1, 'c' ):
		CHECK( "asc" );
		return Tok_asc;
	    case HASH( 'a', 1, 'd' ):
		CHECK( "and" );
		return Tok_and;
	    case HASH( 'a', 1, 'g' ):
		CHECK( "avg" );
		return Tok_avg;
	    case HASH( 'a', 1, 'l' ):
		CHECK( "all" );
		return Tok_all;
	    case HASH( 'a', 1, 's' ):
		CHECK( "abs" );
		return Tok_abs;
	    case HASH( 'a', 1, 'y' ):
		CHECK( "any" );
		return Tok_any;
	    case HASH( 'b', 0, 'y' ):
		CHECK( "by" );
		return Tok_by;
	    case HASH( 'b', 5, 'n' ):
		CHECK( "between" );
		return Tok_between;
	    case HASH( 'c', 2, 'l' ):
		CHECK( "ceil" );
		return Tok_ceil;
	    case HASH( 'c', 2, 'r' ):
		CHECK( "char" );
		return Tok_character;
	    case HASH( 'c', 3, 'e' ):
		CHECK( "close" );
		return Tok_close;
	    case HASH( 'c', 3, 'k' ):
		CHECK( "check" );
		return Tok_check;
	    case HASH( 'c', 3, 't' ):
		CHECK( "count" );
		return Tok_count;
	    case HASH( 'c', 4, 'e' ):
		CHECK( "create" );
		return Tok_create;
	    case HASH( 'c', 4, 't' ):
		CHECK( "commit" );
		return Tok_commit;
	    case HASH( 'c', 5, 't' ):
		CHECK( "current" );
		return Tok_current;
	    case HASH( 'c', 7, 'r' ):
		CHECK( "character" );
		return Tok_character;
	    case HASH( 'd', 1, 'c' ):
		CHECK( "dec" );
		return Tok_decimal;
	    case HASH( 'd', 2, 'c' ):
		CHECK( "desc" );
		return Tok_desc;
	    case HASH( 'd', 2, 'e' ):
		CHECK( "date" );
		return Tok_date;
	    case HASH( 'd', 2, 'p' ):
		CHECK( "drop" );
		return Tok_drop;
	    case HASH( 'd', 4, 'e' ):
		if ( yyLex[1] == 'e' ) {
		    CHECK( "delete" );
		    return Tok_delete;
		} else {
		    CHECK( "double" );
		    return Tok_double;
		}
	    case HASH( 'd', 5, 'e' ):
		CHECK( "declare" );
		return Tok_declare;
	    case HASH( 'd', 5, 'l' ):
		CHECK( "decimal" );
		return Tok_decimal;
	    case HASH( 'd', 5, 't' ):
		CHECK( "default" );
		return Tok_default;
	    case HASH( 'd', 6, 't' ):
		CHECK( "distinct" );
		return Tok_distinct;
	    case HASH( 'e', 4, 'e' ):
		CHECK( "escape" );
		return Tok_escape;
	    case HASH( 'f', 3, 't' ):
		CHECK( "float" );
		return Tok_float;
	    case HASH( 'f', 1, 'r' ):
		CHECK( "for" );
		return Tok_for;
	    case HASH( 'f', 3, 'r' ):
		CHECK( "floor" );
		return Tok_floor;
	    case HASH( 'f', 5, 'n' ):
		CHECK( "foreign" );
		return Tok_foreign;
	    case HASH( 'f', 2, 'm' ):
		CHECK( "from" );
		return Tok_from;
	    case HASH( 'g', 3, 'p' ):
		CHECK( "group" );
		return Tok_group;
	    case HASH( 'h', 4, 'g' ):
		CHECK( "having" );
		return Tok_having;
	    case HASH( 'i', 0, 'n' ):
		CHECK( "in" );
		return Tok_in;
	    case HASH( 'i', 0, 's' ):
		CHECK( "is" );
		return Tok_is;
	    case HASH( 'i', 1, 't' ):
		CHECK( "int" );
		return Tok_integer;
	    case HASH( 'i', 2, 'o' ):
		CHECK( "into" );
		return Tok_into;
	    case HASH( 'i', 3, 'x' ):
		CHECK( "index" );
		return Tok_index;
	    case HASH( 'i', 4, 't' ):
		CHECK( "insert" );
		return Tok_insert;
	    case HASH( 'i', 5, 'r' ):
		CHECK( "integer" );
		return Tok_integer;
	    case HASH( 'k', 1, 'y' ):
		CHECK( "key" );
		return Tok_key;
	    case HASH( 'l', 2, 'e' ):
		CHECK( "like" );
		return Tok_like;
	    case HASH( 'l', 3, 'r' ):
		CHECK( "lower" );
		return Tok_lower;
	    case HASH( 'l', 4, 'h' ):
		CHECK( "length" );
		return Tok_length;
	    case HASH( 'm', 1, 'd' ):
		CHECK( "mod" );
		return Tok_mod;
	    case HASH( 'm', 1, 'n' ):
		CHECK( "min" );
		return Tok_min;
	    case HASH( 'm', 1, 'x' ):
		CHECK( "max" );
		return Tok_max;
	    case HASH( 'n', 1, 't' ):
		CHECK( "not" );
		return Tok_not;
	    case HASH( 'n', 2, 'l' ):
		CHECK( "null" );
		return Tok_null;
	    case HASH( 'n', 5, 'c' ):
		CHECK( "numeric" );
		return Tok_numeric;
	    case HASH( 'o', 0, 'f' ):
		CHECK( "of" );
		return Tok_of;
	    case HASH( 'o', 0, 'n' ):
		CHECK( "on" );
		return Tok_on;
	    case HASH( 'o', 0, 'r' ):
		CHECK( "or" );
		return Tok_or;
	    case HASH( 'o', 3, 'r' ):
		CHECK( "order" );
		return Tok_order;
	    case HASH( 'p', 3, 'r' ):
		CHECK( "power" );
		return Tok_power;
	    case HASH( 'p', 5, 'y' ):
		CHECK( "primary" );
		return Tok_primary;
	    case HASH( 'p', 7, 'n' ):
		CHECK( "precision" );
		return Tok_precision;
	    case HASH( 'r', 2, 'l' ):
		CHECK( "real" );
		return Tok_real;
	    case HASH( 'r', 5, 'e' ):
		CHECK( "replace" );
		return Tok_replace;
	    case HASH( 'r', 6, 'k' ):
		CHECK( "rollback" );
		return Tok_rollback;
	    case HASH( 'r', 8, 's' ):
		CHECK( "references" );
		return Tok_references;
	    case HASH( 's', 1, 'm' ):
		CHECK( "sum" );
		return Tok_sum;
	    case HASH( 's', 1, 't' ):
		CHECK( "set" );
		return Tok_set;
	    case HASH( 's', 2, 'e' ):
		CHECK( "some" );
		return Tok_some;
	    case HASH( 's', 2, 'n' ):
		CHECK( "sign" );
		return Tok_sign;
	    case HASH( 's', 4, 't' ):
		CHECK( "select" );
		return Tok_select;
	    case HASH( 's', 5, 'x' ):
		CHECK( "soundex" );
		return Tok_soundex;
	    case HASH( 's', 6, 't' ):
		CHECK( "smallint" );
		return Tok_smallint;
	    case HASH( 's', 7, 'g' ):
		CHECK( "substring" );
		return Tok_substring;
	    case HASH( 't', 0, 'o' ):
		CHECK( "to" );
		return Tok_to;
	    case HASH( 't', 3, 'e' ):
		CHECK( "table" );
		return Tok_table;
	    case HASH( 't', 7, 'e' ):
		CHECK( "translate" );
		return Tok_translate;
	    case HASH( 'u', 2, 'r' ):
		CHECK( "user" );
		return Tok_user;
	    case HASH( 'u', 3, 'n' ):
		CHECK( "union" );
		return Tok_union;
	    case HASH( 'u', 3, 'r' ):
		CHECK( "upper" );
		return Tok_upper;
	    case HASH( 'u', 4, 'e' ):
		if ( yyLex[1] == 'n' ) {
		    CHECK( "unique" );
		    return Tok_unique;
		} else {
		    CHECK( "update" );
		    return Tok_update;
		}
	    case HASH( 'v', 2, 'w' ):
		CHECK( "view" );
		return Tok_view;
	    case HASH( 'v', 4, 's' ):
		CHECK( "values" );
		return Tok_values;
	    case HASH( 'v', 5, 'r' ):
		CHECK( "varchar" );
		return Tok_varchar;
	    case HASH( 'w', 2, 'h' ):
		CHECK( "with" );
		return Tok_with;
	    case HASH( 'w', 2, 'k' ):
		CHECK( "work" );
		return Tok_work;
	    case HASH( 'w', 3, 'e' ):
		CHECK( "where" );
		return Tok_where;
	    }
	    return Tok_Name;
	} else if ( isdigit(yyCh) || yyCh == '.' ) {
	    if ( yyCh == '.' ) {
		readChar();
		if ( isdigit(yyCh) || yyCh == 'e' || yyCh == 'E' ) {
		    while ( isdigit(yyCh) )
			readChar();
		    return readExponent();
		} else {
		    return Tok_Dot;
		}
	    } else {
		while ( isdigit(yyCh) )
		    readChar();
		if ( yyCh == '.' ) {
		    readChar();
		    while ( isdigit(yyCh) )
			readChar();
		}
		return readExponent();
	    }
	} else {
	    int prevCh = yyCh;
	    readChar();

	    switch ( prevCh ) {
	    case '\'':
		while ( TRUE ) {
		    while ( yyCh != '\'' && yyCh != '\n' && yyCh != EOF ) {
			yyStr += QChar( yyCh );
			readChar();
		    }
		    readChar();
		    if ( yyCh != '\'' )
			break;
		    yyStr += QChar( '\'' );
		    readChar();
		}
		return Tok_String;
	    case '(':
		return Tok_LeftParen;
	    case ')':
		return Tok_RightParen;
	    case '*':
		return Tok_Aster;
	    case '+':
		return Tok_Plus;
	    case ',':
		return Tok_Comma;
	    case '-':
		return Tok_Minus;
	    case '/':
		return Tok_Div;
	    case ':':
		return Tok_Colon;
	    case ';':
		return Tok_Semicolon;
	    case '<':
		if ( yyCh == '=' ) {
		    readChar();
		    return Tok_LessEq;
		} else if ( yyCh == '>' ) {
		    readChar();
		    return Tok_NotEq;
		} else {
		    return Tok_LessThan;
		}
	    case '=':
		return Tok_Equal;
	    case '>':
		if ( yyCh == '=' ) {
		    readChar();
		    return Tok_GreaterEq;
		} else {
		    return Tok_GreaterThan;
		}
	    default:
		error( "Hostile character 0%.2o", (unsigned char) yyCh );
		readChar();
	    }
	}
    }
    strcpy( yyLex, "{end of input}" );
    return Tok_Eoi;
}

/*
  Returns the table id corresponding to tableName among the activated tables,
  considering the aliases as well.
*/
int Parser::resolveTableId( const QString& tableName )
{
    QMap<QString, int>::ConstIterator id = yyAliasTableMap.find( tableName );
    if ( id == yyAliasTableMap.end() )
	id = yyActiveTableMap.find( tableName );
    if ( id == yyActiveTableMap.end() ) {
	error( "Cannot use table '%s' in this statement", tableName.latin1() );
	return -1;
    }
    return *id;
}

/*
  Replaces all Node_UnresolvedField nodes by Node_ResolvedField.
  Sometimes, virtual machine instructions are generated to uniquely
  resolve the field. We have to be careful not to call this function
  in the middle of the generation of instructions for a loop, because
  then the resolving instructions would needlessly be executed over
  and over.
*/
void Parser::resolveFieldNames( QVariant *expr )
{
    if ( expr->type() == QVariant::List ) {
	QValueList<QVariant>::Iterator v = expr->asList().begin();
	int node = (*v).toInt();

	if ( node == Node_UnresolvedField ) {
	    /*
	      Replace the (Node_UnresolvedField t s) by
	      (Node_ResolvedField n s).
	    */
	    *v = (int) Node_ResolvedField;

	    QValueList<QVariant>::Iterator updateMe = ++v;
	    QString tableName = (*updateMe).toString();
	    QString columnName = (*++v).toString();
	    QMap<QString, int>::ConstIterator id;

	    if ( tableName.isEmpty() ) {
		// no table was specified
		int numTables = (int) yyActiveTableIds.count();
		if ( numTables == 1 ) {
		    // only one table, no ambiguity
		    *updateMe = yyActiveTableIds.first();
		} else {
		    id = yyLookedUpColumnMap.find( columnName );
		    if ( id == yyLookedUpColumnMap.end() ) {
			/*
			  We haven't resolved columnName yet. Let's
			  generate the instructions that do that.
			  Afterwards, we will use aliasId as an alias
			  for the real table id. We'll aslo store the
			  value into yyLookedUpColumnMap, so that we
			  don't resolve the same columnName twice in
			  the same statement.
			*/
			int aliasId = -( yyLookedUpColumnMap.count() + 1 );
			id = yyLookedUpColumnMap.insert( columnName, aliasId );

			yyProg->append( new PushSeparator );
			QValueList<int>::ConstIterator t =
				yyActiveTableIds.begin();
			while ( t != yyActiveTableIds.end() ) {
			    yyProg->append( new Push(*t) );
			    ++t;
			}
			yyProg->append( new MakeList );
			yyProg->append( new LookupUnique(columnName, aliasId) );
		    }
		    *updateMe = *id;
		}
	    } else {
		// a table was specified
		*updateMe = resolveTableId( tableName );
	    }
	} else if ( node == Node_UnresolvedStar ) {
	    /*
	      Replace (Node_UnresolvedStar "") by
	      (Node_ResolvedStar n1 n2 ... nN) where n1, n2, ..., nN
	      are the ids of the active tables, and replace
	      (Node_UnresolvedStar "table-name") by
	      (Node_ResolvedStar table-id).
	    */
	    *v = (int) Node_ResolvedStar;

	    QString tableName = (*++v).toString();
	    if ( tableName.isEmpty() ) {
		expr->asList().remove( v );

		QValueList<int>::ConstIterator id = yyActiveTableIds.begin();
		while ( id != yyActiveTableIds.end() ) {
		    expr->asList().append( *id );
		    ++id;
		}
	    } else {
		*v = resolveTableId( tableName );
	    }
	} else {
	    // recursively resolve the sub-trees
	    ++v;
	    while ( v != expr->asList().end() ) {
		resolveFieldNames( &*v );
		++v;
	    }
	}
    }
}

void Parser::resolveFieldNames( QValueList<QVariant> *exprs )
{
    QValueList<QVariant>::Iterator e = exprs->begin();
    while ( e != exprs->end() ) {
	resolveFieldNames( &*e );
	++e;
    }
}

/*
  Replace all Node_ResolvedField by Node_ResultColumnNo, using
  resultColumnNos to determine which column is in which position.
*/
void Parser::resolveResultColumnNos( QVariant *expr,
	const QMap<QString, QMap<int, int> >& resultColumnNos )
{
    if ( expr->type() == QVariant::List ) {
	QValueList<QVariant>::Iterator v = expr->asList().begin();
	int node = (*v).toInt();

	if ( node == Node_ResolvedField ) {
	    int no = columnNo( *expr, resultColumnNos );
	    expr->asList().clear();
	    expr->asList().append( (int) Node_ResultColumnNo );
	    expr->asList().append( no );
	} else {
	    ++v;
	    while ( v != expr->asList().end() ) {
		resolveResultColumnNos( &*v, resultColumnNos );
		++v;
	    }
	}
    }
}

void Parser::resolveResultColumnNos( QValueList<QVariant> *exprs,
	const QMap<QString, QMap<int, int> >& resultColumnNos )
{
    QValueList<QVariant>::Iterator e = exprs->begin();
    while ( e != exprs->end() ) {
	resolveResultColumnNos( &*e, resultColumnNos );
	++e;
    }
}

/*
  Finds all the Node_ResolvedField nodes in expr and fills in
  usedFields.
*/
void Parser::computeUsedFields( const QVariant& expr,
				QMap<int, QStringList> *usedFields )
{
    if ( expr.type() == QVariant::List ) {
	QValueList<QVariant>::ConstIterator v = expr.listBegin();
	int node = (*v).toInt();

	if ( node == Node_ResolvedField ) {
	    int id = (*++v).toInt();
	    QString field = (*++v).toString();
	    if ( (*usedFields)[id].contains(field) == 0 )
		(*usedFields)[id].append( field );
	} else {
	    /*
	      Don't do ++v here, as the top-level list might be a
	      forest, not a tree.
	    */
	    while ( v != expr.listEnd() ) {
		computeUsedFields( *v, usedFields );
		++v;
	    }
	}
    }
}

/*
  Tries to find out the return type of an expression. If the returned
  value is an int, then it represents a QVariant::Type. If the
  returned value is a Node_ResolvedField, then the type of the
  expression is that of the field.
*/
QVariant Parser::exprType( const QVariant& expr )
{
    if ( expr.type() == QVariant::List ) {
	QValueList<QVariant>::ConstIterator v = expr.listBegin();
	int node = (*v).toInt();

	switch ( node ) {
	/*
	  Arithmetic functions.
	*/
	case Node_Abs:
	case Node_Add:
	case Node_Ceil:
	case Node_Divide:
	case Node_Floor:
	case Node_Mod:
	case Node_Multiply:
	case Node_Power:
	case Node_Sign:
	case Node_Subtract:
	    return (int) QVariant::Double;

	/*
	  String functions.
	*/
	case Node_Length:
	    return (int) QVariant::Int;
	case Node_Lower:
	case Node_Replace:
	case Node_Soundex:
	case Node_Substring:
	case Node_Translate:
	case Node_Upper:
	    return (int) QVariant::String;

	/*
	  Aggregate functions.
	*/
	case Node_Avg:
	    return (int) QVariant::Double;
	case Node_Max:
	case Node_Min:
	case Node_Sum:
	    // type of sum(X) is type of X
	    return exprType( expr.toList()[1] );
	case Node_Count:
	    return (int) QVariant::Int;
	case Node_ResolvedField:
	    return expr;
	default:
	    return (int) QVariant::Invalid;
	}
    } else {
	return (int) expr.type();
    }
}

/*
  Emits code for the scalar expression or conditional expression expr.
  In the latter case, trueLab is the address to goto if the expression
  is TRUE, and falseLab is the address to goto if it is FALSE.
*/
void Parser::emitExpr( const QVariant& expr, int trueLab, int falseLab )
{
    /*
      An expression is an atom or a list. A list has the operator as
      first element and the operands afterwards. Cf. LISP.
    */
    if ( expr.type() == QVariant::List ) {
	QValueList<QVariant>::ConstIterator v = expr.listBegin();
	int node = (*v).toInt();
	LocalSQLOp *op = 0;
	int tableId;
	QString field;
	int fieldNo;
	int resultColumn;
	int nextCond;

	switch ( node ) {
	case Node_Avg:
	    resultColumn = (*++v).toList()[1].toInt();
	    yyProg->append( new PushGroupAvg(AuxResultId, resultColumn) );
	    break;
	case Node_And:
	    nextCond = yyNextLabel--;
	    emitExpr( *++v, nextCond, falseLab );
	    yyProg->appendLabel( nextCond );
	    emitExpr( *++v, trueLab, falseLab );
	    break;
	case Node_Count:
	    resultColumn = (*++v).toList()[1].toInt();
	    yyProg->append( new PushGroupCount(AuxResultId, resultColumn) );
	    break;
	case Node_In:
	    emitExpr( *++v );
	    yyProg->append( new Push(*++v) );
	    yyProg->append( new In(trueLab, falseLab) );
	    break;
	case Node_Like:
	    emitExpr( *++v );
	    yyProg->append( new Like((*++v).toString(), trueLab, falseLab) );
	    break;
	case Node_Max:
	    resultColumn = (*++v).toList()[1].toInt();
	    yyProg->append( new PushGroupMax(AuxResultId, resultColumn) );
	    break;
	case Node_Min:
	    resultColumn = (*++v).toList()[1].toInt();
	    yyProg->append( new PushGroupMin(AuxResultId, resultColumn) );
	    break;
	case Node_Not:
	    emitExpr( *++v, falseLab, trueLab );
	    break;
	case Node_Or:
	    nextCond = yyNextLabel--;
	    emitExpr( *++v, trueLab, nextCond );
	    yyProg->appendLabel( nextCond );
	    emitExpr( *++v, trueLab, falseLab );
	    break;
	case Node_ResolvedField:
	    tableId = (*++v).toInt();
	    field = (*++v).toString();
	    yyProg->append( new PushFieldValue(tableId, field) );
	    break;
	case Node_ResolvedStar:
	    ++v;
	    while ( v != expr.listEnd() ) {
		tableId = (*v).toInt();
		yyProg->append( new PushStarValue(tableId) );
		++v;
	    }
	    break;
	case Node_ResultColumnNo:
	    fieldNo = (*++v).toInt();
	    yyProg->append( new PushGroupValue(AuxResultId, fieldNo) );
	    break;
	case Node_Sum:
	    resultColumn = (*++v).toList()[1].toInt();
	    yyProg->append( new PushGroupSum(AuxResultId, resultColumn) );
	    break;
	default:
	    ++v;
	    while ( v != expr.listEnd() ) {
		emitExpr( *v );
		++v;
	    }

	    switch ( node ) {
	    case Node_Eq:
		op = new Eq( trueLab, falseLab );
		break;
	    case Node_NotEq:
		op = new Eq( falseLab, trueLab );
		break;
	    case Node_LessThan:
		op = new Lt( trueLab, falseLab );
		break;
	    case Node_GreaterEq:
		op = new Lt( falseLab, trueLab );
		break;
	    case Node_Add:
		op = new Add;
		break;
	    case Node_Subtract:
		op = new Subtract;
		break;
	    case Node_Multiply:
		op = new Multiply;
		break;
	    case Node_Divide:
		op = new Divide;
		break;
	    case Node_Abs:
		op = new Abs;
		break;
	    case Node_Ceil:
		op = new Ceil;
		break;
	    case Node_Floor:
		op = new Floor;
		break;
	    case Node_Length:
		op = new Length;
		break;
	    case Node_Lower:
		op = new Lower;
		break;
	    case Node_Mod:
		op = new Mod;
		break;
	    case Node_Power:
		op = new Power;
		break;
	    case Node_Replace:
		op = new Replace;
		break;
	    case Node_Sign:
		op = new Sign;
		break;
	    case Node_Soundex:
		op = new Soundex;
		break;
	    case Node_Substring:
		op = new Substring;
		break;
	    case Node_Translate:
		op = new Translate;
		break;
	    case Node_Upper:
		op = new Upper;
		break;
	    default:
		error( "Internal error: Unknown node type %d", node );
	    }
	    yyProg->append( op );
	}
    } else {
	yyProg->append( new Push(expr) );
    }
}

/*
  Emits the code for a 'where' clause. Together, cond and constants
  specify what rows to keep; selectColumns and selectColumnNames tell
  which columns to save. If the 'where' clause is only for marking
  (not saving), these two lists should be empty.
*/
void Parser::emitWhere( QVariant *cond, QValueList<QVariant> *constants,
			int resultId, const QValueList<QVariant>& selectColumns,
			const QStringList& selectColumnNames )
{
    QValueList<QVariant> optimizableConstants;
    QValueList<QVariant> unoptimizableConstants;

    resolveFieldNames( cond );

    QValueList<QVariant>::Iterator c = constants->begin();
    while ( c != constants->end() ) {
	resolveFieldNames( &*c );
	int tableId = (*c).asList()[1].asList()[1].toInt();
	if ( tableId < 0 )
	    unoptimizableConstants.append( *c );
	else
	    optimizableConstants.append( *c );
	++c;
    }
    pourConstantsIntoCondition( cond, &unoptimizableConstants );
    emitWhereLoop( *cond, optimizableConstants, resultId, selectColumns,
		   selectColumnNames );
}

/*
  Emits the nested 'where' loops. This code is very clever and tries
  to find out when a RangeSave or RangeMark is possible. Its
  parameters are similar to those of emitWhere(), but it also accepts
  level to tell which loop is being processed in case of nested loops.
*/
void Parser::emitWhereLoop( const QVariant& cond,
			    const QValueList<QVariant>& constants,
			    int resultId,
			    const QValueList<QVariant>& selectColumns,
			    const QStringList& selectColumnNames, int level )
{
    QValueList<QVariant> rangeSaveColumns;
    int tableId = yyActiveTableIds[level];
    int lastLevel = (int) yyActiveTableIds.count() - 1;
    bool saving = !selectColumns.isEmpty();
    bool needLoop = ( cond.isValid() || lastLevel > 0 );

    if ( !needLoop ) {
	QValueList<QVariant>::ConstIterator c = selectColumns.begin();
	while ( c != selectColumns.end() ) {
	    if ( (*c).type() == QVariant::List ) {
		if ( (*c).toList()[0].toInt() == Node_ResolvedField ) {
		    QValueList<QVariant> col;
		    col.append( (*c).toList()[2] );
		    rangeSaveColumns.append( col );
		} else {
		    needLoop = TRUE;
		    break;
		}
	    } else {
		rangeSaveColumns.append( *c );
	    }
	    ++c;
	}
    }

    QValueList<QVariant> constantsForLevel;
    if ( yyActiveTableIds.count() == 1 ) {
	constantsForLevel = constants;
    } else {
	QValueList<QVariant>::ConstIterator c = constants.begin();
	while ( c != constants.end() ) {
	    if ( (*c).toList()[1].toList()[1].toInt() == tableId )
		constantsForLevel.append( *c );
	    ++c;
	}
    }

    if ( saving && level == 0 )
	emitCreateResult( resultId, selectColumnNames, selectColumns );

    if ( needLoop && constantsForLevel.isEmpty() ) {
	yyProg->append( new MarkAll(tableId) );
    } else {
	if ( saving && !needLoop ) {
	    yyProg->append( new PushSeparator );
	    emitConstants( constantsForLevel );

	    yyProg->append( new PushSeparator );
	    QValueList<QVariant>::ConstIterator c = rangeSaveColumns.begin();
	    while ( c != rangeSaveColumns.end() ) {
		yyProg->append( new Push(*c) );
		++c;
	    }
	    yyProg->append( new MakeList );
	    yyProg->append( new MakeList );
	    yyProg->append( new RangeSave(tableId, resultId) );
	} else {
	    emitConstants( constantsForLevel );
	    yyProg->append( new RangeMark(tableId) );
	}
    }

    if ( needLoop ) {
	int next = yyNextLabel--;
	int end = yyNextLabel--;

	yyProg->appendLabel( next );
	yyProg->append( new NextMarked(tableId, end) );

	if ( level == lastLevel ) {
	    if ( saving ) {
		int save = yyNextLabel--;
		if ( cond.isValid() ) {
		    emitExpr( cond, save, next );
		    yyProg->appendLabel( save );
		}
		emitExprList( selectColumns );
		yyProg->append( new SaveResult(resultId) );
	    } else {
		int unmark = yyNextLabel--;
		if ( cond.isValid() ) {
		    emitExpr( cond, next, unmark );
		    yyProg->appendLabel( unmark );
		} else {
		    yyProg->append( new Goto(next) );
		}
		yyProg->append( new Unmark(tableId) );
	    }
	} else {
	    emitWhereLoop( cond, constants, resultId, selectColumns,
			   selectColumnNames, level + 1 );
	}
	yyProg->append( new Goto(next) );
	yyProg->appendLabel( end );

	if ( !saving )
	    yyProg->append( new RewindMarked(tableId) );
    }
}

void Parser::emitExprList( const QValueList<QVariant>& exprs )
{
    yyProg->append( new PushSeparator );
    QValueList<QVariant>::ConstIterator e = exprs.begin();
    while ( e != exprs.end() ) {
	emitExpr( *e );
	++e;
    }
    yyProg->append( new MakeList );
}

/*
  Constants are trees of the form

      (Node_Eq (Node_ResolvedField table-id field-name) constant-value)

   We want to generate (field-name constant-value). The table-id is
   irrelevant because this is used only for single-table statements.
*/
void Parser::emitConstants( const QValueList<QVariant>& constants )
{
    QValueList<QVariant>::ConstIterator v = constants.begin();

    yyProg->append( new PushSeparator );
    while ( v != constants.end() ) {
	QValueList<QVariant>::ConstIterator w = (*v).listBegin();
	QString name = (*++w).toList()[2].toString();
	QVariant valueExpr = *++w;

	yyProg->append( new PushSeparator );
	yyProg->append( new Push(name) );
	emitExpr( valueExpr );
	yyProg->append( new MakeList );
	++v;
    }
    yyProg->append( new MakeList );
}

/*
  Emits the stuff expected by CreateResult.
*/
void Parser::emitFieldDesc( const QString& columnName, const QVariant& column )
{
    QVariant borrowFrom;

    if ( column.type() == QVariant::List &&
	 column.toList()[0].toInt() == Node_ResolvedStar ) {
	QValueList<QVariant>::ConstIterator v = column.listBegin();
	++v;
	while ( v != column.listEnd() ) {
	    int tableId = (*v).toInt();
	    yyProg->append( new PushStarDesc(tableId) );
	    ++v;
	}
    } else {
	yyProg->append( new PushSeparator );
	yyProg->append( new Push(columnName) );
	borrowFrom = exprType( column );
	if ( borrowFrom.type() == QVariant::List ) {
	    int id = borrowFrom.asList()[1].toInt();
	    QString name = borrowFrom.asList()[2].toString();
	    yyProg->append( new PushFieldType(id, name) );
	} else {
	    yyProg->append( new Push(borrowFrom.toInt()) );
	}
	yyProg->append( new MakeList );
    }
}

/*
  Emits the instructions to create a result set.
*/
void Parser::emitCreateResult( int resultId, const QStringList& columnNames,
			       const QValueList<QVariant>& columns )
{
    QStringList::ConstIterator nam = columnNames.begin();
    QValueList<QVariant>::ConstIterator col = columns.begin();

    yyProg->append( new PushSeparator );
    while ( nam != columnNames.end() )
	emitFieldDesc( *nam++, *col++ );
    yyProg->append( new MakeList );
    yyProg->append( new CreateResult(resultId) );
}

/*
  Makes tableName active. This means that the table will be open and
  that the table will be used in resolving fields. Being open means
  nothing, as there are special mechanisms to keep tables open as long
  as possible to avoid repeatedly opening and closing tables.
*/
int Parser::activateTable( const QString& tableName )
{
    QMap<QString, int>::ConstIterator t = yyOpenedTableMap.find( tableName );
    if ( t == yyOpenedTableMap.end() ) {
	int tableId = yyOpenedTableMap.count();
	t = yyOpenedTableMap.insert( tableName, tableId );
	yyProg->append( new Open(tableId, tableName) );
    }
    yyActiveTableMap.insert( tableName, *t );
    yyActiveTableIds.append( *t );
    return *t;
}

/*
  Makes all tables inactive. The tables are not closed yet as an
  optimization.
*/
void Parser::deactivateTables()
{
    yyActiveTableMap.clear();
    yyActiveTableIds.clear();
    yyLookedUpColumnMap.clear();
}

/*
  Closes all opened tables.
*/
void Parser::closeAllTables()
{
    deactivateTables();
    int n = (int) yyOpenedTableMap.count();
    for ( int i = 0; i < n; i++ )
	yyProg->append( new Close(i) );
    yyOpenedTableMap.clear();
}

/*
  Emits instructions to create an index on given columns in tableId.
*/
void Parser::createIndex( int tableId, const QStringList& columns, bool unique,
			  bool notNull )
{
    QStringList::ConstIterator col = columns.begin();

    yyProg->append( new PushSeparator );
    while ( col != columns.end() ) {
	yyProg->append( new Push(*col) );
	++col;
    }
    yyProg->append( new MakeList );
    yyProg->append( new CreateIndex(tableId, unique, notNull) );
}

/*
  Plugs back the constants into the condition. Constants are needed
  equalities with constants like "id = 5" or "sex = 'male'". They are
  parsed separately because that simplifies optimization later.
*/
void Parser::pourConstantsIntoCondition( QVariant *cond,
					 QValueList<QVariant> *constants )
{
    if ( constants == 0 )
	return;

    QValueList<QVariant>::ConstIterator c = constants->begin();
    while ( c != constants->end() ) {
	if ( cond->isValid() ) {
	    QValueList<QVariant> andExpr;
	    andExpr.append( (int) Node_And );
	    andExpr.append( *c );
	    andExpr.append( *cond );
	    *cond = andExpr;
	} else {
	    *cond = *c;
	}
	++c;
    }
    constants->clear();
}

/*
  Returns the number of the column in the result set, using
  resultColumnNos to resolve that.
*/
int Parser::columnNo( const QVariant& column,
		      const QMap<QString, QMap<int, int> >& resultColumnNos )
{
    int tableId = column.toList()[1].toInt();
    QString fieldName = column.toList()[2].toString();

    QMap<QString, QMap<int, int> >::ConstIterator d =
	    resultColumnNos.find( fieldName );

    if ( d == resultColumnNos.end() ) {
	error( "Cannot use field '%s' here", fieldName.latin1() );
	return -1;
    } else {
	QMap<int, int>::ConstIterator e;
	if ( tableId < 0 ) {
	    e = (*d).begin(); // (*d).count() == 1
	} else {
	    e = (*d).find( tableId );
	    if ( e == (*d).end() ) {
		error( "Cannot use field '%s' here", fieldName.latin1() );
		return -1;
	    }
	}
	return *e;
    }
}

/*
  Here starts the recursive-descent parser for SQL. If you don't
  understand that code, read sections 4.1 to 4.4 in the Dragon Book.

  The grammar is not specified explicitly, but it should be fairly
  easy to retrieve it from the parser below if you really need it.
*/

/*
  The two following functions are basic token matchers. Both behave
  the same when the token matches. Otherwise, matchOrSkip() eats the
  next token, but matchOrInsert() does not. Currently, this doesn't
  matter since we stop after the first error.
*/
void Parser::matchOrInsert( int target, const QString& targetStr )
{
    if ( yyTok == target )
	yyTok = getToken();
    else
	error( "Expected %s before '%s'", targetStr.latin1(), yyLex );
}

void Parser::matchOrSkip( int target, const QString& targetStr )
{
    if ( yyTok != target )
	error( "Met '%s' where %s was expected", yyLex, targetStr.latin1() );
    yyTok = getToken();
}

QString Parser::matchName()
{
    QString name( yyLex );
    matchOrSkip( Tok_Name, "name" );
    return name;
}

QString Parser::matchTable()
{
    QString name;

    name = matchName();
    if ( yyTok == Tok_Dot ) {
	yyTok = getToken();
	name = matchName();
    }
    return name;
}

QString Parser::matchColumnName()
{
    int start = yyPos;
    matchScalarExpr();
    return fixedColumnName( yyIn.mid(start, yyPos - start) );
}

QVariant Parser::matchColumnRef()
{
    QString tableName;
    QString columnName;
    int numDots = 0;

    while ( TRUE ) {
	if ( yyTok == Tok_Aster ) {
	    QValueList<QVariant> aster;
	    yyTok = getToken();
	    aster.append( (int) Node_UnresolvedStar );
	    aster.append( columnName );
	    return aster;
	}

	columnName = matchName();
	if ( yyTok != Tok_Dot || numDots == 2 )
	    break;
	yyTok = getToken();
	numDots++;
	tableName = columnName;
    }

    QValueList<QVariant> expr;
    expr.append( (int) Node_UnresolvedField );
    expr.append( tableName );
    expr.append( columnName );
    return expr;
}

QVariant Parser::matchAggregateArgument()
{
    QVariant arg;

    matchOrInsert( Tok_LeftParen, "'('" );
    if ( yyTok == Tok_Aster ) {
	yyTok = getToken();
    } else {
	arg = matchColumnRef();
    }
    matchOrInsert( Tok_RightParen, "')'" );
    return arg;
}

void Parser::matchFunctionArguments( int numArguments,
				     QValueList<QVariant> *expr )
{
    int met = 0;

    matchOrInsert( Tok_LeftParen, "'('" );
    if ( yyTok != Tok_RightParen ) {
	while ( TRUE ) {
	    expr->append( matchScalarExpr() );
	    met++;

	    if ( yyTok != Tok_Comma )
		break;
	    yyTok = getToken();
	}
    }
    matchOrInsert( Tok_RightParen, "')'" );
    if ( met != numArguments )
	error( "Met %d argument%s where %d w%s expected", met,
	       met == 1 ? "" : "s", numArguments,
	       numArguments == 1 ? "as" : "ere" );
}

QVariant Parser::matchPrimaryExpr()
{
    QVariant right;
    bool uminus = FALSE;
    int node;
    int n;

    while ( yyTok == Tok_Plus || yyTok == Tok_Minus ) {
	if ( yyTok == Tok_Minus )
	    uminus = !uminus;
	yyTok = getToken();
    }

    switch ( yyTok ) {
    case Tok_LeftParen:
	yyTok = getToken();
	right = matchScalarExpr();
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    case Tok_Name:
    case Tok_Aster:
	right = matchColumnRef();
	break;
    case Tok_IntNum:
	right = yyNum;
	yyTok = getToken();
	break;
    case Tok_ApproxNum:
	right = yyNum;
	yyTok = getToken();
	break;
    case Tok_String:
	right = yyStr;
	yyTok = getToken();
	break;
    case Tok_null:
	right = NullRep;
	yyTok = getToken();
	break;
    default:
	switch ( yyTok ) {
	case Tok_avg:
	    node = Node_Avg;
	    n = 0;
	    break;
	case Tok_count:
	    node = Node_Count;
	    n = 0;
	    break;
	case Tok_max:
	    node = Node_Max;
	    n = 0;
	    break;
	case Tok_min:
	    node = Node_Min;
	    n = 0;
	    break;
	case Tok_sum:
	    node = Node_Sum;
	    n = 0;
	    break;
	case Tok_abs:
	    node = Node_Abs;
	    n = 1;
	    break;
	case Tok_ceil:
	    node = Node_Ceil;
	    n = 1;
	    break;
	case Tok_floor:
	    node = Node_Floor;
	    n = 1;
	    break;
	case Tok_length:
	    node = Node_Length;
	    n = 1;
	    break;
	case Tok_lower:
	    node = Node_Lower;
	    n = 1;
	    break;
	case Tok_sign:
	    node = Node_Sign;
	    n = 1;
	    break;
	case Tok_soundex:
	    node = Node_Soundex;
	    n = 1;
	    break;
	case Tok_upper:
	    node = Node_Upper;
	    n = 1;
	    break;
	case Tok_mod:
	    node = Node_Mod;
	    n = 2;
	    break;
	case Tok_power:
	    node = Node_Power;
	    n = 2;
	    break;
	case Tok_replace:
	    node = Node_Replace;
	    n = 3;
	    break;
	case Tok_substring:
	    node = Node_Substring;
	    n = 3;
	    break;
	case Tok_translate:
	    node = Node_Translate;
	    n = 3;
	    break;
	default:
	    n = -1;
	    error( "Met '%s' where primary expression was expected", yyLex );
	}

	yyTok = getToken();
	right.asList().append( node );
	if ( n == 0 ) {
	    right.asList().append( matchAggregateArgument() );
	    yyNumAggregateOccs++;
	} else {
	    matchFunctionArguments( n, &right.asList() );
	}
    }

    if ( uminus ) {
	QValueList<QVariant> uminusExp;
	uminusExp.append( (int) Node_Subtract );
	uminusExp.append( 0 );
	uminusExp.append( right );
	right = uminusExp;
    }
    return right;
}

QVariant Parser::matchMultiplicativeExpr()
{
    QVariant left;

    left = matchPrimaryExpr();
    while ( yyTok == Tok_Aster || yyTok == Tok_Div ) {
	QValueList<QVariant> multExp;
	int node = yyTok == Tok_Aster ? Node_Multiply : Node_Divide;
	multExp.append( node );
	multExp.append( left );
	yyTok = getToken();
	multExp.append( matchPrimaryExpr() );
	left = multExp;
    }
    return left;
}

QVariant Parser::matchScalarExpr()
{
    QVariant left;

    left = matchMultiplicativeExpr();
    while ( yyTok == Tok_Plus || yyTok == Tok_Minus ) {
	QValueList<QVariant> scalExp;
	int node = yyTok == Tok_Plus ? Node_Add : Node_Subtract;
	scalExp.append( node );
	scalExp.append( left );
	yyTok = getToken();
	scalExp.append( matchMultiplicativeExpr() );
	left = scalExp;
    }
    return left;
}

QVariant Parser::matchAtom()
{
    QVariant atom;

    if ( yyTok == Tok_String ) {
	atom = yyStr;
	yyTok = getToken();
    } else if ( yyTok == Tok_IntNum || yyTok == Tok_ApproxNum ) {
	atom = yyNum;
	yyTok = getToken();
    } else {
	error( "Expected string or numeral rather than '%s'", yyLex );
    }
    return atom;
}

QVariant Parser::matchAtomList()
{
    QValueList<QVariant> atoms;

    while ( TRUE ) {
	atoms.append( matchAtom() );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
    return atoms;
}

QVariant Parser::matchBetween( const QVariant& left )
{
    QValueList<QVariant> pred;
    QValueList<QVariant> subPred;

    pred.append( (int) Node_And );
    yyTok = getToken();
    subPred.append( (int) Node_GreaterEq );
    subPred.append( left );
    subPred.append( matchScalarExpr() );
    subPred.append( pred );
    matchOrInsert( Tok_and, "'and'" );

    subPred.clear();
    subPred.append( (int) Node_GreaterEq );
    subPred.append( matchScalarExpr() );
    subPred.append( left );
    pred.append( subPred );
    return pred;
}

QVariant Parser::matchIn( const QVariant& left )
{
    QValueList<QVariant> pred;

    pred.append( (int) Node_In );
    yyTok = getToken();
    matchOrInsert( Tok_LeftParen, "'('" );
    pred.append( left );
    pred.append( matchAtomList() );
    matchOrInsert( Tok_RightParen, "')'" );
    return pred;
}

QVariant Parser::matchLike( const QVariant& left )
{
    QValueList<QVariant> pred;

    pred.append( (int) Node_Like );
    yyTok = getToken();
    pred.append( left );
    pred.append( matchAtom() );
    return pred;
}

QVariant Parser::matchPredicate( QValueList<QVariant> *constants )
{
    QValueList<QVariant> pred;
    QValueList<QVariant> subPred;
    QVariant left;
    QVariant right;
    bool leftIsColumnRef = ( yyTok == Tok_Name );
    bool unot = FALSE;

    left = matchScalarExpr();

    switch ( yyTok ) {
    case Tok_Equal:
	pred.append( (int) Node_Eq );
	yyTok = getToken();
	right = matchScalarExpr();
	pred.append( left );
	pred.append( right );
	if ( constants != 0 && leftIsColumnRef &&
	     right.type() != QVariant::List ) {
	    constants->append( pred );
	    return QVariant();
	}
	break;
    case Tok_NotEq:
	pred.append( (int) Node_NotEq );
	yyTok = getToken();
	pred.append( left );
	pred.append( matchScalarExpr() );
	break;
    case Tok_LessThan:
	pred.append( (int) Node_LessThan );
	yyTok = getToken();
	pred.append( left );
	pred.append( matchScalarExpr() );
	break;
    case Tok_GreaterEq:
	pred.append( (int) Node_GreaterEq );
	yyTok = getToken();
	pred.append( left );
	pred.append( matchScalarExpr() );
	break;
    case Tok_GreaterThan:
	yyTok = getToken();
	pred.append( (int) Node_LessThan );
	pred.append( matchScalarExpr() );
	pred.append( left );
	break;
    case Tok_LessEq:
	yyTok = getToken();
	pred.append( (int) Node_GreaterEq );
	pred.append( matchScalarExpr() );
	pred.append( left );
	break;
    case Tok_between:
	pred = matchBetween( left ).toList();
	break;
    case Tok_in:
	pred = matchIn( left ).toList();
	break;
    case Tok_is:
	yyTok = getToken();
	if ( yyTok == Tok_not ) {
	    yyTok = getToken();
	    unot = TRUE;
	}
	matchOrSkip( Tok_null, "'null'" );

	pred.append( (int) Node_Eq );
	pred.append( left );
	pred.append( NullRep );
	break;
    case Tok_like:
	pred = matchLike( left ).toList();
	break;
    case Tok_not:
	yyTok = getToken();
	unot = TRUE;

	switch ( yyTok ) {
	case Tok_between:
	    pred = matchBetween( left ).toList();
	    break;
	case Tok_in:
	    pred = matchIn( left ).toList();
	    break;
	case Tok_like:
	    pred = matchLike( left ).toList();
	    break;
	default:
	    error( "Expected 'between', 'in' or 'like' after 'not'" );
	}
    default:
	error( "Unexpected '%s' in predicate", yyLex );
    }

    if ( unot ) {
	QValueList<QVariant> unotExp;
	unotExp.append( (int) Node_Not );
	unotExp.append( pred );
	pred = unotExp;
    }
    return pred;
}

QVariant Parser::matchPrimarySearchCondition( QValueList<QVariant> *constants )
{
    if ( yyTok == Tok_LeftParen ) {
	yyTok = getToken();
	QVariant expr = matchSearchCondition( constants );
	matchOrInsert( Tok_RightParen, "')'" );
	return expr;
    } else if ( yyTok == Tok_not ) {
	QValueList<QVariant> notExpr;

	notExpr.append( (int) Node_Not );
	yyTok = getToken();
	notExpr.append( matchSearchCondition() );
	return notExpr;
    } else {
	return matchPredicate( constants );
    }
}

QVariant Parser::matchAndSearchCondition( QValueList<QVariant> *constants )
{
    QVariant left;
    QVariant right;

    while ( TRUE ) {
	right = matchPrimarySearchCondition( constants );

	if ( right.isValid() ) {
	    if ( left.isValid() ) {
		QValueList<QVariant> andCond;
		andCond.append( (int) Node_And );
		andCond.append( left );
		andCond.append( right );
		left = andCond;
	    } else {
		left = right;
	    }
	}

	if ( yyTok != Tok_and )
	    break;
	yyTok = getToken();
    }
    return left;
}

QVariant Parser::matchSearchCondition( QValueList<QVariant> *constants )
{
    QVariant left;

    left = matchAndSearchCondition( constants );
    if ( yyTok == Tok_or ) {
	pourConstantsIntoCondition( &left, constants );
	do {
	    QValueList<QVariant> cond;
	    cond.append( (int) Node_Or );
	    yyTok = getToken();
	    cond.append( left );
	    cond.append( matchAndSearchCondition() );
	    left = cond;
	} while ( yyTok == Tok_or );
    }
    return left;
}

QVariant Parser::matchOptWhereClause( QValueList<QVariant> *constants )
{
    QVariant cond;

    if ( yyTok == Tok_where ) {
	yyTok = getToken();
	cond = matchSearchCondition( constants );
    }
    return cond;
}

QValueList<QVariant> Parser::matchOptGroupByClause()
{
    QValueList<QVariant> columns;

    if ( yyTok == Tok_group ) {
	yyTok = getToken();
	matchOrInsert( Tok_by, "'by'" );

	while ( TRUE ) {
	    columns.append( matchColumnRef() );

	    if ( yyTok != Tok_Comma )
		break;
	    yyTok = getToken();
	}
    }
    return columns;
}

QVariant Parser::matchOptHavingClause()
{
    QVariant cond;

    if ( yyTok == Tok_having ) {
	yyTok = getToken();
	cond = matchSearchCondition();
    }
    return cond;
}

void Parser::matchCommitStatement()
{
    yyTok = getToken();
    if ( yyTok == Tok_work )
	yyTok = getToken();
}

void Parser::matchDataType()
{
    int type = (int) QVariant::Invalid;
    int len = 1;
    int prec = 0;

    switch ( yyTok ) {
    case Tok_character:
    case Tok_varchar:
	type = QVariant::String;
	yyTok = getToken();
	matchOrInsert( Tok_LeftParen, "')'" );
	len = (int) yyNum;
	matchOrSkip( Tok_IntNum, "integer" );
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    case Tok_date:
	type = QVariant::DateTime;
	yyTok = getToken();
	break;
    case Tok_integer:
	len = 10;
	type = QVariant::Int;
	yyTok = getToken();
	break;
    case Tok_decimal:
    case Tok_double:
    case Tok_float:
    case Tok_numeric:
	type = QVariant::Double;
	yyTok = getToken();
	if ( yyTok == Tok_LeftParen ) {
	    yyTok = getToken();
	    len = (int) yyNum;
	    matchOrSkip( Tok_IntNum, "integer" );
	    if ( yyTok == Tok_Comma ) {
		yyTok = getToken();
		prec = (int) yyNum;
		matchOrSkip( Tok_IntNum, "integer" );
	    }
	    matchOrInsert( Tok_RightParen, "')'" );
	}
	break;
    default:
	error( "Met '%s' where 'character', 'date', 'decimal', 'double',"
	       " 'float', 'integer', 'numeric' or 'varchar' was expected",
	       yyLex );
    }

    yyProg->append( new Push(type) );
    yyProg->append( new Push(len) );
    yyProg->append( new Push(prec) );
}

QStringList Parser::matchColumnList()
{
    QStringList columns;

    while ( TRUE ) {
	columns.append( matchName() );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
    return columns;
}

void Parser::matchTableConstraintDef()
{
    switch ( yyTok ) {
    case Tok_check:
	yyTok = getToken();
	matchOrInsert( Tok_LeftParen, "'('" );
	matchSearchCondition();
	matchOrInsert( Tok_RightParen, "')'" );
	warning( "'check' clause unsupported (ignored)" );
	break;
    case Tok_foreign:
	yyTok = getToken();
	matchOrInsert( Tok_key, "'key'" );
	warning( "'foreign key' clause unsupported (will still create an"
		 " index)" );
	matchOrInsert( Tok_LeftParen, "'('" );
	yyNeedIndex[NotUnique][Null].append( matchColumnList() );
	matchOrInsert( Tok_RightParen, "')'" );
	matchOrInsert( Tok_references, "'references'" );
	matchTable();
	if ( yyTok == Tok_LeftParen ) {
	    yyTok = getToken();
	    matchColumnList();
	    matchOrInsert( Tok_RightParen, "')'" );
	}
	break;
    case Tok_primary:
	yyTok = getToken();
	matchOrInsert( Tok_key, "'key'" );
	matchOrInsert( Tok_LeftParen, "'('" );
	yyNeedIndex[Unique][NotNull].append( matchColumnList() );
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    case Tok_unique:
	yyTok = getToken();
	matchOrInsert( Tok_LeftParen, "'('" );
	yyNeedIndex[Unique][Null].append( matchColumnList() );
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    default:
	error( "Expected field name or constraint" );
    }
}

void Parser::matchColumnDefOptions( const QString& column )
{
    while ( TRUE ) {
	switch ( yyTok ) {
	case Tok_check:
	    yyTok = getToken();
	    matchOrInsert( Tok_LeftParen, "'('" );
	    matchSearchCondition();
	    matchOrInsert( Tok_RightParen, "')'" );
	    warning( "'check' clause unsupported (ignored)" );
	    break;
	case Tok_default:
	    yyTok = getToken();
	    yyTok = getToken();
	    warning( "'default' clause unsupported (ignored)" );
	    break;
	case Tok_foreign:
	    yyTok = getToken();
	    matchOrSkip( Tok_key, "'key'" );
	    warning( "'foreign key' clause unsupported (will still create an"
		     " index)" );
	    yyNeedIndex[NotUnique][Null].append( column );
	    break;
	case Tok_not:
	    yyTok = getToken();
	    matchOrSkip( Tok_null, "'null'" );
	    warning( "'not null' clause unsupported (ignored)" );
	    break;
	case Tok_primary:
	    yyTok = getToken();
	    matchOrInsert( Tok_key, "'key'" );
	    yyNeedIndex[Unique][NotNull].append( column );
	    break;
	case Tok_references:
	    yyTok = getToken();
	    matchTable();
	    if ( yyTok == Tok_LeftParen ) {
		yyTok = getToken();
		matchColumnList();
		matchOrInsert( Tok_RightParen, "')'" );
	    }
	    warning( "'references' clause unsupported (ignored)" );
	    break;
	case Tok_unique:
	    yyTok = getToken();
	    yyNeedIndex[Unique][Null].append( column );
	    break;
	default:
	    return;
	}
    }
}

void Parser::matchBaseTableElement()
{
    bool notNull = FALSE;

    if ( yyTok == Tok_Name ) {
	QString column = yyLex;

	yyProg->append( new PushSeparator );
	yyProg->append( new Push(column) );
	yyTok = getToken();
	matchDataType();
	if ( yyTok == Tok_not ) {
	    yyTok = getToken();
	    matchOrSkip( Tok_null, "'null'" );
	    notNull = TRUE;
	}
	yyProg->append( new Push((int) notNull) );
	yyProg->append( new MakeList );

	matchColumnDefOptions( column );
    } else {
	matchTableConstraintDef();
    }
}

void Parser::matchBaseTableElementList()
{
    yyProg->append( new PushSeparator );
    while ( TRUE ) {
	matchBaseTableElement();

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
    yyProg->append( new MakeList );
}

void Parser::matchCreateStatement()
{
    QString table;
    QStringList columns;
    bool unique = FALSE;
    int tableId;

    yyTok = getToken();

    switch ( yyTok ) {
    case Tok_unique:
	yyTok = getToken();
	unique = TRUE;
	// fall through
    case Tok_index:
	matchOrInsert( Tok_index, "'index'" );
	if ( yyTok == Tok_Name ) {
	    warning( "Index name '%s' ignored", yyLex );
	    yyTok = getToken();
	}
	matchOrInsert( Tok_on, "'on'" );
	tableId = activateTable( matchTable() );

	matchOrInsert( Tok_LeftParen, "'('" );
	createIndex( tableId, matchColumnList(), unique, FALSE );
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    case Tok_table:
	yyTok = getToken();
	table = matchTable();
	matchOrInsert( Tok_LeftParen, "'('" );
	matchBaseTableElementList();
	matchOrInsert( Tok_RightParen, "')'" );
	yyProg->append( new Create(table) );

	tableId = activateTable( table );
	for ( int i = 0; i < 2; i++ ) {
	    for ( int j = 0; j < 2; j++ ) {
		while ( !yyNeedIndex[i][j].isEmpty() ) {
		    createIndex( tableId, yyNeedIndex[i][j].first(),
				 i == Unique, j == NotNull );
		    yyNeedIndex[i][j].remove( yyNeedIndex[i][j].begin() );
		}
	    }
	}
	break;
    case Tok_view:
	error( "Views are not supported" );
	break;
    default:
	error( "Expected 'index', 'table' or 'view' after 'create'" );
    }
}

void Parser::matchDeleteStatement()
{
    QVariant whereCond;
    QValueList<QVariant> whereConstants;
    QString table;

    yyTok = getToken();
    matchOrInsert( Tok_from, "'from'" );
    int tableId = activateTable( matchTable() );

    whereCond = matchOptWhereClause( &whereConstants );
    emitWhere( &whereCond, &whereConstants );

    yyProg->append( new DeleteMarked(tableId) );
}

void Parser::matchDropStatement()
{
    yyTok = getToken();
    matchOrInsert( Tok_table, "'table'" );
    yyProg->append( new Drop(matchTable()) );
}

void Parser::matchInsertExpr()
{
    QVariant expr = matchScalarExpr();
    resolveFieldNames( &expr );
    emitExpr( expr );
}

void Parser::matchInsertExprList( const QStringList& columns )
{
    QStringList::ConstIterator col = columns.begin();
    int columnNo = 0;

    yyProg->append( new PushSeparator );
    while ( TRUE ) {
	yyProg->append( new PushSeparator );
	if ( columns.isEmpty() ) {
	    yyProg->append( new Push(columnNo) );
	} else {
	    if ( col == columns.end() ) {
		error( "Met more values than columns" );
	    } else {
		yyProg->append( new Push(*col) );
		++col;
	    }
	}
	matchInsertExpr();
	yyProg->append( new MakeList );
	columnNo++;

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
    if ( col != columns.end() )
	error( "Met fewer values than columns" );
    yyProg->append( new MakeList );
}

void Parser::matchInsertStatement()
{
    QStringList columns;

    yyTok = getToken();
    matchOrInsert( Tok_into, "'into'" );
    int tableId = activateTable( matchTable() );

    if ( yyTok == Tok_LeftParen ) {
	yyTok = getToken();
	columns = matchColumnList();
	matchOrInsert( Tok_RightParen, "')'" );
    }
    matchOrInsert( Tok_values, "'values'" );
    matchOrInsert( Tok_LeftParen, "'('" );
    matchInsertExprList( columns );
    matchOrInsert( Tok_RightParen, "')'" );

    yyProg->append( new Insert(tableId) );
}

void Parser::matchRollbackStatement()
{
    yyTok = getToken();
    if ( yyTok == Tok_work )
	yyTok = getToken();
}

void Parser::matchFromClause()
{
    matchOrSkip( Tok_from, "'from'" );
    while ( TRUE ) {
	int tableId = activateTable( matchTable() );
	if ( yyTok == Tok_Name )
	    yyAliasTableMap.insert( matchName(), tableId );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
}

void Parser::matchOptOrderByClause( int resultId,
				    const QStringList& columnNames )
{
    if ( yyTok == Tok_order ) {
	yyTok = getToken();
	matchOrInsert( Tok_by, "'by'" );

	yyProg->append( new PushSeparator );
	while ( TRUE ) {
	    QString columnName = matchColumnName();
	    bool descending = FALSE;

	    yyProg->append( new PushSeparator );
	    int k = columnNames.findIndex( columnName );
	    if ( k == -1 )
		error( "No column named '%s'", columnName.latin1() );
	    yyProg->append( new Push(k) );

	    switch ( yyTok ) {
	    case Tok_desc:
		descending = TRUE;
		// fall through
	    case Tok_asc:
		yyTok = getToken();
	    }
	    yyProg->append( new Push((int) descending) );
	    yyProg->append( new MakeList );

	    if ( yyTok != Tok_Comma )
		break;
	    yyTok = getToken();
	}
	yyProg->append( new MakeList );
	yyProg->append( new Sort(resultId) );
    }
}

void Parser::matchSelectStatement()
{
    QValueList<QVariant> selectColumns;
    QStringList selectColumnNames;
    QValueList<QVariant> groupByColumns;
    QVariant whereCond;
    QValueList<QVariant> whereConstants;
    QVariant havingCond;

    yyTok = getToken();
    while ( TRUE ) {
	int start = yyPos;
	selectColumns.append( matchScalarExpr() );
	QString columnName = fixedColumnName( yyIn.mid(start, yyPos - start) );
	selectColumnNames.append( columnName );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }

    matchFromClause();
    resolveFieldNames( &selectColumns );

    whereCond = matchOptWhereClause( &whereConstants );

    groupByColumns = matchOptGroupByClause();
    resolveFieldNames( &groupByColumns );

    havingCond = matchOptHavingClause();

    if ( havingCond.isValid() && yyNumAggregateOccs == 0 )
	error( "Cannot have 'having' clause in such simple queries" );

    if ( groupByColumns.isEmpty() && yyNumAggregateOccs == 0 ) {
	/*
	  This is the common, easy case where one pass is enough. We
	  read from the tables and generate a result.
	*/

	emitWhere( &whereCond, &whereConstants, ResultId, selectColumns,
		   selectColumnNames );
	matchOptOrderByClause( ResultId, selectColumnNames );
    } else {
	/*
	  This is the general case where two passes are needed. The
	  first pass is a select of all the fields we will need later
	  and generates an intermediate result. That result is used to
	  create another result in the second pass.
	*/

	/*
	  First, let's find out what columns we need for the select.
	  Things can get very tricky when multi-table queries are
	  made and table names are omitted and have to be resolved at
	  run time. The column 'id' may also appear as 'author.id',
	  for example.
	*/
	// QMap<table-id, field-names>
	QMap<int, QStringList> usedFields;
	// QMap<table-id, field-name>
	QMap<int, QString> usedDynamicFields;
	// QMap<field-name, QMap<table-id, column>
	QMap<QString, QMap<int, int> > resultColumnNos;
	QValueList<QVariant> auxColumns;
	QStringList auxColumnNames;
	int n = 0;

	computeUsedFields( selectColumns, &usedFields );
	computeUsedFields( whereCond, &usedFields );
	computeUsedFields( groupByColumns, &usedFields );
	// computeUsedFields( havingCond, &usedFields );

	QMap<int, QStringList>::Iterator f = usedFields.begin();
	while ( f != usedFields.end() ) {
	    QStringList::Iterator g = (*f).begin();
	    while ( g != (*f).end() ) {
		if ( f.key() < 0 ) {
		    usedDynamicFields.insert( f.key(), *g );
		} else {
		    resultColumnNos[*g].insert( f.key(), n );
		    auxColumns.append( resolvedField(ResultId, *g) );
		    auxColumnNames.append( *g );
		    n++;
		}
		++g;
	    }
	    ++f;
	}

	/*
	  Check pertinence of dynamic fields. For example, 'id'
	  (dynamic) and 'author.id' (static) either refer to the same
	  field or the same table, or 'id' is ambiguous (say, with
	  'publisher.id').
	*/
	QMap<int, QString>::Iterator s = usedDynamicFields.begin();
	while ( s != usedDynamicFields.end() ) {
	    switch ( resultColumnNos[*s].count() ) {
	    case 0:
		resultColumnNos[*s].insert( s.key(), n );
		auxColumns.append( resolvedField(n, *s) );
		auxColumnNames.append( *s );
		n++;
		break;
	    case 1:
		break;
	    default:
		error( "Field '%s' is ambiguous", (*s).latin1() );
	    }
	    ++s;
	}

	/*
	  Do the select.
	*/
	emitWhere( &whereCond, &whereConstants, AuxResultId, auxColumns,
		   auxColumnNames );

	/*
	  Second pass.
	*/
	int next = yyNextLabel--;
	int save = yyNextLabel--;
	int end = yyNextLabel--;

	emitCreateResult( ResultId, selectColumnNames, selectColumns );

	yyProg->append( new PushSeparator );
	QValueList<QVariant>::Iterator c = groupByColumns.begin();
	while ( c != groupByColumns.end() ) {
	    yyProg->append( new Push(columnNo(*c, resultColumnNos)) );
	    ++c;
	}
	yyProg->append( new MakeList );
	yyProg->append( new MakeGroupSet(AuxResultId) );
	yyProg->appendLabel( next );
	yyProg->append( new NextGroupSet(AuxResultId, end) );

	resolveResultColumnNos( &havingCond, resultColumnNos );
	if ( havingCond.isValid() ) {
	    emitExpr( havingCond, save, next );
	    yyProg->appendLabel( save );
	}

	resolveResultColumnNos( &selectColumns, resultColumnNos );
	emitExprList( selectColumns );
	yyProg->append( new SaveResult(ResultId) );

	yyProg->append( new Goto(next) );
	yyProg->appendLabel( end );

	matchOptOrderByClause( ResultId, selectColumnNames );
    }
}

void Parser::matchUpdateStatement()
{
    QVariant whereCond;
    QValueList<QVariant> whereConstants;

    yyTok = getToken();
    int tableId = activateTable( matchTable() );
    matchOrInsert( Tok_set, "'set'" );

    QMap<QString, QVariant> assignments;
    QString left;
    QVariant right;

    while ( TRUE ) {
	left = matchName();
	matchOrInsert( Tok_Equal, "'='" );
	right = matchScalarExpr();
	resolveFieldNames( &right );
	assignments.insert( left, right );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }

    whereCond = matchOptWhereClause( &whereConstants );
    emitWhere( &whereCond, &whereConstants );

    int next = yyNextLabel--;
    int end = yyNextLabel--;

    yyProg->appendLabel( next );
    yyProg->append( new NextMarked(tableId, end) );

    QMap<QString, QVariant>::ConstIterator as = assignments.begin();

    yyProg->append( new PushSeparator );
    while ( as != assignments.end() ) {
	yyProg->append( new PushSeparator );
	yyProg->append( new Push(as.key()) );
	emitExpr( *as );
	yyProg->append( new MakeList );
	++as;
    }
    yyProg->append( new MakeList );
    yyProg->append( new Update(tableId) );
    yyProg->append( new Goto(next) );
    yyProg->appendLabel( end );
}

void Parser::matchManipulativeStatement()
{
    switch ( yyTok ) {
    case Tok_commit:
	matchCommitStatement();
	break;
    case Tok_create:
	matchCreateStatement();
	break;
    case Tok_delete:
	matchDeleteStatement();
	break;
    case Tok_drop:
	matchDropStatement();
	break;
    case Tok_insert:
	matchInsertStatement();
	break;
    case Tok_rollback:
	matchRollbackStatement();
	break;
    case Tok_select:
	matchSelectStatement();
	break;
    case Tok_update:
	matchUpdateStatement();
    }
}

void Parser::matchSql()
{
    while ( yyTok != Tok_Eoi ) {
	matchManipulativeStatement();
	deactivateTables();

	if ( yyTok != Tok_Semicolon )
	    break;
	yyTok = getToken();
    }
    if ( yyTok != Tok_Eoi )
	error( "Unexpected '%s' where ';' was expected", yyLex );

    closeAllTables();
}
