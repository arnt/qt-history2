/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_mac.cpp $
**
** Implementation of QWidget and QWindow classes for mac
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qt_mac.h"

#include "qimage.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qabstractlayout.h"
#include "qtextcodec.h"
#include <qcursor.h>
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
#  include <qmenubar.h>
#endif

/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS


/*****************************************************************************
  QWidget globals
 *****************************************************************************/
static bool no_move_blt = FALSE;
static WId serial_id = 0;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
int mac_window_count = 0;


/*****************************************************************************
  Externals
 *****************************************************************************/
void qt_event_request_updates();


/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/

QPoint posInWindow(QWidget *w)
{
    if(w->isTopLevel())
	return QPoint(0, 0);
    int x = 0, y = 0;
    if(w->parentWidget()) {
	QPoint p = posInWindow(w->parentWidget());
	x = p.x() + w->geometry().x();
	y = p.y() + w->geometry().y();
    }
    return QPoint(x, y);
}

#ifdef DEBUG_WINDOW_RGNS
static inline void debug_wndw_rgn(const char *where, QWidget *w, const QRegion &r, bool paint=FALSE) {
    QPoint mp(posInWindow(w));
    qDebug("%s %s %s (%s) [ %d %d %d %d ]", where, paint ? "paint" : "invalid", 
	   w->className(), w->name(), mp.x(), mp.y(), w->width(), w->height());
    QArray<QRect> rs = r.rects();
    int offx = 0, offy = 0;
    if(paint) {
	offx = mp.x();
	offy = mp.y();
    }
    for(int i = 0; i < (int)rs.size(); i++) 
	qDebug("%d %d %d %d", rs[i].x()+offx, rs[i].y()+offy, rs[i].width(), rs[i].height());
}
static inline void debug_wndw_rgn(const char *where, QWidget *w, const Rect *r, bool paint=FALSE) {
    debug_wndw_rgn(where, w, QRegion(r->left, r->top, r->right - r->left, r->bottom - r->top), paint);
}
static inline void dirty_wndw_rgn(const char *where, QWidget *w, const QRegion &r)
{
    debug_wndw_rgn(where, w, r, FALSE);
    InvalWindowRgn((WindowPtr)w->handle(), (RgnHandle)r.handle());
}
static inline void dirty_wndw_rgn(const char *where, QWidget *w, const Rect *r)
{
    debug_wndw_rgn(where, w, r, FALSE);
    InvalWindowRect((WindowPtr)w->handle(), r);
}
#define clean_wndw_rgn(x, y, z) debug_wndw_rgn(x, y, z, TRUE);
#else
static inline void dirty_wndw_rgn_internal(const WindowPtr p, const QRegion &r) 
{ 
    InvalWindowRgn(p, (RgnHandle)r.handle()); 
}
static inline void dirty_wndw_rgn_internal(const WindowPtr p, const Rect *r) 
{ 
    InvalWindowRect(p, r); 
}
#define dirty_wndw_rgn(x, who, where) dirty_wndw_rgn_internal((WindowPtr)who->handle(), where)
#define clean_wndw_rgn(x, y, z)
#define debug_wndw_rgn(w, x, y)
#endif

static inline const Rect *mac_rect(const QRect &qr) 
{
    static Rect r;
    SetRect(&r, qr.left(), qr.top(), qr.right()+1, qr.bottom()+1); //qt says be inclusive!
    return &r;
}
static inline const Rect *mac_rect(const QPoint &qp, const QSize &qs) { return mac_rect(QRect(qp, qs)); }

