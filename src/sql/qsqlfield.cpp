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
    \class QSqlFields qsqlfield.h
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
    : fieldList( other.fieldList ), fieldListStr( other.fieldListStr ), posMap( other.posMap )
{

}

//QSqlFieldList::QSqlFieldList( const QSqlField& t )
//{
//    append( t );
//}

QSqlFieldList& QSqlFieldList::operator=( const QSqlFieldList& other )
{
    fieldList = other.fieldList;
    fieldListStr = other.fieldListStr;
    posMap = other.posMap;
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
    if ( posMap.contains( name ) )
	return posMap[ name ];
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
    if ( fieldListStr.isNull() )
	fieldListStr = field->name();
    else
	fieldListStr += ", " + field->name();
    posMap[ field->name() ] = fieldList.count();
    fieldList.append( *field );
}

/*!
  Removes all fields from the list.

*/

void QSqlFieldList::clear()
{
    fieldListStr = QString::null;
    fieldList.clear();
    posMap.clear();
}

/*!
  Returns a comma-separated list of field names as a string.  This
  string is suitable for use in, for example, generating a select
  statement.

*/

QString QSqlFieldList::toString( const QString& prefix ) const
{
    if ( prefix.isNull() )
	return fieldListStr;
    QString pfix =  prefix + ".";
    QString pflist = fieldListStr;
    pflist = pfix + pflist.replace( QRegExp(", "), QString(", ") + pfix );
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

#endif
