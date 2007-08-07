/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "configureapp.h"
#include "environment.h"
#ifdef COMMERCIAL_VERSION
#  include "tools.h"
#endif

#include <QDate>
#include <qdir.h>
#include <qtemporaryfile.h>
#include <qstack.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qhash.h>

#include <iostream>
#include <windows.h>
#include <conio.h>

std::ostream &operator<<( std::ostream &s, const QString &val ) {
    s << val.toLocal8Bit().data();
    return s;
}


using namespace std;

// Macros to simplify options marking
#define MARK_OPTION(x,y) ( dictionary[ #x ] == #y ? "*" : " " )


bool writeToFile(const char* text, const QString &filename)
{
    QByteArray symFile(text);
    QFile file(filename);
    QDir dir(QFileInfo(file).absoluteDir());
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());
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
    useUnixSeparators = false;
    // Default values for indentation
    optionIndent = 4;
    descIndent   = 25;
    outputWidth  = 0;
    // Get console buffer output width
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdout, &info))
        outputWidth = info.dwSize.X - 1;
    outputWidth = qMin(outputWidth, 79); // Anything wider gets unreadable
    if (outputWidth < 35) // Insanely small, just use 79
        outputWidth = 79;
    int i;

    /*
    ** Set up the initial state, the default
    */
    dictionary[ "CONFIGCMD" ] = argv[ 0 ];

    for ( i = 1; i < argc; i++ )
        configCmdLine += argv[ i ];


    // Get the path to the executable
    QFileInfo sourcePathInfo;
    QT_WA({
        unsigned short module_name[256];
        GetModuleFileNameW(0, reinterpret_cast<wchar_t *>(module_name), sizeof(module_name));
        sourcePathInfo = QString::fromUtf16(module_name);
    }, {
        char module_name[256];
        GetModuleFileNameA(0, module_name, sizeof(module_name));
        sourcePathInfo = QString::fromLocal8Bit(module_name);
    });
    sourcePath = sourcePathInfo.absolutePath();
    sourceDir = sourcePathInfo.dir();
    buildPath = QDir::currentPath();
#if 0
    const QString installPath = QString("C:\\Qt\\%1").arg(QT_VERSION_STR);
#else
    const QString installPath = buildPath;
#endif
    if(sourceDir != buildDir) { //shadow builds!
        cout << "Preparing build tree..." << endl;
        QDir(buildPath).mkpath("bin");

        { //duplicate qmake
            QStack<QString> qmake_dirs;
            qmake_dirs.push("qmake");
            while(!qmake_dirs.isEmpty()) {
                QString dir = qmake_dirs.pop();
                QString od(buildPath + "/" + dir);
                QString id(sourcePath + "/" + dir);
                QFileInfoList entries = QDir(id).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);
                for(int i = 0; i < entries.size(); ++i) {
                    QFileInfo fi(entries.at(i));
                    if(fi.isDir()) {
                        qmake_dirs.push(dir + "/" + fi.fileName());
                        QDir().mkpath(od + "/" + fi.fileName());
                    } else {
                        QDir().mkpath(od );
                        bool justCopy = true;
                        const QString fname = fi.fileName();
                        const QString outFile(od + "/" + fname), inFile(id + "/" + fname);
                        if(fi.fileName() == "Makefile") { //ignore
                        } else if(fi.suffix() == "h" || fi.suffix() == "cpp") {
                            QTemporaryFile tmpFile;
                            if(tmpFile.open()) {
                                QTextStream stream(&tmpFile);
                                stream << "#include \"" << inFile << "\"" << endl;
                                justCopy = false;
                                stream.flush();
                                tmpFile.flush();
                                if(filesDiffer(tmpFile.fileName(), outFile)) {
                                    QFile::remove(outFile);
                                    tmpFile.copy(outFile);
                                }
                            }
                        }
                        if(justCopy && filesDiffer(inFile, outFile))
                            QFile::copy(inFile, outFile);
                    }
                }
            }
        }

        { //make a syncqt script(s) that can be used in the shadow
            QFile syncqt(buildPath + "/bin/syncqt");
            if(syncqt.open(QFile::WriteOnly)) {
                QTextStream stream(&syncqt);
                stream << "#!/usr/bin/perl -w" << endl
                       << "require \"" << sourcePath + "/bin/syncqt\";" << endl;
            }
            QFile syncqt_bat(buildPath + "/bin/syncqt.bat");
            if(syncqt_bat.open(QFile::WriteOnly)) {
                QTextStream stream(&syncqt_bat);
                stream << "@echo off" << endl
                       << "set QTDIR=" << QDir::toNativeSeparators(sourcePath) << endl
                       << fixSeparators(sourcePath) << fixSeparators("/bin/syncqt.bat -outdir \"") << fixSeparators(buildPath) << "\"" << endl;
                syncqt_bat.close();
            }
        }

        //copy the mkspecs
        buildDir.mkpath("mkspecs");
        if(!Environment::cpdir(sourcePath + "/mkspecs", buildPath + "/mkspecs")){
            cout << "Couldn't copy mkspecs!" << sourcePath << " " << buildPath << endl;
            dictionary["DONE"] = "error";
            return;
        }
    }

    dictionary[ "QT_SOURCE_TREE" ]    = fixSeparators(sourcePath);
    dictionary[ "QT_BUILD_TREE" ]     = fixSeparators(buildPath);
    dictionary[ "QT_INSTALL_PREFIX" ] = fixSeparators(installPath);

    dictionary[ "QMAKESPEC" ] = getenv("QMAKESPEC");
    if (dictionary[ "QMAKESPEC" ].size() == 0) {
        dictionary[ "QMAKESPEC" ] = Environment::detectQMakeSpec();
        dictionary[ "QMAKESPEC_FROM" ] = "detected";
    } else {
        dictionary[ "QMAKESPEC_FROM" ] = "env";
    }

    dictionary[ "ARCHITECTURE" ]    = "windows";
    dictionary[ "QCONFIG" ]         = "full";
    dictionary[ "EMBEDDED" ]        = "no";
    dictionary[ "BUILD_QMAKE" ]     = "yes";
    dictionary[ "DSPFILES" ]        = "yes";
    dictionary[ "VCPROJFILES" ]     = "yes";
    dictionary[ "QMAKE_INTERNAL" ]  = "no";
    dictionary[ "FAST" ]            = "no";
    dictionary[ "NOPROCESS" ]       = "no";
    dictionary[ "STL" ]             = "yes";
    dictionary[ "EXCEPTIONS" ]      = "yes";
    dictionary[ "RTTI" ]            = "yes";
    dictionary[ "MMX" ]             = "auto";
    dictionary[ "3DNOW" ]           = "auto";
    dictionary[ "SSE" ]             = "auto";
    dictionary[ "SSE2" ]            = "auto";
    dictionary[ "SYNCQT" ]          = "auto";

    QString version;
    QFile qglobal_h(sourcePath + "/src/corelib/global/qglobal.h");
    if (qglobal_h.open(QFile::ReadOnly)) {
        QTextStream read(&qglobal_h);
        QRegExp version_regexp("^# *define *QT_VERSION_STR *\"([^\"]*)\"");
        QString line;
        while (!read.atEnd()) {
            line = read.readLine();
            if (version_regexp.exactMatch(line)) {
                version = version_regexp.cap(1).trimmed();
                if (!version.isEmpty())
                    break;
            }
        }
        qglobal_h.close();
    }

    if (version.isEmpty())
        version = QString("%1.%2.%3").arg(QT_VERSION>>16).arg(((QT_VERSION>>8)&0xff)).arg(QT_VERSION&0xff);

    dictionary[ "VERSION" ]         = version;
    {
        QRegExp version_re("([0-9]*)\\.([0-9]*)\\.([0-9]*)");
        if(version_re.exactMatch(version)) {
            dictionary[ "VERSION_MAJOR" ] = version_re.cap(1);
            dictionary[ "VERSION_MINOR" ] = version_re.cap(2);
            dictionary[ "VERSION_PATCH" ] = version_re.cap(3);
        }
    }

    dictionary[ "REDO" ]            = "no";
    dictionary[ "DEPENDENCIES" ]    = "no";

    dictionary[ "BUILD" ]           = "debug";
    dictionary[ "BUILDALL" ]        = "auto"; // Means yes, but not explicitly

    dictionary[ "SHARED" ]          = "yes";

    dictionary[ "ZLIB" ]            = "auto";

    dictionary[ "GIF" ]             = "auto";
    dictionary[ "TIFF" ]            = "auto";
    dictionary[ "JPEG" ]            = "auto";
    dictionary[ "PNG" ]             = "auto";
    dictionary[ "MNG" ]             = "auto";
    dictionary[ "LIBTIFF" ]         = "auto";
    dictionary[ "LIBJPEG" ]         = "auto";
    dictionary[ "LIBPNG" ]          = "auto";
    dictionary[ "LIBMNG" ]          = "auto";

    dictionary[ "QT3SUPPORT" ]      = "yes";
    dictionary[ "ACCESSIBILITY" ]   = "yes";
    dictionary[ "OPENGL" ]          = "yes";
    dictionary[ "DIRECT3D" ]        = "no";
    dictionary[ "IPV6" ]            = "yes"; // Always, dynamicly loaded
    dictionary[ "OPENSSL" ]         = "auto";
    dictionary[ "QDBUS" ]           = "auto";

    dictionary[ "STYLE_WINDOWS" ]   = "yes";
    dictionary[ "STYLE_WINDOWSXP" ] = "auto";
    dictionary[ "STYLE_WINDOWSVISTA" ] = "auto";
    dictionary[ "STYLE_PLASTIQUE" ] = "yes";
    dictionary[ "STYLE_CLEANLOOKS" ]= "yes";
    dictionary[ "STYLE_POCKETPC" ]  = "no";
    dictionary[ "STYLE_MOTIF" ]     = "yes";
    dictionary[ "STYLE_CDE" ]       = "yes";

    dictionary[ "SQL_MYSQL" ]       = "no";
    dictionary[ "SQL_ODBC" ]        = "no";
    dictionary[ "SQL_OCI" ]         = "no";
    dictionary[ "SQL_PSQL" ]        = "no";
    dictionary[ "SQL_TDS" ]         = "no";
    dictionary[ "SQL_DB2" ]         = "no";
    dictionary[ "SQL_SQLITE" ]      = "auto";
    dictionary[ "SQL_SQLITE_LIB" ]  = "qt";
    dictionary[ "SQL_SQLITE2" ]     = "no";
    dictionary[ "SQL_IBASE" ]       = "no";

    QString tmp = dictionary[ "QMAKESPEC" ];
    if (tmp.contains("\\")) {
        tmp = tmp.mid( tmp.lastIndexOf( "\\" ) + 1 );
    } else {
        tmp = tmp.mid( tmp.lastIndexOf("/") + 1 );
    }
    dictionary[ "QMAKESPEC" ] = tmp;

    dictionary[ "INCREDIBUILD_XGE" ] = "auto";
}

Configure::~Configure()
{
    for (int i=0; i<3; ++i) {
        QList<MakeItem*> items = makeList[i];
        for (int j=0; j<items.size(); ++j)
            delete items[j];
    }
}

QString Configure::fixSeparators(QString somePath)
{
    return useUnixSeparators ?
           QDir::fromNativeSeparators(somePath) :
           QDir::toNativeSeparators(somePath);
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
        if (i != argCount) {
            dictionary[ "REDO" ] = "yes";
            dictionary[ "CUSTOMCONFIG" ] = "_" + configCmdLine.at(i);
            configCmdLine.clear();
            reloadCmdLine();
        } else {
            dictionary[ "HELP" ] = "yes";
        }
    }
