#include "formeditor.h"
#include "dlayout.h"
#include "xml.h"
#include "connectdlg.h"
#include "windowmanager.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qobjectlist.h>
#include <qvaluelist.h>
#include <qtl.h>
#include <qpopupmenu.h>
#include <qmetaobject.h>
#include <qpainter.h>
#include <qresource.h>
#include <qtextstream.h>

// Needed since they require special treatment from the builder
#include <qgroupbox.h>
#include <qtabwidget.h>
#include <qwizard.h>
#include <qpushbutton.h>

DFormEditor* DFormEditor::s_pLoadingInstance = 0;

/**********************************************
 *
 * DFormEditor
 *
 **********************************************/

DFormEditor::DFormEditor( QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  m_pPopupWidget = 0;
  m_pConnectWidget = 0;

  QGridLayout* grid = new QGridLayout( this, 1, 1, 6, 0 );
  DWindowManager* frame = new DWindowManager( this );
  grid->addWidget( frame, 0, 0 );

  // HACK
  QDialog *dlg = new QDialog( frame );
  m_topLevelWidget = dlg;

  addWidget( dlg );
  QGridLayout* g = new QGridLayout( dlg, 1, 1, 0, 0 );
  DFormWidget* form = new DFormWidget( DFormWidget::Container, this, dlg );
  g->addWidget( form, 0, 0 );
  dlg->reparent( frame, 0, QPoint( 0, 0 ), TRUE );

  QVBoxLayout *vbox = new QVBoxLayout( frame, 2, 0 );
  vbox->addSpacing( 20 );
  vbox->addWidget( dlg );

  qApp->installEventFilter( this );
}

DFormEditor::DFormEditor( const QResource& _resource, QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  m_pPopupWidget = 0;
  m_pConnectWidget = 0;
  
  s_pLoadingInstance = this;

  QGridLayout* grid = new QGridLayout( this, 1, 1, 6, 0 );
  DWindowManager* frame = new DWindowManager( this );
  grid->addWidget( frame, 0, 0 );

  m_topLevelWidget = _resource.createWidget( this );
  if ( !m_topLevelWidget )
    return;

  addWidget( m_topLevelWidget, _resource.tree() );
  m_topLevelWidget->reparent( frame, 0, QPoint( 0, 0 ), TRUE );

  QVBoxLayout *vbox = new QVBoxLayout( frame, 2, 0 );
  vbox->addSpacing( 20 );
  vbox->addWidget( m_topLevelWidget );

  QObject* o = m_topLevelWidget->child( 0, "QMenuBar" );
  if ( o )
    addWidget( (QWidget*)o );

  qApp->installEventFilter( this );
}

DFormEditor::~DFormEditor()
{
}

QResourceItem* DFormEditor::save()
{
  // Write the widgets and layouts
  QResourceItem* t =  qObjectToXML( findInfo( m_topLevelWidget ) );

  // Save connections of all widgets
  QPtrDictIterator<DObjectInfo> pit( m_widgets );
  for( ; pit.current(); ++pit )
  {
    QValueList<DObjectInfo::Connection>::Iterator it = pit.current()->connectionsBegin();
    QValueList<DObjectInfo::Connection>::Iterator end = pit.current()->connectionsEnd();
    for( ; it != end; ++it )
    {
      QResourceItem* c = new QResourceItem( "Connection" );
      c->insertAttrib( "sender", it->senderName );
      c->insertAttrib( "signal", it->signal );
      c->insertAttrib( "receiver", it->receiverName );
      if ( it->slotIsSignal )
	c->insertAttrib( "transmitter", it->slot );
      else
	c->insertAttrib( "slot", it->slot );
      t->append( c );
    }
  }

  return t;
}

void DFormEditor::addWidget( QWidget* _w )
{
  ASSERT( m_widgets[ _w ] == 0 );

  m_widgets.insert( _w, new DObjectInfo( _w ) );
}

void DFormEditor::addWidget( QWidget* _w, const QResourceItem* item )
{
  ASSERT( m_widgets[ _w ] == 0 );

  m_widgets.insert( _w, new DObjectInfo( _w, item ) );
}

DObjectInfo* DFormEditor::info( const QString& _obj_name )
{
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
  {
    if ( _obj_name == it.current()->widget()->name() )
      return it.current();
  }

  return 0;
}

void DFormEditor::slotConnect()
{
  ASSERT( m_pPopupWidget );

  m_pConnectWidget = m_pPopupWidget;
  m_pPopupWidget = 0;

  QApplication::setOverrideCursor( Qt::pointingHandCursor );
}

