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

#include "qwindowsurface_qws_p.h"
#include <qwidget.h>
#include <qscreen_qws.h>
#include <qwsmanager_qws.h>
#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qdatastream.h>
#include <qrgb.h>
#include <private/qapplication_p.h>
#include <private/qwsdisplay_qws_p.h>
#include <private/qwidget_p.h>
#include <private/qwsmanager_p.h>
#include <private/qwslock_p.h>

#include <qdebug.h>

static inline bool isWidgetOpaque(const QWidget *w)
{
    const QBrush brush = w->palette().brush(w->backgroundRole());
    return (brush.style() == Qt::NoBrush || brush.isOpaque());
}

class QWSWindowSurfacePrivate
{
public:
    QWSWindowSurfacePrivate() : widget(0), flags(0) {}

    QWidget *widget;
    QWSWindowSurface::SurfaceFlags flags;
    QRegion dirty;
    QRegion clip;
};

QWSWindowSurface::QWSWindowSurface()
    : d_ptr(new QWSWindowSurfacePrivate)
{
}

bool QWSWindowSurface::create(QWidget *window)
{
    d_ptr->widget = window;
    return true;
}

QWSWindowSurface::~QWSWindowSurface()
{
    delete d_ptr;
}

QWidget *QWSWindowSurface::window() const
{
    return d_ptr->widget;
}

const QRegion QWSWindowSurface::dirtyRegion() const
{
    return d_ptr->dirty;
}

void QWSWindowSurface::setDirty(const QRegion &dirty) const
{
    d_ptr->dirty += dirty;
}

const QRegion QWSWindowSurface::clipRegion() const
{
    return d_ptr->clip;
}

void QWSWindowSurface::setClipRegion(const QRegion &clip)
{
    d_ptr->clip = clip;
}

QWSWindowSurface::SurfaceFlags QWSWindowSurface::surfaceFlags() const
{
    return d_ptr->flags;
}

void QWSWindowSurface::setSurfaceFlags(SurfaceFlags flags)
{
    d_ptr->flags = flags;
}

void QWSWindowSurface::resize(const QSize &size)
{
    if (!window())
        return;

    Q_ASSERT(size == window()->frameGeometry().size());
    Q_UNUSED(size);

    QTLWExtra *topextra = window()->d_func()->extra->topextra;
    QWSManager *manager = topextra->qwsManager;

    QRegion region = window()->geometry();
    if (manager)
        region += topextra->qwsManager->region();
    if (!window()->d_func()->extra->mask.isEmpty())
        region &= window()->d_func()->extra->mask.translated(
            window()->geometry().topLeft());

    if (manager)
        manager->d_func()->dirtyRegion(QDecoration::All, QDecoration::Normal);

    QWidget::qwsDisplay()->requestRegion(window()->data->winid,
                                         key(), data(), region);
    d_ptr->dirty = region.translated(-window()->geometry().topLeft());
}

static inline void flushUpdate(QWidget *widget, const QRegion &region,
                               const QPoint &offset)
{
    static QWSYellowSurface surface;
    static int delay = -1;

    if (delay == -1) {
        delay = qgetenv("QT_FLUSH_UPDATE").toInt() * 10;
        surface.setDelay(delay);
    }

    if (delay == 0)
        return;

    surface.flush(widget, region, offset);
}

void QWSWindowSurface::flush(QWidget *widget, const QRegion &region,
                             const QPoint &offset)
{
    if (!window())
        return;

    Q_UNUSED(offset);

    const bool opaque = isWidgetOpaque(window());
    QRegion toFlush = region + dirtyRegion();

    QTLWExtra *topextra = window()->d_func()->extra->topextra;
    QWSManager *manager = topextra->qwsManager;
    if (manager)
        toFlush += manager->d_func()->paint(paintDevice());

    flushUpdate(widget, toFlush, QPoint(0, 0));

    toFlush.translate(window()->mapToGlobal(QPoint(0, 0)));

    window()->qwsDisplay()->repaintRegion(window()->data->winid, opaque, toFlush);
    d_ptr->dirty = QRegion();
}

