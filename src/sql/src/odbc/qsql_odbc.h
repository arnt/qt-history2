#ifndef QSQL_ODBC_H
#define QSQL_ODBC_H

#include <qmap.h>
#include <qstring.h>
#include <qsqldriver.h>
#include <qsqlfield.h>
#include <qsqlresult.h>
#include <qsqlindex.h>

class QODBCPrivate;
class QODBCDriver;
class QODBCResult : public QSqlResult
{
    friend class QODBCDriver;
public:
    QODBCResult( const QODBCDriver * db, QODBCPrivate* p );
    ~QODBCResult();
protected:
    bool 	fetchFirst();
    bool 	fetchLast();
    bool 	fetch(int i);
    bool 	reset ( const QString& query );
    QVariant 	data( int field );
    bool	isNull( int field );
    int         size();
    int         numRowsAffected();
private:
    QODBCPrivate* 	d;
    typedef QMap<int,QVariant> FieldCache;
    FieldCache fieldCache;
    typedef QMap<int,bool> NullCache;
    NullCache nullCache;
};

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
    QSqlQuery 	        createQuery() const;
    QStringList         tables( const QString& user ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecord          record( const QSqlQuery& query ) const;
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

#endif
