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

#ifndef QPAINTER_H
#define QPAINTER_H

#include "qnamespace.h"
#include "qrect.h"
#include "qpoint.h"
#include "qpixmap.h"
#include "qimage.h"

#ifndef QT_INCLUDE_COMPAT
#include "qpolygon.h"
#include "qpen.h"
#include "qbrush.h"
#include "qmatrix.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#endif

class QBrush;
class QFontInfo;
class QFontMetrics;
class QPaintDevice;
class QPainterPath;
class QPainterPrivate;
class QPen;
class QPolygon;
class QTextItem;
class QMatrix;

class Q_GUI_EXPORT QPainter
{
    Q_DECLARE_PRIVATE(QPainter)

public:
    enum TextDirection { Auto, RTL, LTR };
    enum RenderHint {
        Antialiasing = 0x01,
        TextAntialiasing = 0x02
    };

    Q_DECLARE_FLAGS(RenderHints, RenderHint)

    QPainter();
    QPainter(QPaintDevice *);
    ~QPainter();

    QPaintDevice *device() const;

    bool begin(QPaintDevice *);
    bool end();
    bool isActive() const;

    void initFrom(const QWidget *widget);

    const QFont &font() const;
    void setFont(const QFont &f);

    QFontMetrics fontMetrics() const;
    QFontInfo fontInfo() const;

    void setPen(const QColor &color);
    void setPen(const QPen &pen);
    void setPen(Qt::PenStyle style);
    const QPen &pen() const;

    void setBrush(const QBrush &brush);
    void setBrush(Qt::BrushStyle style);
    const QBrush &brush() const;

    // attributes/modes
    void setBackgroundMode(Qt::BGMode mode);
    Qt::BGMode backgroundMode() const;

    QPoint brushOrigin() const;
    inline void setBrushOrigin(int x, int y);
    inline void setBrushOrigin(const QPoint &);
    void setBrushOrigin(const QPointF &);

    void setBackground(const QBrush &bg);
    const QBrush &background() const;

    // Clip functions
    QRegion clipRegion() const;
    QPainterPath clipPath() const;

    void setClipRect(const QRectF &, Qt::ClipOperation op = Qt::ReplaceClip);
    inline void setClipRect(const QRect &, Qt::ClipOperation op = Qt::ReplaceClip);
    inline void setClipRect(int x, int y, int w, int h, Qt::ClipOperation op = Qt::ReplaceClip);

    void setClipRegion(const QRegion &, Qt::ClipOperation op = Qt::ReplaceClip);

    void setClipPath(const QPainterPath &path, Qt::ClipOperation op = Qt::ReplaceClip);

    void setClipping(bool enable);
    bool hasClipping() const;

    void save();
    void restore();

    // XForm functions
#ifndef QT_NO_TRANSFORMATIONS
    void setMatrix(const QMatrix &matrix, bool combine = false);
    const QMatrix &matrix() const;
    const QMatrix &deviceMatrix() const;
    void resetMatrix();

    void setMatrixEnabled(bool enabled);
    bool matrixEnabled() const;

    void scale(double sx, double sy);
    void shear(double sh, double sv);
    void rotate(double a);
#endif

    inline void translate(const QPointF &offset);
    inline void translate(const QPoint &offset);
    void translate(double dx, double dy);

    QRect window() const;
    void setWindow(const QRect &window);
    inline void setWindow(int x, int y, int w, int h);

    QRect viewport() const;
    void setViewport(const QRect &viewport);
    inline void setViewport(int x, int y, int w, int h);

    void setViewTransformEnabled(bool enable);
    bool viewTransformEnabled() const;

    // drawing functions
    void strokePath(const QPainterPath &path, const QPen &pen);
    void fillPath(const QPainterPath &path, const QBrush &brush);
    void drawPath(const QPainterPath &path);

    void drawLine(const QLineF &line);
    inline void drawLine(int x1, int y1, int x2, int y2);
    inline void drawLine(const QPoint &p1, const QPoint &p2);
    inline void drawLine(const QPointF &p1, const QPointF &p2);

