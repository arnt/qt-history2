/****************************************************************************
** $Id: $
**
** Implementation of QPushButton class
**
** Created : 940221
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

#include "qpushbutton.h"
#ifndef QT_NO_PUSHBUTTON
#include "qdialog.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qbitmap.h"
#include "qpopupmenu.h"
#include "qguardedptr.h"
#include "qptrdict.h" // binary compatibility
#include "qapplication.h"
#include "qtoolbar.h"
#include "qstyle.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

/*!
  \class QPushButton qpushbutton.h
  \brief The QPushButton widget provides a command button.

  \ingroup basic
  \mainclass

  The push button, or command button, is perhaps the most commonly
  used widget in any graphical user interface. Push (click) a button
  to command the computer to perform some action, or to answer a
  question. Typical buttons are OK, Apply, Cancel, Close, Yes, No and
  Help.

  A command button is rectangular and typically displays a text label
  describing its action. An underscored character in the label
  (signified by preceding it with an ampersand in the text) indicates
  an accelerator key, e.g.
  \code
    QPushButton *pb = new QPushButton( "&Download", this );
  \endcode
  In this example the accelerator is \e{Ctrl+D}, and the label text
  will be displayed as <b><u>D</u>ownload</b>.

   Push buttons can display a textual label or a pixmap, and
   optionally a small icon. These can be set using the constructors
   and changed later using setText(), setPixmap() and setIconSet(). If
   the button is disabled the appearance of the text or pixmap and
   iconset will be manipulated with respect to the GUI style to make
   the button look "disabled".

  A push button emits the signal clicked() when it is activated
  by the mouse, the Spacebar or by a keyboard accelerator.
  Connect to this signal to perform the button's action. Push buttons
  also provide less commonly used signals, for example, pressed()
  and released().

  Command buttons in dialogs are by default auto-default buttons,
  i.e. they become the default push button automatically when they
  receive the keyboard input focus. A default button is a push
  button that is activated when the user hits the Enter or Return key
  in a dialog. You can change this with setAutoDefault(). Note that
  auto-default buttons reserve a little extra space which is necessary
  to draw a default-button indicator. If you do not want this space
  around your buttons, call setAutoDefault(FALSE).

  Being so central, the button widget has grown to accommodate a great many
  variations in the past decade. The Microsoft style guide now
  shows about ten different states of Windows push buttons and the
  text implies that there are dozens more when all the combinations of
  features are taken into consideration.

  The most important modes or states are:
  \list
  \i Available or not (grayed out, disabled).
  \i Standard push button, toggling push button or menu button.
  \i On or off (only for toggling push buttons).
  \i Default or normal.  The default button in a dialog can
  generally be "clicked" using the Enter or Return key.
  \i Auto-repeat or not.
  \i Pressed down or not.
  \endlist

  As a general rule, use a push button when the application or dialog
  window performs an action when the user clicks on it (such as Apply,
  Cancel, Close and Help) \e and when the widget is supposed to have
  a wide, rectangular shape with a text label.  Small, typically
  square buttons that change the state of the window rather than
  performing an action (such as the buttons in the top-right
  corner of the QFileDialog) are not command buttons, but tool
  buttons. Qt provides a special class (QToolButton) for these buttons.

  If you need toggle behavior (see setToggleButton()) or a button
  that auto-repeats the activation signal when being pushed down like
  the arrows in a scroll bar (see setAutoRepeat()), a command button is
  probably not what you want. When in doubt, use a tool button.

  A variation of a command button is a menu button. These provide not
  just one command, but several, since when they are clicked they pop
  up a menu of options. Use the method setPopup() to associate a popup
  menu with a push button.

  Other classes of buttons are option buttons (see QRadioButton) and
  check boxes (see QCheckBox).

  <img src="qpushbt-m.png"> <img src="qpushbt-w.png">

  In Qt, the QButton base class provides most of the modes and other
  API, and QPushButton provides GUI logic.  See QButton for more
  information about the API.

  \important text, setText, text, pixmap, setPixmap, accel, setAccel,
  isToggleButton, setDown, isDown, isOn, state, autoRepeat,
  isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released,
  clicked, toggled, state stateChanged

  \sa QToolButton, QRadioButton QCheckBox
  \link guibooks.html#fowler GUI Design Handbook: Push Button\endlink
*/

