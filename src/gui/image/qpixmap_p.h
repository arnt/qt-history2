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

#ifndef QPIXMAP_P_H
#define QPIXMAP_P_H

#include <qpixmap.h>

struct QPixmapData { // internal pixmap data
    QPixmapData() : count(1) { }
    void ref() { ++count; }
    bool deref() { return !--count; }
    uint count;

    QCOORD w, h;
    short d;
    uint uninit:1;
    uint bitmap:1;
    uint selfmask:1;
#if defined(Q_WS_WIN)
    uint mcp:1;
#endif
    int ser_no;
    QBitmap *mask;
#if defined(Q_WS_WIN)
    QPixmap *maskpm;
    union {
        HBITMAP hbm; // if mcp == false
        QMCPI *mcpi; // if mcp == true
    } hbm_or_mcpi;
    uchar *realAlphaBits;
#ifdef Q_OS_TEMP
    uchar *ppvBits; // Pointer to DIBSection bits
#endif
#elif defined(Q_WS_X11)
    void *ximage;
    void *maskgc;
    QPixmap *alphapm;
    QX11Info xinfo;
    Qt::HANDLE xft_hd;
#elif defined(Q_WS_MAC)
    CGImageRef cgimage;
    QPixmap *alphapm;
#elif defined(Q_WS_QWS)
    int id;
    QRgb * clut;
    int numcols;
    int rw;
    int rh;
    bool hasAlpha;
#endif
    QPixmap::Optimization optim;
#if defined(Q_WS_WIN)
    HBITMAP old_hbm;
#endif
    QPaintEngine *paintEngine;
    Qt::HANDLE hd;
};


#endif

