#include "qplugin.h"
#include "qapplicationinterfaces.h"
#include <qapplication.h>

#ifdef _WS_WIN_
#include <qt_windows.h>
#else
#include <dlfcn.h>
#endif

/*!
  \class QPlugInInterface qplugininterface.h

  \brief An abstract class to provide a common interface to functionality a plugin provides.

  \sa QClientInterface
*/

/*!
  \fn QPlugInInterface::QPlugInInterface()
  
  Creates a QPlugInInterface and initializes the object.
*/

/*!
  \fn QPlugInInterface::~QPlugInInterface()
  
  Destroys the QPlugInInterface and disconnects the plugin from the application.
  Reimplement the destructor in the plugin to provide cleanup for object created 
  in your plugin.

  \sa QCleanUpHandler
*/

/*!
  \fn QString QPlugInInterface::name()

  Reimplement this function to return the name of the plugin.
  The default implementation returns QString::null.
*/

/*!
  \fn QString QPlugInInterface::description()

  Reimplement this function to return a description of the plugin.
  The default implementation returns QString::null.
*/

/*!
  \fn QString QPlugInInterface::author()

  Reimplement this function to return information about the author of 
  the plugin.
  The default implementation returns QString::null.
*/

/*!
  \fn QStringList QPlugInInterface::featureList()

  Reimplement this function to provide a list of features your plugin
  provides. This function is used by a QPlugInManager to locate the
  plugin that provides a requested feature.

  The default implementation returns an empty list.

  \sa QPlugInManager
*/

/*!
  \fn QCString QPlugInInterface::queryPlugInInterface() const
  \overload

  Returns the name of the plugin's interface. You have to overwrite this 
  method in your subclass for the plugin loader to identify the library.
*/

/*!
  \fn QStrList QPlugInInterface::queryInterfaceList() const

  Reimplement this function to provide a list of interfaces your plugin
  would like to use. The plugin-loader will connect your plugin's QClientInterface
  to all matching interfaces the application provides.

  \sa clientInterface()
*/

/*!
  \fn QClientInterface* QPlugInInterface::clientInterface( const QCString& request ) const

  Returns a pointer to the client interface the plugin can use to access the
  application's interface \a request.
*/

/*
  Called by the plugin to connect the a QClientInterface matching \a request to the 
  corresponding QApplicationInterface.
*/
QClientInterface* QPlugInInterface::requestClientInterface( const QCString& request ) 
{
    if ( queryInterfaceList().contains( request ) ) {
	QClientInterface* ifc = cIfaces[request];
	if ( !ifc ) {
	    ifc = new QClientInterface;
	    cIfaces.insert( request, ifc );
	}
	return ifc;
    } else {
	return 0;
    }
}

/*!
  \class QCleanUpHandler

  \brief Provides a save class for memory cleanup.
*/

/*!
  \fn QCleanUpHandler::~QCleanUpHandler()
  
  This destructor will delete all handled objects.
*/

/*!
  \fn void QCleanUpHandler::addCleanUp( Type* object )

  Adds an object to the list that will be destroyed upon
  destruction of the cleanup handler itself.
*/

/*!
  \fn bool QCleanUpHandler::clean()

  Return TRUE if there are any undeleted objects this handler
  has to care about.
  This is quite useful for plugins that do not want to be unloaded
  until all memory allocated in the library's scope has been freed.
*/

/*!
  \class QPlugIn qplugin.h

  \brief This class provides a wrapper for library loading and unloading.
*/

/*!
  \enum QPlugIn::LibraryPolicy

  This enum type is used to set and read the plugin's library
  policy.
  Defined values are:
  <ul>
  <li> \c Default - The library get's loaded on first need and never unloaded
  <li> \c OptimizeSpeed - The library is loaded as soon as possible at the cost of memory
  <li> \c Manual - The library has to be loaded and unloaded manually
  </ul>
*/

