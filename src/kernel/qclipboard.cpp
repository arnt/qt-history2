/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard.cpp#30 $
**
** Implementation of QClipboard class
**
** Created : 960430
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qdragobject.h"
#include "qpixmap.h"

#if defined(QT_MAKEDLL)
#define QApplication QBaseApplication
#endif

// REVISED: arnt

/*!
  \class QClipboard qclipboard.h
  \brief The QClipboard class provides access to the window system clipboard.

  \ingroup environment

  The clipboard offers a simple mechanism to copy and paste data between
  applications.

  QClipboard supports the same data types that QDragObject does, and uses
  similar mechanisms.

  Only there is a single QClipboard object in an application, and you
  can gain access to it using QApplication::clipboard().

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

  QClipboard features some convenience functions to access common data
  types: The methods setText() allows exchanging unicode text easily
  over the clipboard, while setPixmap() setImage() allows to exchange
  QPixmap and QImage between applications.  setData() is the ultimate
  in flexibility:  It allows you to add any QMimeSource onto the
  clipboard.  (There are corresponding getters for each of these,
  e.g. text().)

  You can clear the clipboard by calling the method clear().
*/


/*!
  Constructs a clipboard object.

  Note that only QApplication should do this. Call
  QApplication::clipboard() to get a pointer to the application global
  clipboard object.

  There is only one clipboard in the window system, and creating more
  than one object to represent it is almost certainly a bug.
*/

QClipboard::QClipboard( QObject *parent, const char *name )
    : QObject( parent, name )
{
    // nothing
}

/*!
  Destructs the clipboard.

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
  Returns the clipboard text in subtype \a subtype,
  or a null string if the clipboard does not contain any text.
  If \a subtype is null, any subtype is acceptable, and \a subtype
  is set to the chosen subtype.

  Common values for \a subtype are "plain" and "html".

  \sa setText() data(), QString::operator!()
*/

QString QClipboard::text(QCString& subtype) const
{
    QString r;
    QTextDrag::decode( data() ,r, subtype );
    return r;
}

/*!
  Returns the clipboard as plain text, or a null string
  if the clipboard does not contain any text.

  \sa setText() data(), QString::operator!()
*/

QString QClipboard::text() const
{
    QCString subtype = "plain";
    return text(subtype);
}

/*!
  Copies \a text into the clipboard.
  \sa text() setData()
*/

void QClipboard::setText( const QString &text )
{
    setData( new QTextDrag(text) );
}


/*!
  Returns the clipboard image, or a null image if the clipboard does
  not contain an image, or if it contains an image in an unsupported
  image format.

  \sa setImage() pixmap() data(), QImage::isNull()
*/

QImage QClipboard::image() const
{
    QImage r;
    QImageDrag::decode( data(), r );
    return r;
}

/*!
  Copies \a image into the clipboard.

  This is shorthand for:
  \code
    setData(new QImageDrag(image))
  \endcode

  \sa image(), setPixmap() setData()
*/

void QClipboard::setImage( const QImage &image )
{
    setData( new QImageDrag( image ) );
}


/*!
  Returns the clipboard pixmap, or null if the clipboard does not
  contain any pixmap. Note that this can lose information - for
  example, if the image is 24-bit and the display 8-bit the result is
  converted to 8 bits, and if the image has an alpha channel the
  result just has a mask.

  \sa setPixmap() image() data() QPixmap::convertFromImage().
*/

QPixmap QClipboard::pixmap() const
{
    QPixmap r;
    QImageDrag::decode( data(), r );
    return r;
}

/*!
  Copies \a pixmap into the clipboard.  Note that this is slower than
  setImage() - it needs to convert the QPixmap to a QImage first.

  \sa pixmap() setImage() setData()
*/

void QClipboard::setPixmap( const QPixmap &pixmap )
{
    // *could* just use the handle, but that is X hackery, MIME is better.
    setData( new QImageDrag( pixmap.convertToImage() ) );
}

#endif // QT_NO_CLIPBOARD
