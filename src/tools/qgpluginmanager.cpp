#include "qgpluginmanager.h"
#ifndef QT_NO_COMPONENT
#include "qmap.h"

/*!
  \class QPluginManager qpluginmanager.h
  \brief The QPluginManager class provides basic functions to access a certain kind of functionality in libraries.
  \ingroup componentmodel

  A common usage of components is to extend the existing functionality in an application using plugins. The application
  defines interfaces that abstract a certain group of functionality, and a plugin provides a specialized implementation
  of one or more of those interfaces.

  The QPluginManager template has to be instantiated with an interface definition and the IID for this interface. 

  \code
  QPluginManager<MyPluginInterface> *manager = new QPluginManager<MyPluginInterface>( IID_MyPluginInterface );
  \endcode

  It searches a specified directory for all shared libraries, queries for components that implement the specific interface and
  reads information about the features the plugin wants to add to the application. The component can provide the set of features
  provided by implementing either the QFeatureListInterface or the QComponentInterface. The strings returned by the implementations
  of

  \code
  QStringList QFeatureListInterface::featureList() const
  \endcode

  or

  \code
  QString QComponentInterface::name() const
  \endcode
  
  respectively, can then be used to access the component that provides the requested feature:

  \code
  MyPluginInterface *iface;
  manager->queryInterface( "feature", &iface );
  if ( iface )
      iface->execute( "feature" );
  \endcode

  The application can use a QPluginManager instance to create parts of the user interface based on the list of features 
  found in plugins:

  \code
  QPluginManager<MyPluginInterface> *manager = new QPluginManager<MyPluginInterface>( IID_ImageFilterInterface );
  manager->addLibraryPath(...);

  QStringList features = manager->featureList();
  for ( QStringList::Iterator it = features.begin(); it != features.end(); ++it ) {
      MyPluginInterface *iface;
      manager->queryInterface( *it, &iface );

      // use QAction to provide toolbuttons and menuitems for each feature...
  }
  \endcode
*/

/*!
  \fn QPluginManager::QPluginManager( const QUuid& id, const QString& path = QString::null, QLibrary::Policy pol = QLibrary::Delayed, bool cs = TRUE )

  Creates an QPluginManager for interfaces \a id that will load all shared library files in \a path, 
  setting the default policy to \a pol. If \a cs is, FALSE the manager will handle feature strings case insensitive.
  
  \warning
  Setting the cs flag to FALSE requires that components also convert to lower case when comparing with passed strings, so this has
  to be handled with care and documented very well.

  The \a pol parameter is propagated to the QLibrary object created for each library.
*/

/*!
  \fn void QPluginManager::addLibraryPath( const QString& path )

  Calls addLibrary for all shared library files in \a path. 
  The current library policy will be used for all new QLibrary objects.

  \sa addLibrary(), setDefaultPolicy()
*/

/*!
  \fn QLibrary* QPluginManager::addLibrary( const QString& file )

  Tries to load the library \a file, adds the library to the managed list and
  returns the created QLibrary object if successful, otherwise returns 0. If
  there is already a QLibrary object for \a file, this object will be returned.
  The library will stay in memory if the default policy is Immediately, otherwise 
  it gets unloaded again.

  Note that \a file does not have to include the platform dependent file extension.

  \sa removeLibrary(), addLibraryPath()
*/

/*!
  \fn bool QPluginManager::removeLibrary( const QString& file )

  Removes the library \a file from the managed list and returns TRUE if the library could
  be unloaded, otherwise returns FALSE.

  \warning
  The QLibrary object for this file will be destroyed.

  \sa addLibrary()
*/

/*!
  \fn void QPluginManager::setDefaultPolicy( QLibrary::Policy pol )

  Sets the default policy for this plugin manager to \a pol. The default policy is
  propagated to all newly created QLibrary objects.

  \sa defaultPolicy()
*/

/*!
  \fn QLibrary::Policy QPluginManager::defaultPolicy() const

  Returns the current default policy.

  \sa setDefaultPolicy()
*/

/*!
  \fn QRESULT QPluginManager::queryInterface(const QString& feature, Type** iface) const

  Sets \a iface to point to the interface providing \a feature.

  \sa featureList(), library()
*/

/*!
  \fn QLibrary* QPluginManager::library( const QString& feature ) const

  Returns a pointer to the QLibrary providing \a feature.

  \sa featureList(), libraryList()
*/

/*!
  \fn QStringList QPluginManager::featureList() const

  Returns a list of all features provided by the interfaces managed by this 
  interface manager.

  \sa library(), queryInterface()
*/


QGPluginManager::QGPluginManager( const QUuid& id, QLibrary::Policy pol, bool cs )
    : interfaceId( id ), plugDict( 17, cs ), defPol( pol ), casesens( cs )
{
    // Every QLibrary object is destroyed on destruction of the manager
    libDict.setAutoDelete( TRUE );
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
#elif defined(Q_OS_MACX)
    QString filter = "dylib";
#elif defined(Q_OS_UNIX)
    QString filter = "so";
#endif

    QStringList plugins = QDir(path).entryList( "*." + filter );
    for ( QStringList::Iterator p = plugins.begin(); p != plugins.end(); ++p ) {
	QString lib = path + "/" + *p;
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

#endif //QT_NO_COMPONENT
