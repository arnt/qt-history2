#include <sqlinterpreter.h>
#include <qcleanuphandler.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtl.h>

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

void Program::append( qdb::Op* op )
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
    //## todo -- jasmin is this necessary?
    qWarning("Program::setCounter( const QString& label ): not yet implemented");
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

qdb::Op* Program::next()
{
    ++pc;
    if ( pc < (int)ops.count() )
	return ops.at( pc );
    return 0;
}

static QString asListing( const QVariant& v )
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
    case QVariant::Bool:
	s = v.toBool() ? "true" : "false";
	break;
    default:
	s = v.toString();
	break;
    }
    if ( s.isNull() )
	s = ".";
    return s;
}

/*! Returns the contents of the program as a string list.
*/

QStringList Program::listing() const
{
    ((Program*)this)->resetCounter();
    QStringList l;
    int i = 0;
    qdb::Op* op = 0;
    while( (op = ((Program*)this)->next() ) ) {
	QString s =  QString::number( i ).rightJustify(4) + op->name().rightJustify(15);
	s += asListing( op->P(0) ).rightJustify(15);
	s += asListing( op->P(1) ).rightJustify(15);
	s += asListing( op->P(2) ).rightJustify(15);
	l += s;
	++i;
    }
    ((Program*)this)->resetCounter();
    return l;
}


////////

struct ResultSetField
{
    QString name;
    QVariant::Type type;
};

class ResultSet::Header
{
public:
    QMap<int,ResultSetField> fields;
    int position( const QString& name )
    {
	for ( uint i = 0; i < fields.count(); ++i ) {
	    if ( fields[i].name == name )
		return i;
	}
	return -1;
    }

};

/*!  Constructs an empty

*/

ResultSet::ResultSet( qdb::Environment* environment )
    : env( environment )
{
    head = new Header();
}

ResultSet::ResultSet( const ResultSet& other )
    : qdb::ResultSet()
{
    *this = other;
}

ResultSet& ResultSet::operator=( const ResultSet& other )
{
    env = other.env;
    *head = *other.head;
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
    delete head;
}

void ResultSet::clear()
{
    data.clear();
    sortKey.clear();
    head->fields.clear();
}

uint ResultSet::count() const
{
    return head->fields.count();
}

QStringList ResultSet::columns() const
{
    QStringList cols;
    for ( uint i = 0; i < head->fields.count(); ++i )
	 cols += head->fields[i].name;
    return cols;
}

/*!

*/

bool ResultSet::setHeader( const qdb::List& list )
{
    if ( !list.count() ) {
	env->setLastError("internal error:ResultSet::setHeader: no fields defined");
	return 0;
    }
    for ( int i = 0; i < (int)list.count(); ++i ) {
	qdb::List fieldDescription = list[i].toList();
	if ( fieldDescription.count() != 4 ) {
	    env->setLastError("internal error:ResultSet::setHeader: bad field description");
	    return 0;
	}
	head->fields[i].name = fieldDescription[0].toString();
	head->fields[i].type = (QVariant::Type)fieldDescription[1].toInt();
    }
    return TRUE;
}

/*!

*/