bool DFormEditor::eventFilter( QObject* _obj, QEvent* _ev )
{
  // The kind of events we are interested in ...
  if ( _ev->type() == QEvent::MouseButtonPress ||
       _ev->type() == QEvent::MouseButtonRelease ||
       _ev->type() == QEvent::MouseMove )
  {
    DObjectInfo* connect_info = 0;
    // Clear the connectwidget pointer since this mouse press finishes
    // or cancels the connect-action anyway.
    if ( _ev->type() == QEvent::MouseButtonPress )
    {
      connect_info = m_pConnectWidget;
      if ( m_pConnectWidget )
	QApplication::restoreOverrideCursor();
      m_pConnectWidget = 0;
    }

    QMouseEvent* me = (QMouseEvent*)_ev;

    /**
     * Look at the destination of this event
     */

    // Skip these classes
    if ( _obj->inherits( "DSizeHandle" ) || _obj->inherits( "QPopupMenu" ) )
      return FALSE;

    // Special hack for the wizard. We want to allow the buttons
    // "back" and "next" to work.
    if ( _obj->parent() && _obj->parent()->inherits("QWizard") )
      if ( _obj == ((QWizard*)_obj->parent())->backButton() ||
	   _obj == ((QWizard*)_obj->parent())->nextButton() )
	return FALSE;

    // Find info about this widget
    DObjectInfo* info = m_widgets.find( _obj );
    // We dont know it ? So try some special cases
    if ( !info )
    { 
      // Is it the top most form ?
      if ( _obj->inherits( "DFormWidget" ) && ((DFormWidget*)_obj)->isTopMostForm() )
      {
	if( _ev->type() == QEvent::MouseButtonPress )
	  unselectAll();
	return FALSE;
      }
      
      // Try to find out wether the direct or indirect  parent is known to us
      QObject* o = _obj;
      while( !info && o->parent() )
      {
	o = o->parent();
	info = m_widgets.find( o );
      }
      // still no idea ?
      if ( !info )
      {
	// Test wether somene clicked on the faked title bar
	if ( _obj->inherits( "DWindowManager" ) && m_topLevelWidget->parent() == _obj )
	  info = m_widgets.find( m_topLevelWidget );
	// No idea? => Forget it
	if ( !info )
	  return FALSE;
      }
    }

    printf("I know that object\n");

    /**
     * Ok, we know this widget.
     */

    // HACK
    if ( info->widget()->inherits( "DListView" ) && me->button() == RightButton )
      return FALSE;

    // Mouse pressed ?
    if ( _ev->type() == QEvent::MouseButtonPress && _obj->inherits( "QWidget" ) )
    {
      if ( me->state() & ( Qt::ControlButton | Qt::ShiftButton ) )
	// Additional selection
	info->setSelected( !info->isSelected() );
      else if ( me->button() == Qt::RightButton && !_obj->inherits( "DFormWidget" ) )
      {
	m_pPopupWidget = info;

	QPopupMenu* popup = new QPopupMenu( topLevelWidget() );
	popup->insertItem( tr("Connect ..."), this, SLOT( slotConnect() ) );
	QPoint pos = ((QWidget*)_obj)->mapToGlobal( me->pos() );
	popup->popup( pos );
      }
      else if ( me->button() == Qt::LeftButton )
      {
	// Exclusive selection
	selectObjectExclusive( info );
      }
    }

    // Are we in a connect-action right now ?
    if ( me->button() == Qt::LeftButton && connect_info && _obj->inherits( "QWidget" ) )
    {
      // One can not connect with a form widget.
      if ( _obj->inherits( "DFormWidget" ) )
      {
	// TODO: give error message
	return TRUE;
      }
      
      // TODO: Kill this dialog when sender/receiver are deleted or the
      // editor is deleted ...
      DConnectDlg* dlg = new DConnectDlg( this, connect_info, info );
      dlg->show();

      // Eat that event
      return TRUE;
    }

    // Never (almost) eat event from the form widget.
    if ( _obj->inherits( "DFormWidget" ) && ( me->button() != Qt::LeftButton ||
	 ( !((DFormWidget*)_obj)->isLayouted() && ((DFormWidget*)_obj)->mode() != DFormWidget::LayoutHelper ) ) )
      return FALSE;

    return TRUE;
  }

  // We are not interested in this event ...
  return FALSE;
}

void DFormEditor::selectObjectExclusive( DObjectInfo* _info )
{
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
  {
    if ( _info != it.current() )
      it.current()->setSelected( FALSE );
  }

  if ( _info )
  {    
    _info->setSelected( TRUE );
    emit objectSelected( this, _info );
  }
}

void DFormEditor::slotAutoArrange()
{
  /* QList<QWidget> selected;

  // Check wether all selected widgets have the same parent
  QObject* p = 0;
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
  {
    if ( it.current()->isSelected() )
    {
      if ( p == 0 )
	p = ((QObject*)it.currentKey())->parent();
      else if ( p != ((QObject*)it.currentKey())->parent() )
      {
	qDebug("Error: NOT ALL IN SAME PARENT");
	// TODO: Error message
	return;
      }
      selected.append( (QWidget*)it.currentKey() );
    }
  }

  // Is anything selected at all ?
  if ( selected.isEmpty() )
  {
    qDebug("Error: NOTHING SELECTED");
    // TODO: Error message
    return;
  }

  // We must get rid of the size handles
  unselectAll();

  dGuessGrid( selected ); */
}

void DFormEditor::slotGridArrange()
{
  simpleArrange( DFormWidget::GridLayout );
}

void DFormEditor::slotHArrange()
{
  simpleArrange( DFormWidget::HBoxLayout );
}

void DFormEditor::slotVArrange()
{
  simpleArrange( DFormWidget::VBoxLayout );
}

void DFormEditor::simpleArrange( DFormWidget::Layout _l )
{
  QList<QWidget> selected;

  // Check wether all selected widgets have the same parent
  QObject* p = 0;
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
  {
    if ( it.current()->isSelected() )
    {
      if ( p == 0 )
	p = ((QObject*)it.currentKey())->parent();
      else if ( p != ((QObject*)it.currentKey())->parent() )
      {
	qDebug("Error: NOT ALL IN SAME PARENT");
	// TODO: Error message
	return;
      }
      selected.append( (QWidget*)it.currentKey() );
    }
  }

  // Is anything selected at all ?
  if ( selected.isEmpty() )
  {
    qDebug("Error: NOTHING SELECTED");
    // TODO: Error message
    return;
  }

  // We must get rid of the size handles
  unselectAll();

  // Create a new form widget
  QWidget* par = selected.first()->parentWidget();
  ASSERT( par != 0 );
  DFormWidget* f = new DFormWidget( DFormWidget::LayoutHelper, this, par );

  // Find upper left corner for form widget
  int xpos = selected.first()->x();
  int ypos = selected.first()->y();
  QListIterator<QWidget> it2( selected );
  for( ; it2.current(); ++it2 )
  {
    if ( it2.current()->x() < xpos )
      xpos = it2.current()->x();
    if ( it2.current()->y() < ypos )
      ypos = it2.current()->y();
  }

  // Reparent to form widget
  it2.toFirst();
  for( ; it2.current(); ++it2 )
  {
    it2.current()->reparent( f, 0, QPoint( it2.current()->x() - xpos, it2.current()->y() - ypos ), TRUE );
  }

  // Now arrange them
  f->setLayout( _l );
  f->setGeometry( QRect( QPoint( xpos, ypos ), f->sizeHint() ) );
  f->show();

  if ( par->inherits( "DFormWidget" ) )
    ((DFormWidget*)par)->updateLayout();

  addWidget( f );
}

