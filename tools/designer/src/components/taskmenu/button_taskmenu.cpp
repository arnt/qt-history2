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
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_button(button)
{
    QAction *action = 0;

    action = new QAction(this);
    action->setText(tr("Edit Button Text"));
    connect(action, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(action);
}

ButtonTaskMenu::~ButtonTaskMenu()
{
}

QList<QAction*> ButtonTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
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
        m_editor->setObjectName("__qt__passive_m_editor");
        m_editor->installEventFilter(this); // ### we need this??

        m_editor->setFrame(false);
        m_editor->setText(m_button->text());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_button->backgroundRole());
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(const QString &)), this, SLOT(updateText(const QString&)));

        QStyleOptionButton opt;
        opt.init(m_button);
        QRect r = m_button->style()->subRect(QStyle::SR_PushButtonContents, &opt, m_button);

        m_editor->setAttribute(Qt::WA_NoChildEventsForParent);
        m_editor->setParent(m_button->window());
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
    if (m_editor)
        m_editor->deleteLater();
}

