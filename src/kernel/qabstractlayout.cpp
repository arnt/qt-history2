/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qabstractlayout.cpp#2 $
**
** Implementation of the abstract layout base class
**
** Created : 960416
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#include "qabstractlayout.h"
#include "qwidget.h"
#include "qmenubar.h"
#include "qapplication.h"

/*!
  \class QLayout qabstractlayout.h
  \brief The QLayout class is the base class of geometry specifiers.

  \ingroup geomanagement

  This is an abstract base class. The concrete layout managers
  QBoxLayout and QGridLayout inherit from this one.

  Most users of Q*Layout are likely to use some of the basic functions
  provided by QLayout, such as	setMenuBar(), which is necessary
  to manage a menu bar because of the special properties of menu bars,
  and  freeze(), which allows you to freeze the widget's size and
  layout.

  To make your own layout manager, implement the functions
  minSize(), setGeometry() and removeWidget().

  Geometry management stops when the layout manager is deleted.
*/


/*!
  Creates a new top-level QLayout with main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder sets the value of defaultBorder(), which
  is interpreted by subclasses.	 If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  Having several top-level layouts for the same widget will cause
  considerable confusion.

*/

QLayout::QLayout( QWidget *parent, int border, int autoBorder, const char *name )
    : QObject( parent, name )
{
    menubar = 0;
    topLevel = FALSE;
    if ( parent ) {
	if ( parent->layout() ) {
	    warning( "QLayout \"%s\" added to %s \"%s\","
		     " which already had a layout.", QObject::name(),
		     parent->className(), parent->name() );
	} else {
	    topLevel = TRUE;
	    parent->installEventFilter( this );
	    setWidgetLayout( parent, this );
	}
    }
    outsideBorder = border;
    if ( autoBorder < 0 )
	insideSpacing = border;
    else
	insideSpacing = autoBorder;
}

/*!
  \fn QString QLayout::name() const

  Returns the internal object name.
*/

/*!
  \fn QMenuBar* QLayout::menuBar () const
  Returns the menu bar set for this layout, or a null pointer if no
  menu bar is set.
 */



/*!
  \fn bool QLayout::isTopLevel () const

 */

/*!
  \fn const QRect& QLayout::geometry ()

 */


/*!
  \fn bool QLayout::removeWidget (QWidget *w )
  This function is implemented in subclasses to remove \a w from geometry
  management.
 */


/*!
  \fn int QLayout::margin () const
  returns the border width.
 */


/*!
  Returns the main widget (parent widget) of this layout, or 0 if this
  layout is a sub-layout which is not yet inserted.
*/

QWidget * QLayout::mainWidget()
{
    if ( !topLevel ) {
	if ( parent() ) {
	    ASSERT( parent()->inherits( "QLayout" ) );
	    return ((QLayout*)parent())->mainWidget();
	} else {
	    return 0;
	}
    } else {
	ASSERT( parent() && parent()->isWidgetType() );
	return	(QWidget*)parent();
    }
}


/*!
  Constructs a new child QLayout,
  If \a autoBorder is -1, this QLayout inherits \a parent's
  defaultBorder(), otherwise \a autoBorder is used.
*/

QLayout::QLayout( int autoBorder, const char *name )
    : QObject( 0, name )
{
    menubar = 0;
    topLevel	 = FALSE;
    insideSpacing = autoBorder;
}



/*!
  This function is called whenever the parent widget receives a paint
  event. Reimplemented in subclasses to draw decorations that depend on
  the geometry of the layout.

  The default implementation does nothing.

  Note: The parent widget's \link QWidget::paintEvent()
  paintEvent()\endlink function is called after this function. Any
  painting done by the parent widget may obscure part or all of the
  decoration done by this function.
 */

void QLayout::paintEvent( QPaintEvent * )
{
    //############ must distribute to child layouts.
}





/*!
  Sets \a w's layout to \a l.
*/

void QLayout::setWidgetLayout( QWidget *w, QLayout *l )
{
    w->setLayout( l );
}



/*! \fn QSize QLayout::minSize()
  Returns the minimum size this layout needs.
*/

/*! \fn	 bool removeWidget( QWidget *w )

  Remove \a w from geometry management. This function is called
  automatically whenever a child widget is deleted.

  This function is implemented in subclasses. It is the
  responsibility of the reimplementor to propagate the call to
  sub-layouts.	This function returns TRUE if the widget was found.
 */

#if 0
/*!
  Implemented in subclasses to remove cached values used during
  geometry calculations, if any.

  The default implementation does nothing.
*/

void QLayout::clearCache()
{
}
#endif

/*!
  This function is reimplemented in subclasses to
  perform layout.

  The default implementation maintains the geometry() information.
 */
void QLayout::setGeometry( const QRect &r )
{
    rect = r;
}


/*!
  Performs child widget layout when the parent widget is resized.
  Also handles removal of widgets.
*/

