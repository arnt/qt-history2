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

#ifndef DATABASE_H
#define DATABASE_H

#include <qsqlwidget.h>
#include <qsqldialog.h>
#include <qsqlcursor.h>
#include <qvector.h>
#include <qstring.h>

class QSqlDatabase;
class QSqlForm;

class DatabaseSupport
{
public:
    DatabaseSupport();

    void initPreview( const QString &connection, const QString &table, QObject *o,
		      const QMap<QString, QString> &databaseControls );

    QSqlCursor* defCursor();
    QSqlForm* defForm();

protected:
    QSqlDatabase* defaultConnection;
    QSqlCursor* cursor;
    QSqlForm* form;
    QVector< QSqlCursor > autoDeleteCursors;
    QString tbl;
    QMap<QString, QString> dbControls;
    QObject *parent;

};

class QDesignerSqlWidget : public QSqlWidget, public DatabaseSupport
{
    Q_OBJECT

public:
    QDesignerSqlWidget( QWidget *parent, const char *name );

    QSqlCursor* defaultCursor() { return DatabaseSupport::defCursor(); }
    QSqlForm* defaultForm() { return DatabaseSupport::defForm(); }

protected:
    bool event( QEvent* e );
    void paintEvent( QPaintEvent *e );

};

class QDesignerSqlDialog : public QSqlDialog, public DatabaseSupport
{
    Q_OBJECT

public:
    QDesignerSqlDialog( QWidget *parent, const char *name );

    QSqlCursor* defaultCursor() { return DatabaseSupport::defCursor(); }
    QSqlForm* defaultForm() { return DatabaseSupport::defForm(); }

protected:
    bool event( QEvent* e );
    void paintEvent( QPaintEvent *e );

};

#endif
