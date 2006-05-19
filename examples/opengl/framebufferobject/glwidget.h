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
#include <QTimeLine>
#include <QSvgRenderer>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent);
    ~GLWidget();
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);


public slots:
    void animate(qreal);
    void animFinished();
    void draw();

private:
    QPoint anchor;
    float scale;
    float rot_y, rot_z;

    QTimeLine *anim;
    QSvgRenderer *svg_renderer;
    QGLFramebufferObject *fbo;
};

