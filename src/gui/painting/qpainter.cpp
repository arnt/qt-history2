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

// QtGui
#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevice.h"
#include "qpaintengine.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpainterpath.h"
#include "qpicture.h"
#include "qpixmapcache.h"
#include "qpolygon.h"
#include "qtextlayout.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qstyle.h"

#include <private/qfontengine_p.h>
#include <private/qpaintengine_p.h>
#include <private/qpainterpath_p.h>
#include <private/qtextengine_p.h>
#include <private/qwidget_p.h>
#ifdef Q_WS_QWS
#include <private/qpixmap_p.h>
#endif
// QtCore
#include <qdebug.h>

// Other
#include <math.h>

// #define QT_DEBUG_DRAW
#ifdef QT_DEBUG_DRAW
bool qt_show_painter_debug_output = true;
#endif

extern QPixmap qt_pixmapForBrush(int style, bool invert);

void qt_format_text(const QFont &font,
                    const QRectF &_r, int tf, const QString& str, QRectF *brect,
                    int tabstops, int* tabarray, int tabarraylen,
                    QPainter *painter);

QPixmap qt_image_linear_gradient(const QRect &r,
                                 const QPointF &p1, const QColor &color1,
                                 const QPointF &p2, const QColor &color2);

template <class From, class To> QVector<To> qt_convert_points(const From *points, int pointCount, const To & /*dummy*/)
{
    QVector<To> v;
    v.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        v << points[i];
    return v;
}


/*
  Fallback implementation of fill linear gradient. Sets a clipregion matching the shape to
  fill and calls qt_fill_linear_gradient (lots of drawlines) for the region in question
*/
void QPainterPrivate::draw_helper_fill_lineargradient(const QPainterPath &path)
{
    Q_Q(QPainter);
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf(" -> fill_lineargradient()\n");
#endif
    q->save();
    q->setClipPath(path, Qt::IntersectClip);
    QRect bounds = (QPolygonF(path.boundingRect()) * state->matrix).boundingRect().toRect();
    q->resetMatrix();

    QPointF p1 = state->brush.gradientStart() - bounds.topLeft();
    QPointF p2 = state->brush.gradientStop() - bounds.topLeft();

    QPixmap image = qt_image_linear_gradient(bounds,
                                             p1, state->brush.color(),
                                             p2, state->brush.gradientColor());
    engine->drawPixmap(QRectF(bounds.topLeft(), image.size()),
                       image,
                       QRectF(0, 0, image.width(), image.height()),
                       Qt::ComposePixmap);
    q->restore();
}


/*
  Fallback implementation of fill alpha. Creates an alpha pixmap and sets a clipmask matching
  the primitive type to fill (no clip for rects though), then tiles the pixmap across the
  area.
*/
void QPainterPrivate::draw_helper_fill_alpha(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf(" -> fill_alpha()\n");
#endif
    const int BUFFERSIZE = 16;
    QImage image(BUFFERSIZE, BUFFERSIZE, 32);
    image.fill(state->brush.color().rgba());
    image.setAlphaBuffer(true);
    QPixmap pm(image);

    Q_Q(QPainter);
    QRectF bounds = path.boundingRect();

    q->save();
    q->setClipPath(path, Qt::IntersectClip);
    q->drawTiledPixmap(bounds, pm);
    q->restore();
}


/*
  Fallback implementation of fill patterns.
*/
void QPainterPrivate::draw_helper_fill_pattern(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        qDebug() << " -> fill_pattern()" << path.boundingRect();
#endif
    Q_Q(QPainter);
    q->save();
    q->setPen(QPen(state->brush.color(), 1));
    QRectF bounds = path.boundingRect();
    q->setClipPath(path, Qt::IntersectClip);

    qreal bottom = bounds.y() + bounds.height();
    qreal right = bounds.x() + bounds.width();

    switch (state->brush.style()) {
    case Qt::HorPattern:
        if (state->bgMode == Qt::OpaqueMode)
            q->fillRect(bounds, state->bgBrush);
        for (qreal y=bounds.y(); y<bottom; y+=8)
            q->drawLine(QLineF(bounds.x(), y, right, y));
        break;
    case Qt::VerPattern:
        if (state->bgMode == Qt::OpaqueMode)
            q->fillRect(bounds, state->bgBrush);
        for (qreal x=bounds.x(); x<right; x+=8)
            q->drawLine(QLineF(x, bounds.y(), x, bottom));
        break;
    case Qt::CrossPattern:
        if (state->bgMode == Qt::OpaqueMode)
            q->fillRect(bounds, state->bgBrush);
        for (qreal x=bounds.x(); x<right; x+=8)
            q->drawLine(QLineF(x, bounds.y(), x, bottom));
        for (qreal y=bounds.y(); y<bottom; y+=8)
            q->drawLine(QLineF(bounds.x(), y, right, y));
        break;
    default: {
        QPixmap pattern;
        if (state->brush.style() == Qt::TexturePattern)
            pattern = state->brush.texture();
        else
            pattern = qt_pixmapForBrush(state->brush.style(), true);

        q->drawTiledPixmap(bounds, pattern);
    }
    }

    q->restore();
}


/*!
  Fallback implementation for stroking. Will transform coordinates if necesary.
*/
void QPainterPrivate::draw_helper_stroke_normal(const QPainterPath &path, uint emulate)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf(" -> stroke normal()\n");
#endif
    QBrush originalBrush = state->brush;
    Q_Q(QPainter);
    q->setBrush(Qt::NoBrush);
    engine->updateState(state);

    if (engine->hasFeature(QPaintEngine::PainterPaths)) {
        engine->drawPath(emulate & QPaintEngine::CoordTransform
                         ? path * state->matrix
                         : path);
    } else {
        QList<QPolygonF> polygons = path.toSubpathPolygons();
        for (int i=0; i<polygons.size(); ++i) {
            if (emulate & QPaintEngine::CoordTransform) {
                QPolygonF xformed = polygons.at(i) * state->matrix;
                engine->drawPolygon(xformed.data(), xformed.size(), QPaintEngine::PolylineMode);
            } else {
                engine->drawPolygon(polygons.at(i).data(), polygons.at(i).size(),
                                    QPaintEngine::PolylineMode);
            }
        }
    }
    q->setBrush(originalBrush);
}


/*!
  Fallback implementation for path based stroking. This function is used to emulate
  pen width transformation.
*/
void QPainterPrivate::draw_helper_stroke_pathbased(const QPainterPath &input)
{
    QPainterPath path = input;
#ifdef QT_DEBUG_DRAW
    QRectF pathBounds = path.boundingRect();
    if (qt_show_painter_debug_output)
        printf(" -> stroke pathbased(), [%.2f,%.2f,%.2f,%.2f]\n",
               pathBounds.x(), pathBounds.y(), pathBounds.width(), pathBounds.height());
#endif

    Q_Q(QPainter);

    qreal width = state->pen.widthF();
    int txop = state->txop;
    if (!(state->pen.widthF() > .0f)) {
        if (state->txop != TxNone) {
            path = path * state->matrix;
            txop = TxNone;
        }
        width = 1.f;
    }

    QPainterPathStroker stroker;
    stroker.setWidth(width);
    stroker.setDashPattern(state->pen.style());
    stroker.setCapStyle(state->pen.capStyle());
    stroker.setJoinStyle(state->pen.joinStyle());

    // Increase the threshold based on the width and the scale factor of the matrix.
    if (txop > TxTranslate)
        stroker.setCurveThreshold(width / (2 * 10 * state->matrix.m11() * state->matrix.m22()));

    QPainterPath stroke = stroker.createStroke(path);

    if (txop > TxNone)
        stroke = stroke * state->matrix;

    q->save();
    q->resetMatrix();
    // need to set brush with current pen bf changing pen
    q->setBrush(state->pen.brush());
    q->setPen(Qt::NoPen);
    engine->syncState();
    if (engine->hasFeature(QPaintEngine::PainterPaths)) {
        engine->drawPath(stroke);
    } else {
        QList<QPolygonF> polygons = stroke.toFillPolygons();
        for (int i=0; i<polygons.size(); ++i)
            engine->drawPolygon(polygons.at(i).data(), polygons.at(i).size(), QPaintEngine::WindingMode);
    }
    q->restore();
}


// ### make this inline when the X11 define has been removed
void QPainterPrivate::draw_helper(const QPainterPath &path, DrawOperation operation)
{
    draw_helper(path, operation, engine->emulationSpecifier);
}

void QPainterPrivate::draw_helper(const QPainterPath &path, DrawOperation op,
                                  uint emulationSpecifier)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output) {
        printf("QPainter::drawHelper: op=%d, emulation=0x%x ( ", op, emulationSpecifier);
        static struct { uint value; const char *text; } emuMap[] = {
            {    0x0001, "CoordTransform "},
            {    0x0002, "PenWidthTransform "},
            {    0x0004, "PatternTransform "},
            {    0x0008, "PatternBrush "},
            {    0x0010, "PixmapTransform "},
            {    0x0020, "LinearGradients "},
            {    0x0040, "LinearGradientFillPolygon "},
            {    0x0080, "PixmapScale "},
            {    0x0100, "AlphaFill "},
            {    0x0200, "AlphaFillPolygon "},
            {    0x0400, "AlphaStroke "},
            {    0x0800, "AlphaPixmap "},
            {    0x1000, "PainterPaths "},
            {    0x2000, "ClipTransform "},
            {    0x4000, "LineAntialiasing "},
            {    0x10000, "BrushStroke"},
            {0x10000000, "UsesFontEngine "},
            {0x20000000, "PaintOutsidePaintEvent "},
            {       0x0, 0x0},
        };
        for (int i = 0; emuMap[i].text; ++i)
            if (emulationSpecifier & emuMap[i].value)
                printf(emuMap[i].text);
        printf(")\n");
    }
#endif
    enum { Normal, PathBased, None } outlineMode = Normal;
    if (state->pen.style() == Qt::NoPen)
        op = DrawOperation(op&~StrokeDraw);
    if (state->brush.style() == Qt::NoBrush)
        op = DrawOperation(op&~FillDraw);

    Q_Q(QPainter);

    // Creates an outline path to handle the stroking.
    if ((op & StrokeDraw)
        && (emulationSpecifier & QPaintEngine::PenWidthTransform
            || emulationSpecifier & QPaintEngine::AlphaStroke
            || emulationSpecifier & QPaintEngine::LineAntialiasing
            || emulationSpecifier & QPaintEngine::BrushStroke))
        outlineMode = PathBased;

    if (op & FillDraw) {

        // Custom fill, gradients, alpha and patterns
        if ((emulationSpecifier & QPaintEngine::LinearGradients)
            || (emulationSpecifier & QPaintEngine::AlphaFill)
            || (emulationSpecifier & QPaintEngine::PatternTransform)
            || (emulationSpecifier & QPaintEngine::PatternBrush))
        {
            if (((emulationSpecifier & QPaintEngine::LinearGradients)
                 && engine->hasFeature(QPaintEngine::LinearGradientFillPolygon))
                || ((emulationSpecifier & QPaintEngine::AlphaFill)
                    && engine->hasFeature(QPaintEngine::AlphaFillPolygon))) {
                q->drawPath(path);
            } else {
                if (emulationSpecifier & QPaintEngine::LinearGradients)
                    draw_helper_fill_lineargradient(path);
                else if (emulationSpecifier & QPaintEngine::AlphaFill)
                    draw_helper_fill_alpha(path);
                else if (emulationSpecifier & QPaintEngine::PatternTransform
                         || emulationSpecifier & QPaintEngine::PatternBrush)
                    draw_helper_fill_pattern(path);
            }

            // XFormed fills
        } else if (emulationSpecifier & QPaintEngine::CoordTransform) {
            q->save();
            if (outlineMode == PathBased)
                q->setPen(Qt::NoPen);
            else
                outlineMode = None;
            if (engine->hasFeature(QPaintEngine::PainterPaths)) {
                QPainterPath pathCopy = path;
                if (emulationSpecifier & QPaintEngine::CoordTransform)
                    pathCopy = pathCopy * state->matrix;
                q->resetMatrix();
                engine->updateState(state);
                engine->drawPath(pathCopy);
            } else {
                QPolygonF xformed = path.toFillPolygon(state->matrix);
                q->resetMatrix();
                engine->updateState(state);
                engine->drawPolygon(xformed.data(), xformed.size(),
                                    QPaintEngine::PolygonDrawMode(path.fillRule()));
            }
            q->restore();
        // Normal fills, custom outlining only, which is done later.
        } else {
            QPen oldPen = state->pen;
            q->setPen(Qt::NoPen);
            engine->updateState(state);
            if (engine->hasFeature(QPaintEngine::PainterPaths)) {
                engine->drawPath(path);
            } else {
                QList<QPolygonF> polys = path.toFillPolygons();
                for (int i=0; i<polys.size(); ++i)
                    engine->drawPolygon(polys.at(i).data(), polys.at(i).size(),
                                        QPaintEngine::PolygonDrawMode(path.fillRule()));
            }
            q->setPen(oldPen);
        }
    } // end of filling

    // Do the outlining...
    if (op & StrokeDraw) {
        if (outlineMode == Normal && state->pen.style() != Qt::NoPen) {
            draw_helper_stroke_normal(path, emulationSpecifier);
        } else if (outlineMode == PathBased) {
            draw_helper_stroke_pathbased(path);
        }
    } // end of stroking
}


void QPainterPrivate::init()
{
    Q_Q(QPainter);
    state->painter = q;
}

void QPainterPrivate::updateMatrix()
{
    QMatrix m;
    if (state->VxF) {
        qreal scaleW = qreal(state->vw)/qreal(state->ww);
        qreal scaleH = qreal(state->vh)/qreal(state->wh);
        m.setMatrix(scaleW, 0, 0, scaleH, state->vx - state->wx*scaleW, state->vy - state->wy*scaleH);
    }
    if (state->WxF) {
        if (state->VxF)
            m = state->worldMatrix * m;
        else
            m = state->worldMatrix;
    }
    state->matrix = m;

    txinv = false;                                // no inverted matrix
    state->txop  = TxNone;
    if (state->matrix.m12()==0.0 && state->matrix.m21()==0.0
        && state->matrix.m11() >= 0.0 && state->matrix.m22() >= 0.0) {
        if (state->matrix.m11()==1.0 && state->matrix.m22()==1.0) {
            if (state->matrix.dx()!=0.0 || state->matrix.dy()!=0.0)
                state->txop = TxTranslate;
        } else {
            state->txop = TxScale;
        }
    } else {
        state->txop = TxRotShear;
    }
    if (!redirection_offset.isNull()) {
        state->txop |= TxTranslate;
        state->WxF = true;
        // We want to translate in dev space so we do the adding of the redirection
        // offset manually.
        state->matrix = QMatrix(state->matrix.m11(), state->matrix.m12(),
                              state->matrix.m21(), state->matrix.m22(),
                              state->matrix.dx()-redirection_offset.x(),
                              state->matrix.dy()-redirection_offset.y());
    }
    engine->setDirty(QPaintEngine::DirtyTransform);
//     printf("VxF=%d, WxF=%d\n", state->VxF, state->WxF);
//     printf("Using matrix: %f, %f, %f, %f, %f, %f\n",
//            state->matrix.m11(),
//            state->matrix.m12(),
//            state->matrix.m21(),
//            state->matrix.m22(),
//            state->matrix.dx(),
//            state->matrix.dy());
}

/*! \internal */
void QPainterPrivate::updateInvMatrix()
{
    Q_ASSERT(txinv == false);
    txinv = true;                                // creating inverted matrix
    bool invertible;
    QMatrix m;
    if (state->VxF) {
        m.translate(state->vx, state->vy);
        m.scale(1.0*state->vw/state->ww, 1.0*state->vh/state->wh);
        m.translate(-state->wx, -state->wy);
    }
    if (state->WxF) {
        if (state->VxF)
            m = state->worldMatrix * m;
        else
            m = state->worldMatrix;
    }
    invMatrix = m.inverted(&invertible);                // invert matrix
}

