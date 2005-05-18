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

#include <qdatetime.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <private/qwineventnotifier_p.h>
#include <qabstracteventdispatcher.h>
#include <qdebug.h>

//#define QPROCESS_DEBUG

#define SLEEPMIN 10
#define SLEEPMAX 500
#define NOTIFYTIMEOUT 100
#define MAXSINGLEWRITE qint64(10000) //### may not need this now

class QIncrementalSleepTimer
{
public:
    QIncrementalSleepTimer(int msecs)
        : totalTimeOut(msecs)
        , nextSleep(qMin(SLEEPMIN, totalTimeOut))
    {
        timer.start();
    }

    int nextSleepTime()
    {
        if (totalTimeOut == -1)
            return -1;

        int tmp = nextSleep;

        nextSleep = qMin(nextSleep * 2, qMin(SLEEPMAX, timeLeft()));

        return tmp;
    }

    int timeLeft()
    {
        return qMax(totalTimeOut - timer.elapsed(), 0);
    }

    bool hasTimedOut()
    {
        return timer.elapsed() >= totalTimeOut;
    }

    void resetIncrements()
    {
        nextSleep = qMin(SLEEPMIN, timeLeft());
    }

private:
    QTime timer;
    int totalTimeOut;
    int nextSleep;
};

class QWindowsPipeWriter : public QThread
{
    Q_OBJECT
public:
    QWindowsPipeWriter(HANDLE writePipe, QObject * parent = 0);
    ~QWindowsPipeWriter();

    bool waitForWrite(int msecs);
    qint64 write(const char *data, qint64 maxlen);

signals:
    void canWrite();

protected:
   void run();

private:
    QMutex lock;
    QWaitCondition waitCondition;
    bool quitNow;
    HANDLE writePipe;
    QByteArray data;
};


QWindowsPipeWriter::QWindowsPipeWriter(HANDLE pipe, QObject * parent)
  : QThread(parent), quitNow(false)
{

    DuplicateHandle(GetCurrentProcess(), pipe, GetCurrentProcess(),
                         &writePipe, 0, FALSE, DUPLICATE_SAME_ACCESS);
}


QWindowsPipeWriter::~QWindowsPipeWriter()
{
    lock.lock();
    quitNow = true;
    waitCondition.wakeOne();
    lock.unlock();
    if (!wait(100))
        terminate();
    CloseHandle(writePipe);
}

bool QWindowsPipeWriter::waitForWrite(int msecs)
{
    QMutexLocker locker(&lock);
    if (data.isEmpty())
        return true;
    return waitCondition.wait(&lock, msecs);
}

qint64 QWindowsPipeWriter::write(const char *ptr, qint64 maxlen)
{
    if (!isRunning())
        return -1;

    QMutexLocker locker(&lock);
    if (!data.isEmpty())
        return 0;
    data = QByteArray(ptr, maxlen);
    waitCondition.wakeOne();
    return maxlen;
}


void QWindowsPipeWriter::run()
{

    for (;;) {

        lock.lock();

        while(data.isEmpty() && (!quitNow)) {
            waitCondition.wakeOne();
            waitCondition.wait(&lock);
        }

        if (quitNow) {
            lock.unlock();
            break;
        }

        QByteArray copy = data;
        data.clear();

        lock.unlock();

        const char *ptrData = copy.data();
        qint64 maxlen = copy.size();
        qint64 totalWritten = 0;
        while ((!quitNow) && totalWritten < maxlen) {
            DWORD written = 0;
            if (!WriteFile(writePipe, ptrData + totalWritten, qMin<int>(8192, maxlen - totalWritten), &written, 0)) {
                if (GetLastError() == 0xE8 /*NT_STATUS_INVALID_USER_BUFFER*/) {
                    // give the os a rest
                    sleep(100);
                    continue;
                }
                return;
            }
#if defined QPROCESS_DEBUG
            qDebug("QWindowsPipeWriter::run() wrote %d bytes", written);
#endif
            totalWritten += written;
        }
        emit canWrite();
    }
}

