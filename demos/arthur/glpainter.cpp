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

#include <qgl.h>
#include <qpainter.h>
#include <qlayout.h>

#include "glpainter.h"

class GLWidget : public QGLWidget
{
public:
    GLWidget(QWidget *parent);

protected:
    void initializeGL();
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);
    void mousePressEvent(QMouseEvent *) { dw->stopAnimation(); }
    void mouseReleaseEvent(QMouseEvent *) { dw->startAnimation(); }

private:
    DemoWidget *dw;
    int step;
    GLuint cubeList;
    GLuint cubeTextureId;
};

extern void drawPrimitives(DemoWidget *dw, QPainter *p, int count, double distance, int step);
extern void drawShadedCube(DemoWidget *dw, QPainter *p, int iterations, int spread, int step);

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    dw = qobject_cast<DemoWidget *>(parent);
    step = 0;
    Q_ASSERT(dw);
}

void GLWidget::initializeGL()
{
    // cubeList
    QImage tex;
    tex.load(":/res/cubelogo.png");

    cubeTextureId = bindTexture(tex);
    cubeList = glGenLists(1);
    glNewList(cubeList, GL_COMPILE);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
    {
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );

        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );

        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 0.0, 1.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 0.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 0.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );

        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );

        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );

        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 0.0, 1.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
    }
    glEnd();

    glEndList();
}

void GLWidget::timerEvent(QTimerEvent *)
{
    update();
    ++step;
}

void GLWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (dw->attribs()->antialias)
        p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient lg(0, 0, width(), height());
    lg.appendStop(0, Qt::white);
    lg.appendStop(1, Qt::black);
    p.setBrush(lg);
    p.drawRect(0, 0, width(), height());
    p.translate(width()/2, height()/2);
    p.rotate(step % 360);
    p.shear(dw->xfunc(step*0.8), dw->yfunc(step*0.8));
    p.translate(-width()/2, -height()/2);
    dw->drawBackground(&p);
    drawShadedCube(dw, &p, 2, 5, step);
    p.resetMatrix();

    drawPrimitives(dw, &p, 150, 0.3, step);
    p.setFont(QFont("helvetica", 14, QFont::Bold, true));
    p.setPen(Qt::white);
    p.drawText(75, height() - 75/3, "Arthur & OpenGL - together in harmony");


    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(0, 0, 75, 75);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.1, 1.1, -1.1, 1.1, 0.1, 10);
    glTranslatef(0, 0, -1.2f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(step % 360, 1, 0, 0);
    glRotatef(step % 360, 0, 1, 0);
    glRotatef(step % 360, 0, 0, 1);
    glTranslatef(-0.5, -0.5, -0.5);

    glColor4ub(255, 255, 255, dw->attribs()->alpha ? 127 : 255);

    glClear(GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, cubeTextureId);
    glCallList(cubeList);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

GLPainter::GLPainter(QWidget *parent)
    : DemoWidget(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setMargin(0);
    glwidget = new GLWidget(this);
    layout->addWidget(glwidget);
}

void GLPainter::startAnimation()
{
    animTimer.start(50, glwidget);
}

void GLPainter::stopAnimation()
{
    animTimer.stop();
}
