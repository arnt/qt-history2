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

#ifndef QSQLQUERY_H
#define QSQLQUERY_H

#include "QtSql/qsql.h"
#include "QtSql/qsqldatabase.h"
#include "QtCore/qstring.h"

class QCoreVariant;
class QSqlDriver;
class QSqlError;
class QSqlResult;
class QSqlRecord;
template <class Key, class T> class QMap;

class QSqlQueryPrivate;

class Q_SQL_EXPORT QSqlQuery
{
public:
    QSqlQuery(QSqlResult *r);
    QSqlQuery(const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
    explicit QSqlQuery(QSqlDatabase db);
    QSqlQuery(const QSqlQuery& other);
    QSqlQuery& operator=(const QSqlQuery& other);
    ~QSqlQuery();

    bool isValid() const;
    bool isActive() const;
    bool isNull(int field) const;
    int at() const;
    QString lastQuery() const;
    int numRowsAffected() const;
    QSqlError lastError() const;
    bool isSelect() const;
    int size() const;
    const QSqlDriver* driver() const;
    const QSqlResult* result() const;
    bool isForwardOnly() const;
    QSqlRecord record() const;

    void setForwardOnly(bool forward);
    bool exec(const QString& query);
    QCoreVariant value(int i) const;

    bool seek(int i, bool relative = false);
    bool next();
    bool previous();
#ifdef QT_COMPAT
    inline QT_COMPAT bool prev() { return previous(); }
#endif
    bool first();
    bool last();

    void clear();

    // prepared query support
    bool exec();
    bool prepare(const QString& query);
    void bindValue(const QString& placeholder, const QCoreVariant& val,
                   QSql::ParamType type = QSql::In);
    void bindValue(int pos, const QCoreVariant& val, QSql::ParamType type = QSql::In);
    void addBindValue(const QCoreVariant& val, QSql::ParamType type = QSql::In);
    QCoreVariant boundValue(const QString& placeholder) const;
    QCoreVariant boundValue(int pos) const;
    QMap<QString, QCoreVariant> boundValues() const;
    QString executedQuery() const;

private:
    QSqlQueryPrivate* d;
};

#endif // QSQLQUERY_H
