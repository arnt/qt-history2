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

//#define QPROCESS_DEBUG

#if defined QPROCESS_DEBUG
#include <qdebug.h>
#include <qstring.h>
#include <ctype.h>
#include <errno.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len && i < maxSize; ++i) {
        char c = data[i];
        if (isprint(c)) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            char buf[5];
            qsnprintf(buf, sizeof(buf), "\\%3o", c);
            buf[4] = '\0';
            out += QByteArray(buf);
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

#ifndef QT_NO_PROCESS

/*!
    \class QProcess

    \brief The QProcess class is used to start external programs and
    to communicate with them.

    \ingroup io
    \ingroup misc
    \mainclass
    \reentrant

    To start a process, pass the name and command line arguments of
    the program you want to run as arguments to start(). For example:

    \quotefromfile snippets/qprocess/qprocess-simpleexecution.cpp
    \skipto parent
    \printline parent
    \dots
    \skipto program
    \printline program
    \skipto QStringList
    \printuntil start

    QProcess then enters the \l Starting state, and when the program
    has started, QProcess enters the \l Running state and emits
    started().

    QProcess allows you to treat a process as a sequential I/O
    device. You can write to and read from the process just as you
    would access a network connection using QTcpSocket. You can then
    write to the process's standard input by calling write(), and
    read the standard output by calling read(), readLine(), and
    getChar(). Because it inherits QIODevice, QProcess can also be
    used as an input source for QXmlReader, or for generating data to
    be uploaded using QFtp.

    When the process exits, QProcess reenters the \l NotRunning state
    (the initial state), and emits finished().

    The finished() signal provides the exit code and exit status of
    the process as arguments, and you can also call exitCode() to
    obtain the exit code of the last process that finished, and
    exitStatus() to obtain its exit status. If an error occurs at
    any point in time, QProcess will emit the error() signal. You
    can also call error() to find the type of error that occurred
    last, and state() to find the current process state.

    Processes have two predefined output channels: The standard
    output channel (\c stdout) supplies regular console output, and
    the standard error channel (\c stderr) usually supplies the
    errors that are printed by the process. These channels represent
    two separate streams of data. You can toggle between them by
    calling setReadChannel(). QProcess emits readyRead() when data is
    available on the current read channel. It also emits
    readyReadStandardOutput() when new standard output data is
    available, and when new standard error data is available,
    readyReadStandardError() is emitted. Instead of calling read(),
    readLine(), or getChar(), you can explicitly read all data from
    either of the two channels by calling readAllStandardOutput() or
    readAllStandardError().

    The terminology for the channels can be misleading. Be aware that
    the process's output channels correspond to QProcess's
    \e read channels, whereas the process's input channels correspond
    to QProcess's \e write channels. This is because what we read
    using QProcess is the process's output, and what we write becomes
    the process's input.

    QProcess can merge the two output channels, so that standard
    output and standard error data from the running process both use
    the standard output channel. Call setProcessChannelMode() with
    MergedChannels before starting the process to activative
    this feature. You also have the option of forwarding the output of
    the running process to the calling, main process, by passing
    ForwardedChannels as the argument.

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
    available for reading on the current read channel.

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
    Pass one of these values to setReadChannel() to set the
    current read channel of QProcess.

    \value StandardOutput The standard output (stdout) of the running
           process.

    \value StandardError The standard error (stderr) of the running
           process.

    \sa setReadChannel()
*/

/*!
    \enum QProcess::ProcessChannelMode

    This enum describes the process channel modes of QProcess. Pass
    one of these values to setProcessChannelMode() to set the
    current read channel mode.

    \value SeparateChannels QProcess manages the output of the
    running process, keeping standard output and standard error data
    in separate internal buffers. You can select the QProcess's
    current read channel by calling setReadChannel(). This is the
    default channel mode of QProcess.

    \value MergedChannels QProcess merges the output of the running
    process into the standard output channel (\c stdout). The
    standard error channel (\c stderr) will not receive any data. The
    standard output and standard error data of the running process
    are interleaved.

    \value ForwardedChannels QProcess forwards the output of the
    running process onto the main process. Anything the child process
    writes to its standard output and standard error will be written
    to the standard output and standard error of the main process.

    \sa setReadChannelMode()
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
    return value of error().

    \sa error()
*/

