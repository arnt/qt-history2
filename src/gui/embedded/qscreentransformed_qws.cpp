/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef QT_NO_QWS_TRANSFORMED

#include "qscreentransformed_qws.h"
#include <qscreendriverfactory_qws.h>
#include <qvector.h>
#include <private/qpainter_p.h>
#include <private/qdrawhelper_p.h>
#include <qmatrix.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <qwindowsystem_qws.h>
#include <qwsdisplay_qws.h>

//#define QT_REGION_DEBUG

class QTransformedScreenPrivate
{
public:
    QTransformedScreenPrivate(QTransformedScreen *parent)
        : transformation(QTransformedScreen::None), subscreen(0), q(parent) {}

    void configure();

    QTransformedScreen::Transformation transformation;
    QScreen *subscreen;
    QTransformedScreen *q;
};


// Global function -----------------------------------------------------------
static QTransformedScreen *qt_trans_screen = 0;

void qws_setScreenTransformation(int t)
{
    if (qt_trans_screen) {
        qt_trans_screen->setTransformation((QTransformedScreen::Transformation)t);
        qwsServer->refresh();
    }
}

// ---------------------------------------------------------------------------
// Transformed Screen
// ---------------------------------------------------------------------------

/*!
    \internal

    \class QTransformedScreen
    \ingroup qws

    \brief The QTransformedScreen class implements a screen driver for
    a transformed screen.

    Note that this class is only available in \l {Qtopia Core}.
    Custom screen drivers can be added by subclassing the
    QScreenDriverPlugin class, using the QScreenDriverFactory class to
    dynamically load the driver into the application, but there should
    only be one screen object per application.

    Use the QScreen::isTransformed() function to determine if a screen
    is transformed. The QTransformedScreen class itself provides means
    of rotating the screen with its setTransformation() function; the
    transformation() function returns the currently set rotation in
    terms of the \l Transformation enum (which describes the various
    available rotation settings). Alternatively, QTransformedScreen
    provides an implementation of the QScreen::transformOrientation()
    function, returning the current rotation as an integer value.

    \sa QScreen, QScreenDriverPlugin, {Running Applications}
*/

/*!
    \enum QTransformedScreen::Transformation

    This enum describes the various rotations a transformed screen can
    have.

    \value None No rotation
    \value Rot90 90 degrees rotation
    \value Rot180 180 degrees rotation
    \value Rot270 270 degrees rotation
*/

/*!
    \fn bool QTransformedScreen::isTransformed() const
    \reimp
*/

/*!
    Constructs a QTransformedScreen object. The \a displayId argument
    identifies the Qtopia Core server to connect to.
*/
QTransformedScreen::QTransformedScreen(int displayId)
    : QScreen(displayId)
{
    d_ptr = new QTransformedScreenPrivate(this);
    d_ptr->transformation = None;
    qt_trans_screen = this;

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::QTransformedScreen";
#endif
}

void QTransformedScreenPrivate::configure()
{
    q->d = subscreen->depth();
    q->w = subscreen->width();
    q->h = subscreen->height();
    q->dw = subscreen->deviceWidth();
    q->dh = subscreen->deviceHeight();
    q->lstep = subscreen->linestep();
    q->data = subscreen->base();
    q->lstep = subscreen->linestep();
    q->size = subscreen->screenSize();

    q->setOffset(subscreen->offset());
    // ###: works because setTransformation recalculates unconditionally
    q->setTransformation(transformation);
}

/*!
    Destroys the QTransformedScreen object.
*/
QTransformedScreen::~QTransformedScreen()
{
    delete d_ptr;
}

static int getDisplayId(const QString &spec)
{
    QRegExp regexp(QLatin1String(":(\\d+)\\b"));
    if (regexp.lastIndexIn(spec) != -1) {
        const QString capture = regexp.cap(1);
        return capture.toInt();
    }
    return 0;
}

static QTransformedScreen::Transformation filterTransformation(QString &spec)
{
    QRegExp regexp(QLatin1String("\\bRot(\\d+):?\\b"), Qt::CaseInsensitive);
    if (regexp.indexIn(spec) == -1)
        return QTransformedScreen::None;

    const int degrees = regexp.cap(1).toInt();
    spec.remove(regexp.pos(0), regexp.matchedLength());

    return static_cast<QTransformedScreen::Transformation>(degrees / 90);
}

/*!
    \reimp
*/
bool QTransformedScreen::connect(const QString &displaySpec)
{
    QString dspec = displaySpec.trimmed();
    if (dspec.startsWith(QLatin1String("Transformed:"), Qt::CaseInsensitive))
        dspec = dspec.mid(QString(QLatin1String("Transformed:")).size());
    else if (!dspec.compare(QLatin1String("Transformed"), Qt::CaseInsensitive))
        dspec = QString();

    const QString displayIdSpec = QString(QLatin1String(" :%1")).arg(displayId);
    if (dspec.endsWith(displayIdSpec))
        dspec = dspec.left(dspec.size() - displayIdSpec.size());

    d_ptr->transformation = filterTransformation(dspec);

    QString driver = dspec;
    int colon = driver.indexOf(QLatin1Char(':'));
    if (colon >= 0)
        driver.truncate(colon);

    if (!QScreenDriverFactory::keys().contains(driver, Qt::CaseInsensitive))
        if (!dspec.isEmpty())
            dspec.prepend(QLatin1String(":"));

    const int id = getDisplayId(dspec);
    d_ptr->subscreen = qt_get_screen(id, dspec.toLatin1().constData());
    d_ptr->configure();

    // XXX
    qt_screen = this;

    return true;
}

