/****************************************************************************
**
** Implementation of QFSFileEngine class for Unix
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
#include <qfileengine.h>
#include "qfileengine_p.h"
#include <qplatformdefs.h>
#include <qfile.h>
#include <qdir.h>
#ifndef QT_NO_REGEXP
# include <qregexp.h>
#endif

#define d d_func()
#define q q_func()

#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

void 
QFSFileEnginePrivate::init()
{
}

int
QFSFileEnginePrivate::sysOpen(const QString &fileName, int flags)
{
    return ::open(QFile::encodeName(fileName), flags, 0666);
}

bool
QFSFileEngine::remove()
{
    return unlink(QFile::encodeName(d->file)) == 0;
}

bool 
QFSFileEngine::rename(const QString &newName)
{
    return ::rename(QFile::encodeName(d->file), QFile::encodeName(newName)) == 0;
}

QFile::Offset
QFSFileEngine::size() const
{
    QT_STATBUF st;
    int ret = 0;
    if(d->fd != -1) 
        ret = QT_FSTAT(d->fd, &st);
    else 
        ret = QT_STAT(QFile::encodeName(d->file), &st);
    if (ret == -1)
        return 0;
    return st.st_size;
}

uchar
*QFSFileEngine::map(Q_LONG /*len*/)
{
    return 0;
}

bool 
QFSFileEngine::mkdir(const QString &dirName, QDir::Recursivity /*recurse*/) const
{
#if defined(Q_OS_DARWIN)  // Mac X doesn't support trailing /'s
    QString name = dirName;
    if (dirName[dirName.length() - 1] == '/')
        name = dirName.left(dirName.length() - 1);
    int status = ::mkdir(QFile::encodeName(name), 0777);
#else
    int status = ::mkdir(QFile::encodeName(dirName), 0777);
#endif
    return status == 0;
}

bool 
QFSFileEngine::rmdir(const QString &dirName, QDir::Recursivity /*recurse*/) const
{
    return ::rmdir(QFile::encodeName(dirName)) == 0;
}

bool 
QFSFileEngine::rename(const QString &name, const QString &newName) const
{
    return ::rename(QFile::encodeName(name), QFile::encodeName(newName)) == 0;
}

QStringList 
QFSFileEngine::entryList(int filterSpec, const QStringList &filters) const
{
    const bool doDirs     = (filterSpec & QDir::Dirs) != 0;
    const bool doFiles    = (filterSpec & QDir::Files) != 0;
    const bool noSymLinks = (filterSpec & QDir::NoSymLinks) != 0;
    const bool doReadable = (filterSpec & QDir::Readable) != 0;
    const bool doWritable = (filterSpec & QDir::Writable) != 0;
    const bool doExecable = (filterSpec & QDir::Executable) != 0;
    const bool doHidden   = (filterSpec & QDir::Hidden) != 0;
    const bool doSystem   = (filterSpec & QDir::System) != 0;

    QStringList ret;
    DIR *dir = opendir(QFile::encodeName(d->file));
    if (!dir)
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
        fi.setFile(d->file + '/' + fn);
#ifndef QT_NO_REGEXP
        if(!((filterSpec & QDir::AllDirs) && fi.isDir())) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filters.begin(); sit != filters.end(); ++sit) {
                QRegExp rx(*sit, QString::CaseSensitive, QRegExp::Wildcard);
                if (rx.exactMatch(fn)) 
                    matched = true;
            }
            if(!matched)
                continue;
        }
#endif
        if  ((doDirs && fi.isDir()) || (doFiles && fi.isFile()) ||
              (doSystem && (!fi.isFile() && !fi.isDir()))) {
            if (noSymLinks && fi.isSymLink())
                continue;
            if ((filterSpec & QDir::RWEMask) != 0)
                if ((doReadable && !fi.isReadable()) ||
                     (doWritable && !fi.isWritable()) ||
                     (doExecable && !fi.isExecutable()))
                    continue;
            if (!doHidden && fn[0] == '.' &&
                 fn != QString::fromLatin1(".")
                 && fn != QString::fromLatin1(".."))
                continue;
            ret.append(fn);
        }
    }
    if (closedir(dir) != 0) {
        qWarning("QDir::readDirEntries: Cannot close the directory: %s",
                  d->file.local8Bit());
    }
    return ret;
}

bool 
QFSFileEngine::caseSensitive() const
{
    return true;
}

bool 
QFSFileEngine::isRoot() const
{
    return d->file == QString::fromLatin1("/");
}

bool 
QFSFileEngine::setCurrentDirPath(const QString &path)
{
    int r;
    r = ::chdir(QFile::encodeName(path));
    return r >= 0;
}

QString 
QFSFileEngine::currentDirPath(const QString &)
{
    QString result;
    struct stat st;
    if (::stat(".", &st) == 0) {
        char currentName[PATH_MAX+1];
        if (::getcwd(currentName, PATH_MAX))
            result = QFile::decodeName(QByteArray(currentName));
#if defined(QT_DEBUG)
        if (result.isNull())
            qWarning("QDir::currentDirPath: getcwd() failed");
#endif
    } else {
#if defined(QT_DEBUG)
        qWarning("QDir::currentDirPath: stat(\".\") failed");
#endif
    }
    return result;
}

QString 
QFSFileEngine::homeDirPath()
{
    QString home = QFile::decodeName(QByteArray(getenv("HOME")));
    if (home.isNull())
        home = rootDirPath();
    return home;
}

