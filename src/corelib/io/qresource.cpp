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

#include <qset.h>
#include <qhash.h>
#include <qlocale.h>
#include <qglobal.h>
#include <qdatetime.h>
#include <qbytearray.h>
#include <qstringlist.h>
#include <qvector.h>
#include <private/qfileengine_p.h>

#include "qresource_p.h"

//resource glue
class QResource
{
    enum Flags
    {
        Compressed = 0x01,
        Directory = 0x02
    };
    const uchar *tree, *names, *payloads;
    int findNode(const QString &path) const;
    inline int findOffset(int node) const { return node * 14; } //sizeof each tree element
    inline int hash(int offset) const;
    inline QString name(int offset) const;
public:
    inline QResource(): tree(0), names(0), payloads(0) {}
    inline QResource(const uchar *t, const uchar *n, const uchar *d)
        : tree(t), names(n), payloads(d) {}
    bool isContainer(const QString &path) const;
    bool exists(const QString &path) const;
    QByteArray data(const QString &path) const;
    QStringList children(const QString &path) const;
};

Q_DECLARE_TYPEINFO(QResource, Q_MOVABLE_TYPE);

inline int QResource::hash(int node) const
{
    if(!node) //root
        return 0;
    const int offset = findOffset(node);
    int name_offset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                      (tree[offset+2] << 8) + (tree[offset+3] << 0);
    name_offset += 2; //jump past name length
    return (names[name_offset+0] << 24) + (names[name_offset+1] << 16) +
           (names[name_offset+2] << 8) + (names[name_offset+3] << 0);
}
inline QString QResource::name(int node) const
{
    if(!node) // root
        return QString();
    const int offset = findOffset(node);

    QString ret;
    int name_offset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                      (tree[offset+2] << 8) + (tree[offset+3] << 0);
    const short name_length = (names[name_offset+0] << 8) +
                              (names[name_offset+1] << 0);
    name_offset += 2;
    name_offset += 4; //jump past hash
    for(int i = 0; i < name_length*2; i+=2)
        ret += QChar(names[name_offset+i+1], names[name_offset+i]);
    return ret;
}
int QResource::findNode(const QString &path) const
{
    if(path == QLatin1String("/"))
        return 0;

    //the root node is always first
    int child_count = (tree[6] << 24) + (tree[7] << 16) +
                      (tree[8] << 8) + (tree[9] << 0);
    int child       = (tree[10] << 24) + (tree[11] << 16) +
                      (tree[12] << 8) + (tree[13] << 0);

    //now iterate up the tree
    int node = -1;
    QLocale locale;
    QStringList segments = path.split('/', QString::SkipEmptyParts);
    for(int i = 0; child_count && i < segments.size(); ++i) {
        const QString &segment = segments[i];
        const int h = qHash(segment);

        //do the binary search for the hash
        int l = 0, r = child_count-1;
        int sub_node = (l+r+1)/2;
        while(r != l) {
            const int sub_node_hash = hash(child+sub_node);
            if(h == sub_node_hash)
                break;
            else if(h < sub_node_hash)
                r = sub_node - 1;
            else
                l = sub_node;
            sub_node = (l + r + 1) / 2;
        }
        sub_node += child;

        //now do the "harder" compares
        bool found = false;
        if(hash(sub_node) == h) {
            while(sub_node > child && hash(sub_node-1) == h) //backup for collisions
                --sub_node;
            for(; sub_node < child+child_count && hash(sub_node) == h; ++sub_node) { //here we go...
                if(name(sub_node) == segment) {
                    found = true;
                    int offset = findOffset(sub_node) + 4; //jump past name

                    const short flags = (tree[offset+0] << 8) +
                                        (tree[offset+1] << 0);
                    offset += 2;

                    if(i == segments.size()-1) {
                        if(!(flags & Directory)) {
                            const short country = (tree[offset+0] << 8) +
                                                  (tree[offset+1] << 0);
                            offset += 2;

                            const short language = (tree[offset+0] << 8) +
                                                   (tree[offset+1] << 0);
                            offset += 2;

                            if(country == locale.country() && language == locale.language())
                                return sub_node;
                            else if((!country && language == locale.language()) ||
                                    (!country && !language && node == -1))
                                node = sub_node;
                            continue;
                        } else {
                            return sub_node;
                        }
                    }

                    if(!(flags & Directory))
                        return -1;

                    child_count = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                                  (tree[offset+2] << 8) + (tree[offset+3] << 0);
                    offset += 4;
                    child = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                            (tree[offset+2] << 8) + (tree[offset+3] << 0);
                    break;
                }
            }
        }
        if(!found)
            break;
    }
    return node;
}
bool QResource::isContainer(const QString &path) const
{
    int node = findNode(path);
    if(node == -1)
        return false;
    const int offset = findOffset(node) + 4; //jump past name
    const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
    return flags & Directory;
}
bool QResource::exists(const QString &path) const
{
    return findNode(path) != -1;
}
QByteArray QResource::data(const QString &path) const
{
    const int node = findNode(path);
    if(node == -1)
        return QByteArray();
    int offset = findOffset(node) + 4; //jump past name

    const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
    offset += 2;

    offset += 4; //jump past locale

    QByteArray ret;
    if(!(flags & Directory)) {
        const int data_offset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                                (tree[offset+2] << 8) + (tree[offset+3] << 0);
        const uint data_length = (payloads[data_offset+0] << 24) + (payloads[data_offset+1] << 16) +
                                 (payloads[data_offset+2] << 8) + (payloads[data_offset+3] << 0);
        const uchar *data = payloads+data_offset+4;
        if(flags & Compressed)
            ret = qUncompress(data, data_length);
        else
            ret = QByteArray((char*)data, data_length);
    }
    return ret;
}
QStringList QResource::children(const QString &path) const
{
    int node = findNode(path);
    if(node == -1)
        return QStringList();
    int offset = findOffset(node) + 4; //jump past name

    const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
    offset += 2;

    QStringList ret;
    if(flags & Directory) {
        const int child_count = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                                (tree[offset+2] << 8) + (tree[offset+3] << 0);
        offset += 4;
        const int child_off = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                              (tree[offset+2] << 8) + (tree[offset+3] << 0);
        for(int i = child_off; i < child_off+child_count; ++i)
            ret << name(i);
    }
    return ret;
}