/*!
    Returns the currently set rotation.

    \sa setTransformation(), QScreen::transformOrientation()
*/
QTransformedScreen::Transformation QTransformedScreen::transformation() const
{
    return d_ptr->transformation;
}

/*!
    \reimp
*/
int QTransformedScreen::transformOrientation() const
{
    return (int)d_ptr->transformation;
}

/*!
    Rotates this screen object according to the specified \a transformation.

    \sa transformation()
*/
void QTransformedScreen::setTransformation(Transformation transformation)
{
    d_ptr->transformation = transformation;
    QSize s = mapFromDevice(QSize(dw, dh));
    w = s.width();
    h = s.height();

    const QScreen *screen = d_ptr->subscreen;
    s = mapFromDevice(QSize(screen->physicalWidth(), screen->physicalHeight()));
    physWidth = s.width();
    physHeight = s.height();

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::setTransformation" << transformation
             << "size" << w << h << "dev size" << dw << dh;
#endif

}

static inline QRect correctNormalized(const QRect &r) {
    const int x1 = qMin(r.left(), r.right());
    const int x2 = qMax(r.left(), r.right());
    const int y1 = qMin(r.top(), r.bottom());
    const int y2 = qMax(r.top(), r.bottom());

    return QRect( QPoint(x1,y1), QPoint(x2,y2) );
}

template <class DST, class SRC>
static inline void blit90(QScreen *screen, const QImage &image,
                          const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    qt_memrotate90(src, w, h, sstride, dest, dstride);
}

template <class DST, class SRC>
static inline void blit180(QScreen *screen, const QImage &image,
                           const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    qt_memrotate180(src, w, h, sstride, dest, dstride);
}

template <class DST, class SRC>
static inline void blit270(QScreen *screen, const QImage &image,
                           const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    qt_memrotate270(src, w, h, sstride, dest, dstride);
}

typedef void (*BlitFunc)(QScreen *, const QImage &, const QRect &, const QPoint &);

#define SET_BLIT_FUNC(dst, src, rotation, func) \
do {                                            \
    switch (rotation) {                         \
    case Rot90:                                 \
        func = blit90<dst, src>;                \
        break;                                  \
    case Rot180:                                \
        func = blit180<dst, src>;               \
        break;                                  \
    case Rot270:                                \
        func = blit270<dst, src>;               \
        break;                                  \
    default:                                    \
        break;                                  \
    }                                           \
} while (0)

/*!
    \reimp
*/
void QTransformedScreen::blit(const QImage &image, const QPoint &topLeft,
                              const QRegion &region)
{
    const Transformation trans = d_ptr->transformation;
    if (trans == None) {
        d_ptr->subscreen->blit(image, topLeft, region);
        return;
    }

    const QVector<QRect> rects = region.rects();
    const QRect bound = QRect(0, 0, QScreen::w, QScreen::h)
                        & QRect(topLeft, image.size());

    BlitFunc func = 0;
    switch (depth()) {
#ifdef QT_QWS_DEPTH_32
    case 32:
#ifdef QT_QWS_DEPTH_16
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint32, quint16, trans, func);
        else
#endif
            SET_BLIT_FUNC(quint32, quint32, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_24
    case 24:
        SET_BLIT_FUNC(quint24, quint32, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_18
    case 18:
        SET_BLIT_FUNC(quint18, quint32, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint16, quint16, trans, func);
        else
            SET_BLIT_FUNC(quint16, quint32, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint8, quint16, trans, func);
        else
            SET_BLIT_FUNC(quint8, quint32, trans, func);
        break;
#endif
    default:
        return;
    }
    if (!func)
        return;

    QWSDisplay::grab();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i) & bound;

        QPoint dst;
        switch (trans) {
        case Rot90:
            dst = mapToDevice(r.topRight(), QSize(w, h));
            break;
        case Rot180:
            dst = mapToDevice(r.bottomRight(), QSize(w, h));
            break;
        case Rot270:
            dst = mapToDevice(r.bottomLeft(), QSize(w, h));
            break;
        default:
            break;
        }
        func(this, image, r.translated(-topLeft), dst);
    }
    QWSDisplay::ungrab();

}

/*!
    \reimp
*/
void QTransformedScreen::solidFill(const QColor &color, const QRegion &region)
{
    QRegion tr = mapToDevice(region, QSize(w,h));

    Q_ASSERT(tr.boundingRect() == mapToDevice(region.boundingRect(), QSize(w,h)));

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::solidFill region" << region << "transformed" << tr;
#endif
    d_ptr->subscreen->solidFill(color, tr);
}

