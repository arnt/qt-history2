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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qfileengine.h>

#ifdef open
#error qresourceengine_p.h must be included before any system header that defines open
#endif

class QResourceFileEnginePrivate;
class QResourceFileEngine : public QFileEngine
{
private:
    Q_DECLARE_PRIVATE(QResourceFileEngine)
public:
    QResourceFileEngine(const QString &path);
    ~QResourceFileEngine();

    virtual void setFileName(const QString &file);

    virtual bool open(int flags) ;
    virtual bool close();
    virtual void flush();
    virtual Q_LONGLONG size() const;
    virtual Q_LONGLONG at() const;
    virtual bool atEnd() const;
    virtual bool seek(Q_LONGLONG);
    virtual int ungetch(int);
    virtual Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    virtual Q_LONGLONG write(const char *data, Q_LONGLONG len);

    virtual bool remove();
    virtual bool copy(const QString &newName);
    virtual bool rename(const QString &newName);
    virtual bool link(const QString &newName);

    virtual bool isSequential() const;

    virtual bool isRelativePath() const;

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const;

    virtual bool setSize(Q_LONGLONG size);

    virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

    virtual bool caseSensitive() const;

    virtual FileFlags fileFlags(FileFlags type) const;

    virtual bool chmod(uint perms);

    virtual QString fileName(QFileEngine::FileName file) const;

    virtual uint ownerId(FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QDateTime fileTime(FileTime time) const;

    virtual Type type() const;
};

#endif // QRESOURCEENGINE_P_H
