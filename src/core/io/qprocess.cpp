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

#if defined QPROCESS_DEBUG
#include <qstring.h>
#include <ctype.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(c)) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            QString tmp;
            tmp.sprintf("\\%o", c);
            out += tmp.toLatin1();
        }
    }

    if (len < maxSize)
        out += "...";

    return out;
}
#endif

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
    enters the \c Starting state, and when the program has started,
    QProcess enters the \c Running state and emits started().

    QProcess allows you to treat a process as a sequential I/O
    device. You can write to and read from the process just as you
    would access a network connection using QTcpSocket. You can then
    write to the process's standard input by calling write(), and
    read the standard output by calling read(), readLine(), and
    getChar(). Because it inherits QIODevice, QProcess can also be
    used as an input source for QXmlReader, or for generating data to
    be uploaded using QFtp.

    When the process exits, QProcess reenters the \c NotRunning state
    (the initial state), and emits finished().

    The finished() signal provides the exit code of the process as an
    argument, and you can also call exitCode() to obtain the exit code
    of the last process that finished. If an error occurs at any point
    in time, QProcess will emit the error() signal. You can also call
    error() to find the type of error that occurred last, and state()
    to find the current process state.

    Processes have two predefined output channels: The standard
    output channel supplies regular console output, and the standard
    error channel usually supplies the errors that are printed by the
    process. These channels represent two separate streams of data.
    You can toggle between them by calling setReadChannel(). QProcess
    emits readyRead() when data is available on the current input
    channel. It also emits readyReadStandardOutput() when new standard
    output data is available, and when new standard error data is
    available, readyReadStandardError() is emitted. Instead of calling
    read(), readLine(), or readChar(), you can explicitly read all data
    from either of the two channels by calling readAllStandardOutput()
    or readAllStandardError().

    QProcess can merge the two output channels, so that standard
    output and standard error data from the running process both use
    the standard output channel. Call setReadChannelMode() with
    \c MergedOutputChannels before starting the process to activative
    this feature. You also have the option of forwarding the output of
    the running process to the calling, main process, by passing
    \c ForwardedOutputChannels as the argument.

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

    \quotefromfile snippets/process/process.cpp
    \skipto QProcess gzip;
    \printuntil result = gzip.readAll();

    \sa QBuffer, QFile, QTcpSocket
*/

/*!
    \enum QProcess::ProcessChannel

    This enum describes the process channels used by the running process.
    Pass one of these values to QProcess::setReadChannel() to set the
    current input channel of QProcess.

    \value StandardOutput The standard output (stdout) of the running
           process.

    \value StandardError The standard error (stderr) of the running
           process.

    \sa QProcess::setReadChannel()
*/

/*!
    \enum QProcess::ProcessChannelMode

    This enum describes the process channel modes of QProcess. Pass
    one of these values to QProcess::setReadChannelMode() to set the
    current input channel mode.

    \value SeparateChannels QProcess manages the output of the running
    process, keeping standard output and standard error data in
    separate internal buffers. You can select the current input
    channel by calling QProcess::setReadChannel(). This is the
    default channel mode of QProcess.

    \value MergedChannels QProcess merges the output of the running
    process into the standard output channel. The standard error
    channel will not receive any data. The standard output and
    standard error data of the running process are interleaved.

    \value ForwardedChannels QProcess forwards the output of the
    running process onto the main process. Anything the child process
    writes to its standard output and standard error will be written
    to the standard output and standard error of the main process.

    \sa QProcess::setReadChannelMode()
*/

/*!
    \enum QProcess::ProcessError

    This enum describes the different types of errors that are
    reported by QProcess.

    \value FailedToStart The process failed to start. Either the
    invoked program is missing, or you may have insufficient
    permissions to invoke the program.

    \value Crashed The process crashed some time after starting
    successfully.

    \value Timedout The last waitFor...() function timed out. The
    state of QProcess is unchanged, and you can try calling
    waitFor...() again.

    \value WriteError An error occurred when attempting to write to the
    process. For example, the process may not be running, or it may
    have closed its input channel.

    \value ReadError An error occurred when attempting to read from
    the process. For example, the process may not be running.

    \value UnknownError An unknown error occurred. This is the default
    return value of QProcess::error().

    \sa QProcess::error()
*/

