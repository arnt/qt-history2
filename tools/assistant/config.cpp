#include "config.h"
#include "profile.h"
#include "docuparser.h"

#include <qfile.h>
#include <qsettings.h>
#include <qxml.h>


static Config *static_configuration = 0;

Config::Config()
    : profile( 0 )
{
    load();
}

Config* Config::configuration()
{
    if( static_configuration == 0 ) {
	static_configuration = new Config();
	static_configuration->initDefault();
    }
    return static_configuration;
}

void Config::initDefault()
{
}

void Config::setProfile( Profile *prof )
{
    delete profile;
    profile = prof;
}

void Config::load()
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    selCat = settings.readListEntry( key + "CategoriesSelected" );
    webBrows = settings.readEntry( key + "Webbrowser", "" );
    home = settings.readEntry( key + "Homepage", "" );
    pdfApp = settings.readEntry( key + "PDFApplication" );
    fontFam = settings.readEntry( key + "Family" );

    fontFix = settings.readEntry( key + "FixedFamily" );
    fontSiz = settings.readNumEntry( key + "Size" );
    linkUnder = settings.readBoolEntry( key + "LinkUnderline" );
    linkCol = settings.readEntry( key + "LinkColor" );
    src = settings.readEntry( key + "Source" );
    sideBar = settings.readNumEntry( key + "SideBarPage" );
    geom.setRect( settings.readNumEntry( key + "GeometryX" ),
		  settings.readNumEntry( key + "GeometryY" ),
		  settings.readNumEntry( key + "GeometryWidth" ),
		  settings.readNumEntry( key + "GeometryHeight" ) );
    maximized = settings.readBoolEntry( key + "GeometryMaximized" );
    mainWinLayout = settings.readEntry( key + "MainwindowLayout" );
}

void Config::save()
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    settings.writeEntry( key + "CategoriesSelected", selCat );
    settings.writeEntry( key + "Webbrowser", webBrows );
    settings.writeEntry( key + "Homepage", home );
    settings.writeEntry( key + "PDFApplication", pdfApp );
    settings.writeEntry( key + "Family",  fontFam );
    settings.writeEntry( key + "Size",  fontSiz );
    settings.writeEntry( key + "FixedFamily", fontFix );
    settings.writeEntry( key + "LinkUnderline", linkUnder );
    settings.writeEntry( key + "LinkColor", linkCol );
    settings.writeEntry( key + "Source", src );
    settings.writeEntry( key + "SideBarPage", sideBarPage() );
    settings.writeEntry( key + "GeometryX", geom.x() );
    settings.writeEntry( key + "GeometryY", geom.y() );
    settings.writeEntry( key + "GeometryWidth", geom.width() );
    settings.writeEntry( key + "GeometryHeight", geom.height() );
    settings.writeEntry( key + "GeometryMaximized", maximized );
    settings.writeEntry( key + "MainwindowLayout", mainWinLayout );

}

QString Config::title() const
{
    return profile->props[ "title" ];
}

QString Config::aboutName() const
{
    return profile->props[ "aboutname" ];
}


QString Config::aboutURL() const
{
    return profile->props[ "abouturl" ];
}

QStringList Config::docFiles() const
{
    return profile->docs;
}

QPixmap Config::docIcon( const QString &docfile ) const
{
    return QPixmap::fromMimeSource( profile->icons[docfile] );
}

QPixmap Config::applicationIcon() const
{
    return QPixmap::fromMimeSource( profile->props["applicationicon"] );
}


DocuParser *Config::parser( const QString &docfile ) const
{
    if( parserCache.contains( docfile ) )
	return parserCache[docfile];

    DocuParser *handler = new DocuParser;
    QFile f( docfile );
    if( !QFile::exists( docfile ) )
	qWarning( "File does not exists: '%s'", docfile.latin1() );
    QXmlInputSource source( f );
    QXmlSimpleReader reader;
    reader.setContentHandler( handler );
    reader.setErrorHandler( handler );
    bool ok = reader.parse( source );
    if( !ok )
	qWarning( "Failed to parse document: '%s'", docfile.latin1() );

    ( (Config*) this )->parserCache[docfile] = handler;

    return handler;
}


QString Config::docTitle( const QString &docfile ) const
{
    return parser( docfile )->getDocumentationTitle();
}


QString Config::docCategory( const QString &docfile ) const
{
    return parser( docfile )->getCategory();
}
