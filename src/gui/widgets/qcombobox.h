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
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide)

public:
    explicit QComboBox(QWidget *parent = 0);
    ~QComboBox();

    int sizeLimit() const;
    void setSizeLimit(int limit);

    int count() const;
    void setMaxCount(int max);
    int maxCount() const;

    bool autoCompletion() const;
    void setAutoCompletion(bool enable);

    bool duplicatesEnabled() const;
    void setDuplicatesEnabled(bool enable);

    bool autoHide() const;
    void setAutoHide(bool enable);

    bool contains(const QString &text) const;
    virtual int findItem(const QString &text, QAbstractItemModel::MatchFlags flags = QAbstractItemModel::MatchDefault) const;

    enum InsertionPolicy {
        NoInsertion,
        AtTop,
        AtCurrent,
        AtBottom,
        AfterCurrent,
        BeforeCurrent
    };

    InsertionPolicy insertionPolicy() const;
    void setInsertionPolicy(InsertionPolicy policy);

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

    QModelIndex rootIndex() const;
    void setRootIndex(const QModelIndex &index);

    int currentItem() const;
    void setCurrentItem(int row);

    QString currentText() const;
    void setCurrentText(const QString&);

    QString text(int row) const;
    QPixmap pixmap(int row) const;
    QVariant data(int role, int row) const;

    void insertStringList(const QStringList &list, int row = -1);
    void insertItem(const QString &text, int row = -1);
    void insertItem(const QIcon &icon, int row = -1);
    void insertItem(const QIcon &icon, const QString &text, int row = -1);

    void removeItem(int row);

    void setItemText(const QString &text, int row);
    void setItemIcon(const QIcon &icon, int row);
    void setItemData(int role, const QVariant &value, int row);
    void setItem(const QIcon &icon, const QString &text, int row);

    QAbstractItemView *itemView() const;
    void setItemView(QAbstractItemView *itemView);

    QSize sizeHint() const;

    virtual void popup();

    void hide();

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
    QT_COMPAT bool editable() const { return isEditable(); }
    QT_COMPAT void insertItem(const QPixmap &pix, int row = -1)
        { insertItem(QIcon(pix), row); }
    QT_COMPAT void insertItem(const QPixmap &pix, const QString &text, int row = -1)
        { insertItem(QIcon(pix), text, row); }
    QT_COMPAT void changeItem(const QString &text, int row)
        { setItemText(text, row); }
    QT_COMPAT void changeItem(const QPixmap &pix, int row)
        { setItemIcon(QIcon(pix), row); }
    QT_COMPAT void changeItem(const QPixmap &pix, const QString &text, int row)
        { setItem(QIcon(pix), text, row); }
#endif

protected:
    QComboBox(QComboBoxPrivate &dd, QWidget *parent = 0);

private:
    Q_DECLARE_PRIVATE(QComboBox)
    Q_DISABLE_COPY(QComboBox)
    Q_PRIVATE_SLOT(d, void itemSelected(const QModelIndex &item))
    Q_PRIVATE_SLOT(d, void emitHighlighted(const QModelIndex&))
    Q_PRIVATE_SLOT(d, void returnPressed())
    Q_PRIVATE_SLOT(d, void complete())
    Q_PRIVATE_SLOT(d, void resetButton())
};

#endif // QCOMBOBOX_H
