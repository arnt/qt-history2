/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qviewp.cpp#1 $
**
** Implementation of QViewport class
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
#include "qviewp.h"

/*!
\class QViewport qviewp.h
\brief The QViewport widget provides a scrolling area with on-demand scrollbars.

The QViewport can be used in two ways:
<ol>
 <li> to view a (large) QWidget
 <li> to view a (large) object with an arbitrary display requirement.
</ol>

To use a QViewport in the first way, use view(QWidget*) to set the large
widget to be viewed.

For the second way, you must inherit from QViewport and override
viewX(),
viewY(),
viewWidth(),
viewHeight(),
viewVisible(),
moveView(int,int), and one of
drawContents(QPainter*) or 
drawContentsOffset(QPainter*,int,int). Also in this case you must
call updateScrollBars() when the values of the first 5 methods changes.

QWidgets have a maximum size, limited by the underlying window system,
of 32767 by 32767 pixels.

<img src=qviewp-m.gif> <img src=qviewp-w.gif>
*/


/*!
Construct a QViewport.  A single child can then be added with the view()
method.
*/
QViewport::QViewport( QWidget *parent, const char *name, WFlags f ) :
    QWidget( parent, name, f ),
    hbar( QScrollBar::Horizontal, this, "horizontal" ),
    vbar( QScrollBar::Vertical, this, "vertical" ),
    porthole( this, "porthole" ),
    viewed( 0 ),
    vx( 0 ), vy( 0 ), vwidth( 1 ), vheight( 1 )
{
    connect( &hbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( hslide( int ) ) );
    connect( &vbar, SIGNAL( valueChanged( int ) ),
	this, SLOT( vslide( int ) ) );
    porthole.installEventFilter( this );
}

// This variable allows ensureVisible to move the viewed widget then
// update both the sliders.  Otherwise, updating the sliders would
// cause two image scrolls, creating ugly flashing.
//
bool QViewport::signal_choke=FALSE;

void QViewport::hslide( int pos )
{
    if ( !signal_choke ) {
	moveView( -pos, viewY() );
    }
}

void QViewport::vslide( int pos )
{
    if ( !signal_choke ) {
	moveView( viewX(), -pos );
    }
}

/*
  Update scrollbars - all possibilities considered.
*/
void QViewport::updateScrollBars()
{
    int w=width();
    int h=height();

    if ( viewVisible() ) {
	int portw, porth;

	bool needh = w < viewWidth();
	bool needv = h < viewHeight();

	if ( needh && h-scrollBarWidth() < viewHeight() )
	    needv=TRUE;
	if ( needv && w-scrollBarWidth() < viewWidth() )
	    needh=TRUE;

	if ( needh ) {
	    hbar.show();
	    porth=h-scrollBarWidth();
	} else {
	    hslide( 0 );
	    hbar.hide();
	    porth=h;
	}

	if ( needv ) {
	    vbar.show();
	    portw=w-scrollBarWidth();
	} else {
	    vslide( 0 );
	    vbar.hide();
	    portw=w;
	}

	if ( needv ) {
	    vbar.setRange( 0, viewHeight()-porth );
	    vbar.setSteps( vbar.lineStep(), porth );
	}
	if ( needh ) {
	    hbar.setRange( 0, viewWidth()-portw );
	    hbar.setSteps( hbar.lineStep(), portw );
	}

	int top, bottom;

	if ( needh ) {
	    int right=( (needv && emptyCorner() )
		     || alwaysEmptyCorner() )
		? w-scrollBarWidth() : w;
	    if ( scrollBarOnTop() ) {
		hbar.setGeometry( 0, h-scrollBarWidth(), right, scrollBarWidth() );
		top=scrollBarWidth();
		bottom=h;
	    } else {
		hbar.setGeometry( 0, h-scrollBarWidth(), right, scrollBarWidth() );
		top=0;
		bottom=h-scrollBarWidth();
	    }
	} else {
	    top=0;
	    bottom=h;
	}
	if ( needv ) {
	    if ( scrollBarOnLeft() ) {
		vbar.setGeometry( 0, top, scrollBarWidth(), bottom );
		porthole.setGeometry( scrollBarWidth(), top, w-scrollBarWidth(), bottom );
	    } else {
		vbar.setGeometry( w-scrollBarWidth(), top, scrollBarWidth(), bottom );
		porthole.setGeometry( 0, top, w-scrollBarWidth(), bottom );
	    }
	} else {
	    porthole.setGeometry( 0, top, w, bottom );
	}
    } else {
	hbar.hide();
	vbar.hide();
	porthole.setGeometry( 0, 0, w, h );
    }
}

/*!
An override - ensures scrollbars are correct size upon showing.
*/
void QViewport::show()
{
    updateScrollBars();
    QWidget::show();
}

/*!
An override - ensures scrollbars are correct size upon resize.
*/
void QViewport::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );
    updateScrollBars();
}

/*!
Override this to adjust width of scrollbars.
Default returns 16.
*/
int QViewport::scrollBarWidth() const
{
    return 16;
}

/*!
Overriding this to determine on which side the vertical scrollbar appears.
Default returns FALSE ( always on right ).
*/
bool QViewport::scrollBarOnLeft() const
{
    return FALSE;
}

/*!
Overriding this to determine on which side the horizontal scrollbar appears.
Default returns FALSE ( always on bottom ).
*/
bool QViewport::scrollBarOnTop() const
{
    return FALSE;
}

/*!
Overriding this to determine whether the corner between the
scrollbars is empty when both are present.  Otherwise, the horizontal
scrollbar will use that space.

Default returns TRUE ( empty when both appear ).
*/
bool QViewport::emptyCorner() const
{
    return TRUE;
}

