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

#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#include "QtCore/qobject.h"
#include "QtCore/qstring.h"
#include "QtSql/qsqlquery.h"

class QStringList;
class QSqlDatabase;
class QSqlDriverPrivate;
class QSqlError;
class QSqlField;
class QSqlIndex;

class Q_SQL_EXPORT QSqlDriver : public QObject
{
    friend class QSqlDatabase;
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlDriver)

public:
    enum DriverFeature { Transactions, QuerySize, BLOB, Unicode, PreparedQueries,
                         NamedPlaceholders, PositionalPlaceholders, LastInsertId };

    enum StatementType { WhereStatement, SelectStatement, UpdateStatement,
                         InsertStatement, DeleteStatement };

    explicit QSqlDriver(QObject *parent=0);
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
    inline QT_COMPAT QString formatValue(const QSqlField *field, bool trimStrings = false) const
    { return field ? formatValue(*field, trimStrings) : QString(); }
#endif
    virtual QString formatValue(const QSqlField& field, bool trimStrings = false) const;

    virtual QString escapeIdentifier(const QString &identifier) const;
    virtual QString sqlStatement(StatementType type, const QString &tableName,
                                 const QSqlRecord &rec, bool preparedStatement) const;

    QSqlError lastError() const;

    virtual bool hasFeature(DriverFeature f) const = 0;
    virtual void close() = 0;
    virtual QSqlResult *createResult() const = 0;

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

    virtual void virtual_hook(int id, void *data);

private:
    Q_DISABLE_COPY(QSqlDriver)
};

#endif // QSQLDRIVER_H
