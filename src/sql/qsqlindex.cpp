/****************************************************************************
**
** Implementation of QSqlIndex class
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

#include "qsqlindex.h"


#ifndef QT_NO_SQL

/*!
    \class QSqlIndex qsqlindex.h
    \brief Class used for describing SQL database indexes

    \module sql

     This class is used to describe SQL database indexes.  An index
     can belong to only one table in a database.  Information about
     the fields that compromise the index can be obtained by calling
     the fields() method.

     Normally, QSqlIndex objects are created by QSqlDatabase.

     \sa QSqlDatabase
*/


/*!
  Constructs an empty SQL index using database \a database
  and table \a tablename.

*/

QSqlIndex::QSqlIndex( const QString& cursorname, const QString& name )
    : QSqlRecord(), cursor(cursorname), nm(name)
{

}

/*!  Constructs a copy of \a other.

*/

QSqlIndex::QSqlIndex( const QSqlIndex& other )
    : QSqlRecord(other), cursor(other.cursor), nm(other.nm), sorts(other.sorts)
{
}

QSqlIndex& QSqlIndex::operator=( const QSqlIndex& other )
{
    cursor = other.cursor;
    nm = other.nm;
    sorts = other.sorts;
    QSqlRecord::operator=( other );
    return *this;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlIndex::~QSqlIndex()
{

}

/*!
  Sets the name of the index to \a n.

*/

void QSqlIndex::setName( const QString& name )
{
    nm = name;
}


/*!
  Returns the name of the index, or QString::null if no
  name has been set.

*/

QString QSqlIndex::name() const
{
    return nm;
}

/*!
  Appends the field \a field to the list of indexed fields.  The field
  is added in an ascending sort order.

*/

void QSqlIndex::append( const QSqlField& field )
{
    append( field, FALSE );
}

/*!
  Appends the field \a field to the list of indexed fields.  The field
  is added in an ascending sort order, unless \a desc is TRUE.

*/

void QSqlIndex::append( const QSqlField& field, bool desc )
{
    sorts.append( desc );
    QSqlRecord::append( field );
}


/*!

  Returns true if field \a i in the index is sorted in descending
  order, otherwise FALSE is returned.

*/

bool QSqlIndex::isDescending( int i ) const
{
    if ( sorts.at( i ) != sorts.end() )
	return sorts[i];
    return FALSE;
}

/*!

  If \a desc is TRUE, field \a i is sorted in descending order.
  Otherwise, field \a i is sorted in ascending order (the default).
  If the field does not exist, nothing happens.

*/

void QSqlIndex::setDescending( int i, bool desc )
{
    if ( sorts.at( i ) != sorts.end() )
	sorts[i] = desc;
}


QString QSqlIndex::toString( const QString& prefix ) const
{
    QString s;
    for ( uint i = 0; i < count(); ++i ) {
	if ( !prefix.isNull() )
	    s += prefix + ".";
	s += field( i )->name();
	if ( isDescending( i ) )
	    s += " desc";
    }
    return s;
}

/*! \fn QString QSqlIndex::cursorName() const
  Returns the name of the cursor which the index is associated with.
*/

/*! \fn void QSqlIndex::setCursorName( const QString& cursorName )
  Sets the name of the cursor that the index is associated with to \a cursorName.
*/

#endif
