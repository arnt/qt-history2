#include "sqlinterpreter.h"
#include <qcleanuphandler.h>
#include <qdatetime.h>

//using namespace SqlInterpreter;

#define DEBUG_SQLINTERP 1

#ifdef DEBUG_SQLINTERP
#include <iostream>
using namespace std;
#endif

/*!  Constructs an empty

*/

Program::Program( )
{
    ops.setAutoDelete( TRUE );
}



/*! Destroys the object and frees any allocated resources.

*/

Program::~Program()
{

}


/*!  Appends \a op to the program listing.  The program takes
ownership of the pointer.

*/

void Program::append( Interpreter::Op* op )
{
    ops.append( op );
}




/*! Removes the operation at \a i.

*/

void Program::remove( uint i )
{
    ops.remove( i );
}


/*!

*/

void Program::clear()
{
    ops.clear();
}


/*! sets the program counter so that \a i is the next instruction to
be executed.

*/

void Program::setCounter( int i )
{
    pc = i - 1;
}


/*! Sets the program counter to the instruction with label \a label.

*/

void Program::setCounter( const QString& /*label*/ )
{

    qWarning("Program::setCounter( const QString& label ): not yet implemented!");
}


/*! Resets the counter to the beginning of the program (next() will
return the first instruction).

*/

void Program::resetCounter()
{
    pc = -1;
}


/*! Returns the current program counter.

*/

int Program::counter()
{
    return pc;
}


/*! Returns the next program instruction, or 0 if there is none.

*/

Interpreter::Op* Program::next()
{
    ++pc;
    if ( pc < (int)ops.count() )
	return ops.at( pc );
    return 0;
}


////////


/*!  Constructs an empty environment

*/

Environment::Environment()
    : result( this )
{

}


/*! Destroys the object and frees any allocated resources.

*/

Environment::~Environment()
{

}

void Environment::addDriver( int id, const QString& fileName )
{
    drivers[id] = FileDriver( fileName );
}


/*! Returns a reference to the file driver identified by \id

  \sa addDriver()

*/

FileDriver& Environment::fileDriver( int id )
{
    return drivers[id];
}


/*! Returns a reference to the stack.

*/

QValueStack<QVariant>& Environment::stack()
{
    return stck;
}


/*! Rxeturns a reference to the program.

*/

Program& Environment::program()
{
    return pgm;
}


/*!

*/

int Environment::execute()
{
    Interpreter::Op* op = 0;
    pgm.resetCounter();
#ifdef DEBUG_SQLINTERP
    qDebug("Program listing...");
    int i = 0;
    while( (op = pgm.next() ) ) {
	qDebug( QString::number(i).rightJustify(4) +
		op->name().rightJustify(15) +
		op->P(0).toString().rightJustify(10) +
		op->P(1).toString().rightJustify(10) +
		op->P(2).toString().rightJustify(10) );
	++i;
    }
    pgm.resetCounter();
    qDebug("\nExecuting...");
#endif
    while( (op = pgm.next() ) ) {
	//#ifdef DEBUG_SQLINTERP
#if 0
	qDebug( QString::number(pgm.counter()).rightJustify(4) +
		op->name().rightJustify(15) +
		op->P(0).toString().rightJustify(10) +
		op->P(1).toString().rightJustify(10) +
		op->P(2).toString().rightJustify(10) );
#endif
	if ( !op->exec( this ) ) {
	    qWarning("[Line " + QString::number(pgm.counter()) + "] " + pgm.lastError() );
	    break;
	}
    }
    return 0;
}


/*!

*/

void Environment::reset()
{
    stck.clear();
    pgm.clear();
    result.clear();
}

/*!

*/

ResultSet& Environment::resultSet()
{
    return result;
}

/*! Destroys the object and frees any allocated resources.

*/

Label::~Label()
{
}


/*!  Constructs an empty

*/

ResultSet::ResultSet( Interpreter::Environment* environment )
    : env( environment )
{

}


/*! Destroys the object and frees any allocated resources.

*/

ResultSet::~ResultSet()
{
}


/*!

*/

bool ResultSet::setHeader( const QSqlRecord& record )
{
    head = record;
    return TRUE;
}

/*!

*/

bool ResultSet::append( QValueList<QVariant>& buf )
{
    if ( !head.count() ) {
	env->program().setLastError( "ResultSet: no header" );
	return FALSE;
    }
    if ( head.count() != buf.count() ) {
	env->program().setLastError( "ResultSet: incorrect number of buffer fields" );
	return FALSE;
    }
    for ( uint j = 0; j < buf.count(); ++j ) {
	if ( buf[j].type() != head.field(j)->type() ) {
	    env->program().setLastError( "ResultSet: incorrect buffer field type" );
	    return FALSE;
	}
    }
    data.append( buf );
    return TRUE;
}

static bool operator<( const QVariant &v1, const QVariant& v2 )
{
    switch( v1.type() ) {
    case QVariant::String:
    case QVariant::CString:
	return v1.toString() < v2.toString();
    case QVariant::Date:
	return v1.toDate() < v2.toDate();
    case QVariant::Time:
	return v1.toTime() < v2.toTime();
    case QVariant::DateTime:
	return v1.toDateTime() < v2.toDateTime();
    default:
	return v1.toDouble() < v2.toDouble();
    }
}

