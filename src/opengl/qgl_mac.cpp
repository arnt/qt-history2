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
#include <private/qgl_p.h>
#include <qpaintengine_opengl.h>
#include <qt_mac.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>

#include "qgl_mac_p.h"
#ifdef QT_DLOPEN_OPENGL
#include "qlibrary.h"

extern "C" {
    _glCallLists qt_glCallLists;
    _glClearColor qt_glClearColor;
    _glClearIndex qt_glClearIndex;
    _glColor3ub qt_glColor3ub;
    _glDeleteLists qt_glDeleteLists;
    _glDrawBuffer qt_glDrawBuffer;
    _glFlush qt_glFlush;
    _glIndexi qt_glIndexi;
    _glListBase qt_glListBase;
    _glLoadIdentity qt_glLoadIdentity;
    _glMatrixMode qt_glMatrixMode;
    _glOrtho qt_glOrtho;
    _glPopAttrib qt_glPopAttrib;
    _glPopMatrix qt_glPopMatrix;
    _glPushAttrib qt_glPushAttrib;
    _glPushMatrix qt_glPushMatrix;
    _glRasterPos2i qt_glRasterPos2i;
    _glRasterPos3d qt_glRasterPos3d;
    _glReadPixels qt_glReadPixels;
    _glViewport qt_glViewport;

    _aglChoosePixelFormat qt_aglChoosePixelFormat;
    _aglCreateContext qt_aglCreateContext;
    _aglDescribePixelFormat qt_aglDescribePixelFormat;
    _aglDestroyContext qt_aglDestroyContext;
    _aglDestroyPixelFormat qt_aglDestroyPixelFormat;
    _aglDisable qt_aglDisable;
    _aglEnable qt_aglEnable;
    _aglGetCurrentContext qt_aglGetCurrentContext;
    _aglGetError qt_aglGetError;
    _aglIsEnabled qt_aglIsEnabled;
    _aglNextPixelFormat qt_aglNextPixelFormat;
    _aglSetCurrentContext qt_aglSetCurrentContext;
    _aglSetDrawable qt_aglSetDrawable;
    _aglSetInteger qt_aglSetInteger;
    _aglSetOffScreen qt_aglSetOffScreen;
    _aglSwapBuffers qt_aglSwapBuffers;
    _aglUpdateContext qt_aglUpdateContext;
    _aglUseFont qt_aglUseFont;
}; // extern "C"

