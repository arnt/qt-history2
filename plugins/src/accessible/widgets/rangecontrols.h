#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include <qaccessiblewidget.h>

class QScrollBar;
class QSlider;

class QAccessibleRangeControl : public QAccessibleWidget
{
public:
    QAccessibleRangeControl(QWidget *o, Role role, const QString &name = QString());

    QString	text(Text t, int child) const;
};

class QAccessibleSpinWidget : public QAccessibleRangeControl
{
public:
    QAccessibleSpinWidget(QWidget *w);

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
    QAccessibleScrollBar(QWidget *w, const QString &name = QString());

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
    QAccessibleSlider(QWidget *w, const QString &name = QString());

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
