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

    enum SpinBoxElements {
        SpinBoxSelf        = 0,
        Editor,
        ValueUp,
        ValueDown
    };

    int                childCount() const;
    QRect        rect(int child) const;

    int                navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QSpinBox        *spinBox() const;
};

class QAccessibleScrollBar : public QAccessibleWidget
{
public:
    QAccessibleScrollBar(QWidget *w, const QString &name = QString());

    enum ScrollBarElements {
        ScrollBarSelf        = 0,
        LineUp,
        PageUp,
        Position,
        PageDown,
        LineDown
    };

    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QScrollBar *scrollBar() const;
};

class QAccessibleSlider : public QAccessibleWidget
{
public:
    QAccessibleSlider(QWidget *w, const QString &name = QString());

    enum SliderElements {
        SliderSelf  = 0,
        PageLeft,
        Position,
        PageRight
    };

    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;

    int                defaultAction(int child) const;
    QString        actionText(int action, Text t, int child);
    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QSlider *slider() const;
};

#endif // RANGECONTROLS_H
