/****************************************************************************
** $Id$
**
** Implementation of QAccessible class for Win32
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qt_windows.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qmessagebox.h" // ### dependency
#include "qlibrary.h"

#include <winable.h>
#include <oleacc.h>
#ifndef Q_CC_BOR
#include <comdef.h>
#endif

void QAccessible::updateAccessibility( QObject *o, int who, Event reason )
{
    Q_ASSERT(o);

    QString soundName;
    switch ( reason ) {
    case PopupMenuStart:
	soundName = "MenuPopup";
	break;

    case MenuCommand:
	soundName = "MenuCommand";
	break;

    case Alert:
	if ( o->inherits( "QMessageBox" ) ) {
	    QMessageBox *mb = (QMessageBox*)o;
	    switch ( mb->icon() ) {
	    case QMessageBox::Warning:
		soundName = "SystemHand";
		break;
	    case QMessageBox::Critical:
		soundName = "SystemExclamation";
		break;
	    case QMessageBox::Information:
		soundName = ".Default";
		break;
	    default:
		soundName = "SystemAsterisk";
		break;
	    }
	} else {
	    soundName = "SystemAsterisk";
	}
	break;

    default:
	break;
    }

    if ( !!soundName ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    PlaySoundW( (TCHAR*)qt_winTchar( soundName, TRUE ), NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT );
	else
#endif
	    PlaySoundA( soundName.local8Bit(), NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT  );
    }

    typedef void (WINAPI *PtrNotifyWinEvent)(DWORD, HWND, LONG, LONG );
    
    static PtrNotifyWinEvent ptrNotifyWinEvent = 0;
    static bool resolvedNWE = FALSE;
    if ( !resolvedNWE ) {
	resolvedNWE = TRUE;
	ptrNotifyWinEvent = (PtrNotifyWinEvent)QLibrary::resolve( "user32", "NotifyWinEvent" );
    }
    if ( !ptrNotifyWinEvent )
	return;

    // An event has to be associated with a window, 
    // so find the first parent that is a widget.
    QWidget *w = 0;
    if ( o->isWidgetType() ) {
	w = (QWidget*)o;
    } else {
	QObject *p = o;
	while ( ( p = p->parent() ) != 0 ) {
	    if ( p->isWidgetType() ) {
		w = (QWidget*)p;
		break;
	    }
	}
    }

    if ( !w ) {
	if ( reason != QAccessible::ContextHelpStart && 
	     reason != QAccessible::ContextHelpEnd )
	    w = qApp->focusWidget();
	if ( !w ) {
	    w = qApp->activeWindow();
	    if ( !w ) {
		w = qApp->mainWidget();
		if ( !w )
		    return;
	    }
	}
    }

    if ( reason != MenuCommand ) // MenuCommand is faked
	ptrNotifyWinEvent( reason, w->winId(), OBJID_CLIENT, who );
}

class QWindowsEnumerate : public IEnumVARIANT
{
public:
    QWindowsEnumerate( const QMemArray<int> &a )
	: ref( 0 ), current( 0 ),array( a )
    {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID, LPVOID * );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE Clone( IEnumVARIANT **ppEnum );
    HRESULT STDMETHODCALLTYPE Next( unsigned long  celt, VARIANT FAR*  rgVar, unsigned long FAR*  pCeltFetched );
    HRESULT STDMETHODCALLTYPE Reset();
    HRESULT STDMETHODCALLTYPE Skip( unsigned long celt );

private:
    ULONG ref;
    ULONG current;
    QMemArray<int> array;
};

HRESULT STDMETHODCALLTYPE QWindowsEnumerate::QueryInterface( REFIID id, LPVOID *iface )
{
    *iface = 0;
    if ( id == IID_IUnknown )
	*iface = (IUnknown*)this;
    else if ( id == IID_IEnumVARIANT )
	*iface = (IEnumVARIANT*)this;

    if ( *iface ) {
	AddRef();
	return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE QWindowsEnumerate::AddRef()
{
    return ++ref;
}

ULONG STDMETHODCALLTYPE QWindowsEnumerate::Release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

HRESULT STDMETHODCALLTYPE QWindowsEnumerate::Clone( IEnumVARIANT **ppEnum )
{
    QWindowsEnumerate *penum = NULL;
    *ppEnum = NULL;

    penum = new QWindowsEnumerate( array );
    if ( !penum )
	return E_OUTOFMEMORY;
    penum->current = current;
    penum->array = array;
    penum->AddRef();
    *ppEnum = penum;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsEnumerate::Next( unsigned long  celt, VARIANT FAR*  rgVar, unsigned long FAR*  pCeltFetched )
{
    if ( pCeltFetched != NULL )
	*pCeltFetched = 0;

    ULONG l;
    for ( l = 0; l < celt; l++ ) {
	VariantInit( &rgVar[l] );
	if ( (current+1) > array.size() ) {
	    *pCeltFetched = l;
	    return S_FALSE;
	}

	rgVar[l].vt = VT_I4;
	rgVar[l].lVal = array[(int)current];
	++current;
    }
    *pCeltFetched = l;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsEnumerate::Reset()
{
    current = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsEnumerate::Skip( unsigned long celt )
{
    current += celt;
    if ( current > array.size() ) {
	current = array.size();
	return S_FALSE;
    }
    return S_OK;
}

/*
*/
class QWindowsAccessible : public IAccessible, QAccessible
{
public:
    QWindowsAccessible( QAccessibleInterface *a )
	: ref( 0 ), accessible( a )
    {
	accessible->addRef();
    }

