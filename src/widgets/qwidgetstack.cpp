/****************************************************************************
**
** Implementation of QWidgetStack class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwidgetstack.h"
#include "qlayout.h"
#include "../kernel/qlayoutengine_p.h"
#ifndef QT_NO_WIDGETSTACK

#include "qfocusdata.h"
#include "qbutton.h"
#include "qbuttongroup.h"

#include "qapplication.h"

class QWidgetStackPrivate {
public:
    class Invisible: public QWidget
    {
    public:
	Invisible( QWidgetStack * parent )
	    : QWidget( parent, "qt_invisible_widgetstack" )
	{ setBackgroundMode( NoBackground ); }
	const char * className() const
	{ return "QWidgetStackPrivate::Invisible"; }
    };
};


/*!
    \class QWidgetStack
    \brief The QWidgetStack class provides a stack of widgets of which
    only the top widget is user-visible.

    \ingroup organizers
    \mainclass

    The application programmer can move any widget to the top of the
    stack at any time using raiseWidget(), and add or remove widgets
    using addWidget() and removeWidget(). It is not sufficient to pass
    the widget stack as parent to a widget which should be inserted into
    the widgetstack.

    visibleWidget() is the \e get equivalent of raiseWidget(); it
    returns a pointer to the widget that is currently at the top of
    the stack.

    QWidgetStack also provides the ability to manipulate widgets
    through application-specified integer IDs. You can also translate
    from widget pointers to IDs using id() and from IDs to widget
    pointers using widget(). These numeric IDs are unique (per
    QWidgetStack, not globally), but QWidgetStack does not attach any
    additional meaning to them.

    The default widget stack is frameless, but you can use the usual
    QFrame functions (such as setFrameStyle()) to add a frame.

    QWidgetStack provides a signal, aboutToShow(), which is emitted
    just before a managed widget is shown.

    \sa QTabDialog QTabBar QFrame
*/


/*!
    Constructs an empty widget stack.

    The \a parent and \a name arguments are passed to the QFrame
    constructor.
*/

QWidgetStack::QWidgetStack( QWidget * parent, const char *name )
    : QFrame( parent, name )
{
    init();
}

/*!
  Constructs an empty widget stack.

  The \a parent, \a name and \a f arguments are passed to the QFrame
  constructor.
*/
QWidgetStack::QWidgetStack( QWidget * parent, const char *name, WFlags f )
    : QFrame( parent, name, f ) //## merge constructors in 4.0
{
    init();
}

void QWidgetStack::init()
{
   d = 0;
   dict = new QIntDict<QWidget>;
   focusWidgets = 0;
   topWidget = 0;
   invisible = new QWidgetStackPrivate::Invisible( this );
   invisible->hide();
}


/*!
    Destroys the object and frees any allocated resources.
*/

QWidgetStack::~QWidgetStack()
{
    delete focusWidgets;
    delete d;
    delete dict;
}


/*!
    Adds widget \a w to this stack of widgets, with ID \a id.

    If you pass an id \>= 0 this ID is used. If you pass an \a id of
    -1 (the default), the widgets will be numbered automatically. If
    you pass -2 a unique negative integer will be generated. No widget
    has an ID of -1. Returns the ID or -1 on failure (e.g. \w is 0).

    If you pass an id that is already used, then a unique negative
    integer will be generated to prevent two widgets having the same
    id.

    If \a w is not a child of this QWidgetStack moves it using
    reparent().
*/

int QWidgetStack::addWidget( QWidget * w, int id )
{
    static int nseq_no = -2;
    static int pseq_no = 0;

    if ( !w || w == invisible )
	return -1;

    // prevent duplicates
    removeWidget( w );

    if ( id >= 0 && dict->find( id ) )
	id = -2;
    if ( id < -1 )
	id = nseq_no--;
    else if ( id == -1 )
	id = pseq_no++;
    else
	pseq_no = QMAX(pseq_no, id + 1);
	// use id >= 0 as-is

    dict->insert( id, w );

    // preserve existing focus
    QWidget * f = w->focusWidget();
    while( f && f != w )
	f = f->parentWidget();
    if ( f ) {
	if ( !focusWidgets )
	    focusWidgets = new QPtrDict<QWidget>( 17 );
	focusWidgets->replace( w, w->focusWidget() );
    }

    w->hide();
    if ( w->parent() != this )
	w->reparent( this, 0, contentsRect().topLeft(), FALSE );
    w->setGeometry( contentsRect() );
    updateGeometry();
    return id;
}


