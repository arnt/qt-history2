/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogdlg.cpp#7 $
**
** Implementation of QProgressDialog class
**
** Created : 970521
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprogdlg.h"
#include <qpainter.h>
#include <qdrawutl.h>
#include <qapp.h>

RCSTAG("$Id: //depot/qt/main/src/dialogs/qprogdlg.cpp#7 $");

// If the operation is expected to take this long (as predicted by
// progress time), show the progress dialog.
static const int showTime    = 4000;  // Arnt: as per Macintosh guidelines
// Wait at least this long before attempting to make a prediction.
static const int minWaitTime = 50;

// Various layout values
static const int margin_lr   = 10;
static const int margin_tb   = 10;
static const int spacing     = 4;

struct QProgressData
{
    QProgressData( QProgressDialog* that, QWidget* cr, const char* lbtext ) :
	creator(cr),
	cancel( "", that ),
	cancellation_flag( TRUE ),
	label_text( lbtext ),
	the_label( 0 )
    {
    }

    bool	shown_once;
    QWidget	*creator;
    QPushButton	cancel;
    bool	cancellation_flag;
    QString	label_text;
    QWidget	*the_label;
    QTime	starttime;
    QCursor	parentCursor;
};


/*!
  \class QProgressDialog qprogdlg.h
  \brief The QProgressDialog widget provides a progress display.
  \ingroup realwidgets

  A progress dialog is used to give the user an indication of how long an
  operation is going to take to perform, and to reassure them that the
  application has not crashed.
 
  A potential problem with progress dialogs is that it is difficult to know
  when to use them, as operations take different amounts of time on different
  computer hardware.  QProgressDialog offers a solution to this problem:
  it estimates the time the operation will take (based on time for
  steps), and only shows itself if that estimate is beyond 3 seconds.

  Example:
  \code
    QProgressDialog pb("Doing stuff...", steps, this);
    for (int i=0; i<steps; i++) {
	pb.setProgress(i);
	...
    }
    pb.setProgress(steps);
  \endcode

  A QProgressDialog may also have a `cancel' button to abort progress:

  \code
    QProgressDialog pb("Doing stuff...", steps, this);
    pb.setCancelButton("Abort");
    for (int i=0; i<steps; i++) {
	if (pb.setProgress(i)) break;
	...
    }
    pb.setProgress(steps);
  \endcode

  <img src=qprogdlg-m.gif> <img src=qprogdlg-w.gif>
*/


/*!
  Constructs a progress dialog.

  \arg \e label_text is text telling the user what is progressing.
  \arg \e total_steps is the total number of steps in the operation of which
    this progress dialog shows the progress.  For example, if the operation
    is to examine 50 files, this value would be 50, then before examining
    the first file, call setProgress(0), and after examining the last file
    call setProgress(50).
  \arg \e parent, \e name, \e modal, and \e f are sent to the
    QDialog::QDialog() constructor. Note that \e if modal is FALSE (the
    default), you will need to have an event loop proceeding for any
    redrawing of the dialog to occur.  If it is TRUE, the dialog ensures
    events are processed when needed.
*/
QProgressDialog::QProgressDialog( const char* label_text, int total_steps,
	QWidget *creator, const char *name, bool modal, WFlags f ) :
    QSemiModal( 0, name, modal, f),
    the_bar( 0 ),
    d(new QProgressData(this, creator, label_text)),
    totalsteps( total_steps )
{
    d->cancel.hide();
    connect( &d->cancel, SIGNAL(clicked()), this, SIGNAL(cancelled()) );
    connect( this, SIGNAL(cancelled()), this, SLOT(reset()) );
    d->shown_once = FALSE;
}

/*!
  Destroys the progress dialog, destroying any objects ever
  returned by progressBar() or labelWidget().
*/
QProgressDialog::~QProgressDialog()
{
    delete d;
}

/*!
  \fn void QProgressDialog::cancelled()

  This signal is emitted when the cancel button is clicked.
*/

