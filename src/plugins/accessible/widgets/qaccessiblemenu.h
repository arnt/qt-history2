#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include <qaccessiblewidget.h>

class QPopupMenu;
class QMenuBar;

class QAccessiblePopup : public QAccessibleWidget
{
public:
    QAccessiblePopup(QWidget *w);

    int                childCount() const;
    int                childAt(int x, int y) const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QPopupMenu *popupMenu() const;
};

class QAccessibleMenuBar : public QAccessibleWidget
{
public:
    QAccessibleMenuBar(QWidget *w);

    int                childCount() const;
    int                childAt(int x, int y) const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QMenuBar *menuBar() const;
};

#endif // QACCESSIBLEMENU_H
