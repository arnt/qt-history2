/****************************************************************************
**
** Implementation of QSqlRecord class
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

#include "qsqlrecord.h"

#ifndef QT_NO_SQL

#include "qregexp.h"
#include "qmap.h"
#include "qnamespace.h"

class QSqlRecordPrivate
{
public:
    struct info {
	info() : nogen(FALSE),align(0),label(),visible(TRUE){}
	bool    nogen;
	int     align;
	QString label;
	bool    visible;
    };

    QSqlRecordPrivate() {}
    QSqlRecordPrivate( const QSqlRecordPrivate& other )
	: fieldList( other.fieldList ),
	  fieldInfo( other.fieldInfo )
    {
    }
    QSqlRecordPrivate& operator=( const QSqlRecordPrivate& other )
    {
	fieldList = other.fieldList;
	fieldInfo = other.fieldInfo;
	return *this;
    }
    QValueList< QSqlField > fieldList;
    QMap< int, info > fieldInfo;
};

/*!
    \class QSqlRecord qsqlfield.h
    \brief A set of database fields.

    \module sql

    The QSqlRecord class encapsulates the functionality and
    characteristics of a database record (usually a table or view
    within the database), such as adding or removing fields, setting
    and retrieving field values, etc.  In addition, there are several
    functions which alter other characteristics of the record, for
    example, changing the display label associated with a particular
    field when displaying that field on screen.

*/


/*!
  Constructs an empty record.

*/

QSqlRecord::QSqlRecord()
{
    init();
}

/*!  Constructs a copy of \a other.

*/

QSqlRecord::QSqlRecord( const QSqlRecord& other )
{
    init();
    *d = *other.d;
}

/*! \internal
*/
void QSqlRecord::init()
{
    d = new QSqlRecordPrivate();
}

/*! Sets the record equal to \a other.
*/

QSqlRecord& QSqlRecord::operator=( const QSqlRecord& other )
{
    *d = *other.d;
    return *this;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlRecord::~QSqlRecord()
{
    delete d;
}

/*!  Returns the value of the field located at position \a i in the
  record.  It is up to you to check wether this item really exists.

*/

QVariant  QSqlRecord::value( int i ) const
{
    return findField(i)->value();
}

/*!  Returns the value of the field named \a name in the record.  It
  is up to you to check wether this item really exists.

*/

QVariant  QSqlRecord::value( const QString& name ) const
{
    return findField( name )->value();
}

/*!  Returns the position of the field named \a name within the
  record, or -1 if it cannot be found.  Field names are not
  case-sensitive.

*/

int QSqlRecord::position( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( field( i )->name().upper() == name.upper() )
	    return i;
    }
#ifdef QT_CHECK_RANGE
    qWarning( "QSqlRecord::position: unable to find field " + name );
#endif
    return -1;
}

/*!  Returns a pointer to the field at position \a pos within the
  record, or 0 if it cannot be found.

*/

QSqlField* QSqlRecord::field( int i ) const
{
    return findField( i );
}

/*!  Returns a pointer to the field with name \a name within the
  record, or 0 if it cannot be found.  Field names are not
  case-sensitive.

*/

QSqlField* QSqlRecord::field( const QString& name ) const
{
    return findField( name );
}


/*!  Appends a copy of the field \a field to the end of the record.

*/

void QSqlRecord::append( const QSqlField& field )
{
    d->fieldList.append( field );
}

/*!  Prepends a copy of \a field to the beginning of the record.

*/

void QSqlRecord::prepend( const QSqlField& field )
{
    d->fieldList.prepend( field );

}

/*!  Inserts a copy of \a field before \a pos.  If \a pos does not
  exist, it is appended to the end of the record.

*/

void QSqlRecord::insert( int pos, const QSqlField& field )
{
    d->fieldList.insert( d->fieldList.at( pos ), field );
}

/*!  Removes the field at \a pos.  If \a pos does not exist, nothing
  happens.

*/

void QSqlRecord::remove( int pos )
{
    d->fieldList.remove( d->fieldList.at( pos ) );
}

/*!  Removes all fields from the record.

  \sa clearValues()
*/

void QSqlRecord::clear()
{
    d->fieldList.clear();
    d->fieldInfo.clear();
}

/*!  Returns TRUE if there are no fields in the record, otherwise
  FALSE is returned.

*/

bool QSqlRecord::isEmpty() const
{
    return d->fieldList.isEmpty();
}

/*!  Clears the value of all fields in the record.  If \a nullify is
  TRUE, each field is set to null.

*/

void QSqlRecord::clearValues( bool nullify )
{
    for ( uint i = 0; i < count(); ++i ) {
	QVariant v;
	v.cast( field( i )->type() );
	field( i )->setValue( v );
	if ( nullify )
	    field( i )->setNull( TRUE );
    }
}

/*! Sets the generated flag for field \a name to \a generated.  If the
  field does not exist, nothing happens.

  \sa isGenerated()
*/

void QSqlRecord::setGenerated( const QString& name, bool generated )
{
    if ( !field( name ) )
	return;
    d->fieldInfo[ position( name ) ].nogen = !generated;
}

