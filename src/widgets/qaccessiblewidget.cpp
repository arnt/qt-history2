#include "qaccessiblewidget.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qobjectlist.h"
#include "qbutton.h"
#include "qslider.h"
#include "qdial.h"
#include "qspinbox.h"
#include "qscrollbar.h"
#include "qlineedit.h"
#include "qlabel.h"
#include "qlcdnumber.h"
#include "qprogressbar.h"
#include "qgroupbox.h"
#include "qtoolbutton.h"

/*!
  \class QAccessibleWidget qaccessiblewidget.h
  \brief The QAccessibleWidget class implements the QAccessibleInterface for QWidgets.
*/

/*!
  Creates a QAccessibleWidget object for the widget \a w. \a role, \a name, \a description,
  \a value, \a help, \a defAction, \a accelerator and \a state are optional parameters for static values
  of the object's property.
*/
QAccessibleWidget::QAccessibleWidget( QWidget *w, Role role, QString name, 
    QString description, QString value, QString help, QString defAction, QString accelerator, State state )
    : QAccessibleObject( w ), role_(role), name_(name), 
      description_(description),value_(value),help_(help), 
      defAction_(defAction), accelerator_(accelerator), state_(state)
{
}

/*! 
  \reimp

  For widgets with subelements, e.g. item views, this function has to be reimplemented.
*/
QAccessibleInterface* QAccessibleWidget::hitTest( int x, int y, int *who ) const
{
    *who = 0;
    QWidget *widget = (QWidget*)object();
    QPoint gp = widget->mapToGlobal( QPoint( 0, 0 ) );
    if ( !QRect( gp.x(), gp.y(), widget->width(), widget->height() ).contains( x, y ) )
	return 0;

    QPoint rp = widget->mapFromGlobal( QPoint( x, y ) );
    QWidget *w = widget->childAt( rp, TRUE );

    QAccessibleInterface *cacc = QAccessible::accessibleInterface( w );
    if ( cacc )
	return cacc;
    return QAccessible::accessibleInterface( widget );
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
    QWidget *widget = (QWidget*)object();
    QPoint wpos = widget->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), widget->width(), widget->height() );
}

