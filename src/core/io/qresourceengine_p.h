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

#ifndef QRESOURCEENGINE_P_H
#define QRESOURCEENGINE_P_H

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

    virtual uchar *map(Q_LONG len);

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const;

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

#endif // QRESOURCEENGINE_P_H
