#include <qtextstream.h>
#include <qregexp.h>
#include "configureapp.h"
#include "../../install/environment.h"

#include <iostream.h>

ConfigureApp::ConfigureApp( int& argc, char** argv ) : QApplication( argc, argv )
{
    int i;
    qtDir = QEnvironment::getEnv( "QTDIR" );

    if( !qtDir.length() ) {
	cout << "QTDIR is not defined, defaulting to current directory." << endl;
	qtDir = QDir::currentDirPath();
	QEnvironment::putEnv( "QTDIR", qtDir );
    }

    /*
    ** Set up the initial state, the default
    */
    dictionary[ "CONFIGCMD" ] = argv[ 0 ];

    for( i = 1; i < argc; i++ )
	configCmdLine += argv[ i ];

    dictionary[ "QCONFIG" ] = "full";
    dictionary[ "EMBEDDED" ] = "no";
    dictionary[ "DEBUG" ] = "no";
    dictionary[ "SHARED" ] = "yes";
    dictionary[ "GIF" ] = "no";
    dictionary[ "THREAD" ] = "no";
    dictionary[ "ZLIB" ] = "yes";
    dictionary[ "LIBPNG" ] = "yes";
    dictionary[ "JPEG" ] = "no";
    dictionary[ "MNG" ] = "no";
    dictionary[ "BUILD_QMAKE" ] = "yes";
    dictionary[ "DSPFILES" ] = "yes";
    dictionary[ "QMAKESPEC" ] = QEnvironment::getEnv( "QMAKESPEC" );
    dictionary[ "QMAKE_INTERNAL" ] = "yes";
    dictionary[ "LEAN" ] = "no";
    dictionary[ "STL" ] = "no";

    QString tmp = QEnvironment::getEnv( "QMAKESPEC" );
    tmp = tmp.mid( tmp.findRev( "\\" ) + 1 );
    dictionary[ "QMAKESPEC" ] = tmp;

    buildModulesList();
    buildSqlList();

    QObject::connect( &qmakeBuilder, SIGNAL( readyReadStdout() ), this, SLOT( readQmakeBuilderOutput() ) );
    QObject::connect( &qmakeBuilder, SIGNAL( readyReadStderr() ), this, SLOT( readQmakeBuilderError() ) );
    QObject::connect( &qmakeBuilder, SIGNAL( processExited() ), this, SLOT( qmakeBuilt() ) );
//    QObject::connect( &qmake, SIGNAL( readyReadStdout() ), this, SLOT( readQmakeOutput() ) );
//    QObject::connect( &qmake, SIGNAL( readyReadStderr() ), this, SLOT( readQmakeError() ) );
    QObject::connect( &qmake, SIGNAL( processExited() ), this, SLOT( qmakeDone() ) );
}

void ConfigureApp::buildModulesList()
{
    QDir dir( qtDir + "/src" );
    const QFileInfoList* fiList = dir.entryInfoList();
    QFileInfoListIterator listIter( *fiList );
    QFileInfo* fi;

    allModules = QStringList::split( ' ', "styles tools kernel widgets dialogs iconview workspace" );
    
    readLicense();
    
    if( ( licenseInfo[ "PRODUCTS" ].length() ) && ( licenseInfo[ "PRODUCTS" ] != "qt-professional" ) )
	allModules += QStringList::split( ' ', "network canvas table xml opengl sql" );

    while( ( fi = listIter.current() ) ) {
	if( allModules.findIndex( fi->fileName() ) != -1 )
	    modules += fi->fileName();
	++listIter;
    }
}

void ConfigureApp::buildSqlList()
{
    QDir dir( qtDir + "/src/sql/src" );
    const QFileInfoList* fiList = dir.entryInfoList();
    QFileInfoListIterator listIter( *fiList );
    QFileInfo* fi;

    allSqlDrivers = QStringList::split( ' ', "mysql oci odbc psql" );

    while( ( fi = listIter.current() ) ) {
	if( allSqlDrivers.findIndex( fi->fileName() ) != -1 )
	    sqlDrivers += fi->fileName();
	++listIter;
    }
}

