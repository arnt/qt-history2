/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprogressdialog.cpp#15 $
**
** Implementation of QProgressDialog class
**
** Created : 970521
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprogdlg.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qdatetm.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qprogressdialog.cpp#15 $");


// If the operation is expected to take this long (as predicted by
// progress time), show the progress dialog.
static const int showTime    = 4000;
// Wait at least this long before attempting to make a prediction.
static const int minWaitTime = 50;

// Various layout values
static const int margin_lr   = 10;
static const int margin_tb   = 10;
static const int spacing     = 4;


struct QProgressData
{
    QProgressData( QProgressDialog* that, QWidget* parent,
		   const char* labelText, const char *buttonText,
		   int totalSteps ) :
	creator( parent ),
	label( new QLabel(labelText,that,"label") ),
	cancel( new QPushButton(buttonText,that,"cancel") ),
	bar( new QProgressBar(totalSteps,that,"bar") ),
	shown_once( FALSE ),
	cancellation_flag( TRUE )
    {
	label->setAlignment( that->style() != WindowsStyle ?
			     AlignCenter : AlignLeft|AlignVCenter );
    }

    QWidget	 *creator;
    QLabel	 *label;
    QPushButton	 *cancel;
    QProgressBar *bar;
    bool	  shown_once;
    bool	  cancellation_flag;
    QTime	  starttime;
    QCursor	  parentCursor;
};


/*!
  \class QProgressDialog qprogdlg.h
  \brief The QProgressDialog widget provides a progress display.
  \ingroup realwidgets

  A progress dialog is used to give the user an indication of how long an
  operation is going to take to perform, and to reassure them that the
  application has not frozen.
 
  A potential problem with progress dialogs is that it is difficult to know
  when to use them, as operations take different amounts of time on different
  computer hardware.  QProgressDialog offers a solution to this problem:
  it estimates the time the operation will take (based on time for
  steps), and only shows itself if that estimate is beyond 3 seconds.

  Example:
  \code
    QProgressDialog progress( "Copying files...", "Abort Copy", numFiles, this );
    for (int i=0; i<numFiles; i++) {
	progress.setProgress( i );
	if ( progress.wasCancelled() )
	    break;
	... // copy one file
    }
    progress.setProgress( numFiles );
  \endcode

  <img src=qprogdlg-m.gif> <img src=qprogdlg-w.gif>
*/


QLabel *QProgressDialog::label() const
{
    QProgressDialog *that = (QProgressDialog *)this;
    return that->d->label;
}

QPushButton *QProgressDialog::cancel() const
{
    QProgressDialog *that = (QProgressDialog *)this;
    return that->d->cancel;
}

QProgressBar *QProgressDialog::bar() const
{
    QProgressDialog *that = (QProgressDialog *)this;
    return that->d->bar;
}


/*!
  Constructs a progress dialog.

  Default settings:
  <ul>
    <li>The label text is empty.
    <li>The cancel button text is "Cancel".
    <li>The total number of steps is 100.
  </ul>

  \arg \e parent, \e name, \e modal, and \e f are sent to the
    QSemiModal::QSemiModal() constructor. Note that if \e modal is FALSE (the
    default), you will need to have an event loop proceeding for any
    redrawing of the dialog to occur.  If it is TRUE, the dialog ensures
    events are processed when needed.

  \sa setLabelText(), setLabel(), setCancelButtonText(), setCancelButton(),
  setTotalSteps()
*/

QProgressDialog::QProgressDialog( QWidget *creator, const char *name,
				  bool modal, WFlags f )
    : QSemiModal( 0, name, modal, f)
{
    d = new QProgressData(this, creator, "", "Cancel", 100);
    connect( cancel(), SIGNAL(clicked()), this, SIGNAL(cancelled()) );
    connect( this, SIGNAL(cancelled()), this, SLOT(reset()) );
    QAccel *accel = new QAccel( this );
    accel->connectItem( accel->insertItem(Key_Escape),
			cancel(), SIGNAL(clicked()) );
    layout();
}


