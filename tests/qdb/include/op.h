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

#ifndef OP_H
#define OP_H

#include "environment.h"
#include "sqlinterpreter.h"
#include <qvariant.h>
#include <math.h>

/* Base class for all ops.
*/
class Op : public LocalSQLOp
{
public:
    Op( const QVariant& P1 = QVariant(),
	const QVariant& P2 = QVariant(),
	const QVariant& P3 = QVariant() )
	: p1( P1 ), p2( P2 ), p3( P3 ), lab( 0 )
    {
    }
    ~Op();
    void setLabel( int L ) { lab = L; }
    int label() const { return lab; }
    QVariant& P( int i )
    {
	switch( i ) {
	case 0: return p1;
	case 1: return p2;
	default:
	case 2: return p3;
	}
    }

protected:
    void error( LocalSQLEnvironment *env, const QString& msg )
    {
	env->setLastError( QString("internal error:%1: %2").arg(name())
			   .arg(msg) );
    }
    bool checkStack( LocalSQLEnvironment *env, int min )
    {
	bool ok = ( (int) env->stack()->count() >= min );
	if ( !ok )
	    error( env, "not enough stack elements" );
	return ok;
    }

    QVariant p1;
    QVariant p2;
    QVariant p3;
    int lab;
};

/* No op.
*/

class Noop : public Op
{
public:
    Noop( const QString& comment = QString::null )
	: Op( comment ) {}
    QString name() const { return "noop"; }
    int exec( LocalSQLEnvironment* )
    {
	return 1;
    }
};

/* Pushes the variant P1 onto the stack.  If P2 or P3 is specified,
  they are also pushed onto the stack (in that order).  Example:

  Push( 99 )
  Push( 1, "dave", "trolltech" )
  Push( "oslo", "norway" )

  After these ops, the stack will look like this:

  norway (top of stack)
  oslo
  trolltech
  dave
  1
  99
*/

class Push : public Op
{
public:
    Push( const QVariant& P1,
	  const QVariant& P2 = QVariant(),
	  const QVariant& P3 = QVariant() )
	: Op( P1, P2, P3 )
    {
    }
    QString name() const { return "push"; }
    int exec( LocalSQLEnvironment* env )
    {
	if ( p1.isValid() )
	    env->stack()->push( p1 );
	if ( p2.isValid() )
	    env->stack()->push( p2 );
	if ( p3.isValid() )
	    env->stack()->push( p3 );
	return 1;
    }
};

/* Pushes a 'separator' value onto the stack.
*/
class PushSeparator : public Op
{
public:
    PushSeparator() {}
    QString name() const { return "( "; }
    int exec( LocalSQLEnvironment* env )
    {
	env->stack()->push( QVariant("---") );
	return 1;
    }
};

class BinOp : public Op
{
public:
    BinOp() {}
    int exec( LocalSQLEnvironment* env )
    {
	if ( !checkStack(env, 2) )
	    return 0;
	QVariant v2 = env->stack()->pop();
	QVariant v1 = env->stack()->pop();
	env->stack()->push( bin(v1, v2) );
	return 1;
    }

protected:
    virtual QVariant bin( const QVariant& v1, const QVariant& v2 ) = 0;
};

/* Pop the top two elements from the stack, add them together, and
   push the result (which is of type double) back onto the stack.
*/
class Add : public BinOp
{
public:
    Add() {}
    QString name() const { return "add"; }

protected:
    QVariant bin( const QVariant& v1, const QVariant& v2 )
    {
	return v1.toDouble() + v2.toDouble();
    }
};

/* Pop the top two elements from the stack, subtract the first (top of
   stack) from the second (the next on stack) and push the result
   (which is of type double) back onto the stack.
*/
class Subtract : public BinOp
{
public:
    Subtract() {}
    QString name() const { return "subtract"; }

protected:
    QVariant bin( const QVariant& v1, const QVariant& v2 )
    {
	return v1.toDouble() - v2.toDouble();
    }
};

