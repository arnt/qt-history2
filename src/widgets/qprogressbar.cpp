/****************************************************************************
** $Id: $
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
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
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

  \a parent, \a name and \a f are sent to the QFrame::QFrame()
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

  \a parent, \a name and \a f are sent to the QFrame::QFrame()
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
    repaint( FALSE );
}


/*!
  \property QProgressBar::totalSteps
  \brief The total number of steps.

  If totalSteps is null, the progress bar will display a busy indicator.

  \sa totalSteps()
*/

void QProgressBar::setTotalSteps( int totalSteps )
{
    total_steps = totalSteps;
    if ( isVisible() &&
	 ( setIndicator(progress_str, progress_val, total_steps) || !total_steps ) )
	repaint( FALSE );
}


/*!
  \property QProgressBar::progress
  \brief the current amount of progress

  This property is -1 if the progress counting has not started.
*/

void QProgressBar::setProgress( int progress )
{
    if ( progress == progress_val ||
	 progress < 0 || ( ( progress > total_steps ) && total_steps ) )
	return;

    progress_val = progress;

    setIndicator( progress_str, progress_val, total_steps );

    repaint( FALSE );

#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
}

/*!
  \property QProgressBar::progressString
  \brief the current amount of progress as a string

  This property is QString::null is the progress counting has not started.
*/


/*!\reimp
*/
QSize QProgressBar::sizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();
    int cw = style().pixelMetric(QStyle::PM_ProgressBarChunkWidth, this);
    return style().sizeFromContents(QStyle::CT_ProgressBar, this,
				    QSize( cw * 7 + fm.width( '0' ) * 4,
					   fm.height() + 8));
}


/*!
  \reimp
*/
QSize QProgressBar::minimumSizeHint() const
{
    return sizeHint();
}

/*!
  \property QProgressBar::centerIndicator

  \brief where the indicator string should be displayed

  If set to TRUE, the indicator is displayed centered.
  Changing this property sets indicatorFollowsStyle to FALSE.

  \sa indicatorFollowsStyle
*/

void QProgressBar::setCenterIndicator( bool on )
{
    if ( !auto_indicator && on == center_indicator )
	return;
    auto_indicator   = FALSE;
    center_indicator = on;
    repaint( FALSE );
}

/*!
  \property QProgressBar::indicatorFollowsStyle
  \brief whether the display of the indicator string should follow the GUI style or not.

  \sa centerIndicator
*/

void QProgressBar::setIndicatorFollowsStyle( bool on )
{
    if ( on == auto_indicator )
	return;
    auto_indicator = on;
    repaint( FALSE );
}

/*!
  \property QProgressBar::percentageVisible
  \brief whether the current progress value is displayed or not.
*/
void QProgressBar::setPercentageVisible( bool on )
{
    if ( on == percentage_visible )
	return;
    percentage_visible = on;
    repaint( FALSE );
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

  The \a progress may be negative, indicating that the bar is in the
  "reset" state before any progress is set. 

  The default implementation is the percentage of completion or blank in the
  reset state. The percentage is calculated based on the \a progress
  and \a totalSteps. You can set the \a indicator text if you wish.

  To allow efficient repainting of the progress bar, this method
  should return FALSE if the string is unchanged from the last call to
  the function.
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

    QPixmap pm( bar.size() );
    QPainter paint( &pm );
    QBrush fbrush = ( backgroundPixmap() ?
		      QBrush( backgroundColor(), *backgroundPixmap() ) :
		      QBrush( backgroundColor() ) );
    paint.fillRect( bar, fbrush );
    paint.setFont( p->font() );
    style().drawControl(QStyle::CE_ProgressBar, &paint, this,
			QStyle::visualRect(style().subRect(QStyle::SR_ProgressBarContents, this), this ),
			colorGroup());
    if (percentageVisible())
	style().drawControl(QStyle::CE_ProgressBarLabel, &paint, this,
			    QStyle::visualRect(style().subRect(QStyle::SR_ProgressBarLabel, this), this ),
			    colorGroup());
    paint.end();

    p->drawPixmap( bar.x(), bar.y(), pm );
}

#endif
