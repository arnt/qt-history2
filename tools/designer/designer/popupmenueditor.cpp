#include <qapplication.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qdragobject.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qrect.h>
#include <qsize.h>
#include <qstyle.h>
#include "actiondnd.h"
#include "actioneditorimpl.h"
#include "command.h"
#include "formfile.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "metadatabase.h"
#include "pixmapchooser.h"
#include "popupmenueditor.h"

// Drag Object Declaration -------------------------------------------

class PopupMenuEditorItemPtrDrag : public QStoredDrag
{
public:
    PopupMenuEditorItemPtrDrag( PopupMenuEditorItem * item, QWidget * parent = 0,
				const char * name = 0 );
    ~PopupMenuEditorItemPtrDrag() {};
    static bool canDecode( QDragMoveEvent * e );
    static bool decode( QDropEvent * e, PopupMenuEditorItem ** i );
};

// Drag Object Implementation ---------------------------------------

PopupMenuEditorItemPtrDrag::PopupMenuEditorItemPtrDrag( PopupMenuEditorItem * item,
							QWidget * parent, const char * name )
    : QStoredDrag( "qt/popupmenueditoritemptr", parent, name )
{
    QByteArray data( sizeof( Q_LONG ) );
    QDataStream stream( data, IO_WriteOnly );
    stream << ( Q_LONG ) item;
    setEncodedData( data );
}

bool PopupMenuEditorItemPtrDrag::canDecode( QDragMoveEvent * e )
{
    return e->provides( "qt/popupmenueditoritemptr" );
}

bool PopupMenuEditorItemPtrDrag::decode( QDropEvent * e, PopupMenuEditorItem ** i )
{
    QByteArray data = e->encodedData( "qt/popupmenueditoritemptr" );
    QDataStream stream( data, IO_ReadOnly );

    if ( !data.size() )
	return FALSE;

    Q_LONG p = 0;
    stream >> p;
    *i = ( PopupMenuEditorItem *) p;

    return TRUE;
}

// PopupMenuEditorItem Implementation -----------------------------------

