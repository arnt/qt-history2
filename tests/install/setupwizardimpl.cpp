#include "setupwizardimpl.h"
#include "installevent.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qmultilineedit.h>
#include <windows.h>
#include <qbuttongroup.h>
#include <qsettings.h>
#include <qlistview.h>

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) : SetupWizard( pParent, pName, modal, f ), tmpPath( 256 )
{
    sysID = 0;
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
    HKEY environmentKey;

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
	sysGroup->setButton( 0 );
    }
    else if( newPage == progressPage ) {
	installer.GUI = this;
	installer.start();
    }
    else if( newPage == configPage ) {
	QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++" );

	putenv( QString( "QTDIR=" ) + installPath->text().latin1() );
	putenv( QString( "MKSPEC=" ) + mkSpecs[ sysID ].latin1() );

	if( ( RegOpenKeyEx( HKEY_CURRENT_USER, "Environment", 0, KEY_WRITE, &environmentKey ) ) == ERROR_SUCCESS ) {
	    RegSetValueEx( environmentKey, "QTDIR", 0, REG_SZ, (const unsigned char*)installPath->text().latin1(), installPath->text().length() + 1 );
	    RegSetValueEx( environmentKey, "MKSPEC", 0, REG_SZ, (const unsigned char*)mkSpecs[ sysID ].latin1(), mkSpecs[ sysID ].length() + 1 );
	}
	// Tell QSettings where we keep our secrets
        QSettings::insertSearchPath( QString( getenv( "QTDIR" ) ) );
	configList->setSorting( -1 );
	advancedList->setSorting( -1 );
	QCheckListItem* item;
	connect( &configure, SIGNAL( processExited() ),
		 this, SLOT( configDone() ) );
	connect( &configure, SIGNAL( readyReadStdout() ),
		 this, SLOT( readConfigureOutput() ) );
	connect( &configure, SIGNAL( readyReadStderr() ),
		 this, SLOT( readConfigureOutput() ) );

	QString qtdir = getenv( "QTDIR" );

	QString mkspecsdir = qtdir + "/mkspecs";
	QString mkspecsenv = getenv( "MKSPEC" );
	QFileInfo mkspecsenvdirinfo( mkspecsenv );
	QString srcdir = qtdir + "/src";
	QFileInfo* fi;

	// general
	modules = new QCheckListItem ( configList, "Modules" );
	modules->setOpen( TRUE );
	QDir srcDir( srcdir, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* srcdirs = srcDir.entryInfoList();
	QFileInfoListIterator srcDirIterator( *srcdirs );
	srcDirIterator.toLast();
	while ( ( fi = srcDirIterator.current() ) ) {
	    if ( fi->fileName()[0] != '.' && // fi->fileName() != ".." &&
		 fi->fileName() != "tmp" &&
		 fi->fileName() != "compat" &&
		 fi->fileName() != "3rdparty" &&
		 fi->fileName() != "Debug" && // MSVC directory
		 fi->fileName() != "Release" && // MSVC directory
		 fi->fileName() != "moc" ) {
		item = new QCheckListItem( modules, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --srcDirIterator;
	}
	threadModel = new QCheckListItem ( configList, "Threading" );
	threadModel->setOpen( TRUE );
	item = new QCheckListItem( threadModel, "Threaded", QCheckListItem::RadioButton );
	item = new QCheckListItem( threadModel, "Non-threaded", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	buildType = new QCheckListItem ( configList, "Build" );
	buildType->setOpen( TRUE );
	item = new QCheckListItem( buildType, "Static", QCheckListItem::RadioButton );
	item = new QCheckListItem( buildType, "Shared", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	debugMode = new QCheckListItem ( configList, "Mode" );
	debugMode->setOpen( TRUE );
	item = new QCheckListItem( debugMode, "Debug", QCheckListItem::RadioButton );
	item = new QCheckListItem( debugMode, "Release", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	// advanced
	sqldrivers = new QCheckListItem ( advancedList, "SQL Drivers" );
	sqldrivers->setOpen( true );
	QDir sqlsrcDir( srcdir + "/sql/src", QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* sqlsrcdirs = sqlsrcDir.entryInfoList();
	QFileInfoListIterator sqlsrcDirIterator( *sqlsrcdirs );
	sqlsrcDirIterator.toLast();
	while ( ( fi = sqlsrcDirIterator.current() ) ) {
	    if ( fi->fileName() != "." && fi->fileName() != ".." && fi->fileName() != "tmp" ) {
		item = new QCheckListItem( sqldrivers, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --sqlsrcDirIterator;
	}



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

	
	switch( pIEvent->eType ) {
	case InstallEvent::updateProgress:
	    operationProgress->setProgress( pIEvent->progress );
//	    filesDisplay->append( pIEvent->fileName );
	    filesDisplay->setText( pIEvent->fileName );
	    break;
	case InstallEvent::moveNext:
	    next();
	    break;
	}
    }
}

void SetupWizardImpl::readConfigureOutput()
{
}

void SetupWizardImpl::configDone()
{
}