static void qt_create_pipes(QProcessPrivate *that)
{
    // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles (this is done by the
    // DuplicateHandle() call).

    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };

    HANDLE tmpStdin, tmpStdout, tmpStderr;
    if (!CreatePipe(&that->writePipe[0], &tmpStdin, &secAtt, 0))
        return;
    if (!DuplicateHandle(GetCurrentProcess(), tmpStdin, GetCurrentProcess(),
                         &that->writePipe[1], 0, FALSE, DUPLICATE_SAME_ACCESS))
        return;
    if (!CloseHandle(tmpStdin))
        return;
    if (that->processChannelMode == QProcess::ForwardedChannels) {
        tmpStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        if (tmpStdout == INVALID_HANDLE_VALUE)
            return;
        if (!DuplicateHandle(GetCurrentProcess(), tmpStdout, GetCurrentProcess(),
                         &that->standardReadPipe[1], 0, TRUE, DUPLICATE_SAME_ACCESS))
        return;

        tmpStderr = GetStdHandle(STD_OUTPUT_HANDLE);
        if (tmpStderr == INVALID_HANDLE_VALUE)
            return;
        if (!DuplicateHandle(GetCurrentProcess(), tmpStderr, GetCurrentProcess(),
                         &that->errorReadPipe[1], 0, TRUE, DUPLICATE_SAME_ACCESS))
        return;

    } else {
        if (!CreatePipe(&tmpStdout, &that->standardReadPipe[1], &secAtt, 0))
            return;
        if (!DuplicateHandle(GetCurrentProcess(), tmpStdout, GetCurrentProcess(),
                         &that->standardReadPipe[0], 0, FALSE, DUPLICATE_SAME_ACCESS))
        return;
        if (!CloseHandle(tmpStdout))
            return;

        if (that->processChannelMode == QProcess::MergedChannels) {
            if (!DuplicateHandle(GetCurrentProcess(), that->standardReadPipe[1], GetCurrentProcess(),
                         &that->errorReadPipe[1], 0, TRUE, DUPLICATE_SAME_ACCESS))
            return;
        } else {
            if (!CreatePipe(&tmpStderr, &that->errorReadPipe[1], &secAtt, 0))
                return;
            if (!DuplicateHandle(GetCurrentProcess(), tmpStderr, GetCurrentProcess(),
                         &that->errorReadPipe[0], 0, FALSE, DUPLICATE_SAME_ACCESS))
                return;
            if (!CloseHandle(tmpStderr))
                return;
        }
    }
}

void QProcessPrivate::destroyPipe(Q_PIPE pipe[2])
{
    if (pipe[0] == writePipe[0] && pipe[1] == writePipe[1] && pipeWriter) {
        pipeWriter->waitForWrite(ULONG_MAX);
        delete pipeWriter;
        pipeWriter = 0;
    }

    if (pipe[0] != INVALID_Q_PIPE) {
        CloseHandle(pipe[0]);
        pipe[0] = INVALID_Q_PIPE;
    }
    if (pipe[1] != INVALID_Q_PIPE) {
        CloseHandle(pipe[1]);
        pipe[1] = INVALID_Q_PIPE;
    }
}


static QString qt_create_commandline(const QString &program, const QStringList &arguments)
{
    QString programName = program;
    if (!programName.startsWith("\"") && !programName.endsWith("\"") && programName.contains(" "))
        programName = "\"" + programName + "\"";
    programName.replace("/", "\\");

    QString args;
    // add the prgram as the first arrg ... it works better
    args = programName + " ";
    for (int i=0; i<arguments.size(); ++i) {
        QString tmp = arguments.at(i);
        // in the case of \" already being in the string the \ must also be escaped
        tmp.replace( "\\\"", "\\\\\"" );
        // escape a single " because the arguments will be parsed
        tmp.replace( "\"", "\\\"" );
        if (tmp.isEmpty() || tmp.contains(' ') || tmp.contains('\t')) {
            // The argument must not end with a \ since this would be interpreted
            // as escaping the quote -- rather put the \ behind the quote: e.g.
            // rather use "foo"\ than "foo\"
            QString endQuote("\"");
            int i = tmp.length();
            while (i>0 && tmp.at(i-1) == '\\') {
                --i;
                endQuote += "\\";
            }
            args += QString(" \"") + tmp.left(i) + endQuote;
        } else {
            args += ' ' + tmp;
        }
    }
    return args;
}

