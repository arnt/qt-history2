#ifndef QGENERICCOMBOBOX_P_H
#define QGENERICCOMBOBOX_P_H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qgenericcombobox.h>
#include <private/qabstractitemview_p.h>
#endif // QT_H

class ComboListView : public QGenericListView
{
    Q_OBJECT

public:
    ComboListView(QAbstractItemModel *model, QWidget *parent = 0);
    bool ignoreNextMousePress();

protected:
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool event(QEvent * e);

signals:
    void itemSelected(const QModelIndex &);

private:
    bool ignoreMousePress;
};

class QGenericComboBoxPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericComboBox)
public:
    QGenericComboBoxPrivate()
        : QAbstractItemViewPrivate(),
          editable(false),
          listView(0),
          insertion(QGenericComboBox::AtBottom),
          autoCompletion(false),
          sizeLimit(10),
          ignoreMousePressEvent(false) {}
    ~QGenericComboBoxPrivate() {}
    void init();

    bool editable;
    ComboListView *listView;
    QGenericComboBox::InsertionPolicy insertion;
    bool autoCompletion;
    int sizeLimit;
    bool ignoreMousePressEvent;
};

#endif //QGENERICCOMBOBOX_P_H