PopupMenuEditorItem::PopupMenuEditorItem( PopupMenuEditor * menu, QObject * parent, const char * name )
    : QObject( parent, name ),
      a( 0 ),
      g( 0 ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( FALSE ),
      dirty( TRUE )
{
    init();
    a = new QAction( this );
    QObject::connect( a, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
}


PopupMenuEditorItem::PopupMenuEditorItem( QAction * action, PopupMenuEditor * menu,
					  QObject * parent, const char * name )
    : QObject( parent, name ),
      a( action ),
      g( 0 ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( TRUE ),
      dirty( TRUE )
{
    init();
    QObject::connect( a, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
    if ( /*a->name() == "qt_separator_action" ||*/ a->inherits( "QSeparatorAction" ) )
	separator = TRUE;
}

PopupMenuEditorItem::PopupMenuEditorItem( QActionGroup * actionGroup, PopupMenuEditor * menu,
					  QObject * parent, const char * name )
    : QObject( parent, name ),
      a( 0 ),
      g( actionGroup ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( TRUE ),
      dirty( TRUE )
{
    init();
    QObject::connect( g, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
    g->installEventFilter( this );
    // if the actiongroup uses dropdown, insert the children in the sub menu
    PopupMenuEditor * pme = ( actionGroup->usesDropDown() ? s : m );
    QObjectList * l = g->queryList( "QAction" );
    QObject * o = l->first();
    while ( o ) {
	if ( o->parent() == g ) {
	    if ( o->inherits( "QActionGroup" ) )
		pme->insert( ( QActionGroup * ) o );
	    else
		pme->insert( ( QAction * ) o );
	}
	o = l->next();
    }
    delete l;
}

PopupMenuEditorItem::PopupMenuEditorItem( PopupMenuEditorItem * item, PopupMenuEditor * menu,
					  QObject * parent, const char * name )
    : QObject( parent, name ),
      a( item->a ),
      g( item->g ),
      s( 0 ),
      m( menu ),
      separator( item->separator ),
      removable( item->removable ),
      dirty( TRUE )
{
    init();
    if ( item->type() == ActionGroup )
	g->installEventFilter( this );
}

PopupMenuEditorItem::~PopupMenuEditorItem()
{

}

void PopupMenuEditorItem::init()
{
    QAction * action = anyAction();

    if ( action && m && !isSeparator() ) {
	s = new PopupMenuEditor( m->formWindow(), m );
	QString n = "PopupMenuEditor";
	m->formWindow()->unify( s, n, TRUE );
	s->setName( n );
	MetaDataBase::addEntry( s );
    }

    QObject * o = anyAction();
    if ( o )
	QObject::connect( o, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
}

PopupMenuEditorItem::ItemType PopupMenuEditorItem::type() const
{
    if ( separator )
	return Separator;
    else if ( a )
	return Action;
    else if ( g )
	return ActionGroup;
    return Unknown;
}

void PopupMenuEditorItem::setVisible( bool enable )
{
    if ( a )
	a->setVisible( enable );
    else if ( g )
	g->setVisible( enable );
}

bool PopupMenuEditorItem::isVisible() const
{
    if ( a )
	return a->isVisible();
    else if ( g )
	return ( g->isVisible() && g->usesDropDown() );
    return FALSE;
}

void PopupMenuEditorItem::showMenu( int x, int y )
{
    if ( ( !separator ) && s ) {
	s->move( x, y );
	s->show();
	s->raise();
    }
}

void PopupMenuEditorItem::hideMenu()
{
    if ( s ) {
	s->hideSubMenu();
	s->hide();
    }
}

void PopupMenuEditorItem::focusOnMenu()
{
    if ( s ) {
	s->showSubMenu();
	s->setFocus();
    }
}

int PopupMenuEditorItem::count() const
{
    if ( s ) {
	return s->count();
    } else if ( g ) {
	const QObjectList * l = g->children();
	if ( l )
	    return l->count();
    }
    return 0;
}

bool PopupMenuEditorItem::eventFilter( QObject * o, QEvent * event )
{
    if ( event->type() == QEvent::ChildInserted ) {

	QChildEvent * ce = ( QChildEvent * ) event;
	QObject * c = ce->child();
	PopupMenuEditorItem * i = 0;

	if ( c->inherits( "QActionGroup" ) ) {
	    i = new PopupMenuEditorItem( ( QActionGroup * ) c, s );
	} else if ( c->inherits( "QAction" ) ) {
	    i = new PopupMenuEditorItem( ( QAction * ) c, s );
	    if ( c->name() == "qt_separator_action" )
		i->setSeparator( TRUE );
	}

	// The action is performed someplace else, just do it (no undo)
	if ( ( ( QActionGroup * ) o )->usesDropDown() ) {
	    AddActionToPopupCommand cmd( "Add Item", m->formWnd, s, i );
	    cmd.execute();
	} else {
	    AddActionToPopupCommand cmd( "Add Item", m->formWnd, m, i );
	    cmd.execute();
	}

    } else if ( event->type() == QEvent::ChildRemoved ) {

	QChildEvent * ce = ( QChildEvent * ) event;
	QObject * c = ce->child();

	// The action is performed someplace else, just do it (no undo)
	if ( ( ( QActionGroup * ) o )->usesDropDown() ) {
	    RemoveActionFromPopupCommand cmd( "Remove Item",
					      m->formWnd,
					      s,
					      s->find( ( QAction * ) c ) );
	    cmd.execute();
	} else {
	    RemoveActionFromPopupCommand cmd( "Remove Item",
					      m->formWnd,
					      m,
					      m->find( ( QAction * ) c ) );
	    cmd.execute();
	}
    }
    return FALSE;
}

void PopupMenuEditorItem::selfDestruct()
{
    hideMenu();
    int i = m->find( anyAction() );
    m->remove( i );
    a = 0; // the selfDestruct call was caused by the deletion of one of these items
    g = 0;
    delete this;
}

// PopupMenuEditor Implementation -----------------------------------

PopupMenuEditorItem * PopupMenuEditor::draggedItem = 0;
int PopupMenuEditor::clipboardOperation = 0;
PopupMenuEditorItem * PopupMenuEditor::clipboardItem = 0;

PopupMenuEditor::PopupMenuEditor( FormWindow * fw, QWidget * parent, const char * name )
    : QWidget( 0, name, WStyle_Customize | WStyle_NoBorder | WRepaintNoErase ),
      formWnd( fw ),
      parentMenu( parent ),
      iconWidth( 0 ),
      textWidth( 0 ),
      accelWidth( 0 ),
      arrowWidth( 30 ),
      borderSize( 2 ),
      currentField( 1 ),
      currentIndex( 0 ),
      drawAll( 0 )
{
    init();
}

PopupMenuEditor::PopupMenuEditor( FormWindow * fw, PopupMenuEditor * menu,
				  QWidget * parent, const char * name )
    : QWidget( 0, name, WStyle_Customize | WStyle_NoBorder | WRepaintNoErase ),
      formWnd( fw ),
      parentMenu( parent ),
      iconWidth( menu->iconWidth ),
      textWidth( menu->textWidth ),
      accelWidth( menu->accelWidth ),
      arrowWidth( menu->arrowWidth ),
      borderSize( menu->borderSize ),
      currentField( menu->currentField ),
      currentIndex( menu->currentIndex ),
      drawAll( 0 )
{
    init();

    PopupMenuEditorItem * i;
    for ( i = menu->itemList.first(); i; i = menu->itemList.next() ) {
	PopupMenuEditorItem * n = new PopupMenuEditorItem( i, this );
	itemList.append( n );
    }
}

PopupMenuEditor::~PopupMenuEditor()
{
    itemList.setAutoDelete( TRUE );
}

void PopupMenuEditor::init()
{
    reparent( ( QMainWindow * ) formWnd->mainContainer(), pos() );

    addItem.action()->setMenuText( tr("new item") );
    addSeparator.action()->setMenuText( tr("new separator") );

    setAcceptDrops( TRUE );
    setFocusPolicy( StrongFocus );

    lineEdit = new QLineEdit( this );
    lineEdit->hide();
    lineEdit->setFrame( FALSE );
    lineEdit->setEraseColor( eraseColor() );
    lineEdit->installEventFilter( this );

    dropLine = new QWidget( this, 0, Qt::WStyle_NoBorder | WStyle_StaysOnTop );
    dropLine->setBackgroundColor( Qt::black );
    dropLine->hide();

    hide();

    QObject::connect( formWnd->mainWindow()->actioneditor(), SIGNAL( removing( QAction * ) ),
		      this, SLOT( remove( QAction * ) ) );
}

void PopupMenuEditor::insert( PopupMenuEditorItem * item, const int index )
{
    if ( index == -1 ) {
	itemList.append( item );
	if ( isVisible() )
	    currentIndex = itemList.count() - 1;
    } else {
	itemList.insert( index, item );
	if ( isVisible() )
	    currentIndex = index;
    }
    resizeToContents();
    if ( isVisible() && parentMenu ) {
	if ( parentMenu->inherits( "PopupMenuEditor" ) )
	     ((PopupMenuEditor*)parentMenu)->currentItem()->setDirty( TRUE );
	parentMenu->update(); // draw arrow in parent menu
    }
    emit inserted( item->anyAction() );
}

void PopupMenuEditor::insert( QAction * action, const int index )
{
    insert( new PopupMenuEditorItem( action, this ), index );
}

void PopupMenuEditor::insert( QActionGroup * actionGroup, const int index )
{
    insert( new PopupMenuEditorItem( actionGroup, this ), index );
}

int PopupMenuEditor::find( const QAction * action )
{
    PopupMenuEditorItem * i = itemList.first();
    while ( i ) {
	if ( i->anyAction() == action )
	    return itemList.at();
	i = itemList.next();
    }
    return -1;
}

int PopupMenuEditor::find( PopupMenuEditor * menu )
{
    PopupMenuEditorItem * i = itemList.first();
    while ( i ) {
	if ( i->subMenu() == menu )
	    return itemList.at();
	i = itemList.next();
    }
    return -1;
}

int PopupMenuEditor::count()
{
    return itemList.count();
}

PopupMenuEditorItem * PopupMenuEditor::at( const int index )
{
    return itemList.at( index );
}

void PopupMenuEditor::exchange( int a, int b )
{
    PopupMenuEditorItem * ia = itemList.at( a );
    PopupMenuEditorItem * ib = itemList.at( b );
    if ( !ia || !ib ||
	 ia == &addItem || ia == &addSeparator ||
	 ib == &addItem || ib == &addSeparator )
	return; // do nothing
    itemList.replace( b, ia );
    itemList.replace( a, ib );
    ia->setDirty( TRUE );
    ib->setDirty( TRUE );
}

void PopupMenuEditor::cut( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );

    if ( clipboardItem && clipboardOperation == Cut )
	delete clipboardItem;

    clipboardOperation = Cut;
    clipboardItem = itemList.at( idx );

    if ( clipboardItem == &addItem || clipboardItem == &addSeparator ) {
	clipboardOperation = None;
	clipboardItem = 0;
	return; // do nothing
    }

    RemoveActionFromPopupCommand * cmd =
	new RemoveActionFromPopupCommand( "Cut Item", formWnd, this, idx );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
}

void PopupMenuEditor::copy( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );

    if ( clipboardItem && clipboardOperation == Cut )
	delete clipboardItem;

    clipboardOperation = Copy;
    clipboardItem = itemList.at( idx );

    if ( clipboardItem == &addItem || clipboardItem == &addSeparator ) {
	clipboardOperation = None;
	clipboardItem = 0;
    }
}

void PopupMenuEditor::paste( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );

    if ( clipboardItem && clipboardOperation ) {
	PopupMenuEditorItem * n = new PopupMenuEditorItem( clipboardItem, this );
	AddActionToPopupCommand * cmd =
	    new AddActionToPopupCommand( "Paste Item", formWnd, this, n, idx );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void PopupMenuEditor::insertedActions( QPtrList<QAction> & list )
{
    QAction * a = 0;
    PopupMenuEditorItem * i = itemList.first();

    while ( i ) {
	a = i->action();
	if ( a )
	    list.append( a );
	i = itemList.next();
    }
}

void PopupMenuEditor::show()
{
    resizeToContents();
    QWidget::show();
}

void PopupMenuEditor::choosePixmap( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );

    PopupMenuEditorItem * i = 0;
    QAction * a = 0;

    if ( idx < (int)itemList.count() ) {
	i = itemList.at( idx );
	a = i->anyAction();
    } else {
	createItem();
    }

    QIconSet icons( qChoosePixmap( 0, formWnd, 0, 0 ) ); // FIXME: check if we need more params
    SetActionIconsCommand * cmd = new SetActionIconsCommand( "Set icon", formWnd, a, this, icons );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
}

void PopupMenuEditor::showLineEdit( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );

    PopupMenuEditorItem * i = 0;

    if ( idx >= (int)itemList.count() )
	i = &addItem;
    else
	i = itemList.at( idx );

    // open edit currentField for item name
    lineEdit->setText( i->anyAction()->menuText() );
    lineEdit->selectAll();
    lineEdit->setGeometry( borderSize + iconWidth, borderSize + itemPos( i ),
			   textWidth, itemHeight( i ) );
    lineEdit->show();
    lineEdit->setFocus();
}

void PopupMenuEditor::setAccelerator( const int key, const Qt::ButtonState state, const int index )
{
    // FIXME: make this a command

    int idx = ( index == -1 ? currentIndex : index );

    if ( key == Qt::Key_Shift ||
	 key == Qt::Key_Control ||
	 key == Qt::Key_Alt ||
	 key == Qt::Key_Meta ||
	 key == Qt::Key_unknown )
	return; // ignore these keys when they are pressed

    PopupMenuEditorItem * i = 0;

    if ( idx >= (int)itemList.count() )
	i = createItem();
    else
	i = itemList.at( idx );

    int shift = ( state & Qt::ShiftButton ? Qt::SHIFT : 0 );
    int ctrl = ( state & Qt::ControlButton ? Qt::CTRL : 0 );
    int alt = ( state & Qt::AltButton ? Qt::ALT : 0 );
    int meta = ( state & Qt::MetaButton ? Qt::META : 0 );

    QAction * a = i->anyAction();
    QKeySequence ks = a->accel();
    int keys[4] = { ks[0], ks[1], ks[2], ks[3] };
    int n = 0;
    while ( n < 4 && ks[n++] );
    n--;
    if ( n < 4 )
	keys[n] = key | shift | ctrl | alt | meta;
    a->setAccel( QKeySequence( keys[0], keys[1], keys[2], keys[3] ) );
    MetaDataBase::setPropertyChanged( a, "accel", TRUE );
    resizeToContents();
}

void PopupMenuEditor::resizeToContents()
{
    QSize s = contentsSize();
    dropLine->resize( s.width(), 2 );
    s.rwidth() += borderSize * 2;
    s.rheight() += borderSize * 2;
    drawAll = 3;
    resize( s );
    // FIXME: the multilevel drawAll is a hack to keep the drawAll flag for two paint events.
    // It was added because the popup gets two paintEvents after the resize, and is erased
    // in between the two ( despite the WRepaintNoErase flag ).
}

void PopupMenuEditor::showSubMenu( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );
    if ( idx < (int)itemList.count() ) {
	itemList.at( idx )->showMenu( pos().x() + width() - borderSize * 3,
				      pos().y() + itemPos( at( currentIndex ) ) +
				      borderSize * 2 );
	setFocus(); // Keep focus in this widget
    }
}

void PopupMenuEditor::hideSubMenu( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );
    if ( idx < (int)itemList.count() )
	itemList.at( idx )->hideMenu();
}

void PopupMenuEditor::focusOnSubMenu( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );
    if ( idx < (int)itemList.count() )
	itemList.at( idx )->focusOnMenu();
}

// This function has no undo. It is only here to remove an item when its action was
// removed from the action editor.
// Use removeItem to put the command on the command stack.
void PopupMenuEditor::remove( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );
    PopupMenuEditorItem * i = itemList.at( idx );
    if ( i && i->isRemovable() ) {
	itemList.remove( idx );
	int n = itemList.count() + 1;
	if ( currentIndex >= n )
	    currentIndex = itemList.count() + 1;
	emit removed( i->anyAction() );
	resizeToContents();
    }
}

