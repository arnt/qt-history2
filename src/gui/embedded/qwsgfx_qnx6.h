/****************************************************************************
**
** Definition of Qt/Embedded Qnx keyboard drivers.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSGFX_QNX6_H
#define QWSGFX_QNX6_H

#ifndef QT_H
#include <display.h>
#include <disputil.h>
#include <qgfxraster_qws.h>
#include <qgfx_qws.h>
#include <qpolygonscanner.h>
#include <qpen.h>
#include <qstring.h>
#endif // QT_H

// QnxFb Gfx class
template <const int depth, const int type>
class QQnxFbGfx : public QGfxRaster<depth, type> {
    public:
        QQnxFbGfx();
        ~QQnxFbGfx();

        int bitDepth(){ return DISP_BITS_PER_PIXEL (ctx.dsurf->pixel_format);};

        void sync ();
//        void fillRect (int,int,int,int);
//        void blt (int,int,int,int,int,int);
//        void hlineUnclipped (int, int, int);

    private:
        disp_draw_context_t ctx;
};

// Screen class
class QQnxScreen : public QScreen {
    public:
        QQnxScreen(int display_id):QScreen(display_id){};
        ~QQnxScreen(){};

        bool connect(const QString & spec);
        void disconnect();

        bool initDevice();
        void shutdownDevice();
        void setMode(int, int, int);

        QGfx* createGfx (unsigned char*, int, int, int, int);

        int initCursor(void *, bool);
};

#ifndef QT_NO_QWS_CURSOR
class QQnxCursor : public QScreenCursor
{
public:
    QQnxCursor(){};
    ~QQnxCursor(){};

    virtual void init(SWCursorData *,bool=false);

    virtual void set(const QImage &image, int hotx, int hoty);
    virtual void move(int x, int y);
    virtual void show();
    virtual void hide();

    virtual bool restoreUnder(const QRect &, QGfxRasterBase * = 0)
                { return false; }
    virtual void saveUnder() {}
    virtual void drawCursor() {}
    virtual void draw() {}
    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

private:
    int hotx;
    int hoty;
    QBitArray cursor,mask;
    QColor colour0,colour1;
};

#endif // QT_NO_QWS_CURSOR

#endif // QWSGFX_QNX6_H
