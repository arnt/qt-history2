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

#include "qdirmodel.h"
#include <qfile.h>
#include <qvector.h>
#include <qobject.h>
#include <qdragobject.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qstyle.h>
#include <qapplication.h>
#include <private/qabstractitemmodel_p.h>
#include <qdebug.h>


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

/*!
  \class QFileIconProvider

  \brief The QFileIconProvider class provides file icons for the QDirModel class.


*/

/*!
  Constructs a file icon provider.

*/

QFileIconProvider::QFileIconProvider()
{
    file.setPixmap(QApplication::style().stylePixmap(QStyle::SP_FileIcon), QIconSet::Small);
    fileLink.setPixmap(QApplication::style().stylePixmap(QStyle::SP_FileLinkIcon), QIconSet::Small);

    dir.setPixmap(QApplication::style().stylePixmap(QStyle::SP_DirOpenIcon),
                  QIconSet::Small, QIconSet::Normal, QIconSet::On);
    dir.setPixmap(QApplication::style().stylePixmap(QStyle::SP_DirClosedIcon), QIconSet::Small,
                  QIconSet::Normal, QIconSet::Off);
    dirLink.setPixmap(QApplication::style().stylePixmap(QStyle::SP_DirLinkIcon), QIconSet::Small,
                      QIconSet::Normal, QIconSet::On);

    driveHD.setPixmap(QApplication::style().stylePixmap(QStyle::SP_DriveHDIcon), QIconSet::Small);
    computer.setPixmap(QApplication::style().stylePixmap(QStyle::SP_ComputerIcon), QIconSet::Small);
}

/*!
  Destroys the file icon provider.

*/

QFileIconProvider::~QFileIconProvider()
{

}

/*!
  Returns an icon set for the computer.
*/

QIconSet QFileIconProvider::computerIcons() const
{
    return computer;
}

/*!
  Returns an icon set for the file described by \a info.
*/

QIconSet QFileIconProvider::icons(const QFileInfo &info) const
{
    if (info.isRoot())
        return driveHD;
    if (info.isFile())
        return file;
    if (info.isDir())
        return dir;
    if (info.isSymLink())
        return fileLink;
    return QIconSet();
}

/*!
  Returns the type of the file described by \a info.
*/

QString QFileIconProvider::type(const QFileInfo &info) const
{
    if (info.isRoot())
        return "Drive";
    if (info.isFile())
        return info.suffix() + " File";
    if (info.isDir())
        return "Directory";
    if (info.isSymLink())
        return "Symbolic Link";
    return "Unknown";
}

class QDirModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QDirModel)
public:
    struct QDirNode
    {
        QDirNode *parent;
        QFileInfo info;
        mutable QVector<QDirNode> children;
    };

    QDirModelPrivate()
        : rootIsVirtual(true),
          resolveSymlinks(true),
          iconProvider(&defaultProvider) {}

    void init(const QDir &directory);
    QDirNode *node(int row, QDirNode *parent) const;
    QDirNode *parent(QDirNode *child) const;
    QVector<QDirNode> children(QDirNode *parent) const;
    int idx(QDirNode *node) const;
    void refresh(QDirNode *parent);

    void savePersistentIndexes();
    void restorePersistentIndexes();

    QFileInfoList rootChildren() const;
    inline QString rootPath() const
        { return rootIsVirtual ? QObject::tr("My Computer") : root.info.absoluteFilePath(); }
    inline QString rootName() const
        { return rootIsVirtual ? QObject::tr("My Computer") : root.info.fileName(); }
    inline QIconSet rootIcons() const
        { return rootIsVirtual ? iconProvider->computerIcons() : iconProvider->icons(root.info); }

    mutable QDirNode root;
    bool rootIsVirtual;
    bool resolveSymlinks;

    int filterSpec;
    int sortSpec;
    QStringList nameFilters;

    QFileIconProvider *iconProvider;
    QFileIconProvider defaultProvider;

    QStringList savedPaths;
};

