/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/src/qgl.h#8 $
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


#define QGL_VERSION	200
#define QGL_VERSION_STR	"2.0b"

const char *qGLVersion();


#ifndef QT_H
#include <qwidget.h>
#endif // QT_H

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
    QGLFormat( bool doubleBuffer=TRUE );
    QGLFormat( const QGLFormat& f );
    virtual ~QGLFormat();

    QGLFormat&		operator=( const QGLFormat& f );

    bool    		doubleBuffer() const;
    void    		setDoubleBuffer( bool enable );
    bool    		depth() const;
    void    		setDepth( bool enable );
    bool    		rgba() const;
    void    		setRgba( bool enable );
    bool    		alpha() const;
    void    		setAlpha( bool enable );
    bool    		accum() const;
    void    		setAccum( bool enable );
    bool    		stencil() const;
    void    		setStencil( bool enable );
    bool    		stereo() const;
    void    		setStereo( bool enable );

    static const	QGLFormat &defaultFormat();
    static void		setDefaultFormat( const QGLFormat& f );

    static bool		hasOpenGL();

private:

    struct FormatFlags : public QShared {
	bool	doubleBuffer;
	bool	depth;
	bool	rgba;
	bool	alpha;
	bool	accum;
	bool	stencil;
	bool	stereo;
    };

    void		detach();
    FormatFlags*	data;
};


class QGLContext
{
public:
    QGLContext( const QGLFormat& format, QPaintDevice* device );
    virtual ~QGLContext();

    bool		create( const QGLContext* shareContext = 0 );
    bool		isValid() const;
    void		reset();

    const QGLFormat&	format() const;
    void		setFormat( const QGLFormat& format );

    void		makeCurrent();
    void		swapBuffers();

    QPaintDevice*	device() const;

protected:
    bool		chooseContext( const QGLContext* shareContext = 0 );
    void		doneCurrent();

#if defined(Q_WGL)
    virtual int		choosePixelFormat( void* pfd );
#elif defined(Q_GLX)
    virtual void*	chooseVisual();
#endif

protected:
#if defined(Q_WGL)
    HANDLE		rc;
    HANDLE		dc;
    HANDLE		win;
#elif defined(Q_GLX)
    void*		vi;
    void*		cx;
#endif

private:
    bool		valid;
    QGLFormat		glFormat;
    QPaintDevice*	paintDevice;

    friend class QGLWidget;

private:	// Disabled copy constructor and operator=
    QGLContext() {}
    QGLContext( const QGLContext& ) {}
    QGLContext&		operator=( const QGLContext& ) { return *this; }
};


class QGLWidget : public QWidget
{
    Q_OBJECT
public:
    QGLWidget( QWidget* parent=0, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
    QGLWidget( const QGLFormat& format, QWidget* parent=0, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
   ~QGLWidget();

    bool		isValid() const;

    void		makeCurrent();
    bool		doubleBuffer() const;
    void		swapBuffers();

    const QGLFormat&	format() const;
    void		setFormat( const QGLFormat& format );

    const QGLContext*	context() const;
    void		setContext( QGLContext* context, 
				    const QGLContext* shareContext = 0 );

public slots:
    void		updateGL();

protected:
    virtual void	initializeGL();
    virtual void	paintGL();
    virtual void	resizeGL( int w, int h );

    void		paintEvent( QPaintEvent* );
    void		resizeEvent( QResizeEvent* );

private:
    void		gl_init();
    bool		initDone;
    QGLContext*		glcx;

private:	// Disabled copy constructor and operator=
    QGLWidget( const QGLWidget& ) {}
    QGLWidget&		operator=( const QGLWidget& ) { return *this; }
};


//
// QGLFormat inline functions
//

inline bool QGLFormat::doubleBuffer() const
{
    return data->doubleBuffer;
}

inline bool QGLFormat::depth() const
{
    return data->depth;
}

inline bool QGLFormat::rgba() const
{
    return data->rgba;
}

inline bool QGLFormat::alpha() const
{
    return data->alpha;
}

inline bool QGLFormat::accum() const
{
    return data->accum;
}

inline bool QGLFormat::stencil() const
{
    return data->stencil;
}

inline bool QGLFormat::stereo() const
{
    return data->stereo;
}

//
// QGLContext inline functions
//

inline bool QGLContext::isValid() const
{
    return valid;
}
inline const QGLFormat& QGLContext::format() const
{
    return glFormat;
}

inline QPaintDevice* QGLContext::device() const
{
    return paintDevice;
}

//
// QGLWidget inline functions
//

inline bool QGLWidget::isValid() const
{
    return glcx->isValid();
}

inline void QGLWidget::makeCurrent()
{
    glcx->makeCurrent();
}

inline bool QGLWidget::doubleBuffer() const
{
    return glcx->format().doubleBuffer();
}

inline void QGLWidget::swapBuffers()
{
    glcx->swapBuffers();
}

inline const QGLFormat &QGLWidget::format() const
{
    return glcx->format();
}

inline const QGLContext *QGLWidget::context() const
{
    return glcx;
}

inline void QGLWidget::updateGL()
{
    repaint( FALSE );
}


#endif // QGL_H
