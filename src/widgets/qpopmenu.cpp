/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopmenu.cpp#14 $
**
** Implementation of QPopupMenu class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  INCLUDE_MENUITEM_DEF
#include "qpopmenu.h"
#include "qmenubar.h"
#include "qkeycode.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qscrbar.h"				// qDrawMotifArrow
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qpopmenu.cpp#14 $";
#endif


// Motif style colors

static QColor normalColor( 0x72, 0x9e, 0xff );
static QColor darkColor  ( 0x3d, 0x55, 0x8e );
static QColor lightColor ( 0xc7, 0xd7, 0xff );


// Mac style parameters

static const macPopupFrame	= 1;		// popup frame width
static const macItemFrame	= 2;		// menu item frame width
static const macSepHeight	= 2;		// separator item height
static const macItemHMargin	= 3;		// menu item hor text margin
static const macItemVMargin	= 8;		// menu item ver text margin
static const macArrowHMargin	= 6;		// arrow horizontal margin
static const macArrowVMargin	= 4;		// arrow horizontal margin

// Windows style parameters

static const winPopupFrame	= 2;		// popup frame width
static const winItemFrame	= 2;		// menu item frame width
static const winSepHeight	= 2;		// separator item height
static const winItemHMargin	= 3;		// menu item hor text margin
static const winItemVMargin	= 8;		// menu item ver text margin
static const winArrowHMargin= 6;		// arrow horizontal margin
static const winArrowVMargin= 4;		// arrow horizontal margin

// PM style parameters

static const pmPopupFrame	= 2;		// popup frame width
static const pmItemFrame	= 2;		// menu item frame width
static const pmSepHeight	= 2;		// separator item height
static const pmItemHMargin	= 3;		// menu item hor text margin
static const pmItemVMargin	= 8;		// menu item ver text margin
static const pmArrowHMargin	= 6;		// arrow horizontal margin
static const pmArrowVMargin	= 4;		// arrow horizontal margin

// Motif style parameters

static const motifPopupFrame	= 2;		// popup frame width
static const motifItemFrame	= 2;		// menu item frame width
static const motifSepHeight	= 2;		// separator item height
static const motifItemHMargin	= 3;		// menu item hor text margin
static const motifItemVMargin	= 8;		// menu item ver text margin
static const motifArrowHMargin	= 6;		// arrow horizontal margin
static const motifArrowVMargin	= 4;		// arrow horizontal margin
static const motifTabSpacing	= 12;		// space between text and tab

static char sizePopupFrame[] =
    { macPopupFrame, winPopupFrame, pmPopupFrame, motifPopupFrame };

