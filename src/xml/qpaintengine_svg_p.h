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

#ifndef QPAINTENGINE_SVG_P_H
#define QPAINTENGINE_SVG_P_H

#include "qdom.h"
#include "qpaintengine.h"
#include "private/qpicture_p.h" // for QPaintCommands

class QSVGPaintEnginePrivate;

class QSVGPaintEngine : public QPaintEngine, public QPaintCommands
{
    Q_DECLARE_PRIVATE(QSVGPaintEngine)

public:
    QSVGPaintEngine();
    ~QSVGPaintEngine();

    bool begin(QPaintDevice *pdev, QPainterState *state, bool unclipped = false);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPolygon &pa, int index = 0, int npoints = -1);
    void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    void drawEllipse(const QRect &r);
    void drawArc(const QRect &r, int a, int alen);
    void drawPie(const QRect &r, int a, int alen);
    void drawChord(const QRect &r, int a, int alen);
    void drawLineSegments(const QPolygon &, int index = 0, int nlines = -1);
    void drawPolyline(const QPolygon &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPolygon &pa, bool winding = false, int index = 0, int npoints = -1);
    void drawConvexPolygon(const QPolygon &, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPolygon &, int index = 0);
#endif

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, QPainter::PixmapDrawingMode mode);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s);
    void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);

#if defined Q_WS_WIN // ### not liking this!!
    HDC handle() const { return 0; }
#else
    Qt::HANDLE handle() const {return 0; }
#endif
    Type type() const { return SVG; }
    bool play(QPainter *p);

    QString toString() const;

    bool load(QIODevice *dev);
    bool save(QIODevice *dev);
    bool save(const QString &fileName);

    QRect boundingRect() const;
    void setBoundingRect(const QRect &r);

protected:
    QSVGPaintEngine(QSVGPaintEnginePrivate &dptr);

private:
    Q_DISABLE_COPY(QSVGPaintEngine);
};

#endif
