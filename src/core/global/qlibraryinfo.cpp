/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qdir.h"
#include "qfile.h"
#include "qconfig.h"
#include "qsettings.h"
#include "qlibraryinfo.h"
#ifndef QT_NO_QOBJECT
# include "qcoreapplication.h"
#endif

#include "qconfig.cpp"
Q_CORE_EXPORT QString qt_library_config_file;

/*! \class QLibraryInfo
    \brief The QLibraryInfo class provides information about the Qt library.

    \ingroup misc
    \mainclass

    Many pieces of information are established when Qt is
    configured. Installation paths, license information, and even a
    unique build key. This class provides an abstraction for accessing
    this information.
*/

/*! \internal

   You cannot create a QLibraryInfo, instead only the static functions are available to query
   information.
*/

QLibraryInfo::QLibraryInfo()
{
    
}

/*!
  Returns the person to whom this build of Qt is licensed.

  \sa QLibraryInfo::licensedProducts
*/

QString
QLibraryInfo::licensee()
{
    return QT_CONFIGURE_LICENSEE;
}

/*!
  Returns the products that the license for this build of Qt has access to.

  \sa QLibraryInfo::licensee
*/

QString
QLibraryInfo::licensedProducts()
{
    return QT_CONFIGURE_LICENSED_PRODUCTS;
}

/*!
  Returns a unique key identifying this build of Qt and its
  configurations. This key is not globally unique, rather only useful
  for establishing of two configurations are compatible. This can be
  used to compare with QT_BUILD_KEY.
*/

QString
QLibraryInfo::buildKey()
{
    return QT_BUILD_KEY;
}

static bool qt_find_qlibrary_info_config()
{
    { //look in the binary itself
        QString qtconfig = ":/qt/etc/qt.conf";
        if(QFile::exists(qtconfig)) {
            qt_library_config_file = qtconfig;
            return true;
        }
    }
    if(char *qtconfig_str = qgetenv("QTCONFIG")) {     //look in QTCONFIG
        QString qtconfig = QString::fromUtf8(qtconfig_str);
        if(QFile::exists(qtconfig)) {
            qt_library_config_file = qtconfig;
            return true;
        }
    }
    if(char *qtdir = qgetenv("QTDIR")) {     //look in QTDIR
        QString qtconfig = QString::fromUtf8(qtdir) + "/" + "qt.conf";
        if(QFile::exists(qtconfig)) {
            qt_library_config_file = qtconfig;
            return true;
        }
    }
    { //look in the home dir
        QString qtconfig = QDir::homePath() + "/" + ".qt.conf";
        if(QFile::exists(qtconfig)) {
            qt_library_config_file = qtconfig;
            return true;
        }
    }
#ifndef QT_NO_QOBJECT
    if(QCoreApplication *app = QCoreApplication::instance()) { //walk up the file system from the exe to (a) root
        if(app->argv() && app->argv()[0]) {
            bool trySearch = true;
            QString exe = app->argv()[0];
            if(QDir::isRelativePath(exe)) {
                if((exe.indexOf('/') != -1
#ifdef Q_OS_WIN
                    || exe.indexOf('\\') != -1
#endif
                       ) && QFile::exists(exe)) {
                    exe.prepend(QDir::currentPath() + "/");
                } else if(char *path = qgetenv("PATH")) {
#ifdef Q_OS_WIN
                    QStringList paths = QString(path).split(';');
#else
                    QStringList paths = QString(path).split(':');
#endif
                    bool found = false;
                    for(int i = 0; i < paths.size(); ++i) {
                        const QString fexe = paths.at(i) + "/" + exe;
                        if(QFile::exists(fexe)) {
                            found = true;
                            exe = fexe;
                            break;
                        }
                    }
                    trySearch = found;
                }
            }
            if(trySearch) {
                QDir pwd(QFileInfo(exe).path());
                while(1) {
                    if(pwd.exists("qt.conf")) {
                        qt_library_config_file = pwd.filePath("qt.conf");
                        return true;
                    }
                    if(pwd.isRoot())
                        break;
                    pwd.cdUp();
                }
            }
        }
    }
#endif
    { //walk up the file system from PWD to (a) root
        QDir pwd = QDir::current();
        while(1) {
            if(pwd.exists("qt.conf")) {
                qt_library_config_file = pwd.filePath("qt.conf");
                return true;
            }
            if(pwd.isRoot())
                break;
            pwd.cdUp();
        }
    }
#ifdef Q_OS_UNIX
    { //look in the /etc
        QString qtconfig = "/etc/qt.conf";
        if(QFile::exists(qtconfig)) {
            qt_library_config_file = qtconfig;
            return true;
        }
    }
    { //look in the /usr/local/etc
        QString qtconfig = "/usr/local/etc/qt.conf";
        if(QFile::exists(qtconfig)) {
            qt_library_config_file = qtconfig;
            return true;
        }
    }
#endif
#ifdef Q_OS_WIN
    { //registry key
        //###TDB
    }
#endif
    
    return false;     //no luck
}

