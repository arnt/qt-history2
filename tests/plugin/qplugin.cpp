#include "qplugin.h"
#include <qdir.h>
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
  <li> \c DefaultPolicy - The library get's loaded on first need
  <li> \c OptimizeSpeed - The library is loaded as soon as possible
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
: pHnd( 0 ), libfile( filename ), libPol( pol )
{
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

  Reimplement this function for custom plugin support.

  \sa setPolicy()
*/
bool QPlugIn::load()
{
    if ( libfile.isEmpty() )
	return FALSE;

    if ( !pHnd )
#ifdef _WS_WIN_
	pHnd = LoadLibrary( libfile );
#else
	pHnd = dlopen( libfile, (libPol == DefaultPolicy) ? RTLD_LAZY : RTLD_NOW );
#endif	

    infoStringPtr = (STRINGPROC) getSymbolAddress( "infoString" );

    return (bool)pHnd;
}

/*!
  Unloads the library.
*/
void QPlugIn::unload()
{
    if ( pHnd )
#ifdef _WS_WIN
	FreeLibrary( pHnd );
#else
	dlclose( pHnd );
#endif	

    pHnd = 0;
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
void QPlugIn::use()
{
    if ( !pHnd ) {
	if ( libPol != ManualPolicy )
	    load();
	else
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
    }
}

/*!
  Returns the address of the library's exported symbol \a sym.
  Returns 0 if the symbol could not be located.
*/
void* QPlugIn::getSymbolAddress( const QString& sym )
{
#ifdef _WS_WIN_
    return GetProcAddress( pHnd, sym );
#else
    return dlsym( pHnd, sym );
#endif
}

/*!
  Calls the library's infoString() function and returns the result.
*/
const char* QPlugIn::infoString()
{
    use();

    if ( infoStringPtr )
	return infoStringPtr();
    return "";
}

/*!
  \fn bool QPlugIn::addToManager( QPlugInDict& dict )

  Registers this plugin with all widgets and actions it provides in \a dict and
  returns TRUE if successful.
  This pure virtual function gets called by QPlugInManager::addPlugIn() and has 
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
  \fn bool QPlugInManager::addPlugIn( const QString& file )

  Loads the shared library \a file and registers all provided
  widgets and actions.

  \sa addPlugIn()
*/

/*!
  \fn void QPlugInManager::addPlugInPath( const QString& path, const QString& filter )

  Tries to add all shared libraries matching \a filter in \a path.
*/
