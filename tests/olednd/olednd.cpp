#include <qapplication.h>
#include <qwidget,h>

#include <windows.h>
#include <ole2.h>          
#include "enumfetc.h"          

static void
initOleDnd( QWidget* w )
{
}

class QOleDropSource : public IDropSource
{
public:    
    QOleDropSource();

    /* IUnknown methods */
    void QueryInterface(REFIID riid, void ** ppvObj);
    ULONG AddRef();
    ULONG Release();

    /* IDropSource methods */
    void QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    void GiveFeedback(DWORD dwEffect);
 
private:
    ULONG m_refs;     
};  

#define BUF_LEN 1024

class QOleDataObject : public IDataObject
{
public:
    QOleDataObject( const char* text );
   
   /* IUnknown methods */
    HRESULT QueryInterface(REFIID riid, void FAR* FAR* ppvObj);
    ULONG AddRef();
    ULONG Release();

    /* IDataObject methods */    
    HRESULT GetData(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
    HRESULT GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
    HRESULT QueryGetData(LPFORMATETC pformatetc );
    HRESULT GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
    HRESULT SetData(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                       BOOL fRelease);
    HRESULT EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
    HRESULT DAdvise(FORMATETC FAR* pFormatetc, DWORD advf, 
                       LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
    HRESULT DUnadvise(DWORD dwConnection);
    HRESULT EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise);
    
private:
    ULONG m_refs;   
    QString data;
};
   
   

class QOleDropSource : public IDropSource {
    QWidget* src;
public:
    QOleDropSource( QWidget* w ) :
	src(w)
    {
    }


};

static void
startDrag( QWidget* w, QPoint pos, const char* text )
{
    QOleDataObject *obj = new QOleDataObject(text);
    QOleDropSource *src = new QIDataSource(w);
    DWORD effects = DROPEFFECT_COPY /* Need Qt API to allow this | DROPEFFECT_MOVE */;
    DWORD results;
    DoDragDrop(obj, src, effects, &results);
}

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

class Main : public QWidget {
    OleDndSource src;
    OleDndDestination dst;
public:
    Main() :
        src(this),
	dst(this)
    {
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

    int r = m.exec();

    OleUninitialize();

    return r;
}


//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


HRESULT
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

HRESULT
QOleDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{  
     if (fEscapePressed)
        return ResultFromScode(DRAGDROP_S_CANCEL);
    else if (!(grfKeyState & MK_LBUTTON))
        return ResultFromScode(DRAGDROP_S_DROP);
    else
        return NOERROR;                  
}

HRESULT
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

HRESULT 
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
        hText = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, sizeof(m_szBuffer)+1);    
        if (!hText)
            return ResultFromScode(E_OUTOFMEMORY);
        pszText = (LPSTR)GlobalLock(hText);
        lstrcpy(pszText, m_szBuffer);
        GlobalUnlock(hText);
        
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hText; 
 
        return ResultFromScode(S_OK);
    }
    return ResultFromScode(DATA_E_FORMATETC);
}
XX
   
HRESULT 
QOleDataObject::GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)  
{
    return ResultFromScode(DATA_E_FORMATETC);    
}     

HRESULT 
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

HRESULT 
QOleDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{ 
    pformatetcOut->ptd = NULL; 
    return ResultFromScode(E_NOTIMPL);
}        

HRESULT 
QOleDataObject::SetData(LPFORMATETC pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{   
    // A data transfer object that is used to transfer data
    //    (either via the clipboard or drag/drop does NOT
    //    accept SetData on ANY format.
    return ResultFromScode(E_NOTIMPL);
}


HRESULT 
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

HRESULT 
QOleDataObject::DAdvise(FORMATETC FAR* pFormatetc, DWORD advf, 
                       LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
   

HRESULT 
QOleDataObject::DUnadvise(DWORD dwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

HRESULT 
QOleDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
