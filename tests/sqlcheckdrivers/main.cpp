#include <qstring.h>
#include <qvaluelist.h>
#include <qsqldatabase.h>

struct DriverTester {
    DriverTester( const QString& driver,
		  const QString& dbname,
		  const QString& user,
		  const QString& password,
		  const QString& host)
	: drv( driver ),
	  db( dbname ),
	  usr( user ),
	  pword( password ),
	  hst( host )
    {}
    DriverTester() {}
    QString drv;
    QString     db;
    QString   usr;
    QString	  pword;
    QString	  hst;
};

int main( int /*argc*/, char */*argv[]*/ )
{
    QValueList<DriverTester> tests;

    /*
      add connection tests here
    */
    tests.push_back( DriverTester( "QODBC3","post","db","","cocktail" ) );
    tests.push_back( DriverTester( "QOCI8","TROL","scott","tiger","anarki" ) );
    tests.push_back( DriverTester( "QPSQL7","test","db","db","cocktail" ) );
    tests.push_back( DriverTester( "QMYSQL3","trolldump","db","","l" ) );
    /*
      done adding connection tests
    */

    for ( uint i = 0; i < tests.size(); ++i ) {
	QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( tests[i].drv, tests[i].db );
	if ( defaultDB ) {
	    defaultDB->setDatabaseName( tests[i].db );
	    defaultDB->setUserName( tests[i].usr );
	    defaultDB->setPassword( tests[i].pword );
	    defaultDB->setHostName( tests[i].hst );
	    if ( defaultDB->open() ) {
		qWarning( "Driver test passed: " + tests[i].drv );
	    } else {
		qWarning( "Driver test FAILED: " + tests[i].drv );
	    }
	}
	QSqlDatabase::removeDatabase( tests[i].db );
    }

    return 0;
}