PopupMenuEditorItem * PopupMenuEditor::createItem( QAction * a )
{
    PopupMenuEditorItem * i = new PopupMenuEditorItem( a ? a : new QAction( 0 ), this );
    AddActionToPopupCommand * cmd =
	new AddActionToPopupCommand( "Add Item", formWnd, this, i );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
    return i;
}

void PopupMenuEditor::removeItem( const int index )
{
    int idx = ( index == -1 ? currentIndex : index );
    if ( idx < (int)itemList.count() ) {
	RemoveActionFromPopupCommand * cmd = new RemoveActionFromPopupCommand( "Remove Item",
									       formWnd,
									       this,
									       idx );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
	if ( itemList.count() == 0 &&  parentMenu ) {
	    if ( parentMenu->inherits( "PopupMenuEditor" ) )
	     ((PopupMenuEditor*)parentMenu)->currentItem()->setDirty( TRUE );
	    parentMenu->update();
	}
	resizeToContents();
	drawAll++; // one more draw event
    }
}

PopupMenuEditorItem * PopupMenuEditor::currentItem()
{
    int count = itemList.count();
    if ( currentIndex < count )
	return itemList.at( currentIndex );
    else if ( currentIndex == count )
	return &addItem;
    return &addSeparator;
}

PopupMenuEditorItem * PopupMenuEditor::itemAt( const int y )
{
    PopupMenuEditorItem * i = itemList.first();
    int iy = 0;

    while ( i ) {
	iy += itemHeight( i );
	if ( iy > y )
	    return i;
	i = itemList.next();
    }
    iy += itemHeight( &addItem );
    if ( iy > y )
	return &addItem;
    return &addSeparator;
}

