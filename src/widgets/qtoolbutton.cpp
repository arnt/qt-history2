/****************************************************************************
** $Id: $
**
** Implementation of QToolButton class
**
** Created : 980320
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtoolbutton.h"
#ifndef QT_NO_TOOLBUTTON

#include "qdrawutil.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include "qmainwindow.h"
#include "qtooltip.h"
#include "qtoolbar.h"
#include "qimage.h"
#include "qiconset.h"
#include "qtimer.h"
#include "qpopupmenu.h"
#include "qguardedptr.h"

static QToolButton * threeDeeButton = 0;


class QToolButtonPrivate
{
    // ### add tool tip magic here
public:
    QGuardedPtr<QPopupMenu> popup;
    QTimer* popupTimer;
    int delay;
    Qt::ArrowType arrow	    : 2;
    uint instantPopup	    : 1;
    uint autoraise	    : 1;
    uint repeat		    : 1;
    uint discardNextMouseEvent : 1;
};


// NOT REVISED
/*!
  \class QToolButton qtoolbutton.h

  \brief The QToolButton class provides a quick-access button to
  specific commands or options, usually used inside a QToolBar.

  \ingroup basic

  A tool button is a special button that provides quick-access to
  specific commands or options. As opposed to a normal command button,
  a tool button usually doesn't show a text label, but an icon.  Its
  classic usage is to select tools, for example the "pen" tool in a
  drawing program. This would be implemented with a QToolButton as
  toggle button (see setToggleButton() ).

  QToolButton supports auto-raising. In auto-raise mode, the button
  draws a 3D frame only when the mouse points at it.  The feature is
  automatically turned on when a button is used inside a QToolBar.
  Change it with setAutoRaise().

  A tool button's icon is set as QIconSet. This makes it possible to
  specify different pixmaps for the disabled and active state. The
  disabled pixmap is used when the button's functionality is not
  available. The active pixmap is displayed when the button is
  auto-raised because the user is pointing at it.

  The button's look and dimension is adjustable with
  setUsesBigPixmap() and setUsesTextLabel(). When used inside a
  QToolBar, the button automatically adjusts to QMainWindow's settings
  (see QMainWindow::setUsesTextLabel() and
  QMainWindow::setUsesBigPixmaps()).

  A tool button can offer additional choices in a popup menu.  The
  feature is sometimes used with the "Back" button in a web browser.
  After pressing the button down for awhile, a menu pops up showing
  all possible pages to browse back.  With QToolButton you can set a
  popup menu using setPopup(). The default delay is 600ms; you may
  adjust it with setPopupDelay().

  \sa QPushButton QToolBar QMainWindow
  <a href="guibooks.html#fowler">GUI Design Handbook: Push Button</a>
*/


/*!
  Constructs an empty tool button with parent \a parent called \a name.
*/

QToolButton::QToolButton( QWidget * parent, const char *name )
    : QButton( parent, name )
{
    init();
    if ( parent && parent->inherits( "QToolBar" ) ) {
	setAutoRaise( TRUE );
	QToolBar* tb = (QToolBar*)parent;
	if ( tb->mainWindow() ) {
	    connect( tb->mainWindow(), SIGNAL(pixmapSizeChanged(bool)),
	             this, SLOT(setUsesBigPixmap(bool)) );
	    setUsesBigPixmap( tb->mainWindow()->usesBigPixmaps() );
	    connect( tb->mainWindow(), SIGNAL(usesTextLabelChanged(bool)),
	             this, SLOT(setUsesTextLabel(bool)) );
	    setUsesTextLabel( tb->mainWindow()->usesTextLabel() );
	} else {
	    setUsesBigPixmap( FALSE );
	}
    } else {
	setUsesBigPixmap( FALSE );
    }
}


/*!
  Constructs a tool button as arrow button. The ArrowType \a type
  defines the arrow direction. Possible values are LeftArrow,
  RightArrow, UpArrow and DownArrow.

  An arrow button has auto-repeat turned on.

  The \a parent and \a name arguments are sent to the QWidget constructor.
*/
QToolButton::QToolButton( ArrowType type, QWidget *parent, const char *name )
    : QButton( parent, name )
{
    init();
    setUsesBigPixmap( FALSE );
    setAutoRepeat( TRUE );
    d->arrow = type;
    hasArrow = TRUE;
}


/*!  Set-up code common to all the constructors */