Q_GLOBAL_STATIC(QStringList, qt_resource_search_paths)
bool qt_resource_add_search_path(const QString &path)
{
    if(path[0] != QLatin1Char('/')) {
        qWarning("QDir::addResourceSearchPath: Search paths must be absolute (start with /) [%s]",
                 path.toLocal8Bit().data());
        return false;
    }
    qt_resource_search_paths()->prepend(path);
    return true;
}

typedef QVector<QResource> ResourceList;
Q_GLOBAL_STATIC(ResourceList, resourceList)

class QResourceInfo
{
    QString file, searchFile;
    ResourceList related;
    uint container : 1;

    mutable uint hasData : 1;
    mutable QByteArray mData;

    mutable uint hasChildren : 1;
    mutable QStringList mChildren;

    inline void clear() {
        searchFile.clear();
        file.clear();
        hasData = hasChildren = 0;
        container = 0;
        related.clear();
    }
    bool loadResource(const QString &);
public:
    QResourceInfo() { clear(); }
    QResourceInfo(const QString &f) { setFileName(f); }

    void setFileName(const QString &f);
    QString fileName() const { return file; }
    QString searchFileName() const { return searchFile; }

    bool exists() const { return !related.isEmpty(); }
    bool isContainer() const { return container; }
    QByteArray data() const;
    QStringList children() const;
};
bool
QResourceInfo::loadResource(const QString &path)
{
    const ResourceList *list = resourceList();
    for(int i = 0; i < list->size(); ++i) {
        QResource res = list->at(i);
        if(res.exists(path)) {
            if(related.isEmpty())
                container = res.isContainer(path);
            else if(res.isContainer(path) != container)
                qWarning("Resource [%s] has both data and children!", file.toLatin1().constData());
            related.append(res);
        }
    }
    return !related.isEmpty();
}
void
QResourceInfo::setFileName(const QString &f)
{
    if(file == f)
        return;
    clear();
    file = f;
    if(file == QLatin1String(":"))
        file += "/";
    searchFile = file;

    QString path = file;
    if(path.startsWith(QLatin1String(":")))
        path = path.mid(1);
    if(path[0] == QLatin1Char('/')) {
        loadResource(path);
        return;
    } else {
        QStringList searchPaths = *qt_resource_search_paths();
        searchPaths << QLatin1String("");
        for(int i = 0; i < searchPaths.size(); ++i) {
            const QString searchPath(searchPaths.at(i) + "/" + path);
            if(loadResource(searchPath)) {
                searchFile = ":" + searchPath;
                break;
            }
        }
    }
}
QByteArray QResourceInfo::data() const
{
    if(container || related.isEmpty())
        return QByteArray();

    if(!hasData) {
        hasData = true;
        QString path = searchFile;
        if(path.startsWith(":"))
            path = path.mid(1);
        mData = related.at(0).data(path);
    }
    return mData;
}

