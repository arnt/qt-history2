/****************************************************************************
**
** Implementation of QDir class.
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

#include "qdir.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <qstring.h>
#include <qdirengine.h>
#include <qregexp.h>

#define d d_func()
#define q q_func()

#include <stdlib.h>

//************* QDirEngineHandler
static QList<QDirEngineHandler*> *dirHandlers = 0;
QDirEngineHandler::QDirEngineHandler()
{
    if(!dirHandlers)
        dirHandlers = new QList<QDirEngineHandler*>;
    dirHandlers->append(this);
}

QDirEngineHandler::~QDirEngineHandler()
{
    dirHandlers->removeAll(this);
    if(dirHandlers->isEmpty()) {
        delete dirHandlers;
        dirHandlers = 0;
    }
}
QDirEngine *qt_createDirEngine(const QString &path)
{
    if(dirHandlers) {
        for(int i = 0; i < dirHandlers->size(); i++) {
            if(QDirEngine *ret = dirHandlers->at(i)->createDirEngine(path))
                return ret;
        }
    }
    return new QFSDirEngine(path);
}

//************* QDirPrivate
class QDirPrivate
{
    QDir *q_ptr;
    Q_DECLARE_PUBLIC(QDir)

protected:
    QDirPrivate(QDir*, const QDir *copy=0);
    ~QDirPrivate();

    static QDirEngine *createDirEngine(const QString &file);
    void initDirEngine(const QString &file);

    void updateFileLists() const;
    void sortFileList(int, QStringList &, QStringList *, QFileInfoList *) const;

private:
#ifdef QT_COMPAT
    QChar filterSepChar;
#endif
    struct Data {
        inline Data() : dirEngine(0) { ref = 1; clear(); }
        inline ~Data() { delete dirEngine; }
        inline void clear() {
            listsDirty = 1;
            fileInfoDirty = 1;
        }
        mutable QAtomic ref;

        QString path;
        QStringList nameFilters;
        int sortSpec, filterSpec;

        mutable QDirEngine *dirEngine;

        mutable uint fileInfoDirty : 1;
        mutable QFileInfo fi;

        mutable uint listsDirty : 1;
        mutable QStringList files;
        mutable QFileInfoList fileInfos;
    } *data;
    inline bool setPath(const QString &p, bool force) 
    {
        detach();
        if(!data->dirEngine || !QDir::isRelativePath(p))
            initDirEngine(p);
        if(data->dirEngine->setPath(p, force)) {
            data->path = p;
            data->clear();
            return true;
        }
        return false;
    }
    inline void reset() {
        detach();
        data->clear();
    }
    void detach();
};

QDirPrivate::QDirPrivate(QDir *qq, const QDir *copy) : q_ptr(qq)
#ifdef QT_COMPAT
                                                     , filterSepChar(0)
#endif
{ 
    if(copy) {
        ++copy->d->data->ref;
        data = copy->d->data;
    } else {
        data = new QDirPrivate::Data;
        data->clear();
    }
}

QDirPrivate::~QDirPrivate() 
{ 
    if (!--data->ref)
        delete data;
    data = 0;
    q_ptr = 0; 
}

/* For sorting */
struct QDirSortItem {
    QString filename_cache;
    QFileInfo item;
};
static int qt_cmp_si_sortSpec;

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
static int __cdecl qt_cmp_si(const void *n1, const void *n2)
#else
static int qt_cmp_si(const void *n1, const void *n2)
#endif
{
    if (!n1 || !n2)
        return 0;

    QDirSortItem* f1 = (QDirSortItem*)n1;
    QDirSortItem* f2 = (QDirSortItem*)n2;

    if (f1->item.isDir() != f2->item.isDir()) {
        if (qt_cmp_si_sortSpec & QDir::DirsFirst)
            return f1->item.isDir() ? -1 : 1;
        if (qt_cmp_si_sortSpec & QDir::DirsLast)
            return f1->item.isDir() ? 1 : -1;
    }

    int r = 0;
    int sortBy = qt_cmp_si_sortSpec & QDir::SortByMask;

    switch (sortBy) {
      case QDir::Time:
        r = f1->item.lastModified().secsTo(f2->item.lastModified());
        break;
      case QDir::Size:
        r = f2->item.size() - f1->item.size();
        break;
      default:
        ;
    }

    if (r == 0 && sortBy != QDir::Unsorted) {
        // Still not sorted - sort by name
        bool ic = qt_cmp_si_sortSpec & QDir::IgnoreCase;

        if (f1->filename_cache.isNull())
            f1->filename_cache = ic ? f1->item.fileName().toLower()
                                    : f1->item.fileName();
        if (f2->filename_cache.isNull())
            f2->filename_cache = ic ? f2->item.fileName().toLower()
                                    : f2->item.fileName();

        r = f1->filename_cache.compare(f2->filename_cache);
    }

    if (r == 0) // Enforce an order - the order the items appear in the array
        r = (char*)n1 - (char*)n2;

    if (qt_cmp_si_sortSpec & QDir::Reversed)
        return -r;
    return r;
}

#if defined(Q_C_CALLBACKS)
}
#endif

inline void QDirPrivate::sortFileList(int sortSpec, QStringList &l, 
                                       QStringList *names, QFileInfoList *infos) const
{
    if(names)
        names->clear();
    if(infos)
        infos->clear();
    if(!l.isEmpty()) {
        QDirSortItem *si= new QDirSortItem[l.count()];
        int i;
        for (i = 0; i < l.size(); ++i) 
            si[i].item = QFileInfo(data->path + QDir::separator() + l.at(i));
        qt_cmp_si_sortSpec = sortSpec;
        qsort(si, i, sizeof(si[0]), qt_cmp_si);
        // put them back in the list(s)
        for (int j = 0; j<i; j++) {
            if(infos)
                infos->append(si[j].item);
            if(names)
                names->append(si[j].item.fileName());
        }
        delete [] si;
    }
}

