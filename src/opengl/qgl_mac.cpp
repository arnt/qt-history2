/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/src/qgl_mac.cpp#2 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses for Unix/X11 may
** use this file in accordance with the Qt Commercial License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgl.h"

#if defined(Q_WS_MAC)
#include <agl.h>
#include <gl.h>

#include <qt_mac.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qintdict.h>

/*****************************************************************************
  QGLFormat UNIX/AGL-specific code
 *****************************************************************************/

bool QGLFormat::hasOpenGL()
{
    return TRUE;
}


bool QGLFormat::hasOpenGLOverlays()
{
    return FALSE;
}



/*****************************************************************************
  QGLContext AGL-specific code
 *****************************************************************************/

bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    GDHandle dev = GetMainDevice(); //doesn't handle multiple heads, fixme!
    vi = chooseMacVisual(dev);
    if ( !vi )
	return FALSE;

    AGLPixelFormat fmt = (AGLPixelFormat)vi;
    GLint res;
    aglDescribePixelFormat( fmt, AGL_LEVEL, &res );
    glFormat.setPlane( res );
    aglDescribePixelFormat( fmt, AGL_DOUBLEBUFFER, &res );
    glFormat.setDoubleBuffer( res );
    aglDescribePixelFormat( fmt, AGL_DEPTH_SIZE, &res );
    glFormat.setDepth( res );
    aglDescribePixelFormat( fmt, AGL_RGBA, &res );
    glFormat.setRgba( res );
    aglDescribePixelFormat( fmt, AGL_ALPHA_SIZE, &res );
    glFormat.setAlpha( res );
    aglDescribePixelFormat( fmt, AGL_ACCUM_RED_SIZE, &res );
    glFormat.setAccum( res );
    aglDescribePixelFormat( fmt, AGL_STENCIL_SIZE, &res );
    glFormat.setStencil( res );
    aglDescribePixelFormat( fmt, AGL_STEREO, &res );
    glFormat.setStereo( res );

    if ( shareContext &&
	 ( !shareContext->isValid() || !shareContext->cx ) ) {
#if defined(QT_CHECK_NULL)
	    qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
#endif
	    shareContext = 0;
    }

    // sharing between rgba and color-index will give wrong colors
    if ( shareContext && ( format().rgba() != shareContext->format().rgba() ) )
	shareContext = 0;
    AGLContext ctx = aglCreateContext(fmt, (AGLContext)shareContext->cx);
    cx = (void *)ctx;
    if(ctx) {
	if(paintDevice->devType() == QInternal::Widget)
	    aglSetDrawable(ctx, GetWindowPort((WindowPtr)paintDevice->handle()));
	else
	    aglSetDrawable(ctx, (CGrafPtr)paintDevice->handle());
	return TRUE;
    }
    return FALSE;
}


/*
  <strong>Mac only</strong>: This virtual function tries to find a
  visual that matches the format, reducing the demands if the original
  request cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has spcific requirements on visual selection.

  \sa chooseContext()
*/

void *QGLContext::chooseMacVisual(GDHandle device)
{
    int pmDepth = deviceIsPixmap() ? ((QPixmap*)paintDevice)->depth() : 32;
    GLint attribs[30], cnt=0;
    if ( deviceIsPixmap() )
	attribs[cnt++] = AGL_OFFSCREEN;
    else if ( glFormat.doubleBuffer() && !deviceIsPixmap() )
	attribs[cnt++] = AGL_DOUBLEBUFFER;
    if ( glFormat.stereo() )
	attribs[cnt++] = AGL_STEREO;
    if ( glFormat.rgba() ) {
	attribs[cnt++] = AGL_RGBA;
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = pmDepth;
    } else {
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = 8;
    }
    if ( glFormat.alpha() ) {
	attribs[cnt++] = AGL_ALPHA_SIZE;
	attribs[cnt++] = 8;
    }
    if ( glFormat.stencil() ) {
	attribs[cnt++] = AGL_STENCIL_SIZE;
	attribs[cnt++] = 4;
    }
    attribs[cnt++] = AGL_ACCELERATED;
    
    AGLPixelFormat fmt = aglChoosePixelFormat(&device, cnt-1, attribs);
    return fmt;
}

void QGLContext::reset()
{
    if ( !valid )
	return;
    aglDestroyContext( (AGLContext)cx );
    cx = 0;
    if ( vi )
	aglDestroyPixelFormat((AGLPixelFormat)vi);
    vi = 0;
    crWin = FALSE;
    sharing = FALSE;
    valid = FALSE;
    transpColor = QColor();
    initDone = FALSE;
}


void QGLContext::makeCurrent()
{
    if ( !valid ) {
#if defined(QT_CHECK_STATE)
	qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
#endif
	return;
    }

    QMacSavedPortInfo::setPaintDevice(paintDevice);
    aglSetCurrentContext((AGLContext)cx);
    currentCtx = this;
}

void QGLContext::doneCurrent()
{
    if ( currentCtx != this )
	return;
    currentCtx = 0;
    aglSetCurrentContext(NULL);
}


void QGLContext::swapBuffers() const
{
    if ( !valid )
	return;
    aglSwapBuffers((AGLContext)cx);
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor();		// Invalid color
}


uint QGLContext::colorIndex( const QColor&) const
{
    return 0;
}


/*****************************************************************************
  QGLWidget AGL-specific code
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

    if ( isValid() && this->format().hasOverlay() ) {
	olcx = new QGLContext( QGLFormat::defaultOverlayFormat(), this );
        if ( !olcx->create(shareWidget ? shareWidget->overlayContext() : 0) ) {
	    delete olcx;
	    olcx = 0;
	    glcx->glFormat.setOverlay( FALSE );
	}
    }
    else {
	olcx = 0;
    }
}


void QGLWidget::reparent( QWidget* parent, WFlags f, const QPoint& p,
			  bool showIt )
{
    QWidget::reparent( parent, f, p, showIt);
}


void QGLWidget::setMouseTracking( bool enable )
{
    QWidget::setMouseTracking( enable );
}


void QGLWidget::resizeEvent( QResizeEvent * )
{
    if ( !isValid() )
	return;
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
	if ( olcx->format().doubleBuffer() ) {
	    if ( autoSwap )
		olcx->swapBuffers();
	}
	else {
	    glFlush();
	}
    }
}

void QGLWidget::setContext( QGLContext *context,
			    const QGLContext* shareContext,
			    bool deleteOldContext )
{
    if ( context == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QGLWidget::setContext: Cannot set null context" );
#endif
	return;
    }
    if ( !context->deviceIsPixmap() && context->device() != this ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QGLWidget::setContext: Context must refer to this widget" );
#endif
	return;
    }

    if ( glcx )
	glcx->doneCurrent();
    QGLContext* oldcx = glcx;
    glcx = context;

    bool createFailed = FALSE;
    if ( !glcx->isValid() ) {
	if ( !glcx->create( shareContext ? shareContext : oldcx ) )
	    createFailed = TRUE;
    }
    if ( createFailed ) {
	if ( deleteOldContext )
	    delete oldcx;
	return;
    }

    if ( glcx->windowCreated() || glcx->deviceIsPixmap() ) {
	if ( deleteOldContext )
	    delete oldcx;
	return;
    }

    bool visible = isVisible();
    if ( visible )
	hide();

    //blah? FIXME
    if ( visible )
	show();
    glcx->setWindowCreated( TRUE );
}


bool QGLWidget::renderCxPm( QPixmap* )
{
    return FALSE;
}

#endif
