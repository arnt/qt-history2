#include "qtservice.h"
#include <qapplication.h>

static QtService *instance = 0;

void WINAPI QtService::serviceMain( DWORD dwArgc, TCHAR** lpszArgv )
{
    if ( !instance )
	return;

    // Register the control request handler
    instance->serviceStatus = RegisterServiceCtrlHandler( (const TCHAR*)qt_winTchar( instance->servicename, TRUE ), handler );
    if ( instance->serviceStatus == NULL )
	return;
    
    // Start the initialisation
    instance->setStatus(SERVICE_START_PENDING);
    if ( instance->initialize() ) {
	instance->setStatus(SERVICE_RUNNING);
	// Do the real work. 
	instance->status.dwWin32ExitCode = NO_ERROR;
	instance->status.dwCheckPoint = 0;
	instance->status.dwWaitHint = 0;

	int argc = dwArgc;
	char **argv = new char*[ argc ];
	for ( int i = 0; i < argc; i++ ) {
	    QString a = qt_winQString( lpszArgv[i] );
	    argv[i] = new char[ a.length() + 1 ];
	    strcpy( argv[i], a.latin1() );
	}

	instance->status.dwWin32ExitCode = instance->run( argc, argv );
	// When the Run function returns, the service has stopped.
    }
    
    // Tell the service manager we are stopped
    instance->setStatus(SERVICE_STOPPED);
}

void WINAPI QtService::handler ( DWORD code )
{
    if ( !instance )
	return;

    switch (code) {
    case SERVICE_CONTROL_STOP: // 1
	instance->setStatus( SERVICE_STOP_PENDING );
	instance->quit();
	break;

    case SERVICE_CONTROL_PAUSE: // 2
	instance->setStatus( SERVICE_PAUSE_PENDING );
	instance->pause();
	instance->setStatus( SERVICE_PAUSED );
	break;

    case SERVICE_CONTROL_CONTINUE: // 3
	instance->setStatus( SERVICE_CONTINUE_PENDING );
	instance->resume();
	instance->setStatus( SERVICE_RUNNING );
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
    ::SetServiceStatus( instance->serviceStatus, &instance->status );
}

QtService::QtService( const QString &name, bool canPause )
: servicename( name )
{
    TCHAR path[_MAX_PATH];
    ::GetModuleFileName( NULL, path, sizeof(path) );
    filepath = qt_winQString( path );

    if ( filepath.contains( ' ' ) )
	filepath = QString( "\"%1\"" ).arg( filepath );

    if ( instance ) {
	// ###
	delete instance;
    }

    instance = this;

    serviceStatus		    = NULL;
    status.dwServiceType	    = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
    status.dwCurrentState	    = SERVICE_STOPPED;
    status.dwControlsAccepted	    = SERVICE_ACCEPT_STOP;
    if ( canPause )
	status.dwControlsAccepted  |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    status.dwWin32ExitCode	    = NO_ERROR;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint		    = 0;
    status.dwWaitHint		    = 0;
}

QtService::~QtService()
{
    instance = 0;
}

bool QtService::install()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (!hSCM) 
	return FALSE;

    // Create the service
    SC_HANDLE hService = 0;
    TCHAR *name = (TCHAR*)qt_winTchar_new( servicename );
    hService = ::CreateService( hSCM, name, name, 
				SERVICE_ALL_ACCESS, status.dwServiceType, 
				SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				(TCHAR*)qt_winTchar( filepath, TRUE ),
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
    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), DELETE );

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
    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), SERVICE_QUERY_CONFIG );

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
    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), SERVICE_QUERY_STATUS );

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

void QtService::setStatus( DWORD state )
{
    status.dwCurrentState = state;
    ::SetServiceStatus( serviceStatus, &status);
}

bool QtService::start()
{
    if ( !isInstalled() && !install() )
	return FALSE;

    bool res = FALSE;
    SERVICE_TABLE_ENTRY st [2];
    TCHAR *sname = (TCHAR*)qt_winTchar_new( servicename );
    st[0].lpServiceName = sname;
    st[0].lpServiceProc = QtService::serviceMain;
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

	SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), SERVICE_START );
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

    SC_HANDLE hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), SERVICE_STOP );

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
    return servicename;
}

QString QtService::filePath() const
{
    return filepath;
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

    h = RegisterEventSource( NULL, (TCHAR*)qt_winTchar( servicename, TRUE ) );
    if ( !h )
	return;
    const TCHAR* msg = (TCHAR*)qt_winTchar( message, TRUE );
    ReportEvent( h, wType, category, dwEventID, NULL, 1, 0, (const TCHAR**)&msg, NULL );

    DeregisterEventSource( h );
}