#endif

    for( ; i<configCmdLine.size(); ++i ) {
        bool continueElse = false;
        if( configCmdLine.at(i) == "-help"
            || configCmdLine.at(i) == "-h"
            || configCmdLine.at(i) == "-?" )
            dictionary[ "HELP" ] = "yes";

#if !defined(EVAL)
        else if( configCmdLine.at(i) == "-qconfig" ) {
            ++i;
            if (i==argCount)
                break;
            dictionary[ "QCONFIG" ] = configCmdLine.at(i);
        }

        else if ( configCmdLine.at(i) == "-buildkey" ) {
            ++i;
            if (i==argCount)
                break;
            dictionary[ "USER_BUILD_KEY" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-release" ) {
            dictionary[ "BUILD" ] = "release";
            if (dictionary[ "BUILDALL" ] == "auto")
                dictionary[ "BUILDALL" ] = "no";
        } else if( configCmdLine.at(i) == "-debug" ) {
            dictionary[ "BUILD" ] = "debug";
            if (dictionary[ "BUILDALL" ] == "auto")
                dictionary[ "BUILDALL" ] = "no";
        } else if( configCmdLine.at(i) == "-debug-and-release" )
            dictionary[ "BUILDALL" ] = "yes";

        else if( configCmdLine.at(i) == "-shared" )
            dictionary[ "SHARED" ] = "yes";
        else if( configCmdLine.at(i) == "-static" )
            dictionary[ "SHARED" ] = "no";

#endif

        else if( configCmdLine.at(i) == "-platform" ) {
            ++i;
            if (i==argCount)
                break;
            dictionary[ "QMAKESPEC" ] = configCmdLine.at(i);
            dictionary[ "QMAKESPEC_FROM" ] = "commandline";
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
            dictionary[ "LIBPNG" ] = "no";
            dictionary[ "MNG" ] = "no";
            dictionary[ "LIBMNG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-zlib" ) {
            dictionary[ "ZLIB" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-zlib" ) {
            dictionary[ "ZLIB" ] = "system";
        }

        // Image formats --------------------------------------------
        else if( configCmdLine.at(i) == "-no-gif" )
            dictionary[ "GIF" ] = "no";
        else if( configCmdLine.at(i) == "-qt-gif" )
            dictionary[ "GIF" ] = "auto";

        else if( configCmdLine.at(i) == "-no-libtiff" ) {
              dictionary[ "TIFF"] = "no";
              dictionary[ "LIBTIFF" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libtiff" ) {
            dictionary[ "TIFF" ] = "plugin";
            dictionary[ "LIBTIFF" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libtiff" ) {
              dictionary[ "TIFF" ] = "plugin";
              dictionary[ "LIBTIFF" ] = "system";
        }

        else if( configCmdLine.at(i) == "-no-libjpeg" ) {
            dictionary[ "JPEG" ] = "no";
            dictionary[ "LIBJPEG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libjpeg" ) {
            dictionary[ "JPEG" ] = "plugin";
            dictionary[ "LIBJPEG" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libjpeg" ) {
            dictionary[ "JPEG" ] = "plugin";
            dictionary[ "LIBJPEG" ] = "system";
        }

        else if( configCmdLine.at(i) == "-no-libpng" ) {
            dictionary[ "PNG" ] = "no";
            dictionary[ "LIBPNG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libpng" ) {
            dictionary[ "PNG" ] = "qt";
            dictionary[ "LIBPNG" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libpng" ) {
            dictionary[ "PNG" ] = "qt";
            dictionary[ "LIBPNG" ] = "system";
        }

        else if( configCmdLine.at(i) == "-no-libmng" ) {
            dictionary[ "MNG" ] = "no";
            dictionary[ "LIBMNG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libmng" ) {
            dictionary[ "MNG" ] = "qt";
            dictionary[ "LIBMNG" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libmng" ) {
            dictionary[ "MNG" ] = "qt";
            dictionary[ "LIBMNG" ] = "system";
        }

        // Styles ---------------------------------------------------
        else if( configCmdLine.at(i) == "-qt-style-pocketpc" )
            dictionary[ "STYLE_POCKETPC" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-pocketpc" )
            dictionary[ "STYLE_POCKETPC" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-windows" )
            dictionary[ "STYLE_WINDOWS" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-windows" )
            dictionary[ "STYLE_WINDOWS" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-windowsxp" )
            dictionary[ "STYLE_WINDOWSXP" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-windowsxp" )
            dictionary[ "STYLE_WINDOWSXP" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-windowsvista" )
            dictionary[ "STYLE_WINDOWSVISTA" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-windowsvista" )
            dictionary[ "STYLE_WINDOWSVISTA" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-plastique" )
            dictionary[ "STYLE_PLASTIQUE" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-plastique" )
            dictionary[ "STYLE_PLASTIQUE" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-cleanlooks" )
            dictionary[ "STYLE_CLEANLOOKS" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-cleanlooks" )
            dictionary[ "STYLE_CLEANLOOKS" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-motif" )
            dictionary[ "STYLE_MOTIF" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-motif" )
            dictionary[ "STYLE_MOTIF" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-cde" )
            dictionary[ "STYLE_CDE" ] = "yes";
        else if( configCmdLine.at(i) == "-no-style-cde" )
            dictionary[ "STYLE_CDE" ] = "no";

        // Qt 3 Support ---------------------------------------------
        else if( configCmdLine.at(i) == "-no-qt3support" )
            dictionary[ "QT3SUPPORT" ] = "no";

        // Work around compiler nesting limitation
        else
            continueElse = true;
        if (!continueElse) {
        }

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
        else if( configCmdLine.at(i) == "-system-sqlite" )
            dictionary[ "SQL_SQLITE_LIB" ] = "system";
        else if( configCmdLine.at(i) == "-qt-sql-sqlite2" )
            dictionary[ "SQL_SQLITE2" ] = "yes";
        else if( configCmdLine.at(i) == "-plugin-sql-sqlite2" )
            dictionary[ "SQL_SQLITE2" ] = "plugin";
        else if( configCmdLine.at(i) == "-no-sql-sqlite2" )
            dictionary[ "SQL_SQLITE2" ] = "no";

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

        else if( configCmdLine.at(i) == "-no-incredibuild-xge" )
            dictionary[ "INCREDIBUILD_XGE" ] = "no";
        else if( configCmdLine.at(i) == "-incredibuild-xge" )
            dictionary[ "INCREDIBUILD_XGE" ] = "yes";
#if !defined(EVAL)
        // Others ---------------------------------------------------
        else if (configCmdLine.at(i) == "-fast" )
            dictionary[ "FAST" ] = "yes";
        else if (configCmdLine.at(i) == "-no-fast" )
            dictionary[ "FAST" ] = "no";

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
        else if( configCmdLine.at(i) == "-no-accessibility" ) {
            dictionary[ "ACCESSIBILITY" ] = "no";
            cout << "Setting accessibility to NO" << endl;
        }

        else if (configCmdLine.at(i) == "-no-mmx")
            dictionary[ "MMX" ] = "no";
        else if (configCmdLine.at(i) == "-no-3dnow")
            dictionary[ "3DNOW" ] = "no";
        else if (configCmdLine.at(i) == "-no-sse")
            dictionary[ "SSE" ] = "no";
        else if (configCmdLine.at(i) == "-no-sse2")
            dictionary[ "SSE2" ] = "no";
        else if (configCmdLine.at(i) == "-direct3d") {
            QString sdk_dir(QString::fromLocal8Bit(getenv("DXSDK_DIR")));
            QDir dir;
            bool has_d3d = false;

            if (!sdk_dir.isEmpty() && dir.exists(sdk_dir))
                has_d3d = true;

            if (has_d3d && !QFile::exists(sdk_dir + QLatin1String("\\include\\d3d9.h"))) {
                cout << "No Direct3D version 9 SDK found." << endl;
                has_d3d = false;
            }

            // find the first dxguid.lib in the current LIB paths, if it is NOT
            // the D3D SDK one, we're most likely in trouble..
            if (has_d3d) {
                has_d3d = false;
                QString env_lib(QString::fromLocal8Bit(getenv("LIB")));
                QStringList lib_paths = env_lib.split(';');
                for (int i=0; i<lib_paths.size(); ++i) {
                    QString lib_path = lib_paths.at(i);
                    if (QFile::exists(lib_path + QLatin1String("\\dxguid.lib")))
                    {
                        if (lib_path.startsWith(sdk_dir)) {
                            has_d3d = true;
                        } else {
                            cout << "Your D3D/Platform SDK library paths seem to appear in the wrong order." << endl;
                        }
                        break;
                    }
                }
            }

            if (has_d3d) {
                dictionary[ "DIRECT3D" ] = "yes";
            } else {
                cout << "Setting Direct3D to NO, since the proper Direct3D SDK was not detected." << endl
                     << "Make sure you have the Direct3D SDK installed, and that you have run" << endl
                     << "the <path to SDK>\\Utilities\\Bin\\dx_setenv.cmd script." << endl
                     << "The D3D SDK library path *needs* to appear before the Platform SDK library" << endl
                     << "path in your LIB environment variable." << endl;
            }

        } else if( configCmdLine.at(i) == "-no-openssl" ) {
              dictionary[ "OPENSSL"] = "no";
        } else if( configCmdLine.at(i) == "-openssl" ) {
              dictionary[ "OPENSSL" ] = "yes";
        } else if( configCmdLine.at(i) == "-no-qdbus" ) {
            dictionary[ "QDBUS"] = "no";
        } else if( configCmdLine.at(i) == "-qdbus" ) {
            dictionary[ "QDBUS" ] = "yes";
        }

        else if( configCmdLine.at(i) == "-internal" )
            dictionary[ "QMAKE_INTERNAL" ] = "yes";

        else if( configCmdLine.at(i) == "-no-qmake" )
            dictionary[ "BUILD_QMAKE" ] = "no";
        else if( configCmdLine.at(i) == "-qmake" )
            dictionary[ "BUILD_QMAKE" ] = "yes";

        else if( configCmdLine.at(i) == "-dont-process" )
            dictionary[ "NOPROCESS" ] = "yes";
        else if( configCmdLine.at(i) == "-process" )
            dictionary[ "NOPROCESS" ] = "no";

        else if( configCmdLine.at(i) == "-no-qmake-deps" )
            dictionary[ "DEPENDENCIES" ] = "no";
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
            QFileInfo check(configCmdLine.at(i));
            if (!check.isDir()) {
                cout << "Argument passed to -L option is not a directory path. Did you mean the -l option?" << endl;
                dictionary[ "DONE" ] = "error";
                break;
            }
            qmakeLibs += QString("-L" + configCmdLine.at(i));
        } else if( configCmdLine.at(i) == "-l" ) {
            ++i;
            if (i==argCount)
                break;
            qmakeLibs += QString("-l" + configCmdLine.at(i));
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

        else if (configCmdLine.at(i) == "-confirm-license") {
            dictionary["LICENSE_CONFIRMED"] = "yes";
        }

        // Directories ----------------------------------------------
        else if( configCmdLine.at(i) == "-prefix" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_PREFIX" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-bindir" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_BINS" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-libdir" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_LIBS" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-docdir" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_DOCS" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-headerdir" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_HEADERS" ] = configCmdLine.at(i);
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

        else if( configCmdLine.at(i) == "-examplesdir" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_EXAMPLES" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-demosdir" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "QT_INSTALL_DEMOS" ] = configCmdLine.at(i);
        }

        else if( configCmdLine.at(i) == "-make" ) {
            ++i;
            if(i==argCount)
                break;
            dictionary[ "MAKE" ] = configCmdLine.at(i);
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

    // Ensure that QMAKESPEC exists in the mkspecs folder
    QDir mkspec_dir = fixSeparators(sourcePath + "/mkspecs");
    QStringList mkspecs = mkspec_dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    if (dictionary["QMAKESPEC"].toLower() == "features"
        || !mkspecs.contains(dictionary["QMAKESPEC"], Qt::CaseInsensitive)) {
        dictionary[ "HELP" ] = "yes";
        if (dictionary ["QMAKESPEC_FROM"] == "commandline") {
            cout << "Invalid option \"" << dictionary["QMAKESPEC"] << "\" for -platform." << endl;
        } else if (dictionary ["QMAKESPEC_FROM"] == "env") {
            cout << "QMAKESPEC environment variable is set to \"" << dictionary["QMAKESPEC"]
                 << "\" which is not a supported platform" << endl;
        } else { // was autodetected from environment
            cout << "Unable to detect the platform from environment. Use -platform command line"
                    "argument or set the QMAKESPEC environment variable and run configure again" << endl;
        }
        cout << "See the README file for a list of supported operating systems and compilers." << endl;
    } else {
        if( dictionary[ "QMAKESPEC" ].endsWith( "-msvc" ) ||
            dictionary[ "QMAKESPEC" ].endsWith( ".net" ) ||
            dictionary[ "QMAKESPEC" ].endsWith( "-icc" ) ||
            dictionary[ "QMAKESPEC" ].endsWith( "-msvc2005" )) {
            if ( dictionary[ "MAKE" ].isEmpty() ) dictionary[ "MAKE" ] = "nmake";
            dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32";
        } else if ( dictionary[ "QMAKESPEC" ] == QString( "win32-g++" ) ) {
            if ( dictionary[ "MAKE" ].isEmpty() ) dictionary[ "MAKE" ] = "mingw32-make";
            if (Environment::detectExecutable("sh.exe")) {
                dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32-g++-sh";
            } else {
                dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32-g++";
            }
        } else {
            if ( dictionary[ "MAKE" ].isEmpty() ) dictionary[ "MAKE" ] = "make";
            dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32";
        }
    }

    useUnixSeparators = (dictionary["QMAKESPEC"] == "win32-g++");


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


// Output helper functions --------------------------------[ Start ]-
/*!
    Determines the length of a string token.
*/
static int tokenLength(const char *str)
{
    if (*str == 0)
        return 0;

    const char *nextToken = strpbrk(str, " _/\n\r");
    if (nextToken == str || !nextToken)
        return 1;

    return int(nextToken - str);
}

/*!
    Prints out a string which starts at position \a startingAt, and
    indents each wrapped line with \a wrapIndent characters.
    The wrap point is set to the console width, unless that width
    cannot be determined, or is too small.
*/
void Configure::desc(const char *description, int startingAt, int wrapIndent)
{
    int linePos = startingAt;

    bool firstLine = true;
    const char *nextToken = description;
    while (*nextToken) {
        int nextTokenLen = tokenLength(nextToken);
        if (*nextToken == '\n'                         // Wrap on newline, duh
            || (linePos + nextTokenLen > outputWidth)) // Wrap at outputWidth
        {
            printf("\n");
            linePos = 0;
            firstLine = false;
            if (*nextToken == '\n')
                ++nextToken;
            continue;
        }
        if (!firstLine && linePos < wrapIndent) {  // Indent to wrapIndent
            printf("%*s", wrapIndent , "");
            linePos = wrapIndent;
            if (*nextToken == ' ') {
                ++nextToken;
                continue;
            }
        }
        printf("%.*s", nextTokenLen, nextToken);
        linePos += nextTokenLen;
        nextToken += nextTokenLen;
    }
}

/*!
    Prints out an option with its description wrapped at the
    description starting point. If \a skipIndent is true, the
    indentation to the option is not outputted (used by marked option
    version of desc()). Extra spaces between option and its
    description is filled with\a fillChar, if there's available
    space.
*/
void Configure::desc(const char *option, const char *description, bool skipIndent, char fillChar)
{
    if (!skipIndent)
        printf("%*s", optionIndent, "");

    int remaining  = descIndent - optionIndent - strlen(option);
    int wrapIndent = descIndent + qMax(0, 1 - remaining);
    printf("%s", option);

    if (remaining > 2) {
        printf(" "); // Space in front
        for (int i = remaining; i > 2; --i)
            printf("%c", fillChar); // Fill, if available space
    }
    printf(" "); // Space between option and description

    desc(description, wrapIndent, wrapIndent);
    printf("\n");
}

/*!
    Same as above, except it also marks an option with an '*', if
    the option is default action.
*/
void Configure::desc(const char *mark_option, const char *mark, const char *option, const char *description, char fillChar)
{
    const QString markedAs = dictionary.value(mark_option);
    if (markedAs == "auto" && markedAs == mark) // both "auto", always => +
        printf(" +  ");
    else if (markedAs == "auto")                // setting marked as "auto" and option is default => +
        printf(" %c  " , (defaultTo(mark_option) == QLatin1String(mark))? '+' : ' ');
    else if (QLatin1String(mark) == "auto")     // description marked as "auto" and option is available => +
        printf(" %c  " , checkAvailability(mark_option) ? '+' : ' ');
    else                                        // None are "auto", (markedAs == mark) => *
        printf(" %c  " , markedAs == QLatin1String(mark) ? '*' : ' ');

    desc(option, description, true, fillChar);
}

// Output helper functions ---------------------------------[ Stop ]-


bool Configure::displayHelp()
{
    if( dictionary[ "HELP" ] == "yes" ) {
        desc("Usage: configure [-buildkey <key>]\n"
//      desc("Usage: configure [-prefix dir] [-bindir <dir>] [-libdir <dir>]\n"
//                  "[-docdir <dir>] [-headerdir <dir>] [-plugindir <dir>]\n"
//                  "[-datadir <dir>] [-translationdir <dir>]\n"
//                  "[-examplesdir <dir>] [-demosdir <dir>][-buildkey <key>]\n"
                    "[-release] [-debug] [-debug-and-release] [-shared] [-static]\n"
                    "[-no-fast] [-fast] [-no-exceptions] [-exceptions]\n"
                    "[-no-accessibility] [-accessibility] [-no-rtti] [-rtti]\n"
                    "[-no-stl] [-stl] [-no-sql-<driver>] [-qt-sql-<driver>]\n"
                    "[-plugin-sql-<driver>] [-system-sqlite] [-arch <arch>]\n"
                    "[-D <define>] [-I <includepath>] [-L <librarypath>]\n"
                    "[-help] [-no-dsp] [-dsp] [-no-vcproj] [-vcproj]\n"
                    "[-no-qmake] [-qmake] [-dont-process] [-process]\n"
                    "[-no-style-<style>] [-qt-style-<style>] [-redo]\n"
                    "[-saveconfig <config>] [-loadconfig <config>] [-no-zlib]\n"
                    "[-qt-zlib] [-system-zlib] [-no-gif] [-qt-gif] [-no-libpng]\n"
                    "[-qt-libpng] [-system-libpng] [-no-libtiff] [-qt-libtiff]\n"
                    "[-system-libtiff] [-no-libjpeg] [-qt-libjpeg] [-system-libjpeg]\n"
                    "[-no-libmng] [-qt-libmng] [-system-libmng] [-no-qt3support]\n"
                    "[-no-mmx] [-no-3dnow] [-no-sse] [-no-sse2] [-direct3d]\n"
                    "[-openssl] [-no-openssl] [-qdbus] [-no-qdbus] [-platform <spec>]\n\n", 0, 7);

        desc("Installation options:\n\n");
#if !defined(EVAL)
/*
        desc(" These are optional, but you may specify install directories.\n\n", 0, 1);

        desc(                   "-prefix dir",          "This will install everything relative to dir\n(default $QT_INSTALL_PREFIX)\n");

        desc(" You may use these to separate different parts of the install:\n\n", 0, 1);

        desc(                   "-bindir <dir>",        "Executables will be installed to dir\n(default PREFIX/bin)");
        desc(                   "-libdir <dir>",        "Libraries will be installed to dir\n(default PREFIX/lib)");
        desc(                   "-docdir <dir>",        "Documentation will be installed to dir\n(default PREFIX/doc)");
        desc(                   "-headerdir <dir>",     "Headers will be installed to dir\n(default PREFIX/include)");
        desc(                   "-plugindir <dir>",     "Plugins will be installed to dir\n(default PREFIX/plugins)");
        desc(                   "-datadir <dir>",       "Data used by Qt programs will be installed to dir\n(default PREFIX)");
        desc(                   "-translationdir <dir>","Translations of Qt programs will be installed to dir\n(default PREFIX/translations)\n");
        desc(                   "-examplesdir <dir>",   "Examples will be installed to dir\n(default PREFIX/examples)");
        desc(                   "-demosdir <dir>",      "Demos will be installed to dir\n(default PREFIX/demos)");
*/
        desc(" You may use these options to turn on strict plugin loading:\n\n", 0, 1);

        desc(                   "-buildkey <key>",      "Build the Qt library and plugins using the specified <key>.  "
                                                        "When the library loads plugins, it will only load those that have a matching <key>.\n");

        desc("Configure options:\n\n");

        desc(" The defaults (*) are usually acceptable. A plus (+) denotes a default value"
             " that needs to be evaluated. If the evaluation succeeds, the feature is"
             " included. Here is a short explanation of each option:\n\n", 0, 1);

        desc("BUILD", "release","-release",             "Compile and link Qt with debugging turned off.");
        desc("BUILD", "debug",  "-debug",               "Compile and link Qt with debugging turned on.");
        desc("BUILDALL", "yes", "-debug-and-release",   "Compile and link two Qt libraries, with and without debugging turned on.\n");

        desc("SHARED", "yes",   "-shared",              "Create and use shared Qt libraries.");
        desc("SHARED", "no",    "-static",              "Create and use static Qt libraries.\n");

        desc("FAST", "no",      "-no-fast",             "Configure Qt normally by generating Makefiles for all project files.");
        desc("FAST", "yes",     "-fast",                "Configure Qt quickly by generating Makefiles only for library and "
                                                        "subdirectory targets.  All other Makefiles are created as wrappers "
                                                        "which will in turn run qmake\n");

        desc("EXCEPTIONS", "no", "-no-exceptions",      "Disable exceptions on platforms that support it.");
        desc("EXCEPTIONS", "yes","-exceptions",         "Enable exceptions on platforms that support it.\n");

        desc("ACCESSIBILITY", "no",  "-no-accessibility", "Do not compile Windows Active Accessibility support.");
        desc("ACCESSIBILITY", "yes", "-accessibility",    "Compile Windows Active Accessibility support.\n");

        desc("STL", "no",       "-no-stl",              "Do not compile STL support.");
        desc("STL", "yes",      "-stl",                 "Compile STL support.\n");

        desc(                   "-no-sql-<driver>",     "Disable SQL <driver> entirely, by default none are turned on.");
        desc(                   "-qt-sql-<driver>",     "Enable a SQL <driver> in the Qt Library.");
        desc(                   "-plugin-sql-<driver>", "Enable SQL <driver> as a plugin to be linked to at run time.\n"
                                                        "Available values for <driver>:");
        desc("SQL_MYSQL", "auto", "",                   "  mysql", ' ');
        desc("SQL_PSQL", "auto", "",                    "  psql", ' ');
        desc("SQL_OCI", "auto", "",                     "  oci", ' ');
        desc("SQL_ODBC", "auto", "",                    "  odbc", ' ');
        desc("SQL_TDS", "auto", "",                     "  tds", ' ');
        desc("SQL_DB2", "auto", "",                     "  db2", ' ');
        desc("SQL_SQLITE", "auto", "",                  "  sqlite", ' ');
        desc("SQL_SQLITE2", "auto", "",                 "  sqlite2", ' ');
        desc("SQL_IBASE", "auto", "",                   "  ibase", ' ');
        desc(                   "",                     "(drivers marked with a '+' have been detected as available on this system)\n", false, ' ');

        desc(                   "-system-sqlite",       "Use sqlite from the operating system.\n");

        desc("QT3SUPPORT", "no","-no-qt3support",       "Disables the Qt 3 support functionality.\n");

#endif
        desc(                   "-platform <spec>",     "The operating system and compiler you are building on.\n(default %QMAKESPEC%)\n");
        desc(                   "",                     "See the README file for a list of supported operating systems and compilers.\n", false, ' ');

#if !defined(EVAL)
        desc(                   "-D <define>",          "Add an explicit define to the preprocessor.");
        desc(                   "-I <includepath>",     "Add an explicit include path.");
        desc(                   "-L <librarypath>",     "Add an explicit library path.");
        desc(                   "-l <libraryname>",     "Add an explicit library name, residing in a librarypath.\n");
#endif

        desc(                   "-help, -h, -?",        "Display this information.\n");

#if !defined(EVAL)
        // 3rd party stuff options go below here --------------------------------------------------------------------------------
        desc("Third Party Libraries:\n\n");

        desc("ZLIB", "no",      "-no-zlib",             "Do not compile in ZLIB support. Implies -no-libpng.");
        desc("ZLIB", "qt",      "-qt-zlib",             "Use the zlib bundled with Qt.");
        desc("ZLIB", "system",  "-system-zlib",         "Use zlib from the operating system.\nSee http://www.gzip.org/zlib\n");

        desc("GIF", "no",       "-no-gif",              "Do not compile the plugin for GIF reading support.");
        desc("GIF", "auto",     "-qt-gif",              "Compile the plugin for GIF reading support.\nSee also src/plugins/imageformats/gif/qgifhandler.h\n");

        desc("LIBPNG", "no",    "-no-libpng",           "Do not compile in PNG support.");
        desc("LIBPNG", "qt",    "-qt-libpng",           "Use the libpng bundled with Qt.");
        desc("LIBPNG", "system","-system-libpng",       "Use libpng from the operating system.\nSee http://www.libpng.org/pub/png\n");

        desc("LIBMNG", "no",    "-no-libmng",           "Do not compile in MNG support.");
        desc("LIBMNG", "qt",    "-qt-libmng",           "Use the libmng bundled with Qt.");
        desc("LIBMNG", "system","-system-libmng",       "Use libmng from the operating system.\nSee See http://www.libmng.com\n");

        desc("LIBTIFF", "no",    "-no-libtiff",         "Do not compile the plugin for TIFF support.");
        desc("LIBTIFF", "qt",    "-qt-libtiff",         "Use the libtiff bundled with Qt.");
        desc("LIBTIFF", "system","-system-libtiff",     "Use libtiff from the operating system.\nSee http://www.libtiff.org\n");

        desc("LIBJPEG", "no",    "-no-libjpeg",         "Do not compile the plugin for JPEG support.");
        desc("LIBJPEG", "qt",    "-qt-libjpeg",         "Use the libjpeg bundled with Qt.");
        desc("LIBJPEG", "system","-system-libjpeg",     "Use libjpeg from the operating system.\nSee http://www.ijg.org\n");

#endif
        // Qt\Windows only options go below here --------------------------------------------------------------------------------
        desc("Qt/Windows only:\n\n");

        desc("DSPFILES", "no",  "-no-dsp",              "Do not generate VC++ .dsp files.");
        desc("DSPFILES", "yes", "-dsp",                 "Generate VC++ .dsp files, only if spec \"win32-msvc\".\n");

        desc("VCPROJFILES", "no", "-no-vcproj",         "Do not generate VC++ .vcproj files.");
        desc("VCPROJFILES", "yes", "-vcproj",           "Generate VC++ .vcproj files, only if platform \"win32-msvc.net\".\n");

        desc("INCREDIBUILD_XGE", "no", "-no-incredibuild-xge", "Do not add IncrediBuild XGE distribution commands to custom build steps.");
        desc("INCREDIBUILD_XGE", "yes", "-incredibuild-xge",   "Add IncrediBuild XGE distribution commands to custom build steps. This will distribute MOC and UIC steps, and other custom buildsteps which are added to the INCREDIBUILD_XGE variable.\n(The IncrediBuild distribution commands are only added to Visual Studio projects)\n");

#if !defined(EVAL)
        desc("BUILD_QMAKE", "no", "-no-qmake",          "Do not compile qmake.");
        desc("BUILD_QMAKE", "yes", "-qmake",            "Compile qmake.\n");

        desc("NOPROCESS", "yes", "-dont-process",       "Do not generate Makefiles/Project files. This will override -no-fast if specified.");
        desc("NOPROCESS", "no",  "-process",            "Generate Makefiles/Project files.\n");

        desc("RTTI", "no",      "-no-rtti",             "Do not compile runtime type information.");
        desc("RTTI", "yes",     "-rtti",                "Compile runtime type information.\n");
        desc("MMX", "no",       "-no-mmx",              "Do not compile with use of MMX instructions");
        desc("3DNOW", "no",     "-no-3dnow",            "Do not compile with use of 3DNOW instructions");
        desc("SSE", "no",       "-no-sse",              "Do not compile with use of SSE instructions");
        desc("SSE2", "no",      "-no-sse2",             "Do not compile with use of SSE2 instructions");
        desc("DIRECT3D", "yes",  "-direct3d",           "Compile in Direct3D support (experimental - see INSTALL for more info)");
        desc("OPENSSL", "no",    "-no-openssl",         "Do not compile in OpenSSL support");
        desc("OPENSSL", "yes",   "-openssl",            "Compile in OpenSSL support");
        desc("QDBUS", "no",      "-no-qdbus",           "Do not compile in qdbus support");
        desc("QDBUS", "yes",     "-qdbus",              "Compile in qdbus support");

        desc(                   "-arch <arch>",         "Specify an architecture.\n"
                                                        "Available values for <arch>:");
        desc("ARCHITECTURE","windows",       "",        "  windows", ' ');
        desc("ARCHITECTURE","boundschecker", "",        "  boundschecker\n", ' ');

        desc(                   "-no-style-<style>",    "Disable <style> entirely.");
        desc(                   "-qt-style-<style>",    "Enable <style> in the Qt Library.\nAvailable styles: ");

        desc("STYLE_WINDOWS", "yes", "",                "  windows", ' ');
        desc("STYLE_WINDOWSXP", "auto", "",             "  windowsxp", ' ');
        desc("STYLE_WINDOWSVISTA", "auto", "",          "  windowsvista", ' ');
        desc("STYLE_PLASTIQUE", "yes", "",              "  plastique", ' ');
        desc("STYLE_CLEANLOOKS", "yes", "",             "  cleanlooks", ' ');
        desc("STYLE_MOTIF", "yes", "",                  "  motif", ' ');
        desc("STYLE_CDE", "yes", "",                    "  cde\n", ' ');

/*      We do not support -qconfig on Windows yet

        desc(                   "-qconfig <local>",     "Use src/tools/qconfig-local.h rather than the default.\nPossible values for local:");
        for (int i=0; i<allConfigs.size(); ++i)
            desc(               "",                     qPrintable(QString("  %1").arg(allConfigs.at(i))), false, ' ');
        printf("\n");
*/
#endif
        desc(                   "-loadconfig <config>", "Run configure with the parameters from file configure_<config>.cache.");
        desc(                   "-saveconfig <config>", "Run configure and save the parameters in file configure_<config>.cache.");
        desc(                   "-redo",                "Run configure with the same parameters as last time.\n");
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

/*!
    Default value for options marked as "auto" if the test passes.
    (Used both by the autoDetection() below, and the desc() function
    to mark (+) the default option of autodetecting options.
*/
QString Configure::defaultTo(const QString &option)
{
    // We prefer using the system version of the 3rd party libs
    if (option == "ZLIB"
        || option == "LIBJPEG"
        || option == "LIBPNG"
        || option == "LIBMNG"
        || option == "LIBTIFF")
        return "system";

    // We want PNG built-in
    if (option == "PNG")
        return "qt";

    // The JPEG image library can only be a plugin
    if (option == "JPEG"
        || option == "MNG" || option == "TIFF")
        return "plugin";

    // GIF off by default
    if (option == "GIF") {
        if (dictionary["SHARED"] == "yes")
            return "plugin";
        else
            return "yes";
    }

    // By default we do not want to compile OCI driver when compiling with
    // MinGW, due to lack of such support from Oracle. It prob. wont work.
    // (Customer may force the use though)
    if (dictionary["QMAKESPEC"].endsWith("-g++")
        && option == "SQL_OCI")
        return "no";

    if (option == "SQL_MYSQL"
        || option == "SQL_MYSQL"
        || option == "SQL_ODBC"
        || option == "SQL_OCI"
        || option == "SQL_PSQL"
        || option == "SQL_TDS"
        || option == "SQL_DB2"
        || option == "SQL_SQLITE"
        || option == "SQL_SQLITE2"
        || option == "SQL_IBASE")
        return "plugin";

    if (option == "SYNCQT"
        && (!QFile::exists(sourcePath + "/bin/syncqt") ||
            !QFile::exists(sourcePath + "/bin/syncqt.bat")))
        return "no";

    return "yes";
}

/*!
    Checks the system for the availability of a feature.
    Returns true if the feature is available, else false.
*/
bool Configure::checkAvailability(const QString &part)
{
    bool available = false;
    if (part == "STYLE_WINDOWSXP")
        available = (dictionary.value("QMAKESPEC") == "win32-g++" || findFile("uxtheme.h"));

    else if (part == "ZLIB")
        available = findFile("zlib.h");

    else if (part == "LIBJPEG")
        available = findFile("jpeglib.h");
    else if (part == "LIBPNG")
        available = findFile("png.h");
    else if (part == "LIBMNG")
        available = findFile("libmng.h");
    else if (part == "LIBTIFF")
        available = findFile("tiffio.h");
    else if (part == "SQL_MYSQL")
        available = findFile("mysql.h") && findFile("libmySQL.lib");
    else if (part == "SQL_ODBC")
        available = findFile("sql.h") && findFile("sqlext.h") && findFile("odbc32.lib");
    else if (part == "SQL_OCI")
        available = findFile("oci.h") && findFile("oci.lib");
    else if (part == "SQL_PSQL")
        available = findFile("libpq-fe.h") && findFile("libpq.lib") && findFile("ws2_32.lib") && findFile("advapi32.lib");
    else if (part == "SQL_TDS")
        available = findFile("sybfront.h") && findFile("sybdb.h") && findFile("ntwdblib.lib");
    else if (part == "SQL_DB2")
        available = findFile("sqlcli.h") && findFile("sqlcli1.h") && findFile("db2cli.lib");
    else if (part == "SQL_SQLITE")
            available = true; // Built in, we have a fork
    else if (part == "SQL_SQLITE_LIB") {
        if (dictionary[ "SQL_SQLITE_LIB" ] == "system") {
            available = findFile("sqlite3.h") && findFile("sqlite3.lib");
            if (available)
                dictionary[ "QT_LFLAGS_SQLITE" ] += "sqlite3.lib";
        } else
            available = true;
    } else if (part == "SQL_SQLITE2")
        available = findFile("sqlite.h") && findFile("sqlite.lib");
    else if (part == "SQL_IBASE")
        available = findFile("ibase.h") && (findFile("gds32_ms.lib") || findFile("gds32.lib"));
    else if (part == "SSE2")
        available = (dictionary.value("QMAKESPEC") != "win32-msvc") && (dictionary.value("QMAKESPEC") != "win32-g++");
    else if (part == "3DNOW" )
        available = (dictionary.value("QMAKESPEC") != "win32-msvc") && (dictionary.value("QMAKESPEC") != "win32-icc") && findFile("mm3dnow.h") && (dictionary.value("QMAKESPEC") != "win32-g++");
    else if (part == "MMX" || part == "SSE")
        available = (dictionary.value("QMAKESPEC") != "win32-msvc") && (dictionary.value("QMAKESPEC") != "win32-g++");
    else if (part == "OPENSSL")
        available = findFile("openssl\\ssl.h");
    else if (part == "QDBUS")
        available = findFile("dbus\\dbus.h");

    else if (part == "INCREDIBUILD_XGE")
        available = findFile("BuildConsole.exe") && findFile("xgConsole.exe");

    return available;
}

/*
    Autodetect options marked as "auto".
*/
void Configure::autoDetection()
{
    // Style detection
    if (dictionary["STYLE_WINDOWSXP"] == "auto")
        dictionary["STYLE_WINDOWSXP"] = checkAvailability("STYLE_WINDOWSXP") ? defaultTo("STYLE_WINDOWSXP") : "no";
    if (dictionary["STYLE_WINDOWSVISTA"] == "auto") // Vista style has the same requirements as XP style
        dictionary["STYLE_WINDOWSVISTA"] = checkAvailability("STYLE_WINDOWSXP") ? defaultTo("STYLE_WINDOWSVISTA") : "no";

    // Compression detection
    if (dictionary["ZLIB"] == "auto")
        dictionary["ZLIB"] =  checkAvailability("ZLIB") ? defaultTo("ZLIB") : "qt";

    // Image format detection
    if (dictionary["GIF"] == "auto")
        dictionary["GIF"] = defaultTo("GIF");
    if (dictionary["JPEG"] == "auto")
        dictionary["JPEG"] = defaultTo("JPEG");
    if (dictionary["PNG"] == "auto")
        dictionary["PNG"] = defaultTo("PNG");
    if (dictionary["MNG"] == "auto")
        dictionary["MNG"] = defaultTo("MNG");
    if (dictionary["TIFF"] == "auto")
        dictionary["TIFF"] = dictionary["ZLIB"] == "no" ? "no" : defaultTo("TIFF");
    if (dictionary["LIBJPEG"] == "auto")
        dictionary["LIBJPEG"] = checkAvailability("LIBJPEG") ? defaultTo("LIBJPEG") : "qt";
    if (dictionary["LIBPNG"] == "auto")
        dictionary["LIBPNG"] = checkAvailability("LIBPNG") ? defaultTo("LIBPNG") : "qt";
    if (dictionary["LIBMNG"] == "auto")
        dictionary["LIBMNG"] = checkAvailability("LIBMNG") ? defaultTo("LIBMNG") : "qt";
    if (dictionary["LIBTIFF"] == "auto")
        dictionary["LIBTIFF"] = checkAvailability("LIBTIFF") ? defaultTo("LIBTIFF") : "qt";

    // SQL detection (not on by default)
    if (dictionary["SQL_MYSQL"] == "auto")
        dictionary["SQL_MYSQL"] = checkAvailability("SQL_MYSQL") ? defaultTo("SQL_MYSQL") : "no";
    if (dictionary["SQL_ODBC"] == "auto")
        dictionary["SQL_ODBC"] = checkAvailability("SQL_ODBC") ? defaultTo("SQL_ODBC") : "no";
    if (dictionary["SQL_OCI"] == "auto")
        dictionary["SQL_OCI"] = checkAvailability("SQL_OCI") ? defaultTo("SQL_OCI") : "no";
    if (dictionary["SQL_PSQL"] == "auto")
        dictionary["SQL_PSQL"] = checkAvailability("SQL_PSQL") ? defaultTo("SQL_PSQL") : "no";
    if (dictionary["SQL_TDS"] == "auto")
        dictionary["SQL_TDS"] = checkAvailability("SQL_TDS") ? defaultTo("SQL_TDS") : "no";
    if (dictionary["SQL_DB2"] == "auto")
        dictionary["SQL_DB2"] = checkAvailability("SQL_DB2") ? defaultTo("SQL_DB2") : "no";
    if (dictionary["SQL_SQLITE"] == "auto")
        dictionary["SQL_SQLITE"] = checkAvailability("SQL_SQLITE") ? defaultTo("SQL_SQLITE") : "no";
    if (dictionary["SQL_SQLITE_LIB"] == "system")
        if (!checkAvailability("SQL_SQLITE_LIB"))
            dictionary["SQL_SQLITE_LIB"] = "no";
    if (dictionary["SQL_SQLITE2"] == "auto")
        dictionary["SQL_SQLITE2"] = checkAvailability("SQL_SQLITE2") ? defaultTo("SQL_SQLITE2") : "no";
    if (dictionary["SQL_IBASE"] == "auto")
        dictionary["SQL_IBASE"] = checkAvailability("SQL_IBASE") ? defaultTo("SQL_IBASE") : "no";
    if (dictionary["MMX"] == "auto")
        dictionary["MMX"] = checkAvailability("MMX") ? "yes" : "no";
    if (dictionary["3DNOW"] == "auto")
        dictionary["3DNOW"] = checkAvailability("3DNOW") ? "yes" : "no";
    if (dictionary["SSE"] == "auto")
        dictionary["SSE"] = checkAvailability("SSE") ? "yes" : "no";
    if (dictionary["SSE2"] == "auto")
        dictionary["SSE2"] = checkAvailability("SSE2") ? "yes" : "no";
    if (dictionary["OPENSSL"] == "auto")
        dictionary["OPENSSL"] = checkAvailability("OPENSSL") ? "yes" : "no";
    if (dictionary["QDBUS"] == "auto")
        dictionary["QDBUS"] = checkAvailability("QDBUS") ? "yes" : "no";

    // Detection of IncrediBuild buildconsole
    if (dictionary["INCREDIBUILD_XGE"] == "auto")
        dictionary["INCREDIBUILD_XGE"] = checkAvailability("INCREDIBUILD_XGE") ? "yes" : "no";

    // Mark all unknown "auto" to the default value..
    for (QMap<QString,QString>::iterator i = dictionary.begin(); i != dictionary.end(); ++i) {
        if (i.value() == "auto")
            i.value() = defaultTo(i.key());
    }
}

bool Configure::verifyConfiguration()
{
    if (dictionary["SQL_SQLITE_LIB"] == "no" && dictionary["SQL_SQLITE"] != "no") {
        cout << "WARNING: Configure could not detect the presence of a system SQLite3 lib." << endl
             << "Configure will therefore continue with the SQLite3 lib bundled with Qt." << endl
             << "(Press any key to continue..)";
        if(_getch() == 3) // _Any_ keypress w/no echo(eat <Enter> for stdout)
            exit(0);      // Exit cleanly for Ctrl+C

        dictionary["SQL_SQLITE_LIB"] = "qt"; // Set to Qt's bundled lib an continue
    }
    if (dictionary["QMAKESPEC"].endsWith("-g++")
        && dictionary["SQL_OCI"] != "no") {
        cout << "WARNING: Qt does not support compiling the Oracle database driver with" << endl
             << "MinGW, due to lack of such support from Oracle. Consider disabling the" << endl
             << "Oracle driver, as the current build will most likely fail." << endl;
        cout << "(Press any key to continue..)";
        if(_getch() == 3) // _Any_ keypress w/no echo(eat <Enter> for stdout)
            exit(0);      // Exit cleanly for Ctrl+C
    }

    return true;
}

/*
 Things that affect the Qt API/ABI:
   Options:
     minimal-config small-config medium-config large-config full-config

   Options:
     debug release
     stl

 Things that do not affect the Qt API/ABI:
     system-jpeg no-jpeg jpeg
     system-mng no-mng mng
     system-png no-png png
     system-zlib no-zlib zlib
     system-tiff no-tiff tiff
     no-gif gif
     dll staticlib

     internal
     nocrosscompiler
     GNUmake
     largefile
     nis
     nas
     tablet
     ipv6

     X11     : x11sm xinerama xcursor xfixes xrandr xrender fontconfig xkb
     Embedded: embedded freetype
*/
void Configure::generateBuildKey()
{
    QString spec = dictionary["QMAKESPEC"];

    QString compiler = "msvc"; // ICC is compatible
    if (spec.endsWith("-g++"))
        compiler = "mingw";
    else if (spec.endsWith("-borland"))
        compiler = "borland";

    // Build options which changes the Qt API/ABI
    QStringList build_options;
    if (!dictionary["QCONFIG"].isEmpty())
        build_options += dictionary["QCONFIG"] + "-config ";
    if (dictionary["STL"] == "no")
        build_options += "no-stl";
    build_options.sort();

    // Sorted defines that start with QT_NO_
    QStringList build_defines = qmakeDefines.filter(QRegExp("^QT_NO_"));
    build_defines.sort();

    // Build up the QT_BUILD_KEY ifdef
    QString buildKey = "QT_BUILD_KEY \"";
    if (!dictionary["USER_BUILD_KEY"].isEmpty())
        buildKey += dictionary["USER_BUILD_KEY"] + " ";

    QString build32Key = buildKey + "Windows " + compiler + " %1 " + build_options.join(" ") + " " + build_defines.join(" ");
    QString build64Key = buildKey + "Windows x64 " + compiler + " %1 " + build_options.join(" ") + " " + build_defines.join(" ");
    build32Key = build32Key.simplified();
    build64Key = build64Key.simplified();
    build32Key.prepend("#  define ");
    build64Key.prepend("#  define ");

    QString buildkey = // Debug builds
                       "#if (defined(_DEBUG) || defined(DEBUG))\n"
                       "# if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))\n"
                       + build64Key.arg("debug") + "\"\n"
                       "# else\n"
                       + build32Key.arg("debug") + "\"\n"
                       "# endif\n"
                       "#else\n"
                       // Release builds
                       "# if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))\n"
                       + build64Key.arg("release") + "\"\n"
                       "# else\n"
                       + build32Key.arg("release") + "\"\n"
                       "# endif\n"
                       "#endif\n";

    dictionary["BUILD_KEY"] = buildkey;
}

void Configure::generateOutputVars()
{
    // Generate variables for output
    // Build key ----------------------------------------------------
    if ( dictionary.contains("BUILD_KEY") ) {
        qmakeVars += dictionary.value("BUILD_KEY");
    }

    QString build = dictionary[ "BUILD" ];
    bool buildAll = (dictionary[ "BUILDALL" ] == "yes");
    if ( build == "debug") {
        if (buildAll)
            qtConfig += "release";
        qtConfig += "debug";
    } else if (build == "release") {
        if (buildAll)
            qtConfig += "debug";
        qtConfig += "release";
    }

    // Compression --------------------------------------------------
    if ( dictionary[ "ZLIB" ] == "no" )
        qtConfig += "no-zlib";
    if( dictionary[ "ZLIB" ] == "qt" )
        qtConfig += "zlib";
    else if( dictionary[ "ZLIB" ] == "system" )
        qtConfig += "system-zlib";

    // Image formates -----------------------------------------------
    if( dictionary[ "GIF" ] == "no" )
        qtConfig += "no-gif";
    else if( dictionary[ "GIF" ] == "yes" )
        qtConfig += "gif";
    else if( dictionary[ "GIF" ] == "plugin" )
        qmakeFormatPlugins += "gif";

    if( dictionary[ "TIFF" ] == "no" )
          qtConfig += "no-tiff";
    else if( dictionary[ "TIFF" ] == "plugin" )
        qmakeFormatPlugins += "tiff";
    if( dictionary[ "LIBTIFF" ] == "system" )
        qtConfig += "system-tiff";

    if( dictionary[ "JPEG" ] == "no" )
        qtConfig += "no-jpeg";
    else if( dictionary[ "JPEG" ] == "plugin" )
        qmakeFormatPlugins += "jpeg";
    if( dictionary[ "LIBJPEG" ] == "system" )
        qtConfig += "system-jpeg";

    if( dictionary[ "PNG" ] == "no" )
        qtConfig += "no-png";
    else if( dictionary[ "PNG" ] == "qt" )
        qtConfig += "png";
    if( dictionary[ "LIBPNG" ] == "system" )
        qtConfig += "system-png";

    if( dictionary[ "MNG" ] == "no" )
        qtConfig += "no-mng";
    else if( dictionary[ "MNG" ] == "qt" )
        qtConfig += "mng";
    if( dictionary[ "LIBMNG" ] == "system" )
        qtConfig += "system-mng";

    // Styles -------------------------------------------------------
    if ( dictionary[ "STYLE_WINDOWS" ] == "yes" )
        qmakeStyles += "windows";

    if ( dictionary[ "STYLE_PLASTIQUE" ] == "yes" )
        qmakeStyles += "plastique";

    if ( dictionary[ "STYLE_CLEANLOOKS" ] == "yes" )
        qmakeStyles += "cleanlooks";

    if ( dictionary[ "STYLE_WINDOWSXP" ] == "yes" )
        qmakeStyles += "windowsxp";

    if ( dictionary[ "STYLE_WINDOWSVISTA" ] == "yes" )
        qmakeStyles += "windowsvista";

    if ( dictionary[ "STYLE_MOTIF" ] == "yes" )
        qmakeStyles += "motif";

    if ( dictionary[ "STYLE_SGI" ] == "yes" )
        qmakeStyles += "sgi";

    if ( dictionary[ "STYLE_POCKETPC" ] == "yes" )
        qmakeStyles += "pocketpc";

    if ( dictionary[ "STYLE_CDE" ] == "yes" )
        qmakeStyles += "cde";

    // Databases ----------------------------------------------------
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

    if ( dictionary[ "SQL_SQLITE_LIB" ] == "system" )
        qmakeConfig += "system-sqlite";

    if ( dictionary[ "SQL_SQLITE2" ] == "yes" )
        qmakeSql += "sqlite2";
    else if ( dictionary[ "SQL_SQLITE2" ] == "plugin" )
        qmakeSqlPlugins += "sqlite2";

    if ( dictionary[ "SQL_IBASE" ] == "yes" )
        qmakeSql += "ibase";
    else if ( dictionary[ "SQL_IBASE" ] == "plugin" )
        qmakeSqlPlugins += "ibase";

    // Other options ------------------------------------------------
    if( dictionary[ "BUILDALL" ] == "yes" ) {
        qmakeConfig += "build_all";
    }
    qmakeConfig += dictionary[ "BUILD" ];
    dictionary[ "QMAKE_OUTDIR" ] = dictionary[ "BUILD" ];

    if ( dictionary[ "SHARED" ] == "yes" ) {
        QString version = dictionary[ "VERSION" ];
        if (!version.isEmpty()) {
            qmakeVars += "QMAKE_QT_VERSION_OVERRIDE = " + version.left(version.indexOf("."));
            version.remove(QLatin1Char('.'));
        }
        dictionary[ "QMAKE_OUTDIR" ] += "_shared";
    } else {
        dictionary[ "QMAKE_OUTDIR" ] += "_static";
    }

    if( dictionary[ "ACCESSIBILITY" ] == "yes" )
        qtConfig += "accessibility";

    if( !qmakeLibs.isEmpty() )
        qmakeVars += "LIBS           += " + qmakeLibs.join( " " );

    if( !dictionary["QT_LFLAGS_SQLITE"].isEmpty() )
        qmakeVars += "QT_LFLAGS_SQLITE += " + dictionary["QT_LFLAGS_SQLITE"];

    if (dictionary[ "QT3SUPPORT" ] == "yes")
        qtConfig += "qt3support";

    if (dictionary[ "OPENGL" ] == "yes")
        qtConfig += "opengl";

    if (dictionary[ "DIRECT3D" ] == "yes")
        qtConfig += "direct3d";

    if (dictionary[ "OPENSSL" ] == "yes")
        qtConfig += "openssl";

    if (dictionary[ "QDBUS" ] == "yes")
        qtConfig += "qdbus";

    if (dictionary["IPV6"] == "yes")
        qtConfig += "ipv6";
    else if (dictionary["IPV6"] == "no")
        qtConfig += "no-ipv6";

    // Add config levels --------------------------------------------
    QStringList possible_configs = QStringList()
        << "minimal"
        << "small"
        << "medium"
        << "large"
        << "full";

    QString set_config = dictionary["QCONFIG"];
    if (possible_configs.contains(set_config)) {
        foreach(QString cfg, possible_configs) {
            qtConfig += (cfg + "-config");
            if (cfg == set_config)
                break;
        }
    }

    // Directories and settings for .qmake.cache --------------------

    // if QT_INSTALL_* have not been specified on commandline, define them now from QT_INSTALL_PREFIX
    if( !dictionary[ "QT_INSTALL_DOCS" ].size() )
        dictionary[ "QT_INSTALL_DOCS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/doc" );
    if( !dictionary[ "QT_INSTALL_HEADERS" ].size() )
        dictionary[ "QT_INSTALL_HEADERS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/include" );
    if( !dictionary[ "QT_INSTALL_LIBS" ].size() )
        dictionary[ "QT_INSTALL_LIBS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/lib" );
    if( !dictionary[ "QT_INSTALL_BINS" ].size() )
        dictionary[ "QT_INSTALL_BINS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/bin" );
    if( !dictionary[ "QT_INSTALL_PLUGINS" ].size() )
        dictionary[ "QT_INSTALL_PLUGINS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/plugins" );
    if( !dictionary[ "QT_INSTALL_DATA" ].size() )
        dictionary[ "QT_INSTALL_DATA" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] );
    if( !dictionary[ "QT_INSTALL_TRANSLATIONS" ].size() )
        dictionary[ "QT_INSTALL_TRANSLATIONS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/translations" );
    if( !dictionary[ "QT_INSTALL_EXAMPLES" ].size() )
        dictionary[ "QT_INSTALL_EXAMPLES" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/examples");
    if( !dictionary[ "QT_INSTALL_DEMOS" ].size() )
        dictionary[ "QT_INSTALL_DEMOS" ] = fixSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/demos" );

    qmakeVars += QString("OBJECTS_DIR     = ") + fixSeparators( "tmp/obj/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString("MOC_DIR         = ") + fixSeparators( "tmp/moc/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString("RCC_DIR         = ") + fixSeparators("tmp/rcc/" + dictionary["QMAKE_OUTDIR"]);

    if (!qmakeDefines.isEmpty())
        qmakeVars += QString("DEFINES        += ") + qmakeDefines.join( " " );
    if (!qmakeIncludes.isEmpty())
        qmakeVars += QString("INCLUDEPATH    += ") + qmakeIncludes.join( " " );
    if (!qmakeSql.isEmpty())
        qmakeVars += QString("sql-drivers    += ") + qmakeSql.join( " " );
    if (!qmakeSqlPlugins.isEmpty())
        qmakeVars += QString("sql-plugins    += ") + qmakeSqlPlugins.join( " " );
    if (!qmakeStyles.isEmpty())
        qmakeVars += QString("styles         += ") + qmakeStyles.join( " " );
    if (!qmakeStylePlugins.isEmpty())
        qmakeVars += QString("style-plugins  += ") + qmakeStylePlugins.join( " " );
    if (!qmakeFormatPlugins.isEmpty())
        qmakeVars += QString("imageformat-plugins += ") + qmakeFormatPlugins.join( " " );

    if (dictionary["QMAKESPEC"].endsWith("-g++")) {
        QString includepath = qgetenv("INCLUDE");
        bool hasSh = Environment::detectExecutable("sh.exe");
        QChar separator = (!includepath.contains(":\\") && hasSh ? QChar(':') : QChar(';'));
        qmakeVars += QString("TMPPATH            = $$quote($$(INCLUDE))");
        qmakeVars += QString("QMAKE_INCDIR_POST += $$split(TMPPATH,\"%1\")").arg(separator);
        qmakeVars += QString("TMPPATH            = $$quote($$(LIB))");
        qmakeVars += QString("QMAKE_LIBDIR_POST += $$split(TMPPATH,\"%1\")").arg(separator);
    }

    if( !dictionary[ "QMAKESPEC" ].length() ) {
        cout << "Configure could not detect your compiler. QMAKESPEC must either" << endl
             << "be defined as an environment variable, or specified as an" << endl
             << "argument with -platform" << endl;
        dictionary[ "HELP" ] = "yes";

        QStringList winPlatforms;
        QDir mkspecsDir( sourcePath + "/mkspecs" );
        const QFileInfoList &specsList = mkspecsDir.entryInfoList();
        for(int i = 0; i < specsList.size(); ++i) {
            const QFileInfo &fi = specsList.at(i);
            if( fi.fileName().left( 5 ) == "win32" ) {
                winPlatforms += fi.fileName();
            }
        }
        cout << "Available platforms are: " << qPrintable(winPlatforms.join( ", " )) << endl;
        dictionary[ "DONE" ] = "error";
    }
}

#if !defined(EVAL)
void Configure::generateCachefile()
{
    // Generate .qmake.cache
    QFile cacheFile( buildPath + "/.qmake.cache" );
    if( cacheFile.open( QFile::WriteOnly | QFile::Text ) ) { // Truncates any existing file.
        QTextStream cacheStream( &cacheFile );
        for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
            cacheStream << (*var) << endl;
        }
        cacheStream << "CONFIG         += " << qmakeConfig.join( " " ) << " incremental create_prl link_prl depend_includepath QTDIR_build" << endl;
	cacheStream << "QT_BUILD_PARTS  = libs tools examples demos" << endl;
        QString mkspec_path = fixSeparators(sourcePath + "/mkspecs/" + dictionary[ "QMAKESPEC" ]);
        if(QFile::exists(mkspec_path))
            cacheStream << "QMAKESPEC       = " << mkspec_path << endl;
        else
            cacheStream << "QMAKESPEC       = " << fixSeparators(dictionary[ "QMAKESPEC" ]) << endl;
        cacheStream << "ARCH            = " << fixSeparators(dictionary[ "ARCHITECTURE" ]) << endl;
        cacheStream << "QT_BUILD_TREE   = " << fixSeparators(dictionary[ "QT_BUILD_TREE" ]) << endl;
        cacheStream << "QT_SOURCE_TREE  = " << fixSeparators(dictionary[ "QT_SOURCE_TREE" ]) << endl;

        if (dictionary["QT_EDITION"] != "QT_EDITION_OPENSOURCE")
            cacheStream << "DEFINES        *= QT_EDITION=QT_EDITION_DESKTOP" << endl;

        //so that we can build without an install first (which would be impossible)
        cacheStream << "QMAKE_MOC       = $$QT_BUILD_TREE" << fixSeparators("/bin/moc.exe") << endl;
        cacheStream << "QMAKE_UIC       = $$QT_BUILD_TREE" << fixSeparators("/bin/uic.exe") << endl;
        cacheStream << "QMAKE_UIC3      = $$QT_BUILD_TREE" << fixSeparators("/bin/uic3.exe") << endl;
        cacheStream << "QMAKE_RCC       = $$QT_BUILD_TREE" << fixSeparators("/bin/rcc.exe") << endl;
        cacheStream << "QMAKE_DUMPCPP   = $$QT_BUILD_TREE" << fixSeparators("/bin/dumpcpp.exe") << endl;
        cacheStream << "QMAKE_INCDIR_QT = $$QT_BUILD_TREE" << fixSeparators("/include") << endl;
        cacheStream << "QMAKE_LIBDIR_QT = $$QT_BUILD_TREE" << fixSeparators("/lib") << endl;
        cacheStream.flush();
        cacheFile.close();
    }
    QFile configFile( dictionary[ "QT_BUILD_TREE" ] + "/mkspecs/qconfig.pri" );
    if( configFile.open( QFile::WriteOnly | QFile::Text ) ) { // Truncates any existing file.
        QTextStream configStream( &configFile );
        configStream << "CONFIG+= ";
        configStream << dictionary[ "BUILD" ];
        if( dictionary[ "SHARED" ] == "yes" )
            configStream << " shared";
        else
            configStream << " static";

        if( dictionary[ "STL" ] == "yes" )
            configStream << " stl";
        if ( dictionary[ "EXCEPTIONS" ] == "yes" )
            configStream << " exceptions";
        if ( dictionary[ "RTTI" ] == "yes" )
            configStream << " rtti";
        if ( dictionary[ "MMX" ] == "yes" )
            configStream << " mmx";
        if ( dictionary[ "3DNOW" ] == "yes" )
            configStream << " 3dnow";
        if ( dictionary[ "SSE" ] == "yes" )
            configStream << " sse";
        if ( dictionary[ "SSE2" ] == "yes" )
            configStream << " sse2";
        if ( dictionary["INCREDIBUILD_XGE"] == "yes" )
            configStream << " incredibuild_xge";

        configStream << endl;
        configStream << "QT_ARCH = " << dictionary[ "ARCHITECTURE" ] << endl;
        if (dictionary["QT_EDITION"].contains("OPENSOURCE"))
            configStream << "QT_EDITION = " << QLatin1String("OpenSource") << endl;
        else
            configStream << "QT_EDITION = " << dictionary["EDITION"] << endl;
        configStream << "QT_CONFIG += " << qtConfig.join(" ") << endl;

        configStream << "#versioning " << endl
                     << "QT_VERSION = " << dictionary["VERSION"] << endl
                     << "QT_MAJOR_VERSION = " << dictionary["VERSION_MAJOR"] << endl
                     << "QT_MINOR_VERSION = " << dictionary["VERSION_MINOR"] << endl
                     << "QT_PATCH_VERSION = " << dictionary["VERSION_PATCH"] << endl;

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
    QDir(buildPath).mkpath("src/corelib/global");
    QString outName( buildPath + "/src/corelib/global/qconfig.h" );
    QTemporaryFile tmpFile;
    QTextStream tmpStream;

    if(tmpFile.open()) {
        tmpStream.setDevice(&tmpFile);

        if( dictionary[ "QCONFIG" ] == "full" ) {
            tmpStream << "/* Everything */" << endl;
        } else {
            QString configName( "qconfig-" + dictionary[ "QCONFIG" ] + ".h" );
            tmpStream << "/* Copied from " << configName << "*/" << endl;
            tmpStream << "#ifndef QT_BOOTSTRAPPED" << endl;
            QFile inFile( buildPath + "/src/corelib/global/" + configName );
            if( inFile.open( QFile::ReadOnly ) ) {
                QByteArray buffer = inFile.readAll();
                tmpFile.write( buffer.constData(), buffer.size() );
                inFile.close();
            }
            tmpStream << "#endif // QT_BOOTSTRAPPED" << endl;
        }
        tmpStream << endl;

        if( dictionary[ "SHARED" ] == "yes" ) {
            tmpStream << "#ifndef QT_DLL" << endl;
            tmpStream << "#define QT_DLL" << endl;
            tmpStream << "#endif" << endl;
        }
        tmpStream << endl;
        tmpStream << "/* License information */" << endl;
        tmpStream << "#define QT_PRODUCT_LICENSEE \"" << licenseInfo[ "LICENSEE" ] << "\"" << endl;
        tmpStream << "#define QT_PRODUCT_LICENSE \"" << dictionary[ "EDITION" ] << "\"" << endl;
        tmpStream << endl;
        tmpStream << "// Qt Edition" << endl;
        tmpStream << "#ifndef QT_EDITION" << endl;
        tmpStream << "#  define QT_EDITION " << dictionary["QT_EDITION"] << endl;
        tmpStream << "#endif" << endl;
        tmpStream << endl;
        tmpStream << dictionary["BUILD_KEY"];
        tmpStream << endl;
    if (dictionary["EDITION"] == "Trolltech") {
        tmpStream << "/* Used for example to export symbols for the certain autotests*/" << endl;
        tmpStream << "#define QT_BUILD_INTERNAL" << endl;
        tmpStream << endl;
    }
        tmpStream << "/* Machine byte-order */" << endl;
        tmpStream << "#define Q_BIG_ENDIAN 4321" << endl;
        tmpStream << "#define Q_LITTLE_ENDIAN 1234" << endl;
        if ( QSysInfo::ByteOrder == QSysInfo::BigEndian )
            tmpStream << "#define Q_BYTE_ORDER Q_BIG_ENDIAN" << endl;
        else
            tmpStream << "#define Q_BYTE_ORDER Q_LITTLE_ENDIAN" << endl;

        tmpStream << endl << "// Compile time features" << endl;
        tmpStream << "#define QT_ARCH_" << dictionary["ARCHITECTURE"].toUpper() << endl;
        QStringList qconfigList;
        if(dictionary["STL"] == "no")                qconfigList += "QT_NO_STL";
        if(dictionary["STYLE_WINDOWS"] != "yes")     qconfigList += "QT_NO_STYLE_WINDOWS";
        if(dictionary["STYLE_PLASTIQUE"] != "yes")   qconfigList += "QT_NO_STYLE_PLASTIQUE";
        if(dictionary["STYLE_CLEANLOOKS"] != "yes")   qconfigList += "QT_NO_STYLE_CLEANLOOKS";
        if(dictionary["STYLE_WINDOWSXP"] != "yes")   qconfigList += "QT_NO_STYLE_WINDOWSXP";
        if(dictionary["STYLE_WINDOWSVISTA"] != "yes")   qconfigList += "QT_NO_STYLE_WINDOWSVISTA";
        if(dictionary["STYLE_MOTIF"] != "yes")       qconfigList += "QT_NO_STYLE_MOTIF";
        if(dictionary["STYLE_CDE"] != "yes")         qconfigList += "QT_NO_STYLE_CDE";

        if(dictionary["GIF"] == "yes")              qconfigList += "QT_BUILTIN_GIF_READER=1";
        if(dictionary["PNG"] == "no")               qconfigList += "QT_NO_IMAGEFORMAT_PNG";
        if(dictionary["MNG"] == "no")               qconfigList += "QT_NO_IMAGEFORMAT_MNG";
        if(dictionary["JPEG"] == "no")              qconfigList += "QT_NO_IMAGEFORMAT_JPEG";
        if(dictionary["TIFF"] == "no")              qconfigList += "QT_NO_IMAGEFORMAT_TIFF";
        if(dictionary["ZLIB"] == "no") {
            qconfigList += "QT_NO_ZLIB";
            qconfigList += "QT_NO_COMPRESS";
        }

        if(dictionary["QT3SUPPORT"] == "no")        qconfigList += "QT_NO_QT3SUPPORT";
        if(dictionary["ACCESSIBILITY"] == "no")     qconfigList += "QT_NO_ACCESSIBILITY";
        if(dictionary["EXCEPTIONS"] == "no")        qconfigList += "QT_NO_EXCEPTIONS";
        if(dictionary["OPENGL"] == "no")            qconfigList += "QT_NO_OPENGL";
        if(dictionary["DIRECT3D"] == "no")          qconfigList += "QT_NO_DIRECT3D";
        if(dictionary["OPENSSL"] == "no")          qconfigList += "QT_NO_OPENSSL";
        if(dictionary["QDBUS"] == "no")             qconfigList += "QT_NO_QDBUS";
        if(dictionary["IPV6"] == "no")              qconfigList += "QT_NO_IPV6";

        if(dictionary["SQL_MYSQL"] == "yes")        qconfigList += "QT_SQL_MYSQL";
        if(dictionary["SQL_ODBC"] == "yes")         qconfigList += "QT_SQL_ODBC";
        if(dictionary["SQL_OCI"] == "yes")          qconfigList += "QT_SQL_OCI";
        if(dictionary["SQL_PSQL"] == "yes")         qconfigList += "QT_SQL_PSQL";
        if(dictionary["SQL_TDS"] == "yes")          qconfigList += "QT_SQL_TDS";
        if(dictionary["SQL_DB2"] == "yes")          qconfigList += "QT_SQL_DB2";
        if(dictionary["SQL_SQLITE"] == "yes")       qconfigList += "QT_SQL_SQLITE";
        if(dictionary["SQL_SQLITE2"] == "yes")      qconfigList += "QT_SQL_SQLITE2";
        if(dictionary["SQL_IBASE"] == "yes")        qconfigList += "QT_SQL_IBASE";

        qconfigList.sort();
        for (int i = 0; i < qconfigList.count(); ++i)
            tmpStream << addDefine(qconfigList.at(i));

        tmpStream.flush();
        tmpFile.flush();

        // Replace old qconfig.h with new one
        ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL);
        QFile::remove(outName);
        tmpFile.copy(outName);
        tmpFile.close();

        if(!QFile::exists(buildPath + "/include/QtCore/qconfig.h")) {
            if (!writeToFile("#include \"../../src/corelib/global/qconfig.h\"\n",
                             buildPath + "/include/QtCore/qconfig.h")
            || !writeToFile("#include \"../../src/corelib/global/qconfig.h\"\n",
                            buildPath + "/include/Qt/qconfig.h")) {
                dictionary["DONE"] = "error";
                return;
            }
        }
    }

    // Copy configured mkspec to default directory, but remove the old one first, if there is any
    QString defSpec = buildPath + "/mkspecs/default";
    QFileInfo defSpecInfo(defSpec);
    if (defSpecInfo.exists()) {
        if (!Environment::rmdir(defSpec)) {
            cout << "Couldn't update default mkspec! Are files in " << qPrintable(defSpec) << " read-only?" << endl;
            dictionary["DONE"] = "error";
            return;
        }
    }

    QString pltSpec = sourcePath + "/mkspecs/" + dictionary["QMAKESPEC"];
    if (!Environment::cpdir(pltSpec, defSpec)) {
        cout << "Couldn't update default mkspec! Does " << qPrintable(pltSpec) << " exist?" << endl;
        dictionary["DONE"] = "error";
        return;
    }

    outName = defSpec + "/qmake.conf";
    ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    QFile qmakeConfFile(outName);
    if (qmakeConfFile.open(QFile::Append | QFile::WriteOnly | QFile::Text)) {
        QTextStream qmakeConfStream;
        qmakeConfStream.setDevice(&qmakeConfFile);
        qmakeConfStream << endl << "QMAKESPEC_ORIGINAL=" << pltSpec << endl;
        qmakeConfStream.flush();
        qmakeConfFile.close();
    }

    // Generate the new qconfig.cpp file
    QDir(buildPath).mkpath("src/corelib/global");
    outName = buildPath + "/src/corelib/global/qconfig.cpp";

    QTemporaryFile tmpFile2;
    if (tmpFile2.open()) {
        tmpStream.setDevice(&tmpFile2);

        tmpStream << "/* Licensed */" << endl
                  << "static const char qt_configure_licensee_str          [512 + 12] = \"qt_lcnsuser=" << licenseInfo["LICENSEE"] << "\";" << endl
                  << "static const char qt_configure_licensed_products_str [512 + 12] = \"qt_lcnsprod=" << dictionary["EDITION"] << "\";" << endl
                  << "static const char qt_configure_prefix_path_str       [512 + 12] = \"qt_prfxpath=" << QString(dictionary["QT_INSTALL_PREFIX"]).replace( "\\", "\\\\" ) << "\";" << endl
                  << "static const char qt_configure_documentation_path_str[512 + 12] = \"qt_docspath=" << QString(dictionary["QT_INSTALL_DOCS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_headers_path_str      [512 + 12] = \"qt_hdrspath=" << QString(dictionary["QT_INSTALL_HEADERS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_libraries_path_str    [512 + 12] = \"qt_libspath=" << QString(dictionary["QT_INSTALL_LIBS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_binaries_path_str     [512 + 12] = \"qt_binspath=" << QString(dictionary["QT_INSTALL_BINS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_plugins_path_str      [512 + 12] = \"qt_plugpath=" << QString(dictionary["QT_INSTALL_PLUGINS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_data_path_str         [512 + 12] = \"qt_datapath=" << QString(dictionary["QT_INSTALL_DATA"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_translations_path_str [512 + 12] = \"qt_trnspath=" << QString(dictionary["QT_INSTALL_TRANSLATIONS"]).replace( "\\", "\\\\" ) << "\";" << endl
                  << "static const char qt_configure_examples_path_str     [512 + 12] = \"qt_xmplpath=" << QString(dictionary["QT_INSTALL_EXAMPLES"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_demos_path_str        [512 + 12] = \"qt_demopath=" << QString(dictionary["QT_INSTALL_DEMOS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  //<< "static const char qt_configure_settings_path_str [256] = \"qt_stngpath=" << QString(dictionary["QT_INSTALL_SETTINGS"]).replace( "\\", "\\\\" ) << "\";" << endl
                  << "/* strlen( \"qt_lcnsxxxx\" ) == 12 */" << endl
                  << "#define QT_CONFIGURE_LICENSEE qt_configure_licensee_str + 12;" << endl
                  << "#define QT_CONFIGURE_LICENSED_PRODUCTS qt_configure_licensed_products_str + 12;" << endl
                  << "#define QT_CONFIGURE_PREFIX_PATH qt_configure_prefix_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_DOCUMENTATION_PATH qt_configure_documentation_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_HEADERS_PATH qt_configure_headers_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_LIBRARIES_PATH qt_configure_libraries_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_BINARIES_PATH qt_configure_binaries_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_PLUGINS_PATH qt_configure_plugins_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_DATA_PATH qt_configure_data_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_TRANSLATIONS_PATH qt_configure_translations_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_EXAMPLES_PATH qt_configure_examples_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_DEMOS_PATH qt_configure_demos_path_str + 12;" << endl
                  //<< "#define QT_CONFIGURE_SETTINGS_PATH qt_configure_settings_path_str + 12;" << endl
                  << endl;

        tmpStream.flush();
        tmpFile2.flush();
        
        // Replace old qconfig.cpp with new one
        ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
        QFile::remove( outName );
        tmpFile2.copy(outName);
        tmpFile2.close();
    }
}
#endif

#if !defined(EVAL)
void Configure::displayConfig()
{
    // Give some feedback
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

    if (dictionary["EDITION"] == "Trolltech") {
        cout << "Trolltech license file used (" << dictionary["LICENSE FILE"] << ")" << endl;
    } else if (dictionary["EDITION"] == "OpenSource") {
        cout << "You are licensed to use this software under the terms of the GNU GPL." << endl;
        cout << "See " << dictionary["LICENSE FILE"] << endl << endl;
    } else {
        QString l1 = licenseInfo[ "LICENSEE" ];
        QString l2 = licenseInfo[ "LICENSEID" ];
        QString l3 = dictionary["EDITION"] + ' ' + "Edition";
        QString l4 = licenseInfo[ "EXPIRYDATE" ];
        cout << "Licensee...................." << (l1.isNull() ? "" : l1) << endl;
        cout << "License ID.................." << (l2.isNull() ? "" : l2) << endl;
        cout << "Product license............." << (l3.isNull() ? "" : l3) << endl;
        cout << "Expiry Date................." << (l4.isNull() ? "" : l4) << endl << endl;
    }

    cout << "Configuration:" << endl;
    cout << "    " << qmakeConfig.join( "\r\n    " ) << endl;
    cout << "Qt Configuration:" << endl;
    cout << "    " << qtConfig.join( "\r\n    " ) << endl;
    cout << endl;

    cout << "QMAKESPEC..................." << dictionary[ "QMAKESPEC" ] << " (" << dictionary["QMAKESPEC_FROM"] << ")" << endl;
    cout << "Architecture................" << dictionary[ "ARCHITECTURE" ] << endl;
    cout << "Maketool...................." << dictionary[ "MAKE" ] << endl;
    cout << "Debug symbols..............." << (dictionary[ "BUILD" ] == "debug" ? "yes" : "no") << endl;
    cout << "Accessibility support......." << dictionary[ "ACCESSIBILITY" ] << endl;
    cout << "STL support................." << dictionary[ "STL" ] << endl;
    cout << "Exception support..........." << dictionary[ "EXCEPTIONS" ] << endl;
    cout << "RTTI support................" << dictionary[ "RTTI" ] << endl;
    cout << "MMX support................." << dictionary[ "MMX" ] << endl;
    cout << "3DNOW support..............." << dictionary[ "3DNOW" ] << endl;
    cout << "SSE support................." << dictionary[ "SSE" ] << endl;
    cout << "SSE2 support................" << dictionary[ "SSE2" ] << endl;
    cout << "OpenGL support.............." << dictionary[ "OPENGL" ] << endl;
    cout << "Direct3D support............" << dictionary[ "DIRECT3D" ] << endl;
    cout << "OpenSSL support............." << dictionary[ "OPENSSL" ] << endl;
    cout << "QDBus support..............." << dictionary[ "QDBUS" ] << endl;
    cout << "Qt3 compatibility..........." << dictionary[ "QT3SUPPORT" ] << endl << endl;

    cout << "Third Party Libraries:" << endl;
    cout << "    ZLIB support............" << dictionary[ "ZLIB" ] << endl;
    cout << "    GIF support............." << dictionary[ "GIF" ] << endl;
    cout << "    TIFF support............" << dictionary[ "TIFF" ] << endl;
    cout << "    JPEG support............" << dictionary[ "JPEG" ] << endl;
    cout << "    PNG support............." << dictionary[ "PNG" ] << endl;
    cout << "    MNG support............." << dictionary[ "MNG" ] << endl << endl;

    cout << "Styles:" << endl;
    cout << "    Windows................." << dictionary[ "STYLE_WINDOWS" ] << endl;
    cout << "    Windows XP.............." << dictionary[ "STYLE_WINDOWSXP" ] << endl;
    cout << "    Windows Vista..........." << dictionary[ "STYLE_WINDOWSVISTA" ] << endl;
    cout << "    Plastique..............." << dictionary[ "STYLE_PLASTIQUE" ] << endl;
    cout << "    Cleanlooks.............." << dictionary[ "STYLE_CLEANLOOKS" ] << endl;
    cout << "    Motif..................." << dictionary[ "STYLE_MOTIF" ] << endl;
    cout << "    CDE....................." << dictionary[ "STYLE_CDE" ] << endl << endl;
    // Only show the PocketPC style option for CE users

    cout << "Sql Drivers:" << endl;
    cout << "    ODBC...................." << dictionary[ "SQL_ODBC" ] << endl;
    cout << "    MySQL..................." << dictionary[ "SQL_MYSQL" ] << endl;
    cout << "    OCI....................." << dictionary[ "SQL_OCI" ] << endl;
    cout << "    PostgreSQL.............." << dictionary[ "SQL_PSQL" ] << endl;
    cout << "    TDS....................." << dictionary[ "SQL_TDS" ] << endl;
    cout << "    DB2....................." << dictionary[ "SQL_DB2" ] << endl;
    cout << "    SQLite.................." << dictionary[ "SQL_SQLITE" ] << " (" << dictionary[ "SQL_SQLITE_LIB" ] << ")" << endl;
    cout << "    SQLite2................." << dictionary[ "SQL_SQLITE2" ] << endl;
    cout << "    InterBase..............." << dictionary[ "SQL_IBASE" ] << endl << endl;

    cout << "Sources are in.............." << dictionary[ "QT_SOURCE_TREE" ] << endl;
    cout << "Build is done in............" << dictionary[ "QT_BUILD_TREE" ] << endl;
    cout << "Install prefix.............." << dictionary[ "QT_INSTALL_PREFIX" ] << endl;
    cout << "Headers installed to........" << dictionary[ "QT_INSTALL_HEADERS" ] << endl;
    cout << "Libraries installed to......" << dictionary[ "QT_INSTALL_LIBS" ] << endl;
    cout << "Plugins installed to........" << dictionary[ "QT_INSTALL_PLUGINS" ] << endl;
    cout << "Binaries installed to......." << dictionary[ "QT_INSTALL_BINS" ] << endl;
    cout << "Docs installed to..........." << dictionary[ "QT_INSTALL_DOCS" ] << endl;
    cout << "Data installed to..........." << dictionary[ "QT_INSTALL_DATA" ] << endl;
    cout << "Translations installed to..." << dictionary[ "QT_INSTALL_TRANSLATIONS" ] << endl;
    cout << "Examples installed to......." << dictionary[ "QT_INSTALL_EXAMPLES" ] << endl;
    cout << "Demos installed to.........." << dictionary[ "QT_INSTALL_DEMOS" ] << endl << endl;

    if(checkAvailability("INCREDIBUILD_XGE"))
        cout << "Using IncrediBuild XGE......" << dictionary["INCREDIBUILD_XGE"] << endl;
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
void Configure::generateHeaders()
{
    if (dictionary["SYNCQT"] == "yes"
        && findFile("perl.exe")) {
        cout << "Running syncqt..." << endl;
        QStringList args;
        args += buildPath + "/bin/syncqt.bat";
        QStringList env;
        env += QString("QTDIR=" + sourcePath);
        //env += QString("PATH=" + buildPath + "/bin/;%PATH%");
        Environment::execute(args, env, QStringList());
    }
}

void Configure::buildQmake()
{
    if( dictionary[ "BUILD_QMAKE" ] == "yes" ) {
        QStringList args;

        // Build qmake
        QString pwd = QDir::currentPath();
        QDir::setCurrent(buildPath + "/qmake" );

        QString makefile = "Makefile";
        {
            QFile out(makefile);
            if(out.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&out);
                stream << "#AutoGenerated by configure.exe" << endl
                       << "BUILD_PATH = " << QDir::convertSeparators(buildPath) << endl
                       << "SOURCE_PATH = " << QDir::convertSeparators(sourcePath) << endl
                       << "QMAKESPEC = " << dictionary["QMAKESPEC"] << endl;
                if (dictionary["EDITION"] == "OpenSource" ||
                    dictionary["QT_EDITION"].contains("OPENSOURCE"))
                    stream << "QMAKE_OPENSOURCE_EDITION = yes" << endl;
                stream << endl << endl;

                QFile in(sourcePath + "/qmake/" + dictionary["QMAKEMAKEFILE"]);
                if(in.open(QFile::ReadOnly | QFile::Text)) {
                    QString d = in.readAll();
                    //### need replaces (like configure.sh)? --Sam
                    stream << d << endl;
                }
                stream.flush();
                out.close();
            }
        }

        args += dictionary[ "MAKE" ];
        args += "-f";
        args += makefile;

        cout << "Creating qmake..." << endl;
        int exitCode = 0;
        if( exitCode = Environment::execute(args, QStringList(), QStringList()) ) {
            args.clear();
            args += dictionary[ "MAKE" ];
            args += "-f";
            args += makefile;
            args += "clean";
            if( exitCode = Environment::execute(args, QStringList(), QStringList())) {
                cout << "Cleaning qmake failed, return code " << exitCode << endl << endl;
                dictionary[ "DONE" ] = "error";
            } else {
                args.clear();
                args += dictionary[ "MAKE" ];
                args += "-f";
                args += makefile;
                if (exitCode = Environment::execute(args, QStringList(), QStringList())) {
                    cout << "Building qmake failed, return code " << exitCode << endl << endl;
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

        const QFileInfoList &list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i) {
            const QFileInfo &fi = list.at(i);
            if( fi.fileName()[ 0 ] != '.' && fi.fileName() != "qmake.pro" ) {
                entryName = dirName + "/" + fi.fileName();
                if(fi.isDir()) {
                    findProjects( entryName );
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
                        makeList[makeListNumber].append( new MakeItem(sourceDir.relativeFilePath(fi.absolutePath()),
                                                                      fi.fileName(),
                                                                      "Makefile",
                                                                      qmakeTemplate ) );
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
    dir.prepend("/src");
    makeList[inList].append(new MakeItem(sourcePath + dir,
        item + ".pro", buildPath + dir + "/Makefile", Lib ) );
    if( dictionary[ "DSPFILES" ] == "yes" ) {
        makeList[inList].append( new MakeItem(sourcePath + dir,
            item + ".pro", buildPath + dir + "/" + item + ".dsp", Lib ) );
    }
    if( dictionary[ "VCPFILES" ] == "yes" ) {
        makeList[inList].append( new MakeItem(sourcePath + dir,
            item + ".pro", buildPath + dir + "/" + item + ".vcp", Lib ) );
    }
    if( dictionary[ "VCPROJFILES" ] == "yes" ) {
        makeList[inList].append( new MakeItem(sourcePath + dir,
            item + ".pro", buildPath + dir + "/" + item + ".vcproj", Lib ) );
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

        if( spec != "win32-msvc.net" && spec != "win32-msvc2005" )
            dictionary[ "VCPROJFILES" ] = "no";

        int i = 0;
        QString pwd = QDir::currentPath();
        if (dictionary["FAST"] != "yes") {
            QString dirName;
            bool generate = true;
            bool doDsp = (dictionary["DSPFILES"] == "yes" || dictionary["VCPFILES"] == "yes"
                          || dictionary["VCPROJFILES"] == "yes");
            while (generate) {
                QString pwd = QDir::currentPath();
                QString dirPath = fixSeparators(buildPath + dirName);
                QStringList args;

                args << fixSeparators( buildPath + "/bin/qmake" );

                if (doDsp) {
                    if( dictionary[ "DEPENDENCIES" ] == "no" )
                        args << "-nodepend";
                    args << "-tp" <<  "vc";
                    doDsp = false; // DSP files will be done
                    printf("Generating Visual Studio project files...\n");
                } else {
                    printf("Generating Makefiles...\n");
                    generate = false; // Now Makefiles will be done
                }
                args << "-spec";
                args << dictionary[ "QMAKESPEC" ];
                args << "-r";
                args << (sourcePath + "/projects.pro");
                args << "-o";
                args << buildPath;

                QDir::setCurrent( fixSeparators( dirPath ) );
                if( int exitCode = Environment::execute(args, QStringList(), QStringList()) ) {
                    cout << "Qmake failed, return code " << exitCode  << endl << endl;
                    dictionary[ "DONE" ] = "error";
                }
            }
        } else {
            findProjects(sourcePath);
            for ( i=0; i<3; i++ ) {
                for ( int j=0; j<makeList[i].size(); ++j) {
                    MakeItem *it=makeList[i][j];
                    QString dirPath = fixSeparators( it->directory + "/" );
                    QString projectName = it->proFile;
                    QString makefileName = buildPath + "/" + dirPath + it->target;
                    QStringList args;

                    args << fixSeparators( buildPath + "/bin/qmake" );
                    args << projectName;
                    args << dictionary[ "QMAKE_ALL_ARGS" ];

                    cout << "For " << qPrintable(dirPath + projectName) << endl;
                    args << "-o";
                    args << it->target;
                    args << "-spec";
                    args << dictionary[ "QMAKESPEC" ];

                    QDir::setCurrent( fixSeparators( dirPath ) );

                    QFile file(makefileName);
                    if (!file.open(QFile::WriteOnly)) {
                        printf("failed on dirPath=%s, makefile=%s\n",
                            qPrintable(dirPath), qPrintable(makefileName));
                        continue;
                    }
                    QTextStream txt(&file);
                    txt << "all:\n";
                    txt << "\t" << args.join(" ") << "\n";
                    txt << "\t" << dictionary[ "MAKE" ] << " -f " << it->target << "\n";
                    txt << "first: all\n";
                    txt << "qmake:\n";
                    txt << "\t" << args.join(" ") << "\n";
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
    cout << "To reconfigure, run " << qPrintable(make) << " confclean and configure." << endl << endl;
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

#if !defined(EVAL)

bool Configure::showLicense(const QString &licenseFile)
{
    if (dictionary["LICENSE_CONFIRMED"] == "yes") {
        cout << "You have already accepted the terms of the license." << endl << endl;
        return true;
    }

    QString theLicense;
    if (dictionary["EDITION"] == "OpenSource") {
        theLicense = "GNU General Public License";
    } else {
        // the first line of the license file tells us which license it is
        QFile file(licenseFile);
        if (!file.open(QFile::ReadOnly)) {
            cout << "Failed to load LICENSE file";
            return false;
        }
        theLicense = file.readLine().trimmed();
    }

    forever {
        char accept = '?';
        cout << "You are licensed to use this software under the terms of" << endl
             << "the " << theLicense << "." << endl
             << endl
             << "Type '?' to view the " << theLicense << "." << endl
             << "Type 'y' to accept this license offer." << endl
             << "Type 'n' to decline this license offer." << endl
             << endl
             << "Do you accept the terms of the license?" << endl;
        cin >> accept;
        accept = tolower(accept);

        if (accept == 'y') {
            return true;
        } else if (accept == 'n') {
            return false;
        } else {
            // Get console line height, to fill the screen properly
            int i = 0, screenHeight = 25; // default
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (GetConsoleScreenBufferInfo(stdOut, &consoleInfo))
                screenHeight = consoleInfo.srWindow.Bottom
                             - consoleInfo.srWindow.Top
                             - 1; // Some overlap for context

            // Prompt the license content to the user
            QFile file(licenseFile);
            if (!file.open(QFile::ReadOnly)) {
                cout << "Failed to load LICENSE file";
                return false;
            }
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
        }
    }
}

void Configure::readLicense()
{
    dictionary["LICENSE FILE"] = sourcePath + "/LICENSE.GPL";
    if (QFile::exists(dictionary["LICENSE FILE"])) {
        cout << endl << "This is the Qt/Windows Open Source Edition." << endl;
        licenseInfo["LICENSEE"] = "Open Source";
        dictionary["EDITION"] = "OpenSource";
        dictionary["QT_EDITION"] = "QT_EDITION_OPENSOURCE";
        // Ensure that the right QMAKESPEC is used for the Open Source version
        if (!dictionary["QMAKESPEC"].endsWith("-g++")) {
            cout << "The Qt/Windows Open Source Edition only supports the MinGW compiler." << endl;
            dictionary["DONE"] = "error";
            return;
        }
        cout << endl;
        if (!showLicense(dictionary["LICENSE FILE"])) {
            cout << "Configuration aborted since license was not accepted";
            dictionary["DONE"] = "error";
            return;
        }
        return;
    }
#ifndef COMMERCIAL_VERSION
    else {
        cout << endl << "Cannot find the GPL license file (" << dictionary["LICENSE FILE"] << ")" << endl;
        dictionary["DONE"] = "error";
    }
#else
    else if (Tools::checkInternalLicense(dictionary)) {
        licenseInfo["LICENSEE"] = dictionary["EDITION"];
        return;
    }

    Tools::checkLicense(dictionary, licenseInfo, firstLicensePath());
    if (dictionary["DONE"] != "error") {
        // give the user some feedback, and prompt for license acceptance
        cout << endl << "This is the Qt/Windows " << dictionary["EDITION"] << " Edition."<< endl << endl;
        if (!showLicense(dictionary["LICENSE FILE"])) {
            cout << "Configuration aborted since license was not accepted";
            dictionary["DONE"] = "error";
            return;
        }
        if (!dictionary.contains("METERED LICENSE"))
            QFile::remove(sourcePath + "/bin/qtusagereporter.exe");
    }
#endif // COMMERCIAL_VERSION
}

void Configure::reloadCmdLine()
{
    if( dictionary[ "REDO" ] == "yes" ) {
        QFile inFile( buildPath + "/configure" + dictionary[ "CUSTOMCONFIG" ] + ".cache" );
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

void Configure::saveCmdLine()
{
    if( dictionary[ "REDO" ] != "yes" ) {
        QFile outFile( buildPath + "/configure" + dictionary[ "CUSTOMCONFIG" ] + ".cache" );
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
#endif // !EVAL

bool Configure::isDone()
{
    return !dictionary["DONE"].isEmpty();
}

bool Configure::isOk()
{
    return (dictionary[ "DONE" ] != "error");
}

bool
Configure::filesDiffer(const QString &fn1, const QString &fn2)
{
    QFile file1(fn1), file2(fn2);
    if(!file1.open(QFile::ReadOnly) || !file2.open(QFile::ReadOnly))
        return true;
    const int chunk = 2048;
    int used1 = 0, used2 = 0;
    char b1[chunk], b2[chunk];
    while(!file1.atEnd() && !file2.atEnd()) {
        if(!used1)
            used1 = file1.read(b1, chunk);
        if(!used2)
            used2 = file2.read(b2, chunk);
        if(used1 > 0 && used2 > 0) {
            const int cmp = qMin(used1, used2);
            if(memcmp(b1, b2, cmp))
                return true;
            if((used1 -= cmp))
                memcpy(b1, b1+cmp, used1);
            if((used2 -= cmp))
                memcpy(b2, b2+cmp, used2);
        }
    }
    return !file1.atEnd() || !file2.atEnd();
}
