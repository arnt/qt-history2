#include "qaccessiblewidget.h"

#include <qapplication.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qdial.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qgroupbox.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qscrollview.h>
#include <qheader.h>
#include <qrangecontrol.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qiconview.h>
#include <qtextedit.h>

static QString buddyString( QWidget *widget ) 
{
    QWidget *parent = widget->parentWidget();
    QObjectList *ol = parent->queryList( "QLabel", 0, FALSE, FALSE );
    if ( !ol || !ol->count() ) {
	delete ol;
	return QString::null;
    }

    QString str;

    QObjectListIt it(*ol);
    while ( it.current() ) {
	QLabel *label = (QLabel*)it.current();
	++it;
	if ( label->buddy() == widget ) {
	    str = label->text();
	    break;
	}
    }
    delete ol;
    if ( !!str )
	return str;

    if ( parent->inherits( "QGroupBox" ) )
	return ((QGroupBox*)parent)->title();

    return QString::null;
}

static QString stripAmp( const QString &text )
{
    QString n = text;
    for ( uint i = 0; i < n.length(); i++ ) {
	if ( n[(int)i] == '&' )
	    n.remove( i, 1 );
    }
    return n;
}

static QString hotKey( const QString &text )
{
    QString n = text;
    int fa = 0;
    bool ac = FALSE;
    while ( ( fa = n.find( "&", fa ) ) != -1 ) {
	if ( n.at(fa+1) != '&' ) {
	    ac = TRUE;
	    break;
	}
    }
    if ( fa != -1 && ac )
	return "ALT+"+n.at(fa + 1);

    return QString::null;
}

/*!
  \class QAccessibleWidget qaccessiblewidget.h
  \brief The QAccessibleWidget class implements the QAccessibleInterface for QWidgets.
  \preliminary
*/

/*!
  Creates a QAccessibleWidget object for \a o. 
  \a role, \a name, \a description, \a value, \a help, \a defAction, 
  \a accelerator and \a state are optional parameters for static values
  of the object's property.
*/
QAccessibleWidget::QAccessibleWidget( QObject *o, Role role, QString name, 
    QString description, QString value, QString help, QString defAction, QString accelerator, State state )
    : QAccessibleObject( o ), role_(role), name_(name), 
      description_(description),value_(value),help_(help), 
      defAction_(defAction), accelerator_(accelerator), state_(state)
{
}

/*! Returns the widget. */
QWidget *QAccessibleWidget::widget() const
{
    Q_ASSERT(object()->isWidgetType());
    if ( !object()->isWidgetType() )
	return 0;
    return (QWidget*)object();
}

/*! \reimp */
int QAccessibleWidget::controlAt( int x, int y ) const
{
    QWidget *w = widget();
    QPoint gp = w->mapToGlobal( QPoint( 0, 0 ) );
    if ( !QRect( gp.x(), gp.y(), w->width(), w->height() ).contains( x, y ) )
	return -1;

    QPoint rp = w->mapFromGlobal( QPoint( x, y ) );

    QObjectList *list = w->queryList( "QWidget", 0, FALSE, FALSE ); 

    if ( !list || list->isEmpty() )
	return 0;

    QObjectListIt it( *list );
    QWidget *child = 0;
    int index = 1;
    while ( ( child = (QWidget*)it.current() ) ) {
	if ( !child->isTopLevel() && !child->isHidden() && child->geometry().contains( rp ) ) {
	    delete list;
	    return index;
	}
	++it;
	++index;
    }
    delete list;
    return 0;
}

/*! \reimp */
QRect	QAccessibleWidget::rect( int control ) const
{
#if defined(QT_DEBUG)
    if ( control )
	qWarning( "QAccessibleWidget::rect: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );
#else
    Q_UNUSED(control)
#endif
    QWidget *w = widget();
    QPoint wpos = w->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), w->width(), w->height() );
}

/*! \reimp */
int QAccessibleWidget::navigate( NavDirection dir, int startControl ) const
{
#if defined(QT_DEBUG)
    if ( startControl )
	qWarning( "QAccessibleWidget::navigate: This implementation does not support subelements! (ID %d unknown for %s)", startControl, widget()->className() );
#else
    Q_UNUSED(startControl);
#endif
    QWidget *w = widget();
    QObject *o = 0;
    switch ( dir ) {
    case NavFirstChild:
	{
	    QObjectList *list = widget()->queryList( "QWidget", 0, FALSE, FALSE );
	    bool has = !list->isEmpty();
	    delete list;
	    return has ? 1 : -1;
	}
    case NavLastChild:
	{
	    QObjectList *list = widget()->queryList( "QWidget", 0, FALSE, FALSE );
	    bool has = !list->isEmpty();
	    delete list;
	    return has ? childCount() : -1;
	}
    case NavNext:
    case NavPrevious:
	{
	    QWidget *parent = w->parentWidget();
	    QObjectList *sl = parent ? parent->queryList( "QWidget", 0, FALSE, FALSE ) : 0;
	    if ( !sl )
		return 0;
	    QObject *sib;
	    QObjectListIt it( *sl );
	    int index;
	    if ( dir == NavNext ) {
		index = 1;
		while ( ( sib = it.current() ) ) {
		    ++it;
		    ++index;
		    if ( sib == w )
			break;
		}
	    } else {
		it.toLast();
		index = sl->count();
		while ( ( sib = it.current() ) ) {
		    --it;
		    --index;
		    if ( sib == w )
			break;
		}
	    }
	    sib = it.current();
	    delete sl;
	    if ( sib )
		return index;
	    return -1;
	}
	break;
    case NavFocusChild:
	{
	    if ( w->hasFocus() )
		return 0;

	    QWidget *w2 = w->focusWidget();
	    if ( !w2 )
		return -1;

	    QObjectList *list = w->queryList( "QWidget", 0, FALSE, FALSE );
	    int index = list->findRef( w2 );
	    delete list;
	    return ( index != -1 ) ? index+1 : -1;
	}	
    default:
	qWarning( "QAccessibleWidget::navigate: unhandled request" );
	break;
    };
    return -1;
}

