#include <qtextstream.h>
#include <qregexp.h>
#include "configureapp.h"
#include "../install/environment.h"

#include <iostream.h>
#include <windows.h>

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

    dictionary[ "QCONFIG" ]	    = "full";
    dictionary[ "EMBEDDED" ]	    = "no";

    dictionary[ "BUILD_QMAKE" ]	    = "yes";
    dictionary[ "DSPFILES" ]	    = "yes";
    dictionary[ "QMAKESPEC" ]	    = QEnvironment::getEnv( "QMAKESPEC" );
    dictionary[ "QMAKE_INTERNAL" ]  = "no";
    dictionary[ "LEAN" ]	    = "no";
    dictionary[ "NOPROCESS" ]	    = "no";
    dictionary[ "STL" ]		    = "no";
    dictionary[ "VERSION" ]	    = "300";
    dictionary[ "REDO" ]	    = "no";
    dictionary[ "FORCE_PROFESSIONAL" ] = QEnvironment::getEnv( "FORCE_PROFESSIONAL" );
    dictionary[ "DEPENDENCIES" ]    = "no";

    dictionary[ "DEBUG" ]	    = "no";
    dictionary[ "SHARED" ]	    = "yes";
    dictionary[ "THREAD" ]	    = "yes";

    dictionary[ "GIF" ]		    = "no";
    dictionary[ "ZLIB" ]	    = "yes";
    dictionary[ "PNG" ]		    = "yes";
    dictionary[ "JPEG" ]	    = "yes";
    dictionary[ "MNG" ]		    = "no";
    dictionary[ "ACCESSIBILITY" ]   = "yes";
    dictionary[ "BIG_CODECS" ]	    = "yes";
    dictionary[ "TABLET" ]	    = "no";

    dictionary[ "STYLE_WINDOWS" ]   = "yes";
    dictionary[ "STYLE_MOTIF" ]	    = "yes";
    dictionary[ "STYLE_MOTIFPLUS" ] = "yes";
    dictionary[ "STYLE_PLATINUM" ]  = "yes";
    dictionary[ "STYLE_SGI" ]	    = "yes";
    dictionary[ "STYLE_CDE" ]	    = "yes";

    dictionary[ "SQL_MYSQL" ]	    = "no";
    dictionary[ "SQL_ODBC" ]	    = "no";
    dictionary[ "SQL_OCI" ]	    = "no";
    dictionary[ "SQL_PSQL" ]	    = "no";
    dictionary[ "SQL_TDS" ]	    = "no";

    QString tmp = QEnvironment::getEnv( "QMAKESPEC" );
    tmp = tmp.mid( tmp.findRev( "\\" ) + 1 );
    dictionary[ "QMAKESPEC" ] = tmp;

    readLicense();
    
    buildModulesList();

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
    if ( !fiList )
	return;

    QFileInfoListIterator listIter( *fiList );
    QFileInfo* fi;

    licensedModules = QStringList::split( ' ', "styles tools kernel widgets dialogs iconview workspace" );
    
    if( ( licenseInfo[ "PRODUCTS" ] == "qt-enterprise" ) && ( dictionary[ "FORCE_PROFESSIONAL" ] != "yes" ) )
	licensedModules += QStringList::split( ' ', "network canvas table xml opengl sql" );

    if( dictionary[ "QMAKESPEC" ] == "wince-msvc" ) {
	disabledModules += "opengl";
	disabledModules += "sql";
	disabledModules += "table";
    }
    while( ( fi = listIter.current() ) ) {
	if( licensedModules.findIndex( fi->fileName() ) != -1 )
	    modules += fi->fileName();
	++listIter;
    }
}

