#include <qmetaobject.h>
#include <qsettings.h>
#include <qmap.h>

#include <qt_windows.h>

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "qactiveqtbase.h"

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
    if ( !l ) {
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
    PostQuitMessage( 0 );
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

extern "C" QUnknownInterface *ucm_instantiate();

QActiveQtFactoryInterface *CExeModule::factory()
{
    if ( !_factory ) {
	QInterfacePtr<QUnknownInterface> unknown = ucm_instantiate();
	if ( unknown )
	    unknown->queryInterface( IID_QActiveQtFactory, (QUnknownInterface**)&_factory );
    }
    return _factory;
}

static HRESULT WINAPI CreateInstance( void *pUnkOuter, REFIID iid, void **ppUnk )
{
    HRESULT nRes = E_OUTOFMEMORY;
    return nRes;
}

class QClassFactory : public IClassFactory
{
public:
    QClassFactory( CLSID clsid )
	: classID( clsid ), ref( 0 )
    {
    }

    unsigned long WINAPI AddRef()
    {
	return ++ref;
    }
    unsigned long WINAPI Release()
    {
	if ( !--ref ) {
	    delete this;
	    return 0;
	}
	return ref;
    }
    HRESULT WINAPI QueryInterface( REFIID iid, LPVOID *iface )
    {
	*iface = 0;
	if ( iid == IID_IUnknown )
	    *iface = (IUnknown*)this;
	else if ( iid == IID_IClassFactory )
	    *iface = (IClassFactory*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    HRESULT WINAPI CreateInstance( IUnknown *pUnkOuter, REFIID iid, void **ppObject )
    {
	const QString clsid = QUuid(classID).toString();
	QActiveQtBase *activeqt = new QActiveQtBase( classID );
	activeqt->QueryInterface( iid, ppObject );
	return S_OK;
    }
    HRESULT WINAPI LockServer( BOOL fLock )
    {
	if ( fLock )
	    _Module.Lock();
	else
	    _Module.Unlock();

	return S_OK;
    }

private:
    CLSID classID;
    unsigned long ref;
};

static HRESULT WINAPI GetClassObject( void *pv, REFIID iid, void **ppUnk )
{
    HRESULT nRes = E_OUTOFMEMORY;
    QClassFactory *factory = new QClassFactory( iid );
    if ( factory )
	nRes = factory->QueryInterface( IID_IUnknown, ppUnk );
    return nRes;
}


static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
{
    char filename[MAX_PATH];
    GetModuleFileNameA( 0, filename, MAX_PATH-1 );
    QString file = QString::fromLocal8Bit(filename );
    QString path = file.left( file.findRev( "\\" )+1 );
    QString module = file.right( file.length() - path.length() );
    module = module.left( module.findRev( "." ) );

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    const QString appID = _Module.factory()->appID().toString().upper();
    const QString libID = _Module.factory()->typeLibID().toString().upper();

    if ( bRegister ) {
	settings.writeEntry( "/AppID/" + appID + "/.", module );
	settings.writeEntry( "/AppID/" + module + ".EXE/AppID", appID );

	settings.writeEntry( "/TypeLib/" + libID + "/1.0/0/win32/.", file );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/FLAGS/.", "0" );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/HELPDIR/.", path );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/.", module + " 1.0 Type Library" );
    } else {
	settings.removeEntry( "/AppID/" + module + ".EXE/AppID" );
	settings.removeEntry( "/AppID/" + appID + "/." );
	
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/0/win32/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/0/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/FLAGS/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/HELPDIR/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/." );
    }
    
    QStringList keys = _Module.factory()->featureList();
    for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	const QString classID = (*key).upper();
	const QString eventID = _Module.factory()->eventsID(*key).toString().upper();
	const QString ifaceID = _Module.factory()->interfaceID(*key).toString().upper();
	QMetaObject *mo = _Module.factory()->metaObject( *key );

	if ( !mo )
	    continue;
	const QString className = mo->className();

	if ( bRegister ) {	    
	    settings.writeEntry( "/" + module + "." + className + ".1/.", className + " Class" );
	    settings.writeEntry( "/" + module + "." + className + ".1/CLSID/.", classID );
	    settings.writeEntry( "/" + module + "." + className + ".1/Insertable/.", QString::null );
	    
	    settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
	    settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classID );
	    settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + ".1" );
	    
	    settings.writeEntry( "/CLSID/" + classID + "/.", className + " Class" );
	    if ( !appID.isNull() )
		settings.writeEntry( "/CLSID/" + classID + "/AppID", appID );
	    settings.writeEntry( "/CLSID/" + classID + "/Control/.", QString::null );
	    settings.writeEntry( "/CLSID/" + classID + "/Insertable/.", QString::null );
	    settings.writeEntry( "/CLSID/" + classID + "/LocalServer32/.", file + " -activex" );
	    settings.writeEntry( "/CLSID/" + classID + "/MiscStatus/.", "0" );
	    settings.writeEntry( "/CLSID/" + classID + "/MiscStatus/1/.", "131473" );
	    settings.writeEntry( "/CLSID/" + classID + "/Programmable/.", QString::null );
	    settings.writeEntry( "/CLSID/" + classID + "/ToolboxBitmap32/.", file + ", 101" );
	    settings.writeEntry( "/CLSID/" + classID + "/TypeLib/.", libID );
	    settings.writeEntry( "/CLSID/" + classID + "/Version/.", "1.0" );
	    settings.writeEntry( "/CLSID/" + classID + "/VersionIndependentProgID/.", module + "." + className );
	    settings.writeEntry( "/CLSID/" + classID + "/ProgID/.", module + "." + className + ".1" );
	    settings.writeEntry( "/CLSID/" + classID + "/Implemented Categories/.", QString::null );
	    //### TODO: write some list of categories
	    
	    settings.writeEntry( "/Interface/" + ifaceID + "/.", "I" + className );
	    settings.writeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid/.", "{00020424-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid32/.", "{00020424-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + ifaceID + "/TypeLib/.", libID );
	    settings.writeEntry( "/Interface/" + ifaceID + "/TypeLib/Version", "1.0" );
	    
	    settings.writeEntry( "/Interface/" + eventID + "/.", "_I" + className + "Events" );
	    settings.writeEntry( "/Interface/" + eventID + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + eventID + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + eventID + "/TypeLib/.", libID );
	    settings.writeEntry( "/Interface/" + eventID + "/TypeLib/Version", "1.0" );
	} else {	    
	    settings.removeEntry( "/" + module + "." + className + ".1/CLSID/." );
	    settings.removeEntry( "/" + module + "." + className + ".1/Insertable/." );
	    settings.removeEntry( "/" + module + "." + className + ".1/." );
	    
	    settings.removeEntry( "/" + module + "." + className + "/CLSID/." );
	    settings.removeEntry( "/" + module + "." + className + "/CurVer/." );
	    settings.removeEntry( "/" + module + "." + className + "/." );
	    
	    settings.removeEntry( "/CLSID/" + classID + "/AppID" );
	    settings.removeEntry( "/CLSID/" + classID + "/Control/." );
	    settings.removeEntry( "/CLSID/" + classID + "/Insertable/." );
	    settings.removeEntry( "/CLSID/" + classID + "/LocalServer32/." );
	    settings.removeEntry( "/CLSID/" + classID + "/MiscStatus/1/." );
	    settings.removeEntry( "/CLSID/" + classID + "/MiscStatus/." );	    
	    settings.removeEntry( "/CLSID/" + classID + "/Programmable/." );
	    settings.removeEntry( "/CLSID/" + classID + "/ToolboxBitmap32/." );
	    settings.removeEntry( "/CLSID/" + classID + "/TypeLib/." );
	    settings.removeEntry( "/CLSID/" + classID + "/Version/." );
	    settings.removeEntry( "/CLSID/" + classID + "/VersionIndependentProgID/." );
	    settings.removeEntry( "/CLSID/" + classID + "/ProgID/." );
	    //### TODO: remove some list of categories
	    settings.removeEntry( "/CLSID/" + classID + "/Implemented Categories/." );
	    settings.removeEntry( "/CLSID/" + classID + "/." );
	    
	    settings.removeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid/." );
	    settings.removeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid32/." );
	    settings.removeEntry( "/Interface/" + ifaceID + "/TypeLib/Version" );
	    settings.removeEntry( "/Interface/" + ifaceID + "/TypeLib/." );
	    settings.removeEntry( "/Interface/" + ifaceID + "/." );
	    
	    settings.removeEntry( "/Interface/" + eventID + "/ProxyStubClsid/." );
	    settings.removeEntry( "/Interface/" + eventID + "/ProxyStubClsid32/." );
	    settings.removeEntry( "/Interface/" + eventID + "/TypeLib/Version" );
	    settings.removeEntry( "/Interface/" + eventID + "/TypeLib/." );
	    settings.removeEntry( "/Interface/" + eventID + "/." );
	}
    }
    
    return S_OK;
}

