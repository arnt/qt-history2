/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/cube/globjwin.h#1 $
**
** Definition of GLObjectWindow widget class
** The GLObjectWindow contains a GLCube and three sliders connected to
** the GLCube's rotation slots.
**
****************************************************************************/

#ifndef GLOBJWIN_H
#define GLOBJWIN_H

#include <qwidget.h>

/*
  Definition of CubeMonitor widget class. 
*/


class GLObjectWindow : public QWidget
{
    Q_OBJECT;

public:
    GLObjectWindow( QWidget* parent = 0, const char* name = 0 );

};


#endif