void PopupMenuEditor::setFocusAt( const QPoint & pos )
{
    hideSubMenu();
    lineEdit->hide();

    currentIndex = 0;
    int iy = 0;
    PopupMenuEditorItem * i = itemList.first();

    while ( i ) {
	iy += itemHeight( i );
	if ( iy > pos.y() )
	    break;
	i = itemList.next();
	currentIndex++;
    }

    iy += itemHeight( &addItem );
    if ( iy <= pos.y() )
	currentIndex++;

    if ( currentIndex < (int)itemList.count() ) {
	if ( pos.x() < iconWidth )
	    currentField = 0;
	else if ( pos.x() < iconWidth + textWidth )
	    currentField = 1;
	else
	    currentField = 2;
    } else {
	currentField = 1;
    }

    showSubMenu();
}

bool PopupMenuEditor::eventFilter( QObject * o, QEvent * e )
{
    if ( o == lineEdit  && e->type() == QEvent::FocusOut ) {
	leaveEditMode( 0 );
	update();
    }

    return QWidget::eventFilter( o, e );
}

void PopupMenuEditor::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawPrimitive( QStyle::PE_PanelPopup, &p, rect(), colorGroup() );
    drawItems( &p );
}

void PopupMenuEditor::mousePressEvent( QMouseEvent * e )
{
    currentItem()->setDirty( TRUE );
    mousePressPos = e->pos();
    setFocusAt( mousePressPos );
    currentItem()->setDirty( TRUE );
    e->accept();
    update();
}

