#include "qtservice.h"
#include <qapplication.h>

static QtService *instance = 0;

void WINAPI QtService::serviceMainA( DWORD dwArgc, char** lpszArgv )
{
    if ( !instance )
	return;

    // Register the control request handler
    instance->serviceStatus = RegisterServiceCtrlHandlerA( instance->servicename.local8Bit(), handler );

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
	char **argv = lpszArgv;
	instance->status.dwWin32ExitCode = instance->run( argc, argv );
	// When the Run function returns, the service has stopped.
    }
    
    // Tell the service manager we are stopped
    instance->setStatus(SERVICE_STOPPED);
}

void WINAPI QtService::serviceMainW( DWORD dwArgc, TCHAR** lpszArgv )
{
    if ( !instance )
	return;

    // Register the control request handler
    instance->serviceStatus = RegisterServiceCtrlHandlerW( (const TCHAR*)qt_winTchar( instance->servicename, TRUE ), handler );
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
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	TCHAR path[_MAX_PATH];
	::GetModuleFileName( NULL, path, sizeof(path) );
	filepath = qt_winQString( path );
    } else
#endif
    {
	char path[_MAX_PATH];
	::GetModuleFileNameA( NULL, path, sizeof(path) );
	filepath = path;
    }
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
    SC_HANDLE hSCM = 0;
    SC_HANDLE hService = 0;
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based )
	hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    else
#endif
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );

    if (!hSCM) 
	return FALSE;
    
    // Create the service
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	TCHAR *name = (TCHAR*)qt_winTchar_new( servicename );
	hService = ::CreateService( hSCM, name, name, 
				    SERVICE_ALL_ACCESS, status.dwServiceType, 
				    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				    (TCHAR*)qt_winTchar( filepath, TRUE ),
				    NULL, NULL, NULL, NULL, NULL);
    } else
#endif
	 hService = ::CreateServiceA( hSCM, servicename.local8Bit(), servicename.local8Bit(),
				      SERVICE_ALL_ACCESS, status.dwServiceType, 
				      SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				      filepath.local8Bit(),
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
    // Open the Service Control Manager
    SC_HANDLE hSCM = 0;
    SC_HANDLE hService = 0;
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based )
	hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    else
#endif
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );

    if (!hSCM) 
	return FALSE;
    
    bool result = FALSE;
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based )
	hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), DELETE );
    else
#endif
	hService = ::OpenServiceA( hSCM, servicename.local8Bit(), DELETE );

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
    SC_HANDLE hSCM = 0;
    SC_HANDLE hService = 0;
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based )
	hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    else
#endif
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );

    if (hSCM) {
	// Try to open the service
#if defined(UNICODE)
	if ( QApplication::winVersion() & Qt::WV_NT_based )
	    hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), SERVICE_QUERY_CONFIG );
	else
#endif
	    hService = ::OpenServiceA( hSCM, servicename.local8Bit(), SERVICE_QUERY_CONFIG );

	if (hService) {
	    result = TRUE;
	    ::CloseServiceHandle(hService);
	}
	::CloseServiceHandle(hSCM);
    }
    
    return result;
}

bool QtService::isRunning() const
{
    bool result = FALSE;
    
    // Open the Service Control Manager
    SC_HANDLE hSCM = 0;
    SC_HANDLE hService = 0;
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based )
	hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    else
#endif
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );

    if (hSCM) {
	// Try to open the service
#if defined(UNICODE)
	if ( QApplication::winVersion() & Qt::WV_NT_based )
	    hService = ::OpenService( hSCM, (const TCHAR*)qt_winTchar( servicename, TRUE ), SERVICE_QUERY_STATUS );
	else
#endif
	    hService = ::OpenServiceA( hSCM, servicename.local8Bit(), SERVICE_QUERY_STATUS );

	if (hService) {
	    SERVICE_STATUS info;
	    int res = QueryServiceStatus( hService, &info );
	    if ( res )
		result = info.dwCurrentState == SERVICE_RUNNING;

	    ::CloseServiceHandle(hService);
	}
	::CloseServiceHandle(hSCM);
    }

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
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	SERVICE_TABLE_ENTRY st [2];
	TCHAR *sname = (TCHAR*)qt_winTchar_new( servicename );
	st[0].lpServiceName = sname;
	st[0].lpServiceProc = QtService::serviceMainW;
	st[1].lpServiceName = NULL;
	st[1].lpServiceProc = NULL;

	res = ::StartServiceCtrlDispatcher(st);
    } else
#endif
    {
	SERVICE_TABLE_ENTRYA st [2];
	char *sname = new char[ servicename.length() + 1 ];
	strcpy( sname, servicename );
	st[0].lpServiceName = sname;
	st[0].lpServiceProc = QtService::serviceMainA;
	st[1].lpServiceName = NULL;
	st[1].lpServiceProc = NULL;

	res = ::StartServiceCtrlDispatcherA(st);
    }
    if ( !res )
	reportEvent( "The Service failed to start", Error );
    return res;
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

#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	h = RegisterEventSource( NULL, (TCHAR*)qt_winTchar( servicename, TRUE ) );
	if ( !h )
	    return;
	const TCHAR* msg = (TCHAR*)qt_winTchar( message, TRUE );
	ReportEvent( h, wType, category, dwEventID, NULL, 1, 0, (const TCHAR**)&msg, NULL );
    } else
#endif
    {
	h = RegisterEventSourceA( NULL, servicename.local8Bit() );
	if ( !h )
	    return;
	const char* msg = message.local8Bit();
	ReportEventA( h, wType, category, dwEventID, NULL, 1, 0, (const char**)&msg, NULL );
    }

    DeregisterEventSource( h );
}