#if 0
class RecordIterator
{
public:
    RecordIterator() : d(dummyD), k(dummyK), i(-1), j(-1) {}
    RecordIterator( Data& data, ColumnKey& key ) : d(data), k(key), i(0), j(0) {}
    RecordIterator( const RecordIterator& it ) : d(it.d), k(it.k), i(it.i), j(it.j) {}
    const int& operator*() const { return k[i][j]; }
    int& operator*() { return k[i][j]; }
    bool operator==( const RecordIterator& it ) const { return i == it.i; }
    bool operator!=( const RecordIterator& it ) const { return i != it.i; }
    RecordIterator& operator++() {
	if ( i == -1 || j == -1 )
	    return *this;
	return inc();
    }
    RecordIterator operator++(int) {
	if ( i == -1 || j == -1 )
	    return *this;
	RecordIterator tmp = *this;
	inc();
	return tmp;
    }
    RecordIterator& operator--() {
	if ( i == -1 || j == -1 )
	    return *this;
	return dec();
    }
    RecordIterator operator--(int) {
	if ( i == -1 || j == -1 )
	    return *this;
	RecordIterator tmp = *this;
	dec();
	return tmp;
    }
private:
    Data& d;
    ColumnKey& k;
    int i;
    int j;
    Data dummyD;
    ColumnKey dummyK;
    RecordIterator& inc() {
	if ( k[i].count() > 0 && j+1 < (int)k[i].count() ) {
	    ++j;
	    return *this;
	}
	++i;
	j = 0;
	if ( i > (int)d.count() ) {
	    i = -1;
	    j = -1;
	}
	return *this;
    }
    RecordIterator& dec() {
	if ( k[i].count() > 0 && j > 0 ) {
	    --j;
	    return *this;
	}
	--i;
	j = 0;
	if ( i < 0 ) {
	    i = -1;
	    j = -1;
	}
	return *this;
    }
};
#endif

/*!

*/

bool ResultSet::sort( const QSqlIndex* index )
{
    if ( !head.count() ) {
	env->program().setLastError( "ResultSet: no header" );
	return FALSE;
    }
    if ( !data.count() || !index->count() ) /* nothing to do */
	return TRUE;
    uint i = 0;
    for ( i = 0; i < index->count(); ++i ) {
	switch ( index->field(i)->type() ) {
	case QVariant::String:
	case QVariant::CString:
	case QVariant::Int:
	case QVariant::UInt:
	case QVariant::Double:
	case QVariant::Bool:
	case QVariant::Date:
	case QVariant::Time:
	case QVariant::DateTime:
	    continue;
	default:
	    env->program().setLastError( "ResultSet: invalid sort field type" );
	    return FALSE;
	}
    }

    sortKey.clear();

    /* init the sort key */
    for ( i = 0; i < data.count(); ++i ) {
	int sortField = head.position( index->field( index->count()-1 )->name() );
	if ( sortField == -1 ) {
	    env->program().setLastError( "ResultSet: sort field not found:" +
					 index->field( index->count()-1 )->name() );
	    return FALSE;
	}
	/* initialize - also handles the common case (sort by one field) */
	QVariant& v = data[i][sortField];
	if ( sortKey.find( v ) == sortKey.end() ) {
	    QValueList<int> nl;
	    nl.append( i );
	    sortKey[ v ] = nl;
	} else {
	    /* simulate multimap behavior */
	    QValueList<int>& nl = sortKey[ v ];
	    nl.append( i );
	}
    }
    ColumnKey::Iterator it;
    if ( index->count() > 1 ) {
	/* sort rest of fields */
	for ( int idx = index->count()-2; idx >= 0; --idx ) {
	    int sortField = head.position( index->field(idx)->name() );
	    if ( sortField == -1 ) {
		env->program().setLastError( "ResultSet: sort field not found:" +
					     index->field(idx)->name() );
		return FALSE;
	    }
	    ColumnKey subSort;
	    for ( it = sortKey.begin();
		  it != sortKey.end();
		  ++it ) {
		QValueList<int>& l = *it;
		for ( i = 0; i < l.count(); ++i ) {
		    /* put into new sorted map */
		    QVariant& v = data[l[i]][sortField];
		    if ( subSort.find( v ) == subSort.end() ) {
			QValueList<int> nl;
			nl.append( l[i] );
			subSort[ v ] = nl;
		    } else {
			/* simulate multimap behavior */
			QValueList<int>& nl = subSort[ v ];
			nl.append( l[i] );
		    }
		}
	    }
	    sortKey = subSort; /* save and continue */
	}
    }

#ifdef DEBUG_SQLINTERP
    qDebug("number of fields in result set:" + QString::number(head.count()) );
    qDebug("number of result records:" + QString::number( data.count() ) );
    qDebug("sorted results set:");
    //    ColumnKey::Iterator it;
    for ( it = sortKey.begin();
	  it != sortKey.end();
	  ++it ) {
	QValueList<int>& l = *it;
	for ( i = 0; i < l.count(); ++i ) {
	    for ( uint j = 0; j < head.count(); ++j )
		cout << data[l[i]][j].toString().rightJustify(10).latin1();
	    cout << endl;
	}
    }
#endif // DEBUG_SQLINTERP

    return TRUE;
}
