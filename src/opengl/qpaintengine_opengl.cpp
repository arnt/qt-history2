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

#include <private/qpaintengine_p.h>
#include "qapplication.h"
#include "qbrush.h"
#include "qgl.h"
#include <private/qgl_p.h>
#include "qmap.h"
#include <private/qpaintengine_opengl_p.h>
#include "qpen.h"
#include "qvarlengtharray.h"
#include <private/qpainter_p.h>
#include <qdebug.h>

#ifdef Q_OS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif

#include <stdlib.h>

#ifndef CALLBACK // for Windows
#define CALLBACK
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif
#ifndef GL_MULTISAMPLE_FILTER_HINT_NV
#define GL_MULTISAMPLE_FILTER_HINT_NV   0x8534
#endif

// define QT_GL_NO_CONCAVE_POLYGONS to remove support for drawing
// concave polygons (much faster)

//#define QT_GL_NO_CONCAVE_POLYGONS

class QGLDrawable {
public:
    QGLDrawable() : widget(0) {}
    inline void setDevice(QPaintDevice *pdev);
    inline void setAutoBufferSwap(bool);
    inline void swapBuffers();
    inline void makeCurrent();
    inline QSize size() const;
    inline QGLFormat format() const;
    inline GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
    inline GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);

private:
    QGLWidget *widget;
};

void QGLDrawable::setDevice(QPaintDevice *pdev)
{
    widget = static_cast<QGLWidget *>(pdev);
}

inline void QGLDrawable::setAutoBufferSwap(bool enable)
{
    widget->setAutoBufferSwap(enable);
}

inline void QGLDrawable::swapBuffers()
{
    widget->swapBuffers();
}

inline void QGLDrawable::makeCurrent()
{
    widget->makeCurrent();
}

inline QSize QGLDrawable::size() const
{
    return widget->size();
}

inline QGLFormat QGLDrawable::format() const
{
    return widget->format();
}

inline GLuint QGLDrawable::bindTexture(const QImage &image, GLenum target, GLint format)
{
    return widget->bindTexture(image, target, format);
}

inline GLuint QGLDrawable::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
    return widget->bindTexture(pixmap, target, format);
}


class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : bgmode(Qt::TransparentMode)
        , txop(QPainterPrivate::TxNone) {}

    void setGLPen(const QColor &c) {
        pen_color[0] = c.red();
        pen_color[1] = c.green();
        pen_color[2] = c.blue();
        pen_color[3] = c.alpha();
    }

    void setGLBrush(const QColor &c) {
        brush_color[0] = c.red();
        brush_color[1] = c.green();
        brush_color[2] = c.blue();
        brush_color[3] = c.alpha();
    }

    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::BGMode bgmode;
    QRegion crgn;
    uint has_clipping : 1;
    uint has_pen : 1;
    uint has_brush : 1;

    QMatrix matrix;
    GLubyte pen_color[4];
    GLubyte brush_color[4];
    QPainterPrivate::TransformationCodes txop;
    QGLDrawable drawable;
};

QOpenGLPaintEngine::QOpenGLPaintEngine()
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
                   PaintEngineFeatures(AllFeatures
                                       & ~(LinearGradientFill
                                           | RadialGradientFill
                                           | ConicalGradientFill
                                           | PatternBrush)))
{
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);
    Q_ASSERT(static_cast<const QGLWidget *>(pdev));
//     d->pdev = pdev;
    d->drawable.setDevice(pdev);
    d->has_clipping = false;
    d->drawable.setAutoBufferSwap(false);
    setActive(true);
    d->drawable.makeCurrent();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    const QColor &c = d->bgbrush.color();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);

    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    glPopAttrib();
    glFlush();
    d->drawable.swapBuffers();
    setActive(false);
    return true;
}

void QOpenGLPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();
    if (flags & DirtyPen) updatePen(state.pen());
    if (flags & DirtyBrush) updateBrush(state.brush(), state.brushOrigin());
    if (flags & DirtyBackground) updateBackground(state.backgroundMode(), state.backgroundBrush());
    if (flags & DirtyFont) updateFont(state.font());
    if (flags & DirtyTransform) updateMatrix(state.matrix());
    if (flags & DirtyClipPath) {
        updateClipRegion(QRegion(state.clipPath().toFillPolygon().toPolygon(),
                                 state.clipPath().fillRule()),
                         state.clipOperation());
    }
    if (flags & DirtyClipRegion) updateClipRegion(state.clipRegion(), state.clipOperation());
    if (flags & DirtyHints) updateRenderHints(state.renderHints());
}

