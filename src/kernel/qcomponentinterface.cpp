#include "qcomponentinterface.h"
#include "qapplication.h"
#include "qdir.h"

#ifndef QT_NO_PLUGIN

/*!
  \class QUnknownInterface qcomponentinterface.h
*/

QUnknownInterface::QUnknownInterface( QUnknownInterface* parent )
{
}

QUnknownInterface::~QUnknownInterface()
{
}

QString QUnknownInterface::interfaceID() const
{ 
    return "QUnknownInterface"; 
}

bool QUnknownInterface::connectNotify( QApplicationInterface* )
{ 
    return TRUE; 
}

bool QUnknownInterface::disconnectNotify()
{ 
    return TRUE; 
}

QUnknownInterface* QUnknownInterface::queryInterface( const QString& )
{ 
    return 0; 
}

QStringList QUnknownInterface::interfaceList() const 
{ 
    return QStringList(); 
}

/*!
  \class QPlugInInterface qcomponentinterface.h
*/

QPlugInInterface::QPlugInInterface()
: QUnknownInterface()
{
}

QString QPlugInInterface::interfaceID() const 
{ 
    return "QPlugInInterface"; 
}

QString QPlugInInterface::name() const 
{ 
    return QString::null; 
}

QString QPlugInInterface::description() const 
{ 
    return QString::null; 
}

QString QPlugInInterface::author() const 
{ 
    return QString::null; 
}

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
: QUnknownInterface()
{
}

QString QApplicationInterface::interfaceID() const
{ 
    return "QApplicationInterface"; 
}

QString QApplicationInterface::name() const
{
    return QString::null;
}

QString QApplicationInterface::description() const
{
    return QString::null;
}

QString QApplicationInterface::author() const
{
    return QString::null;
}

QString QApplicationInterface::workDirectory() const
{
    return QDir::currentDirPath();
}

QString QApplicationInterface::version() const
{
    return QString::null;
}

QString QApplicationInterface::command() const
{
    return qApp->argv()[0];
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
: QUnknownInterface()
{
#ifdef CHECK_RANGE
    if ( !o )
	qWarning( "Can't create interface with null-object!" );
#endif CHECK_RANGE
    comp = o;
}

QString QApplicationComponentInterface::interfaceID() const
{
    return "QApplicationComponentInterface"; 
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
    return component()->property( p );
}
/*!
  This function is supposed to change the value of the property \a p of the object to \a value and
  return TRUE if the operation was successful, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation sets the property \a p of the handled object to \a value.
*/
bool QApplicationComponentInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    return component()->setProperty( p, v );
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
    return QObject::connect( component(), signal, target, slot );
}

/*!  This function can be used to connect the \a signal of the \a
  sender to the \a slot of the \a handled object.  It should returns
  TRUE if the connection was made successfully, otherwise FALSE.
  Reimplement this function for advanced processing.
*/

bool QApplicationComponentInterface::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    return QObject::connect( sender, signal, component(), slot );
}

/*!
  This function can be used by the plugin to have an event filter \e f installed for the handled object.
  It should returns TRUE if the eventfilter has been installed, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation installes the event filter on for the handled object and returns TRUE.
*/
bool QApplicationComponentInterface::requestEvents( QObject* f )
{
    component()->installEventFilter( f );

    return TRUE;
}

#endif
