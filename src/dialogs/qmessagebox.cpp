/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#2 $
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

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#2 $";
#endif


/*!
\class QMessageBox qmsgbox.h
\brief The QMessageBox widget class provides a modal message box.

A message box is a modal view that displays a text and contains a push button.

The default push button text is "Ok". This can be changed with setButtonText().
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
	: QDialog( parent, name )
{
    label = new QLabel( this, "text" );
    CHECK_PTR( label );
    label->setAlignment( AlignCenter );
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

\sa setButtonText().
*/

const char *QMessageBox::buttonText() const
{
    return button->label();
}

/*!
Sets the push button text to be displayed.

The default push button text is "Ok".

\sa buttonText().
*/

void QMessageBox::setButtonText( const char *text )
{
    button->setLabel( text );
    resize( size() );
}


/*!
Internal geometry management.

\todo Fix boundingRect.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
    QRect br;
    QPainter p;
    p.begin( label );
    br = p.boundingRect( 0,0, 1000,1000, label->alignment(), label->label() );
    p.end();
    QFontMetrics fm( button->font() );
    QRect bbr = fm.boundingRect( button->label() );
    button->resize( bbr.width()+20, bbr.height()+10 );
    br.setSize( QSize(br.width()+10,br.height()+10) );
    int h = (height() - br.height() - button->height())/3;
    button->move( width()/2 - button->width()/2,
		  height() - h - button->height() );
    label->setGeometry( width()/2 - br.width()/2, h,
			br.width(), br.height() );
}
