/****************************************************************************
** $Id$
**
** Implementation of QMessageBox class
**
** Created : 950503
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmessagebox.h"

#ifndef QT_NO_MESSAGEBOX

#include "qaccel.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qimage.h"
#include "qapplication.h"
#include "qstyle.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif


// the Qt logo, for aboutQt
/* XPM */
static const char * const qtlogo_xpm[] = {
/* width height ncolors chars_per_pixel */
"50 50 17 1",
/* colors */
"  c #000000",
". c #495808",
"X c #2A3304",
"o c #242B04",
"O c #030401",
"+ c #9EC011",
"@ c #93B310",
"# c #748E0C",
"$ c #A2C511",
"% c #8BA90E",
"& c #99BA10",
"* c #060701",
"= c #181D02",
"- c #212804",
"; c #61770A",
": c #0B0D01",
"/ c None",
/* pixels */
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$@;.o=::=o.;@$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X*         **X#+$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$#oO*         O  **o#+$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$&.* OO              O*.&$$$$$$$$$$$$$",
"$$$$$$$$$$$$@XOO            * OO    X&$$$$$$$$$$$$",
"$$$$$$$$$$$@XO OO  O  **:::OOO OOO   X@$$$$$$$$$$$",
"$$$$$$$$$$&XO      O-;#@++@%.oOO      X&$$$$$$$$$$",
"$$$$$$$$$$.O  :  *-#+$$$$$$$$+#- : O O*.$$$$$$$$$$",
"$$$$$$$$$#*OO  O*.&$$$$$$$$$$$$+.OOOO **#$$$$$$$$$",
"$$$$$$$$+-OO O *;$$$$$$$$$$$&$$$$;*     o+$$$$$$$$",
"$$$$$$$$#O*  O .+$$$$$$$$$$@X;$$$+.O    *#$$$$$$$$",
"$$$$$$$$X*    -&$$$$$$$$$$@- :;$$$&-    OX$$$$$$$$",
"$$$$$$$@*O  *O#$$$$$$$$$$@oOO**;$$$#    O*%$$$$$$$",
"$$$$$$$;     -+$$$$$$$$$@o O OO ;+$$-O   *;$$$$$$$",
"$$$$$$$.     ;$$$$$$$$$@-OO OO  X&$$;O    .$$$$$$$",
"$$$$$$$o    *#$$$$$$$$@o  O O O-@$$$#O   *o$$$$$$$",
"$$$$$$+=    *@$$$$$$$@o* OO   -@$$$$&:    =$$$$$$$",
"$$$$$$+:    :+$$$$$$@-      *-@$$$$$$:    :+$$$$$$",
"$$$$$$+:    :+$$$$$@o* O    *-@$$$$$$:    :+$$$$$$",
"$$$$$$$=    :@$$$$@o*OOO      -@$$$$@:    =+$$$$$$",
"$$$$$$$-    O%$$$@o* O O    O O-@$$$#*   OX$$$$$$$",
"$$$$$$$. O *O;$$&o O*O* *O      -@$$;    O.$$$$$$$",
"$$$$$$$;*   Oo+$$;O*O:OO--      Oo@+=    *;$$$$$$$",
"$$$$$$$@*  O O#$$$;*OOOo@@-O     Oo;O*  **@$$$$$$$",
"$$$$$$$$X* OOO-+$$$;O o@$$@-    O O     OX$$$$$$$$",
"$$$$$$$$#*  * O.$$$$;X@$$$$@-O O        O#$$$$$$$$",
"$$$$$$$$+oO O OO.+$$+&$$$$$$@-O         o+$$$$$$$$",
"$$$$$$$$$#*    **.&$$$$$$$$$$@o      OO:#$$$$$$$$$",
"$$$$$$$$$+.   O* O-#+$$$$$$$$+;O    OOO:@$$$$$$$$$",
"$$$$$$$$$$&X  *O    -;#@++@#;=O    O    -@$$$$$$$$",
"$$$$$$$$$$$&X O     O*O::::O      OO    Oo@$$$$$$$",
"$$$$$$$$$$$$@XOO                  OO    O*X+$$$$$$",
"$$$$$$$$$$$$$&.*       **  O      ::    *:#$$$$$$$",
"$$$$$$$$$$$$$$$#o*OO       O    Oo#@-OOO=#$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X:* *     O**X#+$$@-*:#$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$%;.o=::=o.#@$$$$$$@X#$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$+$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
};




