/****************************************************************************
**
** Definition of a simple Qt OpenGL widget.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GEAR_H
#define GEAR_H

#include <qgl.h>

class GearWidget : public QGLWidget
{
public:
    GearWidget( QWidget *parent=0, const char *name=0 );

protected:
    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
};


#endif
