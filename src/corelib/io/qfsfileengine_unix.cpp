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

#include <qplatformdefs.h>
#ifndef QT_NO_REGEXP
# include <qregexp.h>
#endif

#include <qfileengine.h>
#include <private/qfsfileengine_p.h>
#include <qfile.h>
#include <qdir.h>
#include <qdebug.h>

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#if !defined(QWS) && defined(Q_OS_MAC)
# include <private/qcore_mac_p.h>
#endif

void QFSFileEnginePrivate::init()
{
}

int QFSFileEnginePrivate::sysOpen(const QString &fileName, int flags)
{
    return QT_OPEN(QFile::encodeName(fileName), flags, 0666);
}

bool QFSFileEngine::remove()
{
    Q_D(QFSFileEngine);
    return unlink(QFile::encodeName(d->file)) == 0;
}

bool QFSFileEngine::copy(const QString &)
{
    return false;
}

bool QFSFileEngine::rename(const QString &newName)
{
    Q_D(QFSFileEngine);
    return ::rename(QFile::encodeName(d->file), QFile::encodeName(newName)) == 0;
}

bool QFSFileEngine::link(const QString &newName)
{
    Q_D(QFSFileEngine);
    return ::symlink(QFile::encodeName(d->file), QFile::encodeName(newName)) == 0;
}

qint64 QFSFileEngine::size() const
{
    Q_D(const QFSFileEngine);
    QT_STATBUF st;
    int ret = 0;
    if(d->fd != -1)
        ret = QT_FSTAT(d->fd, &st);
    else
        ret = QT_STAT(QFile::encodeName(d->file), &st);
    if(ret == -1)
        return 0;
    return st.st_size;
}

bool QFSFileEngine::mkdir(const QString &name, bool createParentDirectories) const
{
    QString dirName = name;
    if(createParentDirectories) {
        dirName = QDir::cleanPath(dirName);
        for(int oldslash = -1, slash=0; slash != -1; oldslash = slash) {
            slash = dirName.indexOf(QDir::separator(), oldslash+1);
            if(slash == -1) {
                if(oldslash == dirName.length())
                    break;
                slash = dirName.length();
            }
            if(slash) {
                QByteArray chunk = QFile::encodeName(dirName.left(slash));
                QT_STATBUF st;
                if(QT_STAT(chunk, &st) != -1) {
                    if((st.st_mode & S_IFMT) != S_IFDIR)
                        return false;
                } else if(::mkdir(chunk, 0777) != 0) {
                        return false;
                }
            }
        }
        return true;
    }
#if defined(Q_OS_DARWIN)  // Mac X doesn't support trailing /'s
    if(dirName[dirName.length() - 1] == '/')
        dirName = dirName.left(dirName.length() - 1);
#endif
    return (::mkdir(QFile::encodeName(dirName), 0777) == 0);
}

