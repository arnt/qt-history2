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
#include <qurl.h>
#include <qmime.h>
#include <qpair.h>
#include <qvector.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qstyle.h>
#include <qapplication.h>
#include <private/qabstractitemmodel_p.h>
#include <qdebug.h>

/*!
  \class QFileIconProvider

  \brief The QFileIconProvider class provides file icon for the QDirModel class.
*/

class QFileIconProviderPrivate
{
    Q_DECLARE_PUBLIC(QFileIconProvider)
public:
    QFileIconProviderPrivate();

    QIcon file;
    QIcon dir;
    QIcon driveHD;
    QIcon computer;
    QIcon fileLink;
    QIcon dirLink;

    QFileIconProvider *q_ptr;
};

QFileIconProviderPrivate::QFileIconProviderPrivate()
{
    file.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_FileIcon), Qt::SmallIconSize);
    fileLink.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_FileLinkIcon),
                       Qt::SmallIconSize);

    dir.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_DirOpenIcon),
                  Qt::SmallIconSize, QIcon::Normal, QIcon::On);
    dir.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_DirClosedIcon),
                  Qt::SmallIconSize, QIcon::Normal, QIcon::Off);
    dirLink.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_DirLinkIcon),
                      Qt::SmallIconSize, QIcon::Normal, QIcon::On);
    driveHD.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_DriveHDIcon),
                      Qt::SmallIconSize);
    computer.setPixmap(QApplication::style()->standardPixmap(QStyle::SP_ComputerIcon),
                       Qt::SmallIconSize);
}

/*!
  Constructs a file icon provider.

*/

QFileIconProvider::QFileIconProvider()
    : d_ptr(new QFileIconProviderPrivate)
{
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

QIcon QFileIconProvider::computerIcon() const
{
    return d_ptr->computer;
}

/*!
  Returns an icon for the file described by \a info.
*/

QIcon QFileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isRoot())
        return d_ptr->driveHD;
    if (info.isFile())
        if (info.isSymLink())
            return d_ptr->fileLink;
        else
            return d_ptr->file;
    if (info.isDir())
        if (info.isSymLink())
            return d_ptr->dirLink;
        else
            return d_ptr->dir;
    return QIcon();
}

/*!
  Returns the type of the file described by \a info.
*/

QString QFileIconProvider::type(const QFileInfo &info) const
{
    if (info.isRoot())
        return QObject::tr("Drive");
    if (info.isFile())
        return info.suffix() + " " + QObject::tr("File");
    if (info.isDir())
        return QObject::tr("Directory");
    if (info.isSymLink())
        return QObject::tr("Symbolic Link");
    return QObject::tr("Unknown");
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
        : resolveSymlinks(true),
          readOnly(true),
          iconProvider(&defaultProvider) {}

    void init();
    QDirNode *node(int row, QDirNode *parent) const;
    QDirNode *parent(QDirNode *child) const;
    QVector<QDirNode> children(QDirNode *parent) const;
    int idx(QDirNode *node) const;
    void refresh(QDirNode *parent);

    void savePersistentIndexes();
    void restorePersistentIndexes();

    QFileInfoList rootChildren() const;
    inline QIcon rootIcon() const { return iconProvider->computerIcon(); }
    inline QFileInfoList entryInfoList(const QString &path) const
        {
            const QDir dir(path);
            QFileInfoList fil = dir.entryInfoList(nameFilters, filters, sort);
            for (int i = 0; i < fil.count(); ++i) {
                QString fn = fil.at(i).fileName();
                if (fn == "." || fn == "..")
                    fil.removeAt(i--);
            }
            return fil;
        }
    inline QStringList entryList(const QString &path) const
        {
            const QDir dir(path);
            QStringList sl = dir.entryList(nameFilters, filters, sort);
            for (int i = 0; i < sl.count(); ++i) {
                QString fn = sl.at(i);
                if (fn == "." || fn == "..")
                    sl.removeAt(i--);
            }
            return sl;
        }

    mutable QDirNode root;
    bool resolveSymlinks;
    bool readOnly;

    QDir::Filters filters;
    QDir::SortFlags sort;
    QStringList nameFilters;

    QFileIconProvider *iconProvider;
    QFileIconProvider defaultProvider;

    QList< QPair<QString,int> > saved;
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

  QDirModel does not store file information internally or cache file data.

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
  The fileInfo(), isDir(), name(), and path() functions provide information
  about the underlying files and directories related to items in the model.
  Directories can be created and removed using mkdir(), rmdir(), and the
  model will be automatically updated to take the changes into account.

  \sa nameFilters(), setFilter(), filter(),
      \link model-view-programming.html Model/View Programming\endlink QListView QTreeView

*/

/*!
    Constructs a new directory model with the given \a parent.
    Only those files matching the \a nameFilters and the
    \a filters are included in the model. The sort order is given by the
    \a sort flags.
*/

QDirModel::QDirModel(const QStringList &nameFilters,
                     QDir::Filters filters,
                     QDir::SortFlags sort,
                     QObject *parent)
    : QAbstractItemModel(*new QDirModelPrivate, parent)
{
    // we always start with QDir::drives()
    d->nameFilters = nameFilters.isEmpty() ? QStringList("*") : nameFilters;
    d->filters = filters;
    d->sort = sort;
    d->root.parent = 0;
    d->root.info = QFileInfo();
    d->root.children = d->children(0);
}

/*!
  Constructs a directory model with the given \a parent.
*/

QDirModel::QDirModel(QObject *parent)
    : QAbstractItemModel(*new QDirModelPrivate, parent)
{
    d->init();
}

/*!
    \internal
*/
QDirModel::QDirModel(QDirModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    d->init();
}

/*!
  Destroys this directory model.
*/

QDirModel::~QDirModel()
{

}

/*!
  Returns the model item index for the item in the \a parent with the
  given \a row and \a column.

*/

QModelIndex QDirModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || column >= 4 || row < 0 || row >= rowCount(parent)) // does lazy population
        return QModelIndex();

    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    QDirModelPrivate::QDirNode *n = d->node(row, p);
    Q_ASSERT(n);

    return createIndex(row, column, n);
}

