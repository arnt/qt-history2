/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_win.cpp#48 $
**
** Implementation of QWidget and QWindow classes for Win32
**
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

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qwidget_win.cpp#48 $");

extern "C" LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

#if !defined(WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW 0x00000080
#endif

const char *qt_reg_winclass( int type );	// defined in qapp_win.cpp
void	    qt_enter_modal( QWidget * );
void	    qt_leave_modal( QWidget * );
bool	    qt_modal_state();
void	    qt_open_popup( QWidget * );
void	    qt_close_popup( QWidget * );


extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QCursor *mouseGrbCur = 0;
static QWidget *keyboardGrb = 0;
static HANDLE	journalRec  = 0;


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


bool QWidget::create()
{
    if ( testWFlags(WState_Created) )		// already created
	return FALSE;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top-level widget

    static int sw = -1, sh = -1;

    bool   topLevel = testWFlags(WType_TopLevel);
    bool   popup    = testWFlags(WType_Popup);
    bool   tool	    = testWFlags(WType_Popup|WStyle_Tool);
    bool   modal    = testWFlags(WType_Modal);
    bool   desktop  = testWFlags(WType_Desktop);
    HANDLE appinst  = qWinAppInst();
    const char *wcln = qt_reg_winclass( tool ? 1 : 0 );
    HANDLE parentw;
    WId	   id;

    if ( popup )				// a popup is a tool window
	setWFlags(WStyle_Tool);

    if ( sw < 0 ) {				// get the screen size
	sw = GetSystemMetrics( SM_CXSCREEN );
	sh = GetSystemMetrics( SM_CYSCREEN );
    }

    bg_col = pal.normal().background();		// default background color

    if ( modal || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	frect.setRect( 0, 0, sw, sh );
	crect = frect;
	modal = popup = FALSE;			// force this flags off
    }

    parentw = topLevel ? 0 : parentWidget()->winId();

    char *title = 0;
    int	 style = WS_CHILD;
    int	 exsty = 0;

    if ( popup ) {
	style = WS_POPUP;
	exsty = WS_EX_TOOLWINDOW;
    } else if ( modal ) {
	style = WS_POPUP | WS_CAPTION;
    } else if ( topLevel && !desktop ) {
	if ( testWFlags(WStyle_Customize) ) {
	    if ( testWFlags(WStyle_NormalBorder|WStyle_DialogBorder) == 0 ) {
		style = WS_POPUP;		// no border
	    } else {
		style = 0;
	    }
	} else {
	    style = WS_OVERLAPPEDWINDOW;
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
		       WStyle_MinMax );
	}
    }
    if ( !desktop ) {
	style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if ( topLevel ) {
	    if ( testWFlags(WStyle_NormalBorder) )
		style |= WS_THICKFRAME;
	    else if ( testWFlags(WStyle_DialogBorder) )
		style |= WS_POPUP | WS_DLGFRAME;
	    if ( testWFlags(WStyle_Title) )
		style |= WS_CAPTION;
	    if ( testWFlags(WStyle_SysMenu) )
		style |= WS_SYSMENU | WS_CAPTION;
	    if ( testWFlags(WStyle_Minimize) )
		style |= WS_MINIMIZEBOX;
	    if ( testWFlags(WStyle_Maximize) )
		style |= WS_MAXIMIZEBOX;
	    if ( testWFlags(WStyle_Tool) )
		exsty |= WS_EX_TOOLWINDOW;
	}
    }
    if ( testWFlags(WStyle_Title) )
	title = qAppName();

    if ( desktop ) {				// desktop widget
	id = GetDesktopWindow();
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else if ( topLevel ) {			// create top-level widget
	if ( exsty )
	    id = CreateWindowEx( exsty, wcln, title, style,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 parentw, 0, appinst, 0 );
	else
	    id = CreateWindow(	 wcln, title, style,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 parentw, 0, appinst, 0 );
	setWinId( id );
	if ( tool )
	    SetWindowPos( id, HWND_TOPMOST, 0, 0, 100, 100, SWP_NOACTIVATE );
    } else {					// create child widget
	id = CreateWindow( wcln, title, style, 0, 0, 100, 30,
			   parentw, NULL, appinst, NULL );
	SetWindowPos( id, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	setWinId( id );
    }

    if ( desktop ) {
	setWFlags( WState_Visible );
    } else {
	RECT  fr, cr;
	POINT pt;
	GetWindowRect( id, &fr );		// update rects
	GetClientRect( id, &cr );
	if ( cr.top == cr.bottom && cr.left == cr.right ) {
	    int x, y, w, h;
	    if ( topLevel ) {
		x = sw/4;
		y = 3*sh/10;
		w = sw/2;
		h = 4*sh/10;
	    } else {
		x = y = 0;
		w = 100;
		h = 30;
	    }
	    MoveWindow( winId(), x, y, w, h, TRUE );
	    GetWindowRect( id, &fr );		// update rects
	    GetClientRect( id, &cr );
	}
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


bool QWidget::destroy()
{
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testWFlags(WState_Created) ) {
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
	if ( mouseGrb == this )
	    releaseMouse();
	if ( keyboardGrb == this )
	    releaseKeyboard();
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qt_close_popup( this );
	if ( !testWFlags(WType_Desktop) )
	    DestroyWindow( winId() );
	setWinId( 0 );
    }
    return TRUE;
}


void QWidget::recreate( QWidget *parent, WFlags, const QPoint &p,
			bool showIt )
{
    HANDLE np = parent ? parent->winId() : 0;
    hide();
    SetParent( winId(), np );
    move( p.x(), p.y() );
    if ( showIt )
	show();
}


QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    POINT p;
    p.x = pos.x();
    p.y = pos.y();
    ClientToScreen( winId(), &p );
    return QPoint( p.x, p.y );
}

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    POINT p;
    p.x = pos.x();
    p.y = pos.y();
    ScreenToClient( winId(), &p );
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
    backgroundColorChange( old );
}

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{
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
    backgroundPixmapChange( old );
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
    SetWindowText( winId(), extra->caption );
}

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra )
	delete extra->icon;
    else
	createExtra();
    extra->icon = new QPixmap( pixmap );
}

