/****************************************************************************
** $Id: $
**
** Implementation of QClipboard class
**
** Created : 960430
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qapplication_p.h"
#include "qdragobject.h"
#include "qpixmap.h"

/*!
    \class QClipboard qclipboard.h
    \brief The QClipboard class provides access to the window system clipboard.

    \ingroup io
    \ingroup environment
    \mainclass

    The clipboard offers a simple mechanism to copy and paste data
    between applications.

    QClipboard supports the same data types that QDragObject does, and
    uses similar mechanisms. For advanced clipboard usage
    read \link dnd.html the drag-and-drop documentation\endlink.

    There is a single QClipboard object in an application, and you can
    access it using QApplication::clipboard().

    Example:
    \code
    QClipboard *cb = QApplication::clipboard();

    // Copy text from the clipboard (paste)
    QString text = cb->text();
    if ( !text.isNull() )
	qDebug( "The clipboard contains: " + text );

    // Copy text into the clipboard
    cb->setText( "This text can be pasted by other programs" );
    \endcode

    QClipboard features some convenience functions to access common data
    types: setText() allows the exchange of Unicode text and
    setPixmap() and setImage() allows the exchange of QPixmaps
    and QImages between applications. The setData() function is the
    ultimate in flexibility: it allows you to add any QMimeSource into the
    clipboard. There are corresponding getters for each of these, e.g.
    text(), image() and pixmap().

    You can clear the clipboard by calling clear().

    The underlying clipboards of the X Window system and MS Windows
    differ. The X Window system has a concept of selection -- when text
    is selected, it is immediately available as the global mouse selection;
    MS Windows only adds text to the clipboard when an explicit copy or cut
    is made. The X Window system also has a concept of ownership; if you
    change the selection within a window, X11 will only notify the owner
    and the previous owner of the change; in MS Windows the clipboard is
    a fully global resource so all applications are notified of changes.
    See the multiclip example in the \e{Qt Designer} examples
    directory for an example of a multiplatform clipboard application
    that also demonstrates selection handling.
*/


/*!
    \internal

    Constructs a clipboard object.

    Do not call this function.

    Call QApplication::clipboard() instead to get a pointer to the
    application's global clipboard object.

    There is only one clipboard in the window system, and creating
    more than one object to represent it is almost certainly an error.
*/

QClipboard::QClipboard( QObject *parent, const char *name )
    : QObject( parent, name )
{
    // nothing
}

#ifndef Q_WS_WIN32
/*!
    \internal

    Destroys the clipboard.

    You should never delete the clipboard. QApplication will do this
    when the application terminates.
*/
QClipboard::~QClipboard()
{
}
#endif

/*!
    \fn void QClipboard::dataChanged()

    This signal is emitted when the clipboard data is changed.
*/

/*!
    \fn void QClipboard::selectionChanged()

    This signal is emitted when the selection is changed. This only
    applies to windowing systems that support selections, e.g. X11.
    Windows doesn't support selections.
*/

/*! \enum QClipboard::Mode
    \keyword clipboard mode

    This enum type is used to control which part of the system clipboard is
    used by QClipboard::data(), QClipboard::setData() and related functions.

    \value Clipboard  indicates that data should be stored and retrieved from
    the global clipboard.

    \value Selection  indicates that data should be stored and retrieved from
    the global mouse selection.

    \e Note: Support for \c Selection is provided only on systems with a
    global mouse selection (e.g. X11).

    \sa QClipboard::supportsSelection()
*/


/*****************************************************************************
  QApplication member functions related to QClipboard.
 *****************************************************************************/

#ifndef QT_NO_MIMECLIPBOARD
// text handling is done directly in qclipboard_qws, for now

/*!
    \overload

    Returns the clipboard text in subtype \a subtype, or a null string
    if the clipboard does not contain any text. If \a subtype is null,
    any subtype is acceptable, and \a subtype is set to the chosen
    subtype.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    text is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the text is retrieved from the global
    mouse selection.

    Common values for \a subtype are "plain" and "html".

    \sa setText(), data(), QString::operator!()
*/
QString QClipboard::text( QString &subtype, Mode mode ) const
{
    QString r;
    QTextDrag::decode( data( mode ), r, subtype );
    return r;
}

