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

#include "qfilesystemmodel_p.h"
#include <qlocale.h>
#include <qmime.h>
#include <qurl.h>
#include <qdebug.h>
#include <qmessagebox.h>

/*!
    \enum QFileSystemModel::Roles
    \value FileIconRole
    \value FilePathRole
    \value FileNameRole
*/

QFileSystemModel::QFileSystemModel(QObject *parent)
    : QAbstractItemModel(*new QFileSystemModelPrivate, parent)
{
    Q_D(QFileSystemModel);
    d->init();
}

QFileSystemModel::QFileSystemModel(QFileSystemModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(QFileSystemModel);
    d->init();
}

QFileSystemModel::~QFileSystemModel()
{
}

/*!
    \reimp
*/
QModelIndex QFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const QFileSystemModel);
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    // get the parent node
    QFileSystemModelPrivate::QFileSystemNode *parentNode = (d->indexValid(parent) ? d->node(parent) :
                                                   const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&d->root));
    Q_ASSERT(parentNode);

    // now get the internal pointer for the index
    int realRow = parentNode->visibleChildren[d->translateVisibleLocation(parentNode, row)];
    const QFileSystemModelPrivate::QFileSystemNode *indexNode = &parentNode->children.at(realRow);
    Q_ASSERT(indexNode);

    return createIndex(row, column, const_cast<QFileSystemModelPrivate::QFileSystemNode*>(indexNode));
}

/*!
    \overload

    Returns the model item index for the given \a path.
*/
QModelIndex QFileSystemModel::index(const QString &path, int column) const
{
    Q_D(const QFileSystemModel);
    QFileSystemModelPrivate::QFileSystemNode *node = d->node(path, false);
    QModelIndex idx = d->index(node);
    if (idx.column() != column)
        idx = idx.sibling(idx.row(), column);
    return idx;
}

/*!
    \internal

    Return the QFileSystemNode that goes to index.
  */
QFileSystemModelPrivate::QFileSystemNode *QFileSystemModelPrivate::node(const QModelIndex &index) const
{
    if (!index.isValid())
        return const_cast<QFileSystemNode*>(&root);
    QFileSystemModelPrivate::QFileSystemNode *indexNode = static_cast<QFileSystemModelPrivate::QFileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);
    return indexNode;
}

/*!
    \internal

    Given a path return the matching QFileSystemNode or &root if invalid
*/
QFileSystemModelPrivate::QFileSystemNode *QFileSystemModelPrivate::node(const QString &path, bool fetch) const
{
    Q_Q(const QFileSystemModel);
    Q_UNUSED(q);
    if (path.isEmpty() || path == myComputer())
        return const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&root);

    // Construct the nodes up to the new root path if they need to be built
    QString absolutePath;
    if (path == rootDir.path())
        absolutePath = rootDir.absolutePath();
    else
        absolutePath = QDir(path).absolutePath();

    // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if ((pathElements.isEmpty())
#ifndef Q_OS_WIN
        && path != QLatin1String("/")
#endif
        )
        return const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&root);
    QModelIndex index = QModelIndex(); // start with "My Computer"
#ifdef Q_OS_WIN
    if (absolutePath.startsWith(QLatin1String("//"))) { // UNC path
        QString host = QLatin1String("\\\\") + pathElements.first();
        int r = 0;
        for (; r < root.children.count(); ++r)
            if (root.children.at(r).fileName.toLower() == host.toLower())
                break;
        QFileSystemModelPrivate::QFileSystemNode *rootNode = const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&root);
        QFileInfo info(host);
        if (!info.exists())
            return rootNode;
        if (r >= root.children.count()) {
            QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate*>(this);
            r = p->addNode(rootNode, host);
            p->addVisibleFiles(rootNode, QStringList(host));
        }
        r = rootNode->visibleLocation(r);
        r = translateVisibleLocation(rootNode, r);
        index = q->index(r, 0, QModelIndex());
        pathElements.pop_front();
    } else {
        if (!pathElements.at(0).contains(QLatin1String(":")))
            pathElements.prepend(QDir(path).rootPath());
        if (pathElements.at(0).endsWith(QLatin1Char('/')))
            pathElements[0].chop(1);
    }
#else
    // add the "/" item, since it is a valid path element on Unix
    if (path[0] == QLatin1Char('/'))
        pathElements.prepend(QLatin1String("/"));
#endif

    QFileSystemModelPrivate::QFileSystemNode *parent = node(index);
    for (int i = 0; i < pathElements.count(); ++i) {
        QString element = pathElements.at(i);
#ifdef Q_OS_WIN
        // On Windows, "filename......." and "filename" are equivalent Task #133928
        while (element.endsWith(QLatin1Char('.')))
            element.chop(1);
#endif
        int row = -1;
        if (parent->children.count() != 0)
            row = findChild(parent, QFileSystemNode(element));

        // we couldn't find the path element, we create a new node since we
        // _know_ that the path is valid
        if (row != -1) {
	    if ((parent->children.count() == 0)
                || (parent->caseSensitive()
                    && parent->children[row].fileName != element)
                || (!parent->caseSensitive()
                    && parent->children[row].fileName.toLower() != element.toLower()))
	        row = -1;
	}

        if (row == -1) {
            // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
            // a path that doesn't exists, I.E. don't blindly create directories.
            QStringList currentPath = pathElements.mid(0, i + 1);
            QFileInfo info(currentPath.join(QLatin1String("/")));
            if (!info.exists())
                return const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&root);
            QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate*>(this);
            row = p->addNode(parent, element);
            parent->children[row].populate(fileInfoGatherer.getInfo(info));
        }
        Q_ASSERT(row >= 0);
        if (parent->visibleLocation(row) == -1) {
            QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate*>(this);
            p->addVisibleFiles(parent, QStringList(element));
            if (!p->bypassFilters.contains(&parent->children.at(row)))
                p->bypassFilters.append(&parent->children.at(row));
            QString dir = q->filePath(this->index(parent));
            if (!parent->children.at(row).hasInformation() && fetch) {
                Fetching f;
                f.dir = dir;
                f.file = element;
                f.node = &parent->children.at(row);
                p->toFetch.append(f);
                p->fetchingTimer.start(0, const_cast<QFileSystemModel*>(q));
            }
        }
        parent = &parent->children[row];
    }

    return parent;
}

