/****************************************************************************
**
** Implementation of OpenGL classes for Qt.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgl.h"

#if defined(Q_WS_MAC)
#ifdef qDebug
#    undef qDebug
#    include <AGL/agl.h>
#    include <AGL/aglRenderers.h>
#    include <OpenGL/gl.h>
#    ifdef QT_NO_DEBUG
#        define qDebug qt_noop(),1?(void)0:qDebug
#    endif
#else
#    include <AGL/agl.h>
#    include <AGL/aglRenderers.h>
#    include <OpenGL/gl.h>
#endif


#include <private/qfontdata_p.h>
#include <private/qfontengine_p.h>
#include <qt_mac.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>
#if !defined(QMAC_OPENGL_DOUBLEBUFFER)
#include <qobjectlist.h>
#endif

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
void qgl_delete_d(const QGLWidget *); //qgl.cpp
QPoint posInWindow(QWidget *); //qwidget_mac.cpp
bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    cx = NULL;
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

    if(shareContext && (!shareContext->isValid() || !shareContext->cx)) {
	    qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
	    shareContext = 0;
    }

    // sharing between rgba and color-index will give wrong colors
    if(shareContext && ( format().rgba() != shareContext->format().rgba()))
	shareContext = 0;
    AGLContext ctx = aglCreateContext(fmt, (AGLContext) (shareContext ? shareContext->cx : NULL));
    if(!ctx) {
	GLenum err = aglGetError();
	if(err == AGL_BAD_MATCH || err == AGL_BAD_CONTEXT) {
	    if(shareContext && shareContext->cx) {
		qWarning("QOpenGL: context sharing mismatch!");
		if(!(ctx = aglCreateContext(fmt, NULL)))
		    return FALSE;
		shareContext = NULL;
	    }
	}
	if(!ctx) {
	    qDebug("QOpenGL: unable to create QGLContext");
	    return FALSE;
	}
    }
    d->sharing = shareContext && shareContext->cx;

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


/*!
  <strong>Mac only</strong>: This virtual function tries to find a
  visual that matches the format using the handle \a device, reducing
  the demands if the original request cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has specific requirements on visual selection.

  \sa chooseContext()
*/

void *QGLContext::chooseMacVisual(GDHandle device)
{
    GLint attribs[20], cnt=0;
    if(deviceIsPixmap()) {
	attribs[cnt++] = AGL_PIXEL_SIZE;
	attribs[cnt++] = ((QPixmap*)d->paintDevice)->depth();
    }
    if(glFormat.stereo())
	attribs[cnt++] = AGL_STEREO;
    if(glFormat.rgba()) {
	attribs[cnt++] = AGL_RGBA;
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = deviceIsPixmap() ? ((QPixmap*)d->paintDevice)->depth() : 32;
    } else {
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = 8;
    }
    if(glFormat.alpha()) {
	attribs[cnt++] = AGL_ALPHA_SIZE;
	attribs[cnt++] = 8;
    }
    if(glFormat.stencil()) {
	attribs[cnt++] = AGL_STENCIL_SIZE;
	attribs[cnt++] = 4;
    }
    if(deviceIsPixmap()) {
	attribs[cnt++] = AGL_OFFSCREEN;
    } else {
	if( glFormat.doubleBuffer())
	    attribs[cnt++] = AGL_DOUBLEBUFFER;
//	attribs[cnt++] = AGL_ACCELERATED;
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
	for( AGLPixelFormat fmt2 = fmt; fmt2; fmt2 = aglNextPixelFormat(fmt2)) {
	    aglDescribePixelFormat( fmt2, AGL_RENDERER_ID, &res);
	    GLint res2;
	    aglDescribePixelFormat( fmt2, AGL_ACCELERATED, &res2);
	    qDebug("%d) 0x%08x 0x%08x %d", x++, (int)res, (int)AGL_RENDERER_GENERIC_ID, (int)res2);
	}
    }
#endif
    return fmt;
}

