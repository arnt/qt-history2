#include "setupwizardimpl.h"
#include "installevent.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qmultilineedit.h>
#include <windows.h>

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) : SetupWizard( pParent, pName, modal, f ), tmpPath( 256 )
{
    setNextEnabled( introPage, false );
//    setFinishEnabled( finishPage, true );
    setBackEnabled( progressPage, false );
//    setBackEnabled( finishPage, false );
    setNextEnabled( progressPage, false );

    // Read the installation control files
    GetTempPath( tmpPath.size(), tmpPath.data() );

    installer.readArchive( "sys.arq", tmpPath.data() );
}

void SetupWizardImpl::clickedPath()
{
    QFileDialog dlg;
    QDir installDir( installPath->text() );

    if( !installDir.exists() )
	installDir.setPath( "C:\\" );

    dlg.setDir( installDir );
    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() ) {
	installPath->setText( dlg.dir()->absPath() );
    }
}

void SetupWizardImpl::clickedSystem( int sys )
{
    sysID = sys;
}

void SetupWizardImpl::showPage( QWidget* newPage )
{
    SetupWizard::showPage( newPage );

    if( newPage == introPage ) {
	QFile licenseFile( QString( tmpPath.data() ) + "\\LICENSE" );
	if( licenseFile.open( IO_ReadOnly ) ) {
	    QByteArray fileData;
	    QFileInfo fi( licenseFile );
	    
	    if( !fileData.resize( fi.size() ) )
		qFatal( "Could not allocate memory for license text!" );
	    licenseFile.readBlock( fileData.data(), fileData.size() );
	    introText->setText( QString( fileData.data() ) );
	}
    }
    else if( newPage == optionsPage ) {
	installPath->setText( QString( "C:\\Qt\\" ) + QString( DISTVER ) );
    }
    else if( newPage == progressPage ) {
	installer.GUI = this;
	installer.start();
    }
}

void SetupWizardImpl::licenseAccepted( )
{
    setNextEnabled( introPage, true );
}

void SetupWizardImpl::customEvent( QCustomEvent* pEvent )
{
    if( pEvent->type() == InstallEvent::eventID ) {
	InstallEvent* pIEvent = (InstallEvent*)pEvent;

	printf( "%12d %s\n",pIEvent->progress, pIEvent->fileName );
	operationProgress->setProgress( pIEvent->progress );
	filesDisplay->setText( pIEvent->fileName );
    }
}