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

#if defined(Q_WS_WIN)
// Internal pixmap memory optimization class for Windows 9x
struct QMCPI;
#endif

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
    };
    inline HBITMAP bm() const;
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
    QPaintEngine *paintEngine;
#ifndef Q_WS_WIN
    Qt::HANDLE hd;
#else
    struct MemDC {
        MemDC() {
            hdc = 0;
            ref = 0;
            bm = 0;
        }
        HDC hdc;
        int ref;
        HGDIOBJ bm;
    };
    MemDC mem_dc;
#endif

    static int allocCell(const QPixmap *p);
    static void freeCell(const QPixmap *p, bool terminate = false);
};


#endif