/*!
  Constructs a progress dialog.

  \arg \e labelText is text telling the user what is progressing.
  \arg \e cancelButtonText is the text on the cancel button.
  \arg \e totalSteps is the total number of steps in the operation of which
    this progress dialog shows the progress.  For example, if the operation
    is to examine 50 files, this value would be 50, then before examining
    the first file, call setProgress(0), and after examining the last file
    call setProgress(50).
  \arg \e name, \e modal, and \e f are sent to the
    QSemiModal::QSemiModal() constructor. Note that if \e modal is FALSE (the
    default), you will need to have an event loop proceeding for any
    redrawing of the dialog to occur.  If it is TRUE, the dialog ensures
    events are processed when needed.

  \sa setLabelText(), setLabel(), setCancelButtonText(), setCancelButton(),
  setTotalSteps()
*/

QProgressDialog::QProgressDialog( const char *labelText,
				  const char *cancelButtonText,
				  int totalSteps,
				  QWidget *creator, const char *name,
				  bool modal, WFlags f )
    : QSemiModal( 0, name, modal, f)
{
    d = new QProgressData(this, creator, labelText, cancelButtonText,
			  totalSteps);
    if ( strlen(cancelButtonText) == 0 )
	cancel()->hide();
    connect( cancel(), SIGNAL(clicked()), this, SIGNAL(cancelled()) );
    connect( this, SIGNAL(cancelled()), this, SLOT(reset()) );
    QAccel *accel = new QAccel( this );
    accel->connectItem( accel->insertItem(Key_Escape),
			cancel(), SIGNAL(clicked()) );
    layout();
}


/*!
  Destroys the progress dialog.
*/

QProgressDialog::~QProgressDialog()
{
    delete d;
}


/*!
  \fn void QProgressDialog::cancelled()

  This signal is emitted when the cancel button is clicked.

  \sa wasCancelled()
*/


/*!
  Sets the label. The progress dialog resizes to fit.
  \sa setLabelText()
*/

void QProgressDialog::setLabel( QLabel *label )
{
    delete d->label;
    d->label = label;
    resize(sizeHint());
}


/*!
  Sets the label text. The progress dialog resizes to fit.
  \sa setLabel()
*/

void QProgressDialog::setLabelText( const char *text )
{
    if ( label() ) {
	label()->setText( text );
	resize(sizeHint());
    }
}


/*!
  Sets the cancellation button.
  \sa setCancelButtonText()
*/

void QProgressDialog::setCancelButton( QPushButton *cancelButton )
{
    delete d->cancel;
    d->cancel = cancelButton;
    connect( d->cancel, SIGNAL(clicked()), this, SIGNAL(cancelled()) );
    resize(sizeHint());
}


/*!
  Sets the cancellation button text.
  \sa setCancelButton()
*/

void QProgressDialog::setCancelButtonText( const char *cancelButtonText )
{
    if ( cancel() ) {
	cancel()->setText(cancelButtonText);
	if ( strlen(cancelButtonText) == 0 )
	    cancel()->hide();
	else
	    cancel()->show();
	resize(sizeHint());
    }
}


/*!
  Returns the TRUE if the dialog was cancelled, otherwise FALSE.
  \sa setProgress(), cancelled()
*/

bool QProgressDialog::wasCancelled() const
{
    return d->cancellation_flag;
}


/*!
  Returns the total number of steps.
  \sa setTotalSteps(), QProgressBar::totalSteps()
*/

int QProgressDialog::totalSteps() const
{
    return bar()->totalSteps();
}


/*!
  Sets the total number of steps.
  \sa totalSteps(), QProgressBar::setTotalSteps()
*/

void QProgressDialog::setTotalSteps( int totalSteps )
{
    bar()->setTotalSteps( totalSteps );
}


