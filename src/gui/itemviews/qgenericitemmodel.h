#ifndef QGENERICITEMMODEL_H
#define QGENERICITEMMODEL_H

#ifndef QT_H
#include <qobject.h>
#include <qmime.h>
#include <qvariant.h>
#include <qlist.h>
#endif

class QGenericItemModel;

class Q_GUI_EXPORT QModelIndex
{
public:
    enum Type { View, HorizontalHeader, VerticalHeader, ToolTip, WhatsThis };
    inline QModelIndex(const QModelIndex &other)
	: r(other.row()), c(other.column()), d(other.data()), t(other.t) {}
    inline QModelIndex(int row = -1, int column = -1, void *data = 0, Type type = View)
	: r(row), c(column), d(data), t(type) {}
    inline ~QModelIndex() {}
    inline int row() const { return r; }
    inline int column() const { return c; }
    inline void *data() const { return d; }
    inline Type type() const { return t; }
    inline bool isValid() const { return (r >= 0) && (c >= 0); }
    inline bool QModelIndex::operator==(const QModelIndex &other) const
	{ return (other.r == r && other.c == c && other.d == d && other.t == t); }
    inline bool operator!=(const QModelIndex &other) const { return !(*this == other); }
private:
    int r, c;
    void *d;
    Type t;
};

typedef QList<QModelIndex> QModelIndexList;
typedef QList<QCoreVariant> QCoreVariantList;

class QItemModel;
class QModelItem;

class Q_GUI_EXPORT QGenericItemModel : public QObject
{
    Q_OBJECT

public:

    QGenericItemModel(QObject *parent = 0, const char *name = 0);
    virtual ~QGenericItemModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                              QModelIndex::Type type = QModelIndex::View) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    inline QModelIndex topLeft(const QModelIndex &parent = 0) const
	{ return index(0, 0, parent); }
    inline QModelIndex bottomRight(const QModelIndex &parent = 0) const
	{ return index(rowCount(parent) - 1, columnCount(parent) - 1, parent); }
    inline QModelIndex sibling(int row, int column, const QModelIndex &idx) const
	{ return index(row, column, parent(idx)); }

    virtual int rowCount(const QModelIndex &parent = 0) const = 0;
    virtual int columnCount(const QModelIndex &parent = 0) const = 0;
    virtual bool hasChildren(const QModelIndex &parent) const;

    virtual const char *format(int i) const;
    virtual QByteArray encodedData(const char *mime, const QModelIndexList &indices) const;
    virtual bool canDecode(QMimeSource *src) const;
    virtual bool decode(QMimeSource *src);

    virtual QVariant data(const QModelIndex &index, int element) const = 0;
    virtual QVariantList data(const QModelIndex &index) const;
    virtual void setData(const QModelIndex &index, int element, const QVariant &value);
    virtual void setData(const QModelIndex &index, const QVariantList &elements);
    virtual void insertData(const QModelIndex &index, const QVariantList &elements);
    virtual void appendData(const QVariantList &elements);

    virtual QVariant::Type type(const QModelIndex &index, int element) const = 0;
    virtual int element(const QModelIndex &index, QVariant::Type type) const = 0;
    virtual int elementCount(const QModelIndex &index) const;

    virtual bool isSelectable(const QModelIndex &index) const;
    virtual bool isEditable(const QModelIndex &index) const;
    virtual bool isDragEnabled(const QModelIndex &index) const;
    virtual bool isDropEnabled(const QModelIndex &index) const;

    virtual bool isSortable() const;
    virtual void sort(int column, SortOrder order);

    virtual bool equal(const QModelIndex &left, const QModelIndex &right) const;
    virtual bool greater(const QModelIndex &left, const QModelIndex &right) const;
    inline bool less(const QModelIndex &left, const QModelIndex &right) const
	{ return !greater(left, right); }

public slots:
    virtual void fetchMore();

signals:
    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &parent, const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

#endif
