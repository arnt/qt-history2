/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollview.cpp#83 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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

const int sbDim = 16;

struct QSVChildRec {
    QSVChildRec(QWidget* c, int xx, int yy) :
	child(c),
	x(xx), y(yy)
    {
    }

    void moveBy(QScrollView* sv, int dx, int dy)
    {
	moveTo( sv, x+dx, y+dy );
    }
    void moveTo(QScrollView* sv, int xx, int yy)
    {
	if ( x != xx || y != yy ) {
	    x = xx;
	    y = yy;
	    hideOrShow(sv);
	}
    }
    void hideOrShow(QScrollView* sv)
    {
	if ( x-sv->contentsX() < -child->width()
	  || x-sv->contentsX() > sv->viewport()->width()
	  || y-sv->contentsY() < -child->height()
	  || y-sv->contentsY() > sv->viewport()->height() )
	{
	    child->move(sv->viewport()->width()+10000,
			sv->viewport()->height()+10000);
	} else {
	    child->move(x-sv->contentsX(), y-sv->contentsY());
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
    }
    ~QScrollViewData()
    {
	deleteAll();
    }

    QSVChildRec* rec(QWidget* w) { return childDict.find(w); }
    QSVChildRec* ancestorRec(QWidget* w)
    {
	while (w->parentWidget() != &viewport) {
	    w = w->parentWidget();
	    if (!w) return 0;
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
	for (QSVChildRec *r = children.first(); r; r=children.next()) {
	    r->hideOrShow(sv);
	}
    }
    void moveAllBy(int dx, int dy)
    {
	for (QSVChildRec *r = children.first(); r; r=children.next()) {
	    r->child->move(r->child->x()+dx,r->child->y()+dy);
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
    QList<QSVChildRec>	children;
    QPtrDict<QSVChildRec>	childDict;
    QWidget*	corner;
    int		vx, vy, vwidth, vheight; // for drawContents-style usage
    int		l_marg, r_marg, t_marg, b_marg;
    QScrollView::ResizePolicy policy;
    QScrollView::ScrollBarMode	vMode;
    QScrollView::ScrollBarMode	hMode;
};

/*!
\class QScrollView qscrollview.h
\brief The QScrollView widget provides a scrolling area with on-demand scrollbars.

The QScrollView is a large canvas - potentially larger than the coordinate
system normally supported by the underlying window system.  This is important,
as is is quite easy to go beyond such limitations (eg. many web pages are
more than 32000 pixels high).  Additionally, the QScrollView can have
QWidgets positioned on it that scroll around with the drawn content.  These
subwidgets can also have positions outside the normal coordinate range
(but they are still limited in size).

To provide content for the widget, inherit from QScrollView and
override drawContentsOffset(), and use resizeContents() to set
the size of the viewed area.  Use addChild() / moveChild()
to position widgets on the view.

Note that the scrolled area is the viewport() widget, not the QScrollView
itself.  So, to turn mouse tracking on for example, use
viewport()->setMouseTracking(TRUE).  The only part of the QScrollView that
is visible is the "corner" and the frame.

Note also the effect of resizePolicy().

<img src=qscrollview-m.gif> <img src=qscrollview-w.gif>
*/


/*!
Constructs a QScrollView.

If you intend to add child widgets, you may see improved refresh
if you include WPaintClever in the widgets flags, \a f.  WPaintClever
will be propagated to the viewport() widget.
*/

QScrollView::QScrollView( QWidget *parent, const char *name, WFlags f ) :
    QFrame( parent, name, f, FALSE )
{
    d = new QScrollViewData(this,(f&WPaintClever));

    connect( &d->hbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( hslide( int ) ) );
    connect( &d->vbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( vslide( int ) ) );
    d->viewport.installEventFilter( this );
}

/*! Destructs the QScrollView.  Any children added with addChild()
  will be destructed. */

QScrollView::~QScrollView()
{
    // Be careful not to get all those useless events...
    d->viewport.removeEventFilter( this );
    QScrollViewData * d2 = d;
    d = 0;
    delete d2;
}

// This variable allows ensureVisible to move the contents then
// update both the sliders.  Otherwise, updating the sliders would
// cause two image scrolls, creating ugly flashing.
//
static bool signal_choke=FALSE;

void QScrollView::hslide( int pos )
{
    if ( !signal_choke ) {
	moveContents( -pos, -contentsY() );
	QApplication::syncX();
    }
}

void QScrollView::vslide( int pos )
{
    if ( !signal_choke ) {
	moveContents( -contentsX(), -pos );
	QApplication::syncX();
    }
}

/*!
  Updates scrollbars - all possibilities considered.  You should never
  need to call this in your code.
*/
void QScrollView::updateScrollBars()
{
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
	if ( showh && h-sbDim-tmarg-bmarg < contentsHeight() ) {
	    needv=TRUE;
	    if (d->vMode == Auto)
		showv=TRUE;
	}
	if ( showv && w-sbDim-lmarg-rmarg < contentsWidth() ) {
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
	porth=h-sbDim-tmarg-bmarg;
    } else {
	if (!needh)
	    hslide( 0 ); // move to left
	d->hbar.hide();
	porth=h-tmarg-bmarg;
    }
    if ( showv ) {
	portw=w-sbDim-lmarg-rmarg;
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
	int right = ( showv || cornerWidget() ) ? w-sbDim : w;
	d->hbar.setGeometry( 0, h-sbDim, right, sbDim );
	bottom=h-sbDim;
    } else {
	bottom=h;
    }
    if ( showv ) {
	d->viewport.setGeometry( lmarg, tmarg,
				 w-sbDim-lmarg-rmarg, bottom-tmarg-bmarg );
	changeFrameRect(QRect(0, 0, w-sbDim, bottom));
	if (cornerWidget())
	    d->vbar.setGeometry( w-sbDim, 0, sbDim, h-sbDim );
	else
	    d->vbar.setGeometry( w-sbDim, 0, sbDim, bottom );
    } else {
	changeFrameRect(QRect(0, 0, w, bottom));
	d->viewport.setGeometry( lmarg, tmarg,
				 w-lmarg-rmarg, bottom-tmarg-bmarg );
    }
    if ( d->corner )
	d->corner->setGeometry( w-sbDim, h-sbDim, sbDim, sbDim );

    if ( contentsX()+d->viewport.width() > contentsWidth() ) {
	int x=QMAX(0,contentsWidth()-d->viewport.width());
	d->hbar.setValue(x);
	// Do it even if it is recursive
	moveContents( -x, -contentsY() );
    }
    if ( contentsY()+d->viewport.height() > contentsHeight() ) {
	int y=QMAX(0,contentsHeight()-d->viewport.height());
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
    d->hideOrShowAll(this);
    updateScrollBars();
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QScrollView::resize( int w, int h )
{
    // Need both this and resize event, due to deferred resize event.
    QWidget::resize( w, h );
    d->hideOrShowAll(this);
    updateScrollBars();
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
    QFrame::resizeEvent( event );
    d->hideOrShowAll(this);
    updateScrollBars();
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

/*!
  Sets the mode for the vertical scrollbar.

  \define QScrollView::ScrollBarMode

  <ul>
   <li> \c Auto (the default) shows a scrollbar when the content is too tall to fit.
   <li> \c AlwaysOff never shows a scrollbar.
   <li> \c AlwaysOn always shows a scrollbar.
  </ul>

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

  \define QScrollView::ResizePolicy

  The policies are:
  <ul>
   <li> \c Default is the initial value.  It converts to \c Manual if
	    the view is resized with resizeContents(), or to \c AutoOne
	    if a child is added.
   <li> \c Manual means the view stays the size set by resizeContents().
   <li> \c AutoOne means that if there is only only child widget, the
	    view stays the size of that widget.
  </ul>
*/
void QScrollView::setResizePolicy( ResizePolicy r )
{
    d->policy = r;
}

/*!
  Returns the currently set ResizePolicy.
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
  Inserts \a child into the scrolled area positioned at (\a x, \a y).
  The position defaults to (0,0). If the child is already in the view,
  it is just moved.
*/
void QScrollView::addChild(QWidget* child, int x, int y)
{
    if ( child->parentWidget() == &d->viewport ) {
	// May already be there
	QSVChildRec *r = d->rec(child);
	if (r) {
	    r->moveTo(this,x,y);
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
    if ( child->parentWidget() != &d->viewport ) {
	child->reparent( &d->viewport, 0, QPoint(0,0), FALSE );
    }
    d->addChildRec(child,x,y)->hideOrShow(this);

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
    if ( obj == &d->viewport ) {
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
  This is a low-level painting routine that draws the viewport
  contents.  Override this if drawContentsOffset() is too high-level.
  (for example, if you don't want to open a QPainter on the viewport).
*/
void QScrollView::viewportPaintEvent( QPaintEvent* pe )
{
    QPainter p(&d->viewport);
    p.setClipRegion(pe->region());
    int ex = pe->rect().x() + contentsX();
    int ey = pe->rect().y() + contentsY();
    int ew = pe->rect().width();
    int eh = pe->rect().height();
    drawContentsOffset(&p, contentsX(), contentsY(), ex, ey, ew, eh);
}


/*!
  To provide simple processing of events on the contents, this method
  receives all resize events sent to the viewport.

  \sa QWidget::resizeEvent()
*/
void QScrollView::viewportResizeEvent( QResizeEvent*  )
{
}

/*!
  To provide simple processing of events on the contents, this method receives all mouse
  press events sent to the viewport.

  \sa QWidget::mousePressEvent()
*/
void QScrollView::viewportMousePressEvent( QMouseEvent* )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all mouse
  release events sent to the viewport.

  \sa QWidget::mouseReleaseEvent()
*/
void QScrollView::viewportMouseReleaseEvent( QMouseEvent* )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all mouse
  double click events sent to the viewport.

  \sa QWidget::mouseDoubleClickEvent()
*/
void QScrollView::viewportMouseDoubleClickEvent( QMouseEvent* )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all mouse
  move events sent to the viewport.

  \sa QWidget::mouseMoveEvent()
*/
void QScrollView::viewportMouseMoveEvent( QMouseEvent* )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drag enter
  events sent to the viewport.

  \sa QWidget::dragEnterEvent()
*/
void QScrollView::viewportDragEnterEvent( QDragEnterEvent * )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drag move
  events sent to the viewport.

  \sa QWidget::dragMoveEvent()
*/
void QScrollView::viewportDragMoveEvent( QDragMoveEvent * )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drag leave
  events sent to the viewport.

  \sa QWidget::dragLeaveEvent()
*/
void QScrollView::viewportDragLeaveEvent( QDragLeaveEvent * )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all drop
  events sent to the viewport.

  \sa QWidget::dropEvent()
*/
void QScrollView::viewportDropEvent( QDropEvent * )
{
}

/*!
  To provide simple processing of events on the contents,
  this method receives all wheel
  events sent to the viewport.

  \sa QWidget::wheelEvent()
*/
void QScrollView::viewportWheelEvent( QWheelEvent * )
{
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
    int pw=d->viewport.width();
    int ph=d->viewport.height();

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

    if ( x < -cx+xmargin )
	cx = -x+pw-xmargin;
    else if ( x >= -cx+pw-xmargin )
	cx = -x+xmargin;

    if ( y < -cy+ymargin )
	cy = -y+ph-ymargin;
    else if ( y >= -cy+ph-ymargin )
	cy = -y+ymargin;

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
    signal_choke=TRUE;
    moveContents( -x, -y );
    d->vbar.setValue( y );
    d->hbar.setValue( x );
    updateScrollBars();
    signal_choke=FALSE;
    updateScrollBars();
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
    int pw=d->viewport.width();
    int ph=d->viewport.height();
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
    if ( -x+d->viewport.width() > contentsWidth() )
	x=QMIN(0,-contentsWidth()+d->viewport.width());
    if ( -y+d->viewport.height() > contentsHeight() )
	y=QMIN(0,-contentsHeight()+d->viewport.height());

    int dx = x - d->vx;
    int dy = y - d->vy;

    if (!dx && !dy)
	return; // Nothing to do

    d->vx = x;
    d->vy = y;

    emit contentsMoving( x, y );

    if ( /*dx && dy ||*/
	 ( QABS(dy) * 5 > d->viewport.height() * 4 ) ||
	 ( QABS(dx) * 5 > d->viewport.width() * 4 ) )
    {
	// Big move
	d->viewport.update();
	d->moveAllBy(dx,dy);
    } else {
	// Small move
	d->viewport.scroll(dx,dy);
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
    if ( ow < viewport()->width() && w >= 0 ) {
	if ( ow < 0 )
	    ow = 0;
	if ( w > viewport()->width() )
	    w = viewport()->width();
	viewport()->update( contentsX()+ow, 0, w-ow, viewport()->height() );
    }

    if ( oh > h ) {
	// Swap
	int t=h;
	h=oh;
	oh=t;
    }
    // Refresh area oh..h
    if ( oh < viewport()->height() && h >= 0 ) {
	if ( oh < 0 )
	    oh = 0;
	if ( h > viewport()->height() )
	    h = viewport()->height();
	viewport()->update( 0, contentsY()+oh, viewport()->width(), h-oh);
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
    if ( w > vp->width() )
	w = vp->width();
    if ( h > vp->height() )
	h = vp->height();

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
    if ( w > vp->width() )
	w = vp->width();
    if ( h > vp->height() )
	h = vp->height();

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

  The default implementation does nothing.
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
    return &d->viewport;
}


void QScrollView::changeFrameRect(const QRect& r)
{
    QRect oldr = frameRect();
    if (oldr != r) {
	setFrameRect(r);
	if ( frameWidth() ) {
	    // Redraw frames
	    update(r);
	    update(oldr);
	}
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
	     (w->focusPolicy() == TabFocus || w->focusPolicy() == StrongFocus)
	     && w->isEnabledToTLW() &&!w->focusProxy() && w->isVisibleToTLW() )
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
