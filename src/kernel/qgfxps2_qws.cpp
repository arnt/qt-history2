/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
**
** Created : 001001
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
    gsosSave(); //save the current state

    int output = 0; //default to vesa
    int fr = 75; //default to 75 hrtz
    if(char *o = getenv("QWS_MONITOR")) {
	bool fr_spec = FALSE;
	const char *m = o;
	if(char *colon = strchr(o, ':')) {
	    fr_spec = TRUE;
	    *colon = '\0';
	    fr = atoi(++colon);
	}

	if(!strcasecmp(m, "VESA"))
	    output = 0;
	else if(!strcasecmp(m, "NTSC")) {
	    if(!fr_spec)
		fr = 0; /* default to non-interlaced */
	    output = 2;
	}
	else if(!strcasecmp(m, "PAL")) {
	    if(!fr_spec)
		fr = 1; /* default to interlaced */
	    output = 3;
	}
	else {
	    fprintf(stderr, "Unknown monitor: %s\n", m);
	    exit(2);
	}
    }
    if(output == 0) {
	/* stolen verabim from the gsx code base */
	static int framerate[4][3]= {
	    /* 60, 75, 85 Hz */
	    {   1,  3,  4},	/*  640x 480 */
	    {   2,  4,  5}, /*  800x 600 */
	    {   1,  3,  4}, /* 1024x 786 */
	    {   1,  2,  0}  /* 1280x1024 */
	};

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

	switch(fr) {
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
    } 
    gsosSetScreen(output, fr, dw, dh, 0x00, 0, 0, 0, 0);

#if 0
    int realwidth, realheight;
    if(gsosGetScreenSize(&realwidth, &realheight)) {
	dw=w=realwidth;
	dh=h=realheight;
    }
#endif
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

    // No blankin' screen, no blinkin' cursor! ##I don't think these are working for the PS2.. FIXME
    const char termctl[] = "\033[9;0]\033[?33l";
    write(1,termctl,sizeof(termctl));

    return TRUE;
}

void
QPS2Screen::disconnect()
{
    gsosClose();

    // a reasonable screensaver timeout ##I don't think these are working for the PS2.. FIXME
    printf("\033[9;15]");
    fflush(stdout);
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
    gsosRestore(); //restore the original state of the card
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
       or b) get an fb, either way will work, a is probably better. For now 
       I will leave it like this until it is decided this is unnacceptable.
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

