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

#include "qpaintengine.h"
#include "qpaintengine_p.h"
#include "qpainter_p.h"
#include "qpolygon.h"
#include "qbitmap.h"
#include "qapplication.h"
#include <qdebug.h>
#include <private/qtextengine_p.h>

#include <qvarlengtharray.h>

qreal QTextItem::descent() const
{
    const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
    return ti->descent;
}

qreal QTextItem::ascent() const
{
    const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
    return ti->ascent;
}

qreal QTextItem::width() const
{
    const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
    return ti->width;
}

QTextItem::RenderFlags QTextItem::renderFlags() const
{
    const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
    return ti->flags;
}

QString QTextItem::text() const
{
    const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
    return QString(ti->chars, ti->num_chars);
}

QFont QTextItem::font() const
{
    const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
    return ti->f ? *ti->f : QApplication::font();
}


#include <private/qfontengine_p.h>

/*! \class QPaintEngine qpaintengine.h
  \brief The QPaintEngine class provides an abstract definition of how
  QPainter draws to a given device on a given platform.

  Qt 4.0 provides several premade implementations of QPaintEngine for the
  different painter backends we support. We provide one paint engine for each
  window system and painting framework we support. This includes X11 on
  Unix/Linux and CoreGraphics on Mac OS X. In addition we provide QPaintEngine
  implementations for OpenGL (accessible through QGLWidget) and PostScript
  (accessible through QPSPrinter on X11). Additionally there is a raster-based
  paint engine that is a fallback for when an engine does not support a certain
  capability.

  If one wants to use QPainter to draw to a different backend, such as
  PDF, one must subclass QPaintEngine and reimplement all its virtual
  functions. The QPaintEngine implementation is then made available by
  subclassing QPaintDevice and reimplementing the virtual function \c
  QPaintDevice::paintEngine().

  QPaintEngine is created and owned by the QPaintDevice that created it.

  The big advantage of the QPaintEngine approach opposed to
  Qt 3's QPainter/QPaintDevice::cmd() approach is that it is now
  possible to adapt to multiple technologies on each platform and take
  advantage of each to the fullest.

  \sa QPainter, QPaintDevice::paintEngine()
*/

/*!
  \enum QPaintEngine::PaintEngineFeature

  This enum is used to describe the features or capabilities that the
  paint engine has. If a feature is not supported by the engine,
  QPainter will do a best effort to emulate that feature through other
  means and pass on an alpha blended QImage to the engine with the
  emulated results. Some features cannot be emulated: AlphaBlend and PorterDuff.

  \value PrimitiveTransform The engine has support for transforming
                            drawing primitives.
  \value PatternTransform   The engine has support for transforming brush
                            patterns.
  \value PixmapTransform    The engine can transform pixmaps, including
                            rotation and shearing.
  \value PatternBrush       The engine is capable of rendering brushes with
                            the brush patterns specified in Qt::BrushStyle.
  \value LinearGradientFill The engine supports linear gradient fills.
  \value RadialGradientFill The engine supports radial gradient fills.
  \value ConicalGradientFill The engine supports conical gradient fills.
  \value AlphaBlend         The engine can alpha blend primitives.
  \value PorterDuff         The engine supports Porter-Duff operations
  \value PainterPaths       The engine has path support.
  \value Antialiasing       The engine can use antialising to improve the appearance
                            of rendered primitives.
  \value BrushStroke
  \value PaintOutsidePaintEvent The engine is capable of painting outside of
                                paint events.
  \value AllFeatures
  \omitvalue UsesFontEngine
*/

/*!
  \enum QPaintEngine::DirtyFlags

  \internal

  This enum is used by QPainter to trigger lazy updates of the various states
  in the QPaintEngine
*/

/*!
    \enum QPaintEngine::PolygonDrawMode

    \value OddEvenMode The polygon should be drawn using OddEven fill
    rule.

    \value WindingMode The polygon should be drawn using Winding fill rule.

    \value ConvexMode The polygon is a convex polygon and can be drawn
    using specialized algorithms where available.

    \value PolylineMode Only the outline of the polygon should be
    drawn.

*/

/*!
    \enum QPaintEngine::DirtyFlag

    \internal
*/

/*!
    \fn void QPaintEngine::syncState()

    \internal

    Updates all dirty states in this engine. This function should ONLY
    be used when drawing with native handles directly and immediate sync
    from QPainters state to the native state is required.
*/

static int qt_polygon_recursion;
struct QT_Point {
    int x;
    int y;
};

