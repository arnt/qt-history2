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

#include "qresource.h"
#include "qresource_p.h"
#include "qset.h"
#include "qhash.h"
#include "qmutex.h"
#include "qdebug.h"
#include "qlocale.h"
#include "qglobal.h"
#include "qvector.h"
#include "qdatetime.h"
#include "qbytearray.h"
#include "qstringlist.h"
#include <qshareddata.h>
#include <qplatformdefs.h>
#include "private/qabstractfileengine_p.h"

//resource glue
class QResourceRoot
{
    enum Flags
    {
        Compressed = 0x01,
        Directory = 0x02
    };
    const uchar *tree, *names, *payloads;
    inline int findOffset(int node) const { return node * 14; } //sizeof each tree element
    int hash(int node) const;
    QString name(int node) const;
    short flags(int node) const;
public:
    mutable QAtomic ref;

    inline QResourceRoot(): tree(0), names(0), payloads(0) {}
    inline QResourceRoot(const uchar *t, const uchar *n, const uchar *d) { setSource(t, n, d); }
    virtual ~QResourceRoot() { }
    int findNode(const QString &path, const QLocale &locale=QLocale()) const;
    inline bool isContainer(int node) const { return flags(node) & Directory; }
    inline bool isCompressed(int node) const { return flags(node) & Compressed; }
    const uchar *data(int node, qint64 *size) const;
    QStringList children(int node) const;
    virtual QString mappingRoot() const { return QString(); }
    bool mappingRootSubdir(const QString &path, QString *match=0) const;
    inline bool operator==(const QResourceRoot &other) const
    { return tree == other.tree && names == other.names && payloads == other.payloads; }
    inline bool operator!=(const QResourceRoot &other) const
    { return !operator==(other); }

protected:
    inline void setSource(const uchar *t, const uchar *n, const uchar *d) {
        tree = t;
        names = n;
        payloads = d;
    }
};

Q_DECLARE_TYPEINFO(QResourceRoot, Q_MOVABLE_TYPE);

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, resourceMutex, (QMutex::Recursive))

typedef QList<QResourceRoot*> ResourceList;
Q_GLOBAL_STATIC(ResourceList, resourceList)

Q_GLOBAL_STATIC(QStringList, resourceSearchPaths)

/*!
    \class QResource
    \brief The QResource class provides an interface for reading directly from resources.

    \ingroup io
    \mainclass
    \reentrant

    QResource is an object that represents a set of data (and possibly
    children) relating to a single resource entity. QResource gives direct
    access to the bytes in their raw format. In this way direct access
    allows reading data without buffer copying or indirection. Indirection
    is often useful when interacting with the resource entity as if it is a
    file, this can be achieved with QFile. The data and children behind a
    QResource are normally compiled into an application/library, but it is
    also possible to load a resource at runtime. When loaded at run time
    the resource file will be loaded as one big set of data and then given
    out in pieces via references into the resource tree.

    A QResource can either be loaded with an absolute path (either treated
    as a file system rooted with /, or in resource notation rooted with
    :). A relative resource can also be opened which will be found through
    the searchPaths().

    A QResource that is representing a file will have data backing it, this
    data can possibly be compressed, in which case qUncompress() must be
    used to access the real data; this happens implicitly when accessed
    through a QFile. A QResource that is representing a directory will have
    only children and no data.

    \sa {The Qt Resource System}, QFile, QDir, QFileInfo
*/

class QResourcePrivate {
public:
    inline QResourcePrivate(QResource *_q) : q_ptr(_q) { clear(); }
    inline ~QResourcePrivate() { clear(); }

    void ensureInitialized() const;
    void ensureChildren() const;

    bool load(const QString &file);
    void clear();

    QLocale locale;
    QString fileName, absoluteFilePath;
    QList<QResourceRoot*> related;
    uint container : 1;
    mutable uint compressed : 1;
    mutable qint64 size;
    mutable const uchar *data;
    mutable QStringList children;

