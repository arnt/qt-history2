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
#if defined(QWS_DEPTH_8GRAYSCALE)
	return pix=qGray(r,g,b);	
#elif defined(QWS_DEPTH_8DIRECT)
	/*
	red_shift = 5;
	green_shift = 3;
        blue_shift = 0;
	red_mask   = 0xe0;
	green_mask = 0x18;
	blue_mask  = 0x07;
	*/
	return pix=((r >> 5) << 5) | ((g >> 6) << 3) |
		   (b >> 5);
#else
	qFatal("QColor alloc called in paletted mode");
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
