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
    QSqlQuery&          operator=( const QSqlQuery& other );    
    virtual ~QSqlQuery();

    bool                isValid() const;
    bool                isActive() const;
    bool	        isNull( int field ) const;
    int                 at() const;
    QString             lastQuery() const;
    int                 numRowsAffected() const;
    QSqlError	        lastError() const;
    bool                isSelect() const;
    int                 size() const;    
    const QSqlDriver*   driver() const;
    const QSqlResult*   result() const;
    
    virtual bool	exec ( const QString& query );
    virtual QVariant    value( int i );
    
    virtual bool	seek( int i, bool relative = FALSE );
    virtual bool        next();
    virtual bool        prev();
    virtual bool        first();
    virtual bool        last();
    
protected:
    virtual void        beforeSeek();
    virtual void        afterSeek();

private:
    void                deref();
    bool                checkDetach();
    QSqlResultShared*   d;
};


#endif // QT_NO_SQL
#endif