void QToolButton::init()
{
    d = new QToolButtonPrivate;
    d->delay = 600;
    d->popup = 0;
    d->popupTimer = 0;
    d->autoraise = FALSE;
    d->arrow = LeftArrow;
    d->instantPopup = FALSE;
    d->discardNextMouseEvent = FALSE;
    bpID = bp.serialNumber();
    spID = sp.serialNumber();

    utl = FALSE;
    ubp = TRUE;
    hasArrow = FALSE;

    s = 0;

    setFocusPolicy( NoFocus );
    setBackgroundMode( PaletteButton);
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
}


/*!  Constructs a tool button that is a child of \a parent (which must be
  a QToolBar) and named \a name.

  The tool button will display \a iconSet, with text label or tool tip \a
  textLabel and status bar message \a grouptext, it will be connected to
  \a slot in object \a receiver.
*/

QToolButton::QToolButton( const QIconSet& iconSet, const QString &textLabel,
			  const QString& grouptext,
			  QObject * receiver, const char *slot,
			  QToolBar * parent, const char *name )
    : QButton( parent, name )
{
    init();
    setAutoRaise( TRUE );
    setIconSet( iconSet );
    setTextLabel( textLabel );
    if ( receiver && slot )
	connect( this, SIGNAL(clicked()), receiver, slot );
    if ( parent->mainWindow() ) {
	connect( parent->mainWindow(), SIGNAL(pixmapSizeChanged(bool)),
		 this, SLOT(setUsesBigPixmap(bool)) );
	setUsesBigPixmap( parent->mainWindow()->usesBigPixmaps() );
	connect( parent->mainWindow(), SIGNAL(usesTextLabelChanged(bool)),
		 this, SLOT(setUsesTextLabel(bool)) );
	setUsesTextLabel( parent->mainWindow()->usesTextLabel() );
    } else {
	setUsesBigPixmap( FALSE );
    }
#ifndef QT_NO_TOOLTIP
    if ( !textLabel.isEmpty() ) {
	if ( !grouptext.isEmpty() )
	    QToolTip::add( this, textLabel,
			   parent->mainWindow()->toolTipGroup(), grouptext );
	else
	    QToolTip::add( this, textLabel );
    }
#endif
}


/*! Destroys the object and frees any allocated resources. */

QToolButton::~QToolButton()
{
    d->popupTimer = 0;
    d->popup = 0;
    delete d;
    delete s;
    threeDeeButton = 0;
}


/*!
  \property QToolButton::toggleButton
  \brief whether this tool button is a toggle button.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A tool button is initially not a toggle button.

  \sa setOn(), toggle()
*/

void QToolButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*!  \reimp
*/
QSize QToolButton::sizeHint() const
{
    constPolish();

    int w = 0, h = 0;

    if ( iconSet().isNull() && !text().isNull() && !usesTextLabel() ) {
     	w = fontMetrics().width( text() );
     	h = fontMetrics().height(); // boundingRect()?
    } else if ( usesBigPixmap() ) {
     	QPixmap pm = iconSet().pixmap( QIconSet::Large, QIconSet::Normal );
     	w = pm.width();
     	h = pm.height();
     	if ( w < 32 )
     	    w = 32;
     	if ( h < 32 )
     	    h = 32;
    } else {
     	QPixmap pm = iconSet().pixmap( QIconSet::Small, QIconSet::Normal );
     	w = pm.width();
     	h = pm.height();
	if ( w < 16 )
	    w = 16;
	if ( h < 16 )
	    h = 16;
    }

    if ( usesTextLabel() ) {
     	h += 4 + fontMetrics().height();
     	int tw = fontMetrics().width( textLabel() ) + fontMetrics().width("  ");
     	if ( tw > w )
     	    w = tw;
    }

    if ( popup() && ! popupDelay() )
     	w += style().pixelMetric(QStyle::PM_MenuButtonIndicator, this);

    return (style().sizeFromContents(QStyle::CT_ToolButton, this, QSize(w, h)).
	    expandedTo(QApplication::globalStrut()));
}

/*!
  \property QToolButton::usesBigPixmap
  \brief whether this toolbutton uses big pixmaps.

  QToolButton automatically connects this property to the relevant signal
  in the QMainWindow in which it resides.  You're strongly urged to
  use QMainWindow::setUsesBigPixmaps() instead.

  \warning If you set some buttons (in a QMainWindow) to have big pixmaps and
  others to have small pixmaps, QMainWindow may have trouble getting the
  geometry correct.
*/

