/****************************************************************************
**
** Definition of Interbase driver classes
**
** Created : 030911
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
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

#ifndef QSQL_IBASE_H
#define QSQL_IBASE_H

#include "qsqlresult.h"
#include "qsqldriver.h"
#include "../cache/qsqlcachedresult.h"


class QIBaseDriverPrivate;
class QIBaseResultPrivate;
class QIBaseDriver;

class QIBaseResult : public QtSqlCachedResult
{
    friend class QIBaseResultPrivate;

public:
    QIBaseResult(const QIBaseDriver* db);
    virtual ~QIBaseResult();

    bool prepare(const QString& query);
    bool exec();

protected:
    bool gotoNext(QtSqlCachedResult::ValueCache& row, int rowIdx);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

private:
    QIBaseResultPrivate* d;
};

class QIBaseDriver : public QSqlDriver
{
    friend class QIBaseDriverPrivate;
    friend class QIBaseResultPrivate;
public:
    QIBaseDriver(QObject *parent = 0);
    QIBaseDriver(void *connection, QObject *parent = 0);
    virtual ~QIBaseDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
                   const QString & user,
                   const QString & password,
                   const QString & host,
                   int port,
                   const QString & connOpts);
    bool open(const QString & db,
            const QString & user,
            const QString & password,
            const QString & host,
            int port) { return open (db, user, password, host, port, QString()); }
    void close();
    QSqlQuery createQuery() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QStringList tables(QSql::TableType) const;

    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString &table) const;

    QString formatValue(const QSqlField* field, bool trimStrings) const;

private:
    QIBaseDriverPrivate* d;
};


#endif

