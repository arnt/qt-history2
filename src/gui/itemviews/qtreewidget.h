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

#ifndef QTREEWIDGET_H
#define QTREEWIDGET_H

#include <QtGui/qtreeview.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

class QTreeWidget;
class QTreeModel;
class QWidgetItemData;

class Q_GUI_EXPORT QTreeWidgetItem
{
    friend class QTreeModel;
public:
    explicit QTreeWidgetItem(QTreeWidget *view);
    QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after);
    explicit QTreeWidgetItem(QTreeWidgetItem *parent);
    QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after);
    virtual ~QTreeWidgetItem();

    virtual QTreeWidgetItem *clone() const;

    inline QTreeWidget *treeWidget() const { return view; }

    inline QAbstractItemModel::ItemFlags flags() const { return itemFlags; }
    inline void setFlags(QAbstractItemModel::ItemFlags flags) { itemFlags = flags; }

    inline QString text(int column) const
        { return data(column, QAbstractItemModel::DisplayRole).toString(); }
    inline void setText(int column, const QString &text)
        { setData(column, QAbstractItemModel::DisplayRole, text); }

    inline QIcon icon(int column) const
        { return qvariant_cast<QIcon>(data(column, QAbstractItemModel::DecorationRole)); }
    inline void setIcon(int column, const QIcon &icon)
        { setData(column, QAbstractItemModel::DecorationRole, icon); }

    inline QString statusTip(int column) const
        { return data(column, QAbstractItemModel::StatusTipRole).toString(); }
    inline void setStatusTip(int column, const QString &statusTip)
        { setData(column, QAbstractItemModel::StatusTipRole, statusTip); }

    inline QString toolTip(int column) const
        { return data(column, QAbstractItemModel::ToolTipRole).toString(); }
    inline void setToolTip(int column, const QString &toolTip)
        { setData(column, QAbstractItemModel::ToolTipRole, toolTip); }

    inline QString whatsThis(int column) const
        { return data(column, QAbstractItemModel::WhatsThisRole).toString(); }
    inline void setWhatsThis(int column, const QString &whatsThis)
        { setData(column, QAbstractItemModel::WhatsThisRole, whatsThis); }

    inline QFont font(int column) const
        { return qvariant_cast<QFont>(data(column, QAbstractItemModel::FontRole)); }
    inline void setFont(int column, const QFont &font)
        { setData(column, QAbstractItemModel::FontRole, font); }

    inline int textAlignment(int column) const
        { return data(column, QAbstractItemModel::TextAlignmentRole).toInt(); }
    inline void setTextAlignment(int column, int alignment)
        { setData(column, QAbstractItemModel::TextAlignmentRole, alignment); }

    inline QColor backgroundColor(int column) const
        { return qvariant_cast<QColor>(data(column, QAbstractItemModel::BackgroundColorRole)); }
    inline void setBackgroundColor(int column, const QColor &color)
        { setData(column, QAbstractItemModel::BackgroundColorRole, color); }

    inline QColor textColor(int column) const
        { return qvariant_cast<QColor>(data(column, QAbstractItemModel::TextColorRole)); }
    inline void setTextColor(int column, const QColor &color)
        { setData(column, QAbstractItemModel::TextColorRole, color); }

    inline Qt::CheckState checkState(int column) const
        { return static_cast<Qt::CheckState>(data(column, QAbstractItemModel::CheckStateRole).toInt()); }
    inline void setCheckState(int column, Qt::CheckState state)
        { setData(column, QAbstractItemModel::CheckStateRole, state); }

    virtual QVariant data(int column, int role) const;
    virtual void setData(int column, int role, const QVariant &value);

    virtual bool operator<(const QTreeWidgetItem &other) const;

#ifndef QT_NO_DATASTREAM
    virtual void read(QDataStream &in);
    virtual void write(QDataStream &out) const;
