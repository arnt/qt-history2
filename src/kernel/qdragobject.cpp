/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.cpp#94 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtextcodec.h"
#include "qdragobject.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qimage.h"
#include "qbuffer.h"
#include "qgif.h"
#include <ctype.h>


// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

struct QDragData {
    QDragData(): autoDelete( TRUE ), hot(0,0) {}
    bool autoDelete;
    QPixmap pixmap;
    QPoint hot;
    QWidget* target;
};

/*!
  After the drag completes, this function will return the QWidget
  which received the drop, or 0 if the data was dropped on some other
  program.

  This can be useful for detecting the case where drag-and-drop is to
  and from the same widget.
*/
QWidget * QDragObject::target()
{
    return d->target;
}

/*!
  \internal
  Sets the target.
*/
void QDragObject::setTarget(QWidget* t)
{
    d->target = t;
}

struct QStoredDragData {
    QStoredDragData() {}
    char* fmt;
    QByteArray enc;
};


// These pixmaps approximate the images in the Windows User Interface Guidelines.

/* XPM */
static const char * move_xpm[] = {
"11 20 3 1",
".	c None",
#if defined(_WS_WIN_)
" 	c #000000", // Windows cursor is traditionally white
"X	c #FFFFFF",
#else
" 	c #FFFFFF", // X11 cursor is traditionally white
"X	c #000000",
#endif
"  .........",
" X ........",
" XX .......",
" XXX ......",
" XXXX .....",
" XXXXX ....",
" XXXXXX ...",
" XXXXXXX ..",
" XXXXXXXX .",
" XXXXXXXXX ",
" XXXXXX    ",
" XXX XX ...",
" XX  XX ...",
" X .. XX ..",
"  ... XX ..",
" ..... XX .",
"...... XX .",
"....... XX ",
"....... XX ",
"........  ."};

/* XPM */
static const char * copy_xpm[] = {
"24 30 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
#if defined(_WS_WIN_) // Windows cursor is traditionally white
"  ......................",
" X .....................",
" XX ....................",
" XXX ...................",
" XXXX ..................",
" XXXXX .................",
" XXXXXX ................",
" XXXXXXX ...............",
" XXXXXXXX ..............",
" XXXXXXXXX .............",
" XXXXXX    .............",
" XXX XX ................",
" XX  XX ................",
" X .. XX ...............",
"  ... XX ...............",
" ..... XX ..............",
"...... XX ..............",
"....... XX .............",
"....... XX .............",
"........  ...           ",
#else
"XX......................",
"X X.....................",
"X  X....................",
"X   X...................",
"X    X..................",
"X     X.................",
"X      X................",
"X       X...............",
"X        X..............",
"X         X.............",
"X      XXXX.............",
"X   X  X................",
"X  XX  X................",
"X X..X  X...............",
"XX...X  X...............",
"X.....X  X..............",
"......X  X..............",
".......X  X.............",
".......X  X.............",
"........XX...           ",
#endif
"............. XXXXXXXXX ",
"............. XXXXXXXXX ",
"............. XXXX XXXX ",
"............. XXXX XXXX ",
"............. XX     XX ",
"............. XXXX XXXX ",
"............. XXXX XXXX ",
"............. XXXXXXXXX ",
"............. XXXXXXXXX ",
".............           "};

