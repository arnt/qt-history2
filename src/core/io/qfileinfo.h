/****************************************************************************
**
** Definition of QFileInfo class.
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
#ifndef __QFILEINFO_H__
#define __QFILEINFO_H__


#ifndef QT_H
#include "qfile.h"
#include "qdatetime.h"
#endif // QT_H

class QDir;
class QFileInfoPrivate;

class Q_CORE_EXPORT QFileInfo
{
protected:
    QFileInfoPrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QFileInfo)
public:
    enum PermissionSpec {
        ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
        ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
        ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
        ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001 };

    QFileInfo();
    QFileInfo(const QString &file);
    QFileInfo(const QFile &file);
#ifndef QT_NO_DIR
    QFileInfo(const QDir &dir, const QString &file);
#endif
    QFileInfo(const QFileInfo &fileinfo);
    ~QFileInfo();

    QFileInfo  &operator=(const QFileInfo &fileinfo);

    void setFile(const QString &file);
    void setFile(const QFile &file);
#ifndef QT_NO_DIR
    void setFile(const QDir &dir, const QString &file);
#endif
    bool exists() const;
    void refresh();

    QString filePath() const;
    QString fileName() const;
#ifndef QT_NO_DIR //###
    QString absFilePath() const;
#endif
    QString baseName(bool complete = false) const;
    QString extension(bool complete = true) const;

#ifndef QT_NO_DIR //###
    QString dirPath(bool absPath = false) const;
#endif
#ifndef QT_NO_DIR
    QDir dir(bool absPath = false) const;
#endif
    QString canonicalPath() const;

    bool isReadable() const;
    bool isWritable() const;
    bool isExecutable() const;
    bool isHidden() const;

#ifndef QT_NO_DIR //###
    bool isRelative() const;
    bool convertToAbs();
#endif

    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;

    QString readLink() const;

    QString owner() const;
    uint ownerId() const;
    QString group() const;
    uint groupId() const;

    bool permission(uint permissionSpec) const;

    QIODevice::Offset size() const;

    QDateTime created() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;

    void detach();

#ifdef QT_COMPAT
    inline QT_COMPAT bool caching() const { return true; }
    inline QT_COMPAT void setCaching(bool /*on*/) {}
#endif
};

typedef QList<QFileInfo> QFileInfoList;

#endif /* __QFILEINFO_H__ */
