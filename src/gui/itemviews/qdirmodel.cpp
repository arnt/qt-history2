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

#ifndef QT_NO_DIRMODEL
#include <qstack.h>
#include <qfile.h>
#include <qurl.h>
#include <qmime.h>
#include <qpair.h>
#include <qvector.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qlocale.h>
#include <qstyle.h>
#include <qapplication.h>
#include <private/qabstractitemmodel_p.h>
#include <qdebug.h>
#if defined(Q_WS_WIN)
#  include "qt_windows.h"
#endif

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
    delete d_ptr;
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
        default:
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
        return QApplication::translate("QFileDialog", "Drive");
    if (info.isFile())
        return info.suffix() + QLatin1String(" ") + QApplication::translate("QFileDialog", "File");
    if (info.isDir())
        return QApplication::translate("QFileDialog", "Directory");
    if (info.isSymLink())
        return QApplication::translate("QFileDialog", "Symbolic Link");
    return QApplication::translate("QFileDialog", "Unknown");
}

class QDirModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QDirModel)

public:
    struct QDirNode
    {
        QDirNode() : parent(0), populated(false), stat(false) {}
        ~QDirNode() { children.clear(); }
        QDirNode *parent;
        QFileInfo info;
        QIcon icon; // cache the icon
        mutable QVector<QDirNode> children;
        mutable bool populated; // have we read the children
        mutable bool stat;
    };

    QDirModelPrivate()
        : resolveSymlinks(true),
          readOnly(true),
          lazyChildCount(false),
          allowAppendChild(true),
          iconProvider(&defaultProvider),
          shouldStat(true) // ### This is set to false by QFileDialog
    { }

    void init();
    QDirNode *node(int row, QDirNode *parent) const;
    QVector<QDirNode> children(QDirNode *parent, bool stat) const;

    void refresh();

    void savePersistentIndexes();
    void restorePersistentIndexes();

    QFileInfoList entryInfoList(const QString &path) const;
    QStringList entryList(const QString &path) const;

    QString name(const QModelIndex &index) const;
    QString size(const QModelIndex &index) const;
    QString type(const QModelIndex &index) const;
    QString time(const QModelIndex &index) const;

    void appendChild(QDirModelPrivate::QDirNode *parent, const QString &path) const;
    static QFileInfo resolvedInfo(QFileInfo info);

    inline QDirNode *node(const QModelIndex &index) const;
    inline void populate(QDirNode *parent) const;
    inline void clear(QDirNode *parent) const;

    void invalidate();

    mutable QDirNode root;
    bool resolveSymlinks;
    bool readOnly;
    bool lazyChildCount;
    bool allowAppendChild;

    QDir::Filters filters;
    QDir::SortFlags sort;
    QStringList nameFilters;

    QFileIconProvider *iconProvider;
    QFileIconProvider defaultProvider;

    QList< QPair<QString,int> > savedPaths;
    QList< QPersistentModelIndex > savedPersistentIndexes;
    QPersistentModelIndex toBeRefreshed;

    bool shouldStat; // use the "carefull not to stat directories" mode
};

void qt_setDirModelShouldNotStat(QDirModelPrivate *modelPrivate)
{
    modelPrivate->shouldStat = false;
}

QDirModelPrivate::QDirNode *QDirModelPrivate::node(const QModelIndex &index) const
{
    QDirModelPrivate::QDirNode *n =
        static_cast<QDirModelPrivate::QDirNode*>(index.internalPointer());
    Q_ASSERT(n);
    return n;
}

void QDirModelPrivate::populate(QDirNode *parent) const
{
    Q_ASSERT(parent);
    parent->children = children(parent, parent->stat);
    parent->populated = true;
}

void QDirModelPrivate::clear(QDirNode *parent) const
{
     Q_ASSERT(parent);
     parent->children.clear();
     parent->populated = false;
}