bool QFSFileEngine::rmdir(const QString &name, bool recurseParentDirectories) const
{
    QString dirName = name;
    if(recurseParentDirectories) {
        dirName = QDir::cleanPath(dirName);
        for(int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            QByteArray chunk = QFile::encodeName(dirName.left(slash));
            QT_STATBUF st;
            if(QT_STAT(chunk, &st) != -1) {
                if((st.st_mode & S_IFMT) != S_IFDIR)
                    return false;
                if(::rmdir(chunk) != 0)
                    return oldslash != 0;
            } else {
                return false;
            }
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    return ::rmdir(QFile::encodeName(dirName)) == 0;
}

QStringList QFSFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    Q_D(const QFSFileEngine);
    const bool doDirs     = (filters & (QDir::Dirs | QDir::AllDirs)) != 0;
    const bool doFiles    = (filters & QDir::Files) != 0;
    const bool doSymLinks = (filters & QDir::NoSymLinks) == 0;
    const bool doReadable = (filters & QDir::Readable) != 0;
    const bool doWritable = (filters & QDir::Writable) != 0;
    const bool doExecable = (filters & QDir::Executable) != 0;
    const bool doHidden   = (filters & QDir::Hidden) != 0;
    const bool doSystem   = (filters & QDir::System) != 0;

    QStringList ret;
    DIR *dir = opendir(QFile::encodeName(d->file));
    if(!dir)
        return ret; // cannot read the directory

    QFileInfo fi;
    dirent   *file;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    union {
        struct dirent mt_file;
        char b[sizeof(struct dirent) + MAXNAMLEN + 1];
    } u;
    while (readdir_r(dir, &u.mt_file, &file) == 0 && file)
#else
    while ((file = readdir(dir)))
#endif // _POSIX_THREAD_SAFE_FUNCTIONS
    {
        QString fn = QFile::decodeName(QByteArray(file->d_name));
        fi.setFile(d->file + QLatin1Char('/') + fn);
#ifndef QT_NO_REGEXP
        if(!((filters & QDir::AllDirs) && fi.isDir())) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filterNames.begin(); sit != filterNames.end(); ++sit) {
                QRegExp rx(*sit,
                           (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                           QRegExp::Wildcard);
                if(rx.exactMatch(fn))
                    matched = true;
            }
            if(!matched)
                continue;
        }
#else
        Q_UNUSED(filterNames);
#endif
        if  ((doDirs && fi.isDir()) || (doFiles && fi.isFile()) ||
              (doSystem && (!fi.isFile() && !fi.isDir())) ||
              (doSymLinks && fi.isSymLink())) {
            if((filters & QDir::PermissionMask) != 0)
                if((doReadable && !fi.isReadable()) ||
                     (doWritable && !fi.isWritable()) ||
                     (doExecable && !fi.isExecutable()))
                    continue;
            if(!doHidden && fn.at(0) == QLatin1Char('.') && fn.length() > 1 && fn != QLatin1String(".."))
                continue;
            ret.append(fn);
        }
    }
    if(closedir(dir) != 0) {
        qWarning("QDir::readDirEntries: Cannot close the directory: %s", d->file.toLocal8Bit().data());
    }
    return ret;
}

bool QFSFileEngine::caseSensitive() const
{
    return true;
}

bool QFSFileEngine::setCurrentPath(const QString &path)
{
    int r;
    r = ::chdir(QFile::encodeName(path));
    return r >= 0;
}

QString QFSFileEngine::currentPath(const QString &)
{
    QString result;
    QT_STATBUF st;
    if(QT_STAT(".", &st) == 0) {
        char currentName[PATH_MAX+1];
        if(::getcwd(currentName, PATH_MAX))
            result = QFile::decodeName(QByteArray(currentName));
#if defined(QT_DEBUG)
        if(result.isNull())
            qWarning("QDir::currentPath: getcwd() failed");
#endif
    } else {
#if defined(QT_DEBUG)
        qWarning("QDir::currentPath: stat(\".\") failed");
#endif
    }
    return result;
}

QString QFSFileEngine::homePath()
{
    QString home = QFile::decodeName(qgetenv("HOME"));
    if(home.isNull())
        home = rootPath();
    return home;
}

QString QFSFileEngine::rootPath()
{
    return QString::fromLatin1("/");
}

QString QFSFileEngine::tempPath()
{
    QString temp = QFile::decodeName(qgetenv("TMPDIR"));
    if(temp.isEmpty())
        temp = QString::fromLatin1("/tmp/");
    return temp;
}

QFileInfoList QFSFileEngine::drives()
{
    QFileInfoList ret;
    ret.append(rootPath());
    return ret;
}

bool QFSFileEnginePrivate::doStat() const
{
    if(!tried_stat) {
        QFSFileEnginePrivate *that = const_cast<QFSFileEnginePrivate*>(this);
	that->tried_stat = true;
	that->could_stat = true;
        if(fd != -1) {
            that->could_stat = !QT_FSTAT(fd, &st);
        } else {
            const QByteArray file = QFile::encodeName(this->file);
            if(QT_LSTAT(file, &st) == 0)
                that->isSymLink = S_ISLNK(st.st_mode);
            that->could_stat = !QT_STAT(file, &st);
        }
    }
    return could_stat;
}