void ConfigureApp::parseCmdLine()
{
    for( QStringList::Iterator args = configCmdLine.begin(); args != configCmdLine.end(); ++args ) {
	if( (*args) == "-help" )
	    dictionary[ "HELP" ] = "yes";
	else if( (*args) == "-?" )
	    dictionary[ "HELP" ] = "yes";
	else if( (*args) == "-qconfig" ) {
	    ++args;
	    dictionary[ "QCONFIG" ] = (*args);
	}
	else if( (*args) == "-release" )
	    dictionary[ "DEBUG" ] = "no";
	else if( (*args) == "-debug" )
	    dictionary[ "DEBUG" ] = "yes";
	else if( (*args) == "-shared" )
	    dictionary[ "SHARED" ] = "yes";
	else if( (*args) == "-static" )
	    dictionary[ "SHARED" ] = "no";
	else if( (*args) == "-no-thread" )
	    dictionary[ "THREAD" ] = "no";
	else if( (*args) == "-thread" )
	    dictionary[ "THREAD" ] = "yes";
	else if( (*args) == "-platform" ) {
	    ++args;
	    dictionary[ "QMAKESPEC" ] = (*args);
	}
	else if( (*args) == "-no-gif" )
	    dictionary[ "GIF" ] = "no";
	else if( (*args) == "-qt-gif" )
	    dictionary[ "GIF" ] = "yes";
	else if( (*args) == "-qt-zlib" )
	    dictionary[ "ZLIB" ] = "yes";
	else if( (*args) == "-system-zlib" )
	    dictionary[ "ZLIB" ] = "no";
	else if( (*args) == "-qt-libpng" )
	    dictionary[ "LIBPNG" ] = "yes";
	else if( (*args) == "-system-libpng" )
	    dictionary[ "LIBPNG" ] = "no";
	else if( (*args) == "-no-mng" )
	    dictionary[ "MNG" ] = "no";
	else if( (*args) == "-system-mng" )
	    dictionary[ "MNG" ] = "yes";
	else if( (*args) == "-no-jpeg" )
	    dictionary[ "JPEG" ] = "no";
	else if( (*args) == "-system-jpeg" )
	    dictionary[ "JPEG" ] = "yes";
	else if( (*args) == "-internal" )
	    dictionary[ "QMAKE_INTERNAL" ] = "yes";
	else if( (*args) == "-no-qmake" )
	    dictionary[ "BUILD_QMAKE" ] = "no";
	else if( (*args) == "-D" ) {
	    ++args;
            qmakeDefines += (*args);
        }
	else if( (*args) == "-I" ) {
	    ++args;
	    qmakeIncludes += (*args);
	}
	else if( (*args) == "-L" ) {
	    ++args;
	    qmakeLibs += (*args);
	}
	else if( (*args) == "-no-dsp" )
	    dictionary[ "DSPFILES" ] = "no";
	else if( (*args) == "-dsp" )
	    dictionary[ "DSPFILES" ] = "yes";
	else if( (*args) == "-lean" )
	    dictionary[ "LEAN" ] = "yes";
	else if( (*args) == "-stl" )
	    dictionary[ "STL" ] = "yes";
	else if( (*args) == "-no-stl" )
	    dictionary[ "STL" ] = "no";

	// Scan to see if any specific modules and drivers are enabled or disabled
	for( QStringList::Iterator module = modules.begin(); module != modules.end(); ++module ) {
	    if( (*args) == QString( "-enable-" ) + (*module) )
		qmakeConfig += (*module);
	    if( (*args) == QString( "-disable-" ) + (*module) )
		qmakeConfig.remove( (*module) );
	}
	for( QStringList::Iterator sql = sqlDrivers.begin(); sql != sqlDrivers.end(); ++sql ) {
	    if( (*args) == QString( "-sql-" ) + (*sql) )
		qmakeSql += (*sql);
	}

    }
    if( dictionary[ "QMAKE_INTERNAL" ] == "yes" )
	qmakeConfig += modules;
}

void ConfigureApp::validateArgs()
{
    QStringList configs;
    // Validate the specified config

    allConfigs = QStringList::split( ' ', "minimal small medium large full" );

    for( QStringList::Iterator config = allConfigs.begin(); config != allConfigs.end(); ++config ) {
	configs += (*config) + "-config";
	if( (*config) == dictionary[ "QCONFIG" ] )
	    break;
    }
    if( config == allConfigs.end() ) {
	dictionary[ "HELP" ] = "yes";
	cout << "No such configuration \"" << dictionary[ "QCONFIG" ].latin1() << "\"" << endl ;
    }
    else
	qmakeConfig += configs;

}

