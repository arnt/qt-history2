/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QImage>
#include "glwidget.h"

#define WIDTH 12
#define HEIGHT 11

const char *logo[] =
{
    "..XX....X...",
    ".X..X...X...",
    "X....X.XXXXX",
    "X....X..X...",
    "X....X..X...",
    "X....X..X...",
    "X....X..X...",
    ".X..X...X...",
    "..XX.....XXX",
    "...XX.......",
    "....XX......"
};
GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setWindowTitle(tr("OpenGL framebuffer objects"));
    makeCurrent();
    fbo = new QGLFramebufferObject(512, 512);
    rot_y = rot_z = 0.0;
    scale = 1.2;
    anim = new QTimeLine(750, this);
    anim->setUpdateInterval(20);
    connect(anim, SIGNAL(valueChanged(qreal)), SLOT(animate(qreal)));
    connect(anim, SIGNAL(finished()), SLOT(animFinished()));

    svg_renderer = new QSvgRenderer(QLatin1String(":/res/bubbles.svg"), this);
    connect(svg_renderer, SIGNAL(repaintNeeded()), this, SLOT(draw()));
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

GLWidget::~GLWidget()
{
    delete fbo;
}

void GLWidget::paintEvent(QPaintEvent *)
{
    draw();
}

void GLWidget::draw()
{
    QPainter p(this); // used for text overlay

    // save the GL state QPainter expects
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // render the SVG file into our framebuffer object
    QPainter fbo_painter(fbo);
    svg_renderer->render(&fbo_painter);
    fbo_painter.end();

    // draw into the widget
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 10, 100);
    glTranslatef(0.0f, 0.0f, -15.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glPushMatrix();

    glRotatef(rot_y, 0, 1, 0);
    glRotatef(rot_z, 0, 0, 1);

    glScalef(scale/WIDTH, scale/WIDTH, scale/WIDTH);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    glTranslatef(-WIDTH+1, -HEIGHT+1, 0);
    for (int y=HEIGHT-1; y>=0; --y) {
        for (int x=0; x<WIDTH; ++x) {
            if (logo[y][x] == 'X') {
                // front side
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, fbo->texture());
                glColor3f(1.0, 1.0, 1.0);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0, 0); glVertex3f(-1, -1, 0);
                    glTexCoord2f(1, 0); glVertex3f(1, -1, 0);
                    glTexCoord2f(1, 1); glVertex3f(1, 1, 0);
                    glTexCoord2f(0, 1); glVertex3f(-1, 1, 0);
                }
                glEnd();
                glDisable(GL_TEXTURE_2D);

                // flip side
                glColor4f(0.45, 0.45, 0.45, 0.9);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(1, 0); glVertex3f(-1, -1, -0.01);
                    glTexCoord2f(1, 1); glVertex3f(-1, 1, -0.01);
                    glTexCoord2f(0, 1); glVertex3f(1, 1, -0.01);
                    glTexCoord2f(0, 0); glVertex3f(1, -1, -0.01);
                }
                glEnd();
            }
            glTranslatef(2, 0, 0);
        }
        glTranslatef(-WIDTH*2, 2, 0);
    }
    glPopMatrix();
    glScalef(1.6, 1.6, 1.6);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 1.0, 0.15);
    glBegin(GL_POLYGON);
    {
        glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
        glTexCoord2f(1, 0); glVertex3f(1, -1, -1);

        glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
        glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
    }
    glEnd();

    // restore the GL state that QPainter expects
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();

    // draw the instruction text
    p.setPen(QColor(197, 197, 197, 157));
    p.setBrush(QColor(197, 197, 197, 127));
    p.drawRect(QRect(0, 0, width(), 50));
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::black);
    const QString str1(tr("A simple OpenGL framebuffer object example."));
    const QString str2(tr("Use the mouse wheel to zoom, press buttons and move mouse to rotate, double-click to flip."));
    QFontMetrics fm(p.font());
    p.drawText(width()/2 - fm.width(str1)/2, 20, str1);
    p.drawText(width()/2 - fm.width(str2)/2, 20 + fm.lineSpacing(), str2);
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    anchor = e->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPoint diff = e->pos() - anchor;
    if (e->buttons() & Qt::LeftButton)
        rot_y += diff.x()/5.0;
    else if (e->buttons() & Qt::RightButton)
        rot_z += diff.x()/5.0;

    anchor = e->pos();
    draw();
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    e->delta() > 0 ? scale += scale*0.1 : scale -= scale*0.1;
    draw();
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    anim->start();
}

void GLWidget::animate(qreal val)
{
    rot_y = val * 180;
    draw();
}

void GLWidget::animFinished()
{
    if (anim->direction() == QTimeLine::Forward)
        anim->setDirection(QTimeLine::Backward);
    else
        anim->setDirection(QTimeLine::Forward);
}
