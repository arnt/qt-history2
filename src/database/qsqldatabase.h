#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qsqlerror.h"
#include "qsqlresultinfo.h"
#include "qsqlindex.h"
#include "qsql.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSqlDatabasePrivate;
class QSqlDatabase : public QObject
{
    Q_OBJECT
public:
    QSqlDatabase( const QString& type, QObject * parent=0, const char * name=0 );
    QSqlDatabase( const QString& type,
    			const QString & db,
    			const QString & user = QString::null,
			const QString & password = QString::null,
			const QString & host = QString::null,
			QObject * parent=0,
			const char * name=0  );
    ~QSqlDatabase();
    void 	reset( const QString & db,
    			const QString & user = QString::null,
			const QString & password = QString::null,
			const QString & host = QString::null );
    bool	open();
    void	close();
    bool 	isOpen() const;
    bool 	isOpenError() const;
    bool    	hasTransactionSupport() const;
    QSql	query( const QString & sqlquery ) const;
    QStringList tables() const;
    QSqlIndex   primaryIndex( const QString& tablename ) const;
    QSqlFieldInfoList fields( const QString& tablename ) const;
    int		exec( const QString & sql ) const;
    QSql	createResult() const;
    bool	transaction();
    bool	commit();
    bool	rollback();
    QString 	databaseName() const;
    QString 	userName() const;
    QString 	password() const;
    QString 	hostName() const;
    QSqlError   lastError() const;
    QSqlDriver* driver() const;
private:
    void 	init( const QString& type );
    QSqlDatabasePrivate* d;
};

#endif // QT_NO_SQL
#endif