/*!
    \enum QProcess::ProcessState

    This enum describes the different states of QProcess.

    \value NotRunning The process is not running.

    \value Starting The process is starting, but the program has not
    yet been invoked.

    \value Running The process is running and is ready for reading and
    writing.

    \sa state()
*/

/*!
    \enum QProcess::ExitStatus

    This enum describes the different exit statuses of QProcess.

    \value NormalExit The process exited normally.

    \value CrashExit The process crashed.

    \sa exitStatus()
*/

/*!
    \fn void QProcess::error(QProcess::ProcessError error)

    This signal is emitted when an error occurs with the process. The
    specified \a error describes the type of error that occurred.
*/

/*!
    \fn void QProcess::started()

    This signal is emitted by QProcess when the process has started,
    and state() returns \l Running.
*/

/*!
    \fn void QProcess::stateChanged(QProcess::ProcessState newState)

    This signal is emitted whenever the state of QProcess changes. The
    \a newState argument is the state QProcess changed to.
*/

/*!
    \fn void QProcess::finished(int exitCode)
    \obsolete
    \overload

    Use finished(int exitCode, QProcess::ExitStatus status) instead.
*/

/*!
    \fn void QProcess::finished(int exitCode, QProcess::ExitStatus exitStatus)

    This signal is emitted when the process finishes. \a exitCode is the exit
    code of the process, and \a exitStatus is the exit status.  After the
    process has finished, the buffers in QProcess are still intact. You can
    still read any data that the process may have written before it finished.

    \sa exitStatus()
*/

/*!
    \fn void QProcess::readyReadStandardOutput()

    This signal is emitted when the process has made new data
    available through its standard output channel (\c stdout). It is
    emitted regardless of the current \l{readChannel()}{read channel}.

    \sa readAllStandardOutput(), readChannel()
*/

/*!
    \fn void QProcess::readyReadStandardError()

    This signal is emitted when the process has made new data
    available through its standard error channel (\c stderr). It is
    emitted regardless of the current \l{readChannel()}{read
    channel}.

    \sa readAllStandardError(), readChannel()
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
    sequenceNumber = 0;
    exitCode = 0;
    exitStatus = QProcess::NormalExit;
    startupSocketNotifier = 0;
    deathNotifier = 0;
    notifier = 0;
    pipeWriter = 0;
    childStartedPipe[0] = INVALID_Q_PIPE;
    childStartedPipe[1] = INVALID_Q_PIPE;
    deathPipe[0] = INVALID_Q_PIPE;
    deathPipe[1] = INVALID_Q_PIPE;
    exitCode = 0;
    crashed = false;
    dying = false;
    emittedReadyRead = false;
    emittedBytesWritten = false;
#ifdef Q_WS_WIN
    pipeWriter = 0;
    processFinishedNotifier = 0;
#endif // Q_WS_WIN
#ifdef Q_OS_UNIX
    serial = 0;
#endif
}

/*! \internal
*/
QProcessPrivate::~QProcessPrivate()
{
    if (stdinChannel.process)
        stdinChannel.process->stdoutChannel.clear();
    if (stdoutChannel.process)
        stdoutChannel.process->stdinChannel.clear();
}

/*! \internal
*/
void QProcessPrivate::cleanup()
{
    processState = QProcess::NotRunning;
#ifdef Q_OS_WIN
    if (pid) {
        CloseHandle(pid->hThread);
        CloseHandle(pid->hProcess);
        delete pid;
        pid = 0;
    }
    if (processFinishedNotifier) {
        processFinishedNotifier->setEnabled(false);
        delete processFinishedNotifier;
        processFinishedNotifier = 0;
    }

#endif
    pid = 0;
    sequenceNumber = 0;
    dying = false;

    if (stdoutChannel.notifier) {
        stdoutChannel.notifier->setEnabled(false);
        delete stdoutChannel.notifier;
        stdoutChannel.notifier = 0;
    }
    if (stderrChannel.notifier) {
        stderrChannel.notifier->setEnabled(false);
        delete stderrChannel.notifier;
        stderrChannel.notifier = 0;
    }
    if (stdinChannel.notifier) {
        stdinChannel.notifier->setEnabled(false);
        delete stdinChannel.notifier;
        stdinChannel.notifier = 0;
    }
    if (startupSocketNotifier) {
        startupSocketNotifier->setEnabled(false);
        delete startupSocketNotifier;
        startupSocketNotifier = 0;
    }
    if (deathNotifier) {
        deathNotifier->setEnabled(false);
        delete deathNotifier;
        deathNotifier = 0;
    }
    if (notifier) {
        delete notifier;
        notifier = 0;
    }
    destroyPipe(stdoutChannel.pipe);
    destroyPipe(stderrChannel.pipe);
    destroyPipe(stdinChannel.pipe);
    destroyPipe(childStartedPipe);
    destroyPipe(deathPipe);
#ifdef Q_OS_UNIX
    serial = 0;
#endif
}

