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

#include "qplatformdefs.h"

#ifndef QT_NO_MIME

#include "qdragobject.h"
#include "qpixmap.h"
#include "qevent.h"
#include "qfile.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qbuffer.h"
#include "qimageio.h"
#include "qimage.h"
#include "qregexp.h"
#include "qdir.h"
#include "qdrag.h"
#include <ctype.h>
#include "qdnd_p.h"
#include "qdrag.h"

#include <private/qobject_p.h>

static QWidget *last_target = 0;

class QDragObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDragObject)
public:
    QDragObjectPrivate(): hot(0,0) {}
    ~QDragObjectPrivate() { delete data; }
    QPixmap pixmap;
    QPoint hot;
    // store default cursors
    QPixmap *pm_cursor;
    QMimeData *data;
};

class QTextDragPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextDrag)
public:
    QTextDragPrivate() { setSubType("plain"); }
    void setSubType(const QString & st);

    enum { nfmt=4 };

    QString txt;
    QByteArray fmt[nfmt];
    QString subtype;
};

class QStoredDragPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QStoredDrag)
public:
    QStoredDragPrivate() {}
    const char* fmt;
    QByteArray enc;
};

class QImageDragPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QImageDrag)
public:
    QImage img;
    QList<QByteArray> ofmts;
};

#define d d_func()
#define q q_func()

class QDragMime : public QMimeData
{
public:
    QDragMime(QDragObject *parent) : QMimeData(parent) { dragObject = parent; }

    QByteArray data(const QString &mimetype) const;
    bool hasFormat(const QString &mimetype) const;
    QStringList formats() const;

    QDragObject *dragObject;
};

QByteArray QDragMime::data(const QString &mimetype) const
{
    return dragObject->encodedData(mimetype.latin1());
}

bool QDragMime::hasFormat(const QString &mimetype) const
{
    return dragObject->provides(mimetype.latin1());
}

QStringList QDragMime::formats() const
{
    int i = 0;
    const char *format;
    QStringList f;
    while ((format = dragObject->format(i))) {
        f.append(QLatin1String(format));
        ++i;
    }
    return f;
}

/*!
    Constructs a drag object called \a name with a parent \a
    dragSource.

    Note that the drag object will be deleted when the \a dragSource is
    deleted.
*/

QDragObject::QDragObject(QWidget * dragSource, const char * name)
    : QObject(*(new QDragObjectPrivate), dragSource)
{
    setObjectName(QLatin1String(name));
    d->data = new QMimeData(dragSource);
}

/*! \internal */
QDragObject::QDragObject(QDragObjectPrivate &dd, QWidget *dragSource)
    : QObject(dd, dragSource)
{
    d->pm_cursor = 0;
    d->data = new QMimeData(dragSource);
}

/*!
    Destroys the drag object, canceling any drag and drop operation in
    which it is involved.
*/

QDragObject::~QDragObject()
{
}

#ifndef QT_NO_DRAGANDDROP
/*!
    Set the pixmap, \a pm, to display while dragging the object. The
    platform-specific implementation will use this where it can - so
    provide a small masked pixmap, and do not assume that the user
    will actually see it. For example, cursors on Windows 95 are of
    limited size.

    The \a hotspot is the point on (or off) the pixmap that should be
    under the cursor as it is dragged. It is relative to the top-left
    pixel of the pixmap.

    \warning We have seen problems with drag cursors on different
    graphics hardware and driver software on Windows. Setting the
    graphics acceleration in the display settings down one tick solved
    the problems in all cases.
*/
void QDragObject::setPixmap(QPixmap pm, const QPoint& hotspot)
{
    d->pixmap = pm;
    d->hot = hotspot;
#if 0
    QDragManager *manager = QDragManager::self();
    if (manager && manager->object == d->data)
        manager->updatePixmap();
#endif
}

/*!
    \overload

    Uses a hotspot that positions the pixmap below and to the right of
    the mouse pointer. This allows the user to clearly see the point
    on the window where they are dragging the data.
*/
void QDragObject::setPixmap(QPixmap pm)
{
    setPixmap(pm,QPoint(-10, -10));
}

/*!
    Returns the currently set pixmap, or a null pixmap if none is set.

    \sa QPixmap::isNull()
*/
QPixmap QDragObject::pixmap() const
{
    return d->pixmap;
}

/*!
    Returns the currently set pixmap hotspot.

    \sa setPixmap()
*/
QPoint QDragObject::pixmapHotSpot() const
{
    return d->hot;
}

/*!
    Starts a drag operation using the contents of this object, using
    DragDefault mode.

    The function returns true if the caller should delete the original
    copy of the dragged data (but see target()); otherwise returns
    false.

    If the drag contains \e references to information (e.g. file names
    in a QUriDrag are references) then the return value should always
    be ignored, as the target is expected to directly manipulate the
    content referred to by the drag object. On X11 the return value should
    always be correct anyway, but on Windows this is not necessarily
    the case; e.g. the file manager starts a background process to
    move files, so the source \e{must not} delete the files!

    Note that on Windows the drag operation will start a blocking modal
    event loop that will not dispatch any QTimers.
*/
bool QDragObject::drag()
{
    return drag(DragDefault);
}