void QDirModelPrivate::invalidate()
{
    QStack<const QDirNode*> nodes;
    nodes.push(&root);
    while (!nodes.empty()) {
        const QDirNode *current = nodes.pop();
        current->stat = false;
        const QVector<QDirNode> children = current->children;
        for (int i = 0; i < children.count(); ++i)
            nodes.push(&children.at(i));
    }
}

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

  \sa nameFilters(), setFilter(), filter(), {Model/View Programming}, QListView, QTreeView

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
    // note that rowCount does lazy population
    if (column < 0 || column >= 4 || row < 0 || parent.column() > 0)
        return QModelIndex();
    // make sure the list of children is up to date
    QDirModelPrivate::QDirNode *p = (parent.isValid() ? d->node(parent) : &d->root);
    Q_ASSERT(p);
    if (!p->populated)
        d->populate(p); // populate without stat'ing
    if (row >= p->children.count())
        return QModelIndex();
    // now get the internal pointer for the index
    QDirModelPrivate::QDirNode *n = d->node(row, parent.isValid() ? p : 0);
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
    QDirModelPrivate::QDirNode *node = d->node(child);
    QDirModelPrivate::QDirNode *par = (node ? node->parent : 0);
    if (par == 0) // parent is the root node
	return QModelIndex();

    // get the parent's row
    const QVector<QDirModelPrivate::QDirNode> children =
        par->parent ? par->parent->children : d->root.children;
    Q_ASSERT(children.count() > 0);
    int row = (par - &(children.at(0)));
    Q_ASSERT(row >= 0);

    return createIndex(row, 0, par);
}

/*!
  Returns the number of rows in the \a parent model item.

*/

int QDirModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QDirModel);
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        if (!d->root.populated) // lazy population
            d->populate(&d->root);
        return d->root.children.count();
    }
    QDirModelPrivate::QDirNode *p = d->node(parent);
    if (p->info.isDir() && !p->populated) // lazy population
        d->populate(p);
    return p->children.count();
}

/*!
  Returns the number of columns in the \a parent model item.

*/

int QDirModel::columnCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
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

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return d->name(index);
        case 1: return d->size(index);
        case 2: return d->type(index);
        case 3: return d->time(index);
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

  \sa Qt::ItemDataRole
*/

bool QDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(QDirModel);
    if (!index.isValid() || index.column() != 0
        || (flags(index) & Qt::ItemIsEditable) == 0 || role != Qt::EditRole)
        return false;

    QDirModelPrivate::QDirNode *node = d->node(index);
    QDir dir = node->info.dir();
    QString name = value.toString();
    if (dir.rename(node->info.fileName(), name)) {
        node->info = QFileInfo(dir, name);
        QModelIndex sibling = index.sibling(index.row(), 3);
        emit dataChanged(index, sibling);

        d->toBeRefreshed = index.parent();
        int slot = metaObject()->indexOfSlot("refresh()");
        QApplication::postEvent(this, new QMetaCallEvent(slot));

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
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) // the invalid index is the "My Computer" item
        return true; // the drives
    QDirModelPrivate::QDirNode *p = d->node(parent);
    Q_ASSERT(p);

    if (d->lazyChildCount) // optimization that only checks for children if the node has been populated
        return p->info.isDir();
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
    QDirModelPrivate::QDirNode *node = d->node(index);
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
    QDir::SortFlags sort = QDir::DirsFirst;
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
        sort |= QDir::Type;
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
    return QStringList(QLatin1String("text/uri-list"));
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
    the given \a action over the row in the model specified by the \a row and
    \a column and by the \a parent index.

    \sa supportedDropActions()
*/