// BEING REVISED: paul
/*!
\class QMessageBox qmessagebox.h
\brief The QMessageBox class provides a modal dialog with a short message, an icon, and some buttons.
\ingroup dialogs
  \mainclass

A message box is a modal dialog that displays an icon, some text and up
to three push buttons.  It's used for simple messages and questions.

QMessageBox provides a range of different messages, arranged roughly
along two axes: severity and complexity.

Severity is
\list
\i Information - for message boxes that are part of normal operation
\i Warning - for message boxes that tell the user about unusual errors
\i Critical - as Warning, but for critical errors
\endlist

The message box has a different icon for each of the severity levels.


Complexity is one button (OK) for a simple messages, or two or even
three buttons for questions.

There are static functions for common cases.
For example:

If a program is unable to find a supporting file, but can do perfectly
well without it:

\code
QMessageBox::information( this, "Application name",
    "Unable to find the user preferences file.\n"
    "The factory default will be used instead." );
\endcode

warning() can be used to tell the user about unusual errors, or
errors which can't be easily fixed:

\code
switch( QMessageBox::warning( this, "Application name",
        "Could not connect to the <mumble> server.\n"
        "This program can't function correctly "
        "without the server.\n\n",
        "Retry",
        "Quit", 0, 0, 1 );
    case 0: // The user clicked the Retry again button or pressed Enter
	// try again
	break;
    case 1: // The user clicked the Quit or pressed Escape
	// exit
	break;
}
\endcode

  The text part of all message box messages can be either rich text or
  plain text. If you specify a rich text formatted string, it will be
  rendered using the default stylesheet. See
  QStyleSheet::defaultSheet() for details. With certain strings that
  contain XML meta characters, the auto-rich text detection may fail,
  interpreting plain text incorrectly as rich text. In these rare cases,
  use QStyleSheet::convertFromPlainText() to convert your plain text
  string to a visually equivalent rich text string or set the text
  format explicitly with setTextFormat().

  Below are some examples of how to use the static member functions.
  After these examples you will find an overview of the non-static
  member functions.

  If a program is unable to find a supporting file, it may do the following:

\code
QMessageBox::information( this, "Application name here",
    "Unable to find the file \"index.html\".\n"
    "The factory default will be used instead." );
\endcode

  The Microsoft Windows User Interface Guidelines strongly recommend
  using the application name as the window's caption.  The message box
  has just one button, OK, and its text tells the user both what
  happened and what the program will do about it.  Because the
  application is able to make do, the message box is just information,
  not a warning or a critical error.

  Exiting a program is part of its normal operation. If there is
  unsaved data the user probably should be asked if they want to save
  the data. For example:

\code
switch( QMessageBox::information( this, "Application name here",
	"The document contains unsaved changes\n"
	"Do you want to save the changes before exiting?",
	"&Save", "&Discard", "Cancel",
	0,      // Enter == button 0
	2 ) ) { // Escape == button 2
    case 0: // Save clicked or Alt+S pressed or Enter pressed.
	// save
	break;
    case 1: // Discard clicked or Alt+D pressed
	// don't save but exit
	break;
    case 2: // Cancel clicked or Alt+C pressed or Escape pressed
	// don't exit
	break;
}
\endcode

  The application name is used as the window caption in accordance with
  the Microsoft recommendation.  The Escape button cancels the entire
  exit operation, and pressing Enter causes the changes to be saved
  before the exit occurs.

  Disk full errors are unusual (in a perfect world, they are) and they
  certainly can be hard to correct.  This example uses predefined buttons
  instead of hard-coded button texts:

\code
switch( QMessageBox::warning( this, "Application name here",
	"Could not save the user preferences,\n"
	"because the disk is full.  You can delete\n"
	"some files and press Retry, or you can\n"
	"abort the Save Preferences operation.",
	QMessageBox::Retry | QMessageBox::Default,
	QMessageBox::Abort | QMessageBox::Escape )) {
    case QMessageBox::Retry: // Retry clicked or Enter pressed
	// try again
	break;
    case QMessageBox::Abort: // Abort clicked or Escape pressed
	// abort
	break;
}
\endcode

  The critical() function should be reserved for critical errors.  In
  this example errorDetails is a QString or const char*, and QString
  is used to concatenate several strings:

\code
QMessageBox::critical( 0, "Application name here",
	QString("An internal error occurred. Please ") +
	"call technical support at 123456789 and report\n"+
	"these numbers:\n\n" + errorDetails +
	"\n\n<Application> will now exit." );
\endcode

  In this example an OK button is displayed.

  QMessageBox provides a very simple About box, which displays an
  appropriate icon and the string you provide:

\code
QMessageBox::about( this, "About <Application>",
	"<Application> is a <one-paragraph blurb>\n\n"
	"Copyright 1951-2002 Such-and-such.  "
	"<License words here.>\n\n"
	"For technical support, call 123456789 or see\n"
	"http://www.such-and-such.com/Application/\n" );
\endcode

  See about() for more information.

  Finally, you can create a QMessageBox from scratch and with custom
  button texts:

\code
QMessageBox mb( "Application name here",
	"Saving the file will overwrite the original file on the disk.\n"
	"Do you really want to save?",
	QMessageBox::Information,
	QMessageBox::Yes | QMessageBox::Default,
	QMessageBox::No,
	QMessageBox::Cancel | QMessageBox::Escape );
mb.setButtonText( QMessageBox::Yes, "Save" );
mb.setButtonText( QMessageBox::No, "Discard" );
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

  QMessageBox defines two enum types: Icon and an unnamed button type.
  Icon defines the \c Information, \c Warning, and \c Critical icons for
  each GUI style.  It is used by the constructor and by the static
  member functions information(), warning() and critical(). A function
  called standardIcon() gives you access to the various icons.

  The button types are:
  \list
  \i Ok - the default for single-button message boxes
  \i Cancel - note that this is \e not automatically Escape
  \i Yes
  \i No
  \i Abort
  \i Retry
  \i Ignore
  \endlist

  Button types can be combined with two modifiers by using OR, '|':
  \list
  \i Default - makes pressing Enter equivalent to
  clicking this button.  Normally used with Ok, Yes or similar.
  \i Escape - makes pressing Escape equivalent to clicking this button.
  Normally used with Abort, Cancel or similar.
  \endlist

  The text(), icon() and iconPixmap() functions provide access to the
  current text and pixmap of the message box. The setText(), setIcon()
  and setIconPixmap() let you change it.  The difference between
  setIcon() and setIconPixmap() is that the former accepts a
  QMessageBox::Icon and can be used to set standard icons, whereas the
  latter accepts a QPixmap and can be used to set custom icons.

  setButtonText() and buttonText() provide access to the buttons.

  QMessageBox has no signals or slots.

  <img src=qmsgbox-m.png> <img src=qmsgbox-w.png>

  \sa QDialog,
      \link http://www.iarchitect.com/errormsg.htm
	    Isys on error messages \endlink,
      \link guibooks.html#fowler GUI Design Handbook: Message Box \endlink
*/


