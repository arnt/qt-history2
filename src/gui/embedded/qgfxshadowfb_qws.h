/****************************************************************************
**
** Implementation of shadow framebuffer driver.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXSHADOWFB_QWS_H
#define QGFXSHADOWFB_QWS_H

#ifndef QT_NO_QWS_SHADOWFB

#ifndef QT_H
#include "qgfxraster_qws.h"
#include "qgfxlinuxfb_qws.h"
#include "qobject.h"
#endif // QT_H

// Define these appropriately to use an accelerated driver
// as the basis for shadowfb

#define SHADOWFB_RASTER_PARENT QGfxRaster<depth,type>
#define SHADOWFB_CURSOR_PARENT QScreenCursor
#define SHADOWFB_SCREEN_PARENT QLinuxFbScreen

// Define this to use a QGfx for the shadow screen updates
// (useful if you have hardware acceleration)
// #define SHADOWFB_USE_QGFX

template <const int depth, const int type>
class QGfxShadow : public SHADOWFB_RASTER_PARENT
{
public:
    QGfxShadow(unsigned char *b,int w,int h);
    virtual ~QGfxShadow();

    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void fillRect( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void blt( int,int,int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif
    virtual void tiledBlt( int,int,int,int );
};

#ifndef QT_NO_QWS_CURSOR
class QShadowScreenCursor : public SHADOWFB_CURSOR_PARENT
{
public:
    QShadowScreenCursor();

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
};
#endif

class QShadowFbScreen;

class QShadowTimerHandler : public QObject
{

public:

    QShadowTimerHandler(QShadowFbScreen *);
    virtual void timerEvent(QTimerEvent *);

private:

    QShadowFbScreen * screen;

};

class QShadowFbScreen : public SHADOWFB_SCREEN_PARENT
{

public:

    QShadowFbScreen(int);
    virtual ~QShadowFbScreen();
    virtual bool initDevice();
    virtual bool connect( const QString & );
    virtual void disconnect();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int,int,int);
    virtual void setDirty( const QRect& );
    void doUpdate();
    virtual int memoryNeeded(const QString&);
    virtual int sharedRamSize(void *);

    virtual void haltUpdates();
    virtual void resumeUpdates();

private:

    uchar * real_screen;
    uchar * shadow_screen;
    QShadowTimerHandler * timer;
    QRegion to_update;

};

#endif

#endif
