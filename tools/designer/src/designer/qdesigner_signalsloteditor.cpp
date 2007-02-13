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

#include "qdesigner_signalsloteditor.h"
#include "qdesigner_workbench.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerComponents>

QDesignerSignalSlotEditor::QDesignerSignalSlotEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("SignalSlotEditorWindow"));
    QWidget *widget = QDesignerComponents::createSignalSlotEditor(workbench->core(), this);

    setCentralWidget(widget);

    setWindowTitle(tr("Signal/Slot Editor"));
}

QDesignerSignalSlotEditor::~QDesignerSignalSlotEditor()
{
}

QRect QDesignerSignalSlotEditor::geometryHint() const
{
    const QRect g = workbench()->availableGeometry();
    const int margin = workbench()->marginHint();

    const QSize sz(g.width() * 1/3, g.height() * 1/6);
    QRect r(QPoint(0, 0), sz);
    r.moveCenter(g.center());
    r.moveTop(margin);

    return r;
}
