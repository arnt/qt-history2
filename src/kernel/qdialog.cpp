/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.cpp#20 $
**
** Implementation of QDialog class
**
** Author  : Haavard Nord
** Created : 950502
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdialog.h"
#include "qpushbt.h"
#include "qapp.h"
#include "qkeycode.h"
#include "qobjcoll.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qdialog.cpp#20 $")


/*!
  \class QDialog qdialog.h
  \brief The QDialog class is the base class of dialog windows.

  A dialog window is a window used to communicate with the user.  It
  offers mechanisms such as default buttons.

  The dialog window can either be modeless or modal.  A modeless
  dialog is a normal window, while a modal window must be finished
  before the user can continue with other parts of the program. */


/*!
  Constructs a dialog named \e name, which will be a child widget of
  \e parent.

  The dialog will by default be modeless, unless you set \e modal to
  TRUE, which constructs a modal dialog.
*/

QDialog::QDialog( QWidget *parent, const char *name,
		  bool modal, WFlags f )
    : QWindow( parent, name, modal ? (f | WType_Modal) : f )
{
    rescode = 0;
    did_move = did_resize = FALSE;
}

/*!
  Destroys the QDialog and all its children.
*/

QDialog::~QDialog()
{
}


/*!
  \internal
  Called from the push button \e pushButton when this push button becomes the
  default button.
*/

void QDialog::setDefault( QPushButton *pushButton )
{
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
	if ( pb != pushButton )
	    pb->setDefault( FALSE );
	++it;
    }
    delete list;
}


/*!
  Starts the dialog and returns the result code. Same as calling
  show(), then result().
*/

int QDialog::exec()
{
    setResult( 0 );
    show();
    return result();
}


/*!
  Closes the dialog and sets the result code to \e r.
*/

void QDialog::done( int r )
{
    hide();
    setResult( r );
}

/*!
  Closes the dialog and sets the result code to \c Accepted.
*/

void QDialog::accept()
{
    done( Accepted );
}

/*!
  Closes the dialog and sets the result code to \c Rejected.
*/

void QDialog::reject()
{
    done( Rejected );
}


// --------------------------------------------------------------------------
// Event handlers
//

/*!
  Calls accept() if Return/Enter is pressed, or reject() if Escape is pressed.
*/

void QDialog::keyPressEvent( QKeyEvent *e )
{
    if ( e->state() == 0 ) {
	switch ( e->key() ) {
	    case Key_Enter:
	    case Key_Return: {
		if ( inherits("QMessageBox") ) {
		    accept();			// ugle hack!!!
		}
		QObjectList *list = queryList( "QPushButton" );
		QObjectListIt it( *list );
		QPushButton *pb;
		while ( (pb = (QPushButton*)it.current()) ) {
		    if ( pb->isDefault() ) {
			emit pb->clicked();
			break;
		    }
		    ++it;
		}
		delete list;
		}
		break;
	    case Key_Escape:
		reject();
		break;
	    default:
		e->ignore();
		return;
	}
    }
    else
	e->ignore();
}

/*!
  Calls reject() if it is a modal dialog, or accepts the close event
  if it is a modeless dialog.
*/

void QDialog::closeEvent( QCloseEvent *e )
{
    if ( testWFlags(WType_Modal) ) {
	e->ignore();				// do not delete the dialog
	reject();
    }
}


// --------------------------------------------------------------------------
// Geometry management.
//

/*!  Shows the dialog box on the screen, as QWidget::show().  This
  implementation also does auto-resize and auto-positioning, to find a
  size that fits the contents and position near the middle of the
  screen.

  \sa exec()
*/

void QDialog::show()
{
    if ( !did_resize )
	adjustSize();
    if ( !did_move ) {
	QWidget *w = parentWidget();
	QPoint p( 0, 0 );
	if ( w )
	    p = w->mapToGlobal( p );
	else
	    w = QApplication::desktop();
	move( p.x() + w->width()/2  - width()/2,
	      p.y() + w->height()/2 - height()/2 );
    }
    QWidget::show();
    did_resize = did_move = FALSE;
}


// --------------------------------------------------------------------------
// Detects any widget geometry changes done by the user.
//

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QDialog::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QDialog::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QDialog::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QDialog::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QDialog::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QDialog::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}
