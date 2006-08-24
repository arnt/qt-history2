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

#include "qdirectpainter_qws.h"

#include "qscreen_qws.h"
#include "private/qobject_p.h"
#include "private/qapplication_p.h"
#include "qwsdisplay_qws.h"
#include "qwidget.h"
#include "qimage.h"
#include <private/qwindowsurface_qws_p.h>

#ifdef Q_WS_QWS
#ifndef QT_NO_DIRECTPAINTER

/*!
    \class QDirectPainter
    \ingroup multimedia
    \ingroup qws

    \brief The QDirectPainter class provides direct access to the
    video hardware.

    When the hardware is known and well defined, as is often the case
    with software for embedded devices, it may be useful to manipulate
    the underlying video hardware directly. Note that this
    functionality is only available in \l {Qtopia Core}.

    Access to the video hardware is provided by the frameBuffer()
    function which returns a pointer to the beginning of the display
    memory.  In order to access the video hardware in a way that is
    co-operative with other applications, a region of the screen must
    be reserved for the application, QDirectPainter provides the
    necessary functionality: The reserveRegion() function attempts to
    reserve the given region and returns the region actually
    reserved. The reserved region can also be retrieved using the
    reservedRegion() function.

    Depending on the hardware, it might be necessary to lock the video
    hardware for exclusive use while writing to it. This is possible
    using the lock() and unlock() functions. Note that calling lock()
    will prevent all other applications from working until unlock() is
    called.

    In addition, QDirectPainter provides several functions returning
    information about the framebuffer as well as the screen: the
    linestep() function returns the length (in bytes) of each scanline
    of the framebuffer while the screenDepth(), screenWidth() and
    screenHeight() function return the screen metrics.

    \sa QScreen, QDecoration
*/

static inline QScreen *getPrimaryScreen()
{
    QScreen *screen = QScreen::instance();
    if (!screen->base()) {
        QList<QScreen*> subScreens = screen->subScreens();
        if (subScreens.size() < 1)
            return 0;
        screen = subScreens.at(0);
    }
    return screen;
}

static inline QSize screenS()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return QSize();
    return QSize(screen->width(), screen->height());
}

static inline QSize devS()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return QSize();
    return QSize(screen->deviceWidth(), screen->deviceHeight());
}


class QDirectPainterPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDirectPainter);
public:

    QWSDirectPainterSurface *surface;
    QRegion requested_region;

    static QDirectPainter *staticPainter;
    static bool seenStaticRegion;
};

QDirectPainter *QDirectPainterPrivate::staticPainter = 0;
bool QDirectPainterPrivate::seenStaticRegion = false;

void qt_directpainter_region(QDirectPainter *dp, const QRegion &alloc)
{
    QDirectPainterPrivate *d = dp->d_func();

    QRegion r = alloc;
    QScreen *screen = d->surface->screen();
    if (screen->isTransformed()) {
        const QSize screenSize(screen->width(), screen->height());
        r = screen->mapToDevice(r, screenSize);
    }
    d->surface->setClipRegion(r);
    if (dp == QDirectPainterPrivate::staticPainter)
        QDirectPainterPrivate::seenStaticRegion = true;
    else
        dp->regionChanged(r);
}


QDirectPainter::QDirectPainter(QObject *parentObject, SurfaceFlag flag)
    :QObject(*new QDirectPainterPrivate, parentObject)
{
    Q_D(QDirectPainter);
    d->surface = new QWSDirectPainterSurface(true);

    if (flag == Reserved)
        d->surface->setReserved();

    QApplicationPrivate *ad = qApp->d_func();
    if (!ad->directPainters)
        ad->directPainters = new QMap<WId, QDirectPainter*>;
    ad->directPainters->insert(d->surface->windowId(), this);
}

QDirectPainter::~QDirectPainter()
{
}

void QDirectPainter::setGeometry(const QRect &r)
{
    setRegion(r);
}