/*!
    \reimp
*/
QSize QTransformedScreen::mapToDevice(const QSize &s) const
{
    switch (d_ptr->transformation) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
    return s;
}

/*!
    \reimp
*/
QSize QTransformedScreen::mapFromDevice(const QSize &s) const
{
    switch (d_ptr->transformation) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
        return s;
}

/*!
    \reimp
*/
QPoint QTransformedScreen::mapToDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    }

    return rp;
}

/*!
    \reimp
*/
QPoint QTransformedScreen::mapFromDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    }

    return rp;
}

/*!
    \reimp
*/
QRect QTransformedScreen::mapToDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (d_ptr->transformation) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    }

    return correctNormalized(tr);
}

/*!
    \reimp
*/
QRect QTransformedScreen::mapFromDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (d_ptr->transformation) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    }

    return correctNormalized(tr);
}

/*!
    \reimp
*/
QRegion QTransformedScreen::mapToDevice(const QRegion &rgn, const QSize &s) const
{
    if (d_ptr->transformation == None)
        return rgn;

#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice size" << s << "rgn:  " << rgn;
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice trgn:  " << trgn;
#endif
    return trgn;
}

/*!
    \reimp
*/
QRegion QTransformedScreen::mapFromDevice(const QRegion &rgn, const QSize &s) const
{
    if (d_ptr->transformation == None)
        return rgn;
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: realRegion count:  " << rgn.rects().size() << " isEmpty? " << rgn.isEmpty() << "  bounds:" << rgn.boundingRect();
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: transRegion count: " << trgn.rects().size() << " isEmpty? " << trgn.isEmpty() << "  bounds:" << trgn.boundingRect();
#endif
    return trgn;
}

/*!
    \reimp
*/
void QTransformedScreen::disconnect()
{
    if (d_ptr->subscreen) {
        d_ptr->subscreen->disconnect();
        delete d_ptr->subscreen;
        d_ptr->subscreen = 0;
    }
}

/*!
    \reimp
*/
bool QTransformedScreen::initDevice()
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->initDevice();

    return false;
}

/*!
    \reimp
*/
void QTransformedScreen::shutdownDevice()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->shutdownDevice();
}

/*!
    \reimp
*/
void QTransformedScreen::setMode(int w,int h, int d)
{
    if (d_ptr->subscreen) {
        d_ptr->subscreen->setMode(w, h, d);
        d_ptr->configure();
        exposeRegion(region(), 0);
    }
}

/*!
    \reimp
*/
bool QTransformedScreen::supportsDepth(int depth) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->supportsDepth(depth);
    return false;
}

/*!
    \reimp
*/
void QTransformedScreen::save()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->save();
    QScreen::save();
}

/*!
    \reimp
*/
void QTransformedScreen::restore()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->restore();
    QScreen::restore();
}

/*!
    \reimp
*/
void QTransformedScreen::blank(bool on)
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->blank(on);
}

/*!
    \reimp
*/
bool QTransformedScreen::onCard(const unsigned char *ptr) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->onCard(ptr);
    return false;
}

/*!
    \reimp
*/
bool QTransformedScreen::onCard(const unsigned char *ptr, ulong &offset) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->onCard(ptr, offset);
    return false;
}

/*!
    \reimp
*/
bool QTransformedScreen::isInterlaced() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->isInterlaced();
    return false;
}

/*!
    \reimp
*/
int QTransformedScreen::memoryNeeded(const QString &str)
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->memoryNeeded(str);
    else
        return QScreen::memoryNeeded(str);
}

/*!
    \reimp
*/
int QTransformedScreen::sharedRamSize(void *ptr)
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->sharedRamSize(ptr);
    else
        return QScreen::sharedRamSize(ptr);
}

/*!
    \reimp
*/
void QTransformedScreen::haltUpdates()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->haltUpdates();
}

/*!
    \reimp
*/
void QTransformedScreen::resumeUpdates()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->resumeUpdates();
}

/*!
    \reimp
*/
void QTransformedScreen::setDirty(const QRect& rect)
{
    if (!d_ptr->subscreen)
        return;

    const QRect r = mapToDevice(rect, QSize(width(), height()));
    d_ptr->subscreen->setDirty(r);
}

/*!
    \reimp
*/
QWSWindowSurface* QTransformedScreen::createSurface(QWidget *widget) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->createSurface(widget);
    return QScreen::createSurface(widget);
}

/*!
    \reimp
*/
QList<QScreen*> QTransformedScreen::subScreens() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->subScreens();
    return QScreen::subScreens();
}

/*!
    \reimp
*/
QRegion QTransformedScreen::region() const
{
    QRegion deviceRegion;
    if (d_ptr->subscreen)
        deviceRegion = d_ptr->subscreen->region();
    else
        deviceRegion = QScreen::region();

    const QSize size(deviceWidth(), deviceHeight());
    return mapFromDevice(deviceRegion, size);
}

#endif // QT_NO_QWS_TRANSFORMED
