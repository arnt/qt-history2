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
** This is a simple QGLWidget displaying an openGL alpha 
**
****************************************************************************/

#ifndef GLACCUM_H
#define GLACCUM_H

#include "glcontrolwidget.h"

class GLAccum : public GLControlWidget
{
    Q_OBJECT

public:

    GLAccum( QWidget* parent = 0, const char* name = 0, WFlags f = 0, 
	   QGLFormat form = QGLFormat ( AlphaChannel ) );
    ~GLAccum();

protected:
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

private:
    void                accFrustum(GLdouble left, GLdouble right, 
				   GLdouble bottom, GLdouble top, 
				   GLdouble nnear, GLdouble ffar, 
				   GLdouble pixdx, GLdouble pixdy, 
				   GLdouble eyedx, GLdouble eyedy, 
     				   GLdouble focus);
    void                accPerspective(GLdouble fovy, GLdouble aspect,
				       GLdouble nnear, GLdouble ffar, 
				       GLdouble pixdx, GLdouble pixdy,
				       GLdouble eyedx, GLdouble eyedy, 
				       GLdouble focus);
    void                displayObjects(void);
};


#endif // GLACCUM_H
