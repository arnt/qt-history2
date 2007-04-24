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


#include "qdesigner_propertyeditor.h"
#include "qdesigner_workbench.h"

#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerComponents>

#include <QtGui/QAction>

QDesignerPropertyEditor::QDesignerPropertyEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("PropertyEditor"));
    QDesignerPropertyEditorInterface *widget = QDesignerComponents::createPropertyEditor(workbench->core(), this);
    workbench->core()->setPropertyEditor(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Property Editor"));
    action()->setShortcut(tr("Ctrl+I"));
    action()->setObjectName(QLatin1String("__qt_property_editor_action"));
}

QDesignerPropertyEditor::~QDesignerPropertyEditor()
{
}

QRect QDesignerPropertyEditor::geometryHint() const
{
    const QRect g = workbench()->availableGeometry();
    const int margin = workbench()->marginHint();
    const int spacing = 40;

    const QSize sz(g.width() * 1/4, g.height() * 4/6);

    return QRect((g.width() - sz.width() - margin), (margin + g.height() * 1/6) + spacing,
                  sz.width(), sz.height());
}

void QDesignerPropertyEditor::showEvent(QShowEvent *event)
{
    if (QDesignerPropertyEditorInterface *e = workbench()->core()->propertyEditor()) {
        // workaround to update the propertyeditor when it is not visible!
        e->setObject(e->object()); // ### remove me
    }

    QDesignerToolWindow::showEvent(event);
}

