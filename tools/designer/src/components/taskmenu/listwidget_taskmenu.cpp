/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::ListWidgetTaskMenu
*/

#include "listwidget_taskmenu.h"
#include "inplace_editor.h"
#include "listwidgeteditor.h"
#include <qdesigner_command_p.h>

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ListWidgetTaskMenu::ListWidgetTaskMenu(QListWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_listWidget(button)
{
    m_editItemsAction = new QAction(this);
    m_editItemsAction->setText(tr("Edit Items..."));
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ListWidgetTaskMenu::~ListWidgetTaskMenu()
{
}

QAction *ListWidgetTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> ListWidgetTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ListWidgetTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_listWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_listWidget != 0);

    ListWidgetEditor dlg(m_formWindow, m_listWidget->window());
    dlg.fillContentsFromListWidget(m_listWidget);
    if (dlg.exec() == QDialog::Accepted) {
        QList<QPair<QString, QIcon> > items;
        for (int i = 0; i < dlg.count(); i++) {
            items.append(qMakePair<QString, QIcon>(dlg.text(i), dlg.icon(i)));
        }
        ChangeListContentsCommand *cmd = new ChangeListContentsCommand(m_formWindow);
        cmd->init(m_listWidget, items);
        cmd->setText(tr("Change List Contents"));
        m_formWindow->commandHistory()->push(cmd);
    }
}

void ListWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

