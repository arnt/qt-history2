/****************************************************************************
** $Id: $
**
** Implementation of QWidgetStack class
**
** Created : 980128
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qwidgetstack.h"
#ifndef QT_NO_WIDGETSTACK

#include "qobjectlist.h"
#include "qbutton.h"
#include "qbuttongroup.h"

#include "qapplication.h"

class QWidgetStackPrivate {
public:
    class Invisible: public QWidget
    {
    public:
	Invisible( QWidgetStack * parent ): QWidget( parent, "qt_invisible_widgetstack" )
	{
	    setBackgroundMode( NoBackground );
	}
	const char * className() const
	{
	    return "QWidgetStackPrivate::Invisible";
	}
    };
};

/*! \class QWidgetStack qwidgetstack.h

  \brief The QWidgetStack class provides a stack of widgets of which only the
  top widget is user-visible.

  \ingroup organizers
  \mainclass

  The application programmer can move any widget to the top of the
  stack at any time using raiseWidget(), and add or remove widgets
  using addWidget() and removeWidget().

  visibleWidget() is the \e get equivalent of raiseWidget(); it
  returns a pointer to the widget that is currently at the top of the
  stack.

  QWidgetStack also provides the ability to manipulate widgets through
  application-specified integer ids. You can also translate from
  widget pointers to ids using id() and from ids to widget pointers
  using widget().  These numeric ids are unique (per QWidgetStack,
  not globally), but QWidgetStack does not attach any
  additional meaning to them.

  The default widget stack is frameless, but you can use the usual
  QFrame functions (such as setFrameStyle()) to add a frame.

  QWidgetStack provides a signal, aboutToShow(), which is
  emitted just before a managed widget is shown.

  \sa QTabDialog QTabBar QFrame
*/


/*!  Constructs an empty widget stack with parent \a parent and called
    \a name.
*/

QWidgetStack::QWidgetStack( QWidget * parent, const char *name )
    : QFrame( parent, name )
{
    d = 0;
    dict = new QIntDict<QWidget>;
    focusWidgets = 0;
    topWidget = 0;
    invisible = new QWidgetStackPrivate::Invisible( this );
}


/*! Destroys the object and frees any allocated resources. */

QWidgetStack::~QWidgetStack()
{
    delete focusWidgets;
    focusWidgets = 0;
    delete d;
    d = 0;
    delete dict;
    dict = 0;
}


/*!  Adds widget \a w to this stack of widgets, with id \a id.

  If you pass an id >= 0 this id is used. If you pass an \a id of -1
  (the default), the widgets will be numbered automatically starting
  at 0. If you pass any other negative integer a unique negative
  integer (<= -2) will be generated. No widget has an id of -1.

  The aboutToShow(int) signal is only emitted for widgets with id >=
  0. The aboutToShow(QWidget *) signal is emitted for any widget.

  If \a w is not a child of \c this, QWidgetStack moves it using
  reparent().

  \sa aboutToShow()
*/

int QWidgetStack::addWidget( QWidget * w, int id )
{
    static int seq_no = -2;

    if ( !w || w == invisible )
	return -1;

    if ( id < -1 )
	id = seq_no--;
    else if ( id == -1 )
	id = dict->count();

    dict->insert( id+1, w );

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

    return id;
}


/*!  Removes widget \a w from this stack of widgets.  Does not delete \a
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
	dict->take( i+1 );
    if ( w == topWidget )
	topWidget = 0;
    if ( dict->isEmpty() )
	invisible->hide(); // let background shine through again
}


/*!  Raises the widget with id, \a id, to the top of the widget stack.

    \sa visibleWidget()
*/

void QWidgetStack::raiseWidget( int id )
{
    if ( id == -1 )
	return;
    QWidget * w = dict->find( id+1 );
    if ( w )
	raiseWidget( w );
}


/*!
    \overload
    Raises widget \a w to the top of the widget stack.
*/

void QWidgetStack::raiseWidget( QWidget * w )
{
    if ( !w || !isMyChild( w ) )
	return;

    topWidget = w;
    if ( !isVisible() )
	return;

    if ( !invisible->isVisible() ) {
	invisible->setGeometry( contentsRect() );
	invisible->lower();
	invisible->show();
	QApplication::sendPostedEvents( invisible, QEvent::ShowWindowRequest );
    }

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.
    QWidget * f = w->focusWidget();
    while ( f && f->parent() != this )
	f = f->parentWidget();
    if ( f && f->parent() == this ) {
	if ( !focusWidgets )
	    focusWidgets = new QPtrDict<QWidget>( 17 );
	focusWidgets->replace( f, f->focusWidget() );
	f->focusWidget()->clearFocus();
	if ( w->focusPolicy() != QWidget::NoFocus ) {
	    f = w;
	} else {
	    // look for the best focus widget we can find
	    // best == what we had (which may be deleted)
	    f = focusWidgets->find( w );
	    if ( f )
		focusWidgets->take( w );
	    // second best == selected button from button group
	    QWidget * fb = 0;
	    // third best == whatever candidate we see first
	    QWidget * fc = 0;
	    bool done = FALSE;
	    const QObjectList * c = w->children();
	    if ( c ) {
		QObjectListIt it( *c );
		QObject * wc;
		while( !done && (wc=it.current()) != 0 ) {
		    ++it;
		    if ( wc->isWidgetType() ) {
			if ( f == wc ) {
			    done = TRUE;
			} else if ( (((QWidget *)wc)->focusPolicy()&QWidget::TabFocus)
				    == QWidget::TabFocus ) {
#ifndef QT_NO_BUTTONGROUP
			    QButton * b = (QButton *)wc;
			    if ( wc->inherits( "QButton" ) &&
				 b->group() && b->isOn() &&
				 b->group()->isExclusive() &&
				 ( fc == 0 ||
				   !fc->inherits( "QButton" ) ||
				   ((QButton*)fc)->group() == b->group() ) )
				fb = b;
			    else
#endif
			    if ( !fc )
				fc = (QWidget*)wc;
			}
		    }
		}
		// f exists iff done
		if ( !done ) {
		    if ( fb )
			f = fb;
		    else if ( fc )
			f = fc;
		    else
			f = 0;
		}
	    }
	}
    }

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() && o != w && o != invisible )
	    ((QWidget *)o)->hide();
    }
    if ( f )
	f->setFocus();

    if ( isVisible() ) {
	emit aboutToShow( w );
	int i = id( w );
	if ( i != -1 )
	    emit aboutToShow( i );
    }

    w->setGeometry( invisible->geometry() );
    w->show();
    //QApplication::sendPostedEvents( w, QEvent::ShowWindowRequest );
}


