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
#include <qdebug.h>

#include <private/qfontengine_p.h>

/*! \class QPaintEngine qpaintengine.h
  \brief The QPaintEngine class provides an abstract definition of how
  QPainter draws to a given device on a given platform.

  Qt 4.0 provides several premade implementations of QPaintEngine for
  the different painter backends we support. We provide one paint
  engine for each window system and painting framework we
  support. This includes X11 on Unix/Linux, GDI/GDI+ on Windows and
  QuickDraw/CoreGraphics on Mac OS X. In addition we provide
  QPaintEngine implementations for OpenGL (accessible through
  QGLWidget) and PostScript (accessible through QPSPrinter on X11).

  If one wants to use QPainter to draw to a different backend, such as
  PDF, one must subclass QPaintEngine and reimplement all its virtual
  functions. The QPaintEngine implementation is then made available by
  subclassing QPaintDevice and reimplementing the virtual function \c
  QPaintDevice::paintEngine().

  QPaintEngine is created and owned by the QPaintDevice that created it.

  The big advantage of the QPaintEngine approach opposed to the
  previous QPainter/QPaintDevice::cmd() approach is that it is now
  possible to adapt to multiple technologies on each platform and take
  advantage of the to the fullest.

  \sa QPainter, QPaintDevice::paintEngine()
*/

/*!
  \enum QPaintEngine::PaintEngineFeature

  This enum is used to describe the features or capabilities that the
  paint engine has. If a feature is not supported by the engine,
  QPainter will do a best effort to emulate that feature through other
  means. The features that are currently emulated are: \c
  CoordTransform, \c PixmapTransform, \c LinearGradients, \c
  PixmapScale, \c AlphaFill and \c ClipTransform.

  \value CoordTransform The engine can transform the points in a
  drawing operation.

  \value PenWidthTransform The engine has support for transforming pen
  widths.

  \value PatternTransform The engine has support for transforming brush
  patterns.

  \value PixmapTransform The engine can transform pixmaps, including
  rotation and shearing.

  \value LinearGradients The engine can fill with linear gradients

  \value LinearGradientFillPolygon The engine is capable of gradient
  fills in polygons only. Engines that support this feature and not
  regular gradient filling will have all primitives converted to
  polygons first so that this feature can be used instead.

  \value PixmapScale The engine can scale pixmaps.

  \value AlphaPixmap The engine can draw alpha pixmaps.

  \value AlphaFill The engine can fill with alpha colors. If this
  feature is not specified, alpha filling can be emulated using alpha
  pixmaps.

  \value AlphaFillPolygon The engine can fill polygons with alpha
  colors. Engines that specify this feature and not AlphaFill will
  have all primitives converted to polygons first, so that this
  feature can be used instead.

  \value AlphaStroke The engine can draw outlines with alpha
  colors. Engines that do not support alphastroke can emulate it using
  AlphaFill, AlphaFillPolygon or AlphaPixmap.

  \value PainterPaths The engine has path support.

  \value ClipTransform The engine is capable of transforming clip regions.

  \value PaintOutsidePaintEvent The engine is capable of painting
  outside of paint events.

  \value PatternBrush The engine is capable of rendering brushes with
  the brush patterns specified in Qt::BrushStyle.

  \value LineAntialiasing The engine is capable of line antialiasing.

  \value FillAntialiasing The engine is capable of fill
  antialiasing. If an engine is capable of fill, but not line
  antialiasing, line antialiasing will be emulated using a filled
  polygon that has the shape of the line.

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

/*!
    \fn void QPaintEngine::drawPolygon(const QPointF *points, int pointCount,
    PolygonDrawMode mode)

    Reimplement this virtual function to draw the polygon defined
    by the \a pointCount first points in \a points, using mode \a
    mode.
*/
void QPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT_X(!qt_polygon_recursion, "QPaintEngine::drawPolygon",
               "At least one drawPolygon function must be implemented");
    qt_polygon_recursion = 1;
    QPolygon p;
    p.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        p << points[i].toPoint();
    drawPolygon(p.data(), pointCount, mode);
    qt_polygon_recursion = 0;
}

/*!
    \reimp

    Reimplement this virtual function to draw the polygon defined by the
    \a pointCount first points in \a points, using mode \a mode.
*/
void QPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT_X(!qt_polygon_recursion, "QPaintEngine::drawPolygon",
               "At least one drawPolygon function must be implemented");
    qt_polygon_recursion = 1;
    QPolygonF p;
    p.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        p << points[i];
    drawPolygon(p.data(), pointCount, mode);
    qt_polygon_recursion = 0;
}



