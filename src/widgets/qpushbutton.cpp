/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.cpp#145 $
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
#include "qcleanuphandler.h"

// NOT REVISED
/*!
  \class QPushButton qpushbutton.h
  \brief The QPushButton widget provides a command button.

  \ingroup basic

  The push button, also referred to as a command button, is perhaps the
  most central widget in any graphical user interface. Push it to
  command the computer to perform some action. Typical actions are Ok,
  Apply, Cancel, Close and Help.

  A command button is rectangular (about 80 x 22 pixels) and typically
  displays a text label describing its action. An underscored
  character in the label, marked with an ampersand in the text,
  signals an accelerator key.

  This code creates a push button labelled "Rock & Roll".  Due to the
  first ampersand, the c displays underscored and the button gets the
  automatic accelerator key Alt-C:

  \code
    QPushButton *p = new QPushButton( "Ro&ck && Roll", this );
  \endcode

  The text can be changed anytime later with setText(). You can also
  define a pixmap with setPixmap(). The text/pixmap is manipulated as
  necessary to create a "disabled" appearance according to the
  respective GUI style when the button is disabled. In addition to the text or pixmap label, a command button
  can also display a little icon. Use the extended constructor or setIconSet() to define this
  icon.

  A push button emits the signal clicked() when it is activated
  with the mouse, the spacebar or a keyboard accelerator.
  Connect to this signal to perform the button's action.  Other
  signals of less importance are pressed() when the button is pressed
  down and released() when it is released.

  Command buttons in dialogs are by default auto-default buttons,
  i.e., they become the default push button automatically when they
  receive the keyboard input focus. A default button is a command
  button that is activated when the user hits the Enter or Return key
  in a dialog. Adjust this behaviour with setAutoDefault(). Note that
  auto-default buttons reserve a little extra space necessary to draw
  a default-button indicator. If you do not want this space around
  your buttons, call setAutoDefault(FALSE).

  Being so central, the widget has grown to accommodate a great many
  variations in the past decade. The Microsoft style guide now
  shows about ten different states of Windows push buttons and the
  text implies that there are dozens more when all the combinations of
  features are taken into consideration.

  Sorted roughly by importance, the most important modes or states are: <ul>
  <li> Available or not (grayed out, disabled).
  <li> Standard push button, toggling push button or menu button.
  <li> On or off (only for toggling push buttons).
  <li> Default or normal.  The default button in a dialog can
  generally be "clicked" using the Enter or Return key.
  <li> Auto-repeat or not.
  <li> Pressed down or not.
  </ul>

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
  probably not what you want. When in doubt, go with a tool button.

  A variation of a command button is a menu button. It provides not
  just one command, but several. Use the method setPopup() to
  associate a popup menu with a push button.

  Other classes of buttons are option buttons (see QRadioButton) and
  check boxes (see QCheckBox).

  <img src="qpushbt-m.png"> <img src="qpushbt-w.png">

  In Qt, the QButton class provides most of the modes and other API,
  and QPushButton provides GUI logic.  See QButton for more
  information about the API.

  \important text, setText, text, pixmap, setPixmap, accel, setAccel,
  isToggleButton, setDown, isDown, isOn, state, autoRepeat,
  isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released,
  clicked, toggled, state stateChanged

  \sa QToolButton, QRadioButton QCheckBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Push Button</a>
*/

/*! \property QPushButton::autoDefault
    \brief whether the push button is the auto default button

  If this property is set to TRUE then the push button will be the focused
  item in a dialog when it is first shown.
*/

/*! \property QPushButton::default
    \brief whether the push button is the default

  If this property is set to TRUE then the push button will be pressed if the
  user hits the enter key in a dialog.
*/

/*! \property QPushButton::flat
    \brief whether the border is disabled
*/

/*! \property QPushButton::iconSet
    \brief the icon set on the push button

  This property will return 0 if there is no icon set on the push button
*/

/*! \property QPushButton::on
    \brief whether the push button is toggled

  This property should only be set for toggle push buttons.
*/