/*!
    \class QPainter
    \brief The QPainter class does low-level painting e.g. on widgets.

    \ingroup multimedia
    \mainclass

    The painter provides highly optimized functions to do most of the
    drawing GUI programs require. QPainter can draw everything from
    simple lines to complex shapes like pies and chords. It can also
    draw aligned text and pixmaps. Normally, it draws in a "natural"
    coordinate system, but it can also do view and world
    transformation.

    The typical use of a painter is:

    \list
    \i Construct a painter.
    \i Set a pen, a brush etc.
    \i Draw.
    \i Destroy the painter.
    \endlist

    Mostly, all this is done inside a paint event. (In fact, 99% of
    all QPainter use is in a reimplementation of
    QWidget::paintEvent(), and the painter is heavily optimized for
    such use.) Here's one very simple example:

    \code
    void SimpleExampleWidget::paintEvent()
    {
        QPainter paint(this);
        paint.setPen(Qt::blue);
        paint.drawText(rect(), Qt::AlignCenter, "The Text");
    }
    \endcode

    If you need to draw a complex shape, especially if you need to do
    so repeatedly, consider creating a QPainterPath and drawing it
    using drawPath().

    Usage is simple, and there are many settings you can use:

    \list

    \i font() is the currently set font. If you set a font that isn't
    available, Qt finds a close match. In fact font() returns what
    you set using setFont() and fontInfo() returns the font actually
    being used (which may be the same).

    \i brush() is the currently set brush; the color or pattern that's
    used for filling e.g. circles.

    \i pen() is the currently set pen; the color or stipple that's
    used for drawing lines or boundaries.

    \i backgroundMode() is \c Opaque or \c Transparent, i.e. whether
    backgroundColor() is used or not.

    \i backgroundColor() only applies when backgroundMode() is Opaque
    and pen() is a stipple. In that case, it describes the color of
    the background pixels in the stipple.

    \i brushOrigin() is the origin of the tiled brushes, normally the
    origin of the window.

    \i viewport(), window(), matrix() and many more make up the
    painter's coordinate transformation system. See \link
    coordsys.html The Coordinate System \endlink for an explanation of
    this, or see below for a very brief overview of the functions.

    \i hasClipping() is whether the painter clips at all. (The paint
    device clips, too.) If the painter clips, it clips to clipRegion().

    \i pos() is the current position, set by moveTo() and used by
    lineTo().

    \endlist

    Note that some of these settings mirror settings in some paint
    devices, e.g. QWidget::font(). QPainter::begin() (or the QPainter
    constructor) copies these attributes from the paint device.
    Calling, for example, QWidget::setFont() doesn't take effect until
    the next time a painter begins painting on it.

    save() saves all of these settings on an internal stack, restore()
    pops them back.

    The core functionality of QPainter is drawing, and there are
    functions to draw most primitives: drawPoint(), drawPoints(),
    drawLine(), drawRect(), drawRoundRect(),
    drawEllipse(), drawArc(), drawPie(), drawChord(),
    drawLine:Segments(), drawPolyline(), drawPolygon(),
    drawConvexPolygon() and drawCubicBezier(). All of these functions
    take integer coordinates; there are no floating-point versions
    since we want drawing to be as fast as possible.

    \table
    \row \i
    \inlineimage qpainter-angles.png
    \i The functions that draw curved primitives accept angles measured
    in 1/16s of a degree. Angles are measured in an counter-clockwise
    direction from the rightmost edge of the primitive being drawn.
    \endtable

    There are functions to draw pixmaps/images, namely drawPixmap(),
    drawImage() and drawTiledPixmap(). drawPixmap() and drawImage()
    produce the same result, except that drawPixmap() is faster
    on-screen while drawImage() may be faster on QPrinter and other
    devices.

    Text drawing is done using drawText(), and when you need
    fine-grained positioning, boundingRect() tells you where a given
    drawText() command would draw.

    There is a drawPicture() function that draws the contents of an
    entire QPicture using this painter. drawPicture() is the only
    function that disregards all the painter's settings: the QPicture
    has its own settings.

    Normally, the QPainter operates on the device's own coordinate
    system (usually pixels), but QPainter has good support for
    coordinate transformation. See \link coordsys.html The Coordinate
    System \endlink for a more general overview and a simple example.

    The most common functions used are scale(), rotate(), translate()
    and shear(), all of which operate on the matrix().
    setMatrix() can replace or add to the currently set
    matrix().

    setViewport() sets the rectangle on which QPainter operates. The
    default is the entire device, which is usually fine, except on
    printers. setWindow() sets the coordinate system, that is, the
    rectangle that maps to viewport(). What's drawn inside the
    window() ends up being inside the viewport(). The window's
    default is the same as the viewport, and if you don't use the
    transformations, they are optimized away, gaining another little
    bit of speed.

    After all the coordinate transformation is done, QPainter can clip
    the drawing to an arbitrary rectangle or region. hasClipping() is
    true if QPainter clips, and clipRegion() returns the clip region.
    You can set it using either setClipRegion() or setClipRect().
    Note that the clipping can be slow. It's all system-dependent,
    but as a rule of thumb, you can assume that drawing speed is
    inversely proportional to the number of rectangles in the clip
    region.

    After QPainter's clipping, the paint device may also clip. For
    example, most widgets clip away the pixels used by child widgets,
    and most printers clip away an area near the edges of the paper.
    This additional clipping is not reflected by the return value of
    clipRegion() or hasClipping().

    QPainter also includes some less-used functions that are very
    useful on those occasions when they're needed.

    isActive() indicates whether the painter is active. begin() (and
    the most usual constructor) makes it active. end() (and the
    destructor) deactivates it. If the painter is active, device()
    returns the paint device on which the painter paints.

    Sometimes it is desirable to make someone else paint on an unusual
    QPaintDevice. QPainter supports a static function to do this,
    redirect(). We recommend not using it, but for some hacks it's
    perfect.

    setTabStops() and setTabArray() can change where the tab stops
    are, but these are very seldomly used.

    \warning A QPainter can only be used inside a paintEvent() or a
    function called by a paintEvent().

    \warning Note that QPainter does not attempt to work around
    coordinate limitations in the underlying window system. Some
    platforms may behave incorrectly with coordinates outside +/-4000.

    \headerfile qdrawutil.h

    \sa QPaintDevice QWidget QPixmap QPrinter QPicture
        \link simple-application.html Application Walkthrough \endlink
        \link coordsys.html Coordinate System Overview \endlink
*/

/*!
    \enum QPainter::RenderHint

    \internal

    \value Antialiasing
*/

/*!
    \enum QPainter::TextDirection
    This enum describes the direction in which text is rendered by
    the painter.

    \value Auto The default text direction is used.
    \value RTL  Text is rendered from right to left.
    \value LTR  Text is rendered from left to right.

    \sa drawText()
*/

/*!
    Constructs a painter.

    Notice that all painter settings (setPen, setBrush etc.) are reset
    to default values when begin() is called.

    \sa begin(), end()
*/

QPainter::QPainter()
{
    d_ptr = new QPainterPrivate(this);
    d_ptr->init();
}

/*!
    Constructs a painter that begins painting the paint device \a pd
    immediately.

    This constructor is convenient for short-lived painters, e.g. in a
    \link QWidget::paintEvent() paint event\endlink and should be used
    only once. The constructor calls begin() for you and the QPainter
    destructor automatically calls end().

    Here's an example using begin() and end():
    \code
        void MyWidget::paintEvent(QPaintEvent *)
        {
            QPainter p;
            p.begin(this);
            p.drawLine(...);        // drawing code
            p.end();
        }
    \endcode

    The same example using this constructor:
    \code
        void MyWidget::paintEvent(QPaintEvent *)
        {
            QPainter p(this);
            p.drawLine(...);        // drawing code
        }
    \endcode

    Since the constructor cannot provide feedback when the initialization
    of the painter failed you should rather use begin() and end() to paint
    on external devices, e.g. printers.

    \sa begin(), end()
*/

QPainter::QPainter(QPaintDevice *pd)
{
    d_ptr = new QPainterPrivate(this);
    d_ptr->init();
    Q_ASSERT(pd != 0);
    begin(pd);
}

/*!
    Destroys the painter.
*/

QPainter::~QPainter()
{
    if (isActive())
        end();
    delete d_ptr;
}

/*!
    Returns the paint device on which this painter is currently
    painting, or 0 if the painter is not active.

    \sa QPaintDevice::paintingActive()
*/

QPaintDevice *QPainter::device() const
{
    Q_D(const QPainter);
    return d->device;
}

/*!
    Returns true if the painter is active painting, i.e. begin() has
    been called and end() has not yet been called; otherwise returns
    false.

    \sa QPaintDevice::paintingActive()
*/

bool QPainter::isActive() const
{
    Q_D(const QPainter);
    if (d->engine) {
        return d->engine->isActive();
    }
    return false;
}

/*!
  Initializes the painters pen, background and font to the same as \a widget.
*/
void QPainter::initFrom(const QWidget *widget)
{
    Q_ASSERT_X(widget, "QPainter::initFrom(const QWidget *widget)", "Widget cannot be 0");
    QPalette pal = widget->palette();
    Q_D(QPainter);
    d->state->pen = pal.color(QPalette::Foreground);
    d->state->bgBrush = pal.background();
    d->state->font = widget->font();
    if (d->engine) {
        d->engine->setDirty(QPaintEngine::DirtyPen);
        d->engine->setDirty(QPaintEngine::DirtyBrush);
        d->engine->setDirty(QPaintEngine::DirtyFont);
    }
    d->state->layoutDirection = widget->layoutDirection();
}


/*!
    Saves the current painter state (pushes the state onto a stack). A
    save() must be followed by a corresponding restore(). end()
    unwinds the stack.

    \sa restore()
*/

void QPainter::save()
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::save()\n");
#endif
    if (!isActive()) {
        qWarning("QPainter::save(), painter not active");
        return;
    }

    Q_D(QPainter);
    d->engine->syncState();

    d->state = new QPainterState(d->states.back());
    // We need top propagate pending changes to the restore later..
    d->state->changeFlags = d->engine->dirtyFlag;
    d->states.push_back(d->state);
    d->engine->updateState(d->state, false);
}

/*!
    Restores the current painter state (pops a saved state off the
    stack).

    \sa save()
*/

void QPainter::restore()
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::restore()\n");
#endif
    Q_D(QPainter);
    if (d->states.size()==0) {
        qWarning("QPainter::restore(), unbalanced save/restore");
        return;
    } else if (!isActive()) {
        qWarning("QPainter::restore(), painter not active");
        return;
    }

    QPainterState *tmp = d->state;
    d->states.pop_back();
    d->state = d->states.back();
    d->txinv = false;

    // trigger clip update if the clip path/region has changed since
    // last save
    if (!d->state->clipInfo.isEmpty()
        && (tmp->changeFlags & (QPaintEngine::DirtyClip | QPaintEngine::DirtyClipPath))) {
        d->state->tmpClipRegion = clipRegion();
        d->state->tmpClipOp = Qt::ReplaceClip;
        //Since we're updating the clip region anyway, pretend that the clip path hasn't changed:
        tmp->changeFlags &= ~QPaintEngine::DirtyClipPath;
        d->engine->setDirty(QPaintEngine::DirtyClip);
    }

    d->engine->updateState(d->state);
    delete tmp;
}


/*!
    Begins painting the paint device \a pd and returns true if
    successful; otherwise returns false.

    The errors that can occur are serious problems, such as these:

    \code
        p->begin(0); // impossible - paint device cannot be 0

        QPixmap pm(0, 0);
        p->begin(&pm); // impossible - pm.isNull();

        p->begin(myWidget);
        p2->begin(myWidget); // impossible - only one painter at a time
    \endcode

    Note that most of the time, you can use one of the constructors
    instead of begin(), and that end() is automatically done at
    destruction.

    \warning A paint device can only be painted by one painter at a
    time.

    \sa end()
*/

bool QPainter::begin(QPaintDevice *pd)
{
    Q_ASSERT(pd);

    Q_D(QPainter);
    if (d->engine) {
        qWarning("QPainter::begin(): Painter is already active."
                 "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    // Ensure fresh painter state
    d->state->init(d->state->painter);

    QPaintDevice *originalDevice = pd;
    QPaintDevice *rpd = redirected(pd, &d->redirection_offset);

    if (rpd) {
        pd = rpd;
    }

#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::begin(), device=%p, type=%d\n", pd, pd->devType());
#endif


    d->state->bgOrigin -= d->redirection_offset; // This will accumulate!! ############

    d->device = pd;
    d->engine = pd->paintEngine();

    if (!d->engine) {
        qWarning("QPainter::begin(), paintdevice returned engine == 0, type: %d\n", pd->devType());
        return true;
    }

    switch (pd->devType()) {
        case QInternal::Widget:
        {
            const QWidget *widget = static_cast<const QWidget *>(pd);
            Q_ASSERT(widget);

            if(!d->engine->hasFeature(QPaintEngine::PaintOutsidePaintEvent)
               && !widget->testAttribute(Qt::WA_PaintOutsidePaintEvent)
               && !widget->testAttribute(Qt::WA_WState_InPaintEvent)) {
                qWarning("QPainter::begin: Widget painting can only begin as a "
                         "result of a paintEvent");
                return false;
            }
            d->state->ww = d->state->vw = widget->width();
            d->state->wh = d->state->vh = widget->height();
            break;
        }
        case QInternal::Pixmap:
        {
            const QPixmap *pm = static_cast<const QPixmap *>(pd);
            Q_ASSERT(pm);
            const_cast<QPixmap *>(pm)->detach();
            d->state->ww = d->state->vw = pm->width();
            d->state->wh = d->state->vh = pm->height();
            if (pm->depth() == 1) {
                d->state->pen = QPen(Qt::color1);
                d->state->brush = QBrush(Qt::color0);
            }
            break;
        }
        case QInternal::ExternalDevice:
        {
            d->state->ww = d->state->vw = pd->metric(QPaintDevice::PdmWidth);
            d->state->wh = d->state->vh = pd->metric(QPaintDevice::PdmHeight);
        }
    }

    // Copy painter properties from original paint device,
    // required for QPixmap::grabWidget()
    if (originalDevice->devType() == QInternal::Widget) {
        QWidget *widget = static_cast<QWidget *>(originalDevice);
        d->state->deviceFont = widget->font();
        d->state->pen = widget->palette().color(widget->foregroundRole());
        d->state->bgBrush = widget->palette().brush(widget->backgroundRole());
    }

    // make sure we have a font compatible with the paintdevice
    d->state->deviceFont = d->state->font = QFont(d->state->deviceFont, d->device);

    if (d->state->ww == 0) // For compat with 3.x painter defaults
        d->state->ww = d->state->wh = d->state->vw = d->state->vh = 1024;

    // Slip a painter state into the engine before we do any other operations
    d->engine->state = d->state;

    d->engine->setPaintDevice(pd);

    if (!d->engine->begin(pd)) {
        qWarning("QPainter::begin(), QPaintEngine::begin() returned false\n");
        return false;
    }

    Q_ASSERT(d->engine->isActive());

    if (!d->redirection_offset.isNull())
        d->updateMatrix();

    Q_ASSERT(d->engine->isActive());
    d->engine->setRenderHint(QPainter::Antialiasing, false);
    d->engine->setRenderHint(QPainter::TextAntialiasing, true);
    ++d->device->painters;

    d->engine->emulationSpecifier = 0;

    return true;
}

/*!
    Ends painting. Any resources used while painting are released. You
    don't normally need to call this since it is called by the
    destructor.

    \sa begin(), isActive()
*/

bool QPainter::end()
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::end()\n");
#endif

    if (!isActive()) {
        qWarning("QPainter::end: Painter is not active, aborted");
        return false;
    }

    Q_D(QPainter);
    if (d->states.size()>1) {
        qWarning("QPainter::end(), painter ended with %d saved states",
                 d->states.size());
    }

    bool ended = d->engine->end();
    d->engine->updateState(0);
    d->engine->setPaintDevice(0);

    if (d->engine->autoDestruct()) {
        delete d->engine;
    }

    d->engine = 0;

    --d->device->painters;
    return ended;
}


/*!
    Returns the paint engine that the painter is currently operating
    on, if the painter is active; otherwise 0.
*/
QPaintEngine *QPainter::paintEngine() const
{
    Q_D(const QPainter);
    return d->engine;
}


/*!
    Returns the font metrics for the painter, if the painter is
    active. It is not possible to obtain metrics for an inactive
    painter, so the return value is undefined if the painter is not
    active.

    \sa fontInfo(), isActive()
*/

QFontMetrics QPainter::fontMetrics() const
{
    Q_D(const QPainter);
    if (d->engine)
        d->engine->updateState(d->state);
    return QFontMetrics(d->state->pfont ? *d->state->pfont : d->state->font, d->device);
}


/*!
    Returns the font info for the painter, if the painter is active.
    It is not possible to obtain font information for an inactive
    painter, so the return value is undefined if the painter is not
    active.

    \sa fontMetrics(), isActive()
*/

QFontInfo QPainter::fontInfo() const
{
    Q_D(const QPainter);
    if (d->engine)
        d->engine->updateState(d->state);
    return QFontInfo(d->state->pfont ? *d->state->pfont : d->state->font);
}

/*!
    Returns the brush origin currently set.

    \sa setBrushOrigin()
*/

QPoint QPainter::brushOrigin() const
{
    Q_D(const QPainter);
    return QPointF(d->state->bgOrigin + d->redirection_offset).toPoint();
}

/*!
    \fn void QPainter::setBrushOrigin(const QPoint &p)

    \overload

    Sets the brush's origin to \a p.
*/

/*!
    \fn void QPainter::setBrushOrigin(int x, int y)

    \overload

    Sets the brush's origin to point (\a x, \a y).
*/

/*!
    Sets the brush origin to \a p.

    The brush origin specifies the (0, 0) coordinate of the painter's
    brush. This setting only applies to pattern brushes and pixmap
    brushes.

    \sa brushOrigin()
*/

void QPainter::setBrushOrigin(const QPointF &p)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setBrushOrigin(), (%.2f,%.2f)\n", p.x(), p.y());
#endif
    d->state->bgOrigin = p - d->redirection_offset;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyBrush);
}

/*!
    Returns the current background brush.

    \sa setBackground() QBrush
*/

const QBrush &QPainter::background() const
{
    Q_D(const QPainter);
    return d->state->bgBrush;
}


/*!
    Returns true if clipping has been set; otherwise returns false.

    \sa setClipping()
*/

bool QPainter::hasClipping() const
{
    Q_D(const QPainter);
    return d->state->tmpClipOp != Qt::NoClip;
}


/*!
    Enables clipping if \a enable is true, or disables clipping if \a
    enable is false.

    \sa hasClipping(), setClipRect(), setClipRegion()
*/

void QPainter::setClipping(bool enable)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setClipping(), enable=%s, was=%s\n",
               enable ? "on" : "off",
               hasClipping() ? "on" : "off");
#endif
    if (!isActive()) {
        qWarning("QPainter::setClipping(), painter not active, state will be reset by begin");
        return;
    }

    if (hasClipping() == enable)
        return;

    if (enable) {
        d->state->tmpClipRegion = clipRegion();
        d->state->tmpClipOp = Qt::ReplaceClip;
    } else {
        d->state->tmpClipRegion = QRegion();
        d->state->tmpClipOp = Qt::NoClip;
    }
    d->engine->setDirty(QPaintEngine::DirtyClip);
    d->engine->updateState(d->state);
}


