/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#1 $
**
** Implementation of QMessageBox class
**
** Author  : Haavard Nord
** Created : 950503
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmsgbox.h"
#include "qlabel.h"
#include "qpushbt.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#1 $";
#endif


/*!
\class QMessageBox qmsgbox.h
\brief The QMessageBox is a modal view that displays a text and contains a
push button.
The default push button text is "Ok". This can be changed with setButtonText().
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
	: QDialog( parent, name )
{
    label = new QLabel( this, "text" );
    CHECK_PTR( label );
    button = new QPushButton( this, "button" );
    CHECK_PTR( button );
    button->setLabel( "Ok" );
    connect( button, SIGNAL(clicked()), SLOT(accept()) );
}


/*!
Returns the message box text currently set, or null if no text has been set.
\sa setText().
*/

const char *QMessageBox::text() const
{
    return label->label();
}

/*!
Sets the message box text to be displayed.
\sa text().
*/

void QMessageBox::setText( const char *text )
{
    label->setLabel( text );
    resize( size() );
}

/*!
Returns the push button text currently set, or null if no text has been set.
Initially, the push button text is "Ok".
\sa setButtonText().
*/

const char *QMessageBox::buttonText() const
{
    return button->label();
}

/*!
Sets the push button text to be displayed.
\sa buttonText().
*/

void QMessageBox::setButtonText( const char *text )
{
    label->setLabel( text );
    resize( size() );
}


/*!
Does simple geometry management.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
}
