/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_qws.cpp#1 $
**
** Implementation of QWidget and QWindow classes for FB
**
** Created : 991026
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qabstractlayout.h"
#include "qtextcodec.h"
//#include "qimagepaintdevice.h"
#include "qwsdisplay_qws.h"
#include "qgfx_qws.h"
#include "qwsmanager_qws.h"
#include "qwsregionmanager_qws.h"

void qt_enter_modal( QWidget * );		// defined in qapplication_x11.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_insert_sip( QWidget*, int, int );	// --- "" ---
int  qt_sip_count( QWidget* );			// --- "" ---
int  qwidget_tlw_gravity = 1;

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

extern bool qt_xdnd_enable( QWidget* w, bool on );

extern void qt_deferred_map_add( QWidget* ); // defined in qapplication_x11.const
extern void qt_deferred_map_take( QWidget* );// defined in qapplication_x11.const

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;

static int takeLocalId()
{
    static int n=-1000;
    return --n;
}

// This repaints a rectangular area of a widget and all its children
// within the widget.

static void paint_children(QWidget * p,const QRect& r)
{
    if(!p)
	return;
    QObjectList * childObjects=(QObjectList*)p->children();
    if(childObjects) {
	QObject * o;
	for(o=childObjects->first();o!=0;o=childObjects->next()) {
	    if( o->isWidgetType() ) {
		QWidget *w = (QWidget *)o;
		if ( w->testWState(Qt::WState_Visible) ) {
		    QRect wr = w->geometry() & r;
		    if ( !wr.isEmpty() ) {
			wr.moveBy(-w->x(),-w->y());
			paint_children(w,wr);
			w->update(wr);
		    }
		}
	    }
	}
    }
}


#if 0
void QWidget::repaintUnclipped( const QRegion &r, bool erase_r )
{
    if (r.isEmpty())
	return;

    if ( erase_r && !testWFlags( WRepaintNoErase ) )
	erase(r);

    QPaintEvent e( r );
    setWState( WState_InPaintEvent );
    qt_set_paintevent_clipping( this, r);
    QApplication::sendEvent( this, &e );
    qt_clear_paintevent_clipping();
    clearWState( WState_InPaintEvent );

    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( obj->isWidgetType() ) {
		QWidget* w = (QWidget*)obj;
		QRegion cr = r&w->geometry();;
		cr.translate(-w->x(),-w->y());
		w->repaintUnclipped(cr, erase_r );
	    }
	}
    }
}
#endif

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

