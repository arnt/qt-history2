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
public:
    QFileIconProvider();
    virtual ~QFileIconProvider();
    virtual QIcon computerIcon() const;
    virtual QIcon icon(const QFileInfo &info) const;
    virtual QString type(const QFileInfo &info) const;

private:
    Q_DECLARE_PRIVATE(QFileIconProvider)
    QFileIconProviderPrivate *d_ptr;
};

class Q_GUI_EXPORT QDirModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool resolveSymlinks READ resolveSymlinks WRITE setResolveSymlinks)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)

public:
    QDirModel(const QString &path = QString::null,
              const QStringList &nameFilters = QStringList(),
              QDir::Filters filters = QDir::NoFilter,
              QDir::SortFlags sort = QDir::NoSort,
              QObject *parent = 0);
    QDirModel(const QDir &directory, QObject *parent = 0);
    ~QDirModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool hasChildren(const QModelIndex &index) const;
    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const; // specific for this model

    void sort(int column, Qt::SortOrder order);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, QDrag::DropAction action,
                      int row, const QModelIndex &parent);
    QDrag::DropActions supportedDropActions() const;

    // QDirModel specific API

    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
 
    void setFilter(QDir::Filters filters);
    QDir::Filters filter() const;

    void setSorting(QDir::SortFlags sort);
    QDir::SortFlags sorting() const;

    void setResolveSymlinks(bool enable);
    bool resolveSymlinks() const;

    void setReadOnly(bool enable);
    bool isReadOnly() const;

    void refresh(const QModelIndex &parent = QModelIndex());

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

private:
    Q_DECLARE_PRIVATE(QDirModel)
    Q_DISABLE_COPY(QDirModel)
};

#endif //QDIRMODEL_H