/*! \internal
*/
bool QProcessPrivate::_q_canReadStandardOutput()
{
    Q_Q(QProcess);
    qint64 available = bytesAvailableFromStdout();
    if (available == 0) {
        if (stdoutChannel.notifier)
            stdoutChannel.notifier->setEnabled(false);
        destroyPipe(stdoutChannel.pipe);
#if defined QPROCESS_DEBUG
        qDebug("QProcessPrivate::canReadStandardOutput(), 0 bytes available");
#endif
        return false;
    }

    char *ptr = outputReadBuffer.reserve(available);
    qint64 readBytes = readFromStdout(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, QLatin1String("Error reading from process")));
        emit q->error(processError);
#if defined QPROCESS_DEBUG
        qDebug("QProcessPrivate::canReadStandardOutput(), failed to read from the process");
#endif
        return false;
    }
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canReadStandardOutput(), read %d bytes from the process' output",
            int(readBytes));
#endif

    if (stdoutChannel.closed) {
        outputReadBuffer.chop(readBytes);
        return false;
    }

    outputReadBuffer.chop(available - readBytes);

    bool didRead = false;
    if (readBytes == 0) {
        if (stdoutChannel.notifier)
            stdoutChannel.notifier->setEnabled(false);
    } else if (processChannel == QProcess::StandardOutput) {
        didRead = true;
        if (!emittedReadyRead) {
            emittedReadyRead = true;
            emit q->readyRead();
            emittedReadyRead = false;
        }
    }
    emit q->readyReadStandardOutput();
    return didRead;
}

/*! \internal
*/
bool QProcessPrivate::_q_canReadStandardError()
{
    Q_Q(QProcess);
    qint64 available = bytesAvailableFromStderr();
    if (available == 0) {
        if (stderrChannel.notifier)
            stderrChannel.notifier->setEnabled(false);
        destroyPipe(stderrChannel.pipe);
        return false;
    }

    char *ptr = errorReadBuffer.reserve(available);
    qint64 readBytes = readFromStderr(ptr, available);
    if (readBytes == -1) {
        processError = QProcess::ReadError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, QLatin1String("Error reading from process")));
        emit q->error(processError);
        return false;
    }
    if (stderrChannel.closed) {
        errorReadBuffer.chop(readBytes);
        return false;
    }

    errorReadBuffer.chop(available - readBytes);

    bool didRead = false;
    if (readBytes == 0) {
        if (stderrChannel.notifier)
            stderrChannel.notifier->setEnabled(false);
    } else if (processChannel == QProcess::StandardError) {
        didRead = true;
        if (!emittedReadyRead) {
            emittedReadyRead = true;
            emit q->readyRead();
            emittedReadyRead = false;
        }
    }
    emit q->readyReadStandardError();
    return didRead;
}

/*! \internal
*/
bool QProcessPrivate::_q_canWrite()
{
    Q_Q(QProcess);
    if (stdinChannel.notifier)
        stdinChannel.notifier->setEnabled(false);

    if (writeBuffer.isEmpty()) {
#if defined QPROCESS_DEBUG
        qDebug("QProcessPrivate::canWrite(), not writing anything (empty write buffer).");
#endif
        return false;
    }

    qint64 written = writeToStdin(writeBuffer.readPointer(),
                                      writeBuffer.nextDataBlockSize());
    if (written < 0) {
        destroyPipe(stdinChannel.pipe);
        processError = QProcess::WriteError;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, QLatin1String("Error writing to process")));
#if defined QPROCESS_DEBUG
        qDebug("QProcessPrivate::canWrite(), failed to write (%s)", strerror(errno));
#endif
        emit q->error(processError);
        return false;
    }

