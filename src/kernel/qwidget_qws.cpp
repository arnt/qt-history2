/****************************************************************************
** $Id: $
**
** Implementation of QWidget and QWindow classes for FB
**
** Created : 991026
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcursor.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qcursor.h"

#include "qwsdisplay_qws.h"
#include "qgfx_qws.h"
#include "qwsmanager_qws.h"
#include "qwsregionmanager_qws.h"

void qt_insert_sip( QWidget*, int, int );	// defined in qapplication_x11.cpp
int  qt_sip_count( QWidget* );			// --- "" ---
int  qt_widget_tlw_gravity = 1;

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

// This repaints all children within a widget.

static void paint_children(QWidget * p,const QRegion& r)
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
		    QRegion wr( QRegion(w->geometry()) & r );
		    if ( !wr.isEmpty() ) {
			wr.translate(-w->x(),-w->y());
			QApplication::postEvent(w,new QPaintEvent(wr,
				   !w->testWFlags(QWidget::WRepaintNoErase) ) );
			paint_children(w,wr);
		    }
		}
	    }
	}
    }
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

void QWidget::create( WId window, bool initializeWindow, bool /*destroyOldWindow*/)
{
    if ( testWState(WState_Created) && window == 0 )
	return;
    setWState( WState_Created );			// set created flag

    if ( !parentWidget() || parentWidget()->isDesktop() )
	setWFlags( WType_TopLevel );		// top-level widget
    else if ( parentWidget()->isVisible() )
	parentWidget()->setChildrenAllocatedDirty();

    alloc_region_index = -1;
    alloc_region_revision = -1;
    isSettingGeometry = FALSE;
    overlapping_children = -1;

    static int sw = -1, sh = -1;		// screen size

    bool topLevel = testWFlags(WType_TopLevel);
    bool popup = testWFlags(WType_Popup);
    bool dialog = testWFlags(WType_Dialog);
    bool desktop = testWFlags(WType_Desktop);
    WId	   id;
    QWSDisplay* dpy = qwsDisplay();

    if ( !window )				// always initialize
	initializeWindow = TRUE;


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

    if ( dialog || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	dialog = popup = FALSE;			// force these flags off
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
	if ( name( 0 ) )
#ifndef QT_NO_WIDGET_TOPEXTRA
	    qwsDisplay()->nameRegion( winId(), name(), caption() );
#else
	    qwsDisplay()->nameRegion( winId(), name(), QString::null );
#endif
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

	qt_fbdpy->addProperty(id,QT_QWS_PROPERTY_WINDOWNAME);
	qt_fbdpy->setProperty(id,QT_QWS_PROPERTY_WINDOWNAME,0,name());

	// If we are session managed, inform the window manager about it
	if ( extra && !extra->mask.isNull() ) {
	    req_region = extra->mask;
	    req_region.translate(crect.x(),crect.y()); //###expensive?
	    req_region &= crect; //??? this is optional
	} else {
	    req_region = crect;
	}
    }
    //### frame rect is not handled in Qt/Embedded
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
	if ( testWFlags(WType_Desktop) ) {
	} else {
	    if ( destroyWindow ) {
		if ( isTopLevel() )
		    qwsDisplay()->destroyRegion( winId() );
	    }
	    if ( parentWidget() && parentWidget()->testWState(WState_Created) ) {
		hideWindow();
	    }
	}
	setWinId( 0 );
    }
}


void QWidget::reparentSys( QWidget *parent, WFlags f, const QPoint &p,
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

    if ( isTopLevel() )
	qwsDisplay()->destroyRegion( winId() );
    if ( parentWidget() && parentWidget()->testWState(WState_Created) )
	hideWindow();

    setWinId( 0 );

    if ( parentObj ) {				// remove from parent
	parentObj->removeChild( this );
	QWidget *p = parentWidget();
	if ( p ) {
	    p->setChildrenAllocatedDirty();
	    p->paintable_region_dirty = TRUE;
	}
	if ( old_winid && testWFlags(WType_TopLevel) )
	    qwsDisplay()->destroyRegion( old_winid );
    }
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
	QWidget *p = parentWidget();
	if ( p ) {
	    p->setChildrenAllocatedDirty();
	    p->paintable_region_dirty = TRUE;
	}
    }
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    //QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    //QColor   bgc    = bg_col;			// save colors
#ifndef QT_NO_WIDGET_TOPEXTRA
    QString capt= caption();
