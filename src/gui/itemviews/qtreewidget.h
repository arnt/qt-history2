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
#include <QtCore/qlist.h>

class QTreeWidget;
class QTreeModel;

class Q_GUI_EXPORT QTreeWidgetItem
{
    friend class QTreeModel;
public:
    QTreeWidgetItem(QTreeWidget *view);
    QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after);
    QTreeWidgetItem(QTreeWidgetItem *parent);
    QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after);
    virtual ~QTreeWidgetItem();

    inline QTreeWidget *treeWidget() const { return view; }

    inline QAbstractItemModel::ItemFlags flags() const { return itemFlags; }
    inline void setFlags(QAbstractItemModel::ItemFlags flags) { itemFlags = flags; }

    inline QString text(int column) const
        { return data(column, QAbstractItemModel::DisplayRole).toString(); }
    inline void setText(int column, const QString &text)
        { setData(column, QAbstractItemModel::DisplayRole, text); }

    inline QIcon icon(int column) const
        { return data(column, QAbstractItemModel::DecorationRole).toIcon(); }
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
        { return data(column, QAbstractItemModel::FontRole).toFont(); }
    inline void setFont(int column, const QFont &font)
        { setData(column, QAbstractItemModel::FontRole, font); }

    inline int textAlignment(int column) const
        { return data(column, QAbstractItemModel::TextAlignmentRole).toInt(); }
    inline void setTextAlignment(int column, int alignment)
        { setData(column, QAbstractItemModel::TextAlignmentRole, alignment); }

    inline QColor backgroundColor(int column) const
        { return data(column, QAbstractItemModel::BackgroundColorRole).toColor(); }
    inline void setBackgroundColor(int column, const QColor &color)
        { setData(column, QAbstractItemModel::BackgroundColorRole, color); }

    inline QColor textColor(int column) const
        { return data(column, QAbstractItemModel::TextColorRole).toColor(); }
    inline void setTextColor(int column, const QColor &color)
        { setData(column, QAbstractItemModel::TextColorRole, color); }

    inline int checkedState(int column) const
        { return data(column, QAbstractItemModel::CheckStateRole).toInt(); }
    inline void setCheckedState(int column, bool state)
        { setData(column, QAbstractItemModel::CheckStateRole, state); }

    virtual QVariant data(int column, int role) const;
    virtual void setData(int column, int role, const QVariant &value);
    virtual void clear();

    virtual bool operator<(const QTreeWidgetItem &other) const;
    virtual QDataStream &operator<<(QDataStream &stream) const;
    virtual QDataStream &operator>>(QDataStream &stream);

    inline QTreeWidgetItem *parent() const { return par; }
    inline QTreeWidgetItem *child(int index) const
        { if (index < 0 || index >= children.size()) return 0;
          return children.at(index); }
    inline int childCount() const { return children.count(); }
    inline int columnCount() const { return values.count(); }
    inline int indexOfChild(QTreeWidgetItem *child) const { return children.indexOf(child); }

    void appendChild(QTreeWidgetItem *child);
    void insertChild(int index, QTreeWidgetItem *child);
    QTreeWidgetItem *takeChild(int index);

private:
    QTreeWidgetItem();
    void sortChildren(int column, Qt::SortOrder order, bool climb);

    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    // One item has a vector of column entries. Each column has a vector of (role, value) pairs.
    QVector< QVector<Data> > values;
    QTreeWidget *view;
    QTreeModel *model;
    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;
    QAbstractItemModel::ItemFlags itemFlags;
};

class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
    Q_OBJECT
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)
    Q_PROPERTY(int topLevelItemCount READ topLevelItemCount)

public:
    QTreeWidget(QWidget *parent = 0);
    ~QTreeWidget();

    int columnCount() const;
    void setColumnCount(int columns);

    QTreeWidgetItem *topLevelItem(int index) const;
    int topLevelItemCount() const;
    void insertTopLevelItem(int index, QTreeWidgetItem *item);
    void appendTopLevelItem(QTreeWidgetItem *item);
    QTreeWidgetItem *takeTopLevelItem(int index);
    int indexOfTopLevelItem(QTreeWidgetItem *item);

    QTreeWidgetItem *headerItem() const;
    void setHeaderItem(QTreeWidgetItem *item);
    void setHeaderLabels(const QStringList &labels);

    QTreeWidgetItem *currentItem() const;
    void setCurrentItem(QTreeWidgetItem *item);

    void sortItems(int column, Qt::SortOrder order);
    void setSortingEnabled(bool enable);
    bool isSortingEnabled() const;

    void openPersistentEditor(QTreeWidgetItem *item, int column = 0);
    void closePersistentEditor(QTreeWidgetItem *item, int column = 0);

    bool isSelected(const QTreeWidgetItem *item) const;
    void setSelected(const QTreeWidgetItem *item, bool select);
    QList<QTreeWidgetItem*> selectedItems() const;
    QList<QTreeWidgetItem*> findItems(const QRegExp &rx) const;

    bool isItemHidden(const QTreeWidgetItem *item) const;
    void setItemHidden(const QTreeWidgetItem *item, bool hide);

    bool isItemVisible(const QTreeWidgetItem *item) const;
    bool isItemOpen(const QTreeWidgetItem *item) const;

public slots:
    void ensureVisible(const QTreeWidgetItem *item);
    void openItem(const QTreeWidgetItem *item);
    void closeItem(const QTreeWidgetItem *item);
    void sortItems(int column);
    void clear();

signals:
    void pressed(QTreeWidgetItem *item, int column, Qt::MouseButton button,
                 Qt::KeyboardModifiers modifiers);
    void clicked(QTreeWidgetItem *item, int column, Qt::MouseButton button,
                 Qt::KeyboardModifiers modifiers);
    void doubleClicked(QTreeWidgetItem *item, int column, Qt::MouseButton button,
                       Qt::KeyboardModifiers modifiers);
    void keyPressed(QTreeWidgetItem *item, int column, Qt::Key key, Qt::KeyboardModifiers modifiers);
    void returnPressed(QTreeWidgetItem *item, int column);
    void expanded(QTreeWidgetItem *item);
    void collapsed(QTreeWidgetItem *item);
    void currentChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void selectionChanged();
    void itemEntered(QTreeWidgetItem *item, int column, Qt::MouseButton button,
                     Qt::KeyboardModifiers modifiers);
    void aboutToShowContextMenu(QMenu *menu, QTreeWidgetItem *item, int column);
    void itemChanged(QTreeWidgetItem *item, int column);

protected:
    void setModel(QAbstractItemModel *model);

private:
    Q_DECLARE_PRIVATE(QTreeWidget)
    Q_DISABLE_COPY(QTreeWidget)
    Q_PRIVATE_SLOT(d, void emitPressed(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitClicked(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitDoubleClicked(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitReturnPressed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitExpanded(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitCollapsed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current))
    Q_PRIVATE_SLOT(d, void emitItemEntered(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitAboutToShowContextMenu(QMenu *meny, const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
};

#endif