inline void QDirPrivate::updateFileLists() const
{
    if(data->listsDirty) {
        QStringList l = data->dirEngine->entryList(data->filterSpec, data->nameFilters);
        sortFileList(data->sortSpec, l, &data->files, &data->fileInfos); 
        data->listsDirty = 0;
    }
}

void QDirPrivate::initDirEngine(const QString &path)
{
    detach();
    delete data->dirEngine;
    data->dirEngine = 0;
    data->clear();
    data->dirEngine = createDirEngine(path);
}

QDirEngine *QDirPrivate::createDirEngine(const QString &path)
{
    return qt_createDirEngine(path);
}

void 
QDirPrivate::detach()
{
    if (data->ref != 1) {
        QDirPrivate::Data *x = data;
        data = new QDirPrivate::Data;
        data->path = x->path;
        data->nameFilters = x->nameFilters;
        data->sortSpec = x->sortSpec;
        data->filterSpec = x->filterSpec;
        initDirEngine(data->path);
        --x->ref;
    }
}

//************* QDir
inline static QStringList qt_makeFilterStringList(const QString &nameFilter)
{
    if (nameFilter.isEmpty())
        return QStringList();

    QChar sep(';');
    int i = nameFilter.indexOf(sep, 0);
    if (i == -1 && nameFilter.indexOf(' ', 0) != -1)
        sep = QChar(' ');

    QStringList ret = nameFilter.split(sep);
    for(QStringList::Iterator it = ret.begin(); it != ret.end(); ++it)
        (*it) = (*it).trimmed();
    return ret;
}

/*!
    \class QDir
    \reentrant
    \brief The QDir class provides access to directory structures and their contents in a platform-independent way.

    \ingroup io
    \mainclass

    A QDir is used to manipulate path names, access information
    regarding paths and files, and manipulate the underlying file
    system.

    A QDir can point to a file using either a relative or an absolute
    path. Absolute paths begin with the directory separator "/"
    (optionally preceded by a drive specification under Windows). If
    you always use "/" as a directory separator, Qt will translate
    your paths to conform to the underlying operating system. Relative
    file names begin with a directory name or a file name and specify
    a path relative to the current directory.

    The "current" path refers to the application's working directory.
    A QDir's own path is set and retrieved with setPath() and path().

    An example of an absolute path is the string "/tmp/quartz", a
    relative path might look like "src/fatlib". You can use the
    function isRelative() to check if a QDir is using a relative or an
    absolute file path. Call convertToAbs() to convert a relative QDir
    to an absolute one. For a simplified path use cleanDirPath(). To
    obtain a path which has no symbolic links or redundant ".."
    elements use canonicalPath(). The path can be set with setPath(),
    and changed with cd() and cdUp().

    QDir provides several static functions, for example, setCurrent()
    to set the application's working directory and currentDirPath() to
    retrieve the application's working directory. Access to some
    common paths is provided with the static functions, current(),
    home() and root() which return QDir objects or currentDirPath(),
    homeDirPath() and rootDirPath() which return the path as a string.
    If you want to know about your application's path use
    \l{QApplication::applicationDirPath()}.

    The number of entries in a directory is returned by count().
    Obtain a string list of the names of all the files and directories
    in a directory with entryList(). If you prefer a list of QFileInfo
    pointers use entryInfoList(). Both these functions can apply a
    name filter, an attributes filter (e.g. read-only, files not
    directories, etc.), and a sort order. The filters and sort may be
    set with calls to setNameFilters(), setFilter() and setSorting().
    They may also be specified in the entryList() and
    entryInfoList()'s arguments.

    Create a new directory with mkdir(), rename a directory with
    rename() and remove an existing directory with rmdir(). Remove a
    file with remove(). You can interrogate a directory with exists(),
    isReadable() and isRoot().

    To get a path with a filename use filePath(), and to get a
    directory name use dirName(); neither of these functions checks
    for the existence of the file or directory.

    The list of root directories is provided by drives(); on Unix
    systems this returns a list containing one root directory, "/"; on
    Windows the list will usually contain "C:/", and possibly "D:/",
    etc.

    It is easiest to work with "/" separators in Qt code. If you need
    to present a path to the user or need a path in a form suitable
    for a function in the underlying operating system use
    convertSeparators().

    Examples:

    See if a directory exists.
    \code
    QDir dir("example");                     // "./example"
    if (!dir.exists())
        qWarning("Cannot find the example directory");
    \endcode
    (Alternatively, we could use the static convenience function
     QFile::exists().)

    Traversing directories and reading a file.
    \code
    QDir dir = QDir::root();                 // "/"
    if (!dir.cd("tmp")) {                    // "/tmp"
        qWarning("Cannot find the \"/tmp\" directory");
    } else {
        QFile file(dir.filePath("ex1.txt")); // "/tmp/ex1.txt"
        if (!file.open(IO_ReadWrite))
            qWarning("Cannot create the file %s", file.name());
    }
    \endcode

    A program that lists all the files in the current directory
    (excluding symbolic links), sorted by size, smallest first:
    \code
    #include <stdio.h>
    #include <qdir.h>

    int main(int argc, char **argv)
    {
        QDir dir;
        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        dir.setSorting(QDir::Size | QDir::Reversed);

        QFileInfoList list = dir.entryInfoList();
        printf("     Bytes Filename\n");
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fi = list.at(i);
            printf("%10li %s\n", fi.size(), fi.fileName().latin1());
        }
        return 0;
    }
    \endcode

    \sa QApplication::applicationDirPath()
*/