/*

+-----------------------------
|      PopupFrame
|   +-------------------------
|   |      ItemFrame
|   |   +---------------------
|   |   |
|   |   |			   \
|   |   |   ^   T E X T	  ^	    | ItemVMargin
|   |   |   |		  |	   /
|   |	      ItemHMargin
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
    setBackgroundColor( normalColor );
    setNumCols( 1 );				// set number of table columns
    setNumRows( 0 );				// set number of table rows
    setClipCellPainting( FALSE );		// don't clip when painting tbl
    setTopMargin( motifPopupFrame );		// reserve space for frame
    setBottomMargin( motifPopupFrame );
    setLeftMargin( motifPopupFrame );
    setRightMargin( motifPopupFrame );
    popupActive = -1;
    tabMark = 0;
}

QPopupMenu::~QPopupMenu()
{
    if ( parentMenu )				// remove from parent menu
	parentMenu->removePopup( this );
}


void QPopupMenu::updateItem( int id )		// update popup menu item
{
    updateCell( indexOf(id), 0, FALSE );
}

void QPopupMenu::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    if ( isVisible() ) {
	updateSize();
	repaint();
    }
}

void QPopupMenu::menuStateChanged()
{
    repaint();
}

void QPopupMenu::menuInsPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;			// set parent menu
    connect( popup, SIGNAL(activated(int)), SLOT(subActivated(int)) );
    connect( popup, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
}

void QPopupMenu::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = 0;
    popup->disconnect( SIGNAL(activated(int)), this, SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(selected(int)),  this, SLOT(subSelected(int)) );
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


void QPopupMenu::setFirstItemActive()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    actItem = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( !(mi->isSeparator() || mi->isDisabled()) ) {
	    repaint( FALSE );
	    return;
	}
	actItem++;
    }
    actItem = -1;
}

void QPopupMenu::hideAllPopups()		// hide all popup menus
{
    register QMenuData *top = this;		// find top level popup
    while ( top->parentMenu && top->parentMenu->isPopup )
	top = top->parentMenu;
    ((QPopupMenu*)top)->hide();			// cascade from top level
}

void QPopupMenu::hidePopups()			// hide popup items
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
    popupActive = -1;				// no active sub menu
}


bool QPopupMenu::tryMenuBar( QMouseEvent *e )	// send event to menu bar
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    return top->isMenuBar ?
	((QMenuBar *)top)->tryMouseEvent( this, e ) : FALSE;
}

void QPopupMenu::byeMenuBar()			// tell menubar to deactivate
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    if ( top->isMenuBar )
	((QMenuBar *)top)->goodbye();
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
    int height    = 0;
    int max_width = 10;
    QFontMetrics fm = fontMetrics();
    QMenuItemListIt it( *mitems );
    register QMenuItem *mi;
    bool hasSubMenu = FALSE;
    int cellh = fm.ascent() + motifItemVMargin + 2*motifItemFrame;
    int tab_width = 0;
    debug( "updateSize()" );	// !!! DEBUG
    while ( (mi=it.current()) ) {
	int w = 0;
	if ( mi->popup() )
	    hasSubMenu = TRUE;
	if ( mi->isSeparator() )
	    height += motifSepHeight;
	else if ( mi->image() ) {
	    height += mi->image()->height() + 2*motifItemFrame;
	    w = mi->image()->width();
	}
	else if ( mi->string() ) {
	    height += cellh;
	    const char *s = mi->string();
	    char *t = strchr( s, '\t' );
	    if ( t ) {				// string contains tab
		w = fm.width( s, (int)t-(int)s );
		int tw = fm.width( t+1 );
		if ( tw > tab_width )
		    tab_width = tw;
	    }
	    else
		w = fm.width( s );
	}
#if defined(CHECK_NULL)
	else
	    warning( "QPopupMenu: Popup has invalid menu item" );
#endif
	if ( max_width < w )
	    max_width = w;
	++it;
    }
    int extra_width = 0;
    if ( tab_width ) {
	extra_width = tab_width + motifTabSpacing;
	tabMark = max_width + motifTabSpacing;
    }
    else
	tabMark = 0;
    max_width  += 2*motifItemHMargin + 2*motifItemFrame;
    if ( hasSubMenu ) {
	if ( fm.ascent() + motifArrowHMargin > extra_width )
	    extra_width = fm.ascent() + motifArrowHMargin;
    }
    max_width += extra_width;
    setNumRows( mitems->count() );
    resize( max_width+2*motifPopupFrame, height+2*motifPopupFrame );
    badSize = FALSE;
}


void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    update();
}

void QPopupMenu::show()
{
    if ( badSize )
	updateSize();
    QWidget::show();
    raise();
    popupActive = -1;
}

void QPopupMenu::hide()
{
    actItem = popupActive = -1;
    hidePopups();
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
    if ( mi->isSeparator() )			// separator height
	h = motifSepHeight;
    else if ( mi->image() )			// image height
	h = mi->image()->height() + 2*motifItemFrame;
    else {					// text height
        QFontMetrics fm = fontMetrics();
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
    int gs = style();
    if ( mi->isSeparator() ) {			// draw separator
	QPen pen( darkColor );	
	p->setPen( pen );
	p->drawLine( 0, 0, cellw, 0 );
	pen.setColor( lightColor );
	p->drawLine( 0, 1, cellw, 1 );
	return;
    }
    if ( row == actItem )			// active item frame
	p->drawShadePanel( 0, 0, cellw, cellh, lightColor, darkColor,
			   motifItemFrame );
    else					// incognito frame
	p->drawShadePanel( 0, 0, cellw, cellh, normalColor, normalColor,
			   motifItemFrame );
    if ( mi->image() )				// draw image
	p->drawPixMap( motifItemFrame + motifItemHMargin, motifItemFrame,
		       *mi->image() );
    else if ( mi->string() ) {			// draw text
	const char *s = mi->string();
	const char *t = strchr( s, '\t' );
	int x = motifItemFrame + motifItemHMargin;
        QFontMetrics fm = fontMetrics();
	int bo = fm.descent() + motifItemVMargin/2;
	if ( mi->isDisabled() )
	    p->setPen( darkGray );
	if ( t ) {				// make tab effect
	    p->drawText( x, cellh-bo, s, (int)t-(int)s );
	    s = t + 1;
	    x = tabMark;
	}
	p->drawText( x, cellh-bo, s );
    }
    if ( mi->popup() ) {			// draw sub menu arrow
	int dim = (cellh-2*motifItemFrame);
	if ( gs == MacStyle ) {
	    QPointArray a;
	    a.setPoints( 3, 0,-dim/2, 0,dim/2, dim/2,0 );
	    a.move( cellw - motifArrowHMargin - dim, cellh/2-dim/2 );
	    p->setBrush( black );
	    p->setPen( NoPen );
	    p->drawPolygon( a );
	}
	else if ( gs == MotifStyle ) {
	    dim /= 2;
	    qDrawMotifArrow( p, MotifRightArrow, row == actItem,
			     cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			     dim, dim,
			     normalColor, normalColor,
			     lightColor, darkColor );
	}
    }
}


// ---------------------------------------------------------------------------
// Event handlers
//

void QPopupMenu::paintEvent( QPaintEvent *e )	// paint popup menu
{
    QPainter paint;
    paint.begin( this );			// draw the popup frame
    QRect r = clientRect();
    switch ( style() ) {
	case MacStyle:
	    paint.drawShadePanel( r, black, black, macPopupFrame );
	    break;
	case MotifStyle:
	    paint.drawShadePanel( r, lightColor, darkColor, motifPopupFrame );
	    break;
    }
    paint.end();
    QTableWidget::paintEvent( e );		// will draw the menu items
}


void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !clientRect().contains( e->pos() ) && !tryMenuBar( e ) ) {
	    hide();
	    byeMenuBar();
	}
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
	if ( popup->isVisible() ) {		// sub menu already open
	    popup->actItem = -1;
	    popup->hidePopups();
	    popup->repaint( FALSE );
	}
	else {					// open sub menu
	    hidePopups();
	    killTimers();
	    startTimer( 20 );
	}
    }
    else
	hidePopups();
}

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    mouseBtDn = FALSE;				// mouse button up
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !clientRect().contains( e->pos() ) && tryMenuBar( e ) )
	    return;
    }
    actItem = item;
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	register QMenuItem *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	if ( popup && style() != MacStyle )
	    popup->setFirstItemActive();
	else {					// normal menu item
	    hideAllPopups();			// hide all popup
	    byeMenuBar();			// deactivate menu bar
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    else				// normal connection
		emit selected( mi->id() );
	}
    }
    else {
	hideAllPopups();
	byeMenuBar();
    }
}

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    int  item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no valid item
	if ( popupActive == -1 ) {		// no active popup sub menu
	    int lastActItem = actItem;
	    actItem = -1;
	    if ( lastActItem >= 0 )
		updateCell( lastActItem, 0, FALSE );
	}
	if ( !clientRect().contains( e->pos() ) && !tryMenuBar( e ) )
	    hidePopups();
    }
    else {					// mouse on valid item
	register QMenuItem *mi = mitems->at( item );
	QPopupMenu *popup = mi->popup();
	if ( actItem == item ) {
	    if ( popupActive == item ) {
		popup->actItem = -1;
		popup->hidePopups();
		popup->repaint( FALSE );
	    }
	    return;
	}
	int lastActItem = actItem;
	actItem = item;
	if ( mi->popup() ) {
	    killTimers();
	    startTimer( 100 );			// open new popup soon
	}
	hidePopups();				// hide popup items
	if ( lastActItem >= 0 )
	    updateCell( lastActItem, 0, FALSE );
	updateCell( actItem, 0, FALSE );
	if ( mi->id() >= 0 )			// valid identifier
	    emit activated( mi->id() );
    }
}


void QPopupMenu::keyPressEvent( QKeyEvent *e )
{
    if ( mouseBtDn )				// cannot handle key event
	return;

    QMenuItem  *mi;
    QPopupMenu *popup;
    int d = 0;
    bool ok_key = TRUE;

    switch ( e->key() ) {
	case Key_Up:
	    d = -1;
	    break;

	case Key_Down:
	    d = 1;
	    break;

	case Key_Escape:
	    hideAllPopups();
	    byeMenuBar();
	    break;

	case Key_Return:
	case Key_Enter:
	    if ( actItem < 0 )
		break;
	    mi = mitems->at( actItem );
	    popup = mi->popup();
	    if ( popup ) {
		hidePopups();
		killTimers();
		startTimer( 20 );
		popup->setFirstItemActive();
	    }
	    else {
		hideAllPopups();
		byeMenuBar();
		if ( mi->signal() )
		    mi->signal()->activate();
		else
		    emit selected( mi->id() );
	    }
	    break;

	default:
	    ok_key = FALSE;

    }

    if ( !ok_key ) {				// send to menu bar
	register QMenuData *top = this;	// find top level
	while ( top->parentMenu )
	    top = top->parentMenu;
	if ( top->isMenuBar )
	    ((QMenuBar*)top)->tryKeyEvent( this, e );
    }

    if ( d && actItem >= 0 ) {			// highlight next/prev
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
	    int lastActItem = actItem;
	    actItem = i;
	    if ( mi->id() >= 0 )
		emit activated( mi->id() );
	    updateCell( lastActItem, 0, FALSE );
	    updateCell( actItem, 0, FALSE );
	}
    }
}


void QPopupMenu::timerEvent( QTimerEvent *e )	// open sub menu
{
    killTimer( e->timerId() );			// single-shot timer
    if ( actItem < 0 )
	return;
    QPopupMenu *popup = mitems->at(actItem)->popup();
    if ( popup ) {				// it is a popup
	QPoint pos( clientSize().width() - motifArrowHMargin,
		    motifPopupFrame + motifArrowVMargin );
	for ( int i=0; i<actItem; i++ )
	    pos.ry() += cellHeight( i );
	popupActive = actItem;
	popup->popup( mapToGlobal(pos) );
    }
}
