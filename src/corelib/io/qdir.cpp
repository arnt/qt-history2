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

#include "qdir.h"
#include <qfileengine.h>
#include <private/qfsfileengine_p.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qregexp.h>
#include <qvector.h>
#include <qdebug.h>
#include <stdlib.h>

static QString driveSpec(const QString &path)
{
#ifdef Q_OS_WIN
    if (path.size() < 2)
        return QString();
    char c = path.at(0).toAscii();
    if (c < 'a' && c > 'z' && c < 'A' && c > 'Z')
        return QString();
    if (path.at(1).toAscii() != ':')
        return QString();
    return path.mid(0, 2);
#else
    Q_UNUSED(path);
    return QString();
#endif
}

//************* QDirPrivate
class QDirPrivate
{
    QDir *q_ptr;
    Q_DECLARE_PUBLIC(QDir)

protected:
    QDirPrivate(QDir*, const QDir *copy=0);
    ~QDirPrivate();

    void initFileEngine(const QString &file);

    void updateFileLists() const;
    void sortFileList(QDir::SortFlags, QStringList &, QStringList *, QFileInfoList *) const;

private:
#ifdef QT3_SUPPORT
    QChar filterSepChar;
#endif
    static inline QChar getFilterSepChar(const QString &nameFilter)
    {
        QChar sep(QLatin1Char(';'));
        int i = nameFilter.indexOf(sep, 0);
        if (i == -1 && nameFilter.indexOf(QLatin1Char(' '), 0) != -1)
            sep = QChar(QLatin1Char(' '));
        return sep;
    }
    static inline QStringList splitFilters(const QString &nameFilter, QChar sep=0) {
        if(sep == 0)
            sep = getFilterSepChar(nameFilter);
        QStringList ret = nameFilter.split(sep);
        for(int i = 0; i < ret.count(); i++)
            ret[i] = ret[i].trimmed();
        return ret;
    }

    struct Data {
        inline Data()
            : ref(1), fileEngine(0)
        { clear(); }
        inline Data(const Data &copy)
            : ref(1), path(copy.path), nameFilters(copy.nameFilters), sort(copy.sort),
              filters(copy.filters), fileEngine(0)
        { clear(); }
        inline ~Data()
        { delete fileEngine; }

        inline void clear() {
            listsDirty = 1;
        }
        mutable QAtomic ref;

        QString path;
        QStringList nameFilters;
        QDir::SortFlags sort;
        QDir::Filters filters;

        mutable QFileEngine *fileEngine;

        mutable uint listsDirty : 1;
        mutable QStringList files;
        mutable QFileInfoList fileInfos;
    } *data;
    inline void setPath(const QString &p)
    {
        detach(false);
        QString path = p;
        if ((path.endsWith(QLatin1Char('/')) || path.endsWith(QLatin1Char('\\')))
                && path.length() > 1) {
#ifdef Q_OS_WIN
            if (!(path.length() == 3 && path.at(1) == ':'))
#endif
                path.truncate(path.length() - 1);
        }
        if(!data->fileEngine || !QDir::isRelativePath(path))
            initFileEngine(path);
        data->fileEngine->setFileName(path);
        data->path = p;
        data->clear();
    }
    inline void reset() {
        detach();
        data->clear();
    }
    void detach(bool createFileEngine = true);
};

QDirPrivate::QDirPrivate(QDir *qq, const QDir *copy) : q_ptr(qq)
#ifdef QT3_SUPPORT
                                                     , filterSepChar(0)
#endif
{
    if(copy) {
        copy->d_func()->data->ref.ref();
        data = copy->d_func()->data;
    } else {
        data = new QDirPrivate::Data;
        data->clear();
    }
}

QDirPrivate::~QDirPrivate()
{
    if (!data->ref.deref())
        delete data;
    data = 0;
    q_ptr = 0;
}

/* For sorting */
struct QDirSortItem {
    QString filename_cache;
    QFileInfo item;
};
static int qt_cmp_si_sort_flags;

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
        if (qt_cmp_si_sort_flags & QDir::DirsFirst)
            return f1->item.isDir() ? -1 : 1;
        if (qt_cmp_si_sort_flags & QDir::DirsLast)
            return f1->item.isDir() ? 1 : -1;
    }

    int r = 0;
    int sortBy = qt_cmp_si_sort_flags & QDir::SortByMask;

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
        bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;

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

    if (qt_cmp_si_sort_flags & QDir::Reversed)
        return -r;
    return r;
}

#if defined(Q_C_CALLBACKS)
}
#endif

inline void QDirPrivate::sortFileList(QDir::SortFlags sort, QStringList &l,
                                      QStringList *names, QFileInfoList *infos) const
{
    if(names)
        names->clear();
    if(infos)
        infos->clear();
    if(!l.isEmpty()) {
        QDirSortItem *si= new QDirSortItem[l.count()];
        int i;
        for (i = 0; i < l.size(); ++i) {
            QString path = data->path;
            if (!path.isEmpty() && !path.endsWith(QLatin1Char('/')))
                path += QLatin1Char('/');
            si[i].item = QFileInfo(path + l.at(i));
        }
        qt_cmp_si_sort_flags = sort;
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
        QStringList l = data->fileEngine->entryList(data->filters, data->nameFilters);
        sortFileList(data->sort, l, &data->files, &data->fileInfos);
        data->listsDirty = 0;
    }
}

void QDirPrivate::initFileEngine(const QString &path)
{
    detach(false);
    delete data->fileEngine;
    data->fileEngine = 0;
    data->clear();
    data->fileEngine = QFileEngine::createFileEngine(path);
}

void QDirPrivate::detach(bool createFileEngine)
{
    qAtomicDetach(data);
    if (createFileEngine) {
        delete data->fileEngine;
        data->fileEngine = QFileEngine::createFileEngine(data->path);
    }
}

