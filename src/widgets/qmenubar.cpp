/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#19 $
**
** Implementation of QMenuBar class
**
** Author  : Haavard Nord
** Created : 941209
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  INCLUDE_MENUITEM_DEF
#include "qmenubar.h"
#include "qkeycode.h"
#include "qpainter.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qmenubar.cpp#19 $";
#endif


// Motif style parameters

static const motifBarFrame	= 2;		// menu bar frame width
static const motifBarHMargin	= 2;		// menu bar hor margin to item
static const motifBarVMargin	= 2;		// menu bar ver margin to item
static const motifItemFrame	= 2;		// menu item frame width
static const motifItemHMargin	= 4;		// menu item hor text margin
static const motifItemVMargin	= 8;		// menu item ver text margin

/*

+-----------------------------
|      BarFrame
|   +-------------------------
|   |      V  BarMargin
|   |   +---------------------
|   | H |      ItemFrame
|   |   |  +-----------------
|   |   |  |			   \
|   |   |  |  ^  T E X T   ^	    | ItemVMargin
|   |   |  |  |		   |	   /
|   |	|      ItemHMargin
|   |
|

*/


// ---------------------------------------------------------------------------
// QMenuBar member functions
//

QMenuBar::QMenuBar( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    initMetaObject();
    isMenuBar = TRUE;
    irects = 0;
    if ( parent )				// filter parent events
	parent->installEventFilter( this );
    move( 0, 0 );
}

QMenuBar::~QMenuBar()
{
    delete irects;
    if ( parent() )
	parent()->removeEventFilter( this );
}


void QMenuBar::updateItem( int )		// update menu bar item
{
    repaint( FALSE );
}


void QMenuBar::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    if ( isVisible() ) {
	updateRects();
	repaint();
    }
}

void QMenuBar::menuStateChanged()
{
    repaint();
}

void QMenuBar::menuInsPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;			// set parent menu
    connect( popup, SIGNAL(activatedRedirect(int)), SLOT(subActivated(int)) );
    connect( popup, SIGNAL(selectedRedirect(int)),  SLOT(subSelected(int)) );
}

void QMenuBar::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(selectedRedirect(int)),  this,
		       SLOT(subSelected(int)) );
}


bool QMenuBar::eventFilter( QObject *object, QEvent *event )
{
    if ( object == parent() && event->type() == Event_Resize ) {
	QResizeEvent *e = (QResizeEvent *)event;
	resize( e->size().width(), height() );
    }
    return FALSE;				// don't stop event
}


void QMenuBar::subActivated( int id )
{
    emit activated( id );
}

void QMenuBar::subSelected( int id )
{
    emit selected( id );
}


bool QMenuBar::tryMouseEvent( QPopupMenu *popup, QMouseEvent *e )
{
    QPoint pos = mapFromGlobal( popup->mapToGlobal( e->pos() ) );
    if ( !rect().contains( pos ) )		// outside
	return FALSE;
    int item = itemAtPos( pos );
    if ( item == -1 && (e->type() == Event_MouseButtonPress ||
			e->type() == Event_MouseButtonRelease) ) {
	hidePopups();
	goodbye();
	return FALSE;
    }
    QMouseEvent ee( e->type(), pos, e->button(), e->state() );
    event( &ee );
    return TRUE;
}

void QMenuBar::tryKeyEvent( QPopupMenu *popup, QKeyEvent *e )
{
    event( e );
}


void QMenuBar::goodbye()			// set to idle state
{
    actItem = -1;
    repaint( FALSE );
}


void QMenuBar::openActPopup()			// open active popup menu
{
    if ( actItem < 0 )
	return;
    QPopupMenu *popup = mitems->at(actItem)->popup();
    if ( popup ) {
	QRect  r = itemRect( actItem );
	QPoint pos = r.bottomLeft() + QPoint(0,1);
	if ( popup->badSize )
	    popup->updateSize();
	pos = mapToGlobal(pos);
	int sh = QApplication::desktop()->height();
	int ph = popup->height();
	if ( pos.y() + ph > sh ) {
	    pos = mapToGlobal( r.topLeft() );
	    pos.ry() -= ph;
	}
	popup->popup( pos );
    }
}

void QMenuBar::hidePopups()			// hide popup items
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
}


void QMenuBar::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    update();
}

void QMenuBar::show()
{
    if ( parentWidget() )
	resize( parentWidget()->width(), height() );
    QWidget::show();
    raise();
}

void QMenuBar::hide()
{
    actItem = -1;
    hidePopups();
    QWidget::hide();
}


// ---------------------------------------------------------------------------
// Item geometry functions
//

void QMenuBar::updateRects()
{
    if ( !badSize )				// size was not changed
	return;
    delete irects;
    if ( mitems->isEmpty() ) {
	irects = 0;
	return;
    }
    irects = new QRect[ mitems->count() ];	// create rectangle array
    CHECK_PTR( irects );
    QFontMetrics fm = fontMetrics();
    int max_width = width();
    int max_height = 0;
    int nlines = 1;				// number of lines
    int nlitems = 0;				// number on items on cur line
    int x = motifBarFrame + motifBarHMargin;
    int y = motifBarFrame + motifBarVMargin;
    int i = 0;
    while ( i < mitems->count() ) {		// for each menu item...
	QMenuItem *mi = mitems->at(i);
	int w, h;
	if ( mi->image() ) {			// image item
	    w = mi->image()->width();
	    h = mi->image()->height();
	}
	else {					// text item
	    w = fm.width( mi->string() ) + 2*motifItemHMargin;
	    h = fm.height() + motifItemVMargin;
	}
	w += 2*motifItemFrame;
	h += 2*motifItemFrame;
	if ( x + w + motifBarFrame > max_width && nlitems > 0 ) {
	    nlines++;				// break line
	    nlitems = 0;
	    x = motifBarFrame + motifBarHMargin;
	    y += h + motifBarHMargin;
	}
	if ( y + h + 2*motifBarFrame > max_height )
	    max_height = y + h + 2*motifBarFrame;
	irects[i].setRect( x, y, w, h );
	x += w;
	nlitems++;
	i++;
    }
    if ( max_height != height() )
	resize( max_width, max_height );
    badSize = FALSE;
}

