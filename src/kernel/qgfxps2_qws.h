/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QGfxRaster (unaccelerated graphics context) class
**
** Created : 940721
**
** Copyright (C) 1992-1999 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGFXSHITE_H
#define QGFXSHITE_H

#include <qgfx_qws.h>
#include <qpoint.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpixmap.h>
#include <qregion.h>
#include <qfont.h>
#include <qwsdisplay_qws.h>
#include <qwsevent_qws.h>

/* hacks to make it build */
#ifndef QGFXRASTER_H
#include <qmemorymanager_qws.h>
#endif

#ifndef QT_NO_QWS_CURSOR
class QPS2CursorData;

class QPS2Cursor : public QScreenCursor
{
public:
    QPS2Cursor();
    ~QPS2Cursor();

    virtual void init(SWCursorData *,bool=FALSE);

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual bool restoreUnder( const QRect &, QGfxRasterBase * = 0 );
    virtual void saveUnder();
    virtual void drawCursor();
    virtual void draw() {}
    virtual bool supportsAlphaCursor() { return TRUE; }

    static bool enabled() { return FALSE; }

private:
    QPS2CursorData *mydata;
};
#endif // QT_NO_QWS_CURSOR

class QGfxPS2 : public QGfxRasterBase, protected QPolygonScanner
{
    friend class QPS2Cursor;
public:

    QGfxPS2(int w,int h, int l);
    ~QGfxPS2();

    // Drawing operations
    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );

    // Fill operations - these use the current source (pixmap,
    // color, etc), and draws outline
    virtual void fillRect( int,int,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );

    // Special case of rect-with-pixmap-fill for speed/hardware acceleration
    virtual void blt( int,int,int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif

    virtual void drawAlpha(int,int,int,int,int,int,int,int) {}

    virtual void hsync(int) {}

    virtual void tiledBlt( int,int,int,int );

    virtual void setSource(const QImage *);
    virtual void setSource(const QPaintDevice *);

    virtual int bitDepth() { return 32; } /* huh? */

protected:
    int tex_width, tex_height, tex_psm;

    static void flushRegisters(bool flushtex=FALSE);

    /* used for blt functions */
    bool mapSourceToTexture(int x, int y, int w, int h);
    bool bltTexture(int x, int y, int clp, int w=-1, int h=-1);
    void buildSourceClut(QRgb * cols,int numcols);

    /* for polygonscanner */
    void processSpans( int n, QPoint* point, int* width );

};

class QPS2Screen : public QScreen
{
public:
    QPS2Screen( int display_id );
    virtual ~QPS2Screen();

    virtual int initCursor(void *, bool=FALSE);
    virtual bool initCard();
    virtual bool connect( const QString &displaySpec );

    virtual bool useOffscreen() { return false; }

    virtual void disconnect();
    virtual void shutdownCard();
    virtual void setMode(int,int,int) { }
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int) { }
    virtual QGfx *createGfx(unsigned char *data,int w,int h,int d, int linestep);

};

extern "C" QScreen * qt_get_screen_ps2( int display_id, const char *spec, char *,unsigned char *);

#endif





