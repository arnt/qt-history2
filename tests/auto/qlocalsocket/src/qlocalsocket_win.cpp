/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlocalsocket.h"
#include "qlocalsocket_p.h"
#include <qt_windows.h>

#define BUFSIZE 512
#include <qdebug.h>
#include <qdatetime.h>

void QLocalSocketPrivate::setErrorString(const QString &function)
{
    Q_Q(QLocalSocket);
    BOOL windowsError = GetLastError();
    switch (windowsError) {
    case ERROR_BROKEN_PIPE:
    case ERROR_NO_DATA:
	error = QLocalSocket::ConnectionError;
	errorString = QLocalSocket::tr("%1: connection error").arg(function);
        state = QLocalSocket::UnconnectedState;
	q->emit stateChanged(state);
	q->emit disconnected();
	break;
    case ERROR_FILE_NOT_FOUND:
	error = QLocalSocket::NotFoundError;
	errorString = QLocalSocket::tr("%1: not found").arg(function);
        state = QLocalSocket::UnconnectedState;
	q->emit stateChanged(state);
	break;
    default:
	error = QLocalSocket::UnknownSocketError;
	errorString = QLocalSocket::tr("%1: unknown error %2").arg(function).arg(windowsError);
#if defined QLOCALSOCKET_DEBUG
	qDebug() << errorString;
#endif
    }
}

QLocalSocketPrivate::QLocalSocketPrivate() : QIODevicePrivate(),
       error(QLocalSocket::UnknownSocketError),
       state(QLocalSocket::UnconnectedState)
{
}

void QLocalSocket::connectToName(const QString &name, OpenMode openMode)
{
    Q_D(QLocalSocket);
    if ((state() == ConnectedState || state() == ConnectingState))
        return;

    d->state = ConnectingState;
    emit stateChanged(d->state);

    QString fullName = QString("\\\\.\\pipe\\%1").arg(name);

    //qDebug() << "socket connectToName:" << fullName;
    if (name.isEmpty()) {
	d->error = QLocalSocket::NotFoundError;
	setErrorString("Not Found ERror");
	d->state = UnconnectedState;
	emit stateChanged(d->state);
	return;
    }

    // Try to open a named pipe; wait for it, if necessary.
    HANDLE localSocket;
    while (true) {
        //QT_WA({CreateFileW()},{CreateFileA()});
        // ### permisions
	localSocket = CreateFileW(
                          (TCHAR*)fullName.utf16(),   // pipe name
                          GENERIC_READ |  // read and write access
                          GENERIC_WRITE,
                          0,              // no sharing
                          NULL,           // default security attributes
                          OPEN_EXISTING,  // opens existing pipe
                          0,              // default attributes
                          NULL);          // no template file

	// Break if the pipe handle is valid.
        if (localSocket != INVALID_HANDLE_VALUE)
            break;

	DWORD error = GetLastError();
        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (error != ERROR_PIPE_BUSY) {
	    d->setErrorString(QLatin1String("QLocalSocket::connectToName"));
	    return;
        }

        // All pipe instances are busy, so wait until connected or up to 5 seconds.
        WaitNamedPipe((TCHAR*)fullName.utf16(), 500);
    }

    d->peerName = name;
    if (setSocketDescriptor((int)localSocket), openMode) {
        d->handle = localSocket;
	emit connected();
        qDebug() << "socket: I am connected and setup!" << (int)localSocket << GetLastError();
    }
}

qint64 QLocalSocket::readData(char *data, qint64 maxSize)
{
    Q_D(QLocalSocket);
    DWORD bytesRead = 0;
    if (!ReadFile(d->handle, data, maxSize, &bytesRead, NULL)) {
	d->setErrorString(QLatin1String("QLocalSocket::readData"));
        return 0;
    }
    return bytesRead;
}

qint64 QLocalSocket::writeData(const char *data, qint64 maxSize)
{
    Q_D(QLocalSocket);
    DWORD bytesWritten = 0;
    if (!WriteFile(d->handle, data, maxSize, &bytesWritten, NULL)
	&& maxSize != bytesWritten) {
	d->setErrorString(QLatin1String("QLocalSocket::writeData"));
	return 0;
    }
    return bytesWritten;
}

