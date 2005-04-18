#include "configureapp.h"
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qhash.h>
#include "keyinfo.h"

#include <iostream>
#include <windows.h>
#include <conio.h>

std::ostream &operator<<( std::ostream &s, const QString &val ) {
    s << val.toLocal8Bit().data();
    return s;
}

using namespace std;

// Macros to simplify options marking, and WinCE only code
#define MARK_OPTION(x,y) ( dictionary[ #x ] == #y ? "*" : " " )
#define WCE(x) if ( dictionary[ "QMAKESPEC" ].startsWith( "wince-" ) ) { x }


bool writeToFile(const char* text, const QString &filename)
{
    QByteArray symFile(text);
    QFile file(filename);
    QDir dir(QFileInfo(file).absoluteDir());
    if (!dir.exists())
        dir.mkdir(dir.absolutePath());
    if (!file.open(QFile::WriteOnly)) {
        cout << "Couldn't write to " << qPrintable(filename) << ": " << qPrintable(file.errorString())
             << endl;
        return false;
    }
    file.write(symFile);
    return true;
}


Configure::Configure( int& argc, char** argv )
{
    QString qtDir = getenv("QTDIR");
    if (qtDir.isEmpty()) {
        cout << "QTDIR environment variable not specified, configuration aborted!" << endl;
        dictionary[ "DONE" ] = "error";
        return;
    }

    int i;

    /*
    ** Set up the initial state, the default
    */
    dictionary[ "CONFIGCMD" ] = argv[ 0 ];

    for ( i = 1; i < argc; i++ )
	configCmdLine += argv[ i ];

    dictionary[ "ARCHITECTURE" ]    = "windows";

    dictionary[ "QCONFIG" ]	    = "full";
    dictionary[ "EMBEDDED" ]	    = "no";

    dictionary[ "BUILD_QMAKE" ]	    = "yes";
    dictionary[ "DSPFILES" ]	    = "yes";
    dictionary[ "VCPFILES" ]	    = "yes";
    dictionary[ "VCPROJFILES" ]	    = "yes";
    dictionary[ "QMAKESPEC" ]	    = getenv( "QMAKESPEC" );
    dictionary[ "QMAKE_INTERNAL" ]  = "no";
    dictionary[ "LEAN" ]	    = "no";
    dictionary[ "FAST" ]            = "no";
    dictionary[ "NOPROCESS" ]	    = "no";
    dictionary[ "STL" ]		    = "yes";
    dictionary[ "EXCEPTIONS" ]	    = "no";
    dictionary[ "RTTI" ]	    = "no";

    QString version;
    QFile profile(QString(getenv("QTDIR")) + "/src/qbase.pri");
    if (profile.open(QFile::ReadOnly)) {
	QTextStream read(&profile);
	QString line;
	while (!read.atEnd()) {
	    line = read.readLine();
	    if (line.contains("VERSION")) {
		version = line.mid(line.indexOf('=') + 1);
		version = version.trimmed();
		if (!version.isEmpty())
		    break;
	    }
	}
	profile.close();
    }

    if (version.isEmpty())
	version = QString("%1.%2.%3").arg(QT_VERSION>>16).arg(((QT_VERSION>>8)&0xff)).arg(QT_VERSION&0xff);

    dictionary[ "VERSION" ]	    = version;
    dictionary[ "REDO" ]	    = "no";
    dictionary[ "FORCE_PROFESSIONAL" ] = getenv( "FORCE_PROFESSIONAL" );
    dictionary[ "DEPENDENCIES" ]    = "no";

    dictionary[ "DEBUG" ]	    = "no";
    dictionary[ "SHARED" ]	    = "yes";

    dictionary[ "ZLIB" ]	    = "yes";

    dictionary[ "GIF" ]		    = "plugin";
    dictionary[ "JPEG" ]	    = "plugin";
    dictionary[ "PNG" ]		    = "qt";
    dictionary[ "MNG" ]		    = "no";
    dictionary[ "LIBJPEG" ]	    = "qt";
    dictionary[ "LIBPNG" ]	    = "qt";
    dictionary[ "LIBMNG" ]	    = "qt";

    dictionary[ "QT3SUPPORT" ]	    = "yes";
    dictionary[ "ACCESSIBILITY" ]   = "yes";
    dictionary[ "BIG_CODECS" ]	    = "yes";
    dictionary[ "OPENGL" ]	    = "yes";
    dictionary[ "IPV6" ]            = "yes"; // Always, dynamicly loaded

    dictionary[ "STYLE_WINDOWS" ]   = "yes";
    dictionary[ "STYLE_MOTIF" ]     = "yes";
    dictionary[ "STYLE_MOTIFPLUS" ] = "no";
    dictionary[ "STYLE_PLATINUM" ]  = "no";
    dictionary[ "STYLE_SGI" ]	    = "no";
    dictionary[ "STYLE_CDE" ]	    = "yes";
    dictionary[ "STYLE_WINDOWSXP" ] = "auto";
    dictionary[ "STYLE_COMPACT" ]   = "no";
    dictionary[ "STYLE_POCKETPC" ]  = "no";
    dictionary[ "STYLE_MAC" ]       = "no";

    dictionary[ "SQL_MYSQL" ]	    = "no";
    dictionary[ "SQL_ODBC" ]	    = "no";
    dictionary[ "SQL_OCI" ]	    = "no";
    dictionary[ "SQL_PSQL" ]	    = "no";
    dictionary[ "SQL_TDS" ]	    = "no";
    dictionary[ "SQL_DB2" ]	    = "no";
    dictionary[ "SQL_SQLITE" ]	    = "no";
    dictionary[ "SQL_IBASE" ]	    = "no";

    // Avoid all those problems of people configuring outside of QTDIR
    QString cwd = QDir::currentPath();
    if (QFileInfo(cwd) != QFileInfo(qtDir))
        QDir::setCurrent(qtDir);

    dictionary[ "QT_SOURCE_TREE" ]  = QDir::convertSeparators( QDir::currentPath() );
    dictionary[ "QT_INSTALL_PREFIX" ] = QDir::convertSeparators( dictionary[ "QT_SOURCE_TREE" ] );
    dictionary[ "QT_INSTALL_TRANSLATIONS" ] = dictionary[ "QT_INSTALL_PREFIX" ] + "\\translations";

    QString tmp = dictionary[ "QMAKESPEC" ];
    tmp = tmp.mid( tmp.lastIndexOf( "\\" ) + 1 );
    dictionary[ "QMAKESPEC" ] = tmp;
    qmakeConfig += "nocrosscompiler";

#if !defined(EVAL)
    WCE( {
	// WinCE has different defaults than the desktop version
	dictionary[ "QMAKE_INTERNAL" ]	= "no";
	dictionary[ "BUILD_QMAKE" ]	= "no";
	dictionary[ "ACCESSIBILITY" ]	= "no"; // < CE 4.0
	dictionary[ "BIG_CODECS" ]	= "no"; // Keep small
	dictionary[ "STL" ]		= "no"; // < CE 4.0
	dictionary[ "OPENGL" ]		= "no"; // < CE 4.0
	dictionary[ "STYLE_MOTIF" ]	= "no";
	dictionary[ "STYLE_MOTIFPLUS" ] = "no";
	dictionary[ "STYLE_PLATINUM" ]  = "no";
	dictionary[ "STYLE_SGI" ]	= "no";
	dictionary[ "STYLE_CDE" ]	= "no";
	dictionary[ "STYLE_WINDOWSXP" ] = "no";
	dictionary[ "STYLE_POCKETPC" ]	= "yes";

	// Disabled modules for CE
	disabledModules += "opengl";		// < CE 4.0
	disabledModules += "sql";		// For now
    } );

    readLicense();
    if (!isOk())
        return;
#endif
}

Configure::~Configure()
{
    for (int i=0; i<3; ++i) {
	QList<MakeItem*> items = makeList[i];
	for (int j=0; j<items.size(); ++j)
	    delete items[j];
    }
}

// We could use QDir::homePath() + "/.qt-license", but
// that will only look in the first of $HOME,$USERPROFILE
// or $HOMEDRIVE$HOMEPATH. So, here we try'em all to be
// more forgiving for the end user..
QString Configure::firstLicensePath()
{
    QStringList allPaths;
    allPaths << "./.qt-license"
             << QString::fromLocal8Bit(getenv("HOME")) + "/.qt-license"
             << QString::fromLocal8Bit(getenv("USERPROFILE")) + "/.qt-license"
             << QString::fromLocal8Bit(getenv("HOMEDRIVE")) + QString::fromLocal8Bit(getenv("HOMEPATH")) + "/.qt-license";
    for (int i = 0; i< allPaths.count(); ++i)
        if (QFile::exists(allPaths.at(i)))
            return allPaths.at(i);
    return QString();
}


// #### somehow I get a compiler error about vc++ reaching the nesting limit without
// undefining the ansi for scoping.
#ifdef for
#undef for
#endif

