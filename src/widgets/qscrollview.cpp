/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollview.cpp#122 $
**
** Implementation of QScrollView class
**
** Created : 950524
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qwidget.h"
#include "qscrollbar.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qfocusdata.h"
#include "qscrollview.h"
#include "qptrdict.h"
#include "qapplication.h"

const int coord_limit = 4000;

struct QSVChildRec {
    QSVChildRec(QWidget* c, int xx, int yy) :
	child(c),
	x(xx), y(yy)
    {
    }

    void moveBy(QScrollView* sv, int dx, int dy, QWidget* clipped_viewport)
    {
	moveTo( sv, x+dx, y+dy, clipped_viewport );
    }
    void moveTo(QScrollView* sv, int xx, int yy, QWidget* clipped_viewport)
    {
	if ( x != xx || y != yy ) {
	    x = xx;
	    y = yy;
	    hideOrShow(sv,clipped_viewport);
	}
    }
    void hideOrShow(QScrollView* sv, QWidget* clipped_viewport)
    {
	if ( clipped_viewport ) {
	    if ( x+child->width() < sv->contentsX()+clipped_viewport->x()
	      || x > sv->contentsX()+clipped_viewport->width()
	      || y+child->height() < sv->contentsY()+clipped_viewport->y()
	      || y > sv->contentsY()+clipped_viewport->height() )
	    {
		child->move(clipped_viewport->width(),
			    clipped_viewport->height());
	    } else {
		child->move(x-sv->contentsX()-clipped_viewport->x(),
			    y-sv->contentsY()-clipped_viewport->y());
	    }
	} else {
	    if ( x-sv->contentsX() < -child->width()
	      || x-sv->contentsX() > sv->visibleWidth()
	      || y-sv->contentsY() < -child->height()
	      || y-sv->contentsY() > sv->visibleHeight() )
	    {
		child->move(sv->visibleWidth()+10000,
			    sv->visibleHeight()+10000);
	    } else {
		child->move(x-sv->contentsX(), y-sv->contentsY());
	    }
	}
    }
    QWidget* child;
    int x, y;
};

struct QScrollViewData {
    QScrollViewData(QWidget* parent, int vpwflags) :
	hbar( QScrollBar::Horizontal, parent, "qt_hbar" ),
	vbar( QScrollBar::Vertical, parent, "qt_vbar" ),
	viewport( parent, "qt_viewport", vpwflags ),
	clipped_viewport( 0 ),
	vx( 0 ), vy( 0 ), vwidth( 1 ), vheight( 1 )
    {
	l_marg = r_marg = t_marg = b_marg = 0;
	viewport.setBackgroundMode( QWidget::PaletteDark );
	vMode = QScrollView::Auto;
	hMode = QScrollView::Auto;
	corner = 0;
	vbar.setSteps( 20, 1/*set later*/ );
	hbar.setSteps( 20, 1/*set later*/ );
	policy = QScrollView::Default;
	signal_choke = FALSE;
    }
    ~QScrollViewData()
    {
	deleteAll();
    }

    QSVChildRec* rec(QWidget* w) { return childDict.find(w); }
    QSVChildRec* ancestorRec(QWidget* w)
    {
	if ( clipped_viewport ) {
	    while (w->parentWidget() != clipped_viewport) {
		w = w->parentWidget();
		if (!w) return 0;
	    }
	} else {
	    while (w->parentWidget() != &viewport) {
		w = w->parentWidget();
		if (!w) return 0;
	    }
	}
	return rec(w);
    }
    QSVChildRec* addChildRec(QWidget* w, int x, int y )
    {
	QSVChildRec *r = new QSVChildRec(w,x,y);
	children.append(r);
	childDict.insert(w, r);
	return r;
    }
    void deleteChildRec(QSVChildRec* r)
    {
	childDict.remove(r->child);
	children.removeRef(r);
	delete r;
    }
    void hideOrShowAll(QScrollView* sv)
    {
	if ( clipped_viewport ) {
	    if ( clipped_viewport->x() <= 0
	      && clipped_viewport->y() <= 0
	      && clipped_viewport->width()+clipped_viewport->x() >=
		    viewport.width()
	      && clipped_viewport->height()+clipped_viewport->y() >=
		    viewport.height() )
	    {
		// clipped_viewport still covers viewport
		return;
	    }
	    // Re-center
	    int nx = -(clipped_viewport->width()+viewport.width())/2;
	    int ny = -(clipped_viewport->height()+viewport.height())/2;
	    clipped_viewport->hide(); // while we mess with it
	    clipped_viewport->move(nx,ny);
	}
	for (QSVChildRec *r = children.first(); r; r=children.next()) {
	    r->hideOrShow(sv, clipped_viewport);
	}
	if ( clipped_viewport ) {
	    clipped_viewport->show();
	}
    }
    void moveAllBy(int dx, int dy)
    {
	if ( clipped_viewport ) {
	    clipped_viewport->move(
		clipped_viewport->x()+dx,
		clipped_viewport->y()+dy
	    );
	    //hideOrShowAll(sv);
	} else {
	    for (QSVChildRec *r = children.first(); r; r=children.next()) {
		r->child->move(r->child->x()+dx,r->child->y()+dy);
	    }
	}
    }
    void deleteAll()
    {
	for (QSVChildRec *r = children.first(); r; r=children.next()) {
	    delete r;
	}
    }
    bool anyVisibleChildren()
    {
	for (QSVChildRec *r = children.first(); r; r=children.next()) {
	    if (r->child->isVisible()) return TRUE;
	}
	return FALSE;
    }
    void autoResize(QScrollView* sv)
    {
	if ( policy == QScrollView::AutoOne ) {
	    QSVChildRec* r = children.first();
	    sv->resizeContents(r->child->width(),r->child->height());
	}
    }

    QScrollBar	hbar;
    QScrollBar	vbar;
    QWidget	viewport;
    QWidget*    clipped_viewport;
    QList<QSVChildRec>	children;
    QPtrDict<QSVChildRec>	childDict;
    QWidget*	corner;
    int		vx, vy, vwidth, vheight; // for drawContents-style usage
    int		l_marg, r_marg, t_marg, b_marg;
    QScrollView::ResizePolicy policy;
    QScrollView::ScrollBarMode	vMode;
    QScrollView::ScrollBarMode	hMode;

    // This variable allows ensureVisible to move the contents then
    // update both the sliders.  Otherwise, updating the sliders would
    // cause two image scrolls, creating ugly flashing.
    //
    bool signal_choke;
};

