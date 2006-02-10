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

#include "qcursor.h"

#ifndef QT_NO_CURSOR

#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <private/qcursor_p.h>

/*!
    \class QCursor

    \brief The QCursor class provides a mouse cursor with an arbitrary
    shape.

    \ingroup appearance
    \ingroup shared
    \mainclass

    This class is mainly used to create mouse cursors that are
    associated with particular widgets and to get and set the position
    of the mouse cursor.

    Qt has a number of standard cursor shapes, but you can also make
    custom cursor shapes based on a QBitmap, a mask and a hotspot.

    To associate a cursor with a widget, use QWidget::setCursor(). To
    associate a cursor with all widgets (normally for a short period
    of time), use QApplication::setOverrideCursor().

    To set a cursor shape use QCursor::setShape() or use the QCursor
    constructor which takes the shape as argument, or you can use one
    of the predefined cursors defined in the \l Qt::CursorShape enum.

    If you want to create a cursor with your own bitmap, either use
    the QCursor constructor which takes a bitmap and a mask or the
    constructor which takes a pixmap as arguments.

    To set or get the position of the mouse cursor use the static
    methods QCursor::pos() and QCursor::setPos().

    \img cursors.png Cursor Shapes

    \section1 A Note for X11 Users

    On X11, Qt supports the \link
    http://www.xfree86.org/4.3.0/Xcursor.3.html Xcursor\endlink
    library, which allows for full color icon themes. The table below
    shows the cursor name used for each Qt::CursorShape value. If a
    cursor cannot be found using the name shown below, a standard X11
    cursor will be used instead. Note: X11 does not provide
    appropriate cursors for all possible Qt::CursorShape values. It
    is possible that some cursors will be taken from the Xcursor
    theme, while others will use an internal bitmap cursor.

    \table
    \header \o Qt::CursorShape Values   \o Cursor Names
    \row \o Qt::ArrowCursor             \o \c left_ptr
    \row \o Qt::UpArrowCursor           \o \c up_arrow
    \row \o Qt::CrossCursor             \o \c cross
    \row \o Qt::WaitCursor              \o \c wait
    \row \o Qt::BusyCursor              \o \c left_ptr_watch
    \row \o Qt::IBeamCursor             \o \c ibeam
    \row \o Qt::SizeVerCursor           \o \c size_ver
    \row \o Qt::SizeHorCursor           \o \c size_hor
    \row \o Qt::SizeBDiagCursor         \o \c size_bdiag
    \row \o Qt::SizeFDiagCursor         \o \c size_fdiag
    \row \o Qt::SizeAllCursor           \o \c size_all
    \row \o Qt::SplitVCursor            \o \c split_v
    \row \o Qt::SplitHCursor            \o \c split_h
    \row \o Qt::PointingHandCursor      \o \c pointing_hand
    \row \o Qt::ForbiddenCursor         \o \c forbidden
    \row \o Qt::WhatsThisCursor         \o \c whats_this
    \endtable

    \sa QWidget, {fowler}{GUI Design Handbook: Cursors}
*/

/*!
    \fn HCURSOR_or_HANDLE QCursor::handle() const

    Returns a handle to the cursor.

    \warning Using the value returned by this function is not
    portable.

    \sa Qt::HANDLE
*/

/*!
    \fn QCursor::QCursor(HCURSOR cursor)

    Constructs a Qt cursor from the given Windows \a cursor.

    \warning This function is only available on Windows.

    \sa handle()
*/

/*!
    \fn QCursor::QCursor(Qt::HANDLE handle)

    Constructs a Qt cursor from the given \a handle.

    \warning This function is only available on X11.

    \sa handle()
*/

/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM


/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCursor &cursor)
    \relates QCursor

    Writes the \a cursor to the \a stream.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator<<(QDataStream &s, const QCursor &c)
{
    s << (qint16)c.shape();                        // write shape id to stream
    if (c.shape() == Qt::BitmapCursor) {                // bitmap cursor
        bool isPixmap = false;
        if (s.version() >= 7) {
            isPixmap = !c.pixmap().isNull();
            s << isPixmap;
        }
        if (isPixmap)
            s << c.pixmap();
        else
            s << *c.bitmap() << *c.mask();
        s << c.hotSpot();
    }
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCursor &cursor)
    \relates QCursor

    Reads the \a cursor from the \a stream.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator>>(QDataStream &s, QCursor &c)
{
    qint16 shape;
    s >> shape;                                        // read shape id from stream
    if (shape == Qt::BitmapCursor) {                // read bitmap cursor
        bool isPixmap = false;
        if (s.version() >= 7)
            s >> isPixmap;
        if (isPixmap) {
            QPixmap pm;
            QPoint hot;
            s >> pm >> hot;
            c = QCursor(pm, hot.x(), hot.y());
        } else {
            QBitmap bm, bmm;
            QPoint hot;
            s >> bm >> bmm >> hot;
            c = QCursor(bm, bmm, hot.x(), hot.y());
        }
    } else {
        c.setShape((Qt::CursorShape)shape);                // create cursor with shape
    }
    return s;
}
#endif // QT_NO_DATASTREAM


/*!
    Constructs a custom pixmap cursor.

    \a pixmap is the image. It is usual to give it a mask (set using
    QPixmap::setMask()). \a hotX and \a hotY define the cursor's hot
    spot.

    If \a hotX is negative, it is set to the \c{pixmap().width()/2}.
    If \a hotY is negative, it is set to the \c{pixmap().height()/2}.

    Valid cursor sizes depend on the display hardware (or the
    underlying window system). We recommend using 32 x 32 cursors,
    because this size is supported on all platforms. Some platforms
    also support 16 x 16, 48 x 48, and 64 x 64 cursors.

    \sa QPixmap::QPixmap(), QPixmap::setMask()
*/