/*!
    Removes widget \a w from this stack of widgets. Does not delete \a
    w. If \a w is the currently visible widget, no other widget is
    substituted.

    \sa visibleWidget() raiseWidget()
*/

void QWidgetStack::removeWidget( QWidget * w )
{
    if ( !w )
	return;
    int i = id( w );
    if ( i != -1 )
	dict->take( i );

    if ( w == topWidget )
	topWidget = 0;
    if ( dict->isEmpty() )
	invisible->hide(); // let background shine through again
    updateGeometry();
}


/*!
    Raises the widget with ID \a id to the top of the widget stack.

    \sa visibleWidget()
*/

void QWidgetStack::raiseWidget( int id )
{
    if ( id == -1 )
	return;
    QWidget * w = dict->find( id );
    if ( w )
	raiseWidget( w );
}

static bool isChildOf( QWidget* child, QWidget *parent )
{
    if ( !child )
	return FALSE;
    QObjectList list = parent->children();
    for (int i = 0; i < list.size(); ++i) {
	QObject *obj = list.at(i);
	if (!obj->isWidgetType())
	    continue;
	QWidget *widget = static_cast<QWidget *>(obj);
	if (!widget->isTopLevel())
	    continue;
	if ( widget == child || isChildOf( child, widget ) )
	    return TRUE;
    }
    return FALSE;
}

/*!
    \overload

    Raises widget \a w to the top of the widget stack.
*/

void QWidgetStack::raiseWidget( QWidget *w )
{
    if ( !w || w == invisible || w->parent() != this || w == topWidget )
	return;

    if ( id(w) == -1 )
	addWidget( w );
    if ( !isVisible() ) {
	topWidget = w;
	return;
    }

    if ( invisible->isHidden() ) {
	invisible->setGeometry( contentsRect() );
	invisible->lower();
	invisible->show();
	QApplication::sendPostedEvents( invisible, QEvent::ShowWindowRequest );
    }

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.
    if ( topWidget ) {
	QWidget * fw = focusWidget();
	QWidget* p = fw;
	while ( p && p != topWidget )
	    p = p->parentWidget();
	if ( p == topWidget ) { // focus was on old page
	    if ( !focusWidgets )
		focusWidgets = new QPtrDict<QWidget>( 17 );
	    focusWidgets->replace( topWidget, fw );
	    fw->clearFocus();
	    // look for the best focus widget we can find
	    // best == what we had (which may be deleted)
	    fw = focusWidgets->take( w );
	    if ( isChildOf( fw, w ) ) {
		fw->setFocus();
	    } else {
		// second best == first child widget in the focus chain
		QFocusData *f = focusData();
		QWidget* home = f->home();
		QWidget *i = home;
		do {
		    if ( ( ( i->focusPolicy() & TabFocus ) == TabFocus )
			 && !i->focusProxy() && i->isVisibleTo(w) && i->isEnabled() ) {
			p = i;
			while ( p && p != w )
			    p = p->parentWidget();
			if ( p == w ) {
			    i->setFocus();
			    break;
			}
		    }
		    i = f->next();
		} while( i != home );
	    }
	}
    }

    if ( isVisible() ) {
	emit aboutToShow( w );
	int i = id( w );
	if ( i != -1 )
	    emit aboutToShow( i );
    }

    topWidget = w;

    QObjectList c = children();
    for (int i = 0; i < c.size(); ++i) {
	QObject * o = c.at(i);
	if ( o->isWidgetType() && o != w && o != invisible )
	    static_cast<QWidget *>(o)->hide();
    }

    w->setGeometry( invisible->geometry() );
    w->show();
}

/*!
    \reimp
*/

void QWidgetStack::frameChanged()
{
    QFrame::frameChanged();
    setChildGeometries();
}


/*!
    \reimp
*/

void QWidgetStack::setFrameRect( const QRect & r )
{
    QFrame::setFrameRect( r );
    setChildGeometries();
}


/*!
    Fixes up the children's geometries.
*/

void QWidgetStack::setChildGeometries()
{
    invisible->setGeometry( contentsRect() );
    if ( topWidget )
	topWidget->setGeometry( invisible->geometry() );
}