/*!
    \fn void QPaintEngine::drawPolygon(const QPointF *points, int pointCount,
    PolygonDrawMode mode)

    Reimplement this virtual function to draw the polygon defined
    by the \a pointCount first points in \a points, using mode \a
    mode.

    The default implementation of this function will try to use drawPath
    if the engine supports the feature QPaintEngine::PainterPaths or try
    the float based drawPolygon() implementation if not.
*/
void QPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT_X(!qt_polygon_recursion, "QPaintEngine::drawPolygon",
               "At least one drawPolygon function must be implemented");
    qt_polygon_recursion = 1;
    Q_ASSERT(sizeof(QT_Point) == sizeof(QPoint));
    QVarLengthArray<QT_Point> p(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        p[i].x = qRound(points[i].x());
        p[i].y = qRound(points[i].y());
    }
    drawPolygon((QPoint *)p.data(), pointCount, mode);
    qt_polygon_recursion = 0;
}

struct QT_PointF {
    qreal x;
    qreal y;
};
/*!
    \overload

    Reimplement this virtual function to draw the polygon defined by the
    \a pointCount first points in \a points, using mode \a mode.

    The default implementation of this function will try to use drawPath()
    if the engine supports the feature QPaintEngine::PainterPaths or try
    the int based drawPolygon() implementation if not.
*/
void QPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT_X(!qt_polygon_recursion, "QPaintEngine::drawPolygon",
               "At least one drawPolygon function must be implemented");
    qt_polygon_recursion = 1;
    Q_ASSERT(sizeof(QT_PointF) == sizeof(QPointF));
    QVarLengthArray<QT_PointF> p(pointCount);
    for (int i=0; i<pointCount; ++i) {
        p[i].x = points[i].x();
        p[i].y = points[i].y();
    }
    drawPolygon((QPointF *)p.data(), pointCount, mode);
    qt_polygon_recursion = 0;
}

/*!
    \enum QPaintEngine::Type

    \value X11
    \value Windows
    \value MacPrinter
    \value CoreGraphics Mac OS X's Quartz2D (CoreGraphics)
    \value QuickDraw Mac OS X's older QuickDraw-based painting
    \value QWindowSystem Qt/Embedded
    \value PostScript
    \value OpenGL
    \value Picture QPicture format
    \value SVG Scalable Vector Graphics XML format
    \value Raster
    \value User First user type ID
    \value MaxUser Last user type ID
*/

/*!
    \fn bool QPaintEngine::isActive() const

    Returns true if the paint engine is actively drawing; otherwise
    returns false.

    \sa setActive()
*/

/*!
    \fn void QPaintEngine::setActive(bool state)

    Sets the active state of the paint engine to \a state.

    \sa isActive()
*/

/*!
    \fn bool QPaintEngine::begin(QPaintDevice *pdev)

    Reimplement this function to initialise your paint engine when
    painting is to start on the paint device \a pdev. Return true if
    the initialization was successful; otherwise return false.

    \sa end() isActive()
*/

/*!
    \fn bool QPaintEngine::end()

    Reimplement this function to finish painting on the current paint
    device. Return true if painting was finished successfully;
    otherwise return false.

    \sa begin() isActive()
*/


/*!
    Draws the first \a pointCount points in the buffer \a points
*/
void QPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    for (int i=0; i<pointCount; ++i) {
        QLineF line(points[i].x(), points[i].y(), points[i].x(), points[i].y());
        drawLines(&line, 1);
    }
}


/*!
    Draws the first \a pointCount points in the buffer \a points

    The default implementation converts the first \a pointCount QPoints in \a points
    to QPointFs and calls the floating point version of drawPoints.

*/
void QPaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    Q_ASSERT(sizeof(QT_PointF) == sizeof(QPointF));
    QT_PointF fp[256];
    while (pointCount) {
        int i = 0;
        while (i < pointCount && i < 256) {
            fp[i].x = points[i].x();
            fp[i].y = points[i].y();
            ++i;
        }
        drawPoints((QPointF *)fp, i);
        points += i;
        pointCount -= i;
    }
}



/*!
    \fn void QPaintEngine::drawEllipse(const QRectF &rect)

    Reimplement this function to draw the largest ellipse that can be
    contained within rectangle \a rect.

    The default implementation calls drawPolygon().
*/
void QPaintEngine::drawEllipse(const QRectF &rect)
{
    QPainterPath path;
    path.moveTo(rect.x() + rect.width(), rect.y() + rect.height()/2);
    path.arcTo(rect, 0, 360);
    if (hasFeature(PainterPaths)) {
        drawPath(path);
    } else {
        QPolygonF polygon = path.toFillPolygon();
        drawPolygon(polygon.data(), polygon.size(), ConvexMode);
    }
}