void QGLContext::reset()
{
    if( !d->valid)
	return;
    if(cx)
	aglDestroyContext( (AGLContext)cx);
    cx = 0;
    if( vi)
	aglDestroyPixelFormat((AGLPixelFormat)vi);
    vi = 0;
    d->oldR = QRect(1, 1, 1, 1);
    d->crWin = FALSE;
    d->sharing = FALSE;
    d->valid = FALSE;
    d->transpColor = QColor();
    d->initDone = FALSE;
}

void QGLContext::makeCurrent()
{
    if( !d->valid) {
	qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
	return;
    }

    aglSetCurrentContext((AGLContext)cx);
    fixBufferRect();
    aglUpdateContext((AGLContext)cx);
    currentCtx = this;
}

void QGLContext::fixBufferRect()
{
    if(d->paintDevice->devType() == QInternal::Widget &&
       !((QWidget*)d->paintDevice)->isTopLevel()) {
	if(!aglIsEnabled((AGLContext)cx, AGL_BUFFER_RECT))
	   aglEnable((AGLContext)cx, AGL_BUFFER_RECT);

	bool update = FALSE;
	QWidget *w = (QWidget *)d->paintDevice;
	QRegion clp = w->clippedRegion();
	if(clp.isEmpty()) {
#if QT_MACOSX_VERSION >= 0x1020
	    if(aglIsEnabled((AGLContext)cx, AGL_CLIP_REGION))
		aglDisable((AGLContext)cx, AGL_CLIP_REGION);
#endif
	    if(!d->oldR.isNull()) {
		update = TRUE;
		d->oldR = QRect();
		GLint offs[4] = { 0, 0, 0, 0 };
		aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
	    }
	} else {
	    QPoint mp(posInWindow(w));
	    GLint offs[4] = {
		mp.x(), w->topLevelWidget()->height() - (mp.y() + w->height()),
		w->width(), w->height() };
	    if(d->oldR != QRect(offs[0], offs[1], offs[2], offs[3])) {
		update = TRUE;
		d->oldR = QRect(offs[0], offs[1], offs[2], offs[3]);
		aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
	    }
#if QT_MACOSX_VERSION >= 0x1020
	    if(!aglIsEnabled((AGLContext)cx, AGL_CLIP_REGION))
		aglEnable((AGLContext)cx, AGL_CLIP_REGION);
	    aglSetInteger((AGLContext)cx, AGL_CLIP_REGION, (const GLint *)clp.handle(TRUE));
#endif
	}
	if(update) {
	    aglUpdateContext((AGLContext)cx);
	    QMacSavedPortInfo::flush(w);
	}
    }
}

void QGLContext::doneCurrent()
{
    if( aglGetCurrentContext() != (AGLContext) cx)
	return;
    currentCtx = 0;
    aglSetCurrentContext(NULL);
}


void QGLContext::swapBuffers() const
{
    if( !d->valid)
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

void QGLContext::generateFontDisplayLists(const QFont & fnt, int listBase)
{
    Style fstyle = normal; //from MacTypes.h
    if(fnt.bold())
	fstyle |= bold;
    if(fnt.italic())
	fstyle |= italic;
    if(fnt.underline())
	fstyle |= underline;
    aglUseFont((AGLContext) cx, (int)fnt.handle(), fstyle, fnt.pointSize(), 0, 256, listBase);
}

/*****************************************************************************
  QGLWidget AGL-specific code
 *****************************************************************************/

#define d d_func()
#define q q_func()

void QGLWidget::init(QGLContext *ctx, const QGLWidget* shareWidget)
{
    d->slcx = d->glcx = 0;
    d->autoSwap = TRUE;

    d->gl_pix = NULL;
    d->req_format = ctx->format();
    d->pending_fix = 0;
#if defined(QMAC_OPENGL_DOUBLEBUFFER)
    d->dblbuf = QMAC_OPENGL_DOUBLEBUFFER;
#else
    d->dblbuf = 1;
#endif

    d->glcx_dblbuf = 2;
    d->clp_serial = 0;
    macInternalDoubleBuffer(FALSE); //just get things going
    macInternalRecreateContext(ctx, shareWidget ? shareWidget->context() : NULL, FALSE);

    if(isValid() && context()->format().hasOverlay()) {
	d->olcx = new QGLContext(QGLFormat::defaultOverlayFormat(), this);
        if(!d->olcx->create(shareWidget ? shareWidget->overlayContext() : 0)) {
	    delete d->olcx;
	    d->olcx = 0;
	    d->glcx->glFormat.setOverlay(FALSE);
	}
    }
    else {
	d->olcx = 0;
    }
}


bool QGLWidget::event(QEvent *e)
{
    return QWidget::event(e);
}

void QGLWidget::macWidgetChangedWindow()
{
    if(!macInternalDoubleBuffer(FALSE)) {
	delete d->glcx;
	d->glcx = NULL;
	macInternalRecreateContext(new QGLContext(d->req_format, this));
    }
}

void QGLWidget::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
}