/* XPM */
static const char * link_xpm[] = {
"24 30 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
#if defined(_WS_WIN_) // Windows cursor is traditionally white
"  ......................",
" X .....................",
" XX ....................",
" XXX ...................",
" XXXX ..................",
" XXXXX .................",
" XXXXXX ................",
" XXXXXXX ...............",
" XXXXXXXX ..............",
" XXXXXXXXX .............",
" XXXXXX    .............",
" XXX XX ................",
" XX  XX ................",
" X .. XX ...............",
"  ... XX ...............",
" ..... XX ..............",
"...... XX ..............",
"....... XX .............",
"....... XX .............",
"........  ...           ",
#else
"XX......................",
"X X.....................",
"X  X....................",
"X   X...................",
"X    X..................",
"X     X.................",
"X      X................",
"X       X...............",
"X        X..............",
"X         X.............",
"X      XXXX.............",
"X   X  X................",
"X  XX  X................",
"X X..X  X...............",
"XX...X  X...............",
"X.....X  X..............",
"......X  X..............",
".......X  X.............",
".......X  X.............",
"........XX...           ",
#endif
"............. XXXXXXXXX ",
"............. XXX    XX ",
"............. XXXX   XX ",
"............. XXX    XX ",
"............. XX   X XX ",
"............. XX  XXXXX ",
"............. XX XXXXXX ",
"............. XXX XXXXX ",
"............. XXXXXXXXX ",
".............           "};


// the universe's only drag manager
static QDragManager * manager = 0;


QDragManager::QDragManager()
    : QObject( qApp, "global drag manager" )
{
    n_cursor = 3;
    pm_cursor = new QPixmap[n_cursor];
    pm_cursor[0] = QPixmap(move_xpm);
    pm_cursor[1] = QPixmap(copy_xpm);
    pm_cursor[2] = QPixmap(link_xpm);
    object = 0;
    dragSource = 0;
    dropWidget = 0;
    if ( !manager )
	manager = this;
    beingCancelled = FALSE;
    restoreCursor = FALSE;
    willDrop = FALSE;
}


QDragManager::~QDragManager()
{
    if ( restoreCursor )
	QApplication::restoreOverrideCursor();
    manager = 0;
    delete [] pm_cursor;
}




/*!  Creates a drag object which is a child of \a dragSource and
  named \a name.

  Note that the drag object will be deleted when \a dragSource is.
*/

QDragObject::QDragObject( QWidget * dragSource, const char * name )
    : QObject( dragSource, name )
{
    d = new QDragData();
    if ( !manager && qApp )
	(void)new QDragManager();
}


/*!  Deletes the drag object and frees up the storage used. */

QDragObject::~QDragObject()
{
    d->autoDelete = FALSE; // so cancel() won't delete this object
    if ( manager && manager->object == this )
	manager->cancel();
    delete d;
}

/*!
  Set the pixmap \a pm to display while dragging the object.
  The platform-specific
  implementation will use this in a loose fashion - so provide a small masked
  pixmap, but do not require that the user ever sees it in all its splendor.
  In particular, cursors on Windows 95 are of limited size.

  The \a hotspot is the point on (or off) the pixmap that should be under the
  cursor as it is dragged. It is relative to the top-left pixel of the pixmap.
*/
void QDragObject::setPixmap(QPixmap pm, QPoint hotspot)
{
    d->pixmap = pm;
    d->hot = hotspot;
    if ( manager && manager->object == this )
	manager->updatePixmap();
}

/*!
  \overload
  Uses a hotspot that positions the pixmap below and to the
  right of the mouse pointer.  This allows the user to clearly
  see the point on the window which they are dragging the data onto.
*/
void QDragObject::setPixmap(QPixmap pm)
{
    setPixmap(pm,QPoint(-10,-10));
}

/*!
  Returns the currently set pixmap
  (which \link QPixmap::isNull() isNull()\endlink if none is set).
*/
QPixmap QDragObject::pixmap() const
{
    return d->pixmap;
}

/*!
  Returns the currently set pixmap hitspot.
*/
QPoint QDragObject::pixmapHotSpot() const
{
    return d->hot;
}

/*!
  Starts a drag operation using the contents of this object,
  using DragDefault mode.

  The function returns TRUE if the caller should delete the
  original copy of the dragged data (but also note target()).

  Note that if the drag contains \e references to information
  (eg. file names is a QUriDrag are references)
  then the return value should always be ignored, as the target
  is expected to manipulate the referred-to content directly.
  On X11 the return value should always be correct anyway, but
  on Windows this is not necessarily the case (eg. the file manager
  starts a background process to move files, so the source 
  <em>must not</em> delete the files!)
*/
bool QDragObject::drag()
{
    return drag( DragDefault );
}


