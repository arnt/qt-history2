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

#include "specialeditorsupport.h"
#include "defaultspecialeditor.h"

// Sdk
#include <abstractformeditor.h>
#include <qextensionmanager.h>

SpecialEditorSupport::SpecialEditorSupport(AbstractFormEditor *core)
    : QObject(core),
      m_core(core)
{
    m_factory = new QDesignerSpecialEditorFactory(core->extensionManager());
    core->extensionManager()->registerExtensions(m_factory, Q_TYPEID(ISpecialEditor));
}

SpecialEditorSupport::~SpecialEditorSupport()
{
//    m_core->extensionManager()->unregisterExtensions(m_factory, Q_TYPEID(ISpecialEditor));
}
