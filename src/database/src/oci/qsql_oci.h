#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>
#include <qstring.h>

class QOCIPrivate;
class QOCIResult;

class QOCIDriver : public QSqlDriver
{
public:
    QOCIDriver( QObject * parent=0, const char * name=0 );
    ~QOCIDriver();
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
    bool    	        beginTransaction();
    bool    	        commitTransaction();
    bool    	        rollbackTransaction();
private:
    void 	        init();
    bool 	        endTrans();
    void 	        cleanup();
    QOCIPrivate*        d;
};

class  QOCIResultPrivate;
class QOCIResult : public QSqlResult
{
public:
    QOCIResult( const QOCIDriver * db, QOCIPrivate* p );
    ~QOCIResult();
protected:
    bool	fetchNext();
    bool 	fetchFirst();
    bool 	fetchLast();
    bool 	fetch(int i);
    bool 	reset ( const QString& query );
    QVariant 	data( int field );
    bool	isNull( int field ) const;
    QSqlFieldList       fields() const;
    int                 size() const;
    int                 affectedRows() const;
private:
    typedef QMap< uint, QVariant > RowCache;
    typedef QMap< uint, RowCache > RowsetCache;
    
    QOCIPrivate* 	d;
    QOCIResultPrivate*  cols;
    RowsetCache     	rowCache;
};

#endif