/*!
    Constructs a QDir pointing to the current directory (".").

    \sa currentDirPath()
*/

QDir::QDir() : d_ptr(new QDirPrivate(this))
{
    d->setPath(QString::fromLatin1("."), true);
    d->data->nameFilters = QStringList(QString::fromLatin1("*"));
    d->data->filterSpec = All;
    d->data->sortSpec = SortSpec(Name | IgnoreCase);
}

/*!
    Constructs a QDir with path \a path, that filters its entries by
    name using \a nameFilter and by attributes using \a filterSpec. It
    also sorts the names using \a sortSpec.

    The default \a nameFilter is an empty string, which excludes
    nothing; the default \a filterSpec is \c All, which also means
    exclude nothing. The default \a sortSpec is \c Name|IgnoreCase,
    i.e. sort by name case-insensitively.

    Example that lists all the files in "/tmp":
    \code
    QDir d( "/tmp" );
    for ( int i = 0; i < d.count(); i++ )
	printf( "%s\n", d[i] );
    \endcode

    If \a path is "" or QString::null, QDir uses "." (the current
    directory). If \a nameFilter is "" or QString::null, QDir uses the
    name filter "*" (all files).

    Note that \a path need not exist.

    \sa exists(), setPath(), setNameFilter(), setFilter(), setSorting()
*/

QDir::QDir(const QString &path, const QString &nameFilter,
             int sortSpec, int filterSpec)  : d_ptr(new QDirPrivate(this))
{
    d->setPath(path.isEmpty() ? QString::fromLatin1(".") : path, true);
    d->data->nameFilters = qt_makeFilterStringList(nameFilter);
    if (d->data->nameFilters.isEmpty())
        d->data->nameFilters = QString::fromLatin1("*");
    d->data->sortSpec = sortSpec;
    d->data->filterSpec = filterSpec;
}

/*!
    Constructs a QDir with path \a path, that filters its entries by
    name using \a nameFilters and by attributes using \a filterSpec. It
    also sorts the names using \a sortSpec.

    The default \a nameFilters is an empty string, which excludes
    nothing; the default \a filterSpec is \c All, which also means
    exclude nothing. The default \a sortSpec is \c Name|IgnoreCase,
    i.e. sort by name case-insensitively.

    Example that lists all the files in "/tmp":
    \code
    QDir d("/tmp");
    for (int i = 0; i < d.count(); i++)
        printf("%s\n", d[i]);
    \endcode

    If \a path is "" or QString::null, QDir uses "." (the current
    directory). If \a nameFilters is "" or QString::null, QDir uses the
    name filter "*" (all files).

    Note that \a path need not exist.

    \sa exists(), setPath(), setNameFilters(), setFilter(), setSorting()
*/

QDir::QDir(const QString &path, const QStringList &nameFilters, 
             int sortSpec, int filterSpec) : d_ptr(new QDirPrivate(this))
{
    d->setPath(path.isEmpty() ? QString::fromLatin1(".") : path, true);
    d->data->nameFilters = nameFilters;
    if (d->data->nameFilters.isEmpty())
        d->data->nameFilters = QString::fromLatin1("*");
    d->data->sortSpec = sortSpec;
    d->data->filterSpec = filterSpec;
}

/*!
    Constructs a QDir that is a copy of the directory \a d.

    \sa operator=()
*/

QDir::QDir(const QDir &dir)  : d_ptr(new QDirPrivate(this, &dir))
{
}

/*!
    Destroys the QDir frees up its resources.
*/

QDir::~QDir()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
    Sets the path of the directory to \a path. The path is cleaned of
    redundant ".", ".." and of multiple separators. No check is made
    to ensure that a directory with this path exists.

    The path can be either absolute or relative. Absolute paths begin
    with the directory separator "/" (optionally preceded by a drive
    specification under Windows). Relative file names begin with a
    directory name or a file name and specify a path relative to the
    current directory. An example of an absolute path is the string
    "/tmp/quartz", a relative path might look like "src/fatlib".

    \sa path(), absPath(), exists(), cleanDirPath(), dirName(),
      absFilePath(), isRelative(), convertToAbs()
*/

void
QDir::setPath(const QString &path)
{
    d->setPath(path, true);
}

/*!
    Returns the path, this may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    The returned path can be either absolute or relative (see
    setPath()).

    \sa setPath(), absPath(), exists(), cleanDirPath(), dirName(),
    absFilePath(), convertSeparators()
*/

QString
QDir::path() const
{
    return d->data->path;
}

/*!
    Returns the absolute path (a path that starts with "/" or with a
    drive specification), which may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    \sa setPath(), canonicalPath(), exists(),  cleanDirPath(),
    dirName(), absFilePath()
*/

QString
QDir::absPath() const
{
    if (QDir::isRelativePath(d->data->path)) 
        return absFilePath("");
    return cleanDirPath(d->data->path);
}


/*!
    Returns the canonical path, i.e. a path without symbolic links or
    redundant "." or ".." elements.

    On systems that do not have symbolic links this function will
    always return the same string that absPath() returns. If the
    canonical path does not exist (normally due to dangling symbolic
    links) canonicalPath() returns QString::null.

    \sa path(), absPath(), exists(), cleanDirPath(), dirName(),
        absFilePath(), QString::isNull()
*/

QString
QDir::canonicalPath() const
{
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    return cleanDirPath(d->data->fi.canonicalPath());
}

/*!
    Returns the name of the directory; this is \e not the same as the
    path, e.g. a directory with the name "mail", might have the path
    "/var/spool/mail". If the directory has no name (e.g. it is the
    root directory) QString::null is returned.

    No check is made to ensure that a directory with this name
    actually exists.

    \sa path(), absPath(), absFilePath(), exists(), QString::isNull()
*/