/*!
  Starts a drag operation using the contents of this object,
  using DragMove mode.
*/
bool QDragObject::dragMove()
{
    return drag( DragMove );
}


/*!
  Starts a drag operation using the contents of this object,
  using DragCopy mode.

  See drag(DragMove) for important further information.
*/
void QDragObject::dragCopy()
{
    (void)drag( DragCopy );
}


/*!
  Starts a drag operation using the contents of this object.

  At this point, the object becomes owned by Qt, not the
  application.  You should not delete the drag object nor
  anything it references.  The actual transfer of data to
  the target application will be done during future event
  processing - after that time the drag object will be deleted.

  Returns TRUE if the dragged data was dragged as a \e move,
  indicating that the caller should remove the original source
  of the data (the drag object must continue to have a copy).

  \define DragMode

  The \a mode is one of:

  <ul>
   <li>\c DragDefault - the mode is determined heuristically.
   <li>\c DragCopy - the data is copied, never moved.
   <li>\c DragMove - the data is moved, if dragged at all.
   <li>\c DragCopyOrMove - the user chooses the mode
	    by using control key to switch from the default.
  </ul>

  Normally one of simpler drag(), dragMove(), or dragCopy() functions
  would be used instead.

  \warning in Qt 1.x, drag operations all return FALSE.  This will change
	    in later versions - the functions are provided in this way to
	    assist preemptive development - code both move and copy with
	    Qt 1.x to be prepared.
*/
bool QDragObject::drag(DragMode mode)
{
    if ( manager )
	return manager->drag( this, mode );
    else
	return FALSE;
}



/*!  Returns a pointer to the drag source where this object originated.
*/

QWidget * QDragObject::source()
{
    if ( parent()->isWidgetType() )
	return (QWidget *)parent();
    else
	return 0;
}


/*! \class QDragObject qdragobject.h

  \brief The QDragObject encapsulates MIME-based information transfer.

  \ingroup kernel

  QDragObject is the base class for all data that needs to be transferred
  between and within applications, both for drag-and-drop and for the
  clipboard.

  See the \link dnd.html Drag-and-drop documentation\endlink for
  an overview of how to provide drag-and-drop in your application.

  See the QClipboard documentation for
  an overview of how to provide cut-and-paste in your application.
*/

static
void stripws(QCString& s)
{
    int f;
    while ( (f=s.find(' ')) >= 0 )
	s.remove(f,1);
}

static
const char * staticCharset(int i)
{
    static QCString localcharset;

    switch ( i ) {
      case 0:
	return "utf8";
      case 1:
	if ( localcharset.isNull() ) {
	    localcharset = QTextCodec::codecForLocale()->name();
	    localcharset = localcharset.lower();
	    stripws(localcharset);
	}
	return localcharset;
      case 2:
	return "utf16";
      case 3:
	return "";
    }
    return 0;
}


class QTextDragPrivate {
public:
    QTextDragPrivate()
    {
	setSubType("plain");
    }

    enum { nfmt=4 };

    QString txt;
    QCString fmt[nfmt];
    QCString subtype;

    void setSubType(const QCString & st)
    {
	subtype = st.lower();
	for ( int i=0; i<nfmt; i++ ) {
	    fmt[i] = "text/";
	    fmt[i].append(subtype);
	    QCString cs = staticCharset(i);
	    if ( !cs.isEmpty() ) {
		fmt[i].append(";charset=");
		fmt[i].append(cs);
	    }
	}
    }
};

void QTextDrag::setSubtype( const QCString & st)
{
    d->setSubType(st);
}

