#include <qaction.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qstyle.h>
#include "command.h"
#include "formwindow.h"
#include "menubareditor.h"
#include "popupmenueditor.h"

extern void find_accel( const QString &txt, QMap<QChar, QWidgetList > &accels, QWidget *w );

// Drag Object Declaration -------------------------------------------

class MenuBarEditorItemPtrDrag : public QStoredDrag
{
public:
    MenuBarEditorItemPtrDrag( MenuBarEditorItem * item,
			      QWidget * parent = 0,
			      const char * name = 0 );
    ~MenuBarEditorItemPtrDrag() {};
    static bool canDecode( QDragMoveEvent * e );
    static bool decode( QDropEvent * e, MenuBarEditorItem ** i );
};

// Drag Object Implementation ---------------------------------------

MenuBarEditorItemPtrDrag::MenuBarEditorItemPtrDrag( MenuBarEditorItem * item,
						    QWidget * parent,
						    const char * name )
    : QStoredDrag( "qt/menubareditoritemptr", parent, name )
{
    QByteArray data( sizeof( Q_LONG ) );
    QDataStream stream( data, IO_WriteOnly );
    stream << ( Q_LONG ) item;
    setEncodedData( data );
}

bool MenuBarEditorItemPtrDrag::canDecode( QDragMoveEvent * e )
{
    return e->provides( "qt/menubareditoritemptr" );
}

bool MenuBarEditorItemPtrDrag::decode( QDropEvent * e, MenuBarEditorItem ** i )
{
    QByteArray data = e->encodedData( "qt/menubareditoritemptr" );
    QDataStream stream( data, IO_ReadOnly );

    if ( !data.size() )
	return FALSE;

    Q_LONG p = 0;
    stream >> p;
    *i = ( MenuBarEditorItem *) p;

    return TRUE;
}

// MenuBarEditorItem ---------------------------------------------------

MenuBarEditorItem::MenuBarEditorItem( MenuBarEditor * bar, int id )
    : menuBar( bar ),
      popupMenu( 0 ),
      visible( TRUE ),
      separator( FALSE ),
      removable( FALSE ),
      autodelete( FALSE ),
      identity( id )
{ }

MenuBarEditorItem::MenuBarEditorItem( PopupMenuEditor * menu, MenuBarEditor * bar, int id )
    : menuBar( bar ),
      popupMenu( menu ),
      visible( TRUE ),
      separator( FALSE ),
      removable( TRUE ),
      autodelete( TRUE ),
      identity( id )
{
    text = menu->name();
}

MenuBarEditorItem::MenuBarEditorItem( QActionGroup * actionGroup,
				      MenuBarEditor * bar, int id )
    : menuBar( bar ),
      popupMenu( 0 ),
      visible( TRUE ),
      separator( FALSE ),
      removable( TRUE ),
      autodelete( TRUE ),
      identity( id )
{
    text = actionGroup->menuText();
    popupMenu = new PopupMenuEditor( menuBar->formWindow(), menuBar );
    popupMenu->insert( actionGroup );
}

MenuBarEditorItem::MenuBarEditorItem( MenuBarEditorItem * item, int id )
    : menuBar( item->menuBar ),
      popupMenu( 0 ),
      text( item->text ),
      visible( item->visible ),
      separator( item->separator ),
      removable( item->removable ),
      autodelete( item->autodelete ),
      identity( id )
{
    popupMenu = new PopupMenuEditor( menuBar->formWindow(), item->popupMenu, menuBar );
}

MenuBarEditorItem::~MenuBarEditorItem()
{
// FIXME: autodelete not used
}

PopupMenuEditor * MenuBarEditorItem::menu()
{
    return popupMenu;
}

int MenuBarEditorItem::id()
{
    return identity;
}

void MenuBarEditorItem::setMenuText( const QString t )
{
    text = t;
}

QString MenuBarEditorItem::menuText()
{
    return text;
}

void MenuBarEditorItem::setVisible( bool enable )
{
    visible = enable;
}

bool MenuBarEditorItem::isVisible()
{
    return visible;
}

