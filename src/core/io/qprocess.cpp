/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#define QPROCESS_DEBUG

#include "qprocess.h"
#include "qprocess_p.h"

#include <qbytearray.h>
#include <qdatetime.h>
#include <qcoreapplication.h>
#include <qsocketnotifier.h>
#include <qtimer.h>

/*! \class QProcess

    \brief The QProcess class is used to start external programs and
    to communicate with them.

    \ingroup io
    \ingroup misc
    \mainclass
*/


QProcessPrivate::QProcessPrivate()
{
    processChannel = QProcess::StandardOutput;
    processError = QProcess::UnknownError;
    processState = QProcess::NotRunning;
    pid = 0;
    exitCode = 0;
    standardReadSocketNotifier = 0;
    errorReadSocketNotifier = 0;
    writeSocketNotifier = 0;
    startupSocketNotifier = 0;
    notifier = 0;
    pipeWriter = 0;
    standardReadPipe[0] = INVALID_Q_PIPE;
    standardReadPipe[1] = INVALID_Q_PIPE;
    errorReadPipe[0] = INVALID_Q_PIPE;
    errorReadPipe[1] = INVALID_Q_PIPE;
    writePipe[0] = INVALID_Q_PIPE;
    writePipe[1] = INVALID_Q_PIPE;
    childStartedPipe[0] = INVALID_Q_PIPE;
    childStartedPipe[1] = INVALID_Q_PIPE;
    exitCode = 0;
    crashed = false;
#ifdef Q_WS_WIN
    pipeWriter = 0;
#endif // Q_WS_WIN
}

QProcessPrivate::~QProcessPrivate()
{
}

void QProcessPrivate::cleanup()
{
    Q_Q(QProcess);

    q->setOpenMode(QIODevice::NotOpen);
    processChannel = QProcess::StandardOutput;
    processError = QProcess::UnknownError;
    processState = QProcess::NotRunning;
#ifdef Q_OS_WIN
    if (pid) {
        delete pid;
    }
#endif
    pid = 0;
    // exitCode = 0; // We deliberately do not reset the exit code.
    crashed = false;
    if (standardReadSocketNotifier) {
        standardReadSocketNotifier->setEnabled(false);
        delete standardReadSocketNotifier;
        standardReadSocketNotifier = 0;
    }
    if (errorReadSocketNotifier) {
        errorReadSocketNotifier->setEnabled(false);
        delete errorReadSocketNotifier;
        errorReadSocketNotifier = 0;
    }
    if (writeSocketNotifier) {
        writeSocketNotifier->setEnabled(false);
        delete writeSocketNotifier;
        writeSocketNotifier = 0;
    }
    if (startupSocketNotifier) {
        startupSocketNotifier->setEnabled(false);
        delete startupSocketNotifier;
        startupSocketNotifier = 0;
    }
    if (notifier) {
        delete notifier;
        notifier = 0;
    }
    destroyPipe(standardReadPipe);
    destroyPipe(errorReadPipe);
    destroyPipe(writePipe);
    destroyPipe(childStartedPipe);
}

void QProcessPrivate::canReadStandardOutput()
{
    Q_Q(QProcess);
    Q_LONGLONG available = bytesAvailableFromStdout();
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardOutput(), %lld bytes available",
           available);
#endif

    if (available == 0)
        return;

    char *ptr = outputReadBuffer.reserve(available);
    Q_LONGLONG readBytes = readFromStdout(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error reading from process"));
        emit q->error(processError);
        return;
    }
    outputReadBuffer.truncate(available - readBytes);

    if (readBytes == 0) {
        if (standardReadSocketNotifier)
            standardReadSocketNotifier->setEnabled(false);
    } else if (processChannel == QProcess::StandardOutput) {
        emit q->readyRead();
    }
    emit q->readyReadStandardOutput();
}

void QProcessPrivate::canReadStandardError()
{
    Q_Q(QProcess);
    Q_LONGLONG available = bytesAvailableFromStderr();
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardError(), %lld bytes available",
           available);