QStringList QResourceInfo::children() const
{
    if(!container || related.isEmpty())
        return QStringList();

    if(!hasChildren) {
        hasChildren = true;
        QString path = searchFile;
        if(path.startsWith(":"))
            path = path.mid(1);
        QSet<QString> kids;
        for(int i = 0; i < related.size(); ++i) {
            QStringList related_children = related.at(i).children(path);
            for(int kid = 0; kid < related_children.size(); ++kid) {
                QString k = related_children.at(kid);
                if(!kids.contains(k)) {
                    mChildren += k;
                    kids.insert(k);
                }
            }
        }
    }
    return mChildren;
}

Q_CORE_EXPORT bool qRegisterResourceData(int version, const unsigned char *tree,
                                         const unsigned char *name, const unsigned char *data)
{
    if(version == 0x01) {
        resourceList()->append(QResource(tree, name, data));
        return true;
    }
    return false;
}

//file type handler
class QResourceFileEngineHandler : public QFileEngineHandler
{
public:
    QResourceFileEngineHandler() { }
    ~QResourceFileEngineHandler() { }
    QFileEngine *createFileEngine(const QString &path);
};
QFileEngine *QResourceFileEngineHandler::createFileEngine(const QString &path)
{
    if (path.size() > 0 && path.startsWith(QLatin1String(":")))
        return new QResourceFileEngine(path);
    return 0;
}

//resource engine
class QResourceFileEnginePrivate : public QFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceFileEngine)
private:
    qint64 offset;
    QResourceInfo resource;
protected:
    QResourceFileEnginePrivate() : offset(0) { }
};

bool QResourceFileEngine::mkdir(const QString &, bool) const
{
    return false;
}

bool QResourceFileEngine::rmdir(const QString &, bool) const
{
    return false;
}

bool QResourceFileEngine::setSize(qint64)
{
    return false;
}

QStringList QResourceFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    Q_D(const QResourceFileEngine);

    const bool doDirs     = (filters & QDir::Dirs) != 0;
    const bool doFiles    = (filters & QDir::Files) != 0;
    const bool doReadable = (filters & QDir::Readable) != 0;

    QStringList ret;
    if((!doDirs && !doFiles) || ((filters & QDir::PermissionMask) && !doReadable))
        return ret;
    if(!d->resource.exists() || !d->resource.isContainer())
        return ret; // cannot read the "directory"

    QStringList entries = d->resource.children();
    for(int i = 0; i < entries.size(); i++) {
        QResourceInfo entry(d->resource.fileName() + "/" + entries[i]);
#ifndef QT_NO_REGEXP
        if(!(filters & QDir::AllDirs && entry.isContainer())) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filterNames.begin(); sit != filterNames.end(); ++sit) {
                QRegExp rx(*sit,
                           (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                           QRegExp::Wildcard);
                if (rx.exactMatch(entries[i])) {
                    matched = true;
                    break;
                }
            }
            if(!matched)
                continue;
        }
#endif
        if  ((doDirs && entry.isContainer()) ||
             (doFiles && !entry.isContainer()))
            ret.append(entries[i]);
    }
    return ret;
}

bool QResourceFileEngine::caseSensitive() const
{
    return true;
}

QResourceFileEngine::QResourceFileEngine(const QString &file) :
    QFileEngine(*new QResourceFileEnginePrivate)
{
    Q_D(QResourceFileEngine);
    d->resource.setFileName(file);
}

QResourceFileEngine::~QResourceFileEngine()
{
}

void QResourceFileEngine::setFileName(const QString &file)
{
    Q_D(QResourceFileEngine);
    d->resource.setFileName(file);
}

bool QResourceFileEngine::open(int flags)
{
    Q_D(QResourceFileEngine);
    if (d->resource.fileName().isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        return false;
    }
    if(flags & QIODevice::WriteOnly)
        return false;
    if(!d->resource.exists())
       return false;
    return true;
}

