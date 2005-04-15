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
#include "lineedit_taskmenu.h"
#include "listwidget_taskmenu.h"
#include "combobox_taskmenu.h"
#include "textedit_taskmenu.h"

#include <QtDesigner/abstractformeditor.h>

#include <QtDesigner/extension.h>
#include <QtDesigner/qextensionmanager.h>

TaskMenuComponent::TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    Q_ASSERT(m_core != 0);

    QExtensionManager *mgr = core->extensionManager();

    ButtonTaskMenuFactory *button_factory = new ButtonTaskMenuFactory(mgr);
    mgr->registerExtensions(button_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    GroupBoxTaskMenuFactory *groupbox_factory = new GroupBoxTaskMenuFactory(mgr);
    mgr->registerExtensions(groupbox_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    LabelTaskMenuFactory *label_factory = new LabelTaskMenuFactory(mgr);
    mgr->registerExtensions(label_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    LineEditTaskMenuFactory *lineEdit_factory = new LineEditTaskMenuFactory(mgr);
    mgr->registerExtensions(lineEdit_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    ListWidgetTaskMenuFactory *listWidget_factory = new ListWidgetTaskMenuFactory(mgr);
    mgr->registerExtensions(listWidget_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    ComboBoxTaskMenuFactory *comboBox_factory = new ComboBoxTaskMenuFactory(mgr);
    mgr->registerExtensions(comboBox_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    TextEditTaskMenuFactory *textEdit_factory = new TextEditTaskMenuFactory(mgr);
    mgr->registerExtensions(textEdit_factory, Q_TYPEID(QDesignerTaskMenuExtension));
}

TaskMenuComponent::~TaskMenuComponent()
{
}

QDesignerFormEditorInterface *TaskMenuComponent::core() const
{
    return m_core;
}

