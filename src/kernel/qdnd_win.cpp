/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_win.cpp#41 $
**
** Implementation of OLE drag and drop for Qt.
**
** Created : 980320
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qapplication.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qdragobject.h"
#include "qimage.h"
#include "qbuffer.h"
#include "qdatastream.h"
#include "qmessagebox.h"
#include "qbitmap.h"
#include "qt_windows.h"
#include <shlobj.h>


extern Qt::WindowsVersion qt_winver;

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
    virtual int cfFor(const char* )=0;
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


static
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


static
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
    QWindowsRegisteredMimeType(int c, const char *m) :
	cf(c), mime(m) {}
    int cf;
    QCString mime;
};

static QList<QWindowsRegisteredMimeType> mimetypes;

static
CLIPFORMAT registerMimeType(const char *mime)
{
    CLIPFORMAT f = RegisterClipboardFormatA(mime);
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
    int		countCf();
    const char* convertorName();
    int		cf(int index);
    int		cfFor(const char* mime);
    const char* mimeFor(int cf);
    bool	canConvert( const char* mime, int cf );
    QByteArray	convertToMime( QByteArray data, const char* , int );
    QByteArray	convertFromMime( QByteArray data, const char* , int );
};

int QWindowsMimeAnyMime::countCf()
{
    return mimetypes.count();
}

const char* QWindowsMimeAnyMime::convertorName()
{
    return "Any-Mime";
}

int QWindowsMimeAnyMime::cf(int index)
{
    return mimetypes.at(index)->cf;
}

int QWindowsMimeAnyMime::cfFor(const char* mime)
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

const char* QWindowsMimeAnyMime::mimeFor(int cf)
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

bool QWindowsMimeAnyMime::canConvert( const char* mime, int cf )
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

QByteArray QWindowsMimeAnyMime::convertToMime( QByteArray data, const char* , int )
{
    return data;
}

QByteArray QWindowsMimeAnyMime::convertFromMime( QByteArray data, const char* , int )
{
    return data;
}



class QWindowsMimeText : public QWindowsMime {
public:
    int		countCf();
    const char* convertorName();
    int		cf(int index);
    int		cfFor(const char* mime);
    const char* mimeFor(int cf);
    bool	canConvert( const char* mime, int cf );
    QByteArray	convertToMime( QByteArray data, const char* , int );
    QByteArray	convertFromMime( QByteArray data, const char* , int );
};

int QWindowsMimeText::countCf()
{
    return 2;
}

const char* QWindowsMimeText::convertorName()
{
    return "Text";
}

int QWindowsMimeText::cf(int index)
{
    return index ? CF_UNICODETEXT : CF_TEXT;  // Note UNICODE first.
}

int QWindowsMimeText::cfFor(const char* mime)
{
    if ( 0==qstricmp( mime, "text/plain" ) )
	return CF_TEXT;
    else if ( 0==qstricmp( mime, "text/utf16" ) )
	return CF_UNICODETEXT;
    else
	return 0;
}

const char* QWindowsMimeText::mimeFor(int cf)
{
    if ( cf == CF_TEXT )
	return "text/plain";
    else if ( cf == CF_UNICODETEXT )
	return "text/utf16";
    else
	return 0;
}

bool QWindowsMimeText::canConvert( const char* mime, int cf )
{
    return cfFor(mime) == cf;
}

QByteArray QWindowsMimeText::convertToMime( QByteArray data, const char* mime, int cf )
{
    if ( cf == CF_TEXT ) {
	if ( data[(int)data.size()-1] ) {
	    // Strip nul
	    QByteArray r(data.size()-1);
	    memcpy(r.data(),data.data(),r.size());
	    return r;
	} else {
	    // Not nul-terminated
	    return data;
	}
    }

    // Windows uses un-marked little-endian nul-terminated Unicode
    int s = data.size();
    if ( !data[s-2] && !data[s-1] )
	s -= 2; // strip nul
    QByteArray r(s+2);
    r[0]=char(0xff);
    r[1]=char(0xfe);
    memcpy(r.data()+2,data.data(),s);
    return r;
}

