/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
**
** Definition of ________ class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <math.h>
#include "qgfxps2_qws.h"
#include <gsos.h>
#include <stdlib.h>
#include <stdio.h>
#include <qpointarray.h>
#include <qpaintdevice.h>
#include <qpaintdevicemetrics.h>
#include <qimage.h>

static int crap_op=0;

/* screen */
QPS2Screen::QPS2Screen( int display_id ) : QScreen(display_id)
{
}

QPS2Screen::~QPS2Screen()
{
    
}

bool
QPS2Screen::initCard()
{
    gsosSave();

    /* stolen more-or-less verabim from the gsx code base */
    static int framerate[4][3]= {
	/* 60, 75, 85 Hz */
	{   1,  3,  4},	/*  640x 480 */
	{   2,  4,  5}, /*  800x 600 */
	{   1,  3,  4}, /* 1024x 786 */
	{   1,  2,  0}  /* 1280x1024 */
    };

    int output = 0, fr = 0; //default to vesa
    if(const char *o = getenv("QWS_MONITOR")) {
	if(!strcasecmp(o, "VESA"))
	    output = 0;
	else if(!strcasecmp(o, "NTSC"))
	    output = 2;
	else if(!strcasecmp(o, "PAL"))
	    output = 3;
    }
    if(output == 0) {
	int res, frIndex;
	switch(dw){
	case 800:    // 800x600
	    res = 1;
	    break;
	case 1024:   // 1024x768
	    res = 2;
	    break;
	case 1280:   // 1280x1024
	    res = 3;
	    break;
	case 640:    // 640x480
	default:
	    res = 0;
	    break;
	}

	switch(75) { /* I'm just taking this code incase this becomes choosable later.. */
	case 60:
	    frIndex = 0;
	    break;
	case 85:
	    frIndex = 2;
	    break;
	case 75:
	default:
	    frIndex = 1;
	    break;
	}
	fr = ((framerate[res][frIndex])<<8) | res;
    } else {
	fr = 1;
    }
    gsosSetScreen(output, fr, dw, dh, 0x00, 0, 0, 0, 0);

    return TRUE;
}

bool
QPS2Screen::connect( const QString & )
{
    if(gsosOpen() != 0)
	return FALSE;

    if(const char *qwssize=getenv("QWS_SIZE")) {
	sscanf(qwssize,"%dx%d",&w,&h);
	dw=w;
	dh=h;
    } else {
	if(gsosGetScreenSize(&w, &h)) {
	    dw = w;
	    dh = h;
	} else { //fallback to some hardcoded values
	    dw=w=1024;
	    dh=h=768;
	}
    }
    d=32;

    mapsize=size=0;
//    screenclut = 0;
    screencols = 0;

    // No blankin' screen, no blinkin' cursor!
    const char termctl[] = "\033[9;0]\033[?33l";
    write(1,termctl,sizeof(termctl));

    return TRUE;
}

void
QPS2Screen::disconnect()
{
    // a reasonable screensaver timeout
    gsosClose();
    printf( "\033[9;15]" );
    fflush( stdout );
}

int QPS2Screen::initCursor(void*, bool)
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor=new QPS2Cursor();
    qt_screencursor->init(NULL,true);
#endif
    return 0;
}

void
QPS2Screen::shutdownCard()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
    gsosRestore();
}

extern "C" QScreen * qt_get_screen_ps2( int display_id, const char *spec, char *,unsigned char *)
{
    if ( !qt_screen ) {
	QPS2Screen * ret=new QPS2Screen( display_id );
	if(ret->connect( spec )) {
	    qt_screen=ret;
	}
    }
    return qt_screen;
}


QGfx * QPS2Screen::createGfx(unsigned char *data,int w,int h,int d, int linestep)
{
    /* this lets off screen painting happen, however it is very wrong,
       since some objects won't have a frame buffer (so called "data"), I
       either need to a) figure out how to get some texture space working
       or b) get an fb, either way will work, a is probably better.
    */
    QGfx *ret;
    if(data)
	ret = QScreen::createGfx(data, w, h, d, linestep);
    else
	ret = new QGfxPS2(w,h, linestep);
    ret->setLineStep(linestep);
    return ret;
}

