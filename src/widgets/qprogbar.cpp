/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qprogbar.cpp#5 $
**
** Implementation of QProgressBar class
**
** Created : 970521
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include <qprogbar.h>
#include <qpainter.h>
#include <qdrawutl.h>
#include <qapp.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qprogbar.cpp#5 $");

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

  \arg \e total_steps is the total number of steps in the operation of which
    this progress bar shows the progress.  For example, if the operation
    is to examine 50 files, this value would be 50, then before examining
    the first file, call setProgress(0), and after examining the last file
    call setProgress(50).
  \arg \e parent, \e name, \e f, and \e allowLines are sent to the
    QFrame::QFrame() constructor.
*/
QProgressBar::QProgressBar( int total_steps,
	QWidget *parent, const char *name, WFlags f, bool allowLines ) :
    QFrame( parent, name, f, allowLines),
    totalsteps( total_steps ),
    progr( -1 ),
    percentage( -1 )
{
    if ( style() == MotifStyle ) {
	setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	setLineWidth( 2 );
    }
}

/*!
  Reset the progress bar, changing the total number of steps
  to a new value. The progress bar `rewinds'.
*/
void QProgressBar::reset( int total_steps )
{
    totalsteps = total_steps;
    reset();
}

/*!
  Reset the progress bar.
  The progress bar `rewinds'.
*/
void QProgressBar::reset()
{
    progr = -1;
    percentage = -1;
}

/*!
  Sets the current amount of progress made to \e prog units of the
  total number of steps.
*/
void QProgressBar::setProgress( int prog )
{
    if ( prog <= progr ) return;
    if ( prog==0 && progr > 0 ) return;

    progr = prog;

    if (isVisible()) {
	if (setIndicator( progress_str, progr, totalsteps ))
	    repaint( FALSE );
    }
}

void QProgressBar::show()
{
    setIndicator( progress_str, progr, totalsteps );
    QFrame::show();
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
    if (!totalsteps) return FALSE; // Sanity check

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
  Returns a size which fits the contents of the progress bar.
*/

QSize QProgressBar::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    return QSize(-1, fm.height()+8);
}

/*!
  Handles paint events for the progress bar.
*/
void QProgressBar::drawContents( QPainter *p )
{
    static const int unit_width = 9; // includes 2 bg pixels
    static const int unit_height = 12;
    const QRect bar = contentsRect();

    if ( style() == WindowsStyle ) {
	// Draw nu units out of a possible u of unit_width width, each
	// a rectangle bordered by background color, all in a sunken panel
	// with a percentage text display at the end.

	QFontMetrics fm = p->fontMetrics();
	int textw = fm.width("100%");
	int u = (bar.width() - textw - 2/*panel*/) / unit_width;
	int ox = ( bar.width() - (u*unit_width+textw) ) / 2;

	if (totalsteps) { // Sanity check
	    // ### This part doesn't change as often as percentage does.
	    int nu = ( u * progr + totalsteps/2 ) / totalsteps;
	    int x = bar.x() + ox;
	    int uh = QMIN(bar.height()-4, unit_height);
	    int vm = (bar.height()-4 - uh)/2 + 2;
	    p->setPen(NoPen);
	    for (int i=0; i<nu; i++) {
		p->fillRect( x+2, bar.y()+vm,
		    unit_width-2, bar.height()-vm-vm, darkBlue );
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
	int pw = bar.width() * progr / totalsteps;

	p->setPen( white );
	p->setClipRect( bar.x(), bar.y(), pw, bar.height() );
	p->fillRect( bar, darkBlue );
	p->drawText( bar, AlignCenter, progress_str );

	p->setPen( darkBlue );
	p->setClipRect( bar.x()+pw, bar.y(), bar.width()-pw, bar.height() );
	p->fillRect( bar, white );
	p->drawText( bar, AlignCenter, progress_str );
    }
}

/*!
  \fn int QProgressBar::totalSteps() const
  Returns the total number of steps, as set at construction or by reset(int).
*/