void PopupMenuEditor::mouseDoubleClickEvent( QMouseEvent * e )
{
    QPoint pos = e->pos();
    currentItem()->setDirty( TRUE );
    setFocusAt( pos );
    currentItem()->setDirty( TRUE );
    if ( currentField == 0 ) {
	choosePixmap();
	resizeToContents();
    } else if ( currentField == 1 ) {
	showLineEdit();
    }
}

void PopupMenuEditor::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() & Qt::LeftButton ) {
	if ( ( e->pos() - mousePressPos ).manhattanLength() > 3 ) {
	    draggedItem = itemAt( mousePressPos.y() );
	    if ( draggedItem == &addItem ) {
		draggedItem = createItem();
		draggedItem->anyAction()->setMenuText( "new item" );
                // FIXME: start rename after drop
	    } else if ( draggedItem == &addSeparator ) {
		draggedItem = createItem( new QSeparatorAction( 0 ) );
		draggedItem->setSeparator( TRUE );
	    }

	    PopupMenuEditorItemPtrDrag * d =
		new PopupMenuEditorItemPtrDrag( draggedItem, this );

	    hideSubMenu();

	    draggedItem->setVisible( FALSE );
	    resizeToContents();

	    // If the item is dropped in the same list,
	    //  we will have two instances of the same pointer
	    // in the list. We use node instead.
	    itemList.find( draggedItem );
	    QLNode * node = itemList.currentNode();

	    d->dragCopy(); // dragevents and stuff happens

	    if ( draggedItem ) { // item was not dropped
		hideSubMenu();
		draggedItem->setVisible( TRUE );
		draggedItem = 0;
		resizeToContents();
		showSubMenu();
	    } else { // item was dropped
		hideSubMenu();
		itemList.takeNode( node )->setVisible( TRUE );
		resizeToContents();
		showSubMenu();
	    }
	}
    }
}

void PopupMenuEditor::dragEnterEvent( QDragEnterEvent * e )
{
    if ( e->provides( "qt/popupmenueditoritemptr" ) ||
	 e->provides( "application/x-designer-actions" ) ||
	 e->provides( "application/x-designer-actiongroup" ) ) {
	e->accept();
	dropLine->show();
    }
}

void PopupMenuEditor::dragLeaveEvent( QDragLeaveEvent * )
{
    dropLine->hide();
}

void PopupMenuEditor::dragMoveEvent( QDragMoveEvent * e )
{
    QPoint pos = e->pos();
    dropLine->move( borderSize, snapToItem( pos.y() ) );

    if ( currentItem() != itemAt( pos.y() ) ) {
	hideSubMenu();
	setFocusAt( pos );
	showSubMenu();
    }
}

void PopupMenuEditor::dropEvent( QDropEvent * e )
{
    if ( !( e->provides( "qt/popupmenueditoritemptr" ) ||
	    e->provides( "application/x-designer-actions" ) ||
	    e->provides( "application/x-designer-actiongroup" ) ) )
	 //e->provides( "application/x-designer-submenu" ) ||
	 //e->provides( "application/x-designer-separator" ) ) )
	return;

    draggedItem = 0;
    PopupMenuEditorItem * i = 0;
    hideSubMenu();

    if ( e->provides( "qt/popupmenueditoritemptr" ) ) {
	PopupMenuEditorItemPtrDrag::decode( e, &i );
    } else {
	if ( e->provides( "application/x-designer-actiongroup" ) ) {
	    QString s( e->encodedData( "application/x-designer-actiongroup" ) );
	    QActionGroup * g = (QDesignerActionGroup*)s.toLong();
	    if ( g->usesDropDown() )
		i = new PopupMenuEditorItem( g, this );
	    else
		dropInPlace( g, e->pos().y() );
	} else if ( e->provides( "application/x-designer-actions" ) ) {
	    QString s( e->encodedData( "application/x-designer-actions" ) );
	    QAction * a = (QDesignerAction*)s.toLong();
	    i = new PopupMenuEditorItem( a, this );
	}
    }

    if ( i )
	dropInPlace( i, e->pos().y() );

    dropLine->hide();
    e->accept();
}

