/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOMBOBOX_H
#define QCOMBOBOX_H

#include <QtGui/qwidget.h>
#include <QtGui/qabstractitemmodel.h>
#include <QtGui/qabstractitemdelegate.h>

class QAbstractItemView;
class QLineEdit;
class QComboBoxPrivate;

class Q_GUI_EXPORT QComboBox : public QWidget
{
    Q_OBJECT

    Q_ENUMS(InsertPolicy)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QString currentText READ currentText)
    Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem)
    Q_PROPERTY(int maxVisibleItems READ maxVisibleItems WRITE setMaxVisibleItems)
    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount)
    Q_PROPERTY(InsertPolicy insertPolicy READ insertPolicy WRITE setInsertPolicy)
    Q_PROPERTY(bool autoCompletion READ autoCompletion WRITE setAutoCompletion)
    Q_PROPERTY(bool duplicatesEnabled READ duplicatesEnabled WRITE setDuplicatesEnabled)

public:
    explicit QComboBox(QWidget *parent = 0);
    ~QComboBox();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int maxItems);

    int count() const;
    void setMaxCount(int max);
    int maxCount() const;

    bool autoCompletion() const;
    void setAutoCompletion(bool enable);

    bool duplicatesEnabled() const;
    void setDuplicatesEnabled(bool enable);

    bool autoHide() const;
    void setAutoHide(bool enable);

    inline int findText(const QString &text,
                        QAbstractItemModel::MatchFlags flags =
                        QAbstractItemModel::MatchExactly | QAbstractItemModel::MatchCase) const
        { return findData(text, QAbstractItemModel::EditRole, flags); }
    int findData(const QVariant &data, int role = QAbstractItemModel::UserRole,
                 QAbstractItemModel::MatchFlags flags =
                 QAbstractItemModel::MatchExactly | QAbstractItemModel::MatchCase) const;

    enum InsertPolicy {
        NoInsert,
        InsertAtTop,
        InsertAtCurrent,
        InsertAtBottom,
        InsertAfterCurrent,
        InsertBeforeCurrent
#ifdef QT_COMPAT
        ,
        NoInsertion = NoInsert,
        AtTop = InsertAtTop,
        AtCurrent = InsertAtCurrent,
        AtBottom = InsertAtBottom,
        AfterCurrent = InsertAfterCurrent,
        BeforeCurrent = InsertBeforeCurrent
#endif
    };
#ifdef QT_COMPAT
    typedef InsertPolicy Policy;
#endif

    InsertPolicy insertPolicy() const;
    void setInsertPolicy(InsertPolicy policy);

    bool isEditable() const;
    void setEditable(bool editable);
    void setLineEdit(QLineEdit *edit);
    QLineEdit *lineEdit() const;
    void setValidator(const QValidator *v);
    const QValidator *validator() const;

    QAbstractItemDelegate *itemDelegate() const;
    void setItemDelegate(QAbstractItemDelegate *delegate);

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);

    QModelIndex rootModelIndex() const;
    void setRootModelIndex(const QModelIndex &index);

    int currentItem() const;
    void setCurrentItem(int index);

    QString currentText() const;

    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    QVariant itemData(int index, int role = QAbstractItemModel::UserRole) const;

    inline void addItem(const QString &text, const QVariant &userData = QVariant())
        { insertItem(count(), text, userData); }
    inline void addItem(const QIcon &icon, const QString &text,
                        const QVariant &userData = QVariant())
        { insertItem(count(), icon, text, userData); }
    inline void addItems(const QStringList &texts)
        { insertItems(count(), texts); }

    inline void insertItem(int index, const QString &text, const QVariant &userData = QVariant())
        { insertItem(index, QIcon(), text, userData); }
    void insertItem(int index, const QIcon &icon, const QString &text,
                    const QVariant &userData = QVariant());
    void insertItems(int index, const QStringList &texts);

    void removeItem(int index);

    void setItemText(int index, const QString &text);
    void setItemIcon(int index, const QIcon &icon);
    void setItemData(int index, const QVariant &value, int role = QAbstractItemModel::UserRole);

    QAbstractItemView *itemView() const;
    void setItemView(QAbstractItemView *itemView);

    QSize sizeHint() const;

    virtual void popup();

    void hide();

public slots:
    void clear();
    void clearEditText();
    virtual void setEditText(const QString &text);

signals:
    void editTextChanged(const QString &);
    void activated(int index);
    void activated(const QString &);
    void highlighted(int index);
    void highlighted(const QString &);

protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);
    void inputMethodEvent(QInputMethodEvent *);
    QVariant inputMethodQuery(Qt::InputMethodQuery) const;

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QComboBox(QWidget *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QComboBox(bool rw, QWidget *parent, const char *name = 0);
    inline QT_COMPAT InsertPolicy insertionPolicy() const { return insertPolicy(); }
    inline QT_COMPAT void setInsertionPolicy(InsertPolicy policy) { setInsertPolicy(policy); }
    inline QT_COMPAT bool editable() const { return isEditable(); }
    inline QT_COMPAT void setCurrentText(const QString& text) {
        int i = findText(text);
        if (i != -1)
            setCurrentItem(i);
        else if (isEditable())
            setEditText(text);
        else
            setItemText(currentItem(), text);
    }
    inline QT_COMPAT QString text(int index) const { return itemText(index); }
    inline QT_COMPAT QPixmap pixmap(int index) const { return itemIcon(index).pixmap(Qt::SmallIconSize); }
    inline QT_COMPAT void insertStringList(const QStringList &list, int index = -1)
        { insertItems(index, list); }
    inline QT_COMPAT void insertItem(const QString &text, int index = -1)
        { insertItem(index, text); }
    inline QT_COMPAT void insertItem(const QPixmap &pix, int index = -1)
        { insertItem(index, QIcon(pix), QString()); }
    inline QT_COMPAT void insertItem(const QPixmap &pix, const QString &text, int index = -1)
        { insertItem(index, QIcon(pix), text); }
    inline QT_COMPAT void changeItem(const QString &text, int index)
        { setItemText(index, text); }
    inline QT_COMPAT void changeItem(const QPixmap &pix, int index)
        { setItemIcon(index, QIcon(pix)); }
    inline QT_COMPAT void changeItem(const QPixmap &pix, const QString &text, int index)
        { setItemIcon(index, QIcon(pix)); setItemText(index, text); }
    inline QT_COMPAT void clearValidator() { setValidator(0); }
    inline QT_COMPAT void clearEdit() { clearEditText(); }
#endif

protected:
    QComboBox(QComboBoxPrivate &dd, QWidget *parent = 0);

private:
    Q_DECLARE_PRIVATE(QComboBox)
    Q_DISABLE_COPY(QComboBox)
    Q_PRIVATE_SLOT(d, void itemSelected(const QModelIndex &item))
    Q_PRIVATE_SLOT(d, void emitHighlighted(const QModelIndex &))
    Q_PRIVATE_SLOT(d, void returnPressed())
    Q_PRIVATE_SLOT(d, void complete())
    Q_PRIVATE_SLOT(d, void resetButton())
};

#endif // QCOMBOBOX_H
