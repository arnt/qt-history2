/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#36 $
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
#include "qimage.h"
#include "qkeycode.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#36 $");

// Message box icons, from page 210 of the Windows style guide.

// Hand-drawn to resemble Microsoft's icons, but in the Mac/Netscape
// palette.  The "question mark" icon, which Microsoft recommends not
// using but a lot of people still use, is left out.

static const unsigned int  information_gif_len = 212;
static const unsigned char information_gif_data[] = {
    0x47,0x49,0x46,0x38,0x39,0x61,0x20,0x00,0x20,0x00,0xc2,0x00,0x00,0xcc,
    0xcc,0xcc,0x99,0x99,0x99,0xff,0xff,0xff,0x00,0x00,0xff,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0xf9,0x04,0x01,0x00,
    0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
    0x99,0x08,0xba,0x0c,0xf1,0xf0,0xb5,0x49,0x17,0x14,0x38,0x63,0x58,0xeb,
    0xd3,0x20,0x28,0x75,0x56,0x68,0x8a,0xa4,0x73,0x0e,0xec,0x70,0x12,0x5d,
    0xb0,0xb6,0xec,0x4b,0xc9,0x33,0x7d,0x0a,0x30,0x83,0xef,0xba,0x1d,0xcf,
    0x27,0x2c,0x82,0x08,0x01,0xc5,0xcf,0xc8,0x44,0xaa,0x80,0xb4,0x9a,0xd0,
    0xb9,0xcc,0x49,0x77,0xc8,0x4f,0x31,0x5a,0xcc,0x56,0x4d,0xdc,0xe9,0x65,
    0x1b,0xc4,0x8e,0x85,0x61,0x73,0xf2,0x0b,0x4a,0x9b,0xbc,0x4f,0x68,0x4b,
    0x3c,0x62,0x67,0xdc,0x47,0xce,0x82,0x00,0x8e,0xce,0xf3,0x7a,0x7b,0x4c,
    0x6f,0x11,0x14,0x7c,0x83,0x19,0x5e,0x49,0x15,0x04,0x87,0x46,0x8d,0x85,
    0x24,0x01,0x8d,0x8e,0x47,0x90,0x91,0x29,0x10,0x94,0x1a,0x8a,0x81,0x29,
    0x4a,0x9a,0x89,0x11,0x8b,0x9f,0x37,0x8e,0x59,0xa5,0xa5,0x87,0xa8,0xa9,
    0x9f,0x7c,0xac,0xad,0x29,0x97,0xb1,0xa5,0x9e,0xb4,0x31,0xa4,0x15,0x09,
    0x00,0x3b
};

static const unsigned int  warning_gif_len = 190;
static const unsigned char warning_gif_data[] = {
    0x47,0x49,0x46,0x38,0x39,0x61,0x20,0x00,0x20,0x00,0xa1,0x00,0x00,0x00,
    0x00,0x00,0x99,0x99,0x99,0xcc,0xcc,0xcc,0xff,0xff,0x00,0x21,0xf9,0x04,
    0x01,0x00,0x00,0x02,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,
    0x00,0x02,0x8f,0x94,0x8f,0x07,0x9b,0xed,0x6f,0xc0,0x98,0x00,0x5a,0x27,
    0x27,0x0d,0xb7,0x8b,0xac,0x01,0x01,0xe7,0x3d,0xa0,0x36,0x88,0x64,0x99,
    0x9c,0xe1,0xc8,0xb6,0xe8,0xac,0xc6,0xca,0x8c,0xd6,0xb6,0x8b,0xeb,0xec,
    0xb9,0x00,0xc2,0x7e,0xb9,0xa0,0x70,0xd5,0x01,0x1a,0x73,0xc3,0x24,0x6d,
    0xc9,0x44,0x42,0x5c,0x46,0x6a,0x73,0xda,0x83,0x46,0x2f,0xbc,0x6a,0xef,
    0xda,0xe0,0x51,0x16,0xb8,0x8d,0x14,0x21,0x4e,0x91,0xcb,0x3e,0x59,0x59,
    0x9d,0x4e,0x81,0x23,0x6f,0x4a,0x5d,0x7e,0x8e,0xdb,0xef,0xed,0xcf,0xfd,
    0x6f,0x86,0x06,0xb8,0x56,0xe7,0xa3,0x07,0xa7,0xa7,0x73,0x88,0xf8,0x57,
    0xb3,0xa8,0x36,0x08,0xf3,0x08,0x68,0x16,0x30,0x49,0xa9,0xc2,0x71,0xc9,
    0x37,0x32,0x54,0xf5,0x09,0x0a,0xda,0x79,0x35,0x5a,0x6a,0x7a,0x5a,0xda,
    0x80,0xba,0x8a,0x6a,0x50,0x00,0x00,0x3b
};

