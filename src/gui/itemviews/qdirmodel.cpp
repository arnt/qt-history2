/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdirmodel.h"
#include <qfile.h>
#include <qvector.h>
#include <qobject.h>
#include <qdragobject.h>
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

static bool qt_copy_file(const QString &from, const QString &to)
{
    QFile src(from);
    if (!src.open(IO_ReadOnly|IO_Raw))
        return false;
    QFile dst(to == from ? "copy_of_" + to : to);
    if (dst.exists() || !dst.open(IO_WriteOnly|IO_Raw))
        return false;
    char buffer[4096];
    Q_LONG len = 0;
    while (!src.atEnd()) {
        len = src.readBlock(buffer, 4096);
        if (len < 0)
            return false;
        len = dst.writeBlock(buffer, len);
    }
    return true;
}

static bool qt_link_file(const QString &from, const QString &to)
{
    qDebug("function not implemented: link %s to %s", from.latin1(), to.latin1());
    return false;
}

static bool qt_move_file(const QString &from, const QString &to)
{
    if (!qt_copy_file(from, to))
        return false;
    return QFile::remove(from);
}

QFileIconProvider::QFileIconProvider()
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

QFileIconProvider::~QFileIconProvider()
{

}

QIconSet QFileIconProvider::icons(const QFileInfo &fileInfo) const
{
    if (fileInfo.isDir())
        return fileInfo.isSymLink() ? linkDir : dir;
   return fileInfo.isSymLink() ? linkFile : file;
}

QString QFileIconProvider::type(const QFileInfo &info) const
{
    return info.isDir() ? "Directory" : info.extension() + " File";
}

class QDirModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDirModel)
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
    void refresh(QDirNode *parent);

    QDir root;
    QVector<QDirNode> tree;

    QFileIconProvider *iconProvider;
    QFileIconProvider defaultProvider;
};

#define d d_func()
#define q q_func()

/*!
  \class QDirModel qdirmodel.h

  \brief data model for browsing the local filesystem.

  This class provides access to the local filesystem,
  and provides functions for renaming and removing files
  and directories, and creating new directories.
*/

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
    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(child.data());
    if (!n)
        return QModelIndex();
    QDirModelPrivate::QDirNode *p = d->parent(n);
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
        if (role != Display)
            return QVariant();
	switch (index.column()) {
        case 0: return "Name";
        case 1: return "Size";
        case 2: return "Type";
        case 3: return "Modified";
        default: return QVariant();
        }
    } else if (index.type() == QModelIndex::VerticalHeader) {
        if (role != Display)
            return QVariant();
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

bool QDirModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid()) {
        qWarning("setData: the index was not valid");
        return false;
    }
    if (index.column() != 0) {
        qWarning("setData: wrong column %d", index.column());
        return false;
    }
    if (role != Edit) {
        qWarning("setData: wrong role %d", role);
        return false;
    }
    if (!isEditable(index)) {
        qWarning("setData: the item is not editable");
        return false;
    }
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    QDir dir = node->info.dir();
    if (dir.rename(node->info.fileName(), value.toString())) {
        QModelIndex par = parent(index);
        QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(par.data());
        d->refresh(p);
        emit contentsChanged(topLeft(par), bottomRight(par));
        return true;
    }
    qWarning("setData: file renaming failed");
    return false;
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

bool QDirModel::isDropEnabled(const QModelIndex &index) const
{
    return isDir(index) && isEditable(index);
}

bool QDirModel::isSortable() const
{
    return true;
}

void QDirModel::sort(int column, SortOrder order)
{
    int spec = (order == Qt::Descending ? QDir::Reversed : 0);

    switch (column) {
    case 0:
        spec |= QDir::Name;
        break;
    case 1:
        spec |= QDir::Size;
        break;
    case 2:
        spec |= QDir::DirsFirst;
        break;
    case 3:
        spec |= QDir::Time;
        break;
    default:
        break;
    };

    setSorting(spec);
}

bool QDirModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    if (!(left.isValid() && right.isValid()))
        return left == right;
    QDirModelPrivate::QDirNode *l = static_cast<QDirModelPrivate::QDirNode*>(left.data());
    QDirModelPrivate::QDirNode *r = static_cast<QDirModelPrivate::QDirNode*>(right.data());
    return l->info.absFilePath() == r->info.absFilePath();
}

