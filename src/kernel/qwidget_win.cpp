/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_win.cpp#182 $
**
** Implementation of QWidget and QWindow classes for Win32
**
** Created : 931205
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qapplication.h"
#include "qpaintdevicedefs.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qaccel.h"
#include "qimage.h"
#include "qfocusdata.h"
#include "qlayout.h"
#include "qt_windows.h"


#if !defined(WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW 0x00000080
#endif

const char* qt_reg_winclass( int );		// defined in qapplication_win.cpp
void	    qt_enter_modal( QWidget * );
void	    qt_leave_modal( QWidget * );
bool	    qt_modal_state();
void	    qt_olednd_unregister( QWidget* widget, QOleDropTarget *dst ); // dnd_win
QOleDropTarget* qt_olednd_register( QWidget* widget );


extern bool qt_nograb();
extern HRGN qt_win_bitmapToRegion(const QBitmap& bitmap);

static QWidget *mouseGrb    = 0;
static QCursor *mouseGrbCur = 0;
static QWidget *keyboardGrb = 0;
static HHOOK	journalRec  = 0;

extern "C" LRESULT CALLBACK QtWndProc( HWND, UINT, WPARAM, LPARAM );


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWState(WState_Created) && window == 0 )
	return;
    setWState( WState_Created );			// set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top-level widget

    static int sw = -1, sh = -1;

    bool   topLevel = testWFlags(WType_TopLevel);
    bool   popup    = testWFlags(WType_Popup);
    bool   tool	    = testWFlags(WType_Popup|WStyle_Tool);
    bool   modal    = testWFlags(WType_Modal);
    bool   desktop  = testWFlags(WType_Desktop);
    HINSTANCE appinst  = qWinAppInst();
    HWND   parentw, destroyw = 0;
    WId	   id;

    const char *windowClassName = qt_reg_winclass( getWFlags() );

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
	fpos = QPoint(0, 0);
	crect = QRect( fpos, QSize(sw, sh) );
    }

    parentw = parentWidget() ? parentWidget()->winId() : 0;

    const char* title = 0;
    int	 style = WS_CHILD;
    int	 exsty = 0;

    if ( window ) {
	style = GetWindowLong( window, GWL_STYLE );
	topLevel = FALSE; // #### needed for some IE plugins??
    } else if ( popup ) {
	style = WS_POPUP;
    }
    else if (topLevel && !desktop ) {
	if ( testWFlags(WStyle_Customize) ) {
	    if ( testWFlags(WStyle_NormalBorder|WStyle_DialogBorder) == 0 ) {
		style = WS_POPUP;		// no border
	    } else {
		style = 0;
	    }
	} else {
	    style = WS_OVERLAPPED;
	    if ( !modal )
		setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );
	    else
		setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu  );
	}
    }
    if ( !desktop ) {
	style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
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

	// The WState_Created flag is checked by translateConfigEvent() in
        // qapplication_win.cpp. We switch it off temporarily to avoid move
        // and resize events during creation
    clearWState( WState_Created );

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
	SetWindowLong( window, GWL_STYLE, style );
	SetWindowLong( window, GWL_WNDPROC, (LONG)QtWndProc );
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
	// WWA: I cannot get the Unicode versions to work.
	if ( exsty )
	    id = CreateWindowExA( exsty, windowClassName, title, style,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 parentw, 0, appinst, 0 );
	else
	    id = CreateWindowA(	 windowClassName, title, style,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 parentw, 0, appinst, 0 );
	setWinId( id );
	if ( tool || ( !parentw && testWFlags( WStyle_StaysOnTop) ) )
	    SetWindowPos( id, HWND_TOPMOST, 0, 0, 100, 100, SWP_NOACTIVATE );
    } else {					// create child widget
	// WWA: I cannot get the Unicode versions to work.
	id = CreateWindowA( windowClassName, title, style, 0, 0, 100, 30,
			   parentw, NULL, appinst, NULL );
	SetWindowPos( id, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	setWinId( id );
    }

    if ( desktop ) {
	setWState( WState_Visible );
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
	fpos = QPoint(fr.left,fr.top);
	if ( fr.top == cr.top &&
	     fr.left == cr.left &&
	     fr.bottom == cr.bottom &&
	     fr.right == cr.right ) {
	} else {
	    createTLExtra();
	    setFRect( QRect( QPoint(fr.left,fr.top),
		      QPoint(fr.right,fr.bottom) ) );
	}
	pt.x = 0;
	pt.y = 0;
	ClientToScreen( id, &pt );
	crect = QRect( QPoint(pt.x+cr.left,  pt.y+cr.top),
		       QPoint(pt.x+cr.right, pt.y+cr.bottom) );
    }

    setWState( WState_Created );			// accept move/resize events
    hdc = 0;					// no display context

    if ( window ) {				// got window from outside
	if ( IsWindowVisible(window) )
	    setWState( WState_Visible );
	else
	    clearWState( WState_Visible );
    }

    if ( destroyw ) {
	DestroyWindow( destroyw );
    }
}


