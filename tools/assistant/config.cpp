/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "config.h"
#include "profile.h"
#include "docuparser.h"

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qfont.h>
#include <qfontinfo.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qxml.h>
#include <qlist.h>

static Config *static_configuration = 0;

inline QString getVersionString()
{
    return QString::number( (QT_VERSION >> 16) & 0xff )
        + QLatin1String(".") + QString::number( (QT_VERSION >> 8) & 0xff );
}

Config::Config()
    : profil( 0 ), fontSiz(-1), maximized(false), hideSidebar( false )
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
        config->profil = Profile::createDefaultProfile();
        config->load();
        config->loadDefaultProfile();
        return config;
    }

    QFile file(profileFileName);
    if (!file.exists()) {
        qWarning( (QLatin1String("File does not exist: ") + profileFileName).ascii() );
        return 0;
    }
    DocuParser *parser = DocuParser::createParser( profileFileName );
    if (!parser) {
        qWarning( (QLatin1String("Failed to create parser for file: ") + profileFileName).ascii() );
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
        qWarning( (QLatin1String("Config::loadConfig(), no profile in: ") + profileFileName).ascii() );
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
    const QString key = QLatin1String("/Qt Assistant/") + getVersionString() + QLatin1String("/");
    const QString profkey = key + QLatin1String("Profile/") + profil->props[QLatin1String("name")] + QLatin1String("/");

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, QLatin1String("/Trolltech") );

    webBrows = settings.readEntry( key + QLatin1String("Webbrowser") );
    home = settings.readEntry( profkey + QLatin1String("Homepage") );
    pdfApp = settings.readEntry( key + QLatin1String("PDFApplication") );
    linkUnder = settings.readBoolEntry( key + QLatin1String("LinkUnderline"), true );
    linkCol = settings.readEntry( key + QLatin1String("LinkColor"), QLatin1String("#0000FF") );
    src = settings.readListEntry( profkey + QLatin1String("Source") );
    sideBar = settings.readNumEntry( key + QLatin1String("SideBarPage") );
    if (qApp->type() != QApplication::Tty) {
        fontFam = settings.readEntry( key + QLatin1String("Family"), qApp->font().family() );

        fontFix = settings.readEntry( key + QLatin1String("FixedFamily"), QLatin1String("courier") );
        fontSiz = settings.readNumEntry( key + QLatin1String("Size"), -1 );
        if ( fontSiz == -1 ) {
            QFontInfo fi( qApp->font() );
            fontSiz = fi.pointSize();
        }
        geom.setRect( settings.readNumEntry( key + QLatin1String("GeometryX"), QApplication::desktop()->availableGeometry().x() ),
                      settings.readNumEntry( key + QLatin1String("GeometryY"), QApplication::desktop()->availableGeometry().y() ),
                      settings.readNumEntry( key + QLatin1String("GeometryWidth"), 800 ),
                      settings.readNumEntry( key + QLatin1String("GeometryHeight"), 600 ) );
        maximized = settings.readBoolEntry( key + QLatin1String("GeometryMaximized"), false );
    }
    mainWinLayout = settings.readEntry( key + QLatin1String("MainwindowLayout") );
    rebuildDocs = settings.readBoolEntry( key + QLatin1String("RebuildDocDB"), true );

    profileNames = settings.entryList( key + QLatin1String("Profile") );
}

void Config::save()
{
    saveSettings();
    saveProfile( profil );
}

void Config::saveSettings()
{
    const QString key = QLatin1String("/Qt Assistant/") + getVersionString() + QLatin1String("/");
    const QString profkey = key + QLatin1String("Profile/") + profil->props[QLatin1String("name")] + QLatin1String("/");

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, QLatin1String("/Trolltech"));

    settings.writeEntry( key + QLatin1String("Webbrowser"), webBrows );
    settings.writeEntry( profkey + QLatin1String("Homepage"), home );
    settings.writeEntry( key + QLatin1String("PDFApplication"), pdfApp );
    settings.writeEntry( key + QLatin1String("LinkUnderline"), linkUnder );
    settings.writeEntry( key + QLatin1String("LinkColor"), linkCol );
    settings.writeEntry( profkey + QLatin1String("Source"), src );
    settings.writeEntry( key + QLatin1String("SideBarPage"), sideBarPage() );
    if (qApp->type() != QApplication::Tty) {
        settings.writeEntry( key + QLatin1String("GeometryX"), geom.x() );
        settings.writeEntry( key + QLatin1String("GeometryY"), geom.y() );
        settings.writeEntry( key + QLatin1String("GeometryWidth"), geom.width() );
        settings.writeEntry( key + QLatin1String("GeometryHeight"), geom.height() );
        settings.writeEntry( key + QLatin1String("GeometryMaximized"), maximized );
        settings.writeEntry( key + QLatin1String("Family"),  fontFam );
        settings.writeEntry( key + QLatin1String("Size"),  fontSiz );
        settings.writeEntry( key + QLatin1String("FixedFamily"), fontFix );
    }
    if ( !hideSidebar )
        settings.writeEntry( key + QLatin1String("MainwindowLayout"), mainWinLayout );
    settings.writeEntry( key + QLatin1String("RebuildDocDB"), rebuildDocs );
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