void ConfigureApp::parseCmdLine()
{
    QStringList::Iterator args = configCmdLine.begin();

    if( (*args) == "-redo" ) {
	configCmdLine.clear();
	dictionary[ "REDO" ] = "yes";
	reloadCmdLine();
	args = configCmdLine.begin();	// We've got a new command line...
    }

    for( ; args != configCmdLine.end(); ++args ) {
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

	else if( (*args) == "-spec" ) {
	    ++args;
	    dictionary[ "QMAKESPEC" ] = (*args);
	}

	else if( (*args) == "-no-gif" )
	    dictionary[ "GIF" ] = "no";
	else if( (*args) == "-qt-gif" )
	    dictionary[ "GIF" ] = "yes";

	else if( (*args) == "-no-zlib" ) {
	    dictionary[ "ZLIB" ] = "no";
	    dictionary[ "PNG" ] = "no";
	} else if( (*args) == "-zlib" ) {
	    dictionary[ "ZLIB" ] = "yes";
	} else if( (*args) == "-system-zlib" ) {
	    dictionary[ "ZLIB" ] = "system";
	}

	else if( (*args) == "-no-png" )
	    dictionary[ "PNG" ] = "no";
	else if( (*args) == "-qt-png" )
	    dictionary[ "PNG" ] = "yes";
	else if( (*args) == "-system-png" )
	    dictionary[ "PNG" ] = "system";

	else if( (*args) == "-no-mng" )
	    dictionary[ "MNG" ] = "no";
	else if( (*args) == "-qt-mng" )
	    dictionary[ "MNG" ] = "yes";
	else if( (*args) == "-system-mng" )
	    dictionary[ "MNG" ] = "system";

	else if( (*args) == "-no-jpeg" )
	    dictionary[ "JPEG" ] = "no";
	else if( (*args) == "-qt-jpeg" )
	    dictionary[ "JPEG" ] = "yes";
	else if( (*args) == "-system-jpeg" )
	    dictionary[ "JPEG" ] = "system";

	else if( (*args) == "-qt-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "yes";
	else if( (*args) == "-plugin-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "plugin";
	else if( (*args) == "-no-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "no";

	else if( (*args) == "-qt-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "yes";
	else if( (*args) == "-plugin-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "plugin";
	else if( (*args) == "-no-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "no";

	else if( (*args) == "-qt-style-platinum" )
	    dictionary[ "STYLE_PLATINUM" ] = "yes";
	else if( (*args) == "-plugin-style-platinum" )
	    dictionary[ "STYLE_PLATINUM" ] = "plugin";
	else if( (*args) == "-no-style-platinum" )
	    dictionary[ "STYLE_PLATINUM" ] = "no";

	else if( (*args) == "-qt-style-motifplus" )
	    dictionary[ "STYLE_MOTIFPLUS" ] = "yes";
	else if( (*args) == "-plugin-style-motifplus" )
	    dictionary[ "STYLE_MOTIFPLUS" ] = "plugin";
	else if( (*args) == "-no-style-motifplus" )
	    dictionary[ "STYLE_MOTIFPLUS" ] = "no";

	else if( (*args) == "-qt-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "yes";
	else if( (*args) == "-plugin-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "plugin";
	else if( (*args) == "-no-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "no";

	else if( (*args) == "-qt-style-sgi" )
	    dictionary[ "STYLE_SGI" ] = "yes";
	else if( (*args) == "-plugin-style-sgi" )
	    dictionary[ "STYLE_SGI" ] = "plugin";
	else if( (*args) == "-no-style-sgi" )
	    dictionary[ "STYLE_SGI" ] = "no";

	else if( (*args) == "-qt-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "yes";
	else if( (*args) == "-plugin-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "plugin";
	else if( (*args) == "-no-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "no";

	else if( (*args) == "-qt-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "yes";
	else if( (*args) == "-plugin-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "plugin";
	else if( (*args) == "-no-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "no";

	else if( (*args) == "-qt-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "yes";
	else if( (*args) == "-plugin-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "plugin";
	else if( (*args) == "-no-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "no";

	else if( (*args) == "-qt-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "yes";
	else if( (*args) == "-plugin-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "plugin";
	else if( (*args) == "-no-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "no";

	else if( (*args) == "-qt-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "yes";
	else if( (*args) == "-plugin-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "plugin";
	else if( (*args) == "-no-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "no";


	else if( (*args) == "-internal" )
	    dictionary[ "QMAKE_INTERNAL" ] = "yes";

	else if( (*args) == "-no-qmake" )
	    dictionary[ "BUILD_QMAKE" ] = "no";

	else if( (*args) == "-dont-process" )
	    dictionary[ "NOPROCESS" ] = "yes";

	else if( (*args) == "-qmake-deps" )
	    dictionary[ "DEPENDENCIES" ] = "yes";

	else if( (*args) == "-D" ) {
	    ++args;
            qmakeDefines += (*args);
        } else if( (*args) == "-I" ) {
	    ++args;
	    qmakeIncludes += (*args);
	} else if( (*args) == "-l" ) {
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

	else if( (*args) == "-accessibility" )
	    dictionary[ "ACCESSIBILITY" ] = "yes";
	else if( (*args) == "-no-accessibility" )
	    dictionary[ "ACCESSIBILITY" ] = "no";

	else if( (*args) == "-no-big-codecs" )
	    dictionary[ "BIG_CODECS" ] = "no";
	else if( (*args) == "-big-codecs" )
	    dictionary[ "BIG_CODECS" ] = "yes";

	else if( (*args) == "-tablet" )
	    dictionary[ "TABLET" ] = "yes";
	else if( (*args) == "-no-tablet" )
	    dictionary[ "TABLET" ] = "no";

	else if( ( (*args) == "-override-version" ) || ( (*args) == "-version-override" ) ){
	    ++args;
	    dictionary[ "VERSION" ] = (*args);
	}

	else if( (*args).find( QRegExp( "^-(en|dis)able-" ) ) != -1 ) {
	    // Scan to see if any specific modules and drivers are enabled or disabled
	    for( QStringList::Iterator module = modules.begin(); module != modules.end(); ++module ) {
		if( (*args) == QString( "-enable-" ) + (*module) ) {
		    enabledModules += (*module);
		    break;
		}
		else if( (*args) == QString( "-disable-" ) + (*module) ) {
		    disabledModules += (*module);
		    break;
		}
	    }
	}

	else {
	    dictionary[ "HELP" ] = "yes";
	    cout << "Unknown option " << (*args) << endl;
	    break;
	}

    }

    if( dictionary[ "QMAKESPEC" ] == "wince-msvc" ) {
	dictionary[ "QMAKE_INTERNAL" ] = "no";
	dictionary[ "ACCESSIBILITY" ] = "no";
    }

    if( dictionary[ "QMAKE_INTERNAL" ] == "yes" ) {
	qmakeConfig += modules;
	if( licenseInfo[ "PRODUCTS" ] == "qt-enterprise" )
	    qmakeConfig += "internal";
    } else {
	for( QStringList::Iterator dis = disabledModules.begin(); dis != disabledModules.end(); ++dis ) {
	    modules.remove( (*dis) );
	}
	for( QStringList::Iterator ena = enabledModules.begin(); ena != enabledModules.end(); ++ena ) {
	    if( modules.findIndex( (*ena) ) == -1 )
		modules += (*ena);
	}
	qmakeConfig += modules;
    }

    for( QStringList::Iterator it = disabledModules.begin(); it != disabledModules.end(); ++it )
	qmakeConfig.remove( (*it) );

    if( ( dictionary[ "REDO" ] != "yes" ) && ( dictionary[ "HELP" ] != "yes" ) )
	saveCmdLine();
}

void ConfigureApp::validateArgs()
{
    QStringList configs;
    // Validate the specified config

    allConfigs = QStringList::split( ' ', "minimal small medium large full" );

    QStringList::Iterator config;
    for( config = allConfigs.begin(); config != allConfigs.end(); ++config ) {
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
	cout << "-help               Bring up this help text." << endl << endl;

	cout << "-debug              Enable debug information." << endl;
	cout << "-release          * Disable debug information." << endl << endl;

	cout << "-shared           * Build Qt as a shared library." << endl;
	cout << "-static             Build Qt as a static library." << endl << endl;

	cout << "-thread             Configure Qt with thread support." << endl;
	cout << "-no-thread        * Configure Qt without thread support." << endl << endl;

	cout << "-spec               Specify a platform, uses %QMAKESPEC% as default." << endl;
	cout << "-qconfig            Specify config, available configs:" << endl;
	for( QStringList::Iterator config = allConfigs.begin(); config != allConfigs.end(); ++config )
	    cout << "                        " << (*config).latin1() << endl;

	cout << "-qt-gif             Enable GIF support." << endl;
	cout << "-no-gif           * Disable GIF support." << endl << endl;

	cout << "-no-zlib            Disable zlib.  Implies -no-png." << endl;
	cout << "-qt-zlib          * Compile in zlib." << endl;
	cout << "-system-zlib        Use existing zlib in system." << endl << endl;

	cout << "-no-png             PNG support through plugin." << endl;
	cout << "-qt-png           * Compile in PNG support." << endl;
	cout << "-system-png	     Use existing libPNG in system." << endl  << endl;

	cout << "-no-mng           * MNG support through plugin." << endl;
	cout << "-qt-mng             Compile in MNG support." << endl;
	cout << "-system-mng         Use existing libMNG in system." << endl << endl;

	cout << "-no-jpeg          * JPEG support through plugin." << endl;
	cout << "-qt-jpeg            Compile in JPEG support" << endl;
	cout << "-system-jpeg        Use existing libJPEG in system" << endl << endl;
	
	cout << "-stl                Enable STL support." << endl;
	cout << "-no-stl           * Disable STL support." << endl  << endl;

	cout << "-accessibility    * Enable Windows Active Accessibility." << endl;
	cout << "-no-accessibility   Disable Windows Active Accessibility." << endl  << endl;

	cout << "-tablet             Enable tablet support." << endl;
	cout << "-no-tablet        * Disable tablet support." << endl  << endl;

	cout << "-big-codecs         Enable the building of big codecs." << endl;
	cout << "-no-big-codecs      Disable the building of big codecs." << endl << endl;

	cout << "-no-dsp             Disable the generation of VC++ .DSP-files." << endl;
	cout << "-dsp              * Enable the generation of VC++ .DSP-files." << endl << endl;

	cout << "-no-qmake           Do not build qmake." << endl;
	cout << "-lean               Only process the Qt core projects." << endl;
	cout << "                    (qt.pro, qtmain.pro)." << endl << endl;

	cout << "-D <define>         Add <define> to the list of defines." << endl;
	cout << "-I <includepath>    Add <includepath> to the include searchpath." << endl;
	cout << "-l <library>        Add <library> to the library list." << endl << endl;

	cout << "-enable-*           Enable the specified module, where module is one of" << endl;
	cout << "                    " << modules.join( " " ) << endl << endl;

	cout << "-disable-*          Disable the specified module, where module is one of" << endl;
	cout << "                    " << modules.join( " " ) << endl << endl << endl;

	cout << "-redo               Run configure with the same parameters as last time." << endl << endl;
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
    } else {
	qmakeConfig += "release";
	dictionary[ "QMAKE_OUTDIR" ] = "release";
    }

    if( dictionary[ "THREAD" ] == "yes" ) {
	qmakeConfig += "thread";
	dictionary[ "QMAKE_OUTDIR" ] += "_mt";
    }

    if( dictionary[ "ACCESSIBILITY" ] == "yes" ) {
	qmakeConfig += "accessibility";
    }

    if( dictionary[ "SHARED" ] == "yes" ) {
	dictionary[ "QMAKE_OUTDIR" ] += "_shared";
	qmakeDefines += "QT_DLL";
    } else {
	dictionary[ "QMAKE_OUTDIR" ] += "_static";
    }

    if( dictionary[ "STL" ] == "no" ) {
	qmakeDefines += "QT_NO_STL";
    }

    if( !qmakeLibs.isEmpty() ) {
	qmakeVars += "LIBS += " + qmakeLibs.join( " " );
    }

    if( dictionary[ "GIF" ] == "yes" )
	qmakeConfig += "gif";
    else if( dictionary[ "GIF" ] == "no" )
	qmakeConfig += "no-gif";

    if( dictionary[ "ZLIB" ] == "yes" )
	qmakeConfig += "zlib";
    else if( dictionary[ "ZLIB" ] == "no" )
	qmakeConfig += "no-zlib";

    if( dictionary[ "JPEG" ] == "no" )
	qmakeConfig += "no-jpeg";
    else if( dictionary[ "JPEG" ] == "yes" )
	qmakeConfig += "jpeg";
    else if( dictionary[ "JPEG" ] == "system" )
	qmakeConfig += "system-jpeg";

    if( dictionary[ "MNG" ] == "no" )
	qmakeConfig += "no-mng";
    else if( dictionary[ "MNG" ] == "yes" )
	qmakeConfig += "mng";
    else if( dictionary[ "MNG" ] == "system" )
	qmakeConfig += "system-mng";

    if( dictionary[ "PNG" ] == "yes" )
	qmakeConfig += "png";
    else if( dictionary[ "PNG" ] == "no" )
	qmakeConfig += "no-png";

    if( dictionary[ "BIG_CODECS" ] == "yes" )
	qmakeConfig += "bigcodecs";
    else if( dictionary[ "BIG_CODECS" ] == "no" )
	qmakeConfig += "no-bigcodecs";

    if( dictionary[ "TABLET" ] == "yes" )
	qmakeConfig += "tablet";
    else if( dictionary[ "TABLET" ] == "no" )
	qmakeConfig += "no-tablet";

    if ( dictionary[ "STYLE_WINDOWS" ] == "yes" )
	qmakeConfig += "style-windows";
    else if ( dictionary[ "STYLE_WINDOWS" ] == "no" )
	qmakeConfig += "no-style-windows";

    if ( dictionary[ "STYLE_MOTIF" ] == "yes" )
	qmakeConfig += "style-motif";
    else if ( dictionary[ "STYLE_MOTIF" ] == "no" )
	qmakeConfig += "no-style-motif";

    if ( dictionary[ "STYLE_MOTIFPLUS" ] == "yes" )
	qmakeConfig += "style-motifplus";
    else if ( dictionary[ "STYLE_MOTIFPLUS" ] == "no" )
	qmakeConfig += "no-style-motifplus";

    if ( dictionary[ "STYLE_PLATINUM" ] == "yes" )
	qmakeConfig += "style-platinum";
    else if ( dictionary[ "STYLE_PLATINUM" ] == "no" )
	qmakeConfig += "no-style-platinum";

    if ( dictionary[ "STYLE_SGI" ] == "yes" )
	qmakeConfig += "style-sgi";
    else if ( dictionary[ "STYLE_SGI" ] == "no" )
	qmakeConfig += "no-style-sgi";

    if ( dictionary[ "STYLE_CDE" ] == "yes" )
	qmakeConfig += "style-cde";
    else if ( dictionary[ "STYLE_CDE" ] == "no" )
	qmakeConfig += "no-style-cde";

    if ( dictionary[ "SQL_MYSQL" ] == "yes" )
	qmakeConfig += "sql-mysql";
    else if ( dictionary[ "SQL_MYSQL" ] == "no" )
	qmakeConfig += "no-sql-mysql";

    if ( dictionary[ "SQL_ODBC" ] == "yes" )
	qmakeConfig += "sql-odbc";
    else if ( dictionary[ "SQL_ODBC" ] == "no" )
	qmakeConfig += "no-sql-odbc";

    if ( dictionary[ "SQL_OCI" ] == "yes" )
	qmakeConfig += "sql-oci";
    else if ( dictionary[ "SQL_OCI" ] == "no" )
	qmakeConfig += "no-sql-oci";

    if ( dictionary[ "SQL_PSQL" ] == "yes" )
	qmakeConfig += "sql-psql";
    else if ( dictionary[ "SQL_PSQL" ] == "no" )
	qmakeConfig += "no-sql-psql";

    if ( dictionary[ "SQL_TDS" ] == "yes" )
	qmakeConfig += "sql-tds";
    else if ( dictionary[ "SQL_TDS" ] == "no" )
	qmakeConfig += "no-sql-tds";


    qmakeVars += "QMAKE_QT_VERSION_OVERRIDE=" + dictionary[ "VERSION" ];

    qmakeVars += QString( "QMAKE_LIBDIR_QT=" ) + QDir::convertSeparators( qtDir + "/lib" );
    qmakeVars += QString( "OBJECTS_DIR=" ) + QDir::convertSeparators( "tmp/obj/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "MOC_DIR=" ) + QDir::convertSeparators( "tmp/moc/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "DEFINES+=" ) + qmakeDefines.join( " " );
    if( licenseInfo[ "PRODUCTS" ].length() )
	qmakeVars += QString( "QT_PRODUCT=" ) + licenseInfo[ "PRODUCTS" ];

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
    if( cacheFile.open( IO_WriteOnly | IO_Translate ) ) { // Truncates any existing file.
	QTextStream cacheStream( &cacheFile );
        for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
	    cacheStream << (*var) << endl;
	}
	cacheStream << "CONFIG+=" << qmakeConfig.join( " " ) << " incremental" << endl;
	cacheStream << "QMAKESPEC=" << dictionary[ "QMAKESPEC" ] << endl;
	if( !qmakeIncludes.isEmpty() ) {
	    cacheStream << "INCLUDEPATH=";
	    for( QStringList::Iterator incs = qmakeIncludes.begin(); incs != qmakeIncludes.end(); ++incs )
		cacheStream << (*incs);
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
    if( cacheFile.open( IO_WriteOnly | IO_Translate ) ) { // Truncates any existing file.
	QTextStream cacheStream( &cacheFile );
	for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
	    cacheStream << (*var) << endl;
	}
	cacheStream << "CONFIG+=" << srcConfig.join( " " ) << endl;
	cacheStream << "QMAKESPEC=" << dictionary[ "QMAKESPEC" ] << endl;
	if( !qmakeIncludes.isEmpty() ) {
	    cacheStream << "INCLUDEPATH=";
	    for( QStringList::Iterator incs = qmakeIncludes.begin(); incs != qmakeIncludes.end(); ++incs )
		cacheStream << (*incs);
	    cacheStream << endl;
	}
	cacheFile.close();
    }
}

void ConfigureApp::generateConfigfiles()
{
    QString outDir( qtDir + "/include" );


    if( dictionary[ "QMAKE_INTERNAL" ] == "yes" )
	outDir = qtDir + "/src/tools";

    QString outName( outDir + "/qconfig.h" );

    ::SetFileAttributesA( outName, FILE_ATTRIBUTE_NORMAL );
    QFile::remove( outName );
    QFile outFile( outName );

    if( outFile.open( IO_WriteOnly | IO_Translate ) ) {
	QTextStream outStream( &outFile );

	if( dictionary[ "QCONFIG" ] == "full" ) {
	    outStream << "// Everything" << endl << endl;
	    outStream << "#if defined( QT_MAKEDLL ) && !defined( QT_DLL )" << endl;
	    outStream << "#define QT_DLL" << endl;
	    outStream << "#endif" << endl;
	} else {
	    QString configName( "qconfig-" + dictionary[ "QCONFIG" ] + ".h" );
	    outStream << "// Copied from " << configName << endl;
	    
	    QFile inFile( qtDir + "/src/tools/" + configName );
	    if( inFile.open( IO_ReadOnly | IO_Translate ) ) {
		QByteArray buffer = inFile.readAll();
		outFile.writeBlock( buffer.data(), buffer.size() );
		inFile.close();
	    }
	}
	outStream << endl;
	outStream << "#define QT_PRODUCT_LICENSEE \"" << licenseInfo[ "LICENSEE" ] << "\"" << endl;
	outStream << "#define QT_PRODUCT_LICENSE \"" << licenseInfo[ "PRODUCTS" ] << "\"" << endl;

	outFile.close();
	if( dictionary[ "QMAKE_INTERNAL" ] == "yes" )
	    ::SetFileAttributesA( outName, FILE_ATTRIBUTE_READONLY );
    }
    outName = outDir + "/qmodules.h";

    ::SetFileAttributesA( outName, FILE_ATTRIBUTE_NORMAL );
    QFile::remove( outName );
    outFile.setName( outName );

    if( outFile.open( IO_WriteOnly | IO_Translate ) ) {
	QTextStream outStream( &outFile );

	outStream << "// These modules are present in this configuration of Qt" << endl;
	for( QStringList::Iterator it = modules.begin(); it != modules.end(); ++it ) {
	    outStream << "#define QT_MODULE_" << (*it).upper() << endl;
	}
	outStream << endl;
	outStream << "// Compile time features" << endl;
	if( dictionary[ "STL" ] == "no" ) {
	    outStream << "#ifndef QT_NO_STL" << endl;
	    outStream << "#define QT_NO_STL" << endl;
	    outStream << "#endif" << endl;
	}
	outFile.close();
	if( dictionary[ "QMAKE_INTERNAL" ] == "yes" )
	    ::SetFileAttributesA( outName, FILE_ATTRIBUTE_READONLY );
    }

}

void ConfigureApp::displayConfig()
{
    // Give some feedback
    cout << "QMAKESPEC..................." << dictionary[ "QMAKESPEC" ] << endl;
    cout << "Configuration..............." << qmakeConfig.join( " " ) << endl;

    cout << "Debug symbols..............." << dictionary[ "DEBUG" ] << endl;
    cout << "Thread support.............." << dictionary[ "THREAD" ] << endl << endl;

    cout << "Accessibility support......." << dictionary[ "ACCESSIBILITY" ] << endl;
    cout << "Big Textcodecs.............." << dictionary[ "BIG_CODECS" ] << endl;
    cout << "Tablet support.............." << dictionary[ "TABLET" ] << endl;
    cout << "STL support................." << dictionary[ "STL" ] << endl << endl;

    cout << "Image formats:" << endl;
    cout << "GIF support................." << dictionary[ "GIF" ] << endl;
    cout << "MNG support................." << dictionary[ "MNG" ] << endl;
    cout << "JPEG support................" << dictionary[ "JPEG" ] << endl;
    cout << "PNG support................." << dictionary[ "PNG" ] << endl << endl;

    cout << "Styles:" << endl;
    cout << "Windows....................." << dictionary[ "STYLE_WINDOWS" ] << endl;
    cout << "Motif......................." << dictionary[ "STYLE_MOTIF" ] << endl;
    cout << "Platinum...................." << dictionary[ "STYLE_PLATINUM" ] << endl;
    cout << "MotifPlus..................." << dictionary[ "STYLE_MOTIFPLUS" ] << endl;
    cout << "CDE........................." << dictionary[ "STYLE_CDE" ] << endl;
    cout << "SGI........................." << dictionary[ "STYLE_SGI" ] << endl << endl;

    cout << "Sql Drivers:" << endl;
    cout << "ODBC........................" << dictionary[ "SQL_ODBC" ] << endl;
    cout << "MySQL......................." << dictionary[ "SQL_MYSQL" ] << endl;
    cout << "OCI........................." << dictionary[ "SQL_OCI" ] << endl;
    cout << "PostgreSQL.................." << dictionary[ "SQL_PSQL" ] << endl;
    cout << "TDS........................." << dictionary[ "SQL_TDS" ] << endl << endl;

    cout << endl;
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
	cout << "Additional libraries........";
	for( QStringList::Iterator libs = qmakeLibs.begin(); libs != qmakeLibs.end(); ++libs )
	    cout << (*libs) << " ";
	cout << endl;
    }
    if( dictionary[ "FORCE_PROFESSIONAL" ] == "yes" ) {
	cout << "Licensing forced to professional edition.  If this is not what you want, unset" << endl;
	cout << "the FORCE_PROFESSIONAL environment variable." << endl;
    }
    if( dictionary[ "QMAKE_INTERNAL" ] == "yes" ) {
	cout << "Using internal configuration." << endl;
    }
    if( dictionary[ "SHARED" ] == "no" ) {
	cout << "WARNING: Using static linking will disable the use of plugins." << endl;
	cout << "         Make sure you compile ALL needed modules into the library." << endl;
    }
}

void ConfigureApp::buildQmake()
{
    if( dictionary[ "QMAKESPEC" ].right( 5 ) == QString( "-msvc" ) ) {
	dictionary[ "MAKE" ] = "nmake";
	dictionary[ "QMAKEMAKEFILE" ] = "Makefile";
    } else {
	dictionary[ "MAKE" ] = "make";
	dictionary[ "QMAKEMAKEFILE" ] = "Makefile";
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

    if( dictionary[ "NOPROCESS" ] == "no" ) {
	while( ( fi = it.current() ) ) {
	    entryName = dirName + "/" + fi->fileName();
	    if( fi->fileName()[ 0 ] != '.' ) {
		if( fi->isDir() ) {
		    findProjects( entryName );
		} else {
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
}
void ConfigureApp::generateMakefiles()
{
    if( dictionary[ "NOPROCESS" ] == "no" ) {
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
    } else {
	cout << "Processing of project files have been disabled." << endl;
	cout << "Only use this option if you really know what you're doing." << endl << endl;
	quit();
    }
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

    if( makeListIterator == makeList.end() ) {// Just in case we have an empty list
	quit();
	return;
    }
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
    if( dictionary[ "DEPENDENCIES" ] == "no" )
	args << "-nodepend";

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

void ConfigureApp::showSummary()
{
    QString make = dictionary[ "MAKE" ];
    cout << endl << endl << "Qt is now configured for building. Just run " << make.latin1() << "." << endl;
    cout << "To reconfigure, run " << make.latin1() << " clean and configure." << endl << endl;
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

void ConfigureApp::readLicense()
{
    QFile licenseFile( QDir::homeDirPath() + "/.qt-license" );

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
    }
    if( QFile::exists( qtDir + "/LICENSE.TROLL" ) ) {
	licenseInfo[ "PRODUCTS" ] = "qt-enterprise";
	dictionary[ "QMAKE_INTERNAL" ] = "yes";
    }
    if( dictionary[ "FORCE_PROFESSIONAL" ] == "yes" )
        licenseInfo[ "PRODUCTS" ]= "qt-professional";
}

void ConfigureApp::reloadCmdLine()
{
    if( dictionary[ "REDO" ] == "yes" ) {
	QFile inFile( qtDir + "/configure.cache" );
	if( inFile.open( IO_ReadOnly ) ) {
	    QTextStream inStream( &inFile );
	    QString buffer;
	    inStream >> buffer;
	    while( buffer.length() ) {
		configCmdLine += buffer;
		inStream >> buffer;
	    }
	    inFile.close();
	}
    }
}

void ConfigureApp::saveCmdLine()
{
    if( dictionary[ "REDO" ] != "yes" ) {
	QFile outFile( qtDir + "/configure.cache" );
	if( outFile.open( IO_WriteOnly ) ) {
	    QTextStream outStream( &outFile );
	    for( QStringList::Iterator it = configCmdLine.begin(); it != configCmdLine.end(); ++it )
		outStream << (*it) << endl;
	    outFile.close();
	}
    }
}