bool QDirModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (!(left.isValid() && right.isValid()))
        return false;

    QDirModelPrivate::QDirNode *l = static_cast<QDirModelPrivate::QDirNode*>(left.data());
    QDirModelPrivate::QDirNode *r = static_cast<QDirModelPrivate::QDirNode*>(right.data());

    // this depends on the sort column
    int spec = sorting();
    if (spec & QDir::Name) // col 0
        return (l->info.fileName() < r->info.fileName());
    if (spec & QDir::Size) // col 1
        return (l->info.size() < r->info.size());
    if (QDir::DirsFirst) // col 2
        return !l->info.isDir();
    if (spec & QDir::Time) // col 3
        return l->info.lastModified() < r->info.lastModified();

    return QAbstractItemModel::lessThan(left, right);
}

bool QDirModel::canDecode(QMimeSource *src) const
{
    return QUriDrag::canDecode(src);
}

bool QDirModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    // FIXME: what about directories ?
    QStringList files;
    if (!QUriDrag::decodeLocalFiles(e, files))
        return false;
    emit contentsRemoved(topLeft(parent), bottomRight(parent));
    bool success = true;
    QString to = path(parent) + QDir::separator();
    QStringList::const_iterator it = files.begin();
    switch (e->action()) {
    case QDropEvent::Copy:
        for (; it != files.end(); ++it)
            success = qt_copy_file(*it, to + QFileInfo(*it).fileName()) && success;
        break;
    case QDropEvent::Link:
        for (; it != files.end(); ++it)
            success = qt_link_file(*it, to + QFileInfo(*it).fileName()) && success;
        break;
    case QDropEvent::Move:
        for (; it != files.end(); ++it)
            success = qt_move_file(*it, to + QFileInfo(*it).fileName()) && success;
        break;
//     case default:
//         return false;
    }
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    if (p)
        p->children = d->children(p);
    else
        d->tree = d->children(0);
    emit contentsInserted(topLeft(parent), bottomRight(parent));
    return success;
}

QDragObject *QDirModel::dragObject(const QModelIndexList &indices, QWidget *dragSource)
{
    QList<QByteArray> uris;
    QList<QModelIndex>::const_iterator it = indices.begin();
    for (; it != indices.end(); ++it)
        uris.append(QUriDrag::localFileToUri(path(*it)));
    return new QUriDrag(uris, dragSource);
}

void QDirModel::setIconProvider(QFileIconProvider *provider)
{
    d->iconProvider = provider;
}

QFileIconProvider *QDirModel::iconProvider() const
{
    return d->iconProvider;
}

void QDirModel::setNameFilters(const QStringList &filters)
{
    // FIXME: this will rebuild the entire structure of the qdirmodel
    emit contentsRemoved(topLeft(), bottomRight());
    d->root.setNameFilters(filters);
    d->tree = d->children(0);
    emit contentsInserted(topLeft(), bottomRight());
}

QStringList QDirModel::nameFilters() const
{
    return d->root.nameFilters();
}

void QDirModel::setFilter(int spec)
{
    d->root.setFilter(spec);
}

QDir::FilterSpec QDirModel::filter() const
{
    return d->root.filter();
}

void QDirModel::setSorting(int spec)
{
    d->root.setSorting(spec);
    d->tree = d->children(0);
}

QDir::SortSpec QDirModel::sorting() const
{
    return d->root.sorting();
}

void QDirModel::setMatchAllDirs(bool enable)
{
    d->root.setMatchAllDirs(enable);
}

bool QDirModel::matchAllDirs() const
{
    return d->root.matchAllDirs();
}

void QDirModel::refresh(const QModelIndex &parent)
{
    emit contentsRemoved(topLeft(parent), bottomRight(parent));
    d->refresh(static_cast<QDirModelPrivate::QDirNode*>(parent.data()));
    emit contentsInserted(topLeft(), bottomRight());
}

