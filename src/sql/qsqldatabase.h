/****************************************************************************
**
** Definition of QSqlDatabase class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#include "qfeatures.h"

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlquery.h"
#include "qstringlist.h"
#endif // QT_H

class QSqlError;
class QSqlDriver;
class QSqlIndex;
class QSqlRecord;

class Q_EXPORT QSqlDatabase : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString databaseName  READ databaseName WRITE setDatabaseName )
    Q_PROPERTY( QString userName  READ userName WRITE setUserName )
    Q_PROPERTY( QString password  READ password WRITE setPassword )
    Q_PROPERTY( QString hostName  READ hostName WRITE setHostName )
    Q_PROPERTY( int port  READ port WRITE setPort )

public:
    ~QSqlDatabase();

    bool	 open();
    bool	 open( const QString& user, const QString& password );
    void	 close();
    bool	 isOpen() const;
    bool	 isOpenError() const;
    QStringList  tables() const;
    QSqlIndex    primaryIndex( const QString& tablename ) const;
    QSqlRecord   record( const QString& tablename ) const;
    QSqlRecord   record( const QSqlQuery& query ) const;
    QSqlQuery	 exec( const QString& query = QString::null ) const;
    QSqlError    lastError() const;

    bool	 transaction();
    bool	 commit();
    bool	 rollback();

    virtual void setDatabaseName( const QString& name );
    virtual void setUserName( const QString& name );
    virtual void setPassword( const QString& password );
    virtual void setHostName( const QString& host );
    virtual void setPort( int p );
    QString	 databaseName() const;
    QString	 userName() const;
    QString	 password() const;
    QString	 hostName() const;
    QString      driverName() const;
    int          port() const;

    QSqlDriver*  driver() const;

    // MOC_SKIP_BEGIN
    QT_STATIC_CONST char * const defaultConnection;
    // MOC_SKIP_END

    static QSqlDatabase* addDatabase( const QString& type, const QString& connectionName = defaultConnection );
    static QSqlDatabase* database( const QString& connectionName = defaultConnection, bool open = TRUE );
    static void          removeDatabase( const QString& connectionName );
    static bool          contains( const QString& connectionName = defaultConnection );
    static QStringList   drivers();

protected:
    QSqlDatabase( const QString& type, const QString& name, QObject * parent=0, const char * objname=0 );
private:
    void	init( const QString& type, const QString& name );
    class Private;
    Private* d;
};

#endif // QT_NO_SQL
#endif
