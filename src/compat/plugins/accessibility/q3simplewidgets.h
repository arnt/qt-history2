#ifndef Q3SIMPLEWIDGETS_H
#define Q3SIMPLEWIDGETS_H

#include <qaccessiblewidget.h>

class Q3AccessibleDisplay : public QAccessibleWidget
{
public:
    Q3AccessibleDisplay(QWidget *w, Role role = StaticText);

    QString text(Text t, int child) const;
    Role role(int child) const;

    int relationTo(int child, const QAccessibleInterface *other, int otherChild) const;
    int navigate(Relation, int entry, QAccessibleInterface **target) const;
};

#endif // Q3SIMPLEWIDGETS_H
