#ifndef Q3COMPLEXWIDGETS_H
#define Q3COMPLEXWIDGETS_H

#include <qaccessiblewidget.h>

class Q3Header;

class Q3AccessibleHeader : public QAccessibleWidget
{
public:
    Q3AccessibleHeader(QWidget *w);

    int childCount() const;

    QRect rect(int child) const;
    QString text(Text t, int child) const;
    Role role(int child) const;
    int state(int child) const;

protected:
    Q3Header *header() const;
};

#endif

