/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_win.cpp#26 $
**
** Implementation of QWidget and QWindow classes for Windows
**
** Author  : Haavard Nord
** Created : 931205
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qapp.h"
#include "qpaintdc.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qwidcoll.h"
#include "qobjcoll.h"
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qwidget_win.cpp#26 $")


const char *qt_reg_winclass( int type );	// defined in qapp_win.cpp
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
    DWORD style = WS_CHILD;
    if ( popup )
	style = WS_POPUP;
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
	style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
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
	if ( modal )
	    id = CreateWindowEx( WS_EX_DLGMODALFRAME, wcln, title, style,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 parentwin, 0,
				 qWinAppInst(), 0 );
	else
	    id = CreateWindow(	 wcln, title, style,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 parentwin, 0,
				 qWinAppInst(), 0 );
	set_id( id );
	if ( popup ) {
	    SetWindowPos( id, HWND_TOPMOST, 0, 0, 100, 100,
			  SWP_NOACTIVATE );
	}
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
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testWFlags(WState_Created) ) {
	emit destroyed();			// send out destroyed signal
	clearWFlags( WState_Created );
	focusChild = 0;
	if ( children() ) {
	    QObjectListIt it(*children());
	    register QObject *obj;
	    while ( (obj=it.current()) ) {	// destroy all widget children
		++it;
		if ( obj->isWidgetType() )
		    ((QWidget*)obj)->destroy();
	    }
	}
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qt_close_popup( this );
	if ( !testWFlags(WType_Desktop) )
	    DestroyWindow( id() );
	set_id( 0 );
    }
    return TRUE;
}


void QWidget::recreate( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    debug( "QWidget::recreate: Not implemented" );
}


QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    POINT p;
    p.x = pos.x();
    p.y = pos.y();
    ClientToScreen( id(), &p );
    return QPoint( p.x, p.y );
}

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    POINT p;
    p.x = pos.x();
    p.y = pos.y();
    ScreenToClient( id(), &p );
    return QPoint( p.x, p.y );
}


void QWidget::setBackgroundColor( const QColor &color )
{
    QColor old = bg_col;
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    if ( backgroundColorChange(old) )
	repaint();
}

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{
    debug( "QWidget::setBackgroundPixmap: Not implemented" );
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( pixmap.isNull() ) {
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    }
    else {
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pixmap );
    }
    if ( backgroundPixmapChange(old) )
	repaint();
}


void QWidget::setCursor( const QCursor &cursor )
{
    ((QCursor*)&cursor)->handle();
    curs = cursor;
}


void QWidget::setCaption( const char *caption )
{
    if ( extra )
	delete [] extra->caption;
    else
	createExtra();
    extra->caption = qstrdup( caption );
    SetWindowText( id(), extra->caption );
}

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra )
	delete extra->icon;
    else
	createExtra();
    extra->icon = new QPixmap( pixmap );
    warning( "QWidget::setIcon: Not implemented" );
}

void QWidget::setIconText( const char *iconText )
{
    if ( extra )
	delete [] extra->iconText;
    else
	createExtra();
    extra->iconText = qstrdup( iconText );
}


extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QCursor *mouseGrbCur = 0;
static QWidget *keyboardGrb = 0;
static HANDLE	journalRec  = 0;

QCursor *qt_grab_cursor()
{
    return mouseGrbCur;
}



LRESULT CALLBACK qJournalRecordProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    return CallNextHookEx( journalRec, nCode, wParam, lParam );
}

void QWidget::grabMouse()
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	journalRec = SetWindowsHookEx( WH_JOURNALRECORD,
				       (HOOKPROC)qJournalRecordProc,
				       GetModuleHandle(0), 0 );
	SetCapture( id() );
	mouseGrb = this;
    }
}

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	journalRec = SetWindowsHookEx( WH_JOURNALRECORD,
				       (HOOKPROC)qJournalRecordProc,
				       GetModuleHandle(0), 0 );
	SetCapture( id() );
	mouseGrbCur = new QCursor( cursor );
	SetCursor( mouseGrbCur->handle() );
	mouseGrb = this;
    }
}

void QWidget::releaseMouse()
{
    if ( !qt_nograb() && mouseGrb == this ) {
	ReleaseCapture();
	if ( journalRec ) {
	    UnhookWindowsHookEx( journalRec );
	    journalRec = 0;
	}
	if ( mouseGrbCur ) {
	    delete mouseGrbCur;
	    mouseGrbCur = 0;
	}
	mouseGrb = 0;
    }
}

void QWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
	if ( keyboardGrb )
	    keyboardGrb->releaseKeyboard();
	keyboardGrb = this;
    }
}
    
void QWidget::releaseKeyboard()
{
    if ( !qt_nograb() && keyboardGrb == this )
	keyboardGrb = 0;
}


QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}


void QWidget::setActiveWindow()
{
    SetActiveWindow( topLevelWidget()->id() );
}


void QWidget::setFocus()
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


bool QWidget::enableUpdates( bool enable )
{
    bool last = !testWFlags( WState_NoUpdates );
    if ( enable )
	clearWFlags( WState_NoUpdates );
    else
	setWFlags( WState_NoUpdates );
    return last;
}


void QWidget::update()
{
    if ( !testWFlags(WState_NoUpdates) )
	InvalidateRect( id(), 0, TRUE );
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( !testWFlags(WNoUpdates) ) {
	RECT r;
	r.left = x;
	r.top  = y;
	if ( w < 0 )
	    r.right = crect.width();
	else
	    rect.right = x + w;
	if ( h < 0 )
	    r.bottom = crect.height();
	else
	    r.bottom = y + h;
	InvalidateRect( id(), &r, TRUE );
    }
}


void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( !testWFlags(WNoUpdates) ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QPaintEvent e( QRect(x,y,w,h) );
	if ( erase )
	    this->erase( x, y, w, h );
	QApplication::sendEvent( this, &e );
    }
}


void QWidget::show()
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
    if ( testWFlags(WType_Popup) )
	SetWindowPos( id(), 0,
		      frect.x(), frect.y(), crect.width(), crect.height(),
		      SWP_NOACTIVATE | SWP_SHOWWINDOW );
    else
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


void QWidget::raise()
{
    SetWindowPos( id(), HWND_TOP, 0, 0, 0, 0,
		  SWP_NOMOVE | SWP_NOSIZE );
}

void QWidget::lower()
{
    SetWindowPos( id(), HWND_BOTTOM, 0, 0, 0, 0,
		  SWP_NOMOVE | SWP_NOSIZE );
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
    createExtra();
    extra->minw = w;
    extra->minh = h;
}

void QWidget::setMaximumSize( int w, int h )
{
    createExtra();
    extra->maxw = w;
    extra->maxh = h;
}

void QWidget::setSizeIncrement( int w, int h )
{
    createExtra();
    extra->incw = w;
    extra->inch = h;
}


void QWidget::erase( int x, int y, int w, int h )
{
    HDC tmphdc;
    if ( hdc )
	tmphdc = hdc;
    else
	tmphdc = GetDC( id() );
    HANDLE hbrush = CreateSolidBrush( bg_col.pixel() );
    RECT r;
    r.left = x;
    r.top  = y;
    if ( w < 0 )
	r.right = crect.width();
    else
	r.right = x + w;
    if ( h < 0 )
	r.bottom = crect.height();
    else
	r.bottom = y + h;
    FillRect( tmphdc, &r, hbrush );
    DeleteObject( hbrush );
    if ( !hdc )
	ReleaseDC( id(), tmphdc );
}

void QWidget::scroll( int dx, int dy )
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
	    case PDM_DEPTH:
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