QByteArray QWindowsMimeText::convertFromMime( QByteArray data, const char* mime, int cf )
{
    if ( cf == CF_TEXT ) {
	QByteArray r(data.size()+1);
	memcpy(r.data(),data.data(),data.size());
	r[(int)data.size()]='\0';
	return r;
    }

    if (data.size() < 2)
	return QByteArray();

    // Windows uses un-marked little-endian nul-terminated Unicode
    if ( data[0] == char(0xff) && data[1] == char(0xfe) )
    {
	// Right way - but skip header and add nul
	QByteArray r(data.size());
	memcpy(r.data(),data.data()+2,data.size()-2);
	r[(int)data.size()-2] = 0;
	r[(int)data.size()-1] = 0;
	return r;
    } else {
	// Wrong way - reorder.
	int s = data.size();
	if ( s&1 ) {
	    // Odd byte - drop last
	    s--;
	}
	char* i = data.data();
	if ( i[0] == char(0xfe) && i[1] == char(0xff) ) {
	    i += 2;
	    s -= 2;
	}
	QByteArray r(s+2);
	char* o = r.data();
	while (s) {
	    o[0] = i[1];
	    o[1] = i[0];
	    i += 2;
	    o += 2;
	    s -= 2;
	}
	r[(int)r.size()-2] = 0;
	r[(int)r.size()-1] = 0;
	return r;
    }
}



class QWindowsMimeImage : public QWindowsMime {
public:
    int		countCf();
    const char* convertorName();
    int		cf(int index);
    int		cfFor(const char* mime);
    const char* mimeFor(int cf);
    bool	canConvert( const char* mime, int cf );
    QByteArray	convertToMime( QByteArray data, const char* mime, int cf );
    QByteArray	convertFromMime( QByteArray data, const char* mime, int cf );
};

int QWindowsMimeImage::countCf()
{
    return 1;
}

const char* QWindowsMimeImage::convertorName()
{
    return "Image";
}

int QWindowsMimeImage::cf(int index)
{
    return CF_DIB;
}

int QWindowsMimeImage::cfFor(const char* mime)
{
    if ( qstrnicmp(mime,"image/",5)==0 ) {
	QStrList ofmts = QImage::outputFormats();
	for (const char* fmt=ofmts.first(); fmt; fmt=ofmts.next())
	    if ( qstricmp(fmt,mime+6)==0 )
		return CF_DIB;
    }
    return 0;
}

const char* QWindowsMimeImage::mimeFor(int cf)
{
    if ( cf == CF_DIB )
	return "image/bmp";
    else
	return 0;
}

bool QWindowsMimeImage::canConvert( const char* mime, int cf )
{
    if ( cf == CF_DIB && qstrnicmp(mime,"image/",5)==0 ) {
	QStrList ofmts = QImage::outputFormats();
	for (const char* fmt=ofmts.first(); fmt; fmt=ofmts.next())
	    if ( qstricmp(fmt,mime+6)==0 )
		return TRUE;
    }
    return FALSE;
}