    QResource *q_ptr;
    Q_DECLARE_PUBLIC(QResource)
};

void
QResourcePrivate::clear()
{
    absoluteFilePath.clear();
    compressed = 0;
    data = 0;
    size = 0;
    children.clear();
    container = 0;
    for(int i = 0; i < related.size(); ++i) {
        QResourceRoot *root = related.at(i);
        if(!root->ref.deref())
            delete root;
    }
    related.clear();
}

bool
QResourcePrivate::load(const QString &file)
{
    related.clear();
    QMutexLocker lock(resourceMutex());
    const ResourceList *list = resourceList();
    for(int i = 0; i < list->size(); ++i) {
        QResourceRoot *res = list->at(i);
        const int node = res->findNode(file);
        if(node != -1) {
            if(related.isEmpty()) {
                container = res->isContainer(node);
                if(!container) {
                    data = res->data(node, &size);
                    compressed = res->isCompressed(node);
                } else {
                    data = 0;
                    size = 0;
                    compressed = 0;
                }
            } else if(res->isContainer(node) != container) {
                qWarning("QResourceInfo: Resource [%s] has both data and children!", file.toLatin1().constData());
            }
            res->ref.ref();
            related.append(res);
        } else if(res->mappingRootSubdir(file)) {
            container = true;
            data = 0;
            size = 0;
            compressed = 0;
            res->ref.ref();
            related.append(res);
        }
    }
    return !related.isEmpty();
}

void
QResourcePrivate::ensureInitialized() const
{
    if(!related.isEmpty())
        return;
    QResourcePrivate *that = const_cast<QResourcePrivate *>(this);
    if(fileName == QLatin1String(":"))
        that->fileName += QLatin1Char('/');
    that->absoluteFilePath = fileName;
    if(!that->absoluteFilePath.startsWith(QLatin1Char(':')))
        that->absoluteFilePath.prepend(QLatin1Char(':'));

    QString path = fileName;
    if(path.startsWith(QLatin1Char(':')))
        path = path.mid(1);

    bool found = false;
    if(path.startsWith(QLatin1Char('/'))) {
        found = that->load(path);
    } else {
        QMutexLocker lock(resourceMutex());
        QStringList searchPaths = *resourceSearchPaths();
        searchPaths << QLatin1String("");
        for(int i = 0; i < searchPaths.size(); ++i) {
            const QString searchPath(searchPaths.at(i) + QLatin1Char('/') + path);
            if(that->load(searchPath)) {
                found = true;
                that->absoluteFilePath = QLatin1Char(':') + searchPath;
                break;
            }
        }
    }
}

void
QResourcePrivate::ensureChildren() const
{
    ensureInitialized();
    if(!children.isEmpty() || !container || related.isEmpty())
        return;

    QString path = absoluteFilePath, k;
    if(path.startsWith(QLatin1Char(':')))
        path = path.mid(1);
    QSet<QString> kids;
    for(int i = 0; i < related.size(); ++i) {
        QResourceRoot *res = related.at(i);
        if(res->mappingRootSubdir(path, &k) && !k.isEmpty()) {
            if(!kids.contains(k)) {
                children += k;
                kids.insert(k);
            }
        } else {
            const int node = res->findNode(path);
            if(node != -1) {
                QStringList related_children = res->children(node);
                for(int kid = 0; kid < related_children.size(); ++kid) {
                    k = related_children.at(kid);
                    if(!kids.contains(k)) {
                        children += k;
                        kids.insert(k);
                    }
                }
            }
        }
    }
}

/*!
    Constructs a QResource pointing to \a file. \a locale is used to
    load a specific localization of a resource data.

    \sa QFileInfo, searchPaths(), setFileName(), setLocale()
*/

QResource::QResource(const QString &file, const QLocale &locale) : d_ptr(new QResourcePrivate(this))
{
    Q_D(QResource);
    d->fileName = file;
    d->locale = locale;
}

/*!
    Releases the resources of the QResource object.
*/
QResource::~QResource()
{
    delete d_ptr;
}