QString
QDir::dirName() const
{
    int pos = d->data->path.lastIndexOf('/');
    if (pos == -1)
        return d->data->path;
    return d->data->path.mid(pos + 1);
}

/*!
    Returns the path name of a file in the directory. Does \e not
    check if the file actually exists in the directory. If the QDir is
    relative the returned path name will also be relative. Redundant
    multiple separators or "." and ".." directories in \a fileName
    will not be removed (see cleanDirPath()).

    If \a acceptAbsPath is true a \a fileName starting with a
    separator "/" will be returned without change. If \a acceptAbsPath
    is false an absolute path will be prepended to the fileName and
    the resultant string returned.

    \sa absFilePath(), isRelative(), canonicalPath()
*/

QString
QDir::filePath(const QString &fileName, bool acceptAbsPath) const
{
    if (acceptAbsPath && !isRelativePath(fileName))
        return QString(fileName);

    QString ret = d->data->path;
    if(!fileName.isEmpty()) {
        if (ret.isEmpty() || (ret[(int)ret.length()-1] != '/' && fileName[0] != '/'))
            ret += '/';
        ret += fileName;
    }
    return ret;
}

/*!
    Returns the absolute path name of a file in the directory. Does \e
    not check if the file actually exists in the directory. Redundant
    multiple separators or "." and ".." directories in \a fileName
    will not be removed (see cleanDirPath()).

    If \a acceptAbsPath is true a \a fileName starting with a
    separator "/" will be returned without change. If \a acceptAbsPath
    is false an absolute path will be prepended to the fileName and
    the resultant string returned.

    \sa filePath()
*/

QString
QDir::absFilePath(const QString &fileName, bool acceptAbsPath) const
{
    if (acceptAbsPath && !isRelativePath(fileName))
        return fileName;
    if(!d->data->dirEngine)
        return fileName;

    QString ret;
    if (isRelativePath(d->data->path)) { //get pwd
        ret = QFSDirEngine::currentDirPath(fileName);
    }
    if(!d->data->path.isEmpty() && d->data->path != ".") {
        if (!ret.isEmpty() && ret.right(1) != QString::fromLatin1("/"))
            ret += '/';
        ret += d->data->path;
    }
    if (!fileName.isEmpty()) {
        if (ret.right(1) != QString::fromLatin1("/"))
            ret += '/';
        ret += fileName;
    }
    return ret;
}

/*!
    Returns \a pathName with the '/' separators converted to
    separators that are appropriate for the underlying operating
    system.

    On Windows, convertSeparators("c:/winnt/system32") returns
    "c:\winnt\system32".

    The returned string may be the same as the argument on some
    operating systems, for example on Unix.
*/

QString
QDir::convertSeparators(const QString &pathName)
{
    QString n(pathName);
#if defined(Q_FS_FAT) || defined(Q_OS_OS2EMX)
    for (int i=0; i<(int)n.length(); i++) {
        if (n[i] == '/')
            n[i] = '\\';
    }
#endif
    return n;
}

/*!
    Changes the QDir's directory to \a dirName.

    If \a acceptAbsPath is true a path starting with separator "/"
    will cause the function to change to the absolute directory. If \a
    acceptAbsPath is false any number of separators at the beginning
    of \a dirName will be removed and the function will descend into
    \a dirName.

    Returns true if the new directory exists and is readable;
    otherwise returns false. Note that the logical cd() operation is
    not performed if the new directory does not exist.

    Calling cd("..") is equivalent to calling cdUp().

    \sa cdUp(), isReadable(), exists(), path()
*/

bool
QDir::cd(const QString &dirName, bool acceptAbsPath )
{
    if (dirName.isEmpty() || dirName == QString::fromLatin1(".")) 
        return true;
    QString newPath = d->data->path;
    if (acceptAbsPath && !isRelativePath(dirName)) {
        newPath = cleanDirPath(dirName);
    } else {
        if (isRoot()) {
            if (dirName == "..") 
                return false;
        } else {
            newPath += '/';
        }

        newPath += dirName;
        if (dirName.indexOf('/') >= 0
            || d->data->path == QString::fromLatin1(".")
            || dirName == QString::fromLatin1("..")) {
            newPath = cleanDirPath(newPath);
            /*
              If newPath starts with .., we convert it to absolute to
              avoid infinite looping on

                  QDir dir(".");
                  while (dir.cdUp())
                      ;
            */
            if (newPath[0] == QChar('.') && newPath[1] == QChar('.') &&
                 (newPath.length() == 2 || newPath[2] == QChar('/')))
                convertToAbs();
        }
    }

    if(!d->setPath(newPath, false))
        return false;
    refresh();
    return true;
}

/*!
    Changes directory by moving one directory up from the QDir's
    current directory.

    Returns true if the new directory exists and is readable;
    otherwise returns false. Note that the logical cdUp() operation is
    not performed if the new directory does not exist.

    \sa cd(), isReadable(), exists(), path()
*/

bool
QDir::cdUp()
{
    return cd(QString::fromLatin1(".."));
}

/*!
    Returns the stringlist set by setNameFilters()
*/

QStringList
QDir::nameFilters() const
{
    return d->data->nameFilters;
}

/*!
    Sets the name filter used by entryList() and entryInfoList() to \a
    nameFilters.

    The \a nameFilters is a wildcard (globbing) filter that understands
    "*" and "?" wildcards. (See \link qregexp.html#wildcard-matching
    QRegExp wildcard matching\endlink.) You may specify several filter
    entries all separated by a single space " " or by a semi-colon
    ";".

    For example, if you want entryList() and entryInfoList() to list
    all files ending with either ".cpp" or ".h", you would use either
    dir.setNameFilters("*.cpp *.h") or dir.setNameFilters("*.cpp;*.h").

    \sa nameFilters(), setFilter()
*/

