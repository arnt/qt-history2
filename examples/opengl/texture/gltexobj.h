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

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLTEXOBJ_H
#define GLTEXOBJ_H

#include <qgl.h>


class GLTexobj : public QGLWidget
{
    Q_OBJECT

public:

    GLTexobj( QWidget* parent, const char* name );
    ~GLTexobj();

public slots:

    void		setXRotation( int degrees );
    void		setYRotation( int degrees );
    void		setZRotation( int degrees );
    void		toggleAnimation();

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject( const QImage& tex1, const QImage& tex2 );

private:
    bool animation;
    GLuint object;
    GLfloat xRot, yRot, zRot, scale;
    QTimer* timer;
};


#endif // GLTEXOBJ_H
