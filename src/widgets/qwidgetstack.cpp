/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwidgetstack.cpp#4 $
**
** Implementation of QWidgetStack class
**
** Created : 980128
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwidgetstack.h"

#include "qobjcoll.h"
#include "qlayout.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qwidgetstack.cpp#4 $");


class QWidgetStackPrivate {
};


/*! \class QWidgetStack qwidgetstack.h

  \brief The QWidgetStack class provides a stack of widgets, where the
  user can see only the top widget.

  \ingroup misc

  This is often used e.g. in tab and wizard dialogs.

  The application programmer can move any widget to the top of the
  stack at any time using the slot raiseWidget(), and add or remove
  widgets using addWidget() and removeWidget().

  The default widget stack is frame-less, but you can use the usual
  QFrame functions (like setFrameStyle()) to add a frame.

  \sa QTabDialog QTabBar QFrame
*/


/*!  Constructs an empty widget stack. */

QWidgetStack::QWidgetStack( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    d = 0;
    dict = new QIntDict<QWidget>;
    l = 0;
    topWidget = 0;
}


/*! Destroys the object and frees any allocated resources. */

QWidgetStack::~QWidgetStack()
{
    delete d;
    delete dict;
}


/*!  Adds \a w to this stack of widgets, with id \a id.

  If \a w is not a child of \c this, QWidgetStack moves it using
  recreate().
*/

void QWidgetStack::addWidget( QWidget * w, int id )
{
    dict->insert( id, w );
    if ( w->parent() != this )
	w->recreate( this, 0, QPoint(0,0), FALSE );
}


/*!  Removes \a w from this stack of widgets.  Does not delete \a w. */

void QWidgetStack::removeWidget( QWidget * w )
{
    dict->take( id( w ) );
}


/*!  Raises \a id to the top of the widget stack. */

void QWidgetStack::raiseWidget( int id )
{
    raiseWidget( dict->find( id ) );
}


/*!  Raises \a w to the top of the widget stack. */

void QWidgetStack::raiseWidget( QWidget * w )
{
    if ( !w || !isMyChild( w ) )
	return;

    topWidget = w;
    if ( !isVisible() )
	return;

    w->show();

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.
    QWidget * f = w->focusWidget();
    while ( f && f->parent() != this )
	f = f->parentWidget();
    if ( f && f->parent() == this ) {
	if ( w->focusPolicy() != QWidget::NoFocus ) {
	    w->setFocus();
	} else {
	    bool done = FALSE;
	    const QObjectList * c = w->children();
	    QObjectListIt it( *c );
	    QObject * wc;
	    while( !done && (wc=it.current()) != 0 ) {
		++it;
		if ( wc->isWidgetType() ) {
		    f = (QWidget *)wc;
		    if ( f->focusPolicy() == QWidget::StrongFocus ||
			 f->focusPolicy() == QWidget::TabFocus ) {
			f->setFocus();
			done = TRUE;
		    }
		}
	    }
	}
    }

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() && o != w )
	    ((QWidget *)o)->hide();
    }
}


/*!  Returns TRUE if \a w is a child of this widget, else FALSE. */

bool QWidgetStack::isMyChild( QWidget * w )
{
    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() && o == w )
	    return TRUE;
    }
    return FALSE;
}


/*! Reimpelemented in order to set the children's geometries
  appropriately. */

void QWidgetStack::frameChanged()
{
    QFrame::frameChanged();
    setChildGeometries();
}


/*!  Fix up the children's geometries. */

void QWidgetStack::setChildGeometries()
{
    delete l;
    l = new QGridLayout( this, 3, 3 );
    if ( frameWidth() ) {
	l->addRowSpacing( 0, frameWidth() );
	l->addRowSpacing( 2, frameWidth() );
	l->addColSpacing( 0, frameWidth() );
	l->addColSpacing( 2, frameWidth() );
    }
    l->setRowStretch( 1, 1 );
    l->setColStretch( 1, 1 );

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() )
	    l->addWidget( (QWidget *)o, 1, 1 );
    }
    l->activate();
}


/*!  Reimplemented in order to set the children's geometries
  appropriately. */

void QWidgetStack::show()
{
    if ( !isVisible() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && o != topWidget )
		((QWidget *)o)->hide();
	}
    }

    setChildGeometries();
    QFrame::show();
}


/*!  Returns a pointer to the widget with ID \a id.  If this widget
  stack does not manage a widget with ID \a id, this function return
  0.

  \sa id() addWidget()
*/

QWidget * QWidgetStack::widget( int id ) const
{
    return dict->find( id );
}


/*!  Returns the ID of the \a widget.  If \a widget is 0 or is not
  being managed by this widget stack, this function returns 0.

  \sa widget() addWidget()
*/

int QWidgetStack::id( QWidget * widget ) const
{
    if ( !widget || !dict )
	return 0;

    QIntDictIterator<QWidget> it( *dict );
    while ( it.current() && it.current() != widget )
	++it;
    return it.current() == widget ? it.currentKey() : 0;
}
