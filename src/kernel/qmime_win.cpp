/****************************************************************************
** $Id: $
**
** Implementation of Win32 MIME <-> clipboard converters
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
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

#include "qmime.h"

#ifndef QT_NO_MIME

#include "qstrlist.h"
#include "qimage.h"
#include "qdatastream.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include <shlobj.h>

#if defined(__MINGW32__)
#include <ctype.h>
#endif

extern bool qt_read_dib( QDataStream&, QImage& ); // qimage.cpp
extern bool qt_write_dib( QDataStream&, QImage );   // qimage.cpp


static QPtrList<QWindowsMime> mimes;

/*!
  \class QWindowsMime qmime.h
  \brief The QWindowsMime class maps open-standard MIME to Window Clipboard formats.
  \ingroup io
  \ingroup draganddrop
  \ingroup misc

  Qt's drag-and-drop and clipboard facilities use the MIME standard.
  On X11, this maps trivially to the Xdnd protocol, but on Windows
  although some applications use MIME types to describe clipboard
  formats, others use arbitrary non-standardized naming conventions,
  or unnamed built-in formats of Windows.

  By instantiating subclasses of QWindowsMime that provide conversions
  between Windows Clipboard and MIME formats, you can convert
  proprietary clipboard formats to MIME formats.

  Qt has predefined support for the following Windows Clipboard formats:
  \list
    \i CF_UNICODETEXT - converted to "text/plain;charset=ISO-10646-UCS-2"
	    and supported by QTextDrag.
    \i CF_TEXT - converted to "text/plain;charset=system" or "text/plain"
	    and supported by QTextDrag.
    \i CF_DIB - converted to "image/*", where * is
		a \link QImage::outputFormats() Qt image format\endlink,
	    and supported by QImageDrag.
    \i CF_HDROP - converted to "text/uri-list",
	    and supported by QUriDrag.
  \endlist

  An example use of this class would be to map the Windows Metafile
  clipboard format (CF_METAFILEPICT) to and from the MIME type "image/x-wmf".
  This conversion might simply be adding or removing a header, or even
  just passing on the data.  See the
  \link dnd.html Drag-and-Drop documentation\endlink for more information
  on choosing and definition MIME types.

  You can check if a MIME type is convertible using canConvert() and
  can perform conversions with convertToMime() and convertFromMime().
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
  Destroys a conversion object, removing it from the global
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

static QPtrList<QWindowsRegisteredMimeType> mimetypes;

/*!
    \internal
  This is an internal function.
*/
int QWindowsMime::registerMimeType(const char *mime)
{
#ifdef Q_OS_TEMP
    CLIPFORMAT f = RegisterClipboardFormat( QString( mime ).ucs2() );
#else
    CLIPFORMAT f = RegisterClipboardFormatA( mime );
#endif
#ifndef QT_NO_DEBUG
    if ( !f )
	qSystemWarning( "QWindowsMime: Failed to register clipboard format" );
#endif
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
    // try to register the mime type
    registerMimeType(mime);
    mt = mimetypes.current();
    if( mt && 0 == qstricmp(mt->mime, mime) )
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
	if ( mt->cf == cf && 0==qstricmp(mt->mime,mime) ) {
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

    return 0==qstricmp(mt->mime,mime);
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
	if ( cs == "ISO-10646-UCS-2" || cs == "utf16" )
	    return CF_UNICODETEXT;
    }
    return 0;
}

const char* QWindowsMimeText::mimeFor(int cf)
{
    if ( cf == CF_TEXT )
	return "text/plain";
    else if ( cf == CF_UNICODETEXT ) {
	QT_WA( {
	    return "text/plain;charset=ISO-10646-UCS-2";
        } , {
	    return "text/plain;charset=utf16";
	} );
    }
    else
	return 0;
}

bool QWindowsMimeText::canConvert( const char* mime, int cf )
{
    return cfFor(mime) == cf;
}

/*
    text/plain is defined as using CRLF, but so many programs don't,
    and programmers just look for '\n' in strings.
    Windows really needs CRLF, so we ensure it here.
*/

QByteArray QWindowsMimeText::convertToMime( QByteArray data, const char* /*mime*/, int cf )
{
    if ( cf == CF_TEXT ) {
	const char* d = data.data();
	const int s = qstrlen(d);
	QByteArray r(data.size()+1);
	char* o = r.data();
	int j=0;
	for (int i=0; i<s; i++) {
	    char c = d[i];
	    if (c!='\r')
		o[j++]=c;
	}
	o[j]=0;
	return r;
    }

    // Windows uses un-marked little-endian nul-terminated Unicode
    int ms = data.size();
    int s;
    // Find NUL
    for (s=0; s<ms-1 && (data[s+0] || data[s+1]); s+=2)
    {
    }

    QByteArray r(s+2);
    r[0]=uchar(0xff); // BOM
    r[1]=uchar(0xfe);
    memcpy(r.data()+2,data.data(),s);
    return r;
}

