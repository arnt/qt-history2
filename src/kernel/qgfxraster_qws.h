/****************************************************************************
** $Id: $
**
** Definition of QGfxRaster (unaccelerated graphics context) class
**
** Created : 940721
**
** Copyright (C) 1992-1999 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGFXRASTER_H
#define QGFXRASTER_H

#ifndef QT_H
#include "qgfx_qws.h"
#include "qpen.h"
#include "qbrush.h"
#include "qimage.h"
#include "qfontmanager_qws.h"
#include "qmemorymanager_qws.h"
#include "qwsdisplay_qws.h"
#include "qpointarray.h"
#include "qpolygonscanner.h"
#include "qapplication.h"
#include "qregion.h"
#endif // QT_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

class QGfxRasterBase : public QGfx {

public:

    QGfxRasterBase(unsigned char *,int w,int h);
    ~QGfxRasterBase();

    virtual void setPen( const QPen & );
    virtual void setFont( const QFont & );
    virtual void setBrushPixmap( const QPixmap * p ) { cbrushpixmap=p; }
    virtual void setBrushOffset( int x, int y );
    virtual void setBrush( const QBrush & );

    virtual void setClipRect( int,int,int,int );
    virtual void setClipRegion( const QRegion & );
    virtual void setClipping(bool);

    // These will be called from qwidget_qws or qwidget_mac
    // to update the drawing area when a widget is moved
    virtual void setOffset( int,int );
    virtual void setWidgetRect( int,int,int,int );
    virtual void setWidgetRegion( const QRegion & );
    virtual void setGlobalRegionIndex( int idx );

    virtual void setDashedLines(bool d);
    virtual void setDashes(char *, int);

    virtual void moveTo( int,int );
    virtual void lineTo( int,int );

    virtual QPoint pos() const;
    
    virtual void setOpaqueBackground(bool b) { opaque=b; }
    virtual void setBackgroundColor(QColor c) { backcolor=c; }

    virtual void setAlphaType(AlphaType);
    virtual void setAlphaSource(unsigned char *,int);
    virtual void setAlphaSource(int,int=-1,int=-1,int=-1);
    virtual void drawText(int,int,const QString &);

    virtual void sync();

    virtual void setLineStep(int i) { lstep=i; }
    int linestep() const { return lstep; }

    int pixelWidth() const { return width; }
    int pixelHeight() const { return height; }
    virtual int bitDepth() = 0;

    virtual void setScreen(QScreen * t,QScreenCursor * c,bool swc,int * ot,
			   int * lo) {
#ifndef QT_NO_QWS_REPEATER
	gfx_screen=t;
#ifndef QT_NO_QWS_CURSOR
	gfx_screencursor=c;
	gfx_swcursor=swc;
#endif
	gfx_lastop=lo;
	gfx_optype=ot;
	setClut(gfx_screen->clut(),gfx_screen->numCols());
#endif
    }

    void save();
    void restore();

    virtual void setRop(RasterOp r) { myrop=r; }

    void setClut(QRgb * cols,int numcols) { clut=cols; clutcols=numcols;  }

protected:

    void* beginTransaction( const QRect& );
    void endTransaction(void*);

    void beginDraw()
    {
	QWSDisplay::grab();
	if ( globalRegionRevision &&
		*globalRegionRevision != currentRegionRevision ) {
	    fixClip();
	}
    }
    void endDraw()
    {
	QWSDisplay::ungrab();
    }
    void fixClip();
    void update_clip();

    bool inClip(int x, int y, QRect* cr=0, bool know_to_be_outside=FALSE);

    virtual void setSourceWidgetOffset( int x, int y );

    virtual void setSourcePen();
    unsigned char *scanLine(int i) { return buffer+(i*lstep); }
    unsigned char *srcScanLine(int i) { return srcbits + (i*srclinestep); }

    // Convert to/from different bit depths
    unsigned int get_value_32(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_24(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_16(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_15(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_8(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_4(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_1(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);

protected:
    QScreen * gfx_screen;
#ifndef QT_NO_QWS_CURSOR
    QScreenCursor * gfx_screencursor;
#endif
    bool gfx_swcursor;
    volatile int * gfx_lastop;
    volatile int * gfx_optype;

    SourceType srctype;
    PixelType srcpixeltype;
    unsigned char * srcbits;
    unsigned char * const buffer;

    PixelType pixeltype;
    int width;
    int height;
    int xoffs;
    int yoffs;
    unsigned int lstep;

    bool opaque;
    QColor backcolor;

    QPen cpen;
    QBrush cbrush;
    QPoint brushoffs;
    bool patternedbrush;
    const QPixmap * cbrushpixmap;
    bool dashedLines;
    char *dashes;
    int numDashes;

    QPen savepen;
    QBrush savebrush;

    bool regionClip;
    QRegion widgetrgn;
    QRegion cliprgn;
    QRect clipbounds;

    int penx;
    int peny;

    int srcwidth;
    int srcheight;
    int srcdepth;
    int srclinestep;
    int srccol;
    QPoint srcwidgetoffs;	    // Needed when source is widget
    bool src_little_endian;
    bool src_normal_palette;
    unsigned int srcclut[256];	    // Source color table - r,g,b values
    unsigned int transclut[256];    // Source clut transformed to destination
                                    // values - speed optimisation

    QRgb * clut;      		    // Destination color table - r,g,b values
    int clutcols;		    // Colours in clut

    int monobitcount;
    unsigned char monobitval;

    AlphaType alphatype;
    unsigned char * alphabits;
    unsigned int * alphabuf;
    int alphalinestep;
    bool ismasking;
    int amonobitcount;
    unsigned char amonobitval;
    bool amonolittletest;
    int calpha;       		 // Constant alpha value
    int calpha2,calpha3,calpha4; // Used for groovy accelerated effect
    unsigned char * maskp;
    QMemoryManager::FontID myfont;

    int clipcursor;
    QRect* cliprect;
    int ncliprect;

    int globalRegionIndex;
    const int *globalRegionRevision;
    int currentRegionRevision;

    RasterOp myrop;

    unsigned long int pixel; // == cpen.pixel() or cbrush.pixel()

    QPolygonScanner::Edge stitchedges;

    friend class QScreenCursor;
};

template <const int depth, const int type>
class QGfxRaster : public QGfxRasterBase, protected QPolygonScanner {

public:

    QGfxRaster(unsigned char *,int w,int h);
    ~QGfxRaster();

    void useBrush();
    void usePen();

    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void fillRect( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void blt( int,int,int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS) || !defined(QT_NO_PIXMAP_TRANSFORMATION)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif
    virtual void tiledBlt( int,int,int,int );

    virtual int bitDepth() { return depth; }

    virtual void setSource(const QImage *);
    virtual void setSource(const QPaintDevice *);
    virtual void setSource(unsigned char *,int,int,int,int,QRgb *,int);

protected:

    virtual void drawThickLine( int,int,int,int );
    virtual void drawThickPolyline( const QPointArray &,int,int );

    void buildSourceClut(QRgb *,int);
    void processSpans( int n, QPoint* point, int* width );

    // Optimised horizontal line drawing
    void hline(int,int,int );
    void hlineUnclipped(int,int,unsigned char* );
    void hImageLineUnclipped(int,int,unsigned char *,unsigned char *,bool);
    void hAlphaLineUnclipped(int,int,unsigned char *,unsigned char *,
			     unsigned char *);
    void drawPointUnclipped( int, unsigned char* );

    void calcPacking(void *,int,int,int&,int&,int&);
};

#endif





