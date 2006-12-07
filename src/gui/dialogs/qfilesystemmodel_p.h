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
#ifndef QFILESYSTEMMODEL_H
#define QFILESYSTEMMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpair.h>
#include <QtCore/qdir.h>
#include <QtGui/qicon.h>

class ExtendedInformation;
class QFileSystemModelPrivate;
class QFileIconProvider;

class Q_AUTOTEST_EXPORT QFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool resolveSymlinks READ resolveSymlinks WRITE setResolveSymlinks)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool nameFilterDisables READ nameFilterDisables WRITE setNameFilterDisables)

Q_SIGNALS:
    void rootPathChanged(const QString &newPath);

public:
    enum Roles {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole
    };

    explicit QFileSystemModel(QObject *parent = 0);
    ~QFileSystemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const QString &path, int column = 0) const;
    QModelIndex parent(const QModelIndex &child) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant myComputer(int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    // QFileSystemModel specific API
    QModelIndex setRootPath(const QString &path);
    QString rootPath() const;
    QDir rootDirectory() const;

    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

    void setFilter(QDir::Filters filters);
    QDir::Filters filter() const;

    void setResolveSymlinks(bool enable);
    bool resolveSymlinks() const;

    void setReadOnly(bool enable);
    bool isReadOnly() const;

    void setNameFilterDisables(bool enable);
    bool nameFilterDisables() const;

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;

    QString filePath(const QModelIndex &index) const;
    bool isDir(const QModelIndex &index) const;
    int size(const QModelIndex &index) const;
    QString type(const QModelIndex &index) const;
    QDateTime lastModified(const QModelIndex &index) const;

    QModelIndex mkdir(const QModelIndex &parent, const QString &name);
    inline bool rmdir(const QModelIndex &index) { QDir dir; return dir.rmdir(filePath(index)); }
    inline QString fileName(const QModelIndex &index) const { return index.data(Qt::DisplayRole).toString(); }
    inline QIcon fileIcon(const QModelIndex &index) const { return qvariant_cast<QIcon>(index.data(Qt::DecorationRole)); }
    QFile::Permissions permissions(const QModelIndex &index) const;
    inline QFileInfo fileInfo(const QModelIndex &index) const { return QFileInfo(filePath(index)); }
    inline bool remove(const QModelIndex &index) { if (isDir(index)) return rmdir(index); else return QFile::remove(filePath(index)); }

protected:
    QFileSystemModel(QFileSystemModelPrivate &, QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(QFileSystemModel)
    Q_DISABLE_COPY(QFileSystemModel)

    Q_PRIVATE_SLOT(d_func(), void directoryChanged(const QString &directory, const QStringList &list));
    Q_PRIVATE_SLOT(d_func(), void performDelayedSort());
    Q_PRIVATE_SLOT(d_func(), void fileSystemChanged(const QString &path, const QList<QPair<QString, QExtendedInformation> > &));
    Q_PRIVATE_SLOT(d_func(), void resolvedName(const QString &fileName, const QString &resolvedName));
};

#endif // QFILESYSTEMMODEL_H

#ifndef QFLATDIRMODEL_P_H
#define QFLATDIRMODEL_P_H

#include <qabstractitemmodel.h>
#include <private/qabstractitemmodel_p.h>
#include "qfileinfogatherer_p.h"
#include <qdir.h>
#include <qicon.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qhash.h>

class Q_AUTOTEST_EXPORT QFileSystemModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QFileSystemModel)