void QWidget::create( WId window, bool initializeWindow, bool /*destroyOldWindow*/)
{
    if ( testWState(WState_Created) && window == 0 )
	return;
    setWState( WState_Created );			// set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top-level widget

    alloc_region_index = -1;
    alloc_region_revision = -1;

    static int sw = -1, sh = -1;		// screen size

    bool topLevel = testWFlags(WType_TopLevel);
    bool popup = testWFlags(WType_Popup);
    bool modal = testWFlags(WType_Modal);
    if ( modal )
	setWFlags(WStyle_Dialog);
    bool desktop = testWFlags(WType_Desktop);
    WId	   id;
    QWSDisplay* dpy = qwsDisplay();

    if ( !window )				// always initialize
	initializeWindow = TRUE;

    setAllocatedRegionDirty();

    if ( popup ) {
	setWFlags(WStyle_Tool); // a popup is a tool window
	setWFlags(WStyle_StaysOnTop); // a popup stays on top
    }
    if ( topLevel && parentWidget() ) {
	// if our parent has WStyle_StaysOnTop, so must we
	QWidget *ptl = parentWidget()->topLevelWidget();
	if ( ptl && ptl->testWFlags( WStyle_StaysOnTop ) )
	    setWFlags(WStyle_StaysOnTop);
    }

    if ( sw < 0 ) {				// get the screen size
	sw = dpy->width();
	sh = dpy->height();
    }

    if ( modal || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	modal = popup = FALSE;			// force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {			// calc pos/size from screen
	crect.setRect( 0, 0, sw/2, 4*sh/10 );
    } else {					// child widget
	crect.setRect( 0, 0, 100, 30 );
    }

    if ( window ) {				// override the old window
	id = window;
	setWinId( window );
    } else if ( desktop ) {			// desktop widget
	id = (WId)-2;				// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else {
	id = topLevel ? dpy->takeId() : takeLocalId();
	setWinId( id );				// set widget id/handle + hd
    }

    if ( !topLevel ) {
	if ( !testWFlags(WStyle_Customize) )
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );
    } else if ( !(desktop || popup) ) {
	if ( testWFlags(WStyle_Customize) ) {	// customize top-level widget
	    if ( testWFlags(WStyle_NormalBorder) ) {
		// XXX ...
	    } else {
		if ( !testWFlags(WStyle_DialogBorder) ) {
		    // XXX ...
		}
	    }
	    if ( testWFlags(WStyle_Tool) ) {
		// XXX ...
	    }
	} else {				// normal top-level widget
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
		       WStyle_MinMax );
	}
    }

    if ( !initializeWindow ) {
	// do no initialization
    } else if ( popup ) {			// popup widget
    } else if ( topLevel && !desktop ) {	// top-level widget
	QWidget *p = parentWidget();	// real parent
	if (p)
	    p = p->topLevelWidget();
	if ( testWFlags(WStyle_DialogBorder)
	     || testWFlags(WStyle_StaysOnTop)
	     || testWFlags(WStyle_Dialog)
	     || testWFlags(WStyle_Tool) ) {
	    // XXX ...
	}

	// find the real client leader, i.e. a toplevel without parent
	while ( p && p->parentWidget()) {
	    p = p->parentWidget()->topLevelWidget();
	}

	// XXX ...
    }

    if ( initializeWindow ) {
    }

    setWState( WState_MouseTracking );
    setMouseTracking( FALSE );			// also sets event mask
    if ( desktop ) {
	setWState( WState_Visible );
    } else if ( topLevel ) {			// set X cursor
	//QCursor *oc = QApplication::overrideCursor();
	if ( initializeWindow ) {
	    //XXX XDefineCursor( dpy, winid, oc ? oc->handle() : cursor().handle() );
	}
	setWState( WState_OwnCursor );
    }

    if ( topLevel ) {
#ifndef QT_NO_QWS_MANAGER
	if ( testWFlags(WStyle_DialogBorder)
	     || testWFlags(WStyle_NormalBorder))
	{
	    // get size of wm decoration
	    QRegion r = QApplication::qwsDecoration().region(this, crect);
	    QRect br( r.boundingRect() );
	    crect.moveBy( crect.x() - br.x(), crect.y() - br.y() );
	    topData()->qwsManager = new QWSManager(this);
	}
#endif	
	// declare the widget's object name as window role
	// If we are session managed, inform the window manager about it
	if ( extra && !extra->mask.isNull() ) {
	    req_region = extra->mask;
	    req_region.translate(crect.x(),crect.y()); //###expensive?
	    req_region &= crect; //??? this is optional
	} else {
	    req_region = crect;
	}
    }
    fpos = crect.topLeft();			// default frame rect
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
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qApp->closePopup( this );
	if ( testWFlags(WType_Desktop) ) {
	} else {
	    if ( destroyWindow ) {
		if ( isTopLevel() )
		    qwsDisplay()->destroyRegion( winId() );
	    }
	}
	setWinId( 0 );
    }
}


void QWidget::reparent( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
#ifndef QT_NO_CURSOR
    QCursor oldcurs;
    bool setcurs=testWState(WState_OwnCursor);
    if ( setcurs ) {
	oldcurs = cursor();
	unsetCursor();
    }
#endif
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );
    reparentFocusWidgets( parent );		// fix focus chains

    setAllocatedRegionDirty(); // affects my siblings

    if ( parentObj ) {				// remove from parent
	parentObj->removeChild( this );
    }
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    //QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    //QColor   bgc    = bg_col;			// save colors
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible );
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {				// reparent children
	QObjectListIt it( *chlist );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() && !((QWidget*)obj)->isTopLevel() ) {
		// QWidget *w = (QWidget *)obj;
		// XXX XReparentWindow( x11Display(), w->winId(), winId(), w->geometry().x(), w->geometry().y() );
	    }
	    ++it;
	}
    }
    /*
    if ( bgp )
	XSetWindowBackgroundPixmap( dpy, winid, bgp->handle() );
    else
	XSetWindowBackground( dpy, winid, bgc.pixel() );
    */
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( old_winid ) {
	// XXX qt_XDestroyWindow( this, dpy, old_winid );
    }
#ifndef QT_NO_CURSOR
    if ( setcurs ) {
	setCursor(oldcurs);
    }
#endif
#ifndef QT_NO_ACCEL
    QObjectList	*accelerators = queryList( "QAccel" );
    QObjectListIt it( *accelerators );
    QObject *obj;
    while ( (obj=it.current()) != 0 ) {
	++it;
	((QAccel*)obj)->repairEventFilter();
    }
    delete accelerators;
