/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#22 $
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
#include "qkeycode.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qmsgbox.cpp#22 $");


/*!
  \class QMessageBox qmsgbox.h
  \brief The QMessageBox widget provides a message box.
  \ingroup dialogs

  A message box is a dialog that displays a text and a push button.

  The default push button text is "Ok". This can be changed with
  setButtonText().

  Enabling auto-resizing will make a message box resize itself whenever
  the contents change.

  Example:
  \code
    QMessageBox mb( QMessageBox::Yes );
    mb.setText( "This program may crash your hardware!!!\nLet's start..." );
    mb.show();
  \endcode
*/


struct QMBData {
    int		 buttonSpec;
    int          numButtons;			// number of buttons
    int		 type[3];			// button types
    QPushButton *button[3];			// buttons
    QSize	 buttonSize;
};

static int mb_buttons[] = {
    QMessageBox::Ok,
    QMessageBox::Yes,
    QMessageBox::No,
    QMessageBox::Abort,
    QMessageBox::Retry,
    QMessageBox::Ignore,
    QMessageBox::Cancel,
    0
};

static const char *mb_texts[] = {
    "Ok", "Yes", "No", "Abort", "Retry", "Ignore", "Cancel", 0
};


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
    init( Ok, 0 );
}


/*!
  Constructs a message box with a button specification \a buttons and a
  \a text.

  The \a buttons argument consists of several buttons OR'ed together:
  <ul>
  <li>\c QMessageBox::Ok
  <li>\c QMessageBox::Cancel
  <li>\c QMessageBox::Yes
  <li>\c QMessageBox::No
  <li>\c QMessageBox::Abort
  <li>\c QMessageBox::Retry
  <li>\c QMessageBox::Ignore
  </ul>

  You may create any combination of buttons, but max three buttons.

  Example:
  \code
    QMessageBox mb( QMessageBox::Ok | QMessageBox::Ignore,
		    "Disk error detected\n"
		    "Do you want to stop?" );
    mb.setCaption( "Hardware failure" );
    if ( mb.exec() == QMessageBox::Ignore )
        // try again
  \endcode

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message
  box becomes modal relative to \e parent.

  The \e parent and \e name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( int buttons, const char *text,
			  QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    init( buttons, text );
}


void QMessageBox::init( int buttons, const char *text )
{
    QFont font( "Helvetica", 12 );

    initMetaObject();
    label = new QLabel( text, this, "text" );
    CHECK_PTR( label );
    label->setAlignment( AlignCenter );
    label->setFont( font );
    font.setWeight( QFont::Bold );

    d = new QMBData;
    CHECK_PTR( d );
    int n=0, i=0;
    d->buttonSpec = buttons;
    d->buttonSize = QSize(0,0);
    while ( mb_buttons[i] ) {
	if ( (buttons & mb_buttons[i]) ) {
	    if ( n == 3 ) {
#if defined(CHECK_RANGE)
		warning( "QMessageBox: Invalid buttons parameter" );
#endif
		break;
	    }
	    QString bn;
	    bn.sprintf( "button_%d", mb_buttons[i] );	// button name
	    QPushButton *b = new QPushButton( mb_texts[i], this, bn );
	    b->setFont( font );
	    QSize s = b->sizeHint();
	    d->buttonSize.setWidth(  QMAX(d->buttonSize.width(), s.width()) );
	    d->buttonSize.setHeight( QMAX(d->buttonSize.height(),s.height()) );
	    connect( b, SIGNAL(clicked()), SLOT(buttonClicked()) );
	    d->type[n] = mb_buttons[i];
	    d->button[n] = b;
	    b->setAutoDefault( TRUE );
	    b->setFocusPolicy( QWidget::StrongFocus );
	    n++;
	}
	i++;
    }
    if ( n ) {
	d->button[0]->setDefault( TRUE );
	d->button[0]->setFocus();
    }
    d->numButtons = n;
    reserved1 = reserved2 = 0;
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
    return buttonText( Ok );
}

/*!
  Sets the push button text to be displayed.

  The default push button text is "Ok".

  \sa buttonText()
*/

void QMessageBox::setButtonText( const char *text )
{
    setButtonText( Ok, text ? text : "Ok" );
}


/*!
  Returns the text of one of the push buttons, or null if there is no such
  button or text.
  \sa setButtonText()
*/

const char *QMessageBox::buttonText( int button ) const
{
    QPushButton *b = 0;
    for ( int i=0; i<d->numButtons; i++ ) {
	if ( d->type[i] == button )
	    b = d->button[i];
    }
    return b ? b->text() : 0;
}