QGfxPS2::QGfxPS2(int w,int h, int l) : QGfxRasterBase(NULL, w, h)
{
    lastop = optype = &crap_op; //son of a...
    lstep = l;
    flushRegisters();
}

QGfxPS2::~QGfxPS2()
{
}

void 
QGfxPS2::flushRegisters(bool flushtex) 
{
    /* flush */
    gsosMakeGiftag( flushtex ? 22 : 21, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE,
		    0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);
    gsosSetPacketAddrData(GSOS_XYOFFSET_1, GsosXyoffsetData(GSOS_XYOFFSET<<4, GSOS_XYOFFSET<<4));
//    gsosSetPacketAddrData(GSOS_FRAME_1, GsosFrameData(0x00, (qt_screen->deviceWidth() +63)/64, 0, 0xff000000));
    gsosSetPacketAddrData(0x100, GsosZbufData(0x100, 0, 0));
    gsosSetPacketAddrData(GSOS_PRMODECONT, GsosPrmodecontData( 0));
    gsosSetPacketAddrData(GSOS_TEX0_1, GsosTex0Data(0x3000, 1, 0, 10, 10, 1, 0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_SCANMSK, GsosScanmskData(0));
    gsosSetPacketAddrData(GSOS_CLAMP_1, GsosClampData(0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_TEX1_1, GsosTex1Data(0, 0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_TEX2_1, GsosTex2Data(0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_MIPTBP1_1, GsosMiptbp1Data(0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_MIPTBP2_1, GsosMiptbp2Data(0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_SCISSOR_1, GsosScissorData(0, qt_screen->deviceWidth(), 0, qt_screen->deviceHeight()));
    gsosSetPacketAddrData(GSOS_ALPHA_1, GsosAlphaData(0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData(1, 1, 0, 0, 0, 0, 0, 0));
    gsosSetPacketAddrData(GSOS_FBA_1, GsosFbaData(0));
    gsosSetPacketAddrData(GSOS_PRMODE, GsosPrmodeData(0, 0, 0, 0, 0, 1, 0, 0));
    gsosSetPacketAddrData(GSOS_TEXCLUT, GsosTexclutData(0, 0, 0));
    gsosSetPacketAddrData(GSOS_TEXA, GsosTexaData(0, 0, 0x80));
    gsosSetPacketAddrData(GSOS_DIMX, GsosDimxData(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
    gsosSetPacketAddrData(GSOS_DTHE, GsosDtheData(0));
    gsosSetPacketAddrData(GSOS_COLCLAMP, GsosColclampData(0));
    gsosSetPacketAddrData(GSOS_PABE, GsosPabeData(0));
    if(flushtex)
	gsosSetPacketAddrData( 0x3f, 0); //why don't they do this??!?
    gsosExec() ;
}

void
QGfxPS2::drawPoint( int x ,int y)
{
    if(cpen.style()==NoPen)
	return;

    if(!ncliprect)
	return;

    usePen();
    QRgb rgb = cpen.color().rgb();

    GFX_START(QRect(x,y,2,2));
    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag( 4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData( GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ) ) ;
	gsosSetPacketAddrData4( GSOS_SCISSOR_1, 
				(GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				(GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 0, 0, 0 )) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_POINT ) ;

	gsosMakeGiftag( 1, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_RGBAQ);
	gsosSetPacket4( qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb) );
	gsosSetPacket4( GSOS_SUBPIX_OFST(x+xoffs), GSOS_SUBPIX_OFST(y+yoffs), 0, 0 );

    }
    gsosExec() ;
    GFX_END;
}


void
QGfxPS2::drawPoints( const QPointArray &pa,int index,int npoints)
{
    if(cpen.style()==NoPen)
	return;

    if(!ncliprect)
	return;

    usePen();
    QRgb rgb = cpen.color().rgb();

    GFX_START(clipbounds);

    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag( 4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData( GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ) ) ;
	gsosSetPacketAddrData4( GSOS_SCISSOR_1, 
				(GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				(GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 0, 0, 0 )) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_POINT ) ;

	int r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)), a(qAlpha(rgb));
	gsosMakeGiftag( npoints, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_RGBAQ);
	while (npoints--) {
	    gsosSetPacket4( r, g, b, a);
	    gsosSetPacket4( GSOS_SUBPIX_OFST(pa[index].x()+xoffs), GSOS_SUBPIX_OFST(pa[index].y()+yoffs), 0, 0 );
	    ++index;
	}

    }
    gsosExec() ;
    GFX_END;
}


void
QGfxPS2::drawLine( int x1,int y1, int x2, int y2)
{
    if(cpen.style()==NoPen)
	return;

    if(!ncliprect)
	return;

    if (cpen.width() > 1) {
	QPointArray pa(5);
	int w = cpen.width() - 1;
	double a = atan2( y2 - y1, x2 - x1 );
	double ix = cos(a) * w / 2;
	double iy = sin(a) * w / 2;

	// No cap.
	pa[0].setX( x1 + iy );
	pa[0].setY( y1 - ix );
	pa[1].setX( x2 + iy );
	pa[1].setY( y2 - ix );
	pa[2].setX( x2 - iy );
	pa[2].setY( y2 + ix );
	pa[3].setX( x1 - iy );
	pa[3].setY( y1 + ix );


	pa[4] = pa[0];

	if((*optype))
	    sync();
	(*optype)=0;
	usePen();

	GFX_START(clipbounds);
	scan(pa, FALSE, 0, 5);
	QPen savePen = cpen;
	cpen = QPen( cpen.color() );
	drawPolyline(pa, 0, 5);
	cpen = savePen;
	GFX_END;
	return;
    }

    x1+=xoffs;
    y1+=yoffs;
    x2+=xoffs;
    y2+=yoffs;

    if(x1>x2) {
	int x3;
	int y3;
	x3=x2;
	y3=y2;
	x2=x1;
	y2=y1;
	x1=x3;
	y1=y3;
    }

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, (x2-x1)+1, QABS((y2-y1))+1));
    usePen();
    QRgb rgb = cpen.color().rgb();

    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag( 4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData( GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ) ) ;
	gsosSetPacketAddrData4( GSOS_SCISSOR_1, 
				(GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				(GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 1, 0, 0 ) ) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_LINE ) ;

	gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	gsosSetPacket4( qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb) );
	gsosSetPacket4( GSOS_SUBPIX_OFST(x1), GSOS_SUBPIX_OFST(y1), 0, 0 ) ;
	gsosSetPacket4( qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb) );
	gsosSetPacket4( GSOS_SUBPIX_OFST(x2), GSOS_SUBPIX_OFST(y2), 0, 0 ) ;

    }
    gsosExec();
    GFX_END;    
}


