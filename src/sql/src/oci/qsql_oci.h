#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>
#include <qstring.h>

class QOCIPrivate;
class QOCIResultPrivate;
class QOCIDriver;
class QOCIResult : public QSqlResult
{
    friend class QOCIDriver;
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
    bool	isNull( int field );
    int                 size();
    int                 affectedRows();
private:
    typedef QMap< uint, QVariant > RowCache;
    typedef QMap< uint, RowCache > RowsetCache;

    QOCIPrivate* 	d;
    QOCIResultPrivate*  cols;
    RowsetCache     	rowCache;
    QSqlRecord          fs;
    bool                cached;
    bool                cacheNext();
};

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
    QSqlQuery 	        createResult() const;
    QStringList         tables( const QString& user ) const;
    QSqlRecord          fields( const QString& tablename ) const;
    QSqlRecord          fields( const QSqlQuery& query ) const;        
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QString             formatValue( const QSqlField* field ) const;
protected:
    bool    	        beginTransaction();
    bool    	        commitTransaction();
    bool    	        rollbackTransaction();
private:
    void 	        init();
    void 	        cleanup();
    QOCIPrivate*        d;
};

#endif
