/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#4 $
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
#include "qpainter.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#4 $";
#endif


/*!
\class QMessageBox qmsgbox.h
\brief The QMessageBox widget provides a modal message box.

A message box is a modal view that displays a text and contains a push button.

The default push button text is "Ok". This can be changed with setButtonText().

Enabling auto-resizing will make a message box resize itself whenever
the contents change.

Example of use:
\code
  QMessageBox mb;
  mb.setText( "This program may crash you hardware!!!\nLet's start..." );
  mb.setButtonText( "Yes" );
  mb.exec();
\endcode
*/


/*!
Constructs a message box with no text and a button with the text "Ok".

If \e parent is 0, then the message box becomes an application-global
modal dialog box.  If \e parent is a real widget, the message
box becomes modal relative to \e parent.

The \e parent and \e name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
	: QDialog( parent, name )
{
    label = new QLabel( this, "text" );
    CHECK_PTR( label );
    label->setAlignment( AlignCenter );
    button = new QPushButton( "Ok", this, "button" );
    CHECK_PTR( button );
    connect( button, SIGNAL(clicked()), SLOT(accept()) );
}


/*!
Returns the message box text currently set, or null if no text has been set.
\sa setText().
*/

const char *QMessageBox::text() const
{
    return label->text();
}

/*!
Sets the message box text to be displayed.
\sa text().
*/

void QMessageBox::setText( const char *text )
{
    label->setText( text );
}

/*!
Returns the push button text currently set, or null if no text has been set.
\sa setButtonText().
*/

const char *QMessageBox::buttonText() const
{
    return button->text();
}

/*!
Sets the push button text to be displayed.

The default push button text is "Ok".

\sa buttonText().
*/

void QMessageBox::setButtonText( const char *text )
{
    button->setText( text ? text : "Ok" );
}


/*!
Adjusts the size of the message box to fit the contents just before
QDialog::exec() or QDialog::show() is called.

This function will not be called if the message box has been explicitly
resized before showing it.
*/

void QMessageBox::adjustSize()
{
    button->adjustSize();
    label->adjustSize();
    int w = QMAX(button->width(),label->width());
    int h = button->height() + label->height();
    resize( w + w/4, h + h/4 );
}


/*!
Internal geometry management.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
    button->adjustSize();
    label->adjustSize();
    int h = (height() - button->height() - label->height())/3;
    button->move( width()/2 - button->width()/2,
		  height() - h - button->height() );
    label->move( width()/2 - label->width()/2, h );
}


// --------------------------------------------------------------------------
// Static QMessageBox functions
//

/*!
Opens a message box directly using the specified parameters.

Example of use:
\code
  QMessageBox::message( "Warning", "Did you feed the giraffe", "Sorry" );
\endcode
*/

int QMessageBox::message( const char *caption,
			  const char *text,
			  const char *buttonText,
			  QWidget    *parent,
			  const char *name )
{
    QMessageBox *mb = new QMessageBox( parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    if ( buttonText )
	mb->setButtonText( buttonText );
    int retcode = mb->exec();
    delete mb;
    return retcode;
}