enum paint_children_ops { 
    PC_None = 0x00,
    PC_Now = 0x01,
    PC_ForceErase = 0x02,
    PC_NoPaint = 0x04,
    PC_NoErase = 0x08
};
static void paint_children(QWidget * p,QRegion r, uchar ops = PC_ForceErase)
{
    if(!p || r.isEmpty() || !p->isVisible() || qApp->closingDown() || qApp->startingUp())
	return;

    QPoint point(posInWindow(p));
    bool r_is_empty = FALSE;
    if(QObjectList * childObjects=(QObjectList*)p->children()) {
	QObjectListIt it(*childObjects);
	for(it.toLast(); it.current(); --it) {
	    if( (*it)->isWidgetType() ) {
		QWidget *w = (QWidget *)(*it);
		if ( w->topLevelWidget() == p->topLevelWidget() && w->isVisible() ) {
		    QRegion wr = w->clippedRegion(FALSE);
		    wr.translate(-point.x(), -point.y());
		    wr &= r;
		    if ( !wr.isEmpty() ) {
			r -= wr;
			wr.translate( -w->geometry().x(), -w->geometry().y() );
			paint_children(w, wr, ops);
			if((r_is_empty = r.isEmpty()))
			    break;
		    }
		}
	    }
	}
    }

    if(!r_is_empty) {
	bool erase = !(ops & PC_NoErase) && ((ops & PC_ForceErase) || !p->testWFlags(QWidget::WRepaintNoErase));
	if((ops & PC_NoPaint)) {
	    if(erase)
		p->erase(r);
	} else {
	    if(ops & PC_Now) {
		p->repaint(r, erase);
	    } else {
		bool painted = FALSE;
		if(!p->testWState(QWidget::WState_BlockUpdates)) {
		    painted = TRUE;
		    clean_wndw_rgn("**paint_children",p, r);
		    p->repaint(r, erase);
		} else if(erase) {
		    erase = FALSE;
		    p->erase(r);
		}
		if(!painted) {
		    QRegion pa(r);
		    pa.translate(point.x(), point.y());
		    dirty_wndw_rgn("paint_children",p, pa);
		}
	    }
	}
    }
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping( QPaintDevice *dev );

static void *qt_root_win() {
    WindowPtr ret = NULL;
#if 0 //FIXME NEED TO FIGURE OUT HOW TO GET DESKTOP
    GetCWMgrPort(ret);
#endif
    return (void *) ret;
}

QMAC_PASCAL OSStatus macSpecialErase(GDHandle, GrafPtr, WindowRef window, RgnHandle rgn,
			 RgnHandle, void *w)
{
    QWidget *widget = (QWidget *)w;
    if(!widget)
	widget = QWidget::find( (WId)window );

    if ( widget ) {
        QRegion reg(rgn);
	QRect oldcrect = widget->crect;
        { //lookup the x and y, don't use qwidget because this callback and be called before its updated
	    Point px = { 0, 0 };
	    QMacSavedPortInfo si(widget);
	    LocalToGlobal(&px);
	    reg.translate(-px.h, -px.v);
	    widget->crect.setRect( px.h, px.v, widget->width(), widget->height() );
	    widget->fstrut_dirty = TRUE;
        }
	paint_children(widget, reg, PC_Now | PC_ForceErase );
	widget->crect = oldcrect; //restore
	widget->fstrut_dirty = TRUE;
    }
    return 0;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

//FIXME How can I create translucent windows? (Need them for pull down menus)
void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow  )
{
    own_id = 0;
    HANDLE root_win = qt_root_win(), destroyw = 0;
    setWState( WState_Created );                        // set created flag

    if ( !parentWidget() || parentWidget()->isDesktop() )
	setWFlags( WType_TopLevel );            // top-level widget

    static short int sw = -1, sh = -1;                // screen size
    bool topLevel = testWFlags( WType_TopLevel );
    bool popup = testWFlags( WType_Popup );
    bool dialog = testWFlags( WType_Dialog );
    bool desktop = testWFlags( WType_Desktop );
    WId    id;

    if ( !window )                              // always initialize
	initializeWindow=TRUE;

    if ( sw < 0 ) {
	GDHandle g = GetMainDevice();
	if(g) {
	    sw = (*g)->gdRect.right;
	    sh = (*g)->gdRect.bottom;
	}
    }

    bg_col = pal.normal().background();

    if ( dialog || popup || desktop ) {          // these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }
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
    if ( !testWFlags(WStyle_Customize) && !(desktop || popup))
	setWFlags( WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );

    if ( desktop ) {                            // desktop widget
	dialog = popup = FALSE;                  // force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {                    // calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {                                    // child widget
	crect.setRect( 0, 0, 100, 30 );
    }

    HANDLE parentw = topLevel ? root_win : parentWidget()->handle();
    if ( window ) {				// override the old window
	if ( destroyOldWindow && own_id )
	    destroyw = hd;
	own_id = 1; //it has become mine!
	id = window;
	hd = (void *)id;
	setWinId(id);
    } else if ( desktop ) {			// desktop widget
	id = (WId)parentw;			// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else if( !parentWidget() || (popup || dialog) ) {
	own_id = 1; //I created it, I own it

	Rect r;
	SetRect(&r, crect.left(), crect.top(), crect.right(), crect.bottom());

	WindowClass wclass = kSheetWindowClass;

	if(testWFlags(WType_Dialog) || testWFlags(WType_Popup) )
	    wclass = kToolbarWindowClass;
	else
	    wclass = kDocumentWindowClass;

	WindowAttributes wattr = kWindowNoAttributes;
	if( testWFlags(WStyle_Customize) ) {
	    if ( testWFlags(WStyle_NormalBorder) || testWFlags( WStyle_DialogBorder) ) {
		if(wclass == kToolbarWindowClass)
		    wclass = kDocumentWindowClass;
		if(wclass == kDocumentWindowClass )
		    wattr |= kWindowStandardDocumentAttributes;
	    } else {

		//FIXME I shouldn't have to do this
		if(wclass == kDocumentWindowClass )
#ifdef Q_WS_MACX
		    wclass = kSheetWindowClass;
#else
                    wclass = kToolbarWindowClass;
#endif

		if( testWFlags( WStyle_Maximize ) )
		    wattr |= kWindowFullZoomAttribute;
		if( testWFlags( WStyle_Minimize ) )
		    wattr |= kWindowCollapseBoxAttribute;
		if( testWFlags( WStyle_SysMenu ) )
		    wattr |= kWindowCloseBoxAttribute;
	    }
	}
	CreateNewWindow(wclass, wattr, &r, (WindowRef *)&id);
	InstallWindowContentPaintProc((WindowPtr)id, NewWindowPaintUPP(macSpecialErase), 0, this);
//	ChangeWindowAttributes((WindowPtr)id, kWindowNoBufferingAttribute, 0);
	fstrut_dirty = TRUE; // when we create a toplevel widget, the frame strut should be dirty

	if(testWFlags( WType_Popup ))
	    SetWindowModality((WindowPtr)id, kWindowModalityNone, NULL);
	if(!mac_window_count++)
	    QMacSavedPortInfo::setPaintDevice(this);

	hd = (void *)id;
	setWinId(id);

    } else {
	while(QWidget::find(++serial_id));
	setWinId(serial_id);
	id = serial_id;
	hd = topLevelWidget()->hd;
	fstrut_dirty = FALSE; // non-toplevel widgets don't have a frame, so no need to update the strut
	setWinId(id);
    }

    bg_col = pal.normal().background();
    setWState( WState_MouseTracking );
    setMouseTracking( FALSE );                  // also sets event mask
    clearWState(WState_Visible);
    dirtyClippedRegion(TRUE);
    macDropEnabled = false;

    if ( destroyw ) {
	mac_window_count--;
	DisposeWindow((WindowPtr)destroyw);
    }
}

void qt_mac_destroy_widget(QWidget *w);

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    deactivateWidgetCleanup();
    if ( testWState(WState_Created) ) {
	dirtyClippedRegion(TRUE);
        clearWState( WState_Created );
        if ( children() ) {
            QObjectListIt it(*children());
            register QObject *obj;
            while ( (obj=it.current()) ) {      // destroy all widget children
                ++it;
                if ( obj->isWidgetType() )
                    ((QWidget*)obj)->destroy(destroySubWindows, destroySubWindows);
            }
        }
	if ( mac_mouse_grabber == this )
	    releaseMouse();
	if ( mac_keyboard_grabber == this )
	    releaseKeyboard();
	if ( acceptDrops() )
	    setAcceptDrops(FALSE);

        if ( testWFlags(WShowModal) )          // just be sure we leave modal
            qt_leave_modal( this );
        else if ( testWFlags(WType_Popup) )
            qApp->closePopup( this );
	if ( testWFlags(WType_Desktop) ) {
	} else {
	    if ( destroyWindow && isTopLevel() && hd && own_id) {
		mac_window_count--;
	        DisposeWindow( (WindowPtr)hd );
	    }
	}
    }
    hd=0;
    setWinId( 0 );
    qt_mac_destroy_widget(this);
}

void QWidget::reparent( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    dirtyClippedRegion(TRUE);

    QCursor oldcurs;
    QPoint oldposinwindow(posInWindow(this));
    bool setcurs=testWState(WState_OwnCursor);
    if ( setcurs ) {
	oldcurs = cursor();
	unsetCursor();
    }

    WindowPtr old_hd = testWFlags(WType_Desktop) ? NULL : (WindowPtr)hd;
    reparentFocusWidgets( parent );		// fix focus chains
    if ( old_hd && own_id && isTopLevel() ) { //don't need old window anymore
	mac_window_count--;
	DisposeWindow( old_hd );
    }

    setWinId( 0 );
    if ( parentObj ) {				// remove from parent
	QObject *oldp = parentObj;
	parentObj->removeChild( this );
	if(isVisible() && !isTopLevel() && oldp->isWidgetType()) 
	    dirty_wndw_rgn("reparent1",this, mac_rect(oldposinwindow, geometry().size()));
    }

    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     dropable = acceptDrops();
    bool     enable = isEnabled();
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible | WState_ForceHide );
    if ( isTopLevel() || (!parent || parent->isVisibleTo( 0 ) ) )
	setWState( WState_ForceHide );	// new widgets do not show up in already visible parents
    if(dropable)
	setAcceptDrops(FALSE);

    //get new hd, now move
    create();
    no_move_blt = TRUE;
    if ( p.isNull() )
	resize( s );
    else
	setGeometry( p.x(), p.y(), s.width(), s.height() );
    no_move_blt = FALSE;

    //reset flags and show (if neccesary)
    setEnabled( enable );
    setFocusPolicy( fp );
    setAcceptDrops(dropable);
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( setcurs )
	setCursor(oldcurs);

    //reparent children
    if(QObjectList	*chldn = queryList()) {
	QObjectListIt it( *chldn );
	for ( QObject *obj; (obj=it.current()); ++it ) {
	    if(obj->inherits("QAccel"))
		((QAccel*)obj)->repairEventFilter();
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget *)obj;
		if(((WindowPtr)w->hd) == old_hd)
		    w->hd = hd; //all my children hd's are now mine!
	    }
	}
	delete chldn;
    }
