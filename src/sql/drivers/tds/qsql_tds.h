/****************************************************************************
**
** Definition of TDS driver classes
**
** Created : 001103
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSQL_TDS_H
#define QSQL_TDS_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>
#include "../shared/qsql_result.h"

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_TDS
#else
#define Q_EXPORT_SQLDRIVER_TDS Q_EXPORT
#endif

class QTDSPrivate;
class QTDSDriver;

class QTDSResult : public QSqlCachedResult
{
    friend class QTDSDriver;
public:
    QTDSResult( const QTDSDriver* db, const QTDSPrivate* p );
    ~QTDSResult();
protected:
    void		cleanup();
    bool		reset ( const QString& query );
    int			size();
    int			numRowsAffected();
    bool		gotoNext();
private:
    QTDSPrivate*	d;
};

class Q_EXPORT_SQLDRIVER_TDS QTDSDriver : public QSqlDriver
{
public:
    QTDSDriver( QObject * parent=0, const char * name=0 );
    ~QTDSDriver();
    bool		hasFeature( DriverFeature f ) const;
    bool		open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null,
			      int port = -1 );
    void		close();
    QStringList         tables( const QString& user ) const;
    QSqlQuery		createQuery() const;
    QSqlRecord          record( const QSqlQuery& query ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecordInfo	recordInfo( const QString& tablename ) const;
    QSqlRecordInfo	recordInfo( const QSqlQuery& query ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;

    QString		formatValue( const QSqlField* field,
				     bool trimStrings ) const;
protected:
    bool		beginTransaction();
    bool		commitTransaction();
    bool		rollbackTransaction();
private:
    void		init();
    bool                inTransaction;
    QTDSPrivate*	d;
    QString             dbName;
    QString             hostName;
};

#endif
