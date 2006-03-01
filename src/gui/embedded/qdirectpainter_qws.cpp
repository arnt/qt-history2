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

#include "qdirectpainter_qws.h"

#include "qscreen_qws.h"
#include "private/qobject_p.h"
#include "private/qapplication_p.h"
#include "private/qwidget_qws_p.h"
#include "qwsdisplay_qws.h"
#include "qwidget.h"
#include "qimage.h"

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

    Access to the video hardware can be retrieved using the
    frameBuffer() function which returns a pointer to the beginning of
    the display memory.  In order to access the video hardware in a
    way that is co-operative with other applications, a region of the
    screen must be reserved for the application. QDirectPainter
    provides the necessary functionality: the reserveRegion() function
    attempts to reserve the given region and returns the region
    actually reserved. The reserved region can also be retrieved using
    the region() function.

    Depending on the hardware, it might be necessary to lock the video
    hardware for exclusive use for a small time while writing to it,
    using the lock() and unlock() functions. Note that calling lock()
    will prevent all other applications from working until unlock() is
    called.

    In addition, QDirectPainter provides several functions returning
    information about the framebuffer as well as the screen: the
    linestep() function returns the length (in bytes) of each scanline
    of the framebuffer while the screenDepth(), screenWidth() and
    screenHeight() function return the screen metrics.

    \sa QScreen
*/

/*!
    \fn QRegion QDirectPainter::reserveRegion(const QRegion &region)

    Attempts to reserve the given \a region, and returns the region
    that is actually reserved.

    This function also releases the previously reserved region, if
    any. If not released explicitly, the region will be released on
    application exit.

    \sa region()
*/
QRegion QDirectPainter::reserveRegion(const QRegion &reg)
{
    static bool firstTime = true;
    //### ideally, application private functionality should be in qapplication...
    QApplicationPrivate *ad = qApp->d_func();

    if (firstTime) {
        firstTime = false;
        ad->directPainterID  = QWidget::qwsDisplay()->takeId();
        QWidget::qwsDisplay()->nameRegion(ad->directPainterID, "QDirectPainter reserved space", "reserved");
    }
    QWidget::qwsDisplay()->requestRegion(ad->directPainterID, -1, QWSBackingStore::OnScreen, reg, QImage::Format_Invalid);

    //### slightly dirty way to do a blocking wait for the region event

    ad->seenRegionEvent  = false;
    while (!ad->seenRegionEvent)
        QApplication::processEvents();

    return ad->directPainterRegion;
}

/*!
    Returns a pointer to the beginning of the display memory. It is
    the applications responsibility to limit itself to modifying only
    the reserved region.

    \sa region(), linestep()
*/
uchar* QDirectPainter::frameBuffer()
{
    return qt_screen->base();
}

/*!
    Returns the reserved region.

    \sa reserveRegion(), frameBuffer()
*/
QRegion QDirectPainter::region()
{
    return qApp->d_func()->directPainterRegion;
}

/*!
    Returns the bit depth of the display.

    \sa screenHeight(), screenWidth()
*/
int QDirectPainter::screenDepth()
{
    return qt_screen->depth();
}

/*!
    Returns the width of the display in pixels.

    \sa screenHeight(), screenDepth()
*/
int QDirectPainter::screenWidth()
{
    return qt_screen->width();
}

/*!
    Returns the height of the display in pixels.

    \sa screenWidth(), screenDepth()
*/
int QDirectPainter::screenHeight()
{
    return qt_screen->height();
}

/*!
    Returns the length (in bytes) of each scanline of the framebuffer.

    \sa frameBuffer()
*/
int QDirectPainter::linestep()
{
    return qt_screen->linestep();
}


/*!
    Locks access to video hardware, stopping all other applications
    from accessing the screen.

    \warning This will prevent all other applications from working
    until unlock() is called.

    \sa unlock()
*/
void QDirectPainter::lock()
{
    //###
    qDebug("QDirectPainter::lock() not implemented");
}
/*!
    Unlocks the video lock, allowing other applications to display on the screen.

    \sa lock()
 */
void QDirectPainter::unlock()
{
    //###
    qDebug("QDirectPainter::unlock() not implemented");
}

#endif //QT_NO_DIRECTPAINTER

#endif
