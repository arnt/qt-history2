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

#ifdef Q_WS_WIN
#include <private/qwineventnotifier_p.h>
#endif

/*!
    \class QProcess

    \brief The QProcess class is used to start external programs and
    to communicate with them.

    \ingroup io
    \ingroup misc
    \mainclass

    To start a process, pass the name and command line arguments of
    the program you want to run as arguments to start(). QProcess then
    enters the \l Starting state, and when the program has started,
    QProcess enters the \l Running state and emits started().

    QProcess allows you to treat a process as a sequential I/O
    device. You can write to and read from the process just as you
    would access a network connection using QTcpSocket. You can then
    write to the process's standard input by calling write(), and
    read the standard output by calling read(), readLine() and
    getChar(). Because it inherits QIODevice, QProcess can also be
    used as an input source for QXmlReader or for generating data to
    be uploaded using QFtp.

    When the process exits, QProcess reenters the NotRunning state
    (the initial state) and emits finished().

    The finished() signal provides the exit code of the process as an
    argument, and you can also call exitCode(), which returns the
    exit code of the last process that finished. At an error occurs
    at any point in time, QProcess will emit the error() signal. You
    can also call processError() to find the type of error that
    occurred last, and processState() to find the current process
    state.

    Processes have two predefined output channels: the standard
    output channel (for regular console output) and the standard
    error channel (where errors are usually printed). These channels
    represent two separate streams of data. You can toggle between
    the two channels by calling setInputChannel(). QProcess emits
    readyRead() when data is available on the current input channel.
    It also emits readyReadStandardOutput() when new standard output
    data is available, and when new standard error data is available,
    readyReadStandardError() is emitted. Instead of calling read(),
    readLine(), or readChar(), you can explicitly read all data from
    either of the two channels by calling readAllStandardOutput() or
    readAllStandardError().

    Certain processes need special environment settings in order to
    operate. You can set environment variables for your process by
    calling setEnvironment(). To set a working directory, call
    setWorkingDirectory(). By default, processes are run in the
    current working directory of the calling process.

    QProcess provides a set of functions which allow it to be used
    without an event loop, by suspending the calling thread until
    certain signals are emitted:

    \list
    \o waitForStarted() blocks until the process has started.

    \o waitForReadyRead() blocks until new data is
    available for reading on the current input channel.

    \o waitForBytesWritten() blocks until one payload of
    data has been written to the process.

    \o waitForFinished() blocks until the process has finished.
    \endlist

    Calling these functions from the main thread (the thread that
    calls QApplication::exec()) may cause your user interface to
    freeze.

    The following example runs \c gzip to compress the string "Qt
    rocks!", without an event loop:

    \omit
    This code doesn't work
    \code
        QProcess gzip;
        gzip.start("gzip", QStringList() << "-c");
        if (!gzip.waitForStarted())
            return false;

        gzip.write("Qt rocks!");
        gzip.flush();

        QByteArray result;
        while (gzip.waitForReadyRead())
            result += gzip.readAll();
    \endcode
    \endomit

    \sa QBuffer, QFile, QTcpSocket
*/

/*! \internal
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
    outputChannelClosing = false;
#ifdef Q_WS_WIN
    pipeWriter = 0;
    processFinishedNotifier = 0;
#endif // Q_WS_WIN
}

/*! \internal
*/
QProcessPrivate::~QProcessPrivate()
{
}

