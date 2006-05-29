/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
#include "qfilesystemwatcher_win_p.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <QSet>
#include <QDateTime>

QWindowsFileSystemWatcherEngine::QWindowsFileSystemWatcherEngine()
    : msg(0)
{
    HANDLE h = QT_WA_INLINE(CreateEventW(0, false, false, 0),
                                CreateEventA(0, false, false, 0));
    if (h) {
        handles.reserve(MAXIMUM_WAIT_OBJECTS);
        handles.append(h);
    }
}

QWindowsFileSystemWatcherEngine::~QWindowsFileSystemWatcherEngine()
{
    if (handles.isEmpty())
        return;

    stop();
    wait();

    CloseHandle(handles.at(0));
    handles[0] = 0;

    foreach (HANDLE h, handles) {
        if (!h)
            continue;
        FindCloseChangeNotification(h);
    }
}

void QWindowsFileSystemWatcherEngine::run()
{
    QMutexLocker locker(&mutex);
    forever {
        QVector<HANDLE> handlesCopy = handles;
        locker.unlock();
        // qDebug() << "QWindowsFileSystemWatcherEngine: waiting on" << handlesCopy.count() << "handles";
        DWORD r = WaitForMultipleObjects(handlesCopy.count(), handlesCopy.constData(), false, INFINITE);
        locker.relock();
        do {
            if (r == WAIT_OBJECT_0) {
                int m = msg;
                msg = 0;
                if (m == 'q') {
                    // qDebug() << "thread told to quit";
                    return;
                }
                if (m != '@')  {
                    qDebug("QWindowsFileSystemWatcherEngine: unknown message '%c' send to thread", char(m));
                }
                break;
            } else if (r > WAIT_OBJECT_0 && r < WAIT_OBJECT_0+handles.count()) {
                int at = r - WAIT_OBJECT_0;
                Q_ASSERT(at < handles.count());
                HANDLE handle = handles.at(at);
                if (!FindNextChangeNotification(handle))
                    qErrnoWarning("QFileSystemWatcher: FindNextChangeNotification failed");

                QHash<QString, PathInfo> &h = pathInfoForHandle[handle];
                QMutableHashIterator<QString, PathInfo> it(h);
                while (it.hasNext()) {
                    QHash<QString, PathInfo>::iterator x = it.next();
                    QFileInfo fileInfo(x.value().path);
                    // qDebug() << "checking" << x.key();
                    if (!fileInfo.exists()) {
                        // qDebug() << x.key() << "removed!";
                        if (x.value().isDir)
                            emit directoryChanged(x.value().path, true);
                        else
                            emit fileChanged(x.value().path, true);
                        h.erase(x);
                    } else if (fileInfo.lastModified() != x.value().timestamp) {
                        // qDebug() << x.key() << "changed!";
                        if (x.value().isDir)
                            emit directoryChanged(x.value().path, false);
                        else
                            emit fileChanged(x.value().path, false);
                        x.value().timestamp = fileInfo.lastModified();
                    }
                }
            } else {
                qErrnoWarning("QFileSystemWatcher: error while waiting for change notification");
            }
            r = WaitForMultipleObjects(handlesCopy.count(), handlesCopy.constData(), false, 0);
        } while (r != WAIT_TIMEOUT);
    }
}

QStringList QWindowsFileSystemWatcherEngine::addPaths(const QStringList &paths,
                                                      QStringList *files,
                                                      QStringList *directories)
{
    if (handles.isEmpty() || handles.count() == MAXIMUM_WAIT_OBJECTS)
        return paths;

    QMutexLocker locker(&mutex);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fileInfo(path.toLower());
        if (!fileInfo.exists())
            continue;

        bool isDir = fileInfo.isDir();
        const QString absolutePath = fileInfo.absolutePath();
        HANDLE handle = handleForDir.value(absolutePath);
        if (!handle) {
            QT_WA({
                handle = FindFirstChangeNotificationW((TCHAR *) absolutePath.utf16(),
                                                      false,
                                                      ((isDir
                                                        ? FILE_NOTIFY_CHANGE_DIR_NAME
                                                        : FILE_NOTIFY_CHANGE_FILE_NAME)
                                                       | FILE_NOTIFY_CHANGE_ATTRIBUTES
                                                       | FILE_NOTIFY_CHANGE_SIZE
                                                       | FILE_NOTIFY_CHANGE_LAST_WRITE
                                                       | FILE_NOTIFY_CHANGE_SECURITY));
            },{
                handle = FindFirstChangeNotificationA(absolutePath.toLocal8Bit(),
                                                      false,
                                                      ((isDir
                                                        ? FILE_NOTIFY_CHANGE_DIR_NAME
                                                        : FILE_NOTIFY_CHANGE_FILE_NAME)
                                                       | FILE_NOTIFY_CHANGE_ATTRIBUTES
                                                       | FILE_NOTIFY_CHANGE_SIZE
                                                       | FILE_NOTIFY_CHANGE_LAST_WRITE
                                                       | FILE_NOTIFY_CHANGE_SECURITY));
            })
            if (!handle)
                continue;
            handles.append(handle);
            handleForDir.insert(absolutePath, handle);
        }

        PathInfo pathInfo;
        pathInfo.absolutePath = absolutePath;
        pathInfo.isDir = isDir;
        pathInfo.path = path;
        pathInfo.timestamp = fileInfo.lastModified();
        pathInfoForHandle[handle].insert(fileInfo.absoluteFilePath(), pathInfo);

        if (isDir)
            directories->append(path);
        else
            files->append(path);

        it.remove();
    }

    if (!isRunning())
        start();
    else
        wakeup();

    return p;
}

QStringList QWindowsFileSystemWatcherEngine::removePaths(const QStringList &paths,
                                                         QStringList *files,
                                                         QStringList *directories)
{
    QMutexLocker locker(&mutex);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fileInfo(path.toLower());

        const QString absolutePath = fileInfo.absolutePath();
        HANDLE handle = handleForDir.value(absolutePath);
        if (!handle)
            continue;

        QHash<QString, PathInfo> &h = pathInfoForHandle[handle];
        if (h.remove(fileInfo.absoluteFilePath())) {
            // ###
            files->removeAll(path);
            directories->removeAll(path);

            if (h.isEmpty()) {
                FindCloseChangeNotification(handle);

                int indexOfHandle = handles.indexOf(handle);
                Q_ASSERT(indexOfHandle != -1);
                handles.remove(indexOfHandle);

                handleForDir.remove(absolutePath);
                // h is now invalid

                it.remove();
            }
        }
    }

    if (handleForDir.isEmpty()) {
        stop();
        locker.unlock();
        wait();
        locker.relock();
    } else {
        wakeup();
    }

    return p;
}

void QWindowsFileSystemWatcherEngine::stop()
{
    msg = 'q';
    SetEvent(handles.at(0));
}

void QWindowsFileSystemWatcherEngine::wakeup()
{
    msg = '@';
    SetEvent(handles.at(0));
}
