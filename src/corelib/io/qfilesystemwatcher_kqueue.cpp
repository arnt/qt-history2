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

#include <qplatformdefs.h>

#include "qfilesystemwatcher.h"
#include "qfilesystemwatcher_kqueue_p.h"

#include <qdebug.h>
#include <qfile.h>
#include <qsocketnotifier.h>
#include <qvarlengtharray.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

// #define KEVENT_DEBUG
#ifdef KEVENT_DEBUG
#  define DEBUG qDebug
#else
#  define DEBUG if(false)qDebug
#endif

QKqueueFileSystemWatcherEngine *QKqueueFileSystemWatcherEngine::create()
{
    int kqfd = kqueue();
    if (kqfd == -1)
        return 0;
    return new QKqueueFileSystemWatcherEngine(kqfd);
}

QKqueueFileSystemWatcherEngine::QKqueueFileSystemWatcherEngine(int kqfd)
    : kqfd(kqfd)
{
    fcntl(kqfd, F_SETFD, FD_CLOEXEC);

    if (pipe(kqpipe) == -1) {
        perror("QKqueueFileSystemWatcherEngine: cannot create pipe");
        kqpipe[0] = kqpipe[1] = -1;
        return;
    }
    fcntl(kqpipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(kqpipe[1], F_SETFD, FD_CLOEXEC);

    struct kevent kev;
    EV_SET(&kev,
           kqpipe[0],
           EVFILT_READ,
           EV_ADD | EV_ENABLE,
           0,
           0,
           0);
    if (kevent(kqfd, &kev, 1, 0, 0, 0) == -1) {
        perror("QKqueueFileSystemWatcherEngine: cannot watch pipe, kevent returned");
        return;
    }
}

QKqueueFileSystemWatcherEngine::~QKqueueFileSystemWatcherEngine()
{
    stop();
    wait();

    close(kqfd);
    close(kqpipe[0]);
    close(kqpipe[1]);

    foreach (int id, pathToID.values())
        ::close(id < 0 ? -id : id);
}

QStringList QKqueueFileSystemWatcherEngine::addPaths(const QStringList &paths,
                                                     QStringList *files,
                                                     QStringList *directories)
{
    QMutexLocker locker(&mutex);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        int fd;
#if defined(O_EVTONLY)
        fd = ::open(QFile::encodeName(path), O_EVTONLY);
#else
        fd = ::open(QFile::encodeName(path), O_RDONLY);
#endif
        if (fd == -1) {
            perror("QKqueueFileSystemWatcherEngine::addPaths: open");
            continue;
        }

        struct stat st;
        if (fstat(fd, &st) == -1) {
            perror("QKqueueFileSystemWatcherEngine::addPaths: fstat");
            ::close(fd);
            continue;
        }
        int id = (S_ISDIR(st.st_mode)) ? -fd : fd;
        if (id < 0) {
            if (directories->contains(path)) {
                ::close(fd);
                continue;
            }
        } else {
            if (files->contains(path)) {
                ::close(fd);
                continue;
            }
        }

        struct kevent kev;
        EV_SET(&kev,
               fd,
               EVFILT_VNODE,
               EV_ADD | EV_ENABLE | EV_ONESHOT,
               NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE,
               0,
               0);
        if (kevent(kqfd, &kev, 1, 0, 0, 0) == -1) {
            perror("QKqueueFileSystemWatcherEngine::addPaths: kevent");
            ::close(fd);
            continue;
        }

        it.remove();
        if (id < 0) {
            DEBUG() << "QKqueueFileSystemWatcherEngine: added directory path" << path;
            directories->append(path);
        } else {
            DEBUG() << "QKqueueFileSystemWatcherEngine: added file path" << path;
            files->append(path);
        }

        pathToID.insert(path, id);
        idToPath.insert(id, path);
    }

    if (!isRunning())
        start();
    else
        write(kqpipe[1], "@", 1);

    return p;
}