/*!
    After the drag completes, this function will return the QWidget
   which received the drop, or 0 if the data was dropped on another
    application.

    This can be useful for detecting the case where drag and drop is
    to and from the same widget.
*/
QWidget *QDragObject::target()
{
    return last_target;
}

/*!
    Starts a drag operation using the contents of this object, using
    \c DragMove mode. Be sure to read the constraints described in
    drag().

    \sa drag() dragCopy() dragLink()
*/
bool QDragObject::dragMove()
{
    return drag(DragMove);
}


/*!
    Starts a drag operation using the contents of this object, using
    \c DragCopy mode. Be sure to read the constraints described in
    drag().

    \sa drag() dragMove() dragLink()
*/
void QDragObject::dragCopy()
{
    (void)drag(DragCopy);
}

/*!
    Starts a drag operation using the contents of this object, using
    \c DragLink mode. Be sure to read the constraints described in
    drag().

    \sa drag() dragCopy() dragMove()
*/
void QDragObject::dragLink()
{
    (void)drag(DragLink);
}


/*!
    \enum QDragObject::DragMode

    This enum describes the possible drag modes.

    \value DragDefault     The mode is determined heuristically.
    \value DragCopy        The data is copied.
    \value DragMove        The data is moved.
    \value DragLink        The data is linked.
    \value DragCopyOrMove  The user chooses the mode by using a
                           control key to switch from the default.
*/


/*!
    \overload
    Starts a drag operation using the contents of this object.

    At this point, the object becomes owned by Qt, not the
    application. You should not delete the drag object or anything it
    references. The actual transfer of data to the target application
    will be done during future event processing - after that time the
    drag object will be deleted.

    Returns true if the dragged data was dragged as a \e move,
    indicating that the caller should remove the original source of
    the data (the drag object must continue to have a copy); otherwise
    returns false.

    The \a mode specifies the drag mode (see
    \l{QDragObject::DragMode}.) Normally one of the simpler drag(),
    dragMove(), or dragCopy() functions would be used instead.
*/
bool QDragObject::drag(DragMode mode)
{
    d->data->clear();
    int i = 0;
    const char *fmt;
    while ((fmt = format(i))) {
        d->data->setData(QLatin1String(fmt), encodedData(fmt));
        ++i;
    }

    QDrag drag(qt_cast<QWidget *>(parent()));
    drag.setData(d->data);
    drag.setPixmap(d->pixmap);
    drag.setHotSpot(d->hot);

    QDrag::DragOperation op;
    switch(mode) {
    case DragDefault:
        op = QDrag::DefaultDrag;
        break;
    case DragCopy:
        op = QDrag::CopyDrag;
        break;
    case DragMove:
        op = QDrag::MoveDrag;
        break;
    case DragLink:
        op = QDrag::LinkDrag;
        break;
    case DragCopyOrMove:
        op = QDrag::CopyOrMoveDrag;
        break;
    }
    drag.setAllowedOperations(op);
    switch(drag.start()) {
    case QDrag::MoveDrag:
        return true;
    default:
        return false;
    }
    last_target = drag.target();

    delete this;
}

#endif


/*!
    Returns a pointer to the widget where this object originated (the drag
    source).
*/

QWidget * QDragObject::source()
{
    if (parent() && parent()->isWidgetType())
        return (QWidget *)parent();
    else
        return 0;
}


/*!
    \class QDragObject qdragobject.h

    \brief The QDragObject class encapsulates MIME-based data
    transfer.

    \ingroup draganddrop

    QDragObject is the base class for all data that needs to be
    transferred between and within applications, both for drag and
    drop and for the \link qclipboard.html clipboard\endlink.

    See the \link dnd.html Drag and drop documentation\endlink for an
    overview of how to provide drag and drop in your application.

    See the QClipboard documentation for an overview of how to provide
    cut and paste in your application.

    The drag() function is used to start a drag operation. You can
    specify the \l DragMode in the call or use one of the convenience
    functions dragCopy(), dragMove(), or dragLink(). The drag source
    where the data originated is retrieved with source(). If the data
    was dropped on a widget within the application, target() will
    return a pointer to that widget. Specify the pixmap to display
    during the drag with setPixmap().
*/

static
void stripws(QByteArray& s)
{
    int f;
    while ((f = s.indexOf(' ')) >= 0)
        s.remove(f,1);
}

static
const char * staticCharset(int i)
{
    static QByteArray localcharset;

    switch (i) {
      case 0:
        return "UTF-8";
      case 1:
        return "ISO-10646-UCS-2";
      case 2:
        return ""; // in the 3rd place - some Xdnd targets might only look at 3
      case 3:
        if (localcharset.isNull()) {
            QTextCodec *localCodec = QTextCodec::codecForLocale();
            if (localCodec) {
                localcharset = localCodec->name();
                localcharset = localcharset.toLower();
                stripws(localcharset);
            } else {
                localcharset = "";
            }
        }
        return localcharset;
    }
    return 0;
}

void QTextDragPrivate::setSubType(const QString & st)
{
    subtype = st.toLower();
    for (int i=0; i<nfmt; i++) {
        fmt[i] = "text/";
        fmt[i].append(subtype.latin1());
        QByteArray cs(staticCharset(i));
        if (!cs.isEmpty()) {
            fmt[i].append(";charset=");
            fmt[i].append(cs);
        }
    }
}