void QOpenGLPaintEngine::updatePen(const QPen &pen)
{
    Q_D(QOpenGLPaintEngine);
    d->cpen = pen;
    d->has_pen = (pen.style() != Qt::NoPen);
    d->setGLPen(pen.color());
    glColor4ubv(d->pen_color);
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    d->cbrush = brush;
    d->has_brush = (brush.style() != Qt::NoBrush);
    d->setGLBrush(brush.color());
    glColor4ubv(d->brush_color);

#if 0 // doesnt work very well yet - use fallback for now
    // all GL polygon stipple patterns needs to be specified as a
    // 32x32 bit mask
    static const GLubyte dense1_pat[] = {
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    static const GLubyte dense2_pat[] = {
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xbb,
        0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee };

    static const GLubyte dense3_pat[] = {
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa,
        0x77, 0x77, 0x77, 0x77, 0xaa, 0xaa, 0xaa, 0xaa,
        0xdd, 0xdd, 0xdd, 0xdd, 0xaa, 0xaa, 0xaa, 0xaa };

    static const GLubyte dense4_pat[] = {
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
        0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa };

    static const GLubyte dense5_pat[] = {
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55,
        0x88, 0x88, 0x88, 0x88, 0x55, 0x55, 0x55, 0x55,
        0x22, 0x22, 0x22, 0x22, 0x55, 0x55, 0x55, 0x55 };

    static const GLubyte dense6_pat[] = {
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00 };

    static const GLubyte dense7_pat[] = {
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    static const GLubyte hor_pat[] = {                      // horizontal pattern
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    static const GLubyte ver_pat[] = {                      // vertical pattern
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };

    static const GLubyte cross_pat[] = {                    // cross pattern
        0xff, 0xff, 0xff, 0xff, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0xff, 0xff, 0xff, 0xff, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0xff, 0xff, 0xff, 0xff, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0xff, 0xff, 0xff, 0xff, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };

    static const GLubyte bdiag_pat[] = {                    // backward diagonal pattern
        0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10,
        0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04,
        0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
        0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40,
        0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10,
        0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04,
        0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
        0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40,
        0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10,
        0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04,
        0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
        0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40,
        0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10,
        0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04,
        0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
        0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40 };


    static const GLubyte fdiag_pat[] = {                    // forward diagonal pattern
        0x80, 0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04,
        0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10,
        0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40,
        0x80, 0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04,
        0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10,
        0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40,
        0x80, 0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04,
        0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10,
        0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40,
        0x80, 0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04,
        0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10,
        0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40 };

    static const GLubyte dcross_pat[] = {                   // diagonal cross pattern
        0x84, 0x84, 0x84, 0x84, 0x48, 0x48, 0x48, 0x48,
        0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x48, 0x48, 0x48, 0x48, 0x84, 0x84, 0x84, 0x84,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x84, 0x84, 0x84, 0x84, 0x48, 0x48, 0x48, 0x48,
        0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x48, 0x48, 0x48, 0x48, 0x84, 0x84, 0x84, 0x84,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x84, 0x84, 0x84, 0x84, 0x48, 0x48, 0x48, 0x48,
        0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x48, 0x48, 0x48, 0x48, 0x84, 0x84, 0x84, 0x84,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x84, 0x84, 0x84, 0x84, 0x48, 0x48, 0x48, 0x48,
        0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x48, 0x48, 0x48, 0x48, 0x84, 0x84, 0x84, 0x84,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 };

    static const GLubyte * const pat_tbl[] = {
        dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
        dense6_pat, dense7_pat, hor_pat, ver_pat, cross_pat, bdiag_pat,
        fdiag_pat, dcross_pat };

    int bs = d->cbrush.style();
    if (bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        glEnable(GL_POLYGON_STIPPLE);
        glPolygonStipple(pat_tbl[bs - Qt::Dense1Pattern]);
    } else {
        glDisable(GL_POLYGON_STIPPLE);
    }
#endif
}

void QOpenGLPaintEngine::updateFont(const QFont &)
{
}

void QOpenGLPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    Q_D(QOpenGLPaintEngine);
    const QColor &c = bgBrush.color();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    d->bgmode = bgMode;
    d->bgbrush = bgBrush;
}

void QOpenGLPaintEngine::updateMatrix(const QMatrix &mtx)
{
    Q_D(QOpenGLPaintEngine);

    d->matrix = mtx;
    GLdouble mat[4][4];

    mat[0][0] = mtx.m11();
    mat[0][1] = mtx.m12();
    mat[0][2] = 0;
    mat[0][3] = 0;

    mat[1][0] = mtx.m21();
    mat[1][1] = mtx.m22();
    mat[1][2] = 0;
    mat[1][3] = 0;

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = mtx.dx();
    mat[3][1] = mtx.dy();
    mat[3][2] = 0;
    mat[3][3] = 1;

    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(&mat[0][0]);
}

void QOpenGLPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_D(QOpenGLPaintEngine);
    QGLFormat f = d->drawable.format();
    bool useStencilBuffer = f.stencil();
    bool useDepthBuffer = f.depth() && !useStencilBuffer;

    // clipping is only supported when a stencil or depth buffer is
    // available
    if (!useStencilBuffer && !useDepthBuffer)
	return;

    if (op == Qt::NoClip) {
        d->has_clipping = false;
        d->crgn = QRegion();
        glDisable(useStencilBuffer ? GL_STENCIL_TEST : GL_DEPTH_TEST);
        return;
    }

    QRegion region = clipRegion * d->matrix;
    switch (op) {
    case Qt::IntersectClip:
        if (d->has_clipping) {
            d->crgn &= region;
            break;
        }
        // fall through
    case Qt::ReplaceClip:
        d->crgn = region;
        break;
    case Qt::UniteClip:
        d->crgn |= region;
        break;
    default:
        break;
    }

    if (useStencilBuffer) {
        glClearStencil(0x0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glClearStencil(0x1);
    } else {
        glClearDepth(0x0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthMask(true);
        glClearDepth(0x1);
    }

    const QVector<QRect> rects = d->crgn.rects();
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < rects.size(); ++i) {
        glScissor(rects.at(i).left(), d->drawable.size().height() - rects.at(i).bottom(),
                  rects.at(i).width(), rects.at(i).height());
        glClear(useStencilBuffer ? GL_STENCIL_BUFFER_BIT : GL_DEPTH_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);

    if (useStencilBuffer) {
        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glEnable(GL_STENCIL_TEST);
    } else {
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
    }
    d->has_clipping = true;
}

void QOpenGLPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    if (hints & QPainter::Antialiasing) {
        glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }
}

void QOpenGLPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QOpenGLPaintEngine);

    // ### this could be done faster I'm sure...
    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];
        double x = r.x();
        double y = r.y();
        double w = r.width();
        double h = r.height();
        if (d->has_brush) {
            glColor4ubv(d->brush_color);
            glRectd(x, y, x+w, y+h);
            if (!d->has_pen)
                return;
        }

        if (d->has_pen) {
            // Specify the outline as 4 separate lines since a quad or a
            // polygon won't give us exactly what we want
            glColor4ubv(d->pen_color);
            glBegin(GL_LINES);
            {
                glVertex2d(x, y);
                glVertex2d(x+w, y);
                glVertex2d(x+w, y-1);
                glVertex2d(x+w, y+h);
                glVertex2d(x+w, y+h);
                glVertex2d(x, y+h);
                glVertex2d(x, y+h);
                glVertex2d(x, y);
            }
            glEnd();
        }
    }
}

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
        const QPointF *end = points + pointCount;
        while (points < end) {
            QPainterPath path;
            path.moveTo(*points);
            path.lineTo(points->x() + 0.001, points->y());
            drawPath(path);
            ++points;
        }
        return;
    }
    glBegin(GL_POINTS);
    {
        for (int i=0; i<pointCount; ++i)
            glVertex2d(points[i].x(), points[i].y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QOpenGLPaintEngine);
    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
        QPainterPath path(lines[0].p1());
        path.lineTo(lines[0].p2());
        for (int i = 1; i < lineCount; ++lines) {
            path.lineTo(lines[i].p1());
            path.lineTo(lines[i].p2());
        }
        drawPath(path);
        return;
    }
    glColor4ubv(d->pen_color);
    glBegin(GL_LINES);
    {
        for (int i = 0; i < lineCount; ++i) {
            glVertex2d(lines[i].x1(), lines[i].y1());
            glVertex2d(lines[i].x2(), lines[i].y2());
        }
    }
    glEnd();
}