/*!
    \enum QProcess::ProcessState

    This enum describes the different states of QProcess.

    \value NotRunning The process is not running.

    \value Starting The process is starting, but the program has not
    yet been invoked.

    \value Running The process is running and is ready for reading and
    writing.

    \sa QProcess::state()
*/

/*!
    \fn QProcess::error(ProcessError error)

    This signal is emitted when an error occurs with the process. The
    specified \a error describes the type of error that occurred.

    \sa QProcess::ProcessError
*/

/*!
    \fn QProcess::started()

    This signal is emitted by QProcess when the process has started,
    and QProcess::state() returns \c Running.

    \sa QProcess::ProcessState
*/

/*!
    \fn QProcess::stateChanged(ProcessState newState)

    This signal is emitted whenever the state of QProcess changes. The
    \a newState argument is the state QProcess changed to.
*/

/*!
    \fn QProcess::finished(int exitCode)

    This signal is emitted when the process finishes. \a exitCode is
    the exit code of the process. After the process has finished, the
    buffers in QProcess are still intact. You can still read any data
    that the process may have written before it finished.
*/

/*!
    \fn QProcess::readyReadStandardOutput()

    This signal is emitted when new data has arrived in the standard
    output channel of QProcess. It is emitted regardless of the current
    input channel.

    \sa QProcess::readAllStandardOutput()
*/

/*!
    \fn QProcess::readyReadStandardError()

    This signal is emitted when new data has arrived in the standard
    error channel of QProcess. It is emitted regardless of the current
    input channel.

    \sa QProcess::readAllStandardError()
*/

/*! \internal
*/
QProcessPrivate::QProcessPrivate()
{
    processChannel = QProcess::StandardOutput;
    processChannelMode = QProcess::SeparateChannels;
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
    writeChannelClosing = false;
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
    qint64 available = bytesAvailableFromStdout();
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardOutput(), %lld bytes available",
           available);
#endif

    if (available == 0)
        return false;

    char *ptr = outputReadBuffer.reserve(available);
    qint64 readBytes = readFromStdout(ptr, available);
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
    qint64 available = bytesAvailableFromStderr();
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardError(), %lld bytes available",
           available);
#endif

    if (available == 0)
        return false;

    char *ptr = errorReadBuffer.reserve(available);
    qint64 readBytes = readFromStderr(ptr, available);
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
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canWrite()");
#endif

    if (writeSocketNotifier)
        writeSocketNotifier->setEnabled(false);

    if (writeBuffer.isEmpty())
        return false;

    qint64 written = writeToStdin(writeBuffer.readPointer(),
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
    if (writeBuffer.isEmpty() && writeChannelClosing)
        closeWriteChannel();
    return true;
}

