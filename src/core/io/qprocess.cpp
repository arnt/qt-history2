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

#include "qprocess.h"
#include "qprocess_p.h"

#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qfile.h>
#include <qsocketnotifier.h>

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
    standardReadPipe[0] = -1;
    standardReadPipe[1] = -1;
    errorReadPipe[0] = -1;
    errorReadPipe[1] = -1;
    writePipe[0] = -1;
    writePipe[1] = -1;
    childStartedPipe[0] = -1;
    childStartedPipe[1] = -1;
}

QProcessPrivate::~QProcessPrivate()
{
}

void QProcessPrivate::cleanup()
{
    processChannel = QProcess::StandardOutput;
    processError = QProcess::UnknownError;
    processState = QProcess::NotRunning;
    pid = 0;
    exitCode = 0;
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
    destroyPipe(standardReadPipe);
    destroyPipe(errorReadPipe);
    destroyPipe(writePipe);
    destroyPipe(childStartedPipe);
}

void QProcessPrivate::readyReadStandardOutput(int)
{
    Q_Q(QProcess);
    Q_LONGLONG available = bytesAvailableFromStdout();
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

    if (readBytes == 0)
        standardReadSocketNotifier->setEnabled(false);
    else if (processChannel == QProcess::StandardOutput)
        emit q->readyRead();
    emit q->readyReadStandardOutput();
}

void QProcessPrivate::readyReadStandardError(int)
{
    Q_Q(QProcess);
    Q_LONGLONG available = bytesAvailableFromStderr();
    if (available == 0)
        return;

    char *ptr = errorReadBuffer.reserve(available);
    Q_LONGLONG readBytes = readFromStdout(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error reading from process"));
        emit q->error(processError);
        return;
    }
    errorReadBuffer.truncate(available - readBytes);

    if (readBytes == 0)
        errorReadSocketNotifier->setEnabled(false);
    else if (processChannel == QProcess::StandardError)
        emit q->readyRead();
    emit q->readyReadStandardError();
}

void QProcessPrivate::readyWrite(int)
{
    writeSocketNotifier->setEnabled(false);

    qDebug("QProcess::readyWrite()");
}

void QProcessPrivate::processDied()
{
    Q_Q(QProcess);

    // in case there is data in the pipe line and this slot by chance
    // got called before the read notifications, call these two slots
    // so the data is made available before the process dies.
    readyReadStandardOutput(0);
    readyReadStandardError(0);

    processState = QProcess::Finishing;
    emit q->stateChanged(processState);

    exitCode = waitForChild();
    cleanup();

    processState = QProcess::NotRunning;
    emit q->stateChanged(processState);
    emit q->finished(exitCode);
}

void QProcessPrivate::startupNotification(int)
{
    Q_Q(QProcess);
    startupSocketNotifier->setEnabled(false);
    if (processStarted()) {
        processState = QProcess::Running;
        emit q->started();
    } else {
        processState = QProcess::NotRunning;
        processError = QProcess::FailedToStart;
        emit q->error(processError);
    }
}

QProcess::QProcess(QObject *parent)
    : QIODevice(*new QProcessPrivate, parent)
{
}

QProcess::QProcess(QProcessPrivate &dd, QObject *parent)
    : QIODevice(dd, parent)
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

Q_LONGLONG QProcess::bytesAvailable() const
{
    Q_D(const QProcess);
    const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                    ? &d->errorReadBuffer
                                    : &d->outputReadBuffer;
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
    if (!d->waitForReadyRead(msecs)) {
        emit error(d->processError);
        return false;
    }

    bool emitReadyRead = false;
    if (d->processChannel == QProcess::StandardOutput) {
        int size = d->outputReadBuffer.size();
        d->readyReadStandardOutput(0);
        emitReadyRead = (size < d->outputReadBuffer.size());
    } else {
        int size = d->errorReadBuffer.size();
        d->readyReadStandardError(0);
        emitReadyRead = (size < d->errorReadBuffer.size());
    }
    if (emitReadyRead) {
        emit readyRead();
        return true;
    }
    return false;
}

bool QProcess::waitForFinished(int msecs)
{
    Q_D(QProcess);
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
    Q_D(QProcess);
    if (len == 1) {
        d->writeBuffer.putChar(*data);
        return 1;
    }

    char *dest = d->writeBuffer.reserve(len);
    memcpy(dest, data, len);
    return len;
}

void QProcess::start(const QString &program, const QStringList &arguments)
{
    Q_D(QProcess);

    // Catch this error as soon as we can. It represents 95% of the
    // reasons for the process failing.
    if (false && !QFile::exists(program)) {
        setErrorString(tr("%1 does not exist").arg(program));
        emit error(QProcess::FailedToStart);
        return;
    }

    d->arguments = arguments;
    d->program = QFile::encodeName(program);

    // Initialize pipes
    d->createPipe(d->childStartedPipe);
    d->startupSocketNotifier = new QSocketNotifier(d->childStartedPipe[0],
                                                   QSocketNotifier::Read, this);
    connect(d->startupSocketNotifier, SIGNAL(activated(int)),
            this, SLOT(startupNotification(int)));

    d->createPipe(d->writePipe);
    d->writeSocketNotifier = new QSocketNotifier(d->writePipe[1],
                                                 QSocketNotifier::Write, this);
    connect(d->writeSocketNotifier, SIGNAL(activated(int)),
            this, SLOT(readyWrite(int)));
    d->writeSocketNotifier->setEnabled(false);

    d->createPipe(d->standardReadPipe);
    d->createPipe(d->errorReadPipe);

    d->standardReadSocketNotifier = new QSocketNotifier(d->standardReadPipe[0],
                                                        QSocketNotifier::Read,
                                                        this);
    connect(d->standardReadSocketNotifier, SIGNAL(activated(int)),
            this, SLOT(readyReadStandardOutput(int)));

    d->errorReadSocketNotifier = new QSocketNotifier(d->errorReadPipe[0],
                                                     QSocketNotifier::Read,
                                                     this);
    connect(d->errorReadSocketNotifier, SIGNAL(activated(int)),
            this, SLOT(readyReadStandardError(int)));

    // Start the process (platform dependent)
    d->processState = QProcess::Starting;
    emit stateChanged(d->processState);

    QCoreApplication::flush();

    d->startProcess();
}

void QProcess::kill()
{
    Q_D(QProcess);
    d->killProcess();
}

#define d d_func()
#include "moc_qprocess.cpp"
