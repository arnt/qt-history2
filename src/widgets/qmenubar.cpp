/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#1 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qmenubar.cpp#1 $";
#endif


// Motif colors

static QColor normalColor( 0x72, 0x9e, 0xff );
static QColor darkColor  ( 0x3d, 0x55, 0x8e );
static QColor lightColor ( 0xc7, 0xd7, 0xff );

// Motif measurements

static const motifMenuFrame	= 2;
static const motifItemFrame	= 2;
static const motifSepHeight	= 2;		// height of separator
static const motifItemHMargin	= 3;		// horizontal text margin
static const motifItemVMargin	= 8;		// extra space between items
static const motifArrowHMargin	= 6;		// horizontal arrow margin


// ---------------------------------------------------------------------------
// QPopupMenu member functions
//

#define PM_TOP_LEVEL	  0x1			// popup menu flags
#define PM_BAD_SIZE	  0x2
#define PM_FIRST_MOUSE_UP 0x4

#define PMTopLevel	  ((pmflags & PM_TOP_LEVEL) == PM_TOP_LEVEL)
#define PMBadSize	  ((pmflags & PM_BAD_SIZE) == PM_BAD_SIZE)
#define PMFirstMouseUp	  ((pmflags & PM_FIRST_MOUSE_UP) == PM_FIRST_MOUSE_UP)

QMenuBar::QMenuBar( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    actItem = -1;
//    pmflags = PM_TOP_LEVEL | PM_BAD_SIZE;	// init flags
    setBackgroundColor( normalColor );
    if ( parent )
	parent->insertEventFilter( this );
}

QMenuBar::~QMenuBar()
{
    if ( parent() )
	parent()->removeEventFilter( this );
}


void QMenuBar::menuContentsChanged()
{
//    pmflags |= PM_BAD_SIZE;
    if ( isVisible() ) {
//	updateSize();
	repaint();
    }
}

void QMenuBar::menuStateChanged()
{
    repaint();
}

