#ifndef SIMPLEWIDGETS_H
#define SIMPLEWIDGETS_H

#include "qaccessiblewidgets.h"

class QButton;

class QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton(QWidget *o, Role r, QString description = QString(),
	QString help = QString());

    QString	text(Text t, int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QButton *button() const;
};

class QAccessibleDisplay : public QAccessibleWidget
{
public:
    QAccessibleDisplay(QWidget *o, Role role, QString description = QString(), 
	QString value = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    QString	text(Text t, int child) const;
    Role	role(int child) const;

    int		relationTo(int child, const QAccessibleInterface *other, int otherChild) const;
};

class QAccessibleText : public QAccessibleWidget
{
public:
    QAccessibleText(QWidget *o, Role role, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    QString	text(Text t, int child) const;
    State	state(int child) const;
};

#endif // SIMPLEWIDGETS_H
