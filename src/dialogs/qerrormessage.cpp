/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qmessagebox.cpp#106 $
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

This is basically a QLabel and a "show this message again" checkbox and a
memory of what not to show.

There are two ways to use this class: In production applications, it can
productively be used to display messages such that the user doesn't need
to look at all of them all the time. To use QErrorMessage like this, you
create the dialog in the usual way and call the message() slot, or
connect signals to it.

The other way is intended for developers: The static qtHandler() installs
a message handler using qInstallMsgHandler() and creates a QErrorMessage
that displays the qDebug()/qWarning()/qFatal() messages.

In both cases will QErrorMessage queue up pending messages, and display
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


/*!  Constructs and installs an error handler window. */

QErrorMessage::QErrorMessage( QWidget * parent, const char * name )
    : QDialog( parent, name )
{
    QGridLayout * grid = new QGridLayout( this, 3, 2, 7 );
    icon = new QLabel( this );
    icon->setPixmap( QMessageBox::standardIcon( QMessageBox::Information,
						style() ) );
    grid->addWidget( icon, 0, 0, AlignTop );
    errors = new QTextView( this, "errors" );
    grid->addWidget( errors, 0, 1, AlignTop );
    again = new QCheckBox( tr( "&Show this message again" ), this, "again" );
    grid->addWidget( again, 1, 1, AlignTop + AlignAuto );
    ok = new QPushButton( tr( "&Ok" ), this, "ok" );
    connect( ok, SIGNAL(clicked()),
	     this, SLOT(accept()) );
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
    printf( "irgh %d", a );
    if ( again->isChecked() == FALSE )
	doNotShow->insert( errors->text(), (int*)42 );
    if ( nextPending() ) {
	printf( "urgh %d", a );
	return;
    } else {
	printf( "argh %d", a );
	QDialog::done( a );
    }
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
	printf( "e %d", pending->count() );
	pending->remove( pending->begin() );
	printf( "f %d", pending->count() );
	if ( !doNotShow->find( p ) ) {
	    errors->setText( p );
	    status = TRUE;
	    break;
	}
    }
    return status;
}


/*! Shows \a m and returns immediately.  If the user has requested
  that \a m not be shown, this function does nothing.
  
  Normally, \a m is shown at once, but if there are pending messages
  already \a m is queued for later display, not shown immediately.
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
