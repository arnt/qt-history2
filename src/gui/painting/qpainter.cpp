/****************************************************************************
**
** Definition of QPainter class.
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

// QtGui
#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpaintengine.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpainterpath.h"
#include "qpicture.h"
#include "qtextlayout.h"
#include "qwidget.h"
#include <private/qfontengine_p.h>
#include <private/qpainterpath_p.h>
#include <private/qtextengine_p.h>
#include <private/qwidget_p.h>

// QtCore
#include <qdebug.h>

// Other
#include <math.h>

void qt_format_text(const QFont &font, const QRect &_r, int tf, const QString& str,
                    int len, QRect *brect, int tabstops, int* tabarray, int tabarraylen,
                    QPainter* painter);
void qt_fill_linear_gradient(const QRect &r, QPainter *pixmap, const QBrush &brush);

// Helper function for filling gradients...
#define QT_FILL_GRADIENT(rect, fillCall, outlineCall)                   \
        QBitmap mask(rect.width(), rect.height());                      \
        mask.fill(Qt::color0);                                              \
        QPainter p(&mask);                                              \
        p.setPen(Qt::NoPen);                                                \
        p.setBrush(Qt::color1);                                             \
        p.fillCall;                                                     \
        save();                                                         \
        QRegion region(mask);                                           \
        region.translate(rect.topLeft());                               \
        setClipRegion(region);                                          \
        qt_fill_linear_gradient(rect, this, d->state->brush);           \
        if (d->state->pen.style() != Qt::NoPen) {                           \
            setBrush(Qt::NoBrush);                                          \
            outlineCall;                                                \
        }                                                               \
        restore();                                                      \
        return;                                                         \


/*!
    \class QPainter qpainter.h
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

    \i viewport(), window(), worldMatrix() and many more make up the
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
    drawLineSegments(), drawPolyline(), drawPolygon(),
    drawConvexPolygon() and drawCubicBezier(). All of these functions
    take integer coordinates; there are no floating-point versions
    since we want drawing to be as fast as possible.

    There are functions to draw pixmaps/images, namely drawPixmap(),
    drawImage() and drawTiledPixmap(). drawPixmap() and drawImage()
    produce the same result, except that drawPixmap() is faster
    on-screen and drawImage() faster and sometimes better on QPrinter
    and QPicture.

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
    and shear(), all of which operate on the worldMatrix().
    setWorldMatrix() can replace or add to the currently set
    worldMatrix().

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

    \value LineAntialiasing
*/

/*!
    \enum QPainter::TextDirection
    \value Auto
    \value RTL right to left
    \value LTR left to right

    \sa drawText()
*/

/*!
    \fn QPaintEngine *QPaintDevice::paintEngine() const

    \internal
*/

/*!
    Constructs a painter.

    Notice that all painter settings (setPen, setBrush etc.) are reset
    to default values when begin() is called.

    \sa begin(), end()
*/

QPainter::QPainter()
{
    d = new QPainterPrivate;
    init();
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
    d = new QPainterPrivate;
    init();
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
    delete d;
}

void QPainter::init()
{
    d->state->painter = this;
}

/*!
    Returns the paint device on which this painter is currently
    painting, or 0 if the painter is not active.

    \sa QPaintDevice::paintingActive()
*/

QPaintDevice *QPainter::device() const
{
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
    d->state->pen = pal.color(QPalette::Foreground);
    d->state->bgBrush = pal.background();
    d->state->font = widget->font();
    if (d->engine) {
        d->engine->setDirty(QPaintEngine::DirtyPen);
        d->engine->setDirty(QPaintEngine::DirtyBrush);
        d->engine->setDirty(QPaintEngine::DirtyFont);
    }
}


/*!
    Saves the current painter state (pushes the state onto a stack). A
    save() must be followed by a corresponding restore(). end()
    unwinds the stack.

    \sa restore()
*/

void QPainter::save()
{
    d->state = new QPainterState(d->states.back());
    d->states.push_back(d->state);
    if (isActive())
        d->engine->updateState(d->state, false);
}

/*!
    Restores the current painter state (pops a saved state off the
    stack).

    \sa save()
*/

void QPainter::restore()
{
    if (d->states.size()==0) {
        qWarning("QPainter::restore(), unbalanced save/restore");
        return;
    }

    QPainterState *tmp = d->state;
    d->states.pop_back();
    d->state = d->states.back();

    d->txinv = false;
    if (d->engine)
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
        p->begin(pm); // impossible - pm.isNull();

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

    if (d->engine) {
        qWarning("QPainter::begin(): Painter is already active."
                 "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    QPaintDevice *rpd = redirected(pd, &d->redirection_offset);
    if (rpd) {
        pd = rpd;
    }

    switch (pd->devType()) {
        case QInternal::Widget:
        {
            const QWidget *widget = static_cast<const QWidget *>(pd);
            Q_ASSERT(widget);

            if(!widget->testWState(Qt::WState_InPaintEvent)) {
                qWarning("QPainter::begin: Widget painting can only begin as a "
                         "result of a paintEvent");
                return false;
            }
            d->state->deviceFont = d->state->font = widget->font();
            d->state->pen = widget->palette().color(widget->foregroundRole());
            d->state->bgBrush = widget->palette().brush(widget->backgroundRole());
            d->state->ww = d->state->vw = widget->width();
            d->state->wh = d->state->vh = widget->height();
            break;
        }
        case QInternal::Pixmap:
        {
            const QPixmap *pm = static_cast<const QPixmap *>(pd);
            Q_ASSERT(pm);
            d->state->ww = d->state->vw = pm->width();
            d->state->wh = d->state->vh = pm->height();
            break;
        }
        case QInternal::ExternalDevice:
        {
            d->state->ww = d->state->vw = pd->metric(QPaintDeviceMetrics::PdmWidth);
            d->state->wh = d->state->vh = pd->metric(QPaintDeviceMetrics::PdmHeight);
        }
    }

    if (d->state->ww == 0) // For compat with 3.x painter defaults
        d->state->ww = d->state->wh = d->state->vw = d->state->vh = 1024;

    d->state->bgOrigin -= d->redirection_offset;
    d->device = pd;
    d->engine = pd->paintEngine();

    if (!d->engine) {
        qWarning("QPainter::begin(), paintdevice returned engine == 0, type: %d\n", pd->devType());
        return true;
    }

    if (!d->engine->begin(pd)) {
        qWarning("QPainter::begin(), QPaintEngine::begin() returned false\n");
        return false;
    }

    if (!d->redirection_offset.isNull())
        updateXForm();

    Q_ASSERT(d->engine->isActive());
    d->engine->clearRenderHints(QPainter::LineAntialiasing);
    d->engine->setRenderHints(QPainter::TextAntialiasing);
    ++d->device->painters;
    return true;
}

/*!
    Ends painting. Any resources used while painting are released.

    Note that while you mostly don't need to call end(), the
    destructor will do it, there is at least one common case when it
    is needed, namely double buffering.

    \code
        QPainter p(myPixmap, this)
        // ...
        p.end(); // stops drawing on myPixmap
        p.begin(this);
        p.drawPixmap(0, 0, myPixmap);
    \endcode

    Since you can't draw a QPixmap while it is being painted, it is
    necessary to close the active painter.

    \sa begin(), isActive()
*/

bool QPainter::end()
{
    if (!isActive()) {
        qWarning("QPainter::end: Painter is not active, aborted");
        return false;
    }

    if (d->states.size()>1) {
        qWarning("QPainter::end(), painter ended with %d saved states",
                 d->states.size());
    }

    bool ended = d->engine->end();
    d->engine->updateState(0);
    if (ended)
        d->engine = 0;

    --d->device->painters;
    return ended;
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
    // ### port properly
//     if (pdev && pdev->devType() == QInternal::Picture)
//         return QFontMetrics(cfont);

//     return QFontMetrics(this);
    if (d->engine)
        d->engine->updateState(d->state);
    return QFontMetrics(d->state->pfont ? *d->state->pfont : d->state->font);
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
    // ### port properly
//     if (pdev && pdev->devType() == QInternal::Picture)
//         return QFontInfo(cfont);

//     return QFontInfo(this);
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
    return d->state->bgOrigin + d->redirection_offset;
}

/*!
    \fn void QPainter::setBrushOrigin(const QPoint &p)

    \overload

    Sets the brush's origin to point \a p.
*/

/*!
    Sets the brush origin to \a (x, y).

    The brush origin specifies the (0, 0) coordinate of the painter's
    brush. This setting only applies to pattern brushes and pixmap
    brushes.

    \sa brushOrigin()
*/

void QPainter::setBrushOrigin(int x, int y)
{
    // ### updateBrush in gc probably does not deal with offset.
    d->state->bgOrigin = QPoint(x, y) - d->redirection_offset;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyBrush);
}

/*!
    Returns the current background brush.

    \sa setBackground() QBrush
*/

const QBrush &QPainter::background() const
{
    return d->state->bgBrush;
}


/*!
    Returns true if clipping has been set; otherwise returns false.

    \sa setClipping()
*/

bool QPainter::hasClipping() const
{
    return d->state->clipEnabled;
}


/*!
    Enables clipping if \a enable is true, or disables clipping if \a
    enable is false.

    \sa hasClipping(), setClipRect(), setClipRegion()
*/

void QPainter::setClipping(bool enable)
{
    Q_ASSERT(d->engine);
    d->state->clipEnabled = enable;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyClip);
}


/*!
    Returns the currently set clip region. Note that the clip region
    is given in physical device coordinates and \e not subject to any
    \link coordsys.html coordinate transformation \endlink.

    \sa setClipRegion(), setClipRect(), setClipping()
*/

