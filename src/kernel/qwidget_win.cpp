/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_win.cpp#6 $
**
** Implementation of QWidget and QWindow classes for Windows + NT
**
** Author  : Haavard Nord
** Created : 931205
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qcolor.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget_win.cpp#6 $";
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
    if ( testWFlags(WState_Created) )		// already created
	return FALSE;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_Overlap );		// overlapping widget

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
    int	   sw;					// screen width
    int	   sh;					// screen height
    bool   overlap = testWFlags( WType_Overlap );
    bool   popup   = testWFlags( WType_Popup );
    bool   modal   = testWFlags( WType_Modal );
    bool   desktop = testWFlags( WType_Desktop );
    HANDLE parentwin = parentWidget() ? parentWidget()->id() : 0;
    WId	   id;

    bg_col = pal.normal().background();		// default background color

    if ( modal ) {				// modal windows overlap
	overlap = TRUE;
	setWFlags( WType_Overlap );
    }

    if ( desktop ) {				// desktop widget
	int sw = GetSystemMetrics( SM_CXSCREEN );
	int sh = GetSystemMetrics( SM_CYSCREEN );
	frect.setRect( 0, 0, sw, sh );
	overlap = popup = FALSE;		// force these flags off
    }

    char *title = 0;
    DWORD style = overlap ? 0 : (WS_CHILD | WS_CLIPSIBLINGS);

    if ( testWFlags(WType_Modal) && !testWFlags(WStyle_Title) )
	 style |= WS_DLGFRAME;
    if ( !testWFlags(WType_Modal) && overlap ) {
	style |= WS_OVERLAPPEDWINDOW;
	setWFlags(WStyle_Border);
	setWFlags(WStyle_Title);
	setWFlags(WStyle_Close);
	setWFlags(WStyle_Resize);
	setWFlags(WStyle_MinMax);
    }
    else {
	if ( testWFlags(WStyle_Border) )
	    style |= WS_BORDER;
	if ( testWFlags(WStyle_Title) )
	    style |= WS_CAPTION;
	if ( testWFlags(WStyle_Close) )
	    style |= WS_SYSMENU;
	if ( testWFlags(WStyle_Resize) )
	    style |= WS_THICKFRAME;
	if ( testWFlags(WStyle_Minimize) )
	    style |= WS_MINIMIZEBOX;
	if ( testWFlags(WStyle_Maximize) )
	    style |= WS_MAXIMIZEBOX;
    }
    if ( testWFlags(WStyle_Title) )
	title = qAppName();

    if ( desktop ) {				// desktop widget
	id = GetDesktopWindow();
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->set_id( 0 );		// remove id from widget mapper
	    set_id( id );			// make sure otherDesktop is
	    otherDesktop->set_id( id );		//   found first
	}
	else
	    set_id( id );
    }
    else if ( overlap ) {			// create overlapped widget
	id = CreateWindow( qWidgetClassName, title, style,
			   CW_USEDEFAULT, CW_USEDEFAULT,
			   CW_USEDEFAULT, CW_USEDEFAULT,
			   parentwin, 0,
			   qWinAppInst(), NULL );
	set_id( id );
    }
    else {					// create child widget
	int x, y, w, h;
	x = y = 10;
	w = h = 40;
	id = CreateWindow( qWidgetClassName, title, style,
			   x, y, w, h,
			   parentwin, NULL, qWinAppInst(), NULL );
	set_id( id );
    }

    if ( !desktop ) {
	RECT cr, ncr;
	POINT pt;
	GetClientRect( id, &cr );
	GetWindowRect( id, &ncr );		// update rects
	ncrect = QRect( QPoint(ncr.left,  ncr.top),
			QPoint(ncr.right, ncr.bottom) );
	pt.x = 0;
	pt.y = 0;
	ClientToScreen( id, &pt );
	crect = QRect( QPoint(pt.x+cr.left,  pt.y+cr.top),
		       QPoint(pt.x+cr.right, pt.y+cr.bottom) );
    }

    hdc = 0;					// no display context
    setCursor( arrowCursor );			// default cursor

    return TRUE;
}

bool QWidget::destroy()				// destroy widget
{
    if ( testWFlags( WState_Created ) ) {
	clearWFlags( WState_Created );
	DestroyWindow( ident );
	set_id( 0 );
    }
    return TRUE;
}


void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    update();
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


void QWidget::setFocus()			// set keyboard input focus
{
    QWidget *oldFocus = qApp->focusWidget();
    if ( this == oldFocus )			// has already focus
	return;
    if ( !acceptFocus() )			// cannot take focus
	return;
    if ( oldFocus ) {				// goodbye to old focus widget
	qApp->focus_widget = 0;
	QFocusEvent out( Event_FocusOut );
	QApplication::sendEvent( oldFocus, &out );
    }
    QWidget *top, *w, *p;
    top = this;
    while ( top->parentWidget() )		// find top level widget
	top = top->parentWidget();
    w = top;
    while ( w->focusChild )			// reset focus chain
	w = w->focusChild;
    w = w->parentWidget();
    while ( w ) {
	w->focusChild = 0;
	w = w->parentWidget();
    }
    w = this;
    while ( (p=w->parentWidget()) ) {		// build new focus chain
	p->focusChild = w;
	w = p;
    }
    qApp->focus_widget = this;
    QFocusEvent in( Event_FocusIn );
    QApplication::sendEvent( this, &in );
}

bool QWidget::focusNextChild()
{
    return TRUE;				// !!!TODO
}

bool QWidget::focusPrevChild()
{
    return TRUE;				// !!!TODO
}