void PopupMenuEditor::keyPressEvent( QKeyEvent * e )
{
    if ( lineEdit->isHidden() ) { // In navigation mode

	switch ( e->key() ) {

	case Qt::Key_Delete:
	    hideSubMenu();
	    removeItem();
	    showSubMenu();
	    break;

	case Qt::Key_Backspace:
	    clearCurrentField();
	    break;

	case Qt::Key_Up:
	    navigateUp( e->state() & Qt::ControlButton );
	    break;

	case Qt::Key_Down:
	    navigateDown( e->state() & Qt::ControlButton );
	    break;

	case Qt::Key_Left:
	    navigateLeft();
	    break;

	case Qt::Key_Right:
	    navigateRight();
	    break;

	case Qt::Key_PageUp:
	    currentIndex = 0;
	    break;

	case Qt::Key_PageDown:
	    currentIndex = itemList.count();
	    break;

	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_F2:
	    enterEditMode( e );
	    // move on
	case Qt::Key_Alt:
	case Qt::Key_Shift:
	case Qt::Key_Control:
	    // do nothing
	    return;

	case Qt::Key_Escape:
	    currentField = 0;
	    navigateLeft();
	    break;

	case Qt::Key_C:
	    if ( e->state() & Qt::ControlButton &&
		 currentIndex < (int)itemList.count() ) {
		copy( currentIndex );
		break;
	    }

	case Qt::Key_X:
	    if ( e->state() & Qt::ControlButton &&
		 currentIndex < (int)itemList.count() ) {
		hideSubMenu();
		cut( currentIndex );
		showSubMenu();
		break;
	    }

	case Qt::Key_V:
	    if ( e->state() & Qt::ControlButton ) {
		hideSubMenu();
		paste( currentIndex < (int)itemList.count() ? currentIndex + 1: itemList.count() );
		showSubMenu();
		break;
	    }

	default:
	    if (  currentItem()->isSeparator() )
		return;
	    if ( currentField == 1 ) {
		showLineEdit();
		QApplication::sendEvent( lineEdit, e );
		e->accept();
		return;
	    } else if ( currentField == 2 ) {
		setAccelerator( e->key(), e->state() );
		showSubMenu();
	    }
	    break;

	}

    } else { // In edit mode
	switch ( e->key() ) {
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Escape:
	    leaveEditMode( e );
	    e->accept();
	    return;
	}
    }
    update();
}

void PopupMenuEditor::focusInEvent( QFocusEvent * )
{
    drawAll = 1;
    showSubMenu();
    update();
}

void PopupMenuEditor::focusOutEvent( QFocusEvent * )
{
    QWidget * fw = qApp->focusWidget();
    if ( !fw || ( !fw->inherits( "PopupMenuEditor" ) && fw != lineEdit ) ) {
	hideSubMenu();
	if ( fw && fw->inherits( "MenuBarEditor" ) )
	    return;
	QWidget * w = this;
	while ( w && w != fw && w->inherits( "PopupMenuEditor" ) ) { // hide all popups
	    w->hide();
	    w = ((PopupMenuEditor *)w)->parentEditor();
	}
    }
}

void PopupMenuEditor::drawItem( QPainter * p, PopupMenuEditorItem * i,
				const QRect & r, const int f ) const
{
    int x = r.x();
    int y = r.y();
    int h = r.height();

    p->fillRect( r, colorGroup().brush( QColorGroup::Background ) );
    i->setDirty( FALSE );

    if ( i->isSeparator() ) {
	style().drawPrimitive( QStyle::PE_Separator, p,
			       QRect( r.x(), r.y() + 2, r.width(), 1 ),
			       colorGroup(), QStyle::Style_Sunken | f );
	return;
    }

    const QAction * a = i->anyAction();
    if ( a->isToggleAction() && a->isOn() ) {
	style().drawPrimitive( QStyle::PE_CheckMark, p,
			       QRect( x , y, iconWidth, h ),
			       colorGroup(), f );
    } else {
	QPixmap icon = a->iconSet().pixmap( QIconSet::Automatic, QIconSet::Normal );
	p->drawPixmap( x + ( iconWidth - icon.width() ) / 2,
		       y + ( h - icon.height() ) / 2,
		       icon );
    }
    x += iconWidth;
    p->drawText( x, y, textWidth, h,
		 QPainter::AlignLeft |
		 QPainter::AlignVCenter |
		 Qt::ShowPrefix |
		 Qt::SingleLine,
		 a->menuText() );

    x += textWidth + borderSize * 3;
    p->drawText( x, y, accelWidth, h,
		 QPainter::AlignLeft | QPainter::AlignVCenter,
		 a->accel() );
    if ( i->count() ) // Item has submenu
	style().drawPrimitive( QStyle::PE_ArrowRight, p,
			       QRect( r.width() - arrowWidth, r.y(), arrowWidth, r.height() ),
			       colorGroup(), f );
}

void PopupMenuEditor::drawWinFocusRect( QPainter * p, const QRect & r ) const
{
    if ( currentIndex < (int)itemList.count() &&
	 ((PopupMenuEditor*)this)->itemList.at( currentIndex )->isSeparator() ) {
	p->drawWinFocusRect( borderSize, r.y(), width() - borderSize * 2, r.height() );
	return;
    }
    int y = r.y();
    int h = r.height();
    if ( currentField == 0 )
	p->drawWinFocusRect( borderSize + 1, y, iconWidth - 2, h );
    else if ( currentField == 1 )
	p->drawWinFocusRect( borderSize + iconWidth, y, textWidth, h );
    else if ( currentField == 2 )
	p->drawWinFocusRect( borderSize + iconWidth + textWidth +
			     borderSize * 3, y, accelWidth, h );
}

