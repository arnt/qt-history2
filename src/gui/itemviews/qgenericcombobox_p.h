#ifndef QGENERICCOMBOBOX_P_H
#define QGENERICCOMBOBOX_P_H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qlineedit.h>
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
          lineEdit(0),
          listView(0),
          delegate(0),
          insertionPolicy(QGenericComboBox::AtBottom),
          autoCompletion(true),
          duplicatesEnabled(false),
          sizeLimit(10),
          ignoreMousePressEvent(false),
          skipCompletion(false) {}
    ~QGenericComboBoxPrivate() {}
    void init();
    void updateLineEditGeometry();
    void returnPressed();
    void complete();
    void itemSelected(const QModelIndex &item);
    bool contains(const QString &text, int role);

    QLineEdit *lineEdit;
    ComboListView *listView;
    QAbstractItemDelegate *delegate;
    QGenericComboBox::InsertionPolicy insertionPolicy;
    bool autoCompletion;
    bool duplicatesEnabled;
    int sizeLimit;
    bool ignoreMousePressEvent;
    bool skipCompletion;
    mutable QSize sizeHint;
};

#endif //QGENERICCOMBOBOX_P_H