/*! \property QPushButton::autoDefault
    \brief whether the push button is the auto default button

  If this property is set to TRUE then the push button will be the focused
  item in a dialog when the dialog is first shown.

  This property's default is FALSE.
*/

/*! \property QPushButton::autoMask
    \brief whether the button is automatically masked

    \sa QWidget::setAutoMask()
*/

/*! \property QPushButton::default
    \brief whether the push button is the default button

  If this property is set to TRUE then the push button will be pressed if the
  user hits the Enter key in a dialog.

  This property's default is FALSE.
*/

/*! \property QPushButton::flat
    \brief whether the border is disabled

    This property's default is FALSE.
*/

/*! \property QPushButton::iconSet
    \brief the icon set on the push button

  This property will return 0 if the push button has no iconset
*/

/*! \property QPushButton::on
    \brief whether the push button is toggled

  This property should only be set for toggle push buttons. The
  default value is FALSE.
  \sa isOn(), toggle(), toggled(), isToggleButton()
*/

/*! \property QPushButton::toggleButton
    \brief whether the button is a toggle button

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), isToggleButton() toggled()
*/

/*! \property QPushButton::menuButton
    \brief whether the push button has a menu button on it
    \obsolete

  If this property is set to TRUE, then a down arrow is drawn on the push
  button to indicate that a menu will pop up if the user clicks on the
  arrow.
*/

class QPushButtonPrivate
{
public:
    QPushButtonPrivate()
	:iconset( 0 )
    {}
    ~QPushButtonPrivate()
    {
	delete iconset;
    }
    QGuardedPtr<QPopupMenu> popup;
    QIconSet* iconset;
};


/*!
  Constructs a push button with no text.

  The \a parent and \a name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*!
  Constructs a push button called \a name with the parent \a parent
  and the text \a text.
*/

QPushButton::QPushButton( const QString &text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}


/*!
  Constructs a push button with an \a icon and a \a text.

  Note that you can also pass a QPixmap object as an icon (thanks to
  the implicit type conversion provided by C++).

  The \a parent and \a name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( const QIconSet& icon, const QString &text,
			  QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
    setIconSet( icon );
}



/*!
  Destroys the push button
 */
QPushButton::~QPushButton()
{
    delete d;
}

void QPushButton::init()
{
    d = 0;
    defButton = FALSE;
    lastEnabled = FALSE;
    hasMenuArrow = FALSE;
    flt = FALSE;
#ifndef QT_NO_DIALOG
    autoDefButton = topLevelWidget()->inherits("QDialog");
#else
    autoDefButton = FALSE;
#endif
    setBackgroundMode( PaletteButton );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
}


/*
  Makes the push button a toggle button if \a enable is TRUE or a normal
  push button if \a enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), isToggleButton() toggled()
*/

void QPushButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*
  Switches a toggle button on if \a enable is TRUE or off if \a enable is
  FALSE.
  \sa isOn(), toggle(), toggled(), isToggleButton()
*/

void QPushButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}

void QPushButton::setAutoDefault( bool enable )
{
    if ( (bool)autoDefButton == enable )
	return;
    autoDefButton = enable;
    update();
    updateGeometry();
}


void QPushButton::setDefault( bool enable )
{
    if ( (bool)defButton == enable )
	return;					// no change
    defButton = enable;
#ifndef QT_NO_DIALOG
    if ( defButton && topLevelWidget()->inherits( "QDialog" ) )
 	((QDialog*)topLevelWidget())->setDefault( this );
#endif
    update();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif

}


/*!\reimp
*/
QSize QPushButton::sizeHint() const
{
    constPolish();

    int w = 0, h = 0;

    // calculate contents size...
    if ( iconSet() && !iconSet()->isNull() ) {
	int iw = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	int ih = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	w += iw;
	h = QMAX( h, ih );
    }

    if ( isMenuButton() )
	w += style().pixelMetric(QStyle::PM_MenuButtonIndicator, this);

    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w += pm->width();
	h += pm->height();
    } else {
	QString s( text() );
	bool empty = s.isEmpty();
	if ( empty )
	    s = QString::fromLatin1("XXXX");
	QFontMetrics fm = fontMetrics();
	QSize sz = fm.size( ShowPrefix, s );
	if(!empty || !w)
	    w += sz.width();
	if(!empty || !h)
	    h = QMAX(h, sz.height());
    }

    return (style().sizeFromContents(QStyle::CT_PushButton, this, QSize(w, h)).
	    expandedTo(QApplication::globalStrut()));
}


