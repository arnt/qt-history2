/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DATABASE2_H
#define DATABASE2_H

#include "qfeatures.h"

#ifndef QT_NO_SQL
#include <qdataview.h>
#include <qdatabrowser.h>
#include <qsqlcursor.h>
#include <qstring.h>
#include <qmap.h>

class QSqlDatabase;
class QSqlForm;

class DatabaseSupport2
{
public:
    DatabaseSupport2();
    virtual ~DatabaseSupport2() {}

    void initPreview( const QString &connection, const QString &table, QObject *o,
		      const QMap<QString, QString> &databaseControls );

protected:
    QSqlDatabase* con;
    QSqlForm* frm;
    QString tbl;
    QMap<QString, QString> dbControls;
    QObject *parent;

};

class QDesignerDataBrowser2 : public QDataBrowser, public DatabaseSupport2
{
    Q_OBJECT

public:
    QDesignerDataBrowser2( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );
};

class QDesignerDataView2 : public QDataView, public DatabaseSupport2
{
    Q_OBJECT

public:
    QDesignerDataView2( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );

};

#define DatabaseSupport DatabaseSupport2
#define QDesignerDataBrowser QDesignerDataBrowser2
#define QDesignerDataView QDesignerDataView2

#endif

#endif