void QFileSystemModel::timerEvent(QTimerEvent *event)
{
    Q_D(QFileSystemModel);
    if (event->timerId() == d->fetchingTimer.timerId()) {
        d->fetchingTimer.stop();
        for (int i = 0; i < d->toFetch.count(); ++i) {
            const QFileSystemModelPrivate::QFileSystemNode *node = d->toFetch.at(i).node;
            if (!node->hasInformation()) {
                d->fileInfoGatherer.fetchExtendedInformation(d->toFetch.at(i).dir,
                                                 QStringList(d->toFetch.at(i).file));
            } else {
                // qDebug() << "yah!, you saved a little gerbil soul";
            }
        }
        d->toFetch.clear();
    }
}

/*!
    Returns true if the model item \a index represents a directory;
    otherwise returns false.
*/
bool QFileSystemModel::isDir(const QModelIndex &index) const
{
    // This function is for public usage only because it could create a file info
    Q_D(const QFileSystemModel);
    if (!index.isValid())
        return true;
    QFileSystemModelPrivate::QFileSystemNode *n = d->node(index);
    if (n->hasInformation())
        return n->isDir();
    return fileInfo(index).isDir();
}

qint64 QFileSystemModel::size(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    if (!index.isValid())
        return 0;
    return d->node(index)->size();
}

QString QFileSystemModel::type(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    if (!index.isValid())
        return QString();
    return d->node(index)->type();
}

QDateTime QFileSystemModel::lastModified(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    if (!index.isValid())
        return QDateTime();
    return d->node(index)->lastModified();
}

/*!
    \reimp
*/
QModelIndex QFileSystemModel::parent(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    if (!d->indexValid(index))
        return QModelIndex();

    QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(index);
    Q_ASSERT(indexNode != 0);
    QFileSystemModelPrivate::QFileSystemNode *parentNode = (indexNode ? indexNode->parent : 0);
    if (parentNode == 0 || parentNode == &d->root)
        return QModelIndex();

    // get the parent's row
    QFileSystemModelPrivate::QFileSystemNode *grandParentNode = parentNode->parent;
    int realRow = d->findChild(grandParentNode, *parentNode);
    Q_ASSERT(realRow >= 0);
    int visualRow = d->translateVisibleLocation(grandParentNode, grandParentNode->visibleLocation(realRow));
    if (visualRow == -1)
        return QModelIndex();
    return createIndex(visualRow, 0, parentNode);
}

/*
    \internal

    return the index for node
*/
QModelIndex QFileSystemModelPrivate::index(const QFileSystemModelPrivate::QFileSystemNode *node) const
{
    Q_Q(const QFileSystemModel);
    QFileSystemModelPrivate::QFileSystemNode *parentNode = (node ? node->parent : 0);
    if (node == &root)
        return QModelIndex();

    // get the parent's row
    int realRow = findChild(parentNode, *node);
    Q_ASSERT(realRow >= 0);
    int visualRow = translateVisibleLocation(parentNode, parentNode->visibleLocation(realRow));
    if (visualRow == -1)
        return QModelIndex();
    return q->createIndex(visualRow, 0, const_cast<QFileSystemNode*>(node));
}

/*!
    \reimp
*/
bool QFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const QFileSystemModel);
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) // drives
        return true;

    const QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
    Q_ASSERT(indexNode);
    return (indexNode->isDir());
}

/*!
    \reimp
 */
bool QFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    Q_D(const QFileSystemModel);
    const QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
    return (!indexNode->populatedChildren);
}

/*!
    \reimp
 */
void QFileSystemModel::fetchMore(const QModelIndex &parent)
{
    Q_D(QFileSystemModel);
    QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
    if (indexNode->populatedChildren)
        return;
    indexNode->populatedChildren = true;
    d->fileInfoGatherer.list(filePath(parent));
}

/*!
    \reimp
*/
int QFileSystemModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QFileSystemModel);
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return d->root.visibleChildren.count();

    const QFileSystemModelPrivate::QFileSystemNode *parentNode = d->node(parent);
    return parentNode->visibleChildren.count();
}

/*!
    \reimp
*/
int QFileSystemModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 4;
}

QVariant QFileSystemModel::myComputer(int role) const
{
    Q_D(const QFileSystemModel);
    switch (role) {
    case Qt::DisplayRole:
        return d->myComputer();
    case Qt::DecorationRole:
        return d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Computer);
    }
    return QVariant();
}