    void drawRects(const QRectF *rects, int rectCount);
    inline void drawRects(const QVector<QRectF> &rectangles);

    void drawRect(const QRectF &rect);
    inline void drawRect(int x1, int y1, int w, int h);
    inline void drawRect(const QRect &rect);

    void drawRoundRect(const QRectF &r, int xround = 25, int yround = 25);
    inline void drawRoundRect(int x, int y, int w, int h, int = 25, int = 25);
    inline void drawRoundRect(const QRect &r, int xround = 25, int yround = 25);

    void drawEllipse(const QRectF &r);
    inline void drawEllipse(const QRect &r);
    inline void drawEllipse(int x, int y, int w, int h);

    void drawPoint(const QPointF &pt);
    inline void drawPoint(const QPoint &p);
    inline void drawPoint(int x, int y);

    void drawPoints(const QPointF *points, int pointCount);
    inline void drawPoints(const QPolygonF &points);
    void drawPoints(const QPoint *points, int pointCount);
    inline void drawPoints(const QPolygon &points);

    void drawArc(const QRectF &rect, int a, int alen);
    inline void drawArc(const QRect &, int a, int alen);
    inline void drawArc(int x, int y, int w, int h, int a, int alen);

    void drawPie(const QRectF &rect, int a, int alen);
    inline void drawPie(int x, int y, int w, int h, int a, int alen);
    inline void drawPie(const QRect &, int a, int alen);

    void drawChord(const QRectF &rect, int a, int alen);
    inline void drawChord(int x, int y, int w, int h, int a, int alen);
    inline void drawChord(const QRect &, int a, int alen);

    void drawLines(const QLineF *lines, int lineCount);
    inline void drawLines(const QVector<QLineF> &lines);

    void drawPolyline(const QPointF *points, int pointCount);
    inline void drawPolyline(const QPolygonF &polyline);
    void drawPolyline(const QPoint *points, int pointCount);
    inline void drawPolyline(const QPolygon &polygon);

    void drawPolygon(const QPointF *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
    inline void drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule = Qt::OddEvenFill);
    void drawPolygon(const QPoint *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
    inline void drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule = Qt::OddEvenFill);

    void drawConvexPolygon(const QPointF *points, int pointCount);
    inline void drawConvexPolygon(const QPolygonF &polygon);
    void drawConvexPolygon(const QPoint *points, int pointCount);
    inline void drawConvexPolygon(const QPolygon &polygon);