void MenuBarEditorItem::setRemovable( bool enable )
{
    removable = enable;
}

bool MenuBarEditorItem::isRemovable()
{
    return removable;
}

void MenuBarEditorItem::setAutoDelete( bool enable )
{
    autodelete = enable;
}

bool MenuBarEditorItem::isAutoDelete()
{
    return autodelete;
}

bool MenuBarEditorItem::isSeparator()
{
    return separator;
}

void MenuBarEditorItem::setSeparator( bool enable )
{
    separator = enable;
}

// MenuBarEditor --------------------------------------------------------

int MenuBarEditor::clipboardOperation = 0;
MenuBarEditorItem * MenuBarEditor::clipboardItem = 0;

MenuBarEditor::MenuBarEditor( FormWindow * fw, QWidget * parent, const char * name )
    : QMenuBar( parent, name ),
      formWnd( fw ),
      draggedItem( 0 ),
      currentIndex( 0 ),
      itemHeight( 0 ),
      separatorWidth( 32 ),
      borderSize( 4 ),
      hideWhenEmpty( TRUE ),
      hasSeparator( FALSE )
{
    setAcceptDrops( TRUE );
    setFocusPolicy( StrongFocus );

    addItem.setMenuText( tr("new menu") );
    addSeparator.setMenuText( tr("new separator") );

    lineEdit = new QLineEdit( this, "menubar lineedit" );
    lineEdit->hide();
    lineEdit->setFrame( FALSE );
    lineEdit->setEraseColor( eraseColor() );

    dropLine = new QWidget( this, "menubar dropline", Qt::WStyle_NoBorder | WStyle_StaysOnTop );
    dropLine->setBackgroundColor( Qt::black );
    dropLine->hide();

    setMinimumHeight( fontMetrics().height() + 2 * borderSize );
}

MenuBarEditor::~MenuBarEditor()
{
    itemList.setAutoDelete( TRUE );
}

FormWindow * MenuBarEditor::formWindow()
{
    return formWnd;
}

MenuBarEditorItem * MenuBarEditor::createItem( int index )
{
    MenuBarEditorItem * i =
	new MenuBarEditorItem(
	    new PopupMenuEditor( formWnd, ( QWidget * ) parent() ), this );
    AddMenuCommand * cmd = new AddMenuCommand( "Add Menu", formWnd, this, i, index );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
    return i;
}

void MenuBarEditor::insertItem( MenuBarEditorItem * item, int index )
{
    item->menu()->parentMenu = this;

    if ( index != -1 )
	itemList.insert( index, item );
    else
	itemList.append( item );

    if ( hideWhenEmpty && itemList.count() == 1 )
	show(); // calls resizeInternals();
    else
	resizeInternals();

    if ( isVisible() )
	update();
}

void MenuBarEditor::insertItem( QString text, PopupMenuEditor * menu, int id, int index )
{
    MenuBarEditorItem * item = new MenuBarEditorItem( menu, this, id );
    if ( !text.isNull() )
	item->setMenuText( text );
    insertItem( item, index );
}

void MenuBarEditor::insertItem( QString text, QActionGroup * group, int id, int index )
{
    MenuBarEditorItem * item = new MenuBarEditorItem( group, this, id );
    if ( !text.isNull() )
	item->setMenuText( text );
    insertItem( item, index );
}


void MenuBarEditor::insertSeparator( int index )
{
    if ( hasSeparator )
	return;

    MenuBarEditorItem * i = createItem( index );
    i->setSeparator( TRUE );
    i->setMenuText( "separator" );
    hasSeparator = TRUE;
}

void MenuBarEditor::removeItemAt( int index )
{
    removeItem( item( index ) );
}

void MenuBarEditor::removeItem( MenuBarEditorItem * item )
{
    if ( item &&
	 item->isRemovable() &&
	 itemList.removeRef( item ) ) {

	if ( item->isSeparator() )
	    hasSeparator = FALSE;

	if ( hideWhenEmpty && itemList.count() == 0 )
	    hide();
	else
	    resizeInternals();

	int n = count() + 1;
	if ( currentIndex >=  n )
	    currentIndex = n;

	if ( isVisible() )
	    update();
    }
}