/* Pop the top two elements from the stack, multiply them together,
 and push the result (which is of type double) back onto the stack.
*/
class Multiply : public BinOp
{
public:
    Multiply() {}
    QString name() const { return "multiply"; }

protected:
    QVariant bin( const QVariant& v1, const QVariant& v2 )
    {
	return v1.toDouble() * v2.toDouble();
    }
};

/* Pop the top two elements from the stack, divide the first (top of
 the stack) from the second (next on stack) and push the result (which
 is of type double) back onto the stack.  Division by zero puts an
 invalid variant back on the stack, will issue a warning, and most
 likely cause another error down the line.
*/
class Divide : public BinOp
{
public:
    Divide() {}
    QString name() const { return "divide"; }

protected:
    QVariant bin( const QVariant& v1, const QVariant& v2 )
    {
	if ( v2.toDouble() == 0 )
	    return QVariant();
	else
	    return v1.toDouble() / v2.toDouble();
    }
};

class Mod : public BinOp
{
public:
    Mod() {}
    QString name() const { return "mod"; }

protected:
    QVariant bin( const QVariant& v1, const QVariant& v2 )
    {
	if ( v2.toDouble() == 0 )
	    return QVariant();
	else
	    return fmod( v1.toDouble(), v2.toDouble() );
    }
};

class Power : public BinOp
{
public:
    Power() {}
    QString name() const { return "power"; }

protected:
    QVariant bin( const QVariant& v1, const QVariant& v2 )
    {
	return pow( v1.toDouble(), v2.toDouble() );
    }
};

class UnOp : public Op
{
public:
    UnOp() {}
    int exec( LocalSQLEnvironment* env )
    {
	if ( !checkStack(env, 1) )
	    return 0;
	QVariant v1 = env->stack()->pop();
	env->stack()->push( un(v1) );
	return 1;
    }

protected:
    virtual QVariant un( const QVariant& v1 ) = 0;
};

class Abs : public UnOp
{
public:
    Abs() {}
    QString name() const { return "abs"; }

private:
    QVariant un( const QVariant& v1 )
    {
	return fabs( v1.toDouble() );
    }
};

class Ceil : public UnOp
{
public:
    Ceil() {}
    QString name() const { return "ceil"; }

private:
    QVariant un( const QVariant& v1 )
    {
	return ceil( v1.toDouble() );
    }
};

class Floor : public UnOp
{
public:
    Floor() {}
    QString name() const { return "floor"; }

private:
    QVariant un( const QVariant& v1 )
    {
	return floor( v1.toDouble() );
    }
};

class Sign : public UnOp
{
public:
    Sign() {}
    QString name() const { return "sign"; }

private:
    QVariant un( const QVariant& v1 )
    {
	return v1.toDouble() == 0.0 ? 0 : v1.toDouble() < 0.0 ? -1 : 1;
    }
};

class PredOp : public Op
{
public:
    PredOp( int trueLab, int falseLab )
	: Op( trueLab, falseLab ) {}
    int exec( LocalSQLEnvironment* env )
    {
	if ( !checkStack(env, 2) )
	    return 0;
	QVariant v2 = env->stack()->pop();
	QVariant v1 = env->stack()->pop();
	env->program()->setCounter( pred(v1, v2) ? p1.toInt() : p2.toInt() );
	return 1;
    }

protected:
    virtual bool pred( const QVariant& v1, const QVariant& v2 ) = 0;
};

/* Pop the top two elements from the stack.  If they are equal, then
 jump to instruction P1.  Otherwise, continue to the next instruction.
*/
class Eq : public PredOp
{
public:
    Eq( int trueLab, int falseLab )
	: PredOp( trueLab, falseLab ) {}
    QString name() const { return "eq"; }
    bool pred( const QVariant& v1, const QVariant& v2 )
    {
	return v1 == v2;
    }
};

/* Pop the top two elements from the stack.  If second element (next
 on stack) is less than the first (top of stack), then jump to
 instruction P1.  Otherwise, continue to the next instruction.
*/
class Lt : public PredOp
{
public:
    Lt( int trueLab, int falseLab )
	: PredOp( trueLab, falseLab ) {}
    QString name() const { return "lt"; }
    bool pred( const QVariant& v1, const QVariant& v2 )
    {
	return v1.toDouble() < v2.toDouble();
    }
};

