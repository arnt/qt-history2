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

#ifndef QABSTRACTITEMMODEL_H
#define QABSTRACTITEMMODEL_H

#ifndef QT_H
#include <qobject.h>
#include <qvariant.h>
#endif

class QAbstractItemModel;

class Q_GUI_EXPORT QModelIndex
{
    friend class QAbstractItemModel;
public:
    enum Type { Null };
    inline QModelIndex(Type type = Null) : r(-1), c(-1), d(0) {}
    inline QModelIndex(const QModelIndex &other)
        : r(other.row()), c(other.column()), d(other.data()) {}
    inline ~QModelIndex() { d = 0; }
    inline int row() const { return r; }
    inline int column() const { return c; }
    inline void *data() const { return d; }
    inline bool isValid() const { return (r >= 0) && (c >= 0); }
    inline bool operator==(const QModelIndex &other) const
    { return (other.r == r && other.c == c && other.d == d); }
    inline bool operator!=(const QModelIndex &other) const { return !(*this == other); }
private:
    inline QModelIndex(int row, int column, void *data)
        : r(row), c(column), d(data) {}
    int r, c;
    void *d;
};

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QModelIndex &);
#endif

class QPersistentModelIndexData;

class Q_GUI_EXPORT QPersistentModelIndex
{
public:
    QPersistentModelIndex();
    QPersistentModelIndex(const QModelIndex &index, QAbstractItemModel *model);
    QPersistentModelIndex(const QPersistentModelIndex &other);
    ~QPersistentModelIndex();
    bool operator<(const QPersistentModelIndex &other) const;
    bool operator==(const QPersistentModelIndex &other) const;
    void operator=(const QPersistentModelIndex &other);
    operator const QModelIndex&() const;
    int row() const;
    int column() const;
    void *data() const;
    bool isValid() const;
    bool operator==(const QModelIndex &other) const;
    bool operator!=(const QModelIndex &other) const;
private:
    QPersistentModelIndexData *d;
    friend Q_GUI_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);
};

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);
#endif

template<typename T>
class QList;
typedef QList<QModelIndex> QModelIndexList;

class QWidget;
class QDropEvent;
class QDragObject;
class QMimeSource;
class QVariant;
class QAbstractItemModelPrivate;
template <class Key, class T>
class QMap;


class Q_GUI_EXPORT QAbstractItemModel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemModel)

public:
    enum Role {
        DisplayRole = 0,
        DecorationRole = 1,
        EditRole = 2,
        ToolTipRole = 3,
        StatusTipRole = 4,
        WhatsThisRole = 5,
        UserRole = 32
    };

    enum MatchFlag {
        MatchContains = 0,
        MatchFromStart = 1,
        MatchFromEnd = 2,
        MatchExactly = MatchFromStart | MatchFromEnd,
        MatchCase = 4,
        MatchWrap = 8,
        MatchDefault = MatchFromStart | MatchWrap
    };

    Q_DECLARE_FLAGS(MatchFlags, MatchFlag);

    QAbstractItemModel(QObject *parent = 0);
    virtual ~QAbstractItemModel();

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex::Null) const = 0;
    virtual QModelIndex parent(const QModelIndex &child) const = 0;

    inline QModelIndex sibling(int row, int column, const QModelIndex &idx) const
        { return index(row, column, parent(idx)); }

    virtual int rowCount(const QModelIndex &parent) const = 0;
    virtual int columnCount(const QModelIndex &parent) const = 0;
    virtual bool hasChildren(const QModelIndex &parent) const;

    virtual bool canDecode(QMimeSource *src) const;
    virtual bool decode(QDropEvent *e, const QModelIndex &parent = QModelIndex::Null);
    virtual QDragObject *dragObject(const QModelIndexList &indices, QWidget *dragSource);

    virtual QVariant data(const QModelIndex &index, int role = DisplayRole) const = 0;
    virtual bool setData(const QModelIndex &index, int role, const QVariant &value);
    inline bool setData(const QModelIndex &index, const QVariant &value)
        { return setData(index, EditRole, value); }

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = DisplayRole) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, int role,
                               const QVariant &value);
    inline bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value)
        { return setHeaderData(section, orientation, EditRole, value); }


    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    virtual bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null,
                            int count = 1);
    virtual bool insertColumns(int column, const QModelIndex &parent = QModelIndex::Null,
                               int count = 1);
    virtual bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null,
                            int count = 1);
    virtual bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null,
                               int count = 1);

    virtual void fetchMore(const QModelIndex &parent);

    virtual bool isSelectable(const QModelIndex &index) const;
    virtual bool isEditable(const QModelIndex &index) const;
    virtual bool isDragEnabled(const QModelIndex &index) const;
    virtual bool isDropEnabled(const QModelIndex &index) const;

    virtual bool isSortable() const;
    virtual void sort(int column, Qt::SortOrder order);

    virtual bool equal(const QModelIndex &left, const QModelIndex &right) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    inline bool greaterThan(const QModelIndex &left, const QModelIndex &right) const
        { return lessThan(right, left); }

    virtual QModelIndex buddy(const QModelIndex &index) const;

    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                                  int hits = 1, MatchFlags flags = MatchDefault) const;

    virtual QSize span(const QModelIndex &index) const;

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void headerDataChanged(Qt::Orientation orientation, int first, int last);
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);
    void columnsInserted(const QModelIndex &parent, int first, int last);
    void columnsRemoved(const QModelIndex &parent, int first, int last);
    void reset();

protected slots:
    void resetPresistentIndexes();
    
protected:
    QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent);

    inline QModelIndex createIndex(int row = -1, int column = -1, void *data = 0) const
        { return QModelIndex(row, column, data); }

    bool isValid(int row, int column, const QModelIndex &parent) const;

    void invalidatePersistentIndexes(const QModelIndex &parent = QModelIndex::Null);
    int persistentIndexesCount() const;
    QModelIndex persistentIndexAt(int position) const;
    void setPersistentIndex(int position, const QModelIndex &index);

    friend class QPersistentModelIndexData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemModel::MatchFlags);

class Q_GUI_EXPORT QAbstractTableModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    QAbstractTableModel(QObject *parent = 0);
    ~QAbstractTableModel();

    virtual int rowCount() const = 0;
    virtual int columnCount() const = 0;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;

protected:
    QAbstractTableModel(QAbstractItemModelPrivate &dd, QObject *parent);

private:
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
};

class Q_GUI_EXPORT QAbstractListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    QAbstractListModel(QObject *parent = 0);
    ~QAbstractListModel();

    virtual int rowCount() const = 0;
    int columnCount() const { return 1; }
    
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex::Null) const;
    
protected:
    QAbstractListModel(QAbstractItemModelPrivate &dd, QObject *parent);

private:
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
};

#endif