/*! \reimp */
int QAccessibleWidget::childCount() const
{
    QObjectList *cl = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    int count = cl->count();
    delete cl;
    return count;
}

/*! \reimp */
QRESULT QAccessibleWidget::queryChild( int control, QAccessibleInterface **iface ) const
{
    *iface = 0;
    QObjectList *cl = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return;

    QObject *o = 0;
    if ( cl->count() >= (uint)control ) 
	o = cl->at( control-1 );
    delete cl;

    if ( !o )
	return;

    QAccessible::queryAccessibleInterface( o, iface );
    return;
}

/*! \reimp */
QRESULT QAccessibleWidget::queryParent( QAccessibleInterface **iface ) const
{
    QAccessible::queryAccessibleInterface( widget()->parentWidget(), iface );
    return;
}

/*! \reimp */
bool QAccessibleWidget::doDefaultAction( int control )
{
#if defined(QT_DEBUG)
    if ( control )
	qWarning( "QAccessibleWidget::doDefaultAction: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );
#else
    Q_UNUSED(control)
#endif
    return FALSE;
}

/*! \reimp */
QString QAccessibleWidget::text( Text t, int control ) const
{
    switch ( t ) {
    case DefaultAction:
	return defAction_;
    case Description:
	if ( !control && description_.isNull() ) {
	    QString desc = QToolTip::textFor( widget() );
	    return desc;
	}
	return description_;
    case Help:
	if ( !control && help_.isNull() ) {
	    QString help = QWhatsThis::textFor( widget() );
	    return help;
	}
	return help_;
    case Accelerator:
	return accelerator_;
    case Name:
	{
	    if ( !control && name_.isNull() && widget()->isTopLevel() )
		return widget()->caption();
	    return name_;
	}
    case Value:
	return value_;
    default:
	break;
    }
    return QString::null;
}

/*! \reimp */
QAccessible::Role QAccessibleWidget::role( int control ) const
{
    if ( !control )
	return role_;
    return NoRole;
}

/*! \reimp */
QAccessible::State QAccessibleWidget::state( int control ) const
{
    if ( control )
	return Normal;

    if ( state_ != Normal )
	return state_;

    int state = Normal;

    QWidget *w = widget();
    if ( w->isHidden() )
	state |= Invisible;
    if ( w->focusPolicy() != QWidget::NoFocus && w->isActiveWindow() )
	state |= Focusable;
    if ( w->hasFocus() )
	state |= Focused;
    if ( !w->isEnabled() )
	state |= Unavailable;
    if ( w->isTopLevel() ) {
	state |= Moveable;
	if ( w->minimumSize() != w->maximumSize() )
	    state |= Sizeable;
    }   

    return (State)state;
}

/*! \reimp */
bool QAccessibleWidget::setFocus( int control )
{
#if defined(QT_DEBUG)
    if ( control )
	qWarning( "QAccessibleWidget::setFocus: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );
#else
    Q_UNUSED(control)
#endif
    if ( widget()->focusPolicy() != QWidget::NoFocus ) {
	widget()->setFocus();
	return TRUE;
    }
    return FALSE;
}

/*! \reimp */
bool QAccessibleWidget::setSelected( int, bool, bool )
{
#if defined(QT_DEBUG)
    qWarning( "QAccessibleWidget::setSelected: This function not supported for simple widgets." );
#endif
    return FALSE;
}

/*! \reimp */
void QAccessibleWidget::clearSelection()
{
#if defined(QT_DEBUG)
    qWarning( "QAccessibleWidget::clearSelection: This function not supported for simple widgets." );
#endif
}

/*! \reimp */
QMemArray<int> QAccessibleWidget::selection() const
{
    return QMemArray<int>();
}

/*!
  \class QAccessibleButton qaccessible.h
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
  \preliminary
*/

/*!
  Creates a QAccessibleButton object for \a o.
  \a role, \a description and \a help are propagated to the QAccessibleWidget constructor.
*/
QAccessibleButton::QAccessibleButton( QObject *o, Role role, QString description,
				     QString help )
: QAccessibleWidget( o, role, QString::null, description, QString::null, 
		    QString::null, QString::null, QString::null )
{
}

/*! \reimp */
bool	QAccessibleButton::doDefaultAction( int /*control*/ )
{
    ((QButton*)widget())->animateClick();
    
    return TRUE;
}

/*! \reimp */
QString QAccessibleButton::text( Text t, int control ) const
{
    QString tx = QAccessibleWidget::text( t, control );
    if ( !!tx )
	return tx;

    switch ( t ) {
    case DefaultAction:
	    return QButton::tr("Press");
    case Accelerator:
	tx = hotKey( ((QButton*)widget())->text() );
	if ( tx.isNull() )
	    tx = hotKey( buddyString( widget() ) );
	return tx;
    case Name:
	tx = ((QButton*)widget())->text();
	if ( tx.isNull() && widget()->inherits("QToolButton") )
	    tx = ((QToolButton*)widget())->textLabel();
	if ( tx.isNull() )
	    tx = buddyString( widget() );

	return stripAmp( tx );
    default:
	break;
    }
    return tx;
}

/*! \reimp */
QAccessible::State QAccessibleButton::state( int control ) const
{
    int state = QAccessibleWidget::state( control );

    QButton *b = (QButton*)widget();
    if ( b->state() == QButton::On )
	state |= Checked;
    else  if ( b->state() == QButton::NoChange )
	    state |= Mixed;
    if ( b->isDown() )
	state |= Pressed;
    if ( b->inherits( "QPushButton" ) ) {
	QPushButton *pb = (QPushButton*)b;
	if ( pb->isDefault() )
	    state |= Default;
    } 
    
    return (State)state;
}

/*! 
  \class QAccessibleRangeControl qaccessiblewidget.h
  \brief The QAccessibleRangeControl class implements the QAccessibleInterface for range controls.
  \preliminary
*/

/*! 
  Constructs a QAccessibleRangeControl object for \a o. 
  \a role, \a name, \a description, \a help, \a defAction and \a accelerator 
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleRangeControl::QAccessibleRangeControl( QObject *o, Role role, QString name, 
						 QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( o, role, name, description, QString::null, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleRangeControl::text( Text t, int control ) const
{
    QString tx = QAccessibleWidget::text( t, control );
    if ( !!tx )
	return stripAmp(tx);

    switch ( t ) {
    case Name:
	return stripAmp( buddyString( widget() ) );
    case Accelerator:
	return hotKey( buddyString( widget() ) );
    case Value:
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
    default:
	break;
    }
    return tx;
}


/*!
  \class QAccessibleSpinWidget qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for up/down widgets.
  \preliminary
*/

/*! 
  Constructs a QAccessibleSpinWidget object for \a o.
*/
QAccessibleSpinWidget::QAccessibleSpinWidget( QObject *o )
: QAccessibleRangeControl( o, SpinBox )
{
}

/*! \reimp */
int QAccessibleSpinWidget::controlAt( int x, int y ) const
{
    QPoint tl = widget()->mapFromGlobal( QPoint( x, y ) );
    if ( ((QSpinWidget*)widget())->upRect().contains( tl ) )
	return 1;
    else if ( ((QSpinWidget*)widget())->downRect().contains( tl ) )
	return 2;

    return -1;
}

/*! \reimp */
QRect QAccessibleSpinWidget::rect( int control ) const
{
    QRect rect;
    switch( control ) {
    case 1:
	rect = ((QSpinWidget*)widget())->upRect();
	break;
    case 2:
	rect = ((QSpinWidget*)widget())->downRect();
	break;
    default:
	rect = widget()->rect();
    }
    QPoint tl = widget()->mapToGlobal( QPoint( 0, 0 ) );
    return QRect( tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height() );
}

/*! \reimp */
int QAccessibleSpinWidget::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return 2;
	break;
    case NavNext:
    case NavDown:
	startControl += 1;
	if ( startControl > 2 )
	    return -1;
	return startControl;
    case NavPrevious:
    case NavUp:
	startControl -= 1;
	if ( startControl < 1 )
	    return -1;
	return startControl;
    default:
	break;
    }

    return -1;
}

