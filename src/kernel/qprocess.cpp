/****************************************************************************
** $Id:$
**
** Implementation of QApplication class
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

#include "qapplication.h"
#include "qprocess.h"


//#define QPROCESS_DEBUG


/*!
  \class QProcess qprocess.h

  \brief The QProcess class is used to start external programs and control
  their behavior.

  \ingroup kernel

  You can start and finish an external program with this class. You can also
  write to stdin of the started program. You can read the output of the program
  on stdout and stderr. You get notified when the program exists.
*/

/*!
  Constructs a QProcess.
*/
QProcess::QProcess( QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteStdinConnected( FALSE )
{
    init();
}

/*!
  Constructs a QProcess with \a arg0 as the command to be executed.

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
  list are the arguments to this command.

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
  \fn void QProcess::dataStdout( const QByteArray& buf )

  When the process wrote data to stdout, this signal is emitted.
*/
/*!
  \fn void QProcess::dataStdout( const QString& buf )

  When the process wrote data to stdout, this signal is emitted.
*/
/*!
  \fn void QProcess::dataStderr( const QByteArray& buf )

  When the process wrote data to stderr, this signal is emitted.
*/
/*!
  \fn void QProcess::dataStderr( const QString& buf )

  When the process wrote data to stderr, this signal is emitted.
*/
/*!
  \fn void QProcess::processExited()

  When the process has exited, this signal is emitted.
*/
/*!
  \fn void QProcess::wroteStdin()

  This signal is emitted if the data send to stdin (via dataStdin()) was
  actually read by the process.
*/


/*!
  Writes data to the stdin of the process. The string is handled as a text. So
  what is written to stdin is the QString::latin1(). The process may or may not
  read this data. If the data was read, the signal wroteStdin() is emitted.
*/
void QProcess::dataStdin( const QString& buf )
{
    QByteArray bbuf;
    bbuf.duplicate( buf.latin1(), buf.length() );
    dataStdin( bbuf );
}


/*
 * Under Windows the implementation is not so nice: it is not that easy to
 * detect, when one of the signals should be emitted; therefore there are some
 * timers that query the information.
 * To keep it a little efficient, use the timers only when they are needed.
 * They are needed, if you are interested in the signals. So use
 * connectNotify() and disconnectNotify() to keep track of your interest.
 */
/*!  \reimpl
*/
void QProcess::connectNotify( const char * signal )
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::connectNotify(): signal %s has been connected", signal );
#endif
    if ( !ioRedirection )
	if ( qstrcmp( signal, SIGNAL(dataStdout(const QString&)) )==0 ||
		qstrcmp( signal, SIGNAL(dataStdout(const QByteArray&)) )==0 ||
		qstrcmp( signal, SIGNAL(dataStderr(const QString&)) )==0 ||
		qstrcmp( signal, SIGNAL(dataStderr(const QByteArray& buf)) )==0
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

/*!  \reimpl
*/
void QProcess::disconnectNotify( const char * )
{
    if ( ioRedirection &&
	    receivers( SIGNAL(dataStdout(const QString&)) ) ==0 &&
	    receivers( SIGNAL(dataStdout(const QByteArray&)) ) ==0 &&
	    receivers( SIGNAL(dataStderr(const QString&)) ) ==0 &&
	    receivers( SIGNAL(dataStderr(const QByteArray& buf)) ) ==0
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
