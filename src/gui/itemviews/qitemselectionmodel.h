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
    inline QItemSelectionRange() : m(0) {}
    inline QItemSelectionRange(const QItemSelectionRange &other)
        : m(other.m), tl(other.tl), br(other.br) {}
    inline QItemSelectionRange(int top, int left, int bottom, int right,
                               const QModelIndex &parent, const QAbstractItemModel *model)
        : m(model)
        {
            if (model) {
                tl = QPersistentModelIndex(model->index(top, left, parent),
                                           const_cast<QAbstractItemModel*>(model));
                br = QPersistentModelIndex(model->index(bottom, right, parent),
                                           const_cast<QAbstractItemModel*>(model));
            }
        }
    inline QItemSelectionRange(const QModelIndex &index, const QAbstractItemModel *model) : m(model)
        { tl = QPersistentModelIndex(index, const_cast<QAbstractItemModel*>(model)); br = tl; }

    inline QItemSelectionRange(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                               const QAbstractItemModel *model)
        : m(model)
        {
            tl = QPersistentModelIndex(topLeft, const_cast<QAbstractItemModel*>(model));
            br = QPersistentModelIndex(bottomRight, const_cast<QAbstractItemModel*>(model));
        }

    inline int top() const { return tl.row(); }
    inline int left() const { return tl.column(); }
    inline int bottom() const { return br.row(); }
    inline int right() const { return br.column(); }
    inline int width() const { return br.column() - tl.column() + 1; }
    inline int height() const { return br.row() - tl.row() + 1; }
    inline QModelIndex parent() const { return (m ? m->parent(tl) : QModelIndex::Null); }
    inline const QAbstractItemModel *model() const { return m; }
    inline bool contains(const QModelIndex &item, const QAbstractItemModel *model) const
    {
        return (model && model->parent(tl) == model->parent(item)
                && tl.row() <= item.row() && tl.column() <= item.column()
                && br.row() >= item.row() && br.column() >= item.column());
    }

    inline bool intersects(const QItemSelectionRange &other) const
    {
        return (parent() == other.parent()
                && ((top() <= other.top() && bottom() >= other.top())
                    || (top() >= other.top() && top() <= other.bottom()))
                && ((left() <= other.left() && right() >= other.left())
                    || (left() >= other.left() && left() <= other.right())));
    }

    inline QItemSelectionRange intersect(const QItemSelectionRange &other) const
    {
        return QItemSelectionRange(qMax(top(), other.top()), qMax(left(), other.left()),
                                   qMin(bottom(), other.bottom()), qMin(right(), other.right()),
                                   other.parent(), other.model());
    }

    inline bool operator==(const QItemSelectionRange &other) const
    {
        return (parent() == other.parent()
                && top() == other.top() && left() == other.left()
                && bottom() == other.bottom() && right() == other.right());
    }

    inline bool operator!=(const QItemSelectionRange &other) const { return !operator==(other); }
    inline bool isValid() const { return m && tl.isValid() && br.isValid()
                                      && top() <= bottom() && left() <= right()
                                      && top() > -1 && left() > -1; }

    QModelIndexList items(const QAbstractItemModel *model) const;

private:
    const QAbstractItemModel *m;
    QPersistentModelIndex tl, br;
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

    Q_DECLARE_FLAGS(SelectionFlags, SelectionFlag)

    QItemSelectionModel(QAbstractItemModel *model, QObject *parent = 0);
    virtual ~QItemSelectionModel();

    QModelIndex currentIndex() const;

    bool isSelected(const QModelIndex &index) const;
    bool isRowSelected(int row, const QModelIndex &parent) const;
    bool isColumnSelected(int column, const QModelIndex &parent) const;

    bool rowIntersectsSelection(int row, const QModelIndex &parent) const;
    bool columnIntersectsSelection(int column, const QModelIndex &parent) const;

    QAbstractItemModel *model() const;
    QModelIndexList selectedIndexes() const;
    const QItemSelection selection() const;

public slots:
    void setCurrentIndex(const QModelIndex &index, SelectionFlags command);
    virtual void select(const QModelIndex &index, SelectionFlags command);
    virtual void select(const QItemSelection &selection, SelectionFlags command);
    virtual void clear();
    virtual void reset();

signals:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentColumnChanged(const QModelIndex &current, const QModelIndex &previous);

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