QRegion QPainter::clipRegion() const
{
    return d->state->clipRegion;
}

/*!
    \fn void QPainter::setClipRect(int x, int y, int w, int h)

    Sets the clip region to the rectangle \a x, \a y, \a w, \a h and
    enables clipping.

    \sa setClipRegion(), clipRegion(), setClipping()
*/

/*!
    \overload

    Sets the clip region of the rectange \a rect.
*/
void QPainter::setClipRect(const QRect &rect) // ### inline?
{
    setClipRegion(QRegion(rect));
}

/*!
    Sets the clip region to \a r and enables clipping.

    Note that the clip region is given in physical device coordinates
    and \e not subject to any \link coordsys.html coordinate
    transformation.\endlink

    \sa setClipRect(), clipRegion(), setClipping()
*/

void QPainter::setClipRegion(const QRegion &r)
{
    if (!isActive()) {
        qWarning("QPainter::setClipRegion(); painter not active");
        return;
    }
    d->state->clipRegion = d->state->matrix * r;
    d->state->clipEnabled = true;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyClip);
}


/*!
    Returns true if view transformation is enabled; otherwise returns
    false.

    \sa setViewXForm(), xForm()
*/

bool QPainter::hasViewXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->VxF;
#else
    return d->state->xlatex || d->state->xlatey;
#endif
}

/*!
    Returns true if world transformation is enabled; otherwise returns
    false.

    \sa setWorldXForm()
*/

bool QPainter::hasWorldXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->WxF;
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
    Sets the window rectangle view transformation for the painter and
    enables view transformation.

    The window rectangle is part of the view transformation. The
    window specifies the logical coordinate system and is specified by
    the \a x, \a y, \a w width and \a h height parameters. Its sister,
    the viewport(), specifies the device coordinate system.

    The default window rectangle is the same as the device's
    rectangle. See the \link coordsys.html Coordinate System Overview
    \endlink for an overview of coordinate transformation.

    \sa window(), setViewport(), setViewXForm(), setWorldMatrix(),
    setWorldXForm()
*/

void QPainter::setWindow(int x, int y, int w, int h)
{
    if (!isActive())
        qWarning("QPainter::setWindow: Will be reset by begin()");
    d->state->wx = x;
    d->state->wy = y;
    d->state->ww = w;
    d->state->wh = h;
    if (d->state->VxF)
        updateXForm();
    else
        setViewXForm(true);
}

/*!
    Returns the window rectangle.

    \sa setWindow(), setViewXForm()
*/

QRect QPainter::window() const
{
    return QRect(d->state->wx, d->state->wy, d->state->ww, d->state->wh);
}

/*!
    \fn void QPainter::setViewport(const QRect &r)

    \overload

    Sets the painter's viewport rectangle to \a r.
*/

/*!
    Sets the viewport rectangle view transformation for the painter
    and enables view transformation.

    The viewport rectangle is part of the view transformation. The
    viewport specifies the device coordinate system and is specified
    by the \a x, \a y, \a w width and \a h height parameters. Its
    sister, the window(), specifies the logical coordinate system.

    The default viewport rectangle is the same as the device's
    rectangle. See the \link coordsys.html Coordinate System Overview
    \endlink for an overview of coordinate transformation.

    \sa viewport(), setWindow(), setViewXForm(), setWorldMatrix(),
    setWorldXForm(), xForm()
*/

void QPainter::setViewport(int x, int y, int w, int h)
{
    if (!isActive())
        qWarning("QPainter::setViewport: Will be reset by begin()");
    d->state->vx = x;
    d->state->vy = y;
    d->state->vw = w;
    d->state->vh = h;
    if (d->state->VxF)
        updateXForm();
    else
        setViewXForm(true);
}

/*!
    Returns the viewport rectangle.

    \sa setViewport(), setViewXForm()
*/

QRect QPainter::viewport() const
{
    return QRect(d->state->vx, d->state->vy, d->state->vw, d->state->vh);
}

/*!
    Enables view transformations if \a enable is true, or disables
    view transformations if \a enable is false.

    \sa hasViewXForm(), setWindow(), setViewport(), setWorldMatrix(),
    setWorldXForm(), xForm()
*/

void QPainter::setViewXForm(bool enable)
{
    if (!isActive())
        qWarning("QPainter::setViewXForm: Will be reset by begin()");
    if (!isActive() || enable == d->state->VxF)
        return;
    d->state->VxF = enable;
    updateXForm();
}

/*!
    Sets the world transformation matrix to \a wm and enables world
    transformation.

    If \a combine is true, then \a wm is combined with the current
    transformation matrix; otherwise \a wm replaces the current
    transformation matrix.

    If \a wm is the identity matrix and \a combine is false, this
    function calls setWorldXForm(false). (The identity matrix is the
    matrix where QWMatrix::m11() and QWMatrix::m22() are 1.0 and the
    rest are 0.0.)

    World transformations are applied after the view transformations
    (i.e. \link setWindow() window\endlink and \link setViewport()
    viewport\endlink).

    The following functions can transform the coordinate system without using
    a QWMatrix:
    \list
    \i translate()
    \i scale()
    \i shear()
    \i rotate()
    \endlist

    They operate on the painter's worldMatrix() and are implemented like this:

    \code
        void QPainter::rotate(double a)
        {
            QWMatrix m;
            m.rotate(a);
            setWorldMatrix(m, true);
        }
    \endcode

    Note that you should always use \a combine when you are drawing
    into a QPicture. Otherwise it may not be possible to replay the
    picture with additional transformations. Using translate(),
    scale(), etc., is safe.

    For a brief overview of coordinate transformation, see the \link
    coordsys.html Coordinate System Overview. \endlink

    \sa worldMatrix() setWorldXForm() setWindow() setViewport()
    setViewXForm() xForm() QWMatrix
*/

void QPainter::setWorldMatrix(const QWMatrix &wm, bool combine)
{
    if (!isActive())
        qWarning("QPainter::setWorldMatrix: Will be reset by begin()");
    if (combine)
        d->state->worldMatrix = wm * d->state->worldMatrix;                        // combines
    else
        d->state->worldMatrix = wm;                                // set new matrix
//     bool identity = d->state->worldMatrix.isIdentity();
//     if (identity && pdev->devType() != QInternal::Picture)
//         setWorldXForm(false);
//     else
    if (!d->state->WxF)
        setWorldXForm(true);
    else
        updateXForm();
}

/*!
    Returns the world transformation matrix.

    \sa setWorldMatrix()
*/

const QWMatrix &QPainter::worldMatrix() const
{
    return d->state->worldMatrix;
}

/*!
    Enables world transformations if \a enable is true, or disables
    world transformations if \a enable is false. The world
    transformation matrix is not changed.

    \sa setWorldMatrix(), setWindow(), setViewport(), setViewXForm(),
    xForm()
*/

void QPainter::setWorldXForm(bool enable)
{
    if (!isActive())
        qWarning("QPainter::setWorldXForm: Will be reset by begin()");
    if (!isActive() || enable == d->state->WxF)
        return;
    d->state->WxF = enable;
    updateXForm();
}

#ifndef QT_NO_TRANSFORMATIONS
/*!
    Scales the coordinate system by \a (sx, sy).

    \sa translate(), shear(), rotate(), resetXForm(), setWorldMatrix(),
    xForm()
*/

void QPainter::scale(double sx, double sy)
{
    QWMatrix m;
    m.scale(sx, sy);
    setWorldMatrix(m, true);
}

/*!
    Shears the coordinate system by \a (sh, sv).

    \sa translate(), scale(), rotate(), resetXForm(), setWorldMatrix(),
    xForm()
*/

void QPainter::shear(double sh, double sv)
{
    QWMatrix m;
    m.shear(sv, sh);
    setWorldMatrix(m, true);
}

/*!
    Rotates the coordinate system \a a degrees counterclockwise.

    \sa translate(), scale(), shear(), resetXForm(), setWorldMatrix(),
    xForm()
*/

void QPainter::rotate(double a)
{
    QWMatrix m;
    m.rotate(a);
    setWorldMatrix(m, true);
}
#endif

/*!
    Resets any transformations that were made using translate(), scale(),
    shear(), rotate(), setWorldMatrix(), setViewport() and
    setWindow().

    \sa worldMatrix(), viewport(), window()
*/

void QPainter::resetXForm()
{
    if (!isActive())
        return;
    d->state->wx = d->state->wy = d->state->vx = d->state->vy = 0;                        // default view origins
    d->state->ww = d->state->vw = d->device->metric(QPaintDeviceMetrics::PdmWidth);
    d->state->wh = d->state->vh = d->device->metric(QPaintDeviceMetrics::PdmHeight);
    d->state->worldMatrix = QWMatrix();
    setWorldXForm(false);
    setViewXForm(false);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyTransform);
}

/*!
    \fn void QPainter::setClipPath(const QPainterPath &path)

    \internal

    Sets the painter's clipping path to \a path.
*/

/*!
    \fn void QPainter::translate(const QPoint &offset)

    \overload

    Translates the coordinate system by the given \a offset.
*/

/*!
    Translates the coordinate system by \a (dx, dy). After this call,
    \a (dx, dy) is added to points.

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

    \sa scale(), shear(), rotate(), resetXForm(), setWorldMatrix(), xForm()
*/

void QPainter::translate(double dx, double dy)
{
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix m;
    m.translate(dx, dy);
    setWorldMatrix(m, true);
#else
    xlatex += (int)dx;
    xlatey += (int)dy;
    d->state->VxF = (bool)xlatex || xlatey;
#endif
}

