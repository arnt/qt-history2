/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.cpp#1 $
**
** Implementation of QButton class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  QPOPMENU_C
#include "qpopmenu.h"
#include "qsignal.h"
#include "qbitmap.h"
#include "qpainter.h"
#include "qscrbar.h"				// qDrawMotifArrow
#include "qapp.h"


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
// QMenuItem member functions
//

QMenuItem::QMenuItem()
{
    id = -1;
    isSeparator = isDisabled = isChecked = FALSE;
    bitmap = 0;
    submenu = 0;
    signal = 0;
}

QMenuItem::~QMenuItem()
{
    delete bitmap;
    delete submenu;
    delete signal;
}


// ---------------------------------------------------------------------------
// QPopupMenu member functions
//

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
	: QTableWidget( 0, name, WType_Popup )
{
    popupParent = 0;
    setBackgroundColor( normalColor );
    items = new QMenuItemList;
    activeItem = -1;
    isTopLevel = TRUE;				// assume top level popup
    badSize = TRUE;
    knownSize = QSize( 0, 0 );
    setNumCols( 1 );
    setNumRows( 0 );
    setClipCellPainting( FALSE );		// we're in charge
    setTopMargin( motifPopupFrame );
    setBottomMargin( motifPopupFrame );
    setLeftMargin( motifPopupFrame );
    setRightMargin( motifPopupFrame );
    cellh = cellw = 0;
}