/*!
  Returns the active configuration information file. This is normally
  only usefull for debugging but could be usefull to retrieve custom
  information. This file is a QSettings readable INI file. Qt will
  automatically find the configuration file by looking (in order):

  \list

  \i A user argument to your application of -qtconfig
  <config_location>.

  \i A resource of the name :/qt/etc/qt.conf.

  \i An environment variable QTCONFIG.

  \i An environment variable QTDIR/qt.conf (for Qt3 compatiblity).

  \i A file in $(HOME)/.qt.conf.

  \i A file of the name qt.conf in the directory from which your
  application executable lives. The filesystem will be walked up from
  that directory to the root.

  \i A file of the name qt.conf in the directory from which your
  application executable was run. The filesystem will be walked up
  from that directory to the root.

  \i A file in /etc/qt.conf (Unix only)

  \i A file in /usr/local//etc/qt.conf (Unix only)

  \endlist

  If no configuration can be found a null QString will be returned.

  \sa QLibrayInfo::location, QSettings
*/

QString
QLibraryInfo::configuration()
{
    if(qt_library_config_file.isNull()) 
        qt_find_qlibrary_info_config();
    if(!QFile::exists(qt_library_config_file))
        return QString();
    return qt_library_config_file;
}

/*!
  Returns the location specified by \a loc. 

  \sa QLibraryInfo::LibraryLocation, QLibraryInfo::configuration
*/

QString
QLibraryInfo::location(LibraryLocation loc)
{
    if(qt_library_config_file.isNull()) 
        qt_find_qlibrary_info_config();
    if(!qt_library_config_file.isNull()) {
        QString key;
        switch(loc) {
        case PrefixPath:
            key = "PrefixPath";
            break;
        case DocumentationPath:
            key = "DocumentationPath";
            break;
        case HeadersPath:
            key = "HeadersPath";
            break;
        case LibrariesPath:
            key = "LibrariesPath";
            break;
        case BinariesPath:
            key = "BinariesPath";
            break;
        case PluginsPath:
            key = "PluginsPath";
            break;
        case DataPath:
            key = "DataPath";
            break;
        case TranslationsPath:
            key = "TranslationsPath";
            break;
        case SettingsPath:
            key = "SettingsPath";
            break;
        default:
            break;
        }
        if(!key.isNull()) {
            QSettings settings(qt_library_config_file, QSettings::IniFormat);
            return settings.value("QtCore/" + key, QDir::currentPath()).toString();
        }
    }
    return QString();
}


/*!
    \enum QLibraryInfo::LibraryLocation

    \keyword library location

    This enum type is used to specify a specific location
    specifier. This for use with QLibraryInfo::location.

    The locations are

    \value PrefixPath The default prefix for all paths.
    \value DocumentationPath The location for documentation upon install.
    \value HeadersPath The location for all headers.
    \value LibrariesPath The location of installed librarires.
    \value BinariesPath The location of installed Qt binaries (tools and applications).
    \value PluginsPath The location of installed Qt plugins.
    \value DataPath The location of general Qt data.
    \value TranslationsPath The location of translation information for Qt strings.
    \value SettingsPath The location for Qt settings.
*/
