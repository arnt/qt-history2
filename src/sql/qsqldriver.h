#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;

class Q_EXPORT QSqlDriver : public QObject
{
    friend class QSqlDatabase;
    Q_OBJECT
public:
    QSqlDriver( QObject * parent=0, const char * name=0 );
    ~QSqlDriver();

    bool 	          isOpen() const;
    bool 	          isOpenError() const;

    virtual bool          beginTransaction();
    virtual bool          commitTransaction();
    virtual bool          rollbackTransaction();
    virtual QStringList   tables( const QString& user ) const;
    virtual QSqlIndex     primaryIndex( const QString& tablename ) const;
    virtual QSqlRecord    record( const QString& tablename ) const;
    virtual QSqlRecord    record( const QSqlQuery& query ) const;
    virtual QString       nullText() const;
    virtual QString       formatValue( const QSqlField* field ) const;
    QSqlError	          lastError() const;

    virtual bool          hasTransactionSupport() const = 0;
    virtual bool          hasQuerySizeSupport() const = 0;
    virtual bool          canEditBinaryFields() const = 0;
    virtual bool          open( const QString & db,
				const QString & user = QString::null,
				const QString & password = QString::null,
				const QString & host = QString::null ) = 0;
    virtual void          close() = 0;
    virtual QSqlQuery     createQuery() const = 0;

protected:
    virtual void          setOpen( bool o );
    virtual void          setOpenError( bool e );
    virtual void	  setLastError( const QSqlError& e );
private:
    int 	          dbState;
    QSqlError 	          error;
#if defined(Q_DISABLE_COPY)
    QSqlDriver( const QSqlDriver & );
    QSqlDriver &operator=( const QSqlDriver & );
#endif
};

#endif	// QT_NO_SQL
#endif