void QPopupMenu::popup( const QPoint &pos, int item )
{						// open popup menu at item+pos
    if ( items->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( badSize )
	updateSize();
    QWidget *desktop = QApplication::desktop();
    int sw = desktop->clientSize().width();
    int sh = desktop->clientSize().height();
    int x = pos.x();
    int y = pos.y();
    int w = clientSize().width();
    int h = clientSize().height();
    if ( item >= 0 ) {				// add item pos to y
	if ( item >= items->count() )		// out of range
	    item = items->count() - 1;
	y -= cellHeight( item )/2;
	while ( --item >= 0 )
	    y -= cellHeight( item );
	y -= motifPopupFrame;
    }
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


void QPopupMenu::insertAny( const char *text, QBitMap *bitmap,
			    QPopupMenu *sub, int id, int index )
{						// insert bitmap + sub menu
    if ( index > (int)items->count() ) {
#if defined(CHECK_RANGE)
	warning( "QPopupMenu::insertItem: Index %d out of range", index );
#endif
	return;
    }
    if ( index == items->count() )		// append
	index = -1;
    QMenuItem *i = new QMenuItem;
    CHECK_PTR( i );
    i->id = id;
    if ( text == 0 && bitmap == 0 && sub == 0 )	// separator
	i->isSeparator = TRUE;
    else {
	i->text = text;
	i->bitmap = bitmap;
	if ( (i->submenu = sub) ) {
	    sub->popupParent = this;
	    sub->isTopLevel = FALSE;		// it is a sub menu
	    connect( sub, SIGNAL(activated(int)), SLOT(subActivated(int)) );
	    connect( sub, SIGNAL(selected(int)),  SLOT(subSelected(int)) );
	}
    }
    if ( index < 0 )
	items->append( i );
    else
	items->insert( index, i );
    badSize = TRUE;				// needs updateSize
}

void QPopupMenu::insertItem( const char *text, int id, int index )
{						// insert text item
    insertAny( text, 0, 0, id, index );
}

void QPopupMenu::insertItem( const char *text, QPopupMenu *sub, int index )
{						// insert text + sub menu
    insertAny( text, 0, sub, -1, index );
}

void QPopupMenu::insertItem( QBitMap *bitmap, int id, int index )
{						// insert bitmap item
    insertAny( 0, bitmap, 0, id, index );
}

void QPopupMenu::insertItem( QBitMap *bitmap, QPopupMenu *sub, int index )
{						// insert bitmap + sub menu
    insertAny( 0, bitmap, sub, -1, index );
}

void QPopupMenu::insertSeparator( int index )	// insert menu separator
{
    insertAny( 0, 0, 0, -1, index );
}

void QPopupMenu::removeItem( int index )	// insert menu separator
{
    if ( index < 0 || index >= items->count() ) {
#if defined(CHECK_RANGE)
	warning( "QPopupMenu::removeItem: Index %d out of range" );
#endif
	return;
    }
    items->remove( index );
    badSize = TRUE;				// needs updateSize
}


int QPopupMenu::index( int id ) const		// get index of specified item
{
    if ( id == -1 )				// invalid identifier
	return -1;
    QMenuItemListIt it( *items );
    QMenuItem *i;
    int k = 0;
    while ( (i=it.current()) ) {
	if ( i->id == id )
	    return k;
	k++;
	++it;
    }
    return -1;
}


bool QPopupMenu::isItemDisabled( int id ) const
{
    int ind = index( id );
    return ind >= 0 ? items->at(ind)->isDisabled : FALSE;
}

void QPopupMenu::setItemEnabled( int id, bool onOff )
{
    int ind = index( id );
    if ( ind >= 0 )
	items->at(ind)->isDisabled = onOff;
}


void QPopupMenu::subActivated( int id )
{
    emit activated( id );
}

void QPopupMenu::subSelected( int id )
{
    emit selected( id );
}


bool QPopupMenu::connectItem( int id, const QObject *receiver,
			      const char *member )
{
    QMenuItem *i = items->at( index(id) );
    if ( !i->signal ) {
	i->signal = new QSignal;
	CHECK_PTR( i->signal );
    }
    return i->signal->connect( receiver, member );
}


void QPopupMenu::hideAllMenus()			// hide all menus
{
    QPopupMenu *popup = this;
    while ( popup->popupParent && popup->popupParent->testFlag(WType_Popup) )
	popup = (QPopupMenu*)popup->popupParent;
    popup->hide();				// cascade from top level
}

void QPopupMenu::hideSubMenus()			// hide all sub menus
{
    QMenuItemListIt it( *items );
    QMenuItem *p;
    while ( (p=it.current()) ) {
	if ( p->submenu )
	    p->submenu->hide();
	++it;
    }
}

int QPopupMenu::itemAtPos( const QPoint &pos )	// get item at pos (x,y)
{
    long row = findRow( pos.y() );		// ask table for row
    long col = findCol( pos.x() );		// ask table for column
    int r = -1;
    if ( row != -1 && col != -1 ) {
	if ( !items->at((int)row)->isSeparator )
	    r = (int)row;
    }
    return r;
}

void QPopupMenu::updateSize()			// update popup size params
{
    QFontMetrics fm( font() );
    int height = 0;
    int max_width = 10;
    QMenuItemListIt it( *items );
    QMenuItem *popup;
    bool hasSubMenu = FALSE;
    cellh = fm.ascent() + motifItemVMargin + 2*motifItemFrame;
    while ( (popup=it.current()) ) {
	if ( popup->submenu )
	    hasSubMenu = TRUE;
	if ( popup->isSeparator )
	    height += motifSepHeight;
	else {
	    height += cellh;
	    int w = fm.width(popup->text);
	    if ( max_width < w )
		max_width = w;
	}
	++it;
    }
    max_width  += 2*motifItemHMargin + 2*motifItemFrame;
    if ( hasSubMenu )
	max_width += fm.ascent() + motifArrowHMargin;
    setNumRows( items->count() );
    cellw = max_width;
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
    firstMouseUp = TRUE;
}

void QPopupMenu::hide()
{
    activeItem = -1;
    hideSubMenus();
    killTimers();
    QWidget::hide();
}


// ---------------------------------------------------------------------------
// Implementation of virtual QTableWidget functions
//

int QPopupMenu::cellHeight( long row )
{
    QMenuItem *popup = items->at( row );
    ASSERT( popup );
    return popup->isSeparator ? motifSepHeight : cellh;
}

int QPopupMenu::cellWidth( long col )
{
    return cellw;
}

void QPopupMenu::paintCell( QPainter *p, long row, long col )
{
    QMenuItem *popup = items->at( row );	// get popup item
    ASSERT( popup );
    if ( popup->isSeparator ) {			// this is a separator
	p->drawShadeLine( 0, 0, cellw, 0, darkColor, lightColor );
	return;
    }
    QFontMetrics fm( font() );
    int bo = fm.descent()+motifItemVMargin/2;	// baseline offset
    if ( row == activeItem )			// this is the active item
	p->drawShadePanel( 0, 0, cellw, cellh,
			   lightColor, darkColor,
			   motifItemFrame, motifItemFrame );
    else					// normal item
	p->drawShadePanel( 0, 0, cellw, cellh,
			   normalColor, normalColor,
			   motifItemFrame, motifItemFrame );
    p->drawText( motifItemFrame + motifItemHMargin, cellh-bo, popup->text );
    if ( popup->submenu ) {			// draw sub menu arrow
	int dim = fm.ascent()*3/4;
	qDrawMotifArrow( p, MotifRightArrow, row == activeItem,
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
    QTableWidget::paintEvent( e );		// the table draws the items
}


void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    if ( !clientRect().contains( e->pos() ) ) {
	hide();
	return;
    }
    int item = itemAtPos( e->pos() );
    QMenuItem *p = item >= 0 ? items->at(item) : 0;
    if ( item != activeItem ) {			// new item activated
	activeItem = item;
	repaint( FALSE );
	if ( activeItem >= 0 )
	    emit activated( p->id );
    }
    if ( p && p->submenu ) {
	if ( !p->submenu->isVisible() )
	    startTimer( 200 );
    }
    else
	hideSubMenus();
}

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    activeItem = itemAtPos( e->pos() );
    repaint( FALSE );
    if ( activeItem >= 0 ) {			// selected menu item!
	QMenuItem *p = items->at(activeItem);
	if ( !p->submenu ) {			// it's not another popup
	    if ( p->signal )			// activate signal
		p->signal->activate();
	    else				// normal connection
		emit selected( p->id );
	    hideAllMenus();
	}
    }
    else {
	if ( isTopLevel ) {
	    if ( !firstMouseUp )
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
    QMenuItem *p = item >= 0 ? items->at(item) : 0;
    if ( item != activeItem ) {			// new item activated
	activeItem = item;
	repaint( FALSE );
	if ( activeItem >= 0 )
	    emit activated( p->id );
    }
    if ( p && p->submenu ) {
	if ( !p->submenu->isVisible() )
	    startTimer( 200 );
    }
    else
	hideSubMenus();
}


void QPopupMenu::timerEvent( QTimerEvent *e )
{
    killTimer( e->timerId() );			// single-shot timer
    if ( activeItem < 0 )
	return;
    QMenuItem *p = items->at( activeItem );
    if ( p->submenu ) {
	QPoint pos( clientSize().width()-motifArrowHMargin, motifPopupFrame );
	for ( int i=0; i<activeItem; i++ )
	    pos.ry() += cellHeight( i );
	p->submenu->popup( mapToGlobal(pos) );
    }
}
