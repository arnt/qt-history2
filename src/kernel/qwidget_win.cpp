/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_win.cpp#1 $
**
** Implementation of QWidget and QView classes for Windows + NT
**
** Author  : Haavard Nord
** Created : 931205
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qview.h"
#include "qcolor.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget_win.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// QWidget member functions
//

const char *qWidgetClassName = "QWidget";

#if defined(_WS_WIN32_)
#define __export
#endif

extern "C" long CALLBACK __export WndProc( HWND, WORD, WORD, LONG );

bool QWidget::create()				// create widget
{
    if ( testFlag( WState_Created ) )		// already created
	return FALSE;

    static bool wc_exists = FALSE;		// window class exists?
    if ( !wc_exists ) {				// create window class
	WNDCLASS wc;
	wc.style	 = CS_DBLCLKS; // | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = (WNDPROC)WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = qWinAppInst();
	wc.hIcon	 = LoadIcon(0,IDI_APPLICATION);
	wc.hCursor	 = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName	 = 0;
	wc.lpszClassName = qWidgetClassName;
	RegisterClass( &wc );
	wc_exists = TRUE;
    }

    QWidget *parent = parentWidget();
    char *title = 0;
    bool  overlap = !parent;
    DWORD style = overlap ? 0 : (WS_CHILD | WS_CLIPSIBLINGS);
    WId	  id;

    if ( overlap )
	setFlag( WType_Overlap );
    if ( testFlag(WType_Modal) && !testFlag(WStyle_Title) )
	 style |= WS_DLGFRAME;
    if ( !testFlag(WType_Modal) && overlap ) {
	style |= WS_OVERLAPPEDWINDOW;
	setFlag(WStyle_Border);
	setFlag(WStyle_Title);
	setFlag(WStyle_Close);
	setFlag(WStyle_Resize);
	setFlag(WStyle_MinMax);
    }
    else {
	if ( testFlag(WStyle_Border) )
	    style |= WS_BORDER;
	if ( testFlag(WStyle_Title) )
	    style |= WS_CAPTION;
	if ( testFlag(WStyle_Close) )
	    style |= WS_SYSMENU;
	if ( testFlag(WStyle_Resize) )
	    style |= WS_THICKFRAME;
	if ( testFlag(WStyle_Minimize) )
	    style |= WS_MINIMIZEBOX;
	if ( testFlag(WStyle_Maximize) )
	    style |= WS_MAXIMIZEBOX;
    }
    if ( testFlag(WStyle_VScroll) )
	style |= WS_VSCROLL;
    if ( testFlag(WStyle_HScroll) )
	style |= WS_HSCROLL;
    if ( testFlag(WStyle_Title) ) {
	title = text();
	if ( !title )
	    title = qAppName();
    }

    if ( overlap ) {				// create overlapped widget
	id = CreateWindow( qWidgetClassName, title, style,
			   CW_USEDEFAULT, CW_USEDEFAULT,
			   CW_USEDEFAULT, CW_USEDEFAULT,
			   parent ? parent->ident : 0, 0,
			   qWinAppInst(), NULL );
    }
    else {					// create child widget
	int x, y, w, h;
	x = y = 10;
	w = h = 40;
	id = CreateWindow( qWidgetClassName, title, style,
			   x, y,
			   w, h,
			   parent->ident, NULL, qWinAppInst(), NULL );
    }
    set_id( id );				// set widget id/handle
    devType = PDT_WIDGET;

    RECT cr, ncr;
    POINT pt;
    GetClientRect( id, &cr );
    GetWindowRect( id, &ncr );			// update rects
    ncrect = QRect( QPoint((QCOOT)ncr.left, (QCOOT)ncr.top),
		    QPoint((QCOOT)ncr.right, (QCOOT)ncr.bottom) );
    pt.x = 0;
    pt.y = 0;
    ClientToScreen( id, &pt );
    rect = QRect( QPoint((QCOOT)(pt.x+cr.left), (QCOOT)(pt.y+cr.top)),
		  QPoint((QCOOT)(pt.x+cr.right), (QCOOT)(pt.y+cr.bottom)) );

    hdc = 0;					// no display context
    setCursor( arrowCursor );			// default cursor

    setFlag( WState_Created );
    return TRUE;
}