/*!
  Sets the label for the cancellation button (eg. "Cancel" or "Abort").
  If this is 0, any previous cancellation button is removed.
  The progress dialog resizes to fit.

  \sa cancelled()
*/
void QProgressDialog::setCancelButton( const char* c )
{
    if ( c ) {
	d->cancel.setText( c );
	d->cancel.show();
    } else {
	d->cancel.hide();
    }
    if (isVisible()) resize(sizeHint());
}

/*!
  Reset the progress dialog, changing the total number of steps
  to a new value. The progress dialog becomes hidden.
*/
void QProgressDialog::reset( int total_steps )
{
    totalsteps = total_steps;
    reset();
}

/*!
  Reset the progress dialog.
  The progress dialog becomes hidden.
*/
void QProgressDialog::reset()
{
    int progress = bar().progress();

    if ( progress >= 0 ) {
	if ( d->creator ) d->creator->setCursor( d->parentCursor );
    }
    if (isVisible()) {
	hide();
    }

    bar().reset( totalsteps );

    d->cancellation_flag = TRUE;
    d->shown_once = FALSE;
}

/*!
  Sets the current amount of progress made to \e prog units of the
  total number of steps.  For the progress dialog to work correctly,
  you must at least call this with the parameter 0 initially, then
  later with QProgressDialog::totalSteps().

  \warning This method calls QApplication::processEvents(), although making
  the progress dialog modal (see QProgressDialog::QProgressDialog()), will block
  most events from receipt by your application.

  Returns TRUE if the user has clicked the cancellation button. 
*/
bool QProgressDialog::setProgress( int prog )
{
    int progress = bar().progress();

    d->cancellation_flag = FALSE;

    if ( prog <= progress ) return d->cancellation_flag;
    if ( prog==0 && progress > 0 ) return d->cancellation_flag;
    if ( prog!=0 && progress < 0 ) return d->cancellation_flag;

    bar().setProgress(prog);

    if ( isVisible() ) {
	if (testWFlags(WType_Modal)) qApp->processEvents();
    } else if ( !d->shown_once ) {
	if ( prog == 0 ) {
	    if ( d->creator ) {
		d->parentCursor = d->creator->cursor();
		d->creator->setCursor( waitCursor );
	    }
	    d->starttime.start();
	} else {
	    int elapsed = d->starttime.elapsed();
	    if ( elapsed > minWaitTime ) {
		int estimate = elapsed * ( totalsteps - prog ) / prog;
		if ( estimate > showTime ) {
		    resize(sizeHint());
		    center();
		    show();
		    d->shown_once = TRUE;
		    if (testWFlags(WType_Modal)) {
			qApp->processEvents();
		    }
		}
	    }
	}
    }

    if ( prog == totalsteps ) {
	reset();
    }

    return d->cancellation_flag;
}

void QProgressDialog::center()
{
    QPoint p(0,0);
    QWidget* w;
    if (d->creator) {
	p = d->creator->mapToGlobal( p );
	w = d->creator;
    } else {
	w = QApplication::desktop();
    }
    setGeometry( p.x() + w->width()/2  - width()/2,
	  p.y() + w->height()/2 - height()/2, width(), height() );
}

/*!
  Change the label on the progress dialog.
  The progress dialog resizes to fit.
*/
void QProgressDialog::setLabel( const char* txt )
{
    if (!txt || d->label_text != txt) {
	d->label_text = txt;
	delete d->the_label;
	d->the_label = 0; // will be created when next needed.
	if (isVisible()) resize(sizeHint());
    }
}

/*!
  Returns a size which fits the contents of the progress dialog.
  The progress dialog resizes itself as required, so this should not
  be needed in user code.
*/

QSize QProgressDialog::sizeHint() const
{
    QSize sh = label().sizeHint();
    QSize bh = bar().sizeHint();
    int h = margin_tb*2 + bh.height() + sh.height() + spacing;
    if ( d->cancel.isVisible() )
	h += d->cancel.sizeHint().height() + spacing;
    return QSize( QMAX(200, sh.width()), h );
}

