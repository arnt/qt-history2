#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qobjectlist.h"
#include "qbutton.h"

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
    : QAccessibleObject(), widget_( w ), role_(role), name_(name), 
      description_(description),value_(value),help_(help), 
      defAction_(defAction), accelerator_(accelerator)
{
}

QWidget *QAccessibleWidget::widget() const
{
    return widget_;
}

QAccessibleInterface* QAccessibleWidget::hitTest( int x, int y, int *who ) const
{
    *who = -1;
    QPoint gp = widget_->mapToGlobal( QPoint( 0, 0 ) );
    if ( !QRect( gp.x(), gp.y(), widget_->width(), widget_->height() ).contains( x, y ) )
	return NULL;

    QPoint rp = widget_->mapFromGlobal( QPoint( x, y ) );
    QWidget *w = widget_->childAt( rp, TRUE );

    QAccessibleInterface *cacc = w->accessibilityInterface();
    if ( cacc )
	return cacc;
    return widget_->accessibilityInterface();
}

QRect	QAccessibleWidget::location( int who ) const
{
    QPoint wpos = widget_->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), widget_->width(), widget_->height() );
}

QAccessibleInterface *QAccessibleWidget::navigate( int dir, int *target ) const
{
    *target = -1;
    QObject *o = 0;
    switch ( dir ) {
    case NavFirstChild:
    case NavLastChild:
	{
	    QObjectList *cl = widget_->queryList( "QWidget", 0, FALSE, FALSE );
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
	    QWidget *parent = widget_->parentWidget();
	    QObjectList *sl = parent ? parent->queryList( "QWidget", 0, FALSE, FALSE ) : 0;
	    if ( !sl )
		return 0;
	    QObject *sib;
	    QObjectListIt it( *sl );
	    if ( dir == NavNext ) {
		while ( ( sib = it.current() ) ) {
		    ++it;
		    if ( sib == widget_ )
			break;
		}
	    } else {
		it.toLast();
		while ( ( sib = it.current() ) ) {
		    --it;
		    if ( sib == widget_ )
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
    QObjectList *cl = widget_->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    int count = cl->count();
    delete cl;
    return count;
}

QAccessibleInterface *QAccessibleWidget::child( int who ) const
{
    if ( !who )
	return widget_->accessibilityInterface();

    QObjectList *cl = widget_->queryList( "QWidget", 0, FALSE, FALSE );
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
    QWidget *p = widget_->parentWidget();

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
    return !!description_ ? description_ : widget_->className();
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
    if ( widget_->isTopLevel() )
	return widget_->caption();
#if defined(QT_DEBUG)
    return !!name_ ? name_ : widget_->name();
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

    if ( widget_->isHidden() )
	state |= QAccessible::Invisible;
    if ( widget_->focusPolicy() != QWidget::NoFocus )
	state |= QAccessible::Focusable;
    if ( widget_->hasFocus() )
	state |= QAccessible::Focused;
    if ( !widget_->isEnabled() )
	state |= QAccessible::Unavailable;

    return (State)state;
}

QAccessibleInterface *QAccessibleWidget::hasFocus( int *who ) const
{
    widget_->setActiveWindow();
    if ( !widget_->isActiveWindow() )
	return 0;

    if ( widget_->hasFocus() ) {
	*who = 0;
	return widget_->accessibilityInterface();
    }

    QWidget *w = qApp->focusWidget();
    if ( !w )
	return 0;

    // find out if we are the parent of the focusWidget
    QWidget *p = w;
    while ( p = p->parentWidget() ) {
	if ( p == widget_ )
	    break;
    }
    if ( p )
	return w->accessibilityInterface();

    // we don't know the focusWidget
    return 0;
}

/*!
  \class QAccessibleButton qaccessible.h
  \brief The QAccessibleButton class provides accessibility information for button type widgets.
*/

QAccessibleButton::QAccessibleButton( QButton *b, Role r, QString description,
				     QString help )
: QAccessibleWidget( b, r, QString::null, description, QString::null, 
			   QString::null, "Press", QString::null )
{
}

bool	QAccessibleButton::doDefaultAction( int /*who*/ )
{
    ((QButton*)widget())->animateClick();
    
    return TRUE;
}

QString	QAccessibleButton::accelerator( int who ) const
{
    QString text = ((QButton*)widget())->text();
    int fa = 0;
    bool ac = FALSE;
    while ( ( fa = text.find( "&", fa ) ) != -1 ) {
	if ( text.at(fa+1) != '&' ) {
	    ac = TRUE;
	    break;
	}
    }
    if ( fa != -1 && ac )
	return "ALT+"+text.at(fa + 1);
    return QAccessibleWidget::accelerator( who );
}

QString	QAccessibleButton::name( int /*who*/ ) const
{
    QString text = ((QButton*)widget())->text();
    
    for ( uint i = 0; i < text.length(); i++ ) {
	if ( text[(int)i] == '&' )
	    text.remove( i, 1 );
    }
    
    return text;
}

QAccessible::State QAccessibleButton::state( int who ) const
{
    int state = QAccessibleWidget::state( who );

    QButton *b = (QButton*)widget();
    if ( b->inherits( "QCheckBox" ) ) {
	if ( b->isOn() )
	    state |= Checked;
    } else if ( b->inherits( "QRadioButton" ) ) {
	if ( b->isOn() )
	    state |= Checked;
    } else if ( b->isToggleButton() && b->isOn() ) {
	state |= Pressed;
    }
    
    return (State)state;
}

#endif
