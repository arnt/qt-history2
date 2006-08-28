/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgl.h"

#if defined(Q_WS_QWS)
#include <GLES/egl.h>
#include <GL/gl.h>
#endif

#include <private/qfont_p.h>
#include <private/qfontengine_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qstack.h>
#include <qdesktopwidget.h>
#include <qdebug.h>

/*****************************************************************************
  QOpenGL debug facilities
 *****************************************************************************/
//#define DEBUG_OPENGL_REGION_UPDATE

bool QGLFormat::hasOpenGL()
{
    return true;
}


bool QGLFormat::hasOpenGLOverlays()
{
    return true;
}


bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    Q_D(QGLContext);
    d->cx = 0;

    EGLConfig configs[5];
    EGLint matchingConfigs;
    const EGLint configAttribs[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     16,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE,           EGL_NONE
    };

    //Ask for an available display
    d->dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    //Display initialization(dont care about the OGLES version numbers)
    if (!eglInitialize(d->dpy, NULL, NULL))
        return false;

    if (!eglChooseConfig(d->dpy, configAttribs, &configs[0],
                         5, &matchingConfigs))
        return false;

    //If there isnt any configuration enough good
    if (matchingConfigs < 1)
        return false;

    memcpy(&d->config, configs, sizeof(EGLConfig));


    GLint res;
    eglGetConfigAttrib(d->dpy, d->config, EGL_LEVEL,&res);
    d->glFormat.setPlane(res);

    /*
    if(deviceIsPixmap())
        res = 0;
    else
    eglDescribePixelFormat(fmt, EGL_DOUBLEBUFFER, &res);
    d->glFormat.setDoubleBuffer(res);
    */

    eglGetConfigAttrib(d->dpy,d->config, EGL_DEPTH_SIZE, &res);
    d->glFormat.setDepth(res);
    if (d->glFormat.depth())
        d->glFormat.setDepthBufferSize(res);

    //eglGetConfigAttrib(d->dpy,d->config, EGL_RGBA, &res);
    //d->glFormat.setRgba(res);

    eglGetConfigAttrib(d->dpy,d->config, EGL_ALPHA_SIZE, &res);
    d->glFormat.setAlpha(res);
    if (d->glFormat.alpha())
        d->glFormat.setAlphaBufferSize(res);

    //eglGetConfigAttrib(d->dpy,d->config, EGL_ACCUM_RED_SIZE, &res);
    //d->glFormat.setAccum(res);
    //if (d->glFormat.accum())
    //    d->glFormat.setAccumBufferSize(res);

    eglGetConfigAttrib(d->dpy, d->config, EGL_STENCIL_SIZE, &res);
    d->glFormat.setStencil(res);
    if (d->glFormat.stencil())
        d->glFormat.setStencilBufferSize(res);

    //eglGetConfigAttrib(d->dpy, d->config, EGL_STEREO, &res);
    //d->glFormat.setStereo(res);

    eglGetConfigAttrib(d->dpy, d->config, EGL_SAMPLE_BUFFERS, &res);
    d->glFormat.setSampleBuffers(res);

    if (d->glFormat.sampleBuffers()) {
        eglGetConfigAttrib(d->dpy, d->config, EGL_SAMPLES, &res);
        d->glFormat.setSamples(res);
    }

    if(shareContext &&
       (!shareContext->isValid() || !shareContext->d_func()->cx)) {
        qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
        shareContext = 0;
    }

    EGLContext ctx = eglCreateContext(d->dpy, d->config,
                                      (shareContext ? shareContext->d_func()->cx : 0),
                                      configAttribs);
    if(!ctx) {
        GLenum err = eglGetError();
        if(err == EGL_BAD_MATCH || err == EGL_BAD_CONTEXT) {
            if(shareContext && shareContext->d_func()->cx) {
                qWarning("QGLContext::chooseContext(): Context sharing mismatch!");
                if(!(ctx = eglCreateContext(d->dpy, d->config, 0, configAttribs)))
                    return false;
                shareContext = 0;
            }
        }
        if(!ctx) {
            qWarning("QGLContext::chooseContext(): Unable to create QGLContext");
            return false;
        }
    }
    d->cx = ctx;
    if (shareContext && shareContext->d_func()->cx) {
        QGLContext *share = const_cast<QGLContext *>(shareContext);
        d->sharing = true;
        share->d_func()->sharing = true;
    }

    // vblank syncing
    GLint interval = d->reqFormat.swapInterval();
    if (interval != -1) {
        if (interval != 0)
            eglSwapInterval(d->dpy, interval);
    }

    if (deviceIsPixmap()) {
        d->surface = eglCreatePixmapSurface(d->dpy, d->config,
                                            (NativeWindowType)d->paintDevice,
                                            configAttribs);
    } else {
        d->surface = eglCreateWindowSurface(d->dpy, d->config,
                                            (NativeWindowType)d->paintDevice,
                                            configAttribs);
    }
        
    if (!d->surface)
        return false;
    
    return true;
}