/*! \internal
*/
void QProcessPrivate::processDied()
{
    Q_Q(QProcess);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::processDied()");
#endif

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
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::startupNotification()");
#endif

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
void QProcessPrivate::closeWriteChannel()
{
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::closeWriteChannel()");
#endif

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
#if defined QPROCESS_DEBUG
    qDebug("QProcess::QProcess(%p)", parent);
#endif
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
    Returns the input channel mode of the QProcess.

    \sa setReadChannelMode(), ProcessChannelMode, setReadChannel()
*/
QProcess::ProcessChannelMode QProcess::readChannelMode() const
{
    Q_D(const QProcess);
    return d->processChannelMode;
}

/*!
    Sets the input channel mode of the QProcess to the \a mode specified.
    This mode will be used the next time start() is called. For example:

    \code
        QProcess builder;
        builder.setReadChannelMode(QProcess::MergedChannels);
        builder.start("make", QStringList() << "-j2");
        if (!builder.waitForFinished())
            qDebug("make failed: %s", builder.errorString().toLocal8Bit().constData());
        else
            qDebug("make output: %s", builder.readAll().constData());
    \endcode

    \sa readChannelMode(), ProcessChannelMode, setReadChannel()
*/
void QProcess::setReadChannelMode(ProcessChannelMode mode)
{
    Q_D(QProcess);
    d->processChannelMode = mode;
}

/*!
    Returns the current input channel of the QProcess.

    \sa setReadChannel()
*/
QProcess::ProcessChannel QProcess::readChannel() const
{
    Q_D(const QProcess);
    return d->processChannel;
}

/*!
    Sets the current input channel of the QProcess to the \a channel
    specified. The current input channel is used by the functions read(),
    readAll(), readLine(), and getChar(). It also determines which
    channel triggers QProcess to emit readyRead().

    \sa readChannel()
*/
void QProcess::setReadChannel(ProcessChannel channel)
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

    \sa closeWriteChannel(), setReadChannel()
*/
void QProcess::closeReadChannel(ProcessChannel channel)
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
        more.closeWriteChannel();
        // QProcess will emit readyRead() once "more" starts printing
    \endcode

    The output channel is implicitly opened when start() is called.

    \sa closeReadChannel()
*/
void QProcess::closeWriteChannel()
{
    Q_D(QProcess);
    d->writeChannelClosing = true;
    if (d->writeBuffer.isEmpty())
        d->closeWriteChannel();

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

    \sa readChannel(), setReadChannel()
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
    emit aboutToClose();
    if (bytesToWrite() > 0)
        flush();
    terminate();
    waitForFinished(-1);
    setOpenMode(QIODevice::NotOpen);
}

/*! \reimp

   Returns true if the process is not running, and no more data is available
   for reading; otherwise returns false.
*/
bool QProcess::atEnd() const
{
    Q_D(const QProcess);
    const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                    ? &d->errorReadBuffer
                                    : &d->outputReadBuffer;
    return readBuffer->isEmpty() && d->processState == NotRunning;
}

/*! \reimp
*/
bool QProcess::isSequential() const
{
    return true;
}

/*! \reimp
*/
qint64 QProcess::bytesAvailable() const
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

/*! \reimp
*/
qint64 QProcess::bytesToWrite() const
{
    Q_D(const QProcess);
    return d->writeBuffer.size();
}

/*!
    Returns the type of error that occurred last.

    \sa error()
*/
QProcess::ProcessError QProcess::error() const
{
    Q_D(const QProcess);
    return d->processError;
}

/*!
    Returns the current state of the process.

    \sa stateChanged()
*/
QProcess::ProcessState QProcess::state() const
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

    If msecs is -1, this function will not time out.

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

    If msecs is -1, this function will not time out.

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

/*!
    Sets the current state of the QProcess to the \a state specified.

    \sa state()
*/
void QProcess::setProcessState(ProcessState state)
{
    Q_D(QProcess);
    d->processState = state;
}

/*!
    This is a platform specific virtual function called by QProcess on
    Unix only. On Windows, it is not called.

    setupChildProcess() is called in the child process context, at the
    very moment before the program is executed. Reimplement this
    function to do last minute initialization of the child process.
*/
void QProcess::setupChildProcess()
{
}

/*! \reimp
*/
qint64 QProcess::readData(char *data, qint64 maxlen)
{
    Q_D(QProcess);
    QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                              ? &d->errorReadBuffer
                              : &d->outputReadBuffer;

    if (maxlen == 1) {
        int c = readBuffer->getChar();
        if (c == -1) {
#if defined QPROCESS_DEBUG
            qDebug("QProcess::readData(%p \"%s\", %d) == -1",
                   data, qt_prettyDebug(data, maxlen, 1).constData(), 1);
#endif
            return -1;
        }
        *data = (char) c;
#if defined QPROCESS_DEBUG
        qDebug("QProcess::readData(%p \"%s\", %d) == 1",
               data, qt_prettyDebug(data, maxlen, 1).constData(), 1);
#endif
        return 1;
    }

    qint64 bytesToRead = qint64(qMin(readBuffer->size(), (int)maxlen));
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        char *ptr = readBuffer->readPointer();
        int bytesToReadFromThisBlock = qMin<qint64>(bytesToRead - readSoFar,
                                            readBuffer->nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        readBuffer->free(bytesToReadFromThisBlock);
    }

#if defined QPROCESS_DEBUG
    qDebug("QProcess::readData(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, readSoFar, 16).constData(), maxlen, readSoFar);
#endif
    return readSoFar;
}

/*! \reimp
*/
qint64 QProcess::writeData(const char *data, qint64 len)
{
    Q_D(QProcess);

    if (d->writeChannelClosing) {
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%p \"%s\", %lld) == 0 (output channel closing)",
           data, qt_prettyDebug(data, len, 16).constData(), len);
#endif
        return 0;
    }

    if (len == 1) {
        d->writeBuffer.putChar(*data);
        if (d->writeSocketNotifier)
            d->writeSocketNotifier->setEnabled(true);
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%p \"%s\", %lld) == 1 (written to buffer)",
           data, qt_prettyDebug(data, len, 16).constData(), len);
#endif
        return 1;
    }

    char *dest = d->writeBuffer.reserve(len);
    memcpy(dest, data, len);
    if (d->writeSocketNotifier)
        d->writeSocketNotifier->setEnabled(true);
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%p \"%s\", %lld) == %lld (written to buffer)",
           data, qt_prettyDebug(data, len, 16).constData(), len, len);
#endif
    return len;
}

