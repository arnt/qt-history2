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
    static void WINAPI serviceMainW( DWORD dwArgc, TCHAR** lpszArgv );
    static void WINAPI serviceMainA( DWORD dwArgc, char** lpszArgv );
    static void WINAPI handler( DWORD dwOpcode );

    QString servicename;
    QString filepath;

    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE serviceStatus;
};

void WINAPI QtServicePrivate::serviceMainW( DWORD dwArgc, TCHAR** lpszArgv )
{
    if ( !instance || !data )
	return;

    // Register the control request handler
    data->serviceStatus = RegisterServiceCtrlHandlerW( instance->serviceName().ucs2(), handler );
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
	    QString a = QString::fromUcs2( lpszArgv[i] );
	    argv[i] = new char[ a.length() + 1 ];
	    strcpy( argv[i], a.local8Bit().data() );
	}

	data->status.dwWin32ExitCode = instance->run( argc, argv );
	// When the Run function returns, the service has stopped.
	delete []*argv;
    }
    
    // Tell the service manager we are stopped
    data->setStatus(SERVICE_STOPPED);
}

void WINAPI QtServicePrivate::serviceMainA( DWORD dwArgc, char **lpszArgv )
{
    if ( !instance || !data )
	return;

    // Register the control request handler
    data->serviceStatus = RegisterServiceCtrlHandlerA( instance->serviceName().local8Bit(), handler );
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

	data->status.dwWin32ExitCode = instance->run( dwArgc, lpszArgv );
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

QtService::QtService( const QString &name, bool canPause, bool useGui )
{
    d = new QtServicePrivate;
    d->servicename = name;

    QT_WA( {
	TCHAR path[_MAX_PATH];
	::GetModuleFileNameW( NULL, path, sizeof(path) );
	d->filepath = QString::fromUcs2( path );
    }, {
	char path[_MAX_PATH];
	::GetModuleFileNameA( NULL, path, sizeof(path) );
	d->filepath = QString::fromLocal8Bit( path );
    } );

    if ( filePath().contains( ' ' ) )
	filePath() = QString( "\"%1\"" ).arg( filePath() );

    if ( instance ) {
	// ###
	delete instance;
    }

    instance = this;

    d->serviceStatus			    = 0;
    d->status.dwServiceType		    = SERVICE_WIN32_OWN_PROCESS;
    if ( useGui )
	d->status.dwServiceType		    |= SERVICE_INTERACTIVE_PROCESS;
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
    SC_HANDLE hSCM = 0;
    QT_WA( {
	hSCM = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    }, {
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    } );
    if (!hSCM)
	return FALSE;

    // Create the service
    SC_HANDLE hService = 0;
    QT_WA( {
	hService = ::CreateServiceW( hSCM, serviceName().ucs2(), serviceName().ucs2(),
				    SERVICE_ALL_ACCESS, d->status.dwServiceType, 
				    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				    filePath().ucs2(), NULL, NULL, NULL, NULL, NULL);
    }, {
	hService = ::CreateServiceA( hSCM, serviceName().local8Bit(), serviceName().local8Bit(),
				    SERVICE_ALL_ACCESS, d->status.dwServiceType, 
				    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				    filePath().local8Bit(), NULL, NULL, NULL, NULL, NULL);
    } );

    if (!hService) {
	::CloseServiceHandle(hSCM);
	reportEvent( "Installing the service failed", Error );
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
    SC_HANDLE hSCM = 0;
    QT_WA( {
	hSCM = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    }, {
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    } );
    if (!hSCM)
	return result;

    // Try to open the service
    SC_HANDLE hService = 0;
    QT_WA( {
	hService = ::OpenServiceW( hSCM, serviceName().ucs2(), DELETE );
    }, {
	hService = ::OpenServiceA( hSCM, serviceName().local8Bit(), DELETE );
    } );

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
    QT_WA( {
	hSCM = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    }, {
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    } );
    if (!hSCM)
	return result;

    // Try to open the service
    SC_HANDLE hService = 0;
    QT_WA( {
	hService = ::OpenServiceW( hSCM, serviceName().ucs2(), SERVICE_QUERY_CONFIG );
    }, {
	hService = ::OpenServiceA( hSCM, serviceName().local8Bit(), SERVICE_QUERY_CONFIG );
    } );

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
    SC_HANDLE hSCM = 0;
    QT_WA( {
	hSCM = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    }, {
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    } );
    if (!hSCM)
	return result;

    // Try to open the service
    SC_HANDLE hService = 0;
    QT_WA( {
	hService = ::OpenServiceW( hSCM, serviceName().ucs2(), SERVICE_QUERY_STATUS );
    }, {
	hService = ::OpenServiceA( hSCM, serviceName().local8Bit(), SERVICE_QUERY_STATUS );
    } );

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

bool QtService::isInteractive() const
{
    return d->status.dwServiceType & SERVICE_INTERACTIVE_PROCESS;
}

bool QtService::canPause() const
{
    return d->status.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE;
}

bool QtService::start()
{
    if ( !isInstalled() && !install() )
	return FALSE;

    bool res = FALSE;
    const length = serviceName().length();

    QT_WA( {
	SERVICE_TABLE_ENTRYW st [2];
	st[0].lpServiceName = (TCHAR*)serviceName().ucs2();
	st[0].lpServiceProc = QtServicePrivate::serviceMainW;
	st[1].lpServiceName = NULL;
	st[1].lpServiceProc = NULL;

	res = ::StartServiceCtrlDispatcherW(st);
    }, {
	SERVICE_TABLE_ENTRYA st [2];
	QCString sname = serviceName().local8Bit();
	st[0].lpServiceName = sname.data();
	st[0].lpServiceProc = QtServicePrivate::serviceMainA;
	st[1].lpServiceName = NULL;
	st[1].lpServiceProc = NULL;

	res = ::StartServiceCtrlDispatcherA(st);
    } );
    if ( !res )
	reportEvent( "The Service failed to start", Error );
    return res;
}

void QtService::exec( int argc, char **argv )
{
    if ( isInstalled() ) {
	SC_HANDLE hSCM = 0;
	SC_HANDLE hService = 0;
	QT_WA( {
	    hSCM = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	}, {
	    hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	} );
	if ( !hSCM )
	    return;

	QT_WA( {
	    hService = ::OpenServiceW( hSCM, serviceName().ucs2(), SERVICE_START );
	}, {
	    hService = ::OpenServiceA( hSCM, serviceName().local8Bit(), SERVICE_START );
	} );

	// Not much point blowing up argv to wide characters - they end up being ANSI anyway.
	if ( hService )
	    StartServiceA( hService, argc, (const char**)argv );
    } else {
	run( argc, argv );
    }
}

void QtService::stop()
{
    if ( !isRunning() )
	return;

    SC_HANDLE hSCM = 0;
    SC_HANDLE hService = 0;
    QT_WA( {
	hSCM = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    }, {
	hSCM = ::OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    } );
    if ( !hSCM )
	return;

    QT_WA( {
	hService = ::OpenServiceW( hSCM, serviceName().ucs2(), SERVICE_STOP );
    }, {
	hService = ::OpenServiceA( hSCM, serviceName().local8Bit(), SERVICE_STOP );
    } );

    if ( !hService )
	return;

    SERVICE_STATUS status;
    ControlService( hService, SERVICE_CONTROL_STOP, &status );

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
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

void QtService::reportEvent( const QString &message, EventType type, uint category, const QByteArray &data )
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

    QT_WA( {
	h = RegisterEventSourceW( NULL, serviceName().ucs2() );
	if ( !h )
	    return;
	const TCHAR *msg = message.ucs2();
	char *bindata = data.size() ? data.data() : 0;
	ReportEventW( h, wType, category, dwEventID, NULL, 1, data.size(), (const TCHAR**)&msg, bindata );
    }, {
	h = RegisterEventSourceA( NULL, serviceName().local8Bit() );
	if ( !h )
	    return;
	QCString cmsg = message.local8Bit();
	const char *msg = cmsg.data();
	char *bindata = data.size() ? data.data() : 0;
	ReportEventA( h, wType, category, dwEventID, NULL, 1, data.size(), (const char**)&msg, bindata );
    } );

    DeregisterEventSource( h );
}
