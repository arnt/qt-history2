/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgfxmach64_qws.cpp#73 $
**
** Implementation of QGfxMach64 (graphics context) class for Mach64 cards
*
** Created : 940721
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <qapplication.h>

//#define DEBUG_INIT

#include <sys/io.h>
#include <unistd.h>
#include <stdio.h>

#include "qgfxraster_qws.h"
#include "qgfxmach64defs_qws.h"

#define LASTOP_LINE 1
#define LASTOP_RECT 2
#define LASTOP_BLT 3
#define LASTOP_BLTPEN 4
#define LASTOP_STRETCHBLT 5
#define LASTOP_RESET 6
#define LASTOP_POLYGON 7
#define LASTOP_TEXT 8
#define LASTOP_ALPHA 9
#define LASTOP_TILEDBLT 10
#define LASTOP_TILEDBLTPEN 11


unsigned char * regbase=0;
unsigned char * regbase2=0;

inline unsigned int regr(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(regbase+regindex));
    return val;
}

inline void regw(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(regbase+regindex))=val;
}

inline void regw2(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(regbase2+regindex))=val;
}

inline void regwf2(volatile unsigned int regindex,float val)
{
    unsigned int writeval;
    *((float *)&writeval)=val;
    *((volatile unsigned long int *)(regbase2+regindex))=writeval;
}

inline unsigned int regr2(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(regbase2+regindex));
    return val;
}

inline void wait_for_fifo(short entries)
{
    int trycount=0;

    while(trycount++) {
	int fifoval=regr(FIFO_STAT);
	if(fifoval & 0x80000000) {
	    qDebug("Resetting engine");
            wait_for_fifo(1);
            regw(GEN_TEST_CNTL,(regr(GEN_TEST_CNTL) & 0xfffffeff));
	    wait_for_fifo(1);
	    regw(GEN_TEST_CNTL,(regr(GEN_TEST_CNTL) | 0x00000100));
	    wait_for_fifo(1);
	    regw(BUS_CNTL,regr(BUS_CNTL) | 0x08a00000);
	    return;
	}
	fifoval=fifoval & 0xffff;
	unsigned int loopc;
	int count=0;
	for(loopc=0;loopc<16;loopc++) {
	    if(!(fifoval & 0x1))
		count++;
	    fifoval=fifoval >> 1;
	}
	if(count>=entries) {
	    return;
	}
	if(trycount>10) {
	    qDebug("Resetting engine");
            wait_for_fifo(1);
            regw(GEN_TEST_CNTL,(regr(GEN_TEST_CNTL) & 0xfffffeff));
	    wait_for_fifo(1);
	    regw(GEN_TEST_CNTL,(regr(GEN_TEST_CNTL) | 0x00000100));
	    wait_for_fifo(1);
	    regw(BUS_CNTL,regr(BUS_CNTL) | 0x08a00000);
	}
    }
}

inline void reset_engine()
{
    lastop=LASTOP_RESET;
    // We use wait_for_fifo(1)'s in case the fifo queue has bunged up
    // for some reason; this is safer
    wait_for_fifo(1);
    regw(GEN_TEST_CNTL,(regr(GEN_TEST_CNTL) & 0xfffffeff));
    wait_for_fifo(1);
    regw(GEN_TEST_CNTL,(regr(GEN_TEST_CNTL) | 0x00000100));
    wait_for_fifo(1);
    regw(BUS_CNTL,regr(BUS_CNTL) | 0x08a00000);
}

inline void wait_for_idle()
{
    wait_for_fifo(16);

    while(true) {
	if((regr(GUI_STAT) & 0x1)==0)
	    return;
    }
}

template <const int depth, const int type>
class QGfxMach64 : public QGfxRaster<depth,type> {

public:

    QGfxMach64(unsigned char *,int w,int h);

    virtual void drawLine(int,int,int,int);
    virtual void drawRect(int,int,int,int);
    virtual void blt( int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif
    virtual void drawText(int,int,const QString &);
    virtual void drawPolygon(const QPointArray &,bool winding,int index,
			     int npoints);
    virtual void tiledBlt(int,int,int,int);

    virtual void drawAlpha(int,int,int,int,int,int,int,int);

    virtual void sync();

    void hsync(int);

    void setPixWidth(int,int,int=16,bool=true);

private:

    unsigned char * regbase;
    unsigned char * regbase2;

    void do_scissors(QRect &);

    bool checkSourceDest();
    bool checkDest();

    QGfx * proxygfx;

};

#define vgabase 0x1000

template<const int depth,const int type>
QGfxMach64<depth,type>::QGfxMach64(unsigned char * a,int b,int c)
    : QGfxRaster<depth,type>(a,b,c)
{
}

template<const int depth,const int type>
inline void QGfxMach64<depth,type>::do_scissors(QRect & r)
{
    wait_for_fifo(4);
    regw(SC_LEFT,r.left());
    regw(SC_TOP,r.top());
    regw(SC_RIGHT,r.right());
    regw(SC_BOTTOM,r.bottom());
}

template<const int depth,const int type>
inline void QGfxMach64<depth,type>::setPixWidth(int d,int s,int sc,bool b)
{
    unsigned int wmask;
    if(b) {
        wmask=BYTE_ORDER_LSB_TO_MSB;
    } else {
	wmask=BYTE_ORDER_MSB_TO_LSB;
    }
    if(d==16) {
	wmask|=DST_16BPP;
    } else {
	// 32
	wmask|=DST_32BPP;
    }
    if(s==16) {
	wmask|=SRC_16BPP | HOST_16BPP;
    } else {
	// 32
	wmask|=SRC_32BPP | HOST_32BPP;
    }
    if(sc==32) {
	wmask|=SCALE_32BPP;
    } else {
	wmask|=SCALE_16BPP;
    }
    wait_for_fifo(1);
    regw(DP_PIX_WIDTH,wmask);
}

// Set up SRC_OFF_PITCH and DST_OFF_PITCH
// Return true if they're both on the card

template<const int depth,const int type>
inline bool QGfxMach64<depth,type>::checkSourceDest()
{
    if ( checkDest() )
	return FALSE;

    // This is not really needed.
    QArray<QRect> rects=setrgn.rects();
    if(rects.size()==0) {
	wait_for_fifo(4);
	regw(SC_LEFT,0);
	regw(SC_TOP,0);
	regw(SC_RIGHT,width);
	regw(SC_BOTTOM,height);
	//qDebug("No region to paint");
	return FALSE;
    }

    int sourcepixelpitch;
    ulong src_buffer_offset;
    if (srctype == SourcePen) {
	src_buffer_offset = -1;
    } else {
	if (!qt_screen->onCard(srcbits,src_buffer_offset))
	    return FALSE;
	if (srclinestep==0) {
	    sourcepixelpitch=width;
	} else {
	    sourcepixelpitch=(srclinestep*8)/srcdepth;
	}
	wait_for_fifo(1);
	regw(SRC_OFF_PITCH,( (sourcepixelpitch / 8 ) << 22) |
	     (src_buffer_offset / 8) );
    }

    return TRUE;
}

// Set up DST_OFF_PITCH, return false if it's not on the card
// For lines, filled rects etc

template<const int depth,const int type>
inline bool QGfxMach64<depth,type>::checkDest()
{
    ulong buffer_offset;
    if (!qt_screen->onCard(buffer,buffer_offset)) {
	return FALSE;
    }
    if (depth!=16) {
	return FALSE;
    }
    int pixelstep=(linestep()*8)/depth;
    wait_for_fifo(1);
    regw(DST_OFF_PITCH,(( pixelstep / 8 ) << 22) | (buffer_offset / 8));
    return TRUE;
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::drawLine(int x1,int y1,int x2,int y2)
{
    QArray<QRect> rects=setrgn.rects();

    if(rects.size()<1)
        return;

    if(cpen.style()==NoPen)
	return;

    if ( cpen.style() != SolidLine) {
	QGfxRaster<depth,type>::drawLine(x1,y1,x2,y2);
	return;
    }

    if(optype!=1 || lastop!=LASTOP_LINE) {
	if(!checkDest()) {
	    QGfxRaster<depth,type>::drawLine(x1,y1,x2,y2);
	    return;
	}
	if(optype>1)
	    wait_for_idle();

	wait_for_fifo(2);
	regw(DP_SRC,0x00000100);
	regw(DP_MIX,(MIX_SRC << 16) | MIX_DST);

	lastop=LASTOP_LINE;
    }

    optype=1;

    unsigned int loopc;

    x1=x1+xoffs;
    y1=y1+yoffs;
    x2=x2+xoffs;
    y2=y2+yoffs;


    if(x1>x2) {
      int x3,y3;
      x3=x2;
      y3=y2;
      x2=x1;
      y2=y1;
      x1=x3;
      y1=y3;
      x1=x3;
      y1=y3;
    }

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, abs(x2-x1)+1, abs(y2-y1)+1))
    wait_for_fifo(1);
    regw(DP_FRGD_CLR,cpen.color().pixel());