/*!
  Handles resize events for the progress dialog, sizing the label,
  dialog, and cancellation button.
*/
void QProgressDialog::resizeEvent( QResizeEvent * )
{
    layout();
}

/*!
  Ensures layout conforms to style of GUI.
*/
void QProgressDialog::styleChange(GUIStyle s)
{
    layout();
    QSemiModal::styleChange(s);
}

void QProgressDialog::layout()
{
    int sp = spacing;
    int mtb = margin_tb;
    int mlr = QMIN(width()/10, margin_lr);
    const bool centered = (style() != WindowsStyle);

    QSize cs = d->cancel.sizeHint();
    QSize bh = bar().sizeHint();
    int cspc;
    int lh = 0;

    // Find spacing and sizes that fit.  It is important that a progress
    // dialog can be made very small if the user demands it so.
    for (int attempt=5; attempt--; ) {
	cspc = d->cancel.isVisible() ? cs.height() + sp : 0;
	lh = QMAX(0, height() - mtb - bh.height() - sp - cspc);

	if ( lh < height()/4 ) {
	    // Getting cramped
	    sp /= 2;
	    mtb /= 2;
	    if (d->cancel.isVisible()) {
		cs.setHeight(QMAX(4,cs.height()-sp-2));
	    }
	    bh.setHeight(QMAX(4,bh.height()-sp-1));
	} else {
	    break;
	}
    }

    if ( d->cancel.isVisible() ) {
	d->cancel.setGeometry(
	    centered ? width()/2 - cs.width()/2 : width() - mlr - cs.width(),
	    height() - mtb - cs.height() + sp,
	    cs.width(), cs.height() );
    }

    label().setGeometry( mlr, 0, width()-mlr*2, lh );
    bar().setGeometry( mlr, lh+sp,
	width()-mlr*2, bh.height() );
}

/*!
  \fn int QProgressDialog::totalSteps() const
  Returns the total number of steps, as set at construction or by reset(int).
*/

/*!
  This method is called once when a QProgressBar is first required.
  Override this if you want to use your own subclass of QProgressBar.
  Note that ownership of the progress bar transfers to the QProgressDialog,
  so you should pass a newly allocated QProgressBar with the given
  \e total_steps and \e this as the parent.
*/
QProgressBar* QProgressDialog::progressBar(int total_steps)
{
    return new QProgressBar(total_steps, this, "bar");
}

/*!
  This method is called once whenever a QWidget is required for the
  area above the progress bar.  The widget is recreated whenever the
  label text is changed or set to 0.
  Override this if you want to use your own subclass of QWidget.
  Note that ownership of the widget transfers to the QProgressDialog,
  so you should pass a newly allocated widget using the given
  \e text and \e this as the parent.  The default implementation
  uses a QLabel, aligned according to the style() of the dialog.
*/
QWidget* QProgressDialog::labelWidget(const QString& str)
{
    QLabel* lbl = new QLabel(str, this, "label");
    lbl->setAlignment( style() != WindowsStyle
		? AlignCenter : AlignLeft|AlignVCenter );
    return lbl;
}

/*!
 Ensures bar exists.
*/
QProgressBar& QProgressDialog::bar()
{
    if (!the_bar) the_bar = progressBar(totalsteps);
    return *the_bar;
}

/*!
 Ensures bar exists.
*/
const QProgressBar& QProgressDialog::bar() const
{
    QProgressDialog* non_const_this = (QProgressDialog*)this;
    return non_const_this->bar();
}

/*!
 Ensures label exists.
*/
QWidget& QProgressDialog::label()
{
    if (!d->the_label) d->the_label = labelWidget(d->label_text);
    return *d->the_label;
}

/*!
 Ensures label exists.
*/
const QWidget& QProgressDialog::label() const
{
    QProgressDialog* non_const_this = (QProgressDialog*)this;
    return non_const_this->label();
}