#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::canWrite(), wrote %d bytes to the process input", int(written));
#endif

    writeBuffer.free(written);
    if (!emittedBytesWritten) {
        emittedBytesWritten = true;
        emit q->bytesWritten(written);
        emittedBytesWritten = false;
    }
    if (stdinChannel.notifier && !writeBuffer.isEmpty())
        stdinChannel.notifier->setEnabled(true);
    if (writeBuffer.isEmpty() && stdinChannel.closed)
        closeWriteChannel();
    return true;
}

/*! \internal
*/
bool QProcessPrivate::_q_processDied()
{
    Q_Q(QProcess);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::_q_processDied()");
#endif
#ifdef Q_OS_UNIX
    if (!waitForDeadChild())
        return false;
#endif
#ifdef Q_OS_WIN
    if (processFinishedNotifier)
        processFinishedNotifier->setEnabled(false);
#endif

    // the process may have died before it got a chance to report that it was
    // either running or stopped, so we will call _q_startupNotification() and
    // give it a chance to emit started() or error(FailedToStart).
    if (processState == QProcess::Starting) {
        if (!_q_startupNotification())
            return true;
    }

    if (dying) {
        // at this point we know the process is dead. prevent
        // reentering this slot recursively by calling waitForFinished()
        // or opening a dialog inside slots connected to the readyRead
        // signals emitted below.
        return true;
    }
    dying = true;
    
    // in case there is data in the pipe line and this slot by chance
    // got called before the read notifications, call these two slots
    // so the data is made available before the process dies.
    _q_canReadStandardOutput();
    _q_canReadStandardError();

    findExitCode();

    if (crashed) {
        exitStatus = QProcess::CrashExit;
        processError = QProcess::Crashed;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, QLatin1String("Process crashed")));
        emit q->error(processError);
    }

    cleanup();

    processState = QProcess::NotRunning;
    emit q->stateChanged(processState);
    emit q->finished(exitCode);
    emit q->finished(exitCode, exitStatus);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::_q_processDied() process is dead");
#endif
    return true;
}

/*! \internal
*/
bool QProcessPrivate::_q_startupNotification()
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
#ifdef Q_OS_UNIX
    // make sure the process manager removes this entry
    waitForDeadChild();
    findExitCode();
#endif
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
    if (stdinChannel.notifier) {
        stdinChannel.notifier->setEnabled(false);
        delete stdinChannel.notifier;
        stdinChannel.notifier = 0;
    }
#ifdef Q_OS_WIN
    // ### Find a better fix, feeding the process little by little
    // instead.
    flushPipeWriter();
#endif
    destroyPipe(stdinChannel.pipe);
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
    Destructs the QProcess object, i.e., killing the process.

    Note that this function will not return until the process is
    terminated.
*/
QProcess::~QProcess()
{
    Q_D(QProcess);
    if (d->processState != NotRunning) {
        qWarning("QProcess: Destroyed while process is still running.");
        kill();
        waitForFinished();
    }
#ifdef Q_OS_UNIX
    // make sure the process manager removes this entry
    d->findExitCode();
#endif
    d->cleanup();
}

/*!
    \obsolete
    Returns the read channel mode of the QProcess. This function is
    equivalent to processChannelMode()

    \sa processChannelMode()
*/
QProcess::ProcessChannelMode QProcess::readChannelMode() const
{
    return processChannelMode();
}

/*!
    \obsolete

    Use setProcessChannelMode(\a mode) instead.

    \sa setProcessChannelMode()
*/
void QProcess::setReadChannelMode(ProcessChannelMode mode)
{
    setProcessChannelMode(mode);
}

/*!
    \since 4.2

    Returns the channel mode of the QProcess standard output and
    standard error channels.

    \sa setReadChannelMode(), ProcessChannelMode, setReadChannel()
*/
QProcess::ProcessChannelMode QProcess::processChannelMode() const
{
    Q_D(const QProcess);
    return d->processChannelMode;
}

/*!
    \since 4.2

    Sets the channel mode of the QProcess standard output and standard
    error channels to the \a mode specified.
    This mode will be used the next time start() is called. For example:

    \code
        QProcess builder;
        builder.setProcessChannelMode(QProcess::MergedChannels);
        builder.start("make", QStringList() << "-j2");

        if (!builder.waitForFinished())
            qDebug() << "Make failed:" << builder.errorString();
        else
            qDebug() << "Make output:" << builder.readAll();
    \endcode

    \sa readChannelMode(), ProcessChannelMode, setReadChannel()
*/
void QProcess::setProcessChannelMode(ProcessChannelMode mode)
{
    Q_D(QProcess);
    d->processChannelMode = mode;
}

