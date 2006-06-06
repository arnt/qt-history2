/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtOpenGL>

class GLWidget : public QGLWidget
{
public:
    GLWidget(QWidget *parent);
    ~GLWidget();
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void timerEvent(QTimerEvent *) { update(); }
    void mousePressEvent(QMouseEvent *) { killTimer(timerId); }
    void mouseReleaseEvent(QMouseEvent *) { timerId = startTimer(20); }

    void drawCube(int i, GLfloat z, GLfloat ri, GLfloat jmp, GLfloat amp);

private:
    GLfloat rot[3], xOffs[3], yOffs[3], xInc[3];
    GLuint pbufferList;
    GLuint cubeTexture;
    int timerId;

    QGLFramebufferObject *fbo;
};

