#ifndef QFILESYSTEMWATCHER_WIN_P_H
#define QFILESYSTEMWATCHER_WIN_P_H

#include "qfilesystemwatcher_p.h"

#include <windows.h>

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QVector>

class QWindowsFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT

public:
    QWindowsFileSystemWatcherEngine();
    ~QWindowsFileSystemWatcherEngine();

    void run();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private:
    void wakeup();

    QMutex mutex;
    QVector<HANDLE> handles;
    int msg;
    QHash<QString, HANDLE> handleForDir;
    class PathInfo {
    public:
        QString absolutePath;
        QString path;
        bool isDir;
        QDateTime timestamp;
    };
    QHash<HANDLE, QHash<QString, PathInfo> > pathInfoForHandle;
};

#endif // QFILESYSTEMWATCHER_WIN_P_H
