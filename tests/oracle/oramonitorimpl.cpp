#include "oramonitorimpl.h"
#include "configdialog.h"
#include "qsettings.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "qmessagebox.h"
#include "qprogressbar.h"
#include "qlabel.h"

OraMonitorImpl::OraMonitorImpl( QWidget* parent, const char* name, WFlags f ) : OraMonitor( parent, name, f )
{
    QSettings config;
    
    connect( &monitorTimer, SIGNAL( timeout() ), this, SLOT( timerFired() ) );

    config.insertSearchPath( QSettings::Windows, "/Breiflabb" );
    net8Name = config.readEntry( "/OraMonitor/Net8Name" );
    if( !net8Name.length() ) {
	clickedConfig();
	return;
    }
    userName = config.readEntry( "/OraMonitor/Username" );
    password = config.readEntry( "/OraMonitor/Password" );
    initDB();
}

OraMonitorImpl::~OraMonitorImpl()
{
}

void OraMonitorImpl::clickedConfig()
{
    ConfigDialog* dlg = new ConfigDialog( this );
    
    if( dlg->exec() ) {
	QSettings config;
	config.insertSearchPath( QSettings::Windows, "/Breiflabb" );
	net8Name = config.readEntry( "/OraMonitor/Net8Name" );
	if( !net8Name.length() )
	    return;
	userName = config.readEntry( "/OraMonitor/Username" );
	password = config.readEntry( "/OraMonitor/Password" );
	initDB();
    }
}

void OraMonitorImpl::initDB()
{
    if( monitorTimer.isActive() )
	monitorTimer.stop();

    database = QSqlDatabase::addDatabase( "QOCI8" );
    if( !database )
	return;
    database->setDatabaseName( net8Name );
    database->setUserName( userName );
    database->setPassword( password );
    
    if( database->open() ) {
	monitorTimer.start( 5000 );
	timerFired();
    }
    else
	QMessageBox::warning( this, "Warning", QString( "Could not open database: " ) + database->lastError().driverText() + " : " + database->lastError().databaseText() );
}

void OraMonitorImpl::destroy()
{
    if( database )
	if( database->isOpen() )
	    database->close();
}

void OraMonitorImpl::timerFired()
{
    // Calculate the buffer cache hit ratio
    {
	double physicalReads( 0 ), blockGets( 1 ), consistentGets( 1 );
	QSqlQuery q( "SELECT value FROM v$sysstat WHERE name = 'physical reads'" );
	if( q.isActive() ) {
	    q.next();
	    physicalReads = q.value( 0 ).toDouble();
	}
	q.exec( "SELECT value FROM v$sysstat WHERE name = 'db block gets'" );
	if( q.isActive() ) {
	    q.next();
	    blockGets = q.value( 0 ).toDouble();
	}
	q.exec( "SELECT value FROM v$sysstat WHERE name = 'consistent gets'" );
	if( q.isActive() ) {
	    q.next();
	    consistentGets = q.value( 0 ).toDouble();
	}
	BufferCache->setProgress( 1000 * ( 1.0 - physicalReads / ( blockGets + consistentGets ) ) );
    }

    // Calculate the library cache hit ratio
    {
	double pins( 0 ), reloads( 0 );
	QSqlQuery q( "SELECT SUM( pins ) FROM v$librarycache" );
	if( q.isActive() ) {
	    q.next();
	    pins = q.value( 0 ).toDouble();
	}
	q.exec( "SELECT SUM( reloads ) FROM v$librarycache" );
	if( q.isActive() ) {
	    q.next();
	    reloads = q.value( 0 ).toDouble();
	}
	LibraryCache->setProgress( 1000 * ( pins - reloads ) / pins );
    }

    // Calculate the dictionary cache hit ratio
    {
	double gets( 1 ), getmisses( 0 ), usage( 0 ), fixed( 0 );
	QSqlQuery q( "SELECT SUM( gets ) FROM v$rowcache" );
	if( q.isActive() ) {
	    q.next();
	    gets = q.value( 0 ).toDouble();
	}
	q.exec( "SELECT SUM( getmisses ) FROM v$rowcache" );
	if( q.isActive() ) {
	    q.next();
	    getmisses = q.value( 0 ).toDouble();
	}
	q.exec( "SELECT SUM( usage ) FROM v$rowcache" );
	if( q.isActive() ) {
	    q.next();
	    usage = q.value( 0 ).toDouble();
	}
	q.exec( "SELECT SUM( fixed ) FROM v$rowcache" );
	if( q.isActive() ) {
	    q.next();
	    fixed = q.value( 0 ).toDouble();
	}
	DictionaryCache->setProgress( 1000 * ( gets - getmisses - usage - fixed ) / gets );
    }

    // Find the shared free memory
    {
	QSqlQuery q( "SELECT bytes from v$sgastat WHERE name = 'free memory' and pool = 'shared pool'" );
	if( q.isActive() ) {
	    q.next();
	    SharedMemory->setText( QString( "%1 bytes" ).arg( q.value( 0 ).toInt() ) );
	}
    }
}

