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
#include "qregexp.h"


#ifndef QT_NO_SQL

/*!
    \class QSqlRecord qsqlfield.h
    \brief Template class used for manipulating a list of SQL database fields.

    \module database
*/


/*!
  Constructs an empty record.

*/

QSqlRecord::QSqlRecord()
{

}

/*!  Constructs a copy of \a other.

*/

QSqlRecord::QSqlRecord( const QSqlRecord& other )
    : fieldList( other.fieldList )
{

}

QSqlRecord& QSqlRecord::operator=( const QSqlRecord& other )
{
    fieldList = other.fieldList;
    return *this;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlRecord::~QSqlRecord()
{

}

/*!
  Returns the value of the field located at position \a i in the record.
  It is up to you to check wether this item really exists.

*/

QVariant  QSqlRecord::value( int i ) const
{
    return findField(i)->value();
}

/*!
  Returns the value of the field named \a name in the record.
  It is up to you to check wether this item really exists.

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

QSqlField* QSqlRecord::field( int i )
{
    return &fieldList[ i ];
}

const QSqlField* QSqlRecord::field( int i ) const
{
    return &fieldList[ i ];
}

QSqlField* QSqlRecord::field( const QString& name )
{
    return &fieldList[ position( name ) ];
}

const QSqlField* QSqlRecord::field( const QString& name ) const
{
    return &fieldList[ position( name ) ];
}


/*!
  Appends a copy of the field \a field to the end of the record.

*/

void QSqlRecord::append( const QSqlField& field )
{
    fieldList.append( field );
}

/*!
  Prepends a copy of \a field to the beginning of the record.

*/

void QSqlRecord::prepend( const QSqlField& field )
{
    fieldList.prepend( field );

}

/*!  Inserts a copy of \a field before \a pos.  If \a pos does not
  exist, it is appended to the end of the record.

*/

void QSqlRecord::insert( int pos, const QSqlField& field )
{
    fieldList.insert( fieldList.at( pos ), field );
}

/*!  Removes all the field at \a pos.  If \a pos does not exist,
  nothing happens.

*/

void QSqlRecord::remove( int pos )
{
    fieldList.remove( fieldList.at( pos ) );
}

/*!
  Removes all fields from the record.

*/

void QSqlRecord::clear()
{
    fieldList.clear();
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

/*!  Returns a comma-separated list of field names as a string.  This
  string is suitable for use in, for example, generating a select
  statement.  If a \a prefix is specified, it is prepended before all
  field names.

*/

QString QSqlRecord::toString( const QString& prefix ) const
{
    QString pflist;
    QString pfix =  prefix.isNull() ? QString::null : prefix + ".";
    int actualFields = 0;
    
    for ( uint i = 0; i < count(); ++i ){
	if ( !field(i)->isCalculated() ) {
	    if( actualFields > 0 )
		pflist += ", ";
	    pflist += pfix + field(i)->name();
	    actualFields++;
	}
    }
    return pflist;
}


/*!
  Returns the number of fields in the record.

*/

uint QSqlRecord::count() const
{
    return fieldList.count();
}

/*!
  \internal

*/

const QSqlField* QSqlRecord::findField( int i ) const
{
#ifdef QT_CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) i > fieldList.count() ){
	qWarning( "QSqlRecord::findField: index out of range" );
	return &dbg;
    }
#endif // QT_CHECK_RANGE
    return &fieldList[ i ];
}

/*!
  \internal

*/

QSqlField* QSqlRecord::findField( int i )
{
#ifdef QT_CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) i > fieldList.count() ){
	qWarning( "QSqlRecord::findField: index out of range" );
	return &dbg;
    }
#endif // QT_CHECK_RANGE
    return &fieldList[ i ];
}

/*!
  \internal

*/

const QSqlField* QSqlRecord::findField( const QString& name ) const
{
#ifdef QT_CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) position( name ) > fieldList.count() ){
	qWarning( "QSqlRecord::findField : index out of range" );
	return &dbg;
    }
#endif // QT_CHECK_RANGE
    return &fieldList[ position( name ) ];
}


/*!
  \internal

*/

QSqlField* QSqlRecord::findField( const QString& name )
{
#ifdef QT_CHECK_RANGE
    static QSqlField dbg;
    if( (unsigned int) position( name ) > fieldList.count() ){
	qWarning( "QSqlRecord::findField : index out of range" );
	return &dbg;
    }
#endif // QT_CHECK_RANGE
    return &fieldList[ position( name ) ];
}

/*! Sets the value of the field at position \a i with \a val.  If the field does
  not exist, nothing happens.

*/

void QSqlRecord::setValue( int i, const QVariant& val )
{
    QSqlField* f = findField( i );
    if ( f )
	f->setValue( val );
}


/*!  Sets the value of field \a name with \a val.  If the field does
  not exist, nothing happens.
*/

void QSqlRecord::setValue( const QString& name, const QVariant& val )
{
    QSqlField* f = findField( name );
    if ( f )
	f->setValue( val );
}

#endif