double QPainter::translationX() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->worldMatrix.dx();
#else
    return d->state->xlatex;
#endif
}

double QPainter::translationY() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->worldMatrix.dy();
#else
    return d->state->xlatey;
#endif
}


/*!
    \internal
*/
void QPainter::drawPath(const QPainterPath &path)
{
    if (!isActive())
	return;

    d->engine->updateState(d->state);

    if (d->engine->hasFeature(QPaintEngine::PainterPaths)) {
        if (!(d->state->VxF || d->state->WxF) || d->engine->hasFeature(QPaintEngine::CoordTransform)) {
            d->engine->drawPath(path);
            return;
        }
    }

    QPainterPathPrivate *pd = path.d_ptr;
    QList<QPointArray> polygons;
    for (int i=0; i<pd->subpaths.size(); ++i) {
	polygons.append(pd->subpaths.at(i).toPolygon());
    }

    if (polygons.isEmpty())
        return;

    // Fill the path...
    if (d->state->brush.style() != Qt::NoBrush) {
	save();
	setPen(Qt::NoPen);
        if (path.fillMode() == QPainterPath::Winding) {
            for (int i=0; i<polygons.size(); ++i)
                drawPolygon(polygons.at(i), true);
        } else {

        }
	restore();
    }

    // Draw the outline of the path...
    if (d->state->pen.style() != Qt::NoPen) {
        if (pd->subpaths.size() > 0) {
            save();
            setBrush(Qt::NoBrush);
        }
	for (int i=0; i<polygons.size(); ++i)
	    drawPolyline(polygons.at(i));
        if (pd->subpaths.size() > 0)
            restore();
    }
}


/*!
    \fn void QPainter::drawLine(int x1, int y1, int x2, int y2)
    \overload

    Draws a line from (\a x1, \a y1) to (\a x2, \a y2) and sets the
    current pen position to (\a x2, \a y2).
*/

/*!
    Draws a line from point \a p1 to point \a p2.

    \sa pen()
*/

void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (!isActive())
        return;

    d->engine->updateState(d->state);

    if ((d->state->WxF || d->state->VxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        d->engine->drawLine(xForm(p1), xForm(p2));
        return;
    }

    d->engine->drawLine(p1, p2);
}

/*!
    \fn void QPainter::drawRect(int x, int y, int w, int h)

    \overload

    Draws a rectangle with upper left corner at \a (x, y) and with
    width \a w and height \a h.
*/

/*!
    Draws the rectangle \a r.

    \sa QPen, drawRoundRect()
*/
void QPainter::drawRect(const QRect &r)
{
    QRect rect = r.normalize();

    if (!isActive() || rect.isEmpty())
        return;
    d->engine->updateState(d->state);

    if (d->state->brush.style() == Qt::LinearGradientPattern
        && !d->engine->hasFeature(QPaintEngine::LinearGradients)) {
        bool doRestore = true;
        if (r.x() == 0 && r.y() == 0
            && r.width() == d->device->metric(QPaintDeviceMetrics::PdmWidth)
            && r.height() == d->device->metric(QPaintDeviceMetrics::PdmHeight)) {
            doRestore = false;
        } else {
            save();
            setClipRect(r);
        }
        qt_fill_linear_gradient(r, this, d->state->brush);
        if (d->state->pen.style() != Qt::NoPen) {
            QBrush oldBrush = d->state->brush;
            setBrush(NoBrush);
            drawRect(r);
            setBrush(oldBrush);
        }
        if (doRestore)
            restore();
        return;
    }

    if (d->state->brush.style() == Qt::SolidPattern
	&& d->state->brush.color().alpha() != 255
	&& !d->engine->hasFeature(QPaintEngine::SolidAlphaFill)) {
	const int BUFFERSIZE = 16;
	QImage image(BUFFERSIZE, BUFFERSIZE, 32);
	image.fill(d->state->brush.color().rgb());
	image.setAlphaBuffer(true);
	QPixmap pm(image);
	drawTiledPixmap(r, pm);
	if (d->state->pen.style() != Qt::NoPen) {
	    save();
	    setBrush(Qt::NoBrush);
	    drawRect(r);
	    restore();
	}
	return;
    }

    if ((d->state->VxF || d->state->WxF)
        && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        if (d->state->txop == TxRotShear) {
  	    drawPolygon(QPointArray(rect));
#if 0 // NB! keep this golden magic nugget of code
	    QPointArray tr = d->state->matrix * QPointArray(QRect(QPoint(0,0), rect.size()));
	    QRect br = tr.boundingRect();
	    QBitmap bm(br.size());
	    bm.fill(Qt::color0);
 	    QPainter pt(&bm);
	    pt.translate(-br.x(), -br.y());
 	    pt.setBrush(Qt::color1);
	    pt.drawPolygon(tr);
	    pt.end();
	    save();
	    resetXForm();
	    QPoint p = d->state->matrix * rect.topLeft();
	    translate(p.x() + br.left(), p.y() + br.top());
 	    setClipRegion(bm);
  	    drawRect(0, 0, br.width(), br.height());
	    restore();
#endif // 0
            return;
        }
        rect = xForm(rect);
    }

    d->engine->drawRect(rect);
}

/*!
    Draws all the rectangles in the \a rects list using the current
    pen and brush.

    \sa drawRect()
*/
void QPainter::drawRects(const QList<QRect> &rects)
{
    if (!isActive())
        return;

    d->engine->updateState(d->state);

    if ((!d->engine->hasFeature(QPaintEngine::DrawRects)
         || !d->engine->hasFeature(QPaintEngine::LinearGradients)
         || ((d->state->VxF || d->state->WxF)
             && !d->engine->hasFeature(QPaintEngine::CoordTransform)))

        ) {
        for (int i=0; i<rects.size(); ++i)
            drawRect(rects.at(i));
        return;
    }

    d->engine->drawRects(rects);
}

/*!
    Draws a single point at position \a p using the current pen's color.

    \sa QPen
*/
void QPainter::drawPoint(const QPoint &p)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if ((d->state->VxF || d->state->WxF)
        && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        d->engine->drawPoint(xForm(p));
        return;
    }

    d->engine->drawPoint(p);
}

/*! \fn void QPainter::drawPoint(int x, int y)

    \overload

    Draws a single point at position (\a x, \a y).
*/

/*!
    Draws the array of points \a pa using the current pen's color.

    If \a index is non-zero (the default is zero), only points
    starting from \a index are drawn. If \a npoints is negative (the
    default) the rest of the points from \a index are drawn. If \a
    npoints is zero or greater, \a npoints points are drawn.

    \sa QPen
*/
void QPainter::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (npoints < 0)
        npoints = pa.size() - index;
    if (index + npoints > (int)pa.size())
        npoints = pa.size() - index;
    if (!isActive() || npoints < 1 || index < 0)
        return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        QPointArray a = xForm(pa, index, npoints);
        index = 0;
        npoints = a.size();
        d->engine->drawPoints(a, index, npoints);
        return;
    }

    d->engine->drawPoints(pa, index, npoints);
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
    if (mode != Qt::TransparentMode && mode != Qt::OpaqueMode) {
        qWarning("QPainter::setBackgroundMode: Invalid mode");
        return;
    }
    d->state->bgMode = mode;
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyBackground);
}

/*!
    Returns the current background mode.

    \sa setBackgroundMode() Qt::BGMode
*/
QPainter::BGMode QPainter::backgroundMode() const
{
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
    d->state->pen = QPen(color, 0, Qt::SolidLine);
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
    if (d->state->pen.style() == style)
        return;
    d->state->pen.setStyle(style);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyPen);
}

/*!
    Returns the painter's current pen.

    \sa setPen()
*/

const QPen &QPainter::pen() const
{
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
    if (d->state->brush == brush)
        return;
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
    // ### Missing optimization from qpainter.cpp
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
    d->state->font = font.resolve(d->state->deviceFont);
    if (d->engine)
        d->engine->setDirty(QPaintEngine::DirtyFont);
}

/*!
    Returns the currently set painter font.

    \sa setFont(), QFont
*/

const QFont &QPainter::font() const
{
    if (d->engine)
        d->engine->updateState(d->state);
    return d->state->pfont ? *d->state->pfont : d->state->font;
}
/*! \fn void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)

    \overload

    Draws a rectangle with rounded corners at \a (x, y), with width \a
    w and height \a h.

    The \a xRnd and \a yRnd arguments specify how rounded the corners
    should be. 0 is angled corners, 99 is maximum roundedness.

    The width and height include all of the drawn lines.

    \sa drawRect(), QPen
*/

/*!
    Draws a rectangle \a r with rounded corners.

    The \a xRnd and \a yRnd arguments specify how rounded the corners
    should be. 0 is angled corners, 99 is maximum roundedness.

    The width and height include all of the drawn lines.

    \sa drawRect(), QPen
*/

