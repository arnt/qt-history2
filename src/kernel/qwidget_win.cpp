/****************************************************************************
** $Id$
**
** Implementation of QWidget and QWindow classes for Win32
**
** Created : 931205
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qapplication.h"
#include "qapplication_p.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qaccel.h"
#include "qimage.h"
#include "qfocusdata.h"
#include "qlayout.h"
#include "qt_windows.h"
#include "qpaintdevicemetrics.h"
#include "qcursor.h"

#ifdef Q_OS_TEMP
#include "sip.h"
#endif

#if defined(QT_NON_COMMERCIAL)
#include "qmessagebox.h"
#define IDM_ABOUTQT	1
#endif
#if defined(__MINGW32__)
#include <imm.h>
#endif

#if !defined(WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW 0x00000080
#endif

#if !defined(GWLP_WNDPROC)
#define GWLP_WNDPROC GWL_WNDPROC
#endif

const QString qt_reg_winclass( int );		// defined in qapplication_win.cpp
void	    qt_olednd_unregister( QWidget* widget, QOleDropTarget *dst ); // dnd_win
QOleDropTarget* qt_olednd_register( QWidget* widget );


extern bool qt_nograb();
extern HRGN qt_win_bitmapToRegion(const QBitmap& bitmap);

static QWidget *mouseGrb    = 0;
static QCursor *mouseGrbCur = 0;
static QWidget *keyboardGrb = 0;
static HHOOK	journalRec  = 0;

extern "C" LRESULT CALLBACK QtWndProc( HWND, UINT, WPARAM, LPARAM );

extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWState(WState_Created) && window == 0 )
	return;
    setWState( WState_Created );			// set created flag

    if ( !parentWidget() || parentWidget()->isDesktop() )
	setWFlags( WType_TopLevel );		// top-level widget

    static int sw = -1, sh = -1;

    bool topLevel = testWFlags(WType_TopLevel);
    bool popup = testWFlags(WType_Popup);
    bool dialog = testWFlags(WType_Dialog);
    bool desktop  = testWFlags(WType_Desktop);
    HINSTANCE appinst  = qWinAppInst();
    HWND   parentw, destroyw = 0;
    WId	   id;

    QString windowClassName = qt_reg_winclass( getWFlags() );

    if ( !window )				// always initialize
	initializeWindow = TRUE;

    if ( popup ) {
	setWFlags(WStyle_StaysOnTop); // a popup stays on top
    }


    if ( sw < 0 ) {				// get the (primary) screen size
	sw = GetSystemMetrics( SM_CXSCREEN );
	sh = GetSystemMetrics( SM_CYSCREEN );
#ifdef Q_OS_TEMP
	SIPINFO si = { 0 };
	si.cbSize = sizeof(si);
	SipGetInfo( &si );
#define Q_OS_TEMP_MENU_HEIGHT	26
	// The menu should be at the bottom, it is to be 26 pixels high
	int iDelta = (si.fdwFlags & SIPF_ON) ? 0 : Q_OS_TEMP_MENU_HEIGHT;
	sw = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
	sh = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top - iDelta;
#endif
    }

    if ( window ) {
	// There's no way we can know the background color of the
	// other window because it could be cleared using the WM_ERASEBKND
	// message.  Therefore we assume white.
	bg_col = white;
    }

    if ( dialog || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	popup = FALSE;				// force this flags off
	if ( qt_winver == Qt::WV_2000 || qt_winver == Qt::WV_98 || qt_winver == Qt::WV_XP )
	    crect.setRect( GetSystemMetrics( 76 ), GetSystemMetrics( 77 ), GetSystemMetrics( 78 ), GetSystemMetrics( 79 ) );
	else
	    crect.setRect( 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ) );
    }

    parentw = parentWidget() ? parentWidget()->winId() : 0;

#ifdef UNICODE
    TCHAR* title = 0;
#endif
    const char *title95 = 0;
    int	 style = WS_CHILD;
    int	 exsty = 0;

    if ( window ) {
	style = GetWindowLongA( window, GWL_STYLE );
#ifndef QT_NO_DEBUG
	if ( !style )
	    qSystemWarning( "QWidget: GetWindowLong failed" );
#endif
	topLevel = FALSE; // #### needed for some IE plugins??
    } else if ( popup ) {
	style = WS_POPUP;
    } else if ( !topLevel ) {
	if ( !testWFlags(WStyle_Customize) )
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );
    } else if (!desktop ) {
	if ( testWFlags(WStyle_Customize) ) {
	    if ( testWFlags(WStyle_NormalBorder|WStyle_DialogBorder) == 0 ) {
		style = WS_POPUP;		// no border
	    } else {
		style = 0;
	    }
	} else {
#ifndef Q_OS_TEMP
	    style = WS_OVERLAPPED;
	    if ( testWFlags(WType_Dialog ) )
		setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WStyle_ContextHelp );
	    else
		setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );
#else
	    style = WS_OVERLAPPED;
	    setWFlags( WStyle_NormalBorder );
#endif
	}
	// workaround for some versions of Windows
	if ( testWFlags( WStyle_MinMax ) )
	    clearWFlags( WStyle_ContextHelp );
    }
    if ( !desktop ) {
	style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
	if ( topLevel ) {
#ifndef Q_OS_TEMP
	    if ( testWFlags(WStyle_NormalBorder) )
		style |= WS_THICKFRAME;
	    else if ( testWFlags(WStyle_DialogBorder) )
		style |= WS_POPUP | WS_DLGFRAME;
#else
	    if ( testWFlags(WStyle_DialogBorder) )
		style |= WS_POPUP;
#endif
	    if ( testWFlags(WStyle_Title) )
		style |= WS_CAPTION;
	    if ( testWFlags(WStyle_SysMenu) )
		style |= WS_SYSMENU;
	    if ( testWFlags(WStyle_Minimize) )
		style |= WS_MINIMIZEBOX;
	    if ( testWFlags(WStyle_Maximize) )
		style |= WS_MAXIMIZEBOX;
	    if ( testWFlags(WStyle_Tool) | testWFlags(WType_Popup) )
		exsty |= WS_EX_TOOLWINDOW;
	    if ( testWFlags(WStyle_ContextHelp) )
		exsty |= WS_EX_CONTEXTHELP;
	}
    }
    if ( testWFlags(WStyle_Title) ) {
#ifdef UNICODE
#  ifndef Q_OS_TEMP
	if ( qt_winver & Qt::WV_NT_based ) {
#  endif
	    title = isTopLevel() ? (TCHAR*)qt_winTchar_new(QString::fromLatin1(qAppName())) : 
				   (TCHAR*)qt_winTchar_new(QString::fromLatin1(name()));
#  ifndef Q_OS_TEMP
	} else
#  endif
#endif
#ifndef Q_OS_TEMP
	{
	    title95 = isTopLevel() ? qAppName() : name();
	}
#endif
    }

	// The WState_Created flag is checked by translateConfigEvent() in
	// qapplication_win.cpp. We switch it off temporarily to avoid move
	// and resize events during creation
    clearWState( WState_Created );

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
	LONG res = SetWindowLongA( window, GWL_STYLE, style );
#ifndef QT_NO_DEBUG
	if ( !res )
	    qSystemWarning( "QWidget: Failed to set window style" );
#endif
	res = SetWindowLongA( window, GWLP_WNDPROC, (LONG)QtWndProc );
#ifndef QT_NO_DEBUG
	if ( !res )
	    qSystemWarning( "QWidget: Failed to set window procedure" );
#endif
    } else if ( desktop ) {			// desktop widget
#ifndef Q_OS_TEMP
	id = GetDesktopWindow();
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
#endif
    } else if ( topLevel ) {			// create top-level widget
	if ( popup )
	    parentw = 0;

#ifdef Q_OS_TEMP

	int x = CW_USEDEFAULT, y = CW_USEDEFAULT, cx, cy;
	SIPINFO si = { 0 };
	si.cbSize = sizeof(si);
	SipGetInfo( &si );

#define MENU_HEIGHT	26

	// The menu should be at the bottom, it is to be 26 pixels high
	int iDelta = (si.fdwFlags & SIPF_ON) ? 0 : MENU_HEIGHT;
	cx = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
	cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top - iDelta;

	TCHAR *cname = (TCHAR*)qt_winTchar(windowClassName,TRUE);
	if ( exsty )
	    id = CreateWindowEx( exsty, cname, title, style, x, y, cx, cy, parentw, 0, appinst, 0 );
	else
	    id = CreateWindow( cname, title, style, x, y, cx, cy, parentw, 0, appinst, 0 );
#else

#  ifdef UNICODE
	if ( qt_winver & Qt::WV_NT_based ) {
		// ### can this give problems due to the buffer in qt_winTchar????
	    TCHAR *cname = (TCHAR*)qt_winTchar(windowClassName,TRUE);
	    if ( exsty )
		id = CreateWindowEx( exsty, cname, title, style,
		    		    CW_USEDEFAULT, CW_USEDEFAULT,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    parentw, 0, appinst, 0 );
	    else
		id = CreateWindow( cname, title, style,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    parentw, 0, appinst, 0 );
	} else
#  endif
	{
	    if ( exsty )
		id = CreateWindowExA( exsty, windowClassName.latin1(), title95, style,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    parentw, 0, appinst, 0 );
	    else
		id = CreateWindowA( windowClassName.latin1(), title95, style,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    parentw, 0, appinst, 0 );
	}

#endif

#ifndef QT_NO_DEBUG
	if ( id == NULL )
	    qSystemWarning( "QWidget: Failed to create window" );
#endif
	setWinId( id );
	if ( testWFlags( WStyle_StaysOnTop) )
	    SetWindowPos( id, HWND_TOPMOST, 0, 0, 100, 100, SWP_NOACTIVATE );
    } else {					// create child widget
#ifdef UNICODE
#ifndef Q_OS_TEMP
	if ( qt_winver & Qt::WV_NT_based ) {
#endif
	    TCHAR *cname = (TCHAR*)qt_winTchar(windowClassName,TRUE);
	    id = CreateWindow( cname, title, style, 0, 0, 100, 30,
			    parentw, NULL, appinst, NULL );
#ifndef Q_OS_TEMP
	} else
#endif
#endif
#ifndef Q_OS_TEMP
	{
	    id = CreateWindowA( windowClassName.latin1(), title95, style, 0, 0, 100, 30,
			    parentw, NULL, appinst, NULL );
	}
#endif
#ifndef QT_NO_DEBUG
	if ( id == NULL )
	    qSystemWarning( "QWidget: Failed to create window" );
#endif
	SetWindowPos( id, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	setWinId( id );
    }

    if ( desktop ) {
	setWState( WState_Visible );
    } else {
	RECT  fr, cr;
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
	if ( topLevel ){
	    // one cannot trust cr.left and cr.top, use a correction POINT instead
	    POINT pt;
	    pt.x = 0;
	    pt.y = 0;
	    ClientToScreen( id, &pt );
 	    crect = QRect( QPoint(pt.x, pt.y),
 			   QPoint(pt.x+cr.right, pt.y+cr.bottom) );

	    QTLWExtra *top = topData();
	    top->ftop = crect.top() - fr.top;
	    top->fleft = crect.left() - fr.left;
	    top->fbottom = fr.bottom - crect.bottom();
	    top->fright = fr.right - crect.right();
	    fstrut_dirty = FALSE;

	    createTLExtra();
 	} else {
	    crect.setCoords( cr.left, cr.top, cr.right, cr.bottom );
	    // in case extra data already exists (eg. reparent()).  Set it.
	}
    }

    setWState( WState_Created );		// accept move/resize events
    hdc = 0;					// no display context

    if ( window ) {				// got window from outside
	if ( IsWindowVisible(window) )
	    setWState( WState_Visible );
	else
	    clearWState( WState_Visible );
    }

#if defined(QT_NON_COMMERCIAL)
    HMENU menu = GetSystemMenu( winId(), FALSE );
#  ifdef Q_OS_TEMP
    AppendMenuW( menu, MF_SEPARATOR, NULL, NULL );
    AppendMenuW( menu, MF_STRING, IDM_ABOUTQT, L"About Qt" );
#  else
    AppendMenuA( menu, MF_SEPARATOR, NULL, NULL );
    AppendMenuA( menu, MF_STRING, IDM_ABOUTQT, "About Qt" );
#  endif
#endif

    if ( destroyw ) {
	DestroyWindow( destroyw );
    }
#ifdef UNICODE
    if ( title )
	delete [] title;
#endif	

    setFontSys();
}


void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    deactivateWidgetCleanup();
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
	if ( testWFlags(WShowModal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qApp->closePopup( this );
	if ( destroyWindow && !testWFlags(WType_Desktop) ) {
	    DestroyWindow( winId() );
	}
	setWinId( 0 );
    }
}


void QWidget::reparentSys( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    QWidget* oldtlw = topLevelWidget();
    WId old_winid = winid;

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    if ( isVisible() ) {
	ShowWindow( winid, SW_HIDE );
	SetParent( winid, 0 );
    }

    bool accept_drops = acceptDrops();
    if ( accept_drops )
	setAcceptDrops( FALSE ); // ole dnd unregister (we will register again below)
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );

    if ( parent != parentObj ) {
	if ( parentObj ) {			// remove from parent
	    parentObj->removeChild( this );
	}
	if ( parent ) {				// insert into new parent
	    parentObj = parent;			// avoid insertChild warning
	    parent->insertChild( this );
	}
    }
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible | WState_ForceHide );
    if ( isTopLevel() || (!parent || parent->isVisibleTo( 0 ) ) )
	setWState( WState_ForceHide );	// new widgets do not show up in already visible parents
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {				// reparent children
	QObjectListIt it( *chlist );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() ) {
		QWidget *w = (QWidget *)obj;
		if ( !w->isPopup() ) {
		    SetParent( w->winId(), winId() );
		}
	    }
	    ++it;
	}
    }

    if ( p.isNull() )
	resize( s );
    else
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

    reparentFocusWidgets( oldtlw );		// fix focus chains

    if ( accept_drops )
	setAcceptDrops( TRUE );

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


void QWidget::setFontSys( QFont *f )
{
    HFONT hf;
    if ( f )
	hf = f->handle();
    else
	hf = font().handle();

    HIMC imc = ImmGetContext( winId() ); // Can we store it?
#ifdef UNICODE
#ifndef Q_OS_TEMP
    if ( qt_winver & WV_NT_based ) {
#endif
	LOGFONT lf;
	if ( GetObject( hf, sizeof(lf), &lf ) )
	    ImmSetCompositionFont( imc, &lf );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	LOGFONTA lf;
	if ( GetObjectA( hf, sizeof(lf), &lf ) )
	    ImmSetCompositionFontA( imc, &lf );
    }
#endif
    ImmReleaseContext( winId(), imc );
}

void QWidget::setMicroFocusHint(int x, int y, int width, int height, bool text, QFont *f)
{
    CreateCaret( winId(), 0, width, height );
    HideCaret( winId() );
    SetCaretPos( x, y );

    if ( text ) {
	// Translate x,y to be relative to the TLW
	QPoint p(x,y);

	COMPOSITIONFORM cf;
	// ### need X-like inputStyle config settings
	cf.dwStyle = CFS_FORCE_POSITION;
	cf.ptCurrentPos.x = p.x();
	cf.ptCurrentPos.y = p.y();

	CANDIDATEFORM candf;
	candf.dwIndex = 0;
	candf.dwStyle = CFS_FORCE_POSITION;
	candf.ptCurrentPos.x = p.x();
	candf.ptCurrentPos.y = p.y() + height + 3;
	candf.rcArea.left = 0;
	candf.rcArea.top = 0;
	candf.rcArea.right = 0;
	candf.rcArea.bottom = 0;


	HIMC imc = ImmGetContext( winId() ); // Should we store it?
	ImmSetCompositionWindow( imc, &cf );
	ImmSetCandidateWindow( imc, &candf );
	ImmReleaseContext( winId(), imc );
	if ( f ) {
	    setFontSys( f );
	}

    }

    if ( QRect( x, y, width, height ) != microFocusHint() )
	extraData()->micro_focus_hint.setRect( x, y, width, height );
}

// defined in qapplication_win.cpp
extern void qt_winEndImeComposition( QWidget *fw );

void QWidget::resetInputContext()
{
    qt_winEndImeComposition( this );
}

void QWidget::setBackgroundColorDirect( const QColor &color )
{
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
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
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setErasePixmap(QPixmap());
    allow_null_pixmaps--;
}


extern void qt_set_cursor( QWidget *, const QCursor & ); // qapplication_win.cpp

void QWidget::setCursor( const QCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle()
	 || (extra && extra->curs) ) {
	createExtra();
	delete extra->curs;
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

#if defined(QT_NON_COMMERCIAL)
static char* ForK( const char *f ) {
    char *res = new char[strlen(f)+1];
    int i = 0;
    while ( f[i] ) {
	res[i] = f[i] ^ 5;
	i++;
    }
    res[i] = '\0';
    return res;
}
#endif

void QWidget::setCaption( const QString &caption )
{
    if ( QWidget::caption() == caption )
	return; // for less flicker
    topData()->caption = caption;

#ifdef Q_OS_TEMP
    SetWindowText( winId(), (TCHAR*)qt_winTchar(caption,TRUE) );
#else
#if defined(QT_NON_COMMERCIAL)
    QString cap;
    char* t = ForK("Qwjiiq`fm");
    char* q = ForK("Tq");
    char* f = ForK("^Cw``rdw`X%(%");
    if ( caption.find( QString(t) ) != -1 && caption.find( QString(q) ) != -1 )
	cap = caption;
    else if ( caption.find( QString(q) + " Example" ) != -1 )
	cap = caption;
    else if ( parentWidget() && parentWidget()->caption().find( QString(t) ) != -1 && parentWidget()->caption().find( QString(q) ) != -1 )
	cap = caption;
    else if ( inherits("QFileDialog") || inherits("QMessageBox") || inherits("QFontDialog") || inherits("QColorDialog") )
	cap = caption;
    else
	cap = QString(f) + caption;

    delete[] t;
    delete[] q;
    delete[] f;
#else
    QString cap = caption;
#endif
#if defined(UNICODE)
    if ( qt_winver & WV_NT_based )
	SetWindowText( winId(), (TCHAR*)qt_winTchar(cap,TRUE) );
    else
#endif
	SetWindowTextA( winId(), cap.local8Bit() );

#endif

    QEvent e( QEvent::CaptionChange );
    QApplication::sendEvent( this, &e );
}

/*
  Create an icon mask the way Windows wants it using CreateBitmap.
*/