/*!
  Creates a plugin using the shared library \a filename.

  The library get's loaded immediately if \a pol is OptimizeSpeed,
  as soon as necessary if \a pol is Default, or not automatically
  if \a pol is Manual.

  When the library get's loaded, the function \a fn will be executed to
  retrieve the interface object. If \a fn is not specified, the function
  "loadInterface" will be used.

  \sa setPolicy(), load()
*/
QPlugIn::QPlugIn( const QString& filename, LibraryPolicy pol, const char* fn )
    : pHnd( 0 ), libfile( filename ), libPol( pol )
{
    if ( fn )
	function = fn;
    else
	function = "loadInterface";
    ifc = 0;
    if ( pol == OptimizeSpeed )
	load();
}

/*!
  Deletes the plugin.

  When the library policy is not Manual, the plugin will try to unload the library.

  \sa unload()
*/
QPlugIn::~QPlugIn()
{
    if ( libPol != Manual )
	unload();
}

/*!
  Loads the shared library and initializes function pointers. Returns TRUE if 
  the library was loaded successfully, otherwise does nothing and returns FALSE.

  This function gets called automatically if the policy is not ManualPolicy. 
  Otherwise you have to make sure that the library has been loaded before usage.

  \sa setPolicy()
*/
bool QPlugIn::load()
{
    if ( libfile.isEmpty() )
	return FALSE;

    if ( !pHnd ) {
#if defined(_WS_WIN_)
	pHnd = LoadLibraryA( libfile );  //### use LoadLibrary for NT_based systems
#elif defined(_WS_X11_)
	pHnd = dlopen( libfile, RTLD_NOW );
#endif
	if ( !pHnd )
	    return FALSE;
    }

    if ( pHnd )
	return ifc ? TRUE : loadInterface();
    return FALSE;
}

/*!
  Unloads the library. 
  Calls the library's onDisconnect routine and returns TRUE if the library 
  was unloaded. Does nothing and returns FALSE otherwise.

  If \a force is set to TRUE, the library gets unloaded
  at any cost, which is in most cases a segmentation fault,
  so you should know what you're doing!

  \sa load, guard
*/
bool QPlugIn::unload( bool force )
{
    if ( pHnd ) {
	if ( ifc ) {
	    ConnectProc dc;
    #if defined(_WS_WIN_)
	    dc = (ConnectProc) GetProcAddress( pHnd, "onDisconnect" );
    #elif defined(_WS_X11_)
	    dc = (ConnectProc) dlsym( pHnd, "onDisconnect" );
    #endif
	    if ( dc ) {
		if ( !dc( qApp ) && !force)
		    return FALSE;
	    }

	    delete ifc;
	    ifc = 0;

#if defined(_WS_WIN_)
	    FreeLibrary( pHnd );
#elif defined(_WS_X11_)
	    dlclose( pHnd );
#endif	
	}
    }
    pHnd = 0;
    return TRUE;
}

/*!
  Sets the current policy to \a pol.
  Forces the library to load if \a pol is set to
  OptimizeSpeed.

  \sa LibraryPolicy
*/
void QPlugIn::setPolicy( LibraryPolicy pol )
{
    libPol = pol;

    if ( libPol == OptimizeSpeed )
	load();
}

/*!
  Returns the current policy.
*/
QPlugIn::LibraryPolicy QPlugIn::policy() const
{
    return libPol;
}

/*!
  Returns the filename of the shared object connected to this plugin.
*/
QString QPlugIn::library() const
{
    return libfile;
}

/*!
  Makes sure the library is loaded unless policy
  is ManualPolicy. Call this method before accessing
  the library functions.

  \sa setPolicy
*/
bool QPlugIn::use()
{
    if ( !pHnd ) {
	if ( libPol != Manual )
	    return load();
#ifdef CHECK_RANGE
	else
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
#endif
	return FALSE;
    }
    return TRUE;
}

