#include <qapplication.h>
#include <qlabel.h>

#include <windows.h>
#include <ole2.h>          
#include "enumfetc.h"          

static void
initOleDnd( QWidget* w )
{
}

class QOleDropSource : public IDropSource
{
    QWidget* src;
public:
    QOleDropSource( QWidget* w ) :
	src(w)
    {
    }

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    /* IDropSource methods */
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);
 
private:
    ULONG m_refs;     
};  

#define BUF_LEN 1024

class QOleDataObject : public IDataObject
{
public:
    QOleDataObject( const char* text );
   
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    /* IDataObject methods */    
    STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
    STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
    STDMETHOD(QueryGetData)(LPFORMATETC pformatetc );
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
    STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                       BOOL fRelease);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf, 
                      LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);
    
private:
    ULONG m_refs;   
    QString data;
};
   
   

class OleDndSource : public QLabel {
    QOleDropSource *src;
public:
    OleDndSource( QWidget* parent ) :
	QLabel("SOURCE",parent)
    {
	src = new QOleDropSource(this);
    }

    ~OleDndSource()
    {
        src->Release();
    }

    void mousePressEvent(QMouseEvent* e)
    {
        DWORD dwEffect;
        QOleDataObject *obj = new QOleDataObject(text());
        // This drag source only allows copying of data.
	// Move and link is not allowed.
        DoDragDrop(obj, src, DROPEFFECT_COPY, &dwEffect);     
        obj->Release();
    }
};

class QOleDropTarget : public IDropTarget
{
    QLabel* widget;
public:    
    QOleDropTarget( QLabel* w );

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDropTarget methods */
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect); 
    
    /* Utility function to read type of drag from key state*/
    STDMETHOD_(BOOL, QueryDrop)(DWORD grfKeyState, LPDWORD pdwEffect);
 
private:
    ULONG m_refs;  
    BOOL m_bAcceptFmt;
};

class OleDndDestination : public QLabel {
    QOleDropTarget *dst;
public:
    OleDndDestination( QWidget* parent ) :
	QLabel("DROP HERE",parent)
    {
	dst = new QOleDropTarget( this );
        // The IDropTarget interface is marshaled by OLE using MSHLFLAGS_TABLEWEAK.
        // (See the MSHLFLAGS enumeration documentation for details). This means 
        // that OLE does not keep a strong lock on IDropTarget. This will result
        // in the interface being released after the first drop. To prevent this
        // from happening we keep a strong lock on IDropTarget by calling 
        // CoLockObjectExternal.           
        CoLockObjectExternal(dst, TRUE, TRUE);
        RegisterDragDrop(winId(), dst);    
    }

    ~OleDndDestination()
    {
        // Revoke the window as a drop target and release the strong lock
        // using CoLockObjectExternal.          
        RevokeDragDrop(winId());
        dst->Release();  
        CoLockObjectExternal(dst, FALSE, TRUE);
    }
};

class Main : public QWidget {
    OleDndSource src;
    OleDndDestination dst;
public:
    Main() :
        src(this),
	dst(this)
    {
	resize(100,100);
    }

    void resizeEvent(QResizeEvent*) 
    {
	src.setGeometry(0,0,width(),height()/2);
	dst.setGeometry(0,height()/2,width(),height()/2);
    }
};

main(int argc, char**argv)
{
    QApplication app(argc,argv);

    if (NOERROR != OleInitialize(NULL))
       return 1;

    Main m;
    app.setMainWidget(&m);
    m.show();

    int r = app.exec();

    OleUninitialize();

    return r;
}


//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDropSource::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDropSource)
    {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


ULONG
QOleDropSource::AddRef(void)
{
    return ++m_refs;
}


ULONG
QOleDropSource::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}  

//---------------------------------------------------------------------
//                    IDropSource Methods
//---------------------------------------------------------------------  

STDMETHODIMP
QOleDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{  
     if (fEscapePressed)
        return ResultFromScode(DRAGDROP_S_CANCEL);
    else if (!(grfKeyState & MK_LBUTTON))
        return ResultFromScode(DRAGDROP_S_DROP);
    else
        return NOERROR;                  
}

STDMETHODIMP
QOleDropSource::GiveFeedback(DWORD dwEffect)
{
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}


