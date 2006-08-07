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

#include "config.h"
#include "profile.h"
#include "docuparser.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QFont>
#include <QFontInfo>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QtXml>
#include <QList>

static Config *static_configuration = 0;

inline QString getVersionString()
{
    return QString::number( (QT_VERSION >> 16) & 0xff )
        + QLatin1String(".") + QString::number( (QT_VERSION >> 8) & 0xff );
}

Config::Config()
    : profil( 0 ), hideSidebar( false ), rebuildDocs(true)
{
    if( !static_configuration ) {
        static_configuration = this;
    } else {
        qWarning( "Multiple configurations not allowed!" );
    }
}

Config *Config::loadConfig(const QString &profileFileName)
{
    Config *config = new Config();

    if (profileFileName.isEmpty()) { // no profile
        if (!config->defaultProfileExists()) {
            config->profil = Profile::createDefaultProfile();
            config->saveProfile(config->profil);
        } else {
            config->profil = new Profile();
        }
        config->loadDefaultProfile();
        config->load();        
        return config;
    }

    QFile file(profileFileName);
    if (!file.exists()) {
        qWarning( (QLatin1String("File does not exist: ") + profileFileName).toAscii().constData() );
        return 0;
    }
    DocuParser *parser = DocuParser::createParser( profileFileName );
    if (!parser) {
        qWarning( (QLatin1String("Failed to create parser for file: ") + profileFileName).toAscii().constData() );
        return 0;
    }
    if (parser->parserVersion() < DocuParser::Qt320) {
        qWarning( "File does not contain profile information" );
        return 0;
    }
    DocuParser320 *profileParser = static_cast<DocuParser320*>(parser);
    parser->parse(&file);
    config->profil = profileParser->profile();
    if (!config->profil) {
        qWarning( (QLatin1String("Config::loadConfig(), no profile in: ") + profileFileName).toAscii().constData() );
        return 0;
    }
    config->profil->setProfileType(Profile::UserProfile);
    config->profil->setDocuParser(profileParser);
    config->load();
    return config;
}

Config *Config::configuration()
{
    Q_ASSERT( static_configuration );
    return static_configuration;
}

void Config::load()
{
    const QString key = getVersionString() + QLatin1String("/");

    const QString pKey = (profil->props[QLatin1String("name")] == QLatin1String("default"))
        ? QString::fromLatin1(QT_VERSION_STR)
        : getVersionString();

    const QString profkey = pKey + QLatin1String("/Profile/") + profil->props[QLatin1String("name")] + QLatin1String("/");

    QSettings settings;

    home = settings.value( profkey + QLatin1String("Homepage") ).toString();
    src = settings.value( profkey + QLatin1String("Source") ).toStringList();
    sideBar = settings.value( key + QLatin1String("SideBarPage") ).toInt();
    if (qApp->type() != QApplication::Tty)
        winGeometry = settings.value(key + QLatin1String("windowGeometry")).toByteArray();

    mainWinState = settings.value(key + QLatin1String("MainWindowState")).toByteArray();
    pointFntSize = settings.value(key + QLatin1String("FontSize"), qApp->font().pointSizeF()).toDouble();
    rebuildDocs = settings.value( key + QLatin1String("RebuildDocDB"), true ).toBool();

    profileNames = settings.value( key + QLatin1String("Profile") ).toStringList();
}

void Config::save()
{
    saveSettings();
    saveProfile( profil );
}

void Config::saveSettings()
{
    const QString key = getVersionString() + QLatin1String("/");

    const QString pKey = (profil->props[QLatin1String("name")] == QLatin1String("default"))
        ? QString::fromLatin1(QT_VERSION_STR)
        : getVersionString();

    const QString profkey = pKey + QLatin1String("/Profile/") + profil->props[QLatin1String("name")] + QLatin1String("/");

    QSettings settings;

    settings.setValue( profkey + QLatin1String("Homepage"), home );
    settings.setValue( profkey + QLatin1String("Source"), src );
    settings.setValue( key + QLatin1String("SideBarPage"), sideBarPage() );
    if (qApp->type() != QApplication::Tty)
        settings.setValue(key + QLatin1String("windowGeometry"), winGeometry);

    settings.setValue( key + QLatin1String("MainWindowState"), mainWinState );
    settings.setValue( key + QLatin1String("FontSize"), pointFntSize);
    settings.setValue( key + QLatin1String("RebuildDocDB"), rebuildDocs );
}

