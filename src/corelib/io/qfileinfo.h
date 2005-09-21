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

#include <QtCore/qfile.h>
#include <QtCore/qlist.h>

QT_MODULE(Core)

class QDir;
class QDateTime;
class QFileInfoPrivate;

class Q_CORE_EXPORT QFileInfo
{
public:
    QFileInfo();
    QFileInfo(const QString &file);
    QFileInfo(const QFile &file);
    QFileInfo(const QDir &dir, const QString &file);
    QFileInfo(const QFileInfo &fileinfo);
    ~QFileInfo();

    QFileInfo &operator=(const QFileInfo &fileinfo);
    bool operator==(const QFileInfo &fileinfo);
    inline bool operator!=(const QFileInfo &fileinfo) { return !(operator==(fileinfo)); }

    void setFile(const QString &file);
    void setFile(const QFile &file);
    void setFile(const QDir &dir, const QString &file);
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

    QString path() const;
    QString absolutePath() const;
    QString canonicalPath() const;
    QDir dir() const;
    QDir absoluteDir() const;

    bool isReadable() const;
    bool isWritable() const;
    bool isExecutable() const;
    bool isHidden() const;

    bool isRelative() const;
    inline bool isAbsolute() const { return !isRelative(); }
    bool makeAbsolute();

    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isRoot() const;

    QString readLink() const;

    QString owner() const;
    uint ownerId() const;
    QString group() const;
    uint groupId() const;

    bool permission(QFile::Permissions permissions) const;
    QFile::Permissions permissions() const;

    qint64 size() const;

    QDateTime created() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;

    void detach();

    bool caching() const;
    void setCaching(bool on);

#ifdef QT3_SUPPORT
    enum Permission {
        ReadOwner = QFile::ReadOwner, WriteOwner = QFile::WriteOwner, ExeOwner = QFile::ExeOwner,
        ReadUser  = QFile::ReadUser,  WriteUser  = QFile::WriteUser,  ExeUser  = QFile::ExeUser,
        ReadGroup = QFile::ReadGroup, WriteGroup = QFile::WriteGroup, ExeGroup = QFile::ExeGroup,
        ReadOther = QFile::ReadOther, WriteOther = QFile::WriteOther, ExeOther = QFile::ExeOther
    };
    Q_DECLARE_FLAGS(PermissionSpec, Permission)

    inline QT3_SUPPORT QString baseName(bool complete) {
        if(complete)
            return completeBaseName();
        return baseName();
    }
    inline QT3_SUPPORT QString extension(bool complete = true) const {
        if(complete)
            return completeSuffix();
        return suffix();
    }
    inline QT3_SUPPORT QString absFilePath() const { return absoluteFilePath(); }

    inline QT3_SUPPORT QString dirPath(bool absPath = false) const {
        if(absPath)
            return absolutePath();
        return path();
    }
    QT3_SUPPORT QDir dir(bool absPath) const;
    inline QT3_SUPPORT bool convertToAbs() { return makeAbsolute(); }
#if !defined(Q_NO_TYPESAFE_FLAGS)
    inline QT3_SUPPORT bool permission(PermissionSpec permissions) const
    { return permission(QFile::Permissions((int)permissions)); }
#endif
#endif

protected:
    QFileInfoPrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QFileInfo)
};
Q_DECLARE_TYPEINFO(QFileInfo, Q_MOVABLE_TYPE);

#ifdef QT3_SUPPORT
Q_DECLARE_OPERATORS_FOR_FLAGS(QFileInfo::PermissionSpec)
#endif

typedef QList<QFileInfo> QFileInfoList;
#ifdef QT3_SUPPORT
typedef QList<QFileInfo>::Iterator QFileInfoListIterator;
#endif

#endif // QFILEINFO_H
