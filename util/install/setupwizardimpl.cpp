#include "setupwizardimpl.h"
#include "environment.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qmultilineedit.h>
#include <qbuttongroup.h>
#include <qsettings.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <zlib/zlib.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtabwidget.h>

#define BUFFERSIZE 64 * 1024
#define FILESTOCOPY 4582

//#define USE_ARCHIVES 1

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) :
    SetupWizard( pParent, pName, modal, f ),
    filesCopied( false ),
    filesToCompile( 0 ),
    filesCompiled( 0 ),
    sysID( 0 ),
    tmpPath( QEnvironment::getTempPath() )
{
    totalFiles = 0;
    // Disable the HELP button
    setHelpEnabled( introPage, false );
    setHelpEnabled( licensePage, false );
    setHelpEnabled( optionsPage, false );
    setHelpEnabled( configPage, false );
    setHelpEnabled( progressPage, false );
    setHelpEnabled( buildPage, false );
    setHelpEnabled( finishPage, false );

    setNextEnabled( introPage, false );
    setNextEnabled( licensePage, false );
    setBackEnabled( progressPage, false );
    setNextEnabled( progressPage, false );
    setBackEnabled( buildPage, false );
    setNextEnabled( buildPage, false );
    setFinishEnabled( finishPage, true );
    setBackEnabled( finishPage, false );
#if defined (USE_ARCHIVES)
    readArchive( "sys.arq", tmpPath );
#endif
    triedToIntegrate = false;

    connect( &autoContTimer, SIGNAL( timeout() ), this, SLOT( timerFired() ) );

    // Intropage
#ifdef USE_ARCHIVES
    QFile licenseFile( tmpPath + "/LICENSE" );
#else
    QFile licenseFile( "LICENSE" );
#endif
    if( licenseFile.open( IO_ReadOnly ) ) {
	QFileInfo fi( licenseFile );
	QByteArray fileData( fi.size() + 2 );
	licenseFile.readBlock( fileData.data(), fi.size() );
	fileData.data()[ fi.size() ] = 0;
	fileData.data()[ fi.size() + 1 ] = 0;
	introText->setText( QString( fileData.data() ) );
    }
    // Optionspage
    installPath->setText( QString( "C:\\Qt\\" ) + QT_VERSION_STR );
    sysGroup->setButton( 0 );
    // Folderspage
    QByteArray buffer( 256 );
    unsigned long buffSize( buffer.size() );
    GetUserNameA( buffer.data(), &buffSize );
    folderGroups->insertItem( "Anyone who uses this computer (all users)" );
    folderGroups->insertItem( QString( "Only for me (" ) + QString( buffer.data() ) + ")" );
    folderPath->setText( QString( "Qt " ) + QT_VERSION_STR );
    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) )   // On NT we also have a common folder
	folderGroups->setEnabled( true );
    else
	folderGroups->setDisabled( true );
}

void SetupWizardImpl::stopProcesses()
{
    if( configure.isRunning() )
	configure.hangUp();
    if( make.isRunning() )
	make.hangUp();
    if( integrator.isRunning() )
	integrator.hangUp();
}

void SetupWizardImpl::clickedPath()
{
    QFileDialog dlg;
    QDir dir( installPath->text() );

    if( !dir.exists() )
	dir.setPath( "C:\\" );

    dlg.setDir( dir );
    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() ) {
	installPath->setText( dlg.dir()->absPath() );
    }
}

void SetupWizardImpl::clickedFolderPath()
{
    folderPath->setText( shell.selectFolder( folderPath->text(), ( folderGroups->currentItem() == 1 ) ) );
}

void SetupWizardImpl::clickedDevSysPath()
{
    QFileDialog dlg;
    QDir dir( devSysPath->text() );

    if( !dir.exists() )
	dir.setPath( devSysFolder );

    dlg.setDir( dir );
    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() ) {
	devSysPath->setText( dlg.dir()->absPath() );
    }
}

void SetupWizardImpl::clickedSystem( int sys )
{
    sysID = sys;
}

void SetupWizardImpl::licenseAction( int act )
{
    if( act )
	setNextEnabled( introPage, false );
    else
	setNextEnabled( introPage, true );
}

void SetupWizardImpl::readConfigureOutput()
{
    updateOutputDisplay( &configure );
}

void SetupWizardImpl::readMakeOutput()
{
    updateOutputDisplay( &make );
}

void SetupWizardImpl::readIntegratorOutput()
{
    updateOutputDisplay( &integrator );
}

void SetupWizardImpl::readConfigureError()
{
    updateOutputDisplay( &configure );
}

void SetupWizardImpl::readMakeError()
{
    updateOutputDisplay( &make );
}

void SetupWizardImpl::readIntegratorError()
{
    updateOutputDisplay( &integrator );
}

void SetupWizardImpl::updateOutputDisplay( QProcess* proc )
{
    QString outbuffer;

    outbuffer = QString( proc->readStdout() );
    
    for( int i = 0; i < (int)outbuffer.length(); i++ ) {
	QChar c = outbuffer[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    currentOLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    if( currentOLine.length() ) {
		if( currentOLine.right( 4 ) == ".cpp" || 
		    currentOLine.right( 2 ) == ".c" ||
		    currentOLine.right( 4 ) == ".pro" ||
		    currentOLine.right( 3 ) == ".ui" )
		    compileProgress->setProgress( ++filesCompiled );

		logOutput( currentOLine );
		currentOLine = "";
	    }
	    break;
	default:
	    currentOLine += c;
	    break;
	}
    }
}

void SetupWizardImpl::updateErrorDisplay( QProcess* proc )
{
    QString outbuffer;

    outbuffer = QString( proc->readStderr() );
    
    for( int i = 0; i < (int)outbuffer.length(); i++ ) {
	QChar c = outbuffer[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    currentELine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    if( currentELine.length() ) {
		if( currentOLine.right( 4 ) == ".cpp" || 
		    currentOLine.right( 2 ) == ".c" || 
		    currentOLine.right( 4 ) == ".pro" )
		    compileProgress->setProgress( ++filesCompiled );

		logOutput( currentELine );
		currentELine = "";
	    }
	    break;
	default:
	    currentELine += c;
	    break;
	}
    }
}