QRect QDirectPainter::geometry() const
{
    Q_D(const QDirectPainter);
    return d->requested_region.boundingRect();
}

void QDirectPainter::setRegion(const QRegion &region)
{
    Q_D(QDirectPainter);
    d->requested_region = region;

    d->surface->setRegion(region);
}

QRegion QDirectPainter::requestedRegion() const
{
    Q_D(const QDirectPainter);
    return d->requested_region;
}

QRegion QDirectPainter::allocatedRegion() const
{
    Q_D(const QDirectPainter);
    return d->surface ? d->surface->region() : QRegion();
}

WId QDirectPainter::winId() const
{
    Q_D(const QDirectPainter);
    return d->surface ? d->surface->windowId() : WId(0);
}

void QDirectPainter::regionChanged(const QRegion &exposedRegion)
{
    Q_UNUSED(exposedRegion);
}

void QDirectPainter::startPainting(bool lockDisplay)
{
    Q_UNUSED(lockDisplay);
}

void QDirectPainter::endPainting()
{
}


/*!
    \fn QRegion QDirectPainter::reserveRegion(const QRegion &region)

    Attempts to reserve the given \a region, and returns the region
    that is actually reserved.

    This function also releases the previously reserved region if
    any. If not released explicitly, the region will be released on
    application exit.

    \sa reservedRegion()
*/
QRegion QDirectPainter::reserveRegion(const QRegion &reg)
{
    if (!QDirectPainterPrivate::staticPainter)
        QDirectPainterPrivate::staticPainter = new QDirectPainter(qApp, Reserved);

    QWSDirectPainterSurface *surface = QDirectPainterPrivate::staticPainter->d_func()->surface;
    surface->setRegion(reg);

    //### slightly dirty way to do a blocking wait for the region event
    QDirectPainterPrivate::seenStaticRegion = false;
    while (!QDirectPainterPrivate::seenStaticRegion)
        QApplication::processEvents();

    return surface->region();
}

/*!
    Returns a pointer to the beginning of the display memory.

    Note that it is the applications responsibility to limit itself to
    modifying only the reserved region.

    \sa reservedRegion(), linestep()
*/
uchar* QDirectPainter::frameBuffer()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return 0;
    return screen->base();
}

/*!
    Returns the reserved region.

    \sa reserveRegion(), frameBuffer()
*/
QRegion QDirectPainter::reservedRegion()
{
    return QDirectPainterPrivate::staticPainter
        ? QDirectPainterPrivate::staticPainter->d_func()->surface->clipRegion()
        : QRegion();
}

/*!
    Returns the bit depth of the display.

    \sa screenHeight(), screenWidth()
*/
int QDirectPainter::screenDepth()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return 0;
    return screen->depth();
}

/*!
    Returns the width of the display in pixels.

    \sa screenHeight(), screenDepth()
*/
int QDirectPainter::screenWidth()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return 0;
    return screen->deviceWidth();
}

/*!
    Returns the height of the display in pixels.

    \sa screenWidth(), screenDepth()
*/
int QDirectPainter::screenHeight()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return 0;
    return screen->deviceHeight();
}

/*!
    Returns the length (in bytes) of each scanline of the framebuffer.

    \sa frameBuffer()
*/
int QDirectPainter::linestep()
{
    QScreen *screen = getPrimaryScreen();
    if (!screen)
        return 0;
    return screen->linestep();
}


/*!
    Locks access to the video hardware.

    Note that calling this function will prevent all other
    applications from working until unlock() is called.

    \sa unlock()
*/
void QDirectPainter::lock()
{
    //###
    qDebug("QDirectPainter::lock() not implemented");
}
/*!
    Unlocks the lock on the video hardware (set by the lock()
    function), allowing other applications to access the screen.

    \sa lock()
 */
void QDirectPainter::unlock()
{
    //###
    qDebug("QDirectPainter::unlock() not implemented");
}

#endif //QT_NO_DIRECTPAINTER

#endif