/*!
  \enum QMessageBox::Icon

  This enum includes provides the following values:

  \value NoIcon the message box does not have any icon.

  \value Information an icon indicating that the message is nothing
  out of the ordinary.

  \value Warning an icon indicating that the message is a warning, but
  can be dealt with.

  \value Critical an icon indicating that the message represents a
  critical problem.

*/


struct QMessageBoxData {
    QMessageBoxData(QMessageBox* parent) :
        iconLabel( parent, "icon" )
    {
    }

    int                 numButtons;             // number of buttons
    QMessageBox::Icon   icon;                   // message box icon
    QLabel              iconLabel;              // label holding any icon
    int                 button[3];              // button types
    int                 defButton;              // default button (index)
    int                 escButton;              // escape button (index)
    QSize               buttonSize;             // button size
    QPushButton        *pb[3];                  // buttons
};

static const int LastButton = QMessageBox::Ignore;

/*
  NOTE: The table of button texts correspond to the button enum.
*/

static const char *mb_texts[] = {
    0,
    QT_TRANSLATE_NOOP("QMessageBox","OK"),
    QT_TRANSLATE_NOOP("QMessageBox","Cancel"),
    QT_TRANSLATE_NOOP("QMessageBox","&Yes"),
    QT_TRANSLATE_NOOP("QMessageBox","&No"),
    QT_TRANSLATE_NOOP("QMessageBox","&Abort"),
    QT_TRANSLATE_NOOP("QMessageBox","&Retry"),
    QT_TRANSLATE_NOOP("QMessageBox","&Ignore"),
    0
};

/*!
  Constructs a message box with no text and a button with the label "OK".

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  The \a parent and \a name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE, WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu )
{
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( parent && parent->icon() && !parent->icon()->isNull() )
        QDialog::setIcon( *parent->icon() );
    else if ( qApp->mainWidget() && qApp->mainWidget()->icon() && !qApp->mainWidget()->icon()->isNull() )
        QDialog::setIcon( *qApp->mainWidget()->icon() );
#endif

    init( Ok, 0, 0 );
}


/*!
  Constructs a message box with a \a caption, a \a text, an \a icon, and up
  to three buttons.

  The \a icon must be one of the following:
  \list
  \i QMessageBox::NoIcon
  \i QMessageBox::Information
  \i QMessageBox::Warning
  \i QMessageBox::Critical
  \endlist

  Each button, \a button0, \a button1 and \a button2, can have one of
  the following values:
  \list
  \i QMessageBox::NoButton
  \i QMessageBox::Ok
  \i QMessageBox::Cancel
  \i QMessageBox::Yes
  \i QMessageBox::No
  \i QMessageBox::Abort
  \i QMessageBox::Retry
  \i QMessageBox::Ignore
  \endlist

  Use QMessageBox::NoButton for the later parameters to have fewer than
  three buttons in your message box.

  One of the buttons can be OR-ed with the \c QMessageBox::Default flag
  to make it the default button (clicked when Enter is pressed).

  One of the buttons can be OR-ed with the \c QMessageBox::Escape flag
  to make it the cancel or close button (clicked when Escape is pressed).

  Example:
\code
QMessageBox mb( "Application Name",
	"Hardware failure.\n\nDisk error detected\nDo you want to stop?",
	QMessageBox::NoIcon,
	QMessageBox::Yes | QMessageBox::Default,
	QMessageBox::No  | QMessageBox::Escape );
if ( mb.exec() == QMessageBox::No )
    // try again
\endcode

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  If \a modal is TRUE the message becomes modal; otherwise it becomes
  modeless.

  The \a parent, \a name, \a modal, and \a f arguments are passed to the
  QDialog constructor.

  \sa setCaption(), setText(), setIcon()
*/

