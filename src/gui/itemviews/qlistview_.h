#ifndef QLISTVIEW__H
#define QLISTVIEW__H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qgenericitemmodel.h>
#include <qiconset.h>
#include <qstring.h>
#include <qvector.h>
#endif

class Q_GUI_EXPORT QListView_Item
{

public:
    QListView_Item()  : edit(true), select(true) {}
    ~QListView_Item() {}

    inline QString text() const { return data(QGenericItemModel::Display).toString(); }
    inline QIconSet iconSet() const { return data(QGenericItemModel::Decoration).toIconSet(); }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { setData(QGenericItemModel::Display, text); }
    inline void setIconSet(const QIconSet &iconSet) { setData(QGenericItemModel::Display, iconSet); }
    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    bool operator ==(const QListView_Item &other) const;

    inline bool operator !=(const QListView_Item &other) const { return !operator==(other); }

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

class QListView_Private;

class Q_GUI_EXPORT QListView_ : public QGenericListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListView_);

public:
    QListView_(QWidget *parent = 0);
    ~QListView_();

    void setText(int row, const QString &text);
    void setIconSet(int row, const QIconSet &iconSet);
//     void setEditable(bool editable);
//     void setSelectable(bool selectable);

    QString text(int row) const;
    QIconSet iconSet(int row) const;
//     bool isEditable(int row) const;
//     bool isSelectable(int row) const;

    QListView_Item item(int row) const;
    void setItem(int row, const QListView_Item &item);
    void appendItem(const QListView_Item &item);
};

#endif // QLISTVIEW_H
