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

#ifndef QITEMSELECTIONMODEL_H
#define QITEMSELECTIONMODEL_H

#include <qabstractitemmodel.h>
#include <qlist.h>

class Q_GUI_EXPORT QItemSelectionRange
{

public:
    inline QItemSelectionRange() :
        t(-1), l(-1), b(-1), r(-1) {}
    inline QItemSelectionRange(const QItemSelectionRange &other)
        : p(other.p), t(other.t), l(other.l), b(other.b), r(other.r) {}
    inline QItemSelectionRange(const QModelIndex &parent,
                               int top, int left, int bottom, int right)
        : p(parent), t(top), l(left), b(bottom), r(right) {}
    inline QItemSelectionRange(const QModelIndex &parent, const QModelIndex &index)
        : p(parent), t(index.row()), l(index.column()),
          b(index.row()), r(index.column()) {}

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
    inline bool isValid() const { return (t <= b && l <= r && t > -1 && l > -1); }

    QModelIndexList items(const QAbstractItemModel *model) const;

private:
    QModelIndex p;
    int t, l, b, r;
};

class QItemSelection;
class QItemSelectionModelPrivate;

class Q_GUI_EXPORT QItemSelectionModel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QItemSelectionModel)
    Q_FLAGS(SelectionFlag)

public:

    enum SelectionFlag {
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

    Q_DECLARE_FLAGS(SelectionFlags, SelectionFlag);

    QItemSelectionModel(QAbstractItemModel *model, QObject *parent = 0);
    virtual ~QItemSelectionModel();

    QModelIndex currentIndex() const;

    bool isSelected(const QModelIndex &index) const;
    bool isRowSelected(int row, const QModelIndex &parent) const;
    bool isColumnSelected(int column, const QModelIndex &parent) const;

    QAbstractItemModel *model() const;
    QModelIndexList selectedIndexes() const;

public slots:
    void setCurrentIndex(const QModelIndex &index, SelectionFlags command);
    virtual void select(const QModelIndex &index, SelectionFlags command);
    virtual void select(const QItemSelection &selection, SelectionFlags command);
    virtual void clear();
    virtual void reset();

signals:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentChanged(const QModelIndex &newItem, const QModelIndex &oldItem);

protected:
    QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model,
                        QObject *parent = 0);
    void emitSelectionChanged(const QItemSelection &newSelection, const QItemSelection &oldSelection);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QItemSelectionModel::SelectionFlags);

class Q_GUI_EXPORT QItemSelection : public QList<QItemSelectionRange>
{
public:
    QItemSelection() {}
    QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                   const QAbstractItemModel *model);
    void select(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                const QAbstractItemModel *model);
    bool contains(const QModelIndex &item, const QAbstractItemModel *model) const;
    QModelIndexList items(QAbstractItemModel *model) const;
    void merge(const QItemSelection &other, QItemSelectionModel::SelectionFlags command);
    static void split(const QItemSelectionRange &range, const QItemSelectionRange &other,
                      QItemSelection *result);
};

#endif