/*! \class QTextDrag qdragobject.h

  \brief The QTextDrag provides a drag-and-drop object for
	      transferring plain and Unicode text.

  \ingroup kernel

  Plain text is single- or multi-line 8-bit text in the local encoding.

  Qt provides no built-in mechanism for delivering only single-line.

  Drag&Drop text does \e not have a NUL terminator when it
  is dropped onto the target.

  For detailed information about drag-and-drop, see the QDragObject class.
*/


/*!  Creates a text drag object and sets it to \a text.  \a dragSource
  must be the drag source, \a name is the object name. */

QTextDrag::QTextDrag( const QString &text,
		      QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    d = new QTextDragPrivate;
    setText( text );
}


/*!  Creates a default text drag object.  \a dragSource must be the drag
  source, \a name is the object name.
*/

QTextDrag::QTextDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    d = new QTextDragPrivate;
}


/*!  Destroys the text drag object and frees all allocated resources.
*/
QTextDrag::~QTextDrag()
{
    delete d;
}


/*!
  Sets the text to be dragged.  You will need to call this if you did
  not pass the text during construction.
*/
void QTextDrag::setText( const QString &text )
{
    d->txt = text;
}


const char * QTextDrag::format(int i) const
{
    if ( i >= d->nfmt )
	return 0;
    return d->fmt[i];
}

static
QTextCodec* findcharset(const QCString& mimetype)
{
    int i=mimetype.find("charset=");
    if ( i >= 0 ) {
	QCString cs = mimetype.mid(i+8);
	stripws(cs);
	i = cs.find(';');
	if ( i >= 0 )
	    cs = cs.left(i);
	// May return 0 if unknown charset
	return QTextCodec::codecForName(cs,cs.length()*3/4);
    }
    // no charset=, use locale
    return QTextCodec::codecForLocale();
}

static
QTextCodec* findcodec(const QMimeSource* e)
{
    QTextCodec* r;
    const char* f;
    int i;
    for ( i=0; (f=e->format(i)); i++ )
	if ( (r = findcharset(QCString(f).lower())) )
	    return r;
    return 0;
}


QByteArray QTextDrag::encodedData(const char* mime) const
{
    QCString r;
    if ( 0==strnicmp(mime,"text/",5) ) {
	QCString m(mime);
	m = m.lower();
	QTextCodec *codec = findcharset(m);
	if ( !codec )
	    return r;
	r = codec->fromUnicode(d->txt);
	if (!codec || codec->mibEnum() != 1000) {
	    // Don't include NUL in size (QCString::resize() adds NUL)
	    ((QByteArray&)r).resize(r.length());
	}
    }
    return r;
}

/*!
  Returns TRUE if the information in \a e can be decoded into a QString.
  \sa decode()
*/
bool QTextDrag::canDecode( const QMimeSource* e )
{
    const char* f;
    for (int i=0; (f=e->format(i)); i++) {
	if ( 0==strnicmp(f,"text/",5) ) {
	    return findcodec(e) != 0;
	}
    }
    return 0;
}

/*!
  Attempts to decode the dropped information in \a e
  into \a str, returning TRUE if successful.  If \a subtype is null,
  any text subtype is accepted, otherwise only that specified is
  accepted.  \a subtype is set to the accepted subtype.

  \sa canDecode()
*/
bool QTextDrag::decode( const QMimeSource* e, QString& str, QCString& subtype )
{
    const char* mime;
    for (int i=0; (mime = e->format(i)); i++) {
	if ( 0==strnicmp(mime,"text/",5) ) {
	    QCString m(mime);
	    m = m.lower();
	    int semi = m.find(';');
	    if ( semi < 0 )
		semi = m.length();
	    QCString foundst = m.mid(5,semi-5);
	    if ( subtype.isNull() || foundst == subtype ) {
		QTextCodec* codec = findcharset(m);
		if ( codec ) {
		    QByteArray payload;

		    payload = e->encodedData(mime);
		    if ( payload.size() ) {
			int l;
			if ( codec->mibEnum() != 1000) {
			    // length is at NUL or payload.size()
			    l = 0;
			    while ( l < (int)payload.size() && payload[l] )
				l++;
			} else {
			    l = payload.size();
			}

			str = codec->toUnicode(payload,l);

			if ( subtype.isNull() )
			    subtype = foundst;

			return TRUE;
		    }
		}
	    }
	}
    }
    return FALSE;
}

