#ifndef QTABLEVIEW_H
#define QTABLEVIEW_H

#ifndef QT_H
#include <qgenerictableview.h>
#include <qabstractitemmodel.h>
#include <qvector.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class Q_GUI_EXPORT QTableViewItem
{

public:
    QTableViewItem() : edit(true), select(true) {}
    ~QTableViewItem() {}

    inline QString text() const { return data(QAbstractItemModel::Display).toString(); }
    inline QIconSet iconSet() const { return data(QAbstractItemModel::Decoration).toIconSet(); }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { setData(QAbstractItemModel::Display, text); }
    inline void setIconSet(const QIconSet &iconSet) { setData(QAbstractItemModel::Display, iconSet); }
    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    bool operator ==(const QTableViewItem &other) const;
    inline bool operator !=(const QTableViewItem &other) const { return !operator==(other); }

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

class QTableViewPrivate;

class Q_GUI_EXPORT QTableView : public QGenericTableView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTableView);

public:
    QTableView(QWidget *parent = 0);
    ~QTableView();

    void setRowCount(int rows);
    void setColumnCount(int columns);
    int rowCount() const;
    int columnCount() const;
    
    QTableViewItem item(int row, int column) const;
    void setItem(int row, int column, const QTableViewItem &item);

    void setText(int row, int column, const QString &text);
    void setIconSet(int row, int column, const QIconSet &iconSet);
    QString text(int row, int column) const;
    QIconSet iconSet(int row, int column) const;

    void setRowText(int row, const QString &text);
    void setRowIconSet(int row, const QIconSet &iconSet);
    QString rowText(int row) const;
    QIconSet rowIconSet(int row) const;

    void setColumnText(int column, const QString &text);
    void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;
};

#endif
