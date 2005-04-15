#ifndef IMAGEDELEGATE_H
#define IMAGEDELEGATE_H

#include <QItemDelegate>

class ImageDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ImageDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:
    void emitCommitData();
};

#endif
