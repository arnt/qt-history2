/****************************************************************************
**
** Definition of the QSVGPaintEngine class.
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

#ifndef QPAINTENGINE_SVG_P_H

#include "qdom.h"
#include "qpaintengine.h"
#include "private/qpicture_p.h" // for QPaintCommands

#if !defined(QT_MODULE_XML) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_XML )
#define QM_EXPORT_SVG
#else
#define QM_EXPORT_SVG Q_XML_EXPORT
#endif

class QSVGPaintEnginePrivate;

class QSVGPaintEngine : public QPaintEngine, public QPaintCommands {
public:
    QSVGPaintEngine();
    ~QSVGPaintEngine();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped = FALSE);
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
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor);
    void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    void drawEllipse(const QRect &r);
    void drawArc(const QRect &r, int a, int alen);
    void drawPie(const QRect &r, int a, int alen);
    void drawChord(const QRect &r, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1);
    void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &, int index = 0);
#endif

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);
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

private:
    QSVGPaintEnginePrivate *d;

    void appendChild(QDomElement &e, PaintCommand c);
    void applyStyle(QDomElement *e, PaintCommand c) const;
    void applyTransform(QDomElement *e) const;
    double parseLen(const QString &str, bool *ok=0, bool horiz=true) const;
    int lenToInt(const QDomNamedNodeMap &map, const QString &attr, int def = 0) const;
    double lenToDouble(const QDomNamedNodeMap &map, const QString &attr, int def = 0) const;
    bool play(const QDomNode &node, QPainter *p);
    void setTransform(const QString &tr, QPainter *p);
    void restoreAttributes(QPainter *p);
    void saveAttributes(QPainter *p);
    void setStyle(const QString &s, QPainter *p);
    void setStyleProperty(const QString &prop, const QString &val, QPen *pen, QFont *font,
			  int *talign, QPainter *p);
    void drawPath(const QString &data, QPainter *p);
    QColor parseColor(const QString &col);

    private:
#if defined(Q_DISABLE_COPY)
    QSVGPaintEngine(const QSVGPaintEngine &);
    QSVGPaintEngine &operator=(const QSVGPaintEngine &);
#endif
};
#endif // QPAINTENGINE_SVG_P_H