/*! Returns TRUE if the field \a name is to be generated (the
  default), otherwise FALSE is returned.  If the field does not exist,
  FALSE is returned.

  \sa setGenerated()
*/

bool QSqlRecord::isGenerated( const QString& name ) const
{
    if ( !field( name ) )
	return FALSE;
    return !d->fieldInfo[ position( name ) ].nogen;
}

/*! Sets the alignment of field \a name to \a align (which is of type
  Qt::AlignmentFlags).  If the field does not exist, nothing happens.

  \sa alignment()

*/
void QSqlRecord::setAlignment( const QString& name, int align )
{
    if ( !field( name ) )
	return;
    d->fieldInfo[ position( name ) ].align = align;
}

/*! Returns the alignment associated with the field \a name.  If the
   field does not exist, \c Qt::AlignLeft is returned.  If the field
   \a name has not been assigned an alignment (using setAlignment()),
   then the following rules are used:

   <ul>
   <li> If the field is a string data type, \c Qt::AlignLeft is returned.

   <li> Otherwise, \c Qt::AlignRight is returned.

   </ul>

   \sa setAlignment()
*/

int QSqlRecord::alignment( const QString& name ) const
{
    if ( !field( name ) )
	return Qt::AlignLeft;

    if ( !d->fieldInfo.contains( position( name ) ) ) {
	 int af = 0;
	 switch( field( name )->type() ) {
	 case QVariant::String:
	 case QVariant::CString:
	     af = Qt::AlignLeft;
	     break;
	 default:
	     af = Qt::AlignRight;
	     break;
	 }
	 return af;
    }
    return d->fieldInfo[ position( name ) ].align;
}

/*! Sets the display label of field \a name to \a label.  If the field
  does not exist, nothing happens.

  \sa displayLabel()
*/

void QSqlRecord::setDisplayLabel( const QString& name, const QString& label )
{
    if ( !field( name ) )
	return;
    d->fieldInfo[ position( name ) ].label = label;
}

/*! Returns the display label associated with the field \a name.  If
   the field does not exist, \a name is returned.

   \sa setDisplayLabel()
*/

QString QSqlRecord::displayLabel( const QString& name ) const
{
    if ( !field( name ) )
	return name;
    return d->fieldInfo[ position( name ) ].label;
}

/*! Sets the visible flag of field \a name to \a visible.  If the
  field does not exist, nothing happens.

  \sa isVisible()
*/

void QSqlRecord::setVisible( const QString& name, bool visible )
{
    if ( !field( name ) )
	return;
    d->fieldInfo[ position( name ) ].visible = visible;
}

/*! Returns TRUE if the field \a name is visible (the default),
 otherwise FALSE is returned.  If the field does not exist, FALSE is
 returned.

 \sa setVisible()
*/

bool QSqlRecord::isVisible( const QString& name ) const
{
    if ( !field( name ) )
	return FALSE;
    if ( !d->fieldInfo.contains( position( name ) ) )
	return TRUE;
    return d->fieldInfo[ position( name ) ].visible;
}

/*!  Returns a comma-separated list of all field names as a string.
  This string is suitable, for example, for generating a SQL SELECT
  statement.  If a \a prefix is specified, it is prepended before all
  field names.

*/

QString QSqlRecord::toString( const QString& prefix, const QString& sep ) const
{
    QString pflist;
    QString pfix =  prefix.isNull() ? QString::null : prefix + ".";
    bool comma = FALSE;

    for ( uint i = 0; i < count(); ++i ){
	if ( isGenerated( field(i)->name() ) ) {
	    if( comma )
		pflist += sep + " ";
	    pflist += pfix + field(i)->name();
	    comma = TRUE;
	}
    }
    return pflist;
}


/*!  Returns the number of fields in the record.

*/

uint QSqlRecord::count() const
{
    return d->fieldList.count();
}

/*!  \internal

*/

QSqlField* QSqlRecord::findField( int i ) const
{
#ifdef QT_CHECK_RANGE
    if( (unsigned int) i > d->fieldList.count() ){
	qWarning( "QSqlRecord::findField: index out of range: " + QString::number( i ) );
	return 0;
    }
#endif // QT_CHECK_RANGE
    return &d->fieldList[ i ];
}

/*!  \internal

*/

QSqlField* QSqlRecord::findField( const QString& name ) const
{
#ifdef QT_CHECK_RANGE
    if( (unsigned int) position( name ) > d->fieldList.count() ){
	qWarning( "QSqlRecord::findField: field not found: " + name );
	return 0;
    }
#endif // QT_CHECK_RANGE
    return &d->fieldList[ position( name ) ];
}

/*! Sets the value of the field at position \a i to \a val.  If the
  field does not exist, nothing happens.

*/

void QSqlRecord::setValue( int i, const QVariant& val )
{
    QSqlField* f = findField( i );
    if ( f )
	f->setValue( val );
}


/*!  Sets the value of field \a name to \a val.  If the field does not
  exist, nothing happens.
*/

void QSqlRecord::setValue( const QString& name, const QVariant& val )
{
    QSqlField* f = findField( name );
    if ( f )
	f->setValue( val );
}

#endif
