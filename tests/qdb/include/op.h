#ifndef OP_H
#define OP_H

#include "environment.h"
#include "sqlinterpreter.h"
#include <qvariant.h>
#include <qlist.h>

/* Base class for all ops.
*/
class Label : public qdb::Op
{
public:
    Label( const QString& label = QString::null )
	: lbl( label ) {}
    Label( const QVariant& P1,
	   const QString& label = QString::null )
	: p1( P1 ), lbl( label ) {}
    Label( const QVariant& P1,
	   const QVariant& P2,
	   const QString& label = QString::null )
	: p1( P1 ), p2( P2 ), lbl( label ) {}
    Label( const QVariant& P1,
	   const QVariant& P2,
	   const QVariant& P3,
	   const QString& label = QString::null )
	: p1( P1 ), p2( P2 ), p3(P3), lbl( label ) {}

    virtual ~Label();
    QVariant& P( int i )
    {
	switch( i ) {
	case 0: return p1;
	case 1: return p2;
	default:
	case 2: return p3;
	}
    }
    QString label() const { return lbl; }
protected:
    QVariant p1;
    QVariant p2;
    QVariant p3;

private:
    QString lbl;

};

/* No op.
*/

class Noop : public Label
{
public:
    Noop( const QString& label = QString::null )
	: Label( label )
    {
    }
    ~Noop() {}
    QString name() const { return "noop"; }
    int exec( qdb::Environment* )
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

class Push : public Label
{
public:
    Push( const QVariant& P1,
	  const QVariant& P2 = QVariant(),
	  const QVariant& P3 = QVariant(),
	  const QString& label = QString::null )
	: Label( P1, P2, P3, label )
    {
    }
    ~Push() {}
    QString name() const { return "push"; }
    int exec( qdb::Environment* env )
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
class Add : public Label
{
public:
    Add( const QString& label = QString::null )
	: Label( label ) {}
    QString name() const { return "add"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:add: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	env->stack()->push( v1.toDouble() + v2.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, subtract the first (top of
   stack) from the second (the next on stack) and push the result
   (which is of type double) back onto the stack.
*/
class Subtract : public Label
{
public:
    Subtract( const QString& label = QString::null )
	: Label( label ) {}
    QString name() const { return "subtract"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:subtract: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	env->stack()->push( v2.toDouble() - v1.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, multiply them together,
 and push the result (which is of type double) back onto the stack.
*/
class Multiply : public Label
{
public:
    Multiply( const QString& label = QString::null )
	: Label( label ) {}
    QString name() const { return "multiply"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:multiply: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	env->stack()->push( v2.toDouble() * v1.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, divide the first (top of
 the stack) from the second (next on stack) and push the result (which
 is of type double) back onto the stack.  Division by zero puts an
 invalid variant back on the stack, will issue a warning, and most
 likely cause another error down the line.
*/
class Divide : public Label
{
public:
    Divide( const QString& label = QString::null )
	: Label( label ) {}
    QString name() const { return "divide"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:divide: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v1.toInt() == 0 ) {
	    env->output() << "divide: division by zero" << endl;
	    env->stack()->push( QVariant() );
	} else
	    env->stack()->push( v2.toDouble() / v1.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack.  If they are equal, then
 jump to instruction P1.  Otherwise, continue to the next instruction.
*/
class Eq : public Label
{
public:
    Eq( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "eq"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:eq: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v1 == v2 ) {
	    if ( p1.type() == QVariant::String )
		env->program()->setCounter( p1.toString() );
	    else
		env->program()->setCounter( p1.toInt() );
	}
	return 1;
    }
};

/* Pop the top two elements from the stack.  If they are not equal,
 then jump to instruction P1.  Otherwise, continue to the next
 instruction.
*/
class Ne : public Label
{
public:
    Ne( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "ne"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:ne: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v1 != v2 ) {
	    if ( p1.type() == QVariant::String )
		env->program()->setCounter( p1.toString() );
	    else
		env->program()->setCounter( p1.toInt() );
	}
	return 1;
    }
};

/* Pop the top two elements from the stack.  If second element (next
 on stack) is less than the first (top of stack), then jump to
 instruction P1.  Otherwise, continue to the next instruction.  In
 other words, jump if NOS<TOS.
*/
class Lt : public Label
{
public:
    Lt( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "lt"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:lt: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v2.toDouble() < v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program()->setCounter( p1.toString() );
	    else
		env->program()->setCounter( p1.toInt() );
	}
	return 1;
    }
};

/* Pop the top two elements from the stack.  If second element (next
 on stack) is less than or equal to the first (top of stack), then
 jump to instruction P1. In other words, jump if NOS<=TOS.
*/
class Le : public Label
{
public:
    Le( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "le"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:le: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v2.toDouble() <= v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program()->setCounter( p1.toString() );
	    else
		env->program()->setCounter( p1.toInt() );
	}
	return 1;
    }
};


/* Pop the top two elements from the stack.  If second element (next
 on stack) is greater than the first (top of stack), then jump to
 instruction P1. In other words, jump if NOS>TOS.
*/
class Gt : public Label
{
public:
    Gt( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "gt"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:gt: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v2.toDouble() > v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program()->setCounter( p1.toString() );
	    else
		env->program()->setCounter( p1.toInt() );
	}
	return 1;
    }
};

/* Pop the top two elements from the stack.  If second element (next
 on stack) is greater than or equal to the first (top of stack),
 then jump to instruction P1. In other words, jump if NOS>=TOS.
*/
class Ge : public Label
{
public:
    Ge( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "ge"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("internal error:ge: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v2.toDouble() >= v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program()->setCounter( p1.toString() );
	    else
		env->program()->setCounter( p1.toInt() );
	}
	return 1;
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

 PushList ( 3 )

 will transform the stack into this:

 list: trolltech, dave, 5 (top of stack )
 99
 blarg

*/

class PushList : public Label
{
public:
    PushList( const QVariant& num,
		const QString& label = QString::null )
	: Label( num, label ) {}
    QString name() const { return "pushlist"; }
    int exec( qdb::Environment* env )
    {
	if ( (int)env->stack()->count() < p1.toInt() ) {
	    env->setLastError("internal error:pushlist: not enough stack elements");
	    return 0;
	}
	qdb::List rec;
	for ( int i = 0; i < p1.toInt(); ++i )
	    rec.prepend( env->stack()->pop() );
	env->stack()->push( rec );
	return 1;
    }
};

/* Pop top of stack (which should be a 'list', see 'PushList') and
 create a file with name 'name'.  The list should be of the form:

 field description
 field description
 field description
 etc...

 where each 'field description' is a value list of variants in the
 following order:

 field name (string)
 field type (QVariant::Type)
 field length (int)
 field decimal precision (int)

*/

class Create : public Label
{
public:
    Create( const QVariant& name,
	    const QString& label = QString::null )
	: Label( name, label ) {}
    QString name() const { return "create"; }
    int exec( qdb::Environment* env )
    {
	qdb::List list = env->stack()->pop().toList();
	if ( !list.count() ) {
	    env->setLastError("internal error:create: no fields defined");
	    return 0;
	}
	env->addFileDriver( 0, p1.toString() );
	qdb::FileDriver* drv = env->fileDriver( 0 );
	return drv->create( list );
    }
};

/* Opens the file 'name'.  The file will be identified by 'id' which
can be used later to refer to the file.
*/

class Open : public Label
{
public:
    Open( const QVariant& id, const QVariant& name,
	      const QString& label = QString::null )
	: Label( id, name, label ) {}
    QString name() const { return "ppen"; }
    int exec( qdb::Environment* env )
    {
	env->addFileDriver( p1.toInt(), p2.toString() );
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->open() ) {
	    env->setLastError("internal error:open: unable to open file:" + p2.toString() );
	    return 0;
	}
	return 1;
    }
};

/* Closes the file specified by 'id'.
*/

class Close : public Label
{
public:
    Close( const QVariant& id,
	   const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "close"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:close: file not open" );
	    return 0;
	}
	return drv->close();
    }
};


/* Pop the top of the stack (which must be a 'list', see PushList) and
   insert it into the file identified by 'id'.  The values of the list
   must be of the following form:

  field data
  field data
  field data
  ...

  Where each 'field data' is a value list of the form:

  name (string)
  data (variant)

  The file must be open (see Open).

*/
class Insert : public Label
{
public:
    Insert( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "insert"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:insert: file not open" );
	    return 0;
	}
	return drv->insert( env->stack()->pop().toList() );
    }
};

/* Marks the current record of the file identified by 'id'.  The file
must be open and positioned on a valid record.
*/

class Mark : public Label
{
public:
    Mark( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "mark"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:mark: file not open" );
	    return 0;
	}
	return drv->mark();
    }
};

/*  Deletes all record from the file which is identified by 'id' which
have been previously marked by Mark. All marks are then cleared.
*/

class DeleteMarked : public Label
{
public:
    DeleteMarked( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "deletemarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:deletemarked: file not open" );
	    return 0;
	}
	return drv->deleteMarked();
    }
};

/*  Pops the top of the stack (which must be a 'list', see PushList)
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

class UpdateMarked : public Label
{
public:
    UpdateMarked( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "updatemarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:updatemarked: file not open" );
	    return 0;
	}
	bool b = drv->updateMarked( env->stack()->pop().toList() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};


/* Go to next record of the file which is identified by 'id'.  On
 failure goto P2.  The file must be open (see Open).
*/

class Next : public Label
{
public:
    Next( const QVariant& id,
	  const QVariant& P2,
	  const QString& label = QString::null )
	: Label( id, P2, label ) {}
    QString name() const { return "next"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:next: file not open" );
	    return 0;
	}
	if ( !drv->next() ) {
	    if ( p2.type() == QVariant::String )
		env->program()->setCounter( p2.toString() );
	    else
		env->program()->setCounter( p2.toInt() );
	}
	return TRUE;
    }
};

/* Go to the instruction at P1.
*/

class Goto : public Label
{
public:
    Goto( const QVariant& P1,
	  const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "goto"; }
    int exec( qdb::Environment* env )
    {
	if ( p1.type() == QVariant::String )
	    env->program()->setCounter( p1.toString() );
	else
	    env->program()->setCounter( p1.toInt() );
	return TRUE;
    }
};

/* Push the value of field number P2 from the file identified by 'id'
 on to the top of the stack.  The file must be open (see Open) and positioned on
 a valid record.
*/

class PushFieldValue : public Label
{
public:
    PushFieldValue( const QVariant& id,
	       const QVariant& P2,
	       const QString& label = QString::null )
	: Label( id, P2, label ) {}
    QString name() const { return "pushfieldvalue"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:pushfieldvalue: file not open" );
	    return 0;
	}
	QVariant v;
	if ( !drv->field( p2.toInt(), v ) ) {
	    env->setLastError("internal error:pushfieldvalue: unable to get field value");
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

class PushFieldDesc : public Label
{
public:
    PushFieldDesc( const QVariant& id,
		   const QVariant& nameOrNumber,
		   const QString& label = QString::null )
	: Label( id, nameOrNumber, label ) {}
    QString name() const { return "pushfielddesc"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:pushfielddesc: file not open" );
	    return 0;
	}
	QVariant v;
	if ( p2.type() == QVariant::String || QVariant::CString ) {
	    if ( !drv->fieldDescription( p2.toString(), v ) ) {
		env->setLastError("internal error:pushfielddesc: unable to get field description");
		return FALSE;
	    }
	} else {
	    if ( !drv->fieldDescription( p2.toInt(), v ) ) {
		env->setLastError("internal error:pushfielddesc: unable to get field description");
		return FALSE;
	    }
	}
	env->stack()->push( v );
	return TRUE;
    }
};

/* Pops the top of the stack (which must be a 'list', see PushList)
  and appends it to the internal result set (see CreateResult).  The
  list must be of the form:

  data (variant)
  data (variant)
  ...etc

  The number of elements in the list must match the number of elements
  in the result set.  It is an error to attempt to save an empty list.

*/

class SaveResult : public Label
{
public:
    SaveResult( const QVariant& id,
		const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "saveresult"; }
    int exec( qdb::Environment* env )
    {
	qdb::List list = env->stack()->pop().toList();
	if ( !list.count() ) {
	    env->setLastError("internal error:saveresult: no values");
	    return 0;
	}
	return env->resultSet( p1.toInt() )->append( list );
    }
};

/*  Pops the top of the stack (which must be a 'list', see PushList)
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

class CreateResult : public Label
{
public:
    CreateResult( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "createresult"; }
    int exec( qdb::Environment* env )
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

class RewindMarked : public Label
{
public:
    RewindMarked( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "rewindmarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	return drv->rewindMarked();
    }
};


/* Go to next marked record of the file identified by 'id'.  On
 failure goto P2.  The file must be open (see Open).
*/

class NextMarked : public Label
{
public:
    NextMarked( const QVariant& id,
		const QVariant& P2,
	    const QString& label = QString::null )
	: Label( id, P2, label ) {}
    QString name() const { return "nextmarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:nextmarked: file not open" );
	    return 0;
	}
	bool b = drv->nextMarked();
	if ( !b ) {
	    if ( p2.type() == QVariant::String )
		env->program()->setCounter( p2.toString() );
	    else
		env->program()->setCounter( p2.toInt() );
	}
	return TRUE;
  }
};


/* Pop the top of the stack (which must be a 'list', see PushList) and
   use it to update all fields of the current record buffer of the
   file identified by 'id'.  The list must be in the following form:

  field data
  field data
  field data
  ...

  Where each 'field data' is a value list of the form:

  name
  data

   The file must be open (see Open).
*/

class Update : public Label
{
public:
    Update( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "update"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:update: file not open" );
	    return 0;
	}
	bool b = drv->update( env->stack()->pop().toList() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};

/* Pop the top of the stack (which must be a 'list', see PushList) and
   use it to 'range scan' the record buffer of the file identified by
   'id'.  A 'range scan' tries to match every record in the file where
   the fields corresponding to the 'list' fields match the values of
   the corresponding list field values.  This can be optimised by
   drivers who use indexes, and therefore speed up the common cases
   such as:

   select field from table where id = 1;
   update table set field="blah" where id = 1;
   delete from table where id = 1;

   In the above examples, 'table' can be range scanned based on the
   'id' field.  If the driver uses an index on the 'id' field of
   'table, it can optimize the search.

   All records that match the 'range scan' will be 'marked' (see
   RangeScan).  The file must be open (see Open).

   The 'list' which is popped from the top of the stack must be of the form:

   field data
   field data
   field data
   ...

   Where each 'field data' is a value list of the form:

   name
   data

//## do we need this?: If the P2 parameter is specified, all records
//which are marked are also saved (see SaveResult). <- but how do we
//specify what fields are saved?  If this was implemented, we would
//only have to visit records once when doing a simple range scan &
//save.

*/
class RangeScan : public Label
{
public:
    RangeScan( const QVariant& id,
	       const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "rangescan"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:rangescan: file not open" );
	    return 0;
	}
	bool b = drv->rangeScan( env->stack()->pop().toList() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};

/*  Pop the top of the stack (which must be a list, see PushList) and
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

class CreateIndex : public Label
{
public:
    CreateIndex( const QVariant& id,
		 const QVariant& unique,
		  const QString& label = QString::null )
	: Label( id, unique, label ) {}
    QString name() const { return "createindex"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:createindex: file not open" );
	    return 0;
	}
	bool b = drv->createIndex( env->stack()->pop().toList(), p2.toBool() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};

/*  Drops (deletes) the file identified by 'id'.  If the file contains
    any indexes, they are also dropped (deleted).  If the file is
    open, it is first closed.
*/

class Drop : public Label
{
public:
    Drop( const QVariant& id, const QVariant& name,
	      const QString& label = QString::null )
	: Label( id, name, label ) {}
    QString name() const { return "drop"; }
    int exec( qdb::Environment* env )
    {
	env->addFileDriver( p1.toInt(), p2.toString() );
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->drop() ) {
	    env->setLastError("internal error:drop: unable to drop file:" + p2.toString() );
	    return 0;
	}
	return 1;
    }
};

/* Pop the top of the stack (which must be a list, see PushList) and
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

class Sort : public Label
{
public:
    Sort( const QVariant& id,
	  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "sort"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("internal error:sort: file not open" );
	    return 0;
	}
	bool b = env->resultSet( p1.toInt() )->sort( env->stack()->pop().toList() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};


/*  Clears all marked records from the file which is identified by
'id'.
*/

class ClearMarked : public Label
{
public:
    ClearMarked( const QVariant& id, const QVariant& name,
	      const QString& label = QString::null )
	: Label( id, name, label ) {}
    QString name() const { return "clearmarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->clearMarked() ) {
	    env->setLastError("internal error:clearmarked: unable to clear marks" );
	    return 0;
	}
	return 1;
    }
};

#endif
