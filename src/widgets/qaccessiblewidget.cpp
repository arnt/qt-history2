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

/*!
  \class QAccessibleWidget qaccessiblewidget.h
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
	return 0;

    QPoint rp = widget_->mapFromGlobal( QPoint( x, y ) );
    QWidget *w = widget_->childAt( rp, TRUE );

    QAccessibleInterface *cacc = w->accessibleInterface();
    if ( cacc )
	return cacc;
    return widget_->accessibleInterface();
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
	    return o ? o->accessibleInterface() : 0;
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
		return sib->accessibleInterface();
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
	return widget_->accessibleInterface();

    QObjectList *cl = widget_->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    QObject *o = cl->at( who-1 );
    delete cl;

    if ( !o )
	return 0;

    return o->accessibleInterface();    
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
    return p->accessibleInterface();
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
	return widget_->accessibleInterface();
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
	return w->accessibleInterface();

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
    ((QButton*)widget())->animateClick();
    
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
	QString text = ((QButton*)widget())->text();
    
	for ( uint i = 0; i < text.length(); i++ ) {
	    if ( text[(int)i] == '&' )
		text.remove( i, 1 );
	}
    
	return text;
    }
    return n;
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
  \class QAccessibleRangeControl qaccessiblewidget.h
  \brief The QAccessibleRangeControl class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleRangeControl object.
*/
QAccessibleRangeControl::QAccessibleRangeControl( QWidget *w, Role role, QString name, 
						 QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( w, role, name, description, QString::null, help, defAction, accelerator )
{
}

QString QAccessibleRangeControl::value( int who ) const
{
    QString v = QAccessibleWidget::value( who );
    if ( v.isNull() ) {
	if ( widget()->inherits( "QSlider" ) ) {
	    QSlider *s = (QSlider*)widget();
	    return QString::number( s->value() );
	} else if ( widget()->inherits( "QDial" ) ) {
	    QDial *d = (QDial*)widget();
	    return QString::number( d->value() );
	} else if ( widget()->inherits( "QSpinBox" ) ) {
	    QSpinBox *s = (QSpinBox*)widget();
	    return s->text();
	} else if ( widget()->inherits( "QScrollBar" ) ) {
	    QScrollBar *s = (QScrollBar*)widget();
	    return QString::number( s->value() );
	} else if ( widget()->inherits( "QProgressBar" ) ) {
	    QProgressBar *p = (QProgressBar*)widget();
	    return QString::number( p->progress() );
	}
    }

    return v;
}

/*!
  \class QAccessibleText qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for widgets with editable text.
*/

QAccessibleText::QAccessibleText( QWidget *w, Role role, QString name, QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( w, role, name, description, QString::null, help, defAction, accelerator )
{
}

QString QAccessibleText::value( int who ) const
{
    QString v = QAccessibleWidget::value( who );
    if ( v.isNull() ) {
	if ( widget()->inherits( "QLineEdit" ) ) {
	    QLineEdit *l = (QLineEdit*)widget();
	    return l->text();
	}
    }

    return v;
}

/*!
  \class QAccessibleDisplay qaccessiblewidget.h
  \brief The QAccessibleDisplay class implements the QAccessibleInterface for widgets that display static information.
*/

QAccessibleDisplay::QAccessibleDisplay( QWidget *w, Role role, QString description, QString value, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( w, role, QString::null, description, value, help, defAction, accelerator )
{
}

QAccessible::Role QAccessibleDisplay::role( int who ) const
{
    if ( widget()->inherits( "QLabel" ) ) {
	QLabel *l = (QLabel*)widget();
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

QString QAccessibleDisplay::name( int who ) const
{
    QString n = QAccessibleWidget::name( who );
    if ( n.isNull() ) {
	if ( widget()->inherits( "QLabel" ) ) {
	    QLabel *l = (QLabel*)widget();
	    return l->text();
	} else if ( widget()->inherits( "QLCDNumber" ) ) {
	    QLCDNumber *l = (QLCDNumber*)widget();
	    if ( l->numDigits() )
		return QString::number( l->value() );
	    return QString::number( l->intValue() );
	} else if ( widget()->inherits( "QGroupBox" ) ) {
	    QGroupBox *g = (QGroupBox*)widget();
	    return g->title();
	}
    }

    return n;
}

#endif
