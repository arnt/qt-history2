/****************************************************************************
**
** Definition of QSqlDatabase class
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

#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlquery.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlError;
class QSqlDriver;
class QSqlIndex;
class QSqlRecord;
class QSqlDatabasePrivate;

class Q_EXPORT QSqlDatabase : public QObject
{
    Q_OBJECT
public:
    QT_STATIC_CONST char * const defaultDatabase;

    static QSqlDatabase* addDatabase( const QString& type, const QString& name = defaultDatabase );
    static QSqlDatabase* database( const QString& name = defaultDatabase, bool open = TRUE );
    static void          removeDatabase( const QString& name );

    ~QSqlDatabase();

    bool	 open();
    bool	 open( const QString& user, const QString& password );
    void	 close();
    bool 	 isOpen() const;
    bool 	 isOpenError() const;
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
    QString 	 databaseName() const;
    QString 	 userName() const;
    QString 	 password() const;
    QString 	 hostName() const;

    QSqlDriver*  driver() const;
protected:
    QSqlDatabase( const QString& type, const QString& name, QObject * parent=0, const char * objname=0 );
private:
    void 	init( const QString& type, const QString& name );
    QSqlDatabasePrivate* d;
};

#endif // QT_NO_SQL
#endif
