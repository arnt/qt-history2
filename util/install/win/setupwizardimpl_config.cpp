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
#if defined(EVAL) || defined(EDU)
    prepareEnvironment();
#  if defined(Q_OS_WIN32)
    QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++ macx-g++ win32-msvc.net win32-g++ win32-watcom" );
    QStringList args;
    args << ( QEnvironment::getEnv( "QTDIR" ) + "\\bin\\configure.exe" );
    args << "-spec";
    args << mkSpecs[ globalInformation.sysId() ];
    if ( globalInformation.sysId() == GlobalInformation::MSVC )
	args << "-dsp";
    else if ( globalInformation.sysId() == GlobalInformation::MSVCNET )
	args << "-vcproj";

    if( qWinVersion() & WV_NT_based ) {
	logOutput( "Execute configure...\n" );
	logOutput( args.join( " " ) + "\n" );

	configure.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	configure.setArguments( args );
	// Start the configure process
	buildPage->compileProgress->setTotalSteps( int(double(filesToCompile) * 2.6) );
	buildPage->restartBuild->setText( "Stop configure" );
	buildPage->restartBuild->setEnabled( TRUE );
	buildPage->restartBuild->show();
	buildPage->compileProgress->show();
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
	    if ( installDir.absPath()[1] == ':' )
		outStream << installDir.absPath().left(2) << endl;
	    outStream << "cd %QTDIR%" << endl;
	    outStream << args.join( " " ) << endl;
	    if( !globalInformation.reconfig() ) {
		QStringList makeCmds = QStringList::split( ' ', "nmake make gmake make nmake mingw32-make nmake make" );
		outStream << makeCmds[ globalInformation.sysId() ].latin1() << endl;
	    }
	    outFile.close();
	}
	logOutput( "Doing the final integration steps..." );
	doFinalIntegration();
	buildPage->compileProgress->setTotalSteps( buildPage->compileProgress->totalSteps() );
	showPage( finishPage );
    }
#  elif defined(Q_OS_UNIX)
    buildPage->compileProgress->show();
    buildPage->restartBuild->show();

    buildPage->compileProgress->setProgress( 0 );
    buildPage->compileProgress->setTotalSteps( int(double(filesToCompile) * 1.8) );
    configDone();
