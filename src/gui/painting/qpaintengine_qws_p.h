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

#ifndef QPAINTENGINE_QWS_P_H
#define QPAINTENGINE_QWS_P_H

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

#include "qatomic.h"
#include "qpaintengine.h"
#include <private/qpaintengine_p.h>

class QPainterState;
class QApplicationPrivate;
class QScreen;
class QGfx;

class QWSPaintEnginePrivate;

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
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);

    void drawLines(const QLineF *lines, int lineCount);
    void drawRects(const QRectF *rects, int rectCount);
    void drawPoints(const QPointF *p, int pointCount);
    void drawEllipse(const QRectF &r);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr, Qt::PixmapDrawingMode mode);

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

    void drawPolyInternal(const QPolygon &a, bool close=true);

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
    friend class QDirectPainter;

private:
    Q_DISABLE_COPY(QWSPaintEngine)
};


class QWSPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWSPaintEngine)
public:
    QWSPaintEnginePrivate() :gfx(0), clipChildren(true) {}
    QGfx *gfx;
    bool clipChildren;
};

#endif // QPAINTENGINE_QWS_P_H
