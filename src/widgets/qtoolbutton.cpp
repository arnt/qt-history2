/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbutton.cpp#69 $
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
    uint instantPopup : 1;
    bool autoraise, repeat;
    Qt::ArrowType arrow;
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
  Constructs an empty tool button.
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
    bpID = bp.serialNumber();
    spID = sp.serialNumber();

    utl = FALSE;
    ubp = TRUE;
    hasArrow = FALSE;

    s = 0;
    son = 0;

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

    int w = 0;
    int h = 0;

    if ( !text().isNull()) {
	w = fontMetrics().width( text() );
	h = fontMetrics().height(); // boundingRect()?
    } else if ( usesBigPixmap() ) {
	QPixmap pm = iconSet(TRUE).pixmap(QIconSet::Large, QIconSet::Normal);
	w = pm.width();
	h = pm.height();
	if ( w < 32 )
	    w = 32;
	if ( h < 32 )
	    h = 32;
    } else {
	w = h = 16;
	QPixmap pm = iconSet(TRUE).pixmap(QIconSet::Small, QIconSet::Normal);
	w = pm.width();
	h = pm.height();
#ifndef Q_WS_QWS // shouldn't be on any platform...
	if ( w < 16 )
	    w = 16;
	if ( h < 16 )
	    h = 16;
#endif
    }

    if ( usesTextLabel() ) {
	h += 4 + fontMetrics().height();
	int tw = fontMetrics().width( textLabel() ) + fontMetrics().width("  ");
	if ( tw > w )
	    w = tw;
    }
    if ( d->popup && !d->delay )
	w += style().menuButtonIndicatorWidth( height() );
#ifdef Q_WS_QWS // ###### should be style option
    return QSize( w + 4, h + 2 ).expandedTo( QApplication::globalStrut() );
#else
    return QSize( w + 7, h + 6 ).expandedTo( QApplication::globalStrut() );
#endif
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
    int x = 0;
    int y = 0;
    int w = width();
    int h = height();
    int miw = style().menuButtonIndicatorWidth( h );
    if ( d->popup && !d->delay )
	w -= miw;

    const QColorGroup &g = colorGroup();

    if ( uses3D() || ( isOn() && !son ) || isDown() ) {
        style().drawToolButton( p, x, y, w, h, g, isOn(), isDown(), isEnabled(), autoRaise() );
    } else if ( parentWidget() && parentWidget()->backgroundPixmap() &&
              !parentWidget()->backgroundPixmap()->isNull() ) {
        p->drawTiledPixmap( 0, 0, width(), height(),
                            *parentWidget()->backgroundPixmap(),
                            this->x(), this->y() );
    }
    drawButtonLabel( p );

    if ( d->popup && !d->delay ) {
	if ( uses3D() )
	    style().drawDropDownButton( p, w, y, miw, h, g, 
		    d->instantPopup || isDown() || isOn(), isEnabled(), autoRaise() );
	style().drawArrow( p, DownArrow, d->instantPopup || isDown() || isOn(), w+2, y+4, 
		miw-4, h-8, g, isEnabled() );
    }

    if ( hasFocus() && !focusProxy() ) {
	if ( style() == WindowsStyle ) {
	    p->drawWinFocusRect( 3, 3, width()-6, height()-6,
				 colorGroup().background() );
	} else {
	    p->setPen( black );
	    p->drawRect( 3, 3, width()-6, height()-6 );
	}
    }
}


/*!\reimp
 */