    void drawTiledPixmap(const QRectF &rect, const QPixmap &pm, const QPointF &offset = QPointF(),
                         Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &, int sx=0, int sy=0,
			 Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawTiledPixmap(const QRect &, const QPixmap &, const QPoint & = QPoint(),
                         Qt::PixmapDrawingMode mode = Qt::ComposePixmap);

#ifndef QT_NO_PICTURE
    void drawPicture(const QPointF &p, const QPicture &picture);
    inline void drawPicture(int x, int y, const QPicture &picture);
    inline void drawPicture(const QPoint &p, const QPicture &picture);
#endif

    void drawPixmap(const QRectF &targetRect, const QPixmap &pixmap, const QRectF &sourceRect,
                    Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                           int sx, int sy, int sw, int sh,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, const QPixmap &pm,
                           int sx, int sy, int sw, int sh,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QPointF &p, const QPixmap &pm, const QRectF &sr,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QPointF &p, const QPixmap &pm,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QPoint &p, const QPixmap &pm,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, const QPixmap &pm,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QRect &r, const QPixmap &pm,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);

    void drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
                   Qt::ImageConversionFlags flags = Qt::AutoColor);
    inline void drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                          Qt::ImageConversionFlags flags = Qt::AutoColor);
    inline void drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                          Qt::ImageConversionFlags flags = Qt::AutoColor);
    inline void drawImage(const QPoint &p, const QImage &image, const QRect &sr,
                          Qt::ImageConversionFlags flags = Qt::AutoColor);
    inline void drawImage(const QRectF &r, const QImage &image);
    inline void drawImage(const QRect &r, const QImage &image);
    inline void drawImage(const QPointF &p, const QImage &image);
    inline void drawImage(const QPoint &p, const QImage &image);
    inline void drawImage(int x, int y, const QImage &image, int sx = 0, int sy = 0,
                          int sw = -1, int sh = -1, Qt::ImageConversionFlags flags = Qt::AutoColor);

    void drawText(const QRectF &, int flags, const QString&, int len = -1, QRectF *br=0);
    void drawText(const QRect &, int flags, const QString&, int len = -1, QRect *br=0);
    void drawText(const QPointF &p, const QString &s, TextDirection dir = Auto);
    inline void drawText(const QPoint &p, const QString &s, TextDirection dir = Auto);
    inline void drawText(int x, int y, const QString &s, TextDirection dir = Auto);
    inline void drawText(int x, int y, int w, int h, int flags, const QString&, int len = -1,
                         QRect *br=0);

    QRectF boundingRect(const QRectF &rect, int flags, const QString &text, int len = -1);
    QRect boundingRect(const QRect &rect, int flags, const QString &text, int len = -1);
    QRect boundingRect(int x, int y, int w, int h, int flags, const QString&, int len = -1);

    void drawTextItem(const QPointF &p, const QTextItem &ti);
    inline void drawTextItem(int x, int y, const QTextItem &ti);
    inline void drawTextItem(const QPoint &p, const QTextItem &ti);

    void fillRect(const QRectF &, const QBrush &);
    inline void fillRect(int x, int y, int w, int h, const QBrush &);
    inline void fillRect(const QRect &, const QBrush &);

    void eraseRect(const QRectF &);
    inline void eraseRect(int x, int y, int w, int h);
    inline void eraseRect(const QRect &);

    void setRenderHint(RenderHint hint, bool on = true);
    RenderHints supportedRenderHints() const;
    RenderHints renderHints() const;

    QPaintEngine *paintEngine() const;

    static void setRedirected(const QPaintDevice *device, QPaintDevice *replacement,
                              const QPoint& offset = QPoint());
    static QPaintDevice *redirected(const QPaintDevice *device, QPoint *offset = 0);
    static void restoreRedirected(const QPaintDevice *device);

