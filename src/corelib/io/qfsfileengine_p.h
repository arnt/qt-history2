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

#ifndef QFSFILEENGINE_H
#define QFSFILEENGINE_H

#include "qfileengine.h"
#include "private/qfileengine_p.h"

class QFSFileEnginePrivate;
class QFSFileEngine : public QFileEngine
{
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QFSFileEngine();
    explicit QFSFileEngine(const QString &file);
    ~QFSFileEngine();

    void setFileName(const QString &file);

    bool open(int flags);
    bool close();
    void flush();
    qint64 size() const;
    qint64 at() const;
    bool seek(qint64);
    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);
    QFile::FileError error() const;
    QString errorString() const;

    bool remove();
    bool copy(const QString &newName);
    bool rename(const QString &newName);
    bool link(const QString &newName);

    bool isSequential() const;

    uchar *map(qint64 off, qint64 len);
    void unmap(uchar *data);

    bool mkdir(const QString &dirName, bool createParentDirectories) const;
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const;

    bool setSize(qint64 size);

    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

    bool caseSensitive() const;

    bool isRelativePath() const;

    FileFlags fileFlags(FileFlags type) const;

    bool chmod(uint perms);

    QString fileName(QFileEngine::FileName file) const;

    uint ownerId(QFileEngine::FileOwner) const;
    QString owner(FileOwner) const;

    QDateTime fileTime(FileTime time) const;

    Type type() const;

    //FS only!!
    bool open(int flags, int fd);
    int handle() const;
    static bool setCurrentPath(const QString &path);
    static QString currentPath(const QString &path = QString());
    static QString homePath();
    static QString rootPath();
    static QString tempPath();
    static QFileInfoList drives();

protected:
    QFSFileEngine(QFSFileEnginePrivate &dd);
};

class QFSFileEnginePrivate : public QFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileEngine)

public:
#ifdef Q_WS_WIN
    static QString fixToQtSlashes(const QString &path);
    static QByteArray win95Name(const QString &path);
#else
    static inline QString fixToQtSlashes(const QString &path) { return path; }
#endif

    QString file;

    inline void resetErrors() {
        error = QFile::UnspecifiedError;
        errorString.clear();
    }
    inline void setError(QFile::FileError err, int errorCode) {
        error = err;
        errorString = qt_error_string(errorCode);
    }
    inline void setError(QFile::FileError err, QString errStr = QString()) {
        error = err;
        errorString = errStr;
    }
    QFile::FileError error;
    QString errorString;

    int fd;
    mutable uint sequential : 1;
    mutable uint external_file : 1;
    QByteArray ungetchBuffer;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
#ifdef Q_OS_UNIX
    mutable uint isSymLink : 1;
#endif
    mutable QT_STATBUF st;
    bool doStat() const;
    int sysOpen(const QString &, int flags);

protected:
    QFSFileEnginePrivate();

    void init();

#if defined(Q_OS_WIN32)
    QFileEngine::FileFlags getPermissions() const;
    QString getLink() const;
#endif
};

#endif

