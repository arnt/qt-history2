/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.cpp#166 $
**
** Implementation of QRadioButton class
**
** Created : 940222
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

#include "qradiobutton.h"
#ifndef QT_NO_RADIOBUTTON
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qtextstream.h"
#include "qapplication.h"

/*!
  \class QRadioButton qradiobutton.h
  \brief The QRadioButton widget provides a radio button with a text label.

  \ingroup basic

  QRadioButton and QCheckBox are both option buttons. That is, they
  can be switched on (checked) or off (unchecked). The classes differ
  in how the choices for the user are restricted. Check boxes define
  "many of many" choices, whereas radio buttons provide a "one of many"
  choice. In a group of radio buttons only one button at a time can
  be checked; if the user selects another button, the previously
  selected button is switched off.

  Although it is technically possible to implement radio-behavior with
  check boxes and vice versa, it is strongly recommended to stick with
  the well-known semantics. Otherwise your users would be pretty
  confused.

  The easiest way to implement a "one of many" choice is simply to
  stick the radio buttons into QButtonGroup.

  Whenever a button is switched on or off it emits the signal
  toggled(). Connect to this signal if you want to trigger an action
  each time the button changes state. Otherwise, use isChecked() to
  query whether or not a particular button is selected.

  <img src=qradiobt-m.png> <img src=qradiobt-w.png>

  \important text, setText, text, pixmap, setPixmap, accel, setAccel,
  isToggleButton, setDown, isDown, isOn, state, autoRepeat,
  isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released,
  clicked, toggled, state stateChanged

  \sa QPushButton QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Radio Button</a>
*/

/*! \property QRadioButton::checked
    \brief Whether the radio button is checked

  This property will not effect any other radio buttons unless they have been
  placed in a QButtonGroup.
*/

static const int gutter = 6; // between button and text
static const int margin = 2; // to right of text


/*!
  Constructs a radio button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name, WRepaintNoErase | WResizeNoErase | WMouseNoMask )
{
    init();
}

/*!
  Constructs a radio button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( const QString &text, QWidget *parent,
			    const char *name )
	: QButton( parent, name, WRepaintNoErase | WResizeNoErase | WMouseNoMask )
{
    init();
    setText( text );
}


/*!
  Initializes the radio button.
*/

void QRadioButton::init()
{
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    setToggleButton( TRUE );
    if ( parentWidget() && parentWidget()->inherits("QButtonGroup") ) {
	QButtonGroup *bgrp = (QButtonGroup *)parentWidget();
	bgrp->setRadioButtonExclusive( TRUE );
    }
}

void QRadioButton::setChecked( bool check )
{
    setOn( check );
}




/*!\reimp
*/
QSize QRadioButton::sizeHint() const
{
    // Any more complex, and we will use style().itemRect()
    // NB: QCheckBox::sizeHint() is similar
    constPolish();

    QPainter p(this);
    QSize sz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				pixmap(), text()).size();

    return (style().sizeFromContents(QStyle::CT_RadioButton, this, sz).
	    expandedTo(QApplication::globalStrut()));
}


/*!\reimp
*/
bool QRadioButton::hitButton( const QPoint &pos ) const
{
    return rect().contains( pos );
}


/*!\reimp
*/
void QRadioButton::drawButton( QPainter *paint )
{
    QPainter *p = paint;
    QRect irect = style().subRect(QStyle::SR_RadioButtonIndicator, this);
    const QColorGroup &cg = colorGroup();

#ifndef QT_NO_TEXTSTREAM
#define SAVE_RADIOBUTTON_PIXMAPS
#endif

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isOn() )
	kf |= 2;
    if ( isEnabled() )
	kf |= 4;
    if( topLevelWidget() != qApp->activeWindow())
	kf |= 8;

    QTextOStream os(&pmkey);
    os << "$qt_radio_" << style().className() << "_"
       << palette().serialNumber() << "_" << kf;
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	drawButtonLabel( p );
	p->drawPixmap( irect.topLeft(), *pm );
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixmap( irect.size() );	// create new pixmap
	Q_CHECK_PTR( pm );
	pm->fill(cg.background());
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx = irect.x();				// save x,y coords
	wy = irect.y();
	irect.moveTopLeft(QPoint(0, 0));
	p->setBackgroundColor( cg.background() );
    }
#endif

    style().drawControl(QStyle::CE_RadioButton, p, this, irect, cg);

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	if ( backgroundPixmap() || backgroundMode() == X11ParentRelative ) {
	    QBitmap bm( pm->size() );
	    bm.fill( color0 );
	    pmpaint.begin( &bm );
	    style().drawControl(QStyle::CE_RadioButtonMask, &pmpaint, this, irect, cg);
	    pmpaint.end();
	    pm->setMask( bm );
	}
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	if (!QPixmapCache::insert(pmkey, pm) )	// save in cache
	    delete pm;
    }
#endif

    drawButtonLabel( p );
}



/*!\reimp
*/
void QRadioButton::drawButtonLabel( QPainter *p )
{
    style().drawControl(QStyle::CE_RadioButtonLabel, p, this,
			style().subRect(QStyle::SR_RadioButtonContents, this),
			colorGroup());
}


/*!
  \reimp
*/
void QRadioButton::resizeEvent( QResizeEvent* e )
{
    QButton::resizeEvent(e);
    int x, w, h;
    GUIStyle gs = style();
    QSize sz = style().exclusiveIndicatorSize();
    if ( gs == WindowsStyle )
	sz.setWidth(sz.width()+1);
    x = sz.width() + gutter;
    w = width() - x;
    if( QApplication::reverseLayout() )
	x = 0;
    h = height();

    QPainter p(this);
    QRect br = style().itemRect( &p, x, 0, w, h,
				 AlignAuto|AlignVCenter|ShowPrefix,
				 isEnabled(),
				 pixmap(), text() );
    if ( autoMask() )
	updateMask();
    update( br.right(), w, 0, h );
}


/*!\reimp
 */
void QRadioButton::updateMask()
{
    QBitmap bm(width(),height());
    bm.fill(color0);

    QPainter p( &bm, this );
    QRect irect = style().subRect(QStyle::SR_RadioButtonIndicator, this);
    QRect crect = style().subRect(QStyle::SR_RadioButtonContents, this);
    QColorGroup cg(color1,color1,color1,color1,color1,color1,color1,color1,color0);

    style().drawControl(QStyle::CE_RadioButtonMask, &p, this, irect, cg);
    p.fillRect(crect, color1);

    setMask(bm);
}

#endif