#ifdef QT_COMPAT

    inline QT_COMPAT void setBackgroundColor(const QColor &color) { setBackground(color); }
    inline QT_COMPAT const QColor &backgroundColor() const { return background().color(); }
    inline QT_COMPAT void drawText(int x, int y, const QString &s, int pos, int len, TextDirection dir = Auto)
        { drawText(x, y, s.mid(pos, len), dir); }
    inline QT_COMPAT void drawText(const QPoint &p, const QString &s, int pos, int len, TextDirection dir = Auto)
        { drawText(p, s.mid(pos, len), dir); }
    inline QT_COMPAT void drawText(int x, int y, const QString &s, int len, TextDirection dir = Auto)
        { drawText(x, y, s.left(len), dir); }
    inline QT_COMPAT void drawText(const QPoint &p, const QString &s, int len, TextDirection dir = Auto)
        { drawText(p, s.left(len), dir); }
    inline QT_COMPAT bool begin(QPaintDevice *pdev, const QWidget *init)
        { bool ret = begin(pdev); initFrom(init); return ret; }
    QT_COMPAT void drawPoints(const QPolygon &pa, int index, int npoints = -1)
    { drawPoints(pa.data() + index, npoints == -1 ? pa.size() - index : npoints); }

    QT_COMPAT void drawCubicBezier(const QPolygon &pa, int index = 0);

    QT_COMPAT void drawLineSegments(const QPolygon &points, int index = 0, int nlines = -1);

    inline QT_COMPAT void drawPolyline(const QPolygon &pa, int index, int npoints = -1)
    { drawPolyline(pa.data() + index, npoints == -1 ? pa.size() - index : npoints); }

    inline QT_COMPAT void drawPolygon(const QPolygon &pa, bool winding, int index = 0, int npoints = -1)
    { drawPolygon(pa.data() + index, npoints == -1 ? pa.size() - index : npoints,
                  winding ? Qt::WindingFill : Qt::OddEvenFill); }

    inline QT_COMPAT void drawPolygon(const QPolygonF &polygon, bool winding, int index = 0,
                                      int npoints = -1)
    { drawPolygon(polygon.data() + index, npoints == -1 ? polygon.size() - index : npoints,
                  winding ? Qt::WindingFill : Qt::OddEvenFill); }

    inline QT_COMPAT void drawConvexPolygon(const QPolygonF &polygon, int index, int npoints = -1)
    { drawConvexPolygon(polygon.data() + index, npoints == -1 ? polygon.size() - index : npoints); }
    inline QT_COMPAT void drawConvexPolygon(const QPolygon &pa, int index, int npoints = -1)
    { drawConvexPolygon(pa.data() + index, npoints == -1 ? pa.size() - index : npoints); }

    static inline QT_COMPAT void redirect(QPaintDevice *pdev, QPaintDevice *replacement)
    { setRedirected(pdev, replacement); }
    static inline QT_COMPAT QPaintDevice *redirect(QPaintDevice *pdev)
    { return const_cast<QPaintDevice*>(redirected(pdev)); }

    inline QT_COMPAT void setWorldMatrix(const QMatrix &wm, bool combine=false) { setMatrix(wm, combine); }
    inline QT_COMPAT const QMatrix &worldMatrix() const { return matrix(); }
    inline QT_COMPAT void setWorldXForm(bool enabled) { setMatrixEnabled(enabled); }
    inline QT_COMPAT bool hasWorldXForm() const { return matrixEnabled(); }
    inline QT_COMPAT void resetXForm() { resetMatrix(); }

    inline QT_COMPAT void setViewXForm(bool enabled) { setViewTransformEnabled(enabled); }
    inline QT_COMPAT bool hasViewXForm() const { return viewTransformEnabled(); }

    QT_COMPAT void map(int x, int y, int *rx, int *ry) const;
    QT_COMPAT QPoint xForm(const QPoint &) const; // map virtual -> deviceb
    QT_COMPAT QRect xForm(const QRect &) const;
    QT_COMPAT QPolygon xForm(const QPolygon &) const;
    QT_COMPAT QPolygon xForm(const QPolygon &, int index, int npoints) const;
    QT_COMPAT QPoint xFormDev(const QPoint &) const; // map device -> virtual
    QT_COMPAT QRect xFormDev(const QRect &) const;
    QT_COMPAT QPolygon xFormDev(const QPolygon &) const;
    QT_COMPAT QPolygon xFormDev(const QPolygon &, int index, int npoints) const;
    QT_COMPAT double translationX() const;
    QT_COMPAT double translationY() const;
#endif

private:
    friend class Q3Painter;
    friend void qt_format_text(const QFont &font,
                               const QRectF &_r, int tf, const QString& str, int len, QRectF *brect,
                               int tabstops, int* tabarray, int tabarraylen,
                               QPainter *painter);

    QPainterPrivate *d_ptr;

    friend class QFontEngine;
    friend class QFontEngineBox;
    friend class QFontEngineFT;
    friend class QFontEngineMac;
    friend class QFontEngineWin;
    friend class QFontEngineXLFD;
    friend class QFontEngineXft;
    friend class QWSManager;
    friend class QPaintEngine;
    friend class QX11PaintEngine;
    friend class QX11PaintEnginePrivate;
    friend class QWin32PaintEngine;
    friend class QWin32PaintEnginePrivate;
};

//
// functions
//

inline void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
    drawLine(QLineF(x1, y1, x2, y2));
}

inline void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
{
    drawLine(QLineF(p1, p2));
}

inline void QPainter::drawLine(const QPointF &p1, const QPointF &p2)
{
    drawLine(QLineF(p1, p2));
}

inline void QPainter::drawLines(const QVector<QLineF> &lines)
{
    drawLines(lines.data(), lines.size());
}

inline void QPainter::drawPolyline(const QPolygonF &polyline)
{
    drawPolyline(polyline.data(), polyline.size());
}

inline void QPainter::drawPolyline(const QPolygon &polyline)
{
    drawPolyline(polyline.data(), polyline.size());
}

