#include "mainwindow.h"
#include "widgetsbar.h"
#include "formeditor.h"
#include "inspector.h"
#include "createdlg.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qxml.h>
#include <qresource.h>

void manipulateXML( QResourceItem* item, bool _root = TRUE );

/**********************************************************
 *
 * DMainWindow
 *
 **********************************************************/

/*!
  \class DMainWindow mainwindow.h
  \brief The main window of the builder
*/

/*!
  Constructs a new mainwindow and an empty form.
*/
DMainWindow::DMainWindow()
{
  QWidget* w = new QWidget( this );
  setCentralWidget( w );

  QVBoxLayout *v = new QVBoxLayout( w );

  v->addSpacing( 3 );

  m_widgetsBar = new DWidgetsBar( w );
  v->addWidget( m_widgetsBar );

  v->addSpacing( 6 );

  m_splitter = new QSplitter( w );
  v->addWidget( m_splitter );

  m_inspector = new DInspector( m_splitter );
  m_editorContainer = new DEditorContainer( m_splitter );

  DFormEditor* editor = new DFormEditor( m_editorContainer );
  m_editorContainer->addEditor( tr("unnamed"), editor );
  connect( editor, SIGNAL( objectSelected( DFormEditor*, DObjectInfo* ) ),
	   m_inspector, SLOT( inspect( DFormEditor*, DObjectInfo* ) ) );

  QMenuBar* menu = menuBar();

  QPopupMenu* popup = new QPopupMenu;
  popup->insertItem( tr("&New resource ..."), this, SLOT( slotNewResource() ) );
  popup->insertItem( tr("&Open ..."), this, SLOT( slotOpen() ) );
  popup->insertItem( tr("Save &as ..."), this, SLOT( slotSaveAs() ) );
  popup->insertSeparator();
  popup->insertItem( tr("E&xit"), qApp, SLOT( quit() ) );
  menu->insertItem( tr("&File"), popup );

  /* popup = new QPopupMenu;
  popup->insertItem( tr("&New resource ..."), this, SLOT( slotNewResource() ) );
  menu->insertItem( tr("&Edit"), popup ); */

  popup = new QPopupMenu;
  popup->insertItem( tr("&Preview"), this, SLOT( slotPreview() ) );
  menu->insertItem( tr("&View"), popup );

  popup = new QPopupMenu;
  // popup->insertItem( tr("&Auto Arrange"), m_formEditor, SLOT( slotAutoArrange() ) );
  popup->insertItem( tr("Arrange selection in &grid"), this, SLOT( slotGridArrange() ) );
  popup->insertItem( tr("Arrange selection &vertical"), this, SLOT( slotVArrange() ) );
  popup->insertItem( tr("Arrange selection &horizontal"), this, SLOT( slotHArrange() ) );
  popup->insertSeparator();
  popup->insertItem( tr("Apply &size hint"), this, SLOT( slotApplySizeHint() ) );
  menu->insertItem( tr("&Arrange"), popup );
}

void DMainWindow::slotApplySizeHint()
{
  DFormEditor* e = (DFormEditor*)m_editorContainer->currentPage();
  if ( !e )
    return;

  e->slotApplySizeHint();
}

void DMainWindow::slotGridArrange()
{
  DFormEditor* e = (DFormEditor*)m_editorContainer->currentPage();
  if ( !e )
    return;

  e->slotGridArrange();
}

void DMainWindow::slotVArrange()
{
  DFormEditor* e = (DFormEditor*)m_editorContainer->currentPage();
  if ( !e )
    return;

  e->slotVArrange();
}

void DMainWindow::slotHArrange()
{
  DFormEditor* e = (DFormEditor*)m_editorContainer->currentPage();
  if ( !e )
    return;

  e->slotHArrange();
}

void DMainWindow::slotOpen()
{
  QString file = QFileDialog::getOpenFileName();
  if ( file.isEmpty() )
    return;

  slotOpen( file );
}

