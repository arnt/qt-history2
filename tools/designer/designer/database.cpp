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
#include "formwindow.h"

#include <qsqldatabase.h>
#include <qsqlform.h>
#include <qsqlcursor.h>
#include <qsqlrecord.h>

QDesignerSqlWidget::QDesignerSqlWidget( QWidget *parent, const char *name )
    : QSqlWidget( parent, name )
{
}

void QDesignerSqlWidget::paintEvent( QPaintEvent *e )
{
#if defined(DESIGNER)
    if ( parentWidget() && parentWidget()->inherits( "FormWindow" ) )
	( (FormWindow*)parentWidget() )->paintGrid( this, e );
#else
    Q_UNUSED( e );
#endif
}




QDesignerSqlDialog::QDesignerSqlDialog( QWidget *parent, const char *name )
    : QSqlDialog( parent, name )
{
}

void QDesignerSqlDialog::paintEvent( QPaintEvent *e )
{
#if defined(DESIGNER)
    if ( parentWidget() && parentWidget()->inherits( "FormWindow" ) )
	( (FormWindow*)parentWidget() )->paintGrid( this, e );
#else
    Q_UNUSED( e );
#endif
}


DatabaseSupport::DatabaseSupport()
{
    cursor = 0;
    defaultConnection = 0;
    form = 0;
    parent = 0;
}

void DatabaseSupport::initPreview( const QString &connection, const QString &table, QObject *o,
				   const QMap<QString, QString> &databaseControls )
{
    tbl = table;
    dbControls = databaseControls;
    parent = o;

    if ( connection != "(default)" )
	defaultConnection = QSqlDatabase::database( connection );
    else
	defaultConnection = QSqlDatabase::database();
    cursor = 0;
    form = new QSqlForm( o, table );
    autoDeleteCursors.resize( 1 );
    autoDeleteCursors.setAutoDelete( TRUE );
}

QSqlCursor* DatabaseSupport::defCursor()
{
    if ( !cursor ) {
	cursor = new QSqlCursor( tbl );
	cursor->select();
	autoDeleteCursors.insert( 0, cursor );
	QSqlRecord* buf = cursor->editBuffer();
	for ( QMap<QString, QString>::Iterator it = dbControls.begin(); it != dbControls.end(); ++it ) {
	    QObject *chld = parent->child( it.key(), "QWidget" );
	    if ( !chld )
		continue;
	    form->insert( (QWidget*)chld, buf->field( *it ) );
	}
	if ( form )
	    form->readFields();
    }
    return cursor;
}

QSqlForm* DatabaseSupport::defForm()
{
    return form;
}