#if !defined(QMAC_QMENUBAR_NO_NATIVE)  //make sure menubars are fixed
    if(QObjectList *menus = queryList("QMenuBar")) {
	QObjectListIt menuit( *menus );
	for ( QMenuBar *mb; (mb=(QMenuBar *)menuit.current()); ++menuit ) {
	    int was_eaten = mb->mac_eaten_menubar;
	    mb->macRemoveNativeMenubar();
	    mb->macCreateNativeMenubar();
	    if(was_eaten)
		mb->menuContentsChanged();
	}
	delete menus;
    }
#endif

    if ( !parent ) {
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
    }

    //repaint the new area, on the window parent
    if(isVisible()) //finally paint my new area 
	dirty_wndw_rgn("reparent2",this, mac_rect(posInWindow(this), geometry().size()));

    //send the reparent event
    QEvent e( QEvent::Reparent );
    QApplication::sendEvent( this, &e );
}


QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    Point mac_p;
    QPoint mp(posInWindow(((QWidget *)this)));
    mac_p.h = mp.x() + pos.x();
    mac_p.v = mp.y() + pos.y();
    if(handle()) {
	QMacSavedPortInfo savedInfo(((QWidget *)this));
	LocalToGlobal(&mac_p);
    }
    return QPoint(mac_p.h, mac_p.v);
}


QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    Point mac_p;
    mac_p.h = pos.x();
    mac_p.v = pos.y();
    if(handle()) {
	QMacSavedPortInfo savedInfo(((QWidget *)this));
	GlobalToLocal(&mac_p);
    }
    for(const QWidget *p = this; p && !p->isTopLevel(); p = p->parentWidget()) {
	mac_p.h -= p->geometry().x();
	mac_p.v -= p->geometry().y();
    }
    return QPoint(mac_p.h, mac_p.v);
}