#endif // QT_NO_ACCEL
    if ( !parent ) {
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
    }
}


QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    int	   x=pos.x(), y=pos.y();
    const QWidget* w = this;
    while (w) {
	x += w->x();
	y += w->y();
	w = w->isTopLevel() ? 0 : w->parentWidget();
    }
    return QPoint( x, y );
}

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    int	   x=pos.x(), y=pos.y();
    const QWidget* w = this;
    while (w) {
	x -= w->x();
	y -= w->y();
	w = w->isTopLevel() ? 0 : w->parentWidget();
    }
    return QPoint( x, y );
}

void QWidget::setMicroFocusHint( int /*x*/, int /*y*/, int /*width*/, int /*height*/,
				 bool /*text*/)
{
    //XXX not implemented
#if 0
    if ( QRect( x, y, width, height ) != microFocusHint() )
	extraData()->micro_focus_hint.setRect( x, y, width, height );

    if ( text ) {
	
    }
#endif
}


void QWidget::setFontSys()
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
    // XXX XSetWindowBackground( x11Display(), winId(), bg_col.pixel() );
    backgroundColorChange( old );
}

static int allow_null_pixmaps = 0;


void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	// XXX XSetWindowBackground( x11Display(), winId(), bg_col.pixel() );
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	QPixmap pm = pixmap;
	if (!pm.isNull()) {
	    if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
		pm = QPixmap( pixmap.size() );
		bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	    }
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pm );
	// XXX XSetWindowBackgroundPixmap( x11Display(), winId(), pm.handle() );
    }
    if ( !allow_null_pixmaps ) {
	backgroundPixmapChange( old );
    }
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setBackgroundPixmap(QPixmap());
    allow_null_pixmaps--;
}

#ifndef QT_NO_CURSOR

void QWidget::setCursor( const QCursor &cursor )
{
    if ( 1/*cursor.handle() != arrowCursor.handle()*/
	 || (extra && extra->curs) ) {
	createExtra();
	delete extra->curs;
	extra->curs = new QCursor(cursor);
    }
    setWState( WState_OwnCursor );
//    QCursor *oc = QApplication::overrideCursor();
}

void QWidget::unsetCursor()
{
    if ( !isTopLevel() ) {
	if (extra ) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	clearWState( WState_OwnCursor );
	// XXX XDefineCursor( x11Display(), winId(), None );
    }
}
#endif //QT_NO_CURSOR

void QWidget::setCaption( const QString &caption )
{
    if ( extra && extra->topextra && extra->topextra->caption == caption )
	return; // for less flicker
    createTLExtra();
    extra->topextra->caption = caption;
    // XXX XSetWMName( x11Display(), winId(), qstring_to_xtp(caption) );
    QEvent e( QEvent::CaptionChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
    } else {
	createTLExtra();
    }
    QBitmap mask;
    if ( pixmap.isNull() ) {
    } else {
	extra->topextra->icon = new QPixmap( pixmap );
	mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
    }
    // XXX
}


void QWidget::setIconText( const QString &iconText )
{
    createTLExtra();
    extra->topextra->iconText = iconText;
    // XXX XSetIconName( x11Display(), winId(), iconText.utf8() );
    // XXX XSetWMIconName( x11Display(), winId(), qstring_to_xtp(iconText) );
}


void QWidget::grabMouse()
{
    if ( mouseGrb )
	mouseGrb->releaseMouse();

    qwsDisplay()->grabMouse(this,TRUE);

    mouseGrb = this;
}

#ifndef QT_NO_CURSOR
void QWidget::grabMouse( const QCursor &cursor )
{
    if ( mouseGrb )
	mouseGrb->releaseMouse();

    qwsDisplay()->grabMouse(this,TRUE);
    qwsDisplay()->selectCursor(this, (int)cursor.handle());
    mouseGrb = this;
}
#endif

void QWidget::releaseMouse()
{
    if ( mouseGrb == this ) {
	qwsDisplay()->grabMouse(this,FALSE);
	mouseGrb = 0;
    }
}

void QWidget::grabKeyboard()
{
    if ( keyboardGrb )
	keyboardGrb->releaseKeyboard();
    // XXX XGrabKeyboard( x11Display(), winid, TRUE, GrabModeAsync, GrabModeAsync, CurrentTime );
    keyboardGrb = this;
}