/*!
    \reimp
*/
void QWidgetStack::show()
{
    //  Reimplemented in order to set the children's geometries
    //  appropriately and to pick the first widget as topWidget if no
    //  topwidget was defined
    QObjectList c = children();
    if ( !isVisible() && !c.isEmpty() ) {
	for (int i = 0; i < c.size(); ++i) {
	    QObject * o = c.at(i);
	    if ( o->isWidgetType() ) {
		if ( !topWidget && o != invisible )
		    topWidget = static_cast<QWidget*>(o);
		if ( o == topWidget )
		    static_cast<QWidget *>(o)->show();
		else
		    static_cast<QWidget *>(o)->hide();
	    }
	}
	setChildGeometries();
    }
    QFrame::show();
}


/*!
    Returns the widget with ID \a id. Returns 0 if this widget stack
    does not manage a widget with ID \a id.

    \sa id() addWidget()
*/

QWidget * QWidgetStack::widget( int id ) const
{
    return id != -1 ? dict->find( id ) : 0;
}


/*!
    Returns the ID of the \a widget. Returns -1 if \a widget is 0 or
    is not being managed by this widget stack.

    \sa widget() addWidget()
*/

int QWidgetStack::id( QWidget * widget ) const
{
    if ( !widget )
	return -1;

    QIntDictIterator<QWidget> it( *dict );
    while ( it.current() && it.current() != widget )
	++it;
    return it.current() == widget ? it.currentKey() : -1;
}


/*!
    Returns the currently visible widget (the one at the top of the
    stack), or 0 if nothing is currently being shown.

    \sa aboutToShow() id() raiseWidget()
*/

QWidget * QWidgetStack::visibleWidget() const
{
    return topWidget;
}


/*!
    \fn void QWidgetStack::aboutToShow( int )

    This signal is emitted just before a managed widget is shown if
    that managed widget has an ID != -1. The argument is the numeric
    ID of the widget.

    If you call visibleWidget() in a slot connected to aboutToShow(),
    the widget it returns is the one that is currently visible, not
    the one that is about to be shown.
*/


/*!
    \fn void QWidgetStack::aboutToShow( QWidget * )

    \overload

    This signal is emitted just before a managed widget is shown. The
    argument is a pointer to the widget.

    If you call visibleWidget() in a slot connected to aboutToShow(),
    the widget returned is the one that is currently visible, not the
    one that is about to be shown.
*/


/*!
    \reimp
*/

void QWidgetStack::resizeEvent( QResizeEvent * e )
{
    QFrame::resizeEvent( e );
    setChildGeometries();
}


/*!
    \reimp
*/

QSize QWidgetStack::sizeHint() const
{
    constPolish();

    QSize size( 0, 0 );

    QIntDictIterator<QWidget> it( *dict );
    QWidget *w;

    while ( (w = it.current()) != 0 ) {
	++it;
	QSize sh = w->sizeHint();
	if ( w->sizePolicy().horData() == QSizePolicy::Ignored )
	    sh.rwidth() = 0;
	if ( w->sizePolicy().verData() == QSizePolicy::Ignored )
	    sh.rheight() = 0;
#ifndef QT_NO_LAYOUT
	size = size.expandedTo( sh ).expandedTo( qSmartMinSize(w) );
#endif
    }
    if ( size.isNull() )
	size = QSize( 128, 64 );
    size += QSize( 2*frameWidth(), 2*frameWidth() );
    return size;
}


/*!
    \reimp
*/
QSize QWidgetStack::minimumSizeHint() const
{
    constPolish();

    QSize size( 0, 0 );

    QIntDictIterator<QWidget> it( *dict );
    QWidget *w;

#ifndef QT_NO_LAYOUT
    while ( (w = it.current()) != 0 ) {
	++it;
	size = size.expandedTo( qSmartMinSize(w) );
    }
#endif
    if ( size.isNull() )
	size = QSize( 64, 32 );
    size += QSize( 2*frameWidth(), 2*frameWidth() );
    return size;
}

/*!
    \reimp
*/
void QWidgetStack::childEvent( QChildEvent * e)
{
    if ( e->child()->isWidgetType() && e->removed() )
	removeWidget( (QWidget*) e->child() );
}

/*! \reimp
 */
bool QWidgetStack::event( QEvent* e )
{
    if ( e->type() == QEvent::LayoutHint )
	updateGeometry(); // propgate layout hints to parent
    return QFrame::event( e );
}
#endif