void
QGfxPS2::fillRect(int x,int y, int w, int h)
{
    setAlphaType(IgnoreAlpha);
    if ( w <= 0 || h <= 0 ) return;

    if(!ncliprect)
	return;

    x += xoffs;
    y += yoffs;

    GFX_START(QRect(x, y, w+1, h+1));
    useBrush();
    QRgb rgb = cbrush.color().rgb();

    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag( 4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ));
	gsosSetPacketAddrData4( GSOS_SCISSOR_1, 
				(GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				(GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 0, 0, 0 ) ) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_SPRITE ) ;

	gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	gsosSetPacket4( qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb) );
	gsosSetPacket4(GSOS_SUBPIX_OFST(x), GSOS_SUBPIX_OFST(y),0,0);
	gsosSetPacket4( qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb) );
	gsosSetPacket4(GSOS_SUBPIX_OFST((x+w)), GSOS_SUBPIX_OFST((y+h)),0,0);

    }
    gsosExec() ;
    GFX_END;    
}


void
QGfxPS2::drawPolyline( const QPointArray &pa, int index, int npoints)
{
    if(!ncliprect)
	return;

    GFX_START(clipbounds);

    usePen();
    QRgb rgb = cpen.color().rgb();
    int r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)), a(qAlpha(rgb));

    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag( 4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData( GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ) ) ;
	gsosSetPacketAddrData4( GSOS_SCISSOR_1, 
				(GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				(GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 1, 0, 0 ) ) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_LSTRIP ) ;

	int end = (index+npoints) > (int)pa.size() ? pa.size() : index+npoints;
	gsosMakeGiftag( (end - index-1), GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			GSOS_GIF_FLG_PACKED, 2,(GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	for(int x=index+1; x < end; x++) {
	    gsosSetPacket4( r, g, b, a ) ;
	    gsosSetPacket4( GSOS_SUBPIX_OFST(pa[x].x() + xoffs), 
			    GSOS_SUBPIX_OFST(pa[x].y() + yoffs), 0, 0 ) ;
	}

    }
    gsosExec();
    GFX_END;    
}


