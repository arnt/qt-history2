/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogressbar.cpp#51 $
**
** Implementation of QProgressBar class
**
** Created : 970521
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

#include "qprogressbar.h"
#ifndef QT_NO_PROGRESSBAR
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qpixmap.h"
#include <limits.h>

/*!
  \class QProgressBar qprogressbar.h
  \brief The QProgressBar widget provides a horizontal progress bar.
  \ingroup advanced

  A progress bar is used to give the user an indication of progress
  of an operation and to reassure user that the application has not crashed.

  QProgressBar only implements the basic progress display, whereas
  QProgressDialog provides a fuller encapsulation.

  <img src=qprogbar-m.png> <img src=qprogbar-w.png>

  \sa QProgressDialog
  <a href="guibooks.html#fowler">GUI Design Handbook: Progress Indicator</a>
*/


/*!
  Constructs a progress bar.

  The total number of steps is set to 100 by default.

  \e parent, \e name and \e f are sent to the QFrame::QFrame()
  constructor.

  \sa setTotalSteps()
*/

QProgressBar::QProgressBar( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | WRepaintNoErase | WResizeNoErase ),
      total_steps( 100 ),
      progress_val( -1 ),
      percentage( -1 ),
      center_indicator( TRUE ),
      auto_indicator( TRUE ),
      percentage_visible( TRUE ),
      d( 0 )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    initFrame();
}


/*!
  Constructs a progress bar.

  \a totalSteps is the total number of steps in the operation of which
  this progress bar shows the progress.  For example, if the operation
  is to examine 50 files, this value would be 50. Before
  examining the first file, call setProgress(0); call setProgress(50) after examining
  the last file .

  \e parent, \e name and \e f are sent to the QFrame::QFrame()
  constructor.

  \sa setTotalSteps(), setProgress()
*/

QProgressBar::QProgressBar( int totalSteps,
			    QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | WRepaintNoErase | WResizeNoErase ),
      total_steps( totalSteps ),
      progress_val( -1 ),
      percentage( -1 ),
      center_indicator( TRUE ),
      auto_indicator( TRUE ),
      percentage_visible( TRUE ),
      d( 0 )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    initFrame();
}


/*!
  Reset the progress bar.
  The progress bar "rewinds."
*/

void QProgressBar::reset()
{
    progress_val = -1;
    percentage = -1;
    setIndicator(progress_str, progress_val, total_steps);
    update();
    if ( autoMask() )
	updateMask();
}


/*!
  \fn int QProgressBar::totalSteps() const
  Returns the total number of steps.
  \sa setTotalSteps()
*/

/*!
  Sets the total number of steps to \a totalSteps. If \a totalSteps is null,
  the progress bar will display a busy indicator.

  \sa totalSteps()
*/

void QProgressBar::setTotalSteps( int totalSteps )
{
    bool clear = ( totalSteps != total_steps ) || !totalSteps;
    total_steps = totalSteps;
    if ( isVisible() ) {
	if ( setIndicator(progress_str, progress_val, total_steps) || !total_steps ) {
	    repaint( clear );
	    if ( autoMask() )
		updateMask();
	}
    }
}


/*!
  \fn int QProgressBar::progress() const
  Returns the current amount of progress, or -1 if the progress counting
  has not started.
  \sa setProgress()
*/

/*!
  Sets the current amount of progress made to \e progress units of the
  total number of steps, or moves the busy indicator if totalSteps is null.
  \sa progress(), totalSteps()
*/

void QProgressBar::setProgress( int progress )
{
    if ( progress == progress_val ||
	 progress < 0 || ( ( progress > total_steps ) && total_steps ) )
	return;

    progress_val = progress;

    setIndicator( progress_str, progress_val, total_steps );

    update();

    if ( !isVisible() )
	return;
    if ( autoMask() )
	updateMask();
}


/*!\reimp
*/
QSize QProgressBar::sizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();
    return QSize( style().progressChunkWidth() * 7 + fm.width( '0' ) * 4, fm.height()+8);
}


/*!
  \reimp
*/
QSize QProgressBar::minimumSizeHint() const
{
    return sizeHint();
}

/*!
  \fn bool QProgressBar::centerIndicator() const

  Returns where the indicator string should be displayed if
  indicatorFollowsStyle() is TRUE.

  \sa setCenterIndicator(), indicatorFollowsStyle(),
      setIndicatorFollowsStyle(), setIndicator()
*/

/*!
  If set to TRUE (the default), the progress bar always shows the indicator
  text at the center of the progress bar regardless of the GUI style
  currently set.  If set to FALSE, the progress bar always shows the
  indicator text outside the progress bar, regardless of the GUI style
  currently set.

  Calling this function always sets indicatorFollowsStyle() to FALSE.

  \sa centerIndicator(), indicatorFollowsStyle(), setIndicatorFollowsStyle(),
      setIndicator()
 */
