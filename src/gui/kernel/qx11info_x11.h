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

#ifndef QX11INFO_X11_H
#define QX11INFO_X11_H

#include "qnamespace.h"

struct QX11InfoData;
class QPaintDevice;
class QApplicationPrivate;
class QX11InfoPrivate;

typedef struct _XDisplay Display;

class QX11Info
{
public:
    QX11Info();
    ~QX11Info();
    QX11Info(const QX11Info &other);
    QX11Info &operator=(const QX11Info &other);

    static Display *display();
    int screen() const;
    int depth() const;
    int cells() const;
    Qt::HANDLE colormap() const;
    bool defaultColormap() const;
    void *visual() const;
    bool defaultVisual() const;

    static int appScreen();
    static int appDepth(int screen = -1);
    static int appCells(int screen = -1);
    static Qt::HANDLE appColormap(int screen = -1);
    static void *appVisual(int screen = -1);
    static Qt::HANDLE appRootWindow(int screen = -1);
    static bool appDefaultColormap(int screen = -1);
    static bool appDefaultVisual(int screen = -1);
    static int appDpiX(int screen = -1);
    static int appDpiY(int screen = -1);
    static void setAppDpiX(int screen, int dpi);
    static void setAppDpiY(int screen, int dpi);

protected:
    void copyX11Data(const QPaintDevice *);
    void cloneX11Data(const QPaintDevice *);
    void setX11Data(const QX11InfoData *);
    QX11InfoData* getX11Data(bool def = false) const;

    QX11InfoData *x11data;

    friend class QX11PaintEngine;
    friend class QPixmap;
    friend class QWidget;
    friend class QGLWidget;
    friend void qt_init(QApplicationPrivate *priv, int, Display *display, Qt::HANDLE visual,
                        Qt::HANDLE colormap);
    friend void qt_cleanup();
};

#endif // QX11INFO_X11_H
