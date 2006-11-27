#ifndef STARDELEGATE_H
#define STARDELEGATE_H

#include <QItemDelegate>

class StarDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    StarDelegate(QWidget *parent = 0)
        : QItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, 
		   const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, 
		      const QModelIndex &index) const;

private slots:
    void commitAndCloseEditor(); 
};

#endif
