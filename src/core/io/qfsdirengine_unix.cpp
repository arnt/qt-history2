/****************************************************************************
**
** Implementation of QFSDirEngine class for Unix
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

#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>

#define d d_func()
#define q q_func()

bool 
QFSDirEngine::mkdir(const QString &dirName, QDir::Recursivity /*recurse*/) const
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
QFSDirEngine::rmdir(const QString &dirName, QDir::Recursivity /*recurse*/) const
{
    return ::rmdir(QFile::encodeName(dirName)) == 0;
}

bool 
QFSDirEngine::rename(const QString &name, const QString &newName) const
{
    return ::rename(QFile::encodeName(name), QFile::encodeName(newName)) == 0;
}

QStringList 
QFSDirEngine::entryList(int filterSpec, const QStringList &filters) const
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
    DIR *dir = opendir(QFile::encodeName(d->path));
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
        fi.setFile(d->path + '/' + fn);
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
                  d->path.local8Bit());
    }
    return ret;
}

bool 
QFSDirEngine::caseSensitive() const
{
    return true;
}

bool 
QFSDirEngine::isRoot() const
{
    return d->path == QString::fromLatin1("/");
}

bool 
QFSDirEngine::setCurrentDirPath(const QString &path)
{
    int r;
    r = ::chdir(QFile::encodeName(path));
    return r >= 0;
}

QString 
QFSDirEngine::currentDirPath(const QString &)
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
QFSDirEngine::homeDirPath()
{
    QString home = QFile::decodeName(QByteArray(getenv("HOME")));
    if (home.isNull())
        home = rootDirPath();
    return home;
}

QString
QFSDirEngine::rootDirPath()
{
    return QString::fromLatin1("/");
}

QFileInfoList
QFSDirEngine::drives()
{
    QFileInfoList ret;
    ret.append(rootDirPath());
    return ret;
}