/*!
    \reimp
*/
QVariant QFileSystemModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QFileSystemModel);
    if (!index.isValid() || index.model() != this)
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return d->name(index);
        case 1: return d->size(index);
        case 2: return d->type(index);
        case 3: return d->time(index);
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return filePath(index);
    case FileNameRole:
        return d->name(index);
    case Qt::DecorationRole:
        if (index.column() == 0) {
            QIcon icon = d->icon(index);
            if (icon.isNull()) {
                if (d->node(index)->isDir())
                    icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Folder);
                else
                    icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::File);
            }
            return icon;
        }
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return Qt::AlignRight;
        break;
    }

    return QVariant();
}

/*!
    \internal
*/
QString QFileSystemModelPrivate::size(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    const QFileSystemNode *n = node(index);
    if (n->isDir()) {
#ifdef Q_OS_MAC
        return QLatin1String("--");
#else
        return QLatin1String("");
#endif
    // Windows   - ""
    // OS X      - "--"
    // Konqueror - "4 KB"
    // Nautilus  - "9 items" (the number of children)
    }
    return size(n->size());
}

QString QFileSystemModelPrivate::size(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes >= tb)
        return QFileSystemModel::tr("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return QFileSystemModel::tr("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return QFileSystemModel::tr("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return QFileSystemModel::tr("%1 KB").arg(QLocale().toString(bytes / kb));
    return QFileSystemModel::tr("%1 bytes").arg(QLocale().toString(bytes));
}

/*!
    \internal
*/
QString QFileSystemModelPrivate::time(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
#ifndef QT_NO_DATESTRING
    return node(index)->lastModified().toString(Qt::SystemLocaleDate);
#else
    Q_UNUSED(index);
    return QString();
#endif
}

/*
    \internal
*/
QString QFileSystemModelPrivate::type(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    return node(index)->type();
}

/*!
    \internal
*/
QString QFileSystemModelPrivate::name(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    QFileSystemNode *dirNode = node(index);
    if (dirNode->isSymLink() && fileInfoGatherer.resolveSymlinks()
        && resolvedSymLinks.contains(dirNode->fileName)) {
        return resolvedSymLinks[dirNode->fileName];
    }
    // ### TODO it would be nice to grab the volume name if dirNode->parent == root
    return dirNode->fileName;
}

/*!
    \internal
*/
QIcon QFileSystemModelPrivate::icon(const QModelIndex &index) const
{
    if (!index.isValid())
        return QIcon();
    return node(index)->icon();
}

/*!
    \reimp
*/
bool QFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(QFileSystemModel);
    if (!index.isValid()
        || index.column() != 0
        || role != Qt::EditRole
        || (flags(index) & Qt::ItemIsEditable) == 0) {
        return false;
    }

    QString newName = value.toString();
    if (newName == index.data().toString())
        return true;

    if (newName.contains(QDir::separator())
        || !d->rootDir.rename(index.data().toString(), newName)) {
#ifndef QT_NO_MESSAGEBOX
        QMessageBox::information(0, QFileSystemModel::tr("Invalid filename"),
                                QFileSystemModel::tr("<b>The name \"%1\" can not be used.</b><p>Try using another name, with fewer characters or no punctuations marks.")
                                .arg(newName),
                                 QMessageBox::Ok);
#endif // QT_NO_MESSAGEBOX
        return false;
    } else {
        QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(index);
        QFileSystemModelPrivate::QFileSystemNode *parentNode = indexNode->parent;
        int itemLocation = d->findChild(parentNode, *indexNode);
        int visibleLocation = parentNode->visibleLocation(itemLocation);

        parentNode->visibleChildren.removeAt(visibleLocation);
        d->removeNode(parentNode, itemLocation);
        itemLocation = d->addNode(parentNode, newName);
        QFileInfo info(d->rootDir, newName);
        parentNode->children[itemLocation].populate(d->fileInfoGatherer.getInfo(info));
        parentNode->visibleChildren.insert(visibleLocation, itemLocation);
        d->delayedSort();
    }
    return true;
}

/*!
    \reimp
*/
QVariant QFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        if (section == 0) {
            // ### TODO oh man this is ugly and doesn't even work all the way!
            // it is still 2 pixels off
            QImage pixmap(16, 1, QImage::Format_Mono);
            pixmap.fill(0);
            pixmap.setAlphaChannel(pixmap.createAlphaMask());
            return pixmap;
        }
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section) {
    case 0: returnValue = tr("Name");
            break;
    case 1: returnValue = tr("Size");
            break;
    case 2: returnValue =
#ifdef Q_OS_MAC
                   tr("Kind", "Match OS X Finder");
#else
                   tr("Type", "All other platforms");
#endif
           break;
    // Windows   - Type
    // OS X      - Kind
    // Konqueror - File Type
    // Nautilus  - Type
    case 3: returnValue = tr("Date Modified");
            break;
    default: return QVariant();
    }
    return returnValue;
}

/*!
    \reimp
*/
Qt::ItemFlags QFileSystemModel::flags(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(index);
    if (d->nameFilterDisables && !d->passNameFilters(indexNode)) {
        flags &= ~Qt::ItemIsEnabled;
        // ### TODO you shouldn't be able to set this as the current item, task 119433
        return flags;
    }

    flags |= Qt::ItemIsDragEnabled;
    if (d->readOnly)
        return flags;
    if ((index.column() == 0) && indexNode->permissions() & QFile::WriteUser) {
        flags |= Qt::ItemIsEditable;
        if (indexNode->isDir())
            flags |= Qt::ItemIsDropEnabled;
    }
    return flags;
}