void DFormEditor::slotApplySizeHint()
{
  // Check wether all selected widgets have the same parent
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
  {
    if ( it.current()->isSelected() )
    {
      ((QWidget*)it.currentKey())->resize( ((QWidget*)it.currentKey())->sizeHint() );
      it.current()->update();
    }
  }
}

void DFormEditor::updateSizeHandles()
{
  // Check wether all selected widgets have the same parent
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
    if ( it.current()->isSelected() )
      it.current()->update();
}

QValueList<DObjectInfo::Connection> DFormEditor::connectionsToReceiver( const QString& _receiver )
{
  QValueList<DObjectInfo::Connection> list;

  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
  {
    QValueList<DObjectInfo::Connection>::Iterator it2 = it.current()->connectionsBegin();
    for( ; it2 != it.current()->connectionsEnd(); ++it2 )
      if ( it2->receiverName == _receiver )
	list.append( *it2 );
  }

  return list;
}

/**********************************************
 *
 * DObjectInfo
 *
 **********************************************/

DObjectInfo::DObjectInfo( QWidget* _widget )
{
  m_widget = _widget;
  m_selected = FALSE;

  // Initialize all size handles
  for( int i = 0; i < 8; ++i )
    m_sizeHandles[i] = 0;
}

DObjectInfo::DObjectInfo( QWidget* _widget, const QResourceItem* item )
{
  m_widget = _widget;
  m_selected = FALSE;

  // Initialize all size handles
  for( int i = 0; i < 8; ++i )
    m_sizeHandles[i] = 0;

  if ( !item )
    return;

  // Find out which custom properties are set
  QStringList props = m_widget->metaObject()->propertyNames();
  QStringList::Iterator it = props.begin();
  for( ; it != props.end(); ++it )
  {
    QMetaProperty* p = m_widget->metaObject()->property( *it, TRUE );
    if ( p && !p->readonly )
    {
      QProperty prop = item->property( p );
      if ( !prop.isEmpty() )
	m_props.insert( p->name, prop );
    }
  }
}

DObjectInfo::~DObjectInfo()
{
  for( int i = 0; i < 8; ++i )
    if ( m_sizeHandles[i] != 0 )
      delete m_sizeHandles[i];
}

void DObjectInfo::removeConnection( const Connection& _connection )
{
  m_connections.remove( _connection );
}

void DObjectInfo::startMove()
{
  m_originalGeometry = m_widget->geometry();
}

void DObjectInfo::endMove()
{
}

void DObjectInfo::move( int _orientation, int _dx, int _dy )
{
  switch( _orientation )
  {
  case 0:
    if ( _dx < 0 )
      _dx = QMAX( _dx, m_originalGeometry.width() - m_widget->maximumWidth() );
    else
      _dx = QMIN( _dx, m_originalGeometry.width() - QMAX( 1, m_widget->minimumWidth() ) );
    if ( _dy < 0 )
      _dy = QMAX( _dy, m_originalGeometry.height() - m_widget->maximumHeight() );
    else
      _dy = QMIN( _dy, m_originalGeometry.height() - QMAX( 1, m_widget->minimumHeight() ) );
    m_widget->setGeometry( m_originalGeometry.x() + _dx, m_originalGeometry.y() + _dy,
			   m_originalGeometry.width() - _dx, m_originalGeometry.height() - _dy );
    break;
  case 1:
    if ( _dy < 0 )
      _dy = QMAX( _dy, m_originalGeometry.height() - m_widget->maximumHeight() );
    else
      _dy = QMIN( _dy, m_originalGeometry.height() - QMAX( 1, m_widget->minimumHeight() ) );
    m_widget->setGeometry( m_originalGeometry.x(), m_originalGeometry.y() + _dy,
			   m_originalGeometry.width(), m_originalGeometry.height() - _dy );
    break;
  case 2:
    if ( _dx > 0 )
      _dx = QMIN( _dx, m_widget->maximumWidth() - m_originalGeometry.width() );
    else
      _dx = QMAX( _dx, QMAX( 1, m_widget->minimumWidth() ) - m_originalGeometry.width() );
    if ( _dy < 0 )
      _dy = QMAX( _dy, m_originalGeometry.height() - m_widget->maximumHeight() );
    else
      _dy = QMIN( _dy, m_originalGeometry.height() - QMAX( 1, m_widget->minimumHeight() ) );
    m_widget->setGeometry( m_originalGeometry.x(), m_originalGeometry.y() + _dy,
			   m_originalGeometry.width() + _dx, m_originalGeometry.height() - _dy );
    break;
  case 3:
    if ( _dx > 0 )
      _dx = QMIN( _dx, m_widget->maximumWidth() - m_originalGeometry.width() );
    else
      _dx = QMAX( _dx, QMAX( 1, m_widget->minimumWidth() ) - m_originalGeometry.width() );
    m_widget->setGeometry( m_originalGeometry.x(), m_originalGeometry.y(),
			   m_originalGeometry.width() + _dx, m_originalGeometry.height() );
    break;
  case 4:
    if ( _dx > 0 )
      _dx = QMIN( _dx, m_widget->maximumWidth() - m_originalGeometry.width() );
    else
      _dx = QMAX( _dx, QMAX( 1, m_widget->minimumWidth() ) - m_originalGeometry.width() );
    if ( _dy > 0 )
      _dy = QMIN( _dy, m_widget->maximumHeight() - m_originalGeometry.height() );
    else
      _dy = QMAX( _dy, QMAX( 1, m_widget->minimumHeight() ) - m_originalGeometry.height() );
    m_widget->setGeometry( m_originalGeometry.x(), m_originalGeometry.y(),
			   m_originalGeometry.width() + _dx, m_originalGeometry.height() + _dy );
    break;
  case 5:
    if ( _dy > 0 )
      _dy = QMIN( _dy, m_widget->maximumHeight() - m_originalGeometry.height() );
    else
      _dy = QMAX( _dy, QMAX( 1, m_widget->minimumHeight() ) - m_originalGeometry.height() );
    m_widget->setGeometry( m_originalGeometry.x(), m_originalGeometry.y(),
			   m_originalGeometry.width(), m_originalGeometry.height() + _dy );
    break;
  case 6:
    if ( _dx < 0 )
      _dx = QMAX( _dx, m_originalGeometry.width() - m_widget->maximumWidth() );
    else
      _dx = QMIN( _dx, m_originalGeometry.width() - QMAX( 1, m_widget->minimumWidth() ) );
    if ( _dy > 0 )
      _dy = QMIN( _dy, m_widget->maximumHeight() - m_originalGeometry.height() );
    else
      _dy = QMAX( _dy, QMAX( 1, m_widget->minimumHeight() ) - m_originalGeometry.height() );
    m_widget->setGeometry( m_originalGeometry.x() + _dx, m_originalGeometry.y(),
			   m_originalGeometry.width() - _dx, m_originalGeometry.height() + _dy );
    break;
  case 7:
    if ( _dx < 0 )
      _dx = QMAX( _dx, m_originalGeometry.width() - m_widget->maximumWidth() );
    else
      _dx = QMIN( _dx, m_originalGeometry.width() - QMAX( 1, m_widget->minimumWidth() ) );
    m_widget->setGeometry( m_originalGeometry.x() + _dx, m_originalGeometry.y(),
			   m_originalGeometry.width() - _dx, m_originalGeometry.height() );
    break;
  default:
    ASSERT( 0 );
  }

  updateSizeHandles();
}