/*!
    \class QDir
    \brief The QDir class provides access to directory structures and their contents.

    \ingroup io
    \reentrant
    \mainclass

    A QDir is used to manipulate path names, access information
    regarding paths and files, and manipulate the underlying file
    system. It can also be used to access Qt's \l{resource system}.

    A QDir can point to a file using either a relative or an absolute
    path. Absolute paths begin with the directory separator "/"
    (optionally preceded by a drive specification under Windows). If
    you always use "/" as a directory separator, Qt will translate
    your paths to conform to the underlying operating system. Relative
    file names begin with a directory name or a file name and specify
    a path relative to the current directory.

    An example of an absolute path is the string "/tmp/quartz", a
    relative path might look like "src/fatlib". You can use the
    isRelative() or isAbsolute() functions to check if a QDir is using
    a relative or an absolute file path. Call makeAbsolute() to
    convert a relative QDir to an absolute one. For a simplified path
    use cleanPath(). To obtain a path which has no symbolic links or
    redundant ".." elements use canonicalPath(). The path can be set
    with setPath(), and changed with cd() and cdUp().

    The current() path (and currentPath()), refers to the application's
    working directory. A QDir's own path is set and retrieved with
    setPath() and path().

    QDir provides several static convenience functions, for example,
    setCurrent() to set the application's working directory and
    current() and currentPath() to retrieve the application's working
    directory. Access to some common paths is provided with the static
    functions, home(), root(), and temp() which return QDir objects or
    homePath(), rootPath(), and tempPath() which return the path as a
    string. For the application's directory, see
    \l{QApplication::applicationDirPath()}.

    The number of entries in a directory is returned by count(). You
    can obtain a string list of the names of all the files and
    directories in a directory with entryList(). If you prefer a list
    of QFileInfo pointers use entryInfoList(). Both these functions
    can apply a name filter, an attributes filter (e.g. read-only,
    files not directories, etc.), and a sort order. The filters and
    sort may be set with calls to setNameFilters(), setFilter() and
    setSorting(). They may also be specified in the entryList() and
    entryInfoList()'s arguments. You can test to see if a filename
    matches a filter using match().

    Create a new directory with mkdir(), rename a directory with
    rename() and remove an existing directory with rmdir(). Remove a
    file with remove(). You can query a directory with exists(),
    isReadable(), isAbsolute(), isRelative(), and isRoot(). You can
    use refresh() to re-read the directory's data from disk.

    To get a path with a filename use filePath(), and to get a
    directory name use dirName(); neither of these functions checks
    for the existence of the file or directory. The path() (changeable
    with setPath()), absolutePath(), absoluteFilePath(), and
    canonicalPath() are also available.

    The list of root directories is provided by drives(); on Unix
    systems this returns a list containing a single root directory,
    "/"; on Windows the list will usually contain "C:/", and possibly
    "D:/", etc.

    It is easiest to work with "/" separators in Qt code. If you need
    to present a path to the user or need a path in a form suitable
    for a function in the underlying operating system use
    convertSeparators().

    Example (check if a directory exists):

    \code
        QDir dir("example");
        if (!dir.exists())
            qWarning("Cannot find the example directory");
    \endcode

    (We could also use the static convenience function
    QFile::exists().)

    Example (traversing directories and reading a file):

    \code
        QDir dir = QDir::root();                 // "/"
        if (!dir.cd("tmp")) {                    // "/tmp"
            qWarning("Cannot find the \"/tmp\" directory");
        } else {
            QFile file(dir.filePath("ex1.txt")); // "/tmp/ex1.txt"
            if (!file.open(QIODevice::ReadWrite))
                qWarning("Cannot create the file %s", file.name());
        }
    \endcode

    A program that lists all the files in the current directory
    (excluding symbolic links), sorted by size, smallest first:

    \code
        #include <QDir>

        #include <stdio.h>

        int main(int argc, char *argv[])
        {
            QDir dir;
            dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
            dir.setSorting(QDir::Size | QDir::Reversed);

            QFileInfoList list = dir.entryInfoList();
            printf("     Bytes Filename\n");
            for (int i = 0; i < list.size(); ++i) {
                QFileInfo fileInfo = list.at(i);
                printf("%10li %s\n", fileInfo.size(), qPrintable(fileInfo.fileName()));
            }
            return 0;
        }
    \endcode

    \sa QFileInfo, QFile, QApplication::applicationDirPath()
*/

/*!
    Constructs a QDir pointing to the given directory \a path. If path
    is empty the program's working directory, ("."), is used.

    \sa currentPath()
*/

QDir::QDir(const QString &path) : d_ptr(new QDirPrivate(this))
{
    Q_D(QDir);
    d->setPath(path.isEmpty() ? QString::fromLatin1(".") : path);
    d->data->nameFilters = QStringList(QString::fromLatin1("*"));
    d->data->filters = TypeMask;
    d->data->sort = SortFlags(Name | IgnoreCase);
}

/*!
    Constructs a QDir with path \a path, that filters its entries by
    name using \a nameFilter and by attributes using \a filters. It
    also sorts the names using \a sort.

    The default \a nameFilter is an empty string, which excludes
    nothing; the default \a filters is \c TypeMask, which also means
    exclude nothing. The default \a sort is \c Name|IgnoreCase,
    i.e. sort by name case-insensitively.

    If \a path is an empty string, QDir uses "." (the current
    directory). If \a nameFilter is an empty string, QDir uses the
    name filter "*" (all files).

    Note that \a path need not exist.

    \sa exists(), setPath(), setNameFilter(), setFilter(), setSorting()
*/

QDir::QDir(const QString &path, const QString &nameFilter,
           SortFlags sort, Filters filters)  : d_ptr(new QDirPrivate(this))
{
    Q_D(QDir);
    d->setPath(path.isEmpty() ? QString::fromLatin1(".") : path);
    d->data->nameFilters = QDir::nameFiltersFromString(nameFilter);
    if (d->data->nameFilters.isEmpty())
        d->data->nameFilters = QStringList(QString::fromLatin1("*"));
    d->data->sort = sort;
    d->data->filters = filters;
}

/*!
    Constructs a QDir object that is a copy of the QDir object for
    directory \a dir.

    \sa operator=()
*/

QDir::QDir(const QDir &dir)  : d_ptr(new QDirPrivate(this, &dir))
{
}

/*!
    Destroys the QDir object frees up its resources. This has no
    effect on the underlying directory in the file system.
*/

