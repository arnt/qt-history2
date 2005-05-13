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

#include "qwidget.h"
#include "qpixmap.h"
#include "qx11info_x11.h"
#include "qt_x11_p.h"

/*!
    \class QX11Info
    \brief The QX11Info class provides information about the X display
    configuration.

    The class provides two APIs: a set of non-static functions that
    provide information about a specific widget or pixmap, and a set
    of static functions that provide the default information for the
    application.

    \warning This class is only available on X11.

    \sa QWidget::x11Info(), QPixmap::x11Info(), QDesktopWidget
*/

/*!
    Constructs an empty QX11Info object.
*/
QX11Info::QX11Info()
    : x11data(0)
{
}

/*!
    Constructs a copy of \a other.
*/
QX11Info::QX11Info(const QX11Info &other)
{
    x11data = other.x11data;
    ++x11data->ref;
}

/*!
    Assigns \a other to this object and returns a reference to this
    object.
*/
QX11Info &QX11Info::operator=(const QX11Info &other)
{
    QX11InfoData *x = other.x11data;
    ++x->ref;
    x = qAtomicSetPtr(&x11data, x);
    if (x && !--x->ref)
        delete x;
    return *this;
}

/*!
    Destroys the QX11Info object.
*/
QX11Info::~QX11Info()
{
    if (x11data && !--x11data->ref)
        delete x11data;
}

/*!
    \internal
    Makes a shallow copy of the X11-specific data of \a fromDevice, if it is not
    null. Otherwise this function sets it to null.
*/

void QX11Info::copyX11Data(const QPaintDevice *fromDevice)
{
    QX11InfoData *xd = 0;
    if (fromDevice) {
        if (fromDevice->devType() == QInternal::Widget)
            xd = static_cast<const QWidget *>(fromDevice)->x11Info().x11data;
        else if (fromDevice->devType() == QInternal::Pixmap)
            xd = static_cast<const QPixmap *>(fromDevice)->x11Info().x11data;
    }
    setX11Data(xd);
}

/*!
    \internal
    Makes a deep copy of the X11-specific data of \a fromDevice, if it is not
    null. Otherwise this function sets it to null.
*/

void QX11Info::cloneX11Data(const QPaintDevice *fromDevice)
{
    QX11InfoData *d = 0;
    if (fromDevice) {
        QX11InfoData *xd;
        if (fromDevice->devType() == QInternal::Widget) {
            xd = static_cast<const QWidget *>(fromDevice)->x11Info().x11data;
	} else {
	    Q_ASSERT(fromDevice->devType() == QInternal::Pixmap);
            xd = static_cast<const QPixmap *>(fromDevice)->x11Info().x11data;
	}
        d = new QX11InfoData(*xd);
        d->ref = 0;
    }
    setX11Data(d);
}

/*!
    \internal
    Makes a shallow copy of the X11-specific data \a d and assigns it to this
    class. This function increments the reference code of \a d.
*/

void QX11Info::setX11Data(const QX11InfoData* d)
{
    if (x11data && !--x11data->ref)
        delete x11data;
    x11data = (QX11InfoData *)d;
    if (x11data)
        ++x11data->ref;
}


/*!
    \internal
    If \a def is false, returns a deep copy of the x11Data, or 0 if x11Data is 0.
    If \a def is true, makes a QX11Data struct filled with the default
    values.

    In either case the caller is responsible for deleting the returned
    struct. But notice that the struct is a shared class, so other
    classes might also have a reference to it. The reference count of
    the returned QX11Data* is 0.
*/

QX11InfoData* QX11Info::getX11Data(bool def) const
{
    QX11InfoData* res = 0;
    if (def) {
        res = new QX11InfoData;
        res->screen = appScreen();
        res->depth = appDepth();
        res->cells = appCells();
        res->colormap = colormap();
        res->defaultColormap = appDefaultColormap();
        res->visual = (Visual*) appVisual();
        res->defaultVisual = appDefaultVisual();
    } else if (x11data) {
        res = new QX11InfoData;
        *res = *x11data;
    }
    res->ref = 0;
    return res;
}

/*!
    Returns the horizontal resolution of the given \a screen in terms of the
    number of dots per inch.

    \sa setAppDpiX(), appDpiY()
*/
int QX11Info::appDpiX(int screen)
{
    if (!X11)
        return 75;
    if (screen < 0)
        screen = X11->defaultScreen;
    if (screen > X11->screenCount)
        return 0;
    return X11->screens[screen].dpiX;
}

/*!
    Sets the horizontal resolution of the given \a screen to the number of
    dots per inch specified by \a xdpi.

    \sa appDpiX(), setAppDpiY()
*/

void QX11Info::setAppDpiX(int screen, int xdpi)
{
    if (!X11)
        return;
    if (screen < 0)
        screen = X11->defaultScreen;
    if (screen > X11->screenCount)
        return;
    X11->screens[screen].dpiX = xdpi;
}

/*!
    Returns the vertical resolution of the given \a screen in terms of the
    number of dots per inch.

    \sa setAppDpiY(), appDpiX()
*/

