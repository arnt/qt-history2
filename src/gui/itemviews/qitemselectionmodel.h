#ifndef QITEMSELECTIONMODEL_H
#define QITEMSELECTIONMODEL_H

#include <qgenericitemmodel.h>
#include <qsharedpointer.h>
#include <qlist.h>

class QItemSelectionRange
{

public:
    inline QItemSelectionRange(const QItemSelectionRange &other)
	: p(other.p), t(other.top()), l(other.left()), b(other.bottom()), r(other.right()) {}
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

    inline bool contains(const QModelIndex &item, const QGenericItemModel *model) const
    {
	return ( parent() == model->parent(item) &&
		 top() <= item.row() && left() <= item.column() &&
		 bottom() >= item.row() && right() >= item.column() );
    }

    inline bool intersects(const QItemSelectionRange &other) const
    {
	bool v = ((top() <= other.top() && bottom() >= other.top()) ||
		   (top() >= other.top() && top() <= other.bottom()));
	bool h = ((left() <= other.left() && right() >= other.left()) ||
		   (left() >= other.left() && left() <= other.right()));
	bool p = (parent() == other.parent());
	return v && h && p;
    }

    inline QItemSelectionRange intersect(const QItemSelectionRange &other) const
    {
	int t = top() > other.top() ? top() : other.top();
	int l = left() > other.left() ? left() : other.left();
	int b = bottom() < other.bottom() ? bottom() : other.bottom();
	int r = right() < other.right() ? right() : other.right();
	QItemSelectionRange isr(p, t, l, b, r);
	return isr;
    }

    inline bool operator==(const QItemSelectionRange &other) const
    {
	return (top() == other.top() && left() == other.left() &&
		bottom() == other.bottom() && right() == other.right());
    }
    inline bool operator!=(const QItemSelectionRange &other) const { return !operator==(other); }
    inline bool isValid() const { return (top() <= bottom() && left() <= right()); }

    QModelIndexList items(const QGenericItemModel *model) const;

private:
    QModelIndex p;
    int t, l, b, r;
};

class QItemSelectionModel;

class QItemSelection : public QSharedObject
{
    friend class QItemSelectionModel;
public:
    QItemSelection() {}
    QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight,
		   const QGenericItemModel *model);
    void select(const QModelIndex &topLeft, const QModelIndex &bottomRight,
		const QGenericItemModel *model);
    bool contains(const QModelIndex &item, const QGenericItemModel *model) const;
    inline void append(QItemSelectionRange &range) { ranges.append(range); }

    bool operator==(const QItemSelection &other) const;
    inline bool operator!=(const QItemSelection &other) const { return !operator==(other); }

    QModelIndexList items(QGenericItemModel *model) const;

//private:
    QList<QItemSelectionRange> ranges;
};

typedef QExplicitSharedPointer<QItemSelection> QItemSelectionPointer;
class QItemSelectionModelPrivate;

class QItemSelectionModel : public QObject, public QSharedObject
{
    Q_OBJECT

public:
    enum SelectionMode {
	Single,
	Multi
    };

    enum SelectionUpdateMode {
	NoUpdate,
	Toggle,
	ClearAndSelect,
	Select,
	Remove
    };

    enum SelectionBehavior {
	SelectItems,
	SelectRows
    };

    QItemSelectionModel(QGenericItemModel *m);
    virtual ~QItemSelectionModel();

    virtual void select(const QModelIndex &item,
			SelectionUpdateMode mode,
			SelectionBehavior behavior);
    virtual void select(QItemSelection *selection,
			SelectionUpdateMode mode,
			SelectionBehavior behavior);
    virtual void clear();

    QModelIndex currentItem() const;
    void setCurrentItem(const QModelIndex &item,
			SelectionUpdateMode mode,
			SelectionBehavior behavior);

    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);

    bool isSelected(const QModelIndex &item) const;
    bool isRowSelected(int row, const QModelIndex &parent) const;
    bool isColumnSelected(int column, const QModelIndex &parent) const;

    QGenericItemModel *model() const;
    QModelIndexList selectedItems() const;

signals:
    void selectionChanged(const QItemSelectionPointer &deselected,
			  const QItemSelectionPointer &selected);
    void currentChanged(const QModelIndex &oldItem, const QModelIndex &newItem);

protected:
    void exchange(QItemSelectionPointer &oldSelection,
		  const QItemSelectionPointer &newSelection,
		  bool alterRanges = true);
    void toggle(const QItemSelectionPointer &selection, bool emitSelectionChanged = true);

private:
    QItemSelectionModelPrivate *d;
};

typedef QExplicitSharedPointer<QItemSelectionModel> QItemSelectionModelPointer;

#endif
