/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwid_win.cpp#99 $
**
** Implementation of QWidget and QWindow classes for Win32
**
** Created : 931205
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qapp.h"
#include "qpaintdc.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidcoll.h"
#include "qobjcoll.h"
#include "qaccel.h"
#include "qimage.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qwid_win.cpp#99 $");


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

extern "C" LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWFlags(WState_Created) && window == 0 )
	return;
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
    HANDLE parentw, destroyw = 0;
    WId	   id;

    if ( !window )				// always initialize
	initializeWindow = TRUE;

    if ( popup )				// a popup is a tool window
	setWFlags(WStyle_Tool);

    if ( sw < 0 ) {				// get the screen size
	sw = GetSystemMetrics( SM_CXSCREEN );
	sh = GetSystemMetrics( SM_CYSCREEN );
    }

    if ( window ) {
	// There's no way we can know the background color of the
	// other window because it could be cleared using the WM_ERASEBKND
	// message.  Therefore we assume white.
	bg_col = white;
    } else {
	bg_col = pal.normal().background();	// default background color
    }

    if ( modal || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	modal = popup = FALSE;			// force this flags off
	frect.setRect( 0, 0, sw, sh );
	crect = frect;
    }

    parentw = topLevel ? 0 : parentWidget()->winId();

    char *title = 0;
    int	 style = WS_CHILD;
    int	 exsty = 0;

    if ( window ) {
	style = GetWindowLong( window, GWL_STYLE );
	topLevel = FALSE; // #### needed for some IE plugins??
    } else if ( popup ) {
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

	// The WState_Creates flag is checked by translateConfigEvent()
        // in qapp_win.cpp. We switch it off temporarily to avoid move
        // and resize events during creation
    clearWFlags( WState_Created );

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
	SetWindowLong( window, GWL_STYLE, style );
	SetWindowLong( window, GWL_WNDPROC, (LONG)WndProc );
    } else if ( desktop ) {			// desktop widget
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
	    if ( initializeWindow ) {
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
	    }
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
	if ( initializeWindow )
	    setCursor( arrowCursor );		// default cursor
    }

    setWFlags( WState_Created );		// accept move/resize events
    hdc = 0;					// no display context

    if ( window ) {				// got window from outside
	if ( IsWindowVisible(window) )
	    setWFlags( WState_Visible );
	else
	    clearWFlags( WState_Visible );
    }

    if ( destroyw ) {
	DestroyWindow( destroyw );
    }
}


void QWidget::create( WId window )
{
    create( window, TRUE, TRUE );
}


bool QWidget::create()
{
    create( 0, TRUE, TRUE );
    return TRUE;
}


void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    if ( testWFlags(WState_Created) ) {
	clearWFlags( WState_Created );
	if ( children() ) {
	    QObjectListIt it(*children());
	    register QObject *obj;
	    while ( (obj=it.current()) ) {	// destroy all widget children
		++it;
		if ( obj->isWidgetType() )
		    ((QWidget*)obj)->destroy(destroySubWindows,
					     destroySubWindows);
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
	if ( destroyWindow && !testWFlags(WType_Desktop) ) {
	    DestroyWindow( winId() );
	}
	setWinId( 0 );
    }
}


bool QWidget::destroy()
{
    destroy( TRUE, TRUE );
    return TRUE;
}


void QWidget::recreate( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );

    reparentFocusWidgets( parent );		// fix focus chains

    if ( parentObj ) {				// remove from parent
	QChildEvent e( Event_ChildRemoved, this );
	QApplication::sendEvent( parentObj, &e );
	parentObj->removeChild( this );
    }
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     enable = isEnabled();		// remember status
    QSize    s	    = size();
    QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    QColor   bgc    = bg_col;			// save colors
    const char* capt= caption();
    flags = f;
    clearWFlags( WState_Created | WState_Visible );
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {				// reparent children
	QObjectListIt it( *chlist );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() ) {
		QWidget *w = (QWidget *)obj;
		SetParent( w->winId(), winId() );
	    }
	    ++it;
	}
    }

    bg_col = bgc;
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    if ( capt ) {
	extra->caption = 0;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( old_winid )
	DestroyWindow( old_winid );

    QObjectList	*accelerators = queryList( "QAccel" );
    QObjectListIt it( *accelerators );
    QObject *obj;
    while ( (obj=it.current()) != 0 ) {
	++it;
	((QAccel*)obj)->repairEventFilter();
    }
    delete accelerators;
    if ( parent ) {
	QChildEvent *e = new QChildEvent( Event_ChildInserted, this );
	QApplication::postEvent( parent, e );
    }
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


void QWidget::setBackgroundColorDirect( const QColor &color )
{
    QColor old = bg_col;
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    backgroundColorChange( old );
}

void QWidget::setBackgroundColor( const QColor &color )
{
    setBackgroundModeDirect( FixedColor );
    setBackgroundColorDirect( color );
}


static int allow_null_pixmaps = 0;

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pixmap );
    }
    if (!allow_null_pixmaps) {
	setBackgroundModeDirect( FixedPixmap );
	backgroundPixmapChange( old );
    }
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setBackgroundPixmap(QPixmap());
    allow_null_pixmaps--;
}