/*!
    Returns the currently set clip region. Note that the clip region
    is given in logical coordinates and \e subject to
    \link coordsys.html coordinate transformation \endlink.

    \sa setClipRegion(), setClipRect(), setClipping()
*/

QRegion QPainter::clipRegion() const
{
    if (!isActive()) {
        qWarning("QPainter::clipRegion(), painter not active");
        return QRegion();
    }

    Q_D(const QPainter);
    QRegion region;
    bool lastWasNothing = true;

    if (!d->txinv)
        const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();

    for (int i=0; i<d->state->clipInfo.size(); ++i) {
        const QPainterClipInfo &info = d->state->clipInfo.at(i);
        QRegion other;
        switch (info.clipType) {

        case QPainterClipInfo::RegionClip: {
            QMatrix matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
                region = info.region * matrix;
                lastWasNothing = false;
                continue;
            }
            if (info.operation == Qt::IntersectClip)
                region &= info.region * matrix;
            else if (info.operation == Qt::UniteClip)
                region |= info.region * matrix;
            else if (info.operation == Qt::NoClip) {
                lastWasNothing = true;
                region = QRegion();
            } else
                region = info.region * matrix;
            break;
        }

        case QPainterClipInfo::PathClip: {
            QMatrix matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
                region = QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                 info.path.fillRule());
                lastWasNothing = false;
                continue;
            }
            if (info.operation == Qt::IntersectClip) {
                region &= QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                  info.path.fillRule());
            } else if (info.operation == Qt::UniteClip) {
                region |= QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                  info.path.fillRule());
            } else if (info.operation == Qt::NoClip) {
                lastWasNothing = true;
                region = QRegion();
            } else {
                region = QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                 info.path.fillRule());
            }
            break;
        }
        }
    }

    return region;
}

/*!
    Returns the currently clip as a path. Note that the clip path is
    given in logical coordinates and \e subject to \link coordsys.html
    coordinate transformation \endlink
*/
QPainterPath QPainter::clipPath() const
{
    // ### Since we do not support path intersections and path unions yet,
    // we just use clipRegion() here...
    if (!isActive()) {
        qWarning("QPainter::clipPath(), painter not active");
        return QPainterPath();
    }

    Q_D(const QPainter);
    // No clip, return empty
    if (d->state->clipInfo.size() == 0) {
        return QPainterPath();
    } else {

        // Update inverse matrix, used below.
        if (!d->txinv)
            const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();

        // For the simple case avoid conversion.
        if (d->state->clipInfo.size() == 1
            && d->state->clipInfo.at(0).clipType == QPainterClipInfo::PathClip) {
            QMatrix matrix = (d->state->clipInfo.at(0).matrix * d->invMatrix);
            return d->state->clipInfo.at(0).path * matrix;

        // Fallback to clipRegion() for now, since we don't have isect/unite for paths
        } else {
            QPainterPath path;
            path.addRegion(clipRegion());
            return path;
        }
    }
}

/*!
    \fn void QPainter::setClipRect(const QRect &rect, Qt::ClipOperation op)

    Sets the clip region to the rectangle \a rect using the clip
    operation \a op. The default operation is to replace the current
    clip rectangle.

    \sa setClipRegion(), clipRegion(), setClipping()
*/

/*!
    \fn void QPainter::setClipRect(int x, int y, int w, int h, Qt::ClipOperation op)

    Sets the clip region to the rectangle \a x, \a y, \a w, \a h and
    enables clipping.

    \sa setClipRegion(), clipRegion(), setClipping()
*/

/*!
    \overload

    Sets the clip region of the rectange \a rect.
*/
void QPainter::setClipRect(const QRectF &rect, Qt::ClipOperation op)
{
    QPainterPath path;
    path.addRect(rect);
    setClipPath(path, op);
}

/*!
    Sets the clip region to \a r using the clip operation \a op. The
    default clip operation is to replace the current clip region.

    Note that the clip region is given in logical coordinates
    and \e subject to \link coordsys.html coordinate
    transformation.\endlink

    \sa setClipRect(), clipRegion(), setClipping()
*/

void QPainter::setClipRegion(const QRegion &r, Qt::ClipOperation op)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    QRect rect = r.boundingRect();
    if (qt_show_painter_debug_output)
        printf("QPainter::setClipRegion(), size=%d, [%d,%d,%d,%d]\n",
           r.rects().size(), rect.x(), rect.y(), rect.width(), rect.height());
#endif
    Q_ASSERT(op != Qt::NoClip);
    if (!isActive()) {
        qWarning("QPainter::setClipRegion(); painter not active");
        return;
    }

//     if (d->state->clipInfo.size() == 0 && op != Qt::NoClip)
//         op = Qt::ReplaceClip;

    d->state->tmpClipRegion = r;
    d->state->tmpClipOp = op;
    if (op == Qt::NoClip || op == Qt::ReplaceClip)
        d->state->clipInfo.clear();
    d->state->clipInfo << QPainterClipInfo(r, op, d->state->worldMatrix);
    d->engine->setDirty(QPaintEngine::DirtyClip);
    d->engine->updateState(d->state);
}

/*!
    Sets the transformation matrix to \a matrix and enables transformations.

    If \a combine is true, then \a matrix is combined with the current
    transformation matrix; otherwise \a matrix replaces the current
    transformation matrix.

    If \a matrix is the identity matrix and \a combine is false, this
    function calls setMatrixEnabled(false). (The identity matrix is the
    matrix where QMatrix::m11() and QMatrix::m22() are 1.0 and the
    rest are 0.0.)

    World transformations are applied after the view transformations
    (i.e. \link setWindow() window\endlink and \link setViewport()
    viewport\endlink).

    The following functions can transform the coordinate system without using
    a QMatrix:
    \list
    \i translate()
    \i scale()
    \i shear()
    \i rotate()
    \endlist

    They operate on the painter's worldMatrix() and are implemented like this:

    \code
        void QPainter::rotate(qreal a)
        {
            QMatrix m;
            m.rotate(a);
            setMatrix(m, true);
        }
    \endcode

    Note that you should always use \a combine when you are drawing
    into a QPicture. Otherwise it may not be possible to replay the
    picture with additional transformations. Using translate(),
    scale(), etc., is safe.

    For a brief overview of coordinate transformation, see the \link
    coordsys.html Coordinate System Overview. \endlink

    \sa matrix(), setMatrixEnabled(), QMatrix
*/

void QPainter::setMatrix(const QMatrix &matrix, bool combine)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setMatrix(), combine=%d\n", combine);
#endif

   if (!isActive()) {
        qWarning("QPainter::setMatrix(), painter not active ");
        return;
    }

    if (combine)
        d->state->worldMatrix = matrix * d->state->worldMatrix;                        // combines
    else
        d->state->worldMatrix = matrix;                                // set new matrix
//     bool identity = d->state->worldMatrix.isIdentity();
//     if (identity && pdev->devType() != QInternal::Picture)
//         setWorldXForm(false);
//     else
    if (!d->state->WxF)
        setMatrixEnabled(true);
    else
        d->updateMatrix();
}

/*!
    Returns the world transformation matrix.

    \sa setMatrix()
*/

const QMatrix &QPainter::matrix() const
{
    Q_D(const QPainter);
    return d->state->worldMatrix;
}


/*!
    Returns the matrix that transforms from logical coordinates to
    device coordinates of the platform dependent paintdevice.

    This function is ONLY needed when using platform painting commands
    on the platform dependent handle, and the platform does not do
    transformations nativly.

    \sa matrix(), QPaintEngine::hasFeature()
*/
const QMatrix &QPainter::deviceMatrix() const
{
    Q_D(const QPainter);
    return d->state->matrix;
}

/*!
    Resets any transformations that were made using translate(), scale(),
    shear(), rotate(), setMatrix(), setViewport() and
    setWindow().

    \sa matrix(), setMatrix()
*/
void QPainter::resetMatrix()
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::resetMatrix()\n");
#endif
    if (!isActive()) {
        qWarning("QPainter::resetMatrix(), painter not active");
        return;
    }

    d->state->wx = d->state->wy = d->state->vx = d->state->vy = 0;                        // default view origins
    d->state->ww = d->state->vw = d->device->metric(QPaintDevice::PdmWidth);
    d->state->wh = d->state->vh = d->device->metric(QPaintDevice::PdmHeight);
    d->state->worldMatrix = QMatrix();
    setMatrixEnabled(false);
    setViewTransformEnabled(false);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyTransform);
}


/*!
    Enables transformations if \a enable is true, or disables
    world transformations if \a enable is false. The world
    transformation matrix is not changed.

    \sa setMatrix(), matrix()
*/

void QPainter::setMatrixEnabled(bool enable)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setMatrixEnabled(), enable=%d\n", enable);
#endif

    if (!isActive()) {
        qWarning("QPainter::setMatrixEnabled(), painter not active");
        return;
    }
    if (enable == d->state->WxF)
        return;

    d->state->WxF = enable;
    d->updateMatrix();
}

/*!
    Returns true if world transformation is enabled; otherwise returns
    false.

    \sa setMatrixEnabled(), setMatrix()
*/

bool QPainter::matrixEnabled() const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->WxF;
#else
    return d->state->xlatex || d->state->xlatey;
#endif
}

#ifndef QT_NO_TRANSFORMATIONS
/*!
    Scales the coordinate system by (\a{sx}, \a{sy}).

    \sa translate(), shear(), rotate(), resetXForm(), setMatrix(),
    xForm()
*/

void QPainter::scale(qreal sx, qreal sy)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::scale(), sx=%f, sy=%f\n", sx, sy);
#endif

    QMatrix m;
    m.scale(sx, sy);
    setMatrix(m, true);
}

/*!
    Shears the coordinate system by (\a{sh}, \a{sv}).

    \sa translate(), scale(), rotate(), resetXForm(), setMatrix(),
    xForm()
*/

void QPainter::shear(qreal sh, qreal sv)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::shear(), sh=%f, sv=%f\n", sh, sv);
#endif
    QMatrix m;
    m.shear(sv, sh);
    setMatrix(m, true);
}

/*!
    Rotates the coordinate system \a a degrees clockwise.

    \sa translate(), scale(), shear(), resetXForm(), setMatrix(),
    xForm()
*/

void QPainter::rotate(qreal a)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::rotate(), angle=%f\n", a);
#endif
    QMatrix m;
    m.rotate(a);
    setMatrix(m, true);
}
#endif


/*!
    \fn void QPainter::translate(qreal dx, qreal dy)

    \overload

    Translates the coordinate system by the vector (\a dx, \a dy).
*/

/*!
    \fn void QPainter::translate(const QPoint &offset)

    \overload

    Translates the coordinate system by the given \a offset.
*/

/*!
    Translates the coordinate system by \a offset. After this call,
    \a offset is added to points.

    For example, the following code draws the same point twice:
    \code
        void MyWidget::paintEvent()
        {
            QPainter paint(this);

            paint.drawPoint(0, 0);

            paint.translate(100.0, 40.0);
            paint.drawPoint(-100, -40);
        }
    \endcode

    \sa scale(), shear(), rotate(), resetXForm(), setMatrix(), xForm()
*/

void QPainter::translate(const QPointF &offset)
{
    qreal dx = offset.x();
    qreal dy = offset.y();
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::translate(), dx=%f, dy=%f\n", dx, dy);
#endif

#ifndef QT_NO_TRANSFORMATIONS
    QMatrix m;
    m.translate(dx, dy);
    setMatrix(m, true);
#else
    xlatex += (int)dx;
    xlatey += (int)dy;
    d->state->VxF = (bool)xlatex || xlatey;
#endif
}

/*!
    Sets the clip path for the painter to \a path, with the clip
    operation \a op.

    The clip path is specified in logical (painter) coordinates.

*/
void QPainter::setClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output) {
        QRectF b = path.boundingRect();
        printf("QPainter::setClipPath(), size=%d, op=%d, bounds=[%.2f,%.2f,%.2f,%.2f]\n",
               path.elementCount(), op, b.x(), b.y(), b.width(), b.height());
    }
#endif
    if (!isActive() || (!hasClipping() && path.isEmpty()))
        return;

    Q_D(QPainter);
//     if (d->state->clipInfo.size() == 0 && op != Qt::NoClip)
//         op = Qt::ReplaceClip;

    d->state->tmpClipPath = path;
    d->state->tmpClipOp = op;
    if (op == Qt::NoClip || op == Qt::ReplaceClip)
        d->state->clipInfo.clear();
    d->state->clipInfo << QPainterClipInfo(path, op, d->state->worldMatrix);
    d->engine->setDirty(QPaintEngine::DirtyClipPath);
    d->engine->updateState(d->state);
}

/*!
    Draws the outline (strokes) the path \a path with the pen specified
    by \a pen
*/
void QPainter::strokePath(const QPainterPath &path, const QPen &pen)
{
    Q_D(QPainter);
    QBrush oldBrush = d->state->brush;
    QPen oldPen = d->state->pen;

    d->state->pen = pen;
    d->state->brush = Qt::NoBrush;
    d->engine->setDirty(QPaintEngine::DirtyFlags(QPaintEngine::DirtyPen | QPaintEngine::DirtyBrush));

    drawPath(path);

    // Reset old state
    d->state->pen = oldPen;
    d->state->brush = oldBrush;
    d->engine->setDirty(QPaintEngine::DirtyFlags(QPaintEngine::DirtyPen | QPaintEngine::DirtyBrush));
}

/*!
    Fills the path \a path using the given \a brush. The outline
    is not drawn.
*/
void QPainter::fillPath(const QPainterPath &path, const QBrush &brush)
{
    Q_D(QPainter);
    QBrush oldBrush = d->state->brush;
    QPen oldPen = d->state->pen;

    d->state->pen = Qt::NoPen;
    d->state->brush = brush;
    d->engine->setDirty(QPaintEngine::DirtyFlags(QPaintEngine::DirtyPen | QPaintEngine::DirtyBrush));

    drawPath(path);

    // Reset old state
    d->state->pen = oldPen;
    d->state->brush = oldBrush;
    d->engine->setDirty(QPaintEngine::DirtyFlags(QPaintEngine::DirtyPen | QPaintEngine::DirtyBrush));
}
/*!
    Draws the painter path specified by \a path using the current pen
    for outline and the current brush for filling.
*/
void QPainter::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    QRectF pathBounds = path.boundingRect();
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPath(), size=%d, [%.2f,%.2f,%.2f,%.2f]\n",
               path.elementCount(),
               pathBounds.x(), pathBounds.y(), pathBounds.width(), pathBounds.height());
#endif

    if (!isActive())
	return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    if (d->engine->hasFeature(QPaintEngine::PainterPaths)) {
        if (d->engine->emulationSpecifier) {
            d->draw_helper(path);
            return;
        }
        d->engine->drawPath(path);
        return;
    }

    uint emulationSpecifier = d->engine->emulationSpecifier;
    QMatrix convertMatrix;
    if (emulationSpecifier & QPaintEngine::CoordTransform) {
        emulationSpecifier &= ~QPaintEngine::CoordTransform;
        convertMatrix = d->state->matrix;
    }

    save();

    if ((emulationSpecifier & QPaintEngine::LinearGradients)
        && d->engine->hasFeature(QPaintEngine::LinearGradientFillPolygon))
        emulationSpecifier &= ~QPaintEngine::LinearGradients;

    if ((emulationSpecifier & QPaintEngine::AlphaFill)
        && d->engine->hasFeature(QPaintEngine::AlphaFillPolygon))
        emulationSpecifier &= ~QPaintEngine::AlphaFill;

    // Fill the path...
    if (d->state->brush.style() != Qt::NoBrush) {
        QList<QPolygonF> fills;
        if (!(d->engine->emulationSpecifier & QPaintEngine::CoordTransform)
              && d->state->txop > QPainterPrivate::TxTranslate) {
            fills = path.toFillPolygons(d->state->matrix);
            d->updateInvMatrix();
            for (int i=0; i<fills.size(); ++i)
                fills[i] = fills.at(i) * d->invMatrix;
        } else {
            fills = path.toFillPolygons(convertMatrix);
        }
        save();
        QPoint tmpRedir = d->redirection_offset;
        d->redirection_offset = QPoint();
        setPen(Qt::NoPen);
	d->engine->updateState(d->state);
        for (int i=0; i<fills.size(); ++i) {
            if (emulationSpecifier) {
                resetMatrix();
                QPainterPath fillPath;
                fillPath.addPolygon(fills.at(i));
                fillPath.setFillRule(path.fillRule());
                d->draw_helper(fillPath, QPainterPrivate::StrokeAndFillDraw, emulationSpecifier);
            } else {
                d->engine->drawPolygon(fills.at(i).data(), fills.at(i).size(),
                                       QPaintEngine::PolygonDrawMode(path.fillRule()));
            }
        }
        d->redirection_offset = tmpRedir;
        restore();
    }

    // Draw the outline of the path...
    if (d->state->pen.style() != Qt::NoPen) {
        d->engine->updateState(d->state);
        // Only use helper if we have other than xform.
        if (d->engine->emulationSpecifier
            && (d->engine->emulationSpecifier != QPaintEngine::CoordTransform)) {
            d->draw_helper(path, QPainterPrivate::StrokeDraw, d->engine->emulationSpecifier);
        } else {
            QList<QPolygonF> polys;
            if (!(d->engine->emulationSpecifier & QPaintEngine::CoordTransform)
                  && d->state->txop > QPainterPrivate::TxTranslate) {
                polys = path.toSubpathPolygons(d->state->matrix);
                d->updateInvMatrix();
                for (int i=0; i<polys.size(); ++i)
                    polys[i] = polys.at(i) * d->invMatrix;
            } else {
                polys = path.toSubpathPolygons(convertMatrix);
            }
            for (int i=0; i<polys.size(); ++i) {
                d->engine->drawPolygon(polys.at(i).data(), polys.at(i).size(),
                                       QPaintEngine::PolylineMode);
            }
	}
    }

    restore();
}


