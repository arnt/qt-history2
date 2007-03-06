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

#include "qfilesystemwatcher.h"
#include "qfilesystemwatcher_dnotify_p.h"

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qsocketnotifier.h>
#include <qfileinfo.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

static int qfswd_fileChanged_pipe[2];
static void (*qfswd_old_sigio_handler)(int) = 0;
static void (*qfswd_old_sigio_action)(int, siginfo_t *, void *) = 0;
static void qfswd_sigio_monitor(int signum, siginfo_t *i, void *v)
{
    ::write(qfswd_fileChanged_pipe[1], &i->si_fd, sizeof(int));

    if (qfswd_old_sigio_handler && qfswd_old_sigio_handler != SIG_IGN)
        qfswd_old_sigio_handler(signum);
    if (qfswd_old_sigio_action)
        qfswd_old_sigio_action(signum, i, v);
}

QDnotifyFileSystemWatcherEngine::QDnotifyFileSystemWatcherEngine()
{
    ::pipe(qfswd_fileChanged_pipe);
    ::fcntl(qfswd_fileChanged_pipe[0], F_SETFL,
            ::fcntl(qfswd_fileChanged_pipe[0], F_GETFL) | O_NONBLOCK);
    
    struct sigaction oldAction;
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = qfswd_sigio_monitor;
    action.sa_flags = SA_SIGINFO;
    ::sigaction(SIGIO, &action, &oldAction);
    if (!(oldAction.sa_flags & SA_SIGINFO))
        qfswd_old_sigio_handler = oldAction.sa_handler;
    else
        qfswd_old_sigio_action = oldAction.sa_sigaction;
}

QDnotifyFileSystemWatcherEngine::~QDnotifyFileSystemWatcherEngine()
{
    for(QHash<int, Directory>::ConstIterator iter = fdToDirectory.begin();
            iter != fdToDirectory.end();
            ++iter)
        ::close(iter->fd);
}

QDnotifyFileSystemWatcherEngine *QDnotifyFileSystemWatcherEngine::create()
{
    return new QDnotifyFileSystemWatcherEngine();
}

void QDnotifyFileSystemWatcherEngine::run()
{
    QSocketNotifier sn(qfswd_fileChanged_pipe[0], QSocketNotifier::Read, this);
    connect(&sn, SIGNAL(activated(int)), SLOT(readFromDnotify()));
    (void) exec();
}

QStringList QDnotifyFileSystemWatcherEngine::addPaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QMutexLocker locker(&mutex);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);

        if(!fi.exists())
            continue;
        
        bool isDir = fi.isDir();

        if (isDir && directories->contains(path)) {
            continue; // Skip monitored directories
        } else if(!isDir && files->contains(path)) {
            continue; // Skip monitored files
        }

        QString fileName;
        if(!isDir) {
            path = fi.path();
            fileName = fi.fileName();
        }

        // Locate the directory entry (creating if needed)
        int fd = pathToFD[path];

        if(fd == 0) {
            DIR *d = ::opendir(path.toUtf8().constData());
            if(!d)
                continue; // Could not open directory

            fd = ::dirfd(d);
            Q_ASSERT(fd);
            if(::fcntl(fd, F_SETSIG, SIGIO) ||
               ::fcntl(fd, F_NOTIFY, DN_MODIFY | DN_CREATE | DN_DELETE |
                                     DN_RENAME | DN_ATTRIB | DN_MULTISHOT)) {
                ::closedir(d);
                continue; // Could not set appropriate flags
            }

            Directory dir;
            dir.path = path;
            dir.fd = fd;

            fdToDirectory.insert(fd, dir);
            pathToFD.insert(path, fd);
        }

        Directory &directory = fdToDirectory[fd];

        if(isDir) {
            directory.isMonitored = true;
        } else {
            Directory::File file;
            file.name = fileName;
            file.lastWrite = fi.lastModified();
            directory.files.append(file);
            pathToFD.insert(fi.filePath(), fd);
        }

        it.remove();

        if(isDir) {
            directories->append(path);
        } else {
            files->append(fi.filePath());
        }
    }

    start();

    return p;
}

QStringList QDnotifyFileSystemWatcherEngine::removePaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QMutexLocker locker(&mutex);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {

        QString path = it.next();
        int fd = pathToFD.take(path);

        if(!fd) 
            continue;

        Directory &directory = fdToDirectory[fd];
        bool isDir = false;
        if(directory.path == path) {
            isDir = true;
            directory.isMonitored = false;
        } else {
            for(int ii = 0; ii < directory.files.count(); ++ii) {
                if(directory.files.at(ii).name == path) {
                    directory.files.removeAt(ii);
                    break;
                }
            }
        }

        if(!directory.isMonitored && directory.files.isEmpty()) {
            // No longer needed
            ::close(directory.fd);
            it.remove();
        }

        if(isDir) {
            directories->removeAll(path);
        } else {
            files->removeAll(path);
        }
    }

    return p;
}

void QDnotifyFileSystemWatcherEngine::readFromDnotify()
{
    int fd;
    int readrv = ::read(qfswd_fileChanged_pipe[0], &fd,sizeof(int));
    Q_ASSERT(readrv == sizeof(int));
    Q_UNUSED(readrv);

    refresh(fd);
}

void QDnotifyFileSystemWatcherEngine::refresh(int fd)
{
    QMutexLocker locker(&mutex);

    QHash<int, Directory>::Iterator iter = fdToDirectory.find(fd);
    if(iter == fdToDirectory.end())
        return;

    Directory &directory = *iter;

    for(int ii = 0; ii < directory.files.count(); ++ii) {
        Directory::File &file = directory.files[ii];
        QFileInfo fi(file.name);
        QDateTime lm = fi.lastModified();
        if(lm != file.lastWrite) {
            // Emit signal
            QString filename = file.name;
            bool removed = fi.exists();

            if(removed) {
                directory.files.removeAt(ii);
                --ii;
            } else {
                file.lastWrite = lm;
            }

            emit fileChanged(filename, removed);
        }
    }

    if(directory.isMonitored) {
        // Emit signal
        bool removed = QFileInfo(directory.path).exists();
        QString path = directory.path;

        if(removed) 
            directory.isMonitored = false;

        emit directoryChanged(path, removed);
    } 

    if(!directory.isMonitored && directory.files.isEmpty()) {
        ::close(directory.fd);
        fdToDirectory.erase(iter);
    }
}

void QDnotifyFileSystemWatcherEngine::stop()
{
    QMetaObject::invokeMethod(this, "quit");
}

#endif // QT_NO_FILESYSTEMWATCHER