void
QDir::setNameFilters(const QStringList &nameFilters)
{
    d->detach();
    d->data->nameFilters = nameFilters;
}

/*!
    Returns the value set by setFilter()
*/

QDir::FilterSpec
QDir::filter() const
{
    return (FilterSpec)d->data->filterSpec;
}

/*!
    \enum QDir::FilterSpec

    This enum describes the filtering options available to QDir, e.g.
    for entryList() and entryInfoList(). The filter value is specified
    by OR-ing together values from the following list:

    \value Dirs  List directories only.
    \value Files  List files only.
    \value  Drives  List disk drives (ignored under Unix).
    \value  NoSymLinks  Do not list symbolic links (ignored by operating
    systems that don't support symbolic links).
    \value All List directories, files, drives and symlinks (this does not list
    broken symlinks unless you specify System).
    \value TypeMask A mask for the the Dirs, Files, Drives and
    NoSymLinks flags.
    \value  Readable  List files for which the application has read access.
    \value  Writable  List files for which the application has write access.
    \value  Executable  List files for which the application has execute
    access. Executables needs to be combined with Dirs or Files.
    \value  RWEMask A mask for the Readable, Writable and Executable flags.
    \value  Modified  Only list files that have been modified (ignored
    under Unix).
    \value  Hidden  List hidden files (on Unix, files starting with a .).
    \value  System  List system files (on Unix, FIFOs, sockets and
    device files)
    \value AccessMask A mask for the Readable, Writable, Executable
    Modified, Hidden and System flags
    \value DefaultFilter Internal flag.

    If you do not set any of \c Readable, \c Writable or \c
    Executable, QDir will set all three of them. This makes the
    default easy to write and at the same time useful.

    Examples: \c Readable|Writable means list all files for which the
    application has read access, write access or both. \c Dirs|Drives
    means list drives, directories, all files that the application can
    read, write or execute, and also symlinks to such
    files/directories.
*/

/*!
    Sets the filter used by entryList() and entryInfoList() to \a
    filterSpec. The filter is used to specify the kind of files that
    should be returned by entryList() and entryInfoList(). See
    \l{QDir::FilterSpec}.

    \sa filter(), setNameFilters()
*/

void
QDir::setFilter(int filterSpec)
{
    d->detach();
    d->data->filterSpec = filterSpec;
}

/*!
    Returns the value set by setSorting()

    \sa setSorting() SortSpec
*/

QDir::SortSpec
QDir::sorting() const
{
    return (SortSpec)d->data->sortSpec;
}

/*!
    \enum QDir::SortSpec

    This enum describes the sort options available to QDir, e.g. for
    entryList() and entryInfoList(). The sort value is specified by
    OR-ing together values from the following list:

    \value Name  Sort by name.
    \value Time  Sort by time (modification time).
    \value Size  Sort by file size.
    \value Unsorted  Do not sort.
    \value SortByMask A mask for Name, Time and Size.

    \value DirsFirst  Put the directories first, then the files.
    \value Reversed  Reverse the sort order.
    \value IgnoreCase  Sort case-insensitively.
    \value DefaultSort Internal flag.

    You can only specify one of the first four.

    If you specify both \c DirsFirst and \c Reversed, directories are
    still put first, but in reverse order; the files will be listed
    after the directories, again in reverse order.
*/

/*!
    Sets the sort order used by entryList() and entryInfoList().

    The \a sortSpec is specified by OR-ing values from the enum
    \l{QDir::SortSpec}.

    \sa sorting() SortSpec
*/

void
QDir::setSorting(int sortSpec)
{
    d->detach();
    d->data->sortSpec = sortSpec;
}


/*!
    Returns the total number of directories and files that were found.

    Equivalent to entryList().count().

    \sa operator[](), entryList()
*/

uint
QDir::count() const
{
    d->updateFileLists();
    return d->data->files.count();
}

/*!
    Returns the file name at position \a index in the list of file
    names. Equivalent to entryList().at(index).

    Returns a QString::null if the \a index is out of range or if the
    entryList() function failed.

    \sa count(), entryList()
*/

QString 
QDir::operator[](int pos) const
{
    d->updateFileLists();
    return d->data->files[pos];
}

/*!
    \overload

    Returns a list of the names of all the files and directories in
    the directory, ordered in accordance with setSorting() and
    filtered in accordance with setFilter() and setNameFilters().

    The filter and sorting specifications can be overridden using the
    \a filterSpec and \a sortSpec arguments.

    Returns an empty list if the directory is unreadable or does not
    exist.

    \sa entryInfoList(), setNameFilters(), setSorting(), setFilter()
*/

QStringList
QDir::entryList(int filterSpec, int sortSpec) const
{
    return entryList(d->data->nameFilters, filterSpec, sortSpec);
}


/*!
    \overload

    Returns a list of QFileInfo objects for all the files and
    directories in the directory, ordered in accordance with
    setSorting() and filtered in accordance with setFilter() and
    setNameFilters().

    The filter and sorting specifications can be overridden using the
    \a filterSpec and \a sortSpec arguments.

    Returns an empty list if the directory is unreadable or does not exist.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/

QFileInfoList 
QDir::entryInfoList(int filterSpec, int sortSpec) const
{
    return entryInfoList(d->data->nameFilters, filterSpec, sortSpec);
}

/*!
    Returns a list of the names of all the files and directories in
    the directory, ordered in accordance with setSorting() and
    filtered in accordance with setFilter() and setNameFilters().

    The filter and sorting specifications can be overridden using the
    \a nameFilters, \a filterSpec and \a sortSpec arguments.

    Returns an empty list if the directory is unreadable or does not
    exist.

    \sa entryInfoList(), setNameFilters(), setSorting(), setFilter()
*/

