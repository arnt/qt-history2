/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopmenu.cpp#60 $
**
** Implementation of QPopupMenu class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 INCLUDE_MENUITEM_DEF
#include "qpopmenu.h"
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qscrbar.h"				// qDrawArrow
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qpopmenu.cpp#60 $")


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
static const winArrowHMargin	= 6;		// arrow horizontal margin
static const winArrowVMargin	= 4;		// arrow horizontal margin

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
static const motifItemVMargin	= 2;		// menu item ver text margin
static const motifArrowHMargin	= 6;		// arrow horizontal margin
static const motifArrowVMargin	= 2;		// arrow horizontal margin
static const motifTabSpacing	= 12;		// space between text and tab

/* static char sizePopupFrame[] =
    { macPopupFrame, winPopupFrame, pmPopupFrame, motifPopupFrame };
*/   // commented out 7/5-95 EE ###

/*

+-----------------------------
|      PopupFrame
|   +-------------------------
|   |	   ItemFrame
|   |	+---------------------
|   |	|
|   |	|			   \
|   |	|   ^	T E X T	  ^	    | ItemVMargin
|   |	|   |		  |	   /
|   |	      ItemHMargin
|

*/


/*----------------------------------------------------------------------------
  \class QPopupMenu qpopmenu.h
  \brief The QPopupMenu class provides a popup menu widget.

  \ingroup menus
  \ingroup realwidgets

  The popup widget is different from other widgets in the way it
  relates to the parent widget.

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  \sa QMenuBar
 ----------------------------------------------------------------------------*/


/*****************************************************************************
  QPopupMenu member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Constructs a popup menu with a null parent and a widget name.

  A popup menu must be a top level widget, i.e. parent must be 0.
  This argument is present merely for API uniformity.
 ----------------------------------------------------------------------------*/

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
    : QTableView( 0, name, WType_Popup )
{
    initMetaObject();
#if defined(CHECK_RANGE)
    if ( parent != 0 )
	warning( "QPopupMenu: Parent must be null" );
#endif
    isPopup	  = TRUE;
    selfItem	  = 0;
    autoaccel	  = 0;
    accelDisabled = FALSE;
    popupActive	  = -1;
    tabMark	  = 0;
    setNumCols( 1 );				// set number of table columns
    setNumRows( 0 );				// set number of table rows
    switch ( style() ) {
	case WindowsStyle:
	    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	    break;
	case MotifStyle:
	    setFrameStyle( QFrame::Panel | QFrame::Raised );
	    setLineWidth( motifPopupFrame );
	    break;
	default:
	    setFrameStyle( QFrame::Panel | QFrame::Plain );
	    setLineWidth( 1 );
    }
}

/*----------------------------------------------------------------------------
  Destroys the popup menu.
 ----------------------------------------------------------------------------*/

QPopupMenu::~QPopupMenu()
{
    delete autoaccel;
    if ( parentMenu )
	parentMenu->removePopup( this );	// remove from parent menu
}


void QPopupMenu::updateItem( int id )		// update popup menu item
{
    updateCell( indexOf(id), 0, FALSE );
}

void QPopupMenu::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    updateAccel( 0 );
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
    connect( popup, SIGNAL(activatedRedirect(int)),
	     SLOT(subActivated(int)) );
    connect( popup, SIGNAL(highlightedRedirect(int)),
	     SLOT(subHighlighted(int)) );
}

void QPopupMenu::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = 0;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(highlightedRedirect(int)), this,
		       SLOT(subHighlighted(int)) );
}


void QPopupMenu::frameChanged()
{
    menuContentsChanged();
}


/*----------------------------------------------------------------------------
  Opens the popup menu so that the item number \e indexAtPoint
  will be at the specified global position \e pos.
 ----------------------------------------------------------------------------*/