QDir::~QDir()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
    Sets the path of the directory to \a path. The path is cleaned of
    redundant ".", ".." and of multiple separators. No check is made
    to see whether a directory with this path actually exists; but you
    can check for yourself using exists().

    The path can be either absolute or relative. Absolute paths begin
    with the directory separator "/" (optionally preceded by a drive
    specification under Windows). Relative file names begin with a
    directory name or a file name and specify a path relative to the
    current directory. An example of an absolute path is the string
    "/tmp/quartz", a relative path might look like "src/fatlib".

    \sa path(), absolutePath(), exists(), cleanPath(), dirName(),
      absoluteFilePath(), isRelative(), makeAbsolute()
*/

void QDir::setPath(const QString &path)
{
    Q_D(QDir);
    d->setPath(path);
}

/*!
    Returns the path. This may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    The returned path can be either absolute or relative (see
    setPath()).

    \sa setPath(), absolutePath(), exists(), cleanPath(), dirName(),
    absoluteFilePath(), convertSeparators(), makeAbsolute()
*/

QString QDir::path() const
{
    Q_D(const QDir);
    return d->data->path;
}

/*!
    Returns the absolute path (a path that starts with "/" or with a
    drive specification), which may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    \sa setPath(), canonicalPath(), exists(), cleanPath(),
    dirName(), absoluteFilePath()
*/

QString QDir::absolutePath() const
{
    Q_D(const QDir);
    if (QDir::isRelativePath(d->data->path))
        return absoluteFilePath(QString::fromLatin1(""));
    return cleanPath(d->data->path);
}


/*!
    Returns the canonical path, i.e. a path without symbolic links or
    redundant "." or ".." elements.

    On systems that do not have symbolic links this function will
    always return the same string that absolutePath() returns. If the
    canonical path does not exist (normally due to dangling symbolic
    links) canonicalPath() returns an empty string.

    Example:

    \code
        QString bin = "/local/bin";         // where /local/bin is a symlink to /usr/bin
        QDir binDir(bin);
        QString canonicalBin = binDir.canonicalPath();
        // canonicalBin now equals "/usr/bin"

        QString ls = "/local/bin/ls";       // where ls is the executable "ls"
        QDir lsDir(ls);
        QString canonicalLs = lsDir.canonicalPath();
        // canonicalLS now equals "/usr/bin/ls".
    \endcode

    \sa path(), absolutePath(), exists(), cleanPath(), dirName(),
        absoluteFilePath()
*/

QString QDir::canonicalPath() const
{
    Q_D(const QDir);

    if(!d->data->fileEngine)
        return QLatin1String("");
    return cleanPath(d->data->fileEngine->fileName(QFileEngine::CanonicalName));
}

/*!
    Returns the name of the directory; this is \e not the same as the
    path, e.g. a directory with the name "mail", might have the path
    "/var/spool/mail". If the directory has no name (e.g. it is the
    root directory) an empty string is returned.

    No check is made to ensure that a directory with this name
    actually exists; but see exists().

    \sa path(), filePath(), absolutePath(), absoluteFilePath()
*/

QString QDir::dirName() const
{
    Q_D(const QDir);
    int pos = d->data->path.lastIndexOf(QLatin1Char('/'));
    if (pos == -1)
        return d->data->path;
    return d->data->path.mid(pos + 1);
}

/*!
    Returns the path name of a file in the directory. Does \e not
    check if the file actually exists in the directory; but see
    exists(). If the QDir is relative the returned path name will also
    be relative. Redundant multiple separators or "." and ".."
    directories in \a fileName are not removed (see cleanPath()).

    \sa dirName() absoluteFilePath(), isRelative(), canonicalPath()
*/

QString QDir::filePath(const QString &fileName) const
{
    Q_D(const QDir);
    if (isAbsolutePath(fileName))
        return QString(fileName);

    QString ret = d->data->path;
    if(!fileName.isEmpty()) {
        if (!ret.isEmpty() && ret[(int)ret.length()-1] != QLatin1Char('/') && fileName[0] != QLatin1Char('/'))
            ret += QLatin1Char('/');
        ret += fileName;
    }
    return ret;
}

/*!
    Returns the absolute path name of a file in the directory. Does \e
    not check if the file actually exists in the directory; but see
    exists(). Redundant multiple separators or "." and ".."
    directories in \a fileName are not removed (see cleanPath()).

    \sa relativeFilePath() filePath() canonicalPath()
*/

QString QDir::absoluteFilePath(const QString &fileName) const
{
    Q_D(const QDir);
    if (isAbsolutePath(fileName))
        return fileName;
    if(!d->data->fileEngine)
        return fileName;

    QString ret;
    if (isRelativePath(d->data->path)) //get pwd
        ret = QFSFileEngine::currentPath(fileName);
    if(!d->data->path.isEmpty() && d->data->path != QLatin1String(".")) {
        if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
            ret += QLatin1Char('/');
        ret += d->data->path;
    }
    if (!fileName.isEmpty()) {
        if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
            ret += QLatin1Char('/');
        ret += fileName;
    }
    return ret;
}

/*!
    Returns the path to \a fileName relative to the directory.

    \code
        QDir dir("/home/bob");
        QString s;

        s = dir.relativePath("images/file.jpg");     // s is "images/file.jpg"
        s = dir.relativePath("/home/mary/file.txt"); // s is "../mary/file.txt"
    \endcode

    \sa absoluteFilePath() filePath() canonicalPath()
*/

QString QDir::relativeFilePath(const QString &fileName) const
{
    QString dir = absolutePath();
    QString file = cleanPath(fileName);

    if (isRelativePath(file) || isRelativePath(dir))
        return convertSeparators(file);

    QString dirDrive = driveSpec(dir);
    QString fileDrive = driveSpec(file);

    bool fileDriveMissing = false;
    if (fileDrive.isEmpty()) {
        fileDrive = dirDrive;
        fileDriveMissing = true;
    }

    if (fileDrive != dirDrive)
        return convertSeparators(file);

    dir.remove(0, dirDrive.size());
    if (!fileDriveMissing)
        file.remove(0, fileDrive.size());

    QString result;
    QStringList dirElts = dir.split(QLatin1Char('/'), QString::SkipEmptyParts);
    QStringList fileElts = file.split(QLatin1Char('/'), QString::SkipEmptyParts);

    int i = 0;
    while (i < dirElts.size() && i < fileElts.size() && dirElts.at(i) == fileElts.at(i))
        ++i;

    for (int j = 0; j < dirElts.size() - i; ++j)
        result += QLatin1String("../");

    for (int j = i; j < fileElts.size(); ++j) {
        result += fileElts.at(j);
        if (j < fileElts.size() - 1)
            result += QLatin1Char('/');
    }

    return convertSeparators(result);
}

