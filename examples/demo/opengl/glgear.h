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

#ifndef GLGEAR_H
#define GLGEAR_H

#include "glcontrolwidget.h"

class GLGear : public GLControlWidget
{
    Q_OBJECT

public:
    GLGear( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 );

protected:
    void draw();
    void animate();
    void initializeGL();
    void resizeGL( int, int );
    void paintGL();

private:
    GLfloat view_rotx, view_roty, view_rotz;
    GLint gear1, gear2, gear3;
    GLfloat angle;
};

#endif // GLGEAR_H