/*!
    \fn void QPainter::drawLines(const QVector<QLineF> &lines)

    Draws the set of lines defined by the list \a lines using the
    current pen and brush.
*/

/*!
    \fn void QPainter::drawLine(int x1, int y1, int x2, int y2)
    \overload

    Draws a line from (\a x1, \a y1) to (\a x2, \a y2) and sets the
    current pen position to (\a x2, \a y2).
*/

/*!
    \fn void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
    \overload

    Draws a line from \a p1 to \a p2.
*/

/*!
    \fn void QPainter::drawLine(const QPointF &p1, const QPointF &p2)
    \overload

    Draws a line from \a p1 to \a p2.
*/

/*!
    Draws a line defined by \a l.

    \sa pen()
*/

void QPainter::drawLine(const QLineF &l)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawLine(), p1=(%.2f,%.2f), p2=(%.2f,%.2f)\n",
               l.startX(), l.startY(), l.endX(), l.endY());
#endif

    if (!isActive())
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    uint lineEmulation = d->engine->emulationSpecifier
                         & (QPaintEngine::CoordTransform
                            | QPaintEngine::PenWidthTransform
                            | QPaintEngine::AlphaStroke
                            | QPaintEngine::LineAntialiasing);
    QLineF line(l);
    if (lineEmulation) {
        if (lineEmulation == QPaintEngine::CoordTransform
            && d->state->txop == QPainterPrivate::TxTranslate) {
            line += QPointF(d->state->matrix.dx(), d->state->matrix.dy());
        } else {
            QPainterPath linePath(line.start());
            linePath.lineTo(line.end());
            d->draw_helper(linePath, QPainterPrivate::StrokeDraw);
            return;
        }
    }
    d->engine->drawLine(line);
}

/*!
    \fn void QPainter::drawRect(int x, int y, int w, int h)

    \overload

    Draws a rectangle with upper left corner at (\a{x}, \a{y}) and with
    width \a w and height \a h.
*/

/*!
    \fn void QPainter::drawRect(const QRect &rect)

    \overload

    Draws the rectangle \a rect with the current pen and brush.
*/

/*!
  Draws the rectangle \a r with the current pen and brush.

  A filled rectangle has a size of r.size(). A stroked rectangle
  has a size of r.size() plus the pen width.
*/
void QPainter::drawRect(const QRectF &r)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawRect(), [%.2f,%.2f,%.2f,%.2f]\n", r.x(), r.y(), r.width(), r.height());
#endif
    QRectF rect = r.normalize();

    if (!isActive() || rect.isEmpty())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    uint emulationSpecifier = d->engine->emulationSpecifier;

    if ((emulationSpecifier & QPaintEngine::AlphaFill)
        && d->engine->hasFeature(QPaintEngine::AlphaFillPolygon))
        emulationSpecifier &= ~QPaintEngine::AlphaFill;

    if (emulationSpecifier) {
        if (emulationSpecifier == QPaintEngine::CoordTransform
            && d->state->txop == QPainterPrivate::TxTranslate) {
            rect.translate(QPointF(d->state->matrix.dx(), d->state->matrix.dy()));
        } else {
            QPainterPath rectPath;
            rectPath.addRect(rect);
            d->draw_helper(rectPath, QPainterPrivate::StrokeAndFillDraw, emulationSpecifier);
            return;
        }
    }
    d->engine->drawRect(rect);
}

/*!
    \fn void QPainter::drawRects(const QVector<QRectF> &rectangles)

    Draws the rectangles specified in \a rectangles using the
    current pen and brush.
*/


/*!
    Draws the first \a rectCount rectangles in the array \a rects
    using the current pen and brush.

    \sa drawRect()
*/
void QPainter::drawRects(const QRectF *rects, int rectCount)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawRects(), count=%d\n", rectCount);
#endif
    if (!isActive() || rectCount <= 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    if (d->state->txop == QPainterPrivate::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        // ### Don't alloc for every conversion.
        for (int i=0; i<rectCount; ++i) {
            d->engine->drawRect(QRectF(rects[i].x() + d->state->matrix.dx(),
                                       rects[i].y() + d->state->matrix.dy(),
                                       rects[i].width(),
                                       rects[i].height()));
        }
    } else if (d->engine->emulationSpecifier) {
        for (int i=0; i<rectCount; ++i)
            drawRect(rects[i]);
    } else {
        d->engine->drawRects(rects, rectCount);
    }
}


/*! \fn void QPainter::drawPoint(const QPoint &p)
    Draws a single point at position \a p using the current pen's color.

    \sa QPen
*/

/*!
    Draws a single point at position \a p using the current pen's color.

    \sa QPen
*/
void QPainter::drawPoint(const QPointF &p)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPoint(), p=(%.2f,%.2f)\n", p.x(), p.y());
#endif
    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    QPointF pt(p);
    if (d->engine->emulationSpecifier) {
        if (d->engine->emulationSpecifier == QPaintEngine::CoordTransform
            && d->state->txop == QPainterPrivate::TxTranslate) {
            pt += QPointF(d->state->matrix.dx(), d->state->matrix.dy());
        } else {
            save();
            setBrush(d->state->pen.color());
            QPainterPath pointPath;
            pointPath.addRect(pt.x(), pt.y(), 1, 1);
            d->draw_helper(pointPath, QPainterPrivate::FillDraw);
            restore();
            return;
        }
    }
    d->engine->drawPoint(pt);
}

/*! \fn void QPainter::drawPoint(int x, int y)

    \overload

    Draws a single point at position (\a x, \a y).
*/

/*!
    \fn void QPainter::drawPoints(const QPolygon &points, int index,
    int npoints)

    \overload

    \compat

    Draws \a npoints points in the list \a points starting on index
    using the current pen.

*/

/*! \fn void QPainter::drawPoints(const QPolygonF &points)

    \overload

    Draws the points in the list \a points.
*/

/*!
    Draws the first \a pointCount points in the array \a points using
    the current pen's color.

    \sa QPen
*/
void QPainter::drawPoints(const QPointF *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPoints(), count=%d\n", pointCount);
#endif
    if (!isActive() || pointCount <= 0)
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    if (d->engine->emulationSpecifier) {
        if (d->engine->emulationSpecifier == QPaintEngine::CoordTransform
            && d->state->txop == QPainterPrivate::TxTranslate) {
            // ### use drawPoints function
            for (int i=0; i<pointCount; ++i) {
                d->engine->drawPoint(QPointF(points[i].x() + d->state->matrix.dx(),
                                             points[i].y() + d->state->matrix.dy()));
            }
        } else {
            QPainterPath path;
            for (int i=0; i<pointCount; ++i)
                path.addRect(points[i].x(), points[i].y(), 1, 1);
            d->draw_helper(path);
        }
    } else {
        d->engine->drawPoints(points, pointCount);
    }
}

/*!
    \overload

    Draws the first \a pointCount points in the array \a points using
    the current pen's color.

    \sa QPen
*/

void QPainter::drawPoints(const QPoint *points, int pointCount)
{
    // ###
    QVector<QPointF> pts = qt_convert_points(points, pointCount, QPointF());
    drawPoints(pts.data(), pts.size());
}

/*!
    Sets the background mode of the painter to \a mode, which must be
    either \c Qt::TransparentMode (the default) or \c Qt::OpaqueMode.

    Transparent mode draws stippled lines and text without setting the
    background pixels. Opaque mode fills these space with the current
    background color.

    Note that in order to draw a bitmap or pixmap transparently, you
    must use QPixmap::setMask().

    \sa backgroundMode(), setBackground()
*/

void QPainter::setBackgroundMode(Qt::BGMode mode)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setBackgroundMode(), mode=%d\n", mode);
#endif

    if (mode != Qt::TransparentMode && mode != Qt::OpaqueMode) {
        qWarning("QPainter::setBackgroundMode: Invalid mode");
        return;
    }
    Q_D(QPainter);
    d->state->bgMode = mode;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyBackground);
}

/*!
    Returns the current background mode.

    \sa setBackgroundMode() Qt::BGMode
*/
Qt::BGMode QPainter::backgroundMode() const
{
    Q_D(const QPainter);
    return d->state->bgMode;
}


/*!
    \overload

    Sets the painter's pen to have style \c Qt::SolidLine, width 0 and the
    specified \a color.

    \sa pen(), QPen
*/

void QPainter::setPen(const QColor &color)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setPen(), color=%04x\n", color.rgb());
#endif
    Q_D(QPainter);
    QPen newPen = QPen(color.isValid() ? color : QColor(Qt::black), 0, Qt::SolidLine);
    d->state->pen = newPen;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyPen);
}

/*!
    Sets a new painter pen.

    The \a pen defines how to draw lines and outlines, and it also
    defines the text color.

    \sa pen()
*/

void QPainter::setPen(const QPen &pen)
{

#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setPen(), color=%04x, (brushStyle=%d) style=%d, cap=%d, join=%d\n",
           pen.color().rgb(), pen.brush().style(), pen.style(), pen.capStyle(), pen.joinStyle());
#endif

    Q_D(QPainter);
    d->state->pen = pen;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyPen);
}

/*!
    \overload

    Sets the painter's pen to have style \a style, width 0 and black
    color.

    \sa pen(), QPen
*/

void QPainter::setPen(Qt::PenStyle style)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setPen(), style=%d\n", style);
#endif

    Q_D(QPainter);
    d->state->pen = QPen(Qt::black, 0, style);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyPen);
}

/*!
    Returns the painter's current pen.

    \sa setPen()
*/

const QPen &QPainter::pen() const
{
    Q_D(const QPainter);
    return d->state->pen;
}


/*!
    \overload

    Sets the painter's brush to \a brush.

    The \a brush defines how shapes are filled.

    \sa brush()
*/

void QPainter::setBrush(const QBrush &brush)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setBrush(), color=%04x, style=%d\n", brush.color().rgb(), brush.style());
#endif
    Q_D(QPainter);
    d->state->brush = brush;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyBrush);
}


/*!
    Sets the painter's brush to black color and the specified \a
    style.

    \sa brush(), QBrush
*/

void QPainter::setBrush(Qt::BrushStyle style)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setBrush(), style=%d\n", style);
#endif

    Q_D(QPainter);
    d->state->brush = QBrush(Qt::black, style);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyBrush);
}

/*!
    Returns the painter's current brush.

    \sa QPainter::setBrush()
*/

const QBrush &QPainter::brush() const
{
    Q_D(const QPainter);

    if (d->state->brush.style() == Qt::LinearGradientPattern) {
        if (!d->txinv) {
            QPainter *that = (QPainter*)this;        // mutable
            that->d_ptr->updateInvMatrix();
        }
        static QBrush tmp_brush = QBrush(d->state->brush.gradientStart() * d->invMatrix,
                                         d->state->brush.color(),
                                         d->state->brush.gradientStop() * d->invMatrix,
                                         d->state->brush.gradientColor());
        return tmp_brush;
    }
    return d->state->brush;
}

/*!
    Sets the background brush of the painter to \a bg.

    The background brush is the brush that is filled in when drawing
    opaque text, stippled lines and bitmaps. The background brush has
    no effect in transparent background mode (which is the default).

    \sa background() setBackgroundMode() Qt::BGMode
*/

void QPainter::setBackground(const QBrush &bg)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setBackground(), color=%04x, style=%d\n", bg.color().rgb(), bg.style());
#endif

    Q_D(QPainter);
    d->state->bgBrush = bg;
    if (d->engine && d->engine->isActive())
        d->engine->setDirty(QPaintEngine::DirtyBackground);
}

/*!
    Sets the painter's font to \a font.

    This font is used by subsequent drawText() functions. The text
    color is the same as the pen color.

    \sa font(), drawText()
*/

void QPainter::setFont(const QFont &font)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setFont(), family=%s, pointSize=%d\n", font.family().toLatin1().constData(), font.pointSize());
#endif

    Q_D(QPainter);
    d->state->font = QFont(font.resolve(d->state->deviceFont), d->device);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyFont);
}

/*!
    Returns the currently set painter font.

    \sa setFont(), QFont
*/

const QFont &QPainter::font() const
{
    Q_D(const QPainter);
    if (d->engine)
        d->engine->updateState(d->state);
    return d->state->pfont ? *d->state->pfont : d->state->font;
}

/*!
    \fn QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)

    \overload

    Draws the rectangle \a x, \a y, \a w, \a h with rounded corners.
*/

/*!
    \fn void QPainter::drawRoundRect(const QRect &r, int xRnd = 25, int yRnd = 25)

    \overload

    Draws the rectangle \a r with rounded corners.
*/


/*!
    Draws a rectangle \a r with rounded corners.

    The \a xRnd and \a yRnd arguments specify how rounded the corners
    should be. 0 is angled corners, 99 is maximum roundedness.

    A filled rectangle has a size of r.size(). A stroked rectangle
    has a size of r.size() plus the pen width.

    \sa drawRect(), QPen
*/
void QPainter::drawRoundRect(const QRectF &r, int xRnd, int yRnd)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawRoundRectangle(), [%.2f,%.2f,%.2f,%.2f]\n", r.x(), r.y(), r.width(), r.height());
#endif

    if (!isActive())
        return;

    if(xRnd >= 100)                          // fix ranges
        xRnd = 99;
    if(yRnd >= 100)
        yRnd = 99;
    if(xRnd <= 0 || yRnd <= 0) {             // draw normal rectangle
        drawRect(r);
        return;
    }

    QRectF rect = r.normalize();

    QPainterPath path;

    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = rect.width();
    qreal h = rect.height();
    qreal rxx = w*xRnd/200;
    qreal ryy = h*yRnd/200;
    // were there overflows?
    if (rxx < 0)
        rxx = w/200*xRnd;
    if (ryy < 0)
        ryy = h/200*yRnd;
    qreal rxx2 = 2*rxx;
    qreal ryy2 = 2*ryy;

    QPointF startPoint;
    qt_find_ellipse_coords(QRectF(x, y, rxx2, ryy2), 90, 90, &startPoint, 0);
    path.moveTo(startPoint);
    path.arcTo(x, y, rxx2, ryy2, 90, 90);
    path.arcTo(x, y+h-ryy2, rxx2, ryy2, 2*90, 90);
    path.arcTo(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*90, 90);
    path.arcTo(x+w-rxx2, y, rxx2, ryy2, 0, 90);
    path.closeSubpath();

    drawPath(path);
}

/*!
    \fn QPainter::drawEllipse(const QRect &r)

    \overload

    Draws an ellipse that fits inside the rectangle \a r.
*/

/*!
    \fn QPainter::drawEllipse(int x, int y, int w, int h)

    \overload

    Draws an ellipse that fits inside the rectangle (\a{x}, \a{y}, \a{w},
    \a{h}).
*/

/*!
    Draws the ellipse that fits inside the rectangle \a r.

    A filled ellipse has a size of r.size(). An stroked ellipse
    has a size of r.size() plus the pen width.
*/
void QPainter::drawEllipse(const QRectF &r)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawEllipse(), [%.2f,%.2f,%.2f,%.2f]\n", r.x(), r.y(), r.width(), r.height());
#endif

    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    QRectF rect(r.normalize());

    if (d->engine->emulationSpecifier) {
        if (d->engine->emulationSpecifier == QPaintEngine::CoordTransform
            && d->state->txop == QPainterPrivate::TxTranslate) {
            rect.translate(QPointF(d->state->matrix.dx(), d->state->matrix.dy()));
        } else {
            QPainterPath path;
            path.addEllipse(rect);
            d->draw_helper(path, QPainterPrivate::StrokeAndFillDraw);
            return;
        }
    }

    d->engine->drawEllipse(rect);
}


/*! \fn void QPainter::drawArc(const QRect &r, int startAngle,
                               int spanAngle)

    \overload

    Draws the arc that fits inside the rectangle \a r, with the given
    \a startAngle and \a spanAngle.
*/

/*!
    \fn void QPainter::drawArc(int x, int y, int w, int h,
                               int startAngle, int spanAngle)

    \overload

    Draws the arc that fits inside the rectangle (\a{x}, \a{y}, \a{w},
    \a{h}), with the given \a startAngle and \a spanAngle.
*/

/*!
    Draws an arc defined by the rectangle \a r, the start angle \a a
    and the arc length \a alen.

    The angles \a a and \a alen are 1/16th of a degree, i.e. a full
    circle equals 5760 (16*360). Positive values of \a a and \a alen
    mean counter-clockwise while negative values mean the clockwise
    direction. Zero degrees is at the 3 o'clock position.

    Example:
    \code
        QPainter p(myWidget);
        p.drawArc(QRect(10,10, 70,100), 100*16, 160*16); // draws a "(" arc
    \endcode

    \sa drawPie(), drawChord()
*/

void QPainter::drawArc(const QRectF &r, int a, int alen)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawArc(), [%.2f,%.2f,%.2f,%.2f], angle=%d, sweep=%d\n",
           r.x(), r.y(), r.width(), r.height(), a/16, alen/16);
#endif

    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    QRectF rect = r.normalize();

    QPointF startPoint;
    qt_find_ellipse_coords(r, a/16.0, alen/16.0, &startPoint, 0);

    QPainterPath path;
    path.moveTo(startPoint);
    path.arcTo(rect, a/16.0, alen/16.0);
    strokePath(path, d->state->pen);
}


/*!
    \fn void QPainter::drawPie(const QRect &rect, int startAngle, int spanAngle)

    \overload

    Draws a pie segment that fits inside the rectangle \a rect with
    the given \a startAngle and \a spanAngle.
*/

/*!
    \fn void QPainter::drawPie(int x, int y, int w, int h, int
    startAngle, int spanAngle)

    \overload

    Draws a pie segment that fits inside the rectangle (\a{x}, \a{y},
    \a{w}, \a{h}) with the given \a startAngle and \a spanAngle.
*/