class In : public PredOp
{
public:
    In( int trueLab, int falseLab )
	: PredOp( trueLab, falseLab ) {}
    QString name() const { return "in"; }
    bool pred( const QVariant& v1, const QVariant& v2 )
    {
	List set = v2.toList();
	return set.find( v1 ) != set.end();
    }
};

class Like : public Op
{
public:
    Like( const QString& wildcard, int trueLab, int falseLab )
	: Op( QString::null, trueLab, falseLab )
    {
	QString regexp;

	for ( int i = 0; i < (int) wildcard.length(); i++ ) {
	    QChar ch = wildcard[i];
	    switch ( ch.unicode() ) {
	    case '_':
		regexp += QChar( '.' );
		break;
	    case '%':
		regexp += QString( ".*" );
		break;
	    case '$':
	    case '(':
	    case ')':
	    case '*':
	    case '+':
	    case '.':
	    case '?':
	    case '[':
	    case '\\':
	    case ']':
	    case '^':
	    case '{':
	    case '|':
	    case '}':
		regexp += QChar( '\\' );
		// fall through
	    default:
		regexp += ch;
	    }
	}
	p1 = regexp;
    }
    QString name() const { return "like"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	QRegExp regexp( p1.toString() );
	env->program()->setCounter( regexp.exactMatch(str) ? p2.toInt()
				    : p3.toInt() );
	return 1;
    }
};

class Upper : public Op
{
public:
    Upper() { }
    QString name() const { return "upper"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	env->stack()->push( str.upper() );
	return 1;
    }
};

class Lower : public Op
{
public:
    Lower() { }
    QString name() const { return "lower"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	env->stack()->push( str.lower() );
	return 1;
    }
};

class Length : public Op
{
public:
    Length() { }
    QString name() const { return "length"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	env->stack()->push( (int) str.length() );
	return 1;
    }
};

class Substring : public Op
{
public:
    Substring() { }
    QString name() const { return "substring"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	int start = env->stack()->pop().toInt();
	int len = env->stack()->pop().toInt();
	env->stack()->push( str.mid(start, len) );
	return 1;
    }
};

class Translate : public Op
{
public:
    Translate() { }
    QString name() const { return "translate"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	QString from = env->stack()->pop().toString();
	QString to = env->stack()->pop().toString();
	QString out;

	for ( int i = 0; i < (int) str.length(); i++ ) {
	    QChar ch = str[i];
	    int k = from.find( ch );
	    if ( k == -1 )
		out += ch;
	    else if ( k < (int) to.length() )
		out += to[k];
	}
	env->stack()->push( out );
	return 1;
    }
};

class Replace : public Op
{
public:
    Replace() { }
    QString name() const { return "replace"; }
    int exec( LocalSQLEnvironment* env )
    {
	QString str = env->stack()->pop().toString();
	QString before = env->stack()->pop().toString();
	QString after = env->stack()->pop().toString();

	if ( before.length() > 0 ) {
	    int k = 0;
	    while ( (k = str.find(before, k)) != -1 ) {
		str.replace( k, before.length(), after );
		k += after.length();
	    }
	}
	env->stack()->push( str );
	return 1;
    }
};

