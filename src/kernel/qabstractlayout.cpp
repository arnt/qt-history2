/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qabstractlayout.cpp#36 $
**
** Implementation of the abstract layout base class
**
** Created : 960416
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qabstractlayout.h"
#include "qwidget.h"
#include "qmenubar.h"
#include "qapplication.h"



/*!
  \class QLayoutItem qabstractlayout.h
  \brief The abstract items which a QLayout manipulates.

  For custom layouts.

  \sa QLayout
*/

/*!
  \class QSpacerItem qabstractlayout.h
  \brief A QLayoutItem that represents blank space.

  For custom layouts.

  \sa QLayout
*/

/*!
  \class QWidgetItem qabstractlayout.h
  \brief A QLayoutItem that represents widget.

  For custom layouts.

  \sa QLayout
*/




/*! \fn QLayoutItem::QLayoutItem (int alignment)
  Constructs a layout item with alignment \a alignment.
  Alignment may not be supported by all subclasses.
 */

/*! \fn int QLayoutItem::alignment () const
  Returns the alignment of this item.
*/


/*! Sets the alignment of this item to \a a.
*/

void QLayoutItem::setAlignment( int a )
{
     align = a;
}


/*! \fn  QSize QLayoutItem::maximumSize () const
  Implemented in subclasses to return the maximum size of this item.
*/

/*! \fn QSize QLayoutItem::minimumSize () const
  Implemented in subclasses to return the minimum size of this item.
*/

/*! \fn QSize QLayoutItem::sizeHint () const
  Implemented in subclasses to return the preferred size of this item.
*/

/*! \fn QSizePolicy::ExpandData QLayoutItem::expanding () const
  Implemented in subclasses to return whether this item "wants" to expand.
*/

/*! \fn void QLayoutItem::setGeometry (const QRect &r )
  Implemented in subclasses to set this item's geometry to \a r.
*/

/*!
  \fn QRect QLayoutItem::geometry() const

  Returns the rectangle covered by this layout item.
 */



/*! \fn virtual bool QLayoutItem::isEmpty () const
  Implemented in subclasses to return whether this item is empty,
  i.e. whether it contains any widgets.
*/


/*! \fn QSpacerItem::QSpacerItem (int w, int h, QSizePolicy::SizeType hData=QSizePolicy::Minimum, QSizePolicy::SizeType vData= QSizePolicy::Minimum)

  Constructs a spacer item with preferred width \a w, preferred height
  \a h, horizontal size policy \a hData and vertical size policy
  \a vData.

  The default values gives a gap that is able to stretch,
  if nothing else wants the space.
*/

/*! \fn QWidgetItem::QWidgetItem (QWidget * w)

  Creates an item containing \a w.
*/

 /*!
  Destructs the QLayoutItem.
*/
QLayoutItem::~QLayoutItem()
{
}


/*!
  Invalidates any cached information in this layout item.

  The default implementation does nothing.
*/

void QLayoutItem::invalidate()
{

}


/*!
  If this item consists of a single QWidget, that widget is returned.
  The default implementation returns 0;
*/

QWidget * QLayoutItem::widget()
{
    return 0;
}


/*!
  Returns the widget managed by this item.
*/

QWidget * QWidgetItem::widget()
{
    return wid;
}


/*!
  Returns TRUE if this layout's preferred height depends on its
  width. The default implementation returns FALSE;

  Reimplement this function in layout managers that support
  height for width.

  \sa heightForWidth(), QWidget::heightForWidth()
*/

bool QLayoutItem::hasHeightForWidth() const
{
    return FALSE;
}





/*!
  Returns an iterator over this item's QLayoutItem children.
  The default implementation returns an empty iterator.

  Reimplement this function in subclasses that can have
  children.
*/

QLayoutIterator QLayoutItem::iterator()
{
    return QLayoutIterator( 0 );
}



