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
TRANSLATOR qdesigner_internal::ToolBarTaskMenu
*/

#include "toolbar_taskmenu.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ToolBarTaskMenu::ToolBarTaskMenu(QToolBar *toolbar, QObject *parent)
    : QDesignerTaskMenu(toolbar, parent),
      m_toolbar(toolbar),
      m_editTextAction(new QAction(tr("Customize..."), this))
{
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editToolBar()));
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ToolBarTaskMenu::~ToolBarTaskMenu()
{
}

QAction *ToolBarTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> ToolBarTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ToolBarTaskMenu::editToolBar()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_toolbar);
    if (!m_formWindow.isNull()) {
    }
    Q_ASSERT(0);
}