void QWidget::setMicroFocusHint(int, int, int, int, bool, QFont * )
{
}

void QWidget::setFontSys( QFont * )
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

    if(isTopLevel()) {
	QMacSavedPortInfo savedInfo(this);
	RGBColor f;
	f.red = bg_col.red() * 256;
	f.green = bg_col.green() * 256;;
	f.blue = bg_col.blue() * 256;
	RGBBackColor(&f);
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
    }
    if ( !allow_null_pixmaps ) {
	backgroundPixmapChange( old );
    }
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setErasePixmap(QPixmap());
    allow_null_pixmaps--;
}


void QWidget::setCursor( const QCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle() || (extra && extra->curs) ) {
	createExtra();
	delete extra->curs;
	extra->curs = new QCursor(cursor);
    }
    setWState( WState_OwnCursor );
}


void QWidget::unsetCursor()
{
    if ( !isTopLevel() ) {
	if (extra ) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	clearWState( WState_OwnCursor );
    }
}

void QWidget::setCaption( const QString &cap )
{
    if ( extra && extra->topextra && extra->topextra->caption == cap )
	return; // for less flicker
    createTLExtra();
    extra->topextra->caption = cap;
    if(isTopLevel()) {
	CFStringRef str = CFStringCreateWithCharacters(NULL, (UniChar *)cap.unicode(), cap.length());
	SetWindowTitleWithCFString((WindowPtr)hd, str);
    }
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
    if ( !pixmap.isNull() ) {
	extra->topextra->icon = new QPixmap( pixmap );
	mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
    }
}

void QWidget::setIconText( const QString &iconText )
{
    createTLExtra();
    extra->topextra->iconText = iconText;
}

#ifndef QT_NO_ACCESSIBILITY

void QWidget::setAccessibilityHint( const QString &hint )
{
    if ( topLevelWidget() )
	return;

    if ( !extra )
	createExtra();
    extra->accessibility_hint = hint;
}

#endif

void QWidget::grabMouse()
{
    mac_mouse_grabber=this;
}

void QWidget::grabMouse( const QCursor & )
{
    mac_mouse_grabber=this;
}

void QWidget::releaseMouse()
{
    mac_mouse_grabber = NULL;
}

void QWidget::grabKeyboard()
{
    mac_keyboard_grabber = this;
}

void QWidget::releaseKeyboard()
{
    mac_keyboard_grabber = NULL;
}


QWidget *QWidget::mouseGrabber()
{
    return mac_mouse_grabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return mac_keyboard_grabber;
}


void QWidget::setActiveWindow()
{
    if(!isVisible() || !isTopLevel() || isPopup() || testWFlags( WStyle_Tool ))
	return;
    SelectWindow( (WindowPtr)hd );
    qApp->setActiveWindow(this);
}


void QWidget::update()
{
    update( 0, 0, width(), height() );
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( !testWState(WState_BlockUpdates) && testWState( WState_Visible ) && isVisible() ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w && h ) {
	    QPoint p(posInWindow(this));
	    dirty_wndw_rgn("update",this, mac_rect(QRect(p.x() + x, p.y() + y, w, h)));
	}
    }
}

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    QRect r(x,y,w,h);
    if ( r.isEmpty() )
	return; // nothing to do
    repaint(QRegion(r), erase); //general function..
}

void QWidget::repaint( const QRegion &reg , bool erase )
{
    if ( !testWState(WState_BlockUpdates) && isVisible() ) {
	setWState( WState_InPaintEvent );
	qt_set_paintevent_clipping( this, reg );
	if ( erase )
	    this->erase(reg);

	QPaintEvent e( reg );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping( this );
	clearWState( WState_InPaintEvent );

#if 1	//When repaint is called the user probably expects a screen update?
	QPoint p(posInWindow(this));
	QRegion clean(reg);
	clean.translate(p.x(), p.y());
	clean &= clippedRegion();
	if(QDIsPortBuffered(GetWindowPort((WindowPtr)hd)))
	    QDFlushPortBuffer(GetWindowPort((WindowPtr)hd), (RgnHandle)clean.handle());
#endif
    }
}

void QWidget::showWindow()
{
    dirtyClippedRegion(TRUE);
    if ( isTopLevel() ) {
	//ick, this is needed because docks are updated by it and mac paints immediatly. FIXME
	QApplication::sendPostedEvents(this, QEvent::LayoutHint);

#ifdef Q_WS_MACX
	//handle transition
	if(qApp->style().inherits("QAquaStyle") && parentWidget() && testWFlags(WShowModal)) 
	    TransitionWindowAndParent((WindowPtr)hd, (WindowPtr)parentWidget()->hd,
				      kWindowSheetTransitionEffect, kWindowShowTransitionAction, NULL);
#endif
	//now actually show it
	ShowHide((WindowPtr)hd, 1);
	setActiveWindow();
    } else { 
	dirty_wndw_rgn("showwindow",this, mac_rect(posInWindow(this), geometry().size()));
    }
}

void QWidget::hideWindow()
{
    deactivateWidgetCleanup();
    dirtyClippedRegion(TRUE);
    if ( isTopLevel() ) {
	ShowHide((WindowPtr)hd, 0);
	if(QWidget *widget = parentWidget() ? parentWidget() : QWidget::find((WId)FrontWindow()))
	    widget->setActiveWindow();
	fstrut_dirty = TRUE;
    } else if(isVisible()) {
	dirty_wndw_rgn("hidewindow",this, mac_rect(posInWindow(this), geometry().size()));
    }
}