/* This will flush registers to some acceptable values. 

   If flushtex is specified it will flush registers relating to textures, chances are you 
   don't want to use this, however if you are putting something to texture memory you
   will want to use them.
*/
void 
QGfxPS2::flushRegisters(bool flushtex) 
{
    /* flush */
    gsosMakeGiftag( flushtex ? 23: 19, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE,
		    0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);
    gsosSetPacketAddrData(GSOS_XYOFFSET_1, GsosXyoffsetData(GSOS_XYOFFSET<<4, GSOS_XYOFFSET<<4));
    gsosSetPacketAddrData(GSOS_FRAME_1, GsosFrameData(0x00, (qt_screen->deviceWidth() +63)/64, 0, 0));
    gsosSetPacketAddrData(0x100, GsosZbufData(0x100, 0, 0));
    gsosSetPacketAddrData(GSOS_PRMODECONT, GsosPrmodecontData( 0));
    gsosSetPacketAddrData(GSOS_SCANMSK, GsosScanmskData(0));
    gsosSetPacketAddrData(GSOS_CLAMP_1, GsosClampData(0, 0, 0, 0, 0, 0));
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
    if(flushtex) {
	gsosSetPacketAddrData(GSOS_TEX0_1, GsosTex0Data(0x3000, 1, 0, 10, 10, 1, 0, 0, 0, 0, 0, 0));
	gsosSetPacketAddrData(GSOS_TEX1_1, GsosTex1Data(0, 0, 0, 0, 0, 0, 0));
	gsosSetPacketAddrData(GSOS_TEX2_1, GsosTex2Data(0, 0, 0, 0, 0, 0));
	gsosSetPacketAddrData( 0x3f, 0); //why don't they do this??!?
    }
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
	
	gsosMakeGiftag(4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
			0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);
	gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0));
	gsosSetPacketAddrData4(GSOS_SCISSOR_1, 
			       (GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
			       (GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y());
	gsosSetPacketAddrData(GSOS_PRMODE, GsosPrmodeData( 0, 0, 0, 0, 0, 0, 0, 0)) ;
	gsosSetPacketAddrData(GSOS_PRIM, GSOS_PRIM_POINT) ;

	int r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)), a(qAlpha(rgb));
	gsosMakeGiftag(npoints, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
		       GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_RGBAQ);
	while (npoints--) {
	    gsosSetPacket4(r, g, b, a);
	    gsosSetPacket4(GSOS_SUBPIX_OFST(pa[index].x()+xoffs), GSOS_SUBPIX_OFST(pa[index].y()+yoffs), 0, 0);
	    ++index;
	}

    }
    gsosExec();
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

    //Qt expects points to be inclusize, so this makes the final point show on a line.
    if(x2 != x1)
	x2 += (x2 > x1) ? 1 : -1;
    if(y2 != y1)
	y2 += (y2 > y1) ? 1 : -1;

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, (x2-x1)+1, QABS((y2-y1))+1));
    usePen();
    QRgb rgb = cpen.color().rgb();

    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag(4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE,
		       0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);
	gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( 1, 1, 0, 0, 0, 0, 0, 0));
	gsosSetPacketAddrData4(GSOS_SCISSOR_1, 
			       (GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
			       (GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y());
	gsosSetPacketAddrData(GSOS_PRMODE, GsosPrmodeData(0, 0, 0, 0, 0, 1, 0, 0));
	gsosSetPacketAddrData(GSOS_PRIM, GSOS_PRIM_LINE);

	gsosMakeGiftag(2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
		       GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	gsosSetPacket4(qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb));
	gsosSetPacket4(GSOS_SUBPIX_OFST(x1), GSOS_SUBPIX_OFST(y1), 0, 0);
	gsosSetPacket4(qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb));
	gsosSetPacket4(GSOS_SUBPIX_OFST(x2), GSOS_SUBPIX_OFST(y2), 0, 0);

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

    if(cbrush.style() == QBrush::NoBrush)
	return;

    if((cbrush.style()!=QBrush::NoBrush) && patternedbrush) {
	srcwidth=cbrushpixmap->width();
	srcheight=cbrushpixmap->height();
	if(cbrushpixmap->depth()==1) {
	    if(opaque) {
		setSource(cbrushpixmap);
		setAlphaType(IgnoreAlpha);
		useBrush();
		srcclut[0]=pixel;
		QBrush tmp=cbrush;
		cbrush=QBrush(backcolor);
		useBrush();
		srcclut[1]=pixel;
		cbrush=tmp;
	    } else {
		useBrush();
		srccol=pixel;
		srctype=SourcePen;
		setAlphaType(LittleEndianMask);
		setAlphaSource(cbrushpixmap->scanLine(0), cbrushpixmap->bytesPerLine());
	    }
	} else {
	    setSource(cbrushpixmap);
	    setAlphaType(IgnoreAlpha);
	}
	tiledBlt(x, y, w, h);
	return;
    }

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

    if(cpen.style()==NoPen)
	return;

    GFX_START(clipbounds);

    usePen();
    QRgb rgb = cpen.color().rgb();
    int r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)), a(qAlpha(rgb));

    for(int clp = 0; clp < ncliprect; clp++) {

	gsosMakeGiftag(4, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);
	gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData(1, 1, 0, 0, 0, 0, 0, 0));
	gsosSetPacketAddrData4(GSOS_SCISSOR_1, 
				(GSOSbit64)cliprect[clp].topLeft().x(), (GSOSbit64)cliprect[clp].bottomRight().x(),
				(GSOSbit64)cliprect[clp].topLeft().y(), (GSOSbit64)cliprect[clp].bottomRight().y() ) ;
	gsosSetPacketAddrData(GSOS_PRMODE, GsosPrmodeData(0, 0, 0, 0, 0, 1, 0, 0 )) ;
	gsosSetPacketAddrData(GSOS_PRIM, GSOS_PRIM_LSTRIP) ;

	int end = (index+npoints) > (int)pa.size() ? pa.size() : index+npoints;
	gsosMakeGiftag((end - index), GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			GSOS_GIF_FLG_PACKED, 2,(GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	for(int x=index; x < end; x++) {
	    gsosSetPacket4(r, g, b, a);
	    gsosSetPacket4(GSOS_SUBPIX_OFST(pa[x].x() + xoffs), GSOS_SUBPIX_OFST(pa[x].y() + yoffs), 0, 0);
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

    if(cbrush.style()!=QBrush::NoBrush)
	scan(pa,winding,index,npoints);
    drawPolyline(pa, index, npoints);
    if(pa[index] != pa[index+npoints-1])
	drawLine(pa[index].x(), pa[index].y(), pa[index+npoints-1].x(), pa[index+npoints-1].y());

    GFX_END;
}

//this must go away, I will rewrite my own polygon scanner to use triangles..
void QGfxPS2::processSpans( int n, QPoint* point, int* width )
{
    if(!ncliprect)
	return;

    int usable = 0;
    for(int x = 0; x < n; x++)
	if(*(width+x)) usable++;
    if(usable) {
	useBrush();
	QRgb rgb = cbrush.color().rgb();
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

	    gsosMakeGiftag( usable * 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0,
			    GSOS_GIF_FLG_PACKED, 2, (GSOS_GIF_REG_XYZ2<<4) | (GSOS_GIF_REG_RGBAQ));
	    
	    QPoint *pf = point;
	    int *pw = width;
	    for(int pn = 0; pn < n; pn++) {
		if(*pw) {
		    gsosSetPacket4( r, g, b, a );
		    gsosSetPacket4(GSOS_SUBPIX_OFST(pf->x()+xoffs), GSOS_SUBPIX_OFST(pf->y()+yoffs),0,0);
		    gsosSetPacket4( r, g, b, a ) ;
		    gsosSetPacket4(GSOS_SUBPIX_OFST(pf->x()+((*pw))+xoffs), GSOS_SUBPIX_OFST(pf->y()+yoffs),0,0);
		}
		pf++;
		pw++;
	    }

	}
	gsosExec() ;
    }
}

/* This maps the source into a texture, for now it will always write to 0, 0 in the 
   texture buffer, it sets up the texture register, then does the write to gs local 
   memory. It will blt from x, y of the source, to x+w, y+h.
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
	    fprintf(stderr, "Woops, forgot a bitdepth in mapSourceToTexture\n");
	    break;
	case 1:
	    tex_psm = 0;
	    bits = (uchar *)malloc(aligned_width*tex_height*4);
	    for(int yi = 0; yi < tex_height; yi++) {
		uint *bitsline = (uint *)(bits + (aligned_width * yi * 4));
		int sy = yi * (srcwidth/8);
		for(int xi = 0; xi < tex_width; xi++) {
		    if(src_little_endian)
			*(bitsline+xi) = srcclut[*(srcbits + (sy + (xi / 8))) >> (xi % 8) & 0x01];
		    else
			*(bitsline+xi) = srcclut[*(srcbits + (sy + (xi / 8))) >> (7 - (xi % 8)) & 0x01];
		}
	    }
	    break;
	case 8:
	    tex_psm = 0;
	    bits = (uchar *)malloc(aligned_width*tex_height*4);
	    for (int i=0; i<tex_height; i++ ) {
		register uint *p = (uint *)(bits + ((aligned_width * i) * 4));
		uchar  *b = (srcbits + (srcwidth * (y + i)));
		uint *end = p + tex_width;
		while (p < end)
		    *p++ = srcclut[(*b++)];
	    }
	    break;
	case 16:
	    tex_psm = 2;
	    bits = (uchar *)malloc(aligned_width*tex_height*2);
	    /* fall through */
	case 32:
	    if(!bits) {
		bits = (uchar *)malloc(aligned_width*tex_height*4);
		tex_psm = 0;
	    }

	    int colsize = srcdepth == 16 ? 2 : 4;
	    for(int i = 0; i < tex_height; i++)
		memcpy(bits + ((aligned_width * i)*colsize),
		       srcbits + (((srcwidth * (y + i)) + x)*colsize), tex_width * colsize);
	    break;
	}
    } 

    if(!bits) { //woops
	tex_psm = 0;
        bits = (uchar *)malloc(aligned_width * tex_height * 4);
    }

    /* take care of the alpha */
    if(!tex_psm) { //need to add support for non rgba alpha blending FIXME!
	unsigned int rgb=0; //quiet compiler
	unsigned char alpha_channel=0; //quiet compiler
	if(srctype == SourcePen) {
	    usePen();
	    alpha_channel = 255;
	    rgb = (0x00FFFFFF) & cpen.color().rgb();
	}

	unsigned int *out = (unsigned int *)bits;
	for(int y = 0; y < tex_height; y++) {
	    int dy = (y * aligned_width);
	    int sy = (y * alphalinestep);
	    for(int x = 0; x < tex_width; x++) {
		if(srctype == SourceImage) {
		    alpha_channel = (out[dy+x] >> 24) & 0xFF;
		    rgb = (0x00FFFFFF) & out[dy+x];
		}

		switch(alphatype) {
		case LittleEndianMask:
		case BigEndianMask:
		{
		    char a = alphabits[sy + (x / 8)];
		    if(alphatype == LittleEndianMask) 
			a = a >> (x % 8);
		    else
			a = a >> (7 - (x % 8));
		    alpha_channel = ((a & 0x01) ? 0xFF : 0x00);
		    break;
		}	
		case SeparateAlpha:
		    alpha_channel = alphabits[sy+x];
		    break;
		case SolidAlpha:
		    //not sure what this one means..

		case InlineAlpha: /* do nothing for these */
		case IgnoreAlpha:
		    break;
		// do not add a default case so the compiler whines when new alphas are added!
		}
		out[dy+x] = ((alpha_channel / 2) << 24) | //divide by two because GS says 0x80 is 1.0
			    ((rgb << 16) & 0x00ff0000) | (rgb & 0x0000ff00) | ((rgb >> 16) & 0xff); //swap to BGR!!!
	    }
	}
    }

    /* now's my last chance to clean up all registers */
    flushRegisters(TRUE);

    /* setup the texture buffer */
    int nw = 0, nh = 0 ;
    while((1<<nw) < tex_width) nw++;
    while((1<<nh) < tex_height) nh++;
    gsosMakeGiftag(1, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);
    gsosSetPacketAddrData(GSOS_TEX0_1, GsosTex0Data(0x3000, (tex_width+63)/64, tex_psm, nw, nh, 1, 1, 0, 0, 0, 0, 0));
    gsosExec();

    /* throw it in the texture */
    gsosWriteImage(0, 0, aligned_width, tex_height, 0x3000, aligned_width / 64, tex_psm, (uchar *)bits);
    free(bits);
    return TRUE;
}

