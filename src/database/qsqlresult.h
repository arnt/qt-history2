#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qsqlerror.h"
#endif // QT_H

#if !defined(QT_NO_SQL)

class QSqlDriver;
class QSql;
struct QSqlResultShared;
class QSqlResultInfo;

class QSqlResult
{
friend class QSql;
friend struct QSqlResultShared;
public:
    enum Location {
	BeforeFirst = -1,
	AfterLast = -2
    };
    virtual ~QSqlResult();
protected:
    QSqlResult(const QSqlDriver * db );
    int		    at() const {return idx;}
    void            setAt( int at );
    void            setActive( bool a );
    void	    setLastError( const QSqlError& e );
    bool            isValid() const;
    bool            isActive() const;
    const QSqlDriver* driver() const {return sqldriver;}
    virtual QVariant data( int i ) = 0;
    virtual bool    isNull( int i ) const = 0;
    virtual bool    reset ( const QString& sqlquery ) = 0;
    virtual bool    fetch( int i ) = 0;
    virtual bool    fetchNext();
    virtual bool    fetchPrevious();
    virtual bool    fetchFirst() = 0;
    virtual bool    fetchLast() = 0;
    virtual const QSqlResultInfo* info() = 0;
private:
    const QSqlDriver*   sqldriver;
    int             idx;
    QString         sql;
    bool            active;
    QSqlError	    error;
    QSqlResult( const QSqlResult & );
    QSqlResult &operator=( const QSqlResult & );
};

#endif	// QT_NO_SQL
#endif