void QWidget::releaseKeyboard()
{
    if ( keyboardGrb == this ) {
	// XXX XUngrabKeyboard( x11Display(), CurrentTime );
	keyboardGrb = 0;
    }
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
    QWidget *tlw = topLevelWidget();
    if ( tlw->isVisible() )
	qwsDisplay()->requestFocus( tlw->winId(), TRUE);
}


void QWidget::update()
{
    //if ( (widget_state & (WState_Visible|WState_BlockUpdates)) ==
    //WState_Visible && isVisible() )
    //QApplication::postEvent( this, new QPaintEvent( rect() ) );
    update(0,0,width(),height());
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( w && h &&
         (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible && isVisible() ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QApplication::postEvent(this,new QPaintEvent(QRect(x,y,w,h),
			   !testWFlags(WRepaintNoErase) ) );
	//erase will be done in QApplication::sendPostedEvents(), if necessary
    }
}

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible && isVisible() ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QRect r(x,y,w,h);
	if ( r.isEmpty() )
	    return; // nothing to do
	if ( erase )
	    this->erase(x,y,w,h);
	QPaintEvent e( r, erase );
	qt_set_paintevent_clipping( this, r );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
    }
}

void QWidget::repaint( const QRegion& reg, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible && isVisible() ) {
	if ( erase )
	    this->erase(reg);
	QPaintEvent e( reg );
	qt_set_paintevent_clipping( this, reg );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
    }
}

void QWidget::showWindow()
{
    setAllocatedRegionDirty();
    if ( testWFlags(WType_TopLevel) ) {
	QRegion r( req_region );
#ifndef QT_NO_QWS_MANAGER
	if ( extra && extra->topextra && extra->topextra->qwsManager )
	    r += extra->topextra->qwsManager->region();
#endif
	qwsDisplay()->requestRegion(winId(), r);
	if ( !testWFlags(WStyle_Tool) )
	    qwsDisplay()->requestFocus(winId(),TRUE);
	if ( testWFlags(WStyle_StaysOnTop) )
	    qwsDisplay()->setAltitude( winId(), 0, TRUE );
    } else {
	update();
    }
}


void QWidget::hideWindow()
{
    setAllocatedRegionDirty();
    deactivateWidgetCleanup();

    if ( testWFlags(WType_TopLevel) ) {
	qwsDisplay()->requestRegion(winId(), QRegion());
	qwsDisplay()->requestFocus(winId(),FALSE);
    } else {
	bool v = testWState(WState_Visible);
	clearWState(WState_Visible);
	parentWidget()->repaint(geometry());
	paint_children( parentWidget(),geometry() );
	if ( v )
	    setWState(WState_Visible);
    }
}

void QWidget::showMinimized()
{
    /* XXX
    if ( testWFlags(WType_TopLevel) )
	XIconifyWindow( x11Display(), winId(), x11Screen() );
    */
    //### if the window is mapped (i.e. not WState_Withdrawn) we have
    // to show it with initial state Iconic! Right now the function only
    // works for widgets that are already visible.
    hide();
    //parentWidget()->repaint(geometry());
}

bool QWidget::isMinimized() const
{
    return FALSE; // XXX
}

void QWidget::showMaximized()
{
    if ( testWFlags(WType_TopLevel) ) {
	createTLExtra();
#ifndef QT_NO_QWS_MANAGER
	if ( extra && extra->topextra && extra->topextra->qwsManager ) {
	    extra->topextra->qwsManager->maximize();
	} else
#endif
	{
	    setGeometry( QApplication::desktop()->rect() );
	}
    }
    show();
    QEvent e( QEvent::ShowMaximized );
    QApplication::sendEvent( this, &e );
    setWState(WState_Maximized);
}

void QWidget::showNormal()
{
    // XXX
}


void QWidget::raise()
{
    setAllocatedRegionDirty();
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    if ( isTopLevel() )
	qwsDisplay()->setAltitude( winId(), 0 );
    else
	paint_children( p,geometry() );
}

void QWidget::lower()
{
    setAllocatedRegionDirty();
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    if ( isTopLevel() )
	qwsDisplay()->setAltitude( winId(), -1 );
    else
	paint_children( p,geometry() );
}

