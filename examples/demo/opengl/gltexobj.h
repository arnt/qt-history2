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

#include "glcontrolwidget.h"

class GLTexobj : public GLControlWidget
{
    Q_OBJECT

public:
    GLTexobj( QWidget* parent, const char* name = 0, Qt::WFlags f = 0 );
    ~GLTexobj();

protected:
    void		animate();
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject( const QImage& tex1, const QImage& tex2 );

    void		setRotationImpulse( double x, double y, double z );

private:
    GLuint object;
    double impX, impY, impZ;
};

#endif // GLTEXOBJ_H
