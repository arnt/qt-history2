/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_win.cpp#9 $
**
** Implementation of QWidget and QWindow classes for Windows
**
** Author  : Haavard Nord
** Created : 931205
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qobjcoll.h"
#include "qpaintdc.h"
#include "qpainter.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget_win.cpp#9 $";
#endif


const char *qt_reg_winclass( int type );	// defined in qapp_x11.cpp
void	    qt_enter_modal( QWidget * );
void	    qt_leave_modal( QWidget * );
bool	    qt_modal_state();
void	    qt_open_popup( QWidget * );
void	    qt_close_popup( QWidget * );


// --------------------------------------------------------------------------
// QWidget member functions
//

bool QWidget::create()				// create widget
{
    if ( testWFlags(WState_Created) )		// already created
	return FALSE;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_Overlap );		// overlapping widget

    QWidget *parent = parentWidget();
    bool   overlap = testWFlags( WType_Overlap );
    bool   popup   = testWFlags( WType_Popup );
    bool   modal   = testWFlags( WType_Modal );
    bool   desktop = testWFlags( WType_Desktop );
    HANDLE parentwin = parentWidget() ? parentWidget()->id() : 0;
    const char *wcln = qt_reg_winclass( popup ? 1 : 0 );
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
	crect = frect;
	overlap = popup = FALSE;		// force these flags off
    }

    char *title = 0;
    DWORD style = WS_CHILD | WS_CLIPSIBLINGS;
    if ( popup )
	style = WS_POPUP | WS_CLIPSIBLINGS;
    else if ( modal )
	style = WS_DLGFRAME;
    else if ( overlap ) {
	style = WS_OVERLAPPEDWINDOW;
	setWFlags(WStyle_Border);
	setWFlags(WStyle_Title);
	setWFlags(WStyle_Close);
	setWFlags(WStyle_Resize);
	setWFlags(WStyle_MinMax);
    }
    if ( !desktop )
	style |= WS_CLIPCHILDREN;
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
	id = CreateWindow( wcln, title, style,
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
	id = CreateWindow( wcln, title, style,
			   x, y, w, h,
			   parentwin, NULL, qWinAppInst(), NULL );
	set_id( id );
    }

    if ( !desktop ) {
	RECT  fr, cr;
	POINT pt;
	GetWindowRect( id, &fr );		// update rects
	GetClientRect( id, &cr );
	frect = QRect( QPoint(fr.left,	fr.top),
		       QPoint(fr.right, fr.bottom) );
	pt.x = 0;
	pt.y = 0;
	ClientToScreen( id, &pt );
	crect = QRect( QPoint(pt.x+cr.left,  pt.y+cr.top),
		       QPoint(pt.x+cr.right, pt.y+cr.bottom) );
	setCursor( arrowCursor );		// default cursor
    }

    hdc = 0;					// no display context

    return TRUE;
}

bool QWidget::destroy()				// destroy widget
{
    if ( testWFlags(WState_Created) ) {
	clearWFlags( WState_Created );
	DestroyWindow( id() );
	set_id( 0 );
    }
    return TRUE;
}


void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    update();
}

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{						// set background pixmap
}


void QWidget::setCursor( const QCursor &c )	// set cursor
{
    ((QCursor*)&c)->handle();
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
	InvalidateRect( id(), 0, TRUE );
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
    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {				// show all widget children
	    object = it.current();		//   (except popups)
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->testWFlags(WExplicitHide) )
		    widget->show();
	    }
	    ++it;
	}
    }
    ShowWindow( id(), SW_SHOW );
    UpdateWindow( id() );
    setWFlags( WState_Visible );
    clearWFlags( WExplicitHide );
    if ( testWFlags(WType_Modal) )
	qt_enter_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_open_popup( this );
}

void QWidget::hide()				// hide widget
{
    setWFlags( WExplicitHide );
    if ( !testWFlags(WState_Visible) )		// not visible
	return;
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testWFlags(WType_Modal) )
	qt_leave_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_close_popup( this );
    ShowWindow( id(), SW_HIDE );
    clearWFlags( WState_Visible );
}


void QWidget::raise()				// raise widget
{
    BringWindowToTop( id() );
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
    if ( testWFlags(WConfigPending) )		// processing config event
	qWinRequestConfig( id(), 0, x, y, 0, 0 );
    else {
	if ( !testWFlags(WState_Visible) )
	    setFRect( QRect(x,y,frect.width(),frect.height()) );
	setWFlags( WConfigPending );
	MoveWindow( id(), x, y, frect.width(), frect.height(), TRUE );
	clearWFlags( WConfigPending );
    }
}

void QWidget::resize( int w, int h )		// resize widget
{
    if ( testWFlags(WConfigPending) )		// processing config event
	qWinRequestConfig( id(), 1, 0, 0, w, h );
    else {
	int x = frect.x();
	int y = frect.y();
	w += frect.width()  - crect.width();
	h += frect.height() - crect.height();
	if ( !testWFlags(WState_Visible) )
	    setFRect( QRect(x,y,w,h) );
	setWFlags( WConfigPending );
	MoveWindow( id(), x, y, w, h, TRUE );
	clearWFlags( WConfigPending );
    }
}

void QWidget::setGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( testWFlags(WConfigPending) )		// processing config event
	qWinRequestConfig( id(), 2, x, y, w, h );
    else {
	if ( !testWFlags(WState_Visible) )
	    setFRect( QRect(x,y,w,h) );
	setWFlags( WConfigPending );
	MoveWindow( id(), x, y, w, h, TRUE );
	clearWFlags( WConfigPending );
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


void QWidget::erase( int x, int y, int w, int h )
{
    HDC tmphdc;
    if ( hdc )
	tmphdc = hdc;
    else
	tmphdc = GetDC( id() );
#error This has to be implemented
    if ( bg_pm )
	SendMessage( id(), WM_ERASEBKGND, (WPARAM)tmphdc, (LPARAM)&l );
    if ( !hdc )
	ReleaseDC( id(), h );
}

void QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    ScrollWindow( id(), dx, dy, 0, 0 );
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
	    // !!!hanord: return widget mm width/height
	    case PDM_WIDTHMM:
		val = GetDeviceCaps( gdc, HORZSIZE );
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
