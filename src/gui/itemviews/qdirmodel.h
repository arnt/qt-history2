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
#include <qfileinfo.h>
#include <qdir.h>

class QDirModelPrivate;

class Q_GUI_EXPORT QFileIconProvider
{
public:
    QFileIconProvider();
    virtual ~QFileIconProvider();
    virtual QIconSet computerIcons() const;
    virtual QIconSet icons(const QFileInfo &info) const;
    virtual QString type(const QFileInfo &info) const;
protected:
    QIconSet file;
    QIconSet dir;
    QIconSet driveHD;
    QIconSet computer;
    QIconSet fileLink;
    QIconSet dirLink;
};

class Q_GUI_EXPORT QDirModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDirModel)

public:
    QDirModel(const QString &path = QString::null,
              const QStringList &nameFilters = QStringList(),
              int filter = QDir::DefaultFilter,
              int sorting = QDir::DefaultSort,
              QObject *parent = 0);
    QDirModel(const QDir &directory, QObject *parent = 0);
    ~QDirModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::invalid,
                      QModelIndex::Type type = QModelIndex::View) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool hasChildren(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;
    bool isDragEnabled(const QModelIndex &index) const;
    bool isDropEnabled(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, Qt::SortOrder order);

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

    void refresh(const QModelIndex &parent = QModelIndex::invalid);

    QModelIndex index(const QString &path) const;
    QString path(const QModelIndex &index) const;
    QString name(const QModelIndex &index) const;
    QIconSet icons(const QModelIndex &index) const;
    QFileInfo fileInfo(const QModelIndex &index) const;

    bool isDir(const QModelIndex &index) const;
    QModelIndex mkdir(const QModelIndex &parent, const QString &name);
    bool rmdir(const QModelIndex &index);
    bool remove(const QModelIndex &index);

protected:
    QDirModel(QDirModelPrivate &, const QDir &directory, QObject *parent = 0);
};

#endif //QDIRMODEL_H
