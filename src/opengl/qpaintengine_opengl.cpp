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
#include <qdebug.h>
#include <qbrush.h>
#include <qgl.h>
#include "qpaintengine_opengl.h"
#include <qpen.h>

class QOpenGLPaintEnginePrivate {
public:
    QGLWidget *dev;
    QPen cpen;
    QBrush cbrush;
};

#define dgl d->dev

QOpenGLPaintEngine::QOpenGLPaintEngine(const QPaintDevice *)
    : QPaintEngine(GCCaps(CoordTransform | PenWidthTransform | PixmapTransform))
{
    d = new QOpenGLPaintEnginePrivate;
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
    delete d;
}

bool QOpenGLPaintEngine::begin(const QPaintDevice *pdev, QPainterState *state, bool begin)
{
    Q_ASSERT(static_cast<const QGLWidget *>(pdev));
    dgl = (QGLWidget *)(pdev);
    dgl->setAutoBufferSwap(false);
    setActive(true);

    dgl->makeCurrent();
    dgl->qglClearColor(state->bgColor);
    glViewport(0, 0, dgl->width(), dgl->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, dgl->width(), dgl->height(), 0, -1, 1);
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
}

void QOpenGLPaintEngine::updateBrush(QPainterState *ps)
{
    dgl->makeCurrent();
    d->cbrush = ps->brush;
}

void QOpenGLPaintEngine::updateFont(QPainterState *ps)
{

}

void QOpenGLPaintEngine::updateRasterOp(QPainterState *ps)
{

}

void QOpenGLPaintEngine::updateBackground(QPainterState *ps)
{
    dgl->makeCurrent();
    dgl->qglClearColor(ps->bgColor);
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
//     glTranslatef(mtx.dx(), mtx.dy(), 0);
}

void QOpenGLPaintEngine::updateClipRegion(QPainterState *ps)
{

}

void QOpenGLPaintEngine::setRasterOp(RasterOp r)
{

}

void QOpenGLPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    dgl->makeCurrent();
    glBegin(GL_LINES); {
	glVertex2i(p1.x(), p1.y());
	glVertex2i(p2.x(), p2.y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawRect(const QRect &r)
{
    dgl->makeCurrent();
    if (d->cbrush.style() != NoBrush) {
	dgl->qglColor(d->cbrush.color());
	glBegin(GL_POLYGON); {
	    glVertex2i(r.x(), r.y());
	    glVertex2i(r.x()+r.width(), r.y());
	    glVertex2i(r.x()+r.width(), r.y());
	    glVertex2i(r.x()+r.width(), r.y()+r.height());
	    glVertex2i(r.x()+r.width(), r.y()+r.height());
	    glVertex2i(r.x(), r.y()+r.height());
	    glVertex2i(r.x(), r.y()+r.height());
	    glVertex2i(r.x(), r.y());
	}
	glEnd();
	dgl->qglColor(d->cpen.color());
	if (d->cpen.style() == NoPen)
	    return;
    }
    QRect rr = r;
    rr.setWidth(r.width()-1);
    rr.setHeight(r.height()-1);

    if (d->cpen.style() != NoPen) {
	glBegin(GL_LINE_LOOP); {
	    glVertex2i(rr.x(), r.y());
	    glVertex2i(rr.x()+rr.width(), rr.y());
	    glVertex2i(rr.x()+rr.width(), rr.y());
	    glVertex2i(rr.x()+rr.width(), rr.y()+rr.height());
	    glVertex2i(rr.x()+rr.width(), rr.y()+rr.height());
	    glVertex2i(rr.x(), rr.y()+rr.height());
	}
	glEnd();
    }
}

void QOpenGLPaintEngine::drawPoint(const QPoint &p)
{
    dgl->makeCurrent();
    glBegin(GL_POINTS); {
	glVertex2i(p.x(), p.y());
    }
    glEnd();

}

void QOpenGLPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    dgl->makeCurrent();
    glBegin(GL_POINTS); {
	for (int i = 0; i < pa.size(); ++i)
	    glVertex2i(pa[i].x(), pa[i].y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{

}

void QOpenGLPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{

}

void QOpenGLPaintEngine::drawEllipse(const QRect &r)
{

}

void QOpenGLPaintEngine::drawArc(const QRect &r, int a, int alen)
{

}

void QOpenGLPaintEngine::drawPie(const QRect &r, int a, int alen)
{

}

void QOpenGLPaintEngine::drawChord(const QRect &r, int a, int alen)
{

}

void QOpenGLPaintEngine::drawLineSegments(const QPointArray &, int index, int nlines)
{

}

void QOpenGLPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{

}

void QOpenGLPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{

}

void QOpenGLPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{

}

void QOpenGLPaintEngine::drawCubicBezier(const QPointArray &pa, int index)
{

}

void QOpenGLPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{

}

void QOpenGLPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{

}

void QOpenGLPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p, bool optim)
{

}

Qt::HANDLE QOpenGLPaintEngine::handle() const
{
    return 0;
}