// Need to allocate space for new vertices on intersecting lines and
// they need to be alive until gluTessEndPolygon() has returned
static QList<GLdouble *> vertexStorage;
static void CALLBACK qgl_tess_combine(GLdouble coords[3],
				      GLdouble *[4],
				      GLfloat [4], GLdouble **dataOut)
{
    GLdouble *vertex;
    vertex = (GLdouble *) malloc(3 * sizeof(GLdouble));
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *dataOut = vertex;
    vertexStorage.append(vertex);
}

static void CALLBACK qgl_tess_error(GLenum errorCode)
{
    qWarning("QOpenGLPaintEngine: tessellation error: %s", gluErrorString(errorCode));
}

static GLUtesselator *qgl_tess = 0;
static void qgl_cleanup_tesselator()
{
    gluDeleteTess(qgl_tess);
}

static void qgl_draw_poly(const QPointF *points, int pointCount, bool winding = false)
{
#ifndef QT_GL_NO_CONCAVE_POLYGONS
    if (!qgl_tess) {
	qgl_tess = gluNewTess();
	qAddPostRoutine(qgl_cleanup_tesselator);
    }
    gluTessProperty(qgl_tess, GLU_TESS_WINDING_RULE,
                    winding ? GLU_TESS_WINDING_NONZERO : GLU_TESS_WINDING_ODD);
    QVarLengthArray<GLdouble> v(pointCount*3);
#ifdef Q_WS_MAC  // This removes warnings.
    gluTessCallback(qgl_tess, GLU_TESS_BEGIN, reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glBegin));
    gluTessCallback(qgl_tess, GLU_TESS_VERTEX,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glVertex3dv));
    gluTessCallback(qgl_tess, GLU_TESS_END, reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glEnd));
    gluTessCallback(qgl_tess, GLU_TESS_COMBINE,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&qgl_tess_combine));
    gluTessCallback(qgl_tess, GLU_TESS_ERROR,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&qgl_tess_error));
