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
# include "qpointer.h"
# include "qcoreapplication.h"
#endif

#ifdef Q_OS_WIN
#include "qt_windows.h"
#endif

#include "qconfig.cpp"


Q_GLOBAL_STATIC(QString, qt_library_config_file)
Q_CORE_EXPORT void qt_set_library_config_file(const QString &p) { *(qt_library_config_file()) = p; }


struct QLibrarySettings
{
    QLibrarySettings();
    ~QLibrarySettings() { delete static_cast<QSettings *>(settings); }

#ifdef QT_NO_QOBJECT
    QSettings *settings;
#else
    QPointer<QSettings> settings;
#endif
};

class QLibraryInfoPrivate
{
    friend struct QLibrarySettings;
private:
    static QSettings *findConfiguration();
    static void cleanup()
    {
        QLibrarySettings *ls = qt_library_settings();
        if (ls) {
            delete static_cast<QSettings *>(ls->settings);
            ls->settings = 0;
        }
    }

public:
    static QSettings *configuration()
    {
        QLibrarySettings *ls = qt_library_settings();
        return ls ? static_cast<QSettings *>(qt_library_settings()->settings) : (QSettings*)0;
    }

    Q_GLOBAL_STATIC(QLibrarySettings, qt_library_settings)
};


QLibrarySettings::QLibrarySettings()
{
    settings = QLibraryInfoPrivate::findConfiguration();
#ifndef QT_NO_QOBJECT
    qAddPostRoutine(QLibraryInfoPrivate::cleanup);
#endif
}