/*!
    Draws a pie defined by the rectangle \a r, the start angle \a a
    and the arc length \a alen.

    The pie is filled with the current brush().

    The angles \a a and \a alen are 1/16th of a degree, i.e. a full
    circle equals 5760 (16*360). Positive values of \a a and \a alen
    mean counter-clockwise while negative values mean the clockwise
    direction. Zero degrees is at the 3 o'clock position.

    \sa drawArc(), drawChord()
*/
void QPainter::drawPie(const QRectF &r, int a, int alen)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPie(), [%.2f,%.2f,%.2f,%.2f], angle=%d, sweep=%d\n",
           r.x(), r.y(), r.width(), r.height(), a/16, alen/16);
#endif

    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    if (a > (360*16)) {
        a = a % (360*16);
    } else if (a < 0) {
        a = a % (360*16);
        if (a < 0) a += (360*16);
    }

    QRectF rect = r.normalize();

    QPainterPath path;
    path.moveTo(rect.center());
    path.arcTo(rect.x(), rect.y(), rect.width(), rect.height(), a/16.0, alen/16.0);
    path.closeSubpath();
    drawPath(path);

}

/*!
    \fn void QPainter::drawChord(const QRect &r, int startAngle, int spanAngle)

    \overload

    Draws a chord that fits inside the rectangle \a r with the given
    \a startAngle and \a spanAngle.
*/

/*!
    \fn void QPainter::drawChord(int x, int y, int w, int h, int
    startAngle, int spanAngle)

    \overload

    Draws a chord that fits inside the rectangle (\a{x}, \a{y}, \a{w},
    \a{h}) with the given \a startAngle and \a spanAngle.
*/


/*!
    Draws a chord defined by the rectangle \a r, the start angle \a a
    and the arc length \a alen.

    The chord is filled with the current brush().

    The angles \a a and \a alen are 1/16th of a degree, i.e. a full
    circle equals 5760 (16*360). Positive values of \a a and \a alen
    mean counter-clockwise while negative values mean the clockwise
    direction. Zero degrees is at the 3 o'clock position.

    \sa drawArc(), drawPie()
*/

void QPainter::drawChord(const QRectF &r, int a, int alen)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawChord(), [%.2f,%.2f,%.2f,%.2f], angle=%d, sweep=%d\n",
           r.x(), r.y(), r.width(), r.height(), a/16, alen/16);
#endif

    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    QRectF rect = r.normalize();

    QPointF startPoint;
    qt_find_ellipse_coords(r, a/16.0, alen/16.0, &startPoint, 0);

    QPainterPath path;
    path.moveTo(startPoint);
    path.arcTo(rect.x(), rect.y(), rect.width(), rect.height(), a/16.0, alen/16.0);
    path.closeSubpath();
    drawPath(path);
}

#ifdef QT3_SUPPORT
/*!
    Draws \a nlines separate lines from points defined in \a a,
    starting at \a{a}\e{[index]} (\a index defaults to 0). If \a nlines is
    -1 (the default) all points until the end of the array are used
    (i.e. (a.size()-index)/2 lines are drawn).

    Draws the 1st line from \a{a}\e{[index]} to \a{a}\e{[index + 1]}. Draws the
    2nd line from \a{a}\e{[index + 2]} to \a{a}\e{[index + 3]} etc.

    \sa drawPolyline(), drawPolygon(), QPen
*/

void QPainter::drawLineSegments(const QPolygon &a, int index, int nlines)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawLineSegments(), count=%d\n", a.size()/2);
#endif

    if (!isActive())
        return;

    if (nlines < 0)
        nlines = a.size()/2 - index/2;
    if (index + nlines*2 > (int)a.size())
        nlines = (a.size() - index)/2;
    if (nlines < 1 || index < 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    QVector<QLineF> lines;
    if (d->engine->emulationSpecifier) {
        if (d->engine->emulationSpecifier == QPaintEngine::CoordTransform
            && d->state->txop == QPainterPrivate::TxTranslate) {
            QPointF offset(d->state->matrix.dx(), d->state->matrix.dy());
            for (int i=index; i<index + nlines*2; i+=2)
                lines << QLineF(a.at(i) + offset, a.at(i+1) + offset);
        } else {
            QPainterPath linesPath;
            for (int i=index; i<index + nlines*2; i+=2) {
                linesPath.moveTo(a.at(i));
                linesPath.lineTo(a.at(i+1));
            }
            d->draw_helper(linesPath, QPainterPrivate::StrokeDraw);
            return;
        }
    } else {
        for (int i=index; i<index + nlines*2; i+=2)
            lines << QLineF(a.at(i), a.at(i+1));
    }

    d->engine->drawLines(lines.data(), lines.size());
}
#endif // QT3_SUPPORT
/*!
    Draws the first \a lineCount lines in the array \a lines
    using the current pen.
*/
void QPainter::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QPainter);
    d->engine->updateState(d->state);

    // Dummy implementation for now.
    for (int i=0; i<lineCount; ++i)
        drawLine(lines[i]);
}


/*!
    \fn void QPainter::drawLines(const QVector<QPointF> &pointPairs)

    \overload

    Draws a line for each pair of points in the vector \a pointPairs using
    the current pen.

    If there is an odd number of points in the array, the last point
    will be ignored.
*/

/*!
    \overload

    Draws the first \a lineCount lines in the array \a pointPairs using
    the current pen.

    The lines are specified as pairs of points so the number of entries
    in \a pointPairs must be at least \a lineCount * 2

    \sa drawLines()
*/
void QPainter::drawLines(const QPointF *pointPairs, int lineCount)
{
    Q_ASSERT_X(pointPairs, "QPainter::drawLines", "pointPairs array cannot be 0");
    // This will go horribly wrong if the layout of QLineF changes!
    drawLines((QLineF*)pointPairs, lineCount);
}

/*!
    \overload

    Draws the first \a lineCount lines in the array \a pointPairs using
    the current pen.

    The lines are specified as pairs of points so the number of entries
    in \a pointPairs must be at least \a lineCount * 2

    \sa drawLines()
*/
void QPainter::drawLines(const QPoint *pointPairs, int lineCount)
{
    Q_ASSERT_X(pointPairs, "QPainter::drawLines", "pointPairs array cannot be 0");
    QVector<QPointF> pts = qt_convert_points(pointPairs, lineCount * 2, QPointF());
    // This will go horribly wrong if the layout of QLineF changes!
    drawLines((QLineF*)pointPairs, pts.size() / 2);
}


/*!
    \fn void QPainter::drawPolyline(const QPolygon &pa, int index, int
    npoints)

    \overload

    \compat

    Draws the polyline defined by the \a npoints points in \a pa
    starting at \a index. (\a index defaults to 0.)

*/

/*!
    \fn void QPainter::drawPolyline(const QPolygon &pa)

    \overload

    Draws the polyline defined by \a pa using the current pen.
*/

/*!
    \fn void QPainter::drawPolyline(const QPolygonF &pa)

    \overload

    Draws the polyline defined by \a pa using the current pen.
*/

/*!
    Draws the polyline defined by the first \a pointCount points in \a
    points using the current pen.

    \sa drawLines(), drawPolygon(), QPen
*/

void QPainter::drawPolyline(const QPointF *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPolyline(), count=%d\n", pointCount);
#endif

    if (!isActive() || pointCount <= 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    uint lineEmulation = d->engine->emulationSpecifier
                         & (QPaintEngine::CoordTransform
                            | QPaintEngine::PenWidthTransform
                            | QPaintEngine::AlphaStroke
                            | QPaintEngine::LineAntialiasing);

    if (lineEmulation) {
        // ###
//         if (lineEmulation == QPaintEngine::CoordTransform
//             && d->state->txop == QPainterPrivate::TxTranslate) {
//         } else {
        QPainterPath polylinePath(points[0]);
        for (int i=1; i<pointCount; ++i)
            polylinePath.lineTo(points[i]);
        d->draw_helper(polylinePath, QPainterPrivate::StrokeDraw);
//         }
    } else {
        d->engine->drawPolygon(points, pointCount, QPaintEngine::PolylineMode);
    }
}

/*! \overload

    Draws the polyline defined by the first \a pointCount points in
    the array \a points using the current pen.
 */
void QPainter::drawPolyline(const QPoint *points, int pointCount)
{
    // ### don't realloc
    QVector<QPointF> pts = qt_convert_points(points, pointCount, QPointF());
    drawPolyline(pts.data(), pts.size());
}


/*! \fn void QPainter::drawPolygon(const QPolygon &pa, bool winding,
                                   int index = 0, int npoints = -1)

    \compat
    \overload

    Draws the polygon defined by the points in the point array \a pa.
*/

/*! \fn void QPainter::drawPolygon(const QPolygon &pa, Qt::FillRule fillRule)

    \overload

    Draws the polygon defined by the points in \a pa using the fill
    rule \a fillRule.
*/

/*! \fn void QPainter::drawPolygon(const QPolygonF &pa, Qt::FillRule fillRule)

    \overload

    Draws the polygon defined by the points in \a pa using the fill
    rule \a fillRule.
*/

/*! \fn void QPainter::drawPolygon(const QPolygonF &polygon, bool winding, int index = 0,
                                   int npoints = -1)
    \compat
    \overload
*/

/*!
    Draws the polygon defined by the first \a pointCount points in the
    array \a points using the current pen and brush.

    The first point is implicitly connected to the last point.

    The polygon is filled with the current brush(). If \a fillRule is
    \c Qt::WindingFill, the polygon is filled using the winding fill algorithm.
    If \a fillRule is \c Qt::OddEvenFill, the polygon is filled using the
    odd-even fill algorithm. See \l{Qt::FillRule} for a more detailed
    description of these fill rules.

    \sa drawLines() drawPolyline() QPen
*/

void QPainter::drawPolygon(const QPointF *points, int pointCount, Qt::FillRule fillRule)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPolygon(), count=%d\n", pointCount);
#endif

    if (!isActive() || pointCount <= 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    uint emulationSpecifier = d->engine->emulationSpecifier;
    if ((emulationSpecifier & QPaintEngine::LinearGradients)
        && d->engine->hasFeature(QPaintEngine::LinearGradientFillPolygon))
        emulationSpecifier &= ~QPaintEngine::LinearGradients;

    if ((emulationSpecifier & QPaintEngine::AlphaFill)
        && d->engine->hasFeature(QPaintEngine::AlphaFillPolygon))
        emulationSpecifier &= ~QPaintEngine::AlphaFill;

    if (emulationSpecifier) {
        // ###
//         if (emulationSpecifier == QPaintEngine::CoordTransform
//             && d->state->txop == QPainterPrivate::TxTranslate) {
//         } else {
        QPainterPath polygonPath(points[0]);
        for (int i=1; i<pointCount; ++i)
            polygonPath.lineTo(points[i]);
        polygonPath.closeSubpath();
        d->draw_helper(polygonPath);
        return;
    }

    d->engine->drawPolygon(points, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
}

/*! \overload

    Draws the polygon defined by the first \a pointCount points in the
    array \a points.
*/
void QPainter::drawPolygon(const QPoint *points, int pointCount, Qt::FillRule fillRule)
{
    // ### don't realloc
    QVector<QPointF> pts = qt_convert_points(points, pointCount, QPointF());
    drawPolygon(pts.data(), pts.size(), fillRule);
}


/*!
    \fn void QPainter::drawConvexPolygon(const QPolygonF &polygon)

    \overload

    Draws the convex polygon defined by \a polygon using the current
    pen and brush.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygon &polygon)

    \overload

    Draws the convex polygon defined by \a polygon using the current
    pen and brush.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygonF &polygon, int
    index, int npoints)

    \compat

    \overload

    Draws the convex polygon defined by \a polygon using the current
    pen and brush.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygon &polygon, int
    index, int npoints)

    \compat

    \overload

    Draws the convex polygon defined by \a polygon using the current
    pen and brush.
*/

/*!
    Draws the convex polygon defined by the first \a pointCount points
    in the array \a points using the current pen and brush.

    If the supplied polygon is not convex, the results are undefined.

    On some platforms (e.g. X11), drawing convex polygons can be
    faster than drawPolygon().

    \sa drawPolygon
*/

void QPainter::drawConvexPolygon(const QPoint *points, int pointCount)
{
    // ### Fix when QPainter::drawPolygon(QPolygon, PolyDrawMode) is in place
    drawPolygon(points, pointCount, Qt::WindingFill);
}

/*!
    Draws the convex polygon defined by the first \a pointCount points
    in the array \a points using the current pen and brush.

    If the supplied polygon is not convex, the results are undefined.

    On some platforms (e.g. X11), drawing convex polygons can be
    faster than drawPolygon().

    \sa drawPolygon
*/
void QPainter::drawConvexPolygon(const QPointF *points, int pointCount)
{
    // ### Fix when QPainter::drawPolygon(QPolygon, PolyDrawMode) is in place
    drawPolygon(points, pointCount, Qt::WindingFill);
}


/*!
    \fn void QPainter::drawPixmap(const QRect &targetRect, const QPixmap &pixmap,
                                  const QRect &sourceRect, Qt::PixmapDrawingMode mode)
    \overload

    Draws the rectangular portion \a sourceRect of the pixmap \a pixmap
    in the rectangle \a targetRect.
*/

/*!
    \fn void QPainter::drawPixmap(const QPointF &p, const QPixmap &pixmap,
                                  const QRectF &sourceRect, Qt::PixmapDrawingMode mode)
    \overload

    Draws the rectangular portion \a sourceRect of the pixmap \a
    pixmap at the point \a p.
*/

/*!
    \fn void QPainter::drawPixmap(const QPointF &p, const QPixmap &pixmap,
                                  Qt::PixmapDrawingMode mode)
    \overload

    Draws the \a pixmap at the point \a p.
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap, Qt::PixmapDrawingMode mode)

    \overload

    Draws the given \a pixmap at position (\a{x}, \a{y}) using the
    specified drawing \a mode.
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, int width, int height,
    const QPixmap &pixmap, Qt::PixmapDrawingMode mode)

    \overload

    Draws the \a pixmap in the rectangle at position (\a{x}, \a{y})
    and of the given \a width and \a height.

*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                  int sx, int sy, int sw, int sh, Qt::PixmapDrawingMode mode)

    \overload

    Draws the rectangular portion with the origin (\a{sx}, \a{sy}),
    width \a sw and height \a sh, of the pixmap \a pm, at the point
    (\a{x}, \a{y}), with a width of \a w and a height of \a h. If \a
    mode is QPainter::CopyPixmap \a pm will not be masked to
    QPixmap::mask()
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh,
                                  Qt::PixmapDrawingMode mode)

    \overload

    Draws a pixmap at (\a{x}, \a{y}) by copying a part of \a pixmap into
    the paint device.

    (\a{x}, \a{y}) specifies the top-left point in the paint device that is
    to be drawn onto. (\a{sx}, \a{sy}) specifies the top-left point in \a
    pixmap that is to be drawn. The default is (0, 0).

    (\a{sw}, \a{sh}) specifies the size of the pixmap that is to be drawn.
    The default, (-1, -1), means all the way to the bottom-right of
    the pixmap.

    \sa QPixmap::setMask()
*/

/*!
    \fn void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr,
                                  Qt::PixmapDrawingMode mode)
    \overload

    Draws the rectangle \a sr of pixmap \a pm with its origin at point
    \a p.
*/

/*!
    \fn void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, Qt::PixmapDrawingMode mode)
    \overload

    Draws the pixmap \a pm with its origin at point \a p.
*/

/*!
    \fn void QPainter::drawPixmap(const QRect &r, const QPixmap &pm, Qt::PixmapDrawingMode mode)
    \overload

    Draws the pixmap \a pm into the rectangle \a r.
*/

/*!
    Draws the rectanglular portion \a sr, of pixmap \a pm, into rectangle
    \a r in the paint device. The blend mode \a mode decides how the
    pixmap is merged with the target paint device.

    \sa Qt::PixmapDrawingMode
*/
void QPainter::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                          Qt::PixmapDrawingMode mode)
{
#if defined QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawPixmap(), target=[%.2f,%.2f,%.2f,%.2f], pix=[%d,%d], source=[%.2f,%.2f,%.2f,%.2f], mode=%d\n",
           r.x(), r.y(), r.width(), r.height(),
           pm.width(), pm.height(),
           sr.x(), sr.y(), sr.width(), sr.height(),
           mode);
#endif

    Q_D(QPainter);
    if (!isActive() || pm.isNull() || (mode == Qt::CopyPixmap && d->device->devType() != QInternal::Pixmap))
        return;
    d->engine->updateState(d->state);

    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();
    qreal sx = sr.x();
    qreal sy = sr.y();
    qreal sw = sr.width();
    qreal sh = sr.height();

    // Sanity-check clipping
    if (sw <= 0 || sw + sx > pm.width())
        sw = pm.width() - sx;

    if (sh <= 0 || sh + sy > pm.height())
        sh = pm.height() - sy;

    if (sx < 0) {
        x -= sx;
        sw += sx;
        sx = 0;
    }

    if (sy < 0) {
        y -= sy;
        sh += sy;
        sy = 0;
    }

    if (w < 0)
        w = sw;
    if (h < 0)
        h = sh;

    if (sw <= 0 || sh <= 0)
        return;

    if ((d->state->txop > QPainterPrivate::TxTranslate
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) ||
        ((w != sw || h != sh) && !d->engine->hasFeature(QPaintEngine::PixmapScale))) {
        QPixmap source;
        if(sx != 0 || sy != 0 || sw != pm.width() || sh != pm.height()) {
            source = QPixmap(qRound(sw), qRound(sh), pm.depth());
            QPainter p(&source);
            p.drawPixmap(QRectF(0, 0, sw, sh), pm, QRectF(sx, sy, sw, sh), Qt::CopyPixmap);
            p.end();
#if defined(Q_WS_QWS)
            source.data->hasAlpha = pm.data->hasAlpha;
#endif
        } else {
            source = pm;
        }

        QMatrix mat(d->state->matrix);
        qreal scalex = w / sw;
        qreal scaley = h / sh;
        mat = QMatrix(scalex, 0, 0, scaley, 0, 0) * mat;
        mat = QPixmap::trueMatrix(mat, qRound(sw), qRound(sh));
        QPixmap pmx = source.transform(mat);
        if (pmx.isNull())                        // xformed into nothing
            return;
        if (pmx.depth() == 1 && !pmx.mask())
            pmx.setMask(*(static_cast<QBitmap *>(&pmx)));

        if (!pmx.mask() && d->state->txop == QPainterPrivate::TxRotShear) {
            QBitmap bm_clip(qRound(sw), qRound(sh), 1);        // make full mask, xform it
            bm_clip.fill(Qt::color1);
            pmx.setMask(bm_clip.transform(mat));
        }
        d->state->matrix.map(x, y, &x, &y);        // compute position of pixmap
        qreal dx, dy;
        mat.map(0, 0, &dx, &dy);
        d->engine->drawPixmap(QRectF(x-dx, y-dy, pmx.width(), pmx.height()), pmx,
                              QRectF(0, 0, pmx.width(), pmx.height()), mode);
    } else {
        if (!d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
            x += qRound(d->state->matrix.dx());
            y += qRound(d->state->matrix.dy());
        }
        d->engine->drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh), mode);
    }

    // If we have CopyPixmap we copy the mask (only the mask, not the
    // alpha channel, which is copied elsewhere) from the source to
    // the target device if it is a pixmap
    if (d->device->devType() == QInternal::Pixmap && pm.mask() && !pm.hasAlphaChannel()) {
        if (mode == Qt::CopyPixmap) {
            QPixmap *p = static_cast<QPixmap *>(d->device);
            QBitmap bitmap(p->width(), p->height());
            bitmap.fill(Qt::color0);
            QPainter pt(&bitmap);
            pt.setPen(Qt::color1);
            const QBitmap *mask = pm.mask();
            //Q_ASSERT(!mask->mask());
            pt.drawPixmap(r, *mask, sr);
            pt.end();
            p->setMask(bitmap);
        } else if (mode == Qt::ComposePixmap) {
            QPixmap *p = static_cast<QPixmap *>(d->device);
            QBitmap bitmap;
            if (p->mask()) {
                bitmap = *p->mask();
                QPainter pt(&bitmap);
                //Q_ASSERT(!pm.mask()->mask());
                pt.drawPixmap(qRound(x), qRound(y), qRound(w), qRound(h),
                              *pm.mask(), qRound(sx), qRound(sy), qRound(sw),
                              qRound(sh));
                pt.end();
                p->setMask(bitmap);
            }
        }
    }
}

