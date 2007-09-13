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

#include "qdesigner_actioneditor.h"
#include "qdesigner_workbench.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerActionEditorInterface>
#include <QtDesigner/QDesignerComponents>

#include <QtGui/QAction>

QT_BEGIN_NAMESPACE

QDesignerActionEditor::QDesignerActionEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("ActionEditor"));
    QDesignerActionEditorInterface *widget = QDesignerComponents::createActionEditor(workbench->core(), this);
    workbench->core()->setActionEditor(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Action Editor"));
    action()->setObjectName(QLatin1String("__qt_action_editor_tool_action"));
}

QDesignerActionEditor::~QDesignerActionEditor()
{
}

QRect QDesignerActionEditor::geometryHint() const
{
    const QRect g = workbench()->availableGeometry();
    const int margin = workbench()->marginHint();

    const QSize sz(g.width() * 1/4, g.height() * 1/6);

    return QRect((g.width() - sz.width() - margin), margin,
                  sz.width(), sz.height());
}

QT_END_NAMESPACE
