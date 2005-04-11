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
#include "inplace_editor.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_button(button)
{
    m_preferredEditAction = new QAction(this);
    m_preferredEditAction->setText(tr("Edit Button Text"));
    connect(m_preferredEditAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_preferredEditAction);
}

ButtonTaskMenu::~ButtonTaskMenu()
{
}

QAction *ButtonTaskMenu::preferredEditAction() const
{
    return m_preferredEditAction;
}

QList<QAction*> ButtonTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void ButtonTaskMenu::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_button);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_button->parentWidget() != 0);

        m_editor = new InPlaceEditor(m_button, m_formWindow);
        m_editor->setObjectName("__qt__passive_m_editor");

        m_editor->setFrame(false);
        m_editor->setText(m_button->text());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_button->backgroundRole());
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(const QString &)), this, SLOT(updateText(const QString&)));

        QStyleOptionButton opt;
        opt.init(m_button);
        QRect r = m_button->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, m_button);

        m_editor->setGeometry(QRect(m_button->mapTo(m_button->window(), r.topLeft()), r.size()));
        m_editor->setFocus();
        m_editor->show();
    }
}

void ButtonTaskMenu::editIcon()
{
    qDebug() << "edit icon";
}

ButtonTaskMenuFactory::ButtonTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *ButtonTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QAbstractButton *button = qobject_cast<QAbstractButton*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new ButtonTaskMenu(button, parent);
        }
    }

    return 0;
}

void ButtonTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setWidgetProperty(m_button, QLatin1String("text"), QVariant(text));
}

void ButtonTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