void DMainWindow::slotOpen( const QString& file )
{
  // try to open the file for reading
  QFile f( file );
  if ( !f.open( IO_ReadOnly ) )
  {
    // TODO: message box
    ASSERT(0);
  }
  f.close();

  if ( m_editorContainer )
    delete m_editorContainer;

  m_editorContainer = new DEditorContainer( m_splitter );
  
  QResource resource( file );
  if ( resource.isEmpty() )
  {
    // TODO: Error message
    ASSERT( 0 );
  }

  // We have to adjust the XML tree here to insert DFormWidgets at certain places.
  manipulateXML( resource.tree() );

  // HACK
  {
    QFile f( "out" );
    if ( !f.open( IO_WriteOnly ) )
    ASSERT(0);

    {
      QTextStream text( &f );
      text << *(resource.tree());
    }
    f.close();
  }
  // END HACK

  DFormEditor* editor = new DFormEditor( resource, m_editorContainer );
  // TODO: Test wether loading was ok!

  QString name = tr("unnamed");
  if ( resource.tree()->hasAttrib( "name" ) )
    name = resource.tree()->attrib( "name" );
  
  m_editorContainer->addEditor( name, editor );
  connect( editor, SIGNAL( objectSelected( DFormEditor*, DObjectInfo* ) ),
	   m_inspector, SLOT( inspect( DFormEditor*, DObjectInfo* ) ) );

  m_editorContainer->show();
}

void DMainWindow::slotSaveAs()
{
  // TODO
  ASSERT( m_editorContainer->currentEditor() );

  QString file = QFileDialog::getSaveFileName();
  if ( file.isEmpty() )
    return;

  QFile f( file );
  if ( !f.open( IO_WriteOnly ) )
  {
    // TODO: message box
    ASSERT(0);
  }

  // Save as XML
  {
    QTextStream text( &f );

    QResourceItem* tree = m_editorContainer->currentEditor()->save();
    text << *tree;
  }
  f.close();
}

void DMainWindow::slotNew()
{
  if ( m_editorContainer )
    delete m_editorContainer;

  m_editorContainer = new DEditorContainer( m_splitter );
  
  DFormEditor* editor = new DFormEditor( m_editorContainer );
  m_editorContainer->addEditor( tr("unnamed"), editor );
  connect( editor, SIGNAL( objectSelected( DFormEditor*, DObjectInfo* ) ),
	   m_inspector->propertyInspector(), SLOT( inspect( DFormEditor*, DObjectInfo* ) ) );

  m_editorContainer->show();
}

void DMainWindow::slotPreview()
{
  if ( m_editorContainer->currentEditor() == 0 )
    // TODO: Give error message
    return;

  // Delete old preview
  QWidget* old = m_dctPreviewWidgets[ m_editorContainer->currentEditor() ];
  if ( old )
  {
    m_dctPreviewWidgets.remove( m_editorContainer->currentEditor() );
    delete old;
  }

  // Write form to XML
  QResourceItem* tree = m_editorContainer->currentEditor()->save();

  // Create new preview from XML
  {
    QResource resource( tree );

    QWidget* w = resource.createWidget( 0 );
    m_dctPreviewWidgets.insert( m_editorContainer->currentEditor(), w );
    w->show();
  }
}

void DMainWindow::slotNewResource()
{
  // ### HACK correct path here
  QResource resource( "wizard.qdl" );
  DCreateWizard dlg( this, resource );
  dlg.exec();
}

void DMainWindow::addResource( const QString& name, QResource& templ )
{
  // We have to adjust the XML tree here to insert DFormWidgets at certain places.
  manipulateXML( templ.tree() );

  DFormEditor* editor = new DFormEditor( templ, m_editorContainer );
  m_editorContainer->addEditor( name, editor );
  connect( editor, SIGNAL( objectSelected( DFormEditor*, DObjectInfo* ) ),
	   m_inspector->propertyInspector(), SLOT( inspect( DFormEditor*, DObjectInfo* ) ) );

  m_editorContainer->show();
}

DMainWindow::~DMainWindow()
{
}

/**********************************************************
 *
 * DEditorContainer
 *
 **********************************************************/

DEditorContainer::DEditorContainer( QWidget* _parent ) : QTabWidget( _parent )
{
}

DEditorContainer::~DEditorContainer()
{
}

void DEditorContainer::addEditor( const QString& _name, DFormEditor* _editor )
{
  addTab( _editor, _name );
  m_lstEditors.append( _editor );
}

DFormEditor* DEditorContainer::currentEditor()
{
  return (DFormEditor*)currentPage();
}