class Soundex : public Op
{
public:
    Soundex() { }
    QString name() const { return "soundex"; }
    int exec( LocalSQLEnvironment* env )
    {
	/*
	  This is not really Soundex, but some vendor seems to
	  implement it that way.
	*/
	QString str = env->stack()->pop().toString().lower();
	str.replace( QRegExp(QString("[^a-z]")), QString::null );
	QString x;

	if ( str.length() > 0 ) {
	    x = str[0].upper();
	    int i = 1;
	    while ( i < (int) str.length() && (int) x.length() < 4 ) {
		int code = 0;

		switch ( str[i].cell() ) {
		case 'a':
		case 'e':
		case 'h':
		case 'i':
		case 'o':
		case 'u':
		case 'w':
		case 'y':
		    break;
		case 'b':
		case 'f':
		case 'p':
		case 'v':
		    code = 1;
		    break;
		case 'c':
		case 'g':
		case 'j':
		case 'k':
		case 'q':
		case 's':
		case 'x':
		case 'z':
		    code = 2;
		    break;
		case 'd':
		case 't':
		    code = 3;
		    break;
		case 'l':
		    code = 4;
		    break;
		case 'm':
		case 'n':
		    code = 5;
		    break;
		case 'r':
		    code = 6;
		}
		if ( code != 0 && code != x[x.length() - 1].cell() - '0' )
		    x += QChar( code + '0' );
		i++;
	    }
	    while ( x.length() < 4 )
		x += QChar( '0' );
	}
	env->stack()->push( x );
	return 1;
    }
};

/* Pop values off the stack until a 'separator' is found and push a
 list of them back on to the top of the stack.  The top of the stack
 becomes the last element of the list, and the 'num'-th element
 becomes the first element of the list.  For example, if the stack
 looks like this:

 5 (top of stack )
 dave
 trolltech
 --separator-- (see PushSeparator)
 99
 blarg

 The following instruction:

 MakeList

 will transform the stack into this:

 list: trolltech, dave, 5 (top of stack )
 99
 blarg

*/

class MakeList : public Op
{
public:
    MakeList()
	: Op() {}
    QString name() const { return ")"; }
    int exec( LocalSQLEnvironment* env )
    {
	if ( !checkStack( env, 1 ) )
	    return 0;
	List rec;
	for ( ; ; ) {
	    if ( env->stack()->count() == 0 )
		return 0;
	    QVariant v = env->stack()->pop();
	    if ( v.toString() == "---" ) /* separator */
		break;
	    rec.prepend( v );
	}
	env->stack()->push( rec );
	return 1;
    }
};

/* Pop top of stack (which should be a 'list', see 'MakeList') and
 create a file with name 'name'.  The list should be of the form:

 field description
 field description
 field description
 ...

 where each 'field description' is a value list of variants in the
 following order:

 field name (string)
 field type (QVariant::Type)
 field length (int)
 field decimal precision (int)
 not null (bool)

*/

class Create : public Op
{
public:
    Create( const QVariant& name )
	: Op( name ) {}
    QString name() const { return "create"; }
    int exec( LocalSQLEnvironment* env )
    {
	List list = env->stack()->pop().toList();
	int id = env->addFileDriver( p1.toString() );
	LocalSQLFileDriver* drv = env->fileDriver( id );
	return drv->create( list );
    }
};

/* Opens the file 'name'.  The file will be identified by 'id' which
can be used later to refer to the file.
*/

class Open : public Op
{
public:
    Open( const QVariant& id, const QVariant& name )
	: Op( id, name ) {}
    QString name() const { return "open"; }
    int exec( LocalSQLEnvironment* env )
    {
	env->addFileDriver( p1.toInt(), p2.toString() );
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->open();
    }
};

/* Closes the file specified by 'id'.
*/

class Close : public Op
{
public:
    Close( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "close"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->close();
    }
};


/* Pop the top of the stack (which must be a 'list', see MakeList) and
   insert it into the file identified by 'id'.  The values of the list
   must be of the following form:

  field data
  field data
  field data
  ...

  Where each 'field data' is a value list of the form:

  nameOrNumber (variant)
  data (variant)

  Where 'nameOrNumber' is the name of the field or the number of the
  field within the file.  The file must be open (see Open).

*/
class Insert : public Op
{
public:
    Insert( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "insert"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->insert( env->stack()->pop().toList() );
    }
};

/* Marks the current record of the file identified by 'id'.  The file
must be open and positioned on a valid record.
*/

class Mark : public Op
{
public:
    Mark( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "mark"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->mark();
    }
};

/*  Deletes all record from the file which is identified by 'id' which
have been previously marked by Mark. All marks are then cleared.
*/

class DeleteMarked : public Op
{
public:
    DeleteMarked( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "deletemarked"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->deleteMarked();
    }
};

