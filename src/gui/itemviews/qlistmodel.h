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
    QListModelItem(QListModel *model = 0);
    ~QListModelItem() {}

    inline QString text() const { return txt; }
    inline QIconSet iconSet() const { return icn; }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { txt = text; }
    inline void setIconSet(const QIconSet &iconSet) { icn = iconSet; }
    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    inline bool operator ==(const QListModelItem &other) const
	{ return txt == other.txt && edit == other.edit && select == other.select; }
    inline bool operator !=(const QListModelItem &other) const { return !operator==(other); }

private:
    QString txt;
    QIconSet icn;
    uint edit : 1;
    uint select : 1;
};

class Q_GUI_EXPORT QListModel : public QGenericItemModel
{
    friend class QListModelItem;

    Q_OBJECT

public:
    QListModel(QObject *parent = 0, const char *name = 0);
    ~QListModel();

    virtual void setText(int row, const QString &text);
    virtual void setIconSet(int row, const QIconSet &iconSet);
    QString text(int row) const;
    QIconSet iconSet(int row) const;

    const QListModelItem *item(const QModelIndex &index) const;

    QModelIndex index(QListModelItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                      QModelIndex::Type type = QModelIndex::View) const;

    int rowCount(const QModelIndex &parent = 0) const;
    int columnCount(const QModelIndex &parent = 0) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);

    QModelIndex insertItem(const QModelIndex &index);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

protected:
    void append(QListModelItem *item);

private:
    QList<QListModelItem*> lst;
};

#endif // QLISTMODEL_H
