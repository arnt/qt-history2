/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_win.cpp#41 $
**
** Implementation of QPrinter class for Win32
**
** Created : 950810
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpaintdevicedefs.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qt_windows.h"


// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ABORTED	2


QPrinter::QPrinter()
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    orient      = Portrait;
    page_size   = A4;
    ncopies     = 1;
    from_pg     = to_pg = min_pg  = max_pg = 0;
    state       = PST_IDLE;
    output_file = FALSE;
}

QPrinter::~QPrinter()
{
}


bool QPrinter::newPage()
{
    if ( hdc && state == PST_ACTIVE ) {
	if ( EndPage(hdc) != SP_ERROR && StartPage(hdc) != SP_ERROR )
	    return TRUE;
	state = PST_ABORTED;
    }
    return FALSE;
}

void QPrinter::setActive()
{
    state = PST_ACTIVE;
}

void QPrinter::setIdle()
{
    state = PST_IDLE;
}


bool QPrinter::abort()
{
    if ( state == PST_ACTIVE )
	state = PST_ABORTED;
    return state == PST_ABORTED;
}

bool QPrinter::aborted() const
{
    return state == PST_ABORTED;
}


bool QPrinter::setup( QWidget *parent )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    PRINTDLG pd;
    memset( &pd, 0, sizeof(PRINTDLG) );
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags	 = PD_RETURNDC;
    pd.hwndOwner = parent ? parent->winId() : 0;
    pd.nFromPage = QMAX(from_pg,min_pg);
    pd.nToPage	 = QMIN(to_pg,max_pg);
    if ( pd.nFromPage > pd.nToPage )
	pd.nFromPage = pd.nToPage = 0;
    pd.nMinPage	 = min_pg;
    pd.nMaxPage	 = max_pg;
    pd.nCopies	 = ncopies;

    bool result = PrintDlg( &pd );
    if ( result && pd.hDC == 0 )
	result = FALSE;
    if ( result ) {				// get values from dlg
	from_pg = pd.nFromPage;
	to_pg	= pd.nToPage;
	ncopies = pd.nCopies;
	hdc	= pd.hDC;
    }
    if ( pd.hDevMode )
	GlobalFree( pd.hDevMode );
    if ( pd.hDevNames )
	GlobalFree( pd.hDevNames );
    return result;
}


static BITMAPINFO *getWindowsBITMAPINFO( const QPixmap &pixmap,
					 const QImage &image )
{
    int w=0, h=0, d=0, ncols=2;
    if ( !pixmap.isNull() ) {
	w = pixmap.width();
	h = pixmap.height();
	d = pixmap.depth();
    } else {
	w = image.width();
	h = image.height();
	d = image.depth();	
    }

    if ( w == 0 || h == 0 || d == 0 )		// invalid image or pixmap
	return 0;

    if ( d > 1 && d <= 8 ) {			// set to nearest valid depth
	d = 8;					//   2..7 ==> 8
	ncols = 256;
    } else if ( d > 8 ) {
	d = 32;					//   > 8  ==> 32
	ncols = 0;
    }

    int   bpl = ((w*d+31)/32)*4;    		// bytes per line
    int	  bmi_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = (char *)malloc( bmi_len );
    memset( bmi_data, 0, bmi_len );
    BITMAPINFO	     *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize		  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth	  = w;
    bmh->biHeight	  = -h;
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = d;
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = bpl*h;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = 0;

    if ( ncols > 0 && !image.isNull() ) {	// image with color table
	RGBQUAD *r = (RGBQUAD*)(bmi_data + sizeof(BITMAPINFOHEADER));
	ncols = QMIN(ncols,image.numColors());
	for ( int i=0; i<ncols; i++ ) {
	    QColor c = image.color(i);
	    r[i].rgbRed = c.red();
	    r[i].rgbGreen = c.green();
	    r[i].rgbBlue = c.blue();
	    r[i].rgbReserved = 0;
	}
    }

    return bmi;
}


bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
	bool ok = state == PST_IDLE;
	if ( ok && !hdc ) {
	    setup( 0 );
	    if ( !hdc )
		ok = FALSE;
	}
	DOCINFO di;
	memset( &di, 0, sizeof(DOCINFO) );
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = (TCHAR*)qt_winTchar(doc_name,TRUE);
	if ( ok && StartDoc(hdc, &di) == SP_ERROR )
	    ok = FALSE;
	if ( ok && StartPage(hdc) == SP_ERROR )
	    ok = FALSE;
	if ( !ok ) {
	    if ( hdc ) {
		DeleteDC( hdc );
		hdc = 0;
	    }
	    state = PST_ABORTED;
	} else {
	    state = PST_ACTIVE;
	}
    } else if ( c == PDC_END ) {
	if ( hdc ) {
	    EndPage( hdc );			// end; printing done
	    EndDoc( hdc );
	    DeleteDC( hdc );
	    hdc = 0;
	}
	state = PST_IDLE;
    } else {					// all other commands...
	if ( state != PST_ACTIVE )		// aborted or error
	    return FALSE;
	if ( hdc == 0 ) {			// device unexpectedly reset
	    state = PST_ABORTED;
	    return FALSE;
	}
	if ( c == PDC_DRAWPIXMAP || c == PDC_DRAWIMAGE ) {
	    QPoint  pos	   = *p[0].point;
	    QPixmap pixmap;
	    QImage  image;

	    int w;
	    int h;
	    int d;

	    if ( c == PDC_DRAWPIXMAP ) {
		pixmap = *p[1].pixmap;
		w = pixmap.width();
		h = pixmap.height();
		d = pixmap.depth();
	    } else {
		image = *p[1].image;
		w = image.width();
		h = image.height();
		d = image.depth();
	    }

	    double xs = 1.0;			// x stretch
	    double ys = 1.0;			// y stretch
	    if ( paint ) {
		bool wxf = paint->hasWorldXForm();
		bool vxf = paint->hasViewXForm();
		if ( wxf || vxf ) {		// map position
		    pos = paint->xForm( pos );
		}
		if ( wxf ) {
		    QWMatrix m = paint->worldMatrix();
		    xs = m.m11();
		    ys = m.m22();
		}
		if ( vxf ) {
		    QRect vr = paint->viewport();
		    QRect wr = paint->window();
		    xs = xs * vr.width() / wr.width();
		    ys = ys * vr.height() / wr.height();
		}
	    }
	    int dw = qRound( xs * w );
	    int dh = qRound( ys * h );
	    BITMAPINFO *bmi = getWindowsBITMAPINFO( pixmap, image );
	    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
	    uchar *bits;

	    if ( c == PDC_DRAWPIXMAP ) {
		bits = new uchar[bmh->biSizeImage];
		GetDIBits( pixmap.handle(), pixmap.hbm(), 0, h,
			   bits, bmi, DIB_RGB_COLORS );
	    } else {
		bits = image.bits();
	    }
	    int rc = GetDeviceCaps(hdc,RASTERCAPS);
	    if ( (rc & RC_STRETCHDIB) != 0 ) {
		// StretchDIBits supported
		StretchDIBits( hdc, pos.x(), pos.y(), dw, dh, 0, 0, w, h,
			       bits, bmi, DIB_RGB_COLORS, SRCCOPY );
	    } else if ( (rc & RC_STRETCHBLT) != 0 ) {
		// StretchBlt supported
		HDC     hdcPrn = CreateCompatibleDC( hdc );
		HBITMAP hbm    = CreateDIBitmap( hdc, bmh, CBM_INIT,
						 bits, bmi, DIB_RGB_COLORS );
		HBITMAP oldHbm = (HBITMAP)SelectObject( hdcPrn, hbm );
		StretchBlt( hdc, pos.x(), pos.y(), dw, dh,
			    hdcPrn, 0, 0, w, h, SRCCOPY );
		SelectObject( hdcPrn, oldHbm );
		DeleteObject( hbm );
		DeleteObject( hdcPrn );
	    }
	    if ( c == PDC_DRAWPIXMAP ) {
		delete [] bits;
	    }
	    free( bmi );
	    return FALSE;			// don't bitblt
	}
    }
    return TRUE;
}


int QPrinter::metric( int m ) const
{
    if ( handle() == 0 )			// not ready
	return 0;
    int query;
    switch ( m ) {
	case PDM_WIDTH:
	    query = HORZRES;
	    break;
	case PDM_HEIGHT:
	    query = VERTRES;
	    break;
	case PDM_WIDTHMM:
	    query = HORZSIZE;
	    break;
	case PDM_HEIGHTMM:
	    query = VERTSIZE;
	    break;
	case PDM_NUMCOLORS:
	    query = NUMCOLORS;
	    break;
	case PDM_DEPTH:
	    query = PLANES;
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QPrinter::metric: Invalid metric command" );
#endif
	    return 0;
    }
    return GetDeviceCaps( handle(), query );
}
