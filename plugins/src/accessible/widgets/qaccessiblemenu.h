#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include "qaccessiblewidgets.h"

class QPopupMenu;
class QMenuBar;

class QAccessiblePopup : public QAccessibleWidget
{
public:
    QAccessiblePopup(QWidget *w);

    int		childCount() const;
    int		childAt(int x, int y) const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QPopupMenu *popupMenu() const;
};

class QAccessibleMenuBar : public QAccessibleWidget
{
public:
    QAccessibleMenuBar(QWidget *w);

    int		childCount() const;
    int		childAt(int x, int y) const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QMenuBar *menuBar() const;
};

#endif // QACCESSIBLEMENU_H
