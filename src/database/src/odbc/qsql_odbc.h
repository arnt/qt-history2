#ifndef QSQL_ODBC_H
#define QSQL_ODBC_H

#include <qmap.h>
#include <qstring.h>
#include <qsqldriver.h>
#include <qsqlresult.h>
#include <qsqlresultinfo.h>
#include <qsqlindex.h>

class QODBCPrivate;
class QODBCResult;

class QODBCDriver : public QSqlDriver
{
public:
    QODBCDriver( QObject * parent=0, const char * name=0 );
    ~QODBCDriver();
    bool                open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null );
    void 	        close();
    QSql 	        createResult() const;
    QStringList         tables( const QString& user ) const;
    QSqlFieldList       fields( const QString& tablename ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
protected:
    bool                beginTransaction();
    bool                commitTransaction();
    bool                rollbackTransaction();
private:
    void init();
    bool endTrans();
    void cleanup();
    QODBCPrivate* d;
};

class QODBCResult : public QSqlResult
{
public:
    QODBCResult( const QODBCDriver * db, QODBCPrivate* p );
    ~QODBCResult();
    const QSqlResultInfo* info();
protected:
    bool 	fetchFirst();
    bool 	fetchLast();
    bool 	fetch(int i);
    bool 	reset ( const QString& query );
    QVariant 	data( int field );
    bool	isNull( int field ) const;
private:
    QODBCPrivate* 	d;
    QSqlResultInfo* 	resultInfo;
    typedef QMap<int,QVariant> FieldCache;
    FieldCache fieldCache;
    typedef QMap<int,bool> NullCache;
    NullCache nullCache;
};

class QODBCResultInfo : public QSqlResultInfo
{
public:
    QODBCResultInfo( const QODBCPrivate* d );
    ~QODBCResultInfo();
};

#endif
