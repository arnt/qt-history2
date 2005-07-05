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

#ifndef QGFXSHADOWFB_QWS_H
#define QGFXSHADOWFB_QWS_H

#ifndef QT_NO_QWS_SHADOWFB

#include "QtGui/qgfxraster_qws.h"
#include "QtGui/qgfxlinuxfb_qws.h"
#include "QtCore/qobject.h"

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

    virtual void drawPoint(int,int);
    virtual void drawPoints(const QPolygon &,int,int);
    virtual void drawLine(int,int,int,int);
    virtual void fillRect(int,int,int,int);
    virtual void drawPolyline(const QPolygon &,int,int);
    virtual void drawPolygon(const QPolygon &,bool,int,int);
    virtual void blt(int,int,int,int,int,int);
    virtual void scroll(int,int,int,int,int,int);
    virtual void stretchBlt(int,int,int,int,int,int);
    virtual void tiledBlt(int,int,int,int);
};

#ifndef QT_NO_QWS_CURSOR
class QShadowScreenCursor : public SHADOWFB_CURSOR_PARENT
{
public:
    QShadowScreenCursor();

    virtual void set(const QImage &image, int hotx, int hoty);
    virtual void move(int x, int y);
};
#endif

class QShadowFbScreen;

class QShadowTimerHandler : public QObject
{

public:

    explicit QShadowTimerHandler(QShadowFbScreen *);
    virtual void timerEvent(QTimerEvent *);

    void start();
    void stop();

private:
    int timerId;
    QShadowFbScreen * screen;

};

class QShadowFbScreen : public SHADOWFB_SCREEN_PARENT
{

public:

    explicit QShadowFbScreen(int);
    virtual ~QShadowFbScreen();
    virtual bool initDevice();
    virtual bool connect(const QString &);
    virtual void disconnect();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int,int,int);
    virtual void setDirty(const QRect&);
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

#endif // QGFXSHADOWFB_QWS_H