void
QGfxPS2::drawPolygon( const QPointArray &pa, bool winding, int index, int npoints)
{
    if(!ncliprect)
	return;

    GFX_START(clipbounds);

    if ( cbrush.style()!=QBrush::NoBrush )
	scan(pa,winding,index,npoints);

    drawPolyline(pa, index, npoints);
    if (pa[index] != pa[index+npoints-1]) {
	usePen();
	QRgb rgb = cpen.color().rgb();
	int r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)), a(qAlpha(rgb));

	for(int clp = 0; clp < ncliprect; clp++) {

	    gsosMakeGiftag( 4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			    0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	    gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ));
	    gsosSetPacketAddrData4( GSOS_SCISSOR_1, 
				    (GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				    (GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	    gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 0, 0, 0 ) ) ;
	    gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_LINE ) ;

	    gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			    GSOS_GIF_FLG_PACKED, 2,
			    (GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));

	    gsosSetPacket4( r, g, b, a ) ;
	    gsosSetPacket4(GSOS_SUBPIX_OFST(pa[index].x() + xoffs), GSOS_SUBPIX_OFST(pa[index].y()+yoffs),0,0);
	    gsosSetPacket4( r, g, b, a ) ;
	    gsosSetPacket4(GSOS_SUBPIX_OFST(pa[index+npoints-1].x() + xoffs), 
			   GSOS_SUBPIX_OFST(pa[index+npoints-1].y() + yoffs),0,0);

	}
	gsosExec() ;
    }
    GFX_END;
}

void QGfxPS2::processSpans( int n, QPoint* point, int* width )
{
    int usable = 0;
    for(int x = 0; x < n; x++)
	if(*(width+x)) usable++;
    if(usable) {
	useBrush();
	QRgb rgb = cbrush.color().rgb();
	int r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)), a(qAlpha(rgb));

	gsosMakeGiftag( 3, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ));
	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 0, 0, 0 ) ) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_LINE ) ;

	gsosMakeGiftag( usable * 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
		    GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	while(n--) {
	    if(*width) {
		gsosSetPacket4( r, g, b, a );
		gsosSetPacket4(GSOS_SUBPIX_OFST(point->x()+xoffs), GSOS_SUBPIX_OFST(point->y()+yoffs),0,0);
		gsosSetPacket4( r, g, b, a ) ;
		gsosSetPacket4(GSOS_SUBPIX_OFST(point->x()+((*width))+xoffs), GSOS_SUBPIX_OFST(point->y()+yoffs),0,0);
	    }
	    point++;
	    width++;
	}

	gsosExec() ;
    }
}

