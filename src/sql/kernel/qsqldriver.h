/****************************************************************************
**
** Definition of QSqlDriver class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qstringlist.h"
#endif // QT_H

#if  defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlDatabase;
class QSqlDriverPrivate;

class QM_EXPORT_SQL QSqlDriver : public QObject
{
    friend class QSqlDatabase;
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlDriver)

public:
    enum DriverFeature { Transactions, QuerySize, BLOB, Unicode, PreparedQueries,
                         NamedPlaceholders, PositionalPlaceholders };

    QSqlDriver(QObject *parent=0);
    ~QSqlDriver();
    virtual bool isOpen() const;
    bool isOpenError() const;

    virtual bool beginTransaction();
    virtual bool commitTransaction();
    virtual bool rollbackTransaction();
    virtual QStringList tables(QSql::TableType tableType) const;
    virtual QSqlIndex primaryIndex(const QString &tableName) const;
    virtual QSqlRecord record(const QString &tableName) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QSqlRecord record(const QSqlQuery& query) const
    { return query.record(); }
    inline QT_COMPAT QSqlRecord recordInfo(const QString& tablename) const
    { return record(tablename); }
    inline QT_COMPAT QSqlRecord recordInfo(const QSqlQuery& query) const
    { return query.record(); }
    inline QT_COMPAT QString nullText() const { return QLatin1String("NULL"); }
#endif
    virtual QString formatValue(const QSqlField* field, bool trimStrings = false) const;
    QSqlError lastError() const;

    virtual bool hasFeature(DriverFeature f) const = 0;
    virtual void close() = 0;
    virtual QSqlQuery createQuery() const = 0;

    virtual bool open(const QString& db,
                       const QString& user = QString(),
                       const QString& password = QString(),
                       const QString& host = QString(),
                       int port = -1,
                       const QString& connOpts = QString()) = 0;
protected:
    virtual void setOpen(bool o);
    virtual void setOpenError(bool e);
    virtual void setLastError(const QSqlError& e);
private:
#if defined(Q_DISABLE_COPY)
    QSqlDriver(const QSqlDriver &);
    QSqlDriver &operator=(const QSqlDriver &);
#endif
};

#endif// QT_NO_SQL
#endif