QSettings *QLibraryInfoPrivate::findConfiguration()
{
    if(!qt_library_config_file()->isNull())
        return (new QSettings(*qt_library_config_file(), QSettings::IniFormat));

    { //look in the binary itself
        const QString qtconfig = QLatin1String(":/qt/etc/qt.conf");
        if(QFile::exists(qtconfig))
            return (new QSettings(qtconfig, QSettings::IniFormat));
    }
#ifndef QT_NO_QOBJECT
    if(QCoreApplication *app = QCoreApplication::instance()) { //walk up the file system from the exe to (a) root
        if(app->argv() && app->argv()[0]) {
            bool trySearch = true;
            QString exe;
#ifndef Q_OS_WIN
            exe = QString::fromLocal8Bit(app->argv()[0]);
#else
            QT_WA({
                unsigned short module_name[256];
                GetModuleFileNameW(0, reinterpret_cast<wchar_t *>(module_name), sizeof(module_name));
                exe = QString::fromUtf16(module_name);
            }, {
                char module_name[256];
                GetModuleFileNameA(0, module_name, sizeof(module_name));
                exe = QString::fromLocal8Bit(module_name);
            });
#endif // Q_OS_WIN
            if(QDir::isRelativePath(exe)) {
                if((exe.indexOf(QLatin1Char('/')) != -1
#ifdef Q_OS_WIN
                    || exe.indexOf(QLatin1Char('\\')) != -1
#endif
                       ) && QFile::exists(exe)) {
                    exe.prepend(QDir::currentPath() + QLatin1Char('/'));
                } else if(char *path = qgetenv("PATH")) {
#ifdef Q_OS_WIN
                    QStringList paths = QString::fromLocal8Bit(path).split(QLatin1Char(';'));
#else
                    QStringList paths = QString::fromLocal8Bit(path).split(QLatin1Char(':'));
#endif
                    bool found = false;
                    for(int i = 0; i < paths.size(); ++i) {
                        const QString fexe = paths.at(i) + QLatin1Char('/') + exe;
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
                for(int count = 0; count < 64; ++count) {
                    if(pwd.exists(QLatin1String("qt.conf")))
                        return (new QSettings(pwd.filePath(QLatin1String("qt.conf")),
                                 QSettings::IniFormat));
                    if(pwd.isRoot())
                        break;
                    pwd.cdUp();
                }
            }
        }
    }
#endif
    if(char *qtdir = qgetenv("QTDIR")) {     //look in QTDIR
        const QString qtconfig = QString::fromUtf8(qtdir) + QLatin1String("/qt.conf");
        if(QFile::exists(qtconfig))
            return (new QSettings(qtconfig, QSettings::IniFormat));
    }
    if(char *qtconfig_str = qgetenv("QTCONFIG")) {     //look in QTCONFIG
        const QString qtconfig = QString::fromUtf8(qtconfig_str);
        if(QFile::exists(qtconfig))
            return (new QSettings(qtconfig, QSettings::IniFormat));
    }
    { //look in the home dir
        const QString qtconfig = QDir::homePath() + QLatin1String("/.qt.conf");
        if(QFile::exists(qtconfig))
            return (new QSettings(qtconfig, QSettings::IniFormat));
    }
    { //walk up the file system from PWD to (a) root
        QDir pwd = QDir::current();
        for(int count = 0; count < 64; ++count) {
            if(pwd.exists(QLatin1String("qt.conf")))
                return (new QSettings(pwd.filePath(QLatin1String("qt.conf")),
                              QSettings::IniFormat));
            if(pwd.isRoot())
                break;
            pwd.cdUp();
        }
    }
#ifdef Q_OS_UNIX
    { //look in the /etc
        const QString qtconfig = QLatin1String("/etc/qt.conf");
        if(QFile::exists(qtconfig))
            return (new QSettings(qtconfig, QSettings::IniFormat));
    }
    { //look in the /usr/local/etc
        const QString qtconfig = QLatin1String("/usr/local/etc/qt.conf");
        if(QFile::exists(qtconfig))
            return (new QSettings(qtconfig, QSettings::IniFormat));
    }
#endif
#ifdef Q_OS_WIN
    { //registry key
        //###TDB
    }
#endif
    return 0;     //no luck
}
static QLibraryInfoPrivate qt_library_data;

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
    const char *str = QT_CONFIGURE_LICENSEE;
    return QString::fromLocal8Bit(str);
}

/*!
  Returns the products that the license for this build of Qt has access to.

  \sa QLibraryInfo::licensee
*/

QString
QLibraryInfo::licensedProducts()
{
    const char *str = QT_CONFIGURE_LICENSED_PRODUCTS;
    return QString::fromLatin1(str);
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
    return QString::fromLatin1(QT_BUILD_KEY);
}

/*!
  Returns the active configuration information settings. This is
  normally only usefull for debugging but could be usefull to retrieve
  custom information. Do not destruct the return value as QLibrary
  retains ownership. Qt will automatically find the configuration file
  by looking (in order):

  \list

  \i A user argument to your application of -qtconfig
  <config_location>.

  \i A resource of the name :/qt/etc/qt.conf.

  \i A file of the name qt.conf in the directory from which your
  application executable lives. The filesystem will be walked up from
  that directory to the root.

  \i A file of the name qt.conf in the directory from which your
  application executable was run. The filesystem will be walked up
  from that directory to the root.

  \i An environment variable QTCONFIG.

  \i An environment variable QTDIR/qt.conf.

  \i A file in $(HOME)/.qt.conf.

  \i A file in /etc/qt.conf. (Unix only)

  \i A file in /usr/local/etc/qt.conf. (Unix only)

  \endlist

  If no configuration can be found then zero will be returned.

  \sa QLibrayInfo::location, QSettings
*/

QSettings
*QLibraryInfo::configuration()
{
    return qt_library_data.configuration();
}

/*!
  Returns the location specified by \a loc.

  \sa QLibraryInfo::LibraryLocation, QLibraryInfo::configuration
*/

QString
QLibraryInfo::location(LibraryLocation loc)
{
    if(!QLibraryInfoPrivate::configuration())
        return QString();

    QString key;
    switch(loc) {
    case PrefixPath:
        key = QLatin1String("Prefix");
        break;
    case DocumentationPath:
        key = QLatin1String("Documentation");
        break;
    case HeadersPath:
        key = QLatin1String("Headers");
        break;
    case LibrariesPath:
        key = QLatin1String("Libraries");
        break;
    case BinariesPath:
        key = QLatin1String("Binaries");
        break;
    case PluginsPath:
        key = QLatin1String("Plugins");
        break;
    case DataPath:
        key = QLatin1String("Data");
        break;
    case TranslationsPath:
        key = QLatin1String("Translations");
        break;
    case SettingsPath:
        key = QLatin1String("Settings");
        break;
    default:
        break;
    }

    QString ret;
    if(!key.isNull()) {
        QSettings *config = QLibraryInfoPrivate::configuration();
        config->beginGroup(QLatin1String("Paths"));

        QString subKey;
        {
            int maj = 0, min = 0, pat = 0;
            QStringList children = config->childGroups();
            for(int child = 0; child < children.size(); ++child) {
                QString cver = children.at(child);
                QStringList cver_list = cver.split(QLatin1Char('.'));
                if(cver_list.size() > 0 && cver_list.size() < 4) {
                    bool ok;
                    int cmaj = -1, cmin = -1, cpat = -1;
                    cmaj = cver_list[0].toInt(&ok);
                    if(!ok || cmaj < 0)
                        continue;
                    if(cver_list.size() >= 2) {
                        cmin = cver_list[1].toInt(&ok);
                        if(!ok)
                            continue;
                        if(cmin < 0)
                            cmin = -1;
                    }
                    if(cver_list.size() >= 3) {
                        cpat = cver_list[2].toInt(&ok);
                        if(!ok)
                            continue;
                        if(cpat < 0)
                            cpat = -1;
                    }
                    if((cmaj >= maj && cmaj <= ((QT_VERSION >> 16) & 0xFF)) &&
                       (cmin == -1 || (cmin >= min && cmin <= ((QT_VERSION >> 8) & 0xFF))) &&
                       (cpat == -1 || (cpat >= pat && cpat <= (QT_VERSION & 0xFF))) &&
                       config->contains(cver + QLatin1Char('/') + key)) {
                        subKey = cver + QLatin1Char('/');
                        maj = cmaj;
                        min = cmin;
                        pat = cpat;
                    }
                }
            }
        }
        if(config->contains(subKey + key)) {
            ret = config->value(subKey + key).toString();
            int rep;
            QRegExp reg_var(QLatin1String("\\$\\(.*\\)"));
            reg_var.setMinimal(true);
            while((rep = reg_var.indexIn(ret)) != -1)
                ret.replace(rep, reg_var.matchedLength(),
                               QString::fromLocal8Bit(qgetenv(ret.mid(rep + 2,
                                           reg_var.matchedLength() - 3).toLatin1().constData())));
        } else {
            ret = QDir::currentPath();
        }
        config->endGroup();
    }
    return ret;
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