#  endif
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

    entry = settings.readEntry( "/Trolltech/Qt/SQL Drivers/DB2", "Off", &settingsOK );
    if ( entry == "Direct" )
	args += "-qt-sql-db2";
    else if ( entry == "Plugin" )
	args += "-plugin-sql-db2";
    else if ( entry == "Off" )
	args += "-no-sql-db2";

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

    entries = settings.readListEntry( "/Trolltech/Qt/Advanced C++", ',', &settingsOK );
    if ( entries.contains( "STL" ) )
	args += "-stl";
    else
	args += "-no-stl";
    if ( entries.contains( "Exceptions" ) )
	args += "-exceptions";
    else
	args += "-no-exceptions";
    if ( entries.contains( "RTTI" ) )
	args += "-rtti";
    else
	args += "-no-rtti";

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

    if ( globalInformation.sysId() == GlobalInformation::MSVC ) {
	entry = settings.readEntry( "/Trolltech/Qt/DSP Generation", "On", &settingsOK );
	if ( entry == "On" )
	    args += "-dsp";
	else if ( entry == "Off" )
	    args += "-no-dsp";
    } else if ( globalInformation.sysId() == GlobalInformation::MSVCNET ) {
	entry = settings.readEntry( "/Trolltech/Qt/VCPROJ Generation", "On", &settingsOK );
	if ( entry == "On" )
	    args += "-vcproj";
	else if ( entry == "Off" )
	    args += "-no-vcproj";
    } else if ( globalInformation.sysId() != GlobalInformation::MSVC && globalInformation.sysId() == GlobalInformation::MSVCNET ) {
	args += "-no-dsp";
	args += "-no-vcproj";
    }

    entry = settings.readEntry( "/Trolltech/Qt/zlib", "Direct", &settingsOK );
    if ( entry == "Direct" )
	args += "-qt-zlib";
    else if ( entry == "System" )
	args += "-system-zlib";
    else if ( entry == "Off" )
	args += "-no-zlib";

    if ( ( ( !globalInformation.reconfig() && !optionsPage->skipBuild->isChecked() )
	    || ( globalInformation.reconfig() && configPage->rebuildInstallation->isChecked() ) )
#  if defined(Q_OS_WIN32)
    && qWinVersion() & WV_NT_based ) {
#  else
    ) {
#  endif
	logOutput( "Execute configure...\n" );
	logOutput( args.join( " " ) + "\n" );

	configure.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	configure.setArguments( args );
	// Start the configure process
	buildPage->compileProgress->setTotalSteps( int(double(filesToCompile) * 2.6) );
	buildPage->restartBuild->setText( "Stop configure" );
	buildPage->restartBuild->setEnabled( TRUE );
	buildPage->restartBuild->show();
	buildPage->compileProgress->show();
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
	    if ( installDir.absPath()[1] == ':' )
		outStream << installDir.absPath().left(2) << endl;
	    outStream << "cd %QTDIR%" << endl;

	    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake make nmake mingw32-make nmake make" );
	    if ( globalInformation.reconfig() )
		outStream << makeCmds[ globalInformation.sysId() ].latin1() << " clean" << endl;
	    
	    // There is a limitation on Windows 9x regarding the length of the
	    // command line. So rather use the configure.cache than specifying
	    // all configure options on the command line.
	    QFile configureCache( installDir.filePath("configure.cache") );
	    if( configureCache.open( IO_WriteOnly | IO_Translate ) ) {
		QTextStream confCacheStream( &configureCache );
		QStringList::Iterator it = args.begin();
		++it; // skip args[0] (configure)
		while ( it != args.end() ) {
		    confCacheStream << *it << endl;
		    ++it;
		}
		configureCache.close();
		outStream << args[0] << " -redo" << endl;
	    } else {
		outStream << args.join( " " ) << endl;
	    }

	    outStream << makeCmds[ globalInformation.sysId() ].latin1() << endl;
	    outFile.close();
	}
	logOutput( "Doing the final integration steps..." );
	// No need to redo the integration step
	if ( !globalInformation.reconfig() )
	    doFinalIntegration();
	buildPage->compileProgress->setTotalSteps( buildPage->compileProgress->totalSteps() );
	showPage( finishPage );
    }
#endif
}

