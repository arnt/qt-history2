#ifndef GLGEAR_H
#define GLGEAR_H

#include "glcontrolwidget.h"

class GLGear : public GLControlWidget
{
    Q_OBJECT

public:
    GLGear( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

protected:
    void animate();
    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
};

#endif // GLGEAR_H