/*  Pops the top of the stack (which must be a 'list', see MakeList)
  and updates all record from the file identified by 'id' which have
  been previously marked by Mark.  The marked records are updated only
  with the fields specified by the list.  The file must be open (see Open). The
  list must be of the following form:

  field data
  field data
  field data
...

  Where each 'field data' is a value list of the form:

  name (string)
  data (variant)

  All marks are then cleared.
*/

class UpdateMarked : public Op
{
public:
    UpdateMarked( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "updatemarked"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	bool b = drv->updateMarked( env->stack()->pop().toList() );
	return b;
    }
};


/* Go to next record of the file which is identified by 'id'.  On
 failure goto P2.  The file must be open (see Open).
*/

class Next : public Op
{
public:
    Next( const QVariant& id,
	  const QVariant& P2 )
	: Op( id, P2 ) {}
    QString name() const { return "next"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->next() )
	    env->program()->setCounter( p2.toInt() );
	return TRUE;
    }
};

/* Go to the instruction at P1.
*/

class Goto : public Op
{
public:
    Goto( int lab )
	: Op( lab ) {}
    QString name() const { return "goto"; }
    int exec( LocalSQLEnvironment* env )
    {
	env->program()->setCounter( p1.toInt() );
	return TRUE;
    }
};

/* Push the value of field number P2 from the file identified by 'id'
 on to the top of the stack.  The file must be open (see Open) and positioned on
 a valid record.
*/

class PushFieldValue : public Op
{
public:
    PushFieldValue( const QVariant& id, const QVariant& P2 )
	: Op( id, P2 ) {}
    QString name() const { return "pushfieldvalue"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	QVariant v;
	if ( p2.type() == QVariant::String || p2.type() == QVariant::CString ) {
	    if ( !drv->field( p2.toString(), v ) )
		return FALSE;
	} else {
	    if ( !drv->field( p2.toInt(), v ) )
		return FALSE;
	}
	env->stack()->push( v );
	return TRUE;
    }
};


/* Push the field type of the field identified by 'nameOrNumber' in
  the file identified by 'id onto the top of the stack.
  'nameOrNumber' can be the name of the field or the number of the
  field within the file.  The field must exist in the file.

*/

class PushFieldType : public Op
{
public:
    PushFieldType( const QVariant& id, const QVariant& nameOrNumber )
	: Op( id, nameOrNumber ) {}
    QString name() const { return "pushfieldtype"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	QVariant v;
	if ( p2.type() == QVariant::String || p2.type() == QVariant::CString ) {
	    if ( !drv->fieldType( p2.toString(), v ) )
		return FALSE;
	} else {
	    if ( !drv->fieldType( p2.toInt(), v ) )
		return FALSE;
	}
	env->stack()->push( v );
	return TRUE;
    }
};

/* Pops the top of the stack (which must be a 'list', see MakeList)
  and appends it to the internal result set (see CreateResult).  The
  list must be of the form:

  data (variant)
  data (variant)
  ...etc

  The number of elements in the list must match the number of elements
  in the result set.  The type of each element must also match the
  corresponding type in the result set.  It is an error to attempt to
  save an empty list.

*/

class SaveResult : public Op
{
public:
    SaveResult( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "saveresult"; }
    int exec( LocalSQLEnvironment* env )
    {
	List list = env->stack()->pop().toList();
	qDebug("SaveResult, list count:" + QString::number(list.count()));
	return env->resultSet( p1.toInt() )->append( list );
    }
};

/*  Pops the top of the stack (which must be a 'list', see MakeList)
  and creates a 'result set'.  The result set will be identified by
  'id'. The result set is an internal memory area which can be added
  to (see SaveResult) and later retrieved (see Environment::result()).
  The 'result set' forms the fundamental mechanism for selecting data
  from a file.  The list must be of the form:

  field desc
  field desc
  ...etc

  Where each 'field desc' is itself a list of the form:

  field name (string)
  field type (QVariant::Type)

*/

