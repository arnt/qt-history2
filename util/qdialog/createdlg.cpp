#include "createdlg.h"
#include "mainwindow.h"

#include <qresource.h>
#include <qpushbutton.h>

DCreateWizard::DCreateWizard( DMainWindow* parent, const QResource& resource )
  : DCreateWizard_skel( parent, resource )
{
  m_pMainWindow = parent;

  setNextEnabled( first->mainWidget(), FALSE );
  setNextEnabled( mwtmpl->mainWidget(), FALSE );
  setNextEnabled( dlgtmpl->mainWidget(), FALSE );

  setAppropriate( mwtmpl->mainWidget(), FALSE );
  setAppropriate( dlgtmpl->mainWidget(), FALSE );

  connect( finishButton(), SIGNAL( clicked() ), this, SLOT( slotFinished() ) );
}

DCreateWizard::~DCreateWizard()
{
}

void DCreateWizard::slotDialog( bool selected )
{
  if ( selected )
  {
    setAppropriate( mwtmpl->mainWidget(), FALSE );
    setAppropriate( dlgtmpl->mainWidget(), TRUE );
    setNextEnabled( first->mainWidget(), TRUE );
    setNextEnabled( dlgtmpl->mainWidget(), FALSE );
 
    m_type = Dialog;
  }
}

void DCreateWizard::slotMainWindow( bool selected )
{
  if ( selected )
  {
    setAppropriate( dlgtmpl->mainWidget(), FALSE );
    setAppropriate( mwtmpl->mainWidget(), TRUE );
    setNextEnabled( first->mainWidget(), TRUE );
    setNextEnabled( mwtmpl->mainWidget(), FALSE );

    m_type = MainWindow;
  }
}

void DCreateWizard::slotVertical( bool selected )
{
  if ( !selected )
    return;

  m_dialogType = Dialog_VerticalButtons;

  setNextEnabled( dlgtmpl->mainWidget(), TRUE );
}

void DCreateWizard::slotHorizontal( bool selected )
{
  if ( !selected )
    return;

  m_dialogType = Dialog_HorizontalButtons;

  setNextEnabled( dlgtmpl->mainWidget(), TRUE );
}

void DCreateWizard::slotSimple( bool selected )
{
  if ( !selected )
    return;

  m_mainWindowType = MainWindow_Simple;

  setNextEnabled( mwtmpl->mainWidget(), TRUE );
}

void DCreateWizard::slotFullFeatured( bool selected )
{
  if ( !selected )
    return;

  m_mainWindowType = MainWindow_FullFeatured;

  setNextEnabled( mwtmpl->mainWidget(), TRUE );
}

void DCreateWizard::slotNameChanged( const QString& name )
{
  if ( name.isEmpty() )
    setFinishEnabled( last->mainWidget(), FALSE );
  else
    setFinishEnabled( last->mainWidget(), TRUE );

  m_name = name;
}

void DCreateWizard::slotFinished()
{
  QString tmpl;

  if ( m_type == Dialog )
  {
    if ( m_dialogType == Dialog_VerticalButtons )
      tmpl = "vdialog.qdl";
    else if ( m_dialogType == Dialog_HorizontalButtons )
      tmpl = "hdialog.qdl";
    else
      ASSERT( 0 );
  }
  else if ( m_type == MainWindow )
  {
    if ( m_mainWindowType == MainWindow_Simple )
      tmpl = "smainwindow.qdl";
    else if ( m_mainWindowType == MainWindow_FullFeatured )
      tmpl = "fmainwindow.qdl";
  }
  else
    ASSERT(0);

  // TODO: Correct paths
  QString path = "templates/";
  path += tmpl;

  QResource resource( path );
  ASSERT( !resource.isEmpty() );

  m_pMainWindow->addResource( m_name, resource );

  hide();
}
