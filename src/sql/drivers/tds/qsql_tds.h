/****************************************************************************
**
** Definition of TDS driver classes
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QSQL_TDS_H
#define QSQL_TDS_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>
#include "../shared/qsql_result.h"

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

class QTDSDriver : public QSqlDriver
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
