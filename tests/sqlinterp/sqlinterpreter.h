#ifndef SQLINTERPRETER_H
#define SQLINTERPRETER_H

#include "environment.h"
#include <qstack.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>

class FileDriver : public Interpreter::FileDriver
{
public:
    FileDriver( const QString& name = QString::null );
    virtual ~FileDriver();
    FileDriver( const FileDriver& other );
    FileDriver& operator=( const FileDriver& other );

    QString name() const { return nm; }

    bool create( const QSqlRecord* record );
    bool open();
    bool close();
    bool isOpen() const { return opened; }
    bool insert( const QSqlRecord* record );
    int at() const { return internalAt; }
    bool next();
    bool mark();
    bool deleteMarked();
    bool commit();
    bool field( uint i, QVariant& v );
    bool updateMarked( const QSqlRecord* record );
    bool rewindMarked();
    bool nextMarked();
    bool update( const QSqlRecord* record );
    void setLastError( const QString& error ) { err = error; }
    QString lastError() const { return err; }
    bool rangeScan( const QSqlRecord* index );
    bool createIndex( const QSqlRecord* index, bool unique );
    bool drop();
    bool fieldDescription( const QString& name, QVariant& v );

protected:
    virtual void setName( const QString& name ) { nm = name; }
    void setIsOpen( bool b ) { opened = b; }
    void setAt( int at ) { internalAt = at; }
    int markedAt() const { return internalMarkedAt; }
    void setMarkedAt( int at ) { internalMarkedAt = at; }

private:
    QString nm;
    class Private;
    Private* d;
    bool opened;
    int internalAt;
    int internalMarkedAt;
    QString err;

};

class ResultSet : public Interpreter::ResultSet
{
public:
    ResultSet( Interpreter::Environment* environment = 0 );
    virtual ~ResultSet();
    ResultSet( const ResultSet& other );
    ResultSet& operator=( const ResultSet& other );

    bool setHeader( const QSqlRecord& record );
    QSqlRecord& header() { return head; }
    bool append( QValueList<QVariant>& buf );
    void clear() { data.clear(); sortKey.clear(); head.clear(); }
    bool sort( const QSqlIndex* index );
    bool first();
    bool last();
    bool next();
    bool prev();
    Record& currentRecord();
    uint size() { return data.count(); }

private:
    QSqlRecord head;
    Data data;
    Interpreter::Environment* env;
    ColumnKey sortKey;
    ColumnKey::ConstIterator keyit;
    Data::Iterator datait;
    int j;
};

class Program : public Interpreter::Program
{
public:
    Program();
    virtual ~Program();

    void append( Interpreter::Op* op );
    void remove( uint i );
    void clear();
    void setCounter( int i );
    void setCounter( const QString& label );
    void resetCounter();
    int counter();
    Interpreter::Op* next();
    void setLastError( const QString& error ) { err = error; }
    QString lastError() const { return err; }

private:
    QList< Interpreter::Op > ops;
    int pc;
    QString err;
    ColumnKey sortKey;

    Program( const Program& other );
    Program& operator=( const Program& other );

};

class Environment : public Interpreter::Environment
{
public:
    Environment();
    virtual ~Environment();

    int execute();
    void reset();
    void addDriver( int id, const QString& fileName );
    void addResult( int id );
    FileDriver& fileDriver( int id );
    QValueStack<QVariant>& stack();
    Program& program();
    ResultSet& resultSet( int id );
    bool save( QIODevice *dev );
    bool save( const QString& filename );
    bool saveListing( QTextStream& stream );
    bool saveListing( const QString& filename );

private:
    QMap<int,FileDriver> drivers;
    QMap<int,ResultSet> results;
    QValueStack<QVariant> stck;
    Program pgm;

};

/* sql ops follow */

