#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include <qaccessiblewidget.h>

class QScrollBar;
class QSlider;
class QSpinBox;

class QAccessibleSpinBox : public QAccessibleWidget
{
public:
    QAccessibleSpinBox(QWidget *w);

    int		childCount() const;
    QRect	rect(int child) const;

    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    int		state(int child) const;

    bool	doAction(int action, int child);

protected:
    QSpinBox	*spinBox() const;
};

class QAccessibleScrollBar : public QAccessibleWidget
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

class QAccessibleSlider : public QAccessibleWidget
{
public:
    QAccessibleSlider(QWidget *w, const QString &name = QString());

    int		childCount() const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;

    bool	doAction(int action, int child);

protected:
    QSlider *slider() const;
};

#endif // RANGECONTROLS_H