void QWidget::stackUnder( QWidget* w)
{
    QWidget *p = parentWidget();
    if ( !p || !w || isTopLevel() || p != w->parentWidget() )
	return;
    setAllocatedRegionDirty();
    int loc = p->childObjects->findRef(w);
    if ( loc >= 0 && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( loc, p->childObjects->take() );
    // #### excessive repaints
    paint_children( p,geometry() );
    paint_children( p,w->geometry() );
}

void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    clearWState(WState_Maximized);
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
    QPoint oldp = pos();
    QSize  olds = size();
    QRect  r( x, y, w, h );

    bool isResize = olds != r.size();

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if ( r.size() == olds && oldp == r.topLeft() )
	return;

    setAllocatedRegionDirty();

    setCRect( r );

    if ( testWFlags(WType_Desktop) )
	return;

    if ( isTopLevel() ) {

	//### ConfigPending not implemented, do we need it?
	//setWState( WState_ConfigPending );
	if ( extra && !extra->mask.isNull() ) {
	    req_region = extra->mask;
	    req_region.translate(crect.x(),crect.y());
	    req_region &= crect; //??? this is optional
	} else {
	    req_region = crect;
	}
	if ( isVisible() ) {
	    QRegion rgn( req_region );
#ifndef QT_NO_QWS_MANAGER
	    if ( extra && extra->topextra && extra->topextra->qwsManager )
		rgn += extra->topextra->qwsManager->region();
#endif
	    if ( isMove && !isResize && alloc_region_index >= 0 )
		qwsDisplay()->moveRegion(winId(), x - oldp.x(), y - oldp.y());
	    else
		qwsDisplay()->requestRegion(winId(), rgn);
	}
    }

    /*
    if( isMove && ( w==olds.width() && h==olds.height() ) &&
	!isTopLevel() ) {
	QGfx * mygfx=parentWidget()->graphicsContext();

	QWidget * par=parentWidget();

	// Code from paintableRegion
	QRegion r;
	if (par->isVisible()) {
	    r = par->allocatedRegion();
	    if (par->extra && par->extra->topextra)
		r += par->extra->topextra->decor_allocated_region;
	    const QObjectList *c = par->children();
	    if ( c ) {
		QObjectListIt it(*c);
		QObject* ch;
		while ((ch=it.current())) {
		    ++it;
		    if ( ch->isWidgetType() &&
			 !((QWidget*)ch)->isTopLevel() &&
			 ((QWidget *)ch)!=this) {
			r -= ((QWidget*)ch)->requestedRegion();
		    }
		}
	    }
	}

	QPoint a(x,y);
        a=mapToGlobal(a);
	QPoint b(oldp.x(),oldp.y());
	b=mapToGlobal(b);

	// If source and destination rect both within clipping region
	// do it the fast way

	if(r.fullyContains(QRect(a.x(),a.y(),w,h)) &&
	   r.fullyContains(QRect(b.x(),b.y(),w,h))) {
	    mygfx->setWidgetRegion(r);
	    mygfx->setSource(parentWidget());
	    mygfx->setSourceOffset(oldp.x(),oldp.y());
	    mygfx->blt(x,y,w,h);
	    delete mygfx;
	    QRect rp=QRect(oldp,olds);
	    parentWidget()->repaint(rp);
	    QRegion r2=QRegion(rp).subtract(r);
	    int loopc;
	    QArray<QRect> tmp=r2.rects();
	    for(loopc=0;loopc<tmp.size();loopc++) {
		paint_children(parentWidget(),tmp[loopc]);
	    }
	    QApplication::postEvent( this,
				     new QMoveEvent( QPoint(x,y), oldp ) );
	    return;
	}
    }
    */

    if ( isVisible() ) {
	if ( isMove ) {
	    QMoveEvent e( r.topLeft(), oldp );
	    QApplication::sendEvent( this, &e );
#ifndef QT_NO_QWS_MANAGER
	    if (extra && extra->topextra && extra->topextra->qwsManager)
		QApplication::sendEvent( extra->topextra->qwsManager, &e );
#endif	
	}
	if ( isResize ) {
	    QResizeEvent e( r.size(), olds );
	    QApplication::sendEvent( this, &e );
#ifndef QT_NO_QWS_MANAGER
	    if (extra && extra->topextra && extra->topextra->qwsManager) {
		QResizeEvent e( r.size(), olds );
		QApplication::sendEvent(topData()->qwsManager, &e);
	    }
#endif	
	    if ( !testWFlags( WNorthWestGravity ) ) {
		QApplication::postEvent(this,new QPaintEvent(visibleRect(),
					!testWFlags(WResizeNoErase) ) );
	    }
	}
	if ( !isTopLevel() || isResize ) {
	    QRect tmp(oldp,olds);
	    QRect tmp2=tmp.unite(r);
	    QWidget *p = parentWidget();
	    if (p) {
		p->update(tmp);
		paint_children( p, tmp2 );
	    } else {
		tmp2.moveBy(-x, -y);
		paint_children( this, tmp2 );
	    }
	}
#ifndef QT_NO_QWS_MANAGER
	if (isResize && extra && extra->topextra && extra->topextra->qwsManager) {
	    QApplication::postEvent(topData()->qwsManager,
				    new QPaintEvent(visibleRect(),
				    !testWFlags(WResizeNoErase) ) );
	}
#endif	
    } else {
	if ( isMove )
	    QApplication::postEvent( this,
				     new QMoveEvent( r.topLeft(), oldp ) );
	if ( isResize )
	    QApplication::postEvent( this,
				     new QResizeEvent( r.size(), olds ) );
    }
}


