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

#ifndef QLISTWIDGET_H
#define QLISTWIDGET_H

#include <QtGui/qlistview.h>
#include <QtCore/qlist.h>

class QListWidget;
class QListModel;

class Q_GUI_EXPORT QListWidgetItem
{
    friend class QListModel;
public:
    QListWidgetItem(QListWidget *view = 0);
    QListWidgetItem(const QString &text, QListWidget *view = 0);
    virtual ~QListWidgetItem();

    inline QListWidget *listWidget() const { return view; }

    inline QAbstractItemModel::ItemFlags flags() const { return itemFlags; }
    inline void setFlags(QAbstractItemModel::ItemFlags flags) { itemFlags = flags; }

    inline QString text() const
        { return data(QAbstractItemModel::DisplayRole).toString(); }
    inline void setText(const QString &text)
        { setData(QAbstractItemModel::DisplayRole, text); }

    inline QIcon icon() const
        { return data(QAbstractItemModel::DecorationRole).toIcon(); }
    inline void setIcon(const QIcon &icon)
        { setData(QAbstractItemModel::DecorationRole, icon); }

    inline QString statusTip() const
        { return data(QAbstractItemModel::StatusTipRole).toString(); }
    inline void setStatusTip(const QString &statusTip)
        { setData(QAbstractItemModel::StatusTipRole, statusTip); }

    inline QString toolTip() const
        { return data(QAbstractItemModel::ToolTipRole).toString(); }
    inline void setToolTip(const QString &toolTip)
         { setData(QAbstractItemModel::ToolTipRole, toolTip); }

    inline QString whatsThis() const
        { return data(QAbstractItemModel::WhatsThisRole).toString(); }
    inline void setWhatsThis(const QString &whatsThis)
        { setData(QAbstractItemModel::WhatsThisRole, whatsThis); }

    inline QFont font() const
        { return data(QAbstractItemModel::FontRole).toFont(); }
    inline void setFont(const QFont &font)
        { setData(QAbstractItemModel::FontRole, font); }

    inline int textAlignment() const
        { return data(QAbstractItemModel::TextAlignmentRole).toInt(); }
    inline void setTextAlignment(int alignment)
        { setData(QAbstractItemModel::TextAlignmentRole, alignment); }

    inline QColor backgroundColor() const
        { return data(QAbstractItemModel::BackgroundColorRole).toColor(); }
    virtual void setBackgroundColor(const QColor &color)
        { setData(QAbstractItemModel::BackgroundColorRole, color); }

    inline QColor textColor() const
        { return data(QAbstractItemModel::TextColorRole).toColor(); }
    inline void setTextColor(const QColor &color)
        { setData(QAbstractItemModel::TextColorRole, color); }

    inline int checked() const
        { return data(QAbstractItemModel::CheckStateRole).toInt(); }
    inline void setChecked(const bool checked)
        { setData(QAbstractItemModel::CheckStateRole, checked); }

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);
    virtual void clear();

    virtual bool operator<(const QListWidgetItem &other) const;
    virtual QDataStream &operator<<(QDataStream &stream) const;
    virtual QDataStream &operator>>(QDataStream &stream);

private:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QVector<Data> values;
    QListWidget *view;
    QListModel *model;
    QAbstractItemModel::ItemFlags itemFlags;
};

class QListWidgetPrivate;

class Q_GUI_EXPORT QListWidget : public QListView
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)

    friend class QListWidgetItem;
public:
    QListWidget(QWidget *parent = 0);
    ~QListWidget();

    QListWidgetItem *item(int row) const;
    int row(const QListWidgetItem *item) const;
    void insertItem(int row, QListWidgetItem *item);
    inline void insertItem(int row, const QString &label) { insertItems(row, QStringList(label)); }
    void insertItems(int row, const QStringList &labels);
    inline void appendItem(QListWidgetItem *item) { insertItem(count(), item); }
    inline void appendItem(const QString &label) { insertItems(count(), QStringList(label)); }
    inline void appendItems(const QStringList &labels) { insertItems(count(), labels); }
    QListWidgetItem *takeItem(int row);
    int count() const;

    QListWidgetItem *currentItem() const;
    void setCurrentItem(QListWidgetItem *item);

    void sortItems(Qt::SortOrder order = Qt::AscendingOrder);

    void openPersistentEditor(QListWidgetItem *item);
    void closePersistentEditor(QListWidgetItem *item);

    bool isSelected(const QListWidgetItem *item) const;
    void setSelected(const QListWidgetItem *item, bool select);
    QList<QListWidgetItem*> selectedItems() const;
    QList<QListWidgetItem*> findItems(const QRegExp &rx) const;

    bool isItemHidden(const QListWidgetItem *item) const;
    void setItemHidden(const QListWidgetItem *item, bool hide);

    bool isItemVisible(const QListWidgetItem *item) const;

#ifdef Q_NO_USING_KEYWORD
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
        { QListView::selectionChanged(selected, deselected); }
    void currentChanged(const QModelIndex &current, const QModelIndex &previous)
        { QListView::currentChanged(current, previous); }
    void ensureVisible(const QModelIndex &index)
        { QListView::ensureVisible(index); }
#else
    using QListView::selectionChanged;
    using QListView::currentChanged;
    using QListView::ensureVisible;
#endif

public slots:
    void ensureVisible(const QListWidgetItem *item);
    void clear();

signals:
    void pressed(QListWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void clicked(QListWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void doubleClicked(QListWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void keyPressed(QListWidgetItem *item, Qt::Key key, Qt::KeyboardModifiers modifiers);
    void returnPressed(QListWidgetItem *item);
    void currentChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void currentTextChanged(const QString &currentText);
    void selectionChanged();
    void itemEntered(QListWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void aboutToShowContextMenu(QMenu *menu, QListWidgetItem *item);
    void itemChanged(QListWidgetItem *item);

protected:
    void setModel(QAbstractItemModel *model);
    void setup();

private:
    Q_DECLARE_PRIVATE(QListWidget)
    Q_DISABLE_COPY(QListWidget)
    Q_PRIVATE_SLOT(d, void emitPressed(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitClicked(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitDoubleClicked(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitReturnPressed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current))
    Q_PRIVATE_SLOT(d, void emitItemEntered(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
};

#endif