bool QResourceFileEngine::close()
{
    Q_D(QResourceFileEngine);
    d->offset = 0;
    return true;
}

void QResourceFileEngine::flush()
{

}

qint64 QResourceFileEngine::read(char *data, qint64 len)
{
    Q_D(QResourceFileEngine);
    if(len > d->resource.data().size()-d->offset)
        len = d->resource.data().size()-d->offset;
    if(len <= 0)
        return 0;
    memcpy(data, d->resource.data().constData()+d->offset, len);
    d->offset += len;
    return len;
}

qint64 QResourceFileEngine::write(const char *, qint64)
{
    return -1;
}

int QResourceFileEngine::ungetch(int)
{
    return -1;
}

bool QResourceFileEngine::remove()
{
    return false;
}

bool QResourceFileEngine::copy(const QString &)
{
    return false;
}

bool QResourceFileEngine::rename(const QString &)
{
    return false;
}

bool QResourceFileEngine::link(const QString &)
{
    return false;
}

qint64 QResourceFileEngine::size() const
{
    Q_D(const QResourceFileEngine);
    if(!d->resource.exists())
        return 0;
    return d->resource.data().size();
}

qint64 QResourceFileEngine::at() const
{
    Q_D(const QResourceFileEngine);
    return d->offset;
}

bool QResourceFileEngine::atEnd() const
{
    Q_D(const QResourceFileEngine);
    if(!d->resource.exists())
        return true;
    return d->offset == d->resource.data().size();
}

bool QResourceFileEngine::seek(qint64 pos)
{
    Q_D(QResourceFileEngine);
    if(!d->resource.exists())
        return false;

    if(d->offset > d->resource.data().size())
        return false;
    d->offset = pos;
    return true;
}

bool QResourceFileEngine::isSequential() const
{
    return false;
}

QFileEngine::FileFlags QResourceFileEngine::fileFlags(QFileEngine::FileFlags type) const
{
    Q_D(const QResourceFileEngine);
    QFileEngine::FileFlags ret = 0;
    if(!d->resource.exists())
        return ret;
    if(type & PermsMask)
        ret |= QFileEngine::FileFlags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm);
    if(type & TypesMask) {
        if(d->resource.isContainer())
            ret |= DirectoryType;
        else
            ret |= FileType;
    }
    if(type & FlagsMask) {
        ret |= ExistsFlag;
        if(d->resource.fileName() == QLatin1String(":/"))
            ret |= RootFlag;
    }
    return ret;
}

bool QResourceFileEngine::chmod(uint)
{
    return false;
}

QString QResourceFileEngine::fileName(FileName file) const
{
    Q_D(const QResourceFileEngine);
    if(file == BaseName) {
	int slash = d->resource.fileName().lastIndexOf(QLatin1Char('/'));
	if (slash == -1)
	    return d->resource.fileName();
	return d->resource.fileName().mid(slash + 1);
    } else if(file == PathName || file == AbsolutePathName) {
	const int slash = d->resource.fileName().lastIndexOf(QLatin1Char('/'));
	if (slash != -1)
	    return d->resource.fileName().left(slash);
    } else if(file == CanonicalName || file == CanonicalPathName) {
        const QString canonicalPath = d->resource.searchFileName();
        if(file == CanonicalPathName) {
            const int slash = canonicalPath.lastIndexOf(QLatin1Char('/'));
            if (slash != -1)
                return canonicalPath.left(slash);
        }
        return canonicalPath;
    }
    return d->resource.fileName();
}

bool QResourceFileEngine::isRelativePath() const
{
    return false;
}

uint QResourceFileEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString QResourceFileEngine::owner(FileOwner) const
{
    return QString();
}

QDateTime QResourceFileEngine::fileTime(FileTime) const
{
    return QDateTime();
}

QFileEngine::Type QResourceFileEngine::type() const
{
    return QFileEngine::Resource;
}

//Initialization and cleanup
Q_GLOBAL_STATIC(QResourceFileEngineHandler, resource_file_handler)

static int qt_force_resource_init() { resource_file_handler(); return 1; }
Q_CORE_EXPORT void qInitResourceIO() { resource_file_handler(); }
static int qt_forced_resource_init = qt_force_resource_init();
Q_CONSTRUCTOR_FUNCTION(qt_force_resource_init)

