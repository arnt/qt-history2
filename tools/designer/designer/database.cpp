/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#if defined(DESIGNER)
#include "database.h"
#else
#include "database2.h"
#endif

#ifndef QT_NO_SQL

#if defined(DESIGNER)
#include "formwindow.h"
#include "mainwindow.h"
#endif

#include <qevent.h>
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
    frm = new QSqlForm( o );
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
#if defined(DESIGNER)
    if ( MainWindow::self->isPreviewing() ) {
#endif
	if ( e->type() == QEvent::Show ) {
	    if ( con ) {
		QSqlCursor* cursor = new QSqlCursor( tbl, TRUE, con );
		setSqlCursor( cursor, TRUE );
		setForm( frm );
		refresh();
		first();
	    }
	    return TRUE;
	}
#if defined(DESIGNER)
    }
#endif
    return b;
}

QDesignerDataView::QDesignerDataView( QWidget *parent, const char *name )
    : QDataView( parent, name )
{
}

bool QDesignerDataView::event( QEvent* e )
{
    bool b = QDataView::event( e );
#if defined(DESIGNER)
    if ( MainWindow::self->isPreviewing() ) {
#endif
	if ( e->type() == QEvent::Show ) {
	    setForm( frm );
	    readFields();
	    return TRUE;
	}
#if defined(DESIGNER)
    }
#endif
    return b;
}


#endif
