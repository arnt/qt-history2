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
#include <qdatetime.h>

void SetupWizardImpl::cleanDone()
{
#if defined(EVAL)
    prepareEnvironment();
#  if defined(Q_OS_WIN32)
    if( qWinVersion() & WV_NT_based ) {
#  elif defined(Q_OS_UNIX)
    if (true) {
#  endif
	buildPage->compileProgress->setProgress( 0 );
        buildPage->compileProgress->setTotalSteps( int(double(filesToCompile) * 1.8) );
	configDone();
    } else { // no proper process handling on DOS based systems - create a batch file instead
	logOutput( "Generating batch file...\n" );
	QFile outFile( optionsPage->installPath->text() + "\\build.bat" );
	QTextStream outStream( &outFile );

	if( outFile.open( IO_WriteOnly | IO_Translate ) ) {
	    outStream << "cd %QTDIR%" << endl;
	    if( !globalInformation.reconfig() ) {
		QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
		outStream << makeCmds[ globalInformation.sysId() ].latin1() << endl;
	    }
	    outFile.close();
	}
	doFinalIntegration();
	buildPage->compileProgress->setTotalSteps( buildPage->compileProgress->totalSteps() );
	showPage( finishPage );
    }
#else
    QStringList args;
    QStringList entries;
    QSettings settings;
    QString entry;
    QStringList::Iterator it;
    QFile tmpFile;
    QTextStream tmpStream;
    bool settingsOK;

#  if defined(Q_OS_WIN32)
    args << ( QEnvironment::getEnv( "QTDIR" ) + "\\bin\\configure.exe" );
#  elif defined(Q_OS_UNIX)
    args << ( QEnvironment::getEnv( "QTDIR" ) + QDir::separator() + "configure" );
#  endif

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

#  if defined(Q_OS_WIN32)
//TODO: Win only, remove these options from wizard on mac?
    entry = settings.readEntry( "/Trolltech/Qt/Accessibility", "On", &settingsOK );
    if ( entry == "On" )
	args += "-accessibility";
    else
	args += "-no-accessibility";
#  endif

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

#  if defined(Q_OS_WIN32)
//TODO: Win only, remove these options from wizard on mac?
    entry = settings.readEntry( "/Trolltech/Qt/Image Formats/PNG", "Direct", &settingsOK );
    if ( entry == "Plugin" )
	args += "-plugin-imgfmt-png";
    else if ( entry == "Direct" )
	args += "-qt-imgfmt-png";
    else if ( entry == "Off" )
	args += "-no-imgfmt-png";
    args += "-qt-png";

    entry = settings.readEntry( "/Trolltech/Qt/Image Formats/JPEG", "Direct", &settingsOK );
    if ( entry == "Plugin" )
	args += "-plugin-imgfmt-jpeg";
    else if ( entry == "Direct" )
	args += "-qt-imgfmt-jpeg";
    else if ( entry == "Off" )
	args += "-no-imgfmt-jpeg";
    args += "-qt-jpeg";

    entry = settings.readEntry( "/Trolltech/Qt/Image Formats/MNG", "Direct", &settingsOK );
    if ( entry == "Plugin" )
	args += "-plugin-imgfmt-mng";
    else if ( entry == "Direct" )
	args += "-qt-imgfmt-mng";
    else if ( entry == "Off" )
	args += "-no-imgfmt-mng";
    args += "-qt-mng";
#  endif

    entry = settings.readEntry( "/Trolltech/Qt/Image Formats/GIF", "Direct", &settingsOK );
    if ( entry == "Direct" )
	args += "-qt-gif";
    else if ( entry == "Off" )
	args += "-no-gif";

#  if defined(Q_OS_WIN32)
//TODO: Win only, remove these options from wizard on mac?
    entry = settings.readEntry( "/Trolltech/Qt/Styles/Windows", "Direct", &settingsOK );
    if ( entry == "Direct" )
	args += "-qt-style-windows";
    else if ( entry == "Plugin" )
	args += "-plugin-style-windows";
    else if ( entry == "Off" )
	args += "-no-style-windows";

    entry = settings.readEntry( "/Trolltech/Qt/Styles/Windows XP", "Off", &settingsOK );
    if ( entry == "Direct" )
	args += "-qt-style-windowsxp";
    else if ( entry == "Plugin" )
	args += "-plugin-style-windowsxp";
    else if ( entry == "Off" )
	args += "-no-style-windowsxp";

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
#  endif

    if( optionsPage && !optionsPage->installExamples->isChecked() )
	args += "-no-examples";
    if( optionsPage && !optionsPage->installTutorials->isChecked() )
	args += "-no-tutorials";

#  if defined(Q_OS_WIN32)
    if( qWinVersion() & WV_NT_based ) {
#  elif defined(Q_OS_UNIX)
    if (true) {
#  endif
	logOutput( "Execute configure...\n" );
	logOutput( args.join( " " ) + "\n" );

	connect( &configure, SIGNAL( processExited() ), this, SLOT( configDone() ) );
	connect( &configure, SIGNAL( readyReadStdout() ), this, SLOT( readConfigureOutput() ) );
	connect( &configure, SIGNAL( readyReadStderr() ), this, SLOT( readConfigureError() ) );
	configure.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	configure.setArguments( args );
	// Start the configure process
	buildPage->compileProgress->setTotalSteps( int(double(filesToCompile) * 2.6) );
	if( !configure.start() ) {
	    logOutput( "Could not start configure process" );
	    emit wizardPageFailed( indexOf(currentPage()) );
	}
    } else { // no proper process handling on DOS based systems - create a batch file instead
	logOutput( "Generating batch file...\n" );
	QDir installDir;
	if ( optionsPage )
	    installDir.setPath( optionsPage->installPath->text() );
	else
	    installDir.setPath( QEnvironment::getEnv( "QTDIR" ) );
	QFile outFile( installDir.filePath("build.bat") );
	QTextStream outStream( &outFile );

	if( outFile.open( IO_WriteOnly | IO_Translate ) ) {
	    outStream << "cd %QTDIR%" << endl;
	    outStream << args.join( " " ) << endl;
	    if( !globalInformation.reconfig() ) {
		QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
		outStream << makeCmds[ globalInformation.sysId() ].latin1() << endl;
	    }
	    outFile.close();
	}
	doFinalIntegration();
	buildPage->compileProgress->setTotalSteps( buildPage->compileProgress->totalSteps() );
	showPage( finishPage );
    }
#endif
}

void SetupWizardImpl::prepareEnvironment()
{
    QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++ macx-g++" );
    QByteArray pathBuffer;
    QStringList path;
    QString qtDir;
    int envSpec = QEnvironment::LocalEnv;

    if( globalInformation.reconfig() ) {
	qtDir = QEnvironment::getEnv( "QTDIR" );
	if ( configPage ) {
	    configPage->currentInstLabel->show();
	    configPage->currentInstallation->show();
	    configPage->rebuildInstallation->show();
	    configPage->currentInstallation->setText( qtDir );
	}
    }
    else {
	qtDir = QDir::convertSeparators( QEnvironment::getFSFileName( optionsPage->installPath->text() ) );
	if ( configPage ) {
	    configPage->currentInstLabel->hide();
	    configPage->currentInstallation->hide();
	    configPage->rebuildInstallation->hide();
	}
    }

#if defined(Q_OS_WIN32)
    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
	// under Windows 9x, we don't compile from the installer -- so there is
	// no need to set the local environment; and doing so, results in not
	// setting the persistent since qtDir\bin is already in the PATH
	path = QStringList::split( QRegExp("[;,]"), QEnvironment::getEnv( "PATH" ) );
	if( path.findIndex( qtDir + "\\bin" ) == -1 ) {
	    path.prepend( qtDir + "\\bin" );
	    QEnvironment::putEnv( "PATH", path.join( ";" ) );
	}
    }
#elif defined(Q_OS_UNIX)
    path = QStringList::split( QRegExp("[:]"), QEnvironment::getEnv( "PATH" ) );
    if( path.findIndex( qtDir + "/bin" ) == -1 ) {
	path.prepend( qtDir + "/bin" );
	QEnvironment::putEnv( "PATH", path.join( ":" ) );
    }
    QStringList dyld = QStringList::split( QRegExp("[:]"), QEnvironment::getEnv( "DYLD_LIBRARY_PATH" ) );
    if( dyld.findIndex( qtDir + "/lib" ) == -1 ) {
	dyld.prepend( qtDir + "/lib" );
	QEnvironment::putEnv( "DYLD_LIBRARY_PATH", dyld.join( ":" ) );
    }
#endif

#if defined(Q_OS_WIN32)
    if( foldersPage && foldersPage->qtDirCheck->isChecked() ) {
	envSpec |= QEnvironment::PersistentEnv;
/*
	if( folderGroups->currentItem() == 0 )
	    envSpec |= QEnvironment::GlobalEnv;
*/
	path.clear();

	if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
	    path = QStringList::split( ';', QEnvironment::getEnv( "PATH", QEnvironment::PersistentEnv ) );
	    if( path.findIndex( qtDir + "\\bin" ) == -1 ) {
		path.prepend( qtDir + "\\bin" );
		QEnvironment::putEnv( "PATH", path.join( ";" ), QEnvironment::PersistentEnv );
	    }
	} else {
	    if( path.findIndex( qtDir + "\\bin" ) == -1 ) {
		QEnvironment::putEnv( "PATH", qtDir + "\\bin;%PATH%", QEnvironment::PersistentEnv );
	    }
	}
    }
#elif defined(Q_OS_UNIX)
//Persistent environment not supported
#endif

    QEnvironment::putEnv( "QTDIR", qtDir, envSpec );
    QEnvironment::putEnv( "QMAKESPEC", mkSpecs[ globalInformation.sysId() ], envSpec );

#if defined(Q_OS_WIN32)
    if( globalInformation.sysId() == GlobalInformation::MSVC ) {
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
	    if( optionsPage && path.findIndex( optionsPage->installPath->text() + "\\bin" ) == -1 )
		path.prepend( optionsPage->installPath->text() + "\\bin" );
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
#endif
}

void SetupWizardImpl::showPageConfig()
{
    // First make sure that the current license information is saved
    if( !globalInformation.reconfig() ) {
	writeLicense( QDir::homeDirPath() + "/.qt-license" );
    }

    prepareEnvironment();

    bool enterprise = licenseInfo[ "PRODUCTS" ] == "qt-enterprise";

    if( configPage->configList->childCount() ) {
	QListViewItem* current = configPage->configList->firstChild();

	while( current ) {
	    QListViewItem* next = current->nextSibling();
	    delete current;
	    current = next;
	}

	current = configPage->advancedList->firstChild();
	while( current ) {
	    QListViewItem* next = current->nextSibling();
	    delete current;
	    current = next;
	}
    }
    QSettings settings;
    configPage->configList->setSorting( -1 );
    configPage->advancedList->setSorting( -1 );
    QCheckListItem *item;
    QCheckListItem *folder;
    QStringList::Iterator it;

    // general
    folder = new QCheckListItem ( configPage->configList, "Modules" );
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

    
    folder = new QCheckListItem ( configPage->configList, "Threading" );
    folder->setOpen( true );
    QString entry = settings.readEntry( "/Trolltech/Qt/Threading", "Threaded", &settingsOK );
    item = new QCheckListItem( folder, "Threaded", QCheckListItem::RadioButton );
    item->setOn( entry == "Threaded" );
    item = new QCheckListItem( folder, "Non-threaded", QCheckListItem::RadioButton );
    item->setOn( entry == "Non-threaded" );

    folder = new QCheckListItem ( configPage->configList, "Library" );
    folder->setOpen( true );
    entry = settings.readEntry( "/Trolltech/Qt/Library", "Shared", &settingsOK );
    staticItem = new QCheckListItem( folder, "Static", QCheckListItem::RadioButton );
    staticItem->setOn( entry == "Static" );
    item = new QCheckListItem( folder, "Shared", QCheckListItem::RadioButton );
    item->setOn( entry == "Shared" );

    folder = new QCheckListItem ( configPage->configList, "Build" );
    folder->setOpen( true );
    entry = settings.readEntry( "/Trolltech/Qt/Build", "Release", &settingsOK );
    item = new QCheckListItem( folder, "Debug", QCheckListItem::RadioButton );
    item->setOn( entry == "Debug" );
    item = new QCheckListItem( folder, "Release", QCheckListItem::RadioButton );	
    item->setOn( entry == "Release" );

    // Advanced options
    QCheckListItem *imfolder = new QCheckListItem( configPage->advancedList, "Image Formats" );
    imfolder->setOpen( true );

    folder = new QCheckListItem( imfolder, "GIF" );
    folder->setOpen( true );
    entry = settings.readEntry( "/Trolltech/Qt/Image Formats/GIF", "Off", &settingsOK );
    gifOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    gifOff->setOn( entry == "Off" );
    gifDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
    gifDirect->setOn( entry == "Direct" );

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

    QCheckListItem *sqlfolder = new QCheckListItem( configPage->advancedList, "Sql Drivers" );
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

    QCheckListItem *stfolder = new QCheckListItem( configPage->advancedList, "Styles" );
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
    motifOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    motifOff->setOn( entry == "Off" );
    motifPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
    motifPlugin->setOn( entry == "Plugin" );
    motifDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
    motifDirect->setOn( entry == "Direct" );

    bool canXPStyle = findXPSupport();
    folder = new QCheckListItem( stfolder, "Windows XP" );
    folder->setOpen( true );
    entry = settings.readEntry( "/Trolltech/Qt/Styles/Windows XP", canXPStyle ? "Plugin" : "Off", &settingsOK );
    xpOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    xpOff->setOn( entry == "Off" );
    xpPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
    xpPlugin->setOn( entry == "Plugin" && canXPStyle );
    xpPlugin->setEnabled( canXPStyle );
    xpDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
    xpDirect->setOn( entry == "Direct" );
    xpDirect->setEnabled( false );

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
    folder = new QCheckListItem( configPage->advancedList, "Advanced C++" );
    folder->setOpen( true );
    advancedCppOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    advancedCppOff->setOn( entry == "Off" );
    advancedCppOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
    advancedCppOn->setOn( entry == "On" );

    folder = new QCheckListItem( configPage->advancedList, "Tablet Support" );
    folder->setOpen( true );
    entry = settings.readEntry( "/Trolltech/Qt/Tablet Support", "Off", &settingsOK );
    tabletOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    tabletOff->setOn( entry == "Off" );
    tabletOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
    tabletOn->setOn( entry == "On" );

    folder = new QCheckListItem( configPage->advancedList, "Accessibility" );
    folder->setOpen( true );
    entry = settings.readEntry( "/Trolltech/Qt/Accessibility", "On", &settingsOK );
    accOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    accOff->setOn( entry == "Off" );
    accOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
    accOn->setOn( entry == "On" );

    entry = settings.readEntry( "/Trolltech/Qt/Big Textcodecs", "On", &settingsOK );
    folder = new QCheckListItem( configPage->advancedList, "Big Textcodecs" );
    folder->setOpen( true );
    bigCodecsOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    bigCodecsOff->setOn( entry == "Off" );
    bigCodecsOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );	
    bigCodecsOn->setOn( entry == "On" );

    optionSelected( 0 );

    setStaticEnabled( staticItem->isOn() );
    setJpegDirect( mngDirect->isOn() );

    setBackEnabled( buildPage, false );
}

