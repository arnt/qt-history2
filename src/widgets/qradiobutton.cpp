/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.cpp#96 $
**
** Implementation of QRadioButton class
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qradiobutton.h"
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qkeycode.h"

/*!
  \class QRadioButton qradiobutton.h
  \brief The QRadioButton widget provides a radio button with a text label.

  \ingroup realwidgets

  QRadioButton and QCheckBox are both toggle buttons, that is, they can be
  switched on (checked) or off (unchecked).  Unlike check boxes, radio
  buttons are normally organized in groups where only one radio button can be
  switched on at a time.

  The QButtonGroup widget is very useful for defining groups of radio buttons.

  <img src=qradiobt-m.gif> <img src=qradiobt-w.gif>

  \sa QPushButton QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Radio Button</a>
*/


static QSize sizeOfBitmap( Qt::GUIStyle gs )
{
    switch ( gs ) {
	case Qt::WindowsStyle:
	    return QSize(12,12);
	case Qt::MotifStyle:
	    return QSize(13,13);
	default:
	    return QSize(10,10);
    }
}

static const int gutter = 6; // between button and text
static const int margin = 2; // to right of text


/*!
  Constructs a radio button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name, WResizeNoErase | WMouseNoMask )
{
    init();
}

/*!
  Constructs a radio button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( const QString &text, QWidget *parent,
			    const char *name )
	: QButton( parent, name, WResizeNoErase | WMouseNoMask )
{
    init();
    setText( text );
}

/*!
  Initializes the radio button.
*/

void QRadioButton::init()
{
    setToggleButton( TRUE );
    noHit = FALSE;
    if ( parentWidget()->inherits("QButtonGroup") ) {
	QButtonGroup *bgrp = (QButtonGroup *)parentWidget();
	bgrp->setRadioButtonExclusive( TRUE );
    }
}


/*!
  \fn bool QRadioButton::isChecked() const
  Returns TRUE if the radio button is checked, or FALSE if it is not checked.
  \sa setChecked()
*/

/*!
  \fn void QRadioButton::setChecked( bool check )
  Checks the radio button if \e check is TRUE, or unchecks it if \e check
  is FALSE.

  Calling this function does not affect other radio buttons unless a radio
  button group has been defined using the QButtonGroup widget.

  \sa isChecked()
*/


/*!
  Returns a size which fits the contents of the radio button.
*/

QSize QRadioButton::sizeHint() const
{
    // Any more complex, and we will use qItemRect()
    // NB: QCheckBox::sizeHint() is similar

    QSize sz;
    if (pixmap()) {
	sz = pixmap()->size();
    } else {
	sz = fontMetrics().size( ShowPrefix, text() );
    }
    GUIStyle gs = style();
    QSize bmsz = sizeOfBitmap( gs );
    if ( sz.height() < bmsz.height() )
	sz.setHeight( bmsz.height() );

    return sz + QSize( bmsz.width()
			+ (text().isEmpty() ? 0 : gutter+margin),
			4 );
}


/*!
  Specifies that this widget may stretch horizontally, but is fixed
  vertically.
*/

QSizePolicy QRadioButton::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}


/*!
  Reimplements QButton::hitButton().  This function is implemented to
  prevent a radio button that is \link isOn() on \endlink from being
  switched off.
*/

bool QRadioButton::hitButton( const QPoint &pos ) const
{
    return noHit ? FALSE : rect().contains( pos );
}


/*!
  Draws the radio button, but not the button label.
  \sa drawButtonLabel()
*/

