/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgfxMatrox_qws.cpp#2 $
**
** Implementation of QGfxMatrox (graphics context) class for Matrox MGA cards
**
** Created : 000503
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qgfxmatroxdefs_qws.h"

#ifndef QT_NO_QWS_MATROX

#include "qgfxlinuxfb_qws.h"

#include "qimage.h"

static unsigned char *matrox_regbase=0;

//#define DEBUG_INIT

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

inline unsigned int matrox_regr(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(matrox_regbase+regindex));
    return val;
}

inline void matrox_regw(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(matrox_regbase+regindex))=val;
}

// Older Matrox hardware has no hardware cursor; leave it for now

template <const int depth, const int type>
class QGfxMatrox : public QGfxRaster<depth,type> {

public:

    QGfxMatrox(unsigned char *,int w,int h);

    virtual void drawRect(int,int,int,int);
    virtual void blt(int,int,int,int);
    virtual void scroll(int,int,int,int,int,int);
    virtual void sync();

private:

    // Convert colour into what the hardware needs
    unsigned int get_color(unsigned int);

    bool checkSourceDest();
    bool checkDest();
    void do_scissors(QRect &);

    unsigned int src_pixel_offset;

};

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::do_scissors(QRect & r)
{
    matrox_regw(CXLEFT,r.left());
    matrox_regw(CXRIGHT,r.right());
    int t=linestep();
    t=(t*8)/depth;
    matrox_regw(YTOP,r.top()*t);
    matrox_regw(YBOT,r.bottom()*t);
}

