/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/src/qgl_win.cpp#1 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qgl.h"


void qwglError( const char* method, const char* func )
{
#if defined(CHECK_NULL)
    LPVOID lpMsgBuf;
    FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		  0, GetLastError(),
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPTSTR) &lpMsgBuf, 0, 0 );
    qWarning( "%s : %s failed: %s", method, func, (const char *)lpMsgBuf );
    LocalFree( lpMsgBuf );
#endif
    const char* dummy = method; // Avoid compiler warning
    dummy = func;
}


/*****************************************************************************
  QGLFormat Win32/WGL-specific code
 *****************************************************************************/

bool QGLFormat::hasOpenGL()
{
    return TRUE;
}


bool QGLFormat::hasOpenGLOverlays()
{
    return FALSE;		// Not supported yet
}


/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/


bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    bool success = TRUE;
    if ( deviceIsPixmap() ) {
	win = 0;
	dc  = paintDevice->handle();
    }
    else { 
	win = ((QWidget*)paintDevice)->winId();
	dc  = GetDC( win );
    }

    if ( !dc ) {
#if defined(CHECK_NULL)
	qWarning( "QGLContext::chooseContext(): Paint device cannot be null" );
#endif
	return FALSE;
    }

    PIXELFORMATDESCRIPTOR pfd;
    PIXELFORMATDESCRIPTOR realPfd;
    int pixelFormatId = choosePixelFormat( &pfd, dc );
    if ( pixelFormatId == 0 ) {
	qwglError( "QGLContext::chooseContext()", "ChoosePixelFormat" );
	success = FALSE;
    }
    else {
	DescribePixelFormat( dc, pixelFormatId, sizeof(PIXELFORMATDESCRIPTOR),
			     &realPfd );
	glFormat.setDoubleBuffer( realPfd.dwFlags & PFD_DOUBLEBUFFER );
	glFormat.setDepth( realPfd.cDepthBits );
	glFormat.setRgba( realPfd.iPixelType == PFD_TYPE_RGBA );
	glFormat.setAlpha( realPfd.cAlphaBits );
	glFormat.setAccum( realPfd.cAccumBits );
	glFormat.setStencil( realPfd.cStencilBits );
	glFormat.setStereo( realPfd.dwFlags & PFD_STEREO );
	glFormat.setDirectRendering( FALSE );

	if ( deviceIsPixmap() && !(realPfd.dwFlags & PFD_DRAW_TO_BITMAP) ) {
#if defined(CHECK_NULL)
	    qWarning( "QGLContext::chooseContext(): Failed to get pixmap rendering context." );
#endif
	    return FALSE;
	}
	if ( deviceIsPixmap() && 
            (((QPixmap*)paintDevice)->depth() != realPfd.cColorBits ) ) {
#if defined(CHECK_NULL)
	    qWarning( "QGLContext::chooseContext(): Failed to get pixmap rendering context of suitable depth." );
#endif
	    return FALSE;
	}

    }

    if ( success && !SetPixelFormat(dc, pixelFormatId, &realPfd) ) {
	qwglError( "QGLContext::chooseContext()", "SetPixelFormat" );
	success = FALSE;
    }

    if ( success && !(rc = wglCreateContext( dc ) ) ) {
	qwglError( "QGLContext::chooseContext()", "wglCreateContext" );
	success = FALSE;
    }

    if ( success && shareContext && shareContext->isValid() )
	sharing = ( wglShareLists( shareContext->rc, rc ) != 0 );
    
    if ( !success ) {
	rc = 0;
	dc = 0;
    }

    return success;
}


/* 

<strong>Win32 only</strong>: This virtual function chooses a pixel format
that matches the OpenGL \link setFormat() format\endlink \a
fmt. Reimplement this function in a subclass if you need a custom context.

  \warning The \a pfd pointer is really a \c PIXELFORMATDESCRIPTOR*.
  We use \c void to avoid using Windows-specific types in our header files.

  \sa chooseContext() */

