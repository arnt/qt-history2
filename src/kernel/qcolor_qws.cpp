/****************************************************************************
** $Id: //depot/qt/fb/src/kernel/qcolor_fb.cpp#2 $
**
** Implementation of QColor class for FB
**
** Created : 991026
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

#include "qcolor.h"
#include "string.h"
#include "qpaintdevice.h"
#include "qapplication.h"
#include "qgfx_qws.h"
#include "qwsdisplay_qws.h"

/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

int QColor::maxColors()
{
    return 256;
}

int QColor::numBitPlanes()
{
    return 32;
}

void QColor::initialize()
{
    if ( color_init )				// already initialized
	return;
    color_init = TRUE;
}

void QColor::cleanup()
{
    if ( !color_init )
	return;
    color_init = FALSE;
}

#if 0
// Return a value for how close a is to b
// Lower is better
static inline int match(QRgb a,QRgb b)
{
    int ret;
     
#ifndef QT_NO_QWS_DEPTH_8
    int h1,s1,v1;
    int h2,s2,v2;
    /*
    QColor tmp1(a);
    QColor tmp2(b);
    tmp1.hsv(&h1,&s1,&v1);
    tmp2.hsv(&h2,&s2,&v2);
    */
    h1=qRed(a);
    s1=qGreen(a);
    v1=qBlue(a);
    h2=qRed(b);
    s2=qGreen(b);
    v2=qBlue(b);
    ret=abs(h1-h2);
    ret+=abs(s1-s2);
    ret+=abs(v1-v2);
#else
    ret=abs(qGray(a)-qGray(b));
#endif

    return ret;
}

inline unsigned int closestMatch(int r,int g,int b)
{
    QRgb * clut=qt_screen->clut();
    int clutcols=qt_screen->numCols();
    if ( r>255 || g>255 || b>255 || r<0 || g<0 || b<0 )
	abort();

    QRgb tomatch=qRgb(r,g,b);
    int loopc;
    unsigned int hold=0xfffff;
    unsigned int tmp;
    int pos=0;
    for(loopc=0;loopc<clutcols;loopc++) {
	tmp=match(clut[loopc],tomatch);
	if(tmp<hold) {
	    hold=tmp;
	    pos=loopc;
	}
    }
    return pos;
}
#endif


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

uint QColor::alloc()
{
    QWSDisplay * qwsd=qApp->desktop()->qwsDisplay();
    int depth=qwsd->depth();

    int r = qRed(rgbVal);
    int g = qGreen(rgbVal);
    int b = qBlue(rgbVal);
    rgbVal &= RGB_MASK;

    int red_shift,green_shift,blue_shift,red_mask,green_mask,blue_mask;

    if(depth==32) {
	red_shift = 16;
	green_shift = 8;
        blue_shift = 0;
	red_mask   = 0xff0000;
	green_mask = 0x00ff00;
	blue_mask  = 0x0000ff;
    } else if(depth==16) {
	const int gd = qwsd->greenDepth(); //5 for 5-5-5, 6 for 5-6-5
	red_shift   =5+gd-(8-5);
	green_shift =5-(8-gd);
	blue_shift  =0-(8-5);
	if(gd==6) {
	    red_mask    =0xf800;
	    green_mask  =0x07e0;
	} else {
	    red_mask=0x7c00;
	    green_mask=0x03e0;
	}
	blue_mask   =0x001f;
    } else if(depth==4) {
	// #### just a hack
	return pix=(r^g^b)&0xf;
    } else if(depth==8) {
	// #### just a hack
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	return pix=qGray(r,g,b);	
#else
	return pix = (r + 25) / 51 * 36 + (g + 25) / 51 * 6 + (b + 25) / 51;
//	return pix = closestMatch( r, g, b );
#endif
    } else if(depth==1) {
	// #### just a hack
	return pix=qGray(r,g,b) < 128;
    } else {
//	qFatal("QColor::alloc can't cope with depth %d",depth);
	static int count;
	if ( count++ < 10 )
	    qWarning("QColor::alloc can't cope with depth %d",depth);
	return 0;
    }
    r = red_shift	> 0 ? r << red_shift   : r >> -red_shift;
    g = green_shift > 0 ? g << green_shift : g >> -green_shift;
    b = blue_shift	> 0 ? b << blue_shift  : b >> -blue_shift;
    pix = (b & blue_mask) | (g & green_mask) | (r & red_mask);
    return 0xff000000 | pix;
}

extern uint qt_get_rgb_val( const char *name ); // qcolor_p.cpp

void QColor::setSystemNamedColor( const QString& name )
{
    if ( !color_init ) {
#if defined(CHECK_STATE)
	qWarning( "QColor::setSystemNamedColor: Cannot perform this operation "
		 "because QApplication does not exist" );
#endif
	alloc();				// makes the color black
	return;
    }
    rgbVal = qt_get_rgb_val( name.latin1() );
    if ( lazy_alloc ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}

int QColor::enterAllocContext()
{
    return 0;
}


void QColor::leaveAllocContext()
{
}


int QColor::currentAllocContext()
{
    return 0;
}


void QColor::destroyAllocContext( int )
{
}
