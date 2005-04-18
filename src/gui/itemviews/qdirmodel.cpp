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

/*!
  \enum QFileIconProvider::IconType
  \value Computer
  \value Desktop
  \value Trashcan
  \value Network
  \value Drive
  \value Folder
  \value File
*/

/*!
    \enum QDirModel::Roles
    \value FileIconRole
    \value FilePathRole
    \value FileNameRole
*/

class QFileIconProviderPrivate
{
    Q_DECLARE_PUBLIC(QFileIconProvider)
public:
    QFileIconProviderPrivate();

    QIcon file;
    QIcon fileLink;
    QIcon directory;
    QIcon directoryLink;
    QIcon harddisk;
    QIcon floppy;
    QIcon cdrom;
    QIcon ram;
    QIcon network;
    QIcon computer;
    QIcon desktop;
    QIcon trashcan;
    QIcon generic;

    QFileIconProvider *q_ptr;
};

QFileIconProviderPrivate::QFileIconProviderPrivate()
{
    QStyle *style = QApplication::style();

    file = QIcon(style->standardPixmap(QStyle::SP_FileIcon));
    fileLink = QIcon(style->standardPixmap(QStyle::SP_FileLinkIcon));

    directory = QIcon(style->standardPixmap(QStyle::SP_DirClosedIcon));
    directory.addPixmap(style->standardPixmap(QStyle::SP_DirOpenIcon), QIcon::Normal, QIcon::On);
    directoryLink = QIcon(style->standardPixmap(QStyle::SP_DirLinkIcon));

    harddisk = QIcon(style->standardPixmap(QStyle::SP_DriveHDIcon));
    floppy = QIcon(style->standardPixmap(QStyle::SP_DriveFDIcon));
    cdrom = QIcon(style->standardPixmap(QStyle::SP_DriveCDIcon));
    generic = ram = harddisk; // FIXME
    network = QIcon(style->standardPixmap(QStyle::SP_DriveNetIcon));
    computer = QIcon(style->standardPixmap(QStyle::SP_ComputerIcon));
    desktop = QIcon(style->standardPixmap(QStyle::SP_DesktopIcon));
    trashcan = QIcon(style->standardPixmap(QStyle::SP_TrashIcon));
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
  Returns an icon set for the given \a type.
*/

QIcon QFileIconProvider::icon(IconType type) const
{
    switch (type) {
    case Computer:
        return d_ptr->computer;
    case Desktop:
        return d_ptr->desktop;
    case Trashcan:
        return d_ptr->trashcan;
    case Network:
        return d_ptr->network;
    case Drive:
        return d_ptr->generic;
    case Folder:
        return d_ptr->directory;
    case File:
        return d_ptr->file;
    default:
        break;
    };
    return QIcon();
}

/*!
  Returns an icon for the file described by \a info.
*/

QIcon QFileIconProvider::icon(const QFileInfo &info) const
{

    if (info.isRoot())
#ifdef Q_OS_WIN
    {
        uint type = DRIVE_UNKNOWN;
	QT_WA({ type = GetDriveTypeW((wchar_t *)info.absoluteFilePath().utf16()); },
        { type = GetDriveTypeA(info.absoluteFilePath().toLocal8Bit()); });
	switch (type) {
	case DRIVE_REMOVABLE:
            return d_ptr->floppy;
	case DRIVE_FIXED:
            return d_ptr->harddisk;
	case DRIVE_REMOTE:
            return d_ptr->network;
	case DRIVE_CDROM:
            return d_ptr->cdrom;
	case DRIVE_RAMDISK:
            return d_ptr->ram;
	case DRIVE_UNKNOWN:
            return d_ptr->generic;
	case DRIVE_NO_ROOT_DIR:
            return d_ptr->generic;
	}
    }
#else
  return d_ptr->generic;
#endif
  if (info.isFile())
    if (info.isSymLink())
      return d_ptr->fileLink;
    else
      return d_ptr->file;
  if (info.isDir())
    if (info.isSymLink())
      return d_ptr->directoryLink;
    else
      return d_ptr->directory;
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

#define Q_DIRMODEL_CACHED

class QDirModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QDirModel)
public:
    struct QDirNode
    {
        QDirNode *parent;
        QFileInfo info;
        mutable QVector<QDirNode> children;
        mutable bool populated; // have we read the children
#ifndef Q_DIRMODEL_CACHED
        inline qint64 size() const { return info.size(); }
        inline QDateTime lastModified() const { return info.lastModified(); }
#else
        inline qint64 size() const { return cached_size; }
        inline QDateTime lastModified() const { return cached_modified; }
        qint64 cached_size;
        QDateTime cached_modified;
#endif
    };

    QDirModelPrivate()
        : resolveSymlinks(true),
          readOnly(true),
          lazyChildCount(false),
          iconProvider(&defaultProvider) {}

    void init();
    QDirNode *node(int row, QDirNode *parent) const;
    QDirNode *parent(QDirNode *child) const;
    QVector<QDirNode> children(QDirNode *parent) const;
    int idx(QDirNode *node) const;
    void refresh(QDirNode *parent);

    void savePersistentIndexes();
    void restorePersistentIndexes();

    inline QIcon rootIcon() const {
        return iconProvider->icon(QFileIconProvider::Computer);
    }

    inline QFileInfoList entryInfoList(const QString &path) const {
        const QDir dir(path);
        QFileInfoList fil = dir.entryInfoList(nameFilters, filters, sort);
        for (int i = 0; i < fil.count(); ++i) {
            QString fn = fil.at(i).fileName();
            if (fn == "." || fn == "..")
                fil.removeAt(i--);
        }
        return fil;
    }

    inline QStringList entryList(const QString &path) const {
        const QDir dir(path);
        QStringList sl = dir.entryList(nameFilters, filters, sort);
        for (int i = 0; i < sl.count(); ++i) {
            QString fn = sl.at(i);
            if (fn == "." || fn == "..")
                sl.removeAt(i--);
        }
        return sl;
    }

    inline void populate(QDirNode *parent) const {
        Q_ASSERT(parent);
        parent->children = children(parent);
        parent->populated = true;
    }

    inline void clear(QDirNode *parent) const {
        Q_ASSERT(parent);
        parent->children = children(0);
        parent->populated = false;
    }

    mutable QDirNode root;
    bool resolveSymlinks;
    bool readOnly;
    bool lazyChildCount;

    QDir::Filters filters;
    QDir::SortFlags sort;
    QStringList nameFilters;

    QFileIconProvider *iconProvider;
    QFileIconProvider defaultProvider;

    QList< QPair<QString,int> > saved;
};

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
  is usually constructed with a parent object:

  \quotefromfile snippets/shareddirmodel/main.cpp
  \skipto QDirModel *model
  \printuntil QDirModel *model

  A tree view can be used to display the contents of the model

  \skipto QTreeView *tree
  \printuntil tree->setModel(

  and the contents of a particular directory can be displayed by
  setting the tree view's root index:

  \printuntil tree->setRootIndex(

  The view's root index can be used to control how much of a
  hierarchical model is displayed. QDirModel provides a convenience
  function that returns a suitable model index for a path to a
  directory within the model.

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
    Q_D(QDirModel);
    // we always start with QDir::drives()
    d->nameFilters = nameFilters.isEmpty() ? QStringList("*") : nameFilters;
    d->filters = filters;
    d->sort = sort;
    d->root.parent = 0;
    d->root.info = QFileInfo();
    d->clear(&d->root);
}

/*!
  Constructs a directory model with the given \a parent.
*/

QDirModel::QDirModel(QObject *parent)
    : QAbstractItemModel(*new QDirModelPrivate, parent)
{
    Q_D(QDirModel);
    d->init();
}

/*!
    \internal
*/
QDirModel::QDirModel(QDirModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(QDirModel);
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
    Q_D(const QDirModel);
    if (column < 0 || column >= 4 || row < 0 || row >= rowCount(parent)) // does lazy population
        return QModelIndex();

    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(parent.internalPointer());
    QDirModelPrivate::QDirNode *n = d->node(row, p);
    Q_ASSERT(n);

    return createIndex(row, column, n);
}

/*!
  Return the parent of the given \a child model item.
*/

QModelIndex QDirModel::parent(const QModelIndex &child) const
{
    Q_D(const QDirModel);
    if (!child.isValid())
	return QModelIndex();

    QDirModelPrivate::QDirNode *node =
        static_cast<QDirModelPrivate::QDirNode*>(child.internalPointer());
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
    Q_D(const QDirModel);
    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(parent.internalPointer());

    bool isDir = !p || p->info.isDir(); // no node pointer means that it is the root
    if (!p) p = &(d->root);
    if (isDir && p->children.isEmpty()) // lazy population
        d->populate(p);
    return p->children.count();
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
    Q_D(const QDirModel);
    if (!index.isValid())
        return QVariant();

    QDirModelPrivate::QDirNode *node =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(node);

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
	    case 0:
                if (node->info.isRoot()) {
                    QString name = node->info.absoluteFilePath();
#ifdef Q_OS_WIN
                    if (name.at(0) == '/') // UNC host
                        return node->info.fileName();
                    if (name.at(name.length() - 1) == '/')
                        name.chop(1);
#endif
                    return name;
                }
                return node->info.fileName();
        case 1: return node->size();
        case 2: return d->iconProvider->type(node->info);
        case 3: return node->lastModified();
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
    Q_D(QDirModel);
    if (!index.isValid() || index.column() != 0
        || (flags(index) & Qt::ItemIsEditable) == 0 || role != Qt::EditRole)
        return false;

    QDirModelPrivate::QDirNode *node =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(node);
    QDir dir = node->info.dir();
    if (dir.rename(node->info.fileName(), value.toString())) {
        QModelIndex par = parent(index);
        QDirModelPrivate::QDirNode *p =
            static_cast<QDirModelPrivate::QDirNode*>(par.internalPointer());
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
        if (role != Qt::DisplayRole)
            return QVariant();
	switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Size");
        case 2: return tr("Type");
        case 3: return tr("Modified");
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
    Q_D(const QDirModel);
    if (!parent.isValid()) // the invalid index is the "My Computer" item
        return true; // the drives
    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(parent.internalPointer());
    Q_ASSERT(p);
    if (d->lazyChildCount) { // optimization that only checks for children if the node has been populated
        if (p->populated)
            return rowCount(parent) > 0; // rowCount will lazily populate if needed
        return p->info.isDir();
    }
    return p->info.isDir() && rowCount(parent) > 0;
}

/*!
  Returns the item flags for the given \a index in the model.

  \sa Qt::ItemFlags
*/
Qt::ItemFlags QDirModel::flags(const QModelIndex &index) const
{
    Q_D(const QDirModel);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;
    flags |= Qt::ItemIsDragEnabled;
    if (d->readOnly)
        return flags;
    QDirModelPrivate::QDirNode *node =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(node);
    if ((index.column() == 0) && node->info.isWritable()) {
        flags |= Qt::ItemIsEditable;
        if (fileInfo(index).isDir()) // is directory and is editable
            flags |= Qt::ItemIsDropEnabled;
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
    if (order == Qt::DescendingOrder)
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
    }

    setSorting(sort);
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/

QStringList QDirModel::mimeTypes() const
{
    return QStringList("text/uri-list");
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

bool QDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_D(QDirModel);
    if (!parent.isValid() || isReadOnly())
        return false;

    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(parent.internalPointer());
    Q_ASSERT(p);
    
    bool success = true;
    QString to = filePath(parent) + QDir::separator();

    QList<QUrl> urls = data->urls();
    QList<QUrl>::const_iterator it = urls.begin();

    int last = row + urls.count() - 1;
    beginInsertRows(parent, row, last);

    switch (action) {
    case Qt::CopyAction:
        for (; it != urls.end(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::LinkAction:
        for (; it != urls.end(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::MoveAction:
        for (; it != urls.end(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName())
                      && QFile::remove(path) && success;
        }
        break;
    default:
        return false;
    }

    d->populate(p);
    endInsertRows();

    return success;
}

/*!
  Returns the drop actions supported by this model.

  \sa Qt::DropActions
*/

Qt::DropActions QDirModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction; // FIXME: LinkAction is not supported yet
}

/*!
  Sets the \a provider of file icons for the directory model.

*/

void QDirModel::setIconProvider(QFileIconProvider *provider)
{
    Q_D(QDirModel);
    d->iconProvider = provider;
}

/*!
  Returns the file icon provider for this directory model.
*/

QFileIconProvider *QDirModel::iconProvider() const
{
    Q_D(const QDirModel);
    return d->iconProvider;
}

/*!
  Sets the name \a filters for the directory model.
*/

void QDirModel::setNameFilters(const QStringList &filters)
{
    Q_D(QDirModel);

    d->savePersistentIndexes();
    beginRemoveRows(QModelIndex(), 0, rowCount(QModelIndex()) - 1);
    
    d->nameFilters = filters;
    d->clear(&d->root); // clear model

    endRemoveRows();
    d->restorePersistentIndexes();
}

/*!
  Returns a list of filters applied to the names in the model.
*/

QStringList QDirModel::nameFilters() const
{
    Q_D(const QDirModel);
    return d->nameFilters;
}

/*!
  Sets the directory model's filter to that specified by \a filters.

  \sa QDir::Filters
*/

void QDirModel::setFilter(QDir::Filters filters)
{
    Q_D(QDirModel);
    
    d->savePersistentIndexes();
    beginRemoveRows(QModelIndex(), 0, rowCount(QModelIndex()) - 1);

    d->filters = filters;
    d->clear(&d->root); // clear model

    endRemoveRows();
    d->restorePersistentIndexes();
}

/*!
  Returns the filter specification for the directory model.

  \sa QDir::Filters
*/

QDir::Filters QDirModel::filter() const
{
    Q_D(const QDirModel);
    return d->filters;
}

/*!
  Sets the directory model's sorting order to that specified by \a sort.

  \sa QDir::SortFlags
*/

void QDirModel::setSorting(QDir::SortFlags sort)
{
    Q_D(QDirModel);

    d->savePersistentIndexes();
    beginRemoveRows(QModelIndex(), 0, rowCount(QModelIndex()) - 1);

    d->sort = sort;
    d->clear(&d->root); // clear model

    endRemoveRows();
    d->restorePersistentIndexes();
}

/*!
  Returns the sorting method used for the directory model.

  \sa QDir::SortFlags */

QDir::SortFlags QDirModel::sorting() const
{
    Q_D(const QDirModel);
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
    Q_D(QDirModel);
    d->resolveSymlinks = enable;
}

bool QDirModel::resolveSymlinks() const
{
    Q_D(const QDirModel);
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
    Q_D(QDirModel);
    d->readOnly = enable;
}

bool QDirModel::isReadOnly() const
{
    Q_D(const QDirModel);
    return d->readOnly;
}

/*!
  \property QDirModel::lazyChildCount
  \brief Whether the directory model optimizes the hasChildren function
  to only check if the item is a directory.

  If this property is set to true, the directory model will make sure that a directory
  actually containes any files before reporting that it has children.
  Otherwise the directory model will report that an item has children if the item
  is a directory.

  This property is false by default
*/

void QDirModel::setLazyChildCount(bool enable)
{
    Q_D(QDirModel);
    d->lazyChildCount = enable;
}

bool QDirModel::lazyChildCount() const
{
    Q_D(const QDirModel);
    return d->lazyChildCount;
}

/*!
  Refreshes (rereads) the children of \a parent.
*/

void QDirModel::refresh(const QModelIndex &parent)
{
    Q_D(QDirModel);

    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(parent.internalPointer());
    QDirModelPrivate::QDirNode *n = (p ? p : &(d->root));

    d->savePersistentIndexes();
    beginRemoveRows(parent, 0, n->children.count() - 1);

    d->refresh(p); // if (p == 0) it reads the drives

    endRemoveRows();
    d->restorePersistentIndexes();
}

/*!
    \overload

    Returns the model item index for the given \a path.
*/

QModelIndex QDirModel::index(const QString &path, int column) const
{
    Q_D(const QDirModel);

    if (path.isEmpty() || path == QObject::tr("My Computer"))
        return QModelIndex();

    QString absolutePath = QDir(path).absolutePath();
    QStringList pathElements = absolutePath.split(QChar('/'), QString::SkipEmptyParts);
    if (pathElements.isEmpty() || !QFileInfo(path).exists())
        return QModelIndex();

    QModelIndex idx; // start with "My Computer"
#ifdef Q_OS_WIN
    if (absolutePath.startsWith("//")) { // UNC path
        QString host = pathElements.first();
        int r = 0;
        for (; r < d->root.children.count(); ++r) {
            if (d->root.children.at(r).info.fileName() == host)
                break;
        }
        if (r >= d->root.children.count()) {
            QFileInfo info("//" + host);
            QDirModelPrivate::QDirNode node;
            node.parent = 0;
            node.info = info; 
            node.populated = false;
#ifdef Q_DIRMODEL_CACHED
            node.cached_size = info.size();
            node.cached_modified = info.lastModified();
#endif
            d->root.children.append(node);
        }
        idx = index(r, 0, QModelIndex());
        pathElements.pop_front();
    }
#else
    // add the "/" item, since it is a valid path element on unix
    pathElements.prepend("/");
#endif
    for (int i = 0; i < pathElements.count(); ++i) {
        QStringList entries;
        Q_ASSERT(!pathElements.at(i).isEmpty()); // we don't allow empty elements
        // get the list of children of the current path element
        if (idx.isValid()) {
            QDirModelPrivate::QDirNode *node =
                static_cast<QDirModelPrivate::QDirNode*>(idx.internalPointer());
            Q_ASSERT(node);
            entries = d->entryList(node->info.absoluteFilePath());
        } else { // parent is "My Computer"
            for (int j = 0; j < d->root.children.count(); ++j)
                entries << QDir::cleanPath(d->root.children.at(j).info.absoluteFilePath());
        }

        // find the row of the current path element in the list of children
        int row = entries.indexOf(pathElements.at(i));
        idx = index(row, column, idx); // will check row and lazily populate
        Q_ASSERT(idx.isValid());
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
    QDirModelPrivate::QDirNode *node =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(node);
    return node->info.isDir();
}

/*!
  Create a directory with the \a name in the \a parent model item.
*/

QModelIndex QDirModel::mkdir(const QModelIndex &parent, const QString &name)
{
    Q_D(QDirModel);
    if (!parent.isValid())
        return QModelIndex();

    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(parent.internalPointer());
    Q_ASSERT(p);
    QString path = p->info.absoluteFilePath();
    // For the indexOf() method to work, the new directory has to be a direct child of
    // the parent directory.

    QDir newDir(name); 
    QDir dir(path);
    if (newDir.isRelative())
        newDir = QDir(path + "/" + name); 
    QString childName = newDir.dirName(); // Get the singular name of the directory
    newDir.cdUp();

    if (newDir.absolutePath() != dir.absolutePath() || !dir.mkdir(name))
        return QModelIndex(); // nothing happened

    refresh(parent);

    QStringList entryList = d->entryList(path);
    int r = entryList.indexOf(childName);
    QModelIndex i = index(r, 0, parent); // return an invalid index

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

    QDirModelPrivate::QDirNode *n =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(n);

    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return false;
    }

    QModelIndex par = parent(index);
    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(par.internalPointer());
    Q_ASSERT(p);

    QDir dir = p->info.dir(); // parent dir
    QString path = n->info.absoluteFilePath();
    if (!dir.rmdir(path))
        return false;

    refresh(par);

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

    QDirModelPrivate::QDirNode *n =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(n);

    if (n->info.isDir())
        return false;

    QModelIndex par = parent(index);
    QDirModelPrivate::QDirNode *p =
        static_cast<QDirModelPrivate::QDirNode*>(par.internalPointer());
    Q_ASSERT(p);

    QDir dir = p->info.dir(); // parent dir
    QString path = n->info.absoluteFilePath();
    if (!dir.remove(path))
        return false;

    refresh(par);
    
    return true;
}

/*!
  Returns the path of the item stored in the model under the
  \a index given.

*/

QString QDirModel::filePath(const QModelIndex &index) const
{
    Q_D(const QDirModel);
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
    Q_D(const QDirModel);
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

    QDirModelPrivate::QDirNode *node =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
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
    clear(&root);
}

QDirModelPrivate::QDirNode *QDirModelPrivate::node(int row, QDirNode *parent) const
{
    if (row < 0)
	return 0;

    bool isDir =  !parent || parent->info.isDir();
    QDirNode *p = (parent ? parent : &root);
    if (isDir && p->children.isEmpty())
        populate(p); // will also resolve symlinks

    if (row >= p->children.count()) {
        qWarning("node: the row does not exist");
        return 0;
    }

    return const_cast<QDirNode*>(&p->children.at(row));
}

QDirModelPrivate::QDirNode *QDirModelPrivate::parent(QDirNode *child) const
{
    return child ? child->parent : 0;
}

QVector<QDirModelPrivate::QDirNode> QDirModelPrivate::children(QDirNode *parent) const
{
    QFileInfoList info;
    if (!parent) {
        info = QDir::drives();
    } else if (parent->info.isDir()) {
        if (resolveSymlinks && parent->info.isSymLink()) {
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
        nodes[i].populated = false;
#ifdef Q_DIRMODEL_CACHED
        nodes[i].cached_size = info.at(i).size();
        nodes[i].cached_modified = info.at(i).lastModified();
#endif
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
        info = QDir::drives(); 
    else if (parent->info.isDir())
        info = entryInfoList(parent->info.filePath());

    QVector<QDirNode> *nodes = parent ? &(parent->children) : &(root.children);
    if (nodes->count() != info.count())
        nodes->resize(info.count());

    for (int i = 0; i < (int)info.count(); ++i) {
        (*nodes)[i].parent = parent;
        if (resolveSymlinks && info.at(i).isSymLink()) {
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
    Q_Q(QDirModel);
    saved.clear();
    const QList<QPersistentModelIndexData*> indexes = persistent.indexes;
    for (int i = 0; i < indexes.count(); ++i) {
        QModelIndex idx = indexes.at(i)->index;
        QString path = q->filePath(idx);
        saved.append(qMakePair(path, idx.column()));
        persistent.indexes.at(i)->ref.ref(); // save
        persistent.indexes[i]->index = QModelIndex(); // invalidated
    }
}

void QDirModelPrivate::restorePersistentIndexes()
{
    Q_Q(QDirModel);
    const QList<QPersistentModelIndexData*> indexes = persistent.indexes;
    QList<QPersistentModelIndexData*> deleteList;
    for (int i = 0; i < indexes.count(); ++i) {
        persistent.indexes[i]->index = q->index(saved.at(i).first, saved.at(i).second);
        if (!persistent.indexes.at(i)->ref.deref()) // if we have no other references
            deleteList.append(indexes.at(i)); // make sure we delete it
    }
    saved.clear();
    while (!deleteList.isEmpty()) {
        QPersistentModelIndexData::destroy(deleteList.last());
        deleteList.removeLast();
    }
}