static const unsigned int  critical_gif_len = 191;
static const unsigned char critical_gif_data[] = {
    0x47,0x49,0x46,0x38,0x39,0x61,0x20,0x00,0x20,0x00,0xa1,0x00,0x00,0xcc,
    0xcc,0xcc,0xff,0x00,0x00,0x99,0x99,0x99,0xff,0xff,0xff,0x21,0xf9,0x04,
    0x01,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,
    0x00,0x02,0x90,0x84,0x8f,0x10,0xcb,0x9b,0x0f,0x51,0x9b,0x2c,0x5a,0x45,
    0xb3,0xbb,0x52,0x7b,0x6e,0x78,0x62,0x20,0x5c,0xe3,0x58,0x42,0xe7,0x99,
    0x76,0xd3,0x20,0xc2,0x54,0x1b,0xbe,0x83,0x4c,0xdd,0x38,0x43,0xe7,0xfa,
    0xbe,0xf8,0x01,0x49,0x87,0x8c,0xb0,0x71,0x9c,0xd5,0x8c,0xbf,0x60,0x53,
    0x53,0x8a,0x09,0x9f,0x50,0x8c,0x67,0x7a,0x43,0x09,0x4e,0x53,0xd6,0x76,
    0xd4,0xd5,0x82,0xb1,0xe2,0x2b,0x36,0xeb,0x11,0x7c,0x99,0x3a,0x67,0x1b,
    0x1a,0xf5,0xbd,0xdd,0xe8,0x89,0x7a,0xc9,0x48,0x22,0xa9,0xbc,0x96,0x5c,
    0x33,0xd7,0xe7,0x97,0x13,0x93,0x71,0xe7,0xb2,0x02,0x47,0x83,0x97,0xd8,
    0xa0,0xb6,0x68,0xb0,0xd6,0xb8,0xf0,0x68,0x21,0x99,0x58,0x79,0x71,0xa9,
    0x05,0x09,0xa1,0xe6,0x75,0x08,0x12,0xb9,0x49,0xf9,0xd8,0x09,0x62,0x9a,
    0x1a,0x2a,0xea,0xa9,0x0a,0x52,0x00,0x00,0x3b
};


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
    QMessageBox mb( "Exit Program",
		    "Saving the file will overwrite the old file on disk.\n"
		    "Do you really want to save?",
		    QMessageBox::Warning,
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No,
		    QMessageBox::Cancel | QMessageBox::Escape );
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
    QMBData(QMessageBox* parent) :
	iconLabel( parent )
    {
    }

    int			numButtons;		// number of buttons
    QMessageBox::Icon	icon;			// message box icon
    QLabel		iconLabel;		// label holding any icon
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
  Constructs a message box with a \a caption, a \a text, an \a icon and up
  to three buttons.

  The \a icon must be one of:
  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical 
  </ul>

  Each button can have one of the following values:
  <ul>
  <li>\c QMessageBox::OK
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
    QMessageBox mb( "Hardware failure",
		    "Disk error detected\nDo you want to stop?",
		    QMessageBox::NoIcon,
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape );
    if ( mb.exec() == QMessageBox::No )
        // try again
  \endcode

  If \a parent is 0, then the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \e parent.

  If \a modal is TRUE the message becomes modal, otherwise it becomes
  modeless.

  The \a parent, \a name, \a modal and \a f arguments are passed to the
  QDialog constructor.

  \sa setCaption(), setText(), setIcon()
*/

QMessageBox::QMessageBox( const char *caption, const char *text, Icon icon,
			  int button1, int button2, int button3,
			  QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    init( button1, button2, button3 );
    setCaption( caption );
    setText( text );
    setIcon( icon );
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
    mbd = new QMBData(this);
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
    QSize maxSize( style() == MotifStyle ? 0 : 75, 0 );
    for ( i=0; i<mbd->numButtons; i++ ) {
	QSize s = mbd->pb[i]->sizeHint();
	maxSize.setWidth(  QMAX(maxSize.width(), s.width()) );
	maxSize.setHeight( QMAX(maxSize.height(),s.height()) );
    }
    mbd->buttonSize = maxSize;
    for ( i=0; i<mbd->numButtons; i++ )
	mbd->pb[i]->resize( maxSize );
}


