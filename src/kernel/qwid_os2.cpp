/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwid_os2.cpp#14 $
**
** Implementation of QWidget and QView classes for OS/2 PM
**
** Created : 940712
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qview.h"
#include "qobjcoll.h"
#include "qcolor.h"
#define	 INCL_PM
#include <os2.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qwid_os2.cpp#14 $");


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

const char *qWidgetClassName = "QWidget";

extern "C" MRESULT EXPENTRY WndProc( HWND, ULONG, MPARAM, MPARAM );

bool QWidget::create()				// create widget
{
    if ( testWFlags( WState_Created ) )		// already created
	return FALSE;

    static bool wc_exists = FALSE;		// window class exists?
    if ( !wc_exists ) {				// create window class
	WinRegisterClass( qPMAppInst(), (PSZ)qWidgetClassName,
			  WndProc,
			  CS_MOVENOTIFY,
			  0 );
	wc_exists = TRUE;
    }

    QWidget *parent = parentWidget();
    char *title = 0;
    bool  overlap = !parent;
    ULONG wstyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    static ULONG wflags = 0;
    WId	  id;

    if ( overlap )
	setWFlags( WType_Overlap );
    if ( testWFlags(WType_Modal) && !testWFlags(WStyle_Title) )
	 wflags |= FCF_DLGBORDER;
    if ( !testWFlags(WType_Modal) && overlap ) {
	wflags |= FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
		  FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST;
	setWFlags(WStyle_Border);
	setWFlags(WStyle_Title);
	setWFlags(WStyle_Close);
	setWFlags(WStyle_Resize);
	setWFlags(WStyle_MinMax);
    }
    else {
	if ( testWFlags(WStyle_Border) )
	    wflags |= FCF_BORDER;
	if ( testWFlags(WStyle_Title) )
	    wflags |= FCF_TITLEBAR;
	if ( testWFlags(WStyle_Close) )
	    wflags |= FCF_SYSMENU;
	if ( testWFlags(WStyle_Resize) )
	    wflags |= FCF_SIZEBORDER;
	if ( testWFlags(WStyle_Minimize) )
	    wflags |= FCF_MINBUTTON;
	if ( testWFlags(WStyle_Maximize) )
	    wflags |= FCF_MAXBUTTON;
    }
    if ( testWFlags(WStyle_Title) )
	title = qAppName();

    HWND frame_win;
    if ( overlap ) {				// create overlapped widget
	static HWND client_win;
	frame_win = WinCreateStdWindow(
		parent ? parent->id() : HWND_DESKTOP,
		wstyle, &wflags, (PSZ)qWidgetClassName,
		(PSZ)title,
		0, 0, 0,
		&client_win );
	id = client_win;
	frm_wnd = frame_win;
	rect = QRect( 0, 0, 0, 0 );
    }
    else {					// create child widget
	int x, y, w, h;
	x = y = 10;
	w = h = 40;
	id = WinCreateWindow(
		parent->ident,
		(PSZ)qWidgetClassName, 0, wstyle,
		x, y, w, h,
		parent->id(),
		HWND_BOTTOM,
		0, 0, 0 );
	frm_wnd = id;
	rect = QRect( x, y, w, h );
    }
    ncrect = rect;
    set_id( id );				// set widget id/handle
    setDevType( PDT_WIDGET );

    hps = 0;					// no drawing yet
    setCursor( arrowCursor );			// default cursor

    setWFlags( WState_Created );
    return TRUE;
}

bool QWidget::destroy()				// destroy widget
{
    if ( testWFlags( WState_Created ) ) {
	clearWFlags( WState_Created );
	WinDestroyWindow( frm_wnd );
	set_id( 0 );
    }
    return TRUE;
}


QColor QWidget::backgroundColor() const		// get background color
{
    if ( bg_col.isValid() )
	return bg_col;
    return white;				// white is default background
}

QColor QWidget::foregroundColor() const		// get foreground color
{
    if ( fg_col.isValid() )
	return fg_col;
    return black;				// black is default foreground
}

void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    setBackgroundMode( FixedColor );
    update();
}

void QWidget::setForegroundColor( const QColor &c )
{						// set foreground color
    fg_col = c;
    update();
}


QFont QWidget::font() const			// get font
{
    return fnt;
}

void QWidget::setFont( const QFont &f )		// set font
{
    fnt = f;
}

QCursor QWidget::cursor() const			// get cursor
{
    return curs;
}

void QWidget::setCursor( const QCursor &c )	// set cursor
{
    ((QCursor*)&c)->update();
    curs = c;
    WinSetPointer( HWND_DESKTOP, curs.handle() );
}


bool QWidget::update()				// update widget
{
    WinInvalidateRect( ident, 0, TRUE );
    return TRUE;
}

bool QWidget::show()				// show widget
{
    if ( testWFlags( WState_Visible ) )
	return FALSE;
    WinSetWindowPos( frm_wnd, 0, ncrect.left(),
		     convertYPos(ncrect.bottom()),
		     ncrect.width(), ncrect.height(),
		     SWP_MOVE | SWP_SIZE | SWP_SHOW );
    setWFlags( WState_Visible );
    return TRUE;
}

bool QWidget::hide()				// hide widget
{
    if ( !testWFlags( WState_Visible ) )
	return FALSE;
    WinShowWindow( frm_wnd, FALSE );
    clearWFlags(WState_Visible);
    return TRUE;
}