/*!
    \internal
*/
void QFileSystemModelPrivate::_q_performDelayedSort()
{
    Q_Q(QFileSystemModel);
    q->sort(sortColumn, sortOrder);
}

static inline QChar getNextChar(const QString &s, int location)
{
    return (location < s.length()) ? s.at(location) : QChar();
}

/*!
    Natural number sort, skips spaces.

    Examples:
    1, 2, 10, 55, 100
    01.jpg, 2.jpg, 10.jpg

    Note on the algorithm:
    Only as many characters as necessary are looked at and at most they all
    are looked at once.

    Slower then QString::compare() (of course)
  */
int QFileSystemModelPrivate::naturalCompare(const QString &s1, const QString &s2,  Qt::CaseSensitivity cs)
{
    for (int l1 = 0, l2 = 0; l1 <= s1.count() && l2 <= s2.count(); ++l1, ++l2) {
        // skip spaces, tabs and 0's
        QChar c1 = getNextChar(s1, l1);
        while (c1.isSpace())
            c1 = getNextChar(s1, ++l1);
        QChar c2 = getNextChar(s2, l2);
        while (c2.isSpace())
            c2 = getNextChar(s2, ++l2);

        if (c1.isDigit() && c2.isDigit()) {
            while (c1.digitValue() == 0)
                c1 = getNextChar(s1, ++l1);
            while (c2.digitValue() == 0)
                c2 = getNextChar(s2, ++l2);

            int lookAheadLocation1 = l1;
            int lookAheadLocation2 = l2;
            int currentReturnValue = 0;
            // find the last digit, setting currentReturnValue as we go if it isn't equal
            for (
                QChar lookAhead1 = c1, lookAhead2 = c2;
                (lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
                lookAhead1 = getNextChar(s1, ++lookAheadLocation1),
                lookAhead2 = getNextChar(s2, ++lookAheadLocation2)
                ) {
                bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
                bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();
                if (!is1ADigit && !is2ADigit)
                    break;
                if (!is1ADigit)
                    return -1;
                if (!is2ADigit)
                    return 1;
                if (currentReturnValue == 0) {
                    if (lookAhead1 < lookAhead2) {
                        currentReturnValue = -1;
                    } else if (lookAhead1 > lookAhead2) {
                        currentReturnValue = 1;
                    }
                }
            }
            if (currentReturnValue != 0)
                return currentReturnValue;
        }

        if (cs == Qt::CaseInsensitive) {
            if (!c1.isLower()) c1 = c1.toLower();
            if (!c2.isLower()) c2 = c2.toLower();
        }
        int r = QString::localeAwareCompare(c1, c2);
        if (r < 0)
            return -1;
        if (r > 0)
            return 1;
    }
    // The two strings are the same (02 == 2) so fall back to the normal sort
    return QString::compare(s1, s2, cs);
}

/*
    \internal
    Helper functor used by sort()
*/
class QFileSystemModelSorter
{
public:
    inline QFileSystemModelSorter(int column) : sortColumn(column) {}

    bool compareNodes(const QFileSystemModelPrivate::QFileSystemNode *l,
                    const QFileSystemModelPrivate::QFileSystemNode *r) const
    {
        switch (sortColumn) {
        case 0: {
#ifndef Q_OS_MAC
            // place directories before files
            bool left = l->isDir();
            bool right = r->isDir();
            if (left ^ right)
                return left;
#endif
            return QFileSystemModelPrivate::naturalCompare(l->fileName,
                                                r->fileName, Qt::CaseInsensitive) < 0;
                }
        case 1:
            // Directories go first
            if (l->isDir() && !r->isDir())
                return true;
            return l->size() < r->size();
        case 2:
            return l->type() < r->type();
        case 3:
            return l->lastModified() < r->lastModified();
        }
        Q_ASSERT(false);
        return false;
    }

    bool operator()(const QPair<const QFileSystemModelPrivate::QFileSystemNode*, int> &l,
                           const QPair<const QFileSystemModelPrivate::QFileSystemNode*, int> &r) const
    {
        return compareNodes(l.first, r.first);
    }


private:
    int sortColumn;
};

/*
    \internal

    Sort all of the children of parent (including their children)
*/
void QFileSystemModelPrivate::sortChildren(int column, Qt::SortOrder order, const QModelIndex &parent)
{
    Q_Q(QFileSystemModel);
    QFileSystemModelPrivate::QFileSystemNode *indexNode = node(parent);
    if (indexNode->children.count() == 0)
        return;

    QList<QPair<const QFileSystemModelPrivate::QFileSystemNode*, int> > values;
    for (int i = 0; i < indexNode->children.count(); ++i) {
        if (filtersAcceptsNode(&indexNode->children.at(i)))
            values.append(QPair<const QFileSystemModelPrivate::QFileSystemNode*, int>(&(indexNode->children[i]), i));
    }
    QFileSystemModelSorter ms(column);
    qStableSort(values.begin(), values.end(), ms);
    // First update the new visible list
    indexNode->visibleChildren.clear();
    for (int i = 0; i < values.count(); ++i)
        indexNode->visibleChildren.append(values.at(i).second);

    for (int i = 0; i < q->rowCount(parent); ++i)
        sortChildren(column, order, q->index(i, 0, parent));
}

/*!
    \reimp
*/
void QFileSystemModel::sort(int column, Qt::SortOrder order)
{
    Q_D(QFileSystemModel);
    if (d->sortOrder == order && d->sortColumn == column && !d->forceSort)
        return;

    emit layoutAboutToBeChanged();
    QModelIndexList oldList = persistentIndexList();
    QList<QPair<QFileSystemModelPrivate::QFileSystemNode*, int> > oldNodes;
    for (int i = 0; i < oldList.count(); ++i) {
        QPair<QFileSystemModelPrivate::QFileSystemNode*, int> pair(d->node(oldList.at(i)), oldList.at(i).column());
        oldNodes.append(pair);
    }

    if (!(d->sortColumn == column && d->sortOrder != order && !d->forceSort)) {
        d->sortChildren(column, order, QModelIndex());
        d->sortColumn = column;
        d->forceSort = false;
    }
    d->sortOrder = order;

    QModelIndexList newList;
    for (int i = 0; i < oldNodes.count(); ++i) {
        QModelIndex idx = d->index(oldNodes.at(i).first);
        idx = idx.sibling(idx.row(), oldNodes.at(i).second);
        newList.append(idx);
    }
    changePersistentIndexList(oldList, newList);
    emit layoutChanged();
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/
QStringList QFileSystemModel::mimeTypes() const
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
QMimeData *QFileSystemModel::mimeData(const QModelIndexList &indexes) const
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
bool QFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!parent.isValid() || isReadOnly())
        return false;

    bool success = true;
    QString to = filePath(parent) + QDir::separator();

    QList<QUrl> urls = data->urls();
    QList<QUrl>::const_iterator it = urls.constBegin();

    switch (action) {
    case Qt::CopyAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::LinkAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::MoveAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName())
                      && QFile::remove(path) && success;
        }
        break;
    default:
        return false;
    }

    return success;
}