bool QDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, int /* column */, const QModelIndex &parent)
{
    Q_D(QDirModel);
    if (!parent.isValid() || isReadOnly())
        return false;

    QDirModelPrivate::QDirNode *p = d->node(parent);

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
    d->nameFilters = filters;
    if (d->shouldStat)
        refresh(QModelIndex());
    else
        d->invalidate();
    emit layoutChanged();
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

  Note that the filter you set should always include the QDir::AllDirs enum value,
  otherwise QDirModel won't be able to read the directory structure.

  \sa QDir::Filters
*/

void QDirModel::setFilter(QDir::Filters filters)
{
    Q_D(QDirModel);
    d->filters = filters;
    if (d->shouldStat)
        refresh(QModelIndex());
    else
        d->invalidate();
    emit layoutChanged();
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
    d->sort = sort;
    if (d->shouldStat)
        refresh(QModelIndex());
    else
        d->invalidate();
    emit layoutChanged();
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

    QDirModelPrivate::QDirNode *n = parent.isValid() ? d->node(parent) : &(d->root);

    int rows = n->children.count();
    if (rows == 0) {
        n->stat = true; // make sure that next time we read all the info
        return;
    }

    d->savePersistentIndexes();
    beginRemoveRows(parent, 0, rows - 1);
    n->stat = true; // make sure that next time we read all the info
    d->clear(n);
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

    if (path.isEmpty() || path == QCoreApplication::translate("QFileDialog", "My Computer"))
        return QModelIndex();

    QString absolutePath = QDir(path).absolutePath();
#ifdef Q_OS_WIN
    absolutePath = absolutePath.toLower();
    // On Windows, "filename......." and "filename" are equivalent
    if (absolutePath.endsWith(QLatin1Char('.'))) {
        int i;
        for (i = absolutePath.count() - 1; i >= 0; --i) {
            if (absolutePath.at(i) != QLatin1Char('.'))
                break;
        }
        absolutePath = absolutePath.left(i+1);
    }
#endif

    QStringList pathElements = absolutePath.split(QChar('/'), QString::SkipEmptyParts);
    if ((pathElements.isEmpty() || !QFileInfo(path).exists())
#ifndef Q_OS_WIN
        && path != QLatin1String("/")
#endif
        )
        return QModelIndex();

    QModelIndex idx; // start with "My Computer"
    if (!d->root.populated) // make sure the root is populated
        d->populate(&d->root);

#ifdef Q_OS_WIN
    if (absolutePath.startsWith(QLatin1String("//"))) { // UNC path
        QString host = pathElements.first();
        int r = 0;
        for (; r < d->root.children.count(); ++r)
            if (d->root.children.at(r).info.fileName() == host)
                break;
        if (r >= d->root.children.count() && d->allowAppendChild)
            d->appendChild(&d->root, QLatin1String("//") + host);
        idx = index(r, 0, QModelIndex());
        pathElements.pop_front();
    } else if (pathElements.at(0).endsWith(QLatin1Char(':'))) {
        pathElements[0] += QLatin1Char('/');
    }
#else
    // add the "/" item, since it is a valid path element on unix
    pathElements.prepend(QLatin1String("/"));
#endif

    for (int i = 0; i < pathElements.count(); ++i) {
        Q_ASSERT(!pathElements.at(i).isEmpty());
        QString element = pathElements.at(i);
        QDirModelPrivate::QDirNode *parent = (idx.isValid() ? d->node(idx) : &d->root);

        Q_ASSERT(parent);
        if (!parent->populated)
            d->populate(parent);

        // search for the element in the child nodes first
        int row = -1;
        for (int j = parent->children.count() - 1; j >= 0; --j) {
            const QFileInfo& fi = parent->children.at(j).info;
            QString childFileName;
#ifdef Q_OS_WIN
            childFileName = idx.isValid() ? fi.fileName() : fi.absoluteFilePath();
            childFileName = childFileName.toLower();
#else
            childFileName = idx.isValid() ? fi.fileName() : fi.absoluteFilePath();
#endif
            if (childFileName == element) {
                if (i == pathElements.count() - 1)
                    parent->children[j].stat = true;
                row = j;
                break;
            }
        }

        // we couldn't find the path element, we create a new node since we _know_ that the path is valid
        if (row == -1) {
            QString newPath = parent->info.absoluteFilePath() + "/" + element;
            if (!d->allowAppendChild || !QFileInfo(newPath).isDir())
                return QModelIndex();
            d->appendChild(parent, newPath);
            row = parent->children.count() - 1;
            if (i == pathElements.count() - 1) // always stat children of  the last element
                parent->children[row].stat = true;
            emit const_cast<QDirModel*>(this)->layoutChanged();
        }

        Q_ASSERT(row >= 0);
        idx = createIndex(row, 0, static_cast<void*>(&parent->children[row]));
        Q_ASSERT(idx.isValid());
    }

    if (column != 0)
        return idx.sibling(idx.row(), column);
    return idx;
}

/*!
  Returns true if the model item \a index represents a directory;
  otherwise returns false.
*/

bool QDirModel::isDir(const QModelIndex &index) const
{
    Q_D(const QDirModel);
    Q_ASSERT(index.isValid());
    QDirModelPrivate::QDirNode *node = d->node(index);
    return node->info.isDir();
}

/*!
  Create a directory with the \a name in the \a parent model item.
*/

QModelIndex QDirModel::mkdir(const QModelIndex &parent, const QString &name)
{
    Q_D(QDirModel);
    if (!parent.isValid() || isReadOnly())
        return QModelIndex();

    QDirModelPrivate::QDirNode *p = d->node(parent);
    QString path = p->info.absoluteFilePath();
    // For the indexOf() method to work, the new directory has to be a direct child of
    // the parent directory.

    QDir newDir(name);
    QDir dir(path);
    if (newDir.isRelative())
        newDir = QDir(path + QLatin1Char('/') + name);
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
    if (!index.isValid() || isReadOnly())
        return false;

    QDirModelPrivate::QDirNode *n = d_func()->node(index);
    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return false;
    }

    QModelIndex par = parent(index);
    QDirModelPrivate::QDirNode *p = d_func()->node(par);
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
    if (!index.isValid() || isReadOnly())
        return false;

    QDirModelPrivate::QDirNode *n = d_func()->node(index);
    if (n->info.isDir())
        return false;

    QModelIndex par = parent(index);
    QDirModelPrivate::QDirNode *p = d_func()->node(par);
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
        if (d->resolveSymlinks && fi.isSymLink())
            fi = d->resolvedInfo(fi);
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
    Q_D(const QDirModel);
    if (!index.isValid())
        return QString();
    QFileInfo info = fileInfo(index);
    if (info.isRoot())
        return info.absoluteFilePath();
    if (d->resolveSymlinks && info.isSymLink())
        info = d->resolvedInfo(info);
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
        return d->iconProvider->icon(QFileIconProvider::Computer);
    QDirModelPrivate::QDirNode *node = d->node(index);
    if (node->icon.isNull())
        node->icon = d->iconProvider->icon(node->info);
    return node->icon;
}

