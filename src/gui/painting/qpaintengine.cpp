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
  CoordTransform, \c PixmapTransform, \c LinearGradientFill, \c
  PixmapScale, \c AlphaFill and \c ClipTransform.

  \value CoordTransform The engine can transform the points in a
  drawing operation.

  \value PenWidthTransform The engine has support for transforming pen
  widths.

  \value PatternTransform The engine has support for transforming brush
  patterns.

  \value PixmapTransform The engine can transform pixmaps, including
  rotation and shearing.

  \value LinearGradientFill The engine can fill with linear gradients

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

    The default implementation of this function will try to use drawPath
    if the engine supports the feature QPaintEngine::PainterPaths or try
    the int based drawPolygon() implementation if not.
*/
void QPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    if (hasFeature(PainterPaths)) {
        Q_ASSERT(pointCount > 0); // these should have been cleared out by QPainter.
        QPainterPath path(points[0]);
        for (int i=1; i<pointCount; ++i)
            path.lineTo(points[i]);
        // ### don't like this... should perhaps introduce strokePath in api
        if (mode == PolylineMode) {
            updateBrush(QBrush(), QPoint(0, 0));
            setDirty(DirtyBrush);
        } else {
            path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
            path.closeSubpath();
        }
        drawPath(path);
    } else {
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
}

/*!
    \overload

    Reimplement this virtual function to draw the polygon defined by the
    \a pointCount first points in \a points, using mode \a mode.

    The default implementation of this function will try to use drawPath()
    if the engine supports the feature QPaintEngine::PainterPaths or try
    the float based drawPolygon() implementation if not.
*/
void QPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    if (hasFeature(PainterPaths)) {
        QPainterPath path(points[0]);
        for (int i=1; i<pointCount; ++i)
            path.lineTo(points[i]);
        if (mode == PolylineMode) {
            updateBrush(QBrush(), QPoint(0, 0));
            setDirty(DirtyBrush);
        } else {
            path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
            path.closeSubpath();
        }
        drawPath(path);
    } else {
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
    Draws the first \a pointCount points in the buffer \a points
*/
void QPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    updateBrush(QBrush(state->pen.color()), QPointF());
    updatePen(QPen(Qt::NoPen));
    for (int i=0; i<pointCount; ++i) {
        QRectF r(points[i].x(), points[i].y(), 1, 1);
        drawRects(&r, 1);
    }
    setDirty(DirtyPen|DirtyBrush);
}


/*!
    Draws the first \a pointCount points in the buffer \a points
*/
void QPaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    QVarLengthArray<QPointF> floatPoints(pointCount);
    for (int i=0; i<pointCount; ++i)
        floatPoints[i] = QPointF(points[i]);
    drawPoints(floatPoints.constData(), pointCount);
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

