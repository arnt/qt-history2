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

#include <private/qfontdata_p.h>
#include <private/qfontengine_p.h>
#include <private/qgl_p.h>
#include <qpaintengine_opengl.h>
#include <private/qt_mac_p.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>

/*****************************************************************************
  QOpenGL debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_UPDATE

/*****************************************************************************
  Externals
 *****************************************************************************/
extern WindowPtr qt_mac_window_for(HIViewRef); //qwidget_mac.cpp
extern QPoint posInWindow(const QWidget *); //qwidget_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle); //qregion_mac.cpp

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
    cx = NULL;
    vi = chooseMacVisual(GetMainDevice());
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
    if(shareContext && (format().rgba() != shareContext->format().rgba()))
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
            qWarning("QOpenGL: unable to create QGLContext");
            return false;
        }
    }
    cx = ctx;
    d->sharing = shareContext && shareContext->cx;
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
        if(glFormat.doubleBuffer())
            attribs[cnt++] = AGL_DOUBLEBUFFER;
//        attribs[cnt++] = AGL_ACCELERATED;
    }

    attribs[cnt] = AGL_NONE;
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
    if(!d->valid)
        return;
    if(cx)
        aglDestroyContext((AGLContext)cx);
    cx = 0;
    if(vi)
        aglDestroyPixelFormat((AGLPixelFormat)vi);
    vi = 0;
    d->crWin = false;
    d->sharing = false;
    d->valid = false;
    d->transpColor = QColor();
    d->initDone = false;
}

void QGLContext::makeCurrent()
{
    if(!d->valid) {
        qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
        return;
    }

    aglSetCurrentContext((AGLContext)cx);
    currentCtx = this;
}

void QGLContext::updatePaintDevice()
{
    if(d->paintDevice->devType() == QInternal::Widget) {
        //get control information
        QWidget *w = (QWidget *)d->paintDevice;
        HIViewRef hiview = (HIViewRef)w->winId();
        WindowPtr window = qt_mac_window_for(hiview);
#ifdef DEBUG_WINDOW_UPDATE
        qDebug("setting on %s %p/%p [%s]", w->metaObject()->className(), hiview, window, w->handle() ? "Inside" : "Outside");
#endif

        //update drawable
        aglSetDrawable((AGLContext)cx, GetWindowPort(window));

        if(!w->isTopLevel()) {
            //get drawable area
            RgnHandle widgetRgn = qt_mac_get_rgn();
            GetControlRegion(hiview, kControlStructureMetaPart, widgetRgn);
            QRegion clp = qt_mac_convert_mac_region(widgetRgn);
            QPoint p = posInWindow(w);
            clp.translate(p.x(), p.y());
            qt_mac_dispose_rgn(widgetRgn);
#ifdef DEBUG_WINDOW_UPDATE
            QVector<QRect> rs = clp.rects();
            for(int i = 0; i < rs.count(); i++)
                qDebug("%d %d %d %d", rs[i].x(), rs[i].y(), rs[i].width(), rs[i].height());
#endif

            //update the clip
            if(!aglIsEnabled((AGLContext)cx, AGL_BUFFER_RECT))
                aglEnable((AGLContext)cx, AGL_BUFFER_RECT);
            if(clp.isEmpty()) {
                GLint offs[4] = { 0, 0, 0, 0 };
                aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
                if(aglIsEnabled((AGLContext)cx, AGL_CLIP_REGION))
                    aglDisable((AGLContext)cx, AGL_CLIP_REGION);
            } else {
                QPoint mp(posInWindow(w));
                const GLint offs[4] = { mp.x(), w->topLevelWidget()->height() - (mp.y() + w->height()), w->width(), w->height() };
                aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
                aglSetInteger((AGLContext)cx, AGL_CLIP_REGION, (const GLint *)clp.handle(true));
                if(!aglIsEnabled((AGLContext)cx, AGL_CLIP_REGION))
                    aglEnable((AGLContext)cx, AGL_CLIP_REGION);
            }
        }
    } else if(d->paintDevice->devType() == QInternal::Pixmap) {
        QPixmap *pm = (QPixmap *)d->paintDevice;
        PixMapHandle mac_pm = GetGWorldPixMap((GWorldPtr)pm->handle());
        aglSetOffScreen((AGLContext)cx, pm->width(), pm->height(),
                        GetPixRowBytes(mac_pm), GetPixBaseAddr(mac_pm));
        GLint offs[4] = { 0, pm->height(), pm->width(), pm->height() };
        aglSetInteger((AGLContext)cx, AGL_BUFFER_RECT, offs);
        aglDisable((AGLContext)cx, AGL_CLIP_REGION);
    } else {
        qWarning("not sure how to render opengl on this device!!");
    }
    aglUpdateContext((AGLContext)cx);
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
    Str255 name;
    FMGetFontFamilyName((FMFontFamily)((UInt32)fnt.handle()), name);
    short fnum;
    GetFNum(name, &fnum);
    aglUseFont((AGLContext) cx, (int)fnum, fstyle, fnt.pointSize(), 0, 256, listBase);
}

/*****************************************************************************
  QGLWidget AGL-specific code
 *****************************************************************************/
#define d d_func()
#define q q_func()

static EventTypeSpec widget_opengl_events[] = {
    { kEventClassControl, kEventControlOwningWindowChanged },
    { kEventClassControl, kEventControlVisibilityChanged },
    { kEventClassControl, kEventControlBoundsChanged }
};
static EventHandlerUPP widget_opengl_handlerUPP = 0;
static void qt_clean_widget_opengl_handler()
{
    DisposeEventHandlerUPP(widget_opengl_handlerUPP);
}

/*!
    \internal
*/
OSStatus QGLWidget::globalEventProcessor(EventHandlerCallRef, EventRef event, void *data)
{
    bool handled_event = true;
    QGLWidget *widget = static_cast<QGLWidget*>(data);
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassControl:
        handled_event = false;
        if(ekind == kEventControlOwningWindowChanged || ekind == kEventControlVisibilityChanged 
           || ekind == kEventControlBoundsChanged) 
            widget->d->updatePaintDevice();
        break;
    default:
        handled_event = false;
        break;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}

void QGLWidget::init(QGLContext *context, const QGLWidget* shareWidget)
{
    if(!widget_opengl_handlerUPP) {
        widget_opengl_handlerUPP = NewEventHandlerUPP(QGLWidget::globalEventProcessor);
        qAddPostRoutine(qt_clean_widget_opengl_handler);
    }
    InstallControlEventHandler(reinterpret_cast<HIViewRef>(winId()), 
                               widget_opengl_handlerUPP, GetEventTypeCount(widget_opengl_events), 
                               widget_opengl_events, (void *)this, &d->event_handler);

    d->glcx = d->olcx = 0;
    d->autoSwap = true;

    setAttribute(Qt::WA_NoBackground);
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

void QGLWidget::setContext(QGLContext *context, const QGLContext* shareContext, bool deleteOldContext)
{
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

void QGLWidgetPrivate::updatePaintDevice()
{
    glcx->updatePaintDevice();
    if(olcx)
        olcx->updatePaintDevice();
}

#endif