void QToolButton::drawButtonLabel( QPainter * p )
{
    int sx = 0;
    int sy = 0;
    int x, y, w, h;
    if ( d->popup && !d->delay ) 
	style().toolButtonRect(0, 0, width() - style().menuButtonIndicatorWidth( height() ), height() ).rect( &x, &y, &w, &h );
    else
	style().toolButtonRect(0, 0, width(), height() ).rect( &x, &y, &w, &h );
    if (isDown() || (isOn()&&!son) ) {
	style().getButtonShift(sx, sy);
	x+=sx;
	y+=sy;
    }
    if ( hasArrow ) {
	style().drawArrow( p, d->arrow, isDown(), x, y, w, h, colorGroup(), isEnabled() );
	return;
    }

    QColor btnText =  colorGroup().buttonText();

    if ( !text().isNull() && !usesTextLabel() ) {
	style().drawItem( p, x, y, w, h,
			  AlignCenter + ShowPrefix,
			  colorGroup(), isEnabled(),
			  0, text(), text().length(), &btnText );
    } else {
	QPixmap pm;
	if ( usesBigPixmap() ) {
	    if ( !isEnabled() )
		pm = iconSet( isOn() ).pixmap( QIconSet::Large, QIconSet::Disabled, isOn() ? QIconSet::On : QIconSet::Off );
	    else if ( uses3D() )
		pm = iconSet( isOn() ).pixmap( QIconSet::Large, QIconSet::Active, isOn() ? QIconSet::On : QIconSet::Off );
	    else
		pm = iconSet( isOn() ).pixmap( QIconSet::Large, QIconSet::Normal, isOn() ? QIconSet::On : QIconSet::Off );
	} else {
	    if ( !isEnabled() )
		pm = iconSet( isOn() ).pixmap( QIconSet::Small, QIconSet::Disabled, isOn() ? QIconSet::On : QIconSet::Off );
	    else if ( uses3D() )
		pm = iconSet( isOn() ).pixmap( QIconSet::Small, QIconSet::Active, isOn() ? QIconSet::On : QIconSet::Off );
	    else
		pm = iconSet( isOn() ).pixmap( QIconSet::Small, QIconSet::Normal, isOn() ? QIconSet::On : QIconSet::Off );
	}

	if ( usesTextLabel() ) {
	    int fh = fontMetrics().height();
	    style().drawItem( p, x, y, w, h - fh,
			      AlignCenter, colorGroup(), TRUE, &pm, QString::null );
	    p->setFont( font() );
	    style().drawItem( p, x, h - fh, w, fh,
			      AlignCenter + ShowPrefix,
			      colorGroup(), isEnabled(),
			      0, textLabel(), textLabel().length(), &btnText );
 	} else {
	    style().drawItem( p, x, y, w, h,
			      AlignCenter, colorGroup(), TRUE, &pm, QString::null );
	}
    }
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
    bool left = FALSE;
    if ( parentWidget() && parentWidget()->isA( "QToolBar" ) ) {
	QToolBar *tb = (QToolBar*)parentWidget();
	if ( tb->orientation() == Vertical && mapToGlobal( tb->rect().center() ).x() > topLevelWidget()->rect().center().x() )
	    left = TRUE;
    }

    int dbw = style().menuButtonIndicatorWidth( height() );
    d->instantPopup = ( ( e->pos().x() < dbw ) && left  ) || ( ( e->pos().x() > ( width() - dbw ) ) && !left );

    if ( e->button() == LeftButton && d->delay <= 0 && d->popup && d->instantPopup ) {
	d->instantPopup = TRUE;
	repaint( FALSE );
	popupTimerDone();
	d->instantPopup = FALSE;
	repaint( FALSE );
	return;
    }
    d->instantPopup = FALSE;
    QButton::mousePressEvent( e );
}

/*!  Returns TRUE if this button should be drawn using raised edges.
  \sa drawButton() */

bool QToolButton::uses3D() const
{
    return !autoRaise() || ( threeDeeButton == this && isEnabled() ) ||
	( d->popup && d->popup->isVisible() && d->delay <= 0 );
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

/*!
  \property QToolButton::onIconSet
  \brief the icon set that is used when the button is in an on-state.

  \sa setIconSet()
*/
void QToolButton::setOnIconSet( const QIconSet& set )
{
    setIconSet( set, TRUE );
}

QIconSet QToolButton::onIconSet() const
{
    return iconSet( TRUE );
}

/*!
  \property QToolButton::offIconSet
  \brief the icon set that is used when the button is in an off-state.

  \sa setIconSet()
*/
QIconSet QToolButton::offIconSet( ) const
{
    return iconSet( FALSE );
}

void QToolButton::setOffIconSet( const QIconSet& set )
{
    setIconSet( set, FALSE );
}



/*!  Sets this tool button to display the icons in \a set
  (setPixmap() is effectively a wrapper for this function).

  For toggle buttons it is possible to set an extra icon set with \a
  on equals TRUE, which will be used exclusively for the on-state.

  QToolButton makes a copy of \a set, so you must delete \a set
  yourself.

  \sa iconSet() QIconSet, setToggleButton(), isOn()
*/

void QToolButton::setIconSet( const QIconSet & set, bool on )
{
    if ( !on ) {
	if ( s )
	    delete s;
	s = new QIconSet( set );
    } else {
	if ( son )
	    delete son;
	son = new QIconSet( set );
    }
    if ( isVisible() )
	update();
}


/*!  Returns a copy of the icon set in use.  If no icon set has been
  set, iconSet() creates one from the pixmap().

  If the button doesn't have a pixmap either, iconSet()'s return value
  is meaningless.

  If \a on equals TRUE, the special icon set for the on-state of the
  button is returned.

  \sa setIconSet() QIconSet
*/

QIconSet QToolButton::iconSet( bool on ) const
{
    QToolButton * that = (QToolButton *)this;

    if ( on && that->son )
	return *that->son;

    if ( pixmap() && (!that->s || (that->s->pixmap().serialNumber() !=
	pixmap()->serialNumber())) ) {
	if ( that->s )
	    delete that->s;
	that->s = new QIconSet( *pixmap() );
    }

    if ( that->s )
	return *that->s;

    QPixmap tmp1;
    QIconSet tmp2( tmp1, QIconSet::Small );
    return tmp2;
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
  Returns the associated popup menu or 0 if no popup menu has been
  defined.

  \sa setPopup()
 */
QPopupMenu* QToolButton::popup() const
{
    return d->popup;
}

void QToolButton::popupPressed()
{
    if ( d->popupTimer && d->delay > 0 )
	d->popupTimer->start( d->delay, TRUE );
}

void QToolButton::popupTimerDone()
{
    if ( d->popup ) {
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
	}
	else {
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
#endif
