/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprocess.h#23 $
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

#ifndef QPROCESS_H
#define QPROCESS_H

#ifndef QT_H
#include "qobject.h"
#include "qstringlist.h"
#include "qdir.h"
#endif // QT_H

#ifndef QT_NO_PROCESS

class QProcessPrivate;


class Q_EXPORT QProcess : public QObject
{
    Q_OBJECT
public:
    QProcess( QObject *parent=0, const char *name=0 );
    QProcess( const QString& arg0, QObject *parent=0, const char *name=0 );
    QProcess( const QStringList& args, QObject *parent=0, const char *name=0 );
    ~QProcess();

    // set the arguments and working directory
    virtual void setArguments( const QStringList& args );
    virtual void addArgument( const QString& arg );
    virtual void setWorkingDirectory( const QDir& dir );

    // control the execution
    virtual bool start();
    virtual bool launch( const QString& buf );
    virtual bool launch( const QByteArray& buf );
    void hangUp() const;
    void kill() const;

    // inquire the status
    bool isRunning() const;
    bool normalExit() const;
    int exitStatus() const;

    // reading
    virtual QByteArray readStdout();
    virtual QByteArray readStderr();

signals:
    void readyReadStdout();
    void readyReadStderr();
    void processExited();
    void wroteToStdin();

public slots:
    // input
    virtual void writeToStdin( const QByteArray& buf );
    virtual void writeToStdin( const QString& buf );
    virtual void closeStdin();

protected: // ### or private?
    void connectNotify( const char * signal );
    void disconnectNotify( const char * signal );
private:
    void setIoRedirection( bool value );
    void setNotifyOnExit( bool value );
    void setWroteStdinConnected( bool value );

    void init();
    void reset();
#if defined(Q_WS_WIN)
    uint readStddev( HANDLE dev, char *buf, uint bytes );
#endif

private slots:
    void socketRead( int fd );
    void socketWrite( int fd );
    void timeout();
    void closeStdinLaunch();

private:
    QProcessPrivate *d;

    QDir        workingDir;
    QStringList arguments;

    QByteArray bufStdout;
    QByteArray bufStderr;
    int  exitStat; // exit status
    bool exitNormal; // normal exit?
    bool ioRedirection; // automatically set be (dis)connectNotify
    bool notifyOnExit; // automatically set be (dis)connectNotify
    bool wroteToStdinConnected; // automatically set be (dis)connectNotify

    friend class QProcessPrivate;
#if defined(Q_OS_UNIX)
    friend class QProcessManager;
    friend class QProc;
#endif
};

#endif // QT_NO_PROCESS

#endif // QPROCESS_H
