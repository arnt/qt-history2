/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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

#include <CoreServices/CoreServices.h>
#include <private/qfont_p.h>
#include <private/qfontengine_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <private/qt_mac_p.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qstack.h>
#include <qdesktopwidget.h>
#include <qdebug.h>

/*****************************************************************************
  QOpenGL debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_UPDATE

/*****************************************************************************
  Externals
 *****************************************************************************/
extern WindowPtr qt_mac_window_for(HIViewRef); //qwidget_mac.cpp
extern QPoint qt_mac_posInWindow(const QWidget *); //qwidget_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle); //qregion_mac.cpp
extern void qt_mac_to_pascal_string(QString s, Str255 str, TextEncoding encoding=0, int len=-1);  //qglobal.cpp

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
bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    Q_D(QGLContext);
    d->cx = NULL;
    d->vi = chooseMacVisual(GetMainDevice());
    if(!d->vi)
        return false;

    AGLPixelFormat fmt = (AGLPixelFormat)d->vi;
    GLint res;
    aglDescribePixelFormat(fmt, AGL_LEVEL, &res);
    d->glFormat.setPlane(res);
    if(deviceIsPixmap())
        res = 0;
    else
        aglDescribePixelFormat(fmt, AGL_DOUBLEBUFFER, &res);
    d->glFormat.setDoubleBuffer(res);
    aglDescribePixelFormat(fmt, AGL_DEPTH_SIZE, &res);
    d->glFormat.setDepth(res);
    if (d->glFormat.depth())
	d->glFormat.setDepthBufferSize(res);
    aglDescribePixelFormat(fmt, AGL_RGBA, &res);
    d->glFormat.setRgba(res);
    aglDescribePixelFormat(fmt, AGL_ALPHA_SIZE, &res);
    d->glFormat.setAlpha(res);
    if (d->glFormat.alpha())
	d->glFormat.setAlphaBufferSize(res);
    aglDescribePixelFormat(fmt, AGL_ACCUM_RED_SIZE, &res);
    d->glFormat.setAccum(res);
    if (d->glFormat.accum())
	d->glFormat.setAccumBufferSize(res);
    aglDescribePixelFormat(fmt, AGL_STENCIL_SIZE, &res);
    d->glFormat.setStencil(res);
    if (d->glFormat.stencil())
	d->glFormat.setStencilBufferSize(res);
    aglDescribePixelFormat(fmt, AGL_STEREO, &res);
    d->glFormat.setStereo(res);
    aglDescribePixelFormat(fmt, AGL_SAMPLE_BUFFERS_ARB, &res);
    d->glFormat.setSampleBuffers(res);
    if (d->glFormat.sampleBuffers()) {
        aglDescribePixelFormat(fmt, AGL_SAMPLES_ARB, &res);
        d->glFormat.setSamples(res);
    }

    if(shareContext && (!shareContext->isValid() || !shareContext->d_func()->cx)) {
        qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
        shareContext = 0;
    }

    // sharing between rgba and color-index will give wrong colors
    if(shareContext && (format().rgba() != shareContext->format().rgba()))
        shareContext = 0;
    AGLContext ctx = aglCreateContext(fmt, (AGLContext) (shareContext ? shareContext->d_func()->cx : NULL));
    if(!ctx) {
        GLenum err = aglGetError();
        if(err == AGL_BAD_MATCH || err == AGL_BAD_CONTEXT) {
            if(shareContext && shareContext->d_func()->cx) {
                qWarning("QOpenGL: context sharing mismatch!");
                if(!(ctx = aglCreateContext(fmt, NULL)))
                    return false;
                shareContext = NULL;
            }
        }
        if(!ctx) {
            qWarning("QOpenGL: unable to create QGLContext");
            return false;
        }
    }
    d->cx = ctx;
    if (shareContext && shareContext->d_func()->cx) {
        d->sharing = true;
        const_cast<QGLContext *>(shareContext)->d_func()->sharing = true;
    }
    return true;
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
    Q_D(QGLContext);
    GLint attribs[40], cnt=0;
    if(deviceIsPixmap()) {
        attribs[cnt++] = AGL_PIXEL_SIZE;
        attribs[cnt++] = ((QPixmap*)d->paintDevice)->depth();
    }
    if(d->glFormat.stereo())
        attribs[cnt++] = AGL_STEREO;
    {
        attribs[cnt++] = AGL_RGBA;
        attribs[cnt++] = AGL_BUFFER_SIZE;
        attribs[cnt++] = deviceIsPixmap() ? ((QPixmap*)d->paintDevice)->depth() : 32;
    }
    if(d->glFormat.alpha()) {
        attribs[cnt++] = AGL_ALPHA_SIZE;
        attribs[cnt++] = d->glFormat.alphaBufferSize() == -1 ? 8 : d->glFormat.alphaBufferSize();
    }
    if(d->glFormat.stencil()) {
        attribs[cnt++] = AGL_STENCIL_SIZE;
        attribs[cnt++] = d->glFormat.stencilBufferSize() == -1 ? 8 : d->glFormat.stencilBufferSize();
    }
    if(d->glFormat.depth()) {
        attribs[cnt++] = AGL_DEPTH_SIZE;
        attribs[cnt++] = d->glFormat.depthBufferSize() == -1 ? 32 : d->glFormat.depthBufferSize();
    }
    if(d->glFormat.accum()) {
        attribs[cnt++] = AGL_ACCUM_RED_SIZE;
        attribs[cnt++] = d->glFormat.accumBufferSize() == -1 ? 16 : d->glFormat.accumBufferSize();
        attribs[cnt++] = AGL_ACCUM_BLUE_SIZE;
        attribs[cnt++] = d->glFormat.accumBufferSize() == -1 ? 16 : d->glFormat.accumBufferSize();
        attribs[cnt++] = AGL_ACCUM_GREEN_SIZE;
        attribs[cnt++] = d->glFormat.accumBufferSize() == -1 ? 16 : d->glFormat.accumBufferSize();
        attribs[cnt++] = AGL_ACCUM_ALPHA_SIZE;
        attribs[cnt++] = d->glFormat.accumBufferSize() == -1 ? 16 : d->glFormat.accumBufferSize();
    }
    {
        attribs[cnt++] = AGL_LEVEL;
        attribs[cnt++] = d->glFormat.plane();
    }

    if(d->glFormat.sampleBuffers()) {
        attribs[cnt++] = AGL_SAMPLE_BUFFERS_ARB;
        attribs[cnt++] = 1;
        attribs[cnt++] = AGL_SAMPLES_ARB;
        attribs[cnt++] = d->glFormat.samples() == -1 ? 4 : d->glFormat.samples();
    }

    if(deviceIsPixmap()) {
        attribs[cnt++] = AGL_OFFSCREEN;
    } else {
        if(d->glFormat.doubleBuffer())
            attribs[cnt++] = AGL_DOUBLEBUFFER;
//        attribs[cnt++] = AGL_ACCELERATED;
    }

    attribs[cnt] = AGL_NONE;

    Q_ASSERT(cnt < 40); // resize buffer above if too small

    AGLPixelFormat fmt;
    if(deviceIsPixmap() || !device)
        fmt = aglChoosePixelFormat(NULL, 0, attribs);
    else
        fmt = aglChoosePixelFormat(NULL, 0, attribs);
    if(!fmt) {
        GLenum err = aglGetError();
        qWarning("got an error tex: %d", (int)err);
    }