/*! \reimp */
int QAccessibleSpinWidget::childCount() const
{
    return 2;
}

/*! \reimp */
QRESULT QAccessibleSpinWidget::queryChild( int control, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return;
}

/*! \reimp */
QString QAccessibleSpinWidget::text( Text t, int control ) const
{
    switch ( t ) {
    case Name:
	switch ( control ) {
	case 1:
	    return QSpinWidget::tr("More");
	case 2:
	    return QSpinWidget::tr("Less");
	default:
	    break;
	}
	break;
    case DefaultAction:
	switch( control ) {
	case 1:
	case 2:
	    return QSpinWidget::tr("Press");
	default:
	    break;
	}
	break;
    default:
	break;
    }
    return QAccessibleRangeControl::text( t, control );;
}

/*! \reimp */
QAccessible::Role QAccessibleSpinWidget::role( int control ) const
{
    switch( control ) {
    case 1:
	return PushButton;
    case 2:
	return PushButton;
    default:
	break;
    }
    return QAccessibleRangeControl::role( control );
}

/*! \reimp */
QAccessible::State QAccessibleSpinWidget::state( int control ) const
{
    int state = QAccessibleRangeControl::state( control );
    switch( control ) {
    case 1:
	if ( !((QSpinWidget*)widget())->isUpEnabled() )
	    state |= Unavailable;
	return (State)state;
    case 2:
	if ( !((QSpinWidget*)widget())->isDownEnabled() )
	    state |= Unavailable;
	return (State)state;
    default:
	break;
    }
    return QAccessibleRangeControl::state( control );
}

