#include "createdlg.h"
#include "mainwindow.h"

#include <qresource.h>

DCreateWizard::DCreateWizard( DMainWindow* parent, const QResource& resource )
  : DCreateWizard_skel( parent, resource )
{
  m_pMainWindow = parent;

  setApproprate( mwtmpl->mainWidget(), FALSE );
  setApproprate( dlgtmpl->mainWidget(), FALSE );
  setApproprate( last->mainWidget(), FALSE );
}

DCreateWizard::~DCreateWizard()
{
}

void DCreateWizard::slotDialog( bool _selected )
{
  printf("Dialog selected\n" );

  if ( _selected )
  {
    setApproprate( mwtmpl->mainWidget(), FALSE );
    setApproprate( dlgtmpl->mainWidget(), TRUE );
    // setFinishEnabled( page1->mainWidget(), TRUE );
  }
}

void DCreateWizard::slotMainWindow( bool _selected )
{
  if ( _selected )
  {
    setApproprate( dlgtmpl->mainWidget(), FALSE );
    setApproprate( mwtmpl->mainWidget(), TRUE );
    // setFinishEnabled( page1->mainWidget(), TRUE );
  }
}

void DCreateWizard::slotVertical( bool )
{
}

void DCreateWizard::slotHorizontal( bool )
{
}
