#include "qplugin.h"
#include <qdir.h>
#include <qtimer.h>
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
  Loads the shared library and initializes function pointers. This function
  gets called automatically if the policy is not ManualPolicy. Otherwise you
  have to make sure that the library has been loaded before usage.

  \sa setPolicy()
*/
bool QPlugIn::load()
{
    if ( libfile.isEmpty() )
	return FALSE;

    if ( !pHnd ) {
#if defined(_WS_WIN_)
	pHnd = LoadLibraryA( libfile );  //### use LoadLibrary for NT_based systems
	ConnectProc c = (ConnectProc) GetProcAddress( pHnd, "onConnect" );
	if ( c )
	    if ( !c( qApp ) )
		return FALSE;
#elif defined(_WS_X11_)
	pHnd = dlopen( libfile, (libPol == DefaultPolicy) ? RTLD_LAZY : RTLD_NOW );
	ConnectProc c = (ConnectProc) dlsym( pHnd, "onConnect" );
	if( c )
	    if ( !c(  qApp ) )
		return FALSE;
#endif
	emit loaded();
    }

    if ( pHnd )
	return ifc ? TRUE : loadInterface();
    return FALSE;
}

/*!
  Unloads the library if there are no more references.
  If \a force is set to TRUE, the library gets unloaded
  at any cost, which is in most cases a segmentation fault,
  so you should know what you're doing!

  \sa load, guard
*/
void QPlugIn::unload( bool force )
{
    if ( pHnd ) {
	if ( count ) {
#ifdef CHECK_RANGE
	    qWarning("Library is still used!");
#endif
	    if ( !force )
		return;
	}
	delete ifc;
	ifc = 0;

#ifdef _WS_WIN_
	ConnectProc dc = (ConnectProc) GetProcAddress( pHnd, "onDisconnect" );
	if ( dc )
	    if ( !dc( qApp ) )
		return;
	FreeLibrary( pHnd );
#else
	ConnectProc dc = (ConnectProc) dlsym( pHnd, "onDisconnect" );
	if ( dc )
	    if( !dc( qApp ) )
		return;
	dlclose( pHnd );
#endif	
	emit unloaded();
    }
    pHnd = 0;
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
	if ( libPol != ManualPolicy )
	    return load();
	else
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
    }

    return TRUE;
}

/*!
  Tries to unloads the library if policy is ManualPolicy.

  \sa setPolicy
*/
void QPlugIn::unuse()
{
    if ( libPol == OptimizeMemory  && !count )
	unload();
}

/*!
  Loads the interface of the shared library and returns TRUE if successful, 
  or FALSE if the interface could not be loaded.
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
    return ifc; // && ifc->inherits( "QPlugInInterface" );
}

/*!
  Calls the library's name() function and returns the result.
*/
QString QPlugIn::name()
{
    use();
    QString str = iface()->name();
    unuse();

    return str;
}

/*!
  Calls the library's description() function and returns the result.
*/
QString QPlugIn::description()
{
    use();
    QString str = iface()->description();
    unuse();

    return str;
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