void QToolButton::setUsesBigPixmap( bool enable )
{
    if ( (bool)ubp == enable )
	return;

    ubp = enable;
    if ( isVisible() ) {
	update();
	updateGeometry();
    }
}


/*!
  \property QToolButton::usesTextLabel
  \brief whether the toolbutton displays a text label below the button pixmap.

  QToolButton automatically connects this slot to the relevant signal
  in the QMainWindow in which is resides.
*/

void QToolButton::setUsesTextLabel( bool enable )
{
    if ( (bool)utl == enable )
	return;

    utl = enable;
    if ( isVisible() ) {
	update();
	updateGeometry();
    }
}


/*!
  \property QToolButton::on
  \brief whether this tool button is on.

  This property has no effect on \link isToggleButton() non-toggling
  buttons. \endlink

  \sa isToggleButton() toggle()
*/

void QToolButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!  Toggles the state of this tool button.

  This function has no effect on \link isToggleButton() non-toggling
  buttons. \endlink

  \sa isToggleButton() toggled()
*/

void QToolButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*! \reimp
 */
void QToolButton::drawButton( QPainter * p )
{
    QStyle::SCFlags controls = QStyle::SC_ToolButton;
    QStyle::SCFlags active = QStyle::SC_None;

    bool use3d = uses3D();
    bool drawarrow = hasArrow;
    Qt::ArrowType arrowtype = d->arrow;
    void *data[3];
    data[0] = (void *) &use3d;
    data[1] = (void *) &drawarrow;
    data[2] = (void *) &arrowtype;

    if (isDown())
	active |= QStyle::SC_ToolButton;

    if (d->popup && !d->delay) {
	controls |= QStyle::SC_ToolButtonMenu;
	if (d->instantPopup || isDown())
	    active |= QStyle::SC_ToolButtonMenu;
    }

    style().drawComplexControl(QStyle::CC_ToolButton,
			       p, this, rect(), colorGroup(),
			       QStyle::CStyle_Default,
			       controls, active, data);
}


/*!\reimp
 */
void QToolButton::enterEvent( QEvent * e )
{
    if ( autoRaise() ) {
	threeDeeButton = this;
	if ( isEnabled() )
	    repaint(FALSE);
    }
    QButton::enterEvent( e );
}


/*!\reimp
 */
void QToolButton::leaveEvent( QEvent * e )
{
    if ( autoRaise() ) {
	QToolButton * o = threeDeeButton;
	threeDeeButton = 0;
	if ( o && o->isEnabled() )
	    o->repaint(FALSE);
    }
    QButton::leaveEvent( e );
}

/*!\reimp
 */
void QToolButton::moveEvent( QMoveEvent * )
{
    //   Reimplemented to handle pseudo transparency in case the toolbars
    //   has a fancy pixmap background.
    if ( parentWidget() && parentWidget()->backgroundPixmap() &&
	 autoRaise() && !uses3D() )
	repaint( FALSE );
}

/*!\reimp
*/
void QToolButton::mousePressEvent( QMouseEvent *e )
{
    QRect popupr =
	style().querySubControlMetrics(QStyle::CC_ToolButton, this,
				       QStyle::SC_ToolButtonMenu);
    d->instantPopup = (popupr.isValid() && popupr.contains(e->pos()));

    if ( d->discardNextMouseEvent ) {
	d->discardNextMouseEvent = FALSE;
	d->instantPopup = FALSE;
	d->popup->removeEventFilter( this );
	return;
    }
    if ( e->button() == LeftButton && d->delay <= 0 && d->popup && d->instantPopup && !d->popup->isVisible() ) {
	openPopup();
	return;
    }
    d->instantPopup = FALSE;
    QButton::mousePressEvent( e );
}

/*! \reimp */
bool QToolButton::eventFilter( QObject *o, QEvent *e )
{
    if ( o != d->popup )
	return QButton::eventFilter( o, e );
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
	{
	    QMouseEvent *me = (QMouseEvent*)e;
	    QPoint p = me->globalPos();
	    if ( QApplication::widgetAt( p, TRUE ) == this )
		d->discardNextMouseEvent = TRUE;
	}
	break;
    default:
	break;
    }
    return QButton::eventFilter( o, e );
}

/*!  Returns TRUE if this button should be drawn using raised edges.
  \sa drawButton() */

bool QToolButton::uses3D() const
{
    return !autoRaise() || ( threeDeeButton == this && isEnabled() ) ||
	( d->popup && d->popup->isVisible() && d->delay <= 0 ) || d->instantPopup;
}


