#include "qsqlfield.h"
#include "qregexp.h"

#ifndef QT_NO_SQL

/*!
    \class QSqlField qsqlfield.h
    \brief Class used for manipulating SQL database fields

    \module database
*/


/*!
  Constructs an empty SQL field using the field name \a fieldName
  and field number \a fieldNumber.  By default, the fields displayLabel()
  uses \a fieldName.

  \sa setDisplayLabel() setReadOnly()

*/

QSqlField::QSqlField( const QString& fieldName, int fieldNumber, QVariant::Type type )
    : nm(fieldName), num(fieldNumber), label(fieldName), ro(FALSE), nul(FALSE), pIdx(FALSE), iv(TRUE), cf(FALSE)
{
    val.cast( type );
}

QSqlField::QSqlField( const QSqlField& other )
    : nm( other.nm ), num( other.num ), val( other.val ), label( other.label ), ro( other.ro ), nul( other.nul ), pIdx( other.pIdx ), iv( other.iv ), cf( other.cf )
{
}

QSqlField& QSqlField::operator=( const QSqlField& other )
{
    nm = other.nm;
    num = other.num;
    val = other.val;
    label = other.label;
    ro = other.ro;
    nul = other.nul;
    pIdx = other.pIdx;
    iv = other.iv;
    cf = other.cf;
    return *this;
}

bool QSqlField::operator==(const QSqlField& other) const
{
    return ( nm == other.nm &&
	     num == other.num &&
	     val == other.val &&
	     label == other.label &&
	     ro == other.ro &&
	     nul == other.nul &&
	     pIdx == other.pIdx &&
	     iv == other.iv &&
	     cf == other.cf );
}


/*!
  Destroys the object and frees any allocated resources.

*/

QSqlField::~QSqlField()
{
}


/*!
  Returns the internal value of the field.

*/

QVariant QSqlField::value() const
{
    return val;
}

/*!
  Sets the internal value of the field to \a value.

*/
void QSqlField::setValue( const QVariant& value )
{
    val = value;
    setIsNull( FALSE );
}

/*! \fn void QSqlField::clear()
  Clears the value of the field.
*/

void QSqlField::clear()
{
    QVariant v;	
    v.cast( type() );
    setValue( v );
}

/*! \fn void QSqlField::setName( const QString& name )
  Sets the name of the field to \a name,
*/

/*! \fn QString QSqlField::name() const
  Returns the name of the field.
*/

/*! \fn void QSqlField::setFieldNumber( int fieldNumber )
  Sets the field number of the field to \a fieldNumber.
*/

/*! \fn int QSqlField::fieldNumber() const
  Returns the field number of the field.
*/

/*! \fn QVariant::Type QSqlField::type() const
  Returns the field type.
*/

/*! \fn void QSqlField::setDisplayLabel( const QString& l )
  Sets the display label text of the field to \a l.
*/

/*! \fn QString QSqlField::displayLabel() const
  Returns the display label of the field.
*/

/*! \fn void QSqlField::setReadOnly( bool readOnly )
  Sets the read only flag of the field to \a readOnly.
*/

/*! \fn bool QSqlField::isReadOnly() const
  Returns TRUE if the field is read only, otherwise FALSE.
*/

/*! \fn void QSqlField::setIsNull( bool n )
  Sets the null flag of the field to \a null.
*/

/*! \fn bool QSqlField::isNull() const
  Returns TRUE if the field is currently null, otherwise FALSE.
*/

/*! \fn void QSqlField::setPrimaryIndex( bool primaryIndex )
  Sets the primary index flag to \a primaryIndex.
*/
/*! \fn bool QSqlField::isPrimaryIndex() const
  Returns TRUE if the field is part of a primary index, otherwise FALSE.
*/

/////////////////

/*!
    \class QSqlFieldList qsqlfield.h
    \brief Template class used for manipulating a list of SQL database fields.

    \module database
*/


/*!
  Constructs an empty field list.

*/

