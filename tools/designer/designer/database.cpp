/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "database.h"

#ifndef QT_NO_SQL
#include "formwindow.h"
#include "mainwindow.h"

#include <qsqldatabase.h>
#include <qsqlform.h>
#include <qsqlcursor.h>
#include <qsqlrecord.h>

DatabaseSupport::DatabaseSupport()
{
    con = 0;
    frm = 0;
    parent = 0;
}

void DatabaseSupport::initPreview( const QString &connection, const QString &table, QObject *o,
				   const QMap<QString, QString> &databaseControls )
{
    tbl = table;
    dbControls = databaseControls;
    parent = o;

    if ( connection != "(default)" )
	con = QSqlDatabase::database( connection );
    else
	con = QSqlDatabase::database();
    frm = new QSqlForm( o, table );
    for ( QMap<QString, QString>::Iterator it = dbControls.begin(); it != dbControls.end(); ++it ) {
	QObject *chld = parent->child( it.key(), "QWidget" );
	if ( !chld )
	    continue;
	frm->insert( (QWidget*)chld, *it );
    }
}

QDesignerDataBrowser::QDesignerDataBrowser( QWidget *parent, const char *name )
    : QDataBrowser( parent, name )
{
}

bool QDesignerDataBrowser::event( QEvent* e )
{
    bool b = QDataBrowser::event( e );
    if (
#if defined(DESIGNER)
	MainWindow::self->isPreviewing()
#else
	TRUE
#endif
	) {
	if ( e->type() == QEvent::Show ) {
	    QSqlCursor* cursor = new QSqlCursor( tbl, TRUE, con );
	    setCursor( cursor, TRUE );
	    setForm( frm );
	    refresh();
	    first();
	    return TRUE;
	}
    }
    return b;
}

QDesignerDataView::QDesignerDataView( QWidget *parent, const char *name )
    : QDataView( parent, name )
{
}

bool QDesignerDataView::event( QEvent* e )
{
    return QDataView::event( e );
}


#endif
