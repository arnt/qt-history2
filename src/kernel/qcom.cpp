#include "qcom.h"

#ifndef QT_NO_COMPONENT
#include "qsettings.h"
#include "qlibrary.h"
#include "qcleanuphandler.h"

// We still have no name for the COM-like stuff here! See ###

/*!
  \class QUnknownInterface qcom.h
  \brief The QUnknownInterface class is the base class for all interfaces of the Qt Component Model.
  \ingroup componentmodel

  All ### interfaces are derived from the QUnknownInterface. This interface provides
  control of the object's lifetime and the ability to navigate a component implementing
  multple interfaces.
  The object's lifetime is controlled using a reference count that is increased and
  decreased using the functions addRef and release, respectively.
  The queryInterface functions determines whether the component supports a specific interface.
  For every interface a Unique Universal Identifier (UUID) is provided to identify requests
  for an interfaces. In Qt, this identifier is wrapped in the \link QUuid QUuid \endlink class
  that provides convenience operators for comparison and copying.
*/

/*!
  \fn QUnknownInterface* QUnknownInterface::queryInterface( const QUuid &request )

  Returns a pointer to an interface specified with \a request, or NULL if this interface 
  can't provide the requested interface. An implementation of this function must call
  addRef() on the pointer it returns.

  Example:
  \code
  QUnknownInterface *MyComponent::queryInterface( const QUuid &request )
  {
      QUnknownInterface *iface = 0;
      if ( request == IID_QUnknownInterface )
          return (QUnknownInterface*)this;
      else if ( request == IID_... )
	  return (...*)this;
      ...

      if ( iface )
          iface->addRef();
      return iface;
  }
  \endcode

  There are five requirements for implementations of queryInterface:
  <ul>
  <li>Always the same QUnknownInterface
  For any component, a query for the QUnknownInterface must always return the same pointer value to 
  allow a client to determine whether two interfaces point to the same component.
  <li>Static set of interfaces
  The number of interfaces for which queryInterface returns a valid pointer must not change.
  <li>Reflexive
  If a client holding a pointer to one interface and queries for that interface, the call must succeed.
  <li>Symmetric
  If a client holding a pointer to one interface and queries for another, a query through the obtained 
  interface for the first interface must succeed.
  <li>Transitive
  If a client holding a pointer to one interface queries successfully for a second, and queries that 
  successfully for a third interface, a query for that third interface on the first interface must succeed.
  </ul>

  \sa addRef(), release()
*/

/*!
  \fn ulong QUnknownInterface::addRef()

  Increases the reference counter for this interface by one and returns
  the old reference count.

  Example:
  \code
  int MyComponent::addRef()
  {
      return ref++;
  }
  \endcode

  This function must be called when this interface is returned as a result of a 
  queryInterface() call. It should be called for every new copy of a pointer to 
  this interface.

  \sa queryInterface(), release()
*/

/*!
  \fn ulong QUnknownInterface::release()

  Decreases the reference count for this interface by one and returns
  the new reference count. If the reference count falls to 0, the object is freed from memory.

  Example:
  \code
  int MyComponent::release()
  {
      if ( !--ref ) {
          delete this;
	  return 0;
      }
      return ref;
  }
  \endcode

  This function should be called whenever a copy of a pointer to this interface is no longer needed.

  \sa addRef()
*/



/*!
  \class QComponentInterface qcom.h
  \brief The QComponentInterface class is an interface to get information about components.
  \ingroup componentmodel
*/

/*!
  \fn QString QComponentInterface::name() const

  Returns a string with the name of the module.
*/

/*!
  \fn QString QComponentInterface::description() const

  Returns a string with a description of the module.
*/

/*!
  \fn QString QComponentInterface::author() const

  Returns a string with information about the author of the module.
*/

/*!
  \fn QString QComponentInterface::version() const

  Returns a string with information about the version of the module.
*/



/*!
  \class QLibraryInterface qcom.h
  \brief The QLibraryInterface class is an interface to control loading and unloading of components.
  \ingroup componentmodel
*/

/*!
  \fn bool QLibraryInterface::init()

  When this function returns FALSE, the component implementing this interface will not be loaded.
*/

/*!
  \fn void QLibraryInterface::cleanup()

  Called by QLibrary before unloading the component implementing this interface.
*/

/*!
  \fn bool QLibraryInterface::canUnload() const

  When this function returns FALSE, the component implementing this interface will not be unloaded.
  This function may be called regularily to unload unused libraries.
*/



/*!
  \class QFeatureListInterface qcom.h
  \brief The QFeatureListInterface class is an interface to retrieve information about features provided by a component.
  \ingroup componentmodel
*/

