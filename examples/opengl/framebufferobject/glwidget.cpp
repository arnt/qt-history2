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

#include <math.h>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setWindowTitle(tr("OpenGL framebuffer objects"));
    makeCurrent();
    fbo = new QGLFramebufferObject(512, 512);
    rot_x = rot_y = rot_z = 0.0f;
    scale = 0.1f;
    anim = new QTimeLine(750, this);
    anim->setUpdateInterval(20);
    connect(anim, SIGNAL(valueChanged(qreal)), SLOT(animate(qreal)));
    connect(anim, SIGNAL(finished()), SLOT(animFinished()));

    svg_renderer = new QSvgRenderer(QLatin1String(":/res/bubbles.svg"), this);
    connect(svg_renderer, SIGNAL(repaintNeeded()), this, SLOT(draw()));

    logo = QImage(":/res/qt4-logo.png");
    logo = logo.convertToFormat(QImage::Format_ARGB32);

    tile_list = glGenLists(1);
    glNewList(tile_list, GL_COMPILE);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
    }
    glEnd();
    glEndList();

    wave = new GLfloat[logo.width()*logo.height()];
    memset(wave, 0, logo.width()*logo.height());
    startTimer(30); // wave timer
}

GLWidget::~GLWidget()
{
    delete[] wave;
    glDeleteLists(tile_list, 1);
    delete fbo;
}

void GLWidget::paintEvent(QPaintEvent *)
{
    draw();
}

void GLWidget::draw()
{
    QPainter p(this); // used for text overlay

    // save the GL state set for QPainter
    saveGLState();

    // render the 'bubbles.svg' file into our framebuffer object
    QPainter fbo_painter(fbo);
    svg_renderer->render(&fbo_painter);
    fbo_painter.end();

    // draw into the GL widget
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 10, 100);
    glTranslatef(0.0f, 0.0f, -15.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width(), height());

    glBindTexture(GL_TEXTURE_2D, fbo->texture());
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    // draw background
    glPushMatrix();
    glScalef(1.7f, 1.7f, 1.7f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.45f);
    glCallList(tile_list);
    glPopMatrix();

    const int w = logo.width();
    const int h = logo.height();

    glRotatef(rot_x, 1.0f, 0.0f, 0.0f);
    glRotatef(rot_y, 0.0f, 1.0f, 0.0f);
    glRotatef(rot_z, 0.0f, 0.0f, 1.0f);
    glScalef(scale/w, scale/w, scale/w);

    // draw the Qt icon
    glTranslatef(-w+1, -h+1, 0.0f);
    for (int y=h-1; y>=0; --y) {
        uint *p = (uint*) logo.scanLine(y);
        uint *end = p + w;
        int  x = 0;
        while (p < end) {
            glColor4ub(qRed(*p), qGreen(*p), qBlue(*p), uchar(qAlpha(*p)*.9));
            glTranslatef(0.0f, 0.0f, wave[y*w+x]);
            glCallList(tile_list);
            glTranslatef(0.0f, 0.0f, -wave[y*w+x]);
            glTranslatef(2.0f, 0.0f, 0.0f);
            ++x;
            ++p;
        }
        glTranslatef(-w*2.0f, 2.0f, 0.0f);
    }

    // restore the GL state that QPainter expects
    restoreGLState();

    // draw the overlayed text using QPainter
    p.setPen(QColor(197, 197, 197, 157));
    p.setBrush(QColor(197, 197, 197, 127));
    p.drawRect(QRect(0, 0, width(), 50));
    p.setPen(Qt::black);
    p.setBrush(Qt::NoBrush);
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
    if (e->buttons() & Qt::LeftButton) {
        rot_x += diff.y()/5.0f;
        rot_y += diff.x()/5.0f;
    } else if (e->buttons() & Qt::RightButton) {
        rot_z += diff.x()/5.0f;
    }

    anchor = e->pos();
    draw();
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    e->delta() > 0 ? scale += scale*0.1f : scale -= scale*0.1f;
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

void GLWidget::saveGLState()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void GLWidget::restoreGLState()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

#define PI 3.14159

void GLWidget::timerEvent(QTimerEvent *)
{
    if (QApplication::mouseButtons() != 0)
        return;

    static bool scale_in = true;

    if (scale_in && scale > 60.0f)
        scale_in = false;
    else if (!scale_in && scale < .5f)
        scale_in = true;

    scale = scale_in ? scale + scale*0.01f : scale-scale*0.01f;
    rot_z += 0.3f;
    rot_x += 0.1f;

    int dx, dy; // disturbance point
    float s, v, W, t;
    int i, j;
    static float wt[128][128];
    const int width = logo.width();
    const int AMP = 5;

    dx = dy = width >> 1;

    W = .3f;
    v = -4; // wave speed

    for (i = 0; i < width; ++i) {
	for ( j = 0; j < width; ++j) {
	    s = sqrt((double) ((j - dx) * (j - dx) + (i - dy) * (i - dy)));
	    wt[i][j] += 0.1f;
	    t = s / v;
            if (s != 0)
                wave[i*width + j] = AMP * sin(2 * PI * W * (wt[i][j] + t)) / (0.2*(s + 2));
            else
                wave[i*width + j] = AMP * sin(2 * PI * W * (wt[i][j] + t));
	}
    }
}
