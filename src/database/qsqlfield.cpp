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
    : QValueList<QSqlField>( l )
{
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
    return QValueList<QSqlField>::operator[](i).value();
}

/*!
  Returns a reference to the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QVariant& QSqlFieldList::operator[]( const QString& name )
{
    return QValueList<QSqlField>::operator[]( position( name ) ).value();
}

/*!
  Returns a reference to the value of the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

QSqlField& QSqlFieldList::field( int i )
{
    return QValueList<QSqlField>::operator[](i);
}

/*!
  Returns a reference to the value of the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

QSqlField& QSqlFieldList::field( const QString& name )
{
    return QValueList<QSqlField>::operator[]( position( name ) );
}

/*!
  Returns the position of the field named \a name within the list,
  or -1 if it cannot be found.

*/

int QSqlFieldList::position( const QString& name )
{
    for (uint i = 0; i < count(); ++i ) {
	if ( (*at(i)).name() == name )
	    return i;
    }
    return -1;
}

#endif
