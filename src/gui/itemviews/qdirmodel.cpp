#include "qdirmodel.h"
#include <qvector.h>
#include <qobject.h>
#include <private/qobject_p.h>

/* XPM */
static const char* const open_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "c c #cccccc",
    "a c #ffffff",
    "................",
    "................",
    "...*****........",
    "..*aaaaa*.......",
    ".*abcbcba******.",
    ".*acbcbcaaaaaa*d",
    ".*abcbcbcbcbcb*d",
    "*************b*d",
    "*aaaaaaaaaa**c*d",
    "*abcbcbcbcbbd**d",
    ".*abcbcbcbcbcd*d",
    ".*acbcbcbcbcbd*d",
    "..*acbcbcbcbb*dd",
    "..*************d",
    "...ddddddddddddd",
    "................"};

/* XPM */
static const char * const closed_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "a c #cccccc",
    "c c #ffffff",
    "................",
    "................",
    "..*****.........",
    ".*ababa*........",
    "*abababa******..",
    "*cccccccccccc*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "**************d.",
    ".dddddddddddddd.",
    "................"};

/* XPM */
static const char * const link_dir_xpm[]={
    "16 16 10 1",
    "h c #808080",
    "g c #a0a0a0",
    "d c #000000",
    "b c #ffff00",
    "f c #303030",
    "# c #999999",
    "a c #cccccc",
    "e c #585858",
    "c c #ffffff",
    ". c None",
    "................",
    "................",
    "..#####.........",
    ".#ababa#........",
    "#abababa######..",
    "#cccccccccccc#d.",
    "#cbababababab#d.",
    "#cabababababa#d.",
    "#cbababdddddddd.",
    "#cababadccccccd.",
    "#cbababdcececcd.",
    "#cababadcefdfcd.",
    "#cbababdccgdhcd.",
    "#######dccchccd.",
    ".dddddddddddddd.",
    "................"};


/* XPM */
static const char* const file_xpm[]={
    "16 16 5 1",
    ". c #7f7f7f",
    "# c None",
    "c c #000000",
    "b c #bfbfbf",
    "a c #ffffff",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".bbbbbbbbbbbc###",
    "ccccccccccccc###"};

/* XPM */
static const char * const link_file_xpm[]={
    "16 16 10 1",
    "h c #808080",
    "g c #a0a0a0",
    "d c #c3c3c3",
    ". c #7f7f7f",
    "c c #000000",
    "b c #bfbfbf",
    "f c #303030",
    "e c #585858",
    "a c #ffffff",
    "# c None",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaadc###",
    ".aaaaaaaaaadc###",
    ".aaaacccccccc###",
    ".aaaacaaaaaac###",
    ".aaaacaeaeaac###",
    ".aaaacaefcfac###",
    ".aaaacaagchac###",
    ".ddddcaaahaac###",
    "ccccccccccccc###"};


Q4FileIconProvider::Q4FileIconProvider()
{
    QPixmap filePixmap(file_xpm);
    file.setPixmap(filePixmap, QIconSet::Small);

    QPixmap openPixmap(open_xpm);
    QPixmap closedPixmap(closed_xpm);
    dir.setPixmap(openPixmap, QIconSet::Small, QIconSet::Normal, QIconSet::On);
    dir.setPixmap(closedPixmap, QIconSet::Small, QIconSet::Normal, QIconSet::Off);

    QPixmap linkFilePixmap(link_file_xpm);
    linkFile.setPixmap(linkFilePixmap, QIconSet::Small);
    
    QPixmap linkDirPixmap(link_dir_xpm);
    linkDir.setPixmap(linkDirPixmap, QIconSet::Small);
}

QIconSet Q4FileIconProvider::icons(const QFileInfo &fileInfo) const
{
    if (fileInfo.isDir())
        return fileInfo.isSymLink() ? linkDir : dir;
   return fileInfo.isSymLink() ? linkFile : file;
}

QString Q4FileIconProvider::type(const QFileInfo &info) const
{
    return info.isDir() ? "Directory" : info.extension() + " File";
}

class QDirModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDirModel);
public:
    struct QDirNode
    {
        QDirNode *parent;
        QFileInfo info;
        QVector<QDirNode> children;
    };

    QDirModelPrivate() : iconProvider(&defaultProvider) {}
    
    QDirNode *node(int row, QDirNode *parent) const;
    QDirNode *parent(QDirNode *child) const;
    QVector<QDirNode> children(QDirNode *parent) const;
    int idx(QDirNode *node) const;

    QDir root;
    QVector<QDirNode> tree;
    
    Q4FileIconProvider *iconProvider;
    Q4FileIconProvider defaultProvider;
};

#define d d_func()
#define q q_func()

QDirModel::QDirModel(const QDir &directory, QObject *parent)
    : QAbstractItemModel(*new QDirModelPrivate, parent)
{
    init(directory);
}

QDirModel::QDirModel(QDirModelPrivate &dd, const QDir &directory, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    init(directory);
}

QDirModel::~QDirModel()
{
}

void QDirModel::init(const QDir &directory)
{
    d->root = directory;
    d->tree = d->children(0);
}

QModelIndex QDirModel::index(int row, int column, const QModelIndex &parent, QModelIndex::Type type) const
{
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    if (p && p->children.isEmpty())
	p->children = d->children(p);
    QDirModelPrivate::QDirNode *n = d->node(row, p);
    if (!n)
	return QModelIndex();
    return QModelIndex(row, column, n, type);
}