void QGLWidget::resizeEvent(QResizeEvent *)
{
    if(!isValid())
	return;
    makeCurrent();
    if(!d->glcx->initialized())
	glInit();
    aglUpdateContext((AGLContext)d->glcx->cx);
    resizeGL(width(), height());

    if(d->olcx) {
	makeOverlayCurrent();
	resizeOverlayGL(width(), height());
    }
}

const QGLContext* QGLWidget::overlayContext() const
{
    return d->olcx;
}


void QGLWidget::makeOverlayCurrent()
{
    if(d->olcx) {
	d->olcx->makeCurrent();
	if(!d->olcx->initialized()) {
	    initializeOverlayGL();
	    d->olcx->setInitialized(TRUE);
	}
    }
}


void QGLWidget::updateOverlayGL()
{
    if(d->olcx) {
	makeOverlayCurrent();
	paintOverlayGL();
	if(d->olcx->format().doubleBuffer()) {
	    if(d->autoSwap)
		d->olcx->swapBuffers();
	}
	else {
	    glFlush();
	}
    }
}

void QGLWidget::setContext(QGLContext *context,
			    const QGLContext* shareContext,
			    bool deleteOldContext)
{
    if(context == 0) {
	qWarning("QGLWidget::setContext: Cannot set null context");
	return;
    }

    const QPaintDevice *me = this;
    if(macInternalDoubleBuffer()) {
	me = d->gl_pix;
    } else if(!context->deviceIsPixmap() && context->device() != me) {
	qWarning("QGLWidget::setContext: Context must refer to this widget");
	return;
    }

    if(d->glcx)
	d->glcx->doneCurrent();
    QGLContext* oldcx = d->glcx;
    d->glcx = context;
    d->slcx = shareContext;
    d->glcx_dblbuf = d->dblbuf;

    if(!d->glcx->isValid()) {
	const QGLContext *share = shareContext;
#if 0
	if(!share && !deleteOldContext)
	    share = oldcx;
#endif
	d->glcx->create(share);
    }
    if(deleteOldContext)
	delete oldcx;
}

bool QGLWidget::renderCxPm(QPixmap*)
{
    return FALSE;
}

const QGLColormap & QGLWidget::colormap() const
{
    return d->cmap;
}

void QGLWidget::setColormap(const QGLColormap &)
{
}

void QGLWidget::cleanupColormaps()
{
}

