/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgfxvoodoo_qws.cpp#2 $
**
** Implementation of QGfxMach64 (graphics context) class for Voodoo 3 cards
**
** Created : 000503
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
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

#include "qgfxvoodoodefs_qws.h"

#ifndef QT_NO_QWS_VOODOO3

#include "qgfxlinuxfb_qws.h"

static unsigned char *voodoo_regbase=0;

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

inline unsigned int voodoo_regr(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(voodoo_regbase+regindex));
    return val;
}

inline void voodoo_regw(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(voodoo_regbase+regindex))=val;
}

inline void voodoo_wait_for_fifo(short entries)
{
    int trycount=0;

    while(trycount++) {
	int fifoval=voodoo_regr(STATUS);
	fifoval=fifoval & 0x1f;
	if(fifoval>=entries) {
	    return;
	}
	if(trycount>10) {
	    qDebug("Resetting engine");
	}
    }
}

template <const int depth, const int type>
class QGfxVoodoo : public QGfxRaster<depth,type> {

public:

    QGfxVoodoo(unsigned char *,int w,int h);

    virtual void drawRect(int,int,int,int);
    virtual void blt(int,int,int,int);
    virtual void scroll(int,int,int,int,int,int);

private:

    bool checkSourceDest();
    bool checkDest();
    void do_scissors(QRect &);

};

template<const int depth,const int type>
inline void QGfxVoodoo<depth,type>::do_scissors(QRect & r)
{
    voodoo_wait_for_fifo(2);
    voodoo_regw(CLIP0MIN,(r.top()) << 16 | r.left());
    voodoo_regw(CLIP0MAX,(r.bottom()+1) << 16 | (r.right()+1));
}

template<const int depth,const int type>
inline bool QGfxVoodoo<depth,type>::checkSourceDest()
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

	int d;
	if(srcdepth==16) {
	    d=3;
	} else if(srcdepth==32) {
	    d=5;
	} else {
	    d=0;
	}

	voodoo_wait_for_fifo(2);
	voodoo_regw(SRCBASEADDR, src_buffer_offset);
	voodoo_regw(SRCFORMAT,(srcstep | (d << 16)));
    }
    return TRUE;
}

template<const int depth,const int type>
inline bool QGfxVoodoo<depth,type>::checkDest()
{
    ulong buffer_offset;
    if (!qt_screen->onCard(buffer,buffer_offset)) {
	return FALSE;
    }
    if (depth!=16 && depth!=32) {
	return FALSE;
    }

    int d;
    if(depth==16) {
	d=3;
    } else if(depth==32) {
	d=5;
    } else {
	d=0;
    }

    voodoo_wait_for_fifo(2);
    voodoo_regw(DSTBASEADDR,buffer_offset);
    voodoo_regw(DSTFORMAT,linestep() | (d << 16));

    return TRUE;
}

template<const int depth,const int type>
QGfxVoodoo<depth,type>::QGfxVoodoo(unsigned char * a,int b,int c)
    : QGfxRaster<depth,type>(a,b,c)
{
}

