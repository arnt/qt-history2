/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard.cpp#7 $
**
** Implementation of QClipboard class
**
** Created : 960430
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qclipbrd.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qclipboard.cpp#7 $");


/*!
  \class QClipboard qclipbrd.h
  \brief The QClipboard class provides access to the window system clipboard.

  The clipboard offers a simple mechanism to copy and paste data between
  applications.

  QClipboard supports these formats (a format is identified by a string):
  <ul>
  <li>"TEXT", zero-terminated char *.
  <li>"PIXMAP" as provided by QPixmap.
  </ul>

  The "PIXMAP" format is not implemented in this version of Qt.

  Only a single QClipboard object may exist in an application. This is
  because QClipboard is a shared window system resource. It is not
  possible to create a QClipboard object the standard C++ way (the
  constructor and destructor are private member functions, but accessible
  to QApplication since it is a friend class).	Call
  QApplication::clipboard() to access the clipboard.

  Example:
  \code
    QClipboard *cb = QApplication::clipboard();
    const char *text;

    // Copy text from the clipboard (paste)
    text = cb->text();
    if ( text )
	debug( "The clipboard contains: %s", text );

    // Copy text into the clipboard
    cb->setText( "This text can be pasted by other programs" );
  \endcode

  \warning
  It is an important GUI principle that all clipboard operations should be
  initiated by the user.
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
    initMetaObject();
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


/*!
  Returns the clipboard text, or null if the clipboard does not contains
  any text.
  \sa setText()
*/

const char *QClipboard::text() const
{
    return (const char *)data("TEXT");
}

/*!
  Copies \e text into the clipboard.
  \sa text()
*/

void QClipboard::setText( const char *text )
{
    setData( "TEXT", (void *)text );
}


/*!
  Returns the clipboard pixmap, or null if the clipboard does not contains
  any pixmap.
  \sa setText()
*/

QPixmap *QClipboard::pixmap() const
{
    return (QPixmap *)data("PIXMAP");
}

/*!
  Copies \e pixmap into the clipboard.
  \sa pixmap()
*/

void QClipboard::setPixmap( const QPixmap &pixmap )
{
    setData( "PIXMAP", (void *)&pixmap );
}


/*****************************************************************************
  QApplication member functions related to QClipboard.
 *****************************************************************************/

extern QObject *qt_clipboard;			// defined in qapp_xyz.cpp

/*!
  Returns a pointer to the application global clipboard.
*/

QClipboard *QApplication::clipboard()
{
    if ( qt_clipboard == 0 ) {
	qt_clipboard = new QClipboard;
	CHECK_PTR( qt_clipboard );
    }
    return (QClipboard *)qt_clipboard;
}
