#ifndef QITEMSELECTIONMODEL_H
#define QITEMSELECTIONMODEL_H

#ifndef QT_H
#include <qabstractitemmodel.h>
#include <qlist.h>
#endif

class QItemSelectionRange
{

public:
    inline QItemSelectionRange(const QItemSelectionRange &other)
        : p(other.p), t(other.t), l(other.l), b(other.b), r(other.r) {}
    inline QItemSelectionRange(int top = 1, int left = 1, int bottom = -1, int right = -1)
        : t(top), l(left), b(bottom), r(right) {}
    inline QItemSelectionRange(const QModelIndex &parent,
                               int top = 1, int left = 1, int bottom = -1, int right = -1)
        : p(parent), t(top), l(left), b(bottom), r(right) {}

    inline int top() const { return t; }
    inline int left() const { return l; }
    inline int bottom() const { return b; }
    inline int right() const { return r; }
    inline int width() const { return r - l + 1; }
    inline int height() const { return b - t + 1; }
    inline QModelIndex parent() const { return p; }

    inline bool contains(const QModelIndex &item, const QAbstractItemModel *model) const
    {
        return (p == model->parent(item)
                && t <= item.row() && l <= item.column()
                && b >= item.row() && r >= item.column());
    }

    inline bool intersects(const QItemSelectionRange &other) const
    {
        return (p == other.p)
                   && ((t <= other.t && b >= other.t) || (t >= other.t && t <= other.b))
                   && ((l <= other.l && r >= other.l) || (l >= other.l && l <= other.r));
    }

    inline QItemSelectionRange intersect(const QItemSelectionRange &other) const
        { return QItemSelectionRange(p, qMax(t, other.t), qMax(l, other.l), qMin(b, other.b), qMin(r, other.r)); }

    inline bool operator==(const QItemSelectionRange &other) const
    {
        return (p == other.p
                && t == other.t && l == other.l
                && b == other.b && r == other.r);
    }
    inline bool operator!=(const QItemSelectionRange &other) const { return !operator==(other); }
    inline bool isValid() const { return (t <= b && l <= r); }

    QModelIndexList items(const QAbstractItemModel *model) const;

private:
    QModelIndex p;
    int t, l, b, r;
};

class QItemSelectionModel;

class QItemSelection : public QList<QItemSelectionRange>
{
public:
    QItemSelection() {}
    QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QAbstractItemModel *model);
    void select(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QAbstractItemModel *model);
    bool contains(const QModelIndex &item, const QAbstractItemModel *model) const;
    QModelIndexList items(QAbstractItemModel *model) const;
    void merge(const QItemSelection &other, int selectionCommand);
};

class QItemSelectionModelPrivate;

class Q_GUI_EXPORT QItemSelectionModel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QItemSelectionModel);

public:

    enum SelectionCommand {
        NoUpdate       = 0x0000,
        Clear          = 0x0001,
        Select         = 0x0002,
        Deselect       = 0x0004,
        Toggle         = 0x0008,
        Current        = 0x0010,
        Rows           = 0x0020,
        Columns        = 0x0040,
        SelectCurrent  = Select | Current,
        ToggleCurrent  = Toggle | Current,
        ClearAndSelect = Clear | Select
    };

    QItemSelectionModel(QAbstractItemModel *model, QObject *parent = 0);
    virtual ~QItemSelectionModel();

    virtual void select(const QModelIndex &item, int selectionCommand);
    virtual void select(const QItemSelection &selection, int selectionCommand);
    virtual void clear();

    QModelIndex currentItem() const;
    void setCurrentItem(const QModelIndex &item, int selectionCommand);

    bool isSelected(const QModelIndex &item) const;
    bool isRowSelected(int row, const QModelIndex &parent) const;
    bool isColumnSelected(int column, const QModelIndex &parent) const;

    QAbstractItemModel *model() const;
    QModelIndexList selectedItems() const;

signals:
    void selectionChanged(const QItemSelection &deselected, const QItemSelection &selected);
    void currentChanged(const QModelIndex &oldItem, const QModelIndex &newItem);

protected:
    QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model, QObject *parent = 0);
    void emitSelectionChanged(const QItemSelection &oldSelection,
                              const QItemSelection &newSelection);
    void mergeCurrentSelection();
};

#endif
