/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.h#2 $
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
    virtual void   *getVisualInfo() const;
    virtual uint    getContext( QPaintDevice * ) const;
#endif

public:
    struct Internal : /* public */ QShared {
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


class QGLContext
{
public:
    QGLContext( const QGLFormat &, QPaintDevice * );
   ~QGLContext();

    const QGLFormat &format() const;
    QPaintDevice    *device() const;

    void    makeCurrent();
    void    swapBuffers();

private:
    void    init();
    void    cleanup();
    void    doneCurrent();

    QGLFormat	  glFormat;
    QPaintDevice *paintDevice;
#if defined(_OS_WIN32_)
    bool	  current, tmpdc;
    HANDLE	  rc, win, dc;
#endif

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
