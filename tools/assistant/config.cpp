#include "config.h"
#include "profile.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qsettings.h>
#include <qxml.h>

static Config *static_configuration = 0;

Config::Config( const QString &name )
    : startWithProfile( FALSE )
{
    if( !static_configuration ) {
	static_configuration = this;
    } else {
	qWarning( "Can only have one instance of Config at a time!" );
    }

    load( name );
}

Config::Config()
    : startWithProfile( FALSE )
{
    profil = new Profile();
}

bool Config::addProfile( const QString &profileFileName, const QString &path )
{
    Config *config = new Config();
    if ( !config->profil->load( profileFileName ) )
	return FALSE;
    QString docPath = path;
    QFileInfo fi( profileFileName );
    if ( docPath.isEmpty() )
	docPath = fi.dirPath( TRUE );
    QValueListIterator<QString> it = config->profil->docs.begin();
    for ( ; it != config->profil->docs.end(); ++it ) {
	QFileInfo dfi( *it );
	(*it) = docPath + "/" + dfi.fileName();
    }
    config->saveProfile( config->profil, TRUE );
    return TRUE;
}

Config *Config::configuration()
{
    Q_ASSERT( static_configuration );
    return static_configuration;
}

void Config::load( const QString &name )
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QString profName = name;
    if ( profName.isEmpty() )
	profName = settings.readEntry( key + "LastProfile", "" );
    else
	startWithProfile = TRUE;
    if ( profName.isEmpty() ) {
	profil = Profile::createDefaultProfile();
	saveProfile( profil, TRUE );
    } else {
	profil = loadProfile( profName );
    }

    const QString profkey = key + profil->props["name"] + "/";

    webBrows = settings.readEntry( key + "Webbrowser" );
    home = settings.readEntry( profkey + "Homepage" );
    pdfApp = settings.readEntry( key + "PDFApplication" );
    fontFam = settings.readEntry( key + "Family", qApp->font().family() );

    fontFix = settings.readEntry( key + "FixedFamily", "courier" );
    fontSiz = settings.readNumEntry( key + "Size", qApp->font().pointSize() );
    linkUnder = settings.readBoolEntry( key + "LinkUnderline", TRUE );
    linkCol = settings.readEntry( key + "LinkColor", "#0000FF" );
    src = settings.readEntry( profkey + "Source", "assistant_about_text" );
    sideBar = settings.readNumEntry( key + "SideBarPage" );
    geom.setRect( settings.readNumEntry( key + "GeometryX", 0 ),
		  settings.readNumEntry( key + "GeometryY", 0 ),
		  settings.readNumEntry( key + "GeometryWidth", -1 ),
		  settings.readNumEntry( key + "GeometryHeight", -1 ) );
    maximized = settings.readBoolEntry( key + "GeometryMaximized", FALSE );
    mainWinLayout = settings.readEntry( key + "MainwindowLayout" );

    profDiffer = ( settings.readEntry( key + "LastProfile" ) != profil->props["name"] );
    profileNames = settings.entryList( key + "Profile" );
}

void Config::reloadProfiles()
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    profileNames = settings.entryList( key + "Profile" );
}

void Config::save()
{
    saveSettings();
    saveProfile( profil );
}

void Config::saveSettings()
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    const QString profkey = key + profil->props["name"] + "/";
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    settings.writeEntry( key + "Webbrowser", webBrows );
    settings.writeEntry( profkey + "Homepage", home );
    settings.writeEntry( key + "PDFApplication", pdfApp );
    settings.writeEntry( key + "Family",  fontFam );
    settings.writeEntry( key + "Size",  fontSiz );
    settings.writeEntry( key + "FixedFamily", fontFix );
    settings.writeEntry( key + "LinkUnderline", linkUnder );
    settings.writeEntry( key + "LinkColor", linkCol );
    settings.writeEntry( profkey + "Source", src );
    settings.writeEntry( key + "SideBarPage", sideBarPage() );
    settings.writeEntry( key + "GeometryX", geom.x() );
    settings.writeEntry( key + "GeometryY", geom.y() );
    settings.writeEntry( key + "GeometryWidth", geom.width() );
    settings.writeEntry( key + "GeometryHeight", geom.height() );
    settings.writeEntry( key + "GeometryMaximized", maximized );
    settings.writeEntry( key + "MainwindowLayout", mainWinLayout );
    settings.writeEntry( key + "LastProfile", profil->props["name"] );
}