/*!
  \property QToolButton::textLabel
  \brief the label of this button.

  Setting this property automatically sets it as tool tip, too.

  \sa setTextLabel( const QString &newLabel , bool tipToo )
*/

void QToolButton::setTextLabel( const QString &newLabel )
{
    setTextLabel( newLabel, TRUE );
}

/*!  Sets the label of this button to \a newLabel and automatically
  sets it as tool tip too if \a tipToo is TRUE.
*/

void QToolButton::setTextLabel( const QString &newLabel , bool tipToo )
{
    if ( tl == newLabel )
	return;

#ifndef QT_NO_TOOLTIP
    if ( tipToo ) {
        QToolTip::remove( this );
        QToolTip::add( this, newLabel );
    }
#endif

    tl = newLabel;
    if ( usesTextLabel() && isVisible() ) {
	update();
	updateGeometry();
    }

}

QIconSet QToolButton::onIconSet() const
{
    return iconSet();
}

QIconSet QToolButton::offIconSet( ) const
{
    return iconSet();
}

/*!
  \property QToolButton::onIconSet
  \brief the icon set that is used when the button is in an "on" state

  \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons. There is
  now an \l iconSet property that replaces both \l onIconSet and
  \l offIconSet.

  For ease of porting, this property is a synonym for \l iconSet. You
  probably want to go over your application code and use the QIconSet
  On/Off mechanism.

  \sa iconSet QIconSet::State
*/
void QToolButton::setOnIconSet( const QIconSet& set )
{
    setIconSet( set );
    /*
      ### Get rid of all qWarning in this file in 4.0.
      Also consider inlining the obsolete functions then.
    */
    qWarning( "QToolButton::setOnIconSet(): This function is not supported"
	      " anymore" );
}

/*!
  \property QToolButton::offIconSet
  \brief the icon set that is used when the button is in an "off" state

  \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons. There is
  now an \l iconSet property that replaces both \l onIconSet and
  \l offIconSet.

  For ease of porting, this property is a synonym for \l iconSet. You
  probably want to go over your application code and use the QIconSet
  On/Off mechanism.

  \sa iconSet QIconSet::State
*/
void QToolButton::setOffIconSet( const QIconSet& set )
{
    setIconSet( set );
}

/*! \property QToolButton::iconSet
    \brief the icon set providing the icon shown on the button

  Setting this property sets \l pixmap to a null pixmap.

  \sa pixmap(), setToggleButton(), isOn()
*/
void QToolButton::setIconSet( const QIconSet & set )
{
    if ( s )
	delete s;
    setPixmap( QPixmap() );
    s = new QIconSet( set );
    if ( isVisible() )
	update();
}

/*! \overload
    \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  sets the \l iconSet property. If you relied on the \a on parameter,
  you probably want to update your code to use the QIconSet On/Off
  mechanism.

  \sa iconSet QIconSet::State
*/
void QToolButton::setIconSet( const QIconSet & set, bool /* on */ )
{
    setIconSet( set );
    qWarning( "QToolButton::setIconSet(): 'on' parameter ignored" );
}

QIconSet QToolButton::iconSet() const
{
    QToolButton *that = (QToolButton *) this;

    if ( pixmap() && !pixmap()->isNull() &&
	 (!that->s || (that->s->pixmap().serialNumber() !=
	 pixmap()->serialNumber())) ) {
	if ( that->s )
	    delete that->s;
	that->s = new QIconSet( *pixmap() );
    }
    if ( that->s )
	return *that->s;
    /*
      In 2.x, we used to return a temporary nonnull QIconSet. If you
      revert to the old behavior, you will break calls to
      QIconSet::isNull() in this file.
    */
    return QIconSet();
}

/*! \overload
    \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  returns the \l iconSet property. If you relied on the \a on
  parameter, you probably want to update your code to use the QIconSet
  On/Off mechanism.
*/
QIconSet QToolButton::iconSet( bool /* on */ ) const
{
    return iconSet();
}

/*!
  Associates the popup menu \a popup with this tool button.

  The popup will be shown each time the tool button has been pressed
  down for a certain amount of time. A typical application example is
  the "back" button in some web browsers's tool bars. If the user clicks it,
  the browser simply browses back to the previous page. If the user
  holds the button down for a while, the tool button shows a menu
  containing the current history list.

  Ownership of the popup menu is not transferred.

  \sa popup()
 */