/*!
    \reimp
*/
Qt::DropActions QFileSystemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

/*!
    Returns the path of the item stored in the model under the
    \a index given.
*/
QString QFileSystemModel::filePath(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    if (!index.isValid())
        return QString();
    Q_ASSERT(index.model() == this);

    QStringList path;
    QModelIndex idx = index;
    while (idx.isValid()) {
        path.prepend(d->name(idx));
        idx = idx.parent();
    }
    QString fullPath = path.join(QDir::separator());
#ifndef Q_OS_WIN
    if ((fullPath.length() > 2) && fullPath[0] == QLatin1Char('/') && fullPath[1] == QLatin1Char('/'))
        fullPath = fullPath.mid(1);
#endif
    return QDir::toNativeSeparators(fullPath);
}

/*!
    Create a directory with the name in the parent model item
*/
QModelIndex QFileSystemModel::mkdir(const QModelIndex &parent, const QString &name)
{
    Q_D(QFileSystemModel);
    if (!parent.isValid())
        return parent;

    QDir dir(filePath(parent));
    if (!dir.mkdir(name))
        return QModelIndex();
    QFileSystemModelPrivate::QFileSystemNode *parentNode = d->node(parent);
    d->addNode(parentNode, name);
    int r = d->findChild(parentNode, QFileSystemModelPrivate::QFileSystemNode(name));
    Q_ASSERT(r >= 0);
    QFileSystemModelPrivate::QFileSystemNode *node = &parentNode->children[r];
    node->populate(d->fileInfoGatherer.getInfo(QFileInfo(dir.absolutePath() + QDir::separator() + name)));
    d->addVisibleFiles(parentNode, QStringList(name));
    return d->index(node);
}

QFile::Permissions QFileSystemModel::permissions(const QModelIndex &index) const
{
    Q_D(const QFileSystemModel);
    return d->node(index)->permissions();
}


/*!
    Sets the directory that is being watched by the model.
    If the path is changed the model will be reset.
  */
QModelIndex QFileSystemModel::setRootPath(const QString &newPath)
{
    Q_D(QFileSystemModel);
    if (d->rootDir.path() == newPath)
        return d->index(rootPath());

    QDir newPathDir(newPath);
    bool showDrives = (newPath.isEmpty() || newPath == d->myComputer());
    if (!showDrives && !newPathDir.exists())
        return d->index(rootPath());

    // We have a new valid root path
    d->rootDir = newPathDir;
    QModelIndex newRootIndex;
    if (showDrives) {
        // otherwise dir will become '.'
        d->rootDir.setPath(QLatin1String(""));
    } else {
        newRootIndex = d->index(newPathDir.path());
    }
    fetchMore(newRootIndex);
    emit rootPathChanged(newPath);
    d->forceSort = true;
    d->delayedSort();
    return newRootIndex;
}

/*!
    The currently set root path

    \sa rootDirectory()
*/
QString QFileSystemModel::rootPath() const
{
    Q_D(const QFileSystemModel);
    return d->rootDir.path();
}

/*!
    The currently set directory

    \sa rootPath();
*/
QDir QFileSystemModel::rootDirectory() const
{
    Q_D(const QFileSystemModel);
    QDir dir(d->rootDir);
    dir.setNameFilters(nameFilters());
    dir.setFilter(filter());
    return dir;
}

/*!
    Sets the \a provider of file icons for the directory model.
*/
void QFileSystemModel::setIconProvider(QFileIconProvider *provider)
{
    Q_D(QFileSystemModel);
    d->fileInfoGatherer.setIconProvider(provider);
}

/*!
    Returns the file icon provider for this directory model.
*/
QFileIconProvider *QFileSystemModel::iconProvider() const
{
    Q_D(const QFileSystemModel);
    return d->fileInfoGatherer.iconProvider();
}

