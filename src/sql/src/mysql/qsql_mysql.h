#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include "../../qsqldriver.h"
#include "../../qsqlresult.h"
#include "../../qsqlfield.h"
#include "../../qsqlindex.h"

class QMySQLPrivate;
class QMySQLDriver;

class QMySQLResult : public QSqlResult
{
    friend class QMySQLDriver;
public:
    QMySQLResult( const QMySQLDriver* db );
    ~QMySQLResult();
protected:
    void 		cleanup();
    bool 		fetch( int i );
    bool 		fetchLast();
    bool 		fetchFirst();
    QVariant 		data( int field );
    bool		isNull( int field );
    bool 		reset ( const QString& query );
    int                 size();
    int                 numRowsAffected();
private:
    QMySQLPrivate* 	d;
};

class QMySQLDriver : public QSqlDriver
{
    friend class QMySQLResult;
public:
    QMySQLDriver( QObject * parent=0, const char * name=0 );
    ~QMySQLDriver();
    bool    		open( const QString & db,
    				const QString & user = QString::null,
				const QString & password = QString::null,
				const QString & host = QString::null );
    void 		close();
    QSqlQuery		createQuery() const;
    QStringList         tables( const QString& user ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecord          record( const QSqlQuery& query ) const;
private:
    void		init();
    QMySQLPrivate* 	d;
};


#endif