bool ConfigureApp::displayHelp()
{
    if( dictionary[ "HELP" ] == "yes" ) {
	cout << endl << endl;
	cout << "Command line arguments:  (* indicates default behaviour)" << endl << endl;
	cout << "-help               Bring up this help text." << endl;
	cout << "-debug              Enable debug information." << endl;
	cout << "-release          * Disable debug information." << endl;
	cout << "-shared           * Build Qt as a shared library." << endl;
	cout << "-static             Build Qt as a static library." << endl;
	cout << "-thread             Configure Qt with thread support." << endl;
	cout << "-no-thread        * Configure Qt without thread support." << endl;
	cout << "-platform           Specify a platform, uses %QMAKESPEC% as default." << endl;
	cout << "-qconfig            Specify config, available configs:" << endl;
	for( QStringList::Iterator config = allConfigs.begin(); config != allConfigs.end(); ++config )
	    cout << "                        " << (*config).latin1() << endl;
	cout << "-qt-gif             Enable GIF support." << endl;
	cout << "-no-gif           * Disable GIF support." << endl;
	cout << "-qt-zlib          * Compile in zlib." << endl;
	cout << "-system-zlib        Use existing zlib in system." << endl;
	cout << "-qt-libpng        * Compile in libPNG." << endl;
	cout << "-system-libpng      Use existing libPNG in system." << endl;
	cout << "-no-mng           * Disable MNG support." << endl;
	cout << "-system-mng         Enable MNG support." << endl;
	cout << "-no-jpeg          * Disable JPEG support." << endl;
	cout << "-system-jpeg        Enable JPEG support." << endl << endl;
	cout << "-stl                Enable STL support." << endl;
	cout << "-no-stl           * Disable STL support." << endl << endl;
	cout << "-no-dsp             Disable the generation of VC++ .DSP-files." << endl;
	cout << "-dsp                Enable the generation of VC++ .DSP-files." << endl;
	cout << "-lean               Only process the Qt core projects." << endl;
	cout << "                    (qt.pro, qtmain.pro)." << endl;
	cout << "-D <define>         Add <define> to the list of defines." << endl;
	cout << "-I <includepath>    Add <includepath> to the include searchpath." << endl;
	cout << "-L <libpath>        Add <libpath> to the library searchpath." << endl;
	cout << "-enable-*           Enable the specified module, where module is one of" << endl;
	cout << "                    " << modules.join( " " ) << endl;
	cout << "-disable-*          Disable the specified module, where module is one of" << endl;
	cout << "                    " << modules.join( " " ) << endl << endl;
	cout << "-sql-*              Compile the specified SQL driver." << endl << endl;
	return true;
    }
    return false;
}

void ConfigureApp::generateOutputVars()
{
    // Generate variables for output
    if( dictionary[ "DEBUG" ] == "yes" ) {
	qmakeConfig += "debug";
	dictionary[ "QMAKE_OUTDIR" ] = "debug";
    }
    else {
	qmakeConfig += "release";
	dictionary[ "QMAKE_OUTDIR" ] = "release";
    }
    if( dictionary[ "THREAD" ] == "yes" ) {
	qmakeConfig += "thread";
	dictionary[ "QMAKE_OUTDIR" ] += "_mt";
    }
    if( dictionary[ "SHARED" ] == "yes" ) {
	dictionary[ "QMAKE_OUTDIR" ] += "_shared";
	qmakeDefines += "QT_DLL";
    }
    else
	dictionary[ "QMAKE_OUTDIR" ] += "_static";
    if( dictionary[ "STL" ] == "no" ) {
	qmakeDefines += "QT_NO_STL";
    }

    qmakeVars += QString( "QMAKE_LIBDIR_QT=" ) + QDir::convertSeparators( qtDir + "/lib" );
    qmakeVars += QString( "OBJECTS_DIR=" ) + QDir::convertSeparators( "tmp/obj/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "MOC_DIR=" ) + QDir::convertSeparators( "tmp/moc/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "sql-drivers+=" ) + qmakeSql.join( " " );
    qmakeVars += QString( "DEFINES+=" ) + qmakeDefines.join( " " );
    if( licenseInfo[ "PRODUCTS" ].length() )
	qmakeVars += QString( "QT_PRODUCT=" ) + licenseInfo[ "PRODUCTS" ];

    if( dictionary[ "JPEG" ] == "yes" )
	qmakeConfig += "jpeg";
    if( dictionary[ "MNG" ] == "yes" )
	qmakeConfig += "mng";
    if( dictionary[ "GIF" ] == "yes" )
	qmakeConfig += "gif";
    if( dictionary[ "ZLIB" ] == "yes" )
	qmakeConfig += "zlib";
    if( dictionary[ "LIBPNG" ] == "yes" )
	qmakeConfig += "libpng";

    if( !dictionary[ "QMAKESPEC" ].length() ) {
	cout << "QMAKESPEC must either be defined as an environment variable, or specified" << endl;
	cout << "as an argument with -spec" << endl;
	dictionary[ "HELP" ] = "yes";

	QStringList winPlatforms;
	QDir mkspecsDir( qtDir + "\\mkspecs" );
	const QFileInfoList* specsList = mkspecsDir.entryInfoList();
	QFileInfoListIterator it( *specsList );
	QFileInfo* fi;

	while( ( fi = it.current() ) ) {
	    if( fi->fileName().left( 5 ) == "win32" ) {
		winPlatforms += fi->fileName();
	    }
	    ++it;
	}
	cout << "Available platforms are: " << winPlatforms.join( ", " ).latin1() << endl;
    }
}