/*!
    Sets the directory model's filter to that specified by \a filters.

    Note that the filter you set should always include the QDir::AllDirs enum value,
    otherwise QFileSystemModel won't be able to read the directory structure.

    \sa QDir::Filters
*/
void QFileSystemModel::setFilter(QDir::Filters filters)
{
    Q_D(QFileSystemModel);
    d->filters = filters;
    // CaseSensitivity might have changed
    setNameFilters(nameFilters());
    d->forceSort = true;
    d->delayedSort();
}

/*!
    Returns the filter specification for the directory model.

    \sa QDir::Filters
*/
QDir::Filters QFileSystemModel::filter() const
{
    Q_D(const QFileSystemModel);
    return d->filters;
}

/*!
    \property QFileSystemModel::resolveSymlinks
    \brief Whether the directory model should resolve symbolic links

    This is only relevant on operating systems that support symbolic links.
*/
void QFileSystemModel::setResolveSymlinks(bool enable)
{
    Q_D(QFileSystemModel);
    d->fileInfoGatherer.setResolveSymlinks(enable);
}

bool QFileSystemModel::resolveSymlinks() const
{
    Q_D(const QFileSystemModel);
    return d->fileInfoGatherer.resolveSymlinks();
}

/*!
    \property QFileSystemModel::readOnly
    \brief Whether the directory model allows writing to the file system

    If this property is set to false, the directory model will allow renaming, copying
    and deleting of files and directories.

    This property is true by default
*/
void QFileSystemModel::setReadOnly(bool enable)
{
    Q_D(QFileSystemModel);
    d->readOnly = enable;
}

bool QFileSystemModel::isReadOnly() const
{
    Q_D(const QFileSystemModel);
    return d->readOnly;
}

/*!
    \property QFileSystemModel::nameFilterDisables
    \brief Whether files that don't pass the name filter are hidden or disabled

    This property is true by default
*/
void QFileSystemModel::setNameFilterDisables(bool enable)
{
    Q_D(QFileSystemModel);
    if (d->nameFilterDisables == enable)
        return;
    d->nameFilterDisables = enable;
    d->forceSort = true;
    d->delayedSort();
}

bool QFileSystemModel::nameFilterDisables() const
{
    Q_D(const QFileSystemModel);
    return d->nameFilterDisables;
}

/*!
    Sets the name \a filters to apply against the exisiting files.
*/
void QFileSystemModel::setNameFilters(const QStringList &filters)
{
    // Prep the regexp's ahead of time
#ifndef QT_NO_REGEXP
    Q_D(QFileSystemModel);
    d->nameFilters.clear();
    const Qt::CaseSensitivity caseSensitive =
        (filter() & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
    for (int i = 0; i < filters.size(); ++i) {
        d->nameFilters << QRegExp(filters.at(i), caseSensitive, QRegExp::Wildcard);
    }
    d->forceSort = true;
    d->delayedSort();
#endif
}

/*!
    Returns a list of filters applied to the names in the model.
*/
QStringList QFileSystemModel::nameFilters() const
{
    Q_D(const QFileSystemModel);
    QStringList filters;
#ifndef QT_NO_REGEXP
    for (int i = 0; i < d->nameFilters.size(); ++i) {
         filters << d->nameFilters.at(i).pattern();
    }
#endif
    return filters;
}

/*!
     \internal

    Performed quick listing and see if any files have been added or removed,
    then fetch more information on visible files.
 */
void QFileSystemModelPrivate::_q_directoryChanged(const QString &directory, const QStringList &files)
{
    QFileSystemModelPrivate::QFileSystemNode *parentNode = node(directory, false);
    QStringList newFiles = files;
    // sort new files for faster inserting
    qSort(newFiles.begin(), newFiles.end());

    // Ignore files we already have and cleanup filtered files that were removed.
    // non-filtered files will be removed in a fileSystemChanged()
    for (int i = parentNode->children.count() - 1;  i >= 0; --i) {
        QStringList::const_iterator iterator;
        if (parentNode->caseSensitive())
            iterator = qBinaryFind(newFiles.begin(), newFiles.end(),
                       parentNode->children.at(i).fileName);
        else
            iterator = qBinaryFind(newFiles.begin(), newFiles.end(),
                       parentNode->children.at(i).fileName, caseInsensitiveLessThan);
        if (iterator == newFiles.constEnd()) {
           removeNode(parentNode, i);
        }
    }
}

/*!
    \internal

    Adds a new file to the children of parentNode

    *WARNING* this will change the count of children
*/
int QFileSystemModelPrivate::addNode(QFileSystemNode *parentNode, const QString &fileName)
{
    // In the common case, itemLocation == count() so check there first
    QFileSystemModelPrivate::QFileSystemNode node(fileName, parentNode);
    int itemLocation = 0;
    if (parentNode->children.count() > 0) {
        if (parentNode->children.at(parentNode->children.count() - 1).hasInformation()
            && parentNode->children.at(parentNode->children.count() - 1) < fileName) {
            itemLocation = parentNode->children.count();
        } else {
            itemLocation = findWhereToInsertChild(parentNode, &node);
        }
    }
    parentNode->children.insert(itemLocation, node);

    for (int i = 0; i < parentNode->visibleChildren.count(); ++i)
        if (parentNode->visibleChildren.at(i) >= itemLocation)
            ++parentNode->visibleChildren[i];
    return itemLocation;
}

/*!
    \internal

    File at parentNode->children(itemLocation) has been removed, remove from the lists
    and emit signals if necessary

    *WARNING* this will change the count of children and could change visibleChildren
 */
void QFileSystemModelPrivate::removeNode(QFileSystemModelPrivate::QFileSystemNode *parentNode, int itemLocation)
{
    Q_Q(QFileSystemModel);
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);

    int vLocation = parentNode->visibleLocation(itemLocation);
    if (vLocation >= 0 && !indexHidden)
        q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                                       translateVisibleLocation(parentNode, vLocation));
    parentNode->children.removeAt(itemLocation);
    // cleanup sort files after removing rather then re-sorting which is O(n)
    if (vLocation >= 0)
        parentNode->visibleChildren.removeAt(vLocation);
    // update all the rest of the visible children
    for (int j = 0; j < parentNode->visibleChildren.count(); ++j)
        if (parentNode->visibleChildren.at(j) > itemLocation)
            --parentNode->visibleChildren[j];
    if (vLocation >= 0 && !indexHidden)
        q->endRemoveRows();
}