void QPopupMenu::popup( const QPoint &pos, int indexAtPoint )
{
    if ( mitems->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( badSize )
	updateSize();
    QWidget *desktop = QApplication::desktop();
    int sw = desktop->width();			// screen width
    int sh = desktop->height();			// screen height
    int x  = pos.x();
    int y  = pos.y();
    if ( indexAtPoint > 0 )			// don't subtract when < 0
	y -= itemPos( indexAtPoint );		// (would subtract 2 pixels!)
    int w  = width();
    int h  = height();
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
    emit activatedRedirect( id );
}

void QPopupMenu::subHighlighted( int id )
{
    emit highlightedRedirect( id );
}

void QPopupMenu::accelActivated( int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi && !mi->isDisabled() ) {
	if ( mi->signal() )			// activate signal
	    mi->signal()->activate();
	else					// normal connection
	    actSig( mi->id() );
    }
}

void QPopupMenu::accelDestroyed()		// accel about to be deleted
{
    autoaccel = 0;				// don't delete it twice!
}


void QPopupMenu::actSig( int id )
{
    if ( receivers(SIGNAL(activated(int))) )
	emit activated( id );
    else
	emit activatedRedirect( id );
}

void QPopupMenu::hilitSig( int id )
{
    if ( receivers(SIGNAL(highlighted(int))) )
	emit highlighted( id );
    else
	emit highlightedRedirect( id );
}


void QPopupMenu::setFirstItemActive()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    actItem = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( !mi->isSeparator() ) {
	    repaint( FALSE );
	    return;
	}
	actItem++;
    }
    actItem = -1;
}

/*----------------------------------------------------------------------------
  \internal
  Hides all popup menus (in this menu tree) that are currently open.
 ----------------------------------------------------------------------------*/

void QPopupMenu::hideAllPopups()
{
    register QMenuData *top = this;		// find top level popup
    while ( top->parentMenu && top->parentMenu->isPopup )
	top = top->parentMenu;
    ((QPopupMenu*)top)->hide();			// cascade from top level
}

/*----------------------------------------------------------------------------
  \internal
  Hides all popup sub-menus.
 ----------------------------------------------------------------------------*/

void QPopupMenu::hidePopups()
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


/*----------------------------------------------------------------------------
  \internal
  Sends the event to the menu bar.
 ----------------------------------------------------------------------------*/

bool QPopupMenu::tryMenuBar( QMouseEvent *e )
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    return top->isMenuBar ?
	((QMenuBar *)top)->tryMouseEvent( this, e ) : FALSE;
}

/*----------------------------------------------------------------------------
  \internal
  Tells the menu bar to go back to idle state.
 ----------------------------------------------------------------------------*/

void QPopupMenu::byeMenuBar()
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    if ( top->isMenuBar )
	((QMenuBar *)top)->goodbye();
}


/*----------------------------------------------------------------------------
  \internal
  Return the item at \e pos, or -1 if there is no item there, or if
  it is a separator item.
 ----------------------------------------------------------------------------*/

int QPopupMenu::itemAtPos( const QPoint &pos )
{
    int row = findRow( pos.y() );		// ask table for row
    int col = findCol( pos.x() );		// ask table for column
    int r = -1;
    if ( row != -1 && col != -1 ) {
	QMenuItem *mi = mitems->at(row);
	if ( !mi->isSeparator() )
	    r = row;				// normal item
    }
    return r;
}

/*----------------------------------------------------------------------------
  \internal
  Returns the y (top) position of item number \e index.
 ----------------------------------------------------------------------------*/

int QPopupMenu::itemPos( int index )		// get y coord for item
{
    int y;
    if ( rowYPos( index, &y ) )			// ask table for position
	return y;
    else
	return 0;				// return 0 if not visible
}


/*----------------------------------------------------------------------------
  \internal
  Calculates and sets the size of the popup menu, based on the size
  of the items.
 ----------------------------------------------------------------------------*/

