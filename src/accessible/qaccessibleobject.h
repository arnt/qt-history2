/****************************************************************************
**
** Definition of the QAccessibleObject and QAccessibleApplication classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QAccessibleObjectPrivate;
class QObject;

class Q_EXPORT QAccessibleObject : public Qt, public QAccessibleInterface
{
public:
    QAccessibleObject(QObject *object);

    QRESULT	queryInterface(const QUuid &, QUnknownInterface**);
    Q_REFCOUNT

    bool	isValid() const;
    QObject	*object() const;

    // properties
    QRect	rect(int control) const;
    void	setText(Text t, int control, const QString &text);

    int		propertyCount(int control) const;
    QString	propertyText(int property, Text t, int control) const;
    QString	property(int property, int control) const;
    void	setProperty(int property, const QString& value, int control);

    // selections
    bool	setSelected(int control, bool on, bool extend);
    void	clearSelection();
    QVector<int> selection() const;

    // actions
    int		actionCount(int control) const;
    int		defaultAction(int control) const;
    bool	doAction(int action, int control);
    QString	actionText(int action, Text t, int control) const;

protected:
    virtual ~QAccessibleObject();

private:
    QAccessibleObjectPrivate *d;
};

class Q_EXPORT QAccessibleApplication : public QAccessibleObject
{
public:
    QAccessibleApplication();

    // relations
    int		childCount() const;
    int		indexOfChild(const QAccessibleInterface*) const;
    int		relationTo(int, const QAccessibleInterface *, int) const;

    // navigation
    int		childAt(int x, int y) const;
    int		navigate(Relation, int, QAccessibleInterface **) const;

    // properties and state
    QString	text(Text t, int control) const;
    Role	role(int control) const;
    State	state(int control) const;

    // actions
    int		defaultAction(int control) const;
    bool	doAction(int action, int control);
    QString	actionText(int action, Text t, int control) const;
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLEOBJECT_H
