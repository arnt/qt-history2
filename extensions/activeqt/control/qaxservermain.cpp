/****************************************************************************
**
** Implementation of win32 COM server startup routines.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qstringlist.h>
#include <qvector.h>

#include "qaxfactory.h"

#include <qt_windows.h>

static DWORD *classRegistration = 0;
static DWORD dwThreadID;
static bool qAxActivity = FALSE;
static HANDLE hEventShutdown;

#ifdef QT_DEBUG
QT_STATIC_CONST DWORD dwTimeOut = 1000;
QT_STATIC_CONST DWORD dwPause = 500;
#else
QT_STATIC_CONST DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
QT_STATIC_CONST DWORD dwPause = 1000; // time to wait for threads to finish up
#endif

extern HANDLE hEventShutdown;
extern bool qAxActivity;
extern HANDLE qAxInstance;
extern bool qAxIsServer;
extern char qAxModuleFilename[MAX_PATH];
extern void qAxInit();
extern void qAxCleanup();
extern HRESULT UpdateRegistry(BOOL bRegister);
typedef int (*QWinEventFilter) (MSG*);
extern int QAxEventFilter( MSG *pMsg );
extern Q_GUI_EXPORT QWinEventFilter qt_set_win_event_filter (QWinEventFilter filter);
extern HRESULT GetClassObject( const GUID &clsid, const GUID &iid, void **ppUnk );
extern ulong qAxLockCount();

#if defined(Q_CC_BOR)
extern "C" __stdcall HRESULT DumpIDL( const QString &outfile, const QString &ver );
#else
STDAPI DumpIDL( const QString &outfile, const QString &ver );
#endif

// Monitors the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    while (1) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            qAxActivity = FALSE;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while ( dwWait == WAIT_OBJECT_0 );
        // timed out
        if ( !qAxActivity && !qAxLockCount() ) // if no activity let's really bail
            break;
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    PostQuitMessage( 0 );

    return 0;
}

// Starts the monitoring thread
static bool StartMonitor()
{
    dwThreadID = GetCurrentThreadId();
    hEventShutdown = CreateEventA( 0, FALSE, FALSE, 0 );
    if ( hEventShutdown == 0 )
        return FALSE;
    DWORD dwThreadID;
    HANDLE h = CreateThread( 0, 0, MonitorProc, 0, 0, &dwThreadID );
    return (h != NULL);
}

void qax_shutDown()
{
    qAxActivity = TRUE;
    if ( hEventShutdown )
	SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
}

/*
    Start the COM server (if necessary).
*/
bool qax_startServer(QAxFactory::ServerType type)
{
    if (qAxIsServer)
	return TRUE;

    const QStringList keys = qAxFactory()->featureList();
    if ( !keys.count() )
	return FALSE;

    if ( !qAxFactory()->isService() )
	StartMonitor();

    classRegistration = new DWORD[keys.count()];
    int object = 0;
    for ( QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key, ++object ) {
	IUnknown* p = 0;
	CLSID clsid = qAxFactory()->classID( *key );

	// Create a QClassFactory (implemented in qaxserverbase.cpp)
	HRESULT hRes = GetClassObject( clsid, IID_IClassFactory, (void**)&p );
	if ( SUCCEEDED(hRes) )
	    hRes = CoRegisterClassObject( clsid, p, CLSCTX_LOCAL_SERVER,
					  type == QAxFactory::MultipleInstances ? REGCLS_MULTIPLEUSE : REGCLS_SINGLEUSE,
					  classRegistration+object );
	if ( p )
	    p->Release();
    }

    // qt_set_win_event_filter( QAxEventFilter ); XXXX
    qAxIsServer = TRUE;
    return TRUE;
}

/*
    Stop the COM server (if necessary).
*/
bool qax_stopServer()
{
    if (!qAxIsServer || !classRegistration)
	return TRUE;

    qAxIsServer = FALSE;
    // qt_set_win_event_filter(0);

    const QStringList keys = qAxFactory()->featureList();
    int object = 0;
    for ( QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key, ++object )
	CoRevokeClassObject( classRegistration[object] );

    delete []classRegistration;
    classRegistration = 0;

    Sleep(dwPause); //wait for any threads to finish

    return TRUE;
}

