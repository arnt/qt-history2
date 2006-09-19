/* possible connection parameters */

#ifndef TST_DATABASES_H
#define TST_DATABASES_H

#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qregexp.h>
#include <qdir.h>
#include <qvariant.h>

#include <QtTest/QtTest>

#if defined (Q_OS_WIN) || defined (Q_OS_WIN32)
#include <qt_windows.h>
#else
#include <unistd.h>
#endif

#define CHECK_DATABASE( db ) \
    if ( !db.isValid() ) { qFatal( "db is Invalid" ); return; }

#define QVERIFY_SQL(q, stmt) QVERIFY2(stmt, qPrintable(q.lastError().text()))

// ### use QSystem::hostName if it is integrated in qtest/main
static QString qGetHostName()
{
    static QString hostname;
    if ( !hostname.isEmpty() )
	return hostname;

    char hn[257];
    if ( gethostname( hn, 255 ) == 0 ) {
        hn[256] = '\0';
	hostname = QString::fromLatin1( hn );
        hostname.replace(QLatin1Char('.'), QLatin1Char('_'));
    }
    return hostname;
}

// to prevent nameclashes on our database server, each machine
// will use its own set of table names. Call this function to get
// "tablename_hostname"
inline static QString qTableName( const QString& prefix )
{
    return prefix + "_" + qGetHostName();
}

class tst_Databases
{
public:
    tst_Databases(): counter(0)
    {
    }

    ~tst_Databases()
    {
	close();
    }

    // returns a testtable consisting of the names of all database connections if
    // driverPrefix is empty, otherwise only those that start with driverPrefix.
    int fillTestTable(const QString& driverPrefix = QString::null ) const
    {
        QTest::addColumn<QString>("dbName");
        int count = 0;

        for (int i = 0; i < dbNames.count(); ++i) {
	    QSqlDatabase db = QSqlDatabase::database( dbNames.at(i) );
	    if ( !db.isValid() )
		continue;
	    if ( driverPrefix.isEmpty() || db.driverName().startsWith( driverPrefix ) ) {
		QTest::newRow(dbNames.at(i).toLatin1()) << dbNames.at(i);
                ++count;
            }
	}

        return count;
    }

    void addDb( const QString& driver, const QString& dbName,
		const QString& user = QString::null, const QString& passwd = QString::null,
		const QString& host = QString::null, int port = -1, const QString params = QString::null )
    {
	QSqlDatabase db;

	if ( !QSqlDatabase::drivers().contains( driver ) ) {
	    qWarning("Driver %s is not installed", driver.toLatin1().constData());
	    return;
	}
	// construct a stupid unique name
	QString cName = QString::number( counter++ ) + "_" + driver + "@";
	cName += host.isEmpty() ? dbName : host;
	if ( port > 0 )
	    cName += ":" + QString::number( port );
	db = QSqlDatabase::addDatabase( driver, cName );
	if ( !db.isValid() ) {
	    qWarning( "Could not create database object" );
	    return;
	}
	db.setDatabaseName( dbName );
	db.setUserName( user );
	db.setPassword( passwd );
	db.setHostName( host );
	db.setPort( port );
	db.setConnectOptions( params );
	dbNames.append( cName );
    }

    void addDbs()
    {
//	addDb( "QOCI8", "PONY", "scott", "tiger" ); // Oracle 9i on horsehead
//	addDb( "QOCI8", "USTEST", "scott", "tiger", "" ); // Oracle 9i on horsehead
//	addDb( "QOCI8", "ICE", "scott", "tiger", "" ); // Oracle 8 on iceblink

//	addDb( "QTDS7", "testdb", "troll", "trondk", "horsehead" );

//	addDb( "QMYSQL3", "testdb", "troll", "trond", "horsehead.troll.no" );
//      addDb( "QMYSQL3", "testdb", "troll", "trond", "horsehead.troll.no", 3307 );
//	addDb( "QMYSQL3", "testdb", "troll", "trond", "horsehead.troll.no", 3308 ); // MySQL 4.1.1
//      addDb( "QMYSQL3", "testdb", "troll", "trond", "horsehead.troll.no", 3309 ); // MySQL 5.0.18 Linux
//      addDb( "QMYSQL3", "testdb", "troll", "trond", "iceblink.troll.no"); // MySQL 5.0.13 Windows

//	addDb( "QPSQL7", "testdb", "troll", "trond", "horsehead.troll.no" );
//	addDb( "QPSQL7", "testdb", "troll", "trond", "horsehead.troll.no", 5434 ); // multi-byte
//	addDb( "QPSQL7", "testdb", "troll", "trond", "horsehead.troll.no", 5435 ); // V7.3
//	addDb( "QPSQL7", "testdb", "troll", "trond", "horsehead.troll.no", 5436 ); // V7.4
//	addDb( "QPSQL7", "testdb", "troll", "trond", "horsehead.troll.no", 5437 ); // V8.0.3

//	addDb( "QDB2", "testdb", "troll", "trond", "horsehead" );
//	addDb( "QDB2", "unitest", "troll", "trond", "horsehead" );

	// yes - interbase really wants the physical path on the host machine.
//	addDb("QIBASE", "/opt/interbase/qttest.gdb", "SYSDBA", "masterkey", "horsehead.troll.no");
//	addDb("QIBASE", "c:\\qttest.gdb", "SYSDBA", "masterkey", "iceblink.troll.no");

	// use in-memory database to prevent local files
	addDb("QSQLITE", ":memory:");
//      addDb("QSQLITE", "/tmp/foo.db");

#if defined (Q_OS_WIN32)
//	addDb( "QODBC3", "DRIVER={SQL SERVER};SERVER=ICEBLINK\\ICEBLINK;", "troll", "trond", "" );
#endif //Q_OS_WIN32

#if 0
	if (testCase) {
	    QString dbPath = testCase->testPath() + "/qtest.mdb";
	    QString winDbPath = dbPath.replace("/", "\\");
	    // copy the Microsoft Access Database into a test-directory
	    if ( QSystem::copyFile("../qsqldatabase/testdata/qtest.mdb", dbPath, TRUE ) ) {
                addDb( "QODBC3", "DRIVER={Microsoft Access Driver (*.mdb)};"
			 "DBQ=" + winDbPath, "troll", "trond", "" );
		QSystem::setFileAttributes(dbPath, FALSE, FALSE);
	    } else {
	        qWarning("Unable to copy MS Access file, skipping testing of Access!");
	    }
	}
#endif //Q_OS_WIN32
    }