QMessageBox::QMessageBox( const QString& caption,
			  const QString &text, Icon icon,
			  int button0, int button1, int button2,
			  QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f | WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu )
{
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( parent && parent->icon() && !parent->icon()->isNull() )
        QDialog::setIcon( *parent->icon() );
    else if ( qApp->mainWidget() && qApp->mainWidget()->icon() && !qApp->mainWidget()->icon()->isNull() )
        QDialog::setIcon( *qApp->mainWidget()->icon() );
#endif

    init( button0, button1, button2 );
#ifndef QT_NO_WIDGET_TOPEXTRA
    setCaption( caption );
#endif
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

#if defined(QT_NON_COMMERCIAL)
static char* unForK( const char *f ) {
    char *res = new char[strlen(f)+1];
    int i = 0;
    while ( f[i] ) {
 res[i] = f[i] ^ 5;
 i++;
    }
    res[i] = '\0';
    return res;
}
#endif

static QString * translatedTextAboutQt = 0;

void QMessageBox::init( int button0, int button1, int button2 )
{
    if ( !translatedTextAboutQt ) {
	translatedTextAboutQt = new QString;

#if defined(QT_NON_COMMERCIAL)
	// This needs to be updated
	char* q = unForK("\x54\x71");
	char* t = unForK("\x51\x77\x6a\x69\x69\x71\x60\x66\x6d");
	char* line1 = unForK("\x39\x6d\x36\x3b\x44\x67\x6a\x70\x71\x25\x54\x71\x39\x2a\x6d\x36\x3b");
	char* line2 = unForK("\x39\x67\x3b\x39\x63\x6a\x6b\x71\x25\x66\x6a\x69\x6a\x77\x38\x77\x60\x61\x3b\x51\x6d\x6c\x76\x25\x64\x75\x75\x69\x6c\x66\x64\x71\x6c\x6a\x6b\x25\x6c\x76\x25\x6b\x6a\x6b\x28\x66\x6a\x68\x68\x60\x77\x66\x6c\x64\x69\x25");
	char* line3 = unForK("\x76\x6a\x63\x71\x72\x64\x77\x60\x39\x2a\x63\x6a\x6b\x71\x3b\x39\x2a\x67\x3b");
	char* line4 = unForK("\x39\x75\x3b\x51\x6d\x6c\x76\x25\x64\x75\x75\x69\x6c\x66\x64\x71\x6c\x6a\x6b\x25\x72\x64\x76\x25\x66\x77\x60\x64\x71\x60\x61\x25\x72\x6c\x71\x6d\x25\x6b\x6a\x6b\x28\x66\x6a\x68\x68\x60\x77\x66\x6c\x64\x69\x25\x54\x71\x25\x73\x60\x77\x76\x6c\x6a\x6b");
	char* line5 = unForK("\x25\x20\x34\x2b\x39\x2a\x75\x3b");
	if ( !parentWidget() || ( parentWidget()->caption().find( QString(q) ) == -1 && parentWidget()->caption().find( QString(t) ) == -1 ) ) {
	    *translatedTextAboutQt = tr( QString(line1) +
		  QString(line2) +
		  QString(line3) +
		  QString(line4) + QString(line5) ).arg( QT_VERSION_STR );
	} else {
	    *translatedTextAboutQt = tr( "<h3>About Qt</h3>"
		  "<p>This application was created with Qt version %1.</p>" ).arg( QT_VERSION_STR );
	}
	*translatedTextAboutQt += tr ("<p>This program uses Qt version %1.</p>"
	    "<p>Qt is a C++ toolkit for cross-platform GUI &amp; "
	    "application development from Trolltech."
	    "<p>Qt provides single-source "
	    "portability across Windows 95/98/NT4/ME/2000, Mac&nbsp;OS&nbsp;X, "
	    "Linux, Solaris, "
	    "HP-UX and many other versions of Unix with X11."
	    "<br>Qt is also available for embedded devices.</p>"
	    "<p>See <tt>http://www.trolltech.com/qt/</tt> "
	    "for more information.</p>").arg( QT_VERSION_STR );

	delete[] q;
	delete[] t;
	delete[] line1;
	delete[] line2;
	delete[] line3;
	delete[] line4;
	delete[] line5;
#else
        *translatedTextAboutQt = tr(
	    "<h3>About Qt</h3>"
	    "<p>This program uses Qt version %1.</p>"
	    "<p>Qt is a C++ toolkit for cross-platform GUI &amp; "
	    "application development from Trolltech."
	    "<p>Qt provides single-source "
	    "portability across Windows 95/98/NT4/ME/2000, Mac&nbsp;OS&nbsp;X, "
	    "Linux, Solaris, "
	    "HP-UX and many other versions of Unix with X11."
	    "<br>Qt is also available for embedded devices.</p>"
	    "<p>See <tt>http://www.trolltech.com/qt/</tt> "
	    "for more information.</p>"
	    ).arg( QT_VERSION_STR );
#endif

    }
    label = new QLabel( this, "text" );
    Q_CHECK_PTR( label );
    label->setAlignment( AlignAuto );

    if ( (button2 && !button1) || (button1 && !button0) ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QMessageBox: Inconsistent button parameters" );
#endif
        button0 = button1 = button2 = 0;
    }
    mbd = new QMessageBoxData(this);
    Q_CHECK_PTR( mbd );
    mbd->numButtons = 0;
    mbd->button[0] = button0;
    mbd->button[1] = button1;
    mbd->button[2] = button2;
    mbd->defButton = -1;
    mbd->escButton = -1;
    int i;
    for ( i=0; i<3; i++ ) {
        int b = mbd->button[i];
        if ( (b & Default) ) {
            if ( mbd->defButton >= 0 ) {
#if defined(QT_CHECK_RANGE)
                qWarning( "QMessageBox: There can be at most one "
                           "default button" );
#endif
            } else {
                mbd->defButton = i;
            }
        }
        if ( (b & Escape) ) {
            if ( mbd->escButton >= 0 ) {
#if defined(QT_CHECK_RANGE)
                qWarning( "QMessageBox: There can be at most one "
                           "escape button" );
#endif
            } else {
                mbd->escButton = i;
            }
        }
        b &= ButtonMask;
        if ( b == 0 ) {
            if ( i == 0 )                       // no buttons, add an Ok button
                b = Ok;
        } else if ( b < 0 || b > LastButton ) {
#if defined(QT_CHECK_RANGE)
            qWarning( "QMessageBox: Invalid button specifier" );
#endif
            b = Ok;
        } else {
            if ( i > 0 && mbd->button[i-1] == 0 ) {
#if defined(QT_CHECK_RANGE)
                qWarning( "QMessageBox: Inconsistent button parameters; "
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
            QCString buttonName;
            buttonName.sprintf( "button%d", i+1 );
            mbd->pb[i] = new QPushButton(
                tr(mb_texts[mbd->button[i]]),
                this, buttonName );
	    if ( mbd->defButton == i ) {
                mbd->pb[i]->setDefault( TRUE );
                mbd->pb[i]->setFocus();
            }
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
    QSize maxSize;
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
  \property QMessageBox::text
  \brief the message box text to be displayed.

  The text will be interpreted either as a plain text or as a rich
  text, depending on the text format setting (\l QMessageBox::textFormat). The
  default setting is \c AutoText, i.e. the message box will try to
  auto-detect the format of the text.

  The initial value of the property is QString::null.

  \sa textFormat
*/
QString QMessageBox::text() const
{
    return label->text();
}


void QMessageBox::setText( const QString &text )
{
    label->setText( text );
}


/*!
  \property QMessageBox::icon
  \brief the messagebox icon

  The icon of the message box can be one of the following predefined icons:
  \list
  \i QMessageBox::NoIcon
  \i QMessageBox::Information
  \i QMessageBox::Warning
  \i QMessageBox::Critical
  \endlist

  The actual pixmap used for displaying the icon depends on the current
  \link QWidget::style() GUI style\endlink.  You can also set a custom
  pixmap icon using the \l QMessageBox::iconPixmap property.

  \sa iconPixmap
*/

QMessageBox::Icon QMessageBox::icon() const
{
    return mbd->icon;
}

//#### Bad API (see QWidget::setIcon). Should be setMessageIcon in 3.0 (same for setIconPixmap and friends)
void QMessageBox::setIcon( Icon icon )
{
    setIconPixmap( standardIcon(icon) );
    mbd->icon = icon;
}

/*!
  \obsolete

  Returns the pixmap used for a standard icon.  This
  allows the pixmaps to be used in more complex message boxes.
  \a icon specifies the required icon, e.g. QMessageBox::Information,
  QMessageBox::Warning or QMessageBox::Critical.

  \a style is unused.
*/

QPixmap QMessageBox::standardIcon( Icon icon, GUIStyle style)
{
    Q_UNUSED(style);
    return QMessageBox::standardIcon(icon);
}


/*!
  Returns the pixmap used for a standard icon.  This
  allows the pixmaps to be used in more complex message boxes.
  \a icon specifies the required icon, e.g. QMessageBox::Information,
  QMessageBox::Warning or QMessageBox::Critical.
*/

QPixmap QMessageBox::standardIcon( Icon icon )
{
    QPixmap pm;
    switch ( icon ) {
    case Information:
	pm = QApplication::style().stylePixmap(QStyle::SP_MessageBoxInformation);
        break;
    case Warning:
	pm = QApplication::style().stylePixmap(QStyle::SP_MessageBoxWarning);
        break;
    case Critical:
	pm = QApplication::style().stylePixmap(QStyle::SP_MessageBoxCritical);
        break;
    default:
	break;
    }
    return pm;
}


/*!
  \property QMessageBox::iconPixmap
  \brief the currently icon

  The icon currently used by the message box. Note that
  it's often hard to draw one pixmap that looks appropriate in both
  Motif and Windows GUI styles; you may want to draw two pixmaps.

  \sa icon
*/

const QPixmap *QMessageBox::iconPixmap() const
{
    return mbd->iconLabel.pixmap();
}


void QMessageBox::setIconPixmap( const QPixmap &pixmap )
{
    mbd->iconLabel.setPixmap(pixmap);
    mbd->icon = NoIcon;
}


/*!
  Returns the text of the messagebox button \a button, or null if the
  message box does not contain the button.

  \sa setButtonText()
*/

QString QMessageBox::buttonText( int button ) const
{
    int index = indexOf(button);
    return index >= 0 && mbd->pb[index]
            ? mbd->pb[index]->text()
            : QString::null;
}


/*!
  Sets the text of the message box button \a button to \a text.
  Setting the text of a button that is not in the message box is
  silently ignored.

  \sa buttonText()
*/

void QMessageBox::setButtonText( int button, const QString &text )
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
    if ( !testWState(WState_Polished) )
        polish();
    resizeButtons();
    label->adjustSize();
    QSize labelSize( label->size() );
    int n  = mbd->numButtons;
    int bw = mbd->buttonSize.width();
    int bh = mbd->buttonSize.height();
    int border = bh / 2 - style().pixelMetric(QStyle::PM_ButtonDefaultIndicator);
    if ( border <= 0 )
        border = 10;
    int btn_spacing = 7;
    if ( style().styleHint(QStyle::SH_GUIStyle) == MotifStyle )
        btn_spacing = border;
    int buttons = mbd->numButtons * bw + (n-1) * btn_spacing;
    int h = bh;
    if ( labelSize.height() )
        h += labelSize.height() + 3*border;
    else
        h += 2*border;
    int lmargin = 0;
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )  {
        mbd->iconLabel.adjustSize();
        lmargin += mbd->iconLabel.width() + border;
        if ( h < mbd->iconLabel.height() + 3*border + bh )
            h = mbd->iconLabel.height() + 3*border + bh;
    }
    int w = QMAX( buttons, labelSize.width() + lmargin ) + 2*border;
    QRect screen = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( pos() ) );
    if ( w > screen.width() )
        w = screen.width();
    resize( w, h );
    setMinimumSize( size() );
}


/*!\reimp
*/
void QMessageBox::resizeEvent( QResizeEvent * )
{
    int i;
    int n  = mbd->numButtons;
    int bw = mbd->buttonSize.width();
    int bh = mbd->buttonSize.height();
    int border = bh / 2 - style().pixelMetric(QStyle::PM_ButtonDefaultIndicator);
    if ( border <= 0 )
        border = 10;
    int btn_spacing = 7;
    if ( style().styleHint(QStyle::SH_GUIStyle) == MotifStyle )
        btn_spacing = border;
    int lmargin = 0;
    mbd->iconLabel.adjustSize();
    mbd->iconLabel.move( border, border );
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )
        lmargin += mbd->iconLabel.width() + border;
    label->setGeometry( lmargin+border,
                        border,
                        width() - lmargin -2*border,
                        height() - 3*border - bh );
    int extra_space = (width() - bw*n - 2*border - (n-1)*btn_spacing);
    if ( style().styleHint(QStyle::SH_GUIStyle) == MotifStyle )
        for ( i=0; i<n; i++ )
            mbd->pb[i]->move( border + i*bw + i*btn_spacing + extra_space*(i+1)/(n+1),
                              height() - border - bh );
    else
        for ( i=0; i<n; i++ )
            mbd->pb[i]->move( border + i*bw + extra_space/2 + i*btn_spacing,
                              height() - border - bh );
}


/*!\reimp
*/
void QMessageBox::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape ) {
        if ( mbd->escButton >= 0 ) {
            QPushButton *pb = mbd->pb[mbd->escButton];
            pb->animateClick();
            e->accept();
            return;
        }
    }
    QDialog::keyPressEvent( e );
}

