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
    \class QDirectPainter qdirectpainter_qws.h
    \brief The QDirectPainter class provides direct access to the video hardware.

    \ingroup multimedia
    \ingroup qws

    Only available in Qtopia Core.

    When the hardware is known and well defined, as is often the case
    with software for embedded devices, it may be useful to manipulate
    the underlying video hardware directly. In order to do this in a
    way that is co-operative with other applications,
    you must reserve a region of the screen for the application.

    QDirectPainter provides this functionality.

    Depending on the hardware, you may also have to lock the video
    hardware for exclusive use for a small time while you write to
    it. Use the functions lock() and unlock() in that case.

*/

/*!
  Attempts to reserve the region \a reg and returns the region
  actually reserved.  Releases the previously reserved region, if
  any. If not released explicitly, the region will be released on
  application exit.


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

/*
  Returns a pointer to the beginning of display memory. It is the
  applications responsibility to limit itself to modifying only the
  region reserved by reserveRegion().
*/
uchar* QDirectPainter::frameBuffer()
{
    return qt_screen->base();
}

/*!
  Returns the reserved region.
*/
QRegion QDirectPainter::region()
{
    return qApp->d_func()->directPainterRegion;
}

/*!
  Returns the bit depth of the display.
*/
int QDirectPainter::screenDepth()
{
    return qt_screen->depth();
}

/*!
  Returns the width of the display in pixels.
*/
int QDirectPainter::screenWidth()
{
    return qt_screen->width();
}

/*!
  Returns the height of the display in pixels.
*/
int QDirectPainter::screenHeight()
{
    return qt_screen->height();
}

/*!
  Returns the length in bytes of each scanline of the framebuffer.
*/
int QDirectPainter::linestep()
{
    return qt_screen->linestep();
}


/*!
  Locks access to video hardware, stopping all other applications from accessing the screen.
  \warning this will prevent all other applications from working until unlock() is called.
*/
void QDirectPainter::lock()
{
    //###
    qDebug("QDirectPainter::lock() not implemented");
}
/*!
  Unlocks the video lock, allowing other applications to display on the screen.
 */
void QDirectPainter::unlock()
{
    //###
    qDebug("QDirectPainter::unlock() not implemented");
}

#endif //QT_NO_DIRECTPAINTER

#endif
