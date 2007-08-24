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
    case ERROR_PIPE_NOT_CONNECTED:
	// ### handle
    default:
	error = QLocalSocket::UnknownSocketError;
	errorString = QLocalSocket::tr("%1: unknown error %2").arg(function).arg(windowsError);
#if defined QLOCALSOCKET_DEBUG
	qDebug() << errorString;
#endif
        state = QLocalSocket::UnconnectedState;
	q->emit stateChanged(state);
    }
}

QLocalSocketPrivate::QLocalSocketPrivate() : QIODevicePrivate(),
       error(QLocalSocket::UnknownSocketError),
       handle(INVALID_HANDLE_VALUE),
       pipeWriter(0),
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

    if (name.isEmpty()) {
	d->error = QLocalSocket::NotFoundError;
	setErrorString("Not Found Error");
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
    if (!d->pipeWriter) {
        d->pipeWriter = new QWindowsPipeWriter(d->handle, this);
	d->pipeWriter->start();
    }
    return d->pipeWriter->write(data, maxSize);
}

void QLocalSocket::abort()
{
}

qint64 QLocalSocket::bytesAvailable() const
{
    Q_D(const QLocalSocket);
    if (state() != ConnectedState)
        return 0;
    DWORD bytes;
    if (PeekNamedPipe(d->handle, NULL, 0, NULL, &bytes, NULL))
        return bytes;
    return 0;
}

qint64 QLocalSocket::bytesToWrite() const
{
    Q_D(const QLocalSocket);
    qint64 size = 0;
#ifdef Q_OS_WIN
    if (d->pipeWriter)
        size += d->pipeWriter->bytesToWrite();
#endif
    return size;
}

bool QLocalSocket::canReadLine() const
{
    if (state() != ConnectedState)
        return false;
    Q_D(const QLocalSocket);
    char line[100];
    DWORD lpBytesRead;
    DWORD lpTotalBytesAvail;
    DWORD lpBytesLeftThisMessage;
    if (PeekNamedPipe(d->handle, &line, 100, &lpBytesRead,
			    &lpTotalBytesAvail,
			    &lpBytesLeftThisMessage)) {
	if (lpTotalBytesAvail > 0)
		return true;
    }
    return false;
    //d->file.canReadLine();
    // use PeekNamedPipe();
}

void QLocalSocket::close()
{
    Q_D(QLocalSocket);
    //if (isValid())
    //	FlushFileBuffers(d->handle);

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
    //if (d->pipeWriter && d->pipeWriter->bytesToWrite() > 0) {
    //    d->pipeWriter->waitForWrite(ULONG_MAX);
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
    d->state = socketState;
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
    DWORD lpFlags = 0;
    DWORD lpOutBufferSize = 0;
    DWORD lpInBufferSize = 0;
    DWORD lpMaxInstances = 0;
    if (0 != GetNamedPipeInfo(d->handle, &lpFlags,
		    &lpOutBufferSize, &lpInBufferSize, &lpMaxInstances)) {
        return lpInBufferSize;
    }
    return 0;
}

void QLocalSocket::setReadBufferSize(qint64 size)
{
    Q_UNUSED(size);
}

bool QLocalSocket::waitForConnected(int msecs)
{
    Q_UNUSED(msecs);
    return state() == ConnectedState;
}

bool QLocalSocket::waitForDisconnected(int msecs)
{
    Q_UNUSED(msecs);
    if (state() == UnconnectedState)
        return false;
    return state() == UnconnectedState;
}

bool QLocalSocket::isValid() const
{
    Q_D(const QLocalSocket);
    return d->handle != INVALID_HANDLE_VALUE;
}

bool QLocalSocket::waitForReadyRead(int msecs)
{
    QIncrementalSleepTimer timer(msecs);
    forever {
        if (bytesAvailable() != 0)
            return true;

        Sleep(timer.nextSleepTime());
        if (timer.hasTimedOut())
            break;
    }

    // ### set error
    return false;
}

bool QLocalSocket::waitForBytesWritten(int msecs)
{
    Q_D(const QLocalSocket);
    if (!d->pipeWriter)
        return false;

    QIncrementalSleepTimer timer(msecs);
    forever {
        if (d->pipeWriter->hadWritten())
            return true;

        if (d->pipeWriter->bytesToWrite() == 0)
            return false;

        // Wait for the pipe writer to acknowledge that it has
        // written. This will succeed if either the pipe writer has
        // already written the data, or if it manages to write data
        // within the given timeout.
        if (d->pipeWriter->waitForWrite(0))
            return true;

        Sleep(timer.nextSleepTime());
        if (timer.hasTimedOut())
            break;
    }

    // ### timeout error
    return false;
}

