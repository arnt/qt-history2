#ifndef QSQL_H
#define QSQL_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#include "qsqlconnection.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSqlResult;
class QSqlResultInfo;

struct Q_EXPORT QSqlResultShared : public QShared
{
    QSqlResultShared( QSqlResult* result )
    : sqlResult(result)
    {}
    virtual ~QSqlResultShared();
    QSqlResult* sqlResult;
};

class Q_EXPORT QSql
{
public:
    QSql( QSqlResult * r );
    QSql( const QString& query = QString::null, const QString& databaseName = QSqlConnection::defaultDatabase );
    QSql( const QSql& other );
    virtual ~QSql();
    QSql& operator=( const QSql& other );
    bool            isValid() const;
    bool            isActive() const;
    bool	    isNull( int field ) const;
    int             at() const;
    QString         query() const;
    bool	    setQuery ( const QString& query );
    QVariant 	    operator[] ( int i );
    QVariant        value( int i );
    bool	    seek( int i, bool relative = FALSE );
    bool    	    next();
    bool    	    previous();
    bool    	    first();
    bool    	    last();
    QSqlFieldList   fields() const;
    int             size() const;
    int             affectedRows() const;
    QSqlError	    lastError() const;
    bool            isSelect() const;
    const QSqlDriver*   driver() const;
private:
    void            deref();
    bool            checkDetach();
    QSqlResultShared* d;
};

#endif	// QT_NO_SQL
#endif
