#ifndef QSQL_PSQL_H
#define QSQL_PSQL_H

#include <qsqlresult.h>
#include <qsqlresultinfo.h>
#include <qsqldriver.h>

class QPSQLPrivate;
class QPSQLDriver;

class QPSQLResult : public QSqlResult
{
public:
    QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p );
    ~QPSQLResult();
    const QSqlResultInfo* 	info();
protected:
    void 		cleanup();
    bool 		fetch( int i );
    bool 		fetchFirst();
    bool 		fetchLast();
    QVariant            data( int i );
    bool		isNull( int field ) const;
    bool 		reset ( const QString& query );
private:
    int			currentSize;
    QPSQLPrivate* 	d;
    QSqlResultInfo* 	resultInfo;
};

class QPSQLResultInfo : public QSqlResultInfo
{
public:
    QPSQLResultInfo( QPSQLPrivate* p );
    ~QPSQLResultInfo();
};

class QPSQLDriver : public QSqlDriver
{
public:
    QPSQLDriver( QObject * parent=0, const char * name=0 );
    ~QPSQLDriver();
    bool        	open( const QString & db,
    			const QString & user = QString::null,
			const QString & password = QString::null,
			const QString & host = QString::null );
    void 		close();
    QSql		createResult() const;
protected:
    bool    		beginTransaction();
    bool    		commitTransaction();
    bool    		rollbackTransaction();
private:
    void		init();
    QPSQLPrivate* 	d;
};


#endif