/*!
  \fn QStringList QFeatureListInterface::featureList() const

  Returns a list of features implemented in this component.
*/


/*!
  \class QComponentServerInterface qcom.h
  \brief The QComponentServerInterface class is an interface to register and unregister components.
  \ingroup componentmodel

  \sa QComponentFactory
*/

/*!
  \fn bool QComponentServerInterface::registerComponents( const QString &filepath ) const

  Registers the components in this server in the system registry and returns 
  TRUE when successfull, otherwise returns FALSE. \a filepath is the absolut path
  to the shared library file.

  \code
  QSettings register;
  bool ok;

  register.insertSearchPath( QSettings::Windows, "/Classes" );
  ok = register.writeEntry( "/CLSID/{DD19964B-A2C8-42AE-AAF9-8ADC509BCA03}/Default", "Test Component" );
  ok = register.writeEntry( "/CLSID/{DD19964B-A2C8-42AE-AAF9-8ADC509BCA03}/InprocServer32/Default", filepath ) && ok;
  
  return ok;
  \endcode
*/

/*!
  \fn bool QComponentServerInterface::unregisterComponents() const

  Removes the component in this server from the system registry and returns 
  TRUE if successfull, otherwise returns FALSE.

  \code
  QSettings settings;
  bool ok;

  settings.insertSearchPath( QSettings::Windows, "/Classes" );
  ok = settings.removeEntry( "/CLSID/{DD19964B-A2C8-42AE-AAF9-8ADC509BCA03}/InprocServer32/Default" );
  ok = settings.removeEntry( "/CLSID/{DD19964B-A2C8-42AE-AAF9-8ADC509BCA03}/Default" ) && ok;
  
  return ok;
  \endcode
*/



/*!
  \class QComponentFactory qcom.h
  \brief The QComponentFactory class provides static functions to create components.
  \ingroup componentmodel

  \sa QComponentServerInterface
*/

QCleanupHandler< QLibrary > qt_component_server_cleanup;

/*!
  Looks up the component identifier \a cid in the system registry, loads the corresponding
  component server and queries for the interface \a iid. Returns the retrieved interface pointer, 
  or NULL if there was an error.

  Example:
  \code
  MyInterface *iface = (MyInterface*)QComponentFactory::createInstance( IID_MyInterface, CID_MyComponent );
  if ( iface ) {
      ...
      iface->release();
  }
  \endcode
*/

QUnknownInterface *QComponentFactory::createInstance( const QUuid &cid, const QUuid &iid )
{
    QUnknownInterface *iface = 0;

    QSettings settings;
    bool ok;

    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    QString file = settings.readEntry( "/CLSID/" + cid.toString() + "/InprocServer32/Default", QString::null, &ok );
    int dot = file.findRev( '.' );
    QString ext = file.right( file.length() - dot );
    if ( ext == ".dll" || ext == ".so" || ext == ".dylib" )
	file = file.left( dot );

    if ( !ok )
	return 0;

    QLibrary *library = new QLibrary( file );
    qt_component_server_cleanup.add( library );

    QComponentFactoryInterface *cfIface = (QComponentFactoryInterface *)library->queryInterface( IID_QComponentFactoryInterface );
    if ( cfIface ) {
	iface = cfIface->createInstance( iid, cid );
	cfIface->release();
    } else {
	iface = library->queryInterface( iid );
    }

    return iface;
}

/*!
  Loads the shared library \a filename and queries for a QComponentServerInterface.
  If the library implements this interface, the \link QComponentServerInterface::registerComponents registerComponents \endlink
  function is called and the result of the call returned. Otherwise returns FALSE.
*/
bool QComponentFactory::registerServer( const QString &filename )
{
    QLibrary lib( filename, QLibrary::Immediately );
    QComponentServerInterface *iface = (QComponentServerInterface*)lib.queryInterface( IID_QComponentServerInterface );
    if ( !iface )
	return FALSE;
    bool ok = iface->registerComponents( filename );
    iface->release();
    return ok;
}

/*!
  Loads the shared library \a filename and queries for a QComponentServerInterface.
  If the library implements this interface, the \link QComponentServerInterface::unregisterComponents unregisterComponents \endlink
  function is called and the result of the call returned. Otherwise returns FALSE.
*/
bool QComponentFactory::unregisterServer( const QString &filename )
{
    QLibrary lib( filename, QLibrary::Immediately );
    QComponentServerInterface *iface = (QComponentServerInterface*)lib.queryInterface( IID_QComponentServerInterface );
    if ( !iface )
	return FALSE;
    bool ok = iface->unregisterComponents();
    iface->release();
    return ok;
}

#endif // QT_NO_COMPONENT
