/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_win.cpp#18 $
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
#include "qdatastream.h"
#include "qmessagebox.h"

#include <windows.h>
#include <ole2.h>          

extern bool qt_read_dib( QDataStream&, QImage& ); // qimage.cpp
extern bool qt_write_dib( QDataStream&, QImage );   // qimage.cpp

/*
  Encapsulation of conversion between MIME and Windows CLIPFORMAT.
  This API will be exposed to Windows users when it matures.
  We might need to use FORMATETC objects rather than simple CLIPFORMAT.
*/
class QWindowsMime {
public:
    QWindowsMime();
    virtual ~QWindowsMime();

    static QWindowsMime* convertor( const char* mime, int cf );
    static const char* cfToMime(int cf);

    virtual const char* convertorName()=0;
    virtual int countCf()=0;
    virtual int cf(int index)=0;
    virtual bool canConvert( const char* mime, int cf )=0;
    virtual const char* mimeFor(int cf)=0;
    virtual int cfFor(const char*)=0;
    virtual QByteArray convertToMime( QByteArray data, const char* mime, int cf )=0;
    virtual QByteArray convertFromMime( QByteArray data, const char* mime, int cf )=0;
};




static QList<QWindowsMime> mimes;

QWindowsMime::QWindowsMime()
{
    mimes.append(this);
}

QWindowsMime::~QWindowsMime()
{
    mimes.remove(this);
}


LPFORMATETC allFormats(int& n)
{
    n = 0;
    QWindowsMime* wm;
    for ( wm = mimes.first(); wm; wm = mimes.next() )
	n += wm->countCf();

    LPFORMATETC fmtetc = new FORMATETC[n];

    int i = 0;
    for ( wm = mimes.first(); wm; wm = mimes.next() ) {
	int t = wm->countCf();
	for (int j=0; j<t; j++) {
	    fmtetc[i].cfFormat = wm->cf(j);
	    fmtetc[i].dwAspect = DVASPECT_CONTENT;
	    fmtetc[i].tymed = TYMED_HGLOBAL;
	    fmtetc[i].ptd = NULL;
	    fmtetc[i].lindex = -1;
	    i++;
	}
    }

    ASSERT(n==i);
    return fmtetc;
}


LPFORMATETC someFormats(const char* mime, int& n)
{
    n = 0;
    QWindowsMime* wm;
    for ( wm = mimes.first(); wm; wm = mimes.next() )
	if (wm->cfFor(mime)) n += wm->countCf();

    LPFORMATETC fmtetc = new FORMATETC[n];

    int i = 0;
    for ( wm = mimes.first(); wm; wm = mimes.next() ) {
	if (wm->cfFor(mime)) {
	    int t = wm->countCf();
	    for (int j=0; j<t; j++) {
		fmtetc[i].cfFormat = wm->cf(j);
		fmtetc[i].dwAspect = DVASPECT_CONTENT;
		fmtetc[i].tymed = TYMED_HGLOBAL;
		fmtetc[i].ptd = NULL;
		fmtetc[i].lindex = -1;
		i++;
	    }
	}
    }

    ASSERT(n==i);
    return fmtetc;
}


struct QWindowsRegisteredMimeType {
    QWindowsRegisteredMimeType(int c, const char* m) :
	cf(c), mime(m) {}
    int cf;
    QString mime;
};

static QList<QWindowsRegisteredMimeType> mimetypes;

static
CLIPFORMAT registerMimeType(const char* mime)
{
    CLIPFORMAT f = RegisterClipboardFormat(mime);
    QWindowsRegisteredMimeType *mt = mimetypes.current();
    if ( !mt || mt->cf != f ) {
	for ( mt = mimetypes.first(); mt && mt->cf!=f; mt = mimetypes.next() )
	    ;
	if (!mt) {
	    mimetypes.append(new QWindowsRegisteredMimeType(f,mime));
	}
    }
    return f;
}
   

class QWindowsMimeAnyMime : public QWindowsMime {
public:
    int countCf()
    {
	return mimetypes.count();
    }

    const char* convertorName()
    {
	return "Any-Mime";
    }

    int cf(int index)
    {
	return mimetypes.at(index)->cf;
    }

    int cfFor(const char* mime)
    {
	QWindowsRegisteredMimeType *mt = mimetypes.current();
	if ( mt ) // quick check with most-recent
	    if ( 0==qstricmp(mt->mime, mime) )
		return mt->cf;
	for ( mt = mimetypes.first(); mt; mt = mimetypes.next() )
	    if ( 0==qstricmp(mt->mime, mime) )
		return mt->cf;
	return 0;
    }

