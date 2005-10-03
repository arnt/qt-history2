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

    void drawCube(int i, GLfloat z, GLfloat ri, GLfloat jmp, GLfloat amp);
    void drawFace();
    void initCommon();
    void initPbuffer();
    void initMain();

private:
    GLfloat rot[3], xOffs[3], yOffs[3], xInc[3];
    GLuint pbufferList;
    GLuint dynamicTexture;

    QGLPbuffer *pbuffer;
};

