/*
    Xbase project source code

    This file contains LocalSQL support class implementations

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

Program::Program()
{
    ops.setAutoDelete( TRUE );
    pendingLabel = 0;
    dirty = FALSE;
}



/*! Destroys the object and frees any allocated resources.

*/

Program::~Program()
{
}


void Program::appendLabel( int lab )
{
    pendingLabel = lab;
}

/*!  Appends \a op to the program listing.  The program takes
ownership of the pointer.

*/

void Program::append( localsql::Op* op )
{
    ops.append( op );
    dirty = TRUE;
    if ( pendingLabel != 0 ) {
	op->setLabel( pendingLabel );
	pendingLabel = 0;
    }
}




/*! Removes the operation at \a i.

*/

void Program::remove( uint i )
{
    ops.remove( i );
    dirty = TRUE;
}


/*! Removes all operations.

*/

void Program::clear()
{
    ops.clear();
    dirty = TRUE;
}


/*! sets the program counter so that \a i is the next instruction to
be executed. If \a i is negative, it is interpreted as a label.

*/

void Program::setCounter( int i )
{
    if ( i < 0 ) {
	if ( dirty ) {
	    int instrNo = 0;
	    localsql::Op *op = ops.first();
	    while ( op != 0 ) {
		if ( op->label() < 0 ) {
		    int n = -( op->label() + 1 );
		    if ( (int) counters.size() < n + 1 )
			counters.resize( n + 1 );
		    counters[n] = instrNo;
		}
		instrNo++;
		op = ops.next();
	    }
	    dirty = FALSE;
	}

	i = counters[-(i + 1)];
    }
    pc = i - 1;
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

localsql::Op* Program::next()
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
    localsql::Op* op = 0;
    while( (op = ((Program*)this)->next() ) ) {
	QString s;
	if ( op->label() != 0 )
	    s = QString::number( op->label() );
	s = s.rightJustify( 8 );
	s += QString::number( i ).rightJustify( 8 );
	s += op->name().rightJustify( 16 );
	s += asListing( op->P(0) ).rightJustify( 16 );
	s += asListing( op->P(1) ).rightJustify( 16 );
	s += asListing( op->P(2) ).rightJustify( 16 );
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

ResultSet::ResultSet( localsql::Environment* environment )
    : env( environment )
{
    head = new Header();
}

ResultSet::ResultSet( const ResultSet& other )
    : localsql::ResultSet()
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

QValueList<QVariant::Type> ResultSet::columnTypes() const
{
    QValueList<QVariant::Type> types;
    for ( uint i = 0; i < count(); ++i )
	types += head->fields[i].type;
    return types;
}


QStringList ResultSet::columnNames() const
{
    QStringList cols;
    for ( uint i = 0; i < head->fields.count(); ++i )
	cols += head->fields[i].name;
    return cols;
}

bool ResultSet::field( const QString& name, QVariant& v )
{
    for ( uint i = 0; i < head->fields.count(); ++i ) {
	if ( head->fields[i].name == name )
	    return field( i, v );
    }
    return FALSE;
}

bool ResultSet::field( uint i, QVariant& v )
{
    if ( i > count() ) {
	env->setLastError( "Unknown field number: " + QString::number(i) );
	return 0;
    }
    v = currentRecord()[i];
    return TRUE;
}

/*!

*/

bool ResultSet::setHeader( const localsql::List& list )
{
    if ( !list.count() ) {
	env->setLastError( "No fields defined" );
	return 0;
    }
    for ( int i = 0; i < (int)list.count(); ++i ) {
	localsql::List fieldDescription = list[i].toList();
	if ( fieldDescription.count() != 4 ) {
	    env->setLastError( "Internal error: Bad field description" );
	    return 0;
	}
	head->fields[i].name = fieldDescription[0].toString();
	head->fields[i].type = (QVariant::Type)fieldDescription[1].toInt();
    }
    return TRUE;
}

/*!

*/

bool ResultSet::append( const localsql::Record& buf )
{
    if ( !env ) {
	qWarning( "Fatal internal error: No environment" );
	return FALSE;
    }
    if ( !head->fields.count() ) {
	env->setLastError( "Internal error: No header" );
	return FALSE;
    }
    if ( head->fields.count() != buf.count() ) {
	env->setLastError( "Internal error: Incorrect number of buffer fields" );
	return FALSE;
    }
    for ( uint j = 0; j < buf.count(); ++j ) {
	if ( buf[j].type() != head->fields[j].type ) {
	    QVariant v;
	    v.cast( head->fields[j].type );
	    env->setLastError( "Incorrect field type: " +
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

localsql::Record& ResultSet::currentRecord()
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


static void reverse( localsql::ColumnKey& colkey, uint elements )
{
    if ( !colkey.count() || !elements )
	return;
    localsql::ColumnKey::Iterator asc = colkey.begin();
    int a = 0;
    localsql::ColumnKey::Iterator des = --colkey.end();
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

bool ResultSet::sort( const localsql::List& index )
{
    if ( !env ) {
	env->setLastError( "Internal error: No environment" );
	return FALSE;
    }
    if ( !head->fields.count() ) {
	env->setLastError( "Internal error: No header" );
	return FALSE;
    }
    if ( !data.count() || data.count() == 1 ) { /* nothing to do */
	return TRUE;
    }
    if ( !index.count() ) {
	env->setLastError("No fields defined");
	return 0;
    }
    if ( (index.count() % 2) != 0 ) {
	env->setLastError("Internal error: Incorrect multiple of list elements");
	return 0;
    }
    uint i = 0;
    QMap<int,bool> desc; /* indicates fields with a descending sort */
    Header sortIndex;
    for ( uint i = 0; i < index.count(); ++i ) {
	localsql::List fieldDescription = index[i].toList();
	if ( fieldDescription.count() != 4 ) {
	    env->setLastError("Internal error: Bad field description");
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
	    QVariant v;
	    v.cast( sortIndex.fields[i].type );
	    env->setLastError( "Internal error: Invalid sort field type" +
			       QString( v.typeName() ) );
	    return FALSE;
	}
	int sortField = head->position( sortIndex.fields[i].name );
	if ( sortField == -1 ) {
	    env->setLastError( "Field not found:" + sortIndex.fields[i].name );
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

    localsql::ColumnKey::Iterator it;
    if ( sortIndex.fields.count() > 1 ) {
	/* sort rest of fields */
	for ( int idx = sortIndex.fields.count()-2; idx >= 0; --idx ) {
	    int sortField = head->position( sortIndex.fields[idx].name );
	    localsql::ColumnKey subSort;
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
    return TRUE;
}