int QX11Info::appDpiY(int screen)
{
    if (!X11)
        return 75;
    if (screen < 0)
        screen = X11->defaultScreen;
    if (screen > X11->screenCount)
        return 0;
    return X11->screens[screen].dpiY;
}

/*!
    Sets the vertical resolution of the given \a screen to the number of
    dots per inch specified by \a ydpi.

    \sa appDpiY(), setAppDpiX()
*/
void QX11Info::setAppDpiY(int screen, int ydpi)
{
    if (!X11)
        return;
    if (screen < 0)
        screen = X11->defaultScreen;
    if (screen > X11->screenCount)
        return;
    X11->screens[screen].dpiY = ydpi;
}

/*!
    Returns the X11 time.

    \sa setAppTime(), appUserTime()
*/
unsigned long QX11Info::appTime()
{
    return X11 ? X11->time : 0;
}

/*!
    Sets the X11 time to the value specified by \a time.

    \sa appTime(), setAppUserTime()
*/
void QX11Info::setAppTime(unsigned long time)
{
    if (X11) {
        X11->time = time;
    }
}

/*!
    Returns the X11 user time.

    \sa setAppUserTime(), appUserTime()
*/
unsigned long QX11Info::appUserTime()
{
    return X11 ? X11->userTime : 0;
}

/*!
    Sets the X11 user time as specified by \a time.

    \sa appUserTime(), setAppTime()
*/
void QX11Info::setAppUserTime(unsigned long time)
{
    if (X11) {
        X11->userTime = time;
    }
}


/*!
    \fn const char *QX11Info::appClass()

    Returns the X11 application class.

    \sa display()
*/

/*!
    Returns the default display for the application.

    \sa appScreen()
*/

Display *QX11Info::display()
{
    return X11 ? X11->display : 0;
}

/*!
    Returns the number of the screen where the application is being
    displayed.

    \sa display(), screen()
*/
int QX11Info::appScreen()
{
    return X11 ? X11->defaultScreen : 0;
}

/*!
    Returns a handle for the application's color map on the given \a screen.

    \sa colormap(), defaultColormap()
*/
Qt::HANDLE QX11Info::appColormap(int screen)
{
    return X11->screens[screen == -1 ? X11->defaultScreen : screen].colormap;
}

/*!
    Returns the current visual used by the application on the given
    \a screen.

    \sa visual(), defaultVisual()
*/

void *QX11Info::appVisual(int screen)
{
    return X11->screens[screen == -1 ? X11->defaultScreen : screen].visual;
}

/*!
    Returns a handle for the applications root window on the given \a screen.

    \sa QApplication::desktop()
*/
Qt::HANDLE QX11Info::appRootWindow(int screen)
{
    return RootWindow(X11->display, screen == -1 ? X11->defaultScreen : screen);
}

/*!
    Returns the color depth (bits per pixel) used by the application on the
    given \a screen.

    \sa depth()
*/

int QX11Info::appDepth(int screen)
{
    return X11->screens[screen == -1 ? X11->defaultScreen : screen].depth; }

/*!
    Returns the number of cells used by the application on the given \a screen.

    \sa cells()
*/

int QX11Info::appCells(int screen)
{ return X11->screens[screen == -1 ? X11->defaultScreen : screen].cells; }

/*!
    Returns true if the application has a default color map on the given
    \a screen; otherwise returns false.
*/
bool QX11Info::appDefaultColormap(int screen)
{ return X11->screens[screen == -1 ? X11->defaultScreen : screen].defaultColormap; }

/*!
    Returns true if the application has a default visual on the given \a screen;
    otherwise returns false.
*/
bool QX11Info::appDefaultVisual(int screen)
{ return X11->screens[screen == -1 ? X11->defaultScreen : screen].defaultVisual; }

/*!
    Returns the number of the screen currently in use.

    \sa appScreen()
*/
int QX11Info::screen() const
{ return x11data ? x11data->screen : QX11Info::appScreen(); }

/*!
    Returns the color depth (bits per pixel) of the X display.

    \sa appDepth()
*/

int QX11Info::depth() const
{ return x11data ? x11data->depth : QX11Info::appDepth(); }

/*!
    Returns the number of cells.

    \sa appCells()
*/

int QX11Info::cells() const
{ return x11data ? x11data->cells : QX11Info::appCells(); }

/*!
    Returns a handle for the color map.

    \sa defaultColormap()
*/

Qt::HANDLE QX11Info::colormap() const
{ return x11data ? x11data->colormap : QX11Info::appColormap(); }

/*!
    Returns true if there is a default color map; otherwise returns false.

    \sa colormap()
*/

bool QX11Info::defaultColormap() const
{ return x11data ? x11data->defaultColormap : QX11Info::appDefaultColormap(); }

/*!
    Returns the current visual.

    \sa appVisual(), defaultVisual()
*/

void *QX11Info::visual() const
{ return x11data ? x11data->visual : QX11Info::appVisual(); }

/*!
    Returns true if there is a default visual; otherwise returns false.

    \sa visual(), appVisual()
*/

bool QX11Info::defaultVisual() const
{ return x11data ? x11data->defaultVisual : QX11Info::appDefaultVisual(); }