#if defined(NEEDS_QMAIN)
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
int qMain( int, char ** );
#else
#if defined( Q_OS_TEMP )
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<pchar> &);
EXTERN_C int __cdecl main( int, char ** );
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<pchar> &);
EXTERN_C int main( int, char ** );
#endif
#endif

EXTERN_C int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GetModuleFileNameA( 0, qAxModuleFilename, MAX_PATH-1 );
    qAxInstance = hInstance;

    lpCmdLine = GetCommandLineA(); //this line necessary for _ATL_MIN_CRT
    QString cmdLine = QString::fromLatin1( lpCmdLine );

    QStringList cmds = cmdLine.split(" ");
    int nRet = 0;
    bool run = TRUE;
    bool runServer = FALSE;
    for ( QStringList::Iterator it = cmds.begin(); it != cmds.end(); ++it ) {
	QString cmd = (*it).toLower();
	if ( cmd == "-activex" || cmd == "/activex" ) {
	    runServer = TRUE;
	} else if ( cmd == "-unregserver" || cmd == "/unregserver" ) {
	    qWarning( "Unregistering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(FALSE);
            run = FALSE;
	    break;
	} else if ( cmd == "-regserver" || cmd == "/regserver" ) {
	    qWarning( "Registering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(TRUE);
            run = FALSE;
            break;
	} else if ( cmd == "-dumpidl" || cmd == "/dumpidl" ) {
	    ++it;
	    if ( it != cmds.end() ) {
		QString outfile = *it;
		++it;
		QString version;
		if ( it != cmds.end() && ( *it == "-version" || *it == "/version" ) ) {
		    ++it;
		    if ( it != cmds.end() )
			version = *it;
		    else
			version = "1.0";
		}

		nRet = DumpIDL( outfile, version );
		switch (nRet) {
		case S_OK:
		    break;
		case -1:
		    qWarning( "Couldn't open %s for writing", (const char*)outfile.local8Bit() );
		    return nRet;
		case 1:
		    qWarning( "Malformed appID value in %s!", qAxModuleFilename );
		    return nRet;
		case 2:
		    qWarning( "Malformed typeLibID value in %s!", qAxModuleFilename );
		    return nRet;
		case 3:
		    qWarning( "Class has no metaobject information (error in %s)!", qAxModuleFilename );
		    return nRet;
		case 4:
		    qWarning( "Malformed classID value in %s!", qAxModuleFilename );
		    return nRet;
		case 5:
		    qWarning( "Malformed interfaceID value in %s!", qAxModuleFilename );
		    return nRet;
		case 6:
		    qWarning( "Malformed eventsID value in %s!", qAxModuleFilename );
		    return nRet;

		default:
		    qFatal( "Unknown error writing IDL from %s", qAxModuleFilename );
		}
	    } else {
		qWarning( "Wrong commandline syntax: <app> -dumpidl <idl file> [-version <x.y.z>]" );
	    }
	    run = FALSE;
	    break;
	}
    }

    if (run) {
	int argc;
	char* cmdp = 0;
	// Use malloc/free for eval package compability
	cmdp = (char*) malloc( (cmdLine.length() + 1) * sizeof(char) );
	qstrcpy( cmdp, cmdLine.latin1() );

	HRESULT hRes = CoInitialize(0);

	QVector<pchar> argv( 8 );
	qWinMain( hInstance, hPrevInstance, cmdp, nShowCmd, argc, argv );
	qAxInit();
	if (runServer)
	    QAxFactory::startServer();
	nRet = main( argc, argv.data() );
	QAxFactory::stopServer();
	qAxCleanup();
	CoUninitialize();

	free( cmdp );
    }

    return nRet;
}


// until such time as mingw runtime calls winmain instead of main
// in a GUI app we need this.
#if defined(Q_OS_WIN32) && defined(Q_CC_GNU)
#include <qtcrtentrypoint.cpp>
#endif