#ifdef ASSISTANT_DEBUG
static void dumpmap( const QMap<QString,QString> &m, const QString &header )
{
    qDebug( header );
    QMap<QString,QString>::ConstIterator it = m.begin();
    while (it != m.end()) {
        qDebug( "  " + it.key() + ":\t\t" + *it );
        ++it;
    }
}
#endif

bool Config::defaultProfileExists()
{
    QSettings settings;
    const QString profKey = QLatin1String(QT_VERSION_STR) + QLatin1String("/Profile/default/");

    if (settings.contains(profKey + QLatin1String("DocFiles"))
        && settings.contains(profKey + QLatin1String("Titles"))
        && settings.contains(profKey + QLatin1String("ImageDirs"))) {        
        QStringList dcfs = settings.value(profKey + QLatin1String("DocFiles") ).toStringList();
        foreach (QString file, dcfs) {
            if (file == Profile::storableFilePath(file))
                return true;
        }
    }
    return false;
}

void Config::loadDefaultProfile()
{
    QSettings settings;
    const QString profKey = QLatin1String(QT_VERSION_STR) + QLatin1String("/Profile/default/");

    if (!defaultProfileExists())
        return;

    // Override the defaults with settings in registry.
    profil->icons.clear();
    profil->indexPages.clear();
    profil->imageDirs.clear();
    profil->docs.clear();
    profil->dcfTitles.clear();
    
    QStringList titles = settings.value( profKey + QLatin1String("Titles") ).toStringList();
    QStringList iconLst = settings.value( profKey + QLatin1String("DocIcons") ).toStringList();
    QStringList indexLst = settings.value( profKey + QLatin1String("IndexPages") ).toStringList();
    QStringList imgDirLst = settings.value( profKey + QLatin1String("ImageDirs") ).toStringList();
    QStringList dcfs = settings.value( profKey + QLatin1String("DocFiles") ).toStringList();
    profil->props[QLatin1String("name")] = QLatin1String("default");
    
    QString filePath;
    QStringList::ConstIterator it = titles.constBegin();
    QStringList::ConstIterator iconIt = iconLst.constBegin();
    QStringList::ConstIterator indexIt = indexLst.constBegin();
    QStringList::ConstIterator imageIt = imgDirLst.constBegin();
    QStringList::ConstIterator dcfIt = dcfs.constBegin();
    while((it != titles.constEnd())
          && (iconIt != iconLst.constEnd())
          && (indexIt != indexLst.constEnd())
          && (imageIt != imgDirLst.constEnd())
          && (dcfIt != dcfs.constEnd())) {
        profil->addDCFIcon( *it, *iconIt );        
        profil->addDCFIndexPage(*it, Profile::loadableFilePath(*indexIt));        
        profil->addDCFImageDir( *it, *imageIt );
        profil->addDCFTitle(Profile::loadableFilePath(*dcfIt), *it);
        ++it, ++iconIt, ++indexIt, ++imageIt, ++dcfIt;
    }
#if ASSISTANT_DEBUG
    dumpmap( profil->icons, QLatin1String("Icons") );
    dumpmap( profil->indexPages, QLatin1String("IndexPages") );
    dumpmap( profil->imageDirs, QLatin1String("ImageDirs") );
    dumpmap( profil->dcfTitles, QLatin1String("dcfTitles") );
    qDebug( "Docfiles: \n  " + profil->docs.join( "\n  " ) );
#endif
}

