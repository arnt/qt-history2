/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qabstractlayout.cpp#11 $
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
  \brief The abstract items with a QLayout manipulates.

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

/*! \fn void QLayoutItem::setAlignment (int a)
  Sets the alignment of this item to \a a.
*/



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

/*! \fn SearchResult QLayoutItem::removeW (QWidget *w )
  Implemented in subclasses to search for \a w, and remove it it if found.
*/

/*! \fn void QLayoutItem::setGeometry (const QRect &r )
  Implemented in subclasses to set this item's geometry to \a r.
*/


/*! \fn virtual bool QLayoutItem::isEmpty () const
  Implemented in subclasses to return whether this item is empty,
  i.e. whether it contains any widgets.
*/


/*! \fn QSpacerItem::QSpacerItem (int w, int h, QSizePolicy::SizeType hData=QSizePolicy::Minimum, QSizePolicy::SizeType vData= QSizePolicy::Minimum)

  Constructs a spacer item with preferred width \a w, preferred height
  \a h, horizontal size policy \a hData and vertical size policy \a
  vData. The default values gives a space that is able to stre, if nothing else wants the space.
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
  Returns the preferred height for this layout item, given the width
  \a w.

  The default implementation returns -1, indicating that the preferred
  height is independent of the width of the item.  The function
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

  Note that caching is essential, without it layout will take
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
	if ( !w->sizePolicy().mayShrinkHorizontally() )
	    s.setWidth( w->sizeHint().width() );
	if ( !w->sizePolicy().mayShrinkVertically() )
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
    QSize s = w->maximumSize();
    if ( s.width() == QCOORD_MAX )
	if ( !w->sizePolicy().mayGrowHorizontally() )
	    s.setWidth( w->sizeHint().width() );

    if ( s.height() ==  QCOORD_MAX )
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
  This function does nothing.
*/
void QSpacerItem::setGeometry( const QRect& )
{
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
    QSize pref = wid->sizeHint().expandedTo( wid->minimumSize() ); //###
	
    if ( align & HorAlign )
	s.setWidth( QMIN( s.width(), pref.width() ) );
    if ( align & VerAlign )
	s.setHeight( QMIN( s.height(), pref.height() ) );
	
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
  \reimplementation
*/

bool QWidgetItem::hasHeightForWidth() const
{
    if ( wid->layout() )
	return wid->layout()->hasHeightForWidth();
    return wid->sizePolicy().hasHeightForWidth();
}

/*!
  \reimplementation
*/

int QWidgetItem::heightForWidth( int w ) const
{
    if ( wid->layout() )
	return wid->layout()->heightForWidth( w );
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
    return wid->sizePolicy().expanding();
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
  Returns FALSE, since a space item never contains widgets.
*/
bool QSpacerItem::isEmpty() const
{
    return TRUE;
}

/*!
  Returns TRUE, since a widget item always contains a widget!
*/
bool QWidgetItem::isEmpty() const
{
    return FALSE;
}


/*!
  \fn SearchResult QWidgetItem::removeW( QWidget *w )

  Returns FoundAndDeleteable, if \a w is found in this item.
*/
QLayoutItem::SearchResult QWidgetItem::removeW( QWidget *w )
{
    return wid == w ? FoundAndDeleteable : NotFound;
}


/*!
  \fn SearchResult QSpacerItem::removeW( QWidget *)

  Returns NotFound, since a space item never contains widgets.
*/
QLayoutItem::SearchResult QSpacerItem::removeW( QWidget *)
{
    return NotFound;
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



/*! \fn void QLayout::addItem (QLayoutItem *item )
    Implemented in subclasses to add \a item. How it is
    added is specific to each subclass.
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
  \fn const QRect& QLayout::geometry ()

  Returns the rectangle covered by this layout.
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
  Returns TRUE if this layout is empty.
  The default implementation returns FALSE.
*/
bool QLayout::isEmpty() const
{
    return FALSE; //### should check
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

/*!
  \fn SearchResult QLayout::removeW( QWidget *w)

  Searches all subitems, returning \c Found if \a w was found and
  removed. Returns \c NotFound otherwise.

  Future versions may return FoundAndDeleteable if this removal caused the
  layout to become empty.
 */
QLayoutItem::SearchResult QLayout::removeW( QWidget *w)
{
    return removeWidget( w ) ? Found : NotFound;
}

/*! \fn  bool QLayout::removeWidget (QWidget *w )

  Implemented in subclasses to look for \a w and remove it if found.
  It is the responsibility of the reimplementor to propagate the call
  to sub-layouts.  Returns TRUE if found, FALSE if not found.
*/


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