QModelIndex QDirModel::index(const QString &path) const
{
    QModelIndex parent;
    QStringList pth = QDir::convertSeparators(path).split('/');
    QStringList entries;
    QDir dir = d->root;
    for (int i = 0; i < pth.count() - 1; ++i) {
        entries = dir.entryList(d->root.nameFilters(), d->root.filter(), d->root.sorting());
        parent = index(entries.indexOf(pth.at(i)), 0, parent);
        if (!dir.cd(pth.at(i))) {
            qWarning("index: the path does not exist");
            return QModelIndex();
        }
    }
    entries = dir.entryList(d->root.nameFilters(), d->root.filter(), d->root.sorting());
    return index(entries.indexOf(pth.last()), 0, parent);
}

QString QDirModel::path(const QModelIndex &index) const
{
    if (!index.isValid())
        return d->root.absPath();
    return fileInfo(index).absFilePath();
}

QString QDirModel::name(const QModelIndex &index) const
{
    if (!index.isValid())
        return d->root.dirName();
    return fileInfo(index).fileName();
}

QFileInfo QDirModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qWarning("fileInfo: index is invalid");
        return QFileInfo();
    }
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("fileInfo: the node does not exist");
        return QFileInfo();
    }
    return node->info;
}

bool QDirModel::isDir(const QModelIndex &index) const
{
    if (!index.isValid())
        return true;
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("isDir: the node does not exist");
        return false;
    }
    return node->info.isDir();
}

QModelIndex QDirModel::mkdir(const QModelIndex &parent, const QString &name)
{
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    int r;
    if (p) {
        QDir dir(p->info.absFilePath());
        if (!dir.mkdir(name))
            return QModelIndex();
        p->children = d->children(p);
        r = dir.entryList(d->root.nameFilters(), d->root.filter(), d->root.sorting()).indexOf(name);
    } else {
        if (!d->root.mkdir(name))
            return QModelIndex();
        d->tree = d->children(0);
        r = d->root.entryList(d->root.nameFilters(), d->root.filter(), d->root.sorting()).indexOf(name);
    }
    QModelIndex i = index(r, 0, parent);
    if (i.isValid())
        emit contentsInserted(i, i);
    return i;
}

bool QDirModel::rmdir(const QModelIndex &index)
{
    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!n) {
        qWarning("rmdir: the node does not exist");
        return false;
    }
    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return false;
    }
    if (!n->info.dir().rmdir(n->info.absFilePath()))
        return false;

    emit contentsRemoved(index, index);

    QDirModelPrivate::QDirNode *p = d->parent(n);
    if (p)
        p->children = d->children(p);
    else
        d->tree = d->children(0);
    
    return true;
}

bool QDirModel::remove(const QModelIndex &index)
{
    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!n) {
        qWarning("remove: the node does not exist");
        return false;
    }
    if (n->info.isDir()) {
        qWarning("remove: the node is a directory");
        return false;
    }

    emit contentsRemoved(index, index);
        
    QDirModelPrivate::QDirNode *p = d->parent(n);
    QDir dir = p ? p->info.dir() : d->root;
    if (!dir.remove(n->info.absFilePath()))
        return false;
    if (p)
        p->children = d->children(p);
    else
        d->tree = d->children(0);

    return true;
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
        dir.setMatchAllDirs(root.matchAllDirs());
        const QFileInfoList info = dir.entryInfoList(root.nameFilters(), root.filter(), root.sorting());
        QVector<QDirNode> nodes(info.count());
	for (int i = 0; i < (int)info.count(); ++i) {
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

void QDirModelPrivate::refresh(QDirNode *parent)
{
    QDir dir = parent ? QDir(parent->info.filePath()) : root;
    const QFileInfoList info = dir.entryInfoList(root.nameFilters(), root.filter(), root.sorting());
    QVector<QDirNode> *nodes = parent ? &(parent->children) : &tree;
    if (nodes->count() != info.count()) {
        qWarning("refresh: the number of children has changed");
        nodes->resize(info.count()); // FIXME: may cause dangling pointers in QModelIndex.d
    }
    for (int i = 0; i < (int)info.count(); ++i) {
        (*nodes)[i].parent = parent;
        (*nodes)[i].info = info.at(i);
        if (nodes->at(i).children.count() > 0)
            refresh(&(*nodes)[i]);
    }
}
