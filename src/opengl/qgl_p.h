/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGL_P_H
#define QGL_P_H

#include "private/qwidget_p.h"
#include "qglcolormap.h"
#include "qgl.h"
#include "qmap.h"

class QGLContext;
class QGLOverlayWidget;
class QPixmap;

class QGLWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGLWidget);
public:
    QGLWidgetPrivate():        QWidgetPrivate() {}
    ~QGLWidgetPrivate() {}

    QGLContext* glcx;
    bool autoSwap;

    QGLColormap cmap;
    QMap<QString, int> displayListCache;

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    QGLContext* olcx;
#elif defined(Q_WS_X11)
    QGLOverlayWidget*        olw;
#endif
#ifdef Q_WS_MAC
    void updatePaintDevice();
#endif
};

#endif // QGL_P_H