bool QGLWidget::macInternalDoubleBuffer(bool fix)
{
#if !defined(QMAC_OPENGL_DOUBLEBUFFER)
    bool need_fix = FALSE;
    if(isTopLevel()) {
	d->dblbuf = 0;
	if(clippedSerial() != d->clp_serial) {
	    if(children()) {
		QRect myrect(rect());
		register QObject *obj;
		for(QObjectListIterator it(*children()); (obj = it.current()); ++it) {
		    if(obj->isWidgetType()) {
			QWidget *w = (QWidget*)obj;
			if(w->isVisible() && !w->isTopLevel() && myrect.intersects(QRect(w->pos(), w->size()))) {
			    d->dblbuf = 1;
			    break;
			}
		    }
		}
	    }
	    d->clp_serial = clippedSerial();
        }
    } else if(clippedSerial() != d->clp_serial) {
	QRegion rgn = clippedRegion();
	d->clp_serial = clippedSerial();
	if(rgn.isNull() || rgn.isEmpty()) { //don't double buffer, we'll just make the area empty
	    d->dblbuf = 0;
	} else {
	    QRect rct(posInWindow(this), size());
	    if(!isTopLevel())
		rct &= topLevelWidget()->rect();
	    d->dblbuf = (rgn != QRegion(rct));
	}
    }
    if(d->glcx_dblbuf != d->dblbuf)
	need_fix = TRUE;
    else if(d->dblbuf && (!d->gl_pix || d->gl_pix->size() != size()))
	need_fix = TRUE;
    if(d->pending_fix || need_fix) {
	if(fix)
	    macInternalRecreateContext(new QGLContext(d->req_format,this));
	else
	    d->pending_fix = TRUE;
    }
#else
    Q_UNUSED(fix);
#endif
    return (bool)d->dblbuf;
}

void QGLWidget::macInternalRecreateContext(QGLContext *ctx, const QGLContext *share_ctx,
					   bool update)
{
    if(QApplication::closingDown())
	return;
    if (!ctx->device())
	ctx->setDevice(this);
    if(d->glcx && QMacBlockingFunction::blocking()) { //nah, let's do it "later"
	if(!d->dblbuf) {
	    d->glcx->fixBufferRect();
	} else if(d->gl_pix && d->gl_pix->size() != size()) {
	    aglSetDrawable((AGLContext)d->glcx->cx, NULL);
	    d->gl_pix->resize(size());
	    PixMapHandle mac_pm = GetGWorldPixMap((GWorldPtr)d->gl_pix->handle());
	    aglSetOffScreen((AGLContext)d->glcx->cx, d->gl_pix->width(), d->gl_pix->height(),
			    GetPixRowBytes(mac_pm), GetPixBaseAddr(mac_pm));
	}
	d->pending_fix = TRUE;
	return;
    }

    QGLContext* oldcx = d->glcx;
    d->glcx_dblbuf = d->dblbuf;
    d->pending_fix = FALSE;
    if(d->dblbuf) {
	setAttribute(WA_NoSystemBackground, true);
	if(d->gl_pix && d->glcx_dblbuf == d->dblbuf) { //currently double buffered, just resize
	    int w = width(), h = height();
	    if(d->gl_pix->size() != size()) {
		aglSetDrawable((AGLContext)d->glcx->cx, NULL);
		d->gl_pix->resize(w, h);
		PixMapHandle mac_pm = GetGWorldPixMap((GWorldPtr)d->gl_pix->handle());
		aglSetOffScreen((AGLContext)d->glcx->cx, d->gl_pix->width(), d->gl_pix->height(),
				GetPixRowBytes(mac_pm), GetPixBaseAddr(mac_pm));
	    }
	} else {
	    if(d->gl_pix && d->glcx && d->glcx->cx) {
		aglSetDrawable((AGLContext)d->glcx->cx, NULL);
		delete d->gl_pix;
	    }
	    d->gl_pix = new QPixmap(width(), height(), QPixmap::BestOptim);
	    if(oldcx)
		qgl_delete_d(this);
	    setContext(ctx, share_ctx ? share_ctx : d->slcx, FALSE);
	}
    } else {
	setEraseColor(black);
	if(oldcx)
	    qgl_delete_d(this);
	setContext(ctx, share_ctx ? share_ctx : d->slcx, FALSE);
	d->glcx->fixBufferRect();
    }
    if(update)
	repaint();
    if(oldcx && oldcx != d->glcx)
	delete oldcx;
}

void QGLWidget::setRegionDirty(bool b) //Internally we must put this off until "later"
{
    QWidget::setRegionDirty(b);
    if (!isVisibleTo(topLevelWidget())) {
	QTimer::singleShot(0, this, SLOT(macInternalFixBufferRect()));
    } else {
	macInternalFixBufferRect();
    }
}

void QGLWidget::macInternalFixBufferRect()
{
    d->glcx->fixBufferRect();
    update();
}
#endif
