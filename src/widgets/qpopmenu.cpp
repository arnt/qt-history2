/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopmenu.cpp#5 $
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
#include "qpainter.h"
#include "qscrbar.h"				// qDrawMotifArrow
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qpopmenu.cpp#5 $";
#endif


// Motif colors

QColor normalColor( 0x72, 0x9e, 0xff );
QColor darkColor  ( 0x3d, 0x55, 0x8e );
QColor lightColor ( 0xc7, 0xd7, 0xff );

// Motif measurements

const motifPopupFrame	= 2;
const motifItemFrame	= 2;
const motifSepHeight	= 2;			// height of separator
const motifItemHMargin	= 3;			// horizontal text margin
const motifItemVMargin	= 8;			// extra space between items
const motifArrowHMargin	= 6;			// horizontal arrow margin


// ---------------------------------------------------------------------------
// QPopupMenu member functions
//

#define PM_TOP_LEVEL	  0x1			// popup menu flags
#define PM_BAD_SIZE	  0x2
#define PM_FIRST_MOUSE_UP 0x4

#define PMTopLevel	  ((pmflags & PM_TOP_LEVEL) == PM_TOP_LEVEL)
#define PMBadSize	  ((pmflags & PM_BAD_SIZE) == PM_BAD_SIZE)
#define PMFirstMouseUp	  ((pmflags & PM_FIRST_MOUSE_UP) == PM_FIRST_MOUSE_UP)

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
	: QTableWidget( 0, name, WType_Popup )
{
    popupParent = 0;				// this is a top level popup
    actItem = -1;
    pmflags = PM_TOP_LEVEL | PM_BAD_SIZE;	// init flags
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
    pmflags |= PM_BAD_SIZE;
    if ( isVisible() ) {
	updateSize();
	repaint();
    }
}

void QPopupMenu::menuStateChanged()
{
    repaint();
}

void QPopupMenu::menuInitSubMenu( QPopupMenu *sub )
{
    sub->popupParent = this;
    sub->pmflags &= ~PM_TOP_LEVEL;		// it is not a top level
    connect( sub, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( sub, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}


void QPopupMenu::popup( const QPoint &pos )	// open popup menu at pos
{
    if ( mitems->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( PMBadSize )
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
    QPopupMenu *popup = this;			// find top level
    while ( popup->popupParent && popup->popupParent->testFlag(WType_Popup) )
	popup = (QPopupMenu*)popup->popupParent;
    popup->hide();				// cascade from top level
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
    pmflags &= ~PM_BAD_SIZE;
}


void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    pmflags |= PM_BAD_SIZE;
    if ( isVisible() )
	update();
}

void QPopupMenu::show()
{
    if ( PMBadSize )
	updateSize();
    QWidget::show();
    raise();
    pmflags |= PM_FIRST_MOUSE_UP;
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
	int dim = cellh*3/5;
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
    if ( !clientRect().contains( e->pos() ) ) {
	hide();
	return;
    }
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
	if ( PMTopLevel ) {
	    if ( !PMFirstMouseUp )
		hide();
	    pmflags &= ~PM_FIRST_MOUSE_UP;
	}
	else					// hide sub menu
	    hide();
    }
}

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
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
	    startTimer( 200 );
    }
    else
	hideSubMenus();
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
