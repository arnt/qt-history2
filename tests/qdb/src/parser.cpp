/*
    Xbase project source code

    This file contains XBase SQL parser implementations

    Copyright (C) 2000 Jasmin Blanchette (jasmin@trolltech.com)
		       Dave Berton (db@trolltech.com)

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

#include <qstring.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "op.h"
#include "sqlinterpreter.h"

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

bool Parser::parse( const QString& commands, localsql::Environment *env )
{
    startTokenizer( commands );
    yyTok = getToken();
    yyEnv = env;
    yyProg = env->program();
    yyNextLabel = -1;
    yyOK = TRUE;
    yyOpenedTableMap.clear();
    yyActiveTableMap.clear();
    yyActiveTableIds.clear();
    yyLookedUpColumnMap.clear();

    matchSql();
    return yyOK;
}

enum { Tok_Eoi, Tok_Equal, Tok_NotEq, Tok_LessThan, Tok_GreaterThan,
       Tok_LessEq, Tok_GreaterEq, Tok_Plus, Tok_Minus, Tok_Aster,
       Tok_Div, Tok_LeftParen, Tok_RightParen, Tok_Comma, Tok_Dot,
       Tok_Colon, Tok_Semicolon, Tok_Name, Tok_IntNum, Tok_ApproxNum,
       Tok_String,

       Tok_all, Tok_and, Tok_any, Tok_asc, Tok_avg, Tok_between,
       Tok_by, Tok_character, Tok_check, Tok_close, Tok_commit,
       Tok_count, Tok_create, Tok_current, Tok_date, Tok_decimal,
       Tok_declare, Tok_default, Tok_delete, Tok_desc, Tok_distinct,
       Tok_double, Tok_escape, Tok_float, Tok_for, Tok_foreign,
       Tok_from, Tok_group, Tok_having, Tok_in, Tok_index,
       Tok_insert, Tok_integer, Tok_into, Tok_is, Tok_key, Tok_like,
       Tok_max, Tok_min, Tok_not, Tok_null, Tok_numeric, Tok_of,
       Tok_on, Tok_or, Tok_order, Tok_precision, Tok_primary,
       Tok_real, Tok_references, Tok_rollback, Tok_select, Tok_set,
       Tok_smallint, Tok_some, Tok_sum, Tok_table, Tok_to, Tok_union,
       Tok_unique, Tok_update, Tok_user, Tok_values, Tok_view, Tok_where,
       Tok_with, Tok_work };

#define HASH( first, omitted, last ) \
    ( ((((first) << 5) | (omitted)) << 7) | (last) )
#define CHECK( target ) \
    if ( qstricmp(target, yyLex) != 0 ) \
	break;

void Parser::startTokenizer( const QString& in )
{
    yyIn = in;
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
    //## need to use env->output() here

    va_list ap;

    va_start( ap, format );
    fprintf( stderr, "%d:%d: ", yyLineNo, yyColumnNo );
    vfprintf( stderr, format, ap );
    putc( '\n', stderr );
    va_end( ap );
}

void Parser::error( const char *format, ... )
{
    //## need to use env->setLastError() here

    va_list ap;

    va_start( ap, format );
    fprintf( stderr, "%d:%d: ", yyLineNo, yyColumnNo );
    vfprintf( stderr, format, ap );
    putc( '\n', stderr );
    va_end( ap );

    // ###
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
	    case HASH( 'a', 1, 'y' ):
		CHECK( "any" );
		return Tok_any;
	    case HASH( 'b', 0, 'y' ):
		CHECK( "by" );
		return Tok_by;
	    case HASH( 'b', 5, 'n' ):
		CHECK( "between" );
		return Tok_between;
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
	    case HASH( 'd', 2, 'c' ):
		CHECK( "desc" );
		return Tok_desc;
	    case HASH( 'd', 2, 'e' ):
		CHECK( "date" );
		return Tok_date;
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
	    case HASH( 'p', 5, 'y' ):
		CHECK( "primary" );
		return Tok_primary;
	    case HASH( 'p', 7, 'n' ):
		CHECK( "precision" );
		return Tok_precision;
	    case HASH( 'r', 2, 'l' ):
		CHECK( "real" );
		return Tok_real;
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
	    case HASH( 's', 4, 't' ):
		CHECK( "select" );
		return Tok_select;
	    case HASH( 's', 6, 't' ):
		CHECK( "smallint" );
		return Tok_smallint;
	    case HASH( 's', 2, 'e' ):
		CHECK( "some" );
		return Tok_some;
	    case HASH( 't', 0, 'o' ):
		CHECK( "to" );
		return Tok_to;
	    case HASH( 't', 3, 'e' ):
		CHECK( "table" );
		return Tok_table;
	    case HASH( 'u', 2, 'r' ):
		CHECK( "user" );
		return Tok_user;
	    case HASH( 'u', 3, 'n' ):
		CHECK( "union" );
		return Tok_union;
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
		if ( yyCh == '-' ) {
		    do {
			readChar();
		    } while ( yyCh != '\n' );
		} else {
		    return Tok_Minus;
		}
		break;
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

void Parser::lookupNames( QVariant *expr )
{
    if ( expr->type() == QVariant::List ) {
	QValueList<QVariant>::Iterator v = expr->asList().begin();
	int tok = (*v).toInt();

	if ( tok == Tok_Name ) {
	    QValueList<QVariant>::Iterator updateMe = ++v;
	    QString tableName = (*updateMe).toString();
	    QString columnName = (*++v).toString();
	    QMap<QString, int>::ConstIterator id;

	    if ( tableName.isEmpty() ) {
		int numTables = (int) yyActiveTableIds.count();
		if ( numTables == 1 ) {
		    *updateMe = yyActiveTableIds.first();
		} else {
		    id = yyLookedUpColumnMap.find( columnName );
		    if ( id == yyLookedUpColumnMap.end() ) {
			int aliasId = -( yyLookedUpColumnMap.count() + 1 );
			id = yyLookedUpColumnMap.insert( columnName, aliasId );

			QMap<QString, int>::ConstIterator t =
				yyActiveTableMap.begin();
			while ( t != yyActiveTableMap.end() ) {
			    yyProg->append( new Push(*t) );
			    ++t;
			}
			yyProg->append( new MakeList(numTables) );
			yyProg->append( new LookupUnique(columnName, aliasId) );
		    }
		    *updateMe = *id;
		}
	    } else {
		id = yyActiveTableMap.find( tableName );
		if ( id == yyActiveTableMap.end() ) {
		    error( "Table '%s' cannot be used in this statement",
			   tableName.latin1() );
		    id = yyActiveTableMap.insert( tableName, 666 );
		    yyActiveTableIds.append( *id );
		}
		*updateMe = *id;
	    }
	} else {
	    ++v;
	    while ( v != expr->asList().end() ) {
		lookupNames( &*v );
		++v;
	    }
	}
    }
}

void Parser::emitExpr( const QVariant& expr, bool fieldValues, int trueLab,
		       int falseLab )
{
    /*
      An expression is an atom or a list. A list has the operator as
      first element and the operands afterwards. Cf. LISP.
    */
    if ( expr.type() == QVariant::List ) {
	QValueList<QVariant>::ConstIterator v = expr.listBegin();
	int tok = (*v).toInt();
	localsql::Op *op = 0;
	int tableId;
	QString field;
	int nextCond;

	switch ( tok ) {
	case Tok_Name:
	    tableId = (*++v).toInt();
	    field = (*++v).toString();
	    if ( fieldValues )
		yyProg->append( new PushFieldValue(tableId, field) );
	    else
		yyProg->append( new PushFieldDesc(tableId, field) );
	    break;
	case Tok_and:
	    nextCond = yyNextLabel--;
	    emitExpr( *++v, fieldValues, nextCond, falseLab );
	    yyProg->appendLabel( nextCond );
	    emitExpr( *++v, fieldValues, trueLab, falseLab );
	    break;
	case Tok_not:
	    emitExpr( *++v, fieldValues, falseLab, trueLab );
	    break;
	case Tok_or:
	    nextCond = yyNextLabel--;
	    emitExpr( *++v, fieldValues, trueLab, nextCond );
	    yyProg->appendLabel( nextCond );
	    emitExpr( *++v, fieldValues, trueLab, falseLab );
	    break;
	default:
	    emitExpr( *++v, fieldValues );
	    emitExpr( *++v, fieldValues );

	    switch ( tok ) {
	    case Tok_Equal:
		op = new Eq( trueLab, falseLab );
		break;
	    case Tok_NotEq:
		op = new Eq( falseLab, trueLab );
		break;
	    case Tok_LessThan:
		op = new Lt( trueLab, falseLab );
		break;
	    case Tok_GreaterEq:
		op = new Lt( falseLab, trueLab );
		break;
	    case Tok_Plus:
		op = new Add;
		break;
	    case Tok_Minus:
		op = new Subtract;
		break;
	    case Tok_Aster:
		op = new Multiply;
		break;
	    case Tok_Div:
		op = new Divide;
	    }
	    yyProg->append( op );
	}
    } else {
	yyProg->append( new Push(expr) );
    }
}

