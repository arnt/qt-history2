#ifndef QACCESSIBLECOMPAT_H
#define QACCESSIBLECOMPAT_H

#include <qaccessiblewidget.h>
#include <qiconview.h>
#include <qlistview.h>
#include <qtextedit.h>

class Q3AccessibleScrollView : public QAccessibleWidget
{
public:
    Q3AccessibleScrollView(QWidget *w, Role role);

    virtual int itemAt(int x, int y) const;
    virtual QRect itemRect(int item) const;
    virtual int itemCount() const;
};

class QAccessibleListView : public Q3AccessibleScrollView
{
public:
    QAccessibleListView(QWidget *o);

    int                itemAt(int x, int y) const;
    QRect        itemRect(int item) const;
    int                itemCount() const;

    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        setSelected(int child, bool on, bool extend);
    void        clearSelection();
    QVector<int> selection() const;

protected:
    QListView *listView() const;
};

#ifndef QT_NO_ICONVIEW
class QAccessibleIconView : public Q3AccessibleScrollView
{
public:
    QAccessibleIconView(QWidget *o);

    int                itemAt(int x, int y) const;
    QRect        itemRect(int item) const;
    int                itemCount() const;

    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        setSelected(int child, bool on, bool extend);
    void        clearSelection();
    QVector<int> selection() const;

protected:
    QIconView *iconView() const;
};
#endif

class QAccessibleTextEdit : public Q3AccessibleScrollView
{
public:
    QAccessibleTextEdit(QWidget *o);

    int                itemAt(int x, int y) const;
    QRect        itemRect(int item) const;
    int                itemCount() const;

    QString        text(Text t, int child) const;
    void        setText(Text t, int control, const QString &text);
    Role        role(int child) const;

protected:
    QTextEdit *textEdit() const;
};

class QWidgetStack;

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack(QWidget *o);

    int                childCount() const;
    int                indexOfChild(const QAccessibleInterface*) const;

    int                childAt(int x, int y) const;

    int                navigate(Relation rel, int entry, QAccessibleInterface **target) const;

protected:
    QWidgetStack *widgetStack() const;
};

#endif