/*! \internal
*/
void QProcessPrivate::cleanup()
{
    Q_Q(QProcess);

    q->setOpenMode(QIODevice::ReadOnly);
    processChannel = QProcess::StandardOutput;
    processError = QProcess::UnknownError;
    processState = QProcess::NotRunning;
#ifdef Q_OS_WIN
    if (pid) {
        delete pid;
    }
    if (processFinishedNotifier) {
        processFinishedNotifier->setEnabled(false);
        delete processFinishedNotifier;
        processFinishedNotifier = 0;
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

/*! \internal
*/
bool QProcessPrivate::canReadStandardOutput()
{
    Q_Q(QProcess);
    Q_LONGLONG available = bytesAvailableFromStdout();
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardOutput(), %lld bytes available",
           available);
#endif

    if (available == 0)
        return false;

    char *ptr = outputReadBuffer.reserve(available);
    Q_LONGLONG readBytes = readFromStdout(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error reading from process"));
        emit q->error(processError);
        return false;
    }
    if (standardOutputClosed) {
        outputReadBuffer.truncate(readBytes);
        return false;
    }

    outputReadBuffer.truncate(available - readBytes);

    bool readyReadEmitted = false;
    if (readBytes == 0) {
        if (standardReadSocketNotifier)
            standardReadSocketNotifier->setEnabled(false);
    } else if (processChannel == QProcess::StandardOutput) {
        readyReadEmitted = true;
        emit q->readyRead();
    }
    emit q->readyReadStandardOutput();
    return readyReadEmitted;
}

/*! \internal
*/
bool QProcessPrivate::canReadStandardError()
{
    Q_Q(QProcess);
    Q_LONGLONG available = bytesAvailableFromStderr();
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardError(), %lld bytes available",
           available);
#endif

    if (available == 0)
        return false;

    char *ptr = errorReadBuffer.reserve(available);
    Q_LONGLONG readBytes = readFromStderr(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error reading from process"));
        emit q->error(processError);
        return false;
    }
    if (standardErrorClosed) {
        errorReadBuffer.truncate(readBytes);
        return false;
    }

    errorReadBuffer.truncate(available - readBytes);

    bool readyReadEmitted = false;
    if (readBytes == 0) {
        if (errorReadSocketNotifier)
            errorReadSocketNotifier->setEnabled(false);
    } else if (processChannel == QProcess::StandardError) {
        readyReadEmitted = true;
        emit q->readyRead();
    }
    emit q->readyReadStandardError();
    return readyReadEmitted;
}

/*! \internal
*/
bool QProcessPrivate::canWrite()
{
    Q_Q(QProcess);
    if (writeSocketNotifier)
        writeSocketNotifier->setEnabled(false);

    if (writeBuffer.isEmpty())
        return false;

    Q_LONGLONG written = writeToStdin(writeBuffer.readPointer(),
                                      writeBuffer.nextDataBlockSize());
    if (written < 0) {
        processError = QProcess::WriteError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Error writing to process"));
        emit q->error(processError);
        return false;
    }

    writeBuffer.free(written);
    emit q->bytesWritten(written);
    if (writeSocketNotifier && !writeBuffer.isEmpty())
        writeSocketNotifier->setEnabled(true);
    if (writeBuffer.isEmpty() && outputChannelClosing)
        closeOutputChannel();
    return true;
}