void Parser::emitCondition( const QVariant& cond,
			    const QValueList<QVariant>& constants,
			    const QValueList<QVariant>& columnsToSave,
			    int level )
{
    int tableId = yyActiveTableIds[level];
    int lastLevel = (int) yyActiveTableIds.count() - 1;
    bool saving = !columnsToSave.isEmpty();

#define NEED_A_LOOP() ( cond.isValid() || lastLevel > 0 )

    QValueList<QVariant> constantsForLevel;
    if ( yyActiveTableIds.count() == 1 ) {
	constantsForLevel = constants;
    } else {
	QValueList<QVariant>::ConstIterator c = constants.begin();
	while ( c != constants.end() ) {
	    if ( (*c).toList()[1] == tableId )
		constantsForLevel.append( *c );
	    ++c;
	}
    }

    if ( cond.isValid() && constantsForLevel.isEmpty() ) {
	if ( saving )
	    yyProg->append( new MarkAll(tableId) );    
    } else {
	emitConstants( constants );
	if ( saving && !NEED_A_LOOP() ) {
	    emitExprList( columnsToSave, FALSE );
	    yyProg->append( new MakeList(2) );
	    yyProg->append( new RangeSave(tableId, 0) );
	} else {
	    yyProg->append( new RangeMark(tableId) );
	}
    }

    int nextRecord = yyNextLabel--;
    int endRecords = yyNextLabel--;

    if ( NEED_A_LOOP() ) {
	if ( saving ) {
	    emitExprList( columnsToSave, FALSE );
	    yyProg->append( new CreateResult(0) );
	}

	yyProg->append( new RewindMarked(tableId) );
	yyProg->appendLabel( nextRecord );
	yyProg->append( new NextMarked(tableId, endRecords) );

	if ( level == lastLevel ) {
	    if ( saving ) {
		int saveRecord = yyNextLabel--;
		emitExpr( cond, TRUE, saveRecord, nextRecord );
		yyProg->appendLabel( saveRecord );
		emitExprList( columnsToSave, TRUE );
		yyProg->append( new SaveResult(0) );
	    } else {
		int unmarkRecord = yyNextLabel--;
		emitExpr( cond, TRUE, nextRecord, unmarkRecord );
		yyProg->appendLabel( unmarkRecord );
		yyProg->append( new Unmark(tableId) );
	    }
	} else {
	    emitCondition( cond, constants, columnsToSave, level + 1 );
	}

	yyProg->append( new Goto(nextRecord) );
	yyProg->appendLabel( endRecords );
	// yyProg->append( new Noop );
    }
}