bool QWidget::isMinimized() const
{
    // true for non-toplevels that have the minimized flag, e.g. MDI children
    return IsWindowCollapsed((WindowRef)hd) || ( !isTopLevel() && testWState( WState_Minimized ) );
}

bool QWidget::isMaximized() const
{
    return testWState(WState_Maximized);
}


void QWidget::showMinimized()
{
    if(isMinimized())
	return;
    if ( isTopLevel() ) {
	if ( isVisible() ) {
	    CollapseWindow((WindowPtr)hd, TRUE);
        } else {
	    topData()->showMode = 1;
	    show();
	    clearWState( WState_Visible );
	    sendHideEventsToChildren(TRUE);
	}
    }
    QEvent e( QEvent::ShowMinimized );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Maximized );
    setWState( WState_Minimized );
}

void QWidget::showMaximized()
{
    if(isMaximized())
	return;
    if ( testWFlags(WType_TopLevel) ) {
	Rect bounds;
	GetPortBounds( GetWindowPort( (WindowPtr)hd ), &bounds );
	ZoomWindow( (WindowPtr)hd, inZoomOut, FALSE);
	GetPortBounds( GetWindowPort( (WindowPtr)hd ), &bounds );
	dirty_wndw_rgn("showMaxim",this, &bounds);

	QRect orect(geometry().x(), geometry().y(), width(), height());
	QMacSavedPortInfo savedInfo(this);
	Point p = { 0, 0 };
	LocalToGlobal(&p);
	crect.setRect( p.h, p.v, bounds.right, bounds.bottom );

	if(isVisible()) {
	    dirtyClippedRegion(TRUE);

	    //issue a move
	    QMoveEvent qme( pos(), orect.topLeft());
	    QApplication::sendEvent( this, &qme );
	    //issue a resize
	    QResizeEvent qre( size(), orect.size());
	    QApplication::sendEvent( this, &qre );
	}
    }
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
	    reparent( 0, WType_TopLevel, QPoint(0,0) );
	    topData()->fullscreen = 0;
	}
	{
	    Rect bounds;
	    GetPortBounds( GetWindowPort( (WindowPtr)hd ), &bounds );
	    ZoomWindow( (WindowPtr)hd, inZoomIn, FALSE);
	    GetPortBounds( GetWindowPort( (WindowPtr)hd ), &bounds );
	    dirty_wndw_rgn("showNormal",this, &bounds);

	    QRect orect(x(), y(), width(), height());
	    QMacSavedPortInfo savedInfo(this);
	    Point p = { 0, 0 };
	    LocalToGlobal(&p);
	    crect.setRect( p.h, p.v, bounds.right, bounds.bottom );

	    if(isVisible()) {
		dirtyClippedRegion(TRUE);

		//issue a move
		QMoveEvent qme( pos(), orect.topLeft());
		QApplication::sendEvent( this, &qme );
		//issue a resize
		QResizeEvent qre( size(), orect.size());
		QApplication::sendEvent( this, &qre );
	    }
	}

	QRect r = topData()->normalGeometry;
	if ( r.width() >= 0 ) {
	    // the widget has been maximized
	    topData()->normalGeometry = QRect(0,0,-1,-1);
	    resize( r.size() );
	    move( r.topLeft() );
	}
    }
    dirtyClippedRegion(TRUE);
    show();
    QEvent e( QEvent::ShowNormal );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Minimized | WState_Maximized );
}


void QWidget::raise()
{
    if(isTopLevel()) {
	BringToFront((WindowPtr)hd);
	setActiveWindow();
    } else if(QWidget *p = parentWidget()) {

	QRegion clp;
	if(isVisible())
	    clp = clippedRegion(FALSE);
	if ( p->childObjects && p->childObjects->findRef(this) >= 0 )
	    p->childObjects->append( p->childObjects->take() );
	if(isVisible()) {
	    dirtyClippedRegion(TRUE);
	    clp ^= clippedRegion(FALSE);
	    dirty_wndw_rgn("raise",this, clp);
	}
    }
}

void QWidget::lower()
{
    if ( isTopLevel() )
	SendBehind((WindowPtr)handle(), NULL);
    else if(QWidget *p = parentWidget()) {
	QRegion clp;
	if(isVisible())
	    clp = clippedRegion(FALSE);
	if ( p->childObjects && p->childObjects->findRef(this) >= 0 )
	    p->childObjects->insert( 0, p->childObjects->take() );
	if(isVisible()) {
	    dirtyClippedRegion(TRUE);
	    clp ^= clippedRegion(FALSE);
	    dirty_wndw_rgn("lower",this, clp);
	}
    }
}


