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
// concave polygons (for speedup purposes)

//#define QT_GL_NO_CONCAVE_POLYGONS

#ifdef QT_USE_FIXED_POINT
#define qToDouble(x) (x).toDouble()
#else
#define qToDouble(x) x
#endif

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate() : bgmode(Qt::TransparentMode) {}

    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::BGMode bgmode;
    QRegion crgn;
};

static void qt_fill_linear_gradient(const QRectF &rect, const QBrush &brush);

#define d d_func()
#define q q_func()

#define dgl ((QGLWidget *)(d->pdev))

QOpenGLPaintEngine::QOpenGLPaintEngine()
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
                   PaintEngineFeatures(CoordTransform
				       | PenWidthTransform
				       | PixmapTransform
				       | PixmapScale
		                       | AlphaFill
		                       | AlphaPixmap
 				       | LinearGradientFill
 				       | FillAntialiasing
 				       | LineAntialiasing
		                       | PaintOutsidePaintEvent))
{
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_ASSERT(static_cast<const QGLWidget *>(pdev));
    d->pdev = pdev;
    dgl->setAutoBufferSwap(false);
    setActive(true);
    dgl->makeCurrent();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    dgl->qglClearColor(dgl->palette().brush(QPalette::Background).color());
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    glViewport(0, 0, dgl->width(), dgl->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, dgl->width(), dgl->height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);

    return true;
}

bool QOpenGLPaintEngine::end()
{
    dgl->makeCurrent();
    glPopAttrib();
    glFlush();
    dgl->swapBuffers();
    setActive(false);
    return true;
}

void QOpenGLPaintEngine::updatePen(const QPen &pen)
{
    dgl->makeCurrent();
    dgl->qglColor(pen.color());
    d->cpen = pen;
    if (pen.color().alpha() != 255) {
 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    }
    if (pen.widthF() < 0.000001)
        glLineWidth(1);
    else
        glLineWidth(pen.widthF());
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
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

    dgl->makeCurrent();
    d->cbrush = brush;
    if (brush.color().alpha() != 255) {
 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    }

    int bs = d->cbrush.style();
    if (bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        glEnable(GL_POLYGON_STIPPLE);
        glPolygonStipple(pat_tbl[bs - Qt::Dense1Pattern]);
    } else {
        glDisable(GL_POLYGON_STIPPLE);
    }
}

void QOpenGLPaintEngine::updateFont(const QFont &)
{
}

void QOpenGLPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    dgl->makeCurrent();
    dgl->qglClearColor(bgBrush.color());
    d->bgmode = bgMode;
    d->bgbrush = bgBrush;
}

void QOpenGLPaintEngine::updateMatrix(const QMatrix &mtx)
{
    GLdouble mat[4][4];

    mat[0][0] = qToDouble(mtx.m11());
    mat[0][1] = qToDouble(mtx.m12());
    mat[0][2] = 0;
    mat[0][3] = 0;

    mat[1][0] = qToDouble(mtx.m21());
    mat[1][1] = qToDouble(mtx.m22());
    mat[1][2] = 0;
    mat[1][3] = 0;

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = qToDouble(mtx.dx());
    mat[3][1] = qToDouble(mtx.dy());
    mat[3][2] = 0;
    mat[3][3] = 1;

    dgl->makeCurrent();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(&mat[0][0]);
}

void QOpenGLPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    bool useStencilBuffer = dgl->format().stencil();
    bool useDepthBuffer = dgl->format().depth() && !useStencilBuffer;

    // clipping is only supported when a stencil or depth buffer is
    // available
    if (!useStencilBuffer && !useDepthBuffer)
	return;

    dgl->makeCurrent();
    if (op == Qt::NoClip) {
        d->crgn = QRegion();
        glDisable(useStencilBuffer ? GL_STENCIL_TEST : GL_DEPTH_TEST);
        return;
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

    switch (op) {
    case Qt::ReplaceClip:
        d->crgn = clipRegion;
        break;
    case Qt::IntersectClip:
        if (!d->crgn.isEmpty())
            d->crgn &= clipRegion;
        else
            d->crgn = clipRegion;
        break;
    case Qt::UniteClip:
        d->crgn |= clipRegion;
        break;
    case Qt::NoClip:
        break;
    }

    const QVector<QRect> rects = d->crgn.rects();
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < rects.size(); ++i) {
        glScissor(rects.at(i).left(), dgl->height() - rects.at(i).bottom(),
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
}

void QOpenGLPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    dgl->makeCurrent();
    if (hints & QPainter::Antialiasing) {
        glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }
}

