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

#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#include <mysql.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_MYSQL
#else
#define Q_EXPORT_SQLDRIVER_MYSQL Q_SQL_EXPORT
#endif

class QMYSQLDriverPrivate;
class QMYSQLResultPrivate;
class QMYSQLDriver;
class QSqlRecordInfo;

class QMYSQLResult : public QSqlResult
{
    friend class QMYSQLDriver;
public:
    QMYSQLResult(const QMYSQLDriver* db);
    ~QMYSQLResult();

    MYSQL_RES* result();
protected:
    void cleanup();
    bool fetch(int i);
    bool fetchNext();
    bool fetchLast();
    bool fetchFirst();
    QCoreVariant data(int field);
    bool isNull(int field);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

#if MYSQL_VERSION_ID >= 40108
    bool prepare(const QString& stmt);
    bool exec();
#endif
private:
    QMYSQLResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_MYSQL QMYSQLDriver : public QSqlDriver
{
    friend class QMYSQLResult;
public:
    QMYSQLDriver(QObject *parent=0);
    QMYSQLDriver(MYSQL *con, QObject * parent=0);
    ~QMYSQLDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
               const QString & user,
               const QString & password,
               const QString & host,
               int port,
               const QString& connOpts);
    void close();
    QSqlResult *createResult() const;
    QStringList tables(QSql::TableType) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QSqlRecord record(const QString& tablename) const;
    QString formatValue(const QSqlField &field,
                                     bool trimStrings) const;
    MYSQL *mysql();

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
private:
    void init();
    QMYSQLDriverPrivate* d;
};


#endif