template<const int depth,const int type>
void QGfxVoodoo<depth,type>::drawRect(int rx,int ry,int w,int h)
{
    QArray<QRect> rects=setrgn.rects();

    if(rects.size()<1) {
	return;
    }

    if( (cbrush.style()!=NoBrush) && (cbrush.style()!=SolidPattern) ) {
	QGfxRaster::drawRect(rx,ry,w,h);
	return;
    }

    qt_fbdpy->grab();
    if(!checkDest()) {
	qt_fbdpy->ungrab();
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
	lastop=LASTOP_RECT;
	voodoo_wait_for_fifo(1);
	voodoo_regw(SRCFORMAT,3 << 16);
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

    if(cbrush.style()!=NoBrush) {
	int p=rects.size();
	if(p<8 ) {
	    // We can wait for all our fifos at once
	    for(loopc=0;loopc<p;loopc++) {
		QRect r=rects[loopc];
		if(xp<=r.right() && yp<=r.bottom() &&
		   x2>=r.left() && y2>=r.top()) {
		    x3=r.left() > xp ? r.left() : xp;
		    y3=r.top() > yp ? r.top() : yp;
		    x4=r.right() > x2 ? x2 : r.right();
		    y4=r.bottom() > y2 ? y2 : r.bottom();
		    int ww=(x4-x3)+1;
		    int hh=(y4-y3)+1;
		    voodoo_wait_for_fifo(7);
		    voodoo_regw(COLORFORE,tmp.alloc());
		    voodoo_regw(COMMANDEXTRA,0x0);
		    voodoo_regw(COMMAND,0x5 | (0xcc << 24));
		    voodoo_regw(CLIP0MIN,0);
		    voodoo_regw(CLIP0MAX,(1024 << 16) | 1280);
		    voodoo_regw(DSTSIZE,(hh << 16) | ww);
		    voodoo_regw(LAUNCHAREA,x3 | (y3 << 16));
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
		    int ww=(x4-x3)+1;
		    int hh=(y4-y3)+1;
		    voodoo_wait_for_fifo(7);
		    voodoo_regw(COLORFORE,tmp.alloc());
		    voodoo_regw(COMMANDEXTRA,0x0);
		    voodoo_regw(COMMAND,0x5 | (0xcc << 24));
		    voodoo_regw(CLIP0MIN,0);
		    voodoo_regw(CLIP0MAX,(1024 << 16) | 1280);
		    voodoo_regw(DSTSIZE,(hh << 16) | ww);
		    voodoo_regw(LAUNCHAREA,x3 | (y3 << 16));
		}
	    }
	}
    }
    GFX_END
    qt_fbdpy->ungrab();
}

template<const int depth,const int type>
inline void QGfxVoodoo<depth,type>::blt(int rx,int ry,int w,int h)
{
    QArray<QRect> rects=setrgn.rects();
    if(rects.size()<1)
	return;

    if(depth!=16 && depth!=32) {
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
	if(alphatype==IgnoreAlpha) {
	    canaccel=true;
	} else {
	}
    }

    if(srctype==SourceImage && canaccel==false) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if(srctype==SourcePen /*&& !(alphatype==BigEndianMask ||
			    alphatype==LittleEndianMask)*/ ) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if( (srcdepth!=32) && (srcdepth!=16) ) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    if( (alphatype==InlineAlpha || alphatype==SolidAlpha)
	&& checkSourceDest() ) {
	//int x2=(rx+w)-1;
	//int y2=(ry+h)-1;
	//qDebug("Has alpha channel, doing that instead");
	//drawAlpha(rx,ry,x2,ry,rx,y2,x2,y2);
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
	return;
    }

    qt_fbdpy->grab();
    
    if(checkSourceDest()) {

	optype=1;
	lastop=LASTOP_BLT;

	int xp=xoffs+rx;
	int yp=yoffs+ry;
	int xp2=srcoffs.x();
	int yp2=srcoffs.y();

	QRect cursRect(xp, yp, w+1, h+1);

	GFX_START(cursRect)

        unsigned int dirmask=0;

	if(yp>yp2) {
	    // Down, reverse
	    if(xp>xp2) {
		// Right, reverse
		xp+=(w-1);
		xp2+=(w-1);
		yp+=(h-1);
		yp2+=(h-1);
		dirmask|=0x4000 | 0x8000;
	    } else {
		// Left, normal
		yp+=(h-1);
		yp2+=(h-1);
		dirmask|=0x8000;
	    }
	} else {
	    // Up, normal
	    // Down, reverse
	    if(xp>xp2) {
		// Right, reverse
		xp+=(w-1);
		xp2+=(w-1);
		dirmask|=0x4000;
	    } else {
		// Left, normal
	    }
	}

	unsigned int loopc;
	for(loopc=0;loopc<rects.size();loopc++) {

	    do_scissors(rects[loopc]);

	    voodoo_wait_for_fifo(5);
	    /*
	    regw(SRC_WIDTH1,w);
	    regw(DST_Y_X,(xp << 16) | yp);
	    regw(DST_HEIGHT_WIDTH,(w << 16) | h);
	    */
	    voodoo_regw(COMMAND,0x1 | (0xcc << 24) | dirmask);
	    voodoo_regw(SRCSIZE,w | (h << 16));
	    voodoo_regw(DSTSIZE,w | (h << 16));
	    voodoo_regw(DSTXY,xp | (yp << 16));
	    voodoo_regw(LAUNCHAREA,xp2 | (yp2 << 16));
	}
	voodoo_wait_for_fifo(2);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,1024 | 1280);
	GFX_END
	qt_fbdpy->ungrab();
	return;
    } else {
	qt_fbdpy->ungrab();
	QGfxRaster<depth,type>::blt(rx,ry,w,h);
    }
}

template <const int depth, const int type>
void QGfxVoodoo<depth,type>::scroll( int rx,int ry,int w,int h,int sx, int sy )
{
     if (!w || !h)
	return;

    int dy = sy - ry;
    int dx = sx - rx;

    if (dx == 0 && dy == 0)
	return;

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
}

class QVoodooScreen : public QLinuxFbScreen {

public:

    QVoodooScreen( int display_id );
    virtual ~QVoodooScreen();
    virtual bool connect( const QString &spec, char *,unsigned char *);
    virtual bool initCard();
    virtual void shutdownCard();

    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
};

QVoodooScreen::QVoodooScreen( int display_id  )
    : QLinuxFbScreen( display_id )
{
}

bool QVoodooScreen::connect( const QString &spec, char *,unsigned char *config )
{
    if (!QLinuxFbScreen::connect( spec )) {
	return FALSE;
    }

    unsigned char * bar=config+0x10;
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
	qDebug("Using physical address %lx, mapping %d",s,0x5fffff);
#endif
	membase=(unsigned char *)mmap(0,0x5fffff,PROT_READ |
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
	voodoo_regbase=membase+0x100000;
    }

    qDebug("Detected Voodoo 3");

    canaccel=true;

    return TRUE;
}


QVoodooScreen::~QVoodooScreen()
{
}

bool QVoodooScreen::initCard()
{
    QLinuxFbScreen::initCard();
    return true;
}

void QVoodooScreen::shutdownCard()
{
    QLinuxFbScreen::shutdownCard();
}

QGfx * QVoodooScreen::createGfx(unsigned char * b,int w,int h,int d,
				int linestep)
{
    QGfx * ret=0;
    if( onCard(b) ) {
	if( d==16 ) {
	    ret = new QGfxVoodoo<16,0>(b,w,h);
	} else if ( d==32) {
	    ret = new QGfxVoodoo<32,0>(b,w,h);
	}
	if(ret) {
	    ret->setLineStep(linestep);
	    return ret;
	}
    }
    return QLinuxFbScreen::createGfx(b,w,h,d,linestep);
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen_voodoo3( int display_id, const char *spec,
					   char *slot,unsigned char *config )
{
    if ( !qt_screen && qws_accel && slot!=0 ) {
	QVoodooScreen * ret=new QVoodooScreen( display_id );
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

#endif // QT_NO_VOODOO3
