/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include <qaccessiblewidget.h>

class QMenu;
class QMenuBar;

class QAccessibleMenu : public QAccessibleWidget
{
public:
    QAccessibleMenu(QWidget *w);

    int                childCount() const;
    int                childAt(int x, int y) const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QMenu *menu() const;
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
