/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprocess.cpp#19 $
**
** Implementation of QProcess class
**
** Created : 20000905
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include <stdio.h>
#include <stdlib.h>

#include "qprocess.h"

#ifndef QT_NO_PROCESS

#include "qapplication.h"


//#define QPROCESS_DEBUG


/*!
  \class QProcess qprocess.h

  \brief The QProcess class is used to start external programs and control
  their behavior.

  \ingroup misc

  You can start and finish an external program with this class. You can also
  write to stdin of the started program. You can read the output of the program
  on stdout and stderr. You get notified when the program exits.

  There are two different ways to run a process: If you use start(), you have
  full control over the process; you can write to the stdin via the
  writeToStdin() slots whenever you want, and you can close stdin via the
  closeStdin() slot.

  If you know the data that should be written to the stdin of the process
  already when you want to run the process, you can use the launch() functions
  instead. These functions take the data that should be written to stdin as an
  argument, write it to stdin and automatically close stdin if all data was
  written.

  If you use a launch() function to run the process, you should not use the
  slots writeToStdin() and closeStdin().
*/

/*!
  Constructs a QProcess object. The parameters \a parent and \a name are passed
  to the QObject constructor.
*/
QProcess::QProcess( QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteStdinConnected( FALSE )
{
    init();
}

/*!
  Constructs a QProcess with \a arg0 as the command to be executed. The
  parameters \a parent and \a name are passed to the QObject constructor.

  The process is not started. You have to call start() explicitly to start the
  process.
*/
QProcess::QProcess( const QString& arg0, QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteStdinConnected( FALSE )
{
    init();
    addArgument( arg0 );
}

/*!
  Constructs a QProcess with \a args as the arguments of the process. The first
  element in the list is the command to be executed. The other elements in the
  list are the arguments to this command. The parameters \a parent and \a name
  are passed to the QObject constructor.

  The process is not started. You have to call start() explicitly to start the
  process.
*/
QProcess::QProcess( const QStringList& args, QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteStdinConnected( FALSE )
{
    init();
    setArguments( args );
}


/*!
  Sets \a args as the arguments for the process. The first element in the list
  is the command to be executed. The other elements in the list are the
  arguments to this command.

  Arguments that were previously set, will be deleted first.
*/
void QProcess::setArguments( const QStringList& args )
{
    arguments = args;
}

/*!
  Adds \a arg to the end of the existing list of arguments.

  The first element in the list of arguments is the command to be executed; the
  following elements are the arguments to this command.
*/
void QProcess::addArgument( const QString& arg )
{
    arguments.append( arg );
}

/*!
  Sets \a dir as the working directory in which the command is executed.
*/
void QProcess::setWorkingDirectory( const QDir& dir )
{
    workingDir = dir;
}


/*!
  Returns TRUE if the process has exited normally, otherwise FALSE.
*/
bool QProcess::normalExit()
{
    // isRunning() has the side effect that it determines the exit status!
    if ( isRunning() )
	return FALSE;
    else
	return exitNormal;
}

/*!
  Returns the exit status of the process. This value is only valid if
  normalExit() is TRUE.
*/
int QProcess::exitStatus()
{
    // isRunning() has the side effect that it determines the exit status!
    if ( isRunning() )
	return 0;
    else
	return exitStat;
}


/*!
  Reads the data that the process has written to stdout. When new data was
  written to stdout, the class emits the signal readyReadStdout().

  \sa readyReadStdout() readStderr()
*/
QByteArray QProcess::readStdout()
{
    QByteArray buf = bufStdout;
    bufStdout.resize( 0 );
    return buf;
}

/*!
  Reads the data that the process has written to stderr. When new data was
  written to stderr, the class emits the signal readyReadStderr().

  \sa readyReadStderr() readStdout()
*/
QByteArray QProcess::readStderr()
{
    QByteArray buf = bufStderr;
    bufStderr.resize( 0 );
    return buf;
}

/*!
  Runs the process and writes the data \a buf to stdin of the process. If all
  data is written to stdin, it closes stdin.

  Returns TRUE on success, otherwise FALSE.

  Notice that you should not use the slots writeToStdin() and closeStdin() on
  processes started with launch(). If you need these slots, use start()
  instead.

  The data \a buf is written to stdin with writeToStdin(); so the data that is
  actually written is the QString::local8Bit() representation.

  The process may or may not read this data. If the data was read, the signal
  wroteStdin() is emitted.

  \sa start() writeToStdin()
*/
bool QProcess::launch( const QString& buf )
{
    if ( start() ) {
	connect( this, SIGNAL(wroteStdin()),
		this, SLOT(closeStdinLaunch()) );
	writeToStdin( buf );
	return TRUE;
    } else {
	return FALSE;
    }
}

/*! \overload
*/
bool QProcess::launch( const QByteArray& buf )
{
    if ( start() ) {
	connect( this, SIGNAL(wroteStdin()),
		this, SLOT(closeStdinLaunch()) );
	writeToStdin( buf );
	return TRUE;
    } else {
	return FALSE;
    }
}

/*!
  This slot is used by the launch() functions to close stdin.
*/
void QProcess::closeStdinLaunch()
{
    disconnect( this, SIGNAL(wroteStdin()),
	    this, SLOT(closeStdinLaunch()) );
    closeStdin();
}


/*!
  \fn void QProcess::readyReadStdout()

  When the process wrote data to stdout, this signal is emitted. You can read
  the data with readStdout().

  \sa readStdout() readyReadStderr()
*/
/*!
  \fn void QProcess::readyReadStderr()

  When the process wrote data to stderr, this signal is emitted. You can read
  the data with readStderr().

  \sa readStderr() readyReadStdout()
*/
/*!
  \fn void QProcess::processExited()

  When the process has exited, this signal is emitted.
*/
/*!
  \fn void QProcess::wroteStdin()

  This signal is emitted if the data send to stdin (via writeToStdin()) was
  actually read by the process.
*/


/*! \overload
  The string \a buf is handled as a text. So what is written to stdin is the
  QString::local8Bit() representation.

  \sa wroteStdin()
*/
void QProcess::writeToStdin( const QString& buf )
{
    writeToStdin( buf.local8Bit() );
}


/*
 * Under Windows the implementation is not so nice: it is not that easy to
 * detect, when one of the signals should be emitted; therefore there are some
 * timers that query the information.
 * To keep it a little efficient, use the timers only when they are needed.
 * They are needed, if you are interested in the signals. So use
 * connectNotify() and disconnectNotify() to keep track of your interest.
 */
/*!  \reimp
*/
void QProcess::connectNotify( const char * signal )
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::connectNotify(): signal %s has been connected", signal );
#endif
    if ( !ioRedirection )
	if ( qstrcmp( signal, SIGNAL(readyReadStdout()) )==0 ||
		qstrcmp( signal, SIGNAL(readyReadStderr()) )==0
	   ) {
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcess::connectNotify(): set ioRedirection to TRUE" );
#endif
	    setIoRedirection( TRUE );
	    return;
	}
    if ( !notifyOnExit && qstrcmp( signal, SIGNAL(processExited()) )==0 ) {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::connectNotify(): set notifyOnExit to TRUE" );
#endif
	setNotifyOnExit( TRUE );
	return;
    }
    if ( !wroteStdinConnected && qstrcmp( signal, SIGNAL(wroteStdin()) )==0 ) {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::connectNotify(): set wroteStdinConnected to TRUE" );
#endif
	setWroteStdinConnected( TRUE );
	return;
    }
}

/*!  \reimp
*/
void QProcess::disconnectNotify( const char * )
{
    if ( ioRedirection &&
	    receivers( SIGNAL(readyReadStdout()) ) ==0 &&
	    receivers( SIGNAL(readyReadStderr()) ) ==0
	    ) {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::disconnectNotify(): set ioRedirection to FALSE" );
#endif
	setIoRedirection( FALSE );
    }
    if ( notifyOnExit && receivers( SIGNAL(processExited()) ) == 0 ) {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::disconnectNotify(): set notifyOnExit to FALSE" );
#endif
	setNotifyOnExit( FALSE );
    }
    if ( wroteStdinConnected && receivers( SIGNAL(wroteStdin()) ) == 0 ) {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::disconnectNotify(): set wroteStdinConnected to FALSE" );
#endif
	setWroteStdinConnected( FALSE );
    }
}

#endif // QT_NO_PROCESS