void PopupMenuEditor::drawItems( QPainter * p )
{
    int flags = 0;
    int idx = 0;
    bool dragging = dropLine->isVisible();
    QColorGroup enabled = colorGroup();
    QColorGroup disabled = palette().disabled();
    QRect focus;
    QRect rect( borderSize, borderSize, width() - borderSize * 2, 0 );

    PopupMenuEditorItem * i = itemList.first();
    while ( i ) {
	if ( i->isVisible() ) {
	    rect.setHeight( itemHeight( i ) );
	    if ( idx == currentIndex )
		focus = rect;
	    if ( drawAll || i->isDirty() || dragging ) {
		if ( i->anyAction()->isEnabled() ) {
		    flags = QStyle::Style_Enabled;
		    p->setPen( enabled.buttonText() );
		} else {
		    flags = QStyle::Style_Default;
		    p->setPen( disabled.buttonText() );
		}
		drawItem( p, i, rect, flags );
	    }
	    rect.moveBy( 0, rect.height() );
	}
	i = itemList.next();
	idx++;
    }

    // Draw the "add item" and "add separator" items
    p->setPen( darkBlue );
    rect.setHeight( itemHeight( &addItem ) );
    if ( idx == currentIndex )
	focus = rect;
    if ( drawAll || addItem.isDirty() || dragging )
	drawItem( p, &addItem, rect, QStyle::Style_Default );
    rect.moveBy( 0, rect.height() );
    idx++;
    rect.setHeight( itemHeight( &addSeparator ) );
    if ( idx == currentIndex )
	focus = rect;
    if ( drawAll || addSeparator.isDirty() || dragging )
	drawItem( p, &addSeparator, rect, QStyle::Style_Default );
    idx++;

    if ( hasFocus() && !draggedItem )
	drawWinFocusRect( p, focus );

    if ( drawAll > 0 )
	drawAll--;
}

QSize PopupMenuEditor::contentsSize()
{
    QRect textRect = fontMetrics().boundingRect( addSeparator.action()->menuText() );
    textWidth = textRect.width();
    accelWidth = textRect.height(); // default size
    iconWidth = textRect.height();

    int w = 0;
    int h = itemHeight( &addItem ) + itemHeight( &addSeparator );
    PopupMenuEditorItem * i = itemList.first();
    QAction * a = 0;
    while ( i ) {
	if ( i->isVisible() ) {
	    if ( !i->isSeparator() ) {
		a = i->anyAction();
		w = a->iconSet().pixmap( QIconSet::Automatic, QIconSet::Normal ).rect().width() +
		    borderSize; // padding
		iconWidth = QMAX( iconWidth, w );
		w = fontMetrics().boundingRect( a->menuText() ).width();
		textWidth = QMAX( textWidth, w );
		w = fontMetrics().boundingRect( a->accel() ).width();
		accelWidth = QMAX( accelWidth, w );
	    }
	    h += itemHeight( i );
	}
	i = itemList.next();
    }

    int width = iconWidth + textWidth + borderSize * 3 + accelWidth + arrowWidth;
    return QSize( width, h );
}

int PopupMenuEditor::itemHeight( const PopupMenuEditorItem * item ) const
{
    if ( !item || ( item && !item->isVisible() ) )
	return 0;
    if ( item->isSeparator() )
	return 4; // FIXME: hardcoded ( get from styles )r
    int padding =  + borderSize * 6;
    QAction * a = item->anyAction();
    int h = a->iconSet().pixmap( QIconSet::Automatic, QIconSet::Normal ).rect().height();
    h = QMAX( h, fontMetrics().boundingRect( a->menuText() ).height() + padding );
    h = QMAX( h, fontMetrics().boundingRect( a->accel() ).height() + padding );
    return h;
}

int PopupMenuEditor::itemPos( const PopupMenuEditorItem * item ) const
{
    PopupMenuEditor * that = ( PopupMenuEditor * ) this;
    int y = 0;
    PopupMenuEditorItem * i = that->itemList.first();
    while ( i ) {
	if ( i == item )
	    return y;
	y += itemHeight( i );
	i = that->itemList.next();
    }
    return y;
}

int PopupMenuEditor::snapToItem( int y )
{
    int iy = 0;
    int dy = 0;

    PopupMenuEditorItem * i = itemList.first();

    while ( i ) {
	    dy = itemHeight( i );
	    if ( iy + dy / 2 > y )
		return iy;
	    iy += dy;
	i = itemList.next();
    }

    return iy;
}

void PopupMenuEditor::dropInPlace( PopupMenuEditorItem * i, int y )
{
    int iy = 0;
    int dy = 0;
    int idx = 0;

    PopupMenuEditorItem * n = itemList.first();

    while ( n ) {
	    dy = itemHeight( n );
	    if ( iy + dy / 2 > y )
		break;
	    iy += dy;
	idx++;
	n = itemList.next();
    }

    AddActionToPopupCommand * cmd = new AddActionToPopupCommand( "Drop Item", formWnd, this, i, idx );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
    currentIndex = idx;
}

