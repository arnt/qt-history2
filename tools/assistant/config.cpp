/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qsettings.h>
#include <qxml.h>
#include <qvaluelist.h>

static Config *static_configuration = 0;

Config::Config( const QString &name )
    : startWithProfile( FALSE ), profileNameValid( TRUE ), hideSidebar( FALSE )
{
    if( !static_configuration ) {
	static_configuration = this;
    } else {
	qWarning( "Can only have one instance of Config at a time!" );
    }

    load( name );
}

Config::Config()
    : startWithProfile( FALSE ), profileNameValid( TRUE ), hideSidebar( FALSE )
{
    profil = new Profile();
}

bool Config::addProfile( const QString &profileFileName, const QString &path )
{
    Config *config = new Config();
    if ( profileFileName == "default" ) {
	config->profil = Profile::createDefaultProfile();
	config->saveProfile( config->profil, TRUE );
	return TRUE;
    }
    if ( !config->profil->load( profileFileName ) )
	return FALSE;
    Profile *p = config->profil;
    QString docPath = path;
    if ( docPath.isEmpty() )
	docPath = p->props["basepath"];
    QFileInfo fi( profileFileName );
    if ( docPath.isEmpty() )
	docPath = fi.dirPath( TRUE );

    QStringList lst;
    lst << "applicationicon" << "abouturl" << "startpage";
    QStringList::ConstIterator pit = lst.begin();
    for ( ; pit != lst.end(); ++pit ) {
	if ( !p->props[*pit].isEmpty() ) {
	    QFileInfo fi( p->props[*pit] );
	    if ( fi.isRelative() ) {
		QDir d( docPath + "/" + fi.dirPath() );
		p->props[*pit] = d.absPath() + "/" + fi.fileName();
	    }
	}
    }

    QValueListIterator<QString> it = p->docs.begin();
    for ( ; it != p->docs.end(); ++it ) {
	QFileInfo dfi( *it );
	QString icon = p->icons[*it];
	QString dir = p->imageDirs[*it];
	QString title = p->titles[*it];
	p->icons.erase( *it );
	p->imageDirs.erase( *it );
	p->titles.erase( *it );
	QDir d( docPath + "/" + dfi.dirPath() );
	(*it) = d.absPath() + "/" + dfi.fileName();
	if ( icon.isEmpty() )
	    p->icons[*it] = icon;
	else
	    p->icons[*it] = d.absPath() + "/" + icon;
	p->imageDirs[*it] = dir;
	p->titles[*it] = title;
    }
    config->saveProfile( p, TRUE );
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
    if ( profName.isEmpty() ) {
	profName = settings.readEntry( key + "LastProfile", "" );
    } else {
	settings.readBoolEntry( key + "Profile/" + profName, FALSE, &profileNameValid );
	if ( !profileNameValid )
	    return;
	startWithProfile = TRUE;
    }

    if ( profName.isEmpty() ) {
	profil = Profile::createDefaultProfile();
	saveProfile( profil, TRUE );
    } else {
	profil = loadProfile( profName );
	if ( !profil ) {
	    profil = Profile::createDefaultProfile();
	    saveProfile( profil, TRUE );
	}
    }

    const QString profkey = key + "Profile/" + profName + "/";

    webBrows = settings.readEntry( key + "Webbrowser" );
    home = settings.readEntry( profkey + "Homepage" );
    pdfApp = settings.readEntry( key + "PDFApplication" );
    fontFam = settings.readEntry( key + "Family", qApp->font().family() );

    fontFix = settings.readEntry( key + "FixedFamily", "courier" );
    fontSiz = settings.readNumEntry( key + "Size", -1 );
    if ( fontSiz == -1 ) {
	QFontInfo fi( qApp->font() );
	fontSiz = fi.pointSize();
    }
    linkUnder = settings.readBoolEntry( key + "LinkUnderline", TRUE );
    linkCol = settings.readEntry( key + "LinkColor", "#0000FF" );
    src = settings.readEntry( profkey + "Source" );
    sideBar = settings.readNumEntry( key + "SideBarPage" );
    geom.setRect( settings.readNumEntry( key + "GeometryX", 0 ),
		  settings.readNumEntry( key + "GeometryY", 0 ),
		  settings.readNumEntry( key + "GeometryWidth", -1 ),
		  settings.readNumEntry( key + "GeometryHeight", -1 ) );
    maximized = settings.readBoolEntry( key + "GeometryMaximized", FALSE );
    mainWinLayout = settings.readEntry( key + "MainwindowLayout" );
    assDocPath = settings.readEntry( key + "assistantDocPath", qInstallPathDocs() + QString( "/html" ) );

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
    const QString profkey = key + "Profile/" + profil->props["name"] + "/";
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
    if ( !hideSidebar )
	settings.writeEntry( key + "MainwindowLayout", mainWinLayout );
    settings.writeEntry( key + "LastProfile", profil->props["name"] );
    settings.writeEntry( key + "assistantDocPath", assDocPath );
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
    profile->props["basepath"] = settings.readEntry( profKey + "BasePath" );
    profile->props["startpage"] = settings.readEntry( profKey + "StartPage" );
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
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    QString lastProfile = settings.readEntry( key + "LastProfile" );
    if ( lastProfile.isEmpty() )
	settings.writeEntry( key + "LastProfile", profile->props["name"] );
    const QString profKey = key + "Profile/" + profile->props["name"] + "/";
    settings.writeEntry( profKey, changed );
    settings.writeEntry( profKey + "AppIcon", profile->props["applicationicon"] );
    settings.writeEntry( profKey + "AboutMenuText", profile->props["aboutmenutext"] );
    settings.writeEntry( profKey + "AboutUrl", profile->props["abouturl"] );
    settings.writeEntry( profKey + "Title", profile->props["title"] );
    settings.writeEntry( profKey + "BasePath", profile->props["basepath"] );
    settings.writeEntry( profKey + "StartPage", profile->props["startpage"] );

    QStringList titles, icons, imgDirs;
    QValueListConstIterator<QString> it = profile->docs.begin();
    for ( ; it != profile->docs.end(); ++it ) {
	titles << profile->titles[*it];
	icons << profile->icons[*it];
	imgDirs << profile->imageDirs[*it];
    }
    settings.writeEntry( profKey + "DocFiles", profile->docs );
    settings.writeEntry( profKey + "DocTitles", titles );
    settings.writeEntry( profKey + "DocIcons", icons );
    settings.writeEntry( profKey + "ImageDirs", imgDirs );
}