void QProgressBar::setCenterIndicator( bool on )
{
    if ( !auto_indicator && on == center_indicator )
	return;
    auto_indicator   = FALSE;
    center_indicator = on;
    repaint( TRUE );
    if ( autoMask() )
	updateMask();
}

/*!
  \fn bool QProgressBar::indicatorFollowsStyle() const

  Returns whether the display of the indicator string should follow the
  GUI style or not.

  \sa setIndicatorFollowsStyle(), setCenterIndicator(), centerIndicator()
      setIndicator()
*/

/*!
  When set to TRUE (the default), the positioning of the indicator string
  follows the GUI style. When set to FALSE, the indicator position is decided
  by the value of indicatorFollowsStyle().

  \sa indicatorFollowsStyle(), centerIndicator(), setCenterIndicator(),
      setIndicator()
 */
void QProgressBar::setIndicatorFollowsStyle( bool on )
{
    if ( on == auto_indicator )
	return;
    auto_indicator = on;
    repaint( TRUE );
    if ( autoMask() )
	updateMask();
}

/*!
  \fn bool QProgressBar::percentageVisible() const

  Returns whether the current progress value is displayed or not.

  \sa setPercentageVisible, setCenterIndicator
*/

/*!
  When set to TRUE (the default), the current progress value is displayed
  according to the GUI style. Otherwise, no value is displayed.

  \sa indicatorFollowsStyle(), setCenterIndicator()
*/
void QProgressBar::setPercentageVisible( bool on )
{
    if ( on == percentage_visible )
	return;
    percentage_visible = on;
    repaint( TRUE );
}

/*!
  \reimp
*/
void QProgressBar::show()
{
    setIndicator( progress_str, progress_val, total_steps );
    QFrame::show();
}

void QProgressBar::initFrame()
{
    setFrameStyle(QFrame::NoFrame);
}

/*! \reimp
 */
void QProgressBar::styleChange( QStyle& old )
{
    initFrame();
    QFrame::styleChange( old );
}


/*!
  This method is called to generate the text displayed in the center of
  the progress bar.

  The progress may be negative, indicating that the bar is in the "reset" state
  before any progress is set.

  The default implementation is the percentage of completion or blank in the
  reset state.

  To allow efficient repainting of the progress bar, this method should return FALSE if the string is unchanged from the
  last call to the method, .
*/

bool QProgressBar::setIndicator( QString & indicator, int progress,
				 int totalSteps )
{
    if ( !totalSteps )
	return FALSE;
    if ( progress < 0 ) {
	indicator = QString::fromLatin1("");
	return TRUE;
    } else {
	// Get the values down to something usable.
	if ( totalSteps > INT_MAX/1000 ) {
	    progress /= 1000;
	    totalSteps /= 1000;
	}

	int np = progress * 100 / totalSteps;
	if ( np != percentage ) {
	    percentage = np;
	    indicator.sprintf( "%d%%", np );
	    return TRUE;
	} else {
	    return FALSE;
	}
    }
}