/* 
   I need to come back and rewrite this properly
*/
bool
QGfxPS2::mapSourceToTexture(int x, int y, int w, int h)
{
    tex_width = w;
    tex_height = h;
    int aligned_width = ((tex_width+63)/64) *64; //multiple of 64

    uchar *bits = NULL;
    if(srctype == SourceImage) {
	switch(srcdepth)
	{
	default:
	    break;
	case 8:
	    tex_psm = 0;
	    bits = (uchar *)malloc(aligned_width*tex_height*4);
	    for (int i=0; i<tex_height; i++ ) {
		register uint *p = (uint *)(bits + ((aligned_width * i) * 4));
		uchar  *b = (srcbits + (srcwidth * (y + i)));
		uint *end = p + (width * 4);
		while ( p < end )
		    *p++ = srcclut[(*b++)];
	    }
	    break;
	case 16:
	    tex_psm = 2;
	    bits = (uchar *)malloc(aligned_width*tex_height*2);
	case 32:
	    if(!bits) {
		bits = (uchar *)malloc(aligned_width*tex_height*4);
		tex_psm = 0;
	    }

	    int colsize = srcdepth == 16 ? 2 : 4;
	    for(int i = 0; i < tex_height; i++)
		memcpy(bits + ((aligned_width * i)*colsize),
		       srcbits + (((srcwidth * (y + i)) + x)*colsize), tex_width * colsize);
	}
    } 

    if(!bits) { //woops
	tex_psm = 0;
        bits = (uchar *)malloc(aligned_width * tex_height * 4);
    }

    /* take care of the alpha */
    if(tex_psm == 0) {
	if(alphatype != IgnoreAlpha) {
	    unsigned int rgb;
	    if(srctype == SourcePen) {
		usePen();
		rgb = (0x00FFFFFF) & cpen.color().rgb();
	    }

	    unsigned int *out = (unsigned int *)bits;
	    for(int y = 0; y < tex_height; y++) {
		int dy = (y * aligned_width);
		int sy = (y * alphalinestep);
		for(int x = 0; x < tex_width; x++) {
		    if(srctype == SourceImage)
			rgb = (0x00FFFFFF) & out[dy+x];
		    if(alphatype ==  LittleEndianMask || alphatype == BigEndianMask) {
			char a = alphabits[sy + (x / 8)];
			if(alphatype == LittleEndianMask) 
			    a = a >> (x % 8);
			else
			    a = a >> (7 - (x % 8));
			out[dy+x] = ((a & 0x01) ? 0xFF000000 : 0) | rgb;
		    }	
		    else if(alphatype == SeparateAlpha)
			out[dy+x] = ((alphabits[sy+x]) << 24) | rgb;
		    else
			out[dy+x] = (qAlpha(rgb) / 2) << 24 | rgb;
		}
	    }
	}
    }

    /* throw it in the texture */
    flushRegisters(TRUE);
    gsosWriteImage(0, 0, aligned_width, tex_height, 0x3000, aligned_width / 64, tex_psm, (uchar *)bits);
    free(bits);
    return TRUE;
}

bool
QGfxPS2::bltTexture(int x, int y, int clp, int w, int h)
{
    flushRegisters();

    if(w == -1) 
	w = tex_width;
    if(h == -1)
	h = tex_height;

    bool do_alpha = FALSE, do_alpha_blend = FALSE;
    switch(alphatype) {
    case InlineAlpha:
    case SeparateAlpha:
	do_alpha = TRUE;
	do_alpha_blend = TRUE;
	break;
    case BigEndianMask:
    case LittleEndianMask:
	do_alpha = TRUE;
	break;
    default:
	break;
    }

    gsosMakeGiftag( 6, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
		    0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );

    gsosSetPacketAddrData(GSOS_ALPHA_1, GsosAlphaData(0, 1, 0, 1, 0));
    gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( do_alpha, 7, 0, 0, 0, 0, 0, 0 ));

    gsosSetPacketAddrData( GSOS_TEX0_1, GsosTex0Data(0x3000, (tex_width+63)/64, tex_psm, 10, 10,
						     1, 1, 0, 0, 0, 0, 0 ));
    QRect clip;
    if(clp == -1)
	clip = clipbounds;
    else
	clip = cliprect[clp];
    gsosSetPacketAddrData4( GSOS_SCISSOR_1,(GSOSbit64)clip.topLeft().x(), (GSOSbit64)clip.bottomRight().x(),
			    (GSOSbit64)clip.topLeft().y(), (GSOSbit64)clip.bottomRight().y() ) ;
    gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 1, 0, do_alpha_blend, 0, 1, 0, 0 ) ) ;
    gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_SPRITE ) ;

    gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 
		    2, (GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_UV);

    gsosSetPacket4(GSOS_SUBPIX_OFST(0), GSOS_SUBPIX_OFST(0),0,0);
    gsosSetPacket4(GSOS_SUBPIX_OFST(x), GSOS_SUBPIX_OFST(y),0,0);

    gsosSetPacket4(GSOS_SUBPIX_OFST((w+1)), GSOS_SUBPIX_OFST(h),0,0);
    gsosSetPacket4(GSOS_SUBPIX_OFST((x+w)), GSOS_SUBPIX_OFST((y+h)+1),0,0);

    gsosExec();

    return TRUE;
}


