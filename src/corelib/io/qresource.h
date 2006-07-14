/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QRESOURCE_H
#define QRESOURCE_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <QtCore/qabstractfileengine.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QResourcePrivate;

class Q_CORE_EXPORT QResource
{
public:
    QResource(const QString &file=QString());
    ~QResource();

    void setFile(const QString &file);
    QString filePath() const;

    QString canonicalFilePath() const;

    bool isValid() const;

    bool isCompressed() const;
    qint64 size() const;
    const uchar *data() const;

    static void addSearchPath(const QString &path);
    static QStringList searchPaths();

    static bool registerResource(const QString &rccFilename, 
                                 const QString &mapRoot = QString());
    static bool unregisterResource(const QString &rccFilename,
                                   const QString &mapRoot = QString());


protected:
    friend class QResourceFileEngine;
    bool isDir() const;
    inline bool isFile() const { return !isDir(); }
    QStringList children() const;

protected:
    QResourcePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QResource)
};

class QResourceFileEnginePrivate;
class Q_CORE_EXPORT QResourceFileEngine : public QAbstractFileEngine
{
private:
    Q_DECLARE_PRIVATE(QResourceFileEngine)
public:
    explicit QResourceFileEngine(const QString &path);
    ~QResourceFileEngine();

    virtual void setFileName(const QString &file);

    virtual bool open(QIODevice::OpenMode flags) ;
    virtual bool close();
    virtual bool flush();
    virtual qint64 size() const;
    virtual qint64 pos() const;
    virtual bool atEnd() const;
    virtual bool seek(qint64);
    virtual qint64 read(char *data, qint64 maxlen);
    virtual qint64 write(const char *data, qint64 len);

    virtual bool remove();
    virtual bool copy(const QString &newName);
    virtual bool rename(const QString &newName);
    virtual bool link(const QString &newName);

    virtual bool isSequential() const;

    virtual bool isRelativePath() const;

    virtual bool mkdir(const QString &dirName, bool createParentDirectories) const;
    virtual bool rmdir(const QString &dirName, bool recurseParentDirectories) const;

    virtual bool setSize(qint64 size);

    virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

    virtual bool caseSensitive() const;

    virtual FileFlags fileFlags(FileFlags type) const;

    virtual bool setPermissions(uint perms);

    virtual QString fileName(QAbstractFileEngine::FileName file) const;

    virtual uint ownerId(FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QDateTime fileTime(FileTime time) const;

    bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0);
    bool supportsExtension(Extension extension) const;
};


QT_END_HEADER

#endif // QRESOURCE_H