static QByteArray qt_create_environment(const QStringList &environment)
{
    QByteArray envlist;
    if (environment.isEmpty()) {
	int pos = 0;
	// add PATH if necessary (for DLL loading)
	QByteArray path = qgetenv("PATH");
#ifdef UNICODE
        if (!(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
	    if (environment.filter(QRegExp("^PATH=",Qt::CaseInsensitive)).isEmpty()
                && !path.isNull()) {
                QString tmp = QString(QLatin1String("PATH=%1")).arg(QString(path));
                uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
                envlist.resize(envlist.size() + tmpSize );
                memcpy(envlist.data()+pos, tmp.utf16(), tmpSize);
                pos += tmpSize;
	    }
	    // add the user environment
	    for (QStringList::ConstIterator it = environment.begin(); it != environment.end(); it++ ) {
                QString tmp = *it;
                uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
                envlist.resize(envlist.size() + tmpSize);
                memcpy(envlist.data()+pos, tmp.utf16(), tmpSize);
                pos += tmpSize;
	    }
	    // add the 2 terminating 0 (actually 4, just to be on the safe side)
	    envlist.resize( envlist.size()+4 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
        } else
#endif // UNICODE
        {
            if (environment.filter(QRegExp("^PATH=",Qt::CaseInsensitive)).isEmpty() && !path.isNull()) {
                QByteArray tmp = QString("PATH=%1").arg(QString(path)).toLocal8Bit();
                uint tmpSize = tmp.length() + 1;
                envlist.resize(envlist.size() + tmpSize);
                memcpy(envlist.data()+pos, tmp.data(), tmpSize);
                pos += tmpSize;
            }
            // add the user environment
            for (QStringList::ConstIterator it = environment.begin(); it != environment.end(); it++) {
                QByteArray tmp = (*it).toLocal8Bit();
                uint tmpSize = tmp.length() + 1;
                envlist.resize(envlist.size() + tmpSize);
                memcpy(envlist.data()+pos, tmp.data(), tmpSize);
                pos += tmpSize;
            }
            // add the terminating 0 (actually 2, just to be on the safe side)
            envlist.resize(envlist.size()+2);
            envlist[pos++] = 0;
            envlist[pos++] = 0;
        }
    }

    return envlist;
}

void QProcessPrivate::startProcess()
{
    Q_Q(QProcess);

    bool success = false;

    if (pid) {
        CloseHandle(pid->hThread);
        CloseHandle(pid->hProcess);
        delete pid;
        pid = 0;
    }
    pid = new PROCESS_INFORMATION;
    memset(pid, 0, sizeof(PROCESS_INFORMATION));

    processState = QProcess::Starting;
    emit q->stateChanged(processState);

    qt_create_pipes(this);
    QString args = qt_create_commandline(program, arguments);
    QByteArray envlist = qt_create_environment(environment);

#if defined QPROCESS_DEBUG
    qDebug("Creating process");
    qDebug("   program : [%s]", program.latin1());
    qDebug("   args : %s", args.latin1());
    qDebug("   pass enviroment : %s", environment.isEmpty() ? "no" : "yes");
#endif

    DWORD dwCreationFlags = 0;
    if (!(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based))
        dwCreationFlags |= CREATE_NO_WINDOW;

#ifdef UNICODE
    if (!(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
        dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
        STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	                                 (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                         (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                         0, 0, 0,
                                         STARTF_USESTDHANDLES,
                                         0, 0, 0,
                                         writePipe[0], standardReadPipe[1], errorReadPipe[1]
        };
        success = CreateProcessW(0, (WCHAR*)args.utf16(),
                                 0, 0, TRUE, dwCreationFlags,
                                 environment.isEmpty() ? 0 : envlist.data(),
                                 workingDirectory.isEmpty() ? 0
                                    : (WCHAR*)QDir::convertSeparators(workingDirectory).utf16(),
                                 &startupInfo, pid);
    } else
#endif // UNICODE
    {
#ifndef Q_OS_TEMP
	    STARTUPINFOA startupInfo = { sizeof( STARTUPINFOA ), 0, 0, 0,
                                         (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                         (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                         0, 0, 0,
                                         STARTF_USESTDHANDLES,
                                         0, 0, 0,
                                         writePipe[0], standardReadPipe[1], errorReadPipe[1]
	    };

	    success = CreateProcessA(0, args.toLocal8Bit().data(),
                                     0, 0, TRUE, dwCreationFlags, environment.isEmpty() ? 0 : envlist.data(),
                                     workingDirectory.isEmpty() ? 0
                                        : QDir::convertSeparators(workingDirectory).toLocal8Bit().data(),
                                     &startupInfo, pid);
#endif // Q_OS_TEMP
    }
#ifndef Q_OS_TEMP

    CloseHandle(writePipe[0]);
    writePipe[0] = INVALID_Q_PIPE;
    CloseHandle(standardReadPipe[1]);
    standardReadPipe[1] = INVALID_Q_PIPE;
    CloseHandle(errorReadPipe[1]);
    errorReadPipe[1] = INVALID_Q_PIPE;
#endif

    processState = QProcess::Running;

    if (QAbstractEventDispatcher::instance(q->thread())) {
        processFinishedNotifier = new QWinEventNotifier(pid->hProcess, q);
        QObject::connect(processFinishedNotifier, SIGNAL(activated(HANDLE)), q, SLOT(processDied()));
        processFinishedNotifier->setEnabled(true);
        notifier = new QTimer(q);
        QObject::connect(notifier, SIGNAL(timeout()), q, SLOT(notified()));
        notifier->start(NOTIFYTIMEOUT);
    }

    // give the process a chance to start ...
    Sleep(SLEEPMIN*2);
    startupNotification();
}

void QProcessPrivate::execChild(const QByteArray &encodedProgramName)
{
    Q_UNUSED(encodedProgramName);
    // unix only
}

bool QProcessPrivate::processStarted()
{
    return processState == QProcess::Running;
}

qint64 QProcessPrivate::bytesAvailableFromStdout() const
{
    DWORD bytesAvail = 0;
    PeekNamedPipe(standardReadPipe[0], 0, 0, 0, &bytesAvail, 0);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::bytesAvailableFromStdout() == %d", bytesAvail);
#endif
    return bytesAvail;
}

qint64 QProcessPrivate::bytesAvailableFromStderr() const
{
    DWORD bytesAvail = 0;
    PeekNamedPipe(errorReadPipe[0], 0, 0, 0, &bytesAvail, 0);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::bytesAvailableFromStderr() == %d", bytesAvail);
#endif
    return bytesAvail;
}

qint64 QProcessPrivate::readFromStdout(char *data, qint64 maxlen)
{
    DWORD read = qMin(maxlen, bytesAvailableFromStdout());
    DWORD bytesRead = 0;

    if (read > 0 && !ReadFile(standardReadPipe[0], data, read, &bytesRead, 0))
        return -1;
    return bytesRead;
}

qint64 QProcessPrivate::readFromStderr(char *data, qint64 maxlen)
{
    DWORD read = qMin(maxlen, bytesAvailableFromStderr());
    DWORD bytesRead = 0;

    if (read > 0 && !ReadFile(errorReadPipe[0], data, read, &bytesRead, 0))
        return -1;
    return bytesRead;
}

void QProcessPrivate::killProcess()
{
    if (pid) {
        if (PostThreadMessage(pid->dwThreadId, WM_CLOSE, 0, 0)) {
            if (WaitForSingleObject(pid->hProcess, SLEEPMIN * 2) == WAIT_OBJECT_0)
                return;
        }
        TerminateProcess(pid->hProcess, 0xf291);
    }
}

bool QProcessPrivate::waitForStarted(int)
{
    Q_Q(QProcess);

    if (processStarted())
        return true;

    processError = QProcess::Timedout;
    q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process opeation timed out"));
    return false;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
    Q_Q(QProcess);

    QIncrementalSleepTimer timer(msecs);

    forever {

        if (!writeBuffer.isEmpty() && (!pipeWriter || pipeWriter->waitForWrite(0))) {
            canWrite();
            timer.resetIncrements();
        }

        bool readyReadEmitted = false;
        if (bytesAvailableFromStdout() != 0) {
            readyReadEmitted = canReadStandardOutput() ? true : readyReadEmitted;
            timer.resetIncrements();
        }

        if (bytesAvailableFromStderr() != 0) {
            readyReadEmitted = canReadStandardError() ? true : readyReadEmitted;
            timer.resetIncrements();
        }

        if (readyReadEmitted)
            return true;

        if (!pid)
            return false;
        if (WaitForSingleObject(pid->hProcess, 0) == WAIT_OBJECT_0) {
            // find the return value if there is noew data to read
            processDied();
            return false;
        }

        Sleep(timer.nextSleepTime());
        if (timer.hasTimedOut())
            break;
    }

    processError = QProcess::Timedout;
    q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process opeation timed out"));
    return false;
}

bool QProcessPrivate::waitForBytesWritten(int msecs)
{

    Q_Q(QProcess);

    QIncrementalSleepTimer timer(msecs);

    forever {

        if (!writeBuffer.isEmpty() && (!pipeWriter || pipeWriter->waitForWrite(0)))
            return canWrite();

        if (bytesAvailableFromStdout() != 0) {
            canReadStandardOutput();
            timer.resetIncrements();
        }

        if (bytesAvailableFromStderr() != 0) {
            canReadStandardError();
            timer.resetIncrements();
        }

        if (!pid)
            return false;
        if (WaitForSingleObject(pid->hProcess, 0) == WAIT_OBJECT_0) {
            processDied();
            return false;
        }

        if (timer.hasTimedOut())
            break;
    }

    processError = QProcess::Timedout;
    q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process opeation timed out"));
    return false;
}


bool QProcessPrivate::waitForFinished(int msecs)
{
    Q_Q(QProcess);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::waitForFinished(%d)", msecs);
#endif

    QIncrementalSleepTimer timer(msecs);

    forever {
        if (!writeBuffer.isEmpty() && (!pipeWriter || pipeWriter->waitForWrite(0))) {
            canWrite();
            timer.resetIncrements();
        }

        if (bytesAvailableFromStdout() != 0) {
            canReadStandardOutput();
            timer.resetIncrements();
        }

        if (bytesAvailableFromStderr() != 0) {
            canReadStandardError();
            timer.resetIncrements();
        }

        if (!pid)
            return true;

        if (WaitForSingleObject(pid->hProcess, timer.nextSleepTime()) == WAIT_OBJECT_0) {
            processDied();
            return true;
        }

        if (timer.hasTimedOut())
            break;
    }
    processError = QProcess::Timedout;
    q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process opeation timed out"));
    return false;
}


void QProcessPrivate::findExitCode()
{
    DWORD theExitCode;
    if (GetExitCodeProcess(pid->hProcess, &theExitCode)) {
        exitCode = theExitCode;
        //### for now we assume a crash if exit code is less than -1 or the magic number
        if (exitCode == 0xf291 || (int)exitCode < 0)
            crashed = true;
    }
}


qint64 QProcessPrivate::writeToStdin(const char *data, qint64 maxlen)
{
    Q_Q(QProcess);

    if (!pipeWriter) {
        pipeWriter = new QWindowsPipeWriter(writePipe[1], q);
        pipeWriter->start();
    }

    return pipeWriter->write(data, qMin(MAXSINGLEWRITE, maxlen));
}

bool QProcessPrivate::waitForWrite(int msecs)
{
    Q_Q(QProcess);

    if (!pipeWriter || pipeWriter->waitForWrite(msecs))
        return true;

    processError = QProcess::Timedout;
    q->setErrorString(QT_TRANSLATE_NOOP(QProcess, "Process opeation timed out"));
    return false;
}

void QProcessPrivate::notified()
{
    notifier->stop();

    if (!writeBuffer.isEmpty() && (!pipeWriter || pipeWriter->waitForWrite(0)))
        canWrite();

    if (bytesAvailableFromStdout())
        canReadStandardOutput();

    if (bytesAvailableFromStderr())
        canReadStandardError();

    notifier->start(NOTIFYTIMEOUT);
}

bool QProcessPrivate::startDetached(const QString &program, const QStringList &arguments)
{
    QString args = qt_create_commandline(program, arguments);

    bool success = false;

    PROCESS_INFORMATION pinfo;

#ifdef UNICODE
    if (!(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
        STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
                                     (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                     (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                   };
        success = CreateProcessW(0, (WCHAR*)args.utf16(),
                                 0, 0, TRUE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_CONSOLE, 0, 0,
                                 &startupInfo, &pinfo);
    } else
#endif // UNICODE
    {
#ifndef Q_OS_TEMP
       STARTUPINFOA startupInfo = { sizeof( STARTUPINFOA ), 0, 0, 0,
                                     (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                     (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                  };
       success = CreateProcessA(0, args.toLocal8Bit().data(),
                                0, 0, TRUE, CREATE_NEW_CONSOLE, 0, 0,
                                &startupInfo, &pinfo);
#endif // Q_OS_TEMP
    }

    if (success) {
        CloseHandle(pinfo.hThread);
        CloseHandle(pinfo.hProcess);
    }
    return success;
}


#include "qprocess_win.moc"

