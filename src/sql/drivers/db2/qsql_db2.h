/****************************************************************************
**
** Definition of IBM DB2 driver classes
**
** Created : 021121
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSQL_DB2_H
#define QSQL_DB2_H

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_DB2
#else
#define Q_EXPORT_SQLDRIVER_DB2 Q_EXPORT
#endif

#include "qsqlresult.h"
#include "qsqlrecord.h"
#include "qsqldriver.h"


class QDB2Driver;
class QDB2DriverPrivate;
class QDB2ResultPrivate;
class QSqlRecordInfo;

class QDB2Result : public QSqlResult
{
    friend class QDB2Driver;
public:
    QDB2Result( const QDB2Driver* dr, const QDB2DriverPrivate* dp );
    ~QDB2Result();
    bool prepare( const QString& query );
    bool exec();

protected:
    QVariant data( int field );
    bool reset ( const QString& query );
    bool fetch( int i );
    bool fetchNext();
    bool fetchFirst();
    bool fetchLast();
    bool isNull( int i );
    int size();
    int numRowsAffected();
private:
    QDB2ResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_DB2 QDB2Driver : public QSqlDriver
{
public:
    QDB2Driver();
    ~QDB2Driver();
    bool hasFeature( DriverFeature ) const;
    bool open( const QString& db, const QString& user, const QString& passwd, const QString&, int );
    void close();
    QSqlRecord record( const QString& tableName ) const;
    QSqlRecord record( const QSqlQuery& query ) const;
    QSqlRecordInfo recordInfo( const QString& tablename ) const;
    QSqlRecordInfo recordInfo( const QSqlQuery& query ) const;
    QStringList tables( const QString& /* user */ ) const;
    QSqlQuery createQuery() const;
    QSqlIndex primaryIndex( const QString& tablename ) const;
private:
    QDB2DriverPrivate* d;
};

#endif