/*!  Returns TRUE if \a w is a child of this widget; otherwise returns FALSE. */

bool QWidgetStack::isMyChild( QWidget * w )
{
    const QObjectList * c = children();
    if ( !c || !w || w == invisible )
	return FALSE;
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() && o == w )
	    return TRUE;
    }
    return FALSE;
}


/*! \reimp */

void QWidgetStack::frameChanged()
{
    QFrame::frameChanged();
    setChildGeometries();
}


/*! \reimp */

void QWidgetStack::setFrameRect( const QRect & r )
{
    QFrame::setFrameRect( r );
    setChildGeometries();
}


/*!  Fixes up the children's geometries. */

void QWidgetStack::setChildGeometries()
{
    invisible->setGeometry( contentsRect() );
    if ( topWidget )
	topWidget->setGeometry( invisible->geometry() );
}


/*! \reimp */
void QWidgetStack::show()
{
    //  Reimplemented in order to set the children's geometries
    //  appropriately.
    if ( !isVisible() && children() ) {
	setChildGeometries();

	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() )
		if ( o == topWidget )
		    ((QWidget *)o)->show();
		else if ( o == invisible && topWidget != 0 )
		    ((QWidget *)o)->show();
		else
		    ((QWidget *)o)->hide();
	}
    }

    QFrame::show();
}


/*!  Returns a pointer to the widget with id \a id.  If this widget
  stack does not manage a widget with id \a id, this function returns
  0.

  \sa id() addWidget()
*/

QWidget * QWidgetStack::widget( int id ) const
{
    return id != -1 ? dict->find( id+1 ) : 0;
}


/*!  Returns the id of the \a widget.  If \a widget is 0 or is not
  being managed by this widget stack, this function returns -1.

  \sa widget() addWidget()
*/

int QWidgetStack::id( QWidget * widget ) const
{
    if ( !widget || !dict )
	return -1;

    QIntDictIterator<QWidget> it( *dict );
    while ( it.current() && it.current() != widget )
	++it;
    return it.current() == widget ? it.currentKey()-1 : -1;
}


/*! Returns a pointer to the currently visible widget (the one at the
  top of the stack), or 0 if nothing is currently being shown.

  \sa aboutToShow() id() raiseWidget()
*/

QWidget * QWidgetStack::visibleWidget() const
{
    return topWidget;
}


/*! \fn void QWidgetStack::aboutToShow( int )

  This signal is emitted just before a managed widget is shown if
  that managed widget has an id != -1. The argument is the numeric
  id of the widget.
*/


/*! \overload void QWidgetStack::aboutToShow( QWidget * )

  This signal is emitted just before a managed widget is shown.  The
  argument is a pointer to the widget.
*/


/*! \reimp */

void QWidgetStack::resizeEvent( QResizeEvent * e )
{
    QFrame::resizeEvent( e );
    setChildGeometries();
}


/*! \reimp */

QSize QWidgetStack::sizeHint() const
{
    constPolish();

    QSize size(0,0);
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && o != invisible ) {
		QWidget *w = (QWidget*)o;
		size = size.expandedTo( w->sizeHint() )
		       .expandedTo(w->minimumSize());
	    }
	}
    }
    if ( size.isNull() )
	return QSize(100,50); //### is this a sensible default???
    return QSize( size.width() + 2*frameWidth(), size.height() + 2*frameWidth() );
}


/*! \reimp */
QSize QWidgetStack::minimumSizeHint() const
{
    constPolish();

    QSize size(0,0);
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() &&  o != invisible ) {
		    QWidget *w = (QWidget*)o;
		    size = size.expandedTo( w->minimumSizeHint())
					    .expandedTo(w->minimumSize());
	    }
	}
    }
    return QSize( size.width() + 2*frameWidth(), size.height() + 2*frameWidth() );
}

/*! \reimp  */
void QWidgetStack::childEvent( QChildEvent * e)
{
    if ( e->child()->isWidgetType() && e->removed() )
	removeWidget( (QWidget*) e->child() );
}
#endif