void MenuBarEditor::removeItem( int id )
{
    removeItemAt( findItem( id ) );
}

int MenuBarEditor::findItem( MenuBarEditorItem * item )
{
    return itemList.findRef( item );
}

int MenuBarEditor::findItem( PopupMenuEditor * menu )
{
    MenuBarEditorItem * i = itemList.first();

    while ( i ) {
	if ( i->menu() == menu )
	    return itemList.at();
	i = itemList.next();
    }

    return -1;
}

int MenuBarEditor::findItem( int id )
{
    MenuBarEditorItem * i = itemList.first();

    while ( i ) {
	if ( i->id() == id )
	    return itemList.at();
	i = itemList.next();
    }
    return -1;
}

int MenuBarEditor::findItem( QPoint & pos )
{
    int x = borderSize;
    int dx = 0;
    int y = 0;
    int w = width();
    QSize s;
    QRect r;

    MenuBarEditorItem * i = itemList.first();

    while ( i ) {

	if ( i->isVisible() ) {

	    s = itemSize( i );
	    dx = s.width();

	    if ( x + dx > w && x > borderSize ) {
		y += itemHeight;
		x = borderSize;
	    }

	    r = QRect( x, y, s.width(), s.height() );

	    if ( r.contains( pos ) )
		return itemList.at();

	    addItemSizeToCoords( i, x, y, w );
	}

	i = itemList.next();
    }

    // check add item
    s = itemSize( &addItem );
    dx = s.width();

    if ( x + dx > w && x > borderSize ) {
	y += itemHeight;
	x = borderSize;
    }

    r = QRect( x, y, s.width(), s.height() );

    if ( r.contains( pos ) )
	return itemList.count();

    return itemList.count() + 1;
}

MenuBarEditorItem * MenuBarEditor::item( int index )
{
    if ( index == -1 )
	return itemList.at( currentIndex );

    int c = itemList.count();
    if ( index == c )
	return &addItem;
    else if ( index > c )
	return &addSeparator;

    return itemList.at( index );
}

int MenuBarEditor::count()
{
    return itemList.count();
}

int MenuBarEditor::current()
{
    return currentIndex;
}

void MenuBarEditor::cut( int index )
{
    if ( clipboardItem && clipboardOperation == Cut )
	delete clipboardItem;

    clipboardOperation = Cut;
    clipboardItem = itemList.at( index );

    if ( clipboardItem == &addItem || clipboardItem == &addSeparator ) {
	clipboardOperation = None;
	clipboardItem = 0;
	return; // do nothing
    }

    RemoveMenuCommand * cmd = new RemoveMenuCommand( "Cut Menu", formWnd, this, index );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
}

void MenuBarEditor::copy( int index )
{
    if ( clipboardItem && clipboardOperation == Cut )
	delete clipboardItem;

    clipboardOperation = Copy;
    clipboardItem = itemList.at( index );

    if ( clipboardItem == &addItem || clipboardItem == &addSeparator ) {
	clipboardOperation = None;
	clipboardItem = 0;
    }
}

void MenuBarEditor::paste( int index )
{
    if ( clipboardItem && clipboardOperation ) {
	MenuBarEditorItem * i = new MenuBarEditorItem( clipboardItem );
	AddMenuCommand * cmd = new AddMenuCommand( "Paste Menu", formWnd, this, i, index );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void MenuBarEditor::exchange( int a, int b )
{
    MenuBarEditorItem * ia = itemList.at( a );
    MenuBarEditorItem * ib = itemList.at( b );
    if ( !ia || !ib ||
	 ia == &addItem || ia == &addSeparator ||
	 ib == &addItem || ib == &addSeparator )
	return; // do nothing
    itemList.replace( b, ia );
    itemList.replace( a, ib );
}

void MenuBarEditor::showLineEdit( int index )
{
    if ( index == -1 )
	index = currentIndex;

    MenuBarEditorItem * i = 0;

    if ( (uint) index >= itemList.count() )
	i = &addItem;
    else
	i = itemList.at( index );

    // open edit field for item name
    lineEdit->setText( i->menuText() );
    lineEdit->selectAll();
    QPoint pos = itemPos( index );
    lineEdit->move( pos.x() + borderSize, pos.y() ); //FIXME: move
    QPainter p( this );
    lineEdit->resize( itemSize( i ) );
    lineEdit->show();
    lineEdit->setFocus();
}

void MenuBarEditor::showItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	MenuBarEditorItem * i = itemList.at( index );
	if ( i->isSeparator() || draggedItem )
	    return;
	PopupMenuEditor * m = i->menu();
	QPoint pos = itemPos( index );
	m->move( pos.x(), pos.y() + itemHeight - 1 );
	m->raise();
	m->show();
	setFocus();
    }
}