void QLocalSocket::abort()
{
}

qint64 QLocalSocket::bytesAvailable() const
{
    //Q_D(const QLocalSocket);
    return 0;//d->file.bytesAvailable();
}

qint64 QLocalSocket::bytesToWrite() const
{
    //Q_D(const QLocalSocket);
    return 0;//d->file.bytesToWrite();
}

bool QLocalSocket::canReadLine() const
{
    //Q_D(const QLocalSocket);
    return (state() == ConnectedState);//d->file.canReadLine();
}

void QLocalSocket::close()
{
    Q_D(QLocalSocket);
    if (state() == UnconnectedState)
	return;
    d->state = ClosingState;
    emit stateChanged(d->state);
    DisconnectNamedPipe(d->handle);
    CloseHandle(d->handle);
    d->handle = INVALID_HANDLE_VALUE;
    d->state = UnconnectedState;
    emit stateChanged(d->state);
    emit disconnected();
}

bool QLocalSocket::flush()
{
    Q_D(QLocalSocket);
    if (!isValid())
        return false;
    if (0 == FlushFileBuffers(d->handle))
        return true;
    qDebug() << "flush failed" << GetLastError();
    return false;
}

void QLocalSocket::disconnectFromName()
{
    flush();
    close();
}

QLocalSocket::LocalSocketError QLocalSocket::error() const
{
    Q_D(const QLocalSocket);
    return d->error;
}

bool QLocalSocket::setSocketDescriptor(int socketDescriptor,
		LocalSocketState socketState, OpenMode openMode)
{
    Q_D(QLocalSocket);
    setOpenMode(openMode);
    d->handle = (int*)socketDescriptor;
    d->state = ConnectedState;
    emit stateChanged(d->state);
    d->handleNotifier.setHandle(d->handle);
    return true;
}

void QLocalSocketPrivate::_q_activated(HANDLE hEvent)
{
    qDebug() << "socket: I got an event? why?" << hEvent;
}

int QLocalSocket::socketDescriptor() const
{
    Q_D(const QLocalSocket);
    return (int)d->handle;
}

void QLocalSocketPrivate::init()
{
    Q_Q(QLocalSocket);
    handle = INVALID_HANDLE_VALUE;
    QObject::connect(&handleNotifier, SIGNAL(activated(HANDLE)),
		     q, SLOT(_q_activated(HANDLE)));
}

qint64 QLocalSocket::readBufferSize() const
{
    Q_D(const QLocalSocket);
    LPDWORD lpFlags;
    LPDWORD lpOutBufferSize;
    LPDWORD lpInBufferSize;
    LPDWORD lpMaxInstances;
    if (0 != GetNamedPipeInfo(d->handle, lpFlags,
		    lpOutBufferSize, lpInBufferSize, lpMaxInstances)) {
        return *lpInBufferSize;
    }
    return 0;
}

void QLocalSocket::setReadBufferSize(qint64 size)
{
}

bool QLocalSocket::waitForConnected(int msecs)
{
    // ### TODO

    // For the time being while developing the QLocalSocket class ...
    msecs = 1000;

    QTime stopWatch;
    stopWatch.start();

    while (state() == ConnectingState) {
        Sleep(10);
    }

    return state() == ConnectedState;
}

bool QLocalSocket::waitForDisconnected(int msecs)
{
    if (state() == UnconnectedState)
        return false;
    // ### would this do anything on windows?
    return (state() == UnconnectedState);
}

bool QLocalSocket::isValid() const
{
    Q_D(const QLocalSocket);
    return d->handle != INVALID_HANDLE_VALUE;
}

bool QLocalSocket::waitForReadyRead(int msecs)
{
    // ### TODO
    return (state() == ConnectedState);//flush();
}

bool QLocalSocket::waitForBytesWritten(int msecs)
{
    // ### TODO
    return (state() == ConnectedState);//flush();
}