/*!
    Returns \a pathName with the '/' separators converted to
    separators that are appropriate for the underlying operating
    system.

    On Windows, convertSeparators("c:/winnt/system32") returns
    "c:\\winnt\\system32".

    The returned string may be the same as the argument on some
    operating systems, for example on Unix.

    \sa separator()
*/

QString QDir::convertSeparators(const QString &pathName)
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

    Returns true if the new directory exists and is readable;
    otherwise returns false. Note that the logical cd() operation is
    not performed if the new directory does not exist.

    Calling cd("..") is equivalent to calling cdUp().

    \sa cdUp(), isReadable(), exists(), path()
*/

bool QDir::cd(const QString &dirName)
{
    Q_D(QDir);

    if (dirName.isEmpty() || dirName == QLatin1String("."))
        return true;
    QString newPath = d->data->path;
    if (isAbsolutePath(dirName)) {
        newPath = cleanPath(dirName);
    } else {
        if (isRoot()) {
            if (dirName == QLatin1String(".."))
                return false;
        } else {
            newPath += QLatin1Char('/');
        }

        newPath += dirName;
        if (dirName.indexOf(QLatin1Char('/')) >= 0
            || d->data->path == QLatin1String(".")
            || dirName == QLatin1String("..")) {
            newPath = cleanPath(newPath);
            /*
              If newPath starts with .., we convert it to absolute to
              avoid infinite looping on

                  QDir dir(".");
                  while (dir.cdUp())
                      ;
            */
            if (newPath.startsWith(QLatin1String(".."))) {
                newPath = QFileInfo(newPath).absoluteFilePath();
            }
        }
    }
    {
        QFileInfo fi(newPath);
        if(!fi.exists())
            return false;
    }

    d->setPath(newPath);
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

bool QDir::cdUp()
{
    return cd(QString::fromLatin1(".."));
}

/*!
    Returns the string list set by setNameFilters()
*/

QStringList QDir::nameFilters() const
{
    Q_D(const QDir);

    return d->data->nameFilters;
}

/*!
    Sets the name filters used by entryList() and entryInfoList() to the
    list of filters specified by \a nameFilters.

    \sa nameFilters(), setFilter()
*/

void QDir::setNameFilters(const QStringList &nameFilters)
{
    Q_D(QDir);
    d->detach();
    d->data->nameFilters = nameFilters;
}

/*!
    Adds \a path to the search paths searched in to find resources
    that are not specified with an absolute path. The default search
    path is to search only in the root (\c{:/}).

    \sa {The Qt Resource System}
*/

void QDir::addResourceSearchPath(const QString &path)
{
#ifdef QT_BUILD_CORE_LIB
    extern bool qt_resource_add_search_path(const QString &);
    qt_resource_add_search_path(path);
#else
    Q_UNUSED(path)
#endif
}


/*!
    Returns the value set by setFilter()
*/

QDir::Filters QDir::filter() const
{
    Q_D(const QDir);

    return d->data->filters;
}

/*!
    \enum QDir::Filter

    This enum describes the filtering options available to QDir; e.g.
    for entryList() and entryInfoList(). The filter value is specified
    by combining values from the following list using the bitwise OR
    operator:

    \value Dirs    List directories that match the filters.
    \value AllDirs  List all directories; i.e. don't apply the filters
                    to directory names.
    \value Files   List files only.
    \value Drives  List disk drives (ignored under Unix).
    \value NoSymLinks  Do not list symbolic links (ignored by operating
                       systems that don't support symbolic links).
    \value All  List directories, files, drives and symlinks (this does not list
                broken symlinks unless you specify System).
                NoSymLinks flags.
    \value Readable  List files for which the application has read access.
    \value Writable  List files for which the application has write access.
    \value Executable  List files for which the application has
                       execute access. Executables needs to be
                       combined with Dirs or Files.
    \value Modified  Only list files that have been modified (ignored
                     under Unix).
    \value Hidden  List hidden files (on Unix, files starting with a .).
    \value System  List system files (on Unix, FIFOs, sockets and
                   device files)
    \value CaseSensitive  The filter should be case sensitive if the file system
                          is case sensitive.

    \omitvalue DefaultFilter
    \omitvalue TypeMask
    \omitvalue  RWEMask
    \omitvalue AccessMask
    \omitvalue PermissionMask
    \omitvalue NoFilter

    Functions that use \c Filter enum values to filter lists of files
    and directories will include symbolic links to files and directories
    unless you set the \c NoSymLinks value.

    If you do not set any of \c Readable, \c Writable, or \c
    Executable, QDir will set all three of them. This makes the
    default easy to write, and at the same time useful.

    Examples: \c Readable|Writable means list all files for which the
    application has read access, write access or both. \c Dirs|Drives
    means list drives, directories, all files that the application can
    read, write or execute, and also symlinks to such
    files/directories.
*/

/*!
    Sets the filter used by entryList() and entryInfoList() to \a
    filters. The filter is used to specify the kind of files that
    should be returned by entryList() and entryInfoList(). See
    \l{QDir::Filter}.

    \sa filter(), setNameFilters()
*/

void QDir::setFilter(Filters filters)
{
    Q_D(QDir);

    d->detach();
    d->data->filters = filters;
}

/*!
    Returns the value set by setSorting()

    \sa setSorting() SortFlag
*/

QDir::SortFlags QDir::sorting() const
{
    Q_D(const QDir);

    return d->data->sort;
}

/*!
    \enum QDir::SortFlag

    This enum describes the sort options available to QDir, e.g. for
    entryList() and entryInfoList(). The sort value is specified by
    OR-ing together values from the following list:

    \value Name  Sort by name.
    \value Time  Sort by time (modification time).
    \value Size  Sort by file size.
    \value Unsorted  Do not sort.

    \value DirsFirst  Put the directories first, then the files.
    \value DirsLast Put the files first, then the directories.
    \value Reversed  Reverse the sort order.
    \value IgnoreCase  Sort case-insensitively.

    \omitvalue SortByMask
    \omitvalue DefaultSort
    \omitvalue NoSort

    You can only specify one of the first four.

    If you specify both \c DirsFirst and \c Reversed, directories are
    still put first, but in reverse order; the files will be listed
    after the directories, again in reverse order.
*/

/*!
    Sets the sort order used by entryList() and entryInfoList().

    The \a sort is specified by OR-ing values from the enum
    \l{QDir::SortFlag}.

    \sa sorting() SortFlag
*/

void QDir::setSorting(SortFlags sort)
{
    Q_D(QDir);

    d->detach();
    d->data->sort = sort;
}


/*!
    Returns the total number of directories and files in the directory.

    Equivalent to entryList().count().

    \sa operator[](), entryList()
*/

uint QDir::count() const
{
    Q_D(const QDir);

    d->updateFileLists();
    return d->data->files.count();
}

/*!
    Returns the file name at position \a pos in the list of file
    names. Equivalent to entryList().at(index).

    Returns an empty string if \a pos is out of range or if the
    entryList() function failed.

    \sa count(), entryList()
*/

QString QDir::operator[](int pos) const
{
    Q_D(const QDir);

    d->updateFileLists();
    return d->data->files[pos];
}

/*!
    \overload

    Returns a list of the names of all the files and directories in
    the directory, ordered in accordance with setSorting() and
    filtered in accordance with setFilter() and setNameFilters().

    The filter and sorting specifications can be overridden using the
    \a filters and \a sort arguments.

    Returns an empty list if the directory is unreadable or does not
    exist or if nothing matches the specification.

    \sa entryInfoList(), setNameFilters(), setSorting(), setFilter()
*/

QStringList QDir::entryList(Filters filters, SortFlags sort) const
{
    Q_D(const QDir);

    return entryList(d->data->nameFilters, filters, sort);
}


/*!
    \overload

    Returns a list of QFileInfo objects for all the files and
    directories in the directory, ordered in accordance with
    setSorting() and filtered in accordance with setFilter() and
    setNameFilters().

    The filter and sorting specifications can be overridden using the
    \a filters and \a sort arguments.

    Returns an empty list if the directory is unreadable or does not
    exist or if nothing matches the specification.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/

QFileInfoList QDir::entryInfoList(Filters filters, SortFlags sort) const
{
    Q_D(const QDir);

    return entryInfoList(d->data->nameFilters, filters, sort);
}

/*!
    Returns a list of the names of all the files and directories in
    the directory, ordered in accordance with setSorting() and
    filtered in accordance with setFilter() and setNameFilters().

    The name filter, file attributes filter, and the sorting
    specifications can be overridden using the \a nameFilters, \a
    filters and \a sort arguments.

    Returns an empty list if the directory is unreadable or does not
    exist or if nothing matches the specification.

    \sa entryInfoList(), setNameFilters(), setSorting(), setFilter()
*/

QStringList QDir::entryList(const QStringList &nameFilters, Filters filters,
                            SortFlags sort) const
{
    Q_D(const QDir);

    if (filters == NoFilter)
        filters = d->data->filters;
    if (sort == NoSort)
        sort = d->data->sort;
    if (filters == NoFilter && sort == NoSort && nameFilters == d->data->nameFilters) {
        d->updateFileLists();
        return d->data->files;
    }
    QStringList l = d->data->fileEngine->entryList(filters, nameFilters), ret;
    d->sortFileList(sort, l, &ret, 0);
    return ret;
}

/*!
    \overload

    Returns a list of QFileInfo objects for all the files and
    directories in the directory, ordered in accordance with
    setSorting() and filtered in accordance with setFilter() and
    setNameFilters().

    The name filter, file attributes filter, and sorting
    specifications can be overridden using the \a nameFilters, \a
    filters, and \a sort arguments.

    Returns an empty list if the directory is unreadable or does not
    exist or if nothing matches the specification.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/

QFileInfoList QDir::entryInfoList(const QStringList &nameFilters, Filters filters,
                                  SortFlags sort) const
{
    Q_D(const QDir);

    if (filters == NoFilter)
        filters = d->data->filters;
    if (sort == NoSort)
        sort = d->data->sort;
    if (filters == NoFilter && sort == NoSort && nameFilters == d->data->nameFilters) {
        d->updateFileLists();
        return d->data->fileInfos;
    }
    QFileInfoList ret;
    QStringList l = d->data->fileEngine->entryList(filters, nameFilters);
    d->sortFileList(sort, l, 0, &ret);
    return ret;
}

/*!
    Creates a sub-directory called \a dirName.

    Returns true on success; otherwise returns false.

    \sa rmdir()
*/

bool QDir::mkdir(const QString &dirName) const
{
    Q_D(const QDir);

    if (dirName.isEmpty()) {
        qWarning("QDir::mkdir: Empty or null file name(s)");
        return false;
    }
    if(!d->data->fileEngine)
        return false;

    QString fn = filePath(dirName);
    return d->data->fileEngine->mkdir(fn, false);
}

/*!
    Removes the directory specified by \a dirName.

    The directory must be empty for rmdir() to succeed.

    Returns true if successful; otherwise returns false.

    \sa mkdir()
*/

bool QDir::rmdir(const QString &dirName) const
{
    Q_D(const QDir);

    if (dirName.isEmpty()) {
        qWarning("QDir::rmdir: Empty or null file name(s)");
        return false;
    }
    if(!d->data->fileEngine)
        return false;

    QString fn = filePath(dirName);
    return d->data->fileEngine->rmdir(fn, false);
}

/*!
    Creates the directory path \a dirPath.

    The function will create all parent directories necessary to
    create the directory.

    Returns true if successful; otherwise returns false.

    \sa rmpath()
*/

bool QDir::mkpath(const QString &dirPath) const
{
    Q_D(const QDir);

    if (dirPath.isEmpty()) {
        qWarning("QDir::mkpath: Empty or null file name(s)");
        return false;
    }
    if(!d->data->fileEngine)
        return false;

    QString fn = filePath(dirPath);
    return d->data->fileEngine->mkdir(fn, true);
}

/*!
    Removes the directory path \a dirPath.

    The function will remove all parent directories in \a dirPath,
    provided that they are empty. This is the opposite of
    mkpath(dirPath).

    Returns true if successful; otherwise returns false.

    \sa mkpath()
*/
bool QDir::rmpath(const QString &dirPath) const
{
    Q_D(const QDir);

    if (dirPath.isEmpty()) {
        qWarning("QDir::rmpath: Empty or null file name(s)");
        return false;
    }
    if(!d->data->fileEngine)
        return false;

    QString fn = filePath(dirPath);
    return d->data->fileEngine->rmdir(fn, true);
}

/*!
    Returns true if the directory is readable \e and we can open files
    by name; otherwise returns false.

    \warning A false value from this function is not a guarantee that
    files in the directory are not accessible.

    \sa QFileInfo::isReadable()
*/


bool QDir::isReadable() const
{
    Q_D(const QDir);

    if(!d->data->fileEngine)
        return false;
    const QFileEngine::FileFlags info = d->data->fileEngine->fileFlags(QFileEngine::DirectoryType
                                                                       |QFileEngine::PermsMask);
    if(!(info & QFileEngine::DirectoryType))
        return false;
    return info & QFileEngine::ReadUserPerm;
}

/*!
    \overload

    Returns true if the \e directory exists; otherwise returns false.
    (If a file with the same name is found this function will return
    false).

    \sa QFileInfo::exists(), QFile::exists()
*/

bool QDir::exists() const
{
    Q_D(const QDir);

    if(!d->data->fileEngine)
        return false;
    const QFileEngine::FileFlags info = d->data->fileEngine->fileFlags(QFileEngine::DirectoryType
                                                                       |QFileEngine::ExistsFlag);
    if(!(info & QFileEngine::DirectoryType))
        return false;
    return info & QFileEngine::ExistsFlag;
}

/*!
    Returns true if the directory is the root directory; otherwise
    returns false.

    Note: If the directory is a symbolic link to the root directory
    this function returns false. If you want to test for this use
    canonicalPath(), e.g.

    \code
        QDir dir("/tmp/root_link");
        dir = dir.canonicalPath();
        if (dir.isRoot())
            qWarning("It is a root link");
    \endcode

    \sa root(), rootPath()
*/

bool QDir::isRoot() const
{
    Q_D(const QDir);

    if(!d->data->fileEngine)
        return true;
    return d->data->fileEngine->fileFlags(QFileEngine::FlagsMask) & QFileEngine::RootFlag;
}

/*!
    \fn bool QDir::isAbsolute() const

    Returns true if the directory's path is absolute; otherwise
    returns false. See isAbsolutePath().

    \sa isRelative() makeAbsolute() cleanPath()
*/

/*!
   \fn bool QDir::isAbsolutePath(const QString &)

    Returns true if \a path is absolute; returns false if it is
    relative.

    \sa isAbsolute() isRelativePath() makeAbsolute() cleanPath()
*/

/*!
    Returns true if the directory path is relative; otherwise returns
    false. (Under Unix a path is relative if it does not start with a
    "/").

    \sa makeAbsolute() isAbsolute() isAbsolutePath() cleanPath()
*/

bool QDir::isRelative() const
{
    Q_D(const QDir);

    if(!d->data->fileEngine)
        return false;
    return d->data->fileEngine->isRelativePath();
}


/*!
    Converts the directory path to an absolute path. If it is already
    absolute nothing happens. Returns true if the conversion
    succeeded; otherwise returns false.

    \sa isAbsolute() isAbsolutePath() isRelative() cleanPath()
*/

bool QDir::makeAbsolute() // ### What do the return values signify?
{
    Q_D(QDir);

    if(!d->data->fileEngine)
        return false;
    QString absolutePath = d->data->fileEngine->fileName(QFileEngine::AbsoluteName);
    if(QDir::isRelativePath(absolutePath))
        return false;
    d->detach();
    d->data->path = absolutePath;
    d->data->fileEngine->setFileName(absolutePath);
    if(!(d->data->fileEngine->fileFlags(QFileEngine::TypesMask) & QFileEngine::DirectoryType))
        return false;
    return true;
}

/*!
    Returns true if directory \a dir and this directory have the same
    path and their sort and filter settings are the same; otherwise
    returns false.

    Example:

    \code
        // The current directory is "/usr/local"
        QDir d1("/usr/local/bin");
        QDir d2("bin");
        if (d1 == d2)
            qDebug("They're the same");
    \endcode
*/

bool QDir::operator==(const QDir &dir) const
{
    const QDirPrivate *d = d_func();
    const QDirPrivate *other = dir.d_func();

    if(d->data == other->data)
        return true;
    Q_ASSERT(d->data->fileEngine && other->data->fileEngine);
    if(d->data->fileEngine->type() != other->data->fileEngine->type() ||
       d->data->fileEngine->caseSensitive() != other->data->fileEngine->caseSensitive())
        return false;
    if(d->data->filters == other->data->filters
       && d->data->sort == other->data->sort
       && d->data->nameFilters == other->data->nameFilters) {
        QString dir1 = absolutePath(), dir2 = dir.absolutePath();
        if(!other->data->fileEngine->caseSensitive())
            return (dir1.toLower() == dir2.toLower());

        return (dir1 == dir2);

    }
    return false;
}

/*!
    Makes a copy of the \a dir object and assigns it to this QDir
    object.
*/

QDir &QDir::operator=(const QDir &dir)
{
    if (this == &dir)
        return *this;

    Q_D(QDir);
    qAtomicAssign(d->data, dir.d_func()->data);
    return *this;
}

/*!
    \overload
    \obsolete

    Sets the directory path to the given \a path.

    Use setPath() instead.
*/

QDir &QDir::operator=(const QString &path)
{
    Q_D(QDir);

    d->setPath(path);
    return *this;
}

/*!
    \fn bool QDir::operator!=(const QDir &dir) const

    Returns true if directory \a dir and this directory have different
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

    Returns true if the file is removed successfully; otherwise
    returns false.
*/

bool QDir::remove(const QString &fileName)
{
    if (fileName.isEmpty()) {
        qWarning("QDir::remove: Empty or null file name");
        return false;
    }
    QString p = filePath(fileName);
    return QFile::remove(p);
}

/*!
    Renames a file or directory from \a oldName to \a newName, and returns
    true if successful; otherwise returns false.

    On most file systems, rename() fails only if \a oldName does not
    exist or if \a newName and \a oldName are not on the same
    partition. On Windows, rename() will fail if \a newName already
    exists. However, there are also other reasons why rename() can
    fail. For example, on at least one file system rename() fails if
    \a newName points to an open file.
*/

bool QDir::rename(const QString &oldName, const QString &newName)
{
    Q_D(QDir);

    if (oldName.isEmpty() || newName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name(s)");
        return false;
    }
    if(!d->data->fileEngine)
        return false;

    QFile file(filePath(oldName));
    if(!file.exists())
        return false;
    return file.rename(filePath(newName));
}

/*!
    Returns true if the file called \a name exists; otherwise returns
    false.

    \sa QFileInfo::exists(), QFile::exists()
*/

bool QDir::exists(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("QDir::exists: Empty or null file name");
        return false;
    }
    QString tmp = filePath(name);
    return QFile::exists(tmp);
}