HBITMAP qt_createIconMask( const QBitmap &bitmap )
{
    QImage bm = bitmap.convertToImage();
    int w = bm.width();
    int h = bm.height();
    int bpl = ((w+15)/16)*2;			// bpl, 16 bit alignment
    uchar *bits = new uchar[bpl*h];
    bm.invertPixels();
    for ( int y=0; y<h; y++ )
	memcpy( bits+y*bpl, bm.scanLine(y), bpl );
    HBITMAP hbm = CreateBitmap( w, h, 1, 1, bits );
    delete [] bits;
    return hbm;
}


void QWidget::setIcon( const QPixmap &pixmap )
{
    QTLWExtra* x = topData();
    delete x->icon;
    x->icon = 0;
    if ( x->winIcon ) {
	DestroyIcon( x->winIcon );
	x->winIcon = 0;
    }
    if ( !pixmap.isNull() ) {			// valid icon
	QPixmap pm( pixmap.size(), pixmap.depth(), QPixmap::NormalOptim );
	QBitmap mask( pixmap.size(), FALSE, QPixmap::NormalOptim );
	if ( pixmap.mask() ) {
	    pm.fill( black );			// make masked area black
	    bitBlt( &mask, 0, 0, pixmap.mask() );
	} else {
	    mask.fill( color1 );
	}
	bitBlt( &pm, 0, 0, &pixmap );
	HBITMAP im = qt_createIconMask(mask);
	ICONINFO ii;
	ii.fIcon    = TRUE;
	ii.hbmMask  = im;
	ii.hbmColor = pm.hbm();
	x->icon = new QPixmap( pixmap );
	x->winIcon = CreateIconIndirect( &ii );
	DeleteObject( im );
    }
    SendMessageA( winId(), WM_SETICON, 0, /* ICON_SMALL */
		  (long)x->winIcon );
    SendMessageA( winId(), WM_SETICON, 1, /* ICON_BIG */
		  (long)x->winIcon );

    QEvent e( QEvent::IconChange );
    QApplication::sendEvent( this, &e );
}