/*!
Overriding this to determine whether the corner between the
scrollbars is \e always empty.  Otherwise, if one scrollbar is
present, it will use that space; if two, effect is determined
by emptyCorner().

Default returns FALSE.
*/
bool QViewport::alwaysEmptyCorner() const
{
    return FALSE;
}

/*!
  Set a widget to appear in the scrolling area.  Any previous widget
  is hidden.  The added widget should have no current parent.

  \code
    QViewport vp(...);
    vp.view(new MyLargeWidget);
  \endcode
*/

void QViewport::view(QWidget* w)
{
    if ( viewed ) {
	w->removeEventFilter( this );
	viewed->hide();
    }
    if ( w ) {
	if ( w->parentWidget() != &porthole ) {
	    w->recreate( &porthole, 0, QPoint(0,0), TRUE );
	}
	w->installEventFilter( this );
    }
    viewed = w;
    updateScrollBars();
}

/*!
  This event filter ensures the scrollbars are updated when the
  viewed widget is resized, shown, hidden, or destroyed.
*/

bool QViewport::eventFilter( QObject *obj, QEvent *e )
{
    if ( obj == viewed ) {
	switch ( e->type() ) {
	  case Event_Resize:
	  case Event_Show:
	  case Event_Hide:
	    updateScrollBars();
	    break;
	  case Event_Destroy:
	    view(0);
	}
    } else if ( obj == &porthole ) {
	if ( e->type() == Event_Paint ) {
	    QPaintEvent* pe = (QPaintEvent*)e;
	    QPainter p(&porthole);
	    p.setClipRect(pe->rect());
	    int ex = pe->rect().x() - viewX();
	    int ey = pe->rect().y() - viewY();
	    int ew = pe->rect().width();
	    int eh = pe->rect().height();
	    drawContentsOffset(&p, viewX(), viewY(), ex, ey, ew, eh);
	}
    }
    return FALSE;  // always continue with standard event processing
}

/*!
\fn QScrollBar& QViewport::horizontalScrollBar()

 Returns the component horizontal scrollbar.  It is made available to allow
 accelerators, autoscrolling, etc., and to allow changing
 of arrow scrollrates: bar.setSteps( rate, bar.pageStep() ).

 It should not be otherwise manipulated.
*/

/*!
\fn QScrollBar& QViewport::verticalScrollBar()

 Returns the component vertical scrollbar.  It is made available to allow
 accelerators, autoscrolling, etc., and to allow changing
 of arrow scrollrates: bar.setSteps( rate, bar.pageStep() ).

 It should not be otherwise manipulated.
*/

/*!
 Sets the background color of the area \e behind the scrolling widget,
 which is only visible if that widget is smaller than the viewport.
*/
void QViewport::setBackgroundColor(const QColor& c)
{
    porthole.setBackgroundColor(c);
}

/*!
 Sets the background pixmap of the area \e behind the scrolling widget,
 which is only visible if that widget is smaller than the viewport.
*/
void QViewport::setBackgroundPixmap(const QPixmap& pm)
{
    porthole.setBackgroundPixmap(pm);
}

/*!
 Move such that ( x, y ) is visible and with at least the given
 pixel margins ( if possible, otherwise, centered ).
*/
void QViewport::ensureVisible( int x, int y, int xmargin, int ymargin )
{
    int pw=porthole.width();
    int ph=porthole.height();

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
    vbar.setValue( -cy );
    hbar.setValue( -cx );
    updateScrollBars();
    signal_choke=FALSE;
}

/*!
 Completely centered ( except at edges of playfield )
*/
void QViewport::centerOn( int x, int y )
{
    ensureVisible( x, y, 32000, 32000 );
}

/*!
 Margins as fraction of visible area.
   0.0 = Allow (x,y) to be on edge of visible area.
   0.5 = Ensure (x,y) is in middle 50% of visible area.
   1.0 = CenterOn (x,y).
*/
void QViewport::centralize( int x, int y, float xmargin, float ymargin )
{
    int pw=porthole.width();
    int ph=porthole.height();
    ensureVisible( x, y, int( xmargin/2.0*pw+0.5 ), int( ymargin/2.0*ph+0.5 ) );
}

/*!
*/
void QViewport::moveView(int x, int y)
{
    if (viewed) {
	viewed->move( x, y );
    } else {
	int dx = x - vx;
	int dy = y - vy;
	vx = x;
	vy = y;
	if (QABS(dx) > width() || QABS(dy) > height())
	    porthole.update();
	else
	    porthole.scroll(dx,dy);
    }
}

int QViewport::viewX() const
{
    return viewed ? viewed->x() : vx;
}

int QViewport::viewY() const
{
    return viewed ? viewed->y() : vy;
}

int QViewport::viewWidth() const
{
    return viewed ? viewed->width() : vwidth;
}

int QViewport::viewHeight() const
{
    return viewed ? viewed->height() : vheight;
}

bool QViewport::viewVisible() const
{
    return !viewed || viewed->isVisible();
}

void QViewport::viewResize( int w, int h )
{
    if ( viewed ) fatal("Called QViewport::view() then QViewport::viewResize()");

    vwidth = w;
    vheight = h;
}

void QViewport::drawContents(QPainter*, int, int, int, int)
{
}

void QViewport::drawContentsOffset(QPainter* p, int ox, int oy,
    int cx, int cy, int cw, int ch)
{
    p->translate(ox,oy);
    drawContents(p, cx, cy, cw, ch);
}