bool QWidget::enableUpdates( bool enable )	// enable widget update/repaint
{
    bool last = !testWFlags( WNoUpdates );
    if ( enable )
	clearWFlags( WNoUpdates );
    else
	setWFlags( WNoUpdates );
    return last;
}

void QWidget::update()				// update widget
{
    if ( !testWFlags(WNoUpdates) )
	InvalidateRect( ident, 0, TRUE );
    return TRUE;
}

void QWidget::update( int x, int y, int w, int h )
{						// update part of widget
    if ( !testWFlags(WNoUpdates) ) {
	RECT rect;
	rect.left   = x;
	rect.top    = y;
	rect.right  = x + w;
	rect.bottom = y + h;
	InvalidateRect( id(), &rect, TRUE );
    }
}

void QWidget::repaint( const QRect &r, bool erase )
{
    if ( !isVisible() || testWFlags(WNoUpdates) ) // ignore repaint
	return;
    QPaintEvent e( r );				// send fake paint event
    if ( erase ) {
	// !!! must erase only part of widget
	HDC h = hdc;
	if ( !hdc )
	    h = GetDC( id() );
	SendMessage( id(), WM_ERASEBKGND, (WPARAM)h, 0 );
	if ( !hdc )
	    ReleaseDC( id(), h );
    }
    QApplication::sendEvent( this, &e );
}


void QWidget::show()				// show widget
{
    if ( testWFlags(WState_Visible) )
	return;
    ShowWindow( ident, SW_SHOW );
    UpdateWindow( ident );
    setWFlags( WState_Visible );
}

void QWidget::hide()				// hide widget
{
    if ( !testWFlags(WState_Visible) )
	return;
    ShowWindow( ident, SW_HIDE );
    clearWFlags(WState_Visible);
}

void QWidget::raise()				// raise widget
{
    BringWindowToTop( ident );
}

void QWidget::lower()				// lower widget
{
#if defined(DEBUG)
    warning( "QWidget::lower: Function not implemented for Windows" );
#endif
}


//
// The internal qWinRequestConfig, defined in qapp_win.cpp, stores move,
// resize and setGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig( WId, int, int, int, int, int );

void QWidget::move( int x, int y )		// move widget
{
    if ( testWFlags(WWin_Config) )		// processing config event
	qWinRequestConfig( ident, 0, x, y, 0, 0 );
    else {
	if ( !testWFlags(WState_Visible) )
	    setNCRect( QRect(x,y,ncrect.width(),ncrect.height()) );
	setWFlags( WWin_Config );
	MoveWindow( ident, x, y, ncrect.width(), ncrect.height(), TRUE );
	clearWFlags( WWin_Config );
    }
}

void QWidget::resize( int w, int h )		// resize widget
{
    if ( testWFlags(WWin_Config) )		// processing config event
	qWinRequestConfig( ident, 1, 0, 0, w, h );
    else {
	if ( !testWFlags(WState_Visible) )
	    setNCRect( QRect(ncrect.left(),ncrect.top(),w,h) );
	setWFlags( WWin_Config );
	MoveWindow( ident, ncrect.left(), ncrect.top(), w, h, TRUE );
	clearWFlags( WWin_Config );
    }
}

void QWidget::setGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( testWFlags(WWin_Config) )		// processing config event
	qWinRequestConfig( ident, 2, x, y, w, h );
    else {
	if ( !testWFlags(WState_Visible) )
	    setNCRect( QRect(x,y,w,h) );
	setWFlags( WWin_Config );
	MoveWindow( ident, x, y, w, h, TRUE );
	clearWFlags( WWin_Config );
    }
}


void QWidget::setMinimumSize( int w, int h )
{
    // !!!TODO
}

void QWidget::setMaximumSize( int w, int h )
{
    // !!!TODO
}

void QWidget::setSizeIncrement( int w, int h )
{
}


void QWidget::erase()				// erase widget contents
{
    HDC h = hdc;
    if ( !hdc )
	h = GetDC( ident );
    SendMessage( ident, WM_ERASEBKGND, (WPARAM)h, 0 );
    if ( !hdc )
	ReleaseDC( ident, h );
}

void QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    ScrollWindow( ident, dx, dy, 0, 0 );
}


void QWidget::drawText( int x, int y, const char *str )
{
    if ( testWFlags(WState_Visible) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


long QWidget::metric( int m ) const		// return widget metrics
{
    long val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = crect.width();
	else
	    val = crect.height();
    }
    else {
	HDC gdc = GetDC( 0 );
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = GetDeviceCaps( gdc, HORSIZE );
		break;
	    case PDM_HEIGHTMM:
		val = GetDeviceCaps( gdc, VERTSIZE );
		break;
	    case PDM_NUMCOLORS:
		if ( GetDeviceCaps(gdc, RASTERCAPS) & RC_PALETTE )
		    val = GetDeviceCaps( gdc, SIZEPALETTE );
		else
		    val = GetDeviceCaps( gdc, NUMCOLORS );
		break;
	    case PDM_NUMPLANES:
		val = GetDeviceCaps( gdc, PLANES );
		break;
	    default:
		val = 0;
#if defined(CHECK_RANGE)
		warning( "QWidget::metric: Invalid metric command" );
#endif
	}
	ReleaseDC( 0, gdc );
 
    }
    return val;
}


// --------------------------------------------------------------------------
// QWindow member functions
//

void QWindow::setCaption( const char *s )		// set caption text
{
    ctext = s;
    SetWindowText( id(), (const char *)ctext );
}

void QWindow::setIconText( const char *s )		// set icon text
{
    itext = s;
}