void QMenuBar::menuInitSubMenu( QPopupMenu *sub )
{
    sub->popupParent = this;
    sub->pmflags &= ~PM_TOP_LEVEL;		// it is not a top level
    connect( sub, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( sub, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}

bool QMenuBar::eventFilter( QObject *object, QEvent *event )
{
    ASSERT( object == parent() );
    if ( event->type() == Event_Resize ) {
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


void QMenuBar::hideAllMenus()			// hide all menus
{
/*
    QMenuBar *popup = this;			// find top level
    while ( popup->popupParent && popup->popupParent->testFlag(WType_Popup) )
	popup = (QMenuBar*)popup->popupParent;
    popup->hide();				// cascade from top level
*/
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

#if 0
void QMenuBar::updateSize()			// update popup size params
{
    int height     = 0;
    int max_width  = 10;
    QFontMetrics    fm( font() );
    QMenuItemListIt it( *mitems );
    QMenuItem      *mi;
    bool hasSubMenu = FALSE;
    int cellh = fm.ascent() + motifItemVMargin + 2*motifItemFrame;

    while ( (mi=it.current()) ) {
	int w = 0;
	if ( mi->popup() )
	    hasSubMenu = TRUE;
	if ( mi->isSeparator() )
	    height += motifSepHeight;
	else if ( mi->bitmap() ) {
	    height += mi->bitmap()->size().height() + 2*motifItemFrame;
	    w = mi->bitmap()->size().width();
	}
	else if ( mi->string() ) {
	    height += cellh;
	    w = fm.width(mi->string());
	}
#if defined(CHECK_NULL)
	else
	    warning( "QMenuBar: Popup has invalid menu item" );
#endif
	if ( max_width < w )
	    max_width = w;
	++it;
    }
    max_width  += 2*motifItemHMargin + 2*motifItemFrame;
    if ( hasSubMenu )
	max_width += fm.ascent() + motifArrowHMargin;
    setNumRows( mitems->count() );
    resize( max_width+2*motifPopupFrame, height+2*motifPopupFrame );
    pmflags &= ~PM_BAD_SIZE;
}
#endif

void QMenuBar::setFont( const QFont &font )
{
    QWidget::setFont( font );
    mbflags |= PM_BAD_SIZE;
    if ( isVisible() )
	update();
}

void QMenuBar::show()
{
    if ( parentWidget() )
	resize( parentWidget()->clientSize().width(), clientSize().height() );
//    if ( PMBadSiza )
//	updateSize();
    QWidget::show();
    raise();
//    pmflags |= PM_FIRST_MOUSE_UP;
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
    int x = 0;
    int y = 0;
    int i = 0;
    while ( i < mitems->count() ) {		// for each menu item...
	QMenuItem *mi = mitems->at(i);
	int w, h;
	if ( mi->bitmap() ) {			// bitmap item
	    w = mi->bitmap()->size().width();
	    h = mi->bitmap()->size().height();
	}
	else {
	    w = fm.width( mi->string() );
	    h = fm.ascent() + motifItemVMargin;
	}
	w += 2*motifItemFrame + 2*motifItemHMargin;
	h += 2*motifItemFrame;
	if ( y + h + 2*motifMenuFrame > max_height )
	    max_height = y + h + 2*motifMenuFrame;
	if ( x + w > max_width && nlitems > 0 ) {
	    nlines++;				// break line
	    nlitems = 0;
	    x = 0;
	    y += h;
	}
	rv[i].setRect( x, y, w, h );
	x += w + 2*motifItemHMargin;
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

#if 0

void QMenuBar::paintCell( QPainter *p, long row, long col )
{
    QMenuItem *mi = mitems->at( row );		// get menu item
    int cellh = cellHeight( row );
    int cellw = cellWidth( col );
    ASSERT( mi );
    if ( mi->isSeparator() ) {			// draw separator
	p->drawShadeLine( 0, 0, cellw, 0, darkColor, lightColor );
	return;
    }
    if ( row == actItem )			// active item frame
	p->drawShadePanel( 0, 0, cellw, cellh,
			   lightColor, darkColor,
			   motifItemFrame, motifItemFrame );
    else					// incognito frame
	p->drawShadePanel( 0, 0, cellw, cellh,
			   normalColor, normalColor,
			   motifItemFrame, motifItemFrame );
    if ( mi->bitmap() )				// draw bitmap
	p->drawPixMap( motifItemFrame + motifItemHMargin, motifItemFrame,
		       *mi->bitmap() );
    else if ( mi->string() ) {			// draw text
	QFontMetrics fm( font() );
	int bo = fm.descent() + motifItemVMargin/2;
	if ( mi->isDisabled() )
	    p->setPen( darkGray );
	p->drawText( motifItemFrame + motifItemHMargin, cellh-bo,
		     mi->string() );
	if ( mi->isDisabled() )			// restore pen
	    p->setPen( foregroundColor() );
    }
    if ( mi->popup() ) {			// draw sub menu arrow
	int dim = cellh*3/5;
	qDrawMotifArrow( p, MotifRightArrow, row == actItem,
			 cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			 dim, dim,
			 normalColor, normalColor,
			 lightColor, darkColor );
    }
}
#endif

// ---------------------------------------------------------------------------
// Event handlers
//

void QMenuBar::paintEvent( QPaintEvent *e )	// paint menu bar
{
    QPainter paint;
    register QPainter *p = &paint;
    p->begin( this );			// draw the popup frame
    p->drawShadePanel( clientRect(), lightColor, darkColor,
			  motifMenuFrame, motifMenuFrame );
    QRect *rv = itemRects();
    QPen pen( foregroundColor(), 2 );
    p->setPen( pen );
    for ( int i=0; i<mitems->count(); i++ ) {
	QRect r = rv[i];
	if ( i == actItem )			// active item frame
	    p->drawShadePanel( r, lightColor, darkColor,
			       motifItemFrame, motifItemFrame );
	else					// incognito frame
	    p->drawShadePanel( r, normalColor, normalColor,
			       motifItemFrame, motifItemFrame );
	p->drawText( r.bottomLeft() + QPoint(2,-2),
		     mitems->at(i)->string() );
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
	if ( !mi->popup() ) {			// it's not another popup
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    else				// normal connection
		emit selected( mi->id() );
	    hideAllMenus();
	}
    }
    else {
/*
	if ( PMTopLevel ) {
	    if ( !PMFirstMouseUp )
		hide();
	    pmflags &= ~PM_FIRST_MOUSE_UP;
	}
	else					// hide sub menu
	    hide();
*/
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
//    else
//	hideSubMenus();
}


void QMenuBar::timerEvent( QTimerEvent *e )
{
    killTimer( e->timerId() );			// single-shot timer
    if ( actItem < 0 )
	return;
    QMenuItem *mi = mitems->at( actItem );
    if ( mi->popup() ) {
	QPoint pos = itemRect( actItem ).bottomLeft();
	mi->popup()->popup( mapToGlobal(pos) );
    }
}
