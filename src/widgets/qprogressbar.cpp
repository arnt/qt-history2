/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogressbar.cpp#11 $
**
** Implementation of QProgressBar class
**
** Created : 970521
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprogbar.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qprogressbar.cpp#11 $");


/*!
  \class QProgressBar qprogbar.h
  \brief The QProgressBar widget provides a horizontal progress bar.
  \ingroup realwidgets

  A progress bar is used to give the user an indication of progress
  of an operation. To reassure them that the application has not crashed.

  QProgressBar only implements the basic progress display, while
  QProgressDialog provides a fuller encapsulation.

  <img src=qprogbar-m.gif> <img src=qprogbar-w.gif>
*/


/*!
  Constructs a progress bar.

  The total number of steps is set to 100 by default.

  \arg \e parent, \e name and \e f are sent to the QFrame::QFrame()
  constructor.

  \sa setTotalSteps()
*/

QProgressBar::QProgressBar( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f ),
	total_steps( 100 ),
	progress_val( -1 ),
	percentage( -1 )
{
    if ( style() == MotifStyle ) {
	setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	setLineWidth( 2 );
    }
}


/*!
  Constructs a progress bar.

  \arg \e totalSteps is the total number of steps in the operation of which
    this progress bar shows the progress.  For example, if the operation
    is to examine 50 files, this value would be 50, then before examining
    the first file, call setProgress(0), and after examining the last file
    call setProgress(50).
  \arg \e parent, \e name and \e f are sent to the QFrame::QFrame()
  constructor.

  \sa setTotalSteps(), setProgress()
*/

QProgressBar::QProgressBar( int totalSteps,
			    QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f ),
	total_steps( totalSteps ),
	progress_val( -1 ),
	percentage( -1 )
{
    if ( style() == MotifStyle ) {
	setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	setLineWidth( 2 );
    }
}


/*!
  Reset the progress bar.
  The progress bar `rewinds'.
*/

void QProgressBar::reset()
{
    progress_val = -1;
    percentage = -1;
}


/*!
  \fn int QProgressBar::totalSteps() const
  Returns the total number of steps.
  \sa setTotalSteps()
*/

/*!
  Sets the total number of steps to \a totalSteps.
  \sa totalSteps()
*/

void QProgressBar::setTotalSteps( int totalSteps )
{
    total_steps = totalSteps;
    if ( isVisible() ) {
	if ( setIndicator(progress_str, progress_val, total_steps) )
	    repaint( FALSE );
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
  total number of steps.
  \sa progress(), totalSteps()
*/

void QProgressBar::setProgress( int progress )
{
    if ( progress <= progress_val )
	return;
    progress_val = progress;
    if ( isVisible() ) {
	if ( setIndicator(progress_str, progress_val, total_steps) )
	    repaint( FALSE );
    }
}


/*!
  Returns a size which fits the contents of the progress bar.
*/

QSize QProgressBar::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    return QSize(-1, fm.height()+8);
}


void QProgressBar::show()
{
    setIndicator( progress_str, progress_val, total_steps );
    QFrame::show();
}


/*! 
  This method is called to generate the text displayed in the center of
  the progress bar, by default it is the percentage of completion. 
  This method should return FALSE if the string is unchanged since the
  last call to the method, to allow efficient repainting of the
  progress bar.  
*/ 

bool QProgressBar::setIndicator( QString& indicator, int progress,
				 int totalSteps )
{
    if ( !totalSteps )
	return FALSE;
    int np = progress * 100 / totalSteps;
    if ( np != percentage ) {
	percentage = np;
	indicator.sprintf( "%d%%", np );
	return TRUE;
    } else {
	return FALSE;
    }
}


/*!
  Handles paint events for the progress bar.
*/

void QProgressBar::drawContents( QPainter *p )
{
    const int unit_width  = 9;	    // includes 2 bg pixels
    const int unit_height = 12;
    const QRect bar = contentsRect();

    if ( style() == WindowsStyle ) {
	// Draw nu units out of a possible u of unit_width width, each
	// a rectangle bordered by background color, all in a sunken panel
	// with a percentage text display at the end.

	QFontMetrics fm = p->fontMetrics();
	int textw = fm.width("100%");
	int u = (bar.width() - textw - 2/*panel*/) / unit_width;
	int ox = ( bar.width() - (u*unit_width+textw) ) / 2;

	if (total_steps) { // Sanity check
	    // ### This part doesn't change as often as percentage does.
	    int nu = ( u * progress_val + total_steps/2 ) / total_steps;
	    int x = bar.x() + ox;
	    int uh = QMIN(bar.height()-4, unit_height);
	    int vm = (bar.height()-4 - uh)/2 + 2;
	    p->setPen(NoPen);
	    for (int i=0; i<nu; i++) {
		p->fillRect( x+2, bar.y()+vm,
			     unit_width-2, bar.height()-vm-vm, 
			     QApplication::winStyleHighlightColor() );
		x += unit_width;
	    }
	}

	// ### This part doesn't actually change.
	const QRect r( ox + bar.x(), bar.y(), u*unit_width + 2, bar.height() );
	qDrawShadePanel( p, r, colorGroup(), TRUE, 1 );

	// ### This part changes every percentage change.
	p->fillRect( r.x()+r.width(), bar.y(), textw, bar.height(),
	    backgroundColor() );
	p->drawText( r.x()+r.width(), bar.y(), textw, bar.height(),
	    AlignRight | AlignVCenter, progress_str );
    } else {
	int pw = bar.width() * progress_val / total_steps;

	p->setPen( white );
	p->setClipRect( bar.x(), bar.y(), pw, bar.height() );
	p->fillRect( bar, QApplication::winStyleHighlightColor() );
	p->drawText( bar, AlignCenter, progress_str );

	p->setPen( QApplication::winStyleHighlightColor() );
	p->setClipRect( bar.x()+pw, bar.y(), bar.width()-pw, bar.height() );
	p->fillRect( bar, white );
	p->drawText( bar, AlignCenter, progress_str );
    }
}
