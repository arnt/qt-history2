#include "qsqlfield.h"

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
    : nm(fieldName), num(fieldNumber), label(fieldName), ro(FALSE)
{
    val.cast( type );
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlField::~QSqlField()
{
}

/*!
  Returns a reference to the internal value of the field.  To determine if the field
  is null use isNull().  To determine if the field is read only, use isReadOnly().

  \sa isNull() setIsNull() isReadOnly() setReadOnly()

*/

QVariant& QSqlField::value()
{
    return val;
}

/*! \fn void setName( const QString& name )
  Sets the name of the field to \a name,
*/

/*! \fn QString name() const
  Returns the name of the field.
*/

/*! \fn void setDisplayLabel( const QString& l )
  Sets the display label text of the field to \a l.
*/

/*! \fn QString displayLabel() const
  Returns the display label of the field.
*/

/*! \fn void setFieldNumber( int fieldNumber )
  Sets the field number of the field to \a fieldNumber.
*/

/*! \fn int fieldNumber() const
  Returns the field number of the field.
*/

/*! \fn void setReadOnly( bool readOnly )
  Sets the read only flag of the field to \a readOnly.
*/

/*! \fn bool isReadOnly() const
  Returns TRUE if the field is read only, otherwise FALSE.
*/

/*! \fn void setIsNull( bool n )
  Sets the null flag of the field to \a null.
*/

/*! \fn bool isNull() const
  Returns TRUE if the field is currently null, otherwise FALSE.
*/

/*! \fn QVariant::Type type() const
  Returns the field type.
*/

//////////

/*!
    \class QSqlFieldList qsqlfield.h
    \brief Class used for manipulating a list of SQL database fields

    \module database
*/


/*!
  Constructs an empty field list.

*/

QSqlFieldList::QSqlFieldList()
{

}

/*!  Constructs a copy of \a l.

*/

QSqlFieldList::QSqlFieldList( const QSqlFieldList& l )
{
    fieldList = l.fieldList;
    fieldListStr = l.fieldListStr;
    posMap = l.posMap;
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
    return value( i );
}

/*!
  Returns a reference to the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlFieldList::operator[]( const QString& name )
{
    return value( name );
}

/*!
  Returns a reference to the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlFieldList::value( int i )
{
#ifdef CHECK_RANGE
    static QVariant dbg;

    if( (unsigned int) i > fieldList.count() ){
	qWarning( "QSqlFieldList warning: index out of range" );
	return dbg;
    }
#endif // CHECK_RANGE
    return fieldList[ i ].value();
}

/*!
  Returns a reference to the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlFieldList::value( const QString& name )
{
#ifdef CHECK_RANGE
    static QVariant dbg;

    if( (unsigned int) position( name ) > fieldList.count() ){
	qWarning( "QSqlFieldList warning: index out of range" );
	return dbg;
    }
#endif // CHECK_RANGE
    return fieldList[ position( name ) ].value();
}

/*!
  Returns a reference to the value of the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

QSqlField& QSqlFieldList::field( int i )
{
    return fieldList[ i ];
}

/*!
  Returns a reference to the value of the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QSqlField& QSqlFieldList::field( const QString& name )
{
    return fieldList[ position( name ) ];
}

/*!
  Returns the position of the field named \a name within the list,
  or -1 if it cannot be found.

*/

int QSqlFieldList::position( const QString& name )
{
    if ( posMap.contains( name ) )
	return posMap[ name ];
    return -1;
}

/*!
  Appends the field \a field to the end of the list of fields.

*/

void QSqlFieldList::append( const QSqlField& field )
{
    if ( fieldListStr.isNull() )
	fieldListStr = field.name();
    else
	fieldListStr += ", " + field.name();
    posMap[ field.name() ] = fieldList.count();
    fieldList.append( field );
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

QString QSqlFieldList::toString() const
{
    return fieldListStr;
}

/*!
  Returns the number of fields in the list.

*/

uint QSqlFieldList::count() const
{
    return fieldList.count();
}

#endif