/*! \reimp */
bool QAccessibleSpinWidget::doDefaultAction( int control )
{
    switch( control ) {
    case 1:
	((QSpinWidget*)widget())->stepUp();
	return TRUE;
    case 2:
	((QSpinWidget*)widget())->stepDown();
	return TRUE;
    default:
	break;
    }
    return QAccessibleRangeControl::doDefaultAction( control );
}

/*!
  \class QAccessibleText qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for widgets with editable text.
  \preliminary
*/

/*! 
  Constructs a QAccessibleText object for \a o. 
  \a role, \a name, \a description, \a help, \a defAction and \a accelerator 
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleText::QAccessibleText( QObject *o, Role role, QString name, QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( o, role, name, description, QString::null, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleText::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;
    switch ( t ) {
    case Name:
	return stripAmp( buddyString( widget() ) );
    case Accelerator:
	return hotKey( buddyString( widget() ) );
    case Value:
	if ( widget()->inherits( "QLineEdit" ) )
	    return ((QLineEdit*)widget())->text();
	break;
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::State QAccessibleText::state( int control ) const
{
    int state = QAccessibleWidget::state( control );
    
    if ( widget()->inherits( "QLineEdit" ) ) {
	QLineEdit *l = (QLineEdit*)widget();
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
  \preliminary
*/

/*! 
  Constructs a QAccessibleDisplay object for \a o. 
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator 
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleDisplay::QAccessibleDisplay( QObject *o, Role role, QString description, QString value, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( o, role, QString::null, description, value, help, defAction, accelerator )
{
}

/*! \reimp */
QAccessible::Role QAccessibleDisplay::role( int control ) const
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
    return QAccessibleWidget::role( control );
}

/*! \reimp */
QString QAccessibleDisplay::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;

    switch ( t ) {
    case Name:
	if ( widget()->inherits( "QLabel" ) ) {
	    return stripAmp( ((QLabel*)widget())->text() );
	} else if ( widget()->inherits( "QLCDNumber" ) ) {
	    QLCDNumber *l = (QLCDNumber*)widget();
	    if ( l->numDigits() )
		return QString::number( l->value() );
	    return QString::number( l->intValue() );
	} else if ( widget()->inherits( "QGroupBox" ) ) {
	    return stripAmp( ((QGroupBox*)widget())->title() );
	}
	break;
    default:
	break;
    }
    return str;
}


/*! 
  \class QAccessibleScrollView qaccessiblewidget.h
  \brief The QAccessibleScrollView class implements the QAccessibleInterface for scrolled widgets.
  \preliminary
*/

/*! 
  Constructs a QAccessibleHeader object for \a o. 
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator 
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleHeader::QAccessibleHeader( QObject *o, QString description, 
    QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleWidget( o, NoRole, description, value, help, defAction, accelerator )
{
    Q_ASSERT(widget()->inherits("QHeader"));
}

/*! Returns the QHeader. */
QHeader *QAccessibleHeader::header() const
{
    return (QHeader *)widget();
}

/*! \reimp */
int QAccessibleHeader::controlAt( int x, int y ) const
{
    QPoint point = header()->mapFromGlobal( QPoint( x, y ) );
    for ( int i = 0; i < header()->count(); i++ ) {
	if ( header()->sectionRect( i ).contains( point ) )
	    return i+1;
    }
    return -1;
}

/*! \reimp */
QRect QAccessibleHeader::rect( int control ) const
{
    QPoint zero = header()->mapToGlobal( QPoint ( 0,0 ) );
    QRect sect = header()->sectionRect( control - 1 );
    return QRect( sect.x() + zero.x(), sect.y() + zero.y(), sect.width(), sect.height() );
}

/*! \reimp */
int QAccessibleHeader::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    int count = header()->count();
    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return count;
    case NavNext:
	return startControl + 1 > count ? -1 : startControl + 1;
    case NavPrevious:
	return startControl - 1 < 1 ? -1 : startControl - 1;
    case NavUp:
	if ( header()->orientation() == Vertical )
	    return startControl - 1 < 1 ? -1 : startControl - 1;
	return -1;
    case NavDown:
	if ( header()->orientation() == Vertical )
	    return startControl + 1 > count ? -1 : startControl + 1;
	break;
    case NavLeft:
	if ( header()->orientation() == Horizontal )
	    return startControl - 1 < 1 ? -1 : startControl - 1;
	break;
    case NavRight:
	if ( header()->orientation() == Horizontal )
	    return startControl + 1 > count ? -1 : startControl + 1;
	break;
    default:
	break;
    }
    return -1;
}

