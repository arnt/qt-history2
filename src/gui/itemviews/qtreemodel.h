#ifndef QTREEMODEL_H
#define QTREEMODEL_H

#ifndef QT_H
#include <qgenericitemmodel.h>
#include <qlist.h>
#include <qvector.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class QTreeModel;

class Q_GUI_EXPORT QTreeModelItem
{
    friend class QTreeModel;

public:
    QTreeModelItem(QTreeModel *model);
    QTreeModelItem(QTreeModelItem *parent);
    virtual ~QTreeModelItem();

    inline const QTreeModelItem *parent() const { return par; }
    inline const QTreeModelItem *child(int index) const { return children.at(index); }
    inline int childCount() const { return children.count(); }
    inline int columnCount() const { return c; }

    inline QString text(int column) const { return column < c ? txt[column] : QString(); }
    inline QIconSet iconSet(int column) const { return column < c ? icn[column] : QIconSet(); }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    virtual void setColumnCount(int columns);
    virtual void setText(int column, const QString &text);
    virtual void setIconSet(int column, const QIconSet &iconSet);
    virtual void setEditable(bool editable) { edit = editable; }
    virtual void setSelectable(bool selectable) { select = selectable; }
    
    inline bool operator ==(const QTreeModelItem &other) const
	{ return par == other.par && children == other.children; }
    inline bool operator !=(const QTreeModelItem &other) const { return !operator==(other); }

    inline QTreeModel *model() const { return mod; }

private:
    QTreeModelItem();
    QTreeModelItem(const QVariant &values);
    QTreeModelItem *par;
    QList<QTreeModelItem*> children;
    QVector<QString> txt;
    QVector<QIconSet> icn;
    QTreeModel *mod;
    int c;
    uint edit : 1;
    uint select : 1;
};

class Q_GUI_EXPORT QTreeModel : public QGenericItemModel
{
    friend class QTreeModelItem;

    Q_OBJECT

public:
    QTreeModel(int columns = 0, QObject *parent = 0, const char *name = 0);
    ~QTreeModel();

    virtual void setColumnCount(int columns);
    int columnCount() const;
/*
    virtual void setText(const QModelIndex &index, int column, const QString &text);
    virtual void setIconSet(const QModelIndex &index, int column, const QIconSet &iconSet);
    QString text(const QModelIndex &index, int column) const;
    QIconSet iconSet(const QModelIndex &index, int column) const;
*/
    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

    QTreeModelItem *item(const QModelIndex &index) const;

    QModelIndex index(QTreeModelItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int element) const;
    void setData(const QModelIndex &index, int element, const QVariant &variant);
    void insertDataList(const QModelIndex &index, const QVariant &variant);
    void appendDataList(const QVariant &variant);

    QVariant::Type type(const QModelIndex &index, int element) const;
    int element(const QModelIndex &index, QVariant::Type type) const;

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

protected:
    void append(QTreeModelItem *item);
    void emitContentsInserted(QTreeModelItem *item);

private:
    int c;
    QList<QTreeModelItem*> tree;
    mutable QTreeModelItem topHeader;
};

#endif // QTREEMODEL_H