QString
QFSFileEngine::rootDirPath()
{
    return QString::fromLatin1("/");
}

QFileInfoList
QFSFileEngine::drives()
{
    QFileInfoList ret;
    ret.append(rootDirPath());
    return ret;
}

bool
QFSFileEnginePrivate::doStat() const
{
    if(!tried_stat) {
        QFSFileEnginePrivate *that = const_cast<QFSFileEnginePrivate*>(this);
	that->tried_stat = true;
	that->could_stat = true;
        if(d->fd != -1) 
            that->could_stat = !QT_FSTAT(d->fd, &st);
        else 
            that->could_stat = !QT_STAT(QFile::encodeName(d->file), &st);
    }
    return could_stat;
}

uint
QFSFileEngine::fileFlags(uint type) const
{
    uint ret = 0;
    if(!d->doStat())
        return ret;
    if(type & PermsMask) {
        if (d->st.st_mode & S_IRUSR)
            ret |= ReadOwner;
        if (d->st.st_mode & S_IWUSR)
            ret |= WriteOwner;
        if (d->st.st_mode & S_IXUSR)
            ret |= ExeOwner;
        if (d->st.st_mode & S_IRUSR)
            ret |= ReadUser;
        if (d->st.st_mode & S_IWUSR)
            ret |= WriteUser;
        if (d->st.st_mode & S_IXUSR)
            ret |= ExeUser;
        if (d->st.st_mode & S_IRGRP)
            ret |= ReadGroup;
        if (d->st.st_mode & S_IWGRP)
            ret |= WriteGroup;
        if (d->st.st_mode & S_IXGRP)
            ret |= ExeGroup;
        if (d->st.st_mode & S_IROTH)
            ret |= ReadOther;
        if (d->st.st_mode & S_IWOTH)
            ret |= WriteOther;
        if (d->st.st_mode & S_IXOTH)
            ret |= ExeOther;
    }
    if(type & TypeMask) {
        if((d->st.st_mode & S_IFMT) == S_IFREG)
            ret |= File;
        if((d->st.st_mode & S_IFMT) == S_IFLNK)
            ret |= Link;
        if((d->st.st_mode & S_IFMT) == S_IFDIR)
            ret |= Directory;
    }
    if(type & FlagsMask) {
        ret |= Exists;
        if(fileName(BaseName)[0] == QChar('.'))
            ret |= Hidden;
    }
    return ret;
}

QString
QFSFileEngine::fileName(FileName file) const
{
    if(file == BaseName) {
        int slash = d->file.lastIndexOf('/');
        if (slash != -1) 
            return d->file.mid(slash + 1);
    } else if(file == DirPath) {
        int slash = d->file.lastIndexOf('/');
        if (slash == -1)
            return QString::fromLatin1(".");
        else if (!slash)
            return QString::fromLatin1("/");
        return d->file.left(slash);
    } else if(file == AbsoluteName || file == AbsoluteDirPath) {
        QString ret;
        if(!d->file.length() || d->file[0] != '/') 
            ret = QDir::currentDirPath();
        if(!d->file.isEmpty() && d->file != ".") {
            if (!ret.isEmpty() && ret.right(1) != QString::fromLatin1("/"))
                ret += '/';
            ret += d->file;
        }
        if(file == AbsoluteDirPath) {
            int slash = ret.lastIndexOf('/');
            if (slash == -1)
                return QDir::currentDirPath();
            else if (!slash)
                return QString::fromLatin1("/");
            return ret.left(slash);
        }
        return ret;
    } else if(file == Canonical) {
        char cur[PATH_MAX+1];
        if (::getcwd(cur, PATH_MAX)) {
            QString ret;
            char real[PATH_MAX+1];
            // need the cast for old solaris versions of realpath that doesn't take
            // a const char*.
            if(::realpath(QFile::encodeName(d->file).data(), real))
                ret = QFile::decodeName(QByteArray(real));
            // always make sure we go back to the current dir
            ::chdir(cur);
            //check it
            struct stat st;
            if (::stat(QFile::encodeName(ret), &st) != 0)
                ret = QString();
            return ret;
        }
        return fileName(AbsoluteName);
    } else if(file == LinkName) {
        if(d->doStat() && (d->st.st_mode & S_IFMT) == S_IFLNK) {
            char s[PATH_MAX+1];
            int len = readlink(QFile::encodeName(d->file), s, PATH_MAX);
            if (len >= 0) {
                s[len] = '\0';
                return QFile::decodeName(QByteArray(s));
            }
        }
        return QString();
    }
    return d->file;
}

bool
QFSFileEngine::isRelativePath() const
{
    int len = d->file.length();
    if (len == 0)
        return true;
    return d->file[0] != '/';
}

uint
QFSFileEngine::ownerId(FileOwner own) const
{
    static const uint nobodyID = (uint) -2;
    if(d->doStat()) {
        if(own == User) 
            return d->st.st_uid;
        else
            return d->st.st_gid;
    }
    return nobodyID;
}

QString
QFSFileEngine::owner(FileOwner own) const
{
    if(own == User) {
        passwd *pw = getpwuid(ownerId(own));
        if (pw)
            return QFile::decodeName(QByteArray(pw->pw_name));
    } else if(own == Group) {
        struct group *gr = getgrgid(ownerId(own));
        if (gr)
            return QFile::decodeName(QByteArray(gr->gr_name));
    }
    return QString::null;
}

