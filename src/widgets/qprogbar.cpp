/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogbar.cpp#1 $
**
** Implementation of QProgressBar class
**
** Created : 970521
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprogbar.h"
#include <qpainter.h>
#include <qdrawutl.h>
#include <qapp.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qprogbar.cpp#1 $");

// If the operation is expected to take this long (as predicted by
// progress time), show the progress bar.
static const int showTime    = 4000;  // Arnt: as per Macintosh guidelines
// Wait at least this long before attempting to make a prediction.
static const int minWaitTime = 50;

// Various layout values
static const int margin_lr   = 10;
static const int margin_tb   = 10;
static const int disp_height = 20;
static const int spacing     = 4;


/*!
  \class QProgressBar qprogbar.h
  \brief The QProgressBar widget provides a horizontal progress bar.
  \ingroup realwidgets

  A progress bar is used to give the user an indication of how long an
  operation is going to take to perform, and to reassure them that the
  application has not crashed.
 
  A potential problem with progress bars is that it is difficult to know
  when to use them, as operations take different amounts of time on different
  computer hardware.  QProgressBar offers a solution to this problem:
  it estimates the time the operation will take (based on time for
  steps), and only shows itself if that estimate is beyond 3 seconds.

  Example:
  \code
    QProgressBar pb("Doing stuff...", steps, this);
    for (int i=0; i<steps; i++) {
	pb.setProgress(i);
	...
    }
    pb.setProgress(steps);
  \endcode

  A QProgressBar may also have a `cancel' button to abort progress:

  \code
    QProgressBar pb("Doing stuff...", steps, this);
    pb.setCancelButton("Abort");
    for (int i=0; i<steps; i++) {
	if (pb.setProgress(i)) break;
	...
    }
    pb.setProgress(steps);
  \endcode
*/


/*!
  Constructs a progress bar.

  \arg \e label_text is text telling the user what is progressing.
  \arg \e total_steps is the total number of steps in the operation of which
    this progress bar shows the progress.  For example, if the operation
    is to examine 50 files, this value would be 50, then before examining
    the first file, call setProgress(0), and after examining the last file
    call setProgress(50).
  \arg \e parent, \e name, \e modal, and \e f are sent to the
    QDialog::QDialog() constructor. Note that \e modal defaults to
    TRUE, unlike in QDialog::QDialog().
*/
QProgressBar::QProgressBar( const char* label_text, int total_steps,
	QWidget *parent, const char *name, bool modal, WFlags f ) :
    QDialog( parent, name, modal, f),
    totalsteps( total_steps ),
    progress( -1 ),
    percentage( -1 ),
    cancel( "", this ),
    cancellation_flag( TRUE ),
    label( "", this )
{
    cancel.hide();
    connect( &cancel, SIGNAL(clicked()), this, SIGNAL(cancelled()) );
    connect( this, SIGNAL(cancelled()), this, SLOT(reset()) );
    label.setAlignment( AlignCenter );
    setLabel( label_text );
}

/*!
  \fn void QProgressBar::cancelled()

  This signal is emitted when the cancel button is clicked.
*/

/*!
  Sets the label for the cancellation button (eg. "Cancel" or "Abort").
  If this is 0, any previous cancellation button is removed.
  The progress bar resizes to fit.

  \sa cancelled()
*/
void QProgressBar::setCancelButton( const char* c )
{
    if ( c ) {
	cancel.setText( c );
	cancel.show();
    } else {
	cancel.hide();
    }
    resize(sizeHint());
}

/*!
  Reset the progress bar, changing the total number of steps
  to a new value. The progress bar becomes hidden.
*/
void QProgressBar::reset( int total_steps )
{
    totalsteps = total_steps;
    reset();
}

/*!
  Reset the progress bar.
  The progress bar becomes hidden.
*/
void QProgressBar::reset()
{
    if ( progress >= 0 ) {
	QWidget* p = parentWidget();
	if ( p ) p->setCursor( parentCursor );
    }
    hideNoLoop();
    progress = -1;
    percentage = -1;
    cancellation_flag = TRUE;
}