#define d d_func()
#define q q_func()

/*!
    \enum QPaintEngine::Type

    \value X11
    \value Windows
    \value Gdiplus (same as Windows)
    \value QuickDraw (same as CoreGraphics)
    \value MacPrinter
    \value CoreGraphics Mac OS X
    \value QWindowSystem Qt/Embedded
    \value PostScript
    \value OpenGL
    \value Picture QPicture format
    \value SVG Scalable Vector Graphics XML format
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
    Calls drawPoint() to draw every point in the polygon \a p.
*/

void QPaintEngine::drawPoints(const QPolygonF &p)
{
    for (int i=0; i<p.size(); ++i)
        drawPoint(p.at(i));
}

/*!
    \fn void QPaintEngine::drawEllipse(const QRectF &rect)

    Reimplement this function to draw the largest ellipse that can be
    contained within rectangle \a rect.

    The default implementation calls drawPolygon().

    \sa drawPolygon
*/

void QPaintEngine::drawEllipse(const QRectF &rect)
{
    QPainterPath path;
    path.moveTo(rect.x() + rect.width(), rect.y() + rect.height()/2);
    path.arcTo(rect, 0, 360);
    QPolygonF polygon = path.toFillPolygon();
    drawPolygon(polygon.data(), polygon.size(), ConvexMode);
}

/*!
    Calls drawLine() for every pair of points in the point array \a
    lines.
*/

void QPaintEngine::drawLines(const QList<QLineF> &lines)
{
    for (int i=0; i<lines.size(); ++i)
        drawLine(lines.at(i));
}

/*!
    \fn void QPaintEngine::drawPixmap(const QRectF &r, const QPixmap
    &pm, const QRectF &sr, Qt::PixmapDrawingMode mode)

    Reimplement this function to draw the part of the \a pm
    specified by the \a sr rectangle in the given \a r using the given
    drawing \a mode.
*/


void qt_fill_tile(QPixmap *tile, const QPixmap &pixmap)
{
    QPainter p(tile);
    p.drawPixmap(0, 0, pixmap, Qt::CopyPixmap);
    int x = pixmap.width();
    while (x < tile->width()) {
        p.drawPixmap(x, 0, *tile, 0, 0, x, pixmap.height(), Qt::CopyPixmap);
        x *= 2;
    }
    int y = pixmap.height();
    while (y < tile->height()) {
        p.drawPixmap(0, y, *tile, 0, 0, tile->width(), y, Qt::CopyPixmap);
        y *= 2;
    }
}

void qt_draw_tile(QPaintEngine *gc, float x, float y, float w, float h,
                  const QPixmap &pixmap, float xOffset, float yOffset,
		  Qt::PixmapDrawingMode mode)
{
    float yPos, xPos, drawH, drawW, yOff, xOff;
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
            gc->drawPixmap(QRectF(xPos, yPos, drawW, drawH), pixmap, QRectF(xOff, yOff, drawW, drawH), mode);
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
    drawn repeatedly until the \a rect is filled using the given
    \a mode.
*/
void QPaintEngine::drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &p,
                                   Qt::PixmapDrawingMode mode)
{
    QBitmap *mask = (QBitmap *)pixmap.mask();

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
            QImage image(tw, th, 32);
            image.fill(QColor(255, 0, 0, 127).rgb());
            image.setAlphaBuffer(true);
            tile = image;
        } else {
            tile = QPixmap(tw, th, pixmap.depth(), QPixmap::BestOptim);
        }
        qt_fill_tile(&tile, pixmap);
        if (mask) {
            QBitmap tilemask(tw, th, false, QPixmap::NormalOptim);
            qt_fill_tile(&tilemask, *mask);
            tile.setMask(tilemask);
        }
        qt_draw_tile(this, rect.x(), rect.y(), rect.width(), rect.height(), tile, p.x(), p.y(), mode);
    } else {
        qt_draw_tile(this, rect.x(), rect.y(), rect.width(), rect.height(), pixmap, p.x(), p.y(), mode);
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
    QPixmap pm;
    pm.fromImage(image, flags);
    drawPixmap(r, pm, sr);
}

/*!
    \fn Type QPaintEngine::type() const

    Reimplement this function to return the paint engine \l{Type}.
*/

/*!
    \fn bool QPaintEngine::testf(uint b) const

    \internal
*/

/*!
    \fn void QPaintEngine::setf(uint b)

    \internal
*/

/*!
    \fn void QPaintEngine::clearf(uint b)

    \internal
*/

/*!
    \fn void QPaintEngine::assignf(uint b)

    \internal
*/