void QWSWindowSurface::release()
{
    QWidget::qwsDisplay()->requestRegion(window()->data->winid, key(), data(), QRegion());
}

QWSWindowSurface* QWSWindowSurfaceFactory::create(QWidget *widget)
{
    QWSWindowSurface* surface = qt_screen->createSurface(widget);
    if (surface)
        return surface;

    if (QApplication::type() == QApplication::GuiServer)
        surface = new QWSLocalMemWindowSurface;
    else
        surface = new QWSSharedMemWindowSurface;

    if (surface)
        surface->create(widget);

    return surface;
}

QWSWindowSurface*
QWSWindowSurfaceFactory::create(const QString &key,
                                const QByteArray &data)
{
    QWSWindowSurface *surface = qt_screen->createSurface(key);

    if (!surface) {
        if (key == QLatin1String("mem"))
            surface = new QWSLocalMemWindowSurface;
        else if (key == QLatin1String("shm"))
            surface = new QWSSharedMemWindowSurface;
        else if (key == QLatin1String("YellowThing"))
            surface = new QWSYellowSurface;
        else if (key == QLatin1String("DirectPainter"))
            surface = new QWSDirectPainterSurface;
#if 0
        else if (key == QLatin1String("DirectFB"))
            surface = new QWSDirectFBWindowSurface;
#endif
    }

    if (surface)
        surface->attach(data);

    return surface;
}

static void scroll(const QImage &img, const QRect &rect, const QPoint &point)
{
    uchar *mem = const_cast<uchar*>(img.bits());

    int lineskip = img.bytesPerLine();
    int depth = img.depth() >> 3;

    const QRect r = rect;
    const QPoint p = rect.topLeft() + point;

    const uchar *src;
    uchar *dest;

    if (r.top() < p.y()) {
        src = mem + r.bottom() * lineskip + r.left() * depth;
        dest = mem + (p.y() + r.height() - 1) * lineskip + p.x() * depth;
        lineskip = -lineskip;
    } else {
        src = mem + r.top() * lineskip + r.left() * depth;
        dest = mem + p.y() * lineskip + p.x() * depth;
    }

    const int w = r.width();
    int h = r.height();
    const int bytes = w * depth;
    do {
        ::memmove(dest, src, bytes);
        dest += lineskip;
        src += lineskip;
    } while (--h);
}

QWSLocalMemWindowSurface::QWSLocalMemWindowSurface()
    : QWSWindowSurface(), mem(0), memsize(0)
{
    setSurfaceFlags(QWSWindowSurface::Buffered);
}

QWSLocalMemWindowSurface::~QWSLocalMemWindowSurface()
{
}

bool QWSLocalMemWindowSurface::create(QWidget *w)
{
    SurfaceFlags flags = Buffered;
    if (isWidgetOpaque(w))
        flags |= Opaque;
    setSurfaceFlags(flags);
    return QWSWindowSurface::create(w);
}

bool QWSLocalMemWindowSurface::attach(const QByteArray &data)
{
    QDataStream stream(data); // XXX: get rid of streaming

    uchar *mem;
    int width;
    int height;
    int f;
    QImage::Format format;
    SurfaceFlags flags;

    stream.readRawData((char*)(&mem), sizeof(mem));
    stream >> width >> height >> f;
    format = (QImage::Format)(f); // XXX
    stream >> f;
    flags = (SurfaceFlags)(f);

    img = QImage(mem, width, height, format);
    setSurfaceFlags(flags);

    return true;
}

void QWSLocalMemWindowSurface::detach()
{
    img = QImage();
}

QImage::Format
QWSLocalMemWindowSurface::preferredImageFormat(const QWidget *widget) const
{
    const bool opaque = isWidgetOpaque(widget);

    if (opaque && qt_screen->depth() <= 16)
        return QImage::Format_RGB16;
    else
        return QImage::Format_ARGB32_Premultiplied;
}

bool QWSLocalMemWindowSurface::isValidFor(QWidget *widget) const
{
    if (img == QImage())
        return true;

    if (preferredImageFormat(widget) == img.format())
        return true;

    // could instead just set the size to 0 to force a resize
    return false;
}