void SetupWizardImpl::installIcons( const QString& iconFolder, const QString& dirName, bool common )
{
    QDir dir( dirName );

    dir.setSorting( QDir::Name | QDir::IgnoreCase );
    const QFileInfoList* filist = dir.entryInfoList();
    QFileInfoListIterator it( *filist );
    QFileInfo* fi;

    while( ( fi = it.current() ) ) {
	if( fi->fileName()[0] != '.' ) { // Exclude dot-dirs
	    if( fi->isDir() )
		installIcons( iconFolder, fi->absFilePath(), common );
	    else if( fi->fileName().right( 4 ) == ".exe" )
		shell.createShortcut( iconFolder, common, fi->baseName(), fi->absFilePath() );
	}
	++it;
    }
}

void SetupWizardImpl::doFinalIntegration()
{
    QString dirName, examplesName, tutorialsName;
    bool common( folderGroups->currentItem() == 1 );

    switch( sysID ) {
    case MSVC:
	{
	    QFile autoexp( devSysPath->text() + "\\Common\\MsDev98\\bin\\autoexp.dat" );

	    if( autoexp.open( IO_ReadOnly ) ) { // First try to open the file to search for existing installations
		QTextStream instream( &autoexp );
		QString existingAutoexp;

		instream >> existingAutoexp;
		if( existingAutoexp.find( "; Trolltech Qt" ) == -1 ) {
		    autoexp.close();
		    if( autoexp.open( IO_Append ) ) { // Reopen the file to append our autoexp additions
			QTextStream outstream( &autoexp );
			outstream << "; Trolltech Qt\nQString=<d->unicode,su> len=<d->len,u>\n";
		    }
		}
		if( autoexp.isOpen() )
		    autoexp.close();
	    }
	}
	break;
    }
    /*
    ** Set up our icon folder and populate it with shortcuts.
    ** Then move to the next page.
    */
    dirName = shell.createFolder( folderPath->text(), common );
    shell.createShortcut( dirName, common, "Designer", QEnvironment::getEnv( "QTDIR" ) + "\\bin\\designer.exe", "GUI designer" );
    shell.createShortcut( dirName, common, "Reconfigure Qt", QEnvironment::getEnv( "QTDIR" ) + "\\bin\\configurator.exe", "Reconfigure the Qt library" );
    shell.createShortcut( dirName, common, "License agreement", "notepad.exe", "Review the license agreement", QString( "\"" ) + QEnvironment::getEnv( "QTDIR" ) + "\\LICENSE\"" );
    shell.createShortcut( dirName, common, "On-line documentation", QEnvironment::getEnv( "QTDIR" ) + "\\bin\\assistant.exe", "Browse the On-line documentation" );
    if( int( qWinVersion() ) & int( WV_DOS_based ) )
	shell.createShortcut( dirName, common, QString( "Build Qt " ) + QT_VERSION_STR, QEnvironment::getEnv( "QTDIR" ) + "\\build.bat", "Build the Qt library" );

    if( installTutorials->isChecked() ) {
	tutorialsName = shell.createFolder( folderPath->text() + "\\Tutorials", common );
	installIcons( tutorialsName, QEnvironment::getEnv( "QTDIR" ) + "\\tutorial", common );
    }
    if( installExamples->isChecked() ) {
	examplesName = shell.createFolder( folderPath->text() + "\\Examples", common );
	installIcons( examplesName, QEnvironment::getEnv( "QTDIR" ) + "\\examples", common );
    }
    /*
    ** Then record the installation in the registry, and set up the uninstallation
    */
    QStringList uninstaller;
    uninstaller << shell.windowsFolderName + "\\quninstall.exe";
    uninstaller << installPath->text();

    if( common )
	uninstaller << ( QString( "\"" ) + shell.commonProgramsFolderName + QString( "\\" ) + folderPath->text() + QString( "\"" ) );
    else
	uninstaller << ( QString( "\"" ) + shell.localProgramsFolderName + QString( "\\" ) + folderPath->text() + QString( "\"" ) );

    uninstaller << QT_VERSION_STR;

    QEnvironment::recordUninstall( QString( "Qt " ) + QT_VERSION_STR, uninstaller.join( " " ) );
}

void SetupWizardImpl::integratorDone()
{
    if( ( !integrator.normalExit() || ( integrator.normalExit() && integrator.exitStatus() ) ) && ( triedToIntegrate ) )
	logOutput( "The integration process failed.\n", true );
    else {
	setNextEnabled( buildPage, true );
    
	/*
	** We still have some more items to do in order to finish all the
	** integration stuff.
	*/
	doFinalIntegration();
	setNextEnabled( buildPage, true );

// The automatic continue feature has been disabled
//	timeCounter = 30;
//	autoContTimer.start( 1000 );
	logOutput( "The build was successful", true );
    }
}

void SetupWizardImpl::makeDone()
{
    QStringList args;
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );

    if( !make.normalExit() || ( make.normalExit() && make.exitStatus() ) )
	logOutput( "The build process failed.\n" );
    else {
	compileProgress->setProgress( compileProgress->totalSteps() );

	if( sysID != MSVC )
	    integratorDone();
	else {
	    connect( &integrator, SIGNAL( processExited() ), this, SLOT( integratorDone() ) );
	    connect( &integrator, SIGNAL( readyReadStdout() ), this, SLOT( readIntegratorOutput() ) );
	    connect( &integrator, SIGNAL( readyReadStderr() ), this, SLOT( readIntegratorError() ) );

	    args << "nmake";

	    integrator.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) + "\\Tools\\Designer\\Integration\\QMsDev" );
	    integrator.setArguments( args );
	    triedToIntegrate = true;
	    if( !integrator.start() )
		logOutput( "Could not start integrator process" );
	}
    }
}