void ConfigureApp::generateCachefile()
{    
    // Generate .qmake.cache
    QFile cacheFile( qtDir + "\\.qmake.cache" );
    if( cacheFile.open( IO_WriteOnly ) ) { // Truncates any existing file.
	QTextStream cacheStream( &cacheFile );
        for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
	    cacheStream << (*var) << endl;
	}
	cacheStream << "CONFIG=" << qmakeConfig.join( " " ) << endl;
	cacheStream << "QMAKESPEC=" << dictionary[ "QMAKESPEC" ] << endl;
	if( !qmakeIncludes.isEmpty() ) {
	    cacheStream << "INCLUDEPATH=";
	    for( QStringList::Iterator incs = qmakeIncludes.begin(); incs != qmakeIncludes.end(); ++incs )
		cacheStream << (*incs);
	    cacheStream << endl;
	}
	if( !qmakeLibs.isEmpty() ) {
	    cacheStream << "LIBPATH=";
	    for( QStringList::Iterator libs = qmakeLibs.begin(); libs != qmakeLibs.end(); ++libs )
		cacheStream << (*libs);
	    cacheStream << endl;
	}
	cacheFile.close();
    }
    // Generate shadow .qmake.cache file in src/
    // This is to avoid problems on Windows
    QStringList srcConfig = qmakeConfig;
    if( dictionary[ "SHARED" ] == "yes" )
	srcConfig += "dll";
    else
	srcConfig += "staticlib";
    
    cacheFile.setName( qtDir + "\\src\\.qmake.cache" );
    if( cacheFile.open( IO_WriteOnly ) ) { // Truncates any existing file.
	QTextStream cacheStream( &cacheFile );
	for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
	    cacheStream << (*var) << endl;
	}
	cacheStream << "CONFIG=" << srcConfig.join( " " ) << endl;
	cacheStream << "QMAKESPEC=" << dictionary[ "QMAKESPEC" ] << endl;
	if( !qmakeIncludes.isEmpty() ) {
	    cacheStream << "INCLUDEPATH=";
	    for( QStringList::Iterator incs = qmakeIncludes.begin(); incs != qmakeIncludes.end(); ++incs )
		cacheStream << (*incs);
	    cacheStream << endl;
	}
	if( !qmakeLibs.isEmpty() ) {
	    cacheStream << "LIBPATH=";
	    for( QStringList::Iterator libs = qmakeLibs.begin(); libs != qmakeLibs.end(); ++libs )
		cacheStream << (*libs);
	    cacheStream << endl;
	}
	cacheFile.close();
    }
}

void ConfigureApp::displayConfig()
{
    // Give some feedback
    cout << "QMAKESPEC..................." << dictionary[ "QMAKESPEC" ] << endl;
    cout << "Configuration..............." << qmakeConfig.join( " " ) << endl;
    cout << "STL support................." << dictionary[ "STL" ] << endl;
    cout << "Thread support.............." << dictionary[ "THREAD" ] << endl;
    cout << "GIF support................." << dictionary[ "GIF" ] << endl;
    cout << "MNG support................." << dictionary[ "MNG" ] << endl;
    cout << "JPEG support................" << dictionary[ "JPEG" ] << endl << endl;
    if( !qmakeDefines.isEmpty() ) {
	cout << "Defines.....................";
	for( QStringList::Iterator defs = qmakeDefines.begin(); defs != qmakeDefines.end(); ++defs )
	    cout << (*defs) << " ";
	cout << endl;
    }
    if( !qmakeIncludes.isEmpty() ) {
	cout << "Include paths...............";
	for( QStringList::Iterator incs = qmakeIncludes.begin(); incs != qmakeIncludes.end(); ++incs )
	    cout << (*incs) << " ";
	cout << endl;
    }
    if( !qmakeLibs.isEmpty() ) {
	cout << "Library paths...............";
	for( QStringList::Iterator libs = qmakeLibs.begin(); libs != qmakeLibs.end(); ++libs )
	    cout << (*libs) << " ";
	cout << endl << endl;
    }
}

