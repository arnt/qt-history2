/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollview.cpp#11 $
**
** Implementation of QScrollView class
**
** Created : 950524
**
** Copyright ( C ) 1995, 1996 by Warwick Allison.
** Copyright ( C ) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwidget.h"
#include "qscrbar.h"
#include "qobjcoll.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qscrollview.h"

const int sbDim = 16;

struct QScrollViewData {
    QScrollViewData(QWidget* parent) :
	hbar( QScrollBar::Horizontal, parent, "qt_hbar" ),
	vbar( QScrollBar::Vertical, parent, "qt_vbar" ),
	viewport( parent, "qt_viewport" ),
	viewed( 0 ),
	vx( 0 ), vy( 0 ), vwidth( 1 ), vheight( 1 )
    {
	viewport.setBackgroundMode( QWidget::PaletteDark );
	vMode = QScrollView::Auto;
	hMode = QScrollView::Auto;
	corner = 0;
	vbar.setSteps( 20, 1/*set later*/ );
	hbar.setSteps( 20, 1/*set later*/ );
    }

    QScrollBar	hbar;
    QScrollBar	vbar;
    QWidget	viewport;
    QWidget*	viewed;
    QWidget*	corner;
    int		vx, vy, vwidth, vheight; // for drawContents-style usage

    QScrollView::ScrollBarMode	vMode		: 2;
    QScrollView::ScrollBarMode	hMode		: 2;
};

/*!
\class QScrollView qscrollview.h
\brief The QScrollView widget provides a scrolling area with on-demand scrollbars.

The QScrollView can be used in two ways:
<ol>
 <li> to view a (large) QWidget
 <li> to view a (large) object with an arbitrary display requirement.
</ol>

To use a QScrollView in the first way, use view(QWidget*) to set the large
widget to be viewed.

For the second technique, you must inherit from QScrollView and override
drawContentsOffset(), and use viewResized() to set
the size of the viewed area.

QWidgets have a maximum size, limited by the underlying window system,
of 32767 by 32767 pixels.  The second technique is not constrained by
this limitation, but currently the scrollbars fail beyond about
1000000 pixels due to integer overflow.

<img src=qscrollview-m.gif> <img src=qscrollview-w.gif>
*/


/*!
Constructs a QScrollView.  A single child can then be added with the view()
method.
*/
QScrollView::QScrollView( QWidget *parent, const char *name, WFlags f ) :
    QFrame( parent, name, f, FALSE )
{
    d = new QScrollViewData(this);

    connect( &d->hbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( hslide( int ) ) );
    connect( &d->vbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( vslide( int ) ) );
    d->viewport.installEventFilter( this );
}

/*!
Destructs the QScrollView.  Any QWidget set as viewed will be destructed.
*/
QScrollView::~QScrollView()
{
    delete d;
}

// This variable allows ensureVisible to move the viewed widget then
// update both the sliders.  Otherwise, updating the sliders would
// cause two image scrolls, creating ugly flashing.
//
static bool signal_choke=FALSE;

void QScrollView::hslide( int pos )
{
    if ( !signal_choke ) {
	moveView( -pos, viewY() );
    }
}

void QScrollView::vslide( int pos )
{
    if ( !signal_choke ) {
	moveView( viewX(), -pos );
    }
}

