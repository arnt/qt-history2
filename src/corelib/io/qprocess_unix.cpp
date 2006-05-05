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
#include "qdebug.h"

#ifndef QT_NO_PROCESS

#if defined QPROCESS_DEBUG
#include "qstring.h"
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

#include "qplatformdefs.h"

#include "qprocess.h"
#include "qprocess_p.h"

#ifdef Q_OS_MAC
#include <private/qcore_mac_p.h>
#endif

#include <qabstracteventdispatcher.h>
#include <private/qcoreapplication_p.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qmap.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qsocketnotifier.h>
#include <qthread.h>

#include <errno.h>
#include <stdlib.h>

static int qt_qprocess_deadChild_pipe[2];
static void (*qt_sa_old_sigchld_handler)(int) = 0;
static void qt_sa_sigchld_handler(int signum)
{
    ::write(qt_qprocess_deadChild_pipe[1], "", 1);
#if defined (QPROCESS_DEBUG)
    fprintf(stderr, "*** SIGCHLD\n");
#endif

    if (qt_sa_old_sigchld_handler && qt_sa_old_sigchld_handler != SIG_IGN)
        qt_sa_old_sigchld_handler(signum);
}

struct QProcessInfo {
    QProcess *process;
    int deathPipe;
    int exitResult;
    pid_t pid;
    int serialNumber;
};

class QProcessManager : public QThread
{
    Q_OBJECT
public:
    QProcessManager();
    ~QProcessManager();

    void run();
    void catchDeadChildren();
    void add(pid_t pid, QProcess *process);
    void remove(QProcess *process);
    void lock();
    void unlock();

private:
    QMutex mutex;
    QMap<int, QProcessInfo *> children;
};

Q_GLOBAL_STATIC(QProcessManager, processManager)