/*!
  Sets a QResource to only load the localization of resource to for \a
  locale. If a resource for the specific locale is not found then the
  C locale is used.

  \sa locale(), setFileName(), QLocale
*/

void QResource::setLocale(const QLocale &locale)
{
    Q_D(QResource);
    d->clear();
    d->locale = locale;
}

/*!
  Returns the locale used to locate the data for the QResource.

   \sa setLocale(), QLocale
*/

QLocale QResource::locale() const
{
    Q_D(const QResource);
    return d->locale;
}

/*!
    Sets a QResource to point to \a file. \a file can either be absolute,
    in which case it is opened directly, if relative then the file will be
    tried to be found in searchPaths().

    \sa fileName(), absoluteFilePath()
*/

void QResource::setFileName(const QString &file)
{
    Q_D(QResource);
    d->clear();
    d->fileName = file;
}

/*!
    Returns the full path to the file that this QResource represents as it
    was passed.

    \sa setFilePath(), absoluteFilePath()
*/

QString QResource::fileName() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return d->fileName;
}

/*!
    Returns the real path that this QResource represents, if the resource
    was found via the searchPaths() it will be indicated in the path.

    \sa fileName()
*/

QString QResource::absoluteFilePath() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return d->absoluteFilePath;
}

/*!
    Returns true if the resource really exists in the resource heirarchy,
    false otherwise.

*/

bool QResource::isValid() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return !d->related.isEmpty();
}

/*!
    \fn bool QResource::isFile() const
    \internal

    Returns true if the resource represents a file and thus has data
    backing it, false if it represents a directory.

    \sa isDir()
*/


/*!
    Returns true if the resource represents a file and the data backing it
    is in a compressed format, false otherwise.

    \sa data(), isFile()
*/

bool QResource::isCompressed() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return d->compressed;
}

/*!
    Returns the size of the data backing the resource.

    \sa data(), isFile()
*/

qint64 QResource::size() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return d->size;
}

/*!
    Returns direct access to a read only segment of data that this resource
    represents. If the resource is compressed the data returns is
    compressed and qUncompress() must be used to access the data. If the
    resource is a directory 0 is returned.

    \sa size(), isCompressed(), isFile()
*/

const uchar *QResource::data() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return d->data;
}

/*! \internal

    Returns true if the resource represents a directory and thus may have
    children() in it, false if it represents a file.

    \sa isFile()
*/

bool QResource::isDir() const
{
    Q_D(const QResource);
    d->ensureInitialized();
    return d->container;
}

/*! \internal


    Returns a list of all resources in this directory, if the resource
    represents a file the list will be empty.

    \sa isDir()
*/

QStringList QResource::children() const
{
    Q_D(const QResource);
    d->ensureChildren();
    return d->children;
}

/*!
  Adds \a path to the search paths searched in to find resources that are
  not specified with an absolute path. The default search path is to search
  only in the root (\c{:/}). The last path added will be consulted first
  upon next QResource creation.

  \sa QResource::QResource, addSearchPath()
*/

void
QResource::addSearchPath(const QString &path)
{
    if(path[0] != QLatin1Char('/')) {
        qWarning("QDir::addResourceSearchPath: Search paths must be absolute (start with /) [%s]",
                 path.toLocal8Bit().data());
        return;
    }
    QMutexLocker lock(resourceMutex());
    resourceSearchPaths()->prepend(path);
}

/*!
  Returns the current search path list. This list is consulted when
  creating a relative resource.

  \sa QResource::QResource, addSearchPath()
*/

QStringList
QResource::searchPaths()
{
    QMutexLocker lock(resourceMutex());
    return *resourceSearchPaths();
}