void QWidget::setIconText( const char *iconText )
{
    if ( extra )
	delete [] extra->iconText;
    else
	createExtra();
    extra->iconText = qstrdup( iconText );
}


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
	SetCapture( winId() );
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
	SetCapture( winId() );
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


bool QWidget::isActiveWindow() const
{
    HWND win = GetActiveWindow();
    QWidget *w = find( win );
    return w && w->topLevelWidget() == topLevelWidget();
}

void QWidget::setActiveWindow()
{
    SetActiveWindow( topLevelWidget()->winId() );
}


void QWidget::update()
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible )
	InvalidateRect( winId(), 0, TRUE );
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
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
	InvalidateRect( winId(), &r, TRUE );
    }
}


void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
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
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->testWFlags(WState_DoHide) )
		    widget->show();
	    }
	}
    }
    if ( testWFlags(WStyle_Tool) )
	SetWindowPos( winId(), 0,
		      frect.x(), frect.y(), crect.width(), crect.height(),
		      SWP_NOACTIVATE | SWP_SHOWWINDOW );
    else
	ShowWindow( winId(), SW_SHOW );
    UpdateWindow( winId() );
    setWFlags( WState_Visible );
    clearWFlags( WState_DoHide );
    if ( testWFlags(WType_Modal) )
	qt_enter_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_open_popup( this );
}

void QWidget::hide()
{
    if ( testWFlags(WFocusSet) )
	clearFocus();
    setWFlags( WState_DoHide );
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
    ShowWindow( winId(), SW_HIDE );
    clearWFlags( WState_Visible );
}

