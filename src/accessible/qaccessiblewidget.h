/****************************************************************************
**
** Definition of the QAccessibleWidget class.
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

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#ifndef QT_H
#include "qaccessibleobject.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QAccessibleWidgetPrivate;

class Q_EXPORT QAccessibleWidget : public QAccessibleObject
{
public:
    QAccessibleWidget( QWidget *o, Role r = Client, QString name = QString(), 
	QString description = QString(), QString value = QString(), 
	QString help = QString(), int defAction = SetFocus, QString defActionName = QString(),
	QString accelerator = QString(), State s = Normal );

    void	addControllingSignal(const QString &signal);

    int		childCount() const;
    int		indexOfChild(const QAccessibleInterface*) const;
    int		relationTo(int, const QAccessibleInterface *, int) const;

    int		childAt(int x, int y) const;
    QRect	rect(int control) const;
    int		navigate(Relation, int, QAccessibleInterface **) const;

    QString	text(Text t, int control) const;
    Role	role(int control) const;
    State	state(int control) const;

    int		defaultAction(int control) const;
    bool	doAction(int action, int control);
    QString	actionText(int action, Text t, int control) const;

protected:
    ~QAccessibleWidget();
    QWidget *widget() const;

private:
    QAccessibleWidgetPrivate *d;
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLEWIDGET_H
