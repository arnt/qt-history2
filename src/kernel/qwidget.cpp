/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.cpp#15 $
**
** Implementation of QWidget class
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** IMPORTANT NOTE: Widget identifier should only be set with the set_id()
** function, otherwise widget mapping will not work.
*****************************************************************************/

#define	 NO_WARNINGS
#include "qwidget.h"
#include "qobjcoll.h"
#include "qapp.h"
#include "qcolor.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget.cpp#15 $";
#endif


// --------------------------------------------------------------------------
// Internal QWidgetMapper class
//
// The purpose of this class is to map widget identifiers to QWidget objects.
// All QWidget objects register themselves in the QWidgetMapper when they
// get an identifier. Widgets unregister themselves when they change ident-
// ifier or when they are destroyed. A widget identifier is really a window
// handle.
//
// The widget mapper is created and destroyed by the main application routines
// in the file qapp_xxx.C.
//

#include "qintdict.h"

declare(QIntDictM,QWidget);
declare(QIntDictIteratorM,QWidget);

static const WDictSize = 101;

class QWidgetMapper : public QIntDictM(QWidget)
{						// maps ids -> widgets
public:
    QWidgetMapper();
   ~QWidgetMapper();
    QWidget *find( WId id );			// find widget
    bool     insert( const QWidget * );		// insert widget
    bool     remove( WId id );			// remove widget
private:
    WId	     cur_id;
    QWidget *cur_widget;
};

QWidgetMapper *QWidget::mapper = 0;		// app global widget mapper

QWidget *QWidget::activeWidget = 0;		// widget in focus


QWidgetMapper::QWidgetMapper() : QIntDictM(QWidget)(WDictSize)
{
    cur_id = 0;
    cur_widget = 0;
}

QWidgetMapper::~QWidgetMapper()
{
    clear();
}

inline QWidget *QWidgetMapper::find( WId id )
{
    if ( id != cur_id ) {			// need to lookup
	cur_widget = QIntDictM(QWidget)::find((long)id);
	if ( cur_widget )
	    cur_id = id;
	else
	    cur_id = 0;
    }
    return cur_widget;
}

inline bool QWidgetMapper::insert( const QWidget *widget )
{
    return QIntDictM(QWidget)::insert((long)widget->id(),widget);
}

inline bool QWidgetMapper::remove( WId id )
{
    if ( cur_id == id ) {			// reset current widget
	cur_id = 0;
	cur_widget = 0;
    }
    return QIntDictM(QWidget)::remove((long)id);
}


// --------------------------------------------------------------------------
// QWidget member functions
//

QWidget::QWidget( QWidget *parent, const char *name, WFlags f )
	: QObject( parent, name )
{
    initMetaObject();				// initialize meta object
    isWidget = TRUE;				// is a widget
    ident = 0;					// default attributes
    flags = f;
    extra = 0;					// no extra widget info
    create();					// platform-dependent init
    if ( parent )
	setBackgroundColor( parent->backgroundColor() );
}

QWidget::~QWidget()
{
    if ( QApplication::main_widget == this )	// reset main widget
	QApplication::main_widget = 0;
    if ( childObjects ) {			// widget has children
	register QObject *obj = childObjects->first();
	while ( obj ) {				// delete all child objects
	    obj->parentObj = 0;			// object should not remove
	    delete obj;				//   itself from the list
	    obj = childObjects->next();
	}
	delete childObjects;
	childObjects = 0;
    }
    destroy();					// platform-dependent cleanup
    delete extra;
    if ( nWidgets() == 0 )			// no more widgets left
	QApplication::quit();
}


void QWidget::createMapper()			// create widget mapper
{
    mapper = new QWidgetMapper;
    CHECK_PTR( mapper );
}