#if 0
    else {
        GLint res;
        int x = 0;
        for(AGLPixelFormat fmt2 = fmt; fmt2; fmt2 = aglNextPixelFormat(fmt2)) {
            aglDescribePixelFormat(fmt2, AGL_RENDERER_ID, &res);
            GLint res2;
            aglDescribePixelFormat(fmt2, AGL_ACCELERATED, &res2);
            qDebug("%d) 0x%08x 0x%08x %d", x++, (int)res, (int)AGL_RENDERER_GENERIC_ID, (int)res2);
        }
    }
#endif
    return fmt;
}

void QGLContext::reset()
{
    Q_D(QGLContext);
    if(!d->valid)
        return;
    if(d->cx)
        aglDestroyContext((AGLContext)d->cx);
    d->cx = 0;
    if(d->vi)
        aglDestroyPixelFormat((AGLPixelFormat)d->vi);
    d->vi = 0;
    d->crWin = false;
    d->sharing = false;
    d->valid = false;
    d->transpColor = QColor();
    d->initDone = false;
}

void QGLContext::makeCurrent()
{
    Q_D(QGLContext);
    if(!d->valid) {
        qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
        return;
    }

    aglSetCurrentContext((AGLContext)d->cx);
    currentCtx = this;
}

static QRegion qt_mac_get_widget_rgn(const QWidget *widget)
{
    if(!widget->isVisible() || widget->isMinimized())
        return QRegion();
    const QRect wrect = QRect(qt_mac_posInWindow(widget), widget->size());
    if(!wrect.isValid())
        return QRegion();

    RgnHandle macr = qt_mac_get_rgn();
    GetControlRegion((HIViewRef)widget->winId(), kControlStructureMetaPart, macr);
    OffsetRgn(macr, wrect.x(), wrect.y());
    QRegion ret = qt_mac_convert_mac_region(macr);

    QPoint clip_pos = wrect.topLeft();
    for(const QWidget *last_clip = 0, *clip = widget; clip; last_clip = clip, clip = clip->parentWidget()) {
        if(clip != widget) {
            GetControlRegion((HIViewRef)clip->winId(), kControlStructureMetaPart, macr);
            OffsetRgn(macr, clip_pos.x(), clip_pos.y());
            ret &= qt_mac_convert_mac_region(macr);
        }
        const QObjectList &children = clip->children();
        for(int i = children.size()-1; i >= 0; i--) {
            if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                if(child == last_clip)
                    break;
                if(child->isVisible() && !child->isMinimized() && !child->isTopLevel()) {
                    const QRect childRect = QRect(clip_pos+child->pos(), child->size());
                    if(childRect.isValid() && wrect.intersects(childRect)) {
                        GetControlRegion((HIViewRef)child->winId(), kControlStructureMetaPart, macr);
                        OffsetRgn(macr, childRect.x(), childRect.y());
                        ret -= qt_mac_convert_mac_region(macr);
                    }
                }
            }
        }
        clip_pos -= widget->pos();
    }
    qt_mac_dispose_rgn(macr);
    return ret;
}

