/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QGfxRaster (unaccelerated graphics context) class
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

class QGfxRasterBase : public QGfx {

public:

    QGfxRasterBase(unsigned char *,int w,int h);
    ~QGfxRasterBase();

    virtual void setPen( const QPen & );
    virtual void setFont( const QFont & );
    virtual void setBrushPixmap( const QPixmap * p ) { cbrushpixmap=p; }
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

    virtual void setOpaqueBackground(bool b) { opaque=b; }
    virtual void setBackgroundColor(QColor c) { backcolor=c; }

    virtual void setSourceOffset(int,int);
    virtual void setMasking(bool on,int colour=0);

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

    void save();
    void restore();

    virtual void setRop(RasterOp r) { myrop=r; }

    virtual void paintCursor(const QImage& image, int hotx, int hoty, QPoint cursorPos);

    void setClut(QRgb * cols,int numcols) { clut=cols; clutcols=numcols;  }

protected:

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

    void useBrush();
    void usePen();
    virtual void setSourcePen();
    unsigned char *scanLine(int i) { return buffer+(i*lstep); }
    unsigned char *srcScanLine(int i) { return srcbits + (i*srclinestep); }

    // Convert to/from different bit depths
    unsigned int get_value_32(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_16(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_15(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_8(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);
    unsigned int get_value_1(int sdepth,unsigned char **srcdata,
			   bool reverse=FALSE);

protected:
    SourceType srctype;
    unsigned char * srcbits;
    unsigned char * const buffer;

    int width;
    int height;
    int xoffs;
    int yoffs;
    unsigned int lstep;

    bool opaque;
    QColor backcolor;

    QPen cpen;
    QBrush cbrush;
    bool patternedbrush;
    const QPixmap * cbrushpixmap;
    bool dashedLines;
    char *dashes;
    int numDashes;

    QPen savepen;
    QBrush savebrush;

    bool regionClip;
    QRegion widgetrgn;
    QRegion setrgn;
    QRegion cliprgn;
    QRect clipbounds;

    int penx;
    int peny;

    int srcwidth;
    int srcheight;
    int srcdepth;
    int srclinestep;
    int srccol;
    QPoint srcoffs;
    int srcwidgetx;		    // Needed when source is widget
    int srcwidgety;
    bool src_little_endian;
    bool src_normal_palette;
    unsigned int srcclut[256];	    // Source colour table - r,g,b values
    unsigned int transclut[256];    // Source clut transformed to destination
                                    // values - speed optimisation

    QRgb * clut;      		    // Destination colour table - r,g,b values
    int clutcols;		    // Colours in clut

    int monobitcount;
    unsigned char monobitval;

    AlphaType alphatype;
    unsigned char * alphabits;
    unsigned int * alphabuf;
    int alphalinestep;
    bool ismasking;
    unsigned int maskcol;
    int amonobitcount;
    unsigned char amonobitval;
    int calpha;       		 // Constant alpha value
    int calpha2,calpha3,calpha4; // Used for groovy accelerated effect
    unsigned char * maskp;
    QMemoryManager::FontID myfont;

    int clipcursor;
    QRect* cliprect;
    int ncliprect;

    int globalRegionIndex;
    int *globalRegionRevision;
    int currentRegionRevision;

    RasterOp myrop;

    unsigned long int pixel; // == cpen.pixel() or cbrush.pixel()

    friend class QScreenCursor;
};

template <const int depth, const int type>
class QGfxRaster : public QGfxRasterBase, private QPolygonScanner {

public:

    QGfxRaster(unsigned char *,int w,int h);
    ~QGfxRaster();

    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void drawRect( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void blt( int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif
    virtual void tiledBlt( int,int,int,int );

    virtual int bitDepth() { return depth; }

    virtual void setSource(const QImage *);
    virtual void setSource(const QPaintDevice *);

protected:

    virtual void drawThickLine( int,int,int,int );

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





