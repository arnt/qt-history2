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

#include "private/qpaintengine_p.h"
#include "qapplication.h"
#include "qbrush.h"
#include "qgl.h"
#include "qmap.h"
#include "qpaintengine_opengl.h"
#include "qpen.h"
#include "qvarlengtharray.h"
#include <private/qpainter_p.h>

#ifdef Q_OS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif

#include <stdlib.h>

#ifndef CALLBACK // for Windows
#define CALLBACK
#endif

// define QT_GL_NO_CONCAVE_POLYGONS to remove support for drawing
// concave polygons (for speedup purposes)

//#define QT_GL_NO_CONCAVE_POLYGONS

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
    {
        dev = 0;
    }

    QGLWidget *dev;
    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::BGMode bgmode;
};

static void qt_fill_linear_gradient(const QRect &rect, const QBrush &brush);

#define d d_func()
#define q q_func()

#define dgl d->dev

QOpenGLPaintEngine::QOpenGLPaintEngine()
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
                   PaintEngineFeatures(CoordTransform
				       | PenWidthTransform
				       | PixmapTransform
				       | PixmapScale
		                       | SolidAlphaFill
				       | LinearGradients
		                       | PaintOutsidePaintEvent))
{
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_ASSERT(static_cast<const QGLWidget *>(pdev));
    dgl = (QGLWidget *)(pdev);
    dgl->setAutoBufferSwap(false);
    setActive(true);

    dgl->makeCurrent();
    dgl->qglClearColor(dgl->palette().brush(QPalette::Background));
    glClear(GL_COLOR_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    glViewport(0, 0, dgl->width(), dgl->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, dgl->width(), dgl->height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.375, 0.375, 0.0);
    return true;
}

bool QOpenGLPaintEngine::end()
{
    dgl->makeCurrent();
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
    if (pen.width() == 0)
        glLineWidth(1);
    else
        glLineWidth(pen.width());
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPoint &)
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

void QOpenGLPaintEngine::updateXForm(const QMatrix &mtx)
{
    GLfloat mat[4][4];

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

    dgl->makeCurrent();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mat[0][0]);
}

void QOpenGLPaintEngine::updateClipRegion(const QRegion &rgn, bool clip)
{
    bool useStencilBuffer = dgl->format().stencil();
    bool useDepthBuffer = dgl->format().depth() && !useStencilBuffer;

    // clipping is only supported when a stencil or depth buffer is
    // available
    if (!useStencilBuffer && !useDepthBuffer)
	return;

    dgl->makeCurrent();
    if (clip) {
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

	const QVector<QRect> rects = rgn.rects();
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
    } else {
	if (useStencilBuffer)
	    glDisable(GL_STENCIL_TEST);
	else
	    glDisable(GL_DEPTH_TEST);
    }
}

void QOpenGLPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    dgl->makeCurrent();
    if (hints & QPainter::LineAntialiasing) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    } else { // i.e. !LineAntialiasing
	glDisable(GL_LINE_SMOOTH);
    }
}

void QOpenGLPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    dgl->makeCurrent();
    glBegin(GL_LINES);
    {
        glVertex2i(p1.x(), p1.y());
        glVertex2i(p2.x(), p2.y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawRect(const QRect &r)
{
    dgl->makeCurrent();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    if (d->cbrush.style() == Qt::LinearGradientPattern) {
	qt_fill_linear_gradient(r, d->cbrush);
	if (d->cpen.style() == Qt::NoPen)
	    return;
    } else {
	if (d->cbrush.style() != Qt::NoBrush) {
	    dgl->qglColor(d->cbrush.color());
	    glRecti(x, y, x+w, y+h);
	    dgl->qglColor(d->cpen.color());
	    if (d->cpen.style() == Qt::NoPen)
		return;
	}
    }

    // The below seems strange and gives different results on
    // different systems - keep it like this for now since it seems to
    // work best with newer GL drivers (e.g. from NVidia).
    w--;
    h--;
    y++;

    if (d->cpen.style() != Qt::NoPen) {
        // Specify the outline as 4 separate lines since a quad or a
        // polygon won't give us exactly what we want
        glBegin(GL_LINES);
        {
            glVertex2i(x, y);
            glVertex2i(x+w, y);
            glVertex2i(x+w, y-1);
            glVertex2i(x+w, y+h-1);
            glVertex2i(x+w+1, y+h);
            glVertex2i(x+1, y+h);
            glVertex2i(x, y+h);
            glVertex2i(x, y);
        }
        glEnd();
    }
}

void QOpenGLPaintEngine::drawPoint(const QPoint &p)
{
    dgl->makeCurrent();
    glBegin(GL_POINTS);
    {
        glVertex2i(p.x(), p.y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawLineSegments(const QPointArray &pa)
{
    dgl->makeCurrent();
    glBegin(GL_LINES);
    {
        for (int i = 0; i < pa.size(); i+=2) {
            glVertex2i(pa[i].x(), pa[i].y());
            glVertex2i(pa[i+1].x(), pa[i+1].y());
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

static void qgl_draw_poly(const QPointArray &pa)
{
#ifndef QT_GL_NO_CONCAVE_POLYGONS
    if (!qgl_tess) {
	qgl_tess = gluNewTess();
	qAddPostRoutine(qgl_cleanup_tesselator);
    }
    QVarLengthArray<GLdouble> v(pa.size()*3);
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
	    for (int i = 0; i < pa.size(); ++i) {
		v[i*3] = (GLdouble) pa[i].x();
		v[i*3+1] = (GLdouble) pa[i].y();
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
        for (int i = 0; i < pa.size(); ++i)
	    glVertex2i(pa[i].x(), pa[i].y());
    }
    glEnd();
#endif
}


void QOpenGLPaintEngine::drawPolygon(const QPointArray &pa, PolygonDrawMode mode)
{
    dgl->makeCurrent();
    dgl->qglColor(d->cbrush.color());
    qgl_draw_poly(pa);
    dgl->qglColor(d->cpen.color());
    if (d->cpen.style() != Qt::NoPen) {
        int x1, y1, x2, y2; // connect last to first point
        pa.point(pa.size()-1, &x1, &y1);
        pa.point(0, &x2, &y2);

        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < pa.size(); ++i)
                glVertex2i(pa[i].x(), pa[i].y());
            if (mode != UnconnectedMode && !(x1 == x2 && y1 == y2))
                glVertex2i(x1, y1);
        }
        glEnd();
    }
}

// returns the highest number closest to v, which is a power of 2
// NB! assumes 32 bit ints
static int nearest_gl_texture_size(int v)
{
    int n = 0, last = 0;
    for (int s = 0; s < 32; ++s) {
        if (((v>>s) & 1) == 1) {
            ++n;
            last = s;
        }
    }
    if (n > 1)
        return 1 << (last+1);
    return 1 << last;
}

static bool add_texture_cleanup = true;
typedef QMap<int, GLuint> TextureCache;
Q_GLOBAL_STATIC(TextureCache, tx_cache)

static void cleanup_texture_cache()
{
    QVarLengthArray<GLuint> textures(tx_cache()->size());
    QMap<int, GLuint>::ConstIterator it;
    int i = 0;
    for(it = tx_cache()->constBegin(); it != tx_cache()->constEnd(); ++it)
        textures[i++] = *it;
    glDeleteTextures(tx_cache()->size(), textures.data());
    tx_cache()->clear();
}

static void bind_texture_from_cache(const QPixmap &pm)
{
    if (tx_cache()->size() > 25)
        cleanup_texture_cache();

    if (tx_cache()->contains(pm.serialNumber())) {
        glBindTexture(GL_TEXTURE_2D, tx_cache()->value(pm.serialNumber()));
    } else {
        // not cached - cache it!
        if (add_texture_cleanup)
            add_texture_cleanup = false;

        // Scale the pixmap if needed. GL textures needs to have the
        // dimensions 2^n+2(border) x 2^m+2(border).
        QImage tx;
        int tx_w = nearest_gl_texture_size(pm.width());
        int tx_h = nearest_gl_texture_size(pm.height());
        QImage im = pm.toImage();
        if (tx_w != pm.width() || tx_h !=  pm.height()) {
            tx = QGLWidget::convertToGLFormat(im.scale(tx_w, tx_h));
        } else {
            tx = QGLWidget::convertToGLFormat(im);
        }

        GLuint tx_id;
        glGenTextures(1, &tx_id);
        glBindTexture(GL_TEXTURE_2D, tx_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx.width(), tx.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, tx.bits());
        tx_cache()->insert(pm.serialNumber(), tx_id);
    }
}

void QOpenGLPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr,
                                    Qt::PixmapDrawingMode blend)
{
    if (pm.depth() == 1) {
	QPixmap tpx(pm.size());
	tpx.fill(d->bgbrush);
	QPainter p(&tpx);
	p.setPen(d->cpen);
	p.drawPixmap(0, 0, pm);
	p.end();
	drawPixmap(r, tpx, sr, blend);
	return;
    }

    // see if we have this pixmap cached as a texture - if not cache it
    bind_texture_from_cache(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
    {
        double x1 = sr.x() / (double) pm.width();
        double x2 = x1 + sr.width() / (double) pm.width();
        double y1 = sr.y() / (double) pm.height();
        double y2 = y1 + sr.height() / (double) pm.height();

        glTexCoord2f(x1, y2); glVertex2i(r.x(), r.y());
        glTexCoord2f(x2, y2); glVertex2i(r.x()+r.width(), r.y());
        glTexCoord2f(x2, y1); glVertex2i(r.x()+r.width(), r.y()+r.height());
        glTexCoord2f(x1, y1); glVertex2i(r.x(), r.y()+r.height());
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pm, const QPoint &,
					 Qt::PixmapDrawingMode)
{
    // see if we have this pixmap cached as a texture - if not cache it
    bind_texture_from_cache(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    GLfloat tc_w = (float) r.width()/pm.width();
    GLfloat tc_h = (float) r.height()/pm.height();

    // Rotate the texture so that it is aligned correctly and the
    // wrapping is done correctly
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glRotatef(180.0, 0.0, 1.0, 0.0);
    glRotatef(180.0, 0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0, 0.0); glVertex2i(r.x(), r.y());
        glTexCoord2f(tc_w, 0.0); glVertex2i(r.x()+r.width(), r.y());
        glTexCoord2f(tc_w, tc_h); glVertex2i(r.x()+r.width(), r.y()+r.height());
        glTexCoord2f(0.0, tc_h); glVertex2i(r.x(), r.y()+r.height());
    }
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
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

template <class T> void qt_swap(T &a, T &b) { T tmp=a; a=b; b=tmp; }

static void qt_fill_linear_gradient(const QRect &rect, const QBrush &brush)
{
    Q_ASSERT(brush.style() == Qt::LinearGradientPattern);

    QPoint gstart = brush.gradientStart();
    QPoint gstop  = brush.gradientStop();

    // save GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(rect.x(), rect.y(), .0);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    gstart -= rect.topLeft();
    gstop -= rect.topLeft();

    QColor gcol1 = brush.color();
    QColor gcol2 = brush.gradientColor();

    int dx = gstop.x() - gstart.x();
    int dy = gstop.y() - gstart.y();

    int rw = rect.width();
    int rh = rect.height();

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

#ifndef QT_GRAD_NO_POLY
        // Fill the area to the left of the gradient

        QPointArray leftFill;
	if (xtop1 > 0)
	    leftFill << QPoint(0, 0);
	leftFill << QPoint(xtop1+1, 0)
		 << QPoint(xbot1+1, rh);
        if (xbot1 > 0)
            leftFill << QPoint(0, rh);
	glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	qgl_draw_poly(leftFill);

        // Fill the area to the right of the gradient
        QPointArray rightFill;
	rightFill << QPoint(xtop2-1, 0);
	if (xtop2 < rw)
	    rightFill << QPoint(rw, 0);
	if (xbot2 < rw)
	    rightFill << QPoint(rw, rh);
	rightFill << QPoint(xbot2-1, rh);
	glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	qgl_draw_poly(rightFill);
#endif // QT_GRAD_NO_POLY

	glBegin(GL_POLYGON);
	{
	    glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	    glVertex2i(xbot1, rect.height());
	    glVertex2i(xtop1, 0);
	    glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	    glVertex2i(xtop2, 0);
	    glVertex2i(xbot2, rect.height());
	}
	glEnd();
    } else {
        // Fill Vertically
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
        QPointArray topFill;
        topFill << QPoint(0, yleft1+1);
	if (yleft1 > 0)
	    topFill << QPoint(0, 0);
	if (yright1 > 0)
	    topFill << QPoint(rw, 0);
	topFill << QPoint(rw, yright1+1);
	glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	qgl_draw_poly(topFill);

        QPointArray bottomFill;
	bottomFill << QPoint(0, yleft2-1);
	if (yleft2 < rh)
	    bottomFill << QPoint(0, rh);
	if (yright2 < rh)
	    bottomFill << QPoint(rw, rh);
	bottomFill << QPoint(rw, yright2-1);
	glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	qgl_draw_poly(bottomFill);
#endif // QT_GRAD_NO_POLY

	glBegin(GL_POLYGON);
	{
	    glColor4ub(gcol1.red(), gcol1.green(), gcol1.blue(), gcol1.alpha());
	    glVertex2i(0, yleft1);
	    glVertex2i(rect.width(), yright1);
	    glColor4ub(gcol2.red(), gcol2.green(), gcol2.blue(), gcol2.alpha());
	    glVertex2i(rect.width(), yright2);
	    glVertex2i(0, yleft2);
	}
	glEnd();
    }

    glPopMatrix();
    glPopAttrib();
}

void QOpenGLPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int)
{
#if defined(Q_WS_WIN) || defined (Q_WS_MAC)
    QPaintEngine::drawTextItem(p, ti, 0);
#else
    dgl->renderText(p.x(), p.y(), QString(ti.chars, ti.num_chars), painter()->font());
#endif
}
