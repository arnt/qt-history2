/****************************************************************************
** $Id: $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgl.h"

#if defined(Q_WS_MAC)
#include <AGL/agl.h>
#include <AGL/aglRenderers.h>
#include <OpenGL/gl.h>

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
    return TRUE;
}



/*****************************************************************************
  QGLContext AGL-specific code
 *****************************************************************************/
QPoint posInWindow(QWidget *); //qwidget_mac.cpp
bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    GDHandle dev = GetMainDevice(); //doesn't handle multiple heads, fixme!
    vi = chooseMacVisual(dev);
    if(!vi)
	return FALSE;

    AGLPixelFormat fmt = (AGLPixelFormat)vi;
    GLint res;
    aglDescribePixelFormat(fmt, AGL_LEVEL, &res);
    glFormat.setPlane(res);
    if(deviceIsPixmap())
	res = 0;
    else
	aglDescribePixelFormat(fmt, AGL_DOUBLEBUFFER, &res);
    glFormat.setDoubleBuffer(res);
    aglDescribePixelFormat(fmt, AGL_DEPTH_SIZE, &res);
    glFormat.setDepth(res);
    aglDescribePixelFormat(fmt, AGL_RGBA, &res);
    glFormat.setRgba(res);
    aglDescribePixelFormat(fmt, AGL_ALPHA_SIZE, &res);
    glFormat.setAlpha(res);
    aglDescribePixelFormat(fmt, AGL_ACCUM_RED_SIZE, &res);
    glFormat.setAccum(res);
    aglDescribePixelFormat(fmt, AGL_STENCIL_SIZE, &res);
    glFormat.setStencil(res);
    aglDescribePixelFormat(fmt, AGL_STEREO, &res);
    glFormat.setStereo(res);

    if (shareContext && (!shareContext->isValid() || !shareContext->cx)) {
#if defined(QT_CHECK_NULL)
	    qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
#endif
	    shareContext = 0;
    }

    // sharing between rgba and color-index will give wrong colors
    if(shareContext && ( format().rgba() != shareContext->format().rgba()))
	shareContext = 0;
    AGLContext ctx = aglCreateContext(fmt, (AGLContext) (shareContext ? shareContext->cx : NULL));
    if((cx = (void *)ctx)) {
#ifdef QMAC_ONE_PIXEL_LOCK
	if(deviceIsPixmap()) {
	    QPixmap *pm = (QPixmap *)d->paintDevice;
	    PixMapHandle mac_pm = GetGWorldPixMap((GWorldPtr)pm->handle());
	    aglSetOffScreen(ctx, pm->width(), pm->height(), 
			    GetPixRowBytes(mac_pm), GetPixBaseAddr(mac_pm));
#else
#error "Not ready to handle that case, tror jeg!"
#endif	    
	} else {
	    aglSetDrawable(ctx, GetWindowPort((WindowPtr)d->paintDevice->handle()));
	}
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
    GLint attribs[20], cnt=0;
    if (deviceIsPixmap()) {
	attribs[cnt++] = AGL_PIXEL_SIZE;
	attribs[cnt++] = ((QPixmap*)d->paintDevice)->depth();
    } 
    if (glFormat.stereo())
	attribs[cnt++] = AGL_STEREO;
    if (glFormat.rgba()) {
	attribs[cnt++] = AGL_RGBA;
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = deviceIsPixmap() ? ((QPixmap*)d->paintDevice)->depth() : 32;
    } else {
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = 8;
    }
    if (glFormat.alpha()) {
	attribs[cnt++] = AGL_ALPHA_SIZE;
	attribs[cnt++] = 8;
    }
    if (glFormat.stencil()) {
	attribs[cnt++] = AGL_STENCIL_SIZE;
	attribs[cnt++] = 4;
    }
    if (deviceIsPixmap()) {
	attribs[cnt++] = AGL_OFFSCREEN;
    } else {
	if ( glFormat.doubleBuffer() ) 
	    attribs[cnt++] = AGL_DOUBLEBUFFER;
	attribs[cnt++] = AGL_ACCELERATED;
    }

    attribs[cnt] = AGL_NONE;
    AGLPixelFormat fmt;
    if(deviceIsPixmap() || !device)
	fmt = aglChoosePixelFormat(NULL, 0, attribs);
    else 
	fmt = aglChoosePixelFormat( NULL, 0, attribs);
    if(!fmt) {
	GLenum err = aglGetError();
	qDebug("got an error tex: %d", (int)err);
    } 
#if 0
    else {
	GLint res;
	int x = 0;
	for( AGLPixelFormat fmt2 = fmt; fmt2; fmt2 = aglNextPixelFormat(fmt2) ) {
	    aglDescribePixelFormat( fmt2, AGL_RENDERER_ID, &res );
	    GLint res2;
	    aglDescribePixelFormat( fmt2, AGL_ACCELERATED, &res2 );
	    qDebug("%d) 0x%08x 0x%08x %d", x++, (int)res, (int)AGL_RENDERER_GENERIC_ID, (int)res2);
	}
    }
#endif
    return fmt;
}

void QGLContext::reset()
{
    if ( !d->valid )
	return;
    if(cx)
	aglDestroyContext( (AGLContext)cx );
    cx = 0;
    if ( vi )
	aglDestroyPixelFormat((AGLPixelFormat)vi);
    vi = 0;
    d->crWin = FALSE;
    d->sharing = FALSE;
    d->valid = FALSE;
    d->transpColor = QColor();
    d->initDone = FALSE;
}

void QGLContext::makeCurrent()
{
    if ( !d->valid ) {
#if defined(QT_CHECK_STATE)
	qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
#endif
	return;
    }

    aglSetCurrentContext((AGLContext)cx);
    fixBufferRect();
    aglUpdateContext((AGLContext)cx);
    currentCtx = this;
}

