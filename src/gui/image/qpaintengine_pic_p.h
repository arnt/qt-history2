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

#ifndef QPAINTENGINE_PIC_P_H
#include "qpaintengine.h"

class QPicturePaintEnginePrivate;
class QBuffer;

class QPicturePaintEngine : public QPaintEngine, public QPaintCommands
{
    Q_DECLARE_PRIVATE(QPicturePaintEngine)
public:
    QPicturePaintEngine();
    ~QPicturePaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &origin);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnbled);

    void drawLine(const QLineF &line);
    void drawRect(const QRectF &r);
    void drawPoint(const QPointF &p);
    void drawEllipse(const QRectF &r);
    void drawPolygon(const QPolygon &p, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                    Qt::PixmapDrawingMode mode);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
			 Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &ti, int textflags);

    Type type() const { return Picture; }

protected:
    QPicturePaintEngine(QPaintEnginePrivate &dptr);

private:
    Q_DISABLE_COPY(QPicturePaintEngine)

    void writeCmdLength(int pos, const QRectF &r, bool corr);
};

#endif // QPAINTENGINE_PIC_P_H