void QWidget::setMinimumSize( int minw, int minh )
{
#if defined(CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() )
	resize( QMAX(minw,width()), QMAX(minh,height()) );
    if ( testWFlags(WType_TopLevel) ) {
	// XXX
    }
    updateGeometry();
}

void QWidget::setMaximumSize( int maxw, int maxh )
{
#if defined(CHECK_RANGE)
    if ( maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX );
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
    if ( maxw < width() || maxh < height() )
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
    if ( testWFlags(WType_TopLevel) ) {
	// XXX
    }
    updateGeometry();
}

void QWidget::setSizeIncrement( int w, int h )
{
    createTLExtra();
    QTLWExtra* x = extra->topextra;
    if ( x->incw == w && x->inch == h )
	return;
    x->incw = w;
    x->inch = h;
    if ( testWFlags(WType_TopLevel) ) {
	// XXX ...
    }
}

void QWidget::setBaseSize( int basew, int baseh )
{
    createTLExtra();
    QTLWExtra* x = extra->topextra;
    if ( x->basew == basew && x->baseh == baseh )
	return;
    x->basew = basew;
    x->baseh = baseh;
    if ( testWFlags(WType_TopLevel) ) {
	// XXX
    }
}
/*
  //Just used in commented-out code
static void drawTileAligned(QPainter& p, const QRect& r, const QPixmap& pm)
{
    p.setClipRect(r);
    if ( !pm.isNull() ) {
	p.drawTiledPixmap(r,pm,QPoint(r.x()%pm.width(),r.y()%pm.height()));
    }
}
*/
void QWidget::erase( int x, int y, int w, int h )
{
    if ( backgroundMode() == NoBackground )
	return;

    erase( QRegion( x, y, w, h ) );

/*
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;

    if ( w != 0 && h != 0 ) {
	QPainter p(this);
	QRect r(x,y,w,h);
	if ( extra && extra->bg_pix ) {
	    drawTileAligned(p,r,*extra->bg_pix);
	} else {
	    p.fillRect(r,bg_col);
	}
    }
*/
}

void QWidget::erase( const QRegion& reg )
{
    if ( backgroundMode() == NoBackground )
	return;

    int xoff = 0;
    int yoff = 0;
    if ( !isTopLevel() && backgroundOrigin() == QWidget::ParentOrigin ) {
	xoff = x();
	yoff = y();
    }

    QArray<QRect> r = reg.rects();
    QPainter p(this);
    for (uint i=0; i<r.size(); i++) {
	const QRect& rr = r[(int)i];
	if ( extra && extra->bg_pix ) {
	    p.setClipRect(rr);
	    if ( !extra->bg_pix->isNull() ) {
		p.drawTiledPixmap(rr,*extra->bg_pix,
				  QPoint((rr.x()+xoff)%extra->bg_pix->width(),
				         (rr.y()+yoff)%extra->bg_pix->height()));
	    }
//	    drawTileAligned(p,rr,*extra->bg_pix);
	} else {
	    p.fillRect(rr,bg_col);
	}
    }
}

void QWidget::scroll( int dx, int dy )
{
    scroll( dx, dy, QRect() );
}