void DObjectInfo::setSelected( bool _mode )
{
  if ( m_selected == _mode )
    return;

  m_selected = _mode;

  if ( !_mode )
  {
    for( int i = 0; i < 8; ++i )
      if ( m_sizeHandles[i] != 0 )
	delete m_sizeHandles[i];
    return;
  }

  bool active = TRUE;
  if ( m_widget->parentWidget()->inherits( "DFormWidget" ) &&
       ((DFormWidget*)m_widget->parentWidget())->isLayouted() )
    active = FALSE;

  for( int i = 0; i < 8; ++i )
  {
    m_sizeHandles[i] = new DSizeHandle( i, active, this, m_widget->parentWidget() );
    m_sizeHandles[i]->show();
    m_sizeHandles[i]->raise();
  }

  updateSizeHandles();
}

void DObjectInfo::updateSizeHandles()
{
  if ( !m_selected )
    return;

  int x = m_widget->x();
  int y = m_widget->y();

  m_sizeHandles[0]->move( x - 3, y - 3 );
  m_sizeHandles[1]->move( x + m_widget->width() / 2 - 3, y - 3 );
  m_sizeHandles[2]->move( x + m_widget->width() - 4, y - 3 );
  m_sizeHandles[3]->move( x + m_widget->width() - 4, y + m_widget->height() / 2 - 3 );
  m_sizeHandles[4]->move( x + m_widget->width() - 4, y + m_widget->height() - 4 );
  m_sizeHandles[5]->move( x + m_widget->width() / 2 - 3, y + m_widget->height() - 4 );
  m_sizeHandles[6]->move( x - 3, y + m_widget->height() - 4 );
  m_sizeHandles[7]->move( x - 3, y + m_widget->height() / 2 - 3 );
}

QProperty* DObjectInfo::property( const QString& name )
{
  QMap<QString,QProperty>::Iterator it = m_props.find( name );
  if ( it != m_props.end() )
    return &it.data();
  
  return 0;
}

void DObjectInfo::removeProperty( const QString& name )
{
  m_props.remove( name );
  m_widget->setProperty( name, DWidgetInfo::defaultProperty( m_widget->className(), name ) );
}

/**********************************************
 *
 * DSizeHandle
 *
 **********************************************/

DSizeHandle::DSizeHandle( int _orientation, bool _active, DObjectInfo* _info, QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  m_info = _info;
  m_orientation = _orientation;

  if ( _active )
  {
    switch( _orientation )
    {
    case 0: // Upper left
    case 4: // Lower right
      setCursor( Qt::sizeFDiagCursor );
      break;
    case 2: // Upper right
    case 6: // Lower left
      setCursor( Qt::sizeBDiagCursor );
      break;
    case 1:
    case 5:
      setCursor( Qt::sizeVerCursor );
      break;
    case 3:
    case 7:
      setCursor( Qt::sizeHorCursor );
      break;
    default:
      ASSERT( 0 );
    }
  }

  m_active = _active;
  if ( _active )
    setBackgroundColor( Qt::black );
  else
    setBackgroundColor( Qt::gray );

  resize( 6, 6 );
}

DSizeHandle::~DSizeHandle()
{
}

void DSizeHandle::mousePressEvent( QMouseEvent* _ev )
{
  if ( !m_active )
    return;

  m_mousePos = _ev->globalPos();
  m_info->startMove();
}

void DSizeHandle::mouseMoveEvent( QMouseEvent* _ev )
{
  if ( !m_active )
    return;

  QPoint p = _ev->globalPos();

  int dx = p.x() - m_mousePos.x();
  int dy = p.y() - m_mousePos.y();

  m_info->move( m_orientation, dx, dy );
}

void DSizeHandle::mouseReleaseEvent( QMouseEvent* )
{
  m_info->endMove();
}

/**********************************************
 *
 * DFormWidget
 *
 **********************************************/

DFormWidget::DFormWidget( QWidget* parent, const QResource& _resource )
  : QWidget( parent, _resource )
{
  m_mode = DFormWidget::TopMost;
  if ( _resource.tree()->hasAttrib( "__mode" ) )
  {
    QString str = _resource.tree()->attrib( "__mode" );
    if ( str == "Container" )
      m_mode = DFormWidget::Container;
    else if ( str == "TopMost" )
      m_mode = DFormWidget::TopMost;
    else if ( str == "ToolBar" )
      m_mode = DFormWidget::ToolBarHelper;
    else
      ASSERT( 0 );
  }

  // m_drag = FALSE;
  m_editor = DFormEditor::loadingInstance();
  m_gridLayout = 0;
  m_layout = DFormWidget::NoLayout;

  setPalettePropagation( SamePalette );
  setColor();

  setAcceptDrops( TRUE );
}

