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
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qabstractitemdelegate.h>
#include <QtCore/qvariant.h>

class QAbstractItemView;
class QLineEdit;
class QComboBoxPrivate;

class Q_GUI_EXPORT QComboBox : public QWidget
{
    Q_OBJECT

    Q_ENUMS(InsertPolicy)
    Q_ENUMS(SizeAdjustPolicy)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QString currentText READ currentText)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(int maxVisibleItems READ maxVisibleItems WRITE setMaxVisibleItems)
    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount)
    Q_PROPERTY(InsertPolicy insertPolicy READ insertPolicy WRITE setInsertPolicy)
    Q_PROPERTY(SizeAdjustPolicy sizeAdjustPolicy READ sizeAdjustPolicy WRITE setSizeAdjustPolicy)
    Q_PROPERTY(int minimumContentsLength READ minimumContentsLength WRITE setMinimumContentsLength)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(bool autoCompletion READ autoCompletion WRITE setAutoCompletion)
    Q_PROPERTY(bool duplicatesEnabled READ duplicatesEnabled WRITE setDuplicatesEnabled)
    Q_PROPERTY(bool frame READ hasFrame WRITE setFrame)

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

    void setFrame(bool);
    bool hasFrame() const;

    inline int findText(const QString &text,
                        QAbstractItemModel::MatchFlags flags =
                        QAbstractItemModel::MatchExactly | QAbstractItemModel::MatchCase) const
        { return findData(text, Qt::EditRole, flags); }
    int findData(const QVariant &data, int role = Qt::UserRole,
                 QAbstractItemModel::MatchFlags flags =
                 QAbstractItemModel::MatchExactly | QAbstractItemModel::MatchCase) const;

    enum InsertPolicy {
        NoInsert,
        InsertAtTop,
        InsertAtCurrent,
        InsertAtBottom,
        InsertAfterCurrent,
        InsertBeforeCurrent
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        ,
        NoInsertion = NoInsert,
        AtTop = InsertAtTop,
        AtCurrent = InsertAtCurrent,
        AtBottom = InsertAtBottom,
        AfterCurrent = InsertAfterCurrent,
        BeforeCurrent = InsertBeforeCurrent
#endif
    };
#ifdef QT3_SUPPORT
    typedef InsertPolicy Policy;
#endif

    InsertPolicy insertPolicy() const;
    void setInsertPolicy(InsertPolicy policy);

    enum SizeAdjustPolicy {
        AdjustToContents,
        AdjustToContentsOnFirstShow,
        AdjustToMinimumContentsLength
    };

    SizeAdjustPolicy sizeAdjustPolicy() const;
    void setSizeAdjustPolicy(SizeAdjustPolicy policy);
    int minimumContentsLength() const;
    void setMinimumContentsLength(int characters);
    QSize iconSize() const;
    void setIconSize(const QSize &size);

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

    int currentIndex() const;
    void setCurrentIndex(int index);

    QString currentText() const;

    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    QVariant itemData(int index, int role = Qt::UserRole) const;

    inline void addItem(const QString &text, const QVariant &userData = QVariant());
    inline void addItem(const QIcon &icon, const QString &text,
                        const QVariant &userData = QVariant());
    inline void addItems(const QStringList &texts)
        { insertItems(count(), texts); }

    inline void insertItem(int index, const QString &text, const QVariant &userData = QVariant());
    void insertItem(int index, const QIcon &icon, const QString &text,
                    const QVariant &userData = QVariant());
    void insertItems(int index, const QStringList &texts);

    void removeItem(int index);

    void setItemText(int index, const QString &text);
    void setItemIcon(int index, const QIcon &icon);
    void setItemData(int index, const QVariant &value, int role = Qt::UserRole);

    QAbstractItemView *view() const;
    void setView(QAbstractItemView *itemView);

    QSize sizeHint() const;

    virtual void showPopup();
    virtual void hidePopup();

    void hide();
    bool event(QEvent *event);

public slots:
    void clear();
    void clearEditText();
    void setEditText(const QString &text);

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
    void showEvent(QShowEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);
    void inputMethodEvent(QInputMethodEvent *);
    QVariant inputMethodQuery(Qt::InputMethodQuery) const;

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QComboBox(QWidget *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QComboBox(bool rw, QWidget *parent, const char *name = 0);
    inline QT3_SUPPORT int currentItem() const { return currentIndex(); }
    inline QT3_SUPPORT void setCurrentItem(int index) { setCurrentIndex(index); }
    inline QT3_SUPPORT InsertPolicy insertionPolicy() const { return insertPolicy(); }
    inline QT3_SUPPORT void setInsertionPolicy(InsertPolicy policy) { setInsertPolicy(policy); }
    inline QT3_SUPPORT bool editable() const { return isEditable(); }
    inline QT3_SUPPORT void popup() { showPopup(); }
    inline QT3_SUPPORT void setCurrentText(const QString& text) {
        int i = findText(text);
        if (i != -1)
            setCurrentIndex(i);
        else if (isEditable())
            setEditText(text);
        else
            setItemText(currentIndex(), text);
    }
    inline QT3_SUPPORT QString text(int index) const { return itemText(index); }
    inline QT3_SUPPORT QPixmap pixmap(int index) const { return itemIcon(index).pixmap(QSize(22,22)); }
    inline QT3_SUPPORT void insertStringList(const QStringList &list, int index = -1)
        { insertItems(index, list); }
    inline QT3_SUPPORT void insertItem(const QString &text, int index = -1)
        { insertItem(index, text); }
    inline QT3_SUPPORT void insertItem(const QPixmap &pix, int index = -1)
        { insertItem(index, QIcon(pix), QString()); }
    inline QT3_SUPPORT void insertItem(const QPixmap &pix, const QString &text, int index = -1)
        { insertItem(index, QIcon(pix), text); }
    inline QT3_SUPPORT void changeItem(const QString &text, int index)
        { setItemText(index, text); }
    inline QT3_SUPPORT void changeItem(const QPixmap &pix, int index)
        { setItemIcon(index, QIcon(pix)); }
    inline QT3_SUPPORT void changeItem(const QPixmap &pix, const QString &text, int index)
        { setItemIcon(index, QIcon(pix)); setItemText(index, text); }
    inline QT3_SUPPORT void clearValidator() { setValidator(0); }
    inline QT3_SUPPORT void clearEdit() { clearEditText(); }
#endif

private:
    Q_DECLARE_PRIVATE(QComboBox)
    Q_DISABLE_COPY(QComboBox)
    Q_PRIVATE_SLOT(d, void itemSelected(const QModelIndex &item))
    Q_PRIVATE_SLOT(d, void emitHighlighted(const QModelIndex &))
    Q_PRIVATE_SLOT(d, void returnPressed())
    Q_PRIVATE_SLOT(d, void complete())
    Q_PRIVATE_SLOT(d, void resetButton())
    Q_PRIVATE_SLOT(d, void dataChanged(const QModelIndex &, const QModelIndex &))
    Q_PRIVATE_SLOT(d, void rowsInserted(const QModelIndex & parent, int start, int end))
    Q_PRIVATE_SLOT(d, void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end))
};

inline void QComboBox::addItem(const QString &atext, const QVariant &auserData)
{ insertItem(count(), atext, auserData); }
inline void QComboBox::addItem(const QIcon &aicon, const QString &atext,
                               const QVariant &auserData)
{ insertItem(count(), aicon, atext, auserData); }

inline void QComboBox::insertItem(int aindex, const QString &atext,
                                  const QVariant &auserData)
{ insertItem(aindex, QIcon(), atext, auserData); }

#endif // QCOMBOBOX_H
