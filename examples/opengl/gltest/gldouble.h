/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLDOUBLE_H
#define GLDOUBLE_H

#include "glcontrolwidget.h"

class GLDouble : public GLControlWidget
{
    Q_OBJECT

public:

    GLDouble( QWidget* parent = 0, const char* name = 0, WFlags f = 0, 
	   QGLFormat form = QGLFormat ( DoubleBuffer ));
    ~GLDouble();

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );
    void                animate();
    void		mousePressEvent( QMouseEvent * );

private:
    GLfloat             spin;
    GLint               enableSpin;
};


#endif // GLDOUBLE_H