/*!
  Attempts to decode the dropped information in \a e
  into \a str, returning TRUE if successful.

  \sa canDecode()
*/
bool QTextDrag::decode( const QMimeSource* e, QString& str )
{
    QCString st;
    return decode(e,str,st);
}


/*
  QImageDrag could use an internal MIME type for communicating QPixmaps
  and QImages rather than always converting to raw data.  This is available
  for that purpose and others.  It is not currently used.
*/
class QImageDragData {
public:
};


/*! \class QImageDrag qdragobject.h

  \brief The QImageDrag provides a drag-and-drop object for
  transferring images.

  \ingroup kernel

  Images are offered to the receiving application in multiple formats,
  determined by the \link QImage::outputFormats() output formats\endlink
  in Qt.

  For detailed information about drag-and-drop, see the QDragObject class.
*/

/*!  Creates an image drag object and sets it to \a image.  \a dragSource
  must be the drag source, \a name is the object name. */

QImageDrag::QImageDrag( QImage image,
			QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name ),
	d(0)
{
    setImage( image );
}

/*!  Creates a default text drag object.  \a dragSource must be the drag
  source, \a name is the object name.
*/

QImageDrag::QImageDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name ),
	d(0)
{
}


/*!  Destroys the image drag object and frees all allocated resources.
*/

QImageDrag::~QImageDrag()
{
    // nothing
}


/*!
  Sets the image to be dragged.  You will need to call this if you did
  not pass the image during construction.
*/
void QImageDrag::setImage( QImage image )
{
    img = image; // ### detach?
    ofmts = QImage::outputFormats();
    ofmts.remove("PBM"); // remove non-raw PPM
    if ( image.depth()!=32 ) {
	// BMP better than PPM for paletted images
	if ( ofmts.remove("BMP") ) // move to front
	    ofmts.insert(0,"BMP");
    }
    // PNG is best of all
    if ( ofmts.remove("PNG") ) // move to front
	ofmts.insert(0,"PNG");
}

const char * QImageDrag::format(int i) const
{
    if ( i < (int)ofmts.count() ) {
	static QCString str;
	str.sprintf("image/%s",(((QImageDrag*)this)->ofmts).at(i));
	str = str.lower();
	if ( str == "image/pbmraw" )
	    str = "image/ppm";
	return str;
    } else {
	return 0;
    }
}

QByteArray QImageDrag::encodedData(const char* fmt) const
{
    if ( qstrnicmp( fmt, "image/", 6 )==0 ) {
	QCString f = fmt+6;
	QByteArray data;
	QBuffer w( data );
	w.open( IO_WriteOnly );
	QImageIO io( &w, f.upper() );
	io.setImage( img );
	if  ( !io.write() )
	    return QByteArray();
	w.close();
	return data;
    } else {
	return QByteArray();
    }
}

/*!
  Returns TRUE if the information in \a e can be decoded into an image.
  \sa decode()
*/
bool QImageDrag::canDecode( const QMimeSource* e )
{
    return e->provides( "image/png" )
        || e->provides( "image/ppm" )
        || e->provides( "image/bmp" )
        || ( qt_builtin_gif_reader() && e->provides( "image/gif" ) );
    // ### more Qt images types
}

