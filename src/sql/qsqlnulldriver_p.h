/****************************************************************************
**
** Definition of QSqlNullDriver class.
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

#ifndef QT_NO_SQL

#ifndef QSQLNULLDRIVER_H
#define QSQLNULLDRIVER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#include <qsqldriver.h>
#include <qsqlresult.h>

class Q_SQL_EXPORT QNullResult : public QSqlResult
{
public:
    QNullResult(const QSqlDriver* d): QSqlResult(d) {}
protected:
    QCoreVariant data(int) { return QCoreVariant(); }
    bool reset (const QString&) { return false; }
    bool fetch(int) { return false; }
    bool fetchFirst() { return false; }
    bool fetchLast() { return false; }
    bool isNull(int) { return false; }
    int size()  { return -1; }
    int numRowsAffected() { return 0; }

    void setAt(int) {}
    void setActive(bool) {}
    void setLastError(const QSqlError&) {}
    void setQuery(const QString&) {}
    void setSelect(bool) {}
    void setForwardOnly(bool) {}

    bool exec() { return false; }
    bool prepare(const QString&) { return false; }
    bool savePrepare(const QString&) { return false; }
    void bindValue(int, const QCoreVariant&, QSql::ParamType) {}
    void bindValue(const QString&, const QCoreVariant&, QSql::ParamType) {}
};

class Q_SQL_EXPORT QNullDriver : public QSqlDriver
{
public:
    QNullDriver(): QSqlDriver() {}
    bool hasFeature(DriverFeature) const { return false; }
    bool open(const QString &, const QString & , const QString & ,
              const QString &, int, const QString&)
    { return false; }
    void close() {}
    QSqlQuery createQuery() const { return QSqlQuery(new QNullResult(this)); }

protected:
    void setOpen(bool) {}
    void setOpenError(bool) {}
    void setLastError(const QSqlError&) {}
};

#endif //QSQLNULLDRIVER_H
#endif //QT_NO_SQL