void QGLContext::updatePaintDevice()
{
    Q_D(QGLContext);
    if(d->paintDevice->devType() == QInternal::Widget) {
        //get control information
        QWidget *w = (QWidget *)d->paintDevice;
        HIViewRef hiview = (HIViewRef)w->winId();
        WindowPtr window = qt_mac_window_for(hiview);
#ifdef DEBUG_WINDOW_UPDATE
        qDebug("setting on %s %p/%p [%s]", w->metaObject()->className(), hiview, window, w->handle() ? "Inside" : "Outside");
#endif

        //update drawable
        if(w->isWindow() && w->isFullScreen()) {
            aglSetFullScreen((AGLContext)d->cx, w->width(), w->height(), 0, QApplication::desktop()->screenNumber(w));
            w->hide();
        } else {
            aglSetDrawable((AGLContext)d->cx, GetWindowPort(window));
        }

        if(!w->isWindow()) {
            QRegion clp = qt_mac_get_widget_rgn(w); //get drawable area

#ifdef DEBUG_WINDOW_UPDATE
            QVector<QRect> rs = clp.rects();
            for(int i = 0; i < rs.count(); i++)
                qDebug("%d %d %d %d", rs[i].x(), rs[i].y(), rs[i].width(), rs[i].height());
#endif
            //update the clip
            if(!aglIsEnabled((AGLContext)d->cx, AGL_BUFFER_RECT))
                aglEnable((AGLContext)d->cx, AGL_BUFFER_RECT);
            if(clp.isEmpty()) {
                GLint offs[4] = { 0, 0, 0, 0 };
                aglSetInteger((AGLContext)d->cx, AGL_BUFFER_RECT, offs);
                if(aglIsEnabled((AGLContext)d->cx, AGL_CLIP_REGION))
                    aglDisable((AGLContext)d->cx, AGL_CLIP_REGION);
            } else {
                QPoint wpos = qt_mac_posInWindow(w);
                const GLint offs[4] = { wpos.x(), w->window()->height() - (wpos.y() + w->height()),
                                        w->width(), w->height() };
                aglSetInteger((AGLContext)d->cx, AGL_BUFFER_RECT, offs);
                aglSetInteger((AGLContext)d->cx, AGL_CLIP_REGION, (const GLint *)clp.handle(true));
                if(!aglIsEnabled((AGLContext)d->cx, AGL_CLIP_REGION))
                    aglEnable((AGLContext)d->cx, AGL_CLIP_REGION);
            }
        }
    } else if(d->paintDevice->devType() == QInternal::Pixmap) {
        QPixmap *pm = (QPixmap *)d->paintDevice;
        PixMapHandle mac_pm = GetGWorldPixMap((GWorldPtr)pm->macQDHandle());
        aglSetOffScreen((AGLContext)d->cx, pm->width(), pm->height(),
                        GetPixRowBytes(mac_pm), GetPixBaseAddr(mac_pm));
        GLint offs[4] = { 0, pm->height(), pm->width(), pm->height() };
        aglSetInteger((AGLContext)d->cx, AGL_BUFFER_RECT, offs);
        aglDisable((AGLContext)d->cx, AGL_CLIP_REGION);
    } else {
        qWarning("not sure how to render opengl on this device!!");
    }
    aglUpdateContext((AGLContext)d->cx);
}

