/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_win.cpp#1 $
**
** Implementation of QPrinter class for Windows
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpaintdc.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter_win.cpp#1 $";
#endif


QPrinter::QPrinter( const char *name )
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    pname = name;
    if ( pname.isEmpty() ) {
	// get default printer
    }
    fromp = top = ncopies = 1;
    minp  = maxp = 0;
}

QPrinter::~QPrinter()
{
}


void QPrinter::setPrinterName( const char *name )
{
    pname = name;
}

void QPrinter::setDocName( const char *name )
{
    dname = name;
}

void QPrinter::setCreator( const char *creator )
{
    cname = creator;
}


void QPrinter::setFromTo( int fromPage, int toPage )
{
    fromp = fromPage;
    top = toPage;
}

void QPrinter::setMinMax( int minPages, int maxPages )
{
    minp = minPages;
    maxp = maxPages;
}

void QPrinter::setNumCopies( int numCopies )
{
    ncopies = numCopies;
}


bool QPrinter::select( QPrinter *p, QWidget *parent )
{
    if ( parent == 0 )
	parent = qApp->mainWidget();
    PRINTDLG pd;
    memset( &pd, 0, sizeof(PRINTDLG) );
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags	 = PD_RETURNDC;
    pd.hwndOwner = parent ? parent->id() : QApplication::desktop()->id();
    pd.nFromPage = p->fromp;
    pd.nToPage	 = p->top;
    pd.nMinPage	 = p->minp;
    pd.nMaxPage	 = p->maxp;
    pd.nCopies	 = p->ncopies;
    bool result = PrintDlg( &pd );
    if ( result ) {				// get values from dlg
	p->fromp   = pd.nFromPage;
	p->top	   = pd.nToPage;
	p->ncopies = pd.nCopies;
	p->hdc	   = pd.hDC;
    }
    if ( pd.hDevMode )
	GlobalFree( pd.hDevMode );
    if ( pd.hDevNames )
	GlobalFree( pd.hDevNames );
    return result;
}


bool QPrinter::cmd( int c, QPDevCmdParam *p )
{
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
	ASSERT( hdc != 0 );
	DOCINFO di;
	di.cbSize      = sizeof(DOCINFO);
	di.lpszDocName = dname;
	di.lpszOutput  = 0;
	ASSERT( StartDoc( hdc, &di ) != SP_ERROR );
	ASSERT( StartPage( hdc ) > 0 );
    }
    if ( c == PDC_END ) {			// end; printing done
	ASSERT( hdc != 0 );
	ASSERT( EndPage( hdc ) > 0 );
	ASSERT( EndDoc( hdc ) != SP_ERROR );
	DeleteDC( hdc );
	hdc = 0;
    }
    return TRUE;
}


