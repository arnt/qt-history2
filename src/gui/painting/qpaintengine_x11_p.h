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

#ifndef QPAINTENGINE_X11_P_H
#define QPAINTENGINE_X11_P_H

#include "qregion.h"
#include "qpen.h"
#include <private/qpaintengine_p.h>

#include "qx11info_x11.h"
#include <private/qt_x11_p.h>

class QX11PaintEnginePrivate : public QPaintEnginePrivate {

public:
    QX11PaintEnginePrivate()
    {
        dpy = 0;
        scrn = -1;
        hd = 0;
        xft_hd = 0;
        //              flags = Qt::IsStartingUp;
        bg_col = Qt::white;                             // default background color
        bg_mode = Qt::TransparentMode;                  // default background mode
        tabstops = 0;                               // default tabbing
        tabarray = 0;
        tabarraylen = 0;
        ps_stack = 0;
        wm_stack = 0;
        gc = gc_brush = 0;
        dpy  = 0;
        //             txop = txinv = 0;
        penRef = brushRef = 0;
        clip_serial = 0;
        //             pfont = 0;
        //             block_ext = false;
        xinfo = 0;
        txop = QPainter::TxNone;
    }
    Display *dpy;
    int scrn;
    Qt::HANDLE hd;
#if !defined (QT_NO_XFT)
    XftDraw *xft_hd;
#else
    Qt::HANDLE xft_hd;
#endif
    GC gc;
    GC gc_brush;

    QColor bg_col;
    uchar bg_mode;
    QPen cpen;
    QBrush cbrush;
    QBrush bg_brush;
    QRegion crgn;
    int tabstops;
    int *tabarray;
    int tabarraylen;

    void *penRef;
    void *brushRef;
    void *ps_stack;
    void *wm_stack;
    uint clip_serial;
    QX11Info *xinfo;
    QPoint bg_origin;
    QPainter::TransformationCodes txop;
};

#endif // QPAINTENGINE_X11_P_H