/*!
    The default implementation of this function calls the floating
    point version of this function
*/
void QPaintEngine::drawEllipse(const QRect &rect)
{
    drawEllipse(QRectF(rect));
}

/*!
    \fn void QPaintEngine::drawPixmap(const QRectF &r, const QPixmap
    &pm, const QRectF &sr)

    Reimplement this function to draw the part of the \a pm
    specified by the \a sr rectangle in the given \a r.
*/


void qt_fill_tile(QPixmap *tile, const QPixmap &pixmap)
{
    QPainter p(tile);
    p.drawPixmap(0, 0, pixmap);
    int x = pixmap.width();
    while (x < tile->width()) {
        p.drawPixmap(x, 0, *tile, 0, 0, x, pixmap.height());
        x *= 2;
    }
    int y = pixmap.height();
    while (y < tile->height()) {
        p.drawPixmap(0, y, *tile, 0, 0, tile->width(), y);
        y *= 2;
    }
}

void qt_draw_tile(QPaintEngine *gc, qreal x, qreal y, qreal w, qreal h,
                  const QPixmap &pixmap, qreal xOffset, qreal yOffset)
{
    qreal yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while(yPos < y + h) {
        drawH = pixmap.height() - yOff;    // Cropping first row
        if (yPos + drawH > y + h)           // Cropping last row
            drawH = y + h - yPos;
        xPos = x;
        xOff = xOffset;
        while(xPos < x + w) {
            drawW = pixmap.width() - xOff; // Cropping first column
            if (xPos + drawW > x + w)           // Cropping last column
                drawW = x + w - xPos;
            gc->drawPixmap(QRectF(xPos, yPos, drawW, drawH), pixmap, QRectF(xOff, yOff, drawW, drawH));
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}


/*!
    Reimplement this function to draw the \a pixmap in the given \a
    rect, starting at the given \a p. The pixmap will be
    drawn repeatedly until the \a rect is filled.
*/
void QPaintEngine::drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &p)
{
    int sw = pixmap.width();
    int sh = pixmap.height();

    if (sw*sh < 8192 && sw*sh < 16*rect.width()*rect.height()) {
        int tw = sw, th = sh;
        while (tw*th < 32678 && tw < rect.width()/2)
            tw *= 2;
        while (tw*th < 32678 && th < rect.height()/2)
            th *= 2;
        QPixmap tile;
        if (pixmap.hasAlphaChannel()) {
            // ####################
            QImage image(tw, th, QImage::Format_ARGB32_Premultiplied);
            image.fill(qRgba(127, 0, 0, 127));
            tile = QPixmap::fromImage(image);
        } else {
            if (pixmap.depth() == 1)
                tile = QBitmap(tw, th);
            else
                tile = QPixmap(tw, th);
        }
        qt_fill_tile(&tile, pixmap);
        qt_draw_tile(this, rect.x(), rect.y(), rect.width(), rect.height(), tile, p.x(), p.y());
    } else {
        qt_draw_tile(this, rect.x(), rect.y(), rect.width(), rect.height(), pixmap, p.x(), p.y());
    }
}

/*!
    \fn void QPaintEngine::drawImage(const QRectF &rectangle, const QImage
    &image, const QRectF &sr, Qt::ImageConversionFlags flags)

    Reimplement this function to draw the part of the \a image
    specified by the \a sr rectangle in the given \a rectangle using
    the given conversion flags \a flags, to convert it to a pixmap.
*/

void QPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                             Qt::ImageConversionFlags flags)
{
    QImage im = image.depth() == 1 ? image.convertToFormat(QImage::Format_RGB32) : image;
    QPixmap pm = QPixmap::fromImage(im, flags);
    drawPixmap(r, pm, sr);
}

/*!
    \fn Type QPaintEngine::type() const

    Reimplement this function to return the paint engine \l{Type}.
*/

/*!
    \fn void QPaintEngine::fix_neg_rect(int *x, int *y, int *w, int *h);

    \internal
*/

/*!
    \fn bool QPaintEngine::testDirty(DirtyFlags df)

    \internal
*/

/*!
    \fn void QPaintEngine::clearDirty(DirtyFlags df)

    \internal
*/

