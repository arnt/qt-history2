/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_win.cpp#25 $
**
** Implementation of QPrinter class for Win32
**
** Created : 950810
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpaintdc.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qwidget.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qprinter_win.cpp#25 $");


// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )	  // set device type
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
	state = PST_ERROR;
    }
    return FALSE;
}

void QPrinter::setActive()
{
    state == PST_ACTIVE;
}

void QPrinter::setIdle()
{
    state == PST_IDLE;
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
    PRINTDLG pd;
    memset( &pd, 0, sizeof(PRINTDLG) );
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags	 = PD_RETURNDC;
    pd.hwndOwner = parent ? parent->topLevelWidget()->winId() : 0;
    pd.nFromPage = QMAX(from_pg,min_pg);
    pd.nToPage	 = QMIN(to_pg,max_pg);
    if ( pd.nFromPage > pd.nToPage )
	pd.nFromPage = pd.nToPage = 0;
    pd.nMinPage	 = min_pg;
    pd.nMaxPage	 = max_pg;
    pd.nCopies	 = ncopies;

    bool result = PrintDlg( &pd );
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


static BITMAPINFO *getWindowsBITMAPINFO( int w, int h, int d,
					 bool bottomUp=FALSE )
{
    int	ncols = 2;

    if ( w == 0 )				// null pixmap
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
    bmh->biHeight	  = bottomUp ? -h : h;
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = d;
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = bpl*h;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = 0;

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
	di.lpszDocName = doc_name;
	if ( ok && StartDoc(hdc, &di) == SP_ERROR )
	    ok = FALSE;
	if ( ok && StartPage(hdc) == SP_ERROR )
	    ok = FALSE;
	if ( !ok ) {
	    if ( hdc ) {
		DeleteDC( hdc );
		hdc = 0;
	    }
	    state = PST_ERROR;
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
	ASSERT( hdc != 0 );
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

	    int dw = w;
	    int dh = h;
	    if ( paint && paint->hasWorldXForm() ) {
		QWMatrix m = paint->worldMatrix();
		dw = qRound(dw*m.m11());
		dh = qRound(dh*m.m22());
	    }
	    BITMAPINFO *bmi = getWindowsBITMAPINFO( w, h, d,TRUE);
	    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
	    uchar *bits = new uchar[bmh->biSizeImage];

	    if ( c == PDC_DRAWPIXMAP)
		GetDIBits( pixmap.handle(), pixmap.hbm(), 0, h,
			   bits, bmi, DIB_RGB_COLORS );
	    else
		bits = image.bits();

	    HANDLE hdcPrn = CreateCompatibleDC( hdc );
	    HANDLE hbm = CreateDIBitmap( hdc, bmh, CBM_INIT,
					 bits, bmi, DIB_RGB_COLORS );
	    HANDLE oldHbm = SelectObject( hdcPrn, hbm );
	    StretchBlt( hdc, pos.x(), pos.y(), dw, dh,
			hdcPrn, 0, 0, w, h, SRCCOPY );
	    SelectObject( hdcPrn, oldHbm );
	    DeleteObject( hbm );
	    DeleteObject( hdcPrn );
	    delete [] bits;
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