DFormWidget::DFormWidget( DFormWidget::Mode _mode, DFormEditor* _editor, QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  // m_drag = FALSE;
  m_editor = _editor;
  m_mode = _mode;
  m_gridLayout = 0;
  m_layout = DFormWidget::NoLayout;

  setPalettePropagation( SamePalette );
  setColor();

  setAcceptDrops( TRUE );
}

DFormWidget::~DFormWidget()
{
}

void DFormWidget::dragEnterEvent( QDragEnterEvent *_ev )
{
  if ( !DWidgetDrag::decode( _ev, m_dragInfo ) )
    return;

  if ( m_layout != NoLayout )
  {
    int r, c;
    DGridLayout::Insert ins = m_gridLayout->insertTest( _ev->pos(), &r, &c );
    if ( ins == DGridLayout::InsertNone )
      _ev->ignore();
    else
    {
      setColor( TRUE );
      QRect rect = m_gridLayout->insertRect( ins, r, c );
      drawDragShadow( rect );
      _ev->accept();
    }
  }
  else
  {    
    setColor( TRUE );
    QPoint pos( _ev->pos().x() - m_dragInfo.sizeHint.width() / 2,
		_ev->pos().y() - m_dragInfo.sizeHint.height() / 2 );
    drawDragShadow( QRect( pos, m_dragInfo.sizeHint ) );
    _ev->accept();
  }
}

void DFormWidget::dragMoveEvent( QDragMoveEvent *_ev )
{
  clearDragShadow();

  if ( m_layout != NoLayout )
  {
    int r, c;
    DGridLayout::Insert ins = m_gridLayout->insertTest( _ev->pos(), &r, &c );
    if ( ins == DGridLayout::InsertNone )
    {
      setColor();
      _ev->ignore();
    }
    else
    {
      setColor( TRUE );
      QRect rect = m_gridLayout->insertRect( ins, r, c );
      drawDragShadow( rect );
      _ev->accept();
    }
  }
  else
  {    
    QPoint pos( _ev->pos().x() - m_dragInfo.sizeHint.width() / 2,
		_ev->pos().y() - m_dragInfo.sizeHint.height() / 2 );
    drawDragShadow( QRect( pos, m_dragInfo.sizeHint ) );
    _ev->accept();
  }
}

void DFormWidget::dragLeaveEvent( QDragLeaveEvent * )
{
  clearDragShadow();

  setColor();
}

void DFormWidget::dropEvent( QDropEvent *_ev ) 
{
  if ( m_layout == GridLayout )
  {
    int r, c;
    if ( m_gridLayout->insertTest( _ev->pos(), &r, &c ) == DGridLayout::InsertNone )
    {
      _ev->ignore();
      return;
    }
  }  

  bool do_replace = FALSE;
  QWidget* par = this;
  if ( m_mode == DFormWidget::GridHelper )  
  {
    do_replace = TRUE;
    par = parentWidget();
  }

  // Create the new widget
  QMetaObject* m = QMetaObjectInit::metaObject( m_dragInfo.className );
  ASSERT( m );
  QObjectFactory f = m->factory();
  ASSERT( f );
  QWidget* w = (QWidget*)(*f)( par, QResource() );
  ASSERT( w != 0 );
  
  m_editor->addWidget( w );

  if ( w->inherits( "QGroupBox" ) )
  {
    ((QGroupBox*)w)->setColumnLayout( 1, Qt::Horizontal );
    DFormWidget* f = new DFormWidget( DFormWidget::Container, m_editor, w );
    m_editor->addWidget( f );
  }
  else if ( w->inherits( "QTabWidget" ) )
  {
    DFormWidget* f = new DFormWidget( DFormWidget::Container, m_editor, w );
    m_editor->addWidget( f );
    ((QTabWidget*)w)->addTab( f, tr("Tab1") );
  }
  else if ( w->inherits( "QListView" ) )
  {
    ((QListView*)w)->addColumn( "Column1" );
    ((QListView*)w)->setRootIsDecorated( TRUE );
    ((QListView*)w)->setSorting( -1 );
    new QListViewItem( ((QListView*)w), "Item" );
  }

  // Make a good inital size
  if ( w->sizeHint().width() < 1 || w->sizeHint().height() < 1 )
    w->resize( 100, 80 );
  else
    w->resize( w->sizeHint() );
  w->show();

  _ev->accept();

  if ( do_replace )
  {
    // This will delete us!
    ((DFormWidget*)parentWidget())->replace( this, w );
    return;
  }

  if ( m_layout != NoLayout )
  {
    int r, c;
    DGridLayout::Insert ins = m_gridLayout->insertTest( _ev->pos(), &r, &c );
    DGridLayout::Matrix matrix( m_gridLayout->matrix() );
    DGridLayout::Mode m = m_gridLayout->mode();
    if ( ins == DGridLayout::InsertFirst )
    {
      matrix.insertColumn( c );
      matrix.insertRow( r );
    }
    else if ( ins == DGridLayout::InsertCol )
      matrix.insertColumn( c );
    else if ( ins == DGridLayout::InsertRow )
      matrix.insertRow( r );
    else
      ASSERT( 0 );
    
    printf("Inserting at %i %i\n",r,c);
    DGridLayout::Cell cell;
    cell.w = w;
    matrix.setCell( r, c, cell );
    
    delete m_gridLayout;
    m_gridLayout = new DGridLayout( this, m, matrix );
    m_gridLayout->fillWithForms( m_editor );
    m_gridLayout->activate();
    m_gridLayout->updateGeometry();
    update();
  }
  else
  {
    QPoint pos( _ev->pos().x() - m_dragInfo.sizeHint.width() / 2,
		_ev->pos().y() - m_dragInfo.sizeHint.height() / 2 );
    w->move( pos );
  }
}

void DFormWidget::drawDragShadow( const QRect& _rect )
{
  if ( m_mode == DFormWidget::GridHelper )
    return;

  m_dragShadow = _rect;

  QPainter painter;
  painter.begin( this );
  painter.setPen( Qt::black );
  
  painter.drawRect( _rect );

  painter.end();
}

void DFormWidget::clearDragShadow()
{
  if ( m_mode == DFormWidget::GridHelper )
    return;

  QPainter painter;
  painter.begin( this );
  painter.setPen( backgroundColor() );
  
  painter.drawRect( m_dragShadow );

  painter.end();

  m_dragShadow = QRect();
}

