/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.h#4 $
**
** Definition of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1997 by Troll Tech AS.  All right reserved.
**
*****************************************************************************/

#ifndef QGL_H
#define QGL_H

#include <qwidget.h>
#if !(defined(Q_WGL) || defined(Q_GLX))
#if defined(_OS_WIN32_)
#define Q_WGL
#else
#define Q_GLX
#endif
#endif
#if defined(Q_WGL)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>


class QGLFormat
{
public:
    QGLFormat();
    QGLFormat( bool doubleBuffer, bool rgba=TRUE,
	       int colorBits=24, int depthBits=16 );
    QGLFormat( const QGLFormat & );
    virtual ~QGLFormat();

    QGLFormat &operator=( const QGLFormat & );

    bool    doubleBuffer() const;
    void    setDoubleBuffer( bool );
    bool    rgba() const;
    void    setRgba( bool );
    int	    colorBits() const;
    void    setColorBits( int );
    int	    depthBits() const;
    void    setDepthBits( int );

    static const QGLFormat &defaultFormat();
    static void  setDefaultFormat( const QGLFormat & );

    static bool  hasOpenGL();

public:
    struct Internal : /* public */ QShared {
	bool        doubleBuffer;
	bool	    rgba;
	int	    colorBits;
	int	    depthBits;
    };

private:
    void    detach();
    Internal *data;
};


class QGLContext
{
public:
    QGLContext( const QGLFormat &, QPaintDevice * );
   ~QGLContext();

    const QGLFormat &format() const;
    QPaintDevice    *device() const;

    void	  makeCurrent();
    void	  swapBuffers();

    bool	  create();

protected:
    virtual bool  chooseContext();
#if defined(Q_GLX)
    virtual bool  chooseVisual();
#endif
    virtual void  cleanup();

private:
    void	  doneCurrent();

    QGLFormat	  glFormat;
    QPaintDevice *paintDevice;
#if defined(Q_WGL)
    bool	  tmpdc;
    HANDLE	  rc, win, dc;
#endif
#if defined(Q_GLX)
    void	 *vi;
    void	 *cx;
#endif

private:
    bool	  current;
    bool	  init;

private:	// Disabled copy constructor and operator=
    QGLContext() {}
    QGLContext( const QGLContext & ) {}
    QGLContext &operator=( const QGLContext & ) { return *this; }

    friend class QGLWidget;
};


class QGLWidget : public QWidget
{
    Q_OBJECT
public:
    QGLWidget( QWidget *parent=0, const char *name=0 );
    QGLWidget( const QGLFormat &format, QWidget *parent=0, const char *name=0 );

    void	makeCurrent();
    bool	doubleBuffer() const;
    void	swapBuffers();

    QGLContext *context();

public slots:
    void	updateGL();

protected:
    virtual void paintGL() = 0;
    virtual void resizeGL( int w, int h ) = 0;

    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );

private:
    QGLContext	glContext;

private:	// Disabled copy constructor and operator=
    QGLWidget( const QGLWidget & ) {}
    QGLWidget &operator=( const QGLWidget & ) { return *this; }
};


//
// QGLFormat inline functions
//

inline bool QGLFormat::doubleBuffer() const
{
    return data->doubleBuffer;
}

inline bool QGLFormat::rgba() const
{
    return data->rgba;
}

inline int QGLFormat::colorBits() const
{
    return data->colorBits;
}

inline int QGLFormat::depthBits() const
{
    return data->depthBits;
}

//
// QGLContext inline functions
//

inline const QGLFormat &QGLContext::format() const
{
    return glFormat;
}

inline QPaintDevice *QGLContext::device() const
{
    return paintDevice;
}

//
// QGLWidget inline functions
//

inline void QGLWidget::makeCurrent()
{
    glContext.makeCurrent();
}

inline bool QGLWidget::doubleBuffer() const
{
    return glContext.format().doubleBuffer();
}

inline void QGLWidget::swapBuffers()
{
    glContext.swapBuffers();
}

inline QGLContext *QGLWidget::context()
{
    return &glContext;
}

inline void QGLWidget::updateGL()
{
    repaint( FALSE );
}


#endif // QGL_H
