/****************************************************************************
**
** Implementation of QSqlField class
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
    switch( type ) {
    case QVariant::String:
    case QVariant::CString:
	af = Qt::AlignLeft;
	break;
    default:
	af = Qt::AlignRight;
	break;
    }
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

/*!  Sets the value of the field to \a value. If the data type of \a
  value differs from the field's current data type, an attempt is made
  to cast it to the proper type.  This preserves the data type of the
  field in the face of assigning, e.g. a QString to an integer data
  type.  For example:

  QSqlCursor myCursor( "classroom" );                     // classroom table
  QSqlField* myField = myCursor.field( "student_count" ); // an integer field
  ...
  myField->setValue( myLineEdit->text() ); // cast the line edit to an integer

*/
void QSqlField::setValue( const QVariant& value )
{
    if ( value.type() != val.type() ) {
	if ( !val.canCast( value.type() ) )
	     qWarning("QSqlField::setValue: cannot cast from %s to %s", value.typeName(), val.typeName() );
	QVariant tmp = value;
	tmp.cast( val.type() );
	val = tmp;
    } else
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
