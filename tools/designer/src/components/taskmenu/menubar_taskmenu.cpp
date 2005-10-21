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

#include "menubar_taskmenu.h"
#include "menueditor.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QMenuBar>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

MenuBarTaskMenu::MenuBarTaskMenu(QMenuBar *menuBar, QObject *parent)
    : QDesignerTaskMenu(menuBar, parent),
      m_menuBar(menuBar)
{
    m_editMenuAction = new QAction(this);
    m_editMenuAction->setText(tr("Edit Menu Bar..."));
    connect(m_editMenuAction, SIGNAL(triggered()), this, SLOT(editMenu()));
    m_taskActions.append(m_editMenuAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}


MenuBarTaskMenu::~MenuBarTaskMenu()
{
}

QAction *MenuBarTaskMenu::preferredEditAction() const
{
    return m_editMenuAction;
}

QList<QAction*> MenuBarTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void MenuBarTaskMenu::editMenu()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_menuBar);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_menuBar != 0);

    MenuEditor dlg(m_formWindow, m_menuBar->window());
    dlg.fillContentsFromWidget(m_menuBar);
    dlg.exec();
}

MenuBarTaskMenuFactory::MenuBarTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *MenuBarTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new MenuBarTaskMenu(menuBar, parent);
        }
    }

    return 0;
}

