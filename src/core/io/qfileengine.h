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

#include "qioengine.h"
#include "qdir.h"

class QFileEnginePrivate;
class Q_CORE_EXPORT QFileEngine : public QIOEngine
{
    Q_DECLARE_PRIVATE(QFileEngine)
public:
    virtual ~QFileEngine();

    static QFileEngine *createFileEngine(const QString &file);
    virtual void setFileName(const QString &file) = 0;

    virtual bool remove() = 0;
    virtual bool rename(const QString &newName) = 0;

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const = 0;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const = 0;

    virtual QStringList entryList(int filterSpec, const QStringList &filters) const = 0;

    virtual bool caseSensitive() const = 0;

    virtual bool isRelativePath() const = 0;

    enum FileInfo { 
        //perms (overlaps the QFileInfo permissions)
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
        ExistsFlag     = 0x0400000,
        RootFlag       = 0x0800000,
        LocalDiskFlag  = 0x0F00000,

        //masks
        PermsMask  = 0x0000FFFF,
        TypesMask  = 0x000F0000,
        FlagsMask  = 0x0FF00000,
        FileInfoAll = FlagsMask | PermsMask | TypesMask
    };
    virtual uint fileFlags(uint type=FileInfoAll) const = 0;

    enum FileName { DefaultName, BaseName, PathName, AbsoluteName, AbsolutePathName, LinkName, CanonicalName };
    virtual QString fileName(FileName file=DefaultName) const = 0;

    enum FileOwner { OwnerUser, OwnerGroup };
    virtual uint ownerId(FileOwner) const = 0;
    virtual QString owner(FileOwner) const = 0;

    enum FileTime { CreationTime, ModificationTime, AccessTime };
    virtual QDateTime fileTime(FileTime time) const = 0;

protected:
    QFileEngine();
    QFileEngine(QFileEnginePrivate &);
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

class QFSFileEnginePrivate;
class QFSFileEngine : public QFileEngine
{
private:
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QFSFileEngine(const QString &file);
    QFSFileEngine();

    virtual void setFileName(const QString &file);

    virtual bool open(int flags);
    virtual bool close();
    virtual void flush();
    virtual QIODevice::Offset size() const;
    virtual QIODevice::Offset at() const;
    virtual bool atEnd() const;
    virtual bool seek(QIODevice::Offset);
    virtual int ungetch(int);
    virtual Q_LONG readBlock(char *data, Q_LONG maxlen);
    virtual Q_LONG writeBlock(const char *data, Q_LONG len);
    virtual QString errorString() const;
    virtual QIODevice::Status errorStatus() const;
    virtual Type type() const;

    virtual bool remove();
    virtual bool rename(const QString &newName);

    virtual bool isSequential() const;

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const;

    virtual QStringList entryList(int filterSpec, const QStringList &filters) const;

    virtual bool caseSensitive() const;

    virtual bool isRelativePath() const;

    virtual uint fileFlags(uint type) const;

    virtual QString fileName(QFileEngine::FileName file) const;

    virtual uint ownerId(QFileEngine::FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QDateTime fileTime(FileTime time) const;

    //FS only!!
    bool open(int flags, int fd); 
    int handle() const;
    static bool setCurrentPath(const QString &path);
    static QString currentPath(const QString &path=QString::null);
    static QString homePath();
    static QString rootPath();
    static QString tempPath();
    static QFileInfoList drives();
};

#endif // QFILEENGINE_H
