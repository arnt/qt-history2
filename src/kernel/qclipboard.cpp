/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard.cpp#30 $
**
** Implementation of QClipboard class
**
** Created : 960430
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

#include "qclipboard.h"
#include "qapplication.h"
#include "qdragobject.h"
#include "qpixmap.h"

#if defined(QT_MAKEDLL)
#define QApplication QBaseApplication
#endif

// BEING REVISED: weis
/*!
  \class QClipboard qclipboard.h
  \brief The QClipboard class provides access to the window systems clipboard.

  \ingroup kernel

  The clipboard offers a simple mechanism to copy and paste data between
  applications.

  QClipboard supports the same data types that
  \link QDragObject drag and drop\endlink supports, and uses much
  of the same mechanisms.

  Only a single QClipboard object may exist in an application. This is
  because QClipboard is a shared window system resource.  Call
  QApplication::clipboard() to access the clipboard.

  Example:
  \code
    QClipboard *cb = QApplication::clipboard();
    QString text;

    // Copy text from the clipboard (paste)
    text = cb->text();
    if ( text )
	qDebug( "The clipboard contains: %s", text );

    // Copy text into the clipboard
    cb->setText( "This text can be pasted by other programs" );
  \endcode
  
  QClipboard features some convenience functions to access common data types.
  The methods setText() and text() allow to exchange unicode text easily over
  the clipboard, while setPixmap(), setImage() and pixmap(), image() allow
  to exchange QPixmap and QImage between applications.
  
  The most flexible methods are data() and setData(). They allow to put
  a QMimeSource() on the clipboard or retrieve it from the clipboard.
  This does not only allow you to put all kind of data type on the clipboard.
  In addition it allows you to exchange some information using different data types.
  For example you want to put a sound on the clipboard. Since
  you can not know exactly what kind of formats the other application understands
  you can feature multiple formats at once. This functionality is provided by
  QMimeSource. The application which retrieves the data from the clipboard receives
  a QMimeSource, too, and can select one of the offered data types.
  
  You can clear the clipboard by calling the method clear().
*/


/*!
  Constructs a clipboard object.

  Note that only QApplication is allowed to do this. Call
  QApplication::clipboard() to get a pointer to the application global
  clipboard object.
*/

QClipboard::QClipboard( QObject *parent, const char *name )
    : QObject( parent, name )
{
    // nothing
}

/*!
  Destroys the clipboard.

  You should never delete the clipboard. QApplication will do this when
  the application terminates.
*/

QClipboard::~QClipboard()
{
}


/*!
  \fn void QClipboard::dataChanged()
  This signal is emitted when the clipboard data is changed.
*/


/*****************************************************************************
  QApplication member functions related to QClipboard.
 *****************************************************************************/

extern QObject *qt_clipboard;			// defined in qapp_xyz.cpp

static void cleanupClipboard()
{
    delete qt_clipboard;
    qt_clipboard = 0;
}

/*!
  Returns a pointer to the application global clipboard.
*/

QClipboard *QApplication::clipboard()
{
    if ( qt_clipboard == 0 ) {
	qt_clipboard = new QClipboard;
	CHECK_PTR( qt_clipboard );
	qAddPostRoutine( cleanupClipboard );
    }
    return (QClipboard *)qt_clipboard;
}


/*!
  Returns the clipboard text, or a
  \link QString::operator!() null string\endlink
  if the clipboard does not contain any text.
  
  \sa setText() data()
*/

QString QClipboard::text() const
{
    QString r;
    QTextDrag::decode(data(),r);
    return r;
}

/*!
  Copies \e text into the clipboard.
  \sa text() setData()
*/

void QClipboard::setText( const QString &text )
{
    setData(new QTextDrag(text));
}


/*!
  Returns the clipboard image, or a null image if the clipboard does not contain
  an image. In addition a null image may be returned if Qt does not
  understand the provided image format.
  
  \sa setImage() pixmap() data()
*/

QImage QClipboard::image() const
{
    QImage r;
    QImageDrag::decode(data(),r);
    return r;
}

/*!
  Copies \e image into the clipboard.

  This is just a shorthand for:
  \code
    setData(new QImageDrag(image))
  \endcode

  \sa image(), setPixmap() setData()
*/

void QClipboard::setImage( const QImage &image )
{
    setData(new QImageDrag(image));
}


/*!
  Returns the clipboard pixmap, or null if the clipboard does not contain
  any pixmap. Note that this usually looses more information than image().
  
  \sa setPixmap() image() data()
*/

QPixmap QClipboard::pixmap() const
{
    QPixmap r;
    QImageDrag::decode(data(),r);
    return r;
}

/*!
  Copies \e pixmap into the clipboard.
  Note that this usually looses more information than setImage(),
  as the data may be converted to an image for transfer.
  
  \sa pixmap() setImage() setData()
*/

void QClipboard::setPixmap( const QPixmap &pixmap )
{
    // *could* just use the handle, but that is X hackery, MIME is better.
    setData(new QImageDrag(pixmap.convertToImage()));
}