    for(loopc=0;loopc<rects.size();loopc++) {
	// Code taken from Mach64 Programmer's Manual
	int dx,dy;
	int mindelta,maxdelta;
	int xdir,ydir,ymajor;
	dx=abs(x2-x1);
	dy=abs(y2-y1);
	mindelta=dx>dy ? dy : dx;      // min
	maxdelta=dx>dy ? dx : dy;	   // max
	if(x1<x2)
	    xdir=1;
	else
	    xdir=0;
	if(y1<y2)
	    ydir=0x0802;
	else
	    ydir=0;
	if(dx<dy)
	    ymajor=4;
	else
	    ymajor=0;

	unsigned int rval=0x00000003;
	rval=(rval & ~0x7) | (unsigned long)(ymajor | ydir | xdir);

	do_scissors(rects[loopc]);

	wait_for_fifo(6);
	regw(DST_CNTL,rval);
	regw(DST_Y_X,((unsigned long)x1 << 16) | y1);
	regw(DST_BRES_ERR,(2*mindelta)-maxdelta);
	regw(DST_BRES_INC,2*mindelta);
	regw(DST_BRES_DEC,2*(mindelta-maxdelta));
	regw(DST_BRES_LNTH,maxdelta+1);
    }
    GFX_END
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::drawRect(int rx,int ry,int w,int h)
{
    QArray<QRect> rects=setrgn.rects();

    if(rects.size()<1) {
	return;
    }

    if( (cbrush.style()!=NoBrush) && (cbrush.style()!=SolidPattern) ) {
	QGfxRaster::drawRect(rx,ry,w,h);
	return;
    }

    if(!checkDest()) {
	QGfxRaster::drawRect(rx,ry,w,h);
	return;
    }

    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))
    // Now draw the lines round the edge if necessary
    if(cpen.style()!=NoPen) {
	drawLine(rx,ry,rx+(w-1),ry);
	drawLine(rx+(w-1),ry+1,rx+(w-1),ry+(h-2));
	drawLine(rx,ry+(h-1),rx+(w-1),ry+(h-1));
	drawLine(rx,ry+1,rx,ry+(h-1));
	rx++;
	ry++;
	w-=2;
	h-=2;
    }

    if(optype!=1 || lastop!=LASTOP_RECT) {
	wait_for_fifo(7);

	// probably not needed
	regw(SC_LEFT,0);
	regw(SC_TOP,0);
	regw(SC_RIGHT,width);
	regw(SC_BOTTOM,height);

        regw(DP_SRC,0x00000100);
        regw(DP_MIX,(MIX_SRC << 16) | MIX_DST);
        regw(DST_CNTL,0x00000003);

	lastop=LASTOP_RECT;
    }

    if(optype>1)
	sync();
    optype=1;

    int loopc;

    int xp=rx+xoffs;
    int yp=ry+yoffs;

    int x2,y2;

    x2=xp+(w-1);
    y2=yp+(h-1);

    int x3,y3,x4,y4;

    QColor tmp=cbrush.color();
    wait_for_fifo(1);
    regw(DP_FRGD_CLR,tmp.alloc());

    if(cbrush.style()!=NoBrush) {
	int p=rects.size();
	if(p<8) {
	    // We can wait for all our fifos at once
	    wait_for_fifo(p*2);
	    for(loopc=0;loopc<p;loopc++) {
		QRect r=rects[loopc];
		if(xp<=r.right() && yp<=r.bottom() &&
		   x2>=r.left() && y2>=r.top()) {
		    x3=r.left() > xp ? r.left() : xp;
		    y3=r.top() > yp ? r.top() : yp;
		    x4=r.right() > x2 ? x2 : r.right();
		    y4=r.bottom() > y2 ? y2 : r.bottom();
		    regw(DST_Y_X,(x3 << 16) | y3);
		    regw(DST_HEIGHT_WIDTH,( ( (x4-x3) +1) << 16) |
			 ( (y4-y3) +1));
		}
	    }
	} else {
	    for(loopc=0;loopc<p;loopc++) {
		QRect r=rects[loopc];
		if(xp<=r.right() && yp<=r.bottom() &&
		   x2>=r.left() && y2>=r.top()) {
		    x3=r.left() > xp ? r.left() : xp;
		    y3=r.top() > yp ? r.top() : yp;
		    x4=r.right() > x2 ? x2 : r.right();
		    y4=r.bottom() > y2 ? y2 : r.bottom();
		    wait_for_fifo(2);
		    regw(DST_Y_X,(x3 << 16) | y3);
		    regw(DST_HEIGHT_WIDTH,( ( (x4-x3) +1) << 16) |
			 ( (y4-y3) +1));
		}
	    }
	}
    }
    GFX_END
}