void QWidget::scroll( int dx, int dy, const QRect& r )
{
    if ( testWState( WState_BlockUpdates ) )
	return;
    bool valid_rect = r.isValid();
    QRect sr = valid_rect?r:rect();
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
    if ( dx > 0 ) {
	x1 = sr.x();
	x2 = x1+dx;
	w -= dx;
    } else {
	x2 = sr.x();
	x1 = x2-dx;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = sr.y();
	y2 = y1+dy;
	h -= dy;
    } else {
	y2 = sr.y();
	y1 = y2-dy;
	h += dy;
    }

    if ( dx == 0 && dy == 0 )
	return;
    if ( w > 0 && h > 0 ) {
	//bitBlt(this,x2,y2,this,x1,y1,w,h);
	QGfx * mygfx=graphicsContext();
	mygfx->setSource(this);
	mygfx->scroll(x2,y2,w,h,x1,y1);
	delete mygfx;
    }
    if ( !valid_rect && children() ) {	// scroll children
	QPoint pd( dx, dy );
	QObjectListIt it(*children());
	register QObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		QWidget *w = (QWidget *)object;
		w->move( w->pos() + pd );
	    }
	    ++it;
	}
    }
    QRegion copied = paintableRegion();
    QPoint p = mapToGlobal( QPoint() );
    copied.translate( -p.x(), -p.y() );
    copied &= QRegion(sr);
    copied.translate(dx,dy);
    QRegion exposed = QRegion(sr) - copied;
    repaint( exposed, !testWFlags(WRepaintNoErase) );
}


void QWidget::drawText( int x, int y, const QString &str )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible && isVisible() ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


int QWidget::metric( int m ) const
{
    int val;
    if ( m == QPaintDeviceMetrics::PdmWidth ||
	 m == QPaintDeviceMetrics::PdmWidthMM ) {
	val = crect.width();
    } else if ( m == QPaintDeviceMetrics::PdmHeight ||
		m == QPaintDeviceMetrics::PdmHeightMM ) {
	val = crect.height();
    } else if ( m == QPaintDeviceMetrics::PdmDepth ) {
	return qwsDisplay()->depth();
    } else if ( m == QPaintDeviceMetrics::PdmDpiX ) {
	return 72;
    } else if ( m == QPaintDeviceMetrics::PdmDpiY ) {
	return 72;
    } else {
	val = QPaintDevice::metric(m);// XXX
    }
    return val;
}

void QWidget::createSysExtra()
{
}

void QWidget::deleteSysExtra()
{
}

void QWidget::createTLSysExtra()
{
}

void QWidget::deleteTLSysExtra()
{
}


bool QWidget::acceptDrops() const
{
    return testWState(WState_DND);
}

void QWidget::setAcceptDrops( bool on )
{
    if ( testWState(WState_DND) != on ) {
	if ( 1/*XXX qt_xdnd_enable( this, on )*/ ) {
	    if ( on )
		setWState(WState_DND);
	    else
		clearWState(WState_DND);
	}
    }
}

QRegion QWidget::requestedRegion() const
{
    if ( isTopLevel() ) {
	return req_region;
    } else {
	if ( testWState( WState_Visible ) ) {
	    QPoint gpos( mapToGlobal(QPoint(0,0)) );
	    QRegion r( QRect(gpos,crect.size()) );
	    if ( extra && !extra->mask.isNull() ) {
		QRegion maskr = extra->mask;
		maskr.translate( gpos.x(), gpos.y() );
		r &= maskr;
	    }
	    return r;
	} else {
	    return QRect();
	}
    }
}

void QWidget::setAllocatedRegionDirty()
{
    if ( !isTopLevel() && !alloc_region_dirty ) {
	// if I am dirty, then this can affect my siblings as well
	const QObjectList *c = parentWidget()->children();
	if ( c ) {
	    QObjectListIt it(*c);
	    QObject* ch;
	    while ((ch=it.current())) {
		++it;
		if ( ch->isWidgetType() )
		    ((QWidget *)ch)->alloc_region_dirty = TRUE;
	    }
	}
    }
}

// check my hierarchy for dirty allocated regions
bool QWidget::isAllocatedRegionDirty() const
{
    if ( isTopLevel() )
	return FALSE;

    if ( alloc_region_dirty )
	return TRUE;

    return parentWidget()->isAllocatedRegionDirty();
}

