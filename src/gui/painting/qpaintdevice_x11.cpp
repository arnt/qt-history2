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

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qpaintengine_x11.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include <private/qt_x11_p.h>
#include "qx11info_x11.h"

/*!
    \class QPaintDevice qpaintdevice.h
    \brief The QPaintDevice class is the base class of objects that
    can be painted.

    \ingroup multimedia

    A paint device is an abstraction of a two-dimensional space that
    can be drawn using a QPainter. The drawing capabilities are
    implemented by the subclasses QWidget, QPixmap, QPicture and
    QPrinter.

    The default coordinate system of a paint device has its origin
    located at the top-left position. X increases to the right and Y
    increases downward. The unit is one pixel. There are several ways
    to set up a user-defined coordinate system using the painter, for
    example, using QPainter::setWorldMatrix().

    Example (draw on a paint device):
    \code
    void MyWidget::paintEvent(QPaintEvent *)
    {
        QPainter p;                     // our painter
        p.begin(this);                  // start painting the widget
        p.setPen(red);                  // red outline
        p.setBrush(yellow);             // yellow fill
        p.drawEllipse(10, 20, 100,100); // 100x100 ellipse at position (10, 20)
        p.end();                        // painting done
    }
    \endcode

    The bit block transfer is an extremely useful operation for
    copying pixels from one paint device to another (or to itself). It
    is implemented as the global function bitBlt().

    Example (scroll widget contents 10 pixels to the right):
    \code
    bitBlt(myWidget, 10, 0, myWidget);
    \endcode

    \warning Qt requires that a QApplication object exists before
    any paint devices can be created. Paint devices access window
    system resources, and these resources are not initialized before
    an application object is created.
*/

/*!
    Constructs a paint device with internal flags \a devflags. This
    constructor can be invoked only from QPaintDevice subclasses.
*/

QPaintDevice::QPaintDevice(uint devflags)
{
    if (!qApp) {                                // global constructor
        qFatal("QPaintDevice: Must construct a QApplication before a "
                "QPaintDevice");
        return;
    }
    devFlags = devflags;
    painters = 0;
}

/*!
    Destroys the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
    if (paintingActive())
        qWarning("QPaintDevice: Cannot destroy paint device that is being "
                  "painted");
}

/*!
    \fn int QPaintDevice::devType() const

    \internal

    Returns the device type identifier, which is \c QInternal::Widget
    if the device is a QWidget, \c QInternal::Pixmap if it's a
    QPixmap, \c QInternal::Printer if it's a QPrinter, \c
    QInternal::Picture if it's a QPicture or \c
    QInternal::UndefinedDevice in other cases (which should never
    happen).
*/

/*!
    \fn bool QPaintDevice::isExtDev() const

    Returns true if the device is an external paint device; otherwise
    returns false.

    External paint devices cannot be bitBlt()'ed from. QPicture and
    QPrinter are external paint devices.
*/


/*!
    \fn bool QPaintDevice::paintingActive() const

    Returns true if the device is being painted, i.e. someone has
    called QPainter::begin() but not yet called QPainter::end() for
    this device; otherwise returns false.

    \sa QPainter::isActive()
*/

/*! \internal

    Returns the X11 Drawable of the paint device. 0 is returned if it
    can't be obtained.
*/

Drawable qt_x11Handle(const QPaintDevice *pd)
{
    Q_ASSERT(pd);
    if (pd->devType() == QInternal::Widget)
        return static_cast<const QWidget *>(pd)->handle();
    else if (pd->devType() == QInternal::Pixmap)
        return static_cast<const QPixmap *>(pd)->handle();
    return 0;
}

/*!
    Returns the QX11Info structure for the \a pd paint device. 0 is
    returned if it can't be obtained.
*/
QX11Info *qt_x11Info(const QPaintDevice *pd)
{
    Q_ASSERT(pd);
    if (pd->devType() == QInternal::Widget)
        return static_cast<const QWidget *>(pd)->x11Info();
    else if (pd->devType() == QInternal::Pixmap)
        return static_cast<const QPixmap *>(pd)->x11Info();
    return 0;
}

/*!
    \internal

    Internal virtual function that returns paint device metrics.

    Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric(int) const
{
    qWarning("QPaintDevice::metrics: Device has no metric information");
    return 0;
}



#ifdef QT_COMPAT

Display *QPaintDevice::x11Display() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->display();
    return QX11Info::display();
}

int QPaintDevice::x11Screen() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->screen();
    return QX11Info::appScreen();
}

void *QPaintDevice::x11Visual() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->visual();
    return QX11Info::appVisual();
}

int QPaintDevice::x11Depth() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
        return info->depth();
    return QX11Info::appDepth();
}

int QPaintDevice::x11Cells() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->cells();
    return QX11Info::appCells();
}

Qt::HANDLE QPaintDevice::x11Colormap() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->colormap();
    return QX11Info::appColormap();
}

bool QPaintDevice::x11DefaultColormap() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->defaultColormap();
    return QX11Info::appDefaultColormap();
}

bool QPaintDevice::x11DefaultVisual() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->defaultVisual();
    return QX11Info::appDefaultVisual();
}

void *QPaintDevice::x11AppVisual(int screen)
{ return QX11Info::appVisual(screen); }

Qt::HANDLE QPaintDevice::x11AppColormap(int screen)
{ return QX11Info::appColormap(screen); }

Display *QPaintDevice::x11AppDisplay()
{ return QX11Info::display(); }

int QPaintDevice::x11AppScreen()
{ return QX11Info::appScreen(); }

int QPaintDevice::x11AppDepth(int screen)
{ return QX11Info::appDepth(screen); }

int QPaintDevice::x11AppCells(int screen)
{ return QX11Info::appCells(screen); }

Qt::HANDLE QPaintDevice::x11AppRootWindow(int screen)
{ return QX11Info::appRootWindow(screen); }

bool QPaintDevice::x11AppDefaultColormap(int screen)
{ return QX11Info::appDefaultColormap(screen); }

bool QPaintDevice::x11AppDefaultVisual(int screen)
{ return QX11Info::appDefaultVisual(screen); }

/*!
    Sets the value returned by x11AppDpiX() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiY()
*/
void QPaintDevice::x11SetAppDpiX(int dpi, int screen)
{
    QX11Info::setAppDpiX(dpi, screen);
}

/*!
    Sets the value returned by x11AppDpiY() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiX()
*/
void QPaintDevice::x11SetAppDpiY(int dpi, int screen)
{
    QX11Info::setAppDpiY(dpi, screen);
}


/*!
    Returns the horizontal DPI of the X display (X11 only) for screen
    \a screen. Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiY(), x11SetAppDpiX(), QPaintDeviceMetrics::logicalDpiX()
*/
int QPaintDevice::x11AppDpiX(int screen)
{
    return QX11Info::appDpiX(screen);
}

/*!
    Returns the vertical DPI of the X11 display (X11 only) for screen
    \a screen.  Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiX(), x11SetAppDpiY(), QPaintDeviceMetrics::logicalDpiY()
*/
int QPaintDevice::x11AppDpiY(int screen)
{
    return QX11Info::appDpiY(screen);
}
#endif
