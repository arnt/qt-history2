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

#include <QtGui/qtableview.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

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
    explicit QTableWidgetItem(const QString &text);
    virtual ~QTableWidgetItem();

    virtual QTableWidgetItem *clone() const;

    inline QTableWidget *tableWidget() const { return view; }

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
    inline void setBackgroundColor(const QColor &color)
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

    virtual bool operator<(const QTableWidgetItem &other) const;

#ifndef QT_NO_DATASTREAM
    virtual void read(QDataStream &in);
    virtual void write(QDataStream &out) const;
#endif

private:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };
#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator>>(QDataStream &in, QTableWidgetItem::Data &data);
    friend QDataStream &operator<<(QDataStream &out, const QTableWidgetItem::Data &data);
#endif

    QVector<Data> values;
    QTableWidget *view;
    QTableModel *model;
    QAbstractItemModel::ItemFlags itemFlags;
};

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QTableWidgetItem &item);
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QTableWidgetItem &item);
#endif

class QTableWidgetPrivate;

class Q_GUI_EXPORT QTableWidget : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount WRITE setRowCount)
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)

    friend class QTableModel;
public:
    explicit QTableWidget(QWidget *parent = 0);
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

    bool isItemSelected(const QTableWidgetItem *item) const;
    void setItemSelected(const QTableWidgetItem *item, bool select);
    void setRangeSelected(const QTableWidgetSelectionRange &range, bool select);

    QList<QTableWidgetSelectionRange> selectedRanges() const;
    QList<QTableWidgetItem*> selectedItems();
    QList<QTableWidgetItem*> findItems(const QRegExp &rx) const;

    int visualRow(int logicalRow) const;
    int visualColumn(int logicalColumn) const;

    QTableWidgetItem *itemAt(const QPoint &p) const;
    inline QTableWidgetItem *itemAt(int x, int y) const  { return itemAt(QPoint(x, y)); }
    QRect visualItemRect(const QTableWidgetItem *item) const;

    const QTableWidgetItem *itemPrototype() const;
    void setItemPrototype(const QTableWidgetItem *item);

public slots:
    void scrollToItem(const QTableWidgetItem *item);
    void insertRow(int row);
    void insertColumn(int column);
    void removeRow(int row);
    void removeColumn(int column);
    void clear();

signals:
    void itemPressed(QTableWidgetItem *item);
    void itemClicked(QTableWidgetItem *item);
    void itemDoubleClicked(QTableWidgetItem *item);

    void itemActivated(QTableWidgetItem *item);
    void itemEntered(QTableWidgetItem *item);
    void itemChanged(QTableWidgetItem *item);
    
    void currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void itemSelectionChanged();

private:
    void setModel(QAbstractItemModel *model);

    Q_DECLARE_PRIVATE(QTableWidget)
    Q_DISABLE_COPY(QTableWidget)

    Q_PRIVATE_SLOT(d, void emitItemPressed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemClicked(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemDoubleClicked(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemActivated(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemEntered(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitItemChanged(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current))
};

#endif // QTABLEWIDGET_H