/*!\reimp
*/
void QPushButton::move( int x, int y )
{
    QWidget::move( x, y );
}

/*!\reimp
*/
void QPushButton::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

/*!\reimp
*/
void QPushButton::resize( int w, int h )
{
    QWidget::resize( w, h );
}

/*!\reimp
*/
void QPushButton::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

/*!\reimp
*/
void QPushButton::setGeometry( int x, int y, int w, int h )
{
    QWidget::setGeometry( x, y, w, h );
}

/*!\reimp
*/
void QPushButton::setGeometry( const QRect &r )
{
    QWidget::setGeometry( r );
}

/*!\reimp
 */
void QPushButton::resizeEvent( QResizeEvent * )
{
    if ( autoMask() )
	updateMask();
}

/*!\reimp
*/
void QPushButton::drawButton( QPainter *paint )
{
    int diw = 0;
    if ( isDefault() || autoDefault() ) {
	diw = style().pixelMetric(QStyle::PM_ButtonDefaultIndicator, this);

	if ( diw > 0 ) {
	    if (backgroundMode() == X11ParentRelative) {
		erase( 0, 0, width(), diw );
		erase( 0, 0, diw, height() );
		erase( 0, height() - diw, width(), diw );
		erase( width() - diw, 0, diw, height() );
	    } else if ( parentWidget() && parentWidget()->backgroundPixmap() ){
		// pseudo tranparency
		paint->drawTiledPixmap( 0, 0, width(), diw,
					*parentWidget()->backgroundPixmap(),
					x(), y() );
		paint->drawTiledPixmap( 0, 0, diw, height(),
					*parentWidget()->backgroundPixmap(),
					x(), y() );
		paint->drawTiledPixmap( 0, height()-diw, width(), diw,
					*parentWidget()->backgroundPixmap(),
					x(), y() );
		paint->drawTiledPixmap( width()-diw, 0, diw, height(),
					*parentWidget()->backgroundPixmap(),
					x(), y() );
	    } else {
		paint->fillRect( 0, 0, width(), diw,
				 colorGroup().brush(QColorGroup::Background) );
		paint->fillRect( 0, 0, diw, height(),
				 colorGroup().brush(QColorGroup::Background) );
		paint->fillRect( 0, height()-diw, width(), diw,
				 colorGroup().brush(QColorGroup::Background) );
		paint->fillRect( width()-diw, 0, diw, height(),
				 colorGroup().brush(QColorGroup::Background) );
	    }

	}
    }

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;
    if (isDown())
	flags |= QStyle::Style_Down;
    if (isOn())
	flags |= QStyle::Style_On;
    if (! isFlat() && ! isDown())
	flags |= QStyle::Style_Raised;
    if (isDefault())
	flags |= QStyle::Style_ButtonDefault;

    style().drawControl(QStyle::CE_PushButton, paint, this, rect(), colorGroup(), flags);
    drawButtonLabel( paint );

    lastEnabled = isEnabled();
}


/*!\reimp
*/
void QPushButton::drawButtonLabel( QPainter *paint )
{

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;
    if (isDown())
	flags |= QStyle::Style_Down;
    if (isOn())
	flags |= QStyle::Style_On;
    if (! isFlat() && ! isDown())
	flags |= QStyle::Style_Raised;
    if (isDefault())
	flags |= QStyle::Style_ButtonDefault;

    style().drawControl(QStyle::CE_PushButtonLabel, paint, this,
			style().subRect(QStyle::SR_PushButtonContents, this),
			colorGroup(), flags);
}


/*!\reimp
 */
void QPushButton::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );

    {
	QPainter p( &bm, this );
	style().drawControlMask(QStyle::CE_PushButton, &p, this, rect());
    }

    setMask( bm );
}