inline int QResourceRoot::hash(int node) const
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
inline QString QResourceRoot::name(int node) const
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
int QResourceRoot::findNode(const QString &_path, const QLocale &locale) const
{
    QString path = _path;
    {
        QString root = mappingRoot();
        if(!root.isEmpty()) {
            if(root == path) {
                path = QLatin1String("/");
            } else {
                if(!root.endsWith(QLatin1String("/")))
                    root += QLatin1String("/");
                if(path.size() >= root.size() && path.startsWith(root))
                    path = path.mid(root.length()-1);
                if(path.isEmpty())
                    path = QLatin1String("/");
            }
        }
    }
    if(path == QLatin1String("/"))
        return 0;

    //the root node is always first
    int child_count = (tree[6] << 24) + (tree[7] << 16) +
                      (tree[8] << 8) + (tree[9] << 0);
    int child       = (tree[10] << 24) + (tree[11] << 16) +
                      (tree[12] << 8) + (tree[13] << 0);

    //now iterate up the tree
    int node = -1;
    QStringList segments = path.split(QLatin1Char('/'), QString::SkipEmptyParts);
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
                            else if((country == QLocale::AnyCountry && language == locale.language()) ||
                                    (country == QLocale::AnyCountry && language == QLocale::C && node == -1))
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
short QResourceRoot::flags(int node) const
{
    if(node == -1)
        return 0;
    const int offset = findOffset(node) + 4; //jump past name
    return (tree[offset+0] << 8) + (tree[offset+1] << 0);
}
const uchar *QResourceRoot::data(int node, qint64 *size) const
{
    if(node == -1) {
        *size = 0;
        return 0;
    }
    int offset = findOffset(node) + 4; //jump past name

    const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
    offset += 2;

    offset += 4; //jump past locale

    if(!(flags & Directory)) {
        const int data_offset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                                (tree[offset+2] << 8) + (tree[offset+3] << 0);
        const uint data_length = (payloads[data_offset+0] << 24) + (payloads[data_offset+1] << 16) +
                                 (payloads[data_offset+2] << 8) + (payloads[data_offset+3] << 0);
        const uchar *ret = payloads+data_offset+4;
        *size = data_length;
        return ret;
    }
    *size = 0;
    return 0;
}
QStringList QResourceRoot::children(int node) const
{
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
bool QResourceRoot::mappingRootSubdir(const QString &path, QString *match) const
{
    const QString root = mappingRoot();
    if(!root.isEmpty()) {
        const QStringList root_segments = root.split(QLatin1Char('/'), QString::SkipEmptyParts),
                          path_segments = path.split(QLatin1Char('/'), QString::SkipEmptyParts);
        if(path_segments.size() <= root_segments.size()) {
            int matched = 0;
            for(int i = 0; i < path_segments.size(); ++i) {
                if(root_segments[i] != path_segments[i])
                    break;
                ++matched;
            }
            if(matched == path_segments.size()) {
                if(match && root_segments.size() > matched)
                    *match = root_segments.at(matched);
                return true;
            }
        }
    }
    return false;
}

Q_CORE_EXPORT bool qRegisterResourceData(int version, const unsigned char *tree,
                                         const unsigned char *name, const unsigned char *data)
{
    QMutexLocker lock(resourceMutex());
    if(version == 0x01 && resourceList()) {
        bool found = false;
        QResourceRoot res(tree, name, data);
        for(int i = 0; i < resourceList()->size(); ++i) {
            if(*resourceList()->at(i) == res) {
                found = true;
                break;
            }
        }
        if(!found) {
            QResourceRoot *root = new QResourceRoot(tree, name, data);
            root->ref.ref();
            resourceList()->append(root);
        }
        return true;
    }
    return false;
}

Q_CORE_EXPORT bool qUnregisterResourceData(int version, const unsigned char *tree,
                                           const unsigned char *name, const unsigned char *data)
{
    QMutexLocker lock(resourceMutex());
    if(version == 0x01 && resourceList()) {
        QResourceRoot res(tree, name, data);
        for(int i = 0; i < resourceList()->size(); ) {
            if(*resourceList()->at(i) == res) {
                QResourceRoot *root = resourceList()->takeAt(i);
                if(!root->ref.deref())
                    delete root;
            } else {
                ++i;
            }
        }
        return true;
    }
    return false;
}

//run time resource creation

#if defined(Q_OS_UNIX)
#define QT_USE_MMAP
#endif

// most of the headers below are already included in qplatformdefs.h
// also this lacks Large File support but that's probably irrelevant
#if defined(QT_USE_MMAP)
// for mmap
#include <sys/mman.h>
#include <errno.h>
#endif

class QDynamicResourceRoot: public QResourceRoot
{
    QString root;
    // for mmap'ed files, this is what needs to be unmapped.
    uchar *unmapPointer;
    unsigned int unmapLength;
    bool fromMM;

public:
    inline QDynamicResourceRoot(const QString &_root) : root(_root), unmapPointer(0), unmapLength(0) { }
    ~QDynamicResourceRoot() {
        if (unmapPointer && unmapLength) {
#if defined(QT_USE_MMAP)
            if(fromMM)
                munmap(unmapPointer, unmapLength);
            else
#endif
                delete [] unmapPointer;
            unmapPointer = 0;
            unmapLength = 0;
        }
    }
    virtual QString mappingRoot() const { return root; }

    bool registerSelf(const QString &filename) {
        bool ok = false;
#ifdef QT_USE_MMAP

#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

        int fd = QT_OPEN(QFile::encodeName(filename), O_RDONLY,
#if defined(Q_OS_WIN)
                         _S_IREAD | _S_IWRITE
#else
                         0666
#endif
            );
        if (fd >= 0) {
            struct stat st;
            if (!fstat(fd, &st)) {
                uchar *ptr;
                ptr = reinterpret_cast<uchar *>(
                    mmap(0, st.st_size,             // any address, whole file
                         PROT_READ,                 // read-only memory
                         MAP_FILE | MAP_PRIVATE,    // swap-backed map from file
                         fd, 0));                   // from offset 0 of fd
                if (ptr && ptr != reinterpret_cast<uchar *>(MAP_FAILED)) {
                    unmapPointer = ptr;
                    unmapLength = st.st_size;
                    fromMM = true;
                    ok = true;
                }
            }
            ::close(fd);
        }
#endif // QT_USE_MMAP
        if(!ok) {
            QFile file(filename);
            if (!file.exists())
                return false;
            unmapLength = file.size();
            unmapPointer = new uchar[unmapLength];

            if (file.open(QIODevice::ReadOnly))
                ok = (unmapLength == (uint)file.read((char*)unmapPointer, unmapLength));

            if (!ok) {
                delete [] unmapPointer;
                unmapPointer = 0;
                unmapLength = 0;
                return false;
            }
            fromMM = false;
        }
        if(!ok)
            return false;

        //setup the data now
        int offset = 0;

        //magic number
        if(unmapPointer[offset+0] != 'q' || unmapPointer[offset+1] != 'r' ||
           unmapPointer[offset+2] != 'e' || unmapPointer[offset+3] != 's') {
            return false;
        }
        offset += 4;

        const int version = (unmapPointer[offset+0] << 24) + (unmapPointer[offset+1] << 16) +
                         (unmapPointer[offset+2] << 8) + (unmapPointer[offset+3] << 0);
        offset += 4;

        const int tree_offset = (unmapPointer[offset+0] << 24) + (unmapPointer[offset+1] << 16) +
                                (unmapPointer[offset+2] << 8) + (unmapPointer[offset+3] << 0);
        offset += 4;

        const int data_offset = (unmapPointer[offset+0] << 24) + (unmapPointer[offset+1] << 16) +
                                (unmapPointer[offset+2] << 8) + (unmapPointer[offset+3] << 0);
        offset += 4;

        const int name_offset = (unmapPointer[offset+0] << 24) + (unmapPointer[offset+1] << 16) +
                                (unmapPointer[offset+2] << 8) + (unmapPointer[offset+3] << 0);
        offset += 4;

        if(version == 0x01) {
            setSource(unmapPointer+tree_offset, unmapPointer+name_offset, unmapPointer+data_offset);
            return true;
        }
        return false;
    }
};

/*!
   A resource can be left out of your binary and then loaded at runtime,
   this can often be useful to load a large set of icons into your
   application that may change based on a setting or that can be edited by
   a user and later recreated. The resource is immediately loaded into
   memory (either by reading as a single file, or being memory mapped),
   this can prove to be a significant gain as only a single file will be
   loaded and then pieces of the data will be given out via the path
   requested in QResource::setFileName(). Returns true upon successful opening
   of \a rccFileName, false upon failure.

   \sa unregisterResource()
*/

bool
QResource::registerResource(const QString &rccFilename, const QString &resourceRoot)
{
    QString r = resourceRoot;
    if(!r.isEmpty()) {
        if(r.startsWith(QLatin1Char(':')))
            r = r.mid(1);
        if(!r.isEmpty()) {
            r = QDir::cleanPath(r);
            if(r[0] != QLatin1Char('/')) {
                qWarning("QDir::registerResource: Registering a resource [%s] must be rooted in an absolute path (start with /) [%s]",
                         rccFilename.toLocal8Bit().data(), resourceRoot.toLocal8Bit().data());
                return false;
            }
        }
    }

    QDynamicResourceRoot *root = new QDynamicResourceRoot(r);
    if(root->registerSelf(rccFilename)) {
        root->ref.ref();
        QMutexLocker lock(resourceMutex());
        resourceList()->append(root);
        return true;
    }
    delete root;
    return false;
}

/*!
  Removes a reference to \a rccFilename, returns true if the resource could
  be unloaded, false otherwise. If there are QResources that currently
  reference resources inside of the resource they will continue to be valid
  but the resource file itself will be removed from the resource roots and
  thus no further QResource can be created pointing into this resource
  data. The resource itself will be unmapped from memory when the last
  QResource points into it.

  \sa registerResource()
*/

bool
QResource::unregisterResource(const QString &rccFilename, const QString &resourceRoot)
{
    Q_UNUSED(rccFilename); //### implement!
    Q_UNUSED(resourceRoot); //### implement!
    return false;
}


//file type handler
class QResourceFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    QResourceFileEngineHandler() { }
    ~QResourceFileEngineHandler() { }
    QAbstractFileEngine *create(const QString &path) const;
};
QAbstractFileEngine *QResourceFileEngineHandler::create(const QString &path) const
{
    if (path.size() > 0 && path.startsWith(QLatin1Char(':')))
        return new QResourceFileEngine(path);
    return 0;
}

//resource engine
class QResourceFileEnginePrivate : public QAbstractFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceFileEngine)
private:
    qint64 offset;
    QResource resource;
    QByteArray uncompressed;
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
    if(!d->resource.isValid() || !d->resource.isDir())
        return ret; // cannot read the "directory"

    QStringList entries = d->resource.children();
    for(int i = 0; i < entries.size(); i++) {
        QResource entry(d->resource.fileName() + QLatin1String("/") + entries[i]);
#ifndef QT_NO_REGEXP
        if(!(filters & QDir::AllDirs && entry.isDir())) {
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
        if  ((doDirs && entry.isDir()) ||
             (doFiles && !entry.isDir()))
            ret.append(entries[i]);
    }
    return ret;
}

