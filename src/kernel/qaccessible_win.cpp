#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qt_windows.h"
#include "qwidget.h"
#include "qobjectlist.h"

#include <winable.h>
#include <oleacc.h>

bool qt_notify_accessibility( QObject *o, int reason )
{
    Q_ASSERT(o);

    // An event has to be associated with a window, 
    // so find the first parent that is a widget.
    QWidget *w = 0;
    if ( o->isWidgetType() ) {
	w = (QWidget*)o;
    } else {
	QObject *p = o;
	while ( ( p = o->parent() ) != 0 ) {
	    if ( p->isWidgetType() ) {
		w = (QWidget*)p;
		break;
	    }
	}
    }

    if ( !w )
	return FALSE;

    NotifyWinEvent( reason, w->winId(), OBJID_WINDOW, CHILDID_SELF );

    return TRUE;
}

/*
*/
class QWindowsAccessible : public IAccessible
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

HRESULT STDMETHODCALLTYPE QWindowsAccessible::GetTypeInfoCount( unsigned int * )
{
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::GetTypeInfo( unsigned int, unsigned long, ITypeInfo ** )
{
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::GetIDsOfNames( const _GUID &, wchar_t **, unsigned int, unsigned long, long * )
{
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::Invoke( long, const _GUID &, unsigned long, unsigned short, tagDISPPARAMS *, tagVARIANT *, tagEXCEPINFO *, unsigned int * )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

/* 
  IAccessible
*/
HRESULT STDMETHODCALLTYPE QWindowsAccessible::accHitTest( long xLeft, long yTop, VARIANT *pvarID )
{ 
    int who;
    QAccessibleInterface *acc = accessible->hitTest( xLeft, yTop, &who );

    if ( !acc ) {
	(*pvarID).vt = VT_EMPTY;
	return S_FALSE;
    }
    if ( acc == accessible ) {
	(*pvarID).vt = VT_I4;
	(*pvarID).lVal = who;
	return S_OK;
    }

    QWindowsAccessible* wacc = new QWindowsAccessible( acc );
    IDispatch *iface;
    wacc->QueryInterface( IID_IDispatch, (void**)&iface );
    if ( iface ) {
	(*pvarID).vt = VT_DISPATCH;
	(*pvarID).pdispVal = iface;
	return S_OK;
    } else {
	delete wacc;
    }

    (*pvarID).vt = VT_EMPTY;
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::accLocation( long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varID )
{ 
    QRect rect = accessible->location( varID.lVal );
    *pxLeft = rect.x();
    *pyTop = rect.y();
    *pcxWidth = rect.width();
    *pcyHeight = rect.height();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::accNavigate( long navDir, VARIANT varStart, VARIANT *pvarEnd )
{
    int target = varStart.lVal;
    QAccessibleInterface *acc = accessible->navigate( (QAccessible::NavDirection)navDir, &target );
    if ( !acc ) {
	(*pvarEnd).vt = VT_EMPTY;
	return S_FALSE;
    } else if ( acc == accessible ) {
	(*pvarEnd).vt = VT_I4;
	(*pvarEnd).lVal = target;
	return S_OK;
    }
    IDispatch *disp = 0;
    QWindowsAccessible* wacc = new QWindowsAccessible( acc );
    wacc->QueryInterface( IID_IDispatch, (void**)&disp );
    (*pvarEnd).vt = VT_DISPATCH;
    (*pvarEnd).pdispVal = disp;
    return S_OK;

    (*pvarEnd).vt = VT_EMPTY;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accChild( VARIANT varChildID, IDispatch** ppdispChild )
{ 
    if ( varChildID.vt == VT_EMPTY )
	return E_INVALIDARG;

    QAccessibleInterface *acc = accessible->child( varChildID.lVal );
    if ( acc ) {
	QWindowsAccessible* wacc = new QWindowsAccessible( acc );
	wacc->QueryInterface( IID_IDispatch, (void**)ppdispChild );
	return S_OK;
    }

    *ppdispChild = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accChildCount( long* pcountChildren )
{ 
    *pcountChildren = accessible->childCount();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accParent( IDispatch** ppdispParent )
{ 
    QAccessibleInterface *acc = accessible->parent();
    if ( acc ) {
	QWindowsAccessible* wacc = new QWindowsAccessible( acc );
	wacc->QueryInterface( IID_IDispatch, (void**)ppdispParent );

	if ( *ppdispParent )
	    return S_OK;
    }

    *ppdispParent = NULL;
    return S_FALSE;
}

/*!
  Properties and methods
*/
HRESULT STDMETHODCALLTYPE QWindowsAccessible::accDoDefaultAction( VARIANT varID )
{
    if ( accessible->doDefaultAction( varID.lVal ) )
	return S_OK;

    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accDefaultAction( VARIANT varID, BSTR* pszDefaultAction )
{ 
    QString def = accessible->defaultAction( varID.lVal );
    if ( !!def ) {
	*pszDefaultAction = SysAllocString( (TCHAR*)qt_winTchar( def, TRUE ) );
	return S_OK;
    }

    *pszDefaultAction = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accDescription( VARIANT varID, BSTR* pszDescription )
{ 
    QString descr = accessible->description( varID.lVal );
    if ( !!descr ) {
	*pszDescription = SysAllocString( (TCHAR*)qt_winTchar( descr, TRUE ) );
	return S_OK;
    }

    *pszDescription = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accHelp( VARIANT varID, BSTR *pszHelp )
{
    QString help = accessible->help( varID.lVal );
    if ( !!help ) {
	*pszHelp = SysAllocString( (TCHAR*)qt_winTchar( help, TRUE ) );
	return S_OK;
    }
    
    *pszHelp = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accHelpTopic( BSTR *pszHelpFile, VARIANT varChild, long *pidTopic )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accKeyboardShortcut( VARIANT varID, BSTR *pszKeyboardShortcut )
{ 
    QString sc = accessible->accelerator( varID.lVal );
    if ( !!sc ) {
	*pszKeyboardShortcut = SysAllocString( (TCHAR*)qt_winTchar( sc, TRUE ) );
	return S_OK;
    }

    *pszKeyboardShortcut = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accName( VARIANT varID, BSTR* pszName )
{
    QString n = accessible->name( varID.lVal );
    if ( !!n ) {
	*pszName = SysAllocString( (TCHAR*)qt_winTchar( n, TRUE ) );
	return S_OK;
    }

    *pszName = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::put_accName( VARIANT varID, BSTR szName )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accRole( VARIANT varID, VARIANT *pvarRole )
{ 
    QAccessible::Role role = accessible->role( varID.lVal );
    if ( role != QAccessible::NoRole ) {
	(*pvarRole).vt = VT_I4;
	(*pvarRole).lVal = role;	
    } else {
	(*pvarRole).vt = VT_EMPTY;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accState( VARIANT varID, VARIANT *pvarState )
{
    (*pvarState).vt = VT_I4;
    (*pvarState).lVal = accessible->state( varID.lVal );
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accValue( VARIANT varID, BSTR* pszValue )
{ 
    QString value = accessible->value( varID.lVal );
    if ( !value.isNull() ) {
	*pszValue = SysAllocString( (TCHAR*)qt_winTchar( value, TRUE ) );
	return S_OK;
    }

    *pszValue = NULL;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::put_accValue( VARIANT varChild, BSTR szValue )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::accSelect( long flagsSelect, VARIANT varID )
{ 
    if ( varID.lVal == CHILDID_SELF ) {
	if ( flagsSelect & ( SELFLAG_TAKEFOCUS ) )
	    qDebug( "Set focus" );

	return S_OK;
    } else {
	if ( flagsSelect & ( SELFLAG_TAKEFOCUS | SELFLAG_TAKESELECTION ) )
	    qDebug( "simulate a click" );
	else if ( flagsSelect & ( SELFLAG_TAKEFOCUS | SELFLAG_ADDSELECTION ) )
	    qDebug( "simulate a CTRL+click on a non-selected object" );
	else if ( flagsSelect & ( SELFLAG_TAKEFOCUS | SELFLAG_REMOVESELECTION ) )
	    qDebug( "simulate a CTRL+click on a selected object" );
	else if ( flagsSelect & ( SELFLAG_TAKEFOCUS | SELFLAG_EXTENDSELECTION ) )
	    qDebug( "simulate a SHIFT+click" );
    }

    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accFocus( VARIANT *pvarID )
{ 
    int who;
    QAccessibleInterface *acc = accessible->hasFocus( &who );

    if ( !acc ) {
	(*pvarID).vt = VT_EMPTY;
	return S_FALSE;
    }
    if ( acc == accessible ) {
	(*pvarID).vt = VT_I4;
	(*pvarID).lVal = who;
	return S_OK;
    }

    QWindowsAccessible* wacc = new QWindowsAccessible( acc );
    IDispatch *iface;
    wacc->QueryInterface( IID_IDispatch, (void**)&iface );
    (*pvarID).vt = VT_DISPATCH;
    (*pvarID).pdispVal = iface;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE QWindowsAccessible::get_accSelection( VARIANT *pvarChildren )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

#endif
