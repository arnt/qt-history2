#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qwidget.h"

/*!
  \class QAccessibleInterface qaccessible.h
  \brief The QAccessibleInterface class is an interface that exposes information about accessible objects.
*/

/*!
  \enum QAccessible::State
  This enum type defines bitflags that can be combined to indicate the state of the accessible object.
  
  \value Disabled
*/

/*!
  \enum QAccessible::Reason
  This enum type defines reasons why the state of the accessible object has changed.

  \value Focus
*/

/*!
  \enum QAccessible::Role
*/

/*!
  \fn QRect QAccessibleInterface::location() const

  Returns the object's current location in screen coordinates. All visual object
  provide this information.
*/

/*!
  \fn QAccessible::State QAccessibleInterface::state() const

  Returns the current state of this object. All objects have a state.

  \sa role()
*/

/*!
  \fn QAccessible::Role QAccessibleInterface::role() const

  Returns the role of the object. All objects have a role.

  \sa state()
*/

/*!
  \fn QString QAccessibleInterface::name() const

  Returns the current name of this object.

  The \e name is a string used by clients to identify, 
  find or announce an accessible object for the user.

  All object have a name that has to be unique within their
  container.

  \sa description(), help()
*/

/*!
  \fn QString QAccessibleInterface::description() const

  Returns the current description text of this object.

  An accessible object's \e description provides a textual description about
  an object's visual appearance. The description is primarily used to provide
  greater context for low-vision or blind users, but is also used for context
  searching or other applications.

  Not all objects have a description. An "Ok" button would not need a description, 
  but a toolbutton that shows a picture of a smiley would.

  \sa name(), help()
*/

/*!
  \fn QString QAccessibleInterface::help() const

  Returns the current help text of this object.

  The \e help text provides information about the function of an 
  accessible object. Not all objects provide this information.

  \sa name(), description()
*/

/*!
  \fn QString QAccessibleInterface::value() const

  Returns the current value of this object.

  The \e value of an accessible object represents visual information
  contained by the object, e.g. the text in a line edit. Usually, this
  value can be modified by the user.

  Not all objects have a value, e.g. static text labels don't.

  \sa name(), state()
*/

/*!
  \fn QString QAccessibleInterface::defaultAction() const
  Returns the current default action of this object.

  An accessible object's \e defaultAction describes the object's primary
  method of manipulation, and should be a verb or a short phrase, e.g. 
  "Press" for a button.

  \sa help()
*/

/*!
  \fn QString QAccessibleInterface::accelerator() const

  Returns the current keyboard shortcut for this object.

  A keyboard shortcut is an underlined character in the text of a menu, menu item 
  or control, and is either the character itself, or a combination of this character
  and a modifier key like ALT, CTRL or SHIFT.

  Command controls like tool buttons also have shortcut keys and usually display them
  in their tooltip.

  All objects that have a shortcut should provide this information.

  \sa help()
*/


/*!
  \class QAccessibleObject qaccessible.h
  \brief The QAccessibleObject class implements the QAccessibleInterface.
*/

QAccessibleObject::QAccessibleObject()
{
}

QAccessibleObject::~QAccessibleObject()
{
}

void QAccessibleObject::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = NULL;
    if ( uuid == IID_QAccessible )
	*iface = (QAccessibleInterface*)this;
    else if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    
    if ( *iface )
	(*iface)->addRef();
    return;
}

ulong QAccessibleObject::addRef()
{
    return ++ref;
}

ulong QAccessibleObject::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

/*!
  \class QAccessibleWidget qaccessible.h
  \brief The QAccessibleWidget class implements the QAccessibleInterface for QWidgets.
*/

QAccessibleWidget::QAccessibleWidget( QWidget *w )
: QAccessibleObject(), widget( w )
{
}

QRect	QAccessibleWidget::location( int who ) const
{
    QPoint wpos = widget->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), widget->width(), widget->height() );
}

bool	QAccessibleWidget::doDefaultAction( int who )
{
    return FALSE;
}

QString	QAccessibleWidget::defaultAction( int who ) const
{
    return "Ignore";
}

QString	QAccessibleWidget::description( int who ) const
{
    return widget->className();
}

QString	QAccessibleWidget::help( int who ) const
{
    return QString::null;
}

QString	QAccessibleWidget::accelerator( int who ) const
{
    return QString::null;
}

QString	QAccessibleWidget::name( int who ) const
{
    return "UNNAMED";
}

QString	QAccessibleWidget::value( int who ) const
{
    return QString::null;
}

QAccessible::Role	QAccessibleWidget::role( int who ) const
{
    return QAccessible::Client;
}

QAccessible::State	QAccessibleWidget::state( int who ) const
{
    return QAccessible::Invisible;
}

#endif