/*!
  Attempts to decode the dropped information in \a e
  into \a img, returning TRUE if successful.

  \sa canDecode()
*/
bool QImageDrag::decode( const QMimeSource* e, QImage& img )
{
    QByteArray payload = e->encodedData( "image/png" );
    if ( payload.isEmpty() )
	payload = e->encodedData( "image/ppm" );
    if ( payload.isEmpty() )
	payload = e->encodedData( "image/bmp" );
    if ( payload.isEmpty() ) // if we get gif, try it even if !builtin
	payload = e->encodedData( "image/gif" );
    // ### more Qt images types
    if ( payload.isEmpty() )
	return FALSE;

    img.loadFromData(payload);
    return !img.isNull();
}

/*!
  Attempts to decode the dropped information in \a e
  into \a pm, returning TRUE if successful.

  This is a convenience function that converts
  to \a pm via a QImage.

  \sa canDecode()
*/
bool QImageDrag::decode( const QMimeSource* e, QPixmap& pm )
{
    QImage img;
    // We avoid dither, since the image probably came from this display
    if ( decode( e, img ) )
	return pm.convertFromImage( img, AvoidDither );
    return FALSE;
}




/*!
  \class QStoredDrag qdragobject.h
  \brief Simple stored-value drag object for arbitrary MIME data.

  When a block of data only has one representation, you can use
  a QStoredDrag to hold it.

  For detailed information about drag-and-drop, see the QDragObject class.
*/

/*!
  Constructs a QStoredDrag.  The parameters are passed
  to the QDragObject constructor, and the format is set to \a mimeType.

  The data will be unset.  Use setEncodedData() to set it.
*/
QStoredDrag::QStoredDrag( const char* mimeType, QWidget * dragSource, const char * name ) :
    QDragObject(dragSource,name)
{
    d = new QStoredDragData();
    d->fmt = qstrdup(mimeType);
}

/*!
  Destroys the drag object and frees all allocated resources.
*/
QStoredDrag::~QStoredDrag()
{
    delete d->fmt;
    delete d;
}

const char * QStoredDrag::format(int i) const
{
    if ( i==0 )
	return d->fmt;
    else
	return 0;
}


/*!
  Sets the encoded data of this drag object to \a encodedData.  The
  encoded data is what's delivered to the drop sites, and must be in a
  strictly defined and portable format.

  The drag object can't be dropped (by the user) until this function
  has been called.
*/

void QStoredDrag::setEncodedData( const QByteArray & encodedData )
{
    d->enc = encodedData.copy();
}

/*!
  Returns the stored data.

  \sa setEncodedData()
*/
QByteArray QStoredDrag::encodedData(const char* m) const
{
    if ( !qstricmp(m,d->fmt) )
	return d->enc;
    else
	return QByteArray();
}


/*!
  \class QUriDrag qdragobject.h
  \brief Provides for drag-and-drop of a list of URI references.

  URIs are a useful way to refer to files that may be distributed
  across multiple machines.  Much of the time a URI will refer to
  a file on a machine local to both the drag source and the
  drop target, and so the URI will be equivalent to passing a
  filename, but more extensible.

  While presenting URIs to the user, use them in Unicode form so
  that the user can confortably edit and view them.
  For use in HTTP or other protocols, use the correctly escaped
  ASCII form (see
*/

/*!
  Creates an object to drag the list of URIs in \a uris.
  The \a dragSource and \a name arguments are passed on to
  QStoredDrag.  Note that URIs are always in escaped UTF8
  encoding, as defined by the W3C.
*/
QUriDrag::QUriDrag( QStrList uris,
	    QWidget * dragSource, const char * name ) :
    QStoredDrag( "text/uri-list", dragSource, name )
{
    setUris(uris);
}

/*!
  Creates a object to drag.  You will need to call
  setUris() before you start the drag().
*/
QUriDrag::QUriDrag( QWidget * dragSource, const char * name ) :
    QStoredDrag( "text/uri-list", dragSource, name )
{
}