void MenuBarEditor::hideItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	PopupMenuEditor * m = itemList.at( index )->menu();
	m->hideSubMenu();
	m->hide();
    }
}

void MenuBarEditor::focusItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	PopupMenuEditor * m = itemList.at( index )->menu();
	m->setFocus();
	m->update();
	update();
    }
}

void MenuBarEditor::deleteItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	RemoveMenuCommand * cmd = new RemoveMenuCommand( "Delete Menu",
							 formWnd,
							 this,
							 currentIndex );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

QSize MenuBarEditor::sizeHint() const
{
    return QSize( parentWidget()->width(), heightForWidth( parentWidget()->width() ) );
}

int MenuBarEditor::heightForWidth( int max_width ) const
{
    MenuBarEditor * that = ( MenuBarEditor * ) this;
    int x = borderSize;
    int y = 0;

    QPainter p( this );
    that->itemHeight = that->itemSize( &(that->addItem) ).height();

    MenuBarEditorItem * i = that->itemList.first();
    while ( i ) {
	if ( i->isVisible() )
	    that->addItemSizeToCoords( i, x, y, max_width );
	i = that->itemList.next();
    }

    that->addItemSizeToCoords( &(that->addItem), x, y, max_width );
    that->addItemSizeToCoords( &(that->addSeparator), x, y, max_width );

    return y + itemHeight;
}

void MenuBarEditor::show()
{
    QWidget::show();
    resizeInternals();

    QResizeEvent e( parentWidget()->size(), parentWidget()->size() );
    QApplication::sendEvent( parentWidget(), &e );
}

void MenuBarEditor::checkAccels( QMap<QChar, QWidgetList > &accels )
{
    QString t;
    MenuBarEditorItem * i = itemList.first();
    while ( i ) {
	t = i->menuText();
	find_accel( t, accels, this );
	// do not check the accelerators in the popup menus
	i = itemList.next();
    }
}

// public slots

void MenuBarEditor::cut()
{
    cut( currentIndex );
}

void MenuBarEditor::copy()
{
    copy( currentIndex );
}

void MenuBarEditor::paste()
{
    paste( currentIndex );
}

// protected

void MenuBarEditor::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    QRect r = rect();
    style().drawPrimitive( QStyle::PE_PanelMenuBar, &p,
			   r, colorGroup() );
    drawItems( p );
}

void MenuBarEditor::mousePressEvent( QMouseEvent * e )
{
    mousePressPos = e->pos();
    hideItem();
    currentIndex = findItem( mousePressPos );
    showItem();
    update();
    e->accept();
}

void MenuBarEditor::mouseDoubleClickEvent( QMouseEvent * )
{
    currentIndex = findItem( mousePressPos );
    if ( currentIndex > (int)itemList.count() ) {
	insertSeparator();
	update();
    } else {
	showLineEdit();
    }
}