QProcessManager::QProcessManager()
{
#if defined (QPROCESS_DEBUG)
    qDebug() << "QProcessManager::QProcessManager()";
#endif
    // initialize the dead child pipe and make it non-blocking. in the
    // extremely unlikely event that the pipe fills up, we do not under any
    // circumstances want to block.
    ::pipe(qt_qprocess_deadChild_pipe);
    ::fcntl(qt_qprocess_deadChild_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(qt_qprocess_deadChild_pipe[1], F_SETFD, FD_CLOEXEC);
    ::fcntl(qt_qprocess_deadChild_pipe[0], F_SETFL,
	    ::fcntl(qt_qprocess_deadChild_pipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(qt_qprocess_deadChild_pipe[1], F_SETFL,
	    ::fcntl(qt_qprocess_deadChild_pipe[1], F_GETFL) | O_NONBLOCK);

    // set up the SIGCHLD handler, which writes a single byte to the dead
    // child pipe every time a child dies.
    struct sigaction oldAction;
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = qt_sa_sigchld_handler;
    action.sa_flags = SA_NOCLDSTOP;
    ::sigaction(SIGCHLD, &action, &oldAction);
    if (oldAction.sa_handler != qt_sa_sigchld_handler)
	qt_sa_old_sigchld_handler = oldAction.sa_handler;
}

QProcessManager::~QProcessManager()
{
    // notify the thread that we're shutting down.
    ::write(qt_qprocess_deadChild_pipe[1], "@", 1);
    ::close(qt_qprocess_deadChild_pipe[1]);
    wait();

    // on certain unixes, closing the reading end of the pipe will cause
    // select in run() to block forever, rather than return with EBADF.
    ::close(qt_qprocess_deadChild_pipe[0]);

    qt_qprocess_deadChild_pipe[0] = -1;
    qt_qprocess_deadChild_pipe[1] = -1;

    qDeleteAll(children.values());
    children.clear();
}

void QProcessManager::run()
{
    forever {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(qt_qprocess_deadChild_pipe[0], &readset);

#if defined (QPROCESS_DEBUG)
        qDebug() << "QProcessManager::run() waiting for children to die";
#endif

        // block forever, or until activity is detected on the dead child
        // pipe. the only other peers are the SIGCHLD signal handler, and the
        // QProcessManager destructor.
        int nselect = select(qt_qprocess_deadChild_pipe[0] + 1, &readset, 0, 0, 0);
        if (nselect < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        // empty only one byte from the pipe, even though several SIGCHLD
        // signals may have been delivered in the meantime, to avoid race
        // conditions.
        char c;
        if (::read(qt_qprocess_deadChild_pipe[0], &c, 1) < 0 || c == '@')
            break;

        // catch any and all children that we can.
        catchDeadChildren();
    }
}

void QProcessManager::catchDeadChildren()
{
    QMutexLocker locker(&mutex);

    // try to catch all children whose pid we have registered, and whose
    // deathPipe is still valid (i.e, we have not already notified it).
    QMap<int, QProcessInfo *>::Iterator it = children.begin();
    while (it != children.end()) {
        // notify all children that they may have died. they need to run
        // waitpid() in their own thread.
        QProcessInfo *info = it.value();
        ::write(info->deathPipe, "", 1);

#if defined (QPROCESS_DEBUG)
        qDebug() << "QProcessManager::run() sending death notice to" << info->process;
#endif
        ++it;
    }
}

static QBasicAtomic idCounter = Q_ATOMIC_INIT(1);
static int qt_qprocess_nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
}

void QProcessManager::add(pid_t pid, QProcess *process)
{
#if defined (QPROCESS_DEBUG)
    qDebug() << "QProcessManager::add() adding pid" << pid << "process" << process;
#endif

    // insert a new info structure for this process
    QProcessInfo *info = new QProcessInfo;
    info->process = process;
    info->deathPipe = process->d_func()->deathPipe[1];
    info->exitResult = 0;
    info->pid = pid;

    int serial = qt_qprocess_nextId();
    process->d_func()->serial = serial;
    children.insert(serial, info);
}

void QProcessManager::remove(QProcess *process)
{
    QMutexLocker locker(&mutex);

    int serial = process->d_func()->serial;
    QProcessInfo *info = children.value(serial);
    if (!info)
        return;

#if defined (QPROCESS_DEBUG)
    qDebug() << "QProcessManager::remove() removing pid" << info->pid << "process" << info->process;
#endif
    
    children.remove(serial);
    delete info;
}

void QProcessManager::lock()
{
    mutex.lock();
}

void QProcessManager::unlock()
{
    mutex.unlock();
}

static void qt_create_pipe(int *pipe)
{
    if (pipe[0] != -1)
        ::close(pipe[0]);
    if (pipe[1] != -1)
        ::close(pipe[1]);
#ifdef Q_OS_IRIX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, pipe) == -1) {
        qWarning("QProcessPrivate::createPipe: Cannot create pipe %p: %s",
                 pipe, qPrintable(qt_error_string(errno)));
    }
#else
    if (::pipe(pipe) != 0) {
        qWarning("QProcessPrivate::createPipe: Cannot create pipe %p: %s",
                 pipe, qPrintable(qt_error_string(errno)));
    }
#endif
}

void QProcessPrivate::destroyPipe(int *pipe)
{
    if (pipe[1] != -1) {
        ::close(pipe[1]);
        pipe[1] = -1;
    }
    if (pipe[0] != -1) {
        ::close(pipe[0]);
        pipe[0] = -1;
    }
}

void QProcessPrivate::startProcess()
{
    Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::startProcess()");
#endif

    processManager()->start();

    // Initialize pipes
    qt_create_pipe(childStartedPipe);
    if (QAbstractEventDispatcher::instance(q->thread())) {
        startupSocketNotifier = new QSocketNotifier(childStartedPipe[0],
                                                    QSocketNotifier::Read, q);
        QObject::connect(startupSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_startupNotification()));
    }

    qt_create_pipe(deathPipe);
    ::fcntl(deathPipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(deathPipe[1], F_SETFD, FD_CLOEXEC);
    if (QAbstractEventDispatcher::instance(q->thread())) {
        deathNotifier = new QSocketNotifier(deathPipe[0],
                                            QSocketNotifier::Read, q);
        QObject::connect(deathNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_processDied()));
    }

    qt_create_pipe(writePipe);
    if (QAbstractEventDispatcher::instance(q->thread())) {
        writeSocketNotifier = new QSocketNotifier(writePipe[1],
                                                  QSocketNotifier::Write, q);
        QObject::connect(writeSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_canWrite()));
        writeSocketNotifier->setEnabled(false);
    }

    qt_create_pipe(standardReadPipe);
    qt_create_pipe(errorReadPipe);

    if (QAbstractEventDispatcher::instance(q->thread())) {
        standardReadSocketNotifier = new QSocketNotifier(standardReadPipe[0],
                                                         QSocketNotifier::Read,
                                                         q);
        QObject::connect(standardReadSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_canReadStandardOutput()));

        errorReadSocketNotifier = new QSocketNotifier(errorReadPipe[0],
                                                      QSocketNotifier::Read,
                                                      q);
        QObject::connect(errorReadSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_canReadStandardError()));
    }

    // Start the process (platform dependent)
    processState = QProcess::Starting;
    emit q->stateChanged(processState);

    QByteArray encodedProg = QFile::encodeName(program);
    processManager()->lock();
    pid_t childPid = fork();
    if (childPid < 0) {
        // Cleanup, report error and return
        processManager()->unlock();
        processState = QProcess::NotRunning;
        emit q->stateChanged(processState);
        processError = QProcess::FailedToStart;
        q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Resource error (fork failure)")));
        emit q->error(processError);
        cleanup();
        return;
    }

    if (childPid == 0) {
        execChild(encodedProg);
        ::_exit(-1);
    }
    processManager()->add(childPid, q);
    pid = Q_PID(childPid);
    processManager()->unlock();

    // parent
    ::close(childStartedPipe[1]);
    ::close(standardReadPipe[1]);
    ::close(errorReadPipe[1]);
    ::close(writePipe[0]);

    childStartedPipe[1] = -1;
    standardReadPipe[1] = -1;
    errorReadPipe[1] = -1;
    writePipe[0] = -1;

    // make all pipes non-blocking
    ::fcntl(deathPipe[0], F_SETFL, ::fcntl(deathPipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(standardReadPipe[0], F_SETFL, ::fcntl(standardReadPipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(errorReadPipe[0], F_SETFL, ::fcntl(errorReadPipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(writePipe[1], F_SETFL, ::fcntl(writePipe[1], F_GETFL) | O_NONBLOCK);
}

void QProcessPrivate::execChild(const QByteArray &programName)
{
    ::signal(SIGPIPE, SIG_DFL);         // reset the signal that we ignored
    
    QByteArray encodedProgramName = programName;
    Q_Q(QProcess);

    // create argument list with right number of elements, and set the
    // final one to 0.
    char **argv = new char *[arguments.count() + 2];
    argv[arguments.count() + 1] = 0;

    // allow invoking of .app bundles on the Mac.
#ifdef Q_OS_MAC
    QFileInfo fileInfo(encodedProgramName);
    if (encodedProgramName.endsWith(".app") && fileInfo.isDir()) {
        QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0,
                                                          QCFString(fileInfo.absoluteFilePath()),
                                                          kCFURLPOSIXPathStyle, true);
        QCFType<CFBundleRef> bundle = CFBundleCreate(0, url);
        url = CFBundleCopyExecutableURL(bundle);
        if (url) {
            QCFString str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            encodedProgramName += "/Contents/MacOS/" + static_cast<QString>(str).toUtf8();
        }
    }
#endif

    // add the program name
    argv[0] = ::strdup(encodedProgramName.constData());

    // add every argument to the list
    for (int i = 0; i < arguments.count(); ++i) {
        QString arg = arguments.at(i);
#ifdef Q_OS_MAC
        argv[i + 1] = ::strdup(arg.toUtf8().constData());
#else
        argv[i + 1] = ::strdup(arg.toLocal8Bit().constData());
#endif
    }

    // on all pipes, close the end that we don't use
    ::close(standardReadPipe[0]);
    ::close(errorReadPipe[0]);
    ::close(writePipe[1]);

    // copy the stdin socket
    ::dup2(writePipe[0], fileno(stdin));
    ::close(writePipe[0]);

    // copy the stdout and stderr if asked to
    if (processChannelMode != QProcess::ForwardedChannels) {
        ::dup2(standardReadPipe[1], fileno(stdout));
        ::dup2(errorReadPipe[1], fileno(stderr));
        ::close(standardReadPipe[1]);
        ::close(errorReadPipe[1]);

        // merge stdout and stderr if asked to
        if (processChannelMode == QProcess::MergedChannels)
            ::dup2(fileno(stdout), fileno(stderr));
    }

    // make sure this fd is closed if execvp() succeeds
    ::close(childStartedPipe[0]);
    ::fcntl(childStartedPipe[1], F_SETFD, FD_CLOEXEC);

    // enter the working directory
    if (!workingDirectory.isEmpty())
        ::chdir(QFile::encodeName(workingDirectory).constData());

    // this is a virtual call, and it base behavior is to do nothing.
    q->setupChildProcess();

    // execute the process
    if (environment.isEmpty()) {
        ::execvp(argv[0], argv);
    } else {
        // if LD_LIBRARY_PATH exists in the current environment, but
        // not in the environment list passed by the programmer, then
        // copy it over.
#if defined(Q_OS_MAC)
        static const char libraryPath[] = "DYLD_LIBRARY_PATH";
#else
        static const char libraryPath[] = "LD_LIBRARY_PATH";
#endif
        const QString libraryPathString = QLatin1String(libraryPath);
        QStringList matches = environment.filter(
                QRegExp(QLatin1Char('^') + libraryPathString + QLatin1Char('=')));
        const QString envLibraryPath = QString::fromLocal8Bit(::getenv(libraryPath));
        if (matches.isEmpty() && !envLibraryPath.isEmpty()) {
            QString entry = libraryPathString;
            entry += QLatin1Char('=');
            entry += envLibraryPath;
            environment << libraryPathString + QLatin1Char('=') + envLibraryPath;
        }

        char **envp = new char *[environment.count() + 1];
        envp[environment.count()] = 0;

        for (int j = 0; j < environment.count(); ++j) {
            QString item = environment.at(j);
            envp[j] = ::strdup(item.toLocal8Bit().constData());
        }

        if (!encodedProgramName.contains("/")) {
            const QString path = QString::fromLocal8Bit(::getenv("PATH"));
            if (!path.isEmpty()) {
                QStringList pathEntries = path.split(QLatin1Char(':'));
                for (int k = 0; k < pathEntries.size(); ++k) {
                    QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                    if (!tmp.endsWith('/')) tmp += '/';
                    tmp += encodedProgramName;
                    argv[0] = tmp.data();
#if defined (QPROCESS_DEBUG)
                    fprintf(stderr, "QProcessPrivate::execChild() searching / starting %s\n", argv[0]);
#endif
                    ::execve(argv[0], argv, envp);
                }
            }
        } else {
#if defined (QPROCESS_DEBUG)
            fprintf(stderr, "QProcessPrivate::execChild() starting %s\n", argv[0]);
#endif
            ::execve(argv[0], argv, envp);
        }
    }

    // notify failure
#if defined (QPROCESS_DEBUG)
    fprintf(stderr, "QProcessPrivate::execChild() failed, notifying parent process\n");
#endif
    ::write(childStartedPipe[1], "", 1);
    ::close(childStartedPipe[1]);
    childStartedPipe[1] = -1;
}

bool QProcessPrivate::processStarted()
{
    char c;
    int i = ::read(childStartedPipe[0], &c, 1);
    if (startupSocketNotifier) {
        startupSocketNotifier->setEnabled(false);
        delete startupSocketNotifier;
        startupSocketNotifier = 0;
    }
    ::close(childStartedPipe[0]);
    childStartedPipe[0] = -1;

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::processStarted() == %s", i <= 0 ? "true" : "false");
#endif
    return i <= 0;
}

qint64 QProcessPrivate::bytesAvailableFromStdout() const
{
    size_t nbytes = 0;
    qint64 available = 0;
    if (::ioctl(standardReadPipe[0], FIONREAD, (char *) &nbytes) >= 0)
        available = (qint64) *((int *) &nbytes);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::bytesAvailableFromStdout() == %lld", available);
#endif
    return available;
}

qint64 QProcessPrivate::bytesAvailableFromStderr() const
{
    size_t nbytes = 0;
    qint64 available = 0;
    if (::ioctl(errorReadPipe[0], FIONREAD, (char *) &nbytes) >= 0)
        available = (qint64) *((int *) &nbytes);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::bytesAvailableFromStderr() == %lld", available);
#endif
    return available;
}

qint64 QProcessPrivate::readFromStdout(char *data, qint64 maxlen)
{
    qint64 bytesRead = qint64(::read(standardReadPipe[0], data, maxlen));
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::readFromStdout(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, bytesRead, 16).constData(), maxlen, bytesRead);
#endif
    return bytesRead;
}

qint64 QProcessPrivate::readFromStderr(char *data, qint64 maxlen)
{
    qint64 bytesRead = qint64(::read(errorReadPipe[0], data, maxlen));
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::readFromStderr(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, bytesRead, 16).constData(), maxlen, bytesRead);
#endif
    return bytesRead;
}

static void qt_ignore_sigpipe()
{
    // Set to ignore SIGPIPE once only.
    static QBasicAtomic atom = Q_ATOMIC_INIT(0);
    if (atom.testAndSet(0, 1)) {
        struct sigaction noaction;
        memset(&noaction, 0, sizeof(noaction));
        noaction.sa_handler = SIG_IGN;
        ::sigaction(SIGPIPE, &noaction, 0);
    }
}

qint64 QProcessPrivate::writeToStdin(const char *data, qint64 maxlen)
{
    qt_ignore_sigpipe();

    qint64 written = qint64(::write(writePipe[1], data, maxlen));
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::writeToStdin(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, maxlen, 16).constData(), maxlen, written);
#endif
    return written;
}

void QProcessPrivate::terminateProcess()
{
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::killProcess()");
#endif
    if (pid)
        ::kill(pid_t(pid), SIGTERM);
}

void QProcessPrivate::killProcess()
{
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::killProcess()");
#endif
    if (pid)
        ::kill(pid_t(pid), SIGKILL);
}

static int qt_native_select(fd_set *fdread, fd_set *fdwrite, int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    int ret;
    do {
        ret = select(FD_SETSIZE, fdread, fdwrite, 0, timeout < 0 ? 0 : &tv);
    } while (ret < 0 && (errno == EINTR));
    return ret;
}

/*
   Returns the difference between msecs and elapsed. If msecs is -1,
   however, -1 is returned.
*/
static int qt_timeout_value(int msecs, int elapsed)
{
    if (msecs == -1)
        return -1;

    int timeout = msecs - elapsed;
    return timeout < 0 ? 0 : timeout;
}

bool QProcessPrivate::waitForStarted(int msecs)
{
    Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForStarted(%d) waiting for child to start (fd = %d)", msecs,
	   childStartedPipe[0]);
#endif

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(childStartedPipe[0], &fds);
    int ret;
    do {
        ret = qt_native_select(&fds, 0, msecs);
    } while (ret < 0 && errno == EINTR);
    if (ret == 0) {
        processError = QProcess::Timedout;
        q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
#if defined (QPROCESS_DEBUG)
        qDebug("QProcessPrivate::waitForStarted(%d) == false (timed out)", msecs);
#endif
        return false;
    }

    bool startedEmitted = _q_startupNotification();
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForStarted() == %s", startedEmitted ? "true" : "false");
#endif
    return startedEmitted;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
    Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForReadyRead(%d)", msecs);
#endif

    QTime stopWatch;
    stopWatch.start();

    forever {
        fd_set fdread;
        fd_set fdwrite;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);

        if (processState == QProcess::Starting)
            FD_SET(childStartedPipe[0], &fdread);

        if (standardReadPipe[0] != -1)
            FD_SET(standardReadPipe[0], &fdread);
        if (errorReadPipe[0] != -1)
            FD_SET(errorReadPipe[0], &fdread);

        FD_SET(deathPipe[0], &fdread);

        if (!writeBuffer.isEmpty() && writePipe[1] != -1)
            FD_SET(writePipe[1], &fdwrite);

        int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
        int ret = qt_native_select(&fdread, &fdwrite, timeout);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (ret == 0) {
            processError = QProcess::Timedout;
            q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
	    return false;
	}

	if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
            if (!_q_startupNotification())
                return false;
	}

        bool readyReadEmitted = false;
	if (standardReadPipe[0] != -1 && FD_ISSET(standardReadPipe[0], &fdread)) {
	    bool canRead = _q_canReadStandardOutput();
            if (processChannel == QProcess::StandardOutput && canRead)
                readyReadEmitted = true;
	}
	if (errorReadPipe[0] != -1 && FD_ISSET(errorReadPipe[0], &fdread)) {
	    bool canRead = _q_canReadStandardError();
            if (processChannel == QProcess::StandardError && canRead)
                readyReadEmitted = true;
	}
        if (readyReadEmitted)
            return true;

	if (writePipe[1] != -1 && FD_ISSET(writePipe[1], &fdwrite))
	    _q_canWrite();

	if (FD_ISSET(deathPipe[0], &fdread)) {
            if (_q_processDied())
                return false;
        }
    }
    return false;
}

