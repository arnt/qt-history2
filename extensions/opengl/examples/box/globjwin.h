/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/box/globjwin.h#3 $
**
** Definition of GLObjectWindow widget class
** The GLObjectWindow contains a GLBox and three sliders connected to
** the GLBox's rotation slots.
**
****************************************************************************/

#ifndef GLOBJWIN_H
#define GLOBJWIN_H

#include <qwidget.h>


class GLObjectWindow : public QWidget
{
    Q_OBJECT
public:
    GLObjectWindow( QWidget* parent = 0, const char* name = 0 );

};


#endif // GLOBJWIN_H