QStringList
QDir::entryList(const QStringList &nameFilters, int filterSpec, int sortSpec) const
{
    if (filterSpec == (int)DefaultFilter)
        filterSpec = d->data->filterSpec;
    if (sortSpec == (int)DefaultSort)
        sortSpec = d->data->sortSpec;
    if (filterSpec == (int)DefaultFilter && sortSpec == (int)DefaultSort && nameFilters == d->data->nameFilters) {
        d->updateFileLists();
        return d->data->files;
    }
    QStringList l = d->data->dirEngine->entryList(d->data->filterSpec, nameFilters), ret;
    d->sortFileList(sortSpec, l, &ret, 0); 
    return ret;
}

/*!
    \overload

    Returns a list of QFileInfo objects for all the files and
    directories in the directory, ordered in accordance with
    setSorting() and filtered in accordance with setFilter() and
    setNameFilters().

    The filter and sorting specifications can be overridden using the
    \a filterSpec and \a sortSpec arguments.

    Returns an empty list if the directory is unreadable or does not exist.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/

QFileInfoList
QDir::entryInfoList(const QStringList &nameFilters, int filterSpec, int sortSpec) const
{
    if (filterSpec == (int)DefaultFilter)
        filterSpec = d->data->filterSpec;
    if (sortSpec == (int)DefaultSort)
        sortSpec = d->data->sortSpec;
    if (filterSpec == (int)DefaultFilter && sortSpec == (int)DefaultSort && nameFilters == d->data->nameFilters) {
        d->updateFileLists();
        return d->data->fileInfos;
    }
    QFileInfoList ret;
    QStringList l = d->data->dirEngine->entryList(d->data->filterSpec, nameFilters);
    d->sortFileList(sortSpec, l, 0, &ret); 
    return ret;
}

/*!
    Creates a directory.

    If \a acceptAbsPath is true a path starting with a separator ('/')
    will create the absolute directory; if \a acceptAbsPath is false
    any number of separators at the beginning of \a dirName will be
    removed.

    Returns true if successful; otherwise returns false.

    \sa rmdir()
*/

bool
QDir::mkdir(const QString &dirName, bool acceptAbsPath) const
{
    if (dirName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name(s)");
        return false;
    }
    if(!d->data->dirEngine)
        return false;

    QString fn = filePath(dirName, acceptAbsPath);
    return d->data->dirEngine->mkdir(fn);
}

/*!
    Removes a directory.

    If \a acceptAbsPath is true a path starting with a separator ('/')
    will remove the absolute directory; if \a acceptAbsPath is false
    any number of separators at the beginning of \a dirName will be
    removed.

    The directory must be empty for rmdir() to succeed.

    Returns true if successful; otherwise returns false.

    \sa mkdir()
*/

bool
QDir::rmdir(const QString &dirName, bool acceptAbsPath ) const
{
    if (dirName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name(s)");
        return false;
    }
    if(!d->data->dirEngine)
        return false;

    QString tmp = filePath(dirName, acceptAbsPath);
    return d->data->dirEngine->rmdir(tmp);
}

/*!
    Returns true if the directory is readable \e and we can open files
    by name; otherwise returns false.

    \warning A false value from this function is not a guarantee that
    files in the directory are not accessible.

    \sa QFileInfo::isReadable()
*/


bool
QDir::isReadable() const
{
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    return d->data->fi.isReadable();
}

/*!
    \overload

    Returns true if the \e directory exists; otherwise returns false.
    (If a file with the same name is found this function will return
    false).

    \sa QFileInfo::exists(), QFile::exists()
*/

bool
QDir::exists() const
{
    //we cannot use the data->fi because the old implementation
    //didn't cache this (but in other places it does!!) [SOURCE COMPAT]
    QFileInfo fi(d->data->path);
    return fi.exists() && fi.isDir();
}

/*!
    Returns true if the directory is the root directory; otherwise
    returns false.

    Note: If the directory is a symbolic link to the root directory
    this function returns false. If you want to test for this use
    canonicalPath(), e.g.
    \code
    QDir d("/tmp/root_link");
    d = d.canonicalPath();
    if (d.isRoot())
        qWarning("It is a root link");
    \endcode

    \sa root(), rootDirPath()
*/

bool
QDir::isRoot() const
{
    if(!d->data->dirEngine)
        return true;    
    return d->data->dirEngine->isRoot();
}

/*!
    Returns true if the directory path is relative to the current
    directory and returns false if the path is absolute (e.g. under
    UNIX a path is relative if it does not start with a "/").

    \sa convertToAbs()
*/

bool
QDir::isRelative() const
{
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    return d->data->fi.isRelative();
}


/*!
    Converts the directory path to an absolute path. If it is already
    absolute nothing is done.

    \sa isRelative()
*/

void
QDir::convertToAbs()
{
    d->detach();
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    d->data->fi.convertToAbs();
    d->data->path = d->data->fi.filePath();
    if(!d->data->dirEngine->setPath(d->data->path, false))
        qWarning("Failure to conver to absolute!");
}

/*!
    Returns true if directory \a d and this directory have the same
    path and their sort and filter settings are the same; otherwise
    returns false.

    Example:
    \code
    // The current directory is "/usr/local"
    QDir d1("/usr/local/bin");
    QDir d2("bin");
    d2.convertToAbs();
    if (d1 == d2)
        qDebug("They're the same");
    \endcode
*/

