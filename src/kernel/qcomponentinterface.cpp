#include "qcomponentinterface.h"
#include "qapplication.h"

#ifndef QT_NO_PLUGIN

/*!
  \class QApplicationInterface qapplicationinterface.h

  \brief This class provides an interface to give runtime access to application components.

  \sa QPlugInInterface
*/

/*!
  Creates an QApplicationInterface that will provide an interface to the application component
  \a object. As the interface depends on the passed object it gets deleted when the object gets
  destroyed.
  It's not valid to pass null for the same reason.
*/
QApplicationInterface::QApplicationInterface()
    : QObject( qApp )
{
}

/*!
  \fn QComponentInterface* QApplicationInterface::queryInterface( const QCString& request )
  \overload

  Returns an interface that matches the \a request, or NULL if no such interface cn be provided.
  This function can be used to provide interfaces to sub-compontents of the object this interface
  represents.
  E.g. a menubar may be considered to be a sub-component of the application's mainwindow, and
  therefore the mainwindow interface can give access to the menubar by providing an interface.
  The interface will be deleted when the corresponding object is destroyed.
*/

/*!
  \class QApplicationComponentInterface qapplicationinterface.h

  \brief This class provides an interface to give runtime access to application components.

  \sa QPlugInInterface
*/

/*!
  Creates a QApplicationComponentInterface that provides an interface to the application component
  \a object.
  As the interface depends on the passed object it gets deleted when the object gets
  destroyed. It's not valid to pass null for the same reason.
*/
QApplicationComponentInterface::QApplicationComponentInterface( QObject* o )
{
#ifdef CHECK_RANGE
    if ( !o )
	qWarning( "Can't create interface with null-object!" );
#endif CHECK_RANGE
    if ( o )
	o->insertChild( this );
}

/*!
  \fn QObject* QComponentInterface::object()
  \reimp

  This function returns the pointer to the handled object.
*/

#ifndef QT_NO_PROPERTIES

/*!
  This function is supposed to return the value of the property \a p of the object.
  Reimplement this function for advanced processing.

  The default implementation returns the \a value of the property of the handled object.
*/
QVariant QApplicationComponentInterface::requestProperty( const QCString& p )
{
    return object()->property( p );
}
/*!
  This function is supposed to change the value of the property \a p of the object to \a value and
  return TRUE if the operation was successful, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation sets the property \a p of the handled object to \a value.
*/
bool QApplicationComponentInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    return object()->setProperty( p, v );
}

#endif

/*!
  This function can be used to connect the \a signal of the handled object to the \a slot of the \a target.
  It should returns TRUE if the connection was made successfully, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation connects the \a signal of the handled object to the \a slot of \a target and
  returns the result.
*/
bool QApplicationComponentInterface::requestConnect( const char* signal, QObject* target, const char* slot )
{
    return connect( object(), signal, target, slot );
}

/*!  This function can be used to connect the \a signal of the \a
  sender to the \a slot of the \a handled object.  It should returns
  TRUE if the connection was made successfully, otherwise FALSE.
  Reimplement this function for advanced processing.
*/

bool QApplicationComponentInterface::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    return connect( sender, signal, object(), slot );
}

/*!
  This function can be used by the plugin to have an event filter \e f installed for the handled object.
  It should returns TRUE if the eventfilter has been installed, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation installes the event filter on for the handled object and returns TRUE.
*/
bool QApplicationComponentInterface::requestEvents( QObject* f )
{
    object()->installEventFilter( f );

    return TRUE;
}

#endif