void QWidget::iconify()
{
    if ( testWFlags(WType_TopLevel) )
	ShowWindow( winId(), SW_SHOWMINIMIZED );
}


void QWidget::raise()
{
    SetWindowPos( winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}

void QWidget::lower()
{
    SetWindowPos( winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}


//
// The internal qWinRequestConfig, defined in qapp_win.cpp, stores move,
// resize and setGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig( WId, int, int, int, int, int );

void QWidget::move( int x, int y )
{
    if ( testWFlags(WConfigPending) ) {		// processing config event
	qWinRequestConfig( winId(), 0, x, y, 0, 0 );
    } else {
	setFRect( QRect(x,y,frect.width(),frect.height()) );
	setWFlags( WConfigPending );
	MoveWindow( winId(), x, y, frect.width(), frect.height(), TRUE );
	clearWFlags( WConfigPending );
    }
}

void QWidget::resize( int w, int h )
{
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( testWFlags(WConfigPending) ) {		// processing config event
	qWinRequestConfig( winId(), 1, 0, 0, w, h );
    } else {
	int x = frect.x();
	int y = frect.y();
	w += frect.width()  - crect.width();
	h += frect.height() - crect.height();
	setFRect( QRect(x,y,w,h) );
	setWFlags( WConfigPending );
	MoveWindow( winId(), x, y, w, h, TRUE );
	clearWFlags( WConfigPending );
    }
}

void QWidget::setGeometry( int x, int y, int w, int h )
{
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( testWFlags(WConfigPending) ) {		// processing config event
	qWinRequestConfig( winId(), 2, x, y, w, h );
    } else {
	w += frect.width()  - crect.width();
	h += frect.height() - crect.height();
	setFRect( QRect(x,y,w,h) );
	setWFlags( WConfigPending );
	MoveWindow( winId(), x, y, w, h, TRUE );
	clearWFlags( WConfigPending );
    }
}


void QWidget::setMinimumSize( int w, int h )
{
#if defined(CHECK_RANGE)
    if ( w < 0 || h < 0 )
	warning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    extra->minw = w;
    extra->minh = h;
    int minw = QMIN(w,crect.width());
    int minh = QMIN(h,crect.height());
    if ( minw < w || minh < h )
	resize( minw, minh );
}

void QWidget::setMaximumSize( int w, int h )
{
#if defined(CHECK_RANGE)
    if ( w > QCOORD_MAX || h > QCOORD_MAX )
	warning("QWidget::setMaximumSize: The largest allowed size is (%d,%d)",
		 QCOORD_MAX, QCOORD_MAX );
#endif
    createExtra();
    extra->maxw = w;
    extra->maxh = h;
    int maxw = QMAX(w,crect.width());
    int maxh = QMAX(h,crect.height());
    if ( maxw > w || maxh > h )
	resize( maxw, maxh );
}

void QWidget::setSizeIncrement( int w, int h )
{
    createExtra();
    extra->incw = w;
    extra->inch = h;
}


void QWidget::erase( int x, int y, int w, int h )
{
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

    bool     tmphdc;
    HBRUSH   brush;
    HPALETTE pal;

    if ( !hdc ) {
	tmphdc = TRUE;
	hdc = GetDC( winId() );
    } else {
	tmphdc = FALSE;
    }

    brush = CreateSolidBrush( bg_col.pixel() );
    if ( QColor::hPal() ) {
	pal = SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    } else {
	pal = 0;
    }
    FillRect( hdc, &r, brush );
    DeleteObject( brush );
    if ( tmphdc ) {
	ReleaseDC( winId(), hdc );
	hdc = 0;
    }
    else if ( pal )
	SelectPalette( hdc, pal, FALSE );
}


void QWidget::scroll( int dx, int dy )
{
    ScrollWindow( winId(), dx, dy, 0, 0 );
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


int QWidget::metric( int m ) const
{
    int val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = crect.width();
	else
	    val = crect.height();
    } else {
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
