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
    if ( val.type() != QVariant::Invalid )
	setNull( FALSE );
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

/*! \fn void QSqlField::setNull( bool n )
  Sets the null flag of the field to \a n.
*/

/*! \fn bool QSqlField::isNull() const
  Returns TRUE if the field is currently null, otherwise FALSE.
*/

/*! \fn void QSqlField::setVisible( bool visible )
  Sets the visible flag of the field to \a visible.
*/

/*! \fn bool QSqlField::isVisible() const
  Returns TRUE if the field is visible when used in a GUI, otherwise FALSE.
*/

/*! \fn void QSqlField::setPrimaryIndex( bool primaryIndex )
  Sets the primary index flag to \a primaryIndex.
*/
/*! \fn bool QSqlField::isPrimaryIndex() const
  Returns TRUE if the field is part of a primary index, otherwise FALSE.
*/

#endif
