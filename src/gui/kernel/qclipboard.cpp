/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qapplication_p.h"
#include "qpixmap.h"
#include "qclipboard_p.h"

/*!
    \class QClipboard qclipboard.h
    \brief The QClipboard class provides access to the window system clipboard.

    \ingroup io
    \ingroup environment
    \mainclass

    The clipboard offers a simple mechanism to copy and paste data
    between applications.

    QClipboard supports the same data types that QDrag does, and
    uses similar mechanisms. For advanced clipboard usage
    read \link dnd.html the drag-and-drop documentation\endlink.

    There is a single QClipboard object in an application, and you can
    access it using QApplication::clipboard().

    Example:
    \code
    QClipboard *cb = QApplication::clipboard();

    // Copy text from the clipboard (paste)
    QString text = cb->text(QClipboard::Clipboard);
    if (!text.isNull())
        qDebug("The clipboard contains: " + text);

    // Copy text into the clipboard
    cb->setText("This text can be pasted by other programs",
                 QClipboard::Clipboard);
    \endcode

    QClipboard features some convenience functions to access common data
    types: setText() allows the exchange of Unicode text and
    setPixmap() and setImage() allows the exchange of QPixmaps
    and QImages between applications. The setMimeData() function is the
    ultimate in flexibility: it allows you to add any QMimeData into the
    clipboard. There are corresponding getters for each of these, e.g.
    text(), image() and pixmap().

    You can clear the clipboard by calling clear().


    \section1 Platform Specific Information

    \section2 X11

    \list

    \i The X11 Window System has the concept of a separate selection
    and clipboard.  When text is selected, it is immediately available
    as the global mouse selection.  The global mouse selection may
    later be copied to the clipboard.  By convention, the middle mouse
    button is used to paste the global mouse selection.

    \i X11 also has the concept of ownership; if you change the
    selection within a window, X11 will only notify the owner and the
    previous owner of the change, i.e. it will not notify all
    applications that the selection or clipboard data changed.

    \i Lastly, the X11 clipboard is event driven, i.e. the clipboard
    will not function properly if the event loop is not running.
    Similarly, it is recommended that the contents of the clipboard
    are stored or retrieved in direct response to user-input events,
    e.g. mouse button or key presses and releases.  You should not
    store or retrieve the clipboard contents in response to timer or
    non-user-input events.

    \endlist

    \section2 Windows

    \list

    \i Microsoft Windows does not support the global mouse selection;
    it only supports the global clipboard, e.g. Windows only adds text
    to the clipboard when an explicit copy or cut is made.

    \i Windows does not have the concept of ownership; the clipboard
    is a fully global resource so all applications are notified of
    changes.

    \endlist

    See the multiclip example in the \e{Qt Designer} examples
    directory for an example of a cross-platform clipboard application
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

QClipboard::QClipboard(QObject *parent)
    : QObject(*new QClipboardPrivate, parent)
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

    \sa setText(), data()
*/
QString QClipboard::text(QString &subtype, Mode mode) const
{
    const QMimeData *data = mimeData(mode);
    if (subtype.isEmpty()) {
        QStringList formats = data->formats();
        if (formats.contains("text/plain"))
            subtype = "plain";
        else {
            for (int i = 0; i < formats.size(); ++i)
                if (formats.at(i).startsWith("text/")) {
                    subtype = formats.at(i).mid(5);
                    break;
                }
        }
    }
    if (subtype.isEmpty())
        return QString();
    if (subtype == "plain")
        return data->text();
    return QString::fromUtf8(data->data("text/" + subtype));
}