/*! \property QPushButton::toggleButton
    \brief whether the button is a toggle button
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

static QPtrDict<QPushButtonPrivate> *d_ptr = 0;

static QCleanupHandler<QPtrDict<QPushButtonPrivate> > qpb_cleanup_private;

static QPushButtonPrivate* d( const QPushButton* foo )
{
    if ( !d_ptr ) {
	d_ptr = new QPtrDict<QPushButtonPrivate>;
	d_ptr->setAutoDelete( TRUE );
	qpb_cleanup_private.add( d_ptr );
    }
    QPushButtonPrivate* ret = d_ptr->find( (void*)foo );
    if ( ! ret ) {
	ret = new QPushButtonPrivate;
	d_ptr->replace( (void*) foo, ret );
    }
    return ret;
}

static bool has_d( const QPushButton* foo )
{
    return d_ptr && d_ptr->find( (void*)foo );
}

static void delete_d( const QPushButton* foo )
{
    if ( d_ptr )
	d_ptr->remove( (void*) foo );
}



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
  Constructs a push button with the parent \a parent and the name \a name
  with the text \a text.
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

  Note that you can also pass a QPixmap object as an icon (thanks to the implicit type conversion C++).

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
    delete_d( this );
}

void QPushButton::init()
{
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


/*!
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


/*!
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
}


/*!\reimp
*/
QSize QPushButton::sizeHint() const
{
    constPolish();
    int w = 0;
    int h = 0;

    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width() + style().buttonMargin();
	h = pm->height() + style().buttonMargin();
    } else {
	QString s( text() );
	if ( s.isEmpty() )
	    s = QString::fromLatin1("XXXX");
	QFontMetrics fm = fontMetrics();
	QSize sz = fm.size( ShowPrefix, s );
	w = sz.width() + style().buttonMargin();
	h = sz.height() + sz.height()/8 + 4 + style().buttonMargin();
//	w += h;
    }


    if ( iconSet() && !iconSet()->isNull() ) {
	int iw = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	int ih = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	w += iw + style().buttonMargin() - 2;
	h = QMAX( h, ih + style().buttonMargin() );
    }


    if ( isDefault() || autoDefault() ) {
	w += 2*style().buttonDefaultIndicatorWidth();
	h += 2*style().buttonDefaultIndicatorWidth();
    }

    if ( isMenuButton() )
	w += style().menuButtonIndicatorWidth( h );

#ifndef Q_WS_QWS
     if ( style() == WindowsStyle ) {
	// in windows style, try a little harder to conform to
	// microsoft's size specifications
	if ( h <= 25 )
	    h = 22;
	if ( w < 85 && !pixmap() && ( isDefault() || autoDefault() ) )
	    w = 80;
     } else if ( !pixmap() && (isDefault() || autoDefault() ) ) {
	 // un-unglify motifstyles until we fix boxlayout to treat
	 // pushbuttons differently
	 if ( w < 80 )
	     w = 80;
     }
#endif

     return QSize( w, h ).expandedTo( QApplication::globalStrut() );
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
	diw = style().buttonDefaultIndicatorWidth();
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

    style().drawPushButton(this, paint);
    drawButtonLabel( paint );
    int x1, y1, x2, y2;
    style().pushButtonContentsRect( this ).coords( &x1, &y1, &x2, &y2 );// get coordinates
    if ( hasFocus() ) {
 	QRect r(x1+2, y1+2, x2-x1-3, y2-y1-3);
 	style().drawFocusRect( paint, r , colorGroup(), &colorGroup().button() );
    }
    lastEnabled = isEnabled();
}


/*!\reimp
*/
void QPushButton::drawButtonLabel( QPainter *paint )
{
    style().drawPushButtonLabel( this, paint );
}


/*!\reimp
 */
void QPushButton::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );

    {
	QPainter p( &bm, this );
	p.setPen( color1 );
	p.setBrush( color1 );
	style().drawButtonMask( &p, 0, 0, width(), height() );
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

  Ownership of the popup menu is not transferred.

  \sa popup()
 */
void QPushButton::setPopup( QPopupMenu* popup )
{
    if ( popup && !::d( this )->popup )
	connect( this, SIGNAL( pressed() ), this, SLOT( popupPressed() ) );

    ::d( this )->popup = popup;
    autoDefButton = FALSE;
    setIsMenuButton( popup != 0 );
}


void QPushButton::setIconSet( const QIconSet& icon )
{
    if (! icon.isNull()) {
	if ( ::d( this )->iconset )
	    *::d( this )->iconset = icon;
	else
	    ::d( this )->iconset = new QIconSet( icon );
    } else if (::d(this)->iconset) {
	delete (::d(this)->iconset);
	::d(this)->iconset = 0;
    }

    update();
    updateGeometry();
}


QIconSet* QPushButton::iconSet() const
{
    if ( !::has_d( this ) )
	return 0;
    return ::d( this )->iconset;
}

/*!
  Returns the associated popup menu or 0 if no popup menu has been
  defined.

  \sa setPopup()
*/
QPopupMenu* QPushButton::popup() const
{
    if ( !::has_d( this ) )
	return 0;
    return ::d( this )->popup;
}

void QPushButton::popupPressed()
{
    QPopupMenu* popup = ::d( this )->popup;
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

#endif