/*!
  Reset the progress dialog.
  The progress dialog becomes hidden.
*/

void QProgressDialog::reset()
{
    if ( progress() >= 0 ) {
	if ( d->creator )
	    d->creator->setCursor( d->parentCursor );
    }
    if ( isVisible() )
	hide();
    bar()->reset();
    d->cancellation_flag = TRUE;
    d->shown_once = FALSE;
}


int QProgressDialog::progress() const
{
    return bar()->progress();
}


/*!
  Sets the current amount of progress made to \e prog units of the
  total number of steps.  For the progress dialog to work correctly,
  you must at least call this with the parameter 0 initially, then
  later with QProgressDialog::totalSteps().

  \warning If the progress dialog is modal
    (see QProgressDialog::QProgressDialog()),
    this function calls QApplication::processEvents(), so take care that
    this does not cause undesirable re-entrancy to your code. For example,
    don't use a QProgressDialog inside a paintEvent()!

  Returns TRUE if the user has clicked the cancellation button. 
*/

void QProgressDialog::setProgress( int progress )
{
    int old_progress = bar()->progress();
    d->cancellation_flag = FALSE;

    if ( progress <= old_progress ||
	 progress == 0 && old_progress > 0 ||
	 progress != 0 && old_progress < 0 )
	 return;

    bar()->setProgress(progress);

    if ( isVisible() ) {
	if (testWFlags(WType_Modal))
	    qApp->processEvents();
    } else if ( !d->shown_once ) {
	if ( progress == 0 ) {
	    if ( d->creator ) {
		d->parentCursor = d->creator->cursor();
		d->creator->setCursor( waitCursor );
	    }
	    d->starttime.start();
	} else {
	    int elapsed = d->starttime.elapsed();
	    if ( elapsed > minWaitTime ) {
		int estimate = elapsed * (totalSteps() - progress) / progress;
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

    if ( progress == totalSteps() )
	reset();

    return;
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
  Returns a size which fits the contents of the progress dialog.
  The progress dialog resizes itself as required, so this should not
  be needed in user code.
*/

QSize QProgressDialog::sizeHint() const
{
    QSize sh = label()->sizeHint();
    QSize bh = bar()->sizeHint();
    int h = margin_tb*2 + bh.height() + sh.height() + spacing;
    if ( strlen(cancel()->text()) != 0 )
	h += cancel()->sizeHint().height() + spacing;
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
    QSemiModal::styleChange(s);
    layout();
}

void QProgressDialog::layout()
{
    int sp = spacing;
    int mtb = margin_tb;
    int mlr = QMIN(width()/10, margin_lr);
    const bool centered = (style() != WindowsStyle);

    bool has_cancel = strlen(cancel()->text()) != 0;
    QSize cs = cancel()->sizeHint();
    QSize bh = bar()->sizeHint();
    int cspc;
    int lh = 0;

    // Find spacing and sizes that fit.  It is important that a progress
    // dialog can be made very small if the user demands it so.
    for (int attempt=5; attempt--; ) {
	cspc = has_cancel ? cs.height() + sp : 0;
	lh = QMAX(0, height() - mtb - bh.height() - sp - cspc);

	if ( lh < height()/4 ) {
	    // Getting cramped
	    sp /= 2;
	    mtb /= 2;
	    if ( has_cancel ) {
		cs.setHeight(QMAX(4,cs.height()-sp-2));
	    }
	    bh.setHeight(QMAX(4,bh.height()-sp-1));
	} else {
	    break;
	}
    }

    if ( has_cancel ) {
	cancel()->setGeometry(
	    centered ? width()/2 - cs.width()/2 : width() - mlr - cs.width(),
	    height() - mtb - cs.height() + sp,
	    cs.width(), cs.height() );
    }

    label()->setGeometry( mlr, 0, width()-mlr*2, lh );
    bar()->setGeometry( mlr, lh+sp, width()-mlr*2, bh.height() );
}