template<const int depth,const int type>
inline void QGfxMach64<depth,type>::blt(int rx,int ry,int w,int h)
{
    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1)
	return;

    if(depth!=16) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    bool canaccel=false;

    if(srcdepth==32) {
	if(alphatype==IgnoreAlpha || alphatype==SolidAlpha ||
	   alphatype==InlineAlpha) {
	    canaccel=true;
	}

	if(alphatype!=IgnoreAlpha) {
	    // Mach64 requires textures to be these sizes
	    int p=srclinestep/4;
	    if(p!=1024 && p!=512 && p!=256 && p!=128 && p!=64 && p!=32 &&
	       p!=16 && p!=8) {
		canaccel=false;
	    }
	}
    } else if(srcdepth==16) {
	if(alphatype==IgnoreAlpha)
	    canaccel=true;
    }

    if(srctype==SourceImage && canaccel==false) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if(srctype==SourcePen && !(alphatype==BigEndianMask ||
			       alphatype==LittleEndianMask) ) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if( (srcdepth!=32) && (srcdepth!=16) ) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if( (alphatype==InlineAlpha || alphatype==SolidAlpha)
	&& checkSourceDest() ) {
	int x2=(rx+w)-1;
	int y2=(ry+h)-1;
	drawAlpha(rx,ry,x2,ry,rx,y2,x2,y2);
	return;
    }

    if(checkSourceDest()) {

	int xp=xoffs+rx;
	int yp=yoffs+ry;
	int xp2=srcoffs.x();
	int yp2=srcoffs.y();

	QRect cursRect(xp, yp, w+1, h+1);
	GFX_START(cursRect)

	if(srctype==SourceImage) {

	    if(optype!=1 || lastop!=LASTOP_BLT) {
		lastop=LASTOP_BLT;

		setPixWidth(depth,srcdepth);

		wait_for_fifo(8);
		regw(DP_WRITE_MASK,0xffffffff);
		regw(DP_MIX,0x00070003);
		regw(DP_SRC,0x00000300);
		regw(CLR_CMP_CNTL,0x00000000);
		regw(DP_FRGD_CLR,0xffffffff);

		if(yp>yp2) {
		    // Down, reverse
		    if(xp>xp2) {
			// Right, reverse
			xp+=(w-1);
			xp2+=(w-1);
			yp+=(h-1);
			yp2+=(h-1);
			regw(DST_CNTL,0x00000000);
		    } else {
			// Left, normal
			yp+=(h-1);
			yp2+=(h-1);
			regw(DST_CNTL,0x00000001);
		    }
		} else {
		    // Up, normal
		    // Down, reverse
		    if(xp>xp2) {
			// Right, reverse
			xp+=(w-1);
			xp2+=(w-1);
			regw(DST_CNTL,0x00000002);
		    } else {
			// Left, normal
			regw(DST_CNTL,0x00000003);
		    }
		}
		regw(SRC_CNTL,0x00000000);
		regw(SRC_Y_X,(xp2 << 16) | yp2);
	    }
	} else {
	    if(optype!=1 || lastop!=LASTOP_BLTPEN) {
		setPixWidth(depth,srcdepth,16,alphatype==LittleEndianMask);
		wait_for_fifo(4);
		regw(DP_WRITE_MASK,0xffffffff);
		QColor tmp=cpen.color();
		regw(DP_FRGD_CLR,tmp.alloc());
		regw(DP_MIX,0x00070003);
		wait_for_fifo(4);
		regw(DP_SRC,0x00030100);
		regw(CLR_CMP_CNTL,0x00000000);
		regw(SRC_CNTL,0x00000004);
		regw(DST_CNTL,0x00000003);
		regw(SRC_Y_X,0);
		lastop=LASTOP_BLTPEN;
	    }
	}

	optype=1;

	unsigned int loopc;
	for(loopc=0;loopc<rects.size();loopc++) {

	    do_scissors(rects[loopc]);

	    wait_for_fifo(3);
	    regw(SRC_WIDTH1,w);
	    regw(DST_Y_X,(xp << 16) | yp);
	    regw(DST_HEIGHT_WIDTH,(w << 16) | h);
	}
	GFX_END
	return;
    } else {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
    }
}

template <const int depth, const int type>
void QGfxMach64<depth,type>::scroll( int rx,int ry,int w,int h,int sx,int sy )
{
    srclinestep=linestep();
    srcdepth=depth;
    srcbits=buffer;
    if(srcdepth==0)
        abort();
    srctype=SourceImage;
    alphatype=IgnoreAlpha;
    ismasking=false;
    setSourceOffset(sx,sy);
    blt(rx,ry,w,h);
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template<const int depth,const int type>
void QGfxMach64<depth,type>::stretchBlt(int rx,int ry,int w,int h,
					int sw,int sh)
{
    // This doesn't use the 2d engine, it uses the 3d/scaler pipeline
    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1)
	return;

    if ( srctype!=SourceImage || !checkSourceDest() ) {
	QGfxRaster<depth,type>::stretchBlt(rx,ry,w,h,sw,sh);
	return;
    }

    lastop=LASTOP_STRETCHBLT;

    if(optype!=2)
	wait_for_idle();
    optype=2;

    int xp=xoffs+rx;
    int yp=yoffs+ry;

    QRect cursRect(rx, ry, w+1, h+1);
    GFX_START(cursRect)

    unsigned int loopc;
    for(loopc=0;loopc<rects.size();loopc++) {

	wait_for_fifo(3);
	regw(SCALE_3D_CNTL,0x00000040);
	regw(ALPHA_TEST_CNTL,0);
	regw(TEX_CNTL,0);
	if(srclinestep<1)
	    abort();

	unsigned long my_src_buffer_offset;
	if (!qt_screen->onCard(srcbits,my_src_buffer_offset))
	    qFatal("checkSourceDest() lied!");

	int srcpixelpitch=srclinestep;
	srcpixelpitch=(srcpixelpitch*8)/srcdepth;
	srcpixelpitch=srcpixelpitch >> 3;
	srcpixelpitch=srcpixelpitch << 3;

	// Need to convert source width/width to
	// fixed-point 8.12 floating point format
	double tmp1=w;
	double tmp2=sw;
	double tmp3=h;
	double tmp4=sh;
	double screentosource=tmp4/tmp3;
	tmp1=tmp2/tmp1;
	tmp3=tmp4/tmp3;


	int mul=4096;
	tmp1*=mul;
	tmp3*=mul;
	unsigned long int h1=(unsigned long int)tmp1;
	unsigned long int h2=(unsigned long int)tmp3;

	// This is a workaround for a bug with hardware clipping -
	// if the stretchblt starts outside the hardware clip
	// rectangle /none/ of it is drawn

	int sy1=rects[loopc].top();

	double mytmp=sy1-yp;
	mytmp*=screentosource;

	int add=(int)mytmp;
	add=add*srclinestep;

	my_src_buffer_offset+=add;

	if(my_src_buffer_offset & 0x7)
	    abort();

	wait_for_fifo(10);

	regw(SCALE_OFF,my_src_buffer_offset);  // Scaler source
	regw(SECONDARY_SCALE_OFF_ACC,srcpixelpitch);
	regw(SCALE_PITCH,srcpixelpitch);
	regw(SECONDARY_SCALE_PITCH,srcpixelpitch);

	regw(SCALE_WIDTH,sw);
	regw(SCALE_HEIGHT,sh);
	regw(SCALE_HACC,0x00000000);
	regw(SCALE_VACC,0x00000000);
	regw(SECONDARY_SCALE_HACC,0x00000000);
	regw(SECONDARY_SCALE_VACC,0x00000000);

	setPixWidth(depth,srcdepth);

	wait_for_fifo(9);
	regw(SCALE_X_INC,h1 << 4);
	regw(SECONDARY_SCALE_X_INC,h1 << 4);
	regw(SCALE_Y_INC,h2 << 4);
	regw(SECONDARY_SCALE_Y_INC,h2 << 4);

	regw(DP_SRC,0x00000505);
	regw(DP_WRITE_MASK,0xffffffff);
	regw(DP_MIX,0x00070003);
	regw(GUI_TRAJ_CNTL,0x00800003);

	wait_for_fifo(6);
	regw(CLR_CMP_CNTL,0x02000000);
	regw(DST_CNTL,0x00000003);

	do_scissors(rects[loopc]);

	regw(DST_X,xp);
	regw(DST_Y,sy1);
	regw(DST_HEIGHT,h);
	regw(DST_WIDTH,w);

	wait_for_idle();
	usleep(1);
	reset_engine();
    }
    GFX_END
}
#endif



