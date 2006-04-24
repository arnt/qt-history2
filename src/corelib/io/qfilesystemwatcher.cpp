#include "qfilesystemwatcher.h"
#include "qfilesystemwatcher_p.h"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QMutex>
#include <QSet>
#include <QTimer>

#if defined(Q_OS_WIN)
#  include "qfilesystemwatcher_win_p.h"
#elif defined(Q_OS_LINUX)
#  include "qfilesystemwatcher_inotify_p.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
#  include "qfilesystemwatcher_kqueue_p.h"
#endif

enum { PollingInterval = 1000 };

class QPollingFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT

    mutable QMutex mutex;
    QHash<QString, QDateTime> files, directories;

public:
    void run();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private Q_SLOTS:
    void timeout();
};

void QPollingFileSystemWatcherEngine::run()
{
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), SLOT(timeout()));
    timer.start(PollingInterval);
    (void) exec();
}

QStringList QPollingFileSystemWatcherEngine::addPaths(const QStringList &paths,
                                                      QStringList *files,
                                                      QStringList *directories)
{
    QMutexLocker locker(&mutex);
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        if (!fi.exists())
            continue;
        if (fi.isDir()) {
            directories->append(path);
            this->directories.insert(path, fi.lastModified());
        } else {
            files->append(path);
            this->files.insert(path, fi.lastModified());
        }
        it.remove();
    }
    start();
    return p;
}

QStringList QPollingFileSystemWatcherEngine::removePaths(const QStringList &paths,
                                                         QStringList *files,
                                                         QStringList *directories)
{
    QMutexLocker locker(&mutex);
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        if (this->directories.remove(path)) {
            directories->removeAll(path);
            it.remove();
        } else if (this->files.remove(path)) {
            files->removeAll(path);
            it.remove();
        }
    }
    if (this->files.isEmpty() && this->directories.isEmpty()) {
        stop();
        wait();
    }
    return p;
}

void QPollingFileSystemWatcherEngine::stop()
{
    quit();
}

void QPollingFileSystemWatcherEngine::timeout()
{
    QMutexLocker locker(&mutex);
    QMutableHashIterator<QString, QDateTime> fit(files);
    while (fit.hasNext()) {
        QHash<QString, QDateTime>::iterator x = fit.next();
        QString path = x.key();
        QFileInfo fi(path);
        if (!fi.exists()) {
            fit.remove();
            emit fileChanged(path, true);
        } else if (x.value() != fi.lastModified()) {
            x.value() = fi.lastModified();
            emit fileChanged(path, false);
        }
    }
    QMutableHashIterator<QString, QDateTime> dit(directories);
    while (dit.hasNext()) {
        QHash<QString, QDateTime>::iterator x = dit.next();
        QString path = x.key();
        QFileInfo fi(path);
        if (!fi.exists()) {
            dit.remove();
            emit directoryChanged(path, true);
        } else if (x.value() != fi.lastModified()) {
            x.value() = fi.lastModified();
            emit directoryChanged(path, false);
        }
    }
}




QFileSystemWatcherEngine *QFileSystemWatcherPrivate::createNativeEngine()
{
#if defined(Q_OS_WIN)
    return new QWindowsFileSystemWatcherEngine;
#elif defined(Q_OS_LINUX)
    return QInotifyFileSystemWatcherEngine::create();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
    return QKqueueFileSystemWatcherEngine::create();
#else
    return 0;
#endif
}

QFileSystemWatcherPrivate::QFileSystemWatcherPrivate()
    : native(0), poller(0)
{
}

void QFileSystemWatcherPrivate::init()
{
    Q_Q(QFileSystemWatcher);
    native = createNativeEngine();
    if (native) {
        QObject::connect(native,
                         SIGNAL(fileChanged(const QString &, bool)),
                         q,
                         SLOT(_q_fileChanged(const QString &, bool)));
        QObject::connect(native,
                         SIGNAL(directoryChanged(const QString &, bool)),
                         q,
                         SLOT(_q_directoryChanged(const QString &, bool)));
    }
}