#define d d_func()
#define q q_func()

/*!
  \class QDirModel qdirmodel.h

  \brief The QDirModel class provides a data model for the local filesystem.

  \ingroup model-view

  This class provides access to the local filesystem, providing functions
  for renaming and removing files and directories, and for creating new
  directories. In the simplest case, it can be used with a suitable display
  widget as part of a browser or filer.

  A directory model that displays the contents of a default directory
  is constructed with a QDir to supply the file infomation, and a parent
  object:

  \code
  QDirModel *model = new QDirModel(QDir(), parent);
  \endcode

  A tree view can be used to display the contents of the model:

  \code
  QTreeView *treeView = new QTreeView(window);
  treeView->setModel(model);
  \endcode

  QDirModel can be accessed using the standard interface provided by
  QAbstractItemModel, but it also provides some convenience functions that are
  specific to a directory model.
  fileInfo(), icons(), mkdir(), name(), path(), isDir(), rmdir(), 

  nameFilters(), setFilter(), filter()

  \sa \link model-view-programming.html Model/View Programming\endlink QListView QTreeView

*/

/*!
    Constructs a new directory model with the given \a parent. The
    model initially contains information about the directory specified
    by \a path. Only those files matching the \a nameFilters and the
    \a filter are included in the model. The sort order is given by \a
    sorting.
*/

QDirModel::QDirModel(const QString &path,
                     const QStringList &nameFilters,
                     int filter,
                     int sorting,
                     QObject *parent)
    : QAbstractItemModel(*new QDirModelPrivate, parent)
{
    // the empty path means that we start with QDir::drives()
    if (path.isEmpty()) {
        d->nameFilters = nameFilters.isEmpty() ? "*" : nameFilters;
        d->filterSpec = filter;
        d->sortSpec = sorting;
        d->rootIsVirtual = true;
        d->root.parent = 0;
        d->root.info = QFileInfo();
        d->root.children = d->children(0);
    } else {
        QDir dir(path);
        dir.setNameFilters(nameFilters);
        dir.setFilter(filter);
        dir.setSorting(sorting);
        d->init(dir);
    }
}

/*!
  Constructs a directory model of the \a directory with a \a parent
  object.
*/

QDirModel::QDirModel(const QDir &directory, QObject *parent)
    : QAbstractItemModel(*new QDirModelPrivate, parent)
{
    d->init(directory);
}

/*!
    \internal
*/
QDirModel::QDirModel(QDirModelPrivate &dd, const QDir &directory, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    d->init(directory);
}

/*!
  Destroys this directory model.
*/

QDirModel::~QDirModel()
{

}

/*!
  Returns the model item index for the item in the \a parent with the
  given \a row, \a column, and \a type.

  The type is a value defined in \l QModelIndex::Type.

*/

QModelIndex QDirModel::index(int row, int column, const QModelIndex &parent, QModelIndex::Type type) const
{
    if (column < 0 || column >= 4 || row < 0 || row >= rowCount(parent)) { // does lazy population
        if (type == QModelIndex::View)
            qWarning("index: row %d column %d does not exist", row, column);
        return QModelIndex::Null;
    }

//     if (!parent.isValid())
//         qDebug("index: creating toplevel index %d %d", row, column);
//     else
//         qDebug("index: creating normal index %d %d", row, column);

    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    QDirModelPrivate::QDirNode *n = d->node(row, p);

    if (!n) {
        qWarning("index: the node does not exist");
	return QModelIndex::Null;
    }

    return createIndex(row, column, n, type);
}

/*!
  Return the parent of the given \a child model item.
*/

QModelIndex QDirModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
	return QModelIndex::Null;

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(child.data());
    if (!node) {
        qWarning("parent: valid index without node");
        return QModelIndex::Null;
    }

    QDirModelPrivate::QDirNode *par = d->parent(node);
    if (!par)
	return QModelIndex::Null; // parent is the root node

    int r = d->idx(par);
    if (r < 0) {
        qWarning("parent: the row was invalid");
        return QModelIndex::Null;
    }
    return createIndex(r, 0, par);
}

