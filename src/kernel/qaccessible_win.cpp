#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qt_windows.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qwhatsthis.h"

#include <winable.h>
#include <oleacc.h>

bool QAccessible::notify( QObject *o, Reason reason )
{
    Q_ASSERT(o);

    DWORD event;

    switch ( reason ) {
    case StateChange:
	event = EVENT_OBJECT_STATECHANGE;
	break;
    case ValueChange:
	event = EVENT_OBJECT_VALUECHANGE;
	break;
    case NameChange:
	event = EVENT_OBJECT_NAMECHANGE;
	break;
    case DescriptionChange:
	event = EVENT_OBJECT_DESCRIPTIONCHANGE;
	break;
    case Selection:
	event = EVENT_OBJECT_SELECTION;
	break;
    case SelectionAdd:
	event = EVENT_OBJECT_SELECTIONADD;
	break;
    case SelectionRemove:
	event = EVENT_OBJECT_SELECTIONREMOVE;
	break;
    case SelectionWithin:
	event = EVENT_OBJECT_SELECTIONWITHIN;
	break;
    case Focus:
	event = EVENT_OBJECT_FOCUS;
	break;
    case MenuStart:
	event = EVENT_SYSTEM_MENUSTART;
	break;
    case MenuEnd:
	event = EVENT_SYSTEM_MENUEND;
	break;
    case PopupMenuStart:
	event = EVENT_SYSTEM_MENUPOPUPSTART;
	break;
    case PopupMenuEnd:
	event = EVENT_SYSTEM_MENUPOPUPEND;
	break;
    case DragDropStart:
	event = EVENT_SYSTEM_DRAGDROPSTART;
	break;
    case DragDropEnd:
	event = EVENT_SYSTEM_DRAGDROPEND;
	break;
    case DialogStart:
	event = EVENT_SYSTEM_DIALOGSTART;
	break;
    case DialogEnd:
	event = EVENT_SYSTEM_DIALOGEND;
	break;
    case ObjectShow:
	event = EVENT_OBJECT_SHOW;
	break;
    case ObjectHide:
	event = EVENT_OBJECT_HIDE;
	break;
    case ObjectReorder:
	event = EVENT_OBJECT_REORDER;
	break;
    default:
	event = 0;
	break;
    }
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

    NotifyWinEvent( event, w->winId(), OBJID_WINDOW, CHILDID_SELF );

    return TRUE;
}

/*
*/
class QWindowsAccessible : public IAccessible
{
public:
    QWindowsAccessible( QWidget *w )
	: ref( 0 ), widget( w )
    {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID, LPVOID * );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE GetTypeInfoCount( unsigned int * );
    HRESULT STDMETHODCALLTYPE GetTypeInfo( unsigned int, unsigned long, ITypeInfo ** );
    HRESULT STDMETHODCALLTYPE GetIDsOfNames( const _GUID &, unsigned short **, unsigned int, unsigned long, long * );
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
    QWidget *widget;
};

/*
*/
IAccessible *qt_createWindowsAccessible( QObject *object )
{
/*
    QAccessibe *a = object->accessibilityInfo();
    if ( !a )
	return NULL;
*/
    QWidget *widget = (QWidget*)object;

    QWindowsAccessible *acc = new QWindowsAccessible( widget );
    IAccessible *iface;
    acc->QueryInterface( IID_IAccessible, (void**)&iface );

    return iface;
}

/*
  IUnknown
*/
HRESULT QWindowsAccessible::QueryInterface( REFIID id, LPVOID *iface )
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

ULONG QWindowsAccessible::AddRef()
{
    return ++ref;
}

ULONG QWindowsAccessible::Release()
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

