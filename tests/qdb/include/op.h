#ifndef OP_H
#define OP_H

#include "environment.h"
#include "sqlinterpreter.h"
#include <qvariant.h>
#include <qlist.h>
#include <qvaluelist.h>

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
    QString name() const { return "Noop"; }
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
    QString name() const { return "Push"; }
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
    QString name() const { return "Add"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Add: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	env->stack()->push( v1.toDouble() + v2.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, subtract the first (what
 was on top of the stack) from the second (the next on stack) and push
 the result (which is of type double) back onto the stack.
*/
class Subtract : public Label
{
public:
    Subtract( const QString& label = QString::null )
	: Label( label ) {}
    QString name() const { return "Subtract"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Subtract: not enough stack elements");
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
    QString name() const { return "Multiply"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Multiply: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	env->stack()->push( v2.toDouble() * v1.toDouble() );
	return 1;
    }
};

/* Pop the top two elements from the stack, divide the first (what was
 on top of the stack) from the second (the next on stack) and push the
 result (which is of type double) back onto the stack.  Division
 by zero puts an invalid variant back on the stack.
*/
class Divide : public Label
{
public:
    Divide( const QString& label = QString::null )
	: Label( label ) {}
    QString name() const { return "Divide"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Divide: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack()->pop();
	QVariant v2 = env->stack()->pop();
	if ( v1.toInt() == 0 )
	    env->stack()->push( QVariant() );
	else
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
    QString name() const { return "Eq"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Eq: not enough stack elements");
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
    QString name() const { return "Ne"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Ne: not enough stack elements");
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

/* Pop the top two elements from the stack.  If second element (the
 next on stack) is less than the first (the top of stack), then jump
 to instruction P1.  Otherwise, continue to the next instruction.  In
 other words, jump if NOS<TOS.
*/
class Lt : public Label
{
public:
    Lt( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "Lt"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Lt: not enough stack elements");
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

/* Pop the top two elements from the stack.  If second element (the
 next on stack) is less than or equal to the first (the top of stack),
 then jump to instruction P1. In other words, jump if NOS<=TOS.
*/
class Le : public Label
{
public:
    Le( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "Le"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Le: not enough stack elements");
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


/* Pop the top two elements from the stack.  If second element (the
 next on stack) is greater than the first (the top of stack), then
 jump to instruction P1. In other words, jump if NOS>TOS.
*/
class Gt : public Label
{
public:
    Gt( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "Gt"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Gt: not enough stack elements");
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

/* Pop the top two elements from the stack.  If second element (the
 next on stack) is greater than or equal to the first (the top of
 stack), then jump to instruction P1. In other words, jump if
 NOS>=TOS.
*/
class Ge : public Label
{
public:
    Ge( const QVariant& P1, const QString& label = QString::null )
	: Label( P1, label ) {}
    QString name() const { return "Ge"; }
    int exec( qdb::Environment* env )
    {
	if ( env->stack()->count() < 2 ) {
	    env->setLastError("Ge: not enough stack elements");
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
    QString name() const { return "PushList"; }
    int exec( qdb::Environment* env )
    {
	if ( (int)env->stack()->count() < p1.toInt() ) {
	    env->setLastError("PushList: not enough stack elements");
	    return 0;
	}
	QValueList<QVariant> rec;
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

 field decimal precision
 field length
 field type
 field name

 'Field type' must be an int which corresponds QVariant::Type.

*/

class Create : public Label
{
public:
    Create( const QVariant& name,
	    const QString& label = QString::null )
	: Label( name, label ) {}
    QString name() const { return "Create"; }
    int exec( qdb::Environment* env )
    {
	QValueList<QVariant> list = env->stack()->pop().toList();
	if ( !list.count() ) {
	    env->setLastError("Create: no fields defined!");
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
    QString name() const { return "Open"; }
    int exec( qdb::Environment* env )
    {
	env->addFileDriver( p1.toInt(), p2.toString() );
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->open() ) {
	    env->setLastError("Open: unable to open file:" + p2.toString() );
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
    QString name() const { return "Close"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("Close: file not open!" );
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

  data
  name


  The file must be open.

*/
class Insert : public Label
{
public:
    Insert( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "Insert"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("Insert: file not open!" );
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
    QString name() const { return "Mark"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("Mark: file not open!" );
	    return 0;
	}
	return drv->mark();
    }
};

/*  Deletes all record from the file identified by 'id' which have
been previously marked by Mark. All marks are cleared after this op.
*/

class DeleteMarked : public Label
{
public:
    DeleteMarked( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "DeleteMarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("DeleteMarked: file not open!" );
	    return 0;
	}
	return drv->deleteMarked();
    }
};

/*  Pops the top of the stack (which must be a 'list', see PushList)
  and updates all record from the file identified by 'id' which have
  been previously marked by Mark.  The marked records are updated only
  with the fields specified by the list.  The file must be open. The
  list must be of the following form:

  field data
  field data
  field data
...

  Where each 'field data' is a value list of the form:

  data
  name

  All marks are cleared after this op.
*/

class UpdateMarked : public Label
{
public:
    UpdateMarked( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "UpdateMarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("UpdateMarked: file not open!" );
	    return 0;
	}
	bool b = drv->updateMarked( env->stack()->pop().toList() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};


/* Go to next record of file identified by 'id'.  On failure goto P2.
 The file must be open.
*/

class Next : public Label
{
public:
    Next( const QVariant& id,
	  const QVariant& P2,
	  const QString& label = QString::null )
	: Label( id, P2, label ) {}
    QString name() const { return "Next"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("Next: file not open!" );
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
    QString name() const { return "Goto"; }
    int exec( qdb::Environment* env )
    {
	if ( p1.type() == QVariant::String )
	    env->program()->setCounter( p1.toString() );
	else
	    env->program()->setCounter( p1.toInt() );
	return TRUE;
    }
};

/* Push the value of field number P2 from the file identified by 'id' on to the
 top of the stack The file must be open and positioned on a valid
 record.
*/

class PushFieldValue : public Label
{
public:
    PushFieldValue( const QVariant& id,
	       const QVariant& P2,
	       const QString& label = QString::null )
	: Label( id, P2, label ) {}
    QString name() const { return "PushFieldValue"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("PushFieldValue: file not open!" );
	    return 0;
	}
	QVariant v;
	if ( !drv->field( p2.toInt(), v ) ) {
	    env->setLastError("PushFieldValue: unable to get field value!");
	    return FALSE;
	}
	env->stack()->push( v );
	return TRUE;
    }
};


/* Push field description information of the field identified by
'nameOrNumber' in the file identified by 'id onto the top of the
stack.  'nameOrNumber' can be the name of the field or the number of
the field within the file.  The field must exist in the file.
*/

//## drop this op???

class PushFieldDesc : public Label
{
public:
    PushFieldDesc( const QVariant& id,
		   const QVariant& nameOrNumber,
		   const QString& label = QString::null )
	: Label( id, nameOrNumber, label ) {}
    QString name() const { return "PushFieldDesc"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("PushFieldDesc: file not open!" );
	    return 0;
	}
	QVariant v;
	if ( p2.type() == QVariant::String || QVariant::CString ) {
	    if ( !drv->fieldDescription( p2.toString(), v ) ) {
		env->setLastError("PushFieldDesc: unable to get field description!");
		return FALSE;
	    }
	} else {
	    if ( !drv->fieldDescription( p2.toInt(), v ) ) {
		env->setLastError("PushFieldDesc: unable to get field description!");
		return FALSE;
	    }
	}
	env->stack()->push( v );
	return TRUE;
    }
};

/* Pops the top of the stack (which must be a 'list', see PushList)
and appends it to the internal result set (see CreateResult).
*/

class SaveResult : public Label
{
public:
    SaveResult( const QVariant& id,
		const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "SaveResult"; }
    int exec( qdb::Environment* env )
    {
	qdb::List list = env->stack()->pop().toList();
	if ( !list.count() ) {
	    env->setLastError("SaveResult: no values!");
	    return 0;
	}
	return env->resultSet( p1.toInt() )->append( list );
    }
};

/*  Pops the top of the stack (which must be a 'list', see PushList)
and creates a 'result set'.  The result set will be identified by
'id'. The result set is an internal memory area which can be added to
(see SaveResult) and later retrieved (see Environment::result()).  The
'result set' forms the fundamental mechanism for selecting data from a
file.  The list must be of the form which identifies fields (see the
docs for Create), since both a field name and type are required to
define a 'result set'.
*/

class CreateResult : public Label
{
public:
    CreateResult( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "CreateResult"; }
    int exec( qdb::Environment* env )
    {
	env->addResultSet( p1.toInt() );
	return env->resultSet( p1.toInt() )->setHeader( env->stack()->pop().toList() );
    }
};

/* Resets the internal marked-record iterator to the beginning (see
Mark).  Marked records can then be sequentially retrieved using
NextMarked.
*/

class RewindMarked : public Label
{
public:
    RewindMarked( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "RewindMarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("RewindMarked: file not open!" );
	    return 0;
	}
	return drv->rewindMarked();
    }
};


/* Go to next marked record of the file identified by 'id'.  On
 failure goto P2.  The file must be open.
*/

class NextMarked : public Label
{
public:
    NextMarked( const QVariant& id,
		const QVariant& P2,
	    const QString& label = QString::null )
	: Label( id, P2, label ) {}
    QString name() const { return "NextMarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("NextMarked: file not open!" );
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

  data
  name

   The file must be open.
*/

class Update : public Label
{
public:
    Update( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "Update"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("Update: file not open!" );
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
   the corresponding list fields.  This can be optimised by drivers
   who use indexes, and therefore speed up the common cases such as:

   select field from table where id = 1;
   update table set field="blah" where id = 1;
   delete from table where id = 1;

   In the above examples, 'table' can be range scanned based on the
   'id' field.  If the driver uses an index on the 'id' field of
   'table, it can optimize the search.

   All records that match the 'range scan' will be 'marked'.  The file
   must be open.

   The 'list' which is popped from the top of the stack must be of the form:

   field data
   field data
   field data
   ...

   Where each 'field data' is a value list of the form:

   data
   name


   The above will range scan the "myfile.dbf" file by name="trolltech".

//## do we need this: If the P2 parameter is specified, all records
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
    QString name() const { return "RangeScan"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("RangeScan: file not open!" );
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

   descending (bool)
   type (QVariant::Type)
   name

  Creates an index on the file identified by 'id'.  The file must be
  open.
*/

class CreateIndex : public Label
{
public:
    CreateIndex( const QVariant& id,
		 const QVariant& unique,
		  const QString& label = QString::null )
	: Label( id, unique, label ) {}
    QString name() const { return "CreateIndex"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("CreateIndex: file not open!" );
	    return 0;
	}
	bool b = drv->createIndex( env->stack()->pop().toList(), p2.toBool() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};

/*  Drops (deletes) the file identified by 'id'.  If the file contains
any indexes, they are also dropped (deleted).
*/

class Drop : public Label
{
public:
    Drop( const QVariant& id, const QVariant& name,
	      const QString& label = QString::null )
	: Label( id, name, label ) {}
    QString name() const { return "Drop"; }
    int exec( qdb::Environment* env )
    {
	env->addFileDriver( p1.toInt(), p2.toString() );
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->drop() ) {
	    env->setLastError("Drop: unable to drop file:" + p2.toString() );
	    return 0;
	}
	return 1;
    }
};

/* Pop the top of the stack (which must be a list, see PushList) and
  use it as a list of fields (see Create) with which to sort the
  'result set' which is identified by 'id' (see CreateResult).

   The 'list' which is popped from the top of the stack must be of the form:

   TRUE (beginning of list)
   list: field description (see PushFieldDesc)
   FALSE
   list: field description (see PushFieldDesc)

   IOW, the list represents alternating 'descending flag'/'field
   descriptions'.  The field descriptions are used to identify the
   fields in the file, and the descending flags are used to indicate a
   descending (rather than ascending) sort.  For example, the
   following code snippet does a sort:

   PushFieldDesc( 0, "id" )
   Push( QVariant( TRUE, 0 )  )
   PushFieldDesc( 0, "name" )
   Push( QVariant( FALSE, 0 ) )
   PushList( 4 )
   Sort( 0 )


   The above will sort the result set file by 'id' DESCending then
   'name' ASCending.

*/

class Sort : public Label
{
public:
    Sort( const QVariant& id,
	  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "Sort"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->isOpen() ) {
	    env->setLastError("Sort: file not open!" );
	    return 0;
	}
	bool b = env->resultSet( p1.toInt() )->sort( env->stack()->pop().toList() );
	if ( !b )
	    env->setLastError( drv->lastError() );
	return b;
    }
};


/*  Clears all marked records from the file identified by 'id'.
*/

class ClearMarked : public Label
{
public:
    ClearMarked( const QVariant& id, const QVariant& name,
	      const QString& label = QString::null )
	: Label( id, name, label ) {}
    QString name() const { return "ClearMarked"; }
    int exec( qdb::Environment* env )
    {
	qdb::FileDriver* drv = env->fileDriver( p1.toInt() );
	if ( !drv->clearMarked() ) {
	    env->setLastError("ClearMarked: unable to clear marks!" );
	    return 0;
	}
	return 1;
    }
};

#endif