/*!
    \class QTextDrag qdragobject.h

    \brief The QTextDrag class is a drag and drop object for
    transferring plain and Unicode text.

    \ingroup draganddrop

    Plain text is passed in a QString which may contain multiple lines
    (i.e. may contain newline characters). The drag target will receive
    the newlines according to the runtime environment, e.g. LF on Unix,
    and CRLF on Windows.

    Qt provides no built-in mechanism for delivering only a single-line.

    For more information about drag and drop, see the QDragObject class
    and the \link dnd.html drag and drop documentation\endlink.
*/


/*!
    Constructs a text drag object with the given \a name, and sets its data
    to \a text. The \a dragSource is the widget that the drag operation started
    from.
*/

QTextDrag::QTextDrag(const QString &text, QWidget * dragSource, const char * name)
    : QDragObject(*new QTextDragPrivate, dragSource)
{
    setObjectName(QLatin1String(name));
    setText(text);
}


/*!
    Constructs a default text drag object with the given \a name.
    The \a dragSource is the widget that the drag operation started from.
*/

QTextDrag::QTextDrag(QWidget * dragSource, const char * name)
    : QDragObject(*(new QTextDragPrivate), dragSource)
{
    setObjectName(QLatin1String(name));
}

/*! \internal */
QTextDrag::QTextDrag(QTextDragPrivate &dd, QWidget *dragSource)
    : QDragObject(dd, dragSource)
{

}

/*!
    Destroys the text drag object.
*/
QTextDrag::~QTextDrag()
{

}

/*!
    \fn void QTextDrag::setSubtype(const QString &subtype)

    Sets the MIME \a subtype of the text being dragged. The default subtype
    is "plain", so the default MIME type of the text is "text/plain".
    You might use this to declare that the text is "text/html" by calling
    setSubtype("html").
*/
void QTextDrag::setSubtype(const QString & st)
{
    d->setSubType(st);
}

/*!
    Sets the \a text to be dragged. You will need to call this if you did
    not pass the text during construction.
*/
void QTextDrag::setText(const QString &text)
{
    d->txt = text;
}


/*!
    \reimp
*/
const char * QTextDrag::format(int i) const
{
    if (i >= d->nfmt)
        return 0;
    return d->fmt[i];
}

QTextCodec* qt_findcharset(const QByteArray& mimetype)
{
    int i=mimetype.indexOf("charset=");
    if (i >= 0) {
        QByteArray cs = mimetype.mid(i+8);
        stripws(cs);
        i = cs.indexOf(';');
        if (i >= 0)
            cs = cs.left(i);
        // win98 often has charset=utf16, and we need to get the correct codec for
        // it to be able to get Unicode text drops.
        if (cs == "utf16")
            cs = "ISO-10646-UCS-2";
        // May return 0 if unknown charset
        return QTextCodec::codecForName(cs);
    }
    // no charset=, use locale
    return QTextCodec::codecForLocale();
}

static QTextCodec *codecForHTML(const QByteArray &ba)
{
    // determine charset
    int mib = 0;
    int pos;
    QTextCodec *c = 0;

    if (ba.size() > 1 && (((uchar)ba[0] == 0xfe && (uchar)ba[1] == 0xff)
                          || ((uchar)ba[0] == 0xff && (uchar)ba[1] == 0xfe))) {
        mib = 1000; // utf16
    } else if (ba.size() > 2
               && (uchar)ba[0] == 0xef
               && (uchar)ba[1] == 0xbb
               && (uchar)ba[2] == 0xbf) {
        mib = 106; // utf-8
    } else {
        pos = 0;
        while ((pos = ba.indexOf('<', pos)) != -1) {
            int end = ba.indexOf('>', pos+1);
            if (end == -1)
                break;
            QString str = ba.mid(pos, end-pos);
            if (str.contains("meta http-equiv=", Qt::CaseInsensitive)) {
                pos = str.indexOf("charset=", 0, Qt::CaseInsensitive) + strlen("charset=");
                if (pos != -1) {
                    int pos2 = ba.indexOf('\"', pos+1);
                    QByteArray cs = ba.mid(pos, pos2-pos);
                    c = QTextCodec::codecForName(cs);
                    if (c)
                        return c;
                }
            }
            pos = end;
        }
    }
    if (mib)
        c = QTextCodec::codecForMib(mib);

    return c;
}

static
QTextCodec* findcodec(const QMimeSource* e)
{
    QTextCodec* r = 0;
    const char* f;
    int i;
    for (i=0; (f=e->format(i)); i++) {
        bool html = !qstrnicmp(f, "text/html", 9);
        if (html)
            r = codecForHTML(e->encodedData(f));
        if (!r)
            r = qt_findcharset(QByteArray(f).toLower());
        if (r)
            return r;
    }
    return 0;
}



