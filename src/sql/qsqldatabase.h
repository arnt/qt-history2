#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qsqlquery.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSqlQuery;
class QSqlDatabasePrivate;
class Q_EXPORT QSqlDatabase : public QObject
{
    Q_OBJECT
public:
    static QSqlDatabase* addDatabase( const QString& type, const QString& name = defaultDatabase );
    static QSqlDatabase* database( const QString& name = defaultDatabase );
    
    QT_STATIC_CONST char * const defaultDatabase;
    
    ~QSqlDatabase();

    bool	open();
    bool	open( const QString& user, const QString& password );
    void	close();
    bool 	isOpen() const;
    bool 	isOpenError() const;
    bool    	hasTransactionSupport() const;
    QSqlQuery	query( const QString & sqlquery ) const;
    QStringList tables() const;
    QSqlIndex   primaryIndex( const QString& tablename ) const;
    QSqlRecord  fields( const QString& tablename ) const;
    QSqlRecord  fields( const QSqlQuery& query ) const;
    int		exec( const QString & sql ) const;
    QSqlQuery	createResult() const;
    bool	transaction();
    bool	commit();
    bool	rollback();

    void        setDatabaseName( const QString& name );
    QString 	databaseName() const;
    virtual void setUserName( const QString& name );
    QString 	userName() const;
    virtual void setPassword( const QString& password );
    QString 	password() const;
    virtual void setHostName( const QString& host );
    QString 	hostName() const;

    QSqlError   lastError() const;
    QSqlDriver* driver() const;
protected:
    QSqlDatabase( const QString& type, const QString& name, QObject * parent=0, const char * objname=0 );
   
private:
    void 	init( const QString& type, const QString& name );
    QSqlDatabasePrivate* d;
};

#endif // QT_NO_SQL
#endif