inline void QPainter::drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule)
{
    drawPolygon(polygon.data(), polygon.size(), fillRule);
}

inline void QPainter::drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule)
{
    drawPolygon(polygon.data(), polygon.size(), fillRule);
}

inline void QPainter::drawConvexPolygon(const QPolygonF &poly)
{
    drawConvexPolygon(poly.data(), poly.size());
}

inline void QPainter::drawConvexPolygon(const QPolygon &poly)
{
    drawConvexPolygon(poly.data(), poly.size());
}

inline void QPainter::drawRect(int x, int y, int w, int h)
{
    drawRect(QRectF(x, y, w, h));
}

inline void QPainter::drawRect(const QRect &r)
{
    drawRect(QRectF(r));
}

inline void QPainter::drawRects(const QVector<QRectF> &rects)
{
    drawRects(rects.data(), rects.size());
}

inline void QPainter::drawPoint(int x, int y)
{
    drawPoint(QPointF(x, y));
}

inline void QPainter::drawPoint(const QPoint &p)
{
    drawPoint(QPointF(p));
}

inline void QPainter::drawPoints(const QPolygonF &points)
{
    drawPoints(points.data(), points.size());
}

inline void QPainter::drawPoints(const QPolygon &points)
{
    drawPoints(points.data(), points.size());
}

inline void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    drawRoundRect(QRectF(x, y, w, h), xRnd, yRnd);
}

inline void QPainter::drawRoundRect(const QRect &rect, int xRnd, int yRnd)
{
    drawRoundRect(QRectF(rect), xRnd, yRnd);
}

inline void QPainter::drawEllipse(const QRect &rect)
{
    drawEllipse(QRectF(rect));
}

inline void QPainter::drawEllipse(int x, int y, int w, int h)
{
    drawEllipse(QRectF(x, y, w, h));
}

inline void QPainter::drawArc(const QRect &r, int a, int alen)
{
    drawArc(QRectF(r), a, alen);
}

inline void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    drawArc(QRectF(x, y, w, h), a, alen);
}

inline void QPainter::drawPie(const QRect &rect, int a, int alen)
{
    drawPie(QRectF(rect), a, alen);
}

inline void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    drawPie(QRectF(x, y, w, h), a, alen);
}

inline void QPainter::drawChord(const QRect &rect, int a, int alen)
{
    drawChord(QRectF(rect), a, alen);
}

inline void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    drawChord(QRectF(x, y, w, h), a, alen);
}

inline void QPainter::setClipRect(int x, int y, int w, int h, Qt::ClipOperation op)
{
    setClipRect(QRectF(x, y, w, h), op);
}

inline void QPainter::setClipRect(const QRect &rect, Qt::ClipOperation op)
{
    setClipRect(QRectF(rect), op);
}

inline void QPainter::eraseRect(const QRect &rect)
{
    eraseRect(QRectF(rect));
}

inline void QPainter::eraseRect(int x, int y, int w, int h)
{
    eraseRect(QRectF(x, y, w, h));
}

inline void QPainter::fillRect(int x, int y, int w,  int h, const QBrush &b)
{
    fillRect(QRectF(x, y, w, h), b);
}

inline void QPainter::fillRect(const QRect &rect, const QBrush &b)
{
    fillRect(QRectF(rect), b);
}

inline void QPainter::setBrushOrigin(int x, int y)
{
    setBrushOrigin(QPoint(x, y));
}

inline void QPainter::setBrushOrigin(const QPoint &p)
{
    setBrushOrigin(QPointF(p));
}

inline void QPainter::drawTiledPixmap(const QRect &rect, const QPixmap &pm, const QPoint &offset,
                                      Qt::PixmapDrawingMode mode)
{
    drawTiledPixmap(QRectF(rect), pm, QPointF(offset), mode);
}

inline void QPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pm, int sx, int sy,
                                      Qt::PixmapDrawingMode mode)
{
    drawTiledPixmap(QRectF(x, y, w, h), pm, QPointF(sx, sy), mode);
}

inline void QPainter::drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect,
                                 Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(targetRect), pixmap, QRectF(sourceRect), mode);
}