/*!
  Destroys the object.
*/
QUriDrag::~QUriDrag()
{
}

/*!
  Changes the list of \a uris to be dragged.
*/
void QUriDrag::setUris( QStrList uris )
{
    QByteArray a;
    int c=0;
    for ( const char* s = uris.first(); s; s = uris.next() ) {
	int l = strlen(s);
	a.resize(c+l+2);
	memcpy(a.data()+c,s,l);
	memcpy(a.data()+c+l,"\r\n",2);
	c+=l+2;
    }
    setEncodedData(a);
}


/*!
  Returns TRUE if decode() would be able to decode \a e.
*/
bool QUriDrag::canDecode( const QMimeSource* e )
{
    return e->provides( "text/uri-list" );
}

/*!
  Decodes URIs from \a e, placing the result in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URIs.
*/
bool QUriDrag::decode( const QMimeSource* e, QStrList& l )
{
    QByteArray payload = e->encodedData( "text/uri-list" );
    if ( payload.size() ) {
	l.clear();
	l.setAutoDelete(TRUE);
	uint c=0;
	char* d = payload.data();
	while (c < payload.size() && d[c]) {
	    uint f = c;
	    // Find line end
	    while (c < payload.size() && d[c] && d[c]!='\r'
		    && d[c] != '\n')
		c++;
	    QCString s(d+f,c-f+1);
	    if ( s[0] != '#' ) // non-comment?
		l.append( s );
	    // Skip junk
	    while (c < payload.size() && d[c] &&
		    (d[c]=='\n' || d[c]=='\r'))
		c++;
	}
	return TRUE;
    }
    return FALSE;
}

static
uint htod(int h)
{
    if (isdigit(h)) return h-'0';
    return tolower(h)-'a'+10;
}

/*!
  Sets the URIs to be the local-file URIs equivalent to \a fnames.

  \sa localFileToUri(), setUris()
*/
void QUriDrag::setFilenames( QStringList fnames )
{
    QStrList uris;
    for (QStringList::Iterator i = fnames.begin();
	    i != fnames.end(); ++i )
	uris.append(localFileToUri(*i));
    setUris(uris);
}

/*!
  Sets the URIs to be the
  Unicode URIs (only useful for
  displaying to humans) \a uuris.

  \sa localFileToUri(), setUris()
*/
void QUriDrag::setUnicodeUris( QStringList uuris )
{
    QStrList uris;
    for (QStringList::Iterator i = uuris.begin();
	    i != uuris.end(); ++i )
	uris.append(unicodeUriToUri(*i));
    setUris(uris);
}

/*!
  Returns the URI equivalent to the Unicode URI (only useful for
  displaying to humans).
  \a uuri.

  \sa uriToLocalFile()
*/
QCString QUriDrag::unicodeUriToUri(const QString& uuri)
{
    QCString utf8 = uuri.utf8();
    QCString escutf8;
    int n = utf8.length();
    for (int i=0; i<n; i++) {
	if ( utf8[i] >= 'a' && utf8[i] <= 'z'
	  || utf8[i] == '/'
	  || utf8[i] >= '0' && utf8[i] <= '9'
	  || utf8[i] >= 'A' && utf8[i] <= 'Z'

	  || utf8[i] == '-' || utf8[i] == '_'
	  || utf8[i] == '.' || utf8[i] == '!'
	  || utf8[i] == '~' || utf8[i] == '*'
	  || utf8[i] == '(' || utf8[i] == ')'
	  || utf8[i] == '\''

	  // Allow this through, so that all URI-references work.
	  || utf8[i] == '#'

	  || utf8[i] == ';'
	  || utf8[i] == '?' || utf8[i] == ':'
	  || utf8[i] == '@' || utf8[i] == '&'
	  || utf8[i] == '=' || utf8[i] == '+'
	  || utf8[i] == '$' || utf8[i] == ',' )
	{
	    escutf8 += utf8[i];
	} else {
	    // Everything else is escaped as %HH
	    QCString s(4);
	    s.sprintf("%%%02x",(uchar)utf8[i]);
	    escutf8 += s;
	}
    }
    return escutf8;
}