void QPainter::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
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

    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if (d->state->brush.style() == Qt::LinearGradientPattern
        && !d->engine->hasFeature(QPaintEngine::LinearGradients)) {
        QT_FILL_GRADIENT(rect,
                         drawRoundRect(0, 0, r.width(), r.height(), xRnd, yRnd),
                         drawRoundRect(r, xRnd, yRnd));

    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        if (d->state->txop == TxRotShear) {
            int x = rect.x();
            int y = rect.y();
            int w = rect.width();
            int h = rect.height();

            w--; // ###?
            h--; // ###?
            int rxx = w*xRnd/200;
            int ryy = h*yRnd/200;
            // were there overflows?
            if (rxx < 0)
                rxx = w/200*xRnd;
            if (ryy < 0)
                ryy = h/200*yRnd;
            int rxx2 = 2*rxx;
            int ryy2 = 2*ryy;
            QPointArray a[4];
            a[0].makeArc(x, y, rxx2, ryy2, 1*16*90, 16*90, d->state->matrix);
            a[1].makeArc(x, y+h-ryy2, rxx2, ryy2, 2*16*90, 16*90, d->state->matrix);
            a[2].makeArc(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*16*90, 16*90, d->state->matrix);
            a[3].makeArc(x+w-rxx2, y, rxx2, ryy2, 0*16*90, 16*90, d->state->matrix);
            // ### is there a better way to join QPointArrays?
            QPointArray aa;
            aa.resize(a[0].size() + a[1].size() + a[2].size() + a[3].size());
            uint j = 0;
            for (int k=0; k<4; k++) {
                for (int i=0; i<a[k].size(); i++) {
                    aa.setPoint(j, a[k].point(i));
                    j++;
                }
            }
            d->engine->drawPolygon(aa, false, 0, aa.size());
            return;
        }
        rect = xForm(rect);
    }
    d->engine->drawRoundRect(rect, xRnd, yRnd);
}

/*! \fn void QPainter::drawEllipse(int x, int y, int w, int h)

    \overload

    Draws an ellipse with center at \a (x + w/2, y + h/2) and size \a
    (w, h).
*/

/*!
    Draws the ellipse that fits inside rectangle \a r.
*/

void QPainter::drawEllipse(const QRect &r)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if (d->state->brush.style() == Qt::LinearGradientPattern
        && !d->engine->hasFeature(QPaintEngine::LinearGradients)) {
        QT_FILL_GRADIENT(rect,
                         drawEllipse(0, 0, rect.width(), rect.height()),
                         drawEllipse(rect));

    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        if (d->state->txop == TxRotShear) {
            QPointArray a;
            a.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), 0, 360*16, d->state->matrix);
            d->engine->drawPolygon(a, false, 0, a.size());
            return;
        }
        rect = xForm(rect);
    }

    d->engine->drawEllipse(rect);
}

/*! \fn void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)

    \overload

    Draws the arc that fits inside the rectangle \a (x, y, w, h), with
    start angle \a a and arc length \a alen.
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

void QPainter::drawArc(const QRect &r, int a, int alen)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        if (d->state->txop == TxRotShear) {
            QPointArray pa;
            pa.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), a, alen, d->state->matrix);
            d->engine->drawPolyline(pa, 0, pa.size());
            return;
        }
        rect = xForm(rect);
    }
    d->engine->drawArc(rect, a, alen);
}


/*!
    \fn void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
    \overload

    Draws a pie segment that fits inside the rectangle \a (x, y, w, h)
    with start angle \a a and arc length \a alen.
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
void QPainter::drawPie(const QRect &r, int a, int alen)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (a > (360*16)) {
        a = a % (360*16);
    } else if (a < 0) {
        a = a % (360*16);
        if (a < 0) a += (360*16);
    }

    QRect rect = r.normalize();

    if (d->state->brush.style() == Qt::LinearGradientPattern
        && !d->engine->hasFeature(QPaintEngine::LinearGradients)) {
        QT_FILL_GRADIENT(rect,
                         drawPie(0, 0, rect.width(), rect.height(), a, alen),
                         drawPie(rect, a, alen));

    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        if (d->state->txop == TxRotShear) {                // rotate/shear
            // arc polyline
            QPointArray pa;
            pa.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), a, alen, d->state->matrix);
            int n = pa.size();
            QPoint p = xForm(QPoint(r.x()+r.width()/2, r.y()+r.height()/2));
            pa.resize(n+2);
            pa.setPoint(n, p);        // add legs
            pa.setPoint(n+1, pa.at(0));
            d->engine->drawPolygon(pa, false, 0, pa.size());
            return;
        }
        rect = xForm(rect);
    }
    d->engine->drawPie(rect, a, alen);
}

/*!
    \fn void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)

    \overload

    Draws a chord that fits inside the rectangle \a (x, y, w, h) with
    start angle \a a and arc length \a alen.
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

void QPainter::drawChord(const QRect &r, int a, int alen)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if (d->state->brush.style() == Qt::LinearGradientPattern
        && !d->engine->hasFeature(QPaintEngine::LinearGradients)) {
        QT_FILL_GRADIENT(rect,
                         drawChord(0, 0, rect.width(), rect.height(), a, alen),
                         drawChord(rect, a, alen));

    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        if (d->state->txop == TxRotShear) {                // rotate/shear
            QPointArray pa;
            pa.makeArc(rect.x(), rect.y(), rect.width()-1, rect.height()-1, a, alen, d->state->matrix); // arc polygon
            int n = pa.size();
            pa.resize(n+1);
            pa.setPoint(n, pa.at(0));                // connect endpoints
            d->engine->drawPolygon(pa, false, 0, pa.size());
            return;
        }
        rect = xForm(rect);
    }
    d->engine->drawChord(rect, a, alen);
}

/*!
    Draws \a nlines separate lines from points defined in \a a,
    starting at \a a[index] (\a index defaults to 0). If \a nlines is
    -1 (the default) all points until the end of the array are used
    (i.e. (a.size()-index)/2 lines are drawn).

    Draws the 1st line from \a a[index] to \a a[index+1]. Draws the
    2nd line from \a a[index+2] to \a a[index+3] etc.

    \sa drawPolyline(), drawPolygon(), QPen
*/

void QPainter::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (nlines < 0)
        nlines = a.size()/2 - index/2;
    if (index + nlines*2 > (int)a.size())
        nlines = (a.size() - index)/2;
    if (!isActive() || nlines < 1 || index < 0)
        return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        QPointArray pa = xForm(a, index, nlines*2);
        if (pa.size() != a.size()) {
            index  = 0;
            nlines = pa.size()/2;
        }
        d->engine->drawLineSegments(pa, index, nlines);
        return;
    }

    d->engine->drawLineSegments(a, index, nlines);
}

/*!
    Draws the polyline defined by the \a npoints points in \a a
    starting at \a a[index]. (\a index defaults to 0.)

    If \a npoints is -1 (the default) all points until the end of the
    array are used (i.e. a.size()-index-1 line segments are drawn).

    \sa drawLineSegments(), drawPolygon(), QPen
*/

void QPainter::drawPolyline(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (a.isEmpty())
	return;

    if (npoints < 0)
        npoints = a.size() - index;
    if (index + npoints > (int)a.size())
        npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
        return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        QPointArray ar = xForm(a, index, npoints);
        d->engine->drawPolyline(ar, index, npoints);
        return;
    }

    d->engine->drawPolyline(a, index, npoints);
}

/*!
    Draws the polygon defined by the \a npoints points in \a a
    starting at \a a[index]. (\a index defaults to 0.)

    If \a npoints is -1 (the default) all points until the end of the
    array are used (i.e. a.size()-index line segments define the
    polygon).

    The first point is always connected to the last point.

    The polygon is filled with the current brush(). If \a winding is
    true, the polygon is filled using the winding fill algorithm. If
    \a winding is false, the polygon is filled using the even-odd
    (alternative) fill algorithm.

    \sa drawLineSegments(), drawPolyline(), QPen
*/

void QPainter::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (a.isEmpty())
	return;

    if (npoints < 0)
        npoints = a.size() - index;
    if (index + npoints > (int)a.size())
        npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
        return;

    if (d->state->brush.style() == Qt::LinearGradientPattern
        && !d->engine->hasFeature(QPaintEngine::LinearGradients)) {
        QRect bounds = a.boundingRect();
        QPointArray copy(a);
        copy.translate(-bounds.x(), -bounds.y());
        QT_FILL_GRADIENT(bounds,
                         drawPolygon(copy, winding, index, npoints),
                         drawPolygon(a, winding, index, npoints));
    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        QPointArray ar = xForm(a, index, npoints);
        d->engine->drawPolygon(ar, winding, index, npoints);
        return;
    }
    d->engine->drawPolygon(a, winding, index, npoints);
}

/*!
    Draws the convex polygon defined by the \a npoints points in \a a
    starting at \a a[index] (\a index defaults to 0).

    If the supplied polygon is not convex, the results are undefined.

    On some platforms (e.g. X Window), this is faster than
    drawPolygon().
*/

void QPainter::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (npoints < 0)
        npoints = a.size() - index;
    if (index + npoints > (int)a.size())
        npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
        return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        QPointArray ar = xForm(a, index, npoints);
        d->engine->drawConvexPolygon(ar, index, npoints);
        return;
    }
    d->engine->drawConvexPolygon(a, index, npoints);
}

/*!
    Draws a cubic Bezier curve defined by the control points in \a a,
    starting at \a a[index] (\a index defaults to 0).

    Control points after \a a[index + 3] are ignored. Nothing happens
    if there aren't enough control points.
*/

void QPainter::drawCubicBezier(const QPointArray &a, int index)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if ((int)a.size() - index < 4) {
        qWarning("QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
                  "points");
        return;
    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::CoordTransform)) {
        QPointArray pa = xForm(a, index, a.size()-index);
        d->engine->drawCubicBezier(pa, index);
        return;
    }

    d->engine->drawCubicBezier(a, index);
}

/*!
    \fn void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                  int sx, int sy, int sw, int sh, bool imask)

    \overload

    Draws the rectangular portion with the origin \a(sx, sy), width \a sw
    and height \a sh, of the pixmap \a pm, at the point \a(x, y), with a
    width of \a w and a height of \a h. If \a imask is true \a pm will not
    be masked to QPixmap::mask()
*/