void QPopupMenu::updateSize()
{
    int height	  = 0;
    int max_width = 10;
    QFontMetrics fm = fontMetrics();
    QMenuItemListIt it( *mitems );
    register QMenuItem *mi;
    bool hasSubMenu = FALSE;
    int cellh = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    int tab_width = 0;
    while ( (mi=it.current()) ) {
	int w = 0;
	if ( mi->popup() )
	    hasSubMenu = TRUE;
	if ( mi->isSeparator() )
	    height += motifSepHeight;
	else if ( mi->pixmap() ) {
	    height += mi->pixmap()->height() + 2*motifItemFrame;
	    w = mi->pixmap()->width();
	}
	else if ( mi->string() ) {
	    height += cellh;
	    const char *s = mi->string();
	    const char *t = strchr( s, '\t' );
	    if ( t ) {				// string contains tab
		w = fm.width( s, (int)((long)t-(long)s) );
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
    resize( max_width+2*frameWidth(), height+2*frameWidth() );
    badSize = FALSE;
}


static QString key_str( long k )		// get key string
{
    QString s;
    if ( (k & SHIFT) == SHIFT )
	s = "Shift";
    if ( (k & CTRL) == CTRL ) {
	if ( s.isEmpty() )
	    s = "Ctrl";
	else
	    s += "+Ctrl";
    }
    if ( (k & ALT) == ALT ) {
	if ( s.isEmpty() )
	    s = "A|t";
	else
	    s += "+Alt";
    }
    k &= ~(SHIFT | CTRL | ALT);
    QString p;
    if ( k >= Key_F1 && k <= Key_F24 )
	p.sprintf( "F%d", k - Key_F1 + 1 );
    else if ( k >= Key_Space && k <= Key_AsciiTilde )
	p.sprintf( "%c", k );
    else {
	switch ( k ) {
	    case Key_Escape:
		p = "Esc";
		break;
	    case Key_Tab:
		p = "Tab";
		break;
	    case Key_Backtab:
		p = "Backtab";
		break;
	    case Key_Backspace:
		p = "Backspace";
		break;
	    case Key_Return:
		p = "Return";
		break;
	    case Key_Enter:
		p = "Enter";
		break;
	    case Key_Insert:
		p = "Ins";
		break;
	    case Key_Delete:
		p = "Del";
		break;
	    case Key_Pause:
		p = "Pause";
		break;
	    case Key_Print:
		p = "Print";
		break;
	    case Key_SysReq:
		p = "SysReq";
		break;
	    case Key_Home:
		p = "Home";
		break;
	    case Key_End:
		p = "End";
		break;
	    case Key_Left:
		p = "Left";
		break;
	    case Key_Up:
		p = "Up";
		break;
	    case Key_Right:
		p = "Right";
		break;
	    case Key_Down:
		p = "Down";
		break;
	    case Key_Prior:
		p = "PgUp";
		break;
	    case Key_Next:
		p = "PgDown";
		break;
	    case Key_CapsLock:
		p = "CapsLock";
		break;
	    case Key_NumLock:
		p = "NumLock";
		break;
	    case Key_ScrollLock:
		p = "ScrollLock";
		break;
	    default:
		p.sprintf( "<%d?>", k );
		break;
	}
    }
    if ( s.isEmpty() )
	s = p;
    else {
	s += '+';
	s += p;
    }
    return s;
}


/*----------------------------------------------------------------------------
  \internal
  The \e parent is 0 when it is updated when a menu item has
  changed a state, or it is something else if called from the menu bar.
 ----------------------------------------------------------------------------*/

void QPopupMenu::updateAccel( QWidget *parent )
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    if ( parent == 0 && autoaccel == 0 )
	return;
    if ( autoaccel )				// build it from scratch
	autoaccel->clear();
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->key() ) {
	    if ( !autoaccel ) {
		autoaccel = new QAccel( parent );
		CHECK_PTR( autoaccel );
		connect( autoaccel, SIGNAL(activated(int)),
			 SLOT(accelActivated(int)) );
		connect( autoaccel, SIGNAL(destroyed()),
			 SLOT(accelDestroyed()) );
		if ( accelDisabled )
		    autoaccel->disable();
	    }
	    long k = mi->key();
	    autoaccel->insertItem( k, mi->id() );
	    if ( mi->string() ) {
		QString s = mi->string();
		int i = s.find('\t');
		QString t = key_str( k );
		if ( i >= 0 )
		    s.replace( i+1, s.length()-i, t );
		else {
		    s += '\t';
		    s += t;
		}
		if ( s != mi->string() ) {
		    mi->setString( s );
		    badSize = TRUE;
		}
	    }
	}
	if ( mi->popup() && parent )		// call recursively
	    mi->popup()->updateAccel( parent );
    }
}