/*!
    \fn void QPaintEngine::setDirty(DirtyFlags df)

    \internal
*/

/*!
    \fn bool QPaintEngine::hasFeature(PaintEngineFeatures feature) const

    Returns true if the paint engine supports the specified \a
    feature; otherwise returns false.
*/

/*!
    \fn void QPaintEngine::updateState(QPainterState *state)

    \internal
*/


/*!
  Creates a paint engine with the featureset specified by \a caps.
*/

QPaintEngine::QPaintEngine(PaintEngineFeatures caps)
    : state(0),
      gccaps(caps),
      active(0),
      selfDestruct(false),
      d_ptr(new QPaintEnginePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
  \internal
*/

QPaintEngine::QPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures caps)
    : state(0),
      gccaps(caps),
      active(0),
      selfDestruct(false),
      d_ptr(&dptr)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the paint engine.
*/
QPaintEngine::~QPaintEngine()
{
    delete d_ptr;
}

/*!
    Returns the paint engine's painter.
*/
QPainter *QPaintEngine::painter() const
{
    return state ? state->painter() : 0;
}

/*!
    The default implementation ignores the \a path and does nothing.
*/

void QPaintEngine::drawPath(const QPainterPath &)
{
    if (hasFeature(PainterPaths)) {
        qWarning("QPaintEngine::drawPath(), must be implemented when feature PainterPaths is set");
    }
}

/*!
    This function draws the text item \a textItem at position \a p. The
    default implementation of this function converts the text to a
    QPainterPath and paints the resulting path.
*/

void QPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
    if (hasFeature(QPaintEngine::UsesFontEngine)) {
	bool useFontEngine = true;
        QMatrix matrix = state->matrix();
        bool simple = matrix.m11() == 1 && matrix.m12() == 0
                        && matrix.m21() == 0 && matrix.m22() == 1;
        if (!simple) {
            useFontEngine = false;
            QFontEngine *fe = ti.fontEngine;
            QFontEngine::FECaps fecaps = fe->capabilites();
            useFontEngine = (fecaps == QFontEngine::FullTransformations);
            if (!useFontEngine
                    && matrix.m11() == matrix.m22()
                    && matrix.m12() == -matrix.m21())
                useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
#if 0
            if (state->txop == QPainterPrivate::TxRotShear) {
                useFontEngine = (fecaps == QFontEngine::FullTransformations);
                if (!useFontEngine
                    && state->matrix.m11() == state->matrix.m22()
                    && state->matrix.m12() == -state->matrix.m21())
                    useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
            } else if (state->txop == QPainterPrivate::TxScale) {
                useFontEngine = (fecaps & QFontEngine::Scale);
            }
#endif
        }
        if (useFontEngine) {
            ti.fontEngine->draw(this, qRound(p.x()), qRound(p.y()), ti);
            return;
        }
    }
#endif

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    ti.fontEngine->addOutlineToPath(p.x(), p.y(), ti.glyphs, ti.num_glyphs, &path, ti.flags);
    if (!path.isEmpty()) {
        painter()->save();
        painter()->setBrush(state->pen().brush());
        painter()->setPen(Qt::NoPen);
        painter()->drawPath(path);
        painter()->restore();
    } else {
        // Fallback: rasterize into a pixmap and draw the pixmap
        QPixmap pm(qRound(ti.width), qRound(ti.ascent + ti.descent));
        pm.fill(Qt::white);

        QPainter painter;
        painter.begin(&pm);
        painter.setPen(Qt::black);
        painter.drawTextItem(QPointF(0., ti.ascent), ti);
        painter.end();

        QImage img = pm.toImage();
        if (img.depth() != 32)
            img = img.convertToFormat(QImage::Format_ARGB32);
        int i = 0;
        QRgb pen_rgb = state->pen().color().rgb() & 0x00ffffff;
        while (i < img.height()) {
            uint *p = (uint *) img.scanLine(i);
            uint *end = p + img.width();

            while (p < end) {
                *p = ((0xff - qGray(*p)) << 24) | pen_rgb;
                ++p;
            }
            ++i;
        }

        pm = QPixmap::fromImage(img);
        this->painter()->drawPixmap(qRound(p.x()), qRound(p.y() - ti.ascent), pm);
    }
}

/*!
    The default implementation splits the list of lines in \a lines
    into \a lineCount separate calls to drawPath() or drawPolygon()
    depending on the feature set of the paint engine.
*/
void QPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    for (int i=0; i<lineCount; ++i) {
        QPointF pts[2] = { lines[i].p1(), lines[i].p2() };
        drawPolygon(pts, 2, PolylineMode);
    }
}