bool QDir::operator==(const QDir &dir) const
{
    return dir.d->data == d->data 
           || (d->data->path == dir.d->data->path 
               && d->data->filterSpec == dir.d->data->filterSpec 
               && d->data->sortSpec == dir.d->data->sortSpec 
               && d->data->nameFilters == dir.d->data->nameFilters);
}

/*!
    Makes a copy of QDir \a d and assigns it to this QDir.
*/

QDir &QDir::operator=(const QDir &dir)
{
    if (!--d->data->ref)
        delete d->data;
    ++dir.d->data->ref;
    d->data = dir.d->data;
    return *this;
}

/*!
    \overload

    Sets the directory path to be the given \a path.
*/

QDir &QDir::operator=(const QString &path)
{
    d->setPath(path, true);
    return *this;
}

/*!
    \fn bool QDir::operator!=(const QDir &d) const

    Returns true if directory \a d and this directory have different
    paths or different sort or filter settings; otherwise returns
    false.

    Example:
    \code
    // The current directory is "/usr/local"
    QDir d1("/usr/local/bin");
    QDir d2("bin");
    if (d1 != d2)
        qDebug("They differ");
    \endcode
*/


/*!
    Removes the file, \a fileName.

    If \a acceptAbsPath is true a path starting with separator "/"
    will remove the file with the absolute path. If \a acceptAbsPath
    is false any number of separators at the beginning of \a fileName
    will be removed and the resultant file name will be removed.

    Returns true if the file is removed successfully; otherwise
    returns false.
*/

bool
QDir::remove(const QString &fileName, bool acceptAbsPath)
{
    if (fileName.isEmpty()) {
        qWarning("QDir::remove: Empty or null file name");
        return false;
    }
    QString p = filePath(fileName, acceptAbsPath);
    return QFile::remove(p);
}

/*!
    Renames a file or directory.

    If \a acceptAbsPaths is true a path starting with a separator
    ('/') will rename the file with the absolute path; if \a
    acceptAbsPaths is false any number of separators at the beginning
    of the names will be removed.

    Returns true if successful; otherwise returns false.

    On most file systems, rename() fails only if \a oldName does not
    exist or if \a newName and \a oldName are not on the same
    partition. On Windows, rename() will fail if \a newName already
    exists. However, there are also other reasons why rename() can
    fail. For example, on at least one file system rename() fails if
    \a newName points to an open file.
*/

bool
QDir::rename(const QString &name, const QString &newName, bool acceptAbsPaths)
{
    if (name.isEmpty() || newName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name(s)");
        return false;
    }
    if(!d->data->dirEngine)
        return false;

    QString fn1 = filePath(name, acceptAbsPaths);
    QString fn2 = filePath(newName, acceptAbsPaths);
    return d->data->dirEngine->rename(fn1, fn2);
}

/*!
    Checks for the existence of the file \a name.

    If \a acceptAbsPath is true a path starting with separator "/"
    will check the file with the absolute path. If \a acceptAbsPath is
    false any number of separators at the beginning of \a name will be
    removed and the resultant file name will be checked.

    Returns true if the file exists; otherwise returns false.

    \sa QFileInfo::exists(), QFile::exists()
*/

bool
QDir::exists(const QString &name, bool acceptAbsPath) const
{
    if (name.isEmpty()) {
        qWarning("QDir::exists: Empty or null file name");
        return false;
    }
    QString tmp = filePath(name, acceptAbsPath);
    return QFile::exists(tmp);
}

/*!
    Returns a list of the root directories on this system. On Windows
    this returns a number of QFileInfo objects containing "C:/", "D:/"
    etc. On other operating systems, it returns a list containing just
    one root directory (e.g. "/").
*/

QFileInfoList
QDir::drives()
{
    return QFSDirEngine::drives();
}

/*!
    Returns the native directory separator; "/" under UNIX (including
    Mac OS X) and "\" under Windows.

    You do not need to use this function to build file paths. If you
    always use "/", Qt will translate your paths to conform to the
    underlying operating system.
*/

QChar
QDir::separator()
{
#if defined(Q_OS_UNIX)
    return '/';
#elif defined (Q_FS_FAT) || defined(Q_WS_WIN)
    return '\\';
#elif defined (Q_OS_MAC)
    return ':';
#else
    return '/';
#endif
}

/*!
    Sets the application's current working directory to \a path.
    Returns true if the directory was successfully changed; otherwise
    returns false.
*/

bool
QDir::setCurrent(const QString &path)
{
    return QFSDirEngine::setCurrentDirPath(path);
}

/*!
    \fn QDir QDir::currentDir() 
    Returns the application's current directory.

    Use path() to access a QDir object's path.

    \sa currentDirPath(), QDir::QDir()
*/

/*!
    Returns the absolute path of the application's current directory.

    \sa current()
*/

QString
QDir::currentDirPath()
{
    return QFSDirEngine::currentDirPath();    
}

/*!
    \fn QDir QDir::currentDir() 

    Returns the home directory.

    Under Windows the \c HOME environment variable is used. If this
    does not exist the \c USERPROFILE environment variable is used. If
    that does not exist the path is formed by concatenating the \c
    HOMEDRIVE and \c HOMEPATH environment variables. If they don't
    exist the rootDirPath() is used (this uses the \c SystemDrive
    environment variable). If none of these exist "C:\" is used.

    Under non-Windows operating systems the \c HOME environment
    variable is used if it exists, otherwise rootDirPath() is used.

    \sa homeDir()
*/

QString
QDir::homeDirPath()
{
    return QFSDirEngine::homeDirPath();
}

/*!
    \fn QDir QDir::root() 

    Returns the root directory.

    \sa rootDirPath() drives()
*/