/* Base class for all ops.
*/
class Label : public Interpreter::Op
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
    int exec( Interpreter::Environment* )
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
    int exec( Interpreter::Environment* env )
    {
	if ( p1.isValid() )
	    env->stack().push( p1 );
	if ( p2.isValid() )
	    env->stack().push( p2 );
	if ( p3.isValid() )
	    env->stack().push( p3 );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Add: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	env->stack().push( v1.toDouble() + v2.toDouble() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Subtract: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	env->stack().push( v2.toDouble() - v1.toDouble() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Multiply: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	env->stack().push( v2.toDouble() * v1.toDouble() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Divide: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v1.toInt() == 0 )
	    env->stack().push( QVariant() );
	else
	    env->stack().push( v2.toDouble() / v1.toDouble() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Eq: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v1 == v2 ) {
	    if ( p1.type() == QVariant::String )
		env->program().setCounter( p1.toString() );
	    else
		env->program().setCounter( p1.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Ne: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v1 != v2 ) {
	    if ( p1.type() == QVariant::String )
		env->program().setCounter( p1.toString() );
	    else
		env->program().setCounter( p1.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Lt: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v2.toDouble() < v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program().setCounter( p1.toString() );
	    else
		env->program().setCounter( p1.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Le: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v2.toDouble() <= v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program().setCounter( p1.toString() );
	    else
		env->program().setCounter( p1.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Gt: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v2.toDouble() > v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program().setCounter( p1.toString() );
	    else
		env->program().setCounter( p1.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( env->stack().count() < 2 ) {
	    env->program().setLastError("Ge: not enough stack elements");
	    return 0;
	}
	QVariant v1 = env->stack().pop();
	QVariant v2 = env->stack().pop();
	if ( v2.toDouble() >= v1.toDouble() ) {
	    if ( p1.type() == QVariant::String )
		env->program().setCounter( p1.toString() );
	    else
		env->program().setCounter( p1.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( (int)env->stack().count() < p1.toInt() ) {
	    env->program().setLastError("PushList: not enough stack elements");
	    return 0;
	}
	QValueList<QVariant> rec;
	for ( int i = 0; i < p1.toInt(); ++i )
	    rec.prepend( env->stack().pop() );
	env->stack().push( rec );
	return 1;
    }
};

/* Pop top of stack (which should be a 'list', see 'PushList') and
 create a file with name 'name'.  The list should be of the form:

 QValueList<QVariant> field;
 QValueList<field> list;

 where 'field' is a value list of variants in the following order:

 list element 0: name of field (string)
 list element 1: type of field (an int corresponding to a QVariant::Type)
*/

class Create : public Label
{
public:
    Create( const QVariant& name,
	    const QString& label = QString::null )
	: Label( name, label ) {}
    QString name() const { return "Create"; }
    int exec( Interpreter::Environment* env )
    {
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("Create: no fields defined!");
	    return 0;
	}
	for ( uint i = 0; i < list.count(); ++i ) {
	    QValueList<QVariant> fieldDescription = list[i].toList();
	    if ( fieldDescription.count() != 2 ) {
		env->program().setLastError("Create: bad field description!");
		return 0;
	    }
	    QString name = fieldDescription[0].toString();
	    QVariant::Type type = fieldDescription[1].type();
	    QSqlField field( name, type );
	    rec.append( field );
	}
	env->addDriver( 0, p1.toString() );
	Interpreter::FileDriver& drv = env->fileDriver( 0 );
	return drv.create( &rec );
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
    int exec( Interpreter::Environment* env )
    {
	env->addDriver( p1.toInt(), p2.toString() );
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.open() ) {
	    env->program().setLastError("Open: unable to open file:" + p2.toString() );
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("Close: file not open!" );
	    return 0;
	}
	return drv.close();
    }
};


/* Pop the top of the stack (which must be a 'list', see PushList) and
   insert it into the file identified by 'id'.  The values of the list
   must corespond in number and type to the file they are being
   inserted to.  The file must be open.
*/
class Insert : public Label
{
public:
    Insert( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "Insert"; }
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("Insert: file not open!" );
	    return 0;
	}
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("Insert: no values!");
	    return 0;
	}
	for ( uint i = 0; i < list.count(); ++i ) {
	    QVariant val = list[i];
	    QSqlField field( QString::null, val.type() );
	    field.setValue( val );
	    rec.append( field );
	}
	return drv.insert( &rec );
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("Mark: file not open!" );
	    return 0;
	}
	return drv.mark();
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("DeleteMarked: file not open!" );
	    return 0;
	}
	return drv.deleteMarked();
    }
};

/*  Pops the top of the stack (which must be a 'list', see PushList)
and updates all record from the file identified by 'id' which have
been previously marked by Mark.  The marked records are updated only
with the fields specified by the list.  The file must be open. The
list must be a list of fields (see the docs of Create). The 'type'
element of eac field will also be used as the value when updating the
file fields.  All marks are cleared after this op.
*/

class UpdateMarked : public Label
{
public:
    UpdateMarked( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "UpdateMarked"; }
    int exec( Interpreter::Environment* env )
    {
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("UpdateMarked: no fields defined!");
	    return 0;
	}
	for ( uint i = 0; i < list.count(); ++i ) {
	    QValueList<QVariant> fieldDescription = list[i].toList();
	    if ( fieldDescription.count() != 2 ) {
		env->program().setLastError("Create: bad field description!");
		return 0;
	    }
	    QString name = fieldDescription[0].toString();
	    QVariant::Type type = fieldDescription[1].type();
	    QSqlField field( name, type );
	    field.setValue( fieldDescription[1] );
	    rec.append( field );
	}
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("UpdateMarked: file not open!" );
	    return 0;
	}
	bool b = drv.updateMarked( &rec );
	if ( !b )
	    env->program().setLastError( drv.lastError() );
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("Next: file not open!" );
	    return 0;
	}
	if ( !drv.next() ) {
	    if ( p2.type() == QVariant::String )
		env->program().setCounter( p2.toString() );
	    else
		env->program().setCounter( p2.toInt() );
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
    int exec( Interpreter::Environment* env )
    {
	if ( p1.type() == QVariant::String )
	    env->program().setCounter( p1.toString() );
	else
	    env->program().setCounter( p1.toInt() );
	return TRUE;
    }
};

/* Push an artificial 'field description' (see PishFieldDesc) on to
   the stack using 'name' and 'type.  Type must correspond to a
   QVariant::Type. This op is useful for creating a field description
   when there is no open file to use (e.g., when creating files, see
   Create).
*/

class PushField : public Label
{
public:
    PushField( const QVariant& name,
	       const QVariant& type,
	       const QString& label = QString::null )
	: Label( name, type, label ) {}
    QString name() const { return "PushField"; }
    int exec( Interpreter::Environment* env )
    {

	QVariant& name = p1;
	QVariant value;
	value.cast( (QVariant::Type)p2.toInt() );
	QValueList<QVariant> field;
	field.append( name );
	field.append( value );
	env->stack().push( field );
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("PushFieldValue: file not open!" );
	    return 0;
	}
	QVariant v;
	if ( !drv.field( p2.toInt(), v ) ) {
	    env->program().setLastError("PushFieldValue: unable to get field value!");
	    return FALSE;
	}
	env->stack().push( v );
	return TRUE;
    }
};


/* Push field description information of the field identified by
'name' in the file identified by 'id onto the top of the stack.  If
the field does not exist, jump to P3.
*/

class PushFieldDesc : public Label
{
public:
    PushFieldDesc( const QVariant& id,
		   const QVariant& name,
		   const QVariant& P3,
		   const QString& label = QString::null )
	: Label( id, name, P3, label ) {}
    QString name() const { return "PushFieldDesc"; }
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("PushFieldDesc: file not open!" );
	    return 0;
	}
	QVariant v;
	if ( !drv.fieldDescription( p2.toString(), v ) ) {
	    env->program().setLastError("PushFieldDesc: unable to get field description!");
	    return FALSE;
	}
	env->stack().push( v );
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
    int exec( Interpreter::Environment* env )
    {
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("SaveResult: no values!");
	    return 0;
	}
	return env->resultSet( p1.toInt() ).append( list );
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
    int exec( Interpreter::Environment* env )
    {
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("CreateResult: no fields defined!");
	    return 0;
	}
	for ( int i = 0; i < (int)list.count(); ++i ) {
	    QValueList<QVariant> fieldDescription = list[i].toList();
	    if ( fieldDescription.count() != 2 ) {
		env->program().setLastError("CreateResult: bad field description!");
		return 0;
	    }
	    QString name = fieldDescription[0].toString();
	    QVariant::Type type = fieldDescription[1].type();
	    QSqlField field( name, type );
	    rec.append( field );
	}
	env->addResult( p1.toInt() );
	return env->resultSet( p1.toInt() ).setHeader( rec );
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("RewindMarked: file not open!" );
	    return 0;
	}
	return drv.rewindMarked();
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
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("NextMarked: file not open!" );
	    return 0;
	}
	bool b = drv.nextMarked();
	if ( !b ) {
	    if ( p2.type() == QVariant::String )
		env->program().setCounter( p2.toString() );
	    else
		env->program().setCounter( p2.toInt() );
	}
	return TRUE;
  }
};


