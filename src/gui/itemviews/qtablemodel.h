#ifndef QTABLEMODEL_H
#define QTABLEMODEL_H

#ifndef QT_H
#include <qgenericitemmodel.h>
#include <qvector.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class QTableModel;

class Q_GUI_EXPORT QTableModelItem
{
    friend class QTableModel;

public:
    QTableModelItem() : edit(true), select(true) {}
    ~QTableModelItem() {}

    inline QString text() const { return txt; }
    inline QIconSet iconSet() const { return icn; }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { txt = text; }
    inline void setIconSet(const QIconSet &iconSet) { icn = iconSet; }
    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

private:
    QString txt;
    QIconSet icn;
    uint edit : 1;
    uint select : 1;
};

class Q_GUI_EXPORT QTableModel : public QGenericItemModel
{
    Q_OBJECT

public:
    QTableModel(int rows = 0, int columns = 0, QObject *parent = 0);
    ~QTableModel();

    virtual void setRowCount(int rows);
    virtual void setColumnCount(int columns);

    virtual void insertRow(int at);
    virtual void insertColumn(int at);

    virtual void removeRow(int at);
    virtual void removeColumn(int at);

    virtual void setText(int row, int column, const QString &text);
    virtual void setIconSet(int row, int column, const QIconSet &iconSet);
    QString text(int row, int column) const;
    QIconSet iconSet(int row, int column) const;

    virtual void setRowText(int row, const QString &text);
    virtual void setRowIconSet(int row, const QIconSet &iconSet);
    QString rowText(int row) const;
    QIconSet rowIconSet(int row) const;

    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

    void setItem(int row, int column, QTableModelItem *item);
    const QTableModelItem *item(int row, int column) const;
    const QTableModelItem *item(const QModelIndex &index) const;
    QTableModelItem *item(const QModelIndex &index);

    QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                      QModelIndex::Type type = QModelIndex::View) const;

    int rowCount(const QModelIndex &parent = 0) const;
    int columnCount(const QModelIndex &parent = 0) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);

    QModelIndex insertItem(const QModelIndex &index);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

private:
    bool isValid(const QModelIndex &index) const;
    inline int tableIndex(int row, int column) const { return (row * c) + column; }
    int r, c;
    QVector<QTableModelItem*> table;
    QVector<QTableModelItem*> leftHeader;
    QVector<QTableModelItem*> topHeader;
};

#endif // QTABLEMODEL