void
QGfxPS2::blt( int rx, int ry, int w, int h, int sx,int sy)
{
    if(!ncliprect)
	return;

    rx += xoffs;
    ry += yoffs;

    GFX_START(QRect(rx, ry, w+1, h+1));

    if(mapSourceToTexture(sx, sy, w, h)) {
	for(int clp = 0; clp < ncliprect; clp++)
	    bltTexture(rx, ry, clp);
    }

    GFX_END;
}


#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
void
QGfxPS2::stretchBlt( int rx, int ry, int w, int h, int sx, int sy)
{
}
#endif

void
QGfxPS2::tiledBlt( int rx, int ry, int w, int h)
{
    if(!ncliprect)
	return;

    rx += xoffs;
    ry += yoffs;

    GFX_START(QRect(rx, ry, w+1, h+1));

    if(mapSourceToTexture(0, 0, w, h)) {
	for(int clp = 0; clp < ncliprect; clp++)
	    bltTexture(rx, ry, clp, w, h);
    }

    GFX_END;
}

void
QGfxPS2::scroll( int rx, int ry, int w, int h,int sx, int sy)
{
    rx += xoffs;
    ry += yoffs;
    sx += xoffs;
    sy += yoffs;

    GFX_START(QRect(rx, ry, w+1, h+1));

    flushRegisters(TRUE);
    gsosMakeGiftag( 6, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
		    0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
    gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 0, 4, 0xFF, 0, 0, 0, 0, 0 ));
    gsosSetPacketAddrData4( GSOS_SCISSOR_1,  /* this has to be wrong!?! */
			    (GSOSbit64)clipbounds.topLeft().x(), (GSOSbit64)clipbounds.bottomRight().x(),
			    (GSOSbit64)clipbounds.topLeft().y(), (GSOSbit64)clipbounds.bottomRight().y() ) ;
    gsosSetPacketAddrData(GSOS_BITBLTBUF, GsosBitbltbufData(0x00, (qt_screen->width() + 63) / 64, 
							    0, 0x00, (qt_screen->width() + 63) / 64, 0));
    gsosSetPacketAddrData(GSOS_TRXREG, GsosTrxregData(w, h));

    int dir = 0;
    if(sy > ry)
	dir = (sx < rx) ? 2 : 0;
    else 
	dir = (sx < rx) ? 3 : 1;
    gsosSetPacketAddrData(GSOS_TRXPOS, GsosTrxposData(sx, sy, rx, ry, dir));
    gsosSetPacketAddrData(GSOS_TRXDIR, GsosTrxdirData(2));
    gsosExec() ;
    gsosFlush();

    GFX_END;    
}

void QGfxPS2::setSource(const QPaintDevice * p)
{
    QPaintDeviceMetrics qpdm(p);
    srcdepth=qpdm.depth();
    srcbits=((QPaintDevice *)p)->scanLine(0);
    srctype=SourceImage;
    setAlphaType(IgnoreAlpha);
    if ( p->devType() == QInternal::Widget ) {
	QWidget * w=(QWidget *)p;
	srcwidth=w->width();
	srcheight=w->height();
	QPoint hold;
	hold=w->mapToGlobal(hold);
	setSourceWidgetOffset( hold.x(), hold.y() );
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=TRUE;
	}
    } else if ( p->devType() == QInternal::Pixmap ) {
	//still a bit ugly
	QPixmap *pix = (QPixmap*)p;
	setSourceWidgetOffset( 0, 0 );
	srcwidth=pix->width();
	srcheight=pix->height();
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=TRUE;
	}
    } else {
	// This is a bit ugly #### I'll say!
	//### We will have to find another way to do this
	setSourceWidgetOffset( 0, 0 );
	buildSourceClut(0,0);
    }
}

void QGfxPS2::setSource(const QImage * i)
{
    srctype=SourceImage;
    srclinestep=i->bytesPerLine();
    srcdepth=i->depth();
    srcbits=i->scanLine(0);
    src_little_endian=(i->bitOrder()==QImage::LittleEndian);
    setSourceWidgetOffset( 0, 0 );
    QSize s = qt_screen->mapToDevice( QSize(i->width(), i->height()) );
    srcwidth=s.width();
    srcheight=s.height();
    src_normal_palette=FALSE;
    if ( srcdepth == 1 )
	buildSourceClut( 0, 0 );
    else  if(srcdepth<=8)
	buildSourceClut(i->colorTable(),i->numColors());
}

