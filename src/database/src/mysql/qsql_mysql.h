#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <qsqlresult.h>
#include <qsqlresultinfo.h>
#include <qsqldriver.h>

class QMySQLPrivate;
class QMySQLDriver;

class QMySQLResult : public QSqlResult
{
public:
    QMySQLResult( const QMySQLDriver* db );
    ~QMySQLResult();
    const QSqlResultInfo* 	info();
protected:
    void 		cleanup();
    bool 		fetch( int i );
    bool 		fetchLast();
    bool 		fetchFirst();
    QVariant 		data( int field );
    bool		isNull( int field ) const;
    bool 		reset ( const QString& query );
private:
    QMySQLPrivate* 	d;
    QSqlResultInfo* 	resultInfo;
};

class QMySQLResultInfo : public QSqlResultInfo
{
public:
    QMySQLResultInfo( QMySQLPrivate* p );
    ~QMySQLResultInfo();
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
    QSql		createResult() const;
    QStringList         tables( const QString& user ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QSqlFieldInfoList   fields( const QString& tablename ) const;
private:
    void		init();
    QMySQLPrivate* 	d;
};


#endif
