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

#ifndef Q3COMPLEXWIDGETS_H
#define Q3COMPLEXWIDGETS_H

#include <qaccessiblewidget.h>

class Q3Header;
class Q3TitleBar;

class Q3AccessibleHeader : public QAccessibleWidget
{
public:
    explicit Q3AccessibleHeader(QWidget *w);

    int childCount() const;

    QRect rect(int child) const;
    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

protected:
    Q3Header *header() const;
};

class Q3AccessibleTitleBar : public QAccessibleWidget
{
public:
    explicit Q3AccessibleTitleBar(QWidget *w);

    int childCount() const;

    QString text(Text t, int child) const;
    QRect rect(int child) const;
    Role role(int child) const;
    State state(int child) const;

    bool doAction(int action, int child, const QVariantList &params);

protected:
    Q3TitleBar *titleBar() const;
};

#endif // Q3COMPLEXWIDGETS_H
