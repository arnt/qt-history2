/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*!
    \since 4.3
    \class QDirIterator
    \brief The QDirIterator class provides an iterator for directory entrylists.

    You can use QDirIterator to navigate entries of a directory one at a time.
    It is similar to QDir::entryList() and QDir::entryInfoList(), but because
    it lists entries one at a time instead of all at once, it scales better
    and is more suitable for large directories. It also supports listing
    directory contents recursively, and following symbolic links. Unlike
    QDir::entryList(), QDirIterator does not support sorting.

    The QDirIterator constructor takes a QDir or a directory as
    argument. After construction, the iterator is located before the first
    directory entry. Here's how to iterate over all the entries sequentially:

    \code
        QDirIterator it("/etc", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            qDebug() << it.next();

            // /etc/.
            // /etc/..
            // /etc/X11
            // /etc/X11/fs
            // ...
        }
    \endcode

    The next() function returns the path to the next directory entry and
    advances the iterator. You can also call filePath() to get the current
    file path without advancing the iterator.  The fileName() function returns
    only the name of the file, similar to how QDir::entryList() works. You can
    also call fileInfo() to get a QFileInfo for the current entry.

    Unlike Qt's container iterators, QDirIterator is uni-directional (i.e.,
    you cannot iterate directories in reverse order) and does not allow random
    access.

    QDirIterator works with all supported file engines, and is implemented
    using QAbstractFileEngineIterator.

    \sa QDir, QDir::entryList(), QAbstractFileEngineIterator
*/

/*! \enum QDirIterator::IteratorFlag

    This enum describes flags that you can combine to configure the behavior
    of QDirIterator.

    \value NoIteratorFlags The default value, representing no flags. The
    iterator will return entries for the assigned path.

    \value Subdirectories List entries inside all subdirectories as well.

    \value FollowSymlinks When combined with \l Subdirectories, this flag
    enables iterating through all subdirectories of the assigned path,
    following all symbolic links. Symbolic link loops (e.g., "link" => "." or
    "link" => "..") are automatically detected and ignored.
*/

#include "qdiriterator.h"

#include "qabstractfileengine.h"

#include <QtCore/qset.h>
#include <QtCore/qstack.h>
#include <QtCore/qvariant.h>

class QDirIteratorPrivate
{
public:
    QDirIteratorPrivate(const QString &path, const QStringList &nameFilters,
                        QDir::Filters filters, QDirIterator::IteratorFlags flags);
    ~QDirIteratorPrivate();

    void pushSubDirectory(const QString &path, const QStringList &nameFilters,
                          QDir::Filters filters);
    void advance();
    bool matchesFilters(const QAbstractFileEngineIterator *it) const;

    QSet<QString> visitedLinks;
    QAbstractFileEngine *engine;
    QStack<QAbstractFileEngineIterator *> fileEngineIterators;
    QString path;
    QFileInfo fileInfo;
    QString currentFilePath;
    QDirIterator::IteratorFlags iteratorFlags;
    QDir::Filters filters;
    QStringList nameFilters;
    bool followNextDir;
    bool first;
    bool done;

    QDirIterator *q;
};

