#ifndef QSQLQUERY_H
#define QSQLQUERY_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSqlResult;
class QSqlResultInfo;
class QSqlDatabase;

struct Q_EXPORT QSqlResultShared : public QShared
{
    QSqlResultShared( QSqlResult* result )
    : sqlResult(result)
    {}
    virtual ~QSqlResultShared();
    QSqlResult* sqlResult;
};

class Q_EXPORT QSqlQuery
{
public:
    QSqlQuery( QSqlResult * r );
    QSqlQuery( const QString& query = QString::null, QSqlDatabase* db = 0 );
    QSqlQuery( const QSqlQuery& other );
    virtual ~QSqlQuery();
    QSqlQuery&            operator=( const QSqlQuery& other );
    bool             isValid() const;
    bool             isActive() const;
    bool	     isNull( int field ) const;
    int              at() const;
    QString          lastQuery() const;
    bool	     exec ( const QString& query );
    virtual QVariant value( int i );
    bool	     seek( int i, bool relative = FALSE );
    bool    	     next();
    bool    	     previous();
    bool    	     first();
    bool    	     last();
    int              size() const;
    int              affectedRows() const;
    QSqlError	     lastError() const;
    bool             isSelect() const;
    const QSqlDriver*   driver() const;
    const QSqlResult*   result() const;
protected:
    virtual void     preSeek();
    virtual void     postSeek();

private:
    void             deref();
    bool             checkDetach();
    QSqlResultShared* d;
};


#endif // QT_NO_SQL
#endif