/*!
    \overload

    The default implementation converts the first \a lineCount lines
    in \a lines to a QLineF and calls the floating point version of
    this function.
*/
void QPaintEngine::drawLines(const QLine *lines, int lineCount)
{
    struct PointF {
        qreal x;
        qreal y;
    };
    struct LineF {
        PointF p1;
        PointF p2;
    };
    Q_ASSERT(sizeof(PointF) == sizeof(QPointF));
    Q_ASSERT(sizeof(LineF) == sizeof(QLineF));
    LineF fl[256];
    while (lineCount) {
        int i = 0;
        while (i < lineCount && i < 256) {
            fl[i].p1.x = lines[i].x1();
            fl[i].p1.y = lines[i].y1();
            fl[i].p2.x = lines[i].x2();
            fl[i].p2.y = lines[i].y2();
            ++i;
        }
        drawLines((QLineF *)fl, i);
        lines += i;
        lineCount -= i;
    }
}


/*!
    \overload

    The default implementation converts the first \a rectCount
    rectangles in the buffer \a rects to a QRectF and calls the
    floating point version of this function.
*/
void QPaintEngine::drawRects(const QRect *rects, int rectCount)
{
    struct RectF {
        qreal x;
        qreal y;
        qreal w;
        qreal h;
    };
    Q_ASSERT(sizeof(RectF) == sizeof(QRectF));
    RectF fr[256];
    while (rectCount) {
        int i = 0;
        while (i < rectCount && i < 256) {
            fr[i].x = rects[i].x();
            fr[i].y = rects[i].y();
            fr[i].w = rects[i].width();
            fr[i].h = rects[i].height();
            ++i;
        }
        drawRects((QRectF *)fr, i);
        rects += i;
        rectCount -= i;
    }
}

/*!
    Draws the first \a rectCount rectangles in the buffer \a
    rects. The default implementation of this function calls drawPath()
    or drawPolygon() depending on the feature set of the paint engine.
*/
void QPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    if (hasFeature(PainterPaths)) {
        for (int i=0; i<rectCount; ++i) {
            QPainterPath path;
            path.addRect(rects[i]);
            if (path.isEmpty())
                continue;
            drawPath(path);
        }
    } else {
        for (int i=0; i<rectCount; ++i) {
            QRectF rf = rects[i];
            QPointF pts[4] = { QPointF(rf.x(), rf.y()),
                               QPointF(rf.x() + rf.width(), rf.y()),
                               QPointF(rf.x() + rf.width(), rf.y() + rf.height()),
                               QPointF(rf.y(), rf.y() + rf.height()) };
            drawPolygon(pts, 4, ConvexMode);
        }
    }
}

/*!
    \internal
    Sets the paintdevice that this engine operates on to \a device
*/
void QPaintEngine::setPaintDevice(QPaintDevice *device)
{
    d_func()->pdev = device;
}

/*!
    Returns the engine that this engine is painting on, if painting is
    active; otherwise returns 0.
*/
QPaintDevice *QPaintEngine::paintDevice() const
{
    return d_func()->pdev;
}

#ifdef Q_WS_WIN
/*!
    \internal

    Empty default implementation.
*/

HDC QPaintEngine::getDC() const
{
    return 0;
}


/*!
    \internal

    Empty default implementation.
*/

void QPaintEngine::releaseDC(HDC) const
{
}

#endif

/*!
    \internal

    Returns the offset from the painters origo to the engines
    origo. This value is used by QPainter for engines who have
    internal double buffering.

    This function only makes sense when the engine is active.
*/
QPoint QPaintEngine::coordinateOffset() const
{
    return QPoint();
}

/*!
    \internal

    Sets the system clip for this engine. The system clip defines the
    basis area that the engine has to draw in. All clips that are
    set must be an intersection with the system clip.

    Reset the systemclip to no clip by setting an empty region.
*/
void QPaintEngine::setSystemClip(const QRegion &region)
{
    if (isActive()) {
        qWarning("QPaintEngine::setSystemClip(), should not be changed while engine is active");
        return;
    }
    d_func()->systemClip = region;
}


/*!
    \internal

    Returns the system clip. The system clip is read only while the
    painter is active. An empty region indicates that system clip
    is not in use.
*/

QRegion QPaintEngine::systemClip() const
{
    return d_func()->systemClip;
}
