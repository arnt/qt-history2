/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#include <QtGui/qaccessibleobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleWidgetPrivate;

class Q_GUI_EXPORT QAccessibleWidget : public QAccessibleObject
{
public:
    explicit QAccessibleWidget(QWidget *o, Role r = Client, const QString& name = QString());

    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    Relation relationTo(int child, const QAccessibleInterface *other, int otherChild) const;

    int childAt(int x, int y) const;
    QRect rect(int child) const;
    int navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const;

    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

    QString actionText(int action, Text t, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    ~QAccessibleWidget();
    QWidget *widget() const;
    QObject *parentObject() const;

    void addControllingSignal(const QString &signal);
    void setValue(const QString &value);
    void setDescription(const QString &desc);
    void setHelp(const QString &help);
    void setAccelerator(const QString &accel);

private:
    QAccessibleWidgetPrivate *d;
};

#endif // QT_NO_ACCESSIBILITY

QT_END_HEADER

#endif // QACCESSIBLEWIDGET_H
