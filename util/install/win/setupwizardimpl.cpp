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
#include <qarchive.h>
#include <qvalidator.h>

#define FILESTOCOPY 4582

static bool createDir( const QString& fullPath )
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

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f, bool reconfig ) :
    SetupWizard( pParent, pName, modal, f ),
    filesCopied( false ),
    filesToCompile( 0 ),
    filesCompiled( 0 ),
    sysID( 0 ),
    tmpPath( QEnvironment::getTempPath() ),
    reconfigMode( reconfig )
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
    installPath->setText( QString( "C:\\Qt\\" ) + QString(QT_VERSION_STR).replace( QRegExp("-"), "" ) );
    sysGroup->setButton( 0 );
    // Folderspage
    QByteArray buffer( 256 );
    unsigned long buffSize( buffer.size() );
    GetUserNameA( buffer.data(), &buffSize );
    folderGroups->insertItem( "Anyone who uses this computer (all users)" );
    folderGroups->insertItem( QString( "Only for me (" ) + QString( buffer.data() ) + ")" );
    folderPath->setText( QString( "Qt " ) + QT_VERSION_STR );
    if( qWinVersion() & Qt::WV_NT_based )   // On NT we also have a common folder
	folderGroups->setEnabled( true );
    else
	folderGroups->setDisabled( true );

    if( reconfig ) {
	removePage( introPage );
	removePage( licensePage );
	removePage( foldersPage );
	removePage( optionsPage );
	removePage( progressPage );
	setTitle( configPage, "Reconfigure Qt" );
    }
    licenseID->setValidator( new QIntValidator( 100000, -1, licenseID ) );
    readLicense( QDir::homeDirPath() + "/.qt-license" );
}