/*!
    Returns a list of the root directories on this system. On Windows
    this returns a number of QFileInfo objects containing "C:/",
    "D:/", etc. On other operating systems, it returns a list
    containing just one root directory (i.e. "/").
*/

QFileInfoList QDir::drives()
{
    return QFSFileEngine::drives();
}

/*!
    Returns the native directory separator: "/" under Unix (including
    Mac OS X) and "\\" under Windows.

    You do not need to use this function to build file paths. If you
    always use "/", Qt will translate your paths to conform to the
    underlying operating system. If you want to display paths to the
    user using their operating system's separator use
    convertSeparators().
*/

QChar QDir::separator()
{
#if defined(Q_OS_UNIX)
    return QLatin1Char('/');
#elif defined (Q_FS_FAT) || defined(Q_WS_WIN)
    return QLatin1Char('\\');
#elif defined (Q_OS_MAC)
    return QLatin1Char(':');
#else
    return QLatin1Char('/');
#endif
}

/*!
    Sets the application's current working directory to \a path.
    Returns true if the directory was successfully changed; otherwise
    returns false.

    \sa current() currentPath() home() root() temp()
*/

bool QDir::setCurrent(const QString &path)
{
    return QFSFileEngine::setCurrentPath(path);
}

/*!
    \fn QDir QDir::current()

    Returns the absolute path of the application's current directory.
    See currentPath() for details.

    \sa drives() homePath() rootPath() tempPath()
*/

