#include "setupwizardimpl.h"
#include <qfiledialog.h>
#include <qlineedit.h>

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) : SetupWizard( pParent, pName, modal, f )
{
    connect( this, SIGNAL( selected( const QString& ) ), this, SLOT( changedPage( const QString& ) ) );
    setFinishEnabled( finishPage, true );
    setBackEnabled( progressPage, false );
    setBackEnabled( finishPage, false );
}

void SetupWizardImpl::clickedPath()
{
    QFileDialog dlg( installPath->text(), QString::null, this, NULL, true );

    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() )
    {
	installPath->setText( dlg.dir()->absPath() );
    }
}

void SetupWizardImpl::clickedSystem( int sys )
{
    sysID = sys;
}

void SetupWizardImpl::changedPage( const QString& pageName )
{
    if( pageName == "Installing" ) {
	
    }
}