QInterfacePtr<QActiveQtFactoryInterface> CExeModule::_factory = 0;

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

    QStringList cmds = QStringList::split( " ", cmdLine );
    int nRet = 0;
    bool bRun = TRUE;
    bool bRunMain = TRUE;
    for ( QStringList::Iterator it = cmds.begin(); it != cmds.end(); ++it ) {
	QString cmd = (*it).lower();
	if ( cmd == "-unregserver" || cmd == "/unregserver" ) {
	    qWarning( "Unregistering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(FALSE);
            bRun = FALSE;
	    break;
	} else if ( cmd == "-regserver" || cmd == "/regserver" ) {
	    qWarning( "Registering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(TRUE);
            bRun = FALSE;
            break;
	} else if ( cmd == "-activex" || cmd == "/activex" ) {
	    bRunMain = FALSE;
	}
    }

    if (bRun) {
	int argc;
	char* cmdp = 0;
	cmdp = new char[ cmdLine.length() + 1 ];
	qstrcpy( cmdp, cmdLine.latin1() );

	QMemArray<pchar> argv( 8 );
	qWinMain( hInstance, hPrevInstance, cmdp, nShowCmd, argc, argv );
	if ( bRunMain ) {
	    nRet = main( argc, argv.data() );
	} else {
	    HRESULT hRes = CoInitialize(NULL);

	    _ASSERTE(SUCCEEDED(hRes));
	    QStringList keys = _Module.factory()->featureList();
	    if ( !keys.count() )
		return nRet;
	    _ATL_OBJMAP_ENTRY *ObjectMap = new _ATL_OBJMAP_ENTRY[ keys.count() + 1 ];
	    memset( ObjectMap+keys.count(), 0, sizeof(_ATL_OBJMAP_ENTRY) );
	    int object = 0;
	    for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
		GUID *clsid = new GUID;
		*clsid = QUuid(*key);
		ObjectMap[object].pclsid = clsid;
		ObjectMap[object].pfnUpdateRegistry = UpdateRegistry;
		ObjectMap[object].pfnGetClassObject = GetClassObject;
		ObjectMap[object].pfnCreateInstance = CreateInstance;
		ObjectMap[object].pCF = NULL;
		ObjectMap[object].dwRegister = 0;
		ObjectMap[object].pfnGetObjectDescription = QActiveQtBase::GetObjectDescription;
		ObjectMap[object].pfnGetCategoryMap = QActiveQtBase::GetCategoryMap;
		ObjectMap[object].pfnObjectMain = QActiveQtBase::ObjectMain;
		++object;
	    }

	    const IID TypeLib = _Module.factory()->typeLibID();
	    _Module.Init(ObjectMap, hInstance, &TypeLib );
	    _Module.dwThreadID = GetCurrentThreadId();

	    _Module.StartMonitor();

	    _ATL_OBJMAP_ENTRY* pEntry = _Module.m_pObjMap;
	    while ( pEntry->pclsid )
	    {
		IUnknown* p = 0;
		HRESULT hRes = GetClassObject( pEntry->pfnCreateInstance, *pEntry->pclsid, (void**) &p );
		if (SUCCEEDED(hRes))
		    hRes = CoRegisterClassObject( *pEntry->pclsid, p, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &pEntry->dwRegister );
		if ( p )
		    p->Release();

		++pEntry;
	    }

	    nRet = main( argc, argv.data() );

	    pEntry = _Module.m_pObjMap;
	    while ( pEntry->pclsid )
	    {
		    pEntry->RevokeClassObject();
		    ++pEntry;
	    }
	    Sleep(dwPause); //wait for any threads to finish

	    _Module.Term();
	    delete QActiveQtBase::typeInfoHolderList;
	    QActiveQtBase::typeInfoHolderList = 0;

	    CoUninitialize();

	    object = 0;
	    while ( ObjectMap[object].pclsid ) {
		delete (GUID*)ObjectMap[object].pclsid;
		object++;
	    }
	    delete[] ObjectMap;
	}
    }

    return nRet;
}
