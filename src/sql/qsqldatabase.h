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
class QSqlDatabasePrivate;
class Q_EXPORT QSqlDatabase : public QObject
{
    Q_OBJECT
public:
    static QSqlDatabase* addDatabase( const QString& type, const QString& name = defaultDatabase );
    static QSqlDatabase* database( const QString& name = defaultDatabase );

    ~QSqlDatabase();

    QT_STATIC_CONST char * const defaultDatabase;

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