/*!
    Returns the clipboard text as plain text, or a null string if the
    clipboard does not contain any text.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    text is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the text is retrieved from the global
    mouse selection.

    \sa setText(), data(), QString::operator!()
*/
QString QClipboard::text( Mode mode ) const
{
    QString subtype("plain");
    return text( subtype, mode );
}

/*!
    Copies \a text into the clipboard as plain text.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    text is stored in the global clipboard.  If \a mode is
    QClipboard::Selection, the text is stored in the global
    mouse selection.

    \sa text(), setData()
*/
void QClipboard::setText( const QString &text, Mode mode )
{
    setData( new QTextDrag(text), mode );
}

/*!
    Returns the clipboard image, or returns a null image if the
    clipboard does not contain an image or if it contains an image in
    an unsupported image format.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    image is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the image is retrieved from the global
    mouse selection.

    \sa setImage() pixmap() data(), QImage::isNull()
*/
QImage QClipboard::image( Mode mode ) const
{
    QImage r;
    QImageDrag::decode( data( mode ), r );
    return r;
}

/*!
    Copies \a image into the clipboard.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    image is stored in the global clipboard.  If \a mode is
    QClipboard::Selection, the data is stored in the global
    mouse selection.

    This is shorthand for:
    \code
        setData( new QImageDrag(image), mode )
    \endcode

    \sa image(), setPixmap() setData()
*/
void QClipboard::setImage( const QImage &image, Mode mode )
{
    setData( new QImageDrag( image ), mode );
}

/*!
    Returns the clipboard pixmap, or null if the clipboard does not
    contain a pixmap. Note that this can lose information. For
    example, if the image is 24-bit and the display is 8-bit, the
    result is converted to 8 bits, and if the image has an alpha
    channel, the result just has a mask.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    pixmap is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the pixmap is retrieved from the global
    mouse selection.

    \sa setPixmap() image() data() QPixmap::convertFromImage().
*/
QPixmap QClipboard::pixmap( Mode mode ) const
{
    QPixmap r;
    QImageDrag::decode( data( mode ), r );
    return r;
}

/*!
    Copies \a pixmap into the clipboard. Note that this is slower
    than setImage() because it needs to convert the QPixmap to a
    QImage first.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    pixmap is stored in the global clipboard.  If \a mode is
    QClipboard::Selection, the pixmap is stored in the global
    mouse selection.

    \sa pixmap() setImage() setData()
*/
void QClipboard::setPixmap( const QPixmap &pixmap, Mode mode )
{
    // *could* just use the handle, but that is X hackery, MIME is better.
    setData( new QImageDrag( pixmap.convertToImage() ), mode );
}


/*! \fn QMimeSource *QClipboard::data( Mode mode ) const
    Returns a reference to a QMimeSource representation of the current
    clipboard data.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    data is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the data is retrieved from the global
    mouse selection.

    \sa setData()
*/

/*! \fn void QClipboard::setData( QMimeSource *src, Mode mode )
    Sets the clipboard data to \a src. Ownership of the data is
    transferred to the clipboard. If you want to remove the data
    either call clear() or call setData() again with new data.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    data is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the data is retrieved from the global
    mouse selection.

    The QDragObject subclasses are reasonable objects to put into the
    clipboard (but do not try to call QDragObject::drag() on the same
    object). Any QDragObject placed in the clipboard should have a
    parent of 0. Do not put QDragMoveEvent or QDropEvent subclasses in
    the clipboard, as they do not belong to the event handler which
    receives them.

    The setText(), setImage() and setPixmap() functions are simpler
    wrappers for setting text, image and pixmap data respectively.

    \sa data()
*/

/*! \fn void QClipboard::clear( Mode mode )
    Clear the clipboard contents.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, this
    function clears the the global clipboard contents.  If \a mode is
    QClipboard::Selection, this function clears the global mouse
    selection contents.

    \sa QClipboard::Mode, supportsSelection()
*/


#endif // QT_NO_MIMECLIPBOARD
#endif // QT_NO_CLIPBOARD
