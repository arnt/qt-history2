/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#6 $
**
** Implementation of QMenuBar class
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
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qmenubar.cpp#6 $";
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
    setBackgroundColor( normalColor );
    if ( parent )				// filter parent events
	parent->installEventFilter( this );
}

QMenuBar::~QMenuBar()
{
    delete irects;
    if ( parent() )
	parent()->removeEventFilter( this );
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
    connect( popup, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( popup, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}

void QMenuBar::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;
    connect( popup, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( popup, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}


bool QMenuBar::eventFilter( QObject *object, QEvent *event )
{
    if ( object == parent() && event->type() == Event_Resize ) {
	QResizeEvent *e = (QResizeEvent *)event;
	resize( e->size().width(), clientSize().height() );
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
    if ( !clientRect().contains( pos ) )	// outside
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
	int sh = QApplication::desktop()->clientSize().height();
	int ph = popup->clientSize().height();
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
	resize( parentWidget()->clientSize().width(), clientSize().height() );
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
	irects[i].setRect( x, y, w, h );
	x += w;
	nlitems++;
	i++;
    }
    if ( max_height != clientSize().height() )
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
	if ( irects[i].contains( pos ) )
	    return i;
	++i;
    }
    return -1;					// no match
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
    p->begin( this );
    
    p->drawShadePanel( clientRect(), lightColor, darkColor,
		       motifBarFrame, motifBarFrame );
    p->setClipRect( motifBarFrame, motifBarFrame,
		    sz.width()  - 2*motifBarFrame,
		    sz.height() - 2*motifBarFrame );
    updateRects();
    for ( int i=0; i<mitems->count(); i++ ) {
	QMenuItem *mi = mitems->at( i );
	QRect r = irects[i];
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
    p->end();
}


void QMenuBar::mousePressEvent( QMouseEvent *e )
{
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


void QMenuBar::resizeEvent( QResizeEvent * )
{
    badSize = TRUE;
}
