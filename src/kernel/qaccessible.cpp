#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qwidget.h"
#include "qapplication.h"
#include "qobjectlist.h"

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

  Returns the object's current location in screen coordinates. 
  
  All visual objects provide this information.
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::hitTest( int x, int y, int *who ) const

  Returns whether the screen coordinates \a x, \a y are within the boundaries of this object.
  If the tested point is on a child of this object which implements an QAccessibilityInterface
  itself, that interface is returned. Otherwise, this interface is returned and the value of \a who
  is set to the identifier of any child element. \a who is set to 0 if the tested point is on
  the the object itself.

  This function returns NULL if the tested point is outside the boundaries of this object.

  All visual objects provide this information.
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::parent() const

  Returns the QAccessibleInterface implementation of the parent object, or NULL if there
  is no such object.

  All objects provide this information.
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

/*!
  Constructs a QAccessibleObject.
*/
QAccessibleObject::QAccessibleObject()
: ref( 0 )
{
}

/*!
  Destroys the QAccessibleObject. 
  
  This will only happen if a call to release() decrements the internal 
  reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
}

/*!
  Implements the QUnknownInterface function to return provide an interface for
  IID_QAccessible and IID_QUnknown.
*/
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

/*!
  \reimp
*/
ulong QAccessibleObject::addRef()
{
    return ++ref;
}

/*!
  \reimp
*/
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

QAccessibleWidget::QAccessibleWidget( QWidget *w, Role role, QString name, 
    QString description, QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleObject(), widget( w ), role_(role), name_(name), 
      description_(description),value_(value),help_(help), 
      defAction_(defAction), accelerator_(accelerator)
{
}

QAccessibleInterface* QAccessibleWidget::hitTest( int x, int y, int *who ) const
{
    *who = -1;
    QPoint gp = widget->mapToGlobal( QPoint( 0, 0 ) );
    if ( !QRect( gp.x(), gp.y(), widget->width(), widget->height() ).contains( x, y ) )
	return NULL;

    QPoint rp = widget->mapFromGlobal( QPoint( x, y ) );
    QWidget *w = widget->childAt( rp, TRUE );

    QAccessibleInterface *cacc = w->accessibilityInterface();
    if ( cacc )
	return cacc;
    return widget->accessibilityInterface();
}

QRect	QAccessibleWidget::location( int who ) const
{
    QPoint wpos = widget->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), widget->width(), widget->height() );
}

QAccessibleInterface *QAccessibleWidget::navigate( int dir, int *target ) const
{
    *target = -1;
    QObject *o = 0;
    switch ( dir ) {
    case NavFirstChild:
    case NavLastChild:
	{
	    QObjectList *cl = widget->queryList( "QWidget", 0, FALSE, FALSE );
	    if ( !cl )
		return 0;
	    if ( dir == NavFirstChild )
		o = cl->first();
	    else
		o = cl->last();
	    delete cl;
	    return o ? o->accessibilityInterface() : 0;
	}
	break;
    case NavNext:
    case NavPrevious:
	{
	    QWidget *parent = widget->parentWidget();
	    QObjectList *sl = parent ? parent->queryList( "QWidget", 0, FALSE, FALSE ) : 0;
	    if ( !sl )
		return 0;
	    QObject *sib;
	    QObjectListIt it( *sl );
	    if ( dir == NavNext ) {
		while ( ( sib = it.current() ) ) {
		    ++it;
		    if ( sib == widget )
			break;
		}
	    } else {
		it.toLast();
		while ( ( sib = it.current() ) ) {
		    --it;
		    if ( sib == widget )
			break;
		}
	    }
	    sib = it.current();
	    if ( sib )
		return sib->accessibilityInterface();
	    return 0;
	}
	break;
    default:
	qDebug( "QAccessibleWidget::navigate: unhandled request" );
	break;
    };
    return 0;
}

int QAccessibleWidget::childCount() const
{
    QObjectList *cl = widget->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    int count = cl->count();
    delete cl;
    return count;
}

QAccessibleInterface *QAccessibleWidget::child( int who ) const
{
    if ( !who )
	return widget->accessibilityInterface();

    QObjectList *cl = widget->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    QObject *o = cl->at( who-1 );
    delete cl;

    if ( !o )
	return 0;

    return o->accessibilityInterface();    
}

QAccessibleInterface *QAccessibleWidget::parent() const
{
    QWidget *p = widget->parentWidget();

    if ( !p )
	p = QApplication::desktop();
    return p->accessibilityInterface();
}

bool	QAccessibleWidget::doDefaultAction( int /*who*/ )
{
    return FALSE;
}

QString	QAccessibleWidget::defaultAction( int /*who*/ ) const
{
    return defAction_;
}

QString	QAccessibleWidget::description( int /*who*/ ) const
{
#if defined(QT_DEBUG)
    return !!description_ ? description_ : widget->className();
#else
    return description_;
#endif
}

QString	QAccessibleWidget::help( int /*who*/) const
{
    return help_;
}

QString	QAccessibleWidget::accelerator( int /*who*/ ) const
{
    return accelerator_;
}

QString	QAccessibleWidget::name( int /*who*/ ) const
{
    if ( widget->isTopLevel() )
	return widget->caption();
#if defined(QT_DEBUG)
    return !!name_ ? name_ : widget->name();
#else
    return name_;
#endif
}

QString	QAccessibleWidget::value( int /*who*/ ) const
{
    return value_;
}

QAccessible::Role	QAccessibleWidget::role( int /*who*/ ) const
{
    return role_;
}

QAccessible::State	QAccessibleWidget::state( int /*who*/ ) const
{
    int state = QAccessible::Normal;

    if ( widget->isHidden() )
	state |= QAccessible::Invisible;
    if ( widget->focusPolicy() != QWidget::NoFocus )
	state |= QAccessible::Focusable;
    if ( widget->hasFocus() )
	state |= QAccessible::Focused;
    if ( !widget->isEnabled() )
	state |= QAccessible::Unavailable;

    return (State)state;
}

QAccessibleInterface *QAccessibleWidget::hasFocus( int *who ) const
{
    widget->setActiveWindow();
    if ( !widget->isActiveWindow() )
	return 0;

    if ( widget->hasFocus() ) {
	*who = 0;
	return widget->accessibilityInterface();
    }

    QWidget *w = qApp->focusWidget();
    if ( !w )
	return 0;

    // find out if we are the parent of the focusWidget
    QWidget *p = w;
    while ( p = p->parentWidget() ) {
	if ( p == widget )
	    break;
    }
    if ( p )
	return w->accessibilityInterface();

    // we don't know the focusWidget
    return 0;
}

#endif
