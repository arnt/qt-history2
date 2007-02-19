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


#include "qdesigner_formwindowcommand_p.h"
#include "layout_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerObjectInspectorInterface>
#include <QtDesigner/QDesignerActionEditorInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QVariant>
#include <QtGui/QWidget>
#include <QtGui/QLabel>

namespace qdesigner_internal {

// ---- QDesignerFormWindowCommand ----
QDesignerFormWindowCommand::QDesignerFormWindowCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
    : QUndoCommand(description),
      m_formWindow(formWindow)
{
}

QDesignerFormWindowInterface *QDesignerFormWindowCommand::formWindow() const
{
    return m_formWindow;
}

QDesignerFormEditorInterface *QDesignerFormWindowCommand::core() const
{
    if (QDesignerFormWindowInterface *fw = formWindow())
        return fw->core();

    return 0;
}

void QDesignerFormWindowCommand::undo()
{
    cheapUpdate();
}

void QDesignerFormWindowCommand::redo()
{
    cheapUpdate();
}

void QDesignerFormWindowCommand::cheapUpdate()
{
    if (core()->objectInspector())
        core()->objectInspector()->setFormWindow(formWindow());

    if (core()->actionEditor())
        core()->actionEditor()->setFormWindow(formWindow());
}

bool QDesignerFormWindowCommand::hasLayout(QWidget *widget) const
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    if (widget && LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout) {
        const QDesignerMetaDataBaseItemInterface *item = core->metaDataBase()->item(widget);
        return item != 0;
    }

    return false;
}

QDesignerPropertySheetExtension* QDesignerFormWindowCommand::propertySheet(QObject *object) const
{
    return  qt_extension<QDesignerPropertySheetExtension*>(formWindow()->core()->extensionManager(), object);
}

void QDesignerFormWindowCommand::updateBuddies(QDesignerFormWindowInterface *form,
                                               const QString &old_name,
                                               const QString &new_name)
{
    QExtensionManager* extensionManager = form->core()->extensionManager();

    typedef QList<QLabel*> LabelList;

    const LabelList label_list = qFindChildren<QLabel*>(form);
    if (label_list.empty())
        return;
    
    const QString buddyProperty = QLatin1String("buddy");

    const LabelList::const_iterator cend = label_list.constEnd();
    for (LabelList::const_iterator it = label_list.constBegin(); it != cend; ++it ) {
        if (QDesignerPropertySheetExtension* sheet = qt_extension<QDesignerPropertySheetExtension*>(extensionManager, *it)) {
            const int idx = sheet->indexOf(buddyProperty);
            if (idx != -1 && sheet->property(idx).toString() == old_name)
                sheet->setProperty(idx, new_name);
        }
    }
}


void QDesignerFormWindowCommand::checkParent(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    if (widget->parentWidget() != parentWidget)
        widget->setParent(parentWidget);
}

} // namespace qdesigner_internal
