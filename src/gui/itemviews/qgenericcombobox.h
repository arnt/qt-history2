#ifndef QGENERICCOMBOBOX_H
#define QGENERICCOMBOBOX_H

#ifndef QT_H
#include <qwidget.h>
#include <qabstractitemmodel.h>
#include <qabstractitemdelegate.h>
#endif

class QGenericListView;
class QLineEdit;
class QGenericComboBoxPrivate;

class Q_GUI_EXPORT QGenericComboBox : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericComboBox)
    Q_ENUMS(InsertionPolicy)
    Q_PROPERTY(int sizeLimit READ sizeLimit WRITE setSizeLimit)
    Q_PROPERTY(bool autoCompletion READ autoCompletion WRITE setAutoCompletion)
    Q_PROPERTY(bool duplicatesEnabled READ duplicatesEnabled WRITE setDuplicatesEnabled)

public:

    enum InsertionPolicy {
        NoInsertion,
        AtTop,
        AtCurrent,
        AtBottom,
        AfterCurrent,
        BeforeCurrent
    };

    QGenericComboBox(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericComboBox();

    int sizeLimit() const;
    void setSizeLimit(int limit);
    int count() const;

    bool autoCompletion() const;
    void setAutoCompletion(bool enable);

    bool duplicatesEnabled() const;
    void setDuplicatesEnabled(bool enable);

    InsertionPolicy insertionPolicy() const;
    void setInsertionPolicy(InsertionPolicy policy);

    bool isEditable() const;
    void setEditable(bool editable);
    void setLineEdit(QLineEdit *edit);
    QLineEdit *lineEdit() const;

    QAbstractItemDelegate *itemDelegate() const;
    void setItemDelegate(QAbstractItemDelegate *delegate);

    QAbstractItemModel *model() const;

    QModelIndex root() const;
    void setRoot(const QModelIndex &index);

    QModelIndex currentItem() const;
    void setCurrentItem(const QModelIndex &index);

    QString currentText() const;
    void setCurrentText(const QString&);

    QModelIndex insertItem(const QString &text, int row = -1);
    QModelIndex insertItem(const QIconSet &icon, int row = -1);
    QModelIndex insertItem(const QString &text, const QIconSet &icon, int row = -1);

    QModelIndex changeItem(const QString &text, int row);
    QModelIndex changeItem(const QIconSet &icon, int row);
    QModelIndex changeItem(const QString &text, const QIconSet &icon, int row);

    QGenericListView *listView() const;

    QSize sizeHint() const;

signals:
    void textChanged(const QString &);
    void activated(const QModelIndex &);
    void rootChanged(const QModelIndex &old, const QModelIndex &root);

protected slots:
    void currentChanged(const QModelIndex &old, const QModelIndex &current);

protected:
    QGenericComboBox(QGenericComboBoxPrivate &dd, QAbstractItemModel *model, QWidget *parent = 0);

    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);


private:
    void popupListView();
    Q_PRIVATE_SLOT(void itemSelected(const QModelIndex &item))
    Q_PRIVATE_SLOT(void returnPressed())
    Q_PRIVATE_SLOT(void complete())
};

#endif /* QGENERICCOMBOBOX_H */
