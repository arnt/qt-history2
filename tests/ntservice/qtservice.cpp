#include "qtservice.h"
#include <qapplication.h>
#include <qt_windows.h>

static QtService *instance = 0;
static QtServicePrivate *data = 0;

class QtServicePrivate
{
public:
    QtServicePrivate()
    {
	data = this;
    }

    void setStatus ( DWORD dwState );
    static void WINAPI serviceMain( DWORD dwArgc, TCHAR** lpszArgv );
    static void WINAPI handler( DWORD dwOpcode );

    QString servicename;
    QString filepath;

    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE serviceStatus;
};

void WINAPI QtServicePrivate::serviceMain( DWORD dwArgc, TCHAR** lpszArgv )
{
    if ( !instance || !data )
	return;

    // Register the control request handler
    data->serviceStatus = RegisterServiceCtrlHandler( (const TCHAR*)qt_winTchar( instance->serviceName(), TRUE ), handler );
    if ( data->serviceStatus == NULL )
	return;
    
    // Start the initialisation
    data->setStatus(SERVICE_START_PENDING);
    if ( instance->initialize() ) {
	data->setStatus(SERVICE_RUNNING);
	// Do the real work. 
	data->status.dwWin32ExitCode = NO_ERROR;
	data->status.dwCheckPoint = 0;
	data->status.dwWaitHint = 0;

	int argc = dwArgc;
	char **argv = new char*[ argc ];
	for ( int i = 0; i < argc; i++ ) {
	    QString a = qt_winQString( lpszArgv[i] );
	    argv[i] = new char[ a.length() + 1 ];
	    strcpy( argv[i], a.latin1() );
	}

	data->status.dwWin32ExitCode = instance->run( argc, argv );
	// When the Run function returns, the service has stopped.
    }
    
    // Tell the service manager we are stopped
    data->setStatus(SERVICE_STOPPED);
}

void WINAPI QtServicePrivate::handler ( DWORD code )
{
    if ( !instance || !data )
	return;

    switch (code) {
    case SERVICE_CONTROL_STOP: // 1
	data->setStatus( SERVICE_STOP_PENDING );
	instance->quit();
	break;

    case SERVICE_CONTROL_PAUSE: // 2
	data->setStatus( SERVICE_PAUSE_PENDING );
	instance->pause();
	data->setStatus( SERVICE_PAUSED );
	break;

    case SERVICE_CONTROL_CONTINUE: // 3
	data->setStatus( SERVICE_CONTINUE_PENDING );
	instance->resume();
	data->setStatus( SERVICE_RUNNING );
	break;

    case SERVICE_CONTROL_INTERROGATE: // 4
    case SERVICE_CONTROL_SHUTDOWN: // 5
	break;

    default:
	if ( code >= 128 && code <= 255 )
	    instance->user( code - 128 );
	break;
    }
    
    // Report current status
    ::SetServiceStatus( data->serviceStatus, &data->status );
}

void QtServicePrivate::setStatus( DWORD state )
{
    status.dwCurrentState = state;
    ::SetServiceStatus( serviceStatus, &status);
}

QtService::QtService( const QString &name, bool canPause )
{
    d = new QtServicePrivate;
    d->servicename = name;

#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based ) {
	TCHAR path[_MAX_PATH];
	::GetModuleFileName( NULL, path, sizeof(path) );
	d->filepath = qt_winQString( path );
    } else
#endif
    {
	char path[_MAX_PATH];
	::GetModuleFileNameA( NULL, path, sizeof(path) );
	d->filepath = QString::fromLocal8Bit( path );
    }

    if ( filePath().contains( ' ' ) )
	filePath() = QString( "\"%1\"" ).arg( filePath() );

    if ( instance ) {
	// ###
	delete instance;
    }

    instance = this;

    d->serviceStatus			    = NULL;
    d->status.dwServiceType		    = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
    d->status.dwCurrentState		    = SERVICE_STOPPED;
    d->status.dwControlsAccepted	    = SERVICE_ACCEPT_STOP;
    if ( canPause )
	d->status.dwControlsAccepted	    |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    d->status.dwWin32ExitCode		    = NO_ERROR;
    d->status.dwServiceSpecificExitCode	    = 0;
    d->status.dwCheckPoint		    = 0;
    d->status.dwWaitHint		    = 0;
}

QtService::~QtService()
{
    instance = 0;
    delete d;
}

