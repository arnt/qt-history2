/****************************************************************************
**
** Implementation of QDesktopWidget class for Win32.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesktopwidget.h"
#include "qt_windows.h"
#include "qapplication_p.h"
#include <qvector.h>
#ifdef Q_OS_TEMP
#include <sipapi.h>
#endif

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate(QDesktopWidget *that);
    ~QDesktopWidgetPrivate();

    static int screenCount;
    static int primaryScreen;

    static QVector<QRect> *rects;
    static QVector<QRect> *workrects;

    struct MONITORINFO
    {
        DWORD   cbSize;
        RECT    rcMonitor;
        RECT    rcWork;
        DWORD   dwFlags;
    };

    typedef BOOL (WINAPI *InfoFunc)(HMONITOR, MONITORINFO*);
    typedef BOOL (CALLBACK *EnumProc)(HMONITOR, HDC, LPRECT, LPARAM);
    typedef BOOL (WINAPI *EnumFunc)(HDC, LPCRECT, EnumProc, LPARAM);

    static EnumFunc enumDisplayMonitors;
    static InfoFunc getMonitorInfo;
    static HMODULE user32hnd;
    static int refcount;
};

int QDesktopWidgetPrivate::screenCount = 1;
int QDesktopWidgetPrivate::primaryScreen = 0;
QDesktopWidgetPrivate::EnumFunc QDesktopWidgetPrivate::enumDisplayMonitors = 0;
QDesktopWidgetPrivate::InfoFunc QDesktopWidgetPrivate::getMonitorInfo = 0;
HMODULE QDesktopWidgetPrivate::user32hnd = 0;
QVector<QRect> *QDesktopWidgetPrivate::rects = 0;
QVector<QRect> *QDesktopWidgetPrivate::workrects = 0;
static int screen_number = 0;
int QDesktopWidgetPrivate::refcount = 0;

BOOL CALLBACK enumCallback(HMONITOR hMonitor, HDC, LPRECT, LPARAM)
{
    QDesktopWidgetPrivate::screenCount++;
    QDesktopWidgetPrivate::rects->resize(QDesktopWidgetPrivate::screenCount);
    QDesktopWidgetPrivate::workrects->resize(QDesktopWidgetPrivate::screenCount);
    // Get the MONITORINFO block
    QDesktopWidgetPrivate::MONITORINFO info;
    memset(&info, 0, sizeof(QDesktopWidgetPrivate::MONITORINFO));
    info.cbSize = sizeof(QDesktopWidgetPrivate::MONITORINFO);
    BOOL res = QDesktopWidgetPrivate::getMonitorInfo(hMonitor, &info);
    if (!res) {
        (*QDesktopWidgetPrivate::rects)[screen_number] = QRect();
        (*QDesktopWidgetPrivate::workrects)[screen_number] = QRect();
        return true;
    }

    // Fill list of rects
    RECT r = info.rcMonitor;
    QRect qr(QPoint(r.left, r.top), QPoint(r.right - 1, r.bottom - 1));
    (*QDesktopWidgetPrivate::rects)[screen_number] = qr;

    r = info.rcWork;
    qr = QRect(QPoint(r.left, r.top), QPoint(r.right - 1, r.bottom - 1));
    (*QDesktopWidgetPrivate::workrects)[screen_number] = qr;

    if (info.dwFlags & 0x00000001) //MONITORINFOF_PRIMARY
        QDesktopWidgetPrivate::primaryScreen = screen_number;

    ++screen_number;
    // Stop the enumeration if we have them all
    return true;
}

QDesktopWidgetPrivate::QDesktopWidgetPrivate(QDesktopWidget *that)
{
    if (rects) {
        ++refcount;
        return;
    }

    rects = new QVector<QRect>();
    workrects = new QVector<QRect>();
    ++refcount;

#ifndef Q_OS_TEMP
    if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
        screenCount = 0;
        // Trying to get the function pointers to Win98/2000 only functions
        user32hnd = LoadLibraryA("user32.dll");
        if (!user32hnd)
            return;
        enumDisplayMonitors = (EnumFunc)GetProcAddress(user32hnd, "EnumDisplayMonitors");
        QT_WA({
            getMonitorInfo = (InfoFunc)GetProcAddress(user32hnd, "GetMonitorInfoW");
        } , {
            getMonitorInfo = (InfoFunc)GetProcAddress(user32hnd, "GetMonitorInfoA");
        });

        if (!enumDisplayMonitors || !getMonitorInfo) {
            screenCount = GetSystemMetrics(80);  // SM_CMONITORS
            rects->resize(screenCount);
            for (int i = 0; i < screenCount; ++i)
                rects->replace(i, that->rect());
        return;
        }
        // Calls enumCallback
        enumDisplayMonitors(0, 0, enumCallback, 0);
        enumDisplayMonitors = 0;
        getMonitorInfo = 0;
        FreeLibrary(user32hnd);
    } else {
        rects->resize(1);
        rects->replace(0, that->rect());
        workrects->resize(1);
        workrects->replace(0, that->rect());
    }
#else
    screenCount = 1;

    if ((user32hnd = LoadLibrary(L"user32.dll"))) {
        // CE >= 4.0 case
        enumDisplayMonitors = (EnumFunc)GetProcAddress(user32hnd, L"EnumDisplayMonitors");
        getMonitorInfo = (InfoFunc)GetProcAddress(user32hnd, L"GetMonitorInfoW");
    }

    if ((!enumDisplayMonitors || !getMonitorInfo) && qt_cever >= 400)
        screenCount = GetSystemMetrics(80);  // SM_CMONITORS, only in CE >= 4.0

    if (!user32hnd || !enumDisplayMonitors || !getMonitorInfo) {
        rects->resize(screenCount);
        for (int i = 0; i < screenCount; ++i)
            rects->at(i) = that->rect();

        RECT r;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
        QRect qr = QRect(QPoint(r.left, r.top), QPoint(r.right - 1, r.bottom - 1));

        // Use SIP information, if available
        SIPINFO sip;
        memset(&sip, 0, sizeof(SIPINFO));
        sip.cbSize = sizeof(SIPINFO);
        if (SipGetInfo(&sip))
            qr = QRect(QPoint(sip.rcVisibleDesktop.left, sip.rcVisibleDesktop.top),
                        QPoint(sip.rcVisibleDesktop.right - 1, sip.rcVisibleDesktop.bottom - 1));

        workrects->resize(screenCount);
        for (int j = 0; j < screenCount; ++j)
            workrects->at(j) = qr;
        return;
    }

    // Calls enumCallback
    enumDisplayMonitors(0, 0, enumCallback, 0);
    enumDisplayMonitors = 0;
    getMonitorInfo = 0;
    FreeLibrary(user32hnd);
#endif // Q_OS_TEMP
}

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
    if (!--refcount) {
        screen_number = 0;
        screenCount = 1;
        primaryScreen = 0;
        enumDisplayMonitors = 0;
        getMonitorInfo = 0;
        user32hnd = 0;
        delete rects;
        rects = 0;
        delete workrects;
        workrects = 0;
    }
}

/*
  \omit
  Function is commented out in header
  \fn void *QDesktopWidget::handle(int screen) const

  Returns the window system handle of the display device with the
  index \a screen, for low-level access.  Using this function is not
  portable.

  The return type varies with platform; see qwindowdefs.h for details.

  \sa x11Display(), QPaintDevice::handle()
  \endomit
*/

