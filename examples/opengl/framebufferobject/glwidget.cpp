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

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    setWindowTitle(tr("OpenGL framebuffer objects"));
    makeCurrent();
    fbo = new QGLFramebufferObject(512, 512);
    rot_y = rot_z = 0.0f;
    scale = 1.2f;
    anim = new QTimeLine(750, this);
    anim->setUpdateInterval(20);
    connect(anim, SIGNAL(valueChanged(qreal)), SLOT(animate(qreal)));
    connect(anim, SIGNAL(finished()), SLOT(animFinished()));

    svg_renderer = new QSvgRenderer(QLatin1String(":/res/bubbles.svg"), this);
    connect(svg_renderer, SIGNAL(repaintNeeded()), this, SLOT(draw()));

    logo = QImage(":/res/qt4-logo.png");
    logo = logo.convertToFormat(QImage::Format_ARGB32);
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

    // save the GL state set for QPainter
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

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
    glEnable(GL_DEPTH_TEST);

    // draw background
    glPushMatrix();
    glScalef(1.6f, 1.6f, 1.6f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.15f);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
    }
    glEnd();
    glPopMatrix();

    glRotatef(rot_y, 0.0f, 1.0f, 0.0f);
    glRotatef(rot_z, 0.0f, 0.0f, 1.0f);
    glScalef(scale/logo.width(), scale/logo.width(), scale/logo.width());

    // draw the "Qt"
    glTranslatef(-logo.width()+1, -logo.height()+1, 0.0f);
    for (int y=logo.height()-1; y>=0; --y) {
        uint *p = (uint*) logo.scanLine(y);
        uint *end = p + logo.width();
        while (p < end) {
            glColor4ub(qRed(*p), qGreen(*p), qBlue(*p), qAlpha(*p));
            glBegin(GL_QUADS);
            {
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
            }
            glEnd();
            glTranslatef(2.0f, 0.0f, 0.0f);
            ++p;
        }
        glTranslatef(-logo.width()*2.0f, 2.0f, 0.0f);
    }

    // restore the GL state that QPainter expects
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();

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
    if (e->buttons() & Qt::LeftButton)
        rot_y += diff.x()/5.0f;
    else if (e->buttons() & Qt::RightButton)
        rot_z += diff.x()/5.0f;

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