/*!
    \reimp
*/
QByteArray QTextDrag::encodedData(const char* mime) const
{
    QByteArray r;
    if (0==qstrnicmp(mime,"text/",5)) {
        QByteArray m(mime);
        m = m.toLower();
        QTextCodec *codec = qt_findcharset(m);
        if (!codec)
            return r;
        QString text(d->txt);
#if defined(Q_WS_WIN)
        int index = text.indexOf(QString::fromLatin1("\r\n"), 0);
        while (index != -1) {
            text.replace(index, 2, QChar('\n'));
            index = text.indexOf("\r\n", index);
        }
#endif
        r = codec->fromUnicode(text);
        if (!codec || codec->mibEnum() != 1000) {
            // Don't include NUL in size (QByteArray::resize() adds NUL)
#if defined(Q_WS_WIN)
            // This is needed to ensure the \0 isn't lost on Windows 95
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                ((QByteArray&)r).resize(r.length()+1);
            else
#endif
                ((QByteArray&)r).resize(r.length());
        }
    }
    return r;
}

/*!
    \fn bool QTextDrag::canDecode(const QMimeSource *source)

    Returns true if the information in the MIME \a source can be decoded
    into a QString; otherwise returns false.

    \sa decode()
*/
bool QTextDrag::canDecode(const QMimeSource* e)
{
    const char* f;
    for (int i=0; (f=e->format(i)); i++) {
        if (0==qstrnicmp(f,"text/",5)) {
            return findcodec(e) != 0;
        }
    }
    return 0;
}

