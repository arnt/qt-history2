#ifndef QSQL_H
#define QSQL_H

#if defined(QT_SQL_SUPPORT)

#include <qstring.h>
#include <qvaluelist.h>
#include <qvariant.h>
#include <qsqlerror.h>

class QSqlDriver;
class QSqlResult;
class QSqlResultInfo;

typedef QMap< uint, QVariant > QSqlRow;
typedef QMap< uint, QSqlRow > QSqlRowset;

struct QSqlResultShared : public QShared
{
    QSqlResultShared( QSqlResult* result = 0 )
    : sqlResult(result)
    {}
    virtual ~QSqlResultShared();
    QSqlResult* sqlResult;
};

class QSql
{
public:
    explicit QSql( QSqlResult * r )
    {
	d = new QSqlResultShared( r );
    }
    virtual ~QSql();
    QSql( const QSql& other )
    : d(other.d)
    {
    	d->ref();
    }
    QSql& operator=( const QSql& other )
    {
	other.d->ref();
	deref();
	d = other.d;
	return *this;
    }
    bool            isValid() const;
    bool            isActive() const;
    bool	    isNull( int field ) const;
    int             at() const;
    QString         query() const;
    const QSqlDriver*   driver() const;
    bool	    operator<< ( const QString& query );
    QVariant 	    operator[] ( int i );
    bool	    seek( int i, bool relative = FALSE );
    bool    	    next();
    bool    	    previous();
    bool    	    first();
    bool    	    last();
    QSqlError	    lastError() const;
    const QSqlResultInfo* info();
private:
    void deref()
    {
	if ( d->deref() ) {
	    delete d;
	    d = 0;
	}
    }
    QSqlResultShared* d;
};

#endif
#endif
