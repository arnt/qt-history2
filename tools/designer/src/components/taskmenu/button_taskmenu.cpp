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

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>

#include <QtGui/QAction>
#include <QtGui/QLineEdit>
#include <QtGui/QVariant>

#include <QtCore/QEvent>
#include <QtCore/qdebug.h>

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

#if 0 // ### implement me
    action = new QAction(that);
    action->setText(tr("Edit button icon"));
    connect(action, SIGNAL(triggered()), this, SLOT(editIcon()));
    m_taskActions.append(action);
#endif

    return m_taskActions;
}

bool ButtonTaskMenu::eventFilter(QObject *object, QEvent *event)
{
    QLineEdit *lineEditor = qt_cast<QLineEdit*>(object);
    if (!lineEditor)
        return false;

    switch (event->type()) {
        default: break;

        case QEvent::FocusOut:
            lineEditor->deleteLater();
            break;
    }

    return false;
}

void ButtonTaskMenu::editText()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_button);
    if (m_formWindow != 0) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_button->parentWidget() != 0);

        m_editor = new QLineEdit();
        m_editor->setBackgroundRole(m_button->backgroundRole());
        m_editor->setObjectName("__qt__passive_m_editor");
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(const QString &)), this, SLOT(updateText(const QString&)));
        m_editor->installEventFilter(this); // ### we need this??
        m_editor->setParent(m_button->parentWidget(), Qt::WStyle_ToolTip);
        m_editor->setGeometry(m_button->geometry());
        m_editor->setFocus();
        m_editor->show();
    }
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

void ButtonTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setProperty(QLatin1String("text"), QVariant(text));
}

void ButtonTaskMenu::updateSelection()
{
    if (!m_formWindow || m_formWindow != m_formWindow->core()->formWindowManager()->activeFormWindow()) {
        m_editor->deleteLater();
    } else {
        AbstractFormWindowCursor *c = m_formWindow->cursor();
        Q_ASSERT(c != 0);

        if (!(c->selectedWidgetCount() == 1 && c->selectedWidget(0) == m_button))
            m_editor->deleteLater();
    }
}

