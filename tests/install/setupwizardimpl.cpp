#include "setupwizardimpl.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) : SetupWizard( pParent, pName, modal, f )
{
    connect( this, SIGNAL( selected( const QString& ) ), this, SLOT( changedPage( const QString& ) ) );
    setFinishEnabled( finishPage, true );
    setBackEnabled( progressPage, false );
    setBackEnabled( finishPage, false );
    setNextEnabled( progressPage, false );
    copyPixmap->hide();
    configurePixmap->hide();
    preparePixmap->hide();
    compilePixmap->hide();
    operationProgress->setTotalSteps( 100 );
    operationProgress->setProgress( 0 );
}

void SetupWizardImpl::clickedPath()
{
    QFileDialog dlg;
    QDir installDir( installPath->text() );

    if( !installDir.exists() )
	installDir.setPath( "C:\\" );

    dlg.setDir( installDir );
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
	installer.GUI = this;
	installer.start();
    }
    else if( pageName == "Options" ) {
	installPath->setText( QString( "C:\\Qt\\" ) + QString( DISTVER ) );
    }
}