/*!
    Returns the clipboard text as plain text, or a null string if the
    clipboard does not contain any text.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    text is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the text is retrieved from the global
    mouse selection.

    \sa setText(), data()
*/
QString QClipboard::text(Mode mode) const
{
    return mimeData(mode)->text();
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
void QClipboard::setText(const QString &text, Mode mode)
{
    QMimeData *data = new QMimeData;
    data->setText(text);
    setMimeData(data, mode);
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
QImage QClipboard::image(Mode mode) const
{
    QPixmap pm = mimeData(mode)->pixmap();
    QImage r = pm.toImage();
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
        QMimeData *data = new QMimeData;
        QPixmap pm;
        pm.fromImage(image);
        data->setPixmap(pm);
        setMimeData(data, mode);
    \endcode

    \sa image(), setPixmap() setData()
*/
void QClipboard::setImage(const QImage &image, Mode mode)
{
    QMimeData *data = new QMimeData;
    QPixmap pm;
    pm.fromImage(image);
    data->setPixmap(pm);
    setMimeData(data, mode);
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
QPixmap QClipboard::pixmap(Mode mode) const
{
    return mimeData(mode)->pixmap();
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
void QClipboard::setPixmap(const QPixmap &pixmap, Mode mode)
{
    QMimeData *data = new QMimeData;
    data->setPixmap(pixmap);
    setMimeData(data, mode);
}


/*! \fn QMimeData *QClipboard::mimeData(Mode mode) const
    Returns a reference to a QMimeData representation of the current
    clipboard data.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    data is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the data is retrieved from the global
    mouse selection.

    The text(), image() and pixmap() functions are simpler
    wrappers for retrieving text, image and pixmap data.

    \sa setData()
*/

/*! \fn void QClipboard::setData(QMimeData *src, Mode mode)
    Sets the clipboard data to \a src. Ownership of the data is
    transferred to the clipboard. If you want to remove the data
    either call clear() or call setData() again with new data.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    data is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the data is retrieved from the global
    mouse selection.

    The setText(), setImage() and setPixmap() functions are simpler
    wrappers for setting text, image and pixmap data respectively.

    \sa data()
*/

/*! \fn void QClipboard::clear(Mode mode)
    Clear the clipboard contents.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, this
    function clears the the global clipboard contents.  If \a mode is
    QClipboard::Selection, this function clears the global mouse
    selection contents.

    \sa QClipboard::Mode, supportsSelection()
*/

#ifdef QT_COMPAT
/*! \fn QMimeSource *QClipboard::data(Mode mode) const
    Returns a reference to a QMimeSource representation of the current
    clipboard data.

    The \a mode argument is used to control which part of the system
    clipboard is used.  If \a mode is QClipboard::Clipboard, the
    data is retrieved from the global clipboard.  If \a mode is
    QClipboard::Selection, the data is retrieved from the global
    mouse selection.

    \sa mimeData()
*/
QMimeSource *QClipboard::data(Mode mode) const
{
    Q_D(const QClipboard);

    if (mode != Clipboard && !supportsSelection())
        return 0;

    if (d->compat_data[mode])
        return d->compat_data[mode];

    d->wrapper[mode]->data = mimeData(mode);
    return d->wrapper[mode];
}


/*! \fn void QClipboard::setData(QMimeSource *src, Mode mode)
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

    \sa setMimeData()
*/
void QClipboard::setData(QMimeSource *source, Mode mode)
{
    Q_D(QClipboard);

    if (mode != Clipboard && !supportsSelection())
        return;

    d->compat_data[mode] = source;
    setMimeData(new QMimeSourceWrapper(d, mode));
}
#endif // QT_COMPAT

const char* QMimeDataWrapper::format(int n) const
{
    QStringList formats = data->formats();
    if (n < 0 || n >= formats.size())
        return 0;
    return formats.at(n).latin1();
}

QByteArray QMimeDataWrapper::encodedData(const char *format) const
{
    return data->data(QLatin1String(format));
}

QVariant QMimeSourceWrapper::retrieveData(const QString &mimetype, QVariant::Type) const
{
    return source->encodedData(mimetype.latin1());
}

bool QMimeSourceWrapper::hasFormat(const QString &mimetype) const
{
    return source->provides(mimetype.latin1());
}

QStringList QMimeSourceWrapper::formats() const
{
    QStringList fmts;
    int i = 0;
    const char *fmt;
    while ((fmt = source->format(i))) {
        fmts.append(QLatin1String(fmt));
        ++i;
    }
    return fmts;
}

#endif // QT_NO_MIMECLIPBOARD
#endif // QT_NO_CLIPBOARD