/*! \fn void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh, bool imask)

    \overload

    Draws a pixmap at \a (x, y) by copying a part of \a pixmap into
    the paint device.

    \a (x, y) specifies the top-left point in the paint device that is
    to be drawn onto. \a (sx, sy) specifies the top-left point in \a
    pixmap that is to be drawn. The default is (0, 0).

    \a (sw, sh) specifies the size of the pixmap that is to be drawn.
    The default, (-1, -1), means all the way to the bottom right of
    the pixmap.

    If \a imask is true the \a pixmap will not be masked to
    QPixmap::mask(). Currently when painting on a QPrinter \a imask is
    always true.

    \sa QPixmap::setMask()
*/

/*!
    \fn void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr, bool imask)
    \overload

    Draws the rectangle \a sr of pixmap \a pm with its origin at point \a
    p. If \a imask is true \a pm will not be masked to QPixmap::mask().
*/

/*!
    \fn void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, bool imask)
    \overload

    Draws the pixmap \a pm with its origin at point \a p. If \a imask is
    true \a pm will not be masked to QPixmap::mask().
*/

/*!
    Draws the rectanglular portion \a sr, of pixmap \a pm, into rectangle
    \a r in the paint device. If \a imask is true \a pm will not be masked
    to QPixmap::mask().
*/
void QPainter::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, bool imask)
{
    if (!isActive() || pm.isNull())
        return;
    d->engine->updateState(d->state);

    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    int sx = sr.x();
    int sy = sr.y();
    int sw = sr.width();
    int sh = sr.height();

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

    if (((d->state->VxF || d->state->WxF) && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) ||
        ((w != sw || h != sh) && !d->engine->hasFeature(QPaintEngine::PixmapScale))) {
        QPixmap source(sw, sh);
        {
            QPainter p(&source);
            p.drawPixmap(QRect(0, 0, sw, sh), pm, QRect(sx, sy, sw, sh), imask);
        }

        QWMatrix mat(d->state->matrix);
        double scalex = w / (double)sw;
        double scaley = h / (double)sh;
        mat = QWMatrix(scalex, 0, 0, scaley, 0, 0) * mat;
        mat = QPixmap::trueMatrix(mat, sw, sh);
        QPixmap pmx;
        if (sx == 0 && sy == 0 &&
            sw == pm.width() && sh == pm.height()) {
            pmx = pm;                        // xform the whole pixmap
        } else {
            QPainter p;
            pmx = QPixmap(sw, sh);                // xform subpixmap
            p.begin(&pmx);
            p.drawPixmap(QRect(0, 0, sw, sh), pm, QRect(sx, sy, sw, sh), imask);
            p.end();
            if (!imask && pm.mask()) {
                QBitmap mask(sw, sh);
                p.begin(&mask);
                p.drawPixmap(QRect(0, 0, sw, sh), *pm.mask(), QRect(sx, sy, sw, sh), true);
                p.end();
                pmx.setMask(mask);
            }
        }
        pmx = pmx.xForm(mat);
        if (pmx.isNull())                        // xformed into nothing
            return;
        if (!pmx.mask() && d->state->txop == TxRotShear) {
            QBitmap bm_clip(sw, sh, 1);        // make full mask, xform it
            bm_clip.fill(Qt::color1);
            pmx.setMask(bm_clip.xForm(mat));
        }
        map(x, y, &x, &y);        // compute position of pixmap
        int dx, dy;
        mat.map(0, 0, &dx, &dy);
        d->engine->drawPixmap(QRect(x-dx, y-dy, pmx.width(), pmx.height()), pmx,
                        QRect(0, 0, pmx.width(), pmx.height()), imask);
        return;
    }

    d->engine->drawPixmap(QRect(x, y, w, h), pm, QRect(sx, sy, sw, sh), imask);
    return;
}

/*!
    \fn void QPainter::drawImage(const QPoint &p, const QImage &i,
                                 const QRect &sr, int conversionFlags)

    \overload

    Draws the rectangular portion specified by \a sr, of image \a i,
    at point \a p. The \a conversionFlags signify how the drawing is
    to be done.

    \sa Qt::ImageConversionFlags
*/

/*!
    \fn void QPainter::drawImage(const QPoint &p, const QImage &i,
                                 int conversionFlags)

    \overload

    Draws the image \a i, at point \a p. The \a conversionFlags
    signify how the drawing is to be done.

    \sa Qt::ImageConversionFlags
*/

/*!
  \internal
  ### remove when combining QImage/QPixmap
*/

void QPainter::drawImage(int, int, const QImage &, int, int, int, int, int)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}


/*!
  \internal
  ### remove when combining QImage/QPixmap
*/
void QPainter::drawImage(const QRect &, const QImage &)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}

/*!
    \fn void QPainter::drawText(int x, int y, int w, int h, int flags,
                                const QString &str, int len, QRect *br)

    \overload

    Draws the string \a str within the rectangle with origin \a(x, y),
    width \a w and height \a h. If \a len is -1 (the default) all the
    text is drawn, otherwise only the first \a len characters are
    drawn. The flags that are given in the \a flags parameter are
    \l{Qt::AlignmentFlags} and \l{Qt::TextFlags} OR'd together. \a br
    (if not null) is set to the actual bounding rectangle of the
    output.
*/

/*!

    \fn void QPainter::drawText(const QPoint &p, const QString &str,
                                TextDirection dir)

    \overload

    Draws the string \a str at point \a p. The text's
    direction is given by \a dir.

    \sa QPainter::TextDirection
*/

/*!
    Draws the string \a str at position \a x, \a y. The text's
    direction is given by \a dir.

    \sa QPainter::TextDirection
*/

void QPainter::drawText(int x, int y, const QString &str, TextDirection dir)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (str.isEmpty())
        return;

    QTextLayout layout(str, d->state->pfont ? *d->state->pfont : d->state->font);
    QTextEngine *engine = layout.engine();

    engine->itemize(QTextEngine::SingleLine);

    if (dir != Auto) {
        int level = dir == RTL ? 1 : 0;
        for (int i = engine->items.size()-1; i >= 0; i--)
            engine->items[i].analysis.bidiLevel = level;
    }

    QTextLine line = layout.createLine();
    line.layout(0x01000000);
    const QScriptLine &sl = engine->lines[0];

    int textFlags = 0;
    if (d->state->font.underline()) textFlags |= Qt::Underline;
    if (d->state->font.overline()) textFlags |= Qt::Overline;
    if (d->state->font.strikeOut()) textFlags |= Qt::StrikeOut;

    // ### call fill rect here...
#if defined(Q_WS_X11)
    extern void qt_draw_background(QPaintEngine *pe, int x, int y, int w,  int h);
    if (backgroundMode() == Qt::OpaqueMode)
        qt_draw_background(d->engine, x, y-sl.ascent.toInt(), sl.textWidth.toInt(), (sl.ascent+sl.descent).toInt() + 1);
#endif

    line.draw(this, x, y - sl.ascent.toInt());
}

/*!
    \overload

    Draws the string \a str within the rectangle \a r. If \a len is -1
    (the default) all the text is drawn, otherwise only the first \a
    len characters are drawn. The flags that are given in the \a flags
    parameter are \l{Qt::AlignmentFlags} and \l{Qt::TextFlags} OR'd
    together. \a br (if not null) is set to the actual bounding
    rectangle of the output.
*/
void QPainter::drawText(const QRect &r, int flags, const QString &str, int len, QRect *br)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (len < 0)
        len = str.length();
    if (len == 0)                                // empty string
        return;

    qt_format_text(font(), r, flags, str, len, br, 0, 0, 0, this);
}

/*!
    \fn void QPainter::drawTextItem(int x, int y, const QTextItem &ti,
                                    int textflags)

    \internal
    \overload
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

void QPainter::drawTextItem(const QPoint &p, const QTextItem &ti, int textFlags)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);
    d->engine->drawTextItem(p, ti, textFlags);
}

/*!
    \fn void QPainter::map(int x, int y, int *rx, int *ry) const

    \internal

    Sets \a(rx, ry) to the point that results from applying the
    painter's current transformation on the point \a(x, y).
*/

/*!
    \fn QRect QPainter::boundingRect(const QRect &r, int flags,
                                     const QString &str, int len)

    \overload

    Returns the bounding rectangle constrained by rectangle \a r.
*/

/*!
    \fn QRect QPainter::boundingRect(int x, int y, int w, int h, int flags,
                                     const QString &str, int len = -1);

    Returns the bounding rectangle of the aligned text that would be
    printed with the corresponding drawText() function using the first
    \a len characters of the string \a str, if \a len is > -1, or the
    whole of the string if \a len is -1. The drawing, and hence the
    bounding rectangle, is constrained to the rectangle that begins at
    point \a (x, y) with width \a w and height \a h, or to the
    rectangle required to draw the text, whichever is the larger.

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
    \row \i \c Qt::SingleLine \i ignores newline characters in the text.
    \row \i \c Qt::ExpandTabs \i expands tabs.
    \row \i \c Qt::ShowPrefix \i interprets "&x" as "<u>x</u>".
    \row \i \c Qt::WordBreak \i breaks the text to fit the rectangle.
    \endtable

    Qt::Horizontal alignment defaults to \c Qt::AlignLeft and vertical
    alignment defaults to \c Qt::AlignTop.

    If several of the horizontal or several of the vertical alignment flags
    are set, the resulting alignment is undefined.

    \sa Qt::TextFlags
*/

