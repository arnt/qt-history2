/****************************************************************************
**
** Definition of OCI driver classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqldriver.h>
#include <qsqlcachedresult.h>

#include <oci.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_OCI
#else
#define Q_EXPORT_SQLDRIVER_OCI Q_SQL_EXPORT
#endif

// Check if OCI supports scrollable cursors (Oracle version >= 9)
#ifdef OCI_STMT_SCROLLABLE_READONLY
#define QOCI_USES_VERSION_9
#endif

class QOCIPrivate;
class QOCIResultPrivate;
class QOCIDriver;

class QOCIResult : public QSqlCachedResult
{
    friend class QOCIDriver;
    friend class QOCIPrivate;
public:
    QOCIResult(const QOCIDriver * db, QOCIPrivate* p);
    ~QOCIResult();
    OCIStmt* statement();
    bool prepare(const QString& query);
    bool exec();

protected:
    bool gotoNext(ValueCache &values, int index);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

private:
    QOCIPrivate*        d;
    QOCIResultPrivate*  cols;
};

#ifdef QOCI_USES_VERSION_9
class QOCI9Result : public QSqlResult
{
    friend class QOCIPrivate;
    friend class QOCIDriver;
public:
    QOCI9Result(const QOCIDriver * db, QOCIPrivate* p);
    ~QOCI9Result();
    OCIStmt*    statement();
    bool         prepare(const QString& query);
    bool         exec();

protected:
    bool        fetchNext();
    bool        fetchPrev();
    bool        fetchFirst();
    bool        fetchLast();
    bool        fetch(int i);
    bool        reset (const QString& query);
    QCoreVariant        data(int field);
    bool        isNull(int field);
    int         size();
    int         numRowsAffected();
    QSqlRecord record() const;

private:
    QOCIPrivate*        d;
    QOCIResultPrivate*  cols;
    bool                cacheNext(int r);
};
#endif //QOCI_USES_VERSION_9

class Q_EXPORT_SQLDRIVER_OCI QOCIDriver : public QSqlDriver
{
public:
    QOCIDriver(QObject* parent = 0);
    QOCIDriver(OCIEnv* env, OCIError* err, OCISvcCtx* ctx, QObject* parent = 0);
    ~QOCIDriver();
    bool                hasFeature(DriverFeature f) const;
    bool                open(const QString & db,
                              const QString & user,
                              const QString & password,
                              const QString & host,
                              int port,
                              const QString& connOpts);
    void                close();
    QSqlQuery                createQuery() const;
    QStringList         tables(QSql::TableType) const;
    QSqlRecord          record(const QString& tablename) const;
    QSqlIndex           primaryIndex(const QString& tablename) const;
    QString             formatValue(const QSqlField* field,
                                     bool trimStrings) const;
    OCIEnv*             environment();
    OCISvcCtx*          serviceContext();

protected:
    bool                beginTransaction();
    bool                commitTransaction();
    bool                rollbackTransaction();
private:
    void                init();
    void                cleanup();
    QOCIPrivate*        d;
};

#endif
