/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#30 $
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

RCSTAG("$Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#30 $");


/*!
  \class QMessageBox qmsgbox.h
  \brief The QMessageBox widget provides a message box.
  \ingroup dialogs

  A message box is a modal dialog that displays an icon, a text and up to
  three push buttons.

  There are three very useful static member functions that display
  standard message boxes.  We recommend using these, but you can also
  construct a QMessageBox from scratch and set custom button texts.

  The information message box should be used to display some information
  to the user.  The user must press a button to progress:
  \code
    QMessageBox::information( 0, "Hello",
    			      "Press OK to continue",
			      QMessageBox::OK | QMessageBox::Default );
  \endcode

  The warning message box should be used to display a warning to the
  user and ask the user to confirm or to cancel the operation.  A typical
  use of a warning box is an exit confirmation message box:
  \code
    int reply =
         QMessageBox::warning( 0, "Exit Program",
			       "Are you really sure you want to exit?",
			       QMessageBox::Yes,
			       QMessageBox::No | QMessageBox::Default |
			       QMessageBox::Escape );
  \endcode

  The critical message box should be used to display a critical warning
  to the user and ask for confirmation.  It is similar to the warning
  box except that it has another icon:
  \code
    int reply =
        QMessageBox::critical( 0, "Disk Failure",
			       "Cannot write to disk.\n"
			       "The disk is probably full.\n"
			       "Do you want to retry?\n",
			       QMessageBox::Retry,
			       QMessageBox::Cancel );
    if ( reply == QMessageBox::Retry ) {
        // retry the operation
    }
  \endcode

  You can also make a message from scratch and set custom button texts:
  \code
    QMessageBox mb( QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No,
		    QMessageBox::Cancel | QMessageBox::Escape );
    mb.setIcon( QMessageBox::Warning );
    mb.setCaption( "Exit Program" );
    mb.setText( "Saving the file will overwrite the old file on disk.\n"
	        "Do you really want to save?" );
    mb.setButtonText( QMessageBox::Yes, "Save" );
    mb.setButtonText( QMessageBox::No, "Don't Save" );
    switch( mb.exec() ) {
        case QMessageBox::Yes:
	    // save and exit
	    break;
        case QMessageBox::No:
	    // exit without saving
	    break;
	case QMessageBox::Cancel:
	    // don't save and don't exit
	    break;
    }
  \endcode

  \bugs Icons in the message box does not work yet.
*/


struct QMBData {
    int			numButtons;		// number of buttons
    QMessageBox::Icon	icon;			// message box icon
    int			button[3];		// button types
    int			defButton;		// default button (index)
    int			escButton;		// escape button (index)
    QSize		buttonSize;		// button size
    QPushButton	       *pb[3];			// buttons
};

static const int LastButton = QMessageBox::Ignore;

/*
  NOTE: The table of button texts correspond to the button enum.
*/

static const char *mb_texts[] = {
    0, "OK", "Cancel", "Yes", "No", "Abort", "Retry", "Ignore", 0
};


/*!
  Constructs a message box with no text and a button with the text "OK".

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  The \e parent and \e name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    init( OK, 0, 0 );
}


/*!
  Constructs a message box with up to three buttons.

  Each button can be one of the following values:
  <ul>
  <li>\c QMessageBox::Ok
  <li>\c QMessageBox::Cancel
  <li>\c QMessageBox::Yes
  <li>\c QMessageBox::No
  <li>\c QMessageBox::Abort
  <li>\c QMessageBox::Retry
  <li>\c QMessageBox::Ignore
  </ul>

  One of the buttons can be combined with the \c QMessageBox::Default flag
  to make a default button.
  
  One of the buttons can be combined with the \c QMessageBox::Escape flag
  to make an escape option.  Hitting the Esc key on the keyboard has
  the same effect as clicking this button with the mouse.

  Example:
  \code
    QMessageBox mb( QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape );
    mb.setCaption( "Hardware failure" );
    mb.setText( "Disk error detected\nDo you want to stop?" );
    if ( mb.exec() == QMessageBox::No )
        // try again
  \endcode

  If \a parent is 0, then the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \e parent.

  The \e parent and \e name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( int button1, int button2, int button3,
			  QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    init( button1, button2, button3 );
}


/*!
  Destroys the message box.
*/

QMessageBox::~QMessageBox()
{
    delete mbd;
}


