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
#include <qwsevent_qws.h>
#include <private/qwindowsurface_qws_p.h>
#include <private/qwsdisplay_qws_p.h>

#ifdef Q_WS_QWS
#ifndef QT_NO_DIRECTPAINTER

/*!
    \class QDirectPainter
    \ingroup multimedia
    \ingroup qws

    \brief The QDirectPainter class provides direct access to the
    underlying hardware.

    \preliminary

    Note that this class is only available in \l {Qtopia Core}.

    QDirectPainter allows a client application to reserve a region of
    the framebuffer and render directly onto the screen. The client
    application gets the complete control over the reserved region,
    i.e. the affected region will never be modified by the screen
    driver.

    Use the reserveRegion() function to reserve a region. This
    function will attempt to reserve the given region and return the
    region actually reserved. The reserved region can also be
    retrieved using the reservedRegion() function.

    To draw on a region reserved by a QDirectPainter instance, the
    application must get hold of a pointer to the framebuffer. In
    general, a pointer to the framebuffer can be retrieved using the
    QDirectPainter::frameBuffer() function. But note that if the
    current screen has subscreens, you must query the screen driver
    instead to identify the correct subscreen. A pointer to the
    current screen driver can always be retrieved using the static
    QScreen::instance() function. Then use QScreen's \l
    {QScreen::}{subScreenIndexAt()} and \l {QScreen::}{subScreens()}
    functions to access the correct subscreen, and the subscreen's \l
    {QScreen::}{base()} function to retrieve a pointer to the
    framebuffer.

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

    \sa QScreen, {Qtopia Core Architecture}
*/

/*!
    \enum QDirectPainter::SurfaceFlag

    \value NonReserved
    \value Reserved
*/

/*!
    \fn QRegion QDirectPainter::region()
    \obsolete

    Use QDirectPainter::reservedRegion() instead.
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
    ~QDirectPainterPrivate() {
        if (QPaintDevice::qwsDisplay()) // make sure not in QApplication destructor
            surface->release();
        delete surface;
    }

    QWSDirectPainterSurface *surface;
    QRegion requested_region;

    static QDirectPainter *staticPainter;
    static bool seenStaticRegion;
};

QDirectPainter *QDirectPainterPrivate::staticPainter = 0;
bool QDirectPainterPrivate::seenStaticRegion = false;

void qt_directpainter_region(QDirectPainter *dp, const QRegion &alloc, int type)
{
    QDirectPainterPrivate *d = dp->d_func();

    QRegion r = alloc;
    QScreen *screen = d->surface->screen();
    if (screen->isTransformed()) {
        const QSize screenSize(screen->width(), screen->height());
        r = screen->mapToDevice(r, screenSize);
    }
    if (type == QWSRegionEvent::Allocation) {
        d->surface->setClipRegion(r);
        if (dp == QDirectPainterPrivate::staticPainter)
            QDirectPainterPrivate::seenStaticRegion = true;
        else
            dp->regionChanged(r);
    }
}

#ifndef QT_NO_QWSEMBEDWIDGET
void qt_directpainter_embedevent(QDirectPainter *dp, const QWSEmbedEvent *event)
{
    if (event->type | QWSEmbedEvent::Region)
        dp->setRegion(event->region);
}
#endif

/*!
    Constructs a QDirectPainter with parent \a parent and flag \a flags.
*/
QDirectPainter::QDirectPainter(QObject *parent, SurfaceFlag flag)
    :QObject(*new QDirectPainterPrivate, parent)
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

/*!
    Destroys the object.

    The reserved region will be released.
*/
QDirectPainter::~QDirectPainter()
{
    /* should not be necessary
    if (this == QDirectPainterPrivate::staticPainter)
        QDirectPainterPrivate::staticPainter = 0;
    */
}

/*!
    \since 4.2

    Request to reserve the rectangle \a rect of the screen.

    The requested rectangle will be the maximum available rectangle for
    this object. The actually allocated region may be less, for instance
    if the rectangle overlaps with another QDirectPainter.

    \sa allocatedRegion(), setRegion()
*/
void QDirectPainter::setGeometry(const QRect &rect)
{
    setRegion(rect);
}

/*!
    \since 4.2

    Returns the bounding rect of the requested region.

    \sa requestedRegion()
*/
QRect QDirectPainter::geometry() const
{
    Q_D(const QDirectPainter);
    return d->requested_region.boundingRect();
}

/*!
    \since 4.2

    Request to reserve the region \a region of the screen.

    The requested region will be the maximum available region for
    this object. The actually allocated region may be less, for instance
    if the region overlaps with another QDirectPainter.
*/
void QDirectPainter::setRegion(const QRegion &region)
{
    Q_D(QDirectPainter);
    d->requested_region = region;

    d->surface->setRegion(region);
}

/*!
    \since 4.2

    Returns the region requested by this QDirectPainter.

    \sa setRegion(), allocatedRegion()
*/
QRegion QDirectPainter::requestedRegion() const
{
    Q_D(const QDirectPainter);
    return d->requested_region;
}

/*!
    \since 4.2

    Returns the region allocated to this QDirectPainter.
*/
QRegion QDirectPainter::allocatedRegion() const
{
    Q_D(const QDirectPainter);
    return d->surface->region();
}

/*!
    \since 4.2

    Returns the window system identifier of the widget.
*/
WId QDirectPainter::winId() const
{
    Q_D(const QDirectPainter);
    return d->surface->windowId();
}

/*!
    \since 4.2

    This function is called whenever the allocated region changes.
*/
void QDirectPainter::regionChanged(const QRegion &region)
{
    Q_UNUSED(region);
}

/*!
    \since 4.2
*/
void QDirectPainter::startPainting(bool lockDisplay)
{
    Q_UNUSED(lockDisplay);
}

/*!
    \since 4.2
*/
void QDirectPainter::endPainting()
{
}


/*!
    \since 4.2
*/
void QDirectPainter::raise()
{
    QWidget::qwsDisplay()->setAltitude(winId(), QWSChangeAltitudeCommand::Raise);
}

/*!
    \since 4.2
*/
void QDirectPainter::lower()
{
    QWidget::qwsDisplay()->setAltitude(winId(), QWSChangeAltitudeCommand::Lower);
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
    \since 4.2

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
