#include "config.h"
#include "profile.h"
#include "docuparser.h"

#include <qapplication.h>
#include <qfile.h>
#include <qsettings.h>
#include <qxml.h>


static Config *static_configuration = 0;

Config::Config( Profile *p )
    : profil( p )
{
    Q_ASSERT( p );

    if( !static_configuration ) {
	static_configuration = this;
    } else {
	qWarning( "Can only have one instance of Config at a time!" );
    }

    load();
}

Config *Config::configuration()
{
    Q_ASSERT( static_configuration );
    return static_configuration;
}

void Config::load()
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    const QString profkey = key + profil->props["name"] + "/";
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    bool ok = FALSE;
    selCat = settings.readListEntry( profkey + "/CategoriesSelected", &ok );
    if( !ok ) {
	selCat.clear();
	selCat << "all";
	QStringList docs = docFiles();
	QStringList::ConstIterator it = docs.begin();
	while( it!=docs.end() ) {
	    selCat << docCategory( *it );
	    ++it;
	}
    }

    webBrows = settings.readEntry( key + "Webbrowser" );
    home = settings.readEntry( profkey + "Homepage" );
    pdfApp = settings.readEntry( key + "PDFApplication" );
    fontFam = settings.readEntry( key + "Family", qApp->font().family() );

    fontFix = settings.readEntry( key + "FixedFamily", "courier" );
    fontSiz = settings.readNumEntry( key + "Size", qApp->font().pointSize() );
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

    profDiffer = ( settings.readEntry( key + "LastProfile" ) != profil->props["name"] );
}

void Config::save()
{
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";
    const QString profkey = key + profil->props["name"] + "/";
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    settings.writeEntry( key + profil->props["name"] + "/CategoriesSelected", selCat );
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

QString Config::title() const
{
    return profil->props[ "title" ];
}

QString Config::aboutName() const
{
    return profil->props[ "aboutname" ];
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

QString Config::docContentsURL( const QString &docfile ) const
{
    return parser( docfile )->contentsURL();
}
