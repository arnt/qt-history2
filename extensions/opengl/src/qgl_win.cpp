/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/src/qgl_win.cpp#4 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qgl.h"
#include <qpixmap.h>
#include <qapplication.h>

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
    static bool checkDone = FALSE;
    static bool hasOl = FALSE;

    if ( !checkDone ) {
	checkDone = TRUE;
	HDC dc = qt_display_dc();
	int pfiMax = DescribePixelFormat( dc, 0, 0, NULL );
	PIXELFORMATDESCRIPTOR pfd;
	for ( int pfi = 1; pfi <= pfiMax; pfi++ ) {
	    DescribePixelFormat( dc, pfi, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	    if ( pfd.bReserved > 0 && (pfd.dwFlags & PFD_SUPPORT_OPENGL) ) {
		// This format has overlays/underlays
		LAYERPLANEDESCRIPTOR lpd;
		wglDescribeLayerPlane( dc, pfi, 1,
				       sizeof(LAYERPLANEDESCRIPTOR), &lpd );
		if ( lpd.dwFlags & LPD_SUPPORT_OPENGL ) {
		    hasOl = TRUE;
		    break;
		}
	    }
	}
    }
    return hasOl;
}


/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/


bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    HDC myDc;

    if ( deviceIsPixmap() ) {
	if ( glFormat.plane() )
	    return FALSE;		// Pixmaps can't have overlay
	win = 0;
	myDc = paintDevice->handle();
    }
    else {
	win = ((QWidget*)paintDevice)->winId();
	myDc = GetDC( win );
    }

    if ( !myDc ) {
#if defined(CHECK_NULL)
	qWarning( "QGLContext::chooseContext(): Paint device cannot be null" );
#endif
	return FALSE;
    }

    if ( glFormat.plane() ) {
	pixelFormatId = ((QGLWidget*)paintDevice)->context()->pixelFormatId;
	if ( !pixelFormatId )		// I.e. the glwidget is invalid
	    return FALSE;

	rc = wglCreateLayerContext( myDc, glFormat.plane() );
	if ( !rc ) {
	    qwglError( "QGLContext::chooseContext()", "CreateLayerContext" );
	    return FALSE;
	}

	LAYERPLANEDESCRIPTOR lpfd;
	wglDescribeLayerPlane( myDc, pixelFormatId, glFormat.plane(),
			       sizeof( LAYERPLANEDESCRIPTOR ), &lpfd );
	glFormat.setDoubleBuffer( lpfd.dwFlags & LPD_DOUBLEBUFFER );
	glFormat.setDepth( lpfd.cDepthBits );
	glFormat.setRgba( lpfd.iPixelType == PFD_TYPE_RGBA );
	glFormat.setAlpha( lpfd.cAlphaBits );
	glFormat.setAccum( lpfd.cAccumBits );
	glFormat.setStencil( lpfd.cStencilBits );
	glFormat.setStereo( lpfd.dwFlags & LPD_STEREO );
	glFormat.setDirectRendering( FALSE );

	if ( glFormat.rgba() ) {
	    if ( lpfd.dwFlags & LPD_TRANSPARENT )
		transpColor = QColor( lpfd.crTransparent & 0xff,
				      (lpfd.crTransparent >> 8) & 0xff, 
				      (lpfd.crTransparent >> 16) & 0xff );
	    else
		transpColor = QColor( 0, 0, 0 );
	}
	else {
	    if ( lpfd.dwFlags & LPD_TRANSPARENT )
		transpColor = QColor( qRgb( 1, 2, 3 ), lpfd.crTransparent );
	    else
		transpColor = QColor( qRgb( 1, 2, 3 ), 0 );
	}
	return TRUE;
    }

    PIXELFORMATDESCRIPTOR pfd;
    PIXELFORMATDESCRIPTOR realPfd;
    pixelFormatId = choosePixelFormat( &pfd, myDc );
    if ( pixelFormatId == 0 ) {
	qwglError( "QGLContext::chooseContext()", "ChoosePixelFormat" );
	return FALSE;
    }
    DescribePixelFormat( myDc, pixelFormatId, sizeof(PIXELFORMATDESCRIPTOR),
			 &realPfd );
    glFormat.setDoubleBuffer( realPfd.dwFlags & PFD_DOUBLEBUFFER );
    glFormat.setDepth( realPfd.cDepthBits );
    glFormat.setRgba( realPfd.iPixelType == PFD_TYPE_RGBA );
    glFormat.setAlpha( realPfd.cAlphaBits );
    glFormat.setAccum( realPfd.cAccumBits );
    glFormat.setStencil( realPfd.cStencilBits );
    glFormat.setStereo( realPfd.dwFlags & PFD_STEREO );
    glFormat.setDirectRendering( FALSE );
    glFormat.setOverlay( realPfd.bReserved != 0 );

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

    if ( !SetPixelFormat(myDc, pixelFormatId, &realPfd) ) {
	qwglError( "QGLContext::chooseContext()", "SetPixelFormat" );
	return FALSE;
    }

    if ( !(rc = wglCreateContext( myDc ) ) ) {
	qwglError( "QGLContext::chooseContext()", "wglCreateContext" );
	return FALSE;
    }

    if ( shareContext && shareContext->isValid() )
	sharing = ( wglShareLists( shareContext->rc, rc ) != 0 );
    
    dc = myDc;

    return TRUE;
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
    if ( glFormat.hasOverlay() )
	p->bReserved = 1;
    return ChoosePixelFormat( pdc, p );
}



