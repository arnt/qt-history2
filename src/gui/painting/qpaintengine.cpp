/****************************************************************************
**
** Definition of QPaintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpaintengine.h"
#include "qpaintengine_p.h"
#include "qpainter_p.h"

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
  PixmapScale, \c SolidAlphaFill and \c ClipTransform.

  \value CoordTransform The engine can transform the points in a
  drawing operation.

  \value PenWidthTransform The engine has support for transforming pen
  widths.

  \value PatterTransform The engine has support for transforming brush
  patterns.

  \value PixmapTransfor The engine can transform pixmaps, including
  rotation and shearing.

  \value LinearGradients The engine can fill with linear gradients

  \value PixmapScale The engine can scale pixmaps.

  \value SolidAlphFill The engine can fill and outline with alpha colors

  \value PainterPaths The engine has path support.

  \value ClipTransform The engine is capable of transforming clip regions.

  \omitvalue UsesFontEngine
*/

/*!
  \enum QPaintEngine::DirtyFlags

  \internal

  This enum is used by QPainter to trigger lazy updates of the various states
  in the QPaintEngine
*/


#define d d_func()
#define q q_func()

/*!
  Creates a paint engine with the featureset specified by \a caps.
*/

QPaintEngine::QPaintEngine(PaintEngineFeatures caps)
    : dirtyFlag(0),
      active(0),
      selfDestruct(false),
      state(0),
      gccaps(caps),
      d_ptr(new QPaintEnginePrivate)
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
      d_ptr(&dptr)
{
    d_ptr->q_ptr = this;
}

QPaintEngine::~QPaintEngine()
{
    delete d_ptr;
}

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
    if (!s || !updateGC) {
        state = s;
        return;
    }

    // assign state after we start checking so the if works, but before update
    // calls since we need it in some cases..
    if (!state || s->painter != state->painter) {
        setDirty(AllDirty);
    } else if (s != state) {
        dirtyFlag = state->changeFlags;
    } else {
        state->changeFlags |= dirtyFlag;
    }
    state = s;

    if (testDirty(DirtyPen)) {
        updatePen(s->pen);
        clearDirty(DirtyPen);
    }
    if (testDirty(DirtyBrush)) {
        updateBrush(s->brush, s->bgOrigin);
        clearDirty(DirtyBrush);
    }
    if (testDirty(DirtyFont)) {
        updateFont(s->font);
        clearDirty(DirtyFont);
    }
    if (testDirty(DirtyBackground)) {
        updateBackground(s->bgMode, s->bgBrush);
        clearDirty(DirtyBackground);
    }
    if (testDirty(DirtyTransform)) {
        updateXForm(s->matrix);
        clearDirty(DirtyTransform);
    }
    if (testDirty(DirtyClip)) {
        updateClipRegion(s->txop > QPainter::TxNone && !hasFeature(ClipTransform)
                         ? s->clipRegionMatrix * s->clipRegion
                         : s->clipRegion,
                         s->clipEnabled);
        clearDirty(DirtyClip);
    }
    if (testDirty(DirtyHints)) {
        updateRenderHints(d->renderhints);
        clearDirty(DirtyHints);
    }
}

/*!
    The default implementation ignores the \a path and does nothing.
*/

void QPaintEngine::drawPath(const QPainterPath &)
{

}

/*!
  This function draws a the text item at the position ti. The default implementation
  of this function will render the text to a pixmap and draw the pixmap instead
 */

void QPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textFlags)
{
    bool useFontEngine = false;
    if (hasFeature(QPaintEngine::UsesFontEngine)) {
	useFontEngine = true;
        if (state->txop > QPainter::TxTranslate) {
            useFontEngine = false;
            QFontEngine *fe = ti.fontEngine;
            QFontEngine::FECaps fecaps = fe->capabilites();
            if (state->txop == QPainter::TxRotShear) {
                useFontEngine = (fecaps == QFontEngine::FullTransformations);
                if (!useFontEngine
                    && state->matrix.m11() == state->matrix.m22()
                    && state->matrix.m12() == -state->matrix.m21())
                    useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
            } else if (state->txop == QPainter::TxScale) {
                useFontEngine = (fecaps & QFontEngine::Scale);
            }
        }
        if (useFontEngine) {
            ti.fontEngine->draw(this, p.x(),  p.y(), ti, textFlags);
        }
    }

    if (!useFontEngine) {
        // Fallback: rasterize into a pixmap and draw the pixmap
        QPixmap pm(ti.width, ti.ascent + ti.descent);
        pm.fill(Qt::white);

        QPainter painter;
        painter.begin(&pm);
        painter.setPen(Qt::black);
        painter.drawTextItem(0, ti.ascent, ti, textFlags);
        painter.end();

        QImage img = pm;
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
        state->painter->drawPixmap(p.x(), p.y() - ti.ascent, pm);
    }
}