/*
    \internal
    Helper functor used by addVisibleFiles()
*/
class QFileSystemModelVisibleFinder
{
public:
    inline QFileSystemModelVisibleFinder(QFileSystemModelPrivate::QFileSystemNode *node, QFileSystemModelSorter *sorter) : parentNode(node), sorter(sorter) {}

    bool operator()(const QString &, int r) const
    {
        return sorter->compareNodes(&(parentNode->children.at(location)), &(parentNode->children.at(r)));
    }

    int location;
private:
    QFileSystemModelPrivate::QFileSystemNode *parentNode;
    QFileSystemModelSorter *sorter;
};

/*!
    \internal

    File at parentNode->children(itemLocation) was not visible before, but now should be
    and emit signals if necessary.

    *WARNING* this will change the visible count
 */
void QFileSystemModelPrivate::addVisibleFiles(QFileSystemNode *parentNode, const QStringList &newFiles)
{
    Q_Q(QFileSystemModel);
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);

    QFileSystemModelSorter sorter(sortColumn);
    QFileSystemModelVisibleFinder vf(parentNode, &sorter);
    for (int i = 0; i < newFiles.count(); ++i) {
        QString newFile = newFiles.at(i);
        int location = findChild(parentNode, QFileSystemNode(newFile));
        Q_ASSERT(location >= 0);

        vf.location = location;
        QList<int>::const_iterator iterator = qUpperBound(parentNode->visibleChildren.begin(), parentNode->visibleChildren.end(), newFile, vf);
        int realHome = (iterator - parentNode->visibleChildren.begin());
        if (!indexHidden)
            q->beginInsertRows(parent, realHome, realHome);
        parentNode->visibleChildren.insert(realHome, location);
    if (!indexHidden)
        q->endInsertRows();
    }
}

/*!
    \internal

    File was visible before, but now should NOT be

    *WARNING* this will change the visible count
 */
void QFileSystemModelPrivate::removeVisibleFile(QFileSystemNode *parentNode, int vLocation)
{
    Q_Q(QFileSystemModel);
    if (vLocation == -1)
        return;
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);
    if (!indexHidden)
        q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                                       translateVisibleLocation(parentNode, vLocation));
    parentNode->visibleChildren.removeAt(vLocation);
    if (!indexHidden)
        q->endRemoveRows();
}

/*!
    \internal

    The thread has received new information about files,
    update and emit dataChanged if it has actually changed.
 */
void QFileSystemModelPrivate::_q_fileSystemChanged(const QString &path, const QList<QPair<QString, QExtendedInformation> > &updates)
{
    Q_Q(QFileSystemModel);
    QVector<int> rowsToUpdate;
    QStringList newFiles;
    QFileSystemModelPrivate::QFileSystemNode *parentNode = node(path, false);
    QModelIndex parentIndex = index(parentNode);
    for (int i = 0; i < updates.count(); ++i) {
        QString fileName = updates.at(i).first;
        Q_ASSERT(!fileName.isEmpty());
        QExtendedInformation info = updates.at(i).second;
        int itemLocation = findChild(parentNode, QFileSystemNode(fileName));
        if (itemLocation < 0) {
            itemLocation = addNode(parentNode, fileName);
            for (int i = 0; i < rowsToUpdate.count(); ++i)
                if (rowsToUpdate.at(i) >= itemLocation)
                    ++rowsToUpdate[i];
        }
        if (parentNode->caseSensitive()) {
            if (parentNode->children.at(itemLocation).fileName != fileName)
                continue;
        } else {
            if (parentNode->children.at(itemLocation).fileName.toLower() != fileName.toLower())
                continue;
        }

        if (parentNode->caseSensitive()) {
            Q_ASSERT(parentNode->children.at(itemLocation).fileName == fileName);
        } else {
            parentNode->children[itemLocation].fileName = fileName;
        }

        if (info.size == -1) {
            removeNode(parentNode, itemLocation);
            continue;
        }

        if (parentNode->children.at(itemLocation) != info ) {
            parentNode->children[itemLocation].populate(info);
            int visibleLocation = parentNode->visibleLocation(itemLocation);
            bypassFilters.removeAll(&(parentNode->children.at(itemLocation)));
            // brand new information.
            if (filtersAcceptsNode(&(parentNode->children[itemLocation]))) {
                if (visibleLocation == -1) {
                    newFiles.append(fileName);
                } else {
                    rowsToUpdate.append(itemLocation);
                }
            } else {
                if (visibleLocation != -1) {
                    removeVisibleFile(parentNode, visibleLocation);
                } else {
                    // The file is not visible, don't do anything
                }
            }
        }
    }

    // bundle up all of the changed signals into as few as possible.
    qSort(rowsToUpdate.begin(), rowsToUpdate.end());
    int min = -2; // ### TODO double check that -2 isn't needed
    int max = -2;
    for (int i = 0; i < rowsToUpdate.count(); ++i) {
        int value = rowsToUpdate.at(i);
        if (min == -2) {
            min = value;
            if (i != rowsToUpdate.count() - 1)
                continue;
        }
        if (i != rowsToUpdate.count() - 1) {
            if ((value == min + 1 && max == -2) || value == max + 1) {
                max = value;
                continue;
            }
        }
        int visibleMin = parentNode->visibleLocation(min);
        Q_ASSERT(parentNode->visibleChildren.at(visibleMin) == min);
        int visibleMax = max >= 0 ? parentNode->visibleLocation(max) : visibleMin;
        Q_ASSERT(visibleMin >= 0 && visibleMax >= 0);
        QModelIndex bottom = q->index(translateVisibleLocation(parentNode, visibleMin), 0, parentIndex);
        QModelIndex top = q->index(translateVisibleLocation(parentNode, visibleMax), 3, parentIndex);
        emit q->dataChanged(bottom, top);
        min = -2;
        max = -2;
    }

    if (newFiles.count() > 0) {
        addVisibleFiles(parentNode, newFiles);
    }

    if (sortColumn != 0 && rowsToUpdate.count() > 0) {
        forceSort = true;
        delayedSort();
    }
}

