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
TRANSLATOR qdesigner_internal::TreeWidgetTaskMenu
*/

#include "treewidget_taskmenu.h"
#include "inplace_editor.h"
#include "treewidgeteditor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

TreeWidgetTaskMenu::TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_treeWidget(button),
      m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}


TreeWidgetTaskMenu::~TreeWidgetTaskMenu()
{
}

QAction *TreeWidgetTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> TreeWidgetTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TreeWidgetTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_treeWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_treeWidget != 0);

    TreeWidgetEditor dlg(m_formWindow, m_treeWidget->window());
    dlg.fillContentsFromTreeWidget(m_treeWidget);
    if (dlg.exec() == QDialog::Accepted)
        dlg.fillTreeWidgetFromContents(m_treeWidget);
}

TreeWidgetTaskMenuFactory::TreeWidgetTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *TreeWidgetTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QTreeWidget *button = qobject_cast<QTreeWidget*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new TreeWidgetTaskMenu(button, parent);
        }
    }

    return 0;
}

void TreeWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