bool qt_resolve_gl_symbols(bool fatal)
{
    static bool gl_syms_resolved = false;
    if (gl_syms_resolved)
	return true;

    QLibrary gl("/System/Library/Frameworks/OpenGL.framework/OpenGL");
    QLibrary agl("/System/Library/Frameworks/AGL.framework/AGL");
    gl.setAutoUnload(false);
    agl.setAutoUnload(false);

    qt_glCallLists = (_glCallLists) gl.resolve("glCallLists");

    if (!qt_glCallLists) { // if this fails the rest will surely fail
	if (fatal)
	    qFatal("Unable to resolve GL/AGL symbols - please check your OpenGL installation.");
	return false;
    }

    qt_glClearColor = (_glClearColor) gl.resolve("glClearColor");
    qt_glClearIndex = (_glClearIndex) gl.resolve("glClearIndex");
    qt_glColor3ub = (_glColor3ub) gl.resolve("glColor3ub");
    qt_glDeleteLists = (_glDeleteLists) gl.resolve("glDeleteLists");
    qt_glDrawBuffer = (_glDrawBuffer) gl.resolve("glDrawBuffer");
    qt_glFlush = (_glFlush) gl.resolve("glFlush");
    qt_glIndexi = (_glIndexi) gl.resolve("glIndexi");
    qt_glListBase = (_glListBase) gl.resolve("glListBase");
    qt_glLoadIdentity = (_glLoadIdentity) gl.resolve("glLoadIdentity");
    qt_glMatrixMode = (_glMatrixMode) gl.resolve("glMatrixMode");
    qt_glOrtho = (_glOrtho) gl.resolve("glOrtho");
    qt_glPopAttrib = (_glPopAttrib) gl.resolve("glPopAttrib");
    qt_glPopMatrix = (_glPopMatrix) gl.resolve("glPopMatrix");
    qt_glPushAttrib = (_glPushAttrib) gl.resolve("glPushAttrib");
    qt_glPushMatrix = (_glPushMatrix) gl.resolve("glPushMatrix");
    qt_glRasterPos2i = (_glRasterPos2i) gl.resolve("glRasterPos2i");
    qt_glRasterPos3d = (_glRasterPos3d) gl.resolve("glRasterPos3d");
    qt_glReadPixels = (_glReadPixels) gl.resolve("glReadPixels");
    qt_glViewport = (_glViewport) gl.resolve("glViewport");

    qt_aglChoosePixelFormat = (_aglChoosePixelFormat) agl.resolve("aglChoosePixelFormat");
    qt_aglCreateContext = (_aglCreateContext) agl.resolve("aglCreateContext");
    qt_aglDescribePixelFormat = (_aglDescribePixelFormat) agl.resolve("aglDescribePixelFormat");
    qt_aglDestroyContext = (_aglDestroyContext) agl.resolve("aglDestroyContext");
    qt_aglDestroyPixelFormat = (_aglDestroyPixelFormat) agl.resolve("aglDestroyPixelFormat");
    qt_aglDisable = (_aglDisable) agl.resolve("aglDisable");
    qt_aglEnable = (_aglEnable) agl.resolve("aglEnable");
    qt_aglGetCurrentContext = (_aglGetCurrentContext) agl.resolve("aglGetCurrentContext");
    qt_aglGetError = (_aglGetError) agl.resolve("aglGetError");
    qt_aglIsEnabled = (_aglIsEnabled) agl.resolve("aglIsEnabled");
    qt_aglNextPixelFormat = (_aglNextPixelFormat) agl.resolve("aglNextPixelFormat");
    qt_aglSetCurrentContext = (_aglSetCurrentContext) agl.resolve("aglSetCurrentContext");
    qt_aglSetDrawable = (_aglSetDrawable) agl.resolve("aglSetDrawable");
    qt_aglSetInteger = (_aglSetInteger) agl.resolve("aglSetInteger");
    qt_aglSetOffScreen = (_aglSetOffScreen) agl.resolve("aglSetOffScreen") ;
    qt_aglSwapBuffers = (_aglSwapBuffers) agl.resolve("aglSwapBuffers");
    qt_aglUpdateContext = (_aglUpdateContext) agl.resolve("aglUpdateContext");
    qt_aglUseFont = (_aglUseFont) agl.resolve("aglUseFont");

    gl_syms_resolved = true;
    return true;
}
#endif // QT_DLOPEN_OPENGL




/*****************************************************************************
  QGLFormat UNIX/AGL-specific code
 *****************************************************************************/
bool QGLFormat::hasOpenGL()
{
    return true;
}


bool QGLFormat::hasOpenGLOverlays()
{
    return true;
}



/*****************************************************************************
  QGLContext AGL-specific code
 *****************************************************************************/
QPoint posInWindow(QWidget *); //qwidget_mac.cpp
bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    cx = NULL;
    GDHandle dev = GetMainDevice(); //doesn't handle multiple heads, fixme!
    vi = chooseMacVisual(dev);
    if(!vi)
	return false;

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
		    return false;
		shareContext = NULL;
	    }
	}
	if(!ctx) {
	    qDebug("QOpenGL: unable to create QGLContext");
	    return false;
	}
    }
    d->sharing = shareContext && shareContext->cx;

    if((cx = (void *)ctx)) {
	if(deviceIsPixmap()) {
	    QPixmap *pm = (QPixmap *)d->paintDevice;
	    PixMapHandle mac_pm = GetGWorldPixMap((GWorldPtr)pm->handle());
	    aglSetOffScreen(ctx, pm->width(), pm->height(),
			    GetPixRowBytes(mac_pm), GetPixBaseAddr(mac_pm));
	} else {
	    aglSetDrawable(ctx, GetWindowPort((WindowPtr)d->paintDevice->handle()));
	}
	return true;
    }
    return false;
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
    {
	attribs[cnt++] = AGL_RGBA;
	attribs[cnt++] = AGL_DEPTH_SIZE;
	attribs[cnt++] = deviceIsPixmap() ? ((QPixmap*)d->paintDevice)->depth() : 32;
    }
    if(glFormat.alpha()) {
	attribs[cnt++] = AGL_ALPHA_SIZE;
	attribs[cnt++] = 8;
    }
    if(glFormat.stencil()) {
	attribs[cnt++] = AGL_STENCIL_SIZE;
	attribs[cnt++] = 4;
    }
    {
	attribs[cnt++] = AGL_LEVEL;
	attribs[cnt++] = glFormat.plane();
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
    if(vi)
	aglDestroyPixelFormat((AGLPixelFormat)vi);
    vi = 0;
    d->oldR = QRegion();
    d->crWin = false;
    d->sharing = false;
    d->valid = false;
    d->transpColor = QColor();
    d->initDone = false;
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
    if(d->paintDevice->devType() == QInternal::Widget && !((QWidget*)d->paintDevice)->isTopLevel()) {
	if(!aglIsEnabled((AGLContext)cx, AGL_BUFFER_RECT))
	   aglEnable((AGLContext)cx, AGL_BUFFER_RECT);
	if(aglIsEnabled((AGLContext)cx, AGL_CLIP_REGION))
	    aglDisable((AGLContext)cx, AGL_CLIP_REGION);

	QWidget *w = (QWidget *)d->paintDevice;
	QRegion clp = qt_cast<QGLWidget*>(w)->d_func()->clippedRegion();
	if(clp != d->oldR) {
	    d->oldR = clp;
	    if(clp.isEmpty()) {
		GLint offs[4] = { 0, 0, 0, 0 };
		aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
	    } else {
		QPoint mp(posInWindow(w));
		int window_height = w->topLevelWidget()->height();
		window_height -= window_height - qt_cast<QGLWidget*>(w->topLevelWidget())->d_func()->clippedRegion(false).boundingRect().height(); //mask?
		GLint offs[4] = { mp.x(), window_height - (mp.y() + w->height()), w->width(), w->height() };
		aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
		aglSetInteger((AGLContext)cx, AGL_CLIP_REGION, (const GLint *)clp.handle(true));
		if(!aglIsEnabled((AGLContext)cx, AGL_CLIP_REGION))
		    aglEnable((AGLContext)cx, AGL_CLIP_REGION); //re-enable it..
	    }
	    aglUpdateContext((AGLContext)cx);
	    QMacSavedPortInfo::flush(w);
	}
    }
}