QStringList QKqueueFileSystemWatcherEngine::removePaths(const QStringList &paths,
                                                        QStringList *files,
                                                        QStringList *directories)
{
    QMutexLocker locker(&mutex);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        int id = pathToID.take(path);
        QString x = idToPath.take(id);
        if (x.isEmpty() || x != path)
            continue;

        int fd = id < 0 ? -id : id;
        struct kevent kev;
        EV_SET(&kev,
               fd,
               EVFILT_VNODE,
               EV_DELETE,
               NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE,
               0,
               0);
        if (kevent(kqfd, &kev, 1, 0, 0, 0) == -1) {
            perror("QKqueueFileSystemWatcherEngine::removeWatch: kevent");
        }
        ::close(fd);

        it.remove();
        if (id < 0)
            directories->removeAll(path);
        else
            files->removeAll(path);
    }

    if (pathToID.isEmpty()) {
        stop();
        locker.unlock();
        wait();
        locker.relock();
    } else {
        write(kqpipe[1], "@", 1);
    }

    return p;
}

void QKqueueFileSystemWatcherEngine::stop()
{
    write(kqpipe[1], "q", 1);
}

void QKqueueFileSystemWatcherEngine::run()
{
    static const struct timespec ZeroTimeout = { 0, 0 };

    forever {
        struct kevent kev;
        DEBUG() << "QKqueueFileSystemWatcherEngine: waiting for kevents...";
        int r = kevent(kqfd, 0, 0, &kev, 1, 0);
        if (r < 0) {
            perror("QKqueueFileSystemWatcherEngine: error during kevent wait");
            return;
        }

        QMutexLocker locker(&mutex);
        do {
            int fd = kev.ident;

            DEBUG() << "QKqueueFileSystemWatcherEngine: processing kevent" << kev.ident << kev.filter;
            if (fd == kqpipe[0]) {
                char c;
                if (read(kqpipe[0], &c, 1) != 1) {
                    perror("QKqueueFileSystemWatcherEngine: error reading from pipe");
                    return;
                }
                switch (c) {
                case 'q':
                    DEBUG() << "QKqueueFileSystemWatcherEngine: thread received 'q', exiting...";
                    return;
                case '@':
                    DEBUG() << "QKqueueFileSystemWatcherEngine: thread received '@', continuing...";
                    break;
                default:
                    DEBUG() << "QKqueueFileSystemWatcherEngine: thread received unknow message" << c;
                    break;
                }
            } else {
                int id = fd;
                QString path = idToPath.value(id);
                if (path.isEmpty()) {
                    // perhaps a directory?
                    id = -id;
                    path = idToPath.value(id);
                    if (path.isEmpty()) {
                        DEBUG() << "QKqueueFileSystemWatcherEngine: received a kevent for a file we're not watching";
                        continue;
                    }
                }
                if (kev.filter != EVFILT_VNODE) {
                    DEBUG() << "QKqueueFileSystemWatcherEngine: received a kevent with the wrong filter";
                    continue;
                }

                if ((kev.fflags & (NOTE_DELETE | NOTE_REVOKE | NOTE_RENAME)) != 0) {
                    DEBUG() << path << "removed, removing watch also";

                    pathToID.remove(path);
                    idToPath.remove(id);
                    ::close(fd);

                    if (id < 0)
                        emit directoryChanged(path, true);
                    else
                        emit fileChanged(path, true);
                } else {
                    DEBUG() << path << "changed, re-enabling watch";

                    if (id < 0)
                        emit directoryChanged(path, false);
                    else
                        emit fileChanged(path, false);

                    // renable the watch
                    EV_SET(&kev,
                           fd,
                           EVFILT_VNODE,
                           EV_ADD | EV_ENABLE | EV_ONESHOT,
                           NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE,
                           0,
                           0);
                    if (kevent(kqfd, &kev, 1, 0, 0, 0) == -1) {
                        perror("QKqueueFileSystemWatcherEngine::processKqueueEvents: kevent EV_ADD");
                    }
                }
            }

            // are there any more?
            r = kevent(kqfd, 0, 0, &kev, 1, &ZeroTimeout);
        } while (r > 0);
    }
}