void SetupWizardImpl::stopProcesses()
{
    if( configure.isRunning() )
	configure.kill();
    if( make.isRunning() )
	make.kill();
    if( integrator.isRunning() )
	integrator.kill();
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
    folderPath->setText( shell.selectFolder( folderPath->text(), ( folderGroups->currentItem() == 0 ) ) );
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
    compileProgress->setProgress( compileProgress->totalSteps() );
    QString dirName, examplesName, tutorialsName;
    bool common( folderGroups->currentItem() == 0 );
    QString qtDir = QEnvironment::getEnv( "QTDIR" );

    switch( sysID ) {
    case MSVC:
	{
	    QFile autoexp( devSysPath->text() + "\\Common\\MsDev98\\bin\\autoexp.dat" );
	    if ( !autoexp.exists() ) {
		autoexp.open( IO_WriteOnly );
	    } else {
		autoexp.open( IO_ReadOnly );
		QString existingUserType = autoexp.readAll();
		autoexp.close();
		if ( existingUserType.find( "; Trolltech Qt" ) == -1 )
		    autoexp.open( IO_WriteOnly | IO_Append );
	    }

	    if( autoexp.isOpen() ) { // First try to open the file to search for existing installations
		QTextStream outstream( &autoexp );
		outstream << "; Trolltech Qt" << endl;
		outstream << "QString=<d->unicode,su> len=<d->len,u>" << endl;
		outstream << "QCString =<shd->data, s>" << endl;
		outstream << "QPoint =x=<xp> y=<yp>" << endl;
		outstream << "QRect =x1=<x1> y1=<y1> x2=<x2> y2=<y2>" << endl;
		outstream << "QSize =width=<wd> height=<ht>" << endl;
		outstream << "QWMatrix =m11=<_m11> m12=<_m12> m21=<_m21> m22=<_m22> dx=<_dx> dy=<_dy>" << endl;
		outstream << "QVariant =Type=<d->typ> value=<d->value>" << endl;
		outstream << "QValueList<*> =Count=<sh->nodes>" << endl;
		outstream << "QPtrList<*> =Count=<numNodes>" << endl;
		outstream << "QGuardedPtr<*> =ptr=<priv->obj>" << endl;
		outstream << "QEvent =type=<t>" << endl;
		outstream << "QObject =class=<metaObj->classname,s> name=<objname,s>" << endl;
		autoexp.close();
	    }

	    QFile usertype( devSysPath->text() + "\\Common\\MsDev98\\bin\\usertype.dat" );
	    if ( !usertype.exists() ) {
		usertype.open( IO_WriteOnly );
	    } else {
		usertype.open( IO_ReadOnly );
		QString existingUserType = usertype.readAll();
		usertype.close();
		if ( existingUserType.find( "Q_OBJECT" ) == -1 )
		    usertype.open( IO_WriteOnly | IO_Append );
	    }
	    if ( usertype.isOpen() ) {
		QTextStream outstream( &usertype );
		outstream << "Q_OBJECT" << endl;
		outstream << "Q_PROPERTY" << endl;
		outstream << "Q_ENUMS" << endl;
		outstream << "emit" << endl;
		outstream << "TRUE" << endl;
		outstream << "FALSE" << endl;
		outstream << "SIGNAL" << endl;
		outstream << "SLOT" << endl;
		outstream << "signals:" << endl;
		outstream << "slots:" << endl;
		usertype.close();
	    }
	}
	break;
    }
    /*
    ** Set up our icon folder and populate it with shortcuts.
    ** Then move to the next page.
    */
    dirName = shell.createFolder( folderPath->text(), common );
    shell.createShortcut( dirName, common, "Designer", qtDir + "\\bin\\designer.exe", "GUI designer", "", qtDir );
    shell.createShortcut( dirName, common, "Reconfigure Qt", qtDir + "\\bin\\install.exe", "Reconfigure the Qt library", "-reconfig", qtDir );
    shell.createShortcut( dirName, common, "License agreement", "notepad.exe", "Review the license agreement", QString( "\"" ) + qtDir + "\\LICENSE\"" );
    shell.createShortcut( dirName, common, "On-line documentation", qtDir + "\\bin\\assistant.exe", "Browse the On-line documentation", "", qtDir );
    shell.createShortcut( dirName, common, "Linguist", qtDir + "\\bin\\linguist.exe", "Qt translation utility", "", qtDir );
    if( qWinVersion() & WV_DOS_based )
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
    uninstaller << ( shell.windowsFolderName + "\\quninstall.exe" );
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
    compileProgress->setTotalSteps( compileProgress->totalSteps() );
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

    if( reconfigMode )
	showPage( finishPage );

    if( !make.normalExit() || ( make.normalExit() && make.exitStatus() ) ) {
	logOutput( "The build process failed!\n" );
	QMessageBox::critical( this, "Error", "The build process failed!" );
	setBackEnabled( buildPage, true );
//	removePage( progressPage );
	setAppropriate( progressPage, false );
    } else {
	compileProgress->setProgress( compileProgress->totalSteps() );

	if( ( sysID != MSVC ) || ( !findFileInPaths( "atlbase.h", QStringList::split( ";", QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
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

    if( reconfigMode && !rebuildInstallation->isChecked() )
	showPage( finishPage );

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
	    backButton()->setEnabled( TRUE );
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
	    if ( item->isOn() ) {
		QString folder;
		QListViewItem *pItem = item;
		while ( (pItem = pItem->parent() ) ) {
		    if ( folder.isEmpty() )
			folder = pItem->text( 0 );
		    else
			folder = pItem->text(0) + "/" + folder;
		}

		settings.writeEntry( "/Trolltech/Qt/" + folder, item->text() );
	    }
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
    } else if( newPage == optionsPage ) {
	setInstallStep( 2 );
    } else if( newPage == licensePage ) {
	QStringList makeCmds = QStringList::split( ' ', "nmake.exe make.exe gmake.exe" );
	QStringList paths = QStringList::split( QRegExp("[;,]"), QEnvironment::getEnv( "PATH" ) );
	if( !findFileInPaths( makeCmds[ sysID ], paths ) ) {
	    setNextEnabled( licensePage, false );
	    QMessageBox::critical( this, "Environment problems", "The installation program can't find the make command '" + makeCmds[ sysID ] + "'.\nMake sure the path to it "
					 "is present in the PATH environment variable.\n"
					 "The installation can't continue." );
	} else {
	    setNextEnabled( licensePage, true );
	}
	setInstallStep( 3 );
    } else if( newPage == foldersPage ) {
	QStringList devSys = QStringList::split( ';',"Microsoft Visual Studio path;Borland C++ Builder path;GNU C++ path" );

	devSysLabel->setText( devSys[ sysID ] );
	devSysPath->setEnabled( sysID == 0 );
	devSysPathButton->setEnabled( sysID == 0 );
	qtDirCheck->setChecked( ( QEnvironment::getEnv( "QTDIR" ).length() == 0 ) );
	if( sysID == 0 )
	    devSysPath->setText( QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual Studio", "ProductDir", QEnvironment::LocalMachine ) );
	setInstallStep( 4 );
    } else if( newPage == configPage ) {
	// First make sure that the current license information is saved
	writeLicense( QDir::homeDirPath() + "/.qt-license" );

	QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++" );
	QByteArray pathBuffer;
	QStringList path;
	QString qtDir;
	int envSpec = QEnvironment::LocalEnv;

	if( reconfigMode ) {
	    qtDir = QEnvironment::getEnv( "QTDIR" );
	    currentInstLabel->show();
	    currentInstallation->show();
	    rebuildInstallation->show();
	    currentInstallation->setText( qtDir );
	}
	else {
	    qtDir = QDir::convertSeparators( QEnvironment::getFSFileName( installPath->text() ) );
	    currentInstLabel->hide();
	    currentInstallation->hide();
	    rebuildInstallation->hide();
	}

	path = QStringList::split( ';', QEnvironment::getEnv( "PATH" ) );
	if( path.findIndex( qtDir + "\\bin" ) == -1 ) {
	    path.prepend( qtDir + "\\bin" );
	    QEnvironment::putEnv( "PATH", path.join( ";" ) );
	}

	if( qtDirCheck->isChecked() ) {
	    envSpec |= QEnvironment::PersistentEnv;
/*
	    if( folderGroups->currentItem() == 0 )
		envSpec |= QEnvironment::GlobalEnv;
*/
	    path.clear();
	    path = QStringList::split( ';', QEnvironment::getEnv( "PATH", envSpec ) );
	    if( path.findIndex( qtDir + "\\bin" ) == -1 ) {
		path.prepend( qtDir + "\\bin" );
		QEnvironment::putEnv( "PATH", path.join( ";" ), envSpec );
	    }
	}

	QEnvironment::putEnv( "QTDIR", qtDir, envSpec );
	QEnvironment::putEnv( "QMAKESPEC", mkSpecs[ sysID ], envSpec );

	if( sysID == 0 ) {
	    QString devdir = QEnvironment::getEnv( "MSDevDir" );
	    if( !devdir.length() ) {
		QString vsCommonDir, msDevDir, msVCDir, osDir;

		if( QMessageBox::warning( this, "Environment", "The Visual C++ environment variables has not been set\nDo you want to do this now?", "Yes", "No", QString::null, 0, 1 ) == 0 ) {
		    envSpec |= QEnvironment::PersistentEnv;
/*
		    if( folderGroups->currentItem() == 0 )
			envSpec |= QEnvironment::GlobalEnv;
*/
		    persistentEnv = true;
		}

		vsCommonDir = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup", "VsCommonDir", QEnvironment::LocalMachine );
		    msDevDir = QEnvironment::getFSFileName( vsCommonDir + "\\MSDev98" );
		QEnvironment::putEnv( "MSDevDir", msDevDir, envSpec );
		    msVCDir = QEnvironment::getFSFileName( QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++", "ProductDir", QEnvironment::LocalMachine ) );
		QEnvironment::putEnv( "MSVCDir", msVCDir, envSpec );
		if( qWinVersion() & WV_NT_based )
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

	bool enterprise = licenseInfo[ "PRODUCTS" ] == "qt-enterprise";

	if( configList->childCount() ) {
	    QListViewItem* current = configList->firstChild();

	    while( current ) {
		QListViewItem* next = current->nextSibling();
		delete current;
		current = next;
	    }

	    current = advancedList->firstChild();
	    while( current ) {
		QListViewItem* next = current->nextSibling();
		delete current;
		current = next;
	    }
	}
	QSettings settings;
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
	folder->setOpen( true );

	bool settingsOK;
	QStringList entries = settings.readListEntry( "/Trolltech/Qt/Modules", ',', &settingsOK );
	QStringList licensedModules = QStringList::split( " ", "network canvas table xml opengl sql" );
	for( it = licensedModules.begin(); it != licensedModules.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    bool on = entries.isEmpty() || entries.find( *it ) != entries.end();
	    item->setOn( enterprise && on );
	    item->setEnabled( enterprise );
	    if ( enterprise )
		allModules << *it;
	}

	licensedModules = QStringList::split( " ", "iconview workspace" );
	for( it = licensedModules.begin(); it != licensedModules.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    bool on = entries.isEmpty() || entries.find( *it ) != entries.end();
	    item->setOn( on );
	    allModules << *it;
	}

	QStringList requiredModules = QStringList::split( " ", "styles dialogs widgets tools kernel" );
	for( it = requiredModules.begin(); it != requiredModules.end(); ++it ) {
	    item = new QCheckListItem( folder, (*it), QCheckListItem::CheckBox );
	    bool on = entries.isEmpty() || entries.find( *it ) != entries.end();
	    item->setOn( on );
	    item->setEnabled( false );
	    allModules << *it;
	}

	
	folder = new QCheckListItem ( configList, "Threading" );
	folder->setOpen( true );
	QString entry = settings.readEntry( "/Trolltech/Qt/Threading", "Threaded", &settingsOK );
	item = new QCheckListItem( folder, "Threaded", QCheckListItem::RadioButton );
	item->setOn( entry == "Threaded" );
	item = new QCheckListItem( folder, "Non-threaded", QCheckListItem::RadioButton );
	item->setOn( entry == "Non-threaded" );

	folder = new QCheckListItem ( configList, "Library" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Library", "Shared", &settingsOK );
	item = new QCheckListItem( folder, "Static", QCheckListItem::RadioButton );
	item->setOn( entry == "Static" );
	item = new QCheckListItem( folder, "Shared", QCheckListItem::RadioButton );
	item->setOn( entry == "Shared" );

	folder = new QCheckListItem ( configList, "Build" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Build", "Release", &settingsOK );
	item = new QCheckListItem( folder, "Debug", QCheckListItem::RadioButton );
	item->setOn( entry == "Debug" );
	item = new QCheckListItem( folder, "Release", QCheckListItem::RadioButton );	
	item->setOn( entry == "Release" );

	// Advanced options
	QCheckListItem *imfolder = new QCheckListItem( advancedList, "Image Formats" );
	imfolder->setOpen( true );

	folder = new QCheckListItem( imfolder, "MNG" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/MNG", "Plugin", &settingsOK );
#if 0
	// ### disable using system MNG for now -- please someone take a closer look
	entryPresent = settings.readEntry( "/Trolltech/Qt/Image Formats/MNG Present", "No", &settingsOK );
	mngPresent = new QCheckListItem( folder, "Present", QCheckListItem::CheckBox );
	mngPresent->setOn( entry == "Yes" );
#endif
	mngOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	mngOff->setOn( entry == "Off" );
	mngPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	mngPlugin->setOn( entry == "Plugin" );
	mngDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	mngDirect->setOn( entry == "Direct" );

	folder = new QCheckListItem( imfolder, "JPEG" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/JPEG", "Plugin", &settingsOK );
#if 0
	// ### disable using system JPEG for now -- please someone take a closer look
	entryPresent = settings.readEntry( "/Trolltech/Qt/Image Formats/JPEG Present", "No", &settingsOK );
	jpegPresent = new QCheckListItem( folder, "Present", QCheckListItem::CheckBox );
	jpegPresent->setOn( entry == "Yes" );
#endif
	jpegOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	jpegOff->setOn( entry == "Off" );
	jpegPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	jpegPlugin->setOn( entry == "Plugin" );
	jpegDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );	    
	jpegDirect->setOn( entry == "Direct" );

	folder = new QCheckListItem( imfolder, "PNG" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/PNG", "Direct", &settingsOK );
#if 0
	// ### disable using system PNG for now -- please someone take a closer look
	entryPresent = settings.readEntry( "/Trolltech/Qt/Image Formats/PNG Present", "No", &settingsOK );
	pngPresent = new QCheckListItem( folder, "Present", QCheckListItem::CheckBox );
	pngPresent->setOn( entry == "Yes" );
#endif
	pngOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	pngOff->setOn( entry == "Off" );
	pngPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	pngPlugin->setOn( entry == "Plugin" );
	pngDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );	    
	pngDirect->setOn( entry == "Direct" );

	QCheckListItem *sqlfolder = new QCheckListItem( advancedList, "Sql Drivers" );
	sqlfolder->setOpen( true );

	folder = new QCheckListItem( sqlfolder, "TDS" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Sql Drivers/TDS", "Off", &settingsOK );
	tdsOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton ); 
	tdsOff->setOn( entry == "Off" );
	tdsOff->setEnabled( enterprise );
	tdsPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	tdsPlugin->setOn( entry == "Plugin" );
	tdsPlugin->setEnabled( enterprise );
	tdsDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	tdsDirect->setOn( entry == "Direct" );
	tdsDirect->setEnabled( enterprise );
	if ( !enterprise )
	    tdsOff->setOn( true );

	folder = new QCheckListItem( sqlfolder, "PostgreSQL" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Sql Drivers/PostgreSQL", "Off", &settingsOK );
	psqlOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton ); 
	psqlOff->setOn( entry == "Off" );
	psqlOff->setEnabled( enterprise );
	psqlPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	psqlPlugin->setOn( entry == "Plugin" );
	psqlPlugin->setEnabled( enterprise );
	psqlDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	psqlDirect->setOn( entry == "Direct" );
	psqlDirect->setEnabled( enterprise );
	if ( !enterprise )
	    psqlOff->setOn( true );

	folder = new QCheckListItem( sqlfolder, "OCI" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Sql Drivers/OCI", "Off", &settingsOK );
	ociOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton ); 
	ociOff->setOn( entry == "Off" );
	ociOff->setEnabled( enterprise );
	ociPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	ociPlugin->setOn( entry == "Plugin" );
	ociPlugin->setEnabled( enterprise );
	ociDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	ociDirect->setOn( entry == "Direct" );
	ociDirect->setEnabled( enterprise );
	if ( !enterprise )
	    ociOff->setOn( true );

	folder = new QCheckListItem( sqlfolder, "MySQL" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Sql Drivers/MySQL", "Off", &settingsOK );
	mysqlOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton ); 
	mysqlOff->setOn( entry == "Off" );
	mysqlOff->setEnabled( enterprise );
	mysqlPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	mysqlPlugin->setOn( entry == "Plugin" );
	mysqlPlugin->setEnabled( enterprise );
	mysqlDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	mysqlDirect->setOn( entry == "Direct" );
	mysqlDirect->setEnabled( enterprise );
	if ( !enterprise )
	    mysqlOff->setOn( true );

	folder = new QCheckListItem( sqlfolder, "ODBC" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Sql Drivers/ODBC", "Off", &settingsOK );
	odbcOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton ); 
	odbcOff->setOn( entry == "Off" );
	odbcOff->setEnabled( enterprise );
	odbcPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	odbcPlugin->setOn( entry == "Plugin" );
	odbcPlugin->setEnabled( enterprise );
	odbcDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	odbcDirect->setOn( entry == "Direct" );
	odbcDirect->setEnabled( enterprise );
	if ( !enterprise )
	    odbcOff->setOn( true );

	QCheckListItem *stfolder = new QCheckListItem( advancedList, "Styles" );
	stfolder->setOpen( true );

	folder = new QCheckListItem( stfolder, "SGI" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Styles/SGI", "Direct", &settingsOK );
	sgiOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	sgiOff->setOn( entry == "Off" );
	sgiPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );	    
	sgiPlugin->setOn( entry == "Plugin" );
	sgiDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	sgiDirect->setOn( entry == "Direct" );

	folder = new QCheckListItem( stfolder, "CDE" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Styles/CDE", "Direct", &settingsOK );
	cdeOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	cdeOff->setOn( entry == "Off" );
	cdePlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	cdePlugin->setOn( entry == "Plugin" );
	cdeDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	cdeDirect->setOn( entry == "Direct" );

	folder = new QCheckListItem( stfolder, "MotifPlus" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Styles/MotifPlus", "Direct", &settingsOK );
	motifplusOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	motifplusOff->setOn( entry == "Off" );
	motifplusPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );	    
	motifplusPlugin->setOn( entry == "Plugin" );
	motifplusDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	motifplusDirect->setOn( entry == "Direct" );

	folder = new QCheckListItem( stfolder, "Platinum" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Styles/Platinum", "Direct", &settingsOK );
	platinumOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	platinumOff->setOn( entry == "Off" );
	platinumPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	platinumPlugin->setOn( entry == "Plugin" );
	platinumDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );	    
	platinumDirect->setOn( entry == "Direct" );

	folder = new QCheckListItem( stfolder, "Motif" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Styles/Motif", "Direct", &settingsOK );
	item = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	item->setEnabled( false );
	item->setOn( entry == "Off" );
	item = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	item->setEnabled( false );
	item->setOn( entry == "Plugin" );
	item = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	item->setOn( entry == "Direct" );

	folder = new QCheckListItem( stfolder, "Windows" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Styles/Windows", "Direct", &settingsOK );
	item = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	item->setEnabled( false );
	item->setOn( entry == "Off" );
	item = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
	item->setEnabled( false );
	item->setOn( entry == "Plugin" );
	item = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
	item->setOn( entry == "Direct" );

	entry = settings.readEntry( "/Trolltech/Qt/Advanced C++", "Off", &settingsOK );
	folder = new QCheckListItem( advancedList, "Advanced C++" );
	folder->setOpen( true );
	advancedCppOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	advancedCppOff->setOn( entry == "Off" );
	advancedCppOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
	advancedCppOn->setOn( entry == "On" );

	folder = new QCheckListItem( advancedList, "Tablet Support" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Tablet Support", "Off", &settingsOK );
	tabletOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	tabletOff->setOn( entry == "Off" );
	tabletOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
	tabletOn->setOn( entry == "On" );

	folder = new QCheckListItem( advancedList, "Accessibility" );
	folder->setOpen( true );
	entry = settings.readEntry( "/Trolltech/Qt/Accessibility", "On", &settingsOK );
	accOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	accOff->setOn( entry == "Off" );
	accOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
	accOn->setOn( entry == "On" );

	entry = settings.readEntry( "/Trolltech/Qt/Big Textcodecs", "On", &settingsOK );
	folder = new QCheckListItem( advancedList, "Big Textcodecs" );
	folder->setOpen( true );
	bigCodecsOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	bigCodecsOff->setOn( entry == "Off" );
	bigCodecsOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );	
	bigCodecsOn->setOn( entry == "On" );

	optionSelected( 0 );
	if( reconfigMode )
	    setInstallStep( 1 );
	else
	    setInstallStep( 5 );

	setBackEnabled( buildPage, false );

    } else if( newPage == progressPage ) {
	saveSettings();
	int totalSize = 0;
	QFileInfo fi;
	totalRead = 0;
	bool copySuccessful = true;

	setInstallStep( 6 );
	if( !filesCopied ) {
	    createDir( installPath->text() );
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

	    {
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
	    }

/*These lines are only to be used when changing the filecount estimate

	    QString tmp( "%1" );
	    tmp = tmp.arg( totalFiles );
	    QMessageBox::information( this, tmp, tmp );
*/
	    operationProgress->setProgress( FILESTOCOPY );

#endif
	    {
		QFile inFile( "install.exe" );
		QFile outFile( installPath->text() + "\\bin\\Install.exe" );
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
	    }


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
    } else if( newPage == buildPage ) {
	QStringList args;
	QStringList entries;
	QSettings settings;
	QString entry;
	QStringList::Iterator it;
	QFile tmpFile;
	QTextStream tmpStream;
	bool settingsOK;

	if( reconfigMode )
	    compileProgress->hide();

	autoContTimer.stop();
	nextButton()->setText( "Next >" );
	saveSettings();

	args << ( QEnvironment::getEnv( "QTDIR" ) + "\\bin\\configure.exe" );

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
	for( it = allModules.begin(); it != allModules.end(); ++it ) {
	    entry = *it;
	    if ( entries.find( entry ) != entries.end() )
		args += QString( "-enable-" ) + entry;
	    else
		args += QString( "-disable-") + entry;
	}

	entry = settings.readEntry( "/Trolltech/Qt/SQL Drivers/MySQL", "Off", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-sql-mysql";
	else if ( entry == "Plugin" )
	    args += "-plugin-sql-mysql";
	else if ( entry == "Off" )
	    args += "-no-sql-mysql";

	entry = settings.readEntry( "/Trolltech/Qt/SQL Drivers/OCI", "Off", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-sql-oci";
	else if ( entry == "Plugin" )
	    args += "-plugin-sql-oci";
	else if ( entry == "Off" )
	    args += "-no-sql-oci";

	entry = settings.readEntry( "/Trolltech/Qt/SQL Drivers/ODBC", "Off", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-sql-odbc";
	else if ( entry == "Plugin" )
	    args += "-plugin-sql-odbc";
	else if ( entry == "Off" )
	    args += "-no-sql-odbc";

	entry = settings.readEntry( "/Trolltech/Qt/SQL Drivers/PostgreSQL", "Off", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-sql-psql";
	else if ( entry == "Plugin" )
	    args += "-plugin-sql-psql";
	else if ( entry == "Off" )
	    args += "-no-sql-psql";

	entry = settings.readEntry( "/Trolltech/Qt/SQL Drivers/TDS", "Off", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-sql-tds";
	else if ( entry == "Plugin" )
	    args += "-plugin-sql-tds";
	else if ( entry == "Off" )
	    args += "-no-sql-tds";

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

	entry = settings.readEntry( "/Trolltech/Qt/Tablet Support", "Off", &settingsOK );
	if ( entry == "On" )
	    args += "-tablet";
	else
	    args += "-no-tablet";

	entry = settings.readEntry( "/Trolltech/Qt/Advanced C++", "Off", &settingsOK );
	if ( entry == "On" )
	    args += "-stl";
	else
	    args += "-no-stl";

	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/PNG", "Direct", &settingsOK );
	if ( entry == "Plugin" )
	    args += "-plugin-imgfmt-png";
	else if ( entry == "Direct" )
	    args += "-qt-imgfmt-png";
	else if ( entry == "Off" )
	    args += "-no-imgfmt-png";
#if 0
	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/PNG Present", "No", &settingsOK );
	if ( entry == "No" )
	    args += "-qt-png";
	else
	    args += "-system-png";
#else
	args += "-qt-png";
#endif

	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/JPEG", "Direct", &settingsOK );
	if ( entry == "Plugin" )
	    args += "-plugin-imgfmt-jpeg";
	else if ( entry == "Direct" )
	    args += "-qt-imgfmt-jpeg";
	else if ( entry == "Off" )
	    args += "-no-imgfmt-jpeg";
#if 0
	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/JPEG Present", "No", &settingsOK );
	if ( entry == "No" )
	    args += "-qt-jpeg";
	else
	    args += "-system-jpeg";
#else
	args += "-qt-jpeg";
#endif

	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/MNG", "Direct", &settingsOK );
	if ( entry == "Plugin" )
	    args += "-plugin-imgfmt-mng";
	else if ( entry == "Direct" )
	    args += "-qt-imgfmt-mng";
	else if ( entry == "Off" )
	    args += "-no-imgfmt-mng";
#if 0
	entry = settings.readEntry( "/Trolltech/Qt/Image Formats/MNG Present", "No", &settingsOK );
	if ( entry == "No" )
	    args += "-qt-mng";
	else
	    args += "-system-mng";
#else
	args += "-qt-mng";
#endif

	entry = settings.readEntry( "/Trolltech/Qt/Styles/Windows", "Direct", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-style-windows";
	else if ( entry == "Plugin" )
	    args += "-plugin-style-windows";
	else if ( entry == "Off" )
	    args += "-no-style-windows";

	entry = settings.readEntry( "/Trolltech/Qt/Styles/Motif", "Direct", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-style-motif";
	else if ( entry == "Plugin" )
	    args += "-plugin-style-motif";
	else if ( entry == "Off" )
	    args += "-no-style-motif";

	entry = settings.readEntry( "/Trolltech/Qt/Styles/Platinum", "Direct", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-style-platinum";
	else if ( entry == "Plugin" )
	    args += "-plugin-style-platinum";
	else if ( entry == "Off" )
	    args += "-no-style-platinum";

	entry = settings.readEntry( "/Trolltech/Qt/Styles/MotifPlus", "Direct", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-style-motifplus";
	else if ( entry == "Plugin" )
	    args += "-plugin-style-motifplus";
	else if ( entry == "Off" )
	    args += "-no-style-motifplus";

	entry = settings.readEntry( "/Trolltech/Qt/Styles/CDE", "Direct", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-style-cde";
	else if ( entry == "Plugin" )
	    args += "-plugin-style-cde";
	else if ( entry == "Off" )
	    args += "-no-style-cde";

	entry = settings.readEntry( "/Trolltech/Qt/Styles/SGI", "Direct", &settingsOK );
	if ( entry == "Direct" )
	    args += "-qt-style-sgi";
	else if ( entry == "Plugin" )
	    args += "-plugin-style-sgi";
	else if ( entry == "Off" )
	    args += "-no-style-sgi";

	if( !installExamples->isChecked() )
	    args += "-no-examples";
	if( !installTutorials->isChecked() )
	    args += "-no-tutorials";

	if( qWinVersion() & WV_NT_based ) {
	    logOutput( "Execute configure...\n" );
	    logOutput( args.join( " " ) + "\n" );

	    configure.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	    configure.setArguments( args );
	    // Start the configure process
	    compileProgress->setTotalSteps( int(double(filesToCompile) * 2.6) );
	    if( !configure.start() )
		logOutput( "Could not start configure process" );
	} else { // no proper process handling on DOS based systems - create a batch file instead
	    logOutput( "Generating batch file...\n" );
	    QFile outFile( installPath->text() + "\\build.bat" );
	    QTextStream outStream( &outFile );

	    if( outFile.open( IO_WriteOnly | IO_Translate ) ) {
		outStream << "cd %QTDIR%" << endl;
		outStream << args.join( " " ) << endl;
		if( !reconfigMode ) {
		    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
		    outStream << makeCmds[ sysID ].latin1() << endl;
		}
		outFile.close();
	    }
	    doFinalIntegration();
	    compileProgress->setTotalSteps( compileProgress->totalSteps() );
	    showPage( finishPage );
	}
	if( reconfigMode )
	    setInstallStep( 2 );
	else
	    setInstallStep( 7 );

    } else if( newPage == finishPage ) {
	autoContTimer.stop();
	nextButton()->setText( "Next >" );
	QString finishMsg;
	if( qWinVersion() & WV_NT_based ) {
	    if( reconfigMode ) {
		if( !rebuildInstallation->isChecked() )
		    finishMsg = "Qt has been reconfigured, and is ready to be rebuilt.";
		else
		    finishMsg = "Qt has been reconfigured and rebuilt, and is ready for use.";
	    }
	    else {
		finishMsg = QString( "Qt has been installed to " ) + installPath->text() + " and is ready to use.\nYou may need to reboot, or open";
		finishMsg += " the environment editing dialog to make changes to the environment visible";
	    }
	} else {
	    if( reconfigMode ) {
		    finishMsg = "The new configuration has been written.\nThe library needs to be rebuilt to activate the ";
		    finishMsg += "new configuration.  To rebuild it, use the \"Build Qt ";
		    finishMsg += QString( QT_VERSION_STR );
		    finishMsg += "\" icon in the Qt program group in the start menu.";
	    }
	    else {
		finishMsg = QString( "The Qt files have been installed to " ) + installPath->text() + " and is ready to be compiled.\n";
		if( persistentEnv ) {
		    finishMsg += "The environment variables needed to use Qt have been recorded into your AUTOEXEC.BAT file.\n";
		    finishMsg += "Please review this file, and take action as appropriate depending on your operating system to get them into the persistent environment. (Windows Me users, run MsConfig)\n\n";
		}
		finishMsg += QString( "To build Qt, use the \"Build Qt " ) + QString( QT_VERSION_STR ) + "\" icon which has been installed into your Start-Menu.";
	    }
	}
	finishText->setText( finishMsg );
	if( reconfigMode )
	    setInstallStep( 3 );
	else
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
    bool enterprise = licenseInfo[ "PRODUCTS" ] == "qt-enterprise";

    if ( item->text(0) == "Static" && item->isOn() ) {
	if ( !QMessageBox::information( this, "Are you sure?", "It will not be possible to build components "
				  "or plugins if you select the static build of the Qt library.\n"
				  "New features, e.g souce code editing in Qt Designer, will not "
				  "be available, "
				  "\nand you or users of your software might not be able "
				  "to use all or new features, e.g. new styles.\n\n"
				  "Are you sure you want to build a static Qt library?",
				  "No, I want to use the cool new stuff", "Yes" ) ) {
		item->setOn( false );
		if ( ( item = (QCheckListItem*)configList->findItem( "Shared", 0, 0 ) ) ) {
		item->setOn( true );
		configList->setCurrentItem( item );
	    }
	} else {
	    if ( accOn->isOn() ) {
		accOn->setOn( false );
		accOff->setOn( true );
	    }
	    if ( bigCodecsOff->isOn() ) {
		bigCodecsOn->setOn( true );
		bigCodecsOff->setOn( false );
	    }
	    if ( mngPlugin->isOn() ) {
		mngDirect->setOn( true );
		mngPlugin->setOn( false );
		mngOff->setOn( false );
	    }
	    if ( pngPlugin->isOn() ) {
		pngDirect->setOn( true );
		pngPlugin->setOn( false );
		pngOff->setOn( false );
	    }
	    if ( jpegPlugin->isOn() ) {
		jpegDirect->setOn( true );
		jpegPlugin->setOn( false );
		jpegOff->setOn( false );
	    }
	    if ( sgiPlugin->isOn() ) {
		sgiPlugin->setOn( false );
		sgiDirect->setOn( true );
	    }
	    if ( cdePlugin->isOn() ) {
		cdePlugin->setOn( false );
		cdeDirect->setOn( true );
	    }
	    if ( motifplusPlugin->isOn() ) {
		motifplusPlugin->setOn( false );
		motifplusDirect->setOn( true );
	    }
	    if ( platinumPlugin->isOn() ) {
		platinumPlugin->setOn( false );
		platinumDirect->setOn( true );
	    }
	    if ( enterprise ) {
		if ( mysqlPlugin->isOn() ) {
		    mysqlPlugin->setOn( false );
		    mysqlDirect->setOn( true );
		}
		if ( ociPlugin->isOn() ) {
		    ociPlugin->setOn( false );
		    ociDirect->setOn( true );
		}
		if ( odbcPlugin->isOn() ) {
		    odbcPlugin->setOn( false );
		    odbcDirect->setOn( true );
		}
		if ( psqlPlugin->isOn() ) {
		    psqlPlugin->setOn( false );
		    psqlDirect->setOn( true );
		}
		if ( tdsPlugin->isOn() ) {
		    tdsPlugin->setOn( false );
		    tdsDirect->setOn( true );
		}
	    }

	    accOn->setEnabled( false );
	    bigCodecsOff->setEnabled( false );
	    mngPlugin->setEnabled( false );
	    pngPlugin->setEnabled( false );
	    jpegPlugin->setEnabled( false );
	    sgiPlugin->setEnabled( false );
	    cdePlugin->setEnabled( false );
	    motifplusPlugin->setEnabled( false );
	    platinumPlugin->setEnabled( false );
	    if ( enterprise ) {
		mysqlPlugin->setEnabled( false );
		ociPlugin->setEnabled( false );
		odbcPlugin->setEnabled( false );
		psqlPlugin->setEnabled( false );
		tdsPlugin->setEnabled( false );
	    }
	}
	return;
    } else if ( item->text( 0 ) == "Shared" && item->isOn() ) {
	accOn->setEnabled( true );
	bigCodecsOff->setEnabled( true );
	mngPlugin->setEnabled( true );
	pngPlugin->setEnabled( true );
	jpegPlugin->setEnabled( true );
	sgiPlugin->setEnabled( true );
	cdePlugin->setEnabled( true );
	motifplusPlugin->setEnabled( true );
	platinumPlugin->setEnabled( true );
	if ( enterprise ) {
	    mysqlPlugin->setEnabled( true );
	    ociPlugin->setEnabled( true );
	    odbcPlugin->setEnabled( true );
	    psqlPlugin->setEnabled( true );
	    tdsPlugin->setEnabled( true );
	}
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

    if( ( i == mysqlDirect || i == mysqlPlugin ) && 
	!(findFileInPaths( "libmysql.lib", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "LIB" ) ) ) && 
	  findFileInPaths( "mysql.h", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The MySQL driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if( ( i == ociDirect || i == ociPlugin ) && 
	!(findFileInPaths( "oci.lib", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "LIB" ) ) ) && 
	  findFileInPaths( "oci.h", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The OCI driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if( ( i == odbcDirect || i == odbcPlugin ) && 
	!(findFileInPaths( "odbc32.lib", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "LIB" ) ) ) && 
	  findFileInPaths( "sql.h", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The ODBC driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if( ( i == psqlDirect || i == psqlPlugin ) && 
	!(findFileInPaths( "libpqdll.lib", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "LIB" ) ) ) && 
	  findFileInPaths( "libpq-fe.h", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The PostgreSQL driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if ( ( i == tdsDirect || i == tdsPlugin ) &&
	!(findFileInPaths( "ntwdblib.lib", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "LIB" ) ) ) && 
	  findFileInPaths( "sqldb.h", QStringList::split( QRegExp( "[;,]" ), QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The PostgreSQL driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    
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
	explainOption->setText( tr("Some of these modules are optional. "
				   "You can deselect the modules that you "
				   "don't require for your development.\n"
				   "By default, all modules are selected.") );
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
				       "All applications created with a static "
				       "library will be at least 1.5MB big.</p>"
				       "<p><font color=\"red\">It is not possible to "
				       "build or use any components or plugins with a "
				       "static Qt library!</font></p>") );
	} else {
	    explainOption->setText("<p>A shared Qt library makes it necessary to "
				   "distribute the Qt DLL together with your software.</p>"
				   "<p>Applications and libraries linked against a shared Qt library "
				   "are small and can make use of components and plugins.</p>" );
	}
    } else if ( i->text( 0 ) == "Sql Drivers" ) {
	explainOption->setText("Select the SQL Drivers you want to support. "
				"<font color=#FF0000>You must have the appropriate client libraries "
				"and header files installed correctly before you can build the Qt SQL drivers.</font>" );
    } else if ( ( i->parent() && i->parent()->text( 0 )== "Sql Drivers" )
	||  ( i->parent() && i->parent()->parent() && i->parent()->parent()->text( 0 )== "Sql Drivers" ) ) {
	explainOption->setText( "Select the SQL Drivers you want to support. "
			        "<font color=#FF0000>You must have the appropriate client libraries "
				"and header files installed correctly before you can build the Qt SQL drivers.</font> "
				"Selected SQL Drivers will be integrated into Qt. \nYou can also "
				"build every SQL Driver as a plugin to be more flexible for later "
				"extensions. Building a plugin is not supported in the installer at "
				"this point. You will have to do it manually after the installation "
				"succeeded. Read the help files in QTDIR\\plugins\\src\\sqldrivers for "
				"further instructions." );
    } else if ( i->text( 0 ) == "Styles" ) {
	explainOption->setText( QString("Select support for the various GUI styles that Qt has emulation for." ) );
    } else if ( ( i->parent() && i->parent()->text( 0 ) == "Styles" ) 
	||  ( i->parent() && i->parent()->parent() && i->parent()->parent()->text( 0 )== "Styles" ) ) {
	QString styleName = (i->parent()->text( 0 ) == "Styles") ? i->text( 0 ) : i->parent()->text( 0 );
	if ( styleName == "Windows" || styleName == "Motif" ) {
	    explainOption->setText( QString("The %1 style is builtin to the Qt library.").arg( styleName ) );
	} else {
	    explainOption->setText( QString("Selects support for the %1 style. The style can be built "
				    "as a plugin, or builtin to the library, or not at all.").arg( styleName ) );
	}
    } else if ( i == accOff ) {
	explainOption->setText( "Turns off accessibility support. People who need accessibility "
				"clients will not be able to use your software." );
    } else if ( i == accOn ) {
	explainOption->setText( "Enables your Qt software to be used with accessibility clients, "
				"like a magnifier or narrator tool. The accessibility information "
				"for the Qt widgets is provided by a plugin on demand and can be "
				"exchanged or extended easily." );
    } else if ( i->text(0) == "Accessibility" ) {
	explainOption->setText( "Accessibility means making software usable and accessible to a wide "
				"range of users, including those with disabilities.\n"
				"This feature relies on components and is not available with a static "
				"Qt library." );
    } else if ( i == bigCodecsOff ) {
	explainOption->setText( "All big textcodecs are provided by plugins and get loaded on demand." );
    } else if ( i == bigCodecsOn ) {
	explainOption->setText( "The big textcodecs are compiled into the Qt library." );
    } else if ( i->text(0) == "Big Textcodecs" ) {
	explainOption->setText( "Textcodecs provide translations between text encodings. For "
				"languages and script systems with many characters it is necessary "
				"to have big data tables that provide the translation. Those codecs "
				"can be left out of the Qt library and will be loaded on demand.\n"
				"Having the codecs in a plugin is not available with a static Qt "
				"library." );
    } else if ( i->text(0) == "Tablet Support" ) {
	explainOption->setText( "Qt can support the Wacom brand tablet device." );
    } else if ( i == tabletOff ) {
	explainOption->setText( "Support for the Wacom tablet is disabled. This is the default option." );
    } else if ( i == tabletOn ) {
	explainOption->setText( "This option builds in support for Wacom(c) tablets.\n"
				"To use a supported tablet, you must have built the Wintab SDK available "
				"at http://www.pointing.com/FTP.HTM and have your INCLUDE and LIBRARY path "
				"set appropriately." );
    } else if ( i->text(0) == "Advanced C++" ) {
	explainOption->setText( "Qt can be built with exception handling and STL support enabled or "
				"disabled. The default is to disable advanced C++ features." );
    } else if ( i == advancedCppOn ) {
	explainOption->setText( "This option builds Qt with exception handling and STL support enabled. "
				"Depending on your compiler, this might cause slower execution, larger "
				"binaries, or compiler issues." );
    } else if ( i == advancedCppOff ) {
	explainOption->setText( "This option turns advanced C++ features off when building Qt." );
    } else if ( i->text(0) == "Image Formats" ) {
	explainOption->setText( "Qt ships with support for a wide range of common image formats. "
				"Standard formats are always included in Qt, and some more special formats "
				"can be left out from the Qt library itself and provided by a plugin instead." );
    } else if ( i == mngPlugin ) {
	explainOption->setText( "Support for MNG images is provided by a plugin that is loaded on demand." );
    } else if ( i == mngOff ) {
	explainOption->setText( "Turn off support for MNG images." );
    } else if ( i == mngDirect ) {
	explainOption->setText( "Support for MNG images is compiled into Qt." );
#if 0
    } else if ( i == mngPresent ) {
	explainOption->setText( "Support for MNG images is provided by linking against an existing libmng." );
#endif
    } else if ( i->text(0) == "MNG" ) {
	explainOption->setText( "Qt supports the \"Multiple-Image Network Graphics\" format either by "
				"linking against an existing libmng, by compiling the mng sources "
				"into Qt, or by loading a plugin on demand." );
    } else if ( i == jpegPlugin ) {
	explainOption->setText( "Support for JPEG images is provided by a plugin that is loaded on demand." );
    } else if ( i == jpegOff ) {
	explainOption->setText( "Turn off support for JPEG images." );
    } else if ( i == jpegDirect ) {
	explainOption->setText( "Support for JPEG images is compiled into Qt." );
#if 0
    } else if ( i == jpegPresent ) {
	explainOption->setText( "Support for JPEG images is provided by linking against an existing libjpeg." );
#endif
    } else if ( i->text(0) == "JPEG" ) {
	explainOption->setText( "Qt supports the \"Joint Photographic Experts Group\" format either "
				"by linking against an existing libjpeg, by compiling the jpeg sources "
				"into Qt, or by loading a plugin on demand." );
    } else if ( i == pngPlugin ) {
	explainOption->setText( "Support for PNG images is provided by a plugin that is loaded on demand." );
    } else if ( i == pngOff ) {
	explainOption->setText( "Turn off support for PNG images." );
    } else if ( i == pngDirect ) {
	explainOption->setText( "Support for PNG images is compiled into Qt." );
#if 0
    } else if ( i == pngPresent ) {
	explainOption->setText( "Support for PNG images is provided by linking against an existing libpng." );
#endif
    } else if ( i->text(0) == "PNG" ) {
	explainOption->setText( "Qt supports the \"Portable Network Graphics\" format either by "
				"linking against an existing libpng, by compiling the png support "
				"into Qt, or by loading a plugin on demand." );
    }
}

void SetupWizardImpl::configPageChanged()
{
    if ( configList->isVisible() ) {
	configList->setSelected( configList->currentItem(), true );
	optionSelected( configList->currentItem() );
    } else if ( advancedList->isVisible() ) {
	advancedList->setSelected( advancedList->currentItem(), true );
	optionSelected( advancedList->currentItem() );
    }
}

void SetupWizardImpl::licenseChanged( const QString & )
{
    QString customer = customerID->text();
    QString license = licenseID->text();
    QString name = licenseeName->text();
    QString date = expiryDate->text();

    setNextEnabled( licensePage, !customer.isEmpty() && 
				 !license.isEmpty() && 
				 !name.isEmpty() && 
				 !date.isEmpty() );
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
    outstream << ( entry + "\n" );

    if( close )
	fileLog.close();
}

void SetupWizardImpl::logOutput( const QString& entry, bool close )
{
    static QTextStream outstream;
    if( !outputLog.isOpen() ) {
	outputLog.setName( installPath->text() + "\\build.log" );
	if( !outputLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    outstream.setDevice( &outputLog );

    outputDisplay->append( entry + "\n" );
    outstream << ( entry + "\n" );

    if( close )
	outputLog.close();
}

void SetupWizardImpl::archiveMsg( const QString& msg )
{
#if defined (USE_ARCHIVES)
    qApp->processEvents();
    if( msg.find(QRegExp("Read \\d*")) == 0 ) { //progress message
	operationProgress->setProgress( msg.right( msg.findRev(' ') + 1).toInt() );
	return;
    } 
    logFiles( msg );
#else
    Q_UNUSED(msg)
#endif
}


#if defined (USE_ARCHIVES)
void SetupWizardImpl::readArchive( const QString& arcname, const QString& installPath )
{
    QArchive ar;
    archive.setVerbosity( QArchive::Destination | QArchive::Verbose | QArchive::Progress );
    connect( &ar, SIGNAL( operationFeedback( const QString& ) ), this, SLOT( archiveMsg( const QString& ) ) );
    ar.setPath( arcname );
    if(ar.open( IO_ReadOnly ) ) 
	ar.readArchive( installPath );
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
		    if( !copyFiles( entryName, targetName, false ) )
			return false;
	    } else {
		if( qApp && !isHidden() ) {
		    qApp->processEvents();
		    operationProgress->setProgress( totalFiles );
		    logFiles( targetName );
		} else {
		    return false;
		}
		if( entryName.right( 4 ) == ".cpp" || 
		    entryName.right( 2 ) == ".c" ||
		    entryName.right( 4 ) == ".pro" ||
		    entryName.right( 3 ) == ".ui" )
		    filesToCompile++;
		bool res = true;
		if ( !QFile::exists( targetName ) )
		    res = CopyFileA( entryName.local8Bit(), targetName.local8Bit(), false );

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
    if( reconfigMode )
	setCaption( QString( "Qt Configuration Wizard - Step %1 of 3" ).arg( step ) );
    else
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

bool SetupWizardImpl::findFileInPaths( QString fileName, QStringList paths )
{
	QDir d;
	for( QStringList::Iterator it = paths.begin(); it != paths.end(); ++it ) {
		if( d.exists( (*it) + "\\" + fileName ) )
			return true;
	}
	return false;
}