void QGLContext::doneCurrent()
{
    if(aglGetCurrentContext() != (AGLContext) d_func()->cx)

        return;
    currentCtx = 0;
    aglSetCurrentContext(NULL);
}


void QGLContext::swapBuffers() const
{
    Q_D(const QGLContext);
    if(!d->valid)
        return;
    aglSwapBuffers((AGLContext)d->cx);
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor(0, 0, 0);                // Invalid color
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
            qWarning("whoa, that's no good..");
        } else {
            cmap[ret] = c;

            GLint vals[4];
            vals[0] = ret;
            vals[1] = c.red();
            vals[2] = c.green();
            vals[3] = c.blue();
            aglSetInteger((AGLContext)d_func()->cx, AGL_COLORMAP_ENTRY, vals);
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
    Str255 name;
    FMGetFontFamilyName((FMFontFamily)((UInt32)fnt.handle()), name);
    short fnum;
    GetFNum(name, &fnum);
    aglUseFont((AGLContext)d_func()->cx, (int)fnum, fstyle, QFontInfo(fnt).pointSize(), 0, 256, listBase);
}

static CFBundleRef qt_getOpenGLBundle()
{
    SInt16 frameworksVRefNum;
    SInt32 frameworksDirID;
    CFBundleRef bundle = 0;

    OSStatus err = FindFolder(kSystemDomain, kFrameworksFolderType, kDontCreateFolder,
		              &frameworksVRefNum, &frameworksDirID);
    if (err == noErr) {
	FSSpec spec;
	FSRef ref;

        Str255 framework_name;
        qt_mac_to_pascal_string(QLatin1String("OpenGL.framework"), framework_name);
        err = FSMakeFSSpec(frameworksVRefNum, frameworksDirID, framework_name, &spec);
	if (err == noErr) {
	    FSpMakeFSRef(&spec, &ref);
	    QCFType<CFURLRef> url = CFURLCreateFromFSRef(kCFAllocatorDefault, &ref);
	    if (url)
		bundle = CFBundleCreate(kCFAllocatorDefault, url);
	}
    }
    return bundle;
}

void *QGLContext::getProcAddress(const QString &proc) const
{
    return CFBundleGetFunctionPointerForName(QCFType<CFBundleRef>(qt_getOpenGLBundle()),
                                             QCFString(proc));
}

/*****************************************************************************
  QGLWidget AGL-specific code
 *****************************************************************************/

/****************************************************************************
  Hacks to glue AGL to an HIView
  ***************************************************************************/