bool QResourceFileEngine::caseSensitive() const
{
    return true;
}

QResourceFileEngine::QResourceFileEngine(const QString &file) :
    QAbstractFileEngine(*new QResourceFileEnginePrivate)
{
    Q_D(QResourceFileEngine);
    d->resource.setFileName(file);
    if(d->resource.isCompressed() && d->resource.size()) {
#ifndef QT_NO_COMPRESS
        d->uncompressed = qUncompress(d->resource.data(), d->resource.size());
#else
        Q_ASSERT("QResourceFileEngine::open: Qt built without support for compression");
#endif
    }
}

QResourceFileEngine::~QResourceFileEngine()
{
}

void QResourceFileEngine::setFileName(const QString &file)
{
    Q_D(QResourceFileEngine);
    d->resource.setFileName(file);
}

bool QResourceFileEngine::open(QIODevice::OpenMode flags)
{
    Q_D(QResourceFileEngine);
    if (d->resource.fileName().isEmpty()) {
        qWarning("QResourceFileEngine::open: Missing file name");
        return false;
    }
    if(flags & QIODevice::WriteOnly)
        return false;
    if(!d->resource.isValid())
       return false;
    return true;
}

bool QResourceFileEngine::close()
{
    Q_D(QResourceFileEngine);
    d->offset = 0;
    d->uncompressed.clear();
    return true;
}

