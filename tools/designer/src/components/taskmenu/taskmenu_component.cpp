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

#include "taskmenu_component.h"
#include "button_taskmenu.h"
#include "groupbox_taskmenu.h"
#include "label_taskmenu.h"

#include <abstractformeditor.h>

#include <extension.h>
#include <qextensionmanager.h>

TaskMenuComponent::TaskMenuComponent(AbstractFormEditor *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    Q_ASSERT(m_core != 0);

    QExtensionManager *mgr = core->extensionManager();

    ButtonTaskMenuFactory *button_factory = new ButtonTaskMenuFactory(mgr);
    mgr->registerExtensions(button_factory, Q_TYPEID(ITaskMenu));

    GroupBoxTaskMenuFactory *groupbox_factory = new GroupBoxTaskMenuFactory(mgr);
    mgr->registerExtensions(groupbox_factory, Q_TYPEID(ITaskMenu));

    LabelTaskMenuFactory *label_factory = new LabelTaskMenuFactory(mgr);
    mgr->registerExtensions(label_factory, Q_TYPEID(ITaskMenu));

}

TaskMenuComponent::~TaskMenuComponent()
{
}

AbstractFormEditor *TaskMenuComponent::core() const
{
    return m_core;
}