/*!
  Returns the number of rows in the \a parent model item.

*/

int QDirModel::rowCount(const QModelIndex &parent) const
{
    if (parent.type() == QModelIndex::HorizontalHeader)
        return 1;

    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());

    bool isDir = !p || p->info.isDir();
    QVector<QDirModelPrivate::QDirNode> *nodes = p ? &(p->children) : &(d->root.children);

    if (isDir && nodes->isEmpty()) {
//        qDebug("rowCount: refreshing children");
	*nodes = d->children(p); // lazy population
    }

//    qDebug("rowCount: returning %d", nodes->count());

    return nodes->count();
}

/*!
  Returns the number of columns in the \a parent model item.

*/

int QDirModel::columnCount(const QModelIndex &parent) const
{
    if (parent.type() == QModelIndex::VerticalHeader)
        return 1;
    return 4;
}

/*!
  Returns the data for the model item \a index with the given \a role.

*/

QVariant QDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qWarning("data: the index was not valid");
        return QVariant();
    }

    if (index.type() == QModelIndex::HorizontalHeader) {
        if (role != DisplayRole)
            return QVariant();
	switch (index.column()) {
        case 0: return "Name";
        case 1: return "Size";
        case 2: return "Type";
        case 3: return "Modified";
        default: return QVariant();
        }
    } else if (index.type() == QModelIndex::VerticalHeader) {
        if (role != DisplayRole)
            return QVariant();
	return index.row();
    }

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("data: the node does not exist");
        return QVariant();
    }

    if (role == DisplayRole || role == EditRole) {
        QFileInfo info = node->info;
        switch (index.column()) {
        case 0: return info.isRoot() ? info.absoluteFilePath() : info.fileName();
        case 1: return info.size();
        case 2: return d->iconProvider->type(info);
        case 3: return info.lastModified();
        default:
            qWarning("data: invalid display value column %d", index.column());
            return QVariant();
        }
    }

    if (role == DecorationRole && index.column() == 0)
        return d->iconProvider->icons(node->info);
    return QVariant();
}

/*!
  Sets the data for the model item \a index with the given \a role to
  the data referenced by the \a value. Returns true if successful;
  otherwise returns false.

*/

bool QDirModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid()) {
        qWarning("setData: the index is not valid");
        return false;
    }
    if (index.column() != 0) {
        qWarning("setData: wrong column %d", index.column());
        return false;
    }
    if (role != EditRole) {
        qWarning("setData: wrong role %d", role);
        return false;
    }
    if (!isEditable(index)) {
        qWarning("setData: the item is not editable");
        return false;
    }
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("setData: the node does not exist");
        return false;
    }
    QDir dir = node->info.dir();
    if (dir.rename(node->info.fileName(), value.toString())) {
        QModelIndex par = parent(index);
        QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(par.data());
        d->refresh(p);
        QModelIndex topLeft = this->index(0, 0, par);
        int rc = rowCount(par);
        int cc = columnCount(par);
        QModelIndex bottomRight = this->index(rc, cc, par);
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    qWarning("setData: file renaming failed");
    return false;
}

/*!
  Returns true if the \a parent model item has children; otherwise
  returns false.
*/

bool QDirModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true; // the drives
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    if (!p) {
        qWarning("hasChildren: valid index without data");
        return false;
    }
    return p->info.isDir() && rowCount(parent) > 0; // rowCount will lazily populate if needed
}

/*!
  Returns true if the model item \a index in the directory model is
  editable; otherwise returns false.

*/

bool QDirModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qWarning("isEditable: the index is invalid");
        return false;
    }
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("isEditable: the node does not exist");
        return false;
    }
    return (index.column() == 0) && node->info.isWritable();
}