extern QTextCodec* findcharset(const QCString& mimetype);

QByteArray QWindowsMimeText::convertFromMime( QByteArray data, const char* mime, int cf )
{
    if ( cf == CF_TEXT ) {
	// Anticipate required space for CRLFs at 1/40
	int maxsize=data.size()+data.size()/40+3;
	QByteArray r(maxsize);
	char* o = r.data();
	const char* d = data.data();
	const int s = data.size();
	bool cr=FALSE;
	int j=0;
	for (int i=0; i<s; i++) {
	    char c = d[i];
	    if (c=='\r')
		cr=TRUE;
	    else {
		if (c=='\n') {
		    if (!cr)
			o[j++]='\r';
		}
		cr=FALSE;
	    }
	    o[j++]=c;
	    if ( j+3 >= maxsize ) {
		maxsize += maxsize/4;
		r.resize(maxsize);
		o = r.data();
	    }
	}
	o[j]=0;
	return r;
    } else if ( cf == CF_UNICODETEXT ) {
	QTextCodec *codec = findcharset(mime);
	QString str = codec->toUnicode( data );
	const QChar *u = str.unicode();
	QString res;
	const int s = str.length();
	int maxsize = s + s/40 + 3;
	res.setLength( maxsize );
	int ri = 0;
	bool cr = FALSE;
	for ( int i=0; i < s; ++i ) {
	    if ( *u == '\r' )
		cr = TRUE;
	    else {
		if ( *u == '\n' && !cr )
		    res[ri++] = QChar('\r');
		cr = FALSE;
	    }
	    res[ri++] = *u;
	    if ( ri+3 >= maxsize ) {
		maxsize += maxsize/4;
		res.setLength( maxsize );
	    }
	    ++u;
	}
	res.truncate( ri );
	const int byteLength = res.length()*2;
	QByteArray r( byteLength + 2 );
	memcpy( r.data(), res.unicode(), byteLength );
	r[byteLength] = 0;
	r[byteLength+1] = 0;
	return r;
    }

    if (data.size() < 2)
	return QByteArray();

    // Windows uses un-marked little-endian nul-terminated Unicode
    if ( (uchar)data[0] == uchar(0xff) && (uchar)data[1] == uchar(0xfe) )
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
	if ( (uchar)i[0] == uchar(0xfe) && (uchar)i[1] == uchar(0xff) ) {
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

#if 0

static const int CF_HTML = 0xc082; // the one MS apps use....

class QWindowsMimeHtml : public QWindowsMime {
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


int QWindowsMimeHtml::countCf()
{
    return 1;
}

const char* QWindowsMimeHtml::convertorName()
{
    return "Html";
}

int QWindowsMimeHtml::cf(int index)
{
    if ( index == 0 )
	return CF_HTML;
    else
	return 0;
}

int QWindowsMimeHtml::cfFor(const char* mime)
{
    if ( 0==qstricmp( mime, "text/html" ) )
	return CF_HTML;
    return 0;
}

const char* QWindowsMimeHtml::mimeFor(int cf)
{
    if ( cf == CF_HTML )
	return "text/html";
    return 0;
}

bool QWindowsMimeHtml::canConvert( const char* mime, int cf )
{
    return cfFor(mime) == cf;
}

#include <qregexp.h>

/*
    The windows HTML clipboard format is as follows (xxxxxxxxxx is a 10 integer number giving the positions
    in bytes). Charset used is mostly utf8, but can be different, ie. we have to look for the <meta> charset tag

Version: 1.0
StartHTML:xxxxxxxxxx
EndHTML:xxxxxxxxxx
StartFragment:xxxxxxxxxx
EndFragment:xxxxxxxxxx
...html...

*/
QByteArray QWindowsMimeHtml::convertToMime( QByteArray _data, const char* /*mime*/, int cf )
{
    QCString data( _data );
    int ms = data.size();
    QCString result;
    qDebug("fragment = \n%s\n", data.data() );
#if 1
    int start = data.find("StartFragment:");
    int end = data.find("EndFragment:");
    if( start != -1 )
	start = data.mid( start+14, 10 ).toInt();
    if( end != -1 )
	end = data.mid( end+12, 10 ).toInt();
    if ( end > start && start > 0 ) {
	result = "<!--StartFragment-->" + data.mid( start, end - start );
	result += "<!--EndFragment-->";
	result.replace( "\r", "" );
	result.replace("<o:p>", "" );
	result.replace("</o:p>", "" );
    }
#endif
    return result;
}

extern QTextCodec* findcharset(const QCString& mimetype);

QByteArray QWindowsMimeHtml::convertFromMime( QByteArray _data, const char* mime, int cf )
{
    QCString data( _data );
    QCString result = 
	"Version 1.0\r\n"		    // 0-12
	"StartHTML:0000000105\r\n"	    // 13-35
	"EndHTML:0000000000\r\n"	    // 36-55
	"StartFragment:0000000000\r\n"	    // 58-86
	"EndFragment:0000000000\r\n\r\n";   // 87-105
	
    if ( data.find( "<!--StartFragment-->" ) == -1 )
	result += "<!--StartFragment-->";
    result += data.data();
    if ( data.find( "<!--EndFragment-->" ) == -1 )
	result += "<!--EndFragment-->";

    // set the correct number for EndHTML
    QCString pos = QString::number( result.size() ).latin1();
    memcpy( (char *)(result.data() + 53 - pos.length()), pos.data(), pos.length() );

    // set correct numbers for StartFragment and EndFragment
    pos = QString::number( result.find( "<!--StartFragment-->" ) + 20 ).latin1();
    memcpy( (char *)(result.data() + 79 - pos.length()), pos.data(), pos.length() );
    pos = QString::number( result.find( "<!--EndFragment-->" ) ).latin1();
    memcpy( (char *)(result.data() + 103 - pos.length()), pos.data(), pos.length() );
    qDebug("text is:\n%s\n", result.data() );
    return result;
}

#endif

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

class QWindowsMimeUri : public QWindowsMime {
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
    if ( qstricmp(mime,"text/uri-list")==0 )
	return CF_HDROP;
    else
	return 0;
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

    QCString texturi;

    int i=0;
    if ( hdrop->fWide ) {
	while ( filesw[i] ) {
	    QString fn = QString::fromUcs2( filesw+i );
	    texturi += QUriDrag::localFileToUri(fn);
	    texturi += "\r\n";
	    i += fn.length()+1;
	}
    } else {
	while ( files[i] ) {
	    QString fn = QString::fromLocal8Bit( files+i );
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
	QT_WA( {
	    size += sizeof(TCHAR)*( (*i).length()+1 );
	} , {
	    size += (*i).local8Bit().length()+1;
	} );
    }

    QByteArray result( size );
    DROPFILES* d = (DROPFILES*)result.data();
    d->pFiles = sizeof(DROPFILES);
    GetCursorPos(&d->pt); // try
    d->fNC = TRUE;
    char* files = ((char* )d) + d->pFiles;

    QT_WA( {
	d->fWide = TRUE;
	TCHAR* f = (TCHAR*)files;

	for ( i = fn.begin(); i!=fn.end(); ++i ) {
	    int l = (*i).length();
	    memcpy(f, (*i).ucs2(), l*sizeof(TCHAR));
	    for (int j = 0; j<l; j++)
		if ( f[j] == '/' )
		    f[j] = '\\';
	    f += l;
	    *f++ = 0;
	}
	*f = 0;
    } , {
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
    } );
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
#if 0
	new QWindowsMimeHtml;
#endif
	new QWindowsMimeUri;
	new QWindowsMimeAnyMime;
	qAddPostRoutine(cleanup_mimes);
    }
}

/*!
  Returns the most-recently created QWindowsMime that can convert
  between the \a mime and \a cf formats.  Returns 0 if no such convertor
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
QPtrList<QWindowsMime> QWindowsMime::all()
{
    return mimes;
}

/*!
  \fn const char* QWindowsMime::convertorName()

  Returns a name for the convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QWindowsMime::countCf()

  Returns the number of Windows Clipboard formats supported by this
  convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QWindowsMime::cf(int index)

  Returns the Windows Clipboard format supported by this convertor
  that is ordinarily at position \a index. This means that cf(0)
  returns the first Windows Clipboard format supported, and
  cf(countCf()-1) returns the last. If \a index is out of range the
  return value is undefined.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QWindowsMime::canConvert( const char* mime, int cf )

  Returns TRUE if the convertor can convert (both ways) between
  \a mime and \a cf; otherwise returns FALSE.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn const char* QWindowsMime::mimeFor(int cf)

  Returns the MIME type used for Windows Clipboard format \a cf, or
  0 if this convertor does not support \a cf.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QWindowsMime::cfFor(const char* mime)

  Returns the Windows Clipboard type used for MIME type \a mime, or
  0 if this convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QByteArray QWindowsMime::convertToMime( QByteArray data, const char* mime, int cf )

  Returns \a data converted from Windows Clipboard format \a cf
    to MIME type \a mime.

  Note that Windows Clipboard formats must all be self-terminating.  The
  input \a data may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QByteArray QWindowsMime::convertFromMime( QByteArray data, const char* mime, int cf )

  Returns \a data converted from MIME type \a mime
    to Windows Clipboard format \a cf.

  Note that Windows Clipboard formats must all be self-terminating.  The
  return value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

#endif // QT_NO_MIME
