/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.cpp#32 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdragobject.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qimage.h"
#include "qbuffer.h"


// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

struct QDragData {
    QDragData(): autoDelete( TRUE ) {}
    bool autoDelete;
};


struct QStoredDragData {
    QStoredDragData() {}
    QString fmt;
    QByteArray enc;
};



// the universe's only drag manager
static QDragManager * manager = 0;


QDragManager::QDragManager()
    : QObject( qApp, "global drag manager" )
{
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
  Starts a drag operation using the contents of this object.

  The drag will use DragDefault mode, whereby the copy or move
  will be determined by heuristics.

  Returns TRUE if the dragged data was dragged as a \e move,
  indicating that the caller should remove the data.

  It does not return until the drag is complete.
*/
bool QDragObject::drag()
{
    if ( manager )
	return manager->drag( this, DragDefault );
    else
	return FALSE;
}


/*!
  Starts a drag operation using the contents of this object.

  The drag will use DragMove mode.

  Returns TRUE if the dragged data was successfully moved,
  indicating that the caller should remove the data.

  It does not return until the drag is complete.
*/
bool QDragObject::dragMove()
{
    if ( manager )
	return manager->drag( this, DragMove );
    else
	return FALSE;
}


/*!
  Starts a drag operation using the contents of this object.

  The drag will use DragCopy mode.  The caller should not
  remove the data after the drag completes.

  It does not return until the drag is complete.
*/
void QDragObject::dragCopy()
{
    if ( manager )
	manager->drag( this, DragCopy );
}


/*!
  Starts a drag operation using the contents of this object.

  Normally one of simpler drag(), dragMove(), or dragCopy() functions
  would be used instead.

  It does not return until the drag is complete.
*/
bool QDragObject::drag(DragMode mode)
{
    if ( manager )
	return manager->drag( this, mode );
    else
	return FALSE;
}




/*!
  \fn QByteArray QDragObject::encodedData(const char*) const

  Returns the encoded payload of this object.  The drag manager
  calls this when the recipient needs to see the content of the drag;
  this generally doesn't happen until the actual drop.

  Subclasses must override this function.
*/



/*!
  Returns TRUE if the drag object can provide the data
  in format \a mimeType.  The default implementation
  iterates over format().
*/
bool QDragObject::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
	if ( !qstricmp(mimeType,fmt) )
	    return TRUE;
    }
    return FALSE;
}


/*!
  \fn const char * QDragObject::format(int i) const

  Returns the \e ith format, or NULL.
*/


/*!  Returns a pointer to the drag source where this object originated.
*/

QWidget * QDragObject::source()
{
    if ( parent()->isWidgetType() )
	return (QWidget *)parent();
    else
	return 0;
}


/*! \class QTextDrag qdragobject.h

  \brief The QTextDrag provides a drag and drop object for
  transferring plain text.

  \ingroup kernel

  Plain text is defined as single- or multi-line US-ASCII or an
  unspecified 8-bit character set.

  Qt provides no built-in mechanism for delivering only single-line
  or only US-ASCII text.

  Drag&Drop text does \e not have a NUL terminator when it
  is dropped onto the target.
*/


/*!  Creates a text drag object and sets it to \a text.  \a parent
  must be the drag source, \a name is the object name. */

QTextDrag::QTextDrag( const char * text,
				  QWidget * parent, const char * name )
    : QStoredDrag( "text/plain", parent, name )
{
    setText( text );
}


/*!  Creates a default text drag object.  \a parent must be the drag
  source, \a name is the object name.
*/

QTextDrag::QTextDrag( QWidget * parent, const char * name )
    : QStoredDrag( "text/plain", parent, name )
{
}


/*!  Destroys the text drag object and frees all allocated resources.
*/

QTextDrag::~QTextDrag()
{
    // nothing
}


/*!
  Sets the text to be dragged.  You will need to call this if you did
  not pass the text during construction.
*/
void QTextDrag::setText( const char * text )
{
    int l = qstrlen(text);
    QByteArray tmp(l);
    memcpy(tmp.data(),text,l);
    setEncodedData( tmp );
}

/*!
  Returns TRUE if the information in \a e can be decoded into a QString.
  \sa decode()
*/
bool QTextDrag::canDecode( QDragMoveEvent* e )
{
    return e->provides( "text/plain" );
}

/*!
  Attempts to decode the dropped information in \a e
  into \a str, returning TRUE if successful.

  \sa decode()
*/
bool QTextDrag::decode( QDropEvent* e, QString& str )
{
    QByteArray payload = e->data( "text/plain" );
    if ( payload.size() ) {
	e->accept();
	str = QString( payload.data(), payload.size()+1 );
	return TRUE;
    }
    return FALSE;
}


/*! \class QImageDrag qdragobject.h

  \brief The QImageDrag provides a drag and drop object for
  tranferring images.

  \ingroup kernel

  Images are offered to the receiving application in multiple formats,
  determined by the \link QImage::outputFormats() output formats\endlink
  in Qt.
*/


/*!  Creates an image drag object and sets it to \a image.  \a parent
  must be the drag source, \a name is the object name. */

QImageDrag::QImageDrag( QImage image,
				  QWidget * parent, const char * name )
    : QDragObject( parent, name )
{
    setImage( image );
}

/*!  Creates a default text drag object.  \a parent must be the drag
  source, \a name is the object name.
*/

QImageDrag::QImageDrag( QWidget * parent, const char * name )
    : QDragObject( parent, name )
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
    img = image;
    // ### should detach?
    ofmts = QImage::outputFormats();
    ofmts.remove("PBM");
    if ( image.depth()!=32 ) {
	// BMP better than PPM for paletted images
	if ( ofmts.remove("BMP") ) // move to front
	    ofmts.insert(0,"BMP");
    }
    // Could do more magic to order mime types
}

const char * QImageDrag::format(int i) const
{
    if ( i < (int)ofmts.count() ) {
	static QString str;
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
	QString f = fmt+6;
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
bool QImageDrag::canDecode( QDragMoveEvent* e )
{
    return e->provides( "image/bmp" )
        || e->provides( "image/ppm" )
        || e->provides( "image/gif" );
    // ### more Qt images types
}

/*!
  Attempts to decode the dropped information in \a e
  into \a img, returning TRUE if successful.

  \sa canDecode()
*/
bool QImageDrag::decode( QDropEvent* e, QImage& img )
{
    QByteArray payload = e->data( "image/bmp" );
    if ( payload.isEmpty() )
	payload = e->data( "image/ppm" );
    if ( payload.isEmpty() )
	payload = e->data( "image/gif" );
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
bool QImageDrag::decode( QDropEvent* e, QPixmap& pm )
{
    QImage img;
    if ( decode( e, img ) )
	return pm.convertFromImage( img );
    return TRUE;
}




/*!
  \class QStoredDrag qdragobject.h
  \brief Simple stored-value drag object for arbitrary MIME data.

  When a block of data only has one representation, you can use
  a QStoredDrag to hold it.
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
    d->fmt = mimeType;
}

/*!
  Destroys the drag object and frees all allocated resources.
*/
QStoredDrag::~QStoredDrag()
{
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

void QStoredDrag::setEncodedData( QByteArray & encodedData )
{
    d->enc = encodedData;
    d->enc.detach();
}

/*!
  Returns the stored data.

  \sa setEncodedData()
*/
QByteArray QStoredDrag::encodedData(const char* m) const
{
    if ( m == d->fmt )
	return d->enc;
    else
	return QByteArray();
}


