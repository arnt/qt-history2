#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlerror.h"
#include "qsql.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver : public QObject
{
    Q_OBJECT
public:
    QSqlDriver( QObject * parent=0, const char * name=0 );
    ~QSqlDriver();
    virtual QSql    query( const QString & sqlquery ) const;
    bool 	    isOpen() const;
    bool 	    isOpenError() const;
    bool    	    hasTransactionSupport() const;
    virtual bool    beginTransaction();
    virtual bool    commitTransaction();
    virtual bool    rollbackTransaction();
    virtual QStringList   tables( const QString& user ) const;
    virtual QSqlIndex     primaryIndex( const QString& tablename ) const;
    virtual QSqlFieldList fields( const QString& tablename ) const;
    QSqlError	    lastError() const;
    virtual bool    open( const QString & db,
    			const QString & user = QString::null,
			const QString & password = QString::null,
			const QString & host = QString::null ) = 0;
    virtual void    close() = 0;
    virtual QSql    createResult() const = 0;
protected:
    void 	    setOpen( bool o );
    void 	    setOpenError( bool e );
    void	    setTransactionSupport( bool t );
    void	    setLastError( const QSqlError& e );
private:
    int 	    dbState;
    bool 	    hasTrans;
    QSqlError 	    error;
#if defined(Q_DISABLE_COPY)
    QSqlDriver( const QSqlDriver & );
    QSqlDriver &operator=( const QSqlDriver & );
#endif
};

#endif	// QT_NO_SQL
#endif