void QWSLocalMemWindowSurface::resize(const QSize &size)
{
    QImage::Format imageFormat = preferredImageFormat(window());
    const int bytes_per_pixel = imageFormat == QImage::Format_RGB16 ? 2 : 4;

    const int bpl = (size.width() * bytes_per_pixel + 3) & ~3;
    memsize = bpl * size.height();

    delete[] mem;
    if (memsize == 0) {
        mem = 0;
        img = QImage();
    } else {
        mem = new uchar[memsize];
        img = QImage(mem, size.width(), size.height(), imageFormat);
    }

    QWSWindowSurface::resize(size);
}

void QWSLocalMemWindowSurface::release()
{
    QWSWindowSurface::release();
    img = QImage();
    delete[] mem;
    mem = 0;
    memsize = 0;
}

void QWSLocalMemWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    const QVector<QRect> rects = area.rects();
    for (int i = 0; i < rects.size(); ++i)
        ::scroll(img, rects.at(i), QPoint(dx, dy));
}

const QByteArray QWSLocalMemWindowSurface::data() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream.writeRawData((char*)(&mem), sizeof(mem));
    stream << img.width() << img.height() << int(img.format()) << surfaceFlags();
    return array;
}

static inline void lock(QWSLock *l)
{
    if (l)
        l->lock(QWSLock::BackingStore);
}

static inline void unlock(QWSLock *l)
{
    if (l)
        l->unlock(QWSLock::BackingStore);
}

QWSSharedMemWindowSurface::QWSSharedMemWindowSurface()
    : QWSLocalMemWindowSurface(), memlock(0)
{
}

QWSSharedMemWindowSurface::~QWSSharedMemWindowSurface()
{
}

bool QWSSharedMemWindowSurface::create(QWidget *w)
{
    memlock = QWSDisplay::Data::getClientLock();
    return QWSLocalMemWindowSurface::create(w);
}

void QWSSharedMemWindowSurface::beginPaint(const QRegion &)
{
    if (!memlock)
        return;
    memlock->lock(QWSLock::BackingStore);
}

void QWSSharedMemWindowSurface::endPaint(const QRegion &)
{
    if (!memlock)
        return;
    memlock->unlock(QWSLock::BackingStore);
}

bool QWSSharedMemWindowSurface::setMemory(int memId)
{
    if (mem.id() == memId)
        return true;

    mem.detach();
    if (!mem.attach(memId)) {
        perror("QWSSharedMemServerWindowSurface: attaching to shared memory");
        qCritical("QWSSharedMemServerWindowSurface: Error attaching to"
                  " shared memory 0x%x", memId);
        return false;
    }

    return true;
}

bool QWSSharedMemWindowSurface::setLock(int lockId)
{
    if (memlock && memlock->id() == lockId)
        return true;
    delete memlock;
    memlock = (lockId == -1 ? 0 : new QWSLock(lockId));
    return true;
}

bool QWSSharedMemWindowSurface::attach(const QByteArray &data)
{
    QDataStream stream(data);

    int memId;
    int width;
    int height;
    int lockId;
    int f;
    QImage::Format format;
    SurfaceFlags flags;

    stream >> memId >> width >> height >> lockId >> f;
    format = (QImage::Format)(f); // XXX
    stream >> f;
    flags = (SurfaceFlags)(f);

    setSurfaceFlags(flags);
    setMemory(memId);
    setLock(lockId);

    uchar *base = static_cast<uchar*>(mem.address());
    img = QImage(base, width, height, format);

    return true;
}

void QWSSharedMemWindowSurface::detach()
{
    img = QImage();
    mem.detach();
}

void QWSSharedMemWindowSurface::resize(const QSize &size)
{
    QImage::Format imageFormat = preferredImageFormat(window());
    const int bytes_per_pixel = imageFormat == QImage::Format_RGB16 ? 2 : 4;

    const int bpl = (size.width() * bytes_per_pixel + 3) & ~3;
    const int imagesize = bpl * size.height();

    if (imagesize == 0) {
        mem.detach();
        img = QImage();
    } else {
        mem.detach();
        if (!mem.create(imagesize)) {
            perror("QWSBackingStore::create allocating shared memory");
            qFatal("Error creating shared memory of size %d", imagesize);
        }
        uchar *base = static_cast<uchar*>(mem.address());
        img = QImage(base, size.width(), size.height(), imageFormat);
    }

    QWSWindowSurface::resize(size);
}

