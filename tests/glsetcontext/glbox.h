/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
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

#ifndef GLBOX_H
#define GLBOX_H

#include <qgl.h>

class Triangles;
class GLBox : public QGLWidget
{
    Q_OBJECT

public:

    GLBox( QWidget* parent, const char* name );
    ~GLBox();

public slots:

    void setXRotation( int degrees );
    void setYRotation( int degrees );
    void setZRotation( int degrees );
    void raiseSibling();
    
protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:
    Triangles * tri;
    
    GLuint object;
    GLfloat xRot, yRot, zRot, scale;

};


#endif // GLBOX_H