bool QProcessPrivate::waitForBytesWritten(int msecs)
{
    Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForBytesWritten(%d)", msecs);
#endif

    QTime stopWatch;
    stopWatch.start();

    while (!writeBuffer.isEmpty()) {
        fd_set fdread;
        fd_set fdwrite;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);

        if (processState == QProcess::Starting)
            FD_SET(childStartedPipe[0], &fdread);

        if (standardReadPipe[0] != -1)
            FD_SET(standardReadPipe[0], &fdread);
        if (errorReadPipe[0] != -1)
            FD_SET(errorReadPipe[0], &fdread);

        FD_SET(deathPipe[0], &fdread);

        if (!writeBuffer.isEmpty() && writePipe[1] != -1)
            FD_SET(writePipe[1], &fdwrite);

	int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
	int ret = qt_native_select(&fdread, &fdwrite, timeout);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        if (ret == 0) {
	    processError = QProcess::Timedout;
	    q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
	    return false;
	}

	if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
	    if (!_q_startupNotification())
		return false;
	}

	if (writePipe[1] != -1 && FD_ISSET(writePipe[1], &fdwrite))
	    return _q_canWrite();

	if (standardReadPipe[0] != -1 && FD_ISSET(standardReadPipe[0], &fdread))
	    _q_canReadStandardOutput();

	if (errorReadPipe[0] != -1 && FD_ISSET(errorReadPipe[0], &fdread))
	    _q_canReadStandardError();

	if (FD_ISSET(deathPipe[0], &fdread)) {
            if (_q_processDied())
                return false;
        }
    }

    return false;
}

