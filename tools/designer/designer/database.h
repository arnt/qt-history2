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

#ifndef DATABASE_H
#define DATABASE_H

#include "qfeatures.h"

#ifndef QT_NO_SQL
#include <qdataview.h>
#include <qdatabrowser.h>
#include <qsqlcursor.h>
#include <qstring.h>
#include <qmap.h>

class QSqlDatabase;
class QSqlForm;

class DatabaseSupport
{
public:
    DatabaseSupport();
    virtual ~DatabaseSupport() {}

    void initPreview( const QString &connection, const QString &table, QObject *o,
		      const QMap<QString, QString> &databaseControls );

protected:
    QSqlDatabase* con;
    QSqlForm* frm;
    QString tbl;
    QMap<QString, QString> dbControls;
    QObject *parent;

};

class QDesignerDataBrowser : public QDataBrowser, public DatabaseSupport
{
    Q_OBJECT

public:
    QDesignerDataBrowser( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );
};

class QDesignerDataView : public QDataView, public DatabaseSupport
{
    Q_OBJECT

public:
    QDesignerDataView( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );

};
#endif

#endif
