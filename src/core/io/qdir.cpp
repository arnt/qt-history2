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
    inline void setPath(const QString &p) 
    {
        detach();
        data->path = p;
        initDirEngine(p);
        data->dirEngine->setPath(p);
        data->clear();
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
            si[i].item = QFileInfo(l.at(i));
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
        QStringList l = data->dirEngine->entryInfoList(data->filterSpec, data->nameFilters);
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
    return new QFSDirEngine(path);
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

QDir::QDir() : d_ptr(new QDirPrivate(this))
{
    d->setPath(QString::fromLatin1("."));
    d->data->nameFilters = QStringList(QString::fromLatin1("*"));
    d->data->filterSpec = All;
    d->data->sortSpec = SortSpec(Name | IgnoreCase);
}

QDir::QDir(const QString &path, const QString &nameFilter,
             int sortSpec, int filterSpec)  : d_ptr(new QDirPrivate(this))
{
    d->setPath(path.isEmpty() ? QString::fromLatin1(".") : path);
    d->data->nameFilters = qt_makeFilterStringList(nameFilter);
    if (d->data->nameFilters.isEmpty())
        d->data->nameFilters = QString::fromLatin1("*");
    d->data->sortSpec = sortSpec;
    d->data->filterSpec = filterSpec;
}

QDir::QDir(const QString &path, const QStringList &nameFilters, 
             int sortSpec, int filterSpec) : d_ptr(new QDirPrivate(this))
{
    d->setPath(path.isEmpty() ? QString::fromLatin1(".") : path);
    d->data->nameFilters = nameFilters;
    if (d->data->nameFilters.isEmpty())
        d->data->nameFilters = QString::fromLatin1("*");
    d->data->sortSpec = sortSpec;
    d->data->filterSpec = filterSpec;
}

QDir::QDir(const QDir &dir)  : d_ptr(new QDirPrivate(this, &dir))
{
}

QDir::~QDir()
{
    delete d_ptr;
    d_ptr = 0;
}

void
QDir::setPath(const QString &path)
{
    d->setPath(path);
}

QString
QDir::path() const
{
    return d->data->path;
}

QString
QDir::absPath() const
{
    if (QDir::isRelativePath(d->data->path)) 
        return absFilePath("");
    return cleanDirPath(d->data->path);
}


QString
QDir::canonicalPath() const
{
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    return d->data->fi.canonicalPath();
}

QString
QDir::dirName() const
{
    int pos = d->data->path.lastIndexOf('/');
    if (pos == -1)
        return d->data->path;
    return d->data->path.mid(pos + 1);
}

QString
QDir::filePath(const QString &fileName, bool acceptAbsPath) const
{
    if (acceptAbsPath && !isRelativePath(fileName))
        return QString(fileName);

    QString ret = d->data->path;
    if (ret.isEmpty() 
        || (ret[(int)ret.length()-1] != '/' && fileName.size() && fileName[0] != '/'))
        ret += '/';
    ret += fileName;
    return ret;
}

QString
QDir::absFilePath(const QString &fileName, bool acceptAbsPath) const
{
    if (acceptAbsPath && !isRelativePath(fileName))
        return fileName;
    if(!d->data->dirEngine)
        return fileName;

    QString ret;
    if (isRelativePath(d->data->path)) //get pwd
        ret = QFSDirEngine::currentDirPath(fileName);
    if (ret.right(1) != QString::fromLatin1("/"))
        ret += '/';
    ret += d->data->path;
    if (!fileName.isEmpty()) {
        if (ret.right(1) != QString::fromLatin1("/"))
            ret += '/';
        ret += fileName;
    }
    return cleanDirPath(ret);
}

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
    if (!exists()) 
        return false;

    d->setPath(newPath);
    refresh();
    return true;
}

bool
QDir::cdUp()
{
    return cd(QString::fromLatin1(".."));
}

QStringList
QDir::nameFilters() const
{
    return d->data->nameFilters;
}

void
QDir::setNameFilters(const QStringList &nameFilters)
{
    d->detach();
    d->data->nameFilters = nameFilters;
}

QDir::FilterSpec
QDir::filter() const
{
    return (FilterSpec)d->data->filterSpec;
}

void
QDir::setFilter(int filterSpec)
{
    d->detach();
    d->data->filterSpec = filterSpec;
}

QDir::SortSpec
QDir::sorting() const
{
    return (SortSpec)d->data->sortSpec;
}

void
QDir::setSorting(int sortSpec)
{
    d->detach();
    d->data->sortSpec = sortSpec;
}

uint
QDir::count() const
{
    d->updateFileLists();
    return d->data->files.count();
}

QString 
QDir::operator[](int pos) const
{
    d->updateFileLists();
    return d->data->files[pos];
}

QStringList
QDir::entryList(int filterSpec, int sortSpec) const
{
    return entryList(d->data->nameFilters, filterSpec, sortSpec);
}

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
    QStringList l = d->data->dirEngine->entryInfoList(d->data->filterSpec, nameFilters), ret;
    d->sortFileList(sortSpec, l, &ret, 0); 
    return ret;
}

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
    QStringList l = d->data->dirEngine->entryInfoList(d->data->filterSpec, nameFilters);
    d->sortFileList(sortSpec, l, 0, &ret); 
    return ret;
}

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

bool
QDir::isReadable() const
{
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    return d->data->fi.isReadable();
}

bool
QDir::exists() const
{
    //we cannot use the data->fi because the old implementation
    //didn't cache this (but in other places it does!!) [SOURCE COMPAT]
    QFileInfo fi(d->data->path);
    return fi.exists() && fi.isDir();
}

bool
QDir::isRoot() const
{
    if(!d->data->dirEngine)
        return true;    
    return d->data->dirEngine->isRoot();
}

bool
QDir::isRelative() const
{
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    return d->data->fi.isRelative();
}

void
QDir::convertToAbs()
{
    d->detach();
    if(d->data->fileInfoDirty)
        d->data->fi = QFileInfo(d->data->path);
    d->data->fi.convertToAbs();
    d->data->path = d->data->fi.filePath();
    d->data->dirEngine->setPath(d->data->path);
}

bool QDir::operator==(const QDir &dir) const
{
    return dir.d->data == d->data 
           || (d->data->path == dir.d->data->path 
               && d->data->filterSpec == dir.d->data->filterSpec 
               && d->data->sortSpec == dir.d->data->sortSpec 
               && d->data->nameFilters == dir.d->data->nameFilters);
}

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

QFileInfoList
QDir::drives()
{
    return QFSDirEngine::drives();
}

char
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

bool
QDir::setCurrent(const QString &path)
{
    return QFSDirEngine::setCurrentDirPath(path);
}

QString
QDir::currentDirPath()
{
    return QFSDirEngine::currentDirPath();    
}

QString
QDir::homeDirPath()
{
    return QFSDirEngine::homeDirPath();
}

QString
QDir::rootDirPath()
{
    return QFSDirEngine::rootDirPath();
}

#ifndef QT_NO_REGEXP
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

bool
QDir::match(const QString &filter, const QString &fileName)
{
    return match(QStringList(filter), fileName);
}
#endif

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
            if(i <= len - 1 && p[i+1] == '.') {
                int dotcount = 1;
                if(i <= len - 2 && p[i+2] == '.') 
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
                        if(last == -1) {
                            for(int i2 = iwrite-1; i2 >= 0; i2--) {
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
    delete out;
    return ret;
}

bool
QDir::isRelativePath(const QString &path)
{
    QFileInfo fi(path);
    return fi.isRelative();
}

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