void SetupWizardImpl::configDone()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
    QStringList args;

    if( !configure.normalExit() || ( configure.normalExit() && configure.exitStatus() ) )
	logOutput( "The configure process failed.\n" );
    else {
	connect( &make, SIGNAL( processExited() ), this, SLOT( makeDone() ) );
	connect( &make, SIGNAL( readyReadStdout() ), this, SLOT( readMakeOutput() ) );
	connect( &make, SIGNAL( readyReadStderr() ), this, SLOT( readMakeError() ) );

	args << makeCmds[ sysID ];

	make.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	make.setArguments( args );

	if( !make.start() )
	    logOutput( "Could not start make process" );
    }
}

void SetupWizardImpl::saveSettings()
{
    QApplication::setOverrideCursor( Qt::waitCursor );
    saveSet( configList );
    saveSet( advancedList );
    QApplication::restoreOverrideCursor();
}

void SetupWizardImpl::saveSet( QListView* list )
{
    QSettings settings;
    settings.writeEntry( "/Trolltech/Qt/ResetDefaults", "FALSE" );

    QListViewItemIterator it( list );
    while ( it.current() ) {
	QListViewItem *itm = it.current();
	++it;
	if ( itm->rtti() != QCheckListItem::RTTI )
	    continue;	
	QCheckListItem *item = (QCheckListItem*)itm;
	if ( item->type() == QCheckListItem::RadioButton ) {
	    if ( item->isOn() )
		settings.writeEntry( "/Trolltech/Qt/" + item->parent()->text(0), item->text() );
	} else if ( item->type() == QCheckListItem::CheckBox ) {
	    QStringList lst;
	    QListViewItem *p = item->parent();
	    if ( p )
		--it;
	    QString c = p->text( 0 );
	    while ( ( itm = it.current() ) &&
		itm->rtti() == QCheckListItem::RTTI &&
		item->type() == QCheckListItem::CheckBox ) {
		item = (QCheckListItem*)itm;
		++it;
		if ( item->isOn() )
		    lst << item->text( 0 );
	    }
	    if ( lst.count() )
		settings.writeEntry( "/Trolltech/Qt/" + p->text(0), lst, ',' );
	}
    }
}