/*!
  Returns the file information for the model item \a index.

*/

QFileInfo QDirModel::fileInfo(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QDirModelPrivate::QDirNode *node = d_func()->node(index);
    return node->info;
}

/*
  The root node is never seen outside the model.
*/

void QDirModelPrivate::init()
{
    filters = QDir::AllEntries;
    sort = QDir::Name;
    nameFilters << QLatin1String("*");
    root.parent = 0;
    root.info = QFileInfo();
    clear(&root);
}

QDirModelPrivate::QDirNode *QDirModelPrivate::node(int row, QDirNode *parent) const
{
    if (row < 0)
	return 0;

    bool isDir = !parent || parent->info.isDir();
    QDirNode *p = (parent ? parent : &root);
    if (isDir && !p->populated)
        populate(p); // will also resolve symlinks

    if (row >= p->children.count()) {
        qWarning("node: the row does not exist");
        return 0;
    }

    return const_cast<QDirNode*>(&p->children.at(row));
}

QVector<QDirModelPrivate::QDirNode> QDirModelPrivate::children(QDirNode *parent, bool stat) const
{
    Q_ASSERT(parent);
    QFileInfoList infoList;
    if (parent == &root) {
        parent = 0;
        infoList = QDir::drives();
    } else if (parent->info.isDir()) {
        if (parent->info.isSymLink()) {
            QString link = parent->info.readLink();
            if (link.at(link.size() - 1) == QDir::separator())
                link.chop(1);
            if (stat)
                infoList = entryInfoList(link);
            else
                infoList = QDir(link).entryInfoList(nameFilters, QDir::AllEntries | QDir::System);
        } else {
            if (stat)
                infoList = entryInfoList(parent->info.absoluteFilePath());
            else
                infoList = QDir(parent->info.absoluteFilePath()).entryInfoList(nameFilters, QDir::AllEntries | QDir::System);
        }
    }

    QVector<QDirNode> nodes(infoList.count());
    for (int i = 0; i < infoList.count(); ++i) {
        QDirNode &node = nodes[i];
        node.parent = parent;
        node.info = infoList.at(i);
        node.populated = false;
        node.stat = shouldStat;
    }

    return nodes;
}

void QDirModelPrivate::refresh()
{
    Q_Q(QDirModel);
    q->refresh(toBeRefreshed);
    toBeRefreshed = QModelIndex();
}