/*!
  Sets the current amount of progress made to \e prog units of the
  total number of steps.  For the progress bar to work correctly,
  you must at least call this with the parameter 0 initially, then
  later with QProgressBar::totalSteps().

  \warning This method calls QApplication::processEvents(), although making
  the progress bar modal (see QProgressBar::QProgressBar()), will block
  most events from receipt by your application.

  Returns TRUE if the user has clicked the cancellation button. 
*/
bool QProgressBar::setProgress( int prog )
{
    cancellation_flag = FALSE;

    if ( prog <= progress ) return cancellation_flag;
    if ( prog==0 && progress > 0 ) return cancellation_flag;
    if ( prog!=0 && progress < 0 ) return cancellation_flag;

    progress = prog;

    if ( isVisible() ) {
	if (setIndicator( progress_str, progress, totalsteps )) {
	    repaint( barArea(), FALSE );
	    qApp->processEvents();
	}
    } else {
	if ( progress == 0 ) {
	    QWidget* p = parentWidget();
	    if ( p ) {
		parentCursor = p->cursor();
		p->setCursor( waitCursor );
	    }
	    starttime.start();
	} else {
	    int elapsed = starttime.elapsed();
	    if ( elapsed > minWaitTime ) {
		int estimate = elapsed * ( totalsteps - progress ) / progress;
		if ( estimate > showTime ) {
		    setIndicator( progress_str, progress, totalsteps );
		    showNoLoop();
		    qApp->processEvents();
		}
	    }
	}
    }

    if ( progress == totalsteps ) {
	reset();
	qApp->processEvents();
    }

    return cancellation_flag;
}

/*! 
  This method is called to generate the text displayed in the center of
  the progress bar, by default it is the percentage of completion. 
  This method should return FALSE if the string is unchanged since the
  last call to the method, to allow efficient repainting of the
  progress bar.  
*/ 

bool QProgressBar::setIndicator( QString& indicator, int progress, int totalsteps )
{
    int np = progress * 100 / totalsteps;
    if ( np != percentage ) {
	percentage = np;
	indicator.sprintf( "%d%%", np );
	return TRUE;
    } else {
	return FALSE;
    }
}

/*!
  Change the label on the progress bar.
  The progress bar resizes to fit.
*/
void QProgressBar::setLabel( const char* txt )
{
    label.setText( txt );
    resize(sizeHint());
}

/*!
  Returns a size which fits the contents of the progress bar.
  The progress bar resizes itself as required, so this should not
  be needed in user code.
*/

QSize QProgressBar::sizeHint() const
{
    QSize sh = label.sizeHint();
    int h = margin_tb*2 + disp_height + sh.height() + spacing;
    if ( cancel.isVisible() )
	h += cancel.sizeHint().height() + spacing;
    return QSize( QMAX(200, sh.width()), h );
}

/*!
  Handles resize events for the progress bar, sizing the label,
  bar, and cancellation button.
*/
void QProgressBar::resizeEvent( QResizeEvent * )
{
    int lh = height() - margin_tb - disp_height - spacing;
    if ( cancel.isVisible() ) {
	QSize cs = cancel.sizeHint();
	cancel.setGeometry( width()/2 - cs.width()/2,
	    height() - margin_tb - cs.height() + spacing,
	    cs.width(), cs.height() );
	lh -= cs.height() + spacing;
    }
    label.setGeometry( 0, 0, width(), lh );
}

QRect QProgressBar::barArea() const
{
    return QRect(margin_lr, label.height() + spacing,
	width() - 2*margin_lr, disp_height);
}

/*!
  Handles paint events for the progress bar.
*/
void QProgressBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    QRect bar = barArea();
    int pw = bar.width() * progress / totalsteps;

    qDrawShadePanel( &p, bar.x()-2, bar.y()-2, bar.width()+4, bar.height()+4,
	colorGroup(), TRUE, 2, 0 );

    p.setPen( white );
    p.setClipRect( bar.x(), bar.y(), pw, bar.height() );
    p.fillRect( bar, blue );
    p.drawText( bar, AlignCenter, progress_str );

    p.setPen( blue );
    p.setClipRect( bar.x()+pw+1, bar.y(), bar.width()-pw, bar.height() );
    p.fillRect( bar, white );
    p.drawText( bar, AlignCenter, progress_str );
}

/*!
  \fn int QProgressBar::totalSteps() const
  Returns the total number of steps, as set at construction or by reset(int).
*/
