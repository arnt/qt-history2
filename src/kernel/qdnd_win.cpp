/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_win.cpp#9 $
**
** WM_FILES implementation for Qt.
**
** Created : 980320
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapplication.h"
#include "qwidget.h"
#include "qdragobject.h"
#include "qimage.h"
#include "qbuffer.h"
#include "qmessagebox.h"

#include <windows.h>
#include <ole2.h>          

static bool
mimeEqCf( const char* mime, CLIPFORMAT cf )
{
    if ( qstrncmp(mime,"text/",5)==0 )
	return cf == CF_TEXT || cf == CF_UNICODETEXT;
    if ( qstrncmp(mime,"image/",6)==0 )
	return cf == CF_DIB;
    // #### user-extension?
QMessageBox::information(0,"...","Bad mimeEqCf");
    return FALSE;
}

static bool
isAcceptableCf( CLIPFORMAT cf )
{
    return cf==CF_TEXT || cf==CF_UNICODETEXT || cf==CF_DIB;
}

static int
countAcceptableCf()
{
    return 3;
}

static CLIPFORMAT
acceptableCf( int i )
{
    static CLIPFORMAT ok[]={CF_TEXT,CF_UNICODETEXT,CF_DIB};
    if (i<3) return ok[i];
QMessageBox::critical(0,"...","Bad acceptableCf");
    return 0;
}

static CLIPFORMAT
cfFromMime(const char* mime)
{
    if ( qstrncmp(mime,"text/",5)==0 )
	return CF_TEXT; // CF_UNICODETEXT

    if ( qstrncmp(mime,"image/",6)==0 )
	return CF_DIB;

QMessageBox::critical(0,"cfFromMime",mime);
    return 0;
}

static QByteArray
cfFromMime(QByteArray data, const char* mime, CLIPFORMAT cf)
{
    QByteArray result;

    if ( qstrncmp(mime,"text/",5)==0 ) {
	if ( cf == CF_TEXT ) {
	    result = data;
	} else if ( cf == CF_UNICODETEXT ) {
	    result = data;
	}
    }

    if ( qstrcmp(mime,"image/bmp")==0 ) {
	result = data;
    } else if ( qstrncmp(mime,"image/",6)==0 ) {
	if ( cf == CF_DIB ) {
	    QImage img;  // Convert from BMP
	    img.loadFromData((const unsigned char*)data.data(),data.size());

	    if ( !img.isNull() ) {
		QStrList ofmts = QImage::outputFormats();
		for (const char* fmt=ofmts.first(); fmt; fmt=ofmts.next()) {
		    if ( qstricmp(fmt,mime+6)==0 ) {
			QByteArray ba;
			QBuffer iod(ba);
			iod.open(IO_WriteOnly);
			QImageIO iio(&iod, fmt);
			iio.setImage(img);
			if (iio.write()) {
			    result = ba;
			    break;
			}
		    }
		}
	    }
	}
    }

    return result;
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


class QOleDataObject : public IDataObject
{
public:
    QOleDataObject( QDragObject* );
   
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
    QDragObject* object;
};
   
   

class QOleDropTarget : public IDropTarget
{
public:    
    QWidget* widget;

    QOleDropTarget( QWidget* w );

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


void QDragManager::cancel()
{
    if ( object ) {
	beingCancelled = TRUE;
	if ( object->autoDelete() )
	    delete object;
	object = 0;
    }

    // insert cancel code here

    if ( restoreCursor ) {
	QApplication::restoreOverrideCursor();
	restoreCursor = FALSE;
    }
}


void QDragManager::move( const QPoint & globalPos )
{
    // tbi
}


void QDragManager::drop()
{
    // tbi
}



const char * QDragMoveEvent::format( int  )
{
    return 0;
}

static
QOleDropTarget *current_drop=0;
LPDATAOBJECT current_dropobj = 0;

QByteArray qt_olednd_obtain_data( const char *format )
{
    QByteArray tmp;

    FORMATETC fmtetc;
    STGMEDIUM medium;   
    HGLOBAL hText;
    HRESULT hr;

    fmtetc.cfFormat = cfFromMime(format);
    fmtetc.ptd = NULL;
    fmtetc.dwAspect = DVASPECT_CONTENT;  
    fmtetc.lindex = -1;
    fmtetc.tymed = TYMED_HGLOBAL;       
    
    // User has dropped on us. Get the data from drag source
    hr = current_dropobj->GetData(&fmtetc, &medium);
    if (!FAILED(hr)) {
	// Display the data and release it.
	hText = medium.hGlobal;
	void* d = GlobalLock(hText);
	int len = GlobalSize(medium.hGlobal);
	tmp.resize(len);
	memcpy(tmp.data(),d,len);
	GlobalUnlock(hText);
	ReleaseStgMedium(&medium);
    }
    return tmp;
}

QByteArray QDragMoveEvent::data( const char * format )
{
    return qt_olednd_obtain_data( format );
}

QByteArray QDropEvent::data( const char * format )
{
    return qt_olednd_obtain_data( format );
}


void QDragManager::startDrag( QDragObject * o )
{
    if ( object == o ) {
	debug( "meaningless" );
	return;
    }

    if ( object ) {
	cancel();
	dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    object = o;
    dragSource = (QWidget *)(object->parent());

    DWORD dwEffect;
    QOleDropSource *src = new QOleDropSource(dragSource);
    QOleDataObject *obj = new QOleDataObject(o);
    // This drag source only allows copying of data.
    // Move and link is not allowed.
    DoDragDrop(obj, src, DROPEFFECT_COPY, &dwEffect);     
    obj->Release();
}

void qt_olednd_unregister( QWidget* widget, QOleDropTarget *dst )
{
    RevokeDragDrop(widget->winId());
    dst->widget = 0;
    dst->Release();
    CoLockObjectExternal(dst, FALSE, TRUE);
}

QOleDropTarget* qt_olednd_register( QWidget* widget )
{
    QOleDropTarget* dst = new QOleDropTarget( widget );
    CoLockObjectExternal(dst, TRUE, TRUE);
    RegisterDragDrop(widget->winId(), dst);
    return dst;
}


/***********************************************************
 * Standard implementation of IEnumFormatEtc. This code
 * is from \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
 ***********************************************************/
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize);
STDAPI_(void) OleStdFree(LPVOID pmem);
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc);
STDAPI_(LPENUMFORMATETC)
  OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc);


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