void QDirModelPrivate::savePersistentIndexes()
{
    Q_Q(QDirModel);
    savedPaths.clear();
    const QList<QPersistentModelIndexData*> indexes = persistent.indexes;
    for (int i = 0; i < indexes.count(); ++i) {
        QModelIndex idx = indexes.at(i)->index;
        QString path = q->filePath(idx);
        savedPaths.append(qMakePair(path, idx.column()));
        savedPersistentIndexes.append(idx); // make sure the ref  >= 1
    }
}

void QDirModelPrivate::restorePersistentIndexes()
{
    Q_Q(QDirModel);
    bool allow = allowAppendChild;
    allowAppendChild = false;
    for (int i = 0; i < persistent.indexes.count(); ++i) {
        QModelIndex index;
        if (i < savedPaths.count())
            index = q->index(savedPaths.at(i).first, savedPaths.at(i).second);
        persistent.indexes.at(i)->index = index;
    }
    savedPersistentIndexes.clear();
    savedPaths.clear();
    allowAppendChild = allow;
}

QFileInfoList QDirModelPrivate::entryInfoList(const QString &path) const
{
    const QDir dir(path);
    return dir.entryInfoList(nameFilters, filters | QDir::NoDotAndDotDot, sort);
}

QStringList QDirModelPrivate::entryList(const QString &path) const
{
    const QDir dir(path);
    return dir.entryList(nameFilters, filters | QDir::NoDotAndDotDot, sort);
}

QString QDirModelPrivate::name(const QModelIndex &index) const
{
    const QDirNode *n = node(index);
    const QFileInfo info = n->info;
    if (info.isRoot()) {
        QString name = info.absoluteFilePath();
#ifdef Q_OS_WIN
        if (name.startsWith(QLatin1Char('/'))) // UNC host
            return info.fileName();
        if (name.endsWith(QLatin1Char('/')))
            name.chop(1);
#endif
        return name;
    }
    return info.fileName();
}

QString QDirModelPrivate::size(const QModelIndex &index) const
{
    quint64 bytes = node(index)->info.size();
    if (bytes >= 1000000000)
        return QLocale().toString(bytes / 1000000000) + QString(" GB");
    if (bytes >= 1000000)
        return QLocale().toString(bytes / 1000000) + QString(" MB");
    if (bytes >= 1000)
        return QLocale().toString(bytes / 1000) + QString(" KB");
    return QLocale().toString(bytes) + QString(" bytes");
}

QString QDirModelPrivate::type(const QModelIndex &index) const
{
    return iconProvider->type(node(index)->info);
}

QString QDirModelPrivate::time(const QModelIndex &index) const
{
#ifndef QT_NO_DATESTRING
    return node(index)->info.lastModified().toString("yyyy-MM-dd hh:mm:ss");
#else
    Q_UNUSED(index);
    return QString();
#endif
}

void QDirModelPrivate::appendChild(QDirModelPrivate::QDirNode *parent, const QString &path) const
{
    QDirModelPrivate::QDirNode node;
    node.populated = false;
    node.stat = shouldStat;
    node.parent = (parent == &root ? 0 : parent);
    node.info = QFileInfo(path);
    node.info.setCaching(true);

    // The following append(node) may reallocate the vector, thus
    // we need to update the pointers to the childnodes parent.
    QDirModelPrivate *that = const_cast<QDirModelPrivate *>(this);
    that->savePersistentIndexes();
    parent->children.append(node);
    for (int i = 0; i < parent->children.count(); ++i) {
        QDirNode *childNode = &parent->children[i];
        for (int j = 0; j < childNode->children.count(); ++j)
            childNode->children[j].parent = childNode;
    }
    that->restorePersistentIndexes();
}

QFileInfo QDirModelPrivate::resolvedInfo(QFileInfo info)
{
#ifdef Q_OS_WIN
    // On windows, we cannot create a shortcut to a shortcut.
    return QFileInfo(info.readLink());
#else
    QStringList paths;
    do {
        QFileInfo link(info.readLink());
        if (link.isRelative())
            info.setFile(info.absolutePath(), link.filePath());
        else
            info = link;
        if (paths.contains(info.absoluteFilePath()))
            return QFileInfo();
        paths.append(info.absoluteFilePath());
    } while (info.isSymLink());
    return info;
#endif
}

#include "moc_qdirmodel.cpp"

#endif // QT_NO_DIRMODEL