void ConfigureApp::buildQmake()
{
    if( dictionary[ "QMAKESPEC" ] == QString( "win32-msvc" ) ) {
	dictionary[ "MAKE" ] = "nmake";
	dictionary[ "QMAKEMAKEFILE" ] = "Makefile";
    }
    else {
	dictionary[ "MAKE" ] = "make";
	dictionary[ "QMAKEMAKEFILE" ] = "Makefile.borland";
    }

    if( dictionary[ "BUILD_QMAKE" ] == "yes" ) {
	QStringList args;

	// Build qmake
	cout << "Creating qmake..." << endl;
	qmakeBuilder.setWorkingDirectory( qtDir + "/qmake" );
	args += dictionary[ "MAKE" ];
	args += "-f";
	args += dictionary[ "QMAKEMAKEFILE" ];
	qmakeBuilder.setArguments( args );
	if( !qmakeBuilder.start() ) {
	    cout << "Could not start qmake build process" << endl << endl;
	    quit();
	}
    }
    else
	qmakeBuilt();
}

void ConfigureApp::readQmakeBuilderError()
{
    QString tmp = qmakeBuilder.readStderr();
    for( int i = 0; i < ( int )tmp.length(); i++ ) {
	QChar c = tmp[ i ];
	switch( char( c ) ) {
	case 0x00:
	    break;
	case '\t':
	    outputLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    outputLine += c;
	    if( outputLine.length() ) {
		cout << outputLine.latin1();
		cout.flush();
		outputLine = "";
	    }
	    break;
	default:
	    outputLine += c;
	    break;
	}
    }
}

void ConfigureApp::readQmakeBuilderOutput()
{
    QString tmp = qmakeBuilder.readStdout();
    for( int i = 0; i < ( int )tmp.length(); i++ ) {
	QChar c = tmp[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    outputLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    outputLine += c;
	    if( outputLine.length() ) {
		cout << outputLine.latin1();
		cout.flush();
		outputLine = "";
	    }
	    break;
	default:
	    outputLine += c;
	    break;
	}
    }
}

void ConfigureApp::qmakeBuilt()
{
    generateMakefiles();
}

void ConfigureApp::findProjects( const QString& dirName )
{
    QDir dir( dirName );
    QString entryName;
    const QFileInfoList* list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo* fi;

    while( ( fi = it.current() ) ) {
	entryName = dirName + "/" + fi->fileName();
	if( fi->fileName()[ 0 ] != '.' ) {
	    if( fi->isDir() )
		findProjects( entryName );
	    else {
		if( fi->fileName().right( 4 ) == ".pro" ) {
		    makeList += dirName;
		    makeList += fi->fileName();
		    if( fi->fileName() == "qtmain.pro" )
			makeList += "Makefile.main";
		    else
			makeList += "Makefile";

		    if( dictionary[ "DSPFILES" ] == "yes" ) {
			makeList += dirName;
			makeList += fi->fileName();
			makeList += fi->fileName().left( fi->fileName().length() - 4 ) + ".dsp";
		    }
		}
	    }
	}
	++it;
    }
}
void ConfigureApp::generateMakefiles()
{
    cout << "Creating makefiles in src..." << endl;

    if( dictionary[ "QMAKESPEC" ] != "win32-msvc" )
	dictionary[ "DSPFILES" ] = "no";

    if( dictionary[ "LEAN" ] == "yes" ) {
	makeList += qtDir + "/src";
	makeList += "qt.pro";
	makeList += "Makefile";
	if( dictionary[ "DSPFILES" ] == "yes" ) {
	    makeList += qtDir + "/src";
	    makeList += "qt.pro";
	    makeList += "qt.dsp";
	}
	makeList += qtDir + "/src";
	makeList += "qtmain.pro";
	makeList += "Makefile.main";
	if( dictionary[ "DSPFILES" ] == "yes" ) {
	    makeList += qtDir + "/src";
	    makeList += "qtmain.pro";
	    makeList += "qtmain.dsp";
	}
    }
    else
	findProjects( qtDir );

    // Start the qmakes for the makelist.
    makeListIterator = makeList.begin();

    // We call this directly, as the code is the same for the first as
    // for subsequent items.
    qmakeDone();
}