/*!
    Returns the absolute path for the root directory.

    For UNIX operating systems this returns "/". For Windows file
    systems this normally returns "c:/".

    \sa root() drives()
*/

QString
QDir::rootDirPath()
{
    return QFSDirEngine::rootDirPath();
}

#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns true if the \a fileName matches any of the wildcard (glob)
    patterns in the list of \a filters; otherwise returns false.

    (See \link qregexp.html#wildcard-matching QRegExp wildcard
    matching.\endlink)

    \sa QRegExp::exactMatch()
*/


bool
QDir::match(const QStringList &filters, const QString &fileName)
{
    for(QStringList::ConstIterator sit = filters.begin(); sit != filters.end(); ++sit) {
        QRegExp rx(*sit, d->data->dirEngine->caseSensitive() ? 
                   QString::CaseSensitive : QString::CaseInsensitive, QRegExp::Wildcard);
        if (rx.exactMatch(fileName))
            return true;
    }
    return false;
}

/*!
    Returns true if the \a fileName matches the wildcard (glob)
    pattern \a filter; otherwise returns false. The \a filter may
    contain multiple patterns separated by spaces or semicolons.

    (See \link qregexp.html#wildcard-matching QRegExp wildcard
    matching.\endlink)

    \sa QRegExp::exactMatch()
*/

bool
QDir::match(const QString &filter, const QString &fileName)
{
    return match(QStringList(filter), fileName);
}
#endif

/*!
    Removes all multiple directory separators "/" and resolves any
    "."s or ".."s found in the path, \a filePath.

    Symbolic links are kept. This function does not return the
    canonical path, but rather the simplest version of the input.
    For example, "./local" becomes "local", "local/../bin" becomes
    "bin" and "/local/usr/../bin" becomes "/local/bin".

    \sa absPath() canonicalPath()
*/


QString
QDir::cleanDirPath(const QString &in)
{
    if (in.isEmpty())
        return in;
    QString name = convertSeparators(in);

    int used = 0, levels = 0;
    const int len = name.length();
    QChar *out = new QChar[len];
    const QChar *p = name.unicode();
    for(int i = 0, last = -1, iwrite = 0; i < len; i++) {
        if(p[i] == '/') {
            bool eaten = false;
            if(i < len - 1 && p[i+1] == '.') {
                int dotcount = 1;
                if(i < len - 2 && p[i+2] == '.') 
                    dotcount++;
                if(i == len - dotcount - 1) {
                    if(last == -1) {
                        for(int i2 = iwrite-1; i2 >= 0; i2--) {
                            if(out[i2] == '/') 
                                last = i2;
                        }
                    }
                    used -= iwrite - last - 1;
                    break; 
                }
                if(p[i+dotcount+1] == '/') {
                    if(dotcount == 2 && levels) {
                        if(last == -1 || iwrite - last == 1) {
                            for(int i2 = (last == -1) ? (iwrite-1) : (last-1); i2 >= 0; i2--) {
                                if(out[i2] == '/') {
                                    eaten = true;
                                    last = i2;
                                    break;
                                }
                            }
                        } else {
                            eaten = true;
                        }
                        if(eaten) {
                            levels--;
                            used -= iwrite - last;
                            iwrite = last;
                            last = -1;
                        }
                    } else if(dotcount == 1) {
                        eaten = true;
                    }
                    if(eaten)
                        i += dotcount;
                }
            } else if(last != -1 && iwrite - last == 1) {
#ifdef Q_OS_WIN
                eaten = (iwrite > 2);
#else
                eaten = true;
#endif
            } else if(i == len-1) {
                eaten = true;
            } else {
                levels++;
            }
            if(!eaten)
                last = i - (i - iwrite);
            else
                continue;
        } else if(!i && p[i] == '.') {
            int dotcount = 1;
            if(len >= 1 && p[1] == '.') 
                dotcount++;
            if(len >= dotcount && p[dotcount] == '/') {
                if(dotcount == 1) {
                    i++;
                    continue;
                }
            }
        }
        out[iwrite++] = p[i];
        used++;
    }
    QString ret;
    if(used == len)
        ret = name;
    else
	ret = QString(out, used);
    delete [] out;
    return ret;
}

/*!
    Returns true if \a path is relative; returns false if it is
    absolute.

    \sa isRelative()
*/

bool
QDir::isRelativePath(const QString &path)
{
    QFileInfo fi(path);
    return fi.isRelative();
}

/*!
    Refreshes the directory information.
*/

void
QDir::refresh() const
{
    d->data->clear();
}

#ifdef QT_COMPAT
static inline QChar getFilterSepChar(const QString &nameFilter)
{
    QChar sep(';');
    int i = nameFilter.indexOf(sep, 0);
    if (i == -1 && nameFilter.indexOf(' ', 0) != -1)
        sep = QChar(' ');
    return sep;
}

QStringList QDir::entryList(const QString &nameFilter, int filterSpec, int sortSpec) const
{ 
    return entryList(nameFilter.split(getFilterSepChar(nameFilter)), filterSpec, sortSpec); 
}

QFileInfoList QDir::entryInfoList(const QString &nameFilter, int filterSpec, int sortSpec) const
{ 
    return entryInfoList(nameFilter.split(getFilterSepChar(nameFilter)), filterSpec, sortSpec); 
}

QString QDir::nameFilter() const
{ 
    return nameFilters().join(QString(d->filterSepChar)); 
}

void QDir::setNameFilter(const QString &nameFilter)
{ 
    d->filterSepChar = getFilterSepChar(nameFilter); 
    setNameFilters(nameFilter.split(d->filterSepChar)); 
}
#endif
