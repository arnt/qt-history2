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
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QString currentText READ currentText WRITE setCurrentText DESIGNABLE false)
    Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem)
    Q_PROPERTY(int sizeLimit READ sizeLimit WRITE setSizeLimit)
    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount)
    Q_PROPERTY(InsertionPolicy insertionPolicy READ insertionPolicy WRITE setInsertionPolicy)
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

    QGenericComboBox(QWidget *parent = 0);
    QGenericComboBox(bool rw, QWidget *parent=0);
    QGenericComboBox(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericComboBox();

    int sizeLimit() const;
    void setSizeLimit(int limit);

    int count() const;
    void setMaxCount(int max);
    int maxCount() const;

    bool autoCompletion() const;
    void setAutoCompletion(bool enable);

    bool duplicatesEnabled() const;
    void setDuplicatesEnabled(bool enable);
    virtual bool contains(const QString &text);

    InsertionPolicy insertionPolicy() const;
    void setInsertionPolicy(InsertionPolicy policy);

    bool isEditable() const;
    void setEditable(bool editable);
    void setLineEdit(QLineEdit *edit);
    QLineEdit *lineEdit() const;
    void setValidator (const QValidator *v);
    const QValidator * validator () const;

    QAbstractItemDelegate *itemDelegate() const;
    void setItemDelegate(QAbstractItemDelegate *delegate);

    QAbstractItemModel *model() const;

    QModelIndex root() const;
    void setRoot(const QModelIndex &index);

    int currentItem() const;
    void setCurrentItem(int row);

    QString currentText() const;
    void setCurrentText(const QString&);

    QString text (int row) const;
    QPixmap pixmap (int row) const;

    void insertStringList(const QStringList &list, int row = -1);
    void insertItem(const QString &text, int row = -1);
    void insertItem(const QIconSet &icon, int row = -1);
    void insertItem(const QString &text, const QIconSet &icon, int row = -1);

    void removeItem(int row);

    void setItemText(const QString &text, int row);
    void setItemIcon(const QIconSet &icon, int row);
    void setItem(const QString &text, const QIconSet &icon, int row);
#ifdef QT_COMPAT
    QT_COMPAT void changeItem(const QString &text, int row) { setItemText(text, row); }
    QT_COMPAT void changeItem(const QIconSet &icon, int row) { setItemIcon(icon, row); }
    QT_COMPAT void changeItem(const QString &text, const QIconSet &icon, int row)
        { setItem(text, icon, row); }
#endif

    QGenericListView *listView() const;

    QSize sizeHint() const;

    virtual void popup();

public slots:
    void clear();
    void clearValidator();
    void clearEdit();
    virtual void setEditText(const QString &text);

signals:
    void textChanged(const QString &);
    void activated(int row);
    void activated(const QString &);
    void activated(const QModelIndex &);
    void highlighted(int row);
    void highlighted(const QString &);
    void highlighted(const QModelIndex &);
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
    Q_PRIVATE_SLOT(void itemSelected(const QModelIndex &item))
    Q_PRIVATE_SLOT(void emitHighlighted(const QModelIndex&))
    Q_PRIVATE_SLOT(void returnPressed())
    Q_PRIVATE_SLOT(void complete())
};

#endif // QGENERICCOMBOBOX_H