/*!
\class QScrollView qscrollview.h
\brief The QScrollView widget provides a scrolling area with on-demand scrollbars.

\ingroup abstractwidgets

The QScrollView is a large canvas - potentially larger than the
coordinate system normally supported by the underlying window system.
This is important, as is is quite easy to go beyond such limitations
(eg. many web pages are more than 32000 pixels high).  Additionally,
the QScrollView can have QWidgets positioned on it that scroll around
with the drawn content.  These subwidgets can also have positions
outside the normal coordinate range (but they are still limited in
size).

To provide content for the widget, inherit from QScrollView and
override drawContentsOffset(), and use resizeContents() to set the size
of the viewed area.  Use addChild() / moveChild() to position widgets
on the view.  For large numbers of such child widgets, consider using
packChildWidgets() to improve performance.

To use QScrollView effectively, it is important to understand its
widget structure in the three styles of usage: a single large child widget,
a large panning area with some widgets, a large panning area with many widgets.

<dl>
<dt><b>One Big Widget</b>
<dd>

<img src=qscrollview-vp2.png>

The first, simplest usage of QScrollView depicated above is
appropriate for scrolling areas
which are \e never more than about 4000 pixels in either dimension (this
is about the maximum reliable size on X11 servers).  In this usage, you
just make one large child in the QScrollView.  The child should
be a child of the viewport() of the scrollview, and be added with addChild():
\code
    QScrollView* sv = new QScrollView(...);
    QVBox* big_box = new QVBox(sv->viewport());
    sv->addChild(big_box);
\endcode
You may go on to add arbitrary child widgets to the single child in
the scrollview, as you would with any widget:
\code
    QLabel* child1 = new QLabel("CHILD", big_box);
    QLabel* child2 = new QLabel("CHILD", big_box);
    QLabel* child3 = new QLabel("CHILD", big_box);
    ...
\endcode
Here, the QScrollView has 4 children - the viewport(),
the verticalScrollBar(), the horizontalScrollBar(), and
a small cornerWidget().  The viewport() has 1 child, the big QVBox.
The QVBox has the three labels as child widgets.  When the view is scrolled,
the QVBox is moved, and its children move with it as child widgets normally
do.

<dt><b>Very Big View, some Widgets</b>
<dd>

<img src=qscrollview-vp.png>

The second usage of QScrollView depicated above is
appropriate when few, if any, widgets are on a very large scrolling area
that is potentially larger than 4000 pixels in either dimension. In this
usage, you call resizeContents() to set the size of the area, and override
drawContents() to paint the contents.  You may also add some widgets,
by making them children of the viewport() and adding them with
addChild() (this is the same as the process for the single large
widget in the previous example):
\code
    QScrollView* sv = new QScrollView(...);
    QLabel* child1 = new QLabel("CHILD", sv->viewport());
    sv->addChild(child1);
    QLabel* child2 = new QLabel("CHILD", sv->viewport());
    sv->addChild(child2);
    QLabel* child3 = new QLabel("CHILD", sv->viewport());
    sv->addChild(child3);
\endcode
Here, the QScrollView has the same 4 children - the viewport(),
the verticalScrollBar(), the horizontalScrollBar(), and
a small cornerWidget().  The viewport()
has the three labels as child widgets.  When the view is scrolled,
the scrollview moves the child widgets individually.

<dt><b>Very Big View, many Widgets</b>
<dd>

<img src=qscrollview-cl.png>

The final usage of QScrollView depicated above is
appropriate when many widgets are on a very large scrolling area
that is potentially larger than 4000 pixels in either dimension. In this
usage, you call resizeContents() to set the size of the area, and override
drawContents() to paint the contents.  You then call enableClipper(TRUE)
and add widgets, again
by making them children of the viewport() and adding them with
addChild():
\code
    QScrollView* sv = new QScrollView(...);
    sv->enableClipper(TRUE);
    QLabel* child1 = new QLabel("CHILD", sv->viewport());
    sv->addChild(child1);
    QLabel* child2 = new QLabel("CHILD", sv->viewport());
    sv->addChild(child2);
    QLabel* child3 = new QLabel("CHILD", sv->viewport());
    sv->addChild(child3);
\endcode

Here, the QScrollView has 4 children - the clipper() (\e not the
viewport() this time), the verticalScrollBar(), the
horizontalScrollBar(), and a small cornerWidget().  The clipper() has
1 child - the viewport().  The viewport() has the three labels as
child widgets.  When the view is scrolled, the viewport() is moved,
and its children move with it as child widgets normally do.

</dl>

Normally you will use the first or third method if you want any child
widgets in the view.

Note that the widget you see in the scrolled area is the viewport()
widget, not the QScrollView itself.  So, to turn mouse tracking on for
example, use viewport()->setMouseTracking(TRUE).

To enable drag-and-drop, you would setAcceptDrops(TRUE) on the
QScrollView (since drag-and-drop events propagate to the parent), but
to work out what logical position in the view, you would need to map
the drop co-ordinate from being relative to the QScrollView to being
relative to the contents - use the function mapToContents() for this.

To handle mouse events on the scrolling area, subclass scrollview as
you would subclass other widgets, but rather than overriding
mousePressEvent(), override viewportMousePressEvent() instead (if you
override mousePressEvent() you'll only get called when part of the
QScrollView is clicked - and the only such part is the "corner" (if
you don't set a cornerWidget()) and the frame, everything else being
covered up by the viewport, clipper, or scrollbars.

<img src=qscrollview-m.png> <img src=qscrollview-w.png>
*/


/*! \enum QScrollView::ResizePolicy

  This enum type is used to control QScrollView's reaction to resize
  events.  There are three possible settings:<ul>

  <li> \c Default - QScrollView selects one of the other settings
  automatically when it has to.  At the time of writing, QScrollView
  changs to \c Manual if you resize the contents with
  resizeContents(), and to \c AutoOne if a child is added.

  <li> \c Manual - the view stays the size set by resizeContents().

  <li> \c AutoOne - if there is only only child widget, the view stays
  the size of that widget.  Otherwise, the behaviour is undefined.

  <li> \c ResizeOne - if there is only one child, that child is
  resized to the size of the scroll view, but never smaller than its
  sizeHint().

  </ul>
*/

/*!
Constructs a QScrollView.

If you intend to add child widgets, you may see improved refresh
if you include \c WPaintClever in the widgets flags, \a f.  \c WPaintClever
as well as \c WNorthWestGravity is propagated to the viewport() widget.
*/

QScrollView::QScrollView( QWidget *parent, const char *name, WFlags f ) :
    QFrame( parent, name, f & ~WNorthWestGravity, FALSE )
{
    d = new QScrollViewData(this,WResizeNoErase|WRepaintNoErase| (f&WPaintClever) | (f&WNorthWestGravity));

    connect( &d->hbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( hslide( int ) ) );
    connect( &d->vbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( vslide( int ) ) );
    d->viewport.installEventFilter( this );

    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( style().defaultFrameWidth() );
}

/*! Destructs the QScrollView.  Any children added with addChild()
  will be destructed. */

QScrollView::~QScrollView()
{
    // Be careful not to get all those useless events...
    if ( d->clipped_viewport )
	d->clipped_viewport->removeEventFilter( this );
    else
	d->viewport.removeEventFilter( this );
    delete d;
}


void QScrollView::hslide( int pos )
{
    if ( !d->signal_choke ) {
	moveContents( -pos, -contentsY() );
	QApplication::syncX();
    }
}

void QScrollView::vslide( int pos )
{
    if ( !d->signal_choke ) {
	moveContents( -contentsX(), -pos );
	QApplication::syncX();
    }
}

/*!
  Called when the horizontal scrollbar geometry changes.  This is provided
  as a protected function so that subclasses can do interesting things
  like providing extra buttons in some of the space normally used by the
  scrollbars.

  The default implementation simply gives all the space to \a hbar.

  \sa setVBarGeometry()
*/
void QScrollView::setHBarGeometry(QScrollBar& hbar,
    int x, int y, int w, int h)
{
    hbar.setGeometry( x, y, w, h );
}