inline void QPainter::drawPixmap(const QPointF &p, const QPixmap &pm, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(p.x(), p.y(), -1, -1), pm, QRectF(), mode);
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(p.x(), p.y(), -1, -1), pm, QRectF(), mode);
}

inline void QPainter::drawPixmap(const QRect &r, const QPixmap &pm, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(r), pm, QRectF(), mode);
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pm, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(x, y, -1, -1), pm, QRectF(), mode);
}

inline void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                 Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(x, y, w, h), pm, QRectF(), mode);
}

inline void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                 int sx, int sy, int sw, int sh, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh), mode);
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pm,
                                 int sx, int sy, int sw, int sh, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(x, y, -1, -1), pm, QRectF(sx, sy, sw, sh), mode);
}

inline void QPainter::drawPixmap(const QPointF &p, const QPixmap &pm, const QRectF &sr,
                                 Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(p.x(), p.y(), -1, -1), pm, sr, mode);
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr,
                                 Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRectF(p.x(), p.y(), -1, -1), pm, sr, mode);
}

inline QRect QPainter::boundingRect(int x, int y, int w, int h, int flags,
                                    const QString&s, int len)
{
    return boundingRect(QRect(x, y, w, h), flags, s, len);
}

inline void QPainter::drawTextItem(int x, int y, const QTextItem &ti)
{
    drawTextItem(QPointF(x, y), ti);
}

inline void QPainter::drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                                Qt::ImageConversionFlags flags)
{
    drawImage(QRectF(targetRect), image, QRectF(sourceRect), flags);
}

inline void QPainter::drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                                Qt::ImageConversionFlags flags)
{
    drawImage(QRectF(p.x(), p.y(), -1, -1), image, sr, flags);
}

inline void QPainter::drawImage(const QPoint &p, const QImage &image, const QRect &sr,
                                Qt::ImageConversionFlags flags)
{
    drawImage(QRect(p.x(), p.y(), -1, -1), image, sr, flags);
}


inline void QPainter::drawImage(const QRectF &r, const QImage &image)
{
    drawImage(r, image, QRect(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QRect &r, const QImage &image)
{
    drawImage(r, image, QRectF(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QPointF &p, const QImage &image)
{
    drawImage(QRectF(p.x(), p.y(), -1, -1), image, QRectF(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QPoint &p, const QImage &image)
{
    drawImage(QRectF(p.x(), p.y(), -1, -1), image, QRectF(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(int x, int y, const QImage &image, int sx, int sy, int sw, int sh,
                                Qt::ImageConversionFlags flags)
{
    drawImage(QRectF(x, y, -1, -1), image, QRectF(sx, sy, sw, sh), flags);
}

inline void QPainter::drawTextItem(const QPoint &p, const QTextItem &ti)
{
    drawTextItem(QPointF(p), ti);
}

inline void QPainter::drawText(const QPoint &p, const QString &s, TextDirection dir)
{
    drawText(QPointF(p), s, dir);
}

inline void QPainter::drawText(int x, int y, int w, int h, int flags, const QString &str,
                               int len, QRect *br)
{
    drawText(QRect(x, y, w, h), flags, str, len, br);
}

inline void QPainter::drawText(int x, int y, const QString &s, TextDirection dir)
{
    drawText(QPointF(x, y), s, dir);
}

inline void QPainter::translate(const QPointF &offset)
{
    translate(offset.x(), offset.y());
}

inline void QPainter::translate(const QPoint &offset)
{
    translate(offset.x(), offset.y());
}

inline void QPainter::setViewport(int x, int y, int w, int h)
{
    setViewport(QRect(x, y, w, h));
}

inline void QPainter::setWindow(int x, int y, int w, int h)
{
    setWindow(QRect(x, y, w, h));
}

#ifndef QT_NO_PICTURE
inline void QPainter::drawPicture(int x, int y, const QPicture &p)
{
    drawPicture(QPoint(x, y), p);
}

inline void QPainter::drawPicture(const QPoint &pt, const QPicture &p)
{
    drawPicture(QPointF(pt), p);
}
#endif

#endif // #ifndef QPAINTER_H