void MenuBarEditor::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() & Qt::LeftButton ) {
	if ( ( e->pos() - mousePressPos ).manhattanLength() > 3 ) {
	    bool itemCreated = FALSE;
	    draggedItem = item( findItem( mousePressPos ) );
	    if ( draggedItem == &addItem ) {
		draggedItem = createItem();
		itemCreated = TRUE;
	    } else if ( draggedItem == &addSeparator && !hasSeparator ) {
		draggedItem = createItem();
		draggedItem->setSeparator( TRUE );
		draggedItem->setMenuText( "separator" );
		hasSeparator = TRUE;
		itemCreated = TRUE;
	    }
	    MenuBarEditorItemPtrDrag * d =
		new MenuBarEditorItemPtrDrag( draggedItem, this );
	    hideItem();
	    draggedItem->setVisible( FALSE );
	    update();

	    // If the item is dropped in the same list,
	    //  we will have two instances of the same pointer
	    // in the list.
	    itemList.find( draggedItem );
	    QLNode * node = itemList.currentNode();
	    d->dragCopy(); // dragevents and stuff happens
	    if ( draggedItem ) { // item was not dropped
		if ( itemCreated ) {
		    removeItem( draggedItem );
		} else {
		    hideItem();
		    draggedItem->setVisible( TRUE );
		    draggedItem = 0;
		    showItem();
		}
	    } else { // item was dropped
		hideItem();
		itemList.takeNode( node )->setVisible( TRUE );
		showItem();
	    }
	    update();
	}
    }
}

void MenuBarEditor::dragEnterEvent( QDragEnterEvent * e )
{
    if ( MenuBarEditorItemPtrDrag::canDecode( e ) ) {
	e->accept();
	dropLine->show();
    }
}

void MenuBarEditor::dragLeaveEvent( QDragLeaveEvent * )
{
    dropLine->hide();
}

void MenuBarEditor::dragMoveEvent( QDragMoveEvent * e )
{

    QPoint pos = e->pos();
    dropLine->move( snapToItem( pos ) );

    int idx = findItem( pos );
    if ( currentIndex != idx ) {
	hideItem();
	currentIndex = idx;
	showItem();
    }
}

void MenuBarEditor::dropEvent( QDropEvent * e )
{
    MenuBarEditorItem * i = 0;

    if ( MenuBarEditorItemPtrDrag::decode( e, &i ) ) {
	draggedItem = 0;
	hideItem(); //FIXME: remove
	dropInPlace( i, e->pos() );
	e->accept();
    }

    dropLine->hide();
}

void MenuBarEditor::keyPressEvent( QKeyEvent * e )
{
    if ( lineEdit->isHidden() ) { // In navigation mode
	switch ( e->key() ) {

	case Qt::Key_Delete:
	    hideItem();
	    deleteItem();
	    showItem();
	    break;

	case Qt::Key_Left:
	    e->accept();
	    navigateLeft( e->state() & Qt::ControlButton );
	    return;

	case Qt::Key_Right:
	    e->accept();
	    navigateRight( e->state() & Qt::ControlButton );
	    return; // no update

	case Qt::Key_Down:
	    e->accept();
	    focusItem();
	    return; // no update

	case Qt::Key_PageUp:
	    currentIndex = 0;
	    break;

	case Qt::Key_PageDown:
	    currentIndex = itemList.count();
	    break;

	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_F2:
	    e->accept();
	    enterEditMode();
	    return; // no update

	case Qt::Key_Up:
	case Qt::Key_Alt:
	case Qt::Key_Shift:
	case Qt::Key_Control:
	case Qt::Key_Escape:
	    e->ignore();
	    setFocus(); // FIXME: this is because some other widget get the focus when CTRL is pressed
	    return; // no update

	case Qt::Key_C:
	    if ( e->state() & Qt::ControlButton && currentIndex < (int)itemList.count() ) {
		copy( currentIndex );
		break;
	    }

	case Qt::Key_X:
	    if ( e->state() & Qt::ControlButton && currentIndex < (int)itemList.count() ) {
		hideItem();
		cut( currentIndex );
		showItem();
		break;
	    }

	case Qt::Key_V:
	    if ( e->state() & Qt::ControlButton ) {
		hideItem();
		paste( currentIndex < (int)itemList.count() ? currentIndex + 1: itemList.count() );
		showItem();
		break;
	    }

	default:
	    if ( e->ascii() >= 32 || e->ascii() == 0 ) {
		showLineEdit();
		QApplication::sendEvent( lineEdit, e );
		e->accept();
	    } else {
		e->ignore();
	    }
	    return;
	}
    } else { // In edit mode

	switch ( e->key() ) {
	case Qt::Key_Control:
	    e->ignore();
	    return;
	case Qt::Key_Enter:
	case Qt::Key_Return:
	    leaveEditMode();
	case Qt::Key_Escape:
	    lineEdit->hide();
	    setFocus();
	    break;
	}
    }
    e->accept();
    update();
}

