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
#include "qpointarray.h"
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
class QPointArray;
class QTextItem;
class QMatrix;

class Q_GUI_EXPORT QPainter
{
    Q_DECLARE_PRIVATE(QPainter)

        public:
    enum TextDirection { Auto, RTL, LTR };
    enum RenderHint {
        LineAntialiasing = 0x01,
        TextAntialiasing = 0x02
    };

    Q_DECLARE_FLAGS(RenderHints, RenderHint);

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
    void setBrushOrigin(int x, int y);
    void setBrushOrigin(const QPoint &);

    void setBackground(const QBrush &bg);
    const QBrush &background() const;

    QRegion clipRegion() const;
    void setClipRect(const QRect &);
    inline void setClipRect(int x, int y, int w, int h);
    void setClipRegion(const QRegion &);
    void setClipping(bool enable);
    bool hasClipping() const;

    bool hasViewXForm() const;
    bool hasWorldXForm() const;

    void save();
    void restore();

#ifndef QT_NO_TRANSFORMATIONS
    void setWorldMatrix(const QMatrix &wm, bool combine=false);
    const QMatrix &worldMatrix() const;
    void setWorldXForm(bool enable);

    void setViewXForm(bool enable);
    QRect window() const;
    void setWindow(const QRect &window);
    void setWindow(int x, int y, int w, int h);
    QRect viewport() const;
    void setViewport(const QRect &viewport);
    void setViewport(int x, int y, int w, int h);

    void scale(double sx, double sy);
    void shear(double sh, double sv);
    void rotate(double a);
#endif
    inline void translate(const QPoint &offset);
    void translate(double dx, double dy);
    void resetXForm();
    double translationX() const;
    double translationY() const;

    // drawing functions
    void strokePath(const QPainterPath &path, const QPen &pen);
    void fillPath(const QPainterPath &path, const QBrush &brush);
    void drawPath(const QPainterPath &path);
    void setClipPath(const QPainterPath &path);

