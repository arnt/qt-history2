/****************************************************************************
**
** Definition of QResourceEngine classes.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef __QRESOURCEENGINE_H__
#define __QRESOURCEENGINE_H__

#include <qdirengine.h>
#include <qfileengine.h>
#include <qfileinfoengine.h>

class QResourceDirEnginePrivate;
class QResourceDirEngine : public QDirEngine
{
private:
    Q_DECLARE_PRIVATE(QResourceDirEngine)
public:
    QResourceDirEngine(const QString &path);

    virtual void setPath(const QString &path);

    virtual bool mkdir(const QString &dirName, QDir::Recursivity recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursivity recurse) const;
    virtual bool rename(const QString &name, const QString &newName) const;
    virtual QStringList entryList(int filterSpec, const QStringList &filters) const;
    virtual bool caseSensitive() const;
    virtual bool isRoot() const;
};

class QResourceFileEnginePrivate;
class QResourceFileEngine : public QFileEngine
{
private:
    Q_DECLARE_PRIVATE(QResourceFileEngine)
public:
    QResourceFileEngine(const QString &file);

    virtual bool isOpen() const;
    virtual bool open(int flags);
    virtual bool close();
    virtual void flush();

    virtual Q_LONG readBlock(uchar *data, Q_LONG len);
    virtual Q_LONG writeBlock(const uchar *data, Q_LONG len);

    virtual void setFileName(const QString &file);

    virtual bool remove();
    virtual bool rename(const QString &newName);

    virtual QFile::Offset size() const;
    virtual QFile::Offset at() const;
    virtual bool atEnd() const;
    virtual bool seek(QFile::Offset pos);
    virtual bool isSequential() const;
    
    virtual int handle() const;

    //maybe
    virtual uchar *map(Q_LONG len);
};

class QResourceFileInfoEnginePrivate;
class QResourceFileInfoEngine : public QFileInfoEngine
{
private:
    Q_DECLARE_PRIVATE(QResourceFileInfoEngine)
public:
    QResourceFileInfoEngine(const QString &file);

    virtual uint fileFlags(uint type) const;

    virtual void setFileName(const QString &file);
    virtual QString fileName(FileName file) const;
    virtual bool isRelativePath() const;

    virtual uint ownerId(FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QIODevice::Offset size() const;

    virtual QDateTime fileTime(FileTime time) const;
};

#endif /* __QRESOURCEENGINE_H__ */
