#include "inspector.h"
#include "formeditor.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qscrollbar.h>
#include <qmetaobject.h>
#include <qpushbutton.h>

/**************************************************
 *
 * DInspector
 *
 **************************************************/

DInspector::DInspector( QWidget* _parent, const char* _name )
  : QTabWidget( _parent, _name )
{
  props = new DPropertyInspector( this );
  addTab( props, tr("Properties") );

  QWidget* w = new QWidget( this );
  QHBoxLayout* hbox = new QHBoxLayout( w, 6, 6 );
  connects = new DConnectionInspector( w );
  hbox->addWidget( connects );
  addTab( w, tr("Connections") );
}

DInspector::~DInspector()
{
}

void DInspector::inspect( DFormEditor* _editor, DObjectInfo* _info )
{
  props->inspect( _editor, _info );
  connects->inspect( _editor, _info );
}

/**************************************************
 *
 * DPropertyInspector
 *
 **************************************************/

DPropertyInspector::DPropertyInspector( QWidget* _parent = 0, const char* _name = 0 )
  : QForm( _parent, _name )
{
  m_info = 0;
}

DPropertyInspector::~DPropertyInspector()
{
}
  
void DPropertyInspector::inspect( DFormEditor*, DObjectInfo* _info )
{
  if ( m_info == _info )
    return;
  
  m_widgets.setAutoDelete( TRUE );
  m_widgets.clear();
  m_widgets.setAutoDelete( FALSE );

  m_info = _info;
  QWidget* object = m_info->widget();

  if ( viewport()->layout() )
    delete viewport()->layout();
  
  QStringList props = object->metaObject()->propertyNames();
  
  QGridLayout* g = new QGridLayout( viewport(), props.count() * 2, 2, 6, 6 );
  g->setRowStretch( props.count() * 2, 1 );
  
  QStringList::Iterator it = props.begin();
  for( uint i = 0; it != props.end(); ++i, ++it )
  {
    QPushButton* l = new QPushButton( *it, viewport() );
    l->setToggleButton( TRUE );
    if ( _info->containsProperty( *it ) )
      l->setOn( TRUE );
    else
      l->setOn( FALSE );
    m_widgets.append( l );
    l->show();
    g->addWidget( l, 2 * i, 0 );

    DPropertyEditor* ed = DPropEditorFactory::create( m_info, *it, l, viewport() );
    if ( ed )
    {
      connect( l, SIGNAL( toggled( bool ) ), ed, SLOT( slotToggled( bool ) ) );

      ed->show();
      g->addWidget( ed, 2 * i, 1 );
      m_widgets.append( ed );
    }
    
    if ( i + 1 < props.count() )
    {
      QFrame *f = new QFrame( viewport() );
      m_widgets.append( f );
      f->setFrameStyle( QFrame::Sunken | QFrame::HLine );
      g->addMultiCellWidget( f, 2 * i + 1, 2 * i + 1, 0, 1 );
      f->show();
    } 
  }
  g->setColStretch( 1, 1 );
  g->activate();

  qDebug("====== Inspector done =========");
  
  // ###########
  resizeEvent( 0 );

  verticalScrollBar()->setSteps( viewport()->height() / props.count(), height() / 2 );
}

/**************************************************
 *
 * DPropertyEditor
 *
 **************************************************/

DPropertyEditor::DPropertyEditor( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
				  QWidget* _parent, const char* _name )
  : QFrame( _parent, _name )
{
  toggle = _toggle;
  prop_name = _prop_name;
  obj = _inspect;
}

DPropertyEditor::~DPropertyEditor()
{
}

void DPropertyEditor::slotUpdate()
{
  obj->widget()->property( prop_name, &prop );
}

void DPropertyEditor::slotToggled( bool toggled )
{
  if ( toggled )
  {
    // Now there is a property that has to be saved
    obj->setProperty( prop_name, prop );
  }
  else
  {
    // Dont save this property since we are told to
    // use always the default
    obj->removeProperty( prop_name );
    // Update the property editor to reflect the default value
    slotUpdate();
  }
}

void DPropertyEditor::apply()
{
  if ( isValid() )
  {
    obj->setProperty( prop_name, prop );
    toggle->setOn( TRUE );
  }
}

/**************************************************
 *
 * DStringPropEditor
 *
 **************************************************/

DStringPropEditor::DStringPropEditor( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
				      QWidget* _parent, const char* _name )
  : DPropertyEditor( _inspect, _prop_name, _toggle, _parent, _name )
{
  lineedit = new QLineEdit( this );
  
  QHBoxLayout *h = new QHBoxLayout( this );
  h->addWidget( lineedit );

  connect( lineedit, SIGNAL( returnPressed() ),
	   this, SLOT( returnPressed() ) );

  slotUpdate();
}

DStringPropEditor::~DStringPropEditor()
{
}

void DStringPropEditor::slotUpdate()
{
  DPropertyEditor::slotUpdate();
  if ( isValid() )
    lineedit->setText( property().stringValue() );
}

void DStringPropEditor::returnPressed()
{
  property().setValue( lineedit->text() );
  apply();
}

/**************************************************
 *
 * DPixmapPropEditor
 *
 **************************************************/