void Config::removeProfile( const QString &name )
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    const QString profKey = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/Profile/" + name + "/";
    settings.removeEntry( profKey );
    settings.removeEntry( profKey + "AppIcon" );
    settings.removeEntry( profKey + "AboutMenuText" );
    settings.removeEntry( profKey + "AboutUrl" );
    settings.removeEntry( profKey + "Title" );
    settings.removeEntry( profKey + "BasePath" );
    settings.removeEntry( profKey + "DocFiles" );
    settings.removeEntry( profKey + "DocTitles" );
    settings.removeEntry( profKey + "DocIcons" );
    settings.removeEntry( profKey + "ImageDirs" );
    settings.removeEntry( profKey + "StartPage" );
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

QStringList Config::mimePaths()
{
    QStringList lst;
    QValueListConstIterator<QString> it = profil->docs.begin();
    for ( ; it != profil->docs.end(); ++it ) {
	QFileInfo fi( *it );
	lst << fi.dirPath( TRUE );
	QDir dir( fi.dirPath( TRUE ) + "/" + profil->imageDirs[*it] );
	lst << dir.path();
    }
    return lst;
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

QString Config::homePage() const
{
    return home.isEmpty() ? profil->props["startpage"] : home;
}

QString Config::source() const
{
    return src.isEmpty() ? profil->props["startpage"] : src;
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

QString Config::basePath() const
{
    return profil->props["basepath"];
}

bool Config::needsNewDoc() const
{
    return profil->changed;
}

bool Config::startedWithProfile() const
{
    return startWithProfile;
}

bool Config::validProfileName() const
{
    return profileNameValid;
}

void Config::hideSideBar( bool b )
{
    hideSidebar = b;
}

bool Config::sideBarHidden() const
{
    return hideSidebar;
}
