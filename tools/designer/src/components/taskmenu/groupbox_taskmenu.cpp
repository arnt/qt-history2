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

#include "groupbox_taskmenu.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>

#include <QtGui/QAction>
#include <QtGui/QLineEdit>
#include <QtGui/QVariant>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/qdebug.h>

GroupBoxTaskMenu::GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent)
    : QObject(parent),
      m_groupbox(groupbox)
{
}

GroupBoxTaskMenu::~GroupBoxTaskMenu()
{
}

QList<QAction*> GroupBoxTaskMenu::taskActions() const
{
    if (!m_taskActions.isEmpty())
        return m_taskActions;

    QAction *action = 0;

    GroupBoxTaskMenu *that = const_cast<GroupBoxTaskMenu*>(this);

    action = new QAction(that);
    action->setText(tr("Edit title"));
    connect(action, SIGNAL(triggered()), this, SLOT(editTitle()));
    m_taskActions.append(action);

#if 0 // ### implement me
    action = new QAction(that);
    action->setText(tr("Edit groupbox icon"));
    connect(action, SIGNAL(triggered()), this, SLOT(editIcon()));
    m_taskActions.append(action);
#endif

    return m_taskActions;
}

bool GroupBoxTaskMenu::eventFilter(QObject *object, QEvent *event)
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

void GroupBoxTaskMenu::editTitle()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_groupbox);
    if (m_formWindow != 0) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_groupbox->parentWidget() != 0);

        m_editor = new QLineEdit();
        m_editor->setFrame(false);
        m_editor->setText(m_groupbox->title());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_groupbox->backgroundRole());
        m_editor->setObjectName("__qt__passive_m_editor");
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(const QString &)), this, SLOT(updateText(const QString&)));
        m_editor->installEventFilter(this); // ### we need this??
        QStyleOption opt; // ## QStyleOptionGroupBox
        opt.init(m_groupbox);
        QRect r = QRect(QPoint(), m_groupbox->size()); // ### m_groupbox->style()->subRect(QStyle::SR_GroupBoxTitle, &opt, m_groupbox);
        r.setHeight(20);
        m_editor->setParent(m_groupbox->parentWidget(), Qt::WStyle_ToolTip);
        m_editor->setGeometry(QRect(m_groupbox->mapToParent(r.topLeft()), r.size()));
        m_editor->setFocus();
        m_editor->show();
    }
}

void GroupBoxTaskMenu::editIcon()
{
    qDebug() << "edit icon";
}

GroupBoxTaskMenuFactory::GroupBoxTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *GroupBoxTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QGroupBox *groupbox = qt_cast<QGroupBox*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new GroupBoxTaskMenu(groupbox, parent);
        }
    }

    return 0;
}

void GroupBoxTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setProperty(QLatin1String("title"), QVariant(text));
}

void GroupBoxTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