bool QWidget::raise()				// raise widget
{
    WinSetWindowPos( frm_wnd, 0, 0, 0, 0, 0, SWP_ACTIVATE );
    return TRUE;
}

bool QWidget::lower()				// lower widget
{
    WinSetWindowPos( frm_wnd, 0, 0, 0, 0, 0, SWP_DEACTIVATE );
    return TRUE;
}


//
// The internal qWinRequestConfig, defined in qapp_os2.cpp, stores move,
// resize and changeGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig( WId, int, int, int, int, int );

bool QWidget::move( int x, int y )		// move widget
{
    if ( testWFlags( WWin_Config ) )		// processing config event
	qWinRequestConfig( ident, 0, x, y, 0, 0 );
    else {
	if ( !testWFlags( WState_Visible ) ) {
	    ncrect = QRect(x,y,ncrect.width(),ncrect.height());
	    rect = ncrect;
	    return TRUE;
	}
	setWFlags( WWin_Config );
	WinSetWindowPos( frm_wnd, 0, x, convertYPos(y+ncrect.height()),
			 0, 0, SWP_MOVE );
	clearWFlags( WWin_Config );
    }
    return TRUE;
}

bool QWidget::resize( int w, int h )		// resize widget
{
    if ( testWFlags( WWin_Config ) )		// processing config event
	qWinRequestConfig( ident, 1, 0, 0, w, h );
    else {
	if ( !testWFlags( WState_Visible ) ) {
	    ncrect = QRect(ncrect.left(),ncrect.top(),w,h);
	    rect = ncrect;
	    return TRUE;
	}
	setWFlags( WWin_Config );
	WinSetWindowPos( frm_wnd, 0, 0, 0, w, h, SWP_SIZE );
	clearWFlags( WWin_Config );
    }
    return TRUE;
}

bool QWidget::changeGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( testWFlags( WWin_Config ) )		// processing config event
	qWinRequestConfig( ident, 2, x, y, w, h );
    else {
	if ( !testWFlags( WState_Visible ) ) {
	    ncrect = QRect(x,y,w,h);
	    rect = ncrect;
	    return TRUE;
	}
	setWFlags( WWin_Config );
	WinSetWindowPos( frm_wnd, 0, x, convertYPos(y+h), w, h,
			 SWP_MOVE | SWP_SIZE );
	reposChildren();
	clearWFlags( WWin_Config );
    }
    return TRUE;
}


int QWidget::convertYPos( int y )		// get correct y-pos
{
    HWND h;					// parent window
    if ( testWFlags(WType_Overlap) )
	h = HWND_DESKTOP;
    else
	h = parentWidget()->id();
    RECTL r;
    WinQueryWindowRect( h, &r );
    return r.yTop - r.yBottom - y;
}

void QWidget::reposChildren()			// change childrens pos
{
    if ( !isParentType() )			// ordinary widget
	return;
    int h = convertYPos( 0 );
    QView *v = (QView *)this;
    register QWidget *w;
    QObjectListIt it(*v->children());
    while ( it.current() ) {
	if ( it.current()->isWidgetType() ) {
	    w = (QWidget*)it.current();
	    int x = w->geometry().left();
	    int y = h - w->geometry().bottom();
	    debug( "repos (%d,%d)->(%d,%d)", w->geometry().left(),
		w->geometry().top(), x, y );
	    if ( w->isVisible() )
		WinSetWindowPos( w->id(), 0, x, y, 0, 0,
				 SWP_MOVE );
	}
	++it;
    }
}


bool QWidget::erase()				// erase widget contents
{
    bool tmp_hps = hps == 0;
    long bgc, txc;
    if ( tmp_hps ) {				// create new hps
	hps = WinGetPS( ident );
	GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, 0 );
    }
    RECTL r;
    WinQueryWindowRect( ident, &r );
    WinFillRect( hps, &r, backgroundColor().pixel() );
    if ( tmp_hps ) {
	WinReleasePS( hps );
	hps = 0;
    }
    return TRUE;
}

bool QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    WinScrollWindow( ident, dx, -dy, 0, 0, 0, 0,
		     SW_SCROLLCHILDREN | SW_INVALIDATERGN );
    return TRUE;
}


bool QWidget::drawText( int x, int y, const char *str )
{						// draw text in widget
    bool tmp_hps = hps == 0;
    int	 bgm;
    long bgc, txc;
    if ( tmp_hps ) {				// create new hps
	hps = WinGetPS( ident );
	GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, 0 );
    }
    else					// save painter settings
	GpiSavePS( hps );
    GpiSetColor( hps, foregroundColor().pixel() );
    POINTL p;
    p.x = x;
    p.y = clientSize().height() - y + 1;
    GpiCharStringAt( hps, &p, strlen(str), (PSZ)str );
    if ( tmp_hps ) {
	WinReleasePS( hps );
	hps = 0;
    }
    else					// restore painter settings
	GpiRestorePS( hps, -1 );
    return TRUE;
}


/*****************************************************************************
  QView member functions
 *****************************************************************************/

void QView::setCaption( const char *s )			// set caption text
{
    ctext = s;
    WinSetWindowText( frm_wnd, (PSZ)s );
}

void QView::setIconText( const char *s )		// set icon text
{
    itext = s;
}