void Config::loadDefaultProfile()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, QLatin1String("/Trolltech"));
    const QString key = QLatin1String("/Qt Assistant/") + QLatin1String(QT_VERSION_STR) + QLatin1String("/Profile");
    const QString profKey = key + QLatin1String("/default/");

    if( settings.entryList( key + QLatin1String("/default")).count() == 0 ) {
        return;
    }

    // Override the defaults with settings in registry.
    profil->icons.clear();
    profil->indexPages.clear();
    profil->imageDirs.clear();
    profil->docs.clear();
    profil->dcfTitles.clear();

    QStringList titles = settings.readListEntry( profKey + QLatin1String("Titles") );
    QStringList iconLst = settings.readListEntry( profKey + QLatin1String("DocIcons") );
    QStringList indexLst = settings.readListEntry( profKey + QLatin1String("IndexPages") );
    QStringList imgDirLst = settings.readListEntry( profKey + QLatin1String("ImageDirs") );
    QStringList dcfs = settings.readListEntry( profKey + QLatin1String("DocFiles") );

    QStringList::ConstIterator it = titles.begin();
    QStringList::ConstIterator iconIt = iconLst.begin();
    QStringList::ConstIterator indexIt = indexLst.begin();
    QStringList::ConstIterator imageIt = imgDirLst.begin();
    QStringList::ConstIterator dcfIt = dcfs.begin();
    for( ; it != titles.end();
        ++it, ++iconIt, ++indexIt, ++imageIt, ++dcfIt )
    {
        profil->addDCFIcon( *it, *iconIt );
        profil->addDCFIndexPage( *it, *indexIt );
        profil->addDCFImageDir( *it, *imageIt );
        profil->addDCFTitle( *dcfIt, *it );
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
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, QLatin1String("/Trolltech") );
    QString versionString = (profile->props[QLatin1String("name")] == QLatin1String("default"))
        ? QLatin1String(QT_VERSION_STR)
        : getVersionString();
    const QString key = QLatin1String("/Qt Assistant/") + versionString + QLatin1String("/");
    const QString profKey = key + QLatin1String("Profile/") + profile->props[QLatin1String("name")] + QLatin1String("/");

    QStringList indexes, icons, imgDirs, dcfs;
    QStringList titles = profile->dcfTitles.keys();
    QStringList::ConstIterator it = titles.begin();
    for ( ; it != titles.end(); ++it ) {
        indexes << profile->indexPages[*it];
        icons << profile->icons[*it];
        imgDirs << profile->imageDirs[*it];
        dcfs << profile->dcfTitles[*it];
    }

    settings.writeEntry( profKey + QLatin1String("Titles"), titles );
    settings.writeEntry( profKey + QLatin1String("DocFiles"), dcfs );
    settings.writeEntry( profKey + QLatin1String("IndexPages"), indexes );
    settings.writeEntry( profKey + QLatin1String("DocIcons"), icons );
    settings.writeEntry( profKey + QLatin1String("ImageDirs"), imgDirs );

#if ASSISTANT_DEBUG
    qDebug( "Titles:\n  - " + ( (QStringList*) &titles )->join( "\n  - " ) );
    qDebug( "Docfiles:\n  - " + dcfs.join( "\n  - " ) );
    qDebug( "IndexPages:\n  - " + indexes.join( "\n  - " ) );
    qDebug( "DocIcons:\n  - " + icons.join( "\n  - " ) );
    qDebug( "ImageDirs:\n  - " + imgDirs.join( "\n  - " ) );
#endif
}

QStringList Config::mimePaths()
{
    static QStringList lst;

    if( lst.count() > 0 )
        return lst;

    for (QMap<QString,QString>::ConstIterator it = profil->dcfTitles.begin();
         it != profil->dcfTitles.end(); ++it ) {

        // Mime source for .dcf file path
        QFileInfo info( *it );
        QString dcfPath = info.absolutePath();
        if (lst.contains(dcfPath) == 0)
            lst << dcfPath;

        // Image dir for .dcf
        QString imgDir = QDir::convertSeparators( dcfPath + QDir::separator()
                                                  + profil->imageDirs[it.key()] );
        if (lst.contains(imgDir) == 0)
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
    return profil->props[QLatin1String("title")];
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

    return QPixmap(name);
}

QPixmap Config::applicationIcon() const
{
    QString name = profil->props[QLatin1String("applicationicon")];
    QString resName = QLatin1String(":/trolltech/assistant/images/") + name;

    if (QFile::exists(resName))
        return QPixmap(resName);

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
    return profil->indexPages
        [title];
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
        ? QFile::decodeName( qInstallPathDocs() ) + QLatin1String("/html")
        : profil->props[QLatin1String("assistantdocs")];
}