/*!
  \fn bool QDirModel::isDragEnabled(const QModelIndex &index) const

  Returns true if the model item \a index in the directory model can
  be dragged; otherwise returns false.
*/

bool QDirModel::isDragEnabled(const QModelIndex &index) const
{
    return index.isValid();
}

/*!
  Returns true if the model item \a index in the directory model can
  receive items dropped on it; otherwise returns false.

*/

bool QDirModel::isDropEnabled(const QModelIndex &index) const
{
    return isDir(index) && isEditable(index);
}

/*!
  Returns true if the items in the directory model can be sorted;
  otherwise returns false.

*/

bool QDirModel::isSortable() const
{
    return true;
}

/*!
  Sort the model items in the \a column using the \a order given.
  The order is a value defined in \l Qt::SortOrder.

*/

void QDirModel::sort(int column, Qt::SortOrder order)
{
    int spec = (order == Qt::DescendingOrder ? QDir::Reversed : 0);

    switch (column) {
    case 0:
        spec |= QDir::Name;
        break;
    case 1:
        spec |= QDir::Size;
        break;
    case 2:
        // FIXME: we should sort on the type string
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

/*!
  \fn bool QDirModel::equal(const QModelIndex &first, const QModelIndex &second) const

  Returns true if the \a first model index is equal to the \a second
  index given.
*/

bool QDirModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    if (!(left.isValid() && right.isValid()))
        return left == right;
    QDirModelPrivate::QDirNode *l = static_cast<QDirModelPrivate::QDirNode*>(left.data());
    QDirModelPrivate::QDirNode *r = static_cast<QDirModelPrivate::QDirNode*>(right.data());
    return l->info.absoluteFilePath() == r->info.absoluteFilePath();
}

/*!
  \fn bool QDirModel::lessThan(const QModelIndex &first, const QModelIndex &second) const

  Returns true if the \a first model item is less than the \a second
  item given.

*/

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
    if (spec & QDir::DirsFirst) // col 2
        return !l->info.isDir();
    if (spec & QDir::Time) // col 3
        return l->info.lastModified() < r->info.lastModified();

    return QAbstractItemModel::lessThan(left, right);
}

/*!
  \fn bool QDirModel::canDecode(QMimeSource *source) const

  Returns true if the directory model can decode the \a source
  information; otherwise returns false.
*/

bool QDirModel::canDecode(QMimeSource *src) const
{
    return QUriDrag::canDecode(src);
}

/*!
    Returns true if this directory model (whose parent is \a parent),
    can decode drop event \a e.
*/

bool QDirModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    if (!parent.isValid()) {
        qWarning("decode: the parent index is invalid");
        return false;
    }
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    if (!p) {
        qWarning("decode: the node does not exist");
        return false;
    }
    QStringList files;    // FIXME: what about directories ?
    if (!QUriDrag::decodeLocalFiles(e, files))
        return false;
    d->savePersistentIndexes();
    emit rowsRemoved(parent, 0, rowCount(parent) - 1);
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
    default:
        return false;
    }
    p->children = d->children(p);
    d->restorePersistentIndexes();
    emit rowsInserted(parent, 0, rowCount(parent) - 1);
    return success;
}

/*!
  Returns the drag object for the list of model item \a indices
  originally dragged from the \a dragSource widget.
*/

QDragObject *QDirModel::dragObject(const QModelIndexList &indices, QWidget *dragSource)
{
    QList<QByteArray> uris;
    QList<QModelIndex>::const_iterator it = indices.begin();
    for (; it != indices.end(); ++it)
        uris.append(QUriDrag::localFileToUri(path(*it)));
    return new QUriDrag(uris, dragSource);
}

/*!
  Sets the \a provider of file icons for the directory model.

*/

void QDirModel::setIconProvider(QFileIconProvider *provider)
{
    d->iconProvider = provider;
}

/*!
  Returns the file icon provider for this directory model.
*/

QFileIconProvider *QDirModel::iconProvider() const
{
    return d->iconProvider;
}

