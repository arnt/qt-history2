#ifndef QLISTMODEL_H
#define QLISTMODEL_H

#ifndef QT_H
#include <qiconset.h>
#include <qstring.h>
#include <qvector.h>
#include <qgenericitemmodel.h>
#endif

class Q_GUI_EXPORT QListModelItem
{

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

#endif // QLISTMODEL_H