/*!
  \reimp

  For widgets with subelements, e.g. item views, this function has to be reimplemented.
*/
QAccessibleInterface *QAccessibleWidget::navigate( NavDirection dir, int *target ) const
{
    QWidget *widget = (QWidget*)object();
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
	    return QAccessible::accessibleInterface( o );
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
		return QAccessible::accessibleInterface( sib );
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
  Returns the number of all child widgets. For widgets with subelements, 
  e.g. item views, this function has to be reimplemented.
*/
int QAccessibleWidget::childCount() const
{
    QObjectList *cl = object()->queryList( "QWidget", 0, FALSE, FALSE );
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
	return QAccessible::accessibleInterface( object() );

    QObjectList *cl = object()->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    QObject *o = cl->at( who-1 );
    delete cl;

    if ( !o )
	return 0;

    return QAccessible::accessibleInterface( o );    
}

/*!
  \reimp

  Returns the parent widget.
*/
QAccessibleInterface *QAccessibleWidget::parent() const
{
    QWidget *w = (QWidget*)object();
    w = w->parentWidget();
    return QAccessible::accessibleInterface( w );
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
*/
QString	QAccessibleWidget::description( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::description: This implementation does not support subelements!" );
#endif
    return description_;
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

  If the widget is a top level widget and no name has been set in the constructor, 
  the widget's caption() is returned.

  \a QWidget::caption
*/
QString	QAccessibleWidget::name( int who ) const
{
    Q_UNUSED(who)
#if defined(QT_DEBUG)
    if ( who )
	qWarning( "QAccessibleWidget::name: This implementation does not support subelements!" );
#endif
    QWidget *widget = (QWidget*)object();
    if ( name_.isNull() && widget->isTopLevel() )
	return widget->caption();
    return name_;
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

    if ( state_ != Normal )
	return state_;

    int state = Normal;

    QWidget *widget = (QWidget*)object();
    if ( widget->isHidden() )
	state |= Invisible;
    if ( widget->focusPolicy() != QWidget::NoFocus && widget->isActiveWindow() )
	state |= Focusable;
    if ( widget->hasFocus() )
	state |= Focused;
    if ( !widget->isEnabled() )
	state |= Unavailable;
    if ( widget->isTopLevel() ) {
	state |= Moveable;
	if ( widget->minimumSize() != widget->maximumSize() )
	    state |= Sizeable;
    }   

    return (State)state;
}

/*!
  \reimp
*/
QAccessibleInterface *QAccessibleWidget::hasFocus( int *who ) const
{
    QWidget* widget = (QWidget*)object();
    if ( !widget->isActiveWindow() )
	return 0;

    if ( widget->hasFocus() ) {
	*who = 0;
	return QAccessible::accessibleInterface( widget );
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
	return QAccessible::accessibleInterface( w );

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
		    QString::null, QString::null, QString::null )
{
}

/*!
  \reimp

  Reimplemented to press the button.

  \sa QButton::animateClick()
*/
bool	QAccessibleButton::doDefaultAction( int /*who*/ )
{
    ((QButton*)object())->animateClick();
    
    return TRUE;
}

/*!
  \reimp
*/
QString QAccessibleButton::defaultAction( int who ) const
{
    QString da = QAccessibleWidget::defaultAction( who );
    if ( da.isNull() )
	return QButton::tr("Press");
    return da;
}

/*!
  \reimp

  If available, returns the first character in the button's text
  that is marked as the accelerator with an ampersand.

  \sa QButton::text
*/
QString	QAccessibleButton::accelerator( int who ) const
{
    QString acc = QAccessibleWidget::accelerator( who );
    if ( acc.isNull() ) {
	QString text = ((QButton*)object())->text();

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
    }
    return acc;
}

/*!
  \reimp

  Returns the text of the button.

  \sa QButton::text
*/
QString	QAccessibleButton::name( int who ) const
{
    QString n = QAccessibleWidget::name( who );
    if ( n.isNull() ) {
	n = ((QButton*)object())->text();
    
	for ( uint i = 0; i < n.length(); i++ ) {
	    if ( n[(int)i] == '&' )
		n.remove( i, 1 );
	}
    }
    if ( n.isNull() && object()->inherits("QToolButton") )
	n = ((QToolButton*)object())->textLabel();

    return n;
}

/*!
  \reimp

  Adds states "Checked", "Mixed" or "Pressed" to the widget's state.

  \sa QButton::isToggleButton, QButton::isOn
*/
QAccessible::State QAccessibleButton::state( int who ) const
{
    int state = QAccessibleWidget::state( who );

    QButton *b = (QButton*)object();
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
  \class QAccessibleRangeControl qaccessiblewidget.h
  \brief The QAccessibleRangeControl class implements the QAccessibleInterface for range controls.
*/

/*! 
  Constructs a QAccessibleRangeControl object for the widget \a w. \a role, \a name, \a description,
  \a help, \a defAction and \a accelerator are propagated to the QAccessibleWidget constructor.
*/
QAccessibleRangeControl::QAccessibleRangeControl( QWidget *w, Role role, QString name, 
						 QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( w, role, name, description, QString::null, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleRangeControl::value( int who ) const
{
    QString v = QAccessibleWidget::value( who );
    if ( v.isNull() ) {
	if ( object()->inherits( "QSlider" ) ) {
	    QSlider *s = (QSlider*)object();
	    return QString::number( s->value() );
	} else if ( object()->inherits( "QDial" ) ) {
	    QDial *d = (QDial*)object();
	    return QString::number( d->value() );
	} else if ( object()->inherits( "QSpinBox" ) ) {
	    QSpinBox *s = (QSpinBox*)object();
	    return s->text();
	} else if ( object()->inherits( "QScrollBar" ) ) {
	    QScrollBar *s = (QScrollBar*)object();
	    return QString::number( s->value() );
	} else if ( object()->inherits( "QProgressBar" ) ) {
	    QProgressBar *p = (QProgressBar*)object();
	    return QString::number( p->progress() );
	}
    }

    return v;
}

/*!
  \class QAccessibleText qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for widgets with editable text.
*/

/*! 
  Constructs a QAccessibleText object for the widget \a w. \a role, \a name, \a description,
  \a help, \a defAction and \a accelerator are propagated to the QAccessibleWidget constructor.
*/
QAccessibleText::QAccessibleText( QWidget *w, Role role, QString name, QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( w, role, name, description, QString::null, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleText::value( int who ) const
{
    QString v = QAccessibleWidget::value( who );
    if ( v.isNull() ) {
	if ( object()->inherits( "QLineEdit" ) ) {
	    QLineEdit *l = (QLineEdit*)object();
	    return l->text();
	}
    }

    return v;
}

/*! \reimp */
QAccessible::State QAccessibleText::state( int who ) const
{
    int state = QAccessibleWidget::state( who );
    
    if ( object()->inherits( "QLineEdit" ) ) {
	QLineEdit *l = (QLineEdit*)object();
	if ( l->isReadOnly() )
	    state |= ReadOnly;
	if ( l->echoMode() == QLineEdit::Password )
	    state |= Protected;
	state |= Selectable;
	if ( l->hasSelectedText() )
	    state |= Selected;
    }

    return (State)state;
}

/*!
  \class QAccessibleDisplay qaccessiblewidget.h
  \brief The QAccessibleDisplay class implements the QAccessibleInterface for widgets that display static information.
*/

/*! 
  Constructs a QAccessibleDisplay object for the widget \a w. \a role, \a description,
  \a value, \a help, \a defAction and \a accelerator are propagated to the QAccessibleWidget constructor.
*/
QAccessibleDisplay::QAccessibleDisplay( QWidget *w, Role role, QString description, QString value, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( w, role, QString::null, description, value, help, defAction, accelerator )
{
}

/*! \reimp */
QAccessible::Role QAccessibleDisplay::role( int who ) const
{
    if ( object()->inherits( "QLabel" ) ) {
	QLabel *l = (QLabel*)object();
	if ( l->pixmap() || l->picture() )
	    return Graphic;
#ifndef QT_NO_PICTURE
	if ( l->picture() )
	    return Graphic;
#endif
#ifndef QT_NO_MOVIE
	if ( l->movie() )
	    return Animation;
#endif
    }
    return QAccessibleWidget::role( who );
}

/*! \reimp */
QString QAccessibleDisplay::name( int who ) const
{
    QString n = QAccessibleWidget::name( who );
    if ( n.isNull() ) {
	if ( object()->inherits( "QLabel" ) ) {
	    QLabel *l = (QLabel*)object();
	    return l->text();
	} else if ( object()->inherits( "QLCDNumber" ) ) {
	    QLCDNumber *l = (QLCDNumber*)object();
	    if ( l->numDigits() )
		return QString::number( l->value() );
	    return QString::number( l->intValue() );
	} else if ( object()->inherits( "QGroupBox" ) ) {
	    QGroupBox *g = (QGroupBox*)object();
	    return g->title();
	}
    }

    return n;
}

#endif
