#ifndef QSQL_MSQL_H
#define QSQL_MSQL_H

#if defined(QT_SQL_SUPPORT)
#if defined(QT_SQL_MSQL_SUPPORT)

#include "qsql_base.h"

class QMSQLPrivate;
class QMSQLDriver;

class QMSQLResult : public QSqlResult
{
public:
    QMSQLResult( const QMSQLDriver* db );
    ~QMSQLResult();
    const QSqlResultInfo* 	info();
protected:
    void 		cleanup();
    bool 		fetch( int i );
    bool 		fetchFirst();
    bool 		fetchLast();
    QString 		string( int field );
    QByteArray          binary( int field );
    bool		isNull( int i ) const;
    bool 		reset ( const QString& query );
private:
    QMSQLPrivate* 	d;
    QSqlResultInfo* 	resultInfo;
};

class QMSQLResultInfo : public QSqlResultInfo
{
public:
    QMSQLResultInfo( QMSQLPrivate* p );
    ~QMSQLResultInfo();
};

class QMSQLDriver : public QSqlDriver
{
friend class QMSQLResult;
public:
    QMSQLDriver( QObject * parent=0, const char * name=0 );
    ~QMSQLDriver();
    bool    		open( const QString & db,
    				const QString & user = QString::null,
				const QString & password = QString::null,
				const QString & host = QString::null );
    bool 		open();
    void 		close();
    QSql		createResult() const;
private:
    void		init();
    QMSQLPrivate* 	d;
};


#endif
#endif
#endif