void QMessageBox::show()
{
    QDialog::show();
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

  \sa setIcon(), iconPixmap()
*/

QMessageBox::Icon QMessageBox::icon() const
{
    return mbd->icon;
}


/*!
  Sets the icon of the message box to \a icon, which is a predefined icon:

  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical 
  </ul>

  The actual pixmap used for displaying the icon depends on the current
  \link style() GUI style\endlink.  You can also set a custom pixmap icon
  using the setIconPixmap() function.

  \sa icon(), setIconPixmap(), iconPixmap()
*/

void QMessageBox::setIcon( Icon icon )
{
    setIconPixmap( standardIcon(icon, style()) );
    mbd->icon = icon;
}

/*!
  Returns the pixmap used for a standard icon.  This
  allows the pixmaps to be used in more complex message boxes.
*/
QPixmap QMessageBox::standardIcon( Icon icon, GUIStyle style )
{
    uint icon_size;
    const uchar *icon_data;
    switch ( icon ) {
	case Information:
	    icon_size = information_gif_len;
	    icon_data = information_gif_data;
	    break;
	case Warning:
	    icon_size = warning_gif_len;
	    icon_data = warning_gif_data;
	    break;
	case Critical:
	    icon_size = critical_gif_len;
	    icon_data = critical_gif_data;
	    break;
	default:
	    icon_size = 0;
	    icon_data = 0;	    
    }
    QPixmap pm;
    if ( icon_size ) {
	QImage image;
	image.loadFromData( icon_data, icon_size );
	if ( style == MotifStyle ) {
	    // All that colour looks ugly in Motif
	    QColorGroup g = QApplication::palette()->normal();
	    switch ( icon ) {
	      case Information:
		image.setColor( 2, 0xff000000 | g.light().rgb() );
		image.setColor( 3, 0xff000000 | g.text().rgb() );
		break;
	      case Warning:
		image.setColor( 3, 0xff000000 | g.base().rgb() );
		break;
	      case Critical:
		image.setColor( 1, 0xff000000 | qRgb(0,0,0) );
	        break;
	      default:
		; // Can't happen
	    }
	}
	pm.convertFromImage(image);
    }
    return pm;
}


/*!
  Returns the icon pixmap of the message box.

  Example:
  \code
    QMessageBox mb(...);
    mb.setIcon( QMessageBox::Warning );
    mb.iconPixmap();	// returns the warning icon pixmap
  \endcode

  \sa setIconPixmap(), icon()
*/

const QPixmap *QMessageBox::iconPixmap() const
{
    return mbd->iconLabel.pixmap();
}

/*!
  Sets the icon of the message box to a custom \a pixmap.

  \sa iconPixmap(), setIcon()
*/

void QMessageBox::setIconPixmap( const QPixmap &pixmap )
{
    mbd->iconLabel.setPixmap(pixmap);
    mbd->icon = NoIcon;
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
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )  {
	mbd->iconLabel.adjustSize();
	w += mbd->iconLabel.pixmap()->width() + border;
    }
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
    else if ( style() == MotifStyle )
	border += 6;
    int lmargin = 0;
    mbd->iconLabel.adjustSize();
    mbd->iconLabel.move( border, border );
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )
	lmargin += mbd->iconLabel.pixmap()->width() + border;
    label->move( (width() + lmargin)/2 - label->width()/2,
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
    QMessageBox *mb = new QMessageBox( caption, text, NoIcon, OK, 0, 0,
				       parent, name );
    CHECK_PTR( mb );
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
    QMessageBox *mb = new QMessageBox( caption, text, NoIcon,
				       Yes, No, 0,
				       parent, name );
    CHECK_PTR( mb );
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
    QMessageBox *mb = new QMessageBox( caption, text, Information,
				       button1, button2, button3,
				       parent, "information" );
    CHECK_PTR( mb );
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
    QMessageBox *mb = new QMessageBox( caption, text, Warning,
				       button1, button2, button3,
				       parent, "warning" );
    CHECK_PTR( mb );
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
    QMessageBox *mb = new QMessageBox( caption, text, Critical,
				       button1, button2, button3,
				       parent, "critical" );
    CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}

void QMessageBox::setStyle( GUIStyle s )
{
    QWidget::setStyle( s );
    if ( mbd->icon != NoIcon ) {
	// Reload icon for new style
	setIcon( mbd->icon );
    }
}