Profile* Config::loadProfile( const QString &name )
{
    Profile *profile = new Profile();

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/Profile";
    const QString profKey = key + "/" + name + "/";

    QStringList profileLst = settings.entryList( key );

    bool found = FALSE;
    QValueListConstIterator<QString> it = profileLst.begin();
    for ( ; it != profileLst.end(); ++it ) {
	if ( *it == name ) {
	    found = TRUE;
	    break;
	}
    }
    if ( !found )
	return 0;

    profile->props["name"] = name;
    profile->changed = settings.readBoolEntry( profKey );
    profile->props["applicationicon"] = settings.readEntry( profKey + "AppIcon" );
    profile->props["aboutmenutext"] = settings.readEntry( profKey + "AboutMenuText" );
    profile->props["abouturl"] = settings.readEntry( profKey + "AboutUrl" );
    profile->props["title"] = settings.readEntry( profKey + "Title" );
    profile->props["docbasepath"] = settings.readEntry( profKey + "BasePath" );
    profile->docs = settings.readListEntry( profKey + "DocFiles" );
    QStringList iconLst = settings.readListEntry( profKey + "DocIcons" );
    QStringList titleLst = settings.readListEntry( profKey + "DocTitles" );
    QStringList imgDirLst = settings.readListEntry( profKey + "ImageDirs" );

    it = profile->docs.begin();
    QValueListConstIterator<QString> iconIt = iconLst.begin();
    QValueListConstIterator<QString> titleIt = titleLst.begin();
    QValueListConstIterator<QString> imageIt = imgDirLst.begin();
    for( ; it != profile->docs.end() && iconIt != iconLst.end()
	&& titleIt != titleLst.end() && imageIt != imgDirLst.end();
	++it, ++iconIt, ++titleIt, ++imageIt )
    {
	profile->icons[*it] = *iconIt;
	profile->titles[*it] = *titleIt;
	profile->imageDirs[*it] = *imageIt;
    }

    return profile;
}

void Config::saveProfile( Profile *profile, bool changed )
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    const QString profKey = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/" + "Profile/" + profile->props["name"];
    settings.writeEntry( profKey, changed );
    settings.writeEntry( profKey + "/AppIcon", profile->props["applicationicon"] );
    settings.writeEntry( profKey + "/AboutMenuText", profile->props["aboutmenutext"] );
    settings.writeEntry( profKey + "/AboutUrl", profile->props["abouturl"] );
    settings.writeEntry( profKey + "/Title", profile->props["title"] );
    settings.writeEntry( profKey + "/BasePath", profile->props["docbasepath"] );

    QStringList titles, icons, imgDirs;
    QValueListConstIterator<QString> it = profile->docs.begin();
    for ( ; it != profile->docs.end(); ++it ) {
	titles << profile->titles[*it];
	icons << profile->icons[*it];
	imgDirs << profile->imageDirs[*it];
    }
    settings.writeEntry( profKey + "/DocFiles", profile->docs );
    settings.writeEntry( profKey + "/DocTitles", titles );
    settings.writeEntry( profKey + "/DocIcons", icons );
    settings.writeEntry( profKey + "/ImageDirs", imgDirs );
}

void Config::removeProfile( const QString &name )
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    const QString profKey = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/" + "Profile/" + name;
    settings.removeEntry( profKey );
    settings.removeEntry( profKey + "/AppIcon" );
    settings.removeEntry( profKey + "/AboutMenuText" );
    settings.removeEntry( profKey + "/AboutUrl" );
    settings.removeEntry( profKey + "/Title" );
    settings.removeEntry( profKey + "/BasePath" );
    settings.removeEntry( profKey + "/DocFiles" );
    settings.removeEntry( profKey + "/DocTitles" );
    settings.removeEntry( profKey + "/DocIcons" );
    settings.removeEntry( profKey + "/ImageDirs" );
}

bool Config::setCurrentProfile( const QString &name )
{
    Profile *oldProf = profil;
    profil = loadProfile( name );
    if ( !profil ) {
	profil = oldProf;
	return FALSE;
    }
    delete oldProf;
    oldProf = 0;
    return TRUE;
}

QStringList Config::profiles() const
{
    return profileNames;
}

QString Config::title() const
{
    return profil->props[ "title" ];
}

QString Config::aboutApplicationMenuText() const
{
    return profil->props[ "aboutmenutext" ];
}


QString Config::aboutURL() const
{
    return profil->props[ "abouturl" ];
}

QStringList Config::docFiles() const
{
    return profil->docs;
}

QPixmap Config::docIcon( const QString &docfile ) const
{
    return QPixmap::fromMimeSource( profil->icons[docfile] );
}

QPixmap Config::applicationIcon() const
{
    return QPixmap::fromMimeSource( profil->props["applicationicon"] );
}

QString Config::docTitle( const QString &docfile ) const
{
    return profil->titles[docfile];
}

QString Config::docImageDir( const QString &docfile ) const
{
    return profil->imageDirs[docfile];
}

QString Config::docContentsURL( const QString & /*docfile*/ ) const
{
    //return parser( docfile )->contentsURL();
    // makes this sense? it should be good enough to
    // assume it would be index.htm*
    return QString( "index.html" );
}

QString Config::docBasePath() const
{
    return profil->props["docbasepath"];
}

bool Config::needsNewDoc() const
{
    return profil->changed;
}

bool Config::startedWithProfile() const
{
    return startWithProfile;
}