bool QLayout::eventFilter( QObject *o, QEvent *e )
{
    if ( !o->isWidgetType() )
	return FALSE;

    //	  QWidget *p = (QWidget*)o;
    //		 if ( p != parentWidget() ) return FALSE;
    switch ( e->type() ) {
    case QEvent::Resize: {
	QResizeEvent *r = (QResizeEvent*)e;
	int mbh = 0;
	if ( menubar )
	    mbh = menubar->heightForWidth( r->size().width() );
	setGeometry( QRect( outsideBorder, mbh + outsideBorder,
			 r->size().width() - 2*outsideBorder,
			 r->size().height() - mbh - 2*outsideBorder ) );
	break;
    }
    case QEvent::ChildRemoved: {
	QChildEvent *c = (QChildEvent*)e;
	if ( c->child()->isWidgetType() ) {
	    QWidget *w = (QWidget*)c->child();
	    if ( w == menubar )
		menubar = 0;
	    removeWidget( w );
	    QEvent *lh = new QEvent( QEvent::LayoutHint );
	    QApplication::postEvent( o, lh );
	}
	break;
    }
    case QEvent::LayoutHint:
	activate(); //######## ######@#!#@!$ should be optimized somehow...
	break;
    case QEvent::Paint:
	paintEvent( (QPaintEvent*) e );
	break;
    default:
	break;
    }
    return FALSE;			    // standard event processing

}



/*!
  Deletes all layout children. Geometry management stops when
  a toplevel layout is deleted.
  \internal
  The layout classes will probably be fatally confused if you delete
  a sublayout
*/

QLayout::~QLayout()
{
    //note that this function may be called during the QObject destructor,
    //when the parent no longer is a QWidget.
    if ( isTopLevel() && parent()->isWidgetType() &&
	 ((QWidget*)parent())->layout() == this )
	setWidgetLayout( (QWidget*)parent(), 0 );
}


/*!
  This function is called from addLayout functions in subclasses,
  to add \a l layout as a sublayout.
*/
//############## do we like this API???
void QLayout::addChildLayout( QLayout *l )
{
    if ( l->topLevel ) {
#if defined(CHECK_NULL)
	warning( "QLayout: Attempt to add top-level layout as child" );
#endif
	return;
    }
    if ( l->parent() ) {
#if defined(CHECK_NULL)
	warning( "QLayout::addChildLayout(), layout already added." );
#endif
	return;
    }
    insertChild( l );
    if ( l->insideSpacing < 0 )
	l->insideSpacing = insideSpacing;
}

/*!
  \fn int QLayout::defaultBorder() const
  Returns the default border for the geometry manager.
*/

/*!
  \overload void QLayout::freeze()

  This version of the method fixes the main widget at its minimum size.
  You can also achieve this with freeze( 0, 0 );
*/


/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets. For widgets which should not be
  resizable, but where a QLayout subclass is used to set up the initial
  geometry.

  A frozen layout cannot be unfrozen, the only sensible thing to do
  is to delete it.

  The size is adjusted to a valid value. Thus freeze(0,0) fixes the
  widget to its minimum size.
*/

void QLayout::freeze( int w, int h )
{
    warning( "QLayout::freeze( %d, %d ) not implemented", w, h );
}


/*!
  Makes the geometry manager take account of the menu bar \a w. All
  child widgets are placed below the bottom edge of the menu bar.

  A menu bar does its own geometry managing, never do addWidget()
  on a menu bar.
*/

void QLayout::setMenuBar( QMenuBar *w )
{
    menubar = w;
}




/*!
  Returns the minimum size of this layout. This is the smallest size
  that the layout can have, while still respecting the specifications.

  The default implementation allows unlimited resizing.
*/

QSize QLayout::minimumSize()
{
    return QSize( 0, 0 );
}


/*!
  Returns the maximum size of this layout. This is the largest size
  that the layout can have, while still respecting the specifications.

  The default implementation allows unlimited resizing.
*/

QSize QLayout::maximumSize()
{
    return QSize( QCOORD_MAX, QCOORD_MAX );
}


/*!
  Returns whether this layout can make use of more space than
  sizeHint().  A value of Vertical or Horizontal means that it wants
  to grow in only one dimension, while BothDirections means that it wants to
  grow in both dimensions.

  The default implementation returns NoDirection.
*/

QSizePolicy::Expansiveness QLayout::expansive()
{
    return QSizePolicy::NoDirection;
}

/*!  Redoes the layout for mainWidget().  You should generally not
  need to call this, as it is automatically called at most appropriate
  times.

  However, if you set up a QLayout for a visible widget without
  resizing that widget, you need to call this function in order to lay
  it out.
*/

bool QLayout::activate()
{
    // Paul: If adding stuff to a QLayout for a widget causes
    // postEvent(thatWidget, QEvent::LayoutHint), activate() becomes
    // unnecessary in that case too.

#if 1
    invalidate(); //######### need to invalidate all child layouts!!!
    QSize s = mainWidget()->size();
    int mbh = menubar ? menubar->heightForWidth( s.width() ) : 0;
    setGeometry( QRect( outsideBorder, mbh + outsideBorder,
			s.width() - 2*outsideBorder,
			s.height() - mbh - 2*outsideBorder ) );
#endif
    return TRUE;
}




/*!
  Invalidates any cached information in this layout.

  The default implementation does nothing.
*/

void QLayout::invalidate()
{

}