void QToolButton::setPopup( QPopupMenu* popup )
{
    if ( popup && !d->popupTimer ) {
	connect( this, SIGNAL( pressed() ), this, SLOT( popupPressed() ) );
	d->popupTimer = new QTimer( this );
	connect( d->popupTimer, SIGNAL( timeout() ), this, SLOT( popupTimerDone() ) );
    }
    d->popup = popup;

    update();
}

/*!
  Returns the associated popup menu, or 0 if no popup menu has been
  defined.

  \sa setPopup()
 */
QPopupMenu* QToolButton::popup() const
{
    return d->popup;
}

/*!
  Opens the associated popup menu. If there is no such menu, this
  function does nothing. This function does not return until the popup
  menu has been closed by the user.
*/
void QToolButton::openPopup()
{
    if ( !d->popup )
	return;

    d->instantPopup = TRUE;
    repaint( FALSE );
    popupTimerDone();
    d->instantPopup = FALSE;
    repaint( FALSE );
}

void QToolButton::popupPressed()
{
    if ( d->popupTimer && d->delay > 0 )
	d->popupTimer->start( d->delay, TRUE );
}

void QToolButton::popupTimerDone()
{
    if ( !d->popup )
	return;

    d->popup->installEventFilter( this );
    d->repeat = autoRepeat();
    setAutoRepeat( FALSE );
    bool horizontal = TRUE;
    bool topLeft = TRUE;
    if ( parentWidget() && parentWidget()->inherits("QToolBar") ) {
	if ( ( (QToolBar*) parentWidget() )->orientation() == Vertical )
	    horizontal = FALSE;
    }
    if ( horizontal ) {
	if ( topLeft ) {
	    if ( mapToGlobal( QPoint( 0, rect().bottom() ) ).y() + d->popup->sizeHint().height() <= qApp->desktop()->height() )
		d->popup->exec( mapToGlobal( rect().bottomLeft() ) );
	    else
		d->popup->exec( mapToGlobal( rect().topLeft() - QPoint( 0, d->popup->sizeHint().height() ) ) );
	} else {
	    QSize sz( d->popup->sizeHint() );
	    QPoint p = mapToGlobal( rect().topLeft() );
	    p.ry() -= sz.height();
	    d->popup->exec( p );
	}
    } else {
	if ( topLeft ) {
	    if ( mapToGlobal( QPoint( rect().right(), 0 ) ).x() + d->popup->sizeHint().width() <= qApp->desktop()->width() )
		d->popup->exec( mapToGlobal( rect().topRight() ) );
	    else
		d->popup->exec( mapToGlobal( rect().topLeft() - QPoint( d->popup->sizeHint().width(), 0 ) ) );
	} else {
	    QSize sz( d->popup->sizeHint() );
	    QPoint p = mapToGlobal( rect().topLeft() );
	    p.rx() -= sz.width();
	    d->popup->exec( p );
	}
    }
    setDown( FALSE );
    if ( d->repeat )
	setAutoRepeat( TRUE );
}

/*!
  \property QToolButton::popupDelay
  \brief the time delay between pressing the button and the appearance of the associated popup menu in milliseconds.

  Usually this is around half a second. A value of 0 will add a special section to the toolbutton
  that can be used to open the popupmenu.

  \sa setPopup()
*/
void QToolButton::setPopupDelay( int delay )
{
    d->delay = delay;

    update();
}

int QToolButton::popupDelay() const
{
    return d->delay;
}


/*!
  \property QToolButton::autoRaise
  \brief whether auto-raising is enabled or not.
 */
void QToolButton::setAutoRaise( bool enable )
{
    d->autoraise = enable;

    update();
}

bool QToolButton::autoRaise() const
{
    return d->autoraise;
}

/*!
  Returns TRUE if the button is on and the "on" pixmap is the same as
  the "off" pixmap, otherwise returns FALSE.

  We don't really compare the pixmaps, not even their serial numbers,
  but we do check whether the "on" component is generated or not.
*/
bool QToolButton::isOnAndNoOnPixmap()
{
    return isOn() &&
	   ( s == 0 ||
	    (s->isGenerated(QIconSet::Small, QIconSet::Normal, QIconSet::On) &&
	     s->isGenerated(QIconSet::Large, QIconSet::Normal, QIconSet::On)) );
}

#ifndef QT_NO_PALETTE
/*! \reimp */
void QToolButton::paletteChange( const QPalette & )
{
    if ( s )
	s->clearGenerated();
}
#endif

#endif