/*!
  Returns the preferred height for this layout item, given the width
  \a w.

  The default implementation returns -1, indicating that the preferred
  height is independent of the width of the item.  Using the function
  hasHeightForWidth() will typically be much faster than calling this
  function and testing for -1.

  Reimplement this function in layout managers that support
  height for width. A typical implementation will look like this:
  \code
  int MyLayout::heightForWidth( int w ) const
  {
      if ( cache_dirty || cached_width != w ) {
          //Not all C++ compilers support "mutable" yet:
          MyLayout * mthis = (MyLayout*)this;
	  int h = calculateHeightForWidth( w );
          mthis->cached_hfw = h;
	  return h;
      }
      return cached_hfw;
  }
  \endcode

  Caching is strongly recommended, without it layout will take
  exponential time.  \sa hasHeightForWidth()
*/

int QLayoutItem::heightForWidth( int ) const
{
    return -1;
}



static const int HorAlign = Qt::AlignHCenter | Qt::AlignRight | Qt::AlignLeft;
static const int VerAlign = Qt::AlignVCenter | Qt::AlignBottom | Qt::AlignTop;

static QSize smartMinSize( QWidget *w )
{
    QSize s(0,0);
    if ( w->layout() ) {
	//###this is hacky
	s = w->layout()->minimumSize();
    } else {
	if ( w->sizePolicy().mayShrinkHorizontally() )
	    s.setWidth( w->minimumSizeHint().width() );
	else
	    s.setWidth( w->sizeHint().width() );
	if ( w->sizePolicy().mayShrinkVertically() )
	    s.setHeight( w->minimumSizeHint().height() ); 
	else
	    s.setHeight( w->sizeHint().height() );
    }
    QSize min = w->minimumSize();
    if ( min.width() > 0 )
	s.setWidth( min.width() );
    if ( min.height() > 0 )
	s.setHeight( min.height() );

    return s;
}

//returns the max size of a box containing \a w with alignment \a align.
static QSize smartMaxSize( QWidget *w, int align = 0 )
{
    if ( align & HorAlign && align & VerAlign )
	return QSize( QCOORD_MAX, QCOORD_MAX );
    QSize s = w->maximumSize();
    if ( s.width() == QCOORD_MAX && !(align&HorAlign) )
	if ( !w->sizePolicy().mayGrowHorizontally() )
	    s.setWidth( w->sizeHint().width() );

    if ( s.height() ==  QCOORD_MAX && !(align&VerAlign) )
	if ( !w->sizePolicy().mayGrowVertically() )
	    s.setHeight( w->sizeHint().height() );

    //s = s.expandedTo( w->minimumSize() ); //### ???

    if (align & HorAlign )
	s.setWidth( QCOORD_MAX );
    if (align & VerAlign )
	s.setHeight( QCOORD_MAX );
    return s;
}




/*!
  This function stores \a r, so it can be returned by geometry().
*/
void QSpacerItem::setGeometry( const QRect &r )
{
    rect = r;
}

/*!
  Sets the geometry of this item's widget to be contained within \a r,
  taking alignment and maximum size into account.
*/
void QWidgetItem::setGeometry( const QRect &r )
{
    QSize s = r.size().boundedTo( smartMaxSize( wid ) );
    int x = r.x();
    int y = r.y();
    if ( align & (HorAlign|VerAlign) ) {
	QSize pref = wid->sizeHint().expandedTo( wid->minimumSize() ); //###
	if ( align & HorAlign )
	    s.setWidth( QMIN( s.width(), pref.width() ) );
	if ( align & VerAlign ) {
	    if ( hasHeightForWidth() )
		s.setHeight( QMIN( s.height(), heightForWidth(s.width()) ) );
	    else
		s.setHeight( QMIN( s.height(), pref.height() ) );
	}
    }
    if ( align & Qt::AlignRight )
	x = x + ( r.width() - s.width() );
    else if ( !(align & Qt::AlignLeft) )
	x = x + ( r.width() - s.width() ) / 2;

    if ( align & Qt::AlignBottom )
	y = y + ( r.height() - s.height() );
    else if ( !(align & Qt::AlignTop) )
	y = y + ( r.height() - s.height() ) / 2;

    wid->setGeometry( x, y, s.width(), s.height() );
}