//---------------------------------------------------------------------
//                    QOleDataObject Constructor
//---------------------------------------------------------------------        

QOleDataObject::QOleDataObject( const char* text )
{
    m_refs = 1;    
    data = text;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDataObject)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


ULONG
QOleDataObject::AddRef(void)
{
    return ++m_refs;
}


ULONG
QOleDataObject::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}  

//---------------------------------------------------------------------
//                    IDataObject Methods    
//  
// The following methods are NOT supported for data transfer using the
// clipboard or drag-drop: 
//
//      IDataObject::SetData    -- return E_NOTIMPL
//      IDataObject::DAdvise    -- return OLE_E_ADVISENOTSUPPORTED
//                 ::DUnadvise
//                 ::EnumDAdvise
//      IDataObject::GetCanonicalFormatEtc -- return E_NOTIMPL
//                     (NOTE: must set pformatetcOut->ptd = NULL)
//---------------------------------------------------------------------  

STDMETHODIMP 
QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium) 
{   
    HGLOBAL hText; 
    LPSTR pszText;
    
    pmedium->tymed = NULL;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = NULL;
    
    // This method is called by the drag-drop target to obtain the text
    // that is being dragged.
    if (pformatetc->cfFormat == CF_TEXT &&
       pformatetc->dwAspect == DVASPECT_CONTENT &&
       pformatetc->tymed == TYMED_HGLOBAL)
    {
        hText = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, data.length()+1);    
        if (!hText)
            return ResultFromScode(E_OUTOFMEMORY);
        pszText = (LPSTR)GlobalLock(hText);
        lstrcpy(pszText, data.data());
        GlobalUnlock(hText);
        
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hText; 
 
        return ResultFromScode(S_OK);
    }
    return ResultFromScode(DATA_E_FORMATETC);
}
   
STDMETHODIMP 
QOleDataObject::GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)  
{
    return ResultFromScode(DATA_E_FORMATETC);    
}     

STDMETHODIMP 
QOleDataObject::QueryGetData(LPFORMATETC pformatetc) 
{   
    // This method is called by the drop target to check whether the source
    // provides data is a format that the target accepts.
    if (pformatetc->cfFormat == CF_TEXT 
        && pformatetc->dwAspect == DVASPECT_CONTENT
        && pformatetc->tymed & TYMED_HGLOBAL)
        return ResultFromScode(S_OK); 
    else return ResultFromScode(S_FALSE);
}

STDMETHODIMP 
QOleDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{ 
    pformatetcOut->ptd = NULL; 
    return ResultFromScode(E_NOTIMPL);
}        