void SetupWizardImpl::prepareEnvironment()
{
    QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++ macx-g++ win32-msvc.net win32-g++ win32-watcom" );
    QByteArray pathBuffer;
    QStringList path;
    QString qtDir;
    int envSpec = QEnvironment::LocalEnv;

    if( globalInformation.reconfig() ) {
	qtDir = QEnvironment::getEnv( "QTDIR" );
	if ( configPage ) {
	    configPage->currentInstallation->setText( qtDir );
	}
    }
    else {
	qtDir = QDir::convertSeparators( QEnvironment::getFSFileName( optionsPage->installPath->text() ) );
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
    if ( globalInformation.sysId() != GlobalInformation::Other )
	QEnvironment::putEnv( "QMAKESPEC", mkSpecs[ globalInformation.sysId() ], envSpec );
    else
	QEnvironment::putEnv( "QMAKESPEC", optionsPage->sysOtherCombo->currentText(), envSpec );
#if defined(Q_OS_WIN32)
    // MSVC 6.0 stuff --------------------------------------------------
    if( globalInformation.sysId() == GlobalInformation::MSVC ) {
	QString devdir = QEnvironment::getEnv( "MSDevDir" );
	if( !devdir.length() ) {
	    QString vsCommonDir, msDevDir, msVCDir, osDir;

	    if( QMessageBox::warning( this, "MSVC 6.0 Environment", "The Visual C++ environment variables has not been set\nDo you want to do this now?", "Yes", "No", QString::null, 0, 1 ) == 0 ) {
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

    // MSVC.NET stuff --------------------------------------------------
    if( globalInformation.sysId() == GlobalInformation::MSVCNET ) {
	QString envProductDir = QEnvironment::getEnv( "VCINSTALLDIR" );
	if ( !envProductDir.length() ) {
	    if( QMessageBox::warning( this, "MSVC.NET Environment", "The Visual C++ environment variables has not been set\nDo you want to do this now?", "Yes", "No", QString::null, 0, 1 ) == 0 ) {
		envSpec |= QEnvironment::PersistentEnv;
		persistentEnv = true;
	    }

	    // Finding
	    QString check71         = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\7.1\\Setup\\VC", "ProductDir", QEnvironment::LocalMachine );
	    QString version         = (check71.length() ? "7.1" : "7.0");
	    QString vsInstallDir    = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\"+version+"\\Setup\\VS", "EnvironmentDirectory", QEnvironment::LocalMachine );
	    QString vcInstallDir    = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\"+version+"\\Setup\\VS", "ProductDir", QEnvironment::LocalMachine );
	    QString MSVCDir         = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\"+version+"\\Setup\\VC", "ProductDir", QEnvironment::LocalMachine );
	    QString VS7CommonDir    = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\"+version+"\\Setup\\VS", "VS7CommonDir", QEnvironment::LocalMachine );
	    QString VS7CommonBinDir = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\"+version+"\\Setup\\VS", "VS7CommonBinDir", QEnvironment::LocalMachine );
	    QString FrameworkDir    = QEnvironment::getRegistryString( "Software\\Microsoft\\.NETFramework", "InstallRoot", QEnvironment::LocalMachine );
	    QString FrameworkSDKDir = QEnvironment::getRegistryString( "Software\\Microsoft\\.NETFramework", "sdkInstallRoot", QEnvironment::LocalMachine );
	    QString devEnvDir       = vsInstallDir;

	    // Adding
	    QStringList path = QStringList::split( ';', QEnvironment::getEnv( "PATH", envSpec ) );
	    path.prepend( FrameworkSDKDir + "bin" );
	    path.prepend( VS7CommonBinDir + "bin" );
	    path.prepend( VS7CommonBinDir + "bin\\prerelease" );
	    path.prepend( VS7CommonBinDir );
	    path.prepend( MSVCDir + "Bin" );
	    path.prepend( devEnvDir );

	    QStringList incl = QStringList::split( ';', QEnvironment::getEnv( "INCLUDE", envSpec ) );
	    incl.prepend( FrameworkSDKDir + "include" );
	    incl.prepend( MSVCDir + "PlatformSDK\\include" );
	    incl.prepend( MSVCDir + "PlatformSDK\\include\\prerelease" );
	    incl.prepend( MSVCDir + "Include" );
	    incl.prepend( MSVCDir + "ATLMFC\\INCLUDE" );

	    QStringList lib = QStringList::split( ';', QEnvironment::getEnv( "LIB", envSpec ) );
	    lib.prepend( FrameworkSDKDir + "lib" );
	    lib.prepend( MSVCDir + "PlatformSDK\\lib" );
	    lib.prepend( MSVCDir + "PlatformSDK\\lib\\prerelease" );
	    lib.prepend( MSVCDir + "LIB" );
	    lib.prepend( MSVCDir + "ATLMFC\\LIB" );

	    // Commiting
	    QEnvironment::putEnv( "VSINSTALLDIR", vsInstallDir, envSpec );
	    QEnvironment::putEnv( "VCINSTALLDIR", vcInstallDir, envSpec );
	    QEnvironment::putEnv( "VSCOMNTOOLS",  VS7CommonBinDir, envSpec );
	    QEnvironment::putEnv( "FrameworkDir", FrameworkDir, envSpec );
	    QEnvironment::putEnv( "FrameworkSDKDir", FrameworkSDKDir, envSpec );
	    QEnvironment::putEnv( "LIB", lib.join( ";" ), envSpec );
	    QEnvironment::putEnv( "PATH", path.join( ";" ), envSpec );
	    QEnvironment::putEnv( "INCLUDE", incl.join( ";" ), envSpec );
	}
    }

    if( qWinVersion() & WV_NT_based ) {
	SendNotifyMessageW( HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)"Environment" );
    }
#endif
}

void SetupWizardImpl::showPageConfig()
{
#if defined(EVAL) || defined(EDU)
    setBackEnabled( buildPage, false );

    static bool alreadyInitialized = FALSE;
    if ( !alreadyInitialized ) {
	configPage->installList->setSorting( -1 );

	QCheckListItem *item;
	QCheckListItem *folder;

	folder = new QCheckListItem ( configPage->installList, "Database drivers" );
	folder->setOpen( true );

	item = new QCheckListItem( folder, "DB2", QCheckListItem::CheckBox );
	item->setOn( findFile( "db2cli.dll" ) );
	db2PluginInstall = item;

#if !defined(Q_OS_MACX)
	item = new QCheckListItem( folder, "TDS", QCheckListItem::CheckBox );
	item->setOn( findFile( "ntwdblib.dll" ) );
	tdsPluginInstall = item;

	item = new QCheckListItem( folder, "Oracle (OCI)", QCheckListItem::CheckBox );
	item->setOn( findFile( "oci.dll" ) );
	ociPluginInstall = item;
#endif

	if ( globalInformation.sysId() != GlobalInformation::Borland ) {
	    // I was not able to make Postgres work with Borland
	    item = new QCheckListItem( folder, "PostgreSQL", QCheckListItem::CheckBox );
	    item->setOn( TRUE );
	    psqlPluginInstall = item;
	} else {
	    psqlPluginInstall = 0;
	}

	item = new QCheckListItem( folder, "MySQL", QCheckListItem::CheckBox );
	item->setOn( TRUE );
	mysqlPluginInstall = item;

#if !defined(Q_OS_MACX)
	item = new QCheckListItem( folder, "ODBC", QCheckListItem::CheckBox );
	item->setOn( findFile( "odbc32.dll" ) );
	odbcPluginInstall = item;
#endif

	alreadyInitialized = TRUE;
    }

    optionSelected( 0 );
#else

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
    entry = settings.readEntry( "/Trolltech/Qt/zlib", "Direct", &settingsOK );
    folder = new QCheckListItem( configPage->advancedList, "zlib" );
    folder->setOpen( true );
    zlibOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    zlibOff->setOn( entry == "Off" );
    zlibSystem = new QCheckListItem( folder, "System", QCheckListItem::RadioButton );
    zlibSystem->setOn( entry == "System" );
    zlibDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
    zlibDirect->setOn( entry == "Direct" );

    if ( globalInformation.sysId() == GlobalInformation::MSVC ) {
	entry = settings.readEntry( "/Trolltech/Qt/DSP Generation", "On", &settingsOK );
	folder = new QCheckListItem( configPage->advancedList, "DSP Generation" );
	folder->setOpen( true );
	dspOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	dspOff->setOn( entry == "Off" );
	dspOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
	dspOn->setOn( entry == "On" );
    } else if ( globalInformation.sysId() == GlobalInformation::MSVCNET ) {
	entry = settings.readEntry( "/Trolltech/Qt/VCPROJ Generation", "On", &settingsOK );
	folder = new QCheckListItem( configPage->advancedList, "VCPROJ Generation" );
	folder->setOpen( true );
	vcprojOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
	vcprojOff->setOn( entry == "Off" );
	vcprojOn = new QCheckListItem( folder, "On", QCheckListItem::RadioButton );
	vcprojOn->setOn( entry == "On" );
    }
    
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
    // PNG is required by the build system (ie. we use PNG), so don't allow it to be turned off
    pngOff->setEnabled( FALSE );
    pngPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
    pngPlugin->setOn( entry == "Plugin" );
    pngDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );	    
    pngDirect->setOn( entry == "Direct" );

    QCheckListItem *sqlfolder = new QCheckListItem( configPage->advancedList, "Sql Drivers" );
    sqlfolder->setOpen( true );

    folder = new QCheckListItem( sqlfolder, "DB2" );
    folder->setOpen( findFile( "db2cli.lib" ) && findFile( "sqlcli1.h" ) );
    entry = settings.readEntry( "/Trolltech/Qt/Sql Drivers/DB2", "Off", &settingsOK );
    db2Off = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton ); 
    db2Off->setOn( entry == "Off" );
    db2Off->setEnabled( enterprise );
    db2Plugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
    db2Plugin->setOn( entry == "Plugin" );
    db2Plugin->setEnabled( enterprise );
    db2Direct = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
    db2Direct->setOn( entry == "Direct" );
    db2Direct->setEnabled( enterprise );
    if ( !enterprise )
	db2Off->setOn( true );

    folder = new QCheckListItem( sqlfolder, "TDS" );
    folder->setOpen( findFile( "ntwdblib.lib" ) && findFile( "sqldb.h" ) );
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
    folder->setOpen( findFile( "libpqdll.lib" ) && findFile( "libpq-fe.h" ) );
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
    folder->setOpen( findFile( "oci.lib" ) && findFile( "oci.h" ) );
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
    folder->setOpen( findFile( "libmysql.lib" ) && findFile( "mysql.h" ) );
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
    folder->setOpen( findFile( "odbc32.lib" ) && findFile( "sql.h" ) );
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
    folder->setOpen( canXPStyle );
    entry = settings.readEntry( "/Trolltech/Qt/Styles/Windows XP", canXPStyle ? "Plugin" : "Off", &settingsOK );
    xpOff = new QCheckListItem( folder, "Off", QCheckListItem::RadioButton );
    xpOff->setOn( entry == "Off" || !canXPStyle );
    xpPlugin = new QCheckListItem( folder, "Plugin", QCheckListItem::RadioButton );
    xpPlugin->setOn( entry == "Plugin" && canXPStyle );
    xpPlugin->setEnabled( true );
    xpDirect = new QCheckListItem( folder, "Direct", QCheckListItem::RadioButton );
    xpDirect->setOn( entry == "Direct" && canXPStyle );
    xpDirect->setEnabled( true );

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

    entries = settings.readListEntry( "/Trolltech/Qt/Advanced C++", ',', &settingsOK );
    folder = new QCheckListItem( configPage->advancedList, "Advanced C++" );
    folder->setOpen( true );
    advancedRTTI = new QCheckListItem( folder, "RTTI", QCheckListItem::CheckBox );
    advancedRTTI->setOn( entries.contains( "RTTI" ) );
    advancedExceptions = new QCheckListItem( folder, "Exceptions", QCheckListItem::CheckBox );
    advancedExceptions->setOn( entries.contains( "Exceptions" ) );
    advancedSTL = new QCheckListItem( folder, "STL", QCheckListItem::CheckBox );
    advancedSTL->setOn( entries.contains( "STL" ) );

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
#endif
}

void SetupWizardImpl::showPageBuild()
{
    autoContTimer.stop();
    nextButton()->setText( "Next >" );
    saveSettings();

    if( globalInformation.reconfig() && configPage->rebuildInstallation->isChecked() && qWinVersion() & WV_NT_based ) {
	QStringList args;
	QStringList makeCmds = QStringList::split( ' ', "nmake make gmake make nmake mingw32-make nmake make" );

	buildPage->compileProgress->hide();
	buildPage->restartBuild->hide();

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
    } else
	cleanDone();	// We're not doing a reconfig, so skip the clean step

}

// Please note that file exists tests should also be
// added to the verifyConfig() function below..
void SetupWizardImpl::optionSelected( QListViewItem *i )
{
    if ( !i ) {
	configPage->explainOption->setText( "" );
	return;
    }

    if ( i->rtti() != QCheckListItem::RTTI )
	return;

#if defined(EVAL) || defined(EDU)
    if ( i == mysqlPluginInstall ) {
	configPage->explainOption->setText( tr(
		    "Installs the MySQL 3.x database driver."
		    ) );
    }
    if ( i == ociPluginInstall ) {
	configPage->explainOption->setText( tr(
		    "<p>Installs the Oracale Call Interface (OCI) driver.</p> "
		    "<p><font color=\"red\">Choosing this option requires "
		    "that the Oracle Client is installed and set up. "
		    "The driver depends on the oci.dll.</font></p>"
		    ) );
    }
    if ( i == odbcPluginInstall ) {
	configPage->explainOption->setText( tr(
		    "Installs the Open Database Connectivity (ODBC) driver. "
		    "This driver depends on the odbc32.dll which should be "
		    "available on all modern Windows installations."
		    ) );
    }
    if ( i == psqlPluginInstall ) {
	configPage->explainOption->setText( tr(
		    "Installs the PostgreSQL 7.1 driver. This driver can "
		    "be used to access PostgreSQL 6 databases as well "
		    "as PostgreSQL 7 databases."
		    ) );
    }
    if ( i == tdsPluginInstall ) {
	configPage->explainOption->setText( tr(
		    "Installs the TDS driver to access Sybase Adaptive "
		    "Server and Microsoft SQL Server (it is recommended "
		    "to rather use ODBC instead of TDS where applicable). "
		    "<p><font color=\"red\">Choosing this option requires "
		    "that the ntwdblib.dll is available.</font></p>"
		    ) );
    }
    if ( i == db2PluginInstall ) {
	configPage->explainOption->setText( tr(
		    "Installs the DB2 driver. This driver can "
		    "be used to access DB2 databases."
		    "<p><font color=\"red\">Choosing this option requires "
		    "that the DB2 Client is installed and set up. "
		    "The driver depends on the db2cli.dll.</font></p>"
		    ) );
    }
    return; // ### at the moment, the other options are not available in the evaluation version
#endif
    if( mysqlDirect && ( i == mysqlDirect->parent() || i == mysqlDirect || i == mysqlPlugin ) && 
	!(findFile( "libmysql.lib" ) && findFile( "mysql.h" ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The MySQL driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if( ociDirect && ( i == ociDirect->parent() || i == ociDirect || i == ociPlugin ) && 
	!(findFile( "oci.lib" ) && findFile( "oci.h" ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The OCI driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if( odbcDirect && ( i == odbcDirect->parent() || i == odbcDirect || i == odbcPlugin ) && 
	!(findFile( "odbc32.lib" ) && findFile( "sql.h" ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The ODBC driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if( psqlDirect && ( i == psqlDirect->parent() || i == psqlDirect || i == psqlPlugin ) && 
	!(findFile( "libpqdll.lib" ) && findFile( "libpq-fe.h" ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The PostgreSQL driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if ( tdsDirect && ( i == tdsDirect->parent() || i == tdsDirect || i == tdsPlugin ) &&
	!(findFile( "ntwdblib.lib" ) && findFile( "sqldb.h" ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The TDS driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if ( db2Direct && ( i == db2Direct->parent() || i == db2Direct || i == db2Plugin ) &&
	!(findFile( "db2cli.lib" ) && findFile( "sqlcli1.h" ) ) )
	QMessageBox::warning( this, "Client libraries needed", "The DB2 driver may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    if ( ( i == xpPlugin || i == xpDirect || i == xpPlugin->parent() ) && !findXPSupport() ) {
	QMessageBox::warning( this, "Platform SDK needed", "The Windows XP style requires a Platform SDK with support for\n"
							   "Windows XP to be installed properly. The required libraries and\n"
							   "headers were not found in the LIB and INCLUDE environment variable paths." );
    }
    if ( ( i == tabletOn->parent() || i == tabletOn ) && !findFile( "wintab.h" ) ) {
	QMessageBox::warning( this, "SDK required", "Qt may not build and link properly because\n"
				    "the client libraries and headers were not found in the LIB and INCLUDE environment variable paths." );
    }

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
	configPage->explainOption->setText( "Qt can be built with exception handling, STL support and RTTI support "
		                "enabled or disabled.\n"
				"The default is to disable all advanced C++ features." );
    } else if ( i == advancedSTL ) {
	configPage->explainOption->setText( "This option builds Qt with exception handling and STL support enabled. "
				"Depending on your compiler, this might cause slower execution, larger "
				"binaries, or compiler issues." );
    } else if ( i == advancedExceptions ) {
	configPage->explainOption->setText( "This option builds Qt with exception handling enabled. "
				"Depending on your compiler, this might cause slower execution, larger "
				"binaries, or compiler issues." );
    } else if ( i == advancedRTTI ) {
	configPage->explainOption->setText( "This option builds Qt with runtime type information (RTTI). "
				"Depending on your compiler, this might cause slower execution, larger "
				"binaries, or compiler issues." );
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
    } else if ( i->text(0) == "zlib" ) {
	configPage->explainOption->setText( "Qt supports the 3rd party zlib library either by compiling it into "
					    "Qt, or by linking against the library supplied with the system." );
    } else if ( i == zlibDirect ) {
	configPage->explainOption->setText( "Support for the 3rd party zlib library is compiled into Qt." );
    } else if ( i == zlibSystem ) {
	configPage->explainOption->setText( "Support for the 3rd party zlib library is provided by linking against "
					    "an existing zlib.lib" );
    } else if ( i == zlibOff ) {
	configPage->explainOption->setText( "Turn off support for the 3rd party zlib library" );
    } else if ( i->text(0) == "DSP Generation" ) {
	configPage->explainOption->setText( "qmake will generate the Visual Studio 6 project files (dsp) as well as "
					    "makefiles for the pro files when Qt is being configured." );
    } else if ( i == dspOn ) {
	configPage->explainOption->setText( "Visual Studio 6 project file (dsp) generation is turned on" );
    } else if ( i == dspOff ) {
	configPage->explainOption->setText( "Visual Studio 6 project file (dsp) generation is turned off" );
    } else if ( i->text(0) == "VCPROJ Generation" ) {
	configPage->explainOption->setText( "qmake will generate the Visual Studio .NET project files (vcproj) as well as "
					    "makefiles for the pro files when Qt is being configured." );
    } else if ( i == vcprojOn ) {
	configPage->explainOption->setText( "Visual Studio .NET project file (vcproj) generation is turned on" );
    } else if ( i == vcprojOff ) {
	configPage->explainOption->setText( "Visual Studio .NET project file (vcproj) generation is turned off" );
    }
}

bool SetupWizardImpl::verifyConfig()
{
    bool result = TRUE;
#if !defined(EVAL) && !defined(EDU)
    if( mysqlOff && !mysqlOff->isOn() && !(findFile( "libmysql.lib" ) && findFile( "mysql.h" ) ) ) {
	mysqlOff->parent()->setText( 0, mysqlOff->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
    if( ociOff && !ociOff->isOn() && !(findFile( "oci.lib" ) && findFile( "oci.h" ) ) ) {
	ociOff->parent()->setText( 0, ociOff->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
    if( odbcOff && !odbcOff->isOn() && !(findFile( "odbc32.lib" ) && findFile( "sql.h" ) ) ) {
	odbcOff->parent()->setText( 0, odbcOff->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
    if( psqlOff && !psqlOff->isOn() && !(findFile( "libpqdll.lib" ) && findFile( "libpq-fe.h" ) ) ) {
	psqlOff->parent()->setText( 0, psqlOff->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
    if( tdsOff && !tdsOff->isOn() && !(findFile( "ntwdblib.lib" ) && findFile( "sqldb.h" ) ) ) {
	tdsOff->parent()->setText( 0, tdsOff->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
    if( xpOff && !xpOff->isOn() && !findXPSupport() ) {
	xpOff->parent()->setText( 0, xpOff->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
    if ( tabletOn->isOn() && !findFile( "wintab.h" ) ) {
	tabletOn->parent()->setText( 0, tabletOn->parent()->text(0).append( " <--" ) );
	result = FALSE;
    }
#endif
    return result;
}