/*!
  \reimp
*/

QRect QSpacerItem::geometry() const
{
    return rect;
}


/*!
  \reimp
*/

QRect QWidgetItem::geometry() const
{
    return wid->geometry();
}


/*!
  \reimp
*/

QRect QLayout::geometry() const
{
    return rect;
}



/*!
  \reimp
*/

bool QWidgetItem::hasHeightForWidth() const
{
    if ( isEmpty() )
	return FALSE;
    if ( wid->layout() )
	return wid->layout()->hasHeightForWidth();
    return wid->sizePolicy().hasHeightForWidth();
}

/*!
  \reimp
*/

int QWidgetItem::heightForWidth( int w ) const
{
    if ( wid->layout() )
	return wid->layout()->totalHeightForWidth( w );
    return wid->heightForWidth( w );
}

/*!
  Returns whether this space item is expanding.
*/
QSizePolicy::ExpandData QSpacerItem::expanding() const
{
    return sizeP.expanding();
}

/*!
  Returns whether this item's widget is expanding.
*/

QSizePolicy::ExpandData QWidgetItem::expanding() const
{
    if ( isEmpty() )
	return QSizePolicy::NoDirection;
    return wid->layout() ? wid->layout()->expanding()
	: wid->sizePolicy().expanding();
}

/*!
  Returns the minimum size of this space item.
*/
QSize QSpacerItem::minimumSize() const
{
    return QSize( sizeP.mayShrinkHorizontally() ? 0 : width,
		  sizeP.mayShrinkVertically() ? 0 : height );;
}

/*!
  Returns the minimum size of this item.
*/

QSize QWidgetItem::minimumSize() const
{
    if ( isEmpty() )
	return QSize(0,0);
    return smartMinSize( wid );
}


/*!
  Returns the maximum size of this space item.
*/
QSize QSpacerItem::maximumSize() const
{
    return QSize( sizeP.mayGrowHorizontally() ? QCOORD_MAX : width,
		  sizeP.mayGrowVertically() ? QCOORD_MAX : height );
}

/*!
  Returns the maximum size of this item.
*/
QSize QWidgetItem::maximumSize() const
{
	return smartMaxSize( wid, align );
}

/*!
  Returns the preferred size of this space item.
*/
QSize QSpacerItem::sizeHint() const
{
	return QSize( width, height );
}

/*!
  Returns the preferred size of this item.
*/
QSize QWidgetItem::sizeHint() const
{
    if ( isEmpty() )
	return QSize(0,0);
    //########### Should minimumSize() override sizeHint ????????????
    if ( wid->layout() )
	return QSize( QMAX( wid->sizeHint().width(), wid->minimumWidth() ),
		      QMAX( wid->sizeHint().height(), wid->minimumHeight() ));
    return QSize( wid->minimumWidth() == 0 ?
		  wid->sizeHint().width() : wid->minimumWidth(),
		  wid->minimumHeight() == 0 ?
		  wid->sizeHint().height() : wid->minimumHeight() );
}

/*!
  Returns TRUE, since a space item never contains widgets.
*/
bool QSpacerItem::isEmpty() const
{
    return TRUE;
}

/*!
  Returns TRUE, if the widget has been hidden, FALSE otherwise.
*/
bool QWidgetItem::isEmpty() const
{
    return wid->testWState( QWidget::WState_ForceHide );
}




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

  To make your own layout manager, make a subclass of QGLayoutIterator
  and implement the functions addItem(), sizeHint(), setGeometry() and
  iterator().

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

  There can only be one top-level layout for a widget. It is returned
  by QWidget::layout()
*/

