/****************************************************************************
**
** Implementation of QDir class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qdir.h"

#ifndef QT_NO_DIR

#include "qdir_p.h"
#include "qfileinfo.h"
#include "qregexp.h"
#include "qstringlist.h"

#include <private/qmutexpool_p.h>

#include <stdlib.h>
#include <limits.h>
#include <errno.h>


void QDir::slashify(QString&)
{
}

QString QDir::homeDirPath()
{
    QString d;
    d = QFile::decodeName(QByteArray(getenv("HOME")));
    slashify(d);
    if (d.isNull())
        d = rootDirPath();
    return d;
}

QString QDir::canonicalPath() const
{
    QString r;
    char cur[PATH_MAX+1];
    if (::getcwd(cur, PATH_MAX)) {
        char tmp[PATH_MAX+1];
        // need the cast for old solaris versions of realpath that doesn't take
        // a const char*.
        if(::realpath(QFile::encodeName(dPath).data(), tmp))
            r = QFile::decodeName(QByteArray(tmp));
        if (!exists(r))
            r = QString();
        slashify(r);

        // always make sure we go back to the current dir
        ::chdir(cur);
    }
    return r;
}

bool QDir::mkdir(const QString &dirName, bool acceptAbsPath) const
{
#if defined(Q_OS_DARWIN)  // Mac X doesn't support trailing /'s
    QString name = dirName;
    if (dirName[dirName.length() - 1] == '/')
        name = dirName.left(dirName.length() - 1);
    int status =
        ::mkdir(QFile::encodeName(filePath(name,acceptAbsPath)), 0777);
#else
    int status =
        ::mkdir(QFile::encodeName(filePath(dirName,acceptAbsPath)), 0777);
#endif
    return status == 0;
}

bool QDir::rmdir(const QString &dirName, bool acceptAbsPath) const
{
    return ::rmdir(QFile::encodeName(filePath(dirName,acceptAbsPath))) == 0;
}

bool QDir::isReadable() const
{
    return ::access(QFile::encodeName(dPath), R_OK | X_OK) == 0;
}

bool QDir::isRoot() const
{
    return dPath == QString::fromLatin1("/");
}

bool QDir::rename(const QString &name, const QString &newName,
                   bool acceptAbsPaths       )
{
    if (name.isEmpty() || newName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name(s)");
        return false;
    }
    QString fn1 = filePath(name, acceptAbsPaths);
    QString fn2 = filePath(newName, acceptAbsPaths);
    return ::rename(QFile::encodeName(fn1),
                     QFile::encodeName(fn2)) == 0;
}

bool QDir::setCurrent(const QString &path)
{
    int r;
    r = ::chdir(QFile::encodeName(path));
    return r >= 0;
}

QString QDir::currentDirPath()
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
    slashify(result);
    return result;
}

QString QDir::rootDirPath()
{
    QString d = QString::fromLatin1("/");
    return d;
}

bool QDir::isRelativePath(const QString &path)
{
    int len = path.length();
    if (len == 0)
        return true;
    return path[0] != '/';
}

void QDir::readDirEntries(const QStringList &nameFilters, int filterSpec, int sortSpec) const
{
    int i;
    fList.clear();
    fiList.clear();

    bool doDirs            = (filterSpec & Dirs)        != 0;
    bool doFiles    = (filterSpec & Files)        != 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)        != 0;
    bool doWritable = (filterSpec & Writable)        != 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)        != 0;
    bool doSystem   = (filterSpec & System)     != 0;

    QFileInfo fi;
    DIR             *dir;
    dirent   *file;

    dir = opendir(QFile::encodeName(dPath));
    if (!dir)
        return; // cannot read the directory

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
        fi.setFile(*this, fn);
#ifndef QT_NO_REGEXP
        if (!match(nameFilters, fn) && !(allDirs && fi.isDir()))
             continue;
#endif
        if  ((doDirs && fi.isDir()) || (doFiles && fi.isFile()) ||
              (doSystem && (!fi.isFile() && !fi.isDir()))) {
            if (noSymLinks && fi.isSymLink())
                continue;
            if ((filterSpec & RWEMask) != 0)
                if ((doReadable && !fi.isReadable()) ||
                     (doWritable && !fi.isWritable()) ||
                     (doExecable && !fi.isExecutable()))
                    continue;
            if (!doHidden && fn[0] == '.' &&
                 fn != QString::fromLatin1(".")
                 && fn != QString::fromLatin1(".."))
                continue;
            fiList.append(fi);
        }
    }
    if (closedir(dir) != 0) {
        qWarning("QDir::readDirEntries: Cannot close the directory: %s",
                  dPath.local8Bit());
    }

    // Sort...
    if(fiList.count()) {
        QDirSortItem* si= new QDirSortItem[fiList.count()];
        for (i = 0; i < fiList.size(); ++i)
            si[i].item = fiList.at(i);
        qt_cmp_si_sortSpec = sortSpec;
        qsort(si, i, sizeof(si[0]), qt_cmp_si);
        // put them back in the list
        fiList.clear();
        for (int j = 0; j<i; j++) {
            fiList.append(si[j].item);
            fList.append(si[j].item.fileName());
        }
        delete [] si;
    }

    // mutable bitflags not supported by xlC - use that pointer instead.
    QDir *that = const_cast<QDir*>(this);
    if (filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
         nameFilters == nameFilts)
        that->dirty = false;
    else
        that->dirty = true;
}

QFileInfoList QDir::drives()
{
    QFileInfoList drives;
    // non-win32 versions both use just one root directory
    drives.append(rootDirPath());
    return drives;
}
#endif //QT_NO_DIR
