#ifndef SIMPLEWIDGETS_H
#define SIMPLEWIDGETS_H

#include <qaccessiblewidget.h>

class QButton;
class QLineEdit;
class QToolButton;

class QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton(QWidget *w, Role r);

    QString	text(Text t, int child) const;
    int		state(int child) const;

    int		numActions(int child) const;
    QString	actionText(int action, Text text, int child) const;
    bool	doAction(int action, int child);

protected:
    QButton *button() const;
};

class QAccessibleToolButton : public QAccessibleButton
{
public:
    QAccessibleToolButton(QWidget *w, Role role);

    enum ToolButtonElements {
	ToolButtonSelf	= 0,
	ButtonExecute,
	ButtonDropMenu
    };

    Role	role(int child) const;
    int		state(int child) const;

    int		childCount() const;
    QRect	rect(int child) const;

    QString	text(Text t, int child) const;

    int		numActions(int child) const;
    QString	actionText(int action, Text text, int child) const;
    bool	doAction(int action, int child);

protected:
    QToolButton *toolButton() const;

    bool	isSplitButton() const;
};

class QAccessibleDisplay : public QAccessibleWidget
{
public:
    QAccessibleDisplay(QWidget *w, Role role = StaticText);

    QString	text(Text t, int child) const;
    Role	role(int child) const;

    int		relationTo(int child, const QAccessibleInterface *other, int otherChild) const;
    int		navigate(Relation, int entry, QAccessibleInterface **target) const;
};

class QAccessibleLineEdit : public QAccessibleWidget
{
public:
    QAccessibleLineEdit(QWidget *o, const QString &name = QString());

    QString	text(Text t, int child) const;
    int		state(int child) const;

protected:
    QLineEdit *lineEdit() const;
};

#endif // SIMPLEWIDGETS_H
