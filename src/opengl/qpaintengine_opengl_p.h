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

#ifndef QPAINTENGINE_OPENGL_P_H
#define QPAINTENGINE_OPENGL_P_H

#include "qpaintengine.h"

class QOpenGLPaintEnginePrivate;

class QOpenGLPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QOpenGLPaintEngine)
public:
    QOpenGLPaintEngine();
    ~QOpenGLPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawRects(const QRectF *r, int rectCount);
    void drawLines(const QLineF *lines, int lineCount);
    void drawPoints(const QPointF *p, int pointCount);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);

    void drawPath(const QPainterPath &path);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    inline void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
        { QPaintEngine::drawPolygon(points, pointCount, mode); }
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
    void drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                   Qt::ImageConversionFlags conversionFlags);
    void drawTextItem(const QPointF &p, const QTextItem &ti);

#ifdef Q_WS_WIN
    HDC handle() const;
#else
    Qt::HANDLE handle() const;
#endif
    inline Type type() const { return QPaintEngine::OpenGL; }

private:
    void drawPolyInternal(const QPolygonF &pa, bool close = true);
    void drawTextureRect(int tx_width, int tx_height, const QRectF &r, const QRectF &sr, GLenum target);
    Q_DISABLE_COPY(QOpenGLPaintEngine)
};

#endif // QPAINTENGINE_OPENGL_P_H
