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

#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <QtSql/qsqlresult.h>
#include <QtSql/qsqldriver.h>
#include <QtSql/private/qsqlcachedresult_p.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_OCI
#else
#define Q_EXPORT_SQLDRIVER_OCI Q_SQL_EXPORT
#endif

class QOCIPrivate;
class QOCIResultPrivate;
class QOCIDriver;

typedef struct OCIEnv OCIEnv;
typedef struct OCISvcCtx OCISvcCtx;

class Q_EXPORT_SQLDRIVER_OCI QOCIResult : public QSqlCachedResult
{
    friend class QOCIDriver;
    friend class QOCIPrivate;
public:
    QOCIResult(const QOCIDriver * db, QOCIPrivate* p);
    ~QOCIResult();
    bool prepare(const QString& query);
    bool exec();
    QVariant handle() const;

protected:
    bool gotoNext(ValueCache &values, int index);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;
    QVariant lastInsertId() const;

private:
    QOCIPrivate*        d;
    QOCIResultPrivate*  cols;
};

class Q_EXPORT_SQLDRIVER_OCI QOCIDriver : public QSqlDriver
{
public:
    explicit QOCIDriver(QObject* parent = 0);
    QOCIDriver(OCIEnv* env, OCISvcCtx* ctx, QObject* parent = 0);
    ~QOCIDriver();
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
    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QString formatValue(const QSqlField &field,
                        bool trimStrings) const;
    QVariant handle() const;

protected:
    bool                beginTransaction();
    bool                commitTransaction();
    bool                rollbackTransaction();
private:
    QOCIPrivate*        d;
};

#endif // QSQL_OCI_H