/*!
  Returns the URI equivalent to the absolute
  local file \a filename.

  \sa uriToLocalFile()
*/
QCString QUriDrag::localFileToUri(const QString& filename)
{
    QString r = filename;
#ifdef _WS_WIN_
    // Slosh -> Slash
    int slosh;
    while ( (slosh=r.find('\\')) >= 0 ) {
	r[slosh] = '/';
    }
    // Drive
    if ( r[0] != '/' ) {
	int colon = r.find(':');
	if ( colon >= 0 ) {
	    r[colon] = '|';
	    if ( r[colon+1] != '/' )
		r.insert(colon+1,'/');
	}
	r.insert(0,'/');
    }
#endif
    return unicodeUriToUri(QString("file://" + r));
}

/*!
  Returns the Unicode URI (only useful for
  displaying to humans) equivalent to \a uri.

  \sa localFileToUri()
*/
QString QUriDrag::uriToUnicodeUri(const char* uri)
{
    QCString utf8;

    while (*uri) {
	switch (*uri) {
	  case '%': {
		uint ch = uri[1];
		if ( ch && uri[2] ) {
		    ch = htod(ch)*16 + htod(uri[2]);
		    utf8 += char(ch);
		    uri += 2;
		}
	    }
	    break;
	  default:
	    utf8 += *uri;
	}
	++uri;
    }

    return QString::fromUtf8(utf8);
}

/*!
  Returns the name of a local file equivalent to \a uri,
  or a null string if \a uri is not a local file.

  \sa localFileToUri()
*/
QString QUriDrag::uriToLocalFile(const char* uri)
{
    QString file;

    if ( uri && 0==qstrnicmp(uri,"file:/",6) ) {
	uri += 6;
	if ( uri[0] != '/' || uri[1] == '/' ) {
	    // It is local.
	    file = uriToUnicodeUri(uri);
	    if ( uri[1] == '/' ) {
		file.remove(0,1);
	    } else {
		file.insert(0,'/');
	    }
#ifdef _WS_WIN_
	    if ( file.length() > 2 && file[0] == '/' && file[2] == '|' ) {
		file[2] = ':';
		file.remove(0,1);
	    }
	    // Leave slash as slashes.
#endif
	}
    }

    return file;
}

/*!
  Decodes URIs from \a e, converts them to local files if they refer to
  local files, and places them in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URIs.
  The list will be empty if no URIs were local files.
*/
bool QUriDrag::decodeLocalFiles( const QMimeSource* e, QStringList& l )
{
    QStrList u;
    if ( !decode( e, u ) )
	return FALSE;

    l.clear();
    for (const char* s=u.first(); s; s=u.next()) {
	QString lf = uriToLocalFile(s);
	if ( !lf.isNull() )
	    l.append( lf );
    }
    return TRUE;
}

/*!
  Decodes URIs from \a e, converts them to Unicode URIs (only useful for
  displaying to humans),
  placing them in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URIs.
*/
bool QUriDrag::decodeToUnicodeUris( const QMimeSource* e, QStringList& l )
{
    QStrList u;
    if ( !decode( e, u ) )
	return FALSE;

    l.clear();
    for (const char* s=u.first(); s; s=u.next())
	l.append( uriToUnicodeUri(s) );

    return TRUE;
}


/*!
  If the source of the drag operation is a widget in this application,
  this function returns that source, otherwise 0.  The source of the
  operation is the first parameter to to drag object subclass.

  This is useful if your widget needs special behavior when dragging
  to itelf, etc.

  See QDragObject::QDragObject() and subclasses.
*/
QWidget* QDropEvent::source() const
{
    return manager ? manager->dragSource : 0;
}