/*!\reimp
*/
void QPushButton::focusInEvent( QFocusEvent *e )
{
    if ( autoDefButton )
	setDefault( TRUE );
#ifndef QT_NO_DIALOG
    else {
	if ( topLevelWidget()->inherits("QDialog") )
	    ((QDialog*)topLevelWidget())->hideDefault();
    }
#endif
    QButton::focusInEvent( e );
}

/*!\reimp
*/
void QPushButton::focusOutEvent( QFocusEvent *e )
{
#ifndef QT_NO_DIALOG
    if ( defButton && autoDefButton ) {
	if ( topLevelWidget()->inherits("QDialog") )
	    ((QDialog*)topLevelWidget())->setDefault( 0 );
    }
#endif

    QButton::focusOutEvent( e );
    if ( popup() && popup()->isVisible() )	// restore pressed status
	setDown( TRUE );
}

/*!
  Associates the popup menu \a popup with this push button and
  thus turns it into a menu button.

  Ownership of the popup menu is \e not transferred to the push button.

  \sa popup()
 */
void QPushButton::setPopup( QPopupMenu* popup )
{
    if ( !d )
	d = new QPushButtonPrivate;
    if ( popup && !d->popup )
	connect( this, SIGNAL( pressed() ), this, SLOT( popupPressed() ) );

    d->popup = popup;
    autoDefButton = FALSE;
    setIsMenuButton( popup != 0 );
}


void QPushButton::setIconSet( const QIconSet& icon )
{
    if ( !d )
	d = new QPushButtonPrivate;
    if ( !icon.isNull() ) {
	if ( d->iconset )
	    *d->iconset = icon;
	else
	    d->iconset = new QIconSet( icon );
    } else if ( d->iconset) {
	delete d->iconset;
	d->iconset = 0;
    }

    update();
    updateGeometry();
}


QIconSet* QPushButton::iconSet() const
{
    return d ? d->iconset : 0;
}

/*!
  Returns the button's associated popup menu or 0 if no popup menu has
  been defined.

  \sa setPopup()
*/
QPopupMenu* QPushButton::popup() const
{
    return d ? (QPopupMenu*)d->popup : 0;
}

void QPushButton::popupPressed()
{
    QPopupMenu* popup = d ? (QPopupMenu*) d->popup : 0;
    if ( isDown() && popup ) {
	bool horizontal = TRUE;
	bool topLeft = TRUE;			// ### always TRUE
#ifndef QT_NO_TOOLBAR
	if ( parentWidget() && parentWidget()->inherits("QToolBar") ) {
	    if ( ( (QToolBar*) parentWidget() )->orientation() == Vertical )
		horizontal = FALSE;
	}
#endif
	if ( horizontal ) {
	    if ( topLeft ) {
		if ( mapToGlobal( QPoint( 0, rect().bottom() ) ).y() + popup->sizeHint().height() <= qApp->desktop()->height() )
		    popup->exec( mapToGlobal( rect().bottomLeft() ) );
		else
		    popup->exec( mapToGlobal( rect().topLeft() - QPoint( 0, popup->sizeHint().height() ) ) );
	    } else {
		QSize sz( popup->sizeHint() );
		QPoint p = mapToGlobal( rect().topLeft() );
		p.ry() -= sz.height();
		popup->exec( p );
	    }
	}
	else {
	    if ( topLeft ) {
		if ( mapToGlobal( QPoint( rect().right(), 0 ) ).x() + popup->sizeHint().width() <= qApp->desktop()->width() )
		    popup->exec( mapToGlobal( rect().topRight() ) );
		else
		    popup->exec( mapToGlobal( rect().topLeft() - QPoint( popup->sizeHint().width(), 0 ) ) );
	    } else {
		QSize sz( popup->sizeHint() );
		QPoint p = mapToGlobal( rect().topLeft() );
		p.rx() -= sz.width();
		popup->exec( p );
	    }
	}
	setDown( FALSE );
    }
}

void QPushButton::setFlat( bool f )
{
    flt = f;
    update();
}

bool QPushButton::isFlat() const
{
    return flt;
}

/*!
    \obsolete
    \fn virtual void QPushButton::setIsMenuButton( bool enable )
*/

#endif