void QRadioButton::drawButton( QPainter *paint )
{
    QPainter	*p = paint;
    GUIStyle	 gs = style();
    QColorGroup	 g  = colorGroup();
    int		 x, y;

    QFontMetrics fm = fontMetrics();
    QSize lsz = fm.size(ShowPrefix, text());
    QSize sz = style().exclusiveIndicatorSize();
    x = 0;
    y = (height() - lsz.height() + fm.height() - sz.height())/2;

#define SAVE_RADIOBUTTON_PIXMAPS
#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isOn() )
	kf |= 2;
    if ( isEnabled() )
	kf |= 4;
    pmkey.sprintf( "$qt_radio_%d_%d_%d", gs, palette().serialNumber(), kf );
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

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

    style().drawExclusiveIndicator(p, x, y, sz.width(), sz.height(), g, isOn(), isDown(), isEnabled() );

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
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
  Draws the radio button label.
  \sa drawButton()
*/

void QRadioButton::drawButtonLabel( QPainter *p )
{
    int x, y, w, h;
    GUIStyle gs = style();
    QSize sz = sizeOfBitmap( gs );
    if ( gs == WindowsStyle )
	sz.setWidth(sz.width()+1);
    y = 0;
    x = sz.width() + gutter;
    w = width() - x;
    h = height();

    qDrawItem( p, gs, x, y, w, h,
	       AlignLeft|AlignVCenter|ShowPrefix,
	       colorGroup(), isEnabled(),
	       pixmap(), text() );

    if ( hasFocus() ) {
	QRect br = qItemRect( p, gs, x, y, w, h,
			      AlignLeft|AlignVCenter|ShowPrefix,
			      isEnabled(),
			      pixmap(), text() );
	br.setLeft( br.left()-3 );
	br.setRight( br.right()+2 );
	br.setTop( br.top()-2 );
	br.setBottom( br.bottom()+2);
	br = br.intersect( QRect(0,0,width(),height()) );

	if ( gs == WindowsStyle ) {
	    p->drawWinFocusRect( br, backgroundColor() );
	} else {
	    p->setPen( black );
	    p->drawRect( br );
	}
    }
}

void QRadioButton::resizeEvent( QResizeEvent* e )
{
    QButton::resizeEvent(e);
    int x, w, h;
    GUIStyle gs = style();
    QSize sz = sizeOfBitmap( gs );
    if ( gs == WindowsStyle )
	sz.setWidth(sz.width()+1);
    x = sz.width() + gutter;
    w = width() - x;
    h = height();

    QPainter p(this);
    QRect br = qItemRect( &p, gs, x, 0, w, h,
			  AlignLeft|AlignVCenter|ShowPrefix,
			  isEnabled(),
			  pixmap(), text() );
    if ( autoMask() )
	updateMask();
    update( br.right(), w, 0, h );
}

void QRadioButton::updateMask()
{
    QBitmap bm(width(),height());
    {
	bm.fill(color0);
	QPainter p(&bm);
	int x, y, w, h;
	GUIStyle gs = style();
	QFontMetrics fm = fontMetrics();
	QSize lsz = fm.size(ShowPrefix, text());
	QSize sz = sizeOfBitmap( gs );
	if ( gs == WindowsStyle )
	    sz.setWidth(sz.width()+1);
	y = 0;
	x = sz.width() + gutter;
	w = width() - x;
	h = height();

	QColorGroup cg(color1,color1, color1,color1,color1,color1,color1,color1, color0);

	qDrawItem( &p, gs, x, y, w, h,
		   AlignLeft|AlignVCenter|ShowPrefix,
		   cg, TRUE,
		   pixmap(), text() );
	x = 0;
	y = (height() - lsz.height() + fm.height() - sz.height())/2;
	
	style().drawExclusiveIndicatorMask(&p, x, y, sz.width(), sz.height(), isOn() );

	if ( hasFocus() ) {
 	    y = 0;
 	    x = sz.width() + gutter;
 	    w = width() - x;
 	    h = height();
	    QRect br = qItemRect( &p, gs, x, y, w, h,
				  AlignLeft|AlignVCenter|ShowPrefix,
				  isEnabled(),
				  pixmap(), text() );
	    br.setLeft( br.left()-3 );
	    br.setRight( br.right()+2 );
	    br.setTop( br.top()-2 );
	    br.setBottom( br.bottom()+2);
	    br = br.intersect( QRect(0,0,width(),height()) );

	    if ( gs == WindowsStyle ) {
		p.drawWinFocusRect( br );
	    } else {
		p.setPen( color1 );
		p.drawRect( br );
	    }
	}
    }
    setMask(bm);
}

