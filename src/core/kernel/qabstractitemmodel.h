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

#include <QtCore/qvariant.h>
#include <QtCore/qobject.h>

class QAbstractItemModel;
class QPersistentModelIndex;

class Q_CORE_EXPORT QModelIndex
{
    friend class QAbstractItemModel;
public:
    inline QModelIndex() : r(-1), c(-1), d(0), m(0) {}
    inline QModelIndex(const QModelIndex &other)
        : r(other.row()), c(other.column()), d(other.data()), m(other.model()) {}
    inline ~QModelIndex() { d = 0; m = 0; }
    inline int row() const { return r; }
    inline int column() const { return c; }
    inline void *data() const { return d; }
    inline QModelIndex parent() const;
    inline QModelIndex sibling(int row, int column) const;
    inline QModelIndex child(int row, int column) const;
    inline const QAbstractItemModel *model() const { return m; }
    inline bool isValid() const { return (r >= 0) && (c >= 0) && (m != 0); }
    inline bool operator==(const QModelIndex &other) const
        { return (other.r == r && other.c == c && other.d == d && other.m == m); }
    inline bool operator!=(const QModelIndex &other) const
        { return !(*this == other); }
private:
    inline QModelIndex(int row, int column, void *data, const QAbstractItemModel *model)
        : r(row), c(column), d(data), m(model) {}
    int r, c;
    void *d;
    const QAbstractItemModel *m;
};
Q_DECLARE_TYPEINFO(QModelIndex, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QModelIndex &);
#endif

class QPersistentModelIndexData;

class Q_CORE_EXPORT QPersistentModelIndex
{
public:
    QPersistentModelIndex();
    QPersistentModelIndex(const QModelIndex &index);
    QPersistentModelIndex(const QPersistentModelIndex &other);
    ~QPersistentModelIndex();
    bool operator<(const QPersistentModelIndex &other) const;
    bool operator==(const QPersistentModelIndex &other) const;
    void operator=(const QPersistentModelIndex &other);
    bool operator==(const QModelIndex &other) const;
    bool operator!=(const QModelIndex &other) const;
    void operator=(const QModelIndex &other);
    operator const QModelIndex&() const;
    int row() const;
    int column() const;
    void *data() const;
    QModelIndex parent() const;
    QModelIndex sibling(int row, int column) const;
    QModelIndex child(int row, int column) const;
    const QAbstractItemModel *model() const;
    bool isValid() const;
private:
    QPersistentModelIndexData *d;
#ifndef QT_NO_DEBUG_OUTPUT
    friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);
#endif
};
Q_DECLARE_TYPEINFO(QPersistentModelIndex, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);
#endif

template<typename T> class QList;
typedef QList<QModelIndex> QModelIndexList;

class QMimeData;
class QAbstractItemModelPrivate;
template <class Key, class T> class QMap;


class Q_CORE_EXPORT QAbstractItemModel : public QObject
{
    Q_OBJECT

public:
    enum Role {
        DisplayRole = 0,
        DecorationRole = 1,
        EditRole = 2,
        ToolTipRole = 3,
        StatusTipRole = 4,
        WhatsThisRole = 5,
        // Metadata
        FontRole = 6,
        TextAlignmentRole = 7,
        BackgroundColorRole = 8,
        TextColorRole = 9,
        CheckStateRole = 10,
        // Accessibility
        AccessibleTextRole = 11,
        AccessibleDescriptionRole = 12,
        // Reserved
        UserRole = 32
    };

    enum MatchFlag {
        MatchContains = 0,
        MatchFromStart = 1,
        MatchFromEnd = 2,
        MatchExactly = MatchFromStart | MatchFromEnd,
        MatchCase = 4,
        MatchWrap = 8
    };

    Q_DECLARE_FLAGS(MatchFlags, MatchFlag)

    enum ItemFlag {
        ItemIsSelectable = 1,
        ItemIsEditable = 2,
        ItemIsDragEnabled = 4,
        ItemIsDropEnabled = 8,
        ItemIsUserCheckable = 16,
        ItemIsEnabled = 32,
        ItemIsTristate = 64
    };

