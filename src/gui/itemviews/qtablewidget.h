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

#ifndef QTABLEWIDGET_H
#define QTABLEWIDGET_H

#include <qtableview.h>
#include <qlist.h>

class Q_GUI_EXPORT QTableWidgetSelectionRange
{
public:
    QTableWidgetSelectionRange();
    QTableWidgetSelectionRange(int top, int left, int bottom, int right);
    QTableWidgetSelectionRange(const QTableWidgetSelectionRange &other);
    ~QTableWidgetSelectionRange();
    inline int topRow() const { return top; }
    inline int bottomRow() const { return bottom; }
    inline int leftColumn() const { return left; }
    inline int rightColumn() const { return right; }
private:
    int top, left, bottom, right;
};

class QTableWidget;
class QTableModel;

class Q_GUI_EXPORT QTableWidgetItem
{
    friend class QTableWidget;
    friend class QTableModel;
public:
    QTableWidgetItem();
    QTableWidgetItem(const QString &text);
    virtual ~QTableWidgetItem();

    inline QTableWidget *tableWidget() const { return view; }

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
    inline void setBackgroundColor(const QColor &color)
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
    virtual bool operator<(const QTableWidgetItem &other) const;
    virtual void clear();

private:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QVector<Data> values;
    QTableWidget *view;
    QTableModel *model;
    QAbstractItemModel::ItemFlags itemFlags;
};

class Q_GUI_EXPORT QTableWidgetItemCreatorBase
{
public:
    virtual ~QTableWidgetItemCreatorBase();
    virtual QTableWidgetItem *createItem() const = 0;
};

template <class T>
class QTableWidgetItemCreator : public QTableWidgetItemCreatorBase
{
    inline QTableWidgetItem *createItem() const { return new T; }
};

class QTableWidgetPrivate;

class Q_GUI_EXPORT QTableWidget : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount WRITE setRowCount)
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)

    friend class QTableModel;
public:
    QTableWidget(QWidget *parent = 0);
    QTableWidget(int rows, int columns, QWidget *parent = 0);
    ~QTableWidget();

    void setRowCount(int rows);
    int rowCount() const;

    void setColumnCount(int columns);
    int columnCount() const;

    int row(const QTableWidgetItem *item) const;
    int column(const QTableWidgetItem *item) const;

    QTableWidgetItem *item(int row, int column) const;
    void setItem(int row, int column, QTableWidgetItem *item);
    QTableWidgetItem *takeItem(int row, int column);
    void removeItem(QTableWidgetItem *item);

    QTableWidgetItem *verticalHeaderItem(int row) const;
    void setVerticalHeaderItem(int row, QTableWidgetItem *item);

    QTableWidgetItem *horizontalHeaderItem(int column) const;
    void setHorizontalHeaderItem(int column, QTableWidgetItem *item);
    void setVerticalHeaderLabels(const QStringList &labels);
    void setHorizontalHeaderLabels(const QStringList &labels);

    int currentRow() const;
    int currentColumn() const;
    QTableWidgetItem *currentItem() const;
    void setCurrentItem(QTableWidgetItem *item);

    void sortItems(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setSortingEnabled(bool enable);
    bool isSortingEnabled() const;

    void openPersistentEditor(QTableWidgetItem *item);
    void closePersistentEditor(QTableWidgetItem *item);

    bool isSelected(const QTableWidgetItem *item) const;
    void setSelected(const QTableWidgetItem *item, bool select);
    void setSelected(const QTableWidgetSelectionRange &range, bool select);

    QList<QTableWidgetSelectionRange> selectedRanges() const;
    QList<QTableWidgetItem*> selectedItems(bool fillEmptyCells = false);
    QList<QTableWidgetItem*> findItems(const QString &text,
                                       QAbstractItemModel::MatchFlags flags
                                       = QAbstractItemModel::MatchDefault) const;

    int visualRow(const QTableWidgetItem *item) const;
    int visualColumn(const QTableWidgetItem *item) const;
    QTableWidgetItem *visualItem(int visualRow, int visualColumn) const;

    bool isItemVisible(const QTableWidgetItem *item) const;

    QTableWidgetItemCreatorBase *itemCreator() const;
    void setItemCreator(QTableWidgetItemCreatorBase *factory);

#ifdef Q_NO_USING_KEYWORD
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
        { QTableView::selectionChanged(selected, deselected); }
    void currentChanged(const QModelIndex &current, const QModelIndex &previous)
        { QTableView::currentChanged(current, previous); }
    void ensureVisible(const QModelIndex &index)
        { QTableView::ensureVisible(index); }
#else
    using QTableView::selectionChanged;
    using QTableView::currentChanged;
    using QTableView::ensureVisible;
#endif

public slots:
    void ensureVisible(const QTableWidgetItem *item);
    void insertRow(int row);
    void insertColumn(int column);
    void removeRow(int row);
    void removeColumn(int column);
    void clear();

signals:
    void pressed(QTableWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void clicked(QTableWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void doubleClicked(QTableWidgetItem *item, Qt::MouseButton button,
                       Qt::KeyboardModifiers modifiers);
    void keyPressed(QTableWidgetItem *item, Qt::Key key, Qt::KeyboardModifiers modifiers);
    void returnPressed(QTableWidgetItem *item);
    void currentChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void selectionChanged();
    void itemEntered(QTableWidgetItem *item, Qt::MouseButton button,
                     Qt::KeyboardModifiers modifiers);
    void aboutToShowContextMenu(QMenu *menu, QTableWidgetItem *item);
    void itemChanged(QTableWidgetItem *item);

protected:
    void setModel(QAbstractItemModel *model);
    void setup();

private:
    Q_DECLARE_PRIVATE(QTableWidget)
    Q_DISABLE_COPY(QTableWidget)
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