/*!
    Returns the current read channel of the QProcess.

    \sa setReadChannel()
*/
QProcess::ProcessChannel QProcess::readChannel() const
{
    Q_D(const QProcess);
    return d->processChannel;
}

/*!
    Sets the current read channel of the QProcess to the given \a
    channel. The current input channel is used by the functions
    read(), readAll(), readLine(), and getChar(). It also determines
    which channel triggers QProcess to emit readyRead().

    Changing the read channel will clear the unget buffer.

    \sa readChannel()
*/
void QProcess::setReadChannel(ProcessChannel channel)
{
    Q_D(QProcess);
    if (d->processChannel != channel)
        d->buffer.clear();
    d->processChannel = channel;
}

/*!
    Closes the read channel \a channel. After calling this function,
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
        d->stdoutChannel.closed = true;
    else
        d->stderrChannel.closed = true;
}

/*!
    Schedules the write channel of QProcess to be closed. The channel
    will close once all data has been written to the process. After
    calling this function, any attempts to write to the process will
    fail.

    Closing the write channel is necessary for programs that read
    input data until the channel has been closed. For example, the
    program "more" is used to display text data in a console on both
    Unix and Windows. But it will not display the text data until
    QProcess's write channel has been closed. Example:

    \code
        QProcess more;
        more.start("more");
        more.write("Text to display");
        more.closeWriteChannel();
        // QProcess will emit readyRead() once "more" starts printing
    \endcode

    The write channel is implicitly opened when start() is called.

    \sa closeReadChannel()
*/
void QProcess::closeWriteChannel()
{
    Q_D(QProcess);
    d->stdinChannel.closed = true; // closing
    if (d->writeBuffer.isEmpty())
        d->closeWriteChannel();
}

/*!
    \since 4.2

    Redirects the process' standard input to the file indicated by \a
    fileName. When an input redirection is in place, the QProcess
    object will be in read-only mode (calling write() will result in
    error).

    If the file \a fileName does not exist at the moment start() is
    called or is not readable, starting the process will fail.

    Calling setStandardInputFile() after the process has started has no
    effect.

    \sa setStandardOutputFile(), setStandardErrorFile(),
        setStandardOutputProcess()
*/
void QProcess::setStandardInputFile(const QString &fileName)
{
    Q_D(QProcess);
    d->stdinChannel = fileName;
}

/*!
    \since 4.2

    Redirects the process' standard output to the file \a
    fileName. When the redirection is in place, the standard output
    read channel is closed: reading from it using read() will always
    fail, as will readAllStandardOutput().

    If the file \a fileName doesn't exist at the moment start() is
    called, it will be created. If it cannot be created, the starting
    will fail.

    If the file exists and \a mode is QIODevice::Truncate, the file
    will be truncated. Otherwise (if \a mode is QIODevice::Append),
    the file will be appended to.

    Calling setStandardOutputFile() after the process has started has
    no effect.

    \sa setStandardInputFile(), setStandardErrorFile(),
        setStandardOutputProcess()
*/
void QProcess::setStandardOutputFile(const QString &fileName, OpenMode mode)
{
    Q_ASSERT(mode == Append || mode == Truncate);
    Q_D(QProcess);

    d->stdoutChannel = fileName;
    d->stdoutChannel.append = mode == Append;
}

/*!
    \since 4.2

    Redirects the process' standard error to the file \a
    fileName. When the redirection is in place, the standard error
    read channel is closed: reading from it using read() will always
    fail, as will readAllStandardError(). The file will be appended to
    if \a mode is Append, otherwise, it will be truncated.

    See setStandardOutputFile() for more information on how the file
    is opened.

    Note: if setProcessChannelMode() was called with an argument of
    QProcess::MergedChannels, this function has no effect.

    \sa setStandardInputFile(), setStandardOutputFile(),
        setStandardOutputProcess()
*/
void QProcess::setStandardErrorFile(const QString &fileName, OpenMode mode)
{
    Q_ASSERT(mode == Append || mode == Truncate);
    Q_D(QProcess);

    d->stderrChannel = fileName;
    d->stderrChannel.append = mode == Append;
}

