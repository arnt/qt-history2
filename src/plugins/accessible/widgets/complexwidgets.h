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

#ifndef COMPLEXWIDGETS_H
#define COMPLEXWIDGETS_H

#include <qaccessiblewidget.h>

class QAbstractButton;
class QGenericHeader;
class QTabBar;
class QComboBox;
class QTitleBar;

class QAccessibleHeader : public QAccessibleWidget
{
public:
    QAccessibleHeader(QWidget *w);

    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

protected:
    QGenericHeader *header() const;
};

class QAccessibleTabBar : public QAccessibleWidget
{
public:
    QAccessibleTabBar(QWidget *w);

    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);
    bool        setSelected(int child, bool on, bool extend);
    QVector<int> selection() const;

protected:
    QTabBar *tabBar() const;

private:
    QAbstractButton *button(int child) const;
};

class QAccessibleComboBox : public QAccessibleWidget
{
public:
    QAccessibleComboBox(QWidget *w);

    enum ComboBoxElements {
        ComboBoxSelf        = 0,
        CurrentText,
        OpenList,
        PopupList
    };

    int                childCount() const;
    int                childAt(int x, int y) const;
    int                indexOfChild(const QAccessibleInterface *child) const;
    int                navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString        text(Text t, int child) const;
    QRect        rect(int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QComboBox *comboBox() const;
};

class QAccessibleTitleBar : public QAccessibleWidget
{
public:
    QAccessibleTitleBar(QWidget *w);

    int                childCount() const;

    QString        text(Text t, int child) const;
    QRect        rect(int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);

protected:
    QTitleBar *titleBar() const;
};

#endif // COMPLEXWIDGETS_H