/* This will blt a texture (presumably from mapSourceToTexture) at screen
   coord x, y using clipping region clp, if clp is -1 it will use a
   bounding rect of the clipped region.

   You can specify the width and height of the blt itself with w and h, if
   set to -1 then they will use the cached texture width and texture height
   (again from mapSourceToTexture).

   You can also specify the width and height of the texture sample with tw
   and th, if set to -1 then they will use the cached texture width and
   texture height as well.
*/
bool
QGfxPS2::bltTexture(int x, int y, int clp, int w, int h, int tw, int th)
{
    //defaults for blt w/h
    if(w == -1) 
	w = tex_width; 
    if(h == -1)
	h = tex_height;
    //defaults for tex w/h
    if(tw == -1)
	tw = tex_width;
    if(th == -1)
	th = tex_height;

    gsosMakeGiftag(5, GSOS_GIF_EOP_CONTINUE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 1, GSOS_GIF_REG_AD);

    //do alpha blending
    bool do_alpha = (alphatype != IgnoreAlpha);
    gsosSetPacketAddrData(GSOS_ALPHA_1, GsosAlphaData(0, 1, 0, 1, 0));
    gsosSetPacketAddrData(GSOS_TEST_1, GsosTestData( do_alpha, 7, 0, 0, 0, 0, 0, 0 ));

    //handle clips
    QRect clip;
    if(clp == -1)
	clip = clipbounds;
    else
	clip = cliprect[clp];
    gsosSetPacketAddrData4(GSOS_SCISSOR_1,(GSOSbit64)clip.topLeft().x(), (GSOSbit64)clip.bottomRight().x(),
			   (GSOSbit64)clip.topLeft().y(), (GSOSbit64)clip.bottomRight().y()) ;

    //do it on a sprite..
    gsosSetPacketAddrData( GSOS_PRMODE, GsosPrmodeData( 0, 1, 0, do_alpha, 0, 1, 0, 0 ) ) ;
    gsosSetPacketAddrData( GSOS_PRIM, GSOS_PRIM_SPRITE ) ;

    //draw it
    gsosMakeGiftag( 2, GSOS_GIF_EOP_TERMINATE, GSOS_GIF_PRE_IGNORE, 0, GSOS_GIF_FLG_PACKED, 
		    2, (GSOS_GIF_REG_XYZ2<<4) | GSOS_GIF_REG_UV);

    gsosSetPacket4(GSOS_SUBPIX_OFST(0), GSOS_SUBPIX_OFST(0),0,0);
    gsosSetPacket4(GSOS_SUBPIX_OFST(x), GSOS_SUBPIX_OFST(y),0,0);

    gsosSetPacket4(GSOS_SUBPIX_OFST((tw+1)), GSOS_SUBPIX_OFST((th+1)),0,0);
    gsosSetPacket4(GSOS_SUBPIX_OFST((x+w)), GSOS_SUBPIX_OFST((y+h)),0,0);

    gsosExec();

    return TRUE;
}


