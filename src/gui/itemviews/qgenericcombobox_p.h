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
          editor(0),
          listView(0),
          insertionPolicy(QGenericComboBox::AtBottom),
          autoCompletion(true),
          duplicatesEnabled(false),
          sizeLimit(10),
          ignoreMousePressEvent(false),
          lastKey(0) {}
    ~QGenericComboBoxPrivate() {}
    void init();
    void handleReturnPressed();
    void handleTextChanged();
    void itemSelected(const QModelIndex &item);
    bool contains(const QString &text, int role);

    QWidget *editor;
    ComboListView *listView;
    QGenericComboBox::InsertionPolicy insertionPolicy;
    bool autoCompletion;
    bool duplicatesEnabled;
    int sizeLimit;
    bool ignoreMousePressEvent;
    mutable QSize sizeHint;
    int lastKey;
};

#endif //QGENERICCOMBOBOX_P_H