void PopupMenuEditor::dropInPlace( QActionGroup * g, int y )
{
    QObjectList l = *g->children();
    for ( QAction *a = (QAction *)l.last(); a; a = (QAction *)l.prev() ) {
	if ( a->inherits( "ActionGroup" ) )
	    dropInPlace( (QActionGroup *)a, y );
	else
	    dropInPlace( new PopupMenuEditorItem( a, this ), y );
    }
}

void PopupMenuEditor::safeDec()
{
    do  {
	currentIndex--;
    } while ( currentIndex > 0 && !currentItem()->isVisible() );
    if ( currentIndex == 0 &&
	 !currentItem()->isVisible() &&
	 parentMenu ) {
	parentMenu->setFocus();
    }
}

void PopupMenuEditor::safeInc()
{
    int max = itemList.count() + 1;
    if ( currentIndex < max ) {
	do  {
	    currentIndex++;
	} while ( currentIndex < max && !currentItem()->isVisible() ); // skip invisible items
    }
}

void PopupMenuEditor::clearCurrentField()
{
    if ( currentIndex >= (int)itemList.count() )
	return; // currentIndex is addItem or addSeparator
    PopupMenuEditorItem * i = currentItem();
    hideSubMenu();
    if ( i->isSeparator() )
	return;
    if ( currentField == 0 ) {
	QIconSet icons( 0 );
	SetActionIconsCommand * cmd = new SetActionIconsCommand( "Remove icon",
								 formWnd,
								 i->anyAction(),
								 this,
								 icons );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    } else if ( currentField == 2 ) {
	i->anyAction()->setAccel( 0 );
    }
    resizeToContents();
    showSubMenu();
    return;
}

void PopupMenuEditor::navigateUp( bool ctrl )
{
    currentItem()->setDirty( TRUE );
    if ( currentIndex > 0 ) {
	hideSubMenu();
	if ( ctrl ) {
	    ExchangeActionInPopupCommand * cmd =
		new ExchangeActionInPopupCommand( "Move Item Up",
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
	currentItem()->setDirty( TRUE );
	showSubMenu();
    } else if ( parentMenu ) {
	parentMenu->setFocus();
	parentMenu->update();
    }
}

void PopupMenuEditor::navigateDown( bool ctrl )
{
    currentItem()->setDirty( TRUE );
    hideSubMenu();
    if ( ctrl ) {
	if ( currentIndex < ( (int)itemList.count() - 1 ) ) { // safe index
	    ExchangeActionInPopupCommand * cmd =
		new ExchangeActionInPopupCommand( "Move Item Down",
						  formWnd,
						  this,
						  currentIndex,
						  currentIndex + 1 );
	    formWnd->commandHistory()->addCommand( cmd );
	    cmd->execute();
	    safeInc();
	}
    } else { // ! Ctrl
	safeInc();
    }
    if ( currentIndex >= (int)itemList.count() )
	currentField = 1;
    currentItem()->setDirty( TRUE );
    showSubMenu();
}

void PopupMenuEditor::navigateLeft()
{
    currentItem()->setDirty( TRUE );
    if ( currentItem()->isSeparator() ||
	 currentIndex >= (int)itemList.count() ||
	 currentField == 0 ) {
	if ( parentMenu ) {
	    hideSubMenu();
	    parentMenu->setFocus();
	} else if ( !currentItem()->isSeparator() ) {
	    currentField = 2;
	}
    } else {
	currentField--;
    }
}

void PopupMenuEditor::navigateRight()
{
    currentItem()->setDirty( TRUE );
    if ( !currentItem()->isSeparator() &&
	 currentIndex < (int)itemList.count() ) {
	if ( currentField == 2 ) {
	    focusOnSubMenu();
	} else {
	    currentField++;
	    currentField %= 3;
	}
    }
}

void PopupMenuEditor::enterEditMode( QKeyEvent * e )
{
    PopupMenuEditorItem * i = currentItem();

    if ( i == &addSeparator ) {
	i = createItem( new QSeparatorAction( 0 ) );
    } else if ( i->isSeparator() ) {
	return;
    } else if ( currentField == 0 ) {
	choosePixmap();
    } else if ( currentField == 1 ) {
	showLineEdit();
	return;
    } else {// currentField == 2
	setAccelerator( e->key(), e->state() );
    }
    showSubMenu();
    return;
}

void PopupMenuEditor::leaveEditMode( QKeyEvent * e )
{
    setFocus();
    lineEdit->hide();

    if ( e && e->key() == Qt::Key_Escape )
	return;

    PopupMenuEditorItem * i = 0;
    if ( currentIndex >= (int)itemList.count() ) {
	QAction * a = formWnd->mainWindow()->actioneditor()->newActionEx();
	i = createItem( a );
	// Do not put rename on cmd stack (no undo/redo)
	RenameActionCommand rename( "Rename Item", formWnd, i->anyAction(), this, lineEdit->text() );
	rename.execute();
    } else {
	i = itemList.at( currentIndex );
	RenameActionCommand * cmd = new RenameActionCommand( "Rename Item",
							     formWnd,
							     i->anyAction(),
							     this,
							     lineEdit->text() );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }

    resizeToContents();

    if ( i->isSeparator() )
	hideSubMenu();
    else
	showSubMenu();
}