/*! \reimp */
int QAccessibleHeader::childCount() const
{
    return header()->count();
}

/*! \reimp */
QRESULT QAccessibleHeader::queryChild( int control, QAccessibleInterface **iface ) const
{
    return;
}

/*! \reimp */
QString QAccessibleHeader::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;

    switch ( t ) {
    case Name:
	return header()->label( control - 1 );
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleHeader::role( int control ) const
{
    if ( header()->orientation() == Qt::Horizontal )
	return ColumnHeader;
    else
	return RowHeader;
}

/*! \reimp */
QAccessible::State QAccessibleHeader::state( int control ) const
{
    return QAccessibleWidget::state( control );
}

/*! 
  \class QAccessibleViewport qaccessiblewidget.h
  \brief The QAccessibleViewport class hides the viewport of scrollviews for accessibility.
  \internal
*/

QAccessibleViewport::QAccessibleViewport( QObject *o, QObject *sv )
    : QAccessibleWidget( o )
{
    Q_ASSERT( sv->inherits("QScrollView") );
    scrollview = (QScrollView*)sv;
}

QAccessibleScrollView *QAccessibleViewport::scrollView() const
{
    QAccessibleInterface *iface = 0;
    queryAccessibleInterface( scrollview, &iface );
    Q_ASSERT(iface);
    return (QAccessibleScrollView *)iface;
}

int QAccessibleViewport::controlAt( int x, int y ) const
{
    int control = QAccessibleWidget::controlAt( x, y );
    if ( control > 0 )
	return control;

    QPoint p = widget()->mapFromGlobal( QPoint( x,y ) );
    return scrollView()->itemHitTest( p.x(), p.y() );
}

QRect QAccessibleViewport::rect( int control ) const
{
    if ( !control )
	return QAccessibleWidget::rect( control );
    QRect rect = scrollView()->itemLocation( control );
    QPoint tl = widget()->mapToGlobal( QPoint( 0,0 ) );
    return QRect( tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height() );
}

int QAccessibleViewport::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    // ### call itemUp/Down etc. here
    const int items = scrollView()->itemCount();
    switch( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return items;
    case NavNext:
    case NavDown:
	return startControl + 1 > items ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
	return startControl - 1 < 1 ? -1 : startControl - 1;
    default:
	break;
    }

    return -1;
}

int QAccessibleViewport::childCount() const
{
    int widgets = QAccessibleWidget::childCount();
    return widgets ? widgets : scrollView()->itemCount();
}

QString QAccessibleViewport::text( Text t, int control ) const
{
    return scrollView()->text( t, control );
}

bool QAccessibleViewport::doDefaultAction( int control )
{
    return scrollView()->doDefaultAction( control );
}

QAccessible::Role QAccessibleViewport::role( int control ) const
{
    return scrollView()->role( control );
}

QAccessible::State QAccessibleViewport::state( int control ) const
{
    return scrollView()->state( control );
}

bool QAccessibleViewport::setFocus( int control )
{
    return scrollView()->setFocus( control );
}

bool QAccessibleViewport::setSelected( int control, bool on, bool extend )
{
    return scrollView()->setSelected( control, on, extend );
}

void QAccessibleViewport::clearSelection()
{
    scrollView()->clearSelection();
}

QMemArray<int> QAccessibleViewport::selection() const
{
    return scrollView()->selection();
}

/*! 
  \class QAccessibleScrollView qaccessiblewidget.h
  \brief The QAccessibleScrollView class implements the QAccessibleInterface for scrolled widgets.
  \preliminary
*/

/*! 
  Constructs a QAccessibleScrollView object for \a o. 
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator 
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollView::QAccessibleScrollView( QObject *o, Role role, QString name, 
    QString description, QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleWidget( o, role, name, description, value, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleScrollView::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;
    switch ( t ) {
    case Name:
	return buddyString( widget() );
    default:
	break;
    }
	
    return str;
}

/*!
  Returns the ID of the item at viewport position \a x, \a y.
*/
int QAccessibleScrollView::itemHitTest( int x, int y ) const
{
    return 0;
}

/*!
  Returns the location of the item with ID \a item in viewport coordinates.
*/
QRect QAccessibleScrollView::itemLocation( int item ) const
{
    return QRect();
}

/*!
  Returns the number of items.
*/
int QAccessibleScrollView::itemCount() const
{
    return 0;
}

/*! 
  \class QAccessibleListBox qaccessiblewidget.h
  \brief The QAccessibleListBox class implements the QAccessibleInterface for list boxes.
  \preliminary
*/

/*! 
  Constructs a QAccessibleListBox object for \a o. 
*/
QAccessibleListBox::QAccessibleListBox( QObject *o )
    : QAccessibleScrollView( o, List )
{
    Q_ASSERT(widget()->inherits("QListBox"));
}

/*! Returns the list box. */
QListBox *QAccessibleListBox::listBox() const
{    
    return (QListBox*)widget();
}

