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


#if defined Q_WS_WIN

struct QPixmapData { // internal pixmap data
    QPixmapData() : count(1) { }
    ~QPixmapData(){};
    void ref() { ++count; }
    bool deref() { return !--count; }
    int count;
    bool bitmap;
    QImage image;
};

#else // non windows

struct QPixmapData { // internal pixmap data
    QPixmapData() : count(1) { }
    ~QPixmapData();

    void ref() { ++count; }
    bool deref() { return !--count; }
    int count;

    int w, h;
    short d;
    uint uninit:1;
    uint bitmap:1;
    int ser_no;
#if !defined(Q_WS_X11) && !defined(Q_WS_MAC)
    QBitmap *mask;
#endif
#if defined(Q_WS_X11)
    bool alpha;
    QX11Info xinfo;
    Qt::HANDLE x11_mask;
    Qt::HANDLE picture;
    Qt::HANDLE hd2; // sorted in the default display depth
    Qt::HANDLE x11ConvertToDefaultDepth();
#elif defined(Q_WS_MAC)
    uint has_alpha : 1, has_mask : 1;
    void macSetHasAlpha(bool b);
    void macGetAlphaChannel(QPixmap *) const;
    void macSetAlphaChannel(const QPixmap *);
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
#if !defined(Q_WS_MAC)
    Qt::HANDLE hd;
#endif
#ifdef Q_WS_X11
    QBitmap mask_to_bitmap() const;
    static Qt::HANDLE bitmap_to_mask(const QBitmap &, int screen);
#endif
    static int allocCell(const QPixmap *p);
    static void freeCell(QPixmapData *data, bool terminate = false);
};

#endif // Q_WS_WIN

#endif // QPIXMAP_P_H