void QWidget::destroyMapper()			// destroy widget mapper
{
    if ( !mapper )				// already gone
	return;
    register QWidget *w;
    QWidgetMapper *tmp = mapper;
    mapper = 0;					// controlled cleanup
    QIntDictIteratorM(QWidget) it( *((QIntDictM(QWidget)*)tmp) );
    w = it.current();
    while ( w ) {				// remove child widgets first
	if ( w->parentObj ) {			// widget has a parent
	    tmp->remove( w->id() );		//   then remove from dict
	    w = it.current();			// w will be next widget
	}
	else					// skip parentless widgets now
	    w = ++it;
    }
    w = it.toFirst();
    while ( w ) {				// delete parentless widgets
	delete w;
	w = ++it;
    }
    delete tmp;
}

void QWidget::set_id( WId id )			// set widget identifier
{
    if ( !mapper )				// mapper destroyed
	return;
    if ( ident )
	mapper->remove( ident );
    ident = id;
#if defined(_WS_X11_)
    hd = id;					// X11: hd == ident
#endif
    if ( id )
	mapper->insert( this );
}

void QWidget::createExtra()			// create extra data
{
    if ( !extra ) {				// if not exists
	extra = new QWExtra;
	CHECK_PTR( extra );
	extra->guistyle = QApplication::style();// initialize
	extra->minw = extra->minh = -1;
	extra->maxw = extra->maxh = -1;
	extra->incw = extra->inch = -1;
    }
}

QWidget *QWidget::find( WId id )		// find widget with id
{
    return mapper ? mapper->find( id ) : 0;
}

ulong QWidget::nWidgets()			// number of widgets
{
    return mapper ? mapper->count() : 0;
}


GUIStyle QWidget::style() const			// get widget GUI style
{
    return extra ? extra->guistyle : QApplication::style();
}

void QWidget::setStyle( GUIStyle gs )		// set widget GUI style
{
    createExtra();
    extra->guistyle = gs;
}


void QWidget::enable()				// enable events
{
    clearFlag( WState_Disabled );
}

void QWidget::disable()				// disable events
{
    setFlag( WState_Disabled );
}


#if !defined(_WS_X11_)
bool QWidget::setMouseTracking( bool enable )
{
    bool v = testFlag( WMouseTracking );
    if ( onOff )
	setFlag( WMouseTracking );
    else
	clearFlag( WMouseTracking );
    return v;
}
#endif // _WS_X11_


void QWidget::setRect( const QRect &r )		// set rect, update ncrect
{
    ncrect.setLeft( ncrect.left() + r.left() - rect.left() );
    ncrect.setTop( ncrect.top() + r.top() - rect.top() );
    ncrect.setRight( ncrect.right() + r.right() - rect.right() );
    ncrect.setBottom( ncrect.bottom() + r.bottom() - rect.bottom() );
    rect = r;
}

void QWidget::setNCRect( const QRect &r )	// set ncrect, update rect
{
    rect.setLeft( rect.left() + r.left() - ncrect.left() );
    rect.setTop( rect.top() + r.top() - ncrect.top() );
    rect.setRight( rect.right() + r.right() - ncrect.right() );
    rect.setBottom( rect.bottom() + r.bottom() - ncrect.bottom() );
    ncrect = r;
}


QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{						// map to global coordinates
    register QWidget *w = (QWidget*)this;
    QPoint p = pos;
    while ( w ) {
	p += w->rect.topLeft();
	w = w->parentWidget();
    }
    return p;
}

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{						// map from global coordinates
    register QWidget *w = (QWidget*)this;
    QPoint p = pos;
    while ( w ) {
	p -= w->rect.topLeft();
	w = w->parentWidget();
    }
    return p;
}

QPoint QWidget::mapToParent( const QPoint &p ) const
{						// map to parent coordinates
    return p + rect.topLeft();
}

QPoint QWidget::mapFromParent( const QPoint &p ) const
{						// map from parent coordinate
    return p - rect.topLeft();
}


bool QWidget::close( bool forceKill )		// close widget
{
    QCloseEvent event;
    QApplication::sendEvent( this, &event );
    if ( event.isAccepted() || forceKill )
	delete this;
    return event.isAccepted();
}


// --------------------------------------------------------------------------
// QWidget event handling
//

