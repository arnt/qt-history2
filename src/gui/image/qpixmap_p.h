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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qpixmap.h>
#if defined(Q_WS_X11)
#include <qx11info_x11.h>
#endif

#if defined(Q_WS_WIN)
#include "qt_windows.h"
// Internal pixmap memory optimization class for Windows 9x
struct QMCPI;
#endif

struct QPixmapData { // internal pixmap data
    QPixmapData() : count(1) { }
    ~QPixmapData();

    void ref() { ++count; }
    bool deref() { return !--count; }
    uint count;

    int w, h;
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
#ifdef Q_OS_TEMP
    uchar *ppvBits; // Pointer to DIBSection bits
#endif
#elif defined(Q_WS_X11)
    void *maskgc;
    bool alpha;
    QX11Info xinfo;
    Qt::HANDLE xft_hd;
    Qt::HANDLE hd2; // sorted in the default display depth
    Qt::HANDLE x11ConvertToDefaultDepth();
#elif defined(Q_WS_MAC)
    bool alpha;
    void macSetAlpha(bool b);
    void macQDDisposeAlpha();
    void macQDUpdateAlpha();
    uint *pixels;
    uint nbytes;
    CGImageRef cg_data;
    GWorldPtr qd_data, qd_alpha;
#elif defined(Q_WS_QWS)
    int id;
    QRgb *clut;
    int numcols;
    int rw;
    int rh;
    bool hasAlpha;
#endif
    QPaintEngine *paintEngine;
#if !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
    Qt::HANDLE hd;
#endif


    static int allocCell(const QPixmap *p);
    static void freeCell(QPixmapData *data, bool terminate = false);

#ifdef Q_WS_WIN
    void releaseDC(HDC hdc) const;
#endif
};

#endif // QPIXMAP_P_H