void QWidget::stackUnder( QWidget *w )
{
    if ( !w || isTopLevel() )
	return;

    QWidget *p = parentWidget();
    if(!p || p != w->parentWidget())
	return;
    int loc = p->childObjects->findRef(w);
    QRegion clp;
    if(isVisible())
	clp = clippedRegion(FALSE);
    if ( loc >= 0 && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( loc, p->childObjects->take() );
    if(isVisible()) {
	dirtyClippedRegion(TRUE);
	clp ^= clippedRegion(FALSE);
	dirty_wndw_rgn("stackUnder",this, clp);
    }
}


void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    if ( testWFlags(WType_Desktop) )
	return;

    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);

	// Deal with size increment
	if ( extra->topextra ) {
	    if ( extra->topextra->incw ) {
		w = w/extra->topextra->incw;
		w = w*extra->topextra->incw;
	    }
	    if ( extra->topextra->inch ) {
		h = h/extra->topextra->inch;
		h = h*extra->topextra->inch;
	    }
	}
    }

    if ( w < 1 )                                // invalid size
	w = 1;
    if ( h < 1 )
	h = 1;

    QPoint oldp = pos();
    QSize  olds = size();

    QRegion oldregion = clippedRegion(FALSE);

    QRect  r( x, y, w, h );
    dirtyClippedRegion(FALSE);
    crect = r;

    if (!isTopLevel() && size() == olds && oldp == pos() )
	return;
    dirtyClippedRegion(TRUE);

    bool isResize = (olds != size());
    if(isTopLevel() && winid && own_id) {
	if(isResize && isMove && isVisible()) {
	    Rect r;
	    SetRect(&r, x, y, x + w, y + h);
	    SetWindowBounds((WindowPtr)hd, kWindowContentRgn, &r);
	} else {
	    if(isResize)
		SizeWindow((WindowPtr)hd, w, h, 1);
	    if(isMove) 
		MoveWindow((WindowPtr)hd, x, y, 1);
	}
    }

    if(isMove || isResize) {
	if(!isVisible()) {
	    if ( isMove )
		QApplication::postEvent( this, new QMoveEvent( pos(), oldp ) );
	    if ( isResize )
		QApplication::postEvent( this, new QResizeEvent( size(), olds ) );
	} else {
	    QRegion bltregion, clpreg = clippedRegion(FALSE);
	    if( !oldregion.isNull() ) {
		//setup the old clipped region..
		bltregion = oldregion;
		if(isMove && !isTopLevel()) 
		    bltregion.translate(pos().x() - oldp.x(), pos().y() - oldp.y());
		bltregion &= clpreg;
		{   //can't blt that which is dirty
		    RgnHandle r = NewRgn();
		    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, r);
		    if(!EmptyRgn(r)) {
			QRegion dirty(r); //the dirty region
			dirty.translate(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
			if(isMove && !isTopLevel()) //need to be in new coords
			    dirty.translate(pos().x() - oldp.x(), pos().y() - oldp.y());
			bltregion -= dirty;
		    }
		    DisposeRgn(r);
		}

		if(isMove && !no_move_blt && !isTopLevel()) {
		    QWidget *parent = parentWidget() ? parentWidget() : this;
		    QPoint tp(posInWindow(parent));
		    int px = tp.x(), py = tp.y();

		    //save the window state, and do the grunt work
		    int ow = olds.width(), oh = olds.height();
		    QMacSavedPortInfo saveportstate(this);
		    ::RGBColor f;
		    f.red = f.green = f.blue = 0;
		    RGBForeColor( &f );
		    f.red = f.green = f.blue = ~0;
		    RGBBackColor( &f );
		    PenMode(patCopy);

		    //calculate new and old rectangles
		    int nx = px + pos().x(), ny = py + pos().y();  //new
		    Rect newr; SetRect(&newr,nx, ny, nx + ow, ny + oh);
		    int ox = px + oldp.x(), oy = py + oldp.y(); //old
		    Rect oldr; SetRect(&oldr, ox, oy, ox+ow, oy+oh);
		    BitMap *scrn = (BitMap *)*GetPortPixMap(GetWindowPort((WindowPtr)handle()));
		    SetClip((RgnHandle)bltregion.handle());
		    CopyBits(scrn, scrn, &oldr, &newr, srcCopy, NULL);
		}
	    }
	    if(isResize)
		qt_event_request_updates();

	    //finally issue "expose" event
	    QRegion upd((oldregion + clpreg) - bltregion); 
	    if(isResize && !testWFlags(WNorthWestGravity))
		upd += clippedRegion();
	    dirty_wndw_rgn("internalSetGeometry",this, upd);

	    //Do these last, as they may cause an event which paints, and messes up
	    //what we blt above
	    if ( isMove ) { //send the move event..
		QMoveEvent e( pos(), oldp );
		QApplication::sendEvent( this, &e );
	    }
	    if ( isResize ) { //send the resize event..
		QResizeEvent e( size(), olds );
		QApplication::sendEvent( this, &e );
	    }
	}
    }
}