void MenuBarEditor::focusOutEvent( QFocusEvent * e )
{
    QWidget * fw = qApp->focusWidget();
    if ( e->lostFocus() &&
	 ( !fw || !fw->inherits( "PopupMenuEditor" ) ) )
	hideItem();
    update();
}

void MenuBarEditor::resizeInternals()
{
    dropLine->resize( 2, itemHeight );
    updateGeometry();
}

void MenuBarEditor::drawItems( QPainter & p )
{
    QPoint pos( borderSize, 0 );
    uint c = 0;

    p.setPen( colorGroup().buttonText() );

    MenuBarEditorItem * i = itemList.first();
    while ( i ) {
	if ( i->isVisible() )
	    drawItem( p, i, c++, pos ); // updates x y
	i = itemList.next();
    }

    p.setPen( colorGroup().link() );
    drawItem( p, &addItem, c++, pos );
    if ( !hasSeparator )
	drawItem( p, &addSeparator, c, pos );
}

void MenuBarEditor::drawItem( QPainter & p,
			      MenuBarEditorItem * i,
			      int idx,
			      QPoint & pos )
{
    int w = itemSize( i ).width();

    // If the item passes the right border, and it is not the first item on the line
    if ( pos.x() + w > width() && pos.x() > borderSize ) { // wrap
	pos.ry() += itemHeight;
	pos.setX( borderSize );
    }

    if ( i->isSeparator() ) {
	drawSeparator( p, pos );
    } else {
	int flags = QPainter::AlignLeft | QPainter::AlignVCenter |
	    Qt::ShowPrefix | Qt::SingleLine;
	p.drawText( pos.x() + borderSize, pos.y(), w - borderSize, itemHeight,
		    flags, i->menuText() );
    }

    if ( hasFocus() && idx == currentIndex && !draggedItem )
	p.drawWinFocusRect( pos.x(), pos.y() + 1, w, itemHeight - 2 );

    pos.rx() += w;
}

void MenuBarEditor::drawSeparator( QPainter & p, QPoint & pos )
{
    p.save();
    p.setPen( colorGroup().link() );

    int left = pos.x();
    int top = pos.y() + 2;
    int right = left + separatorWidth - 1;
    int bottom = pos.y() + itemHeight - 4;

    p.drawLine( left, top, left, bottom );
    p.drawLine( right, top, right, bottom );

    p.fillRect( left, pos.y() + borderSize * 2,
		separatorWidth - 1, itemHeight - borderSize * 4,
		QBrush( colorGroup().link(), Qt::Dense5Pattern ) );

    p.restore();
}

QSize MenuBarEditor::itemSize( MenuBarEditorItem * i )
{
    if ( i->isSeparator() )
	return QSize( separatorWidth, itemHeight );
    QRect r = fontMetrics().boundingRect( i->menuText() );
    return QSize( r.width() + borderSize * 2, r.height() + borderSize * 4 );
}

void MenuBarEditor::addItemSizeToCoords( MenuBarEditorItem * i, int & x, int & y, const int w )
{
    int dx = itemSize( i ).width();
    if ( x + dx > w && x > borderSize ) {
	y += itemHeight;
	x = borderSize;
    }
    x += dx;
}

QPoint MenuBarEditor::itemPos( const int index )
{
    int x = borderSize;
    int y = 0;
    int w = width();
    int dx = 0;
    int c = 0;

    MenuBarEditorItem * i = itemList.first();

    while ( i ) {
	if ( i->isVisible() ) {
	    dx = itemSize( i ).width();
	    if ( x + dx > w && x > borderSize ) {
		y += itemHeight;
		x = borderSize;
	    }
	    if ( c == index )
		return QPoint( x, y );
	    x += dx;
	    c++;
	}
	i = itemList.next();
    }
    dx = itemSize( &addItem ).width();
    if ( x + dx > width() && x > borderSize ) {
	y += itemHeight;
	x = borderSize;
    }

    return QPoint( x, y );
}