/* Pop the top of the stack (which must be a 'list', see PushList) and
   use it to update all fields of the current record buffer of the
   file identified by 'id'.  The list must correspond in number and
   type to the fields in the file.  The file must be open.
*/

class Update : public Label
{
public:
    Update( const QVariant& id,
	    const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "Update"; }
    int exec( Interpreter::Environment* env )
    {
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("Update: file not open!" );
	    return 0;
	}
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("Update: no values!");
	    return 0;
	}
	for ( uint i = 0;  i < list.count(); ++i ) {
	    QVariant val = list[i];
	    QSqlField field( QString::null, val.type() );
	    field.setValue( val );
	    rec.append( field );
	}
	bool b = drv.update( &rec );
	if ( !b )
	    env->program().setLastError( drv.lastError() );
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

   "dave" (beginning of list)
   list: field description (see PushFieldDesc)
   99
   list: field description (see PushFieldDesc)

   IOW, the list represents alternating 'values'/'field descriptions'.
   The field descriptions are used to identify the fields in the file,
   and the values are used to range scan the actual data.  For
   example, the following code snippet does a range scan:

   Open( 0, "myfile.dbf" )
   PushFieldDesc( 0, "name", 15 )
   Push( "trolltech" )
   PushList( 2 )
   RangeScan( 0 )

   The above will range scan the "myfile.dbf" file by name="trolltech".