/*!\reimp
*/
void QProgressBar::drawContents( QPainter *p )
{
    const QRect bar = contentsRect();

    QPixmap pm;
    pm.resize( bar.size() );

    QPainter paint( &pm );
    QBrush fbrush = backgroundPixmap() ? QBrush( backgroundColor(), *backgroundPixmap() ) : QBrush( backgroundColor() );
    paint.fillRect( bar, fbrush );

    paint.setFont( p->font() );

    if ( !total_steps ) { // draw busy indicator
/*
	} else {
	    bool right = TRUE;
	    int bw = u*unit_width + 2;
	    int x = progress_val % ( (bw-ox) * 2 );
	    if ( x > bw-ox ) {
		x = 2 * (bw-ox) - x;
		right = FALSE;
	    }
	    x += ox + bar.x();

	    QRect all( ox + bar.x(), bar.y(), bw-1, bar.height() );
	    QRect ind( x, bar.y(), 18, bar.height() );

	    p->setClipRegion( QRegion( all ) - QRegion( ind ) );
	    p->eraseRect( all );

	    if ( progress_val > -1 ) {
		p->setClipRegion( QRegion( all ) );
		QColor base = colorGroup().background();
		QColor high = palette().active().highlight();
		int dr = ( base.red() - high.red() ) / 20;
		int dg = ( base.green() - high.green() ) / 20;
		int db = ( base.blue() - high.blue() ) / 20;
		for ( int i = 0; i < 20; i++ ) {
		    QColor d;
		    if ( right )
			d.setRgb( base.red() - dr*i, base.green() - dg*i, base.blue() - db * i );
		    else
			d.setRgb( high.red() + dr*i, high.green() + dg*i, high.blue() + db * i );
		    p->setPen( d );
		    p->drawLine( x+i, bar.y(), x+i, bar.height() );
		}
	    }

	    p->setClipping( FALSE );
	}
*/
/*
	} else {
	    int bw = bar.width();
	    int x = progress_val % ( bw * 2 );
	    if ( x > bw )
		x = 2 * bw - x;
	    x += bar.x();

	    p->setClipRegion( QRegion( bar ) - QRegion( x, bar.y(), 1, bar.height() ) );
	    p->eraseRect( bar );
	    p->setClipRect( bar );
	    p->setPen( colorGroup().highlight() );
	    p->drawLine( x, bar.y(), x, bar.height() );
	}
*/
    } else {
	const int unit_width = style().progressChunkWidth();

	bool hasExtraIndicator = percentage_visible && total_steps && (
				 style() != MotifStyle && auto_indicator || 
				!auto_indicator && !center_indicator );

	int textw = 0;
	if ( hasExtraIndicator ) {
	    QFontMetrics fm = p->fontMetrics();
	    textw = fm.width(QString::fromLatin1("100%")) + 6;
	}
	int u = (bar.width() - textw ) / unit_width;
	int p_v = progress_val;
	int t_s = total_steps;
	if ( u > 0 && progress_val >= INT_MAX / u && t_s >= u ) {
	    // scale down to something usable.
	    p_v /= u;
	    t_s /= u;
	}
	int nu = ( u * p_v + t_s/2 ) / t_s;

	style().drawProgressBar( &paint, bar.x(), bar.y(), u*unit_width + 4, bar.height(), colorGroup() );
	
	// Draw nu units out of a possible u of unit_width width, each
	// a rectangle bordered by background color, all in a sunken panel
	// with a percentage text display at the end.
	int x = 0;
	for (int i=0; i<nu; i++) {
	    style().drawProgressChunk( &paint, bar.x() + x + 2, bar.y() + 2,
			 unit_width, bar.height() - 4, palette().active() );
	    x += unit_width;
	}
	if ( !hasExtraIndicator && percentage_visible && total_steps ) {
	    paint.setPen( colorGroup().highlightedText() );
	    paint.setClipRect( bar.x(), bar.y(), x+2, bar.height() );
	    paint.drawText( bar, AlignCenter | SingleLine, progress_str );
	    if ( progress_val != total_steps ) {
		paint.setClipRect( bar.x() + x+2, bar.y(), bar.width() - x - 2, bar.height() );
		paint.setPen( colorGroup().highlight() );
		paint.drawText( bar, AlignCenter | SingleLine, progress_str );	    
	    }
	} else if ( hasExtraIndicator ) {
	    paint.setPen( colorGroup().foreground() );
	    paint.drawText( bar.x()+u*unit_width + 4, bar.y(), textw, bar.height(),
		AlignCenter | SingleLine, progress_str );
	}
    }

    paint.end();

    p->drawPixmap( bar.x(), bar.y(), pm );
}


/*!
  Draws the progress bar contents mask using the painter \e p.
  Used only in transparent mode.

  \sa QWidget::setAutoMask();
*/
void QProgressBar::drawContentsMask( QPainter *p )
{
    const QRect bar = contentsRect();

    if ( !total_steps ) {
    } else {
	const int unit_width  = style().progressChunkWidth();
	bool hasExtraIndicator = percentage_visible && total_steps && (
				 style() != MotifStyle && auto_indicator || 
				!auto_indicator && !center_indicator );
	int textw = 0;
	if ( hasExtraIndicator ) {
	    QFontMetrics fm = p->fontMetrics();
	    textw = fm.width(QString::fromLatin1("100%")) + 6;
	}
	int u = (bar.width() - textw ) / unit_width;
	int p_v = progress_val;
	int t_s = total_steps;
	if ( u > 0 && progress_val >= INT_MAX / u && t_s >= u ) {
	    // scale down to something usable.
	    p_v /= u;
	    t_s /= u;
	}
	int nu = ( u * p_v + t_s/2 ) / t_s;
	const QRect r( bar.x(), bar.y(), u*unit_width + 4, bar.height() );

	p->fillRect( bar.x() + 2, bar.y() + 2, unit_width * nu, bar.height() - 4, color1 );
	if ( !hasExtraIndicator && percentage_visible && total_steps ) {
	    p->setPen( color1 );
	    p->drawText( bar, AlignCenter, progress_str );
	} else if ( hasExtraIndicator ) {
	    p->setPen( color1 );
	    p->drawText( r.x()+r.width(), bar.y(), textw, bar.height(),
		AlignRight | AlignVCenter, progress_str );
	}
    }
}
#endif