QOleDataObject::QOleDataObject( QDragObject* o ) :
    object(o)
{
    m_refs = 1;    
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
    // This method is called by the drag-drop target to obtain the data
    // that is being dragged.

    pmedium->tymed = NULL;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = NULL;

    for (QDragObject* obj = object; obj; obj = obj->alternative()) {
	if (mimeEqCf(obj->format(),pformatetc->cfFormat) &&
	   pformatetc->dwAspect == DVASPECT_CONTENT &&
	   pformatetc->tymed == TYMED_HGLOBAL)
	{
	    QByteArray data =
		cfFromMime(obj->encodedData(),
			obj->format(), pformatetc->cfFormat);
	    if ( data.size() ) {
		HGLOBAL hData = GlobalAlloc(GMEM_SHARE, data.size());
		if (!hData)
		    return ResultFromScode(E_OUTOFMEMORY);
		void* out = GlobalLock(hData);
		memcpy(out,data.data(),data.size());
		GlobalUnlock(hData);
		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->hGlobal = hData; 
		return ResultFromScode(S_OK);
	    }
QMessageBox::critical(0,"EEEE","Conversion failed late");
	}
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
    if (isAcceptableCf(pformatetc->cfFormat)
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
    int n = countAcceptableCf();
    LPFORMATETC fmtetc = new FORMATETC[n];

    for (int i=0; i<n; i++) {
	fmtetc[i].cfFormat = acceptableCf(i);
	fmtetc[i].dwAspect = DVASPECT_CONTENT;
	fmtetc[i].tymed = TYMED_HGLOBAL;
	fmtetc[i].ptd = NULL;
	fmtetc[i].lindex = -1;
    }

    if (dwDirection == DATADIR_GET){
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(n, fmtetc);
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

    delete fmtetc;
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


         
          
         
QOleDropTarget::QOleDropTarget( QWidget* w ) :
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
    int n = countAcceptableCf();
    m_bAcceptFmt = FALSE;
    for (int i=0; i<n && !m_bAcceptFmt; i++) {
	FORMATETC fmtetc;
	fmtetc.cfFormat = acceptableCf(i);
	fmtetc.ptd      = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;  
	fmtetc.lindex   = -1;
	fmtetc.tymed    = TYMED_HGLOBAL; 
	// Does the drag source provide this format that we accept?
	m_bAcceptFmt = (NOERROR == pDataObj->QueryGetData(&fmtetc))
			    ? TRUE : FALSE;    
    }
if (!m_bAcceptFmt)
QMessageBox::information(0,"...","Bad fmt");
    
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
    if (QueryDrop(grfKeyState, pdwEffect))
    {      
	current_dropobj = pDataObj;
	current_drop = this; // ##### YUCK.  Arnt, we need to put info in event
	QDropEvent de( QPoint(pt.x,pt.y) );
	QApplication::sendEvent( widget, &de );
	current_drop = 0;
	current_dropobj = 0;
	return NOERROR;      
    }
    
    *pdwEffect = DROPEFFECT_NONE;
    return ResultFromScode(DATA_E_FORMATETC);
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