    inline void drawLine(int x1, int y1, int x2, int y2);
    void drawLine(const QPoint &p1, const QPoint &p2);
    inline void drawRect(int x1, int y1, int w, int h);
    void drawRect(const QRect &r);
    void drawRects(const QList<QRect> &rectangleList);
    inline void drawPoint(int x, int y);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa);
    inline void drawRoundRect(int x, int y, int w, int h, int = 25, int = 25);
    void drawRoundRect(const QRect &r, int = 25, int = 25);
    void drawEllipse(int x, int y, int w, int h);
    void drawEllipse(const QRect &r);
    inline void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawArc(const QRect &, int a, int alen);
    inline void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawPie(const QRect &, int a, int alen);
    inline void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawChord(const QRect &, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);

    void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &, int sx=0, int sy=0,
			 Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    void drawTiledPixmap(const QRect &, const QPixmap &, const QPoint & = QPoint(),
                         Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
#ifndef QT_NO_PICTURE
    void drawPicture(int x, int y, const QPicture &picture);
    void drawPicture(const QPoint &p, const QPicture &picture);
#endif

    void drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect,
                    Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                           int sx, int sy, int sw, int sh,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, const QPixmap &pm,
                           int sx, int sy, int sw, int sh,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    void drawPixmap(const QPoint &p, const QPixmap &pm, Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, const QPixmap &pm, Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    void drawPixmap(const QRect &r, const QPixmap &pm, Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    inline void drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                           Qt::PixmapDrawingMode mode = Qt::ComposePixmap);

    void drawText(const QPoint &p, const QString &s, TextDirection dir = Auto);
    inline void drawText(int x, int y, const QString &s, TextDirection dir = Auto);

    inline void drawText(int x, int y, int w, int h, int flags, const QString&, int len = -1,
                  QRect *br=0);
    void drawText(const QRect &, int flags, const QString&, int len = -1, QRect *br=0);

    /* ### QT_COMPAT ? */
    QRect boundingRect(int x, int y, int w, int h, int flags, const QString&, int len = -1);
    QRect boundingRect(const QRect &, int flags, const QString&, int len = -1);

    void drawTextItem(int x, int y, const QTextItem &ti, int textflags = 0);
    void drawTextItem(const QPoint& p, const QTextItem &ti, int textflags = 0);

    inline void drawEdges(int x, int y, int w, int h, Qt::RectangleEdges);
    void drawEdges(const QRect &rect, Qt::RectangleEdges);

    inline void fillRect(int x, int y, int w, int h, const QBrush &);
    void fillRect(const QRect &, const QBrush &);
    inline void eraseRect(int x, int y, int w, int h);
    void eraseRect(const QRect &);

    void map(int x, int y, int *rx, int *ry) const;
    QPoint xForm(const QPoint &) const; // map virtual -> deviceb
    QRect xForm(const QRect &) const;
    QPointArray xForm(const QPointArray &) const;
    QPointArray xForm(const QPointArray &, int index, int npoints) const;
    QPoint xFormDev(const QPoint &) const; // map device -> virtual
    QRect xFormDev(const QRect &) const;
    QPointArray xFormDev(const QPointArray &) const;
    QPointArray xFormDev(const QPointArray &, int index, int npoints) const;

    void setRenderHints(RenderHint hints);
    void clearRenderHints(RenderHint hints);
    RenderHints supportedRenderHints() const;
    RenderHints renderHints() const;

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
    inline QT_COMPAT void drawImage(const QPoint &p, const QImage &image, const QRect &sr,
                                    Qt::ImageConversionFlags flags = Qt::AutoColor)
        { QPixmap pm; pm.fromImage(image, flags); drawPixmap(p, pm, sr); }
    inline QT_COMPAT void drawImage(const QRect &r, const QImage &image)
        { drawPixmap(r, QPixmap(image)); }
    inline QT_COMPAT void drawImage(int x, int y, const QImage &image,
                                    int sx = 0, int sy = 0, int sw = -1, int sh = -1,
                                    Qt::ImageConversionFlags flags = Qt::AutoColor)
        { QPixmap pm; pm.fromImage(image, flags); drawPixmap(QPoint(x, y), pm, QRect(sx, sy, sw, sh)); }
    inline QT_COMPAT void drawImage(const QPoint &p, const QImage &image)
        { drawPixmap(p, QPixmap(image)); }
    QT_COMPAT void drawPoints(const QPointArray &pa, int index, int npoints = -1);
    QT_COMPAT void drawCubicBezier(const QPointArray &pa, int index = 0);


    static inline QT_COMPAT void redirect(QPaintDevice *pdev, QPaintDevice *replacement)
        { setRedirected(pdev, replacement); }
    static inline QT_COMPAT QPaintDevice *redirect(QPaintDevice *pdev)
        { return const_cast<QPaintDevice*>(redirected(pdev)); }
#endif

private:
    friend void qt_format_text(const QFont& font, const QRect &_r,
                               int tf, const QString& str, int len, QRect *brect,
                               int tabstops, int* tabarray, int tabarraylen,
                               QPainter* painter);

    enum TransformationCodes {
        TxNone = 0,
        TxTranslate = 1,
        TxScale = 2,
        TxRotShear = 3
    };

    void updateXForm();
    void updateInvXForm();
    void init();

    QPainterPrivate *d_ptr;

    friend class QFontEngine;
    friend class QFontEngineBox;
    friend class QFontEngineFT;
    friend class QFontEngineMac;
    friend class QFontEngineWin;
    friend class QFontEngineXLFD;
    friend class QFontEngineXft;
    friend class QMacCGContext;
    friend class QWSManager;
    friend class QPaintEngine;
    friend class QX11PaintEngine;
    friend class QX11PaintEnginePrivate;
    friend class QWin32PaintEngine;
    friend class QWin32PaintEnginePrivate;
    friend void qt_mac_set_port(const QPainter *p);
};

//
// functions
//

inline void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
    drawLine(QPoint(x1, y1), QPoint(x2, y2));
}

