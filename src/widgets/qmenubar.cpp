/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#2 $
**
** Implementation of QButton class
**
** Author  : Haavard Nord
** Created : 941209
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  INCLUDE_MENUITEM_DEF
#include "qmenubar.h"
#include "qpainter.h"
#include "qscrbar.h"				// qDrawMotifArrow
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qmenubar.cpp#2 $";
#endif


// Motif style colors

static QColor normalColor( 0x72, 0x9e, 0xff );
static QColor darkColor  ( 0x3d, 0x55, 0x8e );
static QColor lightColor ( 0xc7, 0xd7, 0xff );


// Motif style parameters

static const motifBarFrame	= 2;		// menu bar frame width
static const motifBarHMargin	= 2;		// menu bar hor margin to item
static const motifBarVMargin	= 2;		// menu bar ver margin to item
static const motifItemFrame	= 2;		// menu item frame width
static const motifItemHMargin	= 4;		// menu item hor text margin
static const motifItemVMargin	= 8;		// menu item ver text margin

/*

+-----------------------------
|      motifBarFrame
|   +-------------------------
|   |      V  motifBarMargin
|   |   +---------------------
|   | H |      motifItemFrame
|   |   |  +-----------------
|   |   |  |			   \
|   |   |  |  ^  T E X T   ^	    | motifItemVMargin
|   |   |  |  |		   |	   /
|   |	|    motifItemHMargin
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
    setBackgroundColor( normalColor );
    if ( parent )				// filter parent events
	parent->insertEventFilter( this );
}

QMenuBar::~QMenuBar()
{
    if ( parent() )
	parent()->removeEventFilter( this );
}


void QMenuBar::menuContentsChanged()
{
    repaint();
}

void QMenuBar::menuStateChanged()
{
    repaint();
}

void QMenuBar::menuInsSubMenu( QPopupMenu *sub )
{
    sub->parentMenu = this;
    sub->topLevel = FALSE;			// it is not a top level
    connect( sub, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( sub, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}

void QMenuBar::menuDelSubMenu( QPopupMenu *sub )
{
    sub->parentMenu = this;
    sub->topLevel = TRUE;			// it becomes a top level popup
    connect( sub, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( sub, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}

bool QMenuBar::eventFilter( QObject *object, QEvent *event )
{
    ASSERT( object == parent() );
    if ( event->type() == Event_Resize ) {
	QResizeEvent *e = (QResizeEvent *)event;
	resize( e->size().width(), clientSize().height() );
	delete itemRects();
	repaint( TRUE );
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


void QMenuBar::hideAllMenus()			// hide all menus
{
}

void QMenuBar::hideSubMenus()			// hide all sub menus
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
}


int QMenuBar::itemAtPos( const QPoint &pos )	// get item at pos (x,y)
{
    QRect *rv = itemRects();
    if ( !rv )
	return -1;
    int i = 0;
    while ( i < mitems->count() ) {
	if ( rv[i].contains( pos ) )
	    break;
	++i;
    }
    delete rv;
    return i < mitems->count() ? i : -1;
}


void QMenuBar::setFont( const QFont &font )
{
    QWidget::setFont( font );
    delete itemRects();				// same as updateSize
    update();
}

void QMenuBar::show()
{
    if ( parentWidget() )
	resize( parentWidget()->clientSize().width(), clientSize().height() );
    QWidget::show();
    raise();
}

void QMenuBar::hide()
{
    actItem = -1;
    hideSubMenus();
    killTimers();
    QWidget::hide();
}


// ---------------------------------------------------------------------------
// Item geometry functions
//

QRect *QMenuBar::itemRects()
{
    if ( mitems->isEmpty() )
	return 0;
    QRect *rv = new QRect[ mitems->count() ];
    QFontMetrics fm( font() );
    int max_width = clientSize().width();
    int max_height = 0;
    int nlines = 1;				// number of lines
    int nlitems = 0;				// number on items on cur line
    int wline = 0;				// width of line
    int x = motifBarFrame + motifBarHMargin;
    int y = motifBarFrame + motifBarVMargin;
    int i = 0;
    while ( i < mitems->count() ) {		// for each menu item...
	QMenuItem *mi = mitems->at(i);
	int w, h;
	if ( mi->bitmap() ) {			// bitmap item
	    w = mi->bitmap()->size().width();
	    h = mi->bitmap()->size().height();
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
	rv[i].setRect( x, y, w, h );
	x += w;
	nlitems++;
	i++;
    }
    if ( max_height != clientSize().height() )
	resize( max_width, max_height );
    return rv;
}

QRect QMenuBar::itemRect( int item )
{
    QRect *rv = itemRects();
    QRect r;
    if ( rv ) {
	r = rv[item];
	delete rv;
    }
    else
	r.setRect( 0, 0, 0, 0 );
    return r;
}


// ---------------------------------------------------------------------------
// Event handlers
//

void QMenuBar::paintEvent( QPaintEvent *e )	// paint menu bar
{
    QPainter paint;
    register QPainter *p = &paint;
    QFontMetrics fm( font() );
    QSize sz = clientSize();
    p->begin( this );				// draw the menu bar frame
    
    p->drawShadePanel( clientRect(), lightColor, darkColor,
		       motifBarFrame, motifBarFrame );
    p->setClipRect( motifBarFrame, motifBarFrame,
		    sz.width()  - 2*motifBarFrame,
		    sz.height() - 2*motifBarFrame );
    QRect *rv = itemRects();
    QPen pen( foregroundColor(), 2 );
    p->setPen( pen );
    for ( int i=0; i<mitems->count(); i++ ) {
	QMenuItem *mi = mitems->at( i );
	QRect r = rv[i];
	if ( i == actItem )			// active item frame
	    p->drawShadePanel( r, lightColor, darkColor,
			       motifItemFrame, motifItemFrame );
	else					// incognito frame
	    p->drawShadePanel( r, normalColor, normalColor,
			       motifItemFrame, motifItemFrame );
	if ( mi->bitmap() ) {
	    p->drawPixMap( r.left() + motifItemFrame,
			   r.top() + motifItemFrame,
			   *mi->bitmap() );
	}
	else if ( mi->string() ) {
	    int bo = fm.descent() + motifItemVMargin/2;
	    int w = fm.width( mi->string() );
	    p->drawText( r.left() + r.width()/2 - fm.width(mi->string())/2,
			 r.bottom() - bo,
			 mi->string() );
	}
    }
    delete rv;
    p->end();
}


void QMenuBar::mousePressEvent( QMouseEvent *e )
{
    int item = itemAtPos( e->pos() );
    QMenuItem *mi = item >= 0 ? mitems->at(item) : 0;
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	if ( actItem >= 0 && mi->id() >= 0 )
	    emit activated( mi->id() );
    }
    if ( mi && mi->popup() ) {
	if ( !mi->popup()->isVisible() ) {
	    killTimers();
	    startTimer( 10 );
	}
    }
    else
	hideSubMenus();
}

void QMenuBar::mouseReleaseEvent( QMouseEvent *e )
{
    actItem = itemAtPos( e->pos() );
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	QMenuItem *mi = mitems->at(actItem);
	if ( mi->popup() ) {
	    QPoint pos = itemRect( actItem ).bottomLeft() + QPoint(0,1);
	    mi->popup()->actItem = 0;		// NOTE!!! if separator...
	    mi->popup()->popup( mapToGlobal(pos) );
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
    QMenuItem *mi = item >= 0 ? mitems->at(item) : 0;
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	if ( actItem >= 0 && mi->id() >= 0 )
	    emit activated( mi->id() );
    }
    if ( mi && mi->popup() ) {
	if ( !mi->popup()->isVisible() )
	    startTimer( 10 );
    }
}

bool QMenuBar::tryMouseEvent( QPopupMenu *popup, QMouseEvent *e )
{
    QPoint pos = mapFromGlobal( popup->mapToGlobal( e->pos() ) );
    int item = itemAtPos( pos );
    if ( item >= 0 ) {
	QMenuItem *mi = mitems->at( item );
	if ( popup == mi->popup() )		// popup already open
	    return FALSE;
	popup->hide();				// hide popup
	repaint( FALSE );
	if ( mi->popup() ) {
	    pos = itemRect( actItem ).bottomLeft() + QPoint(0,1);
	    mi->popup()->popup( mapToGlobal(pos) );
	}
	return TRUE;
    }
    return FALSE;
}


void QMenuBar::resizeEvent( QResizeEvent * )
{
}


void QMenuBar::timerEvent( QTimerEvent *e )
{
    killTimer( e->timerId() );			// single-shot timer
    if ( actItem < 0 )
	return;
    QMenuItem *mi = mitems->at( actItem );
    if ( mi->popup() ) {
	QPoint pos = itemRect( actItem ).bottomLeft() + QPoint(0,1);
	mi->popup()->popup( mapToGlobal(pos) );
    }
}
