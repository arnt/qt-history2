/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
static bool qAxActivity = false;
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
extern bool qAxOutProcServer;
extern char qAxModuleFilename[MAX_PATH];
extern QString qAxInit();
extern void qAxCleanup();
extern HRESULT UpdateRegistry(BOOL bRegister);
extern HRESULT GetClassObject(const GUID &clsid, const GUID &iid, void **ppUnk);
extern ulong qAxLockCount();
extern bool qax_winEventFilter(void *message);

#if defined(Q_CC_BOR)
extern "C" __stdcall HRESULT DumpIDL(const QString &outfile, const QString &ver);
#else
STDAPI DumpIDL(const QString &outfile, const QString &ver);
#endif

// Monitors the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    while (1) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            qAxActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!qAxActivity && !qAxLockCount()) // if no activity let's really bail
            break;
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    PostQuitMessage(0);
    
    return 0;
}

// Starts the monitoring thread
static bool StartMonitor()
{
    dwThreadID = GetCurrentThreadId();
    hEventShutdown = CreateEventA(0, false, false, 0);
    if (hEventShutdown == 0)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(0, 0, MonitorProc, 0, 0, &dwThreadID);
    return (h != NULL);
}

void qax_shutDown()
{
    qAxActivity = true;
    if (hEventShutdown)
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
}

/*
    Start the COM server (if necessary).
*/
bool qax_startServer(QAxFactory::ServerType type)
{
    if (qAxIsServer)
        return true;
    
    const QStringList keys = qAxFactory()->featureList();
    if (!keys.count())
        return false;
    
    if (!qAxFactory()->isService())
        StartMonitor();
    
    classRegistration = new DWORD[keys.count()];
    int object = 0;
    for (QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key, ++object) {
        IUnknown* p = 0;
        CLSID clsid = qAxFactory()->classID(*key);
        
        // Create a QClassFactory (implemented in qaxserverbase.cpp)
        HRESULT hRes = GetClassObject(clsid, IID_IClassFactory, (void**)&p);
        if (SUCCEEDED(hRes))
            hRes = CoRegisterClassObject(clsid, p, CLSCTX_LOCAL_SERVER,
                type == QAxFactory::MultipleInstances ? REGCLS_MULTIPLEUSE : REGCLS_SINGLEUSE,
                classRegistration+object);
        if (p)
            p->Release();
    }

    qAxIsServer = true;
    return true;
}

/*
    Stop the COM server (if necessary).
*/
bool qax_stopServer()
{
    if (!qAxIsServer || !classRegistration)
        return true;
    
    qAxIsServer = false;
    
    const QStringList keys = qAxFactory()->featureList();
    int object = 0;
    for (QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key, ++object)
        CoRevokeClassObject(classRegistration[object]);
    
    delete []classRegistration;
    classRegistration = 0;
    
    Sleep(dwPause); //wait for any threads to finish
    
    return true;
}

#if defined(Q_OS_TEMP)
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#endif

#if defined(QT_NEEDS_QMAIN)
int qMain(int, char **);
#define main qMain
#else
#ifdef Q_OS_TEMP
extern "C" int __cdecl main(int, char **);
#else
extern "C" int main(int, char **);
#endif
#endif


EXTERN_C int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nShowCmd)
{
    qAxOutProcServer = true;
    GetModuleFileNameA(0, qAxModuleFilename, MAX_PATH-1);
    qAxInstance = hInstance;

    QByteArray cmdParam;
    QT_WA({
        LPTSTR cmdline = GetCommandLineW();
        cmdParam = QString::fromUtf16(cmdline).toLocal8Bit();
    }, {
        cmdParam = GetCommandLineA();
    });
    
    QList<QByteArray> cmds = cmdParam.split(' ');
    QByteArray unprocessed;

    int nRet = 0;
    bool run = true;
    bool runServer = false;
    for (int i = 0; i < cmds.count(); ++i) {
        QByteArray cmd = cmds.at(i);
        if (cmd == "-activex" || cmd == "/activex" || cmd == "-embedding" || cmd == "/embedding") {
            runServer = true;
        } else if (cmd == "-unregserver" || cmd == "/unregserver") {
            nRet = UpdateRegistry(false);
            run = false;
            break;
        } else if (cmd == "-regserver" || cmd == "/regserver") {
            nRet = UpdateRegistry(true);
            run = false;
            break;
        } else if (cmd == "-dumpidl" || cmd == "/dumpidl") {
            ++i;
            if (i < cmds.count()) {
                QByteArray outfile = cmds.at(i);
                ++i;
                QByteArray version;
                if (i < cmds.count() && (cmds.at(i) == "-version" || cmds.at(i) == "/version")) {
                    ++i;
                    if (i < cmds.count())
                        version = cmds.at(i);
                    else
                        version = "1.0";
                }
                
                nRet = DumpIDL(outfile, version);
            } else {
                qWarning("Wrong commandline syntax: <app> -dumpidl <idl file> [-version <x.y.z>]");
            }
            run = false;
            break;
        } else {
            unprocessed += cmds.at(i) + " ";
        }
    }
    
    if (run) {
        HRESULT hRes = CoInitialize(0);

        int argc;
        QVector<char*> argv(8);
        qWinMain(hInstance, hPrevInstance, unprocessed.data(), nShowCmd, argc, argv);
        qAxInit();
        if (runServer)
            QAxFactory::startServer();
        nRet = main(argc, argv.data());
        QAxFactory::stopServer();
        qAxCleanup();
        CoUninitialize();
        
    }
    
    return nRet;
}


