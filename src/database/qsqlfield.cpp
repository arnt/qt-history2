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
  uses \a fieldName.  The field is not read only.
  
  \sa setDisplayLabel() setReadOnly()

*/

QSqlField::QSqlField( const QString& fieldName, int fieldNumber )
    : nm(fieldName), num(fieldNumber), label(fieldName), ro(FALSE)
{
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlField::~QSqlField()
{
}

/*!
  If isReadOnly() is FALSE, this method sets the internal value
  of the field to \a v.  Otherwise, this method has no effect.

*/

void QSqlField::setValue( const QVariant& v )
{
    if ( !ro )
	val = v;
}

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

/*! 
  Destroys the object and frees any allocated resources.

*/

QSqlFieldList::~QSqlFieldList()
{
    
}

/*! 
  Returns a reference to a field located at position \a i in the list.  
  It is up to you to check wether this item really exists.

*/

QSqlField& QSqlFieldList::operator[]( int i )
{
    return QValueList<QSqlField>::operator[](i);
}

/*! 
  Returns a reference to a field named \a name in the list.  
  It is up to you to check wether this item really exists.

*/

QSqlField& QSqlFieldList::operator[]( const QString& name )
{
    return QValueList<QSqlField>::operator[]( position( name ) );
}

#endif


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
