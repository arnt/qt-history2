#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qsqlerror.h"
#include "qsqlresultinfo.h"
#include "qsql.h"
#endif // QT_H

#if !defined(QT_NO_SQL)

class QSqlViewBase // ### possibly useless?
{
public:
    QSqlViewBase();
    virtual ~QSqlViewBase();
    QSqlFieldInfoList     fields() const {return fieldList;}
protected:
    void	setFields( const QSqlFieldInfoList& fields ) {fieldList = fields;}
private:
    QSqlFieldInfoList fieldList;
};

class QSqlView : public QSqlViewBase // ### possibly useless?
{
public:
    QSqlView( const QString& sql, const QSqlDriver* db );
    ~QSqlView();
};

class QSqlTable : public QSqlViewBase // ### possibly useless?
{
public:
    QSqlTable( const QString& name, const QSqlDriver* db );
    ~QSqlTable();
};

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
    QSqlView    view( const QString & sql ) const;
    QSqlTable   table( const QString & name ) const;
    QStringList tables() const;
    int		exec( const QString & sql ) const;
    QSql	createResult() const;
    bool	transaction();
    bool	commit();
    bool	rollback();
    QString 	databaseName();
    QString 	userName();
    QString 	password();
    QString 	hostName();
    QSqlError   lastError() const;
    QSqlDriver* driver() const;
private:
    void 	init( const QString& type );
    QSqlDatabasePrivate* d;
};

#endif // QT_NO_SQL
#endif