#endif

    if (available == 0)
        return;

    char *ptr = errorReadBuffer.reserve(available);
    Q_LONGLONG readBytes = readFromStderr(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error reading from process"));
        emit q->error(processError);
        return;
    }
    errorReadBuffer.truncate(available - readBytes);

    if (readBytes == 0) {
        if (errorReadSocketNotifier)
            errorReadSocketNotifier->setEnabled(false);
    } else if (processChannel == QProcess::StandardError) {
        emit q->readyRead();
    }
    emit q->readyReadStandardError();
}

void QProcessPrivate::canWrite()
{
    Q_Q(QProcess);
    if (writeSocketNotifier)
        writeSocketNotifier->setEnabled(false);

    if (writeBuffer.isEmpty())
        return;

    Q_LONGLONG written = writeToStdin(writeBuffer.readPointer(),
                                      writeBuffer.nextDataBlockSize());
    if (written < 0) {
        processError = QProcess::WriteError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error writing to process"));
        emit q->error(processError);
        return;
    }

    writeBuffer.free(written);
    if (writeSocketNotifier)
        writeSocketNotifier->setEnabled(true);
}

void QProcessPrivate::processDied()
{
    Q_Q(QProcess);

    // in case there is data in the pipe line and this slot by chance
    // got called before the read notifications, call these two slots
    // so the data is made available before the process dies.
    canReadStandardOutput();
    canReadStandardError();

    processState = QProcess::Finishing;
    emit q->stateChanged(processState);
    emit q->finishing();

    if (crashed) {
        processError = QProcess::Crashed;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process crashed"));
        emit q->error(processError);
    }

    cleanup();

    processState = QProcess::NotRunning;
    emit q->stateChanged(processState);
    emit q->finished(exitCode);
}

void QProcessPrivate::startupNotification()
{
    Q_Q(QProcess);
    if (startupSocketNotifier)
        startupSocketNotifier->setEnabled(false);
    if (processStarted()) {
        processState = QProcess::Running;
        emit q->started();
    } else {
        processState = QProcess::NotRunning;
        processError = QProcess::FailedToStart;
        emit q->error(processError);
        cleanup();
    }
}

QProcess::QProcess(QObject *parent)
    : QIODevice(*new QProcessPrivate, parent)
{
}

QProcess::~QProcess()
{
    Q_D(QProcess);
    d->cleanup();
}

QProcess::ProcessChannel QProcess::inputChannel() const
{
    Q_D(const QProcess);
    return d->processChannel;
}

void QProcess::setInputChannel(ProcessChannel channel)
{
    Q_D(QProcess);
    d->processChannel = channel;
}

QString QProcess::workingDirectory() const
{
    Q_D(const QProcess);
    return d->workingDirectory;
}

void QProcess::setWorkingDirectory(const QString &path)
{
    Q_D(QProcess);
    d->workingDirectory = path;
}

Q_PID QProcess::pid() const
{
    Q_D(const QProcess);
    return d->pid;
}

bool QProcess::canReadLine() const
{
    Q_D(const QProcess);
    const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                    ? &d->errorReadBuffer
                                    : &d->outputReadBuffer;
    return readBuffer->canReadLine();
}

void QProcess::close()
{
}

bool QProcess::flush()
{
    Q_D(QProcess);

    while (!d->writeBuffer.isEmpty()) {
        if (!d->waitForWrite())
            return false;
        d->canWrite();
    }
    return true;
}

bool QProcess::isSequential() const
{
    return true;
}

Q_LONGLONG QProcess::bytesAvailable() const
{
    Q_D(const QProcess);
    const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                    ? &d->errorReadBuffer
                                    : &d->outputReadBuffer;
#if defined QPROCESS_DEBUG
    qDebug("QProcess::bytesAvailable() == %i (%s)", readBuffer->size(),
           (d->processChannel == QProcess::StandardError) ? "stderr" : "stdout");
#endif
    return readBuffer->size();
}

QProcess::ProcessError QProcess::processError() const
{
    Q_D(const QProcess);
    return d->processError;
}

QProcess::ProcessState QProcess::processState() const
{
    Q_D(const QProcess);
    return d->processState;
}

void QProcess::setEnvironment(const QStringList &environment)
{
    Q_D(QProcess);
    d->environment = environment;
}

QStringList QProcess::environment() const
{
    Q_D(const QProcess);
    return d->environment;
}

