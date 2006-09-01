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
#include <GLES/gl.h>
#include <qdirectpainter_qws.h>

#include <qscreen_qws.h>
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

//#define USE_PIXMAP_SURFACE
#ifdef USE_PIXMAP_SURFACE
#include "qegl_qws.h"
#endif

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

#define QT_EGL_CHECK(x) \
    if (!(x)) { \
        EGLint err = eglGetError(); \
        printf("egl " #x " failure %x!\n", err); \
    } \

#define QT_EGL_ERR(txt) \
    do { \
        EGLint err = eglGetError(); \
        if (err != EGL_SUCCESS) \
            printf( txt " failure %x!\n", err); \
    } while (0)


bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    Q_D(QGLContext);
    d->cx = 0;

    EGLConfig configs[5];
    EGLint matchingConfigs;
    EGLint configAttribs[] = {
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_ALPHA_MASK_SIZE, 8,
        EGL_DEPTH_SIZE,     16,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE,           EGL_NONE
    };

    if (d->paintDevice->devType() == QInternal::Image) {
        QImage *img = static_cast<QImage*>(d->paintDevice);
        if (img->format() == QImage::Format_RGB16) {
            configAttribs[1] = 5;
            configAttribs[3] = 6;
            configAttribs[5] = 5;
            configAttribs[7] = 0;
        }
    } else if (d->paintDevice->devType() == QInternal::Widget) {
#ifdef USE_PIXMAP_SURFACE
        if (qt_screen->pixmapDepth() == 16) {
            configAttribs[1] = 5;
            configAttribs[3] = 6;
            configAttribs[5] = 5;
            configAttribs[7] = 0;
        }
        configAttribs[15] = EGL_PIXMAP_BIT;
#else
        configAttribs[1] = 0;
        configAttribs[3] = 0;
        configAttribs[5] = 0;
        configAttribs[7] = 0;
        configAttribs[9] = 0;
#endif
    }
    if (deviceIsPixmap() || d->paintDevice->devType() == QInternal::Image)
        configAttribs[15] = EGL_PIXMAP_BIT;

    //Ask for an available display
    d->dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    QT_EGL_CHECK(d->dpy);

    //Display initialization(dont care about the OGLES version numbers)
    if (!eglInitialize(d->dpy, NULL, NULL))
        return false;

    eglBindAPI(EGL_OPENGL_ES_API);

    if (!eglChooseConfig(d->dpy, configAttribs, &configs[0],
                         5, &matchingConfigs))
        return false;

    //If there isnt any configuration enough good
    if (matchingConfigs < 1)
        return false;

    //memcpy(&d->config, configs, sizeof(EGLConfig));
    d->config = configs[0];

    GLint res;
    eglGetConfigAttrib(d->dpy, d->config, EGL_LEVEL,&res);
    d->glFormat.setPlane(res);
    QT_EGL_ERR("eglGetConfigAttrib");
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

    EGLContext ctx = eglCreateContext(d->dpy, d->config, 0, 0);
                                      //(shareContext ? shareContext->d_func()->cx : 0),
                                      //configAttribs);
    if(!ctx) {
        GLenum err = eglGetError();
        qDebug("eglCreateContext err %x", err);
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

    if (deviceIsPixmap() || d->paintDevice->devType() == QInternal::Image) {
        d->surface = eglCreatePixmapSurface(d->dpy, d->config,
                                            (NativeWindowType)d->paintDevice,
                                            configAttribs);
    } else {
#ifndef USE_PIXMAP_SURFACE
        d->surface = eglCreateWindowSurface(d->dpy, d->config,
                                            (NativeWindowType)d->paintDevice, 0);
        if (!d->surface) {
             GLenum err = eglGetError();
             qDebug("eglCreateWindowSurface err %x", err);
        }
        qDebug() << "create Window surface" << d->surface;
#endif
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
    if (!ok) {
        EGLint err = eglGetError();
        qWarning("QGLContext::makeCurrent(): Failed %x.", err);
    }
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

    QT_EGL_ERR("QGLContext::doneCurrent");
    if (qgl_context_storage.hasLocalData())
        qgl_context_storage.localData()->context = 0;
    currentCtx = 0;
}


void QGLContext::swapBuffers() const
{
    Q_D(const QGLContext);
    if(!d->valid)
        return;
    eglSwapBuffers(d->dpy, d->surface);
    QT_EGL_ERR("QGLContext::swapBuffers");
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


class QGLDirectPainter : public QDirectPainter
{
public:
    QGLDirectPainter(QGLContextPrivate *cd, QGLWidgetPrivate *wd) :glxPriv(cd), glwPriv(wd), image(0), nativePix(0) {}
    ~QGLDirectPainter() {
    }
    void regionChanged(const QRegion &);
    void render();

    QRect geom;
    QGLContextPrivate *glxPriv;
    QGLWidgetPrivate *glwPriv;
    QImage *image;
    NativeWindowType nativePix;
};


void QGLDirectPainter::regionChanged(const QRegion&)
{
#ifdef USE_PIXMAP_SURFACE
    if (geometry() != geom) {
        geom = geometry();
        uchar *fbp = QDirectPainter::frameBuffer() + geom.top() * QDirectPainter::linestep()
                     + ((QDirectPainter::screenDepth()+7)/8) * geom.left();

        QImage *oldImage = image;
        NativeWindowType oldPix = nativePix;
        image = new QImage(fbp, geom.width(), geom.height(), QDirectPainter::screenDepth(),
                           QDirectPainter::linestep(), 0, 0, QImage::IgnoreEndian);
#if 0 // debug
        static int i = 0;
        i = (i+13) %255;
        for (int y = 0; y < image->height(); ++y)
            for (int x = 0; x < image->width(); ++x)
                image->setPixel(x, y, 0xff4000 + i);
#endif
        QT_EGL_ERR("before eglDestroySurface");
        if (glxPriv->surface != EGL_NO_SURFACE) {
            eglDestroySurface(glxPriv->dpy, glxPriv->surface);
            QT_EGL_ERR("eglDestroySurface");
        }
#if 1
        nativePix = QEGL::createNativePixmap(image);
        glxPriv->surface =    eglCreatePixmapSurface(glxPriv->dpy, glxPriv->config, nativePix, 0);////const EGLint *attrib list);
#elif 0
        glxPriv->surface = QtEGL::createEGLSurface(image, glxPriv->dpy, glxPriv->config);
#else
        glxPriv->surface = eglCreateWindowSurface(image, glxPriv->dpy, glxPriv->config);
#endif
        QT_EGL_ERR("createEGLSurface");
        glxPriv->valid =  glxPriv->surface != EGL_NO_SURFACE;
        glwPriv->resizeHandler(geom.size());
        delete oldImage;
        QEGL::destroyNativePixmap(oldPix);
    }
#endif
    if (0) {
    QRegion alloc = allocatedRegion();
    int max = 0;
    QRect allocR;

    for (int i=0; i < alloc.rects().count(); ++i) {
        QRect r = alloc.rects().at(i);
        int a = r.width()*r.height();
        if (a  > max) {
            max = a;
            allocR = r;
        }
    }
    allocR.translate(-geom.topLeft());
    glScissor(allocR.left(), geom.height() - allocR.bottom(), allocR.width(), allocR.height());

    glwPriv->render(allocR);
    }
}

void QGLWidgetPrivate::render(const QRegion &r)
{
    Q_Q(QGLWidget);
    QPaintEvent e(r);
    q->paintEvent(&e); //### slightly hacky...
}

void QGLWidgetPrivate::resizeHandler(const QSize &s)
{
    Q_Q(QGLWidget);

    q->makeCurrent();
    if (!glcx->initialized())
        q->glInit();
    eglWaitNative(EGL_CORE_NATIVE_ENGINE);
    QT_EGL_ERR("QGLWidgetPrivate::resizeHandler");

    q->resizeGL(s.width(), s.height());
}

bool QGLWidget::event(QEvent *e)
{
#if 0 // ??? we have to make sure update() works...
    if (e->type() == QEvent::Paint)
        return true; // We don't paint when the GL widget needs to be painted, but when the directpainter does
#endif
    return QWidget::event(e);
}

void QGLWidget::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
}


void QGLWidget::resizeEvent(QResizeEvent *)
{
    Q_D(QGLWidget);
//     if (!isValid())
//         return;

    if (!d->directPainter)
        d->directPainter = new QGLDirectPainter(d->glcx->d_func(), d);
    d->directPainter->setGeometry(geometry());
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
    directPainter = 0;
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

#if 0 //### to avoid confusing experimental EGL: don't create two GL widgets
    QGLWidget dmy;
    dmy.makeCurrent();
    init_extensions();
#endif
}

