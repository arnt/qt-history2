#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qobjectlist.h"
#include "qbutton.h"
#include "qrangecontrol.h"
#include "qslider.h"
#include "qdial.h"

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
  \enum QAccessible::Event
  This enum type defines event types why the state of the accessible object has changed.

  \value Focus
*/

/*!
  \enum QAccessible::Role
*/

/*!
  \enum QAccessible::NavDirection

  \value NavDirectionMin
  \value NavUp
  \value NavDown
  \value NavLeft
  \value NavRight
  \value NavNext
  \value NavPrevious
  \value NavFirstChild
  \value NavLastChild
  \value NavDirectionMax
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
  \fn QRect QAccessibleInterface::location( int who ) const

  Returns the object's current location in screen coordinates if \a who is 0,
  or the location of the object's subelement with ID \a who.
  
  All visual objects provide this information.
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::navigate( NavDirection direction, int *startEnd ) const

  This function traverses to another object. \a direction specifies in which direction 
  to navigate, and the value of \a startEnd specifies the start point of the navigation, 
  which is either 0 if the navigation starts at the object itself, or an ID of one of 
  the object's subelements.

  The function returns the QAccessibleInterface implementation of the object located at 
  the direction specified, or this QAccessibleInterface if the target object is a subelement
  of this object. \a startEnd is then set to the ID of this subelement.
*/

/*!
  \fn int QAccessibleInterface::childCount() const

  Returns the number of accessible child objects. Every subelement of this
  object that can provide accessibility information is a child, e.g. items
  in a list view.
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::child( int who ) const
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::parent() const

  Returns the QAccessibleInterface implementation of the parent object, or NULL if there
  is no such object.

  All objects provide this information.
*/

/*!
  \fn QAccessible::State QAccessibleInterface::state( int who ) const

  Returns the current state of the object if \a who is 0, or the state of 
  the object's subelement element with ID \a who. All objects have a state.

  \sa role()
*/

/*!
  \fn QAccessible::Role QAccessibleInterface::role( int who ) const

  Returns the role of the object if \a who is 0, or the role of the object's 
  subelement with ID \a who. The role of an object is usually static. 
  All accessible objects have a role.

  \sa state()
*/

/*!
  \fn QString QAccessibleInterface::name( int who ) const

  Returns the current name of the object if \a who is 0, or the name of 
  the object's subelement with ID \a who.

  The \e name is a string used by clients to identify, 
  find or announce an accessible object for the user.

  All object have a name that has to be unique within their
  container.

  \sa description(), help()
*/

/*!
  \fn QString QAccessibleInterface::description( int who ) const

  Returns the current description text of the object if \a who is 0, or the
  name of the object's subelement with ID \a who.

  An accessible object's \e description provides a textual description about
  an object's visual appearance. The description is primarily used to provide
  greater context for low-vision or blind users, but is also used for context
  searching or other applications.

  Not all objects have a description. An "Ok" button would not need a description, 
  but a toolbutton that shows a picture of a smiley would.

  \sa name(), help()
*/

/*!
  \fn QString QAccessibleInterface::help( int who ) const

  Returns the current help text of the object if \a who is 0, or the help
  text of the object's subelement with ID \a who.

  The \e help text provides information about the function of an 
  accessible object. Not all objects provide this information.

  \sa name(), description()
*/

/*!
  \fn QString QAccessibleInterface::value( int who ) const

  Returns the current value of the object if \a who is 0, or the value
  of the object's subelement with ID \a who.

  The \e value of an accessible object represents visual information
  contained by the object, e.g. the text in a line edit. Usually, the
  value can be modified by the user.

  Not all objects have a value, e.g. static text labels don't, and some
  objects have a state that already is the value, e.g. toggle buttons.

  \sa name(), state()
*/

/*!
  \fn bool QAccessibleInterface::doDefaultAction( int who )
*/