/*!
  Sets the name \a filters for the directory model.

*/

void QDirModel::setNameFilters(const QStringList &filters)
{
//    qDebug("setNameFilters:");
    d->savePersistentIndexes(); // FIXME: this will rebuild the entire structure of the qdirmodel
    emit rowsRemoved(QModelIndex::Null, 0, rowCount(QModelIndex::Null) - 1);
    d->nameFilters = filters;
    d->root.children = d->children(0); // clear model
    d->restorePersistentIndexes();
    emit rowsInserted(QModelIndex::Null, 0, rowCount(QModelIndex::Null) - 1);
}

/*!
  Returns a list of filters applied to the names in the model.

*/

QStringList QDirModel::nameFilters() const
{
    return d->nameFilters;
}

/*!
  Sets the directory model's filter to that specified by \a spec.

  \sa QDir::FilterSpec
*/

void QDirModel::setFilter(int spec)
{
    d->savePersistentIndexes();
    emit rowsRemoved(QModelIndex::Null, 0, rowCount(QModelIndex::Null) - 1);
    d->filterSpec = spec;
    d->root.children = d->children(0);
    d->restorePersistentIndexes();
    emit rowsInserted(QModelIndex::Null, 0, rowCount(QModelIndex::Null) - 1);
}

/*!
  Returns the filter specification for the directory model.

  \sa QDir::FilterSpec

*/

QDir::FilterSpec QDirModel::filter() const
{
    return static_cast<QDir::FilterSpec>(d->filterSpec);
}

/*!
  Sets the directory model's sorting order to that specified by \a
  spec.

  \sa QDir::SortSpec
*/

void QDirModel::setSorting(int spec)
{
    QModelIndex parent;
    d->savePersistentIndexes();
    emit rowsRemoved(parent, 0, rowCount(parent) - 1);
    d->sortSpec = spec;
    d->root.children = d->children(0);
    d->restorePersistentIndexes();
    emit rowsInserted(parent, 0, rowCount(parent) - 1);
}

/*!
  Returns the sorting method used for the directory model.

  \sa QDir::SortSpec */

QDir::SortSpec QDirModel::sorting() const
{
    return static_cast<QDir::SortSpec>(d->sortSpec);
}

void QDirModel::setResolveSymlinks(bool enable)
{
    d->resolveSymlinks = enable;
}

bool QDirModel::resolveSymlinks() const
{
    return d->resolveSymlinks;
}

/*!
  Refreshes (rereads) the children of \a parent.

*/

void QDirModel::refresh(const QModelIndex &parent)
{
    d->savePersistentIndexes();
    emit rowsRemoved(parent, 0, rowCount(parent) - 1);
    d->refresh(static_cast<QDirModelPrivate::QDirNode*>(parent.data()));
    d->restorePersistentIndexes();
    emit rowsInserted(parent, 0, rowCount(parent) - 1);
}

/*!
    \overload

    Returns the model item index for the given \a path.
*/