bool QProcess::waitForStarted(int msecs)
{
    Q_D(QProcess);
    if (!d->waitForStarted(msecs)) {
        emit error(d->processError);
        return false;
    }

    d->processState = QProcess::Running;
    emit started();
    return true;
}

bool QProcess::waitForReadyRead(int msecs)
{
    Q_D(QProcess);

    if (d->processState != QProcess::Running) {
        QTime stopWatch;
        stopWatch.start();
        bool started = waitForStarted(msecs);
        if (!started)
            return false;
        msecs -= stopWatch.elapsed();
    }

    if (!d->waitForReadyRead(msecs)) {
        emit error(d->processError);
        return false;
    }

    bool emitReadyRead = false;
    if (d->processChannel == QProcess::StandardOutput) {
        int size = d->outputReadBuffer.size();
        d->canReadStandardOutput();
        emitReadyRead = (size < d->outputReadBuffer.size());
    } else {
        int size = d->errorReadBuffer.size();
        d->canReadStandardError();
        emitReadyRead = (size < d->errorReadBuffer.size());
    }
    if (emitReadyRead) {
        emit readyRead();
        return true;
    }
    return false;
}

bool QProcess::waitForBytesWritten(int msecs)
{
    Q_D(QProcess);
    return d->waitForBytesWritten(msecs);
}

bool QProcess::waitForFinished(int msecs)
{
    Q_D(QProcess);
    if (d->processState == QProcess::NotRunning)
        return true;
    if (d->processState == QProcess::Starting) {
        QTime stopWatch;
        stopWatch.start();
        bool started = waitForStarted(msecs);
        if (!started)
            return false;
        msecs -= stopWatch.elapsed();
    }

    return d->waitForFinished(msecs);
}

Q_LONGLONG QProcess::readData(char *data, Q_LONGLONG maxlen)
{
    Q_D(QProcess);
    QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                              ? &d->errorReadBuffer
                              : &d->outputReadBuffer;

    if (maxlen == 1) {
        int c = readBuffer->getChar();
        if (c == -1)
            return -1;
        *data = (char) c;
        return 1;
    }

    int bytesToRead = qMin(readBuffer->size(), (int)maxlen);
    int readSoFar = 0;
    while (readSoFar < bytesToRead) {
        char *ptr = readBuffer->readPointer();
        int bytesToReadFromThisBlock = qMin(bytesToRead - readSoFar,
                                            readBuffer->nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        readBuffer->free(bytesToReadFromThisBlock);
    }

    return readSoFar;
}

Q_LONGLONG QProcess::writeData(const char *data, Q_LONGLONG len)
{
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%s, %lld)", data, len);
#endif

    Q_D(QProcess);
    if (len == 1) {
        d->writeBuffer.putChar(*data);
        if (d->writeSocketNotifier)
            d->writeSocketNotifier->setEnabled(true);
        return 1;
    }

    char *dest = d->writeBuffer.reserve(len);
    memcpy(dest, data, len);
    if (d->writeSocketNotifier)
        d->writeSocketNotifier->setEnabled(true);
    return len;
}

QByteArray QProcess::readAllStandardOutput()
{
    ProcessChannel tmp = inputChannel();
    setInputChannel(StandardOutput);
    QByteArray data = readAll();
    setInputChannel(tmp);
    return data;
}

QByteArray QProcess::readAllStandardError()
{
    ProcessChannel tmp = inputChannel();
    setInputChannel(StandardError);
    QByteArray data = readAll();
    setInputChannel(tmp);
    return data;
}

void QProcess::start(const QString &program, const QStringList &arguments)
{
    Q_D(QProcess);
    if (d->processState != NotRunning) {
        qWarning("QProcess::start() called when a process is already running.");
        return;
    }

    setOpenMode(QIODevice::ReadWrite);

    d->program = program;
    d->arguments = arguments;

    QCoreApplication::flush();

    d->exitCode = 0;
    d->startProcess();
}

void QProcess::kill()
{
    Q_D(QProcess);
    d->killProcess();
}

int QProcess::exitCode() const
{
    Q_D(const QProcess);
    return d->exitCode;
}

#define d d_func()
#include "moc_qprocess.cpp"
