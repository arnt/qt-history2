/****************************************************************************
**
** Implementation of QDir class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#include "qdir.h"
#include "qdir_p.h"
#include "qnamespace.h"
#include "qfileinfo.h"
#include "qregexp.h"
#include "qstringlist.h"
#include "qdeepcopy.h"
#include "qlibrary.h"

#  include <private/qmutexpool_p.h>

#include <windows.h>
#include <shlobj.h>
#include <limits.h>

static QString *theCWD = 0;

void QDir::slashify(QString& n)
{
    if (n.isNull())
        return;
    for (int i=0; i<(int)n.length(); i++) {
        if (n[i] ==  '\\')
            n[i] = '/';
    }
}

QString QDir::homeDirPath()
{
    typedef HRESULT (WINAPI *PtrSHGetSpecialFolderLocation)(HWND,int,LPITEMIDLIST*);
    typedef BOOL    (WINAPI *PtrSHGetPathFromIDList)(LPCITEMIDLIST,LPSTR);
    typedef HRESULT (WINAPI *PtrSHGetMalloc)(LPMALLOC*);

    static PtrSHGetSpecialFolderLocation ptrSHGetSpecialFolderLocation= 0;
    static PtrSHGetPathFromIDList ptrSHGetPathFromIDList = 0;
    static PtrSHGetMalloc ptrSHGetMalloc = 0;

    static bool shGSFLLookup = false;
    if (!shGSFLLookup) {
        shGSFLLookup = true;
        ptrSHGetSpecialFolderLocation = (PtrSHGetSpecialFolderLocation)QLibrary::resolve("ceshell", "SHGetSpecialFolderLocation");
        ptrSHGetPathFromIDList = (PtrSHGetPathFromIDList)QLibrary::resolve("ceshell", "SHGetPathFromIDList");
        ptrSHGetMalloc = (PtrSHGetMalloc)QLibrary::resolve("ceshell", "SHGetMalloc");
    }

    if (ptrSHGetSpecialFolderLocation) {
        LPMALLOC pIMalloc = 0;
        HRESULT res = ptrSHGetMalloc(&pIMalloc);
        Q_ASSERT(pIMalloc);

        LPITEMIDLIST il;
        if (NOERROR != ptrSHGetSpecialFolderLocation(0, CSIDL_PERSONAL, &il))
            return QString::null;
        char Path[MAX_PATH];
        ptrSHGetPathFromIDList(il, Path);
        QString d = QString::fromLocal8Bit(Path);
        pIMalloc->Free(il);
        slashify(d);
        return d;
    }
    return QString::null;
}


QString QDir::canonicalPath() const
{
    QString aPath = dPath.copy();
    if (aPath.length() && aPath[0] == '.')
        aPath.prepend(QDir::currentDirPath());
    slashify(aPath);

    QStringList aList = QStringList::split('/', aPath);
    QStringList::Iterator it = aList.begin();
    uint elm = aList.size(), len = elm;

    // Create new canonical path at the end
    // of the current string list, by pushing
    // and popping to the back
    while (len--) {
        if ((*it) == ".") {
            ++it;
            continue;
        } else if ((*it) == ".." &&
            aList.size() > elm) {
            aList.pop_back();
        } else {
            aList.push_back((*it));
        }
        ++it;
    }

    // Remove all the original elements, so
    // we are left with only the canonical.
    len = elm;
    while (len--)
        aList.pop_front();

    aPath = aList.join("/");
    return aPath.prepend('/');
}


bool QDir::mkdir(const QString &dirName, bool acceptAbsPath) const
{
    return ::_wmkdir((TCHAR*)filePath(dirName,acceptAbsPath).ucs2()) == 0;
}


bool QDir::rmdir(const QString &dirName, bool acceptAbsPath) const
{
    return ::_wrmdir((TCHAR*)filePath(dirName,acceptAbsPath).ucs2()) == 0;
}


bool QDir::isReadable() const
{
    return ::_waccess((TCHAR*)dPath.ucs2(), R_OK) == 0;
}


bool QDir::isRoot() const
{
    return dPath == "/" || dPath == "//" ||
        (dPath[0].isLetter() && dPath.mid(1,dPath.length()) == ":/");
}


bool QDir::rename(const QString &oldName, const QString &newName,
                   bool acceptAbsPaths       )
{
    if (oldName.isEmpty() || newName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name");
        return false;
    }
    QString fn1 = filePath(oldName, acceptAbsPaths);
    QString fn2 = filePath(newName, acceptAbsPaths);
    return ::_wrename((TCHAR*)fn1.ucs2(), (TCHAR*)fn2.ucs2()) == 0;
}


bool QDir::setCurrent(const QString &path)
{
    QMutexLocker locker(qt_global_mutexpool ?
                         qt_global_mutexpool->get(&theCWD) : 0);

    DWORD res = GetFileAttributes((TCHAR*)path.ucs2());
    if (0xFFFFFFFF == res)
        return false;

    if (! theCWD)
        theCWD = new QString(QDeepCopy<QString>(path));
    else
        *theCWD = QDeepCopy<QString>(path);

    slashify(*theCWD);
    return true;
}


QString QDir::currentDirPath()
{
    QMutexLocker locker(qt_global_mutexpool ?
                         qt_global_mutexpool->get(&theCWD) : 0);
    if (! theCWD)
        setCurrent("/");
    Q_ASSERT(theCWD != 0);
    return *theCWD;
}


QString QDir::rootDirPath()
{

    return QString("/");
}


bool QDir::isRelativePath(const QString &path)
{
    if (path.isEmpty())
        return true;

    return path[0] != '/' && path[0] != '\\';
}


bool QDir::readDirEntries(const QStringList &nameFilters,
                           int filterSpec, int sortSpec)
{
    int i;

    bool doDirs            = (filterSpec & Dirs)        != 0;
    bool doFiles    = (filterSpec & Files)        != 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)        != 0;
    bool doWritable = (filterSpec & Writable)        != 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)        != 0;

    // show hidden files if the user asks explicitly for e.g. .*
    if (!doHidden && !nameFilters.size()) {
        QStringList::ConstIterator sit = nameFilters.begin();
        while (sit != nameFilters.end()) {
            if ((*sit)[0] == '.') {
                doHidden = true;
                break;
            }
            ++sit;
        }
    }
    bool doModified = (filterSpec & Modified)        != 0;
    bool doSystem   = (filterSpec & System)        != 0;

    bool      first = true;
    QString   p = dPath.copy();
    int              plen = p.length();
    HANDLE    ff;
    WIN32_FIND_DATA finfo;
    QFileInfo fi;

#undef        IS_SUBDIR
#undef        IS_RDONLY
#undef        IS_ARCH
#undef        IS_HIDDEN
#undef        IS_SYSTEM
#undef        FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH            FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_ERROR    INVALID_HANDLE_VALUE

    if (plen == 0) {
        qWarning("QDir::readDirEntries: No directory name specified");
        return false;
    }
    if (p.at(plen-1) != '/' && p.at(plen-1) != '\\')
        p += '/';
    p += QString::fromLatin1("*.*");

    ff = FindFirstFile((TCHAR*)p.ucs2(), &finfo);

    if (!fList) {
        fList  = new QStringList;
    } else {
        fList->clear();
    }

    if (ff == FF_ERROR) {
        // if it is a floppy disk drive, it might just not have a file on it
        if (plen > 1 && p[1] == ':' &&
                (p[0]=='A' || p[0]=='a' || p[0]=='B' || p[0]=='b')) {
            if (!fiList) {
                fiList = new QFileInfoList;
            } else {
                fiList->clear();
            }
            return true;
        }
        qWarning("QDir::readDirEntries: Cannot read the directory: %s (UTF-8)",
                  dPath.utf8().data());
        return false;
    }

    if (!fiList) {
        fiList = new QFileInfoList;
    } else {
        fiList->clear();
    }

    for (;;) {
        if (first)
            first = false;
        else {
            if (!FindNextFile(ff,&finfo))
                break;
        }
        int  attrib = finfo.dwFileAttributes;
        bool isDir        = (attrib & IS_SUBDIR) != 0;
        bool isFile        = !isDir;
        bool isSymLink        = false;
        bool isReadable = true;
        bool isWritable = (attrib & IS_RDONLY) == 0;
        bool isExecable = false;
        bool isModified = (attrib & IS_ARCH)   != 0;
        bool isHidden        = (attrib & IS_HIDDEN) != 0;
        bool isSystem        = (attrib & IS_SYSTEM) != 0;

        QString fname = QString::fromUcs2((unsigned short *)finfo.cFileName);

        if (!match(nameFilters, fname) && !(allDirs && isDir))
            continue;

        if  ((doDirs && isDir) || (doFiles && isFile)) {
            QString name = fname;
            slashify(name);
            if (doExecable) {
                QString ext = name.right(4).lower();
                if (ext == ".exe" || ext == ".com" || ext == ".bat" ||
                     ext == ".pif" || ext == ".cmd")
                    isExecable = true;
            }

            if (noSymLinks && isSymLink)
                continue;
            if ((filterSpec & RWEMask) != 0)
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
            fi.setFile(*this, name);
            fiList->append(new QFileInfo(fi));
        }
    }
    FindClose(ff);

#undef        IS_SUBDIR
#undef        IS_RDONLY
#undef        IS_ARCH
#undef        IS_HIDDEN
#undef        IS_SYSTEM
#undef        FF_ERROR

    // Sort...
    QDirSortItem* si= new QDirSortItem[fiList->count()];
    QFileInfo* itm;
    i=0;
    for (itm = fiList->first(); itm; itm = fiList->next())
        si[i++].item = itm;
    qt_cmp_si_sortSpec = sortSpec;
    qsort(si, i, sizeof(si[0]), qt_cmp_si);
    // put them back in the list
    fiList->clear();
    int j;
    for (j=0; j<i; j++) {
        fiList->append(si[j].item);
        fList->append(si[j].item->fileName());
    }
    delete [] si;

    if (filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
         nameFilters == nameFilts)
        dirty = false;
    else
        dirty = true;
    return true;
}


QFileInfoList QDir::drives()
{
    QFileInfoList drives;
    drives.append(rootDirPath());
    return drives;
}
