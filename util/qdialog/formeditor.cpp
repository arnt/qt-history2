#include "formeditor.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qresource.h>
#include <qapplication.h>
#include <qobjectlist.h>
#include <qvaluelist.h>
#include <qtl.h>
#include <qpopupmenu.h>

// Needed since they require special treatment from the builder
#include <qgroupbox.h>
#include <qtabwidget.h>

/**********************************************
 *
 * DFormEditor
 *
 **********************************************/

DFormEditor::DFormEditor( QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  QGridLayout* grid = new QGridLayout( this, 1, 1, 12, 0 );
  
  m_form = new DFormWidget( DFormWidget::TopMost, this, this );
  grid->addWidget( m_form, 0, 0 );

  qApp->installEventFilter( this );
}

DFormEditor::~DFormEditor()
{
}

void DFormEditor::addWidget( QWidget* _w )
{
  m_widgets.insert( _w, new DObjectInfo( _w ) );
}

bool DFormEditor::eventFilter( QObject* _obj, QEvent* _ev )
{
  if ( _ev->type() == QEvent::MouseButtonPress ||
       _ev->type() == QEvent::MouseButtonRelease ||
       _ev->type() == QEvent::MouseMove )
  {
    if ( _obj->inherits( "DSizeHandle" ) || _obj->inherits( "QPopupMenu" ) )
      return FALSE;

    QMouseEvent* me = (QMouseEvent*)_ev;

    DObjectInfo* info = m_widgets.find( _obj );
    if ( !info )
    { 
      // Perhaps it is a form widget
      if ( _obj->inherits( "DFormWidget" ) )
      {
	DFormWidget* f = (DFormWidget*)_obj;
	if ( !f->isTopMostForm() )
	{
	  QObject* o = f->parent();
	  info = m_widgets.find( o );
	}
	// It is the top most form.
	else if ( !( me->state() & ( Qt::ControlButton | Qt::ShiftButton ) ) )
	{
	  // Unselect all on a normal click
	  if ( _ev->type() == QEvent::MouseButtonPress )
	    unselectAll();
	  return FALSE;
	}
      }

      if ( !info )
      {
	// Try to find out wether the parent is know to us
	QObject* o = _obj;
	while( !info && o->parent() )
        {
	  o = o->parent();
	  info = m_widgets.find( o );
	}
	if ( !info )
	  return FALSE;
      }
    }

    if ( _ev->type() == QEvent::MouseButtonPress )
    {
      if ( me->state() & ( Qt::ControlButton | Qt::ShiftButton ) )
	info->setSelected( !info->isSelected() );
      else
	selectObjectExclusive( info );
    }

    // Never (almost) eat event from the form widget.
    if ( _obj->inherits( "DFormWidget" ) && ( me->button() != Qt::LeftButton ||
	 ( !((DFormWidget*)_obj)->isLayouted() && ((DFormWidget*)_obj)->mode() != DFormWidget::LayoutHelper ) ) )
      return FALSE;
    else
      return TRUE;
  }

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
    _info->setSelected( TRUE );
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
	debug("Error: NOT ALL IN SAME PARENT");
	// TODO: Error message
	return;
      }
      selected.append( (QWidget*)it.currentKey() );
    }
  }

  // Is anything selected at all ?
  if ( selected.isEmpty() )
  {
    debug("Error: NOTHING SELECTED");
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
	debug("Error: NOT ALL IN SAME PARENT");
	// TODO: Error message
	return;
      }
      selected.append( (QWidget*)it.currentKey() );
    }
  }

  // Is anything selected at all ?
  if ( selected.isEmpty() )
  {
    debug("Error: NOTHING SELECTED");
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
  QObject* p = 0;
  QPtrDictIterator<DObjectInfo> it( m_widgets );
  for( ; it.current(); ++it )
    if ( it.current()->isSelected() )
      it.current()->update();
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
  for( int i = 0; i < 8; ++i )
    m_sizeHandles[i] = 0;
}

