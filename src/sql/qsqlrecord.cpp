#include "qsqlrecord.h"
#include "qregexp.h"


#ifndef QT_NO_SQL

/*!
    \class QSqlRecord qsqlfield.h
    \brief Template class used for manipulating a list of SQL database fields.

    \module database
*/


/*!
  Constructs an empty record.

*/

QSqlRecord::QSqlRecord()
{

}

/*!  Constructs a copy of \a other.

*/

QSqlRecord::QSqlRecord( const QSqlRecord& other )
    : fieldList( other.fieldList )
{

}

QSqlRecord& QSqlRecord::operator=( const QSqlRecord& other )
{
    fieldList = other.fieldList;
    return *this;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlRecord::~QSqlRecord()
{

}


/*!
  Returns a reference to the field located at position \a i in the record.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlRecord::operator[]( int i )
{
    return findField(i)->val;
}

/*!
  Returns a reference to the field named \a name in the record.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlRecord::operator[]( const QString& name )
{
    return findField( name )->val;
}

/*!
  Returns the value of the field located at position \a i in the record.
  It is up to you to check wether this item really exists.

*/

QVariant  QSqlRecord::value( int i )
{
    return findField(i)->val;
}

/*!
  Returns the value of the field named \a name in the record.
  It is up to you to check wether this item really exists.

*/

QVariant  QSqlRecord::value( const QString& name )
{
    return findField( name )->val;
}

/*!
  Returns the position of the field named \a name within the record,
  or -1 if it cannot be found.

*/

int QSqlRecord::position( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( field( i )->name() == name )
	    return i;
    }
    return -1;
}

QSqlField* QSqlRecord::field( int i )
{
    return &fieldList[ i ];
}

const QSqlField* QSqlRecord::field( int i ) const
{
    return &fieldList[ i ];
}

QSqlField* QSqlRecord::field( const QString& name )
{
    return &fieldList[ position( name ) ];
}

const QSqlField* QSqlRecord::field( const QString& name ) const
{
    return &fieldList[ position( name ) ];
}


/*!
  Appends a copy of the field \a field to the end of the record.

*/

void QSqlRecord::append( const QSqlField& field )
{
    fieldList.append( field );
}

/*!
  Prepends a copy of \a field to the beginning of the record.

*/

void QSqlRecord::prepend( const QSqlField& field )
{
    fieldList.prepend( field );

}

/*!  Inserts a copy of \a field before \a pos.  If \a pos does not
  exist, it is appended to the end of the record.

*/

void QSqlRecord::insert( int pos, const QSqlField& field )
{
    fieldList.insert( fieldList.at( pos ), field );
}

/*!  Removes all the field at \a pos.  If \a pos does not exist,
  nothing happens.

*/

void QSqlRecord::remove( int pos )
{
    fieldList.remove( fieldList.at( pos ) );
}

/*!
  Removes all fields from the record.

*/

void QSqlRecord::clear()
{
    fieldList.clear();
}

/*!
  Clears the value of all fields in the record.

*/

void QSqlRecord::clearValues()
{
    for ( uint i = 0; i < count(); ++i ) {
	QVariant v;
	v.cast( field( i )->type() );
	field( i )->setValue( v );
    }
}

/*!  Returns a comma-separated list of field names as a string.  This
  string is suitable for use in, for example, generating a select
  statement.  If a \a prefix is specified, it is prepended before all
  field names.

*/

QString QSqlRecord::toString( const QString& prefix ) const
{
    QString pflist;
    QString pfix =  prefix.isNull() ? QString::null : prefix + ".";
    for ( uint i = 0; i < count(); ++i ){
	pflist += pfix + field( i )->name();
	if( i != (count() - 1) )
	    pflist += ", ";
    }
    return pflist;
}


/*!
  Returns the number of fields in the record.

*/

uint QSqlRecord::count() const
{
    return fieldList.count();
}

/*!
  \internal

*/

QSqlField* QSqlRecord::findField( int i )
{
#ifdef QT_CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) i > fieldList.count() ){
	qWarning( "QSqlRecord warning: index out of range" );
	return &dbg;
    }
#endif // QT_CHECK_RANGE
    return &fieldList[ i ];
}

/*!
  \internal

*/

QSqlField* QSqlRecord::findField( const QString& name )
{
#ifdef QT_CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) position( name ) > fieldList.count() ){
	qWarning( "QSqlRecord warning: index out of range" );
	return &dbg;
    }
#endif // QT_CHECK_RANGE
    return &fieldList[ position( name ) ];
}

/*! Sets the value of the field at position \a i with \a val.  If the field does
  not exist, nothing happens.

*/

void QSqlRecord::setValue( int i, const QVariant& val )
{
    QSqlField* f = findField( i );
    if ( f )
	f->setValue( val );
}


/*!  Sets the value of field \a name with \a val.  If the field does
  not exist, nothing happens.
*/

void QSqlRecord::setValue( const QString& name, const QVariant& val )
{
    QSqlField* f = findField( name );
    if ( f )
	f->setValue( val );
}

#endif