QFileEngine::FileFlags QFSFileEngine::fileFlags(QFileEngine::FileFlags type) const
{
    Q_D(const QFSFileEngine);
    QFileEngine::FileFlags ret = 0;
    if(!d->doStat())
        return ret;
    if(type & PermsMask) {
        if(d->st.st_mode & S_IRUSR)
            ret |= ReadOwnerPerm;
        if(d->st.st_mode & S_IWUSR)
            ret |= WriteOwnerPerm;
        if(d->st.st_mode & S_IXUSR)
            ret |= ExeOwnerPerm;
        if(d->st.st_mode & S_IRUSR)
            ret |= ReadUserPerm;
        if(d->st.st_mode & S_IWUSR)
            ret |= WriteUserPerm;
        if(d->st.st_mode & S_IXUSR)
            ret |= ExeUserPerm;
        if(d->st.st_mode & S_IRGRP)
            ret |= ReadGroupPerm;
        if(d->st.st_mode & S_IWGRP)
            ret |= WriteGroupPerm;
        if(d->st.st_mode & S_IXGRP)
            ret |= ExeGroupPerm;
        if(d->st.st_mode & S_IROTH)
            ret |= ReadOtherPerm;
        if(d->st.st_mode & S_IWOTH)
            ret |= WriteOtherPerm;
        if(d->st.st_mode & S_IXOTH)
            ret |= ExeOtherPerm;
    }
    if(type & TypesMask) {
#if !defined(QWS) && defined(Q_OS_MAC)
        bool foundAlias = false;
        {
            FSRef fref;
            if(FSPathMakeRef((const UInt8 *)QFile::encodeName(d->file).data(), &fref, NULL) == noErr) {
                Boolean isAlias, isFolder;
                if(FSIsAliasFile(&fref, &isAlias, &isFolder) == noErr && isAlias) {
                    foundAlias = true;
                    ret |= LinkType;
                }
            }
        }
        if(!foundAlias)
#endif
        {
            if(d->isSymLink)//(d->st.st_mode & S_IFMT) == S_IFLNK)
                ret |= LinkType;
            if((d->st.st_mode & S_IFMT) == S_IFREG)
                ret |= FileType;
            else if((d->st.st_mode & S_IFMT) == S_IFDIR)
                ret |= DirectoryType;
        }
    }
    if(type & FlagsMask) {
        ret |= QFileEngine::FileFlags(ExistsFlag | LocalDiskFlag);
        if(fileName(BaseName)[0] == QLatin1Char('.'))
            ret |= HiddenFlag;
        if(d->file == QLatin1String("/"))
            ret |= RootFlag;
    }
    return ret;
}

QString QFSFileEngine::fileName(FileName file) const
{
    Q_D(const QFSFileEngine);
    if(file == BaseName) {
        int slash = d->file.lastIndexOf(QLatin1Char('/'));
        if(slash != -1)
            return d->file.mid(slash + 1);
    } else if(file == PathName) {
        int slash = d->file.lastIndexOf(QLatin1Char('/'));
        if(slash == -1)
            return QLatin1String(".");
        else if(!slash)
            return QLatin1String("/");
        return d->file.left(slash);
    } else if(file == AbsoluteName || file == AbsolutePathName) {
        QString ret;
        if(!d->file.length() || d->file[0] != QLatin1Char('/'))
            ret = QDir::currentPath();
        if(!d->file.isEmpty() && d->file != QLatin1String(".")) {
            if(!ret.isEmpty() && ret.right(1) != QLatin1String("/"))
                ret += QLatin1Char('/');
            ret += d->file;
        }
        bool isDir = ret.endsWith(QLatin1String("/"));
        ret = QDir::cleanPath(ret);
        if (isDir)
            ret += QLatin1String("/");
        if(file == AbsolutePathName) {
            int slash = ret.lastIndexOf(QLatin1Char('/'));
            if(slash == -1)
                return QDir::currentPath();
            else if(!slash)
                return QLatin1String("/");
            return ret.left(slash);
        }
        return ret;
    } else if(file == CanonicalName || file == CanonicalPathName) {
        char cur[PATH_MAX+1];
        if(::getcwd(cur, PATH_MAX)) {
            QString ret;
            char real[PATH_MAX+1];
            // need the cast for old solaris versions of realpath that doesn't take
            // a const char*.
            if(::realpath(QFile::encodeName(d->file).data(), real))
                ret = QFile::decodeName(QByteArray(real));
            ::chdir(cur); // always make sure we go back to the current dir
            //check it
            QT_STATBUF st;
            if(QT_STAT(QFile::encodeName(ret), &st) != 0)
                ret = QString();
            if(!ret.isEmpty() && file == CanonicalPathName) {
                int slash = ret.lastIndexOf(QLatin1Char('/'));
                if(slash == -1)
                    return QDir::currentPath();
                else if(!slash)
                    return QLatin1String("/");
                return ret.left(slash);
            }
            return ret;
        }
        if(file == CanonicalPathName)
            return fileName(AbsolutePathName);
        return fileName(AbsoluteName);
    } else if(file == LinkName) {
        if(d->doStat() && d->isSymLink) {
            char s[PATH_MAX+1];
            int len = readlink(QFile::encodeName(d->file), s, PATH_MAX);
            if(len > 0) {
                QString ret;
                if (S_ISDIR(d->st.st_mode) && s[0] != '/') {
                    QDir parent(d->file);
                    parent.cdUp();
                    ret = parent.path();
                    if(!ret.isEmpty() && ret.right(1) != QLatin1String("/"))
                        ret += QLatin1Char('/');
                }
                s[len] = '\0';
                ret += QFile::decodeName(QByteArray(s));
                return ret;
            }
        }
#if !defined(QWS) && defined(Q_OS_MAC)
        {
            FSRef fref;
            if(FSPathMakeRef((const UInt8 *)QFile::encodeName(d->file).data(), &fref, NULL) == noErr) {
                Boolean isAlias, isFolder;
                if(FSResolveAliasFile(&fref, true, &isFolder, &isAlias) == noErr && isAlias) {
                    AliasHandle alias;
                    if(FSNewAlias(0, &fref, &alias) == noErr && alias) {
                        CFStringRef cfstr;
                        if(FSCopyAliasInfo(alias, 0, 0, &cfstr, 0, 0) == noErr)
                            return QCFString::toQString(cfstr);
                    }
                }
            }
        }
#endif
        return QString();
    }
    return d->file;
}

