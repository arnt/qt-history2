/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.cpp#104 $
**
** Implementation of QCheckBox class
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

#include "qcheckbox.h"
#ifndef QT_NO_CHECKBOX
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qtextstream.h"
#include "qapplication.h"

/*!
  \class QCheckBox qcheckbox.h
  \brief The QCheckBox widget provides a checkbox with a text label.

  \ingroup basic

  QCheckBox and QRadioButton are both option buttons. That is, they
  can be switched on (checked) or off (unchecked). The classes differ
  in how the choices for the user are restricted. Radio buttons define
  a "one of many" choice, whereas checkboxes provide "many of many"
  choices.

  Although it is technically possible to implement radio behavior with
  checkboxes and vice versa, we strongly recommended sticking with
  the well-known semantics.

  A QButtonGroup can be used to group check buttons visually.

  Whenever a checkbox is checked or cleared it emits the signal
  toggled(). Connect to this signal if you want to trigger an action
  each time the checkbox changes state. You can use isChecked() to query
  whether or not a checkbox is checked.

  In addition to the usual checked and unchecked states, QCheckBox
  optionally provides a third state to indicate "no change".  This is
  useful whenever you need to give the user the option of neither
  checking nor unchecking a checkbox. If you need this third state,
  enable it with setTristate() and use state() to query the current
  toggle state. When a tristate checkbox changes state, it emits the
  stateChanged() signal.

  \important text(), setText(), text(), pixmap(), setPixmap(), accel(),
  setAccel(), isToggleButton(), setDown(), isDown(), isOn(), state(),
  autoRepeat(), isExclusiveToggle(), group(), setAutoRepeat(), toggle(),
  pressed(), released(), clicked(), toggled(), state() stateChanged()

  <img src=qchkbox-m.png> <img src=qchkbox-w.png>

  \sa QButton QRadioButton
  <a href="guibooks.html#fowler">Fowler: Check Box</a>
*/

/*! \property QCheckBox::checked
    \brief whether the checkbox is checked 
*/

/*! \property QCheckBox::tristate
    \brief whether the checkbox is a tri-state checkbox 
*/

/*!
  Constructs a checkbox with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name, WRepaintNoErase | WResizeNoErase | WMouseNoMask )
{
    setToggleButton( TRUE );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
}

/*!
  Constructs a checkbox with text \a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( const QString &text, QWidget *parent, const char *name )
	: QButton( parent, name, WRepaintNoErase | WResizeNoErase | WMouseNoMask )
{
    setText( text );
    setToggleButton( TRUE );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
}

/*!
  Sets the checkbox to the "no change" state.

  \sa setTristate()
*/
void QCheckBox::setNoChange()
{
    setTristate(TRUE);
    setState( NoChange );
}

void QCheckBox::setTristate(bool y)
{
    setToggleType( y ? Tristate : Toggle );
}

bool QCheckBox::isTristate() const
{
    return toggleType() == Tristate;
}

static int extraWidth( int gs )
{
    if ( gs == Qt::MotifStyle )
	return 8;
    else
	return 6;
}


/*!\reimp
*/
QSize QCheckBox::sizeHint() const
{
    // Any more complex, and we will use style().itemRect()
    // NB: QCheckBox::sizeHint() is similar
    constPolish();
    QSize sz;
    if (pixmap()) {
	sz = pixmap()->size();
    } else {
	sz = fontMetrics().size( ShowPrefix, text() );
    }
    GUIStyle gs = style().guiStyle();
    QSize bmsz = style().indicatorSize();
    if ( sz.height() < bmsz.height() )
	sz.setHeight( bmsz.height() );

    return sz + QSize( bmsz.width() + (style()==MotifStyle ? 1 : 0)
			+ (text().isEmpty() ? 0 : 4 + extraWidth(gs)),
			4 ).expandedTo( QApplication::globalStrut() );
}


/*!\reimp
*/

