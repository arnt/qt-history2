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

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#ifndef QT_H
#include "qsql.h"
#endif // QT_H

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QString;
class QSqlRecord;
template <typename T> class QVector;
class QCoreVariant;


class QSqlDriver;
class QSqlError;
class QSql;
class QSqlResultPrivate;

class QM_EXPORT_SQL QSqlResult
{
friend class QSqlQuery;
friend class QSqlResultShared;
friend class QSqlResultPrivate;
public:
    virtual ~QSqlResult();

protected:
    enum BindMethod { BindByPosition, BindByName };

    QSqlResult(const QSqlDriver * db);
    int at() const;
    QString lastQuery() const;
    QSqlError lastError() const;
    bool isValid() const;
    bool isActive() const;
    bool isSelect() const;
    bool isForwardOnly() const;
    const QSqlDriver* driver() const;
    virtual void setAt(int at);
    virtual void setActive(bool a);
    virtual void setLastError(const QSqlError& e);
    virtual void setQuery(const QString& query);
    virtual void setSelect(bool s);
    virtual void setForwardOnly(bool forward);

    // prepared query support
    virtual bool exec();
    virtual bool prepare(const QString& query);
    // ### TODO - find a much better name
    virtual bool savePrepare(const QString& sqlquery);
    virtual void bindValue(int pos, const QCoreVariant& val, QSql::ParamType type);
    virtual void bindValue(const QString& placeholder, const QCoreVariant& val,
                           QSql::ParamType type);
    void addBindValue(const QCoreVariant& val, QSql::ParamType type);
    QCoreVariant boundValue(const QString& placeholder) const;
    QCoreVariant boundValue(int pos) const;
    QSql::ParamType bindValueType(const QString& placeholder) const;
    QSql::ParamType bindValueType(int pos) const;
    int boundValueCount() const;
    QVector<QCoreVariant>& boundValues() const;
    QString executedQuery() const;
    QString boundValueName(int pos) const;
    void clear();
    bool hasOutValues() const;

    BindMethod bindMethod() const;

    virtual QCoreVariant data(int i) = 0;
    virtual bool isNull(int i) = 0;
    virtual bool reset (const QString& sqlquery) = 0;
    virtual bool fetch(int i) = 0;
    virtual bool fetchNext();
    virtual bool fetchPrevious();
    virtual bool fetchFirst() = 0;
    virtual bool fetchLast() = 0;
    virtual int size() = 0;
    virtual int numRowsAffected() = 0;
    virtual QSqlRecord record() const;

private:
    QSqlResultPrivate* d;
    void resetBindCount(); //HACK

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSqlResult(const QSqlResult &);
    QSqlResult &operator=(const QSqlResult &);
#endif
};

#endif // QT_NO_SQL
#endif