/*----------------------------------------------------------------------------
  \internal
  It would be better to check in the slot.
 ----------------------------------------------------------------------------*/

void QPopupMenu::enableAccel( bool enable )
{
    if ( autoaccel ) {
	if ( enable )
	    autoaccel->enable();
	else
	    autoaccel->disable();
    }
    else
	accelDisabled = TRUE;			// rememeber when updateAccel
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {		// do the same for sub popups
	++it;
	if ( mi->popup() )			// call recursively
	    mi->popup()->enableAccel( enable );
    }
}


/*----------------------------------------------------------------------------
  Reimplements QWidget::setFont() to be able to refresh the popup menu
  when its font changes.
 ----------------------------------------------------------------------------*/

void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    update();
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::show() for internal purposes.
 ----------------------------------------------------------------------------*/

void QPopupMenu::show()
{
    if ( badSize )
	updateSize();
    QWidget::show();
    raise();
    popupActive = -1;
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::hide() for internal purposes.
 ----------------------------------------------------------------------------*/

void QPopupMenu::hide()
{
    actItem = popupActive = -1;
    mouseBtDn = FALSE;				// mouse button up
    hidePopups();
    killTimers();
    QWidget::hide();
}


/*****************************************************************************
  Implementation of virtual QTableView functions
 *****************************************************************************/

int QPopupMenu::cellHeight( int row )
{
    QMenuItem *mi = mitems->at( row );
    int h;
    if ( mi->isSeparator() )			// separator height
	h = motifSepHeight;
    else if ( mi->pixmap() )			// pixmap height
	h = mi->pixmap()->height() + 2*motifItemFrame;
    else {					// text height
	QFontMetrics fm = fontMetrics();
	h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    }
    return h;
}

int QPopupMenu::cellWidth( int )
{
    return width() - 2*frameWidth();
}


void QPopupMenu::paintCell( QPainter *p, int row, int col )
{
    QColorGroup g = colorGroup();
    QMenuItem *mi = mitems->at( row );		// get menu item
    int cellh	  = cellHeight( row );
    int cellw	  = cellWidth( col );
    GUIStyle gs	  = style();
    bool act	  = row == actItem;
    bool dis	  = (selfItem && selfItem->isDisabled()) || mi->isDisabled();

    if ( !mi->isDirty() )
	return;

    if ( mi->isSeparator() ) {			// draw separator
	p->setPen( g.dark() );
	p->drawLine( 0, 0, cellw, 0 );
	p->setPen( g.light() );
	p->drawLine( 0, 1, cellw, 1 );
	return;
    }
    int pw = motifItemFrame;
    if ( gs != MotifStyle )
	pw = 1;
    if ( gs == WindowsStyle ) {
	p->fillRect( 0, 0, cellw, cellh, act ? darkBlue : g.background() );
	p->setPen( act ? white : g.text() );
    }
    else if ( gs == MotifStyle ) {
	if ( act )				// active item frame
	    qDrawShadePanel( p, 0, 0, cellw, cellh, g, FALSE, pw );
	else					// incognito frame
	    qDrawPlainRect( p, 0, 0, cellw, cellh, g.background(), pw );
	p->setPen( g.text() );
    }

    QColor discol;
    if ( dis ) {
	discol = palette().disabled().text();
	p->setPen( discol );
    }

    if ( mi->pixmap() ) {			// draw pixmap
	QPixmap *pixmap = mi->pixmap();
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( OpaqueMode );
	p->drawPixmap( motifItemFrame + motifItemHMargin, motifItemFrame,
		       *pixmap );
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( TransparentMode );
    }
    else if ( mi->string() ) {			// draw text
	const char *s = mi->string();
	const char *t = strchr( s, '\t' );
	int x = motifItemFrame + motifItemHMargin;
	int m = motifItemVMargin;
	const text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
	if ( t ) {				// draw text before tab
	    p->drawText( x, m, cellw, cellh-2*m, text_flags,
			 s, (int)((long)t-(long)s) );
	    s = t + 1;
	    x = tabMark;
	}
	p->drawText( x, m, cellw, cellh-2*m, text_flags, s );
    }
    if ( mi->popup() ) {			// draw sub menu arrow
	int dim = (cellh-2*motifItemFrame);
	if ( gs == MacStyle ) {
	    QPointArray a;
	    a.setPoints( 3, 0,-dim/2, 0,dim/2, dim/2,0 );
	    a.move( cellw - motifArrowHMargin - dim, cellh/2-dim/2 );
	    p->setBrush( g.foreground() );
	    p->setPen( NoPen );
	    p->drawPolygon( a );
	}
	else {
	    dim /= 2;
	    if ( gs == WindowsStyle && row == actItem ) {
		if ( !dis )
		    discol = white;
		g = QColorGroup( discol, darkBlue, white, white, white,
				 discol, white );
	    }
	    qDrawArrow( p, RightArrow, gs, row == actItem,
			cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			dim, dim, g );
	}
    }
    mi->setDirty( FALSE );
}


void QPopupMenu::paintAll()
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    int row = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->isDirty() )			// this item needs a refresh
	    updateCell( row, 0, FALSE );
	++row;
    }
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Handles paint events for the popup menu.
 ----------------------------------------------------------------------------*/

