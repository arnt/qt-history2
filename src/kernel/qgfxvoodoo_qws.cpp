/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgfxvoodoo_qws.cpp#2 $
**
** Implementation of QGfxMach64 (graphics context) class for Voodoo 3 cards
**
** Created : 000503
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

#include "qgfxvoodoodefs_qws.h"

#ifndef QT_NO_QWS_VOODOO3

unsigned char * regbase=0;

//#define DEBUG_INIT

#define LASTOP_RECT 0

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

inline void wait_for_fifo(short entries)
{
    int trycount=0;

    while(trycount++) {
	int fifoval=regr(STATUS);
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
    
private:
    
    bool checkDest();
    
};

template<const int depth,const int type>
inline bool QGfxVoodoo<depth,type>::checkDest()
{
    ulong buffer_offset;
    if (!qt_screen->onCard(buffer,buffer_offset)) {
	return FALSE;
    }
    if (depth!=16) {
	return FALSE;
    }
    wait_for_fifo(3);
    regw(DSTBASEADDR,buffer_offset);
    regw(DSTFORMAT,linestep() | (3 << 16));
    regw(SRCFORMAT,3 << 16);
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

    if(cbrush.style()!=NoBrush) {
	int p=rects.size();
	if(p<8 ) {
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
		    int ww=(x4-x3)+1;
		    int hh=(y4-y3)+1;
		    wait_for_fifo(7);
		    regw(COLORFORE,tmp.alloc());
		    regw(COMMANDEXTRA,0x0);
		    regw(COMMAND,0x5 | (0xcc << 24));
		    regw(CLIP0MIN,0);
		    regw(CLIP0MAX,(1024 << 16) | 1280);
		    regw(DSTSIZE,(hh << 16) | ww);
		    regw(LAUNCHAREA,x3 | (y3 << 16));
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
		}
	    }
	}
    }
    GFX_END
}

class QVoodooScreen : public QScreen {

public:

    QVoodooScreen() { qDebug("No slot specified!"); }
    QVoodooScreen(char *,unsigned char *);
    virtual ~QVoodooScreen();
    virtual bool connect();
    virtual bool initCard();
    virtual void shutdownCard();
    
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
};

QVoodooScreen::QVoodooScreen(char * graphics_card_slot,
			     unsigned char * config)
{
}

bool QVoodooScreen::connect()
{
    if (!QScreen::connect())
	return FALSE;

    unsigned char * bar=config+0x10;
    unsigned long int * addr=(unsigned long int *)bar;
    unsigned long int s=*(addr+0);
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
	    return;
	}
	regbase=membase+0x100000;
    }

    return TRUE;
}


QVoodooScreen::~QVoodooScreen()
{
}

bool QVoodooScreen::initCard()
{
    QScreen::initCard();
    return true;
}

void QVoodooScreen::shutdownCard()
{
    QScreen::shutdownCard();
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
    return QScreen::createGfx(b,w,h,d,linestep);
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen(char * slot,unsigned char * config)
{
    if ( !qt_screen && qws_accel && slot!=0 ) {
	QVoodooScreen * ret=new QVoodooScreen(slot,config);
	if(ret->connect())
	    qt_screen=ret;
    }
    if ( !qt_screen )
	qt_screen=new QScreen();
    return qt_screen;
}

#endif // QT_NO_VOODOO3
