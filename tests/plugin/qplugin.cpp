#include "qplugin.h"
#include <qdir.h>
#include <qtimer.h>
#include <qapplication.h>

#ifdef _WS_WIN_
#include <qt_windows.h>
#else
#include <dlfcn.h>
#endif

/*!
  \class QPlugIn qplugin.h

  \brief Abstract class for plugin implementation
*/

/*!
  \enum QPlugIn::LibraryPolicy

  This enum type is used to set and read the plugin's library
  policy.
  Defined values are:
  <ul>
  <li> \c DefaultPolicy - The library get's loaded on first need and never unloaded
  <li> \c OptimizeSpeed - The library is loaded as soon as possible at the cost of memory
  <li> \c OptimizeMemory - The library gets unloaded as often as possible at the cost of speed
  <li> \c ManualPolicy - The library has to be loaded and unloaded manually
  </ul>
*/

/*!
  Creates a plugin using the shared library \a filename.
  The library get's loaded immediately if \a pol is OptimizeSpeed,
  otherwise as soon as necessary.

  \sa setPolicy()
*/
QPlugIn::QPlugIn( const QString& filename, LibraryPolicy pol )
    : count( 0 ), pHnd( 0 ), libfile( filename ), libPol( pol )
{
    ifc = 0;
    if ( pol == OptimizeSpeed )
	load();
}

/*!
  Deletes the plugin.

  Unloads the shared library as appropriate.
*/
QPlugIn::~QPlugIn()
{
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

	emit loaded();
    }

    if ( pHnd )
	return ifc ? TRUE : loadInterface();
    return FALSE;
}

/*!
  Unloads the library if there are no more references. Calls the library's
  onDisconnect routine and returns TRUE if the library was unloaded. Does
  nothing and returns FALSE otherwise.

  If \a force is set to TRUE, the library gets unloaded
  at any cost, which is in most cases a segmentation fault,
  so you should know what you're doing!

  \sa load, guard
*/
bool QPlugIn::unload( bool force )
{
    if ( pHnd ) {
	if ( count ) {
#ifdef CHECK_RANGE
	    qWarning("Library is still used!");
#endif
	    if ( !force )
		return FALSE;
	}
	if ( ifc ) {
	    delete ifc;
	    ifc = 0;

	    ConnectProc dc;
#if defined(_WS_WIN_)
	    dc = (ConnectProc) GetProcAddress( pHnd, "onDisconnect" );
#elif defined(_WS_X11_)
	    dc = (ConnectProc) dlsym( pHnd, "onDisconnect" );
#endif
	    if ( dc )
		if ( !dc( qApp ) )
		    return FALSE;
#if defined(_WS_WIN_)
	    FreeLibrary( pHnd );
#elif defined(_WS_X11_)
	    dlclose( pHnd );
#endif	
	}
	emit unloaded();
    }
    pHnd = 0;
    return TRUE;
}

/*! \internal
*/
bool QPlugIn::deref()
{
    bool r = !count--;

    // We can do that because the object an all children are
    // destroyed when the timer fires
    if ( !r && libPol == OptimizeMemory )
	QTimer::singleShot( 0, this, SLOT(unuse()));

    return r;
}

/*! 
  \fn void QPlugIn::ref()
  
  \internal
*/

/*! 
  Call this function for each object you create through
  the library.
*/
void QPlugIn::guard( QObject* o )
{
    if ( !o )
	return;

    ref();
    connect( o, SIGNAL( destroyed() ), this, SLOT(deref()) );
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
    }

    return TRUE;
}

/*!
  Tries to unloads the library if policy is ManualPolicy.

  \sa setPolicy
*/
void QPlugIn::unuse()
{
    if ( libPol == OptimizeMemory && !count )
	unload();
}

/*!
  Loads the interface of the shared library and alls the onConnect routine.
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
    proc = (LoadInterfaceProc) GetProcAddress( pHnd, "loadInterface" );
#else
    proc = (LoadInterfaceProc) dlsym( pHnd, "loadInterface" );
#endif
    if ( !proc )
	return FALSE;
    ifc = proc();

    if ( !ifc )
	return FALSE;

    if ( ifc->queryInterface() != queryInterface() ) {
	delete ifc;
	ifc = 0;
	return FALSE;
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

    QString str = iface()->name();
    unuse();

    return str;
}

/*!
  Calls the library's description() function and returns the result.
*/
QString QPlugIn::description()
{
    if ( !use() )
	return QString::null;

    QString str = iface()->description();
    unuse();

    return str;
}

/*!
  Calls the library's author() function and returns the result.
*/
QString QPlugIn::author()
{
    if ( !use() )
	return QString::null;

    QString str = iface()->author();
    unuse();

    return str;
}

/*!
  Calls the library's featureList() function and returns the result.
*/
QStringList QPlugIn::featureList()
{
    if ( !use() )
	return QStringList();

    QStringList list = iface()->featureList();
    unuse();

    return list;
}

/*!
  \fn bool QPlugIn::addToManager( QPlugInDict& dict )

  Registers this plugin in \a dict and returns TRUE if successful.
  This pure virtual function gets called by QPlugInManager::addPlugIn() and has
  to be reimplemented for custom extensions.
*/

/*!
  \fn bool QPlugIn::removeFromManager( QPlugInDict& dict )

  Remove this plugin from \a dict and returns TRUE if successful.

  This pure virtual function gets called by QPlugInManager::removePlugIn() and has
  to be reimplemented for custom extensions.
*/

/*!
  \class QPlugInManager qplugin.h
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