/*!
    Draws the rectanglular portion \a sourceRect, of image \a image, into rectangle
    \a targetRect in the paint device.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a flags to
    specify how you would prefer this to happen.

    \sa Qt::ImageConversionFlags
*/
void QPainter::drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
                         Qt::ImageConversionFlags flags)
{
    if (!isActive() || image.isNull())
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    qreal x = targetRect.x();
    qreal y = targetRect.y();
    qreal w = targetRect.width();
    qreal h = targetRect.height();
    qreal sx = sourceRect.x();
    qreal sy = sourceRect.y();
    qreal sw = sourceRect.width();
    qreal sh = sourceRect.height();

    // Sanity-check clipping
    if (sw <= 0 || sw + sx > image.width())
        sw = image.width() - sx;

    if (sh <= 0 || sh + sy > image.height())
        sh = image.height() - sy;

    if (sx < 0) {
        x -= sx;
        sw += sx;
        sx = 0;
    }

    if (sy < 0) {
        y -= sy;
        sh += sy;
        sy = 0;
    }

    if (w < 0)
        w = sw;
    if (h < 0)
        h = sh;

    if (sw <= 0 || sh <= 0)
        return;

    if ((d->state->txop > QPainterPrivate::TxTranslate
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) ||
        ((w != sw || h != sh) && !d->engine->hasFeature(QPaintEngine::PixmapScale))) {
        QPixmap pm;
        pm.fromImage(image, flags);
        drawPixmap(targetRect, pm, sourceRect, Qt::ComposePixmap);
        return;
    }

    if (d->state->txop == QPainterPrivate::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
        x += qRound(d->state->matrix.dx());
        y += qRound(d->state->matrix.dy());
    }

    d->engine->drawImage(QRectF(x, y, w, h), image, QRectF(sx, sy, sw, sh), flags);
}

/*!
    \fn void QPainter::drawText(int x, int y, const QString &text, TextDirection dir)

    \overload

    Draws the given \a text at position (\a{x}, \a{y}), in text
    direction \a dir.
*/

/*!
    \fn void QPainter::drawText(int x, int y, int w, int h, int flags,
                                const QString &text, QRect *br)

    \overload

    Draws the given \a text within the rectangle with origin (\a{x},
    \a{y}), width \a w and height \a h. The flags that are given in the
    \a flags parameter are a selection of flags from \l{Qt::AlignmentFlag}s
    and \l{Qt::TextFlag}s combined using the bitwise OR operator. \a br
    (if not null) is set to the actual bounding rectangle of the
    output.
*/

/*!
    Draws the string \a str with the currently defined text direction,
    beginning at position \a p.

    \sa QPainter::TextDirection
*/

void QPainter::drawText(const QPointF &p, const QString &str)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawText(), pos=[%.2f,%.2f], str='%s'\n", p.x(), p.y(), str.toLatin1().constData());
#endif

    if (!isActive() || str.isEmpty())
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    QTextLayout layout(str, d->state->pfont ? *d->state->pfont : d->state->font);
    QTextEngine *engine = layout.engine();
    QTextOption option(Qt::AlignLeft|Qt::AlignAbsolute);
    option.setLayoutDirection(d->state->layoutDirection);
    layout.setTextOption(option);

    engine->itemize();

    QTextLine line = layout.createLine();
    line.layout(0x01000000);
    const QScriptLine &sl = engine->lines[0];
    line.draw(this, QPointF(p.x(), p.y() - sl.ascent));
}

/*!
    \overload

    Draws the string \a str within the rectangle \a r. The flags that
    are given in the \a flags parameter are a selection of flags from
    \l{Qt::AlignmentFlag}s and \l{Qt::TextFlag}s combined using the
    bitwise OR operator. \a br (if not null) is set to the actual
    bounding rectangle of the output.
*/
void QPainter::drawText(const QRect &r, int flags, const QString &str, QRect *br)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawText(), r=[%d,%d,%d,%d], flags=%d, str='%s'\n",
           r.x(), r.y(), r.width(), r.height(), flags, str.toLatin1().constData());
#endif

    if (!isActive() || str.length() == 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    QRectF bounds;
    qt_format_text(font(), r, flags, str, &bounds, 0, 0, 0, this);
    if (br)
        *br = bounds.toRect();
}

/*! \fn void QPainter::drawText(const QPoint &p, const QString &s, TextDirection dir = Auto)

    \overload

    Draws the string \a s at position \a p, in text direction \a dir.
*/

/*! \overload

    Draws the string \a str within the rectangle \a r. The specified \a flags
    are constructed from \l{Qt::AlignmentFlag}s and \l{Qt::TextFlag}s,
    combined using the bitwise OR operator. If \a br is not null, it is set
    to the actual bounding rectangle of the output.
*/
void QPainter::drawText(const QRectF &r, int flags, const QString &str, QRectF *br)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawText(), r=[%.2f,%.2f,%.2f,%.2f], flags=%d, str='%s'\n",
           r.x(), r.y(), r.width(), r.height(), flags, str.toLatin1().constData());
#endif

    if (!isActive() || str.length() == 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    qt_format_text(font(), r, flags, str, br, 0, 0, 0, this);
}

/*!
    \fn void QPainter::drawText(const QRectF &rectangle, const QString &text,
        const QTextOption &option)

    Draws the given \a text in the \a rectangle specified using the \a option
    to control its positioning and orientation.
*/
void QPainter::drawText(const QRectF &r, const QString &text, const QTextOption &o)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawText(), r=[%.2f,%.2f,%.2f,%.2f], str='%s'\n",
           r.x(), r.y(), r.width(), r.height(),  text.toLatin1().constData());
#endif

    if (!isActive() || text.length() == 0)
        return;

    Q_D(QPainter);
    d->engine->updateState(d->state);

    int flags = o.alignment();

    if (o.wrapMode() == QTextOption::WordWrap)
        flags |= Qt::TextWordWrap;
    else if (o.wrapMode() == QTextOption::WrapAnywhere)
        flags |= Qt::TextWrapAnywhere;

    if (o.flags() & QTextOption::IncludeTrailingSpaces)
        flags |= Qt::TextIncludeTrailingSpaces;

    qt_format_text(font(), r, flags, text, 0, 0, 0, 0, this);
}

/*!
    \fn void QPainter::drawTextItem(int x, int y, const QTextItem &ti)

    \internal
    \overload
*/

/*!
    \fn void QPainter::drawTextItem(const QPoint &p, const QTextItem &ti)

    \internal
    \overload

    Draws the text item \a ti at position \a p.
*/

/*! \internal
    Draws the text item \a ti at position \a p.

    This method ignores the painters background mode and
    color. drawText and qt_format_text have to do it themselves, as
    only they know the extents of the complete string.

    It ignores the font set on the painter as the text item has one of its own.

    The underline and strikeout parameters of the text items font are
    ignored aswell. You'll need to pass in the correct flags to get
    underlining and strikeout.
*/

void QPainter::drawTextItem(const QPointF &p, const QTextItem &ti)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawTextItem(), pos=[%.f,%.f], str='%s'\n",
           p.x(), p.y(), QString(ti.chars, ti.num_chars).toLatin1().constData());
#endif
    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);
    d->engine->drawTextItem(p, ti);
}

/*!
    \fn QRect QPainter::boundingRect(int x, int y, int w, int h, int flags,
                                     const QString &text);

    \overload

    Returns the bounding rectangle of the characters in the given \a text,
    constrained by the rectangle beginning at the point (\a{x}, \a{y})
    with width \a w and height \a h.
*/

/*!
    \fn QRect QPainter::boundingRect(const QRect &rect, int flags,
                                     const QString &str)

    \overload

    Returns the bounding rectangle constrained by rectangle \a rect.
*/


QRect QPainter::boundingRect(const QRect &rect, int flags, const QString &str)
{
    if (str.isEmpty())
        return QRect(rect.x(),rect.y(), 0,0);
    QRect brect;
    drawText(rect, flags | Qt::TextDontPrint, str, &brect);
    return brect;
}

/*!
    Returns the bounding rectangle of the aligned text that would be
    printed with the corresponding drawText() function of the string
    \a str. The drawing, and hence the bounding rectangle, is constrained
    to the rectangle \a rect, or to the rectangle required to draw the
    text, whichever is the larger.

    The \a flags argument is
    the bitwise OR of the following flags:
    \table
    \header \i Flag \i Meaning
    \row \i \c Qt::AlignAuto \i aligns according to the language, usually left.
    \row \i \c Qt::AlignLeft \i aligns to the left border.
    \row \i \c Qt::AlignRight \i aligns to the right border.
    \row \i \c Qt::AlignHCenter \i aligns horizontally centered.
    \row \i \c Qt::AlignTop \i aligns to the top border.
    \row \i \c Qt::AlignBottom \i aligns to the bottom border.
    \row \i \c Qt::AlignVCenter \i aligns vertically centered.
    \row \i \c Qt::AlignCenter \i (== \c Qt::AlignHCenter | \c Qt::AlignVCenter).
    \row \i \c Qt::TextSingleLine \i ignores newline characters in the text.
    \row \i \c Qt::TextExpandTabs \i expands tabs.
    \row \i \c Qt::TextShowMnemonic \i interprets "&x" as "<u>x</u>".
    \row \i \c Qt::TextWordBreak \i breaks the text to fit the rectangle.
    \endtable

    Qt::Horizontal alignment defaults to \c Qt::AlignLeft and vertical
    alignment defaults to \c Qt::AlignTop.

    If several of the horizontal or several of the vertical alignment flags
    are set, the resulting alignment is undefined.

    \sa Qt::TextFlags
*/

QRectF QPainter::boundingRect(const QRectF &rect, int flags, const QString &str)
{
    if (str.isEmpty())
        return QRectF(rect.x(),rect.y(), 0,0);
    QRectF brect;
    drawText(rect, flags | Qt::TextDontPrint, str, &brect);
    return brect;
}

/*!
    \fn QRectF QPainter::boundingRect(const QRectF &rectangle,
        const QString &text, const QTextOption &option)

    Returns the bounding rectangle for the given \a text when placed within
    the specified \a rectangle. The \a option can be used to control the
    way the text is positioned and orientated.
*/
QRectF QPainter::boundingRect(const QRectF &r, const QString &text, const QTextOption &o)
{
    if (!isActive() || text.length() == 0)
        return QRectF(r.x(),r.y(), 0,0);

    Q_D(QPainter);
    d->engine->updateState(d->state);

    int flags = o.alignment() | Qt::TextDontPrint;
    if (o.wrapMode() == QTextOption::WordWrap)
        flags |= Qt::TextWordWrap;
    else if (o.wrapMode() == QTextOption::WrapAnywhere)
        flags |= Qt::TextWrapAnywhere;
    if (o.flags() & QTextOption::IncludeTrailingSpaces)
        flags |= Qt::TextIncludeTrailingSpaces;

    QRectF br;
    qt_format_text(font(), r, flags, text, &br, 0, 0, 0, this);
    return br;
}

/*!
  \fn void QPainter::drawTiledPixmap(int x, int y, int w, int h, const
  QPixmap &pixmap, int sx, int sy, Qt::PixmapDrawingMode mode);

    Draws a tiled \a pixmap in the specified rectangle.

    (\a{x}, \a{y}) specifies the top-left point in the paint device
    that is to be drawn onto; with the width and height given by \a w
    and \a h. (\a{sx}, \a{sy}) specifies the top-left point in the \a
    pixmap that is to be drawn; this defaults to (0, 0). The pixmap is
    drawn using the given drawing \a mode.

    Calling drawTiledPixmap() is similar to calling drawPixmap()
    several times to fill (tile) an area with a pixmap, but is
    potentially much more efficient depending on the underlying window
    system.

    \sa drawPixmap()
*/

/*! \fn QPainter::drawTiledPixmap(const QRect &rect, const QPixmap &pixmap,
                                  const QPoint &sp = QPoint(),
                                  Qt::PixmapDrawingMode mode = Qt::ComposePixmap)
    \overload

    Draws a tiled \a pixmap, inside rectangle \a rect with its origin
    at point \a sp, using the given drawing \a mode.
*/

/*!
    \overload

    Draws a tiled \a pixmap, inside rectangle \a r with its origin
    at point \a sp, using the given drawing \a mode.
*/
void QPainter::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &sp,
                               Qt::PixmapDrawingMode mode)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::drawTiledPixmap(), target=[%.2f,%.2f,%.2f,%.2f], pix=[%d,%d], offset=[%.2f,%.2f], mode=%d\n",
           r.x(), r.y(), r.width(), r.height(),
           pixmap.width(), pixmap.height(),
           sp.x(), sp.y(), mode);
#endif

    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    qreal sw = pixmap.width();
    qreal sh = pixmap.height();
    if (!sw || !sh)
        return;
    qreal sx = sp.x();
    qreal sy = sp.y();
    if (sx < 0)
        sx = qRound(sw) - qRound(-sx) % qRound(sw);
    else
        sx = qRound(sx) % qRound(sw);
    if (sy < 0)
        sy = qRound(sh) - -qRound(sy) % qRound(sh);
    else
        sy = qRound(sy) % qRound(sh);

    if (d->state->txop > QPainterPrivate::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
        QPixmap pm;
        if (pixmap.hasAlphaChannel()) {
            // Needed to preserve the alpha channel in the pixmap
            // While setPixel() is needed to switch on alpha buffer on Embedded
            QImage img(qRound(r.width()), qRound(r.height()), 32);
            img.setPixel(0, 0, QColor(255, 0, 0, 127).rgba());
            img.setAlphaBuffer(true);
            pm = img;
        } else {
            pm = QPixmap(qRound(r.width()), qRound(r.height()));
        }
        QPainter p(&pm);
        // Recursive call ok, since the pixmap is not transformed...
        p.setPen(pen());
        p.setBackground(background());
        p.setBackgroundMode(backgroundMode());
        p.drawTiledPixmap(QRectF(0, 0, r.width(), r.height()), pixmap, QPointF(sx, sy), Qt::CopyPixmap);
        p.end();
        if (pixmap.depth() == 1) {
            QBitmap mask(pm.width(), pm.height(), true);
            p.begin(&mask);
            p.drawTiledPixmap(QRectF(0, 0, r.width(), r.height()), pixmap, QPointF(sx, sy));
            p.end();
            pm.setMask(mask);
        }
        drawPixmap(qRound(r.x()), qRound(r.y()), pm);
        return;
    }

    qreal x = r.x();
    qreal y = r.y();
    if (d->state->txop == QPainterPrivate::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
        x += qRound(d->state->matrix.dx());
        y += qRound(d->state->matrix.dy());
    }

    d->engine->drawTiledPixmap(QRectF(x, y, r.width(), r.height()), pixmap, QPointF(sx, sy), mode);
}


/*!
    \fn void QPainter::drawPicture(int x, int y, const QPicture &picture)
    \overload

    Draws picture \a picture at point (\a x, \a y).
*/