/*!
    \fn bool QTextDrag::decode(const QMimeSource *source, QString &string, QString &subtype)

    \overload

    Attempts to decode the dropped information in the MIME \a source into
    the \a string given.
    Returns true if successful; otherwise returns false. If \a subtype
    is null, any text subtype is accepted; otherwise only the
    specified \a subtype is accepted.

    \sa canDecode()
*/
bool QTextDrag::decode(const QMimeSource* e, QString& str, QString& subtype)
{
    if(!e)
        return false;

    const char* mime;
    for (int i=0; (mime = e->format(i)); i++) {
        if (0==qstrnicmp(mime,"text/",5)) {
            QByteArray m(mime);
            m = m.toLower();
            int semi = m.indexOf(';');
            if (semi < 0)
                semi = m.length();
            QString foundst = m.mid(5,semi-5);
            if (subtype.isNull() || foundst == subtype) {
                bool html = !qstrnicmp(mime, "text/html", 9);
                QTextCodec* codec = 0;
                if (html)
                    // search for the charset tag in the HTML
                    codec = codecForHTML(e->encodedData(mime));
                if (!codec)
                    codec = qt_findcharset(m);
                if (codec) {
                    QByteArray payload;

                    payload = e->encodedData(mime);
                    if (payload.size()) {
                        int l;
                        if (codec->mibEnum() != 1000) {
                            // length is at NUL or payload.size()
                            l = 0;
                            while (l < (int)payload.size() && payload[l])
                                l++;
                        } else {
                            l = payload.size();
                        }

                        str = codec->toUnicode(payload,l);

                        if (subtype.isNull())
                            subtype = foundst;

                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/*!
    \fn bool QTextDrag::decode(const QMimeSource *source, QString &string)

    Attempts to decode the dropped information in the MIME \a source into
    the \a string given.
    Returns true if successful; otherwise returns false.

    \sa canDecode()
*/
bool QTextDrag::decode(const QMimeSource* e, QString& str)
{
    QString st;
    return decode(e, str, st);
}


/*
  QImageDrag could use an internal MIME type for communicating QPixmaps
  and QImages rather than always converting to raw data. This is available
  for that purpose and others. It is not currently used.
*/

/*!
    \class QImageDrag qdragobject.h

    \brief The QImageDrag class provides a drag and drop object for
    transferring images.

    \ingroup draganddrop

    Images are offered to the receiving application in multiple
    formats, determined by Qt's \link QImage::outputFormats() output
    formats\endlink.

    For more information about drag and drop, see the QDragObject
    class and the \link dnd.html drag and drop documentation\endlink.
*/

/*!
    Constructs an image drag object with the given \a name, and sets its
    data to \a image. The \a dragSource is the widget that the drag operation
    started from.
*/

QImageDrag::QImageDrag(QImage image,
                        QWidget * dragSource, const char * name)
    : QDragObject(*(new QImageDragPrivate), dragSource)
{
    setObjectName(QLatin1String(name));
    setImage(image);
}

/*!
    Constructs a default image drag object with the given \a name.
    The \a dragSource is the widget that the drag operation started from.
*/

QImageDrag::QImageDrag(QWidget * dragSource, const char * name)
    : QDragObject(*(new QImageDragPrivate), dragSource)
{
    setObjectName(QLatin1String(name));
}

/*! \internal */
QImageDrag::QImageDrag(QImageDragPrivate &dd, QWidget *dragSource)
    : QDragObject(dd, dragSource)
{
}

/*!
    Destroys the image drag object.
*/

QImageDrag::~QImageDrag()
{
    // nothing
}


/*!
    Sets the \a image to be dragged. You will need to call this if you did
    not pass the image during construction.
*/
void QImageDrag::setImage(QImage image)
{
    d->img = image; // ### detach?
    QList<QByteArray> formats = QImageIO::outputFormats();
    formats.removeAll("PBM"); // remove non-raw PPM
    if (image.depth()!=32) {
        // BMP better than PPM for paletted images
        if (formats.removeAll("BMP")) // move to front
            formats.insert(0,"BMP");
    }
    // PNG is best of all
    if (formats.removeAll("PNG")) // move to front
        formats.insert(0,"PNG");

    for(int i = 0; i < formats.count(); i++) {
        QByteArray format("image/");
        format += formats.at(i);
        format = format.toLower();
        if (format == "image/pbmraw")
            format = "image/ppm";
        d->ofmts.append(format);
    }
}

/*!
    \reimp
*/
const char * QImageDrag::format(int i) const
{
    return i < d->ofmts.count() ? d->ofmts.at(i).data() : 0;
}

/*!
    \reimp
*/
QByteArray QImageDrag::encodedData(const char* fmt) const
{
    if (qstrnicmp(fmt, "image/", 6)==0) {
        QByteArray f(fmt+6);
        QByteArray dat;
        QBuffer w(&dat);
        w.open(IO_WriteOnly);
        QImageIO io(&w, f.toUpper());
        io.setImage(d->img);
        if (!io.write())
            return QByteArray();
        w.close();
        return dat;
    } else {
        return QByteArray();
    }
}

/*!
    \fn bool QImageDrag::canDecode(const QMimeSource *source)

    Returns true if the information in the MIME \a source can be decoded
    into an image; otherwise returns false.

    \sa decode()
*/
bool QImageDrag::canDecode(const QMimeSource* e)
{
    const QList<QByteArray> fileFormats = QImageIO::inputFormats();

    static const QByteArray img("image/");
    for (int i = 0; i < fileFormats.count(); ++i) {
        if (e->provides(img + fileFormats.at(i).toLower()))
            return true;
    }

    return false;
}

/*!
    \fn bool QImageDrag::decode(const QMimeSource *source, QImage &image)

    Decode the dropped information in the MIME \a source into the \a image.
    Returns true if successful; otherwise returns false.

    \sa canDecode()
*/
bool QImageDrag::decode(const QMimeSource* e, QImage& img)
{
    if (!e)
        return false;

    QByteArray payload;
    QList<QByteArray> fileFormats = QImageIO::inputFormats();
    // PNG is best of all
    if (fileFormats.removeAll("PNG")) // move to front
        fileFormats.prepend("PNG");
    for (int i = 0; i < fileFormats.count(); ++i) {
        QByteArray type = "image/" + fileFormats.at(i).toLower();
        if (! e->provides(type))
            continue;
        payload = e->encodedData(type);
        if (!payload.isEmpty())
            break;
    }

    if (payload.isEmpty())
        return false;

    img.loadFromData(payload);
    if (img.isNull())
        return false;
    return true;
}

/*!
    \fn bool QImageDrag::decode(const QMimeSource *source, QPixmap &pixmap)

    \overload

    Decodes the dropped information in the MIME \a source into the \a pixmap.
    Returns true if successful; otherwise returns false.

    This is a convenience function that converts the information to a QPixmap
    via a QImage.

    \sa canDecode()
*/
bool QImageDrag::decode(const QMimeSource* e, QPixmap& pm)
{
    if (!e)
        return false;

    QImage img;
    // We avoid dither, since the image probably came from this display
    if (decode(e, img)) {
        if (!pm.fromImage(img, Qt::AvoidDither))
            return false;

        return true;
    }
    return false;
}




/*!
    \class QStoredDrag qdragobject.h
    \brief The QStoredDrag class provides a simple stored-value drag object for arbitrary MIME data.

    \ingroup draganddrop

    When a block of data has only one representation, you can use a
    QStoredDrag to hold it.

    For more information about drag and drop, see the QDragObject
    class and the \link dnd.html drag and drop documentation\endlink.
*/

/*!
    Constructs a QStoredDrag. The \a dragSource and \a name are passed
    to the QDragObject constructor, and the format is set to \a
    mimeType.

    The data will be unset. Use setEncodedData() to set it.
*/
QStoredDrag::QStoredDrag(const char* mimeType, QWidget * dragSource, const char * name) :
    QDragObject(*new QStoredDragPrivate, dragSource)
{
    setObjectName(QLatin1String(name));
    d->fmt = qstrdup(mimeType);
}

/*! \internal */
QStoredDrag::QStoredDrag(QStoredDragPrivate &dd,  const char* mimeType, QWidget * dragSource)
    : QDragObject(dd, dragSource)
{
    d->fmt = qstrdup(mimeType);
}

/*!
    Destroys the drag object.
*/
QStoredDrag::~QStoredDrag()
{
    delete [] (char*)d->fmt;
}

/*!
    \reimp
*/
const char * QStoredDrag::format(int i) const
{
    if (i==0)
        return d->fmt;
    else
        return 0;
}


/*!
    \fn void QStoredDrag::setEncodedData(const QByteArray &data)

    Sets the encoded \a data of this drag object. The encoded data is
    delivered to drop sites; it must be in a strictly defined and portable
    format.

    The drag object can't be dropped (by the user) until this function
    has been called.
*/

void QStoredDrag::setEncodedData(const QByteArray & encodedData)
{
    d->enc = encodedData;
}

/*!
    \fn QByteArray QStoredDrag::encodedData(const char *format) const

    Returns the stored data in the \a format given.

    \sa setEncodedData()
*/
QByteArray QStoredDrag::encodedData(const char* m) const
{
    if (!qstricmp(m, d->fmt))
        return d->enc;
    else
        return QByteArray();
}


/*!
    \class QUriDrag qdragobject.h
    \brief The QUriDrag class provides a drag object for a list of URI references.

    \ingroup draganddrop

    URIs are a useful way to refer to files that may be distributed
    across multiple machines. A URI will often refer to a file on a
    machine local to both the drag source and the drop target, so the
    URI can be equivalent to passing a file name but is more
    extensible.

    Use URIs in Unicode form so that the user can comfortably edit and
    view them. For use in HTTP or other protocols, use the correctly
    escaped ASCII form.

    You can convert a list of file names to file URIs using
    setFileNames(), or into human-readable form with setUnicodeUris().

    Static functions are provided to convert between filenames and
    URIs; e.g. uriToLocalFile() and localFileToUri(). Static functions
    are also provided to convert URIs to and from human-readable form;
    e.g. uriToUnicodeUri() and unicodeUriToUri().
    You can also decode URIs from a MIME source into a list with
    decodeLocalFiles() and decodeToUnicodeUris().
*/

/*!
    Constructs an object to drag the list of \a uris.
    The \a dragSource and \a name are passed to the QStoredDrag constructor.

    Note that URIs are always in escaped UTF8 encoding.
*/
QUriDrag::QUriDrag(const QList<QByteArray> &uris, QWidget * dragSource, const char * name) :
    QStoredDrag("text/uri-list", dragSource)
{
    setObjectName(name);
    setUris(uris);
}

/*!
    Constructs an object to drag with the given \a name.
    You must call setUris() before you start the drag().
    Both the \a dragSource and the \a name are passed to the QStoredDrag
    constructor.
*/
QUriDrag::QUriDrag(QWidget * dragSource, const char * name) :
    QStoredDrag("text/uri-list", dragSource)
{
    setObjectName(name);
}
#endif

/*!
    Destroys the URI drag object.
*/
QUriDrag::~QUriDrag()
{
}

/*!
    \fn void QUriDrag::setUris(const QList<QByteArray> &list)

    Changes the \a list of URIs to be dragged.

    Note that URIs are always in escaped UTF8 encoding.
*/
void QUriDrag::setUris(const QList<QByteArray> &uris)
{
    QByteArray a;
    int c = 0;
    int i;
    int count = uris.count();
    for (i = 0; i < count; ++i)
        c += uris.at(i).size() + 2; //length + \r\n
    a.reserve(c+1);
    for (i = 0; i < count; ++i) {
        a.append(uris.at(i));
        a.append("\r\n");
    }
    a[c] = 0;
    setEncodedData(a);
}


/*!
    \fn bool QUriDrag::canDecode(const QMimeSource *source)

    Returns true if decode() can decode the MIME \a source; otherwise
    returns false.
*/
bool QUriDrag::canDecode(const QMimeSource* e)
{
    return e->provides("text/uri-list");
}

/*!
    \fn bool QUriDrag::decode(const QMimeSource *source, QList<QByteArray> &list)

    Decodes URIs from the MIME \a source, placing the result in the \a list.
    The list is cleared before any items are inserted.

    Returns true if the MIME \a source contained a valid list of URIs;
    otherwise returns false.
*/
bool QUriDrag::decode(const QMimeSource* e, QList<QByteArray>& l)
{
    QByteArray payload = e->encodedData("text/uri-list");
    if (payload.size()) {
        l.clear();
        int c=0;
        const char* data = payload;
        while (c < payload.size() && data[c]) {
            uint f = c;
            // Find line end
            while (c < payload.size() && data[c] && data[c]!='\r'
                    && data[c] != '\n')
                c++;

            if (c - f > 0 && data[f] != '#') {
                QByteArray s(data+f, c-f);
                l.append(s);
            }

            // Skip junk
            while (c < payload.size() && data[c] &&
                    (data[c]=='\n' || data[c]=='\r'))
                c++;
        }
        return true;
    }
    return false;
}

static uint htod(int h)
{
    if (isdigit(h))
        return h - '0';
    return tolower(h) - 'a' + 10;
}

/*!
  \fn QUriDrag::setFilenames(const QStringList &list)

  \obsolete

  Sets the filename's in the drag object to those in the given \a
  list.

  Use setFileNames() instead.
*/

/*!
    \fn void QUriDrag::setFileNames(const QStringList &filenames)

    Sets the URIs to be local file URIs equivalent to the \a filenames.

    \sa localFileToUri(), setUris()
*/
void QUriDrag::setFileNames(const QStringList & fnames)
{
    QList<QByteArray> uris;
    for (QStringList::ConstIterator i = fnames.begin();
    i != fnames.end(); ++i) {
        QByteArray fileUri = localFileToUri(*i);
        if (!fileUri.isEmpty())
            uris.append(fileUri);
    }

    setUris(uris);
}

/*!
    \fn void QUriDrag::setUnicodeUris(const QStringList &list)

    Sets the URIs in the \a list to be Unicode URIs (only useful for
    displaying to humans).

    \sa localFileToUri(), setUris()
*/
void QUriDrag::setUnicodeUris(const QStringList & uuris)
{
    QList<QByteArray> uris;
    for (int i = 0; i < uuris.count(); ++i)
        uris.append(unicodeUriToUri(uuris.at(i)));
    setUris(uris);
}

/*!
    \fn QByteArray QUriDrag::unicodeUriToUri(const QString &string)

    Returns the URI equivalent of the Unicode URI given in the \a string
    (only useful for displaying to humans).

    \sa uriToLocalFile()
*/
QByteArray QUriDrag::unicodeUriToUri(const QString& uuri)
{
    QByteArray utf8 = uuri.toUtf8();
    QByteArray escutf8;
    int n = utf8.length();
    bool isFile = uuri.startsWith("file://");
    for (int i=0; i<n; i++) {
        if (utf8[i] >= 'a' && utf8[i] <= 'z'
          || utf8[i] == '/'
          || utf8[i] >= '0' && utf8[i] <= '9'
          || utf8[i] >= 'A' && utf8[i] <= 'Z'

          || utf8[i] == '-' || utf8[i] == '_'
          || utf8[i] == '.' || utf8[i] == '!'
          || utf8[i] == '~' || utf8[i] == '*'
          || utf8[i] == '(' || utf8[i] == ')'
          || utf8[i] == '\''

          // Allow this through, so that all URI-references work.
          || (!isFile && utf8[i] == '#')

          || utf8[i] == ';'
          || utf8[i] == '?' || utf8[i] == ':'
          || utf8[i] == '@' || utf8[i] == '&'
          || utf8[i] == '=' || utf8[i] == '+'
          || utf8[i] == '$' || utf8[i] == ',')
        {
            escutf8 += utf8[i];
        } else {
            // Everything else is escaped as %HH
            QString s;
            s.sprintf("%%%02x",(uchar)utf8[i]);
            escutf8 += s.latin1();
        }
    }
    return escutf8;
}

/*!
    Returns the URI equivalent to the absolute local \a filename.

    \sa uriToLocalFile()
*/
QByteArray QUriDrag::localFileToUri(const QString& filename)
{
    QString r = filename;

    //check that it is an absolute file
    if (QDir::isRelativePath(r))
        return QByteArray();
#ifdef Q_WS_WIN


    bool hasHost = false;
    // convert form network path
    if (r.left(2) == "\\\\" || r.left(2) == "//") {
        r.remove(0, 2);
        hasHost = true;
    }

    // Slosh -> Slash
    int slosh;
    while ((slosh=r.indexOf('\\')) >= 0) {
        r[slosh] = '/';
    }

    // Drive
    if (r[0] != '/' && !hasHost)
        r.insert(0,'/');

#endif
#if defined (Q_WS_X11) && 0
    // URL without the hostname is considered to be errorneous by XDnD.
    // See: http://www.newplanetsoftware.com/xdnd/dragging_files.html
    // This feature is not active because this would break dnd between old and new qt apps.
    char hostname[257];
    if (gethostname(hostname, 255) == 0) {
        hostname[256] = '\0';
        r.prepend(QString::fromLatin1(hostname));
    }
#endif
    return unicodeUriToUri(QString("file://" + r));
}

/*!
    \fn QString QUriDrag::uriToUnicodeUri(const char *string)

    Returns the Unicode URI (only useful for displaying to humans)
    equivalent of the URI given in the \a string.

    Note that URIs are always in escaped UTF8 encoding.

    \sa localFileToUri()
*/
QString QUriDrag::uriToUnicodeUri(const char* uri)
{
    QByteArray utf8;

    while (*uri) {
        switch (*uri) {
          case '%': {
                uint ch = (uchar) uri[1];
                if (ch && uri[2]) {
                    ch = htod(ch) * 16 + htod((uchar) uri[2]);
                    utf8 += (char) ch;
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
    \fn QString QUriDrag::uriToLocalFile(const char *string)

    Returns the name of a local file equivalent to the URI given in the
    \a string, or a null string if it does not refer to a local file.

    Note that URIs are always in escaped UTF8 encoding.

    \sa localFileToUri()
*/
QString QUriDrag::uriToLocalFile(const char* uri)
{
    QString file;

    if (!uri)
        return file;
    if (0==qstrnicmp(uri,"file:/",6)) // It is a local file uri
        uri += 6;
    else if (QString(uri).indexOf(":/") != -1) // It is a different scheme uri
        return file;

    bool local = uri[0] != '/' || (uri[0] != '\0' && uri[1] == '/');
#ifdef Q_WS_X11
    // do we have a hostname?
    if (!local && uri[0] == '/' && uri[2] != '/') {
        // then move the pointer to after the 'hostname/' part of the uri
        const char* hostname_end = strchr(uri+1, '/');
        if (hostname_end != NULL) {
            char hostname[257];
            if (gethostname(hostname, 255) == 0) {
                hostname[256] = '\0';
                if (qstrncmp(uri+1, hostname, hostname_end - (uri+1)) == 0) {
                    uri = hostname_end + 1; // point after the slash
                    local = true;
                }
            }
        }
    }
#endif
    if (local) {
        file = uriToUnicodeUri(uri);
        if (uri[1] == '/') {
            file.remove((uint)0,1);
        } else {
                file.insert(0,'/');
        }
#ifdef Q_WS_WIN
        if (file.length() > 2 && file[0] == '/' && file[2] == '|') {
            file[2] = ':';
            file.remove(0,1);
        } else if (file.length() > 2 && file[0] == '/' && file[1].isLetter() && file[2] == ':') {
            file.remove(0, 1);
        }
        // Leave slash as slashes.
#endif
    }
#ifdef Q_WS_WIN
    else {
        file = uriToUnicodeUri(uri);
        // convert to network path
        file.insert(1, '/'); // leave as forward slashes
    }
#endif

    return file;
}

/*!
    \fn bool QUriDrag::decodeLocalFiles(const QMimeSource *source, QStringList &list)

    Decodes URIs from the MIME \a source, converting them to local filenames
    where possible, and places them in the \a list (which is first cleared).

    Returns true if the MIME \a source contained a valid list of URIs;
    otherwise returns false. The list will be empty if no URIs referred to
    local files.
*/
bool QUriDrag::decodeLocalFiles(const QMimeSource* e, QStringList& l)
{
    QList<QByteArray> u;
    if (!decode(e, u))
        return false;

    l.clear();
    for (int i = 0; i < u.count(); ++i) {
        QString lf = uriToLocalFile(u.at(i));
        if (!lf.isEmpty())
            l.append(lf);
    }
    return true;
}

/*!
    \fn bool QUriDrag::decodeToUnicodeUris(const QMimeSource *source, QStringList &list)

    Decodes URIs from the MIME \a source, converting them to Unicode URIs
    (only useful for displaying to humans), and places them in the \a list
    (which is first cleared).

    Returns true if the MIME \a source contained a valid list of URIs;
    otherwise returns false.
*/
bool QUriDrag::decodeToUnicodeUris(const QMimeSource* e, QStringList& l)
{
    QList<QByteArray> u;
    if (!decode(e, u))
        return false;

    l.clear();
    for (int i = 0; i < u.count(); ++i)
        l.append(uriToUnicodeUri(u.at(i)));

    return true;
}

/*!
    \class QColorDrag qdragobject.h

    \brief The QColorDrag class provides a drag and drop object for
    transferring colors between widgets.

    \ingroup draganddrop

    This class provides a drag object which can be used to transfer data
    about colors for drag and drop and in the clipboard. For example, it
    is used in QColorDialog.

    The color is set in the constructor but can be changed with
    setColor().

    For more information about drag and drop, see the QDragObject class
    and the \link dnd.html drag and drop documentation\endlink.
*/

/*!
    Constructs a color drag object with the given \a col and \a
    dragsource. The \a dragsource is passed to the QStoredDrag
    constructor.
*/

/*!
    Constructs a color drag object with the given \a col. Passes \a
    dragsource and \a name to the QStoredDrag constructor.
*/

QColorDrag::QColorDrag(const QColor &col, QWidget *dragsource, const char *name)
    : QStoredDrag("application/x-color", dragsource)
{
    setObjectName(name);
    setColor(col);
}

/*!
    Constructs a color drag object with a white color. Passes \a
    dragsource and \a name to the QStoredDrag constructor.
*/

QColorDrag::QColorDrag(QWidget *dragsource, const char *name)
    : QStoredDrag("application/x-color", dragsource)
{
    setObjectName(name);
    setColor(Qt::white);
}

/*!
    \fn void QColorDrag::setColor(const QColor &color)

    Sets the \a color of the color drag.
*/

void QColorDrag::setColor(const QColor &col)
{
    short r = (col.red()   << 8) | col.red();
    short g = (col.green() << 8) | col.green();
    short b = (col.blue()  << 8) | col.blue();

    // make sure we transmit data in network order
    r = htons(r);
    g = htons(g);
    b = htons(b);

    ushort rgba[4] = {
        r, g, b,
        0xffff // Alpha not supported yet.
    };
    QByteArray data;
    data.resize(sizeof(rgba));
    memcpy(data.data(), rgba, sizeof(rgba));
    setEncodedData(data);
}

/*!
    \fn bool QColorDrag::canDecode(QMimeSource *source)

    Returns true if the color drag object can decode the MIME \a source;
    otherwise returns false.
*/

bool QColorDrag::canDecode(QMimeSource *e)
{
    return e->provides("application/x-color");
}

/*!
    \fn bool QColorDrag::decode(QMimeSource *source, QColor &color)

    Decodes the MIME \a source, and sets the decoded values to the
    given \a color.
*/

bool QColorDrag::decode(QMimeSource *e, QColor &col)
{
    QByteArray data = e->encodedData("application/x-color");
    ushort rgba[4];
    if (data.size() != sizeof(rgba))
        return false;

    memcpy(rgba, data.constData(), sizeof(rgba));

    short r = rgba[0];
    short g = rgba[1];
    short b = rgba[2];
    short a = rgba[3];

    // data is in network order
    r = ntohs(r);
    g = ntohs(g);
    b = ntohs(b);
    a = ntohs(a);

    r = (r >> 8) & 0xff;
    g = (g >> 8) & 0xff;
    b = (b >> 8) & 0xff;
    a = (a >> 8) & 0xff;

    col.setRgb(r, g, b, a);
    return true;
}
