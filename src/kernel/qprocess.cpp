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
    : QObject( parent, name )
{
    d = new QProcessPrivate( this );
}

/*!
  Constructs a QProcess with the given command (but does not start it).
*/
QProcess::QProcess( const QString& com, QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QProcessPrivate( this );
    setCommand( com );
}

/*!
  Constructs a QProcess with the given command and arguments (but does not
  start it).
*/
QProcess::QProcess( const QString& com, const QStringList& args, QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QProcessPrivate( this );
    setCommand( com );
    setArguments( args );
}

/*!
  Destructor; if the process is running, it is NOT terminated! Stdin, stdout
  and stderr of the process are closed.
*/
QProcess::~QProcess()
{
    delete d;
}


/*!
  Sets the command that should be executed.
*/
void QProcess::setCommand( const QString& com )
{
    d->command = com;
}

/*!
  Sets the arguments for the command. Arguments that were previously set, will
  be deleted first.
*/
void QProcess::setArguments( const QStringList& args )
{
    d->arguments = args;
}

/*!
  Adds a argument to the end of the existing list of arguments.
*/
void QProcess::addArgument( const QString& arg )
{
    d->arguments.append( arg );
}

/*!
  Sets a working directory in which the command is executed.
*/
void QProcess::setWorkingDirectory( const QDir& dir )
{
    d->workingDir = dir;
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
	return d->exitNormal;
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
	return d->exitStat;
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