void Configure::parseCmdLine()
{
    int argCount = configCmdLine.size();
    int i = 0;
#if !defined(EVAL)
    if (argCount < 1) // skip rest if no arguments
	;
    else if( configCmdLine.at(i) == "-redo" ) {
	configCmdLine.clear();
	dictionary[ "REDO" ] = "yes";
	reloadCmdLine();
    }
    else if( configCmdLine.at(i) == "-loadconfig" ) {
	++i;
	dictionary[ "REDO" ] = "yes";
	dictionary[ "CUSTOMCONFIG" ] = "_" + configCmdLine.at(i);
	configCmdLine.clear();
	reloadCmdLine();
    }
#endif

    for( ; i<configCmdLine.size(); ++i ) {
	if( configCmdLine.at(i) == "-help" )
	    dictionary[ "HELP" ] = "yes";
	else if( configCmdLine.at(i) == "-?" )
	    dictionary[ "HELP" ] = "yes";

#if !defined(EVAL)
	else if( configCmdLine.at(i) == "-qconfig" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "QCONFIG" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-release" )
	    dictionary[ "DEBUG" ] = "no";
	else if( configCmdLine.at(i) == "-debug" )
	    dictionary[ "DEBUG" ] = "yes";

	else if( configCmdLine.at(i) == "-shared" )
	    dictionary[ "SHARED" ] = "yes";
	else if( configCmdLine.at(i) == "-static" )
	    dictionary[ "SHARED" ] = "no";

#endif

	else if( configCmdLine.at(i) == "-spec" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "QMAKESPEC" ] = configCmdLine.at(i);
	} else if( configCmdLine.at(i) == "-arch" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "ARCHITECTURE" ] = configCmdLine.at(i);
	}

#if !defined(EVAL)
	else if( configCmdLine.at(i) == "-no-zlib" ) {
	    dictionary[ "ZLIB" ] = "no";
	    dictionary[ "PNG" ] = "no";
	} else if( configCmdLine.at(i) == "-qt-zlib" ) {
	    dictionary[ "ZLIB" ] = "yes";
	} else if( configCmdLine.at(i) == "-system-zlib" ) {
	    dictionary[ "ZLIB" ] = "system";
	}

        // Image formats --------------------------------------------
	else if( configCmdLine.at(i) == "-no-imgfmt-gif" )
	    dictionary[ "GIF" ] = "no";
	else if( configCmdLine.at(i) == "-plugin-imgfmt-gif" )
	    dictionary[ "GIF" ] = "plugin";

	else if( configCmdLine.at(i) == "-no-imgfmt-jpeg" )
	    dictionary[ "JPEG" ] = "no";
	else if( configCmdLine.at(i) == "-plugin-imgfmt-jpeg" )
	    dictionary[ "JPEG" ] = "plugin";
	else if( configCmdLine.at(i) == "-qt-jpeg" )
	    dictionary[ "LIBJPEG" ] = "qt";
	else if( configCmdLine.at(i) == "-system-jpeg" )
	    dictionary[ "LIBJPEG" ] = "system";

	else if( configCmdLine.at(i) == "-no-imgfmt-png" )
	    dictionary[ "PNG" ] = "no";
	else if( configCmdLine.at(i) == "-qt-imgfmt-png" )
	    dictionary[ "PNG" ] = "qt";
	else if( configCmdLine.at(i) == "-qt-png" )
	    dictionary[ "LIBPNG" ] = "qt";
	else if( configCmdLine.at(i) == "-system-png" )
	    dictionary[ "LIBPNG" ] = "system";

	//else if( configCmdLine.at(i) == "-no-imgfmt-mng" )
	//    dictionary[ "MNG" ] = "no";
	//else if( configCmdLine.at(i) == "-plugin-imgfmt-mng" )
	//    dictionary[ "MNG" ] = "plugin";
	//else if( configCmdLine.at(i) == "-qt-mng" )
	//    dictionary[ "LIBMNG" ] = "qt";
	//else if( configCmdLine.at(i) == "-system-mng" )
	//    dictionary[ "LIBMNG" ] = "system";

        // Styles ---------------------------------------------------
	else if( configCmdLine.at(i) == "-qt-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-motifplus" )
	    dictionary[ "STYLE_MOTIFPLUS" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-motifplus" )
	    dictionary[ "STYLE_MOTIFPLUS" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-motifplus" )
	    dictionary[ "STYLE_MOTIFPLUS" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-sgi" )
	    dictionary[ "STYLE_SGI" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-sgi" )
	    dictionary[ "STYLE_SGI" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-sgi" )
	    dictionary[ "STYLE_SGI" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-windowsxp" )
	    dictionary[ "STYLE_WINDOWSXP" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-windowsxp" )
	    dictionary[ "STYLE_WINDOWSXP" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-windowsxp" )
	    dictionary[ "STYLE_WINDOWSXP" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-pocketpc" )
	    dictionary[ "STYLE_POCKETPC" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-style-pocketpc" )
	    dictionary[ "STYLE_POCKETPC" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-style-pocketpc" )
	    dictionary[ "STYLE_POCKETPC" ] = "no";

        // Databases ------------------------------------------------
	else if( configCmdLine.at(i) == "-qt-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-db2" )
	    dictionary[ "SQL_DB2" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-db2" )
	    dictionary[ "SQL_DB2" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-db2" )
	    dictionary[ "SQL_DB2" ] = "no";

        else if( configCmdLine.at(i) == "-qt-sql-sqlite" )
            dictionary[ "SQL_SQLITE" ] = "yes";
        else if( configCmdLine.at(i) == "-plugin-sql-sqlite" )
            dictionary[ "SQL_SQLITE" ] = "plugin";
        else if( configCmdLine.at(i) == "-no-sql-sqlite" )
            dictionary[ "SQL_SQLITE" ] = "no";

        else if( configCmdLine.at(i) == "-qt-sql-ibase" )
            dictionary[ "SQL_IBASE" ] = "yes";
        else if( configCmdLine.at(i) == "-plugin-sql-ibase" )
            dictionary[ "SQL_IBASE" ] = "plugin";
        else if( configCmdLine.at(i) == "-no-sql-ibase" )
            dictionary[ "SQL_IBASE" ] = "no";
#endif
        // IDE project generation -----------------------------------
	else if( configCmdLine.at(i) == "-no-dsp" )
	    dictionary[ "DSPFILES" ] = "no";
	else if( configCmdLine.at(i) == "-dsp" )
	    dictionary[ "DSPFILES" ] = "yes";

	else if( configCmdLine.at(i) == "-no-vcp" )
	    dictionary[ "VCPFILES" ] = "no";
	else if( configCmdLine.at(i) == "-vcp" )
	    dictionary[ "VCPFILES" ] = "yes";

	else if( configCmdLine.at(i) == "-no-vcproj" )
	    dictionary[ "VCPROJFILES" ] = "no";
	else if( configCmdLine.at(i) == "-vcproj" )
	    dictionary[ "VCPROJFILES" ] = "yes";

#if !defined(EVAL)
        // Others ---------------------------------------------------
	else if( configCmdLine.at(i) == "-lean" )
	    dictionary[ "LEAN" ] = "yes";

        else if (configCmdLine.at(i) == "-fast" )
            dictionary[ "FAST" ] = "yes";

	else if( configCmdLine.at(i) == "-stl" )
	    dictionary[ "STL" ] = "yes";
	else if( configCmdLine.at(i) == "-no-stl" )
	    dictionary[ "STL" ] = "no";

	else if ( configCmdLine.at(i) == "-exceptions" )
	    dictionary[ "EXCEPTIONS" ] = "yes";
	else if ( configCmdLine.at(i) == "-no-exceptions" )
	    dictionary[ "EXCEPTIONS" ] = "no";

	else if ( configCmdLine.at(i) == "-rtti" )
	    dictionary[ "RTTI" ] = "yes";
	else if ( configCmdLine.at(i) == "-no-rtti" )
	    dictionary[ "RTTI" ] = "no";

	else if( configCmdLine.at(i) == "-accessibility" )
	    dictionary[ "ACCESSIBILITY" ] = "yes";
	else if( configCmdLine.at(i) == "-no-accessibility" )
	    dictionary[ "ACCESSIBILITY" ] = "no";

	else if( configCmdLine.at(i) == "-no-big-codecs" )
	    dictionary[ "BIG_CODECS" ] = "no";
	else if( configCmdLine.at(i) == "-big-codecs" )
	    dictionary[ "BIG_CODECS" ] = "yes";

	else if( configCmdLine.at(i) == "-internal" )
	    dictionary[ "QMAKE_INTERNAL" ] = "yes";

	else if( configCmdLine.at(i) == "-no-qmake" )
	    dictionary[ "BUILD_QMAKE" ] = "no";

	else if( configCmdLine.at(i) == "-dont-process" )
	    dictionary[ "NOPROCESS" ] = "yes";

	else if( configCmdLine.at(i) == "-qmake-deps" )
	    dictionary[ "DEPENDENCIES" ] = "yes";

	else if( configCmdLine.at(i) == "-D" ) {
	    ++i;
	    if (i==argCount)
		break;
            qmakeDefines += configCmdLine.at(i);
        } else if( configCmdLine.at(i) == "-I" ) {
	    ++i;
	    if (i==argCount)
		break;
	    qmakeIncludes += configCmdLine.at(i);
	} else if( configCmdLine.at(i) == "-L" ) {
	    ++i;
	    if (i==argCount)
		break;
	    qmakeLibs += configCmdLine.at(i);
	}

        else if( ( configCmdLine.at(i) == "-override-version" ) || ( configCmdLine.at(i) == "-version-override" ) ){
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "VERSION" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-saveconfig" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "CUSTOMCONFIG" ] = "_" + configCmdLine.at(i);
	}

        // Directories ----------------------------------------------
	else if( configCmdLine.at(i) == "-prefix" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_PREFIX" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-headerdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_HEADERS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-docdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_DOCS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-plugindir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_PLUGINS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-datadir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_DATA" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-translationdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_TRANSLATIONS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i).indexOf( QRegExp( "^-(en|dis)able-" ) ) != -1 ) {
	    // Scan to see if any specific modules and drivers are enabled or disabled
	    for( QStringList::Iterator module = modules.begin(); module != modules.end(); ++module ) {
		if( configCmdLine.at(i) == QString( "-enable-" ) + (*module) ) {
		    enabledModules += (*module);
		    break;
		}
		else if( configCmdLine.at(i) == QString( "-disable-" ) + (*module) ) {
		    disabledModules += (*module);
		    break;
		}
	    }
	}

	else {
	    dictionary[ "HELP" ] = "yes";
	    cout << "Unknown option " << configCmdLine.at(i) << endl;
	    break;
	}

#endif
    }

    if( dictionary[ "QMAKESPEC" ].endsWith( "-msvc" ) ||
	dictionary[ "QMAKESPEC" ].endsWith( ".net" ) ||
	dictionary[ "QMAKESPEC" ].endsWith( "-icc" ) ||
    dictionary[ "QMAKESPEC" ] == QString( "win32-msvc2005" )) {
		dictionary[ "MAKE" ] = "nmake";
 		dictionary[ "QMAKEMAKEFILE" ] = "Makefile";
    } else if ( dictionary[ "QMAKESPEC" ] == QString( "win32-g++" ) ) {
	    dictionary[ "MAKE" ] = "mingw32-make";
    	dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32-g++";
    } else {
	    dictionary[ "MAKE" ] = "make";
	    dictionary[ "QMAKEMAKEFILE" ] = "Makefile";
    }

#if !defined(EVAL)
    for( QStringList::Iterator dis = disabledModules.begin(); dis != disabledModules.end(); ++dis ) {
	modules.removeAll( (*dis) );
    }
    for( QStringList::Iterator ena = enabledModules.begin(); ena != enabledModules.end(); ++ena ) {
	if( modules.indexOf( (*ena) ) == -1 )
	    modules += (*ena);
    }
    qtConfig += modules;

    for( QStringList::Iterator it = disabledModules.begin(); it != disabledModules.end(); ++it )
	qtConfig.removeAll(*it);

    if( ( dictionary[ "REDO" ] != "yes" ) && ( dictionary[ "HELP" ] != "yes" ) )
	saveCmdLine();
#endif
}

#if !defined(EVAL)
void Configure::validateArgs()
{
    QStringList configs;
    // Validate the specified config

    allConfigs = QStringList() << "minimal" << "small" <<  "medium" << "large" << "full";

    QStringList::Iterator config;
    for( config = allConfigs.begin(); config != allConfigs.end(); ++config ) {
	configs += (*config) + "-config";
	if( (*config) == dictionary[ "QCONFIG" ] )
	    break;
    }
    if( config == allConfigs.end() ) {
	dictionary[ "HELP" ] = "yes";
	cout << "No such configuration \"" << qPrintable(dictionary[ "QCONFIG" ]) << "\"" << endl ;
    }
    else
	qmakeConfig += configs;
}
#endif

bool Configure::displayHelp()
{
    if( dictionary[ "HELP" ] == "yes" ) {
	cout << endl << endl;
	cout << "Command line arguments:  (* indicates default behaviour)" << endl << endl;
	cout << "-help                Bring up this help text." << endl << endl;

#if !defined(EVAL)
	cout << "-debug             " << MARK_OPTION(DEBUG,yes)	    << " Enable debug information." << endl;
	cout << "-release           " << MARK_OPTION(DEBUG,no)	    << " Disable debug information." << endl << endl;

	cout << "-shared            " << MARK_OPTION(SHARED,yes)    << " Build Qt as a shared library." << endl;
	cout << "-static            " << MARK_OPTION(SHARED,no)	    << " Build Qt as a static library." << endl << endl;
#endif

	cout << "-spec                Specify a platform, uses %QMAKESPEC% as default." << endl;
        cout << "-arch architecture   Specify an architecture:" << endl;
        cout << "                         " << MARK_OPTION(ARCHITECTURE,windows) << " windows" << endl;
        cout << "                         " << MARK_OPTION(ARCHITECTURE,boundschecker) << " boundschecker" << endl << endl;
#if !defined(EVAL)
	cout << "-qconfig             Specify config, available configs:" << endl;
	for (int i=0; i<allConfigs.size(); ++i)
	    cout << "                         " << qPrintable(allConfigs.at(i)) << endl;

	cout << endl;

	cout << "-no-zlib           " << MARK_OPTION(ZLIB,no)	    << " Disable zlib.  Implies -no-png." << endl;
	cout << "-qt-zlib           " << MARK_OPTION(ZLIB,yes)	    << " Compile in zlib." << endl;
	cout << "-system-zlib       " << MARK_OPTION(ZLIB,system)   << " Use existing zlib in system." << endl << endl;

	cout << "-plugin-imgfmt-<format> Enable format (png, jpeg, gif) to" << endl;
	cout << "                        be linked to at runtime." << endl;
	cout << "-qt-imgfmt-<format>     Enable format (png, jpeg) to" << endl;
	cout << "                        be linked into Qt." << endl;
	cout << "-no-imgfmt-<format>     Fully disable format (png, jpeg, gif)" << endl;
	cout << "                        from Qt." << endl << endl;

	cout << "-qt-png            " << MARK_OPTION(LIBPNG,qt)	    << " Use the libpng bundled with Qt." << endl;
	cout << "-system-png        " << MARK_OPTION(LIBPNG,system) << " Use existing libPNG in system." << endl  << endl;

	//cout << "-qt-mng            " << MARK_OPTION(LIBMNG,qt)	    << " Use the libmng bundled with Qt." << endl;
	//cout << "-system-mng        " << MARK_OPTION(LIBMNG,system) << " Use existing libMNG in system." << endl << endl;

	cout << "-qt-jpeg           " << MARK_OPTION(LIBJPEG,qt)    << " Use the libjpeg bundled with Qt." << endl;
	cout << "-system-jpeg       " << MARK_OPTION(LIBJPEG,system)<< " Use existing libJPEG in system" << endl << endl;

	cout << "-stl               " << MARK_OPTION(STL,yes)	    << " Enable STL support." << endl;
	cout << "-no-stl            " << MARK_OPTION(STL,no)	    << " Disable STL support." << endl  << endl;

	cout << "-exceptions        " << MARK_OPTION(EXCEPTIONS,yes)<< " Enable C++ exception support." << endl;
	cout << "-no-exceptions     " << MARK_OPTION(EXCEPTIONS,no) << " Disable C++ exception support." << endl  << endl;

	cout << "-rtti              " << MARK_OPTION(RTTI,yes)	    << " Enable runtime type information." << endl;
	cout << "-no-rtti           " << MARK_OPTION(RTTI,no)	    << " Disable runtime type information." << endl  << endl;

	cout << "-accessibility     " << MARK_OPTION(ACCESSIBILITY,yes) << " Enable Windows Active Accessibility." << endl;
	cout << "-no-accessibility  " << MARK_OPTION(ACCESSIBILITY,no)  << " Disable Windows Active Accessibility." << endl  << endl;

	cout << "-big-codecs        " << MARK_OPTION(BIG_CODECS,yes)<< " Enable the building of big codecs." << endl;
	cout << "-no-big-codecs     " << MARK_OPTION(BIG_CODECS,no) << " Disable the building of big codecs." << endl << endl;

	cout << "-dsp               " << MARK_OPTION(DSPFILES,yes)   << " Enable the generation of VC++ .DSP-files." << endl;
	cout << "-no-dsp            " << MARK_OPTION(DSPFILES,no)  << " Disable the generation of VC++ .DSP-files." << endl << endl;
#endif

	// Only show the VCP generation options for CE users for now
WCE( {	cout << "-vcp               " << MARK_OPTION(VCPFILES,yes)   << " Enable the generation of eMbedded VC++ .VCP-files." << endl;
	cout << "-no-vcp            " << MARK_OPTION(VCPFILES,no)  << " Disable the generation of eMbedded VC++ .VCP-files." << endl << endl; } );

	cout << "-vcproj            " << MARK_OPTION(VCPROJFILES,yes) << " Enable the generation of VC++ .VCPROJ-files." << endl;
	cout << "-no-vcproj         " << MARK_OPTION(VCPROJFILES,no) << " Disable the generation of VC++ .VCPROJ-files." << endl << endl;

#if !defined(EVAL)
	cout << "-no-qmake            Do not build qmake." << endl;
	cout << "-lean                Only process the Qt core projects." << endl;
	cout << "                     (src directory)." << endl << endl;
        cout << "-fast                Configure Qt quickly by generating wrapper" << endl
             << "                     Makefiles which will run qmake later." << endl << endl;

	cout << "-D <define>          Add <define> to the list of defines." << endl;
	cout << "-I <includepath>     Add <includepath> to the include searchpath." << endl;
	cout << "-L <library>         Add <library> to the library list." << endl << endl;

	cout << "-qt-sql-*            Build the specified Sql driver into Qt" << endl;
	cout << "-plugin-sql-*        Build the specified Sql driver into a plugin" << endl;
	cout << "-no-sql-*          * Don't build the specified Sql driver" << endl;
	cout << "                     where sql driver is one of" << endl;
	cout << "                         mysql" << endl;
	cout << "                         psql" << endl;
	cout << "                         oci" << endl;
	cout << "                         odbc" << endl;
	cout << "                         tds" << endl;
	cout << "                         db2" << endl;
	cout << "                         sqlite" << endl;
	cout << "                         ibase" << endl << endl;

	cout << "-qt-style-*        * Build the specified style into Qt" << endl;
	cout << "-plugin-style-*      Build the specified style into a plugin" << endl;
	cout << "-no-style-*          Don't build the specified style" << endl;
	cout << "                     where style is one of" << endl;
	// Only show the PocketPC style option for CE users
WCE( {	cout << "                         pocketpc" << endl; } );
	cout << "                         windows" << endl;
	cout << "                         windowsxp" << endl;
	cout << "                         motif" << endl;
	cout << "                         cde" << endl;
	cout << "                         sgi" << endl;
	cout << "                         motifplus" << endl;
	cout << "                         platinum" << endl << endl;

	cout << "-prefix <prefix>      Install Qt to <prefix>, defaults to current directory." << endl;
	cout << "-docdir <docs>        Install documentation to <docs>, default <prefix>/doc." << endl;
	cout << "-headerdir <headers>  Install Qt headers to <header>, default <prefix>/include." << endl;
	cout << "-plugindir <plugins>  Install Qt plugins to <plugins>, default <prefix>/plugins." << endl;
	cout << "-datadir <data>       Data used by Qt programs will be installed to <data>." << endl;
	cout << "-translationdir <dir> Translations used by Qt programs will be installed to dir." << endl << "(default PREFIX/translations)" << endl;
	cout << "                     Default <prefix>." << endl;

	cout << "-redo                Run configure with the same parameters as last time." << endl;
	cout << "-saveconfig <config> Run configure and save the parameters as <config>." << endl;
	cout << "                     The file will be called configure_<config>.cache" << endl;
	cout << "-loadconfig <config> Run configure with the parameters from <config>." << endl;
	cout << "                     The file must be called configure_<config>.cache" << endl;
#endif
	return true;
    }
    return false;
}

bool Configure::findFileInPaths(const QString &fileName, const QStringList &paths)
{
    QDir d;
    for( QStringList::ConstIterator it = paths.begin(); it != paths.end(); ++it ) {
        // Remove any leading or trailing ", this is commonly used in the environment
        // variables
        QString path = (*it);
        if ( path.startsWith( "\"" ) )
            path = path.right( path.length() - 1 );
        if ( path.endsWith( "\"" ) )
	    path = path.left( path.length() - 1 );
        if( d.exists( path + QDir::separator() + fileName ) )
	    return true;
    }
    return false;
}

bool Configure::findFile( const QString &fileName )
{
    QString file = fileName.toLower();
    QStringList paths;
#if defined(Q_OS_WIN32)
    QRegExp splitReg("[;,]");
#else
    QRegExp splitReg("[:]");
#endif
    if (file.endsWith(".h"))
        paths = QString::fromLocal8Bit(getenv("INCLUDE")).split(splitReg, QString::SkipEmptyParts);
    else if ( file.endsWith( ".lib" ) )
        paths = QString::fromLocal8Bit(getenv("LIB")).split(splitReg, QString::SkipEmptyParts);
    else
        paths = QString::fromLocal8Bit(getenv("PATH")).split(splitReg, QString::SkipEmptyParts);
    return findFileInPaths(file, paths);
}

void Configure::autoDetection()
{
    if (dictionary["STYLE_WINDOWSXP"] == "auto")
        dictionary["STYLE_WINDOWSXP"] = findFile(QLatin1String("uxtheme.h")) ? "plugin" : "no";
}

void Configure::generateOutputVars()
{
    // Generate variables for output
    if( dictionary[ "DEBUG" ] == "yes" ) {
	qmakeConfig += "debug";
	dictionary[ "QMAKE_OUTDIR" ] = "debug";
    } else {
	qmakeConfig += "release";
	dictionary[ "QMAKE_OUTDIR" ] = "release";
    }

    if( dictionary[ "ACCESSIBILITY" ] == "yes" ) {
	qtConfig += "accessibility";
    }

    if( dictionary[ "SHARED" ] == "yes" ) {
	dictionary[ "QMAKE_OUTDIR" ] += "_shared";
    } else {
	dictionary[ "QMAKE_OUTDIR" ] += "_static";
    }

    if( !qmakeLibs.isEmpty() ) {
	qmakeVars += "LIBS += " + qmakeLibs.join( " " );
    }

    if( dictionary[ "ZLIB" ] == "yes" )
	qtConfig += "zlib";
    else if( dictionary[ "ZLIB" ] == "no" )
	qtConfig += "no-zlib";


    if( dictionary[ "GIF" ] == "no" )
	qtConfig += "no-gif";
    else if( dictionary[ "GIF" ] == "qt" )
	qtConfig += "gif";
    else if( dictionary[ "GIF" ] == "plugin" )
	qmakeFormatPlugins += "gif";

    if( dictionary[ "JPEG" ] == "no" )
	qtConfig += "no-jpeg";
    else if( dictionary[ "JPEG" ] == "qt" )
	qtConfig += "jpeg";
    else if( dictionary[ "JPEG" ] == "plugin" )
	qmakeFormatPlugins += "jpeg";

    if( dictionary[ "LIBJPEG" ] == "system" )
	qtConfig += "system-jpeg";

    if (dictionary[ "QT3SUPPORT" ] == "yes")
        qtConfig += "qt3support";

    if (dictionary[ "OPENGL" ] == "yes")
        qtConfig += "opengl";

 //   if( dictionary[ "MNG" ] == "no" )
	//qtConfig += "no-mng";
 //   else if( dictionary[ "MNG" ] == "qt" )
	//qtConfig += "mng";
 //   else if( dictionary[ "MNG" ] == "plugin" )
	//qmakeFormatPlugins += "mng";

 //   if( dictionary[ "LIBMNG" ] == "system" )
	//qtConfig += "system-mng";

    if( dictionary[ "PNG" ] == "no" )
	qtConfig += "no-png";
    else if( dictionary[ "PNG" ] == "qt" )
	qtConfig += "png";
    else if( dictionary[ "PNG" ] == "plugin" )
	qmakeFormatPlugins += "png";

    if( dictionary[ "LIBPNG" ] == "system" )
	qtConfig += "system-png";

    if( dictionary[ "BIG_CODECS" ] == "yes" )
	qtConfig += "bigcodecs";
    else if( dictionary[ "BIG_CODECS" ] == "no" )
	qtConfig += "no-bigcodecs";

    if (dictionary["IPV6"] == "yes")
        qtConfig += "ipv6";
    else if (dictionary["IPV6"] == "no")
        qtConfig += "no-ipv6";

    if ( dictionary[ "STYLE_WINDOWS" ] == "yes" )
	qmakeStyles += "windows";
    else if ( dictionary[ "STYLE_WINDOWS" ] == "plugin" )
	qmakeStylePlugins += "windows";

    if ( dictionary[ "STYLE_WINDOWSXP" ] == "yes" )
	qmakeStyles += "windowsxp";
    else if ( dictionary[ "STYLE_WINDOWSXP" ] == "plugin" )
	qmakeStylePlugins += "windowsxp";

    if ( dictionary[ "STYLE_MOTIF" ] == "yes" )
	qmakeStyles += "motif";
    else if ( dictionary[ "STYLE_MOTIF" ] == "plugin" )
	qmakeStylePlugins += "motif";

    if ( dictionary[ "STYLE_MOTIFPLUS" ] == "yes" )
	qmakeStyles += "motifplus";
    else if ( dictionary[ "STYLE_MOTIFPLUS" ] == "plugin" )
	qmakeStylePlugins += "motifplus";

    if ( dictionary[ "STYLE_PLATINUM" ] == "yes" )
	qmakeStyles += "platinum";
    else if ( dictionary[ "STYLE_PLATINUM" ] == "plugin" )
	qmakeStylePlugins += "platinum";

    if ( dictionary[ "STYLE_SGI" ] == "yes" )
	qmakeStyles += "sgi";
    else if ( dictionary[ "STYLE_SGI" ] == "plugin" )
	qmakeStylePlugins += "sgi";

    if ( dictionary[ "STYLE_POCKETPC" ] == "yes" )
	qmakeStyles += "pocketpc";
    else if ( dictionary[ "STYLE_POCKETPC" ] == "plugin" )
	qmakeStylePlugins += "pocketpc";

    if ( dictionary[ "STYLE_CDE" ] == "yes" )
	qmakeStyles += "cde";
    else if ( dictionary[ "STYLE_CDE" ] == "plugin" )
	qmakeStylePlugins += "cde";

    if ( dictionary[ "SQL_MYSQL" ] == "yes" )
	qmakeSql += "mysql";
    else if ( dictionary[ "SQL_MYSQL" ] == "plugin" )
	qmakeSqlPlugins += "mysql";

    if ( dictionary[ "SQL_ODBC" ] == "yes" )
	qmakeSql += "odbc";
    else if ( dictionary[ "SQL_ODBC" ] == "plugin" )
	qmakeSqlPlugins += "odbc";

    if ( dictionary[ "SQL_OCI" ] == "yes" )
	qmakeSql += "oci";
    else if ( dictionary[ "SQL_OCI" ] == "plugin" )
	qmakeSqlPlugins += "oci";

    if ( dictionary[ "SQL_PSQL" ] == "yes" )
	qmakeSql += "psql";
    else if ( dictionary[ "SQL_PSQL" ] == "plugin" )
	qmakeSqlPlugins += "psql";

    if ( dictionary[ "SQL_TDS" ] == "yes" )
	qmakeSql += "tds";
    else if ( dictionary[ "SQL_TDS" ] == "plugin" )
	qmakeSqlPlugins += "tds";

    if ( dictionary[ "SQL_DB2" ] == "yes" )
	qmakeSql += "db2";
    else if ( dictionary[ "SQL_DB2" ] == "plugin" )
	qmakeSqlPlugins += "db2";

    if ( dictionary[ "SQL_SQLITE" ] == "yes" )
        qmakeSql += "sqlite";
    else if ( dictionary[ "SQL_SQLITE" ] == "plugin" )
        qmakeSqlPlugins += "sqlite";

    if ( dictionary[ "SQL_IBASE" ] == "yes" )
        qmakeSql += "ibase";
    else if ( dictionary[ "SQL_IBASE" ] == "plugin" )
        qmakeSqlPlugins += "ibase";

    if ( dictionary[ "SHARED" ] == "yes" ) {
	QString version = dictionary[ "VERSION" ];
	version.remove(QLatin1Char('.'));
	qmakeVars += "QMAKE_QT_VERSION_OVERRIDE=" + version;
    }

    if( !dictionary[ "QT_INSTALL_HEADERS" ].size() )
	dictionary[ "QT_INSTALL_HEADERS" ] = QDir::convertSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/include" );

    if( !dictionary[ "QT_INSTALL_DOCS" ].size() )
	dictionary[ "QT_INSTALL_DOCS" ] = QDir::convertSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/doc" );

    if( !dictionary[ "QT_INSTALL_PLUGINS" ].size() )
	dictionary[ "QT_INSTALL_PLUGINS" ] = QDir::convertSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/plugins" );

    if( !dictionary[ "QT_INSTALL_LIBS" ].size() )
	dictionary[ "QT_INSTALL_LIBS" ] = QDir::convertSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/lib" );

    if( !dictionary[ "QT_INSTALL_BINS" ].size() )
	dictionary[ "QT_INSTALL_BINS" ] = QDir::convertSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/bin" );

    if( !dictionary[ "QT_INSTALL_DATA" ].size() )
	dictionary[ "QT_INSTALL_DATA" ] = QDir::convertSeparators( dictionary[ "QT_INSTALL_PREFIX" ] );

    qmakeVars += QString( "OBJECTS_DIR=" ) + QDir::convertSeparators( "tmp/obj/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString( "MOC_DIR=" ) + QDir::convertSeparators( "tmp/moc/" + dictionary[ "QMAKE_OUTDIR" ] );

    qmakeVars += QString( "DEFINES+=" ) + qmakeDefines.join( " " );
    qmakeVars += QString( "INCLUDEPATH+=" ) + qmakeIncludes.join( " " );
    qmakeVars += QString( "sql-drivers+=" ) + qmakeSql.join( " " );
    qmakeVars += QString( "sql-plugins+=" ) + qmakeSqlPlugins.join( " " );
    qmakeVars += QString( "styles+=" ) + qmakeStyles.join( " " );
    qmakeVars += QString( "style-plugins+=" ) + qmakeStylePlugins.join( " " );
    qmakeVars += QString( "imageformat-plugins+=" ) + qmakeFormatPlugins.join( " " );

    if( !dictionary[ "QMAKESPEC" ].length() ) {
	cout << "QMAKESPEC must either be defined as an environment variable, or specified" << endl;
	cout << "as an argument with -spec" << endl;
	dictionary[ "HELP" ] = "yes";

	QStringList winPlatforms;
	QDir mkspecsDir( dictionary[ "QT_SOURCE_TREE" ] + "\\mkspecs" );
	const QFileInfoList &specsList = mkspecsDir.entryInfoList();
	for(int i = 0; i < specsList.size(); ++i) {
    	    const QFileInfo &fi = specsList.at(i);
	    if( fi.fileName().left( 5 ) == "win32" ) {
		winPlatforms += fi.fileName();
	    }
	}
	cout << "Available platforms are: " << qPrintable(winPlatforms.join( ", " )) << endl;
    }
}

#if !defined(EVAL)
void Configure::generateCachefile()
{
    // Generate .qmake.cache
    QFile cacheFile( dictionary[ "QT_SOURCE_TREE" ] + "\\.qmake.cache" );
    if( cacheFile.open( QFile::WriteOnly | QFile::Text ) ) { // Truncates any existing file.
	QTextStream cacheStream( &cacheFile );
        for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
	    cacheStream << (*var) << endl;
	}
	cacheStream << "CONFIG+=" << qmakeConfig.join( " " ) << " incremental create_prl link_prl depend_includepath" << endl;
	cacheStream << "QMAKESPEC=" << dictionary[ "QMAKESPEC" ] << endl;
        cacheStream << "ARCH=" << dictionary[ "ARCHITECTURE" ] << endl;
	cacheStream << "QT_BUILD_TREE=" << dictionary[ "QT_SOURCE_TREE" ] << endl;
	cacheStream << "QT_SOURCE_TREE=" << dictionary[ "QT_SOURCE_TREE" ] << endl;
	cacheStream << "QT_INSTALL_PREFIX=" << dictionary[ "QT_INSTALL_PREFIX" ] << endl;
	cacheStream << "QT_INSTALL_TRANSLATIONS=" << dictionary[ "QT_INSTALL_TRANSLATIONS" ] << endl;
	cacheStream << "docs.path=" << dictionary[ "QT_INSTALL_DOCS" ] << endl;
	cacheStream << "headers.path=" << dictionary[ "QT_INSTALL_HEADERS" ] << endl;
	cacheStream << "plugins.path=" << dictionary[ "QT_INSTALL_PLUGINS" ] << endl;
	cacheStream << "libs.path=" << dictionary[ "QT_INSTALL_LIBS" ] << endl;
	cacheStream << "bins.path=" << dictionary[ "QT_INSTALL_BINS" ] << endl;
	cacheStream << "data.path=" << dictionary[ "QT_INSTALL_DATA" ] << endl;
	cacheStream << "translations.path=" << dictionary[ "QT_INSTALL_TRANSLATIONS" ] << endl;
        cacheStream.flush();
	cacheFile.close();
    }
    QFile configFile( dictionary[ "QT_SOURCE_TREE" ] + "\\mkspecs\\qconfig.pri" );
    if( configFile.open( QFile::WriteOnly | QFile::Text ) ) { // Truncates any existing file.
	QTextStream configStream( &configFile );
	configStream << "CONFIG+=";
	if( dictionary[ "SHARED" ] == "yes" )
	    configStream << " shared";
        else
	    configStream << " static";
	if ( dictionary[ "DEBUG" ] == "yes" )
	    configStream << " debug";
	else
	    configStream << " release";
	if( dictionary[ "STL" ] == "yes" )
	    configStream << " stl";
	if ( dictionary[ "EXCEPTIONS" ] == "yes" )
	    configStream << " exceptions";
	if ( dictionary[ "RTTI" ] == "yes" )
	    configStream << " rtti";
	configStream << endl;
        configStream << "QT_CONFIG += " << qtConfig.join(" ") << endl;
        if (licenseInfo[ "PRODUCTS" ].length())
	    configStream << "QT_PRODUCT = " << licenseInfo["PRODUCTS"];
        configStream.flush();
	configFile.close();
    }
}
#endif

QString Configure::addDefine(QString def)
{
    QString result, defNeg, defD = def;

    defD.replace(QRegExp("=.*"), "");
    def.replace(QRegExp("="), " ");

    if(def.startsWith("QT_NO_")) {
        defNeg = defD;
        defNeg.replace("QT_NO_", "QT_");
    } else if(def.startsWith("QT_")) {
        defNeg = defD;
        defNeg.replace("QT_", "QT_NO_");
    }

    if (defNeg.isEmpty()) {
        result = "#ifndef $DEFD\n"
                 "# define $DEF\n"
                 "#endif\n\n";
    } else {
        result = "#if defined($DEFD) && defined($DEFNEG)\n"
                 "# undef $DEFD\n"
                 "#elif !defined($DEFD)\n"
                 "# define $DEF\n"
                 "#endif\n\n";
    }
    result.replace("$DEFNEG", defNeg);
    result.replace("$DEFD", defD);
    result.replace("$DEF", def);
    return result;
}

#if !defined(EVAL)
void Configure::generateConfigfiles()
{
    QString outDir(dictionary[ "QT_SOURCE_TREE" ] + "/src/corelib/global");
    QString outName( outDir + "/qconfig.h" );

    ::SetFileAttributesA( outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    QFile::remove( outName );
    QFile outFile( outName );

    if(outFile.open(QFile::WriteOnly | QFile::Text)) {
	outStream.setDevice(&outFile);

	if( dictionary[ "QCONFIG" ] == "full" ) {
	    outStream << "// Everything" << endl << endl;
	    if( dictionary[ "SHARED" ] == "yes" ) {
		outStream << "#ifndef QT_DLL" << endl;
		outStream << "#define QT_DLL" << endl;
		outStream << "#endif" << endl;
	    }
	} else {
	    QString configName( "qconfig-" + dictionary[ "QCONFIG" ] + ".h" );
	    outStream << "// Copied from " << configName << endl;

	    QFile inFile( dictionary[ "QT_SOURCE_TREE" ] + "/src/corelib/global/" + configName );
	    if( inFile.open( QFile::ReadOnly ) ) {
		QByteArray buffer = inFile.readAll();
		outFile.write( buffer.constData(), buffer.size() );
		inFile.close();
	    }
	}
	outStream << endl;
	outStream << "/* License information */" << endl;
	outStream << "#define QT_PRODUCT_LICENSEE \"" << licenseInfo[ "LICENSEE" ] << "\"" << endl;
	outStream << "#define QT_PRODUCT_LICENSE \"" << licenseInfo[ "PRODUCTS" ] << "\"" << endl;

	outStream << endl;
	outStream << "/* Machine byte-order */" << endl;
	outStream << "#define Q_BIG_ENDIAN 4321" << endl;
	outStream << "#define Q_LITTLE_ENDIAN 1234" << endl;
	if ( QSysInfo::ByteOrder == QSysInfo::BigEndian )
	    outStream << "#define Q_BYTE_ORDER Q_BIG_ENDIAN" << endl;
	else
	    outStream << "#define Q_BYTE_ORDER Q_LITTLE_ENDIAN" << endl;

	outStream << endl << "// Compile time features" << endl;

        QStringList qconfigList;
        if(dictionary["STL"] == "no")                qconfigList += "QT_NO_STL";
        if(dictionary["STYLE_WINDOWS"] != "yes")     qconfigList += "QT_NO_STYLE_WINDOWS";
	if(dictionary["STYLE_WINDOWSXP"] != "yes")   qconfigList += "QT_NO_STYLE_WINDOWSXP";
	if(dictionary["STYLE_MOTIF"] != "yes")       qconfigList += "QT_NO_STYLE_MOTIF";
	if(dictionary["STYLE_MOTIFPLUS"] != "yes")   qconfigList += "QT_NO_STYLE_MOTIFPLUS";
	if(dictionary["STYLE_PLATINUM"] != "yes")    qconfigList += "QT_NO_STYLE_PLATINUM";
	if(dictionary["STYLE_SGI"] != "yes")         qconfigList += "QT_NO_STYLE_SGI";
	if(dictionary["STYLE_CDE"] != "yes")         qconfigList += "QT_NO_STYLE_CDE";
	if(dictionary["STYLE_COMPACT"] != "yes")     qconfigList += "QT_NO_STYLE_COMPACT";
	if(dictionary["STYLE_POCKETPC"] != "yes")    qconfigList += "QT_NO_STYLE_POCKETPC";
	if(dictionary["STYLE_MAC"] != "yes")         qconfigList += "QT_NO_STYLE_MAC";

        if(dictionary["GIF"] == "yes")              qconfigList += "QT_BUILTIN_GIF_READER=1";
        if(dictionary["PNG"] == "no")               qconfigList += "QT_NO_IMAGEIO_PNG";
        if(dictionary["JPEG"] == "no")              qconfigList += "QT_NO_IMAGEIO_JPEG";
        if(dictionary["MNG"] == "no")               qconfigList += "QT_NO_IMAGEIO_MNG";
        if(dictionary["ZLIB"] == "no") {
            qconfigList += "QT_NO_ZLIB";
            qconfigList += "QT_NO_COMPRESS";
        }

        if(dictionary["QT3SUPPORT"] == "no")        qconfigList += "QT_NO_QT3SUPPORT";
        if(dictionary["ACCESSIBILITY"] == "no")     qconfigList += "QT_NO_ACCESSIBILITY";
        if(dictionary["EXCEPTIONS"] == "no")        qconfigList += "QT_NO_EXCEPTIONS";
        if(dictionary["BIG_CODECS"] == "no")        qconfigList += "QT_NO_BIG_CODECS";
        if(dictionary["OPENGL"] == "no")            qconfigList += "QT_NO_OPENGL";
        if(dictionary["IPV6"] == "no")              qconfigList += "QT_NO_IPV6";

        if(dictionary["SQL_MYSQL"] == "yes")        qconfigList += "QT_SQL_MYSQL";
        if(dictionary["SQL_ODBC"] == "yes")         qconfigList += "QT_SQL_ODBC";
        if(dictionary["SQL_OCI"] == "yes")          qconfigList += "QT_SQL_OCI";
        if(dictionary["SQL_PSQL"] == "yes")         qconfigList += "QT_SQL_PSQL";
        if(dictionary["SQL_TDS"] == "yes")          qconfigList += "QT_SQL_TDS";
        if(dictionary["SQL_DB2"] == "yes")          qconfigList += "QT_SQL_DB2";
        if(dictionary["SQL_SQLITE"] == "yes")       qconfigList += "QT_SQL_SQLITE";
        if(dictionary["SQL_IBASE"] == "yes")        qconfigList += "QT_SQL_IBASE";

        qconfigList.sort();
        for (int i = 0; i < qconfigList.count(); ++i)
            outStream << addDefine(qconfigList.at(i));

        outStream.flush();
	outFile.close();
        if (!writeToFile("#include \"../../src/corelib/global/qconfig.h\"\n",
                         dictionary[ "QT_INSTALL_HEADERS" ] + "/QtCore/qconfig.h")
            || !writeToFile("#include \"../../src/corelib/global/qconfig.h\"\n",
                            dictionary[ "QT_INSTALL_HEADERS" ] + "/Qt/qconfig.h")) {
            dictionary["DONE"] = "error";
            return;
        }
    }

    QString archFile = dictionary[ "QT_SOURCE_TREE" ] + "/src/corelib/arch/" + dictionary[ "ARCHITECTURE" ] + "/arch/qatomic.h";
    QFileInfo archInfo(archFile);
    if (!archInfo.exists()) {
	qDebug("Architecture file %s does not exist!", qPrintable(archFile) );
        dictionary[ "DONE" ] = "error";
        return;
    }
    QDir archhelper;
    archhelper.mkdir(dictionary[ "QT_INSTALL_HEADERS" ] + "/QtCore/arch");
    if (!CopyFileA(archFile.toLocal8Bit(),
                   QString(dictionary[ "QT_INSTALL_HEADERS" ] + "/QtCore/arch/qatomic.h").toLocal8Bit(), FALSE))
	qDebug("Couldn't copy %s to include/arch", qPrintable(archFile) );
    if (!SetFileAttributesA(QString(dictionary[ "QT_INSTALL_HEADERS" ]
                                    + "/QtCore/arch/qatomic.h").toLocal8Bit(),
			    FILE_ATTRIBUTE_NORMAL))
	qDebug("Couldn't reset writable file attribute for qatomic.h");

    // Create qatomic.h "symlinks"
    QString atomicContents = QString("#include \"../../src/corelib/arch/" + dictionary[ "ARCHITECTURE" ] + "/arch/qatomic.h\"\n");
    if (!writeToFile(atomicContents.toLocal8Bit(),
                     dictionary[ "QT_INSTALL_HEADERS" ] + "/QtCore/arch/qatomic.h")
        || !writeToFile(atomicContents.toLocal8Bit(),
                        dictionary[ "QT_INSTALL_HEADERS" ] + "/Qt/arch/qatomic.h")) {
        dictionary[ "DONE" ] = "error";
        return;
    }

    outDir = dictionary[ "QT_SOURCE_TREE" ];

    // Generate the new qconfig.cpp file
    outName = outDir + "/src/corelib/global/qconfig.cpp";
    ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    outFile.setFileName(outName);
    if (outFile.open(QFile::WriteOnly | QFile::Text)) {
        outStream.setDevice(&outFile);

        outStream << "/* Licensed */" << endl
                  << "static const char qt_configure_licensee_str           [256] = \""
                  << licenseInfo["LICENSEE"] << "\";" << endl
                  << "static const char qt_configure_licensed_products_str  [256] = \""
                  << licenseInfo["PRODUCTS"] << "\";" << endl
                  << "/* strlen( \"qt_lcnsxxxx\" ) == 12 */" << endl
                  << "#define QT_CONFIGURE_LICENSEE qt_configure_licensee_str + 12;" << endl
                  << "#define QT_CONFIGURE_LICENSED_PRODUCTS qt_configure_licensed_products_str + 12;"
                  << endl;

        outStream.flush();
        outFile.close();
    }




    // Generate the qt.conf settings file used to get a hold of qt's directories at
    // runtime.
    outName = outDir + "/qt.conf";
    ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    QFile::remove( outName );
    outFile.setFileName( outName );

    if( outFile.open( QFile::WriteOnly | QFile::Text ) ) {
	outStream.setDevice(&outFile);
        outStream << "[Paths]" << endl;
	outStream << "Prefix = "
                  << QString(dictionary["QT_INSTALL_PREFIX"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Binaries = "
		  << QString(dictionary["QT_INSTALL_BINS"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Documentation = "
		  << QString(dictionary["QT_INSTALL_DOCS"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Headers = "
		  << QString(dictionary["QT_INSTALL_HEADERS"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Libraries = "
		  << QString(dictionary["QT_INSTALL_LIBS"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Plugins = "
		  << QString(dictionary["QT_INSTALL_PLUGINS"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Data = "
		  << QString(dictionary["QT_INSTALL_DATA"]).replace( "\\", "\\\\" )  << endl;
	outStream << "Translations = "
		  << QString(dictionary["QT_INSTALL_TRANSLATIONS"]).replace( "\\", "\\\\" )  << endl;
        outStream.flush();
	outFile.close();
    }

    outDir = dictionary[ "QT_SOURCE_TREE" ];
    outName = outDir + "/src/qt.rc";
    ::SetFileAttributesA( outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    QFile::remove( outName );
    outFile.setFileName( outName );

    if( outFile.open( QFile::WriteOnly | QFile::Text ) ) {
	outStream.setDevice(&outFile);

	QString version = dictionary["VERSION"];
	QString prodFile = "qt";
	if ( dictionary["SHARED"] == "yes" ) {
	    prodFile += version;
	    prodFile.remove('.');
	}
	prodFile += ".dll";
	QString prodVer = version;
	prodVer.replace('.', ',');

	QString internalName = licenseInfo["PRODUCTS"];

	outStream << "#ifndef Q_CC_BOR" << endl;
	outStream << "# if defined(UNDER_CE) && UNDER_CE >= 400" << endl;
	outStream << "#  include <winbase.h>" << endl;
	outStream << "# else" << endl;
	outStream << "#  include <winver.h>" << endl;
	outStream << "# endif" << endl;
	outStream << "#endif" << endl << endl;
	outStream << "VS_VERSION_INFO VERSIONINFO" << endl;
	outStream << "\tFILEVERSION " << prodVer << ",1" << endl;
	outStream << "\tPRODUCTVERSION " << prodVer << ",0" << endl;
	outStream << "\tFILEFLAGSMASK 0x3fL" << endl;
	outStream << "#ifdef _DEBUG" << endl;
	outStream << "\tFILEFLAGS VS_FF_DEBUG" << endl;
	outStream << "#else" << endl;
	outStream << "\tFILEFLAGS 0x0L" << endl;
	outStream << "#endif" << endl;
	outStream << "\tFILEOS VOS__WINDOWS32" << endl;
	outStream << "\tFILETYPE VFT_DLL" << endl;
	outStream << "\tFILESUBTYPE 0x0L" << endl;
	outStream << "\tBEGIN" << endl;
	outStream << "\t\tBLOCK \"StringFileInfo\"" << endl;
	outStream << "\t\tBEGIN" << endl;
	outStream << "\t\t\tBLOCK \"040904B0\"" << endl;
	outStream << "\t\t\tBEGIN" << endl;
	outStream << "\t\t\t\tVALUE \"CompanyName\", \"Trolltech AS\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"FileDescription\", \"Qt\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"FileVersion\", \"" << prodVer << ",1\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"InternalName\", \"" << internalName << "\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"LegalCopyright\", \"Copyright (C) 2003-$THISYEAR$ Trolltech AS\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"LegalTrademarks\", \"\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"OriginalFilename\", \"" << prodFile << "\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"ProductName\", \"Qt\\0\"" << endl;
	outStream << "\t\t\t\tVALUE \"ProductVersion\", \"" << version << "\\0\"" << endl;
	outStream << "\t\t\tEND" << endl;
	outStream << "\t\tEND" << endl;
	outStream << "\tEND" << endl;
	outStream << "/* End of Version info */" << endl << endl;
        outStream.flush();
    }
}
#endif

#if !defined(EVAL)
void Configure::displayConfig()
{
    // Give some feedback
    if( QFile::exists( dictionary[ "QT_SOURCE_TREE" ] + "/LICENSE.TROLL" ) ) {
	cout << "Trolltech license file used." << dictionary[ "QT_SOURCE_TREE" ] + "/LICENSE.TROLL" << endl;
    } else if ( !firstLicensePath().isEmpty() ) {
	QString l1 = licenseInfo[ "LICENSEE" ];
	QString l2 = licenseInfo[ "LICENSEID" ];
	QString l3 = licenseInfo[ "PRODUCTS" ];
	QString l4 = licenseInfo[ "EXPIRYDATE" ];
	cout << "Licensee...................." << (l1.isNull() ? "" : l1) << endl;
	cout << "License ID.................." << (l2.isNull() ? "" : l2) << endl;
	cout << "Product license............." << (l3.isNull() ? "" : l3) << endl;
	cout << "Expiry Date................." << (l4.isNull() ? "" : l4) << endl << endl;
    }

    cout << "QMAKESPEC..................." << dictionary[ "QMAKESPEC" ] << endl;
    cout << "Architecture................" << dictionary[ "ARCHITECTURE" ] << endl;
    cout << "Maketool...................." << dictionary[ "MAKE" ] << endl << endl;

    cout << "Environment:" << endl;
    QString env = QString::fromLocal8Bit(getenv("INCLUDE")).replace(QRegExp("[;,]"), "\r\n      ");
    if (env.isEmpty())
	env = "Unset";
    cout << "    INCLUDE=\r\n      " << env << endl;
    env = QString::fromLocal8Bit(getenv("LIB")).replace(QRegExp("[;,]"), "\r\n      ");
    if (env.isEmpty())
	env = "Unset";
    cout << "    LIB=\r\n      " << env << endl;
    env = QString::fromLocal8Bit(getenv("PATH")).replace(QRegExp("[;,]"), "\r\n      ");
    if (env.isEmpty())
	env = "Unset";
    cout << "    PATH=\r\n      " << env << endl;

    cout << "Configuration:" << endl;
    cout << "    " << qmakeConfig.join( "\r\n    " ) << endl;
    cout << "Qt Configuration:" << endl;
    cout << "    " << qtConfig.join( "\r\n    " ) << endl;

    cout << "Debug symbols..............." << dictionary[ "DEBUG" ] << endl;

    cout << "Accessibility support......." << dictionary[ "ACCESSIBILITY" ] << endl;
    cout << "Big Textcodecs.............." << dictionary[ "BIG_CODECS" ] << endl;
    cout << "STL support................." << dictionary[ "STL" ] << endl;
    cout << "Exception support..........." << dictionary[ "EXCEPTIONS" ] << endl;
    cout << "RTTI support................" << dictionary[ "RTTI" ] << endl;
    cout << "OpenGL support.............." << dictionary[ "OPENGL" ] << endl << endl;

    cout << "Image formats:" << endl;
    cout << "    GIF support............." << dictionary[ "GIF" ] << endl;
    cout << "    MNG support............." << dictionary[ "MNG" ] << endl;
    cout << "    JPEG support............" << dictionary[ "JPEG" ] << endl;
    cout << "    PNG support............." << dictionary[ "PNG" ] << endl << endl;

    cout << "Styles:" << endl;
WCE( { cout << "    PocketPC............." << dictionary[ "STYLE_POCKETPC" ] << endl; } );
    cout << "    Windows................." << dictionary[ "STYLE_WINDOWS" ] << endl;
    cout << "    Windows XP.............." << dictionary[ "STYLE_WINDOWSXP" ] << endl;
    cout << "    Motif..................." << dictionary[ "STYLE_MOTIF" ] << endl;
    cout << "    Platinum................" << dictionary[ "STYLE_PLATINUM" ] << endl;
    cout << "    MotifPlus..............." << dictionary[ "STYLE_MOTIFPLUS" ] << endl;
    cout << "    CDE....................." << dictionary[ "STYLE_CDE" ] << endl;
    cout << "    SGI....................." << dictionary[ "STYLE_SGI" ] << endl << endl;
    // Only show the PocketPC style option for CE users

    cout << "Sql Drivers:" << endl;
    cout << "    ODBC...................." << dictionary[ "SQL_ODBC" ] << endl;
    cout << "    MySQL..................." << dictionary[ "SQL_MYSQL" ] << endl;
    cout << "    OCI....................." << dictionary[ "SQL_OCI" ] << endl;
    cout << "    PostgreSQL.............." << dictionary[ "SQL_PSQL" ] << endl;
    cout << "    TDS....................." << dictionary[ "SQL_TDS" ] << endl;
    cout << "    DB2....................." << dictionary[ "SQL_DB2" ] << endl;
    cout << "    SQLite.................." << dictionary[ "SQL_SQLITE" ] << endl;
    cout << "    Interbase..............." << dictionary[ "SQL_IBASE" ] << endl << endl;

    cout << "Sources are in.............." << dictionary[ "QT_SOURCE_TREE" ] << endl;
    cout << "Install prefix.............." << dictionary[ "QT_INSTALL_PREFIX" ] << endl;
    cout << "Headers installed to........" << dictionary[ "QT_INSTALL_HEADERS" ] << endl;
    cout << "Libraries installed to......" << dictionary[ "QT_INSTALL_LIBS" ] << endl;
    cout << "Plugins installed to........" << dictionary[ "QT_INSTALL_PLUGINS" ] << endl;
    cout << "Binaries installed to......." << dictionary[ "QT_INSTALL_BINS" ] << endl;
    cout << "Docs installed to..........." << dictionary[ "QT_INSTALL_DOCS" ] << endl;
    cout << "Data installed to..........." << dictionary[ "QT_INSTALL_DATA" ] << endl;
    cout << "Translations installed to..." << dictionary[ "QT_INSTALL_TRANSLATIONS" ] << endl << endl;

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
#endif

#if !defined(EVAL)
void Configure::buildQmake()
{
    if( dictionary[ "BUILD_QMAKE" ] == "yes" ) {
	QStringList args;

	// Build qmake
	cout << "Creating qmake..." << endl;
	QString pwd = QDir::currentPath();
	QDir::setCurrent( dictionary[ "QT_SOURCE_TREE" ] + "/qmake" );
	args += dictionary[ "MAKE" ];
	args += "-f";
	args += dictionary[ "QMAKEMAKEFILE" ];
	if( int r = system( qPrintable(args.join( " " )) ) ) {
	    args.clear();
	    args += dictionary[ "MAKE" ];
	    args += "-f";
	    args += dictionary[ "QMAKEMAKEFILE" ];
	    args += "clean";
	    if( int r = system( qPrintable(args.join( " " )) ) ) {
		cout << "Cleaning qmake failed, return code " << r << endl << endl;
                dictionary[ "DONE" ] = "error";
	    } else {
		args.clear();
		args += dictionary[ "MAKE" ];
		args += "-f";
		args += dictionary[ "QMAKEMAKEFILE" ];
		if (int r = system(qPrintable(args.join( " " )))) {
		    cout << "Building qmake failed, return code " << r << endl << endl;
		    dictionary[ "DONE" ] = "error";
		}
	    }
	}
	QDir::setCurrent( pwd );
    }
}
#endif

void Configure::findProjects( const QString& dirName )
{
    if( dictionary[ "NOPROCESS" ] == "no" ) {
	QDir dir( dirName );
	QString entryName;
	int makeListNumber;
	ProjectType qmakeTemplate;

        static QString qtSourceDir;
        if (qtSourceDir.isEmpty()) {
            qtSourceDir = QDir(dictionary["QT_SOURCE_TREE"]).absoluteFilePath("src");
            qtSourceDir.replace("\\", "/");
        }
	const QFileInfoList &list = dir.entryInfoList();
	for(int i = 0; i < list.size(); ++i) {
	    const QFileInfo &fi = list.at(i);
	    if( fi.fileName()[ 0 ] != '.' && fi.fileName() != "qmake.pro" ) {
		entryName = dirName + "/" + fi.fileName();
		if(fi.isDir()) {
		    if (fi.absoluteFilePath() != qtSourceDir) {
			findProjects( entryName );
		    }
		} else {
		    if( fi.fileName().right( 4 ) == ".pro" ) {
			qmakeTemplate = projectType( fi.absoluteFilePath() );
			switch ( qmakeTemplate ) {
			case Lib:
			case Subdirs:
			    makeListNumber = 1;
			    break;
			default:
			    makeListNumber = 2;
			    break;
			}
			makeList[makeListNumber].append( new MakeItem(
								      dirName,
								      fi.fileName(),
								      "Makefile",
								      qmakeTemplate ) );
			if( dictionary[ "DSPFILES" ] == "yes" ) {
			    makeList[makeListNumber].append( new MakeItem(
									  dirName,
									  fi.fileName(),
									  fi.fileName().left( fi.fileName().length() - 4 ) + ".dsp",
									  qmakeTemplate ) );
			}
			if( dictionary[ "VCPFILES" ] == "yes" ) {
			    makeList[makeListNumber].append( new MakeItem(
									  dirName,
									  fi.fileName(),
									  fi.fileName().left( fi.fileName().length() - 4 ) + ".vcp",
									  qmakeTemplate ) );
			}
			if( dictionary[ "VCPROJFILES" ] == "yes" ) {
			    makeList[makeListNumber].append( new MakeItem(
									  dirName,
									  fi.fileName(),
									  fi.fileName().left( fi.fileName().length() - 4 ) + ".vcproj",
									  qmakeTemplate ) );
			}
		    }
		}
	    }

	}
    }
}

void Configure::appendMakeItem(int inList, const QString &item)
{
    QString dir;
    if (item != "src")
	dir = "/" + item;
    makeList[inList].append(new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	item + ".pro", "Makefile", Lib ) );
    if( dictionary[ "DSPFILES" ] == "yes" ) {
	makeList[inList].append( new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	    item + ".pro", item + ".dsp", Lib ) );
    }
    if( dictionary[ "VCPFILES" ] == "yes" ) {
	makeList[inList].append( new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	    item + ".pro", item + ".vcp", Lib ) );
    }
    if( dictionary[ "VCPROJFILES" ] == "yes" ) {
	makeList[inList].append( new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	    item + ".pro", item + ".vcproj", Lib ) );
    }
}

void Configure::generateMakefiles()
{
    if( dictionary[ "NOPROCESS" ] == "no" ) {
#if !defined(EVAL)
	cout << "Creating makefiles in src..." << endl;
#endif

	QString spec = dictionary[ "QMAKESPEC" ];
	if( spec != "win32-msvc" )
	    dictionary[ "DSPFILES" ] = "no";

	if( !spec.startsWith( "wince-" ) )
	    dictionary[ "VCPFILES" ] = "no";

	if( spec != "win32-msvc.net" && spec != "win32-msvc2005" )
	    dictionary[ "VCPROJFILES" ] = "no";

	int i = 0;
#if !defined(EVAL)
	QStringList qtProjects;
	qtProjects << "winmain" << "tools/moc" << "corelib" << "gui" << "network"
	    << "opengl" << "sql" << "xml" << "qt3support";

        for (i=0;i<qtProjects.size();++i)
            appendMakeItem(0, qtProjects.at(i));

        // Ensure the plugins and tools are done after the main libraries
	findProjects(dictionary["QT_SOURCE_TREE"] + "/src/plugins");
	findProjects(dictionary["QT_SOURCE_TREE"] + "/src/tools");
        appendMakeItem(2, "src"); // Now do src itself
#endif
	if (dictionary["LEAN"] == "no")
	    findProjects(dictionary["QT_SOURCE_TREE"]);

        QString qtDir = QDir::convertSeparators(dictionary["QT_SOURCE_TREE"] + "/");

	QString pwd = QDir::currentPath();
	for ( i=0; i<3; i++ ) {
	    for ( int j=0; j<makeList[i].size(); ++j) {
		MakeItem *it=makeList[i][j];
		QString dirPath = QDir::convertSeparators( it->directory + "/" );
		QString projectName = dirPath + it->proFile;
		QString makefileName = dirPath + it->target;
		QStringList args;

 		args << QDir::convertSeparators( dictionary[ "QT_INSTALL_BINS" ] + "/qmake" );
		args << projectName;
		args << dictionary[ "QMAKE_ALL_ARGS" ];

		if ( makefileName.endsWith( ".dsp" ) ||
		     makefileName.endsWith( ".vcp" ) ||
		     makefileName.endsWith( ".vcproj" ) ) {
		    // We don't create projects for sub directories, continue
		    if ( it->qmakeTemplate == Subdirs )
			continue;
		    if( dictionary[ "DEPENDENCIES" ] == "no" )
			args << "-nodepend";
		    args << "-tp vc";
		} else {
		    cout << "For " << qPrintable(projectName) << endl;
		    args << "-o";
		    args << makefileName;
		}
		args << "-spec";
		args << dictionary[ "QMAKESPEC" ];

		QDir::setCurrent( QDir::convertSeparators( dirPath ) );

                if (dictionary["FAST"] == "yes" && dirPath != qtDir) {
                    QFile file(makefileName);
                    if (!file.open(QFile::WriteOnly)) {
                        printf("failed on dirPath=%s, makefile=%s\n",
                               qPrintable(dirPath), qPrintable(makefileName));
                        continue;
                    }
                    QTextStream txt(&file);
                    txt << "all:\n";
                    txt << "\t" << args.join(" ") << "\n";
                    txt << "\t$(MAKE) -f " << makefileName << "\n";
                    txt << "first: all\n";
                } else {
                    if( int r = system( qPrintable(args.join( " " )) ) ) {
                        cout << "Qmake failed, return code " << r  << endl << endl;
                        dictionary[ "DONE" ] = "error";
                    }
		}
	    }
	}
	QDir::setCurrent( pwd );
    } else {
	cout << "Processing of project files have been disabled." << endl;
	cout << "Only use this option if you really know what you're doing." << endl << endl;
	dictionary[ "DONE" ] = "yes";
	return;
    }
}

void Configure::showSummary()
{
    QString make = dictionary[ "MAKE" ];
    cout << endl << endl << "Qt is now configured for building. Just run " << qPrintable(make) << "." << endl;
    cout << "To reconfigure, run " << qPrintable(make) << " clean and configure." << endl << endl;
}

Configure::ProjectType Configure::projectType( const QString& proFileName )
{
    QFile proFile( proFileName );
    if( proFile.open( QFile::ReadOnly ) ) {
        QString buffer = proFile.readLine(1024);
	while (!buffer.isEmpty()) {
	    QStringList segments = buffer.split(QRegExp( "\\s" ));
	    QStringList::Iterator it = segments.begin();

	    if(segments.size() >= 3) {
		QString keyword = (*it++);
		QString operation = (*it++);
		QString value = (*it++);

		if( keyword == "TEMPLATE" ) {
		    if( value == "lib" )
			return Lib;
		    else if( value == "subdirs" )
			return Subdirs;
		}
	    }
	    // read next line
	    buffer = proFile.readLine(1024);
	}
	proFile.close();
    }
    // Default to app handling
    return App;
}

#if defined (QT4_TECH_PREVIEW)
static uint convertor( const QString &list )
{
    static const unsigned char checksum[] = {
	0x61, 0x74, 0x18, 0x10, 0x06, 0x74, 0x76, 0x0b, 0x02, 0x7b,
	0x78, 0x18, 0x65, 0x72, 0x06, 0x76, 0x6d, 0x1f, 0x01, 0x75,
	0x7e, 0x79, 0x65, 0x01, 0x03, 0x06, 0x6c, 0x6e, 0x18, 0x14,
	0x8f, 0x75, 0x6a, 0x7a, 0x18, 0x7b, 0x76, 0x01, 0x1f, 0x7b,
	0x65, 0x72, 0x06, 0x06, 0x74, 0x76, 0x1f, 0x61, 0x03, 0x6a
    };

    uint length = 0;
    int temp = list.length();
    while ( temp > 0 ) {
        temp--;
	uint alpha = 0x58;
	int currentIndex = 0;
	for ( ;; ) {
	    if ( (uint)list[temp].toLatin1().constData() == alpha ) {
		length -= (length << 5) + currentIndex;
		break;
	    }
	    alpha ^= (uchar)checksum[currentIndex];
	    if ( (uchar)checksum[currentIndex] == 0x8f )
		return checksum[currentIndex] ^ 0x80;
	    ++currentIndex;
	}
	length = uint(-int(length));
	if ( (uint) (alpha - 0x8a) < 6 )
	    length += checksum[alpha - 0x8a];
    }
    return length;
}
#endif // QT4_TECH_PREVIEW

#if !defined(EVAL)
void Configure::readLicense()
{
    QString licensePath = firstLicensePath();
    QFile licenseFile( licensePath );
    if( !licensePath.isEmpty() && licenseFile.open( QFile::ReadOnly ) ) {
	cout << "Reading license file in....." << qPrintable(firstLicensePath()) << endl;

	QString buffer = licenseFile.readLine(1024);
        while (!buffer.isEmpty()) {
	    if( buffer[ 0 ] != '#' ) {
		QStringList components = buffer.split( '=' );
		if ( components.size() >= 2 ) {
		    QStringList::Iterator it = components.begin();
		    QString key = (*it++).trimmed().replace( "\"", QString::null ).toUpper();
		    QString value = (*it++).trimmed().replace( "\"", QString::null );
		    licenseInfo[ key ] = value;
		}
	    }
	    // read next line
	    buffer = licenseFile.readLine(1024);
	}
        licenseFile.close();
    }

    if( QFile::exists( dictionary[ "QT_SOURCE_TREE" ] + "/LICENSE.TROLL" ) ) {
	licenseInfo[ "PRODUCTS" ] = "qt-internal";
	dictionary[ "QMAKE_INTERNAL" ] = "yes";
    } else if (!licenseFile.exists()) {
	cout << "License file not found in " << QDir::homePath() << endl;
#if defined (QT4_TECH_PREVIEW)
        cout << "Please put the Qt license file, '.qt-license' in your home "
             << "directory and run configure again.";
        dictionary["DONE"] = "error";
        return;
#else
	cout << "Enterprise modules will not be available." << endl << endl;
	licenseInfo[ "PRODUCTS" ] = "qt-professional";
#endif
   }

    if( dictionary[ "FORCE_PROFESSIONAL" ] == "yes" )
        licenseInfo[ "PRODUCTS" ]= "qt-professional";

#if defined(QT4_TECH_PREVIEW)
    // Skip for internal build system...
    if (licenseInfo["PRODUCTS"] == "qt-internal") {
        return;
    }

    // Verify license info...
    QString licenseKey = licenseInfo["LICENSEKEY"];
    if (!licenseKey.isEmpty()) {

        // Check validity of the license key
        uint features = featuresForKey(licenseKey);

        if ((features & Feature_Windows) == 0) {
            cout << "Unable to find valid license for Qt/Windows" << endl
                 << "Configuration of Qt aborted" << endl << endl;
            dictionary["DONE"] = "error";
            return;
        }

        QString toLicenseFile = dictionary["QT_SOURCE_TREE"] + "/LICENSE";
        QString usLicenseFile = dictionary["QT_SOURCE_TREE"] + "/.LICENSE-US";
        QString norLicenseFile = dictionary["QT_SOURCE_TREE"] + "/.LICENSE";

        // Copy from .LICENSE(-US) to LICENSE
        if (!QFileInfo(toLicenseFile).exists()) {
            QString from = (features & Feature_US) ? usLicenseFile : norLicenseFile;
            if (!CopyFileA(QDir::convertSeparators(from).toLocal8Bit(),
                           QDir::convertSeparators(toLicenseFile).toLocal8Bit(), FALSE)) {
                cout << "Failed to copy license file";
                dictionary["DONE"] = "error";
                return;
            }
        }

        // Remove the old ones if present...
        if (QFileInfo(usLicenseFile).exists())
            DeleteFileA(usLicenseFile.toLocal8Bit());
        if (QFileInfo(norLicenseFile).exists())
            DeleteFileA(norLicenseFile.toLocal8Bit());

        QFile file(toLicenseFile);
        if (!file.open(QFile::ReadOnly)) {
            cout << "Failed to load LICENSE file";
            dictionary["DONE"] = "error";
            return;
        }

        // Get console line height, to fill the screen properly
        int i = 0, screenHeight = 25; // default
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (GetConsoleScreenBufferInfo(stdOut, &consoleInfo))
            screenHeight = consoleInfo.srWindow.Bottom
                         - consoleInfo.srWindow.Top
                         - 1; // Some overlap for context

        // Prompt the license content to the user
        QStringList licenseContent = QString(file.readAll()).split('\n');
        while(i < licenseContent.size()) {
            cout << licenseContent.at(i) << endl;
            if (++i % screenHeight == 0) {
                cout << "(Press any key for more..)";
                if(_getch() == 3) // _Any_ keypress w/no echo(eat <Enter> for stdout)
                    exit(0);      // Exit cleanly for Ctrl+C
                cout << "\r";     // Overwrite text above
            }
        }

        char accept = 'n';
        cout << endl << "Do you accept the license? (y/n)" << endl;
        cin >> accept;
        if (tolower(accept) != 'y') {
            cout << "Configuration aborted since license was not accepted";
            dictionary["DONE"] = "error";
            return;
        }

    } else {
        cout << "License file does not contain proper license key." << endl;
        dictionary["DONE"] = "error";
        return;
    }
#endif
}
#endif

#if !defined(EVAL)
void Configure::reloadCmdLine()
{
    if( dictionary[ "REDO" ] == "yes" ) {
	QFile inFile( dictionary[ "QT_SOURCE_TREE" ] + "/configure" + dictionary[ "CUSTOMCONFIG" ] + ".cache" );
	if( inFile.open( QFile::ReadOnly ) ) {
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
#endif

#if !defined(EVAL)
void Configure::saveCmdLine()
{
    if( dictionary[ "REDO" ] != "yes" ) {
	QFile outFile( dictionary[ "QT_SOURCE_TREE" ] + "/configure" + dictionary[ "CUSTOMCONFIG" ] + ".cache" );
	if( outFile.open( QFile::WriteOnly | QFile::Text ) ) {
	    QTextStream outStream( &outFile );
	    for( QStringList::Iterator it = configCmdLine.begin(); it != configCmdLine.end(); ++it ) {
		outStream << (*it) << " " << endl;
	    }
            outStream.flush();
	    outFile.close();
	}
    }
}
#endif

bool Configure::isDone()
{
    return !dictionary["DONE"].isEmpty();
}

bool Configure::isOk()
{
    return (dictionary[ "DONE" ] != "error");
}