void DFormWidget::popupMenu( const QPoint& _pos )
{
  if ( m_mode == DFormWidget::GridHelper )
  {
    QPopupMenu* popup = new QPopupMenu( topLevelWidget() );
    popup->insertItem( tr("Create container"), this, SLOT( slotCreateContainer() ) );

    popup->popup( _pos );
    return;
  }
  
  QPopupMenu* popup = new QPopupMenu( topLevelWidget() );
  int id = popup->insertItem( tr("Align Vertical"), this, SLOT( slotAlignVertical() ) );
  if ( m_layout == DFormWidget::VBoxLayout )
    popup->setItemEnabled( id, FALSE );
  id = popup->insertItem( tr("Align Horizontal"), this, SLOT( slotAlignHorizontal() ) );
  if ( m_layout == DFormWidget::HBoxLayout )
    popup->setItemEnabled( id, FALSE );
  id = popup->insertItem( tr("Align in Grid"), this, SLOT( slotAlignGrid() ) );
  if ( m_layout == DFormWidget::GridLayout )
    popup->setItemEnabled( id, FALSE );
  id = popup->insertItem( tr("No Alignment"), this, SLOT( slotNoAlign() ) );
  if ( m_layout == DFormWidget::NoLayout )
    popup->setItemEnabled( id, FALSE );

  if ( parent()->inherits("DFormWidget") && ((DFormWidget*)parentWidget())->layoutType() == GridLayout )
  {
    popup->insertItem( tr("Delete Container"), this, SLOT( slotDeleteContainer() ) );
  }
  
  popup->popup( _pos );
}

void DFormWidget::mousePressEvent( QMouseEvent* _ev )
{
  if ( _ev->button() == RightButton )
  {
    popupMenu( mapToGlobal( _ev->pos() ) );
    return;
  }

  if ( !( _ev->state() & ( Qt::ControlButton | Qt::ShiftButton ) ) )
  {
    m_editor->unselectAll();
    m_selectedChildren.clear();
  }
  else
  {
    const QObjectList *list = children();
    if ( list )
    {
      QObjectListIt it( *list );
      for( ; it.current(); ++it )
      {
	DObjectInfo* info = m_editor->findInfo( it.current() );
	if ( info && info->isSelected() )
	  m_selectedChildren.append( it.current() );
      }
    }
  }

  m_mousePress = _ev->pos();
  drawDragShadow( QRect( m_mousePress, QSize( 1, 1 ) ) );
}

void DFormWidget::mouseMoveEvent( QMouseEvent* _ev )
{
  QRect area( m_mousePress, QSize( _ev->pos().x() - m_mousePress.x(), _ev->pos().y() - m_mousePress.y() ) );
  clearDragShadow();
  drawDragShadow( area );

  // Select all children in the rect ...
  const QObjectList *list = children();
  if ( list )
  {
    // Iterate over all children
    QObjectListIt it( *list );
    for( ; it.current(); ++it )
    {
      // Do we know about this child ?
      DObjectInfo* info = m_editor->findInfo( it.current() );
      if ( info )
      {
	// Was it selected before ? Assume no.
	bool selected = FALSE;
	QListIterator<QObject> it2( m_selectedChildren );
	for( ; it2.current() && !selected; ++it2 )
	  if ( it2.current() == it.current() )
	    selected = TRUE;
	if ( !selected )
	  info->setSelected( area.contains( ((QWidget*)it.current() )->geometry() ) );
      }
    }
  }    
}

void DFormWidget::mouseReleaseEvent( QMouseEvent* )
{
  clearDragShadow();
}

struct DWidgetSort
{
  DWidgetSort() { }
  DWidgetSort( const DWidgetSort& _w ) { w = _w.w; vert = _w.vert; }
  DWidgetSort( bool _vert, QWidget* _w ) { w = _w; vert = _vert; }
  QWidget* w;
  bool vert;

  bool operator<( const DWidgetSort& _s ) { return ( vert ? ( w->y() < _s.w->y() ) : ( w->x() < _s.w->x() ) ); }
};