QSqlFieldList::QSqlFieldList()
{

}

/*!  Constructs a copy of \a other.

*/

QSqlFieldList::QSqlFieldList( const QSqlFieldList& other )
    : fieldList( other.fieldList )
{

}

QSqlFieldList& QSqlFieldList::operator=( const QSqlFieldList& other )
{
    fieldList = other.fieldList;
    return *this;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlFieldList::~QSqlFieldList()
{

}


/*!
  Returns a reference to the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlFieldList::operator[]( int i )
{
    return findField(i)->val;
}

/*!
  Returns a reference to the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlFieldList::operator[]( const QString& name )
{
    return findField( name )->val;
}

/*!
  Returns the value of the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

QVariant  QSqlFieldList::value( int i )
{
    return findField(i)->val;
}

/*!
  Returns the value of the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QVariant  QSqlFieldList::value( const QString& name )
{
    return findField( name )->val;
}

/*!
  Returns the position of the field named \a name within the list,
  or -1 if it cannot be found.

*/

int QSqlFieldList::position( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( field( i )->name() == name )
	    return i;
    }
    return -1;
}

QSqlField* QSqlFieldList::field( int i )
{
    return &fieldList[ i ];
}

const QSqlField* QSqlFieldList::field( int i ) const
{
    return &fieldList[ i ];
}

QSqlField* QSqlFieldList::field( const QString& name )
{
    return &fieldList[ position( name ) ];
}

const QSqlField* QSqlFieldList::field( const QString& name ) const
{
    return &fieldList[ position( name ) ];
}


/*!
  Appends a copy of the field \a field to the end of the list of fields.

*/

void QSqlFieldList::append( const QSqlField* field )
{
    fieldList.append( *field );
}

/*!
  Prepends a copy of \a field to the beginning of the list.

*/

void QSqlFieldList::prepend( const QSqlField* field )
{
    fieldList.prepend( *field );

}

/*!  Inserts a copy of \a field before \a pos.  If \a pos does not
  exist, it is appended to the end of the list.

*/

void QSqlFieldList::insert( int pos, const QSqlField* field )
{
    fieldList.insert( fieldList.at( pos ), *field );
}

/*!  Removes all the field at \a pos.  If \a pos does not exist,
  nothing happens.

*/

void QSqlFieldList::remove( int pos )
{
    fieldList.remove( fieldList.at( pos ) );
}

/*!
  Removes all fields from the list.

*/

void QSqlFieldList::clear()
{
    fieldList.clear();
}

/*!
  Clears the value of all fields in the list.

*/

void QSqlFieldList::clearValues()
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

QString QSqlFieldList::toString( const QString& prefix ) const
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
  Returns the number of fields in the list.

*/

uint QSqlFieldList::count() const
{
    return fieldList.count();
}

/*!
  \internal

*/

QSqlField* QSqlFieldList::findField( int i )
{
#ifdef CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) i > fieldList.count() ){
	qWarning( "QSqlFields warning: index out of range" );
	return &dbg;
    }
#endif // CHECK_RANGE
    return &fieldList[ i ];
}

/*!
  \internal

*/

QSqlField* QSqlFieldList::findField( const QString& name )
{
#ifdef CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) position( name ) > fieldList.count() ){
	qWarning( "QSqlFields warning: index out of range" );
	return &dbg;
    }
#endif // CHECK_RANGE
    return &fieldList[ position( name ) ];
}

/*! Sets the value of the field at position \a i with \a val.  If the field does
  not exist, nothing happens.

*/

void QSqlFieldList::setValue( int i, const QVariant& val )
{
    QSqlField* f = findField( i );
    if ( f )
	f->setValue( val );
}


/*!  Sets the value of field \a name with \a val.  If the field does
  not exist, nothing happens.
*/

void QSqlFieldList::setValue( const QString& name, const QVariant& val )
{
    QSqlField* f = findField( name );
    if ( f )
	f->setValue( val );
}

#endif