void SetupWizardImpl::showPageBuild()
{
    QStringList args;
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake make" );

    autoContTimer.stop();
    nextButton()->setText( "Next >" );
    saveSettings();

    if( globalInformation.reconfig() ) {
	buildPage->compileProgress->hide();

    	args << makeCmds[ globalInformation.sysId() ] << "clean";
	logOutput( "Starting cleaning process" );
	connect( &cleaner, SIGNAL( processExited() ), this, SLOT( cleanDone() ) );
	connect( &cleaner, SIGNAL( readyReadStdout() ), this, SLOT( readCleanerOutput() ) );
	connect( &cleaner, SIGNAL( readyReadStderr() ), this, SLOT( readCleanerError() ) );
	cleaner.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	cleaner.setArguments( args );
	if( !cleaner.start() ) {
	    logOutput( "Could not start cleaning process" );
	    emit wizardPageFailed( indexOf(currentPage()) );
	}
    }
    else
	cleanDone();	// We're not doing a reconfig, so skip the clean step

}

void SetupWizardImpl::optionSelected( QListViewItem *i )
{
    if ( !i ) {
	configPage->explainOption->setText( tr("Change the configuration.") );
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
    if ( i == xpDirect )
	QMessageBox::warning( this, "Unsupported configuration", "The Windows XP style requires XP components and\n"
								 "can only be used as a plugin with a shared Qt DLL." );
    if ( i == xpPlugin && !findXPSupport() )
	QMessageBox::warning( this, "Platform SDK needed", "The Windows XP style requires a Platform SDK with support for\n"
							   "Windows XP to be installed properly. The required libraries and\n"
							   "headers were not found in the LIB and INCLUDE environment variable paths." );

    if ( i->text(0) == "Required" ) {
	configPage->explainOption->setText( tr("These modules are a necessary part of the Qt library. "
				   "They can not be disabled.") );
    } else if ( i->parent() && i->parent()->text(0) == "Required" ) {
	configPage->explainOption->setText( tr("This module is a necessary part of the Qt library. "
				   "It can not be disabled.") ); 
    } else if ( i->text(0) == "Modules" ) {
	configPage->explainOption->setText( tr("Some of these modules are optional. "
				   "You can deselect the modules that you "
				   "don't require for your development.\n"
				   "By default, all modules are selected.") );
    } else if ( i->parent() && i->parent()->text(0) == "Modules" ) {
	QString moduleText;
	// ### have some explanation of the module here
	configPage->explainOption->setText( tr("Some of these modules are optional. "
				   "You can deselect the modules that you "
				   "don't require for your development.\n"
				   "By default, all modules are selected.") );
    } else if ( i->text(0) == "Threading" ) {
	configPage->explainOption->setText( tr("Build the Qt library with or without thread support. "
			    "By default, threading is supported.") );
    } else if ( i->parent() && i->parent()->text(0) == "Threading" ) {
	if ( i->text(0) == "Threaded" ) {
	    configPage->explainOption->setText("Select this option if you want to be able to use threads "
				   "in your application.");
	} else {
	    configPage->explainOption->setText("Select this option if you do not need thread support.\n"
				   "Some classes will not be available without thread support.");
	}
    } else if ( i->text(0) == "Build" ) {
	configPage->explainOption->setText( tr("<p>Use the debug build of the Qt library to enhance "
				   "debugging of your application. The release build "
				   "is both smaller and faster.</p>") );
    } else if ( i->text(0) == "Debug" ) {
	configPage->explainOption->setText( tr("<p>A debug build will include debug information in the "
	                           "Qt library and tools.  This will facilitate debugging "
				   "of your application.</p>") );
    } else if ( i->text(0) == "Release" ) {
	configPage->explainOption->setText( tr("<p>A release build will build the library and tools without "
				   "debugging information.  The resulting code is suited for "
				   "a release, and is both smaller and faster.</p>") );
    } else if ( i->text(0) == "Library" ) {
	configPage->explainOption->setText( "Build a static or a shared Qt library." );
    } else if ( i->parent() && i->parent()->text( 0 ) == "Library" ) {
	if ( i->text(0) == "Static" ) {
	    configPage->explainOption->setText( tr("<p>Build the Qt library as a static library."
				       "All applications created with a static "
				       "library will be at least 1.5MB big.</p>"
				       "<p><font color=\"red\">It is not possible to "
				       "build or use any components or plugins with a "
				       "static Qt library!</font></p>") );
	} else {
	    configPage->explainOption->setText("<p>A shared Qt library makes it necessary to "
				   "distribute the Qt DLL together with your software.</p>"
				   "<p>Applications and libraries linked against a shared Qt library "
				   "are small and can make use of components and plugins.</p>" );
	}
    } else if ( i->text( 0 ) == "Sql Drivers" ) {
	configPage->explainOption->setText("Select the SQL Drivers you want to support. "
				"<font color=#FF0000>You must have the appropriate client libraries "
				"and header files installed correctly before you can build the Qt SQL drivers.</font>" );
    } else if ( ( i->parent() && i->parent()->text( 0 )== "Sql Drivers" )
	||  ( i->parent() && i->parent()->parent() && i->parent()->parent()->text( 0 )== "Sql Drivers" ) ) {
	configPage->explainOption->setText( "Select the SQL Drivers you want to support. "
			        "<font color=#FF0000>You must have the appropriate client libraries "
				"and header files installed correctly before you can build the Qt SQL drivers.</font> "
				"Selected SQL Drivers will be integrated into Qt. \nYou can also "
				"build every SQL Driver as a plugin to be more flexible for later "
				"extensions. Building a plugin is not supported in the installer at "
				"this point. You will have to do it manually after the installation "
				"succeeded. Read the help files in QTDIR\\plugins\\src\\sqldrivers for "
				"further instructions." );
    } else if ( i->text( 0 ) == "Styles" ) {
	configPage->explainOption->setText( QString("Select support for the various GUI styles that Qt has emulation for." ) );
    } else if ( ( i->parent() && i->parent()->text( 0 ) == "Styles" ) 
	||  ( i->parent() && i->parent()->parent() && i->parent()->parent()->text( 0 )== "Styles" ) ) {
	QString styleName = (i->parent()->text( 0 ) == "Styles") ? i->text( 0 ) : i->parent()->text( 0 );
	if ( styleName == "Windows" ) {
	    configPage->explainOption->setText( QString("The %1 style is builtin to the Qt library.").arg( styleName ) );
	} else {
	    configPage->explainOption->setText( QString("Selects support for the %1 style. The style can be built "
				    "as a plugin, or builtin to the library, or not at all.").arg( styleName ) );
	}
    } else if ( i == accOff ) {
	configPage->explainOption->setText( "Turns off accessibility support. People who need accessibility "
				"clients will not be able to use your software." );
    } else if ( i == accOn ) {
	configPage->explainOption->setText( "Enables your Qt software to be used with accessibility clients, "
				"like a magnifier or narrator tool. The accessibility information "
				"for the Qt widgets is provided by a plugin on demand and can be "
				"exchanged or extended easily." );
    } else if ( i->text(0) == "Accessibility" ) {
	configPage->explainOption->setText( "Accessibility means making software usable and accessible to a wide "
				"range of users, including those with disabilities.\n"
				"This feature relies on components and is not available with a static "
				"Qt library." );
    } else if ( i == bigCodecsOff ) {
	configPage->explainOption->setText( "All big textcodecs are provided by plugins and get loaded on demand." );
    } else if ( i == bigCodecsOn ) {
	configPage->explainOption->setText( "The big textcodecs are compiled into the Qt library." );
    } else if ( i->text(0) == "Big Textcodecs" ) {
	configPage->explainOption->setText( "Textcodecs provide translations between text encodings. For "
				"languages and script systems with many characters it is necessary "
				"to have big data tables that provide the translation. Those codecs "
				"can be left out of the Qt library and will be loaded on demand.\n"
				"Having the codecs in a plugin is not available with a static Qt "
				"library." );
    } else if ( i->text(0) == "Tablet Support" ) {
	configPage->explainOption->setText( "Qt can support the Wacom brand tablet device." );
    } else if ( i == tabletOff ) {
	configPage->explainOption->setText( "Support for the Wacom tablet is disabled. This is the default option." );
    } else if ( i == tabletOn ) {
	configPage->explainOption->setText( "This option builds in support for Wacom(c) tablets.\n"
				"To use a supported tablet, you must have built the Wintab SDK available "
				"at http://www.pointing.com/FTP.HTM and have your INCLUDE and LIBRARY path "
				"set appropriately." );
    } else if ( i->text(0) == "Advanced C++" ) {
	configPage->explainOption->setText( "Qt can be built with exception handling and STL support enabled or "
				"disabled. The default is to disable advanced C++ features." );
    } else if ( i == advancedCppOn ) {
	configPage->explainOption->setText( "This option builds Qt with exception handling and STL support enabled. "
				"Depending on your compiler, this might cause slower execution, larger "
				"binaries, or compiler issues." );
    } else if ( i == advancedCppOff ) {
	configPage->explainOption->setText( "This option turns advanced C++ features off when building Qt." );
    } else if ( i->text(0) == "Image Formats" ) {
	configPage->explainOption->setText( "Qt ships with support for a wide range of common image formats. "
				"Standard formats are always included in Qt, and some more special formats "
				"can be left out from the Qt library itself and provided by a plugin instead." );
    } else if ( i == gifOff ) {
	configPage->explainOption->setText( "Turn off support for GIF images." );
    } else if ( i == gifDirect ) {
	configPage->explainOption->setText( "<p>Support for GIF images is compiled into Qt.</p>"
				"<p><font color=\"red\">If you are in a country "
				"which recognizes software patents and in which "
				"Unisys holds a patent on LZW compression and/or "
				"decompression and you want to use GIF, Unisys "
				"may require you to license the technology. Such "
				"countries include Canada, Japan, the USA, "
				"France, Germany, Italy and the UK.</font></p>" );
    } else if ( i->text(0) == "GIF" ) {
	configPage->explainOption->setText( "Qt supports the \"Graphics Interchange Format\".");
    } else if ( i == mngPlugin ) {
	configPage->explainOption->setText( "Support for MNG images is provided by a plugin that is loaded on demand." );
    } else if ( i == mngOff ) {
	configPage->explainOption->setText( "Turn off support for MNG images." );
    } else if ( i == mngDirect ) {
	configPage->explainOption->setText( "Support for MNG images is compiled into Qt. This requires JPEG support compiled into Qt as well." );
#if 0
    } else if ( i == mngPresent ) {
	configPage->explainOption->setText( "Support for MNG images is provided by linking against an existing libmng." );
#endif
    } else if ( i->text(0) == "MNG" ) {
	configPage->explainOption->setText( "Qt supports the \"Multiple-Image Network Graphics\" format either "
				"by compiling the mng sources "
				"into Qt, or by loading a plugin on demand." );
    } else if ( i == jpegPlugin ) {
	configPage->explainOption->setText( "Support for JPEG images is provided by a plugin that is loaded on demand." );
    } else if ( i == jpegOff ) {
	configPage->explainOption->setText( "Turn off support for JPEG images." );
    } else if ( i == jpegDirect ) {
	configPage->explainOption->setText( "Support for JPEG images is compiled into Qt." );
#if 0
    } else if ( i == jpegPresent ) {
	configPage->explainOption->setText( "Support for JPEG images is provided by linking against an existing libjpeg." );
#endif
    } else if ( i->text(0) == "JPEG" ) {
	configPage->explainOption->setText( "Qt supports the \"Joint Photographic Experts Group\" format either "
				"by compiling the jpeg sources "
				"into Qt, or by loading a plugin on demand." );
    } else if ( i == pngPlugin ) {
	configPage->explainOption->setText( "Support for PNG images is provided by a plugin that is loaded on demand." );
    } else if ( i == pngOff ) {
	configPage->explainOption->setText( "<p>Turn off support for PNG images.</p>"
				"<p><font color=\"red\">Qt Designer, Qt Assistant and Qt Linguist use "
				"the imageformat PNG. If you choose this option, the images in these "
				"programs will be missing.</font></p>" );
    } else if ( i == pngDirect ) {
	configPage->explainOption->setText( "Support for PNG images is compiled into Qt." );
#if 0
    } else if ( i == pngPresent ) {
	configPage->explainOption->setText( "Support for PNG images is provided by linking against an existing libpng." );
#endif
    } else if ( i->text(0) == "PNG" ) {
	configPage->explainOption->setText( "Qt supports the \"Portable Network Graphics\" format either "
				"by compiling the png support "
				"into Qt, or by loading a plugin on demand." );
    }
}
