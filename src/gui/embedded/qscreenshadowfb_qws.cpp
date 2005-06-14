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

#include "qgfxshadowfb_qws.h"

#ifndef QT_NO_QWS_SHADOWFB

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#if !defined(Q_OS_FREEBSD) && !defined (QT_NO_QWS_LINUXFB)
#include <linux/fb.h>
#endif

#define QT_SHADOWFB_TIMER_INTERVAL 20
// Update screen every 20 milliseconds, or 50 times a second

#ifndef QT_NO_QWS_CURSOR

QShadowScreenCursor::QShadowScreenCursor() : SHADOWFB_CURSOR_PARENT ()
{
}

void QShadowScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    QWSDisplay::grab(true);
    QRect r(data->x - hotx, data->y - hoty, image.width(), image.height());
    qt_screen->setDirty(data->bound | r);
    SHADOWFB_CURSOR_PARENT ::set(image, hotx, hoty);
    QWSDisplay::ungrab();
}

void QShadowScreenCursor::move(int x, int y)
{
    QWSDisplay::grab(true);
    QRect r(x - data->hotx, y - data->hoty, data->width, data->height);
    qt_screen->setDirty(r | data->bound);
    SHADOWFB_CURSOR_PARENT ::move(x, y);
    QWSDisplay::ungrab();
}
#endif

template <const int depth, const int type>
QGfxShadow<depth,type>::QGfxShadow(unsigned char *b,int w,int h)
    : SHADOWFB_RASTER_PARENT (b, w, h)
{
}

