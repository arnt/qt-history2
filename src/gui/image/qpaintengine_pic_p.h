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
    void updateBrush(const QBrush &brush, const QPoint &origin);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateXForm(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnbled);

    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawEllipse(const QRect &r);
    void drawPolygon(const QPointArray &pa, PolygonDrawMode mode);

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr,
                    Qt::PixmapDrawingMode mode);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s,
			 Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);

    Type type() const { return Picture; }

protected:
    QPicturePaintEngine(QPaintEnginePrivate &dptr);

private:
    Q_DISABLE_COPY(QPicturePaintEngine)

    void writeCmdLength(int pos, const QRect &r, bool corr);
};

#endif // QPAINTENGINE_PIC_P_H