STDMETHODIMP 
QOleDataObject::SetData(LPFORMATETC pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{   
    // A data transfer object that is used to transfer data
    //    (either via the clipboard or drag/drop does NOT
    //    accept SetData on ANY format.
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP 
QOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{ 
    // A standard implementation is provided by OleStdEnumFmtEtc_Create
    // which can be found in \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
    // This code from ole2ui is copied to the enumfetc.c file in this sample.
    
    SCODE sc = S_OK;
    FORMATETC fmtetc;
    
    fmtetc.cfFormat = CF_TEXT;
    fmtetc.dwAspect = DVASPECT_CONTENT;
    fmtetc.tymed = TYMED_HGLOBAL;
    fmtetc.ptd = NULL;
    fmtetc.lindex = -1;

    if (dwDirection == DATADIR_GET){
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(1, &fmtetc);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;

    } else if (dwDirection == DATADIR_SET){
        // A data transfer object that is used to transfer data
        //    (either via the clipboard or drag/drop does NOT
        //    accept SetData on ANY format.
        sc = E_NOTIMPL;
        goto error;
    } else {
        sc = E_INVALIDARG;
        goto error;
    }

error:
    return ResultFromScode(sc);
}

STDMETHODIMP 
QOleDataObject::DAdvise(FORMATETC FAR* pFormatetc, DWORD advf, 
                       LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
   

STDMETHODIMP 
QOleDataObject::DUnadvise(DWORD dwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP 
QOleDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


         
          
         
QOleDropTarget::QOleDropTarget( QLabel* w ) :
    widget(w)
{
   m_refs = 1; 
   m_bAcceptFmt = FALSE;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDropTarget)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
QOleDropTarget::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
QOleDropTarget::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}  

//---------------------------------------------------------------------
//                    IDropTarget Methods
//---------------------------------------------------------------------  

STDMETHODIMP
QOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{  
    FORMATETC fmtetc;
       
    fmtetc.cfFormat = CF_TEXT;
    fmtetc.ptd      = NULL;
    fmtetc.dwAspect = DVASPECT_CONTENT;  
    fmtetc.lindex   = -1;
    fmtetc.tymed    = TYMED_HGLOBAL; 
    
    // Does the drag source provide CF_TEXT, which is the only format we accept.    
    m_bAcceptFmt = (NOERROR == pDataObj->QueryGetData(&fmtetc)) ? TRUE : FALSE;    
    
    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragLeave()
{   
    m_bAcceptFmt = FALSE;   
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)  
{   
    FORMATETC fmtetc;   
    STGMEDIUM medium;   
    HGLOBAL hText;
    LPSTR pszText;
    HRESULT hr;
     
    if (QueryDrop(grfKeyState, pdwEffect))
    {      
        fmtetc.cfFormat = CF_TEXT;
        fmtetc.ptd = NULL;
        fmtetc.dwAspect = DVASPECT_CONTENT;  
        fmtetc.lindex = -1;
        fmtetc.tymed = TYMED_HGLOBAL;       
        
        // User has dropped on us. Get the CF_TEXT data from drag source
        hr = pDataObj->GetData(&fmtetc, &medium);
        if (FAILED(hr))
            goto error; 
        
        // Display the data and release it.
        hText = medium.hGlobal;
        pszText = (LPSTR)GlobalLock(hText);
	widget->setText(pszText);
        GlobalUnlock(hText);
        ReleaseStgMedium(&medium);
    }
    return NOERROR;      
    
error:
    *pdwEffect = DROPEFFECT_NONE;
    return hr; 
}   

/* OleStdGetDropEffect
** -------------------
**
** Convert a keyboard state into a DROPEFFECT.
**
** returns the DROPEFFECT value derived from the key state.
**    the following is the standard interpretation:
**          no modifier -- Default Drop     (0 is returned)
**          CTRL        -- DROPEFFECT_COPY
**          SHIFT       -- DROPEFFECT_MOVE
**          CTRL-SHIFT  -- DROPEFFECT_LINK
**
**    Default Drop: this depends on the type of the target application.
**    this is re-interpretable by each target application. a typical
**    interpretation is if the drag is local to the same document
**    (which is source of the drag) then a MOVE operation is
**    performed. if the drag is not local, then a COPY operation is
**    performed.
*/
#define OleStdGetDropEffect(grfKeyState)    \
    ( (grfKeyState & MK_CONTROL) ?          \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) :  \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 ) )

//---------------------------------------------------------------------
// QOleDropTarget::QueryDrop: Given key state, determines the type of 
// acceptable drag and returns the a dwEffect. 
//---------------------------------------------------------------------   
STDMETHODIMP_(BOOL)
QOleDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{  
    DWORD dwOKEffects = *pdwEffect; 
    
    if (!m_bAcceptFmt)
        goto dropeffect_none; 
     
    *pdwEffect = OleStdGetDropEffect(grfKeyState);
    if (*pdwEffect == 0) {
        // No modifier keys used by user while dragging. Try in order: MOVE, COPY.
        if (DROPEFFECT_MOVE & dwOKEffects)
            *pdwEffect = DROPEFFECT_MOVE;
        else if (DROPEFFECT_COPY & dwOKEffects)
            *pdwEffect = DROPEFFECT_COPY; 
        else goto dropeffect_none;   
    } 
    else {
        // Check if the drag source application allows the drop effect desired by user.
        // The drag source specifies this in DoDragDrop
        if (!(*pdwEffect & dwOKEffects))
            goto dropeffect_none; 
        // We don't accept links
        if (*pdwEffect == DROPEFFECT_LINK)
            goto dropeffect_none; 
    }  
    return TRUE;

dropeffect_none:
    *pdwEffect = DROPEFFECT_NONE;
    return FALSE;
}   
    
                                                                             
                                                                           
