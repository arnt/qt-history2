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

#include "qplatformdefs.h"

#include "qprocess.h"
#include "qprocess_p.h"

#include <qabstracteventdispatcher.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qmap.h>
#include <qmutex.h>
#include <qsignal.h>
#include <qsocketnotifier.h>

#include <errno.h>
#include <stdlib.h>

static int qt_qprocess_deadChild_pipe[2];
static void (*qt_sa_old_sigchld_handler)(int) = 0;
static void qt_sa_sigchld_handler(int signum)
{
    ::write(qt_qprocess_deadChild_pipe[1], "", 1);
    if (qt_sa_old_sigchld_handler && qt_sa_old_sigchld_handler != SIG_IGN)
        qt_sa_old_sigchld_handler(signum);
}

class QProcessManager : public QObject
{
    Q_OBJECT
public:
    ~QProcessManager();

    static inline QProcessManager &instance()
    {
        // ### add reentrancy
        static QProcessManager singleton;
        return singleton;
    }

    void initialize();

    inline void add(pid_t pid, QProcess *process)
        {
            QMutexLocker lock(&mutex);

#if defined (QPROCESS_DEBUG)
            qDebug("QProcessManager::add(%d, %p)", int(pid), process);
#endif
            children[pid] = process;
        }

    inline bool has(pid_t pid)
        {
            return children.contains(pid);
        }

public slots:
    void deadChildNotification(int);

protected:
    inline QProcessManager() : old_sigchld_handler(0) { }

private:

    QMutex mutex;
    QMap<pid_t, QPointer<QProcess> > children;

    void (*old_sigchld_handler)(int);

    QSocketNotifier *shutdownNotifier;
};

QProcessManager::~QProcessManager()
{
}

void QProcessManager::initialize()
{
    QMutexLocker lock(&mutex);
    if (qt_sa_old_sigchld_handler == 0) {
#if defined (QPROCESS_DEBUG)
        qDebug("QProcessManager::initialize(), initializing process manager");
#endif
        ::pipe(qt_qprocess_deadChild_pipe);
        ::fcntl(qt_qprocess_deadChild_pipe[0], F_SETFL,
                ::fcntl(qt_qprocess_deadChild_pipe[0], F_GETFL) | O_NONBLOCK);
        struct sigaction oldAction;
        struct sigaction action;
        memset(&action, 0, sizeof(action));
        action.sa_handler = qt_sa_sigchld_handler;
        action.sa_flags = SA_NOCLDSTOP;
        ::sigaction(SIGCHLD, &action, &oldAction);
        if (oldAction.sa_handler != qt_sa_sigchld_handler)
            old_sigchld_handler = qt_sa_old_sigchld_handler = oldAction.sa_handler;

        if (QAbstractEventDispatcher::instance(thread())) {
#if defined (QPROCESS_DEBUG)
        qDebug("QProcessManager::initialize(), enabling dead child notifier");
#endif
            shutdownNotifier = new QSocketNotifier(qt_qprocess_deadChild_pipe[0],
                                                   QSocketNotifier::Read, this);
            connect(shutdownNotifier, SIGNAL(activated(int)),
                    this, SLOT(deadChildNotification(int)));
        }
    }
}

void QProcessManager::deadChildNotification(int)
{
    char c;
    ::read(qt_qprocess_deadChild_pipe[0], &c, 1);

    for (;;) {
        int result;
        pid_t childpid = waitpid(0, &result, WNOHANG);
        if (childpid <= 0)
            break;

        mutex.lock();
        QPointer<QProcess> child = children.value(childpid, 0);
        if (child) {
#if defined (QPROCESS_DEBUG)
            qDebug("QProcessManager::deadChildNotification(), caught dead child in %p",
                   (QProcess *)child);
#endif
            ((QProcessPrivate *)child->d_ptr)->exitCode = WEXITSTATUS(result);
            ((QProcessPrivate *)child->d_ptr)->crashed = !WIFEXITED(result);
            children.remove(childpid);
        }
        mutex.unlock();

        if (child)
            qInvokeMetaMember(child, "processDied");
    }
}

static void qt_create_pipe(int *pipe)
{
    if (pipe[0] != -1)
        ::close(pipe[0]);
    if (pipe[1] != -1)
        ::close(pipe[1]);
#ifdef Q_OS_IRIX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, pipe) == -1) {
        qWarning("QProcessPrivate::createPipe(%p) failed: %s",
                 pipe, strerror(errno));
    }