QModelIndex QDirModel::index(const QString &path) const
{
//    qDebug("index: <<<");

    if (path.isEmpty() || path == d->rootPath()) // FIXME: slow
        return QModelIndex::Null;

//    qDebug("index: path '%s'", path.latin1());
    QChar sep = '/';//QDir::separator() // FIXME
    QStringList pth = QDir(path).absolutePath().split(sep, QString::SkipEmptyParts);

//     for (int j = 0; j < pth.count(); ++j)
// 	qDebug("index: path element '%s'", pth.at(j).latin1());

#ifdef Q_OS_WIN
    if (pth.isEmpty()) {
        qWarning("index: no path elements");
        return QModelIndex::Null;
    }
#endif

    QStringList entries;
    QDirModelPrivate::QDirNode *node;
    QModelIndex idx; // start with the root node

#ifndef Q_OS_WIN
    idx = index(0, 0, idx); // "/"
    node = &(d->root);
#endif

    for (int i = 0; i < pth.count(); ++i) {

//       qDebug("index: ---");

        if (pth.at(i).isEmpty()) {
            qWarning("index: path element %d was empty", i);
            continue;
        }

        node = idx.isValid() ? static_cast<QDirModelPrivate::QDirNode*>(idx.data()) : &(d->root);

#ifdef Q_OS_WIN
        if (!idx.isValid()) {
//            qDebug("index: idx is not valid");
            QFileInfoList info = d->rootChildren(); // the children could be drives
            entries.clear();
            for (int j = 0; j < info.count(); ++j)
                entries << QDir::cleanPath(info.at(j).absoluteFilePath()); // FIXME:
        } else
#endif
        {
//            qDebug("index: idx is valid");
            QString absPath = node->info.absoluteFilePath();
//            qDebug("index: getting entries in '%s'", absPath.latin1());
            // FIXME: workaround for problem in QDir
#if 1
            QDir dir(absPath);
            QFileInfoList info = dir.entryInfoList(d->nameFilters, d->filterSpec, d->sortSpec);
//            qDebug("index: fileinfo count %d", info.count());
            entries.clear();
            for (int fi = 0; fi < info.count(); ++fi)
                entries << info.at(fi).fileName();
#else
            entries = QDir(absPath).entryList(d->nameFilters, d->filterSpec, d->sortSpec);
//            qDebug("index: got %d entries", entries.count());
#endif
        }

//          for (int j = 0; j < entries.count(); ++j)
//              qDebug("index: entry %d '%s'", j, entries.at(j).latin1());

        QString entry = pth.at(i);
//        qDebug("index: path element %d is '%s'", i, entry.latin1());
        int r = entries.indexOf(entry);
//        qDebug("index: index of element is %d", r);
        idx = index(r, 0, idx); // will check row and lazily populate
        if (!idx.isValid()) {
            qWarning("index: path does not exist\n");
            return QModelIndex::Null;
        }
    }

//    qDebug("index: >>>\n");
    return idx;
}

/*!
  Returns the path of the item stored in the model under the
  \a index given.

*/

QString QDirModel::path(const QModelIndex &index) const
{
    QString pth;
    if (!index.isValid())
        pth = d->rootPath();
    else
        pth = fileInfo(index).absoluteFilePath();
    return QDir::cleanPath(pth);
}

/*!
  Returns the name of the item stored in the model under the
  \a index given.

*/

QString QDirModel::name(const QModelIndex &index) const
{
    if (!index.isValid())
        return d->rootName();
    QFileInfo info = fileInfo(index);
    if (info.isRoot())
        return info.absoluteFilePath();
    return info.fileName();
}

/*!
  Returns the iconset for the item stored in the model under the
  \a index given.
*/

QIconSet QDirModel::icons(const QModelIndex &index) const
{
    if (!index.isValid())
        return d->rootIcons();
    return d->iconProvider->icons(fileInfo(index));
}

/*!
  Returns the file information for the model item \a index.

*/

QFileInfo QDirModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qWarning("fileInfo: the index is invalid");
        return QFileInfo(); // FIXME: this returns current dir
    }

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("fileInfo: the node does not exist");
        return QFileInfo();
    }

    return node->info;
}

/*!
  Returns true if the model item \a index represents a directory;
  otherwise returns false.
*/

bool QDirModel::isDir(const QModelIndex &index) const
{
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    if (!node) {
        qWarning("isDir: the node does not exist");
        return false;
    }
    return node->info.isDir();
}

/*!
  Create a directory with the \a name in the \a parent model item.

*/