void QWSSharedMemWindowSurface::release()
{
    QWSWindowSurface::release();
    img = QImage();
    mem.detach();
}

void QWSSharedMemWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    lock(memlock);
    QWSLocalMemWindowSurface::scroll(area, dx, dy);
    unlock(memlock);
}

const QByteArray QWSSharedMemWindowSurface::data() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << mem.id() << img.width() << img.height()
           << (memlock ? memlock->id() : -1) << int(img.format()) << surfaceFlags();
    return array;
}


QWSYellowSurface::QWSYellowSurface()
    : QWSWindowSurface(), delay(10)
{
    setSurfaceFlags(QWSWindowSurface::Buffered);
}

bool QWSYellowSurface::create(QWidget *widget)
{
    winId = QWidget::qwsDisplay()->takeId();
    QWidget::qwsDisplay()->nameRegion(winId,
                                      QLatin1String("Debug flush paint"),
                                      QLatin1String("Silly yellow thing"));
    QWidget::qwsDisplay()->setAltitude(winId, 1, true);

    return QWSWindowSurface::create(widget);
}

QWSYellowSurface::~QWSYellowSurface()
{
}

const QByteArray QWSYellowSurface::data() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream << surfaceSize.width() << surfaceSize.height();
    return array;
}

bool QWSYellowSurface::attach(const QByteArray &data)
{
    QDataStream stream(data);

    int width;
    int height;

    stream >> width >> height;

    img = QImage(width, height, QImage::Format_ARGB32);
    img.fill(qRgba(255,255,31,127));

    return true;
}

void QWSYellowSurface::detach()
{
    img = QImage();
}

void QWSYellowSurface::resize(const QSize &)
{
}

void QWSYellowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(offset);

    QWSDisplay *display = QWidget::qwsDisplay();
    QRegion rgn = region;

    if (widget)
        rgn.translate(widget->mapToGlobal(QPoint(0, 0)));

    surfaceSize = region.boundingRect().size();

    display->requestRegion(winId, key(), data(), rgn);
    display->setAltitude(winId, 1, true);
    display->repaintRegion(winId, false, rgn);

    ::usleep(500 * delay);
    display->requestRegion(winId, key(), data(), QRegion());
    ::usleep(500 * delay);
}

QWSDirectPainterSurface::QWSDirectPainterSurface()
    : QWSWindowSurface(), winId(-1)
{
    setSurfaceFlags(Reserved);
}

QWSDirectPainterSurface::~QWSDirectPainterSurface()
{
}

bool QWSDirectPainterSurface::create(QWidget *widget)
{
    winId  = QWidget::qwsDisplay()->takeId();
    qApp->d_func()->directPainterID = winId;
    QWidget::qwsDisplay()->nameRegion(winId,
                                      QLatin1String("QDirectPainter reserved space"),
                                      QLatin1String("reserved"));
    return QWSWindowSurface::create(widget);
}

void QWSDirectPainterSurface::resize(const QRegion &region)
{
    QRegion reg = region;

    if (qt_screen->isTransformed()) {
        const QSize devSize(qt_screen->deviceWidth(), qt_screen->deviceHeight());
        reg = qt_screen->mapFromDevice(region, devSize);
    }

    QWidget::qwsDisplay()->requestRegion(winId, key(), data(), reg);

    //### slightly dirty way to do a blocking wait for the region event
    QApplicationPrivate *ad = qApp->d_func();
    ad->seenRegionEvent = false;
    while (!ad->seenRegionEvent)
        QApplication::processEvents();

    reg = ad->directPainterRegion;
    if (qt_screen->isTransformed()) {
        const QSize screenSize(qt_screen->width(), qt_screen->height());
        reg = qt_screen->mapToDevice(reg, screenSize);
    }
    setClipRegion(reg);
}

void QWSDirectPainterSurface::release()
{
    QWidget::qwsDisplay()->requestRegion(winId, key(), data(), QRegion());
}
