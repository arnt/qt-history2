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

#ifndef QPAINTENGINE_QWS_H
#define QPAINTENGINE_QWS_H

#include "qatomic.h"
#include "qpaintengine.h"

class QWSPaintEnginePrivate;
class QPainterState;
class QApplicationPrivate;
class QScreen;

class QWSPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QWSPaintEngine)

public:
    QWSPaintEngine();
    ~QWSPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool begin(QImage *img);
    bool begin(QScreen *screen);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPoint &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);

    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa);
    void drawEllipse(const QRect &r);
    void drawPolygon(const QPointArray &pa, PolygonDrawMode mode);

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::PixmapDrawingMode mode);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, Qt::PixmapDrawingMode mode);

    virtual Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::QWindowSystem; }

    static void initialize();
    static void cleanup();


    //#### QGfx/QGfxRaster methods moved directly. Should clean up API.
#if 1
    void setGlobalRegionIndex(int idx);
    void setWidgetDeviceRegion(const QRegion &);
    void setClipDeviceRegion(const QRegion &);


    void scroll(int rx,int ry,int w,int h,int sx, int sy);

    void fillRect(int rx,int ry,int w,int h);

    void blt(const QPaintDevice &src, int rx,int ry,int w,int h, int sx, int sy);
    void blt(const QImage &src, int rx,int ry,int w,int h, int sx, int sy);
    void stretchBlt(const QPaintDevice &src, int rx,int ry,int w,int h, int sw,int sh);
    void alphaPenBlt(const void* src, int bpl, bool mono, int rx,int ry,int w,int h, int sx, int sy);
    void tiledBlt(const QImage &src, int rx,int ry,int w,int h, int sx, int sy);

#endif

protected:
    QWSPaintEngine(QPaintEnginePrivate &dptr);

    void drawPolyInternal(const QPointArray &a, bool close=true);

    void copyQWSData(const QWSPaintEngine *);
    void cloneQWSData(const QWSPaintEngine *);

    friend void qt_init(QApplicationPrivate *, int);
    friend void qt_cleanup();
    friend void qt_draw_transformed_rect(QPainter *pp,  int x, int y, int w,  int h, bool fill);
    friend void qt_draw_background(QPainter *pp, int x, int y, int w,  int h);
    friend class QWidget;
    friend class QPixmap;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;

private:
    friend class QWSServer;
    friend class QFontEngine;

private:
    Q_DISABLE_COPY(QWSPaintEngine)
};

#endif
