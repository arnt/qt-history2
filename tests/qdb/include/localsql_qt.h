/*
    Xbase project source code

    This file contains the LocalSQL Qt SQL driver implementation

    Copyright (C) 2000 Dave Berton (db@trolltech.com)
		       Jasmin Blanchette (jasmin@trolltech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef LOCALSQL_QT_H
#define LOCALSQL_QT_H

#include <qsqlresult.h>
#include <qsqldriver.h>

class LocalSQLDriver;

class LocalSQLResult : public QSqlResult
{
public:
    LocalSQLResult( const LocalSQLDriver* db, const QString& path );
    ~LocalSQLResult();
protected:
    void		cleanup();
    bool		fetch( int i );
    bool		fetchFirst();
    bool		fetchLast();
    QVariant            data( int i );
    bool		isNull( int field );
    bool		reset ( const QString& query );
    int                 size();
    int                 numRowsAffected();
private:
    class LocalSQLPrivate;
    LocalSQLPrivate*	d;
};

class LocalSQLDriver : public QSqlDriver
{
public:
    LocalSQLDriver( QObject * parent=0, const char * name=0 );
    ~LocalSQLDriver();
    bool	        hasTransactionSupport() const;
    bool                hasQuerySizeSupport() const;
    bool                canEditBinaryFields() const;
    bool	        open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null,
			      int port = -1 );
    void		close();
    QSqlQuery		createQuery() const;
    QStringList         tables( const QString& user ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecord          record( const QSqlQuery& query ) const;

protected:
    bool		beginTransaction();
    bool		commitTransaction();
    bool		rollbackTransaction();
private:
    void		init();
    QString             databasePath;
};

#endif
