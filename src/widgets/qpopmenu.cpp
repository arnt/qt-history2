/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopmenu.cpp#6 $
**
** Implementation of QButton class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  INCLUDE_MENUITEM_DEF
#include "qpopmenu.h"
#include "qmenubar.h"
#include "qpainter.h"
#include "qscrbar.h"				// qDrawMotifArrow
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qpopmenu.cpp#6 $";
#endif


// Motif style colors

static QColor normalColor( 0x72, 0x9e, 0xff );
static QColor darkColor  ( 0x3d, 0x55, 0x8e );
static QColor lightColor ( 0xc7, 0xd7, 0xff );


// Motif style parameters

static const motifPopupFrame	= 2;		// popup frame width
static const motifItemFrame	= 2;		// menu item frame width
static const motifSepHeight	= 2;		// separator item height
static const motifItemHMargin	= 3;		// menu item hor text margin
static const motifItemVMargin	= 8;		// menu item ver text margin
static const motifArrowHMargin	= 6;		// arrow horizontal margin

/*

+-----------------------------
|      motifPopupFrame
|   +-------------------------
|   |      motifItemFrame
|   |   +---------------------
|   |   |
|   |   |			   \
|   |   |   ^   T E X T	  ^	    | motifItemVMargin
|   |   |   |		  |	   /
|   |	    motifItemHMargin
|

*/


// ---------------------------------------------------------------------------
// QPopupMenu member functions
//

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
	: QTableWidget( 0, name, WType_Popup )
{
    initMetaObject();
    isPopup = TRUE;
    topLevel = TRUE;
    badSize = TRUE;
    setBackgroundColor( normalColor );
    setNumCols( 1 );				// set number of table columns
    setNumRows( 0 );				// set number of table rows
    setClipCellPainting( FALSE );		// don't clip when painting tbl
    setTopMargin( motifPopupFrame );		// reserve space for frame
    setBottomMargin( motifPopupFrame );
    setLeftMargin( motifPopupFrame );
    setRightMargin( motifPopupFrame );
}


void QPopupMenu::menuContentsChanged()
{
    badSize = TRUE;
    if ( isVisible() ) {
	updateSize();
	repaint();
    }
}

void QPopupMenu::menuStateChanged()
{
    repaint();
}

void QPopupMenu::menuInsSubMenu( QPopupMenu *sub )
{
    sub->parentMenu = this;
    sub->topLevel = FALSE;			// it is not a top level popup
    connect( sub, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( sub, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}

void QPopupMenu::menuDelSubMenu( QPopupMenu *sub )
{
    sub->parentMenu = 0;
    sub->topLevel = TRUE;			// it becomes a top level popup
    sub->disconnect( SIGNAL(activated(int)), this, SLOT(subActivated(int)) );
    sub->disconnect( SIGNAL(selected(int)),  this, SLOT(subSelected(int)) );
}


void QPopupMenu::popup( const QPoint &pos )	// open popup menu at pos
{
    if ( mitems->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( badSize )
	updateSize();
    QWidget *desktop = QApplication::desktop();
    int sw = desktop->clientSize().width();	// screen width
    int sh = desktop->clientSize().height();	// screen height
    int x = pos.x();
    int y = pos.y();
    int w = clientSize().width();
    int h = clientSize().height();
    if ( x+w > sw )				// the complete widget must
	x = sw - w;				//   be visible
    if ( y+h > sh )
	y = sh - h;
    if ( x < 0 )
	x = 0;
    if ( y < 0 )
	y = 0;
    move( x, y );
    show();
}


void QPopupMenu::subActivated( int id )
{
    emit activated( id );
}

void QPopupMenu::subSelected( int id )
{
    emit selected( id );
}


void QPopupMenu::hideAllMenus()			// hide all menus
{
    QMenuData *top = this;			// find top level
    while ( top->parentMenu && top->parentMenu->isPopup )
	top = top->parentMenu;
    ((QPopupMenu*)top)->hide();			// cascade from top level
}

void QPopupMenu::hideSubMenus()			// hide all sub menus
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
}


int QPopupMenu::itemAtPos( const QPoint &pos )	// get item at pos (x,y)
{
    long row = findRow( pos.y() );		// ask table for row
    long col = findCol( pos.x() );		// ask table for column
    int r = -1;
    if ( row != -1 && col != -1 ) {
	QMenuItem *mi = mitems->at((int)row);
	if ( !(mi->isSeparator() || mi->isDisabled()) )
	    r = (int)row;			// normal item
    }
    return r;
}


void QPopupMenu::updateSize()			// update popup size params
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
	    warning( "QPopupMenu: Popup has invalid menu item" );
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
    badSize = FALSE;
}


void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    if ( isVisible() )
	update();
}