/*!
  Return the parent of the given \a child model item.
*/

QModelIndex QDirModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
	return QModelIndex();

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(child.data());
    Q_ASSERT(node);

    QDirModelPrivate::QDirNode *par = d->parent(node);
    if (!par)
	return QModelIndex(); // parent is the root node

    int r = d->idx(par);
    Q_ASSERT(r >= 0);
    return createIndex(r, 0, par);
}

/*!
  Returns the number of rows in the \a parent model item.

*/

int QDirModel::rowCount(const QModelIndex &parent) const
{
    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());

    bool isDir = !p || p->info.isDir(); // no node pointer means that it is the root
    QVector<QDirModelPrivate::QDirNode> *nodes = p ? &(p->children) : &(d->root.children);

    if (isDir && nodes->isEmpty())
	*nodes = d->children(p); // lazy population
    return nodes->count();
}

/*!
  Returns the number of columns in the \a parent model item.

*/

int QDirModel::columnCount(const QModelIndex &) const
{
    return 4;
}

/*!
  Returns the data for the model item \a index with the given \a role.

*/

QVariant QDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(node);

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

    if (index.column() == 0) {
        if (role == FileIconRole)
            return fileIcon(index);
        if (role == FilePathRole)
            return filePath(index);
        if (role == FileNameRole)
            return fileName(index);
    }
    return QVariant();
}

/*!
  Sets the data for the model item \a index with the given \a role to
  the data referenced by the \a value. Returns true if successful;
  otherwise returns false.

*/

bool QDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.column() != 0
        || (flags(index) & ItemIsEditable) == 0 || role != EditRole)
        return false;

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(node);
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

    return false;
}

/*!
  Returns the data stored under the given \a role for the specified \a section
  of the header with the given \a orientation.
*/

QVariant QDirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role != DisplayRole)
            return QVariant();
	switch (section) {
        case 0: return "Name";
        case 1: return "Size";
        case 2: return "Type";
        case 3: return "Modified";
        default: return QVariant();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
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
    Q_ASSERT(p);
    return p->info.isDir() && rowCount(parent) > 0; // rowCount will lazily populate if needed
}

/*!
  Returns the item flags for the given \a index in the model.

  \sa QAbstractItemModel::flags() QAbstractItemModel::ItemFlags
*/
QAbstractItemModel::ItemFlags QDirModel::flags(const QModelIndex &index) const
{
    QAbstractItemModel::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;
    flags |= QAbstractItemModel::ItemIsDragEnabled;
    if (d->readOnly)
        return flags;
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(node);
    if ((index.column() == 0) && node->info.isWritable()) {
        flags |= QAbstractItemModel::ItemIsEditable;
        if (fileInfo(index).isDir()) // is directory and is editable
            flags |= QAbstractItemModel::ItemIsDropEnabled;
    }
    return flags;
}