template<const int depth,const int type>
void QGfxMach64<depth,type>::sync()
{
    wait_for_idle();
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::drawPolygon(const QPointArray & pa,
					 bool winding, int index,
					 int npoints)
{
    // Fall back for now since it sometimes seems to get it wrong
    QGfxRaster<depth,type>::drawPolygon(pa,winding,index,npoints);
    return;

    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1) {
	return;
    }


    // Blt source for polygon is illegal
    QRect r=pa.boundingRect();

    int pitch=r.width()/8;

    if(pitch<8)
	pitch=8;
    if((r.width() & 8)!=0)
	pitch++;
    while(pitch & 0x7)
	pitch++;

    //int size=pitch * r.height();

    setPixWidth(1,1,1,false);

    QRect cursRect(r);
    cursRect.moveBy(xoffs, yoffs);
    GFX_START(cursRect)

    wait_for_fifo(10);
    regw(GUI_TRAJ_CNTL,0x00000003);
    regw(DP_WRITE_MASK,0xffffffff);
    regw(DP_MIX,0x00010001);
    regw(DP_SRC,0x00000100);
    regw(CLR_CMP_CNTL,0x00000000);
    regw(SC_LEFT_RIGHT, (r.width()-1) << 16);
    regw(SC_TOP_BOTTOM, (r.height()-1) << 16);
    regw(DST_OFF_PITCH,( (pitch) << 22) /*| someoffset*/ );
    regw(DST_Y_X,0x00000000);
    regw(DST_HEIGHT_WIDTH,(r.width() << 16) | r.height());

    int x1,y1,x2,y2;

    unsigned int loopc;

    wait_for_fifo(3);
    regw(DST_CNTL,0x00000040);
    regw(DP_MIX,0x00050005);
    regw(DP_FRGD_CLR,0xffffffff);

    for(loopc=0;loopc<(pa.size()-1);loopc++) {
	// Code taken from Mach64 Programmer's Manual

	x1=pa[loopc].x()-r.left();
	y1=pa[loopc].y()-r.top();
	x2=pa[loopc+1].x()-r.left();
	y2=pa[loopc+1].y()-r.top();

	if(y2<y1) {
	    int swap;
	    swap=x1;
	    x1=x2;
	    x2=swap;
	    swap=y1;
	    y1=y2;
	    y2=swap;
	}

	int dx,dy;
	int mindelta,maxdelta;
	int xdir,ydir,ymajor;
	dx=abs(x2-x1);
	dy=abs(y2-y1);
	mindelta=dx>dy ? dy : dx;      // min
	maxdelta=dx>dy ? dx : dy;	   // max
	if(x1<x2)
	    xdir=1;
	else
	    xdir=0;
	if(y1<y2)
	    ydir=0x0802;
	else
	    ydir=0;
	if(dx<dy)
	    ymajor=4;
	else
	    ymajor=0;

	unsigned int rval=0x00000003;
	rval=(rval & ~0x7) | (unsigned long)(ymajor | ydir | xdir);

	wait_for_fifo(6);
	regw(DST_CNTL,rval | 0x40);
	regw(DST_Y_X,((unsigned long)x1 << 16) | y1);
	regw(DST_BRES_ERR,(2*mindelta)-maxdelta);
	regw(DST_BRES_INC,2*mindelta);
	regw(DST_BRES_DEC,2*(mindelta-maxdelta));
	regw(DST_BRES_LNTH,maxdelta+1);

    }

    x1=xoffs+r.left();
    y1=yoffs+r.top();
    x2=xoffs+r.right();
    y2=yoffs+r.bottom();

    setPixWidth(depth,1,1,false);

    wait_for_fifo(8);
    regw(DST_CNTL,0x00000043);
    regw(DP_MIX,0x00070007);
    regw(DP_SRC,0x00000100);
    QColor tmp=cbrush.color();
    regw(DP_FRGD_CLR,tmp.alloc());

    lastop=LASTOP_POLYGON;

    optype=1;

    regw(SRC_CNTL,0x00000000);
    //regw(SRC_OFF_PITCH,(pitch << 22) /* some offset */ );
    regw(SRC_Y_X,0x00000000);
    regw(SRC_WIDTH1,r.width());
    checkDest();

    unsigned int loopc2;

    for(loopc2=0;loopc2<rects.size();loopc2++) {
	do_scissors(rects[loopc2]);
	wait_for_fifo(2);
	regw(DST_Y_X,(x1 << 16) | y1);
	regw(DST_HEIGHT_WIDTH,(r.width() << 16) | r.height());
    }
    GFX_END
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::tiledBlt(int rx,int ry,int w,int h)
{
    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1) {
	return;
    }

    if(srctype==SourceImage && (alphatype!=IgnoreAlpha) ) {
	QGfxRaster<depth,type>::tiledBlt(rx,ry,w,h);
	return;
    }

    if( ( (srcdepth!=16 && srcdepth!=1)
	  && srctype==SourceImage) || depth!=16 ) {
	QGfxRaster<depth,type>::tiledBlt(rx,ry,w,h);
	return;
    }

    if ( srctype==SourcePen && checkDest() ) {
	QGfxRaster<depth,type>::tiledBlt(rx,ry,w,h);
	return;
    }

    if ( srctype==SourceImage && !checkSourceDest() ) {
	QGfxRaster<depth,type>::tiledBlt(rx,ry,w,h);
	return;
    }

    int xp=xoffs+rx;
    int yp=yoffs+ry;

    GFX_START(QRect(xp, yp, w+1, h+1))

    if(srctype==SourceImage) {
	if(optype!=1 || lastop!=LASTOP_TILEDBLT) {
	    wait_for_fifo(4);
	    regw(SRC_CNTL,0x00000003);
	    regw(DP_WRITE_MASK,0xffffffff);
	    regw(CLR_CMP_CNTL,0x00000000);
	    regw(DST_CNTL,0x00000003);
	    setPixWidth(depth,srcdepth);
	    if(srcdepth==16 || srcdepth==32) {
		wait_for_fifo(3);
		regw(DP_MIX,0x00070003);
		regw(DP_SRC,0x00000300);
		regw(DP_FRGD_CLR,0xffffffff);
	    } else {
		setPixWidth(depth,1,1,!src_little_endian);
		wait_for_fifo(4);
		regw(DP_MIX,0x00070007);
		regw(DP_SRC,0x00030100);
		regw(DP_FRGD_CLR,srcclut[0]);
		regw(DP_BKGD_CLR,srcclut[1]);
	    }
	    lastop=LASTOP_TILEDBLT;
	}
    } else {
	if(optype!=1 || lastop!=LASTOP_TILEDBLTPEN) {
	    setPixWidth(depth,1,1,alphatype==LittleEndianMask);
	    wait_for_fifo(7);
	    regw(SRC_CNTL,0x00000001);
	    regw(DP_WRITE_MASK,0xffffffff);
	    QColor tmp=cpen.color();
	    regw(DP_FRGD_CLR,tmp.alloc());
	    regw(DP_MIX,0x00070003);
	    regw(DP_SRC,0x00030100);
	    regw(CLR_CMP_CNTL,0x00000000);
	    regw(DST_CNTL,0x00000003);
	    lastop=LASTOP_TILEDBLTPEN;
	}
    }

    optype=1;

    wait_for_fifo(4);
    regw(SC_LEFT,0);
    regw(SC_TOP,0);
    regw(SC_RIGHT,width);
    regw(SC_BOTTOM,height);

    int xp2,yp2;
    int xp3,yp3;
    xp3=(xp+w)-1;
    yp3=(yp+h)-1;

    unsigned int loopc;
    for(loopc=0;loopc<rects.size();loopc++) {
	xp2=srcoffs.x();
	yp2=srcoffs.y();

	QRect r=rects[loopc];
	int myxp=xp > r.left() ? xp : r.left();
	int myyp=yp > r.top() ? yp : r.top();
	int myxp2=xp3 > r.right() ? r.right() : xp3;
	int myyp2=yp3 > r.bottom() ? r.bottom() : yp3;

	int ww2=(myxp2-myxp)+1;
	int hh2=(myyp2-myyp)+1;

	int xo=(myxp-xp);
	int yo=(myyp-yp);

	xp2+=xo;
	yp2+=yo;

	while(xp2<0) {
	    xp2+=srcwidth;
	}
	while(yp2<0) {
	    yp2+=srcheight;
	}

	while(xp2>srcwidth)
	    xp2-=srcwidth;

	while(yp2>srcheight)
	    yp2-=srcheight;

	if(ww2>0 && hh2>0) {
	    wait_for_fifo(6);
	    regw(SRC_Y_X_START,0);
	    regw(SRC_HEIGHT2_WIDTH2,(srcwidth << 16) | srcheight);
	    regw(SRC_HEIGHT1_WIDTH1,( (srcwidth-xp2) << 16 ) | srcheight-yp2);
	    regw(SRC_Y_X,(xp2 << 16) | yp2);
	    regw(DST_Y_X,(myxp << 16) | myyp);
	    regw(DST_HEIGHT_WIDTH,(ww2 << 16) | hh2);
	}
    }

    GFX_END
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::drawText(int x,int y,const QString & s)
{
    QGfxRaster::drawText(x,y,s);
    return;

    /*
    x++; // Demo hack

    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1)

	return;

    GFX_START(clipbounds)

    if(optype!=1 || lastop!=LASTOP_TEXT) {
	int pixelstep=(linestep() * 8)/depth;

	setPixWidth(depth,1,1,false);

	wait_for_fifo(9);
	regw(DST_OFF_PITCH,(( pixelstep/8 ) << 22) |
	     (buffer_offset / 8));
	regw(DP_WRITE_MASK,0xffffffff);
	QColor tmp=cpen.color();
	regw(DP_FRGD_CLR,tmp.alloc());
	regw(DP_MIX,0x00070003);
	regw(DP_SRC,0x00030100);
	regw(CLR_CMP_CNTL,0x00000000);
	regw(SRC_CNTL,0x00000004);
	regw(DST_CNTL,0x00000003);
	regw(SRC_Y_X,0);
	lastop=LASTOP_TEXT;
    }

    optype=1;

    unsigned int loopc2;

    int * offsets=(int *)chunkptr;

    for(loopc=1;loopc<=rects.size();loopc++) {

	int tx=x+xoffs;
	int ty=y+yoffs;

	do_scissors(rects[loopc]);

	for( loopc2=0; loopc2 < s.length(); loopc2++ ) {
	    int index;
	    index=unicache[loopc2];
	    if(index>-1) {
		unsigned char * glyphptr=chunkptr;
		int offp=offsets[index];
		glyphptr+=offp;
		QGlyphMetrics * glyph=(QGlyphMetrics *)glyphptr;

		if(glyph->linestep==-1) {
		    tx+=glyph->advance;
		} else {
		    wait_for_fifo(5);
		    regw(SRC_OFF_PITCH,( glyph->linestep << 22) |
			 (src_buffer_offset/8) );
		    regw(SRC_WIDTH1,(glyph->linestep << 3));
		    int tty=ty-glyph->bearingy;
		    bool skipit=false;
		    if(tty<0) {
			// Oops, trying to draw above drawable.
			// Correct source offset to compensate
			tty=-tty;
			if(tty>glyph->height) {
			    skipit=true;
			} else {
			    regw(SRC_Y_X,tty);
			}
			tty=0;
		    }
		    if(!skipit) {
			regw(DST_Y_X,( (tx+ (glyph->bearingx) ) << 16) |
			     tty );
			// Normally it's a shift by 16,
			// this way we multiply by 8 too
			regw(DST_HEIGHT_WIDTH,(glyph->linestep << 19) |
			     glyph->height);
		    }
		    tx+=glyph->advance;
		}
	    }
	}
    }
    GFX_END
#ifdef DEBUG_LOCKS
    qDebug("accelerated drawText unlock");
#endif
    delete[] unicache;
    setAlphaType(IgnoreAlpha);
    */
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::hsync(int i)
{
    unsigned int loopc;
    int tmp;
    for(loopc=0;loopc<100000;loopc++) {
	tmp=regr(CRTC_VLINE_CRNT_VLINE);
	tmp=(tmp & 0x07ff0000) >> 16;
	if(tmp>i)
	    return;
    }
}

template<const int depth,const int type>
void QGfxMach64<depth,type>::drawAlpha(int x1,int y1,int x2,int y2,
				       int x3,int y3,int x4,int y4)
{
    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1) {
	return;
    }

    if(!checkSourceDest())
	return;

    optype=2;
    lastop=LASTOP_ALPHA;

    int xx[4];
    int yy[4];

    // 2 fractional bits
    xx[0]=(x1+xoffs);
    yy[0]=(y1+yoffs);
    xx[1]=(x2+xoffs);
    yy[1]=(y2+yoffs);
    xx[2]=(x3+xoffs);
    yy[2]=(y3+yoffs);
    xx[3]=(x4+xoffs);
    yy[3]=(y4+yoffs);

    float xs=0.0;
    float ys=0.0;

    unsigned int loopc;
    for(loopc=0;loopc<4;loopc++) {
	if(xx[loopc]<0) {
	    float s1=x2-x1;
	    float p1=-(xx[loopc]);
	    xs=p1/s1;
	    xx[loopc]=0;
	}
	if(yy[loopc]<0) {
	    float s2=y3-y1;
	    float p2=-(yy[loopc]);
	    ys=p2/s2;
	    yy[loopc]=0;
	}
	xx[loopc]*=4;
	yy[loopc]*=4;
    }

    QRect cursRect(x1+xoffs, y1+yoffs, abs(x2-x1)+1, abs(y2-y1)+1);
    GFX_START(cursRect)

    for(loopc=0;loopc<rects.size();loopc++) {

	do_scissors(rects[loopc]);

	// Used for one-over-area
	float ooa,ooa2;

	ooa = 0.25 * 0.25 * ( ( xx[1] - xx[0] ) * ( yy[0] - yy[2] ) +
			      ( yy[1] - yy[0] ) * ( xx[2] - xx[0] ) );

	ooa = -1.0 / ooa;

	ooa2 = 0.25 * 0.25 * ( ( xx[1] - xx[3] ) * ( yy[3] - yy[2] ) +
			       ( yy[1] - yy[3] ) * ( xx[2] - xx[3] ) );

	ooa2 = -1.0 / ooa2;

	int s3df=2;
	int afn=1;
	int asat=0;
	// These source and destination alpha values are guesswork but seem
	// to give the right effect
	// source blend factor is AsAsAs, destination blend factor is
	// 1-As,1-As,1-As. I wish I knew more maths...
	int asrc=4;     // 1
	int adst=5;
	int tlf=0;
	int tfilt=0;
	int tblend=0;
	int texalpha=0;
	if(srcdepth==32 && alphatype==InlineAlpha)
	    texalpha=1;
	int split=0;

	// We only use inline alpha; Mach64 doesn't seem to easily support
	// separate alpha channel

	unsigned int scale3d=
	    ( 1 << 0 )              // 0 = zero extend, 1 = dynamic range
	                        // extend pixels to 32 bit
	    | ( 0 << 1 )            // 1 = 2D dither, 0 = error diffusion dither
	    | ( 0 << 2 )            // 1 = enable dither
	    | ( 0 << 3 )            // 1 = reset error diffusion each line
	    | ( 1 << 4 )            // 1 = round instead of dither
	    | ( 0 << 5 )            // 1 = disable texture cache
	    | ( s3df << 6 )         // 3=shading, 2=texture mapping, 1=scaling
	    | ( 0 << 8 )            // 1 = edge anti-alias
	    | ( split << 9 )        // 1 = split texture cache
	    | ( 0 << 10 )           // 1 = apple YUV mode
	    | ( afn << 11 )         // alpha / fog control
	    | ( asat << 13 )        // alpha saturate blending
	    | ( 0 << 14 )           // 1 = limit red dither range (what for???)
	    | ( 0 << 15 )           // 1 = signed dst blend clamp for mpeg
	    | ( asrc << 16 )        // blend src
	    | ( adst << 19 )        // blend dst
	    | ( tlf << 22 )         // texture environment
	    | ( 1 << 24 )           // 1 = disable mip-mapping (its broke on
	                        // all ragepros!)
	    | ( tfilt << 25 )       // 1 = bilinear filter texture on mag
	    | ( tblend << 26 )      // minification filtering
	    | ( 0 << 28 )           // 1 = LSB of alpha for texture masking
	    | ( 0 << 29 )           // alpha masking mode
	    | ( texalpha << 30 )    // 1 = texture has alpha
	    | ( 0 << 31 )           // 1 = source pixel from host register
	    ;

	setPixWidth(depth,srcdepth,srcdepth);

	wait_for_fifo(5);
	regw(DP_FRGD_CLR,0xffffffff);
	regw(DP_WRITE_MASK,0xffffffff);
	regw(DP_MIX,0x00070003);
	regw(CLR_CMP_CNTL,0);
	regw(GUI_TRAJ_CNTL,3);

	int p=srclinestep/4;

	int logpitch;
	if(p==1024) {
	    logpitch=0xa;
	} else if(p==512) {
	    logpitch=0x9;
	} else if(p==256) {
	    logpitch=0x8;
	} else if(p==128) {
	    logpitch=0x7;
	} else if(p==64) {
	    logpitch=0x6;
	} else if(p==32) {
	    logpitch=0x5;
	} else if(p==16) {
	    logpitch=0x4;
	} else if(p==8) {
	    logpitch=0x3;
	} else {
	    logpitch=0x2;
	}

	double wfx=srcwidth;
	//double wf2=p;  //### not used
	wfx=wfx/p;
	double wfy=srcheight;
	wfy=wfy/p;

	wait_for_fifo(6);
	regw(TEX_SIZE_PITCH,logpitch | (logpitch << 4) | (logpitch << 8)
	     | (logpitch << 12) | (logpitch << 16) | (logpitch << 20) |
	     (logpitch << 24));

	unsigned long foffset;
	if (!qt_screen->onCard(srcbits,foffset))
	    qFatal("checkSourceDest() lied!");

	regw(TEX_0_OFFSET,foffset);
	regw(TEX_1_OFFSET,foffset);
	regw(TEX_2_OFFSET,foffset);
	regw(TEX_3_OFFSET,foffset);
	regw(TEX_4_OFFSET,foffset);
	wait_for_fifo(7);
	regw(TEX_5_OFFSET,foffset);
	regw(TEX_6_OFFSET,foffset);
	regw(TEX_7_OFFSET,foffset);
	regw(TEX_8_OFFSET,foffset);
	regw(TEX_9_OFFSET,foffset);
	regw(TEX_10_OFFSET,foffset);

	unsigned int talpha=1;
	unsigned int ccf=1;
	unsigned int cblend=1;
	unsigned int cfilt=1;

	unsigned int tex;
	tex=(1 << 23) | (1 << 19) | (ccf << 9) | (cblend << 11) | (cfilt << 12)
	    | (talpha << 13);

	regw(TEX_CNTL,tex);

	// Fraction of pixmap to draw. Can go over 1.0 for tiling
	//float inv=1.0; //###not used
	//double wf=1.0;

	wait_for_fifo(5);
	regw(SCALE_3D_CNTL,scale3d);
	regw2(SETUP_CNTL,0x00000000);
	regw(Z_CNTL,0);
	regw(ALPHA_TEST_CNTL,0);
	regw(DP_SRC,DP_BKGD_SRC_3D | DP_FRGD_SRC_3D | DP_MONO_SRC_1);

	wait_for_fifo(5);
	regw2(VERTEX_1_ARGB,0x000000ff | calpha << 24);
	regw2(VERTEX_1_X_Y,xx[0] << 16 | yy[0]);
	regwf2(VERTEX_1_S,xs);
	regwf2(VERTEX_1_T,ys);
	regwf2(VERTEX_1_W,1.0);

	wait_for_fifo(5);
	regw2(VERTEX_2_ARGB,0x0000ff00 | calpha2 << 24);
	regw2(VERTEX_2_X_Y,xx[1] << 16 | yy[1]);
	regwf2(VERTEX_2_S,wfx);
	regwf2(VERTEX_2_T,ys);
	regwf2(VERTEX_2_W,1.0);

	wait_for_fifo(5);
	regw2(VERTEX_3_ARGB,0x00ff0000 | calpha4 << 24);
	regw2(VERTEX_3_X_Y,xx[2] << 16 | yy[2]);
	regwf2(VERTEX_3_S,xs);
	regwf2(VERTEX_3_T,wfy);
	regwf2(VERTEX_3_W,1.0);

	regwf2(ONE_OVER_AREA_UC,ooa);

	wait_for_fifo(6);

	regw2(VERTEX_1_ARGB,0x000000ff | calpha3 << 24);
	regw2(VERTEX_1_X_Y,xx[3] << 16 | yy[3]);
	regwf2(VERTEX_1_S,wfx);
	regwf2(VERTEX_1_T,wfy);
	regwf2(VERTEX_1_W,1.0);

	regwf2(ONE_OVER_AREA_UC,ooa2);

    }

    wait_for_fifo(2);
    regw(DP_SRC,0x300);
    regw(SCALE_3D_CNTL,0);

    GFX_END
}

class QMachScreen : public QScreen {

public:

    QMachScreen() { qDebug("No slot specified!"); }
    QMachScreen(char *,unsigned char *);
    virtual ~QMachScreen();
    virtual bool connect();
    virtual bool initCard();
    virtual int initCursor(void*, bool);
    virtual void shutdownCard();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
};

#ifndef QT_NO_QWS_CURSOR
class QMachCursor : public QScreenCursor
{
public:
    QMachCursor();
    ~QMachCursor();

    virtual void init(SWCursorData *,bool=FALSE);

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
    virtual void show();
    virtual void hide();

    bool restoreUnder( const QRect &, QGfxRasterBase * = 0 )
                { return FALSE; }
    void saveUnder() {}
    void drawCursor() {}
    void draw() {}
    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

private:

};
#endif // QT_NO_QWS_CURSOR

QMachScreen::QMachScreen(char * graphics_card_slot,unsigned char * config)
    : QScreen()
{
}

bool QMachScreen::connect()
{
    if ( !QScreen::connect() )
	return FALSE;

    unsigned char * bar=config+0x10;
    unsigned long int * addr=(unsigned long int *)bar;
    unsigned long int s=*(addr+2);
    unsigned long int olds=s;
    if(s & 0x1) {
#ifdef DEBUG_INIT
	printf("IO space - not right\n");
#endif
	return;
    } else {
#ifdef DEBUG_INIT
	qDebug("First address thing look right");
#endif
	s=s >> 1;
	s=s >> 2;
	s=olds;
	unsigned char * membase;
	int aperturefd;
	aperturefd=open("/dev/mem",O_RDWR);
	if(aperturefd==-1) {
#ifdef DEBUG_INIT
	    qDebug("Can't open /dev/mem");
#endif
	    return;
	}
	s=(s >> 4) << 4;
	membase=(unsigned char *)mmap(0,4096,PROT_READ |
				      PROT_WRITE,MAP_SHARED,
				      aperturefd,s);
	if(membase==0 || membase==(unsigned char *)-1) {
#ifdef DEBUG_INIT
	    qDebug("Failure to mmap /dev/mem, offset %d, %s",s,
		   strerror(errno));
#endif
	    close(aperturefd);
	    return;
	}
	regbase=membase+1024;
	regbase2=membase;
    }

    return TRUE;
}

QMachScreen::~QMachScreen()
{
}

bool QMachScreen::initCard()
{
    QScreen::initCard();

    // Lots of boilerplate from ATI manual, with some extra
    // from XFree Mach64 driver for good measure at the end

    reset_engine();

    wait_for_fifo(7);
    regw(CONTEXT_MASK,0xffffffff);
    regw(DST_OFF_PITCH, (w / 8) << 22);
    regw(DST_Y_X,0);
    regw(DST_HEIGHT,0);
    regw(DST_BRES_ERR,0);
    regw(DST_BRES_INC,0);
    regw(DST_BRES_DEC,0);
    wait_for_fifo(6);
    regw(SRC_OFF_PITCH, (w / 8 ) << 22);
    regw(SRC_Y_X,0);
    regw(SRC_HEIGHT1_WIDTH1,1);
    regw(SRC_Y_X_START,0);
    regw(SRC_HEIGHT2_WIDTH2,1);
    regw(SRC_CNTL,SRC_LINE_X_LEFT_TO_RIGHT);
    wait_for_fifo(13);
    regw(HOST_CNTL,0);
    regw(PAT_REG0,0);
    regw(PAT_REG1,0);
    regw(PAT_CNTL,0);
    regw(SC_LEFT,0);
    regw(SC_TOP,0);
    regw(SC_BOTTOM,h-1);
    regw(SC_RIGHT,w-1);
    regw(DP_BKGD_CLR,0);
    regw(DP_FRGD_CLR,0xffffffff);
    regw(DP_WRITE_MASK,0xffffffff);
    regw(DP_SRC,FRGD_SRC_FRGD_CLR);
    wait_for_fifo(3);
    regw(CLR_CMP_CLR,0);
    regw(CLR_CMP_MASK,0xffffffff);
    regw(CLR_CMP_CNTL,0);
    wait_for_fifo(2);
    regw(DP_PIX_WIDTH,HOST_16BPP | SRC_16BPP | DST_16BPP |
         BYTE_ORDER_LSB_TO_MSB);
    regw(DP_CHAIN_MASK,0x8410);
    wait_for_idle();

    wait_for_fifo(3);
    regw(DST_X,0);
    regw(DST_Y,0);
    regw(DST_HEIGHT,760);
    wait_for_idle();

    wait_for_fifo(5);
    regw(DP_FRGD_CLR,0xffffffff);
    regw(DP_WRITE_MASK,0xffffffff);
    regw(DP_SRC,0x00000100);
    regw(CLR_CMP_CNTL,0x00000000);
    regw(GUI_TRAJ_CNTL,0x00000003);

    wait_for_fifo(9);
    regw(DST_CNTL,0x3);
    regw(DST_BRES_ERR,0);
    regw(DST_BRES_INC,0);
    regw(DST_BRES_DEC,0);
    regw(SRC_Y_X,0);
    regw(SRC_HEIGHT1_WIDTH1,0);
    regw(SRC_Y_X_START,0);
    regw(SRC_HEIGHT2_WIDTH2,0);
    regw(SRC_CNTL,0);

    wait_for_fifo(4);
    regw(HOST_CNTL,regr(HOST_CNTL) & ~HOST_BYTE_ALIGN);
    regw(PAT_REG0,0);
    regw(PAT_REG1,0);
    regw(PAT_CNTL,0);

    wait_for_fifo(7);
    regw(DP_BKGD_CLR,0);
    regw(DP_FRGD_CLR,1);
    regw(DP_MIX,(MIX_SRC << 16) | MIX_DST);
    regw(DP_SRC,FRGD_SRC_FRGD_CLR);
    regw(CLR_CMP_CLR,0);
    regw(CLR_CMP_MASK,0xffffffff);
    regw(CLR_CMP_CNTL,0);

    // Disable register reading in main aperture
    // Frees up 8k or so and is safer
    // (and enable register block 1)
    wait_for_fifo(3);
    regw(BUS_CNTL,regr(BUS_CNTL) | 0x08000001);
    regw(CUR_HORZ_VERT_OFF,0x00000000);
    regw(CUR_HORZ_VERT_POSN,0x00ff00ff);

    return true;
}

int QMachScreen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    extern bool qws_sw_cursor;

    if(qws_sw_cursor==true) {
	return QScreen::initCursor(e,init);
    }
    qt_screencursor=new QMachCursor();
    SWCursorData *data = (SWCursorData *)e - 1;
    qt_screencursor->init(data,init);
#endif
    return 0;
}