/*! \internal
*/
void QProcessPrivate::processDied()
{
    Q_Q(QProcess);

    // in case there is data in the pipe line and this slot by chance
    // got called before the read notifications, call these two slots
    // so the data is made available before the process dies.
    canReadStandardOutput();
    canReadStandardError();

    findExitCode();

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

/*! \internal
*/
bool QProcessPrivate::startupNotification()
{
    Q_Q(QProcess);
    if (startupSocketNotifier)
        startupSocketNotifier->setEnabled(false);
    if (processStarted()) {
        processState = QProcess::Running;
        emit q->started();
        return true;
    }

    processState = QProcess::NotRunning;
    processError = QProcess::FailedToStart;
    emit q->error(processError);
    cleanup();
    return false;
}

/*! \internal
*/
void QProcessPrivate::closeOutputChannel()
{
    if (writeSocketNotifier) {
        writeSocketNotifier->setEnabled(false);
        delete writeSocketNotifier;
        writeSocketNotifier = 0;
    }
    destroyPipe(writePipe);
}

/*!
    Constructs a QProcess object with the given \a parent.
*/
QProcess::QProcess(QObject *parent)
    : QIODevice(*new QProcessPrivate, parent)
{
}

/*!
    Destructs the QProcess object.
*/
QProcess::~QProcess()
{
    Q_D(QProcess);
    if (d->processState != NotRunning) {
        qWarning("QProcess object destroyed while process is still running.");
        terminate();
    }
    d->cleanup();
}

/*!
    Returns the current input channel of QProcess.

    \sa setInputChannel()
*/
QProcess::ProcessChannel QProcess::inputChannel() const
{
    Q_D(const QProcess);
    return d->processChannel;
}

/*!
    Sets the current input channel of QProcess to \a channel. The
    current input channel is used by the functions read(), readAll(),
    readLine() and getChar(). It also decides which channel triggers
    QProcess to emit readyRead().

    \sa inputChannel()
*/
void QProcess::setInputChannel(ProcessChannel channel)
{
    Q_D(QProcess);
    d->processChannel = channel;
}

/*!
    Closes the input channel \a channel. After calling this function,
    QProcess will no longer receive data on the channel. Any data that
    has already been received is still available for reading.

    Call this function to save memory, if you are not interested in
    the output of the process.

    \sa closeOutputChannel(), setInputChannel()
*/
void QProcess::closeInputChannel(ProcessChannel channel)
{
    Q_D(QProcess);

    if (channel == StandardOutput)
        d->standardOutputClosed = true;
    else
        d->standardErrorClosed = true;
}

/*!
    Schedules the output channel of QProcess to be closed. The channel
    will close once all data has been written to the process. After
    calling this function, any attempts to write to the process will
    fail.

    Closing the output channel is necessary for programs that read
    input data until the channel has been closed. For example, the
    program "more" is used to display text data in a console on both
    Unix and Windows. But it will not display the text data until
    QProcess' output channel has been closed. Example:

    \code
        QProcess more;
        more.start("more");
        more.write("Text to display");
        more.closeOutputChannel();
        // QProcess will emit readyRead() once "more" starts printing
    \endcode

    The output channel is implicitly opened when start() is called.

    \sa closeInputChannel()
*/
void QProcess::closeOutputChannel()
{
    Q_D(QProcess);
    d->outputChannelClosing = true;
    if (d->writeBuffer.isEmpty())
        d->closeOutputChannel();

}

/*!
    Returns the working directory that the QProcess will enter before
    the program has started.

    \sa setWorkingDirectory()
*/
QString QProcess::workingDirectory() const
{
    Q_D(const QProcess);
    return d->workingDirectory;
}

/*!
    Sets the working directory to \a dir. QProcess will start the
    process in this directory. The default behavior is to start the
    process in the working directory of the calling process.

    \sa setWorkingDirectory(), start()
*/
void QProcess::setWorkingDirectory(const QString &dir)
{
    Q_D(QProcess);
    d->workingDirectory = dir;
}

/*!
    Returns the native process identifier for the running process, if
    available.  If no process is currently running, 0 is returned.
*/
Q_PID QProcess::pid() const
{
    Q_D(const QProcess);
    return d->pid;
}

/*! \reimp

    This function operates on the current input channel.

    \sa inputChannel(), setInputChannel()
*/
bool QProcess::canReadLine() const
{
    Q_D(const QProcess);
    const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                    ? &d->errorReadBuffer
                                    : &d->outputReadBuffer;
    return readBuffer->canReadLine();
}

/*!
    Closes all communication with the process. After calling this
    function, QProcess will no longer emit readyRead(), and data can no
    longer be read or written.
*/
void QProcess::close()
{
    closeOutputChannel();
    closeInputChannel(QProcess::StandardOutput);
    closeInputChannel(QProcess::StandardError);
    QIODevice::close();
}

/*! \reimp

    Calling this function is equivalent to calling
    waitForBytesWritten() until bytesToWrite() returns false.

    \sa waitForBytesWritten()
*/
bool QProcess::flush()
{
    Q_D(QProcess);

    while (!d->writeBuffer.isEmpty()) {
        if (!d->waitForBytesWritten(0))
            return false;
    }
    return true;
}

/*! \reimp
*/
bool QProcess::isSequential() const
{
    return true;
}

/*! \reimp
*/
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

/*!
    Returns the type of error that occurred last.

    \sa error()
*/
QProcess::ProcessError QProcess::processError() const
{
    Q_D(const QProcess);
    return d->processError;
}

/*!
    Returns the current state of the process.

    \sa stateChanged()
*/
QProcess::ProcessState QProcess::processState() const
{
    Q_D(const QProcess);
    return d->processState;
}

/*!
    Sets the environment that QProcess will use when starting a
    process to \a environment.

    \sa environment()
*/
void QProcess::setEnvironment(const QStringList &environment)
{
    Q_D(QProcess);
    d->environment = environment;
}

/*!
    Returns the environment that QProcess will use when starting a
    process, or an empty QStringList if no environment has been set.
    It no environment has been set, the environment of the calling
    process will be used.

    \sa setEnvironment()
*/
QStringList QProcess::environment() const
{
    Q_D(const QProcess);
    return d->environment;
}

/*!
    Blocks until the process has started and the started() signal has
    been emitted, or until \a msecs milliseconds have passed.

    Returns true if the process was started successfully; otherwise
    returns false (if the operation timed out or if an error
    occurred).

    This function can operate without an event loop. It is
    useful when writing non-GUI applications and when performing
    I/O operations in a non-GUI thread.

    \warning Calling this function from the main (GUI) thread
    might cause your user interface to freeze.

    \sa waitForFinished()
*/
bool QProcess::waitForStarted(int msecs)
{
    Q_D(QProcess);
    if (d->processState == QProcess::Starting) {
        if (!d->waitForStarted(msecs)) {
            emit error(d->processError);
            return false;
        }
        d->processState = QProcess::Running;
        emit started();
    }
    return true;
}

/*! \reimp
*/
bool QProcess::waitForReadyRead(int msecs)
{
    Q_D(QProcess);

    if (d->processChannel == QProcess::StandardOutput && d->standardOutputClosed)
        return false;
    if (d->processChannel == QProcess::StandardError && d->standardErrorClosed)
        return false;

    if (d->waitForReadyRead(msecs))
        return true;

    emit error(d->processError);
    return false;
}

/*! \reimp
*/
bool QProcess::waitForBytesWritten(int msecs)
{
    Q_D(QProcess);
    return d->waitForBytesWritten(msecs);
}

/*!
    Blocks until the process has finished and the finished() signal
    has been emitted, or until \a msecs milliseconds have passed.

    Returns true if the process finished; otherwise returns false (if
    the operation timed out or if an error occurred).

    This function can operate without an event loop. It is
    useful when writing non-GUI applications and when performing
    I/O operations in a non-GUI thread.

    \warning Calling this function from the main (GUI) thread
    might cause your user interface to freeze.

    \sa waitForFinished()
*/
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

/*! \reimp
*/
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

/*! \reimp
*/
Q_LONGLONG QProcess::writeData(const char *data, Q_LONGLONG len)
{
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%s, %lld)", data, len);
#endif

    Q_D(QProcess);

    if (d->outputChannelClosing)
        return 0;

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

/*!
    Regardless of the current input channel, this function returns all
    data available from the standard output of the process as a
    QByteArray.

    \sa readyReadStandardOutput(), readAllStandardError(), inputChannel(), setInputChannel()
*/
QByteArray QProcess::readAllStandardOutput()
{
    ProcessChannel tmp = inputChannel();
    setInputChannel(StandardOutput);
    QByteArray data = readAll();
    setInputChannel(tmp);
    return data;
}

/*!
    Regardless of the current input channel, this function returns all
    data available from the standard error of the process as a
    QByteArray.

    \sa readyReadStandardError(), readAllStandardOutput(), inputChannel(), setInputChannel()
*/
QByteArray QProcess::readAllStandardError()
{
    ProcessChannel tmp = inputChannel();
    setInputChannel(StandardError);
    QByteArray data = readAll();
    setInputChannel(tmp);
    return data;
}

/*!
    Starts the program \a program in a new process, passing the
    command line arguments in \a arguments. The openMode is set to \a
    mode. QProcess will immediately enter the Starting state. If the
    process starts successfully, QProcess will emit started();
    otherwise, error() will be emitted.

    \sa pid(), started()
*/
void QProcess::start(const QString &program, const QStringList &arguments, OpenMode mode)
{
    Q_D(QProcess);
    if (d->processState != NotRunning) {
        qWarning("QProcess::start() called when a process is already running.");
        return;
    }

    d->outputReadBuffer.clear();
    d->errorReadBuffer.clear();
    setOpenMode(mode);

    d->outputChannelClosing = false;
    d->standardOutputClosed = false;
    d->standardErrorClosed = false;

    d->program = program;
    d->arguments = arguments;

    QCoreApplication::flush();

    d->exitCode = 0;
    d->startProcess();
}

/*!
    Terminates the current process, causing it to crash.
*/
void QProcess::terminate()
{
    Q_D(QProcess);
    d->killProcess();
}

/*!
    Returns the exit code of the last process that finished.
*/
int QProcess::exitCode() const
{
    Q_D(const QProcess);
    return d->exitCode;
}

#define d d_func()
#include "moc_qprocess.cpp"
