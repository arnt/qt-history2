#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqlresultinfo.h>
#include <qsqldriver.h>
#include <qstring.h>

class QOCIPrivate;
class QOCIResult;

class QOCIDriver : public QSqlDriver
{
public:
    QOCIDriver( QObject * parent=0, const char * name=0 );
    ~QOCIDriver();
    bool        open( const QString & db,
    			const QString & user = QString::null,
			const QString & password = QString::null,
			const QString & host = QString::null );
    void 	close();
    QSql 	createResult() const;
    QStringList tables() const;
protected:
    bool    	beginTransaction();
    bool    	commitTransaction();
    bool    	rollbackTransaction();
private:
    void 	init();
    bool 	endTrans();
    void 	cleanup();
    QOCIPrivate* d;
};

class  QOCIResultPrivate;
class QOCIResult : public QSqlResult
{
public:
    QOCIResult( const QOCIDriver * db, QOCIPrivate* p );
    ~QOCIResult();
    const QSqlResultInfo* info();
protected:
    bool	fetchNext();
    bool 	fetchFirst();
    bool 	fetchLast();
    bool 	fetch(int i);
    bool 	reset ( const QString& query );
    QVariant 	data( int field );
    bool	isNull( int field ) const;
private:
    QOCIPrivate* 	d;
    QOCIResultPrivate*  cols;
    QSqlResultInfo* 	resultInfo;
    QSqlRowset     	rowCache;
};

class QOCIResultInfo : public QSqlResultInfo
{
public:
    QOCIResultInfo( const QOCIPrivate* d );
    ~QOCIResultInfo();
};

#endif
