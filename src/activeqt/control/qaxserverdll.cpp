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

#include <qapplication.h>
#include <qwidget.h>

#include <qt_windows.h>

bool qax_ownQApp = false;
HHOOK qax_hhook = 0;

// in qaxserver.cpp
extern char qAxModuleFilename[MAX_PATH];
extern bool qAxIsServer;
extern ITypeLib *qAxTypeLibrary;
extern unsigned long qAxLockCount();
extern QString qAxInit();
extern void qAxCleanup();
extern HANDLE qAxInstance;

extern HRESULT UpdateRegistry(int bRegister);
extern HRESULT GetClassObject(const GUID &clsid, const GUID &iid, void **ppUnk);

STDAPI DllRegisterServer()
{
    return UpdateRegistry(true);
}

STDAPI DllUnregisterServer()
{
    return UpdateRegistry(false);
}

STDAPI DllGetClassObject(const GUID &clsid, const GUID &iid, void** ppv)
{
    GetClassObject(clsid, iid, ppv);
    if (!*ppv)
        return CLASS_E_CLASSNOTAVAILABLE;
    return S_OK;
}

STDAPI DllCanUnloadNow()
{
    if (qAxLockCount())
        return S_FALSE;
    if (!qax_ownQApp)
        return S_OK;
    
    // check if qApp still runs widgets (in other DLLs)
    QWidgetList widgets = qApp->allWidgets();
    int count = widgets.count();
    for (int w = 0; w < widgets.count(); ++w) {
        // remove all Qt generated widgets
        QWidget *widget = widgets.at(w);
        if (widget->windowFlags() & Qt::Desktop)
            count--;
    }
    if (count)
        return S_FALSE;
    
    // no widgets left - destroy qApp
    if (qax_hhook)
        UnhookWindowsHookEx(qax_hhook);
    
    delete qApp;
    qax_ownQApp = false;
    
    // never allow unloading - safety net for Internet Explorer
    return S_FALSE;
}


EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    GetModuleFileNameA(hInstance, qAxModuleFilename, MAX_PATH-1);
    qAxInstance = hInstance;
    qAxIsServer = true;
    
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstance);
        qAxInit();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        qAxCleanup();
    }
    
    return true;
}
