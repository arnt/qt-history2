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

#include <qdebug.h>

#define NOTIFYTIMEOUT 100

void QLocalSocketPrivate::setErrorString(const QString &function)
{
    Q_Q(QLocalSocket);
    BOOL windowsError = GetLastError();
    QLocalSocket::LocalSocketState currentState = state;
    switch (windowsError) {
    case ERROR_PIPE_NOT_CONNECTED:
    case ERROR_BROKEN_PIPE:
    case ERROR_NO_DATA:
	error = QLocalSocket::ConnectionError;
	errorString = QLocalSocket::tr("%1: Connection error").arg(function);
        state = QLocalSocket::UnconnectedState;
	break;
    case ERROR_FILE_NOT_FOUND:
	error = QLocalSocket::NotFoundError;
	errorString = QLocalSocket::tr("%1: Invalid name").arg(function);
        state = QLocalSocket::UnconnectedState;
	break;
    default:
	error = QLocalSocket::UnknownSocketError;
	errorString = QLocalSocket::tr("%1: Unknown error %2").arg(function).arg(windowsError);
#if defined QLOCALSOCKET_DEBUG
	qWarning() << errorString;
#endif
        state = QLocalSocket::UnconnectedState;
    }

    if (currentState != state) {
        q->emit stateChanged(state);
        if (state == QLocalSocket::UnconnectedState)
            q->emit disconnected();
    }
}

QLocalSocketPrivate::QLocalSocketPrivate() : QIODevicePrivate(),
       error(QLocalSocket::UnknownSocketError),
       handle(INVALID_HANDLE_VALUE),
       pipeWriter(0),
       readyRead(false),
       state(QLocalSocket::UnconnectedState)
{
}

void QLocalSocket::connectToName(const QString &name, OpenMode openMode)
{
    Q_D(QLocalSocket);
    if (state() == ConnectedState || state() == ConnectingState)
        return;

    d->error = QLocalSocket::UnknownSocketError;
    d->errorString = QString();
    d->state = ConnectingState;
    emit stateChanged(d->state);

    if (name.isEmpty()) {
	d->error = QLocalSocket::NotFoundError;
	setErrorString("Not Found Error");
	d->state = UnconnectedState;
	emit stateChanged(d->state);
	return;
    }

    QString fullName = QString("\\\\.\\pipe\\%1").arg(name);
    // Try to open a named pipe; wait for it, if necessary.
    HANDLE localSocket;
    forever {
	DWORD permissions = (openMode & QIODevice::ReadOnly) ? GENERIC_READ : 0;
	permissions |= (openMode & QIODevice::WriteOnly) ? GENERIC_WRITE : 0;
	QT_WA({
	localSocket = CreateFileW(
                          (TCHAR*)fullName.utf16(),   // pipe name
                          permissions,
                          0,              // no sharing
                          NULL,           // default security attributes
                          OPEN_EXISTING,  // opens existing pipe
                          0,              // default attributes
                          NULL);          // no template file
	}, {
	localSocket = CreateFileA(
                          fullName.toLocal8Bit().constData(),   // pipe name
                          permissions,
                          0,              // no sharing
                          NULL,           // default security attributes
                          OPEN_EXISTING,  // opens existing pipe
                          0,              // default attributes
                          NULL);          // no template file
	
	});
        if (localSocket != INVALID_HANDLE_VALUE)
            break;

	DWORD error = GetLastError();
        // It is really and error if error is not ERROR_PIPE_BUSY
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
    d->readyRead = false;
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
    return (d->pipeWriter) ? d->pipeWriter->bytesToWrite() : 0;
}

bool QLocalSocket::canReadLine() const
{
    Q_D(const QLocalSocket);
    if (state() != ConnectedState)
        return false;
    char line[100];
    DWORD lpBytesRead = 0;
    DWORD lpTotalBytesAvail  = 0;
    DWORD lpBytesLeftThisMessage = true;
    while (lpBytesLeftThisMessage
           && PeekNamedPipe(d->handle, &line, 100, &lpBytesRead,
                            &lpTotalBytesAvail,
                            &lpBytesLeftThisMessage)) {
	for (uint i = 0; i < lpBytesRead; ++i) {
	    if (line[i] == '\n') {
            	if (!d->readyRead) {
  		    emit const_cast<QLocalSocket*>(this)->readyRead();
  		    const_cast<QLocalSocketPrivate*>(d)->readyRead = true;
		}
		return true;
	    }
	}
    }
    return QIODevice::canReadLine();
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
    if (d->pipeWriter) {
	d->pipeWriter->deleteLater();
	d->pipeWriter = 0;
    }
}

bool QLocalSocket::flush()
{
    Q_D(QLocalSocket);
    if (d->pipeWriter)
        return d->pipeWriter->waitForWrite(0);
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
    d->notifier.start(NOTIFYTIMEOUT);
    return true;
}

void QLocalSocketPrivate::_q_notified()
{
    Q_Q(QLocalSocket);
    if (q->bytesAvailable() != 0) {
        if (!readyRead) {
	    readyRead = true;
	    q->emit readyRead();
	}
    }

    if (q->isValid())
	notifier.start(NOTIFYTIMEOUT);
}

int QLocalSocket::socketDescriptor() const
{
    Q_D(const QLocalSocket);
    return (int)d->handle;
}

void QLocalSocketPrivate::init()
{
    Q_Q(QLocalSocket);
    QObject::connect(&notifier, SIGNAL(timeout()), q, SLOT(_q_notified()));
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
    return false;
}

bool QLocalSocket::isValid() const
{
    Q_D(const QLocalSocket);
    return d->handle != INVALID_HANDLE_VALUE;
}

bool QLocalSocket::waitForReadyRead(int msecs)
{
    Q_D(QLocalSocket);
    QIncrementalSleepTimer timer(msecs);
    forever {
        if (bytesAvailable() != 0) {
            if (!d->readyRead) {
	    	d->readyRead = true;
		emit readyRead();
	    }
	    return true;
	}

        Sleep(timer.nextSleepTime());
        if (timer.hasTimedOut())
            break;
    }

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

    return false;
}