    Q_DECLARE_FLAGS(ItemFlags, ItemFlag)

    explicit QAbstractItemModel(QObject *parent = 0);
    virtual ~QAbstractItemModel();

    bool hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const = 0;
    virtual QModelIndex parent(const QModelIndex &child) const = 0;

    inline QModelIndex sibling(int row, int column, const QModelIndex &idx) const
        { return index(row, column, parent(idx)); }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = DisplayRole) const = 0;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = EditRole);

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = DisplayRole) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                               int role = EditRole);

    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, const QModelIndex &parent);
    virtual Qt::DropActions supportedDropActions() const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    inline bool insertRow(int row, const QModelIndex &parent = QModelIndex())
        { return insertRows(row, 1, parent); }
    inline bool insertColumn(int column, const QModelIndex &parent = QModelIndex())
        { return insertColumns(column, 1, parent); }
    inline bool removeRow(int row, const QModelIndex &parent = QModelIndex())
        { return removeRows(row, 1, parent); }
    inline bool removeColumn(int column, const QModelIndex &parent = QModelIndex())
        { return removeColumns(column, 1, parent); }

    virtual void fetchMore(const QModelIndex &parent);
    virtual ItemFlags flags(const QModelIndex &index) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual QModelIndex buddy(const QModelIndex &index) const;
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                                  int hits = 1,
                                  MatchFlags flags = MatchFlags(MatchFromStart | MatchWrap)) const;
    virtual QSize span(const QModelIndex &index) const;

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void headerDataChanged(Qt::Orientation orientation, int first, int last);
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void columnsInserted(const QModelIndex &parent, int first, int last);
    void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void reset();

public slots:
    virtual bool submit();
    virtual void revert();

protected slots:
    void resetPersistentIndexes();

protected:
    QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent = 0);

    inline QModelIndex createIndex(int row, int column, void *data = 0) const
        { return QModelIndex(row, column, data, this); }

    void encodeData(const QModelIndexList &indexes, QDataStream &stream) const;
    void encodeData(const QModelIndex &parent, QDataStream &stream) const;
    bool decodeData(int row, const QModelIndex &parent, QDataStream &stream);

    // Persistent indexes
    void invalidatePersistentIndex(const QModelIndex &index);
    void invalidatePersistentIndexes(const QModelIndex &parent = QModelIndex());
    int persistentIndexesCount() const;
    QModelIndex persistentIndexAt(int position) const;
    void setPersistentIndex(int position, const QModelIndex &index);
    int persistentIndexPosition(const QModelIndex &index, int from = 0) const;

    friend class QPersistentModelIndexData;

private:
    Q_DECLARE_PRIVATE(QAbstractItemModel)
    Q_DISABLE_COPY(QAbstractItemModel)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemModel::MatchFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemModel::ItemFlags);

class Q_CORE_EXPORT QAbstractTableModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QAbstractTableModel(QObject *parent = 0);
    ~QAbstractTableModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

protected:
    QAbstractTableModel(QAbstractItemModelPrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(QAbstractTableModel)
    QModelIndex parent(const QModelIndex &child) const;
    bool hasChildren(const QModelIndex &parent) const;
};

class Q_CORE_EXPORT QAbstractListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QAbstractListModel(QObject *parent = 0);
    ~QAbstractListModel();

    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

protected:
    QAbstractListModel(QAbstractItemModelPrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(QAbstractListModel)
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;
};

// inline implementations

inline QModelIndex QModelIndex::parent() const
{ return m ? m->parent(*this) : QModelIndex(); }

inline QModelIndex QModelIndex::sibling(int row, int column) const
{ return m ? m->index(row, column, m->parent(*this)) : QModelIndex(); }

inline QModelIndex QModelIndex::child(int row, int column) const
{ return m ? m->index(row, column, *this) : QModelIndex(); }

#endif // QABSTRACTITEMMODEL_H