void qt_draw_tile(QPaintEngine *gc, qreal x, qreal y, qreal w, qreal h,
                  const QPixmap &pixmap, qreal xOffset, qreal yOffset,
		  Qt::PixmapDrawingMode mode)
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
            QImage image(tw, th, 32);
            image.fill(QColor(255, 0, 0, 127).rgb());
            image.setAlphaBuffer(true);
            tile = QPixmap::fromImage(image);
        } else {
            tile = QPixmap(tw, th, pixmap.depth());
        }
        qt_fill_tile(&tile, pixmap);
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
    QPixmap pm = QPixmap::fromImage(image, flags);
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
            && s->pen.widthF() > .0f && !hasFeature(PenWidthTransform))
            emulationSpecifier |= PenWidthTransform;
        else
            emulationSpecifier &= ~PenWidthTransform;

        if ((s->brush.style() > Qt::SolidPattern && s->brush.style() < Qt::LinearGradientPattern
             || s->brush.style() == Qt::TexturePattern)
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
            && s->pen.widthF() > .0f && !hasFeature(PenWidthTransform))
            emulationSpecifier |= PenWidthTransform;
        else
            emulationSpecifier &= ~PenWidthTransform;

        if (d->renderhints & QPainter::Antialiasing && s->pen.style() != Qt::NoPen
            && !hasFeature(LineAntialiasing))
            emulationSpecifier |= LineAntialiasing;
        else
            emulationSpecifier &= ~LineAntialiasing;

        if (d->renderhints & QPainter::Antialiasing && s->pen.style() != Qt::NoPen
            && !hasFeature(FillAntialiasing))
            emulationSpecifier |= FillAntialiasing;
        else
            emulationSpecifier &= ~FillAntialiasing;

        // Emulate brushstrokes
        if (!s->pen.isSolid() && !hasFeature(BrushStroke))
            emulationSpecifier |= BrushStroke;
        else
            emulationSpecifier &= ~BrushStroke;
    }


    if (testDirty(DirtyBackground)) {
        updateBackground(s->bgMode, s->bgBrush);
        clearDirty(DirtyBackground);
    }


    if (testDirty(DirtyBrush)) {
        updateBrush(s->brush, s->bgOrigin);
        clearDirty(DirtyBrush);

        if (s->brush.style() == Qt::LinearGradientPattern && !hasFeature(LinearGradientFill))
            emulationSpecifier |= LinearGradientFill;
        else
            emulationSpecifier &= ~LinearGradientFill;

        if (s->brush.style() == Qt::RadialGradientPattern && !hasFeature(RadialGradientFill))
            emulationSpecifier |= RadialGradientFill;
        else
            emulationSpecifier &= ~RadialGradientFill;

        if (s->brush.style() == Qt::ConicalGradientPattern && !hasFeature(ConicalGradientFill))
            emulationSpecifier |= ConicalGradientFill;
        else
            emulationSpecifier &= ~ConicalGradientFill;

        if (!s->brush.isOpaque() && !hasFeature(AlphaFill))
            emulationSpecifier |= AlphaFill;
        else
            emulationSpecifier &= ~AlphaFill;

        if (((s->brush.style() > Qt::SolidPattern && s->brush.style() < Qt::LinearGradientPattern)
             || s->brush.style() == Qt::TexturePattern)
            && s->txop > QPainterPrivate::TxTranslate && !hasFeature(PatternTransform))
            emulationSpecifier |= PatternTransform;
        else
            emulationSpecifier &= ~PatternTransform;

        if (((s->brush.style() > Qt::SolidPattern && s->brush.style() < Qt::LinearGradientPattern)
             || s->brush.style() == Qt::TexturePattern)
            && !hasFeature(PatternBrush))
            emulationSpecifier |= PatternBrush;
        else
            emulationSpecifier &= ~PatternBrush;
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

            pm = QPixmap::fromImage(img);
            state->painter->drawPixmap(qRound(p.x()), qRound(p.y() - ti.ascent), pm);
        }
    }
}

/*!
    The default implementation splits the list of lines in \a lines
    into \a lineCount separate calls to drawPath() or drawPolygon()
    depending on the feature set of the paint engine.
*/
void QPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    if (hasFeature(PainterPaths)) {
        updateBrush(QBrush(), QPointF(0, 0));
        for (int i=0; i<lineCount; ++i) {
            QPainterPath path(lines[i].p1());
            path.lineTo(lines[i].p2());
            drawPath(path);
        }
        setDirty(QPaintEngine::DirtyBrush);
    } else {
        for (int i=0; i<lineCount; ++i) {
            QPointF pts[2] = { lines[i].p1(), lines[i].p2() };
            drawPolygon(pts, 2, PolylineMode);
        }
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
    QVarLengthArray<QLineF, 32> floatLines(lineCount);
    for (int i=0; i<lineCount; ++i)
        floatLines[i] = QLineF(lines[i]);
    drawLines(floatLines.constData(), lineCount);
}


/*!
    \overload

    The default implementation converts the first \a rectCount
    rectangles in the buffer \a rects to a QRectF and calls the
    floating point version of this function.
*/
void QPaintEngine::drawRects(const QRect *rects, int rectCount)
{
    QVarLengthArray<QRectF, 32> floatRects(rectCount);
    for (int i=0; i<rectCount; ++i)
        floatRects[i] = QRectF(rects[i]);
    drawRects(floatRects.constData(), rectCount);
}

/*!
    Draws the first \a rectCount rectangles in the buffer \a
    rects. The default implementation of this function calls drawPath
    or drawPolygon depending on the feature set of the paint engine.
*/
void QPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    if (hasFeature(PainterPaths)) {
        for (int i=0; i<rectCount; ++i) {
            QPainterPath path;
            path.addRect(rects[i]);
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
        d->renderhints |= hint;
    } else {
        d->renderhints &= ~hint;
    }
    setDirty(DirtyHints);
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
    Returns the engine that this engine is painting on, if painting is
    active; otherwise returns 0.
*/
QPaintDevice *QPaintEngine::paintDevice() const
{
    return d->pdev;
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
    d->systemClip = region;
}


/*!
    \internal

    Returns the system clip. The system clip is read only while the
    painter is active. An empty region indicates that system clip
    is not in use.
*/

QRegion QPaintEngine::systemClip() const
{
    return d->systemClip;
}