void QMessageBox::init( int button1, int button2, int button3 )
{
    initMetaObject();
    label = new QLabel( this, "text" );
    CHECK_PTR( label );
    label->setAlignment( AlignLeft );

    if ( (button3 && !button2) || (button2 && !button1) ) {
#if defined(CHECK_RANGE)
	::warning( "QMessageBox: Inconsistent button parameters" ); 
#endif
	button1 = button2 = button3 = 0;
    }
    mbd = new QMBData;
    CHECK_PTR( mbd );
    mbd->numButtons = 0;
    mbd->button[0] = button1;
    mbd->button[1] = button2;
    mbd->button[2] = button3;
    mbd->defButton = -1;
    mbd->escButton = -1;
    int i;
    for ( i=0; i<3; i++ ) {
	int b = mbd->button[i];
	if ( (b & Default) ) {
	    if ( mbd->defButton >= 0 ) {
#if defined(CHECK_RANGE)
		::warning( "QMessageBox: There can be at most one "
			   "default button" );
#endif
	    } else {
		mbd->defButton = i;
	    }
	}
	if ( (b & Escape) ) {
	    if ( mbd->escButton >= 0 ) {
#if defined(CHECK_RANGE)
		::warning( "QMessageBox: There can be at most one "
			   "escape button" );
#endif
	    } else {
		mbd->escButton = i;
	    }
	}
	b &= ButtonMask;
	if ( b == 0 ) {
	    if ( i == 0 )
		b = OK;
	} else if ( b < 0 || b > LastButton ) {
#if defined(CHECK_RANGE)
	    ::warning( "QMessageBox: Invalid button specifier" );
#endif
	    b = OK;
	} else {
	    if ( i > 0 && mbd->button[i-1] == 0 ) {
#if defined(CHECK_RANGE)
		::warning( "QMessageBox: Inconsistent button parameters; "
			   "button %d defined but not button %d",
			   i+1, i );
#endif
		b = 0;
	    }
	}
	mbd->button[i] = b;
	if ( b )
	    mbd->numButtons++;
    }
    for ( i=0; i<3; i++ ) {
	if ( i >= mbd->numButtons ) {
	    mbd->pb[i] = 0;
	} else {
	    QString buttonName;
	    buttonName.sprintf( "button%d", i+1 );
	    mbd->pb[i] = new QPushButton( mb_texts[mbd->button[i]],
					  this, buttonName );
	    if ( mbd->defButton == i ) {
		mbd->pb[i]->setDefault( TRUE );
		mbd->pb[i]->setFocus();
	    }
	    if ( mbd->defButton >= 0 )
		mbd->pb[i]->setAutoDefault( TRUE );
	    mbd->pb[i]->setFocusPolicy( QWidget::StrongFocus );
	    connect( mbd->pb[i], SIGNAL(clicked()), SLOT(buttonClicked()) );
	}
    }
    resizeButtons();
    reserved1 = reserved2 = 0;
}


int QMessageBox::indexOf( int button ) const
{
    int index = -1;
    for ( int i=0; i<mbd->numButtons; i++ ) {
	if ( mbd->button[i] == button ) {
	    index = i;
	    break;
	}
    }
    return index;
}