void QGLContext::reset()
{
    Q_D(QGLContext);
    if (!d->valid)
        return;
    if (d->cx)
        eglDestroyContext(d->dpy, d->cx);
    d->cx = 0;
    d->crWin = false;
    d->sharing = false;
    d->valid = false;
    d->transpColor = QColor();
    d->initDone = false;
    qgl_share_reg()->removeShare(this);
}

void QGLContext::makeCurrent()
{
    Q_D(QGLContext);
    if(!d->valid) {
        qWarning("QGLContext::makeCurrent(): Cannot make invalid context current");
        return;
    }

    bool ok = eglMakeCurrent(d->dpy, d->surface, d->surface, d->cx);
    if (!ok)
        qWarning("QGLContext::makeCurrent(): Failed.");

    if (ok) {
        if (!qgl_context_storage.hasLocalData() && QThread::currentThread())
            qgl_context_storage.setLocalData(new QGLThreadContext);
        if (qgl_context_storage.hasLocalData())
            qgl_context_storage.localData()->context = this;
        currentCtx = this;
    }
}

void QGLContext::doneCurrent()
{
    Q_D(QGLContext);
    eglMakeCurrent(d->dpy, d->surface, d->surface, 0);
    if (qgl_context_storage.hasLocalData())
        qgl_context_storage.localData()->context = 0;
    currentCtx = 0;
}


void QGLContext::swapBuffers() const
{
    Q_D(const QGLContext);
    if(!d->valid)
        return;
    eglSwapBuffers(d->dpy, d->cx);
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor(0, 0, 0);                // Invalid color
}

uint QGLContext::colorIndex(const QColor&c) const
{
    //### color index doesn't work on egl
    return 0;
}

void QGLContext::generateFontDisplayLists(const QFont & fnt, int listBase)
{
}

void *QGLContext::getProcAddress(const QString &proc) const
{
    return (void*)eglGetProcAddress(reinterpret_cast<const char *>(proc.toLatin1().data()));
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
    Q_D(QGLWidget);
    if (!isValid())
        return;
    makeCurrent();
    if (!d->glcx->initialized())
        glInit();
    eglWaitNative(EGL_CORE_NATIVE_ENGINE);
    resizeGL(width(), height());

    //handle overlay
}

const QGLContext* QGLWidget::overlayContext() const
{
    return 0;
}

void QGLWidget::makeOverlayCurrent()
{
    //handle overlay
}

void QGLWidget::updateOverlayGL()
{
    //handle overlay
}

void QGLWidget::setContext(QGLContext *context, const QGLContext* shareContext, bool deleteOldContext)
{
    Q_D(QGLWidget);
    if(context == 0) {
        qWarning("QGLWidget::setContext: Cannot set null context");
        return;
    }

    if(d->glcx)
        d->glcx->doneCurrent();
    QGLContext* oldcx = d->glcx;
    d->glcx = context;
    if(!d->glcx->isValid())
        d->glcx->create(shareContext ? shareContext : oldcx);
    if(deleteOldContext)
        delete oldcx;
}

void QGLWidgetPrivate::init(QGLContext *context, const QGLWidget* shareWidget)
{
    Q_Q(QGLWidget);
    QGLExtensions::init();
    glcx = 0;
    autoSwap = true;

    if (!context->device())
        context->setDevice(q);
    q->setAttribute(Qt::WA_NoSystemBackground);
    q->setContext(context, shareWidget ? shareWidget->context() : 0);

    if(q->isValid() && glcx->format().hasOverlay()) {
        //no overlay
        qWarning("QtOpenGL ES doesn't currently support overlays");
    }
}

bool QGLWidgetPrivate::renderCxPm(QPixmap*)
{
    return false;
}

void QGLWidgetPrivate::cleanupColormaps()
{
}

const QGLColormap & QGLWidget::colormap() const
{
    return d_func()->cmap;
}

void QGLWidget::setColormap(const QGLColormap &)
{
}

void QGLExtensions::init()
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    QGLWidget dmy;
    dmy.makeCurrent();
    init_extensions();
}