/*!
  Loads the interface of the shared library and calls the onConnect routine.
  Returns TRUE if successful, or FALSE if the interface could not be loaded.
*/
bool QPlugIn::loadInterface()
{
    if ( !pHnd ) {
	qWarning("QPlugIn::loadInterface(): Failed to load library - no handle!");
	return FALSE;
    }

    LoadInterfaceProc proc;
#ifdef _WS_WIN_
    proc = (LoadInterfaceProc) GetProcAddress( pHnd, function );
#else
    proc = (LoadInterfaceProc) dlsym( pHnd, function );
#endif
    if ( !proc )
	return FALSE;
    ifc = proc();

    if ( !ifc )
	return FALSE;

    if ( ifc->queryPlugInInterface() != queryPlugInInterface() ) {
	delete ifc;
	ifc = 0;
	return FALSE;
    } else {
	QStrList appIfaces = qApp->queryInterfaceList();
	for ( uint i = 0; i < appIfaces.count(); i++ ) {
	    QCString iface = appIfaces.at( i );
	    QStrList clIface = plugInterface()->queryInterfaceList();
	    if ( clIface.contains( iface ) ) {
		QClientInterface* ci = plugInterface()->requestClientInterface( iface );
		QApplicationInterface* ai = qApp->requestApplicationInterface( iface );
		if ( ai && ci ) {
		    QObject::connect( ci, SIGNAL(writeProperty(const QCString&, const QVariant&)), ai, SLOT(requestSetProperty(const QCString&, const QVariant&)) );
		    QObject::connect( ci, SIGNAL(readProperty(const QCString&,QVariant&)), ai, SLOT(requestProperty(const QCString&,QVariant&)) );
		} else {
#ifdef CHECK_RANGE
		    qWarning( "Can't setup connection for interface \"%s\"", (const char*)iface );
		    if ( !ai )
			qWarning( "\tApplication failed to provide requested implementation!");
		    if ( !ci )
			qWarning( "\tPlugIn failed to provide requested implementation!");
#endif
		}
	    }
	}
    }

#if defined(_WS_WIN_)
    ConnectProc c = (ConnectProc) GetProcAddress( pHnd, "onConnect" );
#elif defined(_WS_X11_)
    ConnectProc c = (ConnectProc) dlsym( pHnd, "onConnect" );
#endif
    if( c )
	c( qApp );

    return ifc != 0;
}

/*!
  Calls the library's name() function and returns the result.
*/
QString QPlugIn::name()
{
    if ( !use() )
	return QString::null;

    QString str = plugInterface()->name();

    return str;
}

/*!
  Calls the library's description() function and returns the result.
*/
QString QPlugIn::description()
{
    if ( !use() )
	return QString::null;

    QString str = plugInterface()->description();

    return str;
}

/*!
  Calls the library's author() function and returns the result.
*/
QString QPlugIn::author()
{
    if ( !use() )
	return QString::null;

    QString str = plugInterface()->author();

    return str;
}

/*!
  Calls the library's featureList() function and returns the result.
*/
QStringList QPlugIn::featureList()
{
    if ( !use() )
	return QStringList();

    QStringList list = plugInterface()->featureList();

    return list;
}

/*!
  \class QPlugInManager qpluginmanager.h
  \brief Template class for plugin management.

  The QPlugInManager provides basic support for plugins.
*/

/*!
  \fn QPlugInManager::QPlugInManager( const QString &path, QPlugIn::LibraryPolicy pol )

  Creates a plugin manager.
  The manager looks up and loads all shared libraries in \a path.

  \sa addPlugInPath(), addPlugIn()
*/

/*!
  \fn QPlugInManager::~QPlugInManager()
  Deletes the plugin manager.

  Calls the destructor of all managed plugins, too.
*/

/*!
  \fn void QPlugInManager::setDefaultPolicy( QPlugIn::LibraryPolicy pol )

  Sets the current default policy to \a pol.
  The default policy does not affect plugins already registered to
  this manager.

  \sa QPlugIn::setPolicy
*/

/*!
  \fn QPlugIn::LibraryPolicy QPlugInManager::defaultPolicy() const

  Returns the current default policy.

  \sa setDefaultPolicy
*/

/*!
  \fn QPlugIn* QPlugInManager::addLibrary( const QString& file )

  Loads the shared library \a file and registers all provided
  widgets and actions. Returns the QPlugIn* created when successful, 
  otherwise 0.

  \sa addPlugIn()
*/

/*!
  \fn bool QPlugInManager::removeLibrary( const QString& file )

  Tries to unload the library. Returns TRUE when successful, and removes 
  the library from management. Otherwise returns FALSE.
*/

/*!
  \fn void QPlugInManager::addPlugInPath( const QString& path, const QString& filter )

  Tries to add all shared libraries matching \a filter in \a path.
*/