/*!
    \internal
*/
void QFileSystemModelPrivate::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
    resolvedSymLinks[fileName] = resolvedName;
}

/*!
    \internal
*/
void QFileSystemModelPrivate::init()
{
    Q_Q(QFileSystemModel);
    qRegisterMetaType<QList<QPair<QString,QExtendedInformation> > >("QList<QPair<QString,QExtendedInformation> >");
    q->connect(&fileInfoGatherer, SIGNAL(newListOfFiles(const QString &, const QStringList &)),
               q, SLOT(_q_directoryChanged(const QString &, const QStringList &)));
    q->connect(&fileInfoGatherer, SIGNAL(updates(const QString &, const QList<QPair<QString, QExtendedInformation> > &)),
            q, SLOT(_q_fileSystemChanged(const QString &, const QList<QPair<QString, QExtendedInformation> > &)));
    q->connect(&fileInfoGatherer, SIGNAL(nameResolved(const QString &, const QString &)),
            q, SLOT(_q_resolvedName(const QString &, const QString &)));
    q->connect(&delayedSortTimer, SIGNAL(timeout()), q, SLOT(_q_performDelayedSort()), Qt::QueuedConnection);
}

/*!
    \internal

    Returns false if node doesn't pass the filters otherwise true

    QDir::Modified is not supported
    QDir::Drives is not supported
*/
bool QFileSystemModelPrivate::filtersAcceptsNode(const QFileSystemNode *node) const
{
    // always accept drives
    if (node->parent == &root || bypassFilters.contains(node))
        return true;

    // If we don't know anything yet don't accept it
    if (!node->hasInformation())
        return false;

    const bool filterPermissions = ((filters & QDir::PermissionMask)
                                   && (filters & QDir::PermissionMask) != QDir::PermissionMask);
    const bool hideDirs          = !(filters & (QDir::Dirs | QDir::AllDirs));
    const bool hideFiles         = !(filters & QDir::Files);
    const bool hideReadable      = !(!filterPermissions || (filters & QDir::Readable));
    const bool hideWritable      = !(!filterPermissions || (filters & QDir::Writable));
    const bool hideExecutable    = !(!filterPermissions || (filters & QDir::Executable));
    const bool hideHidden        = !(filters & QDir::Hidden);
    const bool hideSystem        = !(filters & QDir::System);
    const bool hideSymlinks      = (filters & QDir::NoSymLinks);
    const bool hideDotAndDotDot  = (filters & QDir::NoDotAndDotDot);

    // Note that we match the behavior of entryList and not QFileInfo on this and this
    // incompatibility won't be fixed until Qt 5 at least
    bool isDotOrDot = (  (node->fileName == QLatin1String(".")
                       || node->fileName == QLatin1String("..")));
    if (   (hideHidden && (!isDotOrDot && node->isHidden()))
        || (hideSystem && node->isSystem())
        || (hideDirs && node->isDir())
        || (hideFiles && node->isFile())
        || (hideSymlinks && node->isSymLink())
        || (hideReadable && node->isReadable())
        || (hideWritable && node->isWritable())
        || (hideExecutable && node->isExecutable())
        || (hideDotAndDotDot && isDotOrDot))
        return false;

    return nameFilterDisables || passNameFilters(node);
}

/*
    \internal

    Returns true if node passes the name filters and should be visible.
 */
bool QFileSystemModelPrivate::passNameFilters(const QFileSystemNode *node) const
{
#ifndef QT_NO_REGEXP
    if (nameFilters.isEmpty())
        return true;

    // Check the name regularexpression filters
    if (!(node->isDir() && (filters & QDir::AllDirs))) {
        for (int i = 0; i < nameFilters.size(); ++i) {
            if (nameFilters.at(i).exactMatch(node->fileName))
                return true;
        }
        return false;
    }
#endif
    return true;
}

//#include "moc_qfilesystemmodel.cpp"
