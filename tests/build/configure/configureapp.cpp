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
    dictionary[ "QMAKEPATH" ] = QEnvironment::getEnv( "QMAKEPATH" );
    dictionary[ "QMAKE_INTERNAL" ] = "yes";

    QString tmp = QEnvironment::getEnv( "QMAKEPATH" );
    tmp = tmp.mid( tmp.findRev( "\\" ) + 1 );
    dictionary[ "QMAKEPATH" ] = tmp;

    allModules = QStringList::split( ' ', "tools kernel widgets dialogs iconview workspace network canvas table xml opengl sql styles" );
    allSqlDrivers = QStringList::split( ' ', "mysql oci odbc psql" );
    allConfigs = QStringList::split( ' ', "minimal small medium large full" );

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
	    dictionary[ "QMAKEPATH" ] = (*args);
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
	else if( (*args) == "-no-dsp" )
	    dictionary[ "DSPFILES" ] = "no";
	else if( (*args) == "-dsp" )
	    dictionary[ "DSPFILES" ] = "yes";

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
	cout << "-platform           Specify a platform, uses %QMAKEPATH% as default." << endl;
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
	cout << "-no-dsp             Disable the generation of VC++ .DSP-files." << endl;
	cout << "-dsp                Enable the generation of VC++ .DSP-files." << endl;
	cout << "-D <define>         Add <define> to the list of defines." << endl;
	cout << "-enable-*           Enable the specified module." << endl;
	cout << "-disable-*          Disable the specified module." << endl << endl;
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

    qmakeVars += QString( "QMAKE_LIBDIR_QT=" ) + QDir::convertSeparators( qtDir + "/lib" );
    qmakeVars += QString( "OBJECTS_DIR=" ) + QDir::convertSeparators( "tmp/obj/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "MOC_DIR=" ) + QDir::convertSeparators( "tmp/moc/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "sql-drivers+=" ) + qmakeSql.join( " " );
    qmakeVars += QString( "DEFINES+=" ) + qmakeDefines.join( " " );
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

    if( !dictionary[ "QMAKEPATH" ].length() ) {
	cout << "QMAKEPATH must either be defined as an environment variable, or specified" << endl;
	cout << "as an argument with -platform" << endl;
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
	cacheStream << "QMAKEPATH=" << dictionary[ "QMAKEPATH" ] << endl;
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
	cacheStream << "QMAKEPATH=" << dictionary[ "QMAKEPATH" ] << endl;
	cacheFile.close();
    }
}

void ConfigureApp::displayConfig()
{
    // Give some feedback
    cout << "QMAKEPATH..................." << dictionary[ "QMAKEPATH" ] << endl;
    cout << "Configuration..............." << qmakeConfig.join( " " ) << endl;
    cout << "Thread support.............." << dictionary[ "THREAD" ] << endl;
    cout << "GIF support................." << dictionary[ "GIF" ] << endl;
    cout << "MNG support................." << dictionary[ "MNG" ] << endl;
    cout << "JPEG support................" << dictionary[ "JPEG" ] << endl << endl;
}

void ConfigureApp::buildQmake()
{
    if( dictionary[ "BUILD_QMAKE" ] == "yes" ) {
	if( dictionary[ "QMAKEPATH" ] == QString( "win32-msvc" ) )
	    dictionary[ "MAKE" ] = "nmake";
	else
	    dictionary[ "MAKE" ] = "make";

	// Build qmake
	cout << "Creating qmake..." << endl;
	qmakeBuilder.setWorkingDirectory( qtDir + "/qmake" );
	qmakeBuilder.setArguments( dictionary[ "MAKE" ] );
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
	args << "-path";
	args << dictionary[ "QMAKEPATH" ];
	if( makefileName.right( 4 ) == ".dsp" ) {
	    args << "-t";
	    args << "vcapp";
	}
	else
	    cout << "For " << projectName.latin1() << endl;

	str = args.join( " " );
//	cout << str << endl;
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
    cout << endl << endl << "Qt is now configured for building. Just run " << dictionary[ "MAKE" ] << "." << endl;
    cout << "To reconfigure, run " << dictionary[ "MAKE" ] << " clean and configure." << endl << endl;
}

void ConfigureApp::copyDefsFile()
{
    QFile src( QString( QEnvironment::getEnv( "QTDIR" ) ) + "/mkspecs/" + dictionary[ "QMAKEPATH" ] + "/qplatformdefs.h" );
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
