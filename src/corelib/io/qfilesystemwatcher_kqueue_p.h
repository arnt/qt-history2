#ifndef FILEWATCHER_KQUEUE_P_H
#define FILEWATCHER_KQUEUE_P_H

#include "qfilesystemwatcher_p.h"

#include <QHash>
#include <QMutex>
#include <QThread>
#include <QVector>

struct kevent;

class QKqueueFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT
public:
    ~QKqueueFileSystemWatcherEngine();

    static QKqueueFileSystemWatcherEngine *create();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private:
    QKqueueFileSystemWatcherEngine(int kqfd);

    void run();

    int kqfd;
    int kqpipe[2];

    QMutex mutex;
    QHash<QString, int> pathToID;
    QHash<int, QString> idToPath;
};

#endif // FILEWATCHER_KQUEUE_P_H