void QGLContext::fixBufferRect() 
{
    if(d->paintDevice->devType() == QInternal::Widget) {
	if(!aglIsEnabled((AGLContext)cx, AGL_BUFFER_RECT))
	   aglEnable((AGLContext)cx, AGL_BUFFER_RECT);

	QWidget *w = (QWidget *)d->paintDevice;
	QRegion clp = w->clippedRegion();
	if(clp.isEmpty() || clp.isNull()) {
	    GLint offs[4] = { 0, 0, 0, 0 };
	    aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
	} else {
	    QPoint mp(posInWindow(w));
	    GLint offs[4] = { 
		mp.x(),  w->topLevelWidget()->height() - (mp.y() + w->height()),
		w->width(), w->height() };
	    aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
	}
    }
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
    if ( !d->valid )
	return;
    aglSwapBuffers((AGLContext)cx);
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor();		// Invalid color
}

static QColor cmap[256];
static bool cmap_init = FALSE;
uint QGLContext::colorIndex( const QColor&c) const
{
    int ret = -1;
    if(!cmap_init) {
	cmap_init = TRUE;
	for(int i = 0; i < 256; i++)
	    cmap[i] = QColor();
    } else {
	for(int i = 0; i < 256; i++) {
	    if(cmap[i].isValid() && cmap[i] == c) {
		ret = i;
		break;
	    }
	}
    }
    if(ret == -1) {
	for(ret = 0; ret < 256; ret++) 
	    if(!cmap[ret].isValid())
		break;
	if(ret == 256) {
	    ret = -1;
	    qDebug("whoa, that's no good..");
	} else {
	    cmap[ret] = c;

	    GLint vals[4];
	    vals[0] = ret;
	    vals[1] = c.red();
	    vals[2] = c.green();
	    vals[3] = c.blue();
	    aglSetInteger((AGLContext)cx, AGL_COLORMAP_ENTRY, vals);
	}
    }
    return (uint)(ret == -1 ? 0 : ret);
}

/*****************************************************************************
  QGLWidget AGL-specific code
 *****************************************************************************/

void QGLWidget::init( const QGLFormat& format, const QGLWidget* shareWidget )
{
    glcx = 0;
    autoSwap = TRUE;

    gl_pix = NULL;
    req_format = format;
#if defined(QMAC_OPENGL_DOUBLEBUFFER)
    dblbuf = QMAC_OPENGL_DOUBLEBUFFER;
#else
    dblbuf = 1;
#endif
    clp_serial = 0;
    macInternalDoubleBuffer(FALSE); //just get things going
    macInternalRecreateContext(format, shareWidget ? shareWidget->context() : NULL);
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
#if 0
    fixReparented();
    if ( showIt )
	show();
#endif
}

void QGLWidget::fixReparented()
{
    if(!macInternalDoubleBuffer(FALSE)) 
	macInternalRecreateContext(req_format);
}

void QGLWidget::setMouseTracking( bool enable )
{
    QWidget::setMouseTracking( enable );
}


void QGLWidget::resizeEvent( QResizeEvent * )
{
    if ( !isValid() )
	return;
    if(macInternalDoubleBuffer(FALSE))
	macInternalRecreateContext(req_format);
    makeCurrent();
    if ( !glcx->initialized() )
	glInit();
    aglUpdateContext((AGLContext)glcx->cx);
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

    const QPaintDevice *me = this;
    if(macInternalDoubleBuffer()) 
	me = gl_pix;
    if ( !context->deviceIsPixmap() && context->device() != me ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QGLWidget::setContext: Context must refer to this widget" );
#endif
	return;
    }

    if ( glcx )
	glcx->doneCurrent();
    QGLContext* oldcx = glcx;
    glcx = context;

    if ( !glcx->isValid() ) {
	const QGLContext *share = shareContext;
	if(!share && !deleteOldContext)
	    share = oldcx;
	glcx->create(share);
    }
    if ( deleteOldContext )
	delete oldcx;
}

bool QGLWidget::renderCxPm( QPixmap* )
{
    return FALSE;
}

const QGLColormap & QGLWidget::colormap() const
{
    return cmap;
}

void QGLWidget::setColormap( const QGLColormap & )
{
}

void QGLWidget::cleanupColormaps()
{	
}

bool QGLWidget::macInternalDoubleBuffer(bool fix)
{
#if !defined(QMAC_OPENGL_DOUBLEBUFFER)
    if(clippedSerial() != clp_serial) {
	QRegion rgn = clippedRegion();
	clp_serial = clippedSerial();
	QPoint p = posInWindow(this);
	bool old_dblbuf = dblbuf;
	dblbuf = (rgn != QRegion(QRect(p, size())));
	if(fix && old_dblbuf != dblbuf) 
	    macInternalRecreateContext(req_format);
    }
#else
    Q_UNUSED(fix);
#endif
    return (bool)dblbuf;
}

void QGLWidget::macInternalRecreateContext(const QGLFormat& format, const QGLContext *share_ctx)
{
//    qDebug("In %s mode", dblbuf ? "Pixmap" : "Widget");
    if(dblbuf) {
	if(!gl_pix || gl_pix->width() != width() || gl_pix->height() != height()) {
	    if(gl_pix && glcx && glcx->cx) {
		aglSetDrawable((AGLContext)glcx->cx, NULL);
		delete gl_pix;
	    }
	    gl_pix = new QPixmap(width(), height(), QPixmap::BestOptim);
	}
	setContext(new QGLContext(format, gl_pix));
    } else {
	setContext(new QGLContext(format, this ), share_ctx);
	fixBufferRect();
    }
    update();
}
#endif