void QPopupMenu::show()
{
    if ( badSize )
	updateSize();
    QWidget::show();
    raise();
    firstMouseUp = FALSE;
}

void QPopupMenu::hide()
{
    actItem = -1;
    hideSubMenus();
    killTimers();
    QWidget::hide();
}


// ---------------------------------------------------------------------------
// Implementation of virtual QTableWidget functions
//

int QPopupMenu::cellHeight( long row )
{
    QMenuItem *mi = mitems->at( row );
    int h = 0;					// default cell height
    ASSERT( mi );
    if ( mi->isSeparator() )			// separator height
	h = motifSepHeight;
    else if ( mi->bitmap() )			// bitmap height
	h = mi->bitmap()->size().height() + 2*motifItemFrame;
    else {					// text height
	QFontMetrics fm( font() );
	h = fm.ascent() + motifItemVMargin + 2*motifItemFrame;
    }
    return h;
}

int QPopupMenu::cellWidth( long col )
{
    return clientSize().width() - 2*motifPopupFrame;
}


void QPopupMenu::paintCell( QPainter *p, long row, long col )
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
	int dim = (cellh-2*motifItemFrame)/2;
	qDrawMotifArrow( p, MotifRightArrow, row == actItem,
			 cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			 dim, dim,
			 normalColor, normalColor,
			 lightColor, darkColor );
    }
}


// ---------------------------------------------------------------------------
// Event handlers
//

void QPopupMenu::paintEvent( QPaintEvent *e )	// paint popup menu
{
    QPainter paint;
    paint.begin( this );			// draw the popup frame
    paint.drawShadePanel( clientRect(), lightColor, darkColor,
			  motifPopupFrame, motifPopupFrame );
    paint.end();
    QTableWidget::paintEvent( e );		// will draw the menu items
}


void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	hide();
	return;
    }
    QMenuItem *mi = item >= 0 ? mitems->at(item) : 0;
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	if ( actItem >= 0 && mi->id() >= 0 )
	    emit activated( mi->id() );
    }
    if ( mi && mi->popup() ) {
	if ( !mi->popup()->isVisible() )
	    startTimer( 200 );
    }
    else
	hideSubMenus();
}

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    actItem = itemAtPos( e->pos() );
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	QMenuItem *mi = mitems->at(actItem);
	if ( !mi->popup() ) {			// it's not another popup
	    hideAllMenus();
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    else				// normal connection
		emit selected( mi->id() );
	}
    }
    else {
	if ( topLevel ) {
	    if ( firstMouseUp )
		hide();
	    firstMouseUp = FALSE;
	}
	else					// hide sub menu
	    hide();
    }
}

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no item
	if ( !tryMenuBar( e ) ) {		// menu bar didn't want it
	    if ( clientRect().contains( e->pos() ) )
		hideSubMenus();
	}
    }
    else {
	QMenuItem *mi = mitems->at( item );
	bool hide_sub = TRUE;
	if ( mi->popup() ) {
	    if ( mi->popup()->isVisible() )
		hide_sub = FALSE;
	    else
		startTimer( 200 );
	}
	if ( hide_sub )	    
	    hideSubMenus();
	if ( item != actItem ) {		// new item activated
	    actItem = item;
	    repaint( FALSE );
	    if ( mi->id() >= 0 )		// valid identifier
		emit activated( mi->id() );
	}
    }
}


bool QPopupMenu::tryMenuBar( QMouseEvent *e )
{
    QMenuData *top = this;			// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    if ( top->isMenuBar )
	return ((QMenuBar *)top)->tryMouseEvent( this, e );
    else
	return FALSE;
}


void QPopupMenu::timerEvent( QTimerEvent *e )
{
    killTimer( e->timerId() );			// single-shot timer
    if ( actItem < 0 )
	return;
    QMenuItem *mi = mitems->at( actItem );
    if ( mi->popup() ) {
	QPoint pos( clientSize().width()-motifArrowHMargin, motifPopupFrame );
	for ( int i=0; i<actItem; i++ )
	    pos.ry() += cellHeight( i );
	mi->popup()->popup( mapToGlobal(pos) );
    }
}