/*!
  Updates scrollbars - all possibilities considered.
*/
void QScrollView::updateScrollBars()
{
    int fw = frameWidth();

    int w = width();
    int h = height();

    int portw, porth;

    bool needh;
    bool needv;
    bool showh;
    bool showv;

    if ( !d->viewed || d->viewed->isVisible() ) {
	// Do we definately need the scrollbar?
	needh = w-fw*2 < viewWidth();
	needv = h-fw*2 < viewHeight();

	// Do we intend to show the scrollbar?
	if (d->hMode == AlwaysOn) showh = TRUE;
	else if (d->hMode == AlwaysOff) showh = FALSE;
	else showh = needh;
	if (d->vMode == AlwaysOn) showv = TRUE;
	else if (d->vMode == AlwaysOff) showv = FALSE;
	else showv = needv;

	// Given other scrollbar will be shown, NOW do we need one?
	if ( showh && h-sbDim-fw*2 < viewHeight() ) {
	    needv=TRUE; if (d->vMode == Auto) showv=TRUE;
	}
	if ( showv && w-sbDim-fw*2 < viewWidth() ) {
	    needh=TRUE; if (d->hMode == Auto) showh=TRUE;
	}
    } else {
	// Scrollbars not needed, only show scrollbar that are always on.
	needh = needv = FALSE;
	showh = d->hMode == AlwaysOn;
	showv = d->vMode == AlwaysOn;
    }

    // Hide unneeded scrollbar, calculate viewport size
    if ( showh ) {
	porth=h-sbDim-fw*2;
    } else {
	if (!needh) hslide( 0 ); // move widget to left
	d->hbar.hide();
	porth=h-fw*2;
    }
    if ( showv ) {
	portw=w-sbDim-fw*2;
    } else {
	if (!needv) vslide( 0 ); // move widget to top
	d->vbar.hide();
	portw=w-fw*2;
    }

    // Configure scrollbars that we will show
    if ( showv ) {
	if ( needv ) {
	    d->vbar.setRange( 0, viewHeight()-porth );
	    d->vbar.setSteps( d->vbar.lineStep(), porth );
	} else {
	    d->vbar.setRange( 0, 0 );
	}
    }
    if ( showh ) {
	if ( needh ) {
	    d->hbar.setRange( 0, viewWidth()-portw );
	    d->hbar.setSteps( d->hbar.lineStep(), portw );
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
	d->viewport.setGeometry( fw, fw, w-sbDim-fw*2, bottom-fw*2 );
	setFrameRect(QRect(0, 0, w-sbDim, bottom));
	if (cornerWidget())
	    d->vbar.setGeometry( w-sbDim, 0, sbDim, h-sbDim );
	else
	    d->vbar.setGeometry( w-sbDim, 0, sbDim, bottom );
    } else {
	setFrameRect(QRect(0, 0, w, bottom));
	d->viewport.setGeometry( fw, fw, w-fw*2, bottom-fw*2 );
    }
    if ( d->corner )
	d->corner->setGeometry( w-sbDim, h-sbDim, sbDim, sbDim );

    // Finally, show the scrollbars.
    if ( showh ) d->hbar.show();
    if ( showv ) d->vbar.show();
}


/*!
An override - ensures scrollbars are correct size upon showing.
*/
void QScrollView::show()
{
    QWidget::show();
    updateScrollBars();
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QScrollView::resize( int w, int h )
{
    // Need both this and resize event, due to deferred resize event.
    QWidget::resize( w, h );
    updateScrollBars();
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QScrollView::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );
    updateScrollBars();
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
   <li> \c Auto shows a scrollbar when the widget or area is too tall to fit.
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
   <li> \c Auto shows a scrollbar when the widget or area is too wide to fit.
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
  unless you seperately
  recreate the widget after setting some other corner widget (or 0).

  Any \e newly viewed widget should have no current parent.

  By default, no corner widget is present.

  \sa setVScrollBarMode(), setHScrollBarMode()
*/
void QScrollView::setCornerWidget(QWidget* corner)
{
    QWidget* oldcorner = d->corner;
    if (oldcorner != corner) {
	if (oldcorner) oldcorner->hide();
	d->corner = corner;
	if ( corner && corner->parentWidget() != this )
	    corner->recreate( this, 0, QPoint(0,0), FALSE );
	updateScrollBars();
	if ( corner ) corner->show();
    }
}

/*!
  Sets the widget to appear in the scrolling area.
  Passing 0 shows no widget in the scrolling area.
  Any previous widget is hidden.
  You may call view() with the same widget at different times.

  All widgets set here will be deleted by the QScrollView when it destructs
  unless you seperately
  recreate the widget after viewing some other widget (or 0).

  Any \e newly viewed widget should have no current parent.

  \code
    QScrollView vp(...);
    vp.view(new MyLargeWidget);
  \endcode
*/

void QScrollView::view(QWidget* w)
{
    if ( d->viewed ) {
	w->removeEventFilter( this );
	d->viewed->hide(); // Don't delete - allow using any of a set of widgets
    }
    if ( w ) {
	if ( w->parentWidget() != &d->viewport ) {
	    w->recreate( &d->viewport, 0, QPoint(0,0), isVisible() );
	} else {
	    w->show();
	}
	w->installEventFilter( this );
    }
    d->viewed = w;
    updateScrollBars();
}

/*!
  This event filter ensures the scrollbars are updated when the
  viewed widget is resized, shown, hidden, or destroyed.
*/

bool QScrollView::eventFilter( QObject *obj, QEvent *e )
{
    if ( obj == d->viewed ) {
	switch ( e->type() ) {
	  case Event_Move:
	    {
		QMoveEvent* mv = (QMoveEvent*)e;
		if ( mv->pos() != mv->oldPos() ) {
		    int cx = mv->pos().x();
		    int cy = mv->pos().y();
		    bool fix=FALSE;
		    if (cx>0) {
			fix=TRUE;
			cx=0;
		    }
		    if (cy>0) {
			fix=TRUE;
			cy=0;
		    }
		    signal_choke=TRUE;
		    moveView( cx, cy );
		    d->vbar.setValue( -cy );
		    d->hbar.setValue( -cx );
		    updateScrollBars();
		    signal_choke=FALSE;
		    if (fix)
			d->viewed->move(cx,cy);
		}
		break;
	    }
	  case Event_Resize:
	  case Event_Show:
	  case Event_Hide:
	    updateScrollBars();
	    break;
	  case Event_Destroy:
	    view(0);
	}
    } else if ( obj == &d->viewport ) {
	if ( e->type() == Event_Paint ) {
	    QPainter p;
	    p.begin(&d->viewport);
	    QPaintEvent* pe = (QPaintEvent*)e;
	    p.setClipRect(pe->rect());
	    int ex = pe->rect().x() - viewX();
	    int ey = pe->rect().y() - viewY();
	    int ew = pe->rect().width();
	    int eh = pe->rect().height();
	    drawContentsOffset(&p, viewX(), viewY(), ex, ey, ew, eh);
	    p.end();
	}
    }
    return FALSE;  // always continue with standard event processing
}

/*!
 Returns the component horizontal scrollbar.  It is made available to allow
 accelerators, autoscrolling, etc., and to allow changing
 of arrow scrollrates: bar->setSteps( rate, bar->pageStep() ).

 It should not be otherwise manipulated.
*/
QScrollBar* QScrollView::horizontalScrollBar() { return &d->hbar; }

/*!
 Returns the component vertical scrollbar.  It is made available to allow
 accelerators, autoscrolling, etc., and to allow changing
 of arrow scrollrates: bar->setSteps( rate, bar->pageStep() ).

 It should not be otherwise manipulated.
*/
QScrollBar* QScrollView::verticalScrollBar() { return &d->vbar; }

/*!
 Moves the scrolled widget or area so that the point (x, y) is visible
 with at least 50-pixel margins (if possible, otherwise centered).
*/
void QScrollView::ensureVisible( int x, int y )
{
    ensureVisible(x, y, 50, 50);
}

/*!
 Moves the scrolled widget or area so that the point (x, y) is visible
 with at least the given pixel margins (if possible, otherwise centered).
*/
void QScrollView::ensureVisible( int x, int y, int xmargin, int ymargin )
{
    updateScrollBars();

    int pw=d->viewport.width();
    int ph=d->viewport.height();

    int cx=viewX();
    int cy=viewY();
    int cw=viewWidth();
    int ch=viewHeight();

    if ( pw < xmargin*2 ) xmargin=pw/2;
    if ( ph < ymargin*2 ) ymargin=ph/2;

    if ( cw <= pw ) { xmargin=0; cx=0; }
    if ( ch <= ph ) { ymargin=0; cy=0; }

    if ( x < -cx+xmargin ) {
	cx = -x+pw-xmargin;
    } else if ( x >= -cx+pw-xmargin ) {
	cx = -x+xmargin;
    }

    if ( y < -cy+ymargin ) {
	cy = -y+ph-ymargin;
    } else if ( y >= -cy+ph-ymargin ) {
	cy = -y+ymargin;
    }

    if ( cx > 0 ) cx=0;
    else if ( cx < pw-cw && cw>pw ) cx=pw-cw;
    if ( cy > 0 ) cy=0;
    else if ( cy < ph-ch && ch>ph ) cy=ph-ch;

    // Choke signal handling while we update BOTH sliders.
    signal_choke=TRUE;
    moveView( cx, cy );
    d->vbar.setValue( -cy );
    d->hbar.setValue( -cx );
    updateScrollBars();
    signal_choke=FALSE;
}

/*!
 Moves the scrolled widget or area so that the point (x,y) is in the
 center of visible area.
*/
void QScrollView::center( int x, int y )
{
    ensureVisible( x, y, 32000, 32000 );
}

/*!
 Moves the scrolled widget or area so that the point (x,y) is visible,
 with the given margins (as fractions of visible area).

 eg.
 <ul>
   <li>Margin 0.0 allows (x,y) to be on edge of visible area.
   <li>Margin 0.5 ensures (x,y) is in middle 50% of visible area.
   <li>Margin 1.0 ensures (x,y) is in the center of the viewing area.
 </ul>
*/
void QScrollView::center( int x, int y, float xmargin, float ymargin )
{
    int pw=d->viewport.width();
    int ph=d->viewport.height();
    ensureVisible( x, y, int( xmargin/2.0*pw+0.5 ), int( ymargin/2.0*ph+0.5 ) );
}


/*!
  \fn void QScrollView::viewMoved(int x, int y)

  This signal is emitted after the view is moved.

  \sa viewX(), viewY()
*/

/*!
  Moves the viewed widget or area.
*/
void QScrollView::moveView(int x, int y)
{
    if (d->viewed) {
	if (d->viewed->pos() == QPoint(x,y))
	    return; // Nothing to do

	d->viewed->move( x, y );
    } else {
	int dx = x - d->vx;
	int dy = y - d->vy;

	if (!dx && !dy)
	    return; // Nothing to do

	d->vx = x;
	d->vy = y;

	d->viewport.scroll(dx,dy);
    }

    emit viewMoved( x, y );
}

/*!
  Returns the horizontal position of the viewed widget/area.
  Note that this is normally a negative value - the origin of the
  viewed area is to the left and above the viewport containing it.
*/
int QScrollView::viewX() const
{
    return d->viewed ? d->viewed->x() : d->vx;
}

/*!
  Returns the vertical position of the viewed widget/area.
  Note that this is normally a negative value - the origin of the
  viewed area is to the left and above the viewport containing it.
*/
int QScrollView::viewY() const
{
    return d->viewed ? d->viewed->y() : d->vy;
}

/*!
  Returns the width of the viewed area.
*/
int QScrollView::viewWidth() const
{
    return d->viewed ? d->viewed->width() : d->vwidth;
}

/*!
  Returns the height of the viewed area.
*/
int QScrollView::viewHeight() const
{
    return d->viewed ? d->viewed->height() : d->vheight;
}

/*!
  Set the size of the viewed area.
*/
void QScrollView::viewResize( int w, int h )
{
    if ( d->viewed ) {
	// Strange.  Why did the programmer do that.  Oh well, do it.
	d->viewed->resize(w,h);
    }
    d->vwidth = w;
    d->vheight = h;

    // Could more efficiently scroll if shrinking, repaint if growing, etc.
    updateScrollBars();
    update();
}

/*!
  \fn void QScrollView::drawContentsOffset(QPainter*, int offsetx, int offsety, int clipx, int clipy, int clipw, int cliph)

  Override this method if you are viewing a drawing area rather than a widget.

  Draw the contents of the viewed area, offset by the given
  amount, clipped to the given area. Note that the final coordinates
  you give to QPainter methods must be within the range supported
  by the underlying window system - about +/- 32000.  See
  scrollview/scrollview.cpp for an example.

  The clip rectangle of the painter is already set appropriately.

  Note that QPainter::translate() is not sufficient.

  The default implementation does nothing.
*/
void QScrollView::drawContentsOffset(QPainter*, int, int, int, int, int, int)
{
    // If QPainter could handle large translations...
    //
    // p->translate(offsetx,offsety);
    // drawContents(p, clipx, clipy, clipw, cliph);
}

/*!
An override - ensures scrollbars are correct size when frame style changes.
*/
void QScrollView::frameChanged()
{
    updateScrollBars();
    update();
}


/*!
  Returns the viewport widget of the scrollview.  This is the widget
  containing the viewed widget or which is the drawing area.
*/
QWidget* QScrollView::viewport()
{
    return &d->viewport;
}
