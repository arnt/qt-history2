#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <qaccessiblewidget.h>

class QWidgetStack;

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack(QWidget *o);

    int		childCount() const;
    int		indexOfChild(const QAccessibleInterface*) const;

    int		childAt(int x, int y) const;

    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

protected:
    QWidgetStack *widgetStack() const;
};


#endif // CONTAINERS_H
