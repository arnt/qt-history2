/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_win.cpp#6 $
**
** Implementation of QPrinter class for Windows
**
** Author  : Haavard Nord
** Created : 950810
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpaintdc.h"
#include "qapp.h"
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qprinter_win.cpp#6 $")


// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )	  // set device type
{
    orient = Portrait;
    page_size = A4;
    from_pg = to_pg  = ncopies = 1;
    min_pg  = max_pg = 0;
    state = PST_IDLE;
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
    if ( parent == 0 )
	parent = qApp->mainWidget();
    PRINTDLG pd;
    memset( &pd, 0, sizeof(PRINTDLG) );
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags	 = PD_RETURNDC;
    pd.hwndOwner = parent ? parent->id() : QApplication::desktop()->id();
    pd.nFromPage = from_pg;
    pd.nToPage	 = to_pg;
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


bool QPrinter::cmd( int c, QPainter *, QPDevCmdParam *p )
{
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
	bool ok = state == PST_IDLE;
	if ( ok && !hdc ) {
	    setup( 0 );
	    if ( !hdc )
		ok = FALSE;
	}
	DOCINFO di;
	di.cbSize      = sizeof(DOCINFO);
	di.lpszDocName = doc_name;
	di.lpszOutput  = 0;
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
	}
	else
	    state = PST_ACTIVE;
    }
    else if ( c == PDC_END ) {
	if ( hdc ) {
	    EndPage( hdc );			// end; printing done
	    EndDoc( hdc );
	    DeleteDC( hdc );
	    hdc = 0;
	}
	state = PST_IDLE;
    }
    else {					// all other commands...
	if ( state != PST_ACTIVE )		// aborted or error
	    return FALSE;
    }
    return TRUE;
}


long QPrinter::metric( int m ) const
{
    long val;
    PageSize s = pageSize();
    ASSERT( s >= A4 && s <= Executive );
    static int widths[]	 = { 559, 480, 576, 576, 504 };
    static int heights[] = { 806, 693, 756, 972, 684 };
    static int widthsMM[]  = { 210, 182, 216, 216, 191 };
    static int heightsMM[] = { 297, 257, 279, 356, 254 };
    switch ( m ) {
	case PDM_WIDTH:
	    val = widths[ s ];
	    break;
	case PDM_HEIGHT:
	    val = heights[ s ];
	    break;
	case PDM_WIDTHMM:
	    val = widthsMM[ s ];
	    break;
	case PDM_HEIGHTMM:
	    val = heightsMM[ s ];
	    break;
	case PDM_NUMCOLORS:
	    val = 16777216;
	    break;
	case PDM_DEPTH:
	    val = 24;
	    break;
	default:
	    val = 0;
#if defined(CHECK_RANGE)
	    warning( "QPixmap::metric: Invalid metric command" );
#endif
    }
    return val;
}