/*!
    \fn void QPaintEngine::fix_neg_rect(int *x, int *y, int *w, int *h);

    \internal
*/

/*!
    \fn bool QPaintEngine::hasClipping() const

    \internal
*/

/*!
    \fn bool QPaintEngine::testDirty(DirtyFlags df)

    \internal
*/

/*!
    \fn void QPaintEngine::setDirty(DirtyFlags df)

    \internal
*/

/*!
    \fn void QPaintEngine::clearDirty(DirtyFlags df)

    \internal
*/

/*!
    \fn bool QPaintEngine::hasFeature(PaintEngineFeatures feature) const

    Returns true if the paint engine supports the specified \a
    feature; otherwise returns false.
*/

/*!
    \fn void QPaintEngine::updateState(QPainterState *state, bool updateGC)

    \internal
*/


/*!
  Creates a paint engine with the featureset specified by \a caps.
*/

QPaintEngine::QPaintEngine(PaintEngineFeatures caps)
    : dirtyFlag(0),
      active(0),
      selfDestruct(false),
      state(0),
      gccaps(caps),
      d_ptr(new QPaintEnginePrivate),
      emulationSpecifier(0)
{
    d_ptr->q_ptr = this;
}

/*!
  \internal
*/

QPaintEngine::QPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures caps)
    : dirtyFlag(0),
      active(0),
      selfDestruct(false),
      state(0),
      gccaps(caps),
      d_ptr(&dptr),
      emulationSpecifier(0)
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
    return state->painter;
}

/*!
  \internal

  This function is responsible for calling the reimplemented updateXXX functions in
  the QPaintEngine based on what is currently marked as dirty. If \a updateGC is
  false we don't call the update functions.
*/

void QPaintEngine::updateInternal(QPainterState *s, bool updateGC)
{
    Q_ASSERT(state);
    if (!s || !updateGC) {
        state = s;
        return;
    }

    // assign state after we start checking so the if works, but before update
    // calls since we need it in some cases..
    if (s->painter != state->painter) {
        setDirty(AllDirty);
    } else if (s != state) {
        // Update all changes since last save, even if not propagated to the painter yet.
        // This will ensure that the painter state is always correct. ('=' is not enough!)
        dirtyFlag |= state->changeFlags;
    } else {
        state->changeFlags |= dirtyFlag;
    }
    state = s;

    if (testDirty(DirtyHints)) {
        updateRenderHints(d->renderhints);
        clearDirty(DirtyHints);

        // Emulate line antialiasing only if we can antialias through polygon filling.
        if (d->renderhints & QPainter::Antialiasing && s->pen.style() != Qt::NoPen
            && !hasFeature(LineAntialiasing) && hasFeature(FillAntialiasing))
            emulationSpecifier |= LineAntialiasing;
        else
            emulationSpecifier &= ~LineAntialiasing;
    }


    if (testDirty(DirtyTransform)) {
        updateMatrix(s->matrix);
        clearDirty(DirtyTransform);

        if (state->txop >= QPainterPrivate::TxTranslate && !hasFeature(CoordTransform))
            emulationSpecifier |= CoordTransform;
        else
            emulationSpecifier &= ~CoordTransform;

        if (state->txop >= QPainterPrivate::TxTranslate
            && s->pen.width() != 0 && !hasFeature(PenWidthTransform))
            emulationSpecifier |= PenWidthTransform;
        else
            emulationSpecifier &= ~PenWidthTransform;

        if ((s->brush.style() > Qt::SolidPattern && s->brush.style() < Qt::LinearGradientPattern
             || s->brush.style() == Qt::CustomPattern)
            && s->txop > QPainterPrivate::TxTranslate && !hasFeature(PatternTransform))
            emulationSpecifier |= PatternTransform;
        else
            emulationSpecifier &= ~PatternTransform;
    }


    if (testDirty(DirtyPen)) {
        updatePen(s->pen);
        clearDirty(DirtyPen);

        // Check for emulation of alpha stroking
        if (s->pen.style() != Qt::NoPen && s->pen.color().alpha() != 255 && !hasFeature(AlphaStroke))
            emulationSpecifier |= AlphaStroke;
        else
            emulationSpecifier &= ~AlphaStroke;

        // Check for emulation of pen xform
        if (s->txop > QPainterPrivate::TxTranslate
            && s->pen.width() != 0 && !hasFeature(PenWidthTransform))
            emulationSpecifier |= PenWidthTransform;
        else
            emulationSpecifier &= ~PenWidthTransform;

        // Emulate line antialiasing only if we can antialias through polygon filling.
        if (d->renderhints & QPainter::Antialiasing && s->pen.style() != Qt::NoPen
            && !hasFeature(LineAntialiasing) && hasFeature(FillAntialiasing))
            emulationSpecifier |= LineAntialiasing;
        else
            emulationSpecifier &= ~LineAntialiasing;
    }


    if (testDirty(DirtyBackground)) {
        updateBackground(s->bgMode, s->bgBrush);
        clearDirty(DirtyBackground);
    }


    if (testDirty(DirtyBrush)) {
        updateBrush(s->brush, s->bgOrigin);
        clearDirty(DirtyBrush);

        if (s->brush.style() == Qt::LinearGradientPattern && !hasFeature(LinearGradients))
            emulationSpecifier |= LinearGradients;
        else
            emulationSpecifier &= ~LinearGradients;

        if (s->brush.style() != Qt::LinearGradientPattern
            && s->brush.color().alpha() != 255 && !hasFeature(AlphaFill))
            emulationSpecifier |= AlphaFill;
        else
            emulationSpecifier &= ~AlphaFill;

        if ((s->brush.style() > Qt::SolidPattern && s->brush.style() < Qt::LinearGradientPattern
             || s->brush.style() == Qt::CustomPattern)
            && s->txop > QPainterPrivate::TxTranslate && !hasFeature(PatternTransform))
            emulationSpecifier |= PatternTransform;
        else
            emulationSpecifier &= ~PatternTransform;
    }


    if (testDirty(DirtyFont)) {
        updateFont(s->font);
        clearDirty(DirtyFont);
    }


    if (testDirty(DirtyClipPath)) {
        if (hasFeature(ClipTransform)) {
            updateClipPath(s->tmpClipPath, s->tmpClipOp);
        } else {
            QPainterPath path = s->txop > QPainterPrivate::TxNone
                                ? (s->tmpClipPath * s->matrix)
                                : s->tmpClipPath;
            updateClipPath(path, s->tmpClipOp);
        }
        clearDirty(DirtyClipPath);
    }


    if (testDirty(DirtyClip)) {
        if (hasFeature(ClipTransform)) {
            updateClipRegion(s->tmpClipRegion, s->tmpClipOp);
        } else {
            QRegion region = s->txop > QPainterPrivate::TxNone
                             ? (s->tmpClipRegion * s->matrix)
                             : s->tmpClipRegion;
            updateClipRegion(region, s->tmpClipOp);
        }
        clearDirty(DirtyClip);
    }


    // It might be the case that a update call flags a previously
    // updated state to dirty. For this case we need to call
    // updateInternal() again to update these states. This is to be
    // sure that all states are in sync when the function returns.
    if (dirtyFlag)
        updateInternal(state);

    if (d->pdev->depth() < 16 || !hasFeature(AlphaPixmap))
        emulationSpecifier &= ~(AlphaFill | AlphaStroke);
}