void QGLContext::doneCurrent()
{
    if(aglGetCurrentContext() != (AGLContext) cx)
	return;
    currentCtx = 0;
    aglSetCurrentContext(NULL);
}


void QGLContext::swapBuffers() const
{
    if(!d->valid)
	return;
    aglSwapBuffers((AGLContext)cx);
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor(0, 0, 0);		// Invalid color
}

static QColor cmap[256];
static bool cmap_init = false;
uint QGLContext::colorIndex(const QColor&c) const
{
    int ret = -1;
    if(!cmap_init) {
	cmap_init = true;
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

void QGLWidgetPrivate::setRegionDirty(bool b) //Internally we must put this off until "later"
{
    QWidgetPrivate::setRegionDirty(b);
    QTimer::singleShot(1, q, SLOT(macInternalFixBufferRect()));
}

#if 0
void QGLWidgetPrivate::macWidgetChangedWindow()
{
    if(d->glcx)
	aglSetDrawable((AGLContext)d->glcx->cx, GetWindowPort((WindowPtr)handle()));
    if(d->olcx)
	aglSetDrawable((AGLContext)d->olcx->cx, GetWindowPort((WindowPtr)handle()));
}
#endif

void QGLWidget::init(QGLContext *context, const QGLWidget* shareWidget)
{
    qt_resolve_gl_symbols();

    d->glcx = d->olcx = 0;
    d->autoSwap = true;
    d->clp_serial = 0;
    setEraseColor(black);
    setContext(context, shareWidget ? shareWidget->context() : 0);

    if(isValid() && d->glcx->format().hasOverlay()) {
	d->olcx = new QGLContext(QGLFormat::defaultOverlayFormat(), this);
        if(!d->olcx->create(shareWidget ? shareWidget->overlayContext() : 0)) {
	    delete d->olcx;
	    d->olcx = 0;
	    d->glcx->glFormat.setOverlay(false);
	}
    }
}


bool QGLWidget::event(QEvent *e)
{
    return QWidget::event(e);
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
	aglUpdateContext((AGLContext)d->olcx);
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
	    d->olcx->setInitialized(true);
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

    if(d->glcx)
	d->glcx->doneCurrent();
    QGLContext* oldcx = d->glcx;
    d->glcx = context;
    if(!d->glcx->isValid()) {
	const QGLContext *share = shareContext;
	d->glcx->create(share);
    }
    if(deleteOldContext)
	delete oldcx;
    macInternalFixBufferRect();
}

bool QGLWidget::renderCxPm(QPixmap*)
{
    return false;
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

void QGLWidget::macInternalFixBufferRect()
{
    d->glcx->fixBufferRect();
    if(d->olcx)
	d->olcx->fixBufferRect();
    update();
}

#endif