/*! \reimp */
int QAccessibleListBox::itemHitTest( int x, int y ) const
{
    QListBoxItem *item = listBox()->itemAt( QPoint( x, y ) );
    return listBox()->index( item ) + 1;
}

/*! \reimp */
QRect QAccessibleListBox::itemLocation( int item ) const
{
    return listBox()->itemRect( listBox()->item( item-1 ) );
}

/*! \reimp */
int QAccessibleListBox::itemCount() const
{
    return listBox()->count();
}

/*! \reimp */
QString QAccessibleListBox::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );

    QListBoxItem *item = listBox()->item( control - 1 );
    if ( item )
	return item->text();
    return QString::null;
}

/*! \reimp */
QAccessible::Role QAccessibleListBox::role( int control ) const
{
    if ( !control )
	return QAccessibleScrollView::role( control );
    return ListItem;
}

/*! \reimp */
QAccessible::State QAccessibleListBox::state( int control ) const
{
    int state = QAccessibleScrollView::state( control );
    QListBoxItem *item;
    if ( !control || !( item = listBox()->item( control - 1 ) ) )
	return (State)state;

    if ( item->isSelectable() ) {
	if ( listBox()->selectionMode() == QListBox::Multi )
	    state |= MultiSelectable;
	else if ( listBox()->selectionMode() == QListBox::Extended )
	    state |= ExtSelectable;
	else if ( listBox()->selectionMode() == QListBox::Single )
	    state |= Selectable;
	if ( item->selected() )
	    state |= Selected;
    }
    if ( listBox()->focusPolicy() != QWidget::NoFocus ) {
	state |= Focusable;
	if ( item->current() )
	    state |= Focused;
    }
    if ( !listBox()->itemVisible( item ) )
	state |= Invisible;

    return (State)state;
}

/*! \reimp */
bool QAccessibleListBox::setFocus( int control )
{
    bool res = QAccessibleScrollView::setFocus( 0 );
    if ( !control || !res )
	return res;

    QListBoxItem *item = listBox()->item( control -1 );
    if ( !item )
	return FALSE;
    listBox()->setCurrentItem( item );
    return TRUE;
}

/*! \reimp */
bool QAccessibleListBox::setSelected( int control, bool on, bool extend )
{
    if ( !control || ( extend && 
	listBox()->selectionMode() != QListBox::Extended && 
	listBox()->selectionMode() != QListBox::Multi ) )
	return FALSE;

    QListBoxItem *item = listBox()->item( control -1 );
    if ( !item )
	return FALSE;
    if ( !extend ) {
	listBox()->setSelected( item, on );
    } else {
	int current = listBox()->currentItem();
	bool down = control > current;
	for ( int i = current; i != control;) {
	    down ? i++ : i--;
	    listBox()->setSelected( i, on );
	}

    }
    return TRUE;
}

/*! \reimp */
void QAccessibleListBox::clearSelection()
{
    listBox()->clearSelection();
}

/*! \reimp */
QMemArray<int> QAccessibleListBox::selection() const
{
    QMemArray<int> array;
    uint size = 0;
    const uint c = listBox()->count();
    array.resize( c );
    for ( uint i = 0; i < c; ++i ) {
	if ( listBox()->isSelected( i ) ) {
	    ++size;
	    array[ (int)size-1 ] = i+1;
	}
    }
    array.resize( size );
    return array;
}

/*! 
  \class QAccessibleListView qaccessiblewidget.h
  \brief The QAccessibleListView class implements the QAccessibleInterface for list views.
  \preliminary
*/

static QListViewItem *findLVItem( QListView* listView, int control )
{
    int id = 1;
    QListViewItemIterator it( listView );
    QListViewItem *item = it.current();
    while ( item && id < control ) {
	++it;
	++id;
	item = it.current();
    }
    return item;
}

/*! 
  Constructs a QAccessibleListView object for \a o. 
*/
QAccessibleListView::QAccessibleListView( QObject *o )
    : QAccessibleScrollView( o, Outline )
{
}

/*! Returns the list view. */
QListView *QAccessibleListView::listView() const
{
    Q_ASSERT(widget()->inherits("QListView"));
    return (QListView*)widget();
}

/*! \reimp */
int QAccessibleListView::itemHitTest( int x, int y ) const
{
    QListViewItem *item = listView()->itemAt( QPoint( x, y ) );
    if ( !item )
	return 0;

    QListViewItemIterator it( listView() );
    int c = 1;
    while ( it.current() ) {
	if ( it.current() == item )
	    return c;
	++c;
	++it;
    }
    return 0;
}

/*! \reimp */
QRect QAccessibleListView::itemLocation( int control ) const
{
    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return QRect();
    return listView()->itemRect( item );
}

/*! \reimp */
int QAccessibleListView::itemCount() const
{
    QListViewItemIterator it( listView() );
    int c = 0;
    while ( it.current() ) {
	++c;
	++it;
    }

    return c;
}

/*! \reimp */
QString QAccessibleListView::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );

    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return QString::null;
    return item->text( 0 );
}

/*! \reimp */
QAccessible::Role QAccessibleListView::role( int control ) const
{
    if ( !control )
	return QAccessibleScrollView::role( control );
    return OutlineItem;
}

