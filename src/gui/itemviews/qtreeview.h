#ifndef QTREEVIEW_H
#define QTREEVIEW_H

#ifndef QT_H
#include <qgenerictreeview.h>
#include <qlist.h>
#include <qvector.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class QTreeView;
class QTreeModel;

class Q_GUI_EXPORT QTreeViewItem
{
    friend class QTreeModel;
public:
    QTreeViewItem(QTreeView *view);
    QTreeViewItem(QTreeViewItem *parent);
    virtual ~QTreeViewItem();

    inline const QTreeViewItem *parent() const { return par; }
    inline const QTreeViewItem *child(int index) const { return children.at(index); }
    inline QTreeViewItem *child(int index) { return children.at(index); }
    inline int childCount() const { return children.count(); }

    inline int columnCount() const { return c; }
    inline QString text(int column) const { return data(column, QAbstractItemModel::Display).toString(); }
    inline QIconSet iconSet(int column) const { return data(column, QAbstractItemModel::Decoration).toIconSet(); }

    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    void setColumnCount(int columns);
    inline void setText(int column, const QString &text) { setData(column, QAbstractItemModel::Display, text); }
    inline void setIconSet(int column, const QIconSet &iconSet) { setData(column, QAbstractItemModel::Decoration, iconSet); }

    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    inline bool operator ==(const QTreeViewItem &other) const
	{ return par == other.par && children == other.children; }
    inline bool operator !=(const QTreeViewItem &other) const { return !operator==(other); }

    QVariant data(int column, int role) const;
    void setData(int column, int role, const QVariant &value);

private:
    QTreeViewItem();

    QTreeViewItem *par;
    QList<QTreeViewItem*> children;

    struct Data {
	Data() {}
	Data(int r, QVariant v) {
	    role = r;
	    value = v;
	}
	int role;
	QVariant value;
    };

    QVector< QVector<Data> > values;
    QTreeView *view;
    int c;
    uint edit : 1;
    uint select : 1;
};

class QTreeViewPrivate;

class Q_GUI_EXPORT QTreeView : public QGenericTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeView);

    friend class QTreeViewItem;
public:
    QTreeView(QWidget *parent = 0);

    void setColumnCount(int columns);
    int columnCount() const;

    void setColumnText(int column, const QString &text);
    void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

protected:
    void append(QTreeViewItem *item);
};

#endif
