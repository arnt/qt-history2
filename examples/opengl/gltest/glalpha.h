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

#ifndef GLALPHA_H
#define GLALPHA_H

#include "glcontrolwidget.h"

class GLAlpha : public GLControlWidget
{
    Q_OBJECT

public:

    GLAlpha( QWidget* parent = 0, const char* name = 0, WFlags f = 0, 
	   QGLFormat form = QGLFormat ( AlphaChannel ) );
    ~GLAlpha();

protected:
    void                mousePressEvent( QMouseEvent * );
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

private:
    int leftFirst;
    void drawLeftTriangle(void);
    void drawRightTriangle(void);
};


#endif // GLALPHA_H
