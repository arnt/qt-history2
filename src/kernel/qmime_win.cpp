/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmime_win.cpp#10 $
**
** Implementation of Win32 MIME <-> clipboard converters
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qmime.h"
#include "qstrlist.h"
#include "qimage.h"
#include "qdatastream.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qt_windows.h"
#include <shlobj.h>

extern bool qt_read_dib( QDataStream&, QImage& ); // qimage.cpp
extern bool qt_write_dib( QDataStream&, QImage );   // qimage.cpp
extern Qt::WindowsVersion qt_winver;


static QList<QWindowsMime> mimes;

/*!
  \class QWindowsMime qmime.h
  \brief Maps open-standard MIME types with Window Clipboard formats.

  The drag-and-drop and clipboard facilities of Qt use the MIME standard.
  On X11, this maps trivially to the Xdnd protocol, but on Windows only
  some applications use MIME types to describe the clipboard formats,
  while others use arbitrary non-standardized naming conventions, or
  completely unnamed built-in formats of Windows.

  By instantiating subclasses of QWindowsMime that provide conversions
  between Windows Clipboard and MIME formats, you can convert
  proprietary clipboard formats to MIME formats.

  Qt has predefined support for the following Windows Clipboard formats:
  <ul>
    <li> \c CF_UNICODETEXT - converted to "text/plain;charset=utf16"
	    and thus supported by QTextDrag.
    <li> \c CF_TEXT - converted to "text/plain;charset=system" or "text/plain"
	    and thus supported by QTextDrag.
    <li> \c CF_DIB - converted to "image/*", where * is
		a \link QImage::outputFormats() Qt image format\endlink,
	    and thus supported by QImageDrag.
    <li> \c CF_HDROP - converted to "text/uri-list",
	    and thus supported by QUriDrag.
  </ul>

  An example usage of this class would be to map the Windows Metafile
  clipboard format (CF_METAFILEPICT) to and from the MIME type "image/x-wmf".
  This conversion might simply be adding or removing a header, or even
  just passing on the data.  See the
  \link dnd.html Drag-and-Drop documentation\endlink for more information
  on choosing and definition MIME types.
*/

/*!
  Constructs a new conversion object, adding it to the globally accessed
  list of available convertors.
*/
QWindowsMime::QWindowsMime()
{
    mimes.append(this);
}

/*!
  Destructs a conversion object, removing it from the global
  list of available convertors.
*/
QWindowsMime::~QWindowsMime()
{
    mimes.remove(this);
}



struct QWindowsRegisteredMimeType {
    QWindowsRegisteredMimeType(int c, const char *m) :
	cf(c), mime(m) {}
    int cf;
    QCString mime;
};

static QList<QWindowsRegisteredMimeType> mimetypes;

/*!
  This is an internal function.
*/
int QWindowsMime::registerMimeType(const char *mime)
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



