#ifndef VARIANTDELEGATE_H
#define VARIANTDELEGATE_H

#include <QItemDelegate>

class VariantDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    VariantDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:
    void emitCommitData();
};

#endif