#else
    gluTessCallback(qgl_tess, GLU_TESS_BEGIN, reinterpret_cast<GLvoid (CALLBACK *)()>(&glBegin));
    gluTessCallback(qgl_tess, GLU_TESS_VERTEX,
                    reinterpret_cast<GLvoid (CALLBACK *)()>(&glVertex3dv));
    gluTessCallback(qgl_tess, GLU_TESS_END, reinterpret_cast<GLvoid (CALLBACK *)()>(&glEnd));
    gluTessCallback(qgl_tess, GLU_TESS_COMBINE,
                    reinterpret_cast<GLvoid (CALLBACK *)()>(&qgl_tess_combine));
    gluTessCallback(qgl_tess, GLU_TESS_ERROR,
                    reinterpret_cast<GLvoid (CALLBACK *) ()>(&qgl_tess_error));
#endif
    gluTessBeginPolygon(qgl_tess, NULL);
    {
	gluTessBeginContour(qgl_tess);
	{
	    for (int i = 0; i < pointCount; ++i) {
		v[i*3] = points[i].x();
		v[i*3+1] = points[i].y();
		v[i*3+2] = 0.0;
		gluTessVertex(qgl_tess, &v[i*3], &v[i*3]);
	    }
	}
	gluTessEndContour(qgl_tess);
    }
    gluTessEndPolygon(qgl_tess);
    // clean up after the qgl_tess_combine callback
    for (int i=0; i < vertexStorage.size(); ++i)
	free(vertexStorage[i]);
    vertexStorage.clear();
#else
    glBegin(GL_POLYGON);
    {
        for (int i = 0; i < pointCount; ++i)
	    glVertex2d(points[i].x(), points[i].y());
    }
    glEnd();
#endif
}

void QOpenGLPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QOpenGLPaintEngine);
    if(!pointCount)
        return;
    glColor4ubv(d->brush_color);
    if (d->has_brush && mode != PolylineMode)
        qgl_draw_poly(points, pointCount, mode == QPaintEngine::WindingMode);
    if (d->has_pen) {
        glColor4ubv(d->pen_color);
        double x1 = points[0].x();
        double y1 = points[0].y();
        double x2 = points[pointCount - 1].x();
        double y2 = points[pointCount - 1].y();

        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < pointCount; ++i)
                glVertex2d(points[i].x(), points[i].y());
            if (mode != PolylineMode && !(x1 == x2 && y1 == y2))
                glVertex2d(x1, y1);
        }
        glEnd();
    }
}

void QOpenGLPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QOpenGLPaintEngine);
    if (path.isEmpty())
        return;

    if (d->has_brush) {
        QPolygonF poly = path.toFillPolygon();
        bool had_pen = d->has_pen;
        QPen old_pen = d->cpen;
        d->has_pen = false;
        d->cpen.setStyle(Qt::NoPen);
        drawPolygon(poly.data(), poly.size(),
                    path.fillRule() == Qt::OddEvenFill ? OddEvenMode : WindingMode);
        d->has_pen = had_pen;
        d->cpen = old_pen;
    }

    if (d->has_pen) {
        QPainterPathStroker stroker;
        stroker.setDashPattern(d->cpen.style());
        stroker.setCapStyle(d->cpen.capStyle());
        stroker.setJoinStyle(d->cpen.joinStyle());
        QPainterPath stroke;
        qreal width = d->cpen.widthF();
        QPolygonF poly;
        if (width == 0) {
            stroker.setWidth(1);
            stroke = stroker.createStroke(path * d->matrix);
            if (stroke.isEmpty())
                return;
            poly = stroke.toFillPolygon();
        } else {
            stroker.setWidth(width);
            stroker.setCurveThreshold( 1 / (2 * 10 * d->matrix.m11() * d->matrix.m22()));
            stroke = stroker.createStroke(path);
            if (stroke.isEmpty())
                return;
            poly = stroke.toFillPolygon(d->matrix);
        }

        bool had_pen = d->has_pen;
        bool had_brush = d->has_brush;
        QBrush old_brush = d->cbrush;
        d->has_pen = false;
        d->has_brush = true;
        d->setGLBrush(d->cpen.brush().color());

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        drawPolygon(poly.data(), poly.size(), WindingMode);
        glPopMatrix();

        d->has_pen = had_pen;
        d->has_brush = had_brush;
        d->cbrush = old_brush;
        d->setGLBrush(d->cbrush.color());
    }
}

void QOpenGLPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QOpenGLPaintEngine);
    if (pm.depth() == 1) {
	QPixmap tpx(pm.size());
	tpx.fill(d->bgbrush.color());
	QPainter p(&tpx);
	p.setPen(d->cpen);
	p.drawPixmap(0, 0, pm);
	p.end();
	drawPixmap(r, tpx, sr);
	return;
    }
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
		    ? GL_TEXTURE_RECTANGLE_NV
		    : GL_TEXTURE_2D;
    if (r.size() != pm.size())
        target = GL_TEXTURE_2D;
    d->drawable.bindTexture(pm, target);

    drawTextureRect(pm.width(), pm.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    d->drawable.bindTexture(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);

    GLdouble tc_w = r.width()/pm.width();
    GLdouble tc_h = r.height()/pm.height();

    // Rotate the texture so that it is aligned correctly and the
    // wrapping is done correctly
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glRotated(180.0, 0.0, 1.0, 0.0);
    glRotated(180.0, 0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    {
	glTexCoord2d(0.0, 0.0);
	glVertex2d(r.x(), r.y());

	glTexCoord2d(tc_w, 0.0);
	glVertex2d(r.x()+r.width(), r.y());

	glTexCoord2d(tc_w, tc_h);
	glVertex2d(r.x()+r.width(), r.y()+r.height());

	glTexCoord2d(0.0, tc_h);
	glVertex2d(r.x(), r.y()+r.height());
    }
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void QOpenGLPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
    Q_D(QOpenGLPaintEngine);
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
		    ? GL_TEXTURE_RECTANGLE_NV
		    : GL_TEXTURE_2D;
    if (r.size() != image.size())
        target = GL_TEXTURE_2D;
    d->drawable.bindTexture(image, target);
    drawTextureRect(image.width(), image.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTextureRect(int tx_width, int tx_height, const QRectF &r,
					 const QRectF &sr, GLenum target)
{
    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(target);

    glBegin(GL_QUADS);
    {
	qreal x1, x2, y1, y2;
	if (target == GL_TEXTURE_2D) {
	    x1 = sr.x() / tx_width;
	    x2 = x1 + sr.width() / tx_width;
	    y1 = 1.0 - ((sr.y() / tx_height) + (sr.height() / tx_height));
	    y2 = 1.0 - (sr.y() / tx_height);
	} else {
	    x1 = sr.x();
	    x2 = sr.width();
	    y1 = sr.y();
	    y2 = sr.height();
	}

        glTexCoord2d(x1, y2);
	glVertex2d(r.x(), r.y());

        glTexCoord2d(x2, y2);
	glVertex2d(r.x()+r.width(), r.y());

        glTexCoord2d(x2, y1);
	glVertex2d(r.x()+r.width(), r.y()+r.height());

        glTexCoord2d(x1, y1);
	glVertex2d(r.x(), r.y()+r.height());
    }
    glEnd();

    glDisable(target);
    glPopAttrib();
}

#ifdef Q_WS_WIN
HDC
#else
Qt::HANDLE
#endif
QOpenGLPaintEngine::handle() const
{
    return 0;
}

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
    QPaintEngine::drawTextItem(p, ti);
}
