#include "mainwindow.h"
#include "widgetsbar.h"
#include "formeditor.h"
#include "inspector.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>

DMainWindow::DMainWindow()
{
  QWidget* w = new QWidget( this );
  setCentralWidget( w );

  QVBoxLayout *v = new QVBoxLayout( w );

  v->addSpacing( 3 );

  m_widgetsBar = new DWidgetsBar( w );
  v->addWidget( m_widgetsBar );

  v->addSpacing( 6 );

  QSplitter *splitter = new QSplitter( w );
  v->addWidget( splitter );

  m_inspector = new DInspector( splitter );
  m_formEditor = new DFormEditor( splitter );

  QMenuBar* menu = menuBar();

  QPopupMenu* popup = new QPopupMenu;
  popup->insertItem( tr("Exit"), qApp, SLOT( quit() ) );
  menu->insertItem( tr("File"), popup );

  popup = new QPopupMenu;
  popup->insertItem( tr("&Auto Arrange"), m_formEditor, SLOT( slotAutoArrange() ) );
  popup->insertItem( tr("Arrange in &Grid"), m_formEditor, SLOT( slotGridArrange() ) );
  popup->insertItem( tr("Arrange &Vertical"), m_formEditor, SLOT( slotVArrange() ) );
  popup->insertItem( tr("Arrange &Horizontal"), m_formEditor, SLOT( slotHArrange() ) );
  popup->insertSeparator();
  popup->insertItem( tr("Apply Size Hint"), m_formEditor, SLOT( slotApplySizeHint() ) );
  menu->insertItem( tr("Arrange"), popup );
}

DMainWindow::~DMainWindow()
{
}
