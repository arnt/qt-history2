#define Q_INITGUID
#include "qcomponentinterface.h"
#ifndef QT_NO_STYLE
#include "qstyleinterface.h"
#endif
#ifndef QT_NO_SQL
#include "qsqldriverinterface.h"
#endif
#undef Q_INITGUID

#ifndef QT_NO_COMPONENT

// {1D8518CD-E8F5-4366-99E8-879FD7E482DE}
Q_GUID(IID_QUnknownInterface, 
0x1d8518cd, 0xe8f5, 0x4366, 0x99, 0xe8, 0x87, 0x9f, 0xd7, 0xe4, 0x82, 0xde);

// {5F3968A5-F451-45b1-96FB-061AD98F926E}
Q_GUID(IID_QComponentInterface, 
0x5f3968a5, 0xf451, 0x45b1, 0x96, 0xfb, 0x6, 0x1a, 0xd9, 0x8f, 0x92, 0x6e);

/*!
  \class QUnknownInterface qcomponentinterface.h
  \brief This class serves as a base class for interfaces.
*/

/*!
  \fn QUnknownInterface* QUnknownInterface::queryInterface( const QGuid& request )

  Returns an interface that matches \a request, or NULL if this interface 
  can't provide the requested interface.
*/

/*!
  \fn unsigned long QUnknownInterface::addRef()

  Increases the reference counter for this interface by one and returns
  the old reference count.
  This function is called automatically when this interface is returned
  as a result of a queryInterface() call.

  \sa release()
*/

/*!
  \fn unsigned long  QUnknownInterface::release()

  Decreases the reference counter for this interface by one and returns
  the new reference count.
  The interface should delete itself as soon as the reference counter reaches
  null.

  \sa addRef()
*/

/*!
  \class QComponentInterface qcomponentinterface.h

  \brief This interface provides functions to get information about components.
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

#endif // QT_NO_COMPONENT
