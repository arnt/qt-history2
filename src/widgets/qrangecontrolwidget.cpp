/****************************************************************************
**
** Implementation of QRangeControlWidget class
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

#include "qrangecontrol.h"

#ifndef QT_NO_RANGECONTROLWIDGET

#include "qrect.h"
#include "qtimer.h"
#include "qapplication.h"

static uint theButton = 0;

class QRangeControlWidgetPrivate
{
public:
    QRangeControlWidgetPrivate()
    {
	upEnabled = TRUE;
	downEnabled = TRUE;
	buttonDown = 0;
	up = QRect();
	down = QRect();
	auRepTimer = 0;
	bsyms = QRangeControlWidget::UpDownArrows;
    }
    bool upEnabled;
    bool downEnabled;
    uint buttonDown;
    QRect up;
    QRect down;
    QTimer *auRepTimer;
    QRangeControlWidget::ButtonSymbols bsyms;
};

/*!  Constructs an empty range control widget.

*/

QRangeControlWidget::QRangeControlWidget( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    d = new QRangeControlWidgetPrivate();
    setFocusPolicy( QWidget::NoFocus );
    arrange();
    updateDisplay();
}


/*! Destroys the object and frees any allocated resources.

*/

QRangeControlWidget::~QRangeControlWidget()
{
    delete d;
}

/*! \reimp

*/

void QRangeControlWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;

    uint oldButtonDown = d->buttonDown;

    if ( d->down.contains( e->pos() ) && d->downEnabled )
	d->buttonDown = 1;
    else if ( d->up.contains( e->pos() ) && d->upEnabled )
	d->buttonDown = 2;
    else
	d->buttonDown = 0;

    theButton = d->buttonDown;
    if ( oldButtonDown != d->buttonDown ) {
	if ( !d->buttonDown ) {
	    repaint( d->down.unite( d->up ), FALSE );
	} else if ( d->buttonDown & 1 ) {
	    repaint( d->down, FALSE );
	    if ( !d->auRepTimer ) {
		d->auRepTimer = new QTimer( this );
		connect( d->auRepTimer, SIGNAL( timeout() ), this, SLOT( stepDown() ) );
		d->auRepTimer->start( 300 );
	    }
	    stepDown();
	} else if ( d->buttonDown & 2 ) {
	    repaint( d->up, FALSE );
	    if ( !d->auRepTimer ) {
		d->auRepTimer = new QTimer( this );
		connect( d->auRepTimer, SIGNAL( timeout() ), this, SLOT( stepUp() ) );
		d->auRepTimer->start( 300 );
	    }
	    stepUp();
	}
    }
}

/*!

*/

void QRangeControlWidget::arrange()
{
    QSize bs;

    bs.setHeight( height()/2 );
    bs.setWidth( width() );

    if ( bs.height() < 8 )
	bs.setHeight( 8 );
    bs = bs.expandedTo( QApplication::globalStrut() );

    d->up = QRect( 0, 0, bs.width(), bs.height() );
    d->down = QRect( 0, bs.height(), bs.width(), bs.height() );
}

void QRangeControlWidget::stepUp()
{
    if ( d->auRepTimer && sender() == d->auRepTimer ) {
	d->auRepTimer->stop();
	d->auRepTimer->start( 100 );
    }
    emit stepUpPressed();
}

void QRangeControlWidget::resizeEvent( QResizeEvent* )
{
    arrange();
}

/*!

*/

void QRangeControlWidget::stepDown()
{
    if ( d->auRepTimer && sender() == d->auRepTimer ) {
	d->auRepTimer->stop();
	d->auRepTimer->start( 100 );
    }
    emit stepDownPressed();
}


/*!

*/

void QRangeControlWidget::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;

    uint oldButtonDown = theButton;
    theButton = 0;
    if ( oldButtonDown != theButton ) {
	if ( oldButtonDown & 1 )
	    repaint( d->down, FALSE );
	else if ( oldButtonDown & 2 )
	    repaint( d->up, FALSE );
    }
    delete d->auRepTimer;
    d->auRepTimer = 0;

    d->buttonDown = 0;
}


/*!

*/

void QRangeControlWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ( !(e->state() & LeftButton ) )
	return;

    uint oldButtonDown = theButton;
    if ( oldButtonDown & 1 && !d->down.contains( e->pos() ) ) {
	if ( d->auRepTimer )
	    d->auRepTimer->stop();
	theButton = 0;
	repaint( d->down, FALSE );
    } else if ( oldButtonDown & 2 && !d->up.contains( e->pos() ) ) {
	if ( d->auRepTimer )
	    d->auRepTimer->stop();
	theButton = 0;
	repaint( d->up, FALSE );
    } else if ( !oldButtonDown && d->up.contains( e->pos() ) && d->buttonDown & 2 ) {
	if ( d->auRepTimer )
	    d->auRepTimer->start( 500 );
	theButton = 2;
	repaint( d->up, FALSE );
    } else if ( !oldButtonDown && d->down.contains( e->pos() ) && d->buttonDown & 1 ) {
	if ( d->auRepTimer )
	    d->auRepTimer->start( 500 );
	theButton = 1;
	repaint( d->down, FALSE );
    }
}


/*!

*/

void QRangeControlWidget::wheelEvent( QWheelEvent *e )
{
    e->accept();
    static float offset = 0;
    static QRangeControlWidget* offset_owner = 0;
    if ( offset_owner != this ) {
	offset_owner = this;
	offset = 0;
    }
    offset += -e->delta()/120;
    if ( QABS( offset ) < 1 )
	return;
    int ioff = int(offset);
    int i;
    for( i=0; i < QABS( ioff ); i++ )
	offset > 0 ? stepDown() : stepUp();
    offset -= ioff;
}


/*!

*/

void QRangeControlWidget::drawContents( QPainter *p )
{
    style().drawRangeControlWidgetButton( p, d->down.x(), d->down.y(), d->down.width(), d->down.height(),
					  d->downEnabled ? colorGroup() : palette().disabled(),
					  this, TRUE, d->downEnabled, theButton & 1 );
    style().drawRangeControlWidgetSymbol( p, d->down.x(), d->down.y(), d->down.width(), d->down.height(),
					  d->downEnabled ? colorGroup() : palette().disabled(),
					  this, TRUE, d->downEnabled, theButton & 1 );

    style().drawRangeControlWidgetButton( p, d->up.x(), d->up.y(), d->up.width(), d->up.height(),
					  d->upEnabled ? colorGroup() : palette().disabled(),
					  this, FALSE, d->upEnabled, theButton & 2 );
    style().drawRangeControlWidgetSymbol( p, d->up.x(), d->up.y(), d->up.width(), d->up.height(),
					  d->upEnabled ? colorGroup() : palette().disabled(),
					  this, FALSE, d->upEnabled, theButton & 2 );
}


/*!

*/

void QRangeControlWidget::styleChange( QStyle& old )
{
    if ( style() == WindowsStyle )
	setFrameStyle( WinPanel | Sunken );
    else
	setFrameStyle( Panel | Sunken );

    arrange();
    QWidget::styleChange( old );
}


/*!

*/

QRect QRangeControlWidget::upRect() const
{
    return d->up;
}


/*!

*/

QRect QRangeControlWidget::downRect() const
{
    return d->down;
}


/*!

*/

void QRangeControlWidget::updateDisplay()
{
    if ( !isEnabled() ) {
	d->upEnabled = FALSE;
	d->downEnabled = FALSE;
    }
    if ( theButton & 1 && ( d->downEnabled ) == 0 ) {
	theButton &= ~1;
	d->buttonDown &= ~1;
    }

    if ( theButton & 2 && ( d->upEnabled ) == 0 ) {
	theButton &= ~2;
	d->buttonDown &= ~2;
    }
    repaint( FALSE );
}


/*!

*/

void QRangeControlWidget::setEnabled( bool on )
{
    bool b = isEnabled();
    d->upEnabled = on;
    d->downEnabled = on;
    QFrame::setEnabled( on );
    if ( isEnabled() != b )
	updateDisplay();
}


/*!

*/

void QRangeControlWidget::setUpEnabled( bool on )
{
    if ( d->upEnabled != on ) {
	d->upEnabled = on;
	updateDisplay();
    }
}


/*!

*/

void QRangeControlWidget::setDownEnabled( bool on )
{
    if ( d->downEnabled != on ) {
	d->downEnabled = on;
	updateDisplay();
    }
}


/*!

*/

void QRangeControlWidget::setButtonSymbols( ButtonSymbols bs )
{
    d->bsyms = bs;
}


/*!

*/

QRangeControlWidget::ButtonSymbols QRangeControlWidget::buttonSymbols() const
{
    return d->bsyms;
}
#endif