void QWidget::setIconText( const QString &iconText )
{
    topData()->iconText = iconText;
}


QCursor *qt_grab_cursor()
{
    return mouseGrbCur;
}


LRESULT CALLBACK qJournalRecordProc( int nCode, WPARAM wParam, LPARAM lParam )
{
#ifndef Q_OS_TEMP
    return CallNextHookEx( journalRec, nCode, wParam, lParam );
#else
    return 0;
#endif
}

void QWidget::grabMouse()
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
#ifndef Q_OS_TEMP
	journalRec = SetWindowsHookExA( WH_JOURNALRECORD,
				       (HOOKPROC)qJournalRecordProc,
				       GetModuleHandleA(0), 0 );
#endif
	SetCapture( winId() );
	mouseGrb = this;
    }
}

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
#ifndef Q_OS_TEMP
	journalRec = SetWindowsHookExA( WH_JOURNALRECORD,
				       (HOOKPROC)qJournalRecordProc,
				       GetModuleHandleA(0), 0 );
#endif
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
#ifndef Q_OS_TEMP
	    UnhookWindowsHookEx( journalRec );
#endif
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
    SetForegroundWindow( topLevelWidget()->winId() );
}


void QWidget::update()
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible )
	InvalidateRect( winId(), 0, !testWFlags( WRepaintNoErase) );
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
	InvalidateRect( winId(), &r, !testWFlags( WRepaintNoErase) );
    }
}