void QMessageBox::setButtonText( int button, const char *text )
{
    QPushButton *b = 0;
    for ( int i=0; i<d->numButtons; i++ ) {
	if ( d->type[i] == button )
	    b = d->button[i];
    }
    if ( b ) {
	b->setText( text );
	QSize s = b->sizeHint();
	d->buttonSize.setWidth(  QMAX(d->buttonSize.width(), s.width()) );
	d->buttonSize.setHeight( QMAX(d->buttonSize.height(),s.height()) );
    }
}


/*!
  Internal slot.
*/

void QMessageBox::buttonClicked()
{
    int type = 0;
    const QObject *s = sender();
    for ( int i=0; i<d->numButtons; i++ ) {
	if ( d->button[i] == s )
	    type = d->type[i];
    }
    done( type );
}


/*!
  Adjusts the size of the message box to fit the contents just before
  QDialog::exec() or QDialog::show() is called.

  This function will not be called if the message box has been explicitly
  resized before showing it.
*/

void QMessageBox::adjustSize()
{
    int i;
    QSize smax = d->buttonSize;
    int border = smax.height()/2;
    if ( border == 0 )
	border = 10;
    else if ( style() == MotifStyle )
	border += 6;
    for ( i=0; i<d->numButtons; i++ )
	d->button[i]->resize( smax );
    label->adjustSize();
    int bw = d->numButtons * smax.width() + (d->numButtons-1)*border;
    int w = QMAX( bw, label->width() ) + 2*border;
    int h = smax.height();
    if ( label->height() )
	h += label->height() + 3*border;
    else
	h += 2*border;
    resize( w, h );	
}


/*!
  Internal geometry management.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
    int i;
    int n  = d->numButtons;
    int bw = d->buttonSize.width();
    int bh = d->buttonSize.height();
    int border = bh/2;
    if ( border == 0 )
	border = 10;
    label->move( width()/2 - label->width()/2,
		 (height() - border - bh - label->height()) / 2 );
    int space = (width() - bw*n)/(n+1);
    for ( i=0; i<n; i++ ) {
	d->button[i]->move( space*(i+1)+bw*i,
			    height() - border - bh );
    }
}


void QMessageBox::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape ) {
	int r = 0;
	if ( d->buttonSpec & Ignore )
	    r = Ignore;
	else if ( d->buttonSpec & Retry )
	    r = Retry;
	else if ( d->buttonSpec & Cancel )
	    r = Cancel;
	else if ( d->buttonSpec & No )
	    r = No;
	done( r );
    } else {
	QDialog::keyPressEvent( e );
    }
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
    QMessageBox *mb = new QMessageBox( Ok, text, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    if ( buttonText )
	mb->setButtonText( Ok, buttonText );
    int retcode = mb->exec();
    delete mb;
    return retcode == Ok;
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
    QMessageBox *mb = new QMessageBox( Yes|No, text, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    if ( yesButtonText )
	mb->setButtonText( Yes, yesButtonText );
    if ( noButtonText )
	mb->setButtonText( No, noButtonText );
    int retcode = mb->exec();
    delete mb;
    return retcode == Yes;
}


/*!
  Opens a message box with an "Ok" and a "Cancel" button.
  Returns the button that was pressed.

  Note that \a caption is not always shown, it depends on the window
  manager.

  Example:
  \code
    if ( QMessageBox::okCancel(title,text) == QMessageBox::Ok )
	... // Ok was pressed
  \endcode
*/

int QMessageBox::okCancel( const char *caption,
			   const char *text,
			   QWidget *parent, const char *name )
{
    QMessageBox *mb = new QMessageBox( Ok|Cancel, text, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    int retcode = mb->exec();
    delete mb;
    return retcode ? retcode : Cancel;
}


/*!
  Opens a message box with a "Yes" and a "No" button.
  Returns the button that was pressed.

  Note that \a caption is not always shown, it depends on the window
  manager.

  Example:
  \code
    if ( QMessageBox::yesNo(title,text) == QMessageBox::Yes )
	... // Yes was pressed
  \endcode
*/

int QMessageBox::yesNo( const char *caption,
			const char *text,
			QWidget *parent, const char *name )
{
    QMessageBox *mb = new QMessageBox( Yes|No, text, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    int retcode = mb->exec();
    delete mb;
    return retcode;
}


/*!
  Opens a message box with "Yes", "No" and "Cancel" buttons.
  Returns the button that was pressed.

  Note that \a caption is not always shown, it depends on the window
  manager.

  Example:
  \code
    int r = QMessageBox::yesNoCancel(title,text);
    switch ( r ) {
	case QMessageBox::Yes:
	    // "Yes" was pressed
	    break;
	case QMessageBox::No:
	    // "No" was pressed
	    break;
	default:
	    // "Cancel"
   }
  \endcode
*/

int QMessageBox::yesNoCancel( const char *caption,
			      const char *text,
			      QWidget *parent, const char *name )
{
    QMessageBox *mb = new QMessageBox( Yes|No|Cancel, text, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    int retcode = mb->exec();
    delete mb;
    return retcode;
}