*/
class RangeScan : public Label
{
public:
    RangeScan( const QVariant& id,
		  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "RangeScan"; }
    int exec( Interpreter::Environment* env )
    {
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("RangeScan: no range scan fields defined!");
	    return 0;
	}
	if ( (list.count() % 2) != 0 ) {
	    env->program().setLastError("RangeScan: wrong multiple of list elements!");
	    return 0;
	}
	for ( uint j = 0; j < list.count(); ++j ) {
	    QValueList<QVariant> fieldDescList = list[j].toList();
	    if ( fieldDescList.count() != 2 ) {
		env->program().setLastError("RangeScan: bad field description!");
		return 0;
	    }
	    QString name = fieldDescList[0].toString();
	    QVariant::Type type = fieldDescList[1].type();
	    QSqlField field( name, type );
	    QVariant value = list[++j];
	    field.setValue( value );
	    rec.append( field );
	}
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("RangeScan: file not open!" );
	    return 0;
	}
	bool b = drv.rangeScan( &rec );
	if ( !b )
	    env->program().setLastError( drv.lastError() );
	return b;
    }
};

/*  Pop the top of the stack (which must be a list, see PushList) and
use it as a description of fields.  Creates an index on the file
identified by 'id'.  The file must be open.
*/

class CreateIndex : public Label
{
public:
    CreateIndex( const QVariant& id,
		 const QVariant& unique,
		  const QString& label = QString::null )
	: Label( id, unique, label ) {}
    QString name() const { return "CreateIndex"; }
    int exec( Interpreter::Environment* env )
    {
	QSqlRecord rec;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("CreateIndex: no fields defined!");
	    return 0;
	}
	for ( uint i = 0; i < list.count(); ++i ) {
	    QValueList<QVariant> fieldDescription = list[i].toList();
	    if ( fieldDescription.count() != 2 ) {
		env->program().setLastError("Create: bad field description!");
		return 0;
	    }
	    QString name = fieldDescription[0].toString();
	    QVariant::Type type = fieldDescription[1].type();
	    QSqlField field( name, type );
	    field.setValue( fieldDescription[1] );
	    rec.append( field );
	}
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("CreateIndex: file not open!" );
	    return 0;
	}
	bool b = drv.createIndex( &rec, p2.toBool() );
	if ( !b )
	    env->program().setLastError( drv.lastError() );
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
    int exec( Interpreter::Environment* env )
    {
	env->addDriver( p1.toInt(), p2.toString() );
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.drop() ) {
	    env->program().setLastError("Drop: unable to drop file:" + p2.toString() );
	    return 0;
	}
	return 1;
    }
};

/* Pop the top of the stack (which must be a list, see PushList) and
use it as a list of fields (see Create) with which to sort the 'result
set' which is identified by 'id' (see CreateResult).

//## more: we should allow ASC/DESC sorting somehow
*/

class Sort : public Label
{
public:
    Sort( const QVariant& id,
	  const QString& label = QString::null )
	: Label( id, label ) {}
    QString name() const { return "Sort"; }
    int exec( Interpreter::Environment* env )
    {
	QSqlIndex idx;
	QValueList<QVariant> list = env->stack().pop().toList();
	if ( !list.count() ) {
	    env->program().setLastError("Sort: no fields defined!");
	    return 0;
	}
	for ( uint i = 0; i < list.count(); ++i ) {
	    QValueList<QVariant> fieldDescription = list[i].toList();
	    if ( fieldDescription.count() != 2 ) {
		env->program().setLastError("Create: bad field description!");
		return 0;
	    }
	    QString name = fieldDescription[0].toString();
	    QVariant::Type type = fieldDescription[1].type();
	    QSqlField field( name, type );
	    field.setValue( fieldDescription[1] );
	    idx.append( field );
	}
	Interpreter::FileDriver& drv = env->fileDriver( p1.toInt() );
	if ( !drv.isOpen() ) {
	    env->program().setLastError("Sort: file not open!" );
	    return 0;
	}
	bool b = env->resultSet( p1.toInt() ).sort( &idx );
	if ( !b )
	    env->program().setLastError( drv.lastError() );
	return b;
    }
};


#endif
