/****************************************************************************
**
** Definition of QDir class.
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

#ifndef QDIR_H
#define QDIR_H

#ifndef QT_H
#include "qglobal.h"
#include "qlist.h"
#include "qfileinfo.h"
#include "qstringlist.h"
#endif // QT_H


#ifndef QT_NO_DIR
typedef QList<QFileInfo> QFileInfoList;

class Q_CORE_EXPORT QDir
{
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

                      DefaultFilter = -1 };

    enum SortSpec   { Name        = 0x00,
                      Time        = 0x01,
                      Size        = 0x02,
                      Unsorted    = 0x03,
                      SortByMask  = 0x03,

                      DirsFirst   = 0x04,
                      Reversed    = 0x08,
                      IgnoreCase  = 0x10,
                      DefaultSort = -1 };

    QDir();
    QDir(const QString &path, const QStringList &nameFilters = QStringList(), 
         int sortSpec = Name | IgnoreCase, int filterSpec = All);
    QDir(const QDir &);

    ~QDir();

    QDir &operator=(const QDir &);
    QDir &operator=(const QString &path);

    void setPath(const QString &path);
    QString path() const;
    QString absPath() const;
    QString canonicalPath() const;

    QString dirName() const;
    QString filePath(const QString &fileName,
                     bool acceptAbsPath = true) const;
    QString absFilePath(const QString &fileName,
                        bool acceptAbsPath = true) const;

    static QString convertSeparators(const QString &pathName);

    bool cd(const QString &dirName, bool acceptAbsPath = true);
    bool cdUp();

#ifdef QT_COMPAT
    inline QT_COMPAT QString nameFilter() const { return nameFilts.join(" "); }
#ifndef QT_NO_REGEXP
    inline QT_COMPAT void setNameFilter(const QString &nameFilter) { setNameFilters(nameFilter); }
#endif
#endif
    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);

    FilterSpec filter() const;
    void setFilter(int filterSpec);
    SortSpec sorting() const;
    void setSorting(int sortSpec);

    bool        matchAllDirs() const;
    void setMatchAllDirs(bool);

    uint count() const;
    QString        operator[](int) const;

    QStringList entryList(int filterSpec = DefaultFilter,
                                   int sortSpec   = DefaultSort ) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QStringList entryList(const QString &nameFilter,
                                           int filterSpec = DefaultFilter,
                                           int sortSpec = DefaultSort) const
    {
        QChar sep(';');
        int i = nameFilter.indexOf(sep, 0);
        if (i == -1 && nameFilter.indexOf(' ', 0) != -1)
            sep = QChar(' ');
        return entryList(nameFilter.split(sep), filterSpec, sortSpec);
    }
#endif
    QStringList entryList(const QStringList &nameFilters, int filterSpec = DefaultFilter, 
                          int sortSpec = DefaultSort) const;

    QFileInfoList entryInfoList(int filterSpec = DefaultFilter,
                                                int sortSpec = DefaultSort) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QFileInfoList entryInfoList(const QString &nameFilter,
                                                 int filterSpec = DefaultFilter,
                                                 int sortSpec = DefaultSort) const
    {     
        QChar sep(';');
        int i = nameFilter.indexOf(sep, 0);
        if (i == -1 && nameFilter.indexOf(' ', 0) != -1)
            sep = QChar(' ');
        return entryInfoList(nameFilter.split(sep), filterSpec, sortSpec);
    }
#endif
    QFileInfoList entryInfoList(const QStringList &nameFilters, int filterSpec = DefaultFilter,
                                int sortSpec = DefaultSort) const;

    static QFileInfoList drives();

    bool mkdir(const QString &dirName,
                        bool acceptAbsPath = true) const;
    bool rmdir(const QString &dirName,
                        bool acceptAbsPath = true) const;

    bool isReadable() const;
    bool exists()   const;
    bool isRoot()   const;

    bool isRelative() const;
    void convertToAbs();

    bool operator==(const QDir &) const;
    bool operator!=(const QDir &) const;

    bool remove(const QString &fileName,
                         bool acceptAbsPath = true);
    bool rename(const QString &name, const QString &newName,
                         bool acceptAbsPaths = true );
    bool exists(const QString &name,
                         bool acceptAbsPath = true) const;

    static char separator();

    static bool setCurrent(const QString &path);
    static QDir current();
    static QDir home();
    static QDir root();
    static QString currentDirPath();
    static QString homeDirPath();
    static QString rootDirPath();

#ifndef QT_NO_REGEXP
    static bool match(const QStringList &filters, const QString &fileName);
    static bool match(const QString &filter, const QString &fileName);
#endif
    static QString cleanDirPath(const QString &dirPath);
    static bool isRelativePath(const QString &path);
    void refresh() const;

private:
#ifdef Q_OS_MAC
    typedef struct FSSpec FSSpec;
    static FSSpec *make_spec(const QString &);
#endif
    void init();
    void readDirEntries(const QStringList &nameFilters,
                        int FilterSpec, int SortSpec) const;

    static void slashify(QString &);

    QString dPath;
    mutable QStringList fList;
    mutable QFileInfoList fiList;
    mutable QStringList nameFilts;
    mutable FilterSpec filtS;
    mutable SortSpec sortS;
    mutable uint dirty : 1;
    uint allDirs : 1;
};

inline QStringList QDir::nameFilters() const
{
    return nameFilts;
}

inline QString QDir::path() const
{
    return dPath;
}

inline QDir::FilterSpec QDir::filter() const
{
    return filtS;
}

inline QDir::SortSpec QDir::sorting() const
{
    return sortS;
}

inline bool QDir::matchAllDirs() const
{
    return allDirs;
}

inline bool QDir::operator!=(const QDir &d) const
{
    return !(*this == d);
}


struct QDirSortItem {
    QString filename_cache;
    QFileInfo item;
};

#endif // QT_NO_DIR
#endif // QDIR_H