template <const int depth, const int type>
QGfxShadow<depth,type>::~QGfxShadow()
{
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPoint(int x, int y)
{
    QWSDisplay::grab(true);
    qt_screen->setDirty(QRect(x+xoffs, y+yoffs, 1, 1) & clipbounds);
    SHADOWFB_RASTER_PARENT ::drawPoint(x, y);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPoints(const QPolygon &pa,int x,int y)
{
    QWSDisplay::grab(true);
    QRect r = pa.boundingRect();
    r.translate(xoffs, yoffs);
    qt_screen->setDirty(r & clipbounds);
    SHADOWFB_RASTER_PARENT ::drawPoints(pa, x, y);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawLine(int x1,int y1,int x2,int y2)
{
    QWSDisplay::grab(true);
    QRect r;
    r.setCoords(x1+xoffs, y1+yoffs, x2+xoffs, y2+yoffs);
    qt_screen->setDirty(r & clipbounds);
    SHADOWFB_RASTER_PARENT ::drawLine(x1, y1, x2, y2);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::fillRect(int x,int y,int w,int h)
{
    QWSDisplay::grab(true);
    qt_screen->setDirty(QRect(x+xoffs, y+yoffs, w, h) & clipbounds);
    SHADOWFB_RASTER_PARENT ::fillRect(x, y, w, h);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPolyline(const QPolygon &pa,int x,int y)
{
    QWSDisplay::grab(true);
    QRect r = pa.boundingRect();
    r.translate(xoffs, yoffs);
    qt_screen->setDirty(r & clipbounds);
    SHADOWFB_RASTER_PARENT ::drawPolyline(pa, x, y);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPolygon(const QPolygon &pa,bool w,int x,int y)
{
    QWSDisplay::grab(true);
    QRect r = pa.boundingRect();
    r.translate(xoffs, yoffs);
    qt_screen->setDirty(r & clipbounds);
    SHADOWFB_RASTER_PARENT ::drawPolygon(pa, w, x, y);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::blt(int x,int y,int w,int h, int sx, int sy)
{
    QWSDisplay::grab(true);
    qt_screen->setDirty(QRect(x+xoffs, y+yoffs, w, h) & clipbounds);
    SHADOWFB_RASTER_PARENT ::blt(x, y, w, h, sx, sy);
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::scroll(int x,int y,int w,int h,int sx,int sy)
{
    QWSDisplay::grab(true);
    int dy = sy - y;
    int dx = sx - x;
    qt_screen->setDirty(QRect(qMin(x,sx) + xoffs, qMin(y,sy) + yoffs,
                           w+abs(dx), h+abs(dy)) & clipbounds);
    SHADOWFB_RASTER_PARENT ::scroll(x, y, w, h, sx, sy);
    QWSDisplay::ungrab();
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template <const int depth, const int type>
void QGfxShadow<depth,type>::stretchBlt(int x,int y,int w,int h,int sx,int sy)
{
    QWSDisplay::grab(true);
    qt_screen->setDirty(QRect(x + xoffs, y + yoffs, w, h) & clipbounds);
    SHADOWFB_RASTER_PARENT ::stretchBlt(x, y, w, h, sx, sy);
    QWSDisplay::ungrab();
}
#endif

template <const int depth, const int type>
void QGfxShadow<depth,type>::tiledBlt(int x,int y,int w,int h)
{
    QWSDisplay::grab(true);
    qt_screen->setDirty(QRect(x + xoffs, y + yoffs, w, h) & clipbounds);
    SHADOWFB_RASTER_PARENT ::tiledBlt(x, y, w, h);
    QWSDisplay::ungrab();
}

QShadowTimerHandler::QShadowTimerHandler(QShadowFbScreen * s)
{
    screen=s;
    start();
}

void QShadowTimerHandler::timerEvent(QTimerEvent *)
{
    screen->doUpdate();
}

void QShadowTimerHandler::start()
{
    timerId = startTimer(QT_SHADOWFB_TIMER_INTERVAL);
}
void QShadowTimerHandler::stop()
{
    if (timerId)
        killTimer(timerId);
    timerId = 0;
}



QShadowFbScreen::QShadowFbScreen(int display_id)
    : SHADOWFB_SCREEN_PARENT (display_id)
{
    timer=new QShadowTimerHandler(this);
}

QShadowFbScreen::~QShadowFbScreen()
{
    delete timer;
}

bool QShadowFbScreen::initDevice()
{
    return SHADOWFB_SCREEN_PARENT ::initDevice();
}

bool QShadowFbScreen::connect(const QString &displaySpec)
{
    bool ret=SHADOWFB_SCREEN_PARENT ::connect(displaySpec);
    if(!ret)
        return false;

    real_screen=data;

    to_update=QRect(0,0,w,h);

    return true;
}

void QShadowFbScreen::disconnect()
{
    data=real_screen;

    SHADOWFB_SCREEN_PARENT ::disconnect();
}

int QShadowFbScreen::initCursor(void* end_of_location, bool init)
{
    /*
      The end_of_location parameter is unusual: it's the address
      after the cursor data.
    */
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)end_of_location - 1;
    qt_screencursor=new QShadowScreenCursor();
    qt_screencursor->init(data, init);
    return sizeof(SWCursorData);
#else
    return 0;
#endif
}

void QShadowFbScreen::shutdownDevice()
{
    SHADOWFB_SCREEN_PARENT ::shutdownDevice();
}

void QShadowFbScreen::save()
{
    SHADOWFB_SCREEN_PARENT ::save();
}

void QShadowFbScreen::restore()
{
    SHADOWFB_SCREEN_PARENT ::restore();
}

QGfx * QShadowFbScreen::createGfx(unsigned char * bytes,int w,int h,int d,
                                  int linestep)
{
    if(bytes==base()) {
    QGfx* ret;
    if (false) {
        //Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
        ret = new QGfxShadow<1,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
        ret = new QGfxShadow<4,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
        ret = new QGfxShadow<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
        ret = new QGfxShadow<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
        ret = new QGfxShadow<24,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if(d==32) {
        ret = new QGfxShadow<32,0>(bytes,w,h);
#endif
    } else {
        qFatal("Can't drive depth %d",d);
        ret = 0; // silence gcc
    }
    ret->setLineStep(linestep);
    return ret;
    } else {
        return SHADOWFB_SCREEN_PARENT ::createGfx(bytes,w,h,d,linestep);
    }
}

void QShadowFbScreen::setMode(int nw,int nh,int nd)
{
    SHADOWFB_SCREEN_PARENT ::setMode(nw,nh,nd);
}

void QShadowFbScreen::setDirty(const QRect& r)
{
    to_update=to_update.unite(r);
}

void QShadowFbScreen::doUpdate()
{
    QVector<QRect> rectlist=to_update.rects();
    QRect screen(0,0,w,h);
#ifdef SHADOWFB_USE_QGFX
    // This is here to allow accelerated shadowfb copies
    QGfx * gfx=SHADOWFB_SCREEN_PARENT ::createGfx(real_screen,w,h,d,lstep);
    gfx->setSource(data,w,h,lstep,d,&screenclut,screencols);
#endif
    for(unsigned int loopc=0;loopc<rectlist.size();loopc++) {
        QRect r=rectlist[loopc];
        r=r.intersect(screen);
#ifdef SHADOWFB_USE_QGFX
        gfx->blt(r.left(),r.top(),r.width(),r.height(),r.left(),r.top());
#else
        for(int loopc2=r.top();loopc2<=r.bottom();loopc2++) {
            int offset=((r.left() * d)/8);
            int width=(((r.right()-r.left()) +1) * d)/8;
            offset/=sizeof(PackType);
            width=(width + (sizeof(PackType) * 2)) / sizeof(PackType);
            PackType * dest=((PackType *) (real_screen +
                            (lstep*loopc2))) + offset;
            PackType * src=((PackType *) (data+  (lstep*loopc2)))
                           + offset;
            for(int loopc3=0;loopc3<width;loopc3++) {
                *dest=*src;
                dest++;
                src++;
            }
        }
#endif
    }
#ifdef SHADOWFB_USE_QGFX
    delete gfx;
#endif
    to_update=QRegion();
}

int QShadowFbScreen::memoryNeeded(const QString &displaySpec)
{
    // This is fairly ugly but I'm not sure how else to handle it

    int myfd;

    // Check for explicitly specified device
    const int len = 8; // "/dev/fbx"
    int m = displaySpec.indexOf("/dev/fb");

    QByteArray dev = (m>=0) ? displaySpec.mid(m, len).toLatin1() : QByteArray("/dev/fb0");

    myfd=open(dev.constData(), O_RDWR);
    if (myfd<0) {
        qWarning("Can't open framebuffer device %s",dev.constData());
        return false;
    }

    fb_fix_screeninfo finfo;
    fb_var_screeninfo vinfo;

    /* Get fixed screen information */
    if (ioctl(myfd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("reading /dev/fb0");
        qWarning("Error reading fixed information");
        return false;
    }

    /* Get variable screen information */
    if (ioctl(myfd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("reading /dev/fb0");
        qWarning("Error reading variable information");
        return false;
    }

    return vinfo.yres*finfo.line_length;

    close(myfd);
}

int QShadowFbScreen::sharedRamSize(void * end)
{
#ifdef SHADOWFB_USE_QGFX
    shadow_screen=data+size;
    data=shadow_screen;
    return SHADOWFB_SCREEN_PARENT ::sharedRamSize((void *)shadow_screen);
#else
    shadow_screen=(uchar *)end;
    shadow_screen-=size;
    data=shadow_screen;
    int ret=SHADOWFB_SCREEN_PARENT ::sharedRamSize((void *)shadow_screen);
    return ret+size;
#endif
}

void QShadowFbScreen::haltUpdates()
{
    timer->stop();
}

void QShadowFbScreen::resumeUpdates()
{
    timer->start();
}

#endif