QRect QPainter::boundingRect(int x, int y, int w, int h, int flags, const QString &str, int len)
{
    QRect brect;
    if (str.isEmpty())
        brect.setRect(x,y, 0,0);
    else
        drawText(QRect(x, y, w, h), flags | DontPrint, str, len, &brect);
    return brect;
}

/*!\internal internal, used by drawTiledPixmap */
void qt_draw_tile(QPaintEngine *gc, int x, int y, int w, int h,
                  const QPixmap &pixmap, int xOffset, int yOffset)
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
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
            gc->drawPixmap(QRect(xPos, yPos, drawW, drawH), pixmap, QRect(xOff, yOff, drawW, drawH), true);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}

/*!
    \fn void QPainter::drawTiledPixmap(const QRect &r, const QPixmap &pm, const QPoint &sp)
    \overload

    Draws a tiled pixmap, \a pm, inside rectangle \a r with its origin
    at point \a sp.
*/

/*!
    \fn void QPainter::drawTiledPixmap(const QRect &r, const QPixmap &pm)
    \overload

    Draws a tiled pixmap, \a pm, inside rectangle \a r.
*/

/*!
    Draws a tiled \a pixmap in the specified rectangle.

    \a (x, y) specifies the top-left point in the paint device that is
    to be drawn onto; with the width and height given by \a w and \a
    h. \a (sx, sy) specifies the top-left point in \a pixmap that is
    to be drawn. The default is (0, 0).

    Calling drawTiledPixmap() is similar to calling drawPixmap()
    several times to fill (tile) an area with a pixmap, but is
    potentially much more efficient depending on the underlying window
    system.

    \sa drawPixmap()
*/

void QPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx, int sy)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    int sw = pixmap.width();
    int sh = pixmap.height();
    if (!sw || !sh)
        return;
    if (sx < 0)
        sx = sw - -sx % sw;
    else
        sx = sx % sw;
    if (sy < 0)
        sy = sh - -sy % sh;
    else
        sy = sy % sh;

    if ((d->state->VxF || d->state->WxF)
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
        QPixmap pm(w, h);
        QPainter p(&pm);
        // Recursive call ok, since the pixmap is not transformed...
        p.drawTiledPixmap(0, 0, w, h, pixmap, sx, sy);
        p.end();
        drawPixmap(x, y, pm);
        return;
    }

    bool optim = (pixmap.mask() && pixmap.depth() > 1 && d->state->txop <= TxTranslate);
    d->engine->drawTiledPixmap(QRect(x, y, w, h), pixmap, QPoint(sx, sy), optim);
}

/*!
    \fn void QPainter::drawPicture(const QPoint &p, const QPicture &pic)
    \overload

    Draws picture \a pic at point \a p.
*/

/*!
    Replays the picture \a p translated by (\a x, \a y).

    This function does exactly the same as QPicture::play() when
    called with (\a x, \a y) = (0, 0).
*/

void QPainter::drawPicture(int x, int y, const QPicture &p)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);
    save();
    translate(x, y);
    const_cast<QPicture *>(&p)->play(this);
    restore();
}


/*!
    \fn void QPainter::eraseRect(const QRect &r)

    \overload

    Erases the area inside the rectangle \a r.
*/

/*!
    Erases the area inside \a x, \a y, \a w, \a h. Equivalent to
    \c{fillRect(x, y, w, h, backgroundColor())}.
*/

void QPainter::eraseRect(int x, int y, int w, int h)
{
    if (!isActive())
        return;
    d->engine->updateState(d->state);

    if (d->state->bgBrush.pixmap())
        drawTiledPixmap(QRect(x, y, w, h), *d->state->bgBrush.pixmap(), -d->state->bgOrigin);
    else
        fillRect(x, y, w, h, d->state->bgBrush);
}

/*! \fn void QPainter::fillRect(const QRect &r, const QBrush &brush)

    \overload

    Fills the rectangle \a r with the \a brush.

    You can specify a QColor as \a brush, since there is a QBrush
    constructor that takes a QColor argument and creates a solid
    pattern brush.

    \sa drawRect()
*/

/*!
    Fills the rectangle \a (x, y, w, h) with the \a brush.

    You can specify a QColor as \a brush, since there is a QBrush
    constructor that takes a QColor argument and creates a solid
    pattern brush.

    \sa drawRect()
*/

void QPainter::fillRect(int x, int y, int w, int h, const QBrush &brush)
{
    QPen   oldPen   = pen();                        // save pen
    QBrush oldBrush = this->brush();                // save brush
    setPen(Qt::NoPen);
    setBrush(brush);
    drawRect(x, y, w, h);                        // draw filled rect
    setBrush(oldBrush);                        // restore brush
    setPen(oldPen);                                // restore pen
}


/*!
  \internal
*/

void QPainter::updateXForm()
{
    QWMatrix m;
    if (d->state->VxF) {
        double scaleW = (double)d->state->vw/(double)d->state->ww;
        double scaleH = (double)d->state->vh/(double)d->state->wh;
        m.setMatrix(scaleW, 0, 0, scaleH, d->state->vx - d->state->wx*scaleW, d->state->vy - d->state->wy*scaleH);
    }
    if (d->state->WxF) {
        if (d->state->VxF)
            m = d->state->worldMatrix * m;
        else
            m = d->state->worldMatrix;
    }
    d->state->matrix = m;


    d->txinv = false;                                // no inverted matrix
    d->state->txop  = TxNone;
    if (d->state->matrix.m12()==0.0 && d->state->matrix.m21()==0.0
        && d->state->matrix.m11() >= 0.0 && d->state->matrix.m22() >= 0.0) {
        if (d->state->matrix.m11()==1.0 && d->state->matrix.m22()==1.0) {
            if (d->state->matrix.dx()!=0.0 || d->state->matrix.dy()!=0.0)
                d->state->txop = TxTranslate;
        } else {
            d->state->txop = TxScale;
        }
    } else {
        d->state->txop = TxRotShear;
    }
    if (!d->redirection_offset.isNull()) {
        d->state->txop |= TxTranslate;
        d->state->WxF = true;
        // We want to translate in dev space so we do the adding of the redirection
        // offset manually.
        d->state->matrix = QWMatrix(d->state->matrix.m11(), d->state->matrix.m12(),
                              d->state->matrix.m21(), d->state->matrix.m22(),
                              d->state->matrix.dx()-d->redirection_offset.x(),
                              d->state->matrix.dy()-d->redirection_offset.y());
    }
    d->engine->setDirty(QPaintEngine::DirtyTransform);
//     printf("VxF=%d, WxF=%d\n", d->state->VxF, d->state->WxF);
//     printf("Using matrix: %f, %f, %f, %f, %f, %f\n",
//            d->state->matrix.m11(),
//            d->state->matrix.m12(),
//            d->state->matrix.m21(),
//            d->state->matrix.m22(),
//            d->state->matrix.dx(),
//            d->state->matrix.dy());
}


/*! \internal */
void QPainter::updateInvXForm()
{
    Q_ASSERT(d->txinv == false);
    d->txinv = true;                                // creating inverted matrix
    bool invertible;
    QWMatrix m;
    if (d->state->VxF) {
        m.translate(d->state->vx, d->state->vy);
        m.scale(1.0*d->state->vw/d->state->ww, 1.0*d->state->vh/d->state->wh);
        m.translate(-d->state->wx, -d->state->wy);
    }
    if (d->state->WxF) {
        if (d->state->VxF)
            m = d->state->worldMatrix * m;
        else
            m = d->state->worldMatrix;
    }
    d->invMatrix = m.invert(&invertible);                // invert matrix
}


/*!
    Returns the point \a p transformed from model coordinates to
    device coordinates.

    \sa xFormDev(), QWMatrix::map()
*/

QPoint QPainter::xForm(const QPoint &p) const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->state->txop) {
    case TxNone:
        return p;
    case TxTranslate:
        return QPoint(qRound(p.x() + d->state->matrix.dx()), qRound(p.y() + d->state->matrix.dy()));
    case TxScale:
        return QPoint(qRound(d->state->matrix.m11()*p.x() + d->state->matrix.dx()),
                      qRound(d->state->matrix.m22()*p.y() + d->state->matrix.dy()));
    default:
        return QPoint(qRound(d->state->matrix.m11()*p.x() + d->state->matrix.m21()*p.y()+d->state->matrix.dx()),
                      qRound(d->state->matrix.m12()*p.x() + d->state->matrix.m22()*p.y()+d->state->matrix.dy()));
    }
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

    \sa xFormDev(), QWMatrix::map()
*/

QRect QPainter::xForm(const QRect &r)        const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->state->txop) {
    case TxNone:
        return r;
    case TxTranslate: {
        QRect rect(r);
        rect.moveBy(qRound(d->state->matrix.dx()), qRound(d->state->matrix.dy()));
        return rect;
    }
    case TxScale:
        return QRect(qRound(d->state->matrix.m11()*r.x() + d->state->matrix.dx()),
                     qRound(d->state->matrix.m22()*r.y() + d->state->matrix.dy()),
                     qRound(d->state->matrix.m11()*r.width()),
                     qRound(d->state->matrix.m22()*r.height()));
    case TxRotShear:
        return d->state->matrix.mapRect(r);
    }
    return r;
#else
    return QRect(r.x()+d->state->xlatex, r.y()+d->state->xlatey, r.width(), r.height());
#endif
}

/*!
    \overload

    Returns the point array \a a transformed from model coordinates
    to device coordinates.

    \sa xFormDev(), QWMatrix::map()
*/

QPointArray QPainter::xForm(const QPointArray &a) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
        return a;
    return d->state->matrix * a;