void ConfigureApp::readQmakeError()
{
    QString tmp = qmake.readStderr();
    for( int i = 0; i < ( int )tmp.length(); i++ ) {
	QChar c = tmp[ i ];
	switch( char( c ) ) {
	case 0x00:
	    break;
	case '\t':
	    outputLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    outputLine += c;
	    if( outputLine.length() ) {
		cout << outputLine.latin1();
		cout.flush();
		outputLine = "";
	    }
	    break;
	default:
	    outputLine += c;
	    break;
	}
    }
}

void ConfigureApp::readQmakeOutput()
{
    QString tmp = qmake.readStdout();
    for( int i = 0; i < ( int )tmp.length(); i++ ) {
	QChar c = tmp[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    outputLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    outputLine += c;
	    if( outputLine.length() ) {
		cout << outputLine.latin1();
		cout.flush();
		outputLine = "";
	    }
	    break;
	default:
	    outputLine += c;
	    break;
	}
    }
}

void ConfigureApp::qmakeDone()
{
    QString str;

    if( makeListIterator == makeList.end() ) // Just in case we have an empty list
	quit();
    else {
	QString dirPath = *makeListIterator + "/";
	dirPath = QDir::convertSeparators( dirPath );
	++makeListIterator;
	QString projectName = dirPath + (*makeListIterator);
	++makeListIterator;
	QString makefileName = dirPath + (*makeListIterator);
	++makeListIterator;
	QStringList args;
	args << QDir::convertSeparators( qtDir + "/bin/qmake" );
	args << projectName;
	args << dictionary[ "QMAKE_ALL_ARGS" ];
	args << "-o";
        args << makefileName;
	args << "-spec";
	args << dictionary[ "QMAKESPEC" ];
	if( makefileName.right( 4 ) == ".dsp" ) {
	    args << "-t";
	    if( isProjectLibrary( projectName ) )
		args << "vclib";
	    else
		args << "vcapp";
	}
	else
	    cout << "For " << projectName.latin1() << endl;

	str = args.join( " " );
	qmake.setWorkingDirectory( QDir::convertSeparators( dirPath ) );
	qmake.setArguments( args );
	if( !qmake.start() ) {	// This will start the qmake, pick up control again in qmakeDone()
	    cout << "Could not start qmake..." << endl << endl;
	    quit();
	}
    }
}

void ConfigureApp::showSummary()
{
    QString make = dictionary[ "MAKE" ];
    cout << endl << endl << "Qt is now configured for building. Just run " << make.latin1() << "." << endl;
    cout << "To reconfigure, run " << make.latin1() << " clean and configure." << endl << endl;
}

void ConfigureApp::copyDefsFile()
{
    QFile src( QString( QEnvironment::getEnv( "QTDIR" ) ) + "/mkspecs/" + dictionary[ "QMAKESPEC" ] + "/qplatformdefs.h" );
    QFile trg( QString( QEnvironment::getEnv( "QTDIR" ) ) + "/include/qplatformdefs.h" );
    
    cout << "Copying platform definition file..." << endl;
    if( src.open( IO_ReadOnly ) ) {
	QByteArray buffer( src.size() );
	if( trg.open( IO_WriteOnly ) ) {
	    if( buffer.size() ) {
		src.readBlock( buffer.data(), buffer.size() );
		trg.writeBlock( buffer.data(), buffer.size() );
	    }
	    trg.close();
	}
	src.close();
    }
}

bool ConfigureApp::isProjectLibrary( const QString& proFileName )
{
    QFile proFile( proFileName );
    QString buffer;

    if( proFile.open( IO_ReadOnly ) ) {
	while( proFile.readLine( buffer, 1024 ) != -1 ) {
	    QStringList segments = QStringList::split( QRegExp( "\\s" ), buffer );
	    QStringList::Iterator it = segments.begin();

	    QString keyword = (*it++);
	    QString operation = (*it++);
	    QString value = (*it++);

	    if( keyword == "TEMPLATE" ) {
		if( value == "lib" )
		    return true;
		else
		    return false;
	    }
	}
	proFile.close();
    }
    return false;
}

bool ConfigureApp::readLicense()
{
    QFile licenseFile( qtDir + "/.qt-license" );

    if( QFile::exists( qtDir + "/LICENSE.TROLL" ) ) {
	licenseInfo[ "PRODUCTS" ] = "qt-internal";
	return true;
    }
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
	return true;
    }
    return false;
}
