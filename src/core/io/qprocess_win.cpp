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

void QProcessPrivate::createPipe(Q_PIPE pipe[2])
{
    // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles (this is done by the
    // DuplicateHandle() call).

    HANDLE tmpPipe;
    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
    if (!CreatePipe(&pipe[0], &tmpPipe, &secAtt, 0)) {
	    return; //### false;
	}
	if (!DuplicateHandle(GetCurrentProcess(), tmpPipe, GetCurrentProcess(), &pipe[1], 0, FALSE, DUPLICATE_SAME_ACCESS)) {
	    return; //### false;
	}
	if (!CloseHandle(tmpPipe)) {
	    return;  //### false;
	}
}

void QProcessPrivate::destroyPipe(Q_PIPE pipe[2])
{
    if (pipe[0] != INVALID_Q_PIPE) {
        CloseHandle(pipe[0]);
        pipe[0] = INVALID_Q_PIPE; 
    }
    if (pipe[1] != INVALID_Q_PIPE) {
        CloseHandle(pipe[1]);
        pipe[1] = INVALID_Q_PIPE;
    }

}

void QProcessPrivate::startProcess()
{
    Q_Q(QProcess);

    createPipe(standardReadPipe);
    createPipe(errorReadPipe);
    createPipe(writePipe);

    // construct the arguments for CreateProcess()
    QString args;
    QString appName = QString::null;

    for (int i=0; i<arguments.size(); ++i) {
        ///### handle .bat files
   	    QString tmp = arguments.at(i);
	    // escape a single " because the arguments will be parsed
	    tmp.replace( "\"", "\\\"" );
	    if (tmp.isEmpty() || tmp.contains(' ') || tmp.contains('\t')) {
	        // The argument must not end with a \ since this would be interpreted
	        // as escaping the quote -- rather put the \ behind the quote: e.g.
	        // rather use "foo"\ than "foo\"
	        QString endQuote("\"");
	        int i = tmp.length();
	        while (i>=0 && tmp.at(i-1) == '\\') {
                --i;
		        endQuote += "\\";
	        }
            args += QString(" \"") + tmp.left(i) + endQuote;
	    } else {
	        args += ' ' + tmp;
	    }
    }

    bool success;
    
    pid = new PROCESS_INFORMATION;
    memset(pid, 0, sizeof(PROCESS_INFORMATION));

    processState = QProcess::Starting;
    emit q->stateChanged(processState);

#ifdef UNICODE
    if (!(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
	    STARTUPINFOW startupInfo = {
	        sizeof( STARTUPINFO ), 0, 0, 0,
	        (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	        0, 0, 0,
	        STARTF_USESTDHANDLES,
	        0, 0, 0,
	        writePipe[0], standardReadPipe[1], errorReadPipe[1]
	    };
        //### why dump
	    WCHAR *applicationName = _wcsdup((const WCHAR*)program.constData());
	    WCHAR *commandLine = _wcsdup((const WCHAR*)args.ucs2());
        QByteArray envlist;
	    if (environment.isEmpty()) {
	        int pos = 0;
	        // add PATH if necessary (for DLL loading)
	        char *path = qgetenv("PATH");
	        if (environment.grep(QRegExp("^PATH=",FALSE)).empty() && path) {
		        QString tmp = QString("PATH=%1").arg(qgetenv("PATH"));
		        uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		        envlist.resize(envlist.size() + tmpSize );
		        memcpy(envlist.data()+pos, tmp.ucs2(), tmpSize);
		        pos += tmpSize;
	        }
	        // add the user environment
	        for (QStringList::Iterator it = environment.begin(); it != environment.end(); it++ ) {
		        QString tmp = *it;
		        uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		        envlist.resize(envlist.size() + tmpSize);
		        memcpy(envlist.data()+pos, tmp.ucs2(), tmpSize);
		        pos += tmpSize;
	        }
	        // add the 2 terminating 0 (actually 4, just to be on the safe side)
	        envlist.resize( envlist.size()+4 );
	        envlist[pos++] = 0;
	        envlist[pos++] = 0;
	        envlist[pos++] = 0;
	        envlist[pos++] = 0;
	    }
	    success = CreateProcessW(applicationName, commandLine,
		0, 0, TRUE,  CREATE_UNICODE_ENVIRONMENT, 
        environment.isEmpty() ? 0 : envlist.data(),
		(WCHAR*)workingDirectory.ucs2(),
		&startupInfo, (PROCESS_INFORMATION*)pid);
	
        free(applicationName);
	    free(commandLine);
    } else
#endif // UNICODE
    {
#ifndef Q_OS_TEMP
	    STARTUPINFOA startupInfo = { sizeof( STARTUPINFOA ), 0, 0, 0,
	        (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	        0, 0, 0,
	        STARTF_USESTDHANDLES,
	        0, 0, 0,
	        writePipe[0], standardReadPipe[1], errorReadPipe[1]
	    };
        QByteArray envlist;
	    if (environment.isEmpty()) {
            int pos = 0;
	        // add PATH if necessary (for DLL loading)
	        char *path = qgetenv("PATH");
	        if (environment.grep( QRegExp("^PATH=",FALSE) ).empty() && path ) {
		        QByteArray tmp = QString("PATH=%1").arg(qgetenv("PATH")).local8Bit();
		        uint tmpSize = tmp.length() + 1;
		        envlist.resize(envlist.size() + tmpSize);
		        memcpy(envlist.data()+pos, tmp.data(), tmpSize);
		        pos += tmpSize;
	        }
	        // add the user environment
	        for (QStringList::Iterator it = environment.begin(); it != environment.end(); it++) {
		        QByteArray tmp = (*it).local8Bit();
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
	    success = CreateProcessA(program.constData(), args.toLocal8Bit().data(),
		0, 0, TRUE, 0, environment.isEmpty() ? 0 : envlist.data(),
		workingDirectory.local8Bit(), &startupInfo, (PROCESS_INFORMATION*)pid);
#endif // Q_OS_TEMP
    }
    if (!success) {
	    delete pid;
        pid = 0;
        //### emit error
    }
#ifndef Q_OS_TEMP
    CloseHandle(writePipe[0]);
    standardReadPipe[1] = INVALID_Q_PIPE;
    CloseHandle(standardReadPipe[1]);
    standardReadPipe[1] = INVALID_Q_PIPE;
    CloseHandle(errorReadPipe[1]);
    errorReadPipe[1] = INVALID_Q_PIPE;
#endif
    processState = QProcess::Running;
    emit q->stateChanged(processState);
    emit q->started();
}

void QProcessPrivate::execChild()
{
    qWarning("QProcessPrivate::execChild() unimplemented for win32 (use Q3Process instead)");
}

bool QProcessPrivate::processStarted()
{
    qWarning("QProcessPrivate::processStarted() unimplemented for win32 (use Q3Process instead)");
    return false;
}

Q_LONGLONG QProcessPrivate::bytesAvailableFromStdout() const
{
    qWarning("QProcessPrivate::bytesAvailableFromStdout() unimplemented for win32 (use Q3Process instead)");
    return -1;
}

Q_LONGLONG QProcessPrivate::bytesAvailableFromStderr() const
{
    qWarning("QProcessPrivate::bytesAvailableFromStderr() unimplemented for win32 (use Q3Process instead)");
    return -1;
}

Q_LONGLONG QProcessPrivate::readFromStdout(char *data, Q_LONGLONG maxlen)
{
    qWarning("QProcessPrivate::readFromStdout() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

Q_LONGLONG QProcessPrivate::readFromStderr(char *data, Q_LONGLONG maxlen)
{
    qWarning("QProcessPrivate::readFromStderr() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

void QProcessPrivate::killProcess()
{
    qWarning("QProcessPrivate::killProcess() unimplemented for win32 (use Q3Process instead)");
}

bool QProcessPrivate::waitForStarted(int msecs)
{
    qWarning("QProcessPrivate::waitForStarted() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(msecs);
    return false;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
    qWarning("QProcessPrivate::waitForReadyRead() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(msecs);
    return false;
}

bool QProcessPrivate::waitForFinished(int msecs)
{
    qWarning("QProcessPrivate::waitForFinished() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(msecs);
    return false;
}

Q_LONGLONG QProcessPrivate::writeToStdin(const char *data, Q_LONGLONG maxlen)
{
    return 0;
}

bool QProcessPrivate::waitForWrite(int msecs)
{
    return false;
}

