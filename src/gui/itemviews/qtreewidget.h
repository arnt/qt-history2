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

#include <qtreeview.h>
#include <qlist.h>

class QTreeWidget;

class Q_GUI_EXPORT QTreeWidgetItem
{
    friend class QTreeModel;
public:
    QTreeWidgetItem(QTreeWidget *view);
    QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after);
    QTreeWidgetItem(QTreeWidgetItem *parent);
    QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after);
    virtual ~QTreeWidgetItem();

    inline QAbstractItemModel::ItemFlags flags() const { return itemFlags; }
    inline void setFlags(QAbstractItemModel::ItemFlags flags) { itemFlags = flags; }

    inline QString text(int column) const
        { return data(column, QAbstractItemModel::DisplayRole).toString(); }
    inline void setText(int column, const QString &text)
        { setData(column, QAbstractItemModel::DisplayRole, text); }

    inline QIconSet icon(int column) const
        { return data(column, QAbstractItemModel::DecorationRole).toIcon(); }
    inline void setIcon(int column, const QIconSet &icon)
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
    virtual bool operator<(const QTreeWidgetItem &other) const;

    inline QTreeWidgetItem *parent() const { return par; }
    inline QTreeWidgetItem *child(int index) const { return children.at(index); }
    inline int childCount() const { return children.count(); }
    inline int columnCount() const { return values.count(); }
    inline int indexOfChild(QTreeWidgetItem *child) const { return children.indexOf(child); }
 
    void appendChild(QTreeWidgetItem *child);
    void insertChild(int index, QTreeWidgetItem *child);
    QTreeWidgetItem *takeChild(int index);

protected:
    QTreeWidgetItem();

    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    // One item has a vector of column entries. Each column has a vector of (role, value) pairs.
    QVector< QVector<Data> > values;

private:
    QTreeWidget *view;
    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;
    QAbstractItemModel::ItemFlags itemFlags;
};

class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeWidget)

public:
    QTreeWidget(QWidget *parent = 0);

    int columnCount() const;
    void setColumnCount(int columns);

    QTreeWidgetItem *topLevelItem(int index) const;
    int topLevelItemCount() const;
    void insertTopLevelItem(int index, QTreeWidgetItem *item);
    void appendTopLevelItem(QTreeWidgetItem *item);

    QTreeWidgetItem *headerItem();
    void setHeaderItem(QTreeWidgetItem *item);

    QTreeWidgetItem *currentItem() const;
    void setCurrentItem(QTreeWidgetItem *item);

    void openPersistentEditor(QTreeWidgetItem *item, int column = 0);
    void closePersistentEditor(QTreeWidgetItem *item, int column = 0);
    
    bool isSelected(const QTreeWidgetItem *item) const;
    void setSelected(const QTreeWidgetItem *item, bool select);
    QList<QTreeWidgetItem*> selectedItems() const;

    bool isVisible(const QTreeWidgetItem *item) const;
    void ensureItemVisible(const QTreeWidgetItem *item);

public slots:
    void clear();

signals:
    void pressed(QTreeWidgetItem *item, int column, int button);
    void clicked(QTreeWidgetItem *item, int column, int button);
    void doubleClicked(QTreeWidgetItem *item, int column, int button);
    void keyPressed(QTreeWidgetItem *item, int column, Qt::Key, Qt::ButtonState);
    void expanded(QTreeWidgetItem *item);
    void collapsed(QTreeWidgetItem *item);
    void currentChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void selectionChanged();

protected:
    void appendItem(QTreeWidgetItem *item);
    void removeItem(QTreeWidgetItem *item);
    void setModel(QAbstractItemModel *model);

private:
    Q_PRIVATE_SLOT(d, void emitPressed(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitClicked(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitDoubleClicked(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state));
    Q_PRIVATE_SLOT(d, void emitExpanded(const QModelIndex &index));
    Q_PRIVATE_SLOT(d, void emitCollapsed(const QModelIndex &index));
    Q_PRIVATE_SLOT(d, void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current));
};

#endif