HRESULT QWindowsAccessible::GetTypeInfoCount( unsigned int * )
{
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::GetTypeInfo( unsigned int, unsigned long, ITypeInfo ** )
{
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::GetIDsOfNames( const _GUID &, unsigned short **, unsigned int, unsigned long, long * )
{
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::Invoke( long, const _GUID &, unsigned long, unsigned short, tagDISPPARAMS *, tagVARIANT *, tagEXCEPINFO *, unsigned int * )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

/* 
  IAccessible
*/
HRESULT QWindowsAccessible::accHitTest( long xLeft, long yTop, VARIANT *pvarID )
{ 
    QWidget *w = QApplication::widgetAt( xLeft, yTop, TRUE );
    if ( !w ) {
	(*pvarID).vt = VT_EMPTY;
	return S_FALSE;
    }
    if ( w == widget ) {
	(*pvarID).vt = VT_I4;
	(*pvarID).lVal = CHILDID_SELF;
	return S_OK;
    } 

    QWidget *p = w;
    while ( ( p = p->parentWidget() ) ) {
	if ( p == widget )
	    break;
    }
    if ( p == widget ) {
	QWindowsAccessible* acc = new QWindowsAccessible( w );
	IDispatch *iface;
	acc->QueryInterface( IID_IDispatch, (void**)&iface );
	if ( iface ) {
	    (*pvarID).vt = VT_DISPATCH;
	    (*pvarID).pdispVal = iface;
	    return S_OK;
	} else {
	    delete acc;
	}
    }
    
    (*pvarID).vt = VT_EMPTY;
    return S_FALSE;
}

HRESULT QWindowsAccessible::accLocation( long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varID )
{ 
    if ( varID.lVal == CHILDID_SELF ) {
	QPoint wpos = widget->mapToGlobal( QPoint( 0, 0 ) );
	*pxLeft = wpos.x();
	*pyTop = wpos.y();
	*pcxWidth = widget->width();
	*pcyHeight = widget->height();
    } else {
    }

    return S_OK;
}

HRESULT QWindowsAccessible::accNavigate( long navDir, VARIANT varStart, VARIANT *pvarEnd )
{
    if ( varStart.lVal == CHILDID_SELF ) {
	// ask parent widget for next child in focus chain
	// look up accessibility object in QPtrDict (?)
	(*pvarEnd).vt = VT_EMPTY;
    } else {
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accChild( VARIANT varChildID, IDispatch** ppdispChild )
{ 
    *ppdispChild = NULL;
    if ( varChildID.vt == VT_EMPTY )
	return E_INVALIDARG;

    long id = varChildID.lVal;
    QObjectList *ol = widget->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !ol || !ol->count() ) {
	delete ol;
	return S_FALSE;
    }

    QObject *o = ol->at( id-1 );
    delete ol;
    if ( !o )
	return S_FALSE;

    Q_ASSERT( o->isWidgetType() );
    QWidget *w = (QWidget*)o;

    QWindowsAccessible* acc = new QWindowsAccessible( w );
    acc->QueryInterface( IID_IDispatch, (void**)ppdispChild );

    if ( *ppdispChild )
	return S_OK;

    delete acc;
    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accChildCount( long* pcountChildren )
{ 
    QObjectList *ol = widget->queryList( "QWidget", 0, FALSE, FALSE );

    if ( ol )
	*pcountChildren = ol->count();
    else
	*pcountChildren = 0;

    delete ol;

    return S_OK;
}

HRESULT QWindowsAccessible::get_accParent( IDispatch** ppdispParent )
{ 
    *ppdispParent = NULL;
    if ( !widget->parentWidget() )
	return S_FALSE;

    QWindowsAccessible* acc = new QWindowsAccessible( widget->parentWidget() );
    acc->QueryInterface( IID_IDispatch, (void**)ppdispParent );

    if ( *ppdispParent )
	return S_OK;

    return S_FALSE;
}

HRESULT QWindowsAccessible::accDoDefaultAction( VARIANT varID )
{
    if ( varID.lVal == CHILDID_SELF ) {
    } else {
    }

    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::get_accDefaultAction( VARIANT varID, BSTR* pszDefaultAction )
{ 
    *pszDefaultAction = NULL;
    if ( varID.lVal == CHILDID_SELF ) {
	QString def = widget->accessibilityInfo()->defaultAction();
	if ( !!def ) {
	    *pszDefaultAction = SysAllocString( (TCHAR*)qt_winTchar( def, TRUE ) );
	    return S_OK;
	}
    } else {
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accDescription( VARIANT varID, BSTR* pszDescription )
{ 
    *pszDescription = NULL;
    if ( varID.lVal == CHILDID_SELF ) {
	QString descr = widget->accessibilityInfo()->description();
	if ( !!descr ) {
	    *pszDescription = SysAllocString( (TCHAR*)qt_winTchar( descr, TRUE ) );
	    return S_OK;
	}
    } else {
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accHelp( VARIANT varID, BSTR *pszHelp )
{
    *pszHelp = NULL;
    if ( varID.lVal == CHILDID_SELF ) {
	QString help = widget->accessibilityInfo()->help();
	if ( !!help ) {
	    *pszHelp = SysAllocString( (TCHAR*)qt_winTchar( widget->accessibilityInfo()->help(), TRUE ) );
	    return S_OK;
	}
    } else {
    }
    
    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accHelpTopic( BSTR *pszHelpFile, VARIANT varChild, long *pidTopic )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::get_accKeyboardShortcut( VARIANT varID, BSTR *pszKeyboardShortcut )
{ 
    *pszKeyboardShortcut = NULL;
    if ( varID.lVal == CHILDID_SELF ) {
	QString sc;
	if ( !!sc ) {
	    // ?
	    return S_OK;
	}
    } else {
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accName( VARIANT varID, BSTR* pszName )
{
    *pszName = NULL;
    if ( varID.lVal == CHILDID_SELF ) {
	QString n = widget->accessibilityInfo()->name();
	if ( !!n ) {
	    *pszName = SysAllocString( (TCHAR*)qt_winTchar( n, TRUE ) );
	    return S_OK;
	}
    } else {
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::put_accName( VARIANT varID, BSTR szName )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::get_accRole( VARIANT varID, VARIANT *pvarRole )
{ 
    if ( varID.lVal == CHILDID_SELF ) {
	(*pvarRole).vt = VT_I4;
	long role = ROLE_SYSTEM_CLIENT;
	if ( widget->inherits( "QMainWindow" ) )
	    role = ROLE_SYSTEM_APPLICATION;
	else if ( widget->inherits( "QCheckBox" ) )
	    role = ROLE_SYSTEM_CHECKBUTTON;
	else if ( widget->inherits( "QComboBox" ) )
	    role = ROLE_SYSTEM_COMBOBOX;
	else if ( widget->inherits( "QWorkspaceChild" ) )
	    role = ROLE_SYSTEM_DOCUMENT;
	else if ( widget->inherits( "QDial" ) )
	    role = ROLE_SYSTEM_DIAL;
	else if ( widget->inherits( "QDialog" ) )
	    role = ROLE_SYSTEM_DIALOG;
	else if ( widget->inherits( "QSizeGrip" ) )
	    role = ROLE_SYSTEM_GRIP;
	else if ( widget->inherits( "QGroupBox" ) )
	    role = ROLE_SYSTEM_GROUPING;
	else if ( widget->inherits( "QListBox" ) )
	    role = ROLE_SYSTEM_LIST;
	else if ( widget->inherits( "QMenuBar" ) )
	    role = ROLE_SYSTEM_MENUBAR;
	else if ( widget->inherits( "QPopupMenu" ) )
	    role = ROLE_SYSTEM_MENUPOPUP;
	else if ( widget->inherits( "QListView" ) )
	    role = ROLE_SYSTEM_OUTLINE;

	(*pvarRole).lVal = role;
	return S_OK;
    } else {
	(*pvarRole).vt = VT_EMPTY;
	return S_FALSE;
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accState( VARIANT varID, VARIANT *pvarState )
{
    if ( varID.lVal == CHILDID_SELF ) {
	(*pvarState).vt = VT_I4;
	(*pvarState).lVal = 0;
	if ( !widget->isEnabled() )
	    (*pvarState).lVal |= STATE_SYSTEM_UNAVAILABLE;
	if ( widget->isActiveWindow() && widget->focusPolicy() != QWidget::NoFocus )
	    (*pvarState).lVal |= STATE_SYSTEM_FOCUSABLE;
	if ( widget->hasFocus() )
	    (*pvarState).lVal |= STATE_SYSTEM_FOCUSED;

	return S_OK;
    } else {
	return S_FALSE;
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::get_accValue( VARIANT varID, BSTR* pszValue )
{ 
    *pszValue = NULL;
    if ( varID.lVal == CHILDID_SELF ) {
	QString value = widget->accessibilityInfo()->value();
	if ( !value.isNull() ) {
	    *pszValue = SysAllocString( (TCHAR*)qt_winTchar( widget->accessibilityInfo()->value(), TRUE ) );
	    return S_OK;
	}
    } else {
    }

    return S_FALSE;
}

HRESULT QWindowsAccessible::put_accValue( VARIANT varChild, BSTR szValue )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT QWindowsAccessible::accSelect( long flagsSelect, VARIANT varID )
{ 
    if ( varID.lVal == CHILDID_SELF ) {
	if ( flagsSelect & ( SELFLAG_TAKEFOCUS ) )
	    widget->setFocus();
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

HRESULT QWindowsAccessible::get_accFocus( VARIANT *pvarID )
{ 
    if ( !widget->isActiveWindow() )
	return S_FALSE;

    if ( widget->hasFocus() ) {
	(*pvarID).vt = VT_I4;
	(*pvarID).lVal = CHILDID_SELF;
    } else {
	(*pvarID).vt = VT_EMPTY;
    }	
	
    return S_OK;
}

HRESULT QWindowsAccessible::get_accSelection( VARIANT *pvarChildren )
{ 
    return DISP_E_MEMBERNOTFOUND;
}

#endif
