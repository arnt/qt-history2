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

class QTreeModel;
class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeWidget)

    friend class QTreeWidgetItem;
public:
    QTreeWidget(QWidget *parent = 0);

    int columnCount() const;
    void setColumnCount(int columns);

    QTreeWidgetItem *headerItem();
    void setHeaderItem(QTreeWidgetItem *item);

protected:
    void appendItem(QTreeWidgetItem *item);
    void removeItem(QTreeWidgetItem *item);
    bool isSelected(QTreeWidgetItem *item) const;
    void setModel(QAbstractItemModel *model);
};

class Q_GUI_EXPORT QTreeWidgetItem
{
    friend class QTreeModel;
public:
    QTreeWidgetItem(QTreeWidget *view);
    QTreeWidgetItem(QTreeWidgetItem *parent);
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

    inline QTreeWidgetItem *parent() { return par; }
    inline QTreeWidgetItem *child(int index) { return children.at(index); }
    inline int childCount() const { return children.count(); }
    inline bool isSelected() { return view->isSelected(this); }
    inline int columnCount() const { return values.count(); }

protected:
    QTreeWidgetItem();

private:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QTreeWidget *view;
    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;
    QVector< QVector<Data> > values;
    QAbstractItemModel::ItemFlags itemFlags;
    // One item has a vector of column entries. Each column has a vector of (role, value) pairs.
};

#endif