void Parser::emitExprList( const QValueList<QVariant>& exprs, bool fieldValues )
{
    QValueList<QVariant>::ConstIterator e = exprs.begin();
    while ( e != exprs.end() ) {
	emitExpr( *e, fieldValues );
	++e;
    }
    yyProg->append( new MakeList(exprs.count()) );
}

void Parser::emitConstants( const QValueList<QVariant>& constants )
{
    QValueList<QVariant>::ConstIterator v = constants.begin();
    while ( v != constants.end() ) {
	QValueList<QVariant>::ConstIterator w = (*v).listBegin();
	QValueList<QVariant> nameExpr = (*++w).toList();
	QVariant valueExpr = *++w;

	emitExpr( nameExpr, FALSE );
	emitExpr( valueExpr, FALSE );
	yyProg->append( new MakeList(2) );
	++v;
    }
    yyProg->append( new MakeList(constants.count()) );
}

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

void Parser::deactivateTables()
{
    yyActiveTableMap.clear();
    yyLookedUpColumnMap.clear();
}

void Parser::closeAllTables()
{
    int n = (int) yyOpenedTableMap.count();
    for ( int i = 0; i < n; i++ )
	yyProg->append( new Close(i) );
    yyOpenedTableMap.clear();
}

void Parser::pourConstantsIntoCondition( QVariant *cond,
					 QValueList<QVariant> *constants )
{
    if ( constants == 0 )
	return;

    QValueList<QVariant>::ConstIterator c = constants->begin();
    while ( c != constants->end() ) {
	if ( cond->isValid() ) {
	    QValueList<QVariant> andExpr;
	    andExpr.append( (int) Tok_and );
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

QVariant Parser::matchColumnRef()
{
    QString tableName;
    QString columnName;
    int numDots = 0;

    while ( TRUE ) {
	columnName = matchName();
	if ( yyTok != Tok_Dot || numDots == 2 )
	    break;
	yyTok = getToken();
	numDots++;
	tableName = columnName;
    }

    QValueList<QVariant> expr;
    expr.append( (int) Tok_Name );
    expr.append( tableName );
    expr.append( columnName );
    return expr;
}

QVariant Parser::matchFunctionRefArguments()
{
    matchOrInsert( Tok_LeftParen, "'('" );
    if ( yyTok == Tok_Aster ) {
	yyTok = getToken();
    } else {
	matchScalarExpr();
    }
    matchOrInsert( Tok_RightParen, "')'" );
    return QVariant();
}

QVariant Parser::matchPrimaryExpr()
{
    QVariant right;
    bool uminus = FALSE;

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
    case Tok_avg:
	yyTok = getToken();
	matchFunctionRefArguments();
	// ###
	break;
    case Tok_count:
	yyTok = getToken();
	matchFunctionRefArguments();
	// ###
	break;
    case Tok_max:
	yyTok = getToken();
	matchFunctionRefArguments();
	// ###
	break;
    case Tok_min:
	yyTok = getToken();
	matchFunctionRefArguments();
	// ###
	break;
    case Tok_sum:
	yyTok = getToken();
	matchFunctionRefArguments();
	// ###
	break;
    default:
	error( "Met '%s' where primary expression was expected", yyLex );
    }

    if ( uminus ) {
	QValueList<QVariant> uminusExp;
	uminusExp.append( (int) Tok_Minus );
	uminusExp.append( 0 );
	uminusExp.append( right );
	return uminusExp;
    } else {
	return right;
    }
}

QVariant Parser::matchMultiplicativeExpr()
{
    QVariant left;

    left = matchPrimaryExpr();
    while ( yyTok == Tok_Aster || yyTok == Tok_Div ) {
	QValueList<QVariant> multExp;
	multExp.append( yyTok );
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
	scalExp.append( yyTok );
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

QVariant Parser::matchPredicate( QValueList<QVariant> *constants )
{
    QValueList<QVariant> pred;
    QValueList<QVariant> between;
    QVariant left;
    QVariant right;
    bool leftMayBeColumnRef = ( yyTok == Tok_Name ); // ### leftIs?

    left = matchScalarExpr();

    switch ( yyTok ) {
    case Tok_Equal:
	pred.append( yyTok );
	yyTok = getToken();
	right = matchScalarExpr();
	pred.append( left );
	pred.append( right );
	if ( constants != 0 && leftMayBeColumnRef &&
	     right.type() != QVariant::List ) {
	    constants->append( pred );
	    return QVariant();
	}
	break;
    case Tok_NotEq:
    case Tok_LessThan:
    case Tok_GreaterEq:
	pred.append( yyTok );
	yyTok = getToken();
	pred.append( left );
	pred.append( matchScalarExpr() );
	break;
    case Tok_GreaterThan:
	yyTok = getToken();
	pred.append( (int) Tok_LessThan );
	pred.append( matchScalarExpr() );
	pred.append( left );
	break;
    case Tok_LessEq:
	yyTok = getToken();
	pred.append( (int) Tok_GreaterEq );
	pred.append( matchScalarExpr() );
	pred.append( left );
	break;
    case Tok_between:
	between.append( (int) Tok_and );

	yyTok = getToken();
	pred.append( (int) Tok_GreaterEq );
	pred.append( left );
	pred.append( matchScalarExpr() );
	between.append( pred );
	matchOrInsert( Tok_and, "'and'" );

	pred.clear();
	pred.append( (int) Tok_LessEq );
	pred.append( left );
	pred.append( matchScalarExpr() );
	between.append( pred );
	pred = between;
	break;
    case Tok_in:
	yyTok = getToken();
	matchOrInsert( Tok_LeftParen, "'('" );
	matchAtomList();
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    case Tok_is:
	if ( !leftMayBeColumnRef )
	    error( "Expected column reference before 'is'" );

	yyTok = getToken();
	if ( yyTok == Tok_not ) {
	    yyTok = getToken();
	    matchOrSkip( Tok_null, "'null'" );
	} else {
	    matchOrSkip( Tok_null, "'null'" );
	}
	break;
    case Tok_like:
	yyTok = getToken();
	matchAtom();
	break;
    case Tok_not:
	yyTok = getToken();

	switch ( yyTok ) {
	case Tok_between:
	    yyTok = getToken();
	    matchScalarExpr();
	    matchOrInsert( Tok_and, "'and'" );
	    matchScalarExpr();
	    break;
	case Tok_in:
	    yyTok = getToken();
	    matchOrInsert( Tok_LeftParen, "'('" );
	    matchAtomList();
	    matchOrInsert( Tok_RightParen, "')'" );
	    break;
	case Tok_like:
	    yyTok = getToken();
	    matchAtom();
	    break;
	default:
	    error( "Expected 'between', 'in' or 'like' after 'not'" );
	}
    default:
	error( "Unexpected '%s' in predicate", yyLex );
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

	notExpr.append( yyTok );
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

	if ( left.isValid() && right.isValid() ) {
	    QValueList<QVariant> andCond;
	    andCond.append( (int) Tok_and );
	    andCond.append( left );
	    andCond.append( right );
	    left = andCond;
	} else {
	    left = right;
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
	    cond.append( yyTok );
	    yyTok = getToken();
	    cond.append( left );
	    cond.append( matchAndSearchCondition() );
	    left = cond;
	} while ( yyTok == Tok_or );
    }
    return left;
}

void Parser::matchOptWhereClause( const QValueList<QVariant>& columnsToSave )
{
    QVariant cond;
    QValueList<QVariant> constants;
    QValueList<QVariant> optimizableConstants;
    QValueList<QVariant> unoptimizableConstants;

    if ( yyTok == Tok_where ) {
	yyTok = getToken();
	cond = matchSearchCondition( &constants );

	int unoptimizableTableId = -1;
	if ( yyActiveTableIds.count() > 1 )
	    unoptimizableTableId = yyActiveTableIds.last();

	lookupNames( &cond );
	QValueList<QVariant>::Iterator c = constants.begin();
	while ( c != constants.end() ) {
	    lookupNames( &*c );
	    int tableId = (*c).asList()[1].asList()[1].toInt();
	    if ( tableId < 0 || tableId == unoptimizableTableId )
		unoptimizableConstants.append( *c );
	    else
		optimizableConstants.append( *c );
	    ++c;
	}
    }

    pourConstantsIntoCondition( &cond, &unoptimizableConstants );
    emitCondition( cond, optimizableConstants, columnsToSave );
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
    int len = 10;
    int prec = 0;

    switch ( yyTok ) {
    case Tok_character:
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
	error( "Met '%s' where 'character', 'date' or 'numeric' was expected",
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
    case Tok_unique:
	yyTok = getToken();
	break;
    case Tok_primary:
	yyTok = getToken();
	matchOrInsert( Tok_key, "'key'" );
	matchOrInsert( Tok_LeftParen, "'('" );
	matchColumnList();
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    case Tok_foreign:
	yyTok = getToken();
	matchOrInsert( Tok_key, "'key'" );
	matchOrInsert( Tok_LeftParen, "'('" );
	matchColumnList();
	matchOrInsert( Tok_RightParen, "')'" );
	matchOrInsert( Tok_references, "'references'" );
	matchTable();
	if ( yyTok == Tok_LeftParen ) {
	    yyTok = getToken();
	    matchColumnList();
	    matchOrInsert( Tok_RightParen, "')'" );
	}
	break;
    case Tok_check:
	yyTok = getToken();
	matchOrInsert( Tok_LeftParen, "'('" );
	matchSearchCondition();
	matchOrInsert( Tok_RightParen, "')'" );
	break;
    default:
	error( "Expected field name or constraint" );
    }
}

void Parser::matchBaseTableElement()
{
    if ( yyTok == Tok_Name ) {
	yyProg->append( new Push(yyLex) );
	yyTok = getToken();
	matchDataType();
	yyProg->append( new MakeList(4) );
    } else {
	matchTableConstraintDef();
    }
}

void Parser::matchBaseTableElementList()
{
    int n = 0;

    while ( TRUE ) {
	matchBaseTableElement();
	n++;

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
    yyProg->append( new MakeList(n) );
}

void Parser::matchCreateStatement()
{
    QString table;
    QStringList columns;
    QStringList::Iterator col;
    bool unique = FALSE;
    int tableId;

    yyTok = getToken();

    switch ( yyTok ) {
    case Tok_unique:
	yyTok = getToken();
	unique = TRUE;
	if ( yyTok != Tok_index )
	    error( "Expected 'index' after 'unique'" );
	// fall through
    case Tok_index:
	yyTok = getToken();
	if ( yyTok == Tok_Name ) {
	    warning( "Index name '%s' ignored", yyLex );
	    yyTok = getToken();
	}
	matchOrInsert( Tok_on, "'on'" );
	tableId = activateTable( matchTable() );

	matchOrInsert( Tok_LeftParen, "'('" );
	columns = matchColumnList();
	matchOrInsert( Tok_RightParen, "')'" );

	col = columns.begin();
	while ( col != columns.end() ) {
	    yyProg->append( new PushFieldDesc(tableId, *col) );
	    ++col;
	}
	yyProg->append( new MakeList(columns.count()) );
	yyProg->append( new CreateIndex(tableId, (int) unique) );
	break;
    case Tok_table:
	yyTok = getToken();
	table = matchTable();
	matchOrInsert( Tok_LeftParen, "'('" );
	matchBaseTableElementList();
	matchOrInsert( Tok_RightParen, "')'" );
	yyProg->append( new Create(table) );
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
    yyTok = getToken();
    matchOrInsert( Tok_from, "'from'" );
    matchTable();
    matchOptWhereClause();
}

void Parser::matchInsertExpr()
{
    if ( yyTok == Tok_null ) {
	yyTok = getToken();
	error( "Null not supported yet" );
    } else {
	QVariant expr = matchScalarExpr();
	lookupNames( &expr );
	emitExpr( expr, TRUE );
    }
}

void Parser::matchInsertExprList( const QStringList& columns )
{
    QStringList::ConstIterator col = columns.begin();
    int n = 0;

    while ( TRUE ) {
	if ( columns.isEmpty() ) {
	    yyProg->append( new Push(n) );
	} else {
	    if ( col == columns.end() ) {
		error( "Met more values than columns" );
	    } else {
		yyProg->append( new Push(*col) );
		++col;
	    }
	}
	matchInsertExpr();
	yyProg->append( new MakeList(2) );
	n++;

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
    if ( col != columns.end() )
	error( "Met fewer values than columns" );
    yyProg->append( new MakeList(n) );
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
	    yyActiveTableMap.insert( matchName(), tableId );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
}

void Parser::matchOrderByClause()
{
    yyTok = getToken();
    matchOrInsert( Tok_by, "'by'" );

    while ( TRUE ) {
	switch ( yyTok ) {
	case Tok_IntNum:
	    yyTok = getToken();
	    break;
	case Tok_Name:
	    matchColumnRef();

	    switch ( yyTok ) {
	    case Tok_asc:
		yyTok = getToken();
		break;
	    case Tok_desc:
		yyTok = getToken();
	    }
	    break;
	default:
	    error( "Met '%s' where a numeral or column reference was expected",
		   yyLex );
	}

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }
}

void Parser::matchSelectStatement()
{
    QValueList<QVariant> columnsToSave;

    yyTok = getToken();
    if ( yyTok == Tok_Aster ) {
	yyTok = getToken();
    } else {
	while ( TRUE ) {
	    columnsToSave.append( matchScalarExpr() );

	    if ( yyTok != Tok_Comma )
		break;
	    yyTok = getToken();
	}
    }

    matchFromClause();

    QValueList<QVariant>::Iterator c = columnsToSave.begin();
    while ( c != columnsToSave.end() ) {
	lookupNames( &*c );
	++c;
    }

    matchOptWhereClause( columnsToSave );
    if ( yyTok == Tok_order )
	matchOrderByClause();
}

void Parser::matchUpdateStatement()
{
    yyTok = getToken();
    int tableId = activateTable( matchTable() );
    matchOrInsert( Tok_set, "'set'" );

    QMap<QString, QVariant> assignments;
    QString left;
    QVariant right;

    while ( TRUE ) {
	left = matchName();
	matchOrInsert( Tok_Equal, "'='" );
	if ( yyTok == Tok_null ) {
	    yyTok = getToken();
	    // ###
	} else {
	    right = matchScalarExpr();
	    lookupNames( &right );
	}
	assignments.insert( left, right );

	if ( yyTok != Tok_Comma )
	    break;
	yyTok = getToken();
    }

    matchOptWhereClause();

    int nextMarkedRecord = yyNextLabel--;
    int endRecords = yyNextLabel--;

    yyProg->append( new RewindMarked(tableId) );
    yyProg->appendLabel( nextMarkedRecord );
    yyProg->append( new NextMarked(tableId, endRecords) );

    QMap<QString, QVariant>::ConstIterator as = assignments.begin();
    while ( as != assignments.end() ) {
	yyProg->append( new PushFieldDesc(tableId, as.key()) );
	emitExpr( *as, TRUE );
	yyProg->append( new MakeList(2) );
	++as;
    }
    yyProg->append( new MakeList(assignments.count()) );
    yyProg->append( new Update(tableId) );
    yyProg->append( new Goto(nextMarkedRecord) );
    yyProg->appendLabel( endRecords );
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