QRegion QWidget::allocatedRegion() const
{
    if (isVisible()) {
	if ( isTopLevel() ) {
	    return alloc_region;
	} else {
	    if ( isAllocatedRegionDirty() ) {
		QRegion r( requestedRegion() );
		r &= parentWidget()->allocatedRegion();
		const QObjectList *c = parentWidget()->children();
		if ( c ) {
		    QObjectListIt it(*c);
		    QObject* ch;
		    bool clip=FALSE;
		    while ((ch=it.current())) {
			++it;
			if ( ch->isWidgetType() ) {
			    if ( ((QWidget*)ch) == this )
				clip=TRUE;
			    else if ( clip && !((QWidget*)ch)->isTopLevel())
				r -= ((QWidget*)ch)->requestedRegion();
			}
		    }
		}
		// if I'm dirty, so are my chlidren.
		c = children();
		if ( c ) {
		    QObjectListIt it(*c);
		    QObject* ch;
		    while ((ch=it.current())) {
			++it;
			if ( ch->isWidgetType() )
			    ((QWidget *)ch)->alloc_region_dirty = TRUE;
		    }
		}

		alloc_region = r;
		alloc_region_dirty = FALSE;
	    }
	    return alloc_region;
	}
    } else {
	return QRegion();
    }
}

QRegion QWidget::paintableRegion() const
{
    QRegion r;
    if (isVisible()) {
	r = allocatedRegion();
#ifndef QT_NO_QWS_MANAGER
	if (extra && extra->topextra)
	    r += extra->topextra->decor_allocated_region;
#endif
	const QObjectList *c = children();
	if ( c ) {
	    QObjectListIt it(*c);
	    QObject* ch;
	    while ((ch=it.current())) {
		++it;
		if ( ch->isWidgetType() && !((QWidget*)ch)->isTopLevel()) {
		    r -= ((QWidget*)ch)->requestedRegion();
		}
	    }
	}
    }
    return r;
}


void QWidget::setMask( const QRegion& region )
{
    alloc_region_dirty = true;

    createExtra();

    if ( region.isNull() && extra->mask.isNull() )
	return;

    extra->mask = region;

    if ( isTopLevel() ) {
	if ( !region.isNull() ) {
	    req_region = extra->mask;
	    req_region.translate(crect.x(),crect.y()); //###expensive?
	    req_region &= crect; //??? this is optional
	} else
	    req_region = QRegion(crect);
    }
    if ( isVisible() ) {
	if ( isTopLevel() ) {
	    QRegion rgn( req_region );
#ifndef QT_NO_QWS_MANAGER
	    if ( extra && extra->topextra && extra->topextra->qwsManager )
		rgn += extra->topextra->qwsManager->region();
#endif
	    qwsDisplay()->requestRegion(winId(), rgn);
	} else {
	//XXX
	}
    }
}

void QWidget::setMask( const QBitmap &bitmap )
{
    setMask( QRegion( bitmap ) );
}

void QWidget::clearMask()
{
    setMask( QRegion() );
}

void QWidget::setName( const char *name )
{
    QObject::setName( name );
    if ( isTopLevel() ) {
	// XXX
    }
}

QGfx * QWidget::graphicsContext() const
{
    QGfx * qgfx_qws;
    qgfx_qws=qwsDisplay()->screenGfx();
    QPoint offset=mapToGlobal(QPoint(0,0));
    QRegion r; // empty if not visible
    if ( isVisible() && topLevelWidget()->isVisible() ) {
	r = paintableRegion();
	int rgnIdx = topLevelWidget()->alloc_region_index;
	if ( rgnIdx >= 0 ) {
	    QWSDisplay::grab();
	    int *rgnRev = qwsDisplay()->regionManager()->revision( rgnIdx );
	    if ( topLevelWidget()->alloc_region_revision != *rgnRev ) {
		// The TL region has changed, so we better make sure we're
		// not writing to any regions we don't own anymore.
		// We'll get a RegionModified event soon that will get our
		// regions back in sync again.
		r &= qwsDisplay()->regionManager()->region( rgnIdx );
	    }
	    qgfx_qws->setGlobalRegionIndex( rgnIdx );
	    QWSDisplay::ungrab();
	} else {
	    r = QRegion();
	}
    }
    qgfx_qws->setWidgetRegion(r);
    qgfx_qws->setOffset(offset.x(),offset.y());
    // Clip the window decoration for TL windows.
    // It is possible for these windows to draw on the wm decoration if
    // they change the clip region.  Bug or feature?
#ifndef QT_NO_QWS_MANAGER
    if ( extra && extra->topextra && extra->topextra->qwsManager )
	qgfx_qws->setClipRegion(rect());
#endif

    return qgfx_qws;
}

unsigned char * QWidget::scanLine(int i) const
{
    // Should add widget x() here, maybe
    unsigned char * base=qwsDisplay()->frameBuffer();
    base+=i*bytesPerLine();
    return base;
}

int QWidget::bytesPerLine() const
{
    return qt_screen->linestep();
}