bool QProcessPrivate::waitForFinished(int msecs)
{
    Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForFinished(%d)", msecs);
#endif

    QTime stopWatch;
    stopWatch.start();

    forever {
        fd_set fdread;
        fd_set fdwrite;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);

        if (processState == QProcess::Starting)
            FD_SET(childStartedPipe[0], &fdread);

        if (standardReadPipe[0] != -1)
            FD_SET(standardReadPipe[0], &fdread);
        if (errorReadPipe[0] != -1)
            FD_SET(errorReadPipe[0], &fdread);

        if (processState == QProcess::Running)
            FD_SET(deathPipe[0], &fdread);

        if (!writeBuffer.isEmpty() && writePipe[1] != -1)
            FD_SET(writePipe[1], &fdwrite);

	int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
	int ret = qt_native_select(&fdread, &fdwrite, timeout);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
	if (ret == 0) {
	    processError = QProcess::Timedout;
	    q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
	    return false;
	}

	if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
	    if (!_q_startupNotification())
		return false;
	}
	if (writePipe[1] != -1 && FD_ISSET(writePipe[1], &fdwrite))
	    _q_canWrite();

	if (standardReadPipe[0] != -1 && FD_ISSET(standardReadPipe[0], &fdread))
	    _q_canReadStandardOutput();

	if (errorReadPipe[0] != -1 && FD_ISSET(errorReadPipe[0], &fdread))
	    _q_canReadStandardError();

	if (FD_ISSET(deathPipe[0], &fdread)) {
            if (_q_processDied())
                return true;
	}
    }
    return false;
}