/*!
    \class QDesktopWidget qdesktopwidget.h
    \brief The QDesktopWidget class provides access to screen information on multi-head systems.

    \ingroup advanced
    \ingroup environment

    Systems with more than one graphics card and monitor can manage the
    physical screen space available either as multiple desktops, or as a
    large virtual desktop, which usually has the size of the bounding
    rectangle of all the screens (see isVirtualDesktop()). For an
    application, one of the available screens is the primary screen, i.e.
    the screen where the main widget resides (see primaryScreen()). All
    windows opened in the context of the application must be
    constrained to the boundaries of the primary screen; for example,
    it would be inconvenient if a dialog box popped up on a different
    screen, or split over two screens.

    The QDesktopWidget provides information about the geometry of the
    available screens with screenGeometry(). The number of screens
    available is returned by numScreens(). The screen number that a
    particular point or widget is located in is returned by
    screenNumber().

    Widgets provided by Qt use this class, for example, to place
    tooltips, menus and dialog boxes according to the parent or
    application widget.

    Applications can use this class to save window positions, or to place
    child widgets on one screen.

    \img qdesktopwidget.png Managing Multiple Screens

    In the illustration above, Application One's primary screen is
    screen 0, and App Two's primary screen is screen 1.


*/

/*!
    Creates the desktop widget.

    If the system supports a virtual desktop, this widget will have
    the size of the virtual desktop; otherwise this widget will have
    the size of the primary screen.

    Instead of using QDesktopWidget directly, use
    QAppliation::desktop().
*/
QDesktopWidget::QDesktopWidget()
: QWidget(0, Qt::WType_Desktop)
{
    setObjectName("desktop");
    d = new QDesktopWidgetPrivate(this);
}

/*!
    Destroy the object and free allocated resources.
*/
QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

/*!
    Returns true if the system manages the available screens in a
    virtual desktop; otherwise returns false.

    For virtual desktops, screen() will always return the same widget.
    The size of the virtual desktop is the size of this desktop
    widget.
*/
bool QDesktopWidget::isVirtualDesktop() const
{
    return true;
}

/*!
    Returns the index of the primary screen.

    \sa numScreens()
*/
int QDesktopWidget::primaryScreen() const
{
    return d->primaryScreen;
}