DPixmapPropEditor::DPixmapPropEditor( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
				      QWidget* _parent, const char* _name )
  : DPropertyEditor( _inspect, _prop_name, _toggle, _parent, _name )
{
  label = new QLabel( this );
  
  QHBoxLayout *h = new QHBoxLayout( this );
  h->addWidget( label );
  
  label->installEventFilter( this );

  slotUpdate();
}

DPixmapPropEditor::~DPixmapPropEditor()
{
}

void DPixmapPropEditor::slotUpdate()
{
  DPropertyEditor::slotUpdate();

  if ( isValid() )
  {
    QPixmap pix = property().pixmapValue();
    if ( !pix.isNull() )
      label->setPixmap( pix );
    else
      label->setText( "None" );
  }
}

bool DPixmapPropEditor::eventFilter( QObject* o, QEvent* e )
{
  if ( o == label && e->type() == QEvent::MouseButtonPress )
  {
    openFile();
    return TRUE;
  }
  
  return FALSE;
}

void DPixmapPropEditor::mousePressEvent( QMouseEvent* )
{
  openFile();
}

void DPixmapPropEditor::openFile()
{
  QString fn = QFileDialog::getOpenFileName();
  if ( fn.isEmpty() )
    return;
  
  QPixmap pix( fn );
  if ( pix.isNull() )
  {
    QMessageBox::critical( this, tr("Loading Error"), tr("Could not load pixmap") );
    return;
  }
  
  property().setValue( pix );
  apply();

  label->setPixmap( pix ); 
}

/**************************************************
 *
 * DPropEditorFactory
 *
 **************************************************/

DPropEditorFactory::DPropEditorFactory()
{
}

DPropEditorFactory::~DPropEditorFactory()
{
}

DPropertyEditor* DPropEditorFactory::create( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
					     QWidget* _parent, const char* _name )
{
  QMetaProperty* prop = _inspect->widget()->metaObject()->property( _prop_name, TRUE );
  
  if ( !prop )
    return 0;
  
  if ( strcmp( prop->type, "QString" ) == 0 )
    return new DStringPropEditor( _inspect, _prop_name, _toggle, _parent, _name );
  if ( strcmp( prop->type, "QPixmap" ) == 0 )
    return new DPixmapPropEditor( _inspect, _prop_name, _toggle, _parent, _name );

  return 0;
}

/**************************************************
 *
 * DPropEditorFactory
 *
 **************************************************/

DConnectionInspector::DConnectionInspector( QWidget* _parent, const char* _name ) : QListView( _parent, _name )
{
  m_info = 0;

  setRootIsDecorated( TRUE );

  addColumn( tr("Connection") );
  addColumn( tr("Name") );
  addColumn( tr("Class") );
}

DConnectionInspector::~DConnectionInspector()
{
}

void DConnectionInspector::inspect( DFormEditor* _editor, DObjectInfo* _info )
{
  clear();
  m_info = _info;

  QMetaObject *meta = m_info->widget()->metaObject();
  if ( !meta )
    return;

  // Find all connections which connect to our slots.
  // In addition we will get all connections which connect to
  // one of our signals in a slot-semantic ( you can use
  // a signal as a slot ).
  QValueList<DObjectInfo::Connection> cons;
  QString name = _info->widget()->name();
  if ( !name.isEmpty() && name != "unnamed" )
    cons = _editor->connectionsToReceiver( name );


  QListViewItem* signal = new QListViewItem( this, tr("Signals"), "", "" );
  QListViewItem* slot = new QListViewItem( this, tr("Slots"), "", "" );
  
  int n = meta->nSignals( TRUE );
  int i;
  for( i = 0; i < n; ++i )
  {
    QMetaData* d = meta->signal( i, TRUE );
    QListViewItem* item = new QListViewItem( signal, (const char*)d->name, "", "" );

    // Find connections where we act as sender
    QValueList<DObjectInfo::Connection>::Iterator it = _info->connectionsBegin();
    for( ; it != _info->connectionsEnd(); ++it )
    {
      if ( it->signal == d->name )
      {
	QString class_name = "(no instance)";
	DObjectInfo* info = _editor->info( it->receiverName );
	if ( info )
	  class_name = info->widget()->className();
        (void)new QListViewItem( item, it->slot, it->receiverName, class_name );
      }
    }

    // Find connections where we act as receiver
    // And yes: It is possible to use a signal in a slot-semantic!
    it = cons.begin();
    for( ; it != cons.end(); ++it )
    {
      if ( it->slot == d->name )
      {
	QString class_name = "(no instance)";
	DObjectInfo* info = _editor->info( it->senderName );
	if ( info )
	  class_name = info->widget()->className();
        (void)new QListViewItem( item, it->signal, it->senderName, class_name );
      }
    }
  }

  n = meta->nSlots( TRUE );
  for( i = 0; i < n; ++i )
  {
    QMetaData* d = meta->slot( i, TRUE );
    QListViewItem* item = new QListViewItem( slot, (const char*)d->name, "", "" );

    // Find connections where we act as a receiver
    QValueList<DObjectInfo::Connection>::Iterator it = cons.begin();
    for( ; it != cons.end(); ++it )
    {
      if ( it->slot == d->name )
      {
	QString class_name = "(no instance)";
	DObjectInfo* info = _editor->info( it->senderName );
	if ( info )
	  class_name = info->widget()->className();
        (void)new QListViewItem( item, it->signal, it->senderName, class_name );
      }
    }
  }
}
