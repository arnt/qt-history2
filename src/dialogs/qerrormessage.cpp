/****************************************************************************
** $Id: $
**
** Implementation of a nice qInstallMsgHandler() handler
**
** Created : 2000-05-27, after Kalle Dalheimer's birthday
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

#include "qerrormessage.h"

#ifndef QT_NO_ERRORMESSAGE

#include "qstringlist.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qlabel.h"
#include "qtextview.h"

#include "qmessagebox.h"
#include "qstylesheet.h"
#include "qlayout.h"
#include "qdict.h"

#include <stdio.h>

/*! \class QErrorMessage qerrormessage.h

  \brief The QErrorMessage class provides an error message display dialog.

  \ingroup dialogs
  \ingroup misc

This is basically a QLabel and a "show this message again" checkbox which
remembers what not to show.

There are two ways to use this class:
\list 1
\i For production applications. In this context the class can be used to
display messages which you don't need the user to see more than once. To use
QErrorMessage like this, you create the dialog in the usual way and call the
message() slot, or connect signals to it.

\i For developers. In this context the static qtHandler() installs
a message handler using qInstallMsgHandler() and creates a QErrorMessage
that displays qDebug(), qWarning() and qFatal() messages.
\endlist

In both cases QErrorMessage will queue pending messages, and display
them (or not) in order, as soon as the user presses Enter or clicks OK
after seeing each message.

\sa QMessageBox QStatusBar::message()
*/

static QErrorMessage * qtMessageHandler = 0;


void jump( QtMsgType t, const char * m )
{
    if ( !qtMessageHandler )
	return;

    QString tmp;
    switch( t ) {
    case QtDebugMsg:
	tmp = "<p><b>Debug Message:</b></p>\n";
	break;
    case QtWarningMsg:
	tmp = "<p><b>Warning:</b></p>\n";
	break;
    case QtFatalMsg:
	tmp = "<p><b>Fatal Error:</b></p>\n";
	break;
    }
    tmp += QStyleSheet::convertFromPlainText( m );
    qtMessageHandler->message( m );
}


/*!  Constructs and installs an error handler window.
    The parent \a parent and name \a name are passed on to the QDialog
    constructor.
*/

QErrorMessage::QErrorMessage( QWidget * parent, const char * name )
    : QDialog( parent, name )
{
    QGridLayout * grid = new QGridLayout( this, 3, 2, 7 );
    icon = new QLabel( this, "qt_icon_lbl" );
#ifndef QT_NO_MESSAGEBOX
    icon->setPixmap( QMessageBox::standardIcon( QMessageBox::Information) );
#endif
    grid->addWidget( icon, 0, 0, AlignTop );
    errors = new QTextView( this, "errors" );
    grid->addWidget( errors, 0, 1, AlignTop );
    again = new QCheckBox( tr( "&Show this message again" ), this, "again" );
    grid->addWidget( again, 1, 1, AlignTop + AlignAuto );
    ok = new QPushButton( tr( "&OK" ), this, "ok" );
    connect( ok, SIGNAL(clicked()),
	     this, SLOT(accept()) );
    ok->setFocus();
    grid->addMultiCellWidget( ok, 2, 2, 0, 1, AlignCenter );
    grid->setColStretch( 1, 42 );
    grid->setRowStretch( 0, 42 );
    pending = new QStringList;
    doNotShow = new QDict<int>; // small, but speed is unimportant
    doNotShow->setAutoDelete( FALSE );
}


/*! Destroys the object and frees any allocated resources.  Notably,
the list of "do not show again" messages is deleted. */

QErrorMessage::~QErrorMessage()
{
    if ( this == qtMessageHandler ) {
	qtMessageHandler = 0;
	QtMsgHandler tmp = qInstallMsgHandler( 0 );
	// in case someone else has later stuck in another...
	if ( tmp != jump )
	    qInstallMsgHandler( tmp );
    }

    delete pending;
    pending = 0;
    delete doNotShow;
    doNotShow = 0;
}


/*! \reimp */

void QErrorMessage::done( int a )
{
    if ( again->isChecked() == FALSE )
	doNotShow->insert( errors->text(), (int*)42 );
    if ( nextPending() )
	return;
    else
	QDialog::done( a );
}


/*!  Returns a pointer to a QErrorMessage object that outputs the
default Qt messages.  This function creates such an object, if there
isn't one already.
*/

QErrorMessage * QErrorMessage::qtHandler()
{
    if ( !qtMessageHandler ) {
	qtMessageHandler
	    = new QErrorMessage( 0, "automatic qInstallMsgHandler handler" );
	qInstallMsgHandler( jump );
    }
    return qtMessageHandler;
}


/*! \internal */

bool QErrorMessage::nextPending()
{
    bool status = FALSE;
    while ( ! pending->isEmpty() ) {
	QString p = *pending->begin();
	pending->remove( pending->begin() );
	if ( !doNotShow->find( p ) ) {
	    errors->setText( p );
	    status = TRUE;
	    break;
	}
    }
    return status;
}


/*! Shows message \a m and returns immediately.  If the user has requested
  that \a m not be shown, this function does nothing.

  Normally, \a m is shown at once, but if there are pending messages,
  \a m is queued for later display.
*/

void QErrorMessage::message( const QString & m )
{
    if ( doNotShow->find( m ) )
	return;
    pending->append( m );
    if ( isVisible() )
	return;
    if ( nextPending() )
	show();
}

#endif //QT_NO_ERRORMESSAGE