public:
    class QFileSystemNode
    {
    public:
        QFileSystemNode(const QString &filename = QString(), QFileSystemNode *p=0)
            : fileName(filename), populatedChildren(false), parent(p), info(0) {}
        ~QFileSystemNode() { delete info; }

        QString fileName;

        inline int size() const { if (info && !info->isDir()) return info->size; return 0; }
        inline QString type() const { if (info) return info->displayType; return QLatin1String(""); }
        inline QDateTime lastModified() const { if (info) return info->lastModified; return QDateTime(); }
        inline QFile::Permissions permissions() const { if (info) return info->permissions; return 0; }
        inline bool isReadable() const { return (permissions() & QFile::ReadUser); }
        inline bool isWritable() const { return (permissions() & QFile::WriteUser); }
        inline bool isExecutable() const { return (permissions() & QFile::ExeUser); }
        inline bool isDir() const {
            if (info)
                return info->isDir();
            if (children.count() > 0)
                return true;
            return false;
        }
        inline bool isFile() const { if (info) return info->isFile(); return true; }
        inline bool isSystem() const { if (info) return info->isSystem(); return true; }
        inline bool isHidden() const { if (info) return info->isHidden; return false; }
        inline bool isSymLink() const { if (info) return info->isSymLink; return false; }
        inline bool caseSensitive() const { if (info) return info->caseSensitive; return true; }
        inline QIcon icon() const { if (info) return info->icon; return QIcon(); }

        inline bool operator <(const QFileSystemNode &node) const {
	    if (caseSensitive())
                return fileName < node.fileName;
            return fileName.toLower() < node.fileName.toLower();
        }
        inline bool operator >(const QString &name) const {
            if (caseSensitive())
                return fileName > name;
            return fileName.toLower() > name.toLower();
        }
        inline bool operator <(const QString &name) const {
            if (caseSensitive())
                return fileName < name;
            return fileName.toLower() < name.toLower();
        }
        inline bool operator !=(const QExtendedInformation &fileInfo) const {
            return !operator==(fileInfo);
        }
        bool operator ==(const QString &name) const {
            if (caseSensitive())
                return fileName == name;
            return fileName.toLower() == name.toLower();
        }
        bool operator ==(const QExtendedInformation &fileInfo) const {
            return info && (*info == fileInfo);
        }

        inline bool hasInformation() const { return info != 0; }

        void populate(const QExtendedInformation &fileInfo) {
            if (!info)
                info = new QExtendedInformation();
            (*info) = fileInfo;
        }

        // children shouldn't normally be accessed directly, use node()
        inline int visibleLocation(int childRow) {
            return visibleChildren.indexOf(childRow);
        }
        bool populatedChildren;
        QList<QFileSystemNode> children;
        QList<int> visibleChildren;
        QFileSystemNode *parent;

    private:
        QExtendedInformation *info;

    };

    QFileSystemModelPrivate() :
            forceSort(true),
            sortColumn(0),
            sortOrder(Qt::AscendingOrder),
            readOnly(true),
            filters(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs),
            nameFilterDisables(true) // false on windows, true on mac and unix
    {
        delayedSortTimer.setSingleShot(true);
    }

    void init();
    /*
      \internal

      Return true if index which is owned by node is hidden by the filter.
    */
    inline bool isHiddenByFilter(QFileSystemNode *indexNode, const QModelIndex &index) const
    {
       return (indexNode != &root && !index.isValid());
    }
    QFileSystemNode *node(const QModelIndex &index) const;
    QFileSystemNode *node(const QString &path, bool shouldExist = true) const;
    inline QModelIndex index(const QString &path) { return index(node(path)); }
    QModelIndex index(const QFileSystemNode *node) const;
    bool filtersAcceptsNode(const QFileSystemNode *node) const;
    bool passNameFilters(const QFileSystemNode *node) const;
    void removeNode(QFileSystemNode *parentNode, int itemLocation);
    int addNode(QFileSystemNode *parentNode, const QString &fileName);
    void addVisibleFiles(QFileSystemNode *parentNode, const QStringList &newFiles);
    void removeVisibleFile(QFileSystemNode *parentNode, int visibleLocation);
    void sortChildren(int column, Qt::SortOrder order, const QModelIndex &parent, bool filter);

    inline int translateVisibleLocation(QFileSystemNode *parent, int row) const {
        return (sortOrder == Qt::AscendingOrder) ? row : parent->visibleChildren.count() - row - 1;
    }

    inline static QString myComputer() {
        // ### TODO We should query the system to find out what the string should be
        // XP == "My Computer",
        // Vista == "Computer",
        // OS X == (user generated) "Benjamin's PowerBook G4"
        return QFileSystemModel::tr("My Computer");
    }

    inline void delayedSort() {
        if (!delayedSortTimer.isActive())
            delayedSortTimer.start(0);
    }

    static bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
    {
       return s1.toLower() < s2.toLower();
    }

    static bool nodeCaseInsensitiveLessThan(const QFileSystemModelPrivate::QFileSystemNode &s1, const QFileSystemModelPrivate::QFileSystemNode &s2)
    {
       return s1.fileName.toLower() < s2.fileName.toLower();
    }

    inline int findChild(const QFileSystemNode *parent, const QFileSystemNode &node) const {
        QList<QFileSystemNode>::const_iterator iterator;
        if (parent->caseSensitive())
	    iterator = (qBinaryFind(parent->children.begin(), parent->children.end(), node));
	else
	    iterator = (qBinaryFind(parent->children.begin(), parent->children.end(), node, nodeCaseInsensitiveLessThan));
        if (iterator == parent->children.end())
            return -1;
        int location = (iterator - parent->children.begin());
        Q_ASSERT(location < parent->children.count());
        return location;
    }

    inline int findWhereToInsertChild(const QFileSystemNode *parent, const QFileSystemNode *node) const {
        QList<QFileSystemNode>::const_iterator iterator;
        if (parent->caseSensitive()) {
	    iterator = (qUpperBound(parent->children.begin(), parent->children.end(), *node));
	} else
	    iterator = (qUpperBound(parent->children.begin(), parent->children.end(), *node, QFileSystemModelPrivate::nodeCaseInsensitiveLessThan));
        return (iterator - parent->children.begin());
    }

    QIcon icon(const QModelIndex &index) const;
    QString name(const QModelIndex &index) const;
    QString size(const QModelIndex &index) const;
    QString type(const QModelIndex &index) const;
    QString time(const QModelIndex &index) const;

    void directoryChanged(const QString &directory, const QStringList &list);
    void performDelayedSort();
    void fileSystemChanged(const QString &path, const QList<QPair<QString, QExtendedInformation> > &);
    void resolvedName(const QString &fileName, const QString &resolvedName);

    static int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs);

    QDir rootDir;
    QFileInfoGatherer fileInfoGatherer;
    QTimer delayedSortTimer;
    bool forceSort;
    int sortColumn;
    Qt::SortOrder sortOrder;
    bool readOnly;
    QDir::Filters filters;
    bool nameFilterDisables;
#ifndef QT_NO_REGEXP
    QList<QRegExp> nameFilters;
#endif
    // ### resolvedSymLinks goes away in Qt5
    QHash<QString, QString> resolvedSymLinks;

    QFileSystemNode root;
};

#endif