bool QProcessPrivate::waitForWrite(int msecs)
{
    fd_set fdwrite;
    FD_ZERO(&fdwrite);
    FD_SET(writePipe[1], &fdwrite);

    int ret;
    do {
        ret = qt_native_select(0, &fdwrite, msecs < 0 ? 0 : msecs) == 1;
    } while (ret < 0 && errno == EINTR);
    return ret == 1;
}

void QProcessPrivate::findExitCode()
{
    Q_Q(QProcess);
    processManager()->remove(q);
}

bool QProcessPrivate::waitForDeadChild()
{
    Q_Q(QProcess);
    
    // read a byte from the death pipe
    char c;
    ::read(deathPipe[0], &c, 1);
    
    // check if our process is dead
    int exitStatus;
    pid_t waitResult = waitpid(pid_t(pid), &exitStatus, WNOHANG);
    if (waitResult > 0) {
        processManager()->remove(q);
        crashed = !WIFEXITED(exitStatus);
        exitCode = WEXITSTATUS(exitStatus);
#if defined QPROCESS_DEBUG
        qDebug() << "QProcessPrivate::waitForDeadChild() dead with exitCode"
                 << exitCode << ", crashed?" << crashed;
#endif
        return true;
    }
#if defined QPROCESS_DEBUG
    qDebug() << "QProcessPrivate::waitForDeadChild() not dead!";
#endif
    return false;
}

