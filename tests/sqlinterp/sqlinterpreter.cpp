#include "sqlinterpreter.h"
#include <qsql.h>
#include <qcleanuphandler.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtl.h>

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
{
}


/*! Destroys the object and frees any allocated resources.

*/

Environment::~Environment()
{
    reset();
}

void Environment::addDriver( int id, const QString& fileName )
{
    drivers[id] = FileDriver( fileName );
}

void Environment::addResult( int id )
{
    results[id] = ResultSet( this );
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


/*! Returns a reference to the program.

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
    QTextStream cout( stdout, IO_WriteOnly );
    saveListing( cout );
    qDebug("\nExecuting...");
#endif
    while( (op = pgm.next() ) ) {
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
    uint i = 0;
    for( i = 0; i < drivers.count(); ++i )
	drivers[i].close();
    drivers.clear();
    results.clear();
}

/*!

*/

ResultSet& Environment::resultSet( int id )
{
    return results[id];
}

bool Environment::save( QIODevice *dev )
{
    if ( !dev || !dev->isOpen() )
	return FALSE;
    pgm.resetCounter();
    int i = 0;
    QDataStream stream( dev );
    Interpreter::Op* op = 0;
    while( (op = pgm.next() ) ) {
	stream << i << op->name();
	if ( op->P(0).isValid() )
	     stream << op->P(0);
	if ( op->P(1).isValid() )
	     stream << op->P(1);
	if ( op->P(2).isValid() )
	     stream << op->P(2);
	stream << "\n";
	++i;
    }
    pgm.resetCounter();
    return TRUE;
}

bool Environment::save( const QString& filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    save( &f );
    f.close();
    return TRUE;
}

static QString asListing( QVariant& v )
{
    QString s;
    switch( v.type() ) {
    case QVariant::List: {
	s = "list(";
	QValueList<QVariant> l = v.toList();
	for ( uint i = 0; i < l.count(); ++i )
	    s += asListing( l[i] ) + (i<l.count()-1?QString(","):QString(")"));
	break;
    }
    default:
	s = v.toString();
    }
    if ( s.isNull() )
	s = ".";
    return s;
}

bool Environment::saveListing( QTextStream& stream )
{
    pgm.resetCounter();
    int i = 0;
    Interpreter::Op* op = 0;
    while( (op = pgm.next() ) ) {
	stream << QString::number( i ).rightJustify(4) << op->name().rightJustify(15);
	stream << asListing( op->P(0) ).rightJustify(15);
	stream << asListing( op->P(1) ).rightJustify(15);
	stream << asListing( op->P(2) ).rightJustify(15);
	stream << "\n";
	++i;
    }
    pgm.resetCounter();
    return TRUE;
}


bool Environment::saveListing( const QString& filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    QTextStream stream( &f );
    saveListing( stream );
    f.close();
    return TRUE;
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

ResultSet::ResultSet( const ResultSet& other )
    : Interpreter::ResultSet()
{
    *this = other;
}

ResultSet& ResultSet::operator=( const ResultSet& other )
{
    env = other.env;
    head = other.head;
    data = other.data;
    env = other.env;
    sortKey = other.sortKey;
    keyit = other.keyit;
    datait = other.datait;
    j = other.j;
    return *this;
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
    if ( !env ) {
	qWarning( "ResultSet: no environment" );
	return FALSE;
    }
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

/* Provided so that we can sort a list of variants. Note that this
   does not work for variant types that have no value (e.g., picture,
   icon, etc), however we are only dealing with basic database types,
   so its cool.
*/
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

bool ResultSet::first()
{
    if ( !data.count() )
	return FALSE;
    keyit = sortKey.begin();
    datait = data.begin();
    j = 0;
    return TRUE;
}

bool ResultSet::last()
{
    if ( !data.count() )
	return FALSE;
    if ( sortKey.count() ) {
	keyit = --sortKey.end();
	j = keyit.data().count()-1;
    } else {
	datait = --data.end();
    }
    return TRUE;
}

bool ResultSet::next()
{
    if ( !data.count() )
	return FALSE;
    if ( sortKey.count() ) {
	if ( j+1 > (int)keyit.data().count()-1 ) { /* go to next map element */
	    if ( keyit == --sortKey.end() )
		return FALSE;
	    ++keyit;
	    j = 0;
	} else /* go to next list element in the same map element */
	    ++j;
    } else {
	if ( datait == --data.end() )
	    return FALSE;
	++datait;
    }
    return TRUE;
}

bool ResultSet::prev()
{
    if ( !data.count() )
	return FALSE;
    if ( sortKey.count() ) {
	if ( j-1 < 0 ) { /* go to previous map element */
	    if ( keyit == sortKey.begin() )
		return FALSE;
	    --keyit;
	    j = keyit.data().count()-1;
	} else /* go to previous list element in the same map element */
	    --j;
    } else {
	if ( datait == data.begin() )
	    return FALSE;
	--datait;
    }
    return TRUE;
}

Record& ResultSet::currentRecord()
{
    if ( !data.count() ) {
	qWarning("ResultSet::currentRecord: no data available!");
	return data[0];
    }
    if ( sortKey.count() ) {
	return data[ keyit.data()[j] ];
    } else {
	return *datait;
    }
}


static void reverse( ColumnKey& colkey, uint elements )
{
    if ( !colkey.count() || !elements )
	return;
    ColumnKey::Iterator asc = colkey.begin();
    int a = 0;
    ColumnKey::Iterator des = --colkey.end();
    int d = des.data().count()-1;
    for ( uint i = 0; i < elements/2; ++i ) {
	qSwap<int>( asc.data()[a], des.data()[d] );
	/* increment asc */
	if ( a+1 > (int)asc.data().count()-1 ) { /* go to next map element */
	    ++asc;
	    a = 0;
	} else /* go to next list element in the same map element */
	    ++a;
	/* decrement des */
	if ( d-1 < 0 ) { /* go to previous map element */
	    --des;
	    d = des.data().count()-1;
	} else /* go to previous list element in the same map element */
	    --d;
    }
}

/*!

*/

bool ResultSet::sort( const QSqlIndex* index )
{
    if ( !env ) {
	env->program().setLastError( "ResultSet: no environment" );
	return FALSE;
    }
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
    for ( i = 0; i < index->count(); ++i ) {
	int sortField = head.position( index->field( index->count()-1 )->name() );
	if ( sortField == -1 ) {
	    env->program().setLastError( "ResultSet: sort field not found:" +
					 index->field( index->count()-1 )->name() );
	    return FALSE;
	}
    }

    sortKey.clear();

    /* init the sort key */
    for ( i = 0; i < data.count(); ++i ) {
	int sortField = head.position( index->field( index->count()-1 )->name() );
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
    if ( index->count() > 1 ) {
	/* reverse logic below */
	if ( index->isDescending( index->count()-1 ) ) {
	    /* descending */
	    if ( !index->isDescending( index->count()-2 ) )
		reverse( sortKey, data.count() );
	} else {
	    /* ascending? */
	    if ( index->isDescending( index->count()-2 ) )
		reverse( sortKey, data.count() );
	}
    }
    if ( index->count() == 1 && index->isDescending( 0 ) )
	reverse( sortKey, data.count() );

    ColumnKey::Iterator it;
    if ( index->count() > 1 ) {
	/* sort rest of fields */
	for ( int idx = index->count()-2; idx >= 0; --idx ) {
	    int sortField = head.position( index->field(idx)->name() );
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
	    if ( idx > 0 ) {
		/* reverse logic below */
		if ( index->isDescending( idx ) ) {
		    /* descending */
		    if ( !index->isDescending( idx-1 ) )
			reverse( sortKey, data.count() );
		} else {
		    /* ascending? */
		    if ( index->isDescending( idx-1 ) )
			reverse( sortKey, data.count() );
		}
	    }
	    if ( idx == 0 && index->isDescending( idx ) )
		reverse( sortKey, data.count() );
	}
    }

#ifdef DEBUG_SQLINTERP
    qDebug("number of fields in result set:" + QString::number(head.count()) );
    qDebug("number of result records:" + QString::number( data.count() ) );

    if ( size() > 0 ) {
	qDebug("sorted results set:");
	first();
	do {
	    Record& rec = currentRecord();
	    for ( uint j = 0; j < rec.count(); ++j )
		cout << rec[j].toString().rightJustify(10).latin1();
	    cout << endl;
	} while ( next() );
    }

#endif // DEBUG_SQLINTERP

    return TRUE;
}