/*!
    Returns the absolute path of the application's current directory.

    \sa current() drives() homePath() rootPath() tempPath()
*/

QString QDir::currentPath()
{
    return QFSFileEngine::currentPath();
}

/*!
    \fn QDir QDir::home()

    Returns the user's home directory. See homePath() for details.

    \sa drives() currentPath() rootPath() tempPath()
*/

/*!
    Returns the user's home directory.

    Under Windows the \c HOME environment variable is used. If this
    does not exist the \c USERPROFILE environment variable is used. If
    that does not exist the path is formed by concatenating the \c
    HOMEDRIVE and \c HOMEPATH environment variables. If they don't
    exist the rootPath() is used (this uses the \c SystemDrive
    environment variable). If none of these exist "C:\" is used.

    Under non-Windows operating systems the \c HOME environment
    variable is used if it exists, otherwise rootPath() is used.

    \sa home() drives() currentPath() rootPath() tempPath()
*/

QString QDir::homePath()
{
    return QFSFileEngine::homePath();
}

/*!
    \fn QDir QDir::temp()

    Returns the system's temporary directory. See tempPath() for
    details.

    \sa drives() currentPath() homePath() rootPath()
*/

/*!
    Returns the system's temporary directory.

    On Unix/Linux systems this is usually \c{/tmp}; on Windows this is
    usually the path in the \c TEMP or \c TMP environment variable.

    \sa temp() drives() currentPath() homePath() rootPath()
*/
QString QDir::tempPath()
{
    return QFSFileEngine::tempPath();
}

