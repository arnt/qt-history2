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

#ifndef QPAINTENGINE_OPENGL_H
#define QPAINTENGINE_OPENGL_H

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

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPoint &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateXForm(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawLineSegments(const QPointArray &pa);
    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::PixmapDrawingMode mode);
    void drawPoint(const QPoint &p);
    void drawPolyInternal(const QPointArray &pa, bool close = true);
    void drawPolygon(const QPointArray &pa, PolygonDrawMode mode);
    void drawRect(const QRect &r);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s,
			 Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);

#ifdef Q_WS_WIN
    HDC handle() const;
#else
    Qt::HANDLE handle() const;
#endif
    inline Type type() const { return QPaintEngine::OpenGL; }

private:
#if defined(Q_DISABLE_COPY)
    QOpenGLPaintEngine(const QOpenGLPaintEngine &);
    QOpenGLPaintEngine &operator=(const QOpenGLPaintEngine &);
#endif
};

#endif