void 
QGfxPS2::buildSourceClut(QRgb * cols,int numcols)
{
    if (!cols) {
	useBrush();
	srcclut[0]=pixel;
	transclut[0]=pixel;
	usePen();
	srcclut[1]=pixel;
	transclut[1]=pixel;
	return;
    }

    // Copy clut
    for(int loopc=0;loopc<numcols;loopc++)
	srcclut[loopc] = cols[loopc];
}

#ifndef QT_NO_QWS_CURSOR
struct QPS2CursorData
{
    SWCursorData d;

    GSOSbit64 cursortex0; /* not used yet.. */
    uchar cursordata[(64*64)*4];

    GSOSbit64 grabtex0; /* not used yet.. */
    uchar grabdata[(64*64)*4];
};

QPS2Cursor::QPS2Cursor() : QScreenCursor()
{
    mydata = new QPS2CursorData;
    data = &mydata->d;
}

QPS2Cursor::~QPS2Cursor()
{
    delete mydata;
}

void QPS2Cursor::init(SWCursorData *, bool)
{
    // initialise our gfx
    gfx = (QGfxRasterBase*)qt_screen->screenGfx();
    clipWidth = gfx->pixelWidth();
    clipHeight = gfx->pixelHeight();
    gfx->setClipRect( 0, 0, clipWidth, clipHeight);

    save_under = FALSE;
    fb_end = fb_start = 0; //bleh

    mydata->d.hotx = mydata->d.hoty = 0;
    mydata->d.width = mydata->d.height = 0;

    mydata->d.bound = QRect(); //null at first!
    mydata->d.x = gfx->pixelWidth()/2;
    mydata->d.y = gfx->pixelHeight()/2;
    gsosReadImage(mydata->d.x-5, mydata->d.x-5, 
		  64, 64, 0x00, (qt_screen->width()+63)/64, 0, mydata->grabdata);
    mydata->d.enable = TRUE;
}

void QPS2Cursor::set(const QImage& image,int hx,int hy)
{
    QWSDisplay::grab( TRUE );
    
    bool save = restoreUnder(mydata->d.bound);
    mydata->d.hotx = hx;
    mydata->d.hoty = hy;
    mydata->d.width = image.width();
    mydata->d.height = image.height();

    for (int i=0; i<mydata->d.height; i++ ) {
	register uint *p = (uint *)(mydata->cursordata + ((64 * i) * 4));
	uchar  *b = image.scanLine(i);
	for(uint *end = p + (mydata->d.width * 4); p < end; b++, p++) {
	    if(*b >= image.numColors())
		*p = qRgba(128,255,255,0);
	    else {
		QRgb rgb = image.color(*b);
		*p = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb) / 2);
	    }
	}
    }

    mydata->d.bound = QRect( mydata->d.x - mydata->d.hotx, mydata->d.y - mydata->d.hoty,
			     mydata->d.width, mydata->d.height);
    if (save) saveUnder();
    QWSDisplay::ungrab();
}