bool QWidget::destroy()				// destroy widget
{
    if ( testFlag( WState_Created ) ) {
	clearFlag( WState_Created );
	DestroyWindow( ident );
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
}


bool QWidget::update()				// update widget
{
    InvalidateRect( ident, 0, TRUE );
    return TRUE;
}

bool QWidget::show()				// show widget
{
    if ( testFlag( WState_Visible ) )
	return FALSE;
    ShowWindow( ident, SW_SHOW );
    UpdateWindow( ident );
    setFlag( WState_Visible );
    return TRUE;
}

bool QWidget::hide()				// hide widget
{
    if ( !testFlag( WState_Visible ) )
	return FALSE;
    ShowWindow( ident, SW_HIDE );
    clearFlag(WState_Visible);
    return TRUE;
}

bool QWidget::raise()				// raise widget
{
    BringWindowToTop( ident );
    return TRUE;
}

bool QWidget::lower()				// lower widget
{
#if defined(DEBUG)
    warning( "QWidget::lower: Function not implemented for Windows" );
#endif
    return FALSE;
}


//
// The internal qWinRequestConfig, defined in qapp_win.C, stores move,
// resize and changeGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig( WId, int, int, int, int, int );

bool QWidget::move( int x, int y )		// move widget
{
    if ( testFlag( WWin_Config ) )		// processing config event
	qWinRequestConfig( ident, 0, x, y, 0, 0 );
    else {
	if ( !testFlag( WState_Visible ) )
	    setNCRect( QRect(x,y,ncrect.width(),ncrect.height()) );
	setFlag( WWin_Config );
	MoveWindow( ident, x, y, ncrect.width(), ncrect.height(), TRUE );
	clearFlag( WWin_Config );
    }
    return TRUE;
}

bool QWidget::resize( int w, int h )		// resize widget
{
    if ( testFlag( WWin_Config ) )		// processing config event
	qWinRequestConfig( ident, 1, 0, 0, w, h );
    else {
	if ( !testFlag( WState_Visible ) )
	    setNCRect( QRect(ncrect.left(),ncrect.top(),w,h) );
	setFlag( WWin_Config );
	MoveWindow( ident, ncrect.left(), ncrect.top(), w, h, TRUE );
	clearFlag( WWin_Config );
    }
    return TRUE;
}

bool QWidget::changeGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( testFlag( WWin_Config ) )		// processing config event
	qWinRequestConfig( ident, 2, x, y, w, h );
    else {
	if ( !testFlag( WState_Visible ) )
	    setNCRect( QRect(x,y,w,h) );
	setFlag( WWin_Config );
	MoveWindow( ident, x, y, w, h, TRUE );
	clearFlag( WWin_Config );
    }
    return TRUE;
}


bool QWidget::erase()				// erase widget contents
{
    HDC h = hdc;
    if ( !hdc )
	h = GetDC( ident );
    SendMessage( ident, WM_ERASEBKGND, (WPARAM)h, 0 );
    if ( !hdc )
	ReleaseDC( ident, h );
    return TRUE;
}

bool QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    ScrollWindow( ident, dx, dy, 0, 0 );
    return TRUE;
}


bool QWidget::drawText( int x, int y, const char *str )
{						// draw text in widget
    bool tmp_hdc = hdc == 0;
    int	 bgm;
    COLORREF bgc, txc;
    uint ta;
    if ( tmp_hdc )				// create new hdc
	hdc = GetDC( ident );
    else {					// save painter settings
	bgm = GetBkMode( hdc );
	bgc = GetBkColor( hdc );
	txc = GetTextColor( hdc );
	ta = GetTextAlign( hdc );
    }
    SetBkMode( hdc, TRANSPARENT );		// default settings
    SetBkColor( hdc, backgroundColor().pixel() );
    SetTextColor( hdc, foregroundColor().pixel() );
    SetTextAlign( hdc, TA_BASELINE );
    TextOut( hdc, x, y, str, lstrlen(str) );	// print text
    if ( tmp_hdc ) {
	ReleaseDC( ident, hdc );
	hdc = 0;
    }
    else {					// restore painter settings
	SetBkMode( hdc, bgm );
	SetBkColor( hdc, bgc );
	SetTextColor( hdc, txc );
	SetTextAlign( hdc, ta );
    }
    return TRUE;
}


// --------------------------------------------------------------------------
// QView member functions
//

void QView::setCaption( const char *s )			// set caption text
{
    ctext = s;
    SetWindowText( id(), (const char *)ctext );
}

void QView::setIconText( const char *s )		// set icon text
{
    itext = s;
}