inline void QPainter::drawRect(int x, int y, int w, int h)
{
    drawRect(QRect(x, y, w, h));
}

inline void QPainter::drawPoint(int x, int y)
{
    drawPoint(QPoint(x, y));
}

inline void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    drawRoundRect(QRect(x, y, w, h), xRnd, yRnd);
}

inline void QPainter::drawEllipse(int x, int y, int w, int h)
{
    drawEllipse(QRect(x, y, w, h));
}

inline void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    drawArc(QRect(x, y, w, h), a, alen);
}

inline void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    drawPie(QRect(x, y, w, h), a, alen);
}

inline void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    drawChord(QRect(x, y, w, h), a, alen);
}

inline void QPainter::setClipRect(int x, int y, int w, int h)
{
    setClipRect(QRect(x, y, w, h));
}

inline void QPainter::setWindow(const QRect &r)
{
    setWindow(r.x(), r.y(), r.width(), r.height());
}

inline void QPainter::setViewport(const QRect &r)
{
    setViewport(r.x(), r.y(), r.width(), r.height());
}

inline void QPainter::eraseRect(int x, int y, int w, int h)
{
    eraseRect(QRect(x, y, w, h));
}

inline void QPainter::fillRect(int x, int y, int w,  int h, const QBrush &b)
{
    fillRect(QRect(x, y, w, h), b);
}

inline void QPainter::setBrushOrigin(int x, int y)
{
    setBrushOrigin(QPoint(x, y));
}

inline void QPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pm, int sx, int sy,
                                      Qt::PixmapDrawingMode mode)
{
    drawTiledPixmap(QRect(x, y, w, h), pm, QPoint(sx, sy), mode);
}

inline void QPainter::drawPicture(int x, int y, const QPicture &picture)
{
    drawPicture(QPoint(x, y), picture);
}


inline void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                 int sx, int sy, int sw, int sh, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRect(x, y, w, h), pm, QRect(sx, sy, sw, sh), mode);
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pm,
                                 int sx, int sy, int sw, int sh, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRect(x, y, -1, -1), pm, QRect(sx, sy, sw, sh), mode);
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr,
                                 Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRect(p.x(), p.y(), -1, -1), pm, sr, mode);
}

inline void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                 Qt::PixmapDrawingMode mode)
{
    drawPixmap(QRect(x, y, w, h), pm, mode);
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pm, Qt::PixmapDrawingMode mode)
{
    drawPixmap(QPoint(x, y), pm, mode);
}


inline QRect QPainter::boundingRect(const QRect &r, int flags,
                                    const QString&s, int len)
{
    return boundingRect(r.x(), r.y(), r.width(), r.height(), flags, s, len);
}

inline void QPainter::drawTextItem(int x, int y, const QTextItem &ti, int textflags)
{
    drawTextItem(QPoint(x, y), ti, textflags);
}

inline void QPainter::drawText(int x, int y, int w, int h, int flags, const QString &str,
                               int len, QRect *br)
{
    drawText(QRect(x, y, w, h), flags, str, len, br);
}

inline void QPainter::drawText(int x, int y, const QString &s, TextDirection dir)
{
    drawText(QPoint(x, y), s, dir);
}

inline void QPainter::drawEdges(int x, int y, int w, int h, Qt::RectangleEdges edges)
{
    drawEdges(QRect(x, y, w, h), edges);
}


inline void QPainter::translate(const QPoint &offset)
{
    translate(offset.x(), offset.y());
}

inline void QPainter::map(int x, int y, int *rx, int *ry) const
{
    QPoint p(x, y);
    p = xForm(p);
    *rx = p.x();
    *ry = p.y();
}

#endif // #ifndef QPAINTER_H
