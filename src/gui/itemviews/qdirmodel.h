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

#ifndef QDIRMODEL_H
#define QDIRMODEL_H

#include <qabstractitemmodel.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qicon.h>

class QDirModelPrivate;
class QFileIconProviderPrivate;

class Q_GUI_EXPORT QFileIconProvider
{
    Q_DECLARE_PRIVATE(QFileIconProvider)
public:
    QFileIconProvider();
    virtual ~QFileIconProvider();
    virtual QIcon computerIcon() const;
    virtual QIcon icon(const QFileInfo &info) const;
    virtual QString type(const QFileInfo &info) const;

private:
    QFileIconProviderPrivate *d_ptr;
};

class Q_GUI_EXPORT QDirModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDirModel)
    Q_PROPERTY(bool resolveSymlinks READ resolveSymlinks WRITE setResolveSymlinks)

public:
    QDirModel(const QString &path = QString::null,
              const QStringList &nameFilters = QStringList(),
              int filter = QDir::DefaultFilter,
              int sorting = QDir::DefaultSort,
              QObject *parent = 0);
    QDirModel(const QDir &directory, QObject *parent = 0);
    ~QDirModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool hasChildren(const QModelIndex &index) const;
    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const; // specific for this model
    
    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

    bool equal(const QModelIndex &left, const QModelIndex &right) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    bool canDecode(QMimeSource *src) const;
    bool decode(QDropEvent *e, const QModelIndex &parent);
    QDragObject *dragObject(const QModelIndexList &indices, QWidget *dragSource);

    // QDirModel specific API

    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void setFilter(int spec);
    QDir::FilterSpec filter() const;
    void setSorting(int spec);
    QDir::SortSpec sorting() const;

    void setResolveSymlinks(bool enable);
    bool resolveSymlinks() const;

    void refresh(const QModelIndex &parent = QModelIndex::Null);

    QModelIndex index(const QString &path) const;
    QString path(const QModelIndex &index) const;
    QString name(const QModelIndex &index) const;
    QIcon icon(const QModelIndex &index) const;
    QFileInfo fileInfo(const QModelIndex &index) const;

    bool isDir(const QModelIndex &index) const;
    QModelIndex mkdir(const QModelIndex &parent, const QString &name);
    bool rmdir(const QModelIndex &index);
    bool remove(const QModelIndex &index);

protected:
    QDirModel(QDirModelPrivate &, const QDir &directory, QObject *parent = 0);
};

#endif //QDIRMODEL_H