class Q_EXPORT QWindowsMimeAnyMime : public QWindowsMime {
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



class Q_EXPORT QWindowsMimeText : public QWindowsMime {
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
    if ( index == 0 )
	return CF_UNICODETEXT;
    else
	return CF_TEXT;
}

int QWindowsMimeText::cfFor(const char* mime)
{
    if ( 0==qstricmp( mime, "text/plain" ) )
	return CF_TEXT;
    QCString m(mime);
    int i = m.find("charset=");
    if ( i >= 0 ) {
	QCString cs(m.data()+i+8);
	i = cs.find(";");
	if ( i>=0 )
	    cs = cs.left(i);
	if ( cs == "system" )
	    return CF_TEXT;
	if ( cs == "utf16" )
	    return CF_UNICODETEXT;
    }
    return 0;
}

const char* QWindowsMimeText::mimeFor(int cf)
{
    if ( cf == CF_TEXT )
	return "text/plain";
    else if ( cf == CF_UNICODETEXT )
	return "text/plain;charset=utf16";
    else
	return 0;
}

bool QWindowsMimeText::canConvert( const char* mime, int cf )
{
    return cfFor(mime) == cf;
}

QByteArray QWindowsMimeText::convertToMime( QByteArray data, const char* /*mime*/, int cf )
{
    if ( cf == CF_TEXT ) {
	// text/plain doesn't have a NUL, but CF_TEXT does.
	QByteArray r(data.size()+1);
	memcpy(r.data(),data.data(),data.size());
	r[(int)data.size()]=0;
	return r;
    }

    // Windows uses un-marked little-endian nul-terminated Unicode
    int ms = data.size();
    int s;
    // Find NUL
    for (s=0; s<ms-1 && (data[s+0] || data[s+1]); s+=2)
	;

    QByteArray r(s+2);
    r[0]=char(0xff); // BOM
    r[1]=char(0xfe);
    memcpy(r.data()+2,data.data(),s);
    return r;
}

QByteArray QWindowsMimeText::convertFromMime( QByteArray data, const char* /*mime*/, int cf )
{
    if ( cf == CF_TEXT ) {
	// Ad NUL (### only really necessary if no NUL already)
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



class Q_EXPORT QWindowsMimeImage : public QWindowsMime {
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
    return index == 0 ? CF_DIB : 0;
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


class Q_EXPORT QWindowsMimeUri : public QWindowsMime {
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

int QWindowsMimeUri::countCf()
{
    return 1;
}

const char* QWindowsMimeUri::convertorName()
{
    return "Uris";
}

int QWindowsMimeUri::cf(int index)
{
    return index == 0 ? CF_HDROP : 0;
}

int QWindowsMimeUri::cfFor(const char* mime)
{
    return qstricmp(mime,"text/uri-list")==0;
}

const char* QWindowsMimeUri::mimeFor(int cf)
{
    if ( cf == CF_HDROP )
	return "text/uri-list";
    else
	return 0;
}

bool QWindowsMimeUri::canConvert( const char* mime, int cf )
{
    return cf == CF_HDROP && 0==qstricmp(mime,"text/uri-list");
}

QByteArray QWindowsMimeUri::convertToMime( QByteArray data, const char* mime, int cf )
{
    if ( qstricmp(mime,"text/uri-list")!=0 || cf != CF_HDROP )  // Sanity
	return QByteArray();

    LPDROPFILES hdrop = (LPDROPFILES)data.data();
    const char* files = (const char* )data.data() + hdrop->pFiles;
    const ushort* filesw = (const ushort*)files;

    int i=0;
    QCString texturi;
    if ( hdrop->fWide ) {
	while ( filesw[i] ) {
	    QString fn = qt_winQString( (void*)(filesw+i) );
	    texturi += QUriDrag::localFileToUri(fn);
	    texturi += "\r\n";
	    i += fn.length()+1;
	}
    } else {
	while ( files[i] ) {
	    QString fn = qt_winMB2QString( files+i );
	    texturi += QUriDrag::localFileToUri(fn);
	    texturi += "\r\n";
	    i += fn.length()+1;
	}
    }

    return texturi;
}

QByteArray QWindowsMimeUri::convertFromMime( QByteArray data, const char* mime, int cf )
{
    if ( qstricmp(mime,"text/uri-list")!=0 || cf != CF_HDROP )  // Sanity
	return QByteArray();

    QStoredDrag t("text/uri-list");
    t.setEncodedData(data);
    QStringList fn;
    QUriDrag::decodeLocalFiles( &t, fn );

    int size = sizeof(DROPFILES)+2;
    QStringList::Iterator i;
    for ( i = fn.begin(); i!=fn.end(); ++i ) {
	if ( qt_winver == Qt::WV_NT )
	    size += (*i).length()+1;
	else
	    size += (*i).local8Bit().length()+1;
    }

    QByteArray result(size*sizeof(TCHAR));
    DROPFILES* d = (DROPFILES*)result.data();
    d->pFiles = sizeof(DROPFILES);
    GetCursorPos(&d->pt); // try
    d->fNC = TRUE;
    char* files = ((char* )d) + d->pFiles;

    if ( qt_winver == Qt::WV_NT ) {
	d->fWide = TRUE;
	WCHAR* f = (WCHAR*)files;

	for ( i = fn.begin(); i!=fn.end(); ++i ) {
	    int l = (*i).length();
	    memcpy(f, (WCHAR*)(*i).unicode(), l*sizeof(WCHAR));
	    for (int j = 0; j<l; j++)
		if ( f[j] == '/' )
		    f[j] = '\\';
	    f += l;
	    *f++ = 0;
	}
	*f = 0;
    } else {
	d->fWide = FALSE;
	char* f = files;

	for ( i = fn.begin(); i!=fn.end(); ++i ) {
	    QCString c = (*i).local8Bit();
	    int l = c.length();
	    memcpy(f, c.data(), l);
	    for (int j = 0; j<l; j++)
		if ( f[j] == '/' )
		    f[j] = '\\';
	    f += l;
	    *f++ = 0;
	}
	*f = 0;
    }
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
    mimetypes.setAutoDelete(TRUE);
    mimetypes.clear();
}

/*!
  This is an internal function.
*/
void QWindowsMime::initialize()
{
    if ( mimes.isEmpty() ) {
	new QWindowsMimeImage;
	new QWindowsMimeText;
	new QWindowsMimeUri;
	new QWindowsMimeAnyMime;
	qAddPostRoutine(cleanup_mimes);
    }
}

/*!
  Returns the most-recently created QWindowsMime that can convert
  between \a mime and \a cf formats.  Returns 0 if no such convertor
  exists.
*/
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

/*!
  Returns a MIME type for \a cf, or 0 if none exists.
*/
const char* QWindowsMime::cfToMime(int cf)
{
    const char* m=0;
    QWindowsMime* wm;
    for ( wm = mimes.first(); wm && !m; wm = mimes.next() ) {
	m = wm->mimeFor(cf);
    }
    return m;
}

/*!
  Returns a list of all currently defined QWindowsMime objects.
*/
QList<QWindowsMime> QWindowsMime::all()
{
    return mimes;
}

/*!
  \fn const char* QWindowsMime::convertorName()

  Returns a name for the convertor.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn int QWindowsMime::countCf()

  Returns the number of Windows Clipboard formats supported by this
  convertor.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn int QWindowsMime::cf(int index)

  Returns the Windows Clipboard formats supported by this
  convertor that is orderinally at \a index.  So cf(0) returns
  the first Windows Clipboard format supported, cf(countCf()-1) returns
  the last.  Return values out of range are undefined.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn bool QWindowsMime::canConvert( const char* mime, int cf )

  Returns TRUE if the convertor can convert (both ways) between
  \a mime and \a cf.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn const char* QWindowsMime::mimeFor(int cf)

  Returns the MIME type used for Windows Clipboard format \a cf, or
  0 if this convertor does not support \a cf.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn int QWindowsMime::cfFor(const char* mime)

  Returns the Windows Clipboard type used for MIME type \a mime, or
  0 if this convertor does not support \a mime.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn QByteArray QWindowsMime::convertToMime( QByteArray data, const char* mime, int cf )

  Returns \a data converted from Windows Clipboard format \a cf
    to MIME type \a mime.

  Note that Windows Clipboard formats must all be self-terminating.  The
  input \a data may contain trailing data.

  All subclasses must override this purely virtual function.
*/

/*!
  \fn QByteArray QWindowsMime::convertFromMime( QByteArray data, const char* mime, int cf )

  Returns \a data converted from MIME type \a mime
    to Windows Clipboard format \a cf.

  Note that Windows Clipboard formats must all be self-terminating.  The
  return value may contain trailing data.

  All subclasses must override this purely virtual function.
*/

