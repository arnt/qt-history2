#ifndef QACTIVEQTBASE_H
#define QACTIVEQTBASE_H

#include <qmap.h>
#include <quuid.h>
#include <qobject.h>
#include <qintdict.h>
#include "qactiveqt.h"

#include <atlbase.h>
class CExeModule : public CComModule
{
public:
    LONG Unlock();
    DWORD dwThreadID;
    HANDLE hEventShutdown;
    void MonitorShutdown();
    bool StartMonitor();
    bool bActivity;

    static QActiveQtFactoryInterface *factory();
    static QInterfacePtr<QActiveQtFactoryInterface> _factory;
};

extern CExeModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <atlhost.h>

class QWidget;

/////////////////////////////////////////////////////////////////////////////
// QActiveQtBase
class QActiveQtBase : 
    public QObject,
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch,
    public CComControl<QActiveQtBase>,
    public IOleControlImpl<QActiveQtBase>,
    public IOleObjectImpl<QActiveQtBase>,
    public IOleInPlaceActiveObjectImpl<QActiveQtBase>,
    public IViewObjectExImpl<QActiveQtBase>,
    public IOleInPlaceObjectWindowlessImpl<QActiveQtBase>,
    public IQuickActivateImpl<QActiveQtBase>,
    public IProvideClassInfo2,
    public CComCoClass<QActiveQtBase>,
    public IConnectionPointContainer,
    public IPersistPropertyBag
{
public:
    typedef QMap<QUuid,IConnectionPoint*> ConnectionPoints;
    typedef QMap<QUuid,IConnectionPoint*>::Iterator ConnectionPointsIterator;

    QActiveQtBase( IID QAxClass );

    ~QActiveQtBase();

BEGIN_COM_MAP(QActiveQtBase)
    {&IID_IDispatch,
	offsetofclass(IDispatch, _ComMapClass),
	_ATL_SIMPLEMAPENTRY},
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
{NULL, 0, 0}}; return _entries;}


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
    HRESULT WINAPI QueryInterface( REFIID iid, void **iface )
    {
	*iface = 0;
	if ( iid == IID_IUnknown )
	    *iface = (IUnknown*)(IDispatch*)this;
	else if ( iid == IID_IDispatch )
	    *iface = (IDispatch*)this;
	else if ( iid == IID_IViewObjectEx )
	    *iface = (IViewObjectEx*)this;
	else if ( iid == IID_IViewObject2 )
	    *iface = (IViewObject2*)(IViewObjectEx*)this;
	else if ( iid == IID_IViewObject )
	    *iface = (IViewObject*)(IViewObjectEx*)this;
	else if ( iid == IID_IOleInPlaceObjectWindowless )
	    *iface = (IOleInPlaceObjectWindowless*)this;
	else if ( iid == IID_IOleInPlaceObject )
	    *iface = (IOleInPlaceObject*)(IOleInPlaceObjectWindowless*)this;
	else if ( iid == IID_IOleWindow )
	    *iface = (IOleWindow*)(IOleInPlaceObjectWindowless*)this;
	else if ( iid == IID_IOleInPlaceActiveObject )
	    *iface = (IOleInPlaceActiveObject*)this;
	else if ( iid == IID_IOleControl )
	    *iface = (IOleControl*)this;
	else if ( iid == IID_IOleObject )
	    *iface = (IOleObject*)this;
	else if ( iid == IID_IQuickActivate )
	    *iface = (IQuickActivate*)this;
	else if ( iid == IID_IConnectionPointContainer )
	    *iface = (IConnectionPointContainer*)this;
	else if ( iid == IID_IProvideClassInfo )
	    *iface = (IProvideClassInfo*)this;
	else if ( iid == IID_IProvideClassInfo2 )
	    *iface = (IProvideClassInfo2*)this;
	else if ( iid == IID_IPersistPropertyBag )
	    *iface = (IPersistPropertyBag*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

BEGIN_MSG_MAP(QActiveQtBase)
    CHAIN_MSG_MAP(CComControl<QActiveQtBase>)
    DEFAULT_REFLECTION_HANDLER()
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SHOWWINDOW, ForwardMessage )
    MESSAGE_HANDLER(WM_PAINT, ForwardMessage )
    MESSAGE_HANDLER(WM_SIZE, ForwardMessage )
    MESSAGE_HANDLER(WM_ACTIVATE, ForwardMessage)
    MESSAGE_HANDLER(WM_KEYUP, ForwardMessage)
    MESSAGE_HANDLER(WM_KEYDOWN, ForwardMessage)
    MESSAGE_HANDLER(WM_CHAR, ForwardMessage)
    MESSAGE_HANDLER(WM_SETFOCUS, ForwardMessage )
    MESSAGE_HANDLER(WM_KILLFOCUS, ForwardMessage )
    MESSAGE_HANDLER(WM_ACTIVATE, ForwardMessage )
END_MSG_MAP()

// IViewObjectEx
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IDispatch
    CComTypeInfoHolder *_tih;

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
    {
	    *pctinfo = 1;
	    return S_OK;
    }
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
	    return _tih->GetTypeInfo(itinfo, lcid, pptinfo);
    }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
	    LCID lcid, DISPID* rgdispid)
    {
	    return _tih->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    }

    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr);

// IProvideClassInfo2
    CComTypeInfoHolder *_tih2;

    STDMETHOD(GetClassInfo)(ITypeInfo** pptinfo)
    {
	return _tih2->GetTypeInfo(0, LANG_NEUTRAL, pptinfo);
    }

    STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID)
    {
	if (pGUID == NULL)
	    return E_POINTER;
	
	if ( dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID )
	{
	    *pGUID = _Module.factory()->eventsID( QUuid( IID_QAxClass ) );
	    return S_OK;
	}
	*pGUID = GUID_NULL;
	return E_FAIL;
    }


// IOleControl
    STDMETHOD(OnAmbientPropertyChange)(DISPID);

// IOleObject
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(SetExtent)(DWORD dwAspect, SIZEL *pSz );

// IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**);
    STDMETHOD(FindConnectionPoint)(REFIID, IConnectionPoint**);

// IPersist
    STDMETHOD(GetClassID)(GUID*clsid) 
    {
	*clsid = IID_QAxClass;
	return S_OK;
    }

// IPersistPropertyBag
    STDMETHOD(InitNew)(VOID);
    STDMETHOD(Load)(IPropertyBag *, IErrorLog *);
    STDMETHOD(Save)(IPropertyBag *, BOOL, BOOL);

// IPersistStorage
    STDMETHOD(IsDirty)(VOID);

    bool qt_emit( int, QUObject* );
    void emitPropertyChanged( DISPID dispId );
    bool emitRequestPropertyChange( DISPID dispId );

    void readMetaData();

    static QPtrList<CComTypeInfoHolder> *typeInfoHolderList;
    QIntDict<QMetaData>* slotlist;
    QMap<int,DISPID>* signallist;
    QIntDict<QMetaProperty>* proplist;
    QMap<int, DISPID>* proplist2;

    bool eventFilter( QObject *o, QEvent *e );
private:
    QWidget* activeqt;
    ConnectionPoints points;
    bool initNewCalled;
    bool dirtyflag;
    unsigned long ref;

    const IID IID_QAxClass;

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT ForwardMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
};

#endif //QACTIVEQTBASE_H