QLayout::QLayout( QWidget *parent, int border, int autoBorder, const char *name )
    : QObject( parent, name )
{
    menubar = 0;
    topLevel = FALSE;
    frozen = FALSE;
    autoMinimum = FALSE;
    activated = FALSE;
    if ( parent ) {
	if ( parent->layout() ) {
	    warning( "QLayout \"%s\" added to %s \"%s\","
		     " which already had a layout.", QObject::name(),
		     parent->className(), parent->name() );
	} else {
	    topLevel = TRUE;
	    if ( parent->isTopLevel() )
		autoMinimum = TRUE;
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



/*! \fn void QLayout::addItem (QLayoutItem *item )
    Implemented in subclasses to add \a item. How it is
    added is specific to each subclass.
*/

/*! \fn QLayoutIterator iterator();
  Implemented in subclasses to return an iterator that iterates over
  the children of this layout.

  Layout implementors must subclass QGLayoutIterator.
*/

/*!
  \fn void QLayout::add (QWidget * w)

  Adds \a w to this layout in a manner specific to the layout. This
  function uses addItem.
*/

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

  Returns TRUE if this layout is a top level layout, i.e. not a child
  of another layout.
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
  Constructs a new child QLayout, and places it inside
  \a parentLayout, using the default placement defined by
  addItem().

  If \a autoBorder is -1, this QLayout inherits \a parentLayout's
  defaultBorder(), otherwise \a autoBorder is used.

*/

QLayout::QLayout( QLayout *parentLayout, int autoBorder, const char *name )
    : QObject( parentLayout, name )

{
    menubar = 0;
    topLevel = FALSE;
    insideSpacing = autoBorder < 0 ? parentLayout->insideSpacing : autoBorder;
    parentLayout->addItem( this );
}


/*!
  Constructs a new child QLayout,
  If \a autoBorder is -1, this QLayout inherits its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  This layout has to be inserted into another layout before use.
*/

QLayout::QLayout( int autoBorder, const char *name )
    : QObject( 0, name )
{
    menubar = 0;
    topLevel	 = FALSE;
    insideSpacing = autoBorder;
}


/*!
  Returns TRUE if this layout is empty.
  The default implementation returns FALSE.
*/
bool QLayout::isEmpty() const
{
    return FALSE; //### should check
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


/*!
  This function is reimplemented in subclasses to
  perform layout.

  The default implementation maintains the geometry() information.
  Reimplementors must call this function.
 */
void QLayout::setGeometry( const QRect &r )
{
    rect = r;
}


static bool removeWidget( QLayoutItem *lay, QWidget *w )
{
    QLayoutIterator it = lay->iterator();
    QLayoutItem *child;
    while ( (child = it.current() ) ) {
	if ( child->widget() == w ) {
	    it.removeCurrent();
	    lay->invalidate();
	    return TRUE;
	} else if ( removeWidget( child, w ) ) {
	    lay->invalidate();
	    return TRUE;
	}
	it.next();
    }
    return FALSE;
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
	if ( activated )
	    setGeometry( QRect( outsideBorder, mbh + outsideBorder,
			 r->size().width() - 2*outsideBorder,
			 r->size().height() - mbh - 2*outsideBorder ) );
	else
	    activate();
	break;
    }
    case QEvent::ChildRemoved: {
	QChildEvent *c = (QChildEvent*)e;
	QWidget *w = (QWidget*)c->child();
	if ( w == menubar )
	    menubar = 0;
	if ( removeWidget( this, w ) ) {
	    QEvent *lh = new QEvent( QEvent::LayoutHint );
	    QApplication::postEvent( o, lh );
	}
	break;
    }
    case QEvent::LayoutHint:
	activate(); //######## Check that LayoutHint events are collapsed
	break;
    default:
	break;
    }
    return FALSE;			    // standard event processing

}


/*!
  \internal
  Also takes outsideBorder and menu bar into account. May change name
  or disappear altogether if we find a better solution.
*/

int QLayout::totalHeightForWidth( int w ) const
{
    int b = topLevel ? 2*outsideBorder : 0;
    int h = heightForWidth( w - b ) + b;
    if ( menubar )
	h += menubar->heightForWidth( w );
    return h;
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
    if ( isTopLevel() && parent() && parent()->isWidgetType() &&
	 ((QWidget*)parent())->layout() == this )
	setWidgetLayout( (QWidget*)parent(), 0 );
}


/*!
  This function is called from addLayout functions in subclasses,
  to add \a l layout as a sublayout.
*/
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
  \obsolete
  Fixes the size of the main widget and distributes the available
  space to the child widgets. For widgets which should not be
  resizable, but where a QLayout subclass is used to set up the initial
  geometry.

  As a special case, freeze(0,0) is equivalent to setResizeMode( \c Fixed )
*/

void QLayout::freeze( int w, int h )
{
    if ( w <= 0 || h <= 0 ) {
	setResizeMode( Fixed );
    } else {
	setResizeMode( FreeResize ); // layout will not change min/max size
	setFixedSize( w, h );
    }
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
  \fn QSize QLayout::sizeHint()

  Implemented in subclasses to return the preferred size of this layout.
*/

/*!
  Returns the minimum size of this layout. This is the smallest size
  that the layout can have, while still respecting the specifications.

  The default implementation allows unlimited resizing.
*/

QSize QLayout::minimumSize() const
{
    return QSize( 0, 0 );
}


/*!
  Returns the maximum size of this layout. This is the largest size
  that the layout can have, while still respecting the specifications.

  The default implementation allows unlimited resizing.
*/

QSize QLayout::maximumSize() const
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

QSizePolicy::ExpandData QLayout::expanding() const
{
    return QSizePolicy::NoDirection;
}




static void  invalidateRecursive( QLayoutItem *lay )
{
    lay->invalidate();
    QLayoutIterator it = lay->iterator();
    QLayoutItem *child;
    while ( (child = it.current() ) ) {
	invalidateRecursive( child );
	it.next();
    }
}




/*!  Redoes the layout for mainWidget().  You should generally not
  need to call this, as it is automatically called at most appropriate
  times.

  However, if you set up a QLayout for a visible widget without
  resizing that widget, you need to call this function in order to lay
  it out.

  \sa QWidget::updateGeometry()
*/

bool QLayout::activate()
{
    // Paul: If adding stuff to a QLayout for a widget causes
    // postEvent(thatWidget, QEvent::LayoutHint), activate() becomes
    // unnecessary in that case too.
    activated = TRUE;
    invalidateRecursive( this );
    QSize s = mainWidget()->size();
    int mbh = menubar ? menubar->heightForWidth( s.width() ) : 0;
    setGeometry( QRect( outsideBorder, mbh + outsideBorder,
			s.width() - 2*outsideBorder,
			s.height() - mbh - 2*outsideBorder ) );
    if ( frozen )
	mainWidget()->setFixedSize( sizeHint() ); //### will trigger resize
    else if ( autoMinimum )
	mainWidget()->setMinimumSize( minimumSize() );

    //###if ( sizeHint or sizePolicy has changed )
    mainWidget()->updateGeometry();
    return TRUE;
}





/*!
  \class QSizePolicy qsizepolicy.h
  \brief A layout attribute describing horizontal and vertical resizing.

  Widgets which override QWidget::sizePolicy() return a QSizePolicy
  describing the horizontal and vertical resizing policy best used when
  laying out the widget.

  Only the constructor is of interest in most applications.
*/

/*!
  \fn QSizePolicy::QSizePolicy ()

  Default constructor, produces a minimally initialized QSizePolicy.
*/

/*!
  \fn QSizePolicy::QSizePolicy (SizeType hor, SizeType ver, bool hfw=FALSE)

  This is the constructor normally used to return a value in the overridden
  \link QWidget::sizeHint() sizeHint() \endlink function of a QWidget
  subclass.

  It constructs a QSizePolicy with independent horizontal and vertical
  sizing types, \a hor and \a ver respectively.  These sizing types
  affect how the widget is treated by the \a link QLayout layout
  engine\endlink.

  \define QSizePolicy::SizeType

  The sizing types are:
<ul>
    <li> \c Fixed - the sizeHint() is the only acceptable alternative,
		so never grow or shrink
		(eg. the vertical direction of a pushbutton)
    <li> \c Minimum - the sizeHint() is minimal, and sufficient. The widget
		can be expanded, but there is no advantage to it being larger.
		(eg. the horizontal direction of a pushbutton)
    <li> \c Maximum - the sizeHint() is a maximum, the widget can be shrunk
		any amount without detriment if other widgets need the space
	       	(eg. a separator line)
    <li> \c Preferred - the sizeHint() is best, but the widget can
 		be shrunk below that and still be useful. The widget
		can be expanded, but there is no advantage to it being
		larger than sizeHint()
		(the default QWidget policy)
    <li> \c MinimumExpanding - the sizeHint() is a minimum,
		the widget can make use of extra space, so it
		should get as much space as possible.
		(not currently used by any standard Qt widgets)
    <li> \c Expanding - the sizeHint() is a sensible size, but the widget can
 		be shrunk below that and still be useful.
		The widget can make use of extra space, so it should
		get as much space as possible.
		(eg. the horizontal direction of a slider)
</ul>

  If \a hfw is TRUE, the preferred height of the widget is dependent on the
  width of the widget (for example, a widget with automatic word-breaking).
*/

/*! \fn QSizePolicy::SizeType QSizePolicy::horData() const
Returns the horizontal component of the size policy.
*/


/*! \fn QSizePolicy::SizeType QSizePolicy::verData() const
Returns the vertical component of the size policy.
*/

/*! \fn bool QSizePolicy::mayShrinkHorizontally() const
Returns TRUE if the widget can sensibly be narrower than its sizeHint().
*/


/*! \fn bool QSizePolicy::mayShrinkVertically() const
Returns TRUE if the widget can sensibly be lower than its sizeHint().
*/


/*! \fn bool QSizePolicy::mayGrowHorizontally() const
Returns TRUE if the widget can sensibly be wider than its sizeHint().
*/


/*! \fn bool QSizePolicy::mayGrowVertically() const
Returns TRUE if the widget can sensibly be taller than its sizeHint().
*/

/*! \fn QSizePolicy::ExpandData QSizePolicy::expanding() const
Returns a value indicating if the widget
*/

/*! \fn void QSizePolicy::setHorData( SizeType d )
Sets the horizontal component of the size policy to \a d.
*/


/*! \fn void QSizePolicy::setVerData( SizeType d )
Sets the vertical component of the size policy to \a d.
*/

/*! \fn bool QSizePolicy::hasHeightForWidth() const
Returns TRUE if the widget's preferred height depends on its width.
*/


/*! \fn void QSizePolicy::setHeightForWidth( bool b )
Sets the hasHeightForWidth() flag to \a b.
*/


/*!
  \class QGLayoutIterator qabstractlayout.h
  \brief The abstract base class of internal layout iterators.

  To be subclassed by custom layout implementors.

  The QGLayoutIterator implements the functionality of
  QLayoutIterator. Each subclass of QLayout needs a
  QGLayoutIterator subclass.
*/


/*! \fn uint QGLayoutIterator::count() const
  Implemented in subclasses to return the number of items in the layout.
 */

/*! \fn void QGLayoutIterator::toFirst()
  Implemented in subclasses to move the iterator to the first item
  in the layout.
*/

/*! \fn void QGLayoutIterator::next()
  Implemented in subclasses to move the iterator to the next item.
 */

/*! \fn QLayoutItem *QGLayoutIterator::current()
  Implemented in subclasses to return the current item, or 0 if there
  is no next element.
 */

/*! \fn void QGLayoutIterator::removeCurrent()
  Implemented in subclasses to remove the current item and move
  the iterator to the next item.
 */


/*!
  \class QLayoutIterator qabstractlayout.h
  \brief The QLayoutIterator class provides iterators over QLayoutItem

  Use QLayoutItem::iterator() to create an iterator over a layout.

  QLayoutIterator uses explicit sharing with a reference count. If
  an iterator is copied, and one of the copies is modified,
  both iterators will be modified.

  A QLayoutIterator is not protected against changes in its layout. If
  the layout is modified or deleted, the iterator will become invalid.
  It is not posible to test for validity. It is safe to delete an
  invalid layout. Any other access may lead to an illegal memory
  reference, and the abnormal termination of the program.

  Calling removeCurrent() leaves the iterator in a valid state, but
  may invalidate any other iterators that access the same layout.


  The following code will draw a rectangle for each layout item
  in the layout structure of the widget.
  \code
  static void paintLayout( QPainter *p, QLayoutItem *lay )
  {
      QLayoutIterator it = lay->iterator();
      QLayoutItem *child;
      while ( (child = it.current() ) ) {
          paintLayout( p, child );
	  it.next();
      }
      p->drawRect( lay->geometry() );
  }
  void ExampleWidget::paintEvent( QPaintEvent * )
  {
      QPainter p( this );
      if ( layout() )
          paintLayout( &p, layout() );
  }
  \endcode

  All the functionality of QLayoutIterator is implemented by
  subclasses of QGLayoutIterator. Note that there is not much
  point in subclassing QLayoutIterator, since ~QLayoutIterator() is not
  virtual.
*/




/*! \fn QLayoutIterator::QLayoutIterator( QGLayoutIterator *i )
  Constructs an iterator based on \a i.

  This constructor is provided for layout implementors. Application
  programmers should use QLayoutItem::iterator() to create an iterator
  over a layout.
*/

/*! \fn QLayoutIterator::QLayoutIterator( const QLayoutIterator &i )
  Creates a shallow copy of \a i; if the copy is modified, then the
  original will also be modified.
*/

/*! \fn QLayoutIterator::~QLayoutIterator()
  Destroys the iterator.
*/

/*! \fn QLayoutIterator &QLayoutIterator::operator=( const QLayoutIterator &i )
  Assigns \a i to this iterator and returns a reference to this iterator.
*/

/*! \fn uint QLayoutIterator::count() const
  Returns the number of child items in the layout item this iterator
  iterates over.
*/

/*! \fn bool QLayoutIterator::isEmpty() const
  Returns TRUE if the layout item has no children.
*/

/*! \fn void QLayoutIterator::toFirst()
   Moves the iterator to the first child item.
*/

/*! \fn void QLayoutIterator::next()
  Moves the iterator to the next child item.
*/

/*! \fn QLayoutItem *QLayoutIterator::current()
  Returns the current item, or 0 if there is no current item.
*/

/*! \fn void QLayoutIterator::removeCurrent()
  Removes the current child item from the layout and moves the
  iterator to the next item. This iterator will still be valid, but any
  other iterator over the same layout may become invalid.
*/


/*!
  \define QLayout::ResizeMode
  Sets the resize mode to \a mode.

    The possible values are are:
<ul>
    <li> \c Fixed - the main widget's size is set to sizeHint(), it
    cannot be resized at all.
    <li> \c Minimum - The main widget's minimum size is set to
    minimumSize(), it cannot be smaller.
    <li> \c FreeResize - the widget is not constrained.
</ul>
    The default value is \c Minimum for top level widgets, and \c FreeResize
    for all others.

*/

void QLayout::setResizeMode( ResizeMode mode )
{
    if ( mode == resizeMode() )
	return;
    switch (mode) {
    case Fixed:
	frozen = TRUE;
	break;
    case FreeResize:
	frozen = FALSE;
	autoMinimum = FALSE;
	break;
    case Minimum:
	frozen = FALSE;
	autoMinimum = TRUE;
	break;
    }
    activate();
}


/*!
  Returns the resize mode.
*/

QLayout::ResizeMode QLayout::resizeMode() const
{
    return frozen ? Fixed : (autoMinimum ? Minimum : FreeResize );
}