void QCheckBox::drawButton( QPainter *paint )
{
    QPainter	*p = paint;
    GUIStyle	 gs = style().guiStyle();
    const QColorGroup & g = colorGroup();
    int		 x, y;

    QFontMetrics fm = fontMetrics();
    QSize lsz = fm.size(ShowPrefix, text());
    QSize sz = style().indicatorSize();
    x = gs == MotifStyle ? 1 : 0;
    if( QApplication::reverseLayout() )
	x = width() - x -sz.width();
    y = (height() - lsz.height() + fm.height() - sz.height())/2;

#ifndef QT_NO_TEXTSTREAM
#define SAVE_CHECKBOX_PIXMAPS
#endif

#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isEnabled() )
	kf |= 2;
    if ( hasFocus() )
	kf |= 4;				// active vs. normal colorgroup
    if( topLevelWidget() != qApp->activeWindow())
	kf |= 8;

    kf |= state() << 4;
    QTextOStream os(&pmkey);
    os << "$qt_check_" << style().className() << "_"
			 << palette().serialNumber() << "_" << kf;
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( x, y, *pm );
	drawButtonLabel( p );
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx = 0, wy = 0;
    if ( use_pm ) {
	pm = new QPixmap( sz );			// create new pixmap
	Q_CHECK_PTR( pm );
	pm->fill( g.background() );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx=x;  wy=y;				// save x,y coords
	x = y = 0;
	p->setBackgroundColor( g.background() );
    }
#endif

    style().drawIndicator(p, x, y, sz.width(), sz.height(), colorGroup(), state(), isDown(), isEnabled());

#if defined(SAVE_CHECKBOX_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	if ( backgroundPixmap() || backgroundMode() == X11ParentRelative ) {
	    QBitmap bm( pm->size() );
	    bm.fill( color0 );
	    pmpaint.begin( &bm );
	    style().drawIndicatorMask( &pmpaint, 0, 0, bm.width(), bm.height(), isOn() );
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
void QCheckBox::drawButtonLabel( QPainter *p )
{
    int x, y, w, h;
    GUIStyle gs = style().guiStyle();
    QSize sz = style().indicatorSize();
    y = 0;
    x = sz.width() + extraWidth( gs ); //###
    w = width() - x;
    if ( QApplication::reverseLayout() )
	x = 0;
    h = height();

    style().drawItem( p, x, y, w, h,
		      AlignAuto|AlignVCenter|ShowPrefix,
		      colorGroup(), isEnabled(),
		      pixmap(), text() );

    if ( hasFocus() ) {
	QRect br = style().itemRect( p, x, y, w, h,
				   AlignAuto|AlignVCenter|ShowPrefix,
				   isEnabled(),
				   pixmap(), text() );
	br.setLeft( br.left()-3 );
	br.setRight( br.right()+2 );
	br.setTop( br.top()-2 );
	br.setBottom( br.bottom()+2);
	br = br.intersect( QRect(0,0,width(),height()) );

	style().drawFocusRect(p, br, colorGroup());
    }
}

/*!
  \reimp
*/
void QCheckBox::resizeEvent( QResizeEvent* )
{
    int x, w, h;
    GUIStyle gs = style().guiStyle();
    QSize sz = style().indicatorSize();
    x = sz.width() + extraWidth( gs );
    w = width() - x;
    if ( QApplication::reverseLayout() )
	x = 0;
    h = height();

    QPainter p(this);
    QRect br = style().itemRect( &p, x, 0, w, h,
				 AlignAuto|AlignVCenter|ShowPrefix,
				 isEnabled(),
				 pixmap(), text() );
    update( br.right(), w, 0, h );
    if ( autoMask() )
	updateMask();
}

/*!
  \reimp
*/
void QCheckBox::updateMask()
{
    QBitmap bm(width(),height());
    bm.fill(color0);
    QPainter p( &bm, this );
    int x, y, w, h;
    GUIStyle gs = style().guiStyle();
    QFontMetrics fm = fontMetrics();
    QSize lsz = fm.size(ShowPrefix, text());
    QSize sz = style().indicatorSize();
    y = 0;
    x = sz.width() + extraWidth(gs);
    w = width() - x;
    if ( QApplication::reverseLayout() )
	x = 0;
    h = height();

    QColorGroup cg(color1,color1,color1,color1,color1,color1,color1,color1,color0);

    style().drawItem( &p, x, y, w, h,
		      AlignAuto|AlignVCenter|ShowPrefix,
		      cg, TRUE,
		      pixmap(), text() );
    y = (height() - lsz.height() + fm.height() - sz.height())/2;

    style().drawIndicatorMask(&p, 0, y, sz.width(), sz.height(), state() );

    if ( hasFocus() ) {
	y = 0;
	QRect br = style().itemRect( &p, x, y, w, h,
				     AlignAuto|AlignVCenter|ShowPrefix,
				     isEnabled(),
				     pixmap(), text() );
	br.setLeft( br.left()-3 );
	br.setRight( br.right()+2 );
	br.setTop( br.top()-2 );
	br.setBottom( br.bottom()+2);
	br = br.intersect( QRect(0,0,width(),height()) );

	style().drawFocusRect( &p, br, cg );
    }
    setMask(bm);
}

#ifndef QT_NO_ACCESSIBILITY

/*! \reimp */
QString QCheckBox::stateDescription() const
{
    return isChecked() ? tr("checked") : tr("unchecked");
}

/*! \reimp */
QString QCheckBox::useDescription() const
{
    return isChecked() ? tr("Press the space bar to uncheck the checkbox.") : 
		         tr("Press the space bar to check the checkbox.");
}

/*! \reimp */
QString QCheckBox::typeDescription() const
{
    return tr("check box");
}

#endif

#endif
