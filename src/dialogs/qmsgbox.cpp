/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#23 $
**
** Implementation of QMessageBox class
**
** Created : 950503
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmsgbox.h"
#include "qlabel.h"
#include "qpushbt.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#23 $");


/*!
  \class QMessageBox qmsgbox.h

  \brief The QMessageBox widget provides a message box.

  \ingroup dialogs

  A message box is a dialog that displays a text and a push button.

  The default push button text is "Ok". This can be changed with
  setButtonText().

  Enabling auto-resizing will make a message box resize itself whenever
  the contents change.

  Example of use:
  \code
    QMessageBox mb;
    mb.setText( "This program may crash your hardware!!!\nLet's start..." );
    mb.setButtonText( "Yes!" );
    mb.show();
  \endcode
*/


/*!
  Constructs a message box with no text and a button with the text "Ok".

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message
  box becomes modal relative to \e parent.

  The \e parent and \e name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    initMetaObject();
    label = new QLabel( this, "text" );
    CHECK_PTR( label );
    label->setAlignment( AlignCenter );
    button = new QPushButton( "Ok", this, "button" );
    CHECK_PTR( button );
    connect( button, SIGNAL(clicked()), SLOT(accept()) );
    QFont font( "Helvetica", 12 );
    label->setFont( font );
    font.setWeight( QFont::Bold );
    button->setFont( font );
    reserved1 = 0;
}


/*!
  Returns the message box text currently set, or null if no text has been set.
  \sa setText()
*/

const char *QMessageBox::text() const
{
    return label->text();
}

/*!
  Sets the message box text to be displayed.
  \sa text()
*/

void QMessageBox::setText( const char *text )
{
    label->setText( text );
}

/*!
  Returns the push button text currently set, or null if no text has been set.
  \sa setButtonText()
*/

const char *QMessageBox::buttonText() const
{
    return button->text();
}

/*!
  Sets the push button text to be displayed.

  The default push button text is "Ok".

  \sa buttonText()
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
    QSize bSize = button->sizeHint();
    if ( button2() ) {
	QSize s2 = button2()->sizeHint();
	bSize = QSize( QMAX( s2.width(), bSize.width() ), 
		       QMAX( s2.height(), bSize.height() ));
	button2()->resize( bSize );
    }
    button->resize( bSize );
    label->adjustSize();
    QString labelStr = label->text();
    int nlines = labelStr.contains( '\n' );
    QFontMetrics fm = label->fontMetrics();
    nlines += 2;
    int bw = button->width();
    if ( button2() )
	bw += button2()->width() + 5;
    int w = QMAX( bw, label->width() );
    int h = button->height() + fm.lineSpacing()*nlines;
    resize( w + w/3, h + h/3 );
}


/*!
  Internal geometry management.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
    QSize bSize = button->sizeHint();
    if ( button2() ) {
	QSize s2 = button2()->sizeHint();
	bSize = QSize( QMAX( s2.width(), bSize.width() ), 
		       QMAX( s2.height(), bSize.height() ));
	button2()->resize( bSize );
    }
    button->resize( bSize );
    label->adjustSize();
    int h = (height() - button->height() - label->height())/3;
    label->move( width()/2 - label->width()/2, h );
    if ( button2() ) {
	int bw = button->width() + button2()->width();
	int w = width() - bw;
	button->move( w/3, height() - h - button->height() );
	button2()->move( width() - w/3 - button2()->width() ,
			 height() - h - button->height() );
    } else
	button->move( width()/2 - button->width()/2,
		      height() - h - button->height() );
}


/*****************************************************************************
  Static QMessageBox functions
 *****************************************************************************/

/*!
  Opens a modal message box directly using the specified parameters.

  Example:
  \code
    QMessageBox::message( "Warning", "Did you feed the giraffe?", "Sorry!" );
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


/*!
  Queries the user using a modal message box with two buttons.
  Note that \a caption is not always shown, it depends on the window manager.

  \a text is the question the user is to answer. \a yesButtonText defaults to
  "Yes" and \a noButtonText defaults to "No".

  Example:
  \code
    bool ok = QMessageBox::query( "Consider this carefully", 
                             "Should I delete all your files?", 
                             "Go ahead!", "Wait a minute" );
    if ( ok )
        deleteAllFiles();
  \endcode

*/

bool QMessageBox::query( const char *caption,
			       const char *text,
			       const char *yesButtonText,
			       const char *noButtonText,
			       QWidget *parent, const char *name )
{
    QMessageBox *mb = new QMessageBox( parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    if ( yesButtonText )
	mb->setButtonText( yesButtonText );
    else
	mb->setButtonText( "Yes" );
    mb->button->setDefault( TRUE );
    mb->reserved1 = (void*)new QPushButton( mb );
    connect( mb->button2(), SIGNAL(clicked()), mb, SLOT(reject()) );
    if ( noButtonText )
	mb->button2()->setText( noButtonText );
    else
	mb->button2()->setText( "No" );
    mb->button2()->setFont( mb->button->font() );
    mb->button2()->setAutoDefault( TRUE );
    int retcode = mb->exec();
    delete mb;
    return retcode;
}