#endif
    void operator=(const QTreeWidgetItem &other);

    inline QTreeWidgetItem *parent() const { return par; }
    inline QTreeWidgetItem *child(int index) const
        { if (index < 0 || index >= children.size()) return 0; return children.at(index); }
    inline int childCount() const { return children.count(); }
    inline int columnCount() const { return values.count(); }
    inline int indexOfChild(QTreeWidgetItem *child) const { return children.indexOf(child); }

    void addChild(QTreeWidgetItem *child);
    void insertChild(int index, QTreeWidgetItem *child);
    QTreeWidgetItem *takeChild(int index);

private:
    QTreeWidgetItem();
    void sortChildren(int column, Qt::SortOrder order, bool climb);
    QVariant childrenCheckState(int column) const;

    // One item has a vector of column entries. Each column has a vector of (role, value) pairs.
    QVector< QVector<QWidgetItemData> > values;
    QTreeWidget *view;
    QTreeModel *model;
    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;
    QAbstractItemModel::ItemFlags itemFlags;
};

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QTreeWidgetItem &item);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QTreeWidgetItem &item);
#endif

class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
    Q_OBJECT
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)
    Q_PROPERTY(int topLevelItemCount READ topLevelItemCount)

public:
    explicit QTreeWidget(QWidget *parent = 0);
    ~QTreeWidget();

    int columnCount() const;
    void setColumnCount(int columns);

    QTreeWidgetItem *topLevelItem(int index) const;
    int topLevelItemCount() const;
    void insertTopLevelItem(int index, QTreeWidgetItem *item);
    void addTopLevelItem(QTreeWidgetItem *item);
    QTreeWidgetItem *takeTopLevelItem(int index);
    int indexOfTopLevelItem(QTreeWidgetItem *item);

    QTreeWidgetItem *headerItem() const;
    void setHeaderItem(QTreeWidgetItem *item);
    void setHeaderLabels(const QStringList &labels);

    QTreeWidgetItem *currentItem() const;
    void setCurrentItem(QTreeWidgetItem *item);

    QTreeWidgetItem *itemAt(const QPoint &p) const;
    inline QTreeWidgetItem *itemAt(int x, int y) const { return itemAt(QPoint(x, y)); }
    QRect visualItemRect(const QTreeWidgetItem *item) const;

    void sortItems(int column, Qt::SortOrder order);
    void setSortingEnabled(bool enable);
    bool isSortingEnabled() const;

    void openPersistentEditor(QTreeWidgetItem *item, int column = 0);
    void closePersistentEditor(QTreeWidgetItem *item, int column = 0);

    bool isItemSelected(const QTreeWidgetItem *item) const;
    void setItemSelected(const QTreeWidgetItem *item, bool select);
    QList<QTreeWidgetItem*> selectedItems() const;
    QList<QTreeWidgetItem*> findItems(const QRegExp &rx) const;

    bool isItemHidden(const QTreeWidgetItem *item) const;
    void setItemHidden(const QTreeWidgetItem *item, bool hide);

    bool isItemExpanded(const QTreeWidgetItem *item) const;
    void setItemExpanded(const QTreeWidgetItem *item, bool expand);

public slots:
    void scrollToItem(const QTreeWidgetItem *item);
    void expandItem(const QTreeWidgetItem *item);
    void collapseItem(const QTreeWidgetItem *item);
    void clear();

signals:
    void itemPressed(QTreeWidgetItem *item, int column);
    void itemClicked(QTreeWidgetItem *item, int column);
    void itemDoubleClicked(QTreeWidgetItem *item, int column);
    void itemActivated(QTreeWidgetItem *item, int column);
    void itemEntered(QTreeWidgetItem *item, int column);
    void itemChanged(QTreeWidgetItem *item, int column);
    void itemExpanded(QTreeWidgetItem *item);
    void itemCollapsed(QTreeWidgetItem *item);
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void itemSelectionChanged();

private:
    void setModel(QAbstractItemModel *model);

    Q_DECLARE_PRIVATE(QTreeWidget)
    Q_DISABLE_COPY(QTreeWidget)

    Q_PRIVATE_SLOT(d, void emitItemPressed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemClicked(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemDoubleClicked(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemActivated(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemEntered(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemChanged(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemExpanded(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemCollapsed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current))

};

#endif // QTREEWIDGET_H
