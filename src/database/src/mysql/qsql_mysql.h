#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <qsqldriver.h>
#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqlindex.h>

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
    QSqlResultFields    fields();
    int                 size() const;
    int                 affectedRows() const;
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
    QSql		createResult() const;
    QStringList         tables( const QString& user ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QSqlFieldList       fields( const QString& tablename ) const;
private:
    void		init();
    QMySQLPrivate* 	d;
};


#endif