    ~QWindowsAccessible()
    {
	accessible->release();
    }

    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID, LPVOID * );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE GetTypeInfoCount( unsigned int * );
    HRESULT STDMETHODCALLTYPE GetTypeInfo( unsigned int, unsigned long, ITypeInfo ** );
    HRESULT STDMETHODCALLTYPE GetIDsOfNames( const _GUID &, wchar_t **, unsigned int, unsigned long, long * );
    HRESULT STDMETHODCALLTYPE Invoke( long, const _GUID &, unsigned long, unsigned short, tagDISPPARAMS *, tagVARIANT *, tagEXCEPINFO *, unsigned int * );

    HRESULT STDMETHODCALLTYPE accHitTest( long xLeft, long yTop, VARIANT *pvarID );
    HRESULT STDMETHODCALLTYPE accLocation( long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varID );
    HRESULT STDMETHODCALLTYPE accNavigate( long navDir, VARIANT varStart, VARIANT *pvarEnd );
    HRESULT STDMETHODCALLTYPE get_accChild( VARIANT varChildID, IDispatch** ppdispChild );
    HRESULT STDMETHODCALLTYPE get_accChildCount( long* pcountChildren );
    HRESULT STDMETHODCALLTYPE get_accParent( IDispatch** ppdispParent );

    HRESULT STDMETHODCALLTYPE accDoDefaultAction( VARIANT varID );
    HRESULT STDMETHODCALLTYPE get_accDefaultAction( VARIANT varID, BSTR* pszDefaultAction );
    HRESULT STDMETHODCALLTYPE get_accDescription( VARIANT varID, BSTR* pszDescription );
    HRESULT STDMETHODCALLTYPE get_accHelp( VARIANT varID, BSTR *pszHelp );
    HRESULT STDMETHODCALLTYPE get_accHelpTopic( BSTR *pszHelpFile, VARIANT varChild, long *pidTopic );
    HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut( VARIANT varID, BSTR *pszKeyboardShortcut );
    HRESULT STDMETHODCALLTYPE get_accName( VARIANT varID, BSTR* pszName );
    HRESULT STDMETHODCALLTYPE put_accName( VARIANT varChild, BSTR szName );
    HRESULT STDMETHODCALLTYPE get_accRole( VARIANT varID, VARIANT *pvarRole );
    HRESULT STDMETHODCALLTYPE get_accState( VARIANT varID, VARIANT *pvarState );
    HRESULT STDMETHODCALLTYPE get_accValue( VARIANT varID, BSTR* pszValue );
    HRESULT STDMETHODCALLTYPE put_accValue( VARIANT varChild, BSTR szValue );

    HRESULT STDMETHODCALLTYPE accSelect( long flagsSelect, VARIANT varID );
    HRESULT STDMETHODCALLTYPE get_accFocus( VARIANT *pvarID );
    HRESULT STDMETHODCALLTYPE get_accSelection( VARIANT *pvarChildren );

private:
    ULONG ref;
    QAccessibleInterface *accessible;
};