QPoint MenuBarEditor::snapToItem( const QPoint & pos )
{
    int x = borderSize;
    int y = 0;
    int dx = 0;

    MenuBarEditorItem * n = itemList.first();

    while ( n ) {
	if ( n->isVisible() ) {
	    dx = itemSize( n ).width();
	    if ( x + dx > width() && x > borderSize ) {
		y += itemHeight;
		x = borderSize;
	    }
	    if ( pos.y() > y &&
		 pos.y() < y + itemHeight &&
		 pos.x() < x + dx / 2 ) {
		return QPoint( x, y );
	    }
	    x += dx;
	}
	n = itemList.next();
    }

    return QPoint( x, y );
}

void MenuBarEditor::dropInPlace( MenuBarEditorItem * i, const QPoint & pos )
{
    int x = borderSize;
    int y = 0;
    int dx = 0;
    int idx = 0;

    MenuBarEditorItem * n = itemList.first();

    while ( n ) {
	if ( n->isVisible() ) {
	    dx = itemSize( n ).width();
	    if ( x + dx > width() && x > borderSize ) {
		y += itemHeight;
		x = borderSize;
	    }
	    if ( pos.y() > y &&
		 pos.y() < y + itemHeight &&
		 pos.x() < x + dx / 2 )
		break;
	    x += dx;
	}
	n = itemList.next();
	idx++;
    }

    AddMenuCommand * cmd = new AddMenuCommand( "Add Menu", formWnd, this, i, idx );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
    currentIndex = idx;
}


void MenuBarEditor::safeDec()
{
    do  {
	currentIndex--;
    } while ( currentIndex > 0 && !( item( currentIndex )->isVisible() ) );
}

void MenuBarEditor::safeInc()
{
    int max = (int)itemList.count();
    if ( !hasSeparator )
	max += 1;
    if ( currentIndex < max ) {
	do  {
	    currentIndex++;
	    // skip invisible items
	} while ( currentIndex < max && !( item( currentIndex )->isVisible() ) );
    }
}

void MenuBarEditor::navigateLeft( bool ctrl )
{
    // FIXME: handle invisible items
    if ( currentIndex > 0 ) {
	hideItem();
	if ( ctrl ) {
	    ExchangeMenuCommand * cmd = new ExchangeMenuCommand( "Move Menu Left",
								 formWnd,
								 this,
								 currentIndex,
								 currentIndex - 1 );
	    formWnd->commandHistory()->addCommand( cmd );
	    cmd->execute();
	    safeDec();
	} else {
	    safeDec();
	}
	showItem();
    }
    update();
}

void MenuBarEditor::navigateRight( bool ctrl )
{
// FIXME: handle invisible items
    hideItem();
    if ( ctrl ) {
	if ( currentIndex < ( (int)itemList.count() - 1 ) ) {
	    ExchangeMenuCommand * cmd =	new ExchangeMenuCommand( "Move Menu Right",
								 formWnd,
								 this,
								 currentIndex,
								 currentIndex + 1 );
	    formWnd->commandHistory()->addCommand( cmd );
	    cmd->execute();
	    safeInc();
	}
    } else {
	safeInc();
    }
    showItem();
    update();
}

void MenuBarEditor::enterEditMode()
{
    if ( currentIndex > (int)itemList.count() ) {
	insertSeparator();
    } else {
	showLineEdit();
    }
}

void MenuBarEditor::leaveEditMode()
{
    MenuBarEditorItem * i = 0;
    if ( currentIndex >= (int)itemList.count() ) {
	i = createItem();
	// do not put rename on cmd stack
	RenameMenuCommand rename( "Rename Menu", formWnd, this, lineEdit->text(), i );
	rename.execute();
    } else {
	i = itemList.at( currentIndex );
	RenameMenuCommand * cmd =
	    new RenameMenuCommand( "Rename Menu", formWnd, this, lineEdit->text(), i );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
    showItem();
}