/*!
  Sort the model items in the \a column using the \a order given.
  The order is a value defined in \l Qt::SortOrder.
*/

void QDirModel::sort(int column, Qt::SortOrder order)
{
    QDir::SortFlags sort = 0;
    if(order == Qt::DescendingOrder)
        sort |= QDir::Reversed;

    switch (column) {
    case 0:
        sort |= QDir::Name;
        break;
    case 1:
        sort |= QDir::Size;
        break;
    case 2:
        // FIXME: we should sort on the type string
        sort |= QDir::DirsFirst;
        break;
    case 3:
        sort |= QDir::Time;
        break;
    default:
        break;
    };

    setSorting(sort);
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/

QStringList QDirModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

/*!
    Returns an object that contains a serialized description of the specified
    \a indexes. The format used to describe the items corresponding to the
    indexes is obtained from the mimeTypes() function.

    If the list of indexes is empty, 0 is returned rather than a serialized
    empty list.
*/

QMimeData *QDirModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QList<QModelIndex>::const_iterator it = indexes.begin();
    for (; it != indexes.end(); ++it)
        if ((*it).column() == 0)
            urls << QUrl::fromLocalFile(filePath(*it));
    QMimeData *data = new QMimeData();
    data->setUrls(urls);
    return data;
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action over the row in the model specified by the \a row
    and the \a parent index.

    \sa supportedDropActions()
*/

bool QDirModel::dropMimeData(const QMimeData *data, QDrag::DropAction action,
                             int row, const QModelIndex &parent)
{
    Q_UNUSED(row);
    if (!parent.isValid() || isReadOnly())
        return false;

    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    Q_ASSERT(p);

    QList<QUrl> urls = data->urls();
    d->savePersistentIndexes();
    emit rowsAboutToBeRemoved(parent, 0, rowCount(parent) - 1);
    bool success = true;
    QString to = filePath(parent) + QDir::separator();
    QList<QUrl>::const_iterator it = urls.begin();

    switch (action) {
    case QDrag::CopyAction:
        for (; it != urls.end(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case QDrag::LinkAction:
        for (; it != urls.end(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case QDrag::MoveAction:
        for (; it != urls.end(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName())
                      && QFile::remove(path) && success;
        }
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
  Returns the drop actions supported by this model.

  \sa QDrag::DropActions
*/

QDrag::DropActions QDirModel::supportedDropActions() const
{
    return QDrag::CopyAction | QDrag::MoveAction; // FIXME: LinkAction is not supported yet
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
    d->savePersistentIndexes(); // FIXME: this will rebuild the entire structure of the qdirmodel
    emit rowsAboutToBeRemoved(QModelIndex(), 0, rowCount(QModelIndex()) - 1);
    d->nameFilters = filters;
    d->root.children = d->children(0); // clear model
    d->restorePersistentIndexes();
    emit rowsInserted(QModelIndex(), 0, rowCount(QModelIndex()) - 1);
}

/*!
  Returns a list of filters applied to the names in the model.
*/

QStringList QDirModel::nameFilters() const
{
    return d->nameFilters;
}

/*!
  Sets the directory model's filter to that specified by \a filters.

  \sa QDir::Filters
*/

void QDirModel::setFilter(QDir::Filters filters)
{
    d->savePersistentIndexes();
    emit rowsAboutToBeRemoved(QModelIndex(), 0, rowCount(QModelIndex()) - 1);
    d->filters = filters;
    d->root.children = d->children(0);
    d->restorePersistentIndexes();
    emit rowsInserted(QModelIndex(), 0, rowCount(QModelIndex()) - 1);
}

/*!
  Returns the filter specification for the directory model.

  \sa QDir::Filters
*/

QDir::Filters QDirModel::filter() const
{
    return d->filters;
}

/*!
  Sets the directory model's sorting order to that specified by \a sort.

  \sa QDir::SortFlags
*/

void QDirModel::setSorting(QDir::SortFlags sort)
{
    QModelIndex parent;
    d->savePersistentIndexes();
    emit rowsAboutToBeRemoved(parent, 0, rowCount(parent) - 1);
    d->sort = sort;
    d->root.children = d->children(0);
    d->restorePersistentIndexes();
    emit rowsInserted(parent, 0, rowCount(parent) - 1);
}

/*!
  Returns the sorting method used for the directory model.

  \sa QDir::SortFlags */

QDir::SortFlags QDirModel::sorting() const
{
    return d->sort;
}

/*!
    \property QDirModel::resolveSymlinks
    \brief Whether the directory model should resolve symbolic links

    This is only relevant on operating systems that support symbolic
    links.
*/
void QDirModel::setResolveSymlinks(bool enable)
{
    d->resolveSymlinks = enable;
}

bool QDirModel::resolveSymlinks() const
{
    return d->resolveSymlinks;
}

/*!
  \property QDirModel::readOnly
  \brief Whether the directory model allows writing to the file system

  If this property is set to false, the directory model will allow renaming, copying
  and deleting of files and directories.

  This property is true by default
*/

void QDirModel::setReadOnly(bool enable)
{
    d->readOnly = enable;
}

bool QDirModel::isReadOnly() const
{
    return d->readOnly;
}

/*!
  Refreshes (rereads) the children of \a parent.

*/

void QDirModel::refresh(const QModelIndex &parent)
{
    d->savePersistentIndexes();
    emit rowsAboutToBeRemoved(parent, 0, rowCount(parent) - 1);
    d->refresh(static_cast<QDirModelPrivate::QDirNode*>(parent.data()));
    d->restorePersistentIndexes();
    emit rowsInserted(parent, 0, rowCount(parent) - 1);
}

/*!
    \overload

    Returns the model item index for the given \a path.
*/

QModelIndex QDirModel::index(const QString &path, int column) const
{
    if (path.isEmpty() || path == QObject::tr("My Computer"))// the root path
        return QModelIndex();

    const QChar sep = '/';
    QStringList pth = QDir(path).absolutePath().split(sep, QString::SkipEmptyParts);

#ifdef Q_OS_WIN
    if (pth.isEmpty()) {
        qWarning("index: no path elements");
        return QModelIndex();
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

        if (pth.at(i).isEmpty()) {
            qWarning("index: path element %d was empty", i);
            continue;
        }
        node = idx.isValid() ? static_cast<QDirModelPrivate::QDirNode*>(idx.data()) : &(d->root);

#ifdef Q_OS_WIN
        if (!idx.isValid()) {
            QFileInfoList info = d->rootChildren(); // the children could be drives
            entries.clear();
            for (int j = 0; j < info.count(); ++j)
                entries << QDir::cleanPath(info.at(j).absoluteFilePath());
        } else
#endif
        {
            QString absPath = node->info.absoluteFilePath();

            QFileInfoList info = d->entryInfoList(absPath);
            entries.clear();
            for (int fi = 0; fi < info.count(); ++fi)
                entries << info.at(fi).fileName();
        }

        QString entry = pth.at(i);
        int r = entries.indexOf(entry);
        idx = index(r, column, idx); // will check row and lazily populate
        if (!idx.isValid()) {
            QString converted = QDir::convertSeparators(path);
            QString warn = QString("index: the path '%1' could not be found\n").arg(converted);
            qWarning(warn.toLatin1().constData());
            return QModelIndex();
        }
    }

    return idx;
}

/*!
  Returns true if the model item \a index represents a directory;
  otherwise returns false.
*/

bool QDirModel::isDir(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(node);
    return node->info.isDir();
}

/*!
  Create a directory with the \a name in the \a parent model item.

*/

QModelIndex QDirModel::mkdir(const QModelIndex &parent, const QString &name)
{
    if (!parent.isValid())
        return QModelIndex();

    QDirModelPrivate::QDirNode *p = static_cast<QDirModelPrivate::QDirNode*>(parent.data());
    Q_ASSERT(p);

    d->savePersistentIndexes();

    QString path = p->info.absoluteFilePath();
    QDir dir(path);
    if (!dir.mkdir(name)) {
        d->restorePersistentIndexes();
        return QModelIndex();
    }
    p->children = d->children(p);

    d->restorePersistentIndexes();

    QStringList entryList = d->entryList(path);
    int r = entryList.indexOf(name);
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
    if (!index.isValid())
        return false;

    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(n);

    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return false;
    }

    QDirModelPrivate::QDirNode *p = d->parent(n);
    Q_ASSERT(p);

    emit rowsAboutToBeRemoved(parent(index), index.row(), 1);
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
    if (!index.isValid())
        return false;

    QDirModelPrivate::QDirNode *n = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(n);

    if (n->info.isDir())
        return false;

    QDirModelPrivate::QDirNode *p = d->parent(n);
    Q_ASSERT(p);

    emit rowsAboutToBeRemoved(parent(index), index.row(), index.row());
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

/*!
  Returns the path of the item stored in the model under the
  \a index given.

*/

QString QDirModel::filePath(const QModelIndex &index) const
{
    if (index.isValid()) {
        QFileInfo fi = fileInfo(index);
        if (d->resolveSymlinks && fi.isSymLink()) {
            QString link = fi.readLink();
            if (link.at(link.size() - 1) == QDir::separator())
                link.chop(1);
            return QDir::cleanPath(link);
        }
        return QDir::cleanPath(fi.absoluteFilePath());
    }
    return QString(); // root path
}

/*!
  Returns the name of the item stored in the model under the
  \a index given.

*/

QString QDirModel::fileName(const QModelIndex &index) const
{
    if (!index.isValid())
        return QObject::tr("My Computer");
    QFileInfo info = fileInfo(index);
    if (info.isRoot())
        return info.absoluteFilePath();
    return info.fileName();
}

/*!
  Returns the icons for the item stored in the model under the given
  \a index.
*/

QIcon QDirModel::fileIcon(const QModelIndex &index) const
{
    if (!index.isValid())
        return d->rootIcon();
    return d->iconProvider->icon(fileInfo(index));
}

/*!
  Returns the file information for the model item \a index.

*/

QFileInfo QDirModel::fileInfo(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QDirModelPrivate::QDirNode *node = static_cast<QDirModelPrivate::QDirNode*>(index.data());
    Q_ASSERT(node);

    return node->info;
}

/*
  The root node is never seen outside the model.
*/

void QDirModelPrivate::init()
{
    filters = QDir::TypeMask;
    sort = QDir::Name;
    nameFilters << "*";
    root.parent = 0;
    root.info = QFileInfo();
    root.children = children(0);
}

QDirModelPrivate::QDirNode *QDirModelPrivate::node(int row, QDirNode *parent) const
{
    if (row < 0)
	return 0;

    bool isDir =  !parent || parent->info.isDir();
    QVector<QDirNode> *nodes = parent ? &(parent->children) : &(root.children);

    if (isDir && nodes->isEmpty())
	*nodes = children(parent); // children will also resolve symlinks

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
        if (d->resolveSymlinks && parent->info.isSymLink()) {
            QString link = parent->info.readLink();
            if (link.at(link.size() - 1) == QDir::separator())
                link.chop(1);
            info = entryInfoList(link);
        } else {
            info = entryInfoList(parent->info.filePath());
        }
    }

    QVector<QDirNode> nodes(info.count());
    for (int i = 0; i < info.count(); ++i) {
        nodes[i].parent = parent;
        nodes[i].info = info.at(i);
    }

    return nodes;
}

int QDirModelPrivate::idx(QDirNode *node) const
{
    Q_ASSERT(node);

    if (node == &root)
        return 0;

    const QVector<QDirNode> children = node->parent ? node->parent->children : root.children;
    Q_ASSERT(children.count() > 0);

    const QDirNode *first = &(children.at(0));
    return (node - first);
}

void QDirModelPrivate::refresh(QDirNode *parent)
{
    QFileInfoList info;
    if (!parent)
        info = rootChildren();
    else if (parent->info.isDir())
        info = entryInfoList(parent->info.filePath());

    QVector<QDirNode> *nodes = parent ? &(parent->children) : &(root.children);
    if (nodes->count() != info.count())
        nodes->resize(info.count());

    for (int i = 0; i < (int)info.count(); ++i) {
        (*nodes)[i].parent = parent;
        if (d->resolveSymlinks && info.at(i).isSymLink()) {
            QString link = info.at(i).readLink();
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
    saved.clear();
    for (int i = 0; i < persistentIndexes.count(); ++i) {
        QModelIndex idx = persistentIndexes.at(i)->index;
        saved.append(qMakePair(q->filePath(idx), idx.column()));
        ++persistentIndexes.at(i)->ref; // save
        persistentIndexes[i]->index = QModelIndex(); // invalidated
    }
}

void QDirModelPrivate::restorePersistentIndexes()
{
    QList<QPersistentModelIndexData*> deleteList;
    for (int i = 0; i < persistentIndexes.count(); ++i) {
        persistentIndexes[i]->index = q->index(saved.at(i).first,
                                               saved.at(i).second);
        if (!--persistentIndexes.at(i)->ref)
            deleteList.append(persistentIndexes.at(i));
    }
    saved.clear();
    while (!deleteList.isEmpty()) {
        QPersistentModelIndexData::destroy(deleteList.last());
        deleteList.removeLast();
    }
}

QFileInfoList QDirModelPrivate::rootChildren() const
{
    return QDir::drives();
}