void DFormWidget::setLayout( DFormWidget::Layout _l, bool _force )
{
  if ( _l == m_layout && !_force )
    return;

  DObjectInfo* info = m_editor->findInfo( this );
  if ( info )
    info->setSelected( FALSE );

  // Delete old layout
  if ( m_layout != DFormWidget::NoLayout && layout() )
    delete layout();

  m_layout = _l;

  setColor();
  
  // Now order the widgets
  switch( _l )
  {
  case DFormWidget::GridLayout:
    {
      QList<QWidget> lst;
      const QObjectList *list = children();
      if ( list )
      {
	QObjectListIt it( *list );
	for( ; it.current(); ++it )
	{
	  if ( it.current()->inherits( "QWidget" ) && !it.current()->inherits( "DSizeHandle" ) &&
	       !it.current()->inherits( "QPopupMenu" ) )
	    lst.append( (QWidget*)it.current() );
	}
      }
      m_gridLayout = dGuessGrid( this, lst );
      m_gridLayout->fillWithForms( m_editor );
#ifdef GRID_TEST
      setMinimumSize( QSize( 400, 400 ) );
      return;
#endif
      m_gridLayout->activate();

      resize( sizeHint() );

      // TODO: That is a QLayout HACK. Still needed ?
      QWidget* w = parentWidget();
      while( w && ( !w->inherits( "DFormWidget" ) || !((DFormWidget*)w)->isTopMostForm() ) )
      {
	if ( w->layout() )
	{
	  w->layout()->invalidate();
	  w->layout()->activate();
	  w->resize( w->sizeHint() );
	}
	w = w->parentWidget();
      }
    }
    break;
  case DFormWidget::VBoxLayout:
  case DFormWidget::HBoxLayout:
    {
      if ( _l == DFormWidget::VBoxLayout )
	m_gridLayout = new DGridLayout( this, DGridLayout::Vertical );
      else
	m_gridLayout = new DGridLayout( this, DGridLayout::Horizontal );

      // Sort the widgets by their upper left corner
      QValueList<DWidgetSort> lst;
      const QObjectList *list = children();
      if ( list )
      {
	QObjectListIt it( *list );
	for( ; it.current(); ++it )
	{
	  if ( it.current()->inherits( "QWidget" ) && !it.current()->inherits( "DSizeHandle" ) &&
	       !it.current()->inherits( "QPopupMenu" ) )
	    lst.append( DWidgetSort( ( _l == DFormWidget::VBoxLayout ), (QWidget*)it.current() ) );
	}
      }
      qBubbleSort( lst );
      
      // Add all widgets to the layout
      QValueList<DWidgetSort>::Iterator it = lst.begin();
      int pos = 0;
      for( ; it != lst.end(); ++it, ++pos )
      {
	if ( it->w->inherits("DStretch") )
	  ((DStretch*)it->w)->setOrientation( _l == DFormWidget::VBoxLayout ?
					      Qt::Vertical : Qt::Horizontal );
	else if ( it->w->inherits("DSpacing") )
	  ((DSpacing*)it->w)->setOrientation( _l == DFormWidget::VBoxLayout ?
					      Qt::Vertical : Qt::Horizontal );
	
	qDebug("Adding widget class %s", it->w->className() );
	if ( _l == DFormWidget::VBoxLayout )
	  m_gridLayout->addWidget2( it->w, pos, 0 );
	else
	  m_gridLayout->addWidget2( it->w, 0, pos );
      }

      m_gridLayout->activate();

      qDebug("%s has size hint of %i %i", className(), sizeHint().width(), sizeHint().height() );
      resize( sizeHint() );

      QWidget* w = parentWidget();
      while( w && ( !w->inherits( "DFormWidget" ) || !((DFormWidget*)w)->isTopMostForm() ) )
      {
	if ( w->layout() )
	{
	  qDebug("%s has size hint of %i %i", w->className(), w->sizeHint().width(), w->sizeHint().height() );
	  w->layout()->invalidate();
	  w->layout()->activate();
	  w->resize( w->sizeHint() );
	  qDebug("2. %s has size hint of %i %i", w->className(), w->sizeHint().width(), w->sizeHint().height() );
	}
	w = w->parentWidget();
      }
    }
    break;
  case DFormWidget::NoLayout:
    break;
  default:
    ASSERT( 0 );
  }

  m_editor->updateSizeHandles();
}

void DFormWidget::slotAlignVertical()
{
  setLayout( DFormWidget::VBoxLayout );
}

void DFormWidget::slotAlignHorizontal()
{
  setLayout( DFormWidget::HBoxLayout );
}

void DFormWidget::slotAlignGrid()
{
  setLayout( DFormWidget::GridLayout );
}

void DFormWidget::slotNoAlign()
{
  setLayout( DFormWidget::NoLayout );
}

void DFormWidget::slotCreateContainer()
{
  m_mode = LayoutHelper;
  setColor();
}

void DFormWidget::slotDeleteContainer()
{
  setLayout( DFormWidget::NoLayout );
  // TODO: May have children
  m_mode = GridHelper;
  setColor();
}

