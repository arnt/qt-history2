/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/glwidget.h#3 $
**
** Definition of GLWidget class
**
** Author  : Haavard Nord (hanord@troll.no)
** Created : 960620
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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qwidget.h>


class GLWidget : public QWidget
{
    Q_OBJECT
public:
    GLWidget( QWidget *parent=0, const char *name=0 );

    static bool doubleBuffer();
    static void setDoubleBuffer( bool enable );

public slots:
    void	updateGL();

protected:
    virtual void paintGL() = 0;
    virtual void resizeGL( int w, int h ) = 0;

    void	swapBuffers();

    void	makeCurrentGL();
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );

private:
    static void initialize();
    static bool dblBuf;    
};


inline bool GLWidget::doubleBuffer()
{
    return dblBuf;
}

inline void GLWidget::updateGL()
{
    repaint( FALSE );
}


#endif // GLWIDGET_H
