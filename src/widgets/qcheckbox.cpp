/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.cpp#104 $
**
** Implementation of QCheckBox class
**
** Created : 940222
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qcheckbox.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qtextstream.h"

// NOT REVISED
/*!
  \class QCheckBox qcheckbox.h
  \brief The QCheckBox widget provides a check box with a text label.

  \ingroup realwidgets

  QCheckBox and QRadioButton are both option buttons. That is, they
  can be switched on (checked) or off (unchecked). The classes differ
  in how the choices for the user are restricted. Radio buttons define
  a "one of many" choice, while check-boxes provide "many of many"
  choices.

  While it is technically possible to imlement radio-behaviour with
  check boxes and vice versa, it's strongly recommended to stick with
  the well-known semantics. Otherwise your users would be pretty
  confused.

  Use QButtonGroup to group check-buttons visually.

  Whenver a check box is checked or cleared, it emits the signal
  toggled(). Connect to this signal if you want to trigger an action
  each time the box changes state. Otherwise, use isChecked() to query
  whether or not a particular check box is selected.

  In addition to the usual checked and unchecked states, QCheckBox
  optionally provides a third state to indicate "no change".  This is
  useful whenever you need to give the user the option of neither
  setting nor unsetting an option. If you need that third state,
  enable it with setTristate() and use state() to query the current
  toggle state. When a tristate box changes state, it emits the
  stateChanged() signal.

  \important text, setText, text, pixmap, setPixmap, accel, setAccel,
  isToggleButton, setDown, isDown, isOn, state, autoRepeat,
  isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released,
  clicked, toggled, state stateChanged

  <img src=qchkbox-m.png> <img src=qchkbox-w.png>

  \sa QButton QRadioButton
  <a href="guibooks.html#fowler">Fowler: Check Box.</a>
*/



/*!
  Constructs a check box with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name, WResizeNoErase | WMouseNoMask )
{
    setToggleButton( TRUE );
}

/*!
  Constructs a check box with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( const QString &text, QWidget *parent, const char *name )
	: QButton( parent, name, WResizeNoErase | WMouseNoMask )
{
    setText( text );
    setToggleButton( TRUE );
}


/*!
  \fn bool QCheckBox::isChecked() const
  Returns TRUE if the check box is checked, or FALSE if it is not checked.
  \sa setChecked()
*/

/*!
  \fn void QCheckBox::setChecked( bool check )
  Checks the check box if \e check is TRUE, or unchecks it if \e check
  is FALSE.
  \sa isChecked()
*/

/*!
  Sets the checkbox into the "no change" state.

  \sa setTristate()
*/
void QCheckBox::setNoChange()
{
    setTristate(TRUE);
    setState( NoChange );
}

/*!
  Makes the check box a tristate check box if \a y is TRUE.  A tristate
  check box provides an additional state NoChange.

  Use tristate check boxes whenever you need to give the user the
  option of neither setting nor unsetting an option. A typical example
  is the "Italic" check box in the font dialog of a word processor
  when the marked text is partially Italic and partially not.

  \sa setNoChange() stateChanged(), state()
*/
void QCheckBox::setTristate(bool y)
{
    setToggleType( y ? Tristate : Toggle );
}

static int extraWidth( int gs )
{
    if ( gs == Qt::MotifStyle )
	return 8;
    else
	return 6;
}


/*!
  Returns a size which fits the contents of the check box.
*/

QSize QCheckBox::sizeHint() const
{
    // Any more complex, and we will use style().itemRect()
    // NB: QCheckBox::sizeHint() is similar

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
			4 );
}


/*!
  Draws the check box.  Calls drawButtonLabel() to draw the content.
  \sa drawButtonLabel()
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
    y = (height() - lsz.height() + fm.height() - sz.height())/2;

#define SAVE_CHECKBOX_PIXMAPS
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isEnabled() )
	kf |= 2;
    if ( hasFocus() )
	kf |= 4;				// active vs. normal colorgroup
    kf |= state() << 3;
    QTextOStream(&pmkey) << "$qt_check_" << style().className() << "_"
			 << palette().serialNumber() << "_" << kf;
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( x, y, *pm );
	drawButtonLabel( p );
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixmap( sz );			// create new pixmap
	CHECK_PTR( pm );
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
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	if (!QPixmapCache::insert(pmkey, pm) )	// save in cache
	    delete pm;
    }
#endif

    drawButtonLabel( p );
}


/*!
  Draws the check box label.
  \sa drawButton()
*/

void QCheckBox::drawButtonLabel( QPainter *p )
{
    int x, y, w, h;
    GUIStyle gs = style().guiStyle();
    QSize sz = style().indicatorSize();
    y = 0;
    x = sz.width() + extraWidth( gs ); //###
    w = width() - x;
    h = height();

    style().drawItem( p, x, y, w, h,
		      AlignLeft|AlignVCenter|ShowPrefix,
		      colorGroup(), isEnabled(),
		      pixmap(), text() );

    if ( hasFocus() ) {
	QRect br = style().itemRect( p, x, y, w, h,
				   AlignLeft|AlignVCenter|ShowPrefix,
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

void QCheckBox::resizeEvent( QResizeEvent* )
{
    int x, w, h;
    GUIStyle gs = style().guiStyle();
    QSize sz = style().indicatorSize();
    x = sz.width() + extraWidth( gs );
    w = width() - x;
    h = height();

    QPainter p(this);
    QRect br = style().itemRect( &p, x, 0, w, h,
				 AlignLeft|AlignVCenter|ShowPrefix,
				 isEnabled(),
				 pixmap(), text() );
    update( br.right(), w, 0, h );
    if ( autoMask() )
	updateMask();
}

void QCheckBox::updateMask()
{

    QBitmap bm(width(),height());
    {
	int x, y, w, h;
	GUIStyle gs = style().guiStyle();
	bm.fill(color0);
	QPainter p( &bm, this );

	QColorGroup cg(color1,color1,color1,color1,color1,color1,color1,color1, color0);
	QFontMetrics fm = fontMetrics();
	QSize lsz = fm.size(ShowPrefix, text());
	QSize sz = style().indicatorSize();
	x = gs == MotifStyle ? 1 : 0;
	y = (height() - lsz.height() + fm.height() - sz.height())/2;
	
	style().drawIndicatorMask(&p, x, y, sz.width(), sz.height(), state() );

	sz = style().indicatorSize();
	y = 0;
	x = sz.width() + extraWidth( gs );
	w = width() - x;
	h = height();
	style().drawItem( &p, x, y, w, h,
			  AlignLeft|AlignVCenter|ShowPrefix,
			  cg, TRUE,
			  pixmap(), text() );

	if ( hasFocus() ) {
	    QRect br = style().itemRect( &p, x, y, w, h,
					 AlignLeft|AlignVCenter|ShowPrefix,
					 isEnabled(),
					 pixmap(), text() );
	    br.setLeft( br.left()-3 );
	    br.setRight( br.right()+2 );
	    br.setTop( br.top()-2 );
	    br.setBottom( br.bottom()+2);
	    br = br.intersect( QRect(0,0,width(),height()) );

	    style().drawFocusRect( &p, br, cg );
	}
    }
    setMask(bm);
}


/*!
  Specifies that this widget may stretch horizontally, but is fixed
  vertically.
*/

QSizePolicy QCheckBox::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}
