#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <qaccessiblewidget.h>

class QWidgetStack;

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack(QWidget *o);

    int		childAt(int x, int y) const;

protected:
    QWidgetStack *widgetStack() const;
};


#endif // CONTAINERS_H
