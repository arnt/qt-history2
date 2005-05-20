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

#include "qdesigner_components.h"

#include <formeditor/formeditor.h>
#include <widgetbox/widgetbox.h>
#include <propertyeditor/propertyeditor.h>
#include <objectinspector/objectinspector.h>
#include <taskmenu/taskmenu_component.h>
#include <resourceeditor/resourceeditor.h>
#include <signalsloteditor/signalsloteditorwindow.h>

void QDesignerComponents::initializeResources()
{
    extern bool qInitResources_formeditor();
    extern bool qInitResources_widgetbox();

    qInitResources_formeditor();
    qInitResources_widgetbox();
}

QDesignerFormEditorInterface *QDesignerComponents::createFormEditor(QObject *parent)
{
    return new qdesigner_internal::FormEditor(parent);
}

QObject *QDesignerComponents::createTaskMenu(QDesignerFormEditorInterface *core, QObject *parent)
{
    return new qdesigner_internal::TaskMenuComponent(core, parent);
}

QDesignerWidgetBoxInterface *QDesignerComponents::createWidgetBox(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::WidgetBox(core, parent);
}

QDesignerPropertyEditorInterface *QDesignerComponents::createPropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::PropertyEditor(core, parent);
}

QDesignerObjectInspectorInterface *QDesignerComponents::createObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::ObjectInspector(core, parent);
}

QWidget *QDesignerComponents::createResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::ResourceEditor(core, parent);
}

QWidget *QDesignerComponents::createSignalSlotEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::SignalSlotEditorWindow(core, parent);
}

