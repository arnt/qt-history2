#ifndef COMPLEXWIDGETS_H
#define COMPLEXWIDGETS_H

#include <qaccessiblewidget.h>

class QButton;
class QHeader;
class QTabBar;
class QComboBox;
class QTitleBar;

class QAccessibleHeader : public QAccessibleWidget
{
public:
    QAccessibleHeader(QWidget *w);

    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

protected:
    QHeader *header() const;
};

class QAccessibleTabBar : public QAccessibleWidget
{
public:
    QAccessibleTabBar(QWidget *w);

    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child);
    bool        setSelected(int child, bool on, bool extend);
    QVector<int> selection() const;

protected:
    QTabBar *tabBar() const;

private:
    QButton *button(int child) const;
};

class QAccessibleComboBox : public QAccessibleWidget
{
public:
    QAccessibleComboBox(QWidget *w);

    enum ComboBoxElements {
        ComboBoxSelf        = 0,
        CurrentText,
        OpenList,
        PopupList
    };

    int                childCount() const;
    int                childAt(int x, int y) const;
    int                indexOfChild(const QAccessibleInterface *child) const;
    int                navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString        text(Text t, int child) const;
    QRect        rect(int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child);

protected:
    QComboBox *comboBox() const;
};

class QAccessibleTitleBar : public QAccessibleWidget
{
public:
    QAccessibleTitleBar(QWidget *w);

    int                childCount() const;

    QString        text(Text t, int child) const;
    QRect        rect(int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child);

protected:
    QTitleBar *titleBar() const;
};

#endif // COMPLEXWIDGETS_H