void QFileSystemWatcherPrivate::_q_fileChanged(const QString &path, bool removed)
{
    Q_Q(QFileSystemWatcher);
    if (!files.contains(path)) {
        // the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed)
        files.removeAll(path);
    emit q->fileChanged(path);
}

void QFileSystemWatcherPrivate::_q_directoryChanged(const QString &path, bool removed)
{
    Q_Q(QFileSystemWatcher);
    if (!directories.contains(path)) {
        // perhaps the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed)
        directories.removeAll(path);
    emit q->directoryChanged(path);
}



/*! \class QFileSystemWatcher
    \brief The QFileSystemWatcher class provides an interface for monitoring files and directories for modications.
    \ingroup io
    \reentrant

    QFileSystemWatcher monitors the file system for changes to files
    and directories.

    To watch a particlar file or directory, call addPath(). Multiple
    paths can be added using the addPaths() function. You can later
    remove these paths using removePath() and removePaths().

    QFileSystemWatcher examines each path added to it. Files that have
    been added to the QFileSystemWatcher can be accessed using the
    files() function, and directories using the directories()
    function.

    The fileChanged() signal is emitted when a file has been modified
    or removed from the disk. Similarly, the directoryChanged() signal
    is emitted when a directory has been modified or removed from
    disk. Note that QFileSystemWatcher stops monitoring files and
    directories once they have been removed from disk.
*/


/*!
    Constructs a new file system watcher object with the given \a
    parent.
*/
QFileSystemWatcher::QFileSystemWatcher(QObject *parent)
    : QObject(*new QFileSystemWatcherPrivate, parent)
{
    d_func()->init();
}

/*!
    Constructs a new file system watcher object with the given \a
    parent which monitors \a paths.
*/
QFileSystemWatcher::QFileSystemWatcher(const QStringList &paths, QObject *parent)
    : QObject(*new QFileSystemWatcherPrivate, parent)
{
    d_func()->init();
    addPaths(paths);
}

/*!
    Destroyed the file system watcher object.
*/
QFileSystemWatcher::~QFileSystemWatcher()
{
    Q_D(QFileSystemWatcher);
    if (d->native) {
        d->native->stop();
        d->native->wait();
        delete d->native;
        d->native = 0;
    }
    if (d->poller) {
        d->poller->stop();
        d->poller->wait();
        delete d->poller;
        d->poller = 0;
    }
}

/*!
    Adds \a path to the file system watcher if \a path exists. If \a
    path does not exist, this function does nothing.

    If \a path specifies a directory, the diretoryChanged() signal
    will be emitted when \a path is modified or removed from the disk;
    otherwise the fileChanged() signal is emitted when \a path is
    modified or removed from disk.
*/
void QFileSystemWatcher::addPath(const QString &path)
{
    addPaths(QStringList(path));
}

/*!
    Adds each path in \a paths to the file system watcher. If a path
    does not exist, This function ignores paths that do not exist.

    If a path specifies a directory, the diretoryChanged() signal will
    be emitted when the path is modified or removed from the disk;
    otherwise the fileChanged() signal is emitted when the path is
    modified or removed from disk.
*/
void QFileSystemWatcher::addPaths(const QStringList &paths)
{
    Q_D(QFileSystemWatcher);
    if (paths.isEmpty())
        return;
    QStringList p = paths;
    if (objectName() != QLatin1String("_qt_autotest_force_engine_poller")) {
        if (d->native)
            p = d->native->addPaths(p, &d->files, &d->directories);
        if (p.isEmpty())
            return;
    } else {
        qDebug() << "QFileSystemWatcher: skipping native engine, using only polling engine";
    }
    if (objectName() != QLatin1String("_qt_autotest_force_engine_native")) {
        // try polling instead
        if (!d->poller) {
            d->poller = new QPollingFileSystemWatcherEngine; // that was a mouthful
            QObject::connect(d->poller,
                             SIGNAL(fileChanged(const QString &, bool)),
                             this,
                             SLOT(_q_fileChanged(const QString &, bool)));
            QObject::connect(d->poller,
                             SIGNAL(directoryChanged(const QString &, bool)),
                             this,
                             SLOT(_q_directoryChanged(const QString &, bool)));
        }
        p = d->poller->addPaths(p, &d->files, &d->directories);
    } else{
        qDebug() << "QFileSystemWatcher: skipping polling engine, using only native engine";
    }
    if (!p.isEmpty())
        qWarning() << "QFileSystemWatcher: failed to add paths" << p;
}

/*!
    Removes \a path from the file system watcher.
*/
void QFileSystemWatcher::removePath(const QString &path)
{
    removePaths(QStringList(path));
}

/*!
    Removes \a paths from the file system watcher.
*/
void QFileSystemWatcher::removePaths(const QStringList &paths)
{
    Q_D(QFileSystemWatcher);
    QStringList p = paths;
    if (d->native)
        p = d->native->removePaths(p, &d->files, &d->directories);
    if (d->poller)
        (void) d->poller->removePaths(p, &d->files, &d->directories);
}

/*!
    \fn void fileChanged(const QString &path)

    Emitted when the file at \a path is modified or removed from the
    disk.
*/

/*!
    \fn void directoryChanged(const QString &path)

    Emitted when the directory at \a path is modified or removed from
    the disk.
*/

#include "moc_qfilesystemwatcher.cpp"
#include "qfilesystemwatcher.moc"
