#ifndef QSQL_PSQL_H
#define QSQL_PSQL_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>

class QPSQLPrivate;
class QPSQLDriver;

class QPSQLResult : public QSqlResult
{
public:
    QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p );
    ~QPSQLResult();
protected:
    void 		cleanup();
    bool 		fetch( int i );
    bool 		fetchFirst();
    bool 		fetchLast();
    QVariant            data( int i );
    bool		isNull( int field );
    bool 		reset ( const QString& query );
    QSqlFieldList       fields();
    int                 size();
    int                 affectedRows();
private:
    int			currentSize;
    QPSQLPrivate* 	d;
    bool                binary;
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
    QStringList         tables( const QString& user ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QSqlFieldList       fields( const QString& tablename ) const;
protected:
    bool    		beginTransaction();
    bool    		commitTransaction();
    bool    		rollbackTransaction();
private:
    void		init();
    QPSQLPrivate* 	d;
};


#endif