/*!
    \since 4.2

    Pipes the standard output stream of this process to the \a
    destination process' standard input.

    The following shell command:
    \code
        command1 | command2
    \endcode

    Can be accomplished with QProcesses with the following code:
    \code
        QProcess process1;
        QProcess process2;

        process1.setStandardOutputProcess(process2);

        process1.start("command1");
        process2.start("command2");
    \endcode
*/
void QProcess::setStandardOutputProcess(QProcess *destination)
{
    QProcessPrivate *dfrom = d_func();
    QProcessPrivate *dto = destination->d_func();
    dfrom->stdoutChannel.pipeTo(dto);
    dto->stdinChannel.pipeFrom(dfrom);
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

    \sa workingDirectory(), start()
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

    This function operates on the current read channel.

    \sa readChannel(), setReadChannel()
*/
bool QProcess::canReadLine() const
{
    Q_D(const QProcess);
    const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                    ? &d->errorReadBuffer
                                    : &d->outputReadBuffer;
    return readBuffer->canReadLine() || QIODevice::canReadLine();
}

/*!
    Closes all communication with the process. After calling this
    function, QProcess will no longer emit readyRead(), and data can no
    longer be read or written.
*/
void QProcess::close()
{
    emit aboutToClose();
    while (waitForBytesWritten(-1))
        ;
    kill();
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
    return QIODevice::atEnd() && (!isOpen() || readBuffer->isEmpty());
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
    return readBuffer->size() + QIODevice::bytesAvailable();
}

/*! \reimp
*/
qint64 QProcess::bytesToWrite() const
{
    Q_D(const QProcess);
    qint64 size = d->writeBuffer.size();
#ifdef Q_OS_WIN
    size += d->pipeWriterBytesToWrite();
#endif
    return size;
}

/*!
    Returns the type of error that occurred last.

    \sa state()
*/
QProcess::ProcessError QProcess::error() const
{
    Q_D(const QProcess);
    return d->processError;
}

/*!
    Returns the current state of the process.

    \sa stateChanged(), error()
*/
QProcess::ProcessState QProcess::state() const
{
    Q_D(const QProcess);
    return d->processState;
}

/*!
    Sets the environment that QProcess will use when starting a process to the
    \a environment specified which consists of a list of key=value pairs.

    For example, the following code adds the \c{C:\\BIN} directory to the list of
    executable paths (\c{PATHS}) on Windows:

    \quotefromfile snippets/qprocess-environment/main.cpp
    \skipto QProcess process;
    \printuntil process.start

    \sa environment(), systemEnvironment()
*/
void QProcess::setEnvironment(const QStringList &environment)
{
    Q_D(QProcess);
    d->environment = environment;
}

/*!
    Returns the environment that QProcess will use when starting a
    process, or an empty QStringList if no environment has been set
    using setEnvironment(). If no environment has been set, the
    environment of the calling process will be used.

    \sa setEnvironment(), systemEnvironment()
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

    \sa started(), waitForReadyRead(), waitForBytesWritten(), waitForFinished()
*/
bool QProcess::waitForStarted(int msecs)
{
    Q_D(QProcess);
    if (d->processState == QProcess::Starting) {
        if (!d->waitForStarted(msecs))
            return false;
        d->processState = QProcess::Running;
        emit started();
    }
    return d->processState == QProcess::Running;
}

/*! \reimp
*/
bool QProcess::waitForReadyRead(int msecs)
{
    Q_D(QProcess);

    if (d->processState == QProcess::NotRunning)
        return false;
    if (d->processChannel == QProcess::StandardOutput && d->stdoutChannel.closed)
        return false;
    if (d->processChannel == QProcess::StandardError && d->stderrChannel.closed)
        return false;
    return d->waitForReadyRead(msecs);
}

/*! \reimp
*/
bool QProcess::waitForBytesWritten(int msecs)
{
    Q_D(QProcess);
    if (d->processState == QProcess::NotRunning)
        return false;
    if (d->processState == QProcess::Starting) {
        QTime stopWatch;
        stopWatch.start();
        bool started = waitForStarted(msecs);
        if (!started)
            return false;
        if (msecs != -1)
            msecs -= stopWatch.elapsed();
    }

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

    \sa finished(), waitForStarted(), waitForReadyRead(), waitForBytesWritten()
*/
bool QProcess::waitForFinished(int msecs)
{
    Q_D(QProcess);
    if (d->processState == QProcess::NotRunning)
        return false;
    if (d->processState == QProcess::Starting) {
        QTime stopWatch;
        stopWatch.start();
        bool started = waitForStarted(msecs);
        if (!started)
            return false;
        if (msecs != -1)
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
  This function is called in the child process context just before the
    program is executed on Unix or Mac OS X (i.e., after \e fork(), but before
    \e execve()). Reimplement this function to do last minute initialization
    of the child process. Example:

    \code
        class SandboxProcess : public QProcess
        {
            ...
         protected:
             void setupChildProcess();
            ...
        };

        void SandboxProcess::setupChildProcess()
        {
            // Drop all privileges in the child process, and enter
            // a chroot jail.
        #if defined Q_OS_UNIX
            ::setgroups(0, 0);
            ::chroot("/etc/safe");
            ::chdir("/");
            ::setgid(safeGid);
            ::setuid(safeUid);
            ::umask(0);
        #endif
        }

    \endcode

    \warning This function is called by QProcess on Unix and Mac OS X
    only. On Windows, it is not called.
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
                   data, qt_prettyDebug(data, 1, maxlen).constData(), 1);
#endif
            return -1;
        }
        *data = (char) c;
#if defined QPROCESS_DEBUG
        qDebug("QProcess::readData(%p \"%s\", %d) == 1",
               data, qt_prettyDebug(data, 1, maxlen).constData(), 1);
#endif
        return 1;
    }

    qint64 bytesToRead = qint64(qMin(readBuffer->size(), (int)maxlen));
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        const char *ptr = readBuffer->readPointer();
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

    if (d->stdinChannel.closed) {
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%p \"%s\", %lld) == 0 (write channel closing)",
           data, qt_prettyDebug(data, len, 16).constData(), len);
#endif
        return 0;
    }

    if (len == 1) {
        d->writeBuffer.putChar(*data);
        if (d->stdinChannel.notifier)
            d->stdinChannel.notifier->setEnabled(true);
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%p \"%s\", %lld) == 1 (written to buffer)",
           data, qt_prettyDebug(data, len, 16).constData(), len);