/*!\reimp
*/
void QMessageBox::showEvent( QShowEvent *e )
{
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::Alert );
#endif
    QDialog::showEvent( e );
}

/*****************************************************************************
  Static QMessageBox functions
 *****************************************************************************/

/*!\fn int QMessageBox::message( const QString &,const QString&,const QString&,QWidget*,const char * )
  \obsolete
  Opens a modal message box directly using the specified parameters.

  Please use information(), warning() or critical() instead.
*/

/*! \fn bool QMessageBox::query( const QString &,const QString&,const QString&,const QString&,QWidget *, const char * )
  \obsolete
  Queries the user using a modal message box with two buttons.
  Note that \a caption is not always shown, it depends on the window manager.

  Please use information(), warning() or critical() instead.
*/

/*!
  Opens an information message box with the caption \a caption and the
  text \a text. The dialog may have up to three buttons. Each of the
  buttons, \a button0, \a button1 and \a button2 may be set to
  one of the following values:

  \list
  \i QMessageBox::NoButton
  \i QMessageBox::Ok
  \i QMessageBox::Cancel
  \i QMessageBox::Yes
  \i QMessageBox::No
  \i QMessageBox::Abort
  \i QMessageBox::Retry
  \i QMessageBox::Ignore
  \endlist

  If you don't want all three buttons, set the last button, or last two
  buttons to QMessageBox::NoButton.

  Returns the index of the button that was clicked.

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  \sa warning(), critical()
*/

