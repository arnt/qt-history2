#ifndef QGENERICCOMBOBOX_H
#define QGENERICCOMBOBOX_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericListView;
class QGenericComboBoxPrivate;

class Q_GUI_EXPORT QGenericComboBox : public QAbstractItemView
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

    bool autoCompletion() const;
    void setAutoCompletion(bool enable);

    bool duplicatesEnabled() const;
    void setDuplicatesEnabled(bool enable);

    InsertionPolicy insertionPolicy() const;
    void setInsertionPolicy(InsertionPolicy policy);

    bool isEditable() const;
    void setEditable(bool editable);

    QModelIndex insertItem(const QString &text, int row = -1);
    QModelIndex insertItem(const QIconSet &icon, int row = -1);
    QModelIndex insertItem(const QString &text, const QIconSet &icon, int row = -1);

    QModelIndex changeItem(const QString &text, int row);
    QModelIndex changeItem(const QIconSet &icon, int row);
    QModelIndex changeItem(const QString &text, const QIconSet &icon, int row);

    QGenericListView *listView() const;

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &index);
    QModelIndex itemAt(int x, int y) const;

    QSize sizeHint() const;

signals:
    void textChanged(const QString &);
    void activated(const QModelIndex &);

protected slots:
    void updateCurrentEditor();
    void currentChanged(const QModelIndex &old, const QModelIndex &current);

    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected:
    QGenericComboBox(QGenericComboBoxPrivate &dd, QAbstractItemModel *model, QWidget *parent = 0);

    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);

    int horizontalOffset() const;
    int verticalOffset() const;

    void setSelection(const QRect&, int command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *e);

    bool startEdit(const QModelIndex &item,
                   QAbstractItemDelegate::StartEditAction action,
                   QEvent *event);

    void updateGeometries();


private:
    void popupListView();
    Q_PRIVATE_SLOT(void handleReturnPressed())
    Q_PRIVATE_SLOT(void itemSelected(const QModelIndex &item))
};

#endif /* QGENERICCOMBOBOX_H */
