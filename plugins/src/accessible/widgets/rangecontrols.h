#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include "qaccessiblewidgets.h"

class QScrollBar;
class QSlider;

class QAccessibleRangeControl : public QAccessibleWidget
{
public:
    QAccessibleRangeControl(QWidget *o, Role role, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    QString	text(Text t, int child) const;
};

class QAccessibleSpinWidget : public QAccessibleRangeControl
{
public:
    QAccessibleSpinWidget(QWidget *o);

    int		childCount() const;
    QRect	rect(int child) const;

    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);
};

class QAccessibleScrollBar : public QAccessibleRangeControl
{
public:
    QAccessibleScrollBar(QWidget *o, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;

    bool	doAction(int action, int child);

protected:
    QScrollBar *scrollBar() const;
};

class QAccessibleSlider : public QAccessibleRangeControl
{
public:
    QAccessibleSlider(QWidget *o, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;
    int		relationTo(int child, const QAccessibleInterface *other, int otherChild);

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;

    bool	doAction(int action, int child);

protected:
    QSlider *slider() const;
};

#endif // RANGECONTROLS_H