bool QFSFileEngine::isRelativePath() const
{
    Q_D(const QFSFileEngine);
    int len = d->file.length();
    if(len == 0)
        return true;
    return d->file[0] != QLatin1Char('/');
}

uint QFSFileEngine::ownerId(FileOwner own) const
{
    Q_D(const QFSFileEngine);
    static const uint nobodyID = (uint) -2;
    if(d->doStat()) {
        if(own == OwnerUser)
            return d->st.st_uid;
        else
            return d->st.st_gid;
    }
    return nobodyID;
}

QString QFSFileEngine::owner(FileOwner own) const
{
    if(own == OwnerUser) {
        passwd *pw = getpwuid(ownerId(own));
        if(pw)
            return QFile::decodeName(QByteArray(pw->pw_name));
    } else if(own == OwnerGroup) {
        struct group *gr = getgrgid(ownerId(own));
        if(gr)
            return QFile::decodeName(QByteArray(gr->gr_name));
    }
    return QString();
}

bool QFSFileEngine::chmod(uint perms)
{
    Q_D(QFSFileEngine);
    mode_t mode = 0;
    if(perms & ReadOwnerPerm)
        mode |= S_IRUSR;
    if(perms & WriteOwnerPerm)
        mode |= S_IWUSR;
    if(perms & ExeOwnerPerm)
        mode |= S_IXUSR;
    if(perms & ReadUserPerm)
        mode |= S_IRUSR;
    if(perms & WriteUserPerm)
        mode |= S_IWUSR;
    if(perms & ExeUserPerm)
        mode |= S_IXUSR;
    if(perms & ReadGroupPerm)
        mode |= S_IRGRP;
    if(perms & WriteGroupPerm)
        mode |= S_IWGRP;
    if(perms & ExeGroupPerm)
        mode |= S_IXGRP;
    if(perms & ReadOtherPerm)
        mode |= S_IROTH;
    if(perms & WriteOtherPerm)
        mode |= S_IWOTH;
    if(perms & ExeOtherPerm)
        mode |= S_IXOTH;
    if(d->fd != -1)
        return !fchmod(d->fd, mode);
    const QByteArray file = QFile::encodeName(d->file);
    return !::chmod(file.data(), mode);
}

bool QFSFileEngine::setSize(qint64 size)
{
    Q_D(QFSFileEngine);
    if(d->fd != -1)
        return !QT_FTRUNCATE(d->fd, size);
    const QByteArray file = QFile::encodeName(d->file);
    return !QT_TRUNCATE(file.data(), size);
}