    const char* mimeFor(int cf)
    {
	QWindowsRegisteredMimeType *mt = mimetypes.current();
	if ( mt ) // quick check with most-recent
	    if ( mt->cf == cf )
		return mt->mime;
	for ( mt = mimetypes.first(); mt; mt = mimetypes.next() )
	    if ( mt->cf == cf )
		return mt->mime;
	return 0;
    }

    bool canConvert( const char* mime, int cf )
    {
	QWindowsRegisteredMimeType *mt = mimetypes.current();
	if ( mt ) // quick check with most-recent
	    if ( mt->cf == cf && 0==stricmp(mt->mime,mime) ) {
		return TRUE;
	    }
	for ( mt = mimetypes.first(); mt; mt = mimetypes.next() )
	    if ( mt->cf == cf )
		break;
	if ( !mt ) {
	    registerMimeType(mime);
	    mt = mimetypes.current();
	    if ( !mt || mt->cf != cf ) {
		return FALSE;
	    }
	}

	return 0==stricmp(mt->mime,mime);
    }

    QByteArray convertToMime( QByteArray data, const char*, int )
    {
	return data;
    }

    QByteArray convertFromMime( QByteArray data, const char*, int )
    {
	return data;
    }
};

class QWindowsMimeText : public QWindowsMime {
public:
    int countCf()
    {
	return 1 /* 2 with unicode */;
    }

    const char* convertorName()
    {
	return "Text";
    }

    int cf(int index)
    {
	return CF_TEXT;
	// return index ? CF_TEXT : CF_UNICODETEXT;  // Note UNICODE first.
    }

    int cfFor(const char* mime)
    {
	if ( qstricmp( mime, "text/plain" ) )
	    return CF_TEXT;
	else
	    return 0;
    }

    const char* mimeFor(int cf)
    {
	if ( cf == CF_TEXT )
	    return "text/plain";
	else
	    return 0;
    }

    bool canConvert( const char* mime, int cf )
    {
	return cf == CF_TEXT && qstrncmp(mime,"text/",5)==0;
    }

    QByteArray convertToMime( QByteArray data, const char*, int )
    {
	return data;
    }

    QByteArray convertFromMime( QByteArray data, const char*, int )
    {
	return data;
    }
};

class QWindowsMimeImage : public QWindowsMime {
public:
    int countCf()
    {
	return 1;
    }

    const char* convertorName()
    {
	return "Image";
    }

    int cf(int index)
    {
	return CF_DIB;
    }

    int cfFor(const char* mime)
    {
	if ( qstrnicmp(mime,"image/",5)==0 ) {
	    QStrList ofmts = QImage::outputFormats();
	    for (const char* fmt=ofmts.first(); fmt; fmt=ofmts.next())
		if ( qstricmp(fmt,mime+6)==0 )
		    return CF_DIB;
	}
	return 0;
    }

    const char* mimeFor(int cf)
    {
	if ( cf == CF_DIB )
	    return "image/bmp";
	else
	    return 0;
    }

    bool canConvert( const char* mime, int cf )
    {
	if ( cf == CF_DIB && qstrncmp(mime,"image/",5)==0 ) {
	    QStrList ofmts = QImage::outputFormats();
	    for (const char* fmt=ofmts.first(); fmt; fmt=ofmts.next())
		if ( qstricmp(fmt,mime+6)==0 )
		    return TRUE;
	}
	return FALSE;
    }

    QByteArray convertToMime( QByteArray data, const char* mime, int cf )
    {
	if ( qstrncmp(mime,"image/",6)!=0 || cf != CF_DIB )  // Sanity
	    return QByteArray();

	QImage img;  // Convert from DIB to chosen image format
	QBuffer iod(data);
	iod.open(IO_ReadOnly);
	QDataStream s(&iod);
	s.setByteOrder( QDataStream::LittleEndian );// Intel byte order ####
	if (qt_read_dib( s, img )) { // ##### encaps "-14"
	    QString ofmt = mime+6;
	    QByteArray ba;
	    QBuffer iod(ba);
	    iod.open(IO_WriteOnly);
	    QImageIO iio(&iod, ofmt.upper());
	    iio.setImage(img);
	    if (iio.write()) {
		iod.close();
		return ba;
	    }
	}

	// Failed
	return QByteArray();
    }

    QByteArray convertFromMime( QByteArray data, const char* mime, int cf )
    {
	if ( qstrncmp(mime,"image/",6)!=0 || cf != CF_DIB ) // Sanity
	    return QByteArray();

	QImage img;
	img.loadFromData((unsigned char*)data.data(),data.size());
	if (img.isNull())
	    return QByteArray();

	QByteArray ba;
	QBuffer iod(ba);
	iod.open(IO_WriteOnly);
	QDataStream s(&iod);
	s.setByteOrder( QDataStream::LittleEndian );// Intel byte order ####
	if (qt_write_dib(s, img)) {
	    return ba;
	} else {
	    return QByteArray();
	}
    }
};