void QProcessPrivate::_q_notified()
{
}

/*! \internal
 */
bool QProcessPrivate::startDetached(const QString &program, const QStringList &arguments)
{
    processManager()->start();

    pid_t childPid = fork();
    if (childPid == 0) {
        ::setsid();
        ::signal(SIGHUP, SIG_IGN);
        ::signal(SIGPIPE, SIG_DFL);

        if (fork() == 0) {
            char **argv = new char *[arguments.size() + 2];
            for (int i = 0; i < arguments.size(); ++i) {
#ifdef Q_OS_MAC
                argv[i + 1] = ::strdup(arguments.at(i).toUtf8().constData());
#else
                argv[i + 1] = ::strdup(arguments.at(i).toLocal8Bit().constData());
#endif
            }
            argv[arguments.size() + 1] = 0;

            if (!program.contains(QLatin1Char('/'))) {
                const QString path = QString::fromLocal8Bit(::getenv("PATH"));
                if (!path.isEmpty()) {
                    QStringList pathEntries = path.split(QLatin1Char(':'));
                    for (int k = 0; k < pathEntries.size(); ++k) {
                        QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                        if (!tmp.endsWith('/')) tmp += '/';
                        tmp += QFile::encodeName(program);
                        argv[0] = tmp.data();
                        ::execv(argv[0], argv);
                    }
                }
            } else {
                QByteArray tmp = QFile::encodeName(program);
                argv[0] = tmp.data();
                ::execv(argv[0], argv);
            }

            ::_exit(1);
        }

        ::chdir("/");
        ::_exit(1);
    }
    int result;
    while (waitpid(childPid, &result, 0) == -1 && errno == EINTR)
        ;                       // nothing
    return WIFEXITED(result);
}

void QProcessPrivate::initializeProcessManager()
{
    (void) processManager();
}

#include "qprocess_unix.moc"

#endif // QT_NO_PROCESS
