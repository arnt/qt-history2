/****************************************************************************
**
** Definition of QQuickDrawGC/QCoreGraphicsGC class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGC_MAC_H
#define QGC_MAC_H
#include "qabstractgc.h"
#include "qshared.h"
#ifdef Q_WS_MAC //just for now (to get the coregraphics switch) ###
# include "qt_mac.h"
#endif

class QQuickDrawGCPrivate;
class QPainterState;

class QQuickDrawGC : public QAbstractGC
{
public:
    QQuickDrawGC(const QPaintDevice *);
    ~QQuickDrawGC();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void setRasterOp(RasterOp r);

    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int w, int h);
    void drawPoint(int x, int y);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &penColor);
    void drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd);
    void drawEllipse(int x, int y, int w, int h);
    void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif

    void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh);
    void drawTextItem(int x, int y, const QTextItem &ti, int textflags);
    void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx, int sy, bool optim);

    virtual Qt::HANDLE handle() const;

    inline Type type() const { return QAbstractGC::QuickDraw; }
    static void initialize();
    static void cleanup();

protected:
    void drawPolyInternal(const QPointArray &a, bool close=true, bool inset=true);
    void setClippedRegionInternal(QRegion *);

    void setupQDFont();
    void setupQDBrush();
    void setupQDPen();
    void setupQDPort(bool force=false, QPoint *off=NULL, QRegion *rgn=NULL);

    friend class QMacStylePainter;
    friend class QFontEngineMac;

private:
    QQuickDrawGCPrivate *d;

private:
#if defined(Q_DISABLE_COPY)
    QQuickDrawGC(const QQuickDrawGC &);
    QQuickDrawGC &operator=(const QQuickDrawGC &);
#endif
};

#ifdef USE_CORE_GRAPHICS
class QCoreGraphicsGCPrivate;
class QCoreGraphicsGC : public QAbstractGC
{
public:
    QCoreGraphicsGC(const QPaintDevice *);
    ~QCoreGraphicsGC();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void setRasterOp(RasterOp r);

    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int w, int h);
    void drawPoint(int x, int y);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &penColor);
    void drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd);
    void drawEllipse(int x, int y, int w, int h);
    void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif
    void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh);
    void drawTextItem(int x, int y, const QTextItem &ti, int textflags);
    void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx, int sy, bool optim);

    virtual Qt::HANDLE handle() const;

    inline Type type() const { return QAbstractGC::CoreGraphics; }
    static void initialize();
    static void cleanup();

protected:
    void drawPolyInternal(const QPointArray &a, bool close=true);

private:
    QCoreGraphicsGCPrivate *d;

private:
#if defined(Q_DISABLE_COPY)
    QCoreGraphicsGC(const QCoreGraphicsGC &);
    QCoreGraphicsGC &operator=(const QCoreGraphicsGC &);
#endif
};
#endif

#endif