bool QPS2Cursor::restoreUnder( const QRect &r, QGfxRasterBase *g )
{
    if (!mydata->d.enable)
	return FALSE;

    if (r.isNull() || !r.intersects(mydata->d.bound))
	return FALSE;

    if ( g && !g->isScreenGfx() )
	return FALSE;

    if (!save_under) {
	QGfxPS2::flushRegisters(TRUE);
	QWSDisplay::grab(TRUE);

	gsosWriteImage( 0, 0, 64, 64, 0x3000, 1, 0, (uchar *)mydata->grabdata );

	gsosMakeGiftag( 5, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
	gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0 ));
	gsosSetPacketAddrData(GSOS_TEX0_1, GsosTex0Data(0x3000, 1, 0, 10, 10, 1, 1, 0, 0, 0, 0, 0 ));

	gsosSetPacketAddrData4(GSOS_SCISSOR_1, 
			       (GSOSbit64)(mydata->d.x - mydata->d.hotx), 
			       (GSOSbit64)((mydata->d.x - mydata->d.hotx) + mydata->d.width), 
			       (GSOSbit64)(mydata->d.y - mydata->d.hoty), 
			       (GSOSbit64)((mydata->d.y - mydata->d.hoty) + mydata->d.height));

	gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 1, 0, 0, 0, 1, 0, 0 ) ) ;
	gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_SPRITE ) ;

	gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 2,
			(GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_UV);

	gsosSetPacket4(GSOS_SUBPIX_OFST(5), GSOS_SUBPIX_OFST(5),0,0);
	gsosSetPacket4(GSOS_SUBPIX_OFST(mydata->d.x - mydata->d.hotx-1), 
		       GSOS_SUBPIX_OFST(mydata->d.y - mydata->d.hoty-1),0,0);

	gsosSetPacket4(GSOS_SUBPIX_OFST(5+mydata->d.width), GSOS_SUBPIX_OFST(5+mydata->d.height),0,0);
	gsosSetPacket4(GSOS_SUBPIX_OFST((mydata->d.x - mydata->d.hotx)+mydata->d.width),
		       GSOS_SUBPIX_OFST((mydata->d.y - mydata->d.hoty)+mydata->d.height),0,0);

	gsosExec() ;
	save_under = TRUE;
	return TRUE;
    }
    return FALSE;
}

void QPS2Cursor::saveUnder()
{
    QGfxPS2::flushRegisters();
    gsosReadImage((mydata->d.x - mydata->d.hotx)-5, (mydata->d.y - mydata->d.hoty)-5, 
		  64, 64, 0x00, (qt_screen->width()+63)/64, 0, mydata->grabdata);
    drawCursor();
    save_under = FALSE;
    qt_sw_cursor = TRUE;
    QWSDisplay::ungrab();
}

void QPS2Cursor::drawCursor()
{
    QGfxPS2::flushRegisters(TRUE);
    gsosWriteImage( 0, 0, 64, 64, 0x3000, 1, 0, (uchar *)mydata->cursordata );

    gsosMakeGiftag( 6, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD );
    gsosSetPacketAddrData(GSOS_ALPHA_1, GsosAlphaData(0, 1, 0, 1, 0));
    gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 7, 0, 0, 0, 0, 0, 0 ));
    gsosSetPacketAddrData(GSOS_TEX0_1, GsosTex0Data(0x3000, 1, 0, 10, 10, 1, 1, 0, 0, 0, 0, 0 ));
    gsosSetPacketAddrData4(GSOS_SCISSOR_1, 
			   (GSOSbit64)(mydata->d.x - mydata->d.hotx), 
			   (GSOSbit64)((mydata->d.x - mydata->d.hotx) + mydata->d.width), 
			   (GSOSbit64)(mydata->d.y - mydata->d.hoty), 
			   (GSOSbit64)((mydata->d.y - mydata->d.hoty) + mydata->d.height));

    gsosSetPacketAddrData(GSOS_PRMODE, GsosPrmodeData( 0, 1, 0, 1, 0, 1, 0, 0 ) ) ;
    gsosSetPacketAddrData(GSOS_PRIM, GSOS_PRIM_SPRITE ) ;

    gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 2,
		    (GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_UV);

    gsosSetPacket4(GSOS_SUBPIX_OFST(0), GSOS_SUBPIX_OFST(0),0,0);
    gsosSetPacket4(GSOS_SUBPIX_OFST(mydata->d.x - mydata->d.hotx), 
		   GSOS_SUBPIX_OFST(mydata->d.y - mydata->d.hoty),0,0);

    gsosSetPacket4(GSOS_SUBPIX_OFST(mydata->d.width), GSOS_SUBPIX_OFST(mydata->d.height),0,0);
    gsosSetPacket4(GSOS_SUBPIX_OFST((mydata->d.x - mydata->d.hotx)+mydata->d.width), 
		   GSOS_SUBPIX_OFST((mydata->d.y - mydata->d.hoty)+mydata->d.height),0,0);

    gsosExec() ;
}

#endif // QT_NO_QWS_CURSOR