void Config::saveProfile( Profile *profile )
{
    if (profil->profileType() == Profile::UserProfile)
        return;
    
    const QString key = (profile->props[QLatin1String("name")] == QLatin1String("default"))
        ? QString::fromLatin1(QT_VERSION_STR)
        : getVersionString();

    const QString profKey = key + QLatin1String("/Profile/") + profile->props[QLatin1String("name")] + QLatin1String("/");

    QString path = QLibraryInfo::location(QLibraryInfo::DocumentationPath).replace("\\", "/");
    QStringList indexes, icons, imgDirs, dcfs;
    QStringList titles = profile->dcfTitles.keys();
    QStringList::ConstIterator it = titles.constBegin();
    QString filePath;
    for ( ; it != titles.constEnd(); ++it ) {
        
        indexes << Profile::storableFilePath(profile->indexPages[*it]);
        icons << profile->icons[*it];
        imgDirs << profile->imageDirs[*it];
        dcfs << Profile::storableFilePath(profile->dcfTitles[*it]);
    }

    QSettings settings;
    settings.setValue( profKey + QLatin1String("Titles"), titles );
    settings.setValue( profKey + QLatin1String("DocFiles"), dcfs );
    settings.setValue( profKey + QLatin1String("IndexPages"), indexes );
    settings.setValue( profKey + QLatin1String("DocIcons"), icons );
    settings.setValue( profKey + QLatin1String("ImageDirs"), imgDirs );
    
#if ASSISTANT_DEBUG
    qDebug() << "Titles:\n  - " << ((QStringList*)&titles)->join("\n  - ");
    qDebug() << "Docfiles:\n  - " << dcfs.join("\n  - " );
    qDebug() << "IndexPages:\n  - " << indexes.join("\n  - ");
    qDebug() << "DocIcons:\n  - " << icons.join("\n  - " );
    qDebug() << "ImageDirs:\n  - " << imgDirs.join("\n  - " );
#endif
}

QStringList Config::mimePaths()
{
    static QStringList lst;

    if( lst.count() > 0 )
        return lst;

    for (QMap<QString,QString>::ConstIterator it = profil->dcfTitles.constBegin();
         it != profil->dcfTitles.constEnd(); ++it ) {

        // Mime source for .dcf file path
        QFileInfo info( *it );
        QString dcfPath = info.absolutePath();
        if (!lst.contains(dcfPath))
            lst << dcfPath;

        // Image dir for .dcf
        QString imgDir = QDir::convertSeparators( dcfPath + QDir::separator()
                                                  + profil->imageDirs[it.key()] );
        if (!lst.contains(imgDir))
            lst << imgDir;
    }
    return lst;
}

QStringList Config::profiles() const
{
    return profileNames;
}

QString Config::title() const
{
    QString s = profil->props[QLatin1String("title")];
    if (s.isEmpty())
        s = QObject::tr("Qt Assistant by Trolltech");
    return s;
}

QString Config::aboutApplicationMenuText() const
{
    return profil->props[QLatin1String("aboutmenutext")];
}

QString Config::aboutURL() const
{
    return profil->props[QLatin1String("abouturl")];
}

QString Config::homePage() const
{
    return home.isEmpty() ? profil->props[QLatin1String("startpage")] : home;
}

QStringList Config::source() const
{
    return src.size() == 0 ? QStringList(profil->props[QLatin1String("startpage")]) : src;
}

QStringList Config::docFiles() const
{
    return profil->docs;
}

QPixmap Config::docIcon( const QString &title ) const
{
    // ### To allow qdoc generated dcf files to reference the doc icons from qmake_image_col
    QString name = profil->icons[title];
    QString resName = QLatin1String(":/trolltech/assistant/images/") + name;

    if (QFile::exists(resName))
        return QPixmap(resName);

    if (name.startsWith("file:"))
        name = name.mid(5);
    return QPixmap(name);
}

QPixmap Config::applicationIcon() const
{
    QString name = profil->props[QLatin1String("applicationicon")];
    QString resName = QLatin1String(":/trolltech/assistant/images/") + name;

    if (QFile::exists(resName))
        return QPixmap(resName);

    if (name.startsWith("file:"))
        name = name.mid(5);
    return QPixmap(name);
}

QStringList Config::docTitles() const
{
    return QStringList(profil->indexPages.keys());
}

QString Config::docImageDir( const QString &docfile ) const
{
    return profil->imageDirs[docfile];
}

QString Config::indexPage( const QString &title ) const
{
    return profil->indexPages[title];
}

void Config::hideSideBar( bool b )
{
    hideSidebar = b;
}

bool Config::sideBarHidden() const
{
    return hideSidebar;
}

QString Config::assistantDocPath() const
{
    return profil->props[QLatin1String("assistantdocs")].isEmpty()
        ? QLibraryInfo::location(QLibraryInfo::DocumentationPath) + QLatin1String("/html")
        : profil->props[QLatin1String("assistantdocs")];
}
