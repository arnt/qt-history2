#include <qpainter.h>
#include <qrect.h>
#include <qlineedit.h>
#include <qsize.h>
#include <qapplication.h>
#include <qdatastream.h>
#include <qdragobject.h>
#include <qcstring.h>
#include <qobjectlist.h>
#include "popupmenueditor.h"
#include "command.h"
#include "actiondnd.h"
#include "formwindow.h"
#include "formfile.h"
#include "mainwindow.h"
#include "actioneditorimpl.h"
#include "pixmapchooser.h"
#include "metadatabase.h"

// Drag Object Declaration -------------------------------------------

class PopupMenuEditorItemPtrDrag : public QStoredDrag
{
public:
    PopupMenuEditorItemPtrDrag( PopupMenuEditorItem * item, QWidget * parent = 0, const char * name = 0 );
    ~PopupMenuEditorItemPtrDrag() {};
    static bool canDecode( QDragMoveEvent * e );
    static bool decode( QDropEvent * e, PopupMenuEditorItem ** i );
};

// Drag Object Implementation ---------------------------------------

PopupMenuEditorItemPtrDrag::PopupMenuEditorItemPtrDrag( PopupMenuEditorItem * item, QWidget * parent, const char * name )
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

PopupMenuEditorItem::PopupMenuEditorItem( PopupMenuEditor * menu )
    : a( 0 ),
      g( 0 ),
      w( 0 ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( FALSE ),
      autodelete( TRUE ) // We create the action ourselves, so we must delete it.
{
    init();
    a = new QAction( 0 );
    QObject::connect( a, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
}


PopupMenuEditorItem::PopupMenuEditorItem( QAction * action, PopupMenuEditor * menu )
    : a( action ),
      g( 0 ),
      w( 0 ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( TRUE ),
      autodelete( FALSE )
{
    init();
    QObject::connect( a, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
    if ( /*a->name() == "qt_separator_action" ||*/ a->inherits( "QSeparatorAction" ) )
	separator = TRUE;
}

PopupMenuEditorItem::PopupMenuEditorItem( QActionGroup * actionGroup, PopupMenuEditor * menu )
    : a( 0 ),
      g( actionGroup ),
      w( 0 ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( TRUE ),
      autodelete( FALSE )
{
    init();
    QObject::connect( g, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
    g->installEventFilter( this );

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

PopupMenuEditorItem::PopupMenuEditorItem( QWidget * widget, PopupMenuEditor * menu )
    : a( 0 ),
      g( 0 ),
      w( widget ),
      s( 0 ),
      m( menu ),
      separator( FALSE ),
      removable( TRUE ),
      autodelete( FALSE )
{
    init();
}

PopupMenuEditorItem::PopupMenuEditorItem( PopupMenuEditorItem * item, PopupMenuEditor * menu )
    : a( item->a ),
      g( item->g ),
      w( item->w ),
      s( 0 ),
      m( menu ),
      separator( item->separator ),
      removable( item->removable ),
      autodelete( item->autodelete )
{
    init();
    if ( item->type() == ActionGroup ) {
	g->installEventFilter( this );
    }
}

PopupMenuEditorItem::~PopupMenuEditorItem()
{
    if ( autodelete ) {
	if ( a ) {
	    QObject::disconnect( a, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
	    delete a;
	} else if ( g ) {
	    QObject::disconnect( g, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
	    delete g;
	}
	if ( s ) {
	    s->hide();
	    delete s;
	}
    }
}

void PopupMenuEditorItem::init()
{
    QAction * a = anyAction();
    if ( a && m && !isSeparator() ) {
	// FIXME: unify doesn't
	s = new PopupMenuEditor( m->formWindow(), m );
	QString n = QString( a->name() ) + "Menu";
	m->formWindow()->unify( s, n, TRUE );
	s->setName( n );
	MetaDataBase::addEntry( s );
    }
       
    QObject * o = 0;
    if ( a ) {
	o = a;
    } else if ( g ) {
	o = g;
    } else if ( w ) {
	o = w;
    }
    if ( o ) {
	QObject::connect( o, SIGNAL( destroyed() ), this, SLOT( selfDestruct() ) );
    }
}

PopupMenuEditorItem::ItemType PopupMenuEditorItem::type()
{
    if ( separator ) {
	return Separator;
    } else if ( a ) {
	return Action;
    } else if ( g ) {
	return ActionGroup;
    } else if ( w ) {
	return Widget;
    }
    return Unknown;
}

QAction * PopupMenuEditorItem::action()
{
    return a;
}

QActionGroup * PopupMenuEditorItem::actionGroup()
{
    return g;
}

QAction * PopupMenuEditorItem::anyAction()
{
    if ( a ) {
	return a;
    }
    return ( QAction * ) g;
}

QWidget * PopupMenuEditorItem::widget()
{
    return w;
}

void PopupMenuEditorItem::setSeparator( bool enable )
{
    separator = enable;
}

bool PopupMenuEditorItem::isSeparator()
{
    //FIXME check QAction::name for "qt_separator_action"
    return separator;
}

void PopupMenuEditorItem::setVisible( bool enable )
{
    if ( a ) {
	a->setVisible( enable );
    } else if ( g ) {
	g->setVisible( enable );
    } else if ( w ) {
	if ( enable ) {
	    w->show();
	} else {
	    w->hide();
	}
    }
}

bool PopupMenuEditorItem::isVisible()
{
    if ( a ) {
	return a->isVisible();
    } else if ( g ) {
	return ( g->isVisible() && g->usesDropDown() );
    } else if ( w ) {
	return w->isVisible();
    }
    return FALSE;
}

void PopupMenuEditorItem::setRemovable( bool enable )
{
    removable = enable;
}

bool PopupMenuEditorItem::isRemovable()
{
    return removable;
}

void PopupMenuEditorItem::setAutoDelete( bool enable )
{
    autodelete = enable;
}

bool PopupMenuEditorItem::isAutoDelete()
{
    return autodelete;
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
	s->hideCurrentItemMenu();
	s->hide();
    }
}

void PopupMenuEditorItem::focusMenu()
{
    if ( s ) {
	s->showCurrentItemMenu();
	s->setFocus();
    }
}

int PopupMenuEditorItem::count()
{
    if ( s ) {
	return s->count();
    } else if ( g ) {
	const QObjectList * l = g->children();
	if ( l ) {
	    return l->count();
	}
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
	    
	    if ( c->name() == "qt_separator_action" ) {
		i->setSeparator( TRUE );
	    }
	}

	i->setAutoDelete( FALSE );

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
    int i = m->find( anyAction() );//FIXME: support w
    m->remove( i );
    a = 0; // the selfDestruct call was caused by the deletion of one of these items
    g = 0;
    w = 0;
    delete this;
}

// PopupMenuEditor Implementation -----------------------------------

PopupMenuEditorItem * PopupMenuEditor::draggedItem = 0;
int PopupMenuEditor::clipboardOperation = 0;
PopupMenuEditorItem * PopupMenuEditor::clipboardItem = 0;

PopupMenuEditor::PopupMenuEditor( FormWindow * fw, QWidget * parent, const char * name )
    : QWidget( 0, name, WStyle_Customize | WStyle_NoBorder ),
      formWnd( fw ),
      parentMenu( parent ),
      iconWidth( 0 ),
      textWidth( 0 ),
      acceleratorWidth( 0 ),
      arrowWidth( 30 ),
      widgetWidth( 0 ),
      separatorHeight( 4 ),
      itemHeight( 0 ),
      borderSize( 2 ),
      currentField( 1 ),
      currentIndex( 0 )
{
    init();
}

PopupMenuEditor::PopupMenuEditor( FormWindow * fw, PopupMenuEditor * menu, QWidget * parent, const char * name )
    : QWidget( 0, name, WStyle_Customize | WStyle_NoBorder ), // ?
      formWnd( fw ),
      parentMenu( parent ),
      iconWidth( menu->iconWidth ),
      textWidth( menu->textWidth ),
      acceleratorWidth( menu->acceleratorWidth ),
      arrowWidth( menu->arrowWidth ),
      widgetWidth( menu->widgetWidth ),
      separatorHeight( menu->separatorHeight ),
      itemHeight( menu->itemHeight ),
      borderSize( menu->borderSize ),
      currentField( menu->currentField ),
      currentIndex( menu->currentIndex )
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

    // FIXME: move this out of the object
    //QString n = QString( "PopupMenu%1" ).arg( (int)this ); // hack because the name is not unified
    
    addItem.action()->setMenuText( "new item" );
    addSeparator.action()->setMenuText( "new separator" );
    
    setAcceptDrops( TRUE );
    setFocusPolicy( StrongFocus );

    lineEdit = new QLineEdit( this );
    lineEdit->hide();
    lineEdit->setFrame( FALSE );
    lineEdit->setEraseColor( eraseColor() );
    
    dropLine = new QWidget( this, 0, Qt::WStyle_NoBorder | WStyle_StaysOnTop );
    dropLine->setBackgroundColor( Qt::black );
    dropLine->hide();

    hide();

    QObject::connect( formWnd->mainWindow()->actioneditor(), SIGNAL( removing( QAction * ) ),
		      this, SLOT( remove( QAction * ) ) );
}

void PopupMenuEditor::insert( PopupMenuEditorItem * item, int index )
{
    if ( index == -1 ) {
	itemList.append( item );
	//currentIndex = itemList.count() - 1;
    } else {
	itemList.insert( index, item );
	//currentIndex = index;
    }
    resizeToContents();
    if ( isVisible() && parentMenu ) {
	parentMenu->update(); // draw arrow in parent menu
    }
    emit inserted( item->anyAction() );
}

void PopupMenuEditor::insert( QAction * action, int index )
{
    insert( new PopupMenuEditorItem( action, this ), index );
}

void PopupMenuEditor::insert( QActionGroup * actionGroup, int index )
{
    insert( new PopupMenuEditorItem( actionGroup, this ), index );
}

void PopupMenuEditor::insert( QWidget * widget, int index )
{
    insert( new PopupMenuEditorItem( widget, this ), index );
}

int PopupMenuEditor::find( QAction * action )
{
    PopupMenuEditorItem * i = itemList.first();

    while ( i ) {
	
	if ( i->anyAction() == action ) {
	    return itemList.at();
	}	
	i = itemList.next();
    }
    
    return -1;
}

int PopupMenuEditor::count()
{
    return itemList.count();
}

PopupMenuEditorItem * PopupMenuEditor::at( int index )
{
    return itemList.at( index );
}

void PopupMenuEditor::exchange( int a, int b )
{
    PopupMenuEditorItem * ia = itemList.at( a );
    PopupMenuEditorItem * ib = itemList.at( b );
    itemList.replace( b, ia );
    itemList.replace( a, ib );
}

void PopupMenuEditor::cut( int index )
{
    if ( clipboardItem && clipboardOperation == Cut ) {
	delete clipboardItem;
    }
    
    clipboardOperation = Cut;
    clipboardItem = itemList.at( index );
    RemoveActionFromPopupCommand * cmd =
	new RemoveActionFromPopupCommand( "Cut Item", formWnd, this, index );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
}

void PopupMenuEditor::copy( int index )
{
    if ( clipboardItem && clipboardOperation == Cut ) {
	delete clipboardItem;
    }
    
    clipboardOperation = Copy;
    clipboardItem = itemList.at( index );
}

void PopupMenuEditor::paste( int index )
{
    if ( clipboardItem && clipboardOperation ) {
	PopupMenuEditorItem * n = new PopupMenuEditorItem( clipboardItem, this );
	AddActionToPopupCommand * cmd =
	    new AddActionToPopupCommand( "Paste Item", formWnd, this, n, index );
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

void PopupMenuEditor::choosePixmap( int index )
{

    if ( index == -1 ) {
	index = currentIndex;
    }
    
    PopupMenuEditorItem * i = 0;
    QAction * a = 0;

    if ( (uint) index < itemList.count() ) {
	i = itemList.at( index );
	a = i->anyAction();
    } else {
	createItem();
    }

    QIconSet icons( qChoosePixmap( 0, formWnd, 0, 0 ) );
    SetActionIconsCommand * cmd = new SetActionIconsCommand( "Set icon", formWnd, a, this, icons );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
}

void PopupMenuEditor::showLineEdit( int index )
{
    if ( index == -1 ) {
	index = currentIndex;
    }

    PopupMenuEditorItem * i = 0;
    
    if ( (uint) index >= itemList.count() ) {
	i = &addItem;
    } else {
	i = itemList.at( index );
    }
    
    // open edit currentField for item name
    lineEdit->setText( i->anyAction()->menuText() );
    lineEdit->selectAll();
    lineEdit->setGeometry( borderSize + iconWidth, borderSize + currentItemYCoord(),
			   textWidth, itemHeight - 1/* - borderSize*/ );
    lineEdit->show();
    lineEdit->setFocus();
}

void PopupMenuEditor::setAccelerator( int key, Qt::ButtonState state, int index )
{
    // FIXME: make this a command
    if ( index == -1 ) {
	index = currentIndex;
    }

    if ( key == Qt::Key_Shift ||
	 key == Qt::Key_Control ||
	 key == Qt::Key_Alt ||
	 key == Qt::Key_Meta ||
	 key == Qt::Key_unknown )
	return; // ignore these keys when they are pressed

    PopupMenuEditorItem * i = 0;

    if ( (uint) index >= itemList.count() ) {
	createItem();
    } else {
	i = itemList.at( index );
    }

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
    if ( n < 4 ) {
	keys[n] = key | shift | ctrl | alt | meta;
    }
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
    resize( s );
}

void PopupMenuEditor::showCurrentItemMenu()
{
    if ( currentIndex < itemList.count() ) {
	PopupMenuEditorItem * i = itemList.at( currentIndex );
	i->showMenu( pos().x() + width() - borderSize * 3,
		     pos().y() + currentItemYCoord() + borderSize * 2 );
	setFocus();
    }
}

void PopupMenuEditor::hideCurrentItemMenu()
{
    if ( currentIndex < itemList.count() ) {
	PopupMenuEditorItem * i = itemList.at( currentIndex );
	i->hideMenu();
    }
}

void PopupMenuEditor::focusCurrentItemMenu()
{
    if ( currentIndex < itemList.count() ) {
	itemList.at( currentIndex )->focusMenu();
    }
}

void PopupMenuEditor::remove( int index )
{
    PopupMenuEditorItem * i = itemList.at( index );
    if ( i && i->isRemovable() ) {
	itemList.remove( index );
	resizeToContents();
	uint n = itemList.count() + 1;
	if ( currentIndex >= n ) {
	    currentIndex = itemList.count() + 1;
	}
	emit removed( i->anyAction() );
    }
}

void PopupMenuEditor::remove( QAction * a )
{
    remove( find( a ) );
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

void PopupMenuEditor::deleteCurrentItem()
{
    if ( currentIndex < itemList.count() ) {
	RemoveActionFromPopupCommand * cmd = new RemoveActionFromPopupCommand( "Remove Item",
									       formWnd,
									       this,
									       currentIndex );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
    if ( parentMenu ) {
	parentMenu->update();
    }
}

PopupMenuEditorItem * PopupMenuEditor::currentItem()
{
    uint count = itemList.count();
    if ( currentIndex < count ) {
	return itemList.at( currentIndex );
    } else if ( currentIndex == count ) {
	return &addItem;
    }
    return &addSeparator;
}

PopupMenuEditorItem * PopupMenuEditor::itemAt( const int y )
{
    PopupMenuEditorItem * i = itemList.first();
    int iy = 0;

    while ( i ) {

	if ( i->isVisible() ) {	    
	    if ( i->isSeparator() ) {
		iy += separatorHeight;
	    } else {
		iy += itemHeight;
	    }
	}

	if ( iy > y ) {
	    return i;
	}
	
	i = itemList.next();
    }

    iy += itemHeight;
    if ( iy > y ) {
	return &addItem;
    }
    return &addSeparator;
}

void PopupMenuEditor::setFocusAt( const QPoint & pos )
{
    hideCurrentItemMenu();
    
    if ( !lineEdit->isHidden() ) {
	lineEdit->hide();
    }

    currentIndex = 0;
    int iy = 0;
    PopupMenuEditorItem * i = itemList.first();

    while ( i ) {

	if ( i->isVisible() ) {
	    if ( i->isSeparator() ) {
		iy += separatorHeight;
	    } else {
		iy += itemHeight;
	    }
	}

	if ( iy > pos.y() ) {
	    break;
	}
	
	i = itemList.next();
	currentIndex++;
    }

    iy += itemHeight;
    if ( iy <= pos.y() ) {
	currentIndex++;
    }

    if ( currentIndex < itemList.count() ) {
	if ( pos.x() < iconWidth ) {
	    currentField = 0;
	} else if ( pos.x() < iconWidth + textWidth ) {
	    currentField = 1;
	} else {
	    currentField = 2;
	}
    } else {
	currentField = 1;
    }

    showCurrentItemMenu();
}

void PopupMenuEditor::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    drawPopup( p );
    drawItems( p );
}

void PopupMenuEditor::mousePressEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton ) {
	mousePressPos = e->pos();
	setFocusAt( mousePressPos );
	update();
    }
}

void PopupMenuEditor::mouseDoubleClickEvent( QMouseEvent * e )
{
    QPoint pos = e->pos();
    setFocusAt( pos );

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
		draggedItem->anyAction()->setMenuText( "new item" ); // FIXME: start rename after drop
	    } else if ( draggedItem == &addSeparator ) {
		draggedItem = createItem( new QSeparatorAction( 0 ) );
		draggedItem->setSeparator( TRUE );
	    }
	    
	    PopupMenuEditorItemPtrDrag * d =
		new PopupMenuEditorItemPtrDrag( draggedItem, this );

	    hideCurrentItemMenu();

	    draggedItem->setVisible( FALSE );
	    resizeToContents();

	    // If the item is dropped in the same list,
	    //  we will have two instances of the same pointer
	    // in the list. We use node instead.
	    itemList.find( draggedItem );
	    QLNode * node = itemList.currentNode();
	    
	    d->dragCopy(); // dragevents and stuff happens

	    if ( draggedItem ) { // item was not dropped
		hideCurrentItemMenu();
		draggedItem->setVisible( TRUE );
		draggedItem = 0;
		resizeToContents();
		showCurrentItemMenu();
	    } else { // item was dropped
		hideCurrentItemMenu();
		//FIXME: move to centralized node management
		itemList.takeNode( node )->setVisible( TRUE );		
		resizeToContents();
		showCurrentItemMenu();
	    }
	    update();
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
	hideCurrentItemMenu();
	setFocusAt( pos );
	showCurrentItemMenu();
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
    hideCurrentItemMenu(); //FIXME: remove
    
    PopupMenuEditorItem * i = 0;

    if ( e->provides( "qt/popupmenueditoritemptr" ) ) {
	PopupMenuEditorItemPtrDrag::decode( e, &i );
    } else {	
	if ( e->provides( "application/x-designer-actiongroup" ) ) {
	    QString s( e->encodedData( "application/x-designer-actiongroup" ) );
	    QActionGroup * g = (QDesignerActionGroup*)s.toLong();
	    i = new PopupMenuEditorItem( g, this );
	} else if ( e->provides( "application/x-designer-actions" ) ) {
	    QString s( e->encodedData( "application/x-designer-actions" ) );
	    QAction * a = (QDesignerAction*)s.toLong();
	    i = new PopupMenuEditorItem( a, this );
	}
    }

    if ( i )
	dropInPlace( i, e->pos().y() );

    e->accept();
    dropLine->hide();
}

void PopupMenuEditor::keyPressEvent( QKeyEvent * e )
{
    if ( lineEdit->isHidden() ) { // In navigation mode

	switch ( e->key() ) {

	case Qt::Key_Delete:
	    hideCurrentItemMenu();
	    deleteCurrentItem();
	    showCurrentItemMenu();
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
	case Qt::Key_Escape:
	    // do nothing
	    return;
	 
	case Qt::Key_C:
	    if ( e->state() & Qt::ControlButton &&
		 currentIndex < itemList.count() ) {
		copy( currentIndex );
		break;
	    }
	    
	case Qt::Key_X:
	    if ( e->state() & Qt::ControlButton &&
		 currentIndex < itemList.count() ) {
		hideCurrentItemMenu();
		cut( currentIndex );
		showCurrentItemMenu();
		break;
	    }
	    
	case Qt::Key_V:
	    if ( e->state() & Qt::ControlButton ) {
		hideCurrentItemMenu();
		paste( currentIndex < itemList.count() ? currentIndex + 1: itemList.count() );
		showCurrentItemMenu();
		break;
	    }
	    
	default:
	    if (  currentItem()->isSeparator() ) {
		return;
	    }
	    if ( currentField == 1 ) {
		showLineEdit();
		QApplication::sendEvent( lineEdit, e );
		e->accept();
		return;
	    } else if ( currentField == 2 ) {
		setAccelerator( e->key(), e->state() );
		showCurrentItemMenu();
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

void PopupMenuEditor::focusOutEvent( QFocusEvent * )
{
    QWidget * w = qApp->focusWidget();
    if ( !w || ( !( w->inherits( "PopupMenuEditor" ) ||
		    w->inherits( "MenuBarEditor" ) ) &&
		 w != lineEdit ) ) {
	qDebug( "FocusOutEvent" );
	hideCurrentItemMenu();
	hide();
    }
	
}

int PopupMenuEditor::drawAction( QPainter & p, QAction * a, int x, int y )
{
    QPixmap icon = a->iconSet().pixmap( QIconSet::Automatic, QIconSet::Normal );
    
    if ( a->isToggleAction() && a->isOn() ) {
	p.moveTo( iconWidth, y + 1 );
	p.setPen( "white" );
	p.lineTo( iconWidth, y + itemHeight - 2 );
	p.lineTo( 3, y + itemHeight - 2 );
	p.setPen( "black" );
	p.lineTo( 3, y + 1);
	p.lineTo( iconWidth, y + 1 );
	drawToggle( p, y );
    }
	
    p.drawPixmap( x + ( iconWidth - icon.width() ) / 2,
		  y + ( iconWidth - icon.height() ) / 2,
		  icon );
    x += iconWidth;
    p.eraseRect( x, y, textWidth, itemHeight - borderSize ); // erase old text
    p.drawText( x, y, textWidth, itemHeight,
		QPainter::AlignLeft |
		QPainter::AlignVCenter |
		Qt::ShowPrefix |
		Qt::SingleLine,
		a->menuText() );

    x += textWidth + borderSize * 3;
    p.eraseRect( x, y, acceleratorWidth, itemHeight - borderSize ); // erase old text
    p.drawText( x, y, acceleratorWidth, itemHeight,
		QPainter::AlignLeft | QPainter::AlignVCenter,
		a->accel() );

    return itemHeight;
}

int PopupMenuEditor::drawActionGroup( QPainter & p, QActionGroup * g, int x, int y )
{
    int h = 0;

    if ( g->usesDropDown() ) { //FIXME: if the ag is visible, it usesDropDown
	h += drawAction( p, ( QAction * ) g, x, y );
	drawArrow( p, y );
	return h; // we will draw the children in the submenu
    }
    return h;
}

int PopupMenuEditor::drawSeparator( QPainter & p, const int y )
{
    int w = width();
    int m = y + separatorHeight / 2;
    p.setPen( "white" );
    p.drawLine( 1, m, w - 2, m );
    m--;
    p.setPen( "black" );
    p.drawLine( 1, m , w - 2, m );

    return separatorHeight;
}

void PopupMenuEditor::drawArrow( QPainter & p, const int y )
{
    const int my = y + itemHeight / 2;
    const int mx = width() - borderSize - arrowWidth / 2;

    QPointArray a; // FIXME: use bitmap
    
    a.setPoints( 20,
		 mx - 2, my - 4,
		 mx - 2, my - 3,
		 mx - 2, my - 2,
		 mx - 2, my - 1,
		 mx - 2, my,
		 mx - 2, my + 1,
		 mx - 2, my + 2,
		 mx - 2, my + 3,
		 
		 mx - 1, my - 3,
		 mx - 1, my - 2,
		 mx - 1, my - 1,
		 mx - 1, my,
		 mx - 1, my + 1,
		 mx - 1, my + 2,

		 mx, my - 2,
		 mx, my - 1,
		 mx, my,
		 mx, my + 1,

		 mx + 1, my - 1,
		 mx + 1, my );

    p.drawPoints( a );
}

void PopupMenuEditor::drawToggle( QPainter & p, const int y )
{
    const int my = y + itemHeight / 2;
    const int mx = borderSize + iconWidth / 2;

    QPointArray a; // FIXME: use bitmap
        a.setPoints( 21,
		 
		     mx - 3, my - 1,
		     mx - 3, my,
		     mx - 3, my + 1,
		     
		     mx - 2, my,
		     mx - 2, my + 1,
		     mx - 2, my + 2,
		 
		     mx - 1, my + 1,
		     mx - 1, my + 2,
		     mx - 1, my + 3,

		     mx, my,
		     mx, my + 1,
		     mx, my + 2,

		     mx + 1, my - 1,
		     mx + 1, my,
		     mx + 1, my + 1,

		     mx + 2, my - 2,
		     mx + 2, my - 1,
		     mx + 2, my,

		     mx + 3, my - 3,
		     mx + 3, my - 2,
		     mx + 3, my - 1 );
    
    p.drawPoints( a );   
}

void PopupMenuEditor::drawWinFocusRect( QPainter & p, const int y )
{
    if ( currentIndex < itemList.count() &&
	 itemList.at( currentIndex )->isSeparator() ) {
	p.drawWinFocusRect( borderSize, y, width() - borderSize * 2, separatorHeight );
	return;
    }
    if ( currentField == 0 ) {
	p.drawWinFocusRect( borderSize + 1, y + 1, iconWidth - 2, itemHeight - 2 );
    } else if ( currentField == 1 ) {
	p.drawWinFocusRect( borderSize + iconWidth, y + 1, textWidth, itemHeight - 2 );
    } else if ( currentField == 2 ) {
	p.drawWinFocusRect( borderSize + iconWidth + textWidth +
			    borderSize * 2, y + 1, acceleratorWidth, itemHeight - 2 );
    }
    
}

void PopupMenuEditor::drawPopup( QPainter & p )
{
    int w = width() - 1;
    int h = height() - 1;

    // draw border
    p.setPen( "lightgray" );
    p.drawLine( 0, 0, w, 0 );
    p.drawLine( 0, h, 0, 0 );
    p.setPen( "black" );
    p.drawLine( w, 1, w, h );
    p.drawLine( w, h, 1, h );

    w--; h--;
    p.setPen( "white" );
    p.drawLine( 1, 1, w, 1 );
    p.drawLine( 1, h, 1, 1 );
    p.setPen( "black" );
    p.drawLine( w, 2, w, h );
    p.drawLine( w, h, 2, h );
}

void PopupMenuEditor::drawItems( QPainter & p )
{
    int y = borderSize;
    int fy = 0;
    uint c = 0;

    p.setPen( paletteForegroundColor() );
    
    PopupMenuEditorItem * i = itemList.first();
    while ( i ) {
	if ( c == currentIndex ) {
	    fy = y;
	}
	if ( i->isVisible() ) {
	    if ( i->isSeparator() ) {
		y += drawSeparator( p, y );
	    } else {
		if ( i->count() ) {
		    drawArrow( p, y );
		}
		if ( i->type() == PopupMenuEditorItem::Action ) {
		    y += drawAction( p, i->action(), borderSize, y );
		} else if ( i->type() == PopupMenuEditorItem::ActionGroup ) {
		    y += drawActionGroup( p, i->actionGroup(), borderSize, y );
		}
	    }
	}
	i = itemList.next();
	c++;
    }

    p.setPen( "darkgray" );

    int iy = y;
    y += drawAction( p, addItem.action(), borderSize, y );

    int sy = y;
    drawAction( p, addSeparator.action(), borderSize, y ); // add is always the last itemList
    
    if ( hasFocus() && !draggedItem ) {
	if ( fy ) {
	    drawWinFocusRect( p, fy );
	} else if ( currentIndex == itemList.count() ) {
	    drawWinFocusRect( p, iy );
	} else if ( currentIndex > itemList.count() ) {
	    drawWinFocusRect( p, sy );
	}
    }
}

QSize PopupMenuEditor::contentsSize()
{
    QPainter p( this );
    int b2 = borderSize * 2;
    
    QRect textRect = p.fontMetrics().boundingRect( addSeparator.action()->menuText() );
    textWidth = textRect.width() + b2;
    itemHeight = textRect.height() + b2 * 4;
    acceleratorWidth = itemHeight;
    iconWidth = itemHeight;

    int height = itemHeight * 2; // addItem and addSeparator
    
    PopupMenuEditorItem * i = itemList.first();
    while ( i ) {
	height += itemSize( p, i );
	i = itemList.next();
    }

    int maxItemWidth = iconWidth + textWidth + borderSize + acceleratorWidth + arrowWidth;
    int maxWidgetWidth = widgetWidth + arrowWidth;
    int maxWidth = ( maxItemWidth > maxWidgetWidth ) ? maxItemWidth : maxWidgetWidth;
    return QSize( maxWidth, height );
}

int PopupMenuEditor::itemSize( QPainter & p, PopupMenuEditorItem * i )
{
    if ( i->isVisible() ) {	

	if ( i->isSeparator() ) {
	    return separatorHeight;
	}
	if ( i->type() == PopupMenuEditorItem::Action ) {
	    return actionSize( p, i->action() );
	}
	if ( i->type() == PopupMenuEditorItem::ActionGroup ) {
	    return actionGroupSize( p, i->actionGroup() );
	}
	
    }
    return 0;
}

int PopupMenuEditor::actionSize( QPainter & p, QAction * a )
{
    int b2 = borderSize * 2;
    
    // get the largest bitmap dimensions
    QPixmap pm = a->iconSet().pixmap( QIconSet::Automatic, QIconSet::Normal );
    int iw = pm.width() + b2;
    if ( iw > iconWidth ) {
	iconWidth = iw;
    }
    int ih = pm.height() + b2;
    if ( ih > itemHeight ) {
	itemHeight = ih;
    }
		
    //  get the widest item text
    int tw = p.boundingRect( rect(), Qt::ShowPrefix, a->menuText() ).width() + b2;
    if ( tw > textWidth )
	textWidth = tw;

    // get the widest keyboard accelerator text
    int sw = p.boundingRect( rect(), 0, a->accel() ).width() + b2;
    if ( sw > acceleratorWidth )
	acceleratorWidth = sw;

    return itemHeight;
}

int PopupMenuEditor::actionGroupSize( QPainter & p, QActionGroup * g )
{
    return actionSize( p, ( QAction * ) g ); // the item itself
}

int PopupMenuEditor::currentItemYCoord()
{
    uint c = 0;
    int y = 0;
    
    PopupMenuEditorItem * i = itemList.first();
 
    while ( i ) {

	if ( c == currentIndex )
	    return y;
	c++;

	if ( i->isVisible() ) {
	    
	    if ( i->isSeparator() ) {
		y += separatorHeight;
	    } else {
		y += itemHeight;
	    }
	}
	i = itemList.next();
    }
    
    return y;
}

int PopupMenuEditor::snapToItem( int y )
{
    int iy = 0;
    int dy = 0;

    PopupMenuEditorItem * i = itemList.first();

    while ( i ) {
	if ( i->isVisible() ) {
	    if ( i->isSeparator() ) {
		dy = separatorHeight;
	    } else {
		dy = itemHeight;
	    } 
	    if ( iy + dy / 2 > y ) {
		return iy;
	    }
	    iy += dy;
	}
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
	if ( n->isVisible() ) {
	    if ( n->isSeparator() ) {
		dy = separatorHeight;
	    } else {
		dy = itemHeight;
	    }
	    if ( iy + dy / 2 > y ) {
		break;
	    }
	    iy += dy;   
	}
	idx++;
	n = itemList.next();
    }
    
    AddActionToPopupCommand * cmd = new AddActionToPopupCommand( "Drop Item", formWnd, this, i, idx );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
    currentIndex = idx;
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
    uint max = itemList.count() + 1;
    if ( currentIndex < max ) {
	do  {
	    currentIndex++;
	} while ( currentIndex < max && !currentItem()->isVisible() ); // skip invisible items
    }
}

void PopupMenuEditor::clearCurrentField()
{
    if ( currentIndex >= itemList.count() ) {
	return; // currentIndex is addItem or addSeparator
    }
    PopupMenuEditorItem * i = currentItem();
    hideCurrentItemMenu();
    if ( i->isSeparator() ) {
	return;
    } else if ( currentField == 0 ) {
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
    showCurrentItemMenu();
    return;
}

void PopupMenuEditor::navigateUp( bool ctrl )
{
    if ( currentIndex > 0 ) {
	hideCurrentItemMenu();
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
	showCurrentItemMenu();
    } else if ( parentMenu ) {
	parentMenu->update(); //FIXME
	parentMenu->setFocus();
    }
}

void PopupMenuEditor::navigateDown( bool ctrl )
{	    
    hideCurrentItemMenu();
    if ( ctrl ) {
	if ( currentIndex < ( itemList.count() - 1 ) ) { // safe index
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
    if ( currentIndex >= itemList.count() ) {
	currentField = 1;
    }
    showCurrentItemMenu();
}

void PopupMenuEditor::navigateLeft()
{
    if ( currentItem()->isSeparator() ||
	 currentIndex >= itemList.count() ||
	 currentField == 0 ) {
	if ( parentMenu ) {
	    hideCurrentItemMenu();
	    parentMenu->setFocus();
	    parentMenu->update();
	} else if ( !currentItem()->isSeparator() ) {
	    currentField = 2;
	}
    } else {
	currentField--;
    }
}

void PopupMenuEditor::navigateRight()
{
    if ( !currentItem()->isSeparator() && currentIndex < itemList.count() ) {
	if ( currentField == 2 ) {
	    focusCurrentItemMenu();
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
	i->setSeparator( TRUE );
    } else if ( i->isSeparator() ) {
	return;
    } else if ( currentField == 0 ) {
	choosePixmap();
    } else if ( currentField == 1 ) {
	showLineEdit();
	return;
    } else { // currentField == 2
	setAccelerator( e->key(), e->state() );
    }
    showCurrentItemMenu();
    return;
}

void PopupMenuEditor::leaveEditMode( QKeyEvent * e )
{
    setFocus();
    lineEdit->hide();

    if ( e->key() == Qt::Key_Escape ) {
	return;
    }
    
    PopupMenuEditorItem * i = 0;    
    if ( currentIndex >= itemList.count() ) {
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
    
    if ( i->isSeparator() ) {
	hideCurrentItemMenu();
    } else {
	showCurrentItemMenu();		
    }

    update();
}
