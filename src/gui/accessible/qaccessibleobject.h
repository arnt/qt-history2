/****************************************************************************
**
** Definition of the QAccessibleObject and QAccessibleApplication classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEOBJECT_H
#define QACCESSIBLEOBJECT_H

#ifndef QT_H
#include "qaccessible.h"
#endif // QT_H

#ifndef QT_NO_ACCESSIBILITY_SUPPORT

class QAccessibleObjectPrivate;
class QObject;

class Q_GUI_EXPORT QAccessibleObject : public Qt, public QAccessibleInterface
{
public:
    QAccessibleObject(QObject *object);

    bool isValid() const;
    QObject *object() const;

    // properties
    QRect rect(int child) const;
    void setText(Text t, int child, const QString &text);

    // actions
    int userActionCount(int child) const;
    bool doAction(int action, int child, const QVariantList &params);
    QString actionText(int action, Text t, int child) const;

protected:
    virtual ~QAccessibleObject();

private:
    QAccessibleObjectPrivate *d;
};

class Q_GUI_EXPORT QAccessibleApplication : public QAccessibleObject
{
public:
    QAccessibleApplication();

    // relations
    int childCount() const;
    int indexOfChild(const QAccessibleInterface*) const;
    int relationTo(int, const QAccessibleInterface *, int) const;

    // navigation
    int childAt(int x, int y) const;
    int navigate(Relation, int, QAccessibleInterface **) const;

    // properties and state
    QString text(Text t, int child) const;
    Role role(int child) const;
    int state(int child) const;

    // actions
    int userActionCount(int child) const;
    bool doAction(int action, int child, const QVariantList &params);
    QString actionText(int action, Text t, int child) const;
};

#endif //QT_NO_ACCESSIBILITY

#endif //QACCESSIBLEOBJECT_H
