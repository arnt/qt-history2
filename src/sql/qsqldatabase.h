/****************************************************************************
**
** Definition of QSqlDatabase class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlquery.h"
#ifdef QT_COMPAT
#include "qsqlrecord.h"
#endif
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlError;
class QSqlDriver;
class QSqlIndex;
class QSqlRecord;
class QSqlRecordInfo;
class QSqlDatabasePrivate;

class QM_EXPORT_SQL QSqlDriverCreatorBase
{
public:
    virtual QSqlDriver* createObject() const = 0;
};

template <class type>
class QSqlDriverCreator: public QSqlDriverCreatorBase
{
public:
    QSqlDriver* createObject() const { return new type; }
};

class QM_EXPORT_SQL QSqlDatabase : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlDatabase);

    Q_PROPERTY( QString databaseName  READ databaseName WRITE setDatabaseName )
    Q_PROPERTY( QString userName  READ userName WRITE setUserName )
    Q_PROPERTY( QString password  READ password WRITE setPassword )
    Q_PROPERTY( QString hostName  READ hostName WRITE setHostName )
    Q_PROPERTY( int port READ port WRITE setPort )
    Q_PROPERTY( QString connectOptions READ connectOptions WRITE setConnectOptions )

public:
    ~QSqlDatabase();

    bool		open();
    bool		open( const QString& user, const QString& password );
    void		close();
    bool		isOpen() const;
    bool		isOpenError() const;
    QStringList		tables() const;
    QStringList		tables( QSql::TableType type ) const;
    QSqlIndex		primaryIndex( const QString& tablename ) const;
    QSqlRecord		record( const QString& tablename ) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QSqlRecord record( const QSqlQuery& query ) const
    { return query.record(); }
    inline QT_COMPAT QSqlRecord recordInfo( const QString& tablename ) const
    { return record(tablename); }
    inline QT_COMPAT QSqlRecord recordInfo( const QSqlQuery& query ) const
    { return query.record(); }
#endif
    QSqlQuery		exec( const QString& query = QString() ) const;
    QSqlError		lastError() const;

    bool		transaction();
    bool		commit();
    bool		rollback();

    virtual void	setDatabaseName( const QString& name );
    virtual void	setUserName( const QString& name );
    virtual void	setPassword( const QString& password );
    virtual void	setHostName( const QString& host );
    virtual void	setPort( int p );
    void 		setConnectOptions( const QString& options = QString() );
    QString		databaseName() const;
    QString		userName() const;
    QString		password() const;
    QString		hostName() const;
    QString		driverName() const;
    int         	port() const;
    QString 		connectOptions() const;

    QSqlDriver*		driver() const;

    static QString defaultConnection;

    static QSqlDatabase* addDatabase( const QString& type, const QString& connectionName = defaultConnection );
    static QSqlDatabase* addDatabase( QSqlDriver* driver, const QString& connectionName = defaultConnection );
    static QSqlDatabase* database( const QString& connectionName = defaultConnection, bool open = TRUE );
    static void          removeDatabase( const QString& connectionName );
    static void          removeDatabase( QSqlDatabase* db );
    static bool          contains( const QString& connectionName = defaultConnection );
    static QStringList   drivers();
    static void          registerSqlDriver( const QString& name, QSqlDriverCreatorBase* creator );
    static bool 	 isDriverAvailable( const QString& name );

protected:
    QSqlDatabase(const QString& type, const QString& name, QObject * parent=0);
    QSqlDatabase(QSqlDriver* driver, QObject * parent=0);
private:
    void 	init( const QString& type, const QString& name );
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QSqlDatabase( const QSqlDatabase & );
    QSqlDatabase &operator=( const QSqlDatabase & );
#endif

};

#endif // QT_NO_SQL
#endif
