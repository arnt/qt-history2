/****************************************************************************
**
** Definition of GLWidget class
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
