#include "setupwizardimpl.h"
#include "environment.h"
#include "confirmdlg.h"
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

#define BUFFERSIZE 64 * 1024

//#define USE_ARCHIVES 1

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) :
    SetupWizard( pParent, pName, modal, f ),
    filesCopied( false ),
    filesToCompile( 0 ),
    filesCompiled( 0 ),
    sysID( 0 ),
    app( NULL ),
    tmpPath( QEnvironment::getTempPath() )
{
    setNextEnabled( introPage, false );
    setBackEnabled( progressPage, false );
    setNextEnabled( progressPage, false );
    setBackEnabled( buildPage, false );
    setNextEnabled( buildPage, false );
    setFinishEnabled( finishPage, true );
    setBackEnabled( finishPage, false );
#if defined (USE_ARCHIVES)
    readArchive( "sys.arq", tmpPath );
#endif
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

void SetupWizardImpl::licenseAccepted( )
{
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
    
    for( int i = 0; i < outbuffer.length(); i++ ) {
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
		if( currentOLine.right( 4 ) == ".cpp" ) {
		    filesCompiled++;
		    compileProgress->setProgress( filesCompiled );
		}
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
    
    for( int i = 0; i < outbuffer.length(); i++ ) {
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
		if( currentELine.right( 4 ) == ".cpp" ) {
		    filesCompiled++;
		    compileProgress->setProgress( filesCompiled );
		}
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

void SetupWizardImpl::installIcons( QString iconFolder, QString dirName, bool common )
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

void SetupWizardImpl::integratorDone()
{
    QString dirName, examplesName, tutorialsName;
    bool common( folderGroups->currentItem() == 1 );

    logOutput( QString::null, true );
    
    /*
    ** We still have some more items to do in order to finish all the
    ** integration stuff.
    */
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
    shell.createShortcut( dirName, common, "On-line documentation", QEnvironment::getEnv( "QTDIR" ) + "\\doc\\index.html", "Browse the On-line documentation" );
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

    uninstaller << DISTVER;

    QEnvironment::recordUninstall( QString( "Qt " ) + DISTVER, uninstaller.join( " " ) );

    setNextEnabled( buildPage, true );
}

void SetupWizardImpl::makeDone()
{
    QStringList args;
    QStringList integrators = QStringList::split( ' ', "integrate_msvc.bat integrate_borland.bat integrate_gcc.bat" );

    connect( &integrator, SIGNAL( processExited() ), this, SLOT( integratorDone() ) );
    connect( &integrator, SIGNAL( readyReadStdout() ), this, SLOT( readIntegratorOutput() ) );
    connect( &integrator, SIGNAL( readyReadStderr() ), this, SLOT( readIntegratorError() ) );

    args << tmpPath + QString( "\\" ) + integrators[ sysID ];
    args << devSysPath->text() + "\\Common\\MsDev98\\Addins";

    integrator.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
    integrator.setArguments( args );
    integrator.start();
    setNextEnabled( buildPage, true );
}

void SetupWizardImpl::configDone()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
    QStringList args;

    connect( &make, SIGNAL( processExited() ), this, SLOT( makeDone() ) );
    connect( &make, SIGNAL( readyReadStdout() ), this, SLOT( readMakeOutput() ) );
    connect( &make, SIGNAL( readyReadStderr() ), this, SLOT( readMakeError() ) );

    args << makeCmds[ sysID ];

    make.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
    make.setArguments( args );

    make.start();
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
    // radios
    QListViewItem* config = list->firstChild();
    while ( config ) {
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    if ( item->type() == QCheckListItem::RadioButton ) {
		if ( item->isOn() ) {
		    settings.writeEntry( "/Trolltech/Qt/" + config->text(0), item->text() );
		    break;
		}
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	config = config->nextSibling();
    }

    // checks
    config = list->firstChild();
    QStringList lst;
    while ( config ) {
	bool foundChecks = FALSE;
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    if ( item->type() == QCheckListItem::CheckBox ) {
		if ( item->isOn() )
		    lst += item->text();
		foundChecks = TRUE;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	if ( foundChecks )
	    settings.writeEntry( "/Trolltech/Qt/" + config->text(0), lst, ',' );
	config = config->nextSibling();
	lst.clear();
    }
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
	installPath->setText( QString( "C:\\Qt\\" ) + DISTVER );
	sysGroup->setButton( 0 );
    }
    else if( newPage == foldersPage ) {
	QStringList devSys = QStringList::split( ';',"Microsoft Visual Studio path;Borland C++ Builder path;GNU C++ path" );

	folderPath->setText( QString( "Qt " ) + DISTVER );
	devSysLabel->setText( devSys[ sysID ] );
	devSysPath->setEnabled( sysID == 0 );
	devSysPathButton->setEnabled( sysID == 0 );
	if( sysID == 0 )
	    devSysPath->setText( QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual Studio", "ProductDir", QEnvironment::LocalMachine ) );
	if( int( qWinVersion() ) & int( Qt::WV_NT_based ) )   // On NT we also have a common folder
	    folderGroups->setEnabled( true );
	else
	    folderGroups->setDisabled( true );
    }
    else if( newPage == progressPage ) {
	int totalSize( 0 );
	QFileInfo fi;
	totalRead = 0;

	if( !filesCopied ) {
#if defined (USE_ARCHIVES)
	    fi.setFile( "qt.arq" );
	    if( fi.exists() )
		totalSize = fi.size();

	    fi.setFile( "build.arq" );
	    if( fi.exists() )
		totalSize += fi.size();
	    
	    fi.setFile( "uninstall.arq" );
	    if( fi.exists() )
		totalSize += fi.size();

	    if( installDocs->isChecked() ) {
		fi.setFile( "doc.arq" );
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

	    filesDisplay->append( "Starting copy process\n" );
	    readArchive( "build.arq", installPath->text() );
	    readArchive( "uninstall.arq", shell.windowsFolderName );

	    readArchive( "qt.arq", installPath->text() );
	    if( installDocs->isChecked() )
		readArchive( "doc.arq", installPath->text() );
	    if( installExamples->isChecked() )
		readArchive( "examples.arq", installPath->text() );
	    if( installTutorials->isChecked() )
		readArchive( "tutorial.arq", installPath->text() );
#else
	    copyFiles( QDir::currentDirPath(), installPath->text(), true );
#endif
	    filesCopied = true;
	    logFiles( "All files have been copied,\nThis log has been saved to the installation directory.\n", true );
	}
	setNextEnabled( progressPage, true );
    }
    else if( newPage == configPage ) {
	QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++" );
	QByteArray pathBuffer;
	QStringList path;
	ConfirmDlg confirm;

	path = QStringList::split( ';', QEnvironment::getEnv( "PATH" ) );
	if( path.findIndex( "%QTDIR%\\lib" ) == -1 )
	    path.prepend( "%QTDIR%\\lib" );
	if( path.findIndex( "%QTDIR%\\bin" ) == -1 )
	    path.prepend( "%QTDIR%\\bin" );
	QEnvironment::putEnv( "PATH", path.join( ";" ) );

	confirm.confirmText->setText( "Do you want to set QTDIR to point to the new installation?" );
	if( confirm.exec() ) {
	    QEnvironment::putEnv( "QTDIR", installPath->text(), QEnvironment::LocalEnv | QEnvironment::DefaultEnv );
	    QEnvironment::putEnv( "MKSPEC", mkSpecs[ sysID ], QEnvironment::LocalEnv | QEnvironment::DefaultEnv );

	    path.clear();
	    path = QStringList::split( ';', QEnvironment::getEnv( "PATH", QEnvironment::DefaultEnv ) );
	    if( path.findIndex( "%QTDIR%\\lib" ) == -1 )
		path.prepend( "%QTDIR%\\lib" );
	    if( path.findIndex( "%QTDIR%\\bin" ) == -1 )
		path.prepend( "%QTDIR%\\bin" );
	    QEnvironment::putEnv( "PATH", path.join( ";" ), QEnvironment::DefaultEnv );
	}
	else {
	    QEnvironment::putEnv( "QTDIR", installPath->text(), QEnvironment::LocalEnv );
	    QEnvironment::putEnv( "MKSPEC", mkSpecs[ sysID ], QEnvironment::LocalEnv );
	}


	configList->clear();
	advancedList->clear();
	configList->setSorting( -1 );
	advancedList->setSorting( -1 );
	QCheckListItem* item;
	connect( &configure, SIGNAL( processExited() ), this, SLOT( configDone() ) );
	connect( &configure, SIGNAL( readyReadStdout() ), this, SLOT( readConfigureOutput() ) );
	connect( &configure, SIGNAL( readyReadStderr() ), this, SLOT( readConfigureError() ) );

	QString qtdir = QEnvironment::getEnv( "QTDIR" );

	QString mkspecsdir = qtdir + "/mkspecs";
	QString mkspecsenv = QEnvironment::getEnv( "MKSPEC" );
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
    else if( newPage == buildPage ) {
	QStringList args;
	QStringList entries;
	QSettings settings;
	QString entry;
	QStringList::Iterator it;
	QFile tmpFile;
	QTextStream tmpStream;
	bool settingsOK;

	outputDisplay->append( "Execute configure...\n" );

	args << QEnvironment::getEnv( "QTDIR" ) + "\\configure.exe";
	entry = settings.readEntry( "/Trolltech/Qt/Mode", "Debug", &settingsOK );
	if ( entry == "Debug" )
	    args += "-debug";
	else
	    args += "-release";

	entry = settings.readEntry( "/Trolltech/Qt/Build", "Shared", &settingsOK );
	if ( entry == "Static" )
	    args += "-static";
	else
	    args += "-shared";

	entry = settings.readEntry( "/Trolltech/Qt/Threading", QString::null, &settingsOK );
	if ( entry == "Threaded" )
	    args += "-thread";

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

	outputDisplay->append( args.join( " " ) + "\n" );
	configure.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	configure.setArguments( args );

	// Start the configure process
	configure.start();
	compileProgress->setTotalSteps( filesToCompile );
    }
}

bool SetupWizardImpl::createDir( QString fullPath )
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

#if defined (USE_ARCHIVES)
void SetupWizardImpl::readArchive( QString arcname, QString installPath )
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
		    if( app ) {
			app->processEvents();
			operationProgress->setProgress( totalRead );
			logFiles( dirName + "\\\n" );
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

		    if( app ) {
			app->processEvents();
			operationProgress->setProgress( totalRead );
			logFiles( fileName + "\n" );
		    }
		    // Try to count the files to get some sort of idea of compilation progress
		    if( ( entryName.right( 4 ) == ".cpp" ) || ( entryName.right( 2 ) == ".h" ) )
			filesToCompile++;

		    // Get timestamp from the archive
		    inStream >> timeStamp;
//		    qDebug( "%s", timeStamp.toString().latin1() );
		    outStream.setDevice( &outFile );
		    inStream >> entryLength;
		    totalRead += sizeof( entryLength );
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
#endif

void SetupWizardImpl::logFiles( QString entry, bool close )
{
    if( !fileLog.isOpen() ) {
	fileLog.setName( installPath->text() + "\\install.log" );
	if( !fileLog.open( IO_WriteOnly ) )
	    return;
    }
    QTextStream outstream( &fileLog );

    filesDisplay->append( entry );
//    filesDisplay->setText( filesDisplay->text() + entry );
    outstream << entry;

    if( close )
	fileLog.close();
}

void SetupWizardImpl::logOutput( QString entry, bool close )
{
    QTextStream outstream;
    if( !outputLog.isOpen() ) {
	outputLog.setName( installPath->text() + "\\buildlog.txt" );
	if( !outputLog.open( IO_WriteOnly ) )
	    return;
    }
    outstream.setDevice( &outputLog );

    outputDisplay->append( entry );
    outstream << entry;

    if( close )
	outputLog.close();
}

#if !defined( USE_ARCHIVES )
void SetupWizardImpl::copyFiles( QString sourcePath, QString destPath, bool topLevel )
{
    QDir dir( sourcePath );
    const QFileInfoList* list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo* fi;

    while( ( fi = it.current() ) ) {
	if( fi->fileName()[ 0 ] != '.' ) {
	    QString entryName = sourcePath + QDir::separator() + fi->fileName();
	    QString targetName = destPath + QDir::separator() + fi->fileName();
	    if( app ) {
		app->processEvents();
//		operationProgress->setProgress( totalRead );
		logFiles( targetName + "\n" );
	    }
	    if( fi->isDir() ) {
		if( !dir.exists( targetName ) )
		    createDir( targetName );
		if( topLevel ) {
		    if( fi->fileName() == "doc" )
			if( installDocs->isChecked() )
			    copyFiles( entryName, targetName );
			else
			    continue;
		    else if( fi->fileName() == "tutorial" )
			if ( installTutorials->isChecked() )
			    copyFiles( entryName, targetName );
			else
			    continue;
		    else if( fi->fileName() == "examples" )
			if( installExamples->isChecked() )
			    copyFiles( entryName, targetName );
			else
			    continue;
		}
		copyFiles( entryName, targetName );
	    }
	    else {
		QByteArray buffer( fi->size() );
		QFile inFile( entryName );
		QFile outFile( targetName );

		if( inFile.open( IO_ReadOnly ) ) {
		    if( outFile.open( IO_WriteOnly ) ) {
			if( buffer.size() ) {
			    inFile.readBlock( buffer.data(), buffer.size() );
			    outFile.writeBlock( buffer.data(), buffer.size() );
			}
			outFile.close();
		    }
		    inFile.close();
		}
	    }
	}
	++it;
    }
}
#endif

