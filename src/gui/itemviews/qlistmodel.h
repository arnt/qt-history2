#ifndef QLISTMODEL_H
#define QLISTMODEL_H

#ifndef QT_H
#include <qgenericitemmodel.h>
#include <qlist.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class QListModel;

class Q_GUI_EXPORT QListModelItem
{
    friend class QListModel;

public:
    QListModelItem()  : edit(true), select(true) {}
    ~QListModelItem() {}

    inline QString text() const { return data(QGenericItemModel::Display).toString(); }
    inline QIconSet iconSet() const { return data(QGenericItemModel::Decoration).toIconSet(); }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { setData(QGenericItemModel::Display, text); }
    inline void setIconSet(const QIconSet &iconSet) { setData(QGenericItemModel::Display, iconSet); }
    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    bool operator ==(const QListModelItem &other) const;

    inline bool operator !=(const QListModelItem &other) const { return !operator==(other); }

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

private:
    struct Data {
	int role;
	QVariant value;
    public:
	Data() {}
	Data(int r, QVariant v) {
	    role = r;
	    value = v;
	}
    };

    QVector<Data> values;
    uint edit : 1;
    uint select : 1;
};


class QListModelPrivate;

class Q_GUI_EXPORT QListModel : public QGenericItemModel
{
    friend class QListModelItem;

    Q_OBJECT
    Q_DECLARE_PRIVATE(QListModel);

public:
    QListModel(QObject *parent = 0);
    ~QListModel();

    virtual void setText(int row, const QString &text);
    virtual void setIconSet(int row, const QIconSet &iconSet);
    QString text(int row) const;
    QIconSet iconSet(int row) const;

    QListModelItem item(int row) const;
    void setItem(int row, const QListModelItem &item);
    void append(const QListModelItem &item);

private:
    QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                      QModelIndex::Type type = QModelIndex::View) const;

    int rowCount(const QModelIndex &parent = 0) const;
    int columnCount(const QModelIndex &parent = 0) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);

    QModelIndex insertItem(const QModelIndex &index);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;
};

#endif // QLISTMODEL_H