/*!
    Returns the number of available screens.

    \sa primaryScreen()
*/
int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

/*!
    Returns a widget that represents the screen with index \a screen.
    This widget can be used to draw directly on the desktop, using an
    unclipped painter like this:

    \code
    QPainter paint(QApplication::desktop()->screen(0), true);
    paint.draw...
    ...
    paint.end();
    \endcode

    If the system uses a virtual desktop, the returned widget will
    have the geometry of the entire virtual desktop i.e. bounding
    every \a screen.

    \sa primaryScreen(), numScreens(), isVirtualDesktop()
*/
QWidget *QDesktopWidget::screen(int /*screen*/)
{
    // It seems that a Qt::WType_Desktop cannot be moved?
    return this;
}

/*!
  Returns the available geometry of the screen with index \a screen. What
  is available will be subrect of screenGeometry() based on what the
  platform decides is available (for example excludes the Qt::Dock and Menubar
  on Mac OS X, or the taskbar on Windows).

  \sa screenNumber(), screenGeometry()
*/
const QRect& QDesktopWidget::availableGeometry(int screen) const
{
    if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
        if (screen < 0 || screen >= d->screenCount)
            screen = d->primaryScreen;

        return d->workrects->at(screen);
    } else {
        return d->workrects->at(d->primaryScreen);
    }
}

/*!
    \fn const QRect &QDesktopWidget::availableGeometry(const QWidget *widget) const
    \overload

    Returns the available geometry of the screen which contains \a widget.

    \sa screenGeometry()
*/

/*!
    \fn const QRect &QDesktopWidget::availableGeometry(const QPoint &p) const
    \overload

    Returns the available geometry of the screen which contains \a p.

    \sa screenGeometry()
*/


/*!
    Returns the geometry of the screen with index \a screen.

    \sa screenNumber()
*/
const QRect& QDesktopWidget::screenGeometry(int screen) const
{
    if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
        if (screen < 0 || screen >= d->screenCount)
            screen = d->primaryScreen;

        return d->rects->at(screen);
    } else {
        return d->rects->at(d->primaryScreen);
    }
}

/*!
    \fn const QRect &QDesktopWidget::screenGeometry(const QWidget *widget) const
    \overload

    Returns the geometry of the screen which contains \a widget.
*/

/*!
    \fn const QRect &QDesktopWidget::screenGeometry(const QPoint &p) const
    \overload

    Returns the geometry of the screen which contains \a p.
*/


/*!
    Returns the index of the screen that contains the largest
    part of \a widget, or -1 if the widget not on a screen.

    \sa primaryScreen()
*/
int QDesktopWidget::screenNumber(const QWidget *widget) const
{
    if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
        if (!widget)
            return d->primaryScreen;
        QRect frame = widget->frameGeometry();
        if (!widget->isTopLevel())
            frame.moveTopLeft(widget->mapToGlobal(QPoint(0,0)));

        int maxSize = -1;
        int maxScreen = -1;

        for (int i = 0; i < d->screenCount; ++i) {
            QRect sect = d->rects->at(i).intersect(frame);
            int size = sect.width() * sect.height();
            if (size > maxSize && sect.width() > 0 && sect.height() > 0) {
                maxSize = size;
                maxScreen = i;
            }
        }
        return maxScreen;
    } else {
        return d->primaryScreen;
    }
}

/*!
    \overload
    Returns the index of the screen that contains \a point, or -1 if
    no screen contains the point.

    \sa primaryScreen()
*/
int QDesktopWidget::screenNumber(const QPoint &point) const
{
    if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
        for (int i = 0; i < d->screenCount; ++i) {
            if (d->rects->at(i).contains(point))
                return i;
        }
    }
    return -1;
}

/*!
    \reimp
*/
void QDesktopWidget::resizeEvent(QResizeEvent *)
{
    QVector<QRect> oldrects = *d->rects;
    QVector<QRect> oldworkrects = *d->workrects;
    int oldscreencount = d->screenCount;

    delete d;
    d = new QDesktopWidgetPrivate(this);

    for (int i = 0; i < qMin(oldscreencount, d->screenCount); ++i) {
        QRect oldrect = oldrects[i];
        QRect newrect = d->rects->at(i);
        if (oldrect != newrect)
            emit resized(i);
    }

#ifdef Q_OS_TEMP
    for (int j = 0; j < qMin(oldscreencount, d->screenCount); ++j) {
        QRect oldrect = oldworkrects[j];
        QRect newrect = d->workrects->at(j);
        if (oldrect != newrect)
            emit workAreaResized(j);
    }
#endif
}

/*! \fn void QDesktopWidget::resized(int screen)
    This signal is emitted when the size of \a screen changes.
*/

/*! \fn void QDesktopWidget::workAreaResized(int screen)
    This signal is emitted when the work area available on \a screen changes.
*/
