#ifndef QGENERICCOMBOBOX_H
#define QGENERICCOMBOBOX_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericComboBoxPrivate;

class Q_GUI_EXPORT QGenericComboBox : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericComboBox)
    Q_ENUMS(InsertionPolicy)
    Q_PROPERTY(int sizeLimit READ sizeLimit WRITE setSizeLimit)

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

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &index);
    QModelIndex itemAt(int x, int y) const;

protected:
    QGenericComboBox(QGenericComboBoxPrivate &dd, QAbstractItemModel *model, QWidget *parent = 0);

    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);

    int horizontalOffset() const;
    int verticalOffset() const;


    void setSelection(const QRect&, int command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *e);

    bool startEdit(const QModelIndex &item,
                   QAbstractItemDelegate::StartEditAction action,
                   QEvent *event);

private:
    void popupListView();
};

#endif /* QGENERICCOMBOBOX_H */
