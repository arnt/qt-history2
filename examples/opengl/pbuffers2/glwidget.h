/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
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
#include <QImage>
#include <QTimeLine>
#include <QSvgRenderer>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent);
    ~GLWidget();

    void saveGLState();
    void restoreGLState();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);
    void wheelEvent(QWheelEvent *);

public slots:
    void animate(qreal);
    void animFinished();
    void draw();

private:
    QPoint anchor;
    float scale;
    float rot_x, rot_y, rot_z;
    GLuint tile_list;
    GLfloat *wave;

    QImage logo;
    QTimeLine *anim;
    QSvgRenderer *svg_renderer;

    GLuint dynamicTexture;
    QGLPixelBuffer *pbuffer;
};

