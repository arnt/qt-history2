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

#include "qcorevariant.h"
#include "qsqldriver.h"
#include "qsqlerror.h"
#include "qsqlresult.h"

class QSqlNullResult : public QSqlResult
{
public:
    inline QSqlNullResult(const QSqlDriver* d): QSqlResult(d)
    { QSqlResult::setLastError(QSqlError("Driver not loaded", "Driver not loaded")); }
protected:
    inline QCoreVariant data(int) { return QCoreVariant(); }
    inline bool reset (const QString&) { return false; }
    inline bool fetch(int) { return false; }
    inline bool fetchFirst() { return false; }
    inline bool fetchLast() { return false; }
    inline bool isNull(int) { return false; }
    inline int size()  { return -1; }
    inline int numRowsAffected() { return 0; }

    inline void setAt(int) {}
    inline void setActive(bool) {}
    inline void setLastError(const QSqlError&) {}
    inline void setQuery(const QString&) {}
    inline void setSelect(bool) {}
    inline void setForwardOnly(bool) {}

    inline bool exec() { return false; }
    inline bool prepare(const QString&) { return false; }
    inline bool savePrepare(const QString&) { return false; }
    inline void bindValue(int, const QCoreVariant&, QSql::ParamType) {}
    inline void bindValue(const QString&, const QCoreVariant&, QSql::ParamType) {}
};

class QSqlNullDriver : public QSqlDriver
{
public:
    inline QSqlNullDriver(): QSqlDriver()
    { QSqlDriver::setLastError(QSqlError("Driver not loaded", "Driver not loaded")); }
    inline bool hasFeature(DriverFeature) const { return false; }
    inline bool open(const QString &, const QString & , const QString & ,
              const QString &, int, const QString&)
    { return false; }
    inline void close() {}
    inline QSqlQuery createQuery() const { return QSqlQuery(new QSqlNullResult(this)); }

protected:
    inline void setOpen(bool) {}
    inline void setOpenError(bool) {}
    inline void setLastError(const QSqlError&) {}
};

#endif //QSQLNULLDRIVER_H
#endif //QT_NO_SQL