#endif
        return 1;
    }

    char *dest = d->writeBuffer.reserve(len);
    memcpy(dest, data, len);
    if (d->stdinChannel.notifier)
        d->stdinChannel.notifier->setEnabled(true);
#if defined QPROCESS_DEBUG
    qDebug("QProcess::writeData(%p \"%s\", %lld) == %lld (written to buffer)",
           data, qt_prettyDebug(data, len, 16).constData(), len, len);
#endif
    return len;
}

/*!
    Regardless of the current read channel, this function returns all
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
    Regardless of the current read channel, this function returns all
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

    On Windows, arguments that contain spaces are wrapped in quotes.

    Note: processes are started asynchronously, which means the started()
    and error() signals may be delayed. Call waitForStarted() to make
    sure the process has started (or has failed to start) and those signals
    have been emitted.

    \sa pid(), started(), waitForStarted()
*/
void QProcess::start(const QString &program, const QStringList &arguments, OpenMode mode)
{
    Q_D(QProcess);
    if (d->processState != NotRunning) {
        qWarning("QProcess::start: Process is already running");
        return;
    }

#if defined QPROCESS_DEBUG
    qDebug() << "QProcess::start(" << program << "," << arguments << "," << mode << ")";
#endif

    d->outputReadBuffer.clear();
    d->errorReadBuffer.clear();

    if (d->stdinChannel.type != QProcessPrivate::Channel::Normal)
        mode &= ~WriteOnly;     // not open for writing
    if (d->stdoutChannel.type != QProcessPrivate::Channel::Normal &&
        (d->stderrChannel.type != QProcessPrivate::Channel::Normal ||
         d->processChannelMode == MergedChannels))
        mode &= ~ReadOnly;      // not open for reading
    if (mode == 0)
        mode = Unbuffered;
    setOpenMode(mode);

    d->stdinChannel.closed = false;
    d->stdoutChannel.closed = false;
    d->stderrChannel.closed = false;

    d->program = program;
    d->arguments = arguments;

    d->exitCode = 0;
    d->exitStatus = NormalExit;
    d->processError = QProcess::UnknownError;
    d->errorString.clear();
    d->startProcess();
}


