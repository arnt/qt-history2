/****************************************************************************
** $Id: $
**
** Declaration of the QAxServerBase class
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
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

#ifndef QAXSERVERBASE_H
#define QAXSERVERBASE_H

#include <qmetaobject.h>
#include <qmap.h>
#include <quuid.h>
#include <qobject.h>
#include <qintdict.h>
#include "qaxfactory.h"

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

    static QAxFactoryInterface *factory();
    static QInterfacePtr<QAxFactoryInterface> _factory;
};

extern CExeModule _Module;
#include <atlcom.h>
#include <atlctl.h>

extern GUID IID_IAxServerBase;

struct IAxServerBase : public IUnknown
{
    virtual QObject *qObject() = 0;
    virtual QWidget *widget() = 0;
};

class QWidget;
class QAxPropertyPage;

/////////////////////////////////////////////////////////////////////////////
// QAxServerBase
class QAxServerBase : 
    public QObject,
    public IAxServerBase,
    public IDispatch,
    public CWindowImpl<QAxServerBase>,

    public IOleObject,
    public IOleControl,
#ifdef QAX_VIEWOBJECTEX
    public IViewObjectEx,
#else
    public IViewObject2,
#endif
    public IOleInPlaceObject,
    public IProvideClassInfo2,
    public IConnectionPointContainer,
    public IPersistPropertyBag,
    public ISpecifyPropertyPages,
    public IPropertyPage2
{
public:
    typedef QMap<QUuid,IConnectionPoint*> ConnectionPoints;
    typedef QMap<QUuid,IConnectionPoint*>::Iterator ConnectionPointsIterator;

    QAxServerBase( const QString &classname );

    ~QAxServerBase();

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
    HRESULT WINAPI QueryInterface( REFIID iid, void **iface );

    BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);

// IAxServerBase
    QObject *qObject()
    {
        return this;
    }
    QWidget *widget()
    {
	return activeqt;
    }

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
	
	if ( dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID ) {
	    *pGUID = _Module.factory()->eventsID( class_name );
	    return S_OK;
	}
	*pGUID = GUID_NULL;
	return E_FAIL;
    }

// IOleObject
    STDMETHOD(Advise)( IAdviseSink* pAdvSink, DWORD* pdwConnection );
    STDMETHOD(Close)( DWORD dwSaveOption );
    STDMETHOD(DoVerb)( LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect );
    STDMETHOD(EnumAdvise)( IEnumSTATDATA** ppenumAdvise );
    STDMETHOD(EnumVerbs)( IEnumOLEVERB** ppEnumOleVerb );
    STDMETHOD(GetClientSite)( IOleClientSite** ppClientSite );
    STDMETHOD(GetClipboardData)( DWORD dwReserved, IDataObject** ppDataObject );
    STDMETHOD(GetExtent)( DWORD dwDrawAspect, SIZEL* psizel );
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(GetMoniker)( DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk );
    STDMETHOD(GetUserClassID)( CLSID* pClsid );
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(InitFromData)( IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved );
    STDMETHOD(IsUpToDate)();
    STDMETHOD(SetClientSite)( IOleClientSite* pClientSite );
    STDMETHOD(SetColorScheme)( LOGPALETTE* pLogPal );
    STDMETHOD(SetExtent)( DWORD dwDrawAspect, SIZEL* psizel );
    STDMETHOD(SetHostNames)( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj );
    STDMETHOD(SetMoniker)( DWORD dwWhichMoniker, IMoniker* ppmk );
    STDMETHOD(Unadvise)( DWORD dwConnection );
    STDMETHOD(Update)();

// IViewObject
    STDMETHOD(Draw)( DWORD dwAspect, LONG lIndex, void *pvAspect, DVTARGETDEVICE *ptd, 
		    HDC hicTargetDevice, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
		    BOOL(__stdcall*pfnContinue)(DWORD), DWORD dwContinue );
    STDMETHOD(GetColorSet)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		    HDC hicTargetDev, LOGPALETTE **ppColorSet );
    STDMETHOD(Freeze)( DWORD dwAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze );
    STDMETHOD(Unfreeze)( DWORD dwFreeze );
    STDMETHOD(SetAdvise)( DWORD aspects, DWORD advf, IAdviseSink *pAdvSink );
    STDMETHOD(GetAdvise)( DWORD *aspects, DWORD *advf, IAdviseSink **pAdvSink );

// IViewObject2
    STDMETHOD(GetExtent)( DWORD dwAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL lpsizel );

#ifdef QAX_VIEWOBJECTEX
    // IViewObjectEx
    STDMETHOD(GetRect)( DWORD dwAspect, LPRECTL pRect );
    STDMETHOD(GetViewStatus)( DWORD *pdwStatus );
    STDMETHOD(QueryHitPoint)( DWORD dwAspect, LPCRECT pRectBounds, POINT ptlLoc, LONG lCloseHint, DWORD *pHitResult );
    STDMETHOD(QueryHitRect)( DWORD dwAspect, LPCRECT pRectBounds, LPCRECT prcLoc, LONG lCloseHint, DWORD *pHitResult );
    STDMETHOD(GetNaturalExtent)( DWORD dwAspect, LONG lindex, DVTARGETDEVICE *ptd, HDC hicTargetDev, DVEXTENTINFO *pExtentInfo, LPSIZEL pSizel );
    DECLARE_VIEW_STATUS()
#endif

// IOleControl
    STDMETHOD(FreezeEvents)(BOOL);
    STDMETHOD(GetControlInfo)(LPCONTROLINFO);
    STDMETHOD(OnAmbientPropertyChange)(DISPID);
    STDMETHOD(OnMnemonic)(LPMSG);

// IOleWindow
    STDMETHOD(GetWindow)(HWND *pHwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

// IOleInPlaceObject
    STDMETHOD(InPlaceDeactivate)();
    STDMETHOD(UIDeactivate)();
    STDMETHOD(SetObjectRects)(LPCRECT lprcPosRect, LPCRECT lprcClipRect);
    STDMETHOD(ReactivateAndUndo)();


// IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**);
    STDMETHOD(FindConnectionPoint)(REFIID, IConnectionPoint**);

// IPersist
    STDMETHOD(GetClassID)(GUID*clsid) 
    {
	*clsid = _Module.factory()->classID( class_name );
	return S_OK;
    }

// IPersistPropertyBag
    STDMETHOD(InitNew)(VOID);
    STDMETHOD(Load)(IPropertyBag *, IErrorLog *);
    STDMETHOD(Save)(IPropertyBag *, BOOL, BOOL);

// IPersistStorage
    STDMETHOD(IsDirty)(VOID);

// ISpecifyPropertyPages
    STDMETHOD(GetPages)( CAUUID *pPages );

// IPropertyPage
    STDMETHOD(SetPageSite)( IPropertyPageSite *pPageSite );
    STDMETHOD(Activate)( HWND hWndParent, LPCRECT pRect, BOOL bModal );
    STDMETHOD(Deactivate)();
    STDMETHOD(GetPageInfo)( PROPPAGEINFO *pPageInfo );
    STDMETHOD(SetObjects)( ULONG cObjects, IUnknown **ppUnk );
    STDMETHOD(Show)( UINT nCmdShow );
    STDMETHOD(Move)( LPCRECT pRect );
    STDMETHOD(IsPageDirty)();
    STDMETHOD(Apply)();
    STDMETHOD(Help)( LPCOLESTR pszHelpDir );
    STDMETHOD(TranslateAccelerator)( MSG *pMsg );

// IPropertyPage2
    STDMETHOD(EditProperty)( DISPID prop );

/* IPersistStorage
    STDMETHOD(InitNew)(IStorage *pStg ) { return E_NOTIMPL; }
    STDMETHOD(Load)(IStorage *pStg ) { return E_NOTIMPL; }
    STDMETHOD(Save)(IStorage *pStg, BOOL fSameAsLoad ) { return E_NOTIMPL; }
    STDMETHOD(SaveCompleted)( IStorage *pStgNew ) { return E_NOTIMPL; }
    STDMETHOD(HandsOffStorage)() { return E_NOTIMPL; }
*/

    bool qt_emit( int, QUObject* );
    void emitPropertyChanged( DISPID dispId );
    bool emitRequestPropertyChange( DISPID dispId );

    void readMetaData();

    static QPtrList<CComTypeInfoHolder> *typeInfoHolderList;

    bool eventFilter( QObject *o, QEvent *e );
