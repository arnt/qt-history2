#ifndef QLISTVIEW_H
#define QLISTVIEW_H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qabstractitemmodel.h>
#include <qiconset.h>
#include <qstring.h>
#include <qvector.h>
#endif

class Q_GUI_EXPORT QListViewItem
{

public:
    QListViewItem()  : edit(true), select(true) {}
    ~QListViewItem() {}

    inline QString text() const { return data(QAbstractItemModel::Display).toString(); }
    inline QIconSet iconSet() const { return data(QAbstractItemModel::Decoration).toIconSet(); }

    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { setData(QAbstractItemModel::Display, text); }
    inline void setIconSet(const QIconSet &iconSet) { setData(QAbstractItemModel::Display, iconSet); }

    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    bool operator ==(const QListViewItem &other) const;
    inline bool operator !=(const QListViewItem &other) const { return !operator==(other); }

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

private:
    struct Data {
        Data() {}
        Data(int r, QVariant v) {
            role = r;
            value = v;
        }
        int role;
        QVariant value;
    };

    QVector<Data> values;
    uint edit : 1;
    uint select : 1;
};

class QListViewPrivate;

class Q_GUI_EXPORT QListView : public QGenericListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListView)

public:
#ifdef QT_COMPAT
    QListView(QWidget *parent, const char* name);
#endif
    QListView(QWidget *parent = 0);
    ~QListView();

    void setText(int row, const QString &text);
    void setIconSet(int row, const QIconSet &iconSet);
    QString text(int row) const;
    QIconSet iconSet(int row) const;

    QListViewItem item(int row) const;
    void setItem(int row, const QListViewItem &item);
    void appendItem(const QListViewItem &item);
};

#endif
