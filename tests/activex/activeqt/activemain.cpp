#include <qapplication.h>
#include <qt_windows.h>
#include <qregexp.h>

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "activeqt.h"

const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up

// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0) {
        bActivity = true;
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return l;
}

//Monitors the shutdown event
void CExeModule::MonitorShutdown()
{
    while (1) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!bActivity && m_nLockCnt == 0) { // if no activity let's really bail
            break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_QActiveX, QActiveX )
END_OBJECT_MAP()


/////////////////////////////////////////////////////////////////////////////
//
#if defined( Q_OS_TEMP )
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
#endif

#ifdef Q_OS_TEMP
EXTERN_C int __cdecl main( int, char ** );
#else
EXTERN_C int main( int, char ** );
#endif
CExeModule _Module;

extern "C" int WINAPI WinMain(HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    lpCmdLine = GetCommandLineA(); //this line necessary for _ATL_MIN_CRT
    QString cmdLine = QString::fromLatin1( lpCmdLine );

    HRESULT hRes = CoInitialize(NULL);

    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init (ObjectMap, hInstance, &LIBID_ACTIVEQTEXELib );
    _Module.dwThreadID = GetCurrentThreadId();

    cmdLine = cmdLine.replace( QRegExp("/"), "-" );
    QStringList cmds = QStringList::split( " ", cmdLine );
    int nRet = 0;
    bool bRun = TRUE;
    bool bRunMain = FALSE;
    for ( QStringList::Iterator it = cmds.begin(); it != cmds.end(); ++it ) {
	QString cmd = (*it).lower();
	if ( cmd == "-unregserver" || cmd == "-uninstall" ) {
	    const QString appID = QUuid( IID_ActiveQtApp ).toString().upper();
	    nRet = QActiveX::UpdateRegistry( FALSE );
            bRun = FALSE;
            break;
	} else if ( cmd == "-regserver" || cmd == "-install" ) {
	    const QString appID = QUuid( IID_ActiveQtApp ).toString().upper();
	    nRet = QActiveX::UpdateRegistry( TRUE );
            bRun = FALSE;
            break;
	} else if ( cmd == "-run" ) {
	    bRun = TRUE;
	    bRunMain = TRUE;
	}
    }

    if (bRun) {
	int argc;
	char* cmdp = 0;
	cmdp = new char[ cmdLine.length() + 1 ];
	qstrcpy( cmdp, cmdLine.latin1() );

	QMemArray<pchar> argv( 8 );
	qWinMain( hInstance, hPrevInstance, cmdp, nShowCmd, argc, argv );
	_Module.StartMonitor();
	hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
	_ASSERTE(SUCCEEDED(hRes));
	QApplication app( argc, argv.data() );
	QWidget *widget = 0;
	if ( bRunMain ) {
#if 0
	    QUnknownInterface *unknown = ucm_instantiate();
	    QInterfacePtr<QWidgetFactoryInterface> iface = 0;
	    if ( unknown->queryInterface( IID_QWidgetFactory, (QUnknownInterface**)&iface ) == QS_OK ) {
		QString key = iface->featureList()[0];
		widget = iface->create( key );
		QObject::connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
		widget->show();
		iface->release();
	    }
	    unknown->release();
#else
	    widget = axmain(0);
#endif
	    Q_ASSERT(widget);
	    QObject::connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
	    widget->show();
	}
	nRet = app.exec();
	delete widget;
	widget = 0;

	_Module.RevokeClassObjects();
	Sleep(dwPause); //wait for any threads to finish
    }

    _Module.Term();
    CoUninitialize();
    return nRet;
}