bool QResourceFileEngine::flush()
{
    return false;
}

qint64 QResourceFileEngine::read(char *data, qint64 len)
{
    Q_D(QResourceFileEngine);
    if(len > size()-d->offset)
        len = size()-d->offset;
    if(len <= 0)
        return 0;
    if(d->resource.isCompressed())
        memcpy(data, d->uncompressed.constData()+d->offset, len);
    else
        memcpy(data, d->resource.data()+d->offset, len);
    d->offset += len;
    return len;
}

qint64 QResourceFileEngine::write(const char *, qint64)
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
    if(!d->resource.isValid())
        return 0;
    if(d->resource.isCompressed())
        return d->uncompressed.size();
    return d->resource.size();
}

qint64 QResourceFileEngine::pos() const
{
    Q_D(const QResourceFileEngine);
    return d->offset;
}

bool QResourceFileEngine::atEnd() const
{
    Q_D(const QResourceFileEngine);
    if(!d->resource.isValid())
        return true;
    return d->offset == size();
}

bool QResourceFileEngine::seek(qint64 pos)
{
    Q_D(QResourceFileEngine);
    if(!d->resource.isValid())
        return false;

    if(d->offset > size())
        return false;
    d->offset = pos;
    return true;
}

bool QResourceFileEngine::isSequential() const
{
    return false;
}