#else
    if (::pipe(pipe) != 0) {
        qWarning("QProcessPrivate::createPipe(%p) failed: %s",
                 pipe, strerror(errno));
    }
#endif
}

void QProcessPrivate::destroyPipe(int *pipe)
{
    if (pipe[0] != -1) {
        ::close(pipe[0]);
        pipe[0] = -1;
    }
    if (pipe[1] != -1) {
        ::close(pipe[1]);
        pipe[1] = -1;
    }
}

void QProcessPrivate::startProcess()
{
    Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::startProcess()");
#endif

    // Initialize pipes
    qt_create_pipe(childStartedPipe);
    if (QAbstractEventDispatcher::instance(q->thread())) {
        startupSocketNotifier = new QSocketNotifier(childStartedPipe[0],
                                                    QSocketNotifier::Read, q);
        QObject::connect(startupSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(startupNotification()));
    }
    qt_create_pipe(writePipe);

    if (QAbstractEventDispatcher::instance(q->thread())) {
        writeSocketNotifier = new QSocketNotifier(writePipe[1],
                                                  QSocketNotifier::Write, q);
        QObject::connect(writeSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(canWrite()));
        writeSocketNotifier->setEnabled(false);
    }

    qt_create_pipe(standardReadPipe);
    qt_create_pipe(errorReadPipe);

    if (QAbstractEventDispatcher::instance(q->thread())) {
        standardReadSocketNotifier = new QSocketNotifier(standardReadPipe[0],
                                                         QSocketNotifier::Read,
                                                         q);
        QObject::connect(standardReadSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(canReadStandardOutput()));

        errorReadSocketNotifier = new QSocketNotifier(errorReadPipe[0],
                                                      QSocketNotifier::Read,
                                                      q);
        QObject::connect(errorReadSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(canReadStandardError()));
    }

    // Start the process (platform dependent)
    processState = QProcess::Starting;
    emit q->stateChanged(processState);

    QProcessManager::instance().initialize();

    pid = (Q_PID) fork();
    if (pid == 0) {
        execChild();
        ::_exit(-1);
    }

    QProcessManager::instance().add(pid, q);

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
    ::fcntl(standardReadPipe[0], F_SETFL, ::fcntl(standardReadPipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(errorReadPipe[0], F_SETFL, ::fcntl(errorReadPipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(writePipe[1], F_SETFL, ::fcntl(writePipe[1], F_GETFL) | O_NONBLOCK);
}

void QProcessPrivate::execChild()
{
    QByteArray prog = QFile::encodeName(program);

    // create argument list with right number of elements, and set the
    // final one to 0.
    char **argv = new char *[arguments.count() + 2];
    argv[arguments.count() + 1] = 0;

    // allow invoking of .app bundles on the Mac.
#ifdef Q_OS_MAC
    QFileInfo fileInfo(prog);
    if (fileInfo.isDir() && prog.endsWith(".app")) {
        QByteArray tmp = prog;
        int lastSlashPos = tmp.lastIndexOf('/');
        if(lastSlashPos != -1)
            tmp.remove(0, lastSlashPos + 1);
        tmp = prog + "/Contents/MacOS/" + tmp;
        tmp.resize(tmp.size() - 4); // chop off the .app
        if(QFile::exists(tmp))
            prog = tmp;
    }
#endif

    // add the program name
    argv[0] = new char[prog.size() + 1];
    memcpy(argv[0], prog.data(), prog.size());
    argv[0][prog.size()] = '\0';

    // add every argument to the list
    for (int i = 0; i < arguments.count(); ++i) {
        QString arg = arguments.at(i);
        argv[i + 1] = new char[arg.size() + 1];
        memcpy(argv[i + 1], arg.toLocal8Bit(), arg.size());
        argv[i + 1][arg.size()] = '\0';
    }

    // on all pipes, close the end that we don't use
    ::close(standardReadPipe[0]);
    ::close(errorReadPipe[0]);
    ::close(writePipe[1]);

    // copy the stdin socket
    ::dup2(writePipe[0], fileno(stdin));

    // copy the stdout and stderr if asked to
    if (processChannelMode != QProcess::ForwardedChannels) {
        ::dup2(standardReadPipe[1], fileno(stdout));
        ::dup2(errorReadPipe[1], fileno(stderr));

        // merge stdout and stderr if asked to
        if (processChannelMode == QProcess::MergedChannels)
            ::dup2(fileno(stdout), fileno(stderr));
    }

    // make sure this fd is closed if execvp() succeeds
    ::close(childStartedPipe[0]);
    ::fcntl(childStartedPipe[1], F_SETFD, FD_CLOEXEC);

    // enter the working directory
    if (!workingDirectory.isEmpty())
        ::chdir(QFile::encodeName(workingDirectory).data());

    // execute the process
    if (environment.isEmpty()) {
        ::execvp(prog.data(), argv);
    } else {
        // if LD_LIBRARY_PATH exists in the current environment, but
        // not in the environment list passed by the programmer, then
        // copy it over.
#if defined(Q_OS_MACX)
        static const char libraryPath[] = "DYLD_LIBRARY_PATH";
#else
        static const char libraryPath[] = "LD_LIBRARY_PATH";
#endif
        QStringList matches = environment.filter(QRegExp("^" + QByteArray(libraryPath) + "="));
        char *envLibraryPath = ::getenv(libraryPath);
        if (matches.isEmpty() && envLibraryPath != 0) {
            QString entry = libraryPath;
            entry += "=";
            entry += envLibraryPath;
            environment << QString(libraryPath) +  "=" + QString(envLibraryPath);
        }

        char **envp = new char *[environment.count() + 1];
        envp[environment.count()] = 0;

        for (int j = 0; j < environment.count(); ++j) {
            QString item = environment.at(j);
            envp[j] = new char[item.size() + 1];
            memcpy(envp[j], item.toLocal8Bit(), item.size());
            envp[j][item.size()] = '\0';
        }

        if (!prog.contains("/")) {
            char *path = ::getenv("PATH");
            if (path) {
                QStringList pathEntries = QString(path).split(":");
                for (int k = 0; k < pathEntries.size(); ++k) {
                    QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                    if (!tmp.endsWith('/')) tmp += '/';
                    tmp += prog;
                    ::execve(tmp.data(), argv, envp);
                }
            }
        } else {
            ::execve(prog.data(), argv, envp);
        }
    }

    // notify failure
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

qint64 QProcessPrivate::writeToStdin(const char *data, qint64 maxlen)
{
    qint64 written = qint64(::write(writePipe[1], data, maxlen));
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::writeToStdin(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, maxlen, 16).constData(), maxlen, written);
#endif
    return written;
}

void QProcessPrivate::killProcess()
{
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::killProcess()");
#endif
    if (pid)
        ::kill(pid, SIGTERM);
}

static int qt_native_select(fd_set *fdread, fd_set *fdwrite, int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    return select(FD_SETSIZE, fdread, fdwrite, 0, timeout < 0 ? 0 : &tv);
}

bool QProcessPrivate::waitForStarted(int msecs)
{
    Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForStarted(%d) ...", msecs);
#endif

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(childStartedPipe[0], &fds);
    int ret = qt_native_select(&fds, 0, msecs);
    if (ret == 0) {
        processError = QProcess::Timedout;
        q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out"));
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForStarted(%d) == false (timed out)", msecs);
#endif
        return false;
    }

    bool startedEmitted = startupNotification();
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

    if (QProcessManager::instance().has(pid)) {
        QTime stopWatch;
        stopWatch.start();
        forever {
            fd_set fdread;
            FD_ZERO(&fdread);
            if (processState == QProcess::NotRunning)
                FD_SET(childStartedPipe[0], &fdread);
            FD_SET(qt_qprocess_deadChild_pipe[0], &fdread);
            FD_SET(standardReadPipe[0], &fdread);
            FD_SET(errorReadPipe[0], &fdread);

            fd_set fdwrite;
            FD_ZERO(&fdwrite);
            if (!writeBuffer.isEmpty() && writePipe[1] != -1)
                FD_SET(writePipe[1], &fdwrite);

            int timeout = msecs - stopWatch.elapsed();
            int ret = qt_native_select(&fdread, &fdwrite, timeout);
            if (ret == 0) {
                processError = QProcess::Timedout;
                q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out"));
                return false;
            }

            if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
                if (!startupNotification())
                    return false;
            }

            bool readyReadEmitted = false;
            if (FD_ISSET(standardReadPipe[0], &fdread))
                readyReadEmitted = canReadStandardOutput() ? true : readyReadEmitted;
            if (FD_ISSET(errorReadPipe[0], &fdread))
                readyReadEmitted = canReadStandardError() ? true : readyReadEmitted;
            if (readyReadEmitted)
                return true;

            if (writePipe[1] != -1 && FD_ISSET(writePipe[1], &fdwrite))
                canWrite();
            if (FD_ISSET(qt_qprocess_deadChild_pipe[0], &fdread)) {
                QProcessManager::instance().deadChildNotification(0);
                if (processState != QProcess::Running)
                    return readyReadEmitted;
            }
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

    if (QProcessManager::instance().has(pid)) {
        QTime stopWatch;
        stopWatch.start();
        forever {
            fd_set fdread;
            FD_ZERO(&fdread);
            if (processState == QProcess::NotRunning)
                FD_SET(childStartedPipe[0], &fdread);
            FD_SET(qt_qprocess_deadChild_pipe[0], &fdread);
            FD_SET(standardReadPipe[0], &fdread);
            FD_SET(errorReadPipe[0], &fdread);

            fd_set fdwrite;
            FD_ZERO(&fdwrite);
            if (!writeBuffer.isEmpty() && writePipe[1] != -1)
                FD_SET(writePipe[1], &fdwrite);

            int timeout = msecs - stopWatch.elapsed();
            int ret = qt_native_select(&fdread, &fdwrite, timeout);
            if (ret == 0) {
                processError = QProcess::Timedout;
                q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out"));
                return false;
            }

            if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
                if (!startupNotification())
                    return false;
            }

            if (writePipe[1] != -1 && FD_ISSET(writePipe[1], &fdwrite)) {
                if (canWrite())
                    return true;
            }
            if (FD_ISSET(standardReadPipe[0], &fdread))
                canReadStandardOutput();
            if (FD_ISSET(errorReadPipe[0], &fdread))
                canReadStandardError();
            if (FD_ISSET(qt_qprocess_deadChild_pipe[0], &fdread)) {
                QProcessManager::instance().deadChildNotification(0);
                if (processState != QProcess::Running)
                    return false;
            }
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

    if (QProcessManager::instance().has(pid)) {
        QTime stopWatch;
        stopWatch.start();
        forever {
            fd_set fdread;
            FD_ZERO(&fdread);
            if (processState == QProcess::NotRunning)
                FD_SET(childStartedPipe[0], &fdread);
            FD_SET(qt_qprocess_deadChild_pipe[0], &fdread);
            FD_SET(standardReadPipe[0], &fdread);
            FD_SET(errorReadPipe[0], &fdread);

            fd_set fdwrite;
            FD_ZERO(&fdwrite);
            if (!writeBuffer.isEmpty() && writePipe[1] != -1)
                FD_SET(writePipe[1], &fdwrite);

            int timeout = msecs - stopWatch.elapsed();
            int ret = qt_native_select(&fdread, &fdwrite, timeout);
            if (ret == 0) {
                processError = QProcess::Timedout;
                q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process opeation timed out"));
                return false;
            }

            if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
                if (!startupNotification())
                    return false;
            }

            if (writePipe[1] != -1 && FD_ISSET(writePipe[1], &fdwrite))
                canWrite();
            if (FD_ISSET(standardReadPipe[0], &fdread))
                canReadStandardOutput();
            if (FD_ISSET(errorReadPipe[0], &fdread))
                canReadStandardError();
            if (FD_ISSET(qt_qprocess_deadChild_pipe[0], &fdread)) {
                QProcessManager::instance().deadChildNotification(0);
                if (processState != QProcess::Running)
                    return true;
            }
        }
    }
    return false;
}

bool QProcessPrivate::waitForWrite(int msecs)
{
    fd_set fdwrite;
    FD_ZERO(&fdwrite);
    FD_SET(writePipe[1], &fdwrite);

    return qt_native_select(0, &fdwrite, msecs < 0 ? 0 : msecs) == 1;
}

void QProcessPrivate::findExitCode()
{
}

void QProcessPrivate::notified()
{
}

bool QProcessPrivate::execute(const QString &program, const QStringList &arguments,
                             const QString &workingDir, const QStringList &environment)
{
    return false;
}

#include "qprocess_unix.moc"