/*! \reimp */
QAccessible::State QAccessibleListView::state( int control ) const
{
    int state = QAccessibleScrollView::state( control );
    QListViewItem *item;
    if ( !control || !( item = findLVItem( listView(), control ) ) )
	return (State)state;

    if ( item->isSelectable() ) {
	if ( listView()->selectionMode() == QListView::Multi )
	    state |= MultiSelectable;
	else if ( listView()->selectionMode() == QListView::Extended )
	    state |= ExtSelectable;
	else if ( listView()->selectionMode() == QListView::Single )
	    state |= Selectable;
	if ( item->isSelected() )
	    state |= Selected;
    }
    if ( listView()->focusPolicy() != QWidget::NoFocus ) {
	state |= Focusable;
	if ( item == listView()->currentItem() )
	    state |= Focused;
    }
    if ( item->childCount() ) {
	if ( item->isOpen() )
	    state |= Expanded;
	else
	    state |= Collapsed;
    }
    if ( !listView()->itemRect( item ).isValid() )
	state |= Invisible;

    if ( item->rtti() == QCheckListItem::RTTI ) {
	if ( ((QCheckListItem*)item)->isOn() )
	    state|=Checked;
    }
    return (State)state;
}

/*! \reimp
QAccessibleInterface *QAccessibleListView::focusChild( int *control ) const
{
    QListViewItem *item = listView()->currentItem();
    if ( !item )
	return 0;

    QListViewItemIterator it( listView() );
    int c = 1;
    while ( it.current() ) {
	if ( it.current() == item ) {
	    *control = c;
	    return (QAccessibleInterface*)this;
	}
	++c;
	++it;
    }
    return 0;
}
*/
/*! \reimp */
bool QAccessibleListView::setFocus( int control )
{
    bool res = QAccessibleScrollView::setFocus( 0 );
    if ( !control || !res )
	return res;

    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return FALSE;
    listView()->setCurrentItem( item );
    return TRUE;
}

/*! \reimp */
bool QAccessibleListView::setSelected( int control, bool on, bool extend )
{
    if ( !control || ( extend && 
	listView()->selectionMode() != QListView::Extended && 
	listView()->selectionMode() != QListView::Multi ) )
	return FALSE;

    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return FALSE;
    if ( !extend ) {
	listView()->setSelected( item, on );
    } else {
	QListViewItem *current = listView()->currentItem();
	if ( !current )
	    return FALSE;
	bool down = item->itemPos() > current->itemPos();
	QListViewItemIterator it( current );
	while ( it.current() ) {
	    listView()->setSelected( it.current(), on );
	    if ( it.current() == item )
		break;
	    if ( down )
		++it;
	    else
		--it;
	}
    }
    return TRUE;
}

/*! \reimp */
void QAccessibleListView::clearSelection()
{
    listView()->clearSelection();
}

/*! \reimp */
QMemArray<int> QAccessibleListView::selection() const
{
    QMemArray<int> array;
    uint size = 0;
    int id = 1;
    array.resize( size );
    QListViewItemIterator it( listView() );
    while ( it.current() ) {
	if ( it.current()->isSelected() ) {
	    ++size;
	    array.resize( size );
	    array[ (int)size-1 ] = id;
	}
	++it;
	++id;
    }
    return array;
}

/*! 
  \class QAccessibleIconView qaccessiblewidget.h
  \brief The QAccessibleIconView class implements the QAccessibleInterface for icon views.
  \preliminary
*/

static QIconViewItem *findIVItem( QIconView *iconView, int control )
{
    int id = 1;
    QIconViewItem *item = iconView->firstItem();
    while ( item && id < control ) {
	item = item->nextItem();
	++id;
    }

    return item;
}

/*! 
  Constructs a QAccessibleIconView object for \a o. 
*/
QAccessibleIconView::QAccessibleIconView( QObject *o )
    : QAccessibleScrollView( o, Outline )
{
    Q_ASSERT(widget()->inherits("QIconView"));
}

/*! Returns the icon view. */
QIconView *QAccessibleIconView::iconView() const
{
    return (QIconView*)widget();
}

/*! \reimp */
int QAccessibleIconView::itemHitTest( int x, int y ) const
{
    QIconViewItem *item = iconView()->findItem( QPoint( x, y ) );
    return iconView()->index( item ) + 1;
}

/*! \reimp */
QRect QAccessibleIconView::itemLocation( int control ) const
{
    QIconViewItem *item = findIVItem( iconView(), control );

    if ( !item )
	return QRect();
    return item->rect();
}

/*! \reimp */
int QAccessibleIconView::itemCount() const
{
    return iconView()->count();
}

/*! \reimp */
QString QAccessibleIconView::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );

    QIconViewItem *item = findIVItem( iconView(), control );
    if ( !item )
	return QString::null;
    return item->text();
}

/*! \reimp */
QAccessible::Role QAccessibleIconView::role( int control ) const
{
    if ( !control )
	return QAccessibleScrollView::role( control );
    return OutlineItem;
}

