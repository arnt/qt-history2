/****************************************************************************
** $Id: $
**
** Implementation of win32 ActiveX server DLL routines
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qapplication.h>
#include <qwidgetlist.h>

#include <atlbase.h>
#include "qaxserverbase.h"
#include "../shared/types.h"

// in qaxservermain.cpp
extern char module_filename[MAX_PATH];
extern bool is_server;
extern ITypeLib *typeLibrary;

extern HRESULT __stdcall UpdateRegistry(int bRegister);
extern HRESULT __stdcall GetClassObject( void *pv, const GUID &iid, void **ppUnk );

bool qax_ownQApp = FALSE;

// in qapplication_win.cpp
extern Q_EXPORT bool qt_win_use_simple_timers;

STDAPI DllRegisterServer()
{
    return UpdateRegistry(TRUE);
}

STDAPI DllUnregisterServer()
{
    return UpdateRegistry(FALSE);
}

STDAPI DllGetClassObject(const GUID &clsid, const GUID &riid, void** ppv)
{
    GetClassObject( 0, clsid, ppv );
    if ( !*ppv )
	return CLASS_E_CLASSNOTAVAILABLE;
    return S_OK;
}

HHOOK hhook = 0;


STDAPI DllCanUnloadNow()
{
    if ( _Module.m_nLockCnt )
	return S_FALSE;
    if ( !qax_ownQApp )
	return S_OK;

    // check if qApp still runs widgets (in other DLLs)
    QWidgetList *widgets = qApp->allWidgets();
    int count = 0;
    if ( widgets ) {
	count = widgets->count();

	// remove all Qt generated widgets
	QWidgetListIt it( *widgets );
	while ( it.current() ) {
	    QWidget *w = it.current();
	    if ( w->testWFlags( Qt::WType_Desktop ) )
		count--;
	    ++it;
	}
	delete widgets;
    }
    if ( count )
	return S_FALSE;

    // no widgets left - destroy qApp
    if ( hhook )
	UnhookWindowsHookEx( hhook );

    delete qApp;
    qApp = 0;
    qax_ownQApp = FALSE;

    // never allow unloading - safety net for Internet Explorer
    return S_FALSE;
}


BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved )
{
    GetModuleFileNameA( hInstance, module_filename, MAX_PATH-1 );

    is_server = TRUE;

    if ( dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH ) {
        const IID TypeLib = _Module.factory()->typeLibID();
	_Module.Init(0, hInstance, &TypeLib );
	qt_win_use_simple_timers = TRUE;

	QString libFile( module_filename );
	BSTR oleLibFile = QStringToBSTR( libFile );
	LoadTypeLibEx( oleLibFile, REGKIND_NONE, &typeLibrary );
	SysFreeString( oleLibFile );
    } else {
	if ( typeLibrary )
	    typeLibrary->Release();

	_Module.Term();
    }

    return TRUE;
}
