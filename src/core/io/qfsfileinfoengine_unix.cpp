/****************************************************************************
**
** Implementation of QFSFileInfoEngine class for Unix
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

#include <qfileinfoengine.h>
#include "qfileinfoengine_p.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <qatomic.h>
#include "qfile.h"
#include "qdir.h"

#define d d_func()
#define q q_func()

#include <stdlib.h>
#include <limits.h>

bool
QFSFileInfoEnginePrivate::doStat() const
{
    if(!tried_stat) {
        QFSFileInfoEnginePrivate *that = const_cast<QFSFileInfoEnginePrivate*>(this);
	that->tried_stat = true;
	that->could_stat = true;
	if (::stat(QFile::encodeName(file), &st) != 0)
	    that->could_stat = false;
    }
    return could_stat;
}

void 
QFSFileInfoEnginePrivate::init()
{
}

uint
QFSFileInfoEngine::fileFlags(uint type) const
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
QFSFileInfoEngine::fileName(FileName file) const
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
            if (ret.right(1) != QString::fromLatin1("/"))
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
        QString ret;
        char cur[PATH_MAX+1];
        if (::getcwd(cur, PATH_MAX)) {
            char real[PATH_MAX+1];
            // need the cast for old solaris versions of realpath that doesn't take
            // a const char*.
            if(::realpath(QFile::encodeName(d->file).data(), real))
                ret = QFile::decodeName(QByteArray(real));
            struct stat st;
            if (::stat(QFile::encodeName(ret), &st) != 0)
                ret = QString();
            // always make sure we go back to the current dir
            ::chdir(cur);
        }
        return ret;
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
QFSFileInfoEngine::isRelativePath() const
{
    int len = d->file.length();
    if (len == 0)
        return true;
    return d->file[0] != '/';
}

uint
QFSFileInfoEngine::ownerId(FileOwner own) const
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
QFSFileInfoEngine::owner(FileOwner own) const
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

