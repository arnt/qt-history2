/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard.cpp#22 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qclipboard.h"
#include "qapplication.h"
#include "qpixmap.h"

/*!
  \class QClipboard qclipboard.h
  \brief The QClipboard class provides access to the window system clipboard.

  \ingroup kernel

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
    QString text;

    // Copy text from the clipboard (paste)
    text = cb->text();
    if ( text )
	debug( "The clipboard contains: %s", text );

    // Copy text into the clipboard
    cb->setText( "This text can be pasted by other programs" );
  \endcode
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
  Copies text into the clipboard, where \e format is the clipboard format
  string and \e data is the data to be copied.

  We recommend that you use setText() or setPixmap() instead.
*/
void QClipboard::setData( const char* format, void * )
{
#if defined(CHECK_RANGE)
    warning( "QClipboard::data: Unknown format: %s", format );
#endif
}
