/****************************************************************************
**
** Definition of QSqlQuery class.
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

#ifndef QSQLQUERY_H
#define QSQLQUERY_H

#ifndef QT_H
#include "qsql.h"
#include "qsqldatabase.h"
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_SQL

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
    QSqlQuery(const char *query, QSqlDatabase db = QSqlDatabase());
    Q_EXPLICIT QSqlQuery(QSqlDatabase db);
    QSqlQuery(const QSqlQuery& other);
    QSqlQuery& operator=(const QSqlQuery& other);
    virtual ~QSqlQuery();

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

    virtual void setForwardOnly(bool forward);
    virtual bool exec (const QString& query);
    virtual QCoreVariant value(int i) const;

    virtual bool seek(int i, bool relative = false);
    virtual bool next();
    virtual bool previous();
#ifdef QT_COMPAT
    inline QT_COMPAT bool prev() { return previous(); }
#endif
    virtual bool first();
    virtual bool last();

    virtual void clear();

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

protected:
    virtual void beforeSeek();
    virtual void afterSeek();

private:
    void init(const QString& query, QSqlDatabase db);
//     void detach();
    QSqlQueryPrivate* d;
};


#endif // QT_NO_SQL
#endif