/*!
    \internal
*/
QDirIteratorPrivate::QDirIteratorPrivate(const QString &path, const QStringList &nameFilters,
                                         QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : engine(0), path(path), iteratorFlags(flags), followNextDir(false), first(true), done(false)
{
    if (filters == QDir::NoFilter)
        filters = QDir::AllEntries;
    this->filters = filters;
    this->nameFilters = nameFilters;

    fileInfo.setFile(path);
    pushSubDirectory(fileInfo.isSymLink() ? fileInfo.canonicalFilePath() : path,
                     nameFilters, filters);
}

/*!
    \internal
*/
QDirIteratorPrivate::~QDirIteratorPrivate()
{
    delete engine;
}

/*!
    \internal
*/
void QDirIteratorPrivate::pushSubDirectory(const QString &path, const QStringList &nameFilters,
                                           QDir::Filters filters)
{
    if (iteratorFlags & QDirIterator::FollowSymlinks) {
        if (fileInfo.filePath() != path)
            fileInfo.setFile(path);
        if (fileInfo.isSymLink()) {
            visitedLinks << fileInfo.canonicalFilePath();
        } else {
            visitedLinks << fileInfo.absoluteFilePath();
        }
    }
    
    if (engine || (engine = QAbstractFileEngine::create(this->path))) {
        engine->setFileName(path);
        QAbstractFileEngineIterator *it = engine->beginEntryList(filters, nameFilters);
        if (it) {
            it->setPath(path);
            fileEngineIterators << it;
        } else {
            // No iterator; no entry list.
        }
    }
}

/*!
    \internal
*/
void QDirIteratorPrivate::advance()
{
    // Store the current entry
    if (!fileEngineIterators.isEmpty())
        currentFilePath = fileEngineIterators.top()->currentFilePath();

    // Advance to the next entry
    if (followNextDir) {
        // Start by navigating into the current directory.
        followNextDir = false;

        QAbstractFileEngineIterator *it = fileEngineIterators.top();

        QString subDir = it->currentFilePath();
#ifdef Q_OS_WIN
        if (fileInfo.isSymLink())
            subDir = fileInfo.canonicalFilePath();
#endif
        pushSubDirectory(subDir, it->nameFilters(), it->filters());
    }

    if (fileEngineIterators.isEmpty())
        done = true;

    bool foundValidEntry = false;
    while (!fileEngineIterators.isEmpty()) {
        QAbstractFileEngineIterator *it = fileEngineIterators.top();

        // Find the next valid iterator that matches the filters.
        foundValidEntry = false;
        while (it->hasNext()) {
            it->next();
            if (matchesFilters(it)) {
                foundValidEntry = true;
                break;
            }
        }

        if (!foundValidEntry) {
            // If this iterator is done, pop and delete it, and continue
            // iteration on the parent. Otherwise break, we're done.
            if (!fileEngineIterators.isEmpty()) {
                delete it;
                it = fileEngineIterators.pop();
                continue;
            }
            break;
        }

        fileInfo = it->currentFileInfo();

        // If we're doing flat iteration, we're done.
        if (!(iteratorFlags & QDirIterator::Subdirectories))
            break;

        // Subdirectory iteration.
        QString filePath = fileInfo.filePath();

        // Never follow . and ..
        if (fileInfo.fileName() == QLatin1String(".") || fileInfo.fileName() == QLatin1String(".."))
            break;

        // Never follow non-directory entries
        if (!fileInfo.isDir())
            break;
      
        // Check symlinks
        if (fileInfo.isSymLink() && !(iteratorFlags & QDirIterator::FollowSymlinks)) {
            // Follow symlinks only if FollowSymlinks was passed
            break;
        }

        // Stop link loops
        if (visitedLinks.contains(fileInfo.canonicalFilePath()))
            break;

        // Signal that we want to follow this entry.
        followNextDir = true;
        break;
    }

    if (!foundValidEntry)
        done = true;
}

/*!
    \internal

    This convenience function implements the iterator's filtering logics and
    applies then to the current directory entry.

    It returns true if the current entry matches the filters (i.e., the
    current entry will be returned as part of the directory iteration);
    otherwise, false is returned.
*/
bool QDirIteratorPrivate::matchesFilters(const QAbstractFileEngineIterator *it) const
{
    const bool filterPermissions = ((filters & QDir::PermissionMask)
                                    && (filters & QDir::PermissionMask) != QDir::PermissionMask);
    const bool skipDirs     = !(filters & (QDir::Dirs | QDir::AllDirs));
    const bool skipFiles    = !(filters & QDir::Files);
    const bool skipSymlinks = (filters & QDir::NoSymLinks);
    const bool doReadable   = !filterPermissions || (filters & QDir::Readable);
    const bool doWritable   = !filterPermissions || (filters & QDir::Writable);
    const bool doExecutable = !filterPermissions || (filters & QDir::Executable);
    const bool includeHidden = (filters & QDir::Hidden);
    const bool includeSystem = (filters & QDir::System);

#ifndef QT_NO_REGEXP
    // Prepare name filters
    QList<QRegExp> regexps;
    bool hasNameFilters = !nameFilters.isEmpty() && !(nameFilters.contains(QLatin1String("*")));
    if (hasNameFilters) {
        for (int i = 0; i < nameFilters.size(); ++i) {
            regexps << QRegExp(nameFilters.at(i),
                               (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                               QRegExp::Wildcard);
        }
    }
#endif

    QString fileName = it->currentFileName();
    if (fileName.isEmpty()) {
        // invalid entry
        return false;
    }

    QFileInfo fi = it->currentFileInfo();
    QString filePath = it->currentFilePath();

#ifndef QT_NO_REGEXP
    // Pass all entries through name filters, except dirs if the AllDirs
    // filter is passed.
    if (hasNameFilters && !((filters & QDir::AllDirs) && fi.isDir())) {
        bool matched = false;
        for (int i = 0; i < regexps.size(); ++i) {
            if (regexps.at(i).exactMatch(fileName)) {
                matched = true;
                break;
            }
        }
        if (!matched)
            return false;
    }
#endif
    
    bool dotOrDotDot = (fileName == QLatin1String(".") || fileName == QLatin1String(".."));
    if ((filters & QDir::NoDotAndDotDot) && dotOrDotDot)
        return false;

    bool isHidden = !dotOrDotDot && fi.isHidden();
    if (!includeHidden && isHidden)
        return false;
    
    bool alwaysShow = (filters & QDir::TypeMask) == 0
        && ((isHidden && includeHidden)
            || (includeSystem && ((fi.exists() && !fi.isFile() && !fi.isDir() && !fi.isSymLink())
                                  || (!fi.exists() && fi.isSymLink()))));

    // Skip files and directories
    if ((filters & QDir::AllDirs) == 0 && skipDirs && fi.isDir()) {
        if (!alwaysShow)
            return false;
    }

    if ((skipFiles && (fi.isFile() || !fi.exists()))
        || (skipSymlinks && fi.isSymLink())) {
        if (!alwaysShow)
            return false;
    }

    if (filterPermissions
        && ((doReadable && !fi.isReadable())
            || (doWritable && !fi.isWritable())
            || (doExecutable && !fi.isExecutable()))) {
        return false;
    }

    if (!includeSystem && !dotOrDotDot && ((fi.exists() && !fi.isFile() && !fi.isDir() && !fi.isSymLink())
                                           || (!fi.exists() && fi.isSymLink()))) {
        return false;
    }
    
    return true;
}

/*!
    Constructs a QDirIterator that can iterate over \a dir's entrylist, using
    \a dir's name filters and regular filters. You can pass options via \a
    flags to decide how the directory should be iterated.

    By default, \a flags is NoIteratorFlags, which provides the same behavior
    as in QDir::entryList().

    The sorting in \a dir is ignored.

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QDir &dir, IteratorFlags flags)
    : d(new QDirIteratorPrivate(dir.path(), dir.nameFilters(), dir.filter(), flags))
{
    d->q = this;
}

/*!
    Constructs a QDirIterator that can iterate over \a path, with no name
    filtering and \a filters for entry filtering. You can pass options via \a
    flags to decide how the directory should be iterated.

    By default, \a filters is QDir::NoFilter, and \a flags is NoIteratorFlags,
    which provides the same behavior as in QDir::entryList().

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QString &path, QDir::Filters filters, IteratorFlags flags)
    : d(new QDirIteratorPrivate(path, QStringList(QLatin1String("*")), filters, flags))
{
    d->q = this;
}

/*!
    Constructs a QDirIterator that can iterate over \a path. You can pass
    options via \a flags to decide how the directory should be iterated.

    By default, \a flags is NoIteratorFlags, which provides the same behavior
    as in QDir::entryList().

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QString &path, IteratorFlags flags)
    : d(new QDirIteratorPrivate(path, QStringList(QLatin1String("*")), QDir::NoFilter, flags))
{
    d->q = this;
}

/*!
    Constructs a QDirIterator that can iterate over \a path, using \a
    nameFilters and \a filters. You can pass options via \a flags to decide
    how the directory should be iterated.

    By default, \a flags is NoIteratorFlags, which provides the same behavior
    as QDir::entryList().

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QString &path, const QStringList &nameFilters,
                           QDir::Filters filters, IteratorFlags flags)
    : d(new QDirIteratorPrivate(path, nameFilters, filters, flags))
{
    d->q = this;
}

/*!
    Destroys the QDirIterator.
*/
QDirIterator::~QDirIterator()
{
    qDeleteAll(d->fileEngineIterators);
    delete d;
}

/*!
    Advances the iterator to the next entry, and returns the file path of this
    new entry. If hasNext() returns false, this function does nothing, and
    returns a null QString.

    You can call fileName() or filePath() to get the current entry file name
    or path, or fileInfo() to get a QFileInfo for the current entry.

    \sa hasNext(), fileName(), filePath(), fileInfo()
*/
QString QDirIterator::next()
{
    if (!hasNext())
        return QString();
    d->advance();
    return filePath();
}

/*!
    Returns true if there is at least one more entry in the directory;
    otherwise, false is returned.

    \sa next(), fileName(), filePath(), fileInfo()
*/
bool QDirIterator::hasNext() const
{
    if (d->first) {
        d->first = false;
        d->advance();
        if (!d->fileEngineIterators.isEmpty())
            d->currentFilePath = d->fileEngineIterators.top()->currentFilePath();
    }
    return !d->done;
}

/*!
    Returns the file name for the current directory entry, without the path
    prepended. If the current entry is invalid (i.e., isValid() returns
    false), a null QString is returned.

    This function is provided for the convenience when iterating single
    directories. For recursive iteration, you should call filePath() or
    fileInfo() instead.
    
    \sa filePath(), fileInfo()
*/
QString QDirIterator::fileName() const
{
    if (d->fileInfo.path() != d->currentFilePath)
        d->fileInfo.setFile(d->currentFilePath);
    return d->fileInfo.fileName();
}

/*!
    Returns the full file path for the current directory entry. If the current
    entry is invalid (i.e., isValid() returns false), a null QString is
    returned.

    \sa fileInfo(), fileName()
*/
QString QDirIterator::filePath() const
{
    return d->currentFilePath;
}

/*!
    Returns a QFileInfo for the current directory entry. If the current entry
    is invalid (i.e., isValid() returns false), a null QFileInfo is returned.

    \sa filePath(), fileName()
*/
QFileInfo QDirIterator::fileInfo() const
{
    if (d->fileInfo.path() != d->currentFilePath)
        d->fileInfo.setFile(d->currentFilePath);
    return d->fileInfo;
}

/*!
    Returns the base directory of the iterator.
*/
QString QDirIterator::path() const
{
    return d->path;
}