class CreateResult : public Op
{
public:
    CreateResult( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "createresult"; }
    int exec( LocalSQLEnvironment* env )
    {
	env->addResultSet( p1.toInt() );
	return env->resultSet( p1.toInt() )->setHeader( env->stack()->pop().toList() );
    }
};

/* Resets the internal marked-record iterator for the file which is
  identified by 'id' to the beginning (see Mark).  Marked records can
  then be sequentially retrieved using NextMarked.  The file does not
  need to be open.
*/

class RewindMarked : public Op
{
public:
    RewindMarked( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "rewindmarked"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->rewindMarked();
    }
};

/* Go to next marked record of the file identified by 'id'.  On
 failure goto P2.  The file must be open (see Open).
*/

class NextMarked : public Op
{
public:
    NextMarked( const QVariant& id,
		const QVariant& P2 )
	: Op( id, P2 ) {}
    QString name() const { return "nextmarked"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	bool b = drv->nextMarked();
	if ( !b )
	    env->program()->setCounter( p2.toInt() );
	return TRUE;
  }
};


/* Pop the top of the stack (which must be a 'list', see MakeList) and
   use it to update all fields of the current record buffer of the
   file identified by 'id'.  The list must be in the following form:

  field data
  field data
  field data
  ...

  Where each 'field data' is a value list of the form:

  nameOrNumber
  data

  Where 'nameOrNumber' is the name of the field or the number of the
  field within the file.  The file must be open (see Open).

*/

class Update : public Op
{
public:
    Update( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "update"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	bool b = drv->update( env->stack()->pop().toList() );
	return b;
    }
};

/* Pop the top of the stack (which must be a 'list', see MakeList) and
   use it to 'range mark' the record buffer of the file identified by
   'id'.  A 'range mark' tries to match every record in the file where
   the fields corresponding to the 'list' fields match the values of
   the corresponding list field values.  This can be optimised by
   drivers who use indexes, and therefore speed up the common cases
   such as:

   select field from table where id = 1;
   update table set field="blah" where id = 1;
   delete from table where id = 1;

   In the above examples, 'table' can be range markned based on the
   'id' field.  If the driver uses an index on the 'id' field of
   'table, it can optimize the search.

   All records that match the 'range mark' will be 'marked' (see
   RangeMark).  The file must be open (see Open).

   The 'list' which is popped from the top of the stack must be of the form:

   field data
   field data
   field data
   ...

   Where each 'field data' is a value list of the form:

   nameOrNumber
   data

*/
class RangeMark : public Op
{
public:
    RangeMark( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "rangemark"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->rangeMark( env->stack()->pop().toList() );
    }
};

/* Similar to RangeMark, except the list is of the form:

   (rangemark list)
   (column list)

   Where 'column list' is a list of column names of the same form used
   by CreateResult, and 'rangemark list' is a list of the same form
   used by RangeMark.

   The 'resultid' parameter indicates the new result where the data
   will be saved..
*/

class RangeSave : public Op
{
public:
    RangeSave( int id, int resultid )
	: Op( id, resultid ) {}
    QString name() const { return "rangesave"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	List list = env->stack()->pop().toList();
	List range = list[0].toList();
	List columns = list[1].toList();
	LocalSQLResultSet* result = env->resultSet( p2.toInt() );
	return drv->rangeSave( range, columns, result );
    }
};

/*  Pop the top of the stack (which must be a list, see MakeList) and
  use it as a list of field names, e.g.:

   field name
   field name
   field name
   ...

   Creates an index on the file identified by 'id'.  The file must be
   open (see Open).
*/

class CreateIndex : public Op
{
public:
    CreateIndex( const QVariant& id,
		 bool unique,
		 bool notnull )
	: Op( id, QVariant( unique, 1 ), QVariant( notnull, 1 ) ) {}
    QString name() const { return "createindex"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->createIndex( env->stack()->pop().toList(), p2.toBool(), p3.toBool() );
    }
};

/*  Drops (deletes) the file 'name'.  If the file contains
    any indexes, they are also dropped (deleted).  If the file is
    open, it is first closed.
*/