/*!
    \fn void QPainter::drawPicture(const QPoint &p, const QPicture &picture)
    \overload

    Draws picture \a picture at point \a p.
*/

/*!
    Replays the picture \a picture at point \a p.

    This function does exactly the same as QPicture::play() when
    called with \a p = QPoint(0, 0).
*/

void QPainter::drawPicture(const QPointF &p, const QPicture &picture)
{
    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);
    save();
    translate(p);
    const_cast<QPicture *>(&picture)->play(this);
    restore();
}

/*! \fn void QPainter::eraseRect(const QRect &rect)

    \overload

    Erases the area inside the rectangle \a rect. Equivalent to
    \c{fillRect(rect, backgroundColor())}.
*/

/*!
    Erases the area inside the rectangle \a r. Equivalent to
    \c{fillRect(r, backgroundColor())}.
*/
void QPainter::eraseRect(const QRectF &r)
{
    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    if (d->state->bgBrush.texture().isNull())
        fillRect(r, d->state->bgBrush);
    else
        drawTiledPixmap(r, d->state->bgBrush.texture(), -d->state->bgOrigin);
}

/*!
  \fn void QPainter::eraseRect(int x, int y, int w, int h)
  \overload

    Erases the area inside \a x, \a y, \a w, \a h. Equivalent to
    \c{fillRect(x, y, w, h, backgroundColor())}.
*/


/*!
    Fills the rectangle \a r with the \a brush.

    You can specify a QColor as \a brush, since there is a QBrush
    constructor that takes a QColor argument and creates a solid
    pattern brush.

    \sa drawRect()
*/
void QPainter::fillRect(const QRectF &r, const QBrush &brush)
{
    QPen oldPen   = pen();
    bool swap = oldPen.style() != Qt::NoPen;
    if (swap)
        setPen(Qt::NoPen);
    QBrush oldBrush = this->brush();
    setBrush(brush);
    drawRect(r);
    setBrush(oldBrush);
    if (swap)
        setPen(oldPen);
}

/*!
  \fn void QPainter::fillRect(const QRect &rect, const QBrush &brush)

    \overload

    Fills the rectangle \a rect with the \a brush.

    You can specify a QColor as \a brush, since there is a QBrush
    constructor that takes a QColor argument and creates a solid
    pattern brush.

    \sa drawRect()
*/

/*!
  \fn void QPainter::fillRect(int x, int y, int w, int h, const QBrush &brush)

    \overload

    Fills the rectangle (\a{x}, \a{y}, \a{w}, \a{h}) with the \a brush.

    You can specify a QColor as \a brush, since there is a QBrush
    constructor that takes a QColor argument and creates a solid
    pattern brush.

    \sa drawRect()
*/


/*!
  Sets the render hint \a hint on this painter if \a on is true;
  otherwise clears the render hint.
*/
void QPainter::setRenderHint(RenderHint hint, bool on)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setRenderHint(), hint=%x, %s\n", hint, on ? "on" : "off");
#endif

    if (!isActive()) {
        qWarning("Painter must be active to set rendering hints");
        return;
    }

    Q_D(QPainter);
    d->engine->setRenderHint(hint, on);
}

/*!
    Returns a flag that specifies the rendering hints that this
    painter supports.
*/
QPainter::RenderHints QPainter::supportedRenderHints() const
{
    if (!isActive()) {
        qWarning("Painter must be active to set rendering hints");
        return 0;
    }
    Q_D(const QPainter);
    return d->engine->supportedRenderHints();
}

/*!
    Returns a flag that specifies the rendering hints that are set for
    this painter.
*/
QPainter::RenderHints QPainter::renderHints() const
{
    if (!isActive()) {
        qWarning("Painter must be active to set rendering hints");
        return 0;
    }
    Q_D(const QPainter);
    return d->engine->renderHints();
}

/*!
    Returns true if view transformation is enabled; otherwise returns
    false.

    \sa setViewTransformEnabled(), matrix()
*/

bool QPainter::viewTransformEnabled() const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->VxF;
#else
    return d->state->xlatex || d->state->xlatey;
#endif
}

/*!
    \fn void QPainter::setWindow(const QRect &r)

    \overload

    Sets the painter's window to the rectangle \a r.
*/

/*!
  \fn void QPainter::setWindow(int x, int y, int w, int h)

    Sets the window rectangle view transformation for the painter and
    enables view transformation.

    The window rectangle is part of the view transformation. The
    window specifies the logical coordinate system and is specified by
    the \a x, \a y, \a w width and \a h height parameters. Its sister,
    the viewport(), specifies the device coordinate system.

    The default window rectangle is the same as the device's
    rectangle. See the \link coordsys.html Coordinate System Overview
    \endlink for an overview of coordinate transformation.

    \sa window(), setViewport(), setViewTransformEnabled(), setMatrix(),
    setMatrixEnabled()
*/

void QPainter::setWindow(const QRect &r)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setWindow(), [%d,%d,%d,%d]\n", r.x(), r.y(), r.width(), r.height());
#endif

    if (!isActive()) {
        qWarning("QPainter::setWindow(), painter not active");
        return;
    }

    Q_D(QPainter);
    // Must update clip before changing matrix.
    if (d->engine->hasFeature(QPaintEngine::ClipTransform)
        && d->engine->testDirty(QPaintEngine::DirtyClip))
        d->engine->updateState(d->state);

    d->state->wx = r.x();
    d->state->wy = r.y();
    d->state->ww = r.width();
    d->state->wh = r.height();
    if (d->state->VxF)
        d->updateMatrix();
    else
        setViewTransformEnabled(true);
}

/*!
    Returns the window rectangle.

    \sa setWindow(), setViewTransformEnabled()
*/

QRect QPainter::window() const
{
    Q_D(const QPainter);
    return QRect(d->state->wx, d->state->wy, d->state->ww, d->state->wh);
}

/*!
    \fn void QPainter::setViewport(const QRect &r)

    \overload

    Sets the painter's viewport rectangle to \a r.
*/

/*!
  \fn void QPainter::setViewport(int x, int y, int w, int h)

    Sets the viewport rectangle view transformation for the painter
    and enables view transformation.

    The viewport rectangle is part of the view transformation. The
    viewport specifies the device coordinate system and is specified
    by the \a x, \a y, \a w width and \a h height parameters. Its
    sister, the window(), specifies the logical coordinate system.

    The default viewport rectangle is the same as the device's
    rectangle. See the \link coordsys.html Coordinate System Overview
    \endlink for an overview of coordinate transformation.

    \sa viewport(), setWindow(), setViewTransformEnabled(), setMatrix(),
    setMatrixEnabled()
*/

void QPainter::setViewport(const QRect &r)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setViewport(), [%d,%d,%d,%d]\n", r.x(), r.y(), r.width(), r.height());
#endif

    if (!isActive()) {
        qWarning("QPainter::setViewport(), painter not active");
        return;
    }

    Q_D(QPainter);

    // Must update clip before changing matrix.
    if (d->engine->hasFeature(QPaintEngine::ClipTransform)
        && d->engine->testDirty(QPaintEngine::DirtyClip))
        d->engine->updateState(d->state);

    d->state->vx = r.x();
    d->state->vy = r.y();
    d->state->vw = r.width();
    d->state->vh = r.height();
    if (d->state->VxF)
        d->updateMatrix();
    else
        setViewTransformEnabled(true);
}

/*!
    Returns the viewport rectangle.

    \sa setViewport(), setViewTransformEnabled()
*/

QRect QPainter::viewport() const
{
    Q_D(const QPainter);
    return QRect(d->state->vx, d->state->vy, d->state->vw, d->state->vh);
}

/*! \fn bool QPainter::hasViewXForm() const
    \compat

    Use viewTransformEnabled() instead.
*/

/*! \fn bool QPainter::hasWorldXForm() const
    \compat

    Use matrixEnabled() instead.
*/

/*! \fn void QPainter::resetXForm()
    \compat

    Use resetMatrix() instead.
*/

/*! \fn void QPainter::setViewXForm(bool enabled)
    \compat

    Use setViewTransformEnabled() instead.
*/

/*! \fn void QPainter::setWorldMatrix(const QMatrix &wm, bool combine=false)
    \compat

    Use setMatrix() instead.
*/

/*! \fn void QPainter::setWorldXForm(bool enabled)
    \compat

    Use setMatrixEnabled() instead.
*/

/*! \fn const QMatrix &QPainter::worldMatrix() const
    \compat

    Use matrix() instead.
*/

/*!
    Enables view transformations if \a enable is true, or disables
    view transformations if \a enable is false.

    \sa viewTransformEnabled(), setWindow(), setViewport(), setMatrix(),
    setMatrixEnabled()
*/

void QPainter::setViewTransformEnabled(bool enable)
{
#ifdef QT_DEBUG_DRAW
    if (qt_show_painter_debug_output)
        printf("QPainter::setViewTransformEnabled(), enable=%d\n", enable);
#endif

    if (!isActive()) {
        qWarning("QPainter::setViewTransformEnabled(), painter not active");
        return;
    }
    Q_D(QPainter);
    if (enable == d->state->VxF)
        return;

    // Must update clip before changing matrix.
    if (d->engine->hasFeature(QPaintEngine::ClipTransform)
        && d->engine->testDirty(QPaintEngine::DirtyClip))
        d->engine->updateState(d->state);

    d->state->VxF = enable;
    d->updateMatrix();
}

#ifdef QT3_SUPPORT

qreal QPainter::translationX() const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->worldMatrix.dx();
#else
    return d->state->xlatex;
#endif
}

qreal QPainter::translationY() const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->worldMatrix.dy();
#else
    return d->state->xlatey;
#endif
}

/*!
    \fn void QPainter::map(int x, int y, int *rx, int *ry) const

    \internal

    Sets (\a{rx}, \a{ry}) to the point that results from applying the
    painter's current transformation on the point (\a{x}, \a{y}).
*/
void QPainter::map(int x, int y, int *rx, int *ry) const
{
    Q_D(const QPainter);
    QPoint p(x, y);
    p = p * d->state->matrix;
    *rx = p.x();
    *ry = p.y();
}

/*!
    Returns the point \a p transformed from model coordinates to
    device coordinates.

    \sa xFormDev(), QMatrix::map()
*/

QPoint QPainter::xForm(const QPoint &p) const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == QPainterPrivate::TxNone)
        return p;
    return p * d->state->matrix;
#else
    return QPoint(p.x() + d->state->xlatex, p.y() + d->state->xlatey);
#endif
}


/*!
    \overload

    Returns the rectangle \a r transformed from model coordinates to
    device coordinates.

    If world transformation is enabled and rotation or shearing has
    been specified, then the bounding rectangle is returned.

    \sa xFormDev(), QMatrix::map()
*/

QRect QPainter::xForm(const QRect &r) const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == QPainterPrivate::TxNone)
        return r;
    return d->state->matrix.mapRect(r);
#else
    return QRect(r.x()+d->state->xlatex, r.y()+d->state->xlatey, r.width(), r.height());
#endif
}

/*!
    \overload

    Returns the point array \a a transformed from model coordinates
    to device coordinates.

    \sa xFormDev(), QMatrix::map()
*/

QPolygon QPainter::xForm(const QPolygon &a) const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == QPainterPrivate::TxNone)
        return a;
    return a * d->state->matrix;
#else
    QPolygon p(a);
    p.translate(d->state->xlatex, d->state->xlatey);
    return p;
#endif
}

/*!
    \overload

    Returns the point array \a av transformed from model coordinates
    to device coordinates. The \a index is the first point in the
    array and \a npoints denotes the number of points to be
    transformed. If \a npoints is negative, all points from
    \a{av}\e{[index]} until the last point in the array are transformed.

    The returned point array consists of the number of points that
    were transformed.

    Example:
    \code
        QPolygon a(10);
        QPolygon b;
        b = painter.xForm(a, 2, 4);  // b.size() == 4
        b = painter.xForm(a, 2, -1); // b.size() == 8
    \endcode

    \sa xFormDev(), QMatrix::map()
*/

QPolygon QPainter::xForm(const QPolygon &av, int index, int npoints) const
{
    Q_D(const QPainter);
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPolygon a(lastPoint-index);
    memcpy(a.data(), av.data()+index, (lastPoint-index)*sizeof(QPoint));
#ifndef QT_NO_TRANSFORMATIONS
    return a * d->state->matrix;
#else
    a.translate(d->state->xlatex, d->state->xlatey);
    return a;
#endif
}

/*!
    \overload

    Returns the point \a p transformed from device coordinates to
    model coordinates.

    \sa xForm(), QMatrix::map()
*/

QPoint QPainter::xFormDev(const QPoint &p) const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    if(d->state->txop == QPainterPrivate::TxNone)
        return p;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->d_ptr->updateInvMatrix();
    }
    return p * d->invMatrix;
#else
    return QPoint(p.x() - xlatex, p.y() - xlatey);
#endif
}

/*!
    Returns the rectangle \a r transformed from device coordinates to
    model coordinates.

    If world transformation is enabled and rotation or shearing is
    used, then the bounding rectangle is returned.

    \sa xForm(), QMatrix::map()
*/

QRect QPainter::xFormDev(const QRect &r)  const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == QPainterPrivate::TxNone)
        return r;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->d_ptr->updateInvMatrix();
    }
    return d->invMatrix.mapRect(r);
#else
    return QRect(r.x()-d->state->xlatex, r.y()-d->state->xlatey, r.width(), r.height());
#endif
}

/*!
    \overload

    Returns the point array \a a transformed from device coordinates
    to model coordinates.

    \sa xForm(), QMatrix::map()
*/

QPolygon QPainter::xFormDev(const QPolygon &a) const
{
    Q_D(const QPainter);
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == QPainterPrivate::TxNone)
        return a;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->d_ptr->updateInvMatrix();
    }
    return a * d->invMatrix;
#else
    QPolygon p(a);
    p.translate(-d->state->xlatex, -d->state->xlatey);
    return p;
#endif

}

/*!
    \overload

    Returns the point array \a ad transformed from device coordinates
    to model coordinates. The \a index is the first point in the array
    and \a npoints denotes the number of points to be transformed. If
    \a npoints is negative, all points from \a{ad}\e{[index]} until the
    last point in the array are transformed.

    The returned point array consists of the number of points that
    were transformed.

    Example:
    \code
        QPolygon a(10);
        QPolygon b;
        b = painter.xFormDev(a, 1, 3);  // b.size() == 3
        b = painter.xFormDev(a, 1, -1); // b.size() == 9
    \endcode

    \sa xForm(), QMatrix::map()
*/

QPolygon QPainter::xFormDev(const QPolygon &ad, int index, int npoints) const
{
    Q_D(const QPainter);
    int lastPoint = npoints < 0 ? ad.size() : index+npoints;
    QPolygon a(lastPoint-index);
    memcpy(a.data(), ad.data()+index, (lastPoint-index)*sizeof(QPoint));
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == QPainterPrivate::TxNone)
        return a;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->d_ptr->updateInvMatrix();
    }
    return a * d->invMatrix;
#else
    a.translate(-d->state->xlatex, -d->state->xlatey);
    return a;
#endif
}

/*!
    \fn void QPainter::drawPoints(const QPolygon &points)

    \overload

    Draws the array of points \a points using the current pen's color.

    \sa drawPoints
*/

/*!
    Draws a cubic Bezier curve defined by the control points in \a a,
    starting at \a{a}\e{[index]} (\a index defaults to 0).

    Control points after \a{a}\e{[index + 3]} are ignored. Nothing happens
    if there aren't enough control points.
*/
void QPainter::drawCubicBezier(const QPolygon &a, int index)
{
    if (!isActive())
        return;
    Q_D(QPainter);
    d->engine->updateState(d->state);

    if ((int)a.size() - index < 4) {
        qWarning("QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
                  "points");
        return;
    }

    QPainterPath path;
    path.moveTo(a.at(index));
    path.cubicTo(a.at(index+1), a.at(index+2), a.at(index+3));
    strokePath(path, d->state->pen);
}
#endif

struct QPaintDeviceRedirection
{
    QPaintDeviceRedirection() : device(0), replacement(0) {}
    QPaintDeviceRedirection(const QPaintDevice *device, QPaintDevice *replacement,
                            const QPoint& offset)
        : device(device), replacement(replacement), offset(offset) { }
    const QPaintDevice *device;
    QPaintDevice *replacement;
    QPoint offset;
    bool operator==(const QPaintDevice *pdev) const { return device == pdev; }
    Q_DUMMY_COMPARISON_OPERATOR(QPaintDeviceRedirection)
};

typedef QList<QPaintDeviceRedirection> QPaintDeviceRedirectionList;
Q_GLOBAL_STATIC(QPaintDeviceRedirectionList, globalRedirections)

/*!
  \internal

    Redirects all paint commands for a paint device, \a device, to
    another paint device, \a replacement. The optional point \a offset
    defines an offset within the replaced device. After painting you
    must call restoreRedirected().

    In general, you'll probably find calling QPixmap::grabWidget() or
    QPixmap::grabWindow() is an easier solution.

    \sa redirected()
*/
void QPainter::setRedirected(const QPaintDevice *device,
                             QPaintDevice *replacement,
                             const QPoint &offset)
{
    Q_ASSERT(device != 0);
    QPaintDeviceRedirectionList *redirections = globalRedirections();
    Q_ASSERT(redirections != 0);

    QPoint roffset;
    QPaintDevice *rdev = redirected(replacement, &roffset);
    *redirections += QPaintDeviceRedirection(device, rdev ? rdev : replacement, offset + roffset);
}


/*!\internal

  Restores the previous redirection for \a device after a call to
  setRedirected().

  \sa redirected()
 */
