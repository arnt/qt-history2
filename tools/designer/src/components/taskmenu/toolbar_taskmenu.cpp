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

#include "toolbar_taskmenu.h"
#include "menueditor.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ToolBarTaskMenu::ToolBarTaskMenu(QToolBar *toolBar, QObject *parent)
    : QDesignerTaskMenu(toolBar, parent),
      m_toolBar(toolBar)
{
    m_editToolBarAction = new QAction(this);
    m_editToolBarAction->setText(tr("Edit Tool Bar..."));

    connect(m_editToolBarAction, SIGNAL(triggered()), this, SLOT(editToolBar()));
    m_taskActions.append(m_editToolBarAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ToolBarTaskMenu::~ToolBarTaskMenu()
{
}

QAction *ToolBarTaskMenu::preferredEditAction() const
{
    return m_editToolBarAction;
}

QList<QAction*> ToolBarTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ToolBarTaskMenu::editToolBar()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_toolBar);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_toolBar != 0);

    MenuEditor dlg(m_formWindow, m_toolBar->window());
    dlg.fillContentsFromWidget(m_toolBar);
    dlg.exec();
}

ToolBarTaskMenuFactory::ToolBarTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *ToolBarTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QToolBar *toolbar = qobject_cast<QToolBar*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new ToolBarTaskMenu(toolbar, parent);
        }
    }

    return 0;
}