void QWidget::setMinimumSize( int minw, int minh)
{
    //I'm not happy to be doing this, but apparently this helps (ie on a mainwindow, so the
    //status bar doesn't fall of the bottom) this might need a FIXME!!!
    if(isTopLevel() && !parentWidget() && !isPopup()) {
	minw+=10;
	minh+=10;
    }

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

void QWidget::setMaximumSize( int maxw, int maxh)
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

void QWidget::erase( int x, int y, int w, int h )
{
    erase( QRegion( x, y, w, h ) );
}

void QWidget::erase( const QRegion& reg )
{
    if ( backgroundMode() == NoBackground )
	return;
    QRect rr(reg.boundingRect());

    int xoff = 0;
    int yoff = 0;
    if ( !isTopLevel() ) {
	if( backgroundOrigin() == QWidget::ParentOrigin ) {
	    xoff = x();
	    yoff = y();
	} else if(backgroundOrigin() == QWidget::WindowOrigin ) {
	    QPoint mp(posInWindow(this));
	    xoff = mp.x();
	    yoff = mp.y();
	}
    }

    bool unclipped = testWFlags( WPaintUnclipped );
    clearWFlags( WPaintUnclipped );
    QPainter p( this );
    if ( unclipped )
	setWFlags( WPaintUnclipped );

    p.setClipRegion(reg);
    if ( extra && extra->bg_pix ) {
	if ( !extra->bg_pix->isNull() ) {
	    QPoint point(rr.x()+(xoff%extra->bg_pix->width()), rr.y()+(yoff%extra->bg_pix->height()));
	    p.drawTiledPixmap(rr,*extra->bg_pix, point);
	}
    } else {
	p.fillRect(rr, bg_col);
    }
#if 0
    p.flush();
#endif
    p.end();
}


void QWidget::scroll( int dx, int dy)
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
    if ( w > 0 && h > 0 ) 
	bitBlt(this,x2,y2,this,x1,y1,w,h);

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

    QPoint p(posInWindow(this));
    QRegion copied(clippedRegion());
    copied.translate( -p.x(), -p.y() );
    copied &= QRegion(sr);
    copied.translate(dx,dy);
    repaint( QRegion(sr) - copied, !testWFlags(WRepaintNoErase) );
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
    switch(m) {
    case QPaintDeviceMetrics::PdmHeightMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmHeight)*100)/288;
    case QPaintDeviceMetrics::PdmWidthMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmWidth)*100)/288;
    case QPaintDeviceMetrics::PdmWidth:
    {
	if ( !isTopLevel() )
	    return crect.width();
	Rect windowBounds;
	GetPortBounds( GetWindowPort( ((WindowPtr)hd) ), &windowBounds );
	return windowBounds.right;
    }

    case QPaintDeviceMetrics::PdmHeight:
    {
	if ( !isTopLevel() )
	    return crect.height();
	Rect windowBounds;
	GetPortBounds( GetWindowPort( ((WindowPtr)hd) ), &windowBounds );
	return windowBounds.bottom;
    }
    case QPaintDeviceMetrics::PdmDepth:// FIXME : this is a lie in most cases
	return 16;
    case QPaintDeviceMetrics::PdmDpiX: // FIXME : this is a lie in most cases
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
	return 80;
    case QPaintDeviceMetrics::PdmDpiY: // FIXME : this is a lie in most cases
    case QPaintDeviceMetrics::PdmPhysicalDpiY:
	return 80;
    default: //leave this so the compiler complains when new ones are added
	qWarning("QWidget::metric unhandled parameter %d",m);
	return QPaintDevice::metric(m);// XXX
    }
    return 0;
}

void QWidget::createSysExtra()
{
    extra->child_dirty = extra->clip_dirty = TRUE;
    extra->macDndExtra = 0;
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
    return macDropEnabled;
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this; //mutable
    if(isDesktop() || !fstrut_dirty) {
	that->fstrut_dirty = isVisible();
	return;
    }
    that->fstrut_dirty = FALSE;

    that->fstrut_dirty = FALSE;
    QTLWExtra *top = that->topData();
    top->fleft = top->fright = top->ftop = top->fbottom = 0;
    if(isTopLevel()) {
	Rect window_r, content_r;
	//get bounding rects
	RgnHandle rgn = NewRgn();
	GetWindowRegion((WindowPtr)hd, kWindowStructureRgn, rgn);
	GetRegionBounds(rgn, &window_r);
	GetWindowRegion((WindowPtr)hd, kWindowContentRgn, rgn);
	GetRegionBounds(rgn, &content_r);
	DisposeRgn(rgn); 
	//put into qt structure
	top->fleft = content_r.left - window_r.left;
	top->ftop = content_r.top - window_r.top;
	top->fright = window_r.right - content_r.right;
	top->fbottom = window_r.bottom - window_r.bottom;
    }
}

void qt_macdnd_unregister( QWidget *widget, QWExtra *extra ); //dnd_mac
void qt_macdnd_register( QWidget *widget, QWExtra *extra ); //dnd_mac

void QWidget::setAcceptDrops( bool on )
{
    if ( (on && macDropEnabled) || (!on && !macDropEnabled) )
	return;

    if ( on ) {
	topLevelWidget()->createExtra();
	QWExtra *extra = topLevelWidget()->extraData();
	qt_macdnd_register( topLevelWidget(), extra );
	macDropEnabled = true;
    } else {
	macDropEnabled = false;
	QWExtra *extra = topLevelWidget()->extraData();
	qt_macdnd_unregister( topLevelWidget(), extra );
    }
}

