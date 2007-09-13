/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_PSQL_H
#define QSQL_PSQL_H

#include <QtSql/qsqlresult.h>
#include <QtSql/qsqldriver.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_PSQL
#else
#define Q_EXPORT_SQLDRIVER_PSQL Q_SQL_EXPORT
#endif

QT_BEGIN_HEADER

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

QT_BEGIN_NAMESPACE

class QPSQLResultPrivate;
class QPSQLDriverPrivate;
class QPSQLDriver;
class QSqlRecordInfo;

class QPSQLResult : public QSqlResult
{
    friend class QPSQLResultPrivate;
public:
    QPSQLResult(const QPSQLDriver* db, const QPSQLDriverPrivate* p);
    ~QPSQLResult();

    QVariant handle() const;
    void virtual_hook(int id, void *data);

protected:
    void cleanup();
    bool fetch(int i);
    bool fetchFirst();
    bool fetchLast();
    QVariant data(int i);
    bool isNull(int field);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;
    QVariant lastInsertId() const;

private:
    QPSQLResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_PSQL QPSQLDriver : public QSqlDriver
{
    Q_OBJECT
public:
    enum Protocol {
        Version6 = 6,
        Version7 = 7,
        Version71 = 8,
        Version73 = 9
    };

    explicit QPSQLDriver(QObject *parent=0);
    explicit QPSQLDriver(PGconn *conn, QObject *parent=0);
    ~QPSQLDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
              const QString & user,
              const QString & password,
              const QString & host,
              int port,
              const QString& connOpts);
    bool isOpen() const;
    void close();
    QSqlResult *createResult() const;
    QStringList tables(QSql::TableType) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QSqlRecord record(const QString& tablename) const;

    Protocol protocol() const;
    QVariant handle() const;

    QString escapeIdentifier(const QString &identifier, IdentifierType type) const;
    QString formatValue(const QSqlField &field, bool trimStrings) const;

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

protected Q_SLOTS:
    bool subscribeToNotificationImplementation(const QString &name);
    bool unsubscribeFromNotificationImplementation(const QString &name);
    QStringList subscribedToNotificationsImplementation() const;

private Q_SLOTS:
    void _q_handleNotification(int);

private:
    void init();
    QPSQLDriverPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSQL_PSQL_H