void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    if ( testWState(WState_Created) ) {
	clearWState( WState_Created );
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
	    qApp->closePopup( this );
	if ( destroyWindow && !testWFlags(WType_Desktop) ) {
	    DestroyWindow( winId() );
	}
	setWinId( 0 );
    }
}


void QWidget::reparent( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );

    reparentFocusWidgets( parent );		// fix focus chains

    if ( parentObj ) {				// remove from parent
	parentObj->removeChild( this );
    }
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    } else {
	qApp->noteTopLevel(this);
    }
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QColor   bgc    = bg_col;			// save colors
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible );
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
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
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
    if ( !parent ) {
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
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


void QWidget::setFontSys()
{
}

void QWidget::setMicroFocusHint(int x, int y, int width, int height)
{
}

void QWidget::setSizeGrip(bool /* sizegrip */)
{
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


static int allow_null_pixmaps = 0;


void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
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
	backgroundPixmapChange( old );
    }
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setBackgroundPixmap(QPixmap());
    allow_null_pixmaps--;
}


extern void qt_set_cursor( QWidget *, const QCursor & ); // qapplication_win.cpp

void QWidget::setCursor( const QCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle()
	 || (extra && extra->curs) ) {
	createExtra();
	extra->curs = new QCursor(cursor);
    }
    setWState( WState_OwnCursor );
    qt_set_cursor( this, QWidget::cursor() );
}

void QWidget::unsetCursor()
{
    if ( !isTopLevel() ) {
	if (extra ) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	clearWState( WState_OwnCursor );
	qt_set_cursor( this, cursor() );
    }
}


void QWidget::setCaption( const QString &caption )
{
    if ( extra && extra->topextra && extra->topextra->caption == caption )
	return; // for less flicker
    createExtra();
    extra->topextra->caption = caption;
    SetWindowText( winId(), (TCHAR*)qt_winTchar(caption,TRUE) );
}


/*
  Create an icon mask the way Windows wants it using CreateBitmap.
*/

static HBITMAP createIconMask( const QBitmap &bitmap )
{
    QImage bm = bitmap.convertToImage();
    int w = bm.width();
    int h = bm.height();
    int bpl = ((w+15)/16)*2;			// bpl, 16 bit alignment
    uchar *bits = new uchar[bpl*h];
    for ( int y=0; y<h; y++ )
	memcpy( bits+y*bpl, bm.scanLine(y), bpl );
    HBITMAP hbm = CreateBitmap( w, h, 1, 1, bits );
    delete [] bits;
    return hbm;
}


void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
	if ( extra->winIcon ) {
	    DestroyIcon( extra->winIcon );
	    extra->winIcon = 0;
	}
    } else {
	createTLExtra();
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
	HBITMAP im = createIconMask(mask);
	ICONINFO ii;
	ii.fIcon    = TRUE;
	ii.hbmMask  = im;
	ii.hbmColor = pm.hbm();
	extra->topextra->icon = new QPixmap( pixmap );
	extra->winIcon = CreateIconIndirect( &ii );
	DeleteObject( im );
    }
    SendMessage( winId(), WM_SETICON, 0, /* ICON_SMALL */
		 (long)extra->winIcon );
    SendMessage( winId(), WM_SETICON, 1, /* ICON_BIG */
		 (long)extra->winIcon );
}


void QWidget::setIconText( const QString &iconText )
{
    createTLExtra();
    extra->topextra->iconText = iconText;
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
     return topLevelWidget() == qApp->activeWindow();
//    HWND win = GetActiveWindow();
//     QWidget *w = find( win );
//     return w && w->topLevelWidget() == topLevelWidget();
}

void QWidget::setActiveWindow()
{
    SetActiveWindow( topLevelWidget()->winId() );
}