QModelIndex QDirModel::mkdir(const QModelIndex &parent, const QString &name)
{
    if (!parent.isValid()) {
        qWarning("mkdir: parent was invalid");
        return QModelIndex::Null;
    }

    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());

    if (!p) {
        qWarning("mkdir: parent node does not exist");
        return QModelIndex::Null;
    }

    d->savePersistentIndexes();

    QDir dir(p->info.absoluteFilePath());
    if (!dir.mkdir(name)) {
        d->restorePersistentIndexes();
        return QModelIndex::Null;
    }
    p->children = d->children(p);

    d->restorePersistentIndexes();

    int r = dir.entryList(d->nameFilters, d->filterSpec, d->sortSpec).indexOf(name);
    QModelIndex i = index(r, 0, parent); // return an invalid index
    emit rowsInserted(parent, r, 1);
    return i;
}

/*!
  Removes the directory corresponding to the model item \a index in the
  directory model, returning true if successful. If the directory
  cannot be removed, false is returned.

*/

bool QDirModel::rmdir(const QModelIndex &index)
{
    if (!index.isValid()) {
        qWarning("rmdir: the index is invalid");
        return false;
    }

    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());

    if (!n) {
        qWarning("rmdir: the node does not exist");
        return false;
    }

    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return false;
    }

    QDirModelPrivate::QDirNode *p = d->parent(n);
    if (!p) {
        qWarning("rmdir: the parent node does not exist");
        return false;
    }

    emit rowsRemoved(parent(index), index.row(), 1);
    d->savePersistentIndexes();

    QDir dir = p->info.dir(); // parent dir
    QString path = n->info.absoluteFilePath();
    if (!dir.rmdir(path)) {
        d->restorePersistentIndexes();
        return false;
    }
    p->children = d->children(p);

    d->restorePersistentIndexes();

    return true;
}

/*!
  Removes the model item \a index from the directory model, returning
  true if successful. If the item cannot be removed, false is returned.

 */

bool QDirModel::remove(const QModelIndex &index)
{
    if (!index.isValid()) {
        qWarning("remove: the index is invalid");
        return false;
    }

    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());

    if (!n) {
        qWarning("remove: the node does not exist");
        return false;
    }
    if (n->info.isDir()) {
        qWarning("remove: the node is a directory");
        return false;
    }

    QDirModelPrivate::QDirNode *p = d->parent(n);

    if (!p) {
        qWarning("remove: the parent node does not exist");
        return false;
    }

    emit rowsRemoved(parent(index), index.row(), 1);
    d->savePersistentIndexes();

    QDir dir = p->info.dir(); // parent dir
    QString path = n->info.absoluteFilePath();
    if (!dir.remove(path)) {
        d->restorePersistentIndexes();
        return false;
    }
    p->children = d->children(p);

    d->restorePersistentIndexes();

    return true;
}

/*
  The root node is never seen outside the model.
*/

void QDirModelPrivate::init(const QDir &directory)
{
    rootIsVirtual = false;

    root.parent = 0;
    root.info = QFileInfo(directory.absolutePath());
    root.children = children(0); // FIXME: ???

    filterSpec = directory.filter();
    sortSpec = directory.sorting();
    nameFilters = directory.nameFilters();

//     qDebug("init: %s", root.info.absoluteFilePath().latin1());
//     qDebug("init: %s", root.info.fileName().latin1());
}

QDirModelPrivate::QDirNode *QDirModelPrivate::node(int row, QDirNode *parent) const
{
    if (row < 0) {
        qWarning("node: row is negative");
	return 0;
    }

    bool isDir =  !parent || parent->info.isDir();
    QVector<QDirNode> *nodes = parent ? &(parent->children) : &(root.children);

    if (isDir && nodes->isEmpty()) {
//        qDebug("node: refreshing children");
	*nodes = children(parent);
    } else if (parent && parent->info.isSymLink() && d->resolveSymlinks) {
        qDebug("node: it's a link");
    }

    if (row >= nodes->count()) {
        qWarning("node: the row does not exist");
        return 0;
    }

    return const_cast<QDirNode*>(&nodes->at(row));
}

QDirModelPrivate::QDirNode *QDirModelPrivate::parent(QDirNode *child) const
{
    return child ? child->parent : 0;
}