static QStringList parseCombinedArgString(const QString &program)
{
    QStringList args;
    QString tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < program.size(); ++i) {
        if (program.at(i) == QLatin1Char('"')) {
            ++quoteCount;
            if (quoteCount == 3) {
                // third consecutive quote
                quoteCount = 0;
                tmp += program.at(i);
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && program.at(i).isSpace()) {
            if (!tmp.isEmpty()) {
                args += tmp;
                tmp.clear();
            }
        } else {
            tmp += program.at(i);
        }
    }
    if (!tmp.isEmpty())
        args += tmp;

    return args;
}

/*!
    \overload

    Starts the program \a program in a new process. \a program is a
    single string of text containing both the program name and its
    arguments. The arguments are separated by one or more
    spaces. For example:

    \code
        QProcess process;
        process.start("del /s *.txt");
        // same as process.start("del", QStringList() << "/s" << "*.txt");
        ...
    \endcode

    The \a program string can also contain quotes, to ensure that arguments
    containing spaces are correctly supplied to the new process. For example:

    \code
        QProcess process;
        process.start("dir \"My Documents\"");
    \endcode

    Note that, on Windows, quotes need to be both escaped and quoted.
    For example, the above code would be specified in the following
    way to ensure that \c{"My Documents"} is used as the argument to
    the \c dir executable:

    \code
        QProcess process;
        process.start("dir \"\"\"My Documents\"\"\"");
    \endcode

    The OpenMode is set to \a mode.
*/
void QProcess::start(const QString &program, OpenMode mode)
{
    QStringList args = parseCombinedArgString(program);

    QString prog = args.first();
    args.removeFirst();

    start(prog, args, mode);
}

/*!
    Attempts to terminate the process.

    The process may not exit as a result of calling this function (it is given
    the chance to prompt the user for any unsaved files, etc).

    On Windows, terminate() posts a WM_CLOSE message to all toplevel windows
    of the process and then to the main thread of the process itself. On Unix
    and Mac OS X the SIGTERM signal is sent.

    Console applications on Windows that do not run an event loop, or whose
    event loop does not handle the WM_CLOSE message, can only be terminated by
    calling kill().

    \sa kill()
*/
void QProcess::terminate()
{
    Q_D(QProcess);
    d->terminateProcess();
}

/*!
    Kills the current process, causing it to exit immediately.

    On Windows, kill() uses TerminateProcess, and on Unix and Mac OS X, the
    SIGKILL signal is sent to the process.

    \sa terminate()
*/
void QProcess::kill()
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
    \since 4.1

    Returns the exit status of the last process that finished.

    On Windows, if the process was terminated with TerminateProcess()
    from another application this function will still return NormalExit
    unless the exit code is less than 0.
*/
QProcess::ExitStatus QProcess::exitStatus() const
{
    Q_D(const QProcess);
    return d->exitStatus;
}

/*!
    Starts the program \a program with the arguments \a arguments in a
    new process, waits for it to finish, and then returns the exit
    code of the process. Any data the new process writes to the
    console is forwarded to the calling process.

    The environment and working directory are inherited by the calling
    process.

    On Windows, arguments that contain spaces are wrapped in quotes.
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

    On Windows, arguments that contain spaces are wrapped in quotes.
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

    The \a program string can also contain quotes, to ensure that arguments
    containing spaces are correctly supplied to the new process.
*/
bool QProcess::startDetached(const QString &program)
{
    QStringList args = parseCombinedArgString(program);

    QString prog = args.first();
    args.removeFirst();

    return QProcessPrivate::startDetached(prog, args);
}

#ifdef Q_OS_MAC
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#elif !defined(Q_OS_WIN)
  extern char **environ;
#endif

/*!
    \since 4.1

    Returns the environment of the calling process as a list of
    key=value pairs. Example:

    \code
        QStringList environment = QProcess::systemEnvironment();
        // environment = {"PATH=/usr/bin:/usr/local/bin",
                          "USER=greg", "HOME=/home/greg"}
    \endcode

    \sa environment(), setEnvironment()
*/
QStringList QProcess::systemEnvironment()
{
    QStringList tmp;
    char *entry = 0;
    int count = 0;
    while ((entry = environ[count++]))
        tmp << QString::fromLocal8Bit(entry);
    return tmp;
}

/*!
    \typedef Q_PID
    \relates QProcess

    Typedef for the identifiers used to represent processes on the underlying
    platform. On Unix, this corresponds to \l qint64; on Windows, it
    corresponds to \c{_PROCESS_INFORMATION*}.

    \sa QProcess::pid()
*/

#include "moc_qprocess.cpp"

#endif // QT_NO_PROCESS