/**********************************************************
 *
 * XML Tools
 *
 **********************************************************/

void manipulateXML( QResourceItem* it, bool _root )
{
  /**
   * A usual QWidget is a placeholder that can container other widgets.
   * So we replace it with a DFormWidget. During saving we will revert that.
   * If the top most widget in the resource tree is a QWidget or if the QWidget
   * is the central widget of a QMainWindow then we set its mode to TopMost.
   * That means that during saving it saves with a <QWidget> tag instead of
   * a <Widget> or <Layout> tag.
   */
  if ( it->type() == "QWidget" )
  {
    it->setType( "DFormWidget" );
    if ( _root || ( it->parent() && it->parent()->type() == "CentralWidget" ) )
      it->insertAttrib( "__mode", "TopMost" );
    else
      it->insertAttrib( "__mode", "Container" );
  }
  /**
   * Everything is a DGridLayout, so transform the ResourceTree.
   */
  else if ( it->type() == "QGridLayout" )
  {
    it->setType( "DGridLayout" );
    it->insertAttrib( "__mode", "grid" );
  }
  else if ( it->type() == "QHBoxLayout" )
  {
    it->setType( "DGridLayout" );
    it->insertAttrib( "__mode", "horizontal" );
  }
  else if ( it->type() == "QVBoxLayout" )
  {
    it->setType( "DGridLayout" );
    it->insertAttrib( "__mode", "vertical" );
  }
  /**
   * Special handling for QListView.
   */
  else if ( it->type() == "QListView" )
  {
    it->setType( "DListView" );
  }
  /**
   * Special handling for QToolBar.
   */
  else if ( it->type() == "QToolBar" )
  {
    QResourceItem* n = new QResourceItem( "XXXX" );
    it->append( n );
    QResourceItem* f = new QResourceItem( "DFormWidget" );
    // This mode tells the DFormWidget to save with a <QToolBar> tag.
    f->insertAttrib( "__mode", "ToolBar" );
    n->append( f );
    QResourceItem* l = new QResourceItem( "Layout" );
    f->append( l );
    QResourceItem* h = new QResourceItem( "DGridLayout" );
    l->append( h );
    h->insertAttrib( "__mode", "horizontal" );
    // h->insertAttrib( "border", "0" );

    QResourceItem* i = it->firstChild();
    while( i )
    {
      if ( i->type() == "Widget" || i->type() == "WhatsThis" )
      {
	h->append( it->extract( i ) );
	i = it->firstChild();
      }
      else if ( i->type() == "Separator" )
      {
	it->remove( i );
	QResourceItem* w = new QResourceItem( "Widget" );
	h->append( w );
	QResourceItem* s = new QResourceItem( "DSeparator" );
	w->append( s );
	i = it->firstChild();
      }
      else
	i = i->nextSibling();
    }

    n->setType( "Widget" );
  }

  // Manipulate children
  QResourceItem* i = it->firstChild();
  while( i )
  {
    /**
     * Special handling for children of container classes.
     */
    if ( it->type() == "QDialog" && i->type() == "Layout" )
    {
      QResourceItem* l = new QResourceItem( "Layout" );
      QResourceItem* v = new QResourceItem( "QVBoxLayout" );
      QResourceItem* t = new QResourceItem( "Widget" );
      QResourceItem* w = new QResourceItem( "DFormWidget" );
      w->insertAttrib( "__mode", "Container" );
      t->append( w );
      v->append( t );
      l->append( v );
      w->append( it->extractAndReplace( i, l ) );
      i = t;
    }
    /**
     * Fill in DFormWidgets in 
     * 1) Cells of DGridLayout
     * 2) Pages of QWizard
     * 3) Layouts in a QVBoxLayout/QHBoxLayout 
     */
    else if ( it->type() != "DFormWidget" && i->type() == "Layout" )
    {
      QResourceItem* t = new QResourceItem( "Widget" );
      QResourceItem* w = new QResourceItem( "DFormWidget" );
      w->insertAttrib( "__mode", "Container" );
      t->append( w );
      w->append( it->extractAndReplace( i, t ) );
      i = t;
    }
    i = i->nextSibling();
  }

  // Handle all children
  i = it->firstChild();
  for( ; i; i = i->nextSibling() )
  {
    manipulateXML( i, FALSE );
  }
}