void
QGfxPS2::blt(int rx, int ry, int w, int h, int sx,int sy)
{
    if(!ncliprect)
	return;

    rx += xoffs;
    ry += yoffs;

    GFX_START(QRect(rx, ry, w+1, h+1));
    QWSDisplay::grab(TRUE);

    /* this is really gross, however it'll work for now, basically I see
       the buffers wrapping at some point so I'm going to segment up blts
       into BLT_CHUNKxBLT_CHUNK image chunks..
    */
#define     BLT_CHUNK 256 //should be a multiple of 64 for efficency in mapSourceToTexture
#if defined(BLT_CHUNK)
    if(w > BLT_CHUNK || h > BLT_CHUNK) {
	for(int wi = 0; wi < w; wi += BLT_CHUNK) {
	    for(int hi = 0; hi < h; hi += BLT_CHUNK) {
		QRect br(rx + wi, ry + hi, (w - wi) < BLT_CHUNK ? w - wi : BLT_CHUNK, 
			 (h - hi) < BLT_CHUNK ? h - hi : BLT_CHUNK);
		if(clipbounds.intersects(br)) {
		    if(mapSourceToTexture(sx+wi, sy+hi, br.width(), br.height())) {
			for(int clp = 0; clp < ncliprect; clp++)
			    bltTexture(br.x(), br.y(), clp);
		    }
		}
	    }
	}
    } else 
#endif //BLT_CHUNK hack
	if(mapSourceToTexture(sx, sy, w, h)) {
	    for(int clp = 0; clp < ncliprect; clp++)
		bltTexture(rx, ry, clp);
	}

    QWSDisplay::ungrab();
    GFX_END;

}