template<const int depth,const int type>
inline unsigned int QGfxMatrox<depth,type>::get_color(unsigned int i)
{
    unsigned int ret;
    if(depth==32) {
	ret=i;
    } else if(depth==16) {
	i=i & 0xffff;
	ret=(i << 16) | i;
    } else {
	// 8
	i=i & 0xff;
	ret=(i << 24) | (i << 16) | (i << 8) | i;
    }
    return ret;
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::sync()
{
    int loopc;
    for(loopc=0;loopc<1000000;loopc++) {
        unsigned int stat=matrox_regr(STATUS);
	if((!(stat & 0x10000)) && loopc>20) {
	    return;
	}
    }
    qDebug("Idle timeout!");
}

template<const int depth,const int type>
inline bool QGfxMatrox<depth,type>::checkDest()
{
    ulong buffer_offset;
    if (!qt_screen->onCard(buffer,buffer_offset)) {
	return FALSE;
    }

    int d;
    if(depth==16) {
	d=0x1;
    } else if(depth==32) {
	d=0x2;
    } else {
	// 8bpp
	d=0;
    }

    int t=linestep();
    t=(t*8)/depth;
    if(t & 0x1f) {
	qDebug("Unaligned pixel linestep %d",t);
	return false;
    }
    unsigned int b=(buffer_offset >> 6) << 6;
    b=(b*8)/depth;
    matrox_regw(PITCH,0x8000 | t);
    matrox_regw(YDSTORG,b);
    matrox_regw(MACCESS,d);
    matrox_regw(CXLEFT,0);
    matrox_regw(CXRIGHT,width);
    matrox_regw(YTOP,(0*t)+b);
    matrox_regw(YBOT,(height*t)+b);
    matrox_regw(PLNWT,0xffffffff);

    return TRUE;
}

template<const int depth,const int type>
inline bool QGfxMatrox<depth,type>::checkSourceDest()
{
    if ( !checkDest() ) {
	return FALSE;
    }

    ulong src_buffer_offset;
    if (srctype == SourcePen) {
	src_buffer_offset = -1;
    } else {
	if (!qt_screen->onCard(srcbits,src_buffer_offset)) {
	    return FALSE;
	}
	if(src_buffer_offset & 0x7) {
	    qDebug("Unaligned offset %lx",src_buffer_offset);
	    return FALSE;
	}
	int srcstep;
	if (srclinestep==0) {
	    srcstep=(width*srcdepth)/8;
	} else {
	    srcstep=srclinestep;;
	}

	src_pixel_offset=(src_buffer_offset * 8)/depth;

    }
    return TRUE;
}

template<const int depth,const int type>
QGfxMatrox<depth,type>::QGfxMatrox(unsigned char * a,int b,int c)
    : QGfxRaster<depth,type>(a,b,c)
{
}

template<const int depth,const int type>
void QGfxMatrox<depth,type>::drawRect(int rx,int ry,int w,int h)
{
    if(ncliprect<1) {
	return;
    }

    if( (cbrush.style()!=NoBrush) && (cbrush.style()!=SolidPattern) ) {
	QGfxRaster::drawRect(rx,ry,w,h);
	return;
    }

    QWSDisplay::grab( TRUE );
    if(!checkDest()) {
	QWSDisplay::ungrab();
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

    (*optype)=1;
    (*lastop)=LASTOP_RECT;

    int loopc;

    int xp=rx+xoffs;
    int yp=ry+yoffs;

    int x2,y2;

    x2=xp+(w-1);
    y2=yp+(h-1);

    int x3,y3,x4,y4;

    QColor tmp=cbrush.color();

    checkDest();

    matrox_regw(FCOL,get_color(tmp.alloc()));

    (*optype)=1;

    // Last in 1d00-1dff range

    if(cbrush.style()!=NoBrush) {
	int p=ncliprect;
	for(loopc=0;loopc<p;loopc++) {
	    QRect r=cliprect[loopc];
	    if(xp<=r.right() && yp<=r.bottom() &&
	       x2>=r.left() && y2>=r.top()) {
		x3=r.left() > xp ? r.left() : xp;
		y3=r.top() > yp ? r.top() : yp;
		x4=r.right() > x2 ? x2 : r.right();
		y4=r.bottom() > y2 ? y2 : r.bottom();
		int hh=(y4-y3)+1;
		int p=y3;
		int t=linestep();
		t=(t*8)/depth;
		//t&=0x1f;
		p*=(t >> 5);
		matrox_regw(DWGCTL,DWG_MODE);
		matrox_regw(FXLEFT,x3-1);
		matrox_regw(FXRIGHT,x4+1);
		matrox_regw(YDST,p);
		matrox_regw(LEN | EXEC,hh);
	    }
	}
    }
    GFX_END
    QWSDisplay::ungrab();
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::blt(int rx,int ry,int w,int h)
{
    if(ncliprect<1)
	return;

    if(depth!=16 && depth!=32 && depth!=8) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    bool canaccel=false;

    if((srcdepth==32 || srcdepth==16 || srcdepth==8) &&
       alphatype==IgnoreAlpha) {
	canaccel=true;
    }

    if(srctype==SourceImage && canaccel==false) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if(srctype==SourcePen) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    QWSDisplay::grab( TRUE );

    if(checkSourceDest()) {

	(*optype)=1;
	(*lastop)=LASTOP_BLT;

	int xp=xoffs+rx;
	int yp=yoffs+ry;
	int xp2=srcoffs.x();
	int yp2=srcoffs.y();

	QRect cursRect(xp, yp, w+1, h+1);

	GFX_START(cursRect)

	int loopc;
	for(loopc=0;loopc<ncliprect;loopc++) {

	    do_scissors(cliprect[loopc]);

	    int tw=w;
	    int th=h;
	    int start,end;
	    int typ=yp;

	    int src_pixel_linestep=(srclinestep*8)/srcdepth;

	    if((yp < yp2) || ((yp == yp2) && (xp <= xp2))) {
		matrox_regw(AR5,src_pixel_linestep);
		matrox_regw(DWGCTL,DWG_BITBLT | DWG_SHIFTZERO | DWG_SGNZERO |
			  DWG_BFCOL | DWG_REPLACE);
		tw--;
		start=yp2*src_pixel_linestep+xp2+src_pixel_offset;
		end=start+tw;
	    } else {
		matrox_regw(SGN,5);
		matrox_regw(AR5,-src_pixel_linestep);
		matrox_regw(DWGCTL,DWG_BITBLT | DWG_SHIFTZERO | DWG_BFCOL |
			  DWG_REPLACE);
		tw--;
		end=(yp2+th-1)*src_pixel_linestep+xp2+src_pixel_offset;
		start=end+tw;
		typ+=th-1;
	    }
	    matrox_regw(AR0,end);
	    matrox_regw(AR3,start);
	    matrox_regw(FXBNDRY,((xp+tw)<<16) | xp);
	    int p=typ;
	    int t=linestep();
	    t=(t*8)/depth;
	    //t&=0x1f;
	    p*=(t >> 5);
	    matrox_regw(YDST,p);
	    matrox_regw(LEN | EXEC,th);

	}
	do_scissors(QRect(0,0,width,height));
	QWSDisplay::ungrab();

	GFX_END

	return;
    } else {
	QWSDisplay::ungrab();
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
    }
}

template <const int depth, const int type>
void QGfxMatrox<depth,type>::scroll( int rx,int ry,int w,int h,int sx, int sy )
{
    if (!w || !h)
	return;

    int dy = sy - ry;
    int dx = sx - rx;

    if (dx == 0 && dy == 0)
	return;

    GFX_START(QRect(QMIN(rx+xoffs,sx+xoffs), QMIN(ry+yoffs,sy+yoffs), w+QABS(dx)+1, h+QABS(dy)+1))

    srcbits=buffer;

    srclinestep=linestep();
    srcdepth=depth;
    if(srcdepth==0)
	abort();
    srctype=SourceImage;
    alphatype=IgnoreAlpha;
    ismasking=FALSE;
    setSourceOffset(sx,sy);
    blt(rx,ry,w,h);

    GFX_END
}

class QMatroxScreen : public QLinuxFbScreen {

public:

    QMatroxScreen( int display_id );
    virtual ~QMatroxScreen();
    virtual bool connect( const QString &spec, char *,unsigned char *);
    virtual bool initCard();
    virtual void shutdownCard();
    virtual bool useOffscreen() { return false; }

    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

protected:

    virtual int pixmapOffsetAlignment() { return 256; }
    virtual int pixmapLinestepAlignment() { return 256; }

private:

    unsigned int src_pixel_offset;

};

QMatroxScreen::QMatroxScreen( int display_id  )
    : QLinuxFbScreen( display_id )
{
}

bool QMatroxScreen::connect( const QString &spec, char *,unsigned char *config )
{
    if (!QLinuxFbScreen::connect( spec )) {
	qDebug("Matrox driver couldn't connect to framebuffer");
	return FALSE;
    }

    canaccel=false;

    unsigned short int * manufacturer=(unsigned short int *)config;
    if(*manufacturer!=0x102b) {
	qDebug("This does not appear to be a Matrox card");
	qDebug("Are you sure QWS_CARD_SLOT is pointing to the right entry in "
	       "/proc/bus/pci?");
	return FALSE;
    }

    unsigned char * bar=config+0x14;
    unsigned long int * addr=(unsigned long int *)bar;
    unsigned long int s=*(addr+0);
    unsigned long int olds=s;
    if(s & 0x1) {
#ifdef DEBUG_INIT
	printf("IO space - not right\n");
#endif
	return FALSE;
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
	    return FALSE;
	}
	s=(s >> 4) << 4;
#ifdef DEBUG_INIT
	qDebug("Using physical address %lx, mapping %d",s,0x3fff);
#endif
	membase=(unsigned char *)mmap(0,0x3fff,PROT_READ |
				      PROT_WRITE,MAP_SHARED,
				      aperturefd,s);
	if(membase==0 || membase==(unsigned char *)-1) {
#ifdef DEBUG_INIT
	    qDebug("Failure to mmap /dev/mem, offset %d, %s",s,
		   strerror(errno));
#endif
	    close(aperturefd);
	    return FALSE;
	}
	matrox_regbase=membase;
    }

    qDebug("Detected Matrox card");

    canaccel=true;

    return TRUE;
}


QMatroxScreen::~QMatroxScreen()
{
}

bool QMatroxScreen::initCard()
{
    QLinuxFbScreen::initCard();
    return true;
}

void QMatroxScreen::shutdownCard()
{
    QLinuxFbScreen::shutdownCard();
}

QGfx * QMatroxScreen::createGfx(unsigned char * b,int w,int h,int d,
				int linestep)
{
    QGfx * ret=0;
    if( onCard(b) ) {
	if( d==16 ) {
	    ret = new QGfxMatrox<16,0>(b,w,h);
	} else if ( d==32 ) {
	    ret = new QGfxMatrox<32,0>(b,w,h);
	} else if ( d==8 ) {
	    ret = new QGfxMatrox<8,0>(b,w,h);
	}
	if(ret) {
	    ret->setLineStep(linestep);
	    return ret;
	}
    }
    return QLinuxFbScreen::createGfx(b,w,h,d,linestep);
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen_matrox( int display_id, const char *spec,
					   char *slot,unsigned char *config )
{
    if ( !qt_screen && qws_accel && slot!=0 ) {
	QMatroxScreen * ret=new QMatroxScreen( display_id );
	if(ret->connect( spec, slot, config )) {
	    qt_screen=ret;
	}
    }
    if ( !qt_screen ) {
	qt_screen=new QLinuxFbScreen( display_id );
	qt_screen->connect( spec );
    }
    return qt_screen;
}

#endif // QT_NO_QWS_MATROX