bool QWidget::event( QEvent *e )		// receive event
{
    if ( eventFilters ) {			// pass through event filters
	if ( activate_filters( e ) )		// eaten by some filter
	    return TRUE;
    }
    switch ( e->type() ) {

	case Event_Timer:
	    timerEvent( (QTimerEvent*)e );
	    break;

	case Event_MouseMove:
	    mouseMoveEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonPress:
#if 0	/* NOTE!!! Experimental */
	    if ( !testFlag(WState_FocusA) ) {
		if ( focusInEvent( e ) ) {
		    setFocus();
		}
	    }
#endif
	    mousePressEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonRelease:
	    mouseReleaseEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonDblClick:
	    mouseDoubleClickEvent( (QMouseEvent*)e );
	    break;

	case Event_KeyPress: {
	    QKeyEvent *k = (QKeyEvent*)e;
	    keyPressEvent( k );
#if defined(_WS_X11_)
	    if ( !k->isAccepted() && parentObj )// child didn't want it, then
		return parentObj->event( e );	//   pass event to parent
#endif
	    }
	    break;

	case Event_KeyRelease: {
	    QKeyEvent *k = (QKeyEvent*)e;
	    keyReleaseEvent( k );
#if defined(_WS_X11_)
	    if ( !k->isAccepted() && parentObj )// child didn't want it, then
		return parentObj->event( e );	//   pass event to parent
#endif
	    }
	    break;

	case Event_FocusIn:
#if 0	/* NOTE!!! Experimental */
	    setFlag( WState_FocusP );		// set focus pending flag
	    if ( res = focusInEvent(e) )
		setFocus();
	    clearFlag( WState_FocusP );
#endif
	    break;

	case Event_FocusOut:
#if 0	/* NOTE!!! Experimental */
	    if ( testFlag(WState_FocusA) ) {
		clearFlag( WState_FocusA );
		focusOutEvent( e );
	    }
#endif
	    break;

	case Event_Paint:
	    paintEvent( (QPaintEvent*)e );
	    break;

	case Event_Move:
	    moveEvent( (QMoveEvent*)e );
	    break;

	case Event_Resize:
	    resizeEvent( (QResizeEvent*)e );
	    break;

	case Event_Close: {
	    QCloseEvent *c = (QCloseEvent *)e;
	    closeEvent( c );
	    if ( !c->isAccepted() )
		return FALSE;
	    }
	    break;

	default:
	    return FALSE;
    }
    return TRUE;
}

void QWidget::timerEvent( QTimerEvent * )
{
}

void QWidget::mouseMoveEvent( QMouseEvent * )
{
}

void QWidget::mousePressEvent( QMouseEvent * )
{
}

void QWidget::mouseReleaseEvent( QMouseEvent * )
{
}

void QWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );			// try mouse press event
}

void QWidget::keyPressEvent( QKeyEvent *e )
{
    e->ignore();
}

void QWidget::keyReleaseEvent( QKeyEvent *e )
{
    e->ignore();
}

void QWidget::focusInEvent( QFocusEvent * )
{
}

void QWidget::focusOutEvent( QFocusEvent * )
{
}

void QWidget::paintEvent( QPaintEvent * )
{
}

void QWidget::moveEvent( QMoveEvent * )
{
}

void QWidget::resizeEvent( QResizeEvent * )
{
}

void QWidget::showEvent( QShowEvent *e )
{
}

void QWidget::hideEvent( QHideEvent *e )
{
}

void QWidget::closeEvent( QCloseEvent *e )
{
}


#if defined(_WS_MAC_)

bool QWidget::macEvent( MSG * )			// Macintosh event
{
    return FALSE;
}

#elif defined(_WS_WIN_)

bool QWidget::winEvent( MSG * )			// Windows (+NT) event
{
    return FALSE;
}

#elif defined(_WS_PM_)

bool QWidget::pmEvent( QMSG * )			// OS/2 PM event
{
    return FALSE;
}

#elif defined(_WS_X11_)

bool QWidget::x11Event( XEvent * )		// X11 event
{
    return FALSE;
}

#endif
