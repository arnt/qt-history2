/****************************************************************************
**
** Definition of QSqlSelectCursor class
**
** Created : 2002-11-13
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#include "qsqlselectcursor.h"
#include "qsqldriver.h"

#ifndef QT_NO_SQL

class QSqlSelectCursorPrivate
{
public:
    QSqlSelectCursorPrivate() : populated( FALSE ) {}
    QString query;
    bool populated : 1;
};

/*!
    \class QSqlSelectCursor qsqlselectcursor.h
    \brief The QSqlSelectCursor class provides browsing of general SQL
    queries.

    \ingroup database
    \module sql

    QSqlSelectCursor is a convenience class that makes it possible to
    display the result set of general SQL queries in data-aware Qt
    widgets. QSqlSelectCursor is read only and does not support
    insert, update and delete operations.
*/

/*! Constructs a read only cursor on database \a db using the query \a query.
 */
QSqlSelectCursor::QSqlSelectCursor( const QString& query, QSqlDatabase* db )
    : QSqlCursor( QString::null, FALSE, db )
{
    d = new QSqlSelectCursorPrivate;
    d->query = query;
    setMode( ReadOnly );
    if ( !query.isNull() )
	exec( query );
}

/*! Constructs a copy of \a other */
QSqlSelectCursor::QSqlSelectCursor( const QSqlSelectCursor& other )
    : QSqlCursor( other )
{
    d = new QSqlSelectCursorPrivate;
    d->query = other.d->query;
    d->populated = other.d->populated;
}

/*! Destroys the object and frees any allocated resources */
QSqlSelectCursor::~QSqlSelectCursor()
{
    delete d;
}

/*! \reimp */
bool QSqlSelectCursor::exec( const QString& query )
{
    d->query = query;
    bool ret = QSqlCursor::exec( query );
    if ( ret ) {
	QSqlCursor::clear();
	populateCursor();
    }
    return ret;
}

/*! \reimp */
bool QSqlSelectCursor::select( const QString&, const QSqlIndex& )
{
    bool ret = QSqlCursor::exec( d->query );
    if ( ret && !d->populated )
	populateCursor();
    return ret;
}

/*! \internal */
void QSqlSelectCursor::populateCursor()
{
    QSqlRecordInfo inf = driver()->recordInfo( *(QSqlQuery*)this );
    for ( QSqlRecordInfo::iterator it = inf.begin(); it != inf.end(); ++it )
	QSqlCursor::append( *it );
    d->populated = TRUE;
}

#endif // QT_NO_SQL
