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
struct QSqlResultShared;
class QSqlResultInfo;
class QSqlResultPrivate;
class Q_EXPORT QSqlResult
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
    int		    at() const;
    void            setAt( int at );
    void            setActive( bool a );
    void	    setLastError( const QSqlError& e );
    void            setQuery( const QString& query );
    QString         query() const;
    QSqlError       lastError() const;
    bool            isValid() const;
    bool            isActive() const;
    void            setSelect( bool s );
    bool            isSelect() const;
    const QSqlDriver* driver() const;
    virtual QVariant data( int i ) = 0;
    virtual bool    isNull( int i ) = 0;
    virtual bool    reset ( const QString& sqlquery ) = 0;
    virtual bool    fetch( int i ) = 0;
    virtual bool    fetchNext();
    virtual bool    fetchPrevious();
    virtual bool    fetchFirst() = 0;
    virtual bool    fetchLast() = 0;
    virtual QSqlFieldList   fields() = 0;
    virtual int             size() = 0;
    virtual int             affectedRows() = 0;
private:
    QSqlResultPrivate* d;
    QSqlResult( const QSqlResult & );
    QSqlResult &operator=( const QSqlResult & );
};

#endif	// QT_NO_SQL
#endif