#else
    QPointArray p(a);
    p.translate(d->state->xlatex, d->state->xlatey);
    return p;
#endif
}

/*!
    \overload

    Returns the point array \a av transformed from model coordinates
    to device coordinates. The \a index is the first point in the
    array and \a npoints denotes the number of points to be
    transformed. If \a npoints is negative, all points from \a
    av[index] until the last point in the array are transformed.

    The returned point array consists of the number of points that
    were transformed.

    Example:
    \code
        QPointArray a(10);
        QPointArray b;
        b = painter.xForm(a, 2, 4);  // b.size() == 4
        b = painter.xForm(a, 2, -1); // b.size() == 8
    \endcode

    \sa xFormDev(), QWMatrix::map()
*/

QPointArray QPainter::xForm(const QPointArray &av, int index, int npoints) const
{
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPointArray a(lastPoint-index);
    memcpy(a.data(), av.data()+index, (lastPoint-index)*sizeof(QPoint));
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->matrix*a;
#else
    a.translate(d->state->xlatex, d->state->xlatey);
    return a;
#endif
}

/*!
    \overload

    Returns the point \a p transformed from device coordinates to
    model coordinates.

    \sa xForm(), QWMatrix::map()
*/

QPoint QPainter::xFormDev(const QPoint &p) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if(d->state->txop == TxNone)
        return p;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->updateInvXForm();
    }
    return QPoint(qRound(d->invMatrix.m11()*p.x() + d->invMatrix.m21()*p.y() + d->invMatrix.dx()),
                  qRound(d->invMatrix.m12()*p.x() + d->invMatrix.m22()*p.y() + d->invMatrix.dy()));
#else
    return QPoint(p.x() - xlatex, p.y() - xlatey);
#endif
}

/*!
    Returns the rectangle \a r transformed from device coordinates to
    model coordinates.

    If world transformation is enabled and rotation or shearing is
    used, then the bounding rectangle is returned.

    \sa xForm(), QWMatrix::map()
*/

QRect QPainter::xFormDev(const QRect &r)  const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
        return r;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->updateInvXForm();
    }
    if (d->state->txop == TxRotShear) {                        // rotation/shear
        return d->invMatrix.mapRect(r);
    } else {
        return QRect(qRound(d->invMatrix.m11()*r.x() + d->invMatrix.dx()),
                     qRound(d->invMatrix.m22()*r.y() + d->invMatrix.dy()),
                     qRound(d->invMatrix.m11()*r.width()),
                     qRound(d->invMatrix.m22()*r.height()));
    }
#else
    return QRect(r.x()-d->state->xlatex, r.y()-d->state->xlatey, r.width(), r.height());
#endif
}

/*!
    \overload

    Returns the point array \a a transformed from device coordinates
    to model coordinates.

    \sa xForm(), QWMatrix::map()
*/

QPointArray QPainter::xFormDev(const QPointArray &a) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
        return a;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->updateInvXForm();
    }
    return d->invMatrix * a;
#else
    QPointArray p(a);
    p.translate(-d->state->xlatex, -d->state->xlatey);
    return p;
#endif

}

/*!
    \overload

    Returns the point array \a ad transformed from device coordinates
    to model coordinates. The \a index is the first point in the array
    and \a npoints denotes the number of points to be transformed. If
    \a npoints is negative, all points from \a ad[index] until the
    last point in the array are transformed.

    The returned point array consists of the number of points that
    were transformed.

    Example:
    \code
        QPointArray a(10);
        QPointArray b;
        b = painter.xFormDev(a, 1, 3);  // b.size() == 3
        b = painter.xFormDev(a, 1, -1); // b.size() == 9
    \endcode

    \sa xForm(), QWMatrix::map()
*/

QPointArray QPainter::xFormDev(const QPointArray &ad, int index, int npoints) const
{
    int lastPoint = npoints < 0 ? ad.size() : index+npoints;
    QPointArray a(lastPoint-index);
    memcpy(a.data(), ad.data()+index, (lastPoint-index)*sizeof(QPoint));
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
        return a;
    if (!d->txinv) {
        QPainter *that = (QPainter*)this;        // mutable
        that->updateInvXForm();
    }
    return d->invMatrix * a;
#else
    a.translate(-d->state->xlatex, -d->state->xlatey);
    return a;
#endif
}

/*!
    Sets the render hints supplied in \a hints. Several render hints
    can be OR'ed together in \a hints.
*/
void QPainter::setRenderHints(RenderHint hints)
{
    if (!isActive()) {
        qWarning("Painter must be active to set rendering hints");
        return;
    }

    d->engine->setRenderHints(hints);
}

/*!
    Clears the render hints supplied in \a hints. Several render hints
    can be OR'ed together in \a hints.
*/
void QPainter::clearRenderHints(RenderHint hints)
{
    if (!isActive()) {
        qWarning("Painter must be active to clear rendering hints");
        return;
    }
    d->engine->clearRenderHints(hints);
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
    return d->engine->renderHints();
}


#if defined Q_WS_WIN
/*! \internal */
HDC QPainter::handle() const
#else
/*! \internal */
Qt::HANDLE QPainter::handle() const
#endif
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->engine);
    d->engine->updateState(d->state);
    return d->engine->handle();
}

double QPainter::m11() const { return d->state->matrix.m11(); }
double QPainter::m12() const { return d->state->matrix.m12(); }
double QPainter::m21() const { return d->state->matrix.m21(); }
double QPainter::m22() const { return d->state->matrix.m22(); }
double QPainter::dx() const { return d->state->matrix.dx(); }
double QPainter::dy() const { return d->state->matrix.dy(); }
double QPainter::im11() const { return d->invMatrix.m11(); }
double QPainter::im12() const { return d->invMatrix.m12(); }
double QPainter::im21() const { return d->invMatrix.m21(); }
double QPainter::im22() const { return d->invMatrix.m22(); }
double QPainter::idx() const { return d->invMatrix.dx(); }
double QPainter::idy() const { return d->invMatrix.dy(); }

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

static QList<QPaintDeviceRedirection> redirections;


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
    redirections.ensure_constructed();
    redirections += QPaintDeviceRedirection(device, replacement, offset);
}


/*!\internal

  Restores the previous redirection for \a device after a call to
  setRedirected().

  \sa redirected()
 */