/*!
  Called when the vertical scrollbar geometry changes.  This is provided
  as a protected function so that subclasses can do interesting things
  like providing extra buttons in some of the space normally used by the
  scrollbars.

  The default implementation simply gives all the space to \a vbar.

  \sa setHBarGeometry()
*/
void QScrollView::setVBarGeometry( QScrollBar& vbar,
    int x, int y, int w, int h)
{
    vbar.setGeometry( x, y, w, h );
}


/*! Returns the viewport size for size (\a x, \a y).

  The viewport size depends on \a x,y (the size of the contents), the
  size of this widget, the modes of the horizontal and vertical scroll
  bars.

  This function permits widgets that can trade vertical and horizontal
  space for each other to control scroll bar appearance better.  For
  example, a word processor or web browser can control the width of
  the right margin accurately, whether there needs to be a vertical
  scroll bar or not.
*/


QSize QScrollView::viewportSize( int x, int y ) const
{
    int fw = frameWidth();
    int lmarg = fw+d->l_marg;
    int rmarg = fw+d->r_marg;
    int tmarg = fw+d->t_marg;
    int bmarg = fw+d->b_marg;

    int w = width();
    int h = height();

    bool needh, needv;
    bool showh, showv;

    if ( d->policy != AutoOne || d->anyVisibleChildren() ) {
	// Do we definitely need the scrollbar?
	needh = w-lmarg-rmarg < x;
	needv = h-tmarg-bmarg < y;

	// Do we intend to show the scrollbar?
	if (d->hMode == AlwaysOn)
	    showh = TRUE;
	else if (d->hMode == AlwaysOff)
	    showh = FALSE;
	else
	    showh = needh;

	if (d->vMode == AlwaysOn)
	    showv = TRUE;
	else if (d->vMode == AlwaysOff)
	    showv = FALSE;
	else
	    showv = needv;

	// Given other scrollbar will be shown, NOW do we need one?
	if ( showh && h-verticalScrollBar()->extent()-tmarg-bmarg < y ) {
	    if (d->vMode == Auto)
		showv=TRUE;
	}
	if ( showv && w-horizontalScrollBar()->extent()-lmarg-rmarg < x ) {
	    if (d->hMode == Auto)
		showh=TRUE;
	}
    } else {
	// Scrollbars not needed, only show scrollbar that are always on.
	showh = d->hMode == AlwaysOn;
	showv = d->vMode == AlwaysOn;
    }

    return QSize( w-lmarg-rmarg - (showv ? verticalScrollBar()->extent() : 0),
		  h-tmarg-bmarg - (showh ? horizontalScrollBar()->extent() : 0) );
}


/*!
  Updates scrollbars - all possibilities considered.  You should never
  need to call this in your code.
*/
void QScrollView::updateScrollBars()
{
    // I support this should use viewportSize()... but it needs
    // so many of the temporary variables from viewportSize.  hm.
    int fw = frameWidth();
    int lmarg = fw+d->l_marg;
    int rmarg = fw+d->r_marg;
    int tmarg = fw+d->t_marg;
    int bmarg = fw+d->b_marg;

    int w = width();
    int h = height();

    int portw, porth;

    bool needh;
    bool needv;
    bool showh;
    bool showv;

    if ( d->policy != AutoOne || d->anyVisibleChildren() ) {
	// Do we definitely need the scrollbar?
	needh = w-lmarg-rmarg < contentsWidth();
	needv = h-tmarg-bmarg < contentsHeight();

	// Do we intend to show the scrollbar?
	if (d->hMode == AlwaysOn)
	    showh = TRUE;
	else if (d->hMode == AlwaysOff)
	    showh = FALSE;
	else
	    showh = needh;

	if (d->vMode == AlwaysOn)
	    showv = TRUE;
	else if (d->vMode == AlwaysOff)
	    showv = FALSE;
	else
	    showv = needv;

	// Given other scrollbar will be shown, NOW do we need one?
	if ( showh && h-verticalScrollBar()->extent()-tmarg-bmarg < contentsHeight() ) {
	    needv=TRUE;
	    if (d->vMode == Auto)
		showv=TRUE;
	}
	if ( showv && w-horizontalScrollBar()->extent()-lmarg-rmarg < contentsWidth() ) {
	    needh=TRUE;
	    if (d->hMode == Auto)
		showh=TRUE;
	}
    } else {
	// Scrollbars not needed, only show scrollbar that are always on.
	needh = needv = FALSE;
	showh = d->hMode == AlwaysOn;
	showv = d->vMode == AlwaysOn;
    }

    // Hide unneeded scrollbar, calculate viewport size
    if ( showh ) {
        porth=h-horizontalScrollBar()->extent()-tmarg-bmarg;
    } else {
	if (!needh)
	    hslide( 0 ); // move to left
	d->hbar.hide();
	porth=h-tmarg-bmarg;
    }
    if ( showv ) {
	portw=w-verticalScrollBar()->extent()-lmarg-rmarg;
    } else {
	if (!needv)
	    vslide( 0 ); // move to top
	d->vbar.hide();
	portw=w-lmarg-rmarg;
    }

    // Configure scrollbars that we will show
    if ( showv ) {
	if ( needv ) {
	    d->vbar.setRange( 0, contentsHeight()-porth );
	    d->vbar.setSteps( QScrollView::d->vbar.lineStep(), porth );
	} else {
	    d->vbar.setRange( 0, 0 );
	}
    }
    if ( showh ) {
	if ( needh ) {
	    d->hbar.setRange( 0, contentsWidth()-portw );
	    d->hbar.setSteps( QScrollView::d->hbar.lineStep(), portw );
	} else {
	    d->hbar.setRange( 0, 0 );
	}
    }

    // Position the scrollbars, viewport, and corner widget.
    int bottom;
    if ( showh ) {
	int right = ( showv || cornerWidget() ) ? w-verticalScrollBar()->extent() : w;
	if ( style() == WindowsStyle )
            setHBarGeometry(d->hbar, fw, h-horizontalScrollBar()->extent()-fw,
                            right-fw-fw, horizontalScrollBar()->extent() );
	else
            setHBarGeometry(d->hbar, 0, h-horizontalScrollBar()->extent(), right,
                            horizontalScrollBar()->extent() );
	bottom=h-horizontalScrollBar()->extent();
    } else {
        bottom=h;
    }
    if ( showv ) {
	clipper()->setGeometry( lmarg, tmarg,
                                w-verticalScrollBar()->extent()-lmarg-rmarg,
                                bottom-tmarg-bmarg );
	if ( style() == WindowsStyle )
	    changeFrameRect(QRect(0, 0, w, h) );
	else
	    changeFrameRect(QRect(0, 0, w-verticalScrollBar()->extent(), bottom));
	if (cornerWidget()) {
	    if ( style() == WindowsStyle )
                setVBarGeometry( d->vbar, w-verticalScrollBar()->extent()-fw,
                                 fw, verticalScrollBar()->extent(),
                                 h-horizontalScrollBar()->extent()-fw-fw );
	    else
                setVBarGeometry( d->vbar, w-verticalScrollBar()->extent(), 0,
                                 verticalScrollBar()->extent(),
                                 h-horizontalScrollBar()->extent() );
	}
	else {
	    if ( style() == WindowsStyle )
                setVBarGeometry( d->vbar, w-verticalScrollBar()->extent()-fw,
                                 fw, verticalScrollBar()->extent(),
                                 bottom-fw-fw );
	    else
                setVBarGeometry( d->vbar, w-verticalScrollBar()->extent(), 0,
                                 verticalScrollBar()->extent(), bottom );
	}
    } else {
	if ( style() == WindowsStyle )
	    changeFrameRect(QRect(0, 0, w, h));
	else
	    changeFrameRect(QRect(0, 0, w, bottom));
	clipper()->setGeometry( lmarg, tmarg,
				 w-lmarg-rmarg, bottom-tmarg-bmarg );
    }
    if ( d->corner ) {
	if ( style() == WindowsStyle )
            d->corner->setGeometry( w-verticalScrollBar()->extent()-fw,
                                    h-horizontalScrollBar()->extent()-fw,
                                    verticalScrollBar()->extent(),
                                    horizontalScrollBar()->extent() );
	else
            d->corner->setGeometry( w-verticalScrollBar()->extent(),
                                    h-horizontalScrollBar()->extent(),
                                    verticalScrollBar()->extent(),
                                    horizontalScrollBar()->extent() );
    }

    if ( contentsX()+visibleWidth() > contentsWidth() ) {
	int x=QMAX(0,contentsWidth()-visibleWidth());
	d->hbar.setValue(x);
	// Do it even if it is recursive
	moveContents( -x, -contentsY() );
    }
    if ( contentsY()+visibleHeight() > contentsHeight() ) {
	int y=QMAX(0,contentsHeight()-visibleHeight());
	d->vbar.setValue(y);
	// Do it even if it is recursive
	moveContents( -contentsX(), -y );
    }

    // Finally, show the scrollbars.
    if ( showh )
	d->hbar.show();
    if ( showv )
	d->vbar.show();
}


