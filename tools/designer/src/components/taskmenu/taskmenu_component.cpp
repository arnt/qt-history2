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
#include <abstractformeditor.h>

TaskMenuComponent::TaskMenuComponent(AbstractFormEditor *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
}

TaskMenuComponent::~TaskMenuComponent()
{
}

AbstractFormEditor *TaskMenuComponent::core() const
{
    return m_core;
}

