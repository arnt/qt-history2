/****************************************************************************
**
** Implementation of the QOpenGLPaintEngine class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qpainter_p.h>
#include "qapplication.h"
#include "qbrush.h"
#include "qgl.h"
#include "qmap.h"
#include "qpaintengine_opengl.h"
#include "qpen.h"
#include "private/qpaintengine_p.h"

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine);
public:
    QOpenGLPaintEnginePrivate()
    {
	rop = Qt::CopyROP;
	dev = 0;
    }

    QGLWidget *dev;
    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::RasterOp rop;
    Qt::BGMode bgmode;
};

#define d d_func()
#define q q_func()

#define dgl d->dev

QOpenGLPaintEngine::QOpenGLPaintEngine(const QPaintDevice *)
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
		   GCCaps(CoordTransform | PenWidthTransform | PixmapTransform))
{
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev, QPainterState *state, bool begin)
{
    Q_ASSERT(static_cast<const QGLWidget *>(pdev));
    dgl = (QGLWidget *)(pdev);
    dgl->setAutoBufferSwap(false);
    setActive(true);

    dgl->makeCurrent();
    dgl->qglClearColor(state->bgBrush.color());
    glShadeModel(GL_FLAT);
    glViewport(0, 0, dgl->width(), dgl->height());
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, dgl->width(), dgl->height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return true;
}

bool QOpenGLPaintEngine::end()
{
    glFlush();
    dgl->swapBuffers();
    return true;
}

void QOpenGLPaintEngine::updatePen(QPainterState *ps)
{
    dgl->makeCurrent();
    dgl->qglColor(ps->pen.color());
    d->cpen = ps->pen;
    d->cbrush = ps->brush;
    if (ps->pen.width() == 0)
	glLineWidth(1);
    else
	glLineWidth(ps->pen.width());
}

void QOpenGLPaintEngine::updateBrush(QPainterState *ps)
{
    d->cbrush = ps->brush;
    
    // all GL polygon stipple patterns needs to be 32x32 bit
    static const GLubyte dense1_pat[] = { 
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff,
	0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    
    static const uchar dense2_pat[] = { 
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff,
	0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    
    static const uchar dense3_pat[] = { 
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee,
	0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    
    static const uchar dense4_pat[] = { 
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    
    static const uchar dense5_pat[] = { 
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
	0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    
    static const uchar dense6_pat[] = { 
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    
    static const uchar dense7_pat[] = { 
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00,
	0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    
    static const uchar hor_pat[] = {                      // horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xff, 0xff, 0xff,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0xff,
	0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xff, 0xff, 0xff,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    static const uchar ver_pat[] = {                      // vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82 };
    
    static const uchar cross_pat[] = {                    // cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0xff, 0xff, 0xff,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20,	0x08, 0x82, 0x20, 0xff,
	0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82,
	0x20, 0xff, 0xff, 0xff,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82,
	0x20, 0x08, 0x82, 0x20,	0x08, 0x82, 0x20, 0x08,
	0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82 };
    
    static const uchar bdiag_pat[] = {                    // backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01,	0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01,	0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01,	0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01,	0x80, 0x80, 0x40, 0x40,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    
    static const uchar fdiag_pat[] = {                    // forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40,	0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40,	0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40,	0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40,	0x80, 0x80, 0x01, 0x01,
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    
    static const uchar dcross_pat[] = {                   // diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41,	0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41,	0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41,	0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41,	0x80, 0x80, 0x41, 0x41,
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    
    static const uchar * const pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
	dense6_pat, dense7_pat, hor_pat, ver_pat, cross_pat, bdiag_pat,
	fdiag_pat, dcross_pat };
    
    int bs = d->cbrush.style();
    if (bs >= Dense1Pattern && bs <= DiagCrossPattern) {
	glEnable(GL_POLYGON_STIPPLE);
	glPolygonStipple(pat_tbl[bs - Dense1Pattern]);
    } else {
	glDisable(GL_POLYGON_STIPPLE);
    }
}

void QOpenGLPaintEngine::updateFont(QPainterState *ps)
{

}

void QOpenGLPaintEngine::updateRasterOp(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->rop = ps->rasterOp;
}

void QOpenGLPaintEngine::updateBackground(QPainterState *ps)
{
    dgl->makeCurrent();
    dgl->qglClearColor(ps->bgBrush.color());
    d->bgmode = ps->bgMode;
    d->bgbrush = ps->bgBrush;
}

void QOpenGLPaintEngine::updateXForm(QPainterState *ps)
{
    GLfloat mat[4][4];
    QWMatrix mtx = ps->matrix;

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

void QOpenGLPaintEngine::updateClipRegion(QPainterState *ps)
{

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
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);
    if (d->cbrush.style() != NoBrush) {
 	dgl->qglColor(d->cbrush.color());
	glRecti(x, y, x+w, y+h);
 	dgl->qglColor(d->cpen.color());
	if (d->cpen.style() == NoPen)
	    return;
    }
    w--;
    h--;
    y++;

    if (d->cpen.style() != NoPen) {
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

void QOpenGLPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    dgl->makeCurrent();
    glBegin(GL_POINTS);
    {
	for (int i = index; i < npoints; ++i)
	    glVertex2i(pa[i].x(), pa[i].y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{
    GLint opmode;

    dgl->makeCurrent();
    if (xorPaint) {
	glGetIntegerv(GL_LOGIC_OP_MODE, &opmode);
	glEnable(GL_COLOR_LOGIC_OP); // ### RGBA only - fix for indexed mode
	glLogicOp(GL_XOR);
	dgl->qglColor(white);
    } else {
        if (qGray(bgColor.rgb()) < 128)
	    dgl->qglColor(white);
        else
	    dgl->qglColor(black);
    }

    glLineStipple(1, 0x5555);
    glEnable(GL_LINE_STIPPLE);

    // adjusting needed due to differences in how QPainter and OpenGL works
    QRect rr = r;
    rr.setWidth(r.width()-1);
    rr.setHeight(r.height()-1);
    rr.moveBy(0, 1);

    if (d->cpen.style() != NoPen) {
 	glBegin(GL_LINE_LOOP);
	{
	    glVertex2i(rr.x(), rr.y());
	    glVertex2i(rr.x()+rr.width(), rr.y());
	    glVertex2i(rr.x()+rr.width(), rr.y()+rr.height());
	    glVertex2i(rr.x(), rr.y()+rr.height());
	}
 	glEnd();
    }
    glDisable(GL_LINE_STIPPLE);
    if (xorPaint) {
	glDisable(GL_COLOR_LOGIC_OP);
	glLogicOp(opmode);
    }
    dgl->qglColor(d->cpen.color());
}

void QOpenGLPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    QPointArray a;

    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    w--;
    h--;
    int rxx = w*xRnd/200;
    int ryy = h*yRnd/200;
    int rxx2 = 2*rxx;
    int ryy2 = 2*ryy;
    int xx, yy;

    // ###### WWA: this should use the new makeArc (with xmat)
    a.makeEllipse(x, y, rxx2, ryy2);
    int s = a.size()/4;
    int i = 0;
    while (i < s) {
	a.point(i, &xx, &yy);
	xx += w - rxx2;
	a.setPoint(i++, xx, yy);
    }
    i = 2*s;
    while (i < 3*s) {
	a.point(i, &xx, &yy);
	yy += h - ryy2;
	a.setPoint(i++, xx, yy);
    }
    while ( i < 4*s ) {
	a.point(i, &xx, &yy);
	xx += w - rxx2;
	yy += h - ryy2;
	a.setPoint(i++, xx, yy);
    }
    drawPolyInternal(a);
}

void QOpenGLPaintEngine::drawEllipse(const QRect &r)
{
    QPointArray pa;
    pa.makeEllipse(r.x(), r.y(), r.width(), r.height());
    drawPolyInternal(pa);
}

void QOpenGLPaintEngine::drawArc(const QRect &r, int a, int alen)
{
    QPointArray pa;
    pa.makeArc(r.x(), r.y(), r.width(), r.height(), a, alen);
    drawPolyInternal(pa, false);
}

void QOpenGLPaintEngine::drawPie(const QRect &r, int a, int alen)
{
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    QPointArray pa;
    pa.makeArc(x, y, w, h, a, alen);
    int n = pa.size();
    int cx, cy;
    cx = x+w/2;
    cy = y+h/2;
    pa.resize( n+2 );
    pa.setPoint(n, cx, cy); // add legs
    pa.setPoint(n+1, pa.at(0));
    drawPolyInternal(pa);
}

void QOpenGLPaintEngine::drawChord(const QRect &r, int a, int alen)
{
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    QPointArray pa;
    pa.makeArc(x, y, w-1, h-1, a, alen); // arc polygon
    int n = pa.size();
    pa.resize(n + 1);
    pa.setPoint(n, pa.at(0)); // connect endpoints
    drawPolyInternal(pa);
}

void QOpenGLPaintEngine::drawLineSegments(const QPointArray &pa, int index, int nlines)
{
    dgl->makeCurrent();
    glBegin(GL_LINES);
    {
	for (int i = index; i < nlines*2; i+=2) {
	    glVertex2i(pa[i].x(), pa[i].y());
	    glVertex2i(pa[i+1].x(), pa[i+1].y());
	}
    }
    glEnd();
}

void QOpenGLPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    dgl->makeCurrent();
    glBegin(GL_LINE_STRIP);
    {
	for (int i = index; i < npoints; ++i)
	    glVertex2i(pa[i].x(), pa[i].y());
    }
    glEnd();
}

#define DRAW_GL_POLYGON(pa, index, npoints) \
    glBegin(GL_POLYGON); \
    { \
	for (int i = index; i < npoints; ++i) \
	    glVertex2i(pa[i].x(), pa[i].y()); \
    } \
    glEnd()

void QOpenGLPaintEngine::drawPolygon(const QPointArray &pa, bool, int index, int npoints)
{
    dgl->makeCurrent();
    dgl->qglColor(d->cbrush.color());
    DRAW_GL_POLYGON(pa, index, npoints);
    glEnd();
    dgl->qglColor(d->cpen.color());
}

void QOpenGLPaintEngine::drawPolyInternal(const QPointArray &a, bool close)
{
    if (a.size() < 2)
	return;

    int x1, y1, x2, y2; // connect last to first point
    a.point(a.size()-1, &x1, &y1);
    a.point(0, &x2, &y2);
    bool do_close = close && !(x1 == x2 && y1 == y2);

    if (close && d->cbrush.style() != NoBrush) { // draw filled polygon
	// fake background for opaque polygons with a stipple pattern
	if (d->cbrush.style() != SolidPattern && d->bgmode == Qt::OpaqueMode) {
	    dgl->qglColor(d->bgbrush.color());
	    glDisable(GL_POLYGON_STIPPLE);
	    DRAW_GL_POLYGON(a, 0, a.size());
	    glEnable(GL_POLYGON_STIPPLE);	    
	}
	dgl->qglColor(d->cbrush.color());
	drawPolygon(a, false, 0, a.size());
	if (d->cpen.style() == NoPen) { // draw fake outline
	    drawPolyline(a, 0, a.size());
	    if (do_close)
		drawLine(QPoint(x1,y1), QPoint(x2,y2));
	}
    }
    if (d->cpen.style() != NoPen) { // draw outline
	dgl->qglColor(d->cpen.color());
	drawPolyline(a, 0, a.size());
	if (do_close)
	    drawLine(QPoint(x1,y1), QPoint(x2,y2));
    }
}

void QOpenGLPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    drawPolygon(pa, false, index, npoints);
}

void QOpenGLPaintEngine::drawCubicBezier(const QPointArray &a, int index)
{
    QPointArray pa(a);
    if (index != 0 || a.size() > 4) {
	pa = QPointArray(4);
	for (int i = 0; i < 4; i++)
	    pa.setPoint(i, a.point(index + i));
    }
    if (d->cpen.style() != NoPen) {
	pa = pa.cubicBezier();
	drawPolyline(pa, 0, pa.size());
    }
}

// ### NB! assumes 32 bit ints
// returns the highest number closest to v, which is a power of 2
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
QMap<int, GLuint> tx_cache;

static void cleanup_texture_cache()
{
    GLuint *textures = new GLuint[tx_cache.size()];
    QMap<int, GLuint>::ConstIterator it;
    int i = 0;
    for(it = tx_cache.constBegin(); it != tx_cache.constEnd(); ++it)
	textures[i++] = *it;
    glDeleteTextures(tx_cache.size(), textures);
    tx_cache.clear();
    delete textures;
}

static void bind_texture_from_cache(const QPixmap &pm)
{
    if (tx_cache.size() > 25)
	cleanup_texture_cache();

    if (tx_cache.contains(pm.serialNumber())) {
	glBindTexture(GL_TEXTURE_2D, tx_cache.value(pm.serialNumber()));
    } else {
        // not cached - cache it!
	if (add_texture_cleanup) {
	    qAddPostRoutine(cleanup_texture_cache);
	    add_texture_cleanup = false;
	}

	// Scale the pixmap if needed. GL textures needs to have the
	// dimensions 2^n+2(border) x 2^m+2(border).
	QImage tx;
	int tx_w = nearest_gl_texture_size(pm.width());
	int tx_h = nearest_gl_texture_size(pm.height());
	if (tx_w != pm.width() || tx_h !=  pm.height()) {
	    QImage im = pm;
	    tx = QGLWidget::convertToGLFormat(im.scale(tx_w, tx_h));
	} else {
	    tx = QGLWidget::convertToGLFormat(pm);
	}

	GLuint tx_id;
	glGenTextures(1, &tx_id);
	glBindTexture(GL_TEXTURE_2D, tx_id);
	// ### fix: better error handling - what if there's no more room for textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx.width(), tx.height(), 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, tx.bits());
	tx_cache.insert(pm.serialNumber(), tx_id);
    }
}

void QOpenGLPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &)
{
    // see if we have this pixmap cached as a texture - if not cache it
    bind_texture_from_cache(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat c[4];
    glGetFloatv(GL_CURRENT_COLOR, c);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
    {
	glTexCoord2f(0.0, 1.0); glVertex2i(r.x(), r.y());
	glTexCoord2f(1.0, 1.0); glVertex2i(r.x()+r.width(), r.y());
	glTexCoord2f(1.0, 0.0); glVertex2i(r.x()+r.width(), r.y()+r.height());
	glTexCoord2f(0.0, 0.0); glVertex2i(r.x(), r.y()+r.height());
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glColor4f(c[0], c[1], c[2], c[3]);
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pm, const QPoint &p, bool)
{
    // see if we have this pixmap cached as a texture - if not cache it
    bind_texture_from_cache(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat c[4];
    glGetFloatv(GL_CURRENT_COLOR, c);
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
    glColor4f(c[0], c[1], c[2], c[3]);
}

void QOpenGLPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{

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
