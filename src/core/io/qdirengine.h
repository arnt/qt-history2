/****************************************************************************
**
** Definition of QDirEngine and QFSDirEngine classes.
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
#ifndef __QDIRENGINE_H__
#define __QDIRENGINE_H__

#include "qdir.h"
#include "qfileinfo.h"

class QDirEnginePrivate;

/* It seems to me cd/cdup must operate on a string rather than using cwd (or possibly
   doing cwd if it is a non-virtual file system).
*/
class QDirEngine
{
protected:
    QDirEnginePrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QDirEngine)
public:
    QDirEngine(QDirEnginePrivate &);
    virtual ~QDirEngine();

    virtual bool setPath(const QString &path, bool force) = 0;

    virtual bool mkdir(const QString &dirName) const = 0;
    virtual bool rmdir(const QString &dirName) const = 0;
    virtual bool rename(const QString &name, const QString &newName) const = 0;
    virtual QStringList entryList(int filterSpec, const QStringList &filters) const = 0;
    virtual bool caseSensitive() const = 0;
    virtual bool isRoot() const = 0;
};

class QFSDirEnginePrivate;
class QFSDirEngine : public QDirEngine
{
private:
    Q_DECLARE_PRIVATE(QFSDirEngine)
public:
    QFSDirEngine(const QString &path);

    virtual bool setPath(const QString &path, bool force);

    virtual bool mkdir(const QString &dirName) const;
    virtual bool rmdir(const QString &dirName) const;
    virtual bool rename(const QString &name, const QString &newName) const;
    virtual QStringList entryList(int filterSpec, const QStringList &filters) const;
    virtual bool caseSensitive() const;
    virtual bool isRoot() const;

    static bool setCurrentDirPath(const QString &path);
    static QString currentDirPath(const QString &path=QString::null);
    static QString homeDirPath();
    static QString rootDirPath();
    static QFileInfoList drives();
};


#endif /* __QDIRENGINE_H__ */
