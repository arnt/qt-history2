#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

/*!
  \class QAccessible qaccessible.h
  \brief The QAccessible class provides information about accessible objects.
*/

/*!
  \enum QAccessible::State
  This enum type defines bitflags that can be combined to indicate the state of an accessible object.
  
  \value Disabled
*/

/*!
  \enum QAccessible::Reason
  This enum type defines reasons why the state of an accessible object has changed.

  \value Focus
*/

/*!
  Creates a QAccessible event. 
*/
QAccessible::QAccessible()
{
}

#ifndef Q_WS_WIN

void QAccessible::notify( QObject *o, Reason reason )
{
}

#endif


/*!
  Sets the current state for this object to \a state.

  \a state can be any combination of

  \sa state()
*/
void QAccessible::setState( QAccessible::State state )
{
}

/*!
  Returns the current state of this object.

  \sa setState()
*/
QAccessible::State QAccessible::state() const
{
    return state_;
}

/*!
  Sets the current name for this object to \a name.

  The \e name is a string used by clients to identify, 
  find or announce an accessible object for the user.

  \sa name(), setDescription(), setHelp()
*/
void QAccessible::setName( const QString &name )
{
    name_ = name;
    
}

/*!
  Returns the current name of this object.

  \sa setName(), description(), help()
*/
QString QAccessible::name() const
{
    return name_;
}

/*!
  Sets the current description text for this object to \a desc.

  An accessible object's \e description provides a textual description about
  an object's visual appearance. The description is primarily used to provide
  greater context for low-vision or blind users, but is also used for context
  searching or other applications.

  An "Ok" button would not need a description, but a toolbutton that shows a
  picture of a smiley would.

  \sa description(), name(), help()
*/
void QAccessible::setDescription( const QString &desc )
{
    descr_ = desc;
}

/*!
  Returns the current description text of this object.

  \sa setDescription(), name(), help()
*/
QString QAccessible::description() const
{
    return descr_;
}

/*!
  Sets the current help text for this object to \a help.

  The \e help text provides information that tells the user about
  the function of an accessible object.

  \sa help(), setName(), setDescription()
*/
void QAccessible::setHelp( const QString &help )
{
    help_ = help;
}

/*!
  Returns the current help text of this object.

  \sa setHelp(), name(), description()
*/
QString QAccessible::help() const
{
    return help_;
}

/*!
  Sets the current value for this object to \a value.

  The \e value of an accessible object represents visual information
  contained by the object, e.g. the text in a line edit.

  \sa value(), setState()
*/
void QAccessible::setValue( const QString &value )
{
    value_ = value;
}

/*!
  Returns the current value of this object.

  \sa setValue(), state()
*/
QString QAccessible::value() const
{
    return value_;
}

/*!
  Sets the current default action for this object to \a def.

  An accessible object's \e defaultAction describes the object's primary
  method of manipulation, and should be a verb or a short phrase.

  \sa defaultAction(), setHelp()
*/
void QAccessible::setDefaultAction( const QString &def )
{
    default_ = def;
}

/*!
  Returns the current default action of this object.

  \sa setDefaultAction(), help()
*/
QString QAccessible::defaultAction() const
{
    return default_;
}

#endif