QAbstractFileEngine::FileFlags QResourceFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    Q_D(const QResourceFileEngine);
    QAbstractFileEngine::FileFlags ret = 0;
    if(!d->resource.isValid())
        return ret;

    if(type & PermsMask)
        ret |= QAbstractFileEngine::FileFlags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm);
    if(type & TypesMask) {
        if(d->resource.isDir())
            ret |= DirectoryType;
        else
            ret |= FileType;
    }
    if(type & FlagsMask) {
        ret |= ExistsFlag;
        if(d->resource.absoluteFilePath() == QLatin1String(":/"))
            ret |= RootFlag;
    }
    return ret;
}

bool QResourceFileEngine::setPermissions(uint)
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
        const QString path = (file == AbsolutePathName) ? d->resource.absoluteFilePath() : d->resource.fileName();
	const int slash = path.lastIndexOf(QLatin1Char('/'));
	if (slash != -1)
	    return path.left(slash);
    } else if(file == CanonicalName || file == CanonicalPathName) {
        const QString absoluteFilePath = d->resource.absoluteFilePath();
        if(file == CanonicalPathName) {
            const int slash = absoluteFilePath.lastIndexOf(QLatin1Char('/'));
            if (slash != -1)
                return absoluteFilePath.left(slash);
        }
        return absoluteFilePath;
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

bool QResourceFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    Q_UNUSED(extension);
    Q_UNUSED(option);
    Q_UNUSED(output);
    return false;
}

bool QResourceFileEngine::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

//Initialization and cleanup
Q_GLOBAL_STATIC(QResourceFileEngineHandler, resource_file_handler)

static int qt_force_resource_init() { resource_file_handler(); return 1; }
Q_CORE_EXPORT void qInitResourceIO() { resource_file_handler(); }
static int qt_forced_resource_init = qt_force_resource_init();
Q_CONSTRUCTOR_FUNCTION(qt_force_resource_init)