void QOpenGLPaintEngine::drawLine(const QLineF &line)
{
    dgl->makeCurrent();
    dgl->qglColor(d->cpen.color());
    glBegin(GL_LINES);
    {
        glVertex2d(qToDouble(line.x1()), qToDouble(line.y1()));
        glVertex2d(qToDouble(line.x2()), qToDouble(line.y2()));
    }
    glEnd();
}

void QOpenGLPaintEngine::drawRect(const QRectF &r)
{
    dgl->makeCurrent();

    double x = qToDouble(r.x());
    double y = qToDouble(r.y());
    double w = qToDouble(r.width());
    double h = qToDouble(r.height());
    if (d->cbrush.style() == Qt::LinearGradientPattern) {
	painter()->save();
	painter()->setClipRect(r, Qt::IntersectClip);
	syncState();
	qt_fill_linear_gradient(r, d->cbrush);
	painter()->restore();
	if (d->cpen.style() == Qt::NoPen)
	    return;
    } else if (d->cbrush.style() != Qt::NoBrush) {
        dgl->qglColor(d->cbrush.color());
        glRectf(x, y, x+w, y+h);
        if (d->cpen.style() == Qt::NoPen)
            return;
    }

    if (d->cpen.style() != Qt::NoPen) {
        // Specify the outline as 4 separate lines since a quad or a
        // polygon won't give us exactly what we want
        dgl->qglColor(d->cpen.color());
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

void QOpenGLPaintEngine::drawPoint(const QPointF &p)
{
    dgl->makeCurrent();
    glBegin(GL_POINTS);
    {
        glVertex2d(qToDouble(p.x()), qToDouble(p.y()));
    }
    glEnd();
}

void QOpenGLPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    dgl->makeCurrent();
    glBegin(GL_LINES);
    {
        for (int i = 0; i < lineCount; ++i) {
            glVertex2d(qToDouble(lines[i].x1()), qToDouble(lines[i].y1()));
            glVertex2d(qToDouble(lines[i].x2()), qToDouble(lines[i].y2()));
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

static void qgl_draw_poly(const QPointF *points, int pointCount)
{
#ifndef QT_GL_NO_CONCAVE_POLYGONS
    if (!qgl_tess) {
	qgl_tess = gluNewTess();
	qAddPostRoutine(qgl_cleanup_tesselator);
    }
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
		v[i*3] = qToDouble(points[i].x());
		v[i*3+1] = qToDouble(points[i].y());
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
    if(!pointCount)
        return;
    dgl->makeCurrent();
    dgl->qglColor(d->cbrush.color());
    if (d->cbrush.style() != Qt::NoBrush && mode != PolylineMode)
        qgl_draw_poly(points, pointCount);
    if (d->cpen.style() != Qt::NoPen) {
        dgl->qglColor(d->cpen.color());
        double x1 = qToDouble(points[pointCount - 1].x());
        double y1 = qToDouble(points[pointCount - 1].y());
        double x2 = qToDouble(points[0].x());
        double y2 = qToDouble(points[0].y());

        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < pointCount; ++i)
                glVertex2d(qToDouble(points[i].x()), qToDouble(points[i].y()));
            if (mode != PolylineMode && !(x1 == x2 && y1 == y2))
                glVertex2d(x1, y1);
        }
        glEnd();
    }
}


void QOpenGLPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                                    Qt::PixmapDrawingMode blend)
{
    if (pm.depth() == 1) {
	QPixmap tpx(pm.size());
	tpx.fill(d->bgbrush.color());
	QPainter p(&tpx);
	p.setPen(d->cpen);
	p.drawPixmap(0, 0, pm);
	p.end();
	drawPixmap(r, tpx, sr, blend);
	return;
    }
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
		    ? GL_TEXTURE_RECTANGLE_NV
		    : GL_TEXTURE_2D;
    dgl->makeCurrent();
    dgl->bindTexture(pm, target);

    drawTextureRect(pm.width(), pm.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &,
					 Qt::PixmapDrawingMode)
{
    dgl->makeCurrent();
    dgl->bindTexture(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    GLdouble tc_w = qToDouble(qreal(r.width())/pm.width());
    GLdouble tc_h = qToDouble(qreal(r.height())/pm.height());

    // Rotate the texture so that it is aligned correctly and the
    // wrapping is done correctly
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glRotated(180.0, 0.0, 1.0, 0.0);
    glRotated(180.0, 0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    {
	glTexCoord2d(0.0, 0.0);
	glVertex2d(qToDouble(r.x()), qToDouble(r.y()));

	glTexCoord2d(tc_w, 0.0);
	glVertex2d(qToDouble(r.x()+r.width()), qToDouble(r.y()));

	glTexCoord2d(tc_w, tc_h);
	glVertex2d(qToDouble(r.x()+r.width()), qToDouble(r.y()+r.height()));

	glTexCoord2d(0.0, tc_h);
	glVertex2d(qToDouble(r.x()), qToDouble(r.y()+r.height()));
    }
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void QOpenGLPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
		    ? GL_TEXTURE_RECTANGLE_NV
		    : GL_TEXTURE_2D;
    dgl->makeCurrent();
    dgl->bindTexture(image, target);
    drawTextureRect(image.width(), image.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTextureRect(int tx_width, int tx_height, const QRectF &r,
					 const QRectF &sr, GLenum target)
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(target);
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
    {
	qreal x1, x2, y1, y2;
	if (target == GL_TEXTURE_2D) {
	    x1 = sr.x() / tx_width;
	    x2 = x1 + sr.width() / tx_width;
	    y1 = sr.y() / tx_height;
	    y2 = y1 + sr.height() / tx_height;
	} else {
	    x1 = sr.x();
	    x2 = sr.width();
	    y1 = sr.y();
	    y2 = sr.height();
	}

        glTexCoord2d(qToDouble(x1), qToDouble(y2));
	glVertex2d(qToDouble(r.x()), qToDouble(r.y()));

        glTexCoord2d(qToDouble(x2), qToDouble(y2));
	glVertex2d(qToDouble(r.x()+r.width()), qToDouble(r.y()));

        glTexCoord2d(qToDouble(x2), qToDouble(y1));
	glVertex2d(qToDouble(r.x()+r.width()), qToDouble(r.y()+r.height()));

        glTexCoord2d(qToDouble(x1), qToDouble(y1));
	glVertex2d(qToDouble(r.x()), qToDouble(r.y()+r.height()));
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

static void qt_fill_linear_gradient(const QRectF &rect, const QBrush &brush)
{
    Q_ASSERT(brush.style() == Qt::LinearGradientPattern);

    const QLinearGradient *lg = static_cast<const QLinearGradient *>(brush.gradient());

    QPointF gstart = lg->start();
    QPointF gstop  = lg->finalStop();

    // save GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(qToDouble(rect.x()), qToDouble(rect.y()), .0);
    glShadeModel(GL_SMOOTH);

    QPoint goff = QPoint(qRound(rect.x()), qRound(rect.y()));

    gstart -= goff;
    gstop -= goff;

    QColor gcol1 = lg->stops().first().second;
    QColor gcol2 = lg->stops().last().second;

    int dx = qRound(gstop.x() - gstart.x());
    int dy = qRound(gstop.y() - gstart.y());

    qreal rw = rect.width();
    qreal rh = rect.height();

    if (qAbs(dx) > qAbs(dy)) { // Fill horizontally
        // Make sure we fill left to right.
        if (gstop.x() < gstart.x()) {
            qSwap(gcol1, gcol2);
            qSwap(gstart, gstop);
        }
        // Find the location where the lines covering the gradient intersect
        // the lines making up the top and bottom of the target rectangle.
        // Note: This might be outside the target rect, but that is ok.
        int xtop1, xtop2, xbot1, xbot2;
        if (dy == 0) {
            xtop1 = xbot1 = qRound(gstart.x());
            xtop2 = xbot2 = qRound(gstop.x());
        } else {
            double gamma = double(dx) / double(-dy);
            xtop1 = qRound((-gstart.y() + gamma * gstart.x() ) / gamma);
            xtop2 = qRound((-gstop.y()  + gamma * gstop.x()  ) / gamma);
            xbot1 = qRound((rh - gstart.y() + gamma * gstart.x() ) / gamma);
            xbot2 = qRound((rh - gstop.y()  + gamma * gstop.x()  ) / gamma);
            Q_ASSERT(xtop2 > xtop1);
        }

#ifndef QT_GRAD_NO_POLY
        // Fill the area to the left of the gradient
        QPolygonF leftFill;
	if (xtop1 > 0)
	    leftFill << QPointF(0, 0);
	leftFill << QPointF(xtop1+1, 0)
		 << QPointF(xbot1+1, rh);
        if (xbot1 > 0)
            leftFill << QPointF(0, rh);
	glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	qgl_draw_poly(leftFill.data(), leftFill.size());

        // Fill the area to the right of the gradient
        QPolygonF rightFill;
	rightFill << QPointF(xtop2-1, 0);
	if (xtop2 < rw)
	    rightFill << QPointF(rw, 0);
	if (xbot2 < rw)
	    rightFill << QPointF(rw, rh);
	rightFill << QPointF(xbot2-1, rh);
	glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	qgl_draw_poly(rightFill.data(), rightFill.size());
#endif // QT_GRAD_NO_POLY

	glBegin(GL_POLYGON);
	{
	    glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	    glVertex2d(xbot1, qToDouble(rect.height()));
	    glVertex2d(xtop1, 0);
	    glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	    glVertex2d(xtop2, 0);
	    glVertex2d(xbot2, qToDouble(rect.height()));
	}
	glEnd();
    } else {
        // Fill Vertically
        // Code below is a conceptually equal to the one above except that all
        // coords are swapped x <-> y.
        // Make sure we fill top to bottom...
        if (gstop.y() < gstart.y()) {
            qSwap(gstart, gstop);
            qSwap(gcol1, gcol2);
        }
        int yleft1, yleft2, yright1, yright2;
        if (dx == 0) {
            yleft1 = yright1 = qRound(gstart.y());
            yleft2 = yright2 = qRound(gstop.y());
        } else {
            double gamma = double(dy) / double(-dx);
            yleft1 = qRound((-gstart.x() + gamma * gstart.y()) / gamma);
            yleft2 = qRound((-gstop.x() + gamma * gstop.y()) / gamma);
            yright1 = qRound((rw - gstart.x() + gamma*gstart.y()) / gamma);
            yright2 = qRound((rw - gstop.x() + gamma*gstop.y()) / gamma);
            Q_ASSERT(yleft2 > yleft1);
        }

#ifndef QT_GRAD_NO_POLY
        QPolygonF topFill;
        topFill << QPointF(0, yleft1+1);
	if (yleft1 > 0)
	    topFill << QPointF(0, 0);
	if (yright1 > 0)
	    topFill << QPointF(rw, 0);
	topFill << QPointF(rw, yright1+1);
	glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	qgl_draw_poly(topFill.data(), topFill.size());

        QPolygonF bottomFill;
	bottomFill << QPointF(0, yleft2-1);
	if (yleft2 < rh)
	    bottomFill << QPointF(0, rh);
	if (yright2 < rh)
	    bottomFill << QPointF(rw, rh);
	bottomFill << QPointF(rw, yright2-1);
	glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	qgl_draw_poly(bottomFill.data(), bottomFill.size());
#endif // QT_GRAD_NO_POLY

	glBegin(GL_POLYGON);
	{
	    glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	    glVertex2d(0, yleft1);
	    glVertex2d(qToDouble(rect.width()), yright1);
	    glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	    glVertex2d(qToDouble(rect.width()), yright2);
	    glVertex2d(0, yleft2);
	}
	glEnd();
    }

    glPopMatrix();
    glPopAttrib();
}

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
#if defined(Q_WS_WIN) || defined (Q_WS_MAC)
    QPaintEngine::drawTextItem(p, ti);
#else
    dgl->renderText(qRound(p.x()), qRound(p.y()), ti.text(), painter()->font());
#endif
}