QModelIndex QDirModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
	return QModelIndex();
    QDirModelPrivate::QDirNode *p = d->parent(static_cast<QDirModelPrivate::QDirNode*>(child.data()));
    if (!p)
	return QModelIndex();
    return QModelIndex(d->idx(p), 0, p);
}

int QDirModel::rowCount(const QModelIndex &parent) const
{
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    if (p && p->children.isEmpty())
	p->children = d->children(p);
    QVector<QDirModelPrivate::QDirNode> children =  p ? p->children : d->tree;
    return children.count();
}

int QDirModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant QDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qWarning("data: the index was not valid");
        return QVariant();
    }
    
    if (index.type() == QModelIndex::HorizontalHeader) {
	switch (index.column()) {
        case 0: return "Name";
        case 1: return "Size";
        case 2: return "Type";
        case 3: return "Modified";
        default: return QVariant();
        }
    } else if (index.type() == QModelIndex::VerticalHeader) {
	return index.row();
    }
    
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("data: the node does not exist");
        return QVariant();
    }
    
    if (role == Display || role == Edit) {
        switch (index.column()) {
        case 0: return node->info.fileName();
        case 1: return node->info.size();
        case 2: return d->iconProvider->type(node->info);
        case 3: return node->info.lastModified();
        default:
            qWarning("data: invalid display value column %d", index.column());
            return QVariant();
        }
    }
    
    if (role == Decoration && index.column() == 0)
        return d->iconProvider->icons(node->info);
    return QVariant();
}

void QDirModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid()) {
        qWarning("setData: the index was not valid");
        return;
    }
    if (index.column() != 0) {
        qWarning("setData: wrong column %d", index.column());
        return;
    }
    if (role != Edit) {
        qWarning("setData: wrong role %d", role);
        return;
    }
    if (!isEditable(index)) {
        qWarning("setData: the item is not editable");
        return;
    }
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    QDir dir = node->info.dir();
    if (dir.rename(node->info.fileName(), value.toString()))
        emit contentsChanged(index, index);
    else
        qWarning("setData: file renaming failed");
}

bool QDirModel::hasChildren(const QModelIndex &parent) const
{
    return (parent.data() && static_cast<QDirModelPrivate::QDirNode*>(parent.data())->info.isDir());
}

bool QDirModel::isSelectable(const QModelIndex &) const
{
    return true;
}

bool QDirModel::isEditable(const QModelIndex &index) const
{
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("isEditable: the node does not exist");
        return false;
    }
    return (index.column() == 0) && node->info.isWritable();
}

bool QDirModel::isDragEnabled(const QModelIndex &) const
{
    return true;
}

void QDirModel::setIconProvider(Q4FileIconProvider *provider)
{
    d->iconProvider = provider;
}

Q4FileIconProvider *QDirModel::iconProvider() const
{
    return d->iconProvider;
}

void QDirModel::setNameFilter(const QString &filter)
{    
    d->root.setNameFilter(filter);
}

QString QDirModel::nameFilter() const
{
    return d->root.nameFilter();
}

void QDirModel::setFilter(QDir::FilterSpec spec)
{
    d->root.setFilter(spec);
}

QDir::FilterSpec QDirModel::filter() const
{
    return d->root.filter();
}

void QDirModel::setSorting(QDir::SortSpec spec)
{
    d->root.setSorting(spec);
}

QDir::SortSpec QDirModel::sorting() const
{
    return d->root.sorting();
}

QFileInfo QDirModel::fileInfo(const QModelIndex &index) const
{
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("fileInfo: the node does not exist");
        return QFileInfo();
    }
    return node->info;
}

void QDirModel::mkdir(const QModelIndex &parent, const QString &name)
{
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    if (p) {
        p->info.dir().mkdir(name);
        p->children = d->children(p);
    } else {
        d->root.mkdir(name);
        d->tree = d->children(0);
    }
}

void QDirModel::rmdir(const QModelIndex &index)
{
    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!n) {
        qWarning("rmdir: the node does not exist");
        return;
    }
    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return;
    }
    n->info.dir().rmdir(n->info.absFilePath());

    // FIXME: invalidates parent and deletes node
    QDirModelPrivate::QDirNode *p = d->parent(n);
    if (p)
        p->children = d->children(p);
    else
        d->tree = d->children(0);
}

QDirModelPrivate::QDirNode *QDirModelPrivate::node(int row, QDirNode *parent) const
{
    if (row < 0)
	return 0;
    if (parent && parent->children.isEmpty())
	parent->children = children(parent);
    QVector<QDirNode> v = parent ? parent->children : tree;
    return !v.isEmpty() && row < v.count() ? const_cast<QDirNode*>(&v.at(row)) : 0;
}

QDirModelPrivate::QDirNode *QDirModelPrivate::parent(QDirNode *child) const
{
    return child ? child->parent : 0;
}

QVector<QDirModelPrivate::QDirNode> QDirModelPrivate::children(QDirNode *parent) const
{
    if (parent && parent->children.isEmpty() && parent->info.isDir() || !parent) {
        QDir dir = parent ? QDir(parent->info.filePath()) : root;
        const QFileInfoList info = dir.entryInfoList(root.nameFilter(), root.filter(), root.sorting());
        QVector<QDirNode> nodes(info.count());
	for (int i = 0; i < (int)dir.count(); ++i) {
            nodes[i].parent = parent;
            nodes[i].info = info.at(i);
        }
	return nodes;
    }
    return QVector<QDirNode>();
}

int QDirModelPrivate::idx(QDirNode *node) const
{
    const QVector<QDirNode> children = node && node->parent ? node->parent->children : tree;
    if (children.isEmpty())
	return -1;
    const QDirNode *first = &(children.at(0));
    return (node - first);
}