void QPainter::restoreRedirected(const QPaintDevice *device)
{
    Q_ASSERT(device != 0);
    redirections.ensure_constructed();
    for (int i = redirections.size()-1; i >= 0; --i)
        if (redirections.at(i) == device) {
            redirections.removeAt(i);
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
    redirections.ensure_constructed();
    for (int i = redirections.size()-1; i >= 0; --i)
        if (redirections.at(i) == device) {
            if (offset)
                *offset = redirections.at(i).offset;
            return redirections.at(i).replacement;
        }
    return 0;
}


void qt_format_text(const QFont& font, const QRect &_r,
                     int tf, const QString& str, int len, QRect *brect,
                     int tabstops, int *, int tabarraylen,
                     QPainter* painter)
{
    // we need to copy r here to protect against the case (&r == brect).
    QRect r(_r);

    bool dontclip  = (tf & Qt::DontClip)  == Qt::DontClip;
    bool wordbreak  = (tf & Qt::WordBreak)  == Qt::WordBreak;
    bool singleline = (tf & Qt::SingleLine) == Qt::SingleLine;
    bool showprefix = (tf & Qt::ShowPrefix) == Qt::ShowPrefix;
    bool noaccel = (tf & Qt::NoAccel) == Qt::NoAccel;

    bool isRightToLeft = str.isRightToLeft();
    if ((tf & Qt::AlignHorizontal_Mask) == Qt::AlignAuto)
        tf |= isRightToLeft ? Qt::AlignRight : Qt::AlignLeft;

    bool expandtabs = ((tf & Qt::ExpandTabs) &&
                        (((tf & Qt::AlignLeft) && !isRightToLeft) ||
                          ((tf & Qt::AlignRight) && isRightToLeft)));

    if (!painter)
        tf |= Qt::DontPrint;

    int maxUnderlines = 0;
    int numUnderlines = 0;
    int underlinePositionStack[32];
    int *underlinePositions = underlinePositionStack;

        // ### port properly
    QFont fnt(painter
              ? (painter->d->state->pfont
                 ? *painter->d->state->pfont
                 : painter->d->state->font)
              : font);
    QFontMetrics fm(fnt);

    QString text = str;
    // compatible behaviour to the old implementation. Replace
    // tabs by spaces
    QChar *chr = text.data();
    const QChar *end = chr + len;
    bool haveLineSep = false;
    while (chr != end) {
        if (*chr == '\r' || (singleline && *chr == '\n')) {
            *chr = ' ';
        } else if (*chr == '\n') {
            *chr = QChar::LineSeparator;
            haveLineSep = true;
        } else if (*chr == '&') {
            ++maxUnderlines;
        }
        ++chr;
    }
    if (!expandtabs) {
        chr = (QChar*)text.unicode();
        while (chr != end) {
            if (*chr == '\t')
                *chr = ' ';
            ++chr;
        }
    } else if (!tabarraylen && !tabstops) {
        tabstops = fm.width('x')*8;
    }

    if (noaccel || showprefix) {
        if (maxUnderlines > 32)
            underlinePositions = new int[maxUnderlines];
        QChar *cout = (QChar*)text.unicode();
        QChar *cin = cout;
        int l = len;
        while (l) {
            if (*cin == '&') {
                ++cin;
                --l;
                if (!l)
                    break;
                if (*cin != '&')
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
    if (tf & Qt::DontPrint)
        numUnderlines = 0;

    underlinePositions[numUnderlines] = -1;
    int height = 0;
    int width = 0;

    QTextLayout textLayout(text, fnt);
    textLayout.engine()->underlinePositions = underlinePositions;

    if (text.isEmpty()) {
        height = fm.height();
        width = 0;
        tf |= QPainter::DontPrint;
    } else {
        int lineWidth = wordbreak ? qMax(0, r.width()) : 0x01000000;
        if(!wordbreak)
            tf |= Qt::IncludeTrailingSpaces;
        textLayout.beginLayout((tf & Qt::DontPrint) ? QTextLayout::NoBidi : QTextLayout::SingleLine);

        int leading = fm.leading();
        height = -leading;

        textLayout.clearLines();
        while (1) {
            QTextLine l = textLayout.createLine();
            if (!l.isValid())
                break;

            l.layout(lineWidth);
            height += leading;
            l.setPosition(QPoint(0, height));
            height += l.ascent() + l.descent();
            width = qMax(width, l.textWidth());
        }
    }

    int yoff = 0;
    int xoff = 0;
    if (tf & Qt::AlignBottom)
        yoff = r.height() - height;
    else if (tf & Qt::AlignVCenter)
        yoff = (r.height() - height)/2;
    if (tf & Qt::AlignRight)
        xoff = r.width() - width;
    else if (tf & Qt::AlignHCenter)
        xoff = (r.width() - width)/2;
    if (brect)
        *brect = QRect(r.x() + xoff, r.y() + yoff, width, height);

    if (!(tf & QPainter::DontPrint)) {
        bool restoreClipping = false;
        bool painterHasClip = false;
        QRegion painterClipRegion;
        if (!dontclip) {
            QRegion reg(r);
#ifdef QT_NO_TRANSFORMATIONS
            reg.translate(painter->d->state->xlatex, painter->d->state->xlatey);
#endif
            if (painter->hasClipping())
                reg &= painter->clipRegion();

            painterHasClip = painter->hasClipping();
            painterClipRegion = painter->clipRegion();
            restoreClipping = true;
            painter->setClipRegion(reg);
        } else {
            if (painter->hasClipping()){
                painterHasClip = painter->hasClipping();
                painterClipRegion = painter->clipRegion();
                restoreClipping = true;
                painter->setClipping(false);
            }
        }

        int _tf = 0;
        if (fnt.underline()) _tf |= Qt::Underline;
        if (fnt.overline()) _tf |= Qt::Overline;
        if (fnt.strikeOut()) _tf |= Qt::StrikeOut;

        for (int i = 0; i < textLayout.numLines(); i++) {
            QTextLine line = textLayout.lineAt(i);

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
            extern void qt_draw_background(QPaintEngine *pe, int x, int y, int w,  int h);

            if (painter->backgroundMode() == Qt::OpaqueMode)
                qt_draw_background(painter->d->engine, r.x() + line.x() + xoff, r.y() + yoff + line.y(),
                                   line.width(), line.ascent() + line.descent() + 1);
#endif
            line.draw(painter, r.x() + xoff + line.x(), r.y() + yoff);
        }

        if (restoreClipping) {
            painter->setClipRegion(painterClipRegion);
            painter->setClipping(painterHasClip);
        }
    }

    if (underlinePositions != underlinePositionStack)
        delete [] underlinePositions;
}



template <class T> void qt_swap(T &a, T &b) { T tmp=a; a=b; b=tmp; }

// #define QT_GRAD_NO_POLY
// #define QT_GRAD_NO_LINE

void qt_fill_linear_gradient(const QRect &rect, QPainter *p, const QBrush &brush)
{
    Q_ASSERT(brush.style() == Qt::LinearGradientPattern);

    QPoint gstart = p->xForm(brush.gradientStart());
    QPoint gstop  = p->xForm(brush.gradientStop());

    QPen oldPen = p->pen();
    p->translate(rect.topLeft());

    gstart -= rect.topLeft();
    gstop -= rect.topLeft();

    QColor gcol1 = brush.color();
    QColor gcol2 = brush.gradientColor();

    int dx = gstop.x() - gstart.x();
    int dy = gstop.y() - gstart.y();

    int rw = rect.width();
    int rh = rect.height();

    p->clearRenderHints(QPainter::LineAntialiasing);

    if (QABS(dx) > QABS(dy)) { // Fill horizontally
        // Make sure we fill left to right.
        if (gstop.x() < gstart.x()) {
            qt_swap(gcol1, gcol2);
            qt_swap(gstart, gstop);
        }
        // Find the location where the lines covering the gradient intersect
        // the lines making up the top and bottom of the target rectangle.
        // Note: This might be outside the target rect, but that is ok.
        int xtop1, xtop2, xbot1, xbot2;
        if (dy == 0) {
            xtop1 = xbot1 = gstart.x();
            xtop2 = xbot2 = gstop.x();
        } else {
            double gamma = double(dx) / double(-dy);
            xtop1 = qRound((-gstart.y() + gamma * gstart.x() ) / gamma);
            xtop2 = qRound((-gstop.y()  + gamma * gstop.x()  ) / gamma);
            xbot1 = qRound((rh - gstart.y() + gamma * gstart.x() ) / gamma);
            xbot2 = qRound((rh - gstop.y()  + gamma * gstop.x()  ) / gamma);
            Q_ASSERT(xtop2 > xtop1);
        }

        p->setPen(Qt::NoPen);
#ifndef QT_GRAD_NO_POLY
        // Fill the area to the left of the gradient
        QPointArray leftFill;
        leftFill << QPoint(0, 0)
                 << QPoint(xtop1+1, 0)
                 << QPoint(xbot1+1, rh);
        if (xbot1 > 0)
            leftFill << QPoint(0, rh);
        p->setBrush(gcol1);
        p->drawPolygon(leftFill);

        // Fill the area to the right of the gradient
        QPointArray rightFill;
        rightFill << QPoint(rw, rh)
                  << QPoint(xbot2-1, rh)
                  << QPoint(xtop2-1, 0);
        if (xtop2 < rw)
            rightFill << QPoint(rw, 0);
        p->setBrush(gcol2);
        p->drawPolygon(rightFill);
#endif // QT_GRAD_NO_POLY

#ifndef QT_GRAD_NO_LINE
        // Fill the gradient.
        double r = gcol1.red();
        double g = gcol1.green();
        double b = gcol1.blue();
        int steps = (xtop2-xtop1);
        double rinc = (gcol2.red()-r) / steps;
        double ginc = (gcol2.green()-g) / steps;
        double binc = (gcol2.blue()-b) / steps;
        for (int x=0; x<=steps; ++x) {
            p->setPen(QColor(int(r), int(g), int(b)));
            p->drawLine(x+xtop1, 0, x+xbot1, rh);
            r += rinc;
            g += ginc;
            b += binc;
        }
#else
        p->setPen(Qt::black);
        p->drawLine(xtop1, 0, xbot1, rh);
        p->drawLine(xtop2, 0, xbot2, rh);
#endif // QT_GRAD_NO_LINE
    } else {
        // Fill Verticallty
        // Code below is a conceptually equal to the one above except that all
        // coords are swapped x <-> y.
        // Make sure we fill top to bottom...
        if (gstop.y() < gstart.y()) {
            qt_swap(gstart, gstop);
            qt_swap(gcol1, gcol2);
        }
        int yleft1, yleft2, yright1, yright2;
        if (dx == 0) {
            yleft1 = yright1 = gstart.y();
            yleft2 = yright2 = gstop.y();
        } else {
            double gamma = double(dy) / double(-dx);
            yleft1 = qRound((-gstart.x() + gamma * gstart.y()) / gamma);
            yleft2 = qRound((-gstop.x() + gamma * gstop.y()) / gamma);
            yright1 = qRound((rw - gstart.x() + gamma*gstart.y()) / gamma);
            yright2 = qRound((rw - gstop.x() + gamma*gstop.y()) / gamma);
            Q_ASSERT(yleft2 > yleft1);
        }

#ifndef QT_GRAD_NO_POLY
        p->setPen(Qt::NoPen);
        QPointArray topFill;
        topFill << QPoint(0, 0)
                << QPoint(0, yleft1 + 1)
                << QPoint(rw, yright1 + 1);
        if (yright1 > 0)
            topFill << QPoint(rw, 0);
        p->setBrush(gcol1);
        p->drawPolygon(topFill);

        QPointArray bottomFill;
        bottomFill << QPoint(rw, rh)
                   << QPoint(rw, yright2-1)
                   << QPoint(0, yleft2-1);
        if (yleft2 < rh)
            bottomFill << QPoint(0, rh);
        p->setBrush(gcol2);
        p->drawPolygon(bottomFill);
#endif // QT_GRAD_NO_POLY

#ifndef QT_GRAD_NO_LINE
        double r = gcol1.red();
        double g = gcol1.green();
        double b = gcol1.blue();
        int steps = (yleft2-yleft1);
        double rinc = (gcol2.red()-r) / steps;
        double ginc = (gcol2.green()-g) / steps;
        double binc = (gcol2.blue()-b) / steps;
        for (int y=0; y<=steps; ++y) {
            p->setPen(QColor(int(r), int(g), int(b)));
            p->drawLine(0, y+yleft1, rw, y+yright1);
            r += rinc;
            g += ginc;
            b += binc;
        }
#else
        p->setPen(Qt::black);
        p->drawLine(0, yleft1, rw, yright1);
        p->drawLine(0, yleft2, rw, yright2);
#endif // QT_GRAD_NO_LINE
    }

    p->translate(-rect.topLeft());
    p->setPen(oldPen);

}
