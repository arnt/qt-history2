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
#include <qxmlparser.h>
#include <qresource.h>

void manipulateXML( QXMLIterator it, bool _root = TRUE );

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

  // ##### HACK
  v->setRowStretch( 1, 0 );
  v->setRowStretch( 3, 1 );

  m_inspector = new DInspector( m_splitter );
  m_editorContainer = new DEditorContainer( m_splitter );

  DFormEditor* editor = new DFormEditor( m_editorContainer );
  m_editorContainer->addEditor( tr("unnamed"), editor );
  connect( editor, SIGNAL( objectSelected( DFormEditor*, DObjectInfo* ) ),
	   m_inspector, SLOT( inspect( DFormEditor*, DObjectInfo* ) ) );

  QMenuBar* menu = menuBar();

  QPopupMenu* popup = new QPopupMenu;
  popup->insertItem( tr("&New project"), this, SLOT( slotNew() ) );
  popup->insertItem( tr("&Open ..."), this, SLOT( slotOpen() ) );
  popup->insertItem( tr("Save &as ..."), this, SLOT( slotSaveAs() ) );
  popup->insertSeparator();
  popup->insertItem( tr("E&xit"), qApp, SLOT( quit() ) );
  menu->insertItem( tr("&File"), popup );

  popup = new QPopupMenu;
  popup->insertItem( tr("&New resource ..."), this, SLOT( slotNewResource() ) );
  menu->insertItem( tr("&Edit"), popup );

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
  if ( !resource.isValid() )
  {
    // TODO: Error message
    ASSERT( 0 );
  }

  // Iterate over all children
  QResource r = resource.firstChild();
  for( ; r.isValid(); r = r.nextSibling() )
  {
    // We have to adjust the XML tree here to insert DFormWidgets at certain places.
    manipulateXML( r.xmlTree() );

    DFormEditor* editor = new DFormEditor( m_editorContainer );
    if ( !editor->load( r ) )
    {
      // TODO: Error message
      ASSERT( 0 );
    }

    QString name = tr("unnamed");
    if ( r.xmlTree()->hasAttrib( "name" ) )
      name = r.xmlTree()->attrib( "name" );

    m_editorContainer->addEditor( name, editor );
    connect( editor, SIGNAL( objectSelected( DFormEditor*, DObjectInfo* ) ),
	     m_inspector, SLOT( inspect( QFormEditor*, DObjectInfo* ) ) );
  }

  m_editorContainer->show();
}

void DMainWindow::slotSaveAs()
{
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

    QXMLParseTree tree;
    m_editorContainer->save( tree.rootTag() );
    text << tree;
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
  QXMLParseTree tree;
  m_editorContainer->currentEditor()->save( tree.rootTag() );

  // Create new preview from XML
  {
    QResource resource( &tree );
    // We konw that it has only one child at all;
    QResource r2( resource.firstChild() );
    if ( !r2.isValid() )
    {
      // TODO: Give error message
      return;
    }

    QWidget* w = r2.createWidget( 0 );
    m_dctPreviewWidgets.insert( m_editorContainer->currentEditor(), w );
    w->show();
  }
}

void DMainWindow::slotNewResource()
{
  // ### HACK correct path here
  QResource resource( "wizard.qdl" );
  // ### HACK search with name of child
  QResource wizard = resource.firstChild();
  DCreateWizard dlg( this, wizard );
  dlg.exec();
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

void DEditorContainer::save( QXMLTag* _tag )
{
  QListIterator<DFormEditor> it( m_lstEditors );
  for( ; it.current(); ++it )
    it.current()->save( _tag );
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

void manipulateXML( QXMLIterator it, bool _root )
{
  // TODO: If root test wether we support this Widget class as root.
  
  if ( it->tagName() == "QWidget" )
  {
    debug("-- SUBSTITUE QWidget");
    it->setTagName( "DFormWidget" );
    if ( _root )
      it->insertAttrib( "__mode", "TopMost" );
    else
      it->insertAttrib( "__mode", "Container" );
  }
  else if ( it->tagName() == "QGridLayout" )
  {
    debug("-- SUBSTITUE QGridLayout");
    // TODO: insert intermediate DFormWidgets
    it->setTagName( "DGridLayout" );
    it->insertAttrib( "__mode", "grid" );
  }
  else if ( it->tagName() == "QHBoxLayout" )
  {
    it->insertAttrib( "__mode", "horizontal" );
  }
  else if ( it->tagName() == "QVBoxLayout" )
  {
    it->insertAttrib( "__mode", "vertical" );
  }

  // Manipulate children
  QXMLIterator i = it->begin();
  while( i.isValid() )
  {
    if ( it->tagName() == "Cell" && i->tagName() == "Layout" )
    {
      QXMLTag* t = new QXMLTag( "Widget" );
      QXMLTag* w = new QXMLTag( "DFormWidget" );
      w->insertAttrib( "__mode", "Container" );
      QXMLIterator tmp = i->nextSibling();
      t->insert( w );
      w->insert( it->extractAndReplace( i, t ) );
      i = tmp;
    }

    ++i;
  }

  // Handle all children
  i = it->begin();
  for( ; i != it->end(); ++i )
  {
    manipulateXML( i, FALSE );
  }
}