#endif
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
    if ( p.isNull() )
	resize( s );
    else
	setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    setFocusPolicy( fp );
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
	setCaption( capt );
    }
#endif
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
    reparentFocusWidgets( parent );		// fix focus chains
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

void QWidget::setMicroFocusHint( int x, int y, int width, int height,
				 bool /*text*/, QFont *)
{
    if ( QRect( x, y, width, height ) != microFocusHint() )
	extraData()->micro_focus_hint.setRect( x, y, width, height );

//XXX not implemented
#if 0
    if ( text ) {

    }
#endif
}


void QWidget::setFontSys( QFont * )
{
}


void QWidget::setBackgroundColorDirect( const QColor &color )
{
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    // XXX XSetWindowBackground( x11Display(), winId(), bg_col.pixel() );
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
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setErasePixmap(QPixmap());
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

#ifndef QT_NO_WIDGET_TOPEXTRA
void QWidget::setCaption( const QString &caption )
{
    if ( extra && extra->topextra && extra->topextra->caption == caption )
	return; // for less flicker
    createTLExtra();
    extra->topextra->caption = caption;
    qwsDisplay()->setCaption(this, caption);
    QEvent e( QEvent::CaptionChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setIcon( const QPixmap &unscaledPixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
    } else {
	createTLExtra();
    }
    QBitmap mask;
    if ( unscaledPixmap.isNull() ) {
    } else {
	QImage unscaledIcon = unscaledPixmap.convertToImage();
	QPixmap pixmap;
#ifndef QT_NO_IMAGE_SMOOTHSCALE
	pixmap.convertFromImage( unscaledIcon.smoothScale( 16, 16 ) );
#else
	pixmap.convertFromImage( unscaledIcon );
#endif
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
#endif

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
    if ( testWFlags(WType_TopLevel) ) {
	QRegion r( req_region );
#ifndef QT_NO_QWS_MANAGER
	if ( extra && extra->topextra && extra->topextra->qwsManager )
	    r += extra->topextra->qwsManager->region();
#endif
	qwsDisplay()->requestRegion(winId(), r);
	if ( !testWFlags(WStyle_Tool) )
	    qwsDisplay()->requestFocus(winId(),TRUE);
	qwsDisplay()->setAltitude( winId(),
	    testWFlags(WStyle_StaysOnTop) ? 1 : 0, TRUE );
    } else if ( !topLevelWidget()->in_show ) {
	QWidget *p = parentWidget();
	p->setChildrenAllocatedDirty();
	p->paintable_region_dirty = TRUE;
	p->overlapping_children = -1;
	update();
    }
}


void QWidget::hideWindow()
{
    deactivateWidgetCleanup();

    if ( testWFlags(WType_TopLevel) ) {
	qwsDisplay()->requestRegion(winId(), QRegion());
	qwsDisplay()->requestFocus(winId(),FALSE);
    } else {
	QWidget *p = parentWidget();
	p->setChildrenAllocatedDirty();
	p->paintable_region_dirty = TRUE;
	if ( p->overlapping_children )
	    p->overlapping_children = -1;
	bool v = testWState(WState_Visible);
	clearWState(WState_Visible);
	// Must post the event since the parent might well be destructing
	QApplication::postEvent(p,new QPaintEvent(geometry(),
		       !p->testWFlags(QWidget::WRepaintNoErase) ) );
	paint_children( p,geometry() );
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
    clearWState( WState_Maximized );
    setWState( WState_Minimized );
}

bool QWidget::isMinimized() const
{
    // true for non-toplevels that have the minimized flag, e.g. MDI children
    return FALSE || (!isTopLevel() && testWState( WState_Minimized ) ); // XXX
}

void QWidget::showMaximized()
{
    extern QRect qt_maxWindowRect;

    if ( testWFlags(WType_TopLevel) ) {
	createTLExtra();
#ifndef QT_NO_QWS_MANAGER
	if ( extra && extra->topextra && extra->topextra->qwsManager ) {
	    extra->topextra->qwsManager->maximize();
	} else
#endif
	{
	    setGeometry( qt_maxWindowRect );
	}
    }
    show();
    QEvent e( QEvent::ShowMaximized );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Minimized );
    setWState(WState_Maximized);
}

void QWidget::showNormal()
{
    if ( isTopLevel() ) {
	if ( topData()->fullscreen ) {
	    reparent( 0, WType_TopLevel, QPoint(0,0) );
	    topData()->fullscreen = 0;
	}
	QRect r = topData()->normalGeometry;
	if ( r.width() >= 0 ) {
	    topData()->normalGeometry = QRect(0,0,-1,-1);
	    setGeometry( r );
	}
    }
    show();
    clearWState( WState_Minimized | WState_Maximized );
}


void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    if ( isTopLevel() )
	qwsDisplay()->setAltitude( winId(), 0 );
    else if ( p ) {
	p->setChildrenAllocatedDirty();
	paint_children( p,geometry() );
    }
}

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    if ( isTopLevel() )
	qwsDisplay()->setAltitude( winId(), -1 );
    else if ( p ) {
	p->setChildrenAllocatedDirty();
	paint_children( p,geometry() );
    }
}

void QWidget::stackUnder( QWidget* w)
{
    QWidget *p = parentWidget();
    if ( !p || !w || isTopLevel() || p != w->parentWidget() )
	return;
    int loc = p->childObjects->findRef(w);
    if ( loc >= 0 && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( loc, p->childObjects->take() );
    if ( p ) {
	// #### excessive repaints
	p->setChildrenAllocatedDirty();
	paint_children( p,geometry() );
	paint_children( p,w->geometry() );
    }
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

    QRegion oldAlloc;
    if ( !isTopLevel() && isMove && ( w==olds.width() && h==olds.height() ) ) {
	oldAlloc = allocatedRegion();
    }

    crect = r;

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

    if ( isVisible() ) {
	isSettingGeometry = TRUE;
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
/*
	    if ( !testWFlags( WStaticContents ) ) {
		QApplication::postEvent(this,new QPaintEvent(visibleRect(),
					!testWFlags(WResizeNoErase) ) );
	    }
*/
	}

	QWidget *p = parentWidget();
	if ( !isTopLevel() || isResize ) {
	    if ( p && !isTopLevel() ) {
		p->paintable_region_dirty = TRUE;
		QRegion oldr( QRect(oldp, olds) );
		QRegion upd( ( QRegion(r) | oldr ) & p->rect() );
		if ( p->isSettingGeometry ) {
		    dirtyChildren.translate( x, y );
		    if ( oldp != r.topLeft() ) {
			dirtyChildren |= upd;
			QApplication::postEvent( this, new QPaintEvent(rect(),
			    !testWFlags(QWidget::WRepaintNoErase)) );
		    } else {
			dirtyChildren |= QRegion(r) - oldr;
			QApplication::postEvent( this, new QPaintEvent(rect(),
			    !testWFlags(QWidget::WResizeNoErase)) );
		    }
		    p->dirtyChildren |= dirtyChildren;
		} else {
#define FAST_WIDGET_MOVE
#ifdef FAST_WIDGET_MOVE
		    if ( isMove && ( w==olds.width() && h==olds.height() ) ) {
			QPoint gd = p->mapFromGlobal( QPoint(0,0) );
			oldAlloc.translate( gd.x() + x - oldp.x(), gd.y() + y - oldp.y() );
			QRegion alloc( allocatedRegion() );
			alloc.translate( gd.x(), gd.y() );
			QRegion scrollRegion( alloc & oldAlloc );
			if ( !scrollRegion.isEmpty() ) {
			    upd -= scrollRegion;

			    QGfx * gfx = p->graphicsContext(FALSE);
			    gfx->setClipRegion( scrollRegion );
			    gfx->scroll(x,y,w,h,oldp.x(),oldp.y());
			    delete gfx;
			}
		    }
#endif
		    if ( !upd.isEmpty() )
			QApplication::postEvent( p, new QPaintEvent(upd, TRUE) );
		    dirtyChildren.translate( x, y );
		    dirtyChildren |= upd;
		    paint_children( p, dirtyChildren & p->rect() );
		    p->setChildrenAllocatedDirty();
		}
		p->overlapping_children = -1;
	    } else {
		QApplication::postEvent( this, new QPaintEvent(rect(),
		    !testWFlags(QWidget::WRepaintNoErase)) );
		if ( oldp != r.topLeft() )
		    dirtyChildren |= rect();
		paint_children( this, dirtyChildren );
	    }
	}
#ifndef QT_NO_QWS_MANAGER
	if (isResize && extra && extra->topextra && extra->topextra->qwsManager) {
	    QApplication::postEvent(topData()->qwsManager,
				    new QPaintEvent(visibleRect(), TRUE ) );
	}
#endif
	isSettingGeometry = FALSE;
	dirtyChildren = QRegion();
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
#if defined(QT_CHECK_RANGE)
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
#if defined(QT_CHECK_RANGE)
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

void QWidget::erase( int x, int y, int w, int h )
{
    if ( backgroundMode() == NoBackground )
	return;

    erase( QRegion( x, y, w, h ) );

}

void QWidget::erase( const QRegion& reg )
{
    //this is experimental, I need to test more, but it seems in unclipped mode erasing shouldn't happen????
    if ( backgroundMode() == NoBackground || testWFlags(WPaintUnclipped) )
	return;

    QPainter p(this);
    p.setClipRegion( reg );
    if ( extra && extra->bg_pix ) {
	if ( !extra->bg_pix->isNull() ) {
	    int xoff = 0;
	    int yoff = 0;
	    if ( !isTopLevel() ) {
		if ( backgroundOrigin() == QWidget::ParentOrigin ) {
		    xoff = x();
		    yoff = y();
		} else if ( backgroundOrigin() == QWidget::WindowOrigin ) {
		    QPoint off = mapTo( topLevelWidget(), QPoint(0,0) );
		    xoff = off.x();
		    yoff = off.y();
		}
	    }
	    p.drawTiledPixmap(rect(),*extra->bg_pix,
			      QPoint(xoff%extra->bg_pix->width(),
				     yoff%extra->bg_pix->height()));
	}
    } else {
	p.fillRect(rect(),bg_col);
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
    QRegion copied = paintableRegion();
    QPoint p = mapToGlobal( QPoint() );
    copied.translate( -p.x(), -p.y() );
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
    QRegion reg = sr;
    QRegion tmp = paintableRegion();
    reg.translate( dx, dy );
    tmp.translate( -p.x(), -p.y() );
    tmp.translate( dx, dy );
    copied &= reg;
    copied &= tmp;
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
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	val = crect.width();
    } else if ( m == QPaintDeviceMetrics::PdmWidthMM ) {
	// 75 dpi is 3dpmm
	val = (crect.width()*100)/288;
    } else if ( m == QPaintDeviceMetrics::PdmHeight ) {
	val = crect.height();
    } else if ( m == QPaintDeviceMetrics::PdmHeightMM ) {
	val = (crect.height()*100)/288;
    } else if ( m == QPaintDeviceMetrics::PdmDepth ) {
	return qwsDisplay()->depth();
    } else if ( m == QPaintDeviceMetrics::PdmDpiX || m == QPaintDeviceMetrics::PdmPhysicalDpiX ) {
	return 72;
    } else if ( m == QPaintDeviceMetrics::PdmDpiY || m == QPaintDeviceMetrics::PdmPhysicalDpiY ) {
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

void QWidget::updateOverlappingChildren() const
{
    if ( overlapping_children != -1 || isSettingGeometry )
	return;

    QRegion r;
    const QObjectList *c = children();
    if ( c ) {
	QObjectListIt it(*c);
	QObject* ch;
	while ((ch=it.current())) {
	    ++it;
	    if ( ch->isWidgetType() && !((QWidget*)ch)->isTopLevel() ) {
		QWidget *w = (QWidget *)ch;
		QRegion rr( w->requestedRegion() );
		QRegion ir = r & rr;
		if ( !ir.isEmpty() ) {
		    overlapping_children = 1;
		    return;
		}
		r |= rr;
	    }
	}
    }
    overlapping_children = 0;
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
	    return QRegion();
	}
    }
}

void QWidget::setChildrenAllocatedDirty()
{
    const QObjectList *c = children();
    if ( c ) {
	QObjectListIt it(*c);
	QObject* ch;
	while ((ch=it.current())) {
	    ++it;
	    if ( ch->isWidgetType() ) {
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

inline bool QRect::intersects( const QRect &r ) const
{
    return ( QMAX( x1, r.x1 ) <= QMIN( x2, r.x2 ) &&
	     QMAX( y1, r.y1 ) <= QMIN( y2, r.y2 ) );
}

QRegion QWidget::allocatedRegion() const
{
    if (isVisible()) {
	if ( isTopLevel() ) {
	    return alloc_region;
	} else {
	    if ( isAllocatedRegionDirty() ) {
		const QObjectList *c;
		QRegion r( requestedRegion() );
		r &= parentWidget()->allocatedRegion();
		parentWidget()->updateOverlappingChildren();
		if ( parentWidget()->overlapping_children ) {
		    c = parentWidget()->children();
		    if ( c ) {
			QObjectListIt it(*c);
			QObject* ch;
			bool clip=FALSE;
			while ((ch=it.current())) {
			    ++it;
			    if ( ch->isWidgetType() ) {
				if ( ((QWidget*)ch) == this )
				    clip=TRUE;
				else if ( clip && !((QWidget*)ch)->isTopLevel()) {
				    if ( ((QWidget*)ch)->geometry().intersects( geometry() ) )
					r -= ((QWidget*)ch)->requestedRegion();
				}
			    }
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
			if ( ch->isWidgetType() && !((QWidget*)ch)->isTopLevel() ) {
			    ((QWidget *)ch)->alloc_region_dirty = TRUE;
			}
		    }
		}

		alloc_region = r;
		alloc_region_dirty = FALSE;
		paintable_region_dirty = TRUE;
	    }
	    return alloc_region;
	}
    } else {
	return QRegion();
    }
}

QRegion QWidget::paintableRegion() const
{
    if (isVisible()) {
	if ( paintable_region_dirty || isAllocatedRegionDirty() ) {
	    paintable_region = allocatedRegion();
	    const QObjectList *c = children();
	    if ( c ) {
		QObjectListIt it(*c);
		QObject* ch;
		while ((ch=it.current())) {
		    ++it;
		    if ( ch->isWidgetType() && !((QWidget*)ch)->isTopLevel() ) {
			paintable_region -= ((QWidget*)ch)->requestedRegion();
		    }
		}
	    }
	    paintable_region_dirty = FALSE;
	}
	if ( !isTopLevel() )
	    return paintable_region;
	else {
	    QRegion r( paintable_region );
#ifndef QT_NO_QWS_MANAGER
	    if (extra && extra->topextra)
		r += extra->topextra->decor_allocated_region;
#endif
	    return r;
	}
    }

    return QRegion();
}


void QWidget::setMask( const QRegion& region )
{
    alloc_region_dirty = TRUE;

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
	    parentWidget()->paintable_region_dirty = TRUE;
	    parentWidget()->repaint(geometry());
	    paint_children( parentWidget(),geometry() );
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

QGfx * QWidget::graphicsContext(bool clip_children) const
{
    QGfx * qgfx_qws;
    qgfx_qws=qwsDisplay()->screenGfx();
    QPoint offset=mapToGlobal(QPoint(0,0));
    QRegion r; // empty if not visible
    if ( isVisible() && topLevelWidget()->isVisible() ) {
	int rgnIdx = topLevelWidget()->alloc_region_index;
	if ( rgnIdx >= 0 ) {
	    r = clip_children ? paintableRegion() : allocatedRegion();
	    QWSDisplay::grab();
	    const int *rgnRev = qwsDisplay()->regionManager()->revision( rgnIdx );
	    if ( topLevelWidget()->alloc_region_revision != *rgnRev ) {
		// The TL region has changed, so we better make sure we're
		// not writing to any regions we don't own anymore.
		// We'll get a RegionModified event soon that will get our
		// regions back in sync again.
		QRegion req = qwsDisplay()->regionManager()->region( rgnIdx );
		QSize s( qt_screen->deviceWidth(), qt_screen->deviceHeight() );
		req = qt_screen->mapFromDevice( req, s );
		r &= req;
	    }
	    qgfx_qws->setGlobalRegionIndex( rgnIdx );
	    QWSDisplay::ungrab();
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
    if(base)
	base+=i*bytesPerLine();
    return base;
}

int QWidget::bytesPerLine() const
{
    return qt_screen->linestep();
}

bool QWidget::isMaximized() const
{
    return testWState(WState_Maximized);
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this;

    if( !isVisible() || isDesktop() ) {
	that->fstrut_dirty = isVisible();
	return;
    }

    //FIXME: need to fill in frame strut info
}

// This one
void QWidget::resetInputContext()
{
}
