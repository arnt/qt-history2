#include "qcomponentinterface.h"

#ifndef QT_NO_COMPONENT

/*!
  \class QUnknownInterface qcomponentinterface.h
  \brief The QUnknownInterface class serves as a base class for interfaces.
*/

/*!
  \fn QUnknownInterface* QUnknownInterface::queryInterface( const QUuid& request )

  Returns an interface that matches \a request, or NULL if this interface 
  can't provide the requested interface.

  Every component implementing this interface has to provide a proper implementation 
  of this function. 
*/

/*!
  \fn ulong QUnknownInterface::addRef()

  Increases the reference counter for this interface by one and returns
  the old reference count.
  This function is called automatically when this interface is returned
  as a result of a queryInterface() call. QRefCountInterface provides a
  standard implementation of this function.

  \sa release()
*/

/*!
  \fn ulong QUnknownInterface::release()

  Decreases the reference counter for this interface by one and returns
  the new reference count.
  The interface should delete itself as soon as the reference counter reaches
  null. QRefCountInterface provides a standard implementation of this function.

  \sa addRef()
*/

/*!
  \class QComponentInterface qcomponentinterface.h
  \brief The QComponentInterface class provides functions to get information about components.
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
  \class QRefCountInterface qcomponentinterface.h
  \brief The QRefCountInterface class provides a standard reference count implementation interfaces.
*/

/*!
  Creates a QRefCountInterface object with the refcounter being zero.
*/

QRefCountInterface::QRefCountInterface()
: ref( 0 )
{
}

/*!
  Destroys the object.
*/

QRefCountInterface::~QRefCountInterface()
{
}

/*!
  Increases the reference counter by one and returns the old value.
*/

ulong QRefCountInterface::addRef()
{
    return ref++;
}

/*!
  Decreases the reference counter by one and returns the new value. If the counter reaches zero,
  the object gets automatically destroyed.
*/

ulong QRefCountInterface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

#endif // QT_NO_COMPONENT
