/****************************************************************************
**
** Definition of Nurb widget class
**
** Copyright (C) 1996 by Troll Tech AS.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies.
** No representations are made about the suitability of this software for any
** purpose. It is provided "as is" without express or implied warranty.
**
*****************************************************************************/

#ifndef NURB_H
#define NURB_H

#include "glwidget.h"
#include <GL/gl.h>
#include <GL/glu.h>


class Nurb : public GLWidget
{
    Q_OBJECT
public:
    Nurb( QWidget *parent=0, const char *name=0 );

public slots:
    void	animate();

    void	doLeft()  { rotY -= 5; updateGL(); }
    void	doRight() { rotY += 5; updateGL(); }
    void	doUp()    { rotX += 5; updateGL(); }
    void	doDown()  { rotX -= 5; updateGL(); }

protected:
    void	paintGL();
    void	resizeGL( int, int );

private slots:
    void	animTimeout();

private:
    QTimer     *animTimer;
    int		rotX, rotY;
    static GLUnurbsObj *theNurbs;    
};


#endif // NURB_H