/*
*/
IAccessible *qt_createWindowsAccessible( QAccessibleInterface *access )
{
    QWindowsAccessible *acc = new QWindowsAccessible( access );
    IAccessible *iface;
    acc->QueryInterface( IID_IAccessible, (void**)&iface );

    return iface;
}

/*
  IUnknown
*/
HRESULT STDMETHODCALLTYPE QWindowsAccessible::QueryInterface( REFIID id, LPVOID *iface )
{
    *iface = 0;
    if ( id == IID_IUnknown )
	*iface = (IUnknown*)this;
    else if ( id == IID_IDispatch )
	*iface = (IDispatch*)this;
    else if ( id == IID_IAccessible )
	*iface = (IAccessible*)this;

    if ( *iface ) {
	AddRef();
	return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE QWindowsAccessible::AddRef()
{
    return ++ref;
}

ULONG STDMETHODCALLTYPE QWindowsAccessible::Release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

/*
  IDispatch
*/

HRESULT STDMETHODCALLTYPE QWindowsAccessible::GetTypeInfoCount( unsigned int * pctinfo)
{
    // We don't use a type library
    *pctinfo = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::GetTypeInfo( unsigned int, unsigned long, ITypeInfo **pptinfo )
{
    // We don't use a type library
    *pptinfo = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::GetIDsOfNames( const _GUID &, wchar_t **rgszNames, unsigned int, unsigned long, long *rgdispid )
{
#ifndef Q_CC_BOR
    // PROPERTIES:  Hierarchical
    if ( _bstr_t(rgszNames[0]) == _bstr_t(L"accParent") ) 
	rgdispid[0] = DISPID_ACC_PARENT;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accChildCount") ) 
	rgdispid[0] = DISPID_ACC_CHILDCOUNT;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accChild") ) 
	rgdispid[0] = DISPID_ACC_CHILD;

    // PROPERTIES:  Descriptional
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accName(") ) 
	rgdispid[0] = DISPID_ACC_NAME;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accValue") ) 
	rgdispid[0] = DISPID_ACC_VALUE;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accDescription") ) 
	rgdispid[0] = DISPID_ACC_DESCRIPTION;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accRole") ) 
	rgdispid[0] = DISPID_ACC_ROLE;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accState") ) 
	rgdispid[0] = DISPID_ACC_STATE;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accHelp") ) 
	rgdispid[0] = DISPID_ACC_HELP;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accHelpTopic") ) 
	rgdispid[0] = DISPID_ACC_HELPTOPIC;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accKeyboardShortcut") ) 
	rgdispid[0] = DISPID_ACC_KEYBOARDSHORTCUT;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accFocus") ) 
	rgdispid[0] = DISPID_ACC_FOCUS;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accSelection") ) 
	rgdispid[0] = DISPID_ACC_SELECTION;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accDefaultAction") ) 
	rgdispid[0] = DISPID_ACC_DEFAULTACTION;

    // METHODS
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accSelect") ) 
	rgdispid[0] = DISPID_ACC_SELECT;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accLocation") ) 
	rgdispid[0] = DISPID_ACC_LOCATION;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accNavigate") ) 
	rgdispid[0] = DISPID_ACC_NAVIGATE;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accHitTest") ) 
	rgdispid[0] = DISPID_ACC_HITTEST;
    else if( _bstr_t(rgszNames[0]) == _bstr_t(L"accDoDefaultAction") ) 
	rgdispid[0] = DISPID_ACC_DODEFAULTACTION;
    else 
	return DISP_E_UNKNOWNINTERFACE;

    return S_OK;
#else
    Q_UNUSED( rgszNames );
    Q_UNUSED( rgdispid );

    return DISP_E_MEMBERNOTFOUND;
#endif
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::Invoke( long dispIdMember, const _GUID &, unsigned long, unsigned short wFlags, tagDISPPARAMS *pDispParams, tagVARIANT *pVarResult, tagEXCEPINFO *, unsigned int * )
{ 
    HRESULT hr = DISP_E_MEMBERNOTFOUND;

    switch( dispIdMember )
    {
	case DISPID_ACC_PARENT:
	    if ( wFlags == DISPATCH_PROPERTYGET ) {
		if ( pVarResult == NULL )
		    return E_INVALIDARG;
		hr = get_accParent( &pVarResult->pdispVal );
	    } else {
		hr = DISP_E_MEMBERNOTFOUND;
	    }
	    break;

	case DISPID_ACC_CHILDCOUNT:
	    if ( wFlags == DISPATCH_PROPERTYGET ) {
		if ( pVarResult == NULL )
		    return E_INVALIDARG;
		hr = get_accChildCount( &pVarResult->lVal );
	    } else {
		hr = DISP_E_MEMBERNOTFOUND;
	    }
	    break;
	
	case DISPID_ACC_CHILD:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accChild( pDispParams->rgvarg[0], &pVarResult->pdispVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_NAME:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accName( pDispParams->rgvarg[0], &pVarResult->bstrVal );
    	    else if ( wFlags == DISPATCH_PROPERTYPUT )
		hr = put_accName( pDispParams->rgvarg[0], pVarResult->bstrVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_VALUE:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accValue( pDispParams->rgvarg[0], &pVarResult->bstrVal );
	    else if ( wFlags == DISPATCH_PROPERTYPUT )
		hr = put_accValue( pDispParams->rgvarg[0], pVarResult->bstrVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_DESCRIPTION:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accDescription( pDispParams->rgvarg[0], &pVarResult->bstrVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_ROLE:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accRole( pDispParams->rgvarg[0], pVarResult );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_STATE:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accState( pDispParams->rgvarg[0], pVarResult );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_HELP:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accHelp( pDispParams->rgvarg[0], &pVarResult->bstrVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_HELPTOPIC:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accHelpTopic( &pDispParams->rgvarg[2].bstrVal, pDispParams->rgvarg[1], &pDispParams->rgvarg[0].lVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_KEYBOARDSHORTCUT:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accKeyboardShortcut( pDispParams->rgvarg[0], &pVarResult->bstrVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_FOCUS:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accFocus( pVarResult );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_SELECTION:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accSelection( pVarResult );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_DEFAULTACTION:
	    if ( wFlags == DISPATCH_PROPERTYGET )
		hr = get_accDefaultAction( pDispParams->rgvarg[0], &pVarResult->bstrVal );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_SELECT:
	    if ( wFlags == DISPATCH_METHOD )
		hr = accSelect( pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0] );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_LOCATION:
	    if ( wFlags == DISPATCH_METHOD )
		hr = accLocation( &pDispParams->rgvarg[4].lVal, &pDispParams->rgvarg[3].lVal, &pDispParams->rgvarg[2].lVal, &pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0] );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_NAVIGATE:
	    if ( wFlags == DISPATCH_METHOD )
		hr = accNavigate( pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0], pVarResult );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_HITTEST:
	    if ( wFlags == DISPATCH_METHOD )
		hr = accHitTest( pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].lVal, pVarResult );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	case DISPID_ACC_DODEFAULTACTION:
	    if ( wFlags == DISPATCH_METHOD )
		hr = accDoDefaultAction( pDispParams->rgvarg[0] );
	    else
		hr = DISP_E_MEMBERNOTFOUND;
	    break;

	default:
	    hr = DISP_E_MEMBERNOTFOUND;
	    break;
    } 
    
    return hr;
}

/* 
  IAccessible
*/
HRESULT STDMETHODCALLTYPE QWindowsAccessible::accHitTest( long xLeft, long yTop, VARIANT *pvarID )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    int control = accessible->controlAt( xLeft, yTop );
    if ( control == -1 ) {
	(*pvarID).vt = VT_EMPTY;
	return S_FALSE;
    }
    QAccessibleInterface *acc = 0;
    if ( control )
	accessible->queryChild( control, &acc );
    if ( !acc ) {
	(*pvarID).vt = VT_I4;
	(*pvarID).lVal = control;
	return S_OK;
    }

    QWindowsAccessible* wacc = new QWindowsAccessible( acc );
    acc->release();
    IDispatch *iface = 0;
    wacc->QueryInterface( IID_IDispatch, (void**)&iface );
    if ( iface ) {
	(*pvarID).vt = VT_DISPATCH;
	(*pvarID).pdispVal = iface;
	return S_OK;
    } else {
	delete wacc;
    }

    (*pvarID).vt = VT_EMPTY;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::accLocation( long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varID )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QRect rect = accessible->rect( varID.lVal );
    if ( rect.isValid() ) {
	*pxLeft = rect.x();
	*pyTop = rect.y();
	*pcxWidth = rect.width();
	*pcyHeight = rect.height();
    } else {
	*pxLeft = 0;
	*pyTop = 0;
	*pcxWidth = 0;
	*pcyHeight = 0;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::accNavigate( long navDir, VARIANT varStart, VARIANT *pvarEnd )
{
    if ( !accessible->isValid() )
	return E_FAIL;

    int control = accessible->navigate( (NavDirection)navDir, varStart.lVal );
    if ( control == -1 ) {
	(*pvarEnd).vt = VT_EMPTY;
	return S_FALSE;
    }
    QAccessibleInterface *acc = 0;
    if ( control ) {
	if ( varStart.lVal || navDir == NavFirstChild || navDir == NavLastChild || navDir == NavFocusChild ) {
	    accessible->queryChild( control, &acc );
	} else {
	    QAccessibleInterface *parent = 0;
	    accessible->queryParent( &parent );
	    if ( parent ) {
		parent->queryChild( control, &acc );
		parent->release();
	    }
	}
    } else {
	acc = accessible;
	acc->addRef();
    }
    if ( !acc ) {
	(*pvarEnd).vt = VT_I4;
	(*pvarEnd).lVal = control;
	return S_OK;
    }
    
    QWindowsAccessible* wacc = new QWindowsAccessible( acc );
    acc->release();
    IDispatch *iface = 0;
    wacc->QueryInterface( IID_IDispatch, (void**)&iface );
    if ( iface ) {
	(*pvarEnd).vt = VT_DISPATCH;
	(*pvarEnd).pdispVal = iface;
	return S_OK;
    } else {
	delete wacc;
    }

    (*pvarEnd).vt = VT_EMPTY;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accChild( VARIANT varChildID, IDispatch** ppdispChild )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    if ( varChildID.vt == VT_EMPTY )
	return E_INVALIDARG;

    QAccessibleInterface *acc = 0;
    if ( !varChildID.lVal ) {
	acc = accessible;
	acc->addRef();
    } else {
	accessible->queryChild( varChildID.lVal, &acc );
    }

    if ( acc ) {
	QWindowsAccessible* wacc = new QWindowsAccessible( acc );
	acc->release();
	wacc->QueryInterface( IID_IDispatch, (void**)ppdispChild );
	return S_OK;
    }

    *ppdispChild = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accChildCount( long* pcountChildren )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    *pcountChildren = accessible->childCount();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accParent( IDispatch** ppdispParent )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QAccessibleInterface *acc = 0;
    accessible->queryParent( &acc );
    if ( acc ) {
	QWindowsAccessible* wacc = new QWindowsAccessible( acc );
	acc->release();
	wacc->QueryInterface( IID_IDispatch, (void**)ppdispParent );

	if ( *ppdispParent )
	    return S_OK;
    }

    *ppdispParent = NULL;
    return S_FALSE;
}

/*
  Properties and methods
*/
HRESULT STDMETHODCALLTYPE QWindowsAccessible::accDoDefaultAction( VARIANT varID )
{
    if ( !accessible->isValid() )
	return E_FAIL;

    if ( accessible->doDefaultAction( varID.lVal ) )
	return S_OK;

    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accDefaultAction( VARIANT varID, BSTR* pszDefaultAction )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QString def = accessible->text( DefaultAction, varID.lVal );
    if ( !!def ) {
	*pszDefaultAction = SysAllocString( (TCHAR*)qt_winTchar( def, TRUE ) );
	return S_OK;
    }

    *pszDefaultAction = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accDescription( VARIANT varID, BSTR* pszDescription )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QString descr = accessible->text( Description, varID.lVal );
    if ( !!descr ) {
	*pszDescription = SysAllocString( (TCHAR*)qt_winTchar( descr, TRUE ) );
	return S_OK;
    }

    *pszDescription = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accHelp( VARIANT varID, BSTR *pszHelp )
{
    if ( !accessible->isValid() )
	return E_FAIL;

    QString help = accessible->text( Help, varID.lVal );
    if ( !!help ) {
	*pszHelp = SysAllocString( (TCHAR*)qt_winTchar( help, TRUE ) );
	return S_OK;
    }
    
    *pszHelp = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accHelpTopic( BSTR *, VARIANT, long * )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accKeyboardShortcut( VARIANT varID, BSTR *pszKeyboardShortcut )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QString sc = accessible->text( Accelerator, varID.lVal );
    if ( !!sc ) {
	*pszKeyboardShortcut = SysAllocString( (TCHAR*)qt_winTchar( sc, TRUE ) );
	return S_OK;
    }

    *pszKeyboardShortcut = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accName( VARIANT varID, BSTR* pszName )
{
    if ( !accessible->isValid() )
	return E_FAIL;

    QString n = accessible->text( Name, varID.lVal );
    if ( !!n ) {
	*pszName = SysAllocString( (TCHAR*)qt_winTchar( n, TRUE ) );
	return S_OK;
    }

    *pszName = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::put_accName( VARIANT, BSTR )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accRole( VARIANT varID, VARIANT *pvarRole )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    Role role = accessible->role( varID.lVal );
    if ( role != NoRole ) {
	(*pvarRole).vt = VT_I4;
	(*pvarRole).lVal = role;	
    } else {
	(*pvarRole).vt = VT_EMPTY;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accState( VARIANT varID, VARIANT *pvarState )
{
    if ( !accessible->isValid() )
	return E_FAIL;

    (*pvarState).vt = VT_I4;
    (*pvarState).lVal = accessible->state( varID.lVal );
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accValue( VARIANT varID, BSTR* pszValue )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QString value = accessible->text( Value, varID.lVal );
    if ( !value.isNull() ) {
	*pszValue = SysAllocString( (TCHAR*)qt_winTchar( value, TRUE ) );
	return S_OK;
    }

    *pszValue = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::put_accValue( VARIANT, BSTR )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::accSelect( long flagsSelect, VARIANT varID )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    bool res = FALSE;
    if ( flagsSelect & SELFLAG_TAKEFOCUS )
	res = accessible->setFocus( varID.lVal );
    if ( flagsSelect & SELFLAG_TAKESELECTION ) {
	accessible->clearSelection();
	res = accessible->setSelected( varID.lVal, TRUE, FALSE );
    }
    if ( flagsSelect & SELFLAG_EXTENDSELECTION )
	res = accessible->setSelected( varID.lVal, TRUE, TRUE );
    if ( flagsSelect & SELFLAG_ADDSELECTION )
	res = accessible->setSelected( varID.lVal, TRUE, FALSE );
    if ( flagsSelect & SELFLAG_REMOVESELECTION )
	res = accessible->setSelected( varID.lVal, FALSE, FALSE );

    return res ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accFocus( VARIANT *pvarID )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    int control = accessible->navigate( NavFocusChild, 0 );
    if ( control == -1 ) {
	(*pvarID).vt = VT_EMPTY;
	return S_FALSE;
    }
    QAccessibleInterface *acc = 0;
    if ( control )
	accessible->queryChild( control, &acc );
    if ( !acc ) {
	(*pvarID).vt = VT_I4;
	(*pvarID).lVal = control;
	return S_OK;
    }
    
    QWindowsAccessible* wacc = new QWindowsAccessible( acc );
    acc->release();
    IDispatch *iface = 0;
    wacc->QueryInterface( IID_IDispatch, (void**)&iface );
    if ( iface ) {
	(*pvarID).vt = VT_DISPATCH;
	(*pvarID).pdispVal = iface;
	return S_OK;
    } else {
	delete wacc;
    }

    (*pvarID).vt = VT_EMPTY;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accSelection( VARIANT *pvarChildren )
{ 
    if ( !accessible->isValid() )
	return E_FAIL;

    QMemArray<int> sel = accessible->selection();
    if ( sel.isEmpty() ) {
	(*pvarChildren).vt = VT_EMPTY;
	return S_FALSE;
    }
    if ( sel.size() == 1 ) {
	(*pvarChildren).vt = VT_I4;
	(*pvarChildren).lVal = sel[0];
	return S_OK;
    }
    IEnumVARIANT *iface = new QWindowsEnumerate( sel );
    IUnknown *uiface;
    iface->QueryInterface( IID_IUnknown, (void**)&uiface );
    (*pvarChildren).vt = VT_UNKNOWN;
    (*pvarChildren).punkVal = uiface;
    
    return S_OK;
}

#endif