/*!
  Draws the rectangles in the list \a rects.
*/
void QPaintEngine::drawRects(const QList<QRect> &/*rects*/)
{
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
  Sets the the render hints specified by \a hints in addition to the
  currently set render hints.

  \sa clearRenderHints()
*/
void QPaintEngine::setRenderHints(QPainter::RenderHints hints)
{
    d->renderhints |= hints;
    setDirty(DirtyHints);
}

/*!
  Clears the render hints specified by \a hints.
*/
void QPaintEngine::clearRenderHints(QPainter::RenderHints hints)
{
    d->renderhints &= ~hints;
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
  \fn QPaintEngine::updatePen(const QPen &pen)

  This function is called when the engine needs to be updated with the
  a new pen, specified by \a pen.
*/


/*!
  \fn QPaintEngine::updateBrush(const QBrush &brush, const QPoint &origin)

  This function is called when the engine needs to be updated with
  a new brush, specified with \a brush. \a origin describes the brush origin.
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
  \fn QPaintEngine::updateXForm(const QWMatrix &matrix)

  This function is called when the engine needs to be updated with
  new transformation settings, specified with \a matrix.
*/

/*!
  \fn QPaintEngine::updateClipRegion(const QRegion &region, bool enabled)

  This function is called when the clip region changes, specified by \a region or
  when clipping is enabled or disabled, specified by \a enabled.
*/

void QWrapperPaintEngine::updatePen(const QPen &pen) { wrap->updatePen(pen); }
void QWrapperPaintEngine::updateBrush(const QBrush &brush, const QPoint &pt)
{ wrap->updateBrush(brush, pt); }
void QWrapperPaintEngine::updateFont(const QFont &font) { wrap->updateFont(font); }
void QWrapperPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{ wrap->updateBackground(bgMode, bgBrush); }
void QWrapperPaintEngine::updateXForm(const QWMatrix &matrix) { wrap->updateXForm(matrix); }
void QWrapperPaintEngine::updateClipRegion(const QRegion &clipRegion, bool clipEnabled)
{ wrap->updateClipRegion(clipRegion, clipEnabled); }
void QWrapperPaintEngine::drawLine(const QPoint &p1, const QPoint &p2) { wrap->drawLine(p1, p2); }
void QWrapperPaintEngine::drawRect(const QRect &r) { wrap->drawRect(r); }
void QWrapperPaintEngine::drawPoint(const QPoint &p) { wrap->drawPoint(p); }
void QWrapperPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{ wrap->drawPoints(pa, index, npoints); }
void QWrapperPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{ wrap->drawRoundRect(r, xRnd, yRnd); }
void QWrapperPaintEngine::drawEllipse(const QRect &r)
{ wrap->drawEllipse(r); }
void QWrapperPaintEngine::drawArc(const QRect &r, int a, int alen)
{ wrap->drawArc(r, a, alen); }
void QWrapperPaintEngine::drawPie(const QRect &r, int a, int alen)
{ wrap->drawPie(r, a, alen); }
void QWrapperPaintEngine::drawChord(const QRect &r, int a, int alen)
{ wrap->drawChord(r, a, alen); }
void QWrapperPaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{ wrap->drawLineSegments(a, index, nlines); }
void QWrapperPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{ wrap->drawPolyline(pa, index, npoints); }
void QWrapperPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{ wrap->drawPolygon(pa, winding, index, npoints); }

void QWrapperPaintEngine::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{ wrap->drawConvexPolygon(a, index, npoints); }
#ifndef QT_NO_BEZIER
void QWrapperPaintEngine::drawCubicBezier(const QPointArray &a, int index)
{ wrap->drawCubicBezier(a, index); }
#endif

void QWrapperPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr,
                                     Qt::BlendMode mode)
{ wrap->drawPixmap(r, pm, sr, mode); }
void QWrapperPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{ wrap->drawTextItem(p, ti, textflags); }
void QWrapperPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s)
{ wrap->drawTiledPixmap(r, pixmap, s); }

