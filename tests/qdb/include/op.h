/*
    Xbase project source code

    This file contains the XBase SQL virtual machine ops

    Copyright (C) 2000 Dave Berton (db@trolltech.com)
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

/* Base class for all ops.
*/
class Op : public localsql::Op
{
public:
    Op( const QVariant& P1 = QVariant(),
	 const QVariant& P2 = QVariant(),
	 const QVariant& P3 = QVariant() )
	: p1( P1 ), p2( P2 ), p3( P3 ), lab( 0 ) {}

    virtual ~Op();
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
    void error( localsql::Environment *env, const QString& msg )
    {
	env->setLastError( QString("internal error:%1: %2").arg(name())
			   .arg(msg) );
    }
    bool checkStack( localsql::Environment *env, int min )
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
    Noop() {}
    ~Noop() {}
    QString name() const { return "noop"; }
    int exec( localsql::Environment* )
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
    ~Push() {}
    QString name() const { return "push"; }
    int exec( localsql::Environment* env )
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


/* Pop the top two elements from the stack, add them together, and
   push the result (which is of type double) back onto the stack.
*/
class Add : public Op
{
public:
    Add() {}
    QString name() const { return "add"; }
    int exec( localsql::Environment* env )
    {
	if ( !checkStack(env, 2) )
	    return 0;
	QVariant v2 = env->stack()->pop();
	QVariant v1 = env->stack()->pop();
	env->stack()->push( v1.toDouble() + v2.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, subtract the first (top of
   stack) from the second (the next on stack) and push the result
   (which is of type double) back onto the stack.
*/
class Subtract : public Op
{
public:
    Subtract() {}
    QString name() const { return "subtract"; }
    int exec( localsql::Environment* env )
    {
	if ( !checkStack(env, 2) )
	    return 0;
	QVariant v2 = env->stack()->pop();
	QVariant v1 = env->stack()->pop();
	env->stack()->push( v1.toDouble() - v2.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, multiply them together,
 and push the result (which is of type double) back onto the stack.
*/
class Multiply : public Op
{
public:
    Multiply() {}
    QString name() const { return "multiply"; }
    int exec( localsql::Environment* env )
    {
	if ( !checkStack(env, 2) )
	    return 0;
	QVariant v2 = env->stack()->pop();
	QVariant v1 = env->stack()->pop();
	env->stack()->push( v1.toDouble() * v2.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, divide the first (top of
 the stack) from the second (next on stack) and push the result (which
 is of type double) back onto the stack.  Division by zero puts an
 invalid variant back on the stack, will issue a warning, and most
 likely cause another error down the line.
*/
class Divide : public Op
{
public:
    Divide() {}
    QString name() const { return "divide"; }
    int exec( localsql::Environment* env )
    {
	if ( !checkStack(env, 2) )
	    return 0;
	QVariant v2 = env->stack()->pop();
	QVariant v1 = env->stack()->pop();
	if ( v2.toDouble() == 0 ) {
	    error( env, "division by zero" );
	    env->stack()->push( QVariant() );
	} else
	    env->stack()->push( v1.toDouble() / v2.toDouble() );
	return 1;
    }
};

class CompOp : public Op
{
public:
    CompOp( int trueLab, int falseLab )
	: Op( trueLab, falseLab ) {}
    int exec( localsql::Environment* env )
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
class Eq : public CompOp
{
public:
    Eq( int trueLab, int falseLab )
	: CompOp( trueLab, falseLab ) {}
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
class Lt : public CompOp
{
public:
    Lt( int trueLab, int falseLab )
	: CompOp( trueLab, falseLab ) {}
    QString name() const { return "lt"; }
    bool pred( const QVariant& v1, const QVariant& v2 )
    {
	return v1.toDouble() < v2.toDouble();
    }
};

/* Pop the top two elements from the stack.  If second element (next
 on stack) is less than or equal to the first (top of stack), then
 jump to instruction P1. In other words, jump if NOS<=TOS.
*/
class Le : public CompOp
{
public:
    Le( int trueLab, int falseLab )
	: CompOp( trueLab, falseLab ) {}
    QString name() const { return "le"; }
    bool pred( const QVariant& v1, const QVariant& v2 )
    {
	return v1.toDouble() <= v2.toDouble();
    }
};

/* Pop the first 'num' values off the stack and push a list of them
 back on to the top of the stack.  The top of the stack becomes the
 last element of the list, and the 'num'-th element becomes the first
 element of the list.  For example, if the stack looks like this:

 5 (top of stack )
 dave
 trolltech
 99
 blarg

 The following instruction:

 MakeList ( 3 )

 will transform the stack into this:

 list: trolltech, dave, 5 (top of stack )
 99
 blarg

*/

class MakeList : public Op
{
public:
    MakeList( const QVariant& num )
	: Op( num ) {}
    QString name() const { return "makelist"; }
    int exec( localsql::Environment* env )
    {
	if ( !checkStack(env, p1.toInt()) )
	    return 0;
	localsql::List rec;
	for ( int i = 0; i < p1.toInt(); ++i )
	    rec.prepend( env->stack()->pop() );
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

*/

class Create : public Op
{
public:
    Create( const QVariant& name )
	: Op( name ) {}
    QString name() const { return "create"; }
    int exec( localsql::Environment* env )
    {
	localsql::List list = env->stack()->pop().toList();
	int id = env->addFileDriver( p1.toString() );
	localsql::FileDriver* drv = env->fileDriver( id );
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
    int exec( localsql::Environment* env )
    {
	env->addFileDriver( p1.toInt(), p2.toString() );
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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


/* Push field description information of the field identified by
  'nameOrNumber' in the file identified by 'id onto the top of the
  stack.  'nameOrNumber' can be the name of the field or the number of
  the field within the file.  The field must exist in the file.  The
  'field description' is a list of the following form:

  field name (string)
  field type (QVariant::Type)
  field length (int)
  field decimal precision (int)

*/

class PushFieldDesc : public Op
{
public:
    PushFieldDesc( const QVariant& id,
		   const QVariant& nameOrNumber )
	: Op( id, nameOrNumber ) {}
    QString name() const { return "pushfielddesc"; }
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
	QVariant v;
	if ( p2.type() == QVariant::String || p2.type() == QVariant::CString ) {
	    if ( !drv->fieldDescription( p2.toString(), v ) )
		return FALSE;
	} else {
	    if ( !drv->fieldDescription( p2.toInt(), v ) )
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
    int exec( localsql::Environment* env )
    {
	localsql::List list = env->stack()->pop().toList();
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

  Where 'field desc' is a list of the form:

  field name (string)
  field type (QVariant::Type)
  field length (int)
  field decimal precision (int)

  See also 'PushFieldDesc' which can provide the above information.

*/

class CreateResult : public Op
{
public:
    CreateResult( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "createresult"; }
    int exec( localsql::Environment* env )
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->rangeMark( env->stack()->pop().toList() );
    }
};

/* Similar to RangeMark, except the list is of the form:

   (rangemark list)
   (column list)

   Where 'column list' is a list of column names of the same form used
   by SaveResult.  This list identifies the columns to be used.

   The 'resultid' parameter indicates the result to save the data to.
*/

class RangeSave : public Op
{
public:
    RangeSave( int id, int resultid )
	: Op( id, resultid ) {}
    QString name() const { return "rangesave"; }
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
	localsql::List list = env->stack()->pop().toList();
	localsql::List range = list[0].toList();
	localsql::List columns = list[1].toList();
	localsql::ResultSet* result = env->resultSet( p2.toInt() );
	return drv->rangeSave( range, columns, result );
    }
};


/*  Pop the top of the stack (which must be a list, see MakeList) and
  use it as a description of fields, which must be of the form:

   field data
   field data
   field data
   ...

   Where each 'field data' is a value list of the form:

   field name (string)
   field type (QVariant::Type)
   field length (int)
   field decimal precision (int)

   Creates an index on the file identified by 'id'.  The file must be
   open (see Open).  See also PushFieldDesc.
*/

class CreateIndex : public Op
{
public:
    CreateIndex( const QVariant& id,
		 const QVariant& unique )
	: Op( id, unique ) {}
    QString name() const { return "createindex"; }
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->createIndex( env->stack()->pop().toList(), p2.toBool() );
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
    int exec( localsql::Environment* env )
    {
	int id = env->addFileDriver( p1.toString() );
	localsql::FileDriver* drv = env->fileDriver( id );
	return drv->drop();
    }
};

/* Pop the top of the stack (which must be a list, see MakeList) and
   use it as a list of fields with which to sort the 'result set' which
   is identified by 'id' (see CreateResult).

   The 'list' which is popped from the top of the stack must be of the form:

   field description
   descending (bool)
   field description
   descending (bool)

   Where 'field description' is a list of the form:

   field name (string)
   field type (QVariant::Type)
   field length (int)
   field decimal precision (int)

   and 'descending' is a bool indicating if the field should be sorted
   in descending order (otherwise, it is sorted in ascending order).
   The file must be open (see Open).  See also PushFieldDesc.

*/

class Sort : public Op
{
public:
    Sort( const QVariant& id )
	: Op( id ) {}
    QString name() const { return "sort"; }
    int exec( localsql::Environment* env )
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
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
    int exec( localsql::Environment* env )
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
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->markAll();
    }
};

/* UnMarks the current record of the file identified by 'id'.  The file
must be open and positioned on a valid record.
*/

class UnMark : public Op
{
public:
    UnMark( int id )
	: Op( id ) {}
    QString name() const { return "unmark"; }
    int exec( localsql::Environment* env )
    {
	localsql::FileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->unmark();
    }
};

#endif