QVector<QDirModelPrivate::QDirNode> QDirModelPrivate::children(QDirNode *parent) const
{
    QFileInfoList info;
    if (!parent) {
        info = rootChildren();
    } else if (parent->info.isDir()) {
        QDir dir = QDir(parent->info.filePath());
        info = dir.entryInfoList(nameFilters, filterSpec, sortSpec);
    }

//    qDebug("children: info count %d", info.count());

    QVector<QDirNode> nodes(info.count());
    for (int i = 0; i < (int)info.count(); ++i) {
        nodes[i].parent = parent;
        if (d->resolveSymlinks && info.at(i).isSymLink()) {
            QString link = info.at(i).readLink();
            //qDebug("children: it's a link: %s", link.latin1());
            if (link.at(link.size() - 1) == QDir::separator())
                link.chop(1);
            nodes[i].info = QFileInfo(link);
        } else {
            nodes[i].info = info.at(i);
        }
    }

    return nodes;
}

int QDirModelPrivate::idx(QDirNode *node) const
{
    if (!node) {
        qWarning("idx: the node is null");
        return -1;
    }

    if (node == &root)
        return 0;

    const QVector<QDirNode> children = node->parent ? node->parent->children : root.children;
    if (children.isEmpty()) {
        qWarning("idx: the parent's child list was empty");
	return -1;
    }

    const QDirNode *first = &(children.at(0));
    return (node - first);
}

void QDirModelPrivate::refresh(QDirNode *parent)
{
    QFileInfoList info;
    if (!parent) {
        info = rootChildren();
    } else if (parent->info.isDir()) {
        QDir dir = QDir(parent->info.filePath());
        info = dir.entryInfoList(nameFilters, filterSpec, sortSpec);
    }

    QVector<QDirNode> *nodes = parent ? &(parent->children) : &(root.children);
    if (nodes->count() != info.count()) {
        qWarning("refresh: the number of children has changed");
        nodes->resize(info.count());
    }

    for (int i = 0; i < (int)info.count(); ++i) {
        (*nodes)[i].parent = parent;
        if (d->resolveSymlinks && info.at(i).isSymLink()) {
            QString link = info.at(i).readLink();
            //qDebug("refresh: it's a link: %s", link.latin1());
            if (link.at(link.size() - 1) == QDir::separator())
                link.chop(1);
            (*nodes)[i].info = QFileInfo(link);
        } else {
            (*nodes)[i].info = info.at(i);
        }
        if (nodes->at(i).children.count() > 0)
            refresh(&(*nodes)[i]);
    }
}

void QDirModelPrivate::savePersistentIndexes()
{
    savedPaths.clear();
    for (int i = 0; i < persistentIndexes.count(); ++i) {
        QModelIndex idx = persistentIndexes.at(i)->index;
        QString pth = q->path(idx);
//        qDebug("savePersistentIndexes: saving '%s'", pth.latin1());
        savedPaths.append(pth);
        ++persistentIndexes.at(i)->ref; // save
        persistentIndexes[i]->index = QModelIndex::Null; // invalidated
    }
}

void QDirModelPrivate::restorePersistentIndexes()
{
    QList<QPersistentModelIndexData*> deleteList;
    for (int i = 0; i < persistentIndexes.count(); ++i) {
        QString pth = savedPaths.at(i);
        persistentIndexes[i]->index = q->index(pth);
        if (!--persistentIndexes.at(i)->ref)
            deleteList.append(persistentIndexes.at(i));
    }
    savedPaths.clear();
    while (!deleteList.isEmpty()) {
        QPersistentModelIndexData::destroy(deleteList.last());
        deleteList.removeLast();
    }
}

QFileInfoList QDirModelPrivate::rootChildren() const
{
    if (rootIsVirtual)
        return QDir::drives();
    QDir dir = QDir(root.info.absoluteFilePath());
    return dir.entryInfoList(nameFilters, filterSpec, sortSpec);
}
