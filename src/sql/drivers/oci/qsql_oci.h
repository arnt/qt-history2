/****************************************************************************
**
** Definition of OCI driver classes
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

#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>
#include <qstring.h>

#include <oci.h>

// Check if OCI supports scrollable cursors (Oracle version >= 9)
#ifdef OCI_STMT_SCROLLABLE_READONLY
#define QOCI_USES_VERSION_9
#endif


class QOCIPrivate;
class QOCIResultPrivate;
class QOCIDriver;

class QOCIResult : public QSqlResult
{
    friend class QOCIDriver;
public:
    QOCIResult( const QOCIDriver * db, QOCIPrivate* p );
    ~QOCIResult();
    OCIStmt*    statement();

protected:
    bool	fetchNext();
    bool	fetchFirst();
    bool	fetchLast();
    bool	fetch(int i);
    bool	reset ( const QString& query );
    QVariant	data( int field );
    bool	isNull( int field );
    int         size();
    int         numRowsAffected();

private:
    typedef QMap< uint, QSqlField > RowCache;
    typedef QMap< uint, RowCache > RowsetCache;

    QOCIPrivate*	d;
    QOCIResultPrivate*  cols;
    RowsetCache	        rowCache;
    QSqlRecord          fs;
    bool                cached;
    bool                cacheNext();
};

#ifdef QOCI_USES_VERSION_9
class QOCI9Result : public QSqlResult
{
    friend class QOCIDriver;
public:
    QOCI9Result( const QOCIDriver * db, QOCIPrivate* p );
    ~QOCI9Result();
    OCIStmt*    statement();

protected:
    bool	fetchNext();
    bool	fetchPrev();
    bool	fetchFirst();
    bool	fetchLast();
    bool	fetch(int i);
    bool	reset ( const QString& query );
    QVariant	data( int field );
    bool	isNull( int field );
    int         size();
    int         numRowsAffected();

private:
    QOCIPrivate*	d;
    QOCIResultPrivate*  cols;
    QSqlRecord          fs;
    bool                cacheNext( int r );
};
#endif //QOCI_USES_VERSION_9

class QOCIDriver : public QSqlDriver
{
public:
    QOCIDriver( QObject * parent=0, const char * name=0 );
    ~QOCIDriver();
    bool		hasFeature( DriverFeature f ) const;
    bool                open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null,
			      int port = -1 );
    void	        close();
    QSqlQuery	        createQuery() const;
    QStringList         tables( const QString& user ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecord          record( const QSqlQuery& query ) const;
    QSqlRecordInfo      recordInfo( const QString& tablename ) const;
    QSqlRecordInfo      recordInfo( const QSqlQuery& query ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QString             formatValue( const QSqlField* field,
				     bool trimStrings ) const;
    OCIEnv*             environment();
    OCISvcCtx*          serviceContext();

protected:
    bool	        beginTransaction();
    bool	        commitTransaction();
    bool	        rollbackTransaction();
private:
    void	        init();
    void	        cleanup();
    QOCIPrivate*        d;
};

#endif