void QWidget::setMask( const QRegion &region )
{
    createExtra();
    if ( region.isNull() && extra->mask.isNull() )
	return;

    QRegion clp;
    if(isVisible())
	clp = clippedRegion(FALSE);
    extra->mask = region;
    if(isVisible()) {
	dirtyClippedRegion(TRUE);
	clp ^= clippedRegion(FALSE);
	dirty_wndw_rgn("setMask",this, clp);
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

void QWidget::setName( const char * )
{
}


void QWidget::propagateUpdates()
{
    QMacSavedPortInfo savedInfo(this);
    RgnHandle r = NewRgn();
    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, r);
    if(!EmptyRgn(r)) {
	QRegion rgn(r);
	rgn.translate(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
	debug_wndw_rgn("*****propagatUpdates", topLevelWidget(), rgn);
	BeginUpdate((WindowPtr)hd);
	paint_children( this, rgn );
	EndUpdate((WindowPtr)hd);
    }
    DisposeRgn(r);
}

void QWidget::dirtyClippedRegion(bool dirty_myself)
{
    if(dirty_myself) {
	//dirty myself
	if(extra)
	    extra->child_dirty = extra->clip_dirty = TRUE;
	//when I get dirty so do my children
	if(QObjectList *chldn = queryList()) {
	    QObjectListIt it(*chldn);
	    for(QObject *obj; (obj = it.current()); ++it ) {
		if(obj->isWidgetType()) {
		    QWidget *w = (QWidget *)(*it);
		    if(w->topLevelWidget() == topLevelWidget() &&
		       !w->isTopLevel() && w->isVisible() && w->extra)
			w->extra->clip_dirty = TRUE;
		}
	    }
	}
    }

    //handle the rest of the widgets
    QPoint myp(posInWindow(this));
    QRect myr(myp.x(), myp.y(), width(), height());
    QWidget *last = this, *w;
    for(QWidget *widg = parentWidget(); widg; last = widg, widg = widg->parentWidget()) {
	QPoint widgp(posInWindow(widg));
	myr = myr.intersect(QRect(widgp.x(), widgp.y(), widg->width(), widg->height()));
	if(widg->extra)
	    widg->extra->child_dirty = TRUE;

	if(const QObjectList *chldn = widg->children()) {
	    for(QObjectListIt it(*chldn); it.current() && it.current() != last; ++it) {
		if((*it)->isWidgetType()) {
		    w = (QWidget *)(*it);
		    if(w->topLevelWidget() == topLevelWidget() && !w->isTopLevel() && w->isVisible()) {
			QPoint wp(posInWindow(w));
			if(myr.intersects(QRect(wp.x(), wp.y(), w->width(), w->height()))) {
			    if(w->extra)
				w->extra->clip_dirty = TRUE;
			    if(QObjectList *chldn2 = w->queryList()) {
				QObjectListIt it2(*chldn2);
				for(QObject *obj; (obj = it2.current()); ++it2 ) {
				    if(obj->isWidgetType()) {
					QWidget *w = (QWidget *)(*it2);
					if(w->topLevelWidget() == topLevelWidget() &&
					   !w->isTopLevel() && w->isVisible() && w->extra)
					    w->extra->clip_dirty = TRUE;
				    }
				}
				delete chldn2;
			    }
			}
		    }
		}
	    }
	}
    }
}

bool QWidget::isClippedRegionDirty()
{
    if(!extra || extra->clip_dirty)
	return TRUE;
    if(/*!isTopLevel() && */(parentWidget() && parentWidget()->isClippedRegionDirty()))
	return TRUE;
    return FALSE;
}

QRegion QWidget::clippedRegion(bool do_children)
{
    if(!isVisible() ||  (qApp->closingDown() || qApp->startingUp()))
	return QRegion();

    createExtra();

    if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	if(!do_children)
	    return extra->clip_sibs;
	return extra->clip_saved;
    }
    
    QRegion mask;
    //clip out my children
    if(do_children && extra->child_dirty) {
	extra->child_dirty = FALSE;
	extra->clip_children = QRegion(0, 0, width(), height());
	if(const QObjectList *chldnlst=children()) {
	    for(QObjectListIt it(*chldnlst); it.current(); ++it) {
		if((*it)->isWidgetType()) {
		    QWidget *cw = (QWidget *)(*it);
		    if( cw->isVisible() && cw->topLevelWidget() == topLevelWidget() ) {
			QRegion childrgn(cw->x(), cw->y(), cw->width(), cw->height());
			if(cw->extra && !cw->extra->mask.isNull()) {
			    mask = cw->extra->mask;
			    mask.translate(cw->x(), cw->y());
			    childrgn &= mask;
			}
			extra->clip_children -= childrgn;
		    }
		}
	    }
	}
    }

    if(isClippedRegionDirty()) {
	extra->clip_dirty = FALSE;
	QPoint tmp = posInWindow(this);
	extra->clip_sibs = QRegion(tmp.x(), tmp.y(), width(), height());
	//clip my rect with my mask
	if(extra && !extra->mask.isNull()) {
	    mask = extra->mask;
	    mask.translate(tmp.x(), tmp.y());
	    extra->clip_sibs &= mask;
	}

	//clip away my siblings
	if(!isTopLevel() && parentWidget()) {
	    if(const QObjectList *siblst = parentWidget()->children()) {
		//loop to this because its in zorder, and i don't care about people behind me
		QObjectListIt it(*siblst);
		for(it.toLast(); it.current() && it.current() != this; --it) {
		    if((*it)->isWidgetType()) {
			QWidget *sw = (QWidget *)(*it);
			tmp = posInWindow(sw);
			QRect sr(tmp.x(), tmp.y(), sw->width(), sw->height());
			if(sw->topLevelWidget() == topLevelWidget() &&
			   sw->isVisible() && extra->clip_sibs.contains(sr)) {
			    QRegion sibrgn(sr);
			    if(sw->extra && !sw->extra->mask.isNull()) {
				mask = sw->extra->mask;
				mask.translate(tmp.x(), tmp.y());
				sibrgn &= mask;
			    }
			    extra->clip_sibs -= sibrgn;
			}
		    }
		}
	    }
	}

	if(isTopLevel()) {
	    QRegion contents;
	    RgnHandle r = NewRgn();
	    GetWindowRegion((WindowPtr)hd, kWindowContentRgn, r);
	    if(!EmptyRgn(r)) {
		contents = QRegion(r);
		contents.translate(-geometry().x(), -geometry().y());
	    }
	    DisposeRgn(r);
	    extra->clip_sibs &= contents;
	}
	else if(parentWidget()) {
	    extra->clip_sibs &= parentWidget()->clippedRegion(FALSE);
	} 
    }

    //translate my stuff and my children now
    QRegion chldrgns = extra->clip_children;
    QPoint mp = posInWindow(this);
    chldrgns.translate(mp.x(), mp.y());
    extra->clip_saved = extra->clip_sibs & chldrgns;

    if(do_children)
	return extra->clip_saved;
    return extra->clip_sibs;
}


void QWidget::resetInputContext()
{

}