bool QtService::install()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (!hSCM) 
	return FALSE;

    // Create the service
    SC_HANDLE hService = 0;
    TCHAR *name = (TCHAR*)qt_winTchar_new( serviceName() );
    hService = ::CreateService( hSCM, name, name, 
				SERVICE_ALL_ACCESS, d->status.dwServiceType, 
				SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				(TCHAR*)qt_winTchar( filePath(), TRUE ),
				NULL, NULL, NULL, NULL, NULL);

    if (!hService) {
	::CloseServiceHandle(hSCM);
	return FALSE;
    }

    // tidy up
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    reportEvent( "Service installed" );
    return TRUE;
}

bool QtService::uninstall()
{
    bool result = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (!hSCM) 
	return result;

    // Try to open the service
    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( serviceName(), TRUE ), DELETE );

    if ( hService ) {
	if ( ::DeleteService(hService) )
	    result = TRUE;
	::CloseServiceHandle(hService);
    }

    ::CloseServiceHandle (hSCM);
    if ( result )
	reportEvent( "Service uninstalled" );
    return result;
}

bool QtService::isInstalled() const
{
    bool result = FALSE;
    
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( !hSCM )
	return result;

    // Try to open the service
    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( serviceName(), TRUE ), SERVICE_QUERY_CONFIG );

    if (hService) {
	result = TRUE;
	::CloseServiceHandle(hService);
    }
    ::CloseServiceHandle(hSCM);
    
    return result;
}

bool QtService::isRunning() const
{
    bool result = FALSE;
    
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( !hSCM )
	return result;

    // Try to open the service
    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( serviceName(), TRUE ), SERVICE_QUERY_STATUS );

    if (hService) {
	SERVICE_STATUS info;
	int res = QueryServiceStatus( hService, &info );
	if ( res )
	    result = info.dwCurrentState == SERVICE_RUNNING;

	::CloseServiceHandle(hService);
    }
    ::CloseServiceHandle(hSCM);

    return result;
}

bool QtService::start()
{
    if ( !isInstalled() && !install() )
	return FALSE;

    bool res = FALSE;
    SERVICE_TABLE_ENTRY st [2];
    TCHAR *sname = (TCHAR*)qt_winTchar_new( serviceName() );
    st[0].lpServiceName = sname;
    st[0].lpServiceProc = QtServicePrivate::serviceMain;
    st[1].lpServiceName = NULL;
    st[1].lpServiceProc = NULL;

    res = ::StartServiceCtrlDispatcher(st);
    if ( !res )
	reportEvent( "The Service failed to start", Error );
    return res;
}

void QtService::exec( int argc, char **argv )
{
    if ( isInstalled() ) {
	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( !hSCM )
	    return;

	SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( serviceName(), TRUE ), SERVICE_START );
	if ( hService ) {
	    DWORD dwArgc = argc;
	    TCHAR **lpArgs = new TCHAR*[ dwArgc ];
	    for ( int i = 0; i < argc; i++ )
		lpArgs[i] = (TCHAR*)qt_winTchar_new( argv[i] );
	    StartService( hService, dwArgc, (const TCHAR**)lpArgs );
	}
    } else {
	run( argc, argv );
    }
}

void QtService::stop()
{
    if ( !isRunning() )
	return;

    SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( !hSCM )
	return;

    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( serviceName(), TRUE ), SERVICE_STOP );

    if ( hService ) {
	SERVICE_STATUS status;
	ControlService( hService, SERVICE_CONTROL_STOP, &status );
    }
}

bool QtService::initialize()
{
    return TRUE;
}

void QtService::pause()
{
}

void QtService::resume()
{
}

void QtService::user( int code )
{
}

QString QtService::serviceName() const
{
    return d->servicename;
}

QString QtService::filePath() const
{
    return d->filepath;
}

void QtService::reportEvent( const QString &message, EventType type, uint category )
{
    HANDLE h;
    WORD wType;
    WORD dwEventID = 5;

    switch ( type ) {
    case Error:
	wType = EVENTLOG_ERROR_TYPE;
	break;

    case Warning:
	wType = EVENTLOG_WARNING_TYPE;
	break;

    case Information:
	wType = EVENTLOG_INFORMATION_TYPE;

    default:
	wType = EVENTLOG_SUCCESS;
	break;
    }

    h = RegisterEventSource( NULL, (TCHAR*)qt_winTchar( serviceName(), TRUE ) );
    if ( !h )
	return;
    const TCHAR* msg = (TCHAR*)qt_winTchar( message, TRUE );
    ReportEvent( h, wType, category, dwEventID, NULL, 1, 0, (const TCHAR**)&msg, NULL );

    DeregisterEventSource( h );
}
