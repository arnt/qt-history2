/****************************************************************************
**
** Implementation of QFSDirEngine class for Windows
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define _POSIX_
#include <qdirengine.h>
#include "qdirengine_p.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <qatomic.h>
#include "qfile.h"
#include "qdir.h"
#ifndef QT_NO_REGEXP
# include <qregexp.h>
#endif

#include <private/qmutexpool_p.h>

#include <windows.h>
#include <direct.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <ctype.h>
#include <limits.h>

#define d d_func()
#define q q_func()

extern QByteArray qt_win95Name(const QString s);

bool 
QFSDirEngine::mkdir(const QString &dirName) const
{
    QT_WA({
        return ::_wmkdir((TCHAR*)dirName.utf16()) == 0;
    }, {
        return _mkdir(qt_win95Name(dirName)) == 0;
    });
}

bool 
QFSDirEngine::rmdir(const QString &dirName) const
{
 QT_WA({
        return ::_wrmdir((TCHAR*)dirName.utf16()) == 0;
    } , {
        return _rmdir(qt_win95Name(dirName)) == 0;
    });
}

bool 
QFSDirEngine::rename(const QString &name, const QString &newName) const
{
    QT_WA({
        return ::_wrename((TCHAR*)name.utf16(), (TCHAR*)newName.utf16()) == 0;
    } , {
        return ::rename(qt_win95Name(name), qt_win95Name(newName)) == 0;
    });
}

QStringList 
QFSDirEngine::entryInfoList(int filterSpec, const QStringList &filters) const
{
    QStringList ret;

    bool doDirs     = (filterSpec & QDir::Dirs)!= 0;
    bool doFiles    = (filterSpec & QDir::Files) != 0;
    bool noSymLinks = (filterSpec & QDir::NoSymLinks) != 0;
    bool doReadable = (filterSpec & QDir::Readable) != 0;
    bool doWritable = (filterSpec & QDir::Writable) != 0;
    bool doExecable = (filterSpec & QDir::Executable) != 0;
    bool doModified = (filterSpec & QDir::Modified) != 0;
    bool doSystem   = (filterSpec & QDir::System) != 0;
    bool doHidden   = (filterSpec & QDir::Hidden) != 0;
    // show hidden files if the user asks explicitly for e.g. .*
    if (!doHidden && !filters.size()) {
        QStringList::ConstIterator sit = filters.begin();
        while (sit != filters.end()) {
            if ((*sit)[0] == '.') {
                doHidden = true;
                break;
            }
            ++sit;
        }
    }

    bool first = true;
    QString p = d->path;
    const int plen = p.length();
    HANDLE ff;
    WIN32_FIND_DATA finfo;
    QFileInfo fi;

#undef IS_SUBDIR
#undef IS_RDONLY
#undef IS_ARCH
#undef IS_HIDDEN
#undef IS_SYSTEM
#undef FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH     FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_ERROR    INVALID_HANDLE_VALUE

    if (plen == 0) {
        qWarning("QDir::readDirEntries: No directory name specified");
        return ret;
    }
    if (p.at(plen-1) != '/' && p.at(plen-1) != '\\')
        p += '/';
    p += QString::fromLatin1("*.*");

    QT_WA({
        ff = FindFirstFile((TCHAR*)p.utf16(), &finfo);
    }, {
        // Cast is safe, since char is at end of WIN32_FIND_DATA
        ff = FindFirstFileA(qt_win95Name(p),(WIN32_FIND_DATAA*)&finfo);
    });

    if (ff == FF_ERROR) 
        return ret; // cannot read the directory

    for (;;) {
        if (first)
            first = false;
        else {
            QT_WA({
                if (!FindNextFile(ff,&finfo))
                    break;
            } , {
                if (!FindNextFileA(ff,(WIN32_FIND_DATAA*)&finfo))
                    break;
            });
        }
        int  attrib = finfo.dwFileAttributes;

        bool isDir      = (attrib & IS_SUBDIR) != 0;
        bool isFile     = !isDir;
        bool isSymLink  = false;
        bool isReadable = true;
        bool isWritable = (attrib & IS_RDONLY) == 0;
        bool isExecable = false;
        bool isModified = (attrib & IS_ARCH)   != 0;
        bool isHidden   = (attrib & IS_HIDDEN) != 0;
        bool isSystem   = (attrib & IS_SYSTEM) != 0;

        QString fname;
        QT_WA({
            fname = QString::fromUtf16((unsigned short *)finfo.cFileName);
        } , {
            fname = QString::fromLocal8Bit((const char*)finfo.cFileName);
        });

#ifndef QT_NO_REGEXP
        if(!(filterSpec & QDir::AllDirs && isDir)) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filters.begin(); sit != filters.end(); ++sit) {
                QRegExp rx(*sit, QString::CaseInsensitive, QRegExp::Wildcard);
                if (rx.exactMatch(fname)) {
                    matched = true;
                }
            }
            if(!matched)
                continue;
        }
#endif
        if  ((doDirs && isDir) || (doFiles && isFile)) {
            QString name = QDir::convertSeparators(fname);
            if (doExecable) {
                QString ext = name.right(4).toLower();
                if (ext == ".exe" || ext == ".com" || ext == ".bat" ||
                     ext == ".pif" || ext == ".cmd")
                    isExecable = true;
            }

            if (noSymLinks && isSymLink)
                continue;
            if ((filterSpec & QDir::RWEMask) != 0)
                if ((doReadable && !isReadable) ||
                     (doWritable && !isWritable) ||
                     (doExecable && !isExecable))
                    continue;
            if (doModified && !isModified)
                continue;
            if (!doHidden && isHidden)
                continue;
            if (!doSystem && isSystem)
                continue;
	    ret.append(name);
        }
    }
    FindClose(ff);

#undef        IS_SUBDIR
#undef        IS_RDONLY
#undef        IS_ARCH
#undef        IS_HIDDEN
#undef        IS_SYSTEM
#undef        FF_ERROR
    return ret;
}

bool 
QFSDirEngine::caseSensitive() const
{
    return false;
}

bool 
QFSDirEngine::isRoot() const
{
    return d->path == "/" || d->path == "//" ||
		    (d->path[0].isLetter() && d->path.mid(1,d->path.length()) == ":/");
}

bool 
QFSDirEngine::setCurrentDirPath(const QString &path)
{
    int r;
    QT_WA({
        r = ::_wchdir((TCHAR*)path.utf16());
    } , {
        r = QT_CHDIR(qt_win95Name(path));
    });
    return r >= 0;
}

QString 
QFSDirEngine::currentDirPath(const QString &path)
{
    QString ret;
    QT_WA({
        TCHAR currentName[PATH_MAX];
        if (::_wgetcwd(currentName,PATH_MAX) != 0) {
            ret = QString::fromUtf16((ushort*)currentName);
        }
    } , {
        char currentName[PATH_MAX];
        if (QT_GETCWD(currentName,PATH_MAX) != 0) {
            ret = QString::fromLocal8Bit(currentName);
        }
    });
    return QDir::convertSeparators(ret);
}

QString 
QFSDirEngine::homeDirPath()
{
    QString ret = QString::fromLocal8Bit(getenv("HOME"));
    if (ret.isEmpty() || !QFile::exists(ret)) {
        ret = QString::fromLocal8Bit(getenv("USERPROFILE"));
        if (ret.isEmpty() || !QFile::exists(ret)) {
            ret = QString::fromLocal8Bit(getenv("HOMEDRIVE")) + QString::fromLocal8Bit(getenv("HOMEPATH"));
            if (ret.isEmpty() || !QFile::exists(ret))
                ret = rootDirPath();
        }
    }
    return QDir::convertSeparators(ret);
}

QString
QFSDirEngine::rootDirPath()
{
#if defined(Q_FS_FAT)
    QString ret = QString::fromLatin1(getenv("SystemDrive"));
    if (ret.isEmpty())
        ret = "c:";
    ret += "/";
#elif defined(Q_OS_OS2EMX)
    char dir[4];
    _abspath(dir, "/", _MAX_PATH);
    QString ret(dir);
#endif
    return ret;
}

QFileInfoList
QFSDirEngine::drives()
{
    QFileInfoList ret;

#if defined(Q_OS_WIN32)
    Q_UINT32 driveBits = (Q_UINT32) GetLogicalDrives() & 0x3ffffff;
#elif defined(Q_OS_OS2EMX)
    Q_UINT32 driveBits, cur;
    if (DosQueryCurrentDisk(&cur,&driveBits) != NO_ERROR)
	exit(1);
    driveBits &= 0x3ffffff;
#endif
    char driveName[4];

#ifndef Q_OS_TEMP
    qstrcpy(driveName, "A:/");
#else
    qstrcpy(driveName, "/");
#endif
    while(driveBits) {
	if (driveBits & 1)
	    ret.append(QString::fromLatin1(driveName).toUpper());
	driveName[0]++;
	driveBits = driveBits >> 1;
    }
    return ret;
}

