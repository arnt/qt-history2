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

#include "button_taskmenu.h"
#include <QAction>
#include <qdebug.h>

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)
    : QObject(parent),
      m_button(button)
{
}

ButtonTaskMenu::~ButtonTaskMenu()
{
}

QList<QAction*> ButtonTaskMenu::taskActions() const
{
    if (!m_taskActions.isEmpty())
        return m_taskActions;

    QAction *action = 0;

    ButtonTaskMenu *that = const_cast<ButtonTaskMenu*>(this);

    action = new QAction(that);
    action->setText(tr("Edit button text"));
    connect(action, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(action);

    action = new QAction(that);
    action->setText(tr("Edit button icon"));
    connect(action, SIGNAL(triggered()), this, SLOT(editIcon()));
    m_taskActions.append(action);

    return m_taskActions;
}

void ButtonTaskMenu::editText()
{
    qDebug() << "edit text";
}

void ButtonTaskMenu::editIcon()
{
    qDebug() << "edit icon";
}

ButtonTaskMenuFactory::ButtonTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *ButtonTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QAbstractButton *button = qt_cast<QAbstractButton*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new ButtonTaskMenu(button, parent);
        }
    }

    return 0;
}