extern void qt_set_cursor( QWidget *, QCursor * ); // qapp_win.cpp

void QWidget::setCursor( const QCursor &cursor )
{
    ((QCursor*)&cursor)->handle();
    curs = cursor;
    qt_set_cursor( this, &curs );
}


void QWidget::setCaption( const char *caption )
{
    if ( caption && extra && extra->caption &&
	 !strcmp( extra->caption, caption ) )
	return; // for less flicker
    if ( extra )
	delete [] extra->caption;
    else
	createExtra();
    extra->caption = qstrdup( caption );
    SetWindowText( winId(), extra->caption );
}


/*
  Create an icon mask the way Windows wants it using CreateBitmap.
*/

static HANDLE createIconMask( const QBitmap &bitmap )
{
    QImage bm = bitmap.convertToImage();
    int w = bm.width();
    int h = bm.height();
    int bpl = ((w+15)/16)*2;			// bpl, 16 bit alignment
    uchar *bits = new uchar[bpl*h];
    for ( int y=0; y<h; y++ )
	memcpy( bits+y*bpl, bm.scanLine(y), bpl );
    HANDLE hbm = CreateBitmap( w, h, 1, 1, bits );
    delete [] bits;
    return hbm;
}


void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra ) {
	delete extra->icon;
	extra->icon = 0;
	if ( extra->winIcon ) {
	    DestroyIcon( extra->winIcon );
	    extra->winIcon = 0;
	}
    } else {
	createExtra();
    }
    if ( !pixmap.isNull() ) {			// valid icon
	QPixmap pm;
	QBitmap mask;
	if ( pixmap.mask() ) {
	    pm.resize( pixmap.size() );
	    pm.fill( black );
	    bitBlt( &pm, 0, 0, &pixmap );	// make masked area black
	    mask = *pixmap.mask();
	} else  {
	    pm = pixmap;
	    mask.resize( pixmap.size() );
	    mask.fill( color1 );
	}
	HANDLE im = createIconMask(mask);
	ICONINFO ii;
	ii.fIcon    = TRUE;
	ii.hbmMask  = im;
	ii.hbmColor = pm.hbm();
	extra->icon = new QPixmap( pixmap );
	extra->winIcon = CreateIconIndirect( &ii );
	DeleteObject( im );
    }
    SendMessage( winId(), WM_SETICON, 0, /* ICON_SMALL */
		 (long)extra->winIcon );
    SendMessage( winId(), WM_SETICON, 1, /* ICON_BIG */
		 (long)extra->winIcon );
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