private:
    void update();
    void updateGeometry();    
    bool internalCreate();
    HRESULT internalActivate();

    friend class QAxBindable;
    friend class QAxPropertyPage;

    QWidget* activeqt;
    ConnectionPoints points;

    unsigned initNewCalled	:1;
    unsigned dirtyflag		:1;
    unsigned hasStockEvents	:1;
    unsigned m_bWindowOnly	:1;
    unsigned m_bAutoSize	:1;
    unsigned m_bInPlaceActive	:1;
    unsigned m_bUIActive	:1;
    unsigned m_bWndLess		:1;
    unsigned m_bInPlaceSiteEx	:1;
    unsigned m_bWasOnceWindowless:1;
    unsigned m_bRequiresSave	:1;
    unsigned m_bNegotiatedWnd	:1;
    short m_nFreezeEvents;

    union {
	HWND& m_hWndCD;
	HWND* m_phWndCD;
    };

    SIZE m_sizeExtent;
    SIZE m_sizeNatural;
    RECT m_rcPos;
    unsigned long ref;

    QString class_name;

    QIntDict<QMetaData>* slotlist;
    QMap<int,DISPID>* signallist;
    QIntDict<QMetaProperty>* proplist;
    QMap<int, DISPID>* proplist2;

    CComPtr<IAdviseSink> m_spAdviseSink;
    CComPtr<IOleAdviseHolder> m_spOleAdviseHolder;
    CComDispatchDriver m_spAmbientDispatch;
    CComPtr<IOleClientSite> m_spClientSite;
    CComPtr<IOleInPlaceSiteWindowless> m_spInPlaceSite;    

    IPropertyPageSite *propPageSite;
    QAxPropertyPage *propPage;
    QPtrList<IAxServerBase> propObjects;
};

#endif //QAXSERVERBASE_H