void SetupWizardImpl::showPage( QWidget* newPage )
{
    SetupWizard::showPage( newPage );

    if( newPage == introPage ) {
	setInstallStep( 1 );
    }
    else if( newPage == optionsPage ) {
	setInstallStep( 2 );
    }
    else if( newPage == licensePage ) {
	setInstallStep( 3 );
	readLicense( installPath->text() + "/.qt-license" );
    }
    else if( newPage == foldersPage ) {
	QStringList devSys = QStringList::split( ';',"Microsoft Visual Studio path;Borland C++ Builder path;GNU C++ path" );

	devSysLabel->setText( devSys[ sysID ] );
	devSysPath->setEnabled( sysID == 0 );
	devSysPathButton->setEnabled( sysID == 0 );
	if( sysID == 0 ) {
	    QString devdir = QEnvironment::getEnv( "MSDevDir" );
	    if( !devdir.length() ) {
		int envSpec = QEnvironment::LocalEnv;
		QString vsCommonDir, msDevDir, msVCDir, osDir;

		if( QMessageBox::warning( this, "Environment", "The Visual C++ environment variables has not been set\nDo you want to do this now?", "Yes", "No", QString::null, 0, 1 ) == 0 ) {
		    envSpec |= QEnvironment::PersistentEnv;
		    persistentEnv = true;
		}

		vsCommonDir = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup", "VsCommonDir", QEnvironment::LocalMachine );
		    msDevDir = QEnvironment::getFSFileName( vsCommonDir + "\\MSDev98" );
		QEnvironment::putEnv( "MSDevDir", msDevDir, envSpec );
		    msVCDir = QEnvironment::getFSFileName( QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++", "ProductDir", QEnvironment::LocalMachine ) );
		QEnvironment::putEnv( "MSVCDir", msVCDir, envSpec );
		if( int( qWinVersion() ) & int( WV_NT_based ) )
		    osDir = "WINNT";
		else
		    osDir = "WIN95";
		QStringList path = QStringList::split( ';', QEnvironment::getEnv( "PATH", envSpec ) );
		QStringList::Iterator it;

		path.prepend( msDevDir + "\\BIN" );
		path.prepend( msVCDir + "\\BIN" );
		path.prepend( vsCommonDir + "\\Tools\\" + osDir );
		path.prepend( vsCommonDir + "\\Tools" );
		if( path.findIndex( installPath->text() + "\\bin" ) == -1 )
		    path.prepend( installPath->text() + "\\bin" );
		QEnvironment::putEnv( "PATH", path.join( ";" ), envSpec );
		QStringList include = QStringList::split( ';', QEnvironment::getEnv( "INCLUDE", envSpec ) );
		include.prepend( msVCDir + "\\ATL\\INCLUDE" );
		include.prepend( msVCDir + "\\INCLUDE" );
		include.prepend( msVCDir + "\\MFC\\INCLUDE" );
		QEnvironment::putEnv( "INCLUDE", include.join( ";" ), envSpec );
		QStringList lib = QStringList::split( ';', QEnvironment::getEnv( "LIB", envSpec ) );
		lib.prepend( msVCDir + "\\LIB" );
		lib.prepend( msVCDir + "\\MFC\\LIB" );
		QEnvironment::putEnv( "LIB", lib.join( ";" ), envSpec );
	    }
	}
	qtDirCheck->setChecked( ( QEnvironment::getEnv( "QTDIR" ).length() == 0 ) );
	if( sysID == 0 )
	    devSysPath->setText( QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual Studio", "ProductDir", QEnvironment::LocalMachine ) );
	setInstallStep( 4 );
    }
    else if( newPage == configPage ) {
	// First make sure that the current license information is saved
	createDir( installPath->text() );
	writeLicense( installPath->text() + "/.qt-license" );

	QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++" );
	QByteArray pathBuffer;
	QStringList path;
	QString qtDir = QDir::convertSeparators( QEnvironment::getFSFileName( installPath->text() ) );

	createDir( installPath->text() );
	
	path = QStringList::split( ';', QEnvironment::getEnv( "PATH" ) );
//	if( path.findIndex( qtDir + "\\lib" ) == -1 )
//	    path.prepend( qtDir + "\\lib" );
	if( path.findIndex( qtDir + "\\bin" ) == -1 )
	    path.prepend( qtDir + "\\bin" );
	QEnvironment::putEnv( "PATH", path.join( ";" ) );

	if( qtDirCheck->isChecked() ) {
	    QEnvironment::putEnv( "QTDIR", qtDir, QEnvironment::LocalEnv | QEnvironment::PersistentEnv );
	    QEnvironment::putEnv( "QMAKESPEC", mkSpecs[ sysID ], QEnvironment::LocalEnv | QEnvironment::PersistentEnv );

	    path.clear();
	    path = QStringList::split( ';', QEnvironment::getEnv( "PATH", QEnvironment::PersistentEnv ) );
//	    if( path.findIndex( qtDir + "\\lib" ) == -1 )
//		path.prepend( qtDir + "\\lib" );
	    if( path.findIndex( qtDir + "\\bin" ) == -1 )
		path.prepend( qtDir + "\\bin" );
	    QEnvironment::putEnv( "PATH", path.join( ";" ), QEnvironment::PersistentEnv );
	}
	else {
	    QEnvironment::putEnv( "QTDIR", qtDir, QEnvironment::LocalEnv );
	    QEnvironment::putEnv( "QMAKESPEC", mkSpecs[ sysID ], QEnvironment::LocalEnv );
	}

	bool enterprise = licenseInfo[ "PRODUCTS" ] == "qt-enterprise";

	configList->clear();
	advancedList->clear();
	configList->setSorting( -1 );
	advancedList->setSorting( -1 );
	QCheckListItem* item;
	QCheckListItem *folder;
	QStringList::Iterator it;
	connect( &configure, SIGNAL( processExited() ), this, SLOT( configDone() ) );
	connect( &configure, SIGNAL( readyReadStdout() ), this, SLOT( readConfigureOutput() ) );
	connect( &configure, SIGNAL( readyReadStderr() ), this, SLOT( readConfigureError() ) );

	// general
	folder = new QCheckListItem ( configList, "Modules" );
	folder->setOpen( TRUE );

	QStringList licensedModules = QStringList::split( " ", "network canvas table xml opengl sql" );
	for( it = licensedModules.begin(); it != licensedModules.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    item->setOn( enterprise );
	    item->setEnabled( enterprise );
	}

	licensedModules = QStringList::split( " ", "styles iconview workspace" );
	for( it = licensedModules.begin(); it != licensedModules.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    item->setOn( true );
	}

	QStringList requiredModules = QStringList::split( " ", "tools kernel widgets dialogs" );
	for( it = requiredModules.begin(); it != requiredModules.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    item->setOn( true );
	    item->setEnabled( false );
	}

	folder = new QCheckListItem ( configList, "Threading" );
	folder->setOpen( TRUE );
	item = new QCheckListItem( folder, "Threaded", QCheckListItem::RadioButton );
	item->setOn( TRUE );
	item = new QCheckListItem( folder, "Non-threaded", QCheckListItem::RadioButton );

	folder = new QCheckListItem ( configList, "Library" );
	folder->setOpen( TRUE );
	item = new QCheckListItem( folder, "Static", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Shared", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	folder = new QCheckListItem ( configList, "Build" );
	folder->setOpen( TRUE );
	item = new QCheckListItem( folder, "Debug", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Release", QCheckListItem::RadioButton );	
	item->setOn( TRUE );

	// Advanced options
	folder = new QCheckListItem ( advancedList, "SQL Drivers" );
	folder->setOpen( true );
	QStringList sqlList = QStringList::split( " ", "mysql oci odbc psql tds" );
	for( it = sqlList.begin(); it != sqlList.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    item->setOn( false );
	    item->setEnabled( enterprise );
	}

	folder = new QCheckListItem( advancedList, "Big Textcodecs" );
	folder->setOpen( true );
	item = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );	
	item->setOn( true );

	folder = new QCheckListItem( advancedList, "Accessibility" );
	folder->setOpen( true );
	item = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
	item->setOn( true );

	QCheckListItem *imfolder = new QCheckListItem( advancedList, "Image Formats" );
	imfolder->setOpen( true );

	folder = new QCheckListItem( imfolder, "MNG" );
	folder->setOpen( true );
	item = new QCheckListItem( folder, "Present", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	item->setOn( true );

	folder = new QCheckListItem( imfolder, "JPEG" );
	folder->setOpen( true );
	item = new QCheckListItem( folder, "Present", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	item->setOn( true );

	folder = new QCheckListItem( imfolder, "PNG" );
	folder->setOpen( true );
	item = new QCheckListItem( folder, "Present", QCheckListItem::RadioButton );
	item = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	item->setOn( true );
	item = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );


	optionSelected( 0 );
	setInstallStep( 5 );
    }
    else if( newPage == progressPage ) {
	saveSettings();
	int totalSize = 0;
	QFileInfo fi;
	totalRead = 0;
	bool copySuccessful = true;

	setInstallStep( 6 );
	if( !filesCopied ) {
	    filesDisplay->append( "Starting copy process\n" );
#if defined (USE_ARCHIVES)
	    fi.setFile( "qt.arq" );
	    if( fi.exists() )
		totalSize = fi.size();

	    if( installDocs->isChecked() ) {
		fi.setFile( "docs.arq" );
		if( fi.exists() ) 
		    totalSize += fi.size();
	    }

	    if( installExamples->isChecked() ) {
		fi.setFile( "examples.arq" );
		if( fi.exists() )
		    totalSize += fi.size();
	    }

	    if( installTutorials->isChecked() ) {
		fi.setFile( "tutorial.arq" );
		if( fi.exists() )
		    totalSize += fi.size();
	    }

	    operationProgress->setTotalSteps( totalSize );

	    readArchive( "qt.arq", installPath->text() );
	    if( installDocs->isChecked() )
		readArchive( "docs.arq", installPath->text() );
	    if( installExamples->isChecked() )
		readArchive( "examples.arq", installPath->text() );
	    if( installTutorials->isChecked() )
		readArchive( "tutorial.arq", installPath->text() );
#else
	    operationProgress->setTotalSteps( FILESTOCOPY );
	    copySuccessful = copyFiles( QDir::currentDirPath(), installPath->text(), true );

	    QFile inFile( installPath->text() + "\\bin\\quninstall.exe" );
	    QFile outFile( shell.windowsFolderName + "\\quninstall.exe" );
	    QFileInfo fi( inFile );
	    QByteArray buffer( fi.size() );

	    if( buffer.size() ) {
		if( inFile.open( IO_ReadOnly ) ) {
		    if( outFile.open( IO_WriteOnly ) ) {
			inFile.readBlock( buffer.data(), buffer.size() );
			outFile.writeBlock( buffer.data(), buffer.size() );
			outFile.close();
		    }
		    inFile.close();
		}
	    }

/*These lines are only to be used when changing the filecount estimate

	    QString tmp( "%1" );
	    tmp = tmp.arg( totalFiles );
	    QMessageBox::information( this, tmp, tmp );
*/
	    operationProgress->setProgress( FILESTOCOPY );

#endif
	    createDir( installPath->text() + "\\plugins\\designer" );
	    filesCopied = copySuccessful;

	    timeCounter = 30;
	    autoContTimer.start( 1000 );
	    if( copySuccessful )
		logFiles( "All files have been copied,\nThis log has been saved to the installation directory.\nThe build will start automatically in 30 seconds", true );
	    else
		logFiles( "One or more errors occurred during file copying,\nplease review the log and try to amend the situation.\n", true );
	}
	setNextEnabled( progressPage, copySuccessful );
    }
    else if( newPage == buildPage ) {
	QStringList args;
	QStringList entries;
	QSettings settings;
	QString entry;
	QStringList::Iterator it;
	QFile tmpFile;
	QTextStream tmpStream;
	bool settingsOK;

	autoContTimer.stop();
	nextButton()->setText( "Next >" );
	saveSettings();
	if( int( qWinVersion() ) & int( WV_NT_based ) ) {
	    outputDisplay->append( "Execute configure...\n" );

	    args << QEnvironment::getEnv( "QTDIR" ) + "\\bin\\configure.exe";
	    entry = settings.readEntry( "/Trolltech/Qt/Build", "Debug", &settingsOK );
	    if ( entry == "Debug" )
		args += "-debug";
	    else
		args += "-release";

	    entry = settings.readEntry( "/Trolltech/Qt/Library", "Shared", &settingsOK );
	    if ( entry == "Static" )
		args += "-static";
	    else
		args += "-shared";

	    entry = settings.readEntry( "/Trolltech/Qt/Threading", QString::null, &settingsOK );
	    if ( entry == "Threaded" )
		args += "-thread";
	    else
		args += "-no-thread";

	    entries = settings.readListEntry( "/Trolltech/Qt/Modules", ',', &settingsOK );
	    for( it = entries.begin(); it != entries.end(); ++it ) {
		entry = *it;
		args += QString( "-enable-" ) + entry;
	    }

	    entries = settings.readListEntry( "/Trolltech/Qt/SQL Drivers", ',', &settingsOK );
	    for( it = entries.begin(); it != entries.end(); ++it ) {
		entry = *it;
		args += QString( "-sql-" ) + entry;
	    }

	    entry = settings.readEntry( "/Trolltech/Qt/Accessibility", "On", &settingsOK );
	    if ( entry == "On" )
		args += "-accessibility";
	    else
		args += "-no-accessibility";

	    entry = settings.readEntry( "/Trolltech/Qt/Big Textcodecs", "On", &settingsOK );
	    if ( entry == "On" )
		args += "-big-codecs";
	    else
		args += "-no-big-codecs";

	    entry = settings.readEntry( "/Trolltech/Qt/PNG", "Direct", &settingsOK );
	    if ( entry == "Plugin" )
		args += "-no-png";
	    else if ( entry == "Direct" )
		args += "-qt-png";
	    else if ( entry == "Present" )
		args += "-system-png";

	    entry = settings.readEntry( "/Trolltech/Qt/JPEG", "Direct", &settingsOK );
	    if ( entry == "Plugin" )
		args += "-no-jpeg";
	    else if ( entry == "Direct" )
		args += "-qt-jpeg";
	    else if ( entry == "Present" )
		args += "-system-jpeg";

	    entry = settings.readEntry( "/Trolltech/Qt/MNG", "Direct", &settingsOK );
	    if ( entry == "Plugin" )
		args += "-no-mng";
	    else if ( entry == "Direct" )
		args += "-qt-mng";
	    else if ( entry == "Present" )
		args += "-system-mng";

// When we change our minds again, uncomment this line....
//	    args += "-no-qmake";

	    outputDisplay->append( args.join( " " ) + "\n" );
	    configure.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	    configure.setArguments( args );

	    // Start the configure process
	    compileProgress->setTotalSteps( int(double(filesToCompile) * 2.6) );
	    if( !configure.start() )
		logOutput( "Could not start configure process" );
	}
	else {
	    QFile outFile( installPath->text() + "\\build.bat" );
	    QTextStream outStream( &outFile );

	    if( outFile.open( IO_WriteOnly | IO_Translate ) ) {
		outStream << "cd " << QEnvironment::getEnv( "QTDIR" ).latin1() << endl;
		outStream << QEnvironment::getEnv( "QTDIR" ).latin1() << "\\bin\\configure.exe ";
		entry = settings.readEntry( "/Trolltech/Qt/Build", "Debug", &settingsOK );
		if ( entry == "Debug" )
		    outStream << "-debug ";
		else
		    outStream << "-release ";

		entry = settings.readEntry( "/Trolltech/Qt/Library", "Shared", &settingsOK );
		if ( entry == "Static" )
		    outStream << "-static ";
		else
		    outStream << "-shared ";

		entry = settings.readEntry( "/Trolltech/Qt/Threading", QString::null, &settingsOK );
		if ( entry == "Threaded" )
		    outStream << "-thread ";

		entries = settings.readListEntry( "/Trolltech/Qt/Modules", ',', &settingsOK );
		for( it = entries.begin(); it != entries.end(); ++it ) {
		    entry = *it;
		    outStream << "-enable-" << entry.latin1() << " ";
		}

		entries = settings.readListEntry( "/Trolltech/Qt/SQL Drivers", ',', &settingsOK );
		for( it = entries.begin(); it != entries.end(); ++it ) {
		    entry = *it;
		    outStream << "-sql-" << entry.latin1() << " ";
		}
		outStream << "-no-qmake" << endl;
	        QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
		outStream << makeCmds[ sysID ].latin1() << endl;

		outFile.close();
	    }
	    doFinalIntegration();
	    showPage( finishPage );
	}
	setInstallStep( 7 );
    }
    else if( newPage == finishPage ) {
	autoContTimer.stop();
	nextButton()->setText( "Next >" );
	QString finishMsg;
	if( int( qWinVersion() ) & int( WV_NT_based ) ) {
	    finishMsg = QString( "Qt has been installed to " ) + installPath->text() + " and is ready to use.";
	}
	else {
	    finishMsg = QString( "The Qt files have installed to " ) + installPath->text() + " and is ready to be compiled.\n";
	    if( persistentEnv ) {
		finishMsg += "The environment variables needed to use Qt have been recorded into your AUTOEXEC.BAT file.\n";
		finishMsg += "Please review this file, and take action as appropriate depending on your operating system to get them into the persistent environment. (Windows Me users, run MsConfig)\n\n";
	    }
	    finishMsg += QString( "To build Qt, please double-click to \"Build Qt " ) + QString( QT_VERSION_STR ) + "\" icon which has been installed into your Start-Menu.";
	}
	finishText->setText( finishMsg );
	setInstallStep( 8 );
    }
}

void SetupWizardImpl::optionClicked( QListViewItem *i )
{
    if ( !i || i->rtti() != QCheckListItem::RTTI )
	return;

    QCheckListItem *item = (QCheckListItem*)i;
    if ( item->type() != QCheckListItem::RadioButton )
	return;

    if ( item->text(0) == "Static" && item->isOn() ) {
	if ( !QMessageBox::information( this, "Are you sure?", "It will not be possible to build components "
				  "or plugins if you select the static build of the Qt library.\n"
				  "new features, e.g souce code editing in Qt Designer, will not "
				  "be available, "
				  "\nand you or users of your software might not be able "
				  "to use all or new features, e.g. new styles.\n\n"
				  "Are you sure you want to build a static Qt library?",
				  "No, I want to use the cool new stuff", "Yes" ) ) {
	    item->setOn( FALSE );
	    if ( ( item = (QCheckListItem*)configList->findItem( "Shared", 0, 0 ) ) )
		item->setOn( TRUE );
	}
	return;
    }
}

void SetupWizardImpl::optionSelected( QListViewItem *i )
{
    if ( !i ) {
	explainOption->setText( tr("Change the configuration.") );
	return;
    }

    if ( i->rtti() != QCheckListItem::RTTI )
	return;

    if ( i->text(0) == "Required" ) {
	explainOption->setText( tr("These modules are a necessary part of the Qt library. "
				   "They can not be disabled.") );
    } else if ( i->parent() && i->parent()->text(0) == "Required" ) {
	explainOption->setText( tr("This module is a necessary part of the Qt library. "
				   "It can not be disabled.") ); 
    } else if ( i->text(0) == "Modules" ) {
	explainOption->setText( tr("Some of these modules are optional. "
				   "You can deselect the modules that you "
				   "don't require for your development.\n"
				   "By default, all modules are selected.") );
    } else if ( i->parent() && i->parent()->text(0) == "Modules" ) {
	QString moduleText;
	// ### have some explanation of the module here
	explainOption->setText( moduleText );
    } else if ( i->text(0) == "Threading" ) {
	explainOption->setText( tr("Build the Qt library with or without thread support. "
			    "By default, threading is supported.") );
    } else if ( i->parent() && i->parent()->text(0) == "Threading" ) {
	if ( i->text(0) == "Threaded" ) {
	    explainOption->setText("Select this option if you want to be able to use threads "
				   "in your application.");
	} else {
	    explainOption->setText("Select this option if you do not need thread support.\n"
				   "Some classes will not be available without thread support.");
	}
    } else if ( i->text(0) == "Build" || i->parent() && i->parent()->text(0) == "Build" ) {
	explainOption->setText( tr("<p>Use the debug build of the Qt library to enhance "
				   "debugging of your application. The release build "
				   "is both smaller and faster.</p>") );
    } else if ( i->text(0) == "Library" ) {
	explainOption->setText( "Build a static or a shared Qt library." );
    } else if ( i->parent() && i->parent()->text( 0 ) == "Library" ) {
	if ( i->text(0) == "Static" ) {
	    explainOption->setText( tr("<p>Build the Qt library as a static library."
				       "All applications created with a static"
				       "library will be at least 1.5MB big.</p>"
				       "<p><font color=\"red\">It is not possible to "
				       "build or use any components or plugins with a "
				       "static Qt library!</font></p>") );
	} else {
	    explainOption->setText("<p>A shared Qt library makes it necessary to "
				   "distribute the Qt DLL together with your software.</p>"
				   "<p>Applications and libraries linked against a shared Qt library "
				   "can be small and can make use of components and plugins.</p>" );
	}
    } else if ( i->text( 0 ) == "SQL Drivers" ) {
	explainOption->setText("");
    } else if ( i->parent() && i->parent()->text( 0 )== "SQL Drivers" ) {
	//### have some explanation of the SQL Drivers here...
	explainOption->setText("");
    }
}

bool SetupWizardImpl::createDir( const QString& fullPath )
{
    QStringList hierarchy = QStringList::split( QString( "\\" ), fullPath );
    QString pathComponent, tmpPath;
    QDir dirTmp;
    bool success;

    for( QStringList::Iterator it = hierarchy.begin(); it != hierarchy.end(); ++it ) {
	pathComponent = *it + "\\";
	tmpPath += pathComponent;
	success = dirTmp.mkdir( tmpPath );
    }
    return success;
}

void SetupWizardImpl::logFiles( const QString& entry, bool close )
{
    if( !fileLog.isOpen() ) {
	fileLog.setName( installPath->text() + "\\install.log" );
	if( !fileLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    QTextStream outstream( &fileLog );

    filesDisplay->append( entry + "\n" );
//    filesDisplay->setText( filesDisplay->text() + entry );
    outstream << entry + "\n";

    if( close )
	fileLog.close();
}

void SetupWizardImpl::logOutput( const QString& entry, bool close )
{
    QTextStream outstream;
    if( !outputLog.isOpen() ) {
	outputLog.setName( installPath->text() + "\\buildlog.txt" );
	if( !outputLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    outstream.setDevice( &outputLog );

    outputDisplay->append( entry + "\n" );
    outstream << entry + "\n";

    if( close )
	outputLog.close();
}

#if defined (USE_ARCHIVES)
void SetupWizardImpl::readArchive( const QString& arcname, const QString& installPath )
{
    QDataStream inStream, outStream;
    QFile inFile, outFile;
    QDir outDir;
    QByteArray inBuffer;
    QByteArray outBuffer( BUFFERSIZE );
    z_stream ztream;
    int totalOut;
    bool continueDeCompressing;
    QString entryName, dirName;
    int entryLength;

    inFile.setName( arcname );
    // Set up the initial directory.
    // If the dir does not exist, try to create it
    dirName = QDir::convertSeparators( installPath );
    outDir.setPath( dirName );
    if( outDir.exists( dirName ) )
	outDir.cd( dirName );
    else {
	if( createDir( dirName ) )
	    outDir.cd( dirName );
	else
	    return;
    }

    if( inFile.open( IO_ReadOnly ) ) {
	inStream.setDevice( &inFile );
	while( !inStream.atEnd() ) {
	    inStream >> entryLength;
	    totalRead += sizeof( entryLength );
	    inBuffer.resize( entryLength );
	    inStream.readRawBytes( inBuffer.data(), entryLength );
	    totalRead += entryLength;
	    entryName = inBuffer.data();
	    if( entryName.right( 1 ) == "\\" ) {
		if( entryName == "..\\" )
		    outDir.cdUp();
		else {
		    dirName = QDir::convertSeparators( outDir.absPath() + QString( "\\" ) + entryName.left( entryName.length() - 1 ) );
		    if( outDir.exists( dirName ) )
			outDir.cd( dirName );
		    else {
			if( createDir( dirName ) )
			    outDir.cd( dirName );
			else
			    break;
		    }
		    if( qApp ) {
			qApp->processEvents();
			operationProgress->setProgress( totalRead );
			logFiles( dirName + "\\" );
		    }
		}
	    }
	    else
	    {
		QDateTime timeStamp;
		QString fileName = QDir::convertSeparators( outDir.absPath() + QString( "\\" ) + entryName );
		totalOut = 0;
		outFile.setName( fileName );
		
		if( outFile.open( IO_WriteOnly ) ) {

		    // Try to count the files to get some sort of idea of compilation progress
		    if( entryName.right( 4 ) == ".cpp" || 
			entryName.right( 2 ) == ".c" ||
			entryName.right( 4 ) == ".pro" ||
			entryName.right( 3 ) == ".ui" )
			filesToCompile++;

		    // Get timestamp from the archive
		    inStream >> timeStamp;
//		    qDebug( "%s", timeStamp.toString().latin1() );
		    outStream.setDevice( &outFile );
		    inStream >> entryLength;
		    if( qApp ) {
			qApp->processEvents();
			operationProgress->setProgress( totalRead );
			logFiles( fileName );
//			logFiles( QString( fileName + " (%1 bytes)" ).arg( entryLength )  );
		    }
		    totalRead += sizeof( entryLength ) + 8; // Use size 8 bytes for timeStamp
		    inBuffer.resize( entryLength );
		    inStream.readRawBytes( inBuffer.data(), entryLength );
		    totalRead += entryLength;
		    ztream.next_in = (unsigned char*)inBuffer.data();
		    ztream.avail_in = entryLength;
		    ztream.total_in = 0;
		    ztream.msg = NULL;
		    ztream.zalloc = (alloc_func)0;
		    ztream.zfree = (free_func)0;
		    ztream.opaque = (voidpf)0;
		    ztream.data_type = Z_BINARY;
		    inflateInit( &ztream );
		    continueDeCompressing = true;
		    while( continueDeCompressing ) {
			ztream.next_out = (unsigned char*)outBuffer.data();
			ztream.avail_out = outBuffer.size();
			ztream.total_out = 0;
			continueDeCompressing = ( inflate( &ztream, Z_NO_FLUSH ) == Z_OK );
			outStream.writeRawBytes( outBuffer.data(), ztream.total_out );
			totalOut += ztream.total_out;
		    }
		    inflateEnd( &ztream );
		    outFile.close();

		}
	    }
	}

	inFile.close();
    }
}
#else
bool SetupWizardImpl::copyFiles( const QString& sourcePath, const QString& destPath, bool topLevel )
{
    QDir dir( sourcePath );
    const QFileInfoList* list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo* fi;
    bool doCopy;

    while( ( fi = it.current() ) ) {
	if( fi->fileName()[ 0 ] != '.' ) {
	    QString entryName = sourcePath + QDir::separator() + fi->fileName();
	    QString targetName = destPath + QDir::separator() + fi->fileName();
	    doCopy = true;
	    if( fi->isDir() ) {
		if( !dir.exists( targetName ) )
		    createDir( targetName );
		if( topLevel ) {
		    if( fi->fileName() == "doc" )
			doCopy = installDocs->isChecked();
		    else if( fi->fileName() == "tutorial" )
			doCopy = installTutorials->isChecked();
		    else if( fi->fileName() == "examples" )
			doCopy = installExamples->isChecked();
		}
		if( doCopy )
		    if( !copyFiles( entryName, targetName ) )
			return false;
	    } else {
		if( qApp && !isHidden() ) {
		    qApp->processEvents();
		    operationProgress->setProgress( totalFiles );
		    logFiles( targetName );
		} else {
		    return FALSE;
		}
		if( entryName.right( 4 ) == ".cpp" || 
		    entryName.right( 2 ) == ".c" ||
		    entryName.right( 4 ) == ".pro" ||
		    entryName.right( 3 ) == ".ui" )
		    filesToCompile++;
		bool res = TRUE;
		if ( !QFile::exists( targetName ) )
		    res = CopyFileA( entryName.local8Bit(), targetName.local8Bit(), FALSE );

		if ( res ) {
		    totalFiles++;
		    HANDLE inFile, outFile;
		    if( inFile = ::CreateFileA( entryName.latin1(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ){
			if( outFile = ::CreateFileA( targetName.latin1(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ){
			    FILETIME createTime, accessTime, writeTime;
			    ::GetFileTime( inFile, &createTime, &accessTime, &writeTime );
			    ::SetFileTime( outFile, &createTime, &accessTime, &writeTime );
			    ::CloseHandle( outFile );
			}
			::CloseHandle( inFile );
		    }
		} else {
		    QString error = QEnvironment::getLastError();
		    logFiles( QString( "   ERROR: " ) + error + "\n" );
		    if( QMessageBox::warning( this, "File copy error", entryName + ": " + error, "Continue", "Cancel", QString::null, 0 ) )
			return false;
		}
	    }
	}
	++it;
    }
    return true;
}
#endif

void SetupWizardImpl::setInstallStep( int step )
{
    setCaption( QString( "Qt Installation Wizard - Step %1 of 8" ).arg( step ) );
}

void SetupWizardImpl::timerFired()
{
    QString tmp( "Next %1 >" );

    timeCounter--;

    if( timeCounter )
	nextButton()->setText( tmp.arg( timeCounter ) );
    else {
	next();
	autoContTimer.stop();
    }
}

void SetupWizardImpl::readLicense( QString filePath)
{
    QFile licenseFile( filePath );

    if( licenseFile.open( IO_ReadOnly ) ) {
	QString buffer;

	while( licenseFile.readLine( buffer, 1024 ) != -1 ) {
	    if( buffer[ 0 ] != '#' ) {
		QStringList components = QStringList::split( '=', buffer );
		QStringList::Iterator it = components.begin();
		QString key = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null ).upper();
		QString value = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null );

		licenseInfo[ key ] = value;
	    }
	}
	licenseFile.close();

	customerID->setText( licenseInfo[ "CUSTOMERID" ] );
	licenseID->setText( licenseInfo[ "LICENSEID" ] );
	licenseeName->setText( licenseInfo[ "LICENSEE" ] );
	if( licenseInfo[ "PRODUCTS" ] == "qt-enterprise" )
	    productsString->setCurrentItem( 1 );
	else
	    productsString->setCurrentItem( 0 );
	expiryDate->setText( licenseInfo[ "EXPIRYDATE" ] );
    }
}

void SetupWizardImpl::writeLicense( QString filePath )
{
    QFile licenseFile( filePath );

    if( licenseFile.open( IO_WriteOnly | IO_Translate ) ) {
	QTextStream licStream( &licenseFile );
	
	licenseInfo[ "CUSTOMERID" ] = customerID->text();
	licenseInfo[ "LICENSEID" ] = licenseID->text();
	licenseInfo[ "LICENSEE" ] = licenseeName->text();
	if( productsString->currentItem() == 0 )
	    licenseInfo[ "PRODUCTS" ] = "qt-professional";
	else
	    licenseInfo[ "PRODUCTS" ] = "qt-enterprise";

	licenseInfo[ "EXPIRYDATE" ] = expiryDate->text();

	licStream << "# Toolkit license file" << endl;
	licStream << "CustomerID=\"" << licenseInfo[ "CUSTOMERID" ].latin1() << "\"" << endl;
	licStream << "LicenseID=\"" << licenseInfo[ "LICENSEID" ].latin1() << "\"" << endl;
	licStream << "Licensee=\"" << licenseInfo[ "LICENSEE" ].latin1() << "\"" << endl;
	licStream << "Products=\"" << licenseInfo[ "PRODUCTS" ].latin1() << "\"" << endl;
	licStream << "ExpiryDate=" << licenseInfo[ "EXPIRYDATE" ].latin1() << endl;

	licenseFile.close();
    }
}

void SetupWizardImpl::clickedLicenseFile()
{
    QString licensePath = QFileDialog::getOpenFileName( installPath->text(), QString::null, this, NULL, "Browse for license file" );

    if( !licensePath.isEmpty() )
	readLicense( licensePath );

}
