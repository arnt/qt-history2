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

#ifndef GLSTENCIL_H
#define GLSTENCIL_H

#include "glcontrolwidget.h"

class GLStencil : public GLControlWidget
{
    Q_OBJECT

public:

    GLStencil( QWidget* parent = 0, const char* name = 0, WFlags f = 0, 
	   QGLFormat form = QGLFormat::defaultFormat() );
    ~GLStencil();

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:
    GLuint object;
};


#endif // GLSTENCIL_H
