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
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

class QListWidget;
class QListModel;
class QWidgetItemData;

class Q_GUI_EXPORT QListWidgetItem
{
    friend class QListModel;
public:
    explicit QListWidgetItem(QListWidget *view = 0);
    explicit QListWidgetItem(const QString &text, QListWidget *view = 0);
    virtual ~QListWidgetItem();

    virtual QListWidgetItem *clone() const;

    inline QListWidget *listWidget() const { return view; }

    inline QAbstractItemModel::ItemFlags flags() const { return itemFlags; }
    inline void setFlags(QAbstractItemModel::ItemFlags flags) { itemFlags = flags; }

    inline QString text() const
        { return data(QAbstractItemModel::DisplayRole).toString(); }
    inline void setText(const QString &text)
        { setData(QAbstractItemModel::DisplayRole, text); }

    inline QIcon icon() const
        { return qvariant_cast<QIcon>(data(QAbstractItemModel::DecorationRole)); }
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
        { return qvariant_cast<QFont>(data(QAbstractItemModel::FontRole)); }
    inline void setFont(const QFont &font)
        { setData(QAbstractItemModel::FontRole, font); }

    inline int textAlignment() const
        { return data(QAbstractItemModel::TextAlignmentRole).toInt(); }
    inline void setTextAlignment(int alignment)
        { setData(QAbstractItemModel::TextAlignmentRole, alignment); }

    inline QColor backgroundColor() const
        { return qvariant_cast<QColor>(data(QAbstractItemModel::BackgroundColorRole)); }
    virtual void setBackgroundColor(const QColor &color)
        { setData(QAbstractItemModel::BackgroundColorRole, color); }

    inline QColor textColor() const
        { return qvariant_cast<QColor>(data(QAbstractItemModel::TextColorRole)); }
    inline void setTextColor(const QColor &color)
        { setData(QAbstractItemModel::TextColorRole, color); }

    inline Qt::CheckState checkState() const
        { return static_cast<Qt::CheckState>(data(QAbstractItemModel::CheckStateRole).toInt()); }
    inline void setCheckState(Qt::CheckState state)
        { setData(QAbstractItemModel::CheckStateRole, state); }

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);

    virtual bool operator<(const QListWidgetItem &other) const;

#ifndef QT_NO_DATASTREAM
    virtual void read(QDataStream &in);
    virtual void write(QDataStream &out) const;
#endif

private:
    QVector<QWidgetItemData> values;
    QListWidget *view;
    QListModel *model;
    QAbstractItemModel::ItemFlags itemFlags;
};

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QListWidgetItem &item);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QListWidgetItem &item);
#endif

class QListWidgetPrivate;

class Q_GUI_EXPORT QListWidget : public QListView
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow)

    friend class QListWidgetItem;
public:
    explicit QListWidget(QWidget *parent = 0);
    ~QListWidget();

    QListWidgetItem *item(int row) const;
    int row(const QListWidgetItem *item) const;
    void insertItem(int row, QListWidgetItem *item);
    void insertItem(int row, const QString &label);
    void insertItems(int row, const QStringList &labels);
    inline void addItem(const QString &label) { insertItem(count(), label); }
    inline void addItem(QListWidgetItem *item) { insertItem(count(), item); }
    inline void addItems(const QStringList &labels) { insertItems(count(), labels); }
    QListWidgetItem *takeItem(int row);
    int count() const;

    QListWidgetItem *currentItem() const;
    void setCurrentItem(QListWidgetItem *item);

    int currentRow() const;
    void setCurrentRow(int row);

    QListWidgetItem *itemAt(const QPoint &p) const;
    inline QListWidgetItem *itemAt(int x, int y) const { return itemAt(QPoint(x, y)); }
    QRect visualItemRect(const QListWidgetItem *item) const;

    void sortItems(Qt::SortOrder order = Qt::AscendingOrder);

    void openPersistentEditor(QListWidgetItem *item);
    void closePersistentEditor(QListWidgetItem *item);

    bool isItemSelected(const QListWidgetItem *item) const;
    void setItemSelected(const QListWidgetItem *item, bool select);
    QList<QListWidgetItem*> selectedItems() const;
    QList<QListWidgetItem*> findItems(const QRegExp &rx) const;

    bool isItemHidden(const QListWidgetItem *item) const;
    void setItemHidden(const QListWidgetItem *item, bool hide);

public slots:
    void scrollToItem(const QListWidgetItem *item);
    void clear();

signals:
    void itemPressed(QListWidgetItem *item);
    void itemClicked(QListWidgetItem *item);
    void itemDoubleClicked(QListWidgetItem *item);
    void itemActivated(QListWidgetItem *item);
    void itemEntered(QListWidgetItem *item);
    void itemChanged(QListWidgetItem *item);

    void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void currentTextChanged(const QString &currentText);
    void currentRowChanged(int currentRow);

    void itemSelectionChanged();

private:
    void setModel(QAbstractItemModel *model);

    Q_DECLARE_PRIVATE(QListWidget)
    Q_DISABLE_COPY(QListWidget)

    Q_PRIVATE_SLOT(d, void emitItemPressed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemClicked(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemDoubleClicked(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemActivated(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemEntered(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemChanged(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current))
};

#endif // QLISTWIDGET_H