/*!
  \fn QString QAccessibleInterface::defaultAction( int who ) const

  Returns the current default action of the object if \a who is 0, or the
  value of the object's subelement with ID \a who.

  An accessible object's \e defaultAction describes the object's primary
  method of manipulation, and should be a verb or a short phrase, e.g. 
  "Press" for a button.

  \sa help()
*/

/*!
  \fn QString QAccessibleInterface::accelerator( int who ) const

  Returns the keyboard shortcut for this object if \a who is 0, or
  the keyboard shortcut of the object's subelement with ID \a who.

  A keyboard shortcut is an underlined character in the text of a menu, menu item 
  or control, and is either the character itself, or a combination of this character
  and a modifier key like ALT, CTRL or SHIFT.

  Command controls like tool buttons also have shortcut keys and usually display them
  in their tooltip.

  All objects that have a shortcut should provide this information.

  \sa help()
*/

/*!
  \fn QAccessibleInterface *QAccessibleInterface::hasFocus( int *who ) const
*/


/*!
  \class QAccessibleObject qaccessible.h
  \brief The QAccessibleObject class implements the QUnknownInterface.

  This class is mainly provided for convenience. All further implementations 
  of the QAccessibleInterface should use this class as the base class.
*/

/*!
  Creates a QAccessibleObject.
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

/*!
  Creates a QAccessibleWidget object for the widget \a w with \a role, \a name, \a description,
  \a value, \a help, \a defAction and \a accelerator being optional parameters for static values
  of the object's property.
*/
QAccessibleWidget::QAccessibleWidget( QWidget *w, Role role, QString name, 
    QString description, QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleObject(), widget_( w ), role_(role), name_(name), 
      description_(description),value_(value),help_(help), 
      defAction_(defAction), accelerator_(accelerator)
{
     if ( widget_->inherits( "QPopupMenu" ) )
	 role_ = MenuPopup;
}

/*!
  Returns the widget for which this QAccessibleInterface implementation provides information.
*/
QWidget *QAccessibleWidget::widget() const
{
    return widget_;
}

/*! 
  \reimp

  For widgets with subelements, e.g. item views, this function has to be reimplemented.
*/
QAccessibleInterface* QAccessibleWidget::hitTest( int x, int y, int *who ) const
{
    *who = 0;
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

/*!
  \reimp

  For widgets with subelements, e.g. item views, this function has to be reimplemented.
*/
QRect	QAccessibleWidget::location( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::location: This implementation does not support subelements!" );
#endif
    QPoint wpos = widget_->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), widget_->width(), widget_->height() );
}

/*!
  \reimp

  For widgets with subelements, e.g. item views, this function has to be reimplemented.
*/
QAccessibleInterface *QAccessibleWidget::navigate( NavDirection dir, int *target ) const
{
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

/*!
  \reimp

  Returns the number of all child widgets. For widgets with subelements, 
  e.g. item views, this function has to be reimplemented.
*/
int QAccessibleWidget::childCount() const
{
    QObjectList *cl = widget_->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    int count = cl->count();
    delete cl;
    return count;
}

/*!
  \reimp

  For widgets with subelements, e.g. item views, this function has to be reimplemented.
*/
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

/*!
  \reimp

  Returns the parent widget.
*/
QAccessibleInterface *QAccessibleWidget::parent() const
{
    QWidget *p = widget_->parentWidget();

    if ( !p )
	p = QApplication::desktop();
    return p->accessibilityInterface();
}

/*!
  \reimp

  Does nothing and returns FALSE.
*/
bool	QAccessibleWidget::doDefaultAction( int who )
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::doDefaultAction: This implementation does not support subelements!" );
#endif
    return FALSE;
}

/*!
  \reimp
*/
QString	QAccessibleWidget::defaultAction( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::defaultAction: This implementation does not support subelements!" );
#endif
    return defAction_;
}

/*!
  \reimp

  Returns the object's className() when no description is provided 
  and if compiled with debug symbols.

  \sa QObject::className
*/
QString	QAccessibleWidget::description( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::description: This implementation does not support subelements!" );
    return !!description_ ? description_ : widget_->className();
#else
    return description_;
#endif
}