void QMachScreen::shutdownCard()
{
    QScreen::shutdownCard();
}

int ngval(QRgb r)
{
    if(qAlpha(r)<255) {
        return 2;
    } else if(qBlue(r)>240) {
        return 0;
    } else {
        return 1;
    }
}


QGfx * QMachScreen::createGfx(unsigned char * b,int w,int h,int d,int linestep)
{
    QGfx * ret=0;
    if( onCard(b) ) {
	if( d==16 ) {
	    ret = new QGfxMach64<16,0>(b,w,h);
	} else if ( d==32) {
	    ret = new QGfxMach64<32,0>(b,w,h);
	}
	if(ret) {
	    ret->setLineStep(linestep);
	    return ret;
	}
    }
    return QScreen::createGfx(b,w,h,d,linestep);
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen(char * slot,unsigned char * config)
{
    if ( !qt_screen && qws_accel && slot!=0) {
	QMachScreen * ret=new QMachScreen(slot,config);
	if(ret->connect())
	    qt_screen=ret;
    }
    if( !qt_screen ) {
	qt_screen=new QScreen();
	qt_screen->connect();
    }
    return qt_screen;
}

#ifndef QT_NO_QWS_CURSOR

QMachCursor::QMachCursor()
{
}

QMachCursor::~QMachCursor()
{
}

void QMachCursor::init(SWCursorData *,bool)
{
    myoffset=(qt_screen->width()*qt_screen->height()*qt_screen->depth())/8;
    fb_start=qt_screen->base();
}

void QMachCursor::set(const QImage& image,int hx,int hy)
{
    cursor=image;
    hotx=hx;
    hoty=hy;

    if(cursor.isNull()) {
        qDebug("Null cursor image!");
	abort();
        return;
    }

    unsigned int offset=myoffset;
    unsigned char * tmp=fb_start+offset;
    int loopc,loopc2;
    // 3=invert,binary 1==CLR1, 2==nothing(?), 0=CLR0

    // We assume cursors are multiples of 8 pixels wide
    memset(tmp,0xaa,(16*64));
    for(loopc=0;loopc<cursor.height();loopc++) {
        for(loopc2=0;loopc2<(cursor.width()/4);loopc2++) {
            unsigned int v1,v2,v3,v4;
            unsigned int pos=loopc2*4;
            v1=ngval(cursor.pixel(pos,loopc));
            v2=ngval(cursor.pixel(pos+1,loopc));
            v3=ngval(cursor.pixel(pos+2,loopc));
            v4=ngval(cursor.pixel(pos+3,loopc));
            unsigned char put=(v4 << 6) | (v3 << 4) | (v2 << 2) | v1;
            *(tmp++)=put;
        }
        int add=16-(cursor.width()/4);
        tmp+=add;
    }
    wait_for_fifo(3);
    QRgb a=cursor.color(1);
    QRgb b=cursor.color(0);
    unsigned int c,d;
    c=(qRed(a) << 8) | (qGreen(a) << 24) | (qBlue(a) << 16);
    d=(qRed(b) << 8) | (qGreen(b) << 24) | (qBlue(b) << 16);
    regw(CUR_CLR0,c);
    regw(CUR_CLR1,d);
    regw(CUR_OFFSET,offset/8);
}

void QMachCursor::hide()
{
    unsigned int cntlstat=regr(GEN_TEST_CNTL);
    cntlstat=cntlstat & 0xffffff7f;
    wait_for_fifo(1);
    regw(GEN_TEST_CNTL,cntlstat);
}

void QMachCursor::show()
{
    unsigned int cntlstat=regr(GEN_TEST_CNTL);
    cntlstat=cntlstat | 0x80;
    wait_for_fifo(1);
    regw(GEN_TEST_CNTL,cntlstat);
}

void QMachCursor::move(int x,int y)
{
    x-=hotx;
    y-=hoty;
    unsigned int hold=x | (y << 16);
    regw(CUR_HORZ_VERT_POSN,hold);
}

#endif // QT_NO_QWS_CURSOR