class Drop : public Op
{
public:
    Drop( const QVariant& name )
	: Op( name ) {}
    QString name() const { return "drop"; }
    int exec( LocalSQLEnvironment* env )
    {
	int id = env->addFileDriver( p1.toString() );
	LocalSQLFileDriver* drv = env->fileDriver( id );
	return drv->drop();
    }
};


/* Pop the top of the stack (which must be a list, see MakeList) and
   use it as a list of fields with which to sort the 'result set' which
   is identified by 'id' (see CreateResult).

   The 'list' which is popped from the top of the stack must be of the form:

   sort description
   sort description
   ...

   Where each 'sort description' is itself a list of the form:

   field number(uint)
   descending (bool)

   Where 'field number' is the number of the field in the result and
   'descending' is a bool indicating if the field should be sorted in
   descending order (otherwise, it is sorted in ascending order).  The
   file must be open (see Open).  See also PushFieldDesc.

*/

class Sort : public Op
{
public:
    Sort( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "sort"; }
    int exec( LocalSQLEnvironment* env )
    {
	return env->resultSet( p1.toInt() )->sort( env->stack()->pop().toList() );
    }
};


/*  Clears all marked records from the file which is identified by
'id'.
*/

class ClearMarked : public Op
{
public:
    ClearMarked( const QVariant& id, const QVariant& name )
	: Op( id, name ) {}
    QString name() const { return "clearmarked"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->clearMarked();
    }
};

/* Pops the top of the stack (which must be a list, see 'PushList')
and uses it as a list of file ids.  An attempt is made to find
'fieldname' from that list of files.  If it succeeds, 'alias' (which
must be a negative number) can be used as an alias for the field's
table.
*/

class LookupUnique : public Op
{
public:
    LookupUnique( const QString& fieldname, int alias )
	: Op( fieldname, alias ) {}
    QString name() const { return "lookupunique"; }
    int exec( LocalSQLEnvironment* env )
    {
	return env->addFileDriverAlias( env->stack()->pop().toList(), p1.toString(), p2.toInt() );
    }
};


/*  Marks all records records in the file which is identified by 'id'.
*/

class MarkAll : public Op
{
public:
    MarkAll( int id )
	: Op( id ) {}
    QString name() const { return "markall"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->markAll();
    }
};

/* Unmarks the current record of the file identified by 'id'.  The file
must be open and positioned on a valid record.
*/

class Unmark : public Op
{
public:
    Unmark( int id )
	: Op( id ) {}
    QString name() const { return "unmark"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->unmark();
    }
};

/* Push the values of all fields from the file identified by 'id'
 on to the top of the stack.  The file must be open (see Open) and positioned on
 a valid record.
*/

class PushStarValue : public Op
{
public:
    PushStarValue( int id )
	: Op( id ) {}
    QString name() const { return "pushstarvalue"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	QVariant v;
	if ( !drv->star( v ) )
	    return FALSE;
	List list = v.toList();
	for ( uint i = 0; i < list.count(); ++i )
	    env->stack()->push( list[i] );
	return TRUE;
    }
};

/* Push the description of all fields from the file identified by 'id'
 on to the top of the stack.  See also 'PushFieldDesc'.
*/

class PushStarDesc : public Op
{
public:
    PushStarDesc( int id )
	: Op( id ) {}
    QString name() const { return "pushstardesc"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLFileDriver* drv = env->fileDriver( p1.toInt() );
	QVariant v;
	if ( !drv->starDescription( v ) )
	    return FALSE;
	List list = v.toList();
	for ( uint i = 0; i < list.count(); ++i )
	    env->stack()->push( list[i] );
	return TRUE;
    }
};


/* Pop the top of the stack (which must be a list, see MakeList) and
   use it as a list of fields with which to group the result set
   identified by 'resultid' which is identified by 'id' (see
   CreateResult).

   The 'list' which is popped from the top of the stack must be of the form:

   field number in the result (int)
   field number in the result (int)
   field number in the result (int)
   ...

   Note that the result will be sorted as a result of this op.

*/

