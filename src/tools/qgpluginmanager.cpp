#include "qgpluginmanager.h"
#include "qmap.h"

QGPluginManager::QGPluginManager( const QUuid& id, const QString& path, QLibrary::Policy pol, bool cs )
    : interfaceId( id ), plugDict( 17, cs ), defPol( pol ), casesens( cs )
{
    // Every QLibrary object is destroyed on destruction of the manager
    libDict.setAutoDelete( TRUE );
    if ( !path.isEmpty() )
	addLibraryPath( path );
}

QGPluginManager::~QGPluginManager()
{
}

void QGPluginManager::addLibraryPath( const QString& path )
{
    if ( !QDir( path ).exists( ".", TRUE ) )
	return;

#if defined(Q_OS_WIN32)
    QString filter = "dll";
#elif defined(Q_OS_UNIX)
    QString filter = "so";
#elif defined(Q_OS_MACX)
    QString filter = "dylib";
#endif
    QStringList plugins = QDir(path).entryList( "*." + filter );
    for ( QStringList::Iterator p = plugins.begin(); p != plugins.end(); ++p ) {
	QString lib = path + "/" + *p;
	lib = lib.left( lib.length() - filter.length() - 1 );
	libList.append( lib );

	if ( defPol == QLibrary::Immediately ) {
	    if ( !addLibrary( lib ) )
		libList.remove( lib );
	}
    }
}

void QGPluginManager::setDefaultPolicy( QLibrary::Policy pol )
{
    defPol = pol;
}

QLibrary::Policy QGPluginManager::defaultPolicy() const
{
    return defPol;
}

QLibrary* QGPluginManager::library( const QString& feature ) const
{
    if ( feature.isEmpty() )
	return 0;

    // We already have a QLibrary object for this feature
    QLibrary *library = 0;
    if ( ( library = plugDict[feature] ) )
	return library;

    // Find the filename that matches the feature request best
    QMap<int, QStringList> map;
    QStringList::ConstIterator it = libList.begin();
    int best = 0;
    int worst = 15;
    while ( it != libList.end() ) {
	QString lib = *it;
	lib = lib.right( lib.length() - lib.findRev( "/" ) - 1 );
	lib = lib.left( lib.findRev( "." ) );
	int s = feature.similarityWith( lib );
	if ( s < worst )
	    worst = s;
	if ( s > best )
	    best = s;
	map[s].append( *it );
	++it;
    }

    // Start with the best match to get the library object
    QGPluginManager *that = (QGPluginManager*)this;
    for ( int s = best; s >= worst; --s ) {
	QStringList group = map[s];
	QStringList::Iterator git = group.begin();
	while ( git != group.end() ) {
	    QString lib = *git;
	    ++git;
	    if ( that->addLibrary( lib ) && ( library = plugDict[feature] ) )
		return library;
	}
    }

    return 0;
}

QStringList QGPluginManager::featureList() const
{
    // Make sure that all libraries have been loaded once.
    QGPluginManager *that = (QGPluginManager*)this;
    QStringList::ConstIterator it = libList.begin();
    while ( it != libList.end() ) {
	QString lib = *it;
	++it;
	that->addLibrary( lib );
    }

    QStringList list;
    QDictIterator<QLibrary> pit( plugDict );
    while( pit.current() ) {
	list << pit.currentKey();
	++pit;
    }

    return list;
}