QRect QMenuBar::itemRect( int item )
{
    updateRects();
    return irects ? irects[item] : QRect(0,0,0,0);
}

int QMenuBar::itemAtPos( const QPoint &pos )	// get item at pos (x,y)
{
    updateRects();
    if ( !irects )
	return -1;
    int i = 0;
    while ( i < mitems->count() ) {
	if ( irects[i].contains( pos ) ) {
	    QMenuItem *mi = mitems->at(i);
	    return mi->isDisabled() || mi->isSeparator() ? -1 : i;
	}
	++i;
    }
    return -1;					// no match
}


// ---------------------------------------------------------------------------
// Event handlers
//

void QMenuBar::paintEvent( QPaintEvent *e )	// paint menu bar
{
    register QPainter *p;
    QPainter     paint;
    QColorGroup  g  = colorGroup();
    QFontMetrics fm = fontMetrics();
    QSize	 sz = size();

    p = &paint;
    p->begin( this );
    p->drawShadePanel( rect(), g.light(), g.dark(), motifBarFrame );
    p->setClipRect( motifBarFrame, motifBarFrame,
		    sz.width()  - 2*motifBarFrame,
		    sz.height() - 2*motifBarFrame );
    updateRects();
    for ( int i=0; i<mitems->count(); i++ ) {
	QMenuItem *mi = mitems->at( i );
	QRect r = irects[i];
	if ( i == actItem )			// active item frame
	    p->drawShadePanel( r, g.light(), g.dark(), motifItemFrame );
	else					// incognito frame
	    p->drawShadePanel( r, g.background(), g.background(),
			       motifItemFrame );
	if ( mi->image() )
	    p->drawPixMap( r.left() + motifItemFrame,
			   r.top() + motifItemFrame,
			   *mi->image() );
	else if ( mi->string() ) {
	    if ( mi->isDisabled() )
		p->setPen( palette().disabled().text() );
	    else
		p->setPen( g.text() );
	    p->drawText( r, AlignCenter | AlignVCenter | ShowPrefix | DontClip,
			 mi->string() );
	}
    }
    p->end();
}


void QMenuBar::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	actItem = -1;
	repaint( FALSE );
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	if ( mi->id() >= 0 )
	    emit activated( mi->id() );
    }
    QPopupMenu *popup = mi->popup();
    if ( popup ) {
	popup->actItem = -1;
	if ( popup->isVisible() ) {		// sub menu already open
	    popup->hidePopups();
	    popup->repaint( FALSE );
	}
	else {					// open sub menu
	    hidePopups();
	    openActPopup();
	}
    }
    else
	hidePopups();
}

void QMenuBar::mouseReleaseEvent( QMouseEvent *e )
{
    mouseBtDn = FALSE;				// mouse button up
    int item = itemAtPos( e->pos() );
    if ( actItem == -1 && item != -1 )		// ignore mouse release
	return;
    actItem = item;
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	register QMenuItem *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    if ( style() == MacStyle )
		popup->hide();
	    else
		popup->setFirstItemActive();
	}
	else {					// not a popup
	    actItem = -1;
	    repaint( FALSE );
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    else				// normal connection
		emit selected( mi->id() );
	}
    }
}

void QMenuBar::mouseMoveEvent( QMouseEvent *e )
{
    int item = itemAtPos( e->pos() );
    if ( item == -1 )
	return;
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	hidePopups();
	if ( mi->id() != -1 )
	    emit activated( mi->id() );
	if ( mi->popup() )
	    openActPopup();
    }
}


void QMenuBar::keyPressEvent( QKeyEvent *e )
{
    if ( actItem < 0 || mouseBtDn )		// cannot handle key event
	return;

    QMenuItem  *mi;
    QPopupMenu *popup;
    int d = 0;

    switch ( e->key() ) {
	case Key_Left:
	case Key_Up:
	    d = -1;
	    break;

	case Key_Right:
	case Key_Down:
	    d = 1;
	    break;

	case Key_Escape:
	    actItem = -1;
	    repaint( FALSE );
	    break;

	case Key_Return:
	case Key_Enter:
	    mi = mitems->at( actItem );
	    popup = mi->popup();
	    // ... what to do
	    break;
    }

    if ( d ) {					// highlight next/prev
	register int i = actItem;
	int c = mitems->count();
	int n = c;
	while ( n-- ) {
	    i = i + d;
	    if ( i == c )
		i = 0;
	    else if ( i < 0 )
		i = c - 1;
	    mi = mitems->at( i );
	    if ( !(mi->isSeparator() || mi->isDisabled()) )
		break;
	}
	if ( i != actItem ) {
	    actItem = i;
	    repaint( FALSE );
	    hidePopups();
	    popup = mi->popup();
	    if ( popup ) {
		popup->setFirstItemActive();
		openActPopup();
	    }
	    else {
		if ( mi->id() >= 0 )
		    emit activated( mi->id() );
	    }
	}
    }
}


void QMenuBar::resizeEvent( QResizeEvent * )
{
    badSize = TRUE;
}
