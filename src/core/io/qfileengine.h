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

#ifndef QFILEENGINE_H
#define QFILEENGINE_H

#include "QtCore/qdir.h"

#ifdef open
#error qfileengine.h must be included before any system header that defines open
#endif

class QFileEnginePrivate;
class Q_CORE_EXPORT QFileEngine
{
public:
    virtual ~QFileEngine();

    static QFileEngine *createFileEngine(const QString &file);
    virtual void setFileName(const QString &file) = 0;

    virtual bool open(int flags) = 0;
    virtual bool close() = 0;
    virtual void flush() = 0;

    virtual Q_LONGLONG size() const = 0;
    virtual Q_LONGLONG at() const = 0;
    virtual bool seek(Q_LONGLONG off) = 0;

    virtual bool isSequential() const = 0;

    virtual uchar *map(Q_LONGLONG off, Q_LONGLONG len);
    virtual void unmap(uchar *data);

    virtual Q_LONGLONG read(char *data, Q_LONGLONG maxlen) = 0;
    virtual Q_LONGLONG write(const char *data, Q_LONGLONG len) = 0;

    virtual QFile::Error error() const;
    virtual QString errorString() const;

    virtual bool remove() = 0;
    virtual bool copy(const QString &newName) = 0;
    virtual bool rename(const QString &newName) = 0;
    virtual bool link(const QString &newName) = 0;

    enum Type {
        File,
        Resource,

        User = 50,                                // first user type id
        MaxUser = 100                                // last user type id
    };
    virtual Type type() const = 0;

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const = 0;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const = 0;

    virtual bool setSize(Q_LONGLONG size) = 0;

    virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const = 0;

    virtual bool caseSensitive() const = 0;

    virtual bool isRelativePath() const = 0;

    enum FileFlag {
        //perms (overlaps the QFile::Permission)
        ReadOwnerPerm = 0x4000, WriteOwnerPerm = 0x2000, ExeOwnerPerm = 0x1000,
        ReadUserPerm  = 0x0400, WriteUserPerm  = 0x0200, ExeUserPerm  = 0x0100,
        ReadGroupPerm = 0x0040, WriteGroupPerm = 0x0020, ExeGroupPerm = 0x0010,
        ReadOtherPerm = 0x0004, WriteOtherPerm = 0x0002, ExeOtherPerm = 0x0001,

        //types
        LinkType      = 0x10000,
        FileType      = 0x20000,
        DirectoryType = 0x40000,

        //flags
        HiddenFlag     = 0x0100000,
        LocalDiskFlag  = 0x0200000,
        ExistsFlag     = 0x0400000,
        RootFlag       = 0x0800000,

        //masks
        PermsMask  = 0x0000FFFF,
        TypesMask  = 0x000F0000,
        FlagsMask  = 0x0FF00000,
        FileInfoAll = FlagsMask | PermsMask | TypesMask
    };
    Q_DECLARE_FLAGS(FileFlags, FileFlag)
    virtual FileFlags fileFlags(FileFlags type=FileInfoAll) const = 0;

    virtual bool chmod(uint perms) = 0;

    enum FileName { DefaultName, BaseName, PathName, AbsoluteName, AbsolutePathName, LinkName,
                    CanonicalName, CanonicalPathName };
    virtual QString fileName(FileName file=DefaultName) const = 0;

    enum FileOwner { OwnerUser, OwnerGroup };
    virtual uint ownerId(FileOwner) const = 0;
    virtual QString owner(FileOwner) const = 0;

    enum FileTime { CreationTime, ModificationTime, AccessTime };
    virtual QDateTime fileTime(FileTime time) const = 0;

protected:
    QFileEngine();
    QFileEngine(QFileEnginePrivate &);

    QFileEnginePrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QFileEngine)
};

class QFileEngineHandler
{
protected:
    QFileEngineHandler();
    virtual ~QFileEngineHandler();
    virtual QFileEngine *createFileEngine(const QString &path) = 0;

private:
    friend class QFileEngine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileEngine::FileFlags)

class QFSFileEnginePrivate;
class QFSFileEngine : public QFileEngine
{
public:
    QFSFileEngine();
    explicit QFSFileEngine(const QString &file);
    ~QFSFileEngine();

    virtual void setFileName(const QString &file);

    virtual bool open(int flags);
    virtual bool close();
    virtual void flush();
    virtual Q_LONGLONG size() const;
    virtual Q_LONGLONG at() const;
    virtual bool seek(Q_LONGLONG);
    virtual Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    virtual Q_LONGLONG write(const char *data, Q_LONGLONG len);
    virtual QFile::Error error() const;
    virtual QString errorString() const;

    virtual bool remove();
    virtual bool copy(const QString &newName);
    virtual bool rename(const QString &newName);
    virtual bool link(const QString &newName);

    virtual bool isSequential() const;

    virtual uchar *map(Q_LONGLONG off, Q_LONGLONG len);
    virtual void unmap(uchar *data);

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const;

    virtual bool setSize(Q_LONGLONG size);

    virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

    virtual bool caseSensitive() const;

    virtual bool isRelativePath() const;

    virtual FileFlags fileFlags(FileFlags type) const;

    virtual bool chmod(uint perms);

    virtual QString fileName(QFileEngine::FileName file) const;

    virtual uint ownerId(QFileEngine::FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QDateTime fileTime(FileTime time) const;

    virtual Type type() const;

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
private:
    Q_DECLARE_PRIVATE(QFSFileEngine)
};

#endif // QFILEENGINE_H