void QWidget::update()
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible )
	InvalidateRect( winId(), 0, TRUE );
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( w && h &&
	 (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
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
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
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

void QWidget::repaint( const QRegion& reg, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	QPaintEvent e( reg );
	if ( erase )
	    this->erase( reg );
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
		      fpos.x(), fpos.y(), crect.width(), crect.height(),
		      SWP_NOACTIVATE | SWP_SHOWWINDOW );
    else
	ShowWindow( winId(), SW_SHOW );
    setWState( WState_Visible );
    clearWState( WState_ForceHide );

    QShowEvent e(FALSE);
    QApplication::sendEvent( this, &e );

    UpdateWindow( winId() );
}


/*
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    if ( isTopLevel() && parentWidget() && isActiveWindow() )
	SetFocus( parentWidget()->winId() );
    ShowWindow( winId(), SW_HIDE );
}


void QWidget::showMinimized()
{
    if ( testWFlags(WType_TopLevel) )
	ShowWindow( winId(), SW_SHOWMINIMIZED );
}

void QWidget::showMaximized()
{
    if ( testWFlags(WType_TopLevel) )
	ShowWindow( winId(), SW_SHOWMAXIMIZED );
}

void QWidget::showNormal()
{
    if ( testWFlags(WType_TopLevel) )
	ShowWindow( winId(), SW_SHOWNORMAL );
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
// The internal qWinRequestConfig, defined in qapplication_win.cpp, stores move,
// resize and setGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig( WId, int, int, int, int, int );


void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QPoint oldp( pos() );
    QSize  olds( size() );
    if ( isMove == FALSE && olds.width()==w && olds.height()==h )
	return;
    if ( testWState(WState_ConfigPending) ) {	// processing config event
	qWinRequestConfig( winId(), 2, x, y, w, h );
    } else {
	if ( extra && extra->topextra ) {
	    // They might be different
	    QSize fs = frameSize();
	    w += fs.width()  - crect.width();
	    h += fs.height() - crect.height();
	}
	setFRect( QRect(x,y,w,h) );
	setWState( WState_ConfigPending );
	MoveWindow( winId(), x, y, w, h, TRUE );
	clearWState( WState_ConfigPending );
    }

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
    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint) );

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
    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint) );
}

void QWidget::setSizeIncrement( int w, int h )
{
    createTLExtra();
    extra->topextra->incw = w;
    extra->topextra->inch = h;
}


extern void qt_erase_bg( HDC, int, int, int, int,
			 const QColor &, const QPixmap *, int, int );

void QWidget::erase( int x, int y, int w, int h )
{
    // SIMILAR TO region ERASE BELOW

    if ( backgroundMode()==NoBackground )
	return;
    if ( w < 0 )
	w = crect.width() - x;
    if ( h < 0 )
	h = crect.height() - y;

    bool tmphdc;
    if ( !hdc ) {
	tmphdc = TRUE;
	hdc = GetDC( winId() );
    } else {
	tmphdc = FALSE;
    }
    qt_erase_bg( hdc, x, y, w, h, bg_col, backgroundPixmap(), 0, 0 );
    if ( tmphdc ) {
	ReleaseDC( winId(), hdc );
	hdc = 0;
    }
}

void QWidget::erase( const QRegion& rgn )
{
    // SIMILAR TO rect ERASE ABOVE

    if ( backgroundMode()==NoBackground )
	return;

    bool tmphdc;
    if ( !hdc ) {
	tmphdc = TRUE;
	hdc = GetDC( winId() );
    } else {
	tmphdc = FALSE;
    }
    SelectClipRgn( hdc, rgn.handle() );
    qt_erase_bg( hdc, 0, 0, crect.width(), crect.height(), bg_col,
		 backgroundPixmap(), 0, 0 );
    SelectClipRgn( hdc, 0 );
    if ( tmphdc ) {
	ReleaseDC( winId(), hdc );
	hdc = 0;
    }
}


void QWidget::scroll( int dx, int dy )
{
    ScrollWindow( winId(), dx, dy, 0, 0 );
    UpdateWindow( winId() );
}


void QWidget::drawText( int x, int y, const QString &str )
{
    if ( testWState(WState_Visible) ) {
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
	    //###H: return widget mm width/height
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

void QWidget::createSysExtra()
{
    extra->winIcon = 0;
    extra->dropTarget = 0;
}

void QWidget::deleteSysExtra()
{
    if ( extra->winIcon )
	DestroyIcon( extra->winIcon );
    setAcceptDrops( FALSE );
}

void QWidget::createTLSysExtra()
{
}

void QWidget::deleteTLSysExtra()
{
}


bool QWidget::acceptDrops() const
{
    return ( extra && extra->dropTarget );
}

void QWidget::setAcceptDrops( bool on )
{
    // Enablement is defined by extra->dropTarget != 0.

    if ( on ) {
	// Turn on.
	createExtra();
	QWExtra *extra = extraData();
	if ( !extra->dropTarget )
	    extra->dropTarget = qt_olednd_register( this );
    } else {
	// Turn off.
	QWExtra *extra = extraData();
	if ( extra && extra->dropTarget ) {
	    qt_olednd_unregister(this, extra->dropTarget);
	    extra->dropTarget = 0;
	}
    }
}


void QWidget::setMask( const QRegion &region )
{
    // Since SetWindowRegion takes ownership, and we need to translate,
    // we take a copy.
    HRGN wr = CreateRectRgn(0,0,1,1);
    CombineRgn(wr, region.handle(), 0, RGN_COPY);
    RECT cr;
    GetClientRect( winId(), &cr );
    OffsetRgn(wr, crect.x()-fpos.x(), crect.y()-fpos.y());
    SetWindowRgn( winId(), wr, TRUE );
}

void QWidget::setMask( const QBitmap &bitmap )
{
    HRGN wr = qt_win_bitmapToRegion(bitmap);
    RECT cr;
    GetClientRect( winId(), &cr );
    OffsetRgn(wr, crect.x()-fpos.x(), crect.y()-fpos.y());
    SetWindowRgn( winId(), wr, TRUE );
}

void QWidget::clearMask()
{
    SetWindowRgn( winId(), 0, TRUE );
}