DObjectInfo::~DObjectInfo()
{
  for( int i = 0; i < 8; ++i )
    if ( m_sizeHandles[i] != 0 )
      delete m_sizeHandles[i];
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

void DSizeHandle::mouseReleaseEvent( QMouseEvent* _ev )
{
  m_info->endMove();
}

/**********************************************
 *
 * DFormWidget
 *
 **********************************************/

DFormWidget::DFormWidget( DFormWidget::Mode _mode, DFormEditor* _editor, QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  m_drag = FALSE;
  m_editor = _editor;
  m_mode = _mode;
  m_vBoxLayout = 0;
  m_layout = DFormWidget::NoLayout;

  if ( m_mode == DFormWidget::LayoutHelper )
    setBackgroundColor( QColor( 255,170,170 ) );
  else
    setBackgroundColor( QColor( 130,200,130 ) );

  setAcceptDrops( TRUE );
}

DFormWidget::~DFormWidget()
{
}

void DFormWidget::dragEnterEvent( QDragEnterEvent *_ev )
{
  if ( !DWidgetDrag::decode( _ev, m_dragInfo ) )
  {
    return;
  }

  m_drag = TRUE;
  QPoint pos( _ev->pos().x() - m_dragInfo.sizeHint.width() / 2,
	      _ev->pos().y() - m_dragInfo.sizeHint.height() / 2 );
  drawDragShadow( QRect( pos, m_dragInfo.sizeHint ) );
  _ev->accept();
}

void DFormWidget::dragMoveEvent( QDragMoveEvent *_ev )
{
  if ( !m_drag )
    return;

  clearDragShadow();

  QPoint pos( _ev->pos().x() - m_dragInfo.sizeHint.width() / 2,
	      _ev->pos().y() - m_dragInfo.sizeHint.height() / 2 );
  drawDragShadow( QRect( pos, m_dragInfo.sizeHint ) );

  _ev->accept();
}

void DFormWidget::dragLeaveEvent( QDragLeaveEvent *_ev )
{
  clearDragShadow();
  m_drag = FALSE;
}

void DFormWidget::dropEvent( QDropEvent *_ev ) 
{
  m_drag = FALSE;

  QWidget* w = QResourceFactory::factory()->createWidget( m_dragInfo.className, this );
  ASSERT( w != 0 );
  
  m_editor->addWidget( w );

  if ( w->inherits( "QGroupBox" ) )
  {
    (void)new DFormWidget( DFormWidget::Container, m_editor, w );
  }
  else if ( w->inherits( "QTabWidget" ) )
  {
    DFormWidget* f = new DFormWidget( DFormWidget::Container, m_editor, w );
    ((QTabWidget*)w)->addTab( f, tr("Tab1") );
  }

  QPoint pos( _ev->pos().x() - m_dragInfo.sizeHint.width() / 2,
	      _ev->pos().y() - m_dragInfo.sizeHint.height() / 2 );
  w->move( pos );
  w->resize( w->sizeHint() );
  w->show();

  switch( m_layout )
  {
  case DFormWidget::VBoxLayout:
  case DFormWidget::HBoxLayout:
    setLayout( m_layout, TRUE );
    break;
  case DFormWidget::NoLayout:
    break;
  default:
    ASSERT( 0 );
  }
}

void DFormWidget::drawDragShadow( const QRect& _rect )
{
  m_dragShadow = _rect;

  QPainter painter;
  painter.begin( this );
  painter.setRasterOp( Qt::XorROP );

  painter.drawRect( _rect );

  painter.end();
}

void DFormWidget::clearDragShadow()
{
  drawDragShadow( m_dragShadow );
}

void DFormWidget::popupMenu( const QPoint& _pos )
{
  QPopupMenu* popup = new QPopupMenu( this );
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

void DFormWidget::mouseReleaseEvent( QMouseEvent* _ev )
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

  switch( _l )
  {
  case DFormWidget::VBoxLayout:
    setBackgroundColor( QColor( 130,130,200 ) );
    m_vBoxLayout = new QVBoxLayout( this, 6, 6 );
    break;
  case DFormWidget::HBoxLayout:
    setBackgroundColor( QColor( 255,255,180 ) );
    m_hBoxLayout = new QHBoxLayout( this, 6, 6 );
    break;
  case DFormWidget::GridLayout:
    setBackgroundColor( QColor( 130,255,255 ) );
    break;
  case DFormWidget::NoLayout:
    if ( m_mode == DFormWidget::LayoutHelper )
      setBackgroundColor( QColor( 255,170,170 ) );
    else
      setBackgroundColor( QColor( 130,200,130 ) );
    break;
  default:
    ASSERT( 0 );
  }

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
      m_gridLayout->fillWithForms( m_editor, this );
#ifdef GRID_TEST
      setMinimumSize( QSize( 400, 400 ) );
      return;
#endif
      m_gridLayout->activate();

      resize( sizeHint() );

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
      
      QValueList<DWidgetSort>::Iterator it = lst.begin();
      for( ; it != lst.end(); ++it )
      {
	debug("Adding widget class %s", it->w->className() );
	if ( _l == DFormWidget::VBoxLayout )
	  m_vBoxLayout->addWidget( it->w );
	else
	  m_hBoxLayout->addWidget( it->w );
      }

      if ( _l == DFormWidget::VBoxLayout )
	m_vBoxLayout->activate();
      else
	m_hBoxLayout->activate();

      debug("%s has size hint of %i %i", className(), sizeHint().width(), sizeHint().height() );
      resize( sizeHint() );

      QWidget* w = parentWidget();
      while( w && ( !w->inherits( "DFormWidget" ) || !((DFormWidget*)w)->isTopMostForm() ) )
      {
	if ( w->layout() )
	{
	  debug("%s has size hint of %i %i", w->className(), w->sizeHint().width(), w->sizeHint().height() );
	  w->layout()->invalidate();
	  w->layout()->activate();
	  w->resize( w->sizeHint() );
	  debug("2. %s has size hint of %i %i", w->className(), w->sizeHint().width(), w->sizeHint().height() );
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

  m_layout = _l;

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

QSizePolicy DFormWidget::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}