static
void cleanup_mimes()
{
    QWindowsMime* wm;
    while ( (wm = mimes.first()) )
    {
	delete wm;
    }
}

void qt_init_windows_mime()
{
    if ( mimes.isEmpty() ) {
	new QWindowsMimeImage;
	new QWindowsMimeText;
	new QWindowsMimeAnyMime;
	qAddPostRoutine(cleanup_mimes);
    }
}

QWindowsMime*
QWindowsMime::convertor( const char* mime, int cf )
{
    QWindowsMime* wm;
    for ( wm = mimes.first(); wm; wm = mimes.next() ) {
	if ( wm->canConvert(mime,cf) ) {
	    return wm;
	}
    }
    return 0;
}

const char* QWindowsMime::cfToMime(int cf)
{
    const char* m=0;
    QWindowsMime* wm;
    for ( wm = mimes.first(); wm && !m; wm = mimes.next() ) {
	m = wm->mimeFor(cf);
    }
    return m;
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
    QWidget* widget;

public:    
    QOleDropTarget( QWidget* w );

    void releaseQt()
    {
	widget = 0;
    }

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
    BOOL acceptfmt;
};

static
QOleDropTarget *current_drop=0;
static
LPDATAOBJECT current_dropobj = 0;

bool QDragManager::eventFilter( QObject *, QEvent *)
{
    return FALSE;
}

