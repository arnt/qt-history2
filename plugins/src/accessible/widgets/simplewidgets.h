#ifndef SIMPLEWIDGETS_H
#define SIMPLEWIDGETS_H

#include <qaccessiblewidget.h>

class QButton;
class QLineEdit;

class QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton(QWidget *o, Role r);

    QString	text(Text t, int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QButton *button() const;
};

class QAccessibleDisplay : public QAccessibleWidget
{
public:
    QAccessibleDisplay(QWidget *w, Role role = StaticText);

    QString	text(Text t, int child) const;
    Role	role(int child) const;

    int		relationTo(int child, const QAccessibleInterface *other, int otherChild) const;
};

class QAccessibleLineEdit : public QAccessibleWidget
{
public:
    QAccessibleLineEdit(QWidget *o, const QString &name = QString());

    QString	text(Text t, int child) const;
    State	state(int child) const;

protected:
    QLineEdit *lineEdit() const;
};

#endif // SIMPLEWIDGETS_H
