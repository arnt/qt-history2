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
#ifndef __QDIR_H__
#define __QDIR_H__

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

    QDir();
#ifdef QT_COMPAT
    QDir(const QString &path, const QString &nameFilter,
         int sortSpec = Name | IgnoreCase, int filterSpec = All);
#endif
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

    QStringList entryList(int filterSpec = DefaultFilter,
                          int sortSpec = DefaultSort) const;
#ifdef QT_COMPAT
    QT_COMPAT QStringList entryList(const QString &nameFilter, int filterSpec = DefaultFilter,
                                    int sortSpec = DefaultSort) const;
    QT_COMPAT QFileInfoList entryInfoList(const QString &nameFilter, int filterSpec = DefaultFilter,
                                          int sortSpec = DefaultSort) const;
    QT_COMPAT QString nameFilter() const;
    QT_COMPAT void setNameFilter(const QString &nameFilter);
#endif
    QStringList entryList(const QStringList &nameFilters, int filterSpec = DefaultFilter,
                          int sortSpec = DefaultSort) const;

    QFileInfoList entryInfoList(int filterSpec = DefaultFilter,
                                int sortSpec = DefaultSort) const;
    QFileInfoList entryInfoList(const QStringList &nameFilters, int filterSpec = DefaultFilter,
                                int sortSpec = DefaultSort) const;

    bool mkdir(const QString &dirName,
               bool acceptAbsPath = true) const;
    bool rmdir(const QString &dirName,
               bool acceptAbsPath = true) const;

    bool isReadable() const;
    bool exists() const;
    bool isRoot() const;

    static bool isRelativePath(const QString &path);
    bool isRelative() const;
    void convertToAbs();

    bool operator==(const QDir &dir) const;
    inline bool operator!=(const QDir &dir) const {  return !operator==(dir); }

    bool remove(const QString &fileName, bool acceptAbsPath = true);
    bool rename(const QString &name, const QString &newName, bool acceptAbsPaths = true);
    bool exists(const QString &name, bool acceptAbsPath = true) const;

    static QFileInfoList drives();

    static QChar separator();

    static bool setCurrent(const QString &path);
    static inline QDir current() { return QDir(currentDirPath()); }
    static QString currentDirPath();

    static inline QDir home() { return QDir(homeDirPath()); }
    static QString homeDirPath();
    static inline QDir root() { return QDir(rootDirPath()); }
    static QString rootDirPath();

#ifndef QT_NO_REGEXP
    bool match(const QStringList &filters, const QString &fileName);
    bool match(const QString &filter, const QString &fileName);
#endif
    static QString cleanDirPath(const QString &name);
    void refresh() const;
};

#endif /* __QDIR_H__ */
