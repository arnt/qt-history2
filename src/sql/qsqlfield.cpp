/****************************************************************************
**
** Implementation of QSqlField class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

class QSqlFieldPrivate
{
public:
};

/*!
    \class QSqlField qsqlfield.h
    \brief The QSqlField class manipulates the fields in SQL database tables
    and views.

    \module sql

    QSqlField represents the characteristics of a single column in a
    database table or view, such as the data type and column name. A
    field also contains the value of the database column, which can be
    viewed or changed.

    Field data values are stored as QVariants.  Using
    an incompatible type is not permitted.  For example:

    \code
    QSqlField f( "myfield", QVariant::Int );
    f.setValue( QPixmap() );  // will not work
    \endcode

    However, the field will attempt to cast certain data types to the
    field data type where possible:

    \code
    QSqlField f( "myfield", QVariant::Int );
    f.setValue( QString("123") ); // casts QString to int
    \endcode

    QSqlField objects are rarely created explicitly in application
    code.  They are usually accessed indirectly through \l QSqlRecord or
    \l QSqlCursor which already contain a list of fields.  For example:

    \code
    QSqlCursor cur( "Employee" );        // create cursor using the 'Employee' table
    QSqlField* f = cur.field( "name" );  // use the 'name' field
    f->setValue( "Dave" );               // set field value
    ...
    \endcode

    In practice we rarely need to extract a pointer to a field at all.
    The previous example would normally be written:

    \code
    QSqlCursor cur( "Employee" );
    cur.setValue( "name", "Dave" );
    ...
    \endcode

*/

/*!  Constructs an empty field called \a fieldName of type \a
  type.

*/

QSqlField::QSqlField( const QString& fieldName, QVariant::Type type )
    : nm(fieldName), ro(FALSE), nul(FALSE)
{
    d = 0; //new QSqlFieldPrivate();
    val.cast( type );
}

/*! Constructs a copy of \a other.
*/

QSqlField::QSqlField( const QSqlField& other )
    : nm( other.nm ), val( other.val ), ro( other.ro ), nul( other.nul )
{
}

/*! Sets the field equal to \a other.
*/

QSqlField& QSqlField::operator=( const QSqlField& other )
{
    nm = other.nm;
    val = other.val;
    ro = other.ro;
    nul = other.nul;
    return *this;
}

/*! Returns TRUE if the field is equal to \a other, otherwise returns FALSE.
  Fields are considered equal when the following field properties are
  the same:

  <ul>
  <li> \c name()
  <li> \c isNull()
  <li> \c value()
  <li> \c isReadOnly()
  </ul>

*/
bool QSqlField::operator==(const QSqlField& other) const
{
    return ( nm == other.nm &&
	     val == other.val &&
	     ro == other.ro &&
	     nul == other.nul );
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlField::~QSqlField()
{
    //    delete d;
}


/*! \fn QVariant QSqlField::value() const

  Returns the internal value of the field as a QVariant.

*/

/*!  Sets the value of the field to \a value. If the field is
  read-only (isReadOnly() returns TRUE), nothing happens.  If the data
  type of \a value differs from the field's current data type, an
  attempt is made to cast it to the proper type.  This preserves the
  data type of the field in the case of assignment, e.g. a QString to
  an integer data type.  For example:

  \code
  QSqlCursor cur( "Employee" );                 // 'Employee' table
  QSqlField* f = cur.field( "student_count" );	// an integer field
  ...
  f->setValue( myLineEdit->text() );		// cast the line edit text to an integer
  \endcode

  \sa isReadOnly()

*/
void QSqlField::setValue( const QVariant& value )
{
    if ( isReadOnly() )
	return;
    if ( value.type() != val.type() ) {
	if ( !val.canCast( value.type() ) )
	     qWarning("QSqlField::setValue: %s cannot cast from %s to %s",
		      nm.local8Bit().data(), value.typeName(), val.typeName() );
	QVariant tmp = value;
	tmp.cast( val.type() );
	val = tmp;
    } else
	val = value;
    if ( val.type() != QVariant::Invalid )
	setNull( FALSE );
}

/*!  Clears the value of the field.  If the field is read-only, nothing
  happens.  If \a nullify is TRUE (the default), the field is
  set to NULL.
*/

void QSqlField::clear( bool nullify )
{
    if ( isReadOnly() )
	return;
    QVariant v;
    v.cast( type() );
    val = v;
    if ( nullify )
	nul = TRUE;
}

/*! \fn void QSqlField::setName( const QString& name )

  Sets the name of the field to \a name.
*/

/*! \fn QString QSqlField::name() const

  Returns the name of the field.
*/

/*! \fn QVariant::Type QSqlField::type() const

  Returns the field's type.
*/

/*! \fn void QSqlField::setReadOnly( bool readOnly )

  Sets the read only flag of the field's value to \a readOnly.

  \sa setValue()
*/

/*! \fn bool QSqlField::isReadOnly() const

  Returns TRUE if the field's value is read only, otherwise FALSE.
*/

/*! \fn void QSqlField::setNull( bool n )

  Sets the null flag of the field to \a n.  If the field is read-only,
  nothing happens.  If \a n is TRUE, the field is also cleared with
  clear().

  \sa isReadOnly()
*/

/*! \fn bool QSqlField::isNull() const

  Returns TRUE if the field is currently null, otherwise returns FALSE.
*/

#endif