/*!
  \reimp
*/
QString	QAccessibleWidget::help( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::help: This implementation does not support subelements!" );
#endif
    return help_;
}

/*!
  \reimp
*/
QString	QAccessibleWidget::accelerator( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::accelerator: This implementation does not support subelements!" );
#endif
    return accelerator_;
}

/*!
  \reimp

  If the widget is a top level widget, the caption() is returned.
  Returns the object's name when no  name is provided otherwise and
  if compiled with debug symbols 

  \a QWidget::caption, QObject::name
*/
QString	QAccessibleWidget::name( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::name: This implementation does not support subelements!" );
#endif
    if ( widget_->isTopLevel() )
	return widget_->caption();
#if defined(QT_DEBUG)
    return !!name_ ? name_ : widget_->name();
#else
    return name_;
#endif
}

/*!
  \reimp
*/
QString	QAccessibleWidget::value( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::value: This implementation does not support subelements!" );
#endif
    return value_;
}

/*!
  \reimp
*/
QAccessible::Role	QAccessibleWidget::role( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::role: This implementation does not support subelements!" );
#endif
    return role_;
}

/*!
  \reimp

  Sets the state flags Invisible, Focusable, Focused and Unavailable as
  appropriate. Reimplementations should call this implementation and
  use the returned value to add further flags.
*/
QAccessible::State	QAccessibleWidget::state( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::state: This implementation does not support subelements!" );
#endif

    int state = QAccessible::Normal;

    if ( widget_->isHidden() )
	state |= Invisible;
    if ( widget_->focusPolicy() != QWidget::NoFocus )
	state |= Focusable;
    if ( widget_->hasFocus() )
	state |= Focused;
    if ( !widget_->isEnabled() )
	state |= Unavailable;

    return (State)state;
}

/*!
  \reimp
*/
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
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleButton object.
  \a role, \a description and \a help are propagated to the QAccessibleWidget constructor.
  The default action is set to "Press".
*/
QAccessibleButton::QAccessibleButton( QButton *b, Role role, QString description,
				     QString help )
: QAccessibleWidget( b, role, QString::null, description, QString::null, 
		    QString::null, QApplication::tr("Press"), QString::null )
{
}

/*!
  \reimp

  Reimplemented to press the button.

  \sa QButton::animateClick()
*/
bool	QAccessibleButton::doDefaultAction( int /*who*/ )
{
    ((QButton*)widget())->animateClick();
    
    return TRUE;
}

/*!
  \reimp

  If available, returns the first character in the button's text
  that is marked as the accelerator with an ampersand.

  \sa QButton::text
*/
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

/*!
  \reimp

  Returns the text of the button.

  \sa QButton::text
*/
QString	QAccessibleButton::name( int /*who*/ ) const
{
    QString text = ((QButton*)widget())->text();
    
    for ( uint i = 0; i < text.length(); i++ ) {
	if ( text[(int)i] == '&' )
	    text.remove( i, 1 );
    }
    
    return text;
}

/*!
  \reimp

  Adds states "Checked" or "Pressed" to the widget's state.

  \sa QButton::isToggleButton, QButton::isOn
*/
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

/*! 
  \class QAccessibleRangeControl qaccessible.h
  \brief The QAccessibleRangeControl class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleRangeControl object.
*/
QAccessibleRangeControl::QAccessibleRangeControl( QWidget *w, Role role )
: QAccessibleWidget( w, role )
{
}

QString QAccessibleRangeControl::value( int who ) const
{
    if ( widget()->inherits( "QSlider" ) ) {
	QRangeControl *r = (QRangeControl*)(QSlider*)widget();
	return QString::number( r->value() );
    } else if ( widget()->inherits( "QDial" ) ) {
	QRangeControl *r = (QRangeControl*)(QDial*)widget();
	return QString::number( r->value() );
    }

    return QAccessibleWidget::value( who );
}

#endif
