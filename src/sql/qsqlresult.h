#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSql;
class QSqlResultInfo;
class QSqlResultPrivate;
struct QSqlResultShared;

class Q_EXPORT QSqlResult
{
friend class QSqlQuery;
friend struct QSqlResultShared;
public:
    enum Location {
	BeforeFirst = -1,
	AfterLast = -2
    };
    virtual ~QSqlResult();
protected:
    QSqlResult(const QSqlDriver * db );
    int		    at() const;
    QString         lastQuery() const;
    QSqlError       lastError() const;
    bool            isValid() const;
    bool            isActive() const;
    bool            isSelect() const;
    const QSqlDriver* driver() const;
    virtual void    setAt( int at );
    virtual void    setActive( bool a );
    virtual void    setLastError( const QSqlError& e );
    virtual void    setQuery( const QString& query );
    virtual void    setSelect( bool s );
    virtual QVariant data( int i ) = 0;
    virtual bool    isNull( int i ) = 0;
    virtual bool    reset ( const QString& sqlquery ) = 0;
    virtual bool    fetch( int i ) = 0;
    virtual bool    fetchNext();
    virtual bool    fetchPrev();
    virtual bool    fetchFirst() = 0;
    virtual bool    fetchLast() = 0;
    virtual int     size() = 0;
    virtual int     numRowsAffected() = 0;
private:
    QSqlResultPrivate* d;
    QSqlResult( const QSqlResult & );
    QSqlResult &operator=( const QSqlResult & );
};

#endif	// QT_NO_SQL
#endif