/*! \reimp */
QAccessible::State QAccessibleIconView::state( int control ) const
{
    int state = QAccessibleScrollView::state( control );
    QIconViewItem *item;
    if ( !control || !( item = findIVItem( iconView(), control ) ) )
	return (State)state;

    if ( item->isSelectable() ) {
	if ( iconView()->selectionMode() == QIconView::Multi )
	    state |= MultiSelectable;
	else if ( iconView()->selectionMode() == QIconView::Extended )
	    state |= ExtSelectable;
	else if ( iconView()->selectionMode() == QIconView::Single )
	    state |= Selectable;
	if ( item->isSelected() )
	    state |= Selected;
    }
    if ( iconView()->itemsMovable() )
	state |= Moveable;
    if ( iconView()->focusPolicy() != QWidget::NoFocus ) {
	state |= Focusable;
	if ( item == iconView()->currentItem() )
	    state |= Focused;
    }

    return (State)state;
}

/*! \reimp 
QAccessibleInterface *QAccessibleIconView::focusChild( int *control ) const
{
    QIconViewItem *item = iconView()->currentItem();
    if ( !item )
	return 0;

    *control = iconView()->index( item );
    return (QAccessibleInterface*)this;
}
*/
/*! \reimp */
bool QAccessibleIconView::setFocus( int control )
{
    bool res = QAccessibleScrollView::setFocus( 0 );
    if ( !control || !res )
	return res;

    QIconViewItem *item = findIVItem( iconView(), control );
    if ( !item )
	return FALSE;
    iconView()->setCurrentItem( item );
    return TRUE;
}

/*! \reimp */
bool QAccessibleIconView::setSelected( int control, bool on, bool extend  )
{
    if ( !control || ( extend && 
	iconView()->selectionMode() != QIconView::Extended && 
	iconView()->selectionMode() != QIconView::Multi ) )
	return FALSE;

    QIconViewItem *item = findIVItem( iconView(), control );
    if ( !item )
	return FALSE;
    if ( !extend ) {
	iconView()->setSelected( item, on, TRUE );
    } else {
	QIconViewItem *current = iconView()->currentItem();
	if ( !current )
	    return FALSE;
	bool down = FALSE;
	QIconViewItem *temp = current;
	while ( ( temp = temp->nextItem() ) ) {
	    if ( temp == item ) {
		down = TRUE;
		break;
	    }
	}
	temp = current;
	if ( down ) {
	    while ( ( temp = temp->nextItem() ) ) {
		iconView()->setSelected( temp, on, TRUE );
		if ( temp == item )
		    break;
	    }
	} else {
	    while ( ( temp = temp->prevItem() ) ) {
		iconView()->setSelected( temp, on, TRUE );
		if ( temp == item )
		    break;
	    }
	}
    }
    return TRUE;
}

/*! \reimp */
void QAccessibleIconView::clearSelection()
{
    iconView()->clearSelection();
}

/*! \reimp */
QMemArray<int> QAccessibleIconView::selection() const
{
    QMemArray<int> array;
    uint size = 0;
    int id = 1;
    array.resize( iconView()->count() );
    QIconViewItem *item = iconView()->firstItem();
    while ( item ) {
	if ( item->isSelected() ) {
	    ++size;
	    array[ (int)size-1 ] = id;
	}
	item = item->nextItem();
	++id;
    }
    array.resize( size );
    return array;
}


/*! 
  \class QAccessibleTextEdit qaccessiblewidget.h
  \brief The QAccessibleTextEdit class implements the QAccessibleInterface for richtext editors.
  \preliminary
*/

/*!
  Constructs a QAccessibleTextEdit object for \a o.
*/
QAccessibleTextEdit::QAccessibleTextEdit( QObject *o )
: QAccessibleScrollView( o, Pane )
{
    Q_ASSERT(widget()->inherits("QTextEdit"));
}

/*! Returns the text edit. */
QTextEdit *QAccessibleTextEdit::textEdit() const
{

    return (QTextEdit*)widget();
}

/*! \reimp */
int QAccessibleTextEdit::itemHitTest( int x, int y ) const
{
    int p;
    QPoint cp = textEdit()->viewportToContents( QPoint( x,y ) );
    textEdit()->charAt( cp , &p );
    return p + 1;
}

/*! \reimp */
QRect QAccessibleTextEdit::itemLocation( int item ) const
{
    QRect rect = textEdit()->paragraphRect( item - 1 );
    if ( !rect.isValid() )
	return QRect();
    QPoint ntl = textEdit()->contentsToViewport( QPoint( rect.x(), rect.y() ) );
    return QRect( ntl.x(), ntl.y(), rect.width(), rect.height() );
}

/*! \reimp */
int QAccessibleTextEdit::itemCount() const
{
    return textEdit()->paragraphs();
}

/*! \reimp */
QString QAccessibleTextEdit::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );
    return textEdit()->text( control-1 );
}

/*! \reimp */
QAccessible::Role QAccessibleTextEdit::role( int control ) const
{
    if ( control )
	return EditableText;
    return QAccessibleScrollView::role( control );
}
