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

#ifndef QPAINTDEVICE_H
#define QPAINTDEVICE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qrect.h"
#endif // QT_H

#if defined(Q_WS_QWS)
class QWSDisplay;
class QGfx;
#endif

class QPaintEngine;

class Q_GUI_EXPORT QPaintDevice                                // device for QPainter
{
public:
    virtual ~QPaintDevice();

    int devType() const;
    bool isExtDev() const;
    bool paintingActive() const;
    virtual QPaintEngine *paintEngine() const = 0;

#if defined(Q_WS_QWS)
    static QWSDisplay *qwsDisplay();
    virtual unsigned char * scanLine(int) const;
    virtual int bytesPerLine() const;
#if 1//def QT_OLD_GFX
    virtual QGfx * graphicsContext(bool clip_children=true) const;
#endif
#endif

protected:
    QPaintDevice(uint devflags);
    virtual int metric(int) const;

    ushort        devFlags;                        // device flags
    ushort        painters;                        // refcount

private:
#if defined(Q_DISABLE_COPY)
    QPaintDevice(const QPaintDevice &);
    QPaintDevice &operator=(const QPaintDevice &);
#endif

#if defined(Q_WS_X11) && defined(QT_COMPAT)
public:
    QT_COMPAT Display *x11Display() const;
    QT_COMPAT int x11Screen() const;
    QT_COMPAT int x11Depth() const;
    QT_COMPAT int x11Cells() const;
    QT_COMPAT Qt::HANDLE x11Colormap() const;
    QT_COMPAT bool x11DefaultColormap() const;
    QT_COMPAT void *x11Visual() const;
    QT_COMPAT bool x11DefaultVisual() const;

    static QT_COMPAT Display *x11AppDisplay();
    static QT_COMPAT int x11AppScreen();
    static QT_COMPAT int x11AppDepth(int screen = -1);
    static QT_COMPAT int x11AppCells(int screen = -1);
    static QT_COMPAT Qt::HANDLE x11AppRootWindow(int screen = -1);
    static QT_COMPAT Qt::HANDLE x11AppColormap(int screen = -1);
    static QT_COMPAT void *x11AppVisual(int screen = -1);
    static QT_COMPAT bool x11AppDefaultColormap(int screen =-1);
    static QT_COMPAT bool x11AppDefaultVisual(int screen =-1);
    static QT_COMPAT int x11AppDpiX(int screen = -1);
    static QT_COMPAT int x11AppDpiY(int screen = -1);
    static QT_COMPAT void x11SetAppDpiX(int, int);
    static QT_COMPAT void x11SetAppDpiY(int, int);
#endif

    friend class QPainter;
    friend class QPaintDeviceMetrics;
    friend class QQuickDrawPaintEngine;
    friend class QFontEngineMac;
    friend void qt_bit_blt(QPaintDevice *, int, int, const QPaintDevice *, int, int, int, int, bool);
};

#ifdef QT_COMPAT
QT_COMPAT Q_GUI_EXPORT
void bitBlt(QPaintDevice *dst, int dx, int dy,
             const QPaintDevice *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
             bool ignoreMask=false);

QT_COMPAT Q_GUI_EXPORT
void bitBlt(QPaintDevice *dst, int dx, int dy,
             const QImage *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
             int conversion_flags=0);

QT_COMPAT Q_GUI_EXPORT
void bitBlt(QPaintDevice *dst, const QPoint &dp,
            const QPaintDevice *src, const QRect &sr=QRect(0,0,-1,-1),
            bool ignoreMask=false);
#endif

/*****************************************************************************
  Inline functions
 *****************************************************************************/

inline int QPaintDevice::devType() const
{ return devFlags & QInternal::DeviceTypeMask; }

inline bool QPaintDevice::isExtDev() const
{ return (devFlags & QInternal::ExternalDevice) != 0; }

inline bool QPaintDevice::paintingActive() const
{ return painters != 0; }

#endif // QPAINTDEVICE_H