QCursor::QCursor(const QPixmap &pixmap, int hotX, int hotY)
    : d(0)
{
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither|Qt::AvoidDither);
    QBitmap bm = QBitmap::fromImage(img, Qt::ThresholdDither|Qt::AvoidDither);
    QBitmap bmm = pixmap.mask();
    if (!bmm.isNull()) {
        QBitmap nullBm;
        bm.setMask(nullBm);
    }
    else if (!pixmap.mask().isNull()) {
        QImage mimg = pixmap.mask().toImage().convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither|Qt::AvoidDither);
        bmm = QBitmap::fromImage(mimg, Qt::ThresholdDither|Qt::AvoidDither);
    }
    else {
        bmm = QBitmap(bm.size());
        bmm.fill(Qt::color1);
    }

    d = QCursorData::setBitmap(bm, bmm, hotX, hotY);
    d->pixmap = pixmap;
}



/*!
    Constructs a custom bitmap cursor.

    \a bitmap and
    \a mask make up the bitmap.
    \a hotX and
    \a hotY define the cursor's hot spot.

    If \a hotX is negative, it is set to the \c{bitmap().width()/2}.
    If \a hotY is negative, it is set to the \c{bitmap().height()/2}.

    The cursor \a bitmap (B) and \a mask (M) bits are combined like this:
    \list
    \o B=1 and M=1 gives black.
    \o B=0 and M=1 gives white.
    \o B=0 and M=0 gives transparent.
    \o B=1 and M=0 gives an XOR'd result.
    \endlist

    Use the global Qt color Qt::color0 to draw 0-pixels and Qt::color1 to
    draw 1-pixels in the bitmaps.

    Valid cursor sizes depend on the display hardware (or the
    underlying window system). We recommend using 32 x 32 cursors,
    because this size is supported on all platforms. Some platforms
    also support 16 x 16, 48 x 48, and 64 x 64 cursors.

    \sa QBitmap::QBitmap(), QBitmap::setMask()
*/

QCursor::QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
    : d(0)
{
    d = QCursorData::setBitmap(bitmap, mask, hotX, hotY);
}

QCursorData *qt_cursorTable[Qt::LastCursor + 1];
bool QCursorData::initialized = false;

/*! \internal */
void QCursorData::cleanup()
{
    if(!QCursorData::initialized)
        return;

    for (int shape = 0; shape <= Qt::LastCursor; ++shape) {
        delete qt_cursorTable[shape];
        qt_cursorTable[shape] = 0;
    }
    QCursorData::initialized = false;
}

/*! \internal */
void QCursorData::initialize()
{
    if (QCursorData::initialized)
        return;
#ifdef Q_WS_MAC
    InitCursor();
#endif
    for (int shape = 0; shape <= Qt::LastCursor; ++shape)
        qt_cursorTable[shape] = new QCursorData((Qt::CursorShape)shape);
    QCursorData::initialized = true;
}

/*!
    Constructs a cursor with the default arrow shape.
*/
QCursor::QCursor()
{
    if (!QCursorData::initialized) {
        if (qApp->startingUp()) {
            d = 0;
            return;
        }
        QCursorData::initialize();
    }
    QCursorData *c = qt_cursorTable[0];
    c->ref.ref();
    d = c;
}

/*!
    Constructs a cursor with the specified \a shape.

    See \l Qt::CursorShape for a list of shapes.

    \sa setShape()
*/
QCursor::QCursor(Qt::CursorShape shape)
    : d(0)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    setShape(shape);
}


/*!
    Returns the cursor shape identifier. The return value is one of
    the \l Qt::CursorShape enum values (cast to an int).

    \sa setShape()
*/
Qt::CursorShape QCursor::shape() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    return d->cshape;
}

/*!
    Sets the cursor to the shape identified by \a shape.

    See \l Qt::CursorShape for the list of cursor shapes.

    \sa shape()
*/
void QCursor::setShape(Qt::CursorShape shape)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    QCursorData *c = uint(shape) <= Qt::LastCursor ? qt_cursorTable[shape] : 0;
    if (!c)
        c = qt_cursorTable[0];
    c->ref.ref();
    if (!d) {
        d = c;
    } else {
        c = qAtomicSetPtr(&d, c);
        if (!c->ref.deref())
            delete c;
    }
}

/*!
    Returns the cursor bitmap, or 0 if it is one of the standard
    cursors.
*/
const QBitmap *QCursor::bitmap() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    return d->bm;
}

/*!
    Returns the cursor bitmap mask, or 0 if it is one of the standard
    cursors.
*/

const QBitmap *QCursor::mask() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    return d->bmm;
}

/*!
    Returns the cursor pixmap. This is only valid if the cursor is a
    pixmap cursor.
*/

QPixmap QCursor::pixmap() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    return d->pixmap;
}

/*!
    Returns the cursor hot spot, or (0, 0) if it is one of the
    standard cursors.
*/

QPoint QCursor::hotSpot() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    return QPoint(d->hx, d->hy);
}

/*!
    Constructs a copy of the cursor \a c.
*/

QCursor::QCursor(const QCursor &c)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    d = c.d;
    d->ref.ref();
}

/*!
    Destroys the cursor.
*/

QCursor::~QCursor()
{
    if (d && !d->ref.deref())
        delete d;
}


/*!
    Assigns \a c to this cursor and returns a reference to this
    cursor.
*/

QCursor &QCursor::operator=(const QCursor &c)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    qAtomicAssign(d, c.d);
    return *this;
}

/*!
   Returns the cursor as a QVariant.
*/
QCursor::operator QVariant() const
{
    return QVariant(QVariant::Cursor, this);
}

#endif // QT_NO_CURSOR