/*
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
    if ( testWFlags(WStyle_Tool) )
	SetWindowPos( winId(), 0,
		      frect.x(), frect.y(), crect.width(), crect.height(),
		      SWP_NOACTIVATE | SWP_SHOWWINDOW );
    else
	ShowWindow( winId(), SW_SHOW );
    setWFlags( WState_Visible );
    clearWFlags( WState_DoHide );
    UpdateWindow( winId() );
}


/*
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    ShowWindow( winId(), SW_HIDE );
}


void QWidget::iconify()
{
    if ( testWFlags(WType_TopLevel) )
	ShowWindow( winId(), SW_SHOWMINIMIZED );
}


void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    uint f = testWFlags(WStyle_Tool) ? SWP_NOACTIVATE : 0;
    SetWindowPos( winId(), HWND_TOP, 0, 0, 0, 0, f | SWP_NOMOVE | SWP_NOSIZE );
}

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    uint f = testWFlags(WStyle_Tool) ? SWP_NOACTIVATE : 0;
    SetWindowPos( winId(), HWND_BOTTOM, 0, 0, 0, 0, f | SWP_NOMOVE |
		  SWP_NOSIZE );
}


//
// The internal qWinRequestConfig, defined in qapp_win.cpp, stores move,
// resize and setGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig( WId, int, int, int, int, int );

void QWidget::move( int x, int y )
{
    QPoint oldp( pos() );
    if ( oldp.x() == x && oldp.y() == y )
	return;	
    if ( testWFlags(WConfigPending) ) {		// processing config event
	qWinRequestConfig( winId(), 0, x, y, 0, 0 );
    } else {
	setFRect( QRect(x,y,frect.width(),frect.height()) );
	if ( !isVisible() ) {
	    deferMove( oldp );
	} else {
	    cancelMove();
	}
	internalMove( x, y );
    }
}


void QWidget::internalMove( int x, int y )
{
    setWFlags( WConfigPending );
    MoveWindow( winId(), x, y, frect.width(), frect.height(), TRUE );
    clearWFlags( WConfigPending );
}


void QWidget::resize( int w, int h )
{
    if ( w == width() && h == height() )
	return;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( testWFlags(WConfigPending) ) {		// processing config event
	qWinRequestConfig( winId(), 1, 0, 0, w, h );
    } else {
	QSize olds( size() );
	int x = frect.x();
	int y = frect.y();
	w += frect.width()  - crect.width();
	h += frect.height() - crect.height();
	setFRect( QRect(x,y,w,h) );
	if ( !isVisible() ) {
	    deferResize( olds );
	} else {
	    cancelResize();
	}
	internalResize( width(), height() );
    }
}


void QWidget::internalResize( int w, int h )
{
    setWFlags( WConfigPending );
    w += frect.width()  - crect.width();
    h += frect.height() - crect.height();
    MoveWindow( winId(), frect.x(), frect.y(), w, h, TRUE );
    clearWFlags( WConfigPending );
}


void QWidget::setGeometry( int x, int y, int w, int h )
{
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    QPoint oldp( pos() );
    QSize  olds( size() );
    if ( oldp.x()==x && oldp.y()==y && olds.width()==w && olds.height()==h )
	return;
    if ( testWFlags(WConfigPending) ) {		// processing config event
	qWinRequestConfig( winId(), 2, x, y, w, h );
    } else {
	w += frect.width()  - crect.width();
	h += frect.height() - crect.height();
	setFRect( QRect(x,y,w,h) );
	if ( !isVisible() ) {
	    deferMove( oldp );
	    deferResize( olds );
	} else {
	    cancelMove();
	    cancelResize();
	}
	setWFlags( WConfigPending );
	internalSetGeometry( x, y, width(), height() );
	clearWFlags( WConfigPending );
    }
}


void QWidget::internalSetGeometry( int x, int y, int w, int h )
{
    setWFlags( WConfigPending );
    w += frect.width()  - crect.width();
    h += frect.height() - crect.height();
    MoveWindow( winId(), x, y, w, h, TRUE );
    clearWFlags( WConfigPending );
}


void QWidget::setMinimumSize( int minw, int minh )
{
#if defined(CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	warning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() )
	resize( QMAX(minw,width()), QMAX(minh,height()) );
    if ( parentWidget() ) {
	QEvent *e = new QEvent( Event_LayoutHint );
	QApplication::postEvent( parentWidget(), e );
    }

}

void QWidget::setMaximumSize( int maxw, int maxh )
{
#if defined(CHECK_RANGE)
    if ( maxw > QCOORD_MAX || maxh > QCOORD_MAX )
	warning("QWidget::setMaximumSize: The largest allowed size is (%d,%d)",
		 QCOORD_MAX, QCOORD_MAX );
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() )
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
    if ( parentWidget() ) {
	QEvent *e = new QEvent( Event_LayoutHint );
	QApplication::postEvent( parentWidget(), e );
    }
}

void QWidget::setSizeIncrement( int w, int h )
{
    createExtra();
    extra->incw = w;
    extra->inch = h;
}


void QWidget::erase( int x, int y, int w, int h )
{
    if ( backgroundMode()==NoBackground )
	return;
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
    } else if ( pal ) {
	SelectPalette( hdc, pal, FALSE );
    }
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


void QWidget::registerDropType( const char * /* mimeType */ )
{
    // nothing
}