void QPainter::restoreRedirected(const QPaintDevice *device)
{
    Q_ASSERT(device != 0);
    QPaintDeviceRedirectionList *redirections = globalRedirections();
    Q_ASSERT(redirections != 0);
    for (int i = redirections->size()-1; i >= 0; --i)
        if (redirections->at(i) == device) {
            redirections->removeAt(i);
            return;
        }
}


/*!
    \internal

    Returns the replacement for \a device. The optional out parameter
    \a offset returns return the offset within the replaced device.
*/
QPaintDevice *QPainter::redirected(const QPaintDevice *device, QPoint *offset)
{
    Q_ASSERT(device != 0);
    QPaintDeviceRedirectionList *redirections = globalRedirections();
    Q_ASSERT(redirections != 0);
    for (int i = redirections->size()-1; i >= 0; --i)
        if (redirections->at(i) == device) {
            if (offset)
                *offset = redirections->at(i).offset;
            return redirections->at(i).replacement;
        }
    if (offset)
        *offset = QPoint(0, 0);
    return 0;
}


void qt_format_text(const QFont &font, const QRectF &_r,
                    int tf, const QString& str, QRectF *brect,
                    int tabstops, int *, int tabarraylen,
                    QPainter *painter)
{
    // we need to copy r here to protect against the case (&r == brect).
    QRectF r(_r);

    bool dontclip  = (tf & Qt::TextDontClip);
    bool wordwrap  = (tf & Qt::TextWordWrap);
    bool singleline = (tf & Qt::TextSingleLine);
    bool showmnemonic = (tf & Qt::TextShowMnemonic);
    bool hidemnmemonic = (tf & Qt::TextHideMnemonic);

    tf = QStyle::visualAlignment(painter ? painter->layoutDirection() : QApplication::layoutDirection(), QFlag(tf));

    bool isRightToLeft = (painter ? painter->layoutDirection() : qApp->layoutDirection()) == Qt::RightToLeft;
    bool expandtabs = ((tf & Qt::TextExpandTabs) &&
                        (((tf & Qt::AlignLeft) && !isRightToLeft) ||
                          ((tf & Qt::AlignRight) && isRightToLeft)));

    if (!painter)
        tf |= Qt::TextDontPrint;

    int maxUnderlines = 0;
    int numUnderlines = 0;
    int underlinePositionStack[32];
    int *underlinePositions = underlinePositionStack;

    QFont fnt(painter
              ? (painter->d_ptr->state->pfont
                 ? *painter->d_ptr->state->pfont
                 : painter->d_ptr->state->font)
              : font);
    QFontMetricsF fm(fnt);

    QString text = str;
    // compatible behaviour to the old implementation. Replace
    // tabs by spaces
    QChar *chr = text.data();
    const QChar *end = chr + str.length();
    while (chr != end) {
        if (*chr == QLatin1Char('\r') || (singleline && *chr == QLatin1Char('\n'))) {
            *chr = ' ';
        } else if (*chr == QLatin1Char('\n')) {
            *chr = QChar::LineSeparator;
        } else if (*chr == QLatin1Char('&')) {
            ++maxUnderlines;
        }
        ++chr;
    }
    if (!expandtabs) {
        chr = text.data();
        while (chr != end) {
            if (*chr == QLatin1Char('\t'))
                *chr = QLatin1Char(' ');
            ++chr;
        }
    } else if (!tabarraylen && !tabstops) {
        tabstops = qRound(fm.width(QLatin1Char('x'))*8);
    }

    if (hidemnmemonic || showmnemonic) {
        if (maxUnderlines > 32)
            underlinePositions = new int[maxUnderlines];
        QChar *cout = text.data();
        QChar *cin = cout;
        int l = str.length();
        while (l) {
            if (*cin == QLatin1Char('&')) {
                ++cin;
                --l;
                if (!l)
                    break;
                if (*cin != QLatin1Char('&') && !hidemnmemonic)
                    underlinePositions[numUnderlines++] = cout - text.unicode();
            }
            *cout = *cin;
            ++cout;
            ++cin;
            --l;
        }
        int newlen = cout - text.unicode();
        if (newlen != text.length())
            text.resize(newlen);
    }

    // no need to do extra work for underlines if we don't paint
    if (tf & Qt::TextDontPrint)
        numUnderlines = 0;

    underlinePositions[numUnderlines] = -1;
    qreal height = 0;
    qreal width = 0;

    QTextLayout textLayout(text, fnt);
    textLayout.engine()->underlinePositions = underlinePositions;

    if (text.isEmpty()) {
        height = fm.height();
        width = 0;
        tf |= Qt::TextDontPrint;
    } else {
        qreal lineWidth = wordwrap ? qMax<qreal>(0, r.width()) : 0x01000000;
        if(!wordwrap)
            tf |= Qt::TextIncludeTrailingSpaces;
        textLayout.setLayoutMode((tf & Qt::TextDontPrint) ? QTextLayout::NoBidi : QTextLayout::MultiLine);
        textLayout.beginLayout();

        qreal leading = fm.leading();
        height = -leading;

        while (1) {
            QTextLine l = textLayout.createLine();
            if (!l.isValid())
                break;

            l.layout(lineWidth);
            height += leading;
            l.setPosition(QPointF(0., height));
            height += l.ascent() + l.descent();
            width = qMax(width, l.textWidth());
        }
        textLayout.endLayout();
    }

    qreal yoff = 0;
    qreal xoff = 0;
    if (tf & Qt::AlignBottom)
        yoff = r.height() - height;
    else if (tf & Qt::AlignVCenter)
        yoff = (r.height() - height)/2;
    if (tf & Qt::AlignRight)
        xoff = r.width() - width;
    else if (tf & Qt::AlignHCenter)
        xoff = (r.width() - width)/2;
    if (brect)
        *brect = QRectF(r.x() + xoff, r.y() + yoff, width, height);

    if (!(tf & Qt::TextDontPrint)) {
        bool restore = false;
        if (!dontclip) {
            restore = true;
            painter->save();
            painter->setClipRect(r, Qt::IntersectClip);
        }

        for (int i = 0; i < textLayout.numLines(); i++) {
            QTextLine line = textLayout.lineAt(i);

            line.draw(painter, QPointF(r.x() + xoff + line.x(), r.y() + yoff));
        }

        if (restore) {
            painter->restore();
        }
    }

    if (underlinePositions != underlinePositionStack)
        delete [] underlinePositions;
}

QPixmap qt_image_linear_gradient(const QRect &rect,
                                 const QPointF &pt1, const QColor &color1,
                                 const QPointF &pt2, const QColor &color2)
{
#ifdef QT_DEBUG_DRAW
    printf(" -> image_gradient(), [%d, %d, %d, %d]\n"
           "   from: (%.2f, %.2f) col=%x\n"
           "   to  : (%.2f, %.2f) col=%x\n",
           rect.x(), rect.y(), rect.width(), rect.height(),
           pt1.x(), pt1.y(), color1.rgba(), pt2.x(), pt2.y(), color2.rgba());
#endif

    qreal x1 = pt1.x();
    qreal y1 = pt1.y();
    qreal x2 = pt2.x();
    qreal y2 = pt2.y();

    QString key = QString("qt_gradient %1 %2 %3 %4")
                  .arg(rect.x())
                  .arg(rect.y())
                  .arg(rect.width())
                  .arg(rect.height())
                  + QString("%1 %2 %3 %4 %5 %6")
#ifdef QT_USE_FIXED_POINT
                  .arg(x1.value())
                  .arg(y1.value())
                  .arg(color1.rgba())
                  .arg(x2.value())
                  .arg(y2.value())
                  .arg(color2.rgba())
#else
                  .arg(x1)
                  .arg(y1)
                  .arg(color1.rgba())
                  .arg(x2)
                  .arg(y2)
                  .arg(color2.rgba())
#endif
                  ;
    if (QPixmap *pixmap = QPixmapCache::find(key)) {
        return *pixmap;
    }

    QImage image(rect.width(), rect.height(), 32);

    qreal gdx = x2 - x1;
    qreal gdy = y2 - y1;
    qreal glen = sqrt(gdx * gdx + gdy * gdy);

    int a1 = color1.alpha(), r1 = color1.red(), g1 = color1.green(), b1 = color1.blue();
    int a2 = color2.alpha(), r2 = color2.red(), g2 = color2.green(), b2 = color2.blue();

    bool useAlpha = a1 != 255 || a2 != 255;

    if (glen == 0) {
        image.fill(a1);
        image.setAlphaBuffer(useAlpha);
        return image;
    }

    image.setAlphaBuffer(useAlpha);
    qreal t;

    for (int y=0; y<image.height(); ++y) {
        QRgb *scanLine = (QRgb*)image.scanLine(y);
        int oset = 0;
        for (int x=0; x<image.width(); ++x) {
            t = (gdx * (x-x1) + gdy * (y-y1)) / glen;
            if (t < 0) {
                scanLine[oset++] = qRgba(r1, g1, b1, a1);
            } else if (t > glen) {
                scanLine[oset++] = qRgba(r2, g2, b2, a2);
            } else {
                t = t / glen;
                scanLine[oset++] = qRgba(qRound(r1 * (1-t) + r2 * t), qRound(g1 * (1-t) + g2 * t),
                                         qRound(b1 * (1-t) + b2 * t), qRound(a1 * (1-t) + a2 * t));
            }
        }
    }

    QPixmap pixmap;
    pixmap.fromImage(image, Qt::OrderedDither | Qt::OrderedAlphaDither);
    QPixmapCache::insert(key, pixmap);

    return pixmap;
}

void QPainter::setLayoutDirection(Qt::LayoutDirection direction)
{
    Q_D(QPainter);
    d->state->layoutDirection = direction;
}

Qt::LayoutDirection QPainter::layoutDirection() const
{
    Q_D(const QPainter);
    return d->state->layoutDirection;
}

QPainterState::QPainterState(const QPainterState *s)
{
    font = s->font;
    deviceFont = s->deviceFont;
    pfont = s->pfont ? new QFont(*s->pfont) : 0;
    pen = QPen(s->pen);
    brush = QBrush(s->brush);
    bgOrigin = s->bgOrigin;
    bgBrush = QBrush(s->bgBrush);
    tmpClipRegion = QRegion(s->tmpClipRegion);
    tmpClipPath = s->tmpClipPath;
    tmpClipOp = s->tmpClipOp;
    bgMode = s->bgMode;
    VxF = s->VxF;
    WxF = s->WxF;
#ifndef QT_NO_TRANSFORMATIONS
    worldMatrix = s->worldMatrix;
    matrix = s->matrix;
    txop = s->txop;
#else
    xlatex = s->xlatex;
    xlatey = s->xlatey;
#endif
    wx = s->wx;
    wy = s->wy;
    ww = s->ww;
    wh = s->wh;
    vx = s->vx;
    vy = s->vy;
    vw = s->vw;
    vh = s->vh;
    painter = s->painter;
    clipInfo = s->clipInfo;
    changeFlags = 0;
    layoutDirection = s->layoutDirection;
}

QPainterState::QPainterState()
{
    init(0);
}

QPainterState::~QPainterState()
{
        delete pfont;
}

void QPainterState::init(QPainter *p) {
    bgBrush = Qt::white;
        bgMode = Qt::TransparentMode;
        WxF = false;
        VxF = false;
        wx = wy = ww = wh = 0;
        vx = vy = vw = vh = 0;
        changeFlags = 0;
        pfont = 0;
        painter = p;
        pen = QPen();
        bgOrigin = QPointF(0, 0);
        brush = QBrush();
        font = deviceFont = QFont();
        tmpClipRegion = QRegion();
        tmpClipPath = QPainterPath();
        tmpClipOp = Qt::NoClip;
#ifndef QT_NO_TRANSFORMATIONS
        worldMatrix.reset();
        matrix.reset();
        txop = 0;
#else
        xlatex = xlatey = 0;
#endif
        layoutDirection = QApplication::layoutDirection();
}

#ifdef QT3_SUPPORT
static void bitBlt_helper(QPaintDevice *dst, const QPoint &dp,
                          const QPaintDevice *src, const QRect &sr, bool imask)
{
    Q_ASSERT(dst);
    Q_ASSERT(src);

    if (src->devType() == QInternal::Pixmap) {
        const QPixmap *pixmap = static_cast<const QPixmap *>(src);
        QPainter pt(dst);
        if (dst->devType() == QInternal::Pixmap)
            pt.drawPixmap(dp, *pixmap, sr, imask ? Qt::CopyPixmapNoMask : Qt::CopyPixmap);
        else
            pt.drawPixmap(dp, *pixmap, sr, Qt::ComposePixmap);

    } else {
        qWarning("::bitBlt only works when source is of type pixmap");
    }
}

void bitBlt(QPaintDevice *dst, int dx, int dy,
             const QPaintDevice *src, int sx, int sy, int sw, int sh,
             bool ignoreMask )
{
    bitBlt_helper(dst, QPoint(dx, dy), src, QRect(sx, sy, sw, sh), ignoreMask);
}

void bitBlt(QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, bool ignoreMask)
{
    bitBlt_helper(dst, dp, src, sr, ignoreMask);
}

void bitBlt(QPaintDevice *dst, int dx, int dy,
            const QImage *src, int sx, int sy, int sw, int sh, Qt::ImageConversionFlags flags)
{
    QPixmap srcPixmap(src->width(), src->height());
    srcPixmap.fromImage(*src, flags);
    bitBlt_helper(dst, QPoint(dx, dy), &srcPixmap, QRect(sx, sy, sw, sh), false);
}

#endif // QT3_SUPPORT

/*!
    \fn void QPainter::setBackgroundColor(const QColor &color)

    Use setBackground() instead.
*/

/*!
    \fn const QColor &QPainter::backgroundColor() const

    Use background().color() instead.
*/

/*!
    \fn void QPainter::drawText(int x, int y, const QString &text, int pos, int len, TextDirection dir)
    \compat

    Use drawText(x, y, text.mid(pos, len), dir) instead.
*/

/*!
    \fn void QPainter::drawText(const QPoint &p, const QString &text, int pos, int len, TextDirection dir)
    \compat

    Use drawText(p, text.mid(pos, len), dir) instead.
*/

/*!
    \fn void QPainter::drawText(int x, int y, const QString &text, int len, TextDirection dir)
    \compat

    Use drawText(x, y, text.left(len), dir) instead.
*/

/*!
    \fn void QPainter::drawText(const QPoint &p, const QString &s, int len, TextDirection dir)
    \compat

    Use drawText(p, text.left(len), dir) instead.
*/

/*!
    \fn bool QPainter::begin(QPaintDevice *pdev, const QWidget *init)
    \compat
*/


/*!
    \fn void QPainter::drawImage(const QPoint &p, const QImage &image)

    Draws the image \a image at point \a p.
*/

/*!
    \fn void QPainter::drawImage(const QPointF &p, const QImage &image)

    \overload

    Draws the image \a image at point \a p.
*/

/*!
    \fn void QPainter::drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                                 Qt::ImageConversionFlags flags = 0)

    Draws the rectangle \a sr of image \a image with its origin at point \a p.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a flags to
    specify how you would prefer this to happen.

    \sa Qt::ImageConversionFlags
*/

/*!
    \fn void QPainter::drawImage(const QPoint &p, const QImage &image, const QRect &sr,
                                 Qt::ImageConversionFlags flags = 0)
    \overload

    Draws the rectangle \a sr of image \a image with its origin at point \a p.
*/

/*!
    \fn void QPainter::drawImage(const QRectF &rectangle, const QImage &image)

    Draws \a image into \a rectangle.
*/

/*!
    \fn void QPainter::drawImage(const QRect &rectangle, const QImage &image)

    \overload

    Draws \a image into \a rectangle.
*/

/*!
    \fn void QPainter::drawImage(int x, int y, const QImage &image,
                                 int sx, int sy, int sw, int sh,
                                 Qt::ImageConversionFlags flags)
    \overload

    Draws an image at (\a{x}, \a{y}) by copying a part of \a image into
    the paint device.

    (\a{x}, \a{y}) specifies the top-left point in the paint device that is
    to be drawn onto. (\a{sx}, \a{sy}) specifies the top-left point in \a
    image that is to be drawn. The default is (0, 0).

    (\a{sw}, \a{sh}) specifies the size of the image that is to be drawn.
    The default, (-1, -1), means all the way to the bottom-right of
    the image.
*/

/*!
    \fn void QPainter::drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                                 Qt::ImageConversionFlags flags)
    \overload
*/

/*!
    \fn void QPainter::redirect(QPaintDevice *pdev, QPaintDevice *replacement)

    Use setRedirected() instead.
*/

/*!
    \fn QPaintDevice *QPainter::redirect(QPaintDevice *pdev)

    Use redirected() instead.
*/

/*!
    \fn QRect QPainter::boundingRect(const QRect &rect, int flags,
                                     const QString &text, int len)
    \compat
*/

/*!
    \fn void QPainter::drawText(const QRect &r, int flags, const QString &str,
                                int len, QRect *br)
    \compat
*/

/*!
    \fn QRect QPainter::boundingRect(int x, int y, int w, int h, int flags,
                                     const QString &text, int len);

    \compat

    Returns the bounding rectangle of the first \a len characters of
    the string \a str constrained by the rectangle that begins at
    point (\a{x}, \a{y}) with width \a w and height \a h.
*/

/*!
    \fn void QPainter::drawText(int x, int y, int w, int h, int flags,
                                const QString &str, int len, QRect *br)

    \compat

    Draws the string \a str within the rectangle with origin (\a{x},
    \a{y}), width \a w and height \a h. If \a len is -1 (the default)
    all the text is drawn, otherwise only the first \a len characters
    are drawn. The flags that are given in the \a flags parameter are
    \l{Qt::AlignmentFlag}s and \l{Qt::TextFlag}s OR'd together. \a br
    (if not null) is set to the actual bounding rectangle of the
    output.
*/