/*!
    The default implementation ignores the \a path and does nothing.
*/

void QPaintEngine::drawPath(const QPainterPath &)
{

}

/*!
    The default implementation splits \a line into two points and
    calls the integer version of drawLine().
*/
void QPaintEngine::drawLine(const QLineF &line)
{
    QPointF pts[2] = { line.start(), line.end() };
    drawPolygon(pts, 2, PolylineMode);
}

/*!
    \overload

    The default implementation converts \a rf to an integer rectangle
    and calls the integer version of drawRect().
*/
void QPaintEngine::drawRect(const QRectF &rf)
{
    QPointF pts[4] = { QPointF(rf.x(), rf.y()),
                       QPointF(rf.x() + rf.width(), rf.y()),
                       QPointF(rf.x() + rf.width(), rf.y() + rf.height()),
                       QPointF(rf.y(), rf.y() + rf.height()) };
    drawPolygon(pts, 4, ConvexMode);
}

/*!
    \overload

    The default implementation converts \a pf to an integer point and
    calls the integer version of drawPoint().
*/
void QPaintEngine::drawPoint(const QPointF &pf)
{
    drawRect(QRectF(pf.x(), pf.y(), 1, 1));
}


/*!
    This function draws the text item \a ti at position \a p. The
    default implementation of this function converts the text to a
    QPainterPath and paints the resulting path.
*/

void QPaintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
    bool useFontEngine = false;
    if (hasFeature(QPaintEngine::UsesFontEngine)) {
	useFontEngine = true;
        if (state->txop > QPainterPrivate::TxTranslate) {
            useFontEngine = false;
            QFontEngine *fe = ti.fontEngine;
            QFontEngine::FECaps fecaps = fe->capabilites();
            if (state->txop == QPainterPrivate::TxRotShear) {
                useFontEngine = (fecaps == QFontEngine::FullTransformations);
                if (!useFontEngine
                    && state->matrix.m11() == state->matrix.m22()
                    && state->matrix.m12() == -state->matrix.m21())
                    useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
            } else if (state->txop == QPainterPrivate::TxScale) {
                useFontEngine = (fecaps & QFontEngine::Scale);
            }
        }
        if (useFontEngine) {
            ti.fontEngine->draw(this, qRound(p.x()),  qRound(p.y()), ti);
        }
    }
#else
    const bool useFontEngine = false;
#endif

    if (!useFontEngine) {
        QPainterPath path;
        ti.fontEngine->addOutlineToPath(p.x(), p.y(), ti.glyphs, ti.num_glyphs, &path);
        if (!path.isEmpty()) {
            painter()->save();
            painter()->setBrush(state->pen.color());
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
                img = img.convertDepth(32);
            img.setAlphaBuffer(true);
            int i = 0;
            while (i < img.height()) {
                uint *p = (uint *) img.scanLine(i);
                uint *end = p + img.width();
                while (p < end) {
                    *p = ((0xff - qGray(*p)) << 24) | (state->pen.color().rgb() & 0x00ffffff);
                    ++p;
                }
                ++i;
            }

            pm = img;
            state->painter->drawPixmap(qRound(p.x()), qRound(p.y() - ti.ascent), pm);
        }
    }
}

/*!
  Draws the rectangles in the list \a rects.
*/
void QPaintEngine::drawRects(const QList<QRectF> &rects)
{
    for (int i=0; i<rects.size(); ++i)
        drawRect(rects.at(i));
}

/*!
  Returns the set of supported renderhints.
 */
QPainter::RenderHints QPaintEngine::supportedRenderHints() const
{
    return 0;
}

/*!
  Returns the currently set renderhints.
*/
QPainter::RenderHints QPaintEngine::renderHints() const
{
    return d->renderhints;
}

/*!
  Sets the render hint \a hint on this engine if \a on is true;
  otherwise clears the render hint.
*/
void QPaintEngine::setRenderHint(QPainter::RenderHint hint, bool on)
{
    if (on) {
        if (QPainter::RenderHints(d->renderhints & hint) != hint) {
            d->renderhints |= hint;
            setDirty(DirtyHints);
        }
    } else {
        if (QPainter::RenderHints(d->renderhints & hint) != 0) {
            d->renderhints &= ~hint;
            setDirty(DirtyHints);
        }
    }
}

/*!
  This function is caleed when the engine needs to be updated with
  the new set of renderhints specified by \a hints.
*/

void QPaintEngine::updateRenderHints(QPainter::RenderHints /*hints*/)
{
}


/*!
    This function is called when the engine needs to be updated with
    the new clip \a path. The value of \a op specifies how the clip path
    should be combined with the current clip.
*/
void QPaintEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
    updateClipRegion(QRegion(path.toFillPolygon().toPolygon(), path.fillRule()), op);
}

/*!
  \fn QPaintEngine::updatePen(const QPen &pen)

  This function is called when the engine needs to be updated with the
  a new pen, specified by \a pen.
*/


/*!
  \fn QPaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)

  This function is called when the engine needs to be updated with
  a new brush, specified with \a brush. \a origin describes the brush origin.

  If the brush is a gradient brush, the gradient points are specified
  in device coordinates.
*/

/*!
  \fn QPaintEngine::updateFont(const QFont &f)

  This function is called when the engine needs to be updated with
  a new font, specified by \a f
*/

/*!
  \fn QPaintEngine::updateBackground(Qt::BGMode bgmode, const QBrush &brush)

  This function is called when the engine needs to be updated with
  new background settings. \a bgmode describes the background mode and
  \a brush describes the background brush.
*/

/*!
  \fn QPaintEngine::updateMatrix(const QMatrix &matrix)

  This function is called when the engine needs to be updated with
  new transformation settings, specified with \a matrix.
*/

/*!
  \fn QPaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)

  This function is called when the clip region changes. The clip operation \a op
  specifies how the new region \a region should be combined (if at all) with
  the new region.
*/


/*!
    \internal
    Sets the paintdevice that this engine operates on to \a device
*/
void QPaintEngine::setPaintDevice(QPaintDevice *device)
{
    d->pdev = device;
}

/*!
    Returns the engine that this engine is painting on if painting is
    active; otherwise 0;
*/
QPaintDevice *QPaintEngine::paintDevice() const
{
    return d->pdev;
}
