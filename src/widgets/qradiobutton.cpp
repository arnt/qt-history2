/****************************************************************************
**
** Implementation of QRadioButton class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qradiobutton.h"
#ifndef QT_NO_RADIOBUTTON
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qstyle.h"

/*!
    \class QRadioButton qradiobutton.h
    \brief The QRadioButton widget provides a radio button with a text or pixmap label.

    \ingroup basic
    \mainclass

    QRadioButton and QCheckBox are both option buttons. That is, they
    can be switched on (checked) or off (unchecked). The classes
    differ in how the choices for the user are restricted. Check boxes
    define "many of many" choices, whereas radio buttons provide a
    "one of many" choice. In a group of radio buttons only one radio
    button at a time can be checked; if the user selects another
    button, the previously selected button is switched off.

    The easiest way to implement a "one of many" choice is simply to
    put the radio buttons into QButtonGroup.

    Whenever a button is switched on or off it emits the signal
    toggled(). Connect to this signal if you want to trigger an action
    each time the button changes state. Otherwise, use isChecked() to
    see if a particular button is selected.

    Just like QPushButton, a radio button can display text or a
    pixmap. The text can be set in the constructor or with setText();
    the pixmap is set with setPixmap().

    <img src=qradiobt-m.png> <img src=qradiobt-w.png>

    \important text, setText, text, pixmap, setPixmap, accel, setAccel, isToggleButton, setDown, isDown, isOn, state, autoRepeat, isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released, clicked, toggled, state stateChanged

    \sa QPushButton QToolButton
    \link guibooks.html#fowler GUI Design Handbook: Radio Button\endlink
*/

/*!
    \property QRadioButton::checked \brief Whether the radio button is
    checked

    This property will not effect any other radio buttons unless they
    have been placed in the same QButtonGroup. The default value is
    FALSE (unchecked).
*/

/*!
    \property QRadioButton::autoMask \brief whether the radio button
    is automatically masked

    \sa QWidget::setAutoMask()
*/

static QPixmap *qt_radiobutton_painter_pix = 0;

/*!
    Constructs a radio button with no text.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name, WMouseNoMask )
{
    init();
}

/*!
    Constructs a radio button with the text \a text.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QRadioButton::QRadioButton( const QString &text, QWidget *parent,
			    const char *name )
	: QButton( parent, name, WMouseNoMask )
{
    init();
    setText( text );
}


/*
    Initializes the radio button.
*/

void QRadioButton::init()
{
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    setToggleButton( TRUE );
#ifndef QT_NO_BUTTONGROUP
    QButtonGroup *bgrp = qt_cast<QButtonGroup*>(parentWidget());
    if ( bgrp )
	bgrp->setRadioButtonExclusive( TRUE );
#endif
}

void QRadioButton::setChecked( bool check )
{
    setOn( check );
}




/*!
    \reimp
*/
QSize QRadioButton::sizeHint() const
{
    // Any more complex, and we will use style().itemRect()
    // NB: QCheckBox::sizeHint() is similar
    ensurePolished();

    if(!qt_radiobutton_painter_pix)
	qt_radiobutton_painter_pix = new QPixmap(1, 1);
    QPainter p(qt_radiobutton_painter_pix, this);
    QSize sz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				pixmap(), text()).size();

    return (style().sizeFromContents(QStyle::CT_RadioButton, this, sz).
	    expandedTo(QApplication::globalStrut()));
}


/*!
    \reimp
*/
bool QRadioButton::hitButton( const QPoint &pos ) const
{
    QRect r =
	QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonFocusRect,
					     this ), this );
    if ( qApp->reverseLayout() ) {
	r.setRight( width() );
    } else {
	r.setLeft( 0 );
    }
    return r.contains( pos );
}


/*!
    \reimp
*/
void QRadioButton::drawButton( QPainter *paint )
{
    QPainter *p = paint;
    QRect irect = QStyle::visualRect( style().subRect(QStyle::SR_RadioButtonIndicator, this), this );
    const QPalette &pal = palette();

    QStyle::SFlags flags = QStyle::Style_Default;
    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;
    if ( hasFocus() )
	flags |= QStyle::Style_HasFocus;
    if ( isDown() )
	flags |= QStyle::Style_Down;
    if ( testAttribute(WA_UnderMouse) )
	flags |= QStyle::Style_MouseOver;
    if ( state() == QButton::On )
	flags |= QStyle::Style_On;
    else if ( state() == QButton::Off )
	flags |= QStyle::Style_Off;

    style().drawControl(QStyle::CE_RadioButton, p, this, irect, pal, flags);

    drawButtonLabel( p );
}



/*!
    \reimp
*/
void QRadioButton::drawButtonLabel( QPainter *p )
{
    QRect r =
	QStyle::visualRect( style().subRect(QStyle::SR_RadioButtonContents,
					    this), this );

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;
    if (isDown())
	flags |= QStyle::Style_Down;
    if (state() == QButton::On)
	flags |= QStyle::Style_On;
    else if (state() == QButton::Off)
	flags |= QStyle::Style_Off;

    style().drawControl(QStyle::CE_RadioButtonLabel, p, this, r, palette(), flags);
}


/*!
    \reimp
*/
void QRadioButton::resizeEvent( QResizeEvent* e )
{
    QButton::resizeEvent(e);
    if ( isVisible() ) {
    if(!qt_radiobutton_painter_pix)
	qt_radiobutton_painter_pix = new QPixmap(1, 1);
    QPainter p(qt_radiobutton_painter_pix, this);
    QSize isz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				 pixmap(), text()).size();
    QSize wsz = (style().sizeFromContents(QStyle::CT_RadioButton, this, isz).
	    expandedTo(QApplication::globalStrut()));

    update(wsz.width(), isz.width(), 0, wsz.height());
   }
    if (autoMask())
	updateMask();
}

/*!
    \reimp
*/
void QRadioButton::updateMask()
{
    QRect irect =
	QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonIndicator,
					     this ), this );

    QBitmap bm(width(), height());
    bm.fill(color0);

    QPainter p( &bm, this );
    style().drawControlMask(QStyle::CE_RadioButton, &p, this, irect);
    if ( ! text().isNull() || ( pixmap() && ! pixmap()->isNull() ) ) {
	QRect crect =
	    QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonContents,
						 this ), this );
	QRect frect =
	    QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonFocusRect,
						 this ), this );
	QRect label(crect.unite(frect));
	p.fillRect(label, color1);
    }
    p.end();

    setMask(bm);
}

#endif