static EventTypeSpec glwindow_change_events[] = {
    { kEventClassControl, kEventControlOwningWindowChanged },
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    { kEventClassControl, kEventControlVisibilityChanged },
#endif
    { kEventClassControl, kEventControlBoundsChanged }
};
static EventHandlerUPP glwindow_change_handlerUPP = 0;
static void qt_clean_glwindow_change_handler()
{
    DisposeEventHandlerUPP(glwindow_change_handlerUPP);
}
class QMacGLWindowChangeEvent : public QMacWindowChangeEvent
{
    EventHandlerRef event_handler;
    QGLWidget *context;
public:
    QMacGLWindowChangeEvent(QGLWidget *w) : context(w) {
        if(!glwindow_change_handlerUPP) {
            glwindow_change_handlerUPP = NewEventHandlerUPP(QMacGLWindowChangeEvent::globalEventProcessor);
            qAddPostRoutine(qt_clean_glwindow_change_handler);
        }
        InstallControlEventHandler(reinterpret_cast<HIViewRef>(w->winId()),
                                   glwindow_change_handlerUPP, GetEventTypeCount(glwindow_change_events),
                                   glwindow_change_events, (void *)this, &event_handler);
    }
    ~QMacGLWindowChangeEvent() {
        RemoveEventHandler(event_handler);
    }
protected:
    static OSStatus globalEventProcessor(EventHandlerCallRef, EventRef, void *);
    void windowChanged() { context->d_func()->updatePaintDevice(); }
};

OSStatus QMacGLWindowChangeEvent::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *)
{
#if 0 //not really needed right now, but just so I remember
    QMacGLWindowChangeEvent *change = static_cast<QMacGLWindowChangeEvent*>(data);
#endif
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassControl:
        if(ekind == kEventControlOwningWindowChanged
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
           || ekind == kEventControlVisibilityChanged
#endif
           || ekind == kEventControlBoundsChanged) {
            extern void qt_event_request_window_change(); //qapplication_mac.cpp
            qt_event_request_window_change();
        }
        break;
    default:
        break;
    }
    return CallNextEventHandler(er, event);
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
    if(!isValid())
        return;
    d->updatePaintDevice();
    makeCurrent();
    if(!d->glcx->initialized())
        glInit();
    resizeGL(width(), height());

    if(d->olcx) {
        makeOverlayCurrent();
        aglUpdateContext((AGLContext)d->olcx);
        resizeOverlayGL(width(), height());
    }
}

const QGLContext* QGLWidget::overlayContext() const
{
    return d_func()->olcx;
}

void QGLWidget::makeOverlayCurrent()
{
    Q_D(QGLWidget);
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
    Q_D(QGLWidget);
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
        d->glcx->create(shareContext);
    if(deleteOldContext)
        delete oldcx;
}

void QGLWidgetPrivate::init(QGLContext *context, const QGLWidget* shareWidget)
{
    Q_Q(QGLWidget);
    QGLExtensions::init();
    watcher = new QMacGLWindowChangeEvent(q);
    glcx = olcx = 0;
    autoSwap = true;

    q->setAttribute(Qt::WA_NoSystemBackground);
    q->setContext(context, shareWidget ? shareWidget->context() : 0);

    if(q->isValid() && glcx->format().hasOverlay()) {
        olcx = new QGLContext(QGLFormat::defaultOverlayFormat(), q);
        if(!olcx->create(shareWidget ? shareWidget->overlayContext() : 0)) {
            delete olcx;
            olcx = 0;
            glcx->d_func()->glFormat.setOverlay(false);
        }
    }

    updatePaintDevice();
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

void QGLWidgetPrivate::updatePaintDevice()
{
    Q_Q(QGLWidget);
    glcx->updatePaintDevice();
    if(olcx)
        olcx->updatePaintDevice();
    q->update();
}

void QGLExtensions::init()
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    GLint attribs[] = { AGL_RGBA, AGL_NONE };
    AGLPixelFormat fmt = aglChoosePixelFormat(NULL, 0, attribs);
    if (!fmt) {
        qDebug("QGLExtensions: couldn't find any RGB visuals.");
        return;
    }
    AGLContext ctx = aglCreateContext(fmt, NULL);
    if (!ctx) {
        qDebug("QGLExtensions: unable to create context.");
    } else {
        aglSetCurrentContext(ctx);
        init_extensions();
        aglSetCurrentContext(NULL);
        aglDestroyContext(ctx);
    }
    aglDestroyPixelFormat(fmt);
}

#endif