/*!
An override - ensures scrollbars are correct size upon showing.
*/
void QScrollView::show()
{
    if (isVisible()) return;
    QWidget::show();
    updateScrollBars();
    d->hideOrShowAll(this);
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QScrollView::resize( int w, int h )
{
    QWidget::resize( w, h );
    return;
    
    /*
      ### This looks wrong to me, warwick. We shall discuss it. It makes
      writing really nicely behaving widgets like QTextView impossible. I do not 
      see any negative effects removing this code from a first glance.
      
      
    // Need both this and resize event, due to deferred resize event.
    bool u = isUpdatesEnabled();
    setUpdatesEnabled( FALSE );
    QWidget::resize( w, h );
    updateScrollBars();
    d->hideOrShowAll(this);
    setUpdatesEnabled( u );
    */
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QScrollView::resize( const QSize& s )
{
    resize(s.width(),s.height());
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QScrollView::resizeEvent( QResizeEvent* event )
{
    bool u = isUpdatesEnabled();
    setUpdatesEnabled( FALSE );
    QFrame::resizeEvent( event );
    
    // do _not_ update the scrollbars when updates have been
    // disabled. This makes it possible for subclasses to implement
    // dynamic wrapping without a horizontal scrollbar showing up all
    // the time when making a window smaller.
    if ( u )
	updateScrollBars();
    d->hideOrShowAll(this);
    setUpdatesEnabled( u );
}


/*!
An override - pass wheel events to the vertical scrollbar
*/
void QScrollView::wheelEvent( QWheelEvent *e ){
    if (verticalScrollBar())
	QApplication::sendEvent( verticalScrollBar(), e);
}

/*!
  Returns the currently set mode for the vertical scrollbar.

  \sa setVScrollBarMode()
*/
QScrollView::ScrollBarMode QScrollView::vScrollBarMode() const
{
    return d->vMode;
}


/*! \enum QScrollView::ScrollBarMode

  This enum type describes the various modes of QScrollView's scroll
  bars.  The defined modes are: <ul>

   <li> \c Auto - QScrollView shows a scrollbar when the content is
   too tall to fit and not else.  This is the default.

   <li> \c AlwaysOff - QScrollView never shows a scrollbar.

   <li> \c AlwaysOn - QScrollView always shows a scrollbar.

   </ul>

   (The modes for the horizontal and vertical scroll bars are independent.)
*/


/*!
  Sets the mode for the vertical scrollbar.

  \sa vScrollBarMode(), setHScrollBarMode()
*/
void  QScrollView::setVScrollBarMode( ScrollBarMode mode )
{
    if (d->vMode != mode) {
	d->vMode = mode;
	updateScrollBars();
    }
}


/*!
  Returns the currently set mode for the horizontal scrollbar.

  \sa setHScrollBarMode()
*/
QScrollView::ScrollBarMode QScrollView::hScrollBarMode() const
{
    return d->hMode;
}

/*!
  Sets the mode for the horizontal scrollbar.
  <ul>
   <li> \c Auto (the default) shows a scrollbar when the content is too wide to fit.
   <li> \c AlwaysOff never shows a scrollbar.
   <li> \c AlwaysOn always shows a scrollbar.
  </ul>

  \sa hScrollBarMode(), setVScrollBarMode()
*/
void  QScrollView::setHScrollBarMode( ScrollBarMode mode )
{
    if (d->hMode != mode) {
	d->hMode = mode;
	updateScrollBars();
    }
}


/*!
Returns the widget in the corner between the two scrollbars.

By default, no corner widget is present.
*/
QWidget* QScrollView::cornerWidget() const
{
    return d->corner;
}

/*!
  Sets the widget in the corner between the two scrollbars.

  You will probably also want to
  set at least one of the scrollbar modes to AlwaysOn.

  Passing 0 shows no widget in the corner.

  Any previous corner widget is hidden.

  You may call setCornerWidget() with the same widget at different times.

  All widgets set here will be deleted by the QScrollView when it destructs
  unless you separately
  reparent the widget after setting some other corner widget (or 0).

  Any \e newly set widget should have no current parent.

  By default, no corner widget is present.

  \sa setVScrollBarMode(), setHScrollBarMode()
*/
void QScrollView::setCornerWidget(QWidget* corner)
{
    QWidget* oldcorner = d->corner;
    if (oldcorner != corner) {
	if (oldcorner) oldcorner->hide();
	d->corner = corner;

	if ( corner && corner->parentWidget() != this ) {
	    // #### No clean way to get current WFlags
	    corner->reparent( this, (((QScrollView*)corner))->getWFlags(),
			      QPoint(0,0), FALSE );
	}

	updateScrollBars();
	if ( corner ) corner->show();
    }
}


/*!
  Sets the resize policy to \a r.

  \sa resizePolicy() ResizePolicy
*/
void QScrollView::setResizePolicy( ResizePolicy r )
{
    d->policy = r;
}

/*!
  Returns the currently set ResizePolicy.

  \sa setResizePolicy() ResizePolicy
*/
QScrollView::ResizePolicy QScrollView::resizePolicy() const
{
    return d->policy;
}


/*!
  Removes a child from the scrolled area.  Note that this happens
  automatically if the child is deleted.
*/
void QScrollView::removeChild(QWidget* child)
{
    if ( !d ) // In case we are destructing
	return;

    QSVChildRec *r = d->rec(child);
    if ( r ) d->deleteChildRec( r );
}

/*!
  Just an override to keep QObject::removeChild() accessible.
*/
void QScrollView::removeChild(QObject* child)
{
    QFrame::removeChild(child);
}

/*!
  Inserts \a child into the scrolled area positioned at (\a x, \a y).
  The position defaults to (0,0). If the child is already in the view,
  it is just moved.

  You may want to call packChildWidgets() after a large number of
  such additions is complete.
*/
void QScrollView::addChild(QWidget* child, int x, int y)
{
    if ( child->parentWidget() == viewport() ) {
	// May already be there
	QSVChildRec *r = d->rec(child);
	if (r) {
	    r->moveTo(this,x,y,d->clipped_viewport);
	    if ( d->policy > Manual ) {
		d->autoResize(this); // #### better to just deal with this one widget!
	    }
	    return;
	}
    }

    if ( d->children.isEmpty() && d->policy == Default ) {
	setResizePolicy( AutoOne );
	child->installEventFilter( this );
    } else if ( d->policy == AutoOne ) {
	child->removeEventFilter( this );
    }
    if ( child->parentWidget() != viewport() ) {
	child->reparent( viewport(), 0, QPoint(0,0), FALSE );
    }
    d->addChildRec(child,x,y)->hideOrShow(this, d->clipped_viewport);

    if ( d->policy > Manual ) {
	d->autoResize(this); // #### better to just deal with this one widget!
    }
}

/*!
  Repositions \a child to (\a x, \a y).
  This functions the same as addChild().
*/
void QScrollView::moveChild(QWidget* child, int x, int y)
{
    addChild(child,x,y);
}

/*!
  Returns the X position of the given child widget.
  Use this rather than QWidget::x() for widgets added to the view.
*/
int QScrollView::childX(QWidget* child)
{
    return d->rec(child)->x;
}

/*!
  Returns the Y position of the given child widget.
  Use this rather than QWidget::y() for widgets added to the view.
*/
int QScrollView::childY(QWidget* child)
{
    return d->rec(child)->y;
}

/*!
  \obsolete

  Returns TRUE if \a child is visible.  This is equivalent
  to child->isVisible().
*/
bool QScrollView::childIsVisible(QWidget* child)
{
    return child->isVisible();
}

/*!
  \obsolete

  Sets the visibility of \a child. Equivalent to
  QWidget::show() or QWidget::hide().
*/
void QScrollView::showChild(QWidget* child, bool y)
{
    if ( y )
	child->show();
    else
	child->hide();
}


/*!
  This event filter ensures the scrollbars are updated when a single
  contents widget is resized, shown, hidden, or destroyed, and passes
  mouse events to the QScrollView.
*/

bool QScrollView::eventFilter( QObject *obj, QEvent *e )
{
    if (!d) return FALSE; // we are destructing
    if ( obj == &d->viewport || obj == d->clipped_viewport ) {
	switch ( e->type() ) {

	/* Forward many events to viewport...() functions */
	case QEvent::Paint:
	    viewportPaintEvent( (QPaintEvent*)e );
	    break;
	case QEvent::Resize:
	    viewportResizeEvent( (QResizeEvent*)e );
	    break;
	case QEvent::MouseButtonPress:
	    viewportMousePressEvent( (QMouseEvent*)e );
	    break;
	case QEvent::MouseButtonRelease:
	    viewportMouseReleaseEvent( (QMouseEvent*)e );
	    break;
	case QEvent::MouseButtonDblClick:
	    viewportMouseDoubleClickEvent( (QMouseEvent*)e );
	    break;
	case QEvent::MouseMove:
	    viewportMouseMoveEvent( (QMouseEvent*)e );
	    break;
	case QEvent::DragEnter:
	    viewportDragEnterEvent( (QDragEnterEvent*)e );
	    break;
	case QEvent::DragMove:
	    viewportDragMoveEvent( (QDragMoveEvent*)e );
	    break;
	case QEvent::DragLeave:
	    viewportDragLeaveEvent( (QDragLeaveEvent*)e );
	    break;
	case QEvent::Drop:
	    viewportDropEvent( (QDropEvent*)e );
	    break;
	case QEvent::Wheel:
	    viewportWheelEvent( (QWheelEvent*)e );
	    break;


	case QEvent::ChildRemoved:
	    removeChild((QWidget*)((QChildEvent*)e)->child());
	    break;
	default:
	    break;
	}
    } else {
	// must be a child
	QSVChildRec* r = d->rec((QWidget*)obj);
	if (!r) return FALSE; // spurious
	if ( e->type() == QEvent::Resize )
	    d->autoResize(this);
    }
    return FALSE;  // always continue with standard event processing
}

/*!
  This event handler is called whenever the QScrollView receives a
  mousePressEvent() - the press position is translated to be a
  point on the contents.
*/
void QScrollView::contentsMousePressEvent( QMouseEvent* )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  mouseReleaseEvent() - the release position is translated to be a
  point on the contents.
*/
void QScrollView::contentsMouseReleaseEvent( QMouseEvent* )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  mouseDoubleClickEvent() - the click position is translated to be a
  point on the contents.
*/
void QScrollView::contentsMouseDoubleClickEvent( QMouseEvent* )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  mouseMoveEvent() - the mouse position is translated to be a
  point on the contents.
*/
void QScrollView::contentsMouseMoveEvent( QMouseEvent* )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  dragEnterEvent() - the drag position is translated to be a
  point on the contents.
*/
void QScrollView::contentsDragEnterEvent( QDragEnterEvent * )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  dragMoveEvent() - the drag position is translated to be a
  point on the contents.
*/
void QScrollView::contentsDragMoveEvent( QDragMoveEvent * )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  dragLeaveEvent() - the drag position is translated to be a
  point on the contents.
*/
void QScrollView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  dropEvent() - the drop position is translated to be a
  point on the contents.
*/
void QScrollView::contentsDropEvent( QDropEvent * )
{
}

/*!
  This event handler is called whenever the QScrollView receives a
  wheelEvent() - the mouse position is translated to be a
  point on the contents.
*/
void QScrollView::contentsWheelEvent( QWheelEvent * )
{
}


/*!
  This is a low-level painting routine that draws the viewport
  contents.  Override this if drawContentsOffset() is too high-level.
  (for example, if you don't want to open a QPainter on the viewport).
*/
void QScrollView::viewportPaintEvent( QPaintEvent* pe )
{
    QWidget* vp = viewport();
    QPainter p(vp);
    QRect r = pe->rect();
    if ( d->clipped_viewport ) {
	QRect rr(
	    -d->clipped_viewport->x(), -d->clipped_viewport->y(),
	    d->viewport.width(), d->viewport.height()
	);
	r &= rr;
	int ex = r.x() + d->clipped_viewport->x() + contentsX();
	int ey = r.y() + d->clipped_viewport->y() + contentsY();
	int ew = r.width();
	int eh = r.height();
	drawContentsOffset(&p,
	    contentsX()+d->clipped_viewport->x(),
	    contentsY()+d->clipped_viewport->y(),
	    ex, ey, ew, eh);
    } else {
	r &= d->viewport.rect();
	int ex = r.x() + contentsX();
	int ey = r.y() + contentsY();
	int ew = r.width();
	int eh = r.height();
	drawContentsOffset(&p, contentsX(), contentsY(), ex, ey, ew, eh);
    }
}


/*!
  To provide simple processing of events on the contents, this method
  receives all resize events sent to the viewport.

  \sa QWidget::resizeEvent()
*/
void QScrollView::viewportResizeEvent( QResizeEvent* )
{
}

/*!
  To provide simple processing of events on the contents, this method receives all mouse
  press events sent to the viewport.

  The default implementation translates the event and calls
  contentsMousePressEvent().

  \sa contentsMousePressEvent(), QWidget::mousePressEvent()
*/
void QScrollView::viewportMousePressEvent( QMouseEvent* e )
{
    QMouseEvent ce(e->type(), viewportToContents(e->pos()),
	e->globalPos(), e->button(), e->state());
    contentsMousePressEvent(&ce);
}

/*!
  To provide simple processing of events on the contents,
  this method receives all mouse
  release events sent to the viewport.

  The default implementation translates the event and calls
  contentsMouseReleaseEvent().

  \sa QWidget::mouseReleaseEvent()
*/
void QScrollView::viewportMouseReleaseEvent( QMouseEvent* e )
{
    QMouseEvent ce(e->type(), viewportToContents(e->pos()),
	e->globalPos(), e->button(), e->state());
    contentsMouseReleaseEvent(&ce);
}

/*!
  To provide simple processing of events on the contents,
  this method receives all mouse
  double click events sent to the viewport.

  The default implementation translates the event and calls
  contentsMouseDoubleClickEvent().

  \sa QWidget::mouseDoubleClickEvent()
*/
void QScrollView::viewportMouseDoubleClickEvent( QMouseEvent* e )
{
    QMouseEvent ce(e->type(), viewportToContents(e->pos()),
	e->globalPos(), e->button(), e->state());
    contentsMouseDoubleClickEvent(&ce);
}

/*!
  To provide simple processing of events on the contents,
  this method receives all mouse
  move events sent to the viewport.

  The default implementation translates the event and calls
  contentsMouseMoveEvent().

  \sa QWidget::mouseMoveEvent()
*/
void QScrollView::viewportMouseMoveEvent( QMouseEvent* e )
{
    QMouseEvent ce(e->type(), viewportToContents(e->pos()),
	e->globalPos(), e->button(), e->state());
    contentsMouseMoveEvent(&ce);
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drag enter
  events sent to the viewport.

  The default implementation translates the event and calls
  contentsDragEnterEvent().

  \sa QWidget::dragEnterEvent()
*/
void QScrollView::viewportDragEnterEvent( QDragEnterEvent* e )
{
    e->setPoint(viewportToContents(e->pos()));
    contentsDragEnterEvent(e);
    e->setPoint(contentsToViewport(e->pos()));
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drag move
  events sent to the viewport.

  The default implementation translates the event and calls
  contentsDragMoveEvent().

  \sa QWidget::dragMoveEvent()
*/
void QScrollView::viewportDragMoveEvent( QDragMoveEvent* e )
{
    e->setPoint(viewportToContents(e->pos()));
    contentsDragMoveEvent(e);
    e->setPoint(contentsToViewport(e->pos()));
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drag leave
  events sent to the viewport.

  The default implementation calls contentsDragLeaveEvent().

  \sa QWidget::dragLeaveEvent()
*/
void QScrollView::viewportDragLeaveEvent( QDragLeaveEvent* e )
{
    contentsDragLeaveEvent(e);
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drop
  events sent to the viewport.

  The default implementation translates the event and calls
  contentsDropEvent().

  \sa QWidget::dropEvent()
*/
void QScrollView::viewportDropEvent( QDropEvent* e )
{
    e->setPoint(viewportToContents(e->pos()));
    contentsDropEvent(e);
    e->setPoint(contentsToViewport(e->pos()));
}

/*!
  To provide simple processing of events on the contents,
  this method receives all wheel
  events sent to the viewport.

  The default implementation translates the event and calls
  contentsWheelEvent().

  \sa QWidget::wheelEvent()
*/
void QScrollView::viewportWheelEvent( QWheelEvent* e )
{
    QWheelEvent ce( viewportToContents(e->pos()),
	e->globalPos(), e->delta(), e->state());
    contentsWheelEvent(&ce);
    if ( ce.isAccepted() )
	e->accept();
    else
	e->ignore();
}

/*!
 Returns the component horizontal scrollbar.  It is made available to allow
 accelerators, autoscrolling, etc., and to allow changing
 of arrow scroll rates: bar->setSteps( rate, bar->pageStep() ).

 It should not be otherwise manipulated.

 This function never returns 0.
*/
QScrollBar* QScrollView::horizontalScrollBar() const
{
    return &d->hbar;
}

/*!
 Returns the component vertical scrollbar.  It is made available to allow
 accelerators, autoscrolling, etc., and to allow changing
 of arrow scroll rates: bar->setSteps( rate, bar->pageStep() ).

 It should not be otherwise manipulated.

 This function never returns 0.
*/
QScrollBar* QScrollView::verticalScrollBar() const {
    return &d->vbar;
}


/*!
 Scrolls the content so that the point (x, y) is visible
 with at least 50-pixel margins (if possible, otherwise centered).
*/
void QScrollView::ensureVisible( int x, int y )
{
    ensureVisible(x, y, 50, 50);
}

/*!
 Scrolls the content so that the point (x, y) is visible
 with at least the given pixel margins (if possible, otherwise centered).
*/
void QScrollView::ensureVisible( int x, int y, int xmargin, int ymargin )
{
    int pw=visibleWidth();
    int ph=visibleHeight();

    int cx=-contentsX();
    int cy=-contentsY();
    int cw=contentsWidth();
    int ch=contentsHeight();

    if ( pw < xmargin*2 )
	xmargin=pw/2;
    if ( ph < ymargin*2 )
	ymargin=ph/2;

    if ( cw <= pw ) {
	xmargin=0;
	cx=0;
    }
    if ( ch <= ph ) {
	ymargin=0;
	cy=0;
    }

#ifdef QT_1x_SEMANTICS
    if ( x < -cx+xmargin )
	cx = -x+pw-xmargin;
    else if ( x >= -cx+pw-xmargin )
	cx = -x+xmargin;

    if ( y < -cy+ymargin )
	cy = -y+ph-ymargin;
    else if ( y >= -cy+ph-ymargin )
	cy = -y+ymargin;
#else
    if ( x < -cx+xmargin )
	cx = -x+xmargin;
    else if ( x >= -cx+pw-xmargin )
	cx = -x+pw-xmargin;

    if ( y < -cy+ymargin )
	cy = -y+ymargin;
    else if ( y >= -cy+ph-ymargin )
	cy = -y+ph-ymargin;
#endif

    if ( cx > 0 )
	cx=0;
    else if ( cx < pw-cw && cw>pw )
	cx=pw-cw;

    if ( cy > 0 )
	cy=0;
    else if ( cy < ph-ch && ch>ph )
	cy=ph-ch;

    setContentsPos( -cx, -cy );
}

/*!
 Scrolls the content so that the point (x, y) is in the top-left corner.
*/
void QScrollView::setContentsPos( int x, int y )
{
    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    // Choke signal handling while we update BOTH sliders.
    d->signal_choke=TRUE;
    moveContents( -x, -y );
    d->vbar.setValue( y );
    d->hbar.setValue( x );
//     updateScrollBars(); // ### warwick, why should we need that???
    d->signal_choke=FALSE;
//     updateScrollBars(); // ### warwick, why should we need that???
}

/*!
 Scrolls the content by \a x to the left and \a y upwards.
*/
void QScrollView::scrollBy( int dx, int dy )
{
    setContentsPos( contentsX()+dx, contentsY()+dy );
}

/*!
 Scrolls the content so that the point (x,y) is in the
 center of visible area.
*/
void QScrollView::center( int x, int y )
{
    ensureVisible( x, y, 32000, 32000 );
}

/*!
 Scrolls the content so that the point (x,y) is visible,
 with the given margins (as fractions of visible area).

 eg.
 <ul>
   <li>Margin 0.0 allows (x,y) to be on edge of visible area.
   <li>Margin 0.5 ensures (x,y) is in middle 50% of visible area.
   <li>Margin 1.0 ensures (x,y) is in the center of the visible area.
 </ul>
*/
void QScrollView::center( int x, int y, float xmargin, float ymargin )
{
    int pw=visibleWidth();
    int ph=visibleHeight();
    ensureVisible( x, y, int( xmargin/2.0*pw+0.5 ), int( ymargin/2.0*ph+0.5 ) );
}


/*!
  \fn void QScrollView::contentsMoving(int x, int y)

  This signal is emitted just before the contents is moved
  to the given position.

  \sa contentsX(), contentsY()
*/

/*!
  Moves the contents.
*/
void QScrollView::moveContents(int x, int y)
{
    if ( -x+visibleWidth() > contentsWidth() )
	x=QMIN(0,-contentsWidth()+visibleWidth());
    if ( -y+visibleHeight() > contentsHeight() )
	y=QMIN(0,-contentsHeight()+visibleHeight());

    int dx = x - d->vx;
    int dy = y - d->vy;

    if (!dx && !dy)
	return; // Nothing to do

    d->vx = x;
    d->vy = y;

    emit contentsMoving( -x, -y );

    if ( d->clipped_viewport ) {
	// Cheap move (usually)
	d->moveAllBy(dx,dy);
    } else if ( /*dx && dy ||*/
	 ( QABS(dy) * 5 > visibleHeight() * 4 ) ||
	 ( QABS(dx) * 5 > visibleWidth() * 4 )
	)
    {
	// Big move
	QPaintEvent* pe = new QPaintEvent( viewport()->rect(),
				   !viewport()->testWFlags( WRepaintNoErase ) );
	QApplication::postEvent( viewport(), pe );
	d->moveAllBy(dx,dy);
    } else {
	// Small move
	clipper()->scroll(dx,dy);
    }
    d->hideOrShowAll(this);
}

/*!
  Returns the X coordinate of the contents which is at the left
  edge of the viewport.
*/
int QScrollView::contentsX() const
{
    return -d->vx;
}

/*!
  Returns the Y coordinate of the contents which is at the top
  edge of the viewport.
*/
int QScrollView::contentsY() const
{
    return -d->vy;
}

/*!
  Returns the width of the contents area.
*/
int QScrollView::contentsWidth() const
{
    return d->vwidth;
}

/*!
  Returns the height of the contents area.
*/
int QScrollView::contentsHeight() const
{
    return d->vheight;
}

/*!
  Set the size of the contents area to \a w pixels wide and \a h
  pixels high, and updates the viewport accordingly.
*/
void QScrollView::resizeContents( int w, int h )
{
    int ow = d->vwidth;
    int oh = d->vheight;
    d->vwidth = w;
    d->vheight = h;

    // Could more efficiently scroll if shrinking, repaint if growing, etc.
    updateScrollBars();

    if ( d->children.isEmpty() && d->policy == Default )
	setResizePolicy( Manual );

    if ( ow > w ) {
	// Swap
	int t=w;
	w=ow;
	ow=t;
    }
    // Refresh area ow..w
    if ( ow < visibleWidth() && w >= 0 ) {
	if ( ow < 0 )
	    ow = 0;
	if ( w > visibleWidth() )
	    w = visibleWidth();
	clipper()->update( contentsX()+ow, 0, w-ow, visibleHeight() );
    }

    if ( oh > h ) {
	// Swap
	int t=h;
	h=oh;
	oh=t;
    }
    // Refresh area oh..h
    if ( oh < visibleHeight() && h >= 0 ) {
	if ( oh < 0 )
	    oh = 0;
	if ( h > visibleHeight() )
	    h = visibleHeight();
	clipper()->update( 0, contentsY()+oh, visibleWidth(), h-oh);
    }
}

/*!
  Calls update() on rectangle defined by \a x, \a y, \a w, \a h,
  translated appropriately.  If the rectangle in not visible,
  nothing is repainted.

  \sa repaintContents()
*/
void QScrollView::updateContents( int x, int y, int w, int h )
{
    QWidget* vp = viewport();

    // Translate
    x -= contentsX();
    y -= contentsY();

    // Clip to QCOORD space
    if ( x < 0 ) {
	w += x;
	x = 0;
    }
    if ( y < 0 ) {
	h += y;
	y = 0;
    }

    if ( w < 0 || h < 0 )
	return;
    if ( w > visibleWidth() )
	w = visibleWidth();
    if ( h > visibleHeight() )
	h = visibleHeight();

    if ( d->clipped_viewport ) {
	// Translate clipper() to viewport()
	x -= d->clipped_viewport->x();
	y -= d->clipped_viewport->y();
    }

    vp->update( x, y, w, h );
}

/*!
  Calls repaint() on rectangle defined by \a x, \a y, \a w, \a h,
  translated appropriately.  If the rectangle in not visible,
  nothing is repainted.

  \sa updateContents()
*/
void QScrollView::repaintContents( int x, int y, int w, int h, bool erase )
{
    QWidget* vp = viewport();

    // Translate logical to clipper()
    x -= contentsX();
    y -= contentsY();

    // Clip to QCOORD space
    if ( x < 0 ) {
	w += x;
	x = 0;
    }
    if ( y < 0 ) {
	h += y;
	y = 0;
    }

    if ( w < 0 || h < 0 )
	return;
    if ( w > visibleWidth() )
	w = visibleWidth();
    if ( h > visibleHeight() )
	h = visibleHeight();

    if ( d->clipped_viewport ) {
	// Translate clipper() to viewport()
	x += d->clipped_viewport->x();
	y += d->clipped_viewport->y();
    }

    vp->repaint( x, y, w, h, erase );
}


/*!
  For backward compatibility only.
  It is easier to use drawContents(QPainter*,int,int,int,int).

  The default implementation translates the painter appropriately
  and calls drawContents(QPainter*,int,int,int,int).
*/
void QScrollView::drawContentsOffset(QPainter* p, int offsetx, int offsety, int clipx, int clipy, int clipw, int cliph)
{
    p->translate(-offsetx,-offsety);
    drawContents(p, clipx, clipy, clipw, cliph);
}

/*!
  \fn void QScrollView::drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph)

  Reimplement this method if you are viewing a drawing area rather
  than a widget.

  The function should draw the rectangle (\a clipx, \a clipy, \a clipw, \a
  cliph ) of the contents, using painter \a p.  The clip rectangle is
  in the scroll views's coordinates.

  For example:
  \code
  {
    // Fill a 40000 by 50000 rectangle at (100000,150000)

    // Calculate the coordinates... (don't use QPoint, QRect, etc!)
    int x1 = 100000, y1 = 150000;
    int x2 = x1+40000-1, y2 = y1+50000-1;

    // Clip the coordinates so X/Windows will not have problems...
    if (x1 < clipx) x1=clipx;
    if (y1 < clipy) y1=clipy;
    if (x2 > clipx+clipw-1) x2=clipx+clipw-1;
    if (y2 > clipy+cliph-1) y2=clipy+cliph-1;

    // Paint using the small coordinates...
    if ( x2 >= x1 && y2 >= y1 )
	p->fillRect(x1, y1, x2-x1+1, y2-y1+1, red);
  }
  \endcode

  The clip rectangle and translation of the painter \a p is already set
  appropriately.
*/
void QScrollView::drawContents(QPainter*, int, int, int, int)
{
}

/*!
An override - ensures scrollbars are correct size when frame style changes.
*/
void QScrollView::frameChanged()
{
    updateScrollBars();
}


/*!
  Returns the viewport widget of the scrollview.  This is the widget
  containing the contents widget or which is the drawing area.
*/
QWidget* QScrollView::viewport() const
{
    return d->clipped_viewport ? d->clipped_viewport : &d->viewport;
}

/*!
  Returns the clipper widget.
  Contents in the scrollview is ultimately clipped to be inside
  the clipper widget.

  You should not need to access this.

  \sa visibleWidth(), visibleHeight()
*/
QWidget* QScrollView::clipper() const
{
    return &d->viewport;
}

/*!
  Returns the horizontal amount of the content that is visible.
*/
int QScrollView::visibleWidth() const
{
    return clipper()->width();
}

/*!
  Returns the vertical amount of the content that is visible.
*/
int QScrollView::visibleHeight() const
{
    return clipper()->height();
}


void QScrollView::changeFrameRect(const QRect& r)
{
    QRect oldr = frameRect();
    if (oldr != r) {
	setFrameRect(r);
    }
}


/*!
  Sets the margins around the scrolling area.  This is useful for applications
  such as spreadsheets with `locked' rows and columns.  The marginal space
  is \e inside the frameRect() and is left blank - override drawContents()
  or put widgets in the unused area.

  By default all margins are zero.

  \sa frameChanged()
*/
void QScrollView::setMargins(int left, int top, int right, int bottom)
{
    if ( left == d->l_marg &&
	 top == d->t_marg &&
	 right == d->r_marg &&
	 bottom == d->b_marg )
	return;
	
    d->l_marg = left;
    d->t_marg = top;
    d->r_marg = right;
    d->b_marg = bottom;
    updateScrollBars();
}


/*!
  Returns the current left margin.
  \sa setMargins()
*/
int QScrollView::leftMargin() const	
{
    return d->l_marg;
}


/*!
  Returns the current top margin.
  \sa setMargins()
*/
int QScrollView::topMargin() const	
{
    return d->t_marg;
}


/*!
  Returns the current right margin.
  \sa setMargins()
*/
int QScrollView::rightMargin() const	
{
    return d->r_marg;
}


/*!
  Returns the current bottom margin.
  \sa setMargins()
*/
int QScrollView::bottomMargin() const	
{
    return d->b_marg;
}

/*!
  Override so that traversal moves among child widgets, even if they
  are not visible, scrolling to make them so.
*/
bool QScrollView::focusNextPrevChild( bool next )
{
    // first set things up for the scan
    QFocusData *f = focusData();
    QWidget *startingPoint = f->home();
    QWidget *candidate = 0;
    QWidget *w = next ? f->next() : f->prev();
    QSVChildRec *r;

    // then scan for a possible focus widget candidate
    while( !candidate && w != startingPoint ) {
	if ( w != startingPoint &&
	     (w->focusPolicy() & TabFocus) == TabFocus
	     && w->isEnabled() &&!w->focusProxy() && w->isVisible() )
	    candidate = w;
	w = next ? f->next() : f->prev();
    }

    // if we could not find one, maybe super or parentWidget() can?
    if ( !candidate )
	return QFrame::focusNextPrevChild( next );

    // we've found one.
    r = d->ancestorRec( candidate );
    if ( r && ( r->child == candidate ||
		candidate->isVisibleTo( r->child ) ) ) {
	QPoint cp = r->child->mapToGlobal(QPoint(0,0));
	QPoint cr = candidate->mapToGlobal(QPoint(0,0)) - cp;
	ensureVisible( r->x+cr.x()+candidate->width()/2,
		       r->y+cr.y()+candidate->height()/2,
		       candidate->width()/2,
		       candidate->height()/2 );
    }

    candidate->setFocus();
    return TRUE;
}



/*!
  When large numbers of child widgets are in a scrollview, especially
  if they are close together, the scrolling performance can suffer
  greatly.  If you call enableClipper(TRUE), the scrollview will
  use an extra widget to group child widgets.

  Note that you may only call enableClipper() prior to adding widgets.

  For a full discussion, see the overview documentation of this
  class.
*/
void QScrollView::enableClipper(bool y)
{
    if ( !d->clipped_viewport == !y )
	return;
    if ( d->children.count() )
	qFatal("May only call QScrollView::enableClipper() before adding widgets");
    if ( y ) {
	d->clipped_viewport = new QWidget(clipper());
	d->clipped_viewport->setGeometry(-coord_limit/2,-coord_limit/2,
					coord_limit,coord_limit);
	d->viewport.setBackgroundMode(NoBackground); // no exposures for this
	d->viewport.removeEventFilter( this );
	d->clipped_viewport->installEventFilter( this );
    } else {
	delete d->clipped_viewport;
	d->clipped_viewport = 0;
    }
}

/*!
  Returns the
    point \a p
  translated to
    a point on the viewport() widget.
*/
QPoint QScrollView::contentsToViewport(const QPoint& p)
{
    if ( d->clipped_viewport ) {
	return QPoint( p.x() - contentsX() - d->clipped_viewport->x(),
		       p.y() - contentsY() - d->clipped_viewport->y() );
    } else {
	return QPoint( p.x() - contentsX(),
		       p.y() - contentsY() );
    }
}

/*!
  Returns the
    point on the viewport \a vp
  translated to
    a point in the contents.
*/
QPoint QScrollView::viewportToContents(const QPoint& vp)
{
    if ( d->clipped_viewport ) {
	return QPoint( vp.x() + contentsX() + d->clipped_viewport->x(),
		       vp.y() + contentsY() + d->clipped_viewport->y() );
    } else {
	return QPoint( vp.x() + contentsX(),
		       vp.y() + contentsY() );
    }
}


/*!
  Translates
    a point (\a x, \a y) in the contents
  to
    a point (\a vx, \a vy) on the viewport() widget.
*/
void QScrollView::contentsToViewport(int x, int y, int& vx, int& vy)
{
    const QPoint v = contentsToViewport(QPoint(x,y));
    vx = v.x();
    vy = v.y();
}

/*!
  Translates
    a point (\a vx, \a vy) on the viewport() widget
  to
    a point (\a x, \a y) in the contents.
*/
void QScrollView::viewportToContents(int vx, int vy, int& x, int& y)
{
    const QPoint c = viewportToContents(QPoint(vx,vy));
    x = c.x();
    y = c.y();
}


/*!
  Specifies that this widget can use additional space, and that it can
  survive on less than sizeHint().
*/
QSizePolicy QScrollView::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


/*!
  \reimp
*/
QSize	QScrollView::minimumSizeHint() const
{
    //so we can implement it later
    return QWidget::minimumSizeHint();
}


/*!
  \reimp
*/

void QScrollView::drawContents( QPainter * )
{
    //implemented to get rid of a compiler warning.
}