#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
void
QGfxPS2::stretchBlt( int rx, int ry, int w, int h, int sx, int sy)
{
    if(!ncliprect)
	return;

    rx += xoffs;
    ry += yoffs;

    GFX_START(QRect(rx, ry, w+1, h+1));
    QWSDisplay::grab(TRUE);

    if(mapSourceToTexture(sx, sy, srcwidth-sx, srcheight-sy)) {
	for(int clp = 0; clp < ncliprect; clp++)
	    bltTexture(rx, ry, clp, w, h);
    }

    QWSDisplay::ungrab();
    GFX_END;
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
    QWSDisplay::grab(TRUE);

    /* I should probably do the chunking that I do in blt in some general function
       and use it here, however tiledblt hasn't proved to be necesary since the tiles
       have been small thus far..
    */

    if(mapSourceToTexture(0, 0, srcwidth, srcheight)) {
	for(int clp = 0; clp < ncliprect; clp++)
	    bltTexture(rx, ry, clp, w, h, w, h);
    }

    QWSDisplay::ungrab();
    GFX_END;
}

/* This doesn't use textures for blts because screen->screen seemed right
   If this seems to be a bad assumption I will come back and rewrite
   this to use blt()
*/
void
QGfxPS2::scroll( int rx, int ry, int w, int h,int sx, int sy)
{
    rx += xoffs;
    ry += yoffs;
    sx += xoffs;
    sy += yoffs;

    GFX_START(QRect(rx, ry, w+1, h+1));
    QWSDisplay::grab(TRUE);

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

    QWSDisplay::ungrab();
    GFX_END;    
}

/* I should move these to qgfxraster.. */
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
/* I could probably collapse the blts in this chunk of code into some
   single point and use it with the QGfxPS2 but I think right now I'd 
   prefer to keep them separate, so when I break blt()'s in the qgfx fu
   the cursor will still somewhat work.
*/
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
    gsosReadImage(mydata->d.x-5, mydata->d.x-5, 64, 64, 0x00, (qt_screen->width()+63)/64, 0, mydata->grabdata);
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
	QWSDisplay::grab(TRUE);
	QGfxPS2::flushRegisters(TRUE);

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

