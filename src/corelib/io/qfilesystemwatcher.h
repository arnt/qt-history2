#ifndef QFILESYSTEMWATCHER_H
#define QFILESYSTEMWATCHER_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

class QFileSystemWatcherPrivate;

class QFileSystemWatcher : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFileSystemWatcher)

public:
    QFileSystemWatcher(QObject *parent = 0);
    QFileSystemWatcher(const QStringList &paths, QObject *parent = 0);
    ~QFileSystemWatcher();

    void addPath(const QString &file);
    void addPaths(const QStringList &files);
    void removePath(const QString &file);
    void removePaths(const QStringList &files);

    QStringList files() const;
    QStringList directories() const;

Q_SIGNALS:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_fileChanged(const QString &path, bool removed))
    Q_PRIVATE_SLOT(d_func(), void _q_directoryChanged(const QString &path, bool removed))
};

QT_END_HEADER

#endif // QFILESYSTEMWATCHER_H