void QPopupMenu::paintEvent( QPaintEvent *e )
{
    setAllDirty( TRUE );
    QTableView::paintEvent( e );
    setAllDirty( FALSE );
}


/*----------------------------------------------------------------------------
  Handles mouse press events for the popup menu.
 ----------------------------------------------------------------------------*/

void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains( e->pos() ) && !tryMenuBar( e ) ) {
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
	    hilitSig( mi->id() );
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

/*----------------------------------------------------------------------------
  Handles mouse release events for the popup menu.
 ----------------------------------------------------------------------------*/

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    mouseBtDn = FALSE;				// mouse button up
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains( e->pos() ) && tryMenuBar( e ) )
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
		actSig( mi->id() );
	}
    }
    else {
	hideAllPopups();
	byeMenuBar();
    }
}

/*----------------------------------------------------------------------------
  Handles mouse move events for the popup menu.
 ----------------------------------------------------------------------------*/

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    int	 item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no valid item
	if ( popupActive == -1 ) {		// no active popup sub menu
	    int lastActItem = actItem;
	    actItem = -1;
	    if ( lastActItem >= 0 )
		updateCell( lastActItem, 0, FALSE );
	}
	if ( !rect().contains( e->pos() ) && !tryMenuBar( e ) )
	    hidePopups();
    }
    else {					// mouse on valid item
	register QMenuItem *mi = mitems->at( item );
	QPopupMenu *popup = mi->popup();
	if ( actItem == item ) {
	    if ( popupActive == item && popup->actItem != -1 ) {
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
	    hilitSig( mi->id() );
    }
}


/*----------------------------------------------------------------------------
  Handles key press events for the popup menu.
 ----------------------------------------------------------------------------*/

void QPopupMenu::keyPressEvent( QKeyEvent *e )
{
    if ( mouseBtDn )				// cannot handle key event
	return;

    QMenuItem  *mi = 0;
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
		    actSig( mi->id() );
	    }
	    break;

	default:
	    ok_key = FALSE;

    }

    if ( !ok_key ) {				// send to menu bar
	register QMenuData *top = this;		// find top level
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
		hilitSig( mi->id() );
	    updateCell( lastActItem, 0, FALSE );
	    updateCell( actItem, 0, FALSE );
	}
    }
}


/*----------------------------------------------------------------------------
  Handles timer events for the popup menu.
 ----------------------------------------------------------------------------*/

void QPopupMenu::timerEvent( QTimerEvent *e )
{
    killTimer( e->timerId() );			// single-shot timer
    if ( actItem < 0 )
	return;
    QPopupMenu *popup = mitems->at(actItem)->popup();
    if ( popup ) {				// it is a popup
	QPoint pos( width() - motifArrowHMargin,
		    frameWidth() + motifArrowVMargin );
	for ( int i=0; i<actItem; i++ )
	    pos.ry() += (QCOORD)cellHeight( i );
	popupActive = actItem;
	popup->popup( mapToGlobal(pos) );
    }
}
