#include "qapplicationinterface.h"

#ifndef QT_NO_PLUGIN

/*!
  \class QApplicationInterface qapplicationinterface.h

  \brief This class provides an interface to give runtime access to application components.

  \sa QPlugInInterface
*/

/*!
  \fn QApplicationInterface::QApplicationInterface( QObject* object )

  Creates an QApplicationInterface that will provide an interface to the application component
  \a object. As the interface depends on the passed object it gets deleted when the object gets
  destroyed. 
  It's not valid to pass null for the same reason.
*/
QApplicationInterface::QApplicationInterface( QObject* o )
    : QObject( o )
{
#ifdef CHECK_RANGE
    if ( !o )
	qWarning( "Can't create interface with null-object!" );
#endif CHECK_RANGE
}

/*!
  \fn QObject* QApplicationInterface::parent()
  \reimp

  This function is made protected to prevent uncontrolled access to the handled object.
*/

#ifndef QT_NO_PROPERTIES

/*!
  \fn QVariant QApplicationInterface::requestProperty( const QCString& p )

  This function is supposed to return the value of the property \a p of the object.
  Reimplement this function for advanced processing.

  The default implementation returns the \a value of the property of the handled object.
*/
QVariant QApplicationInterface::requestProperty( const QCString& p )
{
    return parent()->property( p );
}
/*!
  \fn void QApplicationInterface::requestSetProperty( const QCString& p, const QVariant& value )

  This function is supposed to change the value of the property \a p of the object to \a value and
  return TRUE if the operation was successful, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation sets the property \a p of the handled object to \a value.
*/
bool QApplicationInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    return parent()->setProperty( p, v );
}

#endif

/*!
  \fn bool QApplicationInterface::requestConnect( const char* signal, QObject* target, const char* slot )

  This function can be used to connect the \a signal of the handled object to the \a slot of the \a target.
  It should returns TRUE if the connection was made successfully, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation connects the \a signal of the handled object to the \a slot of \a target and
  returns the result.
*/
bool QApplicationInterface::requestConnect( const char* signal, QObject* target, const char* slot )
{
    return connect( parent(), signal, target, slot );
}

/*!
  \fn bool QApplicationInterface::requestEvents( QObject* f )

  This function can be used by the plugin to have an event filter \e f installed for the handled object.
  It should returns TRUE if the eventfilter has been installed, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation installes the event filter on for the handled object and returns TRUE.
*/
bool QApplicationInterface::requestEvents( QObject* f )
{
    parent()->installEventFilter( f );

    return TRUE;
}

/*!
  \fn QApplicationInterface* QApplicationInterface::requestInterface( const QCString& request )

  Returns an interface that matches the \a request.
  This function can be used to provide interfaces to sub-compontents of the object this application 
  interface represents. E.g. a menubar may be considered to be a sub-component of the application's
  mainwindow, and therefore the mainwindow interface can give access to the menubar by providing
  an interface.
*/
QApplicationInterface* QApplicationInterface::requestInterface( const QCString& /*request*/ )
{
    return 0;
}

#endif