void QMessageBox::resizeButtons()
{
    int i;
    QSize maxSize(0,0);
    for ( i=0; i<mbd->numButtons; i++ ) {
	QSize s = mbd->pb[i]->sizeHint();
	maxSize.setWidth(  QMAX(maxSize.width(), s.width()) );
	maxSize.setHeight( QMAX(maxSize.height(),s.height()) );
    }
    mbd->buttonSize = maxSize;
    for ( i=0; i<mbd->numButtons; i++ )
	mbd->pb[i]->resize( maxSize );
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
  Returns the icon of the message box.

  The return value is one of the following:
  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical 
  </ul>

  \sa setIcon()
*/

QMessageBox::Icon QMessageBox::icon() const
{
    return mbd->icon;
}


/*!
  Sets the icon the the message box to \a icon.

  \a icon can be one of the following:
  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical 
  </ul>

  \sa icon()
*/

void QMessageBox::setIcon( Icon icon )
{
    mbd->icon = icon;
}


/*!
  This function will be removed in a future version of Qt.
*/

const char *QMessageBox::buttonText() const
{
    return buttonText( OK );
}

/*!
  This function will be removed in a future version of Qt.
*/

void QMessageBox::setButtonText( const char *text )
{
    setButtonText( OK, text ? text : "OK" );
}


/*!
  Returns the text of the message box button \a button, or null if the
  message box does not contain the button.

  Example:
  \code
    QMessageBox mb( QMessageBox::OK, QMessageBox::Cancel, 0 );
    mb.buttonText( QMessageBox::Cancel );  // returns "Cancel"
    mb.buttonText( QMessageBox::Ignore );  // returns 0
  \endcode

  \sa setButtonText()
*/

const char *QMessageBox::buttonText( int button ) const
{
    int index = indexOf(button);
    return index >= 0 && mbd->pb[index] ? mbd->pb[index]->text() : 0;
}


/*!
  Sets the text of the message box button \a button to \a text.
  Setting the text of a button that is not in the message box is quietly
  ignored.

  Example:
  \code
    QMessageBox mb( QMessageBox::OK, QMessageBox::Cancel, 0 );
    mb.setButtonText( QMessageBox::OK, "All Right" );
    mb.setButtonText( QMessageBox::Yes, "Yo" );	  // ignored
  \endcode

  \sa buttonText()
*/

void QMessageBox::setButtonText( int button, const char *text )
{
    int index = indexOf(button);
    if ( index >= 0 && mbd->pb[index] ) {
	mbd->pb[index]->setText( text );
	resizeButtons();
    }
}


/*!
  Internal slot to handle button clicks.
*/

void QMessageBox::buttonClicked()
{
    int reply = 0;
    const QObject *s = sender();
    for ( int i=0; i<mbd->numButtons; i++ ) {
	if ( mbd->pb[i] == s )
	    reply = mbd->button[i];
    }
    done( reply );
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
    QSize smax = mbd->buttonSize;
    int border = smax.height()/2;
    if ( border == 0 )
	border = 10;
    else if ( style() == MotifStyle )
	border += 6;
    for ( i=0; i<mbd->numButtons; i++ )
	mbd->pb[i]->resize( smax );
    label->adjustSize();
    int bw = mbd->numButtons * smax.width() + (mbd->numButtons-1)*border;
    int w = QMAX( bw, label->width() ) + 2*border;
    int h = smax.height();
    if ( label->height() )
	h += label->height() + 3*border;
    else
	h += 2*border;
    resize( w, h );	
}


/*!
  Handles resize events for the message box.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
    int i;
    int n  = mbd->numButtons;
    int bw = mbd->buttonSize.width();
    int bh = mbd->buttonSize.height();
    int border = bh/2;
    if ( border == 0 )
	border = 10;
    label->move( width()/2 - label->width()/2,
		 (height() - border - bh - label->height()) / 2 );
    int space = (width() - bw*n)/(n+1);
    for ( i=0; i<n; i++ ) {
	mbd->pb[i]->move( space*(i+1)+bw*i,
			  height() - border - bh );
    }
}


/*!
  Handles key press events for the message box.
*/

void QMessageBox::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape ) {
	if ( mbd->escButton >= 0 ) {
	    QPushButton *pb = mbd->pb[mbd->escButton];
	    pb->animateClick();
	}
	e->accept();
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
    QMessageBox *mb = new QMessageBox( OK, 0, 0, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    if ( buttonText )
	mb->setButtonText( OK, buttonText );
    int retcode = mb->exec();
    delete mb;
    return retcode == OK;
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
    QMessageBox *mb = new QMessageBox( Yes, No, 0, parent, name );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    if ( yesButtonText )
	mb->setButtonText( Yes, yesButtonText );
    if ( noButtonText )
	mb->setButtonText( No, noButtonText );
    int retcode = mb->exec();
    delete mb;
    return retcode == Yes;
}


/*!
  Opens an information message box with a caption, a text and up to three
  buttons.  Returns the identifier of the button that was clicked.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa warning(), critical()
*/

int QMessageBox::information( QWidget *parent,
			      const char *caption, const char *text,
			      int button1, int button2, int button3 )
{
    QMessageBox *mb = new QMessageBox( button1, button2, button3, parent,
				       "information" );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    mb->setIcon( Information );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Opens a warning message box with a caption, a text and up to three
  buttons.  Returns the identifier of the button that was clicked.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa information(), critical()
*/

int QMessageBox::warning( QWidget *parent,
			  const char *caption, const char *text,
			  int button1, int button2, int button3 )
{
    QMessageBox *mb = new QMessageBox( button1, button2, button3, parent,
				       "warning" );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    mb->setIcon( Warning );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Opens a "critical" message box with a caption, a text and up to three
  buttons.  Returns the identifier of the button that was clicked.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa information(), warning()
*/

int QMessageBox::critical( QWidget *parent,
			   const char *caption, const char *text,
			   int button1, int button2, int button3 )
{
    QMessageBox *mb = new QMessageBox( button1, button2, button3, parent,
				       "critical" );
    CHECK_PTR( mb );
    mb->setCaption( caption );
    mb->setText( text );
    mb->setIcon( Critical );
    int reply = mb->exec();
    delete mb;
    return reply;
}
