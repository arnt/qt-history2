/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.cpp#1 $
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
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qdialog.cpp#1 $";
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
}

/*!
Destroys the QDialog and all its children.
*/

QDialog::~QDialog()
{
}


/*!
Starts the dialog and returns the result code. Same as calling show(), then
result().
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
