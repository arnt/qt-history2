/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.h#1 $
**
** Definition of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1997 by Troll Tech AS.
**
*****************************************************************************/

#ifndef QGL_H
#define QGL_H

#include <qwidget.h>
#if defined(_OS_WIN32_)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>


class QGLFormat
{
public:
    QGLFormat();
    QGLFormat( const QGLFormat & );
    virtual ~QGLFormat();

    QGLFormat &operator=( const QGLFormat & );

    bool    doubleBuffer() const;
    void    setDoubleBuffer( bool enable );

    enum ColorMode { Rgba, ColorIndex };
    ColorMode colorMode() const;
    void    setColorMode( ColorMode );

    int	    colorBits() const;
    void    setColorBits( int );

    int	    depthBits() const;
    void    setDepthBits( int );

    static const QGLFormat &defaultFormat();
    static void  setDefaultFormat( const QGLFormat & );

    static bool  hasOpenGL();

protected:
    bool    isDirty() const;
    void    setDirty( bool );
    QString createKey() const;

#if defined(_OS_WIN32_)
    virtual HANDLE  getContext( QPaintDevice * ) const;
#else
    virtual void   *getVisual() const;
    virtual uint    getContext( QPaintDevice * ) const;
#endif

public:
    struct Internal : public QShared {
	bool	    dirty;
	bool        double_buffer;
	ColorMode   color_mode;
	int	    color_bits;
	int	    depth_bits;
    };

private:
    QGLFormat( int );
    void    detach();

    Internal *data;
};


class QGLWidget : public QWidget
{
    Q_OBJECT
public:
    QGLWidget( const QGLFormat &format, QWidget *parent=0, const char *name=0 );
    QGLWidget( QWidget *parent=0, const char *name=0 );
   ~QGLWidget();

    bool	doubleBuffer() const;
    void	swapBuffers();

    void	makeCurrent();

public slots:
    void	updateGL();

protected:
    virtual void paintGL() = 0;
    virtual void resizeGL( int w, int h ) = 0;

    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
#if defined(_OS_WIN32_)
    bool	winEvent( MSG * );
#endif

private:
    QGLFormat	glf;
};


//
// QGLFormat inline functions
//

inline bool QGLFormat::doubleBuffer() const
{
    return data->double_buffer;
}

inline QGLFormat::ColorMode QGLFormat::colorMode() const
{
    return data->color_mode;
}

inline int QGLFormat::colorBits() const
{
    return data->color_bits;
}

inline int QGLFormat::depthBits() const
{
    return data->depth_bits;
}

inline bool QGLFormat::isDirty() const
{
    return data->dirty;
}

inline void QGLFormat::setDirty( bool dirty )
{
    data->dirty = dirty;
}

//
// QGLWidget inline functions
//

inline bool QGLWidget::doubleBuffer() const
{
    return glf.doubleBuffer();
}

inline void QGLWidget::updateGL()
{
    repaint( FALSE );
}


#endif // QGL_H