/*!
    \fn QDir QDir::root()

    Returns the root directory. See rootPath() for details.

    \sa drives() current() home() temp()
*/

/*!
    Returns the absolute path for the root directory.

    For Unix operating systems this returns "/". For Windows file
    systems this normally returns "c:/".

    \sa root() drives() currentPath() homePath() tempPath()
*/

QString QDir::rootPath()
{
    return QFSFileEngine::rootPath();
}

#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns true if the \a fileName matches any of the wildcard (glob)
    patterns in the list of \a filters; otherwise returns false. The
    matching is case insensitive.

    \sa {QRegExp wildcard matching}, QRegExp::exactMatch() entryList() entryInfoList()
*/


bool QDir::match(const QStringList &filters, const QString &fileName)
{
    for(QStringList::ConstIterator sit = filters.begin(); sit != filters.end(); ++sit) {
        QRegExp rx(*sit, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (rx.exactMatch(fileName))
            return true;
    }
    return false;
}

/*!
    Returns true if the \a fileName matches the wildcard (glob)
    pattern \a filter; otherwise returns false. The \a filter may
    contain multiple patterns separated by spaces or semicolons.
    The matching is case insensitive.

    \sa {QRegExp wildcard matching}, QRegExp::exactMatch() entryList() entryInfoList()
*/

bool QDir::match(const QString &filter, const QString &fileName)
{
    return match(QStringList(filter), fileName);
}
#endif

/*!
    Removes all multiple directory separators "/" and resolves any
    "."s or ".."s found in the path, \a path.

    Symbolic links are kept. This function does not return the
    canonical path, but rather the simplest version of the input.
    For example, "./local" becomes "local", "local/../bin" becomes
    "bin" and "/local/usr/../bin" becomes "/local/bin".

    \sa absolutePath() canonicalPath()
*/

QString QDir::cleanPath(const QString &path)
{
    if (path.isEmpty())
        return path;
    QString name = path;
    QChar dir_separator = separator();
    if(dir_separator != QLatin1Char('/'))
	name.replace(dir_separator, QLatin1Char('/'));

    int used = 0, levels = 0;
    const int len = name.length();
    QVector<QChar> out(len);
    const QChar *p = name.unicode();
    for(int i = 0, last = -1, iwrite = 0; i < len; i++) {
        if(p[i] == QLatin1Char('/')) {
            while(i < len-1 && p[i+1] == QLatin1Char('/')) {
#ifdef Q_OS_WIN //allow unc paths
                if(!i)
                    break;
#endif
                i++;
            }
            bool eaten = false;
            if(i < len - 1 && p[i+1] == QLatin1Char('.')) {
                int dotcount = 1;
                if(i < len - 2 && p[i+2] == QLatin1Char('.'))
                    dotcount++;
                if(i == len - dotcount - 1) {
                    if(dotcount == 1) {
                        break;
                    } else if(levels) {
                        if(last == -1) {
                            for(int i2 = iwrite-1; i2 >= 0; i2--) {
                                if(out[i2] == QLatin1Char('/')) {
                                    last = i2;
                                    break;
                                }
                            }
                        }
                        used -= iwrite - last - 1;
                        break;
                    }
                } else if(p[i+dotcount+1] == QLatin1Char('/')) {
                    if(dotcount == 2 && levels) {
                        if(last == -1 || iwrite - last == 1) {
                            for(int i2 = (last == -1) ? (iwrite-1) : (last-1); i2 >= 0; i2--) {
                                if(out[i2] == QLatin1Char('/')) {
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
                } else {
                    levels++;
                }
            } else if(last != -1 && iwrite - last == 1) {
#ifdef Q_OS_WIN
                eaten = (iwrite > 2);
#else
                eaten = true;
#endif
                last = -1;
            } else if(last != -1 && i == len-1) {
                eaten = true;
            } else {
                levels++;
            }
            if(!eaten)
                last = i - (i - iwrite);
            else
                continue;
        } else if(!i && p[i] == QLatin1Char('.')) {
            int dotcount = 1;
            if(len >= 1 && p[1] == QLatin1Char('.'))
                dotcount++;
            if(len >= dotcount && p[dotcount] == QLatin1Char('/')) {
                if(dotcount == 1) {
                    i++;
                    while(i+1 < len-1 && p[i+1] == QLatin1Char('/'))
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
	ret = QString(out.data(), used);

    // Strip away last slash except for root directories
    if (ret.endsWith(QLatin1Char('/'))
        && !(ret.size() == 1 || (ret.size() == 3 && ret.at(1) == QLatin1Char(':'))))
        ret = ret.left(ret.length() - 1);

    return ret;
}

/*!
    Returns true if \a path is relative; returns false if it is
    absolute.

    \sa isRelative() isAbsolutePath() makeAbsolute()
*/

bool QDir::isRelativePath(const QString &path)
{
    return QFileInfo(path).isRelative();
}

/*!
    Refreshes the directory information.
*/

void QDir::refresh() const
{
    Q_D(const QDir);

    d->data->clear();
}

/*!
    \internal

    Returns a list of name filters from the given \a nameFilter. (If
    there is more than one filter, each pair of filters is separated
    by a space or by a semicolon.)
*/

QStringList QDir::nameFiltersFromString(const QString &nameFilter)
{
    return QDirPrivate::splitFilters(nameFilter);
}

#ifdef QT3_SUPPORT
/*!
    Use nameFilters() instead.
*/
QString QDir::nameFilter() const
{
    Q_D(const QDir);

    return nameFilters().join(QString(d->filterSepChar));
}

/*!
    Use setNameFilters() instead.

    The \a nameFilter is a wildcard (globbing) filter that understands
    "*" and "?" wildcards. (See \l{QRegExp wildcard matching}.) You may
    specify several filter entries, each separated by spaces or by
    semicolons.

    For example, if you want entryList() and entryInfoList() to list
    all files ending with either ".cpp" or ".h", you would use either
    dir.setNameFilters("*.cpp *.h") or dir.setNameFilters("*.cpp;*.h").

    \oldcode
        QString filter = "*.cpp *.cxx *.cc";
        dir.setNameFilter(filter);
    \newcode
        QString filter = "*.cpp *.cxx *.cc";
        dir.setNameFilters(filter.split(' '));
    \endcode
*/
void QDir::setNameFilter(const QString &nameFilter)
{
    Q_D(QDir);

    d->filterSepChar = QDirPrivate::getFilterSepChar(nameFilter);
    setNameFilters(QDirPrivate::splitFilters(nameFilter, d->filterSepChar));
}

/*!
    \fn QString QDir::absPath() const

    Use absolutePath() instead.
*/

/*!
    \fn QString QDir::absFilePath(const QString &fileName, bool acceptAbsPath) const

    Use absoluteFilePath(\a fileName) instead.

    The \a acceptAbsPath parameter is ignored.
*/

/*!
    \fn bool QDir::mkdir(const QString &dirName, bool acceptAbsPath) const

    Use mkdir(\a dirName) instead.

    The \a acceptAbsPath parameter is ignored.
*/

/*!
    \fn bool QDir::rmdir(const QString &dirName, bool acceptAbsPath) const

    Use rmdir(\a dirName) instead.

    The \a acceptAbsPath parameter is ignored.
*/

/*!
    \fn bool QDir::matchAllDirs() const

    Use filter() & AllDirs instead.
*/

/*!
    \fn void QDir::setMatchAllDirs(bool on)

    Use setFilter() instead.
*/

/*!
    \fn QStringList QDir::entryList(const QString &nameFilter, Filters filters,
                                    SortFlags sort) const

    Use the overload that takes a name filter string list as first
    argument instead.
*/

/*!
    \fn QFileInfoList QDir::entryInfoList(const QString &nameFilter, Filters filters,
                                          SortFlags sort) const

    Use the overload that takes a name filter string list as first
    argument instead.
*/

/*!
    \fn void QDir::convertToAbs()

    Use makeAbsolute() instead.
*/

/*!
    \fn QString QDir::cleanDirPath(const QString &name)

    Use cleanPath() instead.
*/

/*!
    \typedef QDir::FilterSpec

    Use QDir::Filters instead.
*/

/*!
    \typedef QDir::SortSpec

    Use QDir::SortFlags instead.
*/

#endif