QByteArray QWindowsMimeImage::convertToMime( QByteArray data, const char* mime, int cf )
{
    if ( qstrnicmp(mime,"image/",6)!=0 || cf != CF_DIB )  // Sanity
	return QByteArray();

    QImage img;  // Convert from DIB to chosen image format
    QBuffer iod(data);
    iod.open(IO_ReadOnly);
    QDataStream s(&iod);
    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order ####
    if (qt_read_dib( s, img )) { // ##### encaps "-14"
	QCString ofmt = mime+6;
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

QByteArray QWindowsMimeImage::convertFromMime( QByteArray data, const char* mime, int cf )
{
    if ( qstrnicmp(mime,"image/",6)!=0 || cf != CF_DIB ) // Sanity
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



static const char* protocol = "file:/";
static int protocol_len = 6;

static
int dtoh(int d) // NOT SAFE, only use internally
{
    return "0123456789abcdef"[d];
}

static
int htod(int h)
{
    if (isdigit(h)) return h-'0';
    return tolower(h)-'a';
}


class QWindowsMimeUrl : public QWindowsMime {
public:
    int		countCf();
    const char* convertorName();
    int		cf(int index);
    int		cfFor(const char* mime);
    const char* mimeFor(int cf);
    bool	canConvert( const char* mime, int cf );
    QByteArray	convertToMime( QByteArray data, const char* mime, int cf );
    QByteArray	convertFromMime( QByteArray data, const char* mime, int cf );
};

int QWindowsMimeUrl::countCf()
{
    return 1;
}

const char* QWindowsMimeUrl::convertorName()
{
    return "Urls";
}

int QWindowsMimeUrl::cf(int index)
{
    return CF_HDROP;
}

int QWindowsMimeUrl::cfFor(const char* mime)
{
    return qstricmp(mime,"url/url")==0;
}

const char* QWindowsMimeUrl::mimeFor(int cf)
{
    if ( cf == CF_HDROP )
	return "url/url";
    else
	return 0;
}

bool QWindowsMimeUrl::canConvert( const char* mime, int cf )
{
    return cf == CF_HDROP && 0==qstricmp(mime,"url/url");
}

QByteArray QWindowsMimeUrl::convertToMime( QByteArray data, const char* mime, int cf )
{
    if ( qstricmp(mime,"url/url")!=0 || cf != CF_HDROP )  // Sanity
	return QByteArray();

    LPDROPFILES hdrop = (LPDROPFILES)data.data();
    const char* files = (const char* )data.data() + hdrop->pFiles;
    const char* end = (const char* )data.data() + data.size();
    const ushort* filesw = (const ushort*)(data.data() + hdrop->pFiles);
    int i=0;
    int size=0;
    bool wide = hdrop->fWide;
    while (
	// until double-NUL
	wide ? filesw[i] || filesw[i+1]
	     : files[i] || files[i+1]
    ) {
	char ch = wide ? filesw[i] : files[i];
	if ( !ch )
	    size+=protocol_len+1;
	else if ( ch == '+' || ch == '%' ) // ### more
	    size+=3;
	else
	    size++;
	i++;
	if ( (wide ? (const char* )(filesw+i) : files+i) >= end )
	    return QByteArray(); // Bad Data
    }
    if (i)
	size += protocol_len;

    QByteArray result(size);

    char* out = result.data();

    if ( size ) {
	memcpy(out, protocol, protocol_len);
	out += protocol_len;
    }

    i = 0;
    while (
	// until double-NUL
	wide ? filesw[i] || filesw[i+1]
	     : files[i] || files[i+1]
    ) {
	char ch = wide ? filesw[i] : files[i];
	if ( !ch ) {
	    *out++ = ch;
	    memcpy(out, protocol, protocol_len);
	    out += protocol_len;
	} else if ( ch == '+' || ch == '%' ) {
	    // special. ### more
	    *out++ = '%';
	    *out++ = dtoh(ch/16);
	    *out++ = dtoh(ch%16);
	} else if ( ch == ' ' ) {
	    *out++ = '+';
	} else if ( ch == ':' ) {
	    *out++ = '|';
	} else {
	    *out++ = ch;
	}
	i++;
	ASSERT( result.data()+size >= out );
    }

    return result;
}

QByteArray QWindowsMimeUrl::convertFromMime( QByteArray data, const char* mime, int cf )
{
    if ( qstricmp(mime,"url/url")!=0 || cf != CF_HDROP )  // Sanity
	return QByteArray();

    DROPFILES hdrop;
    hdrop.pFiles = sizeof(hdrop);
    GetCursorPos(&hdrop.pt); // try
    hdrop.fNC = TRUE;
    hdrop.fWide = FALSE;

    const char* urls = (const char* )data.data();
    int size=0;
    bool expectprotocol=TRUE;
    bool ignore=FALSE;
    uint i=0;

    while (i < data.size()) {
	if ( expectprotocol ) {
	    if ( 0!=qstrncmp( protocol, urls+i, protocol_len ) ) {
		ignore = TRUE;
	    } else {
		i += protocol_len;
		if ( urls[i] == '/' && urls[i+1] != '/' ) {
		    // Host specified!
		    ignore = TRUE;
		}
	    }
	    expectprotocol = FALSE;
	}
	if ( !urls[i] ) {
	    expectprotocol = TRUE;
	} else if ( urls[i] == '%' && urls[i+1] && urls[i+2] ) {
	    i+=2;
	}
	if ( !ignore )
	    size++;
	i++;
    }

    size += sizeof(hdrop);
    size += 2; // double-NUL
    QByteArray result(size);

    char* out = result.data();

    memcpy(out, &hdrop, sizeof(hdrop));
    out += sizeof(hdrop);

    expectprotocol=TRUE;
    ignore=FALSE;
    i=0;
    while (i < data.size()) {
	if ( expectprotocol ) {
	    if ( 0!=qstrncmp( protocol, urls+i, protocol_len ) ) {
		ignore = TRUE;
	    } else {
		i += protocol_len;
		if ( urls[i] == '/' && urls[i+1] != '/' ) {
		    // Host specified!
		    ignore = TRUE;
		}
	    }
	    expectprotocol = FALSE;
	}
	if ( urls[i] == '%' && urls[i+1] && urls[i+2] ) {
	    *out++ = htod(urls[i+1])*16 + htod(urls[i+2]);
	} else {
	    if ( !urls[i] )
		expectprotocol = TRUE;
	    if (!ignore) {
		if ( urls[i] == '+' )
		    *out++ = ' ';
		else if ( urls[i] == '|' )
		    *out++ = ':';
		else
		    *out++ = urls[i];
	    }
	}
	i++;
    }
    // double-NUL
    *out++ = 0;
    *out++ = 0;

    ASSERT( result.data()+size == out );

    return result;
}



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
	new QWindowsMimeUrl;
	new QWindowsMimeAnyMime;
	qAddPostRoutine(cleanup_mimes);
    }
}

QWindowsMime*
QWindowsMime::convertor( const char *mime, int cf )
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

static HCURSOR *cursor = 0;


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


bool QDragMoveEvent::provides( const char* mimeType ) const
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

static
const char* dnd_format( int fn )
{
    if (!current_dropobj) // Sanity
	return 0;

    static QCString fmt;
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

const char* QDragMoveEvent::format( int fn ) const
{
    return dnd_format(fn);
}

const char* QDropEvent::format( int fn ) const
{
    return dnd_format(fn);
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

QByteArray QDragMoveEvent::encodedData( const char *format ) const
{
    return qt_olednd_obtain_data( format );
}

QByteArray QDropEvent::encodedData( const char* format ) const
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
    updatePixmap();
    HRESULT r = DoDragDrop(obj, src, DROPEFFECT_COPY, &result_effect);
    QDragResponseEvent e( r == DRAGDROP_S_DROP );
    QApplication::sendEvent( dragSource, &e );
    obj->Release();

    delete obj; // NOTE: in X11 version, this object lives a little longer.
    object = 0;
    updatePixmap();

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


STDMETHODIMP_(ULONG)
QOleDropSource::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
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
    if ( cursor ) {
	int c = -1;
	switch ( dwEffect ) {
	  case DROPEFFECT_MOVE:
	    c = 0;
	    break;
	  case DROPEFFECT_COPY:
	    c = 1;
	    break;
	  case DROPEFFECT_LINK:
	    c = 2;
	    break;
	}
	if ( c >= 0 ) {
	    SetCursor(cursor[c]);
	    return ResultFromScode(S_OK);
	}
    }
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


STDMETHODIMP_(ULONG)
QOleDataObject::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
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

    QDragEnterEvent de( widget->mapFromGlobal(QPoint(pt.x,pt.y)) );
    QApplication::sendEvent( widget, &de );
    acceptfmt = de.isAccepted();

    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    QDragMoveEvent de( widget->mapFromGlobal(QPoint(pt.x,pt.y)) );
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
	QDropEvent de( widget->mapFromGlobal(QPoint(pt.x,pt.y)) );
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

void QDragManager::updatePixmap()
{
    if ( object ) {
	if ( cursor ) {
	    for ( int i=0; i<n_cursor; i++ ) {
		DestroyCursor(cursor[i]);
	    }
	    delete [] cursor;
	    cursor = 0;
	}

	QPixmap pm = object->pixmap();
	if ( pm.isNull() ) {
	    // None.
	} else {
	    cursor = new HCURSOR[n_cursor];
	    QPoint pm_hot = object->pixmapHotSpot();
	    for (int cnum=0; cnum<n_cursor; cnum++) {
		QPixmap cpm = pm_cursor[cnum];

		int x1 = QMIN(-pm_hot.x(),0);
		int x2 = QMAX(pm.width()-pm_hot.x(),cpm.width());
		int y1 = QMIN(-pm_hot.y(),0);
		int y2 = QMAX(pm.height()-pm_hot.y(),cpm.height());

		int w = x2-x1+1;
		int h = y2-y1+1;

		if ( qt_winver == WV_32s || qt_winver == WV_95 ) {
		    // Limited cursor size
		    int reqw = GetSystemMetrics(SM_CXCURSOR);
		    int reqh = GetSystemMetrics(SM_CYCURSOR);
		    if ( reqw < w ) {
			// Not wide enough - move objectpm right
			pm_hot.setX(pm_hot.x()-w+reqw);
		    }
		    if ( reqh < h ) {
			// Not tall enough - move objectpm right
			pm_hot.setY(pm_hot.y()-h+reqh);
		    }
		    w = reqw;
		    h = reqh;
		}

		QPixmap colorbits(w,h);
		{
		    QPainter p(&colorbits);
		    p.fillRect(0,0,w,h,color1);
		    p.drawPixmap(QMAX(0,-pm_hot.x()),QMAX(0,-pm_hot.y()),pm);
		    p.drawPixmap(QMAX(0,pm_hot.x()),QMAX(0,pm_hot.y()),cpm);
		}

		QBitmap maskbits(w,h,TRUE);
		maskbits.setOptimization(QPixmap::NoOptim);
		{
		    QPainter p(&maskbits);
		    if ( pm.mask() ) {
			QBitmap m(*pm.mask());
			m.setMask(m);
			p.drawPixmap(QMAX(0,-pm_hot.x()),QMAX(0,-pm_hot.y()),m);
		    } else {
			p.fillRect(QMAX(0,-pm_hot.x()),QMAX(0,-pm_hot.y()),
			    pm.width(),pm.height(),color1);
		    }
		    if ( cpm.mask() ) {
			QBitmap m(*cpm.mask());
			m.setMask(m);
			p.drawPixmap(QMAX(0,pm_hot.x()),QMAX(0,pm_hot.y()),m);
		    } else {
			p.fillRect(QMAX(0,pm_hot.x()),QMAX(0,pm_hot.y()),
			    cpm.width(),cpm.height(),
			    color1);
		    }
		}

		ICONINFO ii;
		ii.fIcon = FALSE;
		ii.xHotspot = QMAX(0,pm_hot.x());
		ii.yHotspot = QMAX(0,pm_hot.y());
		ii.hbmMask = maskbits.hbm();
		ii.hbmColor = colorbits.hbm();
		cursor[cnum] = CreateIconIndirect(&ii);
	    }
	}
    }
}
