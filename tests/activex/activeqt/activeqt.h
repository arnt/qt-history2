#ifndef QACTIVEQTBASE_H
#define QACTIVEQTBASE_H

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
};
extern CExeModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <atlhost.h>

#include <qmap.h>
#include <quuid.h>
#include <qobject.h>
#include "activeiface.h"

class QActiveQt;

/////////////////////////////////////////////////////////////////////////////
// QActiveQtBase
class QActiveQtBase : 
    public QObject,
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IQActiveX, &IID_IQActiveX, &LIBID_ACTIVEQTEXELib >,
    public CComControl<QActiveQtBase>,
    public IOleControlImpl<QActiveQtBase>,
    public IOleObjectImpl<QActiveQtBase>,
    public IOleInPlaceActiveObjectImpl<QActiveQtBase>,
    public IViewObjectExImpl<QActiveQtBase>,
    public IOleInPlaceObjectWindowlessImpl<QActiveQtBase>,
    public IQuickActivateImpl<QActiveQtBase>,
    public IProvideClassInfo2Impl<&CLSID_QActiveX, &DIID__IQActiveXEvents, &LIBID_ACTIVEQTEXELib>,
    public CComCoClass<QActiveQtBase, &CLSID_QActiveX>,
    public IConnectionPointContainer,
    public IPersistPropertyBag
{
public:
    typedef QMap<QUuid,IConnectionPoint*> ConnectionPoints;
    typedef QMap<QUuid,IConnectionPoint*>::Iterator ConnectionPointsIterator;

    QActiveQtBase();

    virtual ~QActiveQtBase();

    static HRESULT WINAPI UpdateRegistry(BOOL bRegister);

BEGIN_COM_MAP(QActiveQtBase)
    {&IID_IQActiveX,
	offsetofclass(IQActiveX, _ComMapClass),
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
END_COM_MAP()

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
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr);

// IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**);
    STDMETHOD(FindConnectionPoint)(REFIID, IConnectionPoint**);

// IPersist
    STDMETHOD(GetClassID)(GUID*clsid) 
    {
	*clsid = CLSID_QActiveX;
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

private:
    QActiveQt* activeqt;
    ConnectionPoints points;
    bool initNewCalled;
    bool dirtyflag;

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT ForwardMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
};

#endif //QACTIVEQTBASE_H