bool ResultSet::append( const qdb::Record& buf )
{
    if ( !env ) {
	qWarning( "internal error:ResultSet: no environment" );
	return FALSE;
    }
    if ( !head->fields.count() ) {
	env->setLastError( "internal error:ResultSet: no header" );
	return FALSE;
    }
    if ( head->fields.count() != buf.count() ) {
	env->setLastError( "internal error:ResultSet: incorrect number of buffer fields" );
	return FALSE;
    }
    for ( uint j = 0; j < buf.count(); ++j ) {
	if ( buf[j].type() != head->fields[j].type ) {
	    QVariant v;
	    v.cast( head->fields[j].type );
	    env->setLastError( "internal error:ResultSet: incorrect field type: " +
			       QString( buf[j].typeName() ) + " (expected " +
			       QString( v.typeName() ) + ")" );
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

qdb::Record& ResultSet::currentRecord()
{
    if ( !data.count() ) {
	env->output() << "ResultSet::currentRecord: no data available";
	return data[0];
    }
    if ( sortKey.count() ) {
	return data[ keyit.data()[j] ];
    } else {
	return *datait;
    }
}


static void reverse( qdb::ColumnKey& colkey, uint elements )
{
    if ( !colkey.count() || !elements )
	return;
    qdb::ColumnKey::Iterator asc = colkey.begin();
    int a = 0;
    qdb::ColumnKey::Iterator des = --colkey.end();
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

bool ResultSet::sort( const qdb::List& index )
{
    qDebug("ResultSet::sort");
    if ( !env ) {
	env->setLastError( "internal error:ResultSet::sort: no environment" );
	return FALSE;
    }
    if ( !head->fields.count() ) {
	env->setLastError( "internal error:ResultSet::sort: no header" );
	return FALSE;
    }
    if ( !data.count() || data.count() == 1 ) { /* nothing to do */
	return TRUE;
    }
    if ( !index.count() ) {
	env->setLastError("internal error:ResultSet::sort: no fields defined");
	return 0;
    }
    if ( (index.count() % 2) != 0 ) {
	env->setLastError("internal error:ResultSet::sort: wrong multiple of list elements");
	return 0;
    }
    uint i = 0;
    QMap<int,bool> desc; /* indicates fields with a descending sort */
    Header sortIndex;
    for ( uint i = 0; i < index.count(); ++i ) {
	qdb::List fieldDescription = index[i].toList();
	if ( fieldDescription.count() != 4 ) {
	    env->setLastError("internal error:ResultSet::sort: bad field description");
	    return 0;
	}
	sortIndex.fields[i].name = fieldDescription[0].toString();
	sortIndex.fields[i].type = fieldDescription[1].type();
	desc[i] = index[++i].toBool();
	switch ( sortIndex.fields[i].type ) {
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
	    env->setLastError( "internal error:ResultSet::sort: invalid sort field type" );
	    return FALSE;
	}
	int sortField = head->position( sortIndex.fields[i].name );
	if ( sortField == -1 ) {
	    env->setLastError( "internal error:ResultSet: sort field not found:" +
			       sortIndex.fields[i].name );
	    return FALSE;
	}
    }

    sortKey.clear();

    /* init the sort key */
    for ( i = 0; i < data.count(); ++i ) {
	int sortField = head->position( sortIndex.fields[sortIndex.fields.count()-1].name );
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
    if ( sortIndex.fields.count() > 1 ) {
	/* reverse logic below */
	if ( desc[ sortIndex.fields.count()-1 ] ) {
	    /* descending */
	    if ( desc[ sortIndex.fields.count()-2 ] )
		reverse( sortKey, data.count() );
	} else {
	    /* ascending? */
	    if ( desc[ sortIndex.fields.count()-2 ] )
		reverse( sortKey, data.count() );
	}
    }
    if ( sortIndex.fields.count() == 1 && desc[ 0 ] )
	reverse( sortKey, data.count() );

    qdb::ColumnKey::Iterator it;
    if ( sortIndex.fields.count() > 1 ) {
	/* sort rest of fields */
	for ( int idx = sortIndex.fields.count()-2; idx >= 0; --idx ) {
	    int sortField = head->position( sortIndex.fields[idx].name );
	    qdb::ColumnKey subSort;
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
		if ( desc[ idx ] ) {
		    /* descending */
		    if ( desc[ idx-1 ] )
			reverse( sortKey, data.count() );
		} else {
		    /* ascending? */
		    if ( desc[ idx-1 ] )
			reverse( sortKey, data.count() );
		}
	    }
	    if ( idx == 0 && desc[ idx ] )
		reverse( sortKey, data.count() );
	}
    }

#if 0
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

#endif

    return TRUE;
}
