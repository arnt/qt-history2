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

#define BUFFERSIZE 65536

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
//    setFinishEnabled( finishPage, true );
    setBackEnabled( progressPage, false );
//    setBackEnabled( finishPage, false );
    setNextEnabled( progressPage, false );
    setBackEnabled( buildPage, false );
    setNextEnabled( buildPage, false );

    readArchive( "sys.arq", tmpPath );
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

void SetupWizardImpl::clickedSave()
{
    QString fileName = QFileDialog::getSaveFileName( QEnvironment::getEnv( "QTDIR" ) + "\\buildlog.txt", "*.txt" );

    if( fileName.isEmpty() )
	return;

    QFile logFile( fileName );
    if( logFile.open( IO_WriteOnly ) ) {
	QTextStream outStream( &logFile );
	for( int i = 0; i < outputDisplay->count(); i++ ) {
	    QString entry = outputDisplay->text( i );
	    outStream << entry.latin1();
	}
	logFile.close();
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

void SetupWizardImpl::updateOutputDisplay( QProcess* proc )
{
    QString outbuffer;

    outbuffer = QString( proc->readStdout() ) + QString( proc->readStderr() );
    
    for( int i = 0; i < outbuffer.length(); i++ ) {
	QChar c = outbuffer[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    currentLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    if( currentLine.length() ) {
		if( currentLine.right( 4 ) == ".cpp" ) {
		    filesCompiled++;
		    compileProgress->setProgress( filesCompiled );
		}
		outputDisplay->insertItem( currentLine );
		outputDisplay->setBottomItem( outputDisplay->count() - 1 );
		currentLine = "";
	    }
	    break;
	default:
	    currentLine += c;
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
    /*
    ** We still have some more items to do in order to finish all the
    ** integration stuff.
    */
    switch( sysID ) {
    case MSVC:
	{
	    
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
    if( installTutorials->isChecked() ) {
	tutorialsName = shell.createFolder( folderPath->text() + "\\Tutorials", common );
	installIcons( tutorialsName, QEnvironment::getEnv( "QTDIR" ) + "\\tutorial", common );
    }
    if( installExamples->isChecked() ) {
	examplesName = shell.createFolder( folderPath->text() + "\\Examples", common );
	installIcons( examplesName, QEnvironment::getEnv( "QTDIR" ) + "\\examples", common );
    }
    setNextEnabled( buildPage, true );
}

void SetupWizardImpl::makeDone()
{
    QStringList args;
    QStringList integrators = QStringList::split( ' ', "integrate_msvc.bat integrate_borland.bat integrate_gcc.bat" );

    connect( &integrator, SIGNAL( processExited() ), this, SLOT( integratorDone() ) );
    connect( &integrator, SIGNAL( readyReadStdout() ), this, SLOT( readIntegratorOutput() ) );
    connect( &integrator, SIGNAL( readyReadStderr() ), this, SLOT( readIntegratorOutput() ) );

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
    connect( &make, SIGNAL( readyReadStderr() ), this, SLOT( readMakeOutput() ) );

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
	if( int( qWinVersion ) & int( Qt::WV_NT_based ) )   // On NT we also have a common folder
	    folderGroups->setEnabled( true );
	else
	    folderGroups->setEnabled( false );
    }
    else if( newPage == progressPage ) {
	int totalSize( 0 );
	QFileInfo fi;
	totalRead = 0;

	if( !filesCopied ) {
	    fi.setFile( "qt.arq" );
	    if( fi.exists() )
		totalSize = fi.size();

	    fi.setFile( "build.arq" );
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

	    filesDisplay->insertItem( *WinShell::getInfoImage(), "Starting copy process" );
	    readArchive( "build.arq", installPath->text() );

	    readArchive( "qt.arq", installPath->text() );
	    if( installDocs->isChecked() )
		readArchive( "doc.arq", installPath->text() );
	    if( installExamples->isChecked() )
		readArchive( "examples.arq", installPath->text() );
	    if( installTutorials->isChecked() )
		readArchive( "tutorial.arq", installPath->text() );
	    filesCopied = true;
	    filesDisplay->insertItem( *WinShell::getInfoImage(), "All files have been copied." );
	    filesDisplay->insertItem( *WinShell::getInfoImage(), "This log will be written to the installation directory" );
	    filesDisplay->setBottomItem( filesDisplay->count() - 1 );

	    QFile logFile( installPath->text() + "\\install.log" );
	    if( logFile.open( IO_WriteOnly ) ) {
		QTextStream outStream( &logFile );
		for( int i = filesDisplay->count() - 3; i > 0; i-- ) {
		    QString entry = filesDisplay->text( i );
		    outStream << entry.latin1();
		}
		logFile.close();
	    }
	}
	setNextEnabled( progressPage, true );
    }
    else if( newPage == configPage ) {
	QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++" );
	QByteArray pathBuffer;
	QString path;

	QEnvironment::putEnv( "QTDIR", installPath->text(), QEnvironment::LocalEnv | QEnvironment::DefaultEnv );
	QEnvironment::putEnv( "MKSPEC", mkSpecs[ sysID ], QEnvironment::LocalEnv | QEnvironment::DefaultEnv );
	QEnvironment::putEnv( "PATH", QString( "%QTDIR%\\bin;%QTDIR%\\lib;" ) + QEnvironment::getEnv( "PATH", QEnvironment::LocalEnv ), QEnvironment::LocalEnv );
	QEnvironment::putEnv( "PATH2", QString( "%QTDIR%\\bin;%QTDIR%\\lib;" ) + QEnvironment::getEnv( "PATH", QEnvironment::DefaultEnv ), QEnvironment::DefaultEnv );

	configList->clear();
	advancedList->clear();
	configList->setSorting( -1 );
	advancedList->setSorting( -1 );
	QCheckListItem* item;
	connect( &configure, SIGNAL( processExited() ), this, SLOT( configDone() ) );
	connect( &configure, SIGNAL( readyReadStdout() ), this, SLOT( readConfigureOutput() ) );
	connect( &configure, SIGNAL( readyReadStderr() ), this, SLOT( readConfigureOutput() ) );

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

	outputDisplay->insertItem( "Execute configure...\n" );

	args << QEnvironment::getEnv( "QTDIR" ) + "\\configure.bat";
	entry = settings.readEntry( "/Trolltech/Qt/Mode", &settingsOK );
	if ( entry == "Debug" )
	    args += "-debug";
	else
	    args += "-release";

	entry = settings.readEntry( "/Trolltech/Qt/Build", &settingsOK );
	if ( entry == "Static" )
	    args += "-static";
	else
	    args += "-shared";

	entry = settings.readEntry( "/Trolltech/Qt/Threading", &settingsOK );
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

	tmpFile.setName( QEnvironment::getEnv( "QTDIR" ) + "\\configcmd.bat" );
	if( tmpFile.open( IO_WriteOnly ) ) {
	    tmpStream.setDevice( &tmpFile );
	    tmpStream << args.join( " " );

	    args.clear();
	    args << QString( "CMD /V:ON /C " ) + tmpFile.name();
	    tmpFile.close();
	}

	outputDisplay->insertItem( args.join( " " ) + "\n" );
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
			filesDisplay->insertItem( *WinShell::getOpenFolderImage(), dirName );
			filesDisplay->setBottomItem( filesDisplay->count() - 1 );
		    }
		}
	    }
	    else
	    {
		QString fileName = outDir.absPath() + QString( "\\" ) + entryName;
		totalOut = 0;
		outFile.setName( fileName );
		
		if( outFile.open( IO_WriteOnly ) ) {

		    if( app ) {
			app->processEvents();
			operationProgress->setProgress( totalRead );
			filesDisplay->insertItem( *WinShell::getFileImage(), fileName );
			filesDisplay->setBottomItem( filesDisplay->count() - 1 );
		    }
		    // Try to count the files to get some sort of idea of compilation progress
		    if( ( entryName.right( 4 ) == ".cpp" ) || ( entryName.right( 2 ) == ".h" ) )
			filesToCompile++;

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

