#include "qsqlfield.h"

#ifndef QT_NO_SQL

/*!
    \class QSqlResultField qsqlfield.h
    \brief Base class for manipulating SQL field information.

    \module database
*/


/*!
  Constructs an empty SQL field using the field name \a fieldName
  and field number \a fieldNumber.

*/

QSqlResultField::QSqlResultField( const QString& fieldName, int fieldNumber, QVariant::Type type )
    : nm(fieldName), num(fieldNumber)
{
    val.cast( type );
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlResultField::~QSqlResultField()
{

}

/*!
  Returns a reference to the internal value of the field.

*/

QVariant& QSqlResultField::value()
{
    return val;
}

/*! \fn void QSqlResultField::setName( const QString& name )
  Sets the name of the field to \a name,
*/

/*! \fn QString QSqlResultField::name() const
  Returns the name of the field.
*/

/*! \fn void QSqlResultField::setFieldNumber( int fieldNumber )
  Sets the field number of the field to \a fieldNumber.
*/

/*! \fn int QSqlResultField::fieldNumber() const
  Returns the field number of the field.
*/

/*! \fn QVariant::Type QSqlResultField::type() const
  Returns the field type.
*/

/////////////////////////


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
    : QSqlResultField( fieldName, fieldNumber, type ), label(fieldName), ro(FALSE), nul(FALSE), pIdx(FALSE)
{

}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlField::~QSqlField()
{
}


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


/*!  Constructs a copy of \a l.

*/

//template < class T > QSqlFields< T >::QSqlFields< T >( const QSqlFields< T >& l )

/*!
  Destroys the object and frees any allocated resources.

*/

//template < class T > QSqlFields< T >::~QSqlFields< T >()

/*!
  Returns a reference to the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

//template < class T > QVariant& QSqlFields< T >::operator[]( int i )

/*!
  Returns a reference to the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

//template < class T > QVariant& QSqlFields< T >::operator[]( const QString& name )

/*!
  Returns a reference to the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

//template < class T > QVariant& QSqlFields< T >::value( int i )

/*!
  Returns a reference to the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

//template < class T > QVariant& QSqlFields< T >::value( const QString& name )

/*!
  Returns a reference to the value of the field located at position \a i in the list.
  It is up to you to check wether this item really exists.

*/

//template < class T > T& QSqlFields< T >::field( int i )

/*!
  Returns a reference to the value of the field named \a name in the list.
  It is up to you to check wether this item really exists.

*/

//template < class T > T& QSqlFields< T >::field( const QString& name )

/*!
  Returns the position of the field named \a name within the list,
  or -1 if it cannot be found.

*/

//template < class T > int QSqlFields< T >::position( const QString& name )

/*!
  Appends the field \a field to the end of the list of fields.

*/

//template < class T > void QSqlFields< T >::append( const T& field )

/*!
  Removes all fields from the list.

*/

//template < class T > void QSqlFields< T >::clear()

/*!
  Returns a comma-separated list of field names as a string.  This
  string is suitable for use in, for example, generating a select
  statement.

*/

//template < class T > QString QSqlFields< T >::toString() const

/*!
  Returns the number of fields in the list.

*/

//template < class T > uint QSqlFields< T >::count() const

#endif
