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

#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#include "qstring.h"
#include "qsql.h"
#ifdef QT_COMPAT
#include "qsqlrecord.h"
#endif

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlError;
class QSqlDriver;
class QSqlIndex;
class QSqlRecord;
class QSqlRecordInfo;
class QSqlQuery;
class QSqlDatabasePrivate;

class QM_EXPORT_SQL QSqlDriverCreatorBase
{
public:
    virtual QSqlDriver* createObject() const = 0;
};

template <class type>
class QSqlDriverCreator: public QSqlDriverCreatorBase
{
public:
    QSqlDriver* createObject() const { return new type; }
};

class QM_EXPORT_SQL QSqlDatabase
{
public:
    QSqlDatabase();
    QSqlDatabase(const QSqlDatabase &other);
    ~QSqlDatabase();

    QSqlDatabase &operator=(const QSqlDatabase &other);

    bool open();
    bool open(const QString& user, const QString& password);
    void close();
    bool isOpen() const;
    bool isOpenError() const;
    QStringList tables(QSql::TableType type = QSql::Tables) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QSqlRecord record(const QString& tablename) const;
#ifdef QT_COMPAT
    QT_COMPAT QSqlRecord record(const QSqlQuery& query) const;
    inline QT_COMPAT QSqlRecord recordInfo(const QString& tablename) const
    { return record(tablename); }
    QT_COMPAT QSqlRecord recordInfo(const QSqlQuery& query) const;
#endif
    QSqlQuery exec(const QString& query = QString()) const;
    QSqlError lastError() const;
    bool isValid() const;

    bool transaction();
    bool commit();
    bool rollback();

    void setDatabaseName(const QString& name);
    void setUserName(const QString& name);
    void setPassword(const QString& password);
    void setHostName(const QString& host);
    void setPort(int p);
    void setConnectOptions(const QString& options = QString());
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    QString driverName() const;
    int port() const;
    QString connectOptions() const;

    QSqlDriver* driver() const;

    QT_STATIC_CONST char *defaultConnection;

    static QSqlDatabase addDatabase(const QString& type,
                                 const QString& connectionName = QLatin1String(defaultConnection));
    static QSqlDatabase addDatabase(QSqlDriver* driver,
                                 const QString& connectionName = QLatin1String(defaultConnection));
    static QSqlDatabase cloneDatabase(const QSqlDatabase &other, const QString& connectionName);
    static QSqlDatabase database(const QString& connectionName = QLatin1String(defaultConnection),
                                 bool open = true);
    static void removeDatabase(const QString& connectionName);
    static bool contains(const QString& connectionName = QLatin1String(defaultConnection));
    static QStringList drivers();
    static QStringList connectionNames();
    static void registerSqlDriver(const QString& name, QSqlDriverCreatorBase* creator);
    static bool isDriverAvailable(const QString& name);

protected:
    QSqlDatabase(const QString& type);
    QSqlDatabase(QSqlDriver* driver);

private:
    friend class QSqlDatabasePrivate;
    QSqlDatabasePrivate *d;
};

#ifndef QT_NO_DEBUG
QM_EXPORT_SQL QDebug operator<<(QDebug, const QSqlDatabase &);
#endif

#endif // QT_NO_SQL
#endif