void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QRect r(x,y,w,h);
	if ( r.isEmpty() )
	    return; // nothing to do
	QRegion reg = r;
#ifndef Q_OS_TEMP
	if ( reg.handle() )
	    ValidateRgn( winId(), reg.handle() );
#endif
	QPaintEvent e( r, erase );
	if ( r != rect() )
	    qt_set_paintevent_clipping( this, r );
	if ( erase )
	    this->erase( x, y, w, h );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
    }
}

void QWidget::repaint( const QRegion& reg, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
#ifndef Q_OS_TEMP
	ValidateRgn( winId(), reg.handle() );
#endif
	QPaintEvent e( reg );
	qt_set_paintevent_clipping( this, reg );
	if ( erase )
	    this->erase( reg );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
    }
}

/*
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
    if ( testWFlags(WStyle_Tool) || isPopup() ) {
	QRect fRect = frameGeometry();
	SetWindowPos( winId(), 0,
		      fRect.x(), fRect.y(), fRect.width(), fRect.height(),
		      SWP_NOACTIVATE | SWP_SHOWWINDOW );
    }
    else {
#if defined(QT_NON_COMMERCIAL)
	if ( isTopLevel() && caption() == QString::null
	    && ! ( inherits("QFileDialog") || inherits("QMessageBox")
	    || inherits("QFontDialog") || inherits("QColorDialog") ) ) {
	    char* f = ForK("^Cw``rdw`X%(%");
	    setCaption( QString(f) + QString(qApp->name()) );
	    delete[] f;
	}
#endif
	int sm = SW_SHOW;
	if ( isTopLevel() ) {
#ifdef Q_OS_TEMP
//	    sm = SW_SHOWMAXIMIZED;
#else
	    switch ( topData()->showMode ) {
	    case 1:
		sm = SW_SHOWMINIMIZED;
		break;
	    case 2:
		sm = SW_SHOWMAXIMIZED;
		break;
	    default:
		sm = SW_SHOW;
		break;
	    }
#endif
	    topData()->showMode = 0; // reset
	}
	ShowWindow( winId(), sm );
    }
    UpdateWindow( winId() );
}


/*
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    deactivateWidgetCleanup();
    ShowWindow( winId(), SW_HIDE );
}


void QWidget::showMinimized()
{
    if ( isTopLevel() ) {
#ifndef Q_OS_TEMP
	if ( isVisible() )
	    ShowWindow( winId(), SW_SHOWMINIMIZED );
	else
#endif
	{
	    topData()->showMode = 1;
	    show();
	}
    }
    QEvent e( QEvent::ShowMinimized );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Maximized);
    setWState( WState_Minimized );
}

bool QWidget::isMinimized() const
{
    // true for non-toplevels that have the minimized flag, e.g. MDI children
    return
#ifndef Q_OS_TEMP
		IsIconic(winId()) ||
#endif
		( !isTopLevel() && testWState( WState_Minimized ) );
}

bool QWidget::isMaximized() const
{
    return
#ifndef Q_OS_TEMP
		IsZoomed(winId()) ||
#endif
		( !isTopLevel() && testWState( WState_Maximized ) );
}

void QWidget::showMaximized()
{
    if ( isTopLevel() ) {

	if ( topData()->normalGeometry.width() < 0 )
	    topData()->normalGeometry = geometry();
	if ( isVisible() )
	    ShowWindow( winId(), SW_SHOWMAXIMIZED );
	else {
	    topData()->showMode = 2;
	    show();
	}
    }  else
	show();
    QEvent e( QEvent::ShowMaximized );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Minimized );
    setWState( WState_Maximized );
}

void QWidget::showNormal()
{
    if ( isTopLevel() ) {
	if ( topData()->fullscreen ) {
	    // when reparenting, preserve some widget flags
	    reparent( 0, WType_TopLevel | (getWFlags() & 0xffff0000), QPoint(0,0) );
	    topData()->fullscreen = 0;
	    QRect r = topData()->normalGeometry;
	    if ( r.width() >= 0 ) {
		// the widget has been maximized
		topData()->normalGeometry = QRect(0,0,-1,-1);
		resize( r.size() );
		move( r.topLeft() );
	    }
	}
	show();
	ShowWindow( winId(), SW_SHOWNORMAL );
    } else
	show();
    QEvent e( QEvent::ShowNormal );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Maximized | WState_Minimized );
}


void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    uint f = ( isPopup() || testWFlags(WStyle_Tool) ) ? SWP_NOACTIVATE : 0;
    SetWindowPos( winId(), HWND_TOP, 0, 0, 0, 0, f | SWP_NOMOVE | SWP_NOSIZE );
}

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    uint f = ( isPopup() || testWFlags(WStyle_Tool) ) ? SWP_NOACTIVATE : 0;
    SetWindowPos( winId(), HWND_BOTTOM, 0, 0, 0, 0, f | SWP_NOMOVE |
		  SWP_NOSIZE );
}

void QWidget::stackUnder( QWidget* w)
{
    QWidget *p = parentWidget();
    if ( !w || isTopLevel() || p != w->parentWidget() || this == w )
	return;
    if ( p && p->childObjects && p->childObjects->findRef(w) >= 0 && p->childObjects->findRef(this) >= 0 ) {
	p->childObjects->take();
	p->childObjects->insert( p->childObjects->findRef(w), this );
    }
    SetWindowPos( winId(), w->winId() , 0, 0, 0, 0, SWP_NOMOVE |
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
    QSize  oldSize( size() );
    QPoint oldPos( pos() );
    if ( isMove == FALSE && oldSize.width()==w && oldSize.height()==h )
	return;
    clearWState(WState_Maximized);
    if ( testWState(WState_ConfigPending) ) {	// processing config event
	qWinRequestConfig( winId(), isMove ? 2 : 1, x, y, w, h );
    } else {
	setWState( WState_ConfigPending );
	if ( isTopLevel() ) {
	    QRect fr( frameGeometry() );
	    if ( extra ) {
		fr.setLeft( fr.left() + x - crect.left() );
		fr.setTop( fr.top() + y - crect.top() );
		fr.setRight( fr.right() + ( x + w - 1 ) - crect.right() );
		fr.setBottom( fr.bottom() + ( y + h - 1 ) - crect.bottom() );
	    }
	    MoveWindow( winId(), fr.x(), fr.y(), fr.width(), fr.height(), TRUE );
	} else {
	    crect.setRect( x, y, w, h );
	    MoveWindow( winId(), x, y, w, h, TRUE );
	}
	clearWState( WState_ConfigPending );
    }

     bool isResize = w != oldSize.width() || h != oldSize.height();
     if ( isVisible() ) {
	if ( isMove && pos() != oldPos ) {
	    QMoveEvent e( pos(), oldPos );
	    QApplication::sendEvent( this, &e );
	}
	if ( isResize ) {
	    QResizeEvent e( size(), oldSize );
	    QApplication::sendEvent( this, &e );
	    if ( !testWFlags( WStaticContents ) )
		repaint( visibleRect(), !testWFlags(WResizeNoErase) );
	}
    } else {
	if ( isMove && pos() != oldPos )
	    QApplication::postEvent( this,
				     new QMoveEvent( pos(), oldPos ) );
	if ( isResize )
	    QApplication::postEvent( this,
				     new QResizeEvent( size(), oldSize ) );
    }
}


void QWidget::setMinimumSize( int minw, int minh )
{
#if defined(QT_CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() ) {
	bool resized = testWState( WState_Resized );
	resize( QMAX(minw,width()), QMAX(minh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize( int maxw, int maxh )
{
#if defined(QT_CHECK_RANGE)
    if ( maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX ) {
	qWarning("QWidget::setMaximumSize: The largest allowed size is (%d,%d)",
		 QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
	maxw = QMIN( maxw, QWIDGETSIZE_MAX );
	maxh = QMIN( maxh, QWIDGETSIZE_MAX );
    }
    if ( maxw < 0 || maxh < 0 ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
	maxw = QMAX( maxw, 0 );
	maxh = QMAX( maxh, 0 );
    }
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() ) {
	bool resized = testWState( WState_Resized );
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); //not a user resize
    }
    updateGeometry();
}

void QWidget::setSizeIncrement( int w, int h )
{
    createTLExtra();
    extra->topextra->incw = w;
    extra->topextra->inch = h;
}

void QWidget::setBaseSize( int w, int h )
{
    createTLExtra();
    extra->topextra->basew = w;
    extra->topextra->baseh = h;
}


extern void qt_erase_background( HDC, int, int, int, int,
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
    if ( backgroundOrigin() != WidgetOrigin && !isTopLevel() ) {
	int ox = this->x();
	int oy = this->y();
	if ( backgroundOrigin() == QWidget::WindowOrigin ) {
	    QWidget *topl = this;
	    while(!topl->isTopLevel() && !topl->testWFlags(WSubWindow))
		topl = topl->parentWidget(TRUE);
	    QPoint p = mapTo( topl, QPoint(0,0) );
	    ox = p.x();
	    oy = p.y();
	}
	qt_erase_background( hdc, x, y, w, h, bg_col, backgroundPixmap(), ox, oy );
    } else {
	qt_erase_background( hdc, x, y, w, h, bg_col, backgroundPixmap(), 0, 0 );
    }
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
    if ( backgroundOrigin() != WidgetOrigin && !isTopLevel() ) {
	int ox = x();
	int oy = y();
	if ( backgroundOrigin() == QWidget::WindowOrigin ) {
	    QWidget *topl = this;
	    while(!topl->isTopLevel() && !topl->testWFlags(WSubWindow))
		topl = topl->parentWidget(TRUE);
	    QPoint p = mapTo( topl, QPoint(0,0) );
	    ox = p.x();
	    oy = p.y();
	}
	qt_erase_background( hdc, 0, 0, crect.width(), crect.height(), bg_col,
			     backgroundPixmap(), ox, oy );
    } else {
	qt_erase_background( hdc, 0, 0, crect.width(), crect.height(), bg_col,
		     backgroundPixmap(), 0, 0 );
    }
    SelectClipRgn( hdc, 0 );
    if ( tmphdc ) {
	ReleaseDC( winId(), hdc );
	hdc = 0;
    }
}


void QWidget::scroll( int dx, int dy )
{
    if ( testWState( WState_BlockUpdates ) )
	return;
#ifndef Q_OS_TEMP
    ScrollWindow( winId(), dx, dy, 0, 0 );
#endif
    UpdateWindow( winId() );
}

void QWidget::scroll( int dx, int dy, const QRect& r )
{
    if ( testWState( WState_BlockUpdates ) )
	return;
    RECT wr;
    wr.top = r.top();
    wr.left = r.left();
    wr.bottom = r.bottom()+1;
    wr.right = r.right()+1;
#ifndef Q_OS_TEMP
    ScrollWindow( winId(), dx, dy, &wr, &wr );
#endif
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
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	val = crect.width();
    } else if ( m == QPaintDeviceMetrics::PdmHeight ) {
	val = crect.height();
    } else {
	HDC gdc = GetDC( 0 );
	switch ( m ) {
	case QPaintDeviceMetrics::PdmDpiX:
	case QPaintDeviceMetrics::PdmPhysicalDpiX:
	    val = GetDeviceCaps( gdc, LOGPIXELSX );
	    break;
	case QPaintDeviceMetrics::PdmDpiY:
	case QPaintDeviceMetrics::PdmPhysicalDpiY:
	    val = GetDeviceCaps( gdc, LOGPIXELSY );
	    break;
	case QPaintDeviceMetrics::PdmWidthMM:
	    val = crect.width()
		    * GetDeviceCaps( gdc, HORZSIZE )
		    / GetDeviceCaps( gdc, HORZRES );
	    break;
	case QPaintDeviceMetrics::PdmHeightMM:
	    val = crect.height()
		    * GetDeviceCaps( gdc, VERTSIZE )
		    / GetDeviceCaps( gdc, VERTRES );
	    break;
	case QPaintDeviceMetrics::PdmNumColors:
	    if ( GetDeviceCaps(gdc, RASTERCAPS) & RC_PALETTE )
		val = GetDeviceCaps( gdc, SIZEPALETTE );
	    else
		val = GetDeviceCaps( gdc, NUMCOLORS );
	    break;
	case QPaintDeviceMetrics::PdmDepth:
	    val = GetDeviceCaps( gdc, BITSPIXEL );
	    break;
	default:
	    val = 0;
#if defined(QT_CHECK_RANGE)
	    qWarning( "QWidget::metric: Invalid metric command" );
#endif
	}
	ReleaseDC( 0, gdc );
    }
    return val;
}

void QWidget::createSysExtra()
{
    extra->dropTarget = 0;
}

void QWidget::deleteSysExtra()
{
    setAcceptDrops( FALSE );
}

void QWidget::createTLSysExtra()
{
    extra->topextra->winIcon = 0;
}

void QWidget::deleteTLSysExtra()
{
    if ( extra->topextra->winIcon )
	DestroyIcon( extra->topextra->winIcon );
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

    int fleft = 0, ftop = 0;
    if (isTopLevel()) {
	ftop = topData()->ftop;
	fleft = topData()->fleft;
    }
    OffsetRgn(wr, fleft, ftop );
#ifndef Q_OS_TEMP
    SetWindowRgn( winId(), wr, TRUE );
#endif
}

void QWidget::setMask( const QBitmap &bitmap )
{
    HRGN wr = qt_win_bitmapToRegion(bitmap);
    RECT cr;
    GetClientRect( winId(), &cr );

    int fleft = 0, ftop = 0;
    if (isTopLevel()) {
	ftop = topData()->ftop;
	fleft = topData()->fleft;
    }
    OffsetRgn(wr, fleft, ftop );
#ifndef Q_OS_TEMP
    SetWindowRgn( winId(), wr, TRUE );
#endif
}

void QWidget::clearMask()
{
#ifndef Q_OS_TEMP
    SetWindowRgn( winId(), 0, TRUE );
#endif
}

void QWidget::setName( const char *name )
{
    QObject::setName( name );
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this;

    if ( !isVisible() || isDesktop() ) {
	that->fstrut_dirty = isVisible();
	return;
    }

    RECT  fr, cr;
    GetWindowRect( winId(), &fr );
    GetClientRect( winId(), &cr );

    POINT pt;
    pt.x = 0;
    pt.y = 0;

    ClientToScreen( winId(), &pt );
    that->crect = QRect( QPoint( pt.x, pt.y ),
 			 QPoint( pt.x + cr.right, pt.y + cr.bottom ) );

    QTLWExtra *top = that->topData();
    top->ftop = crect.top() - fr.top;
    top->fleft = crect.left() - fr.left;
    top->fbottom = fr.bottom - crect.bottom();
    top->fright = fr.right - crect.right();

    that->fstrut_dirty = FALSE;
}