void QGLContext::reset()
{
    if ( !valid )
	return;
    doneCurrent();
    if ( rc )
	wglDeleteContext( rc );
    rc  = 0;
    if ( win )
	ReleaseDC( win, dc );
    dc  = 0;
    win = 0;
    pixelFormatId = 0;
    sharing = FALSE;
    valid = FALSE;
    transpColor = QColor();
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
    //### Need to do something with wglRealizeLayerPalette
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
    if ( dc && glFormat.doubleBuffer() && !deviceIsPixmap() ) {
	if ( glFormat.plane() )
	    wglSwapLayerBuffers( dc, WGL_SWAP_OVERLAY1 );  //### hardcoded ol1
	else {
	    if ( glFormat.hasOverlay() )
		wglSwapLayerBuffers( dc, WGL_SWAP_MAIN_PLANE );
	    else
		SwapBuffers( dc );
	}
    }
}


QColor QGLContext::overlayTransparentColor() const
{
    return transpColor;
}


uint QGLContext::colorIndex( const QColor& c ) const
{
    //### Needs to handle layer palette; ref qglColor()
    if ( isValid() ) {
	return c.pixel();		// Assumes standard palette
    }
    return 0;
}


/*****************************************************************************
  QGLWidget Win32/WGL-specific code
 *****************************************************************************/

void QGLWidget::init( const QGLFormat& fmt, const QGLWidget* shareWidget )
{
    glcx = 0;
    autoSwap = TRUE;
    if ( shareWidget )
	setContext( new QGLContext( fmt, this ), shareWidget->context() );
    else
	setContext( new QGLContext( fmt, this ) );
    setBackgroundMode( NoBackground );

    if ( isValid() && format().hasOverlay() ) {
	olcx = new QGLContext( QGLFormat::defaultOverlayFormat(), this );
	if ( !olcx->create() ) {
	    delete olcx;
	    olcx = 0;
	    glcx->glFormat.setOverlay( FALSE );
	}
    }
    else {
	olcx = 0;
    }
}


void QGLWidget::resizeEvent( QResizeEvent * )
{
    makeCurrent();
    if ( !glcx->initialized() )
	glInit();
    resizeGL( width(), height() );
    if ( olcx ) {
	makeOverlayCurrent();
	resizeOverlayGL( width(), height() );
    }
}



const QGLContext* QGLWidget::overlayContext() const
{
    return olcx;
}


void QGLWidget::makeOverlayCurrent()
{
    if ( olcx ) {
	olcx->makeCurrent();
	if ( !olcx->initialized() ) {
	    initializeOverlayGL();
	    olcx->setInitialized( TRUE );
	}
    }
}


void QGLWidget::updateOverlayGL()
{
    if ( olcx ) {
	makeOverlayCurrent();
	paintOverlayGL();
    }
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
