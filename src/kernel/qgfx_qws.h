/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QGfx (graphics context) class
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QGFX_H
#define QGFX_H

#ifndef QT_H
#include "qwidget.h"
#include "qnamespace.h"
#include "qimage.h"
#include "qfontmanager_qws.h"
#endif // QT_H


const int SourceSolid=0;
const int SourcePixmap=1;

#ifndef QT_NO_QWS_CURSOR

extern bool qt_sw_cursor;

class QGfxRasterBase;

#define SW_CURSOR_DATA_SIZE	4096  // 64x64 8-bit cursor

class SWCursorData;

class QScreenCursor
{
public:
    QScreenCursor( );
    virtual ~QScreenCursor();

    virtual void init(SWCursorData *da, bool init = FALSE);

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
    virtual void show() {}
    virtual void hide() {}

    bool restoreUnder( const QRect &r, QGfxRasterBase *g = 0 );
    void saveUnder();
    void drawCursor();
    void draw();
    virtual bool supportsAlphaCursor();

    static bool enabled() { return qt_sw_cursor; }

protected:
    QImage cursor;
    int hotx;
    int hoty;

    QGfxRasterBase *gfx;
    uchar *fb_start;
    uchar *fb_end;
    bool save_under;
    SWCursorData *data;

    int myoffset;

};

extern QScreenCursor * qt_screencursor;

#endif // QT_NO_QWS_CURSOR

struct fb_cmap;

class QScreen {

public:

    QScreen();
    virtual ~QScreen();
    virtual bool initCard();
    virtual bool connect();
    virtual void disconnect();
    virtual int initCursor(void *,bool=false);
    virtual void shutdownCard();
    virtual void setMode(int,int,int);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual QGfx * screenGfx();
    virtual void save();
    virtual void restore();

    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);

    int width() const { return w; }
    int height() const { return h; }
    int depth() const { return d; }
    int linestep() const { return lstep; }
    uchar * base() const { return data; }
    virtual bool onCard(unsigned char *) const;
    virtual bool onCard(unsigned char *, ulong& out_offset) const;

    virtual int screenSize();
    virtual int totalSize();

    virtual int alloc(unsigned int,unsigned int,unsigned int);

    QRgb * clut() { return screenclut; }
    int numCols() { return screencols; }

protected:

    QRgb screenclut[256];
    int screencols;

    bool initted;

    uchar * data;
    int w;
    int lstep;
    int h;
    int d;
    // Used by QWS server for mode restoring
    int startupw;
    int startuph;
    int startupd;
    fb_cmap *startcmap;
    int fd;

    int hotx;
    int hoty;
    QImage cursor;

    int size;	       // Screen size
    int mapsize;       // Total mapped memory
};

extern QScreen * qt_screen;

class Q_EXPORT QGfx : public Qt {
public:
    // With loadable drivers, do probe here
    static QGfx *createGfx( int depth, unsigned char *buffer,
			    int w, int h, int linestep );

    virtual ~QGfx() {}

    virtual void setPen( const QPen & )=0;
    virtual void setFont( const QFont & )=0;
    virtual void setBrush( const QBrush & )=0;
    virtual void setBrushPixmap( const QPixmap * )=0;
    virtual void setClipRect( int,int,int,int )=0;
    virtual void setClipRegion( const QRegion & )=0;
    virtual void setClipping (bool)=0;
    // These will be called from qwidget_qws or qwidget_mac
    // to update the drawing area when a widget is moved
    virtual void setOffset( int,int )=0;
    virtual void setWidgetRect( int,int,int,int )=0;
    virtual void setWidgetRegion( const QRegion & )=0;
    virtual void setGlobalRegionIndex( int idx ) = 0;

    virtual void setDashedLines(bool d) = 0;
    virtual void setDashes(char *, int) = 0;

    virtual void setOpaqueBackground(bool b)=0;
    virtual void setBackgroundColor(QColor c)=0;

    // Drawing operations
    virtual void drawPoint( int,int )=0;
    virtual void drawPoints( const QPointArray &,int,int )=0;
    virtual void moveTo( int,int )=0;
    virtual void lineTo( int,int )=0;
    virtual void drawLine( int,int,int,int )=0;
    virtual void drawPolyline( const QPointArray &,int,int )=0;

    // Fill operations - these use the current source (pixmap,
    // colour, etc), and draws outline
    virtual void drawRect( int,int,int,int )=0;
    virtual void drawPolygon( const QPointArray &,bool,int,int )=0;

    virtual void setLineStep(int)=0;

    // Special case of rect-with-pixmap-fill for speed/hardware acceleration
    virtual void blt( int,int,int,int )=0;
    virtual void scroll( int,int,int,int,int,int )=0;

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int )=0;
#endif
    virtual void tiledBlt( int,int,int,int )=0;

    enum SourceType { SourcePen, SourceImage };

    // Setting up source data - can be solid colour or pixmap data
    virtual void setSource(const QPaintDevice *)=0;
    virtual void setSource(const QImage *)=0;
    virtual void setSource(unsigned char *,int,int)=0;
    // This one is pen
    virtual void setSourcePen()=0;
    virtual void setSourceOffset(int,int)=0;
    virtual void setMasking(bool on,int colour=0)=0;

    virtual void drawAlpha(int,int,int,int,int,int,int,int) {}

    virtual void hsync(int) {}

    // These apply only to blt's. For alpha values for general
    // drawing operations we should probably have a separate QGfx
    // class. It's not a high priority though.

    // Enum values: Ignore alpha information, alpha information encoded in
    // 32-bit rgba along with colours, alpha information in 8bpp
    // format in alphabits

    enum AlphaType { IgnoreAlpha, InlineAlpha, SeparateAlpha,
                     LittleEndianMask, BigEndianMask, SolidAlpha };

    // Can be no alpha, inline (32bit data), separate (for images),
    // LittleEndianMask/BigEndianMask 1bpp masks, constant alpha
    // value
    virtual void setAlphaType(AlphaType)=0;
    // Pointer to data, linestep
    virtual void setAlphaSource(unsigned char *,int)=0;
    virtual void setAlphaSource(int,int=-1,int=-1,int=-1)=0;

    virtual void drawText( int,int,const QString & )=0;

    virtual void setClut(QRgb *,int)=0;

    // Save and restore pen and brush state - necessary when setting
    // up a bitBlt for example
    virtual void save()=0;
    virtual void restore()=0;

    virtual void paintCursor(const QImage& image, int hotx, int hoty, QPoint cursorPos)=0;

protected:
    bool is_screen_gfx;
    
};

// This lives in loadable modules

#ifndef QT_LOADABLE_MODULES
extern "C" QScreen * qt_get_screen(char *,unsigned char *);
#endif

// This is in main lib, loads the right module, calls qt_get_screen
// In non-loadable cases just aliases to qt_get_screen

QScreen * qt_probe_bus();

#endif




