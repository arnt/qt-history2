/****************************************************************************
**
** Definition of QDir class.
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

#ifndef QDIR_H
#define QDIR_H

#include "qstring.h"
#include "qfileinfo.h"
#include "qstringlist.h"

class QDirPrivate;

class Q_CORE_EXPORT QDir
{
protected:
    QDirPrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QDir)
public:
    enum FilterSpec { Dirs        = 0x001,
                      Files       = 0x002,
                      Drives      = 0x004,
                      NoSymLinks  = 0x008,
                      All         = 0x007,
                      TypeMask    = 0x00F,

                      Readable    = 0x010,
                      Writable    = 0x020,
                      Executable  = 0x040,
                      RWEMask     = 0x070,

                      Modified    = 0x080,
                      Hidden      = 0x100,
                      System      = 0x200,
                      AccessMask  = 0x3F0,

                      AllDirs     = 0x400,

                      DefaultFilter = -1 };

    enum SortSpec   { Name        = 0x00,
                      Time        = 0x01,
                      Size        = 0x02,
                      Unsorted    = 0x03,
                      SortByMask  = 0x03,

                      DirsFirst   = 0x04,
                      Reversed    = 0x08,
                      IgnoreCase  = 0x10,
                      DirsLast    = 0x20,
                      DefaultSort = -1 };

    QDir(const QDir &);
    QDir(const QString &path = QString());
    QDir(const QString &path, const QString &nameFilter,
         int sortSpec = Name | IgnoreCase, int filterSpec = All);
    ~QDir();

    QDir &operator=(const QDir &);
    QDir &operator=(const QString &path);

    void setPath(const QString &path);
    QString path() const;
    QString absolutePath() const;
#ifdef QT_COMPAT
    inline QT_COMPAT QString absPath() const { return absolutePath(); }
#endif
    QString canonicalPath() const;

    QString dirName() const;
    QString filePath(const QString &fileName,
                     bool acceptAbsPath = true) const;
    QString absoluteFilePath(const QString &fileName,
                             bool acceptAbsPath = true) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QString absFilePath(const QString &fileName,
                                         bool acceptAbsPath = true) const
       { return absoluteFilePath(fileName, acceptAbsPath); }
#endif

    static QString convertSeparators(const QString &pathName);

    bool cd(const QString &dirName, bool acceptAbsPath = true);
    bool cdUp();

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);

    FilterSpec filter() const;
    void setFilter(int filterSpec);
    SortSpec sorting() const;
    void setSorting(int sortSpec);

#ifdef QT_COMPAT
    inline bool QT_COMPAT matchAllDirs() const
        { return filter() & AllDirs; }
    inline void QT_COMPAT setMatchAllDirs(bool on)
        { setFilter((filter() & ~AllDirs) | (on ? AllDirs : 0)); }
#endif

    uint count() const;
    QString operator[](int) const;

    static QStringList nameFiltersFromString(const QString &nameFilter);

    QStringList entryList(int filterSpec = DefaultFilter, int sortSpec = DefaultSort) const;
    QStringList entryList(const QStringList &nameFilters, int filterSpec = DefaultFilter,
                          int sortSpec = DefaultSort) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QStringList entryList(const QString &nameFilter, int filterSpec = DefaultFilter,
                                           int sortSpec = DefaultSort) const
      { return entryList(nameFiltersFromString(nameFilter), filterSpec, sortSpec); }
#endif

    QFileInfoList entryInfoList(int filterSpec = DefaultFilter, int sortSpec = DefaultSort) const;
    QFileInfoList entryInfoList(const QStringList &nameFilters, int filterSpec = DefaultFilter,
                                int sortSpec = DefaultSort) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QFileInfoList entryInfoList(const QString &nameFilter, int filterSpec = DefaultFilter,
                                                 int sortSpec = DefaultSort) const
       { return entryInfoList(nameFiltersFromString(nameFilter), filterSpec, sortSpec); }
#endif

#ifdef QT_COMPAT
    QT_COMPAT QString nameFilter() const;
    QT_COMPAT void setNameFilter(const QString &nameFilter);
#endif

    enum Recursion { Recursive = 0, NonRecursive = 1 };
    bool mkdir(const QString &dirName, Recursion recurse=NonRecursive, bool acceptAbsPath=true) const;
    bool rmdir(const QString &dirName, Recursion recurse=NonRecursive, bool acceptAbsPath=true) const;
#ifdef QT_COMPAT
    inline QT_COMPAT bool mkdir(const QString &dirName, bool acceptAbsPath) const
        { return mkdir(dirName, NonRecursive, acceptAbsPath); }
    inline QT_COMPAT bool rmdir(const QString &dirName, bool acceptAbsPath) const
        { return rmdir(dirName, NonRecursive, acceptAbsPath); }
#endif

    bool isReadable() const;
    bool exists() const;
    bool isRoot() const;

    static bool isRelativePath(const QString &path);
    inline static bool isAbsolutePath(const QString &path) { return !isRelativePath(path); }
    bool isRelative() const;
    inline bool isAbsolute() { return !isRelative(); }
    bool makeAbsolute();
#ifdef QT_COMPAT
    inline QT_COMPAT void convertToAbs() { makeAbsolute(); }
#endif

    bool operator==(const QDir &dir) const;
    inline bool operator!=(const QDir &dir) const {  return !operator==(dir); }

    bool remove(const QString &fileName, bool acceptAbsPath = true);
    bool rename(const QString &oldName, const QString &newName, bool acceptAbsPaths = true);
    bool exists(const QString &name, bool acceptAbsPath = true) const;

    static QFileInfoList drives();

    static QChar separator();

    static bool setCurrent(const QString &path);
    static inline QDir current() { return QDir(currentPath()); }
    static QString currentPath();
#ifdef QT_COMPAT
    inline QT_COMPAT static QString currentDirPath() { return currentPath(); }
#endif

    static inline QDir home() { return QDir(homePath()); }
    static QString homePath();
    static inline QDir root() { return QDir(rootPath()); }
    static QString rootPath();
    static inline QDir temp() { return QDir(tempPath()); }
    static QString tempPath();
#ifdef QT_COMPAT
    inline QT_COMPAT static QString homeDirPath() { return homePath(); }
    inline QT_COMPAT static QString rootDirPath() { return rootPath(); }
#endif

#ifndef QT_NO_REGEXP
    bool match(const QStringList &filters, const QString &fileName);
    bool match(const QString &filter, const QString &fileName);
#endif
    static QString cleanPath(const QString &path);
#ifdef QT_COMPAT
    inline QT_COMPAT static QString cleanDirPath(const QString &name) { return cleanPath(name); }
#endif
    void refresh() const;
};

#endif // QDIR_H
