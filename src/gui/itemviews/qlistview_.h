#ifndef QLISTVIEW__H
#define QLISTVIEW__H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qlistmodel.h>
#endif

class QListView_Private;

class Q_GUI_EXPORT QListView_ : public QGenericListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListView_);

public:
    QListView_(QWidget *parent = 0);
    ~QListView_();

    void setText(int row, const QString &text);
    void setIconSet(int row, const QIconSet &iconSet);
//     void setEditable(bool editable);
//     void setSelectable(bool selectable);

    QString text(int row) const;
    QIconSet iconSet(int row) const;
//     bool isEditable(int row) const;
//     bool isSelectable(int row) const;

    QListModelItem item(int row) const;
    void setItem(int row, const QListModelItem &item);
    void appendItem(const QListModelItem &item);
};

#endif // QLISTVIEW_H