void QDragManager::cancel()
{
    if ( object ) {
	beingCancelled = TRUE;
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


bool QDragMoveEvent::provides( const char * mimeType )
{
    if (!current_dropobj) // Sanity
	return FALSE;

    int n;
    FORMATETC *fmtetc = someFormats(mimeType,n);
    bool does = FALSE;
    for (int i=0; i<n && !does; i++) {
	int cf = fmtetc[i].cfFormat;
	QWindowsMime* wm = QWindowsMime::convertor(mimeType,cf);
	if ( wm && NOERROR == current_dropobj->QueryGetData(fmtetc+i) )
	    does = TRUE;
    }
    delete fmtetc;
    return does;
}

const char * QDragMoveEvent::format( int fn )
{
    if (!current_dropobj) // Sanity
	return 0;

    static QString fmt;
    fmt="";

    int n;
    LPFORMATETC fmtetc = allFormats(n);
    int i;
    for (i=0; i<n && fn >= 0; i++) {
	// Does the drag source provide this format that we accept?
	if (NOERROR == current_dropobj->QueryGetData(fmtetc+i))
	    fn--;
    }
    if ( fn==-1 )
	fmt = QWindowsMime::cfToMime(fmtetc[i-1].cfFormat);
    delete fmtetc;

    return fmt.isEmpty() ? 0 : (const char*)fmt;
}

QByteArray qt_olednd_obtain_data( const char *format )
{
    QByteArray result;

    if (!current_dropobj) // Sanity
	return result;

#ifdef USE_FORMATENUM // doesn't work yet
    LPENUMFORMATETC FAR fmtenum;
    HRESULT hr=current_dropobj->EnumFormatEtc(DATADIR_GET, &fmtenum);

    if ( hr == NOERROR ) {
	FORMATETC fmtetc;
	ULONG i=0;
	while (NOERROR==fmtenum->Next( i, &fmtetc, &i ) && i) {
	    int cf = fmtetc.cfFormat;
	    QWindowsMime* wm = QWindowsMime::convertor( format, cf );
	    STGMEDIUM medium;   
	    HGLOBAL hText;
	    HRESULT hr;

	    fmtetc.ptd = NULL;
	    fmtetc.dwAspect = DVASPECT_CONTENT;  
	    fmtetc.lindex = -1;
	    fmtetc.tymed = TYMED_HGLOBAL;       
	
	    hr = current_dropobj->GetData(&fmtetc, &medium);
	    if (!FAILED(hr)) {
		// Display the data and release it.
		hText = medium.hGlobal;
		void* d = GlobalLock(hText);
		int len = GlobalSize(medium.hGlobal);
		QByteArray tmp(len);
		memcpy(tmp.data(),d,len);
		GlobalUnlock(hText);
		ReleaseStgMedium(&medium);
		return wm->convertToMime( tmp, format, cf );
	    }
	}
    }
#else

    int n;
    FORMATETC *fmtetc = someFormats(format,n);
    for (int i=0; i<n && result.isNull(); i++) {
	int cf = fmtetc[i].cfFormat;
	QWindowsMime* wm = QWindowsMime::convertor(format,cf);
	if ( wm ) {
	    STGMEDIUM medium;   
	    HGLOBAL hText;
	    HRESULT hr = current_dropobj->GetData(fmtetc+i, &medium);
	    if (!FAILED(hr)) {
		// Display the data and release it.
		hText = medium.hGlobal;
		void* d = GlobalLock(hText);
		int len = GlobalSize(medium.hGlobal);
		QByteArray tmp(len);
		memcpy(tmp.data(),d,len);
		GlobalUnlock(hText);
		ReleaseStgMedium(&medium);
		result = wm->convertToMime( tmp, format, cf );
	    }
	}
    }
    delete fmtetc;
#endif
    return result;
}

QByteArray QDragMoveEvent::data( const char * format )
{
    return qt_olednd_obtain_data( format );
}

QByteArray QDropEvent::data( const char * format )
{
    return qt_olednd_obtain_data( format );
}


bool QDragManager::drag( QDragObject * o, QDragObject::DragMode mode )
{
    if ( object == o ) {
	return FALSE;
    }

    if ( object ) {
	cancel();
	dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    object = o;
    dragSource = (QWidget *)(object->parent());

    const char* fmt;
    for (int i=0; (fmt=object->format(i)); i++)
	registerMimeType(fmt);

    DWORD result_effect;
    QOleDropSource *src = new QOleDropSource(dragSource);
    QOleDataObject *obj = new QOleDataObject(o);
    // This drag source only allows copying of data.
    // Move and link is not allowed.
    DWORD allowed_effects;
    switch (mode) {
      case QDragObject::DragDefault:
	allowed_effects = DROPEFFECT_MOVE|DROPEFFECT_COPY;
	break;
      case QDragObject::DragMove:
	allowed_effects = DROPEFFECT_MOVE;
	break;
      case QDragObject::DragCopy:
	allowed_effects = DROPEFFECT_COPY;
	break;
      case QDragObject::DragCopyOrMove:
	allowed_effects = DROPEFFECT_MOVE|DROPEFFECT_COPY;
	break;
    }
    HRESULT r = DoDragDrop(obj, src, DROPEFFECT_COPY, &result_effect);     
    QDragResponseEvent e( r == DRAGDROP_S_DROP );
    QApplication::sendEvent( dragSource, &e );
    obj->Release();

    return result_effect==DROPEFFECT_MOVE;
}

void qt_olednd_unregister( QWidget* widget, QOleDropTarget *dst )
{
    RevokeDragDrop(widget->winId());
    dst->releaseQt();
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

    QWindowsMime *wm;

    const char* fmt;
    for (int i=0; (fmt=object->format(i)); i++) {
	if ((wm=QWindowsMime::convertor(fmt,pformatetc->cfFormat))
	    && pformatetc->dwAspect == DVASPECT_CONTENT &&
	       pformatetc->tymed == TYMED_HGLOBAL)
	{
	    QByteArray data =
		wm->convertFromMime(object->encodedData(fmt),
			    fmt, pformatetc->cfFormat);
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
    // provides data in a format that the target accepts.

    const char* fmt;
    for (int i=0; (fmt=object->format(i)); i++) {
	if (QWindowsMime::convertor(fmt,pformatetc->cfFormat) &&
	   pformatetc->dwAspect == DVASPECT_CONTENT &&
	   pformatetc->tymed == TYMED_HGLOBAL)
	{
	    return ResultFromScode(S_OK); 
	}
    }
    return ResultFromScode(S_FALSE);
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
    int n;
    LPFORMATETC fmtetc = allFormats(n);

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
   acceptfmt = FALSE;
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
    current_dropobj = pDataObj;
    current_drop = this; // ##### YUCK.  Arnt, we need to put info in event

    QDragEnterEvent de( QPoint(pt.x,pt.y) );
    QApplication::sendEvent( widget, &de );
    acceptfmt = de.isAccepted();

    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    QDragMoveEvent de( QPoint(pt.x,pt.y) );
    if ( acceptfmt )
	de.accept();
    else
	de.ignore();
    QApplication::sendEvent( widget, &de );
    acceptfmt = de.isAccepted();

    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragLeave()
{   
    acceptfmt = FALSE;   
    current_drop = 0;
    current_dropobj = 0;
    QDragLeaveEvent de;
    QApplication::sendEvent( widget, &de );
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
	DragLeave();
	//current_drop = 0;   - already done
	//current_dropobj = 0;
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
    
    if (!acceptfmt)
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