int QGLContext::choosePixelFormat( void *pfd, HDC pdc )
{
    PIXELFORMATDESCRIPTOR *p = (PIXELFORMATDESCRIPTOR *)pfd;
    memset( p, 0, sizeof(PIXELFORMATDESCRIPTOR) );
    p->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    p->nVersion = 1;
    p->dwFlags  = PFD_SUPPORT_OPENGL;
    if ( deviceIsPixmap() )
	p->dwFlags |= PFD_DRAW_TO_BITMAP;
    else
	p->dwFlags |= PFD_DRAW_TO_WINDOW;
    if ( glFormat.doubleBuffer() && !deviceIsPixmap() )
	p->dwFlags |= PFD_DOUBLEBUFFER;
    if ( glFormat.stereo() )
	p->dwFlags |= PFD_STEREO;
    if ( glFormat.depth() )
	p->cDepthBits = 32;
    if ( glFormat.rgba() ) {
	p->iPixelType = PFD_TYPE_RGBA;
	if ( deviceIsPixmap() )
	    p->cColorBits = ((QPixmap*)paintDevice)->depth();
	else
	    p->cColorBits = 24;
    } else {
	p->iPixelType = PFD_TYPE_COLORINDEX;
	p->cColorBits = 8;
    }
    if ( glFormat.alpha() )
	p->cAlphaBits = 8;
    if ( glFormat.accum() )
	p->cAccumBits = p->cColorBits + p->cAlphaBits;
    if ( glFormat.stencil() )
	p->cStencilBits = 4;
    p->iLayerType = PFD_MAIN_PLANE;
    return ChoosePixelFormat( pdc, p );
}



void QGLContext::reset()
{
    if ( !valid )
	return;
    doneCurrent();
    if ( win )
	ReleaseDC( win, dc );
    wglDeleteContext( rc );
    rc  = 0;
    dc  = 0;
    win = 0;
    sharing = FALSE;
    valid = FALSE;
    initDone = FALSE;
}



//
// NOTE: In a multi-threaded environment, each thread has a current
// context. If we want to make this code thread-safe, we probably
// have to use TLS (thread local storage) for keeping current contexts.
//

void QGLContext::makeCurrent()
{
    if ( currentCtx ) {
	if ( currentCtx == this )		// already current
	    return;
	currentCtx->doneCurrent();
    }
    if ( !valid || !dc )
	return;
    if ( QColor::hPal() ) {
	SelectPalette( dc, QColor::hPal(), FALSE );
	RealizePalette( dc );
    }
    if ( !wglMakeCurrent( dc, rc ) )
	qwglError( "QGLContext::makeCurrent()", "wglMakeCurrent" );
    currentCtx = this;
}


void QGLContext::doneCurrent()
{
    if ( currentCtx != this )
	return;
    currentCtx = 0;
    wglMakeCurrent( dc, 0 );
}


void QGLContext::swapBuffers()
{
    if ( dc && glFormat.doubleBuffer() )
	SwapBuffers( dc );
}

QColor QGLContext::overlayTransparentColor() const
{
    warning( "QGLContext::overlayTransparentColor() not implemented for Windows" );
    return QColor();
}


uint QGLContext::colorIndex( const QColor& c ) const
{
    if ( isValid() ) {
	return c.pixel();		// Assumes standard pallette
    }
    return 0;
}


/*****************************************************************************
  QGLWidget Win32/WGL-specific code
 *****************************************************************************/

void QGLWidget::init( const QGLFormat& format, const QGLWidget* shareWidget )
{
    glcx = 0;
    autoSwap = TRUE;
    if ( shareWidget )
	setContext( new QGLContext( format, this ), shareWidget->context() );
    else
	setContext( new QGLContext( format, this ) );
    setBackgroundMode( NoBackground );
}

void QGLWidget::resizeEvent( QResizeEvent * )
{
    makeCurrent();
    if ( !glcx->initialized() )
	glInit();
    resizeGL( width(), height() );
}



const QGLContext* QGLWidget::overlayContext() const
{
    return 0;				// Not supported yet
}


void QGLWidget::makeOverlayCurrent()
{
    ;
}


void QGLWidget::updateOverlayGL()
{
    ;
}


void QGLWidget::setContext( QGLContext *context,
			    const QGLContext* shareContext,
			    bool deleteOldContext )
{
    if ( context == 0 ) {
#if defined(CHECK_NULL)
	qWarning( "QGLWidget::setContext: Cannot set null context" );
#endif
	return;
    }
    if ( !context->deviceIsPixmap() && context->device() != this ) {
#if defined(CHECK_STATE)
	qWarning( "QGLWidget::setContext: Context must refer to this widget" );
#endif
	return;
    }

    if ( glcx )
	glcx->doneCurrent();
    QGLContext* oldcx = glcx;
    glcx = context;

    if ( oldcx && oldcx->windowCreated() && !glcx->deviceIsPixmap() && 
	 !glcx->windowCreated() ) {
	// We already have a context and must therefore create a new
	// window since Windows does not permit setting a new OpenGL
	// context for a window that already has one set.
	destroy( TRUE, TRUE );
	create( 0, TRUE, TRUE );
    }

    if ( !glcx->isValid() )
	glcx->create( shareContext ? shareContext : oldcx );

    if ( deleteOldContext )
	delete oldcx;
}


bool QGLWidget::renderCxPm( QPixmap* )
{
    return FALSE;
}