    void open()
    {
	addDbs();

	QStringList::Iterator it = dbNames.begin();
	while ( it != dbNames.end() ) {
	    QSqlDatabase db = QSqlDatabase::database( (*it), FALSE );
	    if ( db.isValid() && !db.isOpen() ) {
		if ( !db.open() ) {
		    qWarning( "tst_Databases: Unable to open %s on %s:\n%s", qPrintable(db.driverName()), qPrintable(*it), qPrintable(db.lastError().databaseText()));
		    // well... opening failed, so we just ignore the server, maybe it is not running
		    it = dbNames.erase( it );
		} else {
		    ++it;
		}
	    }
	}
    }

    void close()
    {
	for ( QStringList::Iterator it = dbNames.begin(); it != dbNames.end(); ++it ) {
            {
	        QSqlDatabase db = QSqlDatabase::database( (*it), FALSE );
	        if ( db.isValid() && db.isOpen() )
		    db.close();
            }
	    QSqlDatabase::removeDatabase( (*it) );
	}
	dbNames.clear();
    }

    // for debugging only: outputs the connection as string
    static QString dbToString( const QSqlDatabase db )
    {
	QString res = db.driverName() + "@";
	if ( db.driverName().startsWith( "QODBC" ) || db.driverName().startsWith( "QOCI" ) ) {
	    res += db.databaseName();
	} else {
	    res += db.hostName();
	}
	if ( db.port() > 0 ) {
	    res += ":" + QString::number( db.port() );
	}
	return res;
    }

    // drop a table only if it exists to prevent warnings
    static void safeDropTable( QSqlDatabase db, const QString& tableName )
    {
	if (db.tables().contains(tableName, Qt::CaseInsensitive)) {
	    QSqlQuery q("drop table " + tableName, db);
            if (!q.isActive())
                qWarning("unable to drop table %s: %s", tableName.toLocal8Bit().constData(),
                         q.lastError().text().toLocal8Bit().constData());
        }
    }

    static void safeDropView(QSqlDatabase db, const QString &viewName)
    {
        if (isMSAccess(db)) // Access is sooo stupid.
            safeDropTable(db, viewName);
        if (db.tables(QSql::Views).contains(viewName, Qt::CaseInsensitive)) {
            QSqlQuery q("drop view " + viewName, db);
            if (!q.isActive())
                qWarning("unable to drop view %s: %s", viewName.toLocal8Bit().constData(),
                         q.lastError().text().toLocal8Bit().constData());
        }
    }

    // returns the type name of the blob datatype for the database db.
    // blobSize is only used if the db doesn't have a generic blob type
    static QString blobTypeName( QSqlDatabase db, int blobSize = 10000 )
    {
	if ( db.driverName().startsWith( "QMYSQL" ) )
	    return "longblob";
	if ( db.driverName().startsWith( "QPSQL" ) )
	    return "bytea";
	if ( db.driverName().startsWith( "QTDS" )
	    || isSqlServer( db )
	    || isMSAccess( db ) )
	    return "image";
	if ( db.driverName().startsWith( "QDB2" ) )
	    return QString( "blob(%1)" ).arg( blobSize );
        if ( db.driverName().startsWith("QIBASE") )
            return QString( "blob sub_type 0 segment size 4096" );
	if ( db.driverName().startsWith( "QOCI" )
            || db.driverName().startsWith( "QSQLITE" ) )
	    return "blob";
	qDebug( "tst_Databases::blobTypeName: Don't know the blob type for %s",
                dbToString( db ).toLatin1().constData() );
	return "blob";
    }

    static QByteArray printError( const QSqlError& err )
    {
	return QString("'" + err.driverText() + "' || '" + err.databaseText() + "'").toLocal8Bit();
    }

    static bool isSqlServer( QSqlDatabase db ) {
	return db.databaseName().contains( "sql server", Qt::CaseInsensitive ) || db.databaseName().contains( "sqlserver", Qt::CaseInsensitive );
    }

    static bool isMSAccess( QSqlDatabase db ) {
	return db.databaseName().contains( "Access Driver", Qt::CaseInsensitive );
    }

    // -1 on fail, else Oracle version
    static int getOraVersion( QSqlDatabase db )
    {
	int ver = -1;
	QSqlQuery q( "SELECT banner FROM v$version", db );
	q.next();

	QRegExp vers("([0-9]+)\\.[0-9\\.]+[0-9]");
	if ( vers.indexIn( q.value( 0 ).toString() ) ) {
	    bool ok;
	    ver = vers.cap( 1 ).toInt( &ok );
	    if ( !ok )
		ver = -1;
	}
	return ver;
    }

    QStringList		   dbNames;
    int			   counter;
};

#endif

