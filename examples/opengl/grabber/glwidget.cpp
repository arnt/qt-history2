#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "glwidget.h"

GLWidget::GLWidget(QWidget  * parent)
    : QGLWidget(parent)
{
    gear1 = 0;
    gear2 = 0;
    gear3 = 0;
    xRot = 0;
    yRot = 0;
    zRot = 0;
    gear1Rot = 0;

    QTimer  * timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advanceGears()));
    timer->start(20);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(gear1, 1);
    glDeleteLists(gear2, 1);
    glDeleteLists(gear3, 1);
}

void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::initializeGL()
{
    static const GLfloat lightPos[4] = { 5.0, 5.0, 10.0, 1.0 };
    static const GLfloat reflectance1[4] = { 0.8, 0.1, 0.0, 1.0 };
    static const GLfloat reflectance2[4] = { 0.0, 0.8, 0.2, 1.0 };
    static const GLfloat reflectance3[4] = { 0.2, 0.2, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    gear1 = makeGear(reflectance1, 1.0, 4.0, 1.0, 0.7, 20);
    gear2 = makeGear(reflectance2, 0.5, 2.0, 2.0, 0.7, 10);
    gear3 = makeGear(reflectance3, 1.3, 2.0, 0.5, 0.7, 10);

    glEnable(GL_NORMALIZE);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

    glPushMatrix();
    glTranslated(-3.0, -2.0, 0.0);
    glRotated(gear1Rot / 16.0, 0.0, 0.0, 1.0);
    glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(3.1, -2.0, 0.0);
    glRotated(-2.0 * (gear1Rot / 16.0)-9.0, 0.0, 0.0, 1.0);
    glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-3.1, 2.2, -1.8);
    glRotated(90.0, 1.0, 0.0, 0.0);
    glRotated(2.0 * (gear1Rot / 16.0)-2.0, 0.0, 0.0, 1.0);
    glCallList(gear3);
    glPopMatrix();

    glPopMatrix();
}

void GLWidget::resizeGL(int width, int height)
{
#if 0
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
#else
    GLdouble w = (float) width / (float) height;
    GLdouble h = 1.0;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-w, w, -h, h, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -40.0);
#endif
}

void GLWidget::mousePressEvent(QMouseEvent  * event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent  * event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}

void GLWidget::advanceGears()
{
    gear1Rot += 2 * 16;
    updateGL();
}

GLuint GLWidget::makeGear(const GLfloat *reflectance, GLdouble innerRadius,
                          GLdouble outerRadius, GLdouble thickness,
                          GLdouble toothSize, GLint toothCount)
{
    const double Pi = 3.14159265358979323846;

    GLdouble r0 = innerRadius;
    GLdouble r1 = outerRadius - toothSize / 2.0;
    GLdouble r2 = outerRadius + toothSize / 2.0;
    GLdouble delta = (2.0 * Pi / toothCount) / 4.0;
    GLdouble z = thickness / 2.0;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, reflectance);

    glShadeModel(GL_FLAT);

    int i;

    for (int j = 0; j < 2; ++j) {
        GLdouble sign = (j == 0) ? +1.0 : -1.0;

        glNormal3d(0.0, 0.0, sign);

        glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= toothCount; ++i) {
            GLdouble angle = i * 2.0 * Pi / toothCount;
	    glVertex3d(r0 * cos(angle), r0 * sin(angle), sign * z);
	    glVertex3d(r1 * cos(angle), r1 * sin(angle), sign * z);
	    glVertex3d(r0 * cos(angle), r0 * sin(angle), sign * z);
	    glVertex3d(r1 * cos(angle + 3 * delta), r1 * sin(angle + 3 * delta),
                       sign * z);
        }
        glEnd();

        glBegin(GL_QUADS);
        for (i = 0; i < toothCount; ++i) {
            GLdouble angle = i * 2.0 * Pi / toothCount;
	    glVertex3d(r1 * cos(angle), r1 * sin(angle), sign * z);
	    glVertex3d(r2 * cos(angle + delta), r2 * sin(angle + delta),
                       sign * z);
	    glVertex3d(r2 * cos(angle + 2 * delta), r2 * sin(angle + 2 * delta),
                       sign * z);
	    glVertex3d(r1 * cos(angle + 3 * delta), r1 * sin(angle + 3 * delta),
                       sign * z);
        }
        glEnd();
    }

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < toothCount; ++i) {
        for (int j = 0; j < 2; ++j) {
            GLdouble s1 = r1;
            GLdouble s2 = r2;
            if (j == 0)
                qSwap(s1, s2);

            GLdouble angle = ((i * 2.0) + j) * Pi / toothCount;

	    glNormal3d(cos(angle), sin(angle), 0.0);
	    glVertex3d(s2 * cos(angle), s2 * sin(angle), +z);
	    glVertex3d(s2 * cos(angle), s2 * sin(angle), -z);

	    glNormal3d(s1 * sin(angle + delta) - s2 * sin(angle),
                       s2 * cos(angle) - s1 * cos(angle + delta), 0.0);
	    glVertex3d(s1 * cos(angle + delta), s1 * sin(angle + delta), +z);
	    glVertex3d(s1 * cos(angle + delta), s1 * sin(angle + delta), -z);
        }
    }
    glVertex3d(r1, 0.0, +z);
    glVertex3d(r1, 0.0, -z);
    glEnd();

    glShadeModel(GL_SMOOTH);

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= toothCount; ++i) {
	GLdouble angle = i * 2.0 * Pi / toothCount;
	glNormal3d(-cos(angle), -sin(angle), 0.0);
	glVertex3d(r0 * cos(angle), r0 * sin(angle), -thickness / 2.0);
	glVertex3d(r0 * cos(angle), r0 * sin(angle), thickness / 2.0);
    }
    glEnd();

    glEndList();

    return list;
}

void GLWidget::normalizeAngle(int  * angle)
{
    while ( * angle < 0)
         * angle += 360 * 16;
    while ( * angle > 360 * 16)
         * angle -= 360 * 16;
}