int QMessageBox::information( QWidget *parent,
                              const QString& caption, const QString& text,
                              int button0, int button1, int button2 )
{
    QMessageBox *mb = new QMessageBox( caption, text, Information,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_information" );
    Q_CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Opens a warning message box with the caption \a caption and the
  text \a text. The dialog may have up to three buttons. Each of the
  button parameters, \a button0, \a button1 and \a button2 may be set to
  one of the following values:

  \list
  \i QMessageBox::NoButton
  \i QMessageBox::Ok
  \i QMessageBox::Cancel
  \i QMessageBox::Yes
  \i QMessageBox::No
  \i QMessageBox::Abort
  \i QMessageBox::Retry
  \i QMessageBox::Ignore
  \endlist

  If you don't want all three buttons, set the last button, or last two
  buttons to QMessageBox::NoButton.

  Returns the index of the button that was clicked.

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  \sa information(), critical()
*/

int QMessageBox::warning( QWidget *parent,
                          const QString& caption, const QString& text,
                          int button0, int button1, int button2 )
{
    QMessageBox *mb = new QMessageBox( caption, text, Warning,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_warning" );
    Q_CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Opens a critical message box with the caption \a caption and the
  text \a text. The dialog may have up to three buttons. Each of the
  button parameters, \a button0, \a button1 and \a button2 may be set to
  one of the following values:

  \list
  \i QMessageBox::NoButton
  \i QMessageBox::Ok
  \i QMessageBox::Cancel
  \i QMessageBox::Yes
  \i QMessageBox::No
  \i QMessageBox::Abort
  \i QMessageBox::Retry
  \i QMessageBox::Ignore
  \endlist

  If you don't want all three buttons, set the last button, or last two
  buttons to QMessageBox::NoButton.

  Returns the index of the button that was clicked.

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  \sa information(), warning()
*/

int QMessageBox::critical( QWidget *parent,
                           const QString& caption, const QString& text,
                           int button0, int button1, int button2 )
{
    QMessageBox *mb = new QMessageBox( caption, text, Critical,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_critical" );
    Q_CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Displays a simple about box with caption \a caption and
  text \a text. The about box's parent is \a parent.

  about() looks for a suitable icon in four locations:
  \list 1
  \i It prefers \link QWidget::icon() parent->icon() \endlink
  if that exists.
  \i If not, it tries the top-level widget
  containing \a parent.
  \i If that fails, it tries the \link
  QApplication::mainWidget() main widget. \endlink
  \i As a last resort it uses the Information icon.
  \endlist

  The about box has a single button labelled OK.

  \sa QWidget::icon() QApplication::mainWidget()
*/

void QMessageBox::about( QWidget *parent, const QString &caption,
                         const QString& text )
{
    QMessageBox *mb = new QMessageBox( caption, text,
                                       Information,
                                       Ok + Default, 0, 0,
                                       parent, "qt_msgbox_simple_about_box" );
    Q_CHECK_PTR( mb );
    QPixmap i;
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( parent && parent->icon())
        i = *(parent->icon());
    if ( i.isNull() && parent &&
         parent->topLevelWidget()->icon() )
        i = *(parent->topLevelWidget()->icon());
    if ( i.isNull() && qApp && qApp->mainWidget() &&
         qApp->mainWidget()->icon() )
        i = *(qApp->mainWidget()->icon());
#endif
    if ( !i.isNull() )
        mb->setIconPixmap( i );
    mb->exec();
    delete mb;
}


/*! \reimp
*/

void QMessageBox::styleChanged( QStyle& )
{
    if ( mbd->icon != NoIcon ) {
        // Reload icon for new style
        setIcon( mbd->icon );
    }
}


static int textBox( QWidget *parent, QMessageBox::Icon severity,
                    const QString& caption, const QString& text,
                    const QString& button0Text,
                    const QString& button1Text,
                    const QString& button2Text,
                    int defaultButtonNumber,
                    int escapeButtonNumber )
{
    int b[3];
    b[0] = 1;
    b[1] = button1Text.isEmpty() ? 0 : 2;
    b[2] = button2Text.isEmpty() ? 0 : 3;

    int i;
    for( i=0; i<3; i++ ) {
        if ( b[i] && defaultButtonNumber == i )
            b[i] += QMessageBox::Default;
        if ( b[i] && escapeButtonNumber == i )
            b[i] += QMessageBox::Escape;
    }

    QMessageBox *mb = new QMessageBox( caption, text, severity,
                                       b[0], b[1], b[2],
                                       parent, "qt_msgbox_information" );
    Q_CHECK_PTR( mb );
    if ( button0Text.isEmpty() )
        mb->setButtonText( 1, QMessageBox::tr(mb_texts[QMessageBox::Ok]) );
    else
        mb->setButtonText( 1, button0Text );
    if ( b[1] )
        mb->setButtonText( 2, button1Text );
    if ( b[2] )
        mb->setButtonText( 3, button2Text );

#ifndef QT_NO_CURSOR
    mb->setCursor( Qt::arrowCursor );
#endif
    int reply = mb->exec();

    delete mb;
    return reply-1;
}


/*!
    \overload
  Displays an information message box with caption \a caption, text \a
  text and one, two or three buttons. Returns the index of the button
  that was clicked (0, 1 or 2).

  \a button0Text is the text of the first button and is optional.  If
  \a button0Text is not supplied, "OK" (translated) will be used.
  \a button1Text is the text of the second button and is optional.
  \a button2Text is the text of the third button and is optional.  \a
  defaultButtonNumber (0..2) is the index of the default button;
  pressing Return or Enter is the same as clicking the default button.
  It defaults to 0 (the first button).  \a escapeButtonNumber is the
  index of the Escape button; pressing Escape is the same as clicking
  this button.  It defaults to -1 (pressing Escape does nothing);
  supply 0, 1 or 2 to make pressing Escape equivalent to clicking
  the relevant button.

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  \sa warning(), critical()
*/

int QMessageBox::information( QWidget *parent, const QString &caption,
                              const QString& text,
                              const QString& button0Text,
                              const QString& button1Text,
                              const QString& button2Text,
                              int defaultButtonNumber,
                              int escapeButtonNumber )
{
    return textBox( parent, Information, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}


/*!
    \overload
  Displays a warning message box with a caption, a text, and
  1..3 buttons.  Returns the number of the button that was clicked
  (0, 1, or 2).

  \a button0Text is the text of the first button and is optional.  If
  \a button0Text is not supplied, "OK" (translated) will be used.
  \a button1Text is the text of the second button and is optional, and
  \a button2Text is the text of the third button and is optional.  \a
  defaultButtonNumber (0..2) is the index of the default button;
  pressing Return or Enter is the same as clicking the default button.
  It defaults to 0 (the first button).  \a escapeButtonNumber is the
  index of the Escape button; pressing Escape is the same as clicking
  this button.  It defaults to -1 (pressing Escape does nothing);
  supply 0, 1, or 2 to make pressing Escape equivalent to clicking
  the relevant button.

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  \sa information(), critical()
*/

int QMessageBox::warning( QWidget *parent, const QString &caption,
                                 const QString& text,
                                 const QString& button0Text,
                                 const QString& button1Text,
                                 const QString& button2Text,
                                 int defaultButtonNumber,
                                 int escapeButtonNumber )
{
    return textBox( parent, Warning, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}


/*!
    \overload
  Displays a critical error message box with a caption, a text, and
  1..3 buttons.  Returns the number of the button that was clicked
  (0, 1 or 2).

  \a button0Text is the text of the first button and is optional.  If
  \a button0Text is not supplied, "OK" (translated) will be used.
  \a button1Text is the text of the second button and is optional, and
  \a button2Text is the text of the third button and is optional.  \a
  defaultButtonNumber (0..2) is the index of the default button;
  pressing Return or Enter is the same as clicking the default button.
  It defaults to 0 (the first button).  \a escapeButtonNumber is the
  index of the Escape button; pressing Escape is the same as clicking
  this button.  It defaults to -1 (pressing Escape does nothing);
  supply 0, 1, or 2 to make pressing Escape equivalent to clicking
  the relevant button.

  If \a parent is 0, the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \a parent.

  \sa information() warning()
*/

int QMessageBox::critical( QWidget *parent, const QString &caption,
                                  const QString& text,
                                  const QString& button0Text,
                                  const QString& button1Text,
                                  const QString& button2Text,
                                  int defaultButtonNumber,
                                  int escapeButtonNumber )
{
    return textBox( parent, Critical, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}


/*!
  Displays a simple message box about Qt, with caption \a
  caption and optionally centered over \a parent.  The message includes
  the version number of Qt being used by the application.

  This is useful for inclusion in the Help menu.  See the
  examples/menu/menu.cpp example.
*/

void QMessageBox::aboutQt( QWidget *parent, const QString &caption )
{
    QString c = caption;
    if ( c.isNull() )
        c = "About Qt";
    QMessageBox *mb = new QMessageBox( parent, "qt_msgbox_about_qt" );
#ifndef QT_NO_WIDGET_TOPEXTRA
    mb->setCaption( caption.isNull()?QString::fromLatin1("About Qt"):caption );
#endif
    mb->setText( *translatedTextAboutQt );
    QPixmap pm;
    QImage logo( (const char **)qtlogo_xpm);
    if ( qGray(mb->palette().active().text().rgb()) >
         qGray(mb->palette().active().base().rgb()) )
    {
        // light on dark, adjust some colors
        logo.setColor( 0,0xFFffffff);
        logo.setColor( 1,0xFF666666);
        logo.setColor( 2,0xFFcccc66);
        logo.setColor( 4,0xFFcccccc);
        logo.setColor( 6,0xFFffff66);
        logo.setColor( 7,0xFF999999);
        logo.setColor( 8,0xFF3333FF);
        logo.setColor( 9,0xFFffff33);
        logo.setColor(11,0xFFcccc99);
    }
    if ( pm.convertFromImage( logo ) )
        mb->setIconPixmap( pm );
    mb->setButtonText( 0, tr("OK") );
    if ( mb->mbd && mb->mbd->pb[0] ) {
        mb->mbd->pb[0]->setAutoDefault( TRUE );
        mb->mbd->pb[0]->setFocusPolicy( QWidget::StrongFocus );
        mb->mbd->pb[0]->setDefault( TRUE );
        mb->mbd->pb[0]->setFocus();
    }
    mb->exec();
}

/*!
  \reimp
*/

void QMessageBox::setIcon( const QPixmap &pix )
{
    //reimplemented to avoid compiler warning.
#ifndef QT_NO_WIDGET_TOPEXTRA
    QDialog::setIcon( pix );
#endif
}


/*!
  \property QMessageBox::textFormat
  \brief the format of the text displayed by the message box

  The current text format used by the message box.
  See the \l Qt::TextFormat enum for an explanation of the possible options.

  The default format is \c AutoText.

  \sa setText()
*/

Qt::TextFormat QMessageBox::textFormat() const
{
    return label->textFormat();
}


void QMessageBox::setTextFormat( Qt::TextFormat format )
{
    label->setTextFormat( format );
}

// My own personal favorite minimalist error message popped up whilst
// testing Freehand 8 last month.  I took a screen shot.  I believe I
// was trying to convert a file from one format to another.
// Apparently, I...
//
// http://www.people.cornell.edu/pages/mlj8/cant.gif

#endif