/*!
    Regardless of the current input channel, this function returns all
    data available from the standard output of the process as a
    QByteArray.

    \sa readyReadStandardOutput(), readAllStandardError(), readChannel(), setReadChannel()
*/
QByteArray QProcess::readAllStandardOutput()
{
    ProcessChannel tmp = readChannel();
    setReadChannel(StandardOutput);
    QByteArray data = readAll();
    setReadChannel(tmp);
    return data;
}

/*!
    Regardless of the current input channel, this function returns all
    data available from the standard error of the process as a
    QByteArray.

    \sa readyReadStandardError(), readAllStandardOutput(), readChannel(), setReadChannel()
*/
QByteArray QProcess::readAllStandardError()
{
    ProcessChannel tmp = readChannel();
    setReadChannel(StandardError);
    QByteArray data = readAll();
    setReadChannel(tmp);
    return data;
}

/*!
    Starts the program \a program in a new process, passing the
    command line arguments in \a arguments. The OpenMode is set to \a
    mode. QProcess will immediately enter the Starting state. If the
    process starts successfully, QProcess will emit started();
    otherwise, error() will be emitted.

    \sa pid(), started(), launch()
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

    d->writeChannelClosing = false;
    d->standardOutputClosed = false;
    d->standardErrorClosed = false;

    d->program = program;
    d->arguments = arguments;

    QCoreApplication::flush();

    d->exitCode = 0;
    d->processError = QProcess::UnknownError;
    setErrorString(tr("Unknown error"));
    d->startProcess();
}

/*!
    \overload

    Starts the program \a program in a new process. \a program is a
    single string of text containing both the program name and its
    arguments. The arguments are separated by one or more
    spaces. Example:

    \code
        QProcess process;
        process.start("del /s *.txt");
        // same as process.start("del", QStringList() << "/s" << "*.txt");
        ...
    \endcode

    The OpenMode is set to \a mode.
*/
void QProcess::start(const QString &program, OpenMode mode)
{
    QStringList args = program.split(QLatin1Char(' '));
    QString prog = args.first();
    args.removeFirst();
    start(prog, args, mode);
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

/*!
    Starts the program \a program with the arguments \a arguments in a
    new process, waits for it to finish, and then returns the exit
    code of the process. Any data the new process writes to the
    console is forwarded to the calling process.

    The environment and working directory are inherited by the calling
    process.
*/
int QProcess::execute(const QString &program, const QStringList &arguments)
{
    QProcess process;
    process.setReadChannelMode(ForwardedChannels);
    process.start(program, arguments);
    process.waitForFinished(-1);
    return process.exitCode();
}

/*!
    \overload

    Starts the program \a program in a new process. \a program is a
    single string of text containing both the program name and its
    arguments. The arguments are separated by one or more spaces.
*/
int QProcess::execute(const QString &program)
{
    QProcess process;
    process.setReadChannelMode(ForwardedChannels);
    process.start(program);
    process.waitForFinished(-1);
    return process.exitCode();
}

/*!
    Starts the program \a program with the arguments \a arguments in a
    new process, and detaches from it. Returns true on success;
    otherwise returns false. If the calling process exits, the
    detached process will continue to live.

    On Unix, the started process will run in its own session and act
    like a daemon. On Windows, it will run as a regular standalone
    process.
*/
bool QProcess::startDetached(const QString &program, const QStringList &arguments)
{
    return QProcessPrivate::startDetached(program, arguments);
}

/*!
    \overload

    Starts the program \a program in a new process. \a program is a
    single string of text containing both the program name and its
    arguments. The arguments are separated by one or more spaces.
*/
bool QProcess::startDetached(const QString &program)
{
    QStringList args = program.split(QLatin1Char(' '));
    QString prog = args.first();
    args.removeFirst();
    return QProcessPrivate::startDetached(prog, args);
}

#define d d_func()
#include "moc_qprocess.cpp"
