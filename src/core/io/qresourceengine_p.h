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

#include <qfileengine.h>

class QResourceFileEnginePrivate;
class QResourceFileEngine : public QFileEngine
{
private:
    Q_DECLARE_PRIVATE(QResourceFileEngine)
public:
    QResourceFileEngine(const QString &path);

    virtual void setFileName(const QString &file);

    virtual bool open(int flags) ;
    virtual bool close();
    virtual void flush();
    virtual QIODevice::Offset size() const;
    virtual QIODevice::Offset at() const;
    virtual bool atEnd() const;
    virtual bool seek(QIODevice::Offset);
    virtual int ungetch(int);
    virtual Q_LONG readBlock(char *data, Q_LONG maxlen);
    virtual Q_LONG writeBlock(const char *data, Q_LONG len);

    virtual bool remove();
    virtual bool rename(const QString &newName);

    virtual bool isSequential() const;

    virtual bool isRelativePath() const;

    virtual int handle() const;

    virtual uchar *map(Q_LONG len);

    virtual bool mkdir(const QString &dirName, QDir::Recursivity recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursivity recurse) const;

    virtual QStringList entryList(int filterSpec, const QStringList &filters) const;

    virtual bool caseSensitive() const;

    virtual bool isRoot() const;

    virtual uint fileFlags(uint type) const;

    virtual QString fileName(QFileEngine::FileName file) const;

    virtual uint ownerId(FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QDateTime fileTime(FileTime time) const;

    virtual Type type() const;
};

#endif /* __QRESOURCEENGINE_H__ */