QSizePolicy DFormWidget::sizePolicy() const
{
  return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

QColor DFormWidget::bgcolor() const
{
  switch( m_layout )
    {
    case DFormWidget::VBoxLayout:
      return QColor( 130,130,200 );
    case DFormWidget::HBoxLayout:
      return QColor( 255,255,180 );
    case DFormWidget::GridLayout:
      return QColor( 130,255,255 );
    case DFormWidget::NoLayout:
      if ( m_mode == DFormWidget::GridHelper )
	return QColor( 255,170,170 );
      return QColor( 130,200,130 );
    default:
      ASSERT( 0 );
    }

  return Qt::black;
}

void DFormWidget::setColor( bool highlight )
{
  if ( m_mode == ToolBarHelper )
    setBackgroundMode( PaletteBackground );
  else if ( highlight )
    setPalette( bgcolor().light( 120 ) );
  else
    setPalette( bgcolor() );
}

void DFormWidget::paintEvent( QPaintEvent* _ev )
{
  if ( m_layout != GridLayout )
  {
    QWidget::paintEvent( _ev );
    return;
  }
  
  QPainter painter;
  painter.begin( this );
  
  QColor color = bgcolor().dark( 120 );
  painter.setPen( color );
  
  for( uint r = 0; r < m_gridLayout->rows(); ++r )
    for( uint c = 0; c < m_gridLayout->cols(); ++c )
    {
      painter.fillRect( m_gridLayout->col( c ).left, m_gridLayout->row( r ).top,
			m_gridLayout->col( c ).right - m_gridLayout->col( c ).left,
			m_gridLayout->row( r ).bottom - m_gridLayout->row( r ).top, color );
    }
  
  painter.end();
}

void DFormWidget::resizeEvent( QResizeEvent* _ev )
{
  if ( m_layout != NoLayout )
    m_gridLayout->updateGeometry();

  QWidget::resizeEvent( _ev );
}

void DFormWidget::replace( int _row, int _col, QWidget* _w )
{
  ASSERT( m_layout == GridLayout );
  
  DGridLayout::Matrix matrix( m_gridLayout->matrix() );  
  DGridLayout::Mode m = m_gridLayout->mode();
  DGridLayout::Cell cell( matrix.cell( _row, _col ) );
  if ( cell.w )
  {    
    cell.w->hide();
    cell.w->reparent( 0, 0, QPoint( 3000, 3000 ) );
    // delete cell.w;
  }
  
  cell.w = _w;
  matrix.setCell( _row, _col, cell );

  delete m_gridLayout;
  m_gridLayout = new DGridLayout( this, m, matrix );
  m_gridLayout->fillWithForms( m_editor );
  m_gridLayout->activate();
  m_gridLayout->updateGeometry();
  update();
}

void DFormWidget::replace( QWidget* _old, QWidget *_new )
{
  ASSERT( m_layout == GridLayout );

  for( uint r = 0; r < m_gridLayout->rows(); ++r )
    for( uint c = 0; c < m_gridLayout->cols(); ++c )
      if ( m_gridLayout->cell( r, c ).w == _old )
      {
	replace( r, c, _new );
	return;
      }

  qDebug("Could not find widget to replace");
  ASSERT( 0 );
}

QResourceItem* DFormWidget::save()
{
  qDebug("Saving Form");
  switch( m_layout )
  {
  // Save the form if it does not have a layout yet.
  case NoLayout:
    {
      QResourceItem *res = 0;
      QResourceItem *w = 0;
      if ( !isTopMostForm() )
      {
	res = new QResourceItem( "Widget" );
	w = new QResourceItem( "QWidget" );
	res->append( w );
      }
      else
	res = w = new QResourceItem( "QWidget" );

      // Iterate over all children
      const QObjectList *list = children();
      if ( list )
      {
	QObjectListIt it( *list );
	for( ; it.current(); ++it )
        {
	  // An embedded DFormWidget ?
	  if ( it.current()->inherits( "DFormWidget" ) )
	  {
	    QProperty prop( ((DFormWidget*)it.current())->geometry() );
	    QResourceItem* f = ((DFormWidget*)it.current())->save();
	    // Its it layouted ?
	    // -> Insert intermediate widget with the correct geometry
	    if ( f->type() == "Layout" )
	    {
	      QResourceItem *t1 = new QResourceItem( "Widget" );
	      QResourceItem *t2 = new QResourceItem( "QWidget" );
	      t2->setProperty( "geometry", prop );
	      w->append( t1 );
	      t1->append( t2 );
	      t2->append( f );
	    }
	    // Is it a widget ?
	    // -> Write the geometry in the resource
	    else if ( f->type() == "Widget" )
	      f->firstChild()->setProperty( "geometry", prop );
	    else
	      ASSERT(0);
	    w->append( f );
	  }
	  // Dont write this ones!
	  else if ( it.current()->inherits( "DSizeHandle" ) )
	  {
	  }
	  // An embedded widget ?
	  else if ( it.current()->inherits( "QWidget" ) )
	  {
	    QResourceItem* t = new QResourceItem( "Widget" );
	    w->append( t );
	    DObjectInfo* info = m_editor->findInfo( it.current() );
	    // Is it a widget we know ?
	    // Dont save child that we did not construct in the builder
	    if ( info )
	    {
	      // Save the widget
	      QResourceItem* w = qObjectToXML( info );
	      // Save geometry
	      QProperty prop( ((QWidget*)it.current())->geometry() );
	      w->setProperty( "geometry", prop );
	      // Append to the resource tree
	      t->append( w );
	    }
	  }
	}
      }
      return res;
    }
    break;
  /**
   * Since all layouts are realized with DGridLayout, we need only
   * one saving code here for all layouts.
   */
  case HBoxLayout:
  case VBoxLayout:
  case GridLayout:
    {
      QResourceItem* res = 0;

      // Save as toolbar
      if ( m_mode == ToolBarHelper )
      {
	ASSERT( m_layout == HBoxLayout );
	res = new QResourceItem( "QToolBar" );

	// Iterate over all columns
	for( uint x = 0; x < m_gridLayout->matrix().cols(); ++x )
	{
	  DGridLayout::Cell cell = m_gridLayout->matrix().cell( 0, x );
	  if ( cell.w && cell.w->inherits( "DFormWidget" ) )
	    res->append( ((DFormWidget*)cell.w)->save() );
	  else if ( cell.w && cell.w->inherits( "DSeparator" ) )
	    res->append( new QResourceItem( "Separator" ) );
	  else if ( cell.w && strcmp( cell.w->name(), "whatsthis button" ) == 0 )
	    res->append( new QResourceItem( "WhatsThis" ) );
	  else if ( cell.w )
          {
	    QResourceItem* w = new QResourceItem( "Widget" );
	    res->append( w );
	    DObjectInfo* info = m_editor->findInfo( cell.w );
	    if ( info )
	      w->append( qObjectToXML( info ) );
	  }
	}
      }
      // Save usually
      else
      {
	QResourceItem* l = 0;
	if ( isTopMostForm() )
        {
	  res = new QResourceItem( "QWidget" );
	  l = new QResourceItem( "Layout" );
	  res->append( l );
        }
	else
	  res = l = new QResourceItem( "Layout" );
	l->append( m_gridLayout->save( m_editor ) );
      }

      return res;
    }
    break;
  default:
    ASSERT(0);
  }

  // Never reached
  return 0;
}

bool DFormWidget::configure( const QResource& _resource )
{  
  const QResourceItem* r = _resource.tree()->firstChild();
  for( ; r; r = r->nextSibling() )
    if ( r->type() == "Widget" )
    {
      QWidget* w = QResource::createWidget( this, r->firstChild() );
      if ( w == 0 )
	return FALSE;
      m_editor->addWidget( w, r->firstChild() );
    }
  /* else if ( r->type() == "WhatsThis" )
    {
      QWidget* w = QWhatsThis::whatsThisButton( this );
      w->setName( "whatsthis button" );
      m_editor->addWidget( w );
      } */
    else if ( r->type() == "Layout" )
    {
      // try to create the layout with all child widgets
      const QResourceItem* c = r->firstChild();
      QLayout* l = QResource::createLayout( this, c );
      if ( l == 0 )
	return FALSE;

      m_gridLayout = (DGridLayout*)l;
      // What kind of layout did we create ?
      QString m = c->attrib( "__mode" );
      if ( m == "grid" )
	m_layout = GridLayout;
      else if ( m == "vertical" )
	m_layout = VBoxLayout;
      else if ( m == "horizontal" )
	m_layout = HBoxLayout;
      else
	ASSERT( 0 );

      // Tell the editor about all of our child widgets
      const QObjectList *list = children();
      if ( list )
      {
	QObjectListIt it( *list );
	for( ; it.current(); ++it )
	  if ( it.current()->inherits( "QWidget" ) && m_editor->findInfo( it.current() ) == 0 )
	    m_editor->addWidget( (QWidget*)it.current() );
      }
    }

  // Skip QWidget
  if ( !QObject::configure( _resource ) )
    return FALSE;

  // Do that after the QWidget::configure to enshure that we
  // overwrite properties in ever case.
  setColor();
  
  return TRUE;
}
