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

#ifndef QFILEINFO_H
#define QFILEINFO_H

#ifndef QT_H
#include "qfile.h"
#endif // QT_H

class QDir;
class QDateTime;
class QFileInfoPrivate;

class Q_CORE_EXPORT QFileInfo
{
protected:
    QFileInfoPrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QFileInfo)
public:
#ifdef QT_COMPAT
    enum PermissionSpec {
        ReadOwner = QFile::ReadOwner, WriteOwner = QFile::WriteOwner, ExeOwner = QFile::ExeOwner,
        ReadUser  = QFile::ReadUser,  WriteUser  = QFile::WriteUser,  ExeUser  = QFile::ExeUser,
        ReadGroup = QFile::ReadGroup, WriteGroup = QFile::WriteGroup, ExeGroup = QFile::ExeGroup,
        ReadOther = QFile::ReadOther, WriteOther = QFile::WriteOther, ExeOther = QFile::ExeOther 
    };
#endif

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
    QString absoluteFilePath() const;
    QString canonicalFilePath() const;
    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    QString completeSuffix() const;
#ifdef QT_COMPAT
    inline QT_COMPAT QString baseName(bool complete) {
        if(complete)
            return completeBaseName();
        return baseName();
    }
    inline QT_COMPAT QString extension(bool complete = true) const { 
        if(complete)
            return completeSuffix();
        return suffix(); 
    }
    inline QT_COMPAT QString absFilePath() const { return absoluteFilePath(); }
#endif

    QString path() const;
    QString absolutePath() const;
#ifndef QT_NO_DIR
    QDir dir() const;
    QDir absoluteDir() const;
#endif
#ifdef QT_COMPAT
    inline QT_COMPAT QString dirPath(bool absPath = false) const {
        if(absPath)
            return absolutePath();
        return path();
    }
    QT_COMPAT QDir dir(bool absPath) const;
#endif

    QString canonicalPath() const;

    bool isReadable() const;
    bool isWritable() const;
    bool isExecutable() const;
    bool isHidden() const;

    bool isRelative() const;
    inline bool isAbsolute() const { return !isRelative(); }
    bool makeAbsolute();
#ifdef QT_COMPAT
    inline QT_COMPAT bool convertToAbs() { return makeAbsolute(); }
#endif

    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isRoot() const;

    QString readLink() const;

    QString owner() const;
    uint ownerId() const;
    QString group() const;
    uint groupId() const;

    bool permission(uint permissionSpec) const;
    uint permissions() const;

    QIODevice::Offset size() const;

    QDateTime created() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;

    void detach();

    bool caching() const;
    void setCaching(bool on);
};

typedef QList<QFileInfo> QFileInfoList;

#endif // QFILEINFO_H
