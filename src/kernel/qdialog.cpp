/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.cpp#3 $
**
** Implementation of QDialog class
**
** Author  : Haavard Nord
** Created : 950502
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdialog.h"
#include "qapp.h"
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qdialog.cpp#3 $";
#endif


/*!
\class QDialog qdialog.h
\brief The QDialog class is the base class of modal dialog views.
*/


/*!
Constructs a QDialog, named \e name,  which will be a child view of \e parent.
The widget flags \e f should normally be set to zero unless you know what you
are doing.
*/

QDialog::QDialog( QWidget *parent, const char *name, WFlags f )
    : QView( parent, name, f | WType_Modal )
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
Closes the dialog and sets the result code to Accepted.
*/

void QDialog::accept()
{
    done( Accepted );
}

/*!
Closes the dialog and sets the result code to Rejected.
*/

void QDialog::reject()
{
    done( Rejected );
}


// --------------------------------------------------------------------------
// Event handlers
//

/*!
Calls accept() if enter is pressed, or reject() if escape is pressed.
*/

void QDialog::keyPressEvent( QKeyEvent *e )
{
    if ( e->state() == 0 ) {
	switch ( e->key() ) {
	    case Key_Enter:
		accept();
		e->accept();
		break;
	    case Key_Escape:
		reject();
		e->accept();
		break;
	}
    }
}


// --------------------------------------------------------------------------
// Geometry management.
//

void QDialog::show()
{
    if ( !did_resize )
	adjustSize();
    if ( !did_move ) {
	QWidget *w = parentWidget();
	if ( !w )
	    w = QApplication::desktop();
	move( w->width()/2  - width()/2,
	      w->height()/2 - height()/2 );
    }
    QWidget::show();
}


/*!
Virtual function that adjusts the size of the dialog to fit the contents.

This function will not be called if the dialog has been explicitly
resized before showing it.

The default implementation does nothing.
*/

void QDialog::adjustSize()
{
}


// --------------------------------------------------------------------------
// Detects any widget geometry changed done by the user.
//

void QDialog::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

void QDialog::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

void QDialog::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

void QDialog::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

void QDialog::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

void QDialog::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}