class MakeGroupSet : public Op
{
public:
    MakeGroupSet( int resultid )
	: Op( resultid ) {}
    QString name() const { return "makegroupset"; }
    int exec( LocalSQLEnvironment* env )
    {
	return env->resultSet( p1.toInt() )->setGroupSet( env->stack()->pop().toList() );
    }
};


/* Go to next group of the result which is identified by 'id'.  On
   failure goto P2.
*/

class NextGroupSet : public Op
{
public:
    NextGroupSet( const QVariant& id,
	  const QVariant& P2 )
	: Op( id, P2 ) {}
    QString name() const { return "nextgroupset"; }
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLResultSet* res = env->resultSet( p1.toInt() );
	if ( !res->nextGroupSet() )
	    env->program()->setCounter( p2.toInt() );
	return TRUE;
    }
};

class PushGroupOp : public Op
{
public:
    PushGroupOp( int id, uint P2, LocalSQLResultSet::GroupSetAction action )
	: Op( id, P2 ), act( action ) {}
    int exec( LocalSQLEnvironment* env )
    {
	LocalSQLResultSet* res = env->resultSet( p1.toInt() );
	QVariant v;
	if ( !res->groupSetAction(act, p2.toUInt(), v) ) {
	    qDebug("PushGroupOp failed---------------------");
	    return FALSE;
	}
	qDebug("PushGroupOp pushing value, stack count:" + QString::number( env->stack()->count()));
	env->stack()->push( v );
	qDebug("PushGroupOp pushing value, stack count:" + QString::number( env->stack()->count()));
	return TRUE;
    }

private:
    LocalSQLResultSet::GroupSetAction act;
};

/* Push the value of field number P2 from the current group of the
   result set identified by 'id' on to the top of the stack.  The result set
   must be positioned on a valid group (see NextGroup).
*/

class PushGroupValue : public PushGroupOp
{
public:
    PushGroupValue( int id, uint P2 )
	: PushGroupOp( id, P2, LocalSQLResultSet::Value ) {}
    QString name() const { return "pushgroupvalue"; }
};

/* Push the count of field P2 from the current group of the
   result set identified by 'id' on to the top of the stack.  The result set
   must be positioned on a valid group (see NextGroup).
*/

class PushGroupCount : public PushGroupOp
{
public:
    PushGroupCount( int id, uint P2 )
	: PushGroupOp( id, P2, LocalSQLResultSet::Count ) {}
    QString name() const { return "pushgroupcount"; }
};

/* Push the sum of field P2 from the current group of the
   result set identified by 'id' on to the top of the stack.  The result set
   must be positioned on a valid group (see NextGroup).
*/

class PushGroupSum : public PushGroupOp
{
public:
    PushGroupSum( int id, uint P2 )
	: PushGroupOp( id, P2, LocalSQLResultSet::Sum ) {}
    QString name() const { return "pushgroupsum"; }
};

/* Push the avg of field P2 from the current group of the
   result set identified by 'id' on to the top of the stack.  The result set
   must be positioned on a valid group (see NextGroup).
*/

class PushGroupAvg : public PushGroupOp
{
public:
    PushGroupAvg( int id, uint P2 )
	: PushGroupOp( id, P2, LocalSQLResultSet::Avg ) {}
    QString name() const { return "pushgroupavg"; }
};

/* Push the max of field P2 from the current group of the
   result set identified by 'id' on to the top of the stack.  The result set
   must be positioned on a valid group (see NextGroup).
*/

class PushGroupMax : public PushGroupOp
{
public:
    PushGroupMax( int id, uint P2 )
	: PushGroupOp( id, P2, LocalSQLResultSet::Max ) {}
    QString name() const { return "pushgroupmax"; }
};

/* Push the min of field P2 from the current group of the
   result set identified by 'id' on to the top of the stack.  The result set
   must be positioned on a valid group (see NextGroup).
*/

class PushGroupMin : public PushGroupOp
{
public:
    PushGroupMin( int id, uint P2 )
	: PushGroupOp( id, P2, LocalSQLResultSet::Min ) {}
    QString name() const { return "pushgroupmin"; }
};

#endif
