#ifndef QTREEVIEW_H
#define QTREEVIEW_H

#include <qgenerictreeview.h>
#include <qtreemodel.h>

class QTreeView : public QGenericTreeView
{
    Q_OBJECT
    
public:
    QTreeView(QWidget *parent = 0, const char *name = 0);
    QTreeView(QTreeModel *model, QWidget *parent = 0, const char *name = 0);

    virtual void setColumnCount(int columns);
    int columnCount() const;
/*
    virtual void setText(const QModelIndex &item, int column, const QString &text);
    virtual void setIconSet(const QModelIndex &item, int column, const QIconSet &iconSet);
    QString text(const QModelIndex &item, int column) const;
    QIconSet iconSet(const QModelIndex &item, int column) const;
*/  
    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

    inline QTreeModel *tree() const { return ::qt_cast<QTreeModel*>(model()); }
};

#endif